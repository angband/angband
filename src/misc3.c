/* File: misc3.c */ 

/* Purpose: misc code for objects */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "angband.h"




/*
 * Pushs a record back onto free space list
 *
 * Delete_object() should always be called instead.
 *
 * Note that files.c is NOT calling us right now.
 *
 * Items always include a "location" in the cave, if on the floor.
 *
 * This routine only applies to items on the floor.
 * Inven/Equip/Store items are not processed by this routine.
 */
void delete_object_idx(int i)
{
    int16        i_idx = (int16)(i);

    /* One less item */
    i_max--;

    /* Last object deletion, simply wipe */
    if (i_idx == i_max) {

	/* Be cautious, wipe the "dead" object */
	invcopy(&i_list[i_max], OBJ_NOTHING);
    }

    /* When "pushing" an non-final object, we compact */
    else {

	register int i, j;

	/* Compact, via structure copy */
	i_list[i_idx] = i_list[i_max];

	/* Be cautious, wipe the "dead" object */
	invcopy(&i_list[i_max], OBJ_NOTHING);

	/* Extract the "location" of the "moved object" */
	i = i_list[i_idx].iy;
	j = i_list[i_idx].ix;

	/* Hack -- Use iy,ix to look for the old "last items" */
	if (in_bounds(i, j)) {
	    if (cave[i][j].i_idx == i_max) {
		cave[i][j].i_idx = i_idx;
		return;
	    }
	}

	/* Hack -- allow iy,ix to be incorrect */
	for (i = 0; i < cur_height; i++) {
	    for (j = 0; j < cur_width; j++) {
		if (cave[i][j].i_idx == i_max) {
		    cave[i][j].i_idx = i_idx;
		    return;
		}
	    }
	}
    }
}


/*
 * Deletes object from given location			-RAK-	
 */
void delete_object(int y, int x)
{
    register cave_type *c_ptr;

    /* Refuse "illegal" locations */
    if (!in_bounds(y, x)) return;

    /* Find where it was */
    c_ptr = &cave[y][x];

    /* Nothing here?  Don't delete it */
    if (c_ptr->i_idx == 0) return;

    /* Kill the object */
    delete_object_idx(c_ptr->i_idx);

    /* There is nothing here */
    c_ptr->i_idx = 0;

    /* Turn off the field mark */
    c_ptr->info &= ~CAVE_FM;

    /* Redraw the spot (may reset field mark) */
    lite_spot(y, x);
}



/*
 * Erase "i_list" (done when creating a new level)
 */
void wipe_i_list()
{
    register int i;

    /* Wipe the object list */
    for (i = 0; i < MAX_I_IDX; i++) {

	/* Blank the object */
	invcopy(&i_list[i], OBJ_NOTHING);
    }

    /* No "real" items */
    i_max = MIN_I_IDX;
}



/*
 * If too many objects on floor level, delete some of them
 * Note that technically, the player could intentionally
 * collect so many artifacts, and create so many stairs,
 * that we become unable to compact.  Then we are screwed.
 */
static void compact_objects()
{
    register int        i, j;
    register cave_type *c_ptr;
    register inven_type *i_ptr;
    int                 cnt, num;
    int			cur_lev, cur_dis, chance;


    msg_print("Compacting objects...");

    /* Go for at most 20 rounds, or until something gets compacted */
    for (num = 0, cnt = 1; cnt <= 20 && num <= 0; cnt++) {

	/* Get more vicious each iteration */
	cur_lev = 5 * cnt;

	/* Get closer each iteration */
	cur_dis = 5 * (20 - cnt);

	/* Nothing to compact (!!!) */
	if (cur_dis < 0) return;

	/* Examine the dungeon */
	for (i = 0; i < cur_height; i++) {
	    for (j = 0; j < cur_width; j++) {

		/* Do not consider artifacts or stairs */
		if (!valid_grid(i,j)) continue;

		/* Get the location */
		c_ptr = &cave[i][j];

		/* Do not even consider empty grids */
		if (c_ptr->i_idx == 0) continue;

		/* Get the object */
		i_ptr = &i_list[c_ptr->i_idx];

		/* Hack -- High level objects start out "immune" */
		if (k_list[i_ptr->k_idx].level > cur_lev) continue;

		/* Nearby objects start out "immune" */
		if (distance(i, j, char_row, char_col) < cur_dis) continue;

		/* Every object gets a "saving throw" */
		switch (i_ptr->tval) {
		    case TV_VIS_TRAP:
			chance = 15;
			break;
		    case TV_RUBBLE:
		    case TV_INVIS_TRAP:
		    case TV_OPEN_DOOR:
		    case TV_CLOSED_DOOR:
			chance = 5;
			break;
		    case TV_SECRET_DOOR:
			chance = 3;
			break;
		    default:
			chance = 10;
		}

		/* Apply the saving throw */
		if (randint(100) > chance) continue;

		/* Delete it */
		delete_object(i, j);

		/* Count it */
		num++;
	    }
	}
    }

    /* Redraw */
    prt_map();

    /* Update the view, etc */
    update_view();
    update_lite();
    update_monsters();
}

/*
 * Gives pointer to next free space			-RAK-	 
 */
int i_pop(void)
{
    /* Compact if needed */
    if (i_max == MAX_I_IDX) compact_objects();

    /* XXX XXX Hack -- Still screwed? */
    if (i_max == MAX_I_IDX) abort();

    /* Return the next free space */
    return (i_max++);
}



/*
 * Boolean : is object enchanted	  -RAK- 
 */
int magik(int chance)
{
    if (randint(100) <= chance) return (TRUE);

    return (FALSE);
}


/*
 * Enchant a bonus based on degree desired -RAK-
 *
 * Lets just change this to make sense.  Now it goes from base to limit,
 * roughly proportional to the level.... -CWS
 *
 * Actually, apparently the value can exceed the "limit" if the "range"
 * is large (thus an amulet of the Magi (+40) *is* possible).
 *
 * I have no idea how (much less "if") this function works...
 *
 * XXX XXX XXX XXX XXX XXX Somebody needs to verify this! XXX XXX
 */
int m_bonus(int base, int limit, int level)
{
    register int x, stand_dev, tmp, diff = limit - base;

    /* Hack -- enforce legal "range" */
    if (base >= limit) return (base);
    
#ifdef USE_FLOATING_POINT

    /* standard deviation twice as wide at bottom of Angband as top */
    stand_dev = (OBJ_STD_ADJ * (1 + level / 100.0)) + OBJ_STD_MIN;

    /* check for level > max_std to check for overflow... */
    if (stand_dev > 40) stand_dev = 40;

    /* Call an odd function */
    tmp = randnor(0, stand_dev);

    /* Extract a weird value */
    x = (tmp * diff / 150.0) + (level * limit / 200.0) + base;

#else

    /* XXX XXX Hack -- this may not be what was desired */
    stand_dev = (OBJ_STD_ADJ * level / 100) + OBJ_STD_MIN;

    /* check for level > max_std to check for overflow... */
    if (stand_dev > 40) stand_dev = 40;

    /* Call an odd function */
    tmp = randnor(0, stand_dev);

    /* Extract a weird value */
    x = (tmp * diff / 150) + (level * limit / 200) + base;

#endif

    /* Enforce minimum value */
    if (x < base) return (base);

    /* Hack -- allow values *slightly* higher than the limit */
    if (x > limit) x = rand_range(limit, limit + diff / 5);

    /* Return the extracted value */
    return (x);
}


/*
 * Make an artifact bow  XXX Fix this Bow Code XXX
 */
static int make_artifact_bow(inven_type *i_ptr)
{
    if (i_ptr->sval == SV_LONG_BOW) {

	if (randint(2) == 1) {
	    if (v_list[ART_BELTHRONDING].cur_num) return (0);
	    /* XXX Used to be "(x5)", now is "(x4)" */
	    /* XXX Consider adding TR3_XTRA_SHOTS */
	    i_ptr->flags1 |= (TR1_STEALTH | TR1_DEX);
	    i_ptr->flags2 |= (TR2_RES_DISEN);
	    i_ptr->flags3 |= (TR3_HIDE_TYPE | TR3_XTRA_MIGHT);
	    i_ptr->pval = 3;	/* +3 DEX, Stealth */
	    i_ptr->tohit = 20;
	    i_ptr->todam = 22;
	    i_ptr->cost = 35000L;
	    return (ART_BELTHRONDING);
	}
	else {
	    if (v_list[ART_BARD].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_DEX);
	    i_ptr->flags2 |= (TR2_FREE_ACT);
	    i_ptr->flags3 |= (TR3_XTRA_MIGHT);
	    i_ptr->pval = 3;	/* +3 DEX */
	    i_ptr->tohit = 17;
	    i_ptr->todam = 19;
	    i_ptr->cost = 20000L;
	    return (ART_BARD);
	}
    }

    if (i_ptr->sval == SV_LIGHT_XBOW) {
	if (v_list[ART_CUBRAGOL].cur_num) return (0);
	if (randint(2) == 1) return (0);
	i_ptr->flags1 |= (TR1_SPEED);
	i_ptr->flags2 |= (TR2_RES_FIRE);
	i_ptr->flags3 |= (TR3_ACTIVATE | TR3_XTRA_MIGHT);
	i_ptr->pval = 1;	/* +1 to speed */
	i_ptr->tohit = 10;
	i_ptr->todam = 14;
	i_ptr->cost = 38000L;
	return (ART_CUBRAGOL);
    }

    return (0);
}




/*
 * Help generate a unique weapon, return its name
 */
static int make_artifact_hafted(inven_type *i_ptr)
{
    /* Hack -- make sure to make "Grond" as soon as possible */
    if (i_ptr->sval == SV_LEAD_FILLED_MACE) {
	if (v_list[ART_GROND].cur_num) return (0);
	if (!permit_grond) return (0);
	i_ptr->flags1 |= (TR1_SLAY_DEMON | TR1_SLAY_TROLL | TR1_SLAY_ORC |
			  TR1_SLAY_EVIL |
			  TR1_SLAY_UNDEAD |
			  TR1_IMPACT | 
			  TR1_SLAY_ANIMAL |
			  TR1_SPEED | TR1_KILL_DRAGON);
	i_ptr->flags2 |= (TR2_RES_FIRE | TR2_RES_COLD |
			  TR2_RES_ELEC | TR2_RES_ACID);
	i_ptr->flags3 |= (TR3_SEE_INVIS | TR3_TELEPATHY | TR3_AGGRAVATE);
	i_ptr->pval = (-1); /* -1 to speed */
	i_ptr->tohit = 5;
	i_ptr->todam = 25;
	i_ptr->toac = 10;
	i_ptr->damage[0] = 10;
	i_ptr->damage[1] = 8;
	i_ptr->weight = 600;
	i_ptr->cost = 500000L;
	return (ART_GROND);
    }


    if (i_ptr->sval == SV_FLAIL) {
	if (v_list[ART_TOTILA].cur_num) return (0);
	i_ptr->flags1 |= (TR1_STEALTH | TR1_BRAND_FIRE |
			  TR1_SLAY_EVIL);
	i_ptr->flags2 |= (TR2_RES_CONF | TR2_RES_FIRE);
	i_ptr->flags3 |= (TR3_ACTIVATE);
	i_ptr->pval = 2;
	i_ptr->tohit = 6;
	i_ptr->todam = 8;
	i_ptr->damage[1] = 9;
	i_ptr->cost = 55000L;
	return (ART_TOTILA);
    }

    if (i_ptr->sval == SV_TWO_HANDED_FLAIL) {
	if (v_list[ART_THUNDERFIST].cur_num) return (0);
	if (randint(5) > 1) return 0;
	i_ptr->flags1 |= (TR1_SLAY_TROLL | TR1_SLAY_ORC |
			  TR1_SLAY_ANIMAL | TR1_STR | TR1_BRAND_FIRE |
			  TR1_BRAND_ELEC);
	i_ptr->flags2 |= (TR2_RES_DARK | TR2_RES_FIRE | TR2_RES_ELEC);
	i_ptr->pval = 4;
	i_ptr->tohit = 5;
	i_ptr->todam = 18;
	i_ptr->cost = 160000L;
	return (ART_THUNDERFIST);
    }

    if (i_ptr->sval == SV_MORNING_STAR) {
	switch (randint(2)) {

	  case 1:
	    if (v_list[ART_BLOODSPIKE].cur_num) return (0);
	    if (randint(2) > 1) return 0;
	    i_ptr->flags1 |= (TR1_SLAY_TROLL | TR1_SLAY_ORC | 
			      TR1_SLAY_ANIMAL | TR1_STR);
	    i_ptr->flags2 |= (TR2_RES_NEXUS);
	    i_ptr->flags3 |= (TR3_SEE_INVIS);
	    i_ptr->pval = 4;
	    i_ptr->tohit = 8;
	    i_ptr->todam = 22;
	    i_ptr->cost = 30000L;
	    return (ART_BLOODSPIKE);

	  default: /* case 2: */
	    if (v_list[ART_FIRESTAR].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_BRAND_FIRE);
	    i_ptr->flags2 |= (TR2_RES_FIRE);
	    i_ptr->flags3 |= (TR3_ACTIVATE);
	    i_ptr->tohit = 5;
	    i_ptr->todam = 7;
	    i_ptr->toac = 2;
	    i_ptr->cost = 35000L;
	    return (ART_FIRESTAR);
	}
    }

    if (i_ptr->sval == SV_MACE) {
	if (v_list[ART_TARATOL].cur_num) return (0);
	if (randint(2) > 1) return 0;
	i_ptr->flags1 |= (TR1_KILL_DRAGON | TR1_BRAND_ELEC);
	i_ptr->flags2 |= (TR2_RES_DARK | TR2_RES_ELEC);
	i_ptr->flags3 |= (TR3_ACTIVATE);
	i_ptr->tohit = 12;
	i_ptr->todam = 12;
	i_ptr->weight = 200;
	i_ptr->damage[1] = 7;
	i_ptr->cost = 20000L;
	return (ART_TARATOL);
    }

    if (i_ptr->sval == SV_WAR_HAMMER) {
	if (v_list[ART_AULE].cur_num) return (0);
	if (randint(10) > 1) return 0;
	i_ptr->flags1 |= (TR1_SLAY_DEMON | TR1_KILL_DRAGON | TR1_SLAY_EVIL |
			  TR1_SLAY_UNDEAD | TR1_WIS | TR1_BRAND_ELEC);
	i_ptr->flags2 |= (TR2_RES_NEXUS | TR2_FREE_ACT | 
			  TR2_RES_FIRE | TR2_RES_ACID | TR2_RES_COLD | TR2_RES_ELEC);
	i_ptr->flags3 |= (TR3_SEE_INVIS);
	i_ptr->pval = 4;
	i_ptr->tohit = 19;
	i_ptr->todam = 21;
	i_ptr->toac = 5;
	i_ptr->damage[0] = 5;
	i_ptr->damage[1] = 5;
	i_ptr->cost = 250000L;
	return (ART_AULE);
    }

    if (i_ptr->sval == SV_QUARTERSTAFF) {
	switch (randint(7)) {

	  case 1: case 2: case 3:
	    if (v_list[ART_NAR].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_INT | TR1_SLAY_ANIMAL |
			    TR1_BRAND_FIRE);
	    i_ptr->flags2 |= (TR2_RES_FIRE);
	    i_ptr->pval = 3;
	    i_ptr->tohit = 10;
	    i_ptr->todam = 20;
	    i_ptr->cost = 70000L;
	    return (ART_NAR);

	  case 4: case 5: case 6:
	    if (v_list[ART_ERIRIL].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_SLAY_EVIL | TR1_INT | TR1_WIS);
	    i_ptr->flags2 |= (TR2_RES_LITE);
	    i_ptr->flags3 |= (TR3_LITE | TR3_SEE_INVIS | TR3_ACTIVATE);
	    i_ptr->pval = 4;
	    i_ptr->tohit = 3;
	    i_ptr->todam = 5;
	    i_ptr->cost = 20000L;
	    return (ART_ERIRIL);

	  default: /* case 7: */
	    if (v_list[ART_OLORIN].cur_num) return (0);
	    if (randint(2) > 1) return 0;
	    i_ptr->flags1 |= (TR1_SLAY_ORC | TR1_SLAY_TROLL |
			      TR1_SLAY_EVIL |
			      TR1_WIS | TR1_INT | TR1_CHR |
			      TR1_BRAND_FIRE);
	    i_ptr->flags2 |= (TR2_HOLD_LIFE | TR2_RES_NETHER | TR2_RES_FIRE);
	    i_ptr->flags3 |= (TR3_ACTIVATE | TR3_SEE_INVIS);
	    i_ptr->pval = 4;
	    i_ptr->tohit = 10;
	    i_ptr->todam = 13;
	    i_ptr->damage[0] = 2;
	    i_ptr->damage[1] = 10;
	    i_ptr->cost = 130000L;
	    return (ART_OLORIN);
	}
    }

    if (i_ptr->sval == SV_MACE_OF_DISRUPTION) {
	if (v_list[ART_DEATHWREAKER].cur_num) return (0);
	if (randint(5) > 1) return 0;
	i_ptr->flags1 |= (TR1_STR | TR1_BRAND_FIRE |
			  TR1_SLAY_EVIL | TR1_SLAY_DRAGON |
			  TR1_SLAY_ANIMAL | TR1_TUNNEL);
	i_ptr->flags2 |= (TR2_IM_FIRE | TR2_RES_FIRE | TR2_RES_CHAOS |
			  TR2_RES_DISEN | TR2_RES_DARK);
	i_ptr->flags3 |= (TR3_AGGRAVATE);
	i_ptr->pval = 6;	/* +6 STR/Tunnel */
	i_ptr->tohit = 18;
	i_ptr->todam = 18;
	i_ptr->damage[1] = 12;
	i_ptr->cost = 400000L;
	return (ART_DEATHWREAKER);
    }

    if (i_ptr->sval == SV_LUCERN_HAMMER) {
	if (v_list[ART_TURMIL].cur_num) return (0);
	if (randint(2) > 1) return 0;
	i_ptr->flags1 |= (TR1_SLAY_ORC | TR1_WIS | TR1_BRAND_COLD |
			  TR1_INFRA);
	i_ptr->flags2 |= (TR2_RES_COLD | TR2_RES_LITE);
	i_ptr->flags3 |= (TR3_LITE | TR3_HIDE_TYPE | TR3_ACTIVATE | TR3_REGEN);
	i_ptr->pval = 4;	/* +4 WIS/Infra */
	i_ptr->tohit = 10;
	i_ptr->todam = 6;
	i_ptr->toac = 8;
	i_ptr->cost = 30000L;
	return (ART_TURMIL);
    }


    return (0);
}




/*
 * Help generate a unique weapon, return its name
 */
