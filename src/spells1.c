/* File: spells1.c */

/* Purpose: generic bolt/ball/beam code	-BEN- */

#include "angband.h"



#ifdef USE_COLOR

/*
 * Get a legal "multi-hued" color for drawing "spells"
 */
static byte mh_attr(void)
{
    switch (randint(9)) {
        case 1: return (TERM_RED);
        case 2: return (TERM_GREEN);
        case 3: return (TERM_BLUE);
        case 4: return (TERM_YELLOW);
        case 5: return (TERM_ORANGE);
        case 6: return (TERM_VIOLET);
        case 7: return (TERM_L_RED);
        case 8: return (TERM_L_GREEN);
        case 9: return (TERM_L_BLUE);
    }

    return (TERM_WHITE);
}

#endif


/*
 * Return a color to use for the bolt/ball spells
 */
byte spell_color(int type)
{

#ifdef USE_COLOR

    if (!use_color) return (TERM_WHITE);

    switch (type) {

        case GF_MISSILE:
                return (mh_attr());		/* multihued */
        case GF_ELEC:
                return (TERM_YELLOW);
        case GF_POIS:
                return (TERM_GREEN);
        case GF_ACID:
                return (TERM_SLATE);
        case GF_COLD:
                return (TERM_L_BLUE);
        case GF_FIRE:
                return (TERM_RED);
        case GF_HOLY_ORB:
                return (TERM_L_DARK);
        case GF_ARROW:
                return (TERM_L_UMBER);
        case GF_PLASMA:
                return (TERM_RED);
        case GF_NETHER:
                return (TERM_VIOLET);
        case GF_WATER:
                return (TERM_BLUE);
        case GF_CHAOS:
                return (mh_attr());		/* multihued */
        case GF_SHARDS:
                return (TERM_L_UMBER);
        case GF_SOUND:
                return (TERM_ORANGE);
        case GF_CONFUSION:
                return (mh_attr());		/* multihued */
        case GF_DISENCHANT:
                return (TERM_VIOLET);
        case GF_NEXUS:
                return (TERM_L_RED);
        case GF_FORCE:
                return (TERM_WHITE);
        case GF_INERTIA:
                return (TERM_L_WHITE);
        case GF_LITE_WEAK:
        case GF_LITE:
                return (TERM_YELLOW);
        case GF_DARK_WEAK:
        case GF_DARK:
                return (TERM_L_DARK);
        case GF_TIME:
                return (TERM_L_BLUE);
        case GF_GRAVITY:
                return (TERM_SLATE);
        case GF_MANA:
                return (TERM_L_RED);
        case GF_METEOR:
                return (TERM_ORANGE);
        case GF_ICE:
                return (TERM_L_BLUE);
    }

#endif

    /* Standard "color" */
    return (TERM_WHITE);
}





/*
 * Calculate "incremental motion". Used by project() and shoot().
 * Assumes that (*y,*x) lies on the path from (y1,x1) to (y2,x2).
 */
void mmove2(int *y, int *x, int y1, int x1, int y2, int x2)
{
    int d_y, d_x, dist, shift;

    /* Extract the distance travelled */
    d_y = (*y < y1) ? y1 - *y : *y - y1;
    d_x = (*x < x1) ? x1 - *x : *x - x1;
    dist = (d_y > d_x) ? d_y : d_x;

    /* We are calculating the next location */
    dist++;


    /* Calculate the total distance along each axis */
    d_y = (y2 < y1) ? (y1 - y2) : (y2 - y1);
    d_x = (x2 < x1) ? (x1 - x2) : (x2 - x1);

    /* Paranoia -- Hack -- no motion */
    if (!d_y && !d_x) return;


    /* Move mostly vertically */
    if (d_y > d_x) {

#if 0

        int k;

        /* Starting shift factor */
        shift = d_y >> 1;

        /* Extract a shift factor */
        for (k = 0; k < dist; k++) {
            if (shift <= 0) shift += d_y;
            shift -= d_x;
        }

        /* Sometimes move along minor axis */
        if (shift <= 0) (*x) = (x2 < x1) ? (*x - 1) : (*x + 1);

        /* Always move along major axis */
        (*y) = (y2 < y1) ? (*y - 1) : (*y + 1);

#endif

        /* Extract a shift factor */
        shift = (dist * d_x + (d_y-1) / 2) / d_y;

        /* Sometimes move along the minor axis */
        (*x) = (x2 < x1) ? (x1 - shift) : (x1 + shift);

        /* Always move along major axis */
        (*y) = (y2 < y1) ? (y1 - dist) : (y1 + dist);
    }

    /* Move mostly horizontally */
    else {

#if 0

        int k;

        /* Starting shift factor */
        shift = d_x >> 1;

        /* Extract a shift factor */
        for (k = 0; k < dist; k++) {
            if (shift <= 0) shift += d_x;
            shift -= d_y;
        }

        /* Sometimes move along minor axis */
        if (shift <= 0) (*y) = (y2 < y1) ? (*y - 1) : (*y + 1);

        /* Always move along major axis */
        (*x) = (x2 < x1) ? (*x - 1) : (*x + 1);

#endif

        /* Extract a shift factor */
        shift = (dist * d_y + (d_x-1) / 2) / d_x;

        /* Sometimes move along the minor axis */
        (*y) = (y2 < y1) ? (y1 - shift) : (y1 + shift);

        /* Always move along major axis */
        (*x) = (x2 < x1) ? (x1 - dist) : (x1 + dist);
    }
}



/*
 * Determine if a bolt spell cast from (y1,x1) to (y2,x2) will arrive
 * at the final destination, assuming no monster gets in the way.
 *
 * This is slightly (but significantly) different from "los(y1,x1,y2,x2)".
 */
bool projectable(int y1, int x1, int y2, int x2)
{
    int dist, y, x;

    /* Start at the initial location */
    y = y1, x = x1;

    /* See "project()" */
    for (dist = 0; dist < MAX_RANGE; dist++) {

        /* Never pass through walls */
        if (dist && !floor_grid_bold(y, x)) break;

        /* Check for arrival at "final target" */
        if ((x == x2) && (y == y2)) return (TRUE);

        /* Calculate the new location */
        mmove2(&y, &x, y1, x1, y2, x2);
    }


    /* Assume obstruction */
    return (FALSE);
}




/*
 * Apply disenchantment to the player's stuff
 *
 * The "mode" is currently unused.
 *
 * Return "TRUE" if the player notices anything
 */
bool apply_disenchant(int mode)
{
    int			t = 0;

    inven_type		*i_ptr;

    char		i_name[80];


    /* Pick a random slot */
    switch (randint(8)) {
         case 1: t = INVEN_WIELD; break;
         case 2: t = INVEN_BOW; break;
         case 3: t = INVEN_BODY; break;
         case 4: t = INVEN_OUTER; break;
         case 5: t = INVEN_ARM; break;
         case 6: t = INVEN_HEAD; break;
         case 7: t = INVEN_HANDS; break;
         case 8: t = INVEN_FEET; break;
    }

    /* Get the item */
    i_ptr = &inventory[t];

    /* No item, nothing happens */
    if (!i_ptr->k_idx) return (FALSE);


    /* Nothing to disenchant */
    if ((i_ptr->to_h <= 0) && (i_ptr->to_d <= 0) && (i_ptr->to_a <= 0)) {

        /* Nothing to notice */
        return (FALSE);
    }


    /* Describe the object */
    objdes(i_name, i_ptr, FALSE, 0);


    /* Artifacts have 60% chance to resist */
    if (artifact_p(i_ptr) && (rand_int(100) < 60)) {

        /* Message */
        msg_format("Your %s (%c) resist%s disenchantment!",
                   i_name, index_to_label(t),
                   ((i_ptr->number != 1) ? "" : "s"));

        /* Notice */
        return (TRUE);
    }


    /* Disenchant tohit */
    if (i_ptr->to_h > 0) i_ptr->to_h--;
    if ((i_ptr->to_h > 5) && (rand_int(100) < 20)) i_ptr->to_h--;

    /* Disenchant todam */
    if (i_ptr->to_d > 0) i_ptr->to_d--;
    if ((i_ptr->to_d > 5) && (rand_int(100) < 20)) i_ptr->to_d--;

    /* Disenchant toac */
    if (i_ptr->to_a > 0) i_ptr->to_a--;
    if ((i_ptr->to_a > 5) && (rand_int(100) < 20)) i_ptr->to_a--;

    /* Message */
    msg_format("Your %s (%c) %s disenchanted!",
               i_name, index_to_label(t),
               ((i_ptr->number != 1) ? "were" : "was"));

    /* Redraw the choice window */
    p_ptr->redraw |= (PR_CHOOSE);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Notice */
    return (TRUE);
}


/*
 * Apply Nexus
 */
static void apply_nexus(monster_type *m_ptr)
{
    int max1, cur1, max2, cur2, ii, jj;

    switch (randint(7)) {

        case 1: case 2: case 3:

            teleport_flag = TRUE;
            teleport_dist = 200;
            break;

        case 4: case 5:

            teleport_flag = TRUE;
            teleport_dist = 0;
            teleport_to_y = m_ptr->fy;
            teleport_to_x = m_ptr->fx;
            break;

        case 6:

            if (rand_int(100) < p_ptr->skill_sav) {
                msg_print("You resist the effects!");
                break;
            }

            /* Teleport Level */
            tele_level();
            break;

        case 7:

            if (rand_int(100) < p_ptr->skill_sav) {
                msg_print("You resist the effects!");
                break;
            }

            msg_print("Your body starts to scramble...");

            /* Pick a pair of stats */
            ii = rand_int(6);
            for (jj = ii; jj == ii; jj = rand_int(6)) ;

            max1 = p_ptr->stat_max[ii];
            cur1 = p_ptr->stat_cur[ii];
            max2 = p_ptr->stat_max[jj];
            cur2 = p_ptr->stat_cur[jj];

            p_ptr->stat_max[ii] = max2;
            p_ptr->stat_cur[ii] = cur2;
            p_ptr->stat_max[jj] = max1;
            p_ptr->stat_cur[jj] = cur1;

            p_ptr->update |= (PU_BONUS);

            break;
    }
}





/*
 * Note that amulets, rods, and high-level spell books are immune
 * to "inventory damage" of any kind.  Also sling ammo and shovels.
 */


/*
 * Does a given class of objects (usually) hate acid?
 * Note that acid can either melt or corrode something.
 */
static bool hates_acid(inven_type *i_ptr)
{
    /* Analyze the type */
    switch (i_ptr->tval) {

      /* Wearable items */
      case TV_ARROW:
      case TV_BOLT:
      case TV_BOW:
      case TV_SWORD:
      case TV_HAFTED:
      case TV_POLEARM:
      case TV_HELM:
      case TV_CROWN:
      case TV_SHIELD:
      case TV_BOOTS:
      case TV_GLOVES:
      case TV_CLOAK:
      case TV_SOFT_ARMOR:
      case TV_HARD_ARMOR:
      case TV_DRAG_ARMOR:

      /* Staffs/Scrolls are wood/paper */
      case TV_STAFF:
      case TV_SCROLL:

      /* Ouch */
      case TV_CHEST:

      /* Junk is useless */
      case TV_SKELETON:
      case TV_BOTTLE:
      case TV_JUNK:
      
        return (TRUE);
    }

    return (FALSE);
}


/*
 * Does a given object (usually) hate electricity?
 */
static bool hates_elec(inven_type *i_ptr)
{
    switch (i_ptr->tval) {

      case TV_RING:

      case TV_WAND:
      
        return (TRUE);
    }

    return (FALSE);
}


/*
 * Does a given object (usually) hate fire?
 * Hafted/Polearm weapons have wooden shafts.
 * Arrows/Bows are mostly wooden.
 */
