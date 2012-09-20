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
 * Returns spell chance of failure for spell		-RAK-	
 */
int spell_chance(int spell)
{
    int         chance, minfail;
    spell_type *s_ptr;

    /* Paranoia -- warriors */
    if (p_ptr->pclass == 0) return (100);

    /* Access the spell */
    s_ptr = &magic_spell[p_ptr->pclass-1][spell];

    /* Extract the base spell failure rate */
    chance = s_ptr->sfail - 3 * (p_ptr->lev - s_ptr->slevel);

    /* Adjust for primary spell stat */
    chance -= 3 * (stat_adj(cp_ptr->spell_stat) - 1);

    /* Not enough mana to cast */
    if (s_ptr->smana > p_ptr->cmana) {
        chance += 5 * (s_ptr->smana - p_ptr->cmana);
    }

    /* Extract the minimum failure rate */
    switch (stat_adj(cp_ptr->spell_stat)) {
      case 0:
        minfail = 50;
        break;
      case 1:
        minfail = 12;
        break;			   /* 8-14 stat */
      case 2:
        minfail = 8;
        break;			   /* 15-17 stat */
      case 3:
        minfail = 5;
        break;			   /* 18-18/49 stat */
      case 4:
        minfail = 4;
        break;			   /* 18/50-18/69 */
      case 5:
        minfail = 4;
        break;			   /* 18/70-18/89 */
      case 6:
        minfail = 3;
        break;			   /* 18/90-18/99 */
      case 7:
        minfail = 3;
        break;			   /* 18/100 */
      case 8:
      case 9:
      case 10:
        minfail = 2;
        break;			   /* 18/101 - 18/130 */
      case 11:
      case 12:
        minfail = 2;
        break;			   /* 18/131 - 18/150 */
      case 13:
      case 14:
        minfail = 1;
        break;			   /* 18/151 - 18/170 */
      case 15:
      case 16:
        minfail = 1;
        break;			   /* 18/171 - 18/200 */
      default:
        minfail = 0;
        break;			   /* 18/200 - 18/etc */
    }

    /* Non mage/priest characters never get too good */
    if ((p_ptr->pclass != 1) && (p_ptr->pclass != 2)) {
        if (minfail < 5) minfail = 5;
    }

    /* Big priest prayer penalty for edged weapons  -DGK */
    /* XXX XXX So, like, switch weapons and then pray? */
    if (p_ptr->pclass == 2) {
        inven_type *i_ptr;
        i_ptr = &inventory[INVEN_WIELD];
        if ((i_ptr->tval == TV_SWORD) || (i_ptr->tval == TV_POLEARM)) {
            if (!(i_ptr->flags3 & TR3_BLESSED)) {
                chance += 25;
            }
        }
    }

    /* Minimum failure rate */
    if (chance < minfail) chance = minfail;

    /* Stunning makes spells harder */
    if (p_ptr->stun > 50) chance += 25;
    else if (p_ptr->stun) chance += 15;

    /* Always a 5 percent chance of working */
    if (chance > 95) chance = 95;

    /* Return the chance */
    return (chance);
}



/*
 * Determine if a spell is "okay" for the player to cast
 * A spell must be legible, learned, and not forgotten
 */
static bool spell_okay(int j)
{
    spell_type *s_ptr;

    /* Access the spell */
    s_ptr = &magic_spell[p_ptr->pclass-1][j];

    /* Spell is illegal */
    if (s_ptr->slevel > p_ptr->lev) return (FALSE);

    /* Spell is forgotten */
    if (j < 32) {
        if (spell_forgotten1 & (1L << j)) return (FALSE);
    }
    else {
        if (spell_forgotten2 & (1L << (j - 32))) return (FALSE);
    }

    /* Spell is learned */
    if (j < 32) {
        if (spell_learned1 & (1L << j)) return (TRUE);
    }
    else {
        if (spell_learned2 & (1L << (j - 32))) return (TRUE);
    }

    /* Assume unknown */
    return (FALSE);
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
    if (cp_ptr->spell_stat == A_INT) {

        int lev = p_ptr->lev;

        /* Analyze the spell */
        switch (j) {
            case 0: sprintf(p, " dam %dd4", 3+((lev-1)/5)); break;
            case 2: strcpy(p, " range 10"); break;
            case 5: strcpy(p, " heal 4d4"); break;
            case 8: sprintf(p, " dam %d", 10 + (lev / 2)); break;
            case 10: sprintf(p, " dam %dd8", (3+((lev-5)/4))); break;
            case 14: sprintf(p, " range %d", lev * 10); break;
            case 15: strcpy(p," dam 6d8"); break;
            case 16: sprintf(p, " dam %dd8", (5+((lev-5)/4))); break;
            case 24: sprintf(p, " dam %dd8", (8+((lev-5)/4))); break;
            case 26: sprintf(p, " dam %d", 30 + lev); break;
            case 29: sprintf(p, " dur %d+d20", lev); break;
            case 30: sprintf(p, " dam %d", 55 + lev); break;
            case 38: sprintf(p, " dam %dd8", (6+((lev-5)/4))); break;
            case 39: sprintf(p, " dam %d", 40 + lev/2); break;
            case 40: sprintf(p, " dam %d", 40 + lev); break;
            case 41: sprintf(p, " dam %d", 70 + lev); break;
            case 42: sprintf(p, " dam %d", 65 + lev); break;
            case 43: strcpy(p, " dam 300"); break;
            case 49: strcpy(p, " dur 20+d20"); break;
            case 50: strcpy(p, " dur 20+d20"); break;
            case 51: strcpy(p, " dur 20+d20"); break;
            case 52: strcpy(p, " dur 20+d20"); break;
            case 53: strcpy(p, " dur 20+d20"); break;
            case 54: strcpy(p, " dur 25+d25"); break;
            case 55: strcpy(p, " dur 30+d20"); break;
            case 56: strcpy(p, " dur 25+d25"); break;
            case 57: sprintf(p, " dur %d+d25", 30+lev); break;
            case 58: strcpy(p, " dur 6+d8"); break;
        }
    }

    /* Priest spells */
    else {

        int lev = p_ptr->lev;

	int orb = ((p_ptr->pclass == 2) ? 2 : 1) * stat_adj(A_WIS);
	
        /* Analyze the spell */
        switch (j) {
            case 1: strcpy(p, " heal 3d3"); break;
            case 9: sprintf(p, " range %d", 3*lev); break;
            case 10: strcpy(p, " heal 4d4"); break;
            case 17: sprintf(p, " %d+3d6", lev + orb); break;
            case 18: strcpy(p, " heal 8d4"); break;
            case 20: sprintf(p, " dur %d+d25", 3*lev); break;
            case 23: strcpy(p, " heal 16d4"); break;
            case 26: sprintf(p, " dam d%d", 3*lev); break;
            case 27: strcpy(p, " heal 200"); break;
            case 28: sprintf(p, " dam d%d", 3*lev); break;
            case 30: strcpy(p, " heal 1000"); break;
            case 36: strcpy(p, " heal 8d4"); break;
            case 37: strcpy(p, " heal 16d4"); break;
            case 38: strcpy(p, " heal 1000"); break;
            case 41: sprintf(p, " dam d%d", 4*lev); break;
            case 42: sprintf(p, " dam d%d", 4*lev); break;
            case 45: strcpy(p, " dam 200"); break;
            case 52: strcpy(p, " range 10"); break;
            case 53: sprintf(p, " range %d", 8*lev); break;
        }
    }

#endif

}