static int make_artifact_polearm(inven_type *i_ptr)
{
    if (i_ptr->sval == SV_BEAKED_AXE) {
	if (v_list[ART_THEODEN].cur_num) return (0);
	if (randint(2) > 1) return 0;
	i_ptr->flags1 |= (TR1_WIS | TR1_CON |
			  TR1_SEARCH | TR1_SLAY_DRAGON);
	i_ptr->flags3 |= (TR3_TELEPATHY | TR3_SLOW_DIGEST | TR3_HIDE_TYPE | TR3_ACTIVATE);
	i_ptr->pval = 3;
	i_ptr->tohit = 8;
	i_ptr->todam = 10;
	i_ptr->cost = 40000L;
	return (ART_THEODEN);
    }

    if (i_ptr->sval == SV_GLAIVE) {
	if (v_list[ART_PAIN].cur_num) return (0);
	if (randint(3) > 1) return 0;
	i_ptr->tohit = 0;
	i_ptr->todam = 30;
	i_ptr->damage[0] = 10;
	i_ptr->damage[1] = 6;
	i_ptr->cost = 50000L;
	return (ART_PAIN);
    }

    if (i_ptr->sval == SV_HALBERD) {
	if (v_list[ART_OSONDIR].cur_num) return (0);
	i_ptr->flags1 |= (TR1_BRAND_FIRE | TR1_SLAY_UNDEAD |
			  TR1_SLAY_GIANT | TR1_CHR);
	i_ptr->flags2 |= (TR2_RES_SOUND | TR2_RES_FIRE);
	i_ptr->flags3 |= (TR3_SEE_INVIS | TR3_FEATHER);
	i_ptr->pval = 3;
	i_ptr->tohit = 6;
	i_ptr->todam = 9;
	i_ptr->cost = 22000L;
	return (ART_OSONDIR);
    }

    if (i_ptr->sval == SV_PIKE) {
	if (v_list[ART_TIL].cur_num) return (0);
	if (randint(2) > 1) return 0;
	i_ptr->flags1 |= (TR1_SLAY_DEMON | TR1_SLAY_GIANT | TR1_SLAY_TROLL |
			  TR1_BRAND_COLD | TR1_BRAND_FIRE | 
			  TR1_INT);
	i_ptr->flags2 |= (TR2_RES_FIRE | TR2_RES_COLD | TR2_SUST_INT);
	i_ptr->flags3 |= (TR3_SLOW_DIGEST);
	i_ptr->pval = 2;  /* Add 2 to INT */
	i_ptr->tohit = 10;
	i_ptr->todam = 12;
	i_ptr->toac = 10;
	i_ptr->cost = 32000L;
	return (ART_TIL);
    }

    if (i_ptr->sval == SV_SPEAR) {
	switch (randint(6)) {

	  case 1:
	    if (v_list[ART_AEGLOS].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_SLAY_TROLL | TR1_SLAY_ORC |
			      TR1_WIS | TR1_BRAND_COLD);
	    i_ptr->flags2 |= (TR2_FREE_ACT | TR2_RES_COLD);
	    i_ptr->flags3 |= (TR3_ACTIVATE | TR3_SLOW_DIGEST | TR3_BLESSED);
	    i_ptr->pval = 4;
	    i_ptr->tohit = 15;
	    i_ptr->todam = 25;
	    i_ptr->toac = 5;
	    i_ptr->damage[0] = 1;
	    i_ptr->damage[1] = 20;
	    i_ptr->cost = 140000L;
	    return (ART_AEGLOS);

	  case 2:
	    if (v_list[ART_OROME].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_BRAND_FIRE | TR1_SEARCH |
			      TR1_SLAY_GIANT | TR1_INT | TR1_INFRA);
	    i_ptr->flags2 |= (TR2_RES_LITE | TR2_RES_FIRE);
	    i_ptr->flags3 |= (TR3_LITE | TR3_HIDE_TYPE | TR3_ACTIVATE |
			      TR3_FEATHER | TR3_BLESSED | TR3_SEE_INVIS);
	    i_ptr->pval = 4;
	    i_ptr->tohit = 15;
	    i_ptr->todam = 15;
	    i_ptr->cost = 60000L;
	    return (ART_OROME);

	  default: /* case 3 to 6: */
	    if (v_list[ART_NIMLOTH].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_BRAND_COLD | TR1_SLAY_UNDEAD |
			      TR1_STEALTH);
	    i_ptr->flags2 |= (TR2_RES_COLD);
	    i_ptr->flags3 |= (TR3_SEE_INVIS);
	    i_ptr->pval = 3;
	    i_ptr->tohit = 11;
	    i_ptr->todam = 13;
	    i_ptr->cost = 30000L;
	    return (ART_NIMLOTH);
	}
    }

    if (i_ptr->sval == SV_LANCE) {
	if (v_list[ART_EORLINGAS].cur_num) return (0);
	if (randint(3) > 1) return 0;
	i_ptr->flags1 |= (TR1_SLAY_TROLL | TR1_SLAY_ORC | TR1_SLAY_EVIL | TR1_DEX);
	i_ptr->flags3 |= (TR3_SEE_INVIS);
	i_ptr->pval = 2;
	i_ptr->tohit = 3;
	i_ptr->todam = 21;
	i_ptr->weight = 360;
	i_ptr->damage[1] = 12;
	i_ptr->cost = 55000L;
	return (ART_EORLINGAS);
    }

    if (i_ptr->sval == SV_GREAT_AXE) {
	switch (randint(2)) {

	  case 1:
	    if (v_list[ART_DURIN].cur_num) return (0);
	    if (randint(6) > 1) return 0;
	    i_ptr->flags1 |= (TR1_SLAY_DEMON | TR1_SLAY_TROLL | TR1_SLAY_ORC |
			      TR1_KILL_DRAGON | TR1_CON);
	    i_ptr->flags2 |= (TR2_RES_DARK | TR2_RES_LITE | TR2_RES_CHAOS |
			      TR2_RES_FIRE | TR2_RES_ACID | TR2_FREE_ACT);
	    i_ptr->pval = 3;
	    i_ptr->tohit = 10;
	    i_ptr->todam = 20;
	    i_ptr->toac = 15;
	    i_ptr->cost = 150000L;
	    return (ART_DURIN);

	  default: /* case 2: */
	    if (v_list[ART_EONWE].cur_num) return (0);
	    if (randint(8) > 1) return 0;
	    i_ptr->flags1 |= (TR1_STR | TR1_INT | TR1_WIS |
			      TR1_DEX | TR1_CON | TR1_CHR |
			      TR1_SLAY_EVIL | TR1_SLAY_UNDEAD |
			      TR1_SLAY_ORC | TR1_BRAND_COLD);
	    i_ptr->flags2 |= (TR2_FREE_ACT | TR2_IM_COLD | TR2_RES_COLD);
	    i_ptr->flags3 |= (TR3_ACTIVATE | TR3_BLESSED | TR3_SEE_INVIS);
	    i_ptr->pval = 2;	/* +2 to all stats */
	    i_ptr->tohit = 15;
	    i_ptr->todam = 18;
	    i_ptr->toac = 8;
	    i_ptr->cost = 200000L;
	    return (ART_EONWE);
	}
    }

    if (i_ptr->sval == SV_BATTLE_AXE) {
	switch (randint(2)) {

	  case 1:
	    if (v_list[ART_BALLI].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_SLAY_DEMON | TR1_SLAY_TROLL | TR1_SLAY_ORC |
			      TR1_STR | TR1_CON |
			      TR1_STEALTH);
	    i_ptr->flags2 |= (TR2_RES_COLD | TR2_RES_ACID | TR2_RES_FIRE |
			      TR2_RES_BLIND | TR2_RES_ELEC | TR2_FREE_ACT);
	    i_ptr->flags3 |= (TR3_FEATHER | TR3_REGEN | TR3_HIDE_TYPE | TR3_SEE_INVIS);
	    i_ptr->pval = 3;	/* +3 STR/CON/Stealth */
	    i_ptr->tohit = 8;
	    i_ptr->todam = 11;
	    i_ptr->damage[0] = 3;
	    i_ptr->damage[1] = 6;
	    i_ptr->toac = 5;
	    i_ptr->cost = 90000L;
	    return (ART_BALLI);

	  default: /* case 2: */
	    if (v_list[ART_LOTHARANG].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_STR | TR1_DEX | TR1_SLAY_TROLL | TR1_SLAY_ORC);
	    i_ptr->flags3 |= (TR3_ACTIVATE);
	    i_ptr->pval = 1;	/* +1 STR/DEX */
	    i_ptr->tohit = 4;
	    i_ptr->todam = 3;
	    i_ptr->cost = 21000L;
	    return (ART_LOTHARANG);
	}
    }

    if (i_ptr->sval == SV_LOCHABER_AXE) {
	if (v_list[ART_MUNDWINE].cur_num) return (0);
	i_ptr->flags1 |= (TR1_SLAY_EVIL);
	i_ptr->flags2 |= (TR2_RES_FIRE | TR2_RES_COLD |
			  TR2_RES_ELEC | TR2_RES_ACID);
	i_ptr->tohit = 12;
	i_ptr->todam = 17;
	i_ptr->cost = 30000L;
	return (ART_MUNDWINE);
    }

    if (i_ptr->sval == SV_BROAD_AXE) {
	if (v_list[ART_BARUKKHELED].cur_num) return (0);
	i_ptr->flags1 |= (TR1_SLAY_EVIL | TR1_CON |
			  TR1_SLAY_ORC | TR1_SLAY_TROLL | TR1_SLAY_GIANT);
	i_ptr->flags3 |= (TR3_SEE_INVIS);
	i_ptr->pval = 3;	/* +3 CON */
	i_ptr->tohit = 13;
	i_ptr->todam = 19;
	i_ptr->cost = 50000L;
	return (ART_BARUKKHELED);
    }

    if (i_ptr->sval == SV_TRIDENT) {
	switch (randint(3)) {

	  case 1: case 2:
	    if (v_list[ART_WRATH].cur_num) return (0);
	    if (randint(3) > 1) return 0;
	    i_ptr->flags1 |= (TR1_SLAY_EVIL | TR1_STR | TR1_DEX |
			     TR1_SLAY_UNDEAD);
	    i_ptr->flags2 |= (TR2_RES_DARK | TR2_RES_LITE);
	    i_ptr->flags3 |= (TR3_SEE_INVIS | TR3_BLESSED);
	    i_ptr->pval = 2;	/* +2 STR/DEX */
	    i_ptr->tohit = 16;
	    i_ptr->todam = 18;
	    i_ptr->damage[0] = 3;
	    i_ptr->damage[1] = 9;
	    i_ptr->weight = 300;
	    i_ptr->cost = 90000L;
	    return (ART_WRATH);

	  default: /* case 3: */
	    if (v_list[ART_ULMO].cur_num) return (0);
	    if (randint(4) > 1) return 0;
	    i_ptr->flags1 |= (TR1_DEX |
			      TR1_SLAY_ANIMAL | TR1_SLAY_DRAGON);
	    i_ptr->flags2 |= (TR2_IM_ACID | TR2_RES_ACID | TR2_HOLD_LIFE |
			      TR2_RES_NETHER | TR2_FREE_ACT);
	    i_ptr->flags3 |= (TR3_BLESSED | TR3_SLOW_DIGEST | TR3_ACTIVATE |
			      TR3_SEE_INVIS | TR3_REGEN);
	    i_ptr->pval = 4;	/* +4 DEX */
	    i_ptr->tohit = 15;
	    i_ptr->todam = 19;
	    i_ptr->damage[0] = 4;
	    i_ptr->damage[1] = 10;
	    i_ptr->cost = 120000L;
	    return (ART_ULMO);
	}
    }

    if (i_ptr->sval == SV_SCYTHE) {
	if (v_list[ART_AVAVIR].cur_num) return (0);
	i_ptr->flags1 |= (TR1_DEX | TR1_CHR | 
			  TR1_BRAND_FIRE | TR1_BRAND_COLD);
	i_ptr->flags2 |= (TR2_RES_LITE | TR2_FREE_ACT |
			  TR2_RES_FIRE | TR2_RES_COLD);
	i_ptr->flags3 |= (TR3_LITE | TR3_ACTIVATE | TR3_SEE_INVIS);
	i_ptr->pval = 3;	/* +3 DEX/CHR */
	i_ptr->tohit = 8;
	i_ptr->todam = 8;
	i_ptr->toac = 10;
	i_ptr->cost = 18000L;
	return (ART_AVAVIR);
    }

    return (0);
}




/*
 * Help generate a unique weapon, return its name
 */
