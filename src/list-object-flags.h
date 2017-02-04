/**
 * \file list-object-flags.h
 * \brief object flags for all objects
 *
 * Changing flag order will break savefiles. Flags
 * below start from 0 on line 17, so a flag's sequence number is its line
 * number minus 17.
 *
 * index: the flag number
 * id: when the flag is IDd
 * type: what type of flag is it?
 * rune text: the name of the flag's rune
 * message: what is printed when the flag is IDd 
 *  - must be "" or contain exactly one %s
 */
/* index       	id				type		rune text						message */
OF(PROT_FEAR,   OFID_NORMAL,	OFT_PROT,	"protection from fear",			"Your %s strengthens your courage.")
OF(PROT_BLIND,  OFID_NORMAL,	OFT_PROT,	"protection from blindness",	"Your %s soothes your eyes.")
OF(PROT_CONF,   OFID_NORMAL,	OFT_PROT,	"protection from confusion",	"Your %s clears your thoughts.")
OF(PROT_STUN,   OFID_NORMAL,	OFT_PROT,	"protection from stunning",		"Your %s steadies you.")
OF(SLOW_DIGEST, OFID_TIMED,		OFT_MISC,	"slow digestion",				"You realise your %s is slowing your metabolism.")
OF(FEATHER,     OFID_NORMAL,	OFT_MISC,	"feather falling",				"Your %s slows your fall.")
OF(REGEN,       OFID_TIMED,		OFT_MISC,	"regeneration",					"You note that your %s is speeding up your recovery.")
OF(TELEPATHY,   OFID_WIELD,		OFT_MISC,	"telepathy",					"")
OF(SEE_INVIS,   OFID_WIELD,		OFT_MISC,	"see invisible",				"")
OF(FREE_ACT,    OFID_NORMAL,	OFT_MISC,	"free action",					"Your %s keeps you moving.")
OF(HOLD_LIFE,   OFID_NORMAL,	OFT_MISC,	"hold life",					"Your %s warms your spirit.")
OF(IMPACT,      OFID_NORMAL,	OFT_MELEE,	"earthquakes",					"Your %s causes an earthquake!")
OF(BLESSED,		OFID_WIELD,		OFT_MELEE,	"blessed melee",				"")
OF(BURNS_OUT,	OFID_WIELD,		OFT_LIGHT,	"",								"")
OF(TAKES_FUEL,	OFID_WIELD,		OFT_LIGHT,	"",								"")
OF(NO_FUEL,		OFID_WIELD,		OFT_LIGHT,	"",								"")
OF(IMPAIR_HP,	OFID_TIMED,		OFT_BAD,	"impaired hitpoint recovery",	"You feel your %s is slowing your recovery.")
OF(IMPAIR_MANA, OFID_TIMED,		OFT_BAD,	"impaired mana recovery",		"You feel your %s is slowing your mana recovery.")
OF(AFRAID,      OFID_WIELD,		OFT_BAD,	"constant fear",				"")
OF(NO_TELEPORT, OFID_NORMAL,	OFT_BAD,	"teleportation ban",			"Your %s prevents you teleporting.")
OF(AGGRAVATE,   OFID_TIMED,		OFT_BAD,	"aggravation",					"You notice your %s aggravating things around you.")
OF(DRAIN_EXP,   OFID_TIMED,		OFT_BAD,	"experience drain",				"You sense your %s is draining your life.")
OF(STICKY, 		OFID_WIELD,		OFT_BAD,	"stuck on",						"")
OF(FRAGILE,		OFID_WIELD,		OFT_BAD,	"fragile",						"")
OF(LIGHT_1,		OFID_WIELD,		OFT_LIGHT,	"",								"")
OF(LIGHT_2,		OFID_WIELD,		OFT_LIGHT,	"",								"")
OF(DIG_1,		OFID_WIELD,		OFT_DIG,	"",								"")
OF(DIG_2,		OFID_WIELD,		OFT_DIG,	"",								"")
OF(DIG_3,		OFID_WIELD,		OFT_DIG,	"",								"")
OF(EXPLODE,		OFID_WIELD,		OFT_LIGHT,	"",								"")
OF(MAX,			OFID_NONE,		OFT_NONE,	"",								"")
