/* File: wizard.c */

/* Purpose: Wizard mode debugging aids. */

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
 * Hack -- whatever I need at the moment	-BEN-
 */
static void hack_ben(int arg)
{
    msg_print("Hi.");
}


/*
 * Note that the following few routines could probably be placed
 * in "io.c" where other files can make use of them.
 */


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
 * Wizard routine for gaining on stats                  -RAK-
 * Now shows previous stats by using askfor_default        -Bernd-
 */
static void change_character()
{
    int			tmp_int;

    long		tmp_long;

    char		tmp_val[160];


    prt("(3 - 118) Strength     = ", 0, 0);
    if (!askfor_default(tmp_val, format("%d", p_ptr->max_stat[A_STR]), 3)) return;

    tmp_int = atoi(tmp_val);
    if ((tmp_int > 2) && (tmp_int < 119)) {
        p_ptr->max_stat[A_STR] = tmp_int;
        (void)res_stat(A_STR);
    }

    prt("(3 - 118) Intelligence = ", 0, 0);
    if (!askfor_default(tmp_val, format("%d", p_ptr->max_stat[A_INT]), 3)) return;

    tmp_int = atoi(tmp_val);
    if ((tmp_int > 2) && (tmp_int < 119)) {
        p_ptr->max_stat[A_INT] = tmp_int;
        (void)res_stat(A_INT);
    }

    prt("(3 - 118) Wisdom       = ", 0, 0);
    if (!askfor_default(tmp_val, format("%d", p_ptr->max_stat[A_WIS]), 3)) return;

    tmp_int = atoi(tmp_val);
    if ((tmp_int > 2) && (tmp_int < 119)) {
        p_ptr->max_stat[A_WIS] = tmp_int;
        (void)res_stat(A_WIS);
    }

    prt("(3 - 118) Dexterity    = ", 0, 0);
    if (!askfor_default(tmp_val, format("%d", p_ptr->max_stat[A_DEX]), 3)) return;

    tmp_int = atoi(tmp_val);
    if ((tmp_int > 2) && (tmp_int < 119)) {
        p_ptr->max_stat[A_DEX] = tmp_int;
        (void)res_stat(A_DEX);
    }

    prt("(3 - 118) Constitution = ", 0, 0);
    if (!askfor_default(tmp_val, format("%d", p_ptr->max_stat[A_CON]), 3)) return;

    tmp_int = atoi(tmp_val);
    if ((tmp_int > 2) && (tmp_int < 119)) {
        p_ptr->max_stat[A_CON] = tmp_int;
        (void)res_stat(A_CON);
    }

    prt("(3 - 118) Charisma     = ", 0, 0);
    if (!askfor_default(tmp_val, format("%d", p_ptr->max_stat[A_CHR]), 3)) return;

    tmp_int = atoi(tmp_val);
    if ((tmp_int > 2) && (tmp_int < 119)) {
        p_ptr->max_stat[A_CHR] = tmp_int;
        (void)res_stat(A_CHR);
    }

    prt("(1 - 32767) Max Hit Points = ", 0, 0);
    if (!askfor_default(tmp_val, format("%d", p_ptr->mhp), 5)) return;

    tmp_int = atoi(tmp_val);
    if (tmp_val[0] && (tmp_int > 0) && (tmp_int <= MAX_SHORT)) {
        p_ptr->mhp = tmp_int;
        p_ptr->chp = tmp_int;
        p_ptr->chp_frac = 0;
        p_ptr->redraw |= (PR_HP);
    }

    prt("(0 - 32767) Max Mana = ", 0, 0);
    if (!askfor_default(tmp_val, format("%d", p_ptr->msp) ,5)) return;

    tmp_int = atoi(tmp_val);
    if (tmp_val[0] && (tmp_int >= 0) && (tmp_int <= MAX_SHORT)) {
        p_ptr->msp = tmp_int;
        p_ptr->csp = tmp_int;
        p_ptr->csp_frac = 0;
        p_ptr->redraw |= (PR_MANA);
    }

    prt("Gold = ", 0, 0);
    if (!askfor_default(tmp_val, format("%ld", p_ptr->au), 7)) return;

    tmp_long = atol(tmp_val);
    if (tmp_val[0] && (tmp_long >= 0)) {
        p_ptr->au = tmp_long;
        p_ptr->redraw |= (PR_GOLD);
    }

    prt("Experience = ", 0, 0);
    if (!askfor_default(tmp_val, format("%ld", p_ptr->max_exp), 7)) return;

    tmp_long = atol(tmp_val);
    if (tmp_long > -1 && (*tmp_val != '\0')) {
        p_ptr->max_exp = tmp_long;
        check_experience();
    }

    (void)sprintf(tmp_val, "Current=%d  Weight = ", p_ptr->wt);
    prt("Weight = ", 0, 0);
    if (!askfor_default(tmp_val, format("%d", p_ptr->wt), 3)) return;

    tmp_int = atoi(tmp_val);
    if (tmp_int > -1 && (*tmp_val != '\0')) {
        p_ptr->wt = tmp_int;
    }
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
 * - wiz_play_item()
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
 * Note -- Redefining artifacts via "wiz_play_item()" may destroy
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
    char        buf[256];

    /* Clear the screen */
    for (i = 1; i <= 23; i++) prt("", i, j - 2);

    /* Describe fully */
    objdes_store(buf, i_ptr, TRUE, 3);

    prt(buf, 2, j);

    prt(format("kind = %-5d  level = %-4d  tval = %-5d  sval = %-5d",
                i_ptr->k_idx, k_list[i_ptr->k_idx].level,
                i_ptr->tval, i_ptr->sval), 4, j);

    prt(format("number = %-3d  wgt = %-6d  ac = %-5d    damage = %dd%d",
                i_ptr->number, i_ptr->weight,
                i_ptr->ac, i_ptr->dd, i_ptr->ds), 5, j);

    prt(format("pval = %-5d  toac = %-5d  tohit = %-4d  todam = %-4d",
                i_ptr->pval, i_ptr->toac, i_ptr->tohit, i_ptr->todam), 6, j);

    prt(format("name1 = %-4d  name2 = %-4d  cost = %ld",
                i_ptr->name1, i_ptr->name2, (long)item_value(i_ptr)), 7, j);

    prt(format("ident = %04x  timeout = %-d",
                i_ptr->ident, i_ptr->timeout), 8, j);

    /* Wearable items */
    if (wearable_p(i_ptr)) {

        prt("+------------FLAGS1------------+", 10, j);
        prt("AFFECT..........SLAY......BRAND.", 11, j);
        prt("                ae      x q aefc", 12, j);
        prt("siwdcc  ssidsa  nvudotgdd u clio", 13, j);
        prt("tnieoh  trnipt  iinmrrnrr a ierl", 14, j);
        prt("rtsxna..lcfgdk..mldncltgg.k.dced", 15, j);
        prt_binary(i_ptr->flags1, 16, j);

        prt("+------------FLAGS2------------+", 17, j);
        prt("SUST....IMMUN.RESIST............", 18, j);
        prt("        aefcp psaefcp ldbc sn   ", 19, j);
        prt("siwdcc  clioo atclioo ialoshtncd", 20, j);
        prt("tnieoh  ierli raierli trnnnrhehi", 21, j);
        prt("rtsxna..dceds.atdceds.ekdfddrxss", 22, j);
        prt_binary(i_ptr->flags2, 23, j);

        prt("+------------FLAGS3------------+", 10, j+32);
        prt("        ehsi  st    iiiiadta  hp", 11, j+32);
        prt("        aihnf ee    ggggcregb vr", 12, j+32);
        prt("        sdose eld   nnnntalrl ym", 13, j+32);
        prt("        yewta ieirmsrrrriieaeccc", 14, j+32);
        prt("        ktmatlnpgeihaefcvnpvsuuu", 15, j+32);
        prt("        nyoahivaeggoclioaeoasrrr", 16, j+32);
        prt("        opdretitsehtierltxrtesss", 17, j+32);
        prt("        westreshtntsdcedeptedeee", 18, j+32);
        prt_binary(i_ptr->flags3, 19, j+32);
    }

    /* Other items */
    else {

        prt("+-----------FLAGS1-------------+", 10, j);
        prt_binary(i_ptr->flags1, 11, j);

        prt("+-----------FLAGS2-------------+", 13, j);
        prt_binary(i_ptr->flags2, 14, j);

        prt("+-----------FLAGS3-------------+", 16, j);
        prt_binary(i_ptr->flags3, 17, j);
    }
}


