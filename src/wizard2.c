/* File: wizard2.c */

/* Purpose: Wizard commands */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"



#ifdef ALLOW_WIZARD





/*
 * Hack -- quick debugging hook
 */
static void do_cmd_wiz_hack_ben(void)
{
    /* Oops */
    msg_print("Oops.");
}


/*
 * Same as "askfor_aux()" but with a position and explicit default.
 */
static int get_string_default(char *buf, cptr dflt, int row, int col, int len)
{
    /* Go to the cursor location */
    Term_gotoxy(col, row);

    /* Start with the default */
    strcpy(buf, dflt);

    /* Get some input (or the default) */
    return (askfor_aux(buf, len));
}


/*
 * Same as "askfor()" but with an explicit default response.
 */
static int askfor_default(char *buf, cptr dflt, int len)
{
    /* Start with the default */
    strcpy(buf, dflt);

    /* Get some input (or the default) */
    return (askfor_aux(buf, len));
}



/*
 * Output a long int in binary format.
 */
static void prt_binary(u32b flags, int row, int col)
{
    int        	i;
    u32b        bitmask;

    /* Scan the flags */
    for (i = bitmask = 1; i <= 32; i++, bitmask *= 2) {

        /* Dump set bits */
        if (flags & bitmask) {
            Term_putch(col++, row, TERM_BLUE, '*');
        }

        /* Dump unset bits */
        else {
            Term_putch(col++, row, TERM_WHITE, '-');
        }
    }
}


/*
 * Hack -- Teleport to the target
 */
static void do_cmd_wiz_bamf(void)
{
    /* Must have a target */
    if (!target_who) return;

    /* Teleport to the target */
    teleport_player_to(target_row, target_col);
}



/*
 * Aux function for "do_cmd_wiz_change()".	-RAK-
 */
static void do_cmd_wiz_change_aux(void)
{
    int			tmp_int;

    long		tmp_long;

    char		tmp_val[160];


    prt("(3 - 118) Strength     = ", 0, 0);
    sprintf(tmp_val, "%d", p_ptr->stat_max[A_STR]);
    if (!askfor_aux(tmp_val, 3)) return;

    tmp_int = atoi(tmp_val);
    if ((tmp_int >= 3) && (tmp_int <= 18 + 100)) {
        p_ptr->stat_cur[A_STR] = p_ptr->stat_max[A_STR] = tmp_int;
    }

    prt("(3 - 118) Intelligence = ", 0, 0);
    sprintf(tmp_val, "%d", p_ptr->stat_max[A_INT]);
    if (!askfor_aux(tmp_val, 3)) return;

    tmp_int = atoi(tmp_val);
    if ((tmp_int >= 3) && (tmp_int <= 18 + 100)) {
        p_ptr->stat_cur[A_INT] = p_ptr->stat_max[A_INT] = tmp_int;
    }

    prt("(3 - 118) Wisdom       = ", 0, 0);
    sprintf(tmp_val, "%d", p_ptr->stat_max[A_WIS]);
    if (!askfor_aux(tmp_val, 3)) return;

    tmp_int = atoi(tmp_val);
    if ((tmp_int >= 3) && (tmp_int <= 18 + 100)) {
        p_ptr->stat_cur[A_WIS] = p_ptr->stat_max[A_WIS] = tmp_int;
    }

    prt("(3 - 118) Dexterity    = ", 0, 0);
    sprintf(tmp_val, "%d", p_ptr->stat_max[A_DEX]);
    if (!askfor_aux(tmp_val, 3)) return;

    tmp_int = atoi(tmp_val);
    if ((tmp_int >= 3) && (tmp_int <= 18 + 100)) {
        p_ptr->stat_cur[A_DEX] = p_ptr->stat_max[A_DEX] = tmp_int;
    }

    prt("(3 - 118) Constitution = ", 0, 0);
    sprintf(tmp_val, "%d", p_ptr->stat_max[A_CON]);
    if (!askfor_aux(tmp_val, 3)) return;

    tmp_int = atoi(tmp_val);
    if ((tmp_int >= 3) && (tmp_int <= 18 + 100)) {
        p_ptr->stat_cur[A_CON] = p_ptr->stat_max[A_CON] = tmp_int;
    }

    prt("(3 - 118) Charisma     = ", 0, 0);
    sprintf(tmp_val, "%d", p_ptr->stat_max[A_CHR]);
    if (!askfor_aux(tmp_val, 3)) return;

    tmp_int = atoi(tmp_val);
    if ((tmp_int >= 3) && (tmp_int <= 18 + 100)) {
        p_ptr->stat_cur[A_CHR] = p_ptr->stat_max[A_CHR] = tmp_int;
    }


    prt("Gold = ", 0, 0);
    sprintf(tmp_val, "%ld", p_ptr->au);
    if (!askfor_aux(tmp_val, 9)) return;

    tmp_long = atol(tmp_val);
    if (*tmp_val && (tmp_long >= 0)) {
        p_ptr->au = tmp_long;
    }


    prt("Experience = ", 0, 0);
    sprintf(tmp_val, "%ld", p_ptr->max_exp);
    if (!askfor_aux(tmp_val, 8)) return;

    tmp_long = atol(tmp_val);
    if (*tmp_val && (tmp_long >= 0)) {
        p_ptr->max_exp = tmp_long;
        check_experience();
    }
}


