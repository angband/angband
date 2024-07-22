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
#ifndef INCLUDED_BORG_TRAIT_H
#define INCLUDED_BORG_TRAIT_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

#include "borg-item.h"
#include "borg-trait-swap.h"

/*
 * Possible values of "goal"
 */
#define GOAL_KILL    1 /* Monsters */
#define GOAL_TAKE    2 /* Objects */
#define GOAL_MISC    3 /* Stores */
#define GOAL_DARK    4 /* Exploring */
#define GOAL_XTRA    5 /* Searching */
#define GOAL_BORE    6 /* Leaving */
#define GOAL_FLEE    7 /* Fleeing */
#define GOAL_VAULT   8 /* Vaults */
#define GOAL_RECOVER 9 /* Resting safely */
#define GOAL_DIGGING 10 /* Anti-summon Corridor */

/*
 * Player race constants (hard-coded by save-files, arrays, etc)
 */
#define RACE_HUMAN      0
#define RACE_HALF_ELF   1
#define RACE_ELF        2
#define RACE_HOBBIT     3
#define RACE_GNOME      4
#define RACE_DWARF      5
#define RACE_HALF_ORC   6
#define RACE_HALF_TROLL 7
#define RACE_DUNADAN    8
#define RACE_HIGH_ELF   9
#define RACE_KOBOLD     10
#define MAX_RACES       11

enum borg_item_pos { BORG_INVEN = 1, BORG_EQUIP = 2, BORG_QUILL = 4 };

/* NOTE: This must exactly match the prefix_pref enums in borg-trait.c */
enum {
    BI_STR,
    BI_INT,
    BI_WIS,
    BI_DEX,
    BI_CON,
    BI_ASTR,
    BI_AINT,
    BI_AWIS,
    BI_ADEX,
    BI_ACON,
    BI_CSTR,
    BI_CINT,
    BI_CWIS,
    BI_CDEX,
    BI_CCON,
    BI_SSTR,
    BI_SINT,
    BI_SWIS,
    BI_SDEX,
    BI_SCON,
    BI_CLASS,
    BI_LIGHT,
    BI_CURHP,
    BI_MAXHP,
    BI_HP_ADJ,
    BI_CURSP,
    BI_MAXSP,
    BI_SP_ADJ,
    BI_FAIL1,
    BI_FAIL2,
    BI_CLEVEL,
    BI_MAXCLEVEL,
    BI_ESP,
    BI_CURLITE,
    BI_RECALL,
    BI_FOOD,
    BI_FOOD_HI,
    BI_FOOD_LO,
    BI_FOOD_CURE_CONF,
    BI_FOOD_CURE_BLIND,
    BI_SPEED,
    BI_GOLD,
    BI_MOD_MOVES,
    BI_DAM_RED,
    BI_SDIG,
    BI_FEATH,
    BI_REG,
    BI_SINV,
    BI_INFRA,
    BI_FAST_SHOTS,
    BI_DISP,
    BI_DISM,
    BI_DEV,
    BI_SAV,
    BI_STL,
    BI_SRCH,
    BI_THN,
    BI_THB,
    BI_THT,
    BI_DIG,
    BI_IFIRE,
    BI_IACID,
    BI_ICOLD,
    BI_IELEC,
    BI_IPOIS,
    BI_RFIRE,
    BI_RCOLD,
    BI_RELEC,
    BI_RACID,
    BI_RPOIS,
    BI_RFEAR,
    BI_RLITE,
    BI_RDARK,
    BI_RBLIND,
    BI_RCONF,
    BI_RSND,
    BI_RSHRD,
    BI_RNXUS,
    BI_RNTHR,
    BI_RKAOS,
    BI_RDIS,
    BI_HLIFE,
    BI_FRACT,
    BI_SRFIRE,
    BI_SRCOLD,
    BI_SRELEC,
    BI_SRACID,
    BI_SRPOIS,
    BI_SRFEAR,
    BI_SRLITE,
    BI_SRDARK,
    BI_SRBLIND,
    BI_SRCONF,
    BI_SRSND,
    BI_SRSHRD,
    BI_SRNXUS,
    BI_SRNTHR,
    BI_SRKAOS,
    BI_SRDIS,
    BI_SHLIFE,
    BI_SFRACT,

    BI_CDEPTH,
    BI_MAXDEPTH,
    BI_KING,

    BI_ISWEAK,
    BI_ISHUNGRY,
    BI_ISFULL,
    BI_ISGORGED,
    BI_ISBLIND,
    BI_ISAFRAID,
    BI_ISCONFUSED,
    BI_ISPOISONED,
    BI_ISCUT,
    BI_ISSTUN,
    BI_ISHEAVYSTUN,
    BI_ISPARALYZED,
    BI_ISIMAGE,
    BI_ISFORGET,
    BI_ISENCUMB,
    BI_ISSTUDY,
    BI_ISFIXLEV,
    BI_ISFIXEXP,
    BI_HASFIXEXP,
    BI_ISFIXSTR,
    BI_ISFIXINT,
    BI_ISFIXWIS,
    BI_ISFIXDEX,
    BI_ISFIXCON,
    BI_ISFIXALL,

