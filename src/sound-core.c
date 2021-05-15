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
#include "ui-prefs.h"
#if defined(SOUND_SDL) || defined(SOUND_SDL2)
#include "snd-sdl.h"
#endif

#if (!defined(WIN32_CONSOLE_MODE) && defined(WINDOWS) && defined(SOUND) && !defined(USE_SDL) && !defined(USE_SDL2))
#include "snd-win.h"
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
#if defined(SOUND_SDL) || defined(SOUND_SDL2)
	{ "sdl", "SDL_mixer sound module", init_sound_sdl },
#endif /* SOUND_SDL */
#if (!defined(WIN32_CONSOLE_MODE) && defined(WINDOWS) && defined(SOUND) && !defined(USE_SDL) && !defined(USE_SDL2))
	{ "win", "Windows sound module", init_sound_win },
#endif

	{ "", "", NULL },
};

/*
 * After processing the preference files, 'sounds' will contain next_sound_id
 * entries - each representing a sound that needs to be loaded by calling
 * load_sound_hook() for each entry.
 */
static u16b next_sound_id;
static struct sound_data *sounds;

#define SOUND_DATA_ARRAY_INC	10

/* These are the hooks installed by the platform sound module */
static struct sound_hooks hooks;

/*
 * If preload_sounds is true, sounds are loaded immediately when assigned to
 * a message. Otherwise, each sound is only loaded when first played.
 */
static bool preload_sounds = false;

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
 * Iterate through all the sound types supporting by the platform's sound
 * module. Call the platform's sound modules 'load sound' function for each
 * supported file type until the platform's sound module tell us that it
 * could load the sound.
 * NOTE: The platform's sound module does not have to load the sound into
 * memory, it merely has to let us know that it can play the sound when
 * asked to.
 */
static void load_sound(struct sound_data *sound_data)
{
	if ((hooks.load_sound_hook) && (hooks.supported_files_hook)) {
		char path[2048];
		char *filename_buf;
		size_t filename_buf_size;
		int i = 0;
		bool load_success = false;

		const struct sound_file_type *supported_sound_files = hooks.supported_files_hook();

		/* Build the path to the sound file (minus extension) */
		path_build(path, sizeof(path), ANGBAND_DIR_SOUNDS, sound_data->name);

		/*
		 * Loop through all the extensions supported by the
		 * platform's sound module.
		 */
		while ((0 != supported_sound_files[i].type) && (!load_success)) {
			/*
			 * Create a buffer to store the filename plus extension
			 */
			filename_buf_size = strlen(path) + strlen(supported_sound_files[i].extension) + 1;
			filename_buf = mem_zalloc(filename_buf_size);
			my_strcpy(filename_buf, path, filename_buf_size);
			filename_buf = string_append(filename_buf, supported_sound_files[i].extension);

			if (file_exists(filename_buf))
				load_success = hooks.load_sound_hook(filename_buf, supported_sound_files[i].type, sound_data);

			mem_free(filename_buf);
			i++;
		}

		if (!load_success)
			plog_fmt("Failed to load sound '%s'", sound_data->name);
	}
}

/**
 * Parse a string of sound names provided by the preferences parser and:
 *  - Allocate a unique 'sound id' to any new sounds and add them to the
 *    'sounds' array.
 *  - Add each sound assigned to a message type to that message types
 *    'sound map
 */
void message_sound_define(u16b message_id, const char *sounds_str)
{
	char *search;
	char *str;
	char *cur_token;
	char *next_token;

	u16b sound_id = 0;

	u32b hash;
	int i;
	bool found = false;

	/* Delete any existing mapping of message id to sound ids */
	message_sounds[message_id].num_sounds = 0;

	/* sounds_str is a space separated list of sound names */
	str = cur_token = string_make(sounds_str);

	if (!cur_token) return;

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

enum parser_error parse_prefs_sound(struct parser *p)
{
	int msg_index;
	const char *type;
	const char *sounds_local;

	struct prefs_data *d = parser_priv(p);
	assert(d != NULL);
	if (d->bypass) return PARSE_ERROR_NONE;

	type = parser_getsym(p, "type");
	sounds_local = parser_getstr(p, "sounds");

	msg_index = message_lookup_by_name(type);

	if (msg_index < 0)
		return PARSE_ERROR_INVALID_MESSAGE;

	message_sound_define(msg_index, sounds_local);

	return PARSE_ERROR_NONE;
}

errr register_sound_pref_parser(struct parser *p)
{
#ifdef SOUND
	return parser_reg(p, SOUND_PRF_FORMAT, parse_prefs_sound);
#else
	return parser_reg(p, SOUND_PRF_FORMAT, parse_prefs_dummy);
#endif
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

		if (!message_sounds[data->message.type].num_sounds)
			return; /* No sounds for this message */

		s = randint0(message_sounds[data->message.type].num_sounds);

		sound_id = message_sounds[data->message.type].sound_ids[s];

		assert((sound_id >= 0) && (sound_id < next_sound_id));

		/* Ensure the sound is loaded before we play it */
		if (!sounds[sound_id].loaded)
			load_sound(&sounds[sound_id]);

		/* Only bother playing it if the platform can */
		if (sounds[sound_id].loaded)
			hooks.play_sound_hook(&sounds[sound_id]);
	}
}


/**
 * Init the sound "module".
 */
errr init_sound(const char *soundstr, int argc, char **argv)
{
	int i = 0;
	bool done = false;

	/* Release resources previously allocated if called multiple times. */
	close_sound();

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

	if (!hooks.open_audio_hook())
		return 1;

	/* Enable sound */
	event_add_handler(EVENT_SOUND, play_sound, NULL);

	/* Success */
	return (0);
}

/**
 * Shut down the sound "module".
 */
void close_sound(void)
{
	if (0 == next_sound_id) return;	/* Never opened */

	/*
	 * Ask the platforms sound module to free resources for each
	 * sound
	 */
	if (hooks.unload_sound_hook) {
		int i;

		for (i = 0; i < next_sound_id; i++) {
			hooks.unload_sound_hook(&sounds[i]);
			string_free(sounds[i].name);
		}
	}

	mem_free(sounds);
	sounds = NULL;
	next_sound_id = 0;

	/* Close the platform's sound module */
	if (hooks.close_audio_hook) {
		hooks.close_audio_hook();
	}
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