static bool hates_fire(inven_type *i_ptr)
{
    /* Analyze the type */
    switch (i_ptr->tval) {

      /* Wearable */
      case TV_LITE:
      case TV_ARROW:
      case TV_BOW:
      case TV_HAFTED:
      case TV_POLEARM:
      case TV_BOOTS:
      case TV_GLOVES:
      case TV_CLOAK:
      case TV_SOFT_ARMOR:

      /* Books */
      case TV_MAGIC_BOOK:
      case TV_PRAYER_BOOK:

      /* Chests */
      case TV_CHEST:

      /* Staffs/Scrolls burn */
      case TV_STAFF:
      case TV_SCROLL:

        return (TRUE);
    }

    return (FALSE);
}


/*
 * Does a given object (usually) hate cold?
 */
static bool hates_cold(inven_type *i_ptr)
{
    switch (i_ptr->tval) {

      case TV_POTION:

      case TV_FLASK:

      case TV_BOTTLE:
      
        return (TRUE);
    }

    return (FALSE);
}









/*
 * Melt something
 */
static int set_acid_destroy(inven_type *i_ptr)
{
    u32b f1, f2, f3;
    if (!hates_acid(i_ptr)) return (FALSE);
    inven_flags(i_ptr, &f1, &f2, &f3);
    if (f3 & TR3_IGNORE_ACID) return (FALSE);
    return (TRUE);
}


/*
 * Electrical damage
 */
static int set_elec_destroy(inven_type *i_ptr)
{
    u32b f1, f2, f3;
    if (!hates_elec(i_ptr)) return (FALSE);
    inven_flags(i_ptr, &f1, &f2, &f3);
    if (f3 & TR3_IGNORE_ELEC) return (FALSE);
    return (TRUE);
}


/*
 * Burn something
 */
static int set_fire_destroy(inven_type *i_ptr)
{
    u32b f1, f2, f3;
    if (!hates_fire(i_ptr)) return (FALSE);
    inven_flags(i_ptr, &f1, &f2, &f3);
    if (f3 & TR3_IGNORE_FIRE) return (FALSE);
    return (TRUE);
}


/*
 * Freeze things
 */
static int set_cold_destroy(inven_type *i_ptr)
{
    u32b f1, f2, f3;
    if (!hates_cold(i_ptr)) return (FALSE);
    inven_flags(i_ptr, &f1, &f2, &f3);
    if (f3 & TR3_IGNORE_COLD) return (FALSE);
    return (TRUE);
}




/*
 * This seems like a pretty standard "typedef"
 */
typedef int (*inven_func)(inven_type *);

/*
 * Destroys a type of item on a given percent chance
 * Note that missiles are no longer necessarily all destroyed
 * Destruction taken from "melee.c" code for "stealing".
 * Returns number of items destroyed.
 */
static int inven_damage(inven_func typ, int perc)
{
    int		i, j, k, amt;

    inven_type	*i_ptr;

    char	i_name[80];


    /* Count the casualties */
    k = 0;

    /* Scan through the slots backwards */
    for (i = 0; i < INVEN_PACK; i++) {

        /* Get the item in that slot */
        i_ptr = &inventory[i];

        /* Hack -- for now, skip artifacts */
        if (artifact_p(i_ptr)) continue;

        /* Give this item slot a shot at death */
        if ((*typ)(i_ptr)) {

            /* Count the casualties */
            for (amt = j = 0; j < i_ptr->number; ++j) {
                if (rand_int(100) < perc) amt++;
            }

            /* Some casualities */
            if (amt) {

                /* Get a description */
                objdes(i_name, i_ptr, FALSE, 3);

                /* Message */
                msg_format("%sour %s (%c) %s destroyed!",
                           ((i_ptr->number > 1) ?
                            ((amt == i_ptr->number) ? "All of y" :
                            (amt > 1 ? "Some of y" : "One of y")) : "Y"),
                           i_name, index_to_label(i),
                           ((amt > 1) ? "were" : "was"));

                /* Destroy "amt" items */
                inven_item_increase(i,-amt);
                inven_item_optimize(i);

                /* Count the casualties */
                k += amt;
            }
        }
    }

    /* Return the casualty count */
    return (k);
}




/*
 * Acid has hit the player, attempt to affect some armor.
 *
 * Note that the "base armor" of an object never changes.
 *
 * If any armor is damaged (or resists), the player takes less damage.
 */
static int minus_ac(void)
{
    inven_type		*i_ptr = NULL;

    u32b		f1, f2, f3;
    
    char		i_name[80];


    /* Pick a (possibly empty) inventory slot */
    switch (randint(6)) {
        case 1: i_ptr = &inventory[INVEN_BODY]; break;
        case 2: i_ptr = &inventory[INVEN_ARM]; break;
        case 3: i_ptr = &inventory[INVEN_OUTER]; break;
        case 4: i_ptr = &inventory[INVEN_HANDS]; break;
        case 5: i_ptr = &inventory[INVEN_HEAD]; break;
        case 6: i_ptr = &inventory[INVEN_FEET]; break;
    }

    /* Nothing to damage */
    if (!i_ptr->k_idx) return (FALSE);

    /* No damage left to be done */
    if (i_ptr->ac + i_ptr->to_a <= 0) return (FALSE);


    /* Describe */
    objdes(i_name, i_ptr, FALSE, 0);

    /* Extract the flags */
    inven_flags(i_ptr, &f1, &f2, &f3);
    
    /* Object resists */
    if (f3 & TR3_IGNORE_ACID) {

        msg_format("Your %s is unaffected!", i_name);

        return (TRUE);
    }

    /* Message */
    msg_format("Your %s is damaged!", i_name);

    /* Damage the item */
    i_ptr->to_a--;

    /* Redraw the choice window */
    p_ptr->redraw |= (PR_CHOOSE);

    /* Calculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Item was damaged */
    return (TRUE);
}


/*
 * Hurt the player with Acid
 */
void acid_dam(int dam, cptr kb_str)
{
    int inv = (dam < 30) ? 1 : (dam < 60) ? 2 : 3;

    /* Total Immunity */
    if (p_ptr->immune_acid || (dam <= 0)) return;

    /* Resist the damage */
    if (p_ptr->resist_acid) dam = (dam + 2) / 3;
    if (p_ptr->oppose_acid) dam = (dam + 2) / 3;

    /* If any armor gets hit, defend the player */
    if (minus_ac()) dam = (dam + 1) / 2;

    /* Take damage */
    take_hit(dam, kb_str);

    /* Inventory damage */
    inven_damage(set_acid_destroy, inv);
}


/*
 * Hurt the player with electricity
 */
void elec_dam(int dam, cptr kb_str)
{
    int inv = (dam < 30) ? 1 : (dam < 60) ? 2 : 3;

    /* Total immunity */
    if (p_ptr->immune_elec || (dam <= 0)) return;

    /* Resist the damage */
    if (p_ptr->oppose_elec) dam = (dam + 2) / 3;
    if (p_ptr->resist_elec) dam = (dam + 2) / 3;

    /* Take damage */
    take_hit(dam, kb_str);

    /* Inventory damage */
    inven_damage(set_elec_destroy, inv);
}




/*
 * Hurt the player with Fire
 */
void fire_dam(int dam, cptr kb_str)
{
    int inv = (dam < 30) ? 1 : (dam < 60) ? 2 : 3;

    /* Totally immune */
    if (p_ptr->immune_fire || (dam <= 0)) return;

    /* Resist the damage */
    if (p_ptr->resist_fire) dam = (dam + 2) / 3;
    if (p_ptr->oppose_fire) dam = (dam + 2) / 3;

    /* Take damage */
    take_hit(dam, kb_str);

    /* Inventory damage */
    inven_damage(set_fire_destroy, inv);
}


/*
 * Hurt the player with Cold
 */
void cold_dam(int dam, cptr kb_str)
{
    int inv = (dam < 30) ? 1 : (dam < 60) ? 2 : 3;

    /* Total immunity */
    if (p_ptr->immune_cold || (dam <= 0)) return;

    /* Resist the damage */
    if (p_ptr->resist_cold) dam = (dam + 2) / 3;
    if (p_ptr->oppose_cold) dam = (dam + 2) / 3;

    /* Take damage */
    take_hit(dam, kb_str);

    /* Inventory damage */
    inven_damage(set_cold_destroy, inv);
}





/*
 * We are called from "project()" to "damage" cave grids
 * and the inventory items which may be contained inside them
 *
 * We are called both for "beam" effects and "ball" effects.
 *
 * Perhaps we should only SOMETIMES damage things on the ground.
 *
 * The "rad" parameter (unused) is the "distance from ground zero".
 *
 * Note that we determine if the player can "see" anything that happens
 * by taking into account: blindness, line-of-sight, and illumination.
 *
 * XXX XXX XXX We also "see" grids which are "memorized", probably a hack
 *
 * We return "TRUE" if the effect of the projection is "obvious".
 *
 * XXX XXX XXX Perhaps we should affect doors?
 */