/*
 * Change various "permanent" player variables.
 */
static void do_cmd_wiz_change()
{
    /* Interact */
    do_cmd_wiz_change_aux();

    /* Redraw everything */
    do_cmd_redraw();
}


/*
 * Wizard routines for creating objects		-RAK-	
 * And for manipulating them!                   -Bernd-
 *
 * This has been rewritten to make the whole procedure
 * of debugging objects much easier and more comfortable.
 *
 * The following functions are meant to play with objects:
 * Create, modify, roll for them (for statistic purposes) and more.
 * The original functions were by RAK.
 * The function to show an item's debug information was written
 * by David Reeve Sward <sward+@CMU.EDU>.
 *                             Bernd (wiebelt@mathematik.hu-berlin.de)
 *
 * Here are the low-level functions
 * - wiz_display_item()
 *     display an item's debug-info
 * - wiz_create_itemtype()
 *     specify tval and sval (type and subtype of object)
 * - wiz_tweak_item()
 *     specify pval, +AC, +tohit, +todam
 *     Note that the wizard can leave this function anytime,
 *     thus accepting the default-values for the remaining values.
 *     pval comes first now, since it is most important.
 * - wiz_reroll_item()
 *     apply some magic to the item or turn it into an artifact.
 * - wiz_roll_item()
 *     Get some statistics about the rarity of an item:
 *     We create a lot of fake items and see if they are of the
 *     same type (tval and sval), then we compare pval and +AC.
 *     If the fake-item is better or equal it is counted.
 *     Note that cursed items that are better or equal (absolute values)
 *     are counted, too.
 *     HINT: This is *very* useful for balancing the game!
 * - wiz_quantity_item()
 *     change the quantity of an item, but be sane about it.
 *
 * And now the high-level functions
 * - do_cmd_wiz_play()
 *     play with an existing object
 * - wiz_create_item()
 *     create a new object
 *
 * Note -- You do not have to specify "pval" and other item-properties
 * directly. Just apply magic until you are satisfied with the item.
 *
 * Note -- For some items (such as wands, staffs, some rings, etc), you
 * must apply magic, or you will get "broken" or "uncharged" objects.
 *
 * Note -- Redefining artifacts via "do_cmd_wiz_play()" may destroy
 * the artifact.  Be careful.
 *
 * Hack -- this function will allow you to create multiple artifacts.
 * This "feature" may induce crashes or other nasty effects.
 */

/*
 * Just display an item's properties (debug-info)
 * Originally by David Reeve Sward <sward+@CMU.EDU>
 * Verbose item flags by -Bernd-
 */
