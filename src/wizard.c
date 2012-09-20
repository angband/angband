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



/*
 * Verify desire to be a wizard, and do so if verified
 * This routine should only be called if "can_be_wizard"
 */
int enter_wiz_mode(void)
{
    int answer = FALSE;

    /* Already been asked */
    if (noscore & 0x0002) return (TRUE);

    /* Verify request */
    msg_print("Wizard mode is for debugging and experimenting.");
    msg_print("The game will not be scored if you enter wizard mode.");
    answer = get_check("Are you sure you want to enter wizard mode?");

    /* Never Mind */
    if (!answer) return (FALSE);

    /* Remember old setting */
    noscore |= 0x0002;

    /* Make me a wizard */
    return (TRUE);
}


#ifdef ALLOW_SPOILERS

/*
 * Spoiler file
 */
static FILE *fff = NULL;

/*
 * Determine the "Activation" if any for an item
 * Return a string, or NULL for "no activation"
 */
static cptr spoil_activate(inven_type *i_ptr)
{
    if (i_ptr->name1 == ART_NARTHANC)
        return ("fire bolt (9d8) every 8* turns");
    if (i_ptr->name1 == ART_NIMTHANC)
        return ("frost bolt (6d8) every 7* turns");
    if (i_ptr->name1 == ART_DETHANC)
        return ("lightning bolt (4d8) every 6* turns");
    if (i_ptr->name1 == ART_RILIA)
        return ("stinking cloud (12) every 4* turns");
    if (i_ptr->name1 == ART_BELANGIL)
        return ("frost ball (48) every 5* turns");
    if (i_ptr->name1 == ART_DAL)
        return ("remove fear and cure poison every 5 turns");
    if (i_ptr->name1 == ART_RINGIL)
        return ("ice storm (100) every 300 turns");
    if (i_ptr->name1 == ART_ANDURIL)
        return ("fire ball (72) every 400 turns");
    if (i_ptr->name1 == ART_FIRESTAR)
        return ("large fire ball (72) every 100 turns");
    if (i_ptr->name1 == ART_FEANOR)
        return ("haste self (20* turns) every 200 turns");
    if (i_ptr->name1 == ART_THEODEN)
        return ("drain life (120) every 400 turns");
    if (i_ptr->name1 == ART_TURMIL)
        return ("drain life (90) every 70 turns");
    if (i_ptr->name1 == ART_CASPANION)
        return ("door and trap destruction every 10 turns");
    if (i_ptr->name1 == ART_AVAVIR)
        return ("recall every 200 turns");
    if (i_ptr->name1 == ART_TARATOL)
        return ("haste self (20* turns) every 100* turns");
    if (i_ptr->name1 == ART_ERIRIL)
        return ("identify every 10 turns");
    if (i_ptr->name1 == ART_OLORIN)
        return ("probing every 20 turns");
    if (i_ptr->name1 == ART_EONWE)
        return ("mass genocide every 1000 turns");
    if (i_ptr->name1 == ART_LOTHARANG)
        return ("cure wounds (4d7) every 3* turns");
    if (i_ptr->name1 == ART_CUBRAGOL)
        return ("fire branding of bolts every 999 turns");
    if (i_ptr->name1 == ART_ARUNRUTH)
        return ("frost bolt (12d8) every 500 turns");
    if (i_ptr->name1 == ART_AEGLOS)
        return ("ice storm (100) every 500 turns");
    if (i_ptr->name1 == ART_OROME)
        return ("stone to mud every 5 turns");
    if (i_ptr->name1 == ART_SOULKEEPER)
        return ("heal (1000) every 888 turns");
    if (i_ptr->name1 == ART_BELEGENNON)
        return ("phase door every 2 turns");
    if (i_ptr->name1 == ART_CELEBORN)
        return ("genocide every 500 turns");
    if (i_ptr->name1 == ART_LUTHIEN)
        return ("restore life levels every 450 turns");
    if (i_ptr->name1 == ART_ULMO)
        return ("teleport away every 150 turns");
    if (i_ptr->name1 == ART_COLLUIN)
        return ("resistance (20* turns) every 111 turns");
    if (i_ptr->name1 == ART_HOLCOLLETH)
        return ("Sleep II every 55 turns");
    if (i_ptr->name1 == ART_THINGOL)
        return ("recharge item every 70 turns");
    if (i_ptr->name1 == ART_COLANNON)
        return ("teleport every 45 turns");
    if (i_ptr->name1 == ART_TOTILA)
        return ("confuse monster every 15 turns");
    if (i_ptr->name1 == ART_CAMMITHRIM)
        return ("magic missile (2d6) every 2 turns");
    if (i_ptr->name1 == ART_PAURHACH)
        return ("fire bolt (9d8) every 8* turns");
    if (i_ptr->name1 == ART_PAURNIMMEN)
        return ("frost bolt (6d8) every 7* turns");
    if (i_ptr->name1 == ART_PAURAEGEN)
        return ("lightning bolt (4d8) every 6* turns");
    if (i_ptr->name1 == ART_PAURNEN)
        return ("acid bolt (5d8) every 5* turns");
    if (i_ptr->name1 == ART_FINGOLFIN)
        return ("magical arrow (150) every 90* turns");
    if (i_ptr->name1 == ART_HOLHENNETH)
        return ("detection every 55* turns");
    if (i_ptr->name1 == ART_GONDOR)
        return ("heal (500) every 500 turns");
    if (i_ptr->name1 == ART_RAZORBACK)
        return ("star ball (150) every 1000 turns");
    if (i_ptr->name1 == ART_BLADETURNER)
        return ("resistance (and such) every 400 turns");
    if (i_ptr->name1 == ART_GALADRIEL)
        return ("illumination every 10* turns");
    if (i_ptr->name1 == ART_ELENDIL)
        return ("magic mapping every 50* turns");
    if (i_ptr->name1 == ART_THRAIN)
        return ("clairvoyance every 100* turns");
    if (i_ptr->name1 == ART_INGWE)
        return ("dispel evil (x5) every 300* turns");
    if (i_ptr->name1 == ART_CARLAMMAS)
        return ("protection from evil every 225* turns");
    if (i_ptr->name1 == ART_TULKAS)
        return ("haste self (75* turns) every 150* turns");
    if (i_ptr->name1 == ART_NARYA)
        return ("large fire ball (120) every 225* turns");
    if (i_ptr->name1 == ART_NENYA)
        return ("large frost ball (200) every 325* turns");
    if (i_ptr->name1 == ART_VILYA)
        return ("large lightning ball (250) every 425* turns");
    if (i_ptr->name1 == ART_POWER)
        return ("bizarre stuff every 450* turns");
    return (NULL);
}