    BI_ARMOR,
    BI_TOHIT,
    BI_TODAM,
    BI_WTOHIT,
    BI_WTODAM,
    BI_WID,
    BI_WDD,
    BI_WDS,
    BI_BID,
    BI_BTOHIT,
    BI_BTODAM,
    BI_SLING,
    BI_BART,
    BI_BLOWS,
    BI_EXTRA_BLOWS,
    BI_SHOTS,
    BI_HEAVYWEPON,
    BI_HEAVYBOW,
    BI_AMMO_COUNT,
    BI_AMMO_TVAL,
    BI_AMMO_SIDES,
    BI_AMMO_POWER,
    BI_AMISSILES,
    BI_AMISSILES_SPECIAL,
    BI_AMISSILES_CURSED,
    BI_QUIVER_SLOTS,
    BI_FIRST_CURSED,
    BI_WHERE_CURSED,

    BI_CRSENVELOPING,
    BI_CRSIRRITATION,
    BI_CRSTELE,
    BI_CRSPOIS,
    BI_CRSSIREN,
    BI_CRSHALU,
    BI_CRSPARA,
    BI_CRSSDEM,
    BI_CRSSDRA,
    BI_CRSSUND,
    BI_CRSSTONE,
    BI_CRSNOTEL,
    BI_CRSTWEP,
    BI_CRSAGRV,
    BI_CRSHPIMP,
    BI_CRSMPIMP,
    BI_CRSSTEELSKIN,
    BI_CRSAIRSWING,
    BI_CRSFEAR,
    BI_CRSDRAIN_XP,
    BI_CRSFVULN,
    BI_CRSEVULN,
    BI_CRSCVULN,
    BI_CRSAVULN,
    BI_CRSUNKNO,

    BI_WS_ANIMAL,
    BI_WS_EVIL,
    BI_WS_UNDEAD,
    BI_WS_DEMON,
    BI_WS_ORC,
    BI_WS_TROLL,
    BI_WS_GIANT,
    BI_WS_DRAGON,
    BI_WK_UNDEAD,
    BI_WK_DEMON,
    BI_WK_DRAGON,
    BI_W_IMPACT,
    BI_WB_ACID,
    BI_WB_ELEC,
    BI_WB_FIRE,
    BI_WB_COLD,
    BI_WB_POIS,
    BI_APHASE,
    BI_ATELEPORT,
    BI_AESCAPE,
    BI_AFUEL,
    BI_AHEAL,
    BI_AEZHEAL,
    BI_ALIFE,
    BI_AID,
    BI_ASPEED,
    BI_ASTFMAGI,
    BI_ASTFDEST,
    BI_ATPORTOTHER,
    BI_ACUREPOIS,
    BI_ADETTRAP,
    BI_ADETDOOR,
    BI_ADETEVIL,
    BI_AMAGICMAP,
    BI_ARECHARGE,
    BI_ALITE,
    BI_APFE,
    BI_AGLYPH,
    BI_ACCW,
    BI_ACSW,
    BI_ACLW,
    BI_AENCH_TOH,
    BI_AENCH_TOD,
    BI_AENCH_SWEP,
    BI_AENCH_ARM,
    BI_AENCH_SARM,
    BI_ABRAND,
    BI_NEED_ENCHANT_TO_A,
    BI_NEED_ENCHANT_TO_H,
    BI_NEED_ENCHANT_TO_D,
    BI_NEED_BRAND_WEAPON,
    BI_ARESHEAT,
    BI_ARESCOLD,
    BI_ARESPOIS,
    BI_ATELEPORTLVL, /* scroll of teleport level */
    BI_AHWORD, /* Holy Word prayer */
    BI_AMASSBAN, /* ?Mass Banishment */
    BI_ASHROOM,
    BI_AROD1, /* Attack rods */
    BI_AROD2, /* Attack rods */
    BI_WORN_NEED_ID,
    BI_ALL_NEED_ID,
    BI_ADIGGER,
    BI_GOOD_S_CHG,
    BI_GOOD_W_CHG,
    BI_MULTIPLE_BONUSES,
    BI_DINV, /* See Inv Spell Legal */
    BI_WEIGHT, /* weight of all inventory and equipment */
    BI_CARRY, /* carry capacity */
    BI_EMPTY, /* number of empty slots */
    BI_SAURON_DEAD,
    BI_PREP_BIG_FIGHT,

    BI_MAX
};

struct goals {
    /* goals */
    int16_t type; /* Flowing (goal type) */

    struct loc g; /* Goal location */

    bool rising; /* returning to town */
    bool leaving; /* leaving the level */
    bool fleeing; /* fleeing the level */
    bool fleeing_lunal; /* fleeing the level in lunal */
    bool fleeing_munchkin; /* Fleeing level while in munchkin Mode */
    bool fleeing_to_town; /* Fleeing the level to town */
    bool ignoring; /* ignoring monsters */
    bool less; /* return to, but don't use, the next up stairs */