static void wiz_display_item(inven_type *i_ptr)
{
    int 	i, j = 13;

    u32b	f1, f2, f3;
    
    char        buf[256];


    /* Extract the flags */
    inven_flags(i_ptr, &f1, &f2, &f3);
    
    /* Clear the screen */
    for (i = 1; i <= 23; i++) prt("", i, j - 2);

    /* Describe fully */
    objdes_store(buf, i_ptr, TRUE, 3);

    prt(buf, 2, j);

    prt(format("kind = %-5d  level = %-4d  tval = %-5d  sval = %-5d",
                i_ptr->k_idx, k_info[i_ptr->k_idx].level,
                i_ptr->tval, i_ptr->sval), 4, j);

    prt(format("number = %-3d  wgt = %-6d  ac = %-5d    damage = %dd%d",
                i_ptr->number, i_ptr->weight,
                i_ptr->ac, i_ptr->dd, i_ptr->ds), 5, j);

    prt(format("pval = %-5d  toac = %-5d  tohit = %-4d  todam = %-4d",
                i_ptr->pval, i_ptr->to_a, i_ptr->to_h, i_ptr->to_d), 6, j);

    prt(format("name1 = %-4d  name2 = %-4d  cost = %ld",
                i_ptr->name1, i_ptr->name2, (long)item_value(i_ptr)), 7, j);

    prt(format("ident = %04x  timeout = %-d",
                i_ptr->ident, i_ptr->timeout), 8, j);

    prt("+------------FLAGS1------------+", 10, j);
    prt("AFFECT..........SLAY......BRAND.", 11, j);
    prt("                ae      x q aefc", 12, j);
    prt("siwdcc  ssidsa  nvudotgdd u clio", 13, j);
    prt("tnieoh  trnipt  iinmrrnrr a ierl", 14, j);
    prt("rtsxna..lcfgdk..mldncltgg.k.dced", 15, j);
    prt_binary(f1, 16, j);

    prt("+------------FLAGS2------------+", 17, j);
    prt("SUST....IMMUN.RESIST............", 18, j);
    prt("        aefcp psaefcp ldbc sn   ", 19, j);
    prt("siwdcc  clioo atclioo ialoshtncd", 20, j);
    prt("tnieoh  ierli raierli trnnnrhehi", 21, j);
    prt("rtsxna..dceds.atdceds.ekdfddrxss", 22, j);
    prt_binary(f2, 23, j);

    prt("+------------FLAGS3------------+", 10, j+32);
    prt("        ehsi  st    iiiiadta  hp", 11, j+32);
    prt("        aihnf ee    ggggcregb vr", 12, j+32);
    prt("        sdose eld   nnnntalrl ym", 13, j+32);
    prt("        yewta ieirmsrrrriieaeccc", 14, j+32);
    prt("        ktmatlnpgeihaefcvnpvsuuu", 15, j+32);
    prt("        nyoahivaeggoclioaeoasrrr", 16, j+32);
    prt("        opdretitsehtierltxrtesss", 17, j+32);
    prt("        westreshtntsdcedeptedeee", 18, j+32);
    prt_binary(f3, 19, j+32);
}


/*
 * A structure to hold a tval and its description
 */
typedef struct tval_desc {
    int        tval;
    cptr       desc;
} tval_desc;

/*
 * A list of tvals and their textual names
 */
static tval_desc tvals[] = {
    { TV_SWORD,             "Sword"                },
    { TV_POLEARM,           "Polearm"              },
    { TV_HAFTED,            "Hafted Weapon"        },
    { TV_BOW,               "Bow"                  },
    { TV_ARROW,             "Arrows"               },
    { TV_BOLT,              "Bolts"                },
    { TV_SHOT,              "Shots"                },
    { TV_SHIELD,            "Shield"               },
    { TV_CROWN,             "Crown"                },
    { TV_HELM,              "Helm"                 },
    { TV_GLOVES,            "Gloves"               },
    { TV_BOOTS,             "Boots"                },
    { TV_CLOAK,             "Cloak"                },
    { TV_DRAG_ARMOR,        "Dragon Scale Mail"    },
    { TV_HARD_ARMOR,        "Hard Armor"           },
    { TV_SOFT_ARMOR,        "Soft Armor"           },
    { TV_RING,              "Ring"                 },
    { TV_AMULET,            "Amulet"               },
    { TV_LITE,              "Lite"                 },
    { TV_POTION,            "Potion"               },
    { TV_SCROLL,            "Scroll"               },
    { TV_WAND,              "Wand"                 },
    { TV_STAFF,             "Staff"                },
    { TV_ROD,               "Rod"                  },
    { TV_PRAYER_BOOK,       "Priest Book"          },
    { TV_MAGIC_BOOK,        "Magic Book"           },
    { TV_SPIKE,             "Spikes"               },
    { TV_DIGGING,           "Digger"               },
    { TV_CHEST,             "Chest"                },
    { TV_FOOD,              "Food"                 },
    { TV_FLASK,             "Flask"                },
    { TV_NOTHING,           NULL                   }
};


/*
 * Strip an "object name" into a buffer
 */
static void strip_name(char *buf, int k_idx)
{
    char *t;

    inven_kind *k_ptr = &k_info[k_idx];

    cptr str = (k_name + k_ptr->name);


    /* Skip past leading characters */
    while ((*str == ' ') || (*str == '&')) str++;

    /* Copy useful chars */
    for (t = buf; *str; str++) {
        if (*str != '~') *t++ = *str;
    }

    /* Terminate the new name */
    *t = '\0';
}


/*
 * Hack -- title for each column
 *
 * XXX XXX XXX This will not work with "EBCDIC", I would think.
 */
static char head[3] = { 'a', 'A', '0' };


/*
 * Specify tval and sval (type and subtype of object) originally
 * by RAK, heavily modified by -Bernd-
 *
 * This function returns the k_idx of an object type, or zero if failed
 *
 * List up to 50 choices in three columns
 */
