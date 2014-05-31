/*
 * File: stats.c
 * Purpose: Statistics collection on dungeon generation
 *
 * Copyright (c) 2008 Andi Sidwell
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
#include "dungeon.h"
#include "init.h"
#include "wizard.h"
#include "mon-make.h"
#include "monster.h"
#include "mon-constants.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "object.h"
#include "effects.h"
#include "generate.h"
#include "tables.h"
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

/*** handle gold separately ***/
/* gold */
static double gold_total[MAX_LVL], gold_floor[MAX_LVL], gold_mon[MAX_LVL], gold_wall[MAX_LVL];


typedef enum stat_code
{
	ST_BEGIN,
	ST_EQUIPMENT,
	ST_FA_EQUIPMENT,
	ST_SI_EQUIPMENT,
	ST_RESIST_EQUIPMENT,
	ST_RBASE_EQUIPMENT,
	ST_RPOIS_EQUIPMENT,
	ST_RNEXUS_EQUIPMENT,
	ST_RBLIND_EQUIPMENT,
	ST_RCONF_EQUIPMENT,
	ST_SPEED_EQUIPMENT,
	ST_TELEP_EQUIPMENT,
	ST_ARMORS,
	ST_BAD_ARMOR,
	ST_AVERAGE_ARMOR,
	ST_GOOD_ARMOR,	
	ST_STR_ARMOR,
	ST_INT_ARMOR,
	ST_WIS_ARMOR,
	ST_DEX_ARMOR,
	ST_CON_ARMOR,
	ST_CURSED_ARMOR,
	ST_WEAPONS,
	ST_BAD_WEAPONS,
	ST_AVERAGE_WEAPONS,
	ST_GOOD_WEAPONS,
	ST_SLAY_WEAPONS,
	ST_SLAYEVIL_WEAPONS,
	ST_KILL_WEAPONS,
	ST_BRAND_WEAPONS,
	ST_WESTERNESSE_WEAPONS,
	ST_DEFENDER_WEAPONS,
	ST_GONDOLIN_WEAPONS,
	ST_HOLY_WEAPONS,
	ST_XTRABLOWS_WEAPONS,
	ST_TELEP_WEAPONS,
	ST_HUGE_WEAPONS,
	ST_ENDGAME_WEAPONS,
	ST_MORGUL_WEAPONS,
	ST_BOWS,
	ST_BAD_BOWS,
	ST_AVERAGE_BOWS,
	ST_GOOD_BOWS,
	ST_VERYGOOD_BOWS,
	ST_XTRAMIGHT_BOWS,
	ST_XTRASHOTS_BOWS,
	ST_BUCKLAND_BOWS,
	ST_TELEP_BOWS,
	ST_CURSED_BOWS,
	ST_POTIONS,
	ST_GAINSTAT_POTIONS,
	ST_HEALING_POTIONS,
	ST_BIGHEAL_POTIONS,
	ST_RESTOREMANA_POTIONS,
	ST_SCROLLS,
	ST_ENDGAME_SCROLLS,
	ST_ACQUIRE_SCROLLS,
	ST_RODS,
	ST_UTILITY_RODS,
	ST_TELEPOTHER_RODS,
	ST_DETECTALL_RODS,
	ST_ENDGAME_RODS,
	ST_STAVES,
	ST_SPEED_STAVES,
	ST_DESTRUCTION_STAVES,
	ST_KILL_STAVES,
	ST_ENDGAME_STAVES,
	ST_WANDS,
	ST_TELEPOTHER_WANDS,
	ST_RINGS,
	ST_SPEEDS_RINGS,
	ST_STAT_RINGS,
	ST_RPOIS_RINGS,
	ST_FA_RINGS,
	ST_SI_RINGS,
	ST_BRAND_RINGS,
	ST_ELVEN_RINGS,
	ST_ONE_RINGS,
	ST_CURSED_RINGS,
	ST_AMULETS,
	ST_WIS_AMULETS,
	ST_TELEP_AMULETS,
	ST_ENDGAME_AMULETS,
	ST_CURSED_AMULETS,
	ST_AMMO,
	ST_BAD_AMMO,
	ST_AVERAGE_AMMO,
	ST_GOOD_AMMO,
	ST_BRANDSLAY_AMMO,
	ST_VERYGOOD_AMMO,
	ST_AWESOME_AMMO,
	ST_SLAYEVIL_AMMO,
	ST_HOLY_AMMO,
	ST_BOOKS,
	ST_1ST_BOOKS,
	ST_2ND_BOOKS,
	ST_3RD_BOOKS,
	ST_4TH_BOOKS,
	ST_5TH_BOOKS,
	ST_6TH_BOOKS,
	ST_7TH_BOOKS,
	ST_8TH_BOOKS,
	ST_9TH_BOOKS,
	ST_END
}	
stat_code;


struct stat_data
{
	stat_code st;
	char *name;
};

