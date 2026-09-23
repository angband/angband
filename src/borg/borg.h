/**
 *
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

#ifndef INCLUDED_BORG_H
#define INCLUDED_BORG_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"
#include "../obj-ignore.h"

#ifdef ALLOW_BORG

#include "borg-trait.h"

/* type out the borg time so overflows can be larger or smaller */
typedef int32_t borg_time;
#define BORG_TIME_MAX INT32_MAX

/*
 * Use a simple internal random number generator
 */
extern uint32_t borg_rand_local; /* Save personal setting */

/*
 * Date of the last change
 */
extern char borg_engine_date[];

/* options from the borg.txt file */
/* IMPORTANT keep these in sync with borg_settings in borg-init.c */
enum {
    BORG_VERBOSE,
    BORG_MUNCHKIN_START,
    BORG_MUNCHKIN_LEVEL,
    BORG_MUNCHKIN_DEPTH,
    BORG_WORSHIPS_DAMAGE,
    BORG_WORSHIPS_SPEED,
    BORG_WORSHIPS_HP,
    BORG_WORSHIPS_MANA,
    BORG_WORSHIPS_AC,
    BORG_WORSHIPS_GOLD,
    BORG_PLAYS_RISKY,
    BORG_KILLS_UNIQUES,
    BORG_USES_SWAPS,
    BORG_USES_DYNAMIC_CALCS,
    BORG_STOP_DLEVEL,
    BORG_STOP_CLEVEL,
    BORG_NO_DEEPER,
    BORG_STOP_KING,
    BORG_CHEAT_DEATH,
    BORG_RESPAWN_WINNERS,
    BORG_RESPAWN_CLASS,
    BORG_RESPAWN_RACE,
    BORG_CHEST_FAIL_TOLERANCE,
    BORG_DELAY_FACTOR,
    BORG_MONEY_SCUM_AMOUNT,
    BORG_SELF_SCUM,
    BORG_LUNAL_MODE,
    BORG_SELF_LUNAL,
    BORG_ENCHANT_LIMIT,
    BORG_DUMP_LEVEL,
    BORG_SAVE_DEATH,
    BORG_STOP_ON_BELL,
    BORG_ALLOW_STRANGE_OPTS,
    BORG_AUTOSAVE,
    BORG_RESTORE_IGNORE_SETTINGS,
    BORG_MAX_SETTINGS
};
extern int *borg_cfg;

struct borg_best
{
    bool    home;
    uint8_t tval; /* Item type */
    uint8_t sval; /* Item sub-type */
    int16_t pval; /* Item extra-info */
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

    /* avoidance: this is the level of danger the borg tries to avoid */
    /* it is usually the current hit points but can be boosted or reduced */
    /* depending on the situation */
    int16_t avoidance;

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

    /*
     * Temporary statuses
     */
    struct {
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
    } temp;

    /*
     * Status variables
     */
    struct {
        bool active; /* Actually active */
        bool cancel; /* Being cancelled */
        bool save; /* do a save next time we get to press a key! */

        int16_t old_depth;
        int16_t respawning;

        bool cheat_death; /* cheat death is on */

        bool anti_summon; /* borg is in an anti-summon corridor */
        bool digging; /* digging an anti-summon corridor */

        bool need_alter; /* needs to alter to allow digging into a wall */
        bool no_alter; /* do not use "+" to alter during a move */

        bool redraw; /* need to redraw the screen */
        bool vault; /* guess that there is a vault on this level */

        bool desperate; /* borg is desperate and will take risks */
    } status;

    /* times */
    struct {
        borg_time now; /* Current "time" */
        borg_time level; /* When this level began */
        borg_time town; /* When I last left town */
        borg_time morgoth; /* Last time I saw Morgoth */

        /* activity timers */
        borg_time antisummon; /* When last in an anti-summon spot */
        borg_time call_light; /* When we last did call light */
        borg_time wizard_light; /* When we last did wizard light */
        borg_time detect_traps; /* When we last detected traps */
        borg_time detect_doors; /* When we last detected doors */
        borg_time detect_walls; /* When we last detected walls */
        borg_time detect_evil; /* When we last detected evil */
        borg_time detect_obj; /* When we last detected objects */
        borg_time last_kill_mult; /* When a multiplier was last killed */
    } time;