static int wiz_create_itemtype(void)
{
    int                  i, num, max_num;
    int                  col, row;
    int			 tval;

    cptr                 tval_desc;
    char                 ch;

    int			 choice[60];

    char		buf[160];


    /* Clear the screen */
    clear_screen();

    /* Print all tval's and their descriptions */
    for (num = 0; (num < 60) && tvals[num].tval; num++) {
        row = 2 + (num % 20);
        col = 30 * (num / 20);
        ch = head[num/20] + (num%20);
        prt(format("[%c] %s", ch, tvals[num].desc), row, col);
    }

    /* Me need to know the maximal possible tval_index */
    max_num = num;

    /* Choose! */
    if (!get_com("Get what type of object? ", &ch)) return (0);

    /* Analyze choice */
    num = -1;
    if ((ch >= head[0]) && (ch < head[0] + 20)) num = ch - head[0];
    if ((ch >= head[1]) && (ch < head[1] + 20)) num = ch - head[1] + 20;
    if ((ch >= head[2]) && (ch < head[2] + 10)) num = ch - head[2] + 40;

    /* Bail out if choice is illegal */
    if ((num < 0) || (num >= max_num)) return (0);

    /* Base object type chosen, fill in tval */
    tval = tvals[num].tval;
    tval_desc = tvals[num].desc;


    /*** And now we go for k_idx ***/

    /* Clear the screen */
    clear_screen();

    /* We have to search the whole itemlist. */
    for (num = 0, i = 1; (num < 60) && (i < MAX_K_IDX); i++) {

        inven_kind *k_ptr = &k_info[i];

        /* Analyze matching items */
        if (k_ptr->tval == tval) {

            /* Hack -- Skip instant artifacts */
            if (k_ptr->flags3 & TR3_INSTA_ART) continue;

            /* Prepare it */
            row = 2 + (num % 20);
            col = 30 * (num / 20);
            ch = head[num/20] + (num%20);

            /* Acquire the "name" of object "i" */
            strip_name(buf, i);

            /* Print it */
            prt(format("[%c] %s", ch, buf), row, col);

            /* Remember the object index */
            choice[num++] = i;
        }
    }

    /* Me need to know the maximal possible remembered object_index */
    max_num = num;

    /* Choose! */
    if (!get_com(format("What Kind of %s? ", tval_desc), &ch)) return (0);

    /* Analyze choice */
    num = -1;
    if ((ch >= head[0]) && (ch < head[0] + 20)) num = ch - head[0];
    if ((ch >= head[1]) && (ch < head[1] + 20)) num = ch - head[1] + 20;
    if ((ch >= head[2]) && (ch < head[2] + 10)) num = ch - head[2] + 40;

    /* Bail out if choice is "illegal" */
    if ((num < 0) || (num >= max_num)) return (0);

    /* And return successful */
    return (choice[num]);
}


/*
 * Specify pval, flags and some other things.
 * Choices are shown in order of importance.
 * pval goes first, for example.
 *
 * originally by RAK
 *
 * ...not much remains. - Bernd -
 *
 */
static void wiz_tweak_item(inven_type *i_ptr)
{
    char                tmp_val[80];


    /* Hack -- leave artifacts alone */
    if (artifact_p(i_ptr)) return;

    prt("Change 'pval' setting: ", 0, 0);
    if (!askfor_default(tmp_val, format("%d", i_ptr->pval), 5)) return;
    i_ptr->pval = atoi(tmp_val);
    wiz_display_item(i_ptr);

    prt("Change AC modifier: ", 0, 0);
    if (!askfor_default(tmp_val, format("%d", i_ptr->to_a), 5)) return;
    i_ptr->to_a = atoi(tmp_val);
    wiz_display_item(i_ptr);

    prt("New to-hit modifier: ", 0, 0);
    if (!askfor_default(tmp_val, format("%d", i_ptr->to_h), 3)) return;
    i_ptr->to_h = atoi(tmp_val);
    wiz_display_item(i_ptr);

    prt("New to-dam modifier: ", 0, 0);
    if (!askfor_default(tmp_val, format("%d", i_ptr->to_h), 3)) return;
    i_ptr->to_d = atoi(tmp_val);
    wiz_display_item(i_ptr);
}


/*
 * Apply magic to an item or turn it into an artifact. -Bernd-
 */