static const struct stat_data stat_message[] =
{
	{ST_BEGIN, ""},
	{ST_EQUIPMENT, "\n ***EQUIPMENT*** \n All:       "},
	{ST_FA_EQUIPMENT, " Free Action "},
	{ST_SI_EQUIPMENT, " See Invis   "},
	{ST_RESIST_EQUIPMENT, " Low Resist  "},
	{ST_RBASE_EQUIPMENT, " Resist Base "},
	{ST_RPOIS_EQUIPMENT, " Resist Pois "},
	{ST_RNEXUS_EQUIPMENT, " Res. Nexus  "},
	{ST_RBLIND_EQUIPMENT, " Res. Blind  "},	
	{ST_RCONF_EQUIPMENT, " Res. Conf.  "},
	{ST_SPEED_EQUIPMENT, " Speed       "},
	{ST_TELEP_EQUIPMENT, " Telepathy   "},
	{ST_ARMORS,  "\n ***ARMOR***      \n All:      "},
	{ST_BAD_ARMOR, " Bad         "},
	{ST_AVERAGE_ARMOR, " Average     "},
	{ST_GOOD_ARMOR, " Good        "},	
	{ST_STR_ARMOR, " +Strength   "},
	{ST_INT_ARMOR, " +Intel.     "},
	{ST_WIS_ARMOR, " +Wisdom     "},
	{ST_DEX_ARMOR, " +Dexterity  "},
	{ST_CON_ARMOR, " +Const.     "},
	{ST_CURSED_ARMOR, " Cursed       "},
	{ST_WEAPONS, "\n ***WEAPONS***   \n All:       "},
	{ST_BAD_WEAPONS, " Bad         "},
	{ST_AVERAGE_WEAPONS, " Average     "},
	{ST_GOOD_WEAPONS, " Good        "},
	{ST_SLAY_WEAPONS, " Weak Slay   "},
	{ST_SLAYEVIL_WEAPONS, " Slay evil   "},
	{ST_KILL_WEAPONS, " *Slay*      "},
	{ST_BRAND_WEAPONS, " Brand       "},
	{ST_WESTERNESSE_WEAPONS, " Westernesse "},
	{ST_DEFENDER_WEAPONS, " Defender    "},
	{ST_GONDOLIN_WEAPONS, " Gondolin    "},
	{ST_HOLY_WEAPONS, " Holy Avengr "},
	{ST_XTRABLOWS_WEAPONS, " Extra Blows "},
	{ST_TELEP_WEAPONS, " Telepathy   "},
	{ST_HUGE_WEAPONS, " Huge        "},//MoD, SoS and BoC
	{ST_ENDGAME_WEAPONS, " Endgame     "},//MoD, SoS and BoC with slay evil or x2B
	{ST_MORGUL_WEAPONS, " Morgul      "},
	{ST_BOWS, "\n ***LAUNCHERS*** \n All:        "},
	{ST_BAD_BOWS, " Bad         "},
	{ST_AVERAGE_BOWS, " Average     "},
	{ST_GOOD_BOWS, " Good        "},
	{ST_VERYGOOD_BOWS, " Very Good   "},//Power > 15
	{ST_XTRAMIGHT_BOWS, " Extra might "},
	{ST_XTRASHOTS_BOWS, " Extra shots "},
	{ST_BUCKLAND_BOWS, " Buckland    "},
	{ST_TELEP_BOWS, " Telepathy   "},
	{ST_CURSED_BOWS, " Cursed      "},
	{ST_POTIONS, "\n ***POTIONS***   \n All:        "},
	{ST_GAINSTAT_POTIONS, " Gain stat   "},//includes *enlight*
	{ST_HEALING_POTIONS, " Healing     "},
	{ST_BIGHEAL_POTIONS, " Big heal    "},//*heal* and life
	{ST_RESTOREMANA_POTIONS, " Rest. Mana  "},
	{ST_SCROLLS, "\n ***SCROLLS***   \n All:        "},
	{ST_ENDGAME_SCROLLS, " Endgame     "},// destruction, banish, mass banish, rune
	{ST_ACQUIRE_SCROLLS, " Acquire.    "},
	{ST_RODS, "\n ***RODS***      \n All:        "},
	{ST_UTILITY_RODS, " Utility     "},//dtrap, dstairs, dobj, light, illum
	{ST_TELEPOTHER_RODS, " Tele Other  "},
	{ST_DETECTALL_RODS, " Detect all  "},
	{ST_ENDGAME_RODS, " Endgame     "},//speed, healing
	{ST_STAVES, "\n ***STAVES***    \n All:        "},
	{ST_SPEED_STAVES, " Speed       "},
	{ST_DESTRUCTION_STAVES, " Destruction "},
	{ST_KILL_STAVES, " Kill        "},//dispel evil, power, holiness
	{ST_ENDGAME_STAVES, " Endgame     "},//healing, magi, banishment
	{ST_WANDS, "\n ***WANDS***     \n All:        "},
	{ST_TELEPOTHER_WANDS, " Tele Other  "},
	{ST_RINGS, "\n ***RINGS***     \n All:        "},
	{ST_SPEEDS_RINGS, " Speed       "},
	{ST_STAT_RINGS, " Stat        "},//str, dex, con, int
	{ST_RPOIS_RINGS, " Res. Pois.  "},
	{ST_FA_RINGS, " Free Action "},
	{ST_SI_RINGS, " See Invis.  "},
	{ST_BRAND_RINGS, " Brand       "},
	{ST_ELVEN_RINGS, " Elven       "},
	{ST_ONE_RINGS, " The One     "},
	{ST_CURSED_RINGS, " Cursed      "},
	{ST_RINGS, "\n ***AMULETS***   \n All:        "},
	{ST_WIS_AMULETS, " Wisdom      "},
	{ST_TELEP_AMULETS, " Telepathy   "},
	{ST_ENDGAME_AMULETS, " Endgame     "},//Trickery, weaponmastery, magi
	{ST_CURSED_AMULETS, " Cursed      "},
	{ST_AMMO, "\n ***AMMO***      \n All:        "},
	{ST_BAD_AMMO, " Bad         "},
	{ST_AVERAGE_AMMO, " Average     "},
	{ST_GOOD_AMMO, " Good        "},
	{ST_BAD_AMMO, " Brand       "},
	{ST_VERYGOOD_AMMO, " Very Good   "},//seeker or mithril
	{ST_AWESOME_AMMO, " Awesome     "},//seeker, mithril + brand
	{ST_SLAYEVIL_AMMO, " Slay evil   "},
	{ST_HOLY_AMMO, " Holy might  "},
	{ST_BOOKS, "\n ***BOOKS***     \n All:        "},
	{ST_1ST_BOOKS, " Book 1      "},
	{ST_2ND_BOOKS, " Book 2      "},
	{ST_3RD_BOOKS, " Book 3      "},
	{ST_4TH_BOOKS, " Book 4      "},
	{ST_5TH_BOOKS, " Book 5      "},
	{ST_6TH_BOOKS, " Book 6      "},
	{ST_7TH_BOOKS, " Book 7      "},
	{ST_8TH_BOOKS, " Book 8      "},
	{ST_9TH_BOOKS, " Book 9      "},	
};	

double stat_all[ST_END][3][MAX_LVL];
	
/* Values for things we want to find the level where it's
 * most likely to be first found */
typedef enum stat_first_find
{
	ST_FF_BEGIN,
	ST_FF_FA,
	ST_FF_SI,
	ST_FF_RPOIS,
	ST_FF_RNEXUS,
	ST_FF_RCONF,
	ST_FF_RBLIND,
	ST_FF_TELEP,
	ST_FF_BOOK1,
	ST_FF_BOOK2,
	ST_FF_BOOK3,
	ST_FF_BOOK4,
	ST_FF_BOOK5,
	ST_FF_BOOK6,
	ST_FF_BOOK7,
	ST_FF_BOOK8,
	ST_FF_BOOK9,
	ST_FF_END
}	
stat_first_find;

