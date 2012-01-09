/* File: borg6.h */

/* Purpose: Header file for "borg6.c" -BEN- */

#ifndef INCLUDED_BORG6_H
#define INCLUDED_BORG6_H

#include "angband.h"
#include "object/tvalsval.h"
#include "cave.h"

#ifdef ALLOW_BORG

/*
 * This file provides support for "borg6.c".
 */

#include "borg1.h"
#include "borg2.h"
#include "borg3.h"

/*
 * Possible values of "goal"
 */
#define GOAL_KILL   1       /* Monsters */
#define GOAL_TAKE   2       /* Objects */
#define GOAL_MISC   3       /* Stores */
#define GOAL_DARK   4       /* Exploring */
#define GOAL_XTRA   5       /* Searching */
#define GOAL_BORE   6       /* Leaving */
#define GOAL_FLEE   7       /* Fleeing */
#define GOAL_VAULT  8		/* Vaults */
#define GOAL_RECOVER 9		/* Resting safely */
#define GOAL_DIGGING 10		/* Anti-summon Corridor */



/*
 * Minimum "harmless" food
 */

#define SV_FOOD_MIN_OKAY    SV_FOOD_CURE_POISON



/*
 * Attempt to induce "word of recall"
 */
extern bool borg_recall(void);

/*
 * Low level goals
 */
extern bool borg_caution(void);
extern bool borg_attack(bool boosted_bravery);
extern bool borg_munchkin_mage(void);
extern bool borg_munchkin_melee(void);
extern bool borg_recover(void);

extern bool borg_offset_ball(void);
extern bool borg_defend(int p);
extern bool borg_perma_spell(void);

extern bool borg_check_rest(int y, int x);
extern void borg_near_monster_type(int dist);

/*
 * Twitchy goals
 */
extern bool borg_charge_kill(void);
extern bool borg_charge_take(void);
extern bool borg_twitchy(void);



/*
 * Continue a high level goal
 */
extern bool borg_flow_old(int why);

/*
 * Flow to stairs
 */
extern bool borg_flow_stair_both(int why, bool sneak);
extern bool borg_flow_stair_less(int why, bool sneak);
extern bool borg_flow_stair_more(int why, bool sneak, bool brave);


extern bool borg_flow_glyph(int why);
extern bool borg_flow_light(int why);
extern bool borg_check_LIGHT_only(void);
extern bool borg_backup_swap(int p);
extern bool borg_flow_recover(bool viewable, int dist);

/*
 * Flow to shops
 */
extern bool borg_flow_shop_visit(void);
extern bool borg_flow_shop_entry(int n);

/*
 * Flow towards monsters/objects
 */
extern bool borg_flow_kill(bool viewable, int nearness);
extern bool borg_flow_kill_aim(bool viewable);
extern bool borg_flow_kill_corridor_1(bool viewable);
extern bool borg_flow_kill_corridor_2(bool viewable);
extern bool borg_flow_kill_direct(bool viewable, bool twitchy);
extern bool borg_flow_take(bool viewable, int nearness);
extern bool borg_flow_vein(bool viewable, int nearness);
extern bool borg_flow_take_lunal(bool viewable, int nearness);
extern bool borg_flow_take_scum(bool viewable, int nearness);
extern void borg_flow_direct_dig(int m_y, int m_x);
extern bool borg_flow_vault(int nearness);
extern bool borg_excavate_vault(int nearness);
extern bool borg_LIGHT_beam(bool simulation);
/*
 * Flow towards "interesting" grids
 */
extern bool borg_flow_dark(bool neer);

/*
 * Search for secret doors
 */
extern bool borg_flow_spastic(bool bored);

extern bool borg_target(int y, int x);
extern int borg_launch_damage_one(int i, int dam, int typ, int ammo_location);
extern int borg_attack_aux_thrust(void);


extern void borg_log_battle(bool);
extern void borg_log_event(cptr event);
extern bool borg_target_unknown_wall(int g_y,int g_x);

/*
 * Initialize this file
 */
extern void borg_init_6(void);




#endif

#endif

