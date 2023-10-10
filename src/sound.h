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
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef INCLUDED_SOUND_H
#define INCLUDED_SOUND_H

/*
 * Structure to held data relating to a sound.
 *  name :      Base name of the sound (no path or file extension)
 *
 *  hash :      Used to speed up searches
 *
 *  status:     If nothing has yet been done with the sound, this will be
 *              SOUND_ST_UNKNOWN.  If the sound file exists but the platform's
 *              sound module did not have enough information to be able to play
 *              the sound (that is up to the platform's module; it could mean
 *              that the sound has been loaded into memory or that the full
 *              file name has been stored in the platform's data), this will be
 *              SOUND_ST_ERROR.  Otherwise, this will be SOUND_ST_LOADED.
 *              The core sound module will check if status is SOUND_ST_LOADED
 *              before attempting to play it.

 *  plat_data : Platform specific structure used to store any additional
 *              data the platform's sound module needs in order to play the
 *              sound (and release resources when shut down)
 */

#define SOUND_PRF_FORMAT	"sound sym type str sounds"

enum sound_status {
	SOUND_ST_UNKNOWN = 0,
	SOUND_ST_ERROR,
	SOUND_ST_LOADED
};

struct parser;

struct sound_data {
	char *name;
	uint32_t hash;
	enum sound_status status;
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

bool set_preloaded_sounds(bool new_setting);
errr init_sound(const char *soundstr, int argc, char **argv);
void close_sound(void);
errr register_sound_pref_parser(struct parser *p);
void print_sound_help(void);

#endif /* !INCLUDED_SOUND_H */