/*
 * Print a list of spells (for browsing or casting)
 */
static void print_spells(int *spell, int num)
{
    int			i, j, col, index;

    spell_type		*s_ptr;

    cptr		comment;

    char		info[80];
    char		out_val[80];

    /* Print column */
    col = 20;

    /* Index into the spell name table */
    index = (cp_ptr->spell_stat == A_INT) ? 0 : 1;

    /* Title the list */
    prt("", 1, col);
    put_str("Name", 1, col + 5);
    put_str("Lv Mana Fail", 1, col + 35);

    /* Dump the spells */
    for (i = 0; i < num; i++) {

        /* Access the spell */
        j = spell[i];

        /* Access the spell */
        s_ptr = &magic_spell[p_ptr->pclass-1][j];

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
        if (j >= 32 ?
                 ((spell_forgotten2 & (1L << (j - 32)))) :
                 ((spell_forgotten1 & (1L << j)))) {
            comment = " forgotten";
        }
        else if (j >= 32 ?
                 (!(spell_learned2 & (1L << (j - 32)))) :
                 (!(spell_learned1 & (1L << j)))) {
            comment = " unknown";
        }
        else if (j >= 32 ?
                 (!(spell_worked2 & (1L << (j - 32)))) :
                 (!(spell_worked1 & (1L << j)))) {
            comment = " untried";
        }

        /* Dump the spell --(-- */
        sprintf(out_val, "  %c) %-30s%2d %4d %3d%%%s",
                'a' + i, spell_names[index][j],
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
    int			i, j, index;

    spell_type		*s_ptr;

    cptr		comment;

    char		info[80];
    char		out_val[80];


    /* In-active */
    if (!use_choice_win || !term_choice) return;


    /* Activate the choice window */
    Term_activate(term_choice);

    /* Clear it */
    Term_clear();


    /* Index into the spell name table */
    index = (cp_ptr->spell_stat == A_INT) ? 0 : 1;

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
        s_ptr = &magic_spell[p_ptr->pclass-1][j];

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
        if (j >= 32 ?
                 ((spell_forgotten2 & (1L << (j - 32)))) :
                 ((spell_forgotten1 & (1L << j)))) {
            comment = " forgotten";
        }
        else if (j >= 32 ?
                 (!(spell_learned2 & (1L << (j - 32)))) :
                 (!(spell_learned1 & (1L << j)))) {
            comment = " unknown";
        }
        else if (j >= 32 ?
                 (!(spell_worked2 & (1L << (j - 32)))) :
                 (!(spell_worked1 & (1L << j)))) {
            comment = " untried";
        }

        /* Dump the spell --(-- */
        sprintf(out_val, "%c) %-30s%2d %4d %3d%%%s",
                'a' + i, spell_names[index][j],
                s_ptr->slevel, s_ptr->smana, spell_chance(j), comment);
        Term_putstr(0, i, -1, TERM_WHITE, out_val);
    }


    /* Refresh */
    Term_fresh();

    /* Activate the main screen */
    Term_activate(term_screen);
}



/*
 * calculate number of spells player should have,
 * and learn/forget spells until that number is met -JEW-
 *
 * Use "bit_pos()" in this function...
 */
void calc_spells(int stat)
{
    int			i, k, j, index, levels;
    int			num_allowed, new_spells, num_known;

    u32b		mask, spell_flag;

    spell_type		*s_ptr;

    cptr		p;

    char		tmp_str[80];


    index = (cp_ptr->spell_stat == A_INT) ? 0 : 1;

    p = (cp_ptr->spell_stat == A_INT) ? "spell" : "prayer";


    /* XXX Hack -- this technically runs in the "wrong" order */

    /* Check to see if know any spells greater than level, eliminate them */
    for (i = 31; i >= 0; i--) {

        mask = 1L << i;

        if (mask & spell_learned1) {
            s_ptr = &magic_spell[p_ptr->pclass-1][i];
            if (s_ptr->slevel > p_ptr->lev) {
                spell_learned1 &= ~mask;
                spell_forgotten1 |= mask;
                (void)sprintf(tmp_str, "You have forgotten the %s of %s.", p,
                              spell_names[index][i]);
                msg_print(tmp_str);
            }
        }

        if (mask & spell_learned2) {
            s_ptr = &magic_spell[p_ptr->pclass-1][i+32];
            if (s_ptr->slevel > p_ptr->lev) {
                spell_learned2 &= ~mask;
                spell_forgotten2 |= mask;
                (void)sprintf(tmp_str, "You have forgotten the %s of %s.", p,
                              spell_names[index][i + 32]);
                msg_print(tmp_str);
            }
        }
    }


    /* Determine the number of spells allowed */
    levels = p_ptr->lev - cp_ptr->spell_first + 1;
    switch (stat_adj(stat)) {
      case 0:
        num_allowed = 0;
        break;
      case 1:
      case 2:
      case 3:
        num_allowed = 1 * levels;
        break;
      case 4:
      case 5:
        num_allowed = 3 * levels / 2;
        break;
      case 6:
        num_allowed = 2 * levels;
        break;
      default:
        num_allowed = 5 * levels / 2;
        break;
    }

    /* Count the number of spells we know */
    num_known = 0;
    for (i = 0; i < 32; i++) {
        mask = 1L << i;
        if (mask & spell_learned1) num_known++;
        if (mask & spell_learned2) num_known++;
    }

    /* See how many spells we must forget or may learn */
    new_spells = num_allowed - num_known;


    /* We can learn some forgotten spells */
    if (new_spells > 0) {

        /* Remember spells that were forgetten */
        for (i = 0; new_spells > 0; i++) {

            /* No spells left to remember */
            if (i >= 64) break;

            /* Not allowed to remember any more */
            if (i >= num_allowed) break;

            /* No more forgotten spells to remember */
            if (!spell_forgotten1 && !spell_forgotten2) break;

            /* Get the (i+1)th spell learned */
            j = spell_order[i];

            /* Don't process unknown spells... -CFT */
            if (j == 99) continue;

            /* First set of spells */
            if (j < 32) {
                mask = 1L << j;
                if (mask & spell_forgotten1) {
                    s_ptr = &magic_spell[p_ptr->pclass-1][j];
                    if (s_ptr->slevel <= p_ptr->lev) {
                        spell_forgotten1 &= ~mask;
                        spell_learned1 |= mask;
                        new_spells--;
                        sprintf(tmp_str, "You have remembered the %s of %s.",
                                p, spell_names[index][j]);
                        msg_print(tmp_str);
                    }
                    else {
                        /* if was too high lv to remember */
                        num_allowed++;
                    }
                }
            }

            /* Second set of spells */
            else {
                mask = 1L << (j - 32);
                if (mask & spell_forgotten2) {
                    s_ptr = &magic_spell[p_ptr->pclass-1][j];
                    if (s_ptr->slevel <= p_ptr->lev) {
                        spell_forgotten2 &= ~mask;
                        spell_learned2 |= mask;
                        new_spells--;
                        sprintf(tmp_str, "You have remembered the %s of %s.",
                                p, spell_names[index][j]);
                        msg_print(tmp_str);
                    }
                    else {
                        /* if was too high lv to remember */
                        num_allowed++;
                    }
                }
            }
        }
    }


    /* Learn some new spells */
    if (new_spells > 0) {

        /* Count the spells */
        k = 0;

        /* Count remaining learnable spells */
        spell_flag = 0xFFFFFFFFL & ~spell_learned1;
        while (spell_flag) {
            j = bit_pos(&spell_flag);
            s_ptr = &magic_spell[p_ptr->pclass-1][j];
            if (s_ptr->slevel <= p_ptr->lev) k++;
        }

        /* Count remaining learnable spells */
        spell_flag = 0xFFFFFFFFL & ~spell_learned2;
        while (spell_flag) {
            j = bit_pos(&spell_flag) + 32;
            s_ptr = &magic_spell[p_ptr->pclass-1][j];
            if (s_ptr->slevel <= p_ptr->lev) k++;
        }

        /* Cannot learn more spells than exist */
        if (new_spells > k) new_spells = k;
    }


    /* Forget spells */
    if (new_spells < 0) {

        /* Forget spells in the opposite order they were learned */
        for (i = 63; new_spells < 0; i--) {

            /* Hack -- we might run out of spells to forget */
            if (!spell_learned1 && !spell_learned2) {
                new_spells = 0;
                break;
            }

            /* Get the (i+1)th spell learned */
            j = spell_order[i];

            /* don't process unknown spells... -CFT */
            if (j == 99) continue;

            /* First set of spells */
            if (j < 32) {
                mask = 1L << j;
                if (mask & spell_learned1) {
                    spell_learned1 &= ~mask;
                    spell_forgotten1 |= mask;
                    new_spells++;
                    sprintf(tmp_str, "You have forgotten the %s of %s.",
                            p, spell_names[index][j]);
                    msg_print(tmp_str);
                }
            }

            /* Assume second set of spells */
            else {
                mask = 1L << (j - 32);
                if (mask & spell_learned2) {
                    spell_learned2 &= ~mask;
                    spell_forgotten2 |= mask;
                    new_spells++;
                    sprintf(tmp_str, "You have forgotten the %s of %s.",
                            p, spell_names[index][j]);
                    msg_print(tmp_str);
                }
            }
        }

        /* Paranoia -- in case we run out of spells to forget */
        new_spells = 0;
    }


    /* Take note when "study" changes */
    if (character_generated && (new_spells != p_ptr->new_spells)) {

        /* Player can learn new spells now */
        if ((new_spells > 0) && (p_ptr->new_spells == 0)) {
            (void)sprintf(tmp_str, "You can learn some new %ss now.", p);
            msg_print(tmp_str);
        }

        /* Save the new_spells value */
        p_ptr->new_spells = new_spells;

        /* Display "study state" later */
        p_ptr->redraw |= PR_STUDY;
    }
}


/*
 * Gain some mana if you know at least one spell	-RAK-	
 */
void calc_mana(int stat)
{
    int          new_mana, levels;
    s32b        value;
    int          i;
    inven_type  *i_ptr;
    int                   amrwgt, maxwgt;

    static int cumber_armor = FALSE;
    static int cumber_glove = FALSE;

    bool old_glove = cumber_glove;
    bool old_armor = cumber_armor;

    if (spell_learned1 || spell_learned2) {
        levels = p_ptr->lev - cp_ptr->spell_first + 1;
        switch (stat_adj(stat)) {
          case 0:
            new_mana = 0;
            break;
          case 1:
          case 2:
            new_mana = 1 * levels;
            break;
          case 3:
            new_mana = 3 * levels / 2;
            break;
          case 4:
            new_mana = 2 * levels;
            break;
          case 5:
            new_mana = 5 * levels / 2;
            break;
          case 6:
            new_mana = 3 * levels;
            break;
          case 7:
            new_mana = 4 * levels;
            break;
          case 8:
            new_mana = 9 * levels / 2;
            break;
          case 9:
            new_mana = 5 * levels;
            break;
          case 10:
            new_mana = 11 * levels / 2;
            break;
          case 11:
            new_mana = 6 * levels;
            break;
          case 12:
            new_mana = 13 * levels / 2;
            break;
          case 13:
            new_mana = 7 * levels;
            break;
          case 14:
            new_mana = 15 * levels / 2;
            break;
          default:
            new_mana = 8 * levels;
            break;
        }

        /* increment mana by one, so that first level chars have 2 mana */
        if (new_mana) new_mana++;


        /* Assume player is not encumbered by gloves */
        cumber_glove = FALSE;

        /* Get the gloves */
        i_ptr = &inventory[INVEN_HANDS];

        /* good gauntlets of dexterity or free action do not hurt spells */
        if (i_ptr->tval &&
            !((i_ptr->flags2 & TR2_FREE_ACT) ||
              ((i_ptr->flags1 & TR1_DEX) && (i_ptr->pval > 0)))) {

            /* Only mages are affected */
            if (p_ptr->pclass == 1 ||
                p_ptr->pclass == 3 ||
                p_ptr->pclass == 4) {

                cumber_glove = TRUE;
                new_mana = (3 * new_mana) / 4;
            }
        }

        if (cumber_glove && !old_glove) {
            msg_print("Your covered hands interfere with your spellcasting.");
        }

        if (old_glove && !cumber_glove) {
            msg_print("Your hands feel more suitable for spellcasting.");
        }


        /* Assume player not encumbered by armor */
        cumber_armor = FALSE;

        /* Weigh the armor */
        amrwgt = 0;
        for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
            i_ptr = &inventory[i];
            switch (i) {
              case INVEN_HEAD:
              case INVEN_BODY:
              case INVEN_ARM:
              case INVEN_HANDS:
              case INVEN_FEET:
              case INVEN_OUTER:
                amrwgt += i_ptr->weight;
            }
        }

        /* Determine the weight allowance */
        switch (p_ptr->pclass) {
          case 1:
            maxwgt = 300;
            break;
          case 2:
            maxwgt = 350;
            break;
          case 3:
            maxwgt = 350;
            break;
          case 4:
            maxwgt = 400;
            break;
          case 5:
            maxwgt = 400;
            break;
          default:
            maxwgt = 0;
        }

        /* Too much armor */
        if (amrwgt > maxwgt) {
            cumber_armor = TRUE;
            new_mana -= ((amrwgt - maxwgt) / 10);
        }

        if (cumber_armor && !old_armor) {
            msg_print("The weight of your armor encumbers your movement.");
        }

        if (old_armor && !cumber_armor) {
            msg_print("You feel able to move more freely.");
        }


    /*
     * if low int/wis, gloves, and lots of heavy armor, new_mana could be
     * negative.  This would be very unlikely, except when int/wis was high
     * enough to compensate for armor, but was severly drained by an annoying
     * monster.  Since the following code blindly assumes that new_mana is >=
     * 0, we must do the work and return here. -CFT
     */

        /* No mana left */
        if (new_mana < 1) {
            p_ptr->cmana = p_ptr->cmana_frac = p_ptr->mana = 0;
            p_ptr->redraw |= PR_MANA;
            return;
        }

        /* mana can be zero when creating character */
        if (p_ptr->mana != new_mana) {

            if (p_ptr->mana) {
            /*
             * change current mana proportionately to change of max mana,
             * divide first to avoid overflow, little loss of accuracy
             */
                value = ((((long)p_ptr->cmana << 16) + p_ptr->cmana_frac) /
                         p_ptr->mana * new_mana);
                p_ptr->cmana = (value >> 16);
                p_ptr->cmana_frac = (value & 0xFFFF);
            }
            else {
                p_ptr->cmana = new_mana;
                p_ptr->cmana_frac = 0;
            }

            p_ptr->mana = new_mana;

            /* Display mana later */
            p_ptr->redraw |= PR_MANA;
        }
    }

    else if (p_ptr->mana) {
        p_ptr->mana = 0;
        p_ptr->cmana = 0;

        /* Display mana later */
        p_ptr->redraw |= PR_MANA;
    }
}





