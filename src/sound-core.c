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
#include "main.h"
#ifdef SOUND_SDL
#include "snd-sdl.h"
#endif

#define MAX_SOUNDS_PER_MESSAGE	16

struct sound_module
{
	const char *name;
	const char *help;
	errr (*init)(struct sound_hooks *hooks, int argc, char **argv);
};

struct msg_snd_data
{
	u16b num_sounds;
	u16b sound_ids[MAX_SOUNDS_PER_MESSAGE];
};

/*
 * Not the most efficient use of memory, but brutally efficient runtime.
 * At time of writing, MSG_MAX ~= 150. sizeof(struct msg_snd_data) = 34,
 * so we are using ~5kB of memory.
 */
static struct msg_snd_data message_sounds[MSG_MAX];

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
 * After processing the preference files, the 'sounds' will contain
 * next_sound_id entries - each representing a sound that needs to be loaded
 * by calling load_sound_hook() for each entry.
 */
static u16b next_sound_id;
static struct sound_data *sounds;

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

static struct sound_data *grow_sound_list(void)
{
	int new_size;
	int i;

	if (!sounds) {
		sounds = mem_zalloc(sizeof(struct sound_data) * SOUND_DATA_ARRAY_INC);
	} else if (0 == (next_sound_id % SOUND_DATA_ARRAY_INC)) {
		/* Add a new chunk onto the sounds list */
		new_size = ((next_sound_id / SOUND_DATA_ARRAY_INC) + 1) * SOUND_DATA_ARRAY_INC;
		sounds = mem_realloc(sounds, sizeof(struct sound_data) * new_size);

		if (sounds)
			/* Clear the new elements */
			for (i = next_sound_id; i < (next_sound_id + SOUND_DATA_ARRAY_INC); i++)
				memset(&sounds[i], 0x00, sizeof(struct sound_data));
	}

	return sounds;
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

	u16b sound_id;

/*	Fnv32_t hash; */
	u32b hash;
	int i;
	bool found = false;

	/* Delete any existing mapping of message id to sound ids */
	message_sounds[message_id].num_sounds = 0;

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
		found = false;

		/* Have we already processed this sound name? */
		hash = djb2_hash(cur_token);

		i = 0;

		while ((!found) && (i < next_sound_id)) {
			if (sounds[i].hash == hash) {
				if (!strcmp(sounds[i].name, cur_token)) {
					found = true;
					sound_id = i;
				}
			}

			i++;
		}

		if (!found) {
			sound_id = next_sound_id;

			/* Add the new sound to the sound list and load it */
			if (grow_sound_list()) {
				sounds[sound_id].name = string_make(cur_token);
				sounds[sound_id].hash = hash;

				if (preload_sounds)
					load_sound(&sounds[sound_id]);
			}

			next_sound_id++;
		}

		/* Add this sound (by id) to the message->sounds map */
		if (message_sounds[message_id].num_sounds < (MAX_SOUNDS_PER_MESSAGE - 1)) {
			message_sounds[message_id].sound_ids[message_sounds[message_id].num_sounds] = sound_id;
			message_sounds[message_id].num_sounds++;
		}

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
	int s, sound_id;

	if (hooks.play_sound_hook) {

		/* Paranoia */
		assert(data->message.type >= 0);

		if (!message_sounds[type].num_sounds)
			return; /* No sounds for this message */

		s = randint0(message_sounds[data->message.type].num_sounds);

		sound_id = message_sounds[data->message.type].sound_ids[s];

		assert((sound_id >= 0) && (sound_id < next_sound_id));

		/* Ensure the sound is loaded before we play it */
		if (!sounds[sound_id].loaded)
			load_sound(&sounds[sound_id]);

		hooks.play_sound_hook(&sounds[sound_id]);
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
			hooks.unload_sound_hook(&sounds[i]);
			string_free(sounds[i].name);
		}

	mem_free(sounds);

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
