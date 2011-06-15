/*
 * File: stats.c
 * Purpose: Statistics collection on dungeon generation
 *
 * Copyright (c) 2008 Andrew Sidwell
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 */
#include "math.h"
#include "angband.h"
#include "cave.h"
#include "cmds.h"
#include "wizard.h"
#include "object/tvalsval.h"
#include "monster/monster.h"
#include "monster/constants.h"
#include "effects.h"
#include "generate.h"
/*
 * The stats programs here will provide information on the dungeon, the monsters
 * in it, and the items that they drop.  Statistics are gotten from a given level
 * by generating a new level, collecting all the items (noting if they were generated
 * in a vault).  Then all non-unique monsters are killed and their stats are tracked.
 * The items from these monster drops are then collected and analyzed.  Lastly, all
 * unique monsters are killed, and their drops are analyzed.  In this way, it is possible
 * to separate unique drops and normal monster drops.
 *
 * There are two options for simulating the entirety of the dungeon.  There is a "diving"
 * option that begins each level with all artifacts and uniques available. and 
 * there is a "level-clearing" option that simulates all 100 levels of the dungeon, removing
 * artifacts and uniques as they are discovered/killed.  "diving" option only catalogues
 * every 5 levels.
 *
 * At the end of the "level-clearing" log file, extra post-processing is done to find the
 * mean and standard deviation for the level you are likely to first gain an item with
 * a key resistance or item.
 * 
 * In addition to these sims there is a shorter sim that tests for dungeon connectivity.
*/

#ifdef USE_STATS

/*** Statsgen ***/

/* Logfile to store results in */
static ang_file *stats_log = NULL;

 /* this is the size of arrays used to calculate mean and std_dev.
  * these values will be calculated over the first TRIES_SIZE attempts
  * or less if TRIES_SIZE is less than tries
  */
 #define TRIES_SIZE 100
 #define MAX_LVL 101
 
 /* default for number of tries */
int tries=50;
/* the simulation number that we are on */
int iter;
/* amount to add each time an item comes up */
static double addval;
/* flag for whether we are in clearing mode */
bool clearing = FALSE;
/* flag for regenning randart */
bool regen = FALSE;

/*** These are items to track for each iteration ***/
/* total number of artifacts found */
static int art_it[TRIES_SIZE];

/* first level we find items with various abilities */
static int fa_it[TRIES_SIZE], si_it[TRIES_SIZE], po_it[TRIES_SIZE], nx_it[TRIES_SIZE];
static int cf_it[TRIES_SIZE], bl_it[TRIES_SIZE], te_it[TRIES_SIZE];

static int mb1_it[TRIES_SIZE], mb2_it[TRIES_SIZE], mb3_it[TRIES_SIZE];
static int mb4_it[TRIES_SIZE], mb5_it[TRIES_SIZE], mb6_it[TRIES_SIZE];
static int mb7_it[TRIES_SIZE], mb8_it[TRIES_SIZE], mb9_it[TRIES_SIZE];

/*** these are variables for all the things we want to collect ***/
/* gold */
static double gold_total[MAX_LVL], gold_floor[MAX_LVL], gold_mon[MAX_LVL], gold_wall[MAX_LVL];

/**** General equipment flags ****/
/* has FA */
static double faeq_total[MAX_LVL], faeq_mon[MAX_LVL], faeq_vault[MAX_LVL];
/* has SI */
static double sieq_total[MAX_LVL], sieq_mon[MAX_LVL], sieq_vault[MAX_LVL];
/* has relem */
static double reeq_total[MAX_LVL], reeq_mon[MAX_LVL], reeq_vault[MAX_LVL];
/* has rbase */
static double rbeq_total[MAX_LVL], rbeq_mon[MAX_LVL], rbeq_vault[MAX_LVL];
/* has rpois */
static double poeq_total[MAX_LVL], poeq_mon[MAX_LVL], poeq_vault[MAX_LVL];
/* has rnexus */
static double nxeq_total[MAX_LVL], nxeq_mon[MAX_LVL], nxeq_vault[MAX_LVL];
/* has rblind */
static double bleq_total[MAX_LVL], bleq_mon[MAX_LVL], bleq_vault[MAX_LVL];
/* has conf */
static double cfeq_total[MAX_LVL], cfeq_mon[MAX_LVL], cfeq_vault[MAX_LVL];
/* has speed */
static double speq_total[MAX_LVL], speq_mon[MAX_LVL], speq_vault[MAX_LVL];
/* has telep */
static double teeq_total[MAX_LVL], teeq_mon[MAX_LVL], teeq_vault[MAX_LVL];


/**** WEAPONS ****/
static double weap_total[MAX_LVL], weap_mon[MAX_LVL], weap_vault[MAX_LVL];
/* bad average good */
static double bdweap_total[MAX_LVL], bdweap_mon[MAX_LVL], bdweap_vault[MAX_LVL];
static double avweap_total[MAX_LVL], avweap_mon[MAX_LVL], avweap_vault[MAX_LVL];
static double gdweap_total[MAX_LVL], gdweap_mon[MAX_LVL], gdweap_vault[MAX_LVL];
/* has slay (not evil) */
static double slweap_total[MAX_LVL], slweap_mon[MAX_LVL], slweap_vault[MAX_LVL];
/* has slay evil */
static double evweap_total[MAX_LVL], evweap_mon[MAX_LVL], evweap_vault[MAX_LVL];
/* has a *slay* (not evil) */
static double klweap_total[MAX_LVL], klweap_mon[MAX_LVL], klweap_vault[MAX_LVL];
/* has brand */
static double brweap_total[MAX_LVL], brweap_mon[MAX_LVL], brweap_vault[MAX_LVL];
/* is westernesse */
static double weweap_total[MAX_LVL], weweap_mon[MAX_LVL], weweap_vault[MAX_LVL];
/* is defender */
static double deweap_total[MAX_LVL], deweap_mon[MAX_LVL], deweap_vault[MAX_LVL];
/* is gondolin */
static double goweap_total[MAX_LVL], goweap_mon[MAX_LVL], goweap_vault[MAX_LVL];
/* is  holy avenger */
static double haweap_total[MAX_LVL], haweap_mon[MAX_LVL], haweap_vault[MAX_LVL];
/* has extra blows */
static double xbweap_total[MAX_LVL], xbweap_mon[MAX_LVL], xbweap_vault[MAX_LVL];
/* has telep */
static double teweap_total[MAX_LVL], teweap_mon[MAX_LVL], teweap_vault[MAX_LVL];
/* is top weapon BOC, SOS, MOD */
static double huweap_total[MAX_LVL], huweap_mon[MAX_LVL], huweap_vault[MAX_LVL];
/* is uber */
static double ubweap_total[MAX_LVL], ubweap_mon[MAX_LVL], ubweap_vault[MAX_LVL];
/* is morgul */
static double moweap_total[MAX_LVL], moweap_mon[MAX_LVL], moweap_vault[MAX_LVL];

/**** LAUNCHERS ****/
static double bow_total[MAX_LVL], bow_mon[MAX_LVL], bow_vault[MAX_LVL];
/* bad average good */
static double bdbow_total[MAX_LVL], bdbow_mon[MAX_LVL], bdbow_vault[MAX_LVL];
static double avbow_total[MAX_LVL], avbow_mon[MAX_LVL], avbow_vault[MAX_LVL];
static double gdbow_total[MAX_LVL], gdbow_mon[MAX_LVL], gdbow_vault[MAX_LVL];
/* power > 15 */
static double vgbow_total[MAX_LVL], vgbow_mon[MAX_LVL], vgbow_vault[MAX_LVL];
/* xtra might  (longbow, crossbows) */
static double xmbow_total[MAX_LVL], xmbow_mon[MAX_LVL], xmbow_vault[MAX_LVL];
/* xtra shots  (longbow crossbows) */
static double xsbow_total[MAX_LVL], xsbow_mon[MAX_LVL], xsbow_vault[MAX_LVL];
/* buckland  */
static double bubow_total[MAX_LVL], bubow_mon[MAX_LVL], bubow_vault[MAX_LVL];
/* telep */
static double tebow_total[MAX_LVL], tebow_mon[MAX_LVL], tebow_vault[MAX_LVL];
/* cursed */
static double cubow_total[MAX_LVL], cubow_mon[MAX_LVL], cubow_vault[MAX_LVL];

/**** ARMOR *****/
static double arm_total[MAX_LVL], arm_mon[MAX_LVL], arm_vault[MAX_LVL];
/* bad average good */
static double bdarm_total[MAX_LVL], bdarm_mon[MAX_LVL], bdarm_vault[MAX_LVL];
static double avarm_total[MAX_LVL], avarm_mon[MAX_LVL], avarm_vault[MAX_LVL];
static double gdarm_total[MAX_LVL], gdarm_mon[MAX_LVL], gdarm_vault[MAX_LVL];

/* has str */
static double strarm_total[MAX_LVL], strarm_mon[MAX_LVL], strarm_vault[MAX_LVL];
/* has int */
static double intarm_total[MAX_LVL], intarm_mon[MAX_LVL], intarm_vault[MAX_LVL];
/* has wis */
static double wisarm_total[MAX_LVL], wisarm_mon[MAX_LVL], wisarm_vault[MAX_LVL];
/* has dex */
static double dexarm_total[MAX_LVL], dexarm_mon[MAX_LVL], dexarm_vault[MAX_LVL];
/* has con */
static double conarm_total[MAX_LVL], conarm_mon[MAX_LVL], conarm_vault[MAX_LVL];
/* has curse */
static double cuarm_total[MAX_LVL], cuarm_mon[MAX_LVL], cuarm_vault[MAX_LVL];

/* basic artifact info */
static double art_total[MAX_LVL], art_spec[MAX_LVL], art_norm[MAX_LVL];

/* artifact level info */
static double art_shal[MAX_LVL], art_ave[MAX_LVL], art_ood[MAX_LVL];

/* where normal artifacts come from */
static double art_mon[MAX_LVL], art_uniq[MAX_LVL], art_floor[MAX_LVL], art_vault[MAX_LVL], art_mon_vault[MAX_LVL];

/* consumables info */

/*POTIONS*/
static double pot_total[MAX_LVL], pot_mon[MAX_LVL], pot_vault[MAX_LVL];
/* stat gain */
static double gain_total[MAX_LVL], gain_mon[MAX_LVL], gain_vault[MAX_LVL];
/* healing */
static double bigheal_total[MAX_LVL], bigheal_mon[MAX_LVL], bigheal_vault[MAX_LVL];
/* restore mana */
static double rmana_total[MAX_LVL], rmana_mon[MAX_LVL], rmana_vault[MAX_LVL];

/*SCROLLS*/
/* scroll total */
static double scroll_total[MAX_LVL], scroll_mon[MAX_LVL], scroll_vault[MAX_LVL];
/* endgame scrolls, dest, ban, mass_ban, rune */
static double escroll_total[MAX_LVL], escroll_mon[MAX_LVL], escroll_vault[MAX_LVL];
/* acquirement scrolls */
static double acq_total[MAX_LVL], acq_mon[MAX_LVL], acq_vault[MAX_LVL];

/*RODS*/
/* rod total */
static double rod_total[MAX_LVL], rod_mon[MAX_LVL], rod_vault[MAX_LVL];

/*utility, dtrap, dstairs, dobj, light, illum*/
static double urod_total[MAX_LVL], urod_mon[MAX_LVL], urod_vault[MAX_LVL];

/*tele other*/
static double torod_total[MAX_LVL], torod_mon[MAX_LVL], torod_vault[MAX_LVL];

/*detect all */
static double drod_total[MAX_LVL], drod_mon[MAX_LVL], drod_vault[MAX_LVL];

/*endgame: speed, healing*/
static double erod_total[MAX_LVL], erod_mon[MAX_LVL], erod_vault[MAX_LVL];

/*STAVES*/
/*total*/
static double staff_total[MAX_LVL], staff_mon[MAX_LVL], staff_vault[MAX_LVL];

/* speed */
static double sstaff_total[MAX_LVL], sstaff_mon[MAX_LVL], sstaff_vault[MAX_LVL];

/* destruction */
static double dstaff_total[MAX_LVL], dstaff_mon[MAX_LVL], dstaff_vault[MAX_LVL];

/*dispelling, dispel evil, holiness, power */
static double kstaff_total[MAX_LVL], kstaff_mon[MAX_LVL], kstaff_vault[MAX_LVL];

/*powerful: healing, magi, banishment*/
static double pstaff_total[MAX_LVL], pstaff_mon[MAX_LVL], pstaff_vault[MAX_LVL];

/* WANDS */
/* total */
static double wand_total[MAX_LVL], wand_mon[MAX_LVL], wand_vault[MAX_LVL];
/* tele-other */
static double towand_total[MAX_LVL], towand_mon[MAX_LVL], towand_vault[MAX_LVL];

/* RINGS */
static double ring_total[MAX_LVL], ring_mon[MAX_LVL], ring_vault[MAX_LVL];
/* curse ring */
static double curing_total[MAX_LVL], curing_mon[MAX_LVL], curing_vault[MAX_LVL];
/* speed ring */
static double spring_total[MAX_LVL], spring_mon[MAX_LVL], spring_vault[MAX_LVL];
/* stat rings */
static double string_total[MAX_LVL], string_mon[MAX_LVL], string_vault[MAX_LVL];
/* r pois */
static double poring_total[MAX_LVL], poring_mon[MAX_LVL], poring_vault[MAX_LVL];
/* free action */
static double faring_total[MAX_LVL], faring_mon[MAX_LVL], faring_vault[MAX_LVL];
/* see inv */
static double siring_total[MAX_LVL], siring_mon[MAX_LVL], siring_vault[MAX_LVL];
/* branding rings */
static double brring_total[MAX_LVL], brring_mon[MAX_LVL], brring_vault[MAX_LVL];
/* elven rings */
static double elring_total[MAX_LVL], elring_mon[MAX_LVL], elring_vault[MAX_LVL];
/* the one ring */
static double onering_total[MAX_LVL], onering_mon[MAX_LVL], onering_vault[MAX_LVL];

