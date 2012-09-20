/* File: mon-desc.c */

/* Purpose: describe monsters (using monster memory) */

/*
 * Copyright (c) 1989 James E. Wilson, Christopher J. Stuart
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"


#undef CTRL
#define CTRL(C) ((C)&037)



/*
 * Blow method table
 */
static cptr desc_method[] = {
    "???",
    "hit",
    "touch",
    "punch",
    "kick",
    "claw",
    "bite",
    "sting",
    "???",
    "butt",
    "crush",
    "engulf",
    "???",
    "crawl on you",
    "drool on you",
    "spit",
    "???",
    "gaze",
    "wail",
    "release spores",
    "???",
    "beg",
    "insult",
    "moan",
    "???"
};


/*
 * Blow effect table
 */
static cptr desc_effect[] = {
    "???",
    "attack",
    "poison",
    "disenchant",
    "drain charges",
    "steal gold",
    "steal items",
    "eat your food",
    "absorb light",
    "shoot acid",
    "electrify",
    "burn",
    "freeze",
    "blind",
    "confuse",
    "terrify",
    "paralyse",
    "reduce strength",
    "reduce intelligence",
    "reduce wisdom",
    "reduce dexterity",
    "reduce constitution",
    "reduce charisma",
    "reduce all stats",
    "???",
    "lower experience (by 10d6+)",
    "lower experience (by 20d6+)",
    "lower experience (by 40d6+)",
    "lower experience (by 80d6+)",
};



/*
 * Pronoun arrays, by gender.
 */
static cptr wd_he[3] = { "it", "he", "she" };
static cptr wd_his[3] = { "its", "his", "her" };


/*
 * Pluralizer.  Args(count, singular, plural)
 */
#define plural(c,s,p) \
    (((c) == 1) ? (s) : (p))






/*
 * Learn about a monster (by "probing" it)
 */
void lore_do_probe(monster_type *m_ptr)
{
    monster_race *r_ptr = &r_list[m_ptr->r_idx];
    monster_lore *l_ptr = &l_list[m_ptr->r_idx];

    /* Hack -- Memorize some flags */
    l_ptr->flags1 = r_ptr->rflags1;
    l_ptr->flags2 = r_ptr->rflags2;
    l_ptr->flags3 = r_ptr->rflags3;

    /* Redraw the recall window */
    p_ptr->redraw |= (PR_RECALL);
}


/*
 * Take note that the given monster just dropped some treasure
 * Note that learning the "GOOD"/"GREAT" flags gives information
 * about the treasure (even when the monster is killed for the first
 * time, such as uniques, and the treasure has not been examined yet).
 */
void lore_treasure(monster_type *m_ptr, int num_item, int num_gold)
{
    monster_race *r_ptr = &r_list[m_ptr->r_idx];
    monster_lore *l_ptr = &l_list[m_ptr->r_idx];

    /* Note the number of things dropped */
    if (num_item > l_ptr->drop_item) l_ptr->drop_item = num_item;
    if (num_gold > l_ptr->drop_gold) l_ptr->drop_gold = num_gold;

    /* Hack -- memorize the good/great flags */
    if (r_ptr->rflags1 & RF1_DROP_GOOD) l_ptr->flags1 |= RF1_DROP_GOOD;
    if (r_ptr->rflags1 & RF1_DROP_GREAT) l_ptr->flags1 |= RF1_DROP_GREAT;
}







/*
 * Keep track of which lines have been cleared
 */
static int max_clear = 0;


/*
 * Complete the recall
 */
static void recall_shut(void)
{
    /* Hack -- use a special window */
    if (use_recall_win && term_recall) {

        /* Use the recall window */
        Term_activate(term_recall);

        /* Erase the rest of the window */
        Term_erase(0, max_clear, 80-1, 24);

        /* Flush the output */
        Term_fresh();

        /* Re-activate the main screen */
        Term_activate(term_screen);
    }

    /* On top of the normal window */
    else {

        /* Erase one line below us */
        Term_erase(0, max_clear, 80-1, max_clear);

        /* Flush the output */
        Term_fresh();
    }
}


/*
 * Begin the recall
 */
static void recall_open(void)
{
    /* Hack -- use a special window */
    if (use_recall_win && term_recall) {

        /* Erase below us */
        max_clear = 0;
    }

    /* On top of the normal window */
    else {

        /* Flush messages */
        msg_print(NULL);

        /* Erase below us */
        max_clear = 0;
    }
}


/*
 * A simple "recall print" function, with color.
 */
static void recall_putstr(int x, int y, int n, byte a, cptr s)
{

#ifdef USE_COLOR
    /* Sometimes black and white */
    if (!use_color) a = TERM_WHITE;
#else
    /* Always black and white */
    a = TERM_WHITE;
#endif
    
    /* Hack -- use a special window */
    if (use_recall_win && term_recall) {

        /* Use the recall window */
        Term_activate(term_recall);

        /* Hack -- Be sure we have cleared up to this line */
        while (max_clear <= y) {
            Term_erase(0, max_clear, 80-1, max_clear);
            max_clear++;
        }

        /* Dump the text */
        Term_putstr(x, y, n, a, s);

        /* Re-activate the main window */
        Term_activate(term_screen);
    }

    /* On top of the normal window */
    else {

        /* Be sure we have cleared up to the line below us */
        while (max_clear <= y) {
            Term_erase(0, max_clear, 80-1, max_clear);
            max_clear++;
        }

        /* Dump the text */
        Term_putstr(x, y, n, a, s);
    }
}



/*
 * Max line size
 */
#define ROFF_WID 79


/*
 * Buffer text, dumping full lines via "recall_putstr()".
 *
 * Automatically wraps to the next line when necessary.
 * Also wraps to the next line on any "newline" in "str".
 * There are never more chars buffered than can be printed.
 * If "str" is NULL, restart (?).
 */
