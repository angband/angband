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
 * Determine if a spell is "okay" for the player to cast
 * A spell must be legible, learned, and not forgotten
 */
static bool spell_okay(int j)
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

        return (FALSE);
    }

    /* Spell is learned */
    if ((j < 32) ?
        (spell_learned1 & (1L << j)) :
        (spell_learned2 & (1L << (j - 32)))) {

        return (TRUE);
    }

    /* Assume unknown */
    return (FALSE);
}



/*
 * Determine if a spell is "okay" for the player to study
 * A spell must be legible, learnable, and not learned.
 */
static bool spell_okay_study(int j)
{
    magic_type *s_ptr;

    /* Access the spell */
    s_ptr = &mp_ptr->info[j];

    /* Spell is illegible or illegal */
    if (s_ptr->slevel > p_ptr->lev) return (FALSE);

    /* Spell is forgotten */
    if ((j < 32) ?
        (spell_forgotten1 & (1L << j)) :
        (spell_forgotten2 & (1L << (j - 32)))) {

        return (FALSE);
    }

    /* Spell is learned */
    if ((j < 32) ?
        (spell_learned1 & (1L << j)) :
        (spell_learned2 & (1L << (j - 32)))) {

        return (FALSE);
    }

    /* Assume okay */
    return (TRUE);
}


/*
 * Extra information on a spell		-DRS-
 * We can use up to 14 characters of the buffer 'p'
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
            case 43: strcpy(p, " dam 300"); break;
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
            case 1: strcpy(p, " heal 2d8"); break;
            case 9: sprintf(p, " range %d", 3*plev); break;
            case 10: strcpy(p, " heal 4d8"); break;
            case 17: sprintf(p, " %d+3d6", plev + orb); break;
            case 18: strcpy(p, " heal 6d8"); break;
            case 20: sprintf(p, " dur %d+d25", 3*plev); break;
            case 23: strcpy(p, " heal 8d8"); break;
            case 26: sprintf(p, " dam d%d", 3*plev); break;
            case 27: strcpy(p, " heal 300"); break;
            case 28: sprintf(p, " dam d%d", 3*plev); break;
            case 30: strcpy(p, " heal 1000"); break;
            case 36: strcpy(p, " heal 4d8"); break;
            case 37: strcpy(p, " heal 8d8"); break;
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
static void print_spells(int *spell, int num)
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
            sprintf(out_val, "  %c) %-30s", 'a' + i, "(illegible)");
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
                'a' + i, spell_names[mp_ptr->spell_type][j],
                s_ptr->slevel, s_ptr->smana, spell_chance(j), comment);
        prt(out_val, 2 + i, col);
    }

    /* Clear the bottom line */
    prt("", 2 + i, col);
}




/*
 * Hack -- Print a list of spells in the choice window.
 * See "print_spells()" for the basic algorithm.
 */
static void choice_spell(int *spell, int num)
{
    int			i, j;

    magic_type		*s_ptr;

    cptr		comment;

    char		info[80];

    char		out_val[160];


    /* In-active */
    if (!use_choice_win || !term_choice) return;


    /* Activate the choice window */
    Term_activate(term_choice);

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
            sprintf(out_val, "%c) %-30s", 'a' + i, "(illegible)");
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
                'a' + i, spell_names[mp_ptr->spell_type][j],
                s_ptr->slevel, s_ptr->smana, spell_chance(j), comment);
        Term_putstr(0, i, -1, TERM_WHITE, out_val);
    }


    /* Refresh */
    Term_fresh();

    /* Activate the main screen */
    Term_activate(term_screen);
}





/*
 * Note -- never call this function for warriors!
 *
 * Allow user to choose a spell from the given book.
 *
 * If a valid spell is chosen, saves it in '*sn' and returns TRUE
 * If the user hits escape, returns FALSE, and set '*sn' to -1
 * If there are no legal choices, returns FALSE, and sets '*sn' to -2
 */