/* amulets */
static double amu_total[MAX_LVL], amu_mon[MAX_LVL], amu_vault[MAX_LVL];
/* wisdom */
static double wisamu_total[MAX_LVL], wisamu_mon[MAX_LVL], wisamu_vault[MAX_LVL];
/* trickery, weaponmastery, or magi */
static double endamu_total[MAX_LVL], endamu_mon[MAX_LVL], endamu_vault[MAX_LVL];
/* telepathy */
static double teamu_total[MAX_LVL], teamu_mon[MAX_LVL], teamu_vault[MAX_LVL];
/* cursed */
static double cuamu_total[MAX_LVL], cuamu_mon[MAX_LVL], cuamu_vault[MAX_LVL];


/* AMMO */
static double ammo_total[MAX_LVL], ammo_mon[MAX_LVL], ammo_vault[MAX_LVL];
/* bad average good */
static double bdammo_total[MAX_LVL], bdammo_mon[MAX_LVL], bdammo_vault[MAX_LVL];
static double avammo_total[MAX_LVL], avammo_mon[MAX_LVL], avammo_vault[MAX_LVL];
static double gdammo_total[MAX_LVL], gdammo_mon[MAX_LVL], gdammo_vault[MAX_LVL];
/* ego ammo */
static double egammo_total[MAX_LVL], egammo_mon[MAX_LVL], egammo_vault[MAX_LVL];
/* very good ammo, seeker/mithril */
static double vgammo_total[MAX_LVL], vgammo_mon[MAX_LVL], vgammo_vault[MAX_LVL];
/* awesome ammo, seeker/mithril + brand */
static double awammo_total[MAX_LVL], awammo_mon[MAX_LVL], awammo_vault[MAX_LVL];
/* endgame ammo, seeker/mithril + slay evil */
static double evammo_total[MAX_LVL], evammo_mon[MAX_LVL], evammo_vault[MAX_LVL];
/* holy might ammo, seeker/mithril + slay evil */
static double hmammo_total[MAX_LVL], hmammo_mon[MAX_LVL], hmammo_vault[MAX_LVL];

/*** Prayer/spell BOOKS ***/
static double b1_total[MAX_LVL],b1_mon[MAX_LVL],b1_vault[MAX_LVL];
static double b2_total[MAX_LVL],b2_mon[MAX_LVL],b2_vault[MAX_LVL];
static double b3_total[MAX_LVL],b3_mon[MAX_LVL],b3_vault[MAX_LVL];
static double b4_total[MAX_LVL],b4_mon[MAX_LVL],b4_vault[MAX_LVL];
static double b5_total[MAX_LVL],b5_mon[MAX_LVL],b5_vault[MAX_LVL];
static double b6_total[MAX_LVL],b6_mon[MAX_LVL],b6_vault[MAX_LVL];
static double b7_total[MAX_LVL],b7_mon[MAX_LVL],b7_vault[MAX_LVL];
static double b8_total[MAX_LVL],b8_mon[MAX_LVL],b8_vault[MAX_LVL];
static double b9_total[MAX_LVL],b9_mon[MAX_LVL],b9_vault[MAX_LVL];


/* monster info */
static double mon_total[MAX_LVL], mon_ood[MAX_LVL], mon_deadly[MAX_LVL];

/* unique info */
static double uniq_total[MAX_LVL], uniq_ood[MAX_LVL], uniq_deadly[MAX_LVL];

static void init_iter_vals(int k)
{
	art_it[k]=0;
	fa_it[k]=0, si_it[k]=0, po_it[k]=0, nx_it[k]=0;
	cf_it[k]=0, bl_it[k]=0, te_it[k]=0;
	
	mb1_it[k]=0, mb2_it[k]=0, mb3_it[k]=0;
	mb4_it[k]=0, mb5_it[k]=0, mb6_it[k]=0;
	mb7_it[k]=0, mb8_it[k]=0, mb9_it[k]=0;
}
/* set everything to 0.0 to begin */
static void init_stat_vals(int lvl)
{
   
	gold_total[lvl] = 0.0; gold_floor[lvl] = 0.0; gold_mon[lvl] = 0.0;	gold_wall[lvl] = 0.0;
	
	faeq_total[lvl] = 0.0; faeq_mon[lvl] = 0.0; faeq_vault[lvl] = 0.0;
	reeq_total[lvl] = 0.0; reeq_mon[lvl] = 0.0; reeq_vault[lvl] = 0.0;
	rbeq_total[lvl] = 0.0; rbeq_mon[lvl] = 0.0; rbeq_vault[lvl] = 0.0;
	poeq_total[lvl] = 0.0; poeq_mon[lvl] = 0.0; poeq_vault[lvl] = 0.0;
	nxeq_total[lvl] = 0.0; nxeq_mon[lvl] = 0.0; nxeq_vault[lvl] = 0.0;
	speq_total[lvl] = 0.0; speq_mon[lvl] = 0.0; speq_vault[lvl] = 0.0;
	teeq_total[lvl] = 0.0; teeq_mon[lvl] = 0.0; teeq_vault[lvl] = 0.0;
	bleq_total[lvl] = 0.0; bleq_mon[lvl] = 0.0; bleq_vault[lvl] = 0.0;
	cfeq_total[lvl] = 0.0; cfeq_mon[lvl] = 0.0; cfeq_vault[lvl] = 0.0;
	
	weap_total[lvl] = 0.0; weap_mon[lvl] = 0.0; weap_vault[lvl] = 0.0;
	bdweap_total[lvl] = 0.0; bdweap_mon[lvl] = 0.0; bdweap_vault[lvl] = 0.0;
	avweap_total[lvl] = 0.0; avweap_mon[lvl] = 0.0; avweap_vault[lvl] = 0.0;
	gdweap_total[lvl] = 0.0; gdweap_mon[lvl] = 0.0; gdweap_vault[lvl] = 0.0;
	slweap_total[lvl] = 0.0; slweap_mon[lvl] = 0.0; slweap_vault[lvl] = 0.0;
	evweap_total[lvl] = 0.0; evweap_mon[lvl] = 0.0; evweap_vault[lvl] = 0.0;
	klweap_total[lvl] = 0.0; klweap_mon[lvl] = 0.0; klweap_vault[lvl] = 0.0;
	brweap_total[lvl] = 0.0; brweap_mon[lvl] = 0.0; brweap_vault[lvl] = 0.0;
	weweap_total[lvl] = 0.0; weweap_mon[lvl] = 0.0; weweap_vault[lvl] = 0.0;
	deweap_total[lvl] = 0.0; deweap_mon[lvl] = 0.0; deweap_vault[lvl] = 0.0;
	goweap_total[lvl] = 0.0; goweap_mon[lvl] = 0.0; goweap_vault[lvl] = 0.0;
	haweap_total[lvl] = 0.0; haweap_mon[lvl] = 0.0; haweap_vault[lvl] = 0.0;
	xbweap_total[lvl] = 0.0; xbweap_mon[lvl] = 0.0; xbweap_vault[lvl] = 0.0;
	teweap_total[lvl] = 0.0; teweap_mon[lvl] = 0.0; teweap_vault[lvl] = 0.0;
	huweap_total[lvl] = 0.0; huweap_mon[lvl] = 0.0; huweap_vault[lvl] = 0.0;
	ubweap_total[lvl] = 0.0; ubweap_mon[lvl] = 0.0; ubweap_vault[lvl] = 0.0;
	moweap_total[lvl] = 0.0; moweap_mon[lvl] = 0.0; moweap_vault[lvl] = 0.0;
	
	/*bows*/
	bow_total[lvl] = 0.0; bow_mon[lvl] = 0.0; bow_vault[lvl] = 0.0;
	bdbow_total[lvl] = 0.0; bdbow_mon[lvl] = 0.0; bdbow_vault[lvl] = 0.0;
	avbow_total[lvl] = 0.0; avbow_mon[lvl] = 0.0; avbow_vault[lvl] = 0.0;
	gdbow_total[lvl] = 0.0; gdbow_mon[lvl] = 0.0; gdbow_vault[lvl] = 0.0;
	vgbow_total[lvl] = 0.0; vgbow_mon[lvl] = 0.0; vgbow_vault[lvl] = 0.0;
	xmbow_total[lvl] = 0.0; xmbow_mon[lvl] = 0.0; xmbow_vault[lvl] = 0.0;
	xsbow_total[lvl] = 0.0; xsbow_mon[lvl] = 0.0; xsbow_vault[lvl] = 0.0;
	bubow_total[lvl] = 0.0; bubow_mon[lvl] = 0.0; bubow_vault[lvl] = 0.0;
	tebow_total[lvl] = 0.0; tebow_mon[lvl] = 0.0; tebow_vault[lvl] = 0.0;
	cubow_total[lvl] = 0.0; cubow_mon[lvl] = 0.0; cubow_vault[lvl] = 0.0;
	
	/* ammo */
	ammo_total[lvl] = 0.0; ammo_mon[lvl] = 0.0; ammo_vault[lvl] = 0.0;
	bdammo_total[lvl] = 0.0; bdammo_mon[lvl] = 0.0; bdammo_vault[lvl] = 0.0;
	avammo_total[lvl] = 0.0; avammo_mon[lvl] = 0.0; avammo_vault[lvl] = 0.0;
	gdammo_total[lvl] = 0.0; gdammo_mon[lvl] = 0.0; gdammo_vault[lvl] = 0.0;
	egammo_total[lvl] = 0.0; egammo_mon[lvl] = 0.0; egammo_vault[lvl] = 0.0;
	vgammo_total[lvl] = 0.0; vgammo_mon[lvl] = 0.0; vgammo_vault[lvl] = 0.0;
	awammo_total[lvl] = 0.0; awammo_mon[lvl] = 0.0; awammo_vault[lvl] = 0.0;
	evammo_total[lvl] = 0.0; evammo_mon[lvl] = 0.0; evammo_vault[lvl] = 0.0;
	hmammo_total[lvl] = 0.0; hmammo_mon[lvl] = 0.0; hmammo_vault[lvl] = 0.0;
	
	/* armor */
	arm_total[lvl] = 0.0; arm_mon[lvl] = 0.0; arm_vault[lvl] = 0.0;
	bdarm_total[lvl] = 0.0; bdarm_mon[lvl] = 0.0; bdarm_vault[lvl] = 0.0;
	avarm_total[lvl] = 0.0; avarm_mon[lvl] = 0.0; avarm_vault[lvl] = 0.0;
	gdarm_total[lvl] = 0.0; gdarm_mon[lvl] = 0.0; gdarm_vault[lvl] = 0.0;
	strarm_total[lvl] = 0.0; strarm_mon[lvl] = 0.0; strarm_vault[lvl] = 0.0;
	intarm_total[lvl] = 0.0; intarm_mon[lvl] = 0.0; intarm_vault[lvl] = 0.0;
	wisarm_total[lvl] = 0.0; wisarm_mon[lvl] = 0.0; wisarm_vault[lvl] = 0.0;
	dexarm_total[lvl] = 0.0; dexarm_mon[lvl] = 0.0; dexarm_vault[lvl] = 0.0;
	conarm_total[lvl] = 0.0; conarm_mon[lvl] = 0.0; conarm_vault[lvl] = 0.0;
	cuarm_total[lvl] = 0.0; cuarm_mon[lvl] = 0.0; cuarm_vault[lvl] = 0.0;

	art_total[lvl] = 0.0; art_spec[lvl] = 0.0; art_norm[lvl] = 0.0;
	art_shal[lvl] = 0.0; art_ave [lvl] = 0.0;	art_ood [lvl] = 0.0;
	art_mon[lvl] = 0.0; art_uniq[lvl] = 0.0; art_floor[lvl] = 0.0;
	art_vault[lvl] = 0.0; art_mon_vault[lvl] = 0.0;
	
	/* potion */
	pot_total[lvl] =0.0; pot_mon[lvl] =0.0;	pot_vault[lvl] =0.0;	
	gain_total[lvl] =0.0; gain_mon[lvl] =0.0; gain_vault[lvl] =0.0;	
	rmana_total[lvl] =0.0; rmana_mon[lvl] =0.0;	rmana_vault[lvl] =0.0;	
	bigheal_total[lvl] =0.0; bigheal_mon[lvl] =0.0; bigheal_vault[lvl] =0.0;
	
	/*scrolls*/
	scroll_total[lvl] = 0.0;	scroll_mon[lvl] = 0.0; scroll_vault[lvl] = 0.0;	
	escroll_total[lvl] = 0.0; escroll_mon[lvl] = 0.0; escroll_vault[lvl] = 0.0;
	acq_total[lvl] = 0.0; acq_mon[lvl] = 0.0; acq_vault[lvl] = 0.0;
	
	/* rods */
	rod_total[lvl] = 0.0; rod_mon[lvl] = 0.0; rod_vault[lvl] = 0.0;
	urod_total[lvl] = 0.0; urod_mon[lvl] = 0.0; urod_vault[lvl] = 0.0;	
	torod_total[lvl] = 0.0; torod_mon[lvl] = 0.0; torod_vault[lvl] = 0.0;	
	drod_total[lvl] = 0.0; drod_mon[lvl] = 0.0; drod_vault[lvl] = 0.0;	
	erod_total[lvl] = 0.0; erod_mon[lvl] = 0.0; erod_vault[lvl] = 0.0;
	
	/* staves */
	staff_total[lvl] = 0.0; staff_mon[lvl] = 0.0; staff_vault[lvl] = 0.0;	
	sstaff_total[lvl] = 0.0; sstaff_mon[lvl] = 0.0; sstaff_vault[lvl] = 0.0;	
	dstaff_total[lvl] = 0.0; dstaff_mon[lvl] = 0.0; dstaff_vault[lvl] = 0.0;	
	kstaff_total[lvl] = 0.0; kstaff_mon[lvl] = 0.0; kstaff_vault[lvl] = 0.0;	
	pstaff_total[lvl] = 0.0; pstaff_mon[lvl] = 0.0; pstaff_vault[lvl] = 0.0;
	
	/* wands */
	wand_total[lvl] = 0.0; wand_mon[lvl] = 0.0; wand_vault[lvl] = 0.0;	
	towand_total[lvl] = 0.0; towand_mon[lvl] = 0.0; towand_vault[lvl] = 0.0;
	
	/* rings */
	ring_total[lvl] = 0.0; ring_mon[lvl] = 0.0; ring_vault[lvl] = 0.0;	
	curing_total[lvl] = 0.0; curing_mon[lvl] = 0.0; curing_vault[lvl] = 0.0;	
	spring_total[lvl] = 0.0; spring_mon[lvl] = 0.0; spring_vault[lvl] = 0.0;	
	string_total[lvl] = 0.0; string_mon[lvl] = 0.0; string_vault[lvl] = 0.0;	
	faring_total[lvl] = 0.0; faring_mon[lvl] = 0.0; faring_vault[lvl] = 0.0;	
	siring_total[lvl] = 0.0; siring_mon[lvl] = 0.0; siring_vault[lvl] = 0.0;	
	poring_total[lvl] = 0.0; poring_mon[lvl] = 0.0; poring_vault[lvl] = 0.0;	
	brring_total[lvl] = 0.0; brring_mon[lvl] = 0.0; brring_vault[lvl] = 0.0;	
	elring_total[lvl] = 0.0; elring_mon[lvl] = 0.0; elring_vault[lvl] = 0.0;	
	onering_total[lvl] = 0.0; onering_mon[lvl] = 0.0; onering_vault[lvl] = 0.0;
	
	/* amulets */
	amu_total[lvl] = 0.0; amu_mon[lvl] = 0.0; amu_vault[lvl] = 0.0;
	wisamu_total[lvl] = 0.0; wisamu_mon[lvl] = 0.0; wisamu_vault[lvl] = 0.0;
	endamu_total[lvl] = 0.0; endamu_mon[lvl] = 0.0; endamu_vault[lvl] = 0.0;
	teamu_total[lvl] = 0.0; teamu_mon[lvl] = 0.0; teamu_vault[lvl] = 0.0;
	cuamu_total[lvl] = 0.0; cuamu_mon[lvl] = 0.0; cuamu_vault[lvl] = 0.0;
	
	mon_total[lvl] = 0.0; mon_ood[lvl] = 0.0; mon_deadly[lvl] = 0.0;
	
	uniq_total[lvl] = 0.0; uniq_ood[lvl] = 0.0; uniq_deadly[lvl] = 0.0;
	
	/* books */
	b1_total[lvl] =0.0; b1_mon[lvl] =0.0; b1_vault[lvl] =0.0;
	b2_total[lvl] =0.0; b2_mon[lvl] =0.0; b2_vault[lvl] =0.0;
	b3_total[lvl] =0.0; b3_mon[lvl] =0.0; b3_vault[lvl] =0.0;
	b4_total[lvl] =0.0; b4_mon[lvl] =0.0; b4_vault[lvl] =0.0;
	b5_total[lvl] =0.0; b5_mon[lvl] =0.0; b5_vault[lvl] =0.0;
	b6_total[lvl] =0.0; b6_mon[lvl] =0.0; b6_vault[lvl] =0.0;
	b7_total[lvl] =0.0; b7_mon[lvl] =0.0; b7_vault[lvl] =0.0;
	b8_total[lvl] =0.0; b8_mon[lvl] =0.0; b8_vault[lvl] =0.0;
	b9_total[lvl] =0.0; b9_mon[lvl] =0.0; b9_vault[lvl] =0.0;
		
}



