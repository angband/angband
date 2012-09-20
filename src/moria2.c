/* File: moria2.c */

/* Purpose: misc code, mainly to handle player commands */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "angband.h"




/*
 * Moves creature record from one space to another	-RAK-	
 * Hack -- Any "monster" in the destination grid is forgotten.
 */
void move_rec(int y1, int x1, int y2, int x2)
{
    /* Must be some movement */
    if ((y1 != y2) || (x1 != x2)) {

	int m_idx = cave[y1][x1].m_idx;

	/* No monster is at the old location */
	cave[y1][x1].m_idx = 0;

	/* Hack -- erase the old grid */
	lite_spot(y1, x1);

	/* Copy the monster index */
	cave[y2][x2].m_idx = m_idx;

	/* Move the player */
	if (m_idx == 1) {
	    char_row = y2;
	    char_col = x2;
	}

	/* Move a monster */
	else {
	    m_list[m_idx].fy = y2;
	    m_list[m_idx].fx = x2;
	}

	/* Hack -- draw the new grid */
	lite_spot(y2, x2);
    }
}



/*
 * Hack -- Check if a level is a "quest" level
 */
int is_quest(int level)
{
    int i;
    if (!level) return (FALSE);
    for (i = 0; i < QUEST_MAX; i++) {
	if (q_list[i].level == level) return TRUE;
    }
    return FALSE;
}



/*
 * Player hit a real trap. -RAK-
 * No longer includes stores.
 * No longer includes secret doors.
 */
void hit_trap(int y, int x)
{
    int                   i, ty, tx, num, dam;
    register cave_type   *c_ptr;
    register inven_type  *i_ptr;
    bigvtype              tmp;

    /* Disturb the player */
    disturb(0, 0);

    /* Get the cave grid */
    c_ptr = &cave[y][x];

    /* Get the trap */
    i_ptr = &i_list[c_ptr->i_idx];

    /* Make the trap "visible" */
    i_ptr->tval = TV_VIS_TRAP;

    /* Paranoia -- redraw the grid */
    lite_spot(y, x);

    /* Roll for damage */
    dam = damroll(i_ptr->dd, i_ptr->ds);

    /* Examine the trap sub-val */
    switch (i_ptr->sval) {

      case SV_TRAP_PIT:
	msg_print("You fell into a pit!");
	if (p_ptr->ffall) {
	    msg_print("You float gently to the bottom of the pit.");
	}
	else {
	    objdes(tmp, i_ptr, TRUE);
	    take_hit(dam, tmp);
	}
	break;

      case SV_TRAP_ARROW:
	if (test_hit(125, 0, 0, p_ptr->pac + p_ptr->ptoac, CLA_MISC_HIT)) {
	    objdes(tmp, i_ptr, TRUE);
	    take_hit(dam, tmp);
	    msg_print("An arrow hits you.");
	}
	else {
	    msg_print("An arrow barely misses you.");
	}
	break;

      case SV_TRAP_SPIKED_PIT:

	/* Now here is a nasty trap indeed */
	/* It really makes feather falling important */
	msg_print("You fall into a spiked pit!");

	if (p_ptr->ffall) {
	    msg_print("You float gently to the floor of the pit.");
	    msg_print("You carefully avoid touching the spikes.");
	}

	else {

	    /* Extra spike damage */
	    if (randint(2) == 1) {

		msg_print("You are impaled!");
		dam = dam * 2;
		cut_player(randint(dam));
	    }

	    /* Poisonous spikes */
	    if (randint(3) == 1) {

		msg_print("The spikes are poisoned!");

		if (p_ptr->immune_pois ||
		    p_ptr->resist_pois ||
		    p_ptr->oppose_pois) {
		    msg_print("The poison does not affect you!");
		}

		else {
		    dam = dam * 2;
		    p_ptr->poisoned += randint(dam);
		}
	    }

	    objdes(tmp, i_ptr, TRUE);
	    take_hit(dam, tmp);
	}
	break;

      case SV_TRAP_TRAP_DOOR:
	msg_print("You fell through a trap door!");
	new_level_flag = TRUE;
	dun_level++;
	if (p_ptr->ffall) {
	    msg_print("You float gently down to the next level.");
	}
	else {
	    objdes(tmp, i_ptr, TRUE);
	    take_hit(dam, tmp);
	}
	/* make sure can see the message before new level */
	msg_print(NULL);
	break;

      case SV_TRAP_GAS_SLEEP:
	if (p_ptr->paralysis == 0) {
	    msg_print("A strange white mist surrounds you!");
	    if (p_ptr->free_act) {
		msg_print("You are unaffected.");
	    }
	    else {
		msg_print("You fall asleep.");
		p_ptr->paralysis += randint(10) + 4;
	    }
	}
	break;

      case SV_TRAP_LOOSE_ROCK:
	delete_object(y, x);
	place_object(y, x);
	msg_print("Hmmm, there was something under this rock.");
	break;

      case SV_TRAP_DART_STR:
	if (test_hit(125, 0, 0, p_ptr->pac + p_ptr->ptoac, CLA_MISC_HIT)) {
	    objdes(tmp, i_ptr, TRUE);
	    take_hit(dam, tmp);
	    if (!p_ptr->sustain_str) {
		(void)dec_stat(A_STR, 10, FALSE);
		msg_print("A small dart weakens you!");
	    }
	    else {
		msg_print("A small dart hits you.");
	    }
	}
	else {
	    msg_print("A small dart barely misses you.");
	}
	break;

      case SV_TRAP_TELEPORT:
	teleport_flag = TRUE;
	msg_print("You hit a teleport trap!");
	break;

      case SV_TRAP_FALLING_ROCK:
	take_hit(dam, "a falling rock");
	/* XXX XXX XXX Should move the player first! */
	/* XXX See the "move_player()" code */
	delete_object(y, x);
	place_rubble(y, x);
	msg_print("You are hit by falling rock.");
	break;

      case SV_TRAP_GAS_ACID:
	msg_print("A strange red gas surrounds you.");
	acid_dam(randint(8), "corrosion gas");
	break;

      case SV_TRAP_SUMMON:
	delete_object(y, x); /* Rune disappears.    */
	num = 2 + randint(3);
	for (i = 0; i < num; i++) {
	    ty = y;
	    tx = x;
	    (void)summon_monster(&ty, &tx, FALSE);
	}
	break;

      case SV_TRAP_FIRE:
	msg_print("You are enveloped in flames!");
	fire_dam(dam, "a fire trap");
	break;

      case SV_TRAP_ACID:
	msg_print("You are splashed with acid!");
	acid_dam(dam, "an acid trap");
	break;

      case SV_TRAP_GAS_POISON:
	msg_print("A pungent green gas surrounds you!");
	poison_gas(dam, "a poison gas trap");
	break;

      case SV_TRAP_GAS_BLIND:
	msg_print("A black gas surrounds you!");
	if (!p_ptr->resist_blind) {
	    p_ptr->blind += randint(50) + 50;
	}
	break;

      case SV_TRAP_GAS_CONFUSE:
	msg_print("A gas of scintillating colors surrounds you!");
	if ((!p_ptr->resist_conf) && (!p_ptr->resist_chaos)) {
	    p_ptr->confused += randint(15) + 15;
	}
	break;

      case SV_TRAP_DART_SLOW:
	if (test_hit(125, 0, 0, p_ptr->pac + p_ptr->ptoac, CLA_MISC_HIT)) {
	    objdes(tmp, i_ptr, TRUE);
	    take_hit(dam, tmp);
	    if (p_ptr->free_act) {
		msg_print("A small dart hits you!");
	    }
	    else {
		msg_print("A small dart hits you!");
		p_ptr->slow += randint(20) + 10;
	    }
	}
	else {
	    msg_print("A small dart barely misses you.");
	}
	break;

      case SV_TRAP_DART_CON:
	if (test_hit(125, 0, 0, p_ptr->pac + p_ptr->ptoac, CLA_MISC_HIT)) {
	    objdes(tmp, i_ptr, TRUE);
	    take_hit(dam, tmp);
	    if (!p_ptr->sustain_con) {
		(void)dec_stat(A_CON, 10, FALSE);
		msg_print("A small dart saps your health!");
	    }
	    else {
		msg_print("A small dart hits you.");
	    }
	}
	else {
	    msg_print("A small dart barely misses you.");
	}
	break;

      case SV_TRAP_GLYPH:
	break;

      default:
	msg_print("Oops. Undefined trap.");
	break;
    }
}