static int cast_spell(cptr prompt, int item_val, int *sn)
{
    int			i, j = -1;
    int			spell[64], num = 0;
    bool		flag, redraw, okay, ask;
    char		choice;
    u32b		j1, j2;
    inven_type		*i_ptr;
    magic_type		*s_ptr;

    char		out_val[160];

    cptr p = ((mp_ptr->spell_book == TV_MAGIC_BOOK) ? "spell" : "prayer");


    /* Get the spell book */
    i_ptr = &inventory[item_val];

    /* Observe spells in that book */
    j1 = spell_flags[mp_ptr->spell_type][i_ptr->sval][0];
    j2 = spell_flags[mp_ptr->spell_type][i_ptr->sval][1];

    /* Extract spells */
    while (j1) spell[num++] = bit_pos(&j1);
    while (j2) spell[num++] = bit_pos(&j2) + 32;


    /* Assume no usable spells */
    okay = FALSE;

    /* Assume no spells available */
    (*sn) = -2;

    /* Check for known spells */
    for (i = 0; i < num; i++) {

        /* Look for "usable" spells */
        if (spell_okay(spell[i])) okay = TRUE;
    }

    /* No usable spells */
    if (!okay) return (FALSE);


    /* Assume cancelled */
    *sn = (-1);

    /* Nothing chosen yet */
    flag = FALSE;

    /* No redraw yet */
    redraw = FALSE;


    /* Use the "choice" window */
    if (choice_show_spells) {

        /* Display the choices */
        choice_spell(spell, num);

        /* Fix the choice window later */
        p_ptr->redraw |= (PR_CHOICE);
    }


    /* Build a prompt (accept all spells) */
    sprintf(out_val, "(Spells %c-%c, *=List, <ESCAPE>=exit) %s",
            'a', 'a' + num - 1, prompt);

    /* Get a spell from the user */
    while (!flag && get_com(out_val, &choice)) {

        /* Request redraw */
        if ((choice == ' ') || (choice == '*') || (choice == '?')) {

            /* only do this drawing once */
            if (!redraw) {

                /* Save the screen */
                save_screen();

                /* Remember to fix it */
                redraw = TRUE;

                /* Display a list of spells */
                print_spells(spell, num);
            }

            /* Ask again */
            continue;
        }


        /* Note verify */
        ask = (isupper(choice));

        /* Extract request */
        i = ask ? (choice - 'A') : (choice - 'a');

        /* Totally Illegal */
        if ((i < 0) || (i >= num)) {
            bell();
            continue;
        }

        /* Save the spell index */
        j = spell[i];

        /* Check for illegal spell */
        if (!spell_okay(j)) {
            msg_format("You don't know that %s.", p);
            continue;
        }

        /* Verify it */
        if (ask) {

            char tmp_val[160];
            
            /* Access the spell */
            s_ptr = &mp_ptr->info[j];

            /* Prompt */
            sprintf(tmp_val, "Cast %s (%d mana, %d%% fail)?",
                    spell_names[mp_ptr->spell_type][j], s_ptr->smana,
                    spell_chance(j));

            /* Belay that order */
            if (!get_check(tmp_val)) continue;
        }

        /* Stop the loop */
        flag = TRUE;	
    }

    /* Restore the screen */
    if (redraw) restore_screen();


    /* Abort if needed */
    if (!flag) return (FALSE);


    /* Access the spell */
    s_ptr = &mp_ptr->info[j];

    /* Verify if needed */
    if (s_ptr->smana > p_ptr->csp) {

        cptr q;

        /* Allow cancel */
        if (mp_ptr->spell_book == TV_MAGIC_BOOK) {
            q = "You summon your limited strength to cast this one! Confirm?";
            if (!get_check(q)) return (FALSE);
        }

        /* Allow cancel */
        if (mp_ptr->spell_book == TV_PRAYER_BOOK) {
            q = "The gods may think you presumptuous for this! Confirm?";
            if (!get_check(q)) return (FALSE);
        }
    }

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
    int			item_val;
    int			spell[64], num = 0;
    u32b		j1, j2;
    inven_type		*i_ptr;


    /* This command is free */
    energy_use = 0;

    if (p_ptr->pclass == 0) {
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


    /* Restrict choices to "useful" books */
    item_tester_tval = mp_ptr->spell_book;
    
    /* Get a book or stop checking */
    if (!get_item(&item_val, "Browse which Book?", 0, inven_ctr - 1, FALSE)) {
        if (item_val == -2) msg_print("You have no books that you can read.");
        return;
    }


    /* Access the book */
    i_ptr = &inventory[item_val];

    /* Obtain all spells in the book */
    j1 = spell_flags[mp_ptr->spell_type][i_ptr->sval][0];
    j2 = spell_flags[mp_ptr->spell_type][i_ptr->sval][1];

    /* Build spell list */
    while (j1) spell[num++] = bit_pos(&j1);
    while (j2) spell[num++] = bit_pos(&j2) + 32;


    /* Display the spells */
    save_screen();

    /* Display the spells */
    print_spells(spell, num);

    /* Wait for it */
    pause_line(0);

    /* Fix the screen */
    restore_screen();
}




/*
 * Study a book to gain some spells.
 */
void do_cmd_study(void)
{
    int			i, k, j, item_val;

    int			spell[64], num = 0;

    int			study = -1;

    char		query;
        
    u32b		j1, j2;

    inven_type		*i_ptr;

    cptr p = ((mp_ptr->spell_book == TV_MAGIC_BOOK) ? "spell" : "prayer");


    /* This command is free */
    energy_use = 0;

    if (p_ptr->pclass == 0) {
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
    
    /* Get a book or stop checking */
    if (!get_item(&item_val, "Study which Book?", 0, inven_ctr - 1, FALSE)) {
        if (item_val == -2) msg_print("You have no books that you can read.");
        return;
    }

    /* Access the book */
    i_ptr = &inventory[item_val];


    /* Obtain all spells in the book */
    j1 = spell_flags[mp_ptr->spell_type][i_ptr->sval][0];
    j2 = spell_flags[mp_ptr->spell_type][i_ptr->sval][1];

    /* Build spell list */
    while (j1) spell[num++] = bit_pos(&j1);
    while (j2) spell[num++] = bit_pos(&j2) + 32;


    /* Count "learnable" spells */
    for (k = i = 0; i < num; i++) {

        /* Count usable spells */
        if (spell_okay_study(spell[i])) k++;
    }
    
    /* No learnable spells */
    if (!k) {
        msg_format("You cannot learn any %ss in that book.", p);
        return;
    }


    /* Take a turn */
    energy_use = 0;

    /* Mage-spells */
    if (mp_ptr->spell_book == TV_MAGIC_BOOK) {

        /* Save the screen */
        save_screen();

        /* Display the spells */
        print_spells(spell, num);
        
        /* Get a response */
        while (TRUE) {
        
            /* Let player choose a spell */
            if (!get_com("Study which spell? ", &query)) break;

            /* Extract request */
            i = (query - 'a');

            /* Totally Illegal */
            if ((i < 0) || (i >= num)) {
                bell();
                continue;
            }

            /* Check for illegal spell */
            if (!spell_okay_study(spell[i])) {
                bell();
                continue;
            }

            /* Study that spell */
            study = spell[i];

            /* Done */
            break;
        }

        /* Restore the screen */
        restore_screen();
    }

    /* Priest spells */
    if (mp_ptr->spell_book == TV_PRAYER_BOOK) {

        /* Learn a single prayer */
        while (TRUE) {

            /* Pick a spell to learn */
            i = rand_int(num);

            /* Check for illegal spell */
            if (!spell_okay_study(spell[i])) continue;

            /* Learn it */
            study = spell[i];
            
            /* Done */
            break;
        }
    }

    /* Learn a spell */
    if (study >= 0) {
 
        /* Access the spell */
        j = study;

        /* Add the spell */
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
            msg_format("You can learn some more %ss.", p);
        }

        /* Update the mana */
        p_ptr->update |= (PU_MANA);

        /* Redraw Study Status */
        p_ptr->redraw |= (PR_STUDY);
    }
}



/*
 * Throw a magic spell					-RAK-	
 *
 * Note that the "beam" chance is now based on "plev" and not "intelligence".
 * This will make it less common early in the game and more common later.
 */
void do_cmd_cast(void)
{
    int			item_val, j, dir;
    int			chance, beam;
    int			plev = p_ptr->lev;
    
    magic_type   *s_ptr;

    energy_use = 0;

    if (mp_ptr->spell_book != TV_MAGIC_BOOK) {
        msg_print("You cannot cast spells!");
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


    /* Restrict choices to spell books */
    item_tester_tval = mp_ptr->spell_book;
    
    /* Get a spell book */
    if (!get_item(&item_val, "Use which Spell Book? ", 0, inven_ctr-1, FALSE)) {
        if (item_val == -2) msg_print("You are not carrying any spell-books!");
        return;
    }


    /* Ask for a spell */
    if (!cast_spell("Cast which spell?", item_val, &j)) {
        if (j == -2) msg_print("You don't know any spells in that book.");
        return;
    }


    /* Access the spell */
    s_ptr = &mp_ptr->info[j];

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
        switch (j + 1) {

          case 1:
            if (!get_dir(NULL, &dir)) return;
            fire_bolt_or_beam(beam-10, GF_MISSILE, dir,
                              damroll(3 + ((plev - 1) / 5), 4));
            break;

          case 2:
            (void)detect_monsters();
            break;

          case 3:
            teleport_flag = TRUE;
            teleport_dist = 10;
            break;

          case 4:
            (void)lite_area(damroll(2, (plev / 2)), (plev / 10) + 1);
            break;

          case 5:	   /* treasure detection */
            (void)detect_treasure();
            break;

          case 6:
            (void)hp_player(damroll(2, 8));
            if (p_ptr->cut > 15) {
                p_ptr->cut -= 15;
            }
            else {
                p_ptr->cut = 0;
            }
            break;

          case 7:
            (void)detect_object();
            break;

          case 8:
            (void)detect_sdoor();
            (void)detect_trap();
            break;

          case 9:
            if (!get_dir(NULL, &dir)) return;
            fire_ball(GF_POIS, dir,
                      10 + (plev / 2), 2);
            break;

          case 10:
            if (!get_dir(NULL, &dir)) return;
            (void)confuse_monster(dir, plev);
            break;

          case 11:
            if (!get_dir(NULL, &dir)) return;
            fire_bolt_or_beam(beam-10, GF_ELEC, dir,
                              damroll(3+((plev-5)/4),8));
            break;

          case 12:
            (void)destroy_doors_touch();
            break;

          case 13:
            if (!get_dir(NULL, &dir)) return;
            (void)sleep_monster(dir);
            break;

          case 14:
            (void)cure_poison();
            break;

          case 15:
            teleport_flag = TRUE;
            teleport_dist = plev * 5;
            break;

          case 16:
            if (!get_dir(NULL, &dir)) return;
            msg_print("A line of blue shimmering light appears.");
            lite_line(dir);
            break;

          case 17:
            if (!get_dir(NULL, &dir)) return;
            fire_bolt_or_beam(beam-10, GF_COLD, dir,
                              damroll(5+((plev-5)/4),8));
            break;

          case 18:
            if (!get_dir(NULL, &dir)) return;
            (void)wall_to_mud(dir);
            break;

          case 19:
            satisfy_hunger();
            break;

          case 20:
            (void)recharge(5);
            break;

          case 21:
            (void)sleep_monsters_touch();
            break;

          case 22:
            if (!get_dir(NULL, &dir)) return;
            (void)poly_monster(dir);
            break;

          case 23:
            if (ident_spell()) combine_pack();
            break;

          case 24:
            (void)sleep_monsters();
            break;

          case 25:
            if (!get_dir(NULL, &dir)) return;
            fire_bolt_or_beam(beam, GF_FIRE, dir,
                              damroll(8+((plev-5)/4),8));
            break;

          case 26:
            if (!get_dir(NULL, &dir)) return;
            (void)slow_monster(dir);
            break;

          case 27:
            if (!get_dir(NULL, &dir)) return;
            fire_ball(GF_COLD, dir,
                      30 + (plev), 2);
            break;

          case 28:
            (void)recharge(40);
            break;

          case 29:
            if (!get_dir(NULL, &dir)) return;
            (void)teleport_monster(dir);
            break;

          case 30:
            if (!p_ptr->fast) {
                add_fast(randint(20) + plev);
            }
            else {
                add_fast(randint(5));
            }
            break;

          case 31:
            if (!get_dir(NULL, &dir)) return;
            fire_ball(GF_FIRE, dir,
                      55 + (plev), 2);
            break;

          case 32:
            destroy_area(py, px, 15, TRUE);
            break;

          case 33:
            (void)genocide(TRUE);
            break;

          case 34:	   /* door creation */
            (void)door_creation();
            break;

          case 35:	   /* Stair creation */
            (void)stair_creation();
            break;

          case 36:	   /* Teleport level */
            (void)tele_level();
            break;

          case 37:	   /* Earthquake */
            earthquake(py, px, 10);
            break;

          case 38:	   /* Word of Recall */
            if (p_ptr->word_recall == 0) {
                p_ptr->word_recall = rand_int(20) + 15;
                msg_print("The air about you becomes charged...");
            }
            else {
                p_ptr->word_recall = 0;
                msg_print("A tension leaves the air around you...");
            }
            break;

          case 39:	   /* Acid Bolt */
            if (!get_dir(NULL, &dir)) return;
            fire_bolt_or_beam(beam, GF_ACID, dir,
                              damroll(6+((plev-5)/4), 8));
            break;

          case 40:	   /* Cloud kill */
            if (!get_dir(NULL, &dir)) return;
            fire_ball(GF_POIS, dir,
                      20 + (plev / 2), 3);
            break;

          case 41:	   /* Acid Ball */
            if (!get_dir(NULL, &dir)) return;
            fire_ball(GF_ACID, dir,
                      40 + (plev), 2);
            break;

          case 42:	   /* Ice Storm */
            if (!get_dir(NULL, &dir)) return;
            fire_ball(GF_COLD, dir,
                      70 + (plev), 3);
            break;

          case 43:	   /* Meteor Swarm */
            if (!get_dir(NULL, &dir)) return;
            fire_ball(GF_METEOR, dir,
                      65 + (plev), 3);
            break;

          case 44:	   /* Mana Storm */
            if (!get_dir(NULL, &dir)) return;
            fire_ball(GF_MANA, dir, 300, 2);
            break;

          case 45:	   /* Detect Evil */
            (void)detect_evil();
            break;

          case 46:	   /* Detect Enchantment */
            (void)detect_magic();
            break;

          case 47:
            recharge(100);
            break;

          case 48:
            (void)genocide(TRUE);
            break;

          case 49:
            (void)mass_genocide(TRUE);
            break;

          case 50:
            p_ptr->oppose_fire += randint(20) + 20;
            break;

          case 51:
            p_ptr->oppose_cold += randint(20) + 20;
            break;

          case 52:
            p_ptr->oppose_acid += randint(20) + 20;
            break;

          case 53:
            p_ptr->oppose_pois += randint(20) + 20;
            break;

          case 54:
            p_ptr->oppose_fire += randint(20) + 20;
            p_ptr->oppose_cold += randint(20) + 20;
            p_ptr->oppose_elec += randint(20) + 20;
            p_ptr->oppose_pois += randint(20) + 20;
            p_ptr->oppose_acid += randint(20) + 20;
            break;

          case 55:
            hp_player(10);	/* XXX */
            p_ptr->hero += randint(25) + 25;
            break;

          case 56:
            p_ptr->shield += randint(20) + 30;
            msg_print("A mystic shield forms around your body!");
            break;

          case 57:
            hp_player(30);	/* XXX */
            p_ptr->shero += randint(25) + 25;
            break;

          case 58:
            if (!p_ptr->fast) {
                add_fast(randint(30) + 30 + plev);
            }
            else {
                add_fast(randint(10));
            }
            break;

          case 59:
            p_ptr->invuln += randint(8) + 8;
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
    
        /* No mana left */
        p_ptr->csp = 0;
        p_ptr->csp_frac = 0;

        /* Message */
        msg_print("You faint from the effort!");

        /* Hack -- Bypass free action */
        p_ptr->paralysis = randint(5 * (int)(s_ptr->smana - p_ptr->csp));

        /* Damage CON (possibly permanently) */
        if (rand_int(3) == 0) {
            msg_print("You have damaged your health!");
            (void)dec_stat(A_CON, 15 + randint(10), (rand_int(3) == 0));
        }
    }

    /* Update stuff */
    p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
    
    /* Redraw mana */
    p_ptr->redraw |= (PR_MANA);
}



/*
 * Pray
 *
 * Note that "Holy Orb" damage is now based on "plev" and not "wisdom".
 * This will make it weaker early in the game and stronger later.
 */
void do_cmd_pray(void)
{
    int item_val, j, dir, chance;

    magic_type  *s_ptr;
    inven_type   *i_ptr;

    int plev = p_ptr->lev;
    
    energy_use = 0;

    if (mp_ptr->spell_book != TV_PRAYER_BOOK) {
        msg_print("Pray hard enough and your prayers may be answered.");
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


    /* Restrict choices */
    item_tester_tval = mp_ptr->spell_book;
    
    /* Choose a book */
    if (!get_item(&item_val, "Use which Holy Book? ", 0, inven_ctr - 1, FALSE)) {
        if (item_val == -2) msg_print("You are not carrying any prayer books!");
        return;
    }


    /* Choose a spell */
    if (!cast_spell("Recite which prayer?", item_val, &j)) {
        if (j == -2) msg_print("You don't know any prayers in that book.");
        return;
    }


    /* Access the spell */
    s_ptr = &mp_ptr->info[j];

    /* Spell failure chance */
    chance = spell_chance(j);

    /* Check for failure */
    if (rand_int(100) < chance) {
        if (flush_failure) flush();
        msg_print("You failed to concentrate hard enough!");
    }

    /* Success */
    else {

        switch (j + 1) {

          case 1:
            (void)detect_evil();
            break;

          case 2:
            (void)hp_player(damroll(2, 8));
            if (p_ptr->cut > 10) {
                p_ptr->cut -= 10;
            }
            else {
                p_ptr->cut = 0;
            }
            break;

          case 3:
            (void)add_bless(randint(12) + 12);
            break;

          case 4:
            (void)cure_fear();
            break;

          case 5:
            (void)lite_area(damroll(2, (plev / 2)), (plev / 10) + 1);
            break;

          case 6:
            (void)detect_trap();
            break;

          case 7:
            (void)detect_sdoor();
            break;

          case 8:
            if (p_ptr->poisoned) {
                msg_print("The effect of the poison has been reduced.");
                p_ptr->poisoned = (p_ptr->poisoned + 1) / 2;
            }
            break;

          case 9:
            if (!get_dir(NULL, &dir)) return;
            (void)fear_monster(dir, plev);
            break;

          case 10:
            teleport_flag = TRUE;
            teleport_dist = plev * 3;
            break;

          case 11:
            (void)hp_player(damroll(4, 8));
            if (p_ptr->cut > 40) {
                p_ptr->cut = (p_ptr->cut / 2) - 20;
            }
            else {
                p_ptr->cut = 0;
            }
            break;

          case 12:
            add_bless(randint(24) + 24);
            break;

          case 13:
            (void)sleep_monsters_touch();
            break;

          case 14:
            satisfy_hunger();
            break;

          case 15:
            remove_curse();
            break;

          case 16:
            p_ptr->oppose_fire += randint(10) + 10;
            p_ptr->oppose_cold += randint(10) + 10;
            break;

          case 17:
            (void)cure_poison();
            break;

          case 18:
            if (!get_dir(NULL, &dir)) return;
            fire_ball(GF_HOLY_ORB, dir,
                      (damroll(3,6) + plev +
                       (plev / ((p_ptr->pclass == 2) ? 2 : 4))),
                      ((plev < 30) ? 2 : 3));
            break;

          case 19:
            (void)hp_player(damroll(6, 8));
            p_ptr->cut = 0;
            break;

          case 20:
            add_tim_invis(randint(24) + 24);
            break;

          case 21:
            (void)protect_evil();
            break;

          case 22:
            earthquake(py, px, 10);
            break;

          case 23:
            map_area();
            break;

          case 24:
            (void)hp_player(damroll(8, 8));
            p_ptr->cut = 0;
            p_ptr->stun = 0;
            break;

          case 25:
            (void)turn_undead();
            break;

          case 26:
            add_bless(randint(48) + 48);
            break;

          case 27:
            (void)dispel_undead(plev * 3);
            break;

          case 28:
            (void)hp_player(300);
            p_ptr->stun = 0;
            p_ptr->cut = 0;
            break;

          case 29:
            (void)dispel_evil(plev * 3);
            break;

          case 30:
            warding_glyph();
            break;

          case 31:
            (void)dispel_evil(plev * 4);
            (void)hp_player(1000);
            (void)cure_fear();
            (void)cure_poison();
            p_ptr->stun = 0;
            p_ptr->cut = 0;
            break;

          case 32:
            (void)detect_monsters();
            break;

          case 33:
            (void)detection();
            break;

          case 34:
            if (ident_spell()) combine_pack();
            break;

          case 35:	   /* probing */
            (void)probing();
            break;

          case 36:	   /* Clairvoyance */
            wiz_lite();
            break;

          case 37:
            (void)hp_player(damroll(4, 8));
            p_ptr->cut = 0;
            break;

          case 38:
            (void)hp_player(damroll(8, 8));
            p_ptr->cut = 0;
            p_ptr->stun = 0;
            break;

          case 39:
            (void)hp_player(2000);
            p_ptr->stun = 0;
            p_ptr->cut = 0;
            break;

          case 40:	   /* restoration */
            if (res_stat(A_STR)) {
                msg_print("You feel warm all over.");
            }
            if (res_stat(A_INT)) {
                msg_print("You have a warm feeling.");
            }
            if (res_stat(A_WIS)) {
                msg_print("You feel your wisdom returning.");
            }
            if (res_stat(A_DEX)) {
                msg_print("You feel less clumsy.");
            }
            if (res_stat(A_CON)) {
                msg_print("You feel your health returning!");
            }
            if (res_stat(A_CHR)) {
                msg_print("You feel your looks returning.");
            }
            break;

          case 41:	   /* rememberance */
            (void)restore_level();
            break;

          case 42:	   /* dispel undead */
            (void)dispel_undead(plev * 4);
            break;

          case 43:	   /* dispel evil */
            (void)dispel_evil(plev * 4);
            break;

          case 44:	   /* banishment */
            if (banish_evil(100)) {
                msg_print("The Power of your god banishes evil!");
            }
            break;

          case 45:	   /* word of destruction */
            destroy_area(py, px, 15, TRUE);
            break;

          case 46:	   /* annihilation */
            if (!get_dir(NULL, &dir)) return;
            drain_life(dir, 200);
            break;

          case 47:	   /* unbarring ways */
            (void)destroy_doors_touch();
            break;

          case 48:	   /* recharging */
            (void)recharge(15);
            break;

          case 49:	   /* remove (all) curses */
            (void)remove_all_curse();
            break;

          case 50:	   /* enchant weapon */
            (void)enchant_spell(rand_int(4) + 1, rand_int(4) + 1, 0);
            break;

          case 51:	   /* enchant armor */
            (void)enchant_spell(0, 0, rand_int(3) + 2);
            break;

          /* Elemental brand -- only wielded weapon */
          case 52:

            i_ptr = &inventory[INVEN_WIELD];

            /* you can never modify artifacts / ego-items */
            /* you can never modify broken / cursed items */
            if ((i_ptr->k_idx) &&
                (!artifact_p(i_ptr)) && (!ego_item_p(i_ptr)) &&
                (!broken_p(i_ptr)) && (!cursed_p(i_ptr))) {

                cptr act = NULL;

                char i_name[80];

                if (rand_int(2)) {
                    act = "is covered in a fiery shield!";
                    i_ptr->name2 = EGO_FT;
                    i_ptr->flags1 |= (TR1_BRAND_FIRE);
                    i_ptr->flags2 |= (TR2_RES_FIRE);
                    i_ptr->flags3 |= (TR3_IGNORE_FIRE);
                }
                else {
                    act = "glows deep, icy blue!";
                    i_ptr->name2 = EGO_FB;
                    i_ptr->flags1 |= (TR1_BRAND_COLD);
                    i_ptr->flags2 |= (TR2_RES_COLD);
                    i_ptr->flags3 |= (TR3_IGNORE_COLD);
                }

                objdes(i_name, i_ptr, FALSE, 0);

                msg_format("Your %s %s", i_name, act);

                enchant(i_ptr, rand_int(3) + 4, ENCH_TOHIT|ENCH_TODAM);
            }
            else {
                if (flush_failure) flush();
                msg_print("The Branding failed.");
            }
            break;

          case 53:	   /* blink */
            teleport_flag = TRUE;
            teleport_dist = 10;
            break;

          case 54:	   /* teleport */
            teleport_flag = TRUE;
            teleport_dist = plev * 8;
            break;

          case 55:	   /* teleport away */
            if (!get_dir(NULL, &dir)) return;
            (void)teleport_monster(dir);
            break;

          case 56:	   /* teleport level */
            (void)tele_level();
            break;

          case 57:	   /* word of recall */
            if (p_ptr->word_recall == 0) {
                p_ptr->word_recall = rand_int(20) + 15;
                msg_print("The air about you becomes charged...");
            }
            else {
                p_ptr->word_recall = 0;
                msg_print("A tension leaves the air around you...");
            }
            break;

          case 58:	   /* alter reality */
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
    
        /* No mana left */
        p_ptr->csp = 0;
        p_ptr->csp_frac = 0;

        /* Message */
        msg_print("You faint from the effort!");

        /* Hack -- Bypass free action */
        p_ptr->paralysis = randint(5 * (int)(s_ptr->smana - p_ptr->csp));

        /* Damage CON (possibly permanently) */
        if (rand_int(3) == 0) {
            msg_print("You have damaged your health!");
            (void)dec_stat(A_CON, 15 + randint(10), (rand_int(3) == 0));
        }
    }

    /* Update stuff */
    p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
    
    /* Redraw mana */
    p_ptr->redraw |= (PR_MANA);
}

