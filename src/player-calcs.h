/** \file player-calcs.h
	\brief Player temporary status structures.
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2014 Nick McConnell
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

#ifndef PLAYER_CALCS_H
#define PLAYER_CALCS_H

/*
 * Indexes of the various "stats" (hard-coded by savefiles, etc).
 */
enum
{
	A_STR = 0,
	A_INT,
	A_WIS,
	A_DEX,
	A_CON,

	A_MAX
};


/*
 * Skill indexes
 */
enum
{
	SKILL_DISARM,			/* Skill: Disarming */
	SKILL_DEVICE,			/* Skill: Magic Devices */
	SKILL_SAVE,				/* Skill: Saving throw */
	SKILL_STEALTH,			/* Skill: Stealth factor */
	SKILL_SEARCH,			/* Skill: Searching ability */
	SKILL_SEARCH_FREQUENCY,	/* Skill: Searching frequency */
	SKILL_TO_HIT_MELEE,		/* Skill: To hit (normal) */
	SKILL_TO_HIT_BOW,		/* Skill: To hit (shooting) */
	SKILL_TO_HIT_THROW,		/* Skill: To hit (throwing) */
	SKILL_DIGGING,			/* Skill: Digging */

	SKILL_MAX
};

/* Terrain that the player has a chance of digging through */
enum
{
	DIGGING_RUBBLE = 0,
	DIGGING_MAGMA,
	DIGGING_QUARTZ,
	DIGGING_GRANITE,
	DIGGING_DOORS,
	
	DIGGING_MAX
};

/*
 * Bit flags for the "player->notice" variable
 */
#define PN_COMBINE      0x00000001L    /* Combine the pack */
#define PN_REORDER      0x00000002L    /* Reorder the pack */
#define PN_AUTOINSCRIBE 0x00000004L    /* Autoinscribe items */
#define PN_PICKUP       0x00000008L    /* Pick stuff up */
#define PN_SQUELCH      0x00000010L    /* Squelch stuff */
#define PN_SORT_QUIVER  0x00000020L    /* Sort the quiver */
#define PN_MON_MESSAGE	0x00000040L	   /* flush monster pain messages */


/*
 * Bit flags for the "player->update" variable
 */
#define PU_BONUS		0x00000001L	/* Calculate bonuses */
#define PU_TORCH		0x00000002L	/* Calculate torch radius */
#define PU_HP			0x00000004L	/* Calculate chp and mhp */
#define PU_MANA			0x00000008L	/* Calculate csp and msp */
#define PU_SPELLS		0x00000010L	/* Calculate spells */
#define PU_FORGET_VIEW	0x00000020L	/* Forget field of view */
#define PU_UPDATE_VIEW	0x00000040L	/* Update field of view */
#define PU_FORGET_FLOW	0x00000080L	/* Forget flow data */
#define PU_UPDATE_FLOW	0x00000100L	/* Update flow data */
#define PU_MONSTERS		0x00000200L	/* Update monsters */
#define PU_DISTANCE		0x00000400L	/* Update distances */
#define PU_PANEL		0x00000800L	/* Update panel */


/*
 * Bit flags for the "player->redraw" variable
 */
#define PR_MISC			0x00000001L	/* Display Race/Class */
#define PR_TITLE		0x00000002L	/* Display Title */
#define PR_LEV			0x00000004L	/* Display Level */
#define PR_EXP			0x00000008L	/* Display Experience */
#define PR_STATS		0x00000010L	/* Display Stats */
#define PR_ARMOR		0x00000020L	/* Display Armor */
#define PR_HP			0x00000040L	/* Display Hitpoints */
#define PR_MANA			0x00000080L	/* Display Mana */
#define PR_GOLD			0x00000100L	/* Display Gold */
#define PR_HEALTH		0x00000200L	/* Display Health Bar */
#define PR_SPEED		0x00000400L	/* Display Extra (Speed) */
#define PR_STUDY		0x00000800L	/* Display Extra (Study) */
#define PR_DEPTH		0x00001000L	/* Display Depth */
#define PR_STATUS		0x00002000L
#define PR_DTRAP		0x00004000L /* Trap detection indicator */
#define PR_STATE		0x00008000L	/* Display Extra (State) */
#define PR_MAP			0x00010000L	/* Redraw whole map */
#define PR_INVEN		0x00010000L /* Display inven/equip */
#define PR_EQUIP		0x00040000L /* Display equip/inven */
#define PR_MESSAGE		0x00080000L /* Display messages */
#define PR_MONSTER		0x00100000L /* Display monster recall */
#define PR_OBJECT		0x00200000L /* Display object recall */
#define PR_MONLIST		0x00400000L /* Display monster list */
#define PR_ITEMLIST     0x00800000L /* Display item list */

/* Display Basic Info */
#define PR_BASIC \
	(PR_MISC | PR_TITLE | PR_STATS | PR_LEV |\
	 PR_EXP | PR_GOLD | PR_ARMOR | PR_HP |\
	 PR_MANA | PR_DEPTH | PR_HEALTH | PR_SPEED)