static void c_roff(byte a, cptr str)
{
    cptr p, r;

    /* Line buffer */
    static char roff_buf[256];

    /* Current Pointer into roff_buf */
    static char *roff_p = roff_buf;

    /* Last space saved into roff_buf */
    static char *roff_s = NULL;

    /* Place to print current line */
    static int roff_row = 0;


    /* Special handling for "new sequence" */
    if (!str) {

        /* Reset the row */
        roff_row = 1;

        /* Reset the pointer */
        roff_p = roff_buf;

        /* No spaces yet */
        roff_s = NULL;

        /* Simple string (could just return) */
        str = "";
    }

    /* Scan the given string, character at a time */
    for (p = str; *p; p++) {

        int wrap = (*p == '\n');
        char ch = *p;

        /* Clean up the char */
        if (!isprint(ch)) ch = ' ';

        /* We may be "forced" to wrap */
        if (roff_p >= roff_buf + ROFF_WID) wrap = 1;

        /* Hack -- Try to avoid "hanging" single letter words */
        if ((ch == ' ') && (roff_p + 2 >= roff_buf + ROFF_WID)) wrap = 1;

        /* Handle line-wrap */
        if (wrap) {

            /* We must not dump past here */
            *roff_p = '\0';

            /* Assume nothing will be left over */
            r = roff_p;

            /* If we are in the middle of a word, try breaking on a space */
            if (roff_s && (ch != ' ')) {

                /* Nuke the space */
                *roff_s = '\0';

                /* Remember the char after the space */
                r = roff_s + 1;
            }

            /* Dump the line, advance the row */	
            recall_putstr(0, roff_row++, -1, a, roff_buf);

            /* No spaces yet */
            roff_s = NULL;

            /* Restart the buffer scanner */
            roff_p = roff_buf;

            /* Restore any "wrapped" chars */
            while (*r) *roff_p++ = *r++;
        }

        /* Save the char.  Hack -- skip leading spaces (and newlines) */
        if ((roff_p > roff_buf) || (ch != ' ')) {
            if (ch == ' ') roff_s = roff_p;
            *roff_p++ = ch;
        }
    }
}

static void roff(cptr str)
{
    c_roff(TERM_WHITE, str);
}


/*
 * Determine if the "armor" is known
 * The higher the level, the fewer kills needed.
 */
static bool know_armour(int r_idx)
{
    monster_race *r_ptr = &r_list[r_idx];
    monster_lore *l_ptr = &l_list[r_idx];
    
    s32b level = r_ptr->level;
    s32b kills = l_ptr->tkills;

    /* Normal monsters */
    if (kills > 304 / (4 + level)) return (TRUE);
    
    /* Skip non-uniques */
    if (!(r_ptr->rflags1 & RF1_UNIQUE)) return (FALSE);

    /* Unique monsters */
    if (kills > 304 / (38 + (5*level) / 4)) return (TRUE);
    
    /* Assume false */
    return (FALSE);
}


/*
 * Determine if the "damage" of the given attack is known
 * the higher the level of the monster, the fewer the attacks you need,
 * the more damage an attack does, the more attacks you need
 */
static bool know_damage(int r_idx, int i)
{
    monster_race *r_ptr = &r_list[r_idx];
    monster_lore *l_ptr = &l_list[r_idx];
    
    s32b level = r_ptr->level;
    
    s32b a = l_ptr->blows[i];

    s32b d1 = r_ptr->blow[i].d_dice;
    s32b d2 = r_ptr->blow[i].d_side;

    s32b d = d1 * d2;
    
    /* Normal monsters */
    if ((4 + level) * a > 80 * d) return (TRUE);
    
    /* Skip non-uniques */
    if (!(r_ptr->rflags1 & RF1_UNIQUE)) return (FALSE);

    /* Unique monsters */
    if ((4 + level) * (2 * a) > 80 * d) return (TRUE);
    
    /* Assume false */
    return (FALSE);
}


/*
 * Print out what we have discovered about this monster.
 * Note the use of the r_idx "-1" for "last recalled monster"
 * Several vars changed from int, to avoid PC's 16bit ints -CFT
 */
