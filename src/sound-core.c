/**
 * \file sound-core.c
 * \brief core sound support
 *
 * Copyright (c) 2016 Graeme Russ <graeme.russ@gmail.com>
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
#include "angband.h"
#include "init.h"
#include "sound.h"
#include "avltree.h"
#include "main.h"

#ifdef SOUND_SDL
#include "snd-sdl.h"
#endif

struct sound_module
{
	const char *name;
	const char *help;
	errr (*init)(struct sound_hooks *hooks, int argc, char **argv);
};

/**
 * List of sound modules in the order they should be tried.
 */
static const struct sound_module sound_modules[] =
{
#ifdef SOUND_SDL
	{ "sdl", "SDL_mixer sound module", init_sound_sdl },
#endif /* SOUND_SDL */

	{ "", "", NULL },
};

/*
 * Two trees are needed:
 * - A mapping of 'sound names' to 'sound ids' - this is used during the
 *   initial parsing of the sound configuration file to ensure we don't
 *   load the same sounds multiple times (if a sound is mapped to more than
 *   one message). When the sound sub-system is initialised, this tree is
 *   'walked', with the sound name stored in each node passed to the platform
 *   sound module which loads the sound
 * - A mapping of 'message ids' to 'sound ids' - In the past, a static array
 *   of size [MAX_MESSAGES][MAX_SOUND_PER_MESSAGE] was used, which results
 *   in a horrendous waste of memory
 */

/*****************************************************************************
 * Structures to map sound names to ids
 *****************************************************************************/
typedef struct _sound_node sound_node;

struct _sound_node
{
	char *sound_name;
	u16b sound_id;

	TREE_ENTRY(_sound_node) sound_tree;
};

static sound_node *sound_node_new(char *sound_name, u16b sound_id)
{
	sound_node *self = mem_zalloc(sizeof *self);

	self->sound_name = string_make(sound_name);
	self->sound_id = sound_id;
	return self;
}

static int sound_node_compare(sound_node *lhs, sound_node *rhs)
{
	return (int)strcmp(lhs->sound_name, rhs->sound_name);
}

typedef TREE_HEAD(_sound_tree, _sound_node) tree_sound;

TREE_DEFINE(_sound_node, sound_tree)

static tree_sound sounds = TREE_INITIALIZER(sound_node_compare);
/*****************************************************************************/

/*****************************************************************************
 * Structures to map message ids to sound ids
 *****************************************************************************/
typedef struct _msg_sound msg_sound;

struct _msg_sound
{
	int sound_id;
	msg_sound *next;
};

typedef struct _message_node message_node;

struct _message_node
{
	u16b message_id;
	int num_sounds;
	msg_sound *first;

	TREE_ENTRY(_message_node) message_tree;
};

static message_node *message_node_new(u16b message_id)
{
	message_node *self = mem_zalloc(sizeof *self);

	self->message_id = message_id;

	return self;
}

static int message_node_compare(message_node *lhs, message_node *rhs)
{
	return (rhs->message_id - lhs->message_id);
}

typedef TREE_HEAD(_message_tree, _message_node) tree_message;

TREE_DEFINE(_message_node, message_tree)

static tree_message messages = TREE_INITIALIZER(message_node_compare);
/*****************************************************************************/

/*
 * After processing the preference files, the 'sounds' AVL Tree will contain
 * next_sound_id nodes - each representing a sound that needs to be loaded.
 * We create the sound_data array once we know this information, and then
 * populate it by calling load_sound_hook() for each node in the tree
 */
static u16b next_sound_id;
static struct sound_data *sounds_list;

#define SOUND_DATA_ARRAY_INC	10

/* These are the hooks installed by the platform sound module */
static struct sound_hooks hooks;

/*
 * If preload_sounds is true, sounds are loaded immediately when assigned to
 * a message. Otherwise, each sound is only loaded when first played.
 *
 * NOTE: Platform sound modules can 'load on play' by never setting the
 * 'loaded' flag in struct sound_data - This will improve memory footprint,
 * by result in a loss of performance.
 */