/*
 * Create Spoiler files
 */
static void do_cmd_spoilers(void)
{
    msg_print("Sorry, this code is not ready yet...");
}


#endif


#ifdef ALLOW_WIZARD



/*
 * Hack -- whatever I need at the moment	-BEN-
 */
static void hack_ben(int arg)
{
    int y, x;

    /* Hack -- light up artifacts */
    for (y = 0; y < cur_hgt; y++) {
        for (x = 0; x < cur_wid; x++) {
            if (artifact_p(&i_list[cave[y][x].i_idx])) {
                mh_print_rel('*', TERM_RED, 0, y, x);
            }
        }
    }
}


/*
 * Unblock vision (mark monsters as visible)
 */
static void do_cmd_unblock(int dis)
{
    int i;

    /* Process all the monsters */
    for (i = MIN_M_IDX; i < m_max; i++) {

        monster_type *m_ptr = &m_list[i];	

        /* Paranoia -- Skip dead monsters */
        if (m_ptr->dead) continue;

        /* Skip "distant" monsters */	
        if (m_ptr->cdis > dis) continue;

        /* Mark him as visible */
        m_ptr->ml = TRUE;
        lite_spot(m_ptr->fy, m_ptr->fx);
    }
}


/*
 * Note that the following few routines could probably be placed
 * in "io.c" where other files can make use of them.
 */