static bool project_i(int who, int rad, int y, int x, int dam, int typ, int flg)
{
    cave_type	*c_ptr = &cave[y][x];

    bool	obvious = FALSE;


    /* Affect the object (if any) */
    if ((flg & PROJECT_ITEM) && (c_ptr->i_idx)) {

        inven_type	*i_ptr;

        bool	is_art = FALSE;
        bool	ignore = FALSE;
        bool	plural = FALSE;
        bool	do_kill = FALSE;

        cptr	note_kill = NULL;

        u32b	f1, f2, f3;

        char	i_name[80];


        /* Get the object */
        i_ptr = &i_list[c_ptr->i_idx];

        /* Extract the flags */
        inven_flags(i_ptr, &f1, &f2, &f3);

        /* Get the "plural"-ness */
        if (i_ptr->number > 1) plural = TRUE;

        /* Check for artifact */
        if (artifact_p(i_ptr)) is_art = TRUE;

        /* Analyze the type */
        switch (typ) {

            /* Acid -- Lots of things */
            case GF_ACID:
                if (hates_acid(i_ptr)) {
                    do_kill = TRUE;
                    note_kill = (plural ? " melt!" : " melts!");
                    if (f3 & TR3_IGNORE_ACID) ignore = TRUE;
                }
                break;

            /* Elec -- Rings and Wands */
            case GF_ELEC:
                if (hates_elec(i_ptr)) {
                    do_kill = TRUE;
                    note_kill= (plural ? " is destroyed!" : " is destroyed!");
                    if (f3 & TR3_IGNORE_ELEC) ignore = TRUE;
                }
                break;

            /* Fire -- Flammable objects */
            case GF_FIRE:
                if (hates_fire(i_ptr)) {
                    do_kill = TRUE;
                    note_kill = (plural ? " burn up!" : " burns up!");
                    if (f3 & TR3_IGNORE_FIRE) ignore = TRUE;
                }
                break;

            /* Cold -- potions and flasks */
            case GF_COLD:
                if (hates_cold(i_ptr)) {
                    note_kill = (plural ? " shatter!" : " shatters!");
                    do_kill = TRUE;
                    if (f3 & TR3_IGNORE_COLD) ignore = TRUE;
                }
                break;

            /* Fire + Elec */
            case GF_PLASMA:
                if (hates_fire(i_ptr)) {
                    do_kill = TRUE;
                    note_kill = (plural ? " burn up!" : " burns up!");
                    if (f3 & TR3_IGNORE_FIRE) ignore = TRUE;
                }
                if (hates_elec(i_ptr)) {
                    ignore = FALSE;
                    do_kill = TRUE;
                    note_kill= (plural ? " is destroyed!" : " is destroyed!");
                    if (f3 & TR3_IGNORE_ELEC) ignore = TRUE;
                }
                break;

            /* Fire + Cold */
            case GF_METEOR:
                if (hates_fire(i_ptr)) {
                    do_kill = TRUE;
                    note_kill = (plural ? " burn up!" : " burns up!");
                    if (f3 & TR3_IGNORE_FIRE) ignore = TRUE;
                }
                if (hates_cold(i_ptr)) {
                    ignore = FALSE;
                    do_kill = TRUE;
                    note_kill= (plural ? " shatter!" : " shatters!");
                    if (f3 & TR3_IGNORE_COLD) ignore = TRUE;
                }
                break;

            /* Hack -- break potions and such */		
            case GF_ICE:
            case GF_SHARDS:
            case GF_FORCE:
            case GF_SOUND:
                if (hates_cold(i_ptr)) {
                    note_kill = (plural ? " shatter!" : " shatters!");
                    do_kill = TRUE;
                }
                break;

            /* Mana -- destroys everything */
            case GF_MANA:
                do_kill = TRUE;
                note_kill = (plural ? " is destroyed!" : " is destroyed!");

            /* Holy Orb -- destroys cursed non-artifacts */
            case GF_HOLY_ORB:
                if (cursed_p(i_ptr)) {
                    do_kill = TRUE;
                    note_kill= (plural ? " is destroyed!" : " is destroyed!");
                }
                break;

            /* Unlock chests */
            case GF_KILL_TRAP:
            case GF_KILL_DOOR:	

                /* Chests are noticed only if trapped or locked */
                if (i_ptr->tval == TV_CHEST) {

                    /* Disarm/Unlock traps */
                    if (i_ptr->pval > 0) {

                        /* Disarm or Unlock */
                        i_ptr->pval = (0 - i_ptr->pval);

                        /* Identify */
                        inven_known(i_ptr);

                        /* Notice */
                        if (i_ptr->marked) {
                            msg_print("Click!");
                            obvious = TRUE;
                        }
                    }
                }

                break;
        }


        /* Attempt to destroy the object */
        if (do_kill) {

            /* Effect "observed" */
            if (i_ptr->marked) {
                obvious = TRUE;
                objdes(i_name, i_ptr, FALSE, 0);
            }

            /* Artifacts, and other objects, get to resist */
            if (is_art || ignore) {

                /* Observe the resist */
                if (i_ptr->marked) {
                    msg_format("The %s %s unaffected!",
                               i_name, (plural ? "are" : "is"));
                }
            }

            /* Kill it */
            else {

                /* Describe if needed */
                if (i_ptr->marked && note_kill) {
                    msg_format("The %s%s", i_name, note_kill);
                }

                /* Delete the object */
                delete_object(y,x);

                /* Redraw */
                lite_spot(y,x);
            }
        }
    }


    /* Affect the grid */
    if (flg & PROJECT_GRID) {

        /* Analyze the type */
        switch (typ) {

            /* Ignore most effects */
            case GF_ACID:
            case GF_ELEC:
            case GF_FIRE:
            case GF_COLD:
            case GF_PLASMA:
            case GF_METEOR:
            case GF_ICE:
            case GF_SHARDS:
            case GF_FORCE:
            case GF_SOUND:
            case GF_MANA:
            case GF_HOLY_ORB:
                break;

            /* Destroy Traps (and Locks) */
            case GF_KILL_TRAP:

                /* Destroy invisible traps */
                if ((c_ptr->feat & 0x3F) == 0x02) {

                    /* Hack -- special message */
                    if (player_can_see_bold(y,x)) {
                        msg_print("There is a bright flash of light!");
                        obvious = TRUE;
                    }

                    /* Destroy the trap */
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x01);

                    /* Notice */
                    note_spot(y, x);

                    /* Redraw */
                    lite_spot(y, x);
                }

                /* Destroy visible traps */
                if (((c_ptr->feat & 0x3F) >= 0x10) &&
                    ((c_ptr->feat & 0x3F) <= 0x1F)) {

                    /* Hack -- special message */
                    if (c_ptr->feat & CAVE_MARK) {
                        msg_print("There is a bright flash of light!");
                        obvious = TRUE;
                    }

                    /* Destroy the trap */
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x01);

                    /* Notice */
                    note_spot(y, x);

                    /* Redraw */
                    lite_spot(y, x);
                }

                /* Secret / Locked doors are found and unlocked */
                else if (((c_ptr->feat & 0x3F) == 0x30) ||
                         (((c_ptr->feat & 0x3F) >= 0x21) &&
                          ((c_ptr->feat & 0x3F) <= 0x27))) {

                    /* Notice */
                    if (c_ptr->feat & CAVE_MARK) {
                        msg_print("Click!");
                        obvious = TRUE;
                    }

                    /* Unlock the door */
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x20);

                    /* Notice */
                    note_spot(y, x);

                    /* Redraw */
                    lite_spot(y, x);
                }

                break;

            /* Destroy Doors (and traps) */
            case GF_KILL_DOOR:	

                /* Destroy invisible traps */
                if ((c_ptr->feat & 0x3F) == 0x02) {

                    /* Hack -- special message */
                    if (player_can_see_bold(y,x)) {
                        msg_print("There is a bright flash of light!");
                        obvious = TRUE;
                    }

                    /* Destroy the feature */
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x01);

                    /* Forget the wall */
                    c_ptr->feat &= ~CAVE_MARK;

                    /* Notice */
                    note_spot(y, x);

                    /* Redraw */
                    lite_spot(y, x);
                }

                /* Destroy all visible traps and open doors */
                if (((c_ptr->feat & 0x3F) == 0x04) ||
                    ((c_ptr->feat & 0x3F) == 0x05) ||
                    (((c_ptr->feat & 0x3F) >= 0x10) &&
                     ((c_ptr->feat & 0x3F) <= 0x1F))) {

                    /* Hack -- special message */
                    if (c_ptr->feat & CAVE_MARK) {
                        msg_print("There is a bright flash of light!");
                        obvious = TRUE;
                    }

                    /* Destroy the feature */
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x01);

                    /* Forget the wall */
                    c_ptr->feat &= ~CAVE_MARK;

                    /* Notice */
                    note_spot(y, x);

                    /* Redraw */
                    lite_spot(y, x);
                }

                /* Destroy all closed doors */
                if (((c_ptr->feat & 0x3F) >= 0x20) &&
                    ((c_ptr->feat & 0x3F) <= 0x2F)) {

                    /* Hack -- special message */
                    if (c_ptr->feat & CAVE_MARK) {
                        msg_print("There is a bright flash of light!");
                        obvious = TRUE;
                    }

                    /* Destroy the feature */
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x01);

                    /* Forget the wall */
                    c_ptr->feat &= ~CAVE_MARK;

                    /* Notice */
                    note_spot(y, x);

                    /* Redraw */
                    lite_spot(y, x);

                    /* Update some things */
                    p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
                }

                break;


            /* Destroy walls (and doors) */
            case GF_KILL_WALL:	

                /* Non-walls (etc) */
                if ((c_ptr->feat & 0x3F) < 0x20) break;

                /* Permanent walls */
                if ((c_ptr->feat & 0x3F) >= 0x3C) break;

                /* Granite */
                if ((c_ptr->feat & 0x3F) >= 0x38) {

                    /* Message */
                    if (c_ptr->feat & CAVE_MARK) {
                        msg_print("The wall turns into mud!");
                        obvious = TRUE;
                    }
                    
                    /* Destroy the wall */
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x01);
                }

                /* Quartz / Magma with treasure */
                else if ((c_ptr->feat & 0x3F) >= 0x34) {

                    /* Message */
                    if (c_ptr->feat & CAVE_MARK) {
                        msg_print("The vein turns into mud!");
                        msg_print("You have found something!");
                        obvious = TRUE;
                    }

                    /* Destroy the wall */
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x01);

                    /* Place some gold */
                    place_gold(y, x);
                }

                /* Quartz / Magma */
                else if ((c_ptr->feat & 0x3F) >= 0x32) {

                    /* Message */
                    if (c_ptr->feat & CAVE_MARK) {
                        msg_print("The vein turns into mud!");
                        obvious = TRUE;
                    }

                    /* Destroy the wall */
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x01);
                }

                /* Rubble */
                else if ((c_ptr->feat & 0x3F) == 0x31) {

                    /* Message */
                    if (c_ptr->feat & CAVE_MARK) {
                        msg_print("The rubble turns into mud!");
                        obvious = TRUE;
                    }

                    /* Destroy the rubble */
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x01);

                    /* Hack -- place an object */
                    if (rand_int(100) < 10) {

                        /* Found something */
                        if (player_can_see_bold(y,x)) {
                            msg_print("There was something buried in the rubble!");
                            obvious = TRUE;
                        }

                        /* Place gold */
                        place_object(y, x, FALSE, FALSE);
                    }
                }

                /* Destroy doors (and secret doors) */
                else if ((c_ptr->feat & 0x3F) >= 0x20) {
                    
                    /* Hack -- special message */
                    if (c_ptr->feat & CAVE_MARK) {
                        msg_print("The door turns into mud!");
                        obvious = TRUE;
                    }

                    /* Destroy the feature */
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x01);
                }
                    
                /* Forget the wall */
                c_ptr->feat &= ~CAVE_MARK;

                /* Notice */
                note_spot(y, x);

                /* Redraw */
                lite_spot(y, x);
                
                /* Update some things */
                p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);

                break;

            /* Make doors */
            case GF_MAKE_DOOR:

                /* Require a "naked" floor grid */
                if (!naked_grid_bold(y, x)) break;

                /* Create a closed door */
                c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x20);

                /* Notice */
                note_spot(y, x);

                /* Redraw */
                lite_spot(y, x);
                
                /* Observe */
                if (c_ptr->feat & CAVE_MARK) obvious = TRUE;

                /* Update some things */
                p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);

                break;

            /* Make traps */
            case GF_MAKE_TRAP:

                /* Require a "naked" floor grid */
                if (!naked_grid_bold(y, x)) break;

                /* Place a trap */
                place_trap(y, x);

                /* Notice */
                note_spot(y, x);

                /* Redraw */
                lite_spot(y, x);

                break;

            /* Lite up the grid */
            case GF_LITE_WEAK:
            case GF_LITE:

                /* Turn on the light */
                c_ptr->feat |= CAVE_GLOW;

                /* Notice */
                note_spot(y, x);
                
                /* Redraw */
                lite_spot(y, x);

                /* Observe */
                if (player_can_see_bold(y,x)) obvious = TRUE;

                /* Mega-Hack -- Update the monster in the affected grid */
                /* This allows "spear of light" (etc) to work "correctly" */
                if (c_ptr->m_idx) update_mon(c_ptr->m_idx, FALSE);

                break;

            /* Darken the grid */
            case GF_DARK_WEAK:
            case GF_DARK:

                /* Notice */
                if (player_can_see_bold(y,x)) obvious = TRUE;

                /* Turn off the light. */
                c_ptr->feat &= ~CAVE_GLOW;

                /* Hack -- Forget "boring" grids */
                if ((c_ptr->feat & 0x3F) <= 0x02) {
                
                    /* Forget */
                    c_ptr->feat &= ~CAVE_MARK;

                    /* Notice */
                    note_spot(y, x);
                }
                
                /* Redraw */
                lite_spot(y, x);

                /* Mega-Hack -- Update the monster in the affected grid */
                /* This allows "spear of light" (etc) to work "correctly" */
                if (c_ptr->m_idx) update_mon(c_ptr->m_idx, FALSE);

                /* All done */
                break;
        }
    }

    /* Return "Anything seen?" */
    return (obvious);
}