struct stat_ff_data
{
	stat_first_find st_ff;
	stat_code st;
	char *name;
};

static const struct stat_ff_data stat_ff_message[] =
{
	{ST_FF_BEGIN,ST_BEGIN,""},
	{ST_FF_FA,	ST_FA_EQUIPMENT,		"FA     \t"},
	{ST_FF_SI,	ST_SI_EQUIPMENT,		"SI     \t"},
	{ST_FF_RPOIS,	ST_RPOIS_EQUIPMENT,	"Rpois  \t"},
	{ST_FF_RNEXUS,	ST_RNEXUS_EQUIPMENT,  "Rnexus \t"},
	{ST_FF_RCONF,	ST_RCONF_EQUIPMENT,	"Rconf  \t"},
	{ST_FF_RBLIND,	ST_RBLIND_EQUIPMENT,	"Rblind \t"},
	{ST_FF_TELEP,	ST_TELEP_EQUIPMENT,	"Telep  \t"},
	{ST_FF_BOOK1,	ST_1ST_BOOKS,	"Book1  \t"},
	{ST_FF_BOOK2,	ST_2ND_BOOKS, 	"Book2  \t"},
	{ST_FF_BOOK3,	ST_3RD_BOOKS,	"Book3  \t"},
	{ST_FF_BOOK4,	ST_4TH_BOOKS,	"Book4  \t"},
	{ST_FF_BOOK5,	ST_5TH_BOOKS,	"Book5  \t"},
	{ST_FF_BOOK6,	ST_6TH_BOOKS,	"Book6  \t"},
	{ST_FF_BOOK7,	ST_7TH_BOOKS,	"Book7  \t"},
	{ST_FF_BOOK8,	ST_8TH_BOOKS,	"Book8	\t"},
	{ST_FF_BOOK9,	ST_9TH_BOOKS,	"Book9  \t"},
};

int stat_ff_all[ST_FF_END][TRIES_SIZE];



/* basic artifact info */
static double art_total[MAX_LVL], art_spec[MAX_LVL], art_norm[MAX_LVL];

/* artifact level info */
static double art_shal[MAX_LVL], art_ave[MAX_LVL], art_ood[MAX_LVL];

/* where normal artifacts come from */
static double art_mon[MAX_LVL], art_uniq[MAX_LVL], art_floor[MAX_LVL], art_vault[MAX_LVL], art_mon_vault[MAX_LVL];



/* monster info */
static double mon_total[MAX_LVL], mon_ood[MAX_LVL], mon_deadly[MAX_LVL];

/* unique info */
static double uniq_total[MAX_LVL], uniq_ood[MAX_LVL], uniq_deadly[MAX_LVL];


/* set everything to 0.0 to begin */
static void init_stat_vals()
{
	int i,j,k;

	for (i = 0;i<ST_END;i++)
		for (j=0;j<3;k=j++)
			for (k=0;k<MAX_LVL;k++)
				stat_all[i][j][k] = 0.0;
				
	for (i = 1; i<TRIES_SIZE; i++)
		art_it[i] = 0;
	
	for (i = 0; i<ST_FF_END; i++)
		for (j=0; j<TRIES_SIZE; j++)
			stat_ff_all[i][j] = 0.0;
}

/*
 *	Record the first level we find something
 */
static bool first_find(stat_first_find st)
{
	/* make sure we're not on an iteration above our array limit */
	if (iter >= TRIES_SIZE) return FALSE;

	/* make sure we haven't found it earlier on this iteration */
	if (stat_ff_all[st][iter] > 0) return FALSE;

	/* assign the depth to this value */
	stat_ff_all[st][iter] = player->depth;

	/* success */
	return TRUE;
}

/*
 * Add the number of drops for a specifci stat
 */