    /* time stamps for processing see invisible */
    borg_time need_see_invis;
    int16_t   see_inv;

    /* shifting the view (current panel) */
    bool      need_shift_panel; /* to spot off-screens */
    borg_time when_shift_panel;

    /* anti-bounce count to avoid repeated motions */
    int16_t antibounce_count;

    /* activity flags with countdown */
    int16_t no_retreat; /* amount of time to not retreat */
    int16_t resistance; /* borg is Resistant to all elements */

    int16_t no_rest_prep; /* borg won't rest for a few turns */

    int16_t times_twitch; /* how often twitchy on this level */
    int16_t escapes; /* how often teleported on this level */

    /* trying an unknown potion wand rod scroll etc */
    bool trying_unknown;

    bool dont_react; /* don't react to messages, just queue them */
    bool targeting; /* just targetted so expect "Direction" prompt */

    /* goals */
    struct {
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
        bool waiting; /* waiting for an approaching monster */

        int recalling; /* waiting for recall, guessing turns left */
        int descending; /* waiting for deep descent */

        int16_t shop; /* Next shop to visit */
        int16_t ware; /* Next item to buy there */
        int16_t item; /* Next item to sell there */

        bool              do_best;
        struct borg_best *best_item;
    } goal;

    /* borg initialization save */
    struct {
        /*
         * KEYMAP_MODE_ROGUE or KEYMAP_MODE_ORIG
         */
        int key_mode;

        /*
         * object ignore settings
         */
        uint8_t *kinfo_ignore;
        uint8_t  ignore_level[ITYPE_MAX];
        bool   **ego_ignore_types;
    } init_save;

    struct {
        uint16_t breeders; /* number of breeders on this level */
        bool     scary; /* scary guy on this level */

        struct loc   morgoth_panel;
        unsigned int unique;
    } mon;

    struct {
        unsigned int morgoth;
        unsigned int sauron;
        unsigned int tarrasque;
        unsigned int maggot;
        unsigned int bullroarer;
        unsigned int beorn;
        unsigned int grip;
        unsigned int fang;
        unsigned int osse;

        unsigned int squint_eyed_rogue;
        unsigned int small_kobold;
        unsigned int soldier;
        unsigned int cutpurse;
        unsigned int acolyte;
        unsigned int apprentice;
        unsigned int kobold;
        unsigned int shrieker_mushroom_patch;
        unsigned int filthy_street_urchin;
        unsigned int battle_scarred_veteran;
        unsigned int mean_looking_mercenary;
        unsigned int giant_white_mouse;
        unsigned int white_worm_mass;
        unsigned int green_worm_mass;
        unsigned int cave_spider;
        unsigned int red_naga;
        unsigned int giant_red_frog;
        unsigned int radiation_eye;
        unsigned int yellow_worm_mass;
        unsigned int gravity_hound;
        unsigned int inertia_hound;
        unsigned int dread;
        unsigned int dracolich;
        unsigned int dracolisk;
        unsigned int light_hound;
        unsigned int dark_hound;
        unsigned int pukelman;
        unsigned int night_mare;
        unsigned int drolem;
    } id;

    struct {
        bool morgoth;
        bool breeder;

        /* currently fighting a unique */
        /* +1 for normal unique */
        /* +10 for questor (morgoth or sauron) */
        int16_t unique;
        bool    evil_unique; /* evil unique  - for banishment */
        bool    summoner; /* summoner - for banishment */
        int16_t summoner_idx; /* index of a summoner */
    } near;

    bool morgoth_position; /* in position to fight morgoth  */

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

/*
 * Number of turns to (manually) step for (zero means forever)
 */
extern uint16_t borg_step;

extern int w_x; /* Current panel offset (X) */
extern int w_y; /* Current panel offset (Y) */

/*
 * Special "inkey_hack" hook.
 */
extern struct keypress (*inkey_hack)(int flush_first);

/*
 * Set the hook for the game.
 */
extern void borg_update_entrypoint(bool start);

#endif
#endif