/*
 * Helper function for "project()" below.
 *
 * Handle a beam/bolt/ball causing damage to a monster.
 *
 * This routine takes a "source monster" (by index) which is mostly used to
 * determine if the player is causing the damage, and a "radius" (see below),
 * which is used to decrease the power of explosions with distance, and a
 * location, via integers which are modified by certain types of attacks
 * (polymorph and teleport being the obvious ones), a default damage, which
 * is modified as needed based on various properties, and finally a "damage
 * type" (see below).
 *
 * Note that this routine can handle "no damage" attacks (like teleport) by
 * taking a "zero" damage, and can even take "parameters" to attacks (like
 * confuse) by accepting a "damage", using it to calculate the effect, and
 * then setting the damage to zero.  Note that the "damage" parameter is
 * divided by the radius, so monsters not at the "epicenter" will not take
 * as much damage (or whatever)...
 *
 * Note that "polymorph" is dangerous, since a failure in "place_monster()"'
 * may result in a dereference of an invalid pointer.  XXX XXX XXX
 *
 * Various messages are produced, and damage is applied.
 *
 * Just "casting" a substance (i.e. plasma) does not make you immune, you must
 * actually be "made" of that substance, or "breathe" big balls of it.
 *
 * We assume that "Plasma" monsters, and "Plasma" breathers, are immune
 * to plasma.
 *
 * We assume "Nether" is an evil, necromantic force, so it doesn't hurt undead,
 * and hurts evil less.  If can breath nether, then it resists it as well.
 *
 * Damage reductions use the following formulas:
 *   Note that "dam = dam * 6 / (randint(6) + 6);"
 *     gives avg damage of .655, ranging from .858 to .500
 *   Note that "dam = dam * 5 / (randint(6) + 6);"
 *     gives avg damage of .544, ranging from .714 to .417
 *   Note that "dam = dam * 4 / (randint(6) + 6);"
 *     gives avg damage of .444, ranging from .556 to .333
 *   Note that "dam = dam * 3 / (randint(6) + 6);"
 *     gives avg damage of .327, ranging from .427 to .250
 *   Note that "dam = dam * 2 / (randint(6) + 6);"
 *     gives something simple.
 *
 * In this function, "result" messages are postponed until the end, where
 * the "note" string is appended to the monster name, if not NULL.  So,
 * to make a spell have "no effect" just set "note" to NULL.  You should
 * also set "notice" to FALSE, or the player will learn what the spell does.
 *
 * We attempt to return "TRUE" if the player saw anything "useful" happen.
 */
