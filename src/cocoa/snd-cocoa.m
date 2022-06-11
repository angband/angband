/**
 * \file snd-cocoa.m
 * \brief Sound handling for the Cocoa frontend on macOS
 *
 * Copyright (c) 2022 Eric Branlund
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
#if defined(MACH_O_CARBON) && defined(SOUND) && !defined(SOUND_SDL) && !defined(SOUND_SDL2)
#include "sound.h"
#include "snd-cocoa.h"
#import <Cocoa/Cocoa.h>


/*
 * Values for type field in sound_file_type and sample_type field in
 * cocoa_sample
 */
enum {
	COCOA_SOUND_NULL = 0, /* placeholder expected by core sound */
	COCOA_SOUND_BRIDGED_NSSOUND /* use NSSound to handle load and play */
};

/* Holds the platform-specific data for a sound */
struct cocoa_sample {
	union {
		void *bridged_nssound; /* for COCOA_SOUND_BRIDGED_NSSOUND */
	} sample_data;
	int sample_type;
};

/*
 * Supported file types (according to
 * https://wiki.lazarus.freepascal.org/macOS_NSSound there are more but mp3
 * and wav are likely the most useful for Angband)
 */
static const struct sound_file_type supported_sound_formats[] = {
	{ ".mp3", COCOA_SOUND_BRIDGED_NSSOUND },
	{ ".wav", COCOA_SOUND_BRIDGED_NSSOUND },
	{ "", COCOA_SOUND_NULL }
};

/**
 * Load a sound.
 */
static bool load_sound_cocoa(const char *filename, int file_type, struct sound_data *data)
{
	struct cocoa_sample *sample = (struct cocoa_sample*) (data->plat_data);

	if (!sample) {
		sample = mem_zalloc(sizeof(*sample));
	}

	switch (file_type) {
	case COCOA_SOUND_BRIDGED_NSSOUND:
		{
			NSSound *sound = [[NSSound alloc]
				initWithContentsOfFile:[NSString stringWithUTF8String:filename]
				byReference:YES];

			if (sound) {
				sample->sample_data.bridged_nssound =
					(void*) CFBridgingRetain(sound);
				sample->sample_type =
					COCOA_SOUND_BRIDGED_NSSOUND;
			} else {
				/*
				 * Failed to initialize.  Mark it as unplayable.
				 */
				sample->sample_type = COCOA_SOUND_NULL;
			}
		}
		break;

	default:
		/*
		 * Either COCOA_SOUND_NULL or something unrecognized.  In
		 * either case, flag this as unplayable.
		 */
		sample->sample_type = COCOA_SOUND_NULL;
		break;
	}

	data->plat_data = (void*)sample;
	if (sample->sample_type != COCOA_SOUND_NULL) {
		data->status = SOUND_ST_LOADED;
		return true;
	}
	return false;
}

/**
 * Play a sound.
 */
static bool play_sound_cocoa(struct sound_data *data)
{
	struct cocoa_sample *sample = (struct cocoa_sample*) (data->plat_data);

	if (sample) {
		switch (sample->sample_type) {
		case COCOA_SOUND_BRIDGED_NSSOUND:
			{
				NSSound *sound = (__bridge NSSound*) (sample->sample_data.bridged_nssound);

				if (sound) {
					if ([sound isPlaying]) {
						[sound stop];
					}
					[sound play];
				}
			}
			break;

		default:
			/*
			 * Either COCOA_SOUND_NULL or something unrecognized.
			 * In either case, do nothing.
			 */
			break;
		}
	}

	return true;
}

/**
 * Unload a sound.
 */
static bool unload_sound_cocoa(struct sound_data *data)
{
	struct cocoa_sample *sample = (struct cocoa_sample*) (data->plat_data);

	if (sample) {
		switch (sample->sample_type) {
		case COCOA_SOUND_BRIDGED_NSSOUND:
			{
				NSSound *sound = (__bridge NSSound*) (sample->sample_data.bridged_nssound);
				if (sound) {
					/*
					 * Balance the CFBridgingRetain() from
					 * when the sound was loaded.
					 */
					CFRelease(sample->sample_data.bridged_nssound);
				}
			}
			break;

		default:
			/*
			 * Either COCOA_SOUND_NULL or something unrecognized.
			 * Do nothing.
			 */
			break;
		}
		mem_free(sample);
		data->plat_data = NULL;
	}
	data->status = SOUND_ST_UNKNOWN;
	return true;
}

/**
 * Initialize sound support (dummy; nothing needs to be done).
 */
static bool open_audio_cocoa(void)
{
	return true;
}

/**
 * Clean up sound support (dummy; nothing needs to be done).
 */
static bool close_audio_cocoa(void)
{
	return true;
}

/**
 * Return the supported sound formats.
 */
static const struct sound_file_type *supported_files_cocoa(void)
{
	return supported_sound_formats;
}

/**
 * Hook Cocoa-specific sound support into the core sound handling.
 */
errr init_sound_cocoa(struct sound_hooks *hooks, int argc, char **argv)
{
	/*
	 * Load all the sounds when sound.prf is parsed to mimic the past
	 * behavior of the Mac client and avoid lag when a sound is first
	 * played.
	 */
	(void) set_preloaded_sounds(true);
	hooks->open_audio_hook = open_audio_cocoa;
	hooks->close_audio_hook = close_audio_cocoa;
	hooks->load_sound_hook = load_sound_cocoa;
	hooks->unload_sound_hook = unload_sound_cocoa;
	hooks->play_sound_hook = play_sound_cocoa;
	hooks->supported_files_hook = supported_files_cocoa;
	return 0;
}

#endif