/*
 * Return spell number and failure chance
 *
 * returns -1 if no spells in book
 * returns 1 if choose a spell in book to cast
 * returns 0 if don't choose a spell, i.e. exit with an escape 
 */
int cast_spell(cptr prompt, int item_val, int *sn, int *sc)
{
    u32b               j1, j2, tmp;
    register int         i, k;
    int                  spell[64], result;
    int                  first_spell = -1;
    register spell_type *s_ptr;
    inven_type *i_ptr;

    /* if a warrior, abort as if by ESC -CFT */
    if (!p_ptr->pclass) return 0;

    /* Assume nothing to read */
    result = (-1);

    s_ptr = magic_spell[p_ptr->pclass - 1];

    i_ptr = &inventory[item_val];

    i = 0;

    j1 = i_ptr->flags1 & spell_learned;
    j2 = i_ptr->flags2 & spell_learned2;

    while (j1) {
	k = bit_pos(&j1);
	if (s_ptr[k].slevel <= p_ptr->lev) {
	    spell[i] = k;
	    i++;
	}
    }

    while (j2) {
	k = bit_pos(&j2);
	if (s_ptr[k + 32].slevel <= p_ptr->lev) {
	    spell[i] = k + 32;
	    i++;
	}
    }

    /* No usable spells */
    if (i <= 0) return (-1);

    /* Determine where the "spell listing" should start */
    tmp = i_ptr->flags1;
    first_spell = bit_pos(&tmp);
    if (first_spell < 0) {
	tmp = i_ptr->flags2;
	first_spell = bit_pos(&tmp) + 32;
    }

    /* Ask the user for a spell choice */
    result = get_spell(spell, i, sn, sc, prompt, first_spell);

    /* Verify if needed */
    if (result && magic_spell[p_ptr->pclass - 1][*sn].smana > p_ptr->cmana) {
	cptr q;
	if (class[p_ptr->pclass].spell == MAGE) {
	    q = "You summon your limited strength to cast this one! Confirm?";
	}
	else {
	    q = "The gods may think you presumptuous for this! Confirm?";
	}
	result = get_check(q);
    }

    return (result);
}




/*
 * A monster (possibly a questor) has been destroyed
 *
 * Note that EVERY "questor" monster MUST be a Unique.
 * Note that Questor monsters should NEVER be created out of depth.
 *
 * Note that in a few, very rare, circumstances, killing Morgoth
 * may result in the Iron Crown of Morgoth crushing the Lead-Filled
 * Mace "Grond", since the Iron Crown is more important.
 */
static void check_quest(monster_type *m_ptr)
{
    monster_race *r_ptr = &r_list[m_ptr->r_idx];

    cave_type		*c_ptr;

    int			i;
    int                 cur_pos;
    int                 ty, tx;
    int			ny, nx;

    int total = 0;


    /* Only process "Quest Monsters" */
    if (!(r_ptr->cflags2 & MF2_QUESTOR)) return;


    /* Hack -- Mark quests as complete */
    for (i = 0; i < QUEST_MAX; i++) {

	/* Hack -- note completed quests */
	if (q_list[i].level == r_ptr->level) q_list[i].level = 0;

	/* Count incomplete quests */
	if (q_list[i].level) total++;
    }


    /* Need some stairs */
    if (total) {

	/* Start on the dead monster */
	ty = m_ptr->fy;
	tx = m_ptr->fx;

	/* Stagger around until we find a legal grid */
	while (!valid_grid(ty, tx)) {
	    ny = rand_spread(ty, 1);
	    nx = rand_spread(tx, 1);
	    if (!in_bounds(ny,nx)) continue;
	    ty = ny, tx = nx;
	}

	/* Get the cave location */
	c_ptr = &cave[ty][tx];
	delete_object(ty, tx);

	/* Create a stairway */
	cur_pos = i_pop();
	invcopy(&i_list[cur_pos], OBJ_DOWN_STAIR);

	/* And place it near the dead monster */
	i_list[cur_pos].iy = ty;
	i_list[cur_pos].ix = tx;
	c_ptr->i_idx = cur_pos;

	/* Explain the stairway */
	msg_print("A magical stairway appears...");

	/* Update the view/lite */
	update_view();
	update_lite();
	
	/* Update the monsters */
	update_monsters();
    }


    /* Nothing left, game over... */
    else {

	total_winner = TRUE;
	prt_winner();
	msg_print("*** CONGRATULATIONS ***");
	msg_print("You have won the game!");
	msg_print("You cannot save this game...");
	msg_print("But you may retire when you are ready.");
    }
}




