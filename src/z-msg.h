#ifndef INCLUDED_Z_MSG_H
#define INCLUDED_Z_MSG_H

#include "h-basic.h"

/*** Constants ***/


/*** Message constants ***/

enum {
	#define MSG(x, s) MSG_##x,
	#include "z-msg-list.h"
	#undef MSG
	SOUND_MAX = MSG_MAX,
};


/*** Functions ***/

/** Initialisation/exit **/

/**
 * Initialise the messages package.  Should be called before using any other
 * functions in the package.
 */
errr messages_init(void);

/**
 * Free the message package.
 */
void messages_free(void);


/** General info **/

/**
 * Return the current number of messages stored.
 */
u16b messages_num(void);	


/** Individual message handling **/

/**
 * Save a new message into the memory buffer, with text `str` and type `type`.
 * The type should be one of the MSG_ constants defined above.
 *
 * The new message may not be saved if it is identical to the one saved before
 * it, in which case the "count" of the message will be increased instead.
 * This count can be fetched using the message_count() function.
 */
void message_add(const char *str, u16b type);


/**
 * Returns the text of the message of age `age`.  The age of the most recently
 * saved message is 0, the one before that is of age 1, etc.
 *
 * Returns the empty string if the no messages of the age specified are
 * available.
 */
const char *message_str(u16b age);

/**
 * Returns the number of times the message of age `age` was saved. The age of
 * the most recently saved message is 0, the one before that is of age 1, etc.
 *
 * In other words, if message_add() was called five times, one after the other,
 * with the message "The orc sets your hair on fire.", then the text will only
 * have one age (age = 0), but will have a count of 5.
 */
u16b message_count(u16b age);

/**
 * Returns the type of the message of age `age`.  The age of the most recently
 * saved message is 0, the one before that is of age 1, etc.
 *
 * The type is one of the MSG_ constants, defined above.
 */
u16b message_type(u16b age);

/**
 * Returns the display colour of the message memorised `age` messages ago.
 * (i.e. age = 0 represents the last memorised message, age = 1 is the one
 * before that, etc).
 */
byte message_color(u16b age);


/** Message type changes **/

/**
 * Returns the colour for the message type `type`.
 */ 
byte message_type_color(u16b type);

/**
 * Defines the color `color` for the message type `type`.
 */
void message_color_define(u16b type, byte color);

/**
 * Return the MSG_ flag that matches the given string. This does not handle SOUND_MAX.
 *
 * \param name is a string that contains the name of a flag or a number.
 * \return The MSG_ flag that matches the given name.
 */
int message_lookup_by_name(const char *name);

/**
 * Return the MSG_ flag that matches the given sound event name.
 *
 * \param name is the sound name from sound.cfg.
 * \return The MSG_ flag for the corresponding sound.
 */
int message_lookup_by_sound_name(const char *name);

/**
 * Return the sound name for the given message.
 *
 * \param message is the MSG_ flag to find.
 * \return The sound.cfg sound name.
 */
const char *message_sound_name(int message);

#endif /* !INCLUDED_Z_MSG_H */
