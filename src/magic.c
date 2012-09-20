/* File: magic.c */

/* Purpose: code for mage/priest spells */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "angband.h"


/*
 * calculate number of spells player should have,
 * and learn/forget spells until that number is met -JEW- 
 */
void calc_spells(int stat)
{
    register int    i, k;
    register u32b mask;
    u32b          spell_flag;
    int             j, offset;
    int             num_allowed, new_spells, num_known, levels;
    vtype           tmp_str;
    cptr	    p;
    register spell_type  *s_ptr;

    s_ptr = &magic_spell[p_ptr->pclass - 1][0];

    if (stat == A_INT) {
	p = "spell";
	offset = SPELL_OFFSET;
    }
    else {
	p = "prayer";
	offset = PRAYER_OFFSET;
    }


    /* XXX Hack -- this technically runs in the "wrong" order */

    /* Check to see if know any spells greater than level, eliminate them */
    for (i = 31; i >= 0; i--) {

	mask = 1L << i;

	if (mask & spell_learned) {
	    if (s_ptr[i].slevel > p_ptr->lev) {
		spell_learned &= ~mask;
		spell_forgotten |= mask;
		(void)sprintf(tmp_str, "You have forgotten the %s of %s.", p,
			      spell_names[i + offset]);
		msg_print(tmp_str);
	    }
	}

	if (mask & spell_learned2) {
	    if (s_ptr[i + 32].slevel > p_ptr->lev) {
		spell_learned2 &= ~mask;
		spell_forgotten2 |= mask;
		(void)sprintf(tmp_str, "You have forgotten the %s of %s.", p,
			      spell_names[i + offset + 32]);
		msg_print(tmp_str);
	    }
	}
    }


    /* Determine the number of spells allowed */
    levels = p_ptr->lev - class[p_ptr->pclass].first_spell_lev + 1;
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
	if (mask & spell_learned) num_known++;
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
	    if (!spell_forgotten && !spell_forgotten2) break;

	    /* Get the (i+1)th spell learned */
	    j = spell_order[i];

	    /* Don't process unknown spells... -CFT */
	    if (j == 99) continue;

	    /* First set of spells */
	    if (j < 32) {
		mask = 1L << j;
		if (mask & spell_forgotten) {
		    if (s_ptr[j].slevel <= p_ptr->lev) {
			spell_forgotten &= ~mask;
			spell_learned |= mask;
			new_spells--;
			sprintf(tmp_str, "You have remembered the %s of %s.",
				p, spell_names[j + offset]);
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
		    if (s_ptr[j].slevel <= p_ptr->lev) {
			spell_forgotten2 &= ~mask;
			spell_learned2 |= mask;
			new_spells--;
			sprintf(tmp_str, "You have remembered the %s of %s.",
				p, spell_names[j + offset]);
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

	/* only bother with spells learnable by class -CFT */
	spell_flag = spellmasks[p_ptr->pclass][0] & ~spell_learned;
	for (j = 0; spell_flag; j++) {
	    mask = 1L << j;
	    if (spell_flag & mask) {
		spell_flag &= ~mask;
		if (s_ptr[j].slevel <= p_ptr->lev) k++;
	    }
	}

	/* only bother with spells learnable by class -CFT */
	spell_flag = spellmasks[p_ptr->pclass][1] & ~spell_learned2;
	for (j = 32; spell_flag; j++) {
	    mask = 1L << (j - 32);
	    if (spell_flag & mask) {
		spell_flag &= ~mask;
		if (s_ptr[j].slevel <= p_ptr->lev) k++;
	    }
	}

	/* Cannot learn more spells than exist */
	if (new_spells > k) new_spells = k;
    }


    /* Forget spells */
    if (new_spells < 0) {

	/* Forget spells in the opposite order they were learned */
	for (i = 63; new_spells < 0; i--) {

	    /* Hack -- we might run out of spells to forget */
	    if (!spell_learned && !spell_learned2) {
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
		if (mask & spell_learned) {
		    spell_learned &= ~mask;
		    spell_forgotten |= mask;
		    new_spells++;
		    sprintf(tmp_str, "You have forgotten the %s of %s.",
			    p, spell_names[j + offset]);
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
			    p, spell_names[j + offset]);
		    msg_print(tmp_str);
		}
	    }
	}

	/* Paranoia -- in case we run out of spells to forget */
	new_spells = 0;
    }


    /* Take note when "study" changes */
    if (character_generated && (new_spells != p_ptr->new_spells)) {
	if (new_spells > 0 && p_ptr->new_spells == 0) {
	    (void)sprintf(tmp_str, "You can learn some new %ss now.", p);
	    msg_print(tmp_str);
	}
	p_ptr->new_spells = new_spells;
	p_ptr->status |= PY_STUDY;
    }
}


/*
 * gain spells when player wants to		- jw 
 */
void gain_spells(void)
{
    char                query;
    int                 stat, diff_spells, new_spells;
    int                 spells[63], offset, last_known;
    register int        i, j;
    register u32b     spell_flag = 0, spell_flag2 = 0, mask;
    vtype               tmp_str;
    register spell_type *s_ptr;

    if (!p_ptr->pclass) {
	msg_print("A warrior learn magic???  HA!");
	return;
    }

    if (p_ptr->blind > 0) {
	msg_print("You can't see to read your spell book!");
	return;
    }

    if (no_lite()) {
	msg_print("You have no light to read by.");
	return;
    }

    if (p_ptr->confused > 0) {
	msg_print("You are too confused.");
	return;
    }


    new_spells = p_ptr->new_spells;
    diff_spells = 0;
    s_ptr = &magic_spell[p_ptr->pclass - 1][0];


    /* Mage vs Priest */
    if (class[p_ptr->pclass].spell == MAGE) {
	stat = A_INT;
	offset = SPELL_OFFSET;
    }
    else {
	stat = A_WIS;
	offset = PRAYER_OFFSET;
    }


    for (last_known = 0; last_known < 64; last_known++) {
	if (spell_order[last_known] == 99) break;
    }

    if (!new_spells) {
	(void)sprintf(tmp_str, "You can't learn any new %ss!",
		      (stat == A_INT ? "spell" : "prayer"));
	msg_print(tmp_str);
	free_turn_flag = TRUE;
    }
    else {

	/* Determine which spells player can learn */
	spell_flag = 0;
	spell_flag2 = 0;
	for (i = 0; i < inven_ctr; i++) {
	    if (((stat == A_INT) && (inventory[i].tval == TV_MAGIC_BOOK)) ||
		((stat == A_WIS) && (inventory[i].tval == TV_PRAYER_BOOK))) {
		spell_flag |= inventory[i].flags1;
		spell_flag2 |= inventory[i].flags2;
	    }
	}
    }

    /* clear bits for spells already learned */
    spell_flag &= ~spell_learned;
    spell_flag2 &= ~spell_learned2;

    i = 0;

    /* Check the low level spells first */ 
    for (j = 0; spell_flag; j++) {

	/* Get the mode */
	mask = 1L << j;

	if (spell_flag & mask) {
	    spell_flag &= ~mask;
	    if (s_ptr[j].slevel <= p_ptr->lev) {
		spells[i++] = j;
	    }
	}
    }

    /* And THEN check the high level spells */
    for (j = 0; spell_flag2; j++) {

	/* Get the mode */
	mask = 1L << j;

	if (spell_flag2 & mask) {
	    spell_flag2 &= ~mask;
	    if (s_ptr[j + 32].slevel <= p_ptr->lev) {
		spells[i++] = j + 32;
	    }
	}
    }

    if (new_spells > i) {
	msg_print("You seem to be missing a book.");
	diff_spells = new_spells - i;
	new_spells = i;
    }

    if (new_spells == 0);

    else if (stat == A_INT) {

	/* Save the screen */    
	save_screen();

	/* Display spells that can be learned */
	print_spells(spells, i, FALSE, -1);

	/* Let player choose spells until done */
	while (new_spells) {

	    char buf[128];

	    /* Prepare a prompt */
	    sprintf(buf, "Learn which spell (%d left)? ", new_spells);

	    /* Let player choose a spell */
	    if (!get_com(buf, &query)) break;

	    /* Analyze request */
	    j = query - 'a';

	    /* Hack -- Limit selection to the first 22 spells */
	    if (j >= 0 && j < i && j < 22) {

		new_spells--;
		if (spells[j] < 32) {
		    spell_learned |= 1L << spells[j];
		}
		else {
		    spell_learned2 |= 1L << (spells[j] - 32);
		}
		spell_order[last_known++] = spells[j];

		/* Slide the spells */
		for (; j <= i - 1; j++) spells[j] = spells[j + 1];
		i--;
		erase_line(j + 1, 31);
		print_spells(spells, i, FALSE, -1);
		continue;
	    }

	    /* Invalid choice */
	    bell();
	}

	/* Restore screen */
	restore_screen();
    }

    else {

	/* Learn a single prayer */
	if (new_spells) {
	    j = rand_int(i);
	    if (spells[j] < 32) {
		spell_learned |= 1L << spells[j];
	    }
	    else {
		spell_learned2 |= 1L << (spells[j] - 32);
	    }
	    spell_order[last_known++] = spells[j];
	    (void)sprintf(tmp_str,
			  "You have learned the prayer of %s.",
			  spell_names[spells[j] + offset]);
	    msg_print(tmp_str);
	    for (; j <= i - 1; j++) {
		spells[j] = spells[j + 1];
	    }
	    i--;
	    new_spells--;
	}

	/* Report on remaining prayers */
	if (new_spells) {
	    sprintf(tmp_str, "You can learn %d more prayer%s.",
	    	    new_spells, (new_spells == 1) ? "" : "s");
	    msg_print(tmp_str);
	}
    }

    p_ptr->new_spells = new_spells + diff_spells;

    /* Player has gained some spells */
    if (p_ptr->new_spells == 0) {
	p_ptr->status |= PY_STUDY;
    }

    /* set the mana for first level characters when they learn first spell */
    if (p_ptr->mana == 0) calc_mana(stat);
}



/*
 * Gain some mana if you know at least one spell	-RAK-	 
 */
void calc_mana(int stat)
{
    register int          new_mana, levels;
    register s32b        value;
    register int          i;
    register inven_type  *i_ptr;
    int                   amrwgt, maxwgt;

    static int cumber_armor = FALSE;
    static int cumber_glove = FALSE;

    bool old_glove = cumber_glove;
    bool old_armor = cumber_armor;

    if (spell_learned != 0 || spell_learned2 != 0) {
	levels = p_ptr->lev - class[p_ptr->pclass].first_spell_lev + 1;
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
	if (new_mana > 0) new_mana++;


	/* Assume player is not encumbered by gloves */
	cumber_glove = FALSE;

	/* Get the gloves */
	i_ptr = &inventory[INVEN_HANDS];

	/* good gauntlets of dexterity or free action do not hurt spells */
	if ((i_ptr->tval != TV_NOTHING) &&
	    !((i_ptr->flags2 & TR2_FREE_ACT) ||
	      ((i_ptr->flags1 & TR1_DEX) &&
	       (i_ptr->pval > 0)))) {

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
	    msg_print("Your bare hands feel more suitable for spellcasting.");
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


    /* if low int/wis, gloves, and lots of heavy armor, new_mana could be
     * negative.  This would be very unlikely, except when int/wis was high
     * enough to compensate for armor, but was severly drained by an annoying
     * monster.  Since the following code blindly assumes that new_mana is >=
     * 0, we must do the work and return here. -CFT 
     */

	/* No mana left */
	if (new_mana < 1) {
	    p_ptr->cmana = p_ptr->cmana_frac = p_ptr->mana = 0;
	    p_ptr->status |= PY_MANA;
	    return;
	}

	/* mana can be zero when creating character */
	if (p_ptr->mana != new_mana) {

	    if (p_ptr->mana != 0) {
	    /*
	     * change current mana proportionately to change of max mana,
	     * divide first to avoid overflow, little loss of accuracy 
	     */
		value = ((((long)p_ptr->cmana << 16) + p_ptr->cmana_frac) /
			 p_ptr->mana * new_mana);
		p_ptr->cmana = value >> 16;
		p_ptr->cmana_frac = value & 0xFFFF;
	    }
	    else {
		p_ptr->cmana = new_mana;
		p_ptr->cmana_frac = 0;
	    }

	    p_ptr->mana = new_mana;

	    /* can't print mana here, may be in store or inventory mode */
	    p_ptr->status |= PY_MANA;
	}
    }

    else if (p_ptr->mana != 0) {
	p_ptr->mana = 0;
	p_ptr->cmana = 0;
	/* can't print mana here, may be in store or inventory mode */
	p_ptr->status |= PY_MANA;
    }
}


/*
 * Hack -- fire a bolt, or a beam if lucky
 */
static void bolt_or_beam(int prob, int typ, int dir, int dam)
{
    if (randint(100) < prob) {
	line_spell(typ, dir, char_row, char_col, dam);
    }
    else {
	fire_bolt(typ, dir, char_row, char_col, dam);
    }
}


/*
 * Throw a magic spell					-RAK-	 
 */
void cast()
{
    int                    i, j, item_val, dir;
    int                    choice, chance, result;
    register spell_type   *s_ptr;

    free_turn_flag = TRUE;

    if (class[p_ptr->pclass].spell != MAGE) {
	msg_print("You can't cast spells!");
	return;
    }

    if (p_ptr->blind > 0) {
	msg_print("You can't see to read your spell book!");
	return;
    }

    if (no_lite()) {
	msg_print("You have no light to read by.");
	return;
    }
    
    if (p_ptr->confused > 0) {
	msg_print("You are too confused.");
	return;
    }

    if (!find_range(TV_MAGIC_BOOK, TV_NEVER, &i, &j)) {
	msg_print("But you are not carrying any spell-books!");
	return;
    }


    /* Hack -- allow auto-see */
    command_wrk = TRUE;
    
    /* Get a spell book */
    if (!get_item(&item_val, "Use which Spell Book? ", i, j)) return;

    /* Cancel auto-see */
    command_see = FALSE;
    

    /* Ask for a spell */
    result = cast_spell("Cast which spell?", item_val, &choice, &chance);

    /* Cancelled */
    if (!result) return;

    /* Unknown */    
    if (result < 0) {
	msg_print("You don't know any spells in that book.");
	return;
    }


    if (p_ptr->stun > 50) chance += 25;
    else if (p_ptr->stun > 0) chance += 15;

    s_ptr = &magic_spell[p_ptr->pclass - 1][choice];

    /* Failed spell */
    if (randint(100) <= chance) {
	if (flush_failure) flush();
	msg_print("You failed to get the spell off!");
    }

    /* Process spell */
    else {

	/* does missile spell do line? -CFT */
	chance =  stat_adj(A_INT) + p_ptr->lev /
		  (p_ptr->pclass == 1 ? 2 : (p_ptr->pclass == 4 ? 4 : 5));

	/* Spells.  */
	switch (choice + 1) {

	  case 1:
	    if (!get_dir(NULL, &dir)) return;
	    bolt_or_beam(chance-10, GF_MISSILE, dir,
			 damroll(3 + ((p_ptr->lev - 1) / 5), 4));
	    break;

	  case 2:
	    (void)detect_monsters();
	    break;

	  case 3:
	    teleport(10);
	    break;

	  case 4:
	    (void)lite_area(char_row, char_col,
			    damroll(2, (p_ptr->lev / 2)), (p_ptr->lev / 10) + 1);
	    break;

	  case 5:	   /* treasure detection */
	    (void)detect_treasure();
	    break;

	  case 6:
	    (void)hp_player(damroll(4, 4));
	    if (p_ptr->cut > 0) {
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
	    fire_ball(GF_POIS, dir, char_row, char_col,
		      10 + (p_ptr->lev / 2), 2);
	    break;

	  case 10:
	    if (!get_dir(NULL, &dir)) return;
	    (void)confuse_monster(dir, char_row, char_col, p_ptr->lev);
	    break;

	  case 11:
	    if (!get_dir(NULL, &dir)) return;
	    bolt_or_beam(chance-10, GF_ELEC, dir,
			 damroll(3+((p_ptr->lev-5)/4),8));
	    break;
	    
	  case 12:
	    (void)td_destroy();
	    break;
	    
	  case 13:
	    if (!get_dir(NULL, &dir)) return;
	    (void)sleep_monster(dir, char_row, char_col);
	    break;

	  case 14:
	    (void)cure_poison();
	    break;

	  case 15:
	    teleport((int)(p_ptr->lev * 5));
	    break;

	  case 16:
	    if (!get_dir(NULL, &dir)) return;
	    msg_print("A line of blue shimmering light appears.");
	    lite_line(dir, char_row, char_col);
	    break;

	  case 17:
	    if (!get_dir(NULL, &dir)) return;
	    bolt_or_beam(chance-10, GF_COLD, dir,
			 damroll(5+((p_ptr->lev-5)/4),8));
	    break;

	  case 18:
	    if (!get_dir(NULL, &dir)) return;
	    (void)wall_to_mud(dir, char_row, char_col);
	    break;

	  case 19:
	    satisfy_hunger();
	    break;
	    
	  case 20:
	    (void)recharge(5);
	    break;
	    
	  case 21:
	    (void)sleep_monsters1(char_row, char_col);
	    break;
	    
	  case 22:
	    if (!get_dir(NULL, &dir)) return;
	    (void)poly_monster(dir, char_row, char_col);
	    break;
	    
	  case 23:
	    if (!ident_floor()) combine(ident_spell());
	    break;

	  case 24:
	    (void)sleep_monsters2();
	    break;

	  case 25:
	    if (!get_dir(NULL, &dir)) return;
	    bolt_or_beam(chance, GF_FIRE, dir,
			 damroll(8+((p_ptr->lev-5)/4),8));
	    break;
	    
	  case 26:
	    if (!get_dir(NULL, &dir)) return;
	    (void)slow_monster(dir, char_row, char_col);
	    break;

	  case 27:
	    if (!get_dir(NULL, &dir)) return;
	    fire_ball(GF_COLD, dir, char_row, char_col,
		      30 + (p_ptr->lev), 2);
	    break;

	  case 28:
	    (void)recharge(40);
	    break;

	  case 29:
	    if (!get_dir(NULL, &dir)) return;
	    (void)teleport_monster(dir, char_row, char_col);
	    break;

	  case 30:
	    if (p_ptr->fast <= 0) {
		p_ptr->fast += randint(20) + p_ptr->lev;
	    }
	    else {
		p_ptr->fast += randint(5);
	    }
	    break;

	  case 31:
	    if (!get_dir(NULL, &dir)) return;
	    fire_ball(GF_FIRE, dir, char_row, char_col,
		      55 + (p_ptr->lev), 2);
	    break;

	  case 32:
	    destroy_area(char_row, char_col);
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
		p_ptr->word_recall = 15 + randint(20);
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
			 damroll(6+((p_ptr->lev-5)/4), 8));
	    break;

	  case 40:	   /* Cloud kill */
	    if (!get_dir(NULL, &dir)) return;
	    fire_ball(GF_POIS, dir, char_row, char_col,
		      20 + (p_ptr->lev / 2), 3);
	    break;

	  case 41:	   /* Acid Ball */
	    if (!get_dir(NULL, &dir)) return;
	    fire_ball(GF_ACID, dir, char_row, char_col,
		      40 + (p_ptr->lev), 2);
	    break;

	  case 42:	   /* Ice Storm */
	    if (!get_dir(NULL, &dir)) return;
	    fire_ball(GF_COLD, dir, char_row, char_col,
		      70 + (p_ptr->lev), 3);
	    break;

	  case 43:	   /* Meteor Swarm */
	    if (!get_dir(NULL, &dir)) return;
	    fire_ball(GF_METEOR, dir, char_row, char_col,
		      65 + (p_ptr->lev), 3);
	    break;

	  case 44:	   /* Hellfire */
	    if (!get_dir(NULL, &dir)) return;
	    fire_ball(GF_HOLY_ORB, dir, char_row, char_col, 300, 2);
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
	    prt_pac();
	    calc_mana(A_INT);
	    msg_print("A mystic shield forms around your body!");
	    break;
	    
	  case 57:
	    hp_player(30);	/* XXX */
	    p_ptr->shero += randint(25) + 25;
	    break;
	    
	  case 58:
	    if (p_ptr->fast <= 0) {
		p_ptr->fast += randint(30) + 30 + p_ptr->lev;
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
	    if ((spell_worked & (1L << choice)) == 0) {
		spell_worked |= (1L << choice);
		p_ptr->exp += s_ptr->sexp << 2;
		prt_experience();
	    }
	}
	else {
	    if ((spell_worked2 & (1L << (choice - 32))) == 0) {
		 spell_worked2 |= (1L << (choice - 32));
		p_ptr->exp += s_ptr->sexp << 2;
		prt_experience();
	    }
	}
    }

    /* Take a turn */
    free_turn_flag = FALSE;

    /* Use some mana */
    if (s_ptr->smana > p_ptr->cmana) {
	msg_print("You faint from the effort!");
	p_ptr->paralysis = randint((int)(5 * (s_ptr->smana - p_ptr->cmana)));
	p_ptr->cmana = 0;
	p_ptr->cmana_frac = 0;
	if (randint(3) == 1) {
	    msg_print("You have damaged your health!");
	    (void)dec_stat(A_CON, 15 + randint(10), (randint(3) == 1));
	}
    }
    else {
	p_ptr->cmana -= s_ptr->smana;
    }

    /* Display current mana */
    prt_cmana();

    /* Hack -- recalculate all bonuses */
    calc_bonuses();
}



/*
 * Pray
 */
void pray()
{
    int i, j, item_val, dir;
    int choice, chance, result;
    register spell_type  *s_ptr;
    register inven_type   *i_ptr;

    free_turn_flag = TRUE;

    if (class[p_ptr->pclass].spell != PRIEST) {
	msg_print("Pray hard enough and your prayers may be answered.");
	return;
    }

    if (p_ptr->blind > 0) {
	msg_print("You can't see to read your prayer!");
	return;
    }

    if (no_lite()) {
	msg_print("You have no light to read by.");
	return;
    }

    if (p_ptr->confused > 0) {
	msg_print("You are too confused.");
	return;
    }

    if (!find_range(TV_PRAYER_BOOK, TV_NEVER, &i, &j)) {
	msg_print("You are not carrying any Holy Books!");
	return;
    }


    /* Hack -- allow auto-see */
    command_wrk = TRUE;
    
    /* Choose a book */
    if (!get_item(&item_val, "Use which Holy Book? ", i, j)) return;

    /* Cancel auto-see */
    command_see = FALSE;
    

    /* Choose a spell */
    result = cast_spell("Recite which prayer?", item_val, &choice, &chance);

    if (!result) return;
    
    if (result < 0) {
	msg_print("You don't know any prayers in that book.");
	return;
    }



    s_ptr = &magic_spell[p_ptr->pclass - 1][choice];

    if (p_ptr->stun > 50) chance += 25;
    else if (p_ptr->stun > 0) chance += 15;

    /* Check for failure */	    
    if (randint(100) <= chance)	{
	if (flush_failure) flush();
	msg_print("You lost your concentration!");
    }
	    
    /* Success */
    else {

	switch (choice + 1) {

	  case 1:
	    (void)detect_evil();
	    break;

	  case 2:
	    (void)hp_player(damroll(3, 3));
	    if (p_ptr->cut > 0) {
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
	    (void)lite_area(char_row, char_col,
		            damroll(2, (p_ptr->lev / 2)), (p_ptr->lev / 10) + 1);
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
	    (void)fear_monster(dir, char_row, char_col, p_ptr->lev);
	    break;
	    
	  case 10:
	    teleport((int)(p_ptr->lev * 3));
	    break;
	    
	  case 11:
	    (void)hp_player(damroll(4, 4));
	    if (p_ptr->cut > 0) {
		p_ptr->cut = (p_ptr->cut / 2) - 20;
		if (p_ptr->cut < 0) p_ptr->cut = 0;
		msg_print("Your wounds heal.");
	    }
	    break;
	    
	  case 12:
	    bless(randint(24) + 24);
	    break;
	    
	  case 13:
	    (void)sleep_monsters1(char_row, char_col);
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
	    /* Radius increases with level */
	    fire_ball(GF_HOLY_ORB, dir, char_row, char_col,
		      (int)(damroll(3,6) + p_ptr->lev +
			    (p_ptr->pclass == 2 ? 2 : 1) * stat_adj(A_WIS)),
		      (p_ptr->lev<30 ? 2 : 3));
	    break;

	  case 19:
	    (void)hp_player(damroll(8, 4));
	    if (p_ptr->cut > 0) {
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
	    if (p_ptr->cut > 0) {
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
	    (void)dispel_creature(MF2_UNDEAD, (int)(3 * p_ptr->lev));
	    break;
	    
	  case 28:
	    (void)hp_player(200);
	    if (p_ptr->stun > 0) {
		p_ptr->stun = 0;
		msg_print("Your head stops stinging.");
	    }
	    if (p_ptr->cut > 0) {
		p_ptr->cut = 0;
		msg_print("You feel better.");
	    }
	    break;
	    
	  case 29:
	    (void)dispel_creature(MF2_EVIL, (int)(3 * p_ptr->lev));
	    break;
	    
	  case 30:
	    warding_glyph();
	    break;
	    
	  case 31:
	    (void)dispel_creature(MF2_EVIL, (int)(4 * p_ptr->lev));
	    (void)remove_fear();
	    (void)cure_poison();
	    (void)hp_player(1000);
	    if (p_ptr->stun > 0) {
		p_ptr->stun = 0;
		msg_print("Your head stops stinging.");
	    }
	    if (p_ptr->cut > 0) {
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
	    if (!ident_floor()) combine(ident_spell());
	    break;
	    
	  case 35:	   /* probing */
	    (void)probing();
	    break;
	    
	  case 36:	   /* Clairvoyance */
	    wiz_lite();
	    break;
	    
	  case 37:
	    (void)hp_player(damroll(8, 4));
	    if (p_ptr->cut > 0) {
		p_ptr->cut = 0;
		msg_print("Your wounds heal.");
	    }
	    break;
	    
	  case 38:
	    (void)hp_player(damroll(16, 4));
	    if (p_ptr->cut > 0) {
		p_ptr->cut = 0;
		msg_print("Your wounds heal.");
	    }
	    break;
	    
	  case 39:
	    (void)hp_player(2000);
	    if (p_ptr->stun > 0) {
		p_ptr->stun = 0;
		msg_print("Your head stops stinging.");
	    }
	    if (p_ptr->cut > 0) {
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
	    (void)dispel_creature(MF2_UNDEAD, (int)(4 * p_ptr->lev));
	    break;
	    
	  case 43:	   /* dispel evil */
	    (void)dispel_creature(MF2_EVIL, (int)(4 * p_ptr->lev));
	    break;
	    
	  case 44:	   /* banishment */
	    if (banish_evil(100)) {
		msg_print("The Power of your god banishes evil!");
	    }
	    break;
	    
	  case 45:	   /* word of destruction */
	    destroy_area(char_row, char_col);
	    break;

	  case 46:	   /* annihilation */
	    if (!get_dir(NULL, &dir)) return;
	    drain_life(dir, char_row, char_col, 200);
	    break;

	  case 47:	   /* unbarring ways */
	    (void)td_destroy();
	    break;

	  case 48:	   /* recharging */
	    (void)recharge(15);
	    break;

	  case 49:	   /* dispel curse */
	    (void)remove_all_curse();
	    break;

	  case 50:	   /* enchant weapon */

	    i_ptr = &inventory[INVEN_WIELD];
	    if (!i_ptr->tval) i_ptr = &inventory[INVEN_BOW];

	    if (!i_ptr->tval) {
		if (flush_failure) flush();
		msg_print("The enchantment fails.");
		break;
	    }

	    /* Contain variables */
	    if (TRUE) {
		char tmp_str[100], out_val[100];
		objdes(tmp_str, i_ptr, FALSE);
		sprintf(out_val, "Your %s glows brightly!", tmp_str);
		msg_print(out_val);
		if (!enchant(i_ptr, randint(4), ENCH_TOHIT|ENCH_TODAM)) {
		    if (flush_failure) flush();
		    msg_print("The enchantment fails.");
		}
	    }
	    break;

	  case 51:	   /* enchant armor */

	    /* Contain variables */
	    if (TRUE) {
		int                 l, k = 0;
		int                 tmp[16];
		char		    tmp_str[160];
		char		    out_val[160];

		/* Build a list of armor */
		if (inventory[INVEN_BODY].tval)  tmp[k++] = INVEN_BODY;
		if (inventory[INVEN_OUTER].tval) tmp[k++] = INVEN_OUTER;
		if (inventory[INVEN_ARM].tval)   tmp[k++] = INVEN_ARM;
		if (inventory[INVEN_HEAD].tval)  tmp[k++] = INVEN_HEAD;
		if (inventory[INVEN_HANDS].tval) tmp[k++] = INVEN_HANDS;
		if (inventory[INVEN_FEET].tval)  tmp[k++] = INVEN_FEET;

		/* Nothing to enchant */
		if (!k) {
		    if (flush_failure) flush();
		    msg_print("The enchantment fails.");
		    break;
		}

		/* Pick an item to enchant */
		for (l = 0; l < 10; l++) {

		    /* Pick a random item */
		    int n = tmp[rand_int(k)];
		    i_ptr = &inventory[n];

		    /* Hack -- no other items */
		    if (k <= 1) break;

		    /* Lock instantly on to cursed items */
		    if (cursed_p(i_ptr)) break;
		}

		/* Describe the effect */
		objdes(tmp_str, i_ptr, FALSE);
		sprintf(out_val, "Your %s glows faintly!", tmp_str);
		msg_print(out_val);

		/* Attempt to enchant */
		if (!enchant(i_ptr, randint(3)+1, ENCH_TOAC)) {
		    if (flush_failure) flush();
		    msg_print("The enchantment fails.");
		}
	    }
	    break;

	  /* Elemental brand */
	  case 52:

	    i_ptr = &inventory[INVEN_WIELD];

	    /* you can't create an ego weapon from a cursed */
	    /* object.  the curse would "taint" the magic -CFT */
	    /* you also cannot double enchant anything, or */
	    /* you would lose the first enchantment */
	    /* And you cannot modify artifacts */
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
		}
		else {
		    act = " glows deep, icy blue!";
		    i_ptr->name2 = EGO_FB;
		    i_ptr->flags1 |= (TR1_BRAND_COLD);
		    i_ptr->flags2 |= (TR2_RES_COLD);
		    i_ptr->flags3 |= (TR3_IGNORE_COLD);
		}

		objdes(tmp_str, i_ptr, FALSE);

		message("Your ", 0x02);
		message(tmp_str, 0x02);
		message(act, 0);

		enchant(i_ptr, 3+randint(3), ENCH_TOHIT|ENCH_TODAM);
	    }
	    else {
		if (flush_failure) flush();
		msg_print("The Branding fails.");
	    }
	    break;

	  case 53:	   /* blink */
	    teleport(10);
	    break;

	  case 54:	   /* teleport */
	    teleport((int)(p_ptr->lev * 8));
	    break;

	  case 55:	   /* teleport away */
	    if (!get_dir(NULL, &dir)) return;
	    (void)teleport_monster(dir, char_row, char_col);
	    break;

	  case 56:	   /* teleport level */
	    (void)tele_level();
	    break;
	    
	  case 57:	   /* word of recall */
	    if (p_ptr->word_recall == 0) {
		p_ptr->word_recall = 15 + randint(20);
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
	    if ((spell_worked & (1L << choice)) == 0) {
		spell_worked |= (1L << choice);
		p_ptr->exp += s_ptr->sexp << 2;
		prt_experience();
	    }
	}
	else {
	    if ((spell_worked2 & (1L << (choice - 32))) == 0) {
		 spell_worked2 |= (1L << (choice - 32));
		 p_ptr->exp += s_ptr->sexp << 2;
		 prt_experience();
	    }
	}
    }

    /* Take a turn */
    free_turn_flag = FALSE;

    /* Reduce mana */
    if (s_ptr->smana > p_ptr->cmana) {
	msg_print("You faint from fatigue!");
	p_ptr->paralysis = randint((int)(5 * (s_ptr->smana - p_ptr->cmana)));
	p_ptr->cmana = 0;
	p_ptr->cmana_frac = 0;
	if (randint(3) == 1) {
	    msg_print("You have damaged your health!");
	    (void)dec_stat(A_CON, 15 + randint(10), (randint(3) == 1));
	}
    }
    else {
	p_ptr->cmana -= s_ptr->smana;
    }
    
    /* Display current mana */
    prt_cmana();
    
    /* Hack -- Recalculate bonuses */
    calc_bonuses();
}

