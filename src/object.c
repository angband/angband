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
 * Actually "remove" a dungeon object by "excision" from object list.
 *
 * This function should only be called by a few routines (!).
 *
 * All items in the object list are "on the floor" (unless dead).
 *
 * Inven/Equip/Store items are not processed by this routine.
 *
 * This function should probably not be called.  However, it may
 * be useful after loading a savefile to clean up the dungeon.
 */
void remove_object_idx(int i_idx)
{
    /* One less item */
    i_max--;

    /* Compact the array */
    if (i_idx != i_max) {

        int iy = i_list[i_max].iy;
        int ix = i_list[i_max].ix;

        /* Update the cave record if necessary */
        if (cave[iy][ix].i_idx == i_max) cave[iy][ix].i_idx = i_idx;

        /* Compact, via structure copy */
        i_list[i_idx] = i_list[i_max];
    }

    /* Wipe the hole (again) */
    WIPE(&i_list[i_max], inven_type);
}


/*
 * Delete a dungeon object
 */
void delete_object_idx(int i_idx)
{
    inven_type *i_ptr = &i_list[i_idx];
    
    int y = i_ptr->iy;
    int x = i_ptr->ix;

    cave_type *c_ptr = &cave[y][x];


    /* Hack -- Forget the grid */
    if (c_ptr->info & GRID_MARK) {

        /* Forget the grid */
        c_ptr->info &= ~GRID_MARK;

        /* Mega-Hack -- see "update_map()" in "cave.c" */
        if (view_perma_grids && (c_ptr->info & GRID_GLOW)) {
            c_ptr->info |= GRID_MARK;
        }
    }


    /* Wipe the object */
    WIPE(i_ptr, inven_type);


    /* Object is gone */
    c_ptr->i_idx = 0;

    /* Visual update */
    lite_spot(y, x);
    
    
    /* Count the objects */
    i_cnt--;
}


/*
 * Deletes object from given location
 */
void delete_object(int y, int x)
{
    cave_type *c_ptr;

    /* Refuse "illegal" locations */
    if (!in_bounds(y, x)) return;

    /* Find where it was */
    c_ptr = &cave[y][x];

    /* Delete the object */
    if (c_ptr->i_idx >= MIN_I_IDX) delete_object_idx(c_ptr->i_idx);
}



/*
 * Compact the object list because it is getting too full.
 *
 * This function is called only by "process_objects()".
 */
static void compact_objects(int size)
{
    int			i, y, x, num, cnt;
    int                 cur_lev, cur_dis, chance;


    /* Debugging message */
    msg_print("Compacting objects...");

    /* Compact at least 'size' objects */
    for (num = 0, cnt = 1; num <= size; cnt++) {

        /* Get more vicious each iteration */
        cur_lev = 5 * cnt;

        /* Get closer each iteration */
        cur_dis = 5 * (20 - cnt);

        /* Examine the objects */
        for (i = MIN_I_IDX; i < i_max; i++) {
        
            inven_type *i_ptr = &i_list[i];
            inven_kind *k_ptr = &k_list[i_ptr->k_idx];
            
            /* Skip dead objects */
            if (!i_ptr->k_idx) continue;

            /* Hack -- High level objects start out "immune" */
            if (k_ptr->level > cur_lev) continue;

            /* Get the location */
            y = i_ptr->iy;
            x = i_ptr->ix;
                        
            /* Nearby objects start out "immune" */
            if ((cur_dis > 0) && (distance(py, px, y, x) < cur_dis)) continue;

            /* Every object gets a "saving throw" */
            switch (i_ptr->tval) {
                case TV_STORE_DOOR:
                case TV_UP_STAIR:
                case TV_DOWN_STAIR:
                    chance = 100;
                case TV_VIS_TRAP:
                    chance = 85;
                    break;
                case TV_RUBBLE:
                case TV_INVIS_TRAP:
                case TV_OPEN_DOOR:
                case TV_CLOSED_DOOR:
                    chance = 95;
                    break;
                case TV_SECRET_DOOR:
                    chance = 98;
                    break;
                default:
                    chance = 90;
            }

            /* Hack -- only compact artifacts in emergencies */
            if (artifact_p(i_ptr) && (cnt < 1000)) chance = 100;
            
            /* Apply the saving throw */
            if (rand_int(100) < chance) continue;

            /* Delete it */
            delete_object_idx(i);

            /* Count it */
            num++;
        }
    }


    /* Update stuff */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);
    p_ptr->update |= (PU_MONSTERS);

    /* Redraw */
    p_ptr->redraw |= (PR_MAP);
}




/*
 * Delete all the items when player leaves the level
 * Note -- we do NOT visually reflect these (irrelevant) changes
 */
void wipe_i_list()
{
    int i;

    /* Delete the existing objects (backwards!) */
    for (i = i_max - 1; i >= MIN_I_IDX; i--) {

        inven_type *i_ptr = &i_list[i];
        
        int y = i_ptr->iy;
        int x = i_ptr->ix;

        cave_type *c_ptr = &cave[y][x];

        /* Skip dead objects */
        if (!i_ptr->k_idx) continue;
        
        /* Mega-Hack -- preserve artifacts */
        if (p_ptr->preserve) {

            /* Hack -- Preserve artifacts */
            if (artifact_p(i_ptr)) {

                /* Mega-Hack -- Preserve the artifact */
                v_list[i_ptr->name1].cur_num = 0;
            }
        }

        /* Wipe the object */
        WIPE(i_ptr, inven_type);

        /* Object is gone */
        c_ptr->i_idx = 0;
    }

    /* Restart free/heap pointers */
    i_nxt = i_max = MIN_I_IDX;

    /* No more objects */
    i_cnt = 0;
}


/*
 * Acquires and returns the index of a "free" item.
 */
int i_pop(void)
{
    int i, n;

    /* Normal allocation */
    if (i_max < MAX_I_IDX) {

        /* Get next space */
        i = i_max;

        /* Expand object array */
        i_max++;
                
        /* Count the objects */
        i_cnt++;
        
        /* Use this object */
        return (i);
    }
    
    /* Check for some space */
    for (n = MIN_I_IDX; n < MAX_I_IDX; n++) {

        /* Get next space */
        i = i_nxt;
        
        /* Advance (and wrap) the "next" pointer */
        if (++i_nxt >= MAX_I_IDX) i_nxt = MIN_I_IDX;        

        /* Skip objects in use */
        if (i_list[i].k_idx) continue;

        /* Count the objects */
        i_cnt++;
                
        /* Use this object */
        return (i);
    }

    /* XXX XXX XXX Mega-Warning */
    msg_print("Unable to create a new object!");

    /* Try not to crash */
    return (0);
}




/*
 * Choose an object kind that seems "appropriate" to the given level
 *
 * This function uses the "allocation table" built in "init.c".
 *
 * There is a small chance (1/20) of "boosting" the given depth by
 * a potentially large amount (see below).
 *
 * It is (slightly) more likely to acquire an object of the given level
 * than one of a lower level.  This is done by choosing several objects
 * appropriate to the given level and keeping the "hardest" one.
 */
