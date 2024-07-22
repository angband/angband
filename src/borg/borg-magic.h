/**
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2007-9 Andi Sidwell, Chris Carr, Ed Graham, Erik Osheim
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband License":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
#ifndef INCLUDED_BORG_MAGIC_H
#define INCLUDED_BORG_MAGIC_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

/*
 * Spell method values
 */

#define BORG_MAGIC_ICK 0 /* Spell is illegible */
#define BORG_MAGIC_NOP 1 /* Spell takes no arguments */
#define BORG_MAGIC_EXT 2 /* Spell has DETECTION effects */
#define BORG_MAGIC_AIM 3 /* Spell requires a direction */
#define BORG_MAGIC_OBJ 4 /* Spell requires a pack object */
#define BORG_MAGIC_WHO 5 /* Spell requires a monster symbol */

/*
 * Spell status values
 */

#define BORG_MAGIC_ICKY 0 /* Spell is illegible */
#define BORG_MAGIC_LOST 1 /* Spell is forgotten */
#define BORG_MAGIC_HIGH 2 /* Spell is high level */
#define BORG_MAGIC_OKAY 3 /* Spell is learnable */
#define BORG_MAGIC_TEST 4 /* Spell is untried */
#define BORG_MAGIC_KNOW 5 /* Spell is known */

enum borg_spells {
    MAGIC_MISSILE,
    LIGHT_ROOM,
    FIND_TRAPS_DOORS_STAIRS,
    PHASE_DOOR,
    ELECTRIC_ARC,
    DETECT_MONSTERS,
    FIRE_BALL,
    RECHARGING,
    IDENTIFY_RUNE,
    TREASURE_DETECTION,
    FROST_BOLT,
    REVEAL_MONSTERS,
    ACID_SPRAY,
    DISABLE_TRAPS_DESTROY_DOORS,
    TELEPORT_SELF,
    TELEPORT_OTHER,
    RESISTANCE,
    TAP_MAGICAL_ENERGY,
    MANA_CHANNEL,
    DOOR_CREATION,
    MANA_BOLT,
    TELEPORT_LEVEL,
    DETECTION,
    DIMENSION_DOOR,
    THRUST_AWAY,
    SHOCK_WAVE,
    EXPLOSION,
    BANISHMENT,
    MASS_BANISHMENT,
    MANA_STORM,
    DETECT_LIFE,
    FOX_FORM,
    REMOVE_HUNGER,
    STINKING_CLOUD,
    CONFUSE_MONSTER,
    SLOW_MONSTER,
    CURE_POISON,
    RESIST_POISON,
    TURN_STONE_TO_MUD,
    SENSE_SURROUNDINGS,
    LIGHTNING_STRIKE,
    EARTH_RISING,
    TRANCE,
    MASS_SLEEP,
    BECOME_PUKEL_MAN,
    EAGLES_FLIGHT,
    BEAR_FORM,
    TREMOR,
    HASTE_SELF,
    REVITALIZE,
    RAPID_REGENERATION,
    HERBAL_CURING,
    METEOR_SWARM,
    RIFT,
    ICE_STORM,
    VOLCANIC_ERUPTION,
    RIVER_OF_LIGHTNING,
    CALL_LIGHT,
    DETECT_EVIL,
    MINOR_HEALING,
    BLESS,
    SENSE_INVISIBLE,
    HEROISM,
    ORB_OF_DRAINING,
    SPEAR_OF_LIGHT,
    DISPEL_UNDEAD,
    DISPEL_EVIL,
    PROTECTION_FROM_EVIL,
    REMOVE_CURSE,
    PORTAL,
    REMEMBRANCE,
    WORD_OF_RECALL,
    HEALING,
    RESTORATION,
    CLAIRVOYANCE,
    ENCHANT_WEAPON,
    ENCHANT_ARMOUR,
    SMITE_EVIL,
    GLYPH_OF_WARDING,
    DEMON_BANE,
    BANISH_EVIL,
    WORD_OF_DESTRUCTION,
    HOLY_WORD,
    SPEAR_OF_OROME,
    LIGHT_OF_MANWE,
    NETHER_BOLT,
    CREATE_DARKNESS,
    BAT_FORM,
    READ_MINDS,
    TAP_UNLIFE,
    CRUSH,
    SLEEP_EVIL,
    SHADOW_SHIFT,
    DISENCHANT,
    FRIGHTEN,
    VAMPIRE_STRIKE,
    DISPEL_LIFE,
    DARK_SPEAR,
    WARG_FORM,
    BANISH_SPIRITS,
    ANNIHILATE,
    GRONDS_BLOW,
    UNLEASH_CHAOS,
    FUME_OF_MORDOR,
    STORM_OF_DARKNESS,
    POWER_SACRIFICE,
    ZONE_OF_UNMAGIC,
    VAMPIRE_FORM,
    CURSE,
    COMMAND,
    SINGLE_COMBAT,
    OBJECT_DETECTION,
    DETECT_STAIRS,
    HIT_AND_RUN,
    COVER_TRACKS,
    CREATE_ARROWS,
    DECOY,
    BRAND_AMMUNITION,
    SEEK_BATTLE,
    BERSERK_STRENGTH,
    WHIRLWIND_ATTACK,
    SHATTER_STONE,
    LEAP_INTO_BATTLE,
    GRIM_PURPOSE,
    MAIM_FOE,
    HOWL_OF_THE_DAMNED,
    RELENTLESS_TAUNTING,
    VENOM,
    WEREWOLF_FORM,
    BLOODLUST,
    UNHOLY_REPRIEVE,
    FORCEFUL_BLOW,
    QUAKE
};