/* Display Extra Info */
#define PR_EXTRA \
	(PR_STATUS | PR_STATE | PR_STUDY)

/*
 * The range of possible indexes into tables based upon stats.
 * Currently things range from 3 to 18/220 = 40.
 */
#define STAT_RANGE 38

/*** Structures ***/

/**
 * All the variable state that changes when you put on/take off equipment.
 */
typedef struct player_state {
	s16b speed;		/* Current speed */

	s16b num_blows;		/* Number of blows x100 */
	s16b num_shots;		/* Number of shots */

	byte ammo_mult;		/* Ammo multiplier */
	byte ammo_tval;		/* Ammo variety */

	s16b stat_add[A_MAX];	/* Equipment stat bonuses */
	s16b stat_ind[A_MAX];	/* Indexes into stat tables */
	s16b stat_use[A_MAX];	/* Current modified stats */
	s16b stat_top[A_MAX];	/* Maximal modified stats */

	s16b dis_ac;		/* Known base ac */
	s16b ac;			/* Base ac */

	s16b dis_to_a;		/* Known bonus to ac */
	s16b to_a;			/* Bonus to ac */

	s16b to_h;			/* Bonus to hit */
	s16b dis_to_h;		/* Known bonus to hit */

	s16b to_d;			/* Bonus to dam */
	s16b dis_to_d;		/* Known bonus to dam */

	s16b see_infra;		/* Infravision range */

	s16b cur_light;		/* Radius of light (if any) */

	s16b skills[SKILL_MAX];	/* Skills */

	u32b noise;			/* Derived from stealth */

	bool heavy_wield;	/* Heavy weapon */
	bool heavy_shoot;	/* Heavy shooter */
	bool icky_wield;	/* Icky weapon shooter */

	bool cumber_armor;	/* Mana draining armor */
	bool cumber_glove;	/* Mana draining gloves */

	bitflag flags[OF_SIZE];	/* Status flags from race and items */
} player_state;

/**
 * Temporary, derived, player-related variables used during play but not saved
 *
 * Some of these probably should go to the UI
 */
typedef struct player_upkeep {
	bool playing;			/* True if player is playing */
	bool leaving;			/* True if player is leaving */
	bool autosave;			/* True if autosave is pending */

	int energy_use;			/* Energy use this turn */
	int new_spells;			/* Number of spells available */

	struct monster *health_who;			/* Health bar trackee */
	struct monster_race *monster_race;	/* Monster race trackee */
	int object_idx;						/* Object trackee */
	struct object_kind *object_kind;	/* Object kind trackee */

	u32b notice;		/* Bit flags for pending actions such as 
						 * reordering inventory, squelching, etc. */
	u32b update;		/* Bit flags for recalculations needed 
						 * such as HP, or visible area */
	u32b redraw;	    /* Bit flags for things that /have/ changed,
						 * and just need to be redrawn by the UI,
						 * such as HP, Speed, etc.*/

	int command_wrk;		/* Used by the UI to decide whether
							 * to start off showing equipment or
							 * inventory listings when offering
							 * a choice.  See obj-ui.c */

	bool create_up_stair;		/* Create up stair on next level */
	bool create_down_stair;		/* Create down stair on next level */

	int running;				/* Running counter */
	bool running_withpathfind;	/* Are we using the pathfinder ? */
	bool running_firststep;		/* Is this our first step running? */

	int total_weight;			/* Total weight being carried */
	int inven_cnt;				/* Number of items in inventory */
	int equip_cnt;				/* Number of items in equipment */
	int quiver_size;			/* Number of items in the quiver */
	int quiver_slots;			/* Number of inventory slots for the quiver */
	int quiver_remainder;		/* Number of quiver items modulo slot size */
} player_upkeep;


extern const byte adj_str_blow[STAT_RANGE];
extern const byte adj_dex_safe[STAT_RANGE];
extern const byte adj_con_fix[STAT_RANGE];
extern const byte adj_str_hold[STAT_RANGE];

void calc_bonuses(object_type inventory[], player_state *state, bool id_only);
void calc_digging_chances(player_state *state, int chances[DIGGING_MAX]);
int calc_blows(const object_type *o_ptr, player_state *state, int extra_blows);

void health_track(struct player_upkeep *upkeep, struct monster *m_ptr);
void monster_race_track(struct player_upkeep *upkeep, 
						struct monster_race *race);
void track_object(struct player_upkeep *upkeep, int item);
void track_object_kind(struct player_upkeep *upkeep, struct object_kind *kind);
bool tracked_object_is(struct player_upkeep *upkeep, int item);

void notice_stuff(struct player_upkeep *upkeep);
void update_stuff(struct player_upkeep *upkeep);
void redraw_stuff(struct player_upkeep *upkeep);
void handle_stuff(struct player_upkeep *upkeep);
int weight_remaining(void);

#endif /* !PLAYER_CALCS_H */