int get_obj_num(int level)
{
    int i, try, k_idx;


    /* Obtain the table */
    s16b *t_lev = alloc_kind_index;
    kind_entry *table = alloc_kind_table;


    /* Pick an object */
    while (1) {

        /* Town level is easy */
        if (level <= 0) {

            /* Pick a level 0 entry */
            i = rand_int(t_lev[0]);
        }

        /* Other levels sometimes have great stuff */
        else {

            /* Start at the given level */
            try = level;
            
            /* Occasionally, get a "better" object */
            if (rand_int(GREAT_OBJ) == 0) {

                /* What a bizarre calculation */
                try = 1 + (try * MAX_K_LEV / randint(MAX_K_LEV));
                if (try > MAX_K_LEV - 1) try = MAX_K_LEV - 1;
            }

            /* Maximum level */
            if (try > MAX_K_LEV) try = MAX_K_LEV - 1;

            /* Pick any object at or below the given level */
            i = rand_int(t_lev[try]);

            /* Sometimes, try for a "better" item */
            if (rand_int(3) != 0) {

                /* Pick another object at or below the given level */
                int j = rand_int(t_lev[try]);

                /* Keep it if it is "better" */
                if (table[i].locale < table[j].locale) i = j;
            }

            /* Sometimes, try for a "better" item */
            if (rand_int(3) != 0) {

                /* Pick another object at or below the given level */
                int j = rand_int(t_lev[try]);

                /* Keep it if it is "better" */
                if (table[i].locale < table[j].locale) i = j;
            }
        }

        /* Access the "k_idx" of the chosen item */
        k_idx = table[i].k_idx;

        /* Play the "chance game" */
        if (rand_int(table[i].chance) != 0) continue;

        /* Sounds good */
        return (k_idx);
    }

    /* Paranoia */
    return (0);
}






/*
 * Known is true when the "attributes" of an object are "known".
 * These include tohit, todam, toac, cost, and pval (charges).
 *
 * Note that "knowing" an object gives you everything that an "awareness"
 * gives you, and much more.  In fact, the player is always "aware" of any
 * item of which he has full "knowledge".
 *
 * But having full knowledge of, say, one "wand of wonder", does not, by
 * itself, give you knowledge, or even awareness, of other "wands of wonder".
 * It happens that most "identify" routines (including "buying from a shop")
 * will make the player "aware" of the object as well as fully "know" it.
 *
 * This routine also removes inscriptions generated by "feelings".
 */
void inven_known(inven_type *i_ptr)
{    
    /* Remove "default inscriptions" */
    if (i_ptr->note && (i_ptr->ident & ID_SENSE)) {

        /* Access the inscription */
        cptr q = quark_str(i_ptr->note);

        /* Hack -- Remove auto-inscriptions */
        if ((streq(q, "cursed")) ||
            (streq(q, "broken")) ||
            (streq(q, "good")) ||
            (streq(q, "average")) ||
            (streq(q, "excellent")) ||
            (streq(q, "worthless")) ||
            (streq(q, "special")) ||
            (streq(q, "terrible"))) {

            /* Forget the inscription */
            i_ptr->note = 0;
        }
    }

    /* Clear the "Felt" info */
    i_ptr->ident &= ~ID_SENSE;

    /* Clear the "Empty" info */
    i_ptr->ident &= ~ID_EMPTY;

    /* Now we know about the item */
    i_ptr->ident |= ID_KNOWN;
}





/*
 * The player is now aware of the effects of the given object.
 */
void inven_aware(inven_type *i_ptr)
{
    /* Fully aware of the effects */
    x_list[i_ptr->k_idx].aware = TRUE;
}



/*
 * Something has been "sampled"
 */
void inven_tried(inven_type *i_ptr)
{
    /* Mark it as tried (even if "aware") */
    x_list[i_ptr->k_idx].tried = TRUE;
}




/*
 * Helper function.  Compare the "ident" field of two objects.
 * Determine if the "ident" fields of two items "match".
 */
static bool similar_ident(inven_type *i_ptr, inven_type *j_ptr)
{
    /* Hack -- Force low-level flag if allowed */
    if (inven_known_p(i_ptr)) i_ptr->ident |= ID_KNOWN;
    if (inven_known_p(j_ptr)) j_ptr->ident |= ID_KNOWN;


    /* Require identical "sensed item" status */
    if ((i_ptr->ident & ID_SENSE) != (j_ptr->ident & ID_SENSE)) return (0);

#if 0
    /* Require identical "fixed price" status */
    if ((i_ptr->ident & ID_FIXED) != (j_ptr->ident & ID_FIXED)) return (0);
#endif

    /* Require identical "emptiness" */
    if ((i_ptr->ident & ID_EMPTY) != (j_ptr->ident & ID_EMPTY)) return (0);

    /* Require identical "fixed price" status */
    if ((i_ptr->ident & ID_KNOWN) != (j_ptr->ident & ID_KNOWN)) return (0);

    /* Require identical "rumour" */
    if ((i_ptr->ident & ID_RUMOUR) != (j_ptr->ident & ID_RUMOUR)) return (0);

    /* Require identical "mental" */
    if ((i_ptr->ident & ID_MENTAL) != (j_ptr->ident & ID_MENTAL)) return (0);

    /* Require identical "cursed" */
    if ((i_ptr->ident & ID_CURSED) != (j_ptr->ident & ID_CURSED)) return (0);

    /* Require identical "broken" */
    if ((i_ptr->ident & ID_BROKEN) != (j_ptr->ident & ID_BROKEN)) return (0);


    /* Food, Potions, Scrolls, and Rods are "simple" objects */
    if (i_ptr->tval == TV_FOOD) return (1);
    if (i_ptr->tval == TV_POTION) return (1);
    if (i_ptr->tval == TV_SCROLL) return (1);
    if (i_ptr->tval == TV_ROD) return (1);

    /* Normally require both items to be "known" */
    if ((i_ptr->ident & ID_KNOWN) && (j_ptr->ident & ID_KNOWN)) return (1);


    /* Assume no match */
    return (0);
}




/*
 * Determine if an item can "absorb" a second item
 *
 * No object can absorb itself.  This prevents object replication.
 *
 * When an object absorbs another, the second object loses all
 * of its attributes (except for "number", which gets added in),
 * so it is important to verify that all important attributes match.
 *
 * We allow wands (and staffs) to combine if they are known to have
 * equivalent charges.  They are unstacked as they are used.
 * We allow rods to combine when they are fully charged, and again,
 * we unstack them as they are used (see effects.c).
 *
 * We do NOT allow activatable items (artifacts or dragon scale mail)
 * to stack, to keep the "activation" code clean.  Artifacts may stack,
 * but only with another identical artifact (which does not exist).
 * Ego items may stack as long as they have the same ego-item type.
 */
