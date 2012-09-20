/* File: cmd5.c */

/* Purpose: code for mage/priest spells/prayers */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"



/*
 * Determine if a spell is "okay" for the player to cast or study
 * The spell must be legible, not forgotten, and also, to cast,
 * it must be known, and to study, it must not be known.
 */
static bool spell_okay(int j, bool known)
{
    magic_type *s_ptr;

    /* Access the spell */
    s_ptr = &mp_ptr->info[j];

    /* Spell is illegal */
    if (s_ptr->slevel > p_ptr->lev) return (FALSE);

    /* Spell is forgotten */
    if ((j < 32) ?
        (spell_forgotten1 & (1L << j)) :
        (spell_forgotten2 & (1L << (j - 32)))) {

        /* Never okay */
        return (FALSE);
    }

    /* Spell is learned */
    if ((j < 32) ?
        (spell_learned1 & (1L << j)) :
        (spell_learned2 & (1L << (j - 32)))) {

        /* Okay to cast, not to study */
        return (known);
    }

    /* Okay to study, not to cast */
    return (!known);
}



/*
 * Extra information on a spell		-DRS-
 *
 * We can use up to 14 characters of the buffer 'p'
 *
 * The strings in this function were extracted from the code in the
 * functions "do_cmd_cast()" and "do_cmd_pray()" and may be dated.
 */
static void spell_info(char *p, int j)
{
    /* Default */
    strcpy(p, "");

#ifdef DRS_SHOW_SPELL_INFO

    /* Mage spells */
    if (mp_ptr->spell_book == TV_MAGIC_BOOK) {

        int plev = p_ptr->lev;

        /* Analyze the spell */
        switch (j) {
            case 0: sprintf(p, " dam %dd4", 3+((plev-1)/5)); break;
            case 2: strcpy(p, " range 10"); break;
            case 5: strcpy(p, " heal 2d8"); break;
            case 8: sprintf(p, " dam %d", 10 + (plev / 2)); break;
            case 10: sprintf(p, " dam %dd8", (3+((plev-5)/4))); break;
            case 14: sprintf(p, " range %d", plev * 10); break;
            case 15: strcpy(p," dam 6d8"); break;
            case 16: sprintf(p, " dam %dd8", (5+((plev-5)/4))); break;
            case 24: sprintf(p, " dam %dd8", (8+((plev-5)/4))); break;
            case 26: sprintf(p, " dam %d", 30 + plev); break;
            case 29: sprintf(p, " dur %d+d20", plev); break;
            case 30: sprintf(p, " dam %d", 55 + plev); break;
            case 38: sprintf(p, " dam %dd8", (6+((plev-5)/4))); break;
            case 39: sprintf(p, " dam %d", 40 + plev/2); break;
            case 40: sprintf(p, " dam %d", 40 + plev); break;
            case 41: sprintf(p, " dam %d", 70 + plev); break;
            case 42: sprintf(p, " dam %d", 65 + plev); break;
            case 43: sprintf(p, " dam %d", 300 + plev*2); break;
            case 49: strcpy(p, " dur 20+d20"); break;
            case 50: strcpy(p, " dur 20+d20"); break;
            case 51: strcpy(p, " dur 20+d20"); break;
            case 52: strcpy(p, " dur 20+d20"); break;
            case 53: strcpy(p, " dur 20+d20"); break;
            case 54: strcpy(p, " dur 25+d25"); break;
            case 55: strcpy(p, " dur 30+d20"); break;
            case 56: strcpy(p, " dur 25+d25"); break;
            case 57: sprintf(p, " dur %d+d25", 30+plev); break;
            case 58: strcpy(p, " dur 6+d8"); break;
        }
    }

    /* Priest spells */
    if (mp_ptr->spell_book == TV_PRAYER_BOOK) {

        int plev = p_ptr->lev;

        /* See below */
	int orb = (plev / ((p_ptr->pclass == 2) ? 2 : 4));

        /* Analyze the spell */
        switch (j) {
            case 1: strcpy(p, " heal 2d10"); break;
            case 2: strcpy(p, " dur 12+d12"); break;
            case 9: sprintf(p, " range %d", 3*plev); break;
            case 10: strcpy(p, " heal 4d10"); break;
            case 11: strcpy(p, " dur 24+d24"); break;
            case 15: strcpy(p, " dur 10+d10"); break;
            case 17: sprintf(p, " %d+3d6", plev + orb); break;
            case 18: strcpy(p, " heal 6d10"); break;
            case 19: strcpy(p, " dur 24+d24"); break;
            case 20: sprintf(p, " dur %d+d25", 3*plev); break;
            case 23: strcpy(p, " heal 8d10"); break;
            case 25: strcpy(p, " dur 48+d48"); break;
            case 26: sprintf(p, " dam d%d", 3*plev); break;
            case 27: strcpy(p, " heal 300"); break;
            case 28: sprintf(p, " dam d%d", 3*plev); break;
            case 30: strcpy(p, " heal 1000"); break;
            case 36: strcpy(p, " heal 4d10"); break;
            case 37: strcpy(p, " heal 8d10"); break;
            case 38: strcpy(p, " heal 2000"); break;
            case 41: sprintf(p, " dam d%d", 4*plev); break;
            case 42: sprintf(p, " dam d%d", 4*plev); break;
            case 45: strcpy(p, " dam 200"); break;
            case 52: strcpy(p, " range 10"); break;
            case 53: sprintf(p, " range %d", 8*plev); break;
        }
    }

#endif

}