bool preload_sounds = false;

/*
 * Recursive function to delete sound id's from a node in the 'messages' tree
 */
static void delete_message_sounds(msg_sound *sound)
{
	assert(sound);

	if (sound->next) {
		delete_message_sounds(sound->next);
		sound->next = NULL;	/* Paranoia - not strictly needed */
	}

	mem_free(sound);
}

static struct sound_data *grow_sound_list(void)
{
	int new_size;
	int i;

	if (!sounds_list) {
		sounds_list = mem_zalloc(sizeof(struct sound_data) * SOUND_DATA_ARRAY_INC);
	} else if (0 == (next_sound_id % SOUND_DATA_ARRAY_INC)) {
		/* Add a new chunk onto the sounds list */
		new_size = ((next_sound_id / SOUND_DATA_ARRAY_INC) + 1) * SOUND_DATA_ARRAY_INC;
		sounds_list = mem_realloc(sounds_list, sizeof(struct sound_data) * new_size);

		if (sounds_list)
			/* Clear the new elements */
			for (i = next_sound_id; i < (next_sound_id + SOUND_DATA_ARRAY_INC); i++)
				memset(&sounds_list[i], 0x00, sizeof(struct sound_data));
	}

	return sounds_list;
}

/**
 * Call the platform sound modules 'load sound' function
 */
static void load_sound(struct sound_data *sound_data)
{
	if (hooks.load_sound_hook) {
		if(!hooks.load_sound_hook(sound_data))
			plog_fmt("Failed to load sound: %s", sound_data->name);
	}
}

/**
 * Parse a string of sound names provided by the preferences parser and:
 *  - Add any new sounds to the 'sound names' tree and allocate them
 *    unique 'sound ids'
 *  - Add each sound assigned to a message type to that message types
 *    'sound map
 */
void message_sound_define(u16b message_id, const char *sounds_str)
{
	char *search;
	char *str;
	char *cur_token;
	char *next_token;

	sound_node sound_search_node;
	sound_node *existing_sound;
	sound_node *new_sound;
	u16b sound_id;

	message_node message_search_node;
	message_node *message_map;
	msg_sound *new_message_sound;

	/* Do we already have a message->sounds mapping for this message ID? */
	message_search_node.message_id = message_id;

	message_map = TREE_FIND(&messages, _message_node, message_tree, &message_search_node);

	if (!message_map) {
		/* No - create one */
		message_map = message_node_new(message_id);
		TREE_INSERT(&messages, _message_node, message_tree, message_map);
	} else {
		/* Yes - delete the list of sounds previously allocated */
		delete_message_sounds(message_map->first);
		message_map->first = NULL;
	}

	/* sounds_str is a space separated list of sound names */
	str = cur_token = string_make(sounds_str);

	search = strchr(cur_token, ' ');

	if (search) {
		search[0] = '\0';
		next_token = search + 1;
	} else {
		next_token = NULL;
	}

	/* Find all the sample names and add them one by one */
	while (cur_token) {
		/* Have we already processed this sound name? */
		sound_search_node.sound_name = cur_token;

		existing_sound = TREE_FIND(&sounds, _sound_node, sound_tree, &sound_search_node);

		if (existing_sound) {
			/* Yep - Just need to get the ID */
			sound_id = existing_sound->sound_id;
		} else {
			/* Nope - Create a new node to store this sound's ID */
			sound_id = next_sound_id;
			new_sound = sound_node_new(cur_token, sound_id);
			TREE_INSERT(&sounds, _sound_node, sound_tree, new_sound);

			/* Add the new sound to the sound list and load it */
			if (grow_sound_list()) {
				sounds_list[sound_id].name = string_make(cur_token);

				if (preload_sounds)
					load_sound(&sounds_list[sound_id]);
			}

			next_sound_id++;
		}

		/* Add this sound (by id) to the message->sounds map */
		new_message_sound = mem_zalloc(sizeof *new_message_sound);
		new_message_sound->sound_id = sound_id;
		new_message_sound->next = message_map->first;
		message_map->first = new_message_sound;
		message_map->num_sounds++;

		/* Figure out next token */
		cur_token = next_token;

		if (next_token) {
			/* Try to find a space */
			search = strchr(cur_token, ' ');

			/* If we can find one, terminate, and set new "next" */
			if (search) {
				search[0] = '\0';
				next_token = search + 1;
			} else {
				/* Otherwise prevent infinite looping */
				next_token = NULL;
			}
		}
	}

	string_free(str);
}

