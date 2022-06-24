/**
 * \file wiz-stats.c
 * \brief Statistics collection on dungeon generation
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
#include "effects.h"
#include "generate.h"
#include "init.h"
#include "mon-make.h"
#include "monster.h"
#include "obj-init.h"
#include "obj-pile.h"
#include "obj-randart.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "object.h"
#include "ui-command.h"
#include "wizard.h"

/**
 * The stats programs here will provide information on the dungeon, the monsters
 * in it, and the items that they drop.  Statistics are gotten from a given
 * level by generating a new level, collecting all the items (noting if they
 * were generated in a vault).  Then all non-unique monsters are killed and
 * their stats are tracked.
 * The items from these monster drops are then collected and analyzed.  Lastly,
 * all unique monsters are killed, and their drops are analyzed.  In this way,
 * it is possible to separate unique drops and normal monster drops.
 *
 * There are two options for simulating the entirety of the dungeon.  There is
 * a "diving" option that begins each level with all artifacts and uniques
 * available; and there is a "level-clearing" option that simulates all 100
 * levels of the dungeon, removing artifacts and uniques as they are
 * discovered/killed.  "diving" option only catalogues every 5 levels.
 *
 * At the end of the "level-clearing" log file, extra post-processing is done
 * to find the mean and standard deviation for the level you are likely to
 * first gain an item with a key resistance or item.
 * 
 * In addition to these sims there is a shorter sim that tests for dungeon
 * connectivity.
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
bool clearing = false;
/* flag for regenning randart */
bool regen = false;

/*** These are items to track for each iteration ***/
/* total number of artifacts found */
static int art_it[TRIES_SIZE];

/*** handle gold separately ***/
/* gold */
static double gold_total[MAX_LVL], gold_floor[MAX_LVL], gold_mon[MAX_LVL];


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
	const char *name;
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
	const char *name;
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
static void init_stat_vals(void)
{
	int i,j,k;

	for (i = 0; i < ST_END;i++)
		for (j = 0; j < 3; k = j++)
			for (k = 0; k < MAX_LVL; k++)
				stat_all[i][j][k] = 0.0;
				
	for (i = 1; i < TRIES_SIZE; i++)
		art_it[i] = 0;
	
	for (i = 0; i < ST_FF_END; i++)
		for (j = 0; j < TRIES_SIZE; j++)
			stat_ff_all[i][j] = 0.0;
}

/*
 *	Record the first level we find something
 */