static void add_stats(stat_code st, bool vault, bool mon, int number)
{
	int lvl;
	
	/* get player level */
	lvl=player->depth;
	
	/* be careful about bounds */
	if ((lvl > MAX_LVL) || (lvl < 0)) return;
	
	/* add to the total */
	stat_all[st][0][lvl] += addval * number;
	
	/* add to the total from vaults */
	if ((!mon) && (vault)) stat_all[st][2][lvl] += addval * number;
	
	/* add to the total from monsters */
	if (mon) stat_all[st][1][lvl] += addval * number;

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

	bool vault = square_isvault(cave, y, x);
	int effect;
	int number = o_ptr->number;
	static int lvl;
	artifact_type *a_ptr;

	double gold_temp=0;

	assert(o_ptr->kind);

	/* get player depth */
	lvl=player->depth;

	/* check for some stuff that we will use regardless of type */
	/* originally this was armor, but I decided to generalize it */

	/* has free action (hack: don't include Inertia)*/
	if (of_has(o_ptr->flags, OF_FREE_ACT) && 
		!((o_ptr->tval == TV_AMULET) &&
		  (!strstr(o_ptr->kind->name, "Inertia")))) {

			/* add the stats */
			add_stats(ST_FA_EQUIPMENT, vault, mon, number);

			/* record first level */
			first_find(ST_FF_FA);
		}


	/* has see invis */
	if (of_has(o_ptr->flags, OF_SEE_INVIS)){

		add_stats(ST_SI_EQUIPMENT, vault, mon, number);
		first_find(ST_FF_SI);
	}
	/* has at least one basic resist */
 	if ((o_ptr->el_info[ELEM_ACID].res_level == 1) ||
		(o_ptr->el_info[ELEM_ELEC].res_level == 1) ||
		(o_ptr->el_info[ELEM_COLD].res_level == 1) ||
		(o_ptr->el_info[ELEM_FIRE].res_level == 1)){

			add_stats(ST_RESIST_EQUIPMENT, vault, mon, number);
	}

	/* has rbase */
	if ((o_ptr->el_info[ELEM_ACID].res_level == 1) &&
		(o_ptr->el_info[ELEM_ELEC].res_level == 1) &&
		(o_ptr->el_info[ELEM_COLD].res_level == 1) &&
		(o_ptr->el_info[ELEM_FIRE].res_level == 1))
		add_stats(ST_RBASE_EQUIPMENT, vault, mon, number);

	/* has resist poison */
	if (o_ptr->el_info[ELEM_POIS].res_level == 1){

		add_stats(ST_RPOIS_EQUIPMENT, vault, mon, number);
		first_find(ST_FF_RPOIS);
		
	}
	/* has resist nexus */
	if (o_ptr->el_info[ELEM_NEXUS].res_level == 1){

		add_stats(ST_RNEXUS_EQUIPMENT, vault, mon, number);
		first_find(ST_FF_RNEXUS);
	}
	/* has resist blind */
	if (of_has(o_ptr->flags, OF_PROT_BLIND)){

		add_stats(ST_RBLIND_EQUIPMENT, vault, mon, number);
		first_find(ST_FF_RBLIND);
	}

	/* has resist conf */
	if (of_has(o_ptr->flags, OF_PROT_CONF)){

		add_stats(ST_RCONF_EQUIPMENT, vault, mon, number);
		first_find(ST_FF_RCONF);
	}

	/* has speed */
	if (o_ptr->modifiers[OBJ_MOD_SPEED] != 0)
		add_stats(ST_SPEED_EQUIPMENT, vault, mon, number);

	/* has telepathy */
	if (of_has(o_ptr->flags, OF_TELEPATHY)){

		add_stats(ST_TELEP_EQUIPMENT, vault, mon, number);
		first_find(ST_FF_TELEP);
	}

	switch(o_ptr->tval){

		/* armor */
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_HELM:
		case TV_CROWN:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:{

			/* do not include artifacts */
			if (o_ptr->artifact) break;

			/* add to armor total */
			add_stats(ST_ARMORS, vault, mon, number);

			/* check if bad, good, or average */
			if (o_ptr->to_a < 0)
				add_stats(ST_BAD_ARMOR, vault, mon, number);
			if (o_ptr->to_h == 0)
				add_stats(ST_AVERAGE_ARMOR, vault, mon, number);
			if (o_ptr->to_h > 0)
				add_stats(ST_GOOD_ARMOR, vault, mon, number);

			/* has str boost */
			if (o_ptr->modifiers[OBJ_MOD_STR] != 0)
				add_stats(ST_STR_ARMOR, vault, mon, number);

			/* has dex boost */
			if (o_ptr->modifiers[OBJ_MOD_DEX] != 0)
				add_stats(ST_DEX_ARMOR, vault, mon, number);

			/* has int boost */
			if (o_ptr->modifiers[OBJ_MOD_INT] != 0)
				add_stats(ST_INT_ARMOR, vault, mon, number);

			if (o_ptr->modifiers[OBJ_MOD_WIS] != 0)
				add_stats(ST_WIS_ARMOR, vault, mon, number);

			if (o_ptr->modifiers[OBJ_MOD_CON] != 0)
				add_stats(ST_CON_ARMOR, vault, mon, number);

			if (of_has(o_ptr->flags, OF_LIGHT_CURSE))
				add_stats(ST_CURSED_ARMOR, vault, mon, number);

			break;
		}

		/* weapons */
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:{

			/* do not include artifacts */
			if (o_ptr->artifact) break;

			/* add to weapon total */
			add_stats(ST_WEAPONS, vault, mon, number);

			/* check if bad, good, or average */
			if ((o_ptr->to_h < 0)  && (o_ptr->to_d < 0))
				add_stats(ST_BAD_WEAPONS, vault, mon, number);
			if ((o_ptr->to_h == 0) && (o_ptr->to_d == 0))
				add_stats(ST_AVERAGE_WEAPONS, vault, mon, number);
			if ((o_ptr->to_h > 0) && (o_ptr->to_d > 0))
				add_stats(ST_GOOD_WEAPONS, vault, mon, number);

			/* Egos by name - changes results a little */
			if (o_ptr->ego) {
				/* slay evil */
				if (strstr(o_ptr->ego->name, "of Slay Evil"))
					add_stats(ST_SLAYEVIL_WEAPONS, vault, mon, number);

				/* slay weapons */
				else if (strstr(o_ptr->ego->name, "of Slay"))
					add_stats(ST_SLAY_WEAPONS, vault, mon, number);
				/* kill flag */
				if (strstr(o_ptr->ego->name, "of *Slay"))
					add_stats(ST_KILL_WEAPONS, vault, mon, number);

				/* determine westernesse by flags */
				if (strstr(o_ptr->ego->name, "Westernesse"))
					add_stats(ST_WESTERNESSE_WEAPONS, vault, mon, number);

				/* determine defender by flags */
				if (strstr(o_ptr->ego->name, "Defender"))
					add_stats(ST_DEFENDER_WEAPONS, vault, mon, number);

				/* determine gondolin by flags */
				if (strstr(o_ptr->ego->name, "Gondolin"))
					add_stats(ST_GONDOLIN_WEAPONS, vault, mon, number);

				/* determine holy avenger by flags */
				if (strstr(o_ptr->ego->name, "Avenger"))
					add_stats(ST_HOLY_WEAPONS, vault, mon, number);

				/* is morgul */
				if (strstr(o_ptr->ego->name, "Morgul"))
					add_stats(ST_MORGUL_WEAPONS, vault, mon, number);
			}

			/* branded weapons */
			if (o_ptr->brands)
				add_stats(ST_BRAND_WEAPONS, vault, mon, number);

			/* extra blows */
			if (o_ptr->modifiers[OBJ_MOD_BLOWS] > 0)
				add_stats(ST_XTRABLOWS_WEAPONS, vault, mon, number);

			/* telepathy */
			if (of_has(o_ptr->flags, OF_TELEPATHY))
				add_stats(ST_TELEP_WEAPONS, vault, mon, number);

			/* is a top of the line weapon */
			if (((o_ptr->tval == TV_HAFTED) &&
				 (!strstr(o_ptr->kind->name, "Disruption"))) ||
				((o_ptr->tval == TV_POLEARM) &&
				 (!strstr(o_ptr->kind->name, "Slicing"))) ||
				((o_ptr->tval == TV_SWORD) &&
				 (!strstr(o_ptr->kind->name, "Chaos")))) {
				add_stats(ST_HUGE_WEAPONS, vault, mon, number);

				/* is uber need to fix ACB
				if ((of_has(o_ptr->flags, OF_SLAY_EVIL)) || (o_ptr->modifiers[OBJ_MOD_BLOWS] > 0))
				add_stats(ST_UBWE, vault, mon, number); */

			}

			break;
		}

		/* launchers */
		case TV_BOW:{

			/* do not include artifacts */
			if (o_ptr->artifact) break;

			/* add to launcher total */
			add_stats(ST_BOWS, vault, mon, number);

			/* check if bad, average, good, or very good */
			if ((o_ptr->to_h < 0) && (o_ptr->to_d < 0))
				add_stats(ST_BAD_BOWS, vault, mon, number);
			if ((o_ptr->to_h == 0) && (o_ptr->to_d == 0))
				add_stats(ST_AVERAGE_BOWS, vault, mon, number);
			if ((o_ptr->to_h > 0) && (o_ptr->to_d > 0))
				add_stats(ST_GOOD_BOWS, vault, mon, number);
			if ((o_ptr->to_h > 15) || (o_ptr->to_d > 15))
				add_stats(ST_VERYGOOD_BOWS, vault, mon, number);

			/* check long bows and xbows for xtra might and/or shots */
			if (o_ptr->pval > 2)
			{
				if (o_ptr->modifiers[OBJ_MOD_SHOTS] > 0)
					add_stats(ST_XTRASHOTS_BOWS, vault, mon, number);

				if (o_ptr->modifiers[OBJ_MOD_MIGHT] > 0)
					add_stats(ST_XTRAMIGHT_BOWS, vault, mon, number);
			}

			/* check for buckland */
			if ((o_ptr->pval == 2) &&
				kf_has(o_ptr->kind->kind_flags, KF_SHOOTS_SHOTS) &&
				(o_ptr->modifiers[OBJ_MOD_MIGHT] > 0) &&
				(o_ptr->modifiers[OBJ_MOD_SHOTS] > 0))
					add_stats(ST_BUCKLAND_BOWS, vault, mon, number);

			/* has telep */
			if (of_has(o_ptr->flags, OF_TELEPATHY))
				add_stats(ST_TELEP_BOWS, vault, mon, number);

			/* is cursed */
			if (of_has(o_ptr->flags, OF_LIGHT_CURSE))
				add_stats(ST_CURSED_BOWS, vault, mon, number);
			break;
		}

		/* potion */
		case TV_POTION:{

			/* add total amounts */
			add_stats(ST_POTIONS, vault, mon, number);

			/* get effects */
			effect = object_effect(o_ptr);
			 
			/*stat gain*/
			switch(effect){

				case EF_GAIN_STR:
				case EF_GAIN_INT:
				case EF_GAIN_WIS:
				case EF_GAIN_DEX:
				case EF_GAIN_CON:{

					add_stats(ST_GAINSTAT_POTIONS, vault, mon, number);
					break;
				}

				/* Aug */
				case EF_GAIN_ALL:{

					/*Augmentation counts as 5 stat gain pots */
					add_stats(ST_GAINSTAT_POTIONS, vault, mon, number*5);
					
				}

				case EF_ENLIGHTENMENT2:{

					/* *Enlight* counts as 2 stat pots */
					
					add_stats(ST_GAINSTAT_POTIONS, vault, mon, number*2);
					
				}

				case EF_RESTORE_MANA:{

					add_stats(ST_RESTOREMANA_POTIONS, vault, mon, number);
					break;
				}

				case EF_CURE_FULL:{
					add_stats(ST_HEALING_POTIONS, vault, mon, number);
					break;
				}
				
				case EF_CURE_NONORLYBIG:
				case EF_CURE_FULL2:{

					add_stats(ST_BIGHEAL_POTIONS, vault, mon, number);
					break;
				}

			}
			break;
		}

		/* scrolls */
		case TV_SCROLL:{

			/* add total amounts */
			add_stats(ST_SCROLLS, vault, mon, number);

			/* get effects */
			effect=object_effect(o_ptr);

			/* scroll effects */
			switch(effect){

				case EF_BANISHMENT:
				case EF_LOSKILL:
				case EF_RUNE:
				case EF_DESTRUCTION2:{

					/* add to total */
					add_stats(ST_ENDGAME_SCROLLS, vault, mon, number);
					break;
				}

				case EF_ACQUIRE:{

					/* add to total */
					add_stats(ST_ACQUIRE_SCROLLS, vault, mon, number);
					break;
				}

				case EF_ACQUIRE2:{

					/* do the effect of 2 acquires */
					add_stats(ST_ACQUIRE_SCROLLS, vault, mon, number*2);
					break;
				}
			}
			break;
		}

		/* rods */
		case TV_ROD:{

			/* add to total */
			add_stats(ST_RODS, vault, mon, number);

			effect=object_effect(o_ptr);

			switch(effect){

				/* utility */
				case EF_DETECT_TRAP:
				case EF_DETECT_TREASURE:
				case EF_DETECT_DOORSTAIR:
				case EF_LIGHT_LINE:
				case EF_ILLUMINATION:{

					add_stats(ST_UTILITY_RODS, vault, mon, number);
					break;
				}

				/* teleport other */
				case EF_TELE_OTHER:{

					add_stats(ST_TELEPOTHER_RODS, vault, mon, number);
					break;
				}

				/* detect all */
				case EF_DETECT_ALL:{

					add_stats(ST_DETECTALL_RODS, vault, mon, number);
					break;
				}

				/* endgame, speed and healing */
				case EF_HASTE:
				case EF_HEAL3:{

					add_stats(ST_ENDGAME_RODS, vault, mon, number);
					break;
				}
			}

			break;
		}

		/* staves */
		case TV_STAFF:{

			add_stats(ST_STAVES, vault, mon, number);

			effect=object_effect(o_ptr);

			switch(effect){

				case EF_HASTE:{

					add_stats(ST_SPEED_STAVES, vault, mon, number);
					break;
				}

				case EF_DESTRUCTION2:{

					add_stats(ST_DESTRUCTION_STAVES, vault, mon, number);
					break;
				}

				case EF_DISPEL_EVIL60:
				case EF_DISPEL_ALL:
				case EF_STAFF_HOLY:{

					add_stats(ST_KILL_STAVES, vault, mon, number);
					break;
				}

				case EF_CURE_FULL:
				case EF_BANISHMENT:
				case EF_STAFF_MAGI:{

					add_stats(ST_ENDGAME_STAVES, vault, mon, number);
					break;
				}
			}
			break;
		}

		case TV_WAND:{

			add_stats(ST_WANDS, vault, mon, number);

			effect=object_effect(o_ptr);

			switch(effect){

				case EF_TELE_OTHER:{

					add_stats(ST_TELEPOTHER_WANDS, vault, mon, number);
					break;
				}
			}
			break;
		}

		case TV_RING:{

			add_stats(ST_RINGS, vault, mon, number);

			/* is it cursed */
			if (of_has(o_ptr->flags,OF_LIGHT_CURSE))
				add_stats(ST_CURSED_RINGS, vault, mon, number);

			if (strstr(o_ptr->kind->name, "Speed")) {
				add_stats(ST_SPEEDS_RINGS, vault, mon, number);
			} else if ((strstr(o_ptr->kind->name, "Strength")) ||
					   (strstr(o_ptr->kind->name, "Intelligence")) ||
					   (strstr(o_ptr->kind->name, "Dexterity")) ||
					   (strstr(o_ptr->kind->name, "Constitution"))) {
				add_stats(ST_STAT_RINGS, vault, mon, number);
			} else if (strstr(o_ptr->kind->name, "Resist Poison")) {
				add_stats(ST_RPOIS_RINGS, vault, mon, number);
			} else if (strstr(o_ptr->kind->name, "Free Action")) {
				add_stats(ST_FA_RINGS, vault, mon, number);
			} else if (strstr(o_ptr->kind->name, "See invisible")) {
				add_stats(ST_SI_RINGS, vault, mon, number);
			} else if ((strstr(o_ptr->kind->name, "Flames")) ||
					   (strstr(o_ptr->kind->name, "Ice")) ||
					   (strstr(o_ptr->kind->name, "Acid")) ||
					   (strstr(o_ptr->kind->name, "Lightning"))) {
				add_stats(ST_BRAND_RINGS, vault, mon, number);
			} else if ((strstr(o_ptr->kind->name, "Fire")) ||
					   (strstr(o_ptr->kind->name, "Adamant")) ||
					   (strstr(o_ptr->kind->name, "Firmament"))) {
				add_stats(ST_ELVEN_RINGS, vault, mon, number);
			} else if (strstr(o_ptr->kind->name, "Power")) {
				add_stats(ST_ONE_RINGS, vault, mon, number);
			}


			break;
		}

		case TV_AMULET:{

			add_stats(ST_AMULETS, vault, mon, number);

			if (strstr(o_ptr->kind->name, "Wisdom")) {
				add_stats(ST_WIS_AMULETS, vault, mon, number);
			} else if ((strstr(o_ptr->kind->name, "Magi")) || 
					   (strstr(o_ptr->kind->name, "Trickery")) ||
					   (strstr(o_ptr->kind->name, "Weaponmastery"))) {
				add_stats(ST_ENDGAME_AMULETS, vault, mon, number);
			} else if (strstr(o_ptr->kind->name, "ESP")) {
				add_stats(ST_TELEP_AMULETS, vault, mon, number);
			}

			/* is cursed */
			if (of_has(o_ptr->flags, OF_LIGHT_CURSE))
				add_stats(ST_CURSED_AMULETS, vault, mon, number);

			break;
		}

		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:{

			add_stats(ST_AMMO, vault, mon, number);

			/* check if bad, average, good */
			if ((o_ptr->to_h < 0) && (o_ptr->to_d < 0))
				add_stats(ST_BAD_AMMO, vault, mon, number);
			if ((o_ptr->to_h == 0) && (o_ptr->to_d == 0))
				add_stats(ST_AVERAGE_AMMO, vault, mon, number);
			if ((o_ptr->to_h > 0) && (o_ptr->to_d > 0))
				add_stats(ST_GOOD_AMMO, vault, mon, number);

			if (o_ptr->ego)
				add_stats(ST_BRANDSLAY_AMMO, vault, mon, number);

			if (strstr(o_ptr->kind->name, "Seeker") ||
				strstr(o_ptr->kind->name, "Mithril")) {

				/* Mithril and seeker ammo */
				add_stats(ST_VERYGOOD_AMMO, vault, mon, number);

				/* Ego mithril and seeker ammo */
				if (o_ptr->ego) {
					add_stats(ST_AWESOME_AMMO, vault, mon, number);

					if (strstr(o_ptr->ego->name, "of Slay Evil"))
						add_stats(ST_SLAYEVIL_AMMO, vault, mon, number);

					if (strstr(o_ptr->ego->name, "of Holy Might"))
						add_stats(ST_HOLY_AMMO, vault, mon, number);
				}
			}
			break;
		}

		/* prayer books and magic books have the same probability 
		   only track one of them */
		case TV_MAGIC_BOOK:{

			switch(o_ptr->sval){

				/* svals begin at 0 and end at 8 */
				case 0:{

					add_stats(ST_1ST_BOOKS, vault, mon, number);
					first_find(ST_FF_BOOK1);
					break;
				}

				case 1:{

					add_stats(ST_2ND_BOOKS, vault, mon, number);
					first_find(ST_FF_BOOK2);
					break;
				}

				case 2:{

					add_stats(ST_3RD_BOOKS, vault, mon, number);
					first_find(ST_FF_BOOK3);
					break;
				}

				case 3:{

					add_stats(ST_4TH_BOOKS, vault, mon, number);
					first_find(ST_FF_BOOK4);
					break;
				}

				case 4:{

					add_stats(ST_5TH_BOOKS, vault, mon, number);
					first_find(ST_FF_BOOK5);
					break;
				}

				case 5:{

					add_stats(ST_6TH_BOOKS, vault, mon, number);
					first_find(ST_FF_BOOK6);
					break;
				}

				case 6:{

					add_stats(ST_7TH_BOOKS, vault, mon, number);
					first_find(ST_FF_BOOK7);
					break;
				}

				case 7:{

					add_stats(ST_8TH_BOOKS, vault, mon, number);
					first_find(ST_FF_BOOK8);
					break;
				}

				case 8:{

					add_stats(ST_9TH_BOOKS, vault, mon, number);
					first_find(ST_FF_BOOK9);
					break;
				}


			}
			break;
		}
	}
	/* check to see if we have an artifact */
	if (o_ptr->artifact){

		/* add to artifact level total */
		art_total[lvl] += addval;

		/* add to the artifact iteration total */
		if (iter < TRIES_SIZE) art_it[iter]++;

		/* Obtain the artifact info */
		a_ptr = o_ptr->artifact;

		//debugging, print out that we found the artifact
		//msg_format("Found artifact %s",a_ptr->name);

		/* artifact is shallow */
		if (a_ptr->alloc_min < (player->depth - 20)) art_shal[lvl] += addval;

		/* artifact is close to the player depth */
		if ((a_ptr->alloc_min >= player->depth - 20) &&
			(a_ptr->alloc_min <= player->depth )) art_ave[lvl] += addval;

		/* artifact is out of depth */
		if (a_ptr->alloc_min > (player->depth)) art_ood[lvl] += addval;

		/* check to see if it's a special artifact */
		if ((o_ptr->tval == TV_LIGHT) || (o_ptr->tval == TV_AMULET)
			|| (o_ptr->tval == TV_RING)){

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
			if (vault){

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
	if (o_ptr->tval == TV_GOLD){

		int temp = o_ptr->pval;
		gold_temp = temp;
	    gold_total[lvl] += (gold_temp / tries);

		/*From a monster? */
		if ((mon) || (uniq)) gold_mon[lvl] += (gold_temp / tries);
		else gold_floor[lvl] += (gold_temp / tries);
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
void monster_death_stats(int m_idx)
{
	int y, x;
	s16b this_o_idx, next_o_idx = 0;

	monster_type *m_ptr;

	bool uniq;

	assert(m_idx > 0);
	m_ptr = cave_monster(cave, m_idx);


	/* Check if monster is UNIQUE */
	uniq = rf_has(m_ptr->race->flags,RF_UNIQUE);

	/* Get the location */
	y = m_ptr->fy;
	x = m_ptr->fx;

	/* Delete any mimicked objects */
	if (m_ptr->mimicked_o_idx > 0)
		delete_object_idx(m_ptr->mimicked_o_idx);

	/* Drop objects being carried */
	for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx) {
		object_type *o_ptr;

		/* Get the object */
		o_ptr = cave_object(cave, this_o_idx);

		/* Line up the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Paranoia */
		o_ptr->held_m_idx = 0;

		/* Get data */
		get_obj_data(o_ptr, y, x, TRUE, uniq);

		/* delete the object */
		delete_object_idx(this_o_idx);
	}

	/* Forget objects */
	m_ptr->hold_o_idx = 0;
}



/* This will collect stats on a monster avoiding all
 * unique monsters.  Afterwards it will kill the
 * monsters.
 */

static bool stats_monster(monster_type *m_ptr, int i)
{
	static int lvl;

	/* get player depth */
	lvl=player->depth;


	/* Increment monster count */
	mon_total[lvl] += addval;

	/* Increment unique count if appropriate */
	if (rf_has(m_ptr->race->flags, RF_UNIQUE)){

		/* add to total */
		uniq_total[lvl] += addval;

		/* kill the unique if we're in clearing mode */
		if (clearing) m_ptr->race->max_num = 0;

		//debugging print that we killed it
		//msg_format("Killed %s",r_ptr->name);
	}

	/* Is it mostly dangerous (10 levels ood or less?)*/
	if ((m_ptr->race->level > player->depth) && 
		(m_ptr->race->level <= player->depth+10)){

			mon_ood[lvl] += addval;

			/* Is it a unique */
			if (rf_has(m_ptr->race->flags, RF_UNIQUE)) uniq_ood[lvl] += addval;

		}


	/* Is it deadly? */
	if (m_ptr->race->level > player->depth + 10){

		mon_deadly[lvl] += addval;

		/* Is it a unique? */
		if (rf_has(m_ptr->race->flags, RF_UNIQUE)) uniq_deadly[lvl] += addval;

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
	j_ptr = cave_object(cave, o_idx);


	/* Wipe the object */
	object_wipe(j_ptr);

	/* Count objects */
	cave->obj_cnt--;
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
	file_putf(stats_log," Potions:   Aug counts as 5 potions, *enlight* as 2.  Healing potions are \n");
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

	int i;
	
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

		
	for (i=ST_BEGIN; i<ST_END; i++){	
		file_putf(stats_log, "%s%f From Monsters: %f In Vaults: %f \n",	stat_message[i].name, stat_all[i][0][lvl], stat_all[i][1][lvl], stat_all[i][2][lvl]);
	}	


}

/*
 *	Compute and print the mean and standard deviation for an array of known size*
 */
static void mean_and_stdv(int array[TRIES_SIZE])
{
	int k, maxiter;
	double tot=0, mean, stdev, temp=0;

	/* get the maximum iteration value */
	maxiter = MIN(tries, TRIES_SIZE); 

	/* sum the array */
	for (k = 0; k < maxiter; k++) tot += array[k];

	/* compute the mean */
	mean = tot / maxiter;

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
	for (lvl = 1; lvl < MAX_LVL ; lvl++){

		/* calculate the probability of not finding the stat */
		tmpfind=(1 - stat[lvl]);

		/* maximum probability is 98% */
		if (tmpfind < 0.02) tmpfind = 0.02;

		/* multiply probabilities of not finding */
		if (find <= 0) find = tmpfind; else find *= tmpfind;

		/* increase count to 5 */
		tmpcount++;

		/* print output every 5 levels */
		if (tmpcount == 5){

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

	for (k = 0; k < MAX_LVL; k++){

		out += stat[k];
	}

	return out;
} 
#endif
/* post process select items */
static void post_process_stats(void)
{
	double arttot;
	int i,k;

	/* output a title */
	file_putf(stats_log,"\n");
	file_putf(stats_log,"***** POST PROCESSING *****\n");
	file_putf(stats_log,"\n");
	file_putf(stats_log,"Item \t5\t\t\t10\t\t\t15\t\t\t20\t\t\t25\t\t\t");
	file_putf(stats_log,"30\t\t\t35\t\t\t40\t\t\t45\t\t\t50\t\t\t");
	file_putf(stats_log,"55\t\t\t60\t\t\t65\t\t\t70\t\t\t75\t\t\t");
	file_putf(stats_log,"80\t\t\t85\t\t\t90\t\t\t95\t\t\t100\n");
	
	for (i = 1; i < ST_FF_END; i++){
	
			file_putf(stats_log, stat_ff_message[i].name);
			prob_of_find(stat_all[stat_ff_message[i].st][0]);
			mean_and_stdv(stat_ff_all[i]);
	}

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
static void scan_for_objects(void)
{ 
	int y, x;

	for (y = 1; y < cave->height - 1; y++) {
		for (x = 1; x < cave->width - 1; x++) {
			const object_type *o_ptr;


			while ((o_ptr = get_first_object(y, x))) {
				/* get data on the object */
				get_obj_data(o_ptr, y, x, FALSE, FALSE);

				/* delete the object */
				delete_object_stat(cave->o_idx[y][x]);
			}
		}
	}
}

/*
 * This will scan the dungeon for monsters and then kill each
 * and every last one.
*/
static void scan_for_monsters(void)
{ 
	int i;

	/* Go through the monster list */
	for (i = 1; i < cave_monster_max(cave); i++){

		monster_type *m_ptr = cave_monster(cave, i);

		/* Skip dead monsters */
		if (!m_ptr->race) continue;

		stats_monster(m_ptr,i);
	}
}
/*
 * This is the entry point for generation statistics.
 */

static void stats_collect_level(void)
{
	/* Make a dungeon */
	cave_generate(cave,player);

	/* Scan for objects, these are floor objects */
	scan_for_objects();

	/* Get stats (and kill) all non-unique monsters */
	scan_for_monsters();

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
	for (i = 0; z_info && i < z_info->a_max; i++){

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

	for (i = 1; i < z_info->r_max - 1; i++){

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
	for (depth = 0; depth < MAX_LVL; depth += 5){

		player->depth = depth;
		if (player->depth == 0) player->depth = 1;

		/* do many iterations of each level */
		for (iter = 0; iter < tries; iter++){

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
	for (iter=0; iter < tries; iter++){

		/* move all artifacts to uncreated */
		uncreate_artifacts();

		/* move all uniques to alive */
		revive_uniques();

		/* do randart regen */
		if ((regen) && (iter<tries)){

			/* get seed */
			int seed_randart=randint0(0x10000000);

			/* regen randarts */
			do_randart(seed_randart,TRUE);

		}

		/* do game iterations */
		for (depth = 1 ; depth < MAX_LVL; depth++){

			/* debug */
			//msg_format("Attempting level %d",depth);

			/* move player to that depth */
			player->depth = depth;

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
	if (temp == 2){

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
	if (!stats_log){

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
	 
	 if (!OPT(auto_more)){
	 
		/* remember that we turned off auto_more */
		auto_flag = TRUE;

		/* Turn on auto-more */
		option_set(option_name(OPT_auto_more),TRUE);

	}

	/* print heading for the file */
	print_heading();

	/* make sure all stats are 0 */
	init_stat_vals();

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
	for (y = 1; y < DUNGEON_HGT - 1; y++){

			for (x = 1; x < DUNGEON_WID - 1; x++){

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
	oy = d_y_old[0] = player->py;
	ox = d_x_old[0] = player->px;
	d_old_max = 1;

	/* distance from player starts at 0*/
	dist=0;

	/* Assign the distance value to the first square (player)*/
	cave_dist[oy][ox] = dist;

	do{
		d_new_max = 0;
		dist++;

		/* Loop over all visited squares of the previous iteration*/
		for(i=0 ;i < d_old_max; i++){

			/* Get the square we want to look at */
			oy = d_y_old[i];
			ox = d_x_old[i];

			//debug
			//msg("x: %d y: %d dist: %d %d ",ox,oy,dist-1,i);

			/* Get all adjacent squares */
			for (d = 0; d < 8; d++){

				/* Adjacent square location */
				ty = oy + ddy_ddd[d];
				tx = ox + ddx_ddd[d];

				if (!(square_in_bounds_fully(cave, ty,tx))) continue;

				/* Have we been here before? */
				if (cave_dist[ty][tx] >= 0) continue;

				/* Is it a wall? */
				if (square_iswall(cave, ty, tx)) continue;

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
		for (i=0 ;i<d_new_max; i++){

			d_y_old[i] = d_y_new[i];
			d_x_old[i] = d_x_new[i];
		}

		d_old_max = d_new_max;


	} while ((d_old_max > 0) || dist == DIST_MAX);
}

void pit_stats(void)
{
	int tries = 1000;
	int depth = 0;
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

	/* Format second default value */
	strnfmt(tmp_val, sizeof(tmp_val), "%d", player->depth);

	/* Ask for the input - take the first 7 characters*/
	if (!get_string("Depth: ", tmp_val, 7)) return;

	/* get the new value */
	depth = atoi(tmp_val);
	if (depth < 1) depth = 1;

	for (j = 0; j < tries; j++){

		int i;
		int pit_idx = 0;
		int pit_dist = 999;

		for (i = 0; i < z_info->pit_max; i++){

			int offset, dist;
			pit_profile *pit = &pit_info[i];

			if (!pit->name || pit->room_type != type) continue;

			offset = Rand_normal(pit->ave, 10);
			dist = ABS(offset - depth);

			if (dist < pit_dist && one_in_(pit->rarity)){

				pit_idx = i;
				pit_dist = dist;
			}
		}

		hist[pit_idx]++;
	}

	for (p = 0; p < z_info->pit_max; p++){

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

	for (i = 1; i <= tries; i++){

	/* assume no disconnected areas */
	has_dsc = FALSE;

	/* assume you can't get to stairs */
	has_dsc_from_stairs = TRUE;

		/* Make a new cave */
		cave_generate(cave,player);

		/* Fill the distance array */
		calc_cave_distances();

		/*Cycle through the dungeon */
		for (y = 1; y < cave->height - 1; y++){

			for (x = 1; x < cave->width - 1; x++){

				/* don't care about walls */
				if (square_iswall(cave, y, x)) continue;

				/* Can we get there? */
				if (cave_dist[y][x] >= 0){

					/* Is it a  down stairs? */
					if (square_isdownstairs(cave, y, x)) {

						has_dsc_from_stairs = FALSE;

						//debug
						//msg("dist to stairs: %d",cave_dist[y][x]);

					}

					continue;
				}

				/* Ignore vaults as they are often disconnected */
				if (square_isvault(cave, y, x)) continue;

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