void drop_on_square(object_type *j_ptr, int y, int x, bool verbose)
{
	//int i, k, n, d, s;

	int by, bx;
	//int dy, dx;
	//int ty, tx;

	// object_type *o_ptr;

	char o_name[80];

	//bool flag = FALSE;

	bool plural = FALSE;
	
	/* Default */
	by = y;
	bx = x;
	
	/* Set floor to empty */
	cave_set_feat(cave,y,x,FEAT_FLOOR);

			
			
	/* Give it to the floor */
	if (!floor_carry(cave, by, bx, j_ptr))
	{
		/* Message */
		msg("The %s disappear%s.", o_name, PLURAL(plural));

		/* Debug */
		if (p_ptr->wizard) msg("Breakage (too many objects).");

		if (j_ptr->artifact) j_ptr->artifact->created = FALSE;

		/* Failure */
		return;
	}
}
/* 
 * A rewrite of monster death that gets rid of some features
 * That we don't want to deal with.  Namely, no notifying the
 * player and no generation of Morgoth artifacts
 * 
 * It also replaces drop near with a new function that drops all 
 * the items on the exact square that the monster was on.
 */

static void monster_death_stats(int m_idx)
{
	int j, y, x, level; //removed i

	int dump_item = 0;
	int dump_gold = 0;

	int number = 0;
	
	object_type *i_ptr;
	object_type object_type_body;

	//s16b this_o_idx, next_o_idx = 0;

	monster_type *m_ptr = cave_monster(cave, m_idx);

	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	//bool visible = (m_ptr->ml || rf_has(r_ptr->flags, RF_UNIQUE));

	bool great = (rf_has(r_ptr->flags, RF_DROP_GREAT)) ? TRUE : FALSE;
	bool good = (rf_has(r_ptr->flags, RF_DROP_GOOD) ? TRUE : FALSE) || great;

	bool gold_ok = (!rf_has(r_ptr->flags, RF_ONLY_ITEM));
	bool item_ok = (!rf_has(r_ptr->flags, RF_ONLY_GOLD));

	/* This is the get_coin_type function moved inline */
	const char *name = r_ptr->name;

	int force_coin=SV_GOLD_ANY;

	/* Look for textual clues */
	if (my_stristr(name, "copper "))	force_coin=SV_COPPER;
	if (my_stristr(name, "silver "))	force_coin=SV_SILVER;
	if (my_stristr(name, "gold "))		force_coin=SV_GOLD;
	if (my_stristr(name, "mithril "))	force_coin=SV_MITHRIL;
	if (my_stristr(name, "adamantite "))	force_coin=SV_ADAMANTITE;


	


	/* Get the location */
	y = m_ptr->fy;
	x = m_ptr->fx;

	/* Delete any traps the monsters was standing on */
	cave_set_feat(cave,y,x,FEAT_FLOOR);
	
	/* Forget objects */
	m_ptr->hold_o_idx = 0;


	/* Determine how much we can drop */
	if (rf_has(r_ptr->flags, RF_DROP_20) && randint0(100) < 20) number++;
	if (rf_has(r_ptr->flags, RF_DROP_40) && randint0(100) < 40) number++;
	if (rf_has(r_ptr->flags, RF_DROP_60) && randint0(100) < 60) number++;

	if (rf_has(r_ptr->flags, RF_DROP_4)) number += rand_range(2, 6);
	if (rf_has(r_ptr->flags, RF_DROP_3)) number += rand_range(2, 4);
	if (rf_has(r_ptr->flags, RF_DROP_2)) number += rand_range(1, 3);
	if (rf_has(r_ptr->flags, RF_DROP_1)) number++;

	/* Take the best of average of monster level and current depth,
	   and monster level - to reward fighting OOD monsters */
	level = MAX((r_ptr->level + p_ptr->depth) / 2, r_ptr->level);

	/* Drop some objects */
	for (j = 0; j < number; j++)
	{
		/* Get local object */
		i_ptr = &object_type_body;

		/* Wipe the object */
		object_wipe(i_ptr);

		/* Make Gold */
		if (gold_ok && (!item_ok || (randint0(100) < 50)))
		{
			/* Make some gold */
			make_gold(i_ptr, level, force_coin);
			dump_gold++;
		}

		/* Make Object */
		else
		{
			/* Make an object */
			if (!make_object(cave, i_ptr, level, good, great)) continue;
			dump_item++;
		}

		

		/* Drop it in the dungeon */
		drop_near(cave, i_ptr, 0,y, x, TRUE);
	}

} 




/* This will collect stats on a monster avoiding all
 * unique monsters.  Afterwards it will kill the
 * monsters.
 */

static bool stats_monster(monster_type *m_ptr, int i, bool uniq)
{
	static int lvl;
	/* Get monster race */
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
		
	/* get player depth */
	lvl=p_ptr->depth;
		
	
	/* Get out if we're looking at a unique and don't want to yet*/
	if ((!uniq) && (rf_has(r_ptr->flags, RF_UNIQUE))) return FALSE;
		
	/* Increment monster count */
	mon_total[lvl] += addval;
	
	/* Increment unique count if appropriate */
	if ((uniq) && (rf_has(r_ptr->flags, RF_UNIQUE)))
	{
		/* add to total */
		uniq_total[lvl] += addval;
	
		/* kill the unique if we're in clearing mode */
		if (clearing) r_ptr->max_num = 0;
		
		//debugging print that we killed it
		//msg_format("Killed %s",r_ptr->name);
	}	
	
	/* Is it mostly dangerous (10 levels ood or less?)*/
	if ((r_ptr->level > p_ptr->depth) && 
		(r_ptr->level <= p_ptr->depth+10))
		{
			mon_ood[lvl] += addval;
			
			/* Is it a unique */
			if (rf_has(r_ptr->flags, RF_UNIQUE)) uniq_ood[lvl] += addval;
			
		}
		
		
	/* Is it deadly? */
	if (r_ptr->level > p_ptr->depth + 10)
	{
		mon_deadly[lvl] += addval;
	
		/* Is it a unique? */
		if (rf_has(r_ptr->flags, RF_UNIQUE)) uniq_deadly[lvl] += addval;
						
	}	
	
	/* Generate treasure */
	monster_death_stats(i);	
	
	/* remove the monster */
	delete_monster_idx(i);
	
	/* success */
	return TRUE;
}


/*
 * Delete a single dungeon object
 *
 * This piece of code is identical to delete_object_idx
 * except that it does not include light_spot to save
 * time
 */
static void delete_object_stat(int o_idx)
{
	object_type *j_ptr;

	/* Excise */
	excise_object_idx(o_idx);

	/* Object */
	j_ptr = object_byid(o_idx);

	
	/* Wipe the object */
	object_wipe(j_ptr);

	/* Count objects */
	o_cnt--;
}

/*
 *	Record the first level we find something
 */
static bool first_find(int fl[TRIES_SIZE])
{
	/* make sure we're not on an iteration above our array limit */
	if (iter >= TRIES_SIZE) return FALSE;

	/* make sure we haven't found it earlier on this iteration */
	if (fl[iter] > 0) return FALSE;

	/* assign the depth to this value */
	fl[iter] = p_ptr->depth;
	
	/* success */
	return TRUE;
}

/*
 * Add values to each category of statistics based 
 */
static void add_stats(double total[MAX_LVL], double mondrop[MAX_LVL], double invault[MAX_LVL],
			bool vault, bool mon, int number)
{
		int lvl;
		
		/* get player level */
		lvl=p_ptr->depth;

		/* be careful about bounds */
		if ((lvl > 100) || (lvl < 0)) return;
		
		/* increase values */
		//total += lvl; mondrop +=lvl; invault+=lvl;
		
		/* add to the total */
		total[lvl] += addval*number;
		
		/* add to the total from vaults */
		if ((!mon) && (vault)) invault[lvl] += addval * number;
	
		/* add to the total from monsters */
		if (mon) mondrop[lvl] += addval * number;

}

/*
 * This will get data on an object
 * It gets a lot of stuff, pretty much everything that I
 * thought was reasonable to get.  However, you might have
 * a much different opinion.  Luckily, I tried to make it
 * trivial to add new items to log.
*/ 

