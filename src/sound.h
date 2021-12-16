/**
 * \file sound.h
 * \brief Sound handling
 *
 * Copyright (c) 2016 Graeme Russ
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 */

#ifndef INCLUDED_SOUND_H
#define INCLUDED_SOUND_H

/*
 * Structure to held data relating to a sound.
 *  name :      Base name of the sound (no path or file extension)
 *
 *  hash :      Used to speed up searches
 *
 *  loaded :    The platform's sound module sets this flag if it has enough
 *              information to play the sound (this may mean the sound data
 *              is loaded into memory, or that the full filename has been
 *              stored in the platform data. It is up to the platform's
 *              sound module to determine what 'loaded' means. The core
 *              sound module uses this flag to check if the sound needs to
 *              be 'loaded' before attempting to play it.

 *  plat_data : Platform specific structure used to store any additional
 *              data the platform's sound module needs in order to play the
 *              sound (and release resources when shut down)
 */

#define SOUND_PRF_FORMAT	"sound sym type str sounds"

struct sound_data {
	char *name;
	uint32_t hash;
	bool loaded;
	void *plat_data;
};

struct sound_file_type {
	const char *extension;
	int type;
};

struct sound_hooks
{
	bool (*open_audio_hook)(void);
	bool (*close_audio_hook)(void);
	bool (*load_sound_hook)(const char *filename, int file_type, struct sound_data *data);
	bool (*unload_sound_hook)(struct sound_data *data);
	bool (*play_sound_hook)(struct sound_data *data);
	const struct sound_file_type *(*supported_files_hook)(void);
};

errr init_sound(const char *soundstr, int argc, char **argv);
void close_sound(void);
errr register_sound_pref_parser(struct parser *p);
void print_sound_help(void);

#endif /* !INCLUDED_SOUND_H */