/*
 * Allocates basic objects upon a creatures death.
 *
 * Disperse treasures centered at the monster location based on the
 * various flags contained in the monster flags fields.  If "examine"
 * is FALSE, that is, the monster was killed by rubble or something,
 * do NOT look at the GOOD or SPECIAL or WINNER flags.
 *
 * Note first that Unique's cannot be killed by other monsters, so we
 * do not have to worry about "great treasure" disappearing.
 *
 * It looks like there may be a problem if a monster defines SPECIAL
 * without also defining GOOD.  I will have to check... XXX XXX
 */
void monster_death(monster_type *m_ptr, bool examine, bool visible)
{
    int			i, y1, x1, number;

    monster_race *r_ptr = &r_list[m_ptr->r_idx];

    bool good = (examine && (r_ptr->cflags2 & MF2_GOOD));
    bool great = (examine && (r_ptr->cflags2 & MF2_SPECIAL));
    bool winner = (examine && (r_ptr->cflags1 & MF1_WINNER));

    int y = (int)m_ptr->fy;
    int x = (int)m_ptr->fx;

    bool	do_item = (r_ptr->cflags1 & MF1_CARRY_OBJ) ? TRUE : FALSE;
    bool	do_gold = (r_ptr->cflags1 & MF1_CARRY_GOLD) ? TRUE : FALSE;

    int		dump_item = 0;
    int		dump_gold = 0;


    /* Determine how much we can drop */
    number = 0;
    if ((r_ptr->cflags1 & MF1_HAS_60) && (randint(100) < 60)) number++;
    if ((r_ptr->cflags1 & MF1_HAS_90) && (randint(100) < 90)) number++;
    if (r_ptr->cflags1 & MF1_HAS_1D2) number += randint(2);
    if (r_ptr->cflags1 & MF1_HAS_2D2) number += damroll(2, 2);
    if (r_ptr->cflags1 & MF1_HAS_4D2) number += damroll(4, 2);

    /* Dump stuff */
    if (number > 0) {

	/* Drop some objects */    
	for ( ; number > 0; --number) {

	    /* Try 20 times per item, increasing range */
	    for (i = 0; i < 20; ++i) {

		int d = (i + 15) / 10;
		
		/* Pick a location */
		y1 = rand_spread(y, d);
		x1 = rand_spread(x, d);

		/* Must be a legal grid */
		if (!in_bounds(y1, x1)) continue;

		/* Must be "clean" floor grid */
		if (!clean_grid_bold(y1, x1)) continue;

		/* Must be visible to dead monster */
		if (!los(y, x, y1, x1)) continue;

		/* Average dungeon and monster levels */
		object_level = (dun_level + r_ptr->level) / 2;

		/* Place something and count it if seen */                
		if (good) {
		    place_good(y1, x1, great);
		    if (test_lite(y1, x1)) dump_item++;
		}
		else if (do_gold && (randint(2) == 1)) {
		    place_gold(y1, x1);
		    if (test_lite(y1, x1)) dump_gold++;
		}
		else if (do_item) {
		    place_object(y1, x1);
		    if (test_lite(y1, x1)) dump_item++;
		}
		else if (do_gold) {
		    place_gold(y1, x1);
		    if (test_lite(y1, x1)) dump_gold++;
		}

		/* Reset the object level */
		object_level = dun_level;

		/* Actually display the object's grid */
		lite_spot(y1, x1);

		break;
	    }
	}
    }



    /* Mega-Hack -- defeat the "WINNER" monster */
    if (winner) {

	/* Hack -- an "object holder" */
	inven_type prize;


	/* Prepare to make "Grond" */
	invcopy(&prize, OBJ_GROND);

	/* Actually create "Grond" */
	make_artifact(&prize);

	/* Drop it in the dungeon */
	drop_near(&prize, -1, y, x);


	/* Prepare to make "Morgoth" */
	invcopy(&prize, OBJ_MORGOTH);

	/* Actually create "Morgoth" */
	make_artifact(&prize);

	/* Drop it in the dungeon */
	drop_near(&prize, -1, y, x);
    }


    /* Take note of any dropped treasure */
    if (visible && (dump_item || dump_gold)) {

	/* Take notes on treasure */
	lore_treasure(m_ptr, dump_item, dump_gold);
    }
}




/*
 * Hack -- pass a fear code around
 * Used to redo monster fear messages -CWS
 */
static int monster_is_afraid = 0;


/*
 * return whether a monster is "fearless" and will never run away. -CWS
 */
static int fearless(monster_race *r_ptr)
{
    int flag = FALSE;

    /* NoMind --> NoFear */
    if (r_ptr->cflags2 & MF2_MINDLESS) {
	flag = TRUE;
    }

    /* Undead --> (Spells = Mind --> Fear) + (NoSpells = NoMind --> NoFear) */
    if (r_ptr->cflags2 & MF2_UNDEAD) {
	flag = (!(r_ptr->spells1 || r_ptr->spells2 || r_ptr->spells3));
    }

    /* The 'E' and 'g' monsters have NoFear */
    if (r_ptr->r_char == 'E' || r_ptr->r_char == 'g') {
	flag = TRUE;
    }

    /* Demons have NoFear */
    if (r_ptr->cflags2 & MF2_DEMON) {
	flag = TRUE;
    }

    /* But intelligence --> Fear */
    if (r_ptr->cflags2 & MF2_INTELLIGENT) {
	flag = FALSE;
    }

    /* XXX Hack -- No "Normal Move" --> NoFear */
    if (!(r_ptr->cflags1 & MF1_MV_ATT_NORM)) {
	flag = TRUE;
    }

    /* Result */    
    return (flag);
}


/*
 * Decreases monsters hit points and deletes monster if needed.
 * added fear (DGK) and check whether to print fear messages -CWS
 *
 * Genericized name, sex, and capitilization -BEN- 
 *
 * Note that the player will get experience for any monsters killed here.
 * We return "TRUE" in this case, meaning "call prt_experience() now...".
 *
 * Note that it used to be technically possible to loop forever attempting
 * to find an empty, non-artifact location to drop the magical stairway
 * (consider an earthquake, plus the dropping of an artifact by the monster).
 *
 * The new algorithm staggers around until it finds a non-artifact to nuke.
 * This usually means it will destroy a wall, but hey, its a magic stairway.
 */