static void get_obj_data(const object_type *o_ptr, int y, int x, bool mon, bool uniq)
{
	
	bool vault = (cave->info[y][x] & (CAVE_ICKY));
	bitflag f[OF_SIZE];
	int effect;
	int number = o_ptr->number;
	static int lvl;
	artifact_type *a_ptr;
	
	double gold_temp=0;
	
	/* get player depth */
	lvl=p_ptr->depth;	
	
	/* extract flags */
	object_flags(o_ptr,f);
	
	/* check for some stuff that we will use regardless of type */
	/* originally this was armor, but I decided to generalize it */
	
	/* has free action (hack: don't include Inertia)*/
	if (of_has(f,OF_FREE_ACT) && 
		!((o_ptr->tval == TV_AMULET) && (o_ptr->sval==SV_AMULET_INERTIA)))
		{
			/* add the stats */
			add_stats( faeq_total,  faeq_mon,  faeq_vault,vault,mon,number);
			
			/* record first level */
			first_find(fa_it);
		}
	
	
	/* has see invis */
	if (of_has(f,OF_SEE_INVIS))
	{
		add_stats( sieq_total,  sieq_mon,  sieq_vault,vault,mon,number);
		first_find(si_it);
	}
	/* has at least one basic resist */
 	if ((of_has(f,OF_RES_ACID)) ||
		(of_has(f,OF_RES_ELEC)) ||
		(of_has(f,OF_RES_COLD)) ||
		(of_has(f,OF_RES_FIRE)))
	{
		add_stats( reeq_total,  reeq_mon,  reeq_vault,vault,mon,number);
	}
	/* has rbase */
	if ((of_has(f,OF_RES_ACID)) &&
		(of_has(f,OF_RES_ELEC)) &&
		(of_has(f,OF_RES_COLD)) &&
		(of_has(f,OF_RES_FIRE)))
		add_stats( rbeq_total,  rbeq_mon,  rbeq_vault,vault,mon,number);

	/* has resist poison */
	if (of_has(f,OF_RES_POIS))
	{
		add_stats( poeq_total,  poeq_mon,  poeq_vault,vault,mon,number);
		first_find(po_it);
	}
	/* has resist nexus */
	if (of_has(f,OF_RES_NEXUS))
	{
		add_stats( nxeq_total,  nxeq_mon,  nxeq_vault,vault,mon,number);
		first_find(nx_it);
	}
	/* has resist blind */
	if (of_has(f,OF_RES_BLIND))
	{
		add_stats( bleq_total,  bleq_mon,  bleq_vault,vault,mon,number);
		first_find(bl_it);
	}
	
	/* has resist conf */
	if (of_has(f,OF_RES_CONFU))
	{
		add_stats( cfeq_total,  cfeq_mon,  cfeq_vault,vault,mon,number);	
		first_find(cf_it);
	}
	
	/* has speed */
	if (of_has(f,OF_SPEED))
		add_stats( speq_total,  speq_mon,  speq_vault,vault,mon,number);
				
	/* has telepathy */
	if (of_has(f,OF_TELEPATHY))
	{
		add_stats( teeq_total,  teeq_mon,  teeq_vault,vault,mon,number);
		first_find(te_it);
	}
	
	switch(o_ptr->tval)
	{
		/* armor */
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_HELM:
		case TV_CROWN:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		{	
			/* do not include artifacts */
			if (o_ptr->artifact) break;
			
			/* add to armor total */
			add_stats( arm_total,  arm_mon,  arm_vault,vault,mon,number);
			
			/* check if bad, good, or average */
			if (o_ptr->to_a < 0)
				add_stats( bdarm_total,  bdarm_mon,  bdarm_vault, vault, mon,number);
			if (o_ptr->to_h == 0)
				add_stats( avarm_total,  avarm_mon,  avarm_vault, vault, mon,number);	
			if (o_ptr->to_h > 0)
				add_stats( gdarm_total,  gdarm_mon,  gdarm_vault, vault, mon,number);
						
			/* has str boost */
			if (of_has(f,OF_STR))
				add_stats( strarm_total,  strarm_mon,  strarm_vault,vault,mon,number);
			
			/* has dex boost */
			if (of_has(f,OF_DEX))
				add_stats( dexarm_total,  dexarm_mon,  dexarm_vault,vault,mon,number);
				
			/* has int boost */
			if (of_has(f,OF_INT))
				add_stats( intarm_total,  intarm_mon,  intarm_vault,vault,mon,number);

			if (of_has(f,OF_WIS))
				add_stats( wisarm_total,  wisarm_mon,  wisarm_vault,vault,mon,number);

			if (of_has(f,OF_CON))
				add_stats( conarm_total,  conarm_mon,  conarm_vault,vault,mon,number);
				
			if (of_has(f,OF_LIGHT_CURSE))
				add_stats( cuarm_total,  cuarm_mon,  cuarm_vault,vault,mon,number);
			
			break;
		}
	
		/* weapons */
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		{
			/* do not include artifacts */
			if (o_ptr->artifact) break;
				
			/* add to weapon total */
			add_stats( weap_total,  weap_mon,  weap_vault, vault, mon,number);
			
			/* extract flags */
			object_flags(o_ptr,f);
			
			/* check if bad, good, or average */
			if ((o_ptr->to_h < 0)  && (o_ptr->to_d < 0))
				add_stats( bdweap_total,  bdweap_mon,  bdweap_vault, vault, mon,number);
			if ((o_ptr->to_h == 0) && (o_ptr->to_d == 0))
				add_stats( avweap_total,  avweap_mon,  avweap_vault, vault, mon,number);	
			if ((o_ptr->to_h > 0) && (o_ptr->to_d > 0))
				add_stats( gdweap_total,  gdweap_mon,  gdweap_vault, vault, mon,number);
			
			/* slay weapons */
			if ((of_has(f,OF_SLAY_DRAGON)) ||
				(of_has(f,OF_SLAY_DEMON)) ||
				(of_has(f,OF_SLAY_TROLL)) ||
				(of_has(f,OF_SLAY_GIANT)) ||
				(of_has(f,OF_SLAY_UNDEAD)) ||
				(of_has(f,OF_SLAY_ANIMAL)) ||
				(of_has(f,OF_SLAY_ORC)))
					add_stats( slweap_total,  slweap_mon,  slweap_vault, vault, mon,number);
			/* slay evil */
			if (of_has(f,OF_SLAY_EVIL))
				add_stats( slweap_total,  slweap_mon,  slweap_vault, vault, mon,number);
			
			/* kill flag */
			if ((of_has(f,OF_KILL_DRAGON)) ||
				(of_has(f,OF_KILL_DEMON)) ||
				(of_has(f,OF_KILL_UNDEAD)))
					add_stats( klweap_total,  klweap_mon,  klweap_vault, vault, mon,number);
				
			/* branded weapons */
			if ((of_has(f,OF_BRAND_ACID)) ||
				(of_has(f,OF_BRAND_ELEC)) ||
				(of_has(f,OF_BRAND_FIRE)) ||
				(of_has(f,OF_BRAND_COLD)) ||
				(of_has(f,OF_BRAND_POIS)))
					add_stats( brweap_total,  brweap_mon,  brweap_vault, vault, mon,number);
			
			/* determine westernesse by flags */
			if ((of_has(f,OF_STR)) &&
				(of_has(f,OF_DEX)) &&
				(of_has(f,OF_CON)) &&
				(of_has(f,OF_FREE_ACT)) &&
				(of_has(f,OF_SEE_INVIS)) &&
				(of_has(f,OF_SLAY_TROLL)) &&
				(of_has(f,OF_SLAY_GIANT)) &&
				(of_has(f,OF_SLAY_ORC)))
					add_stats( weweap_total,  weweap_mon,  weweap_vault, vault, mon,number);
					
			/* determine defender by flags */
			if ((of_has(f,OF_RES_ACID)) &&
				(of_has(f,OF_RES_ELEC)) &&
				(of_has(f,OF_RES_FIRE)) &&
				(of_has(f,OF_RES_COLD)) &&
				(of_has(f,OF_FREE_ACT)) &&
				(of_has(f,OF_SEE_INVIS)) &&
				(of_has(f,OF_FEATHER)) &&
				(of_has(f,OF_STEALTH)) &&
				(of_has(f,OF_REGEN)))
					add_stats( deweap_total,  deweap_mon,  deweap_vault, vault, mon,number);
			
			/* determine gondolin by flags */
			if ((of_has(f,OF_SLAY_DEMON)) &&
				(of_has(f,OF_SLAY_ORC)) &&
				(of_has(f,OF_SLAY_TROLL)) &&
				(of_has(f,OF_SLAY_DRAGON)) &&
				(of_has(f,OF_FREE_ACT)) &&
				(of_has(f,OF_SEE_INVIS)) &&
				(of_has(f,OF_LIGHT)) &&
				(of_has(f,OF_RES_DARK)))
					add_stats( goweap_total,  goweap_mon,  goweap_vault, vault, mon,number);
					
			/* determine holy avenger by flags */
			if ((of_has(f,OF_SLAY_EVIL)) &&
				(of_has(f,OF_SLAY_UNDEAD)) &&
				(of_has(f,OF_SLAY_DEMON)) &&
				(of_has(f,OF_SEE_INVIS)) &&
				(of_has(f,OF_BLESSED)) &&
				(of_has(f,OF_RES_FEAR)))
					add_stats( haweap_total,  haweap_mon,  haweap_vault, vault, mon,number);
					
			/* extra blows */
			if (of_has(f,OF_BLOWS))
				add_stats( xbweap_total,  xbweap_mon,  xbweap_vault, vault, mon,number);
				
			/* telepathy */	
			if (of_has(f,OF_TELEPATHY))
				add_stats( teweap_total,  teweap_mon,  teweap_vault, vault, mon,number);
			
			/* is a top of the line weapon */
			if (((o_ptr->tval == TV_HAFTED) && (o_ptr->sval == SV_MACE_OF_DISRUPTION)) ||
				((o_ptr->tval == TV_POLEARM) && (o_ptr->sval == SV_SCYTHE_OF_SLICING)) ||
				((o_ptr->tval == TV_SWORD) && (o_ptr->sval == SV_BLADE_OF_CHAOS)))
			{
				add_stats( huweap_total,  huweap_mon,  huweap_vault, vault, mon,number);
			
				/* is uber */
				if ((of_has(f,OF_SLAY_EVIL)) || (of_has(f,OF_BLOWS)))
					add_stats( ubweap_total,  ubweap_mon,  ubweap_vault, vault, mon,number);
			
			}
			
			/* is morgul */
			if (of_has(f,OF_HEAVY_CURSE))
				add_stats( moweap_total,  moweap_mon,  moweap_vault, vault, mon,number);
			
			break;
		}
		
		/* launchers */
		case TV_BOW:
		{
			/* do not include artifacts */
			if (o_ptr->artifact) break;
			
			/* add to launcher total */
			add_stats( bow_total,  bow_mon,  bow_vault, vault, mon,number);
			
			/* extract flags */
			object_flags(o_ptr,f);
			
			/* check if bad, average, good, or very good */
			if ((o_ptr->to_h < 0) && (o_ptr->to_d < 0))
				add_stats( bdbow_total,  bdbow_mon,  bdbow_vault, vault, mon,number);
			if ((o_ptr->to_h == 0) && (o_ptr->to_d == 0))
				add_stats( avbow_total,  avbow_mon,  avbow_vault, vault, mon,number);	
			if ((o_ptr->to_h > 0) && (o_ptr->to_d > 0))
				add_stats( gdbow_total,  gdbow_mon,  gdbow_vault, vault, mon,number);
			if ((o_ptr->to_h > 15) || (o_ptr->to_d > 15))
				add_stats( vgbow_total,  vgbow_mon,  vgbow_vault, vault, mon,number);
		
			/* check long bows and xbows for xtra might and/or shots */
			if ((o_ptr->sval == SV_LONG_BOW) ||
				(o_ptr->sval == SV_LIGHT_XBOW) ||
				(o_ptr->sval == SV_HEAVY_XBOW))
			{	
				if (of_has(f,OF_SHOTS))
					add_stats( xsbow_total,  xsbow_mon,  xsbow_vault, vault, mon,number);
					
				if (of_has(f,OF_MIGHT))
					add_stats( xmbow_total,  xmbow_mon,  xmbow_vault, vault, mon,number);
			}
			
			/* check for buckland */
			if ((o_ptr->sval == SV_SLING) &&
				(of_has(f,OF_MIGHT)) &&
				(of_has(f,OF_SHOTS)))
					add_stats( bubow_total,  bubow_mon,  bubow_vault, vault, mon,number);
					
			/* has telep */
			if (of_has(f,OF_TELEPATHY))
				add_stats( tebow_total,  tebow_mon,  tebow_vault, vault, mon,number);
				
			/* is cursed */
			if (of_has(f,OF_LIGHT_CURSE))
				add_stats( cubow_total,  cubow_mon,  cubow_vault, vault, mon,number);
			break;
		}
		/* potion */
		case TV_POTION:
		{
			/* add total amounts */
			add_stats( pot_total, pot_mon, pot_vault,vault,mon,number);
			
			/* get effects */
			effect=object_effect(o_ptr);
			 
			/*stat gain*/
			switch(effect)
			{
				/* skip CHR */
				case EF_GAIN_STR:
				case EF_GAIN_INT:
				case EF_GAIN_WIS:
				case EF_GAIN_DEX:
				case EF_GAIN_CON:
				{
					add_stats( gain_total,  gain_mon,  gain_vault,vault,mon,number);
					break;
				}
				
				/* Aug */
				case EF_GAIN_ALL:
				{
					int k;
					/*Augmentation counts as 5 stat gain pots */
					for (k=1;k<=5;k++)
						add_stats( gain_total,  gain_mon,  gain_vault,vault,mon,number);
					break;	
				}
				
				case EF_ENLIGHTENMENT2:
				{
					/* *Enlight* counts as 2 stat pots */
					int k;
					for (k=1;k<=2;k++) 
						add_stats( gain_total,  gain_mon,  gain_vault,vault,mon,number);
					break;
				}
				
				case EF_RESTORE_MANA:
				{	
					add_stats( rmana_total,  rmana_mon,  rmana_vault,vault,mon,number);
					break;
				}
				
				case EF_CURE_NONORLYBIG:
				case EF_CURE_FULL2:
				{
					add_stats( bigheal_total,  bigheal_mon,  bigheal_vault,vault,mon,number);
					break;
				}
								
			}
			break;
		}
		
		/* scrolls */
		case TV_SCROLL:
		{
			/* add total amounts */
			add_stats( scroll_total, scroll_mon, scroll_vault,vault,mon,number);
			
			/* get effects */
			effect=object_effect(o_ptr);
			
			/* scroll effects */
			switch(effect)
			{
				case EF_BANISHMENT:
				case EF_LOSKILL:
				case EF_RUNE:
				case EF_DESTRUCTION2:
				{
					/* add to total */
					add_stats( escroll_total, escroll_mon, escroll_vault,vault,mon,number);
					break;
				}
				
				case EF_ACQUIRE:
				{
					/* add to total */
					add_stats( acq_total, acq_mon, acq_vault,vault,mon,number);
					break;
				}
				
				case EF_ACQUIRE2:
				{
					/* do the effect of 2 acquires */
					add_stats( acq_total, acq_mon, acq_vault,vault,mon,number);
					add_stats( acq_total, acq_mon, acq_vault,vault,mon,number);
					break;
				}
			}
			break;
		}
		
		/* rods */
		case TV_ROD:
		{
			/* add to total */
			add_stats( rod_total,  rod_mon,  rod_vault,vault,mon,number);
			
			effect=object_effect(o_ptr);
			
			switch(effect)
			{
			
				/* utility */
				case EF_DETECT_TRAP:
				case EF_DETECT_TREASURE:
				case EF_DETECT_DOORSTAIR:
				case EF_LIGHT_LINE:
				case EF_ILLUMINATION:
				{
					add_stats( urod_total,  urod_mon,  urod_vault,vault,mon,number);
					break;
				}
			
				/* teleport other */
				case EF_TELE_OTHER:
				{
					add_stats( torod_total,  torod_mon,  torod_vault,vault,mon,number);
					break;
				}
				
				/* detect all */
				case EF_DETECT_ALL:
				{
					add_stats( drod_total,  drod_mon,  drod_vault,vault,mon,number);
					break;
				}
			
				/* endgame, speed and healing */
				case EF_HASTE:
				case EF_HEAL3:
				{
					add_stats( erod_total,  erod_mon,  erod_vault,vault,mon,number);
					break;
				}
			}
			
			break;
		}
	
		/* staves */
		case TV_STAFF:
		{
			add_stats( staff_total,  staff_mon,  staff_vault,vault,mon,number);
			
			effect=object_effect(o_ptr);
			
			switch(effect)
			{
				case EF_HASTE:
				{
					add_stats( sstaff_total,  sstaff_mon,  sstaff_vault,vault,mon,number);
					break;
				}
				
				case EF_DESTRUCTION2:
				{
					add_stats( dstaff_total,  dstaff_mon,  dstaff_vault,vault,mon,number);
					break;
				}
			
				case EF_DISPEL_EVIL60:
				case EF_DISPEL_ALL:
				case EF_STAFF_HOLY:
				{
					add_stats( kstaff_total,  kstaff_mon,  kstaff_vault,vault,mon,number);
					break;
				}
				
				case EF_CURE_FULL:
				case EF_BANISHMENT:
				case EF_STAFF_MAGI:
				{
					add_stats( pstaff_total,  pstaff_mon,  pstaff_vault,vault,mon,number);
					break;
				}
			}			
			break;
		}
		
		case TV_WAND:
		{
			add_stats( wand_total,  wand_mon,  wand_vault,vault,mon,number);
			
			effect=object_effect(o_ptr);
			
			switch(effect)
			{
				case EF_TELE_OTHER:
				{
					add_stats( towand_total,  towand_mon,  towand_vault,vault,mon,number);
					break;
				}
			}
			break;
		}
	
		case TV_RING:
		{
			add_stats( ring_total,  ring_mon,  ring_vault,vault,mon,number);
			
			/* is it cursed */
			if (of_has(o_ptr->flags,OF_LIGHT_CURSE))
				add_stats( curing_total,  curing_mon,  curing_vault,vault,mon,number);
			
			switch(o_ptr->sval)
			{
				case SV_RING_SPEED:
				{
					add_stats( spring_total,  spring_mon,  spring_vault,vault,mon,number);
					break;
				}
				
				case SV_RING_STRENGTH:
				case SV_RING_INTELLIGENCE:
				case SV_RING_DEXTERITY:
				case SV_RING_CONSTITUTION:
				{
					add_stats( string_total,  string_mon,  string_vault,vault,mon,number);
					break;
				}
				
				case SV_RING_RESIST_POISON:
				{
					add_stats( poring_total,  poring_mon,  poring_vault,vault,mon,number);
					break;
				}
				
				case SV_RING_FREE_ACTION:
				{
					add_stats( faring_total,  faring_mon,  faring_vault,vault,mon,number);
					break;
				}
				
				case SV_RING_SEE_INVISIBLE:
				{
					add_stats( siring_total,  siring_mon,  siring_vault,vault,mon,number);
					break;
				}
				
				case SV_RING_FLAMES:
				case SV_RING_ACID:
				case SV_RING_ICE:
				case SV_RING_LIGHTNING:
				{
					add_stats( brring_total,  brring_mon,  brring_vault,vault,mon,number);
					break;
				}
				
				case SV_RING_NARYA:
				case SV_RING_NENYA:
				case SV_RING_VILYA:
				{
					add_stats( elring_total,  elring_mon,  elring_vault,vault,mon,number);
					break;
				}
				
				case SV_RING_POWER:
				{
					add_stats( onering_total,  onering_mon,  onering_vault,vault,mon,number);
					break;
				}
				
				
			}
		
		break;
		}
		
		case TV_AMULET:
		{
			add_stats( amu_total,  amu_mon,  amu_vault,vault,mon,number);
			
			/* extract flags */
			object_flags(o_ptr,f);
			
			switch(o_ptr->sval)
			{
				/* wisdom */
				case SV_AMULET_WISDOM:
				{
					add_stats( wisamu_total,  wisamu_mon,  wisamu_vault,vault,mon,number);
					break;
				}
				
				case SV_AMULET_THE_MAGI:
				case SV_AMULET_TRICKERY:
				case SV_AMULET_WEAPONMASTERY:
				{
					add_stats( endamu_total,  endamu_mon,  endamu_vault,vault,mon,number);
					break;
				}
				
				case SV_AMULET_ESP:
				{
					add_stats( teamu_total,  teamu_mon,  teamu_vault,vault,mon,number);
					break;
				}
				
			}
			/* is cursed */
			if (of_has(f,OF_LIGHT_CURSE))
				add_stats( cuamu_total,  cuamu_mon,  cuamu_vault, vault, mon,number);
			
			break;
		}
	
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
		{
			add_stats( ammo_total,  ammo_mon,  ammo_vault,vault,mon,number);
			
			/* extract flags */
			object_flags(o_ptr,f);
			
			/* check if bad, average, good */
			if ((o_ptr->to_h < 0) && (o_ptr->to_d < 0))
				add_stats( bdammo_total,  bdammo_mon,  bdammo_vault, vault, mon,number);
			if ((o_ptr->to_h == 0) && (o_ptr->to_d == 0))
				add_stats( avammo_total,  avammo_mon,  avammo_vault, vault, mon,number);	
			if ((o_ptr->to_h > 0) && (o_ptr->to_d > 0))
				add_stats( gdammo_total,  gdammo_mon,  gdammo_vault, vault, mon,number);
				
			if (o_ptr->ego)
				add_stats( egammo_total,  egammo_mon,  egammo_vault, vault, mon,number);
		
			if ((o_ptr->sval == SV_AMMO_HEAVY) || (o_ptr->sval == SV_AMMO_SILVER))
			{
				/* Mithril and seeker ammo */
				add_stats( vgammo_total,  vgammo_mon,  vgammo_vault, vault, mon,number);
				
				/* Ego mithril and seeker ammo */
				if (o_ptr->ego)
					add_stats( awammo_total,  awammo_mon,  awammo_vault, vault, mon,number);
			
				if (of_has(f,OF_SLAY_EVIL))
					add_stats( evammo_total,  evammo_mon,  evammo_vault, vault, mon,number);
					
				if ((of_has(f,OF_SLAY_EVIL)) && (of_has(f,OF_SLAY_DEMON)))
					add_stats( hmammo_total,  hmammo_mon,  hmammo_vault, vault, mon,number);
			}
			break;
		}
		
		/* prayer books and magic books have the same probability 
		   only track one of them */
		case TV_MAGIC_BOOK:
		{
			switch(o_ptr->sval)
			{
				/* svals begin at 0 and end at 8 */
				case 0:
				{
					add_stats( b1_total,  b1_mon,  b1_vault, vault, mon,number);
					first_find(mb1_it);
					break;
				}
				
				case 1:
				{
					add_stats( b2_total,  b2_mon,  b2_vault, vault, mon,number);
					first_find(mb2_it);
					break;
				}
			
				case 2:
				{
					add_stats( b3_total,  b3_mon,  b3_vault, vault, mon,number);
					first_find(mb3_it);
					break;
				}
				
				case 3:
				{
					add_stats( b4_total,  b4_mon,  b4_vault, vault, mon,number);
					first_find(mb4_it);
					break;
				}
				
				case 4:
				{
					add_stats( b5_total,  b5_mon,  b5_vault, vault, mon,number);
					first_find(mb5_it);
					break;
				}
				
				case 5:
				{
					add_stats( b6_total,  b6_mon,  b6_vault, vault, mon,number);
					first_find(mb6_it);
					break;
				}
				
				case 6:
				{
					add_stats( b7_total,  b7_mon,  b7_vault, vault, mon,number);
					first_find(mb7_it);
					break;
				}
				
				case 7:
				{
					add_stats( b8_total,  b8_mon,  b8_vault, vault, mon,number);
					first_find(mb8_it);
					break;
				}
				
				case 8:
				{
					add_stats( b9_total,  b9_mon,  b9_vault, vault, mon,number);
					first_find(mb9_it);
					break;
				}
			
		
			}
			break;
		}
	}
	/* check to see if we have an artifact */
	if (o_ptr->artifact)
	{	
		/* add to artifact level total */
		art_total[lvl] += addval;
		
		/* add to the artifact iteration total */
		if (iter < TRIES_SIZE) art_it[iter]++;
				
		/* Obtain the artifact info */
		a_ptr = o_ptr->artifact;

		//debugging, print out that we found the artifact
		//msg_format("Found artifact %s",a_ptr->name);
		
		/* artifact is shallow */
		if (a_ptr->alloc_min < (p_ptr->depth - 20)) art_shal[lvl] += addval;
		
		/* artifact is close to the player depth */
		if ((a_ptr->alloc_min >= p_ptr->depth - 20) &&
			(a_ptr->alloc_min <= p_ptr->depth )) art_ave[lvl] += addval;
		
		/* artifact is out of depth */
		if (a_ptr->alloc_min > (p_ptr->depth)) art_ood[lvl] += addval;
		
		/* check to see if it's a special artifact */
		if ((o_ptr->tval == TV_LIGHT) || (o_ptr->tval == TV_AMULET)
			|| (o_ptr->tval == TV_RING))
		{
			/* increment special artifact counter */
			art_spec[lvl] += addval;
		} else {
			/* increment normal artifacts */
			art_norm[lvl] += addval;
		
			/* did it come from a monster? */
			if (mon) art_mon[lvl] += addval;
		
			/* did it come from a unique? */
			if (uniq) art_uniq[lvl] += addval;
		
			/* was it in a vault? */
			if (vault)
			{
				/* did a monster drop it ?*/
				if ((mon) || (uniq)) art_mon_vault[lvl] += addval;
				else art_vault[lvl] += addval;
			} else {	
				/* was it just lyin' on the floor? */
				if ((!uniq) && (!mon)) art_floor[lvl] += addval;
			}
		}
		/* preserve the artifact */
		if (!(clearing)) a_ptr->created = FALSE;
	}
		
	/* Get info on gold. */
	if (o_ptr->tval == TV_GOLD)
	{
		int temp = o_ptr->pval[DEFAULT_PVAL];
		gold_temp = temp;
	    gold_total[lvl] += (gold_temp / tries);
	
		/*From a monster? */
		if ((mon) || (uniq)) gold_mon[lvl] += (gold_temp / tries);
		else gold_floor[lvl] += (gold_temp / tries);
	}	
	
	/* remove the object */	
	delete_object_stat(cave->o_idx[y][x]);
}