int roff_recall(int r_idx)
{
    static int last_r_idx = -1;

    monster_lore  *l_ptr;
    monster_race  *r_ptr;
    u32b         i, j;

    bool		old = FALSE;
    bool		sin = FALSE;

    int			m, n, r;

    cptr		p, q;

    int			n_len;
    
    int			msex = 0;

    bool		breath = FALSE;
    bool		magic = FALSE;

    u32b		flags1 = 0L;
    u32b		flags2 = 0L;
    u32b		flags3 = 0L;
    u32b		flags4 = 0L;
    u32b		flags5 = 0L;
    u32b		flags6 = 0L;

    int			vn = 0;
    cptr		vp[64];
    
    monster_lore        save_mem;

    char		out_val[160];



    /* Special handling of "use last monster recalled" */
    if (r_idx < 0) {
        r_idx = last_r_idx;
        if (r_idx < 0) return 0;
    }

    /* Save this monster ID */
    last_r_idx = r_idx;


    /* Access the race and lore */
    r_ptr = &r_list[r_idx];
    l_ptr = &l_list[r_idx];


    /* Extract a gender (if applicable) */
    if (r_ptr->rflags1 & RF1_FEMALE) msex = 2;
    else if (r_ptr->rflags1 & RF1_MALE) msex = 1;


    /* Cheat -- Know everything */
    if (cheat_know) {

        /* Save the "old" memory */
        save_mem = *l_ptr;

        /* Hack -- Maximal kills */
        l_ptr->tkills = MAX_SHORT;

        /* Hack -- Maximal info */
        l_ptr->wake = l_ptr->ignore = MAX_UCHAR;

        /* Observe "maximal" attacks */
        for (m = 0; m < 4; m++) {

            /* Examine "actual" blows */
            if (r_ptr->blow[m].effect || r_ptr->blow[m].method) {

                /* Hack -- maximal observations */
                l_ptr->blows[m] = MAX_UCHAR;
            }
        }

        /* Hack -- maximal drops */
        l_ptr->drop_gold = l_ptr->drop_item =
            (((r_ptr->rflags1 & RF1_DROP_4D2) ? 8 : 0) +
             ((r_ptr->rflags1 & RF1_DROP_3D2) ? 6 : 0) +
             ((r_ptr->rflags1 & RF1_DROP_2D2) ? 4 : 0) +
             ((r_ptr->rflags1 & RF1_DROP_1D2) ? 2 : 0) +
             ((r_ptr->rflags1 & RF1_DROP_90)  ? 1 : 0) +
             ((r_ptr->rflags1 & RF1_DROP_60)  ? 1 : 0));

        /* Hack -- but only "valid" drops */
        if (r_ptr->rflags1 & RF1_ONLY_GOLD) l_ptr->drop_item = 0;
        if (r_ptr->rflags1 & RF1_ONLY_ITEM) l_ptr->drop_gold = 0;
        
        /* Hack -- observe many spells */
        l_ptr->cast_inate = MAX_UCHAR;
        l_ptr->cast_spell = MAX_UCHAR;

        /* Hack -- know all the flags */
        l_ptr->flags1 = r_ptr->rflags1;
        l_ptr->flags2 = r_ptr->rflags2;
        l_ptr->flags3 = r_ptr->rflags3;
        l_ptr->flags4 = r_ptr->rflags4;
        l_ptr->flags5 = r_ptr->rflags5;
        l_ptr->flags6 = r_ptr->rflags6;
    }


    /* Obtain a copy of the "known" flags */
    flags1 = (r_ptr->rflags1 & l_ptr->flags1);
    flags2 = (r_ptr->rflags2 & l_ptr->flags2);
    flags3 = (r_ptr->rflags3 & l_ptr->flags3);
    flags4 = (r_ptr->rflags4 & l_ptr->flags4);
    flags5 = (r_ptr->rflags5 & l_ptr->flags5);
    flags6 = (r_ptr->rflags6 & l_ptr->flags6);


    /* Assume some "obvious" flags */
    if (r_ptr->rflags1 & RF1_UNIQUE) flags1 |= RF1_UNIQUE;
    if (r_ptr->rflags1 & RF1_QUESTOR) flags1 |= RF1_QUESTOR;
    if (r_ptr->rflags1 & RF1_MALE) flags1 |= RF1_MALE;
    if (r_ptr->rflags1 & RF1_FEMALE) flags1 |= RF1_FEMALE;

    /* Assume some "creation" flags */
    if (r_ptr->rflags1 & RF1_FRIEND) flags1 |= RF1_FRIEND;
    if (r_ptr->rflags1 & RF1_FRIENDS) flags1 |= RF1_FRIENDS;
    if (r_ptr->rflags1 & RF1_ESCORT) flags1 |= RF1_ESCORT;
    if (r_ptr->rflags1 & RF1_ESCORTS) flags1 |= RF1_ESCORTS;

    /* Killing a monster reveals some properties */
    if (l_ptr->tkills) {

        /* Know "race" flags */
        if (r_ptr->rflags3 & RF3_ORC) flags3 |= RF3_ORC;
        if (r_ptr->rflags3 & RF3_TROLL) flags3 |= RF3_TROLL;
        if (r_ptr->rflags3 & RF3_GIANT) flags3 |= RF3_GIANT;
        if (r_ptr->rflags3 & RF3_DRAGON) flags3 |= RF3_DRAGON;
        if (r_ptr->rflags3 & RF3_DEMON) flags3 |= RF3_DEMON;
        if (r_ptr->rflags3 & RF3_UNDEAD) flags3 |= RF3_UNDEAD;
        if (r_ptr->rflags3 & RF3_EVIL) flags3 |= RF3_EVIL;
        if (r_ptr->rflags3 & RF3_ANIMAL) flags3 |= RF3_ANIMAL;

        /* Know "forced" flags */
        if (r_ptr->rflags1 & RF1_FORCE_DEPTH) flags1 |= RF1_FORCE_DEPTH;
        if (r_ptr->rflags1 & RF1_FORCE_MAXHP) flags1 |= RF1_FORCE_MAXHP;
    }


    /* Begin a new "recall" */
    recall_open();

    /* No length yet */
    n_len = 0;

    /* A title (use "The" for non-uniques) */
    if (!(flags1 & RF1_UNIQUE)) {
        recall_putstr(n_len, 0, -1, TERM_WHITE, "The ");
        n_len += 4;
    }

    /* Dump the name */
    recall_putstr(n_len, 0, -1, TERM_WHITE, r_ptr->name);
    n_len += strlen(r_ptr->name);

    /* Append the "standard" attr/char info */
    sprintf(out_val, " ('%c')", r_ptr->r_char);
    recall_putstr(n_len, 0, -1, TERM_WHITE, out_val);
    recall_putstr(n_len + 3, 0, 1, r_ptr->r_attr, out_val + 3);
    n_len += 6;

    /* Append the "optional" attr/char info */
    sprintf(out_val, "/('%c')", l_ptr->l_char);
    recall_putstr(n_len, 0, -1, TERM_WHITE, out_val);
    recall_putstr(n_len + 3, 0, 1, l_ptr->l_attr, out_val + 3);
    n_len += 6;

    /* End the line */
    recall_putstr(n_len, 0, -1, TERM_WHITE, ":");
    n_len += 1;


    /* Begin a new "recall" */
    roff(NULL);


    /* Require a flag to show kills */
    if (!(recall_show_kill)) {

        /* nothing */
    }
    
    /* Treat uniques differently */
    else if (flags1 & RF1_UNIQUE) {

        /* Hack -- Determine if the unique is "dead" */
        bool dead = (l_ptr->max_num == 0);

        /* We've been killed... */
        if (l_ptr->deaths) {

            /* Killed ancestors */
            roff(format("%^s has slain %d of your ancestors",
                        wd_he[msex], l_ptr->deaths));

            /* But we've also killed it */
            if (dead) {
                roff(format(", but you have avenged %s!  ",
                            plural(l_ptr->deaths, "him", "them")));
            }

            /* Unavenged (ever) */
            else {
                roff(format(", who %s unavenged.  ",
                            plural(l_ptr->deaths, "remains", "remain")));
            }
        }

        /* Dead unique who never hurt us */
        else if (dead) {
            roff("You have slain this foe.  ");
        }
    }

    /* Not unique, but killed us */
    else if (l_ptr->deaths) {

        /* Dead ancestors */
        roff(format("%d of your ancestors %s been killed by this creature, ",
                    l_ptr->deaths, plural(l_ptr->deaths, "has", "have")));

        /* Totally exterminated */
        if ((l_ptr->pkills >= 30000) && (l_ptr->cur_num == 0)) {
            roff("and you have exterminated all of these creatures.  ");
        }

        /* Some kills this life */
        else if (l_ptr->pkills) {
            roff(format("and you have exterminated at least %d of the creatures.  ",
                        l_ptr->pkills));
        }

        /* Some kills past lives */
        else if (l_ptr->tkills) {
            roff(format("and %s have exterminated at least %d of the creatures.  ",
                        "your ancestors", l_ptr->tkills));
        }

        /* No kills */
        else {
            roff(format("and %s is not ever known to have been defeated.  ",
                        wd_he[msex]));
        }
    }

    /* Normal monsters */
    else {

        /* Totally exterminated */
        if ((l_ptr->pkills >= 30000) && (l_ptr->cur_num == 0)) {
            roff("You have exterminated all of these creatures.  ");
        }

        /* Killed some this life */
        else if (l_ptr->pkills) {
            roff(format("You have killed at least %d of these creatures.  ",
                        l_ptr->pkills));
        }

        /* Killed some last life */
        else if (l_ptr->tkills) {
            roff(format("Your ancestors have killed at least %d of these creatures.  ",
                        l_ptr->tkills));
        }

        /* Killed none */
        else {
            roff("No battles to the death are recalled.  ");
        }
    }


    /* Descriptions */
    if (recall_show_desc) {

	/* XXX XXX XXX Paranoia -- Hack -- Fake description */
	if (!r_ptr->desc) r_ptr->desc = "You see nothing special.";
	
        /* Description */
        roff(r_ptr->desc);
        roff("  ");
    }


    /* Nothing yet */
    old = FALSE;

    /* Describe location */
    if (r_ptr->level == 0) {
        roff(format("%^s lives in the town", wd_he[msex]));
        old = TRUE;
    }
    else if (l_ptr->tkills) {
        if (depth_in_feet) {
            roff(format("%^s is normally found at depths of %d feet",
                        wd_he[msex], r_ptr->level * 50));
        }
        else {
            roff(format("%^s is normally found on dungeon level %d",
                        wd_he[msex], r_ptr->level));
        }
        old = TRUE;
    }


    /* Describe movement */
    if (TRUE) {

        /* Introduction */
        if (old) {
            roff(", and ");
        }
        else {
            roff(format("%^s ", wd_he[msex]));
            old = TRUE;
        }
        roff("moves");

        /* Random-ness */
        if ((flags1 & RF1_RAND_50) || (flags1 & RF1_RAND_25)) {

            /* Adverb */
            if ((flags1 & RF1_RAND_50) && (flags1 & RF1_RAND_25)) {
                roff(" extremely");
            }
            else if (flags1 & RF1_RAND_50) {
                roff(" somewhat");
            }
            else if (flags1 & RF1_RAND_25) {
                roff(" a bit");
            }

            /* Adjective */
            roff(" erratically");

            /* Hack -- Occasional conjunction */
            if (r_ptr->speed != 110) roff(", and");
        }
        
        /* Speed */
        if (r_ptr->speed > 110) {
            if (r_ptr->speed > 130) roff(" incredibly");
            else if (r_ptr->speed > 120) roff(" very");
            roff(" quickly");
        }
        else if (r_ptr->speed < 110) {
            if (r_ptr->speed < 90) roff(" incredibly");
            else if (r_ptr->speed < 100) roff(" very");
            roff(" slowly");
        }
        else {
            roff(" at normal speed");
        }
    }

    /* The code above includes "attack speed" */
    if (flags1 & RF1_NEVER_MOVE) {

        /* Introduce */
        if (old) {
            roff(", but ");
        }
        else {
            roff(format("%^s ", wd_he[msex]));
            old = TRUE;
        }

        /* Describe */
        roff("does not deign to chase intruders");
    }

    /* End this sentence */
    if (old) {
        roff(".  ");
        old = FALSE;
    }


    /* Describe experience if known */
    if (l_ptr->tkills) {

        /* Introduction */        
        if (flags1 & RF1_UNIQUE) {
            roff("Killing this");
        }
        else {
            roff("A kill of this");
        }

        /* Describe the "quality" */
        if (flags3 & RF3_ANIMAL) roff(" natural");
        if (flags3 & RF3_EVIL) roff(" evil");
        if (flags3 & RF3_UNDEAD) roff(" undead");

        /* Describe the "race" */
        if (flags3 & RF3_DRAGON) roff(" dragon");
        else if (flags3 & RF3_DEMON) roff(" demon");
        else if (flags3 & RF3_GIANT) roff(" giant");
        else if (flags3 & RF3_TROLL) roff(" troll");
        else if (flags3 & RF3_ORC) roff(" orc");
        else roff(" creature");

        /* calculate the integer exp part */
        i = (long)r_ptr->mexp * r_ptr->level / p_ptr->lev;

        /* calculate the fractional exp part scaled by 100, */
        /* must use long arithmetic to avoid overflow  */
        j = ((((long)r_ptr->mexp * r_ptr->level % p_ptr->lev) * (long)1000 /
             p_ptr->lev + 5) / 10);

        /* Mention the experience */
        roff(format(" is worth %ld.%02ld point%s",
                    (long)i, (long)j,
                    (((i == 1) && (j == 0)) ? "" : "s")));

        /* Take account of annoying English */
        p = "th";
        i = p_ptr->lev % 10;
        if ((p_ptr->lev / 10) == 1);
        else if (i == 1) p = "st";
        else if (i == 2) p = "nd";
        else if (i == 3) p = "rd";

        /* Take account of "leading vowels" in numbers */
        q = "";
        i = p_ptr->lev;
        if ((i == 8) || (i == 11) || (i == 18)) q = "n";

        /* Mention the dependance on the player's level */
        roff(format(" for a%s %lu%s level character.  ",
                    q, (long)i, p));
    }


    /* Describe escorts */
    if ((flags1 & RF1_ESCORT) || (flags1 & RF1_ESCORTS)) {
        roff(format("%^s usually appears with escorts.  ",
                    wd_he[msex]));
    }

    /* Describe friends */
    else if ((flags1 & RF1_FRIEND) || (flags1 & RF1_FRIENDS)) {
        roff(format("%^s usually appears in groups.  ",
                    wd_he[msex]));
    }


    /* Collect inate attacks */
    vn = 0;
    if (flags4 & RF4_SHRIEK)		vp[vn++] = "shriek for help";
    if (flags4 & RF4_XXX2X4)		vp[vn++] = "do something";
    if (flags4 & RF4_XXX3X4)		vp[vn++] = "do something";
    if (flags4 & RF4_XXX4X4)		vp[vn++] = "do something";
    if (flags4 & RF4_ARROW_1)		vp[vn++] = "fire arrows";
    if (flags4 & RF4_ARROW_2)		vp[vn++] = "fire arrows";
    if (flags4 & RF4_ARROW_3)		vp[vn++] = "fire missiles";
    if (flags4 & RF4_ARROW_4)		vp[vn++] = "fire missiles";

    /* Describe inate attacks */
    if (vn) {

        /* Intro */
        roff(format("%^s", wd_he[msex]));

        /* Scan */
        for (n = 0; n < vn; n++) {

            /* Intro */
            if (n == 0) roff(" may ");
            else if (n < vn-1) roff(", ");
            else roff(" or ");

            /* Dump */
            roff(vp[n]);
        }

        /* End */
        roff(".  ");
    }


    /* Collect breaths */
    vn = 0;
    if (flags4 & RF4_BR_ACID)		vp[vn++] = "acid";
    if (flags4 & RF4_BR_ELEC)		vp[vn++] = "lightning";
    if (flags4 & RF4_BR_FIRE)		vp[vn++] = "fire";
    if (flags4 & RF4_BR_COLD)		vp[vn++] = "frost";
    if (flags4 & RF4_BR_POIS)		vp[vn++] = "poison";
    if (flags4 & RF4_BR_NETH)		vp[vn++] = "nether";
    if (flags4 & RF4_BR_LITE)		vp[vn++] = "light";
    if (flags4 & RF4_BR_DARK)		vp[vn++] = "darkness";
    if (flags4 & RF4_BR_CONF)		vp[vn++] = "confusion";
    if (flags4 & RF4_BR_SOUN)		vp[vn++] = "sound";
    if (flags4 & RF4_BR_CHAO)		vp[vn++] = "chaos";
    if (flags4 & RF4_BR_DISE)		vp[vn++] = "disenchantment";
    if (flags4 & RF4_BR_NEXU)		vp[vn++] = "nexus";
    if (flags4 & RF4_BR_TIME)		vp[vn++] = "time";
    if (flags4 & RF4_BR_INER)		vp[vn++] = "inertia";
    if (flags4 & RF4_BR_GRAV)		vp[vn++] = "gravity";
    if (flags4 & RF4_BR_SHAR)		vp[vn++] = "shards";
    if (flags4 & RF4_BR_PLAS)		vp[vn++] = "plasma";
    if (flags4 & RF4_BR_WALL)		vp[vn++] = "force";
    if (flags4 & RF4_BR_MANA)		vp[vn++] = "mana";
    if (flags4 & RF4_XXX5X4)		vp[vn++] = "something";
    if (flags4 & RF4_XXX6X4)		vp[vn++] = "something";
    if (flags4 & RF4_XXX7X4)		vp[vn++] = "something";
    if (flags4 & RF4_XXX8X4)		vp[vn++] = "something";

    /* Describe breaths */
    if (vn) {

        /* Note breath */
        breath = TRUE;
        
        /* Intro */
        roff(format("%^s", wd_he[msex]));

        /* Scan */
        for (n = 0; n < vn; n++) {

            /* Intro */
            if (n == 0) roff(" may breathe ");
            else if (n < vn-1) roff(", ");
            else roff(" or ");

            /* Dump */
            roff(vp[n]);
        }
    }


    /* Collect spells */
    vn = 0;
    if (flags5 & RF5_BA_ACID)		vp[vn++] = "produce acid balls";
    if (flags5 & RF5_BA_ELEC)		vp[vn++] = "produce lightning balls";
    if (flags5 & RF5_BA_FIRE)		vp[vn++] = "produce fire balls";
    if (flags5 & RF5_BA_COLD)		vp[vn++] = "produce frost balls";
    if (flags5 & RF5_BA_POIS)		vp[vn++] = "produce poison balls";
    if (flags5 & RF5_BA_NETH)		vp[vn++] = "produce nether balls";
    if (flags5 & RF5_BA_WATE)		vp[vn++] = "produce water balls";
    if (flags5 & RF5_BA_MANA)		vp[vn++] = "produce mana storms";
    if (flags5 & RF5_BA_DARK)		vp[vn++] = "produce darkness storms";
    if (flags5 & RF5_DRAIN_MANA)	vp[vn++] = "drain mana";
    if (flags5 & RF5_MIND_BLAST)	vp[vn++] = "cause mind blasting";
    if (flags5 & RF5_BRAIN_SMASH)	vp[vn++] = "cause brain smashing";
    if (flags5 & RF5_CAUSE_1)		vp[vn++] = "cause light wounds";
    if (flags5 & RF5_CAUSE_2)		vp[vn++] = "cause serious wounds";
    if (flags5 & RF5_CAUSE_3)		vp[vn++] = "cause critical wounds";
    if (flags5 & RF5_CAUSE_4)		vp[vn++] = "cause mortal wounds";
    if (flags5 & RF5_BO_ACID)		vp[vn++] = "produce acid bolts";
    if (flags5 & RF5_BO_ELEC)		vp[vn++] = "produce lightning bolts";
    if (flags5 & RF5_BO_FIRE)		vp[vn++] = "produce fire bolts";
    if (flags5 & RF5_BO_COLD)		vp[vn++] = "produce frost bolts";
    if (flags5 & RF5_BO_POIS)		vp[vn++] = "produce poison bolts";
    if (flags5 & RF5_BO_NETH)		vp[vn++] = "produce nether bolts";
    if (flags5 & RF5_BO_WATE)		vp[vn++] = "produce water bolts";
    if (flags5 & RF5_BO_MANA)		vp[vn++] = "produce mana bolts";
    if (flags5 & RF5_BO_PLAS)		vp[vn++] = "produce plasma bolts";
    if (flags5 & RF5_BO_ICEE)		vp[vn++] = "produce ice bolts";
    if (flags5 & RF5_MISSILE)		vp[vn++] = "produce magic missiles";
    if (flags5 & RF5_SCARE)		vp[vn++] = "terrify";
    if (flags5 & RF5_BLIND)		vp[vn++] = "blind";
    if (flags5 & RF5_CONF)		vp[vn++] = "confuse";
    if (flags5 & RF5_SLOW)		vp[vn++] = "slow";
    if (flags5 & RF5_HOLD)		vp[vn++] = "paralyze";
    if (flags6 & RF6_HASTE)		vp[vn++] = "haste-self";
    if (flags6 & RF6_XXX1X6)		vp[vn++] = "do something";
    if (flags6 & RF6_HEAL)		vp[vn++] = "heal-self";
    if (flags6 & RF6_XXX2X6)		vp[vn++] = "do something";
    if (flags6 & RF6_BLINK)		vp[vn++] = "blink-self";
    if (flags6 & RF6_TPORT)		vp[vn++] = "teleport-self";
    if (flags6 & RF6_XXX3X6)		vp[vn++] = "do something";
    if (flags6 & RF6_XXX4X6)		vp[vn++] = "do something";
    if (flags6 & RF6_TELE_TO)		vp[vn++] = "teleport to";
    if (flags6 & RF6_TELE_AWAY)		vp[vn++] = "teleport away";
    if (flags6 & RF6_TELE_LEVEL)	vp[vn++] = "teleport level";
    if (flags6 & RF6_XXX5)		vp[vn++] = "do something";
    if (flags6 & RF6_DARKNESS)		vp[vn++] = "create darkness";
    if (flags6 & RF6_TRAPS)		vp[vn++] = "create traps";
    if (flags6 & RF6_FORGET)		vp[vn++] = "cause amnesia";
    if (flags6 & RF6_XXX6X6)		vp[vn++] = "do something";
    if (flags6 & RF6_XXX7X6)		vp[vn++] = "do something";
    if (flags6 & RF6_XXX8X6)		vp[vn++] = "do something";
    if (flags6 & RF6_S_MONSTER)		vp[vn++] = "summon a monster";
    if (flags6 & RF6_S_MONSTERS)	vp[vn++] = "summon monsters";
    if (flags6 & RF6_S_ANT)		vp[vn++] = "summon ants";
    if (flags6 & RF6_S_SPIDER)		vp[vn++] = "summon spiders";
    if (flags6 & RF6_S_HOUND)		vp[vn++] = "summon hounds";
    if (flags6 & RF6_S_REPTILE)		vp[vn++] = "summon reptiles";
    if (flags6 & RF6_S_ANGEL)		vp[vn++] = "summon an angel";
    if (flags6 & RF6_S_DEMON)		vp[vn++] = "summon a demon";
    if (flags6 & RF6_S_UNDEAD)		vp[vn++] = "summon an undead";
    if (flags6 & RF6_S_DRAGON)		vp[vn++] = "summon a dragon";
    if (flags6 & RF6_S_HI_UNDEAD)	vp[vn++] = "summon Greater Undead";
    if (flags6 & RF6_S_HI_DRAGON)	vp[vn++] = "summon Ancient Dragons";
    if (flags6 & RF6_S_WRAITH)		vp[vn++] = "summon Ring Wraiths";
    if (flags6 & RF6_S_UNIQUE)		vp[vn++] = "summon Unique Monsters";

    /* Describe spells */
    if (vn) {

        /* Note magic */
        magic = TRUE;
        
        /* Intro */
        if (breath) {
            roff(", and is also");
        }
        else {
            roff(format("%^s is", wd_he[msex]));
        }

        /* Verb Phrase */
        roff(" magical, casting spells");

        /* Adverb */
        if (flags2 & RF2_SMART) roff(" intelligently");
        
        /* Scan */
        for (n = 0; n < vn; n++) {

            /* Intro */
            if (n == 0) roff(" which ");
            else if (n < vn-1) roff(", ");
            else roff(" or ");

            /* Dump */
            roff(vp[n]);
        }
    }


    /* End the sentence about inate/other spells */
    if (breath || magic) {

        /* Total casting */
        m = l_ptr->cast_inate + l_ptr->cast_spell;
        
        /* Average frequency */
        n = (r_ptr->freq_inate + r_ptr->freq_spell) / 2;

        /* Describe the spell frequency */
        if (m > 100) {
            roff(format("; 1 time in %d", 100 / n));
        }

        /* Guess at the frequency */
        else if (m) {
            n = ((n + 9) / 10) * 10;
            roff(format("; about 1 time in %d", 100 / n));
        }
        
        /* End this sentence */
        roff(".  ");
    }


    /* Describe monster "toughness" */
    if (know_armour(r_idx)) {

        roff(format("%^s has an armor rating of %d",
                    wd_he[msex], r_ptr->ac));
        
        roff(format(" and a%s life rating of %dd%d.  ",
                    ((flags1 & RF1_FORCE_MAXHP) ? " maximized" : ""),
                    r_ptr->hdice, r_ptr->hside));
    }



    /* Collect special abilities. */
    vn = 0;
    if (flags2 & RF2_OPEN_DOOR) vp[vn++] = "open doors";
    if (flags2 & RF2_BASH_DOOR) vp[vn++] = "bash down doors";
    if (flags2 & RF2_PASS_WALL) vp[vn++] = "pass through walls";
    if (flags2 & RF2_KILL_WALL) vp[vn++] = "bore through walls";
    if (flags2 & RF2_MOVE_BODY) vp[vn++] = "push past weaker monsters";
    if (flags2 & RF2_KILL_BODY) vp[vn++] = "destroy weaker monsters";
    if (flags2 & RF2_TAKE_ITEM) vp[vn++] = "pick up objects";
    if (flags2 & RF2_KILL_ITEM) vp[vn++] = "destroy objects";

    /* Describe special abilities. */
    if (vn) {

        /* Intro */
        roff(format("%^s", wd_he[msex]));

        /* Scan */
        for (n = 0; n < vn; n++) {

            /* Intro */
            if (n == 0) roff(" can ");
            else if (n < vn-1) roff(", ");
            else roff(" and ");

            /* Dump */
            roff(vp[n]);
        }

        /* End */
        roff(".  ");
    }


    /* Describe special abilities. */
    if (flags2 & RF2_INVISIBLE) {
        roff(format("%^s is invisible.  ", wd_he[msex]));
    }
    if (flags2 & RF2_COLD_BLOOD) {
        roff(format("%^s is cold blooded.  ", wd_he[msex]));
    }
    if (flags2 & RF2_EMPTY_MIND) {
        roff(format("%^s is not detected by telepathy.  ", wd_he[msex]));
    }
    if (flags2 & RF2_WEIRD_MIND) {
        roff(format("%^s is rarely detected by telepathy.  ", wd_he[msex]));
    }
    if (flags2 & RF2_MULTIPLY) {
        roff(format("%^s breeds explosively.  ", wd_he[msex]));
    }
    if (flags2 & RF2_REGENERATE) {
        roff(format("%^s regenerates quickly.  ", wd_he[msex]));
    }


    /* Collect susceptibilities */
    vn = 0;
    if (flags3 & RF3_HURT_ROCK) vp[vn++] = "rock remover";
    if (flags3 & RF3_HURT_LITE) vp[vn++] = "bright light";
    if (flags3 & RF3_HURT_FIRE) vp[vn++] = "fire";
    if (flags3 & RF3_HURT_COLD) vp[vn++] = "cold";

    /* Describe susceptibilities */
    if (vn) {

        /* Intro */
        roff(format("%^s", wd_he[msex]));

        /* Scan */
        for (n = 0; n < vn; n++) {

            /* Intro */
            if (n == 0) roff(" is hurt by ");
            else if (n < vn-1) roff(", ");
            else roff(" and ");

            /* Dump */
            roff(vp[n]);
        }

        /* End */
        roff(".  ");
    }


    /* Collect immunities */
    vn = 0;
    if (flags3 & RF3_IM_ACID) vp[vn++] = "acid";
    if (flags3 & RF3_IM_ELEC) vp[vn++] = "lightning";
    if (flags3 & RF3_IM_FIRE) vp[vn++] = "fire";
    if (flags3 & RF3_IM_COLD) vp[vn++] = "cold";
    if (flags3 & RF3_IM_POIS) vp[vn++] = "poison";

    /* Describe immunities */
    if (vn) {

        /* Intro */
        roff(format("%^s", wd_he[msex]));

        /* Scan */
        for (n = 0; n < vn; n++) {

            /* Intro */
            if (n == 0) roff(" resists ");
            else if (n < vn-1) roff(", ");
            else roff(" and ");

            /* Dump */
            roff(vp[n]);
        }

        /* End */
        roff(".  ");
    }


    /* Collect resistances */
    vn = 0;
    if (flags3 & RF3_RES_NETH) vp[vn++] = "nether";
    if (flags3 & RF3_RES_WATE) vp[vn++] = "water";
    if (flags3 & RF3_RES_PLAS) vp[vn++] = "plasma";
    if (flags3 & RF3_RES_NEXU) vp[vn++] = "nexus";
    if (flags3 & RF3_RES_DISE) vp[vn++] = "disenchantment";

    /* Describe resistances */
    if (vn) {

        /* Intro */
        roff(format("%^s", wd_he[msex]));

        /* Scan */
        for (n = 0; n < vn; n++) {

            /* Intro */
            if (n == 0) roff(" resists ");
            else if (n < vn-1) roff(", ");
            else roff(" and ");

            /* Dump */
            roff(vp[n]);
        }

        /* End */
        roff(".  ");
    }


    /* Collect non-effects */
    vn = 0;
    if (flags3 & RF3_NO_STUN) vp[vn++] = "stunned";
    if (flags3 & RF3_NO_FEAR) vp[vn++] = "frightened";
    if (flags3 & RF3_NO_CONF) vp[vn++] = "confused";
    if (flags3 & RF3_NO_SLEEP) vp[vn++] = "slept";

    /* Describe non-effects */
    if (vn) {

        /* Intro */
        roff(format("%^s", wd_he[msex]));

        /* Scan */
        for (n = 0; n < vn; n++) {

            /* Intro */
            if (n == 0) roff(" cannot be ");
            else if (n < vn-1) roff(", ");
            else roff(" or ");

            /* Dump */
            roff(vp[n]);
        }

        /* End */
        roff(".  ");
    }


    /* Do we know how aware it is? */
    if ((((int)l_ptr->wake * (int)l_ptr->wake) > r_ptr->sleep) ||
        (l_ptr->ignore == MAX_UCHAR) ||
        ((r_ptr->sleep == 0) && (l_ptr->tkills >= 10))) {

        cptr act = NULL;
        
        if (r_ptr->sleep > 200) {
            act = "prefers to ignore";
        }
        else if (r_ptr->sleep > 95) {
            act = "pays very little attention to";
        }
        else if (r_ptr->sleep > 75) {
            act = "pays little attention to";
        }
        else if (r_ptr->sleep > 45) {
            act = "tends to overlook";
        }
        else if (r_ptr->sleep > 25) {
            act = "takes quite a while to see";
        }
        else if (r_ptr->sleep > 10) {
            act = "takes a while to see";
        }
        else if (r_ptr->sleep > 5) {
            act = "is fairly observant of";
        }
        else if (r_ptr->sleep > 3) {
            act = "is observant of";
        }
        else if (r_ptr->sleep > 1) {
            act = "is very observant of";
        }
        else if (r_ptr->sleep > 0) {
            act = "is vigilant for";
        }
        else {
            act = "is ever vigilant for";
        }
        
        roff(format("%^s %s intruders, which %s may notice from %d feet.  ",
             wd_he[msex], act, wd_he[msex], 10 * r_ptr->aaf));
    }


    /* Drops gold and/or items */
    if (l_ptr->drop_gold || l_ptr->drop_item) {

        /* No "n" needed */
        sin = FALSE;
        
        /* Intro */        
        roff(format("%^s may carry", wd_he[msex]));

        /* Count maximum drop */
        n = MAX(l_ptr->drop_gold, l_ptr->drop_item);

        /* One drops (may need an "n") */
        if (n == 1) {
            roff(" a");
            sin = TRUE;
        }

        /* Two drops */
        else if (n == 2) {
            roff(" one or two");
        }

        /* Many drops */
        else {
            roff(format(" up to %d", n));
        }


        /* Great */
        if (flags1 & RF1_DROP_GREAT) {
            p = " exceptional";
        }

        /* Good (no "n" needed) */
        else if (flags1 & RF1_DROP_GOOD) {
            p = " good";
            sin = FALSE;
        }

        /* Okay */
        else {
            p = NULL;
        }


        /* Objects */
        if (l_ptr->drop_item) {

            /* Handle singular "an" */
            if (sin) roff("n");
            sin = FALSE;

            /* Dump "object(s)" */
            if (p) roff(p);
            roff(" object");
            if (n != 1) roff("s");

            /* Conjunction replaces variety, if needed for "gold" below */
            p = " or";
        }

        /* Treasures */
        if (l_ptr->drop_gold) {

            /* Handle singular "an" */
            if (!p) sin = 0;
            if (sin) roff("n");
            sin = FALSE;

            /* Dump "treasure(s)" */
            if (p) roff(p);
            roff(" treasure");
            if (n != 1) roff("s");
        }

        /* End this sentence */
        roff(".  ");
    }


    /* Count the number of "known" attacks */
    for (n = 0, m = 0; m < 4; m++) {

        /* Skip non-attacks */
        if (!r_ptr->blow[m].method) continue;
        
        /* Count known attacks */
        if (l_ptr->blows[m]) n++;
    }

    /* Examine (and count) the actual attacks */
    for (r = 0, m = 0; m < 4; m++) {

        int method, effect, d1, d2;

        /* Skip non-attacks */
        if (!r_ptr->blow[m].method) continue;
        
        /* Skip unknown attacks */
        if (!l_ptr->blows[m]) continue;

        
        /* Extract the attack info */
        method = r_ptr->blow[m].method;
        effect = r_ptr->blow[m].effect;
        d1 = r_ptr->blow[m].d_dice;
        d2 = r_ptr->blow[m].d_side;


        /* Introduce the attack description */
        if (!r) {
            roff(format("%^s can ", wd_he[msex]));
        }
        else if (r < n-1) {
            roff(", ");
        }
        else {
            roff(", and ");
        }

        /* Describe the method */
        roff(desc_method[method]);

        /* Count the attacks as printed */
        r++;

        /* Describe the effect (if any) */
        if (effect) {

            /* Describe the attack type */
            roff(" to ");
            roff(desc_effect[effect]);

            /* Describe damage (if known) */
            if (d1 && d2 && know_damage(r_idx, m)) {

                /* Display the damage */
                roff(" with damage");
                roff(format(" %dd%d", d1, d2));
            }
        }
    }

    /* Finish sentence above */
    if (r) {
        roff(".  ");
    }

    /* Notice lack of attacks */
    else if (flags1 & RF1_NEVER_BLOW) {
        roff(format("%^s has no physical attacks.  ", wd_he[msex]));
    }
    
    /* Or describe the lack of knowledge */
    else {
        roff(format("Nothing is known about %s attack.  ", wd_his[msex]));
    }


    /* Notice "Quest" monsters */
    if (flags1 & RF1_QUESTOR) {
        roff("You feel an intense desire to kill this monster...  ");
    }


    /* Go down a line */
    roff("\n");

    /* Complete the recall */
    recall_shut();


    /* Hack -- Restore monster memory */
    if (cheat_know) {

        /* Restore memory */
        *l_ptr = save_mem;
    }


    /* Not needed for graphic recall */
    if (use_recall_win && term_recall) return (ESCAPE);


    /* Prompt for pause */
    prt("--pause--", 0, n_len + 3);

    /* Get a keypress */
    n = inkey();

    /* Return it */
    return (n);
}