/*
 * Allow user to choose a spell from the given book.
 * Note -- never call this function for warriors!
 *
 * If a valid spell is chosen, saves it in '*sn' and returns TRUE
 * If the user hits escape, returns FALSE, and set '*sn' to -1
 * If there are no legal choices, returns FALSE, and sets '*sn' to -2
 */
static int cast_spell(cptr prompt, int item_val, int *sn)
{
    int			i, use = -1, num, index;
    int			spell[64];
    bool		flag, redraw, okay, ask;
    char		choice;
    cptr		q;
    u32b		j1, j2;
    inven_type		*i_ptr;
    spell_type		*s_ptr;

    char		out_str[160];
    char		tmp_str[160];


    /* Index into the spell name table */
    index = (cp_ptr->spell_stat == A_INT) ? 0 : 1;


    /* Get the spell book */
    i_ptr = &inventory[item_val];

    /* Observe spells in that book */
    j1 = i_ptr->flags1;
    j2 = i_ptr->flags2;

    /* Extract spells */
    num = 0;
    while (j1) spell[num++] = bit_pos(&j1);
    while (j2) spell[num++] = bit_pos(&j2) + 32;


    /* Assume no usable spells */
    okay = FALSE;

    /* Assume no spells available */
    (*sn) = -2;

    /* Check for known spells */
    for (i = 0; i < num; i++) {
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


    /* Hack -- Update the "choice" window */
    choice_spell(spell, num);


    /* Build a prompt (accept all spells) */
    sprintf(out_str, "(Spells %c-%c, *=List, <ESCAPE>=exit) %s",
            'a', 'a' + num - 1, prompt);

    /* Get a spell from the user */
    while (!flag && get_com(out_str, &choice)) {

        /* Request redraw */
        if ((choice == '*') || (choice == '?')) {

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
        use = spell[i];

        /* Check for illegal spell */
        if (!spell_okay(use)) {
            sprintf(tmp_str, "You don't know that %s.",
                ((cp_ptr->spell_stat == A_INT) ? "spell" : "prayer"));
            msg_print(tmp_str);
            continue;
        }

        /* Verify it */
        if (ask) {

            /* Access the spell */
            s_ptr = &magic_spell[p_ptr->pclass-1][use];

            /* Prompt */
            sprintf(tmp_str, "Cast %s (%d mana, %d%% fail)?",
                    spell_names[index][use], s_ptr->smana,
                    spell_chance(use));

            /* Belay that order */
            if (!get_check(tmp_str)) continue;
        }

        /* Stop the loop */
        flag = TRUE;	
    }

    /* Restore the screen */
    if (redraw) restore_screen();


    /* XXX XXX Hack -- update choice window */
    if (!choice_default) choice_inven(0, inven_ctr - 1);
    else choice_equip(INVEN_WIELD, INVEN_TOTAL - 1);


    /* Abort if needed */
    if (!flag) return (FALSE);


    /* Access the spell */
    s_ptr = &magic_spell[p_ptr->pclass-1][use];

    /* Verify if needed */
    if (s_ptr->smana > p_ptr->cmana) {

        if (cp_ptr->spell_stat == A_INT) {
            q = "You summon your limited strength to cast this one! Confirm?";
        }
        else {
            q = "The gods may think you presumptuous for this! Confirm?";
        }

        /* Allow cancel */
        if (!get_check(q)) return (FALSE);
    }

    /* Save the choice */
    (*sn) = use;

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
    int                  i1, i2, num, item_val;
    int                  spell[64];
    u32b		j1, j2;
    inven_type		*i_ptr;


    /* The tval of readible books */
    int read_tval = 0;

    /* Acquire the type value of the books that the player can read, if any */
    if (cp_ptr->spell_stat == A_WIS) read_tval = TV_PRAYER_BOOK;
    else if (cp_ptr->spell_stat == A_INT) read_tval = TV_MAGIC_BOOK;


    /* This command is free */
    energy_use = 0;

    if (p_ptr->blind || no_lite()) {
        msg_print("You cannot see!");
        return;
    }

    if (p_ptr->pclass == 0) {
        msg_print("You cannot read books!");
        return;
    }

    if (p_ptr->confused) {
        msg_print("You are too confused!");
        return;
    }

    if (!find_range(read_tval, &i1, &i2)) {
        msg_print("You are not carrying any usable books!");
        return;
    }


    /* Get a book or stop checking */
    if (!get_item(&item_val, "Browse which Book?", i1, i2, FALSE)) return;

    /* Cancel auto-see */
    command_see = FALSE;


    /* Access the book */
    i_ptr = &inventory[item_val];

    /* Obtain all spells in the book */
    j1 = i_ptr->flags1;
    j2 = i_ptr->flags2;

    /* Build spell list */
    num = 0;
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
 * gain spells when player wants to		- jw
 */
void do_cmd_study(void)
{
    char                query;

    int                 diff_spells, new_spells;
    int                 spell[64], index, last_known;
    int			i, j, ii, jj, num, col;

    u32b		spell_flag1, spell_flag2;

    spell_type		*s_ptr;

    char		buf[160];
    char		tmp_str[160];

    /* The tval of readible books */
    int read_tval = 0;

    /* Acquire the type value of the books that the player can read, if any */
    if (cp_ptr->spell_stat == A_WIS) read_tval = TV_PRAYER_BOOK;
    else if (cp_ptr->spell_stat == A_INT) read_tval = TV_MAGIC_BOOK;


    /* Assume free */
    energy_use = 0;


    if (p_ptr->pclass == 0) {
        msg_print("You cannot learn magic!");
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

    /* No spells */
    if (!(p_ptr->new_spells)) {
        sprintf(tmp_str, "You cannot learn any new %ss!",
                (cp_ptr->spell_stat == A_INT ? "spell" : "prayer"));
        msg_print(tmp_str);
    }


    /* Count available spells */
    new_spells = p_ptr->new_spells;

    /* Count change */
    diff_spells = 0;


    /* Index into the spell name table */
    index = (cp_ptr->spell_stat == A_INT) ? 0 : 1;

    /* Find the next open entry in "spell_order[]" */
    for (last_known = 0; last_known < 64; last_known++) {
        if (spell_order[last_known] == 99) break;
    }

    /* Determine which spells player can learn */
    spell_flag1 = 0L;
    spell_flag2 = 0L;

    /* Check all books */
    for (i = 0; i < inven_ctr; i++) {
        if (inventory[i].tval == read_tval) {
            spell_flag1 |= inventory[i].flags1;
            spell_flag2 |= inventory[i].flags2;
        }
    }

    /* Clear bits of spells already learned */
    spell_flag1 &= ~spell_learned1;
    spell_flag2 &= ~spell_learned2;

    /* Reset */
    i = 0;

    /* Extract available spells */
    while (spell_flag1) {
        j = bit_pos(&spell_flag1);
        s_ptr = &magic_spell[p_ptr->pclass-1][j];
        if (s_ptr->slevel <= p_ptr->lev) spell[i++] = j;
    }

    /* Extract available spells */
    while (spell_flag2) {
        j = bit_pos(&spell_flag2) + 32;
        s_ptr = &magic_spell[p_ptr->pclass-1][j];
        if (s_ptr->slevel <= p_ptr->lev) spell[i++] = j;
    }


    /* Note missing books */
    if (new_spells > i) {
        msg_print("You seem to be missing a book.");
        diff_spells = new_spells - i;
        new_spells = i;
        if (new_spells == 0) return;
    }


    /* Take a turn */
    energy_use = 0;


    /* Mage-spells */
    if (cp_ptr->spell_stat == A_INT) {

        /* Save the screen */
        save_screen();

        /* Let player choose spells until done */
        while (new_spells) {

            /* Column */
            col = 31;

            /* Number of spells */
            num = (i <= 22) ? i : 22;

            /* Title the list */
            prt("", 1, col);
            put_str("Name", 1, col + 5);
            put_str("Lv Mana Fail", 1, col + 35);

            /* List the spells */
            for (ii = 0; ii < num; ii++) {

                /* Access the spell */
                jj = spell[ii];

                /* Access the spell */
                s_ptr = &magic_spell[p_ptr->pclass-1][jj];

                /* List the spell */
                (void)sprintf(buf, "  %c) %-30s%2d %4d %3d%%",
                              'a' + ii, spell_names[index][jj],
                              s_ptr->slevel, s_ptr->smana, spell_chance(jj));
                prt(buf, 2 + ii, col);
            }


            /* Prepare a prompt */
            sprintf(buf, "Learn which spell (%d left)? ", new_spells);

            /* Let player choose a spell */
            if (!get_com(buf, &query)) break;

            /* Analyze request */
            j = query - 'a';

            /* Analyze valid answers */
            if ((j >= 0) && (j < i) && (j < 22)) {

                /* Add the spell */
                if (spell[j] < 32) {
                    spell_learned1 |= 1L << spell[j];
                }
                else {
                    spell_learned2 |= 1L << (spell[j] - 32);
                }

                /* Add the spell to the known list */
                spell_order[last_known++] = spell[j];

                /* Slide the spells */
                for (; j <= i - 1; j++) spell[j] = spell[j + 1];

                /* One less spell available */
                i--;

                /* One less spell to learn */
                new_spells--;

                /* Clear the last spell */
                prt("", j + 1, 31);

                /* Try again */
                continue;
            }

            /* Invalid choice */
            bell();
        }

        /* Restore screen */
        restore_screen();
    }

    /* Priest spells */
    else {

        /* Learn a single prayer */
        if (new_spells) {

            /* Pick a spell to learn */
            j = rand_int(i);

            /* Learn the spell */
            if (spell[j] < 32) {
                spell_learned1 |= 1L << spell[j];
            }
            else {
                spell_learned2 |= 1L << (spell[j] - 32);
            }

            /* Memorize the order */
            spell_order[last_known++] = spell[j];

            /* Mention the result */
            (void)sprintf(tmp_str,
                          "You have learned the prayer of %s.",
                          spell_names[index][spell[j]]);
            msg_print(tmp_str);

            /* Slide the spells */
            for (; j <= i - 1; j++) spell[j] = spell[j + 1];

            /* One less spell available */
            i--;

            /* One less spell to learn */
            new_spells--;
        }

        /* Report on remaining prayers */
        if (new_spells) {
            sprintf(tmp_str, "You can learn %d more prayer%s.",
                    new_spells, (new_spells == 1) ? "" : "s");
            msg_print(tmp_str);
        }
    }

    /* Remember how many spells can be learned */
    p_ptr->new_spells = new_spells + diff_spells;

    /* Update the mana */
    p_ptr->update |= (PU_MANA);

    /* Redraw Study Status */
    p_ptr->redraw |= PR_STUDY;
}


/*
 * Hack -- fire a bolt, or a beam if lucky
 */
static void bolt_or_beam(int prob, int typ, int dir, int dam)
{
    if (randint(100) < prob) {
        line_spell(typ, dir, py, px, dam);
    }
    else {
        fire_bolt(typ, dir, py, px, dam);
    }
}


/*
 * Throw a magic spell					-RAK-	
 */
void do_cmd_cast(void)
{
    int			i, j, item_val, dir;
    int			choice, chance;
    int			plev = p_ptr->lev;
    
    spell_type   *s_ptr;

    energy_use = 0;

    if (cp_ptr->spell_stat != A_INT) {
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

    if (!find_range(TV_MAGIC_BOOK, &i, &j)) {
        msg_print("You are not carrying any spell-books!");
        return;
    }


    /* Get a spell book */
    if (!get_item(&item_val, "Use which Spell Book? ", i, j, FALSE)) return;

    /* Cancel auto-see */
    command_see = FALSE;


    /* Ask for a spell */
    if (!cast_spell("Cast which spell?", item_val, &choice)) {
        if (choice == -2) msg_print("You don't know any spells in that book.");
        return;
    }


    /* Access the spell */
    s_ptr = &magic_spell[p_ptr->pclass-1][choice];

    /* Spell failure chance */
    chance = spell_chance(choice);

    /* Failed spell */
    if (rand_int(100) < chance) {
        if (flush_failure) flush();
        msg_print("You failed to get the spell off!");
    }

    /* Process spell */
    else {

        /* does missile spell do line? -CFT */
        chance =  stat_adj(A_INT) + plev /
                  (p_ptr->pclass == 1 ? 2 : (p_ptr->pclass == 4 ? 4 : 5));

        /* Spells.  */
        switch (choice + 1) {

          case 1:
            if (!get_dir(NULL, &dir)) return;
            bolt_or_beam(chance-10, GF_MISSILE, dir,
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
            (void)lite_area(py, px,
                            damroll(2, (plev / 2)), (plev / 10) + 1);
            break;

          case 5:	   /* treasure detection */
            (void)detect_treasure();
            break;

          case 6:
            (void)hp_player(damroll(4, 4));
            if (p_ptr->cut) {
                p_ptr->cut -= 15;
                if (p_ptr->cut < 0) p_ptr->cut = 0;
                msg_print("Your wounds heal.");
            }
            break;

          case 7:	   /* object detection */
            (void)detect_object();
            break;

          case 8:
            (void)detect_sdoor();
            (void)detect_trap();
            break;

          case 9:
            if (!get_dir(NULL, &dir)) return;
            fire_ball(GF_POIS, dir, py, px,
                      10 + (plev / 2), 2);
            break;

          case 10:
            if (!get_dir(NULL, &dir)) return;
            (void)confuse_monster(dir, py, px, plev);
            break;

          case 11:
            if (!get_dir(NULL, &dir)) return;
            bolt_or_beam(chance-10, GF_ELEC, dir,
                         damroll(3+((plev-5)/4),8));
            break;

          case 12:
            (void)td_destroy();
            break;

          case 13:
            if (!get_dir(NULL, &dir)) return;
            (void)sleep_monster(dir, py, px);
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
            lite_line(dir, py, px);
            break;

          case 17:
            if (!get_dir(NULL, &dir)) return;
            bolt_or_beam(chance-10, GF_COLD, dir,
                         damroll(5+((plev-5)/4),8));
            break;

          case 18:
            if (!get_dir(NULL, &dir)) return;
            (void)wall_to_mud(dir, py, px);
            break;

          case 19:
            satisfy_hunger();
            break;

          case 20:
            (void)recharge(5);
            break;

          case 21:
            (void)sleep_monsters1(py, px);
            break;

          case 22:
            if (!get_dir(NULL, &dir)) return;
            (void)poly_monster(dir, py, px);
            break;

          case 23:
            if (ident_spell()) combine_pack();
            break;

          case 24:
            (void)sleep_monsters2();
            break;

          case 25:
            if (!get_dir(NULL, &dir)) return;
            bolt_or_beam(chance, GF_FIRE, dir,
                         damroll(8+((plev-5)/4),8));
            break;

          case 26:
            if (!get_dir(NULL, &dir)) return;
            (void)slow_monster(dir, py, px);
            break;

          case 27:
            if (!get_dir(NULL, &dir)) return;
            fire_ball(GF_COLD, dir, py, px,
                      30 + (plev), 2);
            break;

          case 28:
            (void)recharge(40);
            break;

          case 29:
            if (!get_dir(NULL, &dir)) return;
            (void)teleport_monster(dir, py, px);
            break;

          case 30:
            if (p_ptr->fast <= 0) {
                p_ptr->fast += randint(20) + plev;
            }
            else {
                p_ptr->fast += randint(5);
            }
            break;

          case 31:
            if (!get_dir(NULL, &dir)) return;
            fire_ball(GF_FIRE, dir, py, px,
                      55 + (plev), 2);
            break;

          case 32:
            destroy_area(py, px);
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
            earthquake();
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
            bolt_or_beam(chance-5, GF_ACID, dir,
                         damroll(6+((plev-5)/4), 8));
            break;

          case 40:	   /* Cloud kill */
            if (!get_dir(NULL, &dir)) return;
            fire_ball(GF_POIS, dir, py, px,
                      20 + (plev / 2), 3);
            break;

          case 41:	   /* Acid Ball */
            if (!get_dir(NULL, &dir)) return;
            fire_ball(GF_ACID, dir, py, px,
                      40 + (plev), 2);
            break;

          case 42:	   /* Ice Storm */
            if (!get_dir(NULL, &dir)) return;
            fire_ball(GF_COLD, dir, py, px,
                      70 + (plev), 3);
            break;

          case 43:	   /* Meteor Swarm */
            if (!get_dir(NULL, &dir)) return;
            fire_ball(GF_METEOR, dir, py, px,
                      65 + (plev), 3);
            break;

          case 44:	   /* Mana Storm */
            if (!get_dir(NULL, &dir)) return;
            fire_ball(GF_MANA, dir, py, px, 300, 2);
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
            if (p_ptr->fast <= 0) {
                p_ptr->fast += randint(30) + 30 + plev;
            }
            else {
                p_ptr->fast += randint(5);
            }
            break;

          case 59:
            p_ptr->invuln += randint(8) + 8;
            break;

          default:
            break;
        }

        /* A spell was cast */
        if (choice < 32) {
            if (!(spell_worked1 & (1L << choice))) {
                spell_worked1 |= (1L << choice);
                p_ptr->exp += s_ptr->sexp << 2;
                check_experience();
            }
        }
        else {
            if (!(spell_worked2 & (1L << (choice - 32)))) {
                 spell_worked2 |= (1L << (choice - 32));
                p_ptr->exp += s_ptr->sexp << 2;
                check_experience();
            }
        }
    }

    /* Take a turn */
    energy_use = 100;

    /* Use some mana */
    if (s_ptr->smana > p_ptr->cmana) {
        msg_print("You faint from the effort!");
        p_ptr->paralysis = randint((int)(5 * (s_ptr->smana - p_ptr->cmana)));
        p_ptr->cmana = 0;
        p_ptr->cmana_frac = 0;
        if (rand_int(3) == 0) {
            msg_print("You have damaged your health!");
            (void)dec_stat(A_CON, 15 + randint(10), (rand_int(3) == 0));
        }
    }
    else {
        p_ptr->cmana -= s_ptr->smana;
    }

    /* Update stuff */
    p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
    
    /* Redraw mana */
    p_ptr->redraw |= (PR_MANA);
    
    /* Handle stuff */
    handle_stuff(TRUE);
}



/*
 * Pray
 */
void do_cmd_pray(void)
{
    int i, j, item_val, dir;
    int choice, chance;
    spell_type  *s_ptr;
    inven_type   *i_ptr;

    int plev = p_ptr->lev;
    
    energy_use = 0;

    if (cp_ptr->spell_stat != A_WIS) {
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

    if (!find_range(TV_PRAYER_BOOK, &i, &j)) {
        msg_print("You are not carrying any Holy Books!");
        return;
    }


    /* Choose a book */
    if (!get_item(&item_val, "Use which Holy Book? ", i, j, FALSE)) return;

    /* Cancel auto-see */
    command_see = FALSE;


    /* Choose a spell */
    if (!cast_spell("Recite which prayer?", item_val, &choice)) {
        if (choice == -1) msg_print("You don't know any prayers in that book.");
        return;
    }


    /* Access the spell */
    s_ptr = &magic_spell[p_ptr->pclass-1][choice];

    /* Spell failure chance */
    chance = spell_chance(choice);

    /* Check for failure */
    if (rand_int(100) < chance) {
        if (flush_failure) flush();
        msg_print("You failed to concentrate hard enough!");
    }

    /* Success */
    else {

        switch (choice + 1) {

          case 1:
            (void)detect_evil();
            break;

          case 2:
            (void)hp_player(damroll(3, 3));
            if (p_ptr->cut) {
                p_ptr->cut -= 10;
                if (p_ptr->cut < 0) p_ptr->cut = 0;
                msg_print("Your wounds heal.");
            }
            break;

          case 3:
            bless(randint(12) + 12);
            break;

          case 4:
            (void)remove_fear();
            break;

          case 5:
            (void)lite_area(py, px,
                            damroll(2, (plev / 2)), (plev / 10) + 1);
            break;

          case 6:
            (void)detect_trap();
            break;

          case 7:
            (void)detect_sdoor();
            break;

          case 8:
            (void)slow_poison();
            break;

          case 9:
            if (!get_dir(NULL, &dir)) return;
            (void)fear_monster(dir, py, px, plev);
            break;

          case 10:
            teleport_flag = TRUE;
            teleport_dist = plev * 3;
            break;

          case 11:
            (void)hp_player(damroll(4, 4));
            if (p_ptr->cut) {
                p_ptr->cut = (p_ptr->cut / 2) - 20;
                if (p_ptr->cut < 0) p_ptr->cut = 0;
                msg_print("Your wounds heal.");
            }
            break;

          case 12:
            bless(randint(24) + 24);
            break;

          case 13:
            (void)sleep_monsters1(py, px);
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
            fire_ball(GF_HOLY_ORB, dir, py, px,
                      (int)(damroll(3,6) + plev +
                            ((p_ptr->pclass == 2) ? 2 : 1) * stat_adj(A_WIS)),
                      ((plev < 30) ? 2 : 3));
            break;

          case 19:
            (void)hp_player(damroll(8, 4));
            if (p_ptr->cut) {
                p_ptr->cut = 0;
                msg_print("Your wounds heal.");
            }
            break;

          case 20:
            detect_inv2(randint(24) + 24);
            break;

          case 21:
            (void)protect_evil();
            break;

          case 22:
            earthquake();
            break;

          case 23:
            map_area();
            break;

          case 24:
            (void)hp_player(damroll(16, 4));
            if (p_ptr->cut) {
                p_ptr->cut = 0;
                msg_print("Your wounds heal.");
            }
            break;

          case 25:
            (void)turn_undead();
            break;

          case 26:
            bless(randint(48) + 48);
            break;

          case 27:
            /* Dispel (undead) monsters */
            (void)dispel_monsters((int)(3 * plev), FALSE, TRUE);
            break;

          case 28:
            (void)hp_player(200);
            if (p_ptr->stun) {
                p_ptr->stun = 0;
                msg_print("Your head stops stinging.");
            }
            if (p_ptr->cut) {
                p_ptr->cut = 0;
                msg_print("You feel better.");
            }
            break;

          case 29:
            /* Dispel (evil) monsters */
            (void)dispel_monsters((int)(3 * plev), TRUE, FALSE);
            break;

          case 30:
            warding_glyph();
            break;

          case 31:
            /* Dispel (evil) monsters */
            (void)dispel_monsters((int)(4 * plev), TRUE, FALSE);
            (void)remove_fear();
            (void)cure_poison();
            (void)hp_player(1000);
            if (p_ptr->stun) {
                p_ptr->stun = 0;
                msg_print("Your head stops stinging.");
            }
            if (p_ptr->cut) {
                p_ptr->cut = 0;
                msg_print("You feel better.");
            }
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
            (void)hp_player(damroll(8, 4));
            if (p_ptr->cut) {
                p_ptr->cut = 0;
                msg_print("Your wounds heal.");
            }
            break;

          case 38:
            (void)hp_player(damroll(16, 4));
            if (p_ptr->cut) {
                p_ptr->cut = 0;
                msg_print("Your wounds heal.");
            }
            break;

          case 39:
            (void)hp_player(2000);
            if (p_ptr->stun) {
                p_ptr->stun = 0;
                msg_print("Your head stops stinging.");
            }
            if (p_ptr->cut) {
                p_ptr->cut = 0;
                msg_print("You feel better.");
            }
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
            /* Dispel (undead) monsters */
            (void)dispel_monsters((int)(4 * plev), FALSE, TRUE);
            break;

          case 43:	   /* dispel evil */
            /* Dispel (evil) monsters */
            (void)dispel_monsters((int)(4 * plev), TRUE, FALSE);
            break;

          case 44:	   /* banishment */
            if (banish_evil(100)) {
                msg_print("The Power of your god banishes evil!");
            }
            break;

          case 45:	   /* word of destruction */
            destroy_area(py, px);
            break;

          case 46:	   /* annihilation */
            if (!get_dir(NULL, &dir)) return;
            drain_life(dir, py, px, 200);
            break;

          case 47:	   /* unbarring ways */
            (void)td_destroy();
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

            /* you can not create an ego weapon from a cursed */
            /* object.  the curse would "taint" the magic -CFT */
            /* And you can never modify artifacts */
            /* And you also cannot modify ego-items, or you */
            /* would "lose" the primary ego-item type */
            if ((i_ptr->tval) &&
                (!i_ptr->name1) &&
                (!i_ptr->name2) &&
                (!cursed_p(i_ptr))) {

                cptr act;
                char tmp_str[100];

                if (rand_int(2)) {
                    act = " is covered in a fiery shield!";
                    i_ptr->name2 = EGO_FT;
                    i_ptr->flags1 |= (TR1_BRAND_FIRE);
                    i_ptr->flags2 |= (TR2_RES_FIRE);
                    i_ptr->flags3 |= (TR3_IGNORE_FIRE);
                    i_ptr->cost += 3000L;
                }
                else {
                    act = " glows deep, icy blue!";
                    i_ptr->name2 = EGO_FB;
                    i_ptr->flags1 |= (TR1_BRAND_COLD);
                    i_ptr->flags2 |= (TR2_RES_COLD);
                    i_ptr->flags3 |= (TR3_IGNORE_COLD);
                    i_ptr->cost += 2500L;
                }

                objdes(tmp_str, i_ptr, FALSE);

                message("Your ", 0x02);
                message(tmp_str, 0x02);
                message(act, 0);

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
            (void)teleport_monster(dir, py, px);
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
            new_level_flag = TRUE;
            break;

          default:
            break;
        }

        /* End of prayers.				 */
        if (choice < 32) {
            if (!(spell_worked1 & (1L << choice))) {
                spell_worked1 |= (1L << choice);
                p_ptr->exp += s_ptr->sexp << 2;
                check_experience();
            }
        }
        else {
            if (!(spell_worked2 & (1L << (choice - 32)))) {
                 spell_worked2 |= (1L << (choice - 32));
                 p_ptr->exp += s_ptr->sexp << 2;
                 check_experience();
            }
        }
    }

    /* Take a turn */
    energy_use = 100;

    /* Reduce mana */
    if (s_ptr->smana > p_ptr->cmana) {
        msg_print("You faint from fatigue!");
        p_ptr->paralysis = randint((int)(5 * (s_ptr->smana - p_ptr->cmana)));
        p_ptr->cmana = 0;
        p_ptr->cmana_frac = 0;
        if (rand_int(3) == 0) {
            msg_print("You have damaged your health!");
            (void)dec_stat(A_CON, 15 + randint(10), (rand_int(3) == 0));
        }
    }
    else {
        p_ptr->cmana -= s_ptr->smana;
    }

    /* Update stuff */
    p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
    
    /* Redraw mana */
    p_ptr->redraw |= (PR_MANA);
        
    /* Handle stuff */
    handle_stuff(TRUE);
}