static void wiz_reroll_item(inven_type *i_ptr)
{
    inven_type  mod_item;
    char        ch;

    bool	changed = FALSE;


    /* Hack -- leave artifacts alone */
    if (artifact_p(i_ptr)) return;


    /* Copy the item to be modified. */
    mod_item = *i_ptr;

    /* Main loop. Ask for magification and artifactification */
    while (TRUE) {

        /* Display full item debug information */
        wiz_display_item(&mod_item);

        /* Ask wizard what to do. */
        if (!get_com("[a]ccept, [n]ormal, [g]ood, [e]xcellent? ", &ch)) {
            changed = FALSE;
            break;
        }

        /* Create/change it! */
        if (ch == 'A' || ch == 'a') {
            changed = TRUE;
            break;
        }

        /* Apply normal magic, but first clear object */
        else if (ch == 'n' || ch == 'N') {
             invcopy(&mod_item, i_ptr->k_idx);
             apply_magic(&mod_item, dun_level, FALSE, FALSE, FALSE);
        }

        /* Apply good magic, but first clear object */
        else if (ch == 'g' || ch == 'g') {
             invcopy(&mod_item, i_ptr->k_idx);
             apply_magic(&mod_item, dun_level, FALSE, TRUE, FALSE);
        }

        /* Apply great magic, but first clear object */
        else if (ch == 'e' || ch == 'e') {
             invcopy(&mod_item, i_ptr->k_idx);
             apply_magic(&mod_item, dun_level, FALSE, TRUE, TRUE);
        }
    }

    /* Notice change */
    if (changed) {

        /* Apply changes */
        *i_ptr = mod_item;

        /* Redraw the choice window */
        p_ptr->redraw |= (PR_CHOOSE);

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);

        /* Combine / Reorder the pack */
        p_ptr->update |= (PU_COMBINE | PU_REORDER);
    }
}



/*
 * Maximum number of rolls
 */
#define TEST_ROLL 100000


/*
 * Try to create an item again. Output some statistics.    -Bernd-
 *
 * The statistics are correct now.  We acquire a clean grid, and then
 * repeatedly place an object in this grid, copying it into an item
 * holder, and then deleting the object.  We fiddle with the artifact
 * counter flags to prevent weirdness.  We use the items to collect
 * statistics on item creation relative to the initial item.
 */	
static void wiz_statistics(inven_type *i_ptr)
{
    long        i, matches, better, worse, other;
    int         x1, y1;
    char        ch;
    char        *quality;
    bool        good, great;

    inven_type  test_item;
    inven_type	*j_ptr = &test_item;
    
    cptr q = "Rolls: %ld, Matches: %ld, Better: %ld, Worse: %ld, Other: %ld";

    /* Hack -- find a clean grid */
    while (TRUE) {

        /* Pick a location */
        y1 = rand_int(cur_hgt);
        x1 = rand_int(cur_wid);

        /* Accept "naked" grids */
        if (naked_grid_bold(y1, x1)) break;
    }


    /* XXX XXX XXX Mega-Hack -- allow multiple artifacts */
    if (artifact_p(i_ptr)) a_info[i_ptr->name1].cur_num = 0;


    /* Interact */
    while (TRUE) {

        cptr pmt = "Roll for [n]ormal, [g]ood, or [e]xcellent treasure? ";
        
        /* Display item */
        wiz_display_item(i_ptr);

        /* Get choices */
        if (!get_com(pmt, &ch)) break;

        if (ch == 'n' || ch == 'N') {
            good = FALSE;
            great = FALSE;
            quality = "normal";
        }
        else if (ch == 'g' || ch == 'G') {
            good = TRUE;
            great = FALSE;
            quality = "good";
        }
        else if (ch == 'e' || ch == 'E') {
            good = TRUE;
            great = TRUE;
            quality = "excellent";
        }
        else {
            good = FALSE;
            great = FALSE;
            break;
        }

        /* Let us know what we are doing */
        msg_format("Creating a lot of %s items. Base level = %d.",
                   quality, dun_level);
        msg_print(NULL);

        /* Set counters to zero */
        matches = better = worse = other = 0;

        /* Let's rock and roll */
        for (i = 0; i <= TEST_ROLL; i++) {


            /* Output every few rolls */
            if ((i < 100) || (i % 100 == 0)) {
            
                /* Do not wait */
                inkey_scan = TRUE;

                /* Allow interupt */
                if (inkey()) {
 
                    /* Flush */
                    flush();

                    /* Stop rolling */
                    break;
                }

                /* Dump the stats */
                prt(format(q, i, matches, better, worse, other), 0, 0);
                Term_fresh();
            }


            /* Create an item at determined position */
            place_object(y1, x1, good, great);

            /* Copy its contents to test_item */
            (*j_ptr) = i_list[cave[y1][x1].i_idx];

            /* Delete it */
            delete_object(y1, x1);

            /* XXX XXX XXX Mega-Hack -- allow multiple artifacts */
            if (artifact_p(j_ptr)) a_info[j_ptr->name1].cur_num = 0;


            /* Test for the same tval and sval. */
            if ((i_ptr->tval) != (j_ptr->tval)) continue;
            if ((i_ptr->sval) != (j_ptr->sval)) continue;

            /* Check for match */
            if ((j_ptr->pval == i_ptr->pval) &&
                (j_ptr->to_a == i_ptr->to_a) &&
                (j_ptr->to_h == i_ptr->to_h) &&
                (j_ptr->to_d == i_ptr->to_d)) {
                matches++;
            }

            /* Check for better */
            else if ((j_ptr->pval >= i_ptr->pval) &&
                    (j_ptr->to_a >= i_ptr->to_a) &&
                    (j_ptr->to_h >= i_ptr->to_h) &&
                    (j_ptr->to_d >= i_ptr->to_d)) {
                better++;
            }

            /* Check for worse */
            else if ((j_ptr->pval <= i_ptr->pval) &&
                    (j_ptr->to_a <= i_ptr->to_a) &&
                    (j_ptr->to_h <= i_ptr->to_h) &&
                    (j_ptr->to_d <= i_ptr->to_d)) {
                worse++;
            }

            /* Assume different */
            else {
                other++;
            }
        }

        /* Final dump */
        msg_format(q, i, matches, better, worse, other);
        msg_print(NULL);
    }


    /* Hack -- Normally only make a single artifact */
    if (artifact_p(i_ptr)) a_info[i_ptr->name1].cur_num = 1;
}