bool item_similar(inven_type *i_ptr, inven_type *j_ptr)
{
    /* Hack -- Identical items cannot be stacked */
    if (i_ptr == j_ptr) return (0);


    /* Different objects types cannot be stacked */
    if (i_ptr->k_idx != j_ptr->k_idx) return (0);


    /* Hack -- refuse weapons/armor if requested */
    if (!stack_allow_items &&
        (wearable_p(i_ptr)) &&
        (i_ptr->tval != TV_LITE) &&
        (i_ptr->tval != TV_RING) &&
        (i_ptr->tval != TV_AMULET)) return (0);

    /* Hack -- refuse wands/staffs/rods if requested */
    if (!stack_allow_wands &&
        ((i_ptr->tval == TV_STAFF) ||
         (i_ptr->tval == TV_WAND) ||
         (i_ptr->tval == TV_ROD))) return (0);


    /* Hack -- normally require matching "discounts" */
    if (!stack_force_costs && (i_ptr->discount != j_ptr->discount)) return (0);

    /* Hack -- normally require matching "inscriptions" */
    if (!stack_force_notes && (i_ptr->note != j_ptr->note)) return (0);


    /* Require identical "pval" codes */
    if (i_ptr->pval != j_ptr->pval) return (0);

    /* Require many identical values */
    if ((i_ptr->tohit     != j_ptr->tohit)     ||
        (i_ptr->todam     != j_ptr->todam)     ||
        (i_ptr->toac      != j_ptr->toac)      ||
        (i_ptr->ac        != j_ptr->ac)        ||
        (i_ptr->dd        != j_ptr->dd)        ||
        (i_ptr->ds        != j_ptr->ds)) {
        return (0);
    }

    /* Require identical flags */
    if ((i_ptr->flags1 != j_ptr->flags1) ||
        (i_ptr->flags2 != j_ptr->flags2) ||
        (i_ptr->flags3 != j_ptr->flags3)) {
        return (0);
    }


    /* Both items must be "fully identified" (see above) */
    if (!similar_ident(i_ptr, j_ptr)) return (0);


    /* Require identical "artifact" names */
    if (i_ptr->name1 != j_ptr->name1) return (0);

    /* Require identical "ego-item" names */
    if (i_ptr->name2 != j_ptr->name2) return (0);


    /* XXX Hack -- never stack "activatable" items */
    if (wearable_p(i_ptr) && (i_ptr->flags3 & TR3_ACTIVATE)) return (0);
    if (wearable_p(j_ptr) && (j_ptr->flags3 & TR3_ACTIVATE)) return (0);


    /* Hack -- Never stack chests */
    if (i_ptr->tval == TV_CHEST) return (0);


    /* No stack can grow bigger than a certain size */
    if (i_ptr->number + j_ptr->number >= MAX_STACK_SIZE) return (0);



    /* Paranoia -- Different weights cannot be stacked */
    if (i_ptr->weight != j_ptr->weight) return (0);

    /* Paranoia -- Timeout should always be zero (see "TR3_ACTIVATE" above) */
    if (i_ptr->timeout || j_ptr->timeout) return (0);


    /* They match, so they must be similar */
    return (TRUE);
}



/*
 * Find the index of the inven_kind with the given tval and sval
 */
int lookup_kind(int tval, int sval)
{
    int k;

    /* Look for it */
    for (k = 0; k < MAX_K_IDX; k++) {

        inven_kind *k_ptr = &k_list[k];
        
        /* Found a match */
        if ((k_ptr->tval == tval) && (k_ptr->sval == sval)) return (k);
    }

    /* Oops */
    msg_format("No object (%d,%d)", tval, sval);
    
    /* Oops */
    return (0);
}


/*
 * Clear an item
 */
void invwipe(inven_type *i_ptr)
{
    /* Clear the record */
    WIPE(i_ptr, inven_type);
}


/*
 * Make "i_ptr" a "clean" copy of the given "kind" of object
 */
void invcopy(inven_type *i_ptr, int k_idx)
{
    inven_kind *k_ptr = &k_list[k_idx];

    /* Clear the record */
    WIPE(i_ptr, inven_type);
    
    /* Save the kind index */
    i_ptr->k_idx = k_idx;

    /* Quick access to tval/sval */
    i_ptr->tval = k_ptr->tval;
    i_ptr->sval = k_ptr->sval;

    /* Save the default "pval" */
    i_ptr->pval = k_ptr->pval;

    /* Default number */
    i_ptr->number = 1;

    /* Default weight */
    i_ptr->weight = k_ptr->weight;

    /* Default magic */
    i_ptr->tohit = k_ptr->tohit;
    i_ptr->todam = k_ptr->todam;
    i_ptr->toac = k_ptr->toac;
    i_ptr->ac = k_ptr->ac;
    i_ptr->dd = k_ptr->dd;
    i_ptr->ds = k_ptr->ds;

    /* Default flags */
    i_ptr->flags1 = k_ptr->flags1;
    i_ptr->flags2 = k_ptr->flags2;
    i_ptr->flags3 = k_ptr->flags3;


    /* Hack -- worthless items are always "broken" */
    if (k_ptr->cost <= 0) i_ptr->ident |= ID_BROKEN;

    /* Hack -- cursed items are always "cursed" */
    if (k_ptr->flags3 & TR3_CURSED) i_ptr->ident |= ID_CURSED;
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
 *
 * Note that since randnor() may return a negative value, a "lot" of bonuses
 * will just be the "base" bonus, even at higher levels, where the base bonus
 * will be awarded more than it would be under a strictly normal distribution.
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
    if (rand_int(MAG_MAX_LEVEL) < remainder) mean_bonus_per_level++;


    /*
     * The "standard" object will differ from the mean by 50%. So as the
     * mean increases, the bonus distribution widens. Since the mean was
     * calculated from the level, the bonus distribution widens with the
     * level as well.
     */

    std_dev = mean_bonus_per_level / 2;

    /* Now compensate for any "lost" remainder. */
    if ((mean_bonus_per_level % 2) && rand_int(2)) std_dev++;

    /* XXX Hack -- repair apparent problem in "randnor()" */
    std_dev = (std_dev * 125) / 100;

    /* Always allow some variety. */
    if (std_dev < 1) std_dev = 1;

    /* Get a random number from a normal distribution. */
    x = randnor(0, std_dev);

    /* Now, shift the normal distribution over to the desired mean. */
    x += mean_bonus_per_level;


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
    char i_name[80];

    /* Describe */
    objdes_store(i_name, i_ptr, FALSE, 0);

    /* Silly message */
    msg_format("%s", i_name);
}




/*
 * Mega-Hack -- Attempt to create one of the "Special Objects"
 *
 * We are only called from "place_object()".
 */