/*
 * This function is like "get_string()" but accepts "default" answers
 * If the default is NULL, this function just works like "get_string()"
 *
 * Output a default value, and accept it if <RETURN> is the first
 * key pressed. Otherwise get a new response from the user.
 */
static int get_string_default(char *buf, cptr dflt, int row, int col, int len)
{
    int i, k, x1, x2;
    int done;

    /* Paranoia -- check len */
    if (len < 1) len = 1;

    /* Paranoia -- check column */
    if ((col < 0) || (col >= 80)) col = 0;

    /* Find the box bounds */
    x1 = col;
    x2 = x1 + len - 1;
    if (x2 >= 80) {
        len = 80 - x1;
        x2 = 80 - 1;
    }

    /* Erase the "answer box" and place the cursor */
    Term_erase(x1, row, x2, row);

    /* No key was pressed */
    i = 0;

    /* Handle defaults */
    if (dflt) {

        /* Paranoia -- verify default */
        /* if (strlen(dflt) > len)... */

        /* Use the default */
        strcpy(buf, dflt);

        /* Show the default */
        Term_putstr(col, row, -1, TERM_YELLOW, dflt);

        /* Put the cursor at the beginning */
        Term_gotoxy(col, row);

        /* Get the first input key. */
        i = inkey();

        /* Escape was pressed. Return FALSE */
        if (i == ESCAPE) return (FALSE);

        /* Return was pressed. Return successful */
        if (i == '\n' || i == '\r') return (TRUE);
    }

    /* Assume no answer (yet) */
    buf[0] = '\0';

    /* Erase the "answer box" and place the cursor */
    Term_erase(x1, row, x2, row);

    /* Process input */
    for (k = 0, done = 0; !done; ) {

        /* Get keys after the first one */
        if (!i) i = inkey();

        /* Analyze the key */
        switch (i) {

          case ESCAPE:
            buf[0] = '\0';
            Term_erase(x1, row, x2, row);
            return (FALSE);

          case '\n':
          case '\r':
            done = TRUE;
            break;

          case '\010':
          case DELETE:
            if (k > 0) {
                buf[--k] = '\0';
                Term_erase(x1 + k, row, x2, row);
            }
            break;

          default:
            if ((k < len) && (isprint(i))) {
                Term_putch(x1 + k, row, TERM_WHITE, i);
                buf[k++] = i;
                buf[k] = '\0';
            }
            else {
                bell();
            }
            break;
        }

        /* Get the next key */
        i = 0;
    }

    /* Remove trailing blanks */
    while ((k > 0) && (buf[k-1] == ' ')) k--;

    /* Terminate it */
    buf[k] = '\0';

    /* Return the result */
    return (TRUE);
}

/*
 * Same as "askfor()" but takes a default response
 */