/*
 * Change the quantity of a the item
 */
static void wiz_quantity_item(inven_type *i_ptr)
{
    int         tmp_int;

    char        tmp_val[100];


    /* Never duplicate artifacts */
    if (artifact_p(i_ptr)) return;

    /* Ask for a value */
    prt("Number of items ", 0, 0);
    askfor_default(tmp_val, format("%d", i_ptr->number), 3);
    tmp_int = atoi(tmp_val);

    /* Paranoia -- require legal quantity */
    if (tmp_int < 1) tmp_int = 1;
    if (tmp_int > 99) tmp_int = 99;

    /* Accept modifications */
    i_ptr->number = tmp_int;
}



/*
 * Play with an item. Options include:
 *   - Output statistics (via wiz_roll_item)
 *   - Reroll item (via wiz_reroll_item)
 *   - Change properties (via wiz_tweak_item)
 *   - Change the number of items (via wiz_quantity_item)
 */
static void do_cmd_wiz_play(void)
{
    int      	item;

    inven_type 	*i_ptr;

    inven_type	forge;

    char 	ch;

    bool 	changed;


    /* Get an item (from equip or inven) */
    if (!get_item(&item, "Play with which object? ", TRUE, TRUE, TRUE)) {
        if (item == -2) msg_print("You have nothing to play with.");
        return;
    }

    /* Get the item (in the pack) */
    if (item >= 0) {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else {
        i_ptr = &i_list[0 - item];
    }


    /* The item was not changed */
    changed = FALSE;


    /* Icky */
    character_icky = TRUE;
    
    /* Save the screen */
    Term_save();


    /* Get a copy of the item */
    forge = (*i_ptr);

    /* The main loop */
    while (TRUE) {

        /* Display the item */
        wiz_display_item(&forge);

        /* Get choice */
        if (!get_com("[a]ccept [s]tatistics [r]eroll [t]weak [q]uantity? ", &ch)) {
            changed = FALSE;
            break;
        }

        if (ch == 'A' || ch == 'a') {
            changed = TRUE;
            break;
        }

        if (ch == 's' || ch == 'S') {
            wiz_statistics(&forge);
        }

        if (ch == 'r' || ch == 'r') {
            wiz_reroll_item(&forge);
        }

        if (ch == 't' || ch == 'T') {
            wiz_tweak_item(&forge);
        }

        if (ch == 'q' || ch == 'Q') {
            wiz_quantity_item(&forge);
        }
    }


    /* Restore the screen */
    Term_load();

    /* Not Icky */
    character_icky = FALSE;
    
    
    /* Accept change */
    if (changed) {

        /* Message */
        msg_print("Changes accepted.");

        /* Change */
        (*i_ptr) = forge;

        /* Redraw the choice window */
        p_ptr->redraw |= (PR_CHOOSE);

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);
        
        /* Combine / Reorder the pack */
        p_ptr->update |= (PU_COMBINE | PU_REORDER);
    }

    /* Ignore change */
    else {
        msg_print("Changes ignored.");
    }
}


/*
 * Wizard routine for creating objects		-RAK-	
 * Heavily modified to allow magification and artifactification  -Bernd-
 *
 * Note that wizards cannot create objects on top of other objects.
 *
 * Hack -- this routine always makes a "dungeon object", and applies
 * magic to it, and attempts to decline cursed items.
 */