/* Print heading infor for the file */
static void print_heading(void)
{
	/* PRINT INFO STUFF */
	file_putf(stats_log," This is a Monte Carlo simulation, results are arranged by level \n");
	file_putf(stats_log," Monsters:  OOD means between 1 and 10 levels deep, deadly is more than \n");
	file_putf(stats_log,"            10 levels deep \n");
	file_putf(stats_log," Artifacts: info on artifact location (vault, floor, etc) \n");
	file_putf(stats_log,"		     do not include special artifacts, only weapons and armor \n");
	file_putf(stats_log," Weapons  : Big dice weapons are either BoC, SoS, or Mod.  Uber \n");
	file_putf(stats_log,"            weapons, are one of the above with xblows or slay evil\n");
	file_putf(stats_log," Launchers: xtra shots and xtra might are only logged for x3 or\n");
	file_putf(stats_log,"            better.  Very good has +to hit or + to dam > 15\n");
	file_putf(stats_log," Amulets:   Endgame amulets are trickery, weaponmaster and magi\n");
	file_putf(stats_log," Armor:     Low resist armor may have more than one basic resist (acid, \n");
	file_putf(stats_log,"		     elec, fire, cold) but not all. \n");
	file_putf(stats_log," Books:     Prayer and Magic books have the same probability. \n");
	file_putf(stats_log," Potions:   Stat gain potions do not include CHR.  Aug counts \n");
	file_putf(stats_log,"			 as 5 potions, *enlight* as 2.  Healing potions are \n");
	file_putf(stats_log,"			 only *Healing* and Life\n");
	file_putf(stats_log," Scrolls:   Endgame scrolls include *Dest*, Rune, MBan and Ban \n");
	file_putf(stats_log,"    		 *Acq* counts as two Acq scrolls");
	file_putf(stats_log," Rods: 	 Utility rods: d-obj, d-stairs, d-traps, light, illum \n");
	file_putf(stats_log,"    		 Endgame rods: Speed, Healing \n");
	file_putf(stats_log," Staves: 	 Kill staves: dispel evil, power, holiness. \n");
	file_putf(stats_log,"    		 Power staves: healing, magi, banishment \n");
}
/* Print all the stats for each level */
static void print_stats(int lvl)
{
	/* check bounds on lvl */
	if ((lvl < 0) || (lvl > 100)) return;
	
	/* print level heading */
	file_putf(stats_log,"\n");
	file_putf(stats_log,"******** LEVEL %d , %d tries********* \n",lvl, tries);
	file_putf(stats_log,"\n");
	
	/* print monster heading */
	file_putf(stats_log," MONSTER INFO \n");
	file_putf(stats_log," Total monsters: %f OOD: %f Deadly: %f \n",
				mon_total[lvl], mon_ood[lvl], mon_deadly[lvl]);
	file_putf(stats_log," Unique monsters: %f OOD: %f Deadly: %f \n",
				uniq_total[lvl], uniq_ood[lvl], uniq_deadly[lvl]);
	/* print artifact heading */
	
	
	
	file_putf(stats_log,"\n ARTIFACT INFO \n");
	
	/* basic artifact info */
	file_putf(stats_log,"Total artifacts: %f  Special artifacts: %f  Weapons/armor: %f \n",
		art_total[lvl], art_spec[lvl], art_norm[lvl]);
		
	/* artifact depth info */
	file_putf(stats_log,"Shallow: %f  Average: %f  Ood: %f \n",
		art_shal[lvl],art_ave[lvl],art_ood[lvl]);
	
	/* more advanced info */
	file_putf(stats_log,"From vaults: %f  From floor (no vault): %f \n",
		art_vault[lvl],art_floor[lvl]);
	file_putf(stats_log,"Uniques: %f  Monsters: %f  Vault denizens: %f \n",
		art_uniq[lvl], art_mon[lvl], art_mon_vault[lvl]);
		
	/* print general equipment stuff */
	file_putf(stats_log,"\n");
	file_putf(stats_log," Equipment info \n");	
	file_putf(stats_log," FA total:     %f From Monsters: %f In Vaults: %f \n",
		faeq_total[lvl], faeq_mon[lvl], faeq_vault[lvl]);
	file_putf(stats_log," SI total:     %f From Monsters: %f In Vaults: %f \n",
		sieq_total[lvl], sieq_mon[lvl], sieq_vault[lvl]);
	file_putf(stats_log," lo res total: %f From Monsters: %f In Vaults: %f \n",
		reeq_total[lvl], reeq_mon[lvl], reeq_vault[lvl]);
	file_putf(stats_log," rbase total: 	%f From Monsters: %f In Vaults: %f \n",
		rbeq_total[lvl], rbeq_mon[lvl], rbeq_vault[lvl]);
	file_putf(stats_log," rpois total:  %f From Monsters: %f In Vaults: %f \n",
		poeq_total[lvl], poeq_mon[lvl], poeq_vault[lvl]);
	file_putf(stats_log," rnexus total: %f From Monsters: %f In Vaults: %f \n",
		nxeq_total[lvl], nxeq_mon[lvl], nxeq_vault[lvl]);
	file_putf(stats_log," rblind total: %f From Monsters: %f In Vaults: %f \n",
		bleq_total[lvl], bleq_mon[lvl], bleq_vault[lvl]);
	file_putf(stats_log," rconf total:  %f From Monsters: %f In Vaults: %f \n",
		cfeq_total[lvl], cfeq_mon[lvl], cfeq_vault[lvl]);
	file_putf(stats_log," speed total:  %f From Monsters: %f In Vaults: %f \n",
		speq_total[lvl], speq_mon[lvl], speq_vault[lvl]);
	file_putf(stats_log," telep total:  %f From Monsters: %f In Vaults: %f \n",
		teeq_total[lvl], teeq_mon[lvl], teeq_vault[lvl]);
	
	/* print weapon stuff */
	file_putf(stats_log,"\n");
	file_putf(stats_log," WEAPON INFO \n");	
	file_putf(stats_log," Total:        %f From Monsters: %f In Vaults: %f \n",
			weap_total[lvl],weap_mon[lvl],weap_vault[lvl]);
	file_putf(stats_log," Bad:          %f From Monsters: %f In Vaults: %f \n",
		bdweap_total[lvl], bdweap_mon[lvl], bdweap_vault[lvl]);
	file_putf(stats_log," Average:      %f From Monsters: %f In Vaults: %f \n",
		avweap_total[lvl], avweap_mon[lvl], avweap_vault[lvl]);
	file_putf(stats_log," Good (great): %f From Monsters: %f In Vaults: %f \n",
		gdweap_total[lvl], gdweap_mon[lvl], gdweap_vault[lvl]);
	file_putf(stats_log," Slay(no evil):%f From Monsters: %f In Vaults: %f \n",
		slweap_total[lvl], slweap_mon[lvl], slweap_vault[lvl]);
	file_putf(stats_log," Slay evil:    %f From Monsters: %f In Vaults: %f \n",
		evweap_total[lvl], evweap_mon[lvl], evweap_vault[lvl]);
	file_putf(stats_log," *Slay*:       %f From Monsters: %f In Vaults: %f \n",
		klweap_total[lvl], klweap_mon[lvl], klweap_vault[lvl]);
	file_putf(stats_log," Branded:      %f From Monsters: %f In Vaults: %f \n",
		brweap_total[lvl], brweap_mon[lvl], brweap_vault[lvl]);
	file_putf(stats_log," West.:        %f From Monsters: %f In Vaults: %f \n",
		weweap_total[lvl], weweap_mon[lvl], weweap_vault[lvl]);
	file_putf(stats_log," Defender:     %f From Monsters: %f In Vaults: %f \n",
		deweap_total[lvl], deweap_mon[lvl], deweap_vault[lvl]);
	file_putf(stats_log," Gondolin:     %f From Monsters: %f In Vaults: %f \n",
		goweap_total[lvl], goweap_mon[lvl], goweap_vault[lvl]);
	file_putf(stats_log," HA:           %f From Monsters: %f In Vaults: %f \n",
		haweap_total[lvl], haweap_mon[lvl], haweap_vault[lvl]);
	file_putf(stats_log," Xtra blows:   %f From Monsters: %f In Vaults: %f \n",
		xbweap_total[lvl], xbweap_mon[lvl], xbweap_vault[lvl]);
	file_putf(stats_log," Telep:        %f From Monsters: %f In Vaults: %f \n",
		teweap_total[lvl], teweap_mon[lvl], teweap_vault[lvl]);
	file_putf(stats_log," Big dice:     %f From Monsters: %f In Vaults: %f \n",
		huweap_total[lvl], huweap_mon[lvl], huweap_vault[lvl]);
	file_putf(stats_log," Uber:         %f From Monsters: %f In Vaults: %f \n",
		ubweap_total[lvl], ubweap_mon[lvl], ubweap_vault[lvl]);
	file_putf(stats_log," Morgul:       %f From Monsters: %f In Vaults: %f \n",
		moweap_total[lvl], moweap_mon[lvl], moweap_vault[lvl]);
		
	/* launcher info */
	file_putf(stats_log,"\n");
	file_putf(stats_log," LAUNCHER INFO \n");
	file_putf(stats_log," Total:        %f From Monsters: %f In Vaults: %f \n",
			bow_total[lvl],bow_mon[lvl],bow_vault[lvl]);
	file_putf(stats_log," Bad:          %f From Monsters: %f In Vaults: %f \n",
		bdbow_total[lvl], bdbow_mon[lvl], bdbow_vault[lvl]);
	file_putf(stats_log," Average:      %f From Monsters: %f In Vaults: %f \n",
		avbow_total[lvl], avbow_mon[lvl], avbow_vault[lvl]);
	file_putf(stats_log," Good (great): %f From Monsters: %f In Vaults: %f \n",
		gdbow_total[lvl], gdbow_mon[lvl], gdbow_vault[lvl]);
	file_putf(stats_log," Very good:    %f From Monsters: %f In Vaults: %f \n",
		vgbow_total[lvl], vgbow_mon[lvl], vgbow_vault[lvl]);
	file_putf(stats_log," Xtra might:   %f From Monsters: %f In Vaults: %f \n",
		xmbow_total[lvl], xmbow_mon[lvl], xmbow_vault[lvl]);
	file_putf(stats_log," Xtra shots:   %f From Monsters: %f In Vaults: %f \n",
		xsbow_total[lvl], xsbow_mon[lvl], xsbow_vault[lvl]);
	file_putf(stats_log," Buckland:     %f From Monsters: %f In Vaults: %f \n",
		bubow_total[lvl], bubow_mon[lvl], bubow_vault[lvl]);
	file_putf(stats_log," Telep:        %f From Monsters: %f In Vaults: %f \n",
		tebow_total[lvl], tebow_mon[lvl], tebow_vault[lvl]);
	file_putf(stats_log," Cursed:       %f From Monsters: %f In Vaults: %f \n",
		cubow_total[lvl], cubow_mon[lvl], cubow_vault[lvl]);
	
	/* print armor heading */
	file_putf(stats_log,"\n");
	file_putf(stats_log," ARMOR INFO \n");
	file_putf(stats_log," Total:        %f From Monsters: %f In Vaults: %f \n",
			arm_total[lvl],arm_mon[lvl],arm_vault[lvl]);
	file_putf(stats_log," Bad:          %f From Monsters: %f In Vaults: %f \n",
		bdarm_total[lvl], bdarm_mon[lvl], bdarm_vault[lvl]);
	file_putf(stats_log," Average:      %f From Monsters: %f In Vaults: %f \n",
		avarm_total[lvl], avarm_mon[lvl], avarm_vault[lvl]);
	file_putf(stats_log," Good (great): %f From Monsters: %f In Vaults: %f \n",
		gdarm_total[lvl], gdarm_mon[lvl], gdarm_vault[lvl]);
	file_putf(stats_log," STR total:    %f From Monsters: %f In Vaults: %f \n",
		strarm_total[lvl], strarm_mon[lvl], strarm_vault[lvl]);
	file_putf(stats_log," INT total:    %f From Monsters: %f In Vaults: %f \n",
		intarm_total[lvl], intarm_mon[lvl], intarm_vault[lvl]);
	file_putf(stats_log," WIS total:    %f From Monsters: %f In Vaults: %f \n",
		wisarm_total[lvl], wisarm_mon[lvl], wisarm_vault[lvl]);
	file_putf(stats_log," DEX total:    %f From Monsters: %f In Vaults: %f \n",
		dexarm_total[lvl], dexarm_mon[lvl], dexarm_vault[lvl]);
	file_putf(stats_log," CON total:    %f From Monsters: %f In Vaults: %f \n",
		conarm_total[lvl], conarm_mon[lvl], conarm_vault[lvl]);
	file_putf(stats_log," cursed total: %f From Monsters: %f In Vaults: %f \n",
		cuarm_total[lvl], cuarm_mon[lvl], cuarm_vault[lvl]);
	
		
	/* ring stuff */
	file_putf(stats_log,"\n");
	file_putf(stats_log," RING INFO \n");
	file_putf(stats_log," Total:        %f From Monsters: %f In Vaults: %f \n",
		ring_total[lvl], ring_mon[lvl], ring_vault[lvl]);
	file_putf(stats_log," cursed total: %f From Monsters: %f In Vaults: %f \n",
		curing_total[lvl], curing_mon[lvl], curing_vault[lvl]);
	file_putf(stats_log," Speed total:  %f From Monsters: %f In Vaults: %f \n",
		spring_total[lvl], spring_mon[lvl], spring_vault[lvl]);
	file_putf(stats_log," Stat total:   %f From Monsters: %f In Vaults: %f \n",
		string_total[lvl], string_mon[lvl], string_vault[lvl]);
	file_putf(stats_log," FA total:     %f From Monsters: %f In Vaults: %f \n",
		faring_total[lvl], faring_mon[lvl], faring_vault[lvl]);
	file_putf(stats_log," SI total:     %f From Monsters: %f In Vaults: %f \n",
		siring_total[lvl], siring_mon[lvl], siring_vault[lvl]);
	file_putf(stats_log," rpois total:  %f From Monsters: %f In Vaults: %f \n",
		poring_total[lvl], poring_mon[lvl], poring_vault[lvl]);
	file_putf(stats_log," brand total:  %f From Monsters: %f In Vaults: %f \n",
		brring_total[lvl], brring_mon[lvl], brring_vault[lvl]);
	file_putf(stats_log," Elven total:  %f From Monsters: %f In Vaults: %f \n",
		elring_total[lvl], elring_mon[lvl], elring_vault[lvl]);
	file_putf(stats_log," One total:    %f From Monsters: %f In Vaults: %f \n",
		onering_total[lvl], onering_mon[lvl], onering_vault[lvl]);
	
	/* amulet stuff */
	file_putf(stats_log,"\n");
	file_putf(stats_log," AMULET INFO \n");
	file_putf(stats_log," Total:        %f From Monsters: %f In Vaults: %f \n",
		amu_total[lvl], amu_mon[lvl], amu_vault[lvl]);
	file_putf(stats_log," Wisdom:       %f From Monsters: %f In Vaults: %f \n",
		wisamu_total[lvl], wisamu_mon[lvl], wisamu_vault[lvl]);
	file_putf(stats_log," Endgame:      %f From Monsters: %f In Vaults: %f \n",
		endamu_total[lvl], endamu_mon[lvl], endamu_vault[lvl]);
	file_putf(stats_log," Telep:        %f From Monsters: %f In Vaults: %f \n",
		teamu_total[lvl], teamu_mon[lvl], teamu_vault[lvl]);
	file_putf(stats_log," Cursed:       %f From Monsters: %f In Vaults: %f \n",
		cuamu_total[lvl], cuamu_mon[lvl], cuamu_vault[lvl]);
		
	/* print ammo stuff */
	file_putf(stats_log,"\n");
	file_putf(stats_log," AMMO INFO \n");
	file_putf(stats_log," Total:        %f From Monsters: %f In Vaults: %f \n",
			ammo_total[lvl],ammo_mon[lvl],ammo_vault[lvl]);
	file_putf(stats_log," Bad:          %f From Monsters: %f In Vaults: %f \n",
		bdammo_total[lvl], bdammo_mon[lvl], bdammo_vault[lvl]);
	file_putf(stats_log," Average:      %f From Monsters: %f In Vaults: %f \n",
		avammo_total[lvl], avammo_mon[lvl], avammo_vault[lvl]);
	file_putf(stats_log," Good (great): %f From Monsters: %f In Vaults: %f \n",
		gdammo_total[lvl], gdammo_mon[lvl], gdammo_vault[lvl]);
	file_putf(stats_log," Ego:          %f From Monsters: %f In Vaults: %f \n",
		egammo_total[lvl], egammo_mon[lvl], egammo_vault[lvl]);
	file_putf(stats_log," Mith/Seek:    %f From Monsters: %f In Vaults: %f \n",
		vgammo_total[lvl], vgammo_mon[lvl], vgammo_vault[lvl]);
	file_putf(stats_log," M/S ego:      %f From Monsters: %f In Vaults: %f \n",
		awammo_total[lvl], awammo_mon[lvl], awammo_vault[lvl]);
	file_putf(stats_log," M/S slay evil:%f From Monsters: %f In Vaults: %f \n",
		evammo_total[lvl], evammo_mon[lvl], evammo_vault[lvl]);
	file_putf(stats_log," M/S Holy:     %f From Monsters: %f In Vaults: %f \n",
		hmammo_total[lvl], hmammo_mon[lvl], hmammo_vault[lvl]);
	
	
	/*print potion stuff */
	file_putf(stats_log,"\n");
	file_putf(stats_log," POTION INFO \n");
	file_putf(stats_log," Total:        %f From Monsters: %f In Vaults: %f \n",
		pot_total[lvl], pot_mon[lvl], pot_vault[lvl]);
	file_putf(stats_log," Stat total:   %f From Monsters: %f In Vaults: %f \n",
		gain_total[lvl], gain_mon[lvl], gain_vault[lvl]);
	file_putf(stats_log," Rmana total:  %f From Monsters: %f In Vaults: %f \n",
		rmana_total[lvl], rmana_mon[lvl], rmana_vault[lvl]);
	file_putf(stats_log," *Heal* total: %f From Monsters: %f In Vaults: %f \n",
		bigheal_total[lvl], bigheal_mon[lvl], bigheal_vault[lvl]);
		
	/*print scroll stuff*/
	file_putf(stats_log,"\n");
	file_putf(stats_log," SCROLL INFO \n");
	file_putf(stats_log," Total:        %f From Monsters: %f In Vaults: %f \n",
		scroll_total[lvl], scroll_mon[lvl], scroll_vault[lvl]);
	file_putf(stats_log," Endgame:      %f From Monsters: %f In Vaults: %f \n",
		escroll_total[lvl], escroll_mon[lvl], escroll_vault[lvl]);
	file_putf(stats_log," Acq total:    %f From Monsters: %f In Vaults: %f \n",
		acq_total[lvl], acq_mon[lvl], acq_vault[lvl]);
	
	/* print rod stuff */
	file_putf(stats_log,"\n");
	file_putf(stats_log," ROD INFO \n");
	file_putf(stats_log," Total:        %f From Monsters: %f In Vaults: %f \n",
		rod_total[lvl], rod_mon[lvl], rod_vault[lvl]);
	file_putf(stats_log," Util total:   %f From Monsters: %f In Vaults: %f \n",
		urod_total[lvl], urod_mon[lvl], urod_vault[lvl]);
	file_putf(stats_log," TO total:     %f From Monsters: %f In Vaults: %f \n",
		torod_total[lvl], torod_mon[lvl], torod_vault[lvl]);
	file_putf(stats_log," DAll total:   %f From Monsters: %f In Vaults: %f \n",
		drod_total[lvl], drod_mon[lvl], drod_vault[lvl]);
	file_putf(stats_log," Endgame:      %f From Monsters: %f In Vaults: %f \n",
		erod_total[lvl], erod_mon[lvl], erod_vault[lvl]);
		
	/* print staff stuff */
	file_putf(stats_log,"\n");
	file_putf(stats_log," STAFF INFO \n");
	file_putf(stats_log," Total:        %f From Monsters: %f In Vaults: %f \n",
		staff_total[lvl], staff_mon[lvl], staff_vault[lvl]);
	file_putf(stats_log," Speed total:  %f From Monsters: %f In Vaults: %f \n",
		sstaff_total[lvl], sstaff_mon[lvl], sstaff_vault[lvl]);
	file_putf(stats_log," *Dest* total: %f From Monsters: %f In Vaults: %f \n",
		dstaff_total[lvl], dstaff_mon[lvl], dstaff_vault[lvl]);
	file_putf(stats_log," Kill total:   %f From Monsters: %f In Vaults: %f \n",
		kstaff_total[lvl], kstaff_mon[lvl], kstaff_vault[lvl]);
	file_putf(stats_log," Powerful:     %f From Monsters: %f In Vaults: %f \n",
		pstaff_total[lvl], pstaff_mon[lvl], pstaff_vault[lvl]);
		
	/* print wand stuff */
	file_putf(stats_log,"\n");
	file_putf(stats_log," WAND INFO \n");
	file_putf(stats_log," Total:        %f From Monsters: %f In Vaults: %f \n",
		wand_total[lvl], wand_mon[lvl], wand_vault[lvl]);
	file_putf(stats_log," TO total:     %f From Monsters: %f In Vaults: %f \n",
		towand_total[lvl], towand_mon[lvl], towand_vault[lvl]);
	
	/* print book stuff */
	file_putf(stats_log,"\n");
	file_putf(stats_log," MAGIC/PRAYER BOOK INFO \n");
	file_putf(stats_log," MB1:         %f From Monsters: %f In Vaults: %f \n",
		b1_total[lvl], b1_mon[lvl], b1_vault[lvl]);
	file_putf(stats_log," MB2:         %f From Monsters: %f In Vaults: %f \n",
		b2_total[lvl], b2_mon[lvl], b2_vault[lvl]);
	file_putf(stats_log," MB3:         %f From Monsters: %f In Vaults: %f \n",
		b3_total[lvl], b3_mon[lvl], b3_vault[lvl]);
	file_putf(stats_log," MB4:         %f From Monsters: %f In Vaults: %f \n",
		b4_total[lvl], b4_mon[lvl], b4_vault[lvl]);
	file_putf(stats_log," MB5:         %f From Monsters: %f In Vaults: %f \n",
		b5_total[lvl], b5_mon[lvl], b5_vault[lvl]);
	file_putf(stats_log," MB6:         %f From Monsters: %f In Vaults: %f \n",
		b6_total[lvl], b6_mon[lvl], b6_vault[lvl]);
	file_putf(stats_log," MB7:         %f From Monsters: %f In Vaults: %f \n",
		b7_total[lvl], b7_mon[lvl], b7_vault[lvl]);
	file_putf(stats_log," MB8:         %f From Monsters: %f In Vaults: %f \n",
		b8_total[lvl], b8_mon[lvl], b8_vault[lvl]);
	file_putf(stats_log," MB9:         %f From Monsters: %f In Vaults: %f \n",
		b9_total[lvl], b9_mon[lvl], b9_vault[lvl]);
		
	/* print gold heading */
	file_putf(stats_log,"\n");
	file_putf(stats_log," MONEY INFO \n");
	file_putf(stats_log,"total gold:    %f From Monsters %f\n",
		gold_total[lvl],gold_mon[lvl]);
	
}

