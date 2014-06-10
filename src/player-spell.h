/**
   \file player-spell.h
   \brief Spell and prayer casting/praying
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
/*
 * The defines below must match the spell numbers in spell.txt
 * if they don't, "interesting" things will probably happen.
 *
 * It would be nice if we could get rid of this dependency.
 */

enum spell_index_arcane_e {
	SPELL_MAGIC_MISSILE,
	SPELL_DETECT_MONSTERS,
	SPELL_PHASE_DOOR,
	SPELL_LIGHT_AREA,
	SPELL_FIND_TRAPS_DOORS,
	SPELL_CURE_LIGHT_WOUNDS,
	SPELL_OBJECT_DETECTION,
	SPELL_XXX,
	SPELL_IDENTIFY,
	SPELL_DETECT_INVISIBLE,
	SPELL_TREASURE_DETECTION,
	SPELL_STINKING_CLOUD,
	SPELL_LIGHTNING_BOLT,
	SPELL_CONFUSE_MONSTER,
	SPELL_SLEEP_MONSTER,
	SPELL_WONDER,
	SPELL_FROST_BOLT,
	SPELL_ACID_BOLT,
	SPELL_FIRE_BOLT,
	SPELL_TRAP_DOOR_DESTRUCTION,
	SPELL_SPEAR_OF_LIGHT,
	SPELL_TURN_STONE_TO_MUD,
	SPELL_DOOR_CREATION,
	SPELL_EARTHQUAKE,
	SPELL_STAIR_CREATION,
	SPELL_CURE_POISON,
	SPELL_SATISFY_HUNGER,
	SPELL_HEROISM,
	SPELL_BERSERKER,
	SPELL_HASTE_SELF,
	SPELL_TELEPORT_SELF,
	SPELL_SLOW_MONSTER,
	SPELL_TELEPORT_OTHER,
	SPELL_TELEPORT_LEVEL,
	SPELL_WORD_OF_RECALL,
	SPELL_POLYMORPH_OTHER,
	SPELL_SHOCK_WAVE,
	SPELL_EXPLOSION,
	SPELL_CLOUD_KILL,
	SPELL_MASS_SLEEP,
	SPELL_BEDLAM,
	SPELL_REND_SOUL,
	SPELL_WORD_OF_DESTRUCTION,
	SPELL_CHAOS_STRIKE,
	SPELL_RESIST_COLD,
	SPELL_RESIST_FIRE,
	SPELL_RESIST_POISON,
	SPELL_RESISTANCE,
	SPELL_SHIELD,
	SPELL_RUNE_OF_PROTECTION,
	SPELL_RECHARGE_ITEM_I,
	SPELL_ENCHANT_ARMOR,
	SPELL_ENCHANT_WEAPON,
	SPELL_RECHARGE_ITEM_II,
	SPELL_ELEMENTAL_BRAND,
	SPELL_FROST_BALL,
	SPELL_ACID_BALL,
	SPELL_FIRE_BALL,
	SPELL_ICE_STORM,
	SPELL_BANISHMENT,
	SPELL_METEOR_SWARM,
	SPELL_MASS_BANISHMENT,
	SPELL_RIFT,
	SPELL_MANA_STORM,

	SPELL_MAX,
};

enum spell_index_prayer_e {
	/* Beginners Handbook */
	PRAYER_DETECT_EVIL,
	PRAYER_CURE_LIGHT_WOUNDS,
	PRAYER_BLESS,
	PRAYER_REMOVE_FEAR,
	PRAYER_CALL_LIGHT,
	PRAYER_FIND_TRAPS_DOORS,
	PRAYER_XXX,
	PRAYER_SLOW_POISON,

	/* Words of Wisdom */
	PRAYER_SCARE_MONSTER,
	PRAYER_PORTAL,
	PRAYER_CURE_SERIOUS_WOUNDS,
	PRAYER_CHANT,
	PRAYER_SANCTUARY,
	PRAYER_SATISFY_HUNGER,
	PRAYER_REMOVE_CURSE,
	PRAYER_RESIST_HEAT_COLD,

	/* Chants and Blessings */
	PRAYER_NEUTRALIZE_POISON,
	PRAYER_ORB_OF_DRAINING,
	PRAYER_CURE_CRITICAL_WOUNDS,
	PRAYER_SENSE_INVISIBLE,
	PRAYER_PROTECTION_FROM_EVIL,
	PRAYER_EARTHQUAKE,
	PRAYER_SENSE_SURROUNDINGS,
	PRAYER_CURE_MORTAL_WOUNDS,
	PRAYER_TURN_UNDEAD,

	/* Exorcism and Dispelling */
	PRAYER_PRAYER,
	PRAYER_DISPEL_UNDEAD,
	PRAYER_HEAL,
	PRAYER_DISPEL_EVIL,
	PRAYER_GLYPH_OF_WARDING,
	PRAYER_HOLY_WORD,

	/* Godly Insights */
	PRAYER_DETECT_MONSTERS,
	PRAYER_DETECTION,
	PRAYER_PERCEPTION,
	PRAYER_PROBING,
	PRAYER_CLAIRVOYANCE,

	/* Purifications and Healing */
	PRAYER_CURE_SERIOUS_WOUNDS2,
	PRAYER_CURE_MORTAL_WOUNDS2,
	PRAYER_HEALING,
	PRAYER_RESTORATION,
	PRAYER_REMEMBRANCE,

	/* Wrath of God */
	PRAYER_DISPEL_UNDEAD2,
	PRAYER_DISPEL_EVIL2,
	PRAYER_BANISH_EVIL,
	PRAYER_WORD_OF_DESTRUCTION,
	PRAYER_ANNIHILATION,

	/* Holy Infusions */
	PRAYER_UNBARRING_WAYS,
	PRAYER_RECHARGING,
	PRAYER_DISPEL_CURSE,
	PRAYER_ENCHANT_WEAPON,
	PRAYER_ENCHANT_ARMOUR,
	PRAYER_ELEMENTAL_BRAND,

	/* Ethereal openings */
	PRAYER_BLINK,
	PRAYER_TELEPORT_SELF,
	PRAYER_TELEPORT_OTHER,
	PRAYER_TELEPORT_LEVEL,
	PRAYER_WORD_OF_RECALL,
	PRAYER_ALTER_REALITY,

	PRAYER_MAX,
};

typedef struct spell_handler_context_s {
	const int spell;
	const int dir;
	const int beam;
	const random_value value;
	const int p1, p2, p3;
} spell_handler_context_t;

typedef bool (*spell_handler_f)(spell_handler_context_t *);

typedef struct spell_info_s {
	u16b spell;
	bool aim;
	const char *info;
	spell_handler_f handler;
} spell_info_t;


extern int get_spell_index(const object_type *o_ptr, int index);
extern const char *get_spell_name(int tval, int index);
extern void get_spell_info(int tval, int index, char *buf, size_t len);
extern bool cast_spell(int tval, int index, int dir);
extern bool spell_needs_aim(int tval, int spell);
extern bool spell_is_identify(int book, int spell);
extern int spell_lookup_by_name(int tval, const char *name);
extern expression_base_value_f spell_value_base_by_name(const char *name);