static void wiz_create_item()
{
    inven_type	forge;

    int         k_idx;


    /* Icky */
    character_icky = TRUE;

    /* Save the screen */    
    Term_save();

    /* Get object base type */
    k_idx = wiz_create_itemtype();

    /* Restore the screen */    
    Term_load();

    /* Not Icky */
    character_icky = FALSE;
    

    /* Return if failed */
    if (!k_idx) return;

    /* Create the item */
    invcopy(&forge, k_idx);

    /* Apply magic (no messages, no artifacts) */
    wizard = FALSE;
    apply_magic(&forge, dun_level, FALSE, FALSE, FALSE);
    wizard = TRUE;

    /* Drop the object from heaven */
    drop_near(&forge, -1, py, px);

    /* All done */
    msg_print("Allocated.");
}


/*
 * Cure everything instantly
 */
static void do_cmd_wiz_cure_all()
{
    /* Remove curses */
    (void)remove_all_curse();

    /* Restore stats */
    (void)res_stat(A_STR);
    (void)res_stat(A_INT);
    (void)res_stat(A_WIS);
    (void)res_stat(A_CON);
    (void)res_stat(A_DEX);
    (void)res_stat(A_CHR);

    /* Restore the level */
    (void)restore_level();

    /* Heal the player */
    p_ptr->chp = p_ptr->mhp;
    p_ptr->chp_frac = 0;

    /* Restore mana */
    p_ptr->csp = p_ptr->msp;
    p_ptr->csp_frac = 0;

    /* Cure stuff */
    (void)set_blind(0);
    (void)set_confused(0);
    (void)set_poisoned(0);
    (void)set_afraid(0);
    (void)set_paralyzed(0);
    (void)set_image(0);
    (void)set_stun(0);
    (void)set_cut(0);
    (void)set_slow(0);

    /* No longer hungry */
    (void)set_food(PY_FOOD_MAX - 1);

    /* Redraw everything */
    do_cmd_redraw();
}


/*
 * Go to any level
 */
static void do_cmd_wiz_jump(int level)
{
    int		i;

    char	tmp_val[160];


    if (level > 0) {
        i = level;
    }
    else {
        prt("Go to which level (0-500)? ", 0, 0);
        i = (-1);
        if (get_string_default(tmp_val, format("%d", dun_level), 0, 27, 10)) {
            i = atoi(tmp_val);
        }
    }

    if (i > MAX_DEPTH - 1) i = MAX_DEPTH - 1;

    if (i >= 0) {
        msg_print("You jump...");
        dun_level = (i < 500) ? i : 500;
        new_level_flag = TRUE;
    }

    prt("", 0, 0);
}


/*
 * Identify a lot of objects
 */

static void do_cmd_wiz_learn()
{
    int			i, m;

    char		tmp_val[160];


    /* Prompt */
    prt("Identify objects up to level (0-200): ", 0, 0);

    /* Identify all the objects */
    if (askfor(tmp_val, 10)) {

        /* Extract a max level */
        m = atoi(tmp_val);

        /* Scan every object */
        for (i = 1; i < MAX_K_IDX; i++) {

            inven_kind *k_ptr = &k_info[i];

            if (k_ptr->level <= m) {
                inven_type inv;
                invcopy(&inv, i);
                inven_aware(&inv);
            }
        }
    }

    /* Clear the prompt */
    prt("", 0, 0);
}


/*
 * Hack -- Rerate Hitpoints
 */
static void do_cmd_rerate()
{
    int         min_value, max_value, i, percent;

    min_value = (PY_MAX_LEVEL * 3 * (p_ptr->hitdie - 1)) / 8 +
        PY_MAX_LEVEL;
    max_value = (PY_MAX_LEVEL * 5 * (p_ptr->hitdie - 1)) / 8 +
        PY_MAX_LEVEL;
    player_hp[0] = p_ptr->hitdie;
    do {
        for (i = 1; i < PY_MAX_LEVEL; i++) {
            player_hp[i] = randint(p_ptr->hitdie);
            player_hp[i] += player_hp[i - 1];
        }
    }
    while ((player_hp[PY_MAX_LEVEL - 1] < min_value) ||
           (player_hp[PY_MAX_LEVEL - 1] > max_value));

    percent = (int)(((long)player_hp[PY_MAX_LEVEL - 1] * 200L) /
                (p_ptr->hitdie + ((PY_MAX_LEVEL - 1) * p_ptr->hitdie)));

    /* Update and redraw hitpoints */
    p_ptr->update |= (PU_HP);
    p_ptr->redraw |= (PR_HP);

    /* Handle stuff */
    handle_stuff();

    /* Message */
    msg_format("Current Life Rating is %d/100.", percent);
}


/*
 * Summon some creatures
 */
static void do_cmd_wiz_summon(int num)
{
    int i;
    
    for (i = 0; i < num; i++) {

        (void)summon_specific(py, px, dun_level, 0);
    }
}


