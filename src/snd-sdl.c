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
#include "snd-sdl.h"
#include "sound.h"

#if defined(SOUND_SDL) || defined(SOUND_SDL2)
#include "SDL.h"
#include "SDL_mixer.h"
#if defined(SOUND_SDL2)
#include "SDL_revision.h"
#endif
#endif

/**
 * Struct representing all data about an event sample
 */
typedef struct
{
	union {
		Mix_Chunk *chunk;	/* Sample in WAVE format */
		Mix_Music *music;	/* Sample in MP3 format */
	} sample_data;

	int sample_type;
} sdl_sample;

/* Supported file types */
enum {
	SDL_NULL = 0,
	SDL_CHUNK,
	SDL_MUSIC
};

static const struct sound_file_type supported_sound_files[] = { {".mp3", SDL_MUSIC},
								{".ogg", SDL_CHUNK},
								{"", SDL_NULL} };

#ifdef SOUND_SDL2
static bool print_sdl_details = false;
#endif

/**
 * Initialise SDL and open the mixer.
 */
static bool open_audio_sdl(void)
{
	/* Initialize variables */
	int audio_rate = 22050;
	Uint16 audio_format = AUDIO_S16;
	int audio_channels = 2;

	/* Initialize the SDL library */
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		plog_fmt("SDL: Couldn't initialize SDL: %s", SDL_GetError());
		return false;
	}

	/* Try to open the audio */
	if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, 4096) < 0) {
		plog_fmt("SDL: Couldn't open mixer: %s", SDL_GetError());
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return false;
	}

#ifdef SOUND_SDL2
	if (print_sdl_details) {
		const SDL_version *pv;
		const char *driver_name;
		SDL_version lv;
		int freq, n_chan;
		Uint16 fmt;

		SDL_Log("SDL audio: Runtime SDL library revision: %s",
			SDL_GetRevision());
		pv = Mix_Linked_Version();
		SDL_MIXER_VERSION(&lv);
		SDL_Log("SDL audio: SDL_mixer library version: "
			"%u.%u.%u (runtime) %u.%u.%u (compiled)",
			pv->major, pv->minor, pv->patch,
			lv.major, lv.minor, lv.patch);
		driver_name = SDL_GetCurrentAudioDriver();
		SDL_Log("SDL audio: Current driver: %s",
			(driver_name) ? driver_name : "Not initialized");
		if (Mix_QuerySpec(&freq, &fmt, &n_chan)) {
			SDL_Log("SDL audio: Mixer channels, frequency, and "
				"format: %d %d %lu", n_chan, freq,
				(unsigned long)fmt);
		} else {
			SDL_Log("SDL audio: Mixer channels, frequency, and"
				"format: %s", Mix_GetError());
		}
	}
#endif

	/* Success */
	return true;
}

/**
 * Load a sound from file.
 */
static bool load_sample_sdl(const char *filename, int ft, sdl_sample *sample)
{
	switch (ft) {
		case SDL_CHUNK:
			sample->sample_data.chunk = Mix_LoadWAV(filename);

			if (sample->sample_data.chunk)
				return true;

			break;

		case SDL_MUSIC:
			sample->sample_data.music = Mix_LoadMUS(filename);

			if (sample->sample_data.music)
				return true;

			break;

		default:
			plog("SDL: Oops - Unsupported file type");
			break;
	}

	return false;
}

/**
 * Load a sound and return a pointer to the associated SDL Sound data
 * structure back to the core sound module.
 */
static bool load_sound_sdl(const char *filename, int ft, struct sound_data *data)
{
	sdl_sample *sample = (sdl_sample *)(data->plat_data);

	if (!sample)
		sample = mem_zalloc(sizeof(*sample));

	/* Try and load the sample file */
	if (load_sample_sdl(filename, ft, sample)) {
		data->status = SOUND_ST_LOADED;
		sample->sample_type = ft;
	} else {
		mem_free(sample);
		sample = NULL;
	}

	data->plat_data = (void *)sample;

	return (NULL != sample);
}

/**
 * Play the sound stored in the provided SDL Sound data structure.
 */
static bool play_sound_sdl(struct sound_data *data)
{
	sdl_sample *sample = (sdl_sample *)(data->plat_data);

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
static bool unload_sound_sdl(struct sound_data *data)
{
	sdl_sample *sample = (sdl_sample *)(data->plat_data);

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

		mem_free(sample);
		data->plat_data = NULL;
		data->status = SOUND_ST_UNKNOWN;
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
	SDL_QuitSubSystem(SDL_INIT_AUDIO);

	return true;
}

static const struct sound_file_type *supported_files_sdl(void)
{
	return supported_sound_files;
}

/**
 * Init the SDL sound module.
 */
errr init_sound_sdl(struct sound_hooks *hooks, int argc, char **argv)
{
	hooks->open_audio_hook = open_audio_sdl;
	hooks->supported_files_hook = supported_files_sdl;
	hooks->close_audio_hook = close_audio_sdl;
	hooks->load_sound_hook = load_sound_sdl;
	hooks->unload_sound_hook = unload_sound_sdl;
	hooks->play_sound_hook = play_sound_sdl;

#ifdef SOUND_SDL2
	{
		int i;

		for (i = 1; i < argc; ++i) {
			if (streq(argv[i], "-v")) {
				print_sdl_details = true;
			}
		}

		if (print_sdl_details) {
			SDL_version vr, vc;

			SDL_GetVersion(&vr);
			SDL_VERSION(&vc);
			SDL_Log("SDL audio: SDL library version: "
				"%u.%u.%u (runtime) %u.%u.%u (compiled; %s)",
				vr.major, vr.minor, vr.patch,
				vc.major, vc.minor, vc.patch, SDL_REVISION);
		}
	}
#endif

	/* Success */
	return (0);
}