static bool make_artifact_special(inven_type *i_ptr)
{
    int			i, what = 0;

    int			k_idx = 0;


    /* No artifacts in the town */
    if (!dun_level) return (FALSE);
    
    
    /* Check the artifact list (just the "specials") */
    for (i = 0; i < ART_MIN_NORMAL; i++) {

        /* Skip "empty" artifacts */
        if (!v_list[i].name) continue;
        
        /* Cannot make an artifact twice */
        if (v_list[i].cur_num) continue;

        /* XXX XXX Enforce minimum "depth" (loosely) */
        if (v_list[i].level > dun_level) {

            /* Acquire the "out-of-depth factor" */
            int d = (v_list[i].level - dun_level) * 5;
        
            /* Roll for out-of-depth creation */
            if (rand_int(d) != 0) continue;
        }

        /* Impossible "rarity roll" */
        if (!v_list[i].rarity) continue;
        
        /* Artifact "rarity roll" */
        if (rand_int(v_list[i].rarity) != 0) return (0);

        /* Find the base object */
        k_idx = lookup_kind(v_list[i].tval, v_list[i].sval);
    
        /* XXX XXX Enforce minimum "object" level (loosely) */
        if (k_list[k_idx].level > object_level) {

            /* Acquire the "out-of-depth factor" */
            int d = (k_list[k_idx].level - object_level) * 5;
        
            /* Roll for out-of-depth creation */
            if (rand_int(d) != 0) continue;
        }

        /* Assign the template */
        invcopy(i_ptr, k_idx);

        /* Return the artifact index */
        what = i;

        /* Okay */        
        break;
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

    /* Mega-Hack -- increase the rating */
    rating += 10;

    /* Mega-Hack -- increase the rating again */
    if (v_list[what].cost > 50000L) rating += 10;

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

    /* No artifacts in the town */
    if (!dun_level) return (FALSE);
    
    /* Hack -- no such thing as "plural" artifacts */
    if (i_ptr->number != 1) return (FALSE);

    /* XXX Note -- see "make_artifact_special()" as well */

    /* Check the artifact list (skip the "specials") */
    for (i = ART_MIN_NORMAL; i < ART_MAX; i++) {

        /* Skip "empty" artifacts */
        if (!v_list[i].name) continue;
        
        /* Cannot make an artifact twice */
        if (v_list[i].cur_num) continue;

        /* Must have the correct fields */
        if (v_list[i].tval != i_ptr->tval) continue;
        if (v_list[i].sval != i_ptr->sval) continue;

        /* XXX XXX Enforce minimum "depth" (loosely) */
        if (v_list[i].level > dun_level) {

            /* Acquire the "out-of-depth factor" */
            int d = (v_list[i].level - dun_level) * 5;
        
            /* Roll for out-of-depth creation */
            if (rand_int(d) != 0) continue;
        }

        /* Impossible "rarity roll" */
        if (!v_list[i].rarity) continue;
        
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

    /* Mega-Hack -- increase the rating */
    rating += 10;

    /* Mega-Hack -- increase the rating again */
    if (v_list[what].cost > 50000L) rating += 10;

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
 * Charge a new wand.
 */
static void charge_wand(inven_type *i_ptr)
{
    switch (i_ptr->sval) {
      case SV_WAND_HEAL_MONSTER:	i_ptr->pval = randint(20) + 8; break;
      case SV_WAND_HASTE_MONSTER:	i_ptr->pval = randint(20) + 8; break;
      case SV_WAND_CLONE_MONSTER:	i_ptr->pval = randint(5)  + 3; break;
      case SV_WAND_TELEPORT_AWAY:	i_ptr->pval = randint(5)  + 6; break;
      case SV_WAND_DISARMING:		i_ptr->pval = randint(5)  + 4; break;
      case SV_WAND_TRAP_DOOR_DEST:	i_ptr->pval = randint(8)  + 6; break;
      case SV_WAND_STONE_TO_MUD:	i_ptr->pval = randint(4)  + 3; break;
      case SV_WAND_LITE:		i_ptr->pval = randint(10) + 6; break;
      case SV_WAND_SLEEP_MONSTER:	i_ptr->pval = randint(15) + 8; break;
      case SV_WAND_SLOW_MONSTER:	i_ptr->pval = randint(10) + 6; break;
      case SV_WAND_CONFUSE_MONSTER:	i_ptr->pval = randint(12) + 6; break;
      case SV_WAND_FEAR_MONSTER:	i_ptr->pval = randint(5)  + 3; break;
      case SV_WAND_DRAIN_LIFE:		i_ptr->pval = randint(3)  + 3; break;
      case SV_WAND_POLYMORPH:		i_ptr->pval = randint(8)  + 6; break;
      case SV_WAND_STINKING_CLOUD:	i_ptr->pval = randint(8)  + 6; break;
      case SV_WAND_MAGIC_MISSILE:	i_ptr->pval = randint(10) + 6; break;
      case SV_WAND_ACID_BOLT:		i_ptr->pval = randint(8)  + 6; break;
      case SV_WAND_ELEC_BOLT:		i_ptr->pval = randint(8)  + 6; break;
      case SV_WAND_FIRE_BOLT:		i_ptr->pval = randint(8)  + 6; break;
      case SV_WAND_COLD_BOLT:		i_ptr->pval = randint(5)  + 6; break;
      case SV_WAND_ACID_BALL:		i_ptr->pval = randint(5)  + 2; break;
      case SV_WAND_ELEC_BALL:		i_ptr->pval = randint(8)  + 4; break;
      case SV_WAND_FIRE_BALL:		i_ptr->pval = randint(4)  + 2; break;
      case SV_WAND_COLD_BALL:		i_ptr->pval = randint(6)  + 2; break;
      case SV_WAND_WONDER:		i_ptr->pval = randint(15) + 8; break;
      case SV_WAND_ANNIHILATION:	i_ptr->pval = randint(2)  + 1; break;
      case SV_WAND_DRAGON_FIRE:		i_ptr->pval = randint(3)  + 1; break;
      case SV_WAND_DRAGON_COLD:		i_ptr->pval = randint(3)  + 1; break;
      case SV_WAND_DRAGON_BREATH:	i_ptr->pval = randint(3)  + 1; break;
    }
}



/*
 * Charge a new staff.
 */
static void charge_staff(inven_type *i_ptr)
{
    switch (i_ptr->sval) {
      case SV_STAFF_DARKNESS:		i_ptr->pval = randint(8)  + 8; break;
      case SV_STAFF_SLOWNESS:		i_ptr->pval = randint(8)  + 8; break;
      case SV_STAFF_HASTE_MONSTERS:	i_ptr->pval = randint(8)  + 8; break;
      case SV_STAFF_SUMMONING:		i_ptr->pval = randint(3)  + 1; break;
      case SV_STAFF_TELEPORTATION:	i_ptr->pval = randint(4)  + 5; break;
      case SV_STAFF_IDENTIFY:		i_ptr->pval = randint(15) + 5; break;
      case SV_STAFF_REMOVE_CURSE:	i_ptr->pval = randint(3)  + 4; break;
      case SV_STAFF_STARLITE:		i_ptr->pval = randint(5)  + 6; break;
      case SV_STAFF_LITE:		i_ptr->pval = randint(20) + 8; break;
      case SV_STAFF_MAPPING:		i_ptr->pval = randint(5)  + 5; break;
      case SV_STAFF_DETECT_GOLD:	i_ptr->pval = randint(20) + 8; break;
      case SV_STAFF_DETECT_ITEM:	i_ptr->pval = randint(15) + 6; break;
      case SV_STAFF_DETECT_TRAP:	i_ptr->pval = randint(5)  + 6; break;
      case SV_STAFF_DETECT_DOOR:	i_ptr->pval = randint(8)  + 6; break;
      case SV_STAFF_DETECT_INVIS:	i_ptr->pval = randint(15) + 8; break;
      case SV_STAFF_DETECT_EVIL:	i_ptr->pval = randint(15) + 8; break;
      case SV_STAFF_CURE_LIGHT:		i_ptr->pval = randint(5)  + 6; break;
      case SV_STAFF_CURING:		i_ptr->pval = randint(3)  + 4; break;
      case SV_STAFF_HEALING:		i_ptr->pval = randint(2)  + 1; break;
      case SV_STAFF_THE_MAGI:		i_ptr->pval = randint(2)  + 2; break;
      case SV_STAFF_SLEEP_MONSTERS:	i_ptr->pval = randint(5)  + 6; break;
      case SV_STAFF_SLOW_MONSTERS:	i_ptr->pval = randint(5)  + 6; break;
      case SV_STAFF_SPEED:		i_ptr->pval = randint(3)  + 4; break;
      case SV_STAFF_PROBING:		i_ptr->pval = randint(6)  + 2; break;
      case SV_STAFF_DISPEL_EVIL:	i_ptr->pval = randint(3)  + 4; break;
      case SV_STAFF_POWER:		i_ptr->pval = randint(3)  + 1; break;
      case SV_STAFF_HOLINESS:		i_ptr->pval = randint(2)  + 2; break;
      case SV_STAFF_GENOCIDE:		i_ptr->pval = randint(2)  + 1; break;
      case SV_STAFF_EARTHQUAKES:	i_ptr->pval = randint(5)  + 3; break;
      case SV_STAFF_DESTRUCTION:	i_ptr->pval = randint(3)  + 1; break;
    }
}



/*
 * Apply magic to armour-type objects
 */
static void a_m_aux_1(inven_type *i_ptr, int level, bool okay, bool good, bool great)
{
    int chance, special, cursed;


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
                }
                else if (rand_int(2) == 0) {
                    give_1_lo_resist(i_ptr);
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
                        give_1_hi_resist(i_ptr);
                        i_ptr->name2 = EGO_ELVENKIND;
                        rating += 25;
                        if (cheat_peek) inven_mention(i_ptr);
                    }
                    else {
                        i_ptr->toac += 10;
                        i_ptr->name2 = EGO_RESIST;
                        rating += 20;
                        if (cheat_peek) inven_mention(i_ptr);
                    }
                    break;

                  case 2:
                    i_ptr->flags2 |= (TR2_RES_ACID);
                    i_ptr->flags3 |= (TR3_IGNORE_ACID);
                    i_ptr->name2 = EGO_RESIST_A;
                    rating += 18;
                    if (cheat_peek) inven_mention(i_ptr);
                    break;

                  case 3: case 4:
                    i_ptr->flags2 |= (TR2_RES_FIRE);
                    i_ptr->flags3 |= (TR3_IGNORE_FIRE);
                    i_ptr->name2 = EGO_RESIST_F;
                    rating += 17;
                    if (cheat_peek) inven_mention(i_ptr);
                    break;

                  case 5: case 6:
                    i_ptr->flags2 |= (TR2_RES_COLD);
                    i_ptr->flags3 |= (TR3_IGNORE_COLD);
                    i_ptr->name2 = EGO_RESIST_C;
                    rating += 16;
                    if (cheat_peek) inven_mention(i_ptr);
                    break;

                  case 7: case 8: case 9:
                    i_ptr->flags2 |= (TR2_RES_ELEC);
                    i_ptr->flags3 |= (TR3_IGNORE_ELEC);
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
                    i_ptr->name2 = EGO_FREE_ACTION;
                    rating += 11;
                    if (cheat_peek) inven_mention(i_ptr);
                    break;

                  case 4: case 5: case 6:
                    i_ptr->flags3 |= (TR3_SHOW_MODS);
                    i_ptr->tohit += 1 + randint(4);
                    i_ptr->todam += 1 + randint(4);
                    i_ptr->name2 = EGO_SLAYING;
                    rating += 17;
                    if (cheat_peek) inven_mention(i_ptr);
                    break;

                  case 7: case 8: case 9:
                    i_ptr->flags1 |= (TR1_DEX);
                    i_ptr->pval = 2 + randint(2);	/* +N DEX */
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

                /* Permanently broken */
                i_ptr->ident |= ID_BROKEN;

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

                    /* Increase the rating */
                    rating += (25 + i_ptr->pval);
                    if (cheat_peek) inven_mention(i_ptr);

                    break;

                  case 2: case 3: case 4: case 5:
                    i_ptr->flags2 |= (TR2_FREE_ACT);
                    i_ptr->name2 = EGO_FREE_ACTION;
                    rating += 15;
                    break;

                  case 6: case 7: case 8: case 9:
                  case 10: case 11: case 12: case 13:
                    i_ptr->flags1 |= TR1_STEALTH;
                    i_ptr->flags3 |= TR3_HIDE_TYPE;
                    i_ptr->pval = randint(3);	/* +N Stealth */
                    i_ptr->name2 = EGO_STEALTH;
                    rating += 16;
                    break;

                  default:
                    i_ptr->flags3 |= TR3_FEATHER;
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

                /* Permanently broken */
                i_ptr->ident |= ID_BROKEN;

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
                        i_ptr->name2 = EGO_MIGHT;
                        rating += 19;
                        if (cheat_peek) inven_mention(i_ptr);
                        break;

                      case 2:
                        i_ptr->flags1 |= (TR1_CHR | TR1_WIS);
                        i_ptr->flags2 |= (TR2_SUST_CHR | TR2_SUST_WIS);
                        i_ptr->pval = randint(3);	/* +N WIS/CHR */
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
                        i_ptr->name2 = EGO_MAGI;
                        rating += 15;
                        if (cheat_peek) inven_mention(i_ptr);
                        break;

                      case 4:
                        i_ptr->flags1 |= TR1_CHR;
                        i_ptr->flags2 |= TR2_SUST_CHR;
                        i_ptr->pval = randint(4);	/* +N CHR */
                        i_ptr->name2 = EGO_BEAUTY;
                        rating += 8;
                        if (cheat_peek) inven_mention(i_ptr);
                        break;

                      case 5:
                        i_ptr->flags1 |= TR1_SEARCH;
                        i_ptr->flags2 |= TR2_RES_BLIND;
                        i_ptr->flags3 |= TR3_SEE_INVIS;
                        i_ptr->pval = (1 + randint(4));	/* +N Search */
                        i_ptr->name2 = EGO_SEEING;
                        rating += 8;
                        if (cheat_peek) inven_mention(i_ptr);
                        break;

                      case 6:
                        i_ptr->flags3 |= TR3_REGEN;
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

                /* Permanently broken */
                i_ptr->ident |= ID_BROKEN;

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
                        i_ptr->name2 = EGO_INTELLIGENCE;
                        rating += 13;
                        if (cheat_peek) inven_mention(i_ptr);
                        break;

                      case 3: case 4: case 5:
                        i_ptr->flags1 |= TR1_WIS;
                        i_ptr->flags2 |= TR2_SUST_WIS;
                        i_ptr->pval = randint(2);	/* +N Wis */
                        i_ptr->name2 = EGO_WISDOM;
                        rating += 13;
                        if (cheat_peek) inven_mention(i_ptr);
                        break;

                      case 6: case 7: case 8: case 9:
                        i_ptr->flags1 |= TR1_INFRA;
                        i_ptr->flags3 |= TR3_HIDE_TYPE;
                        i_ptr->pval = 1 + randint(4);	/* +N Infra */
                        i_ptr->name2 = EGO_INFRAVISION;
                        rating += 11;
                        if (cheat_peek) inven_mention(i_ptr);
                        break;

                      case 10: case 11:
                        i_ptr->flags2 |= (TR2_RES_LITE);
                        i_ptr->flags3 |= (TR3_LITE);
                        i_ptr->name2 = EGO_LITE;
                        rating += 6;
                        if (cheat_peek) inven_mention(i_ptr);
                        break;

                      case 12: case 13:
                        i_ptr->flags2 |= TR2_RES_BLIND;
                        i_ptr->flags3 |= TR3_SEE_INVIS;
                        i_ptr->name2 = EGO_SEEING;
                        if (rand_int(5) == 0) {
                            i_ptr->flags1 |= TR1_SEARCH;
                            i_ptr->pval = randint(2);	/* +N Search */
                        }
                        rating += 8;
                        if (cheat_peek) inven_mention(i_ptr);
                        break;

                     default: /* case 14: */
                        i_ptr->flags3 |= TR3_TELEPATHY;
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

                /* Permanently broken */
                i_ptr->ident |= ID_BROKEN;

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
                    rating += 16;
                }
                else {
                    i_ptr->toac += m_bonus(3, 10, level);
                    i_ptr->pval = randint(3);	/* +N Stealth */
                    i_ptr->flags1 |= TR1_STEALTH;
                    i_ptr->flags3 |= TR3_HIDE_TYPE;
                    i_ptr->name2 = EGO_STEALTH;
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

                /* Permanently broken */
                i_ptr->ident |= ID_BROKEN;

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
    }
}


/*
 * Apply magic to non-armour-type objects
 */
static void a_m_aux_2(inven_type *i_ptr, int level, bool okay, bool good, bool great)
{
    int chance, special, cursed;


    /* Extract the "chance" of "goodness" */
    chance = MAG_BASE_MAGIC + level;
    if (chance > MAG_BASE_MAX) chance = MAG_BASE_MAX;

    /* Extract the "chance" of "greatness" (approx range 3-18 percent) */
    special = (10 * chance) / MAG_DIV_SPECIAL;

    /* Extract the "chance" of ickiness (approx range 11-54 percent) */
    cursed = (10 * chance) / MAG_DIV_CURSED;


    /* Apply magic (good or bad) according to type */
    switch (i_ptr->tval) {


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

                /* Permanently broken */
                i_ptr->ident |= ID_BROKEN;

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

                    i_ptr->name2 = EGO_FIRE;
                    i_ptr->flags1 |= (TR1_BRAND_FIRE);
                    i_ptr->flags3 |= (TR3_IGNORE_FIRE);

                    /* Better stats */
                    i_ptr->tohit += 5;
                    i_ptr->todam += 5;

                    /* Super-charge the whips */
                    while (rand_int(i_ptr->dd * 5L) == 0) i_ptr->dd++;


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

                    /* Sustain a random Stat */
                    switch (randint(6)) {
                        case 1: i_ptr->flags2 |= (TR2_SUST_STR); break;
                        case 2: i_ptr->flags2 |= (TR2_SUST_INT); break;
                        case 3: i_ptr->flags2 |= (TR2_SUST_WIS); break;
                        case 4: i_ptr->flags2 |= (TR2_SUST_DEX); break;
                        case 5: i_ptr->flags2 |= (TR2_SUST_CON); break;
                        case 6: i_ptr->flags2 |= (TR2_SUST_CHR); break;
                    }

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
                    i_ptr->name2 = EGO_DF;
                    rating += 23;
                    break;

                  case 3: case 4:
                    i_ptr->flags1 |= (TR1_BRAND_FIRE);
                    i_ptr->flags2 |= (TR2_RES_FIRE);
                    i_ptr->flags3 |= (TR3_IGNORE_FIRE);
                    i_ptr->tohit += 2;
                    i_ptr->todam += 2;
                    i_ptr->name2 = EGO_FT;
                    rating += 20;
                    break;

                  case 5: case 6:
                    i_ptr->flags1 |= (TR1_BRAND_COLD);
                    i_ptr->flags2 |= (TR2_RES_COLD);
                    i_ptr->flags3 |= (TR3_IGNORE_COLD);
                    i_ptr->tohit += 2;
                    i_ptr->todam += 2;
                    i_ptr->name2 = EGO_FB;
                    rating += 20;
                    break;

                  case 7: case 8:
                    i_ptr->flags1 |= TR1_SLAY_ANIMAL;
                    i_ptr->tohit += 3;
                    i_ptr->todam += 3;
                    i_ptr->name2 = EGO_SLAY_ANIMAL;
                    rating += 15;
                    break;

                  case 9: case 10:
                    i_ptr->flags1 |= TR1_SLAY_DRAGON;
                    i_ptr->tohit += 3;
                    i_ptr->todam += 3;
                    i_ptr->name2 = EGO_SLAY_DRAGON;
                    rating += 18;
                    break;

                  case 11: case 12:
                    i_ptr->flags1 |= TR1_SLAY_EVIL;
                    i_ptr->tohit += 3;
                    i_ptr->todam += 3;
                    i_ptr->name2 = EGO_SLAY_EVIL;

                    /* One in three is also a blessed wisdom booster */
                    if (rand_int(3) == 0) {
                        i_ptr->flags1 |= (TR1_WIS);
                        i_ptr->flags3 |= (TR3_BLESSED);
                        i_ptr->pval = 1;	/* +1 Wisdom */
                    }

                    rating += 18;

                    break;

                  case 13: case 14:
                    i_ptr->flags1 |= (TR1_SLAY_UNDEAD);
                    i_ptr->flags3 |= (TR3_SEE_INVIS);
                    i_ptr->tohit += 2;
                    i_ptr->todam += 2;
                    i_ptr->name2 = EGO_SLAY_UNDEAD;

                    /* One in three is also a Life Holder */
                    if (rand_int(3) == 0) {
                        i_ptr->flags2 |= (TR2_HOLD_LIFE);
                    }

                    rating += 18;

                    break;

                  case 15: case 16: case 17:
                    i_ptr->flags1 |= TR1_SLAY_ORC;
                    i_ptr->tohit += 2;
                    i_ptr->todam += 2;
                    i_ptr->name2 = EGO_SLAY_ORC;
                    rating += 13;
                    break;

                  case 18: case 19: case 20:
                    i_ptr->flags1 |= TR1_SLAY_TROLL;
                    i_ptr->tohit += 2;
                    i_ptr->todam += 2;
                    i_ptr->name2 = EGO_SLAY_TROLL;
                    rating += 13;
                    break;

                  case 21: case 22: case 23:
                    i_ptr->flags1 |= TR1_SLAY_GIANT;
                    i_ptr->tohit += 2;
                    i_ptr->todam += 2;
                    i_ptr->name2 = EGO_SLAY_GIANT;
                    rating += 14;
                    break;

                  case 24: case 25: case 26:
                    i_ptr->flags1 |= TR1_SLAY_DEMON;
                    i_ptr->tohit += 2;
                    i_ptr->todam += 2;
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
                    i_ptr->name2 = EGO_BLESS_BLADE;
                    give_1_lo_resist(i_ptr);
                    rating += 20;
                    break;

                  /* Extra Attacks */
                  case 30:
                    i_ptr->tohit += randint(5);
                    i_ptr->todam += randint(3);
                    i_ptr->flags1 |= (TR1_BLOWS);
                    if (i_ptr->weight <= 80) {
                        i_ptr->pval = randint(3);
                    }
                    else if (i_ptr->weight <= 130) {
                        i_ptr->pval = randint(2);
                    }
                    else {
                        i_ptr->pval = 1;
                    }
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

                /* Permanently broken */
                i_ptr->ident |= ID_BROKEN;

                i_ptr->flags3 |= (TR3_HEAVY_CURSE | TR3_AGGRAVATE | TR3_SEE_INVIS);
                i_ptr->tohit -= 15;
                i_ptr->todam -= 15;
                i_ptr->toac = -10;
                i_ptr->weight += 100;	/* 10 pounds */
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
                    rating += 20;
                    if (cheat_peek) inven_mention(i_ptr);
                    break;

                  case 2:
                    i_ptr->flags3 |= TR3_XTRA_SHOTS;
                    i_ptr->tohit += 10;
                    i_ptr->todam += 3;
                    i_ptr->name2 = EGO_EXTRA_SHOTS;
                    rating += 20;
                    if (cheat_peek) inven_mention(i_ptr);
                    break;

                  case 3: case 4: case 5: case 6:
                    i_ptr->tohit += 5;
                    i_ptr->todam += 12;
                    i_ptr->name2 = EGO_VELOCITY;
                    rating += 11;
                    if (cheat_peek) inven_mention(i_ptr);
                    break;

                  case 7: case 8: case 9: case 10:
                    i_ptr->tohit += 12;
                    i_ptr->todam += 5;
                    i_ptr->name2 = EGO_ACCURACY;
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
                    rating += 5;
                    break;

                  case 4: case 5:
                    i_ptr->name2 = EGO_AMMO_FIRE;
                    i_ptr->flags1 |= (TR1_BRAND_FIRE);
                    i_ptr->flags3 |= (TR3_IGNORE_FIRE);
                    i_ptr->tohit += 2;
                    i_ptr->todam += 4;
                    rating += 6;
                    break;

                  case 6: case 7:
                    i_ptr->name2 = EGO_AMMO_EVIL;
                    i_ptr->flags1 |= TR1_SLAY_EVIL;
                    i_ptr->tohit += 3;
                    i_ptr->todam += 3;
                    rating += 7;
                    break;

                  case 8: case 9:
                    i_ptr->name2 = EGO_AMMO_ANIMAL;
                    i_ptr->flags1 |= TR1_SLAY_ANIMAL;
                    i_ptr->tohit += 2;
                    i_ptr->todam += 2;
                    rating += 5;
                    break;

                  case 10:
                    i_ptr->name2 = EGO_AMMO_DRAGON;
                    i_ptr->flags1 |= TR1_SLAY_DRAGON;
                    i_ptr->tohit += 3;
                    i_ptr->todam += 3;
                    rating += 9;
                    break;

                  case 11:
                    i_ptr->name2 = EGO_AMMO_SLAYING;
                    i_ptr->tohit += 10;
                    i_ptr->todam += 10;
                    i_ptr->dd += 2; /* two extra die of damage */
                    rating += 10;
                    break;
                }
            }

            /* Super-charge the arrows */
            while (magik(special)) i_ptr->dd++;
        }

        /* Cursed missiles */
        else if (magik(cursed)) {

            /* Cursed missiles */
            i_ptr->flags3 |= TR3_CURSED;
            i_ptr->tohit = 0 - (randint(10) + m_bonus(5, 25, level));
            i_ptr->todam = 0 - (randint(10) + m_bonus(5, 25, level));

            /* Permanently damaged missiles */
            if (magik(special)) {

                /* Permanently broken */
                i_ptr->ident |= ID_BROKEN;

                i_ptr->tohit = 0 - (m_bonus(10, 30, level) + 20);
                i_ptr->todam = 0 - (m_bonus(10, 30, level) + 20);
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

                /* Permanently broken */
                i_ptr->ident |= ID_BROKEN;

                i_ptr->flags3 |= TR3_CURSED;
                i_ptr->pval = 0 - m_bonus(1, 10, level);
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

                /* Permanently broken */
                i_ptr->ident |= ID_BROKEN;

                i_ptr->flags3 |= TR3_CURSED;
                i_ptr->pval = 0 - i_ptr->pval;
                
                break;
            }

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

                /* Permanently broken */
                i_ptr->ident |= ID_BROKEN;

                i_ptr->flags3 |= TR3_CURSED;
                i_ptr->pval = 0 - i_ptr->pval;
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

                /* Permanently broken */
                i_ptr->ident |= ID_BROKEN;

                i_ptr->flags3 |= TR3_CURSED;
                i_ptr->todam = 0 - i_ptr->todam;
            }

            break;

          /* Ring of Accuracy */
          case SV_RING_ACCURACY:

            /* Bonus to hit */
            i_ptr->tohit = m_bonus(1, 10, level) + randint(10) + 3;

            /* Cursed */
            if (magik(cursed)) {

                /* Permanently broken */
                i_ptr->ident |= ID_BROKEN;

                i_ptr->flags3 |= TR3_CURSED;
                i_ptr->tohit = 0 - i_ptr->tohit;
            }

            break;

          /* Ring of Protection */
          case SV_RING_PROTECTION:

            /* Bonus to armor class */
            i_ptr->toac = m_bonus(0, 10, level) + randint(5) + 4;

            /* Cursed */
            if (magik(cursed)) {

                /* Permanently broken */
                i_ptr->ident |= ID_BROKEN;

                i_ptr->flags3 |= TR3_CURSED;
                i_ptr->toac = 0 - i_ptr->toac;
            }

            break;

          /* Ring of Slaying */
          case SV_RING_SLAYING:

            /* Bonus to damage and to hit */
            i_ptr->todam = m_bonus(1, 10, level) + randint(3) + 2;
            i_ptr->tohit = m_bonus(1, 10, level) + randint(3) + 2;

            /* Cursed */
            if (magik(cursed)) {

                /* Permanently broken */
                i_ptr->ident |= ID_BROKEN;

                i_ptr->flags3 |= TR3_CURSED;
                i_ptr->tohit = 0 - i_ptr->tohit;
                i_ptr->todam = 0 - i_ptr->todam;
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

                /* Permanently broken */
                i_ptr->ident |= ID_BROKEN;

                i_ptr->flags3 |= TR3_CURSED;
                i_ptr->pval = 0 - i_ptr->pval;
            }

            break;

          /* Amulet of searching */
          case SV_AMULET_SEARCHING:

            i_ptr->pval = randint(2) + m_bonus(0, 8, level);

            if (magik(cursed)) {

                /* Permanently broken */
                i_ptr->ident |= ID_BROKEN;

                i_ptr->flags3 |= TR3_CURSED;
                i_ptr->pval = 0 - i_ptr->pval;
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

            /* Permanently broken */
            i_ptr->ident |= ID_BROKEN;

            i_ptr->flags3 |= TR3_CURSED;
            i_ptr->pval = 0 - (randint(5) + m_bonus(2, 10, level));
            i_ptr->toac = 0 - (randint(3) + m_bonus(0, 6, level));
            break;
        }
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


      case TV_WAND:
      
        /* Hack -- charge wands */
        charge_wand(i_ptr);

        break;


      case TV_STAFF:

        /* Hack -- charge staffs */
        charge_staff(i_ptr);

        break;


      case TV_CHEST:

        /* Hack -- skip ruined chests */
        if (k_list[i_ptr->k_idx].level <= 0) break;
        
        /* Hack -- pick a "difficulty" */
        i_ptr->pval = randint(k_list[i_ptr->k_idx].level);

        /* Never exceed "difficulty" of 55 (really 59) */
        if (i_ptr->pval > 55) i_ptr->pval = 55 + rand_int(5);
        
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
 * In addition, this function cleans up some floating bizarreness,
 * like the "fake-torches" in the dungeon, etc.  Likewise, the Chests
 * get "trapped" here.  Store-bought chests may be really good.  Oh, and
 * wands and staffs get "charged" here.
 *
 * Hack -- due to some annoying aspects of the "special artifact" metaphor
 * (lites, rings, amulets), those artifacts are actualy handled elsewhere,
 * in fact, they can only be generated by the "place_object()" function,
 * when it bypasses the "apply_magic()" routine entirely.
 */
