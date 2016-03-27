/**
 * \file list-effects.h
 * \brief List of effects
 *
 * name: effect code
 * aim: does the effect require aiming?
 * info: info label for spells
 * args: how many arguments the description takes
 * info flags: flags for object description
 * description: text of description
 */
/* name 							aim		info		args	info flags		description */
EFFECT(RANDOM,						false,	NULL,		0,		EFINFO_NONE,	"randomly ")
EFFECT(DAMAGE,						false,	NULL,		0,		EFINFO_NONE,	"damages the player")
EFFECT(HEAL_HP,						false,	"heal",		2,		EFINFO_HEAL,	"heals %s hitpoints%s")
EFFECT(MON_HEAL_HP,					false,	NULL,		0,		EFINFO_NONE,	"heals monster hitpoints")
EFFECT(NOURISH,						false,	NULL,		1,		EFINFO_CONST,	"feeds you for %d turns")
EFFECT(CRUNCH,						false,	NULL,		0,		EFINFO_NONE,	"crunches")
EFFECT(CURE,						false,	NULL,		1,		EFINFO_CURE,	"cures %s")
EFFECT(TIMED_SET,					false,	NULL,		2,		EFINFO_TIMED,	"administers %s for %s turns")
EFFECT(TIMED_INC,					false,	"dur",		2,		EFINFO_TIMED,	"extends %s for %s turns")
EFFECT(TIMED_INC_NO_RES,			false,	"dur",		2,		EFINFO_TIMED,	"extends %s for %s turns (unresistable)")
EFFECT(MON_TIMED_INC,				false,	NULL,		2,		EFINFO_TIMED,	"increases monster %s by %s turns")
EFFECT(TIMED_DEC,					false,	NULL,		2,		EFINFO_TIMED,	"reduces length of %s by %s turns")
EFFECT(SET_NOURISH,					false,	NULL,		1,		EFINFO_CONST,	"leaves you nourished for %d turns")
EFFECT(CONFUSING,					false,	NULL,		0,		EFINFO_NONE,	"causes your next attack upon a monster to confuse it")
EFFECT(RUNE,						false,	NULL,		0,		EFINFO_NONE,	"inscribes a glyph of warding beneath you, which monsters cannot move onto")
EFFECT(RESTORE_STAT,				false,	NULL,		1,		EFINFO_STAT,	"restores your %s")
EFFECT(DRAIN_STAT,					false,	NULL,		1,		EFINFO_STAT,	"reduces your %s")
EFFECT(LOSE_RANDOM_STAT,			false,	NULL,		1,		EFINFO_STAT,	"reduces a stat other than %s")
EFFECT(GAIN_STAT,					false,	NULL,		1,		EFINFO_STAT,	"increases your %s")
EFFECT(RESTORE_EXP,					false,	NULL,		0,		EFINFO_NONE,	"restores your experience")
EFFECT(GAIN_EXP,					false,	NULL,		1,		EFINFO_CONST,	"grants %d experience points")
EFFECT(LOSE_EXP,					false,	NULL,		0,		EFINFO_NONE,	"loses a quarter of your experience")
EFFECT(DRAIN_MANA,					false,	NULL,		0,		EFINFO_NONE,	"drains some mana")
EFFECT(RESTORE_MANA,				false,	NULL,		0,		EFINFO_NONE,	"restores some mana")
EFFECT(REMOVE_CURSE,				false,	NULL,		0,		EFINFO_NONE,	"removes curses")
EFFECT(REMOVE_ALL_CURSE,			false,	NULL,		0,		EFINFO_NONE,	"removes all curses")
EFFECT(RECALL,						false,	NULL,		0,		EFINFO_NONE,	"returns you from the dungeon or takes you to the dungeon after a short delay")
EFFECT(DEEP_DESCENT,				false,	NULL,		0,		EFINFO_NONE,	"teleports you up to five dungeon levels lower than the lowest point you have reached so far")
EFFECT(ALTER_REALITY,				false,	NULL,		0,		EFINFO_NONE,	"creates a new dungeon level")
EFFECT(MAP_AREA,					false,	NULL,		0,		EFINFO_NONE,	"maps the area around you")
EFFECT(DETECT_DOORS,				false,	NULL,		0,		EFINFO_NONE,	"detects doors nearby")
EFFECT(DETECT_STAIRS,				false,	NULL,		0,		EFINFO_NONE,	"detects stairs nearby")
EFFECT(DETECT_GOLD,					false,	NULL,		0,		EFINFO_NONE,	"detects gold nearby")
EFFECT(SENSE_OBJECTS,				false,	NULL,		0,		EFINFO_NONE,	"senses objects nearby")
EFFECT(DETECT_OBJECTS,				false,	NULL,		0,		EFINFO_NONE,	"detects objects nearby")
EFFECT(DETECT_VISIBLE_MONSTERS,		false,	NULL,		0,		EFINFO_NONE,	"detects visible creatures nearby")
EFFECT(DETECT_INVISIBLE_MONSTERS,	false,	NULL,		0,		EFINFO_NONE,	"detects invisible creatures nearby")
EFFECT(DETECT_EVIL,					false,	NULL,		0,		EFINFO_NONE,	"detects evil creatures nearby")
EFFECT(CREATE_STAIRS,				false,	NULL,		0,		EFINFO_NONE,	"creates a staircase beneath your feet")
EFFECT(DISENCHANT,					false,	NULL,		0,		EFINFO_NONE,	"disenchants one of your wielded items")
EFFECT(ENCHANT,						false,	NULL,		0,		EFINFO_NONE,	"attempts to magically enhance an item")
EFFECT(IDENTIFY,					false,	NULL,		0,		EFINFO_NONE,	"reveals to you the extent of an item's magical powers")
EFFECT(IDENTIFY_PACK,				false,	NULL,		0,		EFINFO_NONE,	"identifies all items in your pack")
EFFECT(RECHARGE,					false,	NULL,		0,		EFINFO_NONE,	"tries to recharge a wand or staff, destroying the wand or staff on failure")
EFFECT(PROJECT_LOS,					false,	"power",	1,		EFINFO_SEEN,	"%s which are in line of sight")
EFFECT(PROJECT_LOS_AWARE,			false,	"power",	1,		EFINFO_SEEN,	"%s which are in line of sight")
EFFECT(ACQUIRE,						false,	NULL,		0,		EFINFO_NONE,	"creates good items nearby")
EFFECT(AGGRAVATE,					false,	NULL,		0,		EFINFO_NONE,	"awakens all nearby sleeping monsters and hastens all monsters within line of sight")
EFFECT(SUMMON,						false,	NULL,		1,		EFINFO_SUMM,	"summons %s at the current dungeon level")
EFFECT(BANISH,						false,	NULL,		0,		EFINFO_NONE,	"removes all of a given creature type from the level")
EFFECT(MASS_BANISH,					false,	NULL,		0,		EFINFO_NONE,	"removes all nearby creatures")
EFFECT(PROBE,						false,	NULL,		0,		EFINFO_NONE,	"gives you information on the health and abilities of monsters you can see")
EFFECT(THRUST_AWAY,					false,	NULL,		0,		EFINFO_NONE,	"pushes a creature away")
EFFECT(TELEPORT,					false,	"range",	2,		EFINFO_TELE,	"teleports %s randomly %d grids")
EFFECT(TELEPORT_TO,					false,	NULL,		0,		EFINFO_NONE,	"teleports you to a monster")
EFFECT(TELEPORT_LEVEL,				false,	NULL,		0,		EFINFO_NONE,	"teleports you one level up or down")
EFFECT(DESTRUCTION,					false,	NULL,		1,		EFINFO_QUAKE,	"destroys an area around you in the shape of a circle radius %d, and blinds you for 1d10+10 turns")
EFFECT(EARTHQUAKE,					false,	NULL,		1,		EFINFO_QUAKE,	"causes an earthquake around you of radius %d")
EFFECT(ENLIGHTENMENT,				false,	NULL,		0,		EFINFO_NONE,	"completely lights up and magically maps the level")
EFFECT(LIGHT_AREA,					false,	NULL,		2,		EFINFO_LIGHT,	"lights up the surrounding area, causing %s damage to light-sensitive creatures within %d grids")
EFFECT(DARKEN_AREA,					false,	NULL,		0,		EFINFO_NONE,	"darkens the surrounding area")
EFFECT(BALL,						true,	"dam",		3,		EFINFO_BALL,	"fires a ball of %s with radius %d, dealing %s damage at the centre")
EFFECT(BREATH,						true,	NULL,		3,		EFINFO_BALL,	"breathes a cone of %s with width %d degrees, dealing %s damage at the source")
EFFECT(SWARM,						true,	"dam",		3,		EFINFO_BALL,	"fires a series of %s balls of radius %d, dealing %s damage at the centre of each")
EFFECT(STAR,						false,	"dam",		2,		EFINFO_BOLTD,	"fires a line of %s in all directions, each dealing %s damage")
EFFECT(STAR_BALL,					false,	"dam",		3,		EFINFO_BALL,	"fires balls of %s with radius %d in all directions, dealing %s damage at the centre of each")
EFFECT(BOLT,						true,	"dam",		2,		EFINFO_BOLTD,	"casts a bolt of %s dealing %s damage")
EFFECT(BEAM,						true,	"dam",		2,		EFINFO_BOLTD,	"casts a beam of %s dealing %s damage")
EFFECT(BOLT_OR_BEAM,				true,	"dam",		2,		EFINFO_BOLTD,	"casts a bolt or beam of %s dealing %s damage")
EFFECT(LINE,						true,	"dam",		2,		EFINFO_BOLTD,	"creates a line of %s dealing %s damage")
EFFECT(ALTER,						true,	NULL,		0,		EFINFO_NONE,	"creates a line which may affect terrain and/or items")
EFFECT(BOLT_STATUS,					true,	NULL,		1,		EFINFO_BOLT,	"casts a bolt which %s")
EFFECT(BOLT_STATUS_DAM,				true,	"dam",		2,		EFINFO_BOLTD,	"casts a bolt which %s, dealing %s damage")
EFFECT(BOLT_AWARE,					true,	"dam",		1,		EFINFO_BOLT,	"creates a bolt which %s")
EFFECT(TOUCH,						false,	NULL,		1,		EFINFO_TOUCH,	"%s on all adjacent squares")
EFFECT(TOUCH_AWARE,					false,	NULL,		1,		EFINFO_TOUCH,	"%s on all adjacent squares")
EFFECT(CURSE_ARMOR,					false,	NULL,		0,		EFINFO_NONE,	"curses your worn armor")
EFFECT(CURSE_WEAPON,				false,	NULL,		0,		EFINFO_NONE,	"curses your wielded melee weapon")
EFFECT(BRAND_WEAPON,				false,	NULL,		0,		EFINFO_NONE,	"brands your wielded melee weapon")
EFFECT(BRAND_AMMO,					false,	NULL,		0,		EFINFO_NONE,	"brands a stack of ammunition")
EFFECT(BRAND_BOLTS,					false,	NULL,		0,		EFINFO_NONE,	"brands bolts with fire, in an unbalanced fashion")
EFFECT(BIZARRE,						true,	NULL,		0,		EFINFO_NONE,	"does bizarre things")
EFFECT(WONDER,						true,	NULL,		0,		EFINFO_NONE,	"creates random and unpredictable effects")
EFFECT(TRAP_DOOR,					false,	NULL,		0,		EFINFO_NONE,	"a trap door which drops you down a level")
EFFECT(TRAP_PIT,					false,	NULL,		0,		EFINFO_NONE,	"a pit trap - the fall might hurt")
EFFECT(TRAP_PIT_SPIKES,				false,	NULL,		0,		EFINFO_NONE,	"a pit trap, with nasty spikes")
EFFECT(TRAP_PIT_POISON,				false,	NULL,		0,		EFINFO_NONE,	"a pit trap, with poisoned spikes")
EFFECT(TRAP_RUNE_SUMMON,			false,	NULL,		0,		EFINFO_NONE,	"a rune which summons monsters")
EFFECT(TRAP_RUNE_TELEPORT,			false,	NULL,		0,		EFINFO_NONE,	"a rune which teleports")
EFFECT(TRAP_SPOT_FIRE,				false,	NULL,		0,		EFINFO_NONE,	"a magical fire trap")
EFFECT(TRAP_SPOT_ACID,				false,	NULL,		0,		EFINFO_NONE,	"a magical acid trap")
EFFECT(TRAP_DART_SLOW,				false,	NULL,		0,		EFINFO_NONE,	"a dart which slows movements")
EFFECT(TRAP_DART_LOSE_STR,			false,	NULL,		0,		EFINFO_NONE,	"a dart which drains strength")
EFFECT(TRAP_DART_LOSE_DEX,			false,	NULL,		0,		EFINFO_NONE,	"a dart which drains dexterity")
EFFECT(TRAP_DART_LOSE_CON,			false,	NULL,		0,		EFINFO_NONE,	"a dart which drains constitution")
EFFECT(TRAP_GAS_BLIND,				false,	NULL,		0,		EFINFO_NONE,	"blinding gas")
EFFECT(TRAP_GAS_CONFUSE,			false,	NULL,		0,		EFINFO_NONE,	"confusing gas")
EFFECT(TRAP_GAS_POISON,				false,	NULL,		0,		EFINFO_NONE,	"poison gas")
EFFECT(TRAP_GAS_SLEEP,				false,	NULL,		0,		EFINFO_NONE,	"soporific gas")