/*
 * The table of "symbol info" -- each entry is a string of the form
 * "X:desc" where "X" is the trigger, and "desc" is the "info".
 * Note that there must be no entry for "space".
 */
static cptr ident_info[] = {
    " :A dark grid",
    "!:A potion",
    "\":An amulet (or necklace)",
    "#:A stone wall",
    "$:Treasure",
    "%:A magma or quartz vein",
    "&:Demon (Oh dear!)",
    "':An open door",
    "(:Soft armor",
    "):A shield",
    "*:Gems",
    "+:A closed door",
    ",:Food (or mushroom patch)",
    "-:A wand (or rod)",
    ".:Floor",
    "/:A polearm (Axe/Pike/etc)",
        /* "0:unused", */
    "1:Entrance to General Store",
    "2:Entrance to Armory",
    "3:Entrance to Weaponsmith",
    "4:Entrance to Temple",
    "5:Entrance to Alchemy shop",
    "6:Entrance to Magic store",
    "7:Entrance to Black Market",
    "8:Entrance to your home",
        /* "9:unused", */
    "::Rubble",
    ";:A loose rock",
    "<:An up staircase",
    "=:A ring",
    ">:A down staircase",
    "?:A scroll",
    "@:You",
    "A:Angel",
    "B:Birds",
    "C:Canine",
    "D:Ancient Dragon (Beware)",
    "E:Elemental",
    "F:Giant Fly",
    "G:Ghost",
    "H:Hybrid",
    "I:Minor Demon",
    "J:Jabberwock",
    "K:Killer Beetle",
    "L:Lich",
    "M:Mummy",
        /* "N:unused", */
    "O:Ogre",
    "P:Giant humanoid",
    "Q:Quylthulg (Pulsing Flesh Mound)",
    "R:Reptile (or Amphibian)",
    "S:Giant Spider (or Scorpion)",
    "T:Troll",
    "U:Umber Hulk",
    "V:Vampire",
    "W:Wight (or Wraith)",
    "X:Xorn (or Xaren)",
    "Y:Yeti",
    "Z:Zephyr (Elemental) hound",
    "[:Hard armor",
    "\\:A hafted weapon (mace/whip/etc)",
    "]:Misc. armor",
    "^:A trap",
    "_:A staff",
        /* "`:unused", */
    "a:Giant Ant/Ant Lion",
    "b:Giant Bat",
    "c:Giant Centipede",
    "d:Dragon",
    "e:Floating Eye",
    "f:Feline",
    "g:Golem",
    "h:Humanoid (Dwarf/Elf/Halfling)",
    "i:Icky Thing",
    "j:Jelly",
    "k:Kobold",
    "l:Giant Louse",
    "m:Mold",
    "n:Naga",
    "o:Orc",
    "p:Person (Humanoid)",
    "q:Quadruped",
    "r:Rodent",
    "s:Skeleton",
    "t:Giant Tick",
        /* "u:unused", */
    "v:Vortex",
    "w:Worm (or Worm Mass)",
        /* "x:unused", */
    "y:Yeek",
    "z:Zombie",
    "{:A missile (Arrow/bolt/bullet)",
    "|:An edged weapon (sword/dagger/etc)",
    "}:A launcher (Bow/crossbow/sling)",
    "~:A tool (or miscellaneous item)",
    NULL
};