/*
 * Print a list of spells (for browsing or casting)
 */
static void print_spells(byte *spell, int num)
{
    int			i, j, col;

    magic_type		*s_ptr;

    cptr		comment;

    char		info[80];

    char		out_val[160];


    /* Print column */
    col = 20;

    /* Title the list */
    prt("", 1, col);
    put_str("Name", 1, col + 5);
    put_str("Lv Mana Fail", 1, col + 35);

    /* Dump the spells */
    for (i = 0; i < num; i++) {

        /* Access the spell */
        j = spell[i];

        /* Access the spell */
        s_ptr = &mp_ptr->info[j];

        /* Skip illegible spells */
        if (s_ptr->slevel >= 99) {
            sprintf(out_val, "  %c) %-30s", I2A(i), "(illegible)");
            prt(out_val, 2 + i, col);
            continue;
        }

        /* XXX XXX Could label spells above the players level */

        /* Default to no comment */
        comment = "";

        /* Get an additional comment */
        if (show_spell_info) {
            spell_info(info, j);
            comment = info;
        }

        /* Analyze the spell */
        if ((j < 32) ?
            ((spell_forgotten1 & (1L << j))) :
            ((spell_forgotten2 & (1L << (j - 32))))) {
            comment = " forgotten";
        }
        else if (!((j < 32) ?
                   (spell_learned1 & (1L << j)) :
                   (spell_learned2 & (1L << (j - 32))))) {
            comment = " unknown";
        }
        else if (!((j < 32) ?
                   (spell_worked1 & (1L << j)) :
                   (spell_worked2 & (1L << (j - 32))))) {
            comment = " untried";
        }

        /* Dump the spell --(-- */
        sprintf(out_val, "  %c) %-30s%2d %4d %3d%%%s",
                I2A(i), spell_names[mp_ptr->spell_type][j],
                s_ptr->slevel, s_ptr->smana, spell_chance(j), comment);
        prt(out_val, 2 + i, col);
    }

    /* Clear the bottom line */
    prt("", 2 + i, col);
}




/*
 * Hack -- Print a list of spells in a "term" window.
 * See "print_spells()" for the basic algorithm.
 */
static void display_spells(byte *spell, int num)
{
    int			i, j;

    magic_type		*s_ptr;

    cptr		comment;

    char		info[80];

    char		out_val[160];


    /* Clear it */
    Term_clear();


#if 0
    /* Title the list */
    prt("", 1, col);
    put_str("Name", 1, col + 5);
    put_str("Lv Mana Fail", 1, col + 35);
#endif

    /* Dump the spells */
    for (i = 0; i < num; i++) {

        /* Access the spell */
        j = spell[i];

        /* Access the spell */
        s_ptr = &mp_ptr->info[j];

        /* Skip illegible spells */
        if (s_ptr->slevel >= 99) {
            /* --(-- */
            sprintf(out_val, "%c) %-30s", I2A(i), "(illegible)");
            Term_putstr(0, i, -1, TERM_WHITE, out_val);
            continue;
        }

        /* Default to no comment */
        comment = "";

        /* Get an additional comment */
        if (show_spell_info) {
            spell_info(info, j);
            comment = info;
        }

        /* Analyze the spell */
        if ((j < 32) ?
            ((spell_forgotten1 & (1L << j))) :
            ((spell_forgotten2 & (1L << (j - 32))))) {
            comment = " forgotten";
        }
        else if (!((j < 32) ?
                   (spell_learned1 & (1L << j)) :
                   (spell_learned2 & (1L << (j - 32))))) {
            comment = " unknown";
        }
        else if (!((j < 32) ?
                   (spell_worked1 & (1L << j)) :
                   (spell_worked2 & (1L << (j - 32))))) {
            comment = " untried";
        }

        /* Dump the spell --(-- */
        sprintf(out_val, "%c) %-30s%2d %4d %3d%%%s",
                I2A(i), spell_names[mp_ptr->spell_type][j],
                s_ptr->slevel, s_ptr->smana, spell_chance(j), comment);
        Term_putstr(0, i, -1, TERM_WHITE, out_val);
    }


    /* Refresh */
    Term_fresh();
}





/*
 * Allow user to choose a spell/prayer from the given book.
 *
 * If a valid spell is chosen, saves it in '*sn' and returns TRUE
 * If the user hits escape, returns FALSE, and set '*sn' to -1
 * If there are no legal choices, returns FALSE, and sets '*sn' to -2
 *
 * The "prompt" should be "cast", "recite", or "study"
 * The "known" should be TRUE for cast/pray, FALSE for study
 */