/*
 * Summon a creature of the specified type
 *
 * XXX XXX XXX This function is rather dangerous
 */
static void do_cmd_wiz_named(int r_idx, int slp)
{
    int i, x, y;

    /* Paranoia */
    /* if (!r_idx) return; */

    /* Prevent illegal monsters */
    if (r_idx >= MAX_R_IDX-1) return;

    /* Try 10 times */
    for (i = 0; i < 10; i++) {

        int d = 1;

        /* Pick a location */
        scatter(&y, &x, py, px, d, 0);

        /* Require empty grids */
        if (!empty_grid_bold(y, x)) continue;

        /* Place it (allow groups) */
        if (place_monster_aux(y, x, r_idx, slp, TRUE)) break;
    }
}



/*
 * Hack -- Delete all nearby monsters
 */
static void do_cmd_wiz_zap(void)
{
    int        i;

    /* Genocide everyone nearby */
    for (i = 1; i < m_max; i++) {

        monster_type *m_ptr = &m_list[i];

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Delete nearby monsters */
        if (m_ptr->cdis <= MAX_SIGHT) delete_monster_idx(i);
    }
}



#ifdef ALLOW_SPOILERS

/*
 * External function
 */
extern void do_cmd_spoilers(void);

#endif



/*
 * Hack -- declare external function
 */
extern void do_cmd_wizard(void);



/*
 * Ask for and parse a "wizard command"
 * The "command_arg" may have been set.
 * We return "FALSE" on unknown commands.
 */
void do_cmd_wizard(void)
{
    char		cmd;


    /* Get a "wizard command" */
    (void)(get_com("Wizard Command: ", &cmd));

    /* Analyze the command */
    switch (cmd) {

        /* Nothing */
        case ESCAPE:
        case ' ':
        case '\n':
        case '\r':
            break;


#ifdef ALLOW_SPOILERS

        /* Hack -- Generate Spoilers */
        case '"':
            do_cmd_spoilers();
            break;

#endif


        /* Hack -- Help */
        case '?':
            do_cmd_help("help.hlp");
            break;


        /* Cure all maladies */
        case 'a':
            do_cmd_wiz_cure_all();
            break;

        /* Teleport to target */
        case 'b':
            do_cmd_wiz_bamf();
            break;
            
        /* Create any object */
        case 'c':
            wiz_create_item();
            break;

        /* Detect everything */
        case 'd':
            detection();
            break;

        /* Edit character */
        case 'e':
            do_cmd_wiz_change();
            break;

        /* View item info */
        case 'f':
            (void)identify_fully();
            break;

        /* Good Objects */
        case 'g':
            if (command_arg <= 0) command_arg = 1;
            acquirement(py, px, command_arg, FALSE);
            break;

        /* Hitpoint rerating */
        case 'h':
            do_cmd_rerate(); break;

        /* Identify */
        case 'i':
            (void)ident_spell();
            break;

        /* Go up or down in the dungeon */
        case 'j':
            do_cmd_wiz_jump(command_arg);
            break;

        /* Self-Knowledge */
        case 'k':
            self_knowledge();
            break;

        /* Learn about objects */
        case 'l':
            do_cmd_wiz_learn();
            break;

        /* Magic Mapping */
        case 'm':
            map_area();
            break;

        /* Summon Named Monster */
        case 'n':
            do_cmd_wiz_named(command_arg, TRUE);
            break;

        /* Object playing routines */
        case 'o':
            do_cmd_wiz_play();
            break;

        /* Phase Door */
        case 'p':
            teleport_player(10);
            break;

        /* Summon Random Monster(s) */
        case 's':
            if (command_arg <= 0) command_arg = 1;
            do_cmd_wiz_summon(command_arg);
            break;

        /* Teleport */
        case 't':
            teleport_player(100);
            break;

        /* Very Good Objects */	
        case 'v':
            if (command_arg <= 0) command_arg = 1;
            acquirement(py, px, command_arg, TRUE);
            break;

        /* Wizard Light the Level */
        case 'w':
            wiz_lite();
            break;

        /* Increase Experience */
        case 'x':
            if (command_arg) {
                gain_exp(command_arg);
            }
            else {
                gain_exp(p_ptr->exp + 1);
            }
            break;

        /* Zap Monsters (Genocide) */
        case 'z':
            do_cmd_wiz_zap();
            break;

        /* Hack -- whatever I desire */
        case '_':
            do_cmd_wiz_hack_ben();
            break;

        /* Not a Wizard Command */
        default:
            msg_print("That is not a valid wizard command.");
            break;
    }
}


#else

#ifdef MACINTOSH
static int i = 0;
#endif

#endif