bool mon_take_hit(int m_idx, int dam, bool print_fear)
{
    int			    r_idx;
    s32b                   new_exp, new_exp_frac;
    int                     percentage;
    bool		    seen;

    register monster_type  *m_ptr;
    register monster_race *r_ptr;

    char                    m_name[80];
    char                    m_poss[80];
    vtype                   out_val;

    /* Get the creature */
    m_ptr = &m_list[m_idx];

    /* Save the race index, get the race */
    r_idx = m_ptr->r_idx;
    r_ptr = &r_list[r_idx];

    /* Get the monster name (or "it") */
    monster_desc(m_name, m_ptr, 0);

    /* Get the monster possessive, applying gender if visible */
    monster_desc(m_poss, m_ptr, 0x22);

    /* Hurt it, and wake it up */
    m_ptr->hp -= dam;
    m_ptr->csleep = 0;

    /* It is dead now */
    if (m_ptr->hp < 0) {

	/* Delete ghost file */
	if (m_ptr->r_idx == (MAX_R_IDX-1)) {

	    char                tmp[1024];

	    if (!dun_level) {
		sprintf(tmp, "%s%s%d", ANGBAND_DIR_BONES, PATH_SEP, r_ptr->level);
	    }
	    else {
		sprintf(tmp, "%s%s%d", ANGBAND_DIR_BONES, PATH_SEP, dun_level);
	    }

	    unlink(tmp);
	}

	/* Determine if the "lore" should be updated */
	seen = ((p_ptr->blind < 1 && m_ptr->ml) ||
		(r_ptr->cflags2 & MF2_UNIQUE));

	/* Generate treasure (handle creeping coins) */
	coin_type = get_coin_type(r_ptr);
	monster_death(m_ptr, TRUE, seen);
	coin_type = 0;

	/* Give some experience */
	new_exp = ((long)r_ptr->mexp * r_ptr->level) / p_ptr->lev;
	new_exp_frac = ((((long)r_ptr->mexp * r_ptr->level) % p_ptr->lev)
			* 0x10000L / p_ptr->lev) + p_ptr->exp_frac;

	if (new_exp_frac >= 0x10000L) {
	    new_exp++;
	    p_ptr->exp_frac = new_exp_frac - 0x10000L;
	}
	else {
	    p_ptr->exp_frac = new_exp_frac;
	}

	p_ptr->exp += new_exp;

	/* When drained, the player advances at 10% the normal rate */
	if (p_ptr->exp < p_ptr->max_exp) {
	    p_ptr->max_exp += new_exp/10;
	}


	/* Check to see if a quest (or the game) is complete */
	check_quest(m_ptr);


	/* Delete the monster, decrement the "current" population */
	delete_monster_idx(m_idx);

	/* When the player kills a Unique, it stays dead */
	if (r_ptr->cflags2 & MF2_UNIQUE) {
	    l_list[r_idx].max_num = 0;
	    seen = TRUE;
	}

	/* Recall even invisible uniques or winners */
	if (seen) {

	    /* Count the number of times the monster has been killed */
	    if (l_list[r_idx].r_kills < MAX_SHORT) {
		l_list[r_idx].r_kills++;
	    }

	    /* Auto-recall if possible */
	    if (use_recall_win && term_recall) {
		roff_recall(r_idx);
	    }
	}

	/* No monster, so no fear */
	monster_is_afraid = 0;

	/* Monster is dead */
	return (TRUE);
    }


    /* Check for fear (unless fearless) */    
    if (!fearless(r_ptr)) {

	percentage = (100L * m_ptr->hp) / m_ptr->maxhp;

	/*
	 * Run if at 10% or less of max hit points, or got hit for half its
	 * current hit points -DGK 
	 */
	if (!(m_ptr->monfear) &&
	    ((percentage <= 10 && randint(10) <= percentage) ||
	     (dam >= m_ptr->hp))) {

	    /* Hack -- note fear */             
	    monster_is_afraid = 1;

	    /* Take note */
	    if (print_fear && m_ptr->ml && m_ptr->los) {
		sprintf(out_val, "%s flees in terror!", m_name);
		message(out_val,0x01);
	    }

	    /* Timed fear */
	    m_ptr->monfear = (randint(10) +
			      ((dam >= m_ptr->hp && percentage > 7) ?
			       20 : (11 - percentage) * 5));
	}

	/* Already afraid */
	else if (m_ptr->monfear) {

	    /* When hurt, get brave? */
	    m_ptr->monfear -= randint(dam);

	    /* No longer afraid */
	    if (m_ptr->monfear <= 0) {
		if (monster_is_afraid == 1) monster_is_afraid = (-1);
		m_ptr->monfear = 0;
		if (m_ptr->ml && print_fear) {
		    message(m_name, 0x03);
		    message(" recovers ", 0x02);
		    message(m_poss, 0x02);
		    message(" courage.", 0);
		}
	    }
	}
    }

    /* Not dead yet */
    return (FALSE);
}


/*
 * Player attacks a (poor, defenseless) creature	-RAK-	 
 */