/**
 * Play a sound of type "event".
 */
static void play_sound(game_event_type type, game_event_data *data, void *user)
{
	int s, i, sound_id;
	message_node message_search_node;
	message_node *message_map;
	msg_sound *message_sound;

	if (hooks.play_sound_hook) {

		/* Paranoia */
		assert(data->message.type >= 0);

		message_search_node.message_id = (u16b)(data->message.type);

		message_map = TREE_FIND(&messages, _message_node, message_tree, &message_search_node);

		if (!message_map)
			return; /* No sounds for this message */

		/* Choose a random event */
		message_sound = message_map->first;

		if (!message_sound)
			return;	/* Oops */

		s = randint0(message_map->num_sounds);

		/*
		 * Walk the list - possibly inefficient if you define LOTS of sounds
		 * for LOTS of messages. Not worth using a more 'efficient' data
		 * structure
		 */
		for (i = 0; i < s; i++)
			if (message_sound->next)
				message_sound = message_sound->next;

		sound_id = message_sound->sound_id;

		assert((sound_id >= 0) && (sound_id < next_sound_id));

		/* Ensure the sound is loaded before we play it */
		if (!sounds_list[sound_id].loaded)
			load_sound(&sounds_list[sound_id]);

		hooks.play_sound_hook(&sounds_list[sound_id]);
	}
}

/*
 * Shut down the sound system and free resources.
 */
static void close_audio(void)
{
	int i;

	if (0 == next_sound_id)
		return;	/* Never opened */

	/*
	 * Ask the platforms sound module to free resources for each
	 * sound
	 */
	if (hooks.unload_sound_hook)
		for (i = 0; i < next_sound_id; i++) {
			hooks.unload_sound_hook(&sounds_list[i]);
			string_free(sounds_list[i].name);
		}

	mem_free(sounds_list);

	/* Delete the 'sound names' tree */
	while (sounds.th_root) {
		mem_free(sounds.th_root->sound_name);
		TREE_REMOVE(&sounds, _sound_node, sound_tree, sounds.th_root);
	}

	/* Delete the 'message map' tree */
	while (messages.th_root) {
		delete_message_sounds(messages.th_root->first);
		TREE_REMOVE(&messages, _message_node, message_tree, messages.th_root);
	}

	/* Close the platform's sound module */
	if (hooks.close_audio_hook)
		hooks.close_audio_hook();
}


/**
 * Init the sound "module".
 */
errr init_sound(const char *soundstr, int argc, char **argv)
{
	int i = 0;
	bool done = false;

	/* Try the modules in the order specified by sound_modules[] */
	while (sound_modules[i].init && !done) {
		if (!soundstr || streq(soundstr, sound_modules[i].name))
			if (0 == sound_modules[i].init(&hooks, argc, argv))
				done = true;
		i++;
	}

	/* Check that we have a sound module to use */
	if (!done)
		return 1;

	/* Open the platform specific sound system */
	if (!hooks.open_audio_hook)
		return 1;

	if (!hooks.open_audio_hook(argc, argv))
		return 1;

	/* Enable sound */
	event_add_handler(EVENT_SOUND, play_sound, NULL);
	atexit(close_audio);

	/* Success */
	return (0);
}

/**
 * Print out the 'help' information for the sound module.
 */
void print_sound_help(void)
{
	int i;

	for (i = 0; i < (int)N_ELEMENTS(sound_modules); i++)
		printf("     %s   %s\n", sound_modules[i].name,
		       sound_modules[i].help);
}
