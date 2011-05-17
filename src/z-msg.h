#ifndef INCLUDED_Z_MSG_H
#define INCLUDED_Z_MSG_H

#include "h-basic.h"

/*** Constants ***/


/*** Message constants ***/

enum
{
	MSG_GENERIC       = 0,
	MSG_HIT           = 1,
	MSG_MISS          = 2,
	MSG_FLEE          = 3,
	MSG_DROP          = 4,
	MSG_KILL          = 5,
	MSG_LEVEL         = 6,
	MSG_DEATH         = 7,
	MSG_STUDY         = 8,
	MSG_TELEPORT      = 9,
	MSG_SHOOT         = 10,
	MSG_QUAFF         = 11,
	MSG_ZAP_ROD       = 12,
	MSG_WALK          = 13,
	MSG_TPOTHER       = 14,
	MSG_HITWALL       = 15,
	MSG_EAT           = 16,
	MSG_STORE1        = 17,
	MSG_STORE2        = 18,
	MSG_STORE3        = 19,
	MSG_STORE4        = 20,
	MSG_DIG           = 21,
	MSG_OPENDOOR      = 22,
	MSG_SHUTDOOR      = 23,
	MSG_TPLEVEL       = 24,
	MSG_BELL          = 25,
	MSG_NOTHING_TO_OPEN = 26,
	MSG_LOCKPICK_FAIL = 27,
	MSG_STAIRS_DOWN   = 28,
	MSG_HITPOINT_WARN = 29,
	MSG_ACT_ARTIFACT  = 30,
	MSG_USE_STAFF     = 31,
	MSG_DESTROY       = 32,
	MSG_MON_HIT       = 33,
	MSG_MON_TOUCH     = 34,
	MSG_MON_PUNCH     = 35,
	MSG_MON_KICK      = 36,
	MSG_MON_CLAW      = 37,
	MSG_MON_BITE      = 38,
	MSG_MON_STING     = 39,
	MSG_MON_BUTT      = 40,
	MSG_MON_CRUSH     = 41,
	MSG_MON_ENGULF    = 42,
	MSG_MON_CRAWL     = 43,
	MSG_MON_DROOL     = 44,
	MSG_MON_SPIT      = 45,
	MSG_MON_GAZE      = 46,
	MSG_MON_WAIL      = 47,
	MSG_MON_SPORE     = 48,
	MSG_MON_BEG       = 49,
	MSG_MON_INSULT    = 50,
	MSG_MON_MOAN      = 51,
	MSG_RECOVER       = 52,
	MSG_BLIND         = 53,
	MSG_CONFUSED      = 54,
	MSG_POISONED      = 55,
	MSG_AFRAID        = 56,
	MSG_PARALYZED     = 57,
	MSG_DRUGGED       = 58,
	MSG_SPEED         = 59,
	MSG_SLOW          = 60,
	MSG_SHIELD        = 61,
	MSG_BLESSED       = 62,
	MSG_HERO          = 63,
	MSG_BERSERK       = 64,
	MSG_BOLD          = 65,
	MSG_PROT_EVIL     = 66,
	MSG_INVULN        = 67,
	MSG_SEE_INVIS     = 68,
	MSG_INFRARED      = 69,
	MSG_RES_ACID      = 70,
	MSG_RES_ELEC      = 71,
	MSG_RES_FIRE      = 72,
	MSG_RES_COLD      = 73,
	MSG_RES_POIS      = 74,
	MSG_STUN          = 75,
	MSG_CUT           = 76,
	MSG_STAIRS_UP     = 77,
	MSG_STORE_ENTER   = 78,
	MSG_STORE_LEAVE   = 79,
	MSG_STORE_HOME    = 80,
	MSG_MONEY1        = 81,
	MSG_MONEY2        = 82,
	MSG_MONEY3        = 83,
	MSG_SHOOT_HIT     = 84,
	MSG_STORE5        = 85,
	MSG_LOCKPICK      = 86,
	MSG_DISARM        = 87,
	MSG_IDENT_BAD     = 88,
	MSG_IDENT_EGO     = 89,
	MSG_IDENT_ART     = 90,
	MSG_BR_ELEMENTS   = 91,
	MSG_BR_FROST      = 92,
	MSG_BR_ELEC       = 93,
	MSG_BR_ACID       = 94,
	MSG_BR_GAS        = 95,
	MSG_BR_FIRE       = 96,
	MSG_BR_CONF       = 97,
	MSG_BR_DISEN 	  = 98,
	MSG_BR_CHAOS      = 99,
	MSG_BR_SHARDS     = 100,
	MSG_BR_SOUND      = 101,
	MSG_BR_LIGHT      = 102,
	MSG_BR_DARK       = 103,
	MSG_BR_NETHER     = 104,
	MSG_BR_NEXUS      = 105,
	MSG_BR_TIME       = 106,
	MSG_BR_INERTIA    = 107,
	MSG_BR_GRAVITY    = 108,
	MSG_BR_PLASMA     = 109,
	MSG_BR_FORCE      = 110,
	MSG_SUM_MONSTER   = 111,
	MSG_SUM_ANGEL     = 112,
	MSG_SUM_UNDEAD    = 113,
	MSG_SUM_ANIMAL    = 114,
	MSG_SUM_SPIDER    = 115,
	MSG_SUM_HOUND     = 116,
	MSG_SUM_HYDRA     = 117,
	MSG_SUM_DEMON     = 118,
	MSG_SUM_DRAGON    = 119,
	MSG_SUM_HI_UNDEAD = 120,
	MSG_SUM_HI_DRAGON = 121,
	MSG_SUM_HI_DEMON  = 122,
	MSG_SUM_WRAITH    = 123,
	MSG_SUM_UNIQUE    = 124,
	MSG_WIELD         = 125,
	MSG_CURSED        = 126,
	MSG_PSEUDOID      = 127,
	MSG_HUNGRY        = 128,
	MSG_NOTICE        = 129,
	MSG_AMBIENT_DAY   = 130,
	MSG_AMBIENT_NITE  = 131,
	MSG_AMBIENT_DNG1  = 132,
	MSG_AMBIENT_DNG2  = 133,
	MSG_AMBIENT_DNG3  = 134,
	MSG_AMBIENT_DNG4  = 135,
	MSG_AMBIENT_DNG5  = 136,
	MSG_CREATE_TRAP   = 137,
	MSG_SHRIEK        = 138,
	MSG_CAST_FEAR     = 139,
	MSG_HIT_GOOD      = 140,
	MSG_HIT_GREAT     = 141,
	MSG_HIT_SUPERB    = 142,
	MSG_HIT_HI_GREAT  = 143,
	MSG_HIT_HI_SUPERB = 144,
	MSG_SPELL         = 145,
	MSG_PRAYER        = 146,
	MSG_KILL_UNIQUE   = 147,
	MSG_KILL_KING     = 148,
	MSG_DRAIN_STAT    = 149,
	MSG_MULTIPLY      = 150,

	MSG_MAX           = 151,
	SOUND_MAX         = MSG_MAX
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


#endif /* !INCLUDED_Z_MSG_H */