void apply_magic(inven_type *i_ptr, int level, bool okay, bool good, bool great)
{
    /* Paranoia -- great implies good */
    if (great) good = TRUE;


    /* Apply magic to armour-type objects */
    a_m_aux_1(i_ptr, level, okay, good, great);
    
    /* Apply magic to non-armour-type objects */
    a_m_aux_2(i_ptr, level, okay, good, great);

    /* Hack -- acquire curse */
    if (i_ptr->flags3 & TR3_CURSED) i_ptr->ident |= ID_CURSED;
}



/*
 * Hack -- determine if a template is "good"
 */
static bool kind_is_good(int k_idx)
{
    /* Analyze the item type */
    switch (k_list[k_idx].tval) {
    
        /* Armor -- Good unless damaged */
        case TV_HARD_ARMOR:
        case TV_SOFT_ARMOR:
        case TV_DRAG_ARMOR:
        case TV_SHIELD:
        case TV_CLOAK:
        case TV_BOOTS:
        case TV_GLOVES:
        case TV_HELM:
        case TV_CROWN:
            if (k_list[k_idx].toac < 0) return (FALSE);
            return (TRUE);

        /* Weapons -- Good unless damaged */
        case TV_BOW:
        case TV_SWORD:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_DIGGING:
            if (k_list[k_idx].tohit < 0) return (FALSE);
            if (k_list[k_idx].todam < 0) return (FALSE);
            return (TRUE);
            
        /* Ammo -- Arrows/Bolts are good */
        case TV_BOLT:
        case TV_ARROW:
            return (TRUE);

        /* Books -- High level books are good */
        case TV_MAGIC_BOOK:
        case TV_PRAYER_BOOK:
            if (k_list[k_idx].sval >= SV_BOOK_MIN_GOOD) return (TRUE);
            return (FALSE);
            
        /* Rings -- Rings of Speed are good */
        case TV_RING:
            if (k_list[k_idx].sval == SV_RING_SPEED) return (TRUE);
            return (FALSE);

        /* Amulets -- Amulets of the Magi are good */
        case TV_AMULET:
            if (k_list[k_idx].sval == SV_AMULET_THE_MAGI) return (TRUE);
            return (FALSE);
    }
    
    /* Assume not good */
    return (FALSE);
}