void py_attack(int y, int x)
{
    register int        k, blows;
    int                 cr_idx, r_idx, tot_tohit, base_tohit;
    vtype               out_val;
    char		m_name[80];
    char		m_poss[80];
    register inven_type    *i_ptr;

    cr_idx = cave[y][x].m_idx;
    r_idx = m_list[cr_idx].r_idx;

    m_list[cr_idx].csleep = 0;
    i_ptr = &inventory[INVEN_WIELD];

    /* Extract monster name (or "it") and possessive */
    monster_desc(m_name, &m_list[cr_idx], 0);

    /* Extract monster possessive (or "its") using gender if visible */
    monster_desc(m_poss, &m_list[cr_idx], 0x22);


    /* Auto-Recall if possible and visible */
    if (use_recall_win && term_recall) {
	if (m_list[cr_idx].ml) roff_recall(r_idx);
    }

    /* Start with base bonus */
    tot_tohit = p_ptr->ptohit;

    /* Proper weapon */
    if (i_ptr->tval) {

	/* Calculate blows */
	blows = attack_blows((int)i_ptr->weight);

	/* Good weapon yields bonus to hit */
	tot_tohit += inventory[INVEN_WIELD].tohit;
    }

    /* Fists */
    else {

	/* Two blows */
	blows = 2;

	/* Hard to hit */
	tot_tohit -= 3;
    }

    /* If creature is lit, use base rates, else, make it harder to hit */
    if (m_list[cr_idx].ml) {
	base_tohit = p_ptr->bth;
    }
    else {
	base_tohit = (p_ptr->bth / 2) - (tot_tohit * (BTH_PLUS_ADJ - 1)) -
	             (p_ptr->lev * class_level_adj[p_ptr->pclass][CLA_BTH] / 2);
    }

    /* Assume no fear messages need to be redone */
    monster_is_afraid = 0;

    /* Loop for number of blows, trying to hit the critter. */
    do {

	bool do_quake = FALSE;
	
	/* We hit it! */
	if (test_hit(base_tohit, (int)p_ptr->lev, tot_tohit,
		     (int)r_list[r_idx].ac, CLA_BTH)) {

	    /* Normal weapon.  Hack -- handle "earthquake brand" */
	    if (i_ptr->tval) {
		k = damroll(i_ptr->dd, i_ptr->ds);
		k = tot_dam(i_ptr, k, r_idx);
		if ((i_ptr->flags1 & TR1_IMPACT) && (k > 50)) do_quake = TRUE;
		k = critical_blow((int)i_ptr->weight, tot_tohit, k, CLA_BTH);
		k += i_ptr->todam;
	    }

	    /* Bare hands */
	    else {
		k = damroll(1, 1);
		k = critical_blow(1, 0, k, CLA_BTH);
	    }

	    /* Apply the player damage bonuses */
	    k += p_ptr->ptodam;

	    /* No negative damage (no "sword of healing!") */
	    if (k < 0) k = 0;


	    /* Boring message */
	    if (!wizard) {
		(void) sprintf(out_val, "You hit %s.", m_name);
		msg_print(out_val);
	    }

	    /* Complex message */
	    else {
		(void)sprintf(out_val,
			      "You hit %s with %d hp, doing %d damage.",
			      m_name, m_list[cr_idx].hp, k);
		msg_print(out_val);
	    }

	    /* Confusion attack */
	    if (p_ptr->confusing) {
		p_ptr->confusing = FALSE;
		msg_print("Your hands stop glowing.");
		if ((r_list[r_idx].cflags2 & MF2_CHARM_SLEEP) ||
		    (randint(100) < r_list[r_idx].level)) {
		    (void)sprintf(out_val, "%s is unaffected.", m_name);
		}
		else {
		    (void)sprintf(out_val, "%s appears confused.", m_name);
		    m_list[cr_idx].confused = TRUE;
		}

		/* Uppercase and display the sentence */
		message(out_val,0x01);

		if (m_list[cr_idx].ml && randint(4) == 1) {
		    l_list[r_idx].r_cflags2 |=
			r_list[r_idx].cflags2 & MF2_CHARM_SLEEP;
		}
	    }

	    /* Is it dead yet? */
	    if (mon_take_hit(cr_idx, k, FALSE)) {

		if ((r_list[r_idx].cflags2 & MF2_DEMON) ||
		    (r_list[r_idx].cflags2 & MF2_UNDEAD) ||
		    (r_list[r_idx].cflags2 & MF2_MINDLESS) ||
		    (strchr("EvgX", r_list[r_idx].r_char))) {
		    (void)sprintf(out_val, "You have destroyed %s.", m_name);
		}
		else {
		    (void)sprintf(out_val, "You have slain %s.", m_name);
		}
		msg_print(out_val);
		prt_experience();

		/* No more attacks */
		blows = 0;
	    }

#if 0	    
	    /* Hack -- test for possible weapon breakage */
	    if (FALSE) {
		message("Your weapon breaks!", 0);
		invcopy(&inventory[INVEN_WIELD], OBJ_NOTHING);
		blows = 0;
	    }
#endif


	    /* Hack -- apply earthquake brand */
	    if (do_quake) earthquake();
	}

	else {
	    (void)sprintf(out_val, "You miss %s.", m_name);
	    msg_print(out_val);
	}

	blows--;
    }
    while (blows > 0);


    /* Hack -- delay the fear messages until here */
    if (monster_is_afraid == 1) {
	sprintf(out_val, "%s flees in terror!", m_name);
	message(out_val,0x01);
    }
    if (monster_is_afraid == -1) {
	sprintf(out_val, "%s recovers %s courage.", m_name, m_poss);
	message(out_val,0x01);
    }
}






/*
 * Obtain the "facts" about a thrown object (or missile), taking into
 * account factors such as the current bow.
 *
 * Extract base chance to hit, bonus to hit, total damage,
 * the maximum distance, and the maximum number of shots.
 *
 * The separation of normal weapons from missile launchers via the
 * "bow slot" of 2.7.4 allowed simplification of the missile code below.
 */