static int get_spell(int *sn, cptr prompt, int sval, bool known)
{
    int			i, j = -1;

    byte		spell[64], num = 0;

    bool		flag, redraw, okay, ask;
    char		choice;

    magic_type		*s_ptr;

    char		out_val[160];

    cptr p = ((mp_ptr->spell_book == TV_MAGIC_BOOK) ? "spell" : "prayer");


    /* Extract spells */
    for (i = 0; i < 64; i++) {

        /* Check for this spell */
        if ((i < 32) ?
            (spell_flags[mp_ptr->spell_type][sval][0] & (1L << i)) :
            (spell_flags[mp_ptr->spell_type][sval][1] & (1L << (i - 32)))) {

            /* Collect this spell */
            spell[num++] = i;
        }
    }


    /* Assume no usable spells */
    okay = FALSE;

    /* Assume no spells available */
    (*sn) = -2;

    /* Check for "okay" spells */
    for (i = 0; i < num; i++) {

        /* Look for "okay" spells */
        if (spell_okay(spell[i], known)) okay = TRUE;
    }

    /* No "okay" spells */
    if (!okay) return (FALSE);


    /* Assume cancelled */
    *sn = (-1);

    /* Nothing chosen yet */
    flag = FALSE;

    /* No redraw yet */
    redraw = FALSE;


#ifdef GRAPHIC_CHOICE

    /* Use the "choice" window */
    if (term_choice && use_choice_spells) {

        /* Activate the choice window */
        Term_activate(term_choice);

        /* Save the contents */
        Term_save();

        /* Display the choices */
        display_spells(spell, num);

        /* Activate the screen window */
        Term_activate(term_screen);
    }

#endif

#ifdef GRAPHIC_MIRROR

    /* Use the "mirror" window */
    if (term_mirror && use_mirror_spells) {

        /* Activate the mirror window */
        Term_activate(term_mirror);

        /* Save the contents */
        Term_save();

        /* Display the choices */
        display_spells(spell, num);

        /* Activate the screen window */
        Term_activate(term_screen);
    }

#endif


    /* Build a prompt (accept all spells) */
    strnfmt(out_val, 78, "(%^ss %c-%c, *=List, ESC=exit) %^s which %s? ",
            p, I2A(0), I2A(num - 1), prompt, p);

    /* Get a spell from the user */
    while (!flag && get_com(out_val, &choice)) {

        /* Request redraw */
        if ((choice == ' ') || (choice == '*') || (choice == '?')) {

            /* Show the list */
            if (!redraw) {

                /* Show list */
                redraw = TRUE;

                /* Save the screen */
                Term_save();

                /* Display a list of spells */
                print_spells(spell, num);
            }

            /* Hide the list */
            else {

                /* Hide list */
                redraw = FALSE;

                /* Restore the screen */
                Term_load();
            }
            
            /* Ask again */
            continue;
        }


        /* Note verify */
        ask = (isupper(choice));

        /* Lowercase */
        if (ask) choice = tolower(choice);
        
        /* Extract request */
        i = A2I(choice);

        /* Totally Illegal */
        if ((i < 0) || (i >= num)) {
            bell();
            continue;
        }

        /* Save the spell index */
        j = spell[i];

        /* Require "okay" spells */
        if (!spell_okay(j, known)) {
            bell();
            msg_format("You may not %s that %s.", prompt, p);
            continue;
        }

        /* Verify it */
        if (ask) {

            char tmp_val[160];

            /* Access the spell */
            s_ptr = &mp_ptr->info[j];

            /* Prompt */
            strnfmt(tmp_val, 78, "%^s %s (%d mana, %d%% fail)? ",
                    prompt, spell_names[mp_ptr->spell_type][j],
                    s_ptr->smana, spell_chance(j));

            /* Belay that order */
            if (!get_check(tmp_val)) continue;
        }

        /* Stop the loop */
        flag = TRUE;	
    }


    /* Restore the screen */
    if (redraw) Term_load();


#ifdef GRAPHIC_MIRROR

    /* Restore the "mirror" window */
    if (term_mirror && use_mirror_spells) {

        /* Activate the mirror window */
        Term_activate(term_mirror);

        /* Restore the term */
        Term_load();

        /* Flush the window */
        Term_fresh();

        /* Activate the screen window */
        Term_activate(term_screen);
    }

#endif

#ifdef GRAPHIC_CHOICE

    /* Restore the "choice" window */
    if (term_choice && use_choice_spells) {

        /* Activate the choice window */
        Term_activate(term_choice);

        /* Restore the term */
        Term_load();

        /* Flush the window */
        Term_fresh();

        /* Activate the screen window */
        Term_activate(term_screen);
    }

#endif

    
    /* Abort if needed */
    if (!flag) return (FALSE);

    /* Save the choice */
    (*sn) = j;

    /* Success */
    return (TRUE);
}




/*
 * Peruse the spells/prayers in a Book
 *
 * Note that *all* spells in the book are listed
 */
