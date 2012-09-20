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
 * Items always include a "location" in the cave, if on the floor.
 *
 * This routine only applies to items on the floor.
 *
 * Inven/Equip/Store items are not processed by this routine.
 */
void delete_object_idx(int i)
{
    s16b        i_idx = (s16b)(i);

    /* One less item */
    i_max--;

    /* Last object deletion, simply wipe */
    if (i_idx == i_max) {

	/* Be cautious, wipe the "dead" object */
	invcopy(&i_list[i_max], OBJ_NOTHING);
    }

    /* When "pushing" an non-final object, we compact */
    else {

	register int iy, ix;

	/* Compact, via structure copy */
	i_list[i_idx] = i_list[i_max];

	/* Be cautious, wipe the "dead" object */
	invcopy(&i_list[i_max], OBJ_NOTHING);

	/* Extract the "location" of the "moved object" */
	iy = i_list[i_idx].iy;
	ix = i_list[i_idx].ix;

	/* Hack -- Use iy,ix to look for the old "last items" */
	if (in_bounds(iy, ix)) {

	    /* Update the cave record */
	    if (cave[iy][ix].i_idx == i_max) {
		cave[iy][ix].i_idx = i_idx;
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

    /* Forget about items which are "dark" */
    if (!(c_ptr->info & CAVE_PL)) c_ptr->info &= ~CAVE_FM;

    /* Redraw the spot (may reset field mark) */
    lite_spot(y, x);
}



/*
 * Erase "i_list" (called only from "generate.c")
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
 * When too many objects gather on the floor, delete some of them
 *
 * Note that the player could intentionally collect so many artifacts,
 * and create so many stairs, that we become unable to compact.
 * This may cause the program to crash.
 */
static void compact_objects()
{
    register int        i, j;
    register cave_type *c_ptr;
    register inven_type *i_ptr;
    int                 cnt, num;
    int			cur_lev, cur_dis, chance;


    /* Debugging message */
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

    /* Update the view/lite */
    update_view();
    update_lite();
    
    /* Redraw the monsters */
    update_monsters();
}


/*
 * Acquires and returns the index of a "free" item.
 */
int i_pop(void)
{
    /* Compact if needed */
    if (i_max == MAX_I_IDX) compact_objects();

    /* XXX XXX XXX XXX Out of memory! */
    if (i_max == MAX_I_IDX) quit("Too many objects!");

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


#if 0

/*
 * Enchant a bonus based on degree desired -RAK-
 *
 * Lets just change this to make sense.  Now it goes from base to limit,
 * roughly proportional to the level.... -CWS
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

#endif



/*
 * Enchant a bonus based on degree desired -RAK-
 * 
 * This is rather heavily modified from RAK's Moria m_bonus() and versions of
 * m_bonus() used in Angband up to version 2.7.3. In fact, it was basically
 * rewritten (by randy@PICARD.tamu.edu (Randy Hutson))
 * 
 * The theory behind this m_bonus is to allocate bonuses based upon a "sliding,
 * reshaping" normal distribution. The mean of the normal distribution will
 * be proportionate the level. The standard deviation of bonuses will also
 * increase as the level increases. To understand the bonus distribution,
 * first imagine a normal "bell" curve centered at 0. As the object level
 * increases, the curve will slide to the right and flatten (be vertically
 * squashed). So as the object level increases, the average bonus increases
 * and the potential for large bonuses increases. Also, there are no "gaps"
 * in the mean bonus per level or in the mean standard deviation per level
 * between any levels.
 *
 * Actually, apparently the value can exceed the "limit" if the "range" is
 * large (thus an amulet of the Magi (+40) *is* possible).
 *
 * Only if MBONUS_CAN_EXCEED_LIMIT is defined. Is this behavior really
 * desirable? I don't see the point in having a limit and then exceeding
 * it (except perhaps for variety). Note that even if MBONUS_CAN_EXCEED_LIMIT
 * is defined, there is still a limit of "limit + limit/5". -Randy
 */
int m_bonus(int base, int limit, int level)
{
    int x, bonus_range, mean_bonus_per_level, std_dev, remainder;

    /* Paranoia -- enforce legal "range" */
    if (base >= limit) return (base);

    /* Don't let bonuses become "absurd" at very high levels. */
    if (level > MBONUS_MAX_LEVEL) level = MBONUS_MAX_LEVEL;

    /* Calculate the range of the bonus points to be allotted. */
    bonus_range = limit - base;

    /* The mean bonus is just proportional to the level. */
    mean_bonus_per_level = ((bonus_range * level) / MBONUS_MAX_LEVEL);


    /* 
     * To avoid floating point but still provide a smooth distribution
     * of bonuses, award an extra bonus point sometimes. The probability
     * that a bonus point will be award is based upon the truncated
     * remainder from the calculation of mean_bonus_per_level. Specifically,
     * the chance of a bonus point being award is remainder/MBONUS_MAX_LEVEL.
     */

    /* Hack -- avoid floating point computations */
    remainder = ((bonus_range * level) % MBONUS_MAX_LEVEL);
    if (randint(MBONUS_MAX_LEVEL) <= remainder) ++mean_bonus_per_level;


    /*
     * The "standard" object will differ from the mean by 50%. So as the
     * mean increases, the bonus distribution widens. Since the mean was
     * calculated from the level, the bonus distribution widens with the
     * level as well.
     */ 

    std_dev = mean_bonus_per_level / 2;

    /* Now compensate for any "lost" remainder. */
    if ((mean_bonus_per_level % 2) && rand_int(2)) ++std_dev;


    /*
     * It *appears* that OBJ_STD_ADJ is used to compensate for a peculiarity
     * in randnor(). Empirical tests have revealed that randnor(m, sd)
     * returns values with a S.D. of sd / 1.25, so this oddity is compensated
     * for by effectively multiplying the S.D. by 1.25 first.
     */

    /* Hack -- repair "randnor()" */
    std_dev = (std_dev * 125) / 100;

    /* Always allow some variety. */
    if (std_dev < 1) std_dev = 1;

    /* Get a random number from a normal distribution. */
    x = randnor(0, std_dev);

    /* Now, shift the normal distribution over to the desired mean. */
    x += mean_bonus_per_level;
   

    /*
     * Enforce minimum value. Note that since randnor() may return a
     * negative value, a "lot" of bonuses will just be the base bonus.
     * This is very dependent upon the mean bonus (and thus the level),
     * but even at high levels, the base bonus will be awarded more
     * than it should based strictly upon a normal distribution.
     */

    /* Enforce the minimum */
    if (x < base) return (base);

#ifdef MBONUS_CAN_EXCEED_LIMIT
    /* Hack -- allow values *slightly* higher than the limit */
    if (x > limit) limit = rand_range(limit, limit + bonus_range / 5);
#endif

    /* Enforce the maximum */
    if (x > limit) return (limit);

    /* Return the extracted value */
    return (x);
}






/*
 * Help pick and create a "special" object
 *
 * XXX XXX This function is a total hack.
 * Note that the function below calls us 20 times per "call".
 */
static int make_artifact_special_aux(inven_type *i_ptr)
{
    int lev = object_level;


    /* Analyze it */
    switch (randint(12)) {

      case 1:
	if (v_list[ART_GALADRIEL].cur_num) return (0);
	if (k_list[OBJ_GALADRIEL].level > lev + 40) return (0);
	if ((k_list[OBJ_GALADRIEL].level > lev) && (randint(30) > 1)) return (0);
	invcopy(i_ptr, OBJ_GALADRIEL);
	return (ART_GALADRIEL);
	break;

      case 2:
	if (v_list[ART_ELENDIL].cur_num) return (0);
	if (randint(8) > 1) return (0);
	if (k_list[OBJ_ELENDIL].level > lev + 40) return (0);
	if ((k_list[OBJ_ELENDIL].level > lev) && (randint(30) > 1)) return (0);
	invcopy(i_ptr, OBJ_ELENDIL);
	return (ART_ELENDIL);

      case 3:
	if (v_list[ART_THRAIN].cur_num) return (0);
	if (randint(18) > 1) return (0);
	if (k_list[OBJ_THRAIN].level > lev + 40) return (0);
	if ((k_list[OBJ_THRAIN].level > lev) && (randint(60) > 1)) return (0);
	invcopy(i_ptr, OBJ_THRAIN);
	return (ART_THRAIN);


      case 4:
	if (v_list[ART_CARLAMMAS].cur_num) return (0);
	if (randint(6) > 1) return (0);
	if (k_list[OBJ_CARLAMMAS].level > lev + 40) return (0);
	if ((k_list[OBJ_CARLAMMAS].level > lev) && (randint(35) > 1)) return (0);
	invcopy(i_ptr, OBJ_CARLAMMAS);
	return (ART_CARLAMMAS);

      case 5:
	if (v_list[ART_INGWE].cur_num) return (0);
	if (randint(10) > 1) return (0);
	if (k_list[OBJ_INGWE].level > lev + 40) return (0);
	if ((k_list[OBJ_INGWE].level > lev) && (randint(50) > 1)) return (0);
	invcopy(i_ptr, OBJ_INGWE);
	return (ART_INGWE);

      case 6:
	if (v_list[ART_DWARVES].cur_num) return (0);
	if (randint(25) > 1) return (0);
	if (k_list[OBJ_DWARVES].level > lev + 40) return (0);
	if ((k_list[OBJ_DWARVES].level > lev) && (randint(60) > 1)) return (0);
	invcopy(i_ptr, OBJ_DWARVES);
	return (ART_DWARVES);


      case 7:
	if (v_list[ART_BARAHIR].cur_num) return (0);
	if (randint(20) > 1) return (0);
	if (k_list[OBJ_BARAHIR].level > lev + 40) return (0);
	if ((k_list[OBJ_BARAHIR].level > lev) && (randint(50) > 1)) return (0);
	invcopy(i_ptr, OBJ_BARAHIR);
	return (ART_BARAHIR);

      case 8:
	if (v_list[ART_TULKAS].cur_num) return (0);
	if (randint(25) > 1) return (0);
	if (k_list[OBJ_TULKAS].level > lev + 40) return (0);
	if ((k_list[OBJ_TULKAS].level > lev) && (randint(65) > 1)) return (0);
	invcopy(i_ptr, OBJ_TULKAS);
	return (ART_TULKAS);

      case 9:
	if (v_list[ART_NARYA].cur_num) return (0);
	if (randint(30) > 1) return (0);
	if (k_list[OBJ_NARYA].level > lev + 40) return (0);
	if ((k_list[OBJ_NARYA].level > lev) && (randint(50) > 1)) return (0);
	invcopy(i_ptr, OBJ_NARYA);
	return (ART_NARYA);

      case 10:
	if (v_list[ART_NENYA].cur_num) return (0);
	if (randint(35) > 1) return (0);
	if (k_list[OBJ_NENYA].level > lev + 40) return (0);
	if ((k_list[OBJ_NENYA].level > lev) && (randint(60) > 1)) return (0);
	invcopy(i_ptr, OBJ_NENYA);
	return (ART_NENYA);

      case 11:
	if (v_list[ART_VILYA].cur_num) return (0);
	if (randint(40) > 1) return (0);
	if (k_list[OBJ_VILYA].level > lev + 40) return (0);
	if ((k_list[OBJ_VILYA].level > lev) && (randint(70) > 1)) return (0);
	invcopy(i_ptr, OBJ_VILYA);
	return (ART_VILYA);

      case 12:
	if (v_list[ART_POWER].cur_num) return (0);
	if (randint(60) > 1) return (0);
	if (k_list[OBJ_POWER].level > lev + 40) return (0);
	if ((k_list[OBJ_POWER].level > lev) && (randint(100) > 1)) return (0);
	invcopy(i_ptr, OBJ_POWER);
	return (ART_POWER);
    }

    return (0);
}



/*
 * Attempt to create one of the "Special Objects"
 *
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

    /* Extract the flags and other fields */
    i_ptr->flags1 = v_list[what].flags1;
    i_ptr->flags2 = v_list[what].flags2;
    i_ptr->flags3 = v_list[what].flags3;
    i_ptr->pval = v_list[what].pval;
    i_ptr->ac = v_list[what].ac;
    i_ptr->dd = v_list[what].dd;
    i_ptr->ds = v_list[what].ds;
    i_ptr->toac = v_list[what].toac;
    i_ptr->tohit = v_list[what].tohit;
    i_ptr->todam = v_list[what].todam;
    i_ptr->weight = v_list[what].weight;
    i_ptr->cost = v_list[what].cost;

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
    int i, what = 0;

    /* Hack -- no such thing as "plural" artifacts */
    if (i_ptr->number != 1) return (FALSE);

    /* XXX Note -- see "make_artifact_special()" as well */

    /* Check the artifact list -- skip the "specials" */
    for (i = 16; i < ART_MAX; i++) {

	/* Cannot make an artifact twice */
	if (v_list[i].cur_num) continue;

	/* XXX Paranoia -- hard code the object index */
	/* if (v_list[i].k_idx != i_ptr->k_idx) continue; */

	/* Must have the correct fields */
	if (v_list[i].tval != i_ptr->tval) continue;
	if (v_list[i].sval != i_ptr->sval) continue;

	/* We must make the "rarity roll" */
	if (randint(v_list[i].rarity) != 1) continue;
	
	/* Success */
	what = i;
	break;
    }
    
    /* Nothing made */
    if (!what) return (FALSE);

    /* Mark that artifact as Made */
    v_list[what].cur_num = 1;

    /* Save the Artifact Index */  
    i_ptr->name1 = what;

    /* Extract the flags and other fields */
    i_ptr->flags1 = v_list[what].flags1;
    i_ptr->flags2 = v_list[what].flags2;
    i_ptr->flags3 = v_list[what].flags3;
    i_ptr->pval = v_list[what].pval;
    i_ptr->ac = v_list[what].ac;
    i_ptr->dd = v_list[what].dd;
    i_ptr->ds = v_list[what].ds;
    i_ptr->toac = v_list[what].toac;
    i_ptr->tohit = v_list[what].tohit;
    i_ptr->todam = v_list[what].todam;
    i_ptr->weight = v_list[what].weight;
    i_ptr->cost = v_list[what].cost;

    /* Set the good item flag */
    good_item_flag = TRUE;

    /* Hack -- Describe -- XXX Too much detail */
    if (wizard || peek) {
	char buf[256];
	objdes_store(buf, i_ptr, TRUE);
	msg_print(buf);
    }

    /* An artifact was made */    
    return TRUE;
}


/*
 * Give an item one of the "powerful resistances"
 */		   
static void give_1_hi_resist(inven_type *i_ptr)
{
    switch (randint(10)) {
	case 1: i_ptr->flags2 |= TR2_RES_CONF; break;
	case 2: i_ptr->flags2 |= TR2_RES_SOUND; break;
	case 3: i_ptr->flags2 |= TR2_RES_LITE; break;
	case 4: i_ptr->flags2 |= TR2_RES_DARK; break;
	case 5: i_ptr->flags2 |= TR2_RES_CHAOS; break;
	case 6: i_ptr->flags2 |= TR2_RES_NETHER; break;
	case 7: i_ptr->flags2 |= TR2_RES_SHARDS; break;
	case 8: i_ptr->flags2 |= TR2_RES_NEXUS; break;
	case 9: i_ptr->flags2 |= TR2_RES_BLIND; break;
	case 10: i_ptr->flags2 |= TR2_RES_DISEN; break;
    }
}

/*
 * Give an item one of the "less powerful resistances"
 */		   
static void give_1_lo_resist(inven_type *i_ptr)
{
    switch (randint(10)) {
	case 1: i_ptr->flags2 |= TR2_SUST_STR; break;
	case 2: i_ptr->flags2 |= TR2_SUST_INT; break;
	case 3: i_ptr->flags2 |= TR2_SUST_WIS; break;
	case 4: i_ptr->flags2 |= TR2_SUST_DEX; break;
	case 5: i_ptr->flags2 |= TR2_SUST_CON; break;
	case 6: i_ptr->flags2 |= TR2_SUST_CHR; break;
	case 7: i_ptr->flags2 |= TR2_RES_ACID; break;
	case 8: i_ptr->flags2 |= TR2_RES_ELEC; break;
	case 9: i_ptr->flags2 |= TR2_RES_FIRE; break;
	case 10: i_ptr->flags2 |= TR2_RES_COLD; break;
    }
}


/*
 * Charge a wand  XXX Redo this cleanly!
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
 * Charge a staff  XXX Redo this cleanly!
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
 * The base "chance" of being "good" increases with the "level" parameter,
 * which is usually derived from the dungeon level.  Note that the "good"
 * and "great" flags over-ride this parameter somewhat.
 *			
 * If "okay" is true, this routine will have a small chance of turning
 * the object into an artifact.  If "good" is true, the object is guaranteed
 * to be good.  If "great" is true, it is guaranteed to be "great".  Note
 * that even if "good" or "great" are false, there is still a chance for
 * good stuff to be created, even artifacts (if "okay" is TRUE).  Note that
 * if "great" is true, then objects get an "extra try" to become an artifact.
 *
 * Hack -- inherited from a previous version is code which says that some
 * weapons, all missiles, and all crowns have a "higher than normal" chance
 * of being "excellent".  This is done by "increasing" the "special" variable.
 *
 * For weapons/armor, the "cursed" chance only kicks in AFTER the "chance"
 * roll fails, so weapons/armor have a pretty steady 10 percent chance of
 * being cursed.  But "maximally magic" weapons/armor is "excellent" almost
 * 13 percent of the time.  For rings/amulets, the "cursed" chance over-rides
 * the "chance" setting, so there are a lot of cursed items.
 *
 * Note that the "k_list" has been rebuilt to remove the old problems
 * with multiple "similar" objects.
 *
 * In addition, this function cleans up some floating bizarreness,
 * like the "fake-torches" in the dungeon, etc.  Likewise, the Chests
 * get "trapped" here.  Store-bought chests may be really good.  Oh, and
 * wands and staffs get "charged" here.
 *
 * Hack -- due to some annoying aspects of the "special artifact" metaphor
 * (lites, rings, amulets), those artifacts are actualy handled elsewhere.
 * It might be possible to "convert" rings of speed into the artifact rings,
 * and amulets of the magi into artifact amulets, and then perhaps to have
 * non-artifact phials/stars/arkenstones created in the dungeon.  Or even
 * let lanterns get turned into artifact lites.  But this would still require
 * specialized "conversion" code.  And note that the "place_good()" function
 * never places any lites, rings, or amulets.
 */
void apply_magic(inven_type *i_ptr, int level, bool okay, bool good, bool great)
{
    int chance, special, cursed, crown;

    /* Extract the "chance" of "goodness" */
    chance = OBJ_BASE_MAGIC + level;
    if (chance > OBJ_BASE_MAX) chance = OBJ_BASE_MAX;

    /* Extract the "chance" of "greatness" (approx range 3-18 percent) */
    special = (10 * chance) / OBJ_DIV_SPECIAL;

    /* Extract the "chance" of ickiness (approx range 11-54 percent) */
    cursed = (10 * chance) / OBJ_DIV_CURSED;


    /* Apply magic (good or bad) according to type */
    switch (i_ptr->tval) {


      case TV_DRAG_ARMOR:

	/* Good */
	if (good || magik(chance)) {

	    /* Enchant */
	    i_ptr->toac += randint(3) + m_bonus(0, 5, level);

	    /* Perhaps an artifact */
	    if (great || magik(special)) {

		/* Roll for artifact */
		if (okay && make_artifact(i_ptr)) break;
		if (great && okay && make_artifact(i_ptr)) break;

		/* Even better */
		i_ptr->toac += m_bonus(0, 10, level);

		/* Sometimes add an extra "resist" */
		if (randint(20) == 1) give_1_hi_resist(i_ptr);
		if (randint(5) == 1) give_1_lo_resist(i_ptr);
	    }

	    /* Hack -- adjust cost for "toac" */
	    i_ptr->cost += i_ptr->toac * 500L;
	}

	rating += 30;
	if (wizard || peek) msg_print("Dragon Scale Mail");

	break;


      case TV_HARD_ARMOR:
      case TV_SOFT_ARMOR:
      case TV_SHIELD:

	/* Good */
	if (good || magik(chance)) {

	    /* Enchant */
	    i_ptr->toac += randint(3) + m_bonus(0, 5, level);

	    /* Try for artifacts */
	    if (great || magik(special)) {

		/* Try for artifact */
		if (okay && make_artifact(i_ptr)) return;
		if (great && okay && make_artifact(i_ptr)) break;
		if (great && okay && make_artifact(i_ptr)) break;
		if (great && okay && make_artifact(i_ptr)) break;

		/* Hack -- Try for "Robes of the Magi" */
		if ((i_ptr->tval == TV_SOFT_ARMOR) &&
		    (i_ptr->sval == SV_ROBE) &&
		    (randint(30) == 1)) {

		    i_ptr->flags2 |= (TR2_RES_ELEC | TR2_RES_COLD | 
				      TR2_RES_ACID | TR2_RES_FIRE |
				      TR2_HOLD_LIFE |
				      TR2_SUST_STR | TR2_SUST_DEX |
				      TR2_SUST_CON | TR2_SUST_INT |
				      TR2_SUST_WIS | TR2_SUST_CHR);
		    i_ptr->flags3 |= (TR3_IGNORE_FIRE | TR3_IGNORE_COLD |
				      TR3_IGNORE_ELEC | TR3_IGNORE_ACID);
		    i_ptr->toac += 10 + randint(5);
		    i_ptr->cost = 10000L + (i_ptr->toac * 100);
		    give_1_hi_resist(i_ptr);
		    i_ptr->name2 = EGO_MAGI;

		    rating += 30;
		    if (wizard || peek) msg_print("Robe of the Magi");

		    break;
		}

		/* Make it "Excellent" */
		switch (randint(9)) {

		  case 1:
		    i_ptr->flags2 |= (TR2_RES_ELEC | TR2_RES_COLD |
				      TR2_RES_ACID | TR2_RES_FIRE);
		    i_ptr->flags3 |= (TR3_IGNORE_ELEC | TR3_IGNORE_COLD |
				      TR3_IGNORE_ACID | TR3_IGNORE_FIRE);
		    if (randint(3) == 1) {
			i_ptr->flags1 |= TR1_STEALTH;
			i_ptr->pval = randint(3);
			i_ptr->toac += 15;
			i_ptr->cost += 15000L;
			give_1_hi_resist(i_ptr);
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

	/* Cursed armor */
	else if (magik(cursed)) {
	    i_ptr->flags3 |= TR3_CURSED;
	    i_ptr->toac = 0 - randint(5) - m_bonus(0, 10, level);
	}

	break;


      case TV_GLOVES:

	/* Good */
	if (good || magik(chance)) {

	    /* Make it better */
	    i_ptr->toac += randint(3) + m_bonus(0, 10, level);

	    /* Apply more magic */
	    if (great || magik(special)) {

		/* Roll for artifact */
		if (okay && make_artifact(i_ptr)) return;
		if (great && okay && make_artifact(i_ptr)) break;

		/* Give a random resistance */
		give_1_lo_resist(i_ptr);
		
		/* Make it excellent */
		switch (randint(10)) {

		  case 1: case 2: case 3:
		    i_ptr->flags2 |= (TR2_FREE_ACT);
		    i_ptr->cost += 1000L;
		    i_ptr->name2 = EGO_FREE_ACTION;
		    rating += 11;
		    if (peek) msg_print("Gloves of Free action");
		    break;

		  case 4: case 5: case 6:
		    i_ptr->flags3 |= (TR3_SHOW_MODS);
		    i_ptr->tohit += 1 + randint(4);
		    i_ptr->todam += 1 + randint(4);
		    i_ptr->cost += (i_ptr->tohit + i_ptr->todam) * 250;
		    i_ptr->name2 = EGO_SLAYING;
		    rating += 17;
		    if (peek) msg_print("Gloves of Slaying");
		    break;

		  case 7: case 8: case 9:
		    i_ptr->flags1 |= (TR1_DEX);
		    i_ptr->pval = 2 + randint(2);	/* +N DEX */
		    i_ptr->cost += (i_ptr->pval) * 400;
		    i_ptr->name2 = EGO_AGILITY;
		    rating += 14;
		    if (peek) msg_print("Gloves of Agility");
		    break;

		  case 10:
		    i_ptr->flags1 |= (TR1_STR);
		    i_ptr->flags3 |= (TR3_SHOW_MODS | TR3_HIDE_TYPE);
		    i_ptr->pval = 1 + randint(4);	/* +N STR */
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

	/* Cursed gloves */
	else if (magik(cursed)) {

	    /* Cursed */
	    i_ptr->flags3 |= TR3_CURSED;
	    i_ptr->toac = 0 - (m_bonus(1, 20, level));

	    /* Permanently damaged */
	    if (magik(special)) {

		/* Permanently damaged */
		i_ptr->cost = 0L;

		/* Strength or Dexterity */
		if (randint(2) == 1) {
		    i_ptr->flags1 |= TR1_DEX;
		    i_ptr->pval = 0 - (randint(3) + m_bonus(0, 10, level));
		    i_ptr->name2 = EGO_CLUMSINESS;
		}
		else {
		    i_ptr->flags1 |= TR1_STR;
		    i_ptr->pval = 0 - (randint(3) + m_bonus(0, 10, level));
		    i_ptr->name2 = EGO_WEAKNESS;
		}
	    }
	}

	break;


      case TV_BOOTS:

	/* Good */
	if (good || magik(chance)) {

	    /* Make it better */
	    i_ptr->toac += randint(3) + m_bonus(0, 10, level);

	    /* Apply more magic */
	    if (great || magik(special)) {

		/* Roll for artifact */
		if (okay && make_artifact(i_ptr)) return;
		if (great && okay && make_artifact(i_ptr)) break;

		/* Make it "excellent" */
		switch (randint(24)) {

		  case 1:
		    i_ptr->flags1 |= TR1_SPEED;
		    i_ptr->name2 = EGO_SPEED;

		    /* Base speed */
		    i_ptr->pval = m_bonus(1, 5, level);

		    /* Super-charge the boots */
		    while (randint(2) == 1) i_ptr->pval += 1;
		    
		    /* Calculate the cost */
		    i_ptr->cost += (200000L + (i_ptr->pval * 40000L));
		    
		    /* Increase the rating */
		    rating += (25 + i_ptr->pval);
		    if (wizard || peek) msg_print("Boots of Speed");

		    break;

		  case 2: case 3: case 4: case 5:
		    i_ptr->flags2 |= (TR2_FREE_ACT);
		    i_ptr->cost += 500;
		    i_ptr->cost *= 2;
		    i_ptr->name2 = EGO_FREE_ACTION;
		    rating += 15;
		    break;

		  case 6: case 7: case 8: case 9:
		  case 10: case 11: case 12: case 13:
		    i_ptr->flags1 |= TR1_STEALTH;
		    i_ptr->pval = randint(3);	/* +N Stealth */
		    i_ptr->cost += 500;
		    i_ptr->name2 = EGO_STEALTH;
		    rating += 16;
		    break;

		  default:
		    i_ptr->flags3 |= TR3_FEATHER;
		    i_ptr->cost += 250;
		    i_ptr->name2 = EGO_SLOW_DESCENT;
		    rating += 7;
		    break;
		}
	    }
	}

	/* Cursed */
	else if (magik(cursed)) {

	    /* Cursed */
	    i_ptr->flags3 |= TR3_CURSED;
	    i_ptr->toac = 0 - m_bonus(1, 10, level);

	    /* Permanent damage */
	    if (magik(special)) {

		/* Permanent damage */
		i_ptr->cost = 0L;

		/* More damage */
		i_ptr->toac -= (m_bonus(1, 10, level));

		/* Pick some damage */
		switch (randint(3)) {
		    case 1:
			i_ptr->flags1 |= TR1_SPEED;
			i_ptr->pval = -10;
			i_ptr->name2 = EGO_SLOWNESS;
			break;
		    case 2:
			i_ptr->flags3 |= TR3_AGGRAVATE;
			i_ptr->name2 = EGO_NOISE;
			break;
		    case 3:
			i_ptr->weight = i_ptr->weight * 5;
			i_ptr->name2 = EGO_GREAT_MASS;
			break;
		}
	    }
	}

	break;
	

      case TV_HELM:

	crown = FALSE;

	/* Hack -- crowns are "more magical" */
	if ((i_ptr->sval == SV_IRON_CROWN) ||
	    (i_ptr->sval == SV_GOLDEN_CROWN) ||
	    (i_ptr->sval == SV_JEWELED_CROWN)) {

	    /* Hack -- extra "goodness" based on cost */
	    chance += i_ptr->cost / 100L;

	    /* Hack -- improve the chance for "greatness" */
	    special = special * 2;

	    /* Even more effects below */
	    crown = TRUE;
	}

	/* Apply some magic */
	if (good || magik(chance)) {

	    /* Make it better */
	    i_ptr->toac += randint(3) + m_bonus(0, 10, level);

	    /* Apply more magic */
	    if (great || magik(special)) {

		/* Roll for artifact */
		if (okay && make_artifact(i_ptr)) return;
		if (great && okay && make_artifact(i_ptr)) break;

		/* Process "helms" */
		if (!crown) {

		    /* Make it "excellent" */
		    switch (randint(14)) {

		      case 1: case 2:
			i_ptr->flags1 |= TR1_INT;
			i_ptr->flags2 |= TR2_SUST_INT;
			i_ptr->pval = randint(2);	/* +N INT */
			i_ptr->cost += i_ptr->pval * 500;
			i_ptr->name2 = EGO_INTELLIGENCE;
			rating += 13;
			if (peek) msg_print("Helm of Intelligence");
			break;

		      case 3: case 4: case 5:
			i_ptr->flags1 |= TR1_WIS;
			i_ptr->flags2 |= TR2_SUST_WIS;
			i_ptr->pval = randint(2);	/* +N Wis */
			i_ptr->cost += i_ptr->pval * 500;
			i_ptr->name2 = EGO_WISDOM;
			rating += 13;
			if (peek) msg_print("Helm of Wisdom");
			break;

		      case 6: case 7: case 8: case 9:
			i_ptr->flags1 |= TR1_INFRA;
			i_ptr->pval = 1 + randint(4);	/* +N Infra */
			i_ptr->cost += i_ptr->pval * 250;
			i_ptr->name2 = EGO_INFRAVISION;
			rating += 11;
			if (peek) msg_print("Helm of Infravision");
			break;

		      case 10: case 11:
			i_ptr->flags2 |= (TR2_RES_LITE);
			i_ptr->flags3 |= (TR3_LITE);
			i_ptr->cost += 500;
			i_ptr->name2 = EGO_LITE;
			rating += 6;
			if (peek) msg_print("Helm of Light");
			break;

		      case 12: case 13:
			i_ptr->flags2 |= TR2_RES_BLIND;
			i_ptr->flags3 |= TR3_SEE_INVIS;
			i_ptr->cost += 1000;
			i_ptr->name2 = EGO_SEEING;
			rating += 8;
			if (peek) msg_print("Helm of Seeing");
			break;

		     default: /* case 14: */
			i_ptr->flags3 |= TR3_TELEPATHY;
			i_ptr->cost += 50000L;
			i_ptr->name2 = EGO_TELEPATHY;
			rating += 20;
			if (peek) msg_print("Helm of Telepathy");
			break;
		    }
		}

		/* Process "crowns" */
		else {

		    /* Make it "excellent" */
		    switch (randint(6)) {

		      case 1:
			i_ptr->flags1 |= (TR1_STR | TR1_DEX | TR1_CON);
			i_ptr->flags2 |= (TR2_FREE_ACT | TR2_SUST_STR |
					  TR2_SUST_DEX | TR2_SUST_CON);
			i_ptr->pval = randint(3);	/* +N STR/DEX/CON */
			i_ptr->cost += 1000 + i_ptr->pval * 500;
			i_ptr->name2 = EGO_MIGHT;
			rating += 19;
			if (peek) msg_print("Crown of Might");
			break;

		      case 2:
			i_ptr->flags1 |= (TR1_CHR | TR1_WIS);
			i_ptr->flags2 |= (TR2_SUST_CHR | TR2_SUST_WIS);
			i_ptr->pval = randint(3);	/* +N WIS/CHR */
			i_ptr->cost += 1000 + i_ptr->pval * 500;
			i_ptr->name2 = EGO_LORDLINESS;
			rating += 17;
			if (peek) msg_print("Crown of Lordliness");
			break;

		      case 3:
			i_ptr->flags1 |= (TR1_INT);
			i_ptr->flags2 |= (TR2_RES_ELEC | TR2_RES_COLD |
					  TR2_RES_ACID | TR2_RES_FIRE |
					  TR2_SUST_INT);
			i_ptr->flags3 |= (TR3_IGNORE_ELEC | TR3_IGNORE_COLD |
					  TR3_IGNORE_ACID | TR3_IGNORE_FIRE);
			i_ptr->pval = randint(3);	/* +N INT */
			i_ptr->cost += 3000 + i_ptr->pval * 500;
			i_ptr->name2 = EGO_MAGI;
			rating += 15;
			if (peek) msg_print("Crown of the Magi");
			break;

		      case 4:
			i_ptr->flags1 |= TR1_CHR;
			i_ptr->flags2 |= TR2_SUST_CHR;
			i_ptr->pval = randint(4);	/* +N CHR */
			i_ptr->cost += 750;
			i_ptr->name2 = EGO_BEAUTY;
			rating += 8;
			if (peek) msg_print("Crown of Beauty");
			break;

		      case 5:
			i_ptr->flags1 |= (TR1_SEARCH);
			i_ptr->flags3 |= (TR3_SEE_INVIS);
			i_ptr->pval = 5 * (1 + randint(4));	/* +N Search */
			i_ptr->cost += 1000 + i_ptr->pval * 100;
			i_ptr->name2 = EGO_SEEING;
			rating += 8;
			if (peek) msg_print("Crown of Seeing");
			break;

		      case 6:
			i_ptr->flags3 |= TR3_REGEN;
			i_ptr->cost += 1500;
			i_ptr->name2 = EGO_REGENERATION;
			rating += 10;
			if (peek) msg_print("Crown of Regeneration");
			break;
		    }
		}
	    }
	}

	/* Cursed */
	else if (magik(cursed)) {

	    /* Cursed */
	    i_ptr->flags3 |= TR3_CURSED;
	    i_ptr->toac = 0 - m_bonus(1, 10, level);

	    /* Permanent damage */
	    if (magik(special)) {

		/* Permanent damage */
		i_ptr->cost = 0L;

		/* More damage */
		i_ptr->toac -= (-m_bonus(1, 10, level));

		/* Choose some damage */
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
		    i_ptr->flags1 |= TR1_CHR;
		    i_ptr->pval = -randint(5);
		    i_ptr->name2 = EGO_UGLINESS;
		    break;
		}
	    }
	}
	break;


      case TV_CLOAK:

	/* Apply some magic */
	if (good || magik(chance)) {

	    /* Make it better */
	    i_ptr->toac += randint(3) + m_bonus(0, 20, level);

	    /* Apply more magic */
	    if (great || magik(special)) {

		/* Roll for artifact */
		if (okay && make_artifact(i_ptr)) return;
		if (great && okay && make_artifact(i_ptr)) break;

		/* Make it "excellent" */
		if (randint(2) == 1) {
		    i_ptr->flags3 |= (TR3_IGNORE_ACID);
		    i_ptr->toac += m_bonus(0, 10, level) + (5 + randint(3));
		    i_ptr->cost += 250L;
		    i_ptr->name2 = EGO_PROTECTION;
		    give_1_lo_resist(i_ptr);
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

	/* Cursed */
	else if (magik(cursed)) {

	    /* Cursed */
	    i_ptr->flags3 |= TR3_CURSED;
	    i_ptr->toac = 0 - m_bonus(1, 10, level);

	    /* Permanent damage */
	    if (magik(special)) {

		/* Permanent damage */
		i_ptr->cost = 0L;

		/* Choose some damage */
		switch (randint(3)) {
		    case 1:
			i_ptr->name2 = EGO_IRRITATION;
			i_ptr->flags3 |= (TR3_SHOW_MODS | TR3_AGGRAVATE);
			i_ptr->toac -= m_bonus(1, 10, level);
			i_ptr->tohit -= m_bonus(1, 10, level);
			i_ptr->todam -= m_bonus(1, 10, level);
			break;
		    case 2:
			i_ptr->name2 = EGO_VULNERABILITY;
			i_ptr->toac -= m_bonus(10, 20, level + 50);
			break;
		    case 3:
			i_ptr->name2 = EGO_ENVELOPING;
			i_ptr->flags3 |= (TR3_SHOW_MODS);
			i_ptr->toac -= m_bonus(1, 10, level);
			i_ptr->tohit -= m_bonus(2, 15, level + 10);
			i_ptr->todam -= m_bonus(2, 15, level + 10);
			break;
		}
	    }
	}
	break;


      case TV_DIGGING:

	/* Apply some magic */
	if (good || magik(chance)) {

	    /* Add in a digging bonus */
	    i_ptr->pval += m_bonus(1, 5, level);

	    /* Charge for the digging bonus */
	    i_ptr->cost += i_ptr->pval * 50;

	    /* Hack -- "shovels of fire" */
	    if (great || magik(special)) {
		i_ptr->flags1 |= (TR1_BRAND_FIRE);
		i_ptr->flags3 |= (TR3_IGNORE_FIRE);
		i_ptr->tohit += 5;
		i_ptr->todam += 5;
		i_ptr->cost += 2000L;
		i_ptr->name2 = EGO_FIRE;
		rating += 15;
		if (peek) msg_print("Digger of Fire");
	    }
	}

	/* Cursed shovels */
	else if (magik(cursed)) {

	    /* Cursed */
	    i_ptr->flags3 |= TR3_CURSED;
	    i_ptr->tohit = 0 - randint(5) - m_bonus(1, 10, level);
	    i_ptr->todam = 0 - randint(5) - m_bonus(1, 10, level);

	    /* Permanent damage */
	    if (magik(special)) {
		i_ptr->pval = 0 - m_bonus(1, 15, level);
		i_ptr->cost = 0L;
	    }
	}
	
	break;


      case TV_HAFTED:
      case TV_POLEARM:
      case TV_SWORD:

	/* Apply some magic */
	if (good || magik(chance)) {

	    /* Make it better */
	    i_ptr->tohit += randint(3) + m_bonus(0, 10, level);
	    i_ptr->todam += randint(3) + m_bonus(0, 10, level);

	    /* Hack -- improve the "special" chance */
	    special = special * 3 / 2;
	    
	    /* Make it "excellent" */
	    if (great || magik(special)) {

		/* Roll for artifacts */
		if (okay && make_artifact(i_ptr)) return;
		if (great && okay && make_artifact(i_ptr)) break;
		if (great && okay && make_artifact(i_ptr)) break;
		if (great && okay && make_artifact(i_ptr)) break;

		/* Hack -- Roll for whips of fire */
		if ((i_ptr->tval == TV_HAFTED) &&
		    (i_ptr->sval == SV_WHIP) &&
		    (randint(2) == 1)) {

		    i_ptr->flags1 |= (TR1_BRAND_FIRE);
		    i_ptr->flags3 |= (TR3_IGNORE_FIRE);

		    /* Better stats */
		    i_ptr->tohit += 5;
		    i_ptr->todam += 5;

		    /* Basic cost increase */
		    i_ptr->cost += 2000L;

		    /* this should allow some WICKED whips -CFT */
		    while (randint(5 * (int)i_ptr->dd) == 1) {
			i_ptr->dd++;
			i_ptr->cost += 2500;
			i_ptr->cost *= 2;
		    }

		    i_ptr->name2 = EGO_FIRE;

		    rating += 20;
		    if (peek) msg_print("Whip of Fire");

		    break;
		}

		/* Make it "excellent" */
		switch (randint(30)) {

		  case 1:
		    i_ptr->flags1 |= (TR1_SLAY_DEMON | TR1_WIS |
				      TR1_SLAY_UNDEAD | TR1_SLAY_EVIL);
		    i_ptr->flags3 |= (TR3_BLESSED | TR3_SEE_INVIS);
		    i_ptr->pval = randint(4);  /* Wisdom bonus */
		    i_ptr->tohit += 5;
		    i_ptr->todam += 5;
		    i_ptr->toac += randint(4);

		    /* Obsolete Hack -- Pick "Sustain" based on "Pval" */
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
			i_ptr->pval = m_bonus(0, 2, level);
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
		    i_ptr->tohit += 3 + randint(5);
		    i_ptr->todam += 3 + randint(5);
		    i_ptr->pval = 1;
		    i_ptr->cost += 10000L;
		    i_ptr->cost *= 2;
		    if (randint(500) == 1) {
			i_ptr->pval++;
			i_ptr->cost += 10000L;
			i_ptr->cost *= 2;
		    }
		    i_ptr->name2 = EGO_WEST;
		    give_1_lo_resist(i_ptr);
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
		    give_1_lo_resist(i_ptr);
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

	/* Cursed Weapons */
	else if (magik(cursed)) {
	
	    /* Cursed */
	    i_ptr->flags3 |= TR3_CURSED;
	    i_ptr->tohit = 0 - randint(5) - m_bonus(1, 20, level);
	    i_ptr->todam = 0 - randint(5) - m_bonus(1, 20, level);

	    /* Permanently cursed Weapon of Morgul */
	    if ((level > (20 + randint(15))) && (randint(10) == 1)) {
		i_ptr->flags3 |= (TR3_HEAVY_CURSE | TR3_AGGRAVATE | TR3_SEE_INVIS);
		i_ptr->tohit -= 15;
		i_ptr->todam -= 15;
		i_ptr->toac = -10;
		i_ptr->weight += 100;
		i_ptr->cost = 0L;
		i_ptr->name2 = EGO_MORGUL;
	    }
	}
	break;


      case TV_BOW:

	/* Apply some magic */
	if (good || magik(chance)) {

	    /* Make it better */
	    i_ptr->tohit += randint(3) + m_bonus(0, 10, level);
	    i_ptr->todam += randint(3) + m_bonus(0, 10, level);

	    /* Apply more magic */
	    if (great || magik(special)) {

		/* Make an artifact */
		if (okay && make_artifact(i_ptr)) break;
		if (great && okay && make_artifact(i_ptr)) break;

		/* Make it "excellent" */
		switch (randint(20)) {

		  case 1:
		    i_ptr->flags3 |= TR3_XTRA_MIGHT;
		    i_ptr->tohit += 5;
		    i_ptr->todam += 10;
		    i_ptr->name2 = EGO_EXTRA_MIGHT;
		    i_ptr->cost += 10000L;
		    rating += 20;
		    if (peek) msg_print("Bow of Extra Might");
		    break;

		  case 2:
		    i_ptr->flags3 |= TR3_XTRA_SHOTS;
		    i_ptr->tohit += 10;
		    i_ptr->todam += 3;
		    i_ptr->name2 = EGO_EXTRA_SHOTS;
		    i_ptr->cost += 10000L;
		    rating += 20;
		    if (peek) msg_print("Bow of Extra Shots");
		    break;

		  case 3: case 4: case 5: case 6:
		    i_ptr->tohit += 5;
		    i_ptr->todam += 12;
		    i_ptr->name2 = EGO_MIGHT;
		    i_ptr->cost += 1000L;
		    rating += 11;
		    if (peek) msg_print("Bow of Might");
		    break;

		  case 7: case 8: case 9: case 10:
		    i_ptr->tohit += 12;
		    i_ptr->todam += 5;
		    i_ptr->name2 = EGO_ACCURACY;
		    i_ptr->cost += 1000L;
		    rating += 11;
		    if (peek) msg_print("Bow of Accuracy");
		    break;

		  case 11: case 12: case 13: case 14:
		    i_ptr->tohit += m_bonus(0, 5, level);
		    i_ptr->todam += m_bonus(0, 5, level);
		    break;

		  case 15: case 16: case 17:
		    i_ptr->tohit += m_bonus(0, 5, level);
		    break;

		  default: /* case 18: case 19: case 20: */
		    i_ptr->todam += m_bonus(0, 5, level);
		    break;
		}
	    }
	}

	/* Cursed bows */
	else if (magik(cursed)) {
	    i_ptr->flags3 |= TR3_CURSED;
	    i_ptr->tohit = 0 - m_bonus(5, 30, level);
	    i_ptr->todam = 0 - m_bonus(5, 20, level);
	}
	
	break;


      case TV_BOLT:
      case TV_ARROW:
      case TV_SHOT:

	/* Apply some magic */
	if (good || magik(chance)) {

	    /* Make it better */
	    i_ptr->tohit += randint(5) + m_bonus(0, 15, level);
	    i_ptr->todam += randint(5) + m_bonus(0, 15, level);

	    /* Hack -- improve the "special" chance */
	    special = special * 5 / 2;
	    
	    /* Apply more magic */
	    if (great || magik(special)) {

		/* Make it "excellent" */
		switch (randint(11)) {

		  case 1: case 2: case 3:
		    i_ptr->name2 = EGO_WOUNDING;
		    i_ptr->tohit += 5;
		    i_ptr->todam += 5;
		    i_ptr->dd += 1;	/* Extra die of damage */
		    i_ptr->cost += 30;
		    rating += 5;
		    break;

		  case 4: case 5:
		    i_ptr->name2 = EGO_FIRE;
		    i_ptr->flags1 |= (TR1_BRAND_FIRE);
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
		    i_ptr->name2 = EGO_SLAYING;
		    i_ptr->tohit += 10;
		    i_ptr->todam += 10;
		    i_ptr->dd += 2; /* two extra die of damage */
		    i_ptr->cost += 45;
		    rating += 10;
		    break;
		}
	    }

	    /* Hack -- Very special arrows */
	    while (magik(special)) {
		i_ptr->dd += 1;		/* Extra die of damage */
		i_ptr->cost += i_ptr->dd * 5;
	    }
	}

	/* Cursed missiles */
	else if (magik(cursed)) {

	    /* Cursed missiles */
	    i_ptr->flags3 |= TR3_CURSED;
	    i_ptr->tohit = 0 - randint(10) - m_bonus(5, 25, level);
	    i_ptr->todam = 0 - randint(10) - m_bonus(5, 25, level);

	    /* Permanently damaged missiles */
	    if (magik(special)) {
		i_ptr->tohit -= 20;
		i_ptr->todam -= 20;
		i_ptr->cost = 0L;
		i_ptr->name2 = EGO_BACKBITING;
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
	    }
	    break;

	  /* Ring of Speed! */
	  case SV_RING_SPEED:

	    /* Cursed Ring */
	    if (magik(cursed)) {
		i_ptr->flags3 |= TR3_CURSED;
		i_ptr->pval = 0 - randint(25);
		i_ptr->cost = 0L;
		break;
	    }

	    /* Base speed bonus */
	    i_ptr->pval = m_bonus(1, 6, level);

	    /* Super-charge the ring */
	    while (randint(3) == 1) i_ptr->pval += 2;

	    /* Calculate the Cost */
	    i_ptr->cost = 100000L + (i_ptr->pval * 20000L);

	    /* Rating boost */
	    rating += (25 + i_ptr->pval);
	    if (peek) msg_print("Ring of Speed");
	    break;

	  /* Searching */
	  case SV_RING_SEARCHING:
	    i_ptr->pval = 5 * m_bonus(1, 10, level);
	    i_ptr->cost += i_ptr->pval * 30;
	    if (magik(cursed)) {
		i_ptr->flags3 |= TR3_CURSED;
		i_ptr->pval = -i_ptr->pval;
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
		i_ptr->flags3 |= TR3_CURSED;
		i_ptr->todam = 0 - i_ptr->todam;
		i_ptr->cost = 0L;
	    }
	    break;

	  /* Increase To-Hit */
	  case 20:
	    i_ptr->tohit = m_bonus(1, 10, level);
	    i_ptr->tohit += 3 + randint(10);
	    i_ptr->cost += i_ptr->tohit * 100;
	    if (magik(cursed)) {
		i_ptr->flags3 |= TR3_CURSED;
		i_ptr->tohit = 0 - i_ptr->tohit;
		i_ptr->cost = 0L;
	    }
	    break;

	  /* Protection */
	  case 21:
	    i_ptr->toac = m_bonus(0, 10, level);
	    i_ptr->toac += 4 + randint(5);
	    i_ptr->cost += i_ptr->toac * 100;
	    if (magik(cursed)) {
		i_ptr->flags3 |= TR3_CURSED;
		i_ptr->toac = 0 - i_ptr->toac;
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
		i_ptr->flags3 |= TR3_CURSED;
		i_ptr->tohit = 0 - i_ptr->tohit;
		i_ptr->todam = 0 - i_ptr->todam;
		i_ptr->cost = 0L;
	    }
	    break;
	}
	break;

      case TV_AMULET:

	if ((i_ptr->sval == SV_AMULET_WISDOM) ||
	    (i_ptr->sval == SV_AMULET_CHARISMA)) {
	    i_ptr->pval = m_bonus(1, 5, level);
	    i_ptr->cost += i_ptr->pval * 100;
	    if (magik(cursed)) {
		i_ptr->flags3 |= TR3_CURSED;
		i_ptr->pval = -i_ptr->pval;
		i_ptr->cost = 0L;
	    }
	}
	else if (i_ptr->sval == SV_AMULET_SEARCHING) {
	    i_ptr->pval = 5 * (randint(3) + m_bonus(0, 8, level));
	    i_ptr->cost += 20 * i_ptr->pval;
	    if (magik(cursed)) {
		i_ptr->flags3 |= TR3_CURSED;
		i_ptr->pval = -i_ptr->pval;
		i_ptr->cost = 0L;
	    }
	}
	else if (i_ptr->sval == SV_AMULET_THE_MAGI) {
	    rating += 25;
	    i_ptr->pval = 5 * (randint(2) + m_bonus(0, 10, level));
	    i_ptr->toac = randint(4) + m_bonus(0, 6, level);
	    i_ptr->cost += 20 * i_ptr->pval + 50 * i_ptr->toac;
	}
	else if (i_ptr->sval == SV_AMULET_DOOM) {
	    i_ptr->flags3 |= TR3_CURSED;
	    i_ptr->pval = 0 - randint(5) - m_bonus(2, 10, level);
	    i_ptr->toac = 0 - randint(3) - m_bonus(0, 6, level);
	    i_ptr->cost = 0L;
	}
	break;


      case TV_WAND:
	charge_wand(i_ptr);
	break;


      case TV_STAFF:
	charge_staff(i_ptr);
	break;


      case TV_LITE:

	/* Torches -- random fuel */
	if (i_ptr->sval == SV_LITE_TORCH) {
	    i_ptr->pval = randint(i_ptr->pval);
	}

	/* Lanterns -- random fuel */            
	if (i_ptr->sval == SV_LITE_LANTERN) {
	    i_ptr->pval = randint(i_ptr->pval);
	}

	break;


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
	    i_ptr->flags2 |= (CH2_PARALYSED | CH2_POISON | CH2_LOSE_STR | CH2_LOCKED);
	    break;
	  default:
	    i_ptr->flags2 |= (CH2_SUMMON | CH2_EXPLODE | CH2_LOCKED);
	    break;
	}

	break;
    }
}




/*
 * XXX Mega-Hack -- attempt to place one of the "Special Objects"
 *
 * XXX XXX XXX We rely on the fact that a grid cannot contain
 * both (1) a wall or (2) rubble or a closed/secret door.
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
    if (i_list[c_ptr->i_idx].tval >= TV_MIN_BLOCK) return (FALSE);


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
    if (i_list[c_ptr->i_idx].tval >= TV_MIN_BLOCK) return;


    /* Hack -- One in 500 chance of (maybe) placing a "Special Object" */
    if (rand_int(500) == 1) {
	if (special_place_object(y, x)) return;
    }


    /* Delete anything already there */
    delete_object(y, x);

    /* Hack -- no embedded chests */
    do {
	/* Pick an object based on the "object_level" */
	tmp = get_obj_num(object_level, FALSE);
    } while (opening_chest && (k_list[tmp].tval == TV_CHEST));

    /* Make it */
    cur_pos = i_pop();
    invcopy(&i_list[cur_pos], tmp);

    /* Let it be magic, perhaps even an artifact */
    apply_magic(&i_list[cur_pos], object_level, TRUE, FALSE, FALSE);

    /* Hack -- generate multiple missiles */
    switch (i_list[cur_pos].tval) {
	case TV_SPIKE: case TV_SHOT: case TV_ARROW: case TV_BOLT:
	    i_list[cur_pos].number = damroll(6,7);
    }

    /* Place it */
    i_list[cur_pos].iy = y;
    i_list[cur_pos].ix = x;
    c_ptr->i_idx = cur_pos;

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
 *
 * Really only called when MF2_GOOD monster dies, or scroll of
 * acquirement read, or "vault" is constructed.  Perhaps the
 * normal "place_object()" should occasionally call us...
 *
 * This routine uses "object_level" for the "generation level".
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
    if (i_list[c_ptr->i_idx].tval >= TV_MIN_BLOCK) return;


    /* Hack -- much higher chance of doing "Special Objects" */
    if (randint(10) == 1) {
	if (special_place_object(y, x)) return;
    }


    /* Pick a good "base object" */
    while (1) {

	/* Pick a random object, based on "object_level" */
	k_idx = get_obj_num((object_level + 10), TRUE);

	/* Examine the object */
	tv = k_list[k_idx].tval;
	sv = k_list[k_idx].sval;

	/* Rusty Chainmail is not good */
	if ((tv == TV_HARD_ARMOR) && (sv == SV_RUSTY_CHAIN_MAIL)) continue;

	/* Filthy Rags are not good */
	if ((tv == TV_SOFT_ARMOR) && (sv == SV_FILTHY_RAG)) continue;

	/* Broken daggers/swords are not good */
	if ((tv == TV_SWORD) && (sv == SV_BROKEN_DAGGER)) continue;
	if ((tv == TV_SWORD) && (sv == SV_BROKEN_SWORD)) continue;

	/* Normal weapons/armour are okay (except "shots" or "shovels") */
	if ((tv == TV_HELM) || (tv == TV_SHIELD) || (tv == TV_CLOAK) ||
	    (tv == TV_SWORD) || (tv == TV_HAFTED) || (tv == TV_POLEARM) ||
	    (tv == TV_BOW) || (tv == TV_BOLT) || (tv == TV_ARROW) ||
	    (tv == TV_HARD_ARMOR) || (tv == TV_SOFT_ARMOR) ||
	    (tv == TV_DRAG_ARMOR) || (tv == TV_BOOTS) || (tv == TV_GLOVES)) {
	    break;
	}

	/* XXX Hack -- High spell books are good.  Highest is great. */
	if (((tv == TV_MAGIC_BOOK) || (tv == TV_PRAYER_BOOK)) &&
	     (sv >= (great ? (SV_BOOK + 8) : (SV_BOOK + 4)))) {
	    break;
	}
    }


    /* Delete anything already there */
    delete_object(y, x);

    /* Make a new object, drop into dungeon */
    cur_pos = i_pop();
    invcopy(&i_list[cur_pos], k_idx);

    /* Hack -- generate multiple "decent" missiles */
    switch (i_list[cur_pos].tval) {
	case TV_ARROW: case TV_BOLT:
	    i_list[cur_pos].number = damroll(6,7);
    }

    /* Drop it into the dungeon */
    i_list[cur_pos].iy = y;
    i_list[cur_pos].ix = x;
    c_ptr->i_idx = cur_pos;

    /* Apply some good magic to the item.  Make a great item if requested. */
    apply_magic(&i_list[cur_pos], object_level, TRUE, TRUE, great);

    /* Help the rating */
    if (k_list[k_idx].level > dun_level) {

	/* Help the rating */
	rating += (k_list[k_idx].level - dun_level);

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
 * Create up to "num" objects near the given coordinates
 * Only really called by some of the "vault" routines.
 */
void random_object(int y, int x, int num)
{
    register int        i, j, k;

    /* Attempt to place 'num' objects */
    for (; num > 0; --num) {

	/* Try up to 11 spots looking for empty space */
	for (i = 0; i < 11; ++i) {

	    /* Pick a random location */
	    j = rand_spread(y, 2);
	    k = rand_spread(x, 3);

	    /* Require legal grid */
	    if (!in_bounds(j,k)) continue;
	    
	    /* Require "clean" floor space */
	    if (!clean_grid_bold(j,k)) continue;

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
 * Only really called by "scroll of *acquirement*"
 */
void special_random_object(int y, int x, int num)
{
    register int        i, j, k;

    /* Place them */
    for (; num > 0; --num) {

	/* Try up to 11 spots looking for empty space */
	for (i = 0; i < 12; ++i) {

	    /* Distance (once on player, once 3 away) */
	    int d = (i + 4) / 5;
	    
	    /* Pick a random spot */
	    j = rand_spread(y, d);
	    k = rand_spread(x, d);

	    /* Require legal grid */
	    if (!in_bounds(j, k)) continue;
	    
	    /* Must have a clean grid */
	    if (!clean_grid_bold(j, k)) continue;

	    /* Must have line of sight */
	    if (!los(y, x, j, k)) continue;
	    
	    /* Perhaps attempt to place a "Special Object" */
	    if (randint(5) == 1) {
		if (special_place_object(j, k)) break;
	    }

	    /* Place a "great" object */
	    place_good(j, k, TRUE);

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
    if (i_list[c_ptr->i_idx].tval >= TV_MIN_BLOCK) return;


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

    /* Hack -- no rubble under monsters */
    if (cave[y][x].m_idx > 1) return;
    
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
    char sym;
    cptr name;

    sym = r_ptr->r_char;
    if (sym != '$') return (0);
    
    name = r_ptr->name;
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
    if (i_list[c_ptr->i_idx].tval >= TV_MIN_BLOCK) return;



    /* Pick a Treasure variety */
    i = ((randint(object_level + 2) + 2) / 2) - 1;

    /* Apply "extra" magic */
    if (randint(GREAT_OBJ) == 1) {
	i += randint(object_level + 1);
    }

    /* Hack -- Creeping Coins only generate "themselves" */
    if (coin_type) i = coin_type;

    /* Do not create "illegal" Treasure Types */
    if (i >= MAX_GOLD) i = MAX_GOLD - 1;

    /* Delete the object under us (acidic gold?) */
    delete_object(y, x);

    /* Make it */
    cur_pos = i_pop();
    i_ptr = &i_list[cur_pos];
    invcopy(i_ptr, OBJ_GOLD_LIST + i);

    /* Determine the "cost" */
    i_ptr->cost += (8L * (long)randint((int)i_ptr->cost)) + randint(8);

    /* Place it in the dungeon */
    i_ptr->iy = y;
    i_ptr->ix = x;
    c_ptr->i_idx = cur_pos;

    /* Hack -- make sure "creeping coins" are not too valuable */
    if (coin_type) {
	i_ptr->cost = ((8L * (long)randint((int)k_list[OBJ_GOLD_LIST + i].cost))
		       + (i_ptr->cost)) >> 1;
    }

    /* Under the player */
    if (c_ptr->m_idx == 1) {
	msg_print("You feel something roll beneath your feet.");
    }
}


/*
 * An entry for the object allocator below
 */
typedef struct _kind_entry {
    u16b k_idx;		/* Object kind index */
    byte locale;		/* Base dungeon level */
    byte chance;		/* Rarity of occurance */
} kind_entry;


/*
 * Returns the array number of a random object
 * Uses the locale/chance info for distribution.
 */
int get_obj_num(int level, int good)
{
    register int i, j;

    /* Number of entries in the "k_sort" table */
    static u16b size = 0;

    /* The actual table of entries */
    static kind_entry *table = NULL;

    /* Number of entries at each locale */
    static u16b t_lev[256];

    /* Initialize the table */
    if (!size) {

	inven_kind *k_ptr;

	u16b aux[256];

	/* Clear the level counter and the aux array */
	for (i = 0; i < 256; i++) t_lev[i] = aux[i] = 0;

	/* Scan all of the objects */
	for (i = 0; i < MAX_K_IDX; i++) {

	    /* Get the i'th object */
	    k_ptr = &k_list[i];

	    /* Scan all of the locale/chance pairs */
	    for (j = 0; j < 4; j++) {

		/* Count valid pairs */
		if (k_ptr->chance[j]) {

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

	    /* Scan all of the locale/chance pairs */
	    for (j = 0; j < 4; j++) {

		/* Count valid pairs */
		if (k_ptr->chance[j]) {

		    int r, x, y, z;

		    /* Extract the chance/locale */                    
		    r = k_ptr->chance[j];
		    x = k_ptr->locale[j];

		    /* Skip entries preceding our locale */
		    y = (x > 0) ? t_lev[x-1] : 0;

		    /* Skip previous entries at this locale */
		    z = y + aux[x];

		    /* Load the table entry */
		    table[z].k_idx = i;
		    table[z].locale = x;
		    table[z].chance = r;

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

	/* The "good" parameter overwhelms "chance" requirements */
	if (good) break;

	/* Play the "chance game" */
	if (randint(table[i].chance) == 1) break;
    }

    /* Accept that object */
    return (j);
}