static bool project_m(int who, int r, int y, int x, int dam, int typ, int flg)
{
    int i;

    cave_type *c_ptr = &cave[y][x];

    monster_type *m_ptr = &m_list[c_ptr->m_idx];

    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    cptr name = (r_name + r_ptr->name);

    /* Is the monster "seen"? */
    bool seen = m_ptr->ml;

    /* Were the "effects" obvious (if seen)? */
    bool obvious = FALSE;


    /* Polymorph setting (true or false) */
    int do_poly = 0;

    /* Teleport setting (max distance) */
    int do_dist = 0;

    /* Confusion setting (amount to confuse) */
    int do_conf = 0;

    /* Stunning setting (amount to stun) */
    int do_stun = 0;

    /* Sleep amount (amount to sleep) */
    int do_sleep = 0;


    /* "Damage" factor.  Multiply by "mul/div" */
    int mul = 1, div = 1;

    /* Hold the monster name */
    char m_name[80];

    /* Assume no note */
    cptr note = NULL;

    /* Assume a default death */
    cptr note_dies = " dies.";


    /* Get the monster name (BEFORE polymorphing) */
    monster_desc(m_name, m_ptr, 0);



    /* Some monsters get "destroyed" */
    if ((r_ptr->flags3 & RF3_DEMON) ||
        (r_ptr->flags3 & RF3_UNDEAD) ||
        (r_ptr->flags2 & RF2_STUPID) ||
        (strchr("Evg", r_ptr->r_char))) {

        /* Special note at death */
        note_dies = " is destroyed.";
    }


    /* Hack -- decrease power over distance */
    if (r) div = r + 1;

    /* Adjust damage */
    dam = dam * mul / div;


    /* Analyze the damage type */
    switch (typ) {

      /* Magic Missile -- pure damage */
      case GF_MISSILE:
        if (seen) obvious = TRUE;
        break;

      /* Acid */
      case GF_ACID:
        if (seen) obvious = TRUE;
        if (r_ptr->flags3 & RF3_IM_ACID) {
            note = " resists a lot.";
            dam /= 9;
            if (seen) r_ptr->r_flags3 |= RF3_IM_ACID;
        }
        break;

      /* Electricity */
      case GF_ELEC:
        if (seen) obvious = TRUE;
        if (r_ptr->flags3 & RF3_IM_ELEC) {
            note = " resists a lot.";
            dam /= 9;
            if (seen) r_ptr->r_flags3 |= RF3_IM_ELEC;
        }
        break;

      /* Fire damage */
      case GF_FIRE:
        if (seen) obvious = TRUE;
        if (r_ptr->flags3 & RF3_IM_FIRE) {
            note = " resists a lot.";
            dam /= 9;
            if (seen) r_ptr->r_flags3 |= RF3_IM_FIRE;
        }
        break;

      /* Cold */
      case GF_COLD:
        if (seen) obvious = TRUE;
        if (r_ptr->flags3 & RF3_IM_COLD) {
            note = " resists a lot.";
            dam /= 9;
            if (seen) r_ptr->r_flags3 |= RF3_IM_COLD;
        }
        break;

      /* Poison */
      case GF_POIS:
        if (seen) obvious = TRUE;
        if (r_ptr->flags3 & RF3_IM_POIS) {
            note = " resists a lot.";
            dam /= 9;
            if (seen) r_ptr->r_flags3 |= RF3_IM_POIS;
        }
        break;

      /* Holy Orb -- hurts Evil */
      case GF_HOLY_ORB:
        if (seen) obvious = TRUE;
        if (r_ptr->flags3 & RF3_EVIL) {
            dam *= 2;
            note = " is hit hard.";
            if (seen) r_ptr->r_flags3 |= RF3_EVIL;
        }
        break;

      /* Arrow -- XXX no defense */
      case GF_ARROW:
        if (seen) obvious = TRUE;
        break;

      /* Plasma -- XXX perhaps check ELEC or FIRE */
      case GF_PLASMA:
        if (seen) obvious = TRUE;
        if (prefix(name, "Plasma") ||
            (r_ptr->flags4 & RF4_BR_PLAS)) {
            note = " resists.";
            dam *= 3; dam /= (randint(6)+6);
        }
        break;

      /* Nether -- see above */
      case GF_NETHER:
        if (seen) obvious = TRUE;
        if (r_ptr->flags3 & RF3_UNDEAD) {
            note = " is immune.";
            dam = 0;
            if (seen) r_ptr->r_flags3 |= RF3_UNDEAD;
        }
        else if (r_ptr->flags4 & RF4_BR_NETH) {
            note = " resists.";
            dam *= 3; dam /= (randint(6)+6);
        }
        else if (r_ptr->flags3 & RF3_EVIL) {
            dam /= 2;
            note = " resists somewhat.";
            if (seen) r_ptr->r_flags3 |= RF3_EVIL;
        }
        break;

      /* Water (acid) damage -- Water spirits/elementals are immune */
      case GF_WATER:
        if (seen) obvious = TRUE;
        if ((r_ptr->r_char == 'E') && prefix(name, "W")) {
            note = " is immune.";
            dam = 0;
        }
        break;

      /* Chaos -- Chaos breathers resist */
      case GF_CHAOS:
        if (seen) obvious = TRUE;
        do_poly = TRUE;
        do_conf = (5 + randint(11)) * mul / div;
        if (r_ptr->flags4 & RF4_BR_CHAO) {
            note = " resists.";
            dam *= 3; dam /= (randint(6)+6);
            do_poly = FALSE;
        }
        break;

      /* Shards -- Shard breathers resist */
      case GF_SHARDS:
        if (seen) obvious = TRUE;
        if (r_ptr->flags4 & RF4_BR_SHAR) {
            note = " resists.";
            dam *= 3; dam /= (randint(6)+6);
        }
        break;

      /* Sound -- Sound breathers resist */
      case GF_SOUND:
        if (seen) obvious = TRUE;
        do_stun = (10 + randint(15)) * mul / div;
        if (r_ptr->flags4 & RF4_BR_SOUN) {
            note = " resists.";
            dam *= 2; dam /= (randint(6)+6);
        }
        break;

      /* Confusion */
      case GF_CONFUSION:
        if (seen) obvious = TRUE;
        do_conf = (10 + randint(15)) * mul / div;
        if (r_ptr->flags4 & RF4_BR_CONF) {
            note = " resists.";
            dam *= 2; dam /= (randint(6)+6);
        }
        else if (r_ptr->flags3 & RF3_NO_CONF) {
            note = " resists somewhat.";
            dam /= 2;
        }
        break;

      /* Disenchantment -- Breathers and Disenchanters resist */
      case GF_DISENCHANT:
        if (seen) obvious = TRUE;
        if ((r_ptr->flags4 & RF4_BR_DISE) ||
            prefix(name, "Disen")) {
            note = " resists.";
            dam *= 3; dam /= (randint(6)+6);
        }
        break;

      /* Nexus -- Breathers and Existers resist */
      case GF_NEXUS:
        if (seen) obvious = TRUE;
        if ((r_ptr->flags4 & RF4_BR_NEXU) ||
            prefix(name, "Nexus")) {
            note = " resists.";
            dam *= 3; dam /= (randint(6)+6);
        }
        break;

      /* Force */
      case GF_FORCE:
        if (seen) obvious = TRUE;
        do_stun = randint(15) * mul / div;
        if (r_ptr->flags4 & RF4_BR_WALL) {
            note = " resists.";
            dam *= 3; dam /= (randint(6)+6);
        }
        break;

      /* Inertia -- breathers resist */
      case GF_INERTIA:
        if (seen) obvious = TRUE;
        if (r_ptr->flags4 & RF4_BR_INER) {
            note = " resists.";
            dam *= 3; dam /= (randint(6)+6);
        }
        break;

      /* Time -- breathers resist */
      case GF_TIME:
        if (seen) obvious = TRUE;
        if (r_ptr->flags4 & RF4_BR_TIME) {
            note = " resists.";
            dam *= 3; dam /= (randint(6)+6);
        }
        break;

      /* Gravity -- breathers resist */
      case GF_GRAVITY:
        if (seen) obvious = TRUE;
        do_dist = 10;
        if (r_ptr->flags4 & RF4_BR_GRAV) {
            note = " resists.";
            dam *= 3; dam /= (randint(6)+6);
            do_dist = 0;
        }
        break;

      /* Pure damage */
      case GF_MANA:
        if (seen) obvious = TRUE;
        break;

      /* Meteor -- powerful magic missile */
      case GF_METEOR:
        if (seen) obvious = TRUE;
        break;

      /* Ice -- Cold + Cuts + Stun */
      case GF_ICE:
        if (seen) obvious = TRUE;
        do_stun = randint(15) * mul / div;
        if (r_ptr->flags3 & RF3_IM_COLD) {
            note = " resists a lot.";
            dam /= 9;
            if (seen) r_ptr->r_flags3 |= RF3_IM_COLD;
        }
        break;


      /* Drain Life */
      case GF_OLD_DRAIN:
        if (seen) obvious = TRUE;
        if ((r_ptr->flags3 & RF3_UNDEAD) ||
            (r_ptr->flags3 & RF3_DEMON) ||
            (strchr("Egv", r_ptr->r_char))) {

            if (r_ptr->flags3 & RF3_UNDEAD) {
                if (seen) r_ptr->r_flags3 |= RF3_UNDEAD;
            }
            if (r_ptr->flags3 & RF3_DEMON) {
                if (seen) r_ptr->r_flags3 |= RF3_DEMON;
            }

            note = " is unaffected!";
            obvious = FALSE;
            dam = 0;
        }

        break;

      /* Polymorph monster (Use "dam" as "power") */	
      case GF_OLD_POLY:

        if (seen) obvious = TRUE;
        
        /* Attempt to polymorph (see below) */
        do_poly = TRUE;

        /* Powerful monsters can resist */
        if ((r_ptr->flags1 & RF1_UNIQUE) ||
            (r_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10)) {

            note = " is unaffected!";
            do_poly = FALSE;
            obvious = FALSE;
        }

        /* No "real" damage */
        dam = 0;	

        break;


      /* Clone monsters (Ignore "dam") */
      case GF_OLD_CLONE:

        if (seen) obvious = TRUE;

        /* Heal fully */
        m_ptr->hp = m_ptr->maxhp;

        /* Speed up */
        if (m_ptr->mspeed < 150) m_ptr->mspeed += 10;

        /* Attempt to clone. */
        if (multiply_monster(c_ptr->m_idx)) {
            note = " spawns!";
        }

        /* No "real" damage */
        dam = 0;	

        break;


      /* Heal Monster (use "dam" as amount of healing) */
      case GF_OLD_HEAL:

        if (seen) obvious = TRUE;

        /* Wake up */
        m_ptr->csleep = 0;

        /* Heal */
        m_ptr->hp += dam;

        /* No overflow */
        if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

        /* Redraw (later) if needed */
        if (health_who == c_ptr->m_idx) p_ptr->redraw |= (PR_HEALTH);

        /* Message */
        note = " looks healthier.";

        /* No "real" damage */
        dam = 0;
        break;


      /* Speed Monster (Ignore "dam") */
      case GF_OLD_SPEED:

        if (seen) obvious = TRUE;

        /* Speed up */
        if (m_ptr->mspeed < 150) m_ptr->mspeed += 10;
        note = " starts moving faster.";

        /* No "real" damage */
        dam = 0;
        break;


      /* Slow Monster (Use "dam" as "power") */
      case GF_OLD_SLOW:

        if (seen) obvious = TRUE;

        /* Powerful monsters can resist */
        if ((r_ptr->flags1 & RF1_UNIQUE) ||
            (r_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10)) {

            note = " is unaffected!";
            obvious = FALSE;
        }

        /* Normal monsters slow down */
        else {
            if (m_ptr->mspeed > 60) m_ptr->mspeed -= 10;
            note = " starts moving slower.";
        }

        /* No "real" damage */
        dam = 0;
        break;


      /* Sleep (Use "dam" as "power") */
      case GF_OLD_SLEEP:

        if (seen) obvious = TRUE;

        /* Attempt a saving throw */
        if ((r_ptr->flags1 & RF1_UNIQUE) ||
            (r_ptr->flags3 & RF3_NO_SLEEP) ||
            (r_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10)) {

            /* Memorize a flag */
            if (r_ptr->flags3 & RF3_NO_SLEEP) {
                if (seen) r_ptr->r_flags3 |= RF3_NO_SLEEP;
            }

            /* No obvious effect */
            note = " is unaffected!";
            obvious = FALSE;
        }
        else {

            /* Go to sleep (much) later */
            note = " falls asleep!";
            do_sleep = 500;
        }

        /* No "real" damage */
        dam = 0;
        break;


      /* Confusion (Use "dam" as "power") */
      case GF_OLD_CONF:

        if (seen) obvious = TRUE;

        /* Get confused later */
        do_conf = damroll(3, (dam / 2)) + 1;

        /* Attempt a saving throw */
        if ((r_ptr->flags1 & RF1_UNIQUE) ||
            (r_ptr->flags3 & RF3_NO_CONF) ||
            (r_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10)) {

            /* Memorize a flag */
            if (r_ptr->flags3 & RF3_NO_CONF) {
                if (seen) r_ptr->r_flags3 |= RF3_NO_CONF;
            }

            /* Resist */
            do_conf = 0;

            /* No obvious effect */
            note = " is unaffected!";
            obvious = FALSE;
        }

        /* No "real" damage */
        dam = 0;
        break;


      /* Confusion (Use "dam" as "power") */
      case GF_OLD_SCARE:

        if (seen) obvious = TRUE;

        /* Attempt a saving throw */
        if ((r_ptr->flags1 & RF1_UNIQUE) ||
            (r_ptr->flags3 & RF3_NO_FEAR) ||
            (r_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10)) {

            /* No obvious effect */
            note = " is unaffected!";
            obvious = FALSE;
        }

        /* Get scared */
        else {

            int tmp = m_ptr->monfear + damroll(3, (dam / 2)) + 1;

            /* Set fear */
            m_ptr->monfear = (tmp < 200) ? tmp : 200;

            /* Sound */
            sound(SOUND_FLEE);

            /* Message */
            note = " flees in terror!";
        }

        /* No "real" damage */
        dam = 0;
        break;




      /* Teleport monster (Use "dam" as "power") */
      case GF_OLD_TPORT:

        if (seen) obvious = TRUE;

        /* Prepare to teleport */      	
        do_dist = dam;

        /* No "real" damage */
        dam = 0;
        break;




      /* Lite -- opposite of Dark */
      case GF_LITE:
        if (seen) obvious = TRUE;
        if (r_ptr->flags4 & RF4_BR_LITE) {
            note = " resists.";
            dam *= 2; dam /= (randint(6)+6);
        }
        else if (r_ptr->flags3 & RF3_HURT_LITE) {
            if (seen) r_ptr->r_flags3 |= RF3_HURT_LITE;
            note = " cringes from the light!";
            note_dies = " shrivels away in the light!";
            dam *= 2;
        }
        break;

      /* Dark -- opposite of Lite */
      case GF_DARK:
        if (seen) obvious = TRUE;
        if (r_ptr->flags4 & RF4_BR_DARK) {
            note = " resists.";
            dam *= 2; dam /= (randint(6)+6);
        }
        break;


      /* Lite, but only hurts susceptible creatures */
      case GF_LITE_WEAK:

        /* Hurt by light */
        if (r_ptr->flags3 & RF3_HURT_LITE) {

            /* Obvious effect */
            if (seen) obvious = TRUE;

            /* Memorize the effects */
            if (seen) r_ptr->r_flags3 |= RF3_HURT_LITE;

            /* Special effect */
            note = " cringes from the light!";
            note_dies = " shrivels away in the light!";
        }

        /* Normally no damage */
        else {

            /* No damage */
            dam = 0;
        }

        break;

      /* Dark, but only damages if susceptible */
      case GF_DARK_WEAK:

        /* No damage */
        dam = 0;

        break;



      /* Stone to Mud */
      case GF_KILL_WALL:

        /* Hurt by rock remover */
        if (r_ptr->flags3 & RF3_HURT_ROCK) {

            /* Notice effect */
            if (seen) obvious = TRUE;

            /* Memorize the effects */
            if (seen) r_ptr->r_flags3 |= RF3_HURT_ROCK;

            /* Cute little message */
            note = " loses some skin!";
            note_dies = " dissolves!";
        }

        /* Usually, ignore the effects */
        else {

            /* No damage */
            dam = 0;
        }

        break;


      /* Various -- No damage */
      case GF_KILL_DOOR:
      case GF_KILL_TRAP:
      case GF_MAKE_WALL:
      case GF_MAKE_DOOR:
      case GF_MAKE_TRAP:

        /* No damage */
        dam = 0;

        break;
    }



    /* "Unique" monsters cannot be polymorphed */
    if (r_ptr->flags1 & RF1_UNIQUE) do_poly = FALSE;


    /* "Unique" monsters can only be "killed" by the player */
    if (r_ptr->flags1 & RF1_UNIQUE) {

        /* Uniques may only be killed by the player */
        if (who && (dam > m_ptr->hp)) dam = m_ptr->hp;
    }


    /* Check for death */
    if (dam > m_ptr->hp) {

        /* Extract method of death */
        note = note_dies;
    }

    /* Mega-Hack -- Handle "polymorph" -- monsters get a saving throw */
    else if (do_poly && (randint(90) > r_ptr->level)) {

        /* Default -- assume no polymorph */
        note = " is unaffected!";

        /* Pick a "new" monster race */
        i = poly_r_idx(m_ptr->r_idx);

        /* Handle polymorh */
        if (i != m_ptr->r_idx) {

            /* Obvious */
            if (seen) obvious = TRUE;

            /* Monster polymorphs */
            note = " changes!";

            /* Turn off the damage */
            dam = 0;

            /* "Kill" the "old" monster */
            delete_monster_idx(c_ptr->m_idx);

            /* Create a new monster (no groups) */
            (void)place_monster_aux(y, x, i, FALSE, FALSE);

            /* XXX XXX XXX Hack -- Assume success */

            /* Hack -- Get new monster */
            m_ptr = &m_list[c_ptr->m_idx];
            
            /* Hack -- Get new race */
            r_ptr = &r_info[m_ptr->r_idx];
        }
    }

    /* Handle "teleport" */
    else if (do_dist) {

        /* Obvious */
        if (seen) obvious = TRUE;

        /* Message */
        note = " disappears!";

        /* Teleport */
        teleport_away(c_ptr->m_idx, do_dist);

        /* Hack -- get new location */
        y = m_ptr->fy;
        x = m_ptr->fx;

        /* Hack -- get new grid */
        c_ptr = &cave[y][x];
    }

    /* Sound and Impact breathers never stun */
    else if (do_stun &&
             !(r_ptr->flags4 & RF4_BR_SOUN) &&
             !(r_ptr->flags4 & RF4_BR_WALL)) {

        /* Obvious */
        if (seen) obvious = TRUE;

        /* Get confused */
        if (m_ptr->stunned) {
            note = " is more dazed.";
            if (m_ptr->stunned < 220) {
                m_ptr->stunned += (do_stun / 2);
            }
        }
        else {
            note = " is dazed.";
            m_ptr->stunned = do_stun;
        }
    }

    /* Confusion and Chaos breathers (and sleepers) never confuse */
    else if (do_conf &&
            !(r_ptr->flags3 & RF3_NO_CONF) &&
            !(r_ptr->flags4 & RF4_BR_CONF) &&
            !(r_ptr->flags4 & RF4_BR_CHAO)) {

        /* Obvious */
        if (seen) obvious = TRUE;

        /* Already partially confused */
        if (m_ptr->confused) {
            note = " looks more confused.";
            if (m_ptr->confused < 240) {
                m_ptr->confused += (do_conf / 2);
            }
        }

        /* Was not confused */
        else {
            note = " looks confused.";
            m_ptr->confused = do_conf;
        }
    }


    /* If another monster did the damage, hurt the monster by hand */
    if (who) {

        /* Redraw (later) if needed */
        if (health_who == c_ptr->m_idx) p_ptr->redraw |= (PR_HEALTH);

        /* Wake the monster up */
        m_ptr->csleep = 0;

        /* Hurt the monster */
        m_ptr->hp -= dam;

        /* Dead monster */
        if (m_ptr->hp < 0) {

            /* Generate treasure, etc */
            monster_death(c_ptr->m_idx);

            /* Delete the monster */
            delete_monster_idx(c_ptr->m_idx);

            /* Give detailed messages if destroyed */
            if (note) msg_format("%^s%s", m_name, note);
        }
        
        /* Damaged monster */
        else {

            /* Give detailed messages if visible or destroyed */
            if (note && seen) msg_format("%^s%s", m_name, note);

            /* Hack -- Pain message */
            else if (dam > 0) message_pain(c_ptr->m_idx, dam);

            /* Hack -- handle sleep */
            if (do_sleep) m_ptr->csleep = do_sleep;
        }
    }

    /* If the player did it, give him experience, check fear */
    else {

        bool fear = FALSE;

        /* Hurt the monster, check for fear and death */
        if (mon_take_hit(c_ptr->m_idx, dam, &fear, note_dies)) {
        
            /* Dead monster */
        }

        /* Damaged monster */
        else {

            /* Give detailed messages if visible or destroyed */
            if (note && seen) msg_format("%^s%s", m_name, note);

            /* Hack -- Pain message */
            else if (dam > 0) message_pain(c_ptr->m_idx, dam);

            /* Take note */
            if (fear && (m_ptr->ml)) {

                /* Sound */
                sound(SOUND_FLEE);

                /* Message */
                msg_format("%^s flees in terror!", m_name);
            }
        
            /* Hack -- handle sleep */
            if (do_sleep) m_ptr->csleep = do_sleep;
        }
    }


    /* Hack -- Update the monster */
    update_mon(c_ptr->m_idx, FALSE);

    /* Hack -- Redraw the monster grid anyway */
    lite_spot(y, x);


    /* Return "Anything seen?" */
    return (obvious);
}