/*
 * Identify a character, allow recall of monsters
 *
 * Several "special" responses recall "mulitple" monsters:
 *   ^A (all monsters)
 *   ^U (all unique monsters)
 *   ^N (all non-unique monsters)
 */
void do_cmd_query_symbol(void)
{
    int		i, j, n;
    char	sym, query;
    char	buf[128];

    bool	all = FALSE;
    bool	uniq = FALSE;
    bool	norm = FALSE;

    bool	kills = FALSE;
    bool	level = FALSE;

    u16b	who[MAX_R_IDX];


    /* The turn is free */
    energy_use = 0;

    /* Get a character, or abort */
    if (!get_com("Enter character to be identified: ", &sym)) return;

    /* Find that character info, and describe it */
    for (i = 0; ident_info[i]; ++i) {
        if (sym == ident_info[i][0]) break;
    }

    /* Describe */
    if (sym == CTRL('A')) {
        all = TRUE;
        strcpy(buf, "Full monster list.");
    }
    else if (sym == CTRL('U')) {
        all = uniq = TRUE;
        strcpy(buf, "Unique monster list.");
    }
    else if (sym == CTRL('N')) {
        all = norm = TRUE;
        strcpy(buf, "Non-unique monster list.");
    }
    else if (ident_info[i]) {
        sprintf(buf, "%c - %s.", sym, ident_info[i] + 2);
    }
    else {
        sprintf(buf, "%c - %s.", sym, "Unknown Symbol");
    }

    /* Display the result */
    prt(buf, 0, 0);


    /* Find the set of matching monsters */
    for (n = 0, i = MAX_R_IDX-1; i > 0; i--) {

        monster_race *r_ptr = &r_list[i];
        monster_lore *l_ptr = &l_list[i];

        /* Nothing to recall */
        if (!cheat_know && !l_ptr->sights) continue;

        /* Require non-unique monsters if needed */
        if (norm && (r_ptr->rflags1 & RF1_UNIQUE)) continue;

        /* Require unique monsters if needed */
        if (uniq && !(r_ptr->rflags1 & RF1_UNIQUE)) continue;

        /* Collect "appropriate" monsters */
        if (all || (r_ptr->r_char == sym)) who[n++] = i;
    }

    /* Nothing to recall */
    if (!n) return;

    /* Ask permission or abort */
    put_str("Recall details? (k/p/y/n): ", 0, 40);
    query = inkey();
    prt("", 0, 40);

    /* Sort by kills (and level) */
    if (query == 'k') {
        level = TRUE;
        kills = TRUE;
        query = 'y';
    }

    /* Sort by level */
    if (query == 'p') {
        level = TRUE;
        query = 'y';
    }

    /* Catch "escape" */
    if (query != 'y') return;


    /* Hack -- bubble-sort by level (then experience) */
    if (level) {
        for (i = 0; i < n - 1; i++) {
            for (j = 0; j < n - 1; j++) {
                int i1 = j;
                int i2 = j + 1;
                int l1 = r_list[who[i1]].level;
                int l2 = r_list[who[i2]].level;
                int e1 = r_list[who[i1]].mexp;
                int e2 = r_list[who[i2]].mexp;
                if ((l1 < l2) || ((l1 == l2) && (e1 < e2))) {
                    int tmp = who[i1];
                    who[i1] = who[i2];
                    who[i2] = tmp;
                }
            }
        }
    }

    /* Hack -- bubble-sort by pkills (or kills) */
    if (kills) {
        for (i = 0; i < n - 1; i++) {
            for (j = 0; j < n - 1; j++) {
                int i1 = j;
                int i2 = j + 1;
                int x1 = l_list[who[i1]].pkills;
                int x2 = l_list[who[i2]].pkills;
                int k1 = l_list[who[i1]].tkills;
                int k2 = l_list[who[i2]].tkills;
                if ((x1 < x2) || (!x1 && !x2 && (k1 < k2))) {
                    int tmp = who[i1];
                    who[i1] = who[i2];
                    who[i2] = tmp;
                }
            }
        }
    }


    /* Scan the monster memory. */
    for (i = 0; i < n; i++) {

        /* Graphic Recall window */
        if (use_recall_win && term_recall) {
            roff_recall(who[i]);
            prt("--pause--", 0, 70);
            query = inkey();
        }

        /* On-screen recall */
        else {
            save_screen();
            query = roff_recall(who[i]);
            restore_screen();
        }

        /* Cancel recall */
        if (query == ESCAPE) break;

        /* Back up one entry */
        if (query == '-') i = (i > 0) ? (i - 2) : (n - 2);
    }


    /* Re-display the identity */
    prt(buf, 0, 0);
}