/*
 *	Compute and print the mean and standard deviation for an array of known size*
 */
static void mean_and_stdv(int array[TRIES_SIZE])
{
	int k, maxiter;
	double tot=0, mean, stdev, temp=0;
	
	/* get the maximum iteration value */
	if (tries > TRIES_SIZE) maxiter = TRIES_SIZE; else maxiter = tries; 
	
	/* sum the array */
	for (k = 0; k < maxiter; k++) tot += array[k];
	
	/* compute the mean */
	mean = tot / tries;
	
	/* sum up the squares */
	for (k = 0; k < maxiter; k++) temp += (array[k] - mean) * (array[k] - mean);
	
	/* compute standard dev */
	stdev = sqrt(temp / tries);
	
	/* print to file */
	file_putf(stats_log," mean: %f  std-dev: %f \n",mean,stdev);
	
}

/* calculated the probability of finding an item by
 * a specific level, and print it to the output file
 */

 static void prob_of_find(double stat[MAX_LVL])
 {
	static int lvl,tmpcount;
	double find = 0.0, tmpfind = 0.0;
	
	/* skip town level */
	for (lvl = 1; lvl < MAX_LVL ; lvl++)
	{
		/* calculate the probability of not finding the stat */
		tmpfind=(1 - stat[lvl]);
		
		/* maximum probability is 98% */
		if (tmpfind < 0.02) tmpfind = 0.02;
		
		/* multiply probabilities of not finding */
		if (find <= 0) find = tmpfind; else find *= tmpfind;
	
		/* increase count to 5 */
		tmpcount++;
		
		/* print output every 5 levels */
		if (tmpcount == 5)
		{
			/* print it */
			file_putf(stats_log,"%f \t",1-find);
			
			/* reset temp counter */
			tmpcount=0;
		}
	}
	/* put a new line in prep of next entry */
	file_putf(stats_log,"\n"); 
 }