/*
 * A structure to hold a tval and its description
 */
typedef struct _tval_desc {
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
static void strip_name(char *buf, cptr str)
{
    char *t;
    
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

    int			 option[60];

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
    for (i = num = 0; (num < 60) && (i < MAX_K_IDX); i++) {

        /* Analyze matching items */
        if (k_list[i].tval == tval) {

            /* Hack -- Skip instant artifacts */
            if (k_list[i].flags3 & TR3_INSTA_ART) continue;

            /* Prepare it */
            row = 2 + (num % 20);
            col = 30 * (num / 20);
            ch = head[num/20] + (num%20);
            strip_name(buf, k_list[i].name);
            
            /* Print it */            
            prt(format("[%c] %s", ch, buf), row, col);
            
            /* Remember the object index */
            option[num++] = i;
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
    return (option[num]);
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
    if (!askfor_default(tmp_val, format("%d", i_ptr->toac), 5)) return;
    i_ptr->toac = atoi(tmp_val);
    wiz_display_item(i_ptr);

    prt("New to-hit modifier: ", 0, 0);
    if (!askfor_default(tmp_val, format("%d", i_ptr->tohit), 3)) return;
    i_ptr->tohit = atoi(tmp_val);
    wiz_display_item(i_ptr);

    prt("New to-dam modifier: ", 0, 0);
    if (!askfor_default(tmp_val, format("%d", i_ptr->tohit), 3)) return;
    i_ptr->todam = atoi(tmp_val);
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

    /* Enter "icky" mode */
    character_icky = TRUE;

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
        *i_ptr = mod_item;
        p_ptr->update |= PU_BONUS;
        p_ptr->redraw |= PR_CHOICE;
    }
    
    /* Leave "icky" mode */
    character_icky = FALSE;
}



/*
 * Get a "more" message (on the message line)
 * Then erase the entire message line
 * Leave the cursor on the message line.
 */
static void wiz_more(int x)
{
    /* Print a message */
    c_prt(TERM_L_BLUE, "-more-", 0, x);

    /* Get an acceptable keypress */
    while (1) {
        int cmd = inkey();
        if ((quick_messages) || (cmd == ESCAPE)) break;
        if ((cmd == ' ') || (cmd == '\n') || (cmd == '\r')) break;
        bell();
    }

    /* Clear the line */
    prt("", 0, 0);
}



/*
 * See below (better have a good processor at hand)
 */
#define TEST_ROLL 100000


/*
 * Try to create an item again. Output some statistics.    -Bernd-
 *
 * The statistics are correct now. We try to get a clean grid,
 * call place_object() on this grid, copy the object to test_item,
 * and then delete() it again.  This is done quite a few times.
 */	
static void wiz_statistics(inven_type *i_ptr)
{
    long        i, matches, better, worse, other;
    int         x1, y1;
    char        ch;
    char        *quality;
    bool        good, great;
    inven_type  test_item;


    /* Search a clean grid nearby */
    while (TRUE) {

        /* Pick a location */
        y1 = rand_int(cur_hgt);
        x1 = rand_int(cur_wid);
        
        /* Accept "naked" grids */
        if (naked_grid_bold(y1, x1)) break;
    }


    /* Mega-Hack -- allow multiple artifacts */
    if (artifact_p(i_ptr)) v_list[i_ptr->name1].cur_num = 0;


    /* Interact */
    while (TRUE) {

        /* Display item */
        wiz_display_item(i_ptr);

        /* Get choices */
        get_com("Roll for [n]ormal, [g]ood, or [e]xcellent treasure? ", &ch);

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

        /* Set counters to zero */
        matches = better = worse = other = 0;

        /* Let's rock and roll */
        for (i = 0; i <= TEST_ROLL; i++) {


            /* Do not wait */
            inkey_scan = TRUE;
            
            /* Check for break */
            if (inkey()) {

                /* Flush */
                flush();

                /* Stop rolling */
                break;
            }


            /* Output every few rolls */
            if ((i < 100) || (i % 100 == 0)) {
                prt(format("Rolls: %ld, Matches: %ld, Better: %ld, Worse: %ld, Other: %ld",
                           i, matches, better, worse, other), 0, 0);
                Term_fresh();
            }


            /* Create an item at determined position */
            place_object(y1, x1, good, great);

            /* Copy its contents to test_item */
            test_item = i_list[cave[y1][x1].i_idx];

            /* Delete it */
            delete_object(y1, x1);

            /* Mega-Hack -- allow multiple artifacts */
            if (artifact_p(&test_item)) v_list[test_item.name1].cur_num = 0;


            /* Test for the same tval and sval. */
            if ((i_ptr->tval) != (test_item.tval)) continue;
            if ((i_ptr->sval) != (test_item.sval)) continue;

            /* Check for match */
            if ((test_item.pval == i_ptr->pval) &&
                (test_item.toac == i_ptr->toac) &&
                (test_item.tohit == i_ptr->tohit) &&
                (test_item.todam == i_ptr->todam)) {
                matches++;
            }

            /* Check for better */
            else if ((test_item.pval >= i_ptr->pval) &&
                    (test_item.toac >= i_ptr->toac) &&
                    (test_item.tohit >= i_ptr->tohit) &&
                    (test_item.todam >= i_ptr->todam)) {
                better++;
            }

            /* Check for worse */
            else if ((test_item.pval <= i_ptr->pval) &&
                    (test_item.toac <= i_ptr->toac) &&
                    (test_item.tohit <= i_ptr->tohit) &&
                    (test_item.todam <= i_ptr->todam)) {
                worse++;
            }

            /* Assume different */
            else {
                other++;
            }
        }

        /* One more dump */
        prt(format("Rolls: %ld, Matches: %ld, Better: %ld, Worse: %ld, Other: %ld",
                   i, matches, better, worse, other), 0, 0);
        Term_fresh();

        /* Enough rolling */
        wiz_more(70);
    }


    /* Hack -- Normally only make a single artifact */
    if (artifact_p(i_ptr)) v_list[i_ptr->name1].cur_num = 1;
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
static void wiz_play_item(void)
{
    int      	item_val;

    inven_type 	*i_ptr;

    inven_type	forge;
    
    char 	ch;

    bool 	changed;

    cptr        pmt;


    /* Get an item to play with (no floor) or abort */
    pmt = "Play with which object? ";
    if (!get_item(&item_val, pmt, 0, INVEN_TOTAL-1, FALSE)) {
        if (item_val == -2) msg_print("You have nothing to play with.");
        return;
    }

    /* Get the item (if in inven/equip) */
    i_ptr = &inventory[item_val];

    
    /* The item was not changed */
    changed = FALSE;

    /* Save the screen */
    save_screen();

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
    restore_screen();

    /* Accept change */
    if (changed) {
        msg_print("Changes accepted.");
        (*i_ptr) = forge;
        p_ptr->redraw |= (PR_CHOICE);
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

    /* Get type and subtype (tval and sval) of an object */
    save_screen();
    k_idx = wiz_create_itemtype();
    restore_screen();

    /* Return if failed */
    if (!k_idx) return;

    /* Create the item */
    invcopy(&forge, k_idx);

    /* Apply magic (no messages, no artifacts) */
    wizard = FALSE;
    apply_magic(&forge, dun_level, FALSE, FALSE, FALSE);
    wizard = TRUE;

    /* Drop the object from heaven */
    drop_near(&forge, 0, py, px);

    /* All done */
    msg_print("Allocated.");
}


/*
 * Cure everything -- will not show up until next turn.
 */
static void wizard_cure_all()
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
    
    /* Cure various things */
    (void)cure_blindness();
    (void)cure_confusion();
    (void)cure_poison();
    (void)cure_fear();

    /* No longer hungry */
    p_ptr->food = PY_FOOD_MAX - 1;

    /* No longer slow */
    p_ptr->slow = 0;

    /* Cure hallucination */
    p_ptr->image = 0;

    /* Cure cuts/stun */
    p_ptr->cut = 0;
    p_ptr->stun = 0;

    /* Update everything */
    p_ptr->redraw |= (PR_CAVE);
}


/*
 * Go to any level
 */
static void wizard_goto_level(int level)
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

    if (i > 500) i = 500;

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

static void wizard_identify_many()
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
        for (i = 0; i < MAX_K_IDX; i++) {
            if (k_list[i].level <= m) {
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
            player_hp[i] = randint((int)p_ptr->hitdie);
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
 * Summon a creature of the specified type
 */
static void do_cmd_summon(int r_idx, int slp)
{
    int i, x, y;

    /* Paranoia */
    if (r_idx <= 0) return;
    if (r_idx >= MAX_R_IDX-1) return;

    /* Try 10 times */
    for (i = 0; i < 10; i++) {

        monster_race *r_ptr = &r_list[r_idx];

        int d = 1;

        /* Pick a location */
        scatter(&y, &x, py, px, d, 0);

        /* Require empty grids */
        if (!empty_grid_bold(y, x)) continue;

        /* Place it */
        if (r_ptr->rflags1 & RF1_FRIENDS) {
            place_group(y, x, r_idx, slp);
        }
        else {
            place_monster(y, x, r_idx, slp);
        }

        /* Done */
        break;
    }
}



/*
 * Wizards can genocide anything nearby
 */
static void wizard_genocide(void)
{
    int        i;

    /* Genocide everyone nearby */
    for (i = MIN_M_IDX; i < m_max; i++) {

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
 * Ask for and parse a "wizard command"
 * The "command_arg" may have been set.
 * We return "FALSE" on unknown commands.
 */
int do_wiz_command(void)
{
    char		cmd;


    /* All wizard commands are "free" */
    energy_use = 0;

    /* Get a "wizard command" */
    if (!get_com("Wizard Command: ", &cmd)) cmd = ESCAPE;

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
            do_cmd_spoilers(); break;

#endif


        /* Hack -- Help */
        case '?':
            do_cmd_help("help.hlp"); break;


        /* Cure all maladies */
        case 'a':
            wizard_cure_all(); break;

        /* Create any object */
        case 'c':
            wiz_create_item();
            break;

        /* Detect everything */
        case 'd':
            detection(); break;

        /* Edit character */
        case 'e':
            change_character(); prt("", 0, 0); break;

        /* View item info */
        case 'f':
            (void)identify_fully(); break;

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
            if (ident_spell()) combine_pack();
            break;

        /* Go up or down in the dungeon */
        case 'j':
            wizard_goto_level(command_arg); break;

        /* Self-Knowledge */
        case 'k':
            self_knowledge(); break;

        /* Learn about objects */
        case 'l':
            wizard_identify_many(); break;

        /* Magic Mapping */
        case 'm':
            map_area(); break;

        /* Object playing routines */
        case 'o':
            wiz_play_item(); break;

        /* Phase Door */
        case 'p':
            teleport_flag = TRUE;
            teleport_dist = 10;
            break;

        /* Random Monster */	
        case 'r':
            (void)summon_monster(py, px, dun_level + MON_SUMMON_ADJ);
            break;

        /* Summon */
        case 's':
            do_cmd_summon(command_arg, TRUE); break;

        /* Teleport */
        case 't':
            teleport_flag = TRUE;
            teleport_dist = 100;
            break;

        /* Very Good Objects */	
        case 'v':
            if (command_arg <= 0) command_arg = 1;
            acquirement(py, px, command_arg, TRUE);
            break;

        /* Wizard Light the Level */
        case 'w':
            wiz_lite(); break;

        /* Increase Experience */
        case 'x':
            if (!command_arg) command_arg = p_ptr->exp + 1;
            gain_exp(command_arg);
            break;

        /* Zap Monsters (Genocide) */
        case 'z':
            wizard_genocide(); break;

        /* Hack -- whatever I desire */
        case '_':
            hack_ben(command_arg); break;

        /* Not a Wizard Command */
        default:
            msg_print("That is not a valid wizard command.");
            return (FALSE);
            break;
    }

    /* Success */
    return (TRUE);
}


#endif