static void facts(inven_type *i_ptr, \
		  int *tbth, int *tpth, int *tdam, int *tdis, int *thits)
{
    register int scatter, tmp_weight;

    /* Get the "bow" (if any) */
    inven_type *j_ptr = &inventory[INVEN_BOW];

    /* Paranoia -- everything has weight */
    tmp_weight = MY_MAX(1,i_ptr->weight);

    /* Damage from thrown object */
    *tdam = damroll(i_ptr->dd, i_ptr->ds) + i_ptr->todam;

    /* Base chance to hit */
    *tbth = p_ptr->bthb * 3 / 4;

    /* Plusses to hit */
    *tpth = p_ptr->ptohit + i_ptr->tohit;

    /* Distance based on strength */
    *tdis = (((p_ptr->use_stat[A_STR] + 20) * 10) / tmp_weight);

    /* Max distance of 10, no matter how strong */
    if (*tdis > 10) *tdis = 10;

    /* Default to single shot or throw */
    *thits = 1;


    /* Hack -- Rangers get multiple shots with a bow and arrow */
    if ((p_ptr->pclass == 4) &&
	(i_ptr->tval == TV_ARROW) &&
	(j_ptr->tval == TV_BOW) &&
	((j_ptr->sval == SV_SHORT_BOW) ||
	 (j_ptr->sval == SV_LONG_BOW))) {

	 /* Give the Ranger some extra shots */
	 *thits = attack_blows(j_ptr->weight) / 2;

	 /* Never lose shots */
	 if (*thits < 1) *thits = 1;
    }



    /* Handle Firing a missile while wielding the proper launcher */
    /* The maximum range is increased, the launcher modifiers are */
    /* added in, and then the bow multiplier is applied.  Note that */
    /* Bows of "Extra Might" get extra range and an extra bonus for */
    /* the damage multiplier, and Bows of "Extra Shots" give an extra */
    /* shot.  These only work when the proper missile is used.        */

    /* Examine the launcher */
    if (j_ptr->tval == TV_BOW) {

	/* Extract the "Extra Might" flag */
	bool xm = (j_ptr->flags3 & TR3_XTRA_MIGHT) ? TRUE : FALSE;

	/* Extract the "Extra Shots" flag */
	bool xs = (j_ptr->flags3 & TR3_XTRA_SHOTS) ? TRUE : FALSE;

	/* Analyze the launcher */
	switch (j_ptr->sval) {

	  /* Sling and ammo */
	  case SV_SLING:
	    if (i_ptr->tval != TV_SHOT) break;
	    *tbth = p_ptr->bthb;
	    *tpth += j_ptr->tohit;
	    *tdam += j_ptr->todam;
	    *tdam *= (xm ? 3 : 2);
	    *tdis = (xm ? 25 : 20);
	    if (xs) *thits += 1;
	    break;

	  /* Short Bow and Arrow */
	  case SV_SHORT_BOW:
	    if (i_ptr->tval != TV_ARROW) break;
	    *tbth = p_ptr->bthb;
	    *tpth += j_ptr->tohit;
	    *tdam += j_ptr->todam;
	    *tdam *= (xm ? 3 : 2);
	    *tdis = (xm ? 30 : 25);
	    if (xs) *thits += 1;
	    break;

	  /* Long Bow and Arrow */
	  case SV_LONG_BOW:
	    if (i_ptr->tval != TV_ARROW) break;
	    *tbth = p_ptr->bthb;
	    *tpth += j_ptr->tohit;
	    *tdam += j_ptr->todam;
	    *tdam *= (xm ? 4 : 3);
	    *tdis = (xm ? 35 : 30);
	    if (xs) *thits += 1;
	    break;

	  /* Light Crossbow and Bolt */
	  case SV_LIGHT_XBOW:
	    if (i_ptr->tval != TV_BOLT) break;
	    *tbth = p_ptr->bthb;
	    *tpth += j_ptr->tohit;
	    *tdam += j_ptr->todam;
	    *tdam *= (xm ? 4 : 3);
	    *tdis = (xm ? 35 : 25);
	    if (xs) *thits += 1;
	    break;

	  /* Heavy Crossbow and Bolt */
	  case SV_HEAVY_XBOW:
	    if (i_ptr->tval != TV_BOLT) break;
	    *tbth = p_ptr->bthb;
	    *tpth += j_ptr->tohit;
	    *tdam += j_ptr->todam;
	    *tdam *= (xm ? 5 : 4);
	    *tdis = (xm ? 40 : 30);
	    if (xs) *thits += 1;
	    break;
	}
    }

    /* Hack -- Apply a small "scatter effect" to the maximum range -BEN- */
    scatter = (*tdis+8) / 16;
    if (scatter) *tdis = rand_spread(*tdis, scatter);
}


/*
 * Let an item 'i_ptr' fall to the ground at or near (y,x).
 * The initial location is assumed to be "in_bounds()".
 *
 * This function takes a parameter "chance".  This is the percentage
 * chance that the item will "disappear" instead of drop.  If the object
 * has been thrown, then this is the chance of disappearance on contact.
 */
void drop_near(inven_type *i_ptr, int chance, int y, int x)
{
    register int i, j, k, d;
    int x1, y1;
    int flag, cur_pos;
    register cave_type *c_ptr;
    bigvtype out_val, tmp_str;

    /* No place found yet */
    flag = FALSE;


    /* Start at the drop point */
    i = y1 = y;  j = x1 = x;

    /* See if the object "survives" the fall */
    if (artifact_p(i_ptr) || (randint(100) > chance)) {

	/* Start at the drop point */
	i = y1 = y; j = x1 = x;

	/* Try (20 times) to find an adjacent usable location */
	for (k = 0; !flag && (k < 20); ++k) {

	    /* Distance distribution */
	    d = ((k + 14) / 15);
	    
	    /* Pick a "nearby" location */
	    i = rand_spread(y1, d);
	    j = rand_spread(x1, d);

	    /* Ignore "invalid" locations */
	    if (!valid_grid(i, j)) continue;

	    /* Require empty floor space */
	    if (!clean_grid_bold(i, j)) continue;

	    /* Require "los()" */
	    if ((d > 1) && !los(y1, x1, i, j)) continue;

	    /* Here looks good */
	    flag = TRUE;
	}
    }

    /* Try really hard to place an artifact */
    if (!flag && artifact_p(i_ptr)) {

	/* Start at the drop point */
	i = y1 = y;  j = x1 = x;

	/* Try really hard to drop it */
	for (k = 0; !flag && (k < 1000); k++) {

	    /* Move one space in any direction */
	    i = rand_spread(y1, 1);
	    j = rand_spread(x1, 1);

	    /* Do not move through walls */
	    if (!floor_grid_bold(i,j)) continue;

	    /* Hack -- "bounce" to that location */
	    y1 = i;  x1 = j;

	    /* Get the cave grid */
	    c_ptr = &cave[i][j];

	    /* Nothing here?  Use it */
	    if (c_ptr->i_idx == 0) flag = TRUE;

	    /* After trying 99 places, crush any (normal) object */
	    else if ((k>99) && valid_grid(i,j)) flag = TRUE;
	}

	/* XXX Artifacts will destroy ANYTHING to stay alive */
	if (!flag) {
	    i = y, j = x, flag = TRUE;
	    objdes(tmp_str, i_ptr, FALSE);
	    (void)sprintf(out_val, "The %s crashes to the floor.", tmp_str);
	    message(out_val, 0);
	}
    }

    /* Successful drop */
    if (flag) {

	bool old_floor = floor_grid_bold(i, j);

	/* Crush anything under us (for artifacts) */
	delete_object(i,j);

	/* Make a dungeon object based on the given object */
	cur_pos = i_pop();
	cave[i][j].i_idx = cur_pos;
	i_list[cur_pos] = *i_ptr;
	i_list[cur_pos].iy = i;
	i_list[cur_pos].ix = j;

	/* Under the player.  Hack -- no message if "dropped". */
	if (chance && (cave[i][j].m_idx == 1)) {
	    msg_print("You feel something roll beneath your feet.");
	}

	/* Update the display */
	lite_spot(i, j);

	/* Hack -- react to disappearing doors, etc */
	if (old_floor != floor_grid_bold(i, j)) {
	    update_view();
	    update_lite();
	    update_monsters();
	}
    }

    /* Poor little object */
    else {
	objdes(tmp_str, i_ptr, FALSE);
	(void)sprintf(out_val, "The %s disappears.", tmp_str);
	msg_print(out_val);
    }
}