static int make_artifact_sword(inven_type *i_ptr)
{
    if (i_ptr->sval == SV_MAIN_GAUCHE) {
	if (v_list[ART_MAEDHROS].cur_num) return (0);
	if (randint(4) > 1) return 0;
	i_ptr->flags1 |= (TR1_DEX | TR1_INT |
			  TR1_SLAY_GIANT | TR1_SLAY_TROLL);
	i_ptr->flags2 |= (TR2_FREE_ACT);
	i_ptr->flags3 |= (TR3_SEE_INVIS);
	i_ptr->pval = 3;	/* +3 DEX/INT */
	i_ptr->tohit = 12;
	i_ptr->todam = 15;
	i_ptr->damage[0] = 2;
	i_ptr->damage[1] = 6;
	i_ptr->cost = 20000L;
	return (ART_MAEDHROS);
    }

    if (i_ptr->sval == SV_DAGGER) {
	switch (randint(11)) {

	  case 1:
	    if (v_list[ART_ANGRIST].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_SLAY_TROLL | TR1_SLAY_ORC | TR1_SLAY_EVIL |
			      TR1_DEX);
	    i_ptr->flags2 |= (TR2_RES_DARK | TR2_FREE_ACT | TR2_SUST_DEX);
	    i_ptr->pval = 4;  /* +4 DEX */
	    i_ptr->tohit = 10;
	    i_ptr->todam = 15;
	    i_ptr->toac = 5;
	    i_ptr->damage[0] = 2;
	    i_ptr->damage[1] = 5;
	    i_ptr->cost = 100000L;
	    return (ART_ANGRIST);

	  case 2: case 3:
	    if (v_list[ART_NARTHANC].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_BRAND_FIRE);
	    i_ptr->flags2 |= (TR2_RES_FIRE);
	    i_ptr->flags3 |= (TR3_ACTIVATE);
	    i_ptr->tohit = 4;
	    i_ptr->todam = 6;
	    i_ptr->cost = 12000;
	    return (ART_NARTHANC);

	  case 4: case 5:
	    if (v_list[ART_NIMTHANC].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_BRAND_COLD);
	    i_ptr->flags2 |= (TR2_RES_COLD);
	    i_ptr->flags3 |= (TR3_ACTIVATE);
	    i_ptr->tohit = 4;
	    i_ptr->todam = 6;
	    i_ptr->cost = 11000L;
	    return (ART_NIMTHANC);

	  case 6: case 7:
	    if (v_list[ART_DETHANC].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_BRAND_ELEC);
	    i_ptr->flags2 |= (TR2_RES_ELEC);
	    i_ptr->flags3 |= (TR3_ACTIVATE);
	    i_ptr->tohit = 4;
	    i_ptr->todam = 6;
	    i_ptr->cost = 13000L;
	    return (ART_DETHANC);

	  case 8: case 9:
	    if (v_list[ART_RILIA].cur_num) return (0);
	    i_ptr->flags1 |= (TR2_RES_POIS);
	    i_ptr->flags2 |= (TR2_RES_DISEN);
	    i_ptr->flags3 |= (TR3_ACTIVATE);
	    i_ptr->tohit = 4;
	    i_ptr->todam = 3;
	    i_ptr->damage[0] = 2;
	    i_ptr->damage[1] = 4;
	    i_ptr->cost = 15000L;
	    return (ART_RILIA);

	  default: /* case 10: case 11: */
	    if (v_list[ART_BELANGIL].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_BRAND_COLD | TR1_DEX);
	    i_ptr->flags2 |= (TR2_RES_COLD);
	    i_ptr->flags3 |= (TR3_SLOW_DIGEST | TR3_ACTIVATE | TR3_SEE_INVIS | TR3_REGEN);
	    i_ptr->pval = 2;	/* +2 DEX */
	    i_ptr->tohit = 6;
	    i_ptr->todam = 9;
	    i_ptr->damage[0] = 3;
	    i_ptr->damage[1] = 2;
	    i_ptr->cost = 40000L;
	    return (ART_BELANGIL);
	}
    }

    if (i_ptr->sval == SV_BASTARD_SWORD) {
	if (v_list[ART_CALRIS].cur_num) return (0);
	i_ptr->flags1 |= (TR1_SLAY_DEMON | TR1_SLAY_TROLL | TR1_KILL_DRAGON |
			  TR1_CON | TR1_SLAY_EVIL);
	i_ptr->flags2 |= (TR2_RES_DISEN);
	i_ptr->flags3 |= (TR3_HEAVY_CURSE | TR3_CURSED | TR3_AGGRAVATE);
	i_ptr->pval = 5;
	i_ptr->tohit = -20;
	i_ptr->todam = 20;
	i_ptr->damage[0] = 3;
	i_ptr->damage[1] = 7;
	i_ptr->cost = 100000L;
	return (ART_CALRIS);
    }

    if (i_ptr->sval == SV_BROAD_SWORD) {
	switch (randint(12)) {

	  case 1: case 2:
	    if (v_list[ART_ARUNRUTH].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_SLAY_DEMON | TR1_SLAY_ORC | TR1_DEX);
	    i_ptr->flags2 |= (TR2_FREE_ACT | TR2_RES_COLD);
	    i_ptr->flags3 |= (TR3_ACTIVATE | TR3_FEATHER | TR3_SLOW_DIGEST);
	    i_ptr->pval = 4;
	    i_ptr->tohit = 20;
	    i_ptr->todam = 12;
	    i_ptr->damage[0] = 3;
	    i_ptr->cost = 50000L;
	    return (ART_ARUNRUTH);

	  case 3: case 4: case 5: case 6:
	    if (v_list[ART_GLAMDRING].cur_num) return (0);
	    i_ptr->tohit = 10;
	    i_ptr->todam = 15;
	    i_ptr->flags1 |= (TR1_SLAY_ORC | TR1_SLAY_EVIL | TR1_SEARCH |
			      TR1_BRAND_FIRE);
	    i_ptr->flags2 |= (TR2_RES_LITE | TR2_RES_FIRE);
	    i_ptr->flags3 |= (TR3_LITE | TR3_SLOW_DIGEST);
	    i_ptr->pval = 3;	/* +3 Search */
	    i_ptr->cost = 40000L;
	    return (ART_GLAMDRING);

	  case 7:
	    if (v_list[ART_AEGLIN].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_SLAY_ORC | TR1_SEARCH | TR1_BRAND_ELEC);
	    i_ptr->flags2 |= (TR2_RES_ELEC);
	    i_ptr->flags3 |= (TR3_LITE | TR3_SLOW_DIGEST);
	    i_ptr->pval = 4;	/* +4 Search */
	    i_ptr->tohit = 12;
	    i_ptr->todam = 16;
	    i_ptr->cost = 45000L;
	    return (ART_AEGLIN);

	  default: /* case 8 to 12: */
	    if (v_list[ART_ORCRIST].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_SLAY_ORC | TR1_SLAY_EVIL |
			      TR1_STEALTH | TR1_BRAND_COLD);
	    i_ptr->flags2 |= (TR2_RES_COLD);
	    i_ptr->flags3 |= (TR3_LITE | TR3_SLOW_DIGEST);
	    i_ptr->pval = 3;
	    i_ptr->tohit = 10;
	    i_ptr->todam = 15;
	    i_ptr->cost = 40000L;
	    return (ART_ORCRIST);
	}
    }

    if (i_ptr->sval == SV_TWO_HANDED_SWORD) {
	switch (randint(8)) {

	  case 1: case 2:
	    if (v_list[ART_GURTHANG].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_SLAY_TROLL | TR1_KILL_DRAGON | TR1_STR);
	    i_ptr->flags2 |= (TR2_FREE_ACT);
	    i_ptr->flags3 |= (TR3_REGEN | TR3_SLOW_DIGEST);
	    i_ptr->pval = 2;
	    i_ptr->tohit = 13;
	    i_ptr->todam = 17;
	    i_ptr->cost = 100000L;
	    return (ART_GURTHANG);

	  case 3:
	    if (v_list[ART_ZARCUTHRA].cur_num) return (0);
	    if (randint(3) > 1) return 0;
	    i_ptr->flags1 |= (TR1_SLAY_TROLL | TR1_SLAY_ORC | TR1_SLAY_GIANT |
			      TR1_SLAY_DEMON | TR1_KILL_DRAGON | TR1_SLAY_EVIL | TR1_SLAY_ANIMAL |
			      TR1_STR | TR1_SLAY_UNDEAD | TR1_CHR |
			      TR1_BRAND_FIRE | TR1_INFRA);
	    i_ptr->flags2 |= (TR2_FREE_ACT | TR2_RES_CHAOS | TR2_RES_FIRE);
	    i_ptr->flags3 |= (TR3_HIDE_TYPE | TR3_AGGRAVATE | TR3_SEE_INVIS);
	    i_ptr->pval = 4;	/* +4 STR/CHR */
	    i_ptr->tohit = 19;
	    i_ptr->todam = 21;
	    i_ptr->damage[0] = 6;
	    i_ptr->damage[1] = 4;
	    i_ptr->cost = 200000L;
	    return (ART_ZARCUTHRA);

	  default: /* case 4 to 8: */
	    if (v_list[ART_MORMEGIL].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_SPEED);
	    i_ptr->flags3 |= (TR3_HEAVY_CURSE | TR3_AGGRAVATE | TR3_CURSED);
	    i_ptr->pval = -1;	/* -1 Speed */
	    i_ptr->tohit = -40;
	    i_ptr->todam = -60;
	    i_ptr->toac = -50;
	    i_ptr->cost = 100000L;	/* Kind of academic! */
	    return (ART_MORMEGIL);
	}
    }

    if (i_ptr->sval == SV_CUTLASS) {
	if (v_list[ART_GONDRICAM].cur_num) return (0);
	i_ptr->flags1 |= (TR1_STEALTH | TR1_DEX);
	i_ptr->flags2 |= (TR2_RES_FIRE | TR2_RES_COLD |
			  TR2_RES_ACID | TR2_RES_ELEC);
	i_ptr->flags3 |= (TR3_FEATHER | TR3_REGEN | TR3_SEE_INVIS | TR3_HIDE_TYPE);
	i_ptr->pval = 3;	/* +3 DEX/Stealth */
	i_ptr->tohit = 10;
	i_ptr->todam = 11;
	i_ptr->cost = 28000L;
	return (ART_GONDRICAM);
    }

    if (i_ptr->sval == SV_EXECUTIONERS_SWORD) {
	if (v_list[ART_CRISDURIAN].cur_num) return (0);
	if (randint(2) > 1) return 0;
	i_ptr->flags1 |= (TR1_SLAY_EVIL | TR1_SLAY_UNDEAD |
			 TR1_SLAY_DRAGON | TR1_SLAY_GIANT | TR1_SLAY_ORC |
			 TR1_SLAY_TROLL);
	i_ptr->flags3 |= (TR3_SEE_INVIS);
	i_ptr->tohit = 18;
	i_ptr->todam = 19;
	i_ptr->cost = 100000L;
	return (ART_CRISDURIAN);
    }

    if (i_ptr->sval == SV_KATANA) {
	if (v_list[ART_AGLARANG].cur_num) return (0);
	if (randint(3) > 1) return 0;
	i_ptr->flags1 |= (TR1_DEX);
	i_ptr->flags2 |= (TR2_SUST_CON);
	i_ptr->pval = 5;  /* Add 5 to Dex */
	i_ptr->tohit = 0;
	i_ptr->todam = 0;
	i_ptr->damage[0] = 6;
	i_ptr->damage[1] = 8;
	i_ptr->weight = 50;
	i_ptr->cost = 40000L;
	return (ART_AGLARANG);
    }

    if (i_ptr->sval == SV_LONG_SWORD) {
	switch (randint(15)) {

	  case 1:
	    if (v_list[ART_RINGIL].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_SLAY_DEMON | TR1_SLAY_TROLL |
			      TR1_SLAY_UNDEAD | TR1_SLAY_EVIL |
			      TR1_SPEED | TR1_BRAND_COLD);
	    i_ptr->flags2 |= (TR2_FREE_ACT | TR2_RES_LITE | TR2_RES_COLD);
	    i_ptr->flags3 |= (TR3_LITE | TR3_REGEN | TR3_ACTIVATE |
			      TR3_SEE_INVIS | TR3_SLOW_DIGEST);
	    i_ptr->pval = 1;	/* +1 Speed! */
	    i_ptr->tohit = 22;
	    i_ptr->todam = 25;
	    i_ptr->damage[0] = 4;
	    i_ptr->cost = 300000L;
	    return (ART_RINGIL);

	  case 2: case 3: case 4:
	    if (v_list[ART_ANDURIL].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_SLAY_TROLL | TR1_SLAY_ORC | 
			      TR1_SLAY_EVIL | TR1_STR | TR1_BRAND_FIRE);
	    i_ptr->flags2 |= (TR2_FREE_ACT | TR2_RES_FIRE | TR2_SUST_DEX);
	    i_ptr->flags3 |= (TR3_SEE_INVIS | TR3_ACTIVATE);
	    i_ptr->pval = 4; /* Add 4 to strength */
	    i_ptr->tohit = 10;
	    i_ptr->todam = 15;
	    i_ptr->toac = 5;
	    i_ptr->cost = 80000L;
	    return (ART_ANDURIL);

	  case 5: case 6: case 7: case 8:
	    if (v_list[ART_ANGUIREL].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_SLAY_EVIL |
			      TR1_SLAY_DEMON | TR1_BRAND_ELEC | TR1_STR | TR1_CON);
	    i_ptr->flags2 |= (TR2_RES_ELEC | TR2_FREE_ACT | TR2_RES_LITE);
	    i_ptr->flags3 |= (TR3_LITE | TR3_SEE_INVIS);
	    i_ptr->pval = 2;
	    i_ptr->tohit = 8;
	    i_ptr->todam = 12;
	    i_ptr->cost = 40000L;
	    return (ART_ANGUIREL);

	  default: /* case 9 to 15: */
	    if (v_list[ART_ELVAGIL].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_CHR | TR1_DEX | TR1_STEALTH | 
			     TR1_SLAY_TROLL | TR1_SLAY_ORC);
	    i_ptr->flags3 |= (TR3_FEATHER | TR3_SEE_INVIS | TR3_HIDE_TYPE);
	    i_ptr->pval = 2;	/* +2 DEX/CHR/Stealth */
	    i_ptr->tohit = 2;
	    i_ptr->todam = 7;
	    i_ptr->cost = 30000L;
	    return (ART_ELVAGIL);
	}
    }

    if (i_ptr->sval == SV_RAPIER) {
	if (v_list[ART_FORASGIL].cur_num) return (0);
	i_ptr->flags1 |= (TR1_BRAND_COLD | TR1_SLAY_ANIMAL);
	i_ptr->flags2 |= (TR2_RES_COLD | TR2_RES_LITE);
	i_ptr->flags3 |= (TR3_LITE);
	i_ptr->tohit = 12;
	i_ptr->todam = 19;
	i_ptr->cost = 15000L;
	return (ART_FORASGIL);
    }

    if (i_ptr->sval == SV_SABRE) {
	if (v_list[ART_CARETH].cur_num) return (0);
	i_ptr->flags1 |= (TR1_SLAY_GIANT | TR1_SLAY_ORC | TR1_SLAY_TROLL |
			  TR1_SLAY_DRAGON | TR1_SLAY_ANIMAL | TR1_ATTACK_SPD);
	i_ptr->pval = 1;
	i_ptr->tohit = 6;
	i_ptr->todam = 8;
	i_ptr->cost = 25000L;
	return (ART_CARETH);
    }

    if (i_ptr->sval == SV_SMALL_SWORD) {
	if (v_list[ART_STING].cur_num) return (0);
	i_ptr->flags1 |= (TR1_SLAY_ORC | TR1_SLAY_EVIL |
			 TR1_SLAY_UNDEAD | TR1_DEX | TR1_CON | TR1_STR |
			 TR1_ATTACK_SPD);
	i_ptr->flags2 |= (TR2_FREE_ACT | TR2_RES_LITE);
	i_ptr->flags3 |= (TR3_LITE | TR3_SEE_INVIS);
	i_ptr->pval = 2;	/* +2 DEX/CON/STR/Attacks */
	i_ptr->tohit = 7;
	i_ptr->todam = 8;
	i_ptr->cost = 100000L;
	return (ART_STING);
    }

    if (i_ptr->sval == SV_SCIMITAR) {
	if (v_list[ART_HARADEKKET].cur_num) return (0);
	i_ptr->flags1 |= (TR1_DEX | TR1_ATTACK_SPD | 
			 TR1_SLAY_EVIL | TR1_SLAY_UNDEAD | TR1_SLAY_ANIMAL);
	i_ptr->flags3 |= (TR3_SEE_INVIS | TR3_HIDE_TYPE);
	i_ptr->pval = 2;	/* +2 DEX/Attacks */
	i_ptr->tohit = 9;
	i_ptr->todam = 11;
	i_ptr->cost = 30000L;
	return (ART_HARADEKKET);
    }

    if (i_ptr->sval == SV_SHORT_SWORD) {
	if (v_list[ART_GILETTAR].cur_num) return (0);
	i_ptr->flags1 |= (TR1_ATTACK_SPD | TR1_SLAY_ANIMAL);
	i_ptr->flags3 |= (TR3_SLOW_DIGEST | TR3_REGEN);
	i_ptr->pval = 2;	/* +2 Attacks */
	i_ptr->tohit = 3;
	i_ptr->todam = 7;
	i_ptr->cost = 15000L;
	return (ART_GILETTAR);
    }

    if (i_ptr->sval == SV_BLADE_OF_CHAOS) {
	if (v_list[ART_DOOMCALLER].cur_num) return (0);
	if (randint(3) > 1) return 0;
	i_ptr->flags1 |= (TR1_CON | TR1_SLAY_TROLL | TR1_SLAY_ORC | TR1_SLAY_ANIMAL | 
			  TR1_KILL_DRAGON | TR1_BRAND_COLD | TR1_SLAY_EVIL);
	i_ptr->flags2 |= (TR2_RES_FIRE | TR2_RES_COLD | TR2_RES_ELEC | TR2_RES_ACID | 
			  TR2_FREE_ACT);
	i_ptr->flags3 |= (TR3_AGGRAVATE | TR3_SEE_INVIS | TR3_TELEPATHY);
	i_ptr->pval = -5;	/* -5 CON */
	i_ptr->tohit = 18;
	i_ptr->todam = 28;
	i_ptr->cost = 200000L;
	return (ART_DOOMCALLER);
    }

    return (0);
}


/*
 * Make an artifact soft armour
 */
static int make_artifact_soft_armor(inven_type *i_ptr)
{
    if (i_ptr->sval == SV_SOFT_LEATHER_ARMOR) {
	if (v_list[ART_HITHLOMIR].cur_num) return (0);
	i_ptr->flags1 |= (TR1_STEALTH);
	i_ptr->flags2 |= (TR2_RES_ACID | TR2_RES_FIRE | TR2_RES_COLD |
			  TR2_RES_ELEC | TR2_RES_DARK);
	i_ptr->pval = 4;	/* +4 Stealth */
	i_ptr->toac = 20;
	i_ptr->cost = 45000L;
	return (ART_HITHLOMIR);
    }

    if (i_ptr->sval == SV_LEATHER_SCALE_MAIL) {
	if (v_list[ART_THALKETTOTH].cur_num) return (0);
	i_ptr->flags1 |= (TR1_DEX);
	i_ptr->flags2 |= (TR2_RES_ACID | TR2_RES_SHARDS);
	i_ptr->pval = 3;	/* +3 Dex */
	i_ptr->toac = 25;
	i_ptr->weight = 60;
	i_ptr->cost = 25000L;
	return (ART_THALKETTOTH);
    }

    return (0);
}


/*
 * Make an artifact hard armour
 */
static int make_artifact_hard_armor(inven_type *i_ptr)
{
    if (i_ptr->sval == SV_ADAMANTITE_PLATE_MAIL) {
	if (v_list[ART_SOULKEEPER].cur_num) return (0);
	if (randint(3) > 1) return 0;
	i_ptr->flags2 |= (TR2_HOLD_LIFE | TR2_RES_CHAOS |
			  TR2_RES_DARK | TR2_RES_NEXUS | TR2_RES_NETHER |
			  TR2_RES_ACID | TR2_RES_COLD);
	i_ptr->flags3 |= (TR3_ACTIVATE);
	i_ptr->toac = 20;
	i_ptr->cost = 300000L;
	return (ART_SOULKEEPER);
    }

    if (i_ptr->sval == SV_FULL_PLATE_ARMOUR) {
	if (v_list[ART_ISILDUR].cur_num) return (0);
	i_ptr->flags2 |= (TR2_RES_SOUND | TR2_RES_NEXUS |
			  TR2_RES_ACID | TR2_RES_FIRE | TR2_RES_COLD | TR2_RES_ELEC);
	i_ptr->tohit = 0;
	i_ptr->toac = 25;
	i_ptr->weight = 300;
	i_ptr->cost = 40000L;
	return (ART_ISILDUR);
    }

    if (i_ptr->sval == SV_METAL_BRIGANDINE_ARMOUR) {
	if (v_list[ART_ROHIRRIM].cur_num) return (0);
	i_ptr->flags1 |= (TR1_STR | TR1_DEX);
	i_ptr->flags2 |= (TR2_RES_ACID | TR2_RES_FIRE | TR2_RES_COLD | TR2_RES_ELEC |
			  TR2_RES_SOUND | TR2_RES_CONF);
	i_ptr->tohit = 0;
	i_ptr->pval = 2;	/* +2 STR/DEX */
	i_ptr->toac = 15;
	i_ptr->weight = 200;
	i_ptr->cost = 30000L;
	return (ART_ROHIRRIM);
    }

    if (i_ptr->sval == SV_MITHRIL_CHAIN_MAIL) {
	if (v_list[ART_BELEGENNON].cur_num) return (0);
	i_ptr->flags1 |= (TR1_STEALTH);
	i_ptr->flags2 |= (TR2_RES_ACID | TR2_RES_FIRE | TR2_RES_COLD |
			  TR2_RES_ELEC);
	i_ptr->flags3 |= (TR3_ACTIVATE);
	i_ptr->pval = 4;	/* +4 Stealth */
	i_ptr->toac = 20;
	i_ptr->cost = 105000L;
	return (ART_BELEGENNON);
    }

    if (i_ptr->sval == SV_MITHRIL_PLATE_MAIL) {
	if (v_list[ART_CELEBORN].cur_num) return (0);
	i_ptr->flags1 |= (TR1_STR | TR1_CHR);
	i_ptr->flags2 |= (TR2_RES_ACID | TR2_RES_FIRE | TR2_RES_COLD |
			  TR2_RES_ELEC | TR2_RES_DISEN | TR2_RES_DARK);
	i_ptr->flags3 |= (TR3_ACTIVATE);
	i_ptr->pval = 4;
	i_ptr->toac = 25;
	i_ptr->weight = 250;
	i_ptr->cost = 150000L;
	return (ART_CELEBORN);
    }

    if (i_ptr->sval == SV_CHAIN_MAIL) {
	if (v_list[ART_ARVEDUI].cur_num) return (0);
	i_ptr->flags1 |= (TR1_STR | TR1_CHR);
	i_ptr->flags2 |= (TR2_RES_ACID | TR2_RES_FIRE | TR2_RES_COLD |
			  TR2_RES_ELEC | TR2_RES_NEXUS | TR2_RES_SHARDS);
	i_ptr->pval = 2;
	i_ptr->toac = 15;
	i_ptr->cost = 32000L;
	return (ART_ARVEDUI);
    }

    if (i_ptr->sval == SV_AUGMENTED_CHAIN_MAIL) {
	if (v_list[ART_CASPANION].cur_num) return (0);
	if (randint(3) > 1) return 0;
	i_ptr->flags1 |= (TR1_CON | TR1_WIS | TR1_INT);
	i_ptr->flags2 |= (TR2_RES_ACID | TR2_RES_POIS | TR2_RES_CONF);
	i_ptr->flags3 |= (TR3_ACTIVATE);
	i_ptr->pval = 3;	/* +3 INT/WIS/CON */
	i_ptr->toac = 20;
	i_ptr->cost = 40000L;
	return (ART_CASPANION);
    }

    return (0);
}    


/*
 * Make an artifact Dragon Scale Mail Armour
 */
static int make_artifact_drag_armor(inven_type *i_ptr)
{
    if (i_ptr->sval == SV_DRAGON_MULTIHUED) {
	if (v_list[ART_RAZORBACK].cur_num) return (0);
	if (randint(3) > 1) return 0;
	i_ptr->flags1 |= (TR1_INT | TR1_WIS | TR1_STEALTH);
	i_ptr->flags2 |= (TR2_FREE_ACT | TR2_RES_FIRE | TR2_RES_COLD |
	                  TR2_RES_ACID | TR2_RES_POIS |
			  TR2_RES_ELEC | TR2_IM_ELEC | TR2_RES_LITE);
	i_ptr->flags3 |= (TR3_LITE | TR3_SEE_INVIS | TR3_HIDE_TYPE |
	                  TR3_ACTIVATE | TR3_AGGRAVATE);
	i_ptr->pval = -2;	/* -2 INT/WIS/Stealth */
	i_ptr->toac = 25;
	i_ptr->tohit = -3;
	i_ptr->ac = 30;
	i_ptr->weight = 400;
	i_ptr->cost = 400000L;
	return (ART_RAZORBACK);
    }

    if (i_ptr->sval == SV_DRAGON_POWER) {
	if (v_list[ART_BLADETURNER].cur_num) return (0);
	i_ptr->flags1 |= (TR1_DEX | TR1_SEARCH);
	i_ptr->flags2 |= (TR2_RES_FIRE | TR2_RES_COLD | TR2_RES_ACID | TR2_RES_POIS |
			  TR2_RES_ELEC | TR2_HOLD_LIFE | TR2_RES_CONF | TR2_RES_SOUND |
			  TR2_RES_LITE | TR2_RES_DARK | TR2_RES_CHAOS |
			  TR2_RES_DISEN | TR2_RES_SHARDS |
			  TR2_RES_BLIND | TR2_RES_NEXUS | TR2_RES_NETHER);
	i_ptr->flags3 |= (TR3_HIDE_TYPE | TR3_ACTIVATE | TR3_REGEN);
	i_ptr->pval = -3;	/* -3 DEX, Search */
	i_ptr->toac = 35;
	i_ptr->ac = 50;
	i_ptr->tohit = -4;
	i_ptr->weight = 500;
	i_ptr->cost = 500000L;
	return (ART_BLADETURNER);
    }

    return (0);
}