static bool first_find(stat_first_find st)
{
	/* make sure we're not on an iteration above our array limit */
	if (iter >= TRIES_SIZE) return false;

	/* make sure we haven't found it earlier on this iteration */
	if (stat_ff_all[st][iter] > 0) return false;

	/* assign the depth to this value */
	stat_ff_all[st][iter] = player->depth;

	/* success */
	return true;
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
static void get_obj_data(const struct object *obj, int y, int x, bool mon,
						 bool uniq)
{

	bool vault = square_isvault(cave, loc(x, y));
	int number = obj->number;
	static int lvl;
	const struct artifact *art;

	double gold_temp = 0;

	assert(obj->kind);

	/* get player depth */
	lvl = player->depth;

	/* check for some stuff that we will use regardless of type */
	/* originally this was armor, but I decided to generalize it */

	/* has free action (hack: don't include Inertia)*/
	if (of_has(obj->flags, OF_FREE_ACT) && 
		!((obj->tval == TV_AMULET) &&
		  (!strstr(obj->kind->name, "Inertia")))) {

			/* add the stats */
			add_stats(ST_FA_EQUIPMENT, vault, mon, number);

			/* record first level */
			first_find(ST_FF_FA);
		}


	/* has see invis */
	if (of_has(obj->flags, OF_SEE_INVIS)){

		add_stats(ST_SI_EQUIPMENT, vault, mon, number);
		first_find(ST_FF_SI);
	}
	/* has at least one basic resist */
 	if ((obj->el_info[ELEM_ACID].res_level == 1) ||
		(obj->el_info[ELEM_ELEC].res_level == 1) ||
		(obj->el_info[ELEM_COLD].res_level == 1) ||
		(obj->el_info[ELEM_FIRE].res_level == 1)){

			add_stats(ST_RESIST_EQUIPMENT, vault, mon, number);
	}

	/* has rbase */
	if ((obj->el_info[ELEM_ACID].res_level == 1) &&
		(obj->el_info[ELEM_ELEC].res_level == 1) &&
		(obj->el_info[ELEM_COLD].res_level == 1) &&
		(obj->el_info[ELEM_FIRE].res_level == 1))
		add_stats(ST_RBASE_EQUIPMENT, vault, mon, number);

	/* has resist poison */
	if (obj->el_info[ELEM_POIS].res_level == 1){

		add_stats(ST_RPOIS_EQUIPMENT, vault, mon, number);
		first_find(ST_FF_RPOIS);
		
	}
	/* has resist nexus */
	if (obj->el_info[ELEM_NEXUS].res_level == 1){

		add_stats(ST_RNEXUS_EQUIPMENT, vault, mon, number);
		first_find(ST_FF_RNEXUS);
	}
	/* has resist blind */
	if (of_has(obj->flags, OF_PROT_BLIND)){

		add_stats(ST_RBLIND_EQUIPMENT, vault, mon, number);
		first_find(ST_FF_RBLIND);
	}

	/* has resist conf */
	if (of_has(obj->flags, OF_PROT_CONF)){

		add_stats(ST_RCONF_EQUIPMENT, vault, mon, number);
		first_find(ST_FF_RCONF);
	}

	/* has speed */
	if (obj->modifiers[OBJ_MOD_SPEED] != 0)
		add_stats(ST_SPEED_EQUIPMENT, vault, mon, number);

	/* has telepathy */
	if (of_has(obj->flags, OF_TELEPATHY)){

		add_stats(ST_TELEP_EQUIPMENT, vault, mon, number);
		first_find(ST_FF_TELEP);
	}

	switch(obj->tval){

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
			if (obj->artifact) break;

			/* add to armor total */
			add_stats(ST_ARMORS, vault, mon, number);

			/* check if bad, good, or average */
			if (obj->to_a < 0)
				add_stats(ST_BAD_ARMOR, vault, mon, number);
			if (obj->to_h == 0)
				add_stats(ST_AVERAGE_ARMOR, vault, mon, number);
			if (obj->to_h > 0)
				add_stats(ST_GOOD_ARMOR, vault, mon, number);

			/* has str boost */
			if (obj->modifiers[OBJ_MOD_STR] != 0)
				add_stats(ST_STR_ARMOR, vault, mon, number);

			/* has dex boost */
			if (obj->modifiers[OBJ_MOD_DEX] != 0)
				add_stats(ST_DEX_ARMOR, vault, mon, number);

			/* has int boost */
			if (obj->modifiers[OBJ_MOD_INT] != 0)
				add_stats(ST_INT_ARMOR, vault, mon, number);

			if (obj->modifiers[OBJ_MOD_WIS] != 0)
				add_stats(ST_WIS_ARMOR, vault, mon, number);

			if (obj->modifiers[OBJ_MOD_CON] != 0)
				add_stats(ST_CON_ARMOR, vault, mon, number);

			if (obj->curses)
				add_stats(ST_CURSED_ARMOR, vault, mon, number);

			break;
		}

		/* weapons */
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:{

			/* do not include artifacts */
			if (obj->artifact) break;

			/* add to weapon total */
			add_stats(ST_WEAPONS, vault, mon, number);

			/* check if bad, good, or average */
			if ((obj->to_h < 0)  && (obj->to_d < 0))
				add_stats(ST_BAD_WEAPONS, vault, mon, number);
			if ((obj->to_h == 0) && (obj->to_d == 0))
				add_stats(ST_AVERAGE_WEAPONS, vault, mon, number);
			if ((obj->to_h > 0) && (obj->to_d > 0))
				add_stats(ST_GOOD_WEAPONS, vault, mon, number);

			/* Egos by name - changes results a little */
			if (obj->ego) {
				/* slay evil */
				if (strstr(obj->ego->name, "of Slay Evil"))
					add_stats(ST_SLAYEVIL_WEAPONS, vault, mon, number);

				/* slay weapons */
				else if (strstr(obj->ego->name, "of Slay"))
					add_stats(ST_SLAY_WEAPONS, vault, mon, number);
				/* kill flag */
				if (strstr(obj->ego->name, "of *Slay"))
					add_stats(ST_KILL_WEAPONS, vault, mon, number);

				/* determine westernesse by flags */
				if (strstr(obj->ego->name, "Westernesse"))
					add_stats(ST_WESTERNESSE_WEAPONS, vault, mon, number);

				/* determine defender by flags */
				if (strstr(obj->ego->name, "Defender"))
					add_stats(ST_DEFENDER_WEAPONS, vault, mon, number);

				/* determine gondolin by flags */
				if (strstr(obj->ego->name, "Gondolin"))
					add_stats(ST_GONDOLIN_WEAPONS, vault, mon, number);

				/* determine holy avenger by flags */
				if (strstr(obj->ego->name, "Avenger"))
					add_stats(ST_HOLY_WEAPONS, vault, mon, number);

				/* is morgul */
				if (strstr(obj->ego->name, "Morgul"))
					add_stats(ST_MORGUL_WEAPONS, vault, mon, number);
			}

			/* branded weapons */
			if (obj->brands)
				add_stats(ST_BRAND_WEAPONS, vault, mon, number);

			/* extra blows */
			if (obj->modifiers[OBJ_MOD_BLOWS] > 0)
				add_stats(ST_XTRABLOWS_WEAPONS, vault, mon, number);

			/* telepathy */
			if (of_has(obj->flags, OF_TELEPATHY))
				add_stats(ST_TELEP_WEAPONS, vault, mon, number);

			/* is a top of the line weapon */
			if (((obj->tval == TV_HAFTED) &&
				 (!strstr(obj->kind->name, "Disruption"))) ||
				((obj->tval == TV_POLEARM) &&
				 (!strstr(obj->kind->name, "Slicing"))) ||
				((obj->tval == TV_SWORD) &&
				 (!strstr(obj->kind->name, "Chaos")))) {
				add_stats(ST_HUGE_WEAPONS, vault, mon, number);

				/* is uber need to fix ACB
				if ((of_has(obj->flags, OF_SLAY_EVIL)) || (obj->modifiers[OBJ_MOD_BLOWS] > 0))
				add_stats(ST_UBWE, vault, mon, number); */

			}

			break;
		}

		/* launchers */
		case TV_BOW:{

			/* do not include artifacts */
			if (obj->artifact) break;

			/* add to launcher total */
			add_stats(ST_BOWS, vault, mon, number);

			/* check if bad, average, good, or very good */
			if ((obj->to_h < 0) && (obj->to_d < 0))
				add_stats(ST_BAD_BOWS, vault, mon, number);
			if ((obj->to_h == 0) && (obj->to_d == 0))
				add_stats(ST_AVERAGE_BOWS, vault, mon, number);
			if ((obj->to_h > 0) && (obj->to_d > 0))
				add_stats(ST_GOOD_BOWS, vault, mon, number);
			if ((obj->to_h > 15) || (obj->to_d > 15))
				add_stats(ST_VERYGOOD_BOWS, vault, mon, number);

			/* check long bows and xbows for xtra might and/or shots */
			if (obj->pval > 2)
			{
				if (obj->modifiers[OBJ_MOD_SHOTS] > 0)
					add_stats(ST_XTRASHOTS_BOWS, vault, mon, number);

				if (obj->modifiers[OBJ_MOD_MIGHT] > 0)
					add_stats(ST_XTRAMIGHT_BOWS, vault, mon, number);
			}

			/* check for buckland */
			if ((obj->pval == 2) &&
				kf_has(obj->kind->kind_flags, KF_SHOOTS_SHOTS) &&
				(obj->modifiers[OBJ_MOD_MIGHT] > 0) &&
				(obj->modifiers[OBJ_MOD_SHOTS] > 0))
					add_stats(ST_BUCKLAND_BOWS, vault, mon, number);

			/* has telep */
			if (of_has(obj->flags, OF_TELEPATHY))
				add_stats(ST_TELEP_BOWS, vault, mon, number);

			/* is cursed */
			if (obj->curses)
				add_stats(ST_CURSED_BOWS, vault, mon, number);
			break;
		}

		/* potion */
		case TV_POTION:{

			/* Add total amounts */
			add_stats(ST_POTIONS, vault, mon, number);

			/* Stat gain */
			if (strstr(obj->kind->name, "Strength") ||
				strstr(obj->kind->name, "Intelligence") ||
				strstr(obj->kind->name, "Wisdom") ||
				strstr(obj->kind->name, "Dexterity") ||
				strstr(obj->kind->name, "Constitution")) {
				add_stats(ST_GAINSTAT_POTIONS, vault, mon, number);
			} else if (strstr(obj->kind->name, "Augmentation")) {
				/* Augmentation counts as 5 stat gain pots */
				add_stats(ST_GAINSTAT_POTIONS, vault, mon, number * 5);
			} else if (strstr(obj->kind->name, "*Enlightenment*")) {
				/* *Enlight* counts as 2 stat pots */
				add_stats(ST_GAINSTAT_POTIONS, vault, mon, number * 2);
			} else if (strstr(obj->kind->name, "Restore Mana")) {
				add_stats(ST_RESTOREMANA_POTIONS, vault, mon, number);
			} else if ((strstr(obj->kind->name, "Life")) ||
					   (strstr(obj->kind->name, "*Healing*"))) {
				add_stats(ST_ELVEN_RINGS, vault, mon, number);
			} else if (strstr(obj->kind->name, "Healing")) {
				add_stats(ST_HEALING_POTIONS, vault, mon, number);
			}
			break;
		}

		/* scrolls */
		case TV_SCROLL:{

			/* add total amounts */
			add_stats(ST_SCROLLS, vault, mon, number);

			if (strstr(obj->kind->name, "Banishment") ||
				strstr(obj->kind->name, "Mass Banishment") ||
				strstr(obj->kind->name, "Rune of Protection") ||
				strstr(obj->kind->name, "*Destruction*")) {
				add_stats(ST_ENDGAME_SCROLLS, vault, mon, number);
			} else if (strstr(obj->kind->name, "Acquirement")) {
				add_stats(ST_ACQUIRE_SCROLLS, vault, mon, number);
			} else if (strstr(obj->kind->name, "*Acquirement*")) {
				/* do the effect of 2 acquires */
				add_stats(ST_ACQUIRE_SCROLLS, vault, mon, number * 2);
			}
			break;
		}

		/* rods */
		case TV_ROD:{

			/* add to total */
			add_stats(ST_RODS, vault, mon, number);

			if (strstr(obj->kind->name, "Trap Detection") ||
				strstr(obj->kind->name, "Treasure Detection") ||
				strstr(obj->kind->name, "Door/Stair Location") ||
				strstr(obj->kind->name, "Illumination") ||
				strstr(obj->kind->name, "Light")) {
				add_stats(ST_UTILITY_RODS, vault, mon, number);
			} else if (strstr(obj->kind->name, "Teleport Other")) {
				add_stats(ST_TELEPOTHER_RODS, vault, mon, number);
			} else if (strstr(obj->kind->name, "Detection")) {
				add_stats(ST_DETECTALL_RODS, vault, mon, number);
			} else if (strstr(obj->kind->name, "Speed") ||
					   strstr(obj->kind->name, "Healing")) {
				add_stats(ST_ENDGAME_RODS, vault, mon, number);
			}
			break;
		}

		/* staves */
		case TV_STAFF:{

			add_stats(ST_STAVES, vault, mon, number);

			if (strstr(obj->kind->name, "Speed")) {
				add_stats(ST_SPEED_STAVES, vault, mon, number);
			} else if (strstr(obj->kind->name, "*Destruction*")) {
				add_stats(ST_DESTRUCTION_STAVES, vault, mon, number);
			} else if (strstr(obj->kind->name, "Dispel Evil") ||
					   strstr(obj->kind->name, "Power") ||
					   strstr(obj->kind->name, "Holiness")) {
				add_stats(ST_KILL_STAVES, vault, mon, number);
			} else if (strstr(obj->kind->name, "Healing") ||
					   strstr(obj->kind->name, "Banishment") ||
					   strstr(obj->kind->name, "the Magi")) {
				add_stats(ST_ENDGAME_STAVES, vault, mon, number);
			}
			break;
		}

		case TV_WAND:{

			add_stats(ST_WANDS, vault, mon, number);

			if (strstr(obj->kind->name, "Teleport Other"))
				add_stats(ST_TELEPOTHER_WANDS, vault, mon, number);
			break;
		}

		case TV_RING:{

			add_stats(ST_RINGS, vault, mon, number);

			/* is it cursed */
			if (obj->curses)
				add_stats(ST_CURSED_RINGS, vault, mon, number);

			if (strstr(obj->kind->name, "Speed")) {
				add_stats(ST_SPEEDS_RINGS, vault, mon, number);
			} else if ((strstr(obj->kind->name, "Strength")) ||
					   (strstr(obj->kind->name, "Intelligence")) ||
					   (strstr(obj->kind->name, "Dexterity")) ||
					   (strstr(obj->kind->name, "Constitution"))) {
				add_stats(ST_STAT_RINGS, vault, mon, number);
			} else if (strstr(obj->kind->name, "Resist Poison")) {
				add_stats(ST_RPOIS_RINGS, vault, mon, number);
			} else if (strstr(obj->kind->name, "Free Action")) {
				add_stats(ST_FA_RINGS, vault, mon, number);
			} else if (strstr(obj->kind->name, "See invisible")) {
				add_stats(ST_SI_RINGS, vault, mon, number);
			} else if ((strstr(obj->kind->name, "Flames")) ||
					   (strstr(obj->kind->name, "Ice")) ||
					   (strstr(obj->kind->name, "Acid")) ||
					   (strstr(obj->kind->name, "Lightning"))) {
				add_stats(ST_BRAND_RINGS, vault, mon, number);
			} else if ((strstr(obj->kind->name, "Fire")) ||
					   (strstr(obj->kind->name, "Adamant")) ||
					   (strstr(obj->kind->name, "Firmament"))) {
				add_stats(ST_ELVEN_RINGS, vault, mon, number);
			} else if (strstr(obj->kind->name, "Power")) {
				add_stats(ST_ONE_RINGS, vault, mon, number);
			}


			break;
		}

		case TV_AMULET:{

			add_stats(ST_AMULETS, vault, mon, number);

			if (strstr(obj->kind->name, "Wisdom")) {
				add_stats(ST_WIS_AMULETS, vault, mon, number);
			} else if ((strstr(obj->kind->name, "Magi")) || 
					   (strstr(obj->kind->name, "Trickery")) ||
					   (strstr(obj->kind->name, "Weaponmastery"))) {
				add_stats(ST_ENDGAME_AMULETS, vault, mon, number);
			} else if (strstr(obj->kind->name, "ESP")) {
				add_stats(ST_TELEP_AMULETS, vault, mon, number);
			}

			/* is cursed */
			if (obj->curses)
				add_stats(ST_CURSED_AMULETS, vault, mon, number);

			break;
		}

		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:{

			add_stats(ST_AMMO, vault, mon, number);

			/* check if bad, average, good */
			if ((obj->to_h < 0) && (obj->to_d < 0))
				add_stats(ST_BAD_AMMO, vault, mon, number);
			if ((obj->to_h == 0) && (obj->to_d == 0))
				add_stats(ST_AVERAGE_AMMO, vault, mon, number);
			if ((obj->to_h > 0) && (obj->to_d > 0))
				add_stats(ST_GOOD_AMMO, vault, mon, number);

			if (obj->ego)
				add_stats(ST_BRANDSLAY_AMMO, vault, mon, number);

			if (strstr(obj->kind->name, "Seeker") ||
				strstr(obj->kind->name, "Mithril")) {

				/* Mithril and seeker ammo */
				add_stats(ST_VERYGOOD_AMMO, vault, mon, number);

				/* Ego mithril and seeker ammo */
				if (obj->ego) {
					add_stats(ST_AWESOME_AMMO, vault, mon, number);

					if (strstr(obj->ego->name, "of Slay Evil"))
						add_stats(ST_SLAYEVIL_AMMO, vault, mon, number);

					if (strstr(obj->ego->name, "of Holy Might"))
						add_stats(ST_HOLY_AMMO, vault, mon, number);
				}
			}
			break;
		}

		/* books have the same probability, only track one realm of them */
		case TV_MAGIC_BOOK:{

			switch(obj->sval){

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
	if (obj->artifact){

		/* add to artifact level total */
		art_total[lvl] += addval;

		/* add to the artifact iteration total */
		if (iter < TRIES_SIZE) art_it[iter]++;

		/* Obtain the artifact info */
		art = obj->artifact;

		//debugging, print out that we found the artifact
		//msg_format("Found artifact %s",art->name);

		/* artifact is shallow */
		if (art->alloc_min < (player->depth - 20)) art_shal[lvl] += addval;

		/* artifact is close to the player depth */
		if ((art->alloc_min >= player->depth - 20) &&
			(art->alloc_min <= player->depth )) art_ave[lvl] += addval;

		/* artifact is out of depth */
		if (art->alloc_min > (player->depth)) art_ood[lvl] += addval;

		/* check to see if it's a special artifact */
		if ((obj->tval == TV_LIGHT) || (obj->tval == TV_AMULET)
			|| (obj->tval == TV_RING)){

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
		if (!(clearing)) mark_artifact_created(art, false);
	}

	/* Get info on gold. */
	if (obj->tval == TV_GOLD){

		int temp = obj->pval;
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
static void monster_death_stats(int m_idx)
{
	struct object *obj;
	struct monster *mon;
	bool uniq;

	assert(m_idx > 0);
	mon = cave_monster(cave, m_idx);

	/* Check if monster is UNIQUE */
	uniq = rf_has(mon->race->flags,RF_UNIQUE);

	/* Mimicked objects will have already been counted as floor objects */
	mon->mimicked_obj = NULL;

	/* Drop objects being carried */
	obj = mon->held_obj;
	while (obj) {
		struct object *next = obj->next;

		/* Object no longer held */
		obj->held_m_idx = 0;

		/* Get data */
		get_obj_data(obj, mon->grid.y, mon->grid.x, true, uniq);

		/* Delete the object */
		delist_object(cave, obj);
		object_delete(cave, player->cave, &obj);

		/* Next */
		obj = next;
	}

	/* Forget objects */
	mon->held_obj = NULL;
}



/**
 * This will collect stats on a monster avoiding all unique monsters.
 * Afterwards it will kill the monsters.
 */
static bool stats_monster(struct monster *mon, int i)
{
	static int lvl;

	/* get player depth */
	lvl = player->depth;


	/* Increment monster count */
	mon_total[lvl] += addval;

	/* Increment unique count if appropriate */
	if (rf_has(mon->race->flags, RF_UNIQUE)){

		/* add to total */
		uniq_total[lvl] += addval;

		/* kill the unique if we're in clearing mode */
		if (clearing) mon->race->max_num = 0;

		/* debugging print that we killed it
		   msg_format("Killed %s",race->name); */
	}

	/* Is it mostly dangerous (10 levels ood or less?)*/
	if ((mon->race->level > player->depth) && 
		(mon->race->level <= player->depth + 10)) {

			mon_ood[lvl] += addval;

			/* Is it a unique */
			if (rf_has(mon->race->flags, RF_UNIQUE))
				uniq_ood[lvl] += addval;
	}


	/* Is it deadly? */
	if (mon->race->level > player->depth + 10){

		mon_deadly[lvl] += addval;

		/* Is it a unique? */
		if (rf_has(mon->race->flags, RF_UNIQUE))
			uniq_deadly[lvl] += addval;
	}

	/* Generate treasure */
	monster_death_stats(i);

	/* remove the monster */
	delete_monster_idx(i);

	/* success */
	return true;
}


/**
 * Print heading infor for the file
 */
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

/**
 * Print all the stats for each level
 */
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

/**
 *Compute and print the mean and standard deviation for an array of known size
 */
static void mean_and_stdv(int array[TRIES_SIZE])
{
	int k, maxiter;
	double tot = 0, mean, stdev, temp = 0;

	/* Get the maximum iteration value */
	maxiter = MIN(tries, TRIES_SIZE); 

	/* Sum the array */
	for (k = 0; k < maxiter; k++)
		tot += array[k];

	/* Compute the mean */
	mean = tot / maxiter;

	/* Sum up the squares */
	for (k = 0; k < maxiter; k++) temp += (array[k] - mean) * (array[k] - mean);

	/* Compute standard dev */
	stdev = sqrt(temp / tries);

	/* Print to file */
	file_putf(stats_log," mean: %f  std-dev: %f \n",mean,stdev);

}

/**
 * Calculated the probability of finding an item by a specific level,
 * and print it to the output file
 */

static void prob_of_find(double stat[MAX_LVL])
{
	static int lvl, tmpcount;
	double find = 0.0, tmpfind = 0.0;

	/* Skip town level */
	for (lvl = 1; lvl < MAX_LVL ; lvl++) {

		/* Calculate the probability of not finding the stat */
		tmpfind=(1 - stat[lvl]);

		/* Maximum probability is 98% */
		if (tmpfind < 0.02) tmpfind = 0.02;

		/* Multiply probabilities of not finding */
		if (find <= 0) find = tmpfind; else find *= tmpfind;

		/* Increase count to 5 */
		tmpcount++;

		/* Print output every 5 levels */
		if (tmpcount == 5) {

			/* print it */
			file_putf(stats_log,"%f \t",1-find);

			/* reset temp counter */
			tmpcount=0;
		}
	}

	/* Put a new line in prep of next entry */
	file_putf(stats_log,"\n"); 
 }

#if 0
/**
 * Left this function unlinked for now
 */
static double total(double stat[MAX_LVL])
{
	int k;
	double out = 0;

	for (k = 0; k < MAX_LVL; k++)
		out += stat[k];

	return out;
} 
#endif

/**
 * Post process select items
 */
static void post_process_stats(void)
{
	double arttot;
	int i,k;

	/* Output a title */
	file_putf(stats_log,"\n");
	file_putf(stats_log,"***** POST PROCESSING *****\n");
	file_putf(stats_log,"\n");
	file_putf(stats_log,"Item \t5\t\t\t10\t\t\t15\t\t\t20\t\t\t25\t\t\t");
	file_putf(stats_log,"30\t\t\t35\t\t\t40\t\t\t45\t\t\t50\t\t\t");
	file_putf(stats_log,"55\t\t\t60\t\t\t65\t\t\t70\t\t\t75\t\t\t");
	file_putf(stats_log,"80\t\t\t85\t\t\t90\t\t\t95\t\t\t100\n");
	
	for (i = 1; i < ST_FF_END; i++) {
			file_putf(stats_log, "%s", stat_ff_message[i].name);
			prob_of_find(stat_all[stat_ff_message[i].st][0]);
			mean_and_stdv(stat_ff_all[i]);
	}

	/* Print artifact total */
	arttot = 0;

	for (k = 0; k < MAX_LVL; k++)
		arttot += art_total[k];

	file_putf(stats_log,"\n");
	file_putf(stats_log,"Total number of artifacts found %f \n",arttot);
	mean_and_stdv(art_it);

	/* Temporary stuff goes here */
	/* Dungeon book totals for Eddie
	file_putf(stats_log,"mb5: %f\n",total(b5_total));
	file_putf(stats_log,"mb6: %f\n",total(b6_total));
	file_putf(stats_log,"mb7: %f\n",total(b7_total));
	file_putf(stats_log,"mb8: %f\n",total(b8_total));
	file_putf(stats_log,"mb9: %f\n",total(b9_total));
	*/
}



/**
 * Scans the dungeon for objects
 */
static void scan_for_objects(void)
{ 
	int y, x;

	for (y = 1; y < cave->height - 1; y++) {
		for (x = 1; x < cave->width - 1; x++) {
			struct loc grid = loc(x, y);
			struct object *obj;

			while ((obj = square_object(cave, grid))) {
				/* Get data on the object */
				get_obj_data(obj, y, x, false, false);

				/* Delete the object */
				square_delete_object(cave, grid, obj, false, false);
			}
		}
	}
}

/**
 * This will scan the dungeon for monsters and then kill each
 * and every last one.
 */
static void scan_for_monsters(void)
{ 
	int i;

	/* Go through the monster list */
	for (i = 1; i < cave_monster_max(cave); i++) {
		struct monster *mon = cave_monster(cave, i);

		/* Skip dead monsters */
		if (!mon->race) continue;

		stats_monster(mon, i);
	}
}

/**
 * This is the entry point for generation statistics.
 */
static void stats_collect_level(void)
{
	/* Make a dungeon */
	prepare_next_level(player);

	/* Scan for objects, these are floor objects */
	scan_for_objects();

	/* Get stats (and kill) all non-unique monsters */
	scan_for_monsters();

}

/**
 * This code will go through the artifact list and make each artifact
 * uncreated so that our sim player can find them again!
 */
static void uncreate_artifacts(void)
{
	int i;

	/* Loop through artifacts */
	for (i = 0; z_info && i < z_info->a_max; i++) {
		mark_artifact_created(&a_info[i], false);
	}
}

/**
 * This will revive all the uniques so the sim player
 * can kill them again.
 */
static void revive_uniques(void)
{
	int i;

	for (i = 1; i < z_info->r_max - 1; i++) {
		/* Get the monster info */
		struct monster_race *race = &r_info[i];

		/* Revive the unique monster */
		if (rf_has(race->flags, RF_UNIQUE)) race->max_num = 1;
	}
}

/**
 * This function loops through the level and does N iterations of
 * the stat calling function, assuming diving style.
 */ 
static void diving_stats(void)
{
	int depth;

	/* Iterate through levels */
	for (depth = 0; depth < MAX_LVL; depth += 5) {
		player->depth = depth;
		if (player->depth == 0) player->depth = 1;

		/* Do many iterations of each level */
		for (iter = 0; iter < tries; iter++)
		     stats_collect_level();

		/* Print the output to the file */
		print_stats(depth);

		/* Show the level to check on status */
		do_cmd_redraw();
	}
}

/**
 * This function loops through the level and does N iterations of
 * the stat calling function, assuming clearing style.
 */ 
static void clearing_stats(void)
{
	int depth;

	/* Do many iterations of the game */
	for (iter = 0; iter < tries; iter++) {
		/* Move all artifacts to uncreated */
		uncreate_artifacts();

		/* Move all uniques to alive */
		revive_uniques();

		/* Do randart regen */
		if ((regen) && (iter<tries)) {
			/* Get seed */
			int seed_randart = randint0(0x10000000);

			/* Restore the standard artifacts */
			cleanup_parser(&randart_parser);
			deactivate_randart_file();
			run_parser(&artifact_parser);

			/* regen randarts */
			do_randart(seed_randart, false);
		}

		/* Do game iterations */
		for (depth = 1 ; depth < MAX_LVL; depth++) {
			/* Debug 
			msg_format("Attempting level %d",depth); */

			/* Move player to that depth */
			player->depth = depth;

			/* Get stats */
			stats_collect_level();

			/* Debug
			msg_format("Finished level %d,depth"); */
		}

		msg("Iteration %d complete",iter);
	}

	/* Restore original artifacts */
	if (regen) {
		cleanup_parser(&randart_parser);
		if (OPT(player, birth_randarts)) {
			activate_randart_file();
			run_parser(&randart_parser);
			deactivate_randart_file();
		} else {
			run_parser(&artifact_parser);
		}
	}

	/* Print to file */
	for (depth = 0 ;depth < MAX_LVL; depth++)
		print_stats(depth);

	/* Post processing */
	post_process_stats();

	/* Display the current level */
	do_cmd_redraw(); 
}

/**
 * Check whether statistic collection is enabled.  Prints a message if it is
 * not.
 *
 * \return true if statistics were enabled at compile time; otherwise, return
 * false.
 */
bool stats_are_enabled(void)
{
	return true;
}

/**
 * Generate levels and collect statistics about the objects and monsters
 * in those levels.
 *
 * \param nsim Is the number of simulations to perform.
 * \param simtype Must be either 1 for a diving simulation, 2 for a clearing
 * simulation, or 3 for a clearing simulation with a regeneration of the
 * random artifacts between each simulation.
 */
void stats_collect(int nsim, int simtype)
{
	bool auto_flag;
	char buf[1024];

	/* Make sure the inputs are good! */
	if (nsim < 1 || simtype < 1 || simtype > 3) return;

	tries = nsim;
	addval = 1.0 / tries;

	/* Are we in diving or clearing mode */
	if (simtype == 1) {
		clearing = false;
	} else {
		clearing = true;
		regen = (simtype == 3);
	}

	/* Open log file */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "stats.log");
	stats_log = file_open(buf, MODE_WRITE, FTYPE_TEXT);

	/* Logging didn't work */
	if (!stats_log) {
		msg("Error - can't open stats.log for writing.");
		exit(1);
	}

	/* Turn on auto-more.  This will clear prompts for items
	 * that drop under the player, or that can't fit on the 
	 * floor due to too many items.  This is a very small amount
	 * of items, even on deeper levels, so it's not worth worrying
	 * too much about.
	 */
	 auto_flag = false;
	 
	 if (!OPT(player, auto_more)) {
		/* Remember that we turned off auto_more */
		auto_flag = true;

		/* Turn on auto-more */
		option_set(option_name(OPT_auto_more),true);
	}

	/* Print heading for the file */
	print_heading();

	/* Make sure all stats are 0 */
	init_stat_vals();

	/* Select diving option */
	if (!clearing) diving_stats();

	/* Select clearing option */
	if (clearing) clearing_stats();

	/* Turn auto-more back off */
	if (auto_flag) option_set(option_name(OPT_auto_more), false);

	/* Close log file */
	if (!file_close(stats_log)) {
		msg("Error - can't close stats.log file.");
		exit(1);
	}
}

#define DIST_MAX 10000

static void calc_cave_distances(int **cave_dist)
{
	int dist;

	/* Squares with distance from player of n - 1 */
	struct loc *ogrids;
	int n_old, cap_old;

	/* Squares with distance from player of n */
	struct loc *ngrids;
	int n_new, cap_new;

	/*
	 * The perimeter of the cave should overestimate the space needed so
	 * there's fewer reallocations within the loop.
	 */
	cap_old = 2 * (cave->width + cave->height - 1);
	ogrids = mem_alloc(cap_old * sizeof(*ogrids));
	cap_new = cap_old;
	ngrids = mem_alloc(cap_new * sizeof(*ngrids));

	/* The player's location is the first one to test. */
	ogrids[0] = player->grid;
	n_old = 1;

	/* Distance from player starts at 0 */
	dist = 0;

	/* Assign the distance value to the first square (player) */
	cave_dist[ogrids[0].y][ogrids[0].x] = dist;

	do {
		int i, n_tmp;
		struct loc *gtmp;

		n_new = 0;
		dist++;

		/* Loop over all visited squares of the previous iteration */
		for (i = 0; i < n_old; i++){
			int d;
			/* Get the square we want to look at */
			int oy = ogrids[i].y;
			int ox = ogrids[i].x;

			/* debug
			msg("x: %d y: %d dist: %d %d ",ox,oy,dist-1,i); */

			/* Get all adjacent squares */
			for (d = 0; d < 8; d++) {
				/* Adjacent square location */
				int ty = oy + ddy_ddd[d];
				int tx = ox + ddx_ddd[d];

				if (!(square_in_bounds_fully(cave, loc(tx, ty)))) continue;

				/* Have we been here before? */
				if (cave_dist[ty][tx] >= 0) continue;

				/*
				 * Impassable terrain which isn't a door or
				 * rubble blocks progress.
				 */
				if (!square_ispassable(cave, loc(tx, ty)) &&
					!square_isdoor(cave, loc(tx, ty)) &&
					!square_isrubble(cave, loc(tx, ty))) continue;

				/* Add the new location */
				if (n_new == cap_new - 1) {
					cap_new *= 2;
					ngrids = mem_realloc(ngrids,
						cap_new * sizeof(ngrids));
				}
				ngrids[n_new].y = ty;
				ngrids[n_new].x = tx;
				++n_new;

				/* Assign the distance to that spot */
				cave_dist[ty][tx] = dist;

				/* debug
				msg("x: %d y: %d dist: %d ",tx,ty,dist); */
			}
		}

		/* Swap the lists; do not need to preserve n_old. */
		gtmp = ogrids;
		ogrids = ngrids;
		ngrids = gtmp;
		n_tmp = cap_old;
		cap_old = cap_new;
		cap_new = n_tmp;
		n_old = n_new;
	} while (n_old > 0 && dist < DIST_MAX);

	mem_free(ngrids);
	mem_free(ogrids);
}

/**
 * Generate several pits and collect statistics about the type of inhabitants.
 *
 * \param nsim Is the number of pits to generate.
 * \param pittype Must be 1 (pit), 2 (nest), or 3 (other).
 * \param depth Is the depth to use for the simulations.
 */
void pit_stats(int nsim, int pittype, int depth)
{
	int *hist;
	int j, p;

	/* Initialize hist */
	hist = mem_zalloc(z_info->pit_max * sizeof(*hist));

	for (j = 0; j < nsim; j++) {
		int i;
		int pit_idx = 0;
		int pit_dist = 999;

		for (i = 0; i < z_info->pit_max; i++) {
			int offset, dist;
			struct pit_profile *pit = &pit_info[i];

			if (!pit->name || pit->room_type != pittype) continue;

			offset = Rand_normal(pit->ave, 10);
			dist = ABS(offset - depth);

			if (dist < pit_dist && one_in_(pit->rarity)) {
				pit_idx = i;
				pit_dist = dist;
			}
		}

		hist[pit_idx]++;
	}

	for (p = 0; p < z_info->pit_max; p++) {
		struct pit_profile *pit = &pit_info[p];
		if (pit->name)
			msg("Type: %s, Number: %d.", pit->name, hist[p]);
	}

	mem_free(hist);

	return;
}

struct tunnel_instance {
	int nstep, npierce, ndug, dstart, dend;
	bool early;
};

struct covar_n {
	/* Is an n element array with the sum of each component. */
	double *s;
	/*
	 * Is a (n * (n + 1)) / 2 element array.  c[(i * (i + 1)) / 2 + j]]
	 * for 0 <= i < n and 0 <= j <= i is the sum of the product of the
	 * ith and jth components.
	 */
	double *c;
	/* Is the number of terms added to the sums. */
	uint32_t count;
	/* Is the number of components. */
	int n;
};

static void initialize_covar(struct covar_n *cv, int n)
{
	int i;

	assert(n >= 1);
	cv->count = 0;
	cv->n = n;
	cv->s = mem_alloc(n * sizeof(*cv->s));
	for (i = 0; i < n; ++i) {
		cv->s[i] = 0.0;
	}
	cv->c = mem_alloc(((n * (n + 1)) / 2) * sizeof(*cv->c));
	for (i = 0; i < (n * (n + 1)) / 2; ++i) {
		cv->c[i] = 0.0;
	}
}

static void cleanup_covar(struct covar_n *cv)
{
	mem_free(cv->c);
	mem_free(cv->s);
}

static void add_to_covar(struct covar_n *cv, ...)
{
	/*
	 * This is temporary space to hold the values so the cross terms can
	 * be computed.
	 */
	double *hs;
	va_list vp;
	int i, ij;

	assert(cv->n >= 1);
	hs = mem_alloc(cv->n * sizeof(*hs));

	/*
	 * Get the component values from the variable arguments.  Add to
	 * to the single component sums.
	 */
	va_start(vp, cv);
	for (i = 0; i < cv->n; ++i) {
		hs[i] = va_arg(vp, double);
		cv->s[i] += hs[i];
	}
	va_end(vp);

	/* Compute the cross terms. */
	for (i = 0, ij = 0; i < cv->n; ++i) {
		int j;

		for (j = 0; j <= i; ++j, ++ij) {
			cv->c[ij] += hs[i] * hs[j];
		}
	}

	++cv->count;

	mem_free(hs);
}

static double compute_covar(const struct covar_n *cv, int i, int j)
{
	double result;

	if (j > i) {
		int t = j;

		j = i;
		i = t;
	}
	assert(i >= 0 && i < cv->n && j >= 0);
	if (cv->count <= 1) return 0.0;
	result = cv->c[(i * (i + 1)) / 2 + j] - cv->s[i] * cv->s[i] / cv->count;
	return (i != j || result > 0.0) ? result / (cv->count - 1) : 0.0;
}

static void dump_covar_averages(const struct covar_n *cv, ang_file* fo)
{
	int i;

	for (i = 0; i < cv->n; ++i) {
		if (i != 0) file_put(fo, "\t");
		file_putf(fo, "%.4f", (cv->count > 0) ?
			cv->s[i] / cv->count : 0.0);
	}
}

static void dump_covar_var(const struct covar_n *cv, ang_file* fo)
{
	int i;

	for (i = 0; i < cv->n; ++i) {
		int j;

		for (j = 0; j <= i; ++j) {
			if (j != 0) file_put(fo, "\t");
			file_putf(fo, "%+.6f", compute_covar(cv, i, j));
		}
		file_put(fo, "\n");
	}
}

/* Assumes the count of terms in the sum is maintained elsewhere. */
struct i_sum_sum2 {
	uint32_t sum, sum2_lo, sum2_hi;
};

static void add_to_i_sum_sum2(struct i_sum_sum2 *s, int v)
{
	uint32_t v2 = v * v;

	s->sum += v;
	if (v2 > 4294967295UL - s->sum2_lo) {
		++s->sum2_hi;
	}
	s->sum2_lo += v2;
}

static double stddev_i_sum_sum2(struct i_sum_sum2 s, int count)
{
	double var;

	if (count <= 1) return 0.0;
	var = s.sum2_hi * 4294967296.0 + s.sum2_lo -
		s.sum * ((double) s.sum / count);
	return (var > 0.0) ? sqrt(var / (count - 1)) : 0.0;
}

/* Assumes the count of terms in the sum is maintained elsewhere. */
struct d_sum_sum2 {
	double sum, sum2;
};

static void initialize_d_sum_sum2(struct d_sum_sum2 *s)
{
	s->sum = 0.0;
	s->sum2 = 0.0;
}

static void add_to_d_sum_sum2(struct d_sum_sum2 *s, double v)
{
	s->sum += v;
	s->sum2 += v * v;
}

static double stddev_d_sum_sum2(struct d_sum_sum2 s, int count)
{
	double var;

	if (count <= 1) return 0.0;
	var = s.sum2 - s.sum * s.sum / count;
	return (var > 0.0) ? sqrt(var / (count - 1)) : 0.0;
}

struct tunnel_aggregate {
	/*
	 * Hold the sums for for the normalized number of steps, number of
	 * wall piercings, normalized number of grids excavated, normalized
	 * starting distance, and normalized final distance.  The first
	 * includes all tunnels, the second only those that had early
	 * terminations, the third only those without early termination, and
	 * the fourth only those that did not reach their destination.
	 */
	struct covar_n cv_all, cv_early, cv_noearly, cv_fail;
	/*
	 * As above but drops the normalized final distance since that is
	 * always zero for tunnels that reach their destinations.
	 */
	struct covar_n cv_success;
	/*
	 * Hold the sums for the fraction of tunnels with early termination
	 * and successful termination.
	 */
	struct d_sum_sum2 early_frac, success_frac;
};

static void initialize_tunnel_aggregate(struct tunnel_aggregate *ta)
{
	initialize_covar(&ta->cv_all, 5);
	initialize_covar(&ta->cv_early, 5);
	initialize_covar(&ta->cv_noearly, 5);
	initialize_covar(&ta->cv_fail, 5);
	initialize_covar(&ta->cv_success, 4);
	initialize_d_sum_sum2(&ta->early_frac);
	initialize_d_sum_sum2(&ta->success_frac);
}

static void cleanup_tunnel_aggregate(struct tunnel_aggregate *ta)
{
	cleanup_covar(&ta->cv_success);
	cleanup_covar(&ta->cv_fail);
	cleanup_covar(&ta->cv_noearly);
	cleanup_covar(&ta->cv_early);
	cleanup_covar(&ta->cv_all);
}

static void add_to_tunnel_aggregate(struct tunnel_aggregate *ta,
		const struct tunnel_instance *ti, int ntunnel,
		const struct chunk *c)
{
	/*
	 * Normalize the number of steps taken, number of grids excavated,
	 * starting distance, and final distance by the sum of the dimensions
	 * of the cave - that has the correct units (grids) and should allow
	 * reasonable aggregation of tunneling results from caves with
	 * different sizes.
	 */
	double length_norm = c->width + c->height;
	int early_count = 0, success_count = 0, i;

	assert(c->width > 0 && c->height > 0);

	for (i = 0; i < ntunnel; ++i) {
		double normed[5] = {
			ti[i].nstep / length_norm,
			ti[i].npierce,
			ti[i].ndug / length_norm,
			ti[i].dstart / length_norm,
			ti[i].dend / length_norm
		};

		add_to_covar(&ta->cv_all, normed[0], normed[1], normed[2],
			normed[3], normed[4]);

		if (ti[i].early) {
			add_to_covar(&ta->cv_early, normed[0], normed[1],
				normed[2], normed[3], normed[4]);
			++early_count;
		} else {
			add_to_covar(&ta->cv_noearly, normed[0], normed[1],
				normed[2], normed[3], normed[4]);
		}

		if (ti[i].dend == 0) {
			add_to_covar(&ta->cv_success, normed[0], normed[1],
				normed[2], normed[3]);
			++success_count;
		} else {
			add_to_covar(&ta->cv_fail, normed[0], normed[1],
				normed[2], normed[3], normed[4]);
		}
	}

	if (ntunnel > 0) {
		add_to_d_sum_sum2(&ta->early_frac,
			early_count / (double) ntunnel);
		add_to_d_sum_sum2(&ta->success_frac,
			success_count / (double) ntunnel);
	}
}

struct grid_count_aggregate {
	/*
	 * For everything but the stairs, accumulate the counts normalized by
	 * the area (in grids) for the cave.  For the stairs, accumulate the
	 * unnormalized counts since the number of stairs is typically
	 * independent of the size of the cave.
	 */
	struct d_sum_sum2 floor;
	struct i_sum_sum2 upstair;
	struct i_sum_sum2 downstair;
	struct d_sum_sum2 trap;
	struct d_sum_sum2 lava;
	struct d_sum_sum2 impass_rubble;
	struct d_sum_sum2 pass_rubble;
	struct d_sum_sum2 magma_treasure;
	struct d_sum_sum2 quartz_treasure;
	struct d_sum_sum2 open_door;
	struct d_sum_sum2 closed_door;
	struct d_sum_sum2 broken_door;
	struct d_sum_sum2 secret_door;
	struct d_sum_sum2 traversable_neighbor_histogram[9];
};

static void initialize_grid_count_aggregate(struct grid_count_aggregate *ga)
{
	int i;

	ga->floor.sum = 0.0;
	ga->floor.sum2 = 0.0;
	ga->upstair.sum = 0;
	ga->upstair.sum2_lo = 0;
	ga->upstair.sum2_hi = 0;
	ga->downstair.sum = 0;
	ga->downstair.sum2_lo = 0;
	ga->downstair.sum2_hi = 0;
	ga->trap.sum = 0.0;
	ga->trap.sum2 = 0.0;
	ga->lava.sum = 0.0;
	ga->lava.sum2 = 0.0;
	ga->impass_rubble.sum = 0.0;
	ga->impass_rubble.sum2 = 0.0;
	ga->pass_rubble.sum = 0.0;
	ga->pass_rubble.sum2 = 0.0;
	ga->magma_treasure.sum = 0.0;
	ga->magma_treasure.sum2 = 0.0;
	ga->quartz_treasure.sum = 0.0;
	ga->quartz_treasure.sum2 = 0.0;
	ga->open_door.sum = 0.0;
	ga->open_door.sum2 = 0.0;
	ga->closed_door.sum = 0.0;
	ga->closed_door.sum2 = 0.0;
	ga->broken_door.sum = 0.0;
	ga->broken_door.sum2 = 0.0;
	ga->secret_door.sum = 0.0;
	ga->secret_door.sum2 = 0.0;
	for (i = 0; i < 9; ++i) {
		ga->traversable_neighbor_histogram[i].sum = 0.0;
		ga->traversable_neighbor_histogram[i].sum2 = 0.0;
	}
}

static void add_to_grid_count_aggregate(struct grid_count_aggregate *ga,
		const struct grid_counts *gi, const struct chunk *c)
{
	double area = c->width * c->height;
	int i;

	assert(c->width > 0 && c->height > 0);
	add_to_d_sum_sum2(&ga->floor, gi->floor / area);
	add_to_i_sum_sum2(&ga->upstair, gi->upstair);
	add_to_i_sum_sum2(&ga->downstair, gi->downstair);
	add_to_d_sum_sum2(&ga->trap, gi->trap / area);
	add_to_d_sum_sum2(&ga->lava, gi->lava / area);
	add_to_d_sum_sum2(&ga->impass_rubble, gi->impass_rubble / area);
	add_to_d_sum_sum2(&ga->pass_rubble, gi->pass_rubble / area);
	add_to_d_sum_sum2(&ga->magma_treasure, gi->magma_treasure / area);
	add_to_d_sum_sum2(&ga->quartz_treasure, gi->quartz_treasure / area);
	add_to_d_sum_sum2(&ga->open_door, gi->open_door / area);
	add_to_d_sum_sum2(&ga->closed_door, gi->closed_door / area);
	add_to_d_sum_sum2(&ga->broken_door, gi->broken_door / area);
	add_to_d_sum_sum2(&ga->secret_door, gi->secret_door / area);
	for (i = 0; i < 9; ++i) {
		add_to_d_sum_sum2(&ga->traversable_neighbor_histogram[i],
			gi->traversable_neighbor_histogram[i] / area);
	}
}

struct cgen_stats {
	/*
	 * This is effectively a 2 x z_info->profile_max array where
	 * level_counts[0][i] element is the number of successful builds of
	 * the ith level type and level_counts[1][i] is the number of
	 * unsuccessful builds of the ith level type.
	 */
	uint32_t *level_counts[2];
	/*
	 * This is a z_info->profile_max element array where total_rooms[i] has
	 * the results for the total number of rooms per successful level in
	 * the ith level type.
	 */
	struct i_sum_sum2* total_rooms;
	/*
	 * This is effectively a z_info->profile_max x 2 x room_type_count array
	 * where room_counts[i][0][j] has the results for the number of
	 * successful rooms of the jth type in the ith level type and
	 * room_counts[i][1][j] has the results for number of unsuccessful
	 * rooms of the jth type in the ith level type.
	 */
	struct i_sum_sum2*** room_counts;
	/*
	 * This is a z_info_profile_max element array of the aggregate results,
	 * by level profile, for tunneling.
	 */
	struct tunnel_aggregate *ta;
	/*
	 * This is a z_info_profile_max x 3 element array of the aggregate
	 * results, by level profile, for grid types.
	 */
	struct grid_count_aggregate **ga;
	/*
	 * This is a 2 x room_type_count array for the room counts of the
	 * current level so they can be reverted upon a level failure.
	 */
	uint32_t *curr_room_counts[2];
	/*
	 * This is a flat array of the tunneling results for the current level.
	 */
	struct tunnel_instance *curr_tunn;
	int n_curr_tunn, alloc_curr_tunn;
	/*
	 * badst_counts[i] is the number of levels of type i where the
	 * player's starting location was invalid (not a staircase if playing
	 * with connected stairs and used a staircase to enter; otherwise,
	 * not passable).
	 */
	uint32_t *badst_counts;
	/*
	 * disarea_counts[i] is the number of levels of type i that had at
	 * least one disconnected area that wasn't in a vault.
	 */
	uint32_t *disarea_counts;
	/*
	 * disdstair_counts[i] is the number of levels of type i where the
	 * player is disconnected from all down staircases.
	 */
	uint32_t *disdstair_counts;
	/* Is the number of successfully generated levels. */
	int nsuccess;
	/* Is the number of failed levels. */
	int nfail;
	/*
	 * Are the type indices for the most recently initiated level and room.
	 */
	int level_type, room_type;
	/*
	 * Is the number of possible room types; caches the result of
	 * get_room_builder_count().
	 */
	int room_type_count;
};

static void cgenstat_handle_new_level(game_event_type et, game_event_data *ed,
		void *ud)
{
	struct cgen_stats *gs;
	int i;

	assert(et == EVENT_GEN_LEVEL_START && ud);
	gs = (struct cgen_stats*) ud;
	gs->level_type = (ed->string) ?
		get_level_profile_index_from_name(ed->string) : -1;
	assert(gs->level_type >= 0 && gs->level_type < z_info->profile_max);

	/* Reset counters for the current level. */
	for (i = 0; i < gs->room_type_count; ++i) {
		gs->curr_room_counts[0][i] = 0;
		gs->curr_room_counts[1][i] = 0;
	}
	gs->n_curr_tunn = 0;
}

static void cgenstat_handle_level_end(game_event_type et, game_event_data *ed,
		void *ud)
{
	struct cgen_stats *gs;

	assert(et == EVENT_GEN_LEVEL_END && ud);
	gs = (struct cgen_stats*) ud;
	assert(gs->level_type >= 0 && gs->level_type < z_info->profile_max);
	if (ed->flag) {
		int room_count = 0;
		struct grid_counts gcounts[3];
		int i;

		/* Successfully created.  Transfer room counts. */
		for (i = 0; i < gs->room_type_count; ++i) {
			add_to_i_sum_sum2(
				&gs->room_counts[gs->level_type][0][i],
				gs->curr_room_counts[0][i]);
			room_count += gs->curr_room_counts[0][i];
			add_to_i_sum_sum2(
				&gs->room_counts[gs->level_type][1][i],
				gs->curr_room_counts[1][i]);
		}
		add_to_i_sum_sum2(&gs->total_rooms[gs->level_type], room_count);

		/* Aggregate tunneling results. */
		add_to_tunnel_aggregate(&gs->ta[gs->level_type],
			gs->curr_tunn, gs->n_curr_tunn, cave);

		/*
		 * Summarize what's in the cave and add it to the running
		 * totals.
		 */
		stat_grid_counter_simple(cave, gcounts);
		for (i = 0; i < 3; ++i) {
			add_to_grid_count_aggregate(&gs->ga[gs->level_type][i],
				&gcounts[i], cave);
		}

		/* Update level success count. */
		++gs->level_counts[0][gs->level_type];
		++gs->nsuccess;
	} else {
		/* Creation failed.  Update level failure count. */
		++gs->level_counts[1][gs->level_type];
		++gs->nfail;
	}
}

static void cgenstat_handle_new_room(game_event_type et, game_event_data *ed,
		void *ud)
{
	struct cgen_stats *gs;

	assert(et == EVENT_GEN_ROOM_START && ud);
	gs = (struct cgen_stats*) ud;
	assert(gs->level_type >= 0 && gs->level_type < z_info->profile_max);
	gs->room_type = (ed->string) ?
		get_room_builder_index_from_name(ed->string) : -1;
	assert(gs->room_type >= 0 && gs->room_type < gs->room_type_count);
}

static void cgenstat_handle_room_end(game_event_type et, game_event_data *ed,
		void *ud)
{
	struct cgen_stats *gs;

	assert(et == EVENT_GEN_ROOM_END && ud);
	gs = (struct cgen_stats*) ud;
	assert(gs->level_type >= 0 && gs->level_type < z_info->profile_max);
	assert(gs->room_type >= 0 && gs->room_type < gs->room_type_count);

	/* Update room count for the current level. */
	++gs->curr_room_counts[(ed->flag) ? 0 : 1][gs->room_type];
}

static void cgenstat_handle_tunnel(game_event_type et, game_event_data *ed,
		void *ud)
{
	struct cgen_stats *gs;

	assert(et == EVENT_GEN_TUNNEL_FINISHED && ud);
	gs = (struct cgen_stats*) ud;
	assert(gs->level_type >= 0 && gs->level_type < z_info->profile_max);

	/* Add to the tunneling records. */
	assert(gs->n_curr_tunn >= 0 && gs->n_curr_tunn <= gs->alloc_curr_tunn);
	if (gs->n_curr_tunn == gs->alloc_curr_tunn) {
		gs->alloc_curr_tunn = (gs->alloc_curr_tunn) ?
			gs->alloc_curr_tunn + gs->alloc_curr_tunn : 8;
		gs->curr_tunn = mem_realloc(gs->curr_tunn,
			gs->alloc_curr_tunn * sizeof(*gs->curr_tunn));
	}
	gs->curr_tunn[gs->n_curr_tunn].nstep = ed->tunnel.nstep;
	gs->curr_tunn[gs->n_curr_tunn].npierce = ed->tunnel.npierce;
	gs->curr_tunn[gs->n_curr_tunn].ndug = ed->tunnel.ndug;
	gs->curr_tunn[gs->n_curr_tunn].dstart = ed->tunnel.dstart;
	gs->curr_tunn[gs->n_curr_tunn].dend = ed->tunnel.dend;
	gs->curr_tunn[gs->n_curr_tunn].early = ed->tunnel.early;
	++gs->n_curr_tunn;
}

static void initialize_generation_stats(struct cgen_stats *gs)
{
	int i;

	gs->nsuccess = 0;
	gs->nfail = 0;
	gs->level_type = -1;
	gs->room_type = -1;
	gs->room_type_count = get_room_builder_count();

	gs->level_counts[0] = mem_zalloc(z_info->profile_max *
		sizeof(*gs->level_counts[0]));
	gs->level_counts[1] = mem_zalloc(z_info->profile_max *
		sizeof(*gs->level_counts[1]));

	gs->total_rooms = mem_zalloc(z_info->profile_max *
		sizeof(*gs->total_rooms));

	gs->room_counts = mem_alloc(z_info->profile_max *
		sizeof(*gs->room_counts));
	for (i = 0; i < z_info->profile_max; ++i) {
		gs->room_counts[i] = mem_alloc(2 * sizeof(*gs->room_counts[i]));
		gs->room_counts[i][0] = mem_zalloc(gs->room_type_count *
			sizeof(*gs->room_counts[i][0]));
		gs->room_counts[i][1] = mem_zalloc(gs->room_type_count *
			sizeof(*gs->room_counts[i][1]));
	}

	gs->ta = mem_alloc(z_info->profile_max * sizeof(*gs->ta));
	for (i = 0; i < z_info->profile_max; ++i) {
		initialize_tunnel_aggregate(&gs->ta[i]);
	}

	gs->ga = mem_alloc(z_info->profile_max * sizeof(*gs->ga));
	for (i = 0; i < z_info->profile_max; ++i) {
		gs->ga[i] = mem_alloc(3 * sizeof(*gs->ga[i]));
		initialize_grid_count_aggregate(&gs->ga[i][0]);
		initialize_grid_count_aggregate(&gs->ga[i][1]);
		initialize_grid_count_aggregate(&gs->ga[i][2]);
	}

	gs->curr_room_counts[0] = mem_alloc(gs->room_type_count *
		sizeof(*gs->curr_room_counts[0]));
	gs->curr_room_counts[1] = mem_alloc(gs->room_type_count *
		sizeof(*gs->curr_room_counts[1]));

	gs->curr_tunn = NULL;
	gs->n_curr_tunn = 0;
	gs->alloc_curr_tunn = 0;

	gs->badst_counts = mem_zalloc(z_info->profile_max *
		sizeof(*gs->badst_counts));
	gs->disarea_counts = mem_zalloc(z_info->profile_max *
		sizeof(*gs->disarea_counts));
	gs->disdstair_counts = mem_zalloc(z_info->profile_max *
		sizeof(*gs->disdstair_counts));

	event_add_handler(EVENT_GEN_LEVEL_START, cgenstat_handle_new_level, gs);
	event_add_handler(EVENT_GEN_LEVEL_END, cgenstat_handle_level_end, gs);
	event_add_handler(EVENT_GEN_ROOM_START, cgenstat_handle_new_room, gs);
	event_add_handler(EVENT_GEN_ROOM_END, cgenstat_handle_room_end, gs);
	event_add_handler(EVENT_GEN_TUNNEL_FINISHED, cgenstat_handle_tunnel, gs);
}

static void cleanup_generation_stats(struct cgen_stats *gs)
{
	int i;

	event_remove_handler(EVENT_GEN_LEVEL_START,
		cgenstat_handle_new_level, gs);
	event_remove_handler(EVENT_GEN_LEVEL_END,
		cgenstat_handle_level_end, gs);
	event_remove_handler(EVENT_GEN_ROOM_START,
		cgenstat_handle_new_room, gs);
	event_remove_handler(EVENT_GEN_ROOM_END,
		cgenstat_handle_room_end, gs);
	event_remove_handler(EVENT_GEN_TUNNEL_FINISHED,
		cgenstat_handle_tunnel, gs);

	mem_free(gs->disdstair_counts);
	mem_free(gs->disarea_counts);
	mem_free(gs->badst_counts);

	mem_free(gs->curr_tunn);

	mem_free(gs->curr_room_counts[1]);
	mem_free(gs->curr_room_counts[0]);

	for (i = 0; i < z_info->profile_max; ++i) {
		mem_free(gs->ga[i]);
	}
	mem_free(gs->ga);

	for (i = 0; i < z_info->profile_max; ++i) {
		cleanup_tunnel_aggregate(&gs->ta[i]);
	}
	mem_free(gs->ta);

	for (i = 0; i < z_info->profile_max; ++i) {
		mem_free(gs->room_counts[i][1]);
		mem_free(gs->room_counts[i][0]);
		mem_free(gs->room_counts[i]);
	}
	mem_free(gs->room_counts);

	mem_free(gs->total_rooms);

	mem_free(gs->level_counts[1]);
	mem_free(gs->level_counts[0]);
}

static void dump_generation_stats(ang_file *fo, const struct cgen_stats *gs)
{
	int i;

	file_put(fo, "Number of Successful Levels::\n");
	file_putf(fo, "%d\n\n", gs->nsuccess);

	file_put(fo, "Level Builder Success Count, Probability, and Failure Rate Per Successful Level::\n");
	for (i = 0; i < z_info->profile_max; ++i) {
		file_putf(fo, "\"%s\"\t%lu\t%.6f\t%.6f\n",
			get_level_profile_name_from_index(i),
			(unsigned long) gs->level_counts[0][i],
			(double) gs->level_counts[0][i] / gs->nsuccess,
			(double) gs->level_counts[1][i] / gs->nsuccess);
	}
	file_put(fo, "\n");

	file_put(fo, "Average and Std. Deviation of Room Counts by Level Type::\n");
	for (i = 0; i < z_info->profile_max; ++i) {
		file_putf(fo, "\"%s\"\t%.4f\t%.4f\n",
			get_level_profile_name_from_index(i),
			(gs->level_counts[0][i] > 0) ?
				(double) gs->total_rooms[i].sum /
				gs->level_counts[0][i] : 0.0,
			stddev_i_sum_sum2(gs->total_rooms[i],
				gs->level_counts[0][i]));
	}
	file_put(fo, "\n");

	for (i = 0; i < z_info->profile_max; ++i) {
		int j;
		const char *name;

		/* Skip profiles that were not used. */
		if (!gs->level_counts[0][i]) continue;

		name = get_level_profile_name_from_index(i);

		file_putf(fo, "\"%s\" Mean and Std. Deviation For Room Counts::\n", name);
		for (j = 0; j < gs->room_type_count; ++j) {
			file_putf(fo, "\"%s\"\t%.4f\t%.4f\n",
				get_room_builder_name_from_index(j),
				(double) gs->room_counts[i][0][j].sum /
					gs->level_counts[0][i],
				stddev_i_sum_sum2(gs->room_counts[i][0][j],
					gs->level_counts[0][i]));
		}
		file_put(fo, "\n");

		file_putf(fo, "\"%s\" Mean and Std. Deviation for Room Failure Rates::\n", name);
		for (j = 0; j < gs->room_type_count; ++j) {
			file_putf(fo, "\"%s\"\t%.6f\t%.6f\n",
				get_room_builder_name_from_index(j),
				(double) gs->room_counts[i][1][j].sum /
					gs->level_counts[0][i],
				stddev_i_sum_sum2(gs->room_counts[i][1][j],
					gs->level_counts[0][i]));
		}
		file_put(fo, "\n");

		file_putf(fo, "\"%s\" Grid Fractions (Vault, Room, Other)::\n", name);
		file_put(fo, "floor");
		for (j = 0; j < 3; ++j) {
			file_putf(fo, "\t%.6f\t%.6f",
				gs->ga[i][j].floor.sum / gs->level_counts[0][i],
				stddev_d_sum_sum2(gs->ga[i][j].floor,
					gs->level_counts[0][i]));
		}
		file_put(fo, "\n");
		file_put(fo, "trap");
		for (j = 0; j < 3; ++j) {
			file_putf(fo, "\t%.6f\t%.6f",
				gs->ga[i][j].trap.sum / gs->level_counts[0][i],
				stddev_d_sum_sum2(gs->ga[i][j].trap,
					gs->level_counts[0][i]));
		}
		file_put(fo, "\n");
		file_put(fo, "lava");
		for (j = 0; j < 3; ++j) {
			file_putf(fo, "\t%.6f\t%.6f",
				gs->ga[i][j].lava.sum / gs->level_counts[0][i],
				stddev_d_sum_sum2(gs->ga[i][j].lava,
					gs->level_counts[0][i]));
		}
		file_put(fo, "\n");
		file_put(fo, "imrubb");
		for (j = 0; j < 3; ++j) {
			file_putf(fo, "\t%.6f\t%.6f",
				gs->ga[i][j].impass_rubble.sum /
					gs->level_counts[0][i],
				stddev_d_sum_sum2(gs->ga[i][j].impass_rubble,
					gs->level_counts[0][i]));
		}
		file_put(fo, "\n");
		file_put(fo, "parubb");
		for (j = 0; j < 3; ++j) {
			file_putf(fo, "\t%.6f\t%.6f",
				gs->ga[i][j].pass_rubble.sum /
					gs->level_counts[0][i],
				stddev_d_sum_sum2(gs->ga[i][j].pass_rubble,
					gs->level_counts[0][i]));
		}
		file_put(fo, "\n");
		file_put(fo, "mgmvein");
		for (j = 0; j < 3; ++j) {
			file_putf(fo, "\t%.6f\t%.6f",
				gs->ga[i][j].magma_treasure.sum /
					gs->level_counts[0][i],
				stddev_d_sum_sum2(gs->ga[i][j].magma_treasure,
					gs->level_counts[0][i]));
		}
		file_put(fo, "\n");
		file_put(fo, "qtzvein");
		for (j = 0; j < 3; ++j) {
			file_putf(fo, "\t%.6f\t%.6f",
				gs->ga[i][j].quartz_treasure.sum /
					gs->level_counts[0][i],
				stddev_d_sum_sum2(gs->ga[i][j].quartz_treasure,
					gs->level_counts[0][i]));
		}
		file_put(fo, "\n");
		file_put(fo, "opdoor");
		for (j = 0; j < 3; ++j) {
			file_putf(fo, "\t%.6f\t%.6f",
				gs->ga[i][j].open_door.sum /
					gs->level_counts[0][i],
				stddev_d_sum_sum2(gs->ga[i][j].open_door,
					gs->level_counts[0][i]));
		}
		file_put(fo, "\n");
		file_put(fo, "cldoor");
		for (j = 0; j < 3; ++j) {
			file_putf(fo, "\t%.6f\t%.6f",
				gs->ga[i][j].closed_door.sum /
					gs->level_counts[0][i],
				stddev_d_sum_sum2(gs->ga[i][j].closed_door,
					gs->level_counts[0][i]));
		}
		file_put(fo, "\n");
		file_put(fo, "brdoor");
		for (j = 0; j < 3; ++j) {
			file_putf(fo, "\t%.6f\t%.6f",
				gs->ga[i][j].broken_door.sum /
					gs->level_counts[0][i],
				stddev_d_sum_sum2(gs->ga[i][j].broken_door,
					gs->level_counts[0][i]));
		}
		file_put(fo, "\n");
		file_put(fo, "scdoor");
		for (j = 0; j < 3; ++j) {
			file_putf(fo, "\t%.6f\t%.6f",
				gs->ga[i][j].secret_door.sum /
					gs->level_counts[0][i],
				stddev_d_sum_sum2(gs->ga[i][j].secret_door,
					gs->level_counts[0][i]));
		}
		file_put(fo, "\n\n");

		file_putf(fo, "\"%s\" Stair Average Counts (Vault, Room, Other)::\n", name);
		file_put(fo, "up");
		for (j = 0; j < 3; ++j) {
			file_putf(fo, "\t%.6f\t%.6f",
				gs->ga[i][j].upstair.sum /
					(double) gs->level_counts[0][i],
				stddev_i_sum_sum2(gs->ga[i][j].upstair,
					gs->level_counts[0][i]));
		}
		file_put(fo, "\n");
		file_put(fo, "down");
		for (j = 0; j < 3; ++j) {
			file_putf(fo, "\t%.6f\t%.6f",
				gs->ga[i][j].downstair.sum /
					(double) gs->level_counts[0][i],
				stddev_i_sum_sum2(gs->ga[i][j].downstair,
					gs->level_counts[0][i]));
		}
		file_put(fo, "\n\n");

		file_putf(fo, "\"%s\" Traversable Neighbor Histogram (Vault, Room, Other)::\n", name);
		for (j = 0; j < 9; ++j) {
			int k;

			file_putf(fo, "%d", j);
			for (k = 0; k < 3; ++k) {
				file_putf(fo, "\t%.6f\t%.6f",
					gs->ga[i][k].traversable_neighbor_histogram[j].sum /
						gs->level_counts[0][i],
					stddev_d_sum_sum2(gs->ga[i][k].traversable_neighbor_histogram[j],
						gs->level_counts[0][i]));
			}
			file_put(fo, "\n");
		}
		file_put(fo, "\n");

		file_putf(fo, "\"%s\" Tunneling Total Number, Success Rate, Early Termination Rate::\n", name);
		file_putf(fo, "%lu\t%.6f\t%.6f\t%.6f\t%.6f\n\n",
			(unsigned long) gs->ta[i].cv_all.count,
			gs->ta[i].success_frac.sum /
			gs->level_counts[0][i], stddev_d_sum_sum2(
			gs->ta[i].success_frac, gs->level_counts[0][i]),
			gs->ta[i].early_frac.sum /
			gs->level_counts[0][i], stddev_d_sum_sum2(
			gs->ta[i].early_frac, gs->level_counts[0][i]));

		file_putf(fo, "\"%s\" Tunneling Scaled Averages (all tunnels; steps, wall piercings, excavated, start distance, final distance)::\n", name);
		dump_covar_averages(&gs->ta[i].cv_all, fo);
		file_put(fo, "\n\n");

		file_putf(fo, "\"%s\" Tunneling Scaled Covariances (all tunnels; steps, wall piercings, excavated, start distance, final distance)::\n", name);
		dump_covar_var(&gs->ta[i].cv_all, fo);
		file_put(fo, "\n");

		file_putf(fo, "\"%s\" Tunneling Scaled Averages (tunnels terminated early; steps, wall piercings, excavated, start distance, final distance)::\n", name);
		dump_covar_averages(&gs->ta[i].cv_early, fo);
		file_put(fo, "\n\n");

		file_putf(fo, "\"%s\" Tunneling Scaled Covariances (tunnels terminated early; steps, wall piercings, excavated, start distance, final distance)::\n", name);
		dump_covar_var(&gs->ta[i].cv_early, fo);
		file_put(fo, "\n");

		file_putf(fo, "\"%s\" Tunneling Scaled Averages (tunnels not terminated early; steps, wall piercings, excavated, start distance, final distance)::\n", name);
		dump_covar_averages(&gs->ta[i].cv_noearly, fo);
		file_put(fo, "\n\n");

		file_putf(fo, "\"%s\" Tunneling Scaled Covariances (tunnels not terminated early; steps, wall piercings, excavated, start distance, final distance)::\n", name);
		dump_covar_var(&gs->ta[i].cv_noearly, fo);
		file_put(fo, "\n");

		file_putf(fo, "\"%s\" Tunneling Scaled Averages (tunnels that reached their destinations; steps, wall piercings, excavated, start distance)::\n", name);
		dump_covar_averages(&gs->ta[i].cv_success, fo);
		file_put(fo, "\n\n");

		file_putf(fo, "\"%s\" Tunneling Scaled Covariances (tunnels that reached their destinations; steps, wall piercings, excavated, start distance)::\n", name);
		dump_covar_var(&gs->ta[i].cv_success, fo);
		file_put(fo, "\n");

		file_putf(fo, "\"%s\" Tunneling Scaled Averages (tunnels that did not reach their destinations; steps, wall piercings, excavated, start distance, final distance)::\n", name);
		dump_covar_averages(&gs->ta[i].cv_fail, fo);
		file_put(fo, "\n\n");

		file_putf(fo, "\"%s\" Tunneling Scaled Covariances (tunnels that did not reach their destinations; steps, wall piercings, excavated, start distance, final distance)::\n", name);
		dump_covar_var(&gs->ta[i].cv_fail, fo);
		file_put(fo, "\n");
	}

	file_put(fo, "Counts of Levels with Invalid Starting Locations::\n");
	for (i = 0; i < z_info->profile_max; ++i) {
		file_putf(fo, "\"%s\"\t%lu\n",
			get_level_profile_name_from_index(i),
			(unsigned long) gs->badst_counts[i]);
	}
	file_put(fo, "\n");

	file_put(fo, "Counts of Levels with Disconnected Non-vault Areas::\n");
	for (i = 0; i < z_info->profile_max; ++i) {
		file_putf(fo, "\"%s\"\t%lu\n",
			get_level_profile_name_from_index(i),
			(unsigned long) gs->disarea_counts[i]);
	}
	file_put(fo, "\n");

	file_put(fo, "Counts of Levels with Player Disconnected from Down Stairs::\n");
	for (i = 0; i < z_info->profile_max; ++i) {
		file_putf(fo, "\"%s\"\t%lu\n",
			get_level_profile_name_from_index(i),
			(unsigned long) gs->disdstair_counts[i]);
	}
}

/**
 * Gather whether the dungeon has disconnects in it and whether the player
 * is disconnected from the stairs
 */
void disconnect_stats(int nsim, bool stop_on_disconnect)
{
	int i, y, x;
	int **cave_dist;
	long bad_starts = 0, dsc_area = 0, dsc_from_stairs = 0;
	char path[1024];
	ang_file *disfile;
	struct cgen_stats gs;
	ang_file *gstfile;

	path_build(path, sizeof(path), ANGBAND_DIR_USER, "disconnect.html");
	disfile = file_open(path, MODE_WRITE, FTYPE_TEXT);
	if (disfile) {
		dump_level_header(disfile, "Disconnected Levels");
	}

	path_build(path, sizeof(path), ANGBAND_DIR_USER,
		"disconnect_gstat.txt");
	gstfile = file_open(path, MODE_WRITE, FTYPE_TEXT);

	/*
	 * Set up to collect some statistics about level types, room types,
	 * and tunneling as well.
	 */
	initialize_generation_stats(&gs);

	for (i = 1; i <= nsim; i++) {
		/* Assume no disconnected areas */
		bool has_dsc = false;
		/* Assume you can't get to the down staircase */
		bool has_dsc_from_stairs = true;
		bool has_bad_start, use_stairs;

		/*
		 * 50% of the time act as if came in via a down staircase;
		 * otherwise come in as if by word of recall/trap door/teleport
		 * level.
		 */
		if (one_in_(2)) {
			player->upkeep->create_up_stair = true;
			player->upkeep->create_down_stair = false;
			use_stairs = OPT(player, birth_connect_stairs);
		} else {
			use_stairs = false;
		}

		/* Make a new cave */
		prepare_next_level(player);

		/* Allocate the distance array */
		cave_dist = mem_zalloc(cave->height * sizeof(int*));
		for (y = 0; y < cave->height; y++)
			cave_dist[y] = mem_zalloc(cave->width * sizeof(int));

		/* Set all cave spots to inaccessible */
		for (y = 0; y < cave->height; y++)
			for (x = 1; x < cave->width; x++)
				cave_dist[y][x] = -1;

		/* Fill the distance array with the correct distances */
		calc_cave_distances(cave_dist);

		/* Cycle through the dungeon */
		for (y = 1; y < cave->height - 1; y++) {
			for (x = 1; x < cave->width - 1; x++) {
				struct loc grid = loc(x, y);

				/*
				 * Don't care about impassable terrain that's
				 * not a closed or secret door or impassable
				 * rubble.
				 */
				if (!square_ispassable(cave, grid) &&
					!square_isdoor(cave, grid) &&
					!square_isrubble(cave, grid)) continue;

				/* Can we get there? */
				if (cave_dist[y][x] >= 0) {

					/* Is it a  down stairs? */
					if (square_isdownstairs(cave, grid)) {

						has_dsc_from_stairs = false;

						/* debug
						msg("dist to stairs: %d",cave_dist[y][x]); */
					}
					continue;
				}

				/* Ignore vaults as they are often disconnected */
				if (square_isvault(cave, grid)) continue;

				/* We have a disconnected area */
				has_dsc = true;
			}
		}

		if ((use_stairs && !square_isupstairs(cave, player->grid))
				|| (!use_stairs
				&& !square_ispassable(cave, player->grid))) {
			has_bad_start = true;
			bad_starts++;
			if (gs.level_type >= 0) {
				++gs.badst_counts[gs.level_type];
			}
		} else {
			has_bad_start = false;
		}

		if (has_dsc_from_stairs) {
			dsc_from_stairs++;
			if (gs.level_type >= 0) {
				++gs.disdstair_counts[gs.level_type];
			}
		}

		if (has_dsc) {
			dsc_area++;
			if (gs.level_type >= 0) {
				++gs.disarea_counts[gs.level_type];
			}
		}

		if (has_bad_start || has_dsc || has_dsc_from_stairs) {
			if (disfile) {
				char label[100] = "Level with";

				if (has_bad_start) {
					(void) my_strcat(label,
						" Bad Player Start",
						sizeof(label));
					if (has_dsc || has_dsc_from_stairs) {
						my_strcat(label,
							(has_dsc && has_dsc_from_stairs) ?
							"," : " and",
							sizeof(label));
					}
				}
				if (has_dsc) {
					(void) my_strcat(label,
						" Disconnected Non-Vault",
						sizeof(label));
					if (has_dsc_from_stairs) {
						my_strcat(label,
							(has_bad_start) ?
							", and" : " and",
							sizeof(label));
					}
				}
				if (has_dsc_from_stairs) {
					(void) my_strcat(label,
						" All Downstairs Inaccessible",
						sizeof(label));
				}
				dump_level_body(disfile, label, cave,
					cave_dist);
			}
			if (stop_on_disconnect) i = nsim;
		}

		/* Free arrays */
		for (y = 0; y < cave->height; y++)
			mem_free(cave_dist[y]);
		mem_free(cave_dist);
	}

	msg("Total levels with bad starts: %ld", bad_starts);
	msg("Total levels with disconnected areas: %ld",dsc_area);
	msg("Total levels isolated from stairs: %ld",dsc_from_stairs);
	if (disfile) {
		dump_level_footer(disfile);
		if (file_close(disfile)) {
			msg("Map is in disconnect.html.");
		}
	}
	if (gstfile) {
		dump_generation_stats(gstfile, &gs);
		if (file_close(gstfile)) {
			msg("Level generation statistics are in disconnect_gstat.txt");
		}
	}

	cleanup_generation_stats(&gs);

	/* Redraw the level */
	do_cmd_redraw();
}


#else /* USE_STATS */

bool stats_are_enabled(void)
{
	msg("Statistics generation not turned on in this build.");
	return false;
}

void stats_collect(int nsim, int simtype)
{
}

void disconnect_stats(int nsim, bool stop_on_disconnect)
{
}

void pit_stats(int nsim, int pittype, int depth)
{
}
#endif /* USE_STATS */

/**
 * Visit all grids in a chunk and report requested counts.
 * \param c Is the chunk to use.
 * \param gpreds Is a n_gpred element array.  For each element in the array,
 * the in_vault_count, in_room_count, and in_other_count fields are set to
 * zero at the start of this function.  Then for each grid in the chunk where
 * the pred field evaluates to be true, the following is done: if
 * square_isvault() is also true for that grid, in_vault_count is incremented;
 * if square_isvault() is not true but square_isroom() is true for that grid,
 * in_room_count is incremented; if both square_isvault() and square_isroom()
 * ar false for that grid, in_other_count is incremented.
 * \param n_gpred Is the number of elements in gpreds.
 * \param npreds Is a n_npred element array.  For each element in the array,
 * all elmeents of vault_histogram, room_histogram, and other_histogram are
 * set to zero as the start of this function.  Then for each grid in the chunk
 * where the pred field evaluates to be true, count the number of immediate
 * neighbors where the neigh field evaluates to be true.  If square_isvault()
 * is true for the grid, increment the vault_histogram element corresponding to
 * that count; if square_isvault() is not true but square_isroom() is true for
 * the grid, increment the room_histogram element correspoding to that count;
 * if both square_isvault() and square_isroom() are false for the grid,
 * increment the other_histogram element corresponding to that count.
 * \param n_npred Is the number of elements in npreds.
 */
void stat_grid_counter(struct chunk *c, struct grid_counter_pred *gpreds,
		int n_gpred, struct neighbor_counter_pred *npreds, int n_npred)
{
	int i;
	struct loc grid;

	/* Initialize counts. */
	for (i = 0; i < n_gpred; ++i) {
		gpreds[i].in_vault_count = 0;
		gpreds[i].in_room_count = 0;
		gpreds[i].in_other_count = 0;
	}
	for (i = 0; i < n_npred; ++i) {
		int j;

		for (j = 0; j < 9; ++j) {
			npreds[i].vault_histogram[j] = 0;
			npreds[i].room_histogram[j] = 0;
			npreds[i].other_histogram[j] = 0;
		}
	}

	/* Visit every grid. */
	for (grid.y = 0; grid.y < c->height; ++grid.y) {
		for (grid.x = 0; grid.x < c->width; ++grid.x) {
			if (square_isvault(c, grid)) {
				for (i = 0; i < n_gpred; ++i) {
					if ((*gpreds[i].pred)(c, grid)) {
						++gpreds[i].in_vault_count;
					}
				}
				for (i = 0; i < n_npred; ++i) {
					if ((npreds[i].pred)(c, grid)) {
						int count = count_neighbors(
							NULL, c, grid,
							npreds[i].neigh, false);

						assert(count >= 0 &&
							count <= 8);
						++npreds[i].vault_histogram[count];
					}
				}
			} else if (square_isroom(c, grid)) {
				for (i = 0; i < n_gpred; ++i) {
					if ((*gpreds[i].pred)(c, grid)) {
						++gpreds[i].in_room_count;
					}
				}
				for (i = 0; i < n_npred; ++i) {
					if ((npreds[i].pred)(c, grid)) {
						int count = count_neighbors(
							NULL, c, grid,
							npreds[i].neigh, false);

						assert(count >= 0 &&
							count <= 8);
						++npreds[i].room_histogram[count];
					}
				}
			} else {
				for (i = 0; i < n_gpred; ++i) {
					if ((*gpreds[i].pred)(c, grid)) {
						++gpreds[i].in_other_count;
					}
				}
				for (i = 0; i < n_npred; ++i) {
					if ((npreds[i].pred)(c, grid)) {
						int count = count_neighbors(
							NULL, c, grid,
							npreds[i].neigh, false);

						assert(count >= 0 &&
							count <= 8);
						++npreds[i].other_histogram[count];
					}
				}
			}
		}
	}
}

static bool is_easily_traversed(struct chunk *c, struct loc grid)
{
	return square_ispassable(c, grid) || square_isdoor(c, grid) ||
		square_isrubble(c, grid);
}

static bool is_impassable_rubble(struct chunk *c, struct loc grid)
{
	return !square_ispassable(c, grid) && square_isrubble(c, grid);
}

static bool is_passable_rubble(struct chunk *c, struct loc grid)
{
	return square_ispassable(c, grid) && square_isrubble(c, grid);
}

static bool is_magma_treasure(struct chunk *c, struct loc grid)
{
	return square_ismagma(c, grid) && square_hasgoldvein(c, grid);
}

static bool is_quartz_treasure(struct chunk *c, struct loc grid)
{
	return square_isquartz(c, grid) && square_hasgoldvein(c, grid);
}

/**
 * Use stat_grid_counter() to get the grid counts and immediate neighborhood
 * characteristics most likely to be useful for assessing map quality and
 * balance.
 * \param c Is the chunk to use.
 * \param counts Is a three element array of the count structures.  The first
 * element will hold the count of the features in vaults.  The second element
 * will hold the count of the features in rooms that are not also vaults.  The
 * third element will hold the count of features that are neither in vaults nor
 * rooms.  For all three, the traversable_neighbor_histogram field has the
 * histogram of the number of immediate neighbors that are fairly easily
 * traversed by the player (square_ispassable(), square_isdoor(), or
 * square_isrubble()) for all easily traversable grids in the respective
 * category (vault, room, other).  For the other category, that's a measure of
 * how often corridors bend, intersect, or bunch up into wide corridors.
 */
void stat_grid_counter_simple(struct chunk *c, struct grid_counts counts[3])
{
	struct grid_counter_pred gpreds[] = {
		{ square_isfloor, 0, 0, 0 },
		{ square_isupstairs, 0, 0, 0 },
		{ square_isdownstairs, 0, 0, 0 },
		{ square_istrap, 0, 0, 0 },
		{ square_isfiery, 0, 0, 0 },
		{ is_impassable_rubble, 0, 0, 0 },
		{ is_passable_rubble, 0, 0, 0 },
		{ is_magma_treasure, 0, 0, 0 },
		{ is_quartz_treasure, 0, 0, 0 },
		{ square_isopendoor, 0, 0, 0 },
		{ square_iscloseddoor, 0, 0, 0 },
		{ square_isbrokendoor, 0, 0, 0 },
		{ square_issecretdoor, 0, 0, 0 },
	};
	struct neighbor_counter_pred npreds[] = {
		{ is_easily_traversed, is_easily_traversed,
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
	};
	int i;

	stat_grid_counter(c, gpreds, (int) N_ELEMENTS(gpreds),
		npreds, (int) N_ELEMENTS(npreds));
	counts[0].floor = gpreds[0].in_vault_count;
	counts[0].upstair = gpreds[1].in_vault_count;
	counts[0].downstair = gpreds[2].in_vault_count;
	counts[0].trap = gpreds[3].in_vault_count;
	counts[0].lava = gpreds[4].in_vault_count;
	counts[0].impass_rubble = gpreds[5].in_vault_count;
	counts[0].pass_rubble = gpreds[6].in_vault_count;
	counts[0].magma_treasure = gpreds[7].in_vault_count;
	counts[0].quartz_treasure = gpreds[8].in_vault_count;
	counts[0].open_door = gpreds[9].in_vault_count;
	counts[0].closed_door = gpreds[10].in_vault_count;
	counts[0].broken_door = gpreds[11].in_vault_count;
	counts[0].secret_door = gpreds[12].in_vault_count;
	for (i = 0; i < 9; ++i) {
		counts[0].traversable_neighbor_histogram[i] =
			npreds[0].vault_histogram[i];
	}
	counts[1].floor = gpreds[0].in_room_count;
	counts[1].upstair = gpreds[1].in_room_count;
	counts[1].downstair = gpreds[2].in_room_count;
	counts[1].trap = gpreds[3].in_room_count;
	counts[1].lava = gpreds[4].in_room_count;
	counts[1].impass_rubble = gpreds[5].in_room_count;
	counts[1].pass_rubble = gpreds[6].in_room_count;
	counts[1].magma_treasure = gpreds[7].in_room_count;
	counts[1].quartz_treasure = gpreds[8].in_room_count;
	counts[1].open_door = gpreds[9].in_room_count;
	counts[1].closed_door = gpreds[10].in_room_count;
	counts[1].broken_door = gpreds[11].in_room_count;
	counts[1].secret_door = gpreds[12].in_room_count;
	for (i = 0; i < 9; ++i) {
		counts[1].traversable_neighbor_histogram[i] =
			npreds[0].room_histogram[i];
	}
	counts[2].floor = gpreds[0].in_other_count;
	counts[2].upstair = gpreds[1].in_other_count;
	counts[2].downstair = gpreds[2].in_other_count;
	counts[2].trap = gpreds[3].in_other_count;
	counts[2].lava = gpreds[4].in_other_count;
	counts[2].impass_rubble = gpreds[5].in_other_count;
	counts[2].pass_rubble = gpreds[6].in_other_count;
	counts[2].magma_treasure = gpreds[7].in_other_count;
	counts[2].quartz_treasure = gpreds[8].in_other_count;
	counts[2].open_door = gpreds[9].in_other_count;
	counts[2].closed_door = gpreds[10].in_other_count;
	counts[2].broken_door = gpreds[11].in_other_count;
	counts[2].secret_door = gpreds[12].in_other_count;
	for (i = 0; i < 9; ++i) {
		counts[2].traversable_neighbor_histogram[i] =
			npreds[0].other_histogram[i];
	}
}