/*
 * Helper function for "project()" below.
 *
 * Handle a beam/bolt/ball causing damage to the player.
 *
 * This routine takes a "source monster" (by index), a "distance", a default
 * "damage", and a "damage type".  See "project_m()" above.
 *
 * If the source monster is "breathing" (as opposed to "casting a bolt"),
 * then PROJECT_XTRA will be set, and we can do a little "extra" damage.
 *
 * If "rad" is non-zero, then the blast was centered elsewhere, and the damage
 * is reduced (see "project_m()" above).  This can happen if a monster breathes
 * at the player and hits a wall instead.
 *
 * We return "TRUE" if any "obvious" effects were observed.  XXX XXX Actually,
 * we just assume that the effects were obvious, for historical reasons.
 */
static bool project_p(int who, int r, int y, int x, int dam, int typ, int flg)
{
    int i, k = 0;


    /* Hack -- assume obvious */
    bool obvious = TRUE;

    /* Player blind-ness */
    bool blind = (p_ptr->blind ? TRUE : FALSE);

    /* Player needs a "description" (he is blind) */
    bool fuzzy = FALSE;

    /* Player should take "extra" effects from "breath" */
    bool extra = (flg & PROJECT_XTRA) ? TRUE : FALSE;

    /* "Damage" factor.  Multiply by "mul/div" */
    int mul = 1, div = 1;

    /* Source monster */
    monster_type *m_ptr;

    /* Monster name (for attacks) */
    char m_name[80];

    /* Monster name (for damage) */
    char killer[80];

    /* Hack -- messages */
    cptr act = NULL;


    /* Hack -- decrease power over distance */
    if (r) div = r + 1;

    /* Adjust damage */
    dam = dam * mul / div;

    /* Hack -- always do at least one point of damage */
    if (dam <= 0) dam = 1;

    /* Hack -- Never do excessive damage */
    if (dam > 1600) dam = 1600;


    /* If the player is blind, be more descriptive */
    if (blind) fuzzy = TRUE;


    /* Paranoia */
    if (!who) return (FALSE);

    /* Get the source monster */
    m_ptr = &m_list[who];

    /* Get the monster name */
    monster_desc(m_name, m_ptr, 0);

    /* Get the monster's real name */
    monster_desc(killer, m_ptr, 0x88);


    /* Analyze the damage */
    switch (typ) {

        /* Standard damage -- hurts inventory too */
        case GF_ACID:
            if (fuzzy) msg_print("You are hit by acid!");
            acid_dam(dam, killer);
            break;

        /* Standard damage -- hurts inventory too */
        case GF_FIRE:
            if (fuzzy) msg_print("You are hit by fire!");
            fire_dam(dam, killer);
            break;

        /* Standard damage -- hurts inventory too */
        case GF_COLD:
            if (fuzzy) msg_print("You are hit by cold!");
            cold_dam(dam, killer);
            break;

        /* Standard damage -- hurts inventory too */
        case GF_ELEC:
            if (fuzzy) msg_print("You are hit by lightning!");
            elec_dam(dam, killer);
            break;

        /* Standard damage -- also poisons player */
        case GF_POIS:
            if (fuzzy) msg_print("You are hit by poison!");
            if (p_ptr->resist_pois) dam = (dam + 2) / 3;
            if (p_ptr->oppose_pois) dam = (dam + 2) / 3;
            take_hit(dam, killer);
            if (!(p_ptr->resist_pois || p_ptr->oppose_pois)) {
                (void)set_poisoned(p_ptr->poisoned + rand_int(dam) + 10);
            }
            break;

        /* Standard damage */
        case GF_MISSILE:
            if (fuzzy) msg_print("You are hit by something!");
            take_hit(dam, killer);
            break;

        /* Holy Orb -- Player only takes partial damage */
        case GF_HOLY_ORB:
            if (fuzzy) msg_print("You are hit by something!");
            dam /= 2;
            take_hit(dam, killer);
            break;

        /* Arrow -- XXX no dodging */
        case GF_ARROW:
            if (fuzzy) msg_print("You are hit by something sharp!");
            take_hit(dam, killer);
            break;

        /* Plasma -- XXX No resist */
        case GF_PLASMA:
            if (fuzzy) msg_print("You are hit by something!");
            take_hit(dam, killer);
            if (extra && !p_ptr->resist_sound) {
                int k = (randint((dam > 40) ? 35 : (dam * 3 / 4 + 5)));
                (void)set_stun(p_ptr->stun + k);
            }
            break;

        /* Nether -- drain experience */
        case GF_NETHER:
            if (fuzzy) msg_print("You are hit by something strange!");
            if (p_ptr->resist_neth) {
                dam *= 6; dam /= (randint(6) + 6);
            }
            else {
                if (!extra && p_ptr->hold_life && (rand_int(100) < 90)) {
                    msg_print("You keep hold of your life force!");
                }
                else if (extra && p_ptr->hold_life && (rand_int(100) < 75)) {
                    msg_print("You keep hold of your life force!");
                }
                else if (p_ptr->hold_life) {
                    msg_print("You feel your life slipping away!");
                    lose_exp(200 + (p_ptr->exp/1000) * MON_DRAIN_LIFE);
                }
                else {
                    msg_print("You feel your life draining away!");
                    lose_exp(200 + (p_ptr->exp/100) * MON_DRAIN_LIFE);
                }
            }
            take_hit(dam, killer);
            break;

        /* Water -- stun/confuse */
        case GF_WATER:
            if (fuzzy) msg_print("You are hit by something!");
            if (!extra) {
                if (!p_ptr->resist_sound) {
                    (void)set_stun(p_ptr->stun + randint(15));
                }
            }
            else {
                if (!p_ptr->resist_sound) {
                    (void)set_stun(p_ptr->stun + randint(55));
                }
                if (!p_ptr->resist_conf) {
                    (void)set_confused(p_ptr->confused + randint(8) + 6);
                }
            }
            take_hit(dam, killer);
            break;

        /* Chaos -- many effects */
        case GF_CHAOS:
            if (fuzzy) msg_print("You are hit by something strange!");
            if (p_ptr->resist_chaos) {
                dam *= 6; dam /= (randint(6) + 6);
            }
            if (!p_ptr->resist_conf) {
                (void)set_confused(p_ptr->confused + rand_int(20) + 10);
            }
            if (!p_ptr->resist_chaos) {
                (void)set_image(p_ptr->image + randint(10));
            }
            if (extra && !p_ptr->resist_neth && !p_ptr->resist_chaos) {
                if (p_ptr->hold_life && (rand_int(100) < 75)) {
                    msg_print("You keep hold of your life force!");
                }
                else if (p_ptr->hold_life) {
                    msg_print("You feel your life slipping away!");
                    lose_exp(500 + (p_ptr->exp/1000) * MON_DRAIN_LIFE);
                }
                else {
                    msg_print("You feel your life draining away!");
                    lose_exp(5000 + (p_ptr->exp/100) * MON_DRAIN_LIFE);
                }
            }
            take_hit(dam, killer);
            break;

        /* Shards -- mostly cutting */
        case GF_SHARDS:
            if (fuzzy) msg_print("You are hit by something sharp!");
            if (p_ptr->resist_shard) {
                dam *= 6; dam /= (randint(6) + 6);
            }
            else {
                (void)set_cut(p_ptr->cut + dam);
            }
            take_hit(dam, killer);
            break;

        /* Sound -- mostly stunning */
        case GF_SOUND:
            if (fuzzy) msg_print("You are hit by something!");
            if (p_ptr->resist_sound) {
                dam *= 5; dam /= (randint(6) + 6);
            }
            else if (extra) {
                int k = (randint((dam > 90) ? 35 : (dam / 3 + 5)));
                (void)set_stun(p_ptr->stun + k);
            }
            else {
                int k = (randint((dam > 60) ? 25 : (dam / 3 + 5)));
                (void)set_stun(p_ptr->stun + k);
            }
            take_hit(dam, killer);
            break;

        /* Pure confusion */
        case GF_CONFUSION:
            if (fuzzy) msg_print("You are hit by something!");
            if (p_ptr->resist_conf) {
                dam *= 5; dam /= (randint(6) + 6);
            }
            if (extra) {
                if (!p_ptr->resist_conf) {
                    (void)set_confused(p_ptr->confused + randint(20) + 10);
                }
            }
            else {
                if (!p_ptr->resist_conf) {
                    (void)set_confused(p_ptr->confused + randint(15) + 5);
                }
            }
            take_hit(dam, killer);
            break;

        /* Disenchantment -- see above */
        case GF_DISENCHANT:
            if (fuzzy) msg_print("You are hit by something strange!");
            if (p_ptr->resist_disen) {
                dam *= 6; dam /= (randint(6) + 6);
            }
            else {
                (void)apply_disenchant(0);
            }
            take_hit(dam, killer);
            break;

        /* Nexus -- see above */
        case GF_NEXUS:
            if (fuzzy) msg_print("You are hit by something strange!");
            if (p_ptr->resist_nexus) {
                dam *= 6; dam /= (randint(6) + 6);
            }
            else {
                apply_nexus(m_ptr);
            }
            take_hit(dam, killer);
            break;

        /* Force -- mostly stun */
        case GF_FORCE:
            if (fuzzy) msg_print("You are hit by something!");
            if (extra) {
                if (!p_ptr->resist_sound) {
                    (void)set_stun(p_ptr->stun + randint(20));
                }
            }
            else {
                if (!p_ptr->resist_sound) {
                    (void)set_stun(p_ptr->stun + randint(15) + 1);
                }
            }
            take_hit(dam, killer);
            break;

        /* Inertia -- slowness */
        case GF_INERTIA:
            if (fuzzy) msg_print("You are hit by something strange!");
            (void)set_slow(p_ptr->slow + rand_int(4) + 4);
            take_hit(dam, killer);
            break;

        /* Lite -- blinding */
        case GF_LITE_WEAK:
        case GF_LITE:
            if (fuzzy) msg_print("You are hit by something!");
            if (p_ptr->resist_lite) {
                dam *= 4; dam /= (randint(6) + 6);
            }
            else if (!blind && !p_ptr->resist_blind) {
                (void)set_blind(p_ptr->blind + randint(5) + 2);
            }
            take_hit(dam, killer);
            break;

        /* Dark -- blinding */
        case GF_DARK_WEAK:
        case GF_DARK:
            if (fuzzy) msg_print("You are hit by something!");
            if (p_ptr->resist_dark) {
                dam *= 4; dam /= (randint(6) + 6);
            }
            else if (!blind && !p_ptr->resist_blind) {
                (void)set_blind(p_ptr->blind + randint(5) + 2);
            }
            take_hit(dam, killer);
            break;

        /* Time -- bolt fewer effects XXX */
        case GF_TIME:
            if (fuzzy) msg_print("You are hit by something strange!");
            i = randint(10);
            if (!extra && (i == 10)) i = 9;
            switch (i) {

                case 1: case 2: case 3: case 4: case 5:
                    msg_print("You feel life has clocked back.");
                    lose_exp(100 + (p_ptr->exp / 100) * MON_DRAIN_LIFE);
                    break;

                case 6: case 7: case 8: case 9:

                    switch (randint(6)) {
                        case 1: k = A_STR; act = "strong"; break;
                        case 2: k = A_INT; act = "bright"; break;
                        case 3: k = A_WIS; act = "wise"; break;
                        case 4: k = A_DEX; act = "agile"; break;
                        case 5: k = A_CON; act = "hale"; break;
                        case 6: k = A_CHR; act = "beautiful"; break;
                    }

                    msg_format("You're not as %s as you used to be...", act);

                    p_ptr->stat_cur[k] = (p_ptr->stat_cur[k] * 3) / 4;
                    if (p_ptr->stat_cur[k] < 3) p_ptr->stat_cur[k] = 3;
                    p_ptr->update |= (PU_BONUS);
                    break;

                case 10:

                    msg_print("You're not as powerful as you used to be...");

                    for (k = 0; k < 6; k++) {
                        p_ptr->stat_cur[k] = (p_ptr->stat_cur[k] * 3) / 4;
                        if (p_ptr->stat_cur[k] < 3) p_ptr->stat_cur[k] = 3;
                    }
                    p_ptr->update |= (PU_BONUS);
                    break;
            }
            take_hit(dam, killer);
            break;

        /* Gravity -- stun plus slowness plus teleport */
        case GF_GRAVITY:
            if (fuzzy) msg_print("You are hit by something strange!");
            msg_print("Gravity warps around you.");
            teleport_flag = TRUE;
            teleport_dist = 5;
            (void)set_slow(p_ptr->slow + rand_int(4) + 4);
            if (!p_ptr->resist_sound) {
                if (extra) {
                    int k = (randint((dam > 90) ? 35 : (dam / 3 + 5)));
                    (void)set_stun(p_ptr->stun + k);
                }
                else {
                    (void)set_stun(p_ptr->stun + randint(15) + 1);
                }
            }
            take_hit(dam, killer);
            break;

        /* Pure damage */
        case GF_MANA:
            if (fuzzy) msg_print("You are hit by something!");
            take_hit(dam, killer);
            break;

        /* Pure damage */
        case GF_METEOR:
            if (fuzzy) msg_print("You are hit by something!");
            take_hit(dam, killer);
            break;

        /* Ice -- cold plus stun plus cuts */
        case GF_ICE:
            if (fuzzy) msg_print("You are hit by something sharp!");
            cold_dam(dam, killer);
            if (extra) {
                if (!p_ptr->resist_shard) {
                    (void)set_cut(p_ptr->cut + damroll(8, 10));
                }
                if (!p_ptr->resist_sound) {
                    (void)set_stun(p_ptr->stun + randint(25));
                }
            }
            else {
                if (!p_ptr->resist_shard) {
                    (void)set_cut(p_ptr->cut + damroll(5, 8));
                }
                if (!p_ptr->resist_sound) {
                    (void)set_stun(p_ptr->stun + randint(15) + 1);
                }
            }
            break;

        default:
            msg_print("Oops.  Undefined beam/bolt/ball hit player.");
    }


    /* Disturb */
    disturb(1, 0);


    /* Return "Anything seen?" */
    return (obvious);
}