/*
 * Determines the odds of an object breaking when thrown
 * Note that "impact" is true if the object hit a monster
 * Artifacts never break, see the "drop_near()" function.
 */   
static int breakage_chance(inven_type *i_ptr, bool impact)
{
    /* Examine the item type */
    switch (i_ptr->tval) {

      /* Very breakable objects */
      case TV_POTION:
      case TV_FLASK:
      case TV_BOTTLE:
      case TV_FOOD:
	return (impact ? 100 : 50);

      /* Somewhat breakable objects */
      case TV_LITE:
      case TV_SCROLL:
      case TV_ARROW:
      case TV_JUNK:
      case TV_SKELETON:
	return (impact ? 60 : 30);

      /* Slightly breakable objects */
      case TV_WAND:
      case TV_SHOT:
      case TV_BOLT:
      case TV_SPIKE:
	return (impact ? 40 : 20);
    }

    /* Normal objects */
    return (impact ? 20 : 10);
}


/*
 * Fire/throw an object in a direction
 *
 * Note: if target monster is unseen, make it much more difficult to
 * hit, subtract off most bonuses, and reduce bthb depending on distance 
 *
 * Note:  If we're going to fire again, reroll damage for the
 * next missile. This makes each missile's damage more
 * random, AND it doesn't allow damage bonuses to accumulate!
 */
void shoot(int item_val, int dir)
{
    int			i, j, y, x, ny, nx;
    int			tbth, tpth, tdam, tdis, thits;
    int			cur_dis, visible, shot, mshots;
    
    inven_type          throw_obj;
    inven_type		*i_ptr;
    cave_type		*c_ptr;
    monster_type	*m_ptr;

    bool		flag = FALSE;
    
    int			missile_attr;
    int			missile_char;

    char		m_name[80];

    bigvtype            out_val, tmp_str;

    
    /* Get the missile item */
    i_ptr = &inventory[item_val];

    /* Find the color and symbol for the object for throwing */
    missile_attr = inven_attr(i_ptr);
    missile_char = inven_char(i_ptr);

    /* Count the maximum number of shouts */
    mshots = i_ptr->number;

    /* Keep shooting until out of arrows, count the shots */
    for (shot = 0; shot < mshots; shot++) {

	/* Create a "local missile object" */
	throw_obj = *i_ptr;
	throw_obj.number = 1;

	/* Examine the missile (and bow, and class, etc) */
	facts(&throw_obj, &tbth, &tpth, &tdam, &tdis, &thits);

	/* Not everyone can shoot forever */
	if (shot >= thits) break;

	/* Verify "continued" shots (in the same direction) */
	if (shot && (!get_check("Fire/Throw again?"))) break;

	/* Reduce and describe inventory */
	inven_item_increase(item_val,-1);
	inven_item_describe(item_val);
	inven_item_optimize(item_val);

	/* Hack -- Penalize Heavy Crossbows. ??? ??? */
	/* if (inventory[INVEN_BOW].sval == SV_HEAVY_CROSSBOW) tpth -= 10; */

	/* Start at the player */
	y = char_row;
	x = char_col;

	/* Travel until stopped */
	for (cur_dis = 1; cur_dis < tdis; cur_dis++) {

	    /* XXX Hack -- handle "self-target" */
	    if ((dir == 5) || (!dir && target_at(y, x))) break;

	    /* Move the char */
	    ny = y;
	    nx = x;
	    (void)mmove(dir, &ny, &nx);

	    /* Stopped by walls/doors */
	    if (!floor_grid_bold(ny,nx)) break;

	    /* Save the new location */
	    x = nx;
	    y = ny;

	    /* Where did it get to */
	    c_ptr = &cave[y][x];

	    /* Can the player see the missile? */
	    if (player_can_see(y, x)) {

		/* Draw, Hilite, Fresh, Pause, Erase */
		mh_print_rel(missile_char, missile_attr, 0, y, x);
		move_cursor_relative(y, x);
		Term_fresh();
		delay(8 * delay_spd);
		lite_spot(y, x);
	    }


	    /* Monster here, Try to hit it */
	    if (c_ptr->m_idx > 1) {

		/* Get the monster */
		m_ptr = &m_list[c_ptr->m_idx];

		/* Reduce "tbth" by range */
		tbth = tbth - cur_dis;

		/* Check the visibility */
		visible = m_ptr->ml;

		/* Unseen monsters are "hard to hit" (see note above) */
		if (!visible) {

		    /* Reduce "tbth" by some amount */
		    tbth = ((tbth / (cur_dis + 2)) -
			    (tpth * (BTH_PLUS_ADJ - 1)) -
			    (p_ptr->lev *
			     class_level_adj[p_ptr->pclass][CLA_BTHB] / 2));
		}

		/* Did we hit it? */
		if (test_hit(tbth, (int)p_ptr->lev, tpth,
			     (int)r_list[m_ptr->r_idx].ac, CLA_BTHB)) {

		    i = m_ptr->r_idx;

		    /* Describe the object */
		    objdes(tmp_str, &throw_obj, FALSE);

		    /* Get "the monster" or "it" */
		    monster_desc(m_name, m_ptr, 0);

		    /* Describe the result */
		    if (!visible) {
			(void)sprintf(out_val, "The %s finds a mark.",
				      tmp_str);
		    }
		    else {
			(void)sprintf(out_val, "The %s hits %s.",
				      tmp_str, m_name);
		    }
		    msg_print(out_val);

		    /* Apply special damage */
		    tdam = tot_dam(&throw_obj, tdam, i);
		    tdam = critical_blow((int)throw_obj.weight,
					 tpth, tdam, CLA_BTHB);

		    /* No negative damage */
		    if (tdam < 0) tdam = 0;

		    /* Describe the pain/death result.  Display fear msg's */                
		    if (mon_take_hit((int)c_ptr->m_idx, tdam, TRUE)) {
			(void)sprintf(out_val, "You have killed %s!", m_name);
			message(out_val, 0);
			prt_experience();
		    }
		    else {
			cptr fmt;
			fmt = pain_message((int)c_ptr->m_idx,(int)tdam);
			(void)sprintf(out_val, fmt, m_name);
			message(out_val, 0x01);
		    }

		    /* Note the collision */
		    flag = TRUE;
		}

		/* Stop looking */
		break;
	    }
	}

	/* Chance of breakage */
	j = breakage_chance(&throw_obj, flag);

	/* Drop (or break) near that location */
	drop_near(&throw_obj, j, y, x);
    }
}





