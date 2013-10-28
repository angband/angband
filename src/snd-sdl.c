/*
 * File: snd-sdl.c
 * Purpose: SDL sound support
 *
 * Copyright (c) 2004 Brendon Oliver <brendon.oliver@gmail.com>
 * Copyright (c) 2007 Andi Sidwell <andi@takkaria.org>
 * A large chunk of this file was taken and modified from main-ros.
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

 
#ifdef SOUND_SDL


#include "SDL.h"
#include "SDL_mixer.h"


/* Don't cache audio */
static bool no_cache_audio = FALSE;

/* Arbitary limit on number of samples per event */
#define MAX_SAMPLES      8

/* Struct representing all data about an event sample */
typedef struct
{
	int num;                        /* Number of samples for this event */
	Mix_Chunk *wavs[MAX_SAMPLES];   /* Sample array */
	char *paths[MAX_SAMPLES]; /* Relative pathnames for samples */
} sample_list;


/*
 * Just need an array of SampInfos
 */
static sample_list samples[MSG_MAX];


/*
 * Shut down the sound system and free resources.
 */
static void close_audio(void)
{
	size_t i;
	int j;

	/* Free all the sample data*/
	for (i = 0; i < MSG_MAX; i++)
	{
  		sample_list *smp = &samples[i];

		/* Nuke all samples */
		for (j = 0; j < smp->num; j++)
		{
			Mix_FreeChunk(smp->wavs[j]);
			string_free(smp->paths[j]);
		}
	}

	/* Close the audio */
	Mix_CloseAudio();

	/* XXX This may conflict with the SDL port */
	SDL_Quit();
}


/*
 * Initialise SDL and open the mixer
 */
static bool open_audio(void)
{
	int audio_rate;
	Uint16 audio_format;
	int audio_channels;
	
	/* Initialize variables */
	audio_rate = 22050;
	audio_format = AUDIO_S16;
	audio_channels = 2;

	/* Initialize the SDL library */
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		plog_fmt("Couldn't initialize SDL: %s", SDL_GetError());
		return FALSE;
	}

	/* Try to open the audio */
	if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, 4096) < 0)
	{
		plog_fmt("Couldn't open mixer: %s", SDL_GetError());
		return FALSE;
	}

	/* Success */
	return TRUE;
}



/*
 * Read sound.cfg and map events to sounds; then load all the sounds into
 * memory to avoid I/O latency later.
 */
static bool sound_sdl_init(bool no_cache)
{
	char path[2048];
	char buffer[2048];
	ang_file *fff;


	/* Initialise the mixer  */
	if (!open_audio())
	    return FALSE;


	/* Build the "sound" path */
	path_build(path, sizeof(path), ANGBAND_DIR_XTRA, "sound");
	if (ANGBAND_DIR_XTRA_SOUND)
		mem_free(ANGBAND_DIR_XTRA_SOUND);
	ANGBAND_DIR_XTRA_SOUND = string_make(path);

	/* Find and open the config file */
	path_build(path, sizeof(path), ANGBAND_DIR_XTRA_SOUND, "sound.cfg");
	fff = file_open(path, MODE_READ, -1);

	/* Handle errors */
	if (!fff)
	{
		plog_fmt("Failed to open sound config (%s):\n    %s", 
		          path, strerror(errno));
		return FALSE;
	}

	/* Parse the file */
	/* Lines are always of the form "name = sample [sample ...]" */
	while (file_getl(fff, buffer, sizeof(buffer)))
	{
		char *msg_name;
		char *sample_list;
		char *search;
		char *cur_token;
		char *next_token;
		int event;

		/* Skip anything not beginning with an alphabetic character */
		if (!buffer[0] || !isalpha((unsigned char)buffer[0])) continue;

		/* Split the line into two: message name, and the rest */
		search = strchr(buffer, ' ');
        sample_list = strchr(search + 1, ' ');
		if (!search) continue;
        if (!sample_list) continue;

		/* Set the message name, and terminate at first space */
		msg_name = buffer;
		search[0] = '\0';


		/* Make sure this is a valid event name */
		for (event = MSG_MAX - 1; event >= 0; event--)
		{
			if (strcmp(msg_name, angband_sound_name[event]) == 0)
			    break;
		}
        if (event < 0) continue;

		/* Advance the sample list pointer so it's at the beginning of text */
		sample_list++;
		if (!sample_list[0]) continue;

		/* Terminate the current token */
		cur_token = sample_list;
		search = strchr(cur_token, ' ');
		if (search)
		{
			search[0] = '\0';
			next_token = search + 1;
		}
		else
		{
			next_token = NULL;
		}

        /*
         * Now we find all the sample names and add them one by one
         */
        while (cur_token)
        {
            int num = samples[event].num;

			/* Don't allow too many samples */
			if (num >= MAX_SAMPLES) break;

			/* Build the path to the sample */
			path_build(path, sizeof(path), ANGBAND_DIR_XTRA_SOUND, cur_token);
			if (!file_exists(path)) goto next_token;

			/* Don't load now if we're not caching */
			if (no_cache)
			{
				/* Just save the path for later */
				samples[event].paths[num] = string_make(path);
			}
			else
			{
				/* Load the file now */
				samples[event].wavs[num] = Mix_LoadWAV(path);
				if (!samples[event].wavs[num])
				{
					plog_fmt("%s: %s", SDL_GetError(), strerror(errno));
					goto next_token;
				}
			}

			/* Imcrement the sample count */
			samples[event].num++;

		next_token:

			/* Figure out next token */
			cur_token = next_token;
			if (next_token)
			{
				/* Try to find a space */
				search = strchr(cur_token, ' ');

				/* If we can find one, terminate, and set new "next" */
				if (search)
				{
					search[0] = '\0';
					next_token = search + 1;
				}
				else
				{
					/* Otherwise prevent infinite looping */
					next_token = NULL;
				}
			}
		}
	}

	/* Close the file */
	file_close(fff);


	/* Success */
	return TRUE;
}

/*
 * Play a sound of type "event".
 */
static void play_sound(int event)
{
	Mix_Chunk *wave = NULL;
	int s;

	/* Paranoia */
	if (event < 0 || event >= MSG_MAX) return;

	/* Check there are samples for this event */
	if (!samples[event].num) return;

	/* Choose a random event */
	s = randint0(samples[event].num);
	wave = samples[event].wavs[s];

	/* Try loading it, if it's not cached */
	if (!wave)
	{
		/* Verify it exists */
		const char *filename = samples[event].paths[s];
		if (!file_exists(filename)) return;

		/* Load */
		wave = Mix_LoadWAV(filename);
	}

	/* Check to see if we have a wave again */
	if (!wave)
	{
		plog("SDL sound load failed.");
		return;
	}

	/* Actually play the thing */
	Mix_PlayChannel(-1, wave, 0);
}


/*
 * Init the SDL sound "module".
 */
errr init_sound_sdl(int argc, char **argv)
{
	int i;

	/* Parse args */
	for (i = 1; i < argc; i++)
	{
		if (prefix(argv[i], "-c"))
		{
			no_cache_audio = TRUE;
			plog("Audio cache disabled.");
			continue;
		}
	}

	/* Load sound preferences if requested */
	if (!sound_sdl_init(no_cache_audio))
	{
		plog("Failed to load sound config");

		/* Failure */
		return (1);
	}

	/* Enable sound */
	sound_hook = play_sound;
	atexit(close_audio);

	/* Success */
	return (0);
}


#endif /* SOUND_SDL */
