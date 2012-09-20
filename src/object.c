/* File: object.c */

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
void delete_object_idx(int i_idx)
{
    /* One less item */
    i_max--;

    /* Compact the array */
    if (i_idx != i_max) {

        int iy, ix;

        /* Extract the "location" of the "moved object" */
        iy = i_list[i_max].iy;
        ix = i_list[i_max].ix;

        /* Update the cave record if necessary */
        if (cave[iy][ix].i_idx == i_max) cave[iy][ix].i_idx = i_idx;

        /* Compact, via structure copy */
        i_list[i_idx] = i_list[i_max];
    }

    /* Be cautious, wipe the "dead" object */
    invwipe(&i_list[i_max]);
}


/*
 * Deletes object from given location			-RAK-	
 */
void delete_object(int y, int x)
{
    cave_type *c_ptr;

    /* Refuse "illegal" locations */
    if (!in_bounds(y, x)) return;

    /* Find where it was */
    c_ptr = &cave[y][x];

    /* Nothing here?  Don't delete it */
    if (!(c_ptr->i_idx)) return;

    /* Kill the object */
    delete_object_idx(c_ptr->i_idx);

    /* There is nothing here */
    c_ptr->i_idx = 0;

    /* Forget the grid */
    if (c_ptr->info & GRID_MARK) {

        /* Forget the grid */
        c_ptr->info &= ~GRID_MARK;

        /* Mega-Hack -- see "update_map()" in "cave.c" */
        if (view_perma_grids && (c_ptr->info & GRID_GLOW)) {
            c_ptr->info |= GRID_MARK;
        }
    }

    /* Redraw the spot (may reset field mark) */
    lite_spot(y, x);
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
    int        i, j;
    cave_type *c_ptr;
    inven_type *i_ptr;
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
        for (i = 0; i < cur_hgt; i++) {
            for (j = 0; j < cur_wid; j++) {

                /* Get the location */
                c_ptr = &cave[i][j];

                /* Do not even consider empty grids */
                if (!(c_ptr->i_idx)) continue;

                /* Do not consider artifacts or stairs */
                if (!valid_grid(i,j)) continue;

                /* Get the object */
                i_ptr = &i_list[c_ptr->i_idx];

                /* Hack -- High level objects start out "immune" */
                if (k_list[i_ptr->k_idx].level > cur_lev) continue;

                /* Nearby objects start out "immune" */
                if (distance(i, j, py, px) < cur_dis) continue;

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

    /* Update stuff */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);
    p_ptr->update |= (PU_MONSTERS);

    /* Redraw */
    p_ptr->redraw |= (PR_MAP);
}




/*
 * Tighten up the object list
 */
void tighten_i_list()
{
    /* Compact if space is tight */
    if (i_max > MAX_I_IDX - 15) compact_objects();
}



/*
 * Delete all the items when player leaves the level
 */
void wipe_i_list()
{
    int i;

    /* Delete the existing objects (backwards!) */
    for (i = i_max - 1; i >= MIN_I_IDX; i--) {

        /* Hack -- preserve artifacts */
        if (p_ptr->preserve) {

            /* Hack -- Preserve artifacts */
            if (artifact_p(&i_list[i])) {

                /* Mega-Hack -- Preserve the artifact */
                v_list[i_list[i].name1].cur_num = 0;
            }
        }

        /* Delete the object */
        delete_object_idx(i);
    }

    /* Paranoia */
    i_max = MIN_I_IDX;
}


/*
 * Acquires and returns the index of a "free" item.
 */
int i_pop(void)
{
    /* Normal allocation */
    if (i_max < MAX_I_IDX) return (i_max++);

    /* Warning */
    msg_print("Too many objects!");

    /* Try compacting the objects */
    compact_objects();

    /* Risky allocation */
    if (i_max < MAX_I_IDX) return (i_max++);

    /* Mega-warning */
    msg_print("Unable to create new object!");

    /* Mega-Hack -- try not to crash */
    return (0);
}




/*
 * Determine an enchantment bonus based (loosely) on dungeon level
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
 * It *appears* that there is a peculiarity in randnor().  Empirical tests
 * have revealed that randnor(m, sd) returns values with a S.D. of (sd/1.25),
 * so we compensate for this by multiplying our desired sd by 1.25.  XXX XXX
 */
int m_bonus(int base, int limit, int level)
{
    int x, bonus_range, mean_bonus_per_level, std_dev, remainder;

    /* Paranoia -- enforce legal "range" */
    if (base >= limit) return (base);

    /* Don't let bonuses become "absurd" at very high levels. */
    if (level > MAG_MAX_LEVEL) level = MAG_MAX_LEVEL;

    /* Calculate the range of the bonus points to be allotted. */
    bonus_range = limit - base;

    /* The mean bonus is just proportional to the level. */
    mean_bonus_per_level = ((bonus_range * level) / MAG_MAX_LEVEL);


    /*
     * To avoid floating point but still provide a smooth distribution
     * of bonuses, award an extra bonus point sometimes. The probability
     * that a bonus point will be award is based upon the truncated
     * remainder from the calculation of mean_bonus_per_level. Specifically,
     * the chance of a bonus point being award is remainder/MAG_MAX_LEVEL.
     */

    /* Hack -- avoid floating point computations */
    remainder = ((bonus_range * level) % MAG_MAX_LEVEL);
    if (randint(MAG_MAX_LEVEL) <= remainder) ++mean_bonus_per_level;


    /*
     * The "standard" object will differ from the mean by 50%. So as the
     * mean increases, the bonus distribution widens. Since the mean was
     * calculated from the level, the bonus distribution widens with the
     * level as well.
     */

    std_dev = mean_bonus_per_level / 2;

    /* Now compensate for any "lost" remainder. */
    if ((mean_bonus_per_level % 2) && rand_int(2)) ++std_dev;

    /* XXX Hack -- repair apparent problem in "randnor()" */
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

    /* Enforce the maximum */
    if (x > limit) return (limit);

    /* Return the extracted value */
    return (x);
}




/*
 * Cheat -- describe a created object for the user
 */
static void inven_mention(inven_type *i_ptr)
{
    char buf[256];
    objdes_store(buf, i_ptr, FALSE);
    msg_print(buf);
}




/*
 * Help pick and create a "special" object.  This function is
 * a total hack, and it's distributions are also a total hack.
 */
static int make_artifact_special_aux(inven_type *i_ptr)
{
    int k_idx = 0;
    int name1 = 0;


    /* Hack -- pick an item */
    switch (randint(12)) {

      case 1:
        k_idx = OBJ_GALADRIEL;
        name1 = ART_GALADRIEL;
        break;

      case 2:
        k_idx = OBJ_ELENDIL;
        name1 = ART_ELENDIL;
        break;

      case 3:
        k_idx = OBJ_THRAIN;
        name1 = ART_THRAIN;
        break;

      case 4:
        k_idx = OBJ_CARLAMMAS;
        name1 = ART_CARLAMMAS;
        break;

      case 5:
        k_idx = OBJ_INGWE;
        name1 = ART_INGWE;
        break;

      case 6:
        k_idx = OBJ_DWARVES;
        name1 = ART_DWARVES;
        break;

      case 7:
        k_idx = OBJ_BARAHIR;
        name1 = ART_BARAHIR;
        break;

      case 8:
        k_idx = OBJ_TULKAS;
        name1 = ART_TULKAS;
        break;

      case 9:
        k_idx = OBJ_NARYA;
        name1 = ART_NARYA;
        break;

      case 10:
        k_idx = OBJ_NENYA;
        name1 = ART_NENYA;
        break;

      case 11:
        k_idx = OBJ_VILYA;
        name1 = ART_VILYA;
        break;

      case 12:
        k_idx = OBJ_POWER;
        name1 = ART_POWER;
        break;
    }


    /* Artifact already made */
    if (v_list[name1].cur_num) return (0);

    /* Artifact level */
    if (v_list[name1].level > dun_level) return (0);

    /* Artifact rarity */
    if (rand_int(v_list[name1].rarity) != 0) return (0);

    /* Object level */
    if (k_list[k_idx].level > object_level) return (0);

    /* Assign the template */
    invcopy(i_ptr, k_idx);

    /* Return the artifact index */
    return (name1);
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
    for (what = 0, done = 0; !what && (done < 12); ++done) {

        /* Pick a special object */
        what = make_artifact_special_aux(i_ptr);
    }

    /* Nothing found */
    if (!what) return (FALSE);
    

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

    /* Mega-Hack -- increase the rating */
    rating += 10;

    /* Mega-Hack -- increase the rating again */
    if (i_ptr->cost > 50000L) rating += 10;

    /* Set the good item flag */
    good_item_flag = TRUE;

    /* Cheat -- peek at the item */
    if (cheat_peek) inven_mention(i_ptr);

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
    for (i = 1; i < ART_MAX; i++) {

        /* Cannot make an artifact twice */
        if (v_list[i].cur_num) continue;

        /* XXX Paranoia -- hard code the object index */
        /* if (v_list[i].k_idx != i_ptr->k_idx) continue; */

        /* Must have the correct fields */
        if (v_list[i].tval != i_ptr->tval) continue;
        if (v_list[i].sval != i_ptr->sval) continue;

        /* Hack -- enforce "depth" */
        if (v_list[i].level > dun_level) continue;
        
        /* We must make the "rarity roll" */
        if (rand_int(v_list[i].rarity) != 0) continue;

        /* Success */
        what = i;
        
        /* Done */
        break;
    }

    /* Nothing made */
    if (!what) return (FALSE);


    /* Mark that artifact as Made */
    v_list[what].cur_num = 1;

    /* Save the Artifact Index */
    i_ptr->name1 = what;

    /* Extract the flags */
    i_ptr->flags1 = v_list[what].flags1;
    i_ptr->flags2 = v_list[what].flags2;
    i_ptr->flags3 = v_list[what].flags3;

    /* Extract the other fields */
    i_ptr->pval = v_list[what].pval;
    i_ptr->ac = v_list[what].ac;
    i_ptr->dd = v_list[what].dd;
    i_ptr->ds = v_list[what].ds;
    i_ptr->toac = v_list[what].toac;
    i_ptr->tohit = v_list[what].tohit;
    i_ptr->todam = v_list[what].todam;
    i_ptr->weight = v_list[what].weight;
    i_ptr->cost = v_list[what].cost;

    /* Mega-Hack -- increase the rating */
    rating += 10;

    /* Mega-Hack -- increase the rating again */
    if (i_ptr->cost > 50000L) rating += 10;

    /* Set the good item flag */
    good_item_flag = TRUE;

    /* Cheat -- peek at the item */
    if (cheat_peek) inven_mention(i_ptr);

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
 * Note that the "k_list" was rebuilt (2.7.0) to remove the old problems
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
 * never places any lites, rings, or amulets, except artifact ones.
 */
void apply_magic(inven_type *i_ptr, int level, bool okay, bool good, bool great)
{
    int chance, special, cursed;


    /* Paranoia -- great implies good */
    if (great) good = TRUE;


    /* Extract the "chance" of "goodness" */
    chance = MAG_BASE_MAGIC + level;
    if (chance > MAG_BASE_MAX) chance = MAG_BASE_MAX;

    /* Extract the "chance" of "greatness" (approx range 3-18 percent) */
    special = (10 * chance) / MAG_DIV_SPECIAL;

    /* Extract the "chance" of ickiness (approx range 11-54 percent) */
    cursed = (10 * chance) / MAG_DIV_CURSED;


    /* Apply magic (good or bad) according to type */
    switch (i_ptr->tval) {


      case TV_DRAG_ARMOR:

        /* Good */
        if (good || magik(chance)) {

            /* Enchant (already +10) */
            i_ptr->toac += randint(3) + m_bonus(0, 5, level);

            /* Perhaps an artifact */
            if (great || magik(special)) {

                /* Roll for artifact */
                if (okay && make_artifact(i_ptr)) break;
                if (great && okay && make_artifact(i_ptr)) break;

                /* Even better */
                i_ptr->toac += randint(5) + m_bonus(0, 5, level);

                /* Add an extra "resist" */
                if (rand_int(5) == 0) {
                    give_1_hi_resist(i_ptr);
                    i_ptr->cost += 2000L;
                }
                else if (rand_int(2) == 0) {
                    give_1_lo_resist(i_ptr);
                    i_ptr->cost += 500L;
                }
            }
        }

        rating += 30;

        if (cheat_peek) inven_mention(i_ptr);

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
                    (rand_int(30) == 0)) {

                    i_ptr->flags2 |= (TR2_RES_ELEC | TR2_RES_COLD |
                                      TR2_RES_ACID | TR2_RES_FIRE |
                                      TR2_HOLD_LIFE |
                                      TR2_SUST_STR | TR2_SUST_DEX |
                                      TR2_SUST_CON | TR2_SUST_INT |
                                      TR2_SUST_WIS | TR2_SUST_CHR);
                    i_ptr->flags3 |= (TR3_IGNORE_FIRE | TR3_IGNORE_COLD |
                                      TR3_IGNORE_ELEC | TR3_IGNORE_ACID);
                    i_ptr->toac += randint(5) + 10;
                    i_ptr->cost += 30000L;
                    give_1_hi_resist(i_ptr);
                    i_ptr->name2 = EGO_ROBE_MAGI;

                    rating += 30;

                    if (cheat_peek) inven_mention(i_ptr);

                    break;
                }

                /* Make it "Excellent" */
                switch (randint(9)) {

                  case 1:
                    i_ptr->flags2 |= (TR2_RES_ELEC | TR2_RES_COLD |
                                      TR2_RES_ACID | TR2_RES_FIRE);
                    i_ptr->flags3 |= (TR3_IGNORE_ELEC | TR3_IGNORE_COLD |
                                      TR3_IGNORE_ACID | TR3_IGNORE_FIRE);
                    if (rand_int(3) == 0) {
                        i_ptr->flags1 |= TR1_STEALTH;
                        i_ptr->pval = randint(3);	/* +N Stealth */
                        i_ptr->toac += 15;
                        i_ptr->cost += 15000L;
                        give_1_hi_resist(i_ptr);
                        i_ptr->name2 = EGO_ELVENKIND;
                        rating += 25;
                        if (cheat_peek) inven_mention(i_ptr);
                    }
                    else {
                        i_ptr->toac += 10;
                        i_ptr->cost += 12500L;
                        i_ptr->name2 = EGO_RESIST;
                        rating += 20;
                        if (cheat_peek) inven_mention(i_ptr);
                    }
                    break;

                  case 2:
                    i_ptr->flags2 |= (TR2_RES_ACID);
                    i_ptr->flags3 |= (TR3_IGNORE_ACID);
                    i_ptr->cost += 1000L;
                    i_ptr->name2 = EGO_RESIST_A;
                    rating += 18;
                    if (cheat_peek) inven_mention(i_ptr);
                    break;

                  case 3: case 4:
                    i_ptr->flags2 |= (TR2_RES_FIRE);
                    i_ptr->flags3 |= (TR3_IGNORE_FIRE);
                    i_ptr->cost += 600L;
                    i_ptr->name2 = EGO_RESIST_F;
                    rating += 17;
                    if (cheat_peek) inven_mention(i_ptr);
                    break;

                  case 5: case 6:
                    i_ptr->flags2 |= (TR2_RES_COLD);
                    i_ptr->flags3 |= (TR3_IGNORE_COLD);
                    i_ptr->cost += 600L;
                    i_ptr->name2 = EGO_RESIST_C;
                    rating += 16;
                    if (cheat_peek) inven_mention(i_ptr);
                    break;

                  case 7: case 8: case 9:
                    i_ptr->flags2 |= (TR2_RES_ELEC);
                    i_ptr->flags3 |= (TR3_IGNORE_ELEC);
                    i_ptr->cost += 500L;
                    i_ptr->name2 = EGO_RESIST_E;
                    rating += 15;
                    if (cheat_peek) inven_mention(i_ptr);
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
                    if (cheat_peek) inven_mention(i_ptr);
                    break;

                  case 4: case 5: case 6:
                    i_ptr->flags3 |= (TR3_SHOW_MODS);
                    i_ptr->tohit += 1 + randint(4);
                    i_ptr->todam += 1 + randint(4);
                    i_ptr->cost += 1500L;
                    i_ptr->name2 = EGO_SLAYING;
                    rating += 17;
                    if (cheat_peek) inven_mention(i_ptr);
                    break;

                  case 7: case 8: case 9:
                    i_ptr->flags1 |= (TR1_DEX);
                    i_ptr->pval = 2 + randint(2);	/* +N DEX */
                    i_ptr->cost += 1000L;
                    i_ptr->name2 = EGO_AGILITY;
                    rating += 14;
                    if (cheat_peek) inven_mention(i_ptr);
                    break;

                  case 10:
                    i_ptr->flags1 |= (TR1_STR);
                    i_ptr->flags3 |= (TR3_SHOW_MODS | TR3_HIDE_TYPE);
                    i_ptr->pval = 1 + randint(4);	/* +N STR */
                    i_ptr->tohit += 1 + randint(4);
                    i_ptr->todam += 1 + randint(4);
                    i_ptr->cost += 2500L;
                    i_ptr->name2 = EGO_POWER;
                    rating += 22;
                    if (cheat_peek) inven_mention(i_ptr);
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
                if (rand_int(2) == 0) {
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

                    /* Boots of Speed */
                    i_ptr->flags1 |= TR1_SPEED;
                    i_ptr->flags3 |= TR3_HIDE_TYPE;
                    i_ptr->name2 = EGO_SPEED;

                    /* Base speed (2 to 8) */
                    i_ptr->pval = m_bonus(1, 5, level) + randint(3);

                    /* Super-charge the boots */
                    while (rand_int(2) == 0) i_ptr->pval += randint(2);

                    /* Calculate the cost */
                    i_ptr->cost += 200000L;

                    /* Major bonus for each "speed" point */
                    i_ptr->cost += (i_ptr->pval * 40000L);

                    /* Increase the rating */
                    rating += (25 + i_ptr->pval);
                    if (cheat_peek) inven_mention(i_ptr);

                    break;

                  case 2: case 3: case 4: case 5:
                    i_ptr->flags2 |= (TR2_FREE_ACT);
                    i_ptr->cost += 1000L;
                    i_ptr->name2 = EGO_FREE_ACTION;
                    rating += 15;
                    break;

                  case 6: case 7: case 8: case 9:
                  case 10: case 11: case 12: case 13:
                    i_ptr->flags1 |= TR1_STEALTH;
                    i_ptr->flags3 |= TR3_HIDE_TYPE;
                    i_ptr->pval = randint(3);	/* +N Stealth */
                    i_ptr->cost += 500L;
                    i_ptr->name2 = EGO_STEALTH;
                    rating += 16;
                    break;

                  default:
                    i_ptr->flags3 |= TR3_FEATHER;
                    i_ptr->cost += 250L;
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

                /* Negative armor bonus */
                i_ptr->toac = 0 - (m_bonus(5, 20, level));

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
                        i_ptr->weight += 500;	/* 50 pounds */
                        i_ptr->name2 = EGO_GREAT_MASS;
                        break;
                }
            }
        }

        break;


      case TV_CROWN:

        /* XXX XXX XXX Mega-Hack -- extra "goodness" */
        chance += i_ptr->cost / 100L;

        /* Mega-Hack -- improve the chance for "greatness" */
        special = special * 2;

        /* Apply some magic */
        if (good || magik(chance)) {

            /* Make it better */
            i_ptr->toac += randint(3) + m_bonus(0, 10, level);

            /* Apply more magic */
            if (great || magik(special)) {

                /* Roll for artifact */
                if (okay && make_artifact(i_ptr)) return;
                if (great && okay && make_artifact(i_ptr)) break;

                /* XXX XXX XXX Indent */
                if (TRUE) {

                    /* Make it "excellent" */
                    switch (randint(6)) {

                      case 1:
                        i_ptr->flags1 |= (TR1_STR | TR1_DEX | TR1_CON);
                        i_ptr->flags2 |= (TR2_FREE_ACT | TR2_SUST_STR |
                                          TR2_SUST_DEX | TR2_SUST_CON);
                        i_ptr->pval = randint(3);	/* +N STR/DEX/CON */
                        i_ptr->cost += 2000L;
                        i_ptr->name2 = EGO_MIGHT;
                        rating += 19;
                        if (cheat_peek) inven_mention(i_ptr);
                        break;

                      case 2:
                        i_ptr->flags1 |= (TR1_CHR | TR1_WIS);
                        i_ptr->flags2 |= (TR2_SUST_CHR | TR2_SUST_WIS);
                        i_ptr->pval = randint(3);	/* +N WIS/CHR */
                        i_ptr->cost += 2000L;
                        i_ptr->name2 = EGO_LORDLINESS;
                        rating += 17;
                        if (cheat_peek) inven_mention(i_ptr);
                        break;

                      case 3:
                        i_ptr->flags1 |= (TR1_INT);
                        i_ptr->flags2 |= (TR2_RES_ELEC | TR2_RES_COLD |
                                          TR2_RES_ACID | TR2_RES_FIRE |
                                          TR2_SUST_INT);
                        i_ptr->flags3 |= (TR3_IGNORE_ELEC | TR3_IGNORE_COLD |
                                          TR3_IGNORE_ACID | TR3_IGNORE_FIRE);
                        i_ptr->pval = randint(3);	/* +N INT */
                        i_ptr->cost += 7500L;
                        i_ptr->name2 = EGO_MAGI;
                        rating += 15;
                        if (cheat_peek) inven_mention(i_ptr);
                        break;

                      case 4:
                        i_ptr->flags1 |= TR1_CHR;
                        i_ptr->flags2 |= TR2_SUST_CHR;
                        i_ptr->pval = randint(4);	/* +N CHR */
                        i_ptr->cost += 1000L;
                        i_ptr->name2 = EGO_BEAUTY;
                        rating += 8;
                        if (cheat_peek) inven_mention(i_ptr);
                        break;

                      case 5:
                        i_ptr->flags1 |= TR1_SEARCH;
                        i_ptr->flags2 |= TR2_RES_BLIND;
                        i_ptr->flags3 |= TR3_SEE_INVIS;
                        i_ptr->pval = (1 + randint(4));	/* +N Search */
                        i_ptr->cost += 1000L;
                        i_ptr->name2 = EGO_SEEING;
                        rating += 8;
                        if (cheat_peek) inven_mention(i_ptr);
                        break;

                      case 6:
                        i_ptr->flags3 |= TR3_REGEN;
                        i_ptr->cost += 1500L;
                        i_ptr->name2 = EGO_REGENERATION;
                        rating += 10;
                        if (cheat_peek) inven_mention(i_ptr);
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
                i_ptr->toac = 0 - (m_bonus(5, 20, level));

                /* Choose some damage */
                switch (randint(7)) {
                  case 1:
                    i_ptr->flags1 |= TR1_INT;
                    i_ptr->pval = 0 - randint(5);
                    i_ptr->name2 = EGO_STUPIDITY;
                    break;
                  case 2:
                  case 3:
                    i_ptr->flags1 |= TR1_WIS;
                    i_ptr->pval = 0 - randint(5);
                    i_ptr->name2 = EGO_DULLNESS;
                    break;
                  case 4:
                  case 5:
                    i_ptr->flags1 |= TR1_STR;
                    i_ptr->pval = 0 - randint(5);
                    i_ptr->name2 = EGO_WEAKNESS;
                    break;
                  case 6:
                    i_ptr->flags3 |= TR3_TELEPORT;
                    i_ptr->name2 = EGO_TELEPORTATION;
                    break;
                  case 7:
                    i_ptr->flags1 |= TR1_CHR;
                    i_ptr->pval = 0 - randint(5);
                    i_ptr->name2 = EGO_UGLINESS;
                    break;
                }
            }
        }
        break;

      case TV_HELM:

        /* Apply some magic */
        if (good || magik(chance)) {

            /* Make it better */
            i_ptr->toac += randint(3) + m_bonus(0, 10, level);

            /* Apply more magic */
            if (great || magik(special)) {

                /* Roll for artifact */
                if (okay && make_artifact(i_ptr)) return;
                if (great && okay && make_artifact(i_ptr)) break;

                /* XXX XXX XXX Indent Me */
                if (TRUE) {

                    /* Make it "excellent" */
                    switch (randint(14)) {

                      case 1: case 2:
                        i_ptr->flags1 |= TR1_INT;
                        i_ptr->flags2 |= TR2_SUST_INT;
                        i_ptr->pval = randint(2);	/* +N INT */
                        i_ptr->cost += 500L;
                        i_ptr->name2 = EGO_INTELLIGENCE;
                        rating += 13;
                        if (cheat_peek) inven_mention(i_ptr);
                        break;

                      case 3: case 4: case 5:
                        i_ptr->flags1 |= TR1_WIS;
                        i_ptr->flags2 |= TR2_SUST_WIS;
                        i_ptr->pval = randint(2);	/* +N Wis */
                        i_ptr->cost += 500L;
                        i_ptr->name2 = EGO_WISDOM;
                        rating += 13;
                        if (cheat_peek) inven_mention(i_ptr);
                        break;

                      case 6: case 7: case 8: case 9:
                        i_ptr->flags1 |= TR1_INFRA;
                        i_ptr->flags3 |= TR3_HIDE_TYPE;
                        i_ptr->pval = 1 + randint(4);	/* +N Infra */
                        i_ptr->cost += 500L;
                        i_ptr->name2 = EGO_INFRAVISION;
                        rating += 11;
                        if (cheat_peek) inven_mention(i_ptr);
                        break;

                      case 10: case 11:
                        i_ptr->flags2 |= (TR2_RES_LITE);
                        i_ptr->flags3 |= (TR3_LITE);
                        i_ptr->cost += 500L;
                        i_ptr->name2 = EGO_LITE;
                        rating += 6;
                        if (cheat_peek) inven_mention(i_ptr);
                        break;

                      case 12: case 13:
                        i_ptr->flags2 |= TR2_RES_BLIND;
                        i_ptr->flags3 |= TR3_SEE_INVIS;
                        i_ptr->cost += 1000L;
                        i_ptr->name2 = EGO_SEEING;
                        if (rand_int(5) == 0) {
                            i_ptr->flags1 |= TR1_SEARCH;
                            i_ptr->pval = randint(2);	/* +N Search */
                            i_ptr->cost += 1000L;
                        }
                        rating += 8;
                        if (cheat_peek) inven_mention(i_ptr);
                        break;

                     default: /* case 14: */
                        i_ptr->flags3 |= TR3_TELEPATHY;
                        i_ptr->cost += 50000L;
                        i_ptr->name2 = EGO_TELEPATHY;
                        rating += 20;
                        if (cheat_peek) inven_mention(i_ptr);
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
                i_ptr->toac = 0 - (m_bonus(5, 20, level));

                /* Choose some damage */
                switch (randint(7)) {
                  case 1:
                    i_ptr->flags1 |= TR1_INT;
                    i_ptr->pval = 0 - randint(5);
                    i_ptr->name2 = EGO_STUPIDITY;
                    break;
                  case 2:
                  case 3:
                    i_ptr->flags1 |= TR1_WIS;
                    i_ptr->pval = 0 - randint(5);
                    i_ptr->name2 = EGO_DULLNESS;
                    break;
                  case 4:
                  case 5:
                    i_ptr->flags1 |= TR1_STR;
                    i_ptr->pval = 0 - randint(5);
                    i_ptr->name2 = EGO_WEAKNESS;
                    break;
                  case 6:
                    i_ptr->flags3 |= TR3_TELEPORT;
                    i_ptr->name2 = EGO_TELEPORTATION;
                    break;
                  case 7:
                    i_ptr->flags1 |= TR1_CHR;
                    i_ptr->pval = 0 - randint(5);
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
                    i_ptr->pval = randint(3);	/* +N Stealth */
                    i_ptr->flags1 |= (TR1_STEALTH);
                    i_ptr->flags2 |= (TR2_RES_ACID);
                    i_ptr->flags3 |= (TR3_IGNORE_ACID);
                    i_ptr->name2 = EGO_AMAN;
                    i_ptr->cost += 4000L;
                    rating += 16;
                }
                else {
                    i_ptr->toac += m_bonus(3, 10, level);
                    i_ptr->pval = randint(3);	/* +N Stealth */
                    i_ptr->flags1 |= TR1_STEALTH;
                    i_ptr->flags3 |= TR3_HIDE_TYPE;
                    i_ptr->name2 = EGO_STEALTH;
                    i_ptr->cost += 500L;
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
                        i_ptr->tohit = 0 - m_bonus(1, 10, level);
                        i_ptr->todam = 0 - m_bonus(1, 10, level);
                        break;
                    case 2:
                        i_ptr->name2 = EGO_VULNERABILITY;
                        i_ptr->toac = 0 - m_bonus(20, 40, level);
                        break;
                    case 3:
                        i_ptr->name2 = EGO_ENVELOPING;
                        i_ptr->flags3 |= (TR3_SHOW_MODS);
                        i_ptr->tohit = 0 - m_bonus(5, 15, level);
                        i_ptr->todam = 0 - m_bonus(5, 15, level);
                        break;
                }
            }
        }
        break;


      case TV_DIGGING:

        /* Apply some magic */
        if (good || magik(chance)) {

            /* Hack -- hit/damage bonus */
            i_ptr->tohit += m_bonus(1, 5, level);
            i_ptr->todam += m_bonus(1, 5, level);

            /* Add in a digging bonus */
            i_ptr->pval += m_bonus(1, 5, level);

            /* Hack -- "shovels of fire" */
            if (great || magik(special)) {
                i_ptr->flags1 |= (TR1_BRAND_FIRE);
                i_ptr->flags3 |= (TR3_IGNORE_FIRE);
                i_ptr->tohit += 5;
                i_ptr->todam += 5;
                i_ptr->cost += 2000L;
                i_ptr->name2 = EGO_FIRE;
                rating += 15;
                if (cheat_peek) inven_mention(i_ptr);
            }
        }

        /* Cursed shovels */
        else if (magik(cursed)) {

            /* Cursed */
            i_ptr->flags3 |= TR3_CURSED;
            i_ptr->tohit = 0 - (randint(5) + m_bonus(1, 10, level));
            i_ptr->todam = 0 - (randint(5) + m_bonus(1, 10, level));

            /* Permanent damage */
            if (magik(special)) {
                i_ptr->cost = 0L;
                i_ptr->pval = 0 - m_bonus(1, 15, level);
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

            /* Mega-Hack -- improve the "special" chance */
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
                    (rand_int(2) == 0)) {

                    i_ptr->flags1 |= (TR1_BRAND_FIRE);
                    i_ptr->flags3 |= (TR3_IGNORE_FIRE);

                    /* Better stats */
                    i_ptr->tohit += 5;
                    i_ptr->todam += 5;

                    /* Basic cost increase */
                    i_ptr->cost += 2000L;

                    /* Super-charge the whips */
                    while (rand_int(i_ptr->dd * 5L) == 0) {

                        /* One extra damage die */
                        i_ptr->dd++;

                        /* Major bonus for each increase */
                        i_ptr->cost += (i_ptr->dd * 2000L);
                    }

                    i_ptr->name2 = EGO_FIRE;

                    rating += 20;

                    if (cheat_peek) inven_mention(i_ptr);

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

                    i_ptr->cost += 20000L;
                    i_ptr->name2 = EGO_HA;
                    rating += 30;
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
                    i_ptr->cost += 15000L;
                    i_ptr->name2 = EGO_DF;
                    rating += 23;
                    break;

                  case 3: case 4:
                    i_ptr->flags1 |= (TR1_BRAND_FIRE);
                    i_ptr->flags2 |= (TR2_RES_FIRE);
                    i_ptr->flags2 |= (TR3_IGNORE_FIRE);
                    i_ptr->tohit += 2;
                    i_ptr->todam += 2;
                    i_ptr->cost += 3000L;
                    i_ptr->name2 = EGO_FT;
                    rating += 20;
                    break;

                  case 5: case 6:
                    i_ptr->flags1 |= (TR1_BRAND_COLD);
                    i_ptr->flags2 |= (TR2_RES_COLD);
                    i_ptr->flags3 |= (TR3_IGNORE_COLD);
                    i_ptr->tohit += 2;
                    i_ptr->todam += 2;
                    i_ptr->cost += 2500L;
                    i_ptr->name2 = EGO_FB;
                    rating += 20;
                    break;

                  case 7: case 8:
                    i_ptr->flags1 |= TR1_SLAY_ANIMAL;
                    i_ptr->tohit += 3;
                    i_ptr->todam += 3;
                    i_ptr->cost += 2000L;
                    i_ptr->name2 = EGO_SLAY_ANIMAL;
                    rating += 15;
                    break;

                  case 9: case 10:
                    i_ptr->flags1 |= TR1_SLAY_DRAGON;
                    i_ptr->tohit += 3;
                    i_ptr->todam += 3;
                    i_ptr->cost += 4000L;
                    i_ptr->name2 = EGO_SLAY_DRAGON;
                    rating += 18;
                    break;

                  case 11: case 12:
                    i_ptr->flags1 |= TR1_SLAY_EVIL;
                    i_ptr->tohit += 3;
                    i_ptr->todam += 3;
                    i_ptr->cost += 4000L;
                    i_ptr->name2 = EGO_SLAY_EVIL;

                    /* One in three is also a blessed wisdom booster */
                    if (rand_int(3) == 0) {
                        i_ptr->flags1 |= (TR1_WIS);
                        i_ptr->flags3 |= (TR3_BLESSED);
                        i_ptr->pval = m_bonus(0, 2, level);
                        i_ptr->cost += 1000L;
                    }

                    rating += 18;

                    break;

                  case 13: case 14:
                    i_ptr->flags1 |= (TR1_SLAY_UNDEAD);
                    i_ptr->flags3 |= (TR3_SEE_INVIS);
                    i_ptr->tohit += 2;
                    i_ptr->todam += 2;
                    i_ptr->cost += 3000L;
                    i_ptr->name2 = EGO_SLAY_UNDEAD;

                    /* One in three is also a Life Holder */
                    if (rand_int(3) == 0) {
                        i_ptr->flags2 |= (TR2_HOLD_LIFE);
                        i_ptr->cost += 1000L;
                    }

                    rating += 18;

                    break;

                  case 15: case 16: case 17:
                    i_ptr->flags1 |= TR1_SLAY_ORC;
                    i_ptr->tohit += 2;
                    i_ptr->todam += 2;
                    i_ptr->cost += 1200L;
                    i_ptr->name2 = EGO_SLAY_ORC;
                    rating += 13;
                    break;

                  case 18: case 19: case 20:
                    i_ptr->flags1 |= TR1_SLAY_TROLL;
                    i_ptr->tohit += 2;
                    i_ptr->todam += 2;
                    i_ptr->cost += 1200L;
                    i_ptr->name2 = EGO_SLAY_TROLL;
                    rating += 13;
                    break;

                  case 21: case 22: case 23:
                    i_ptr->flags1 |= TR1_SLAY_GIANT;
                    i_ptr->tohit += 2;
                    i_ptr->todam += 2;
                    i_ptr->cost += 1200L;
                    i_ptr->name2 = EGO_SLAY_GIANT;
                    rating += 14;
                    break;

                  case 24: case 25: case 26:
                    i_ptr->flags1 |= TR1_SLAY_DEMON;
                    i_ptr->tohit += 2;
                    i_ptr->todam += 2;
                    i_ptr->cost += 1200L;
                    i_ptr->name2 = EGO_SLAY_DEMON;
                    rating += 16;
                    break;

                  case 27:
                    i_ptr->flags1 |= (TR1_SLAY_ORC |
                                      TR1_STR | TR1_DEX | TR1_CON);
                    i_ptr->flags2 |= (TR2_FREE_ACT);
                    i_ptr->flags3 |= (TR3_SEE_INVIS);
                    i_ptr->tohit += 3 + randint(5);
                    i_ptr->todam += 3 + randint(5);
                    i_ptr->pval = 1;	/* +1 STR/DEX/CON */
                    i_ptr->cost += 20000L;

                    /* Super-charge */
                    while (rand_int(500) == 0) {

                        /* Add another STR/DEX/CON bonus */
                        i_ptr->pval++;

                        /* Increase the cost */
                        i_ptr->cost += 20000L;
                    }

                    i_ptr->name2 = EGO_WEST;
                    give_1_lo_resist(i_ptr);
                    rating += 20;
                    break;

                  /* Anything can be blessed */
                  case 28: case 29:
                    i_ptr->flags3 |= TR3_BLESSED;
                    i_ptr->flags1 |= TR1_WIS;
                    i_ptr->tohit += 3;
                    i_ptr->todam += 3;
                    i_ptr->pval = randint(3);	/* +N Wisdom */
                    i_ptr->cost += 5000L;
                    i_ptr->name2 = EGO_BLESS_BLADE;
                    give_1_lo_resist(i_ptr);
                    rating += 20;
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
                    i_ptr->cost += 10000L;
                    i_ptr->name2 = EGO_ATTACKS;
                    rating += 20;
                    break;
                }

                /* Cheat -- describe the item */
                if (cheat_peek) inven_mention(i_ptr);
            }
        }

        /* Cursed Weapons */
        else if (magik(cursed)) {

            /* Cursed */
            i_ptr->flags3 |= TR3_CURSED;
            i_ptr->tohit = 0 - (randint(5) + m_bonus(1, 20, level));
            i_ptr->todam = 0 - (randint(5) + m_bonus(1, 20, level));

            /* Permanently cursed Weapon of Morgul */
            if ((level > (20 + randint(15))) && (rand_int(10) == 0)) {
                i_ptr->flags3 |= (TR3_HEAVY_CURSE | TR3_AGGRAVATE | TR3_SEE_INVIS);
                i_ptr->tohit -= 15;
                i_ptr->todam -= 15;
                i_ptr->toac = -10;
                i_ptr->weight += 100;	/* 10 pounds */
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
                    if (cheat_peek) inven_mention(i_ptr);
                    break;

                  case 2:
                    i_ptr->flags3 |= TR3_XTRA_SHOTS;
                    i_ptr->tohit += 10;
                    i_ptr->todam += 3;
                    i_ptr->name2 = EGO_EXTRA_SHOTS;
                    i_ptr->cost += 10000L;
                    rating += 20;
                    if (cheat_peek) inven_mention(i_ptr);
                    break;

                  case 3: case 4: case 5: case 6:
                    i_ptr->tohit += 5;
                    i_ptr->todam += 12;
                    i_ptr->name2 = EGO_VELOCITY;
                    i_ptr->cost += 1000L;
                    rating += 11;
                    if (cheat_peek) inven_mention(i_ptr);
                    break;

                  case 7: case 8: case 9: case 10:
                    i_ptr->tohit += 12;
                    i_ptr->todam += 5;
                    i_ptr->name2 = EGO_ACCURACY;
                    i_ptr->cost += 1000L;
                    rating += 11;
                    if (cheat_peek) inven_mention(i_ptr);
                    break;

                  case 11: case 12: case 13: case 14:
                    i_ptr->tohit += m_bonus(2, 6, level);
                    i_ptr->todam += m_bonus(2, 6, level);
                    break;

                  case 15: case 16: case 17:
                    i_ptr->tohit += m_bonus(2, 6, level);
                    break;

                  default: /* case 18: case 19: case 20: */
                    i_ptr->todam += m_bonus(2, 6, level);
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
                    i_ptr->name2 = EGO_AMMO_WOUNDING;
                    i_ptr->tohit += 5;
                    i_ptr->todam += 5;
                    i_ptr->dd += 1;	/* Extra die of damage */
                    i_ptr->cost += 30L;
                    rating += 5;
                    break;

                  case 4: case 5:
                    i_ptr->name2 = EGO_AMMO_FIRE;
                    i_ptr->flags1 |= (TR1_BRAND_FIRE);
                    i_ptr->flags3 |= (TR3_IGNORE_FIRE);
                    i_ptr->tohit += 2;
                    i_ptr->todam += 4;
                    i_ptr->cost += 25L;
                    rating += 6;
                    break;

                  case 6: case 7:
                    i_ptr->name2 = EGO_AMMO_EVIL;
                    i_ptr->flags1 |= TR1_SLAY_EVIL;
                    i_ptr->tohit += 3;
                    i_ptr->todam += 3;
                    i_ptr->cost += 25L;
                    rating += 7;
                    break;

                  case 8: case 9:
                    i_ptr->name2 = EGO_AMMO_ANIMAL;
                    i_ptr->flags1 |= TR1_SLAY_ANIMAL;
                    i_ptr->tohit += 2;
                    i_ptr->todam += 2;
                    i_ptr->cost += 30L;
                    rating += 5;
                    break;

                  case 10:
                    i_ptr->name2 = EGO_AMMO_DRAGON;
                    i_ptr->flags1 |= TR1_SLAY_DRAGON;
                    i_ptr->tohit += 3;
                    i_ptr->todam += 3;
                    i_ptr->cost += 35L;
                    rating += 9;
                    break;

                  case 11:
                    i_ptr->name2 = EGO_AMMO_SLAYING;
                    i_ptr->tohit += 10;
                    i_ptr->todam += 10;
                    i_ptr->dd += 2; /* two extra die of damage */
                    i_ptr->cost += 45L;
                    rating += 10;
                    break;
                }
            }

            /* Super-charge the arrows */
            while (magik(special)) {

                /* Extra die of damage */
                i_ptr->dd += 1;

                /* Increase the cost */
                i_ptr->cost += (i_ptr->dd * 5L);
            }
        }

        /* Cursed missiles */
        else if (magik(cursed)) {

            /* Cursed missiles */
            i_ptr->flags3 |= TR3_CURSED;
            i_ptr->tohit = 0 - (randint(10) + m_bonus(5, 25, level));
            i_ptr->todam = 0 - (randint(10) + m_bonus(5, 25, level));

            /* Permanently damaged missiles */
            if (magik(special)) {
                i_ptr->tohit = 0 - (m_bonus(10, 30, level) + 20);
                i_ptr->todam = 0 - (m_bonus(10, 30, level) + 20);
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

            /* Stat bonus */
            i_ptr->pval = m_bonus(1, 6, level);

            /* Cursed */
            if (magik(cursed)) {
                i_ptr->pval = 0 - m_bonus(1, 10, level);
                i_ptr->flags3 |= TR3_CURSED;
                i_ptr->cost = 0L;
            }

            break;

          /* Ring of Speed! */
          case SV_RING_SPEED:

            /* Base speed (2 to 8) */
            i_ptr->pval = m_bonus(1, 5, level) + randint(3);

            /* Super-charge the ring */
            while (rand_int(2) == 0) i_ptr->pval += randint(2);

            /* Cursed Ring */
            if (magik(cursed)) {
                i_ptr->flags3 |= TR3_CURSED;
                i_ptr->pval = 0 - i_ptr->pval;
                i_ptr->cost = 0L;
                break;
            }

            /* Base cost (see "k_list.txt") */
            /* i_ptr->cost = 100000; */

            /* Increase cost for every speed point */
            i_ptr->cost += (i_ptr->pval * 20000L);

            /* Rating boost */
            rating += (25 + i_ptr->pval);
            if (cheat_peek) inven_mention(i_ptr);
            break;

          /* Searching */
          case SV_RING_SEARCHING:

            /* Bonus to searching */
            i_ptr->pval = m_bonus(1, 10, level);

            /* Cursed */
            if (magik(cursed)) {
                i_ptr->flags3 |= TR3_CURSED;
                i_ptr->pval = 0 - i_ptr->pval;
                i_ptr->cost = 0L;
            }

            break;

          /* Flames, Acid, Ice */
          case SV_RING_FLAMES:
          case SV_RING_ACID:
          case SV_RING_ICE:

            /* Bonus to armor class */
            i_ptr->toac = m_bonus(1, 10, level) + randint(7) + 5;
            break;

          /* Weakness, Stupidity */
          case SV_RING_WEAKNESS:
          case SV_RING_STUPIDITY:

            /* Always cursed */
            i_ptr->pval = 0 - randint(5);
            break;

          /* WOE, Stupidity */
          case SV_RING_WOE:

            /* Always cursed */
            i_ptr->toac = 0 - (m_bonus(1,10,level) + 5);
            i_ptr->pval = 0 - randint(5);
            break;

          /* Ring of damage */
          case SV_RING_DAMAGE:

            /* Bonus to damage */
            i_ptr->todam = m_bonus(1, 10, level) + randint(10) + 3;

            /* Cursed */
            if (magik(cursed)) {
                i_ptr->flags3 |= TR3_CURSED;
                i_ptr->todam = 0 - i_ptr->todam;
                i_ptr->cost = 0L;
            }

            break;

          /* Ring of Accuracy */
          case SV_RING_ACCURACY:

            /* Bonus to hit */
            i_ptr->tohit = m_bonus(1, 10, level) + randint(10) + 3;

            /* Cursed */
            if (magik(cursed)) {
                i_ptr->flags3 |= TR3_CURSED;
                i_ptr->tohit = 0 - i_ptr->tohit;
                i_ptr->cost = 0L;
            }

            break;

          /* Ring of Protection */
          case SV_RING_PROTECTION:

            /* Bonus to armor class */
            i_ptr->toac = m_bonus(0, 10, level) + randint(5) + 4;

            /* Cursed */
            if (magik(cursed)) {
                i_ptr->flags3 |= TR3_CURSED;
                i_ptr->toac = 0 - i_ptr->toac;
                i_ptr->cost = 0L;
            }

            break;

          /* Ring of Slaying */
          case SV_RING_SLAYING:

            /* Bonus to damage and to hit */
            i_ptr->todam = m_bonus(1, 10, level) + randint(3) + 2;
            i_ptr->tohit = m_bonus(1, 10, level) + randint(3) + 2;

            /* Cursed */
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

        switch (i_ptr->sval) {

          /* Amulet of wisdom/charisma */
          case SV_AMULET_WISDOM:
          case SV_AMULET_CHARISMA:

            i_ptr->pval = m_bonus(1, 5, level);

            if (magik(cursed)) {
                i_ptr->flags3 |= TR3_CURSED;
                i_ptr->pval = 0 - i_ptr->pval;
                i_ptr->cost = 0L;
            }

            break;

          /* Amulet of searching */
          case SV_AMULET_SEARCHING:

            i_ptr->pval = randint(2) + m_bonus(0, 8, level);

            if (magik(cursed)) {
                i_ptr->flags3 |= TR3_CURSED;
                i_ptr->pval = 0 - i_ptr->pval;
                i_ptr->cost = 0L;
            }

            break;

          /* Amulet of the Magi -- never cursed */
          case SV_AMULET_THE_MAGI:

            i_ptr->pval = randint(2) + m_bonus(0, 8, level);
            i_ptr->toac = randint(4) + m_bonus(0, 6, level);
            rating += 25;
            break;

          /* Amulet of Doom -- always cursed */
          case SV_AMULET_DOOM:

            i_ptr->flags3 |= TR3_CURSED;
            i_ptr->pval = 0 - (randint(5) + m_bonus(2, 10, level));
            i_ptr->toac = 0 - (randint(3) + m_bonus(0, 6, level));
            i_ptr->cost = 0L;
            break;
        }
        break;


      case TV_WAND:
        charge_wand(i_ptr);
        break;


      case TV_STAFF:
        charge_staff(i_ptr);
        break;


      case TV_LITE:

        /* Hack -- Torches -- random fuel */
        if (i_ptr->sval == SV_LITE_TORCH) {
            i_ptr->pval = randint(i_ptr->pval);
        }

        /* Hack -- Lanterns -- random fuel */
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
            i_ptr->cost = 0L;
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
 * Attempt to place a "Special Object" at the given location
 * The location must be a valid, empty floor grid.
 */
static int place_object_special(int y, int x)
{
    cave_type		*c_ptr;
    inven_type		*i_ptr;
    inven_type		hack;


    /* Paranoia -- check bounds */
    if (!in_bounds(y, x)) return (FALSE);

    /* Require clean floor space */
    if (!clean_grid_bold(y, x)) return (FALSE);


    /* Hack -- clean up "hack" */
    invwipe(&hack);

    /* Hack -- Try to allocate a special object */
    if (!make_artifact_special(&hack)) return (FALSE);


    /* Get the cave */
    c_ptr = &cave[y][x];
    c_ptr->i_idx = i_pop();
    i_ptr = &i_list[c_ptr->i_idx];

    /* Place the object there (structure copy) */
    (*i_ptr) = hack;

    /* Place the object */
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Is it on the player? */
    if (c_ptr->m_idx == 1) {
        msg_print("You feel something roll beneath your feet.");
    }

    /* Success */
    return (TRUE);
}


/*
 * Attempts to places a random object at the given location -RAK-
 */
void place_object(int y, int x)
{
    int	tmp;
    cave_type		*c_ptr;
    inven_type		*i_ptr;


    /* Paranoia -- check bounds */
    if (!in_bounds(y, x)) return;

    /* Require clean floor space */
    if (!clean_grid_bold(y, x)) return;


    /* Hack -- occasionally place a "special" object */
    if (rand_int(500) == 0) {
        if (place_object_special(y, x)) return;
    }


    /* Hack -- no embedded chests */
    do {
        /* Pick an object based on the "object_level" */
        tmp = get_obj_num(object_level, FALSE);
    } while (opening_chest && (k_list[tmp].tval == TV_CHEST));


    /* Create an object */
    c_ptr = &cave[y][x];
    c_ptr->i_idx = i_pop();
    i_ptr = &i_list[c_ptr->i_idx];

    /* Prepare it */
    invcopy(i_ptr, tmp);

    /* Let it be magic, perhaps even an artifact */
    apply_magic(i_ptr, object_level, TRUE, FALSE, FALSE);

    /* Hack -- generate multiple missiles */
    switch (i_ptr->tval) {
        case TV_SPIKE: case TV_SHOT: case TV_ARROW: case TV_BOLT:
            i_ptr->number = damroll(6,7);
    }

    /* Place it */
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Notice non-cursed out-of-depth objects */
    if (!cursed_p(i_ptr) && (k_list[tmp].level > dun_level)) {

        /* Rating increase */
        rating += (k_list[tmp].level - dun_level);

        /* Cheat -- peek at items */
        if (cheat_peek) inven_mention(i_ptr);
    }

    /* Under the player */
    if (c_ptr->m_idx == 1) {
        msg_print ("You feel something roll beneath your feet.");
    }
}


/*
 * Places a "GOOD" or "GREAT" object at the given location.
 *
 * Only called when scrolls of acquirement are read, monsters with the
 * "GOOD"/"GREAT" flag are killed, and vaults are created.
 *
 * This routine uses "object_level" for the "generation level".
 *
 * This routine requires valid, empty, floor space for reading.
 */
void place_good(int y, int x, bool great)
{
    int		k_idx, tv, sv;

    cave_type		*c_ptr;
    inven_type		*i_ptr;


    /* Paranoia -- check bounds */
    if (!in_bounds(y, x)) return;

    /* Require clean floor space */
    if (!clean_grid_bold(y, x)) return;


    /* XXX XXX Hack -- try for "special objects" */
    if (rand_int(10) == 0) {
        if (place_object_special(y, x)) return;
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
        if ((tv == TV_HARD_ARMOR) || (tv == TV_SOFT_ARMOR) ||
            (tv == TV_DRAG_ARMOR) || (tv == TV_SHIELD) ||
            (tv == TV_CLOAK) || (tv == TV_BOOTS) || (tv == TV_GLOVES) ||
            (tv == TV_HELM) || (tv == TV_CROWN) || 
            (tv == TV_SWORD) || (tv == TV_HAFTED) || (tv == TV_POLEARM) ||
            (tv == TV_BOW) || (tv == TV_BOLT) || (tv == TV_ARROW)) {

            break;
        }

        /* XXX Hack -- High spell books are good.  Highest is great. */
        if ((tv == TV_MAGIC_BOOK) && (sv >= (great ? 8 : 4))) break;

        /* XXX Hack -- High prayer books are good.  Highest is great. */
        if ((tv == TV_PRAYER_BOOK) && (sv >= (great ? 8 : 4))) break;
    }


    /* Make a new object */
    c_ptr = &cave[y][x];
    c_ptr->i_idx = i_pop();
    i_ptr = &i_list[c_ptr->i_idx];

    /* Prepare the object */
    invcopy(i_ptr, k_idx);

    /* Apply some good magic to the item.  Make a great item if requested. */
    apply_magic(i_ptr, object_level, TRUE, TRUE, great);

    /* Hack -- generate multiple "decent" missiles */
    switch (i_ptr->tval) {
        case TV_ARROW: case TV_BOLT:
            i_ptr->number = damroll(6,7);
    }

    /* Drop it into the dungeon */
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Notice non-cursed out-of-depth objects */
    if (!cursed_p(i_ptr) && (k_list[k_idx].level > dun_level)) {

        /* Help the rating */
        rating += (k_list[k_idx].level - dun_level);

        /* Cheat -- peek at items */
        if (cheat_peek) inven_mention(i_ptr);
    }

    if (c_ptr->m_idx == 1) {
        msg_print("You feel something roll beneath your feet.");
    }
}



/*
 * Scatter some "great" objects near the player
 */
void acquirement(int y1, int x1, int num, bool great)
{
    int        y, x, i, d;

    /* Scatter some objects */
    for (; num > 0; --num) {

        /* Check near the player for space */
        for (i = 0; i < 25; ++i) {

            /* Increasing Distance */
            d = (i + 4) / 5;

            /* Pick a location */
            while (1) {
                y = rand_spread(y1, d);
                x = rand_spread(x1, d);
                if (!in_bounds(y, x)) continue;
                if (distance(y1, x1, y, x) > d) continue;
                if (los(y1, x1, y, x)) break;
            }

            /* Must have a clean grid */
            if (!clean_grid_bold(y, x)) continue;

            /* Place a good (or great) object */
            place_good(y, x, great);

            /* Draw the item */
            lite_spot(y, x);

            /* Placement accomplished */
            break;
        }
    }
}





/*
 * Places a random trap at the given location.
 * The location must be a valid, empty, clean, floor grid.
 *
 * Note that all traps start out visible, but are always made "invisible"
 * when created (except pits, which are sometimes visible).  All traps are
 * made "visible" when "discovered" (including pits, which are "uncovered").
 *
 * XXX XXX This routine should be redone to reflect trap "level".
 * That is, it does not make sense to have spiked pits at 50 feet.
 */
void place_trap(int y, int x)
{
    int trap, tval;
    cave_type *c_ptr;
    inven_type *i_ptr;
    bool invis = TRUE;


    /* Paranoia -- verify location */
    if (!in_bounds(y, x)) return;

    /* Require empty, clean, floor grid */
    if (!naked_grid_bold(y, x)) return;


    /* Hack -- Mark the trap as invisible */
    tval = TV_INVIS_TRAP;

    /* Hack -- Pick a trap */
    trap = OBJ_TRAP_LIST + rand_int(MAX_TRAP);

    /* Hack -- Ocassionally, just make a visible open pit */
    if (rand_int(10) == 0) {
        trap = OBJ_OPEN_PIT;
        invis = FALSE;
    }


    /* Make a new object */
    c_ptr = &cave[y][x];
    c_ptr->i_idx = i_pop();
    i_ptr = &i_list[c_ptr->i_idx];
    invcopy(i_ptr, trap);
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Make the trap invisible if needed */
    if (invis) i_ptr->tval = TV_INVIS_TRAP;

    /* Hack -- no trap doors on quest levels */
    if ((i_ptr->sval == SV_TRAP_TRAP_DOOR) && (is_quest(dun_level))) {
        delete_object(y, x);
    }
}


/*
 * Return the "automatic coin type" of a monster race
 * Used to allocate proper treasure when "Creeping coins" die
 * This is one of the few places that we reference monster names
 * However, note that the game will still "work" without this code
 */
int get_coin_type(monster_race *r_ptr)
{
    cptr name = r_ptr->name;
    
    /* Analyze "coin" monsters */
    if (r_ptr->r_char == '$') {

        /* Look for textual clues */
        if (strstr(name, "copper")) return (2);
        if (strstr(name, "silver")) return (5);
        if (strstr(name, "gold")) return (10);
        if (strstr(name, "mithril")) return (16);
        if (strstr(name, "adamantite")) return (17);

        /* Look for textual clues */
        if (strstr(name, "Copper")) return (2);
        if (strstr(name, "Silver")) return (5);
        if (strstr(name, "Gold")) return (10);
        if (strstr(name, "Mithril")) return (16);
        if (strstr(name, "Adamantite")) return (17);
    }
    
    /* Assume nothing */
    return (0);
}


/*
 * Places a treasure (Gold or Gems) at given location
 * The location must be a valid, empty, floor grid.
 */
void place_gold(int y, int x)
{
    int	i;
    cave_type	*c_ptr;
    inven_type	*i_ptr;


    /* Paranoia -- check bounds */
    if (!in_bounds(y, x)) return;

    /* Require clean floor grid */
    if (!clean_grid_bold(y, x)) return;


    /* Pick a Treasure variety */
    i = ((randint(object_level + 2) + 2) / 2) - 1;

    /* Apply "extra" magic */
    if (rand_int(GREAT_OBJ) == 0) {
        i += randint(object_level + 1);
    }

    /* Hack -- Creeping Coins only generate "themselves" */
    if (coin_type) i = coin_type;

    /* Do not create "illegal" Treasure Types */
    if (i >= MAX_GOLD) i = MAX_GOLD - 1;


    /* Make a new object */
    c_ptr = &cave[y][x];
    c_ptr->i_idx = i_pop();
    i_ptr = &i_list[c_ptr->i_idx];
    invcopy(i_ptr, OBJ_GOLD_LIST + i);
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Determine the "cost" */
    i_ptr->cost += (8L * (long)randint((int)i_ptr->cost)) + randint(8);

    /* Hack -- make sure "creeping coins" are not too valuable */
    if (coin_type) {
        i_ptr->cost = ((8L * (long)randint((int)k_list[OBJ_GOLD_LIST + i].cost))
                       + (i_ptr->cost)) / 2;
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
    int i, j;

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
            if (rand_int(GREAT_OBJ) == 0) {

                /* What a bizarre calculation */
                level = 1 + ((level-1) * MAX_K_LEV / randint(MAX_K_LEV));
                if (level > MAX_K_LEV) level = MAX_K_LEV;
            }

            /* Pick any object at or below the given level */
            i = rand_int(t_lev[level]);

            /* Sometimes, try for a "better" item */
            if (rand_int(3) != 0) {

                /* Pick another object at or below the given level */
                j = rand_int(t_lev[level]);

                /* Keep it if it is "better" */
                if (table[i].locale < table[j].locale) i = j;
            }

            /* Sometimes, try for a "better" item */
            if (rand_int(3) != 0) {

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
        if (rand_int(table[i].chance) == 0) break;
    }

    /* Accept that object */
    return (j);
}