static int askfor_default(char *buf, cptr dflt, int len)
{
    int x, y;
    Term_locate(&x, &y);
    return (get_string_default(buf, dflt, y, x, len));
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
    int			tmp_val;

    s32b		tmp_lval;

    char		tmp_str[160];


    prt("(3 - 118) Strength     = ", 0, 0);
    if (!askfor_default(tmp_str, format("%d", p_ptr->max_stat[A_STR]), 3)) return;

    tmp_val = atoi(tmp_str);
    if ((tmp_val > 2) && (tmp_val < 119)) {
        p_ptr->max_stat[A_STR] = tmp_val;
        (void)res_stat(A_STR);
    }

    prt("(3 - 118) Intelligence = ", 0, 0);
    if (!askfor_default(tmp_str, format("%d", p_ptr->max_stat[A_INT]), 3)) return;

    tmp_val = atoi(tmp_str);
    if ((tmp_val > 2) && (tmp_val < 119)) {
        p_ptr->max_stat[A_INT] = tmp_val;
        (void)res_stat(A_INT);
    }

    prt("(3 - 118) Wisdom       = ", 0, 0);
    if (!askfor_default(tmp_str, format("%d", p_ptr->max_stat[A_WIS]), 3)) return;

    tmp_val = atoi(tmp_str);
    if ((tmp_val > 2) && (tmp_val < 119)) {
        p_ptr->max_stat[A_WIS] = tmp_val;
        (void)res_stat(A_WIS);
    }

    prt("(3 - 118) Dexterity    = ", 0, 0);
    if (!askfor_default(tmp_str, format("%d", p_ptr->max_stat[A_DEX]), 3)) return;

    tmp_val = atoi(tmp_str);
    if ((tmp_val > 2) && (tmp_val < 119)) {
        p_ptr->max_stat[A_DEX] = tmp_val;
        (void)res_stat(A_DEX);
    }

    prt("(3 - 118) Constitution = ", 0, 0);
    if (!askfor_default(tmp_str, format("%d", p_ptr->max_stat[A_CON]), 3)) return;

    tmp_val = atoi(tmp_str);
    if ((tmp_val > 2) && (tmp_val < 119)) {
        p_ptr->max_stat[A_CON] = tmp_val;
        (void)res_stat(A_CON);
    }

    prt("(3 - 118) Charisma     = ", 0, 0);
    if (!askfor_default(tmp_str, format("%d", p_ptr->max_stat[A_CHR]), 3)) return;

    tmp_val = atoi(tmp_str);
    if ((tmp_val > 2) && (tmp_val < 119)) {
        p_ptr->max_stat[A_CHR] = tmp_val;
        (void)res_stat(A_CHR);
    }

    prt("(1 - 32767) Max Hit Points = ", 0, 0);
    if (!askfor_default(tmp_str, format("%d", p_ptr->mhp), 5)) return;

    tmp_val = atoi(tmp_str);
    if (tmp_str[0] && (tmp_val > 0) && (tmp_val <= MAX_SHORT)) {
        p_ptr->mhp = tmp_val;
        p_ptr->chp = tmp_val;
        p_ptr->chp_frac = 0;
        p_ptr->redraw |= PR_HP;
    }

    prt("(0 - 32767) Max Mana = ", 0, 0);
    if (!askfor_default(tmp_str, format("%d", p_ptr->mana) ,5)) return;

    tmp_val = atoi(tmp_str);
    if (tmp_str[0] && (tmp_val >= 0) && (tmp_val <= MAX_SHORT)) {
        p_ptr->mana = tmp_val;
        p_ptr->cmana = tmp_val;
        p_ptr->cmana_frac = 0;
        p_ptr->redraw |= PR_MANA;
    }

    prt("Gold = ", 0, 0);
    if (!askfor_default(tmp_str, format("%ld", p_ptr->au), 7)) return;

    tmp_lval = atol(tmp_str);
    if (tmp_str[0] && (tmp_lval >= 0)) {
        p_ptr->au = tmp_lval;
        p_ptr->redraw |= PR_BLOCK;
    }

    prt("Experience = ", 0, 0);
    if (!askfor_default(tmp_str, format("%ld", p_ptr->max_exp), 7)) return;

    tmp_lval = atol(tmp_str);
    if (tmp_lval > -1 && (*tmp_str != '\0')) {
        p_ptr->max_exp = tmp_lval;
        check_experience();
    }

    (void)sprintf(tmp_str, "Current=%d  Weight = ", p_ptr->wt);
    prt("Weight = ", 0, 0);
    if (!askfor_default(tmp_str, format("%d", p_ptr->wt), 3)) return;

    tmp_val = atoi(tmp_str);
    if (tmp_val > -1 && (*tmp_str != '\0')) {
        p_ptr->wt = tmp_val;
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
 * - wiz_modify_item()
 *     specify pval, +AC, +tohit, +todam
 *     Note that the wizard can leave this function anytime,
 *     thus accepting the default-values for the remaining values.
 *     pval comes first now, since it is most important.
 *     This used to be wizard_create_aux2(). Most options are
 *     gone now, since they are obsolete.
 * - wiz_apply_magic()
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
 *     show debug info and eventually play with an item,
 *     that is, enter wiz_apply_magic(), wiz_modify_item(),
 *     wiz_reroll_item() or wiz_quantity_item().
 * - wiz_create_item()
 *     create a new object, apply some dungeon magic
 *     to it or even try to turn it into an artifact.
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

    for (i = 1; i <= 23; i++) prt("", i, j - 2);

    objdes_store(buf, i_ptr, TRUE);
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
                i_ptr->name1, i_ptr->name2, (long)(i_ptr->cost)), 7, j);

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

        /* Do we have a matching tval? */
        if (k_list[i].tval == tval) {

            /* Print it */
            row = 2 + (num % 20);
            col = 30 * (num / 20);
            ch = head[num/20] + (num%20);
            prt(format("[%c] %s", ch, k_list[i].name), row, col);

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
static bool wiz_modify_item(inven_type *i_ptr)
{
    int                 tmp_val;

    char                tmp_str[100];

    bool     	    	changed;

    /*Nothing changed */
    changed = FALSE;

    /* First the pval-value! This is most important */
    prt("Change 'pval' setting: ", 0, 0);
    if (!askfor_default(tmp_str, format("%d", i_ptr->pval), 5)) return(changed);
    tmp_val = atoi(tmp_str);
    changed = (i_ptr->pval != tmp_val);
    i_ptr->pval = tmp_val;
    wiz_display_item(i_ptr);

    prt("Change AC modifier: ", 0, 0);
    if (!askfor_default(tmp_str, format("%d", i_ptr->toac), 5)) return(changed);
    tmp_val = atoi(tmp_str);
    changed = (i_ptr->toac != tmp_val);
    i_ptr->toac = tmp_val;
    wiz_display_item(i_ptr);

    prt("New to-hit modifier: ", 0, 0);
    if (!askfor_default(tmp_str, format("%d", i_ptr->tohit), 3)) return(changed);
    tmp_val = atoi(tmp_str);
    changed = (i_ptr->tohit != tmp_val);
    i_ptr->tohit = tmp_val;
    wiz_display_item(i_ptr);

    prt("New to-dam modifier: ", 0, 0);
    if (!askfor_default(tmp_str, format("%d", i_ptr->tohit), 3)) return(changed);
    tmp_val = atoi(tmp_str);
    changed = (i_ptr->todam != tmp_val);
    i_ptr->todam = tmp_val;
    wiz_display_item(i_ptr);

    return(changed);
}

/*
 * Apply magic to an item or turn it into an artifact. -Bernd-
 */
static bool wiz_apply_magic(inven_type *i_ptr)
{
    int         i;

    inven_type  mod_item;
    char        ch;

    bool        applied;


    /* Nothing applied yet */
    applied = FALSE;

    /* Copy the item to be modified. */
    mod_item = *i_ptr;

    /* Main loop. Ask for magification and artifactification */
    while (TRUE) {

        /* Display full item debug information */
        wiz_display_item(&mod_item);

        /* Mega-Hack -- Allow multiple artifacts (?) */
        if (artifact_p(&mod_item)) v_list[mod_item.name1].cur_num = 0;

        /* Ask wizard what to do. */
        if (!get_com("[c]reate, [g]ood, [e]xcellent, [a]rtifact? ", &ch)) {
            applied = FALSE;
            break;
        }

        /* Create/change it it! */
        if (ch == 'C' || ch == 'c') {
            applied = TRUE;
            *i_ptr = mod_item;
            break;
        }

        /* Apply good magic, but first clear object */
        else if (ch == 'g' || ch == 'g') {
             invcopy(&mod_item, i_ptr->k_idx);
             apply_magic(&mod_item, dun_level, TRUE, TRUE, FALSE);
        }

        /* Apply great magic, but first clear object */
        else if (ch == 'e' || ch == 'e') {
             invcopy(&mod_item, i_ptr->k_idx);
             apply_magic(&mod_item, dun_level, TRUE, TRUE, TRUE);
        }

        /* Try to turn it into artifact. */
        else if (ch == 'a' || ch == 'A') {

            /* Try for some time */
            for (i = 1; i < 1000; i++) {
                if (make_artifact(&mod_item)) break;
            }

            if artifact_p(&mod_item) {
                msg_print(format("Succeded after %d tr%s",
                                 i, (i == 1) ? "y" : "ies"));
            }
            else {
                msg_print(format("Giving up after %d tries!", i));
            }
        }
    }

    /* Result */
    return (applied);
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
 * Try to create the item again. Output some statistics.    -Bernd-
 *
 * The statistics are correct now. We try to get a clean grid,
 * call place_object() or place_good() on this grid, copy the
 * object to test_item, and then delete() it again.
 * This is done quite a few times.
 */	
static void wiz_reroll_item(inven_type *i_ptr)
{
    long        i, matches, better, worse, other;
    int         k, x1, y1;
    char        ch;
    char        *quality;
    bool        good, great;
    inven_type  test_item;


    /* Search a clean grid nearby */
    for (k = 0; k < 50; k++) {

        int d = 20;

        /* Pick a location "near" the player */
        while (1) {
            y1 = rand_spread(py, d);
            x1 = rand_spread(px, d);
            if (in_bounds(y1, x1)) continue;
            if (distance(py, px, y1, x1) > d) continue;
            break;
        }

        /* Must be "clean" floor grid */
        if (naked_grid_bold(y1, x1)) break;
    }

    /* Hack -- Nowhere to use */
    if (k == 50) return;


    /* Mega-Hack -- allow multiple artifacts */
    if (artifact_p(i_ptr)) {
        v_list[i_ptr->name1].cur_num = 0;
    }

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
            message("Ok, no more rolling...", 0);
            break;
        }

        /* Let us know what we are doing */
        msg_print(format("Creating a lot of %s items. Base level=%d ",
                          quality, dun_level));

        /* Set counters to zero */
        matches = better = worse = other = 0;

        /* Let's rock and roll */
        for (i = 0; i <= TEST_ROLL; i++) {

            /* Output every 100 rolls or if a key was pressed */
            if ((i < 100) || (i % 100 == 0)) {
                cptr p;
                p = "Rolls: %ld, Matches: %ld, Better: %ld, Worse: %ld, Other: %ld";
                prt(format(p, i, matches, better, worse, other), 0, 0);
                Term_fresh();
            }

            /* Check for break */
            if (Term_kbhit()) {
                Term_flush();
                break;
            }

            /* Create an item at determined position */
            if (good) place_good(y1, x1, great);
            else place_object(y1, x1);

            /* Copy its contents to test_item */
            test_item = i_list[cave[y1][x1].i_idx];

            /* Delete it */
            delete_object(y1, x1);

            /* Mega-Hack -- allow multiple artifacts */
            if (artifact_p(&test_item)) {
                v_list[test_item.name1].cur_num = 0;
            }

            /* Test for the same tval and sval. */
            if ((i_ptr->tval) != (test_item.tval)) continue;
            if ((i_ptr->sval) != (test_item.sval)) continue;

#if 0
            /* Test for the same flags */
            if (i_ptr->flags1 != test_item.flags1) continue;
            if (i_ptr->flags2 != test_item.flags2) continue;
            if (i_ptr->flags3 != test_item.flags3) continue;
#endif

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

        /* Enough rolling */
        wiz_more(70);
    }

    /* Hack -- Normally only make a single artifact */
    if (artifact_p(i_ptr)) v_list[i_ptr->name1].cur_num = 1;
}

/*
 * Change the quantity of a the item
 */
static bool wiz_quantity_item(inven_type *i_ptr)
{
    int         tmp_val;

    char        tmp_str[100];

    bool        changed = FALSE;


    /* Never duplicate artifacts */
    if (artifact_p(i_ptr)) return (FALSE);

    prt("Number of items ", 0, 0);
    askfor_default(tmp_str, format("%d", i_ptr->number), 3);
    tmp_val = atoi(tmp_str);

    /* Paranoia -- require legal quantity */
    if (tmp_val < 1) tmp_val = 1;
    if (tmp_val > 99) tmp_val = 99;

    /* Any modifications? */
    if (tmp_val != i_ptr->number) {
        changed = TRUE;
        i_ptr->number = tmp_val;
    }

    return (changed);
}

/*
 * Play with an item. Options include:
 * - Apply magic (via wiz_apply_magic)
 * - Redfine properties (via wiz_modify_item)
 * - Output statistics (via wiz_roll_item)
 * - Change the number of items (via wiz_quantity_item)
 * - Verbose description (via identify_fully)
 */
static void wiz_play_item(void)
{
    int      	item_val;
    inven_type 	*i_ptr, item_backup;

    char 	ch;

    bool 	changed;

    cptr        pmt;


    /* Get an item to play with (no floor) or abort */
    pmt = "Play with which object? ";
    if (!get_item(&item_val, pmt, 0, INVEN_TOTAL-1, FALSE)) return;

    /* Get the item (if in inven/equip) */
    i_ptr = &inventory[item_val];

    /* The item was not changed */
    changed = FALSE;

    /* Back up the item */
    item_backup = *i_ptr;

    /* Save the screen */
    save_screen();

    /* The main loop */
    while (TRUE) {

        /* Show the item (everything about it) */
        wiz_display_item(i_ptr);

        /* Get choice */
        pmt = "[c]hange [r]estore [m]agic [t]weak [s]tatistics [q]uantity? ";
        if (!get_com(pmt, &ch)) break;

        if (ch == 'c' || ch == 'c') break;

        if (ch == 'r' || ch == 'R') {
            *i_ptr = item_backup;
            changed = FALSE;
        }

        if (ch == 's' || ch == 'S') wiz_reroll_item(i_ptr);

        if (ch == 'm' || ch == 'M') changed = wiz_apply_magic(i_ptr);
        if (ch == 'q' || ch == 'Q') changed = wiz_quantity_item(i_ptr);
        if (ch == 't' || ch == 'T') changed = wiz_modify_item(i_ptr);
    }

    /* Restore the screen */
    restore_screen();

    /* Message */
    if (changed) {
        message("Item modified.", 0);
    }
    else {
        message("Item unchanged.", 0);
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
    cave_type  	*c_ptr;

    inven_type	forge;

    int         i, k_idx;

    /* Check if player is standing on an object. */
    if (!clean_grid_bold(py, px)) return;

    /* Get type and subtype (tval and sval) of an object */
    save_screen();
    k_idx = wiz_create_itemtype();
    restore_screen();

    /* Return if failed */
    if (!k_idx) return;

    /* Hack -- cancel wizard messages */
    wizard = FALSE;

    /* Try 20 times to get a (non-cursed) object */
    for (i = 0; i < 20; i++) {
        invcopy(&forge, k_idx);
        apply_magic(&forge, dun_level, FALSE, FALSE, FALSE);
        if (!cursed_p(&forge)) break;
    }

    /* OK, we can restore wizard mode */
    wizard = TRUE;

    /* Save the screen */
    save_screen();
    i = wiz_apply_magic(&forge);
    restore_screen();

    /* Catch errors */
    if (!i) {
        message("Not allocated.", 0);
        return;
    }

    /* Hack -- Ask for the number of objects */
    wiz_quantity_item(&forge);

    /* Create the object (finally) */
    c_ptr = &cave[py][px];
    c_ptr->i_idx = i_pop();
    i_list[c_ptr->i_idx] = forge;
    forge.iy = py;
    forge.ix = px;

    /* All done */
    message("Allocated.", 0);
}


/*
 * Cure everything -- will not show up until next turn.
 */
static void wizard_cure_all()
{
    (void)remove_all_curse();
    (void)cure_blindness();
    (void)cure_confusion();
    (void)cure_poison();
    (void)remove_fear();

    (void)res_stat(A_STR);
    (void)res_stat(A_INT);
    (void)res_stat(A_WIS);
    (void)res_stat(A_CON);
    (void)res_stat(A_DEX);
    (void)res_stat(A_CHR);

    (void)restore_level();
    (void)hp_player(2000);

    p_ptr->food = PLAYER_FOOD_MAX;

    /* Hack -- do not QUITE cure things, let dungeon() do it */
    if (p_ptr->slow > 1) p_ptr->slow = 1;
    if (p_ptr->image > 1) p_ptr->image = 1;
    if (p_ptr->cut > 1) p_ptr->cut = 1;
    if (p_ptr->stun > 1) p_ptr->stun = 1;
}


/*
 * Go to any level
 */
static void wizard_goto_level(int level)
{
    int		i;

    char	tmp_str[160];


    if (level > 0) {
        i = level;
    }
    else {
        prt("Go to which level (0-500)? ", 0, 0);
        i = (-1);
        if (get_string_default(tmp_str, format("%d", dun_level), 0, 27, 10)) {
            i = atoi(tmp_str);
        }
    }

    if (i > 500) i = 500;

    if (i >= 0) {
        dun_level = i;
        if (dun_level > 500) dun_level = 500;
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
    
    char		tmp_str[160];


    /* Prompt */
    prt("Identify objects up to level (0-200): ", 0, 0);

    /* Identify all the objects */
    if (askfor(tmp_str, 10)) {

        /* Extract a max level */
        m = atoi(tmp_str);

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
    char        buf[50];

    min_value = (MAX_PLAYER_LEVEL * 3 * (p_ptr->hitdie - 1)) / 8 +
        MAX_PLAYER_LEVEL;
    max_value = (MAX_PLAYER_LEVEL * 5 * (p_ptr->hitdie - 1)) / 8 +
        MAX_PLAYER_LEVEL;
    player_hp[0] = p_ptr->hitdie;
    do {
        for (i = 1; i < MAX_PLAYER_LEVEL; i++) {
            player_hp[i] = randint((int)p_ptr->hitdie);
            player_hp[i] += player_hp[i - 1];
        }
    }
    while ((player_hp[MAX_PLAYER_LEVEL - 1] < min_value) ||
           (player_hp[MAX_PLAYER_LEVEL - 1] > max_value));

    percent = (int)(((long)player_hp[MAX_PLAYER_LEVEL - 1] * 200L) /
                (p_ptr->hitdie + ((MAX_PLAYER_LEVEL - 1) * p_ptr->hitdie)));

    /* Update and redraw hitpoints */
    p_ptr->update |= (PU_HP);
    p_ptr->redraw |= (PR_HP);

    /* Handle stuff */
    handle_stuff(TRUE);
    
    sprintf(buf, "%d%% Life Rating", percent);
    msg_print(buf);
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
        while (1) {
            y = rand_spread(py, d);
            x = rand_spread(px, d);
            if (!in_bounds2(py, px)) continue;
            if (distance(py, px, y, x) > d) continue;
            if (los(py, px, y, x)) break;
        }

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
        if (m_ptr->dead) continue;

        /* Delete nearby monsters */
        if (m_ptr->cdis <= MAX_SIGHT) delete_monster_idx(i);
    }
}



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


#ifdef AUTO_PLAY

        /* Initialize the Borg */
        case '$':
            borg_init();
            break;

#endif


#ifdef ALLOW_SPOILERS

        /* Hack -- Generate Spoilers */
        case '"':
            do_cmd_spoilers(); break;

#endif

#ifdef ALLOW_WIZARD

        /* Hack -- Help */
        case '?':
            do_cmd_help("cmds_w.hlp"); break;


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

        /* Un-block vision */
        case 'u':
            if (command_arg <= 0) command_arg = 1000;
            do_cmd_unblock(command_arg);
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
            p_ptr->exp = p_ptr->exp * 2 + 1;
            if (command_arg) p_ptr->exp = command_arg;
            check_experience();
            break;

        /* Zap Monsters (Genocide) */
        case 'z':
            wizard_genocide(); break;

        /* Hack -- whatever I desire */
        case '_':
            hack_ben(command_arg); break;

#endif

        /* Not a Wizard Command */
        default:
            message ("That is not a valid wizard command.", 0);
            return (FALSE);
            break;
    }

    /* Success */
    return (TRUE);
}

