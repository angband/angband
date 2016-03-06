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
 * power: base power rating for the flag (0 means it is unused or derived)
 * rune text: the name of the flag's rune
 * message: what is printed when the flag is IDd 
 *  - must be "" or contain exactly one %s
 */
/* index       	id				type		power	rune text						message */
OF(PROT_FEAR,   OFID_NORMAL,	OFT_PROT,	6,		"protection from fear",			"Your %s strengthens your courage.")
OF(PROT_BLIND,  OFID_NORMAL,	OFT_PROT,	16,		"protection from blindness",	"Your %s soothes your eyes.")
OF(PROT_CONF,   OFID_NORMAL,	OFT_PROT,	24,		"protection from confusion",	"Your %s clears your thoughts.")
OF(PROT_STUN,   OFID_NORMAL,	OFT_PROT,	12,		"protection from stunning",		"Your %s steadies you.")
OF(SLOW_DIGEST, OFID_TIMED,		OFT_MISC,	2,		"slow digestion",				"You realise your %s is slowing your metabolism.")
OF(FEATHER,     OFID_NORMAL,	OFT_MISC,	1,		"feather falling",				"Your %s slows your fall.")
OF(REGEN,       OFID_TIMED,		OFT_MISC,	5,		"regeneration",					"You note that your %s is speeding up your recovery.")
OF(TELEPATHY,   OFID_WIELD,		OFT_MISC,	35,		"telepathy",					"")
OF(SEE_INVIS,   OFID_WIELD,		OFT_MISC,	6,		"see invisible",				"")
OF(FREE_ACT,    OFID_NORMAL,	OFT_MISC,	8,		"free action",					"Your %s keeps you moving.")
OF(HOLD_LIFE,   OFID_NORMAL,	OFT_MISC,	5,		"hold life",					"Your %s warms your spirit.")
OF(IMPACT,      OFID_NORMAL,	OFT_MELEE,	10,		"earthquakes",					"Your %s causes an earthquake!")
OF(BLESSED,     OFID_WIELD,		OFT_MELEE,	1,		"blessed melee",				"")
OF(BURNS_OUT,   OFID_WIELD,		OFT_LIGHT,	0,		"",								"")
OF(TAKES_FUEL,  OFID_WIELD,		OFT_LIGHT,	0,		"",								"")
OF(NO_FUEL,     OFID_WIELD,		OFT_LIGHT,	5,		"",								"")
OF(IMPAIR_HP,   OFID_TIMED,		OFT_BAD,	-9,		"impair hitpoint recovery",		"You feel your %s is slowing your recovery.")
OF(IMPAIR_MANA, OFID_TIMED,		OFT_BAD,	-9,		"impair mana recovery",			"You feel your %s is slowing your mana recovery.")
OF(AFRAID,      OFID_WIELD,		OFT_BAD,	-20,	"fear",							"")
OF(TELEPORT,    OFID_NORMAL,	OFT_BAD,	-20,	"random teleportation",			"Your %s teleports you.")
OF(AGGRAVATE,   OFID_TIMED,		OFT_BAD,	-20,	"aggravation",					"You notice your %s aggravating things around you.")
OF(DRAIN_EXP,   OFID_TIMED,		OFT_BAD,	-5,		"experience drain",				"You sense your %s is draining your life.")
OF(LIGHT_CURSE, OFID_WIELD,		OFT_CURSE,	-5,     "",								"")
OF(HEAVY_CURSE, OFID_WIELD,		OFT_CURSE,	-15,    "",								"")
OF(PERMA_CURSE, OFID_WIELD,		OFT_CURSE,	-25,    "",								"")
OF(MAX,			OFID_NONE,		OFT_NONE,	0,      "",								"")