/*
 * Make a bash attack on someone.  -CJS-
 * Used to be part of bash (below). 
 *
 * This function should probably access "p_ptr->ptohit" and the shield
 * bonus "inventory[INVEN_ARM].tohit".
 */
void py_bash(int y, int x)
{
    int                     monster, k, avg_max_hp, base_tohit, r_idx;
    register monster_type  *m_ptr;
    register monster_race  *r_ptr;
    vtype                   m_name, out_val;

    monster = cave[y][x].m_idx;
    m_ptr = &m_list[monster];
    r_idx = m_ptr->r_idx;
    m_ptr->csleep = 0;

    /* Get the creature pointer, used many times below */
    r_ptr = &r_list[r_idx];

    /* Extract the monster name (or "it") */
    monster_desc (m_name, m_ptr, 0);

    /* Attempt to bash */
    base_tohit = (p_ptr->use_stat[A_STR] +
		  inventory[INVEN_ARM].weight / 2 +
		  p_ptr->wt / 10);

    /* Harder to bash invisible monsters */
    if (!m_ptr->ml) {
	base_tohit = ((base_tohit / 2) - 
	     (p_ptr->use_stat[A_DEX] * (BTH_PLUS_ADJ - 1)) -
	     (p_ptr->lev * class_level_adj[p_ptr->pclass][CLA_BTH] / 2));
    }

    /* Hack -- test for contact */
    if (test_hit(base_tohit, (int)p_ptr->lev,
		 (int)p_ptr->use_stat[A_DEX], (int)r_ptr->ac, CLA_BTH)) {

	(void)sprintf(out_val, "You hit %s.", m_name);
	msg_print(out_val);
	k = damroll(inventory[INVEN_ARM].dd, inventory[INVEN_ARM].ds);
	k = critical_blow((int)(inventory[INVEN_ARM].weight / 4 +
				p_ptr->use_stat[A_STR]), 0, k, CLA_BTH);
	k += p_ptr->wt / 60 + 3;

	/* No negative damage */
	if (k < 0) k = 0;

	/* See if we done it in.				     */
	if (mon_take_hit(monster, k, TRUE)) {

	    /* Appropriate message */
	    if ((r_list[r_idx].cflags2 & MF2_DEMON) ||
		(r_list[r_idx].cflags2 & MF2_UNDEAD) ||
		(r_list[r_idx].cflags2 & MF2_MINDLESS) ||
		(strchr("EvgX", r_list[r_idx].r_char))) {
		(void)sprintf(out_val, "You have destroyed %s.", m_name);
	    }
	    else {
		(void)sprintf(out_val, "You have slain %s.", m_name);
	    }
	    msg_print(out_val);
	    prt_experience();
	}

	else {

	    /* Powerful monsters cannot be stunned */
	    avg_max_hp = ((r_ptr->cflags2 & MF2_MAX_HP) ?
			   (r_ptr->hd[0] * r_ptr->hd[1]) :
			   ((r_ptr->hd[0] * (r_ptr->hd[1] + 1)) >> 1));

	    /* Start the message */
	    message(m_name, 0x03);

	    /* Apply saving throw */
	    if ((100 + randint(400) + randint(400)) >
		(m_ptr->hp + avg_max_hp)) {
		m_ptr->stunned += randint(3) + 1;
		if (m_ptr->stunned > 24) m_ptr->stunned = 24;
		message(" appears stunned!", 0);
	    }
	    else {
		message(" ignores your bash!", 0);
	    }
	}
    }
    else {
	(void)sprintf(out_val, "You miss %s.", m_name);
	msg_print(out_val);
    }

    /* Stumble */
    if (randint(150) > p_ptr->use_stat[A_DEX]) {
	msg_print("You are off balance.");
	p_ptr->paralysis = (1 + randint(2));
    }
}




/*
 * Attacker's level and plusses,  defender's AC 
 * Note: pth could be less than 0 if weapon is too heavy
 * Always miss 1 out of 20, always hit 1 out of 20
 */
int test_hit(int bth, int level, int pth, int ac, int attack_type)
{
    register int i;

    /* Hack -- disturb the player */
    disturb(1, 0);

    /* Hack -- 1 in 20 always miss, 1 in 20 always hit */
    i = randint(20);
    if (i == 1) return (FALSE);
    if (i == 20) return (TRUE);

    /* Calculate the "attack quality" */    
    i = (bth + pth * BTH_PLUS_ADJ +
	 (level * class_level_adj[p_ptr->pclass][attack_type]));

    /* Apply the "pth" against the "ac" */
    if ((i > 0) && (randint(i) > ((3 * ac) / 4))) {
	return TRUE;
    }

    /* Assume miss */    
    return FALSE;
}


/*
 * Decreases players hit points and sets death flag if necessary
 */
void take_hit(int damage, cptr hit_from)
{
    /* Hack -- Apply "invulnerability" */
    if (p_ptr->invuln > 0 && damage < 9000) {
	return;
    }

    /* Hurt the player */
    p_ptr->chp -= damage;

    /* Dead player */
    if (p_ptr->chp < 0) {

	/* Hack -- allow wizard to abort death */
	if ((wizard) && !(get_check("Die?"))) {
	    p_ptr->chp = p_ptr->mhp;
	    prt_chp();
	    msg_print("OK, so you don't die.");
	    return;
	}

	if (!death) {
	    death = TRUE;
	    (void)strcpy(died_from, hit_from);
	    total_winner = FALSE;
	}

	/* Dead */
	return; 
    }

    /* Display the hitpoints */
    prt_chp();

    /* Hack -- hitpoint warning */    
    if (p_ptr->chp <= p_ptr->mhp * hitpoint_warn / 10) {
	msg_print("*** LOW HITPOINT WARNING! ***");
	msg_print(NULL);	/* make sure they see it -CWS */
    }
}