    int recalling; /* waiting for recall, guessing turns left */

    int16_t shop; /* Next shop to visit */
    int16_t ware; /* Next item to buy there */
    int16_t item; /* Next item to sell there */
};

struct temp {
    /* time stamps for processing see invisible */
    int16_t need_see_invis;
    int16_t see_inv;

    bool res_fire;
    bool res_cold;
    bool res_acid;
    bool res_elec;
    bool res_pois;

    bool prot_from_evil;
    bool fast;
    bool bless;
    bool hero;
    bool berserk;
    bool fastcast;
    bool regen;
    bool smite_evil;
    bool venom;
    bool shield;
};

/*
 * All the information the borg knows about itself
 */
struct borg_struct {
    struct player *player; /* !HACK to work around a MSVC bug */

    /* current traits, set in borg_notice */
    int *trait;
    /* items the borg is carrying or wearing */
    int *has;
    /* activations for artifacts the borg has */
    int *activation;

    /* how powerful the borg thinks it is set in borg_power */
    int32_t power;

    /* Current location */
    struct loc c;

    /* hit points last game turn to track change in hp */
    int16_t oldchp;

    /* activity flags */
    bool lunal_mode;
    bool munchkin_mode;

    bool stair_less; /* Use the next "up" staircase */
    bool stair_more; /* Use the next "down" staircase */

    bool in_shop;

    /* a 3 state boolean */
    /*-1 = not checked yet */
    /* 0 = not ready */
    /* 1 = ready */
    int ready_morgoth;

    struct temp temp;
    /* time stamps for processing see invisible */
    int16_t need_see_invis;
    int16_t see_inv;

    /* shifting the view (current panel) */
    bool    need_shift_panel; /* to spot off-screens */
    int16_t when_shift_panel;
    int16_t time_this_panel; /* Current "time" on current panel*/

    /* activity flags with time */
    int16_t no_retreat; /* amount of time to not retreat */
    int16_t resistance; /* borg is Resistant to all elements */
    int16_t when_call_light; /* When we last did call light */
    int16_t when_wizard_light; /* When we last did wizard light */
    int16_t when_detect_traps; /* When we last detected traps */
    int16_t when_detect_doors; /* When we last detected doors */
    int16_t when_detect_walls; /* When we last detected walls */
    int16_t when_detect_evil; /* When we last detected evil */
    int16_t when_detect_obj; /* When we last detected objects */
    int16_t when_last_kill_mult; /* When a multiplier was last killed */

    int16_t no_rest_prep; /* borg wont rest for a few turns */

    int16_t times_twitch; /* how often twitchy on this level */
    int16_t escapes; /* how often teleported on this level */

    /* goals */
    struct goals goal;

    int16_t stat_max[STAT_MAX]; /* Current "maximal" stat values    */
    int16_t stat_cur[STAT_MAX]; /* Current "natural" stat values    */
    int16_t stat_ind[STAT_MAX]; /* Current "additions" to stat values   */

    /* number of books */
    int16_t amt_book[9];
    /* location of books in inventory */
    int16_t book_idx[9];

    /* need add to stat potions */
    bool need_statgain[STAT_MAX];
    /* Stat potions in inventory*/
    int16_t amt_statgain[STAT_MAX];
};
extern struct borg_struct borg;

extern bool borg_simulate; /* Simulation flag */
extern bool borg_attacking; /* Simulation flag */

/* defense flags */
extern bool borg_on_glyph; /* borg is standing on a glyph of warding */
extern bool borg_create_door; /* borg is going to create doors */
extern bool borg_sleep_spell;
extern bool borg_sleep_spell_ii;
extern bool borg_crush_spell;
extern bool borg_slow_spell; /* borg is about to cast the spell */
extern bool borg_confuse_spell;
extern bool borg_fear_mon_spell;

extern int16_t borg_game_ratio; /* the ratio of borg time to game time */

/* array of the strings that match the BI_* values */
extern const char *prefix_pref[];

/* Mega-Hack - indices of the player classes */
#define CLASS_WARRIOR     0
#define CLASS_MAGE        1
#define CLASS_DRUID       2
#define CLASS_PRIEST      3
#define CLASS_NECROMANCER 4
#define CLASS_PALADIN     5
#define CLASS_ROGUE       6
#define CLASS_RANGER      7
#define CLASS_BLACKGUARD  8

#define MAX_CLASSES 9 /* Max # of classes 0 = warrior, 5 = Paladin */

/*
 * Utility to calculate the number of blows an item will get
 */
extern int borg_calc_blows(borg_item *item);

/*
 * Extract various bonuses
 */
extern void borg_notice(bool notice_swap);

/*
 * Update the "frame" info from the screen
 */
extern void borg_notice_player(void);

extern void borg_trait_init(void);
extern void borg_trait_free(void);

#endif

#endif