/* Left this function unlinked for now */
#if 0 
static double total(double stat[MAX_LVL])
{
	int k;
	double out=0;

	for (k = 0; k < MAX_LVL; k++)
	{
		out += stat[k];
	}

	return out;
} 
#endif
/* post process select items */
static void post_process_stats(void)
{
	double arttot;
	int k;

	/* output a title */
	file_putf(stats_log,"\n");
	file_putf(stats_log,"***** POST PROCESSING *****\n");
	file_putf(stats_log,"\n");
	file_putf(stats_log,"Item \t5\t\t\t10\t\t\t15\t\t\t20\t\t\t25\t\t\t");
	file_putf(stats_log,"30\t\t\t35\t\t\t40\t\t\t45\t\t\t50\t\t\t");
	file_putf(stats_log,"55\t\t\t60\t\t\t65\t\t\t70\t\t\t75\t\t\t");
	file_putf(stats_log,"80\t\t\t85\t\t\t90\t\t\t95\t\t\t100\n");
	
	file_putf(stats_log,"FA   \t");
	prob_of_find(faeq_total);
	mean_and_stdv(fa_it);
	file_putf(stats_log,"SinV \t");
	prob_of_find(sieq_total);
	mean_and_stdv(si_it);
	file_putf(stats_log,"RBl  \t");
	prob_of_find(bleq_total);
	mean_and_stdv(bl_it);
	file_putf(stats_log,"RCf  \t");
	prob_of_find(cfeq_total);
	mean_and_stdv(cf_it);
	file_putf(stats_log,"Nexus \t");
	prob_of_find(nxeq_total);
	mean_and_stdv(nx_it);
	file_putf(stats_log,"Pois \t");
	prob_of_find(poeq_total);
	mean_and_stdv(po_it);
	file_putf(stats_log,"Tel  \t");
	prob_of_find(teeq_total);
	mean_and_stdv(te_it);
	file_putf(stats_log,"\n");
	file_putf(stats_log,"mb1  \t");
	prob_of_find(b1_total);
	mean_and_stdv(mb1_it);
	file_putf(stats_log,"mb2  \t");
	prob_of_find(b2_total);
	mean_and_stdv(mb2_it);
	file_putf(stats_log,"mb3  \t");
	prob_of_find(b3_total);
	mean_and_stdv(mb3_it);
	file_putf(stats_log,"mb4  \t");
	prob_of_find(b4_total);
	mean_and_stdv(mb4_it);
	file_putf(stats_log,"mb5  \t");
	prob_of_find(b5_total);
	mean_and_stdv(mb5_it);
	file_putf(stats_log,"mb6  \t");
	prob_of_find(b6_total);
	mean_and_stdv(mb6_it);
	file_putf(stats_log,"mb7  \t");
	prob_of_find(b7_total);
	mean_and_stdv(mb7_it);
	file_putf(stats_log,"mb8  \t");
	prob_of_find(b8_total);
	mean_and_stdv(mb8_it);
	file_putf(stats_log,"mb9  \t");
	prob_of_find(b9_total);
	mean_and_stdv(mb9_it);
	
	/* print artifact total */
	arttot=0;
	
	for (k=0; k < MAX_LVL; k++)
		arttot += art_total[k];
		
	file_putf(stats_log,"\n");
	file_putf(stats_log,"Total number of artifacts found %f \n",arttot);
	mean_and_stdv(art_it);
	
	/* temporary stuff goes here */
	/* Dungeon book totals for Eddie
	file_putf(stats_log,"mb5: %f\n",total(b5_total));
	file_putf(stats_log,"mb6: %f\n",total(b6_total));
	file_putf(stats_log,"mb7: %f\n",total(b7_total));
	file_putf(stats_log,"mb8: %f\n",total(b8_total));
	file_putf(stats_log,"mb9: %f\n",total(b9_total));
	*/
	
}



/*
 * Scans the dungeon for objects
*/
static void scan_for_objects(bool mon, bool uniq)
{ 

	int y,x;
  /* Get stats on objects */
		for (y = 1; y < DUNGEON_HGT - 1; y++)
		{
			for (x = 1; x < DUNGEON_WID - 1; x++)
			{
				const object_type *o_ptr = get_first_object(y, x);

				if (cave->o_idx[y][x] > 0) do
				{
					get_obj_data(o_ptr,y,x,mon,uniq);
				}
				while ((o_ptr = get_first_object(y,x)));
			}
		}
	
}

/*
 * This will scan the dungeon for monsters and then kill each
 * and every last one.
*/
static void scan_for_monsters(bool uniq)
{ 
	int i;
		
	/* Go through the monster list */
	for (i = 1; i < cave_monster_max(cave); i++)
	{
		monster_type *m_ptr = cave_monster(cave, i);
		
		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;
		
		stats_monster(m_ptr,i, uniq);
	}
}
/*
 * This is the entry point for generation statistics.
 */

static void stats_collect_level(void)
{
	/* Make a dungeon */
	cave_generate(cave,p_ptr);
	
	/* Scan for objects, these are floor objects */
	scan_for_objects(FALSE,FALSE);
		
	/* Get stats (and kill) all non-unique monsters */
	scan_for_monsters(FALSE);
		
	/* Do second scan for objects, monster objects */
	scan_for_objects(TRUE,FALSE);
		
	/* Get stats (and kill) all unique monster */
	scan_for_monsters(TRUE);
		
	/* Do third scan for objects, unique objects */
	scan_for_objects(TRUE,TRUE);	
}

