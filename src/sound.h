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
 * Structure to held data relation to a sound.
 * plat_data is platform specific structure used to store any additional
 * data the platform's sound module needs in order to play the sound (and
 * release resources when shut down)
 */
struct sound_data {
	char *name;
	u32b hash;
	bool loaded;
	void *plat_data;
};

struct sound_hooks
{
	bool (*open_audio_hook)(int argc, char **argv);
	bool (*close_audio_hook)(void);
	bool (*load_sound_hook)(struct sound_data *data);
	bool (*unload_sound_hook)(struct sound_data *data);
	bool (*play_sound_hook)(struct sound_data *data);
};

errr init_sound(const char *soundstr, int argc, char **argv);

void message_sound_define(u16b type, const char *sounds);

void print_sound_help(void);

#endif /* !INCLUDED_SOUND_H */