/*
 * Find the char to use to draw a moving bolt
 * It is moving (or has moved) from (x,y) to (nx,ny).
 * If the distance is not "one", we (may) return "*".
 */
static char bolt_char(int y, int x, int ny, int nx)
{
    if ((ny == y) && (nx == x)) return '*';
    if (ny == y) return '-';
    if (nx == x) return '|';
    if ((ny-y) == (x-nx)) return '/';
    if ((ny-y) == (nx-x)) return '\\';
    return '*';
}



/*
 * Generic "beam"/"bolt"/"ball" projection routine.  -BEN-
 *
 * Input:
 *   who: Index of "source" monster (or "zero" for "player")
 *   rad: Radius of explosion (0 = beam/bolt, 1 to 9 = ball)
 *   y,x: Target location (or location to travel "towards")
 *   dam: Base damage roll to apply to affected monsters (or player)
 *   typ: Type of damage to apply to monsters (and objects)
 *   flg: Extra bit flags (see PROJECT_xxxx in "defines.h")
 *
 * Return:
 *   TRUE if any "effects" of the projection were observed, else FALSE
 *
 * Allows a monster (or player) to project a beam/bolt/ball of a given kind
 * towards a given location (optionally passing over the heads of interposing
 * monsters), and have it do a given amount of damage to the monsters (and
 * optionally objects) within the given radius of the final location.
 *
 * A "bolt" travels from source to target and affects only the target grid.
 * A "beam" travels from source to target, affecting all grids passed through.
 * A "ball" travels from source to the target, exploding at the target, and
 *   affecting everything within the given radius of the target location.
 *
 * Traditionally, a "bolt" does not affect anything on the ground, and does
 * not pass over the heads of interposing monsters, much like a traditional
 * missile, and will "stop" abruptly at the "target" even if no monster is
 * positioned there, while a "ball", on the other hand, passes over the heads
 * of monsters between the source and target, and affects everything except
 * the source monster which lies within the final radius, while a "beam"
 * affects every monster between the source and target, except for the casting
 * monster (or player), and only affects things on the ground in special cases
 * (light, disarm, walls).
 *
 * The player will only get "experience" for monsters killed by himself
 * Unique monsters can only be destroyed by attacks from the player
 *
 * Only 256 grids can be affected per projection, limiting the effective
 * "radius" of standard ball attacks to nine units (diameter nineteen).
 *
 * One can project in a given "direction" by combining PROJECT_THRU with small
 * offsets to the initial location (see "line_spell()"), or by calculating
 * "virtual targets" far away from the player.
 *
 * One can also use PROJECT_THRU to send a beam/bolt along an angled path,
 * continuing until it actually hits somethings (useful for "stone to mud").
 *
 * Bolts and Beams explode INSIDE walls, so that they can destroy doors.
 *
 * Balls must explode BEFORE hitting walls, or they would affect monsters
 * on both sides of a wall.  Some bug reports indicate that this is still
 * happening in 2.7.8, though it appears to be impossible.
 *
 * We "pre-calculate" the blast area only in part for efficiency.
 * More importantly, this lets us do "explosions" from the "inside" out.
 * This results in a more logical distribution of "blast" treasure.
 * It also produces a better (in my opinion) animation of the explosion.
 * It could be (but is not) used to have the treasure dropped by monsters
 * in the middle of the explosion fall "outwards", and then be damaged by
 * the blast as it spreads outwards towards the treasure drop location.
 *
 * Walls and doors are included in the blast area, so that they can be
 * "burned" or "melted" in later versions.  Permanent rock is NEVER included
 * in the blast area, nor are undefined locations.  XXX XXX XXX
 *
 * This algorithm is intended to maximize simplicity, not necessarily
 * efficiency, since this function is not a bottleneck in the code.
 *
 * Objects in the blast area when the blast occurs may be destroyed,
 * even if they are "under" monsters.  But objects dropped by monsters
 * who are destroyed by the blast are "shielded" by the monsters dead body.
 *
 * Note that the damage done by "ball" explosions decreases with distance.
 * This decrease is rapid, grids at radius "dist" take "1/dist" damage.
 *
 * Notice the "napalm" effect of "beam" weapons.  First they "project" to
 * the target, and then the damage "flows" along this beam of destruction.
 * The damage at every grid is the same as at the "center" of a "ball"
 * explosion, since the "beam" grids are treated as if they ARE at the
 * center of a "ball" explosion.
 *
 * The array "gy[],gx[]" with current size "grids" is used to hold the
 * collected locations of all grids in the "blast area" plus "beam path".
 *
 * Note the rather complex usage of the "gm[]" array.  First, gm[0] is always
 * zero.  Second, for N>1, gm[N] is always the index (in gy[],gx[]) of the
 * first blast grid (see above) with radius "N" from the blast center.  Note
 * that only the first gm[1] grids in the blast area thus take full damage.
 * Also, note that gm[rad+1] is always equal to "grids", which is the total
 * number of blast grids.
 *
 * Note that once the projection is complete, (y2,x2) holds the final location
 * of bolts/beams, and the "epicenter" of balls.
 *
 * Note also that "rad" specifies the "inclusive" radius of projection blast,
 * so that a "rad" of "one" actually covers 5 or 9 grids, depending on the
 * implementation of the "distance" function.  Also, a bolt can be properly
 * viewed as a "ball" with a "rad" of "zero".
 *
 * Currently, specifying "beam" plus "ball" means that locations which are
 * covered by the initial "beam", and also covered by the final "ball", except
 * for the final grid (the epicenter of the ball), will be "hit twice", once
 * by the initial beam, and once by the exploding ball.  For the grid right
 * next to the epicenter, this results in 150% damage being done.  The center
 * does not have this problem, for the same reason the final grid in a "beam"
 * plus "bolt" does not -- it is explicitly removed.  Simply removing "beam"
 * grids which are covered by the "ball" will NOT work, as then they will
 * receive LESS damage than they should.  Do not combine "beam" with "ball".
 *
 * Note that if no "target" is reached before the beam/bolt/ball travels the
 * maximum distance allowed (MAX_RANGE), no "blast" will be induced.  This
 * may be relevant even for bolts, since they have a "1x1" mini-blast.
 *
 * It is rather important that the grids are processed from ground-zero out.
 * For example, this is used by the "GF_LITE" / "GF_DARK" ball weapons to do
 * "correct" room darkening.
 *
 * Note that for consistency, we "pretend" that the bolt actually takes "time"
 * to move from point A to point B, even if the player cannot see part of the
 * projection path.  Note that in general, the player will *always* see part
 * of the path, since it either starts at the player or ends on the player.
 *
 * Hack -- we assume that every "projection" is "self-illuminating".
 */