/*
 * Make an artifact shield
 */
static int make_artifact_shield(inven_type *i_ptr)
{
    if (i_ptr->sval == SV_SMALL_METAL_SHIELD) {
	if (v_list[ART_THORIN].cur_num) return (0);
	if (randint(2) > 1) return 0;
	i_ptr->flags1 |= (TR1_CON | TR1_STR | TR1_SEARCH);
	i_ptr->flags2 |= (TR2_FREE_ACT | TR2_RES_ACID | TR2_RES_SOUND |
			  TR2_RES_CHAOS | TR2_IM_ACID);
	i_ptr->flags3 |= (TR3_HIDE_TYPE);
	i_ptr->pval = 4;	/* +4 STR/CON/Search */
	i_ptr->tohit = 0;
	i_ptr->toac = 25;
	i_ptr->cost = 60000L;
	return (ART_THORIN);
    }

    if (i_ptr->sval == SV_LARGE_LEATHER_SHIELD) {
	if (v_list[ART_CELEGORM].cur_num) return (0);
	i_ptr->flags2 |= (TR2_RES_ACID | TR2_RES_FIRE | TR2_RES_COLD | TR2_RES_ELEC |
			  TR2_RES_LITE | TR2_RES_DARK);
	i_ptr->toac = 20;
	i_ptr->weight = 60;
	i_ptr->cost = 12000L;
	return (ART_CELEGORM);
    }

    if (i_ptr->sval == SV_LARGE_METAL_SHIELD) {
	if (v_list[ART_ANARION].cur_num) return (0);
	if (randint(3) > 1) return 0;
	i_ptr->flags2 |= (TR2_RES_ACID | TR2_RES_FIRE | 
			  TR2_RES_COLD | TR2_RES_ELEC |
			  TR2_SUST_STR | TR2_SUST_CON | TR2_SUST_DEX |
			  TR2_SUST_INT | TR2_SUST_WIS | TR2_SUST_CHR);
	i_ptr->tohit = 0;
	i_ptr->toac = 20;
	i_ptr->cost = 160000L;
	return (ART_ANARION);
    }

    return (0);
}


/*
 * Make an artifact helm
 */
static int make_artifact_helm(inven_type *i_ptr)
{
    if (i_ptr->sval == SV_IRON_CROWN) {

	/* Hack -- make this as soon as "possible" */
	if (permit_morgoth && !v_list[ART_MORGOTH].cur_num) {
	    i_ptr->flags1 |= (TR1_STR | TR1_DEX | TR1_CON |
			      TR1_INT | TR1_WIS | TR1_CHR |
			      TR1_INFRA);
	    i_ptr->flags3 |= (TR3_TELEPATHY | TR3_LITE |
			      TR3_SEE_INVIS | TR3_HIDE_TYPE | 
			      TR3_PERMA_CURSE | TR3_HEAVY_CURSE | TR3_CURSED);
	    i_ptr->pval = 125;  /* +125 to all stats! */
	    i_ptr->cost = 10000000L;
	    return (ART_MORGOTH);
	}

	/* Otherwise consider this one... */
	else {

	    if (v_list[ART_BERUTHIEL].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_STR | TR1_DEX | TR1_CON);
	    i_ptr->flags2 |= (TR2_RES_ACID | TR2_FREE_ACT);
	    i_ptr->flags3 |= (TR3_TELEPATHY | TR3_CURSED | TR3_SEE_INVIS);
	    i_ptr->pval = -125;	/* -125 STR/DEX/CON */
	    i_ptr->toac = 20;
	    i_ptr->cost = 10000L;
	    return (ART_BERUTHIEL);
	}
    }


    if (i_ptr->sval == SV_HARD_LEATHER_CAP) {
	if (v_list[ART_THRANDUIL].cur_num) return (0);
	i_ptr->flags1 |= (TR1_INT | TR1_WIS);
	i_ptr->flags2 |= (TR2_RES_ACID | TR2_RES_BLIND);
	i_ptr->flags3 |= (TR3_TELEPATHY);
	i_ptr->pval = 2;	/* +2 INT/WIS */
	i_ptr->toac = 10;
	i_ptr->cost = 50000L;
	return (ART_THRANDUIL);
    }

    if (i_ptr->sval == SV_METAL_CAP) {
	if (v_list[ART_THENGEL].cur_num) return (0);
	i_ptr->flags1 |= (TR1_WIS | TR1_CHR);
	i_ptr->flags2 |= (TR2_RES_ACID);
	i_ptr->pval = 3;	/* +3 WIS/CHR */
	i_ptr->toac = 12;
	i_ptr->cost = 22000L;
	return (ART_THENGEL);
    }

    if (i_ptr->sval == SV_STEEL_HELM) {
	if (v_list[ART_HAMMERHAND].cur_num) return (0);
	i_ptr->flags1 |= (TR1_STR | TR1_CON | TR1_DEX);
	i_ptr->flags2 |= (TR2_RES_NEXUS | TR2_RES_ACID);
	i_ptr->pval = 3;	/* +3 STR/CON/DEX */
	i_ptr->toac = 20;
	i_ptr->cost = 45000L;
	return (ART_HAMMERHAND);
    }

    if (i_ptr->sval == SV_IRON_HELM) {
	switch (randint(12)) {
	  case 1: case 2:
	    if (v_list[ART_DOR].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_CON | TR1_DEX | TR1_STR);
	    i_ptr->flags2 |= (TR2_RES_ACID | TR2_RES_FIRE | TR2_RES_COLD |
			      TR2_RES_ELEC | TR2_RES_LITE | TR2_RES_BLIND);
	    i_ptr->flags3 |= (TR3_TELEPATHY | TR3_LITE | TR3_SEE_INVIS);
	    i_ptr->pval = 4;	/* +4 STR/DEX/CON */
	    i_ptr->toac = 20;
	    i_ptr->cost = 300000L;
	    return (ART_DOR);

	  case 3: case 4: case 5: case 6: case 7:
	    if (v_list[ART_HOLHENNETH].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_INT | TR1_WIS | TR1_SEARCH);
	    i_ptr->flags2 |= (TR2_RES_BLIND | TR2_RES_ACID);
	    i_ptr->flags3 |= (TR3_HIDE_TYPE | TR3_ACTIVATE | TR3_SEE_INVIS);
	    i_ptr->pval = 2;	/* +2 INT/WIS/Search */
	    i_ptr->toac = 10;
	    i_ptr->cost = 100000L;
	    return (ART_HOLHENNETH);

	  default: /* case 8 to 12: */
	    if (v_list[ART_GORLIM].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_INT | TR1_WIS | TR1_SEARCH);
	    i_ptr->flags3 |= (TR3_CURSED | TR3_AGGRAVATE | 
	                      TR3_HIDE_TYPE | TR3_SEE_INVIS);
	    i_ptr->pval = -125;		/* -125 INT/WIS/Search */
	    i_ptr->toac = 10;
	    i_ptr->cost = 0L;
	    return (ART_GORLIM);
	}
    }

    if (i_ptr->sval == SV_GOLDEN_CROWN) {
	if (v_list[ART_GONDOR].cur_num) return (0);
	if (randint(3) > 1) return 0;
	i_ptr->flags1 |= (TR1_STR | TR1_CON | TR1_WIS);
	i_ptr->flags2 |= (TR2_RES_LITE |
			  TR2_RES_BLIND | TR2_RES_ACID | TR2_RES_FIRE);
	i_ptr->flags3 |= (TR3_LITE | TR3_ACTIVATE | TR3_SEE_INVIS | TR3_REGEN);
	i_ptr->pval = 3;	/* +3 STR/CON/WIS */
	i_ptr->toac = 15;
	i_ptr->cost = 100000L;
	return (ART_GONDOR);
    }


    return (0);
}


/*
 * Make an artifact cloak
 */
static int make_artifact_cloak(inven_type *i_ptr)
{
    if (i_ptr->sval == SV_CLOAK) {

	switch (randint(9)) {

	  case 1:
	  case 2:
	    if (v_list[ART_COLLUIN].cur_num) return (0);
	    i_ptr->flags2 |= (TR2_RES_FIRE | TR2_RES_COLD | TR2_RES_POIS |
			      TR2_RES_ELEC | TR2_RES_ACID);
	    i_ptr->flags3 |= (TR3_ACTIVATE);
	    i_ptr->toac = 15;
	    i_ptr->cost = 10000L;
	    return (ART_COLLUIN);

	  case 3:
	  case 4:
	    if (v_list[ART_HOLCOLLETH].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_INT | TR1_WIS | TR1_STEALTH);
	    i_ptr->flags2 |= (TR2_RES_ACID);
	    i_ptr->flags3 |= (TR3_ACTIVATE | TR3_HIDE_TYPE);
	    i_ptr->pval = 2;	/* +2 INT/WIS/Stealth */
	    i_ptr->toac = 4;
	    i_ptr->cost = 13000L;
	    return (ART_HOLCOLLETH);

	  case 5:
	    if (v_list[ART_THINGOL].cur_num) return (0);
	    i_ptr->toac = 18;
	    i_ptr->flags1 |= (TR1_DEX | TR1_CHR);
	    i_ptr->flags2 |= (TR2_RES_FIRE | TR2_RES_ACID | TR2_RES_COLD | TR2_FREE_ACT);
	    i_ptr->flags3 |= (TR3_ACTIVATE);
	    i_ptr->pval = 3;	/* +3 DEX/CHR */
	    i_ptr->cost = 35000L;
	    return (ART_THINGOL);

	  case 6:
	  case 7:
	    if (v_list[ART_THORONGIL].cur_num) return (0);
	    i_ptr->flags2 |= (TR2_RES_ACID | TR2_FREE_ACT);
	    i_ptr->flags3 |= (TR3_SEE_INVIS);
	    i_ptr->toac = 10;
	    i_ptr->cost = 8000L;
	    return (ART_THORONGIL);

	  case 8:
	  case 9:
	    if (v_list[ART_COLANNON].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_STEALTH);
	    i_ptr->flags2 |= (TR2_RES_ACID);
	    i_ptr->flags3 |= (TR3_ACTIVATE);
	    i_ptr->pval = 3;	/* +3 Stealth */
	    i_ptr->toac = 15;
	    i_ptr->cost = 11000L;
	    return (ART_COLANNON);
	}
    }

    if (i_ptr->sval == SV_SHADOW_CLOAK) {

	if (randint(2) == 1) return (0);

	switch (randint(2)) {

	  case 1:
	    if (v_list[ART_LUTHIEN].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_INT | TR1_WIS | TR1_CHR);
	    i_ptr->flags2 |= (TR2_RES_FIRE | TR2_RES_COLD | TR2_RES_ACID);
	    i_ptr->flags3 |= (TR3_ACTIVATE);
	    i_ptr->pval = 2;	/* +2 INT/WIS/CHR */
	    i_ptr->toac = 20;
	    i_ptr->cost = 45000L;
	    return (ART_LUTHIEN);

	  case 2:
	    if (v_list[ART_TUOR].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_STEALTH);
	    i_ptr->flags2 |= (TR2_IM_ACID | TR2_RES_ACID | TR2_FREE_ACT);
	    i_ptr->flags3 |= (TR3_SEE_INVIS);
	    i_ptr->pval = 4;	/* +4 Stealth */
	    i_ptr->toac = 12;
	    i_ptr->cost = 35000L;
	    return (ART_TUOR);
	}
    }

    return (0);
}



/*
 * Make artifact gloves
 */
static int make_artifact_gloves(inven_type *i_ptr)
{
    if (i_ptr->sval == SV_SET_OF_LEATHER_GLOVES) {

	switch (randint(3)) {

	  case 1:
	    if (v_list[ART_CAMBELEG].cur_num) return (0);
	    i_ptr->flags1 |= (TR1_STR | TR1_CON);
	    i_ptr->flags2 |= (TR2_FREE_ACT);
	    i_ptr->flags3 |= (TR3_SHOW_MODS);
	    i_ptr->pval = 2;	/* +2 STR/CON */
	    i_ptr->tohit = 8;
	    i_ptr->todam = 8;
	    i_ptr->toac = 15;
	    i_ptr->cost = 36000L;
	    return (ART_CAMBELEG);

	  default: /* case 2 and 3: */
	    if (v_list[ART_CAMMITHRIM].cur_num) return (0);
	    i_ptr->flags2 |= (TR2_FREE_ACT | TR2_RES_LITE | TR2_SUST_CON);
	    i_ptr->flags3 |= (TR3_LITE | TR3_ACTIVATE);
	    i_ptr->toac = 10;
	    i_ptr->cost = 30000L;
	    return (ART_CAMMITHRIM);
	}
    }

    if (i_ptr->sval == SV_SET_OF_GAUNTLETS) {

	switch (randint(6)) {

	  case 1:
	    if (v_list[ART_PAURHACH].cur_num) return (0);
	    if (randint(4) > 1) return 0;
	    i_ptr->flags2 |= (TR2_RES_FIRE);
	    i_ptr->flags3 |= (TR3_ACTIVATE);
	    i_ptr->toac = 15;
	    i_ptr->cost = 15000L;
	    return (ART_PAURHACH);

	  case 2:
	    if (v_list[ART_PAURNIMMEN].cur_num) return (0);
	    if (randint(4) > 1) return 0;
	    i_ptr->flags2 |= (TR2_RES_COLD);
	    i_ptr->flags3 |= (TR3_ACTIVATE);
	    i_ptr->toac = 15;
	    i_ptr->cost = 13000L;
	    return (ART_PAURNIMMEN);

	  case 3:
	    if (v_list[ART_PAURAEGEN].cur_num) return (0);
	    if (randint(4) > 1) return 0;
	    i_ptr->flags2 |= (TR2_RES_ELEC);
	    i_ptr->flags3 |= (TR3_ACTIVATE);
	    i_ptr->toac = 15;
	    i_ptr->cost = 11000L;
	    return (ART_PAURAEGEN);

	  case 4:
	    if (v_list[ART_PAURNEN].cur_num) return (0);
	    if (randint(4) > 1) return 0;
	    i_ptr->flags2 |= (TR2_RES_ACID);
	    i_ptr->flags3 |= (TR3_ACTIVATE);
	    i_ptr->toac = 15;
	    i_ptr->cost = 12000L;
	    return (ART_PAURNEN);

	  default: /* case 5: case 6: */
	    if (v_list[ART_CAMLOST].cur_num) return (0);
	    if (randint(4) > 1) return 0;
	    i_ptr->flags1 |= (TR1_STR | TR1_DEX);
	    i_ptr->flags3 |= (TR3_SHOW_MODS | TR3_AGGRAVATE | TR3_CURSED);
	    i_ptr->pval = -5;	/* -5 STR/DEX */
	    i_ptr->tohit = -11;
	    i_ptr->todam = -12;
	    i_ptr->toac = 0;
	    i_ptr->cost = 0L;
	    return (ART_CAMLOST);
	}
    }

    if (i_ptr->sval == SV_SET_OF_CESTI) {
	if (v_list[ART_FINGOLFIN].cur_num) return (0);
	if (randint(15) > 1) return 0;
	i_ptr->flags1 |= (TR1_DEX);
	i_ptr->flags2 |= (TR2_RES_ACID | TR2_FREE_ACT);
	i_ptr->flags3 |= (TR3_SHOW_MODS | TR3_ACTIVATE);
	i_ptr->pval = 4;	/* +4 DEX */
	i_ptr->tohit = 10;
	i_ptr->todam = 10;
	i_ptr->toac = 20;
	i_ptr->cost = 110000L;
	return (ART_FINGOLFIN);
    }

    return 0;
}


/*
 * Make artifact boots
 */
static int make_artifact_boots(inven_type *i_ptr)
{
    if (i_ptr->sval == SV_PAIR_OF_HARD_LEATHER_BOOTS) {
	if (v_list[ART_FEANOR].cur_num) return (0);
	if (randint(5) > 1) return 0;
	i_ptr->flags1 |= (TR1_SPEED | TR1_STEALTH);
	i_ptr->flags2 |= (TR2_RES_ACID | TR2_RES_NEXUS);
	i_ptr->flags3 |= (TR3_ACTIVATE);
	i_ptr->pval = 1;
	i_ptr->toac = 20;
	i_ptr->cost = 130000L;
	return (ART_FEANOR);
    }

    if (i_ptr->sval == SV_PAIR_OF_SOFT_LEATHER_BOOTS) {
	if (v_list[ART_DAL].cur_num) return (0);
	i_ptr->flags1 |= (TR1_DEX);
	i_ptr->flags2 |= (TR2_SUST_CON | TR2_FREE_ACT |
			  TR2_RES_NETHER | TR2_RES_CHAOS | TR2_RES_ACID);
	i_ptr->flags3 |= (TR3_ACTIVATE);
	i_ptr->pval = 5;  /* +5 DEX */
	i_ptr->toac = 15;
	i_ptr->cost = 40000L;
	return (ART_DAL);
    }

    if (i_ptr->sval == SV_PAIR_OF_METAL_SHOD_BOOTS) {
	if (v_list[ART_THROR].cur_num) return (0);
	i_ptr->flags1 |= (TR1_CON | TR1_STR);
	i_ptr->flags2 |= (TR2_RES_ACID);
	i_ptr->pval = 3;	/* +3 STR/CON */
	i_ptr->toac = 20;
	i_ptr->cost = 12000L;
	return (ART_THROR);
    }

    return (0);
}



/*
 * Help pick and create a "special" object
 * This function is a total hack.
 */