void do_cmd_browse(void)
{
    int			i, item, sval;

    byte		spell[64], num = 0;

    inven_type		*i_ptr;


    /* Warriors are illiterate */
    if (!mp_ptr->spell_book) {
        msg_print("You cannot read books!");
        return;
    }

    /* No lite */
    if (p_ptr->blind || no_lite()) {
        msg_print("You cannot see!");
        return;
    }

    /* Confused */
    if (p_ptr->confused) {
        msg_print("You are too confused!");
        return;
    }


    /* Restrict choices to "useful" books */
    item_tester_tval = mp_ptr->spell_book;

    /* Get an item (from inven or floor) */
    if (!get_item(&item, "Browse which book? ", FALSE, TRUE, TRUE)) {
        if (item == -2) msg_print("You have no books that you can read.");
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

    /* Access the item's sval */
    sval = i_ptr->sval;


    /* Extract spells */
    for (i = 0; i < 64; i++) {

        /* Check for this spell */
        if ((i < 32) ?
            (spell_flags[mp_ptr->spell_type][sval][0] & (1L << i)) :
            (spell_flags[mp_ptr->spell_type][sval][1] & (1L << (i - 32)))) {

            /* Collect this spell */
            spell[num++] = i;
        }
    }


    /* Save the screen */
    Term_save();

    /* Display the spells */
    print_spells(spell, num);

    /* Wait for it */
    pause_line(0);

    /* Restore the screen */
    Term_load();
}




/*
 * Study a book to gain a new spell/prayer
 */
void do_cmd_study(void)
{
    int			i, item, sval;

    int			j = -1;

    cptr p = ((mp_ptr->spell_book == TV_MAGIC_BOOK) ? "spell" : "prayer");

    inven_type		*i_ptr;


    if (!mp_ptr->spell_book) {
        msg_print("You cannot read books!");
        return;
    }

    if (p_ptr->blind || no_lite()) {
        msg_print("You cannot see!");
        return;
    }

    if (p_ptr->confused) {
        msg_print("You are too confused!");
        return;
    }

    if (!(p_ptr->new_spells)) {
        msg_format("You cannot learn any new %ss!", p);
        return;
    }


    /* Restrict choices to "useful" books */
    item_tester_tval = mp_ptr->spell_book;

    /* Get an item (from inven or floor) */
    if (!get_item(&item, "Study which book? ", FALSE, TRUE, TRUE)) {
        if (item == -2) msg_print("You have no books that you can read.");
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

    /* Access the item's sval */
    sval = i_ptr->sval;


    /* Mage -- Learn a selected spell */
    if (mp_ptr->spell_book == TV_MAGIC_BOOK) {

        /* Ask for a spell, allow cancel */
        if (!get_spell(&j, "study", sval, FALSE) && (j == -1)) return;
    }

    /* Priest -- Learn a random prayer */
    if (mp_ptr->spell_book == TV_PRAYER_BOOK) {

        int k = 0;

        /* Extract spells */
        for (i = 0; i < 64; i++) {

            /* Check spells in the book */
            if ((i < 32) ?
                (spell_flags[mp_ptr->spell_type][sval][0] & (1L << i)) :
                (spell_flags[mp_ptr->spell_type][sval][1] & (1L << (i - 32)))) {

                /* Skip non "okay" prayers */
                if (!spell_okay(i, FALSE)) continue;

                /* Hack -- Prepare the randomizer */
                k++;

                /* Hack -- Apply the randomizer */
                if (rand_int(k) == 0) j = i;
            }
        }
    }

    /* Nothing to study */
    if (j < 0) {

        /* Message */
        msg_format("You cannot learn any %ss in that book.", p);

        /* Abort */
        return;
    }


    /* Take a turn */
    energy_use = 100;

    /* Learn the spell */
    if (j < 32) {
        spell_learned1 |= (1L << j);
    }
    else {
        spell_learned2 |= (1L << (j - 32));
    }

    /* Find the next open entry in "spell_order[]" */
    for (i = 0; i < 64; i++) {

        /* Stop at the first empty space */
        if (spell_order[i] == 99) break;
    }

    /* Add the spell to the known list */
    spell_order[i++] = j;

    /* Mention the result */
    msg_format("You have learned the %s of %s.",
               p, spell_names[mp_ptr->spell_type][j]);

    /* One less spell available */
    p_ptr->new_spells--;

    /* Report on remaining prayers */
    if (p_ptr->new_spells) {
        msg_format("You can learn more %ss.", p);
    }

    /* Redraw Study Status */
    p_ptr->redraw |= (PR_STUDY);
}



/*
 * Cast a spell
 */
void do_cmd_cast(void)
{
    int			item, sval, j, dir;
    int			chance, beam;
    int			plev = p_ptr->lev;

    inven_type		*i_ptr;
    
    magic_type		*s_ptr;


    /* Require spell ability */
    if (mp_ptr->spell_book != TV_MAGIC_BOOK) {
        msg_print("You cannot cast spells!");
        return;
    }

    /* Require lite */
    if (p_ptr->blind || no_lite()) {
        msg_print("You cannot see!");
        return;
    }

    /* Not when confused */
    if (p_ptr->confused) {
        msg_print("You are too confused!");
        return;
    }


    /* Restrict choices to spell books */
    item_tester_tval = mp_ptr->spell_book;

    /* Get an item (from inven or floor) */
    if (!get_item(&item, "Use which book? ", FALSE, TRUE, TRUE)) {
        if (item == -2) msg_print("You have no spell books!");
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

    /* Access the item's sval */
    sval = i_ptr->sval;


    /* Ask for a spell */
    if (!get_spell(&j, "cast", sval, TRUE)) {
        if (j == -2) msg_print("You don't know any spells in that book.");
        return;
    }


    /* Access the spell */
    s_ptr = &mp_ptr->info[j];


    /* Verify "dangerous" spells */
    if (s_ptr->smana > p_ptr->csp) {

        /* Warning */
        msg_print("You do not have enough mana to cast this spell.");

        /* Verify */
        if (!get_check("Attempt it anyway? ")) return;
    }


    /* Spell failure chance */
    chance = spell_chance(j);

    /* Failed spell */
    if (rand_int(100) < chance) {
        if (flush_failure) flush();
        msg_print("You failed to get the spell off!");
    }

    /* Process spell */
    else {

        /* Hack -- chance of "beam" instead of "bolt" */
        beam = ((p_ptr->pclass == 1) ? plev : (plev / 2));

        /* Spells.  */
        switch (j) {

          case 0:
            if (!get_aim_dir(&dir)) return;
            fire_bolt_or_beam(beam-10, GF_MISSILE, dir,
                              damroll(3 + ((plev - 1) / 5), 4));
            break;

          case 1:
            (void)detect_monsters();
            break;

          case 2:
            teleport_flag = TRUE;
            teleport_dist = 10;
            break;

          case 3:
            (void)lite_area(damroll(2, (plev / 2)), (plev / 10) + 1);
            break;

          case 4:
            (void)detect_treasure();
            break;

          case 5:
            (void)hp_player(damroll(2, 8));
            (void)set_cut(p_ptr->cut - 15);
            break;

          case 6:
            (void)detect_object();
            break;

          case 7:
            (void)detect_sdoor();
            (void)detect_trap();
            break;

          case 8:
            if (!get_aim_dir(&dir)) return;
            fire_ball(GF_POIS, dir,
                      10 + (plev / 2), 2);
            break;

          case 9:
            if (!get_aim_dir(&dir)) return;
            (void)confuse_monster(dir, plev);
            break;

          case 10:
            if (!get_aim_dir(&dir)) return;
            fire_bolt_or_beam(beam-10, GF_ELEC, dir,
                              damroll(3+((plev-5)/4),8));
            break;

          case 11:
            (void)destroy_doors_touch();
            break;

          case 12:
            if (!get_aim_dir(&dir)) return;
            (void)sleep_monster(dir);
            break;

          case 13:
            (void)set_poisoned(0);
            break;

          case 14:
            teleport_flag = TRUE;
            teleport_dist = plev * 5;
            break;

          case 15:
            if (!get_aim_dir(&dir)) return;
            msg_print("A line of blue shimmering light appears.");
            lite_line(dir);
            break;

          case 16:
            if (!get_aim_dir(&dir)) return;
            fire_bolt_or_beam(beam-10, GF_COLD, dir,
                              damroll(5+((plev-5)/4),8));
            break;

          case 17:
            if (!get_aim_dir(&dir)) return;
            (void)wall_to_mud(dir);
            break;

          case 18:
            satisfy_hunger();
            break;

          case 19:
            (void)recharge(5);
            break;

          case 20:
            (void)sleep_monsters_touch();
            break;

          case 21:
            if (!get_aim_dir(&dir)) return;
            (void)poly_monster(dir);
            break;

          case 22:
            (void)ident_spell();
            break;

          case 23:
            (void)sleep_monsters();
            break;

          case 24:
            if (!get_aim_dir(&dir)) return;
            fire_bolt_or_beam(beam, GF_FIRE, dir,
                              damroll(8+((plev-5)/4),8));
            break;

          case 25:
            if (!get_aim_dir(&dir)) return;
            (void)slow_monster(dir);
            break;

          case 26:
            if (!get_aim_dir(&dir)) return;
            fire_ball(GF_COLD, dir,
                      30 + (plev), 2);
            break;

          case 27:
            (void)recharge(40);
            break;

          case 28:
            if (!get_aim_dir(&dir)) return;
            (void)teleport_monster(dir);
            break;

          case 29:
            if (!p_ptr->fast) {
                set_fast(randint(20) + plev);
            }
            else {
                set_fast(p_ptr->fast + randint(5));
            }
            break;

          case 30:
            if (!get_aim_dir(&dir)) return;
            fire_ball(GF_FIRE, dir,
                      55 + (plev), 2);
            break;

          case 31:
            destroy_area(py, px, 15, TRUE);
            break;

          case 32:
            (void)genocide();
            break;

          case 33:
            (void)door_creation();
            break;

          case 34:
            (void)stair_creation();
            break;

          case 35:
            (void)tele_level();
            break;

          case 36:
            earthquake(py, px, 10);
            break;

          case 37:
            if (!p_ptr->word_recall) {
                p_ptr->word_recall = rand_int(20) + 15;
                msg_print("The air about you becomes charged...");
            }
            else {
                p_ptr->word_recall = 0;
                msg_print("A tension leaves the air around you...");
            }
            break;

          case 38:
            if (!get_aim_dir(&dir)) return;
            fire_bolt_or_beam(beam, GF_ACID, dir,
                              damroll(6+((plev-5)/4), 8));
            break;

          case 39:
            if (!get_aim_dir(&dir)) return;
            fire_ball(GF_POIS, dir,
                      20 + (plev / 2), 3);
            break;

          case 40:
            if (!get_aim_dir(&dir)) return;
            fire_ball(GF_ACID, dir,
                      40 + (plev), 2);
            break;

          case 41:
            if (!get_aim_dir(&dir)) return;
            fire_ball(GF_COLD, dir,
                      70 + (plev), 3);
            break;

          case 42:
            if (!get_aim_dir(&dir)) return;
            fire_ball(GF_METEOR, dir,
                      65 + (plev), 3);
            break;

          case 43:
            if (!get_aim_dir(&dir)) return;
            fire_ball(GF_MANA, dir,
                      300 + (plev * 2), ((plev < 30) ? 2 : 3));
            break;

          case 44:
            (void)detect_evil();
            break;

          case 45:
            (void)detect_magic();
            break;

          case 46:
            recharge(100);
            break;

          case 47:
            (void)genocide();
            break;

          case 48:
            (void)mass_genocide();
            break;

          case 49:
            (void)set_oppose_fire(p_ptr->oppose_fire + randint(20) + 20);
            break;

          case 50:
            (void)set_oppose_cold(p_ptr->oppose_cold + randint(20) + 20);
            break;

          case 51:
            (void)set_oppose_acid(p_ptr->oppose_acid + randint(20) + 20);
            break;

          case 52:
            (void)set_oppose_pois(p_ptr->oppose_pois + randint(20) + 20);
            break;

          case 53:
            (void)set_oppose_acid(p_ptr->oppose_acid + randint(20) + 20);
            (void)set_oppose_elec(p_ptr->oppose_elec + randint(20) + 20);
            (void)set_oppose_fire(p_ptr->oppose_fire + randint(20) + 20);
            (void)set_oppose_cold(p_ptr->oppose_cold + randint(20) + 20);
            (void)set_oppose_pois(p_ptr->oppose_pois + randint(20) + 20);
            break;

          case 54:
            (void)hp_player(10);	/* XXX */
            (void)set_afraid(0);
            (void)set_hero(p_ptr->hero + randint(25) + 25);
            break;

          case 55:
            (void)set_shield(p_ptr->shield + randint(20) + 30);
            break;

          case 56:
            (void)hp_player(30);	/* XXX */
            (void)set_afraid(0);
            (void)set_shero(p_ptr->shero + randint(25) + 25);
            break;

          case 57:
            if (!p_ptr->fast) {
                (void)set_fast(randint(30) + 30 + plev);
            }
            else {
                (void)set_fast(p_ptr->fast + randint(10));
            }
            break;

          case 58:
            (void)set_invuln(p_ptr->invuln + randint(8) + 8);
            break;

          default:
            break;
        }

        /* A spell was cast */
        if (!((j < 32) ?
              (spell_worked1 & (1L << j)) :
              (spell_worked2 & (1L << (j - 32))))) {

            int e = s_ptr->sexp;

            /* The spell worked */
            if (j < 32) {
                spell_worked1 |= (1L << j);
            }
            else {
                spell_worked2 |= (1L << (j - 32));
            }

            /* Gain experience */
            gain_exp(e * s_ptr->slevel);
        }
    }

    /* Take a turn */
    energy_use = 100;

    /* Sufficient mana */
    if (s_ptr->smana <= p_ptr->csp) {

        /* Use some mana */
        p_ptr->csp -= s_ptr->smana;
    }

    /* Over-exert the player */
    else {

        int oops = s_ptr->smana - p_ptr->csp;
        
        /* No mana left */
        p_ptr->csp = 0;
        p_ptr->csp_frac = 0;

        /* Message */
        msg_print("You faint from the effort!");

        /* Hack -- Bypass free action */
        (void)set_paralyzed(p_ptr->paralyzed + randint(5 * oops + 1));

        /* Damage CON (possibly permanently) */
        if (rand_int(100) < 50) {

            bool perm = (rand_int(100) < 25);

            /* Message */
            msg_print("You have damaged your health!");

            /* Bypass "sustain strength" */
            (void)dec_stat(A_CON, 15 + randint(10), perm);
        }
    }

    /* Update stuff */
    p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

    /* Redraw mana */
    p_ptr->redraw |= (PR_MANA);
}


/*
 * Brand the current weapon
 */
static void brand_weapon(void)
{
    inven_type *i_ptr;

    i_ptr = &inventory[INVEN_WIELD];

    /* you can never modify artifacts / ego-items */
    /* you can never modify broken / cursed items */
    if ((i_ptr->k_idx) &&
        (!artifact_p(i_ptr)) && (!ego_item_p(i_ptr)) &&
        (!broken_p(i_ptr)) && (!cursed_p(i_ptr))) {

        cptr act = NULL;

        char i_name[80];

        if (rand_int(100) < 25) {
            act = "is covered in a fiery shield!";
            i_ptr->name2 = EGO_BRAND_FIRE;
        }

        else {
            act = "glows deep, icy blue!";
            i_ptr->name2 = EGO_BRAND_COLD;
        }

        objdes(i_name, i_ptr, FALSE, 0);

        msg_format("Your %s %s", i_name, act);

        enchant(i_ptr, rand_int(3) + 4, ENCH_TOHIT | ENCH_TODAM);
    }

    else {
        if (flush_failure) flush();
        msg_print("The Branding failed.");
    }
}


/*
 * Pray a prayer
 */
void do_cmd_pray(void)
{
    int item, sval, j, dir, chance;
    int plev = p_ptr->lev;

    inven_type	*i_ptr;
    
    magic_type  *s_ptr;


    /* Must use prayer books */
    if (mp_ptr->spell_book != TV_PRAYER_BOOK) {
        msg_print("Pray hard enough and your prayers may be answered.");
        return;
    }

    /* Must have lite */
    if (p_ptr->blind || no_lite()) {
        msg_print("You cannot see!");
        return;
    }

    /* Must not be confused */
    if (p_ptr->confused) {
        msg_print("You are too confused!");
        return;
    }


    /* Restrict choices */
    item_tester_tval = mp_ptr->spell_book;

    /* Get an item (from inven or floor) */
    if (!get_item(&item, "Use which book? ", FALSE, TRUE, TRUE)) {
        if (item == -2) msg_print("You have no prayer books!");
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

    /* Access the item's sval */
    sval = i_ptr->sval;


    /* Choose a spell */
    if (!get_spell(&j, "recite", sval, TRUE)) {
        if (j == -2) msg_print("You don't know any prayers in that book.");
        return;
    }


    /* Access the spell */
    s_ptr = &mp_ptr->info[j];


    /* Verify "dangerous" prayers */
    if (s_ptr->smana > p_ptr->csp) {

        /* Warning */
        msg_print("You do not have enough mana to recite this prayer.");

        /* Verify */
        if (!get_check("Attempt it anyway? ")) return;
    }


    /* Spell failure chance */
    chance = spell_chance(j);

    /* Check for failure */
    if (rand_int(100) < chance) {
        if (flush_failure) flush();
        msg_print("You failed to concentrate hard enough!");
    }

    /* Success */
    else {

        switch (j) {

          case 0:
            (void)detect_evil();
            break;

          case 1:
            (void)hp_player(damroll(2, 10));
            (void)set_cut(p_ptr->cut - 10);
            break;

          case 2:
            (void)set_blessed(p_ptr->blessed + randint(12) + 12);
            break;

          case 3:
            (void)set_afraid(0);
            break;

          case 4:
            (void)lite_area(damroll(2, (plev / 2)), (plev / 10) + 1);
            break;

          case 5:
            (void)detect_trap();
            break;

          case 6:
            (void)detect_sdoor();
            break;

          case 7:
            (void)set_poisoned(p_ptr->poisoned / 2);
            break;

          case 8:
            if (!get_aim_dir(&dir)) return;
            (void)fear_monster(dir, plev);
            break;

          case 9:
            teleport_flag = TRUE;
            teleport_dist = plev * 3;
            break;

          case 10:
            (void)hp_player(damroll(4, 10));
            (void)set_cut((p_ptr->cut / 2) - 20);
            break;

          case 11:
            (void)set_blessed(p_ptr->blessed + randint(24) + 24);
            break;

          case 12:
            (void)sleep_monsters_touch();
            break;

          case 13:
            satisfy_hunger();
            break;

          case 14:
            remove_curse();
            break;

          case 15:
            (void)set_oppose_fire(p_ptr->oppose_fire + randint(10) + 10);
            (void)set_oppose_cold(p_ptr->oppose_cold + randint(10) + 10);
            break;

          case 16:
            (void)set_poisoned(0);
            break;

          case 17:
            if (!get_aim_dir(&dir)) return;
            fire_ball(GF_HOLY_ORB, dir,
                      (damroll(3,6) + plev +
                       (plev / ((p_ptr->pclass == 2) ? 2 : 4))),
                      ((plev < 30) ? 2 : 3));
            break;

          case 18:
            (void)hp_player(damroll(6, 10));
            (void)set_cut(0);
            break;

          case 19:
            (void)set_tim_invis(p_ptr->tim_invis + randint(24) + 24);
            break;

          case 20:
            (void)set_protevil(p_ptr->protevil + randint(25) + 3 * p_ptr->lev);
            break;

          case 21:
            earthquake(py, px, 10);
            break;

          case 22:
            map_area();
            break;

          case 23:
            (void)hp_player(damroll(8, 10));
            (void)set_cut(0);
            (void)set_stun(0);
            break;

          case 24:
            (void)turn_undead();
            break;

          case 25:
            (void)set_blessed(p_ptr->blessed + randint(48) + 48);
            break;

          case 26:
            (void)dispel_undead(plev * 3);
            break;

          case 27:
            (void)hp_player(300);
            (void)set_stun(0);
            (void)set_cut(0);
            break;

          case 28:
            (void)dispel_evil(plev * 3);
            break;

          case 29:
            warding_glyph();
            break;

          case 30:
            (void)dispel_evil(plev * 4);
            (void)hp_player(1000);
            (void)set_afraid(0);
            (void)set_poisoned(0);
            (void)set_stun(0);
            (void)set_cut(0);
            break;

          case 31:
            (void)detect_monsters();
            break;

          case 32:
            (void)detection();
            break;

          case 33:
            (void)ident_spell();
            break;

          case 34:
            (void)probing();
            break;

          case 35:
            wiz_lite();
            break;

          case 36:
            (void)hp_player(damroll(4, 10));
            (void)set_cut(0);
            break;

          case 37:
            (void)hp_player(damroll(8, 10));
            (void)set_stun(0);
            (void)set_cut(0);
            break;

          case 38:
            (void)hp_player(2000);
            (void)set_stun(0);
            (void)set_cut(0);
            break;

          case 39:
            (void)do_res_stat(A_STR);
            (void)do_res_stat(A_INT);
            (void)do_res_stat(A_WIS);
            (void)do_res_stat(A_DEX);
            (void)do_res_stat(A_CON);
            (void)do_res_stat(A_CHR);
            break;

          case 40:
            (void)restore_level();
            break;

          case 41:
            (void)dispel_undead(plev * 4);
            break;

          case 42:
            (void)dispel_evil(plev * 4);
            break;

          case 43:
            if (banish_evil(100)) {
                msg_print("The power of your god banishes evil!");
            }
            break;

          case 44:
            destroy_area(py, px, 15, TRUE);
            break;

          case 45:
            if (!get_aim_dir(&dir)) return;
            drain_life(dir, 200);
            break;

          case 46:
            (void)destroy_doors_touch();
            break;

          case 47:
            (void)recharge(15);
            break;

          case 48:
            (void)remove_all_curse();
            break;

          case 49:
            (void)enchant_spell(rand_int(4) + 1, rand_int(4) + 1, 0);
            break;

          case 50:
            (void)enchant_spell(0, 0, rand_int(3) + 2);
            break;

          case 51:
            brand_weapon();
            break;

          case 52:
            teleport_flag = TRUE;
            teleport_dist = 10;
            break;

          case 53:
            teleport_flag = TRUE;
            teleport_dist = plev * 8;
            break;

          case 54:
            if (!get_aim_dir(&dir)) return;
            (void)teleport_monster(dir);
            break;

          case 55:
            (void)tele_level();
            break;

          case 56:
            if (p_ptr->word_recall == 0) {
                p_ptr->word_recall = rand_int(20) + 15;
                msg_print("The air about you becomes charged...");
            }
            else {
                p_ptr->word_recall = 0;
                msg_print("A tension leaves the air around you...");
            }
            break;

          case 57:
            msg_print("The world changes!");
            new_level_flag = TRUE;
            break;

          default:
            break;
        }

        /* A prayer was prayed */
        if (!((j < 32) ?
              (spell_worked1 & (1L << j)) :
              (spell_worked2 & (1L << (j - 32))))) {

            int e = s_ptr->sexp;

            /* The spell worked */
            if (j < 32) {
                spell_worked1 |= (1L << j);
            }
            else {
                spell_worked2 |= (1L << (j - 32));
            }

            /* Gain experience */
            gain_exp(e * s_ptr->slevel);
        }
    }

    /* Take a turn */
    energy_use = 100;

    /* Sufficient mana */
    if (s_ptr->smana <= p_ptr->csp) {

        /* Use some mana */
        p_ptr->csp -= s_ptr->smana;
    }

    /* Over-exert the player */
    else {

        int oops = s_ptr->smana - p_ptr->csp;
        
        /* No mana left */
        p_ptr->csp = 0;
        p_ptr->csp_frac = 0;

        /* Message */
        msg_print("You faint from the effort!");

        /* Hack -- Bypass free action */
        (void)set_paralyzed(p_ptr->paralyzed + randint(5 * oops + 1));

        /* Damage CON (possibly permanently) */
        if (rand_int(100) < 50) {

            bool perm = (rand_int(100) < 25);

            /* Message */
            msg_print("You have damaged your health!");

            /* Bypass "sustain strength" */
            (void)dec_stat(A_CON, 15 + randint(10), perm);
        }
    }

    /* Update stuff */
    p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

    /* Redraw mana */
    p_ptr->redraw |= (PR_MANA);
}

