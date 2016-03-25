/**
 * \file snd-sdl.c
 * \brief SDL sound support
 *
 * Copyright (c) 2004 Brendon Oliver <brendon.oliver@gmail.com>
 * Copyright (c) 2007 Andi Sidwell <andi@takkaria.org>
 * Copyright (c) 2016 Graeme Russ <graeme.russ@gmail.com>
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
#include "init.h"
#include "sound.h"

#ifdef SOUND_SDL


#include "SDL.h"
#include "SDL_mixer.h"

typedef enum sdl_sample_type {
	SDL_CHUNK,
	SDL_MUSIC,
	SDL_NULL
} sdl_sample_type;

/**
 * Struct representing all data about an event sample
 */
typedef struct
{
	union {
		Mix_Chunk *chunk;	/* Sample in WAVE format */
		Mix_Music *music;	/* Sample in MP3 format */
	} sample_data;

	sdl_sample_type sample_type;
	char *path;			/* Relative pathnames for samples */
} sdl_sample;

typedef struct sdl_file_type {
	const char *extension;
	sdl_sample_type type;
} sdl_file_type_t;

/* List of supported file types */
static const sdl_file_type_t supported_file_types[] = { {".mp3", SDL_MUSIC},
							{".ogg", SDL_CHUNK},
							{"", SDL_NULL} };
/**
 * Initialise SDL and open the mixer.
 */
static bool open_audio_sdl(int argc, char **argv)
{
	int audio_rate;
	Uint16 audio_format;
	int audio_channels;

	/* Initialize variables */
	audio_rate = 22050;
	audio_format = AUDIO_S16;
	audio_channels = 2;

	/* Initialize the SDL library */
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		plog_fmt("SDL: Couldn't initialize SDL: %s", SDL_GetError());
		return false;
	}

	/* Try to open the audio */
	if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, 4096) < 0) {
		plog_fmt("SDL: Couldn't open mixer: %s", SDL_GetError());
		return false;
	}

	/* Success */
	return true;
}

/**
 * Load a sound from file.
 */
static bool load_sample_sdl(sdl_sample *sample)
{
	switch (sample->sample_type) {
		case SDL_CHUNK:
			sample->sample_data.chunk = Mix_LoadWAV(sample->path);

			if (sample->sample_data.chunk)
				return true;

			break;

		case SDL_MUSIC:
			sample->sample_data.music = Mix_LoadMUS(sample->path);

			if (sample->sample_data.music)
				return true;

			break;

		default:
			plog_fmt("SDL: Oops - Unsupported file type");
			break;
	}

	return false;
}

/**
 * Load a sound and return a pointer to the associated SDL Sound data
 * structure back to the core sound module.
 */
static bool load_sound_sdl(const char *sound_name, void **data)
{
	char path[2048];
	char *filename_buf;
	size_t filename_buf_size;
	sdl_sample *sample = NULL;
	int i = 0;
	bool loaded = false;

	/* Build the path to the sample */
	path_build(path, sizeof(path), ANGBAND_DIR_SOUNDS, sound_name);

	/*
	 * Create a buffer to store the filename plus three character
	 * extension (5 = '.' + 3 character extension + '\0'
	 */
	filename_buf_size = strlen(path) + 5;
	filename_buf = mem_zalloc(filename_buf_size);

	while ((SDL_NULL != supported_file_types[i].type) && (!loaded)) {
		my_strcpy(filename_buf, path, filename_buf_size);
		filename_buf = string_append(filename_buf,
					     supported_file_types[i].extension);

		if (file_exists(filename_buf)) {
			if (!sample)
				sample = mem_zalloc(sizeof(*sample));

			if (sample) {
				sample->sample_type = supported_file_types[i].type;
				sample->path = string_make(filename_buf);

				/* Try and load the sample file */
				if (!load_sample_sdl(sample))
					mem_free(sample->path);
				else
					loaded = true;

			} else {
				/* Out of memory */
				mem_free(filename_buf);
				data = NULL;
				return false;
			}
		}

		i++;
	}

	mem_free(filename_buf);

	if (!loaded) {
		plog_fmt("SDL: Failed to load sound '%s')", sound_name);
		mem_free(sample);
		sample = NULL;
	}

	*data = (void *)sample;

	return (NULL != sample);
}

/**
 * Play the sound stored in the provided SDL Sound data structure.
 */
static bool play_sound_sdl(void *data)
{
	sdl_sample *sample = (sdl_sample *)data;

	if (sample) {
		switch (sample->sample_type) {
			case SDL_CHUNK:
				if (sample->sample_data.chunk)
					return (0 == Mix_PlayChannel(-1, sample->sample_data.chunk, 0));
				break;

			case SDL_MUSIC:
				if (sample->sample_data.music)
					return (0 == Mix_PlayMusic(sample->sample_data.music, 1));
				break;

			default:
				break;
		}
	}

	return false;
}

/**
 * Free resources referenced in the provided SDL Sound data structure.
 */
static bool unload_sound_sdl(void *data)
{
	sdl_sample *sample = (sdl_sample *)data;

	if (sample) {
		switch (sample->sample_type) {
			case SDL_CHUNK:
				if (sample->sample_data.chunk)
					 Mix_FreeChunk(sample->sample_data.chunk);

				break;

			case SDL_MUSIC:
				if (sample->sample_data.music)
					Mix_FreeMusic(sample->sample_data.music);

				break;

			default:
				break;
		}

		if (sample->path)
			mem_free(sample->path);

		mem_free(sample);
	}

	return true;
}

/**
 * Shut down the SDL sound module and free resources.
 */
static bool close_audio_sdl(void)
{
	/*
	 * Close the audio.
	 *
	 * NOTE: All samples will have been free'd by the sound subsystem
	 * calling unload_sound_sdl() for every sample that was loaded.
	 */
	Mix_CloseAudio();

	/* XXX This may conflict with the SDL port */
	SDL_Quit();

	return true;
}

/**
 * Init the SDL sound module.
 */
errr init_sound_sdl(struct sound_hooks *hooks, int argc, char **argv)
{
	hooks->open_audio_hook = open_audio_sdl;
	hooks->close_audio_hook = close_audio_sdl;
	hooks->load_sound_hook = load_sound_sdl;
	hooks->unload_sound_hook = unload_sound_sdl;
	hooks->play_sound_hook = play_sound_sdl;

	/* Success */
	return (0);
}


#endif /* SOUND_SDL */