static int make_artifact_special_aux(inven_type *i_ptr)
{
    int ob = object_level;


    /* Analyze it */
    switch (randint(12)) {

      case 1:
	if (v_list[ART_GALADRIEL].cur_num) return (0);
	if ((k_list[OBJ_GALADRIEL].level - 40) > ob) return (0);
	if ((k_list[OBJ_GALADRIEL].level > ob) && (randint(30) > 1)) return (0);
	invcopy(i_ptr, OBJ_GALADRIEL);
	return (ART_GALADRIEL);
	break;

      case 2:
	if (v_list[ART_ELENDIL].cur_num) return (0);
	if (randint(8) > 1) return (0);
	if ((k_list[OBJ_ELENDIL].level - 40) > ob) return (0);
	if ((k_list[OBJ_ELENDIL].level > ob) && (randint(30) > 1)) return (0);
	invcopy(i_ptr, OBJ_ELENDIL);
	return (ART_ELENDIL);

      case 3:
	if (v_list[ART_THRAIN].cur_num) return (0);
	if (randint(18) > 1) return (0);
	if ((k_list[OBJ_THRAIN].level - 40) > ob) return (0);
	if ((k_list[OBJ_THRAIN].level > ob) && (randint(60) > 1)) return (0);
	invcopy(i_ptr, OBJ_THRAIN);
	return (ART_THRAIN);


      case 4:
	if (v_list[ART_CARLAMMAS].cur_num) return (0);
	if (randint(6) > 1) return (0);
	if ((k_list[OBJ_CARLAMMAS].level - 40) > ob) return (0);
	if ((k_list[OBJ_CARLAMMAS].level > ob) && (randint(35) > 1)) return (0);
	invcopy(i_ptr, OBJ_CARLAMMAS);
	return (ART_CARLAMMAS);

      case 5:
	if (v_list[ART_INGWE].cur_num) return (0);
	if (randint(10) > 1) return (0);
	if ((k_list[OBJ_INGWE].level - 40) > ob) return (0);
	if ((k_list[OBJ_INGWE].level > ob) && (randint(50) > 1)) return (0);
	invcopy(i_ptr, OBJ_INGWE);
	return (ART_INGWE);

      case 6:
	if (v_list[ART_DWARVES].cur_num) return (0);
	if (randint(25) > 1) return (0);
	if ((k_list[OBJ_DWARVES].level - 40) > ob) return (0);
	if ((k_list[OBJ_DWARVES].level > ob) && (randint(60) > 1)) return (0);
	invcopy(i_ptr, OBJ_DWARVES);
	return (ART_DWARVES);


      case 7:
	if (v_list[ART_BARAHIR].cur_num) return (0);
	if (randint(20) > 1) return (0);
	if ((k_list[OBJ_BARAHIR].level - 40) > ob) return (0);
	if ((k_list[OBJ_BARAHIR].level > ob) && (randint(50) > 1)) return (0);
	invcopy(i_ptr, OBJ_BARAHIR);
	return (ART_BARAHIR);

      case 8:
	if (v_list[ART_TULKAS].cur_num) return (0);
	if (randint(25) > 1) return (0);
	if ((k_list[OBJ_TULKAS].level - 40) > ob) return (0);
	if ((k_list[OBJ_TULKAS].level > ob) && (randint(65) > 1)) return (0);
	invcopy(i_ptr, OBJ_TULKAS);
	return (ART_TULKAS);

      case 9:
	if (v_list[ART_NARYA].cur_num) return (0);
	if (randint(30) > 1) return (0);
	if ((k_list[OBJ_NARYA].level - 40) > ob) return (0);
	if ((k_list[OBJ_NARYA].level > ob) && (randint(50) > 1)) return (0);
	invcopy(i_ptr, OBJ_NARYA);
	return (ART_NARYA);

      case 10:
	if (v_list[ART_NENYA].cur_num) return (0);
	if (randint(35) > 1) return (0);
	if ((k_list[OBJ_NENYA].level - 40) > ob) return (0);
	if ((k_list[OBJ_NENYA].level > ob) && (randint(60) > 1)) return (0);
	invcopy(i_ptr, OBJ_NENYA);
	return (ART_NENYA);

      case 11:
	if (v_list[ART_VILYA].cur_num) return (0);
	if (randint(40) > 1) return (0);
	if ((k_list[OBJ_VILYA].level - 40) > ob) return (0);
	if ((k_list[OBJ_VILYA].level > ob) && (randint(70) > 1)) return (0);
	invcopy(i_ptr, OBJ_VILYA);
	return (ART_VILYA);

      case 12:
	if (v_list[ART_POWER].cur_num) return (0);
	if (randint(60) > 1) return (0);
	if ((k_list[OBJ_POWER].level - 40) > ob) return (0);
	if ((k_list[OBJ_POWER].level > ob) && (randint(100) > 1)) return (0);
	invcopy(i_ptr, OBJ_POWER);
	return (ART_POWER);
    }

    return (0);
}



/*
 * Attempt to create one of the "Special Objects"
 * XXX Mega-Hack -- completely ignore the properties
 * of the given "object" -- just dump over them.
 */
static bool make_artifact_special(inven_type *i_ptr)
{
    int			what;
    int			done;


    /* Try to allocate a special object */
    for (what = 0, done = 0; !what; ++done) {

	/* Abort after a while */
	if (done > 21) return FALSE;

	/* Pick a special object */
	what = make_artifact_special_aux(i_ptr);
    }


    /* Mark that artifact as Made */
    v_list[what].cur_num = 1;

    /* Save the Artifact "Name" */  
    i_ptr->name1 = what;

    /* Artifacts cannot be destroyed */
    i_ptr->flags3 |= (TR3_IGNORE_FIRE | TR3_IGNORE_COLD |
		      TR3_IGNORE_ELEC | TR3_IGNORE_ACID);

    /* Set the good item flag */
    good_item_flag = TRUE;

    /* Hack -- Describe */
    if (wizard || peek) {
	char buf[256];
	objdes_store(buf, i_ptr, TRUE);
	msg_print(buf);
    }

    /* Success */
    return (TRUE);    
}


/*
 * Attempt to change an object into an artifact
 */
bool make_artifact(inven_type *i_ptr)
{
    int what = 0;

    /* Hack -- refuse "plural objects" */
    if (i_ptr->number != 1) return (FALSE);
    
    /* Attempt to Make an artifact of the appropriate type */
    switch (i_ptr->tval) {

	case TV_BOW:        what = make_artifact_bow(i_ptr); break;

	case TV_SWORD:      what = make_artifact_sword(i_ptr); break;
	case TV_POLEARM:    what = make_artifact_polearm(i_ptr); break;
	case TV_HAFTED:     what = make_artifact_hafted(i_ptr); break;

	case TV_DRAG_ARMOR: what = make_artifact_drag_armor(i_ptr); break;
	case TV_HARD_ARMOR: what = make_artifact_hard_armor(i_ptr); break;
	case TV_SOFT_ARMOR: what = make_artifact_soft_armor(i_ptr); break;

	case TV_SHIELD:     what = make_artifact_shield(i_ptr); break;
	case TV_HELM:       what = make_artifact_helm(i_ptr); break;
	case TV_BOOTS:      what = make_artifact_boots(i_ptr); break;
	case TV_CLOAK:      what = make_artifact_cloak(i_ptr); break;
	case TV_GLOVES:     what = make_artifact_gloves(i_ptr); break;
    }

    /* Nothing made */
    if (!what) return FALSE;

    /* XXX Note -- see "make_artifact_special()" as well */

    /* Mark that artifact as Made */
    v_list[what].cur_num = 1;

    /* Save the Artifact Index */  
    i_ptr->name1 = what;

    /* Artifacts cannot be destroyed */
    i_ptr->flags3 |= (TR3_IGNORE_FIRE | TR3_IGNORE_COLD |
		      TR3_IGNORE_ELEC | TR3_IGNORE_ACID);

    /* Set the good item flag */
    good_item_flag = TRUE;

    /* Hack -- Describe */
    if (wizard || peek) {
	char buf[256];
	objdes_store(buf, i_ptr, TRUE);
	msg_print(buf);
    }

    /* An artifact was made */    
    return TRUE;
}


/*
 * Gives one of the "new" resistances to an item
 * Some of the resistances are VERY handy!
 *
 * XXX Consider also the new "IGNORE" flags...
 */		   
static void give_1_hi_resist(inven_type *i_ptr)
{
    switch (randint(10)) {
	case 1: i_ptr->flags2 |= TR2_RES_CONF; break;
	case 2: i_ptr->flags2 |= TR2_RES_SOUND; break;
	case 3: i_ptr->flags2 |= TR2_RES_LITE; break;
	case 4: i_ptr->flags2 |= TR2_RES_DARK; break;
	case 5: i_ptr->flags2 |= TR2_RES_CHAOS; break;
	case 6: i_ptr->flags2 |= TR2_RES_DISEN; break;
	case 7: i_ptr->flags2 |= TR2_RES_SHARDS; break;
	case 8: i_ptr->flags2 |= TR2_RES_NEXUS; break;
	case 9: i_ptr->flags2 |= TR2_RES_BLIND; break;
	case 10: i_ptr->flags2 |= TR2_RES_NETHER; break;
    }
}


/*
 * Charge a wand  XXX Redo this with constants!
 */
static void charge_wand(inven_type *i_ptr)
{
    switch (i_ptr->sval) {
      case 0:
	i_ptr->pval = randint(10) + 6;
	break;
      case 1:
	i_ptr->pval = randint(8) + 6;
	break;
      case 2:
	i_ptr->pval = randint(5) + 6;
	break;
      case 3:
	i_ptr->pval = randint(8) + 6;
	break;
      case 4:
	i_ptr->pval = randint(4) + 3;
	break;
      case 5:
	i_ptr->pval = randint(8) + 6;
	break;
      case 6:
	i_ptr->pval = randint(20) + 12;
	break;
      case 7:
	i_ptr->pval = randint(20) + 12;
	break;
      case 8:
	i_ptr->pval = randint(10) + 6;
	break;
      case 9:
	i_ptr->pval = randint(12) + 6;
	break;
      case 10:
	i_ptr->pval = randint(10) + 12;
	break;
      case 11:
	i_ptr->pval = randint(3) + 3;
	break;
      case 12:
	i_ptr->pval = randint(8) + 6;
	break;
      case 13:
	i_ptr->pval = randint(10) + 6;
	break;
      case 14:
	i_ptr->pval = randint(5) + 3;
	break;
      case 15:
	i_ptr->pval = randint(5) + 3;
	break;
      case 16:
	i_ptr->pval = randint(5) + 6;
	break;
      case 17:
	i_ptr->pval = randint(5) + 4;
	break;
      case 18:
	i_ptr->pval = randint(8) + 4;
	break;
      case 19:
	i_ptr->pval = randint(6) + 2;
	break;
      case 20:
	i_ptr->pval = randint(4) + 2;
	break;
      case 21:
	i_ptr->pval = randint(8) + 6;
	break;
      case 22:
	i_ptr->pval = randint(5) + 2;
	break;
      case 23:
	i_ptr->pval = randint(12) + 12;
	break;
      case 24:
	i_ptr->pval = randint(3) + 1;
	break;
      case 25:
	i_ptr->pval = randint(3) + 1;
	break;
      case 26:
	i_ptr->pval = randint(3) + 1;
	break;
      case 27:
	i_ptr->pval = randint(2) + 1;
	break;
      case 28:
	i_ptr->pval = randint(8) + 6;
	break;
      default:
	break;
    }
}


/*
 * Charge a staff  XXX Redo this with constants!
 */
static void charge_staff(inven_type *i_ptr)
{        
    switch (i_ptr->sval) {
      case 0:
	i_ptr->pval = randint(20) + 12;
	break;
      case 1:
	i_ptr->pval = randint(8) + 6;
	break;
      case 2:
	i_ptr->pval = randint(5) + 6;
	break;
      case 3:
	i_ptr->pval = randint(20) + 12;
	break;
      case 4:
	i_ptr->pval = randint(15) + 6;
	break;
      case 5:
	i_ptr->pval = randint(4) + 5;
	break;
      case 6:
	i_ptr->pval = randint(5) + 3;
	break;
      case 7:
	i_ptr->pval = randint(3) + 1;
	break;
      case 8:
	i_ptr->pval = randint(3) + 1;
	break;
      case 9:
	i_ptr->pval = randint(5) + 6;
	break;
      case 10:
	i_ptr->pval = randint(10) + 12;
	break;
      case 11:
	i_ptr->pval = randint(5) + 6;
	break;
      case 12:
	i_ptr->pval = randint(5) + 6;
	break;
      case 13:
	i_ptr->pval = randint(5) + 6;
	break;
      case 14:
	i_ptr->pval = randint(10) + 12;
	break;
      case 15:
	i_ptr->pval = randint(3) + 4;
	break;
      case 16:
	i_ptr->pval = randint(5) + 6;
	break;
      case 17:
	i_ptr->pval = randint(5) + 6;
	break;
      case 18:
	i_ptr->pval = randint(3) + 4;
	break;
      case 19:
	i_ptr->pval = randint(10) + 12;
	break;
      case 20:
	i_ptr->pval = randint(3) + 4;
	break;
      case 21:
	i_ptr->pval = randint(3) + 4;
	break;
      case 22:
	i_ptr->pval = randint(10) + 6;
	break;
      case 23:
	i_ptr->pval = randint(2) + 1;
	break;
      case 24:
	i_ptr->pval = randint(3) + 1;
	break;
      case 25:
	i_ptr->pval = randint(2) + 2;
	break;
      case 26:
	i_ptr->pval = randint(15) + 5;
	break;
      case 27:
	i_ptr->pval = randint(2) + 2;
	break;
      case 28:
	i_ptr->pval = randint(5) + 5;
	break;
      case 29:
	i_ptr->pval = randint(2) + 1;
	break;
      case 30:
	i_ptr->pval = randint(6) + 2;
	break;
      default:
	break;
    }
}



/*
 * Imbue an object with "magical" properties.
 *
 * This also includes Traps on Chests, and Charges on Wands/Staffs.
 *
 * Chance increases with each dungeon level
 *			
 * If "good" is TRUE, make a GOOD object.
 * If "great" is TRUE make a GREAT object
 *
 * Note that the "k_list" has been rebuilt to remove the old problems
 * with multiple "similar" objects.
 *
 * In addition, this function cleans up some floating bizarreness,
 * like the "fake-torches" in the dungeon, etc.  Likewise, the Chests
 * get trapped here.  Store-bought chests may be excellent.
 *
 * If "okay" is true, this routine will have a small chance of turning
 * the object into an artifact.  If "good" is true, the object is guaranteed
 * to be good.  If "great" is true, it is guaranteed to be "great".  Note
 * that even if "good" or "great" are false, there is still a chance for
 * good stuff to be created, even artifacts (if "okay" is TRUE).
 */