/*
 * Attempt to place an object (normal or good/great) at the given location.
 *
 * This routine plays nasty games to generate the "special artifacts".
 *
 * This routine uses "object_level" for the "generation level".
 *
 * This routine requires a clean floor grid destination.
 */
void place_object(int y, int x, bool good, bool great)
{
    cave_type		*c_ptr;
    inven_type		*i_ptr;
    
    inven_type		forge;

    int			prob;
    int			base;

    int			old = rating;


    /* Paranoia -- check bounds */
    if (!in_bounds(y, x)) return;

    /* Require clean floor space */
    if (!clean_grid_bold(y, x)) return;


    /* Chance of "special object" */
    prob = (good ? 10 : 1000);

    /* Base level for the object */
    base = (good ? (object_level + 10) : object_level);
    

    /* Hack -- clear out the forgery */
    invwipe(&forge);
    
    /* Generate a special object, or a normal object */
    if ((rand_int(prob) != 0) || !make_artifact_special(&forge)) {

        /* Pick a good "base object" */
        while (TRUE) {

            int k_idx;

            /* Pick a random object */
            k_idx = get_obj_num(base);

            /* Hack -- prevent embedded chests */
            if (opening_chest && (k_list[k_idx].tval == TV_CHEST)) continue;
            
            /* Check if that kind is "good" */
            if (good && !kind_is_good(k_idx)) continue;

            /* Prepare the object */
            invcopy(&forge, k_idx);

            /* Okay */
            break;
        }

        /* Apply magic (allow artifacts) */
        apply_magic(&forge, object_level, TRUE, good, great);

        /* Hack -- generate multiple spikes/missiles */
        switch (forge.tval) {
            case TV_SPIKE:
            case TV_SHOT:
            case TV_ARROW:
            case TV_BOLT:
                forge.number = damroll(6,7);
        }
    }
    

    /* Create an object */
    c_ptr = &cave[y][x];
    c_ptr->i_idx = i_pop();
    i_ptr = &i_list[c_ptr->i_idx];

    /* Hack -- Place the object */
    (*i_ptr) = (forge);

    /* Drop it into the dungeon */
    i_ptr->iy = y;
    i_ptr->ix = x;


    /* Notice "okay" out-of-depth objects (unless already noticed) */
    if (!cursed_p(i_ptr) && !broken_p(i_ptr) &&
        (rating == old) && (k_list[i_ptr->k_idx].level > dun_level)) {

        /* Rating increase */
        rating += (k_list[i_ptr->k_idx].level - dun_level);

        /* Cheat -- peek at items */
        if (cheat_peek) inven_mention(i_ptr);
    }

    /* Under the player */
    if (c_ptr->m_idx == 1) {

        /* Message */
        msg_print ("You feel something roll beneath your feet.");
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
            scatter(&y, &x, y1, x1, d, 0);

            /* Must have a clean grid */
            if (!clean_grid_bold(y, x)) continue;

            /* Place a good (or great) object */
            place_object(y, x, TRUE, great);

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
 *
 * Now takes a boolean argument to indicate when only "harmful" traps should
 * be created.  This is used for monsters casting Create Traps as well as
 * players reading Create Traps scrolls, and also for some of the dungeon
 * generation calls.  Currently, visible pits and loose rocks are the only
 * non "harmful" traps.     -SHAWN-
 */
void place_trap(int y, int x, bool harmful)
{
    int trap;
    cave_type *c_ptr;
    inven_type *i_ptr;
    bool invis = TRUE;


    /* Paranoia -- verify location */
    if (!in_bounds(y, x)) return;

    /* Require empty, clean, floor grid */
    if (!naked_grid_bold(y, x)) return;


    /* Pick a trap */
    while (TRUE) {

        inven_kind *k_ptr;
            
        /* Hack -- Pick a trap */
        trap = OBJ_TRAP_LIST + rand_int(MAX_TRAP);

        /* Access the trap */
        k_ptr = &k_list[trap];

        /* Hack -- simple "rarity" info */
                
        /* Hack -- loose rocks are not harmful */
        if (harmful && (k_ptr->sval == SV_TRAP_LOOSE_ROCK)) continue;

        /* Hack -- no trap doors on quest levels */
        if ((k_ptr->sval == SV_TRAP_TRAP_DOOR) && (is_quest(dun_level))) continue;

        /* Accept this trap */
        break;
    }

    /* Mega-Hack -- Occasionally, just make a visible open pit */
    if (!harmful && (rand_int(10) == 0)) {

        /* Use the simple "pit" */
        trap = lookup_kind(TV_VIS_TRAP, SV_TRAP_PIT);

        /* Make it visible */
        invis = FALSE;
    }


    /* Make a new "trap" object */
    c_ptr = &cave[y][x];
    c_ptr->i_idx = i_pop();
    i_ptr = &i_list[c_ptr->i_idx];
    invcopy(i_ptr, trap);
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Make the trap invisible if needed */
    if (invis) i_ptr->tval = TV_INVIS_TRAP;
}



/*
 * Places a treasure (Gold or Gems) at given location
 * The location must be a valid, empty, floor grid.
 */
void place_gold(int y, int x)
{
    int	i;
    s32b base;
    
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


    /* Make a new "gold" object */
    c_ptr = &cave[y][x];
    c_ptr->i_idx = i_pop();
    i_ptr = &i_list[c_ptr->i_idx];
    invcopy(i_ptr, OBJ_GOLD_LIST + i);
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Base coin cost */
    base = k_list[OBJ_GOLD_LIST+i].cost;
    
    /* Determine how much the treasure is "worth" */
    i_ptr->pval = (base + (8L * randint(base)) + randint(8));

    /* Under the player */
    if (c_ptr->m_idx == 1) {
        msg_print("You feel something roll beneath your feet.");
    }
}





/*
 * Process the objects
 */
void process_objects(void)
{
    int		i;


    /* Compact objects when we start to get desperate */
    if (MIN_I_IDX + i_cnt + 30 > MAX_I_IDX) compact_objects(30);


    /* Once every ten turns */
    if (!(turn % 10)) {

        /* Process the objects */
        for (i = MIN_I_IDX; i < i_max; i++) {

            /* Get the i'th object */
            inven_type *i_ptr = &i_list[i];

            /* Skip dead objects */
            if (!i_ptr->k_idx) continue;

#ifdef SHIMMER_OBJECTS
            /* Shimmer Multi-Hued Objects */
#endif

            /* Recharge rods on the ground */
            if ((i_ptr->tval == TV_ROD) && (i_ptr->pval)) i_ptr->pval--;
        }

        /* XXX XXX Compact objects when we start to get desperate */
        /* if (MIN_I_IDX + i_cnt + 30 > MAX_I_IDX) compact_objects(30); */
    }
}