/* 
 * this code will go through the artifact list and
 * make each artifact uncreated so that our sim player
 * can find them again!
 */

static void uncreate_artifacts(void)
{
	int i;
	
	/* Loop through artifacts */
	for (i = 0; z_info && i < z_info->a_max; i++)
	{
		artifact_type *a_ptr = &a_info[i];
		
		/* uncreate */
		a_ptr->created = FALSE;
		
	}
}

/*
 * This will revive all the uniques so the sim player
 * can kill them again.
 */

static void revive_uniques(void)
{
	int i;
	
	for (i = 1; i < z_info->r_max - 1; i++)
	{
		/* get the monster info */
		 monster_race *r_ptr = &r_info[i];

		/* revive the unique monster */
		if (rf_has(r_ptr->flags, RF_UNIQUE)) r_ptr->max_num = 1;

	}
		
}	
/* 
 * This function loops through the level and does N iterations of
 * the stat calling function.
 */ 
static void diving_stats(void)
{
	int depth;
	
	/* iterate through levels */
	for (depth = 0; depth < MAX_LVL; depth += 5)
	{
		p_ptr->depth = depth;
		if (p_ptr->depth == 0) p_ptr->depth = 1;
		
		/* do many iterations of each level */
		for (iter = 0; iter < tries; iter++)
		{
			/* get level output */
		     stats_collect_level();
		}
		
		/* print the output to the file */
		print_stats(depth);
		
		/* show the level to check on status */
		do_cmd_redraw();
		
	}
}

static void clearing_stats(void)
{
	int depth;
	
	/* do many iterations of the game */
	for (iter=0; iter < tries; iter++)
	{
		
		/* move all artifacts to uncreated */
		uncreate_artifacts();
		
		/* move all uniques to alive */
		revive_uniques();
		
		/* do randart regen */
		if ((regen) && (iter<tries))
		{
			/* get seed */
			int seed_randart=randint0(0x10000000);
			
			/* regen randarts */
			do_randart(seed_randart,TRUE);
		
		}
		
		/* do game iterations */
		for (depth = 1 ; depth < MAX_LVL; depth++)
		{
			/* debug */
			//msg_format("Attempting level %d",depth);
		
			/* move player to that depth */
			p_ptr->depth = depth;
		
			/* get stats */
			stats_collect_level();
			
			//debug
			//msg_format("Finished level %d,depth");
		}
		
		msg("Iteration %d complete",iter);
		
		
				
	}
	
	/* print to file */
	for (depth=0 ; depth < MAX_LVL; depth++) print_stats(depth);
	
	/* post processing */
	post_process_stats();
	
	/* display the current level */
	do_cmd_redraw(); 
	
}

/*
 * Prompt the user for params for the stats.
 * Sim type and number of sims
 */
static int stats_prompt(void)
{
	static int temp,simtype=1;
	static char tmp_val[100], yn;
	static char prompt[50];
	
	/* This is the prompt for no. of tries*/	
	strnfmt(prompt, sizeof(prompt), "Num of simulations: ");
	
	/* This is the default value (50) */
	strnfmt(tmp_val, sizeof(tmp_val), "%d", tries);
	
	/* Ask for the input */
	if (!get_string(prompt,tmp_val,7)) return 0;
	
	/* get the new value */
	temp = atoi(tmp_val);
	
	/* convert */
	if (temp < 1) temp = 1;
	
	/* save */
	tries=temp;
	
	/* set 'value to add' for arrays */
	addval = 1.0/tries;
	
	/* Get info on what type of run to do */
	strnfmt(prompt, sizeof(prompt), "Type of Sim: Diving (1) or Clearing (2) ");
	
	/* set default */
	strnfmt(tmp_val, sizeof(tmp_val), "%d", simtype);
	
	/* get the input */
	if (!get_string(prompt,tmp_val,4)) return 0;
	
	temp=atoi(tmp_val);
	
	/* make sure that the type is good */
	if ((temp == 1) || (temp == 2)) simtype = temp; else return 0;
	
	/* for clearing sim, check for randart regen */
	if (temp == 2)
	{
		/* Prompt */
		strnfmt(prompt, sizeof(prompt), "Regen randarts? (warning SLOW)");
		
		yn = get_char(prompt, "yn", 3, 'n');
		
		if (( yn == 'y') || (yn == 'Y')) regen=TRUE; else regen = FALSE;
	}
	return simtype;

}

/* 
 *This is the call from wiz_stats.  The top level function
 * in this code
 */
void stats_collect(void)
{
	static int simtype;
	static bool auto_flag;
	char buf[1024];
	int i;
	
	/* prompt the user for sim params */
	simtype=stats_prompt();
	
	/* make sure the results are good! */
	if (!((simtype == 1) || (simtype == 2))) return; 
	
	/* are we in diving or clearing mode */
	if (simtype == 2) clearing = TRUE;  else clearing = FALSE;
	
	/*Open log file*/
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER,
				"stats.log");
	stats_log = file_open(buf, MODE_WRITE, FTYPE_TEXT);
	
	/* Logging didn't work */
	if (!stats_log)
	{
		msg("Error - can't open stats.log for writing.");
		exit(1);
	}

	/* turn on auto-more.  This will clear prompts for items
	 * that drop under the player, or that can't fit on the 
	 * floor due to too many items.  This is a very small amount
	 * of items, even on deeper levels, so it's not worth worrying
	 * too much about.
	 */
	 auto_flag = FALSE;
	 
	 if (!OPT(auto_more)) 
	 {   
		/* remember that we turned off auto_more */
		auto_flag = TRUE;
		
		/* Turn on auto-more */
		option_set(option_name(OPT_auto_more),TRUE);
		
	}

	/* print heading for the file */
	print_heading();

	/* make sure all stats are 0 */
	for (i = 0; i < MAX_LVL; i++) init_stat_vals(i);
	
	/* make sure all iter vals are 0 */
	for (i = 0; i < TRIES_SIZE; i++) init_iter_vals(i);
	
	/* select diving option */
	if (!clearing) diving_stats();
	
	/* select clearing option */
	if (clearing) clearing_stats();
		
	/* Turn auto-more back off */
	if (auto_flag) option_set(option_name(OPT_auto_more),FALSE);
	
	/*Close log file */
	if (!file_close(stats_log))
	{
		msg("Error - can't close randart.log file.");
		exit(1);
	}	
}

#define DIST_MAX 10000

int cave_dist[DUNGEON_HGT][DUNGEON_WID];


void clear_cave_dist(void)
{
	int x,y;
	for (y = 1; y < DUNGEON_HGT - 1; y++)
		{
			for (x = 1; x < DUNGEON_WID - 1; x++)
			{
				cave_dist[y][x] = -1;
			}
		}
}		

void calc_cave_distances(void)
{
	int dist, i;
	int oy, ox, ty, tx, d;

	/* Squares with distance from player of n-1 */  
	int d_x_old[DIST_MAX];
	int d_y_old[DIST_MAX];
	int d_old_max;

	/* Squares with distance from player of n */
	int d_x_new[DIST_MAX];
	int d_y_new[DIST_MAX];
	int d_new_max;
	
	/* Set all cave spots to inaccessible */
	clear_cave_dist();
	
	/* Get player location */
	oy = d_y_old[0] = p_ptr->py;
	ox = d_x_old[0] = p_ptr->px;
	d_old_max = 1;
	
	/* distance from player starts at 0*/
	dist=0;
	
	/* Assign the distance value to the first square (player)*/
	cave_dist[oy][ox] = dist;
	
	do{
		d_new_max = 0;
		dist++;
		
		/* Loop over all visited squares of the previous iteration*/
		for(i=0 ;i < d_old_max; i++)
		{
			/* Get the square we want to look at */
			oy = d_y_old[i];
			ox = d_x_old[i];
			
			//debug
			//msg("x: %d y: %d dist: %d %d ",ox,oy,dist-1,i);
		
			/* Get all adjacent squares */
			for (d = 0; d < 8; d++)
			{
				/* Adjacent square location */
				ty = oy + ddy_ddd[d];
				tx = ox + ddx_ddd[d];
				
				if (!(in_bounds_fully(ty,tx))) continue;
				
				/* Have we been here before? */
				if (cave_dist[ty][tx] >= 0) continue;
				
				/* Is it a wall? */
				if (cave->feat[ty][tx] > FEAT_RUBBLE) continue;
				
				/* Add the new location */
				d_y_new[d_new_max] = ty;
				d_x_new[d_new_max] = tx;
				
				/* Assign the distance to that spot */
				cave_dist[ty][tx] = dist;
				
				d_new_max++;
				
				//debug
				//msg("x: %d y: %d dist: %d ",tx,ty,dist);
			}
		}
		
		/* copy the new distance list to the old one */
		for (i=0 ;i<d_new_max; i++)
		{
			d_y_old[i] = d_y_new[i];
			d_x_old[i] = d_x_new[i];
		}
		
		d_old_max = d_new_max;
	
		
	} while ((d_old_max > 0) || dist == DIST_MAX);
}

void pit_stats(void)
{
	int tries = 1000;
	int depth = p_ptr->command_arg;
	int hist[z_info->pit_max];
	int j, p;
	int type = 1;

	char tmp_val[100];
	
	/* Initialize hist */
	for (p = 0; p < z_info->pit_max; p++)
		hist[p] = 0;
	
	/* Format default value */
	strnfmt(tmp_val, sizeof(tmp_val), "%d", tries);
	
	/* Ask for the input - take the first 7 characters*/
	if (!get_string("Num of simulations: ", tmp_val, 7)) return;
	
	/* get the new value */
	tries = atoi(tmp_val);
	if (tries < 1) tries = 1;
	
	/* Format second default value */	
	strnfmt(tmp_val, sizeof(tmp_val), "%d", type);
	
	/* Ask for the input - take the first 7 characters*/
	if (!get_string("Pit type: ", tmp_val, 7)) return;

	/* get the new value */
	type = atoi(tmp_val);
	if (type < 1) type = 1;
	
	if (depth <= 0)
	{
		/* Format second default value */	
		strnfmt(tmp_val, sizeof(tmp_val), "%d", p_ptr->depth);
	
		/* Ask for the input - take the first 7 characters*/
		if (!get_string("Depth: ", tmp_val, 7)) return;

		/* get the new value */
		depth = atoi(tmp_val);
		if (depth < 1) depth = 1;
	
	}


	for (j = 0; j < tries; j++)
	{
		int i;
		int pit_idx = 0;
		int pit_dist = 999;
		
		for (i = 0; i < z_info->pit_max; i++)
		{
			int offset, dist;
			pit_profile *pit = &pit_info[i];
			
			if (!pit->name || pit->room_type != type) continue;

			offset = Rand_normal(pit->ave, 10);
			dist = ABS(offset - depth);

			if (dist < pit_dist && one_in_(pit->rarity))
			{
				pit_idx = i;
				pit_dist = dist;
			}
		}

		hist[pit_idx]++;
	}

	for (p = 0; p < z_info->pit_max; p++)
	{
		pit_profile *pit = &pit_info[p];
		if (pit->name)
			msg("Type: %s, Number: %d.", pit->name, hist[p]);
	}

	return;

}


/* Gather whether the dungeon has disconnects in it
 * and whether the player is disconnected from the stairs
 */
void disconnect_stats(void)
{
	int i,y,x;
	
	
	bool has_dsc, has_dsc_from_stairs;
				
	static int temp;
	static char tmp_val[100];
	static char prompt[50];
	
	long dsc_area=0, dsc_from_stairs=0;
	
	/* This is the prompt for no. of tries*/	
	strnfmt(prompt, sizeof(prompt), "Num of simulations: ");
	
	/* This is the default value (50) */
	strnfmt(tmp_val, sizeof(tmp_val), "%d", tries);
	
	/* Ask for the input */
	if (!get_string(prompt,tmp_val,7)) return;
	
	/* get the new value */
	temp = atoi(tmp_val);
	
	/* convert */
	if (temp < 1) temp = 1;
	
	/* save */
	tries=temp;
	
	for (i = 1; i <= tries; i++)
	{
	/* assume no disconnected areas */
	has_dsc = FALSE;
	
	/* assume you can't get to stairs */
	has_dsc_from_stairs = TRUE;
	
		/* Make a new cave */
		cave_generate(cave,p_ptr);
		
		/* Fill the distance array */
		calc_cave_distances();
		
		/*Cycle through the dungeon */
		for (y = 1; y < DUNGEON_HGT - 1; y++)
		{
			for (x = 1; x < DUNGEON_WID - 1; x++)
			{
				/* don't care about walls */
				if (cave->feat[y][x] > FEAT_RUBBLE) continue;
				
				/* Can we get there? */
				if (cave_dist[y][x] >= 0)
				{	
					/* Is it a  down stairs? */
					if ((cave->feat[y][x] == FEAT_MORE))// ||
						//(cave->feat[y][x] == FEAT_LESS))
					{
						has_dsc_from_stairs = FALSE;
					
						//debug
						//msg("dist to stairs: %d",cave_dist[y][x]);
					
					}
					
					continue;
				}
				
				/* Ignore vaults as they are often disconnected */
				if (cave->info[y][x] & (CAVE_ICKY)) continue;
				
				/* We have a disconnected area */
				has_dsc = TRUE;
				
			
			}
		}
		
		if (has_dsc_from_stairs) dsc_from_stairs++;
		
		if (has_dsc) dsc_area++;
				
		msg("Iteration: %d",i); 
	}

	msg("Total levels with disconnected areas: %ld",dsc_area);
	msg("Total levels isolated from stairs: %ld",dsc_from_stairs);
	
	/* redraw the level */
	do_cmd_redraw();
}


#else /* USE_STATS */

void stats_collect(void)
{
	msg("Statistics generation not turned on in this build.");
}

void disconnect_stats(void)
{
	msg("Statistics generation not turned on in this build.");
}

void pit_stats(void)
{
	msg("Statistics generation not turned on in this build.");
}
#endif /* USE_STATS */