void apply_magic(inven_type *i_ptr, int level, bool okay, bool good, bool great)
{
    register int32u      chance, special, cursed;
    int32u               tmp, crown;

    chance = OBJ_BASE_MAGIC + level;
    if (chance > OBJ_BASE_MAX) chance = OBJ_BASE_MAX;
    special = chance / OBJ_DIV_SPECIAL;
    cursed = (10 * chance) / OBJ_DIV_CURSED;

    /* Depending on treasure type, it can have certain magical properties */
    switch (i_ptr->tval) {

      /* Dragon Scale Mail is NEVER cursed */
      case TV_DRAG_ARMOR:

	/* all DSM are enchanted, I guess -CFT */
	i_ptr->toac += m_bonus(0, 5, level) + randint(5);

	/* Perhaps an artifact */
	if (great || (magik(chance) && magik(special))) {

	    /* Even better... */
	    i_ptr->toac += randint(5);

	    /* Roll for artifact */
	    if ((great || randint(3) == 1) &&
		okay && make_artifact(i_ptr)) break;

	    /* XXX Add an extra resist? */
	}

	/* Hack -- adjust cost for "toac" */
	i_ptr->cost += ((int32) i_ptr->toac * 500L);

	rating += 30;
	if (wizard || peek) msg_print("Dragon Scale Mail");

	break;

      case TV_HARD_ARMOR:
      case TV_SOFT_ARMOR:
      case TV_SHIELD:

	if (good || magik(chance)) {

	    i_ptr->toac += randint(3) + m_bonus(0, 5, level);

	    /* Hack -- try for Robes of the Magi */
	    if ((i_ptr->tval == TV_SOFT_ARMOR) &&
		(i_ptr->sval == SV_ROBE) &&
		(great || randint(30) == 1) &&
		 magik(special)) {

		i_ptr->flags2 |= (TR2_RES_ELEC | TR2_RES_COLD | TR2_RES_ACID |
				  TR2_RES_FIRE | TR2_HOLD_LIFE |
				  TR2_SUST_STR | TR2_SUST_DEX | TR2_SUST_CON |
				  TR2_SUST_INT | TR2_SUST_WIS | TR2_SUST_CHR);
		i_ptr->flags3 |= (TR3_IGNORE_FIRE | TR3_IGNORE_COLD |
				  TR3_IGNORE_ELEC | TR3_IGNORE_ACID);
		i_ptr->toac += 10 + randint(5);
		i_ptr->cost = 10000L + (i_ptr->toac * 100);
		give_1_hi_resist(i_ptr);	/* JLS */
		i_ptr->name2 = EGO_MAGI;

		rating += 30;
		if (wizard || peek) msg_print("Robe of the Magi");

		break;
	    }

	    /* Try for artifacts */
	    if (great || magik(special)) {

		/* Roll for artifact */
		if ((great || (randint(3) == 1)) &&
		    okay && make_artifact(i_ptr)) return;

		switch (randint(9)) {

		  case 1:

		    i_ptr->flags2 |= (TR2_RES_ELEC | TR2_RES_COLD |
				      TR2_RES_ACID | TR2_RES_FIRE);
		    i_ptr->flags3 |= (TR3_IGNORE_ELEC | TR3_IGNORE_COLD |
				      TR3_IGNORE_ACID | TR3_IGNORE_FIRE);

		    if (randint(3) == 1) {
			give_1_hi_resist(i_ptr);	/* JLS */
			i_ptr->flags1 |= TR1_STEALTH;
			i_ptr->pval = randint(3);
			i_ptr->toac += 15;
			i_ptr->cost += 15000L;
			i_ptr->name2 = EGO_ELVENKIND;
			rating += 25;
			if (peek) msg_print("Elvenkind");
		    }
		    else {
			i_ptr->toac += 8;
			i_ptr->cost += 12500L;
			i_ptr->name2 = EGO_R;
			rating += 20;
			if (peek) msg_print("Resist");
		    }
		    break;

		  case 2:
		    i_ptr->flags2 |= (TR2_RES_ACID);
		    i_ptr->flags3 |= (TR3_IGNORE_ACID);
		    i_ptr->cost += 1000L;
		    i_ptr->name2 = EGO_RESIST_A;
		    rating += 15;
		    if (peek) msg_print("Resist Acid");
		    break;

		  case 3: case 4:
		    i_ptr->flags2 |= (TR2_RES_FIRE);
		    i_ptr->flags3 |= (TR3_IGNORE_FIRE);
		    i_ptr->cost += 600L;
		    i_ptr->name2 = EGO_RESIST_F;
		    rating += 17;
		    if (peek) msg_print("Resist Fire");
		    break;

		  case 5: case 6:
		    i_ptr->flags2 |= (TR2_RES_COLD);
		    i_ptr->flags3 |= (TR3_IGNORE_COLD);
		    i_ptr->cost += 600L;
		    i_ptr->name2 = EGO_RESIST_C;
		    rating += 16;
		    if (peek) msg_print("Resist Cold");
		    break;

		  case 7: case 8: case 9:
		    i_ptr->flags2 |= (TR2_RES_ELEC);
		    i_ptr->flags3 |= (TR3_IGNORE_ELEC);
		    i_ptr->cost += 500L;
		    i_ptr->name2 = EGO_RESIST_E;
		    rating += 15;
		    if (peek) msg_print("Resist Lightning");
		    break;
		}
	    }
	}

	else if (magik(cursed)) {
	    i_ptr->toac = -randint(3) - m_bonus(0, 10, level);
	    i_ptr->cost = 0L;
	    i_ptr->flags3 |= TR3_CURSED;
	}

	break;

      case TV_HAFTED:
      case TV_POLEARM:
      case TV_SWORD:

	/* Apply some magic */
	if (good || magik(chance)) {

	    i_ptr->tohit += randint(3) + m_bonus(0, 10, level);
	    i_ptr->todam += randint(3) + m_bonus(0, 10, level);

	    /* the 3*special/2 is needed because weapons are not as */
	    /* common as before change to treasure distribution, this */
	    /* helps keep same number of ego weapons same as before, */
	    /* see also missiles (it used to be "2*special") */
	    /* XXX In that case we may be in trouble... */

	    if (great || magik(3*special/2)) {

		/* Roll for artifacts (XXX guess at distribution) */
		if ((great || (randint(6) == 1)) &&
		    okay && make_artifact(i_ptr)) return;

		/* Hack -- Roll for whips of fire */
		if ((i_ptr->tval == TV_HAFTED) &&
		    (i_ptr->sval == SV_WHIP) &&
		    (randint(2)==1)) {

		    i_ptr->flags1 |= (TR1_BRAND_FIRE);
		    /* i_ptr->flags2 |= (TR2_RES_FIRE); */
		    i_ptr->flags3 |= (TR3_IGNORE_FIRE);

		    /* Better stats */
		    i_ptr->tohit += 5;
		    i_ptr->todam += 5;

		    /* XXX Added a basic cost increase */
		    i_ptr->cost += 2000L;

		    /* this should allow some WICKED whips -CFT */
		    while (randint(5 * (int)i_ptr->damage[0]) == 1) {
			i_ptr->damage[0]++;
			i_ptr->cost += 2500;
			i_ptr->cost *= 2;
		    }

		    i_ptr->name2 = EGO_FIRE;

		    rating += 20;
		    if (peek) msg_print("Whip of Fire");

		    break;
		}


		switch (randint(30)) {

		  case 1:
		    i_ptr->flags1 |= (TR1_SLAY_DEMON | TR1_WIS |
				      TR1_SLAY_UNDEAD | TR1_SLAY_EVIL);
		    i_ptr->flags3 |= (TR3_BLESSED | TR3_SEE_INVIS);
		    i_ptr->pval = randint(4);  /* Wisdom bonus */
		    i_ptr->tohit += 5;
		    i_ptr->todam += 5;
		    i_ptr->toac += randint(4);

		    /* Obsolete-Hack -- Pick "Sustain" based on "Pval" */
		    switch (i_ptr->pval) {
			case 1: i_ptr->flags2 |= (TR2_SUST_STR); break;
			case 2: i_ptr->flags2 |= (TR2_SUST_INT); break;
			case 3: i_ptr->flags2 |= (TR2_SUST_WIS); break;
			case 4: i_ptr->flags2 |= (TR2_SUST_DEX); break;
		    }

		    i_ptr->cost += i_ptr->pval * 500;
		    i_ptr->cost += 10000L;
		    i_ptr->cost *= 2;
		    i_ptr->name2 = EGO_HA;
		    rating += 30;
		    if (peek) msg_print("Holy Avenger");
		    break;

		  case 2:
		    i_ptr->flags1 |= (TR1_STEALTH);
		    i_ptr->flags2 |= (TR2_FREE_ACT |
				      TR2_RES_FIRE | TR2_RES_COLD |
				      TR2_RES_ELEC | TR2_RES_ACID);
		    i_ptr->flags3 |= (TR3_FEATHER | TR3_REGEN | TR3_SEE_INVIS |
				      TR3_IGNORE_FIRE | TR3_IGNORE_COLD |
				      TR3_IGNORE_ELEC | TR3_IGNORE_ACID);
		    i_ptr->tohit += 3;
		    i_ptr->todam += 3;
		    i_ptr->toac += 5 + randint(5);
		    i_ptr->pval = randint(3);	/* +X Stealth */
		    i_ptr->cost += i_ptr->pval * 500;
		    i_ptr->cost += 7500L;
		    i_ptr->cost *= 2;
		    i_ptr->name2 = EGO_DF;
		    rating += 23;
		    if (peek) msg_print("Defender");
		    break;

		  case 3: case 4:
		    i_ptr->flags1 |= (TR1_BRAND_FIRE);
		    i_ptr->flags2 |= (TR2_RES_FIRE);
		    i_ptr->flags2 |= (TR3_IGNORE_FIRE);
		    i_ptr->tohit += 2;
		    i_ptr->todam += 3;
		    i_ptr->cost += 3000L;
		    i_ptr->name2 = EGO_FT;
		    rating += 20;
		    if (peek) msg_print("Flame");
		    break;

		  case 5: case 6:
		    i_ptr->flags1 |= (TR1_BRAND_COLD);
		    i_ptr->flags2 |= (TR2_RES_COLD);
		    i_ptr->flags3 |= (TR3_IGNORE_COLD);
		    i_ptr->tohit += 2;
		    i_ptr->todam += 2;
		    i_ptr->cost += 2200L;
		    i_ptr->name2 = EGO_FB;
		    rating += 20;
		    if (peek) msg_print("Frost");
		    break;

		  case 7: case 8:
		    i_ptr->flags1 |= TR1_SLAY_ANIMAL;
		    i_ptr->tohit += 3;
		    i_ptr->todam += 3;
		    i_ptr->cost += 2000L;
		    i_ptr->name2 = EGO_SLAY_A;
		    rating += 15;
		    if (peek) msg_print("Slay Animal");
		    break;

		  case 9: case 10:
		    i_ptr->flags1 |= TR1_SLAY_DRAGON;
		    i_ptr->tohit += 3;
		    i_ptr->todam += 3;
		    i_ptr->cost += 4000L;
		    i_ptr->name2 = EGO_SLAY_D;
		    rating += 18;
		    if (peek) msg_print("Slay Dragon");
		    break;

		  case 11: case 12:
		    i_ptr->flags1 |= TR1_SLAY_EVIL;
		    i_ptr->tohit += 3;
		    i_ptr->todam += 3;
		    i_ptr->cost += 4000L;
		    i_ptr->name2 = EGO_SLAY_E;

		    /* One in three is also a blessed wisdom booster */
		    if (randint(3) == 1) {
			i_ptr->flags1 |= (TR1_WIS);
			i_ptr->flags3 |= (TR3_BLESSED);
			i_ptr->pval = m_bonus(0, 3, level);
			i_ptr->cost += (200L * i_ptr->pval);
		    }

		    rating += 18;
		    if (peek) msg_print("Slay Evil");

		    break;

		  case 13: case 14:
		    i_ptr->flags1 |= (TR1_SLAY_UNDEAD);
		    i_ptr->flags3 |= (TR3_SEE_INVIS);
		    i_ptr->tohit += 2;
		    i_ptr->todam += 2;
		    i_ptr->cost += 3000L;
		    i_ptr->name2 = EGO_SLAY_U;

		    /* One in three is also a Life Holder */
		    if (randint(3) == 1) {
			i_ptr->flags2 |= (TR2_HOLD_LIFE);
			i_ptr->cost += 1000L;
		    }

		    rating += 18;
		    if (peek) msg_print("Slay Undead");

		    break;

		  case 15: case 16: case 17:
		    i_ptr->flags1 |= TR1_SLAY_ORC;
		    i_ptr->tohit += 2;
		    i_ptr->todam += 2;
		    i_ptr->cost += 1200L;
		    i_ptr->name2 = EGO_SLAY_O;
		    rating += 13;
		    if (peek) msg_print("Slay Orc");
		    break;

		  case 18: case 19: case 20:
		    i_ptr->flags1 |= TR1_SLAY_TROLL;
		    i_ptr->tohit += 2;
		    i_ptr->todam += 2;
		    i_ptr->cost += 1200L;
		    i_ptr->name2 = EGO_SLAY_T;
		    rating += 13;
		    if (peek) msg_print("Slay Troll");
		    break;

		  case 21: case 22: case 23:
		    i_ptr->flags1 |= TR1_SLAY_GIANT;
		    i_ptr->tohit += 2;
		    i_ptr->todam += 2;
		    i_ptr->cost += 1200L;
		    i_ptr->name2 = EGO_SLAY_G;
		    rating += 14;
		    if (peek) msg_print("Slay Giant");
		    break;

		  case 24: case 25: case 26:
		    i_ptr->flags1 |= TR1_SLAY_DEMON;
		    i_ptr->tohit += 2;
		    i_ptr->todam += 2;
		    i_ptr->cost += 1200L;
		    i_ptr->name2 = EGO_SLAY_DEMON;
		    rating += 16;
		    if (peek) msg_print("Slay Demon");
		    break;

		  case 27:
		    i_ptr->flags1 |= (TR1_SLAY_ORC |
				      TR1_DEX | TR1_CON | TR1_STR);
		    i_ptr->flags2 |= (TR2_FREE_ACT);
		    i_ptr->flags3 |= (TR3_SEE_INVIS);
		    i_ptr->tohit += randint(5) + 3;
		    i_ptr->todam += randint(5) + 3;
		    i_ptr->pval = 1;
		    i_ptr->cost += 10000L;
		    i_ptr->cost *= 2;
		    i_ptr->name2 = EGO_WEST;
		    rating += 20;
		    if (peek) msg_print("Westernesse");
		    break;

		  /* Anything can be blessed */
		  case 28: case 29:
		    i_ptr->flags3 |= TR3_BLESSED;
		    i_ptr->flags1 |= TR1_WIS;
		    i_ptr->tohit += 3;
		    i_ptr->todam += 3;
		    i_ptr->pval = randint(3);
		    i_ptr->cost += i_ptr->pval * 1000;
		    i_ptr->cost += 3000L;
		    i_ptr->name2 = EGO_BLESS_BLADE;
		    rating += 20;
		    if (peek) msg_print("Blessed");
		    break;

		  /* Extra Attacks */
		  case 30:
		    i_ptr->tohit += randint(5);
		    i_ptr->todam += randint(3);
		    i_ptr->flags1 |= (TR1_ATTACK_SPD);
		    if (i_ptr->weight <= 80) {
			i_ptr->pval = randint(3);
		    }
		    else if (i_ptr->weight <= 130) {
			i_ptr->pval = randint(2);
		    }
		    else {
			i_ptr->pval = 1;
		    }
		    i_ptr->cost += (i_ptr->pval * 2000);
		    i_ptr->cost *= 2;
		    i_ptr->name2 = EGO_ATTACKS;
		    rating += 20;
		    if (peek) msg_print("Weapon of Extra Attacks");
		    break;
		}
	    }
	}

	else if (magik(cursed)) {
	    i_ptr->tohit = (-randint(3) - m_bonus(1, 20, level));
	    i_ptr->todam = (-randint(3) - m_bonus(1, 20, level));
	    i_ptr->flags3 |= TR3_CURSED;
	    i_ptr->cost = 0L;

	    /* Weapon of Morgul */
	    if (level > (20 + randint(15)) && randint(10) == 1) {
		i_ptr->flags3 |= (TR3_HEAVY_CURSE | TR3_AGGRAVATE | TR3_SEE_INVIS);
		i_ptr->tohit -= 15;
		i_ptr->todam -= 15;
		i_ptr->toac = -10;
		i_ptr->weight += 100;
		i_ptr->name2 = EGO_MORGUL;
	    }
	}
	break;

      case TV_BOW:

	/* Apply some magic */
	if (good || magik(chance)) {

	    i_ptr->tohit = randint(3) + m_bonus(0, 10, level);
	    i_ptr->todam = randint(3) + m_bonus(0, 10, level);

	    /* Make an artifact */
	    if ((randint(15) == 1) &&
		(great || magik(special)) &&
		okay && make_artifact(i_ptr)) return;

	    switch (randint(20)) {

	      case 1:
		i_ptr->flags3 |= TR3_XTRA_MIGHT;
		i_ptr->tohit += 5;
		i_ptr->todam += 10;
		i_ptr->name2 = EGO_MIGHT;
		rating += 20;
		if (peek) msg_print("Bow of Extra Might");
		break;

	      case 2:
		i_ptr->flags3 |= TR3_XTRA_SHOTS;
		i_ptr->tohit += 10;
		i_ptr->todam += 3;
		i_ptr->name2 = EGO_ACCURACY;
		rating += 20;
		if (peek) msg_print("Bow of Extra Shots");
		break;

	      case 3: case 4: case 5: case 6:
		i_ptr->tohit += 5;
		i_ptr->todam += 12;
		i_ptr->name2 = EGO_MIGHT;
		rating += 11;
		if (peek) msg_print("Bow of Might");
		break;

	      case 7: case 8: case 9: case 10:
		i_ptr->tohit += 12;
		i_ptr->todam += 5;
		i_ptr->name2 = EGO_ACCURACY;
		rating += 11;
		if (peek) msg_print("Accuracy");
		break;

	      case 11: case 12: case 13: case 14:
		i_ptr->tohit += m_bonus(0, 5, level);
		i_ptr->todam += m_bonus(0, 5, level);
		break;

	      default: /* case 15 to 20: */
		if (randint(2) == 1) {
		    i_ptr->tohit += m_bonus(0, 5, level);
		}
		else {
		    i_ptr->todam += m_bonus(0, 5, level);
		}
		break;
	    }
	}

	else if (magik(cursed)) {
	    i_ptr->tohit = (-m_bonus(5, 30, level));
	    i_ptr->todam = (-m_bonus(5, 20, level));
	    i_ptr->flags3 |= TR3_CURSED;
	    i_ptr->cost = 0L;
	}
	break;

      case TV_DIGGING:

	/* Apply some magic */
	if (good || magik(chance)) {

	    /* Extract a digging bonus */
	    tmp = m_bonus(0, 5, level);

	    /* Add in the digging bonuses */
	    i_ptr->pval += tmp;

	    /* Hack -- charge for the digging bonuses */
	    i_ptr->cost += tmp * 50;

	    /* Hack -- "shovels of fire" */
	    if (great || magik(special)) {
		i_ptr->flags1 |= (TR1_BRAND_FIRE);
		/* i_ptr->flags2 |= (TR2_RES_FIRE); */
		i_ptr->flags3 |= (TR3_IGNORE_FIRE);
		i_ptr->tohit += 5;
		i_ptr->todam += 5;
		i_ptr->cost += 2000L;
		i_ptr->name2 = EGO_FIRE;
		rating += 15;
		if (peek) msg_print("Digger of Fire");
	    }
	}

	else if (magik(cursed)) {
	    i_ptr->pval = (-m_bonus(1, 15, level));
	    i_ptr->flags3 |= TR3_CURSED;
	    i_ptr->cost = 0L;
	}
	break;

      case TV_GLOVES:

	/* Apply some magic */
	if (good || magik(chance)) {

	    i_ptr->toac = randint(3) + m_bonus(0, 10, level);

	    if (great || magik(special)) {

		if ((randint(1) == 1) &&
		    okay && make_artifact(i_ptr)) return;

		switch (randint(10)) {

		  case 1:
		  case 2:
		  case 3:
		    i_ptr->flags2 |= (TR2_FREE_ACT);
		    i_ptr->cost += 1000L;
		    i_ptr->name2 = EGO_FREE_ACTION;
		    rating += 11;
		    if (peek) msg_print("Gloves of Free action");
		    break;

		  case 4:
		  case 5:
		  case 6:
		    i_ptr->flags3 |= (TR3_SHOW_MODS);
		    i_ptr->tohit += 1 + randint(4);
		    i_ptr->todam += 1 + randint(4);
		    i_ptr->cost += (i_ptr->tohit + i_ptr->todam) * 250;
		    i_ptr->name2 = EGO_SLAYING;
		    rating += 17;
		    if (peek) msg_print("Gloves of Slaying");
		    break;

		  case 7:
		  case 8:
		  case 9:
		    i_ptr->flags1 |= TR1_DEX;
		    i_ptr->pval = 2 + randint(2);
		    i_ptr->cost += (i_ptr->pval) * 400;
		    i_ptr->name2 = EGO_AGILITY;
		    rating += 14;
		    if (peek) msg_print("Gloves of Agility");
		    break;

		  case 10:
		    i_ptr->flags1 |= TR1_STR;
		    i_ptr->flags3 |= (TR3_SHOW_MODS | TR3_HIDE_TYPE);
		    i_ptr->pval = 1 + randint(4);
		    i_ptr->tohit += 1 + randint(4);
		    i_ptr->todam += 1 + randint(4);
		    i_ptr->cost += ((i_ptr->tohit + i_ptr->todam +
				     i_ptr->pval) * 300);
		    i_ptr->name2 = EGO_POWER;
		    rating += 22;
		    if (peek) msg_print("Gloves of Power");
		    break;
		}
	    }
	}

	else if (magik(cursed)) {
	    if (magik(special)) {
		if (randint(2) == 1) {
		    i_ptr->flags1 |= TR1_DEX;
		    i_ptr->name2 = EGO_CLUMSINESS;
		}
		else {
		    i_ptr->flags1 |= TR1_STR;
		    i_ptr->name2 = EGO_WEAKNESS;
		}
		i_ptr->pval = 0 - (randint(3) + m_bonus(0, 10, level));
	    }
	    i_ptr->toac = 0 - (m_bonus(1, 20, level));
	    i_ptr->flags3 |= TR3_CURSED;
	    i_ptr->cost = 0L;
	}
	break;

      case TV_BOOTS:

	if (good || magik(chance)) {

	    i_ptr->toac = randint(3) + m_bonus(1, 10, level);

	    if (great || magik(special)) {

		if ((randint(24) == 1) &&
		    okay && make_artifact(i_ptr)) return;

		tmp = randint(12);

		if (tmp <= 1) {
		    i_ptr->flags1 |= TR1_SPEED;
		    i_ptr->pval = 1;
		    i_ptr->cost += 300000L;
		    i_ptr->name2 = EGO_SPEED;

		    if (randint(8888) == 1) {
			i_ptr->pval++;
			i_ptr->cost *= 9;
			rating += 20;
			if (wizard || peek) msg_print("Boots of Extra Speed");
		    }

		    rating += 30;
		    if (wizard || peek) msg_print("Boots of Speed");
		}

		else if (tmp <= 3) {
		    i_ptr->flags2 |= (TR2_FREE_ACT);
		    i_ptr->cost += 500;
		    i_ptr->cost *= 2;
		    i_ptr->name2 = EGO_FREE_ACTION;
		    rating += 15;
		}

		else if (tmp <= 8) {
		    i_ptr->flags3 |= TR3_FEATHER;
		    i_ptr->cost += 250;
		    i_ptr->name2 = EGO_SLOW_DESCENT;
		    rating += 7;
		}

		/* Hack -- Metal shod boots are "less magical" */
		else if (i_ptr->sval != SV_PAIR_OF_METAL_SHOD_BOOTS) {
		    i_ptr->flags1 |= TR1_STEALTH;
		    i_ptr->pval = randint(3);
		    i_ptr->cost += 500;
		    i_ptr->name2 = EGO_STEALTH;
		    rating += 16;
		}
	    }
	}

	else if (magik(cursed)) {
	    tmp = randint(3);
	    if (tmp == 1) {
		i_ptr->flags1 |= TR1_SPEED;
		i_ptr->pval = -1;
		i_ptr->name2 = EGO_SLOWNESS;
	    }
	    else if (tmp == 2) {
		i_ptr->flags3 |= TR3_AGGRAVATE;
		i_ptr->name2 = EGO_NOISE;
	    }
	    else {
		i_ptr->weight = i_ptr->weight * 5;
		i_ptr->name2 = EGO_GREAT_MASS;
	    }
	    i_ptr->cost = 0L;
	    i_ptr->toac = (-m_bonus(2, 20, level));
	    i_ptr->flags3 |= TR3_CURSED;
	}
	break;

      case TV_HELM:		   /* Helms */

	crown = FALSE;

	/* Hack -- crowns are "more magical" */
	if ((i_ptr->sval == SV_IRON_CROWN) ||
	    (i_ptr->sval == SV_GOLDEN_CROWN) ||
	    (i_ptr->sval == SV_JEWELED_CROWN)) {

	    chance += i_ptr->cost / 100L;
	    special *= 2;
	    crown = TRUE;
	}

	if (good || magik(chance)) {

	    i_ptr->toac = randint(3) + m_bonus(0, 10, level);

	    if (great || magik(special)) {

		if ((great || (randint(8) == 1)) &&
		    okay && make_artifact(i_ptr)) return;

		if (!crown) {

		    tmp = randint(14);

		    if (tmp < 3) {
			i_ptr->flags1 |= TR1_INT;
			i_ptr->flags2 |= TR2_SUST_INT;
			i_ptr->pval = randint(2);
			i_ptr->cost += i_ptr->pval * 500;
			i_ptr->name2 = EGO_INTELLIGENCE;
			rating += 13;
			if (peek) msg_print("Intelligence");
		    }
		    else if (tmp < 6) {
			i_ptr->pval = randint(2);
			i_ptr->flags1 |= TR1_WIS;
			i_ptr->flags2 |= TR2_SUST_WIS;
			i_ptr->cost += i_ptr->pval * 500;
			i_ptr->name2 = EGO_WISDOM;
			rating += 13;
			if (peek) msg_print("Wisdom");
		    }
		    else if (tmp < 10) {
			i_ptr->flags1 |= TR1_INFRA;
			i_ptr->pval = 1 + randint(4);
			i_ptr->cost += i_ptr->pval * 250;
			i_ptr->name2 = EGO_INFRAVISION;
			rating += 11;
			if (peek) msg_print("Infravision");
		    }
		    else if (tmp < 12) {
			i_ptr->flags2 |= (TR2_RES_LITE);
			i_ptr->flags3 |= (TR3_LITE);
			i_ptr->cost += 500;
			i_ptr->name2 = EGO_LITE;
			rating += 6;
			if (peek) msg_print("Light");
		    }
		    else if (tmp < 14) {
			i_ptr->flags2 |= TR2_RES_BLIND;
			i_ptr->flags3 |= TR3_SEE_INVIS;
			i_ptr->cost += 1000;
			i_ptr->name2 = EGO_SEEING;
			rating += 8;
			if (peek) msg_print("Helm of Seeing");
		    }
		    else {
			i_ptr->flags3 |= TR3_TELEPATHY;
			i_ptr->cost += 50000L;
			i_ptr->name2 = EGO_TELEPATHY;
			rating += 20;
			if (peek) msg_print("Telepathy");
		    }
		}

		else {

		    switch (randint(6)) {

		      case 1:
			i_ptr->flags1 |= (TR1_STR | TR1_DEX | TR1_CON);
			i_ptr->flags2 |= (TR2_FREE_ACT | TR2_SUST_STR |
					  TR2_SUST_DEX | TR2_SUST_CON);
			i_ptr->pval = randint(3);
			i_ptr->cost += 1000 + i_ptr->pval * 500;
			i_ptr->name2 = EGO_MIGHT;
			rating += 19;
			if (peek) msg_print("Crown of Might");
			break;

		      case 2:
			i_ptr->flags1 |= (TR1_CHR | TR1_WIS);
			i_ptr->flags2 |= (TR2_SUST_CHR | TR2_SUST_WIS);
			i_ptr->pval = randint(3);
			i_ptr->cost += 1000 + i_ptr->pval * 500;
			i_ptr->name2 = EGO_LORDLINESS;
			rating += 17;
			if (peek) msg_print("Lordliness");
			break;

		      case 3:
			i_ptr->flags1 |= (TR1_INT);
			i_ptr->flags2 |= (TR2_RES_ELEC | TR2_RES_COLD |
					  TR2_RES_ACID | TR2_RES_FIRE |
					  TR2_SUST_INT);
			i_ptr->flags3 |= (TR3_IGNORE_ELEC | TR3_IGNORE_COLD |
					  TR3_IGNORE_ACID | TR3_IGNORE_FIRE);
			i_ptr->pval = randint(3);
			i_ptr->cost += 3000 + i_ptr->pval * 500;
			i_ptr->name2 = EGO_MAGI;
			rating += 15;
			if (peek) msg_print("Crown of the Magi");
			break;

		      case 4:
			i_ptr->flags1 |= TR1_CHR;
			i_ptr->flags2 |= TR2_SUST_CHR;
			i_ptr->pval = randint(4);
			i_ptr->cost += 750;
			i_ptr->name2 = EGO_BEAUTY;
			rating += 8;
			if (peek) msg_print("Beauty");
			break;

		      case 5:
			i_ptr->flags1 |= (TR1_SEARCH);
			i_ptr->flags3 |= (TR3_SEE_INVIS);
			i_ptr->pval = 5 * (1 + randint(4));
			i_ptr->cost += 1000 + i_ptr->pval * 100;
			i_ptr->name2 = EGO_SEEING;
			rating += 8;
			if (peek) msg_print("Seeing");
			break;

		      case 6:
			i_ptr->flags3 |= TR3_REGEN;
			i_ptr->cost += 1500;
			i_ptr->name2 = EGO_REGENERATION;
			rating += 10;
			if (peek) msg_print("Regeneration");
			break;
		    }
		}
	    }
	}

	else if (magik(cursed)) {

	    i_ptr->toac -= m_bonus(1, 20, level);
	    i_ptr->flags3 |= TR3_CURSED;
	    i_ptr->cost = 0L;

	    if (magik(special))
		switch (randint(7)) {
		  case 1:
		    i_ptr->flags1 |= TR1_INT;
		    i_ptr->pval = -randint(5);
		    i_ptr->name2 = EGO_STUPIDITY;
		    break;
		  case 2:
		  case 3:
		    i_ptr->flags1 |= TR1_WIS;
		    i_ptr->pval = -randint(5);
		    i_ptr->name2 = EGO_DULLNESS;
		    break;
		  case 4:
		  case 5:
		    i_ptr->flags1 |= TR1_STR;
		    i_ptr->pval = -randint(5);
		    i_ptr->name2 = EGO_WEAKNESS;
		    break;
		  case 6:
		    i_ptr->flags3 |= TR3_TELEPORT;
		    i_ptr->name2 = EGO_TELEPORTATION;
		    break;
		  case 7:
		    i_ptr->pval = -randint(5);
		    i_ptr->flags1 |= TR1_CHR;
		    i_ptr->name2 = EGO_UGLINESS;
		    break;
		}
	}
	break;

      case TV_RING:

	switch (i_ptr->sval) {

	  /* Strength, Constitution, Dexterity, Intelligence */
	  case SV_RING_STR:
	  case SV_RING_CON:
	  case SV_RING_DEX:
	  case SV_RING_INT:
	    i_ptr->pval = m_bonus(1, 6, level);
	    i_ptr->cost += i_ptr->pval * 100;
	    if (magik(cursed)) {
		i_ptr->pval = -m_bonus(1, 10, level);
		i_ptr->flags3 |= TR3_CURSED;
		i_ptr->cost = 0L;
		/* XXX Lose "Sustain" field */
	    }
	    break;

	  /* Ring of Speed! */
	  case SV_RING_SPEED:

	    if (magik(cursed)) {
		i_ptr->pval = -randint(3);
		i_ptr->flags3 |= TR3_CURSED;
		i_ptr->cost = 0L;
		break;
	    }

	    /* Basic speed bonus */
	    i_ptr->pval = 1;

	    /* Very rarely, supercharge the ring */
	    while (randint(888 * i_ptr->pval) == 1) {
		i_ptr->pval++;
		i_ptr->cost = (i_ptr->cost + 5000L) * 3;
		rating += 10;
		if (peek) msg_print("Ring of Speed +2");
	    }

	    rating += 35;
	    if (peek) msg_print("Ring of Speed");
	    break;

	  /* Searching */
	  case SV_RING_SEARCHING:
	    i_ptr->pval = 5 * m_bonus(1, 10, level);
	    i_ptr->cost += i_ptr->pval * 30;
	    if (magik(cursed)) {
		i_ptr->pval = -i_ptr->pval;
		i_ptr->flags3 |= TR3_CURSED;
		i_ptr->cost = 0L;
	    }
	    break;

	  /* Flames, Acid, Ice */
	  case SV_RING_FLAMES:
	  case SV_RING_ACID:
	  case SV_RING_ICE:
	    i_ptr->toac = m_bonus(1, 10, level);
	    i_ptr->toac += 5 + randint(7);
	    i_ptr->cost += i_ptr->toac * 100;
	    break;

	  /* WOE, Stupidity */
	  case SV_RING_WOE:
	  case SV_RING_STUPIDITY:
	    i_ptr->toac = (-5) - m_bonus(1,10,level);
	    i_ptr->pval = (-randint(5));
	    break;

	  /* Increase damage */
	  case 19:
	    i_ptr->todam = m_bonus(1, 10, level);
	    i_ptr->todam += 3 + randint(10);
	    i_ptr->cost += i_ptr->todam * 100;
	    if (magik(cursed)) {
		i_ptr->todam = -i_ptr->todam;
		i_ptr->flags3 |= TR3_CURSED;
		i_ptr->cost = 0L;
	    }
	    break;

	  /* Increase To-Hit */
	  case 20:
	    i_ptr->tohit = m_bonus(1, 10, level);
	    i_ptr->tohit += 3 + randint(10);
	    i_ptr->cost += i_ptr->tohit * 100;
	    if (magik(cursed)) {
		i_ptr->tohit = -i_ptr->tohit;
		i_ptr->flags3 |= TR3_CURSED;
		i_ptr->cost = 0L;
	    }
	    break;

	  /* Protection */
	  case 21:
	    i_ptr->toac = m_bonus(0, 10, level);
	    i_ptr->toac += 4 + randint(5);
	    i_ptr->cost += i_ptr->toac * 100;
	    if (magik(cursed)) {
		i_ptr->toac = -i_ptr->toac;
		i_ptr->flags3 |= TR3_CURSED;
		i_ptr->cost = 0L;
	    }
	    break;

	  /* Slaying */
	  case 30:
	    i_ptr->todam = m_bonus(1, 10, level);
	    i_ptr->todam += 2 + randint(3);
	    i_ptr->tohit = m_bonus(1, 10, level);
	    i_ptr->tohit += 2 + randint(3);
	    i_ptr->cost += (i_ptr->tohit + i_ptr->todam) * 100;
	    if (magik(cursed)) {
		i_ptr->tohit = -i_ptr->tohit;
		i_ptr->todam = -i_ptr->todam;
		i_ptr->flags3 |= TR3_CURSED;
		i_ptr->cost = 0L;
	    }
	    break;
	}
	break;

      case TV_AMULET:

	if ((i_ptr->sval == SV_AMULET_WISDOM) ||
	    (i_ptr->sval == SV_AMULET_CHARISMA)) {
	    i_ptr->pval = m_bonus(1, 5, level);
	    if (magik(cursed)) {
		i_ptr->pval = -i_ptr->pval;
		i_ptr->flags3 |= TR3_CURSED;
		i_ptr->cost = 0L;
		/* XXX Remove Sustain */
	    }
	    else {
		i_ptr->cost += i_ptr->pval * 100;
	    }
	}
	else if (i_ptr->sval == SV_AMULET_SEARCHING) {
	    i_ptr->pval = 5 * (randint(3) + m_bonus(0, 8, level));
	    i_ptr->cost += 20 * i_ptr->pval;
	    if (magik(cursed)) {
		i_ptr->pval = -i_ptr->pval;
		i_ptr->flags3 |= TR3_CURSED;
		i_ptr->cost = 0L;
	    }
	}
	else if (i_ptr->sval == SV_AMULET_THE_MAGI) {
	    rating += 25;
	    i_ptr->pval = 5 * (randint(2) + m_bonus(0, 10, level));
	    i_ptr->toac = randint(4) + m_bonus(0, 8, level) - 2;
	    if (i_ptr->toac < 0) i_ptr->toac = 0;
	    i_ptr->cost += 20 * i_ptr->pval + 50 * i_ptr->toac;
	}
	else if (i_ptr->sval == SV_AMULET_DOOM) {
	    i_ptr->pval = (-randint(5) - m_bonus(2, 10, level));
	    i_ptr->toac = (-randint(3) - m_bonus(0, 6, level));
	    i_ptr->flags3 |= TR3_CURSED;
	    i_ptr->cost = 0L;
	}
	break;

      case TV_CLOAK:

	if (good || magik(chance)) {

	    i_ptr->toac += 1 + m_bonus(0, 20, level);

	    if (great || magik(special)) {

		if ((randint(10) == 1) &&
		    okay && make_artifact(i_ptr)) return;

		if (randint(2) == 1) {
		    i_ptr->flags3 |= (TR3_IGNORE_ACID);
		    i_ptr->toac += m_bonus(0, 10, level) + (5 + randint(3));
		    i_ptr->cost += 250L;
		    i_ptr->name2 = EGO_PROTECTION;
		    rating += 10;
		}
		else if (randint(10) == 1) {
		    i_ptr->toac += 10 + randint(10);
		    i_ptr->pval = randint(3);
		    i_ptr->flags1 |= (TR1_STEALTH);
		    i_ptr->flags2 |= (TR2_RES_ACID);
		    i_ptr->flags3 |= (TR3_IGNORE_ACID);
		    i_ptr->name2 = EGO_AMAN;
		    i_ptr->cost += 4000 + (100 * i_ptr->toac);
		    rating += 16;
		}
		else {
		    i_ptr->toac += m_bonus(3, 10, level);
		    i_ptr->pval = randint(3);
		    i_ptr->flags1 |= TR1_STEALTH;
		    i_ptr->name2 = EGO_STEALTH;
		    i_ptr->cost += 500 + (50 * i_ptr->pval);
		    rating += 9;
		}
	    }
	}

	else if (magik(cursed)) {
	    tmp = randint(3);
	    if (tmp == 1) {
		i_ptr->name2 = EGO_IRRITATION;
		i_ptr->flags3 |= (TR3_SHOW_MODS | TR3_AGGRAVATE);
		i_ptr->toac -= m_bonus(1, 10, level);
		i_ptr->tohit -= m_bonus(1, 10, level);
		i_ptr->todam -= m_bonus(1, 10, level);
	    }
	    else if (tmp == 2) {
		i_ptr->name2 = EGO_VULNERABILITY;
		i_ptr->toac -= m_bonus(10, 20, level + 50);
	    }
	    else {
		i_ptr->name2 = EGO_ENVELOPING;
		i_ptr->flags3 |= (TR3_SHOW_MODS);
		i_ptr->toac -= m_bonus(1, 10, level);
		i_ptr->tohit -= m_bonus(2, 15, level + 10);
		i_ptr->todam -= m_bonus(2, 15, level + 10);
	    }
	    i_ptr->cost = 0L;
	    i_ptr->flags3 |= TR3_CURSED;
	}
	break;


      /* Add charges to wands */
      case TV_WAND:
	charge_wand(i_ptr);
	break;

      /* Add charges to staffs */
      case TV_STAFF:
	charge_staff(i_ptr);
	break;

      /* Hack -- "charge" lights */
      case TV_LITE:

	/* Torches */
	if (i_ptr->sval == SV_LITE_TORCH) {
	    i_ptr->pval = randint(i_ptr->pval);
	}

	/* Lanterns */            
	if (i_ptr->sval == SV_LITE_LANTERN) {
	    i_ptr->pval = randint(i_ptr->pval);
	}

	break;

      /* Place traps and contents in Chests */
      case TV_CHEST:

	/* Chests have "goodness" based on the level */
	i_ptr->pval = randint(level);

	/* Pick a trap (or set of traps) for the chest */
	/* Note that the traps give an idea of the "value" */

	switch (randint(i_ptr->pval + 4)) {
	  case 1:
	    i_ptr->flags1 = 0L;
	    i_ptr->flags2 = 0L;
	    break;
	  case 2:
	    i_ptr->flags2 |= CH2_LOCKED;
	    break;
	  case 3:
	  case 4:
	    i_ptr->flags2 |= (CH2_LOSE_STR | CH2_LOCKED);
	    break;
	  case 5:
	  case 6:
	    i_ptr->flags2 |= (CH2_POISON | CH2_LOCKED);
	    break;
	  case 7:
	  case 8:
	  case 9:
	    i_ptr->flags2 |= (CH2_PARALYSED | CH2_LOCKED);
	    break;
	  case 10:
	  case 11:
	    i_ptr->flags2 |= (CH2_EXPLODE | CH2_LOCKED);
	    break;
	  case 12:
	  case 13:
	  case 14:
	    i_ptr->flags2 |= (CH2_SUMMON | CH2_LOCKED);
	    break;
	  case 15:
	  case 16:
	  case 17:
	    i_ptr->flags2 |= (CH2_PARALYSED | CH2_POISON | CH2_LOSE_STR |
			     CH2_LOCKED);
	    break;
	  default:
	    i_ptr->flags2 |= (CH2_SUMMON | CH2_EXPLODE | CH2_LOCKED);
	    break;
	}

	break;

      case TV_BOLT: case TV_ARROW: case TV_SHOT:

	/* Hack -- Generate multiple missiles */
	i_ptr->number = damroll(7,6);

	/* Apply some magic */
	if (good || magik(chance)) {

	    i_ptr->tohit = randint(5) + m_bonus(1, 15, level);
	    i_ptr->todam = randint(5) + m_bonus(1, 15, level);

	    /* see comment for weapons */
	    if (great || magik(5*special/2)) {

		switch (randint(11)) {

		  case 1: case 2: case 3:
		    i_ptr->name2 = EGO_WOUNDING; /* swapped with slaying -CFT */
		    i_ptr->tohit += 5;
		    i_ptr->todam += 5;
		    i_ptr->damage[0] += 1;		/* Extra die of damage */
		    i_ptr->cost += 30;
		    rating += 5;
		    break;

		  case 4: case 5:
		    i_ptr->name2 = EGO_FIRE;
		    i_ptr->flags1 |= (TR1_BRAND_FIRE);
		    /* i_ptr->flags2 |= (TR2_RES_FIRE); */
		    i_ptr->flags3 |= (TR3_IGNORE_FIRE);
		    i_ptr->tohit += 2;
		    i_ptr->todam += 4;
		    i_ptr->cost += 25;
		    rating += 6;
		    break;

		  case 6: case 7:
		    i_ptr->name2 = EGO_SLAY_EVIL;
		    i_ptr->flags1 |= TR1_SLAY_EVIL;
		    i_ptr->tohit += 3;
		    i_ptr->todam += 3;
		    i_ptr->cost += 25;
		    rating += 7;
		    break;

		  case 8: case 9:
		    i_ptr->name2 = EGO_SLAY_ANIMAL;
		    i_ptr->flags1 |= TR1_SLAY_ANIMAL;
		    i_ptr->tohit += 2;
		    i_ptr->todam += 2;
		    i_ptr->cost += 30;
		    rating += 5;
		    break;

		  case 10:
		    i_ptr->name2 = EGO_DRAGON_SLAYING;
		    i_ptr->flags1 |= TR1_SLAY_DRAGON;
		    i_ptr->tohit += 3;
		    i_ptr->todam += 3;
		    i_ptr->cost += 35;
		    rating += 9;
		    break;

		  case 11:
		    i_ptr->name2 = EGO_SLAYING; /* swapped w/ wounding -CFT */
		    i_ptr->tohit += 10; /* reduced because of dice bonus -CFT */
		    i_ptr->todam += 10;
		    i_ptr->damage[0] += 2; /* two extra die of damage */
		    i_ptr->cost += 45;
		    rating += 10;
		    break;
		}
	    }

	    /* Very special arrows */
	    while (magik(special)) {
		i_ptr->damage[0] += 1;
		i_ptr->cost += i_ptr->damage[0] * 5;
	    }
	}

	else if (magik(cursed)) {

	    i_ptr->tohit = (-randint(10)) - m_bonus(5, 25, level);
	    i_ptr->todam = (-randint(10)) - m_bonus(5, 25, level);
	    i_ptr->flags3 |= TR3_CURSED;
	    i_ptr->cost = 0L;

	    if (randint(5)==1) {
		i_ptr->name2 = EGO_BACKBITING;
		i_ptr->tohit -= 20;
		i_ptr->todam -= 20;
	    }
	}

	break;

      case TV_SPIKE:

	/* Hack -- Generate multiple spikes */
	i_ptr->number = damroll(7,6);

	break;
    }
}




