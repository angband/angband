/**
 * \file message.h
 * \brief Message handling
 *
 * Copyright (c) 2007 Elly, Andi Sidwell
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

#ifndef INCLUDED_Z_MSG_H
#define INCLUDED_Z_MSG_H

#include "h-basic.h"

/**
 * Message constants
 */
enum {
	#define MSG(x, s) MSG_##x,
	#include "list-message.h"
	#undef MSG
	SOUND_MAX = MSG_MAX,
};


/* Functions */
void messages_init(void);
void messages_free(void);
u16b messages_num(void);
void message_add(const char *str, u16b type);
const char *message_str(u16b age);
u16b message_count(u16b age);
u16b message_type(u16b age);
byte message_color(u16b age);
byte message_type_color(u16b type);
void message_color_define(u16b type, byte color);
int message_lookup_by_name(const char *name);
int message_lookup_by_sound_name(const char *name);
const char *message_sound_name(int message);
void sound(int type);
void bell(void);
void msg(const char *fmt, ...);
void msgt(unsigned int type, const char *fmt, ...);


#endif /* !INCLUDED_Z_MSG_H */