/*
 * The borgs "usefulness" rating of each spell.
 */
typedef struct borg_spell_rating borg_spell_rating;
struct borg_spell_rating {
    const char      *name; /* Textual name */
    uint8_t          rating; /* Usefulness */
    enum borg_spells spell_enum; /* an enum for quick lookup */
};

/*
 * Forward declare
 */
typedef struct borg_magic borg_magic;

/*
 * A spell/prayer in a book
 */
struct borg_magic {
    const char *name; /* Textual name */
    uint8_t     status; /* Status (see above) */
    uint16_t    effect_index; /* effect index */
    uint8_t     rating; /* Usefulness */
    uint8_t     level; /* Required level */
    uint8_t     power; /* Required power */
    uint8_t     sfail; /* Minimum chance of failure */
    int         book_offset; /* offset of this spell in the book it is in */
    int         book; /* book index */
    int32_t     times; /* Times this spell was cast */
    enum borg_spells spell_enum;
};

/*
 * Spell casting information
 */

extern borg_magic *borg_magics; /* Spell info */

/*
 * Spell functions
 */

/*
 * Does this player cast spells
 */
extern bool borg_can_cast(void);

/*
 * Does this player mostly cast spells
 */
extern bool borg_primarily_caster(void);

/*
 * get the level at which Heroism grants Heroism
 */
extern int borg_heroism_level(void);

/*
 * get the stat used for casting spells
 */
extern int borg_spell_stat(void);

/*
 * Determine if borg can cast a given spell (when fully rested)
 */
extern bool borg_spell_legal(const enum borg_spells spell);

/*
 * Determine if borg can cast a given spell (right now)
 */
extern bool borg_spell_okay(const enum borg_spells spell);

/*
 * Find the power (cost in sp) value for a given spell
 */
extern int borg_get_spell_power(const enum borg_spells spell);

/*
 * find the index in the books array given the books sval
 */
extern int borg_get_book_num(int sval);

/*
 * is this a dungeon book (not a basic book)
 */
extern bool borg_is_dungeon_book(int tval, int sval);

/*
 * Find the magic structure given a book/entry
 */
extern borg_magic *borg_get_spell_entry(int book, int what);

/*
 * Attempt to cast a spell
 */
extern bool borg_spell(const enum borg_spells spell);

/*
 * Attempt to cast a spell if not above fail rate
 */
extern bool borg_spell_fail(const enum borg_spells spell, int allow_fail);

/*
 * Check if a spell can be cast now and is below fail rate
 */
extern bool borg_spell_okay_fail(const enum borg_spells spell, int allow_fail);

/*
 * Check if a spell is known and below fail rate
 */
extern bool borg_spell_legal_fail(const enum borg_spells spell, int allow_fail);

/*
 * Find the fail rate for a spell
 */
extern int borg_spell_fail_rate(const enum borg_spells spell);

/*
 * Initialize the book information
 */
extern void borg_prepare_book_info(void);

/*
 * Cheat/Parse the "spell" screen
 */
extern void borg_cheat_spell(int book);

#endif

#endif