/*
 * XXX Mega-Hack -- attempt to place one of the "Special Objects"
 */
static int special_place_object(int y, int x)
{
    register int	cur_pos;
    cave_type		*c_ptr;
    inven_type		hack;


    /* Is this cave grid sacred? */
    if (!valid_grid(y,x)) return (FALSE);

    /* Get the cave */
    c_ptr = &cave[y][x];

    /* Hack -- do not hurt doors/rubble */
    if (!floor_grid(y, x) && c_ptr->i_idx) return (FALSE);
    
    

    /* Hack -- clean up "hack" */
    invcopy(&hack, OBJ_NOTHING);

    /* Hack -- Try to allocate a special object */
    if (!make_artifact_special(&hack)) return (FALSE);


    /* Delete anything that is there */
    delete_object(y, x);

    /* Make the object, using the index from above */
    cur_pos = i_pop();

    /* Place the object there */
    i_list[cur_pos] = hack;

    /* Place the object */
    i_list[cur_pos].iy = y;
    i_list[cur_pos].ix = x;
    c_ptr->i_idx = cur_pos;

    /* Is it on the player? */
    if (c_ptr->m_idx == 1) {
	msg_print("You feel something roll beneath your feet.");
    }

    /* Success */
    return TRUE;
}


/*
 * Attempts to places a random object at the given location -RAK-
 */