bool project(int who, int rad, int y, int x, int dam, int typ, int flg)
{
    int			i, t;
    int                 y1, x1, y2, x2;
    int			y0, x0, y9, x9;
    int			dist;

    /* Affected location(s) */
    cave_type *c_ptr;

    /* Assume the player sees nothing */
    bool notice = FALSE;

    /* Assume the player has seen nothing */
    bool visual = FALSE;

    /* Assume the player has seen no blast grids */
    bool drawn = FALSE;

    /* Is the player blind? */
    bool blind = (p_ptr->blind ? TRUE : FALSE);

    /* Number of grids in the "blast area" (including the "beam" path) */
    int grids = 0;

    /* Notice "affected" monsters */
    int m_cnt, m_y, m_x;

    /* Coordinates of the affected grids */
    byte gx[256], gy[256];

    /* Encoded "radius" info (see above) */
    byte gm[16];


    /* The source is a monster */
    if (who) {
        x1 = m_list[who].fx;
        y1 = m_list[who].fy;
    }

    /* Hack -- The source is a player */
    else {
        x1 = px;
        y1 = py;
    }


    /* Location of player */
    y0 = py;
    x0 = px;


    /* Hack -- Assume there will be no blast (max radius 16) */
    for (dist = 0; dist < 16; dist++) gm[dist] = 0;


    /* Default "destination" */
    y2 = y; x2 = x;

    /* XXX XXX Apply "offset" mode */


    /* Hack -- Handle stuff */
    handle_stuff();


    /* Start at the source */
    x = x9 = x1;
    y = y9 = y1;
    dist = 0;

    /* Project until done */
    while (1) {

        /* Gather beam grids */
        if (flg & PROJECT_BEAM) {
            gy[grids] = y;
            gx[grids] = x;
            grids++;
        }

        /* XXX XXX Hack -- Display "beam" grids */
        if (!blind && !(flg & PROJECT_HIDE) &&
            dist && (flg & PROJECT_BEAM) &&
            panel_contains(y, x) && player_has_los_bold(y, x)) {

            /* Hack -- Visual effect -- "explode" the grids */
            print_rel('*', spell_color(typ), y, x);
        }

        /* Check the grid */
        c_ptr = &cave[y][x];

        /* Never pass through walls */
        if (dist && !floor_grid_bold(y, x)) break;

        /* Check for arrival at "final target" (if desired) */
        if (!(flg & PROJECT_THRU) && (x == x2) && (y == y2)) break;

        /* If allowed, and we have moved at all, stop when we hit anybody */
        if (c_ptr->m_idx && dist && (flg & PROJECT_STOP)) break;


        /* Calculate the new location */
        y9 = y;
        x9 = x;
        mmove2(&y9, &x9, y1, x1, y2, x2);

        /* Hack -- Balls explode BEFORE reaching walls or doors */
        if (!floor_grid_bold(y9, x9) && (rad > 0)) break;

        /* Keep track of the distance traveled */
        dist++;

        /* Nothing can travel furthur than the maximal distance */
        if (dist > MAX_RANGE) break;

        /* Only do visual effects (and delay) if requested */
        if (!blind && !(flg & PROJECT_HIDE)) {

            /* Only do visuals if the player can "see" the bolt */
            if (player_has_los_bold(y9, x9) && panel_contains(y9, x9)) {

                /* Visual effects -- Display, Highlight, Flush, Pause, Erase */
                print_rel(bolt_char(y, x, y9, x9), spell_color(typ), y9, x9);
                move_cursor_relative(y9, x9);
                Term_fresh();
                visual = TRUE;
                delay(10 * delay_spd);
                lite_spot(y9, x9);
                Term_fresh();
            }

            /* Hack -- make sure to delay anyway for consistency */
            else {

                /* Delay (sometimes) for consistency */
                if (visual) delay(10 * delay_spd);
            }
        }


        /* Save the new location */
        y = y9;
        x = x9;
    }


    /* Save the "blast epicenter" */
    y2 = y;
    x2 = x;

    /* Start the "explosion" */
    gm[0] = 0;

    /* Hack -- make sure beams get to "explode" */
    gm[1] = grids;

    /* If we found a "target", explode there */
    if (dist <= MAX_RANGE) {

        /* Hack -- remove the final "beam" grid */
        if ((flg & PROJECT_BEAM) && (grids > 0)) grids--;

        /* Determine the blast area, work from the inside out */
        for (dist = 0; dist <= rad; dist++) {

            /* Scan the maximal blast area of radius "dist" */
            for (y = y2 - dist; y <= y2 + dist; y++) {
                for (x = x2 - dist; x <= x2 + dist; x++) {

                    /* Note that we DO add perma-rock to the blast */
                    if (!in_bounds2(y, x)) continue;

                    /* Enforce a "circular" explosion */
                    if (distance(y2, x2, y, x) != dist) continue;

                    /* Ball explosions are stopped by walls */
                    if (!los(y2, x2, y, x)) continue;

                    /* Save this grid */
                    gy[grids] = y;
                    gx[grids] = x;
                    grids++;
                }
            }

            /* Encode some more "radius" info */
            gm[dist+1] = grids;
        }
    }


    /* Speed -- ignore "non-explosions" */
    if (!grids) return (FALSE);


    /* Display the "blast area" */
    if (!blind && !(flg & PROJECT_HIDE)) {

        /* Then do the "blast", from inside out */
        for (t = 0; t <= rad; t++) {

            /* Dump everything with this radius */
            for (i = gm[t]; i < gm[t+1]; i++) {

                /* Extract the location */
                y = gy[i];
                x = gx[i];

                /* The player can see it */
                if (player_has_los_bold(y, x) &&
                    panel_contains(y, x)) {
                    drawn = TRUE;
                    print_rel('*', spell_color(typ), y, x);
                }
            }

            /* Hack -- center the cursor */
            move_cursor_relative(y2, x2);

            /* Flush each "radius" seperately */
            Term_fresh();

            /* Delay (efficiently) */
            if (visual || drawn) delay(10 * delay_spd);
        }

        /* Flush the erasing */
        if (drawn) {

            /* Erase the explosion drawn above */
            for (i = 0; i < grids; i++) {

                /* Extract the location */
                y = gy[i];
                x = gx[i];

                /* Erase if needed */
                if (player_has_los_bold(y, x) &&
                    panel_contains(y, x)) {
                    lite_spot(y, x);
                }
            }

            /* Hack -- center the cursor */
            move_cursor_relative(y2, x2);

            /* Flush the explosion */
            Term_fresh();
        }
    }


    /* Start with "dist" of zero */
    dist = 0;

    /* Now hurt the cave grids (and objects) from the inside out */
    for (i = 0; i < grids; i++) {

        /* Hack -- Notice new "dist" values */
        if (gm[dist+1] == i) dist++;

        /* Get the grid location */
        y = gy[i];
        x = gx[i];

        /* Allow the beam/ball to "damage" the grid itself */
        if (project_i(who, dist, y, x, dam, typ, flg)) notice = TRUE;
    }


    /* Start with "dist" of zero */
    dist = 0;

    /* Hack -- Monster tracking */
    m_y = m_x = m_cnt = 0;

    /* Now hurt the monsters, from inside out */
    for (i = 0; i < grids; i++) {

        /* Hack -- Notice new "dist" values */
        if (gm[dist+1] == i) dist++;

        /* Get the grid location */
        y = gy[i];
        x = gx[i];

        /* Walls protect monsters */
        if (!floor_grid_bold(y,x)) continue;

        /* Get the cave grid */
        c_ptr = &cave[y][x];

        /* Affect real monsters (excluding the caster) */
        if (c_ptr->m_idx && (!who || (c_ptr->m_idx != who))) {

            /* Damage the monster */
            if (project_m(who, dist, y, x, dam, typ, flg)) notice = TRUE;

            /* Track affected monsters */
            m_cnt++;
            m_y = y;
            m_x = x;
        }
    }

    /* Hack -- notice when player affects a single monster */
    if (!who && (m_cnt == 1) && (cave[m_y][m_x].m_idx)) {

        monster_type *m_ptr = &m_list[cave[m_y][m_x].m_idx];

        /* Hack -- auto-recall */
        if (m_ptr->ml) recent_track(m_ptr->r_idx);

        /* Hack - auto-track */
        if (m_ptr->ml) health_track(cave[m_y][m_x].m_idx);
    }


    /* Start with "dist" of zero */
    dist = 0;

    /* Now see if the player gets hurt */
    for (i = 0; i < grids; i++) {

        /* Hack -- Notice new "dist" values */
        if (gm[dist+1] == i) dist++;

        /* Get the grid location */
        y = gy[i];
        x = gx[i];

        /* Hack -- Walls protect the player (never happens) */
        if (!floor_grid_bold(y,x)) continue;

        /* Get the cave grid */
        c_ptr = &cave[y][x];

        /* The player is hurt by monsters */
        if ((y == py) && (x == px) && who) {

            /* Damage the player */
            if (project_p(who, dist, y, x, dam, typ, flg)) notice = TRUE;
        }
    }


    /* Return "something was noticed" */
    return (notice);
}





/*** XXX XXX Old function hooks XXX XXX ***/


/*
 * Hack -- apply a hacked "projection()" to all viewable monsters
 */
static bool project_hack(int typ, int dam)
{
    int		i;
    bool	obvious = FALSE;

    /* Affect all (nearby) monsters */
    for (i = 1; i < m_max; i++) {

        monster_type *m_ptr = &m_list[i];

        int y = m_ptr->fy;
        int x = m_ptr->fx;

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Require line of sight */
        if (!player_has_los_bold(y, x)) continue;

        /* Hack -- apply the effect to the monster in that grid */
        if (project_m(1, 0, y, x, dam, typ, 0)) obvious = TRUE;
    }

    /* Result */
    return (obvious);
}


/*
 * Hack -- apply a "projection()" in a direction (or at the target)
 */
static bool project_hook(int typ, int dir, int dam, int flg)
{
    int tx, ty;

    /* Pass through the target if needed */
    flg |= (PROJECT_THRU);

    /* Use the given direction */
    tx = px + ddx[dir];
    ty = py + ddy[dir];

    /* Use an actual "target" */
    if ((dir == 5) && target_okay()) {
        tx = target_col;
        ty = target_row;
    }

    /* Analyze the "dir" and the "target", do NOT explode */
    return (project(0, 0, ty, tx, dam, typ, flg));
}

/*
 * Cast a bolt spell
 */
bool fire_bolt(int typ, int dir, int dam)
{
    int flg = PROJECT_STOP;
    return (project_hook(typ, dir, dam, flg));
}

/*
 * Cast a beam spell
 */
bool fire_beam(int typ, int dir, int dam)
{
    /* Go until we have to stop, do "beam" damage to everyone */
    /* Also, affect all grids (NOT objects) we pass through */
    int flg = PROJECT_BEAM | PROJECT_GRID;
    return (project_hook(typ, dir, dam, flg));
}

/*
 * Cast a ball spell
 */
bool fire_ball(int typ, int dir, int dam, int rad)
{
    int tx, ty;

    int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_STOP;

    /* Use the given direction */
    tx = px + 99 * ddx[dir];
    ty = py + 99 * ddy[dir];

    /* Use an actual "target" */
    if ((dir == 5) && target_okay()) {
        flg &= ~PROJECT_STOP;
        tx = target_col;
        ty = target_row;
    }

    /* Analyze the "dir" and the "target".  Hurt items on floor. */
    return (project(0, rad, ty, tx, dam, typ, flg));
}

/*
 * Cast a bolt spell, or rarely, a beam spell
 */
bool fire_bolt_or_beam(int prob, int typ, int dir, int dam)
{
    if (rand_int(100) < prob) {
        return (fire_beam(typ, dir, dam));
    }
    else {
        return (fire_bolt(typ, dir, dam));
    }
}


/*
 * Some of the old functions
 */

bool lite_line(int dir)
{
    return (fire_beam(GF_LITE_WEAK, dir, damroll(6, 8)));
}

bool drain_life(int dir, int dam)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_DRAIN, dir, dam, flg));
}

bool wall_to_mud(int dir)
{
    int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM;
    return (project_hook(GF_KILL_WALL, dir, 20 + randint(30), flg));
}

bool destroy_door(int dir)
{
    int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM;
    return (project_hook(GF_KILL_DOOR, dir, 0, flg));
}

bool disarm_trap(int dir)
{
    int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM;
    return (project_hook(GF_KILL_TRAP, dir, 0, flg));
}

bool heal_monster(int dir)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_HEAL, dir, damroll(4, 6), flg));
}

bool speed_monster(int dir)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_SPEED, dir, p_ptr->lev, flg));
}

bool slow_monster(int dir)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_SLOW, dir, p_ptr->lev, flg));
}

bool sleep_monster(int dir)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_SLEEP, dir, p_ptr->lev, flg));
}

bool confuse_monster(int dir, int plev)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_CONF, dir, plev, flg));
}

bool fear_monster(int dir, int plev)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_SCARE, dir, plev, flg));
}

bool poly_monster(int dir)
{
    int flg = PROJECT_BEAM;
    return (project_hook(GF_OLD_POLY, dir, p_ptr->lev, flg));
}

bool clone_monster(int dir)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_CLONE, dir, 0, flg));
}

bool teleport_monster(int dir)
{
    int flg = PROJECT_BEAM;
    return (project_hook(GF_OLD_TPORT, dir, MAX_SIGHT * 5, flg));
}



/*
 * Hooks -- affect adjacent grids (radius 1 ball attack)
 */

bool door_creation()
{
    int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
    return (project(0, 1, py, px, 0, GF_MAKE_DOOR, flg));
}

bool trap_creation()
{
    int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
    return (project(0, 1, py, px, 0, GF_MAKE_TRAP, flg));
}

bool destroy_doors_touch()
{
    int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
    return (project(0, 1, py, px, 0, GF_KILL_DOOR, flg));
}

bool sleep_monsters_touch(void)
{
    int flg = PROJECT_HIDE;
    return (project(0, 1, py, px, p_ptr->lev, GF_OLD_SLEEP, flg));
}



/*
 * Hooks -- affect all nearby monsters
 */

bool speed_monsters(void)
{
    return (project_hack(GF_OLD_SPEED, p_ptr->lev));
}

bool slow_monsters(void)
{
    return (project_hack(GF_OLD_SLOW, p_ptr->lev));
}

bool sleep_monsters(void)
{
    return (project_hack(GF_OLD_SLEEP, p_ptr->lev));
}




/*
 * Hack -- call light around the player
 */
bool lite_area(int dam, int rad)
{
    /* Hack -- Message */
    if (!p_ptr->blind) {
        msg_print("You are surrounded by a white light.");
    }

    /* Hook into the "project()" function */
    (void)project(0, rad, py, px, dam, GF_LITE_WEAK, PROJECT_GRID);

    /* Lite up the room */
    lite_room(py, px);

    /* Assume seen */
    return (TRUE);
}


/*
 * Hack -- call darkness around the player
 */
bool unlite_area(int dam, int rad)
{
    /* Hack -- Message */
    if (!p_ptr->blind) {
        msg_print("Darkness surrounds you.");
    }

    /* Hook into the "project()" function */
    (void)project(0, rad, py, px, dam, GF_DARK_WEAK, PROJECT_GRID);

    /* Lite up the room */
    unlite_room(py, px);

    /* Assume seen */
    return (TRUE);
}