void place_object(int y, int x)
{
    register int cur_pos, tmp;
    cave_type *c_ptr;

    /* Certain locations are not valid */
    if (!valid_grid(y,x)) return;

    /* Get the cave */
    c_ptr = &cave[y][x];

    /* Hack -- do not hurt doors/rubble */
    if (!floor_grid(y, x) && c_ptr->i_idx) return;

    
    /* Hack -- One in 500 chance of (maybe) placing a "Special Object" */
    if (rand_int(500) == 1) {
	if (special_place_object(y,x)) return;
    }

    /* Delete anything already there */
    delete_object(y, x);

    /* Hack -- don't generate another chest if opening_chest is true -CWS */
    do {
	tmp = get_obj_num(dun_level, FALSE);
    } while (opening_chest && (k_list[tmp].tval == TV_CHEST));

    /* Make it */
    cur_pos = i_pop();
    invcopy(&i_list[cur_pos], tmp);

    /* Place it */
    i_list[cur_pos].iy = y;
    i_list[cur_pos].ix = x;
    c_ptr->i_idx = cur_pos;

    /* Let it be magic, perhaps even an artifact */
    apply_magic(&i_list[cur_pos], dun_level, TRUE, FALSE, FALSE);

    /* Check the rating */
    if (k_list[tmp].level > dun_level) {

	/* Rating increase */
	rating += (k_list[tmp].level - dun_level);

	/* Hack -- describe the object */
	if (peek) {
	    char buf[200];
	    objdes_store(buf, &i_list[cur_pos], TRUE);
	    msg_print(buf);
	}
    }

    /* Under the player */
    if (c_ptr->m_idx == 1) {
	msg_print ("You feel something roll beneath your feet.");
    }
}


/*
 * Places a "GOOD" object at given row, column co-ordinate ~Ludwig 
 * If "great" is TRUE, place a "GREAT" object
 */
void place_good(int y, int x, bool great)
{
    register int cur_pos, k_idx;
    int          tv, sv;
    cave_type *c_ptr;

    /* Do not hurt artifacts, stairs, store doors */
    if (!valid_grid(y, x)) return;
    
    /* Get the grid */
    c_ptr = &cave[y][x];

    /* Hack -- do not hurt doors/rubble */
    if (!floor_grid(y, x) && c_ptr->i_idx) return;
    

    /* Hack -- much higher chance of doing "Special Objects" */
    if (randint(10) == 1) {
	if (special_place_object(y, x)) return;
    }

    /* Pick a good object */
    while (1) {

	k_idx = get_obj_num((object_level + 10), TRUE);

	/* Examine the object */
	tv = k_list[k_idx].tval;
	sv = k_list[k_idx].sval;

	/* Rusty Chainmail is not good */
	if ((tv == TV_HARD_ARMOR) && (sv == SV_RUSTY_CHAIN_MAIL)) continue;

	/* Filthy Rags are not good */
	if ((tv == TV_SOFT_ARMOR) && (sv == SV_FILTHY_RAG)) continue;

	/* Broken daggers are not good */
	if ((tv == TV_SWORD) && (sv == SV_BROKEN_DAGGER)) continue;

	/* Broken swords are not good */
	if ((tv == TV_SWORD) && (sv == SV_BROKEN_SWORD)) continue;

	/* Normal weapons/armour */
	if ((tv == TV_HELM) || (tv == TV_SHIELD) || (tv == TV_CLOAK) ||
	    (tv == TV_SWORD) || (tv == TV_HAFTED) || (tv == TV_POLEARM) ||
	    (tv == TV_BOW) || (tv == TV_BOLT) || (tv == TV_ARROW) ||
	    (tv == TV_HARD_ARMOR) || (tv == TV_SOFT_ARMOR) ||
	    (tv == TV_DRAG_ARMOR) ||
	    (tv == TV_BOOTS) || (tv == TV_GLOVES)) {
	    break;
	}

	/* XXX Hack -- High spell books are good.  Highest is great. */
	if (((tv == TV_MAGIC_BOOK) || (tv == TV_PRAYER_BOOK)) &&
	     (sv >= (great ? SV_BOOK + 8 : SV_BOOK + 4))) {
	    break;
	}
    }


    /* Delete anything already there */
    delete_object(y, x);

    /* Make a new object, drop into dungeon */
    cur_pos = i_pop();
    invcopy(&i_list[cur_pos], k_idx);
    i_list[cur_pos].iy = y;
    i_list[cur_pos].ix = x;
    c_ptr->i_idx = cur_pos;

    /* Apply some good magic to the item.  Make a great item if requested. */
    apply_magic(&i_list[cur_pos], object_level, TRUE, TRUE, great);

    /* XXX Should we be "helping" the rating? */
    if (k_list[k_idx].level > object_level) {

	/* Hack -- look at it */
	if (peek) {
	    char buf[200];
	    objdes_store(buf, &i_list[cur_pos], TRUE);
	    msg_print(buf);
	}
    }

    if (c_ptr->m_idx == 1) {
	msg_print("You feel something roll beneath your feet.");
    }
}




/*
 * Creates objects nearby the coordinates given		-RAK-	 
 */
void random_object(int y, int x, int num)
{
    register int        i, j, k;

    object_level = dun_level;

    /* Attempt to place 'num' objects */
    for (; num > 0; --num) {

	/* Try up to 11 spots looking for empty space */
	for (i = 0; i < 11; ++i) {

	    /* Pick a random location */
	    j = rand_range(y-2, y+2);
	    k = rand_range(x-3, x+3);

	    /* Require empty floor space */
	    if (!clean_grid(j,k)) continue;

	    /* Place something */
	    if (randint(100) < 75) {
		place_object(j, k);
	    }
	    else {
		place_gold(j, k);
	    }

	    /* Placement accomplished */
	    break;
	}
    }
}


/*
 * Same as above, but always "special"
 */
void special_random_object(int y, int x, int num)
{
    register int        i, j, k;

    object_level = dun_level;

    for (; num > 0; --num) {

	/* Try up to 11 spots looking for empty space */
	for (i = 0; i < 11; ++i) {

	    /* Pick a random spot */
	    j = y - 3 + randint(5);
	    k = x - 4 + randint(7);

	    /* Must have a clean grid */
	    if (!clean_grid(j, k)) continue;

	    /* Place a "Special Object", or a great object */
	    if ((randint(5) != 1) || !special_place_object(j, k)) {
		place_good(j, k, TRUE);
	    }

	    /* Placement accomplished */
	    break;
	}
    }
}




/*
 * Places a random trap at the given location.		-RAK-	 
 * Note that all traps start out visible, but are always made "invisible"
 * when created (except pits, which are sometimes visible).  All traps are
 * made "visible" when "discovered" (including pits, which are "uncovered").
 *
 * XXX XXX This routine should be redone to reflect trap "level".
 */
void place_trap(int y, int x)
{
    int trap, tval;
    cave_type *c_ptr;
    inven_type *i_ptr;
    register int cur_pos;
    bool invis = TRUE;

    /* Do not hurt artifacts, stairs, store doors */
    if (!valid_grid(y, x)) return;

    /* Get the cave grid */
    c_ptr = &cave[y][x];

    /* Hack -- do not hurt doors/rubble */
    if (!floor_grid(y, x) && c_ptr->i_idx) return;

    /* Don't put traps under player/monsters, it's annoying -CFT */
    if (c_ptr->m_idx) return;

    /* Delete whatever is there */
    delete_object(y, x);

    /* Hack -- Mark the trap as invisible */
    tval = TV_INVIS_TRAP;

    /* Hack -- Pick a trap */
    trap = OBJ_TRAP_LIST + rand_int(MAX_TRAP);

    /* Hack -- Ocassionally, just make a visible open pit */
    if (randint(10) == 1) {
	trap = OBJ_OPEN_PIT;
	invis = FALSE;
    }

    /* Make a new object */
    cur_pos = i_pop();
    i_ptr = &i_list[cur_pos];
    invcopy(i_ptr, trap);
    i_ptr->iy = y;
    i_ptr->ix = x;
    c_ptr->i_idx = cur_pos;

    /* Hack -- no trap doors on quest levels */
    if ((i_ptr->sval == SV_TRAP_TRAP_DOOR) && (is_quest(dun_level))) {
	i_ptr->sval = SV_TRAP_SPIKED_PIT;
    }

    /* Hack -- Make the trap invisible */
    if (invis) i_ptr->tval = TV_INVIS_TRAP;
}


/*
 * Places rubble at location y, x			-RAK-	
 */
void place_rubble(int y, int x)
{
    register int        cur_pos;
    register cave_type *c_ptr;
    register inven_type *i_ptr;

    /* Do not hurt artifacts, stairs, store doors */
    if (!valid_grid(y, x)) return;

    /* Delete whatever is there */
    delete_object(y, x);

    cur_pos = i_pop();
    i_ptr = &i_list[cur_pos];
    invcopy(i_ptr, OBJ_RUBBLE);
    i_ptr->iy = y;
    i_ptr->ix = x;
    
    /* Put the rubble in the cave */
    c_ptr = &cave[y][x];
    c_ptr->i_idx = cur_pos;
    
    /* Hack -- nuke any walls */
    c_ptr->fval = CORR_FLOOR;
    
    /* Hack -- update the view if seen */
    if (player_has_los(y, x)) {
	update_view();
	update_lite();
	update_monsters();
    }
}

/*
 * Return the "coin type" of a monster race
 * Used to allocate proper treasure for killing creeping coins
 */
int get_coin_type(monster_race *r_ptr)
{
    cptr name = r_ptr->name;
    if (!stricmp(name, "Creeping copper coins")) return (2);
    if (!stricmp(name, "Creeping silver coins")) return (5);
    if (!stricmp(name, "Creeping gold coins")) return (10);
    if (!stricmp(name, "Creeping mithril coins")) return (16);
    if (!stricmp(name, "Creeping adamantite coins")) return (17);
    return (0);
}

/*
 * Places a treasure (Gold or Gems) at given row, column -RAK-	
 */
void place_gold(int y, int x)
{
    register int        i, cur_pos;
    register inven_type *i_ptr;
    cave_type *c_ptr;

    /* Do not hurt illegals, artifacts, stairs, store doors */
    if (!valid_grid(y, x)) return;

    /* Get the grid */
    c_ptr = &cave[y][x];

    /* Hack -- do not hurt doors/rubble */
    if (!floor_grid(y, x) && c_ptr->i_idx) return;

    /* Pick a Treasure variety */
    i = ((randint(object_level + 2) + 2) / 2) - 1;
    if (randint(GREAT_OBJ) == 1) {
	i += randint(object_level + 1);
    }

    /* Creeping Coins generate pieces of themselves */
    if (coin_type) i = coin_type;

    /* Do not create impossible Treasure Type */
    if (i >= MAX_GOLD) i = MAX_GOLD - 1;

    /* Delete the object (acidic gold?) */
    delete_object(y, x);

    cur_pos = i_pop();
    i_ptr = &i_list[cur_pos];
    invcopy(i_ptr, OBJ_GOLD_LIST + i);
    i_ptr->cost += (8L * (long)randint((int)i_ptr->cost)) + randint(8);
    i_ptr->iy = y;
    i_ptr->ix = x;
    c_ptr->i_idx = cur_pos;

    /* Hack -- average the values to make Creeping _xxx_ coins */
    /* not give too great treasure drops */
    if (coin_type) {
	i_ptr->cost = ((8L * (long)randint((int)k_list[OBJ_GOLD_LIST + i].cost))
		       + (i_ptr->cost)) >> 1;
    }

    if (c_ptr->m_idx == 1) {
	msg_print("You feel something roll beneath your feet.");
    }
}


/*
 * An entry for the object allocator below
 */
typedef struct _kind_entry {
    int16u k_idx;		/* Object kind index */
    int8u locale;		/* Base dungeon level */
    int8u rarity;		/* Rarity of occurance */
} kind_entry;


/*
 * Returns the array number of a random object
 * Uses the locale/rarity info for distribution.
 */
int get_obj_num(int level, int good)
{
    register int i, j;

    /* Number of entries in the "k_sort" table */
    static int16u size = 0;

    /* The actual table of entries */
    static kind_entry *table = NULL;

    /* Number of entries at each locale */
    static int16u t_lev[256];

    /* Initialize the table */
    if (!size) {

	inven_kind *k_ptr;

	int16u aux[256];

	/* Clear the level counter and the aux array */
	for (i = 0; i < 256; i++) t_lev[i] = aux[i] = 0;

	/* Scan all of the objects */
	for (i = 0; i < MAX_K_IDX; i++) {

	    /* Get the i'th object */
	    k_ptr = &k_list[i];

	    /* Scan all of the locale/rarity pairs */
	    for (j = 0; j < 4; j++) {

		/* Count valid pairs */
		if (k_ptr->rarity[j]) {

		    /* Count the total entries */
		    size++;

		    /* Count the entries at each level */
		    t_lev[k_ptr->locale[j]]++;
		}
	    }
	}

	/* Combine the "t_lev" entries */
	for (i = 1; i < 256; i++) t_lev[i] += t_lev[i-1];

	/* Allocate the table */
	C_MAKE(table, size, kind_entry);

	/* Initialize the table */
	for (i = 0; i < MAX_K_IDX; i++) {

	    /* Get the i'th object */
	    k_ptr = &k_list[i];

	    /* Scan all of the locale/rarity pairs */
	    for (j = 0; j < 4; j++) {

		/* Count valid pairs */
		if (k_ptr->rarity[j]) {

		    int r, x, y, z;

		    /* Extract the rarity/locale */                    
		    r = k_ptr->rarity[j];
		    x = k_ptr->locale[j];

		    /* Skip entries preceding our locale */
		    y = (x > 0) ? t_lev[x-1] : 0;

		    /* Skip previous entries at this locale */
		    z = y + aux[x];

		    /* Load the table entry */
		    table[z].k_idx = i;
		    table[z].locale = x;
		    table[z].rarity = r;

		    /* Another entry complete for this locale */
		    aux[x]++;
		}
	    }
	}
    }


    /* Pick an object */
    while (1) {

	/* Town level is easy */
	if (level == 0) {

	    /* Pick a level 0 entry */
	    i = rand_int(t_lev[0]);
	}

	/* Other levels sometimes have great stuff */
	else {

	    /* Never exceed a given level */
	    if (level > MAX_K_LEV) level = MAX_K_LEV;

	    /* Occasionally, get a "better" object */
	    if (randint(GREAT_OBJ) == 1) {

		/* What a bizarre calculation */
		level = 1 + (level * MAX_K_LEV / randint(MAX_K_LEV));
		if (level > MAX_K_LEV) level = MAX_K_LEV;
	    }


	/*
	 * This code has been added to make it slightly more likely to get
	 * the higher level objects.	Originally a uniform distribution
	 * over all objects less than or equal to the dungeon level.  This
	 * distribution makes a level n objects occur approx 2/n% of the time
	 * on level n, and 1/2n are 0th level. 
	 */

	    /* Pick any object at or below the given level */
	    i = rand_int(t_lev[level]);

	    /* Sometimes, try for a "better" item */
	    if (randint(3) != 1) {

		/* Pick another object at or below the given level */
		j = rand_int(t_lev[level]);

		/* Keep it if it is "better" */
		if (table[i].locale < table[j].locale) i = j;
	    }

	    /* Sometimes, try for a "better" item */
	    if (randint(3) != 1) {

		/* Pick another object at or below the given level */
		j = rand_int(t_lev[level]);

		/* Keep it if it is "better" */
		if (table[i].locale < table[j].locale) i = j;
	    }
	}

	/* Access the "k_idx" of the chosen item */
	j = table[i].k_idx;

	/* The "good" parameter overwhelms "rarity" requirements */
	if (good) break;

	/* Play the "rarity game" */
	if (randint(table[i].rarity) == 1) break;
    }

    /* Accept that object */
    return (j);
}



