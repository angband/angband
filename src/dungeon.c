/* File: dungeon.c */ 

/* Purpose: the main command interpreter, updating player status */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "angband.h"


/* ANGBAND game module					-RAK-	 */
/* The code in this section has gone through many revisions, and */
/* some of it could stand some more hard work.	-RAK-	       */

/* It has had a bit more hard work.			-CJS- */

/* And it has had some more... -BEN- */






/*
 * Given an item, return a textual "feeling" about the item.
 * But if the item has already been "felt", return NULL.
 */
static cptr value_check(inven_type *i_ptr)
{
    /* Paranoia -- No item */
    if (i_ptr->tval == TV_NOTHING) return (NULL);

    /* Check for previous feelings */
    if (i_ptr->ident & ID_FELT) return (NULL);

    /* Known items need no feeling */
    if (known2_p(i_ptr)) return (NULL);

    /* The item must be wearable to know about it */
    if (!wearable_p(i_ptr)) return (NULL);


    /* Cursed items (including artifacts/ego-weapons) */
    if (cursed_p(i_ptr)) {

	/* Cursed artifacts are terrible */
	if (artifact_p(i_ptr)) return "terrible";

	/* Cursed ego-items are worthless */
	if (i_ptr->name2) return "worthless";

	/* Cursed */
	return "cursed";
    }


    /* Check for uncursed artifacts */
    if (artifact_p(i_ptr)) return "special";

    /* Check for "good" ego-items */
    if (i_ptr->name2) {

	/* Some of the ego-items are excellent */
	if (i_ptr->name2 < EGO_MIN_WORTHLESS) return "excellent";

	/* The rest are awful */
	return "worthless";
    }


    /* Hack -- Any form of bonus is considered good */
    if ((i_ptr->toac > 0) || (i_ptr->tohit > 0) || (i_ptr->todam > 0)) {
	return "good";
    }

    /* Hack -- "good" digging tools -CFT */
    if ((i_ptr->tval == TV_DIGGING) && (i_ptr->flags1 & TR1_TUNNEL) &&
	(i_ptr->pval > k_list[i_ptr->k_idx].pval)) {
	return "good";
    }

    /* Default to "average" */
    return "average";
}


/*
 * Acquire a "sense" about an item (fighters, rogues, paladins)
 * Note that this "sense" will "pre-empt" normal "feelings".
 */
static void sense_item(int i)
{
    cptr feel;
    inven_type *i_ptr;
    char tmp_str[160];
    char out_val[160];

    /* Get the item */
    i_ptr = &inventory[i];

    /* Mages, Priests, Rangers lose out */
    if (p_ptr->pclass == 1) return;
    if (p_ptr->pclass == 2) return;
    if (p_ptr->pclass == 4) return;

    /* Only wearable items can be sensed */
    if (!wearable_p(i_ptr)) return;

    /* Rings and Amulets cannot be sensed */
    if (i_ptr->tval == TV_LITE) return;
    if (i_ptr->tval == TV_RING) return;
    if (i_ptr->tval == TV_AMULET) return;

    /* Check it for a feeling */
    feel = value_check(i_ptr);

    /* Skip non-feelings */
    if (!feel) return;

    /* Stop everything */
    disturb(0, 0);

    /* Get an object description */
    objdes(tmp_str, i_ptr, FALSE);
    (void)sprintf(out_val,
		  "You feel the %s (%c) you are %s %s %s...",
		  tmp_str, index_to_label(i), describe_use(i),
		  (i_ptr->number == 1) ? "is" : "are", feel);
    message(out_val, 0x04);

    /* We have "felt" it */
    i_ptr->ident |= ID_FELT;

    /* Inscribe it textually */
    inscribe(i_ptr, feel);

    /* Success */
    return;
}


/*
 * Sense the inventory
 *
 *   Class 0 = Warrior
 *   Class 1 = Mage
 *   Class 2 = Priest
 *   Class 3 = Rogue
 *   Class 4 = Ranger
 *   Class 5 = Paladin
 */
static void sense_inventory(void)
{
    int i, i_f, lev2, penalty = 0;

    inven_type *i_ptr;

    char tmp_str[160];
    char out_val[160];


    /* Square the players level */
    lev2 = p_ptr->lev * p_ptr->lev;


    /*** Warriors, Rogues and Paladins get a "great" identifier ***/

    /* Extract a "penalty" */
    if (p_ptr->pclass == 5) penalty = 80;
    if (p_ptr->pclass == 3) penalty = 20;
    if (p_ptr->pclass == 0) penalty = 9;

    /* Warriors, Rogues, Paladins get a "special" identifier */
    if (penalty) {

	/* The feeling just "kicks in" every one in a while */
	if (randint((int)(1000L * penalty / (lev2 + 40)) + 1) == 1) {

	    /* Check everything */
	    for (i = 0; i < INVEN_TOTAL; i++) {

		/* Hack -- Skip "missing" things */
		if (!inventory[i].tval) continue;

		/* Inventory items only get felt 1 in 5 times */
		if ((i < INVEN_WIELD) && (randint(5) != 1)) continue;

		/* Attempt to sense it */
		sense_item(i);
	    }
	}

	/* Do NOT do the "simpler" identification */
	return;
    }


    /*** Mages, Priests, Rangers ***/

    /* Priests use a calculation like "warriors" above */
    if (p_ptr->pclass == 2) {
	if (randint((10000 / (lev2 + 40)) + 1) != 1) return;
    }

    /* Mages/Rangers use a weird formula */
    else {
	if (randint(10 + 12000 / (5 + p_ptr->lev)) != 1) return;
    }

    /* Scan the inventory */
    for (i = 0; i < INVEN_TOTAL; i++) {

	/* Get the item */
	i_ptr = &inventory[i];

	/* Skip non-wearable items */
	if (!wearable_p(i_ptr)) continue;

	/* Paranoia -- No item given */
	if (!i_ptr->tval) continue;

	/* We know about it already, do not tell us again */
	if (i_ptr->ident & ID_FELT) continue;

	/* It is fully known, no information needed */
	if (known2_p(i_ptr)) continue;

	/* We can only feel wearable items */
	if (!wearable_p(i_ptr)) continue;

	/* We cannot "feel" Amulets or Rings or Lites */
	if (i_ptr->tval == TV_LITE) continue;
	if (i_ptr->tval == TV_RING) continue;
	if (i_ptr->tval == TV_AMULET) continue;

	/* Inventory only works 1/5 the time */
	if ((i < INVEN_WIELD) && (randint(5) != 1)) continue;

	/* Mages/Rangers only have a 1/10 chance of feeling */
	if ((p_ptr->pclass != 2) && (randint(10) != 1)) continue;

	/* Default to normal */
	i_f = 0;

	/* Always notice cursed items (including Calris) */
	if (cursed_p(i_ptr)) i_f = -1;

	/* Hack -- Uncursed artifacts feel good */
	else if (artifact_p(i_ptr)) i_f = 1;

	/* Hack -- All "good" ego-items feel good */
	else if (i_ptr->name2 && (i_ptr->name2 < EGO_MIN_WORTHLESS)) i_f = 1;

	/* Sometimes an item just "feels" good */
	else if (i_ptr->tohit>0 || i_ptr->todam>0 || i_ptr->toac>0) i_f = 1;

	/* Skip "unfelt" objects */
	if (!i_f) continue;

	/* Disturb everything */
	disturb(0, 0);

	/* Get an object description */
	objdes(tmp_str, i_ptr, FALSE);
	sprintf(out_val,
		"There's something %s about the %s (%c) you are %s...",
		(i_f > 0 ? "good" : "bad"),
		tmp_str, index_to_label(i), describe_use(i));
	message(out_val, 0x04);

	/* We have "felt" it */
	i_ptr->ident |= ID_FELT;

	/* Inscribe a feeling */
	if (cursed_p(i_ptr)) {
	    inscribe(i_ptr, "cursed");
	}
	else {
	    inscribe(i_ptr, "blessed");
	}
    }
}



/*
 * Regenerate hit points				-RAK-	 
 */
static void regenhp(int percent)
{
    register s32b        new_chp, new_chp_frac;
    int                   old_chp;

    old_chp = p_ptr->chp;
    new_chp = ((long)p_ptr->mhp) * percent + PLAYER_REGEN_HPBASE;
    p_ptr->chp += new_chp >> 16;   /* div 65536 */

    /* check for overflow */
    if (p_ptr->chp < 0 && old_chp > 0) p_ptr->chp = MAX_SHORT;
    new_chp_frac = (new_chp & 0xFFFF) + p_ptr->chp_frac;	/* mod 65536 */
    if (new_chp_frac >= 0x10000L) {
	p_ptr->chp_frac = new_chp_frac - 0x10000L;
	p_ptr->chp++;
    }
    else {
	p_ptr->chp_frac = new_chp_frac;
    }

    /* Must set frac to zero even if equal */
    if (p_ptr->chp >= p_ptr->mhp) {
	p_ptr->chp = p_ptr->mhp;
	p_ptr->chp_frac = 0;
    }

    /* Notice changes */
    if (old_chp != p_ptr->chp) p_ptr->status |= PY_HP;
}


/* 
 * Regenerate mana points				-RAK-	 
 */
static void regenmana(int percent)
{
    register s32b        new_mana, new_mana_frac;
    int                   old_cmana;

    old_cmana = p_ptr->cmana;
    new_mana = ((long)p_ptr->mana) * percent + PLAYER_REGEN_MNBASE;
    p_ptr->cmana += new_mana >> 16;	/* div 65536 */
    /* check for overflow */
    if (p_ptr->cmana < 0 && old_cmana > 0) {
	p_ptr->cmana = MAX_SHORT;
    }
    new_mana_frac = (new_mana & 0xFFFF) + p_ptr->cmana_frac;	/* mod 65536 */
    if (new_mana_frac >= 0x10000L) {
	p_ptr->cmana_frac = new_mana_frac - 0x10000L;
	p_ptr->cmana++;
    }
    else {
	p_ptr->cmana_frac = new_mana_frac;
    }

    /* Must set frac to zero even if equal */
    if (p_ptr->cmana >= p_ptr->mana) {
	p_ptr->cmana = p_ptr->mana;
	p_ptr->cmana_frac = 0;
    }
    
    /* Redraw mana */
    if (old_cmana != p_ptr->cmana) p_ptr->status |= PY_MANA;
}



static void regen_monsters(void)
{
    register int i;
    monster_type *m_ptr;

    /* Regenerate everyone */
    for (i = 0; i < MAX_M_IDX; i++) {

	/* Check the i'th monster */
	m_ptr = &m_list[i];

	/* Paranoia -- Skip "dead" monsters */
	if (m_ptr->hp < 0) continue;

	/* Allow regeneration */
	if (m_ptr->hp < m_ptr->maxhp) {
	    int frac = 2 * m_ptr->maxhp / 100L;
	    if (!frac) frac = 1;
	    m_ptr->hp += frac;
	    if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;
	}
    }
}



/*
 * This is the main function of this file -- it places the user on the
 * current level and processes user input until the level is completed,
 * the user dies, or the game is terminated.
 *
 * Note that "compact_monsters()" is much more likely to succeed
 * here than in creatures.c, so we do it even if there are up to 10
 * monsters slots remaining.  This greatly reduces the chances of
 * failure during monster reproduction.  We do the compaction via
 * "tighten_m_list()" which only compacts if space is tight.
 *
 * This function is "probably" the best place to make sure that "extra"
 * items do not get carried.  That is, if the player somehow manages to
 * place 24 items in his pack, even though this should never happen, then
 * immediate action is required to prevent fatal errors.  Thus we test
 * the pack after every command, and induce "automatic item droppage" if
 * the pack overflows.  Note that no monster action can cause the pack to
 * gain items.  Make sure "disenchant" never affects individual pack items.
 */ 
void dungeon(void)
{
    int                    i;

    register cave_type		*c_ptr;
    register inven_type		*i_ptr;

    /* Regenerate hp and mana */
    int                    regen_amount;


    /* Reset flags and initialize variables (most of it is overkill) */
    new_level_flag	= FALSE;
    teleport_flag	= FALSE;
    find_flag		= 0;

    /* Reset the "command" vars (again, mostly overkill) */
    command_cmd		= 0;
    command_old		= 0;
    command_esc		= 0;
    command_rep		= 0;
    command_arg		= 0;
    command_dir		= -1;

    /* Reset the "object generation" level */
    object_level = dun_level;
    
#ifdef TARGET
    /* target code taken from Morgul -CFT */
    target_mode = FALSE;
#endif

#ifdef USE_MULTIHUED
    /* Forget all of the old Multi-Hued things */
    mh_forget();	
#endif


    /* Turn off searching */
    search_off();


#ifdef NEW_MAPS
    if (TRUE) {
	int x, y;

	/* Forget all of the "map memory" */
	for (y = 0; y < cur_height; y++) {
	    for (x = 0; x < cur_width; x++) {
		/* Start with a blank map */
		cave[y][x].mchar = ' ';
		cave[y][x].mattr = TERM_WHITE;
	    }
	}
    }    
#endif


    /* Remember deepest dungeon level visited */
    if (dun_level > (unsigned)(p_ptr->max_dlv)) {
	p_ptr->max_dlv = dun_level;
    }


    /* Paranoia -- no stairs from town */
    if (!dun_level) create_down_stair = create_up_stair = FALSE;

    /* No stairs down from Quest */
    if (is_quest(dun_level)) create_down_stair = FALSE;

    /* Make a stairway. */
    if (create_up_stair || create_down_stair) {

	register int        cur_pos;

	c_ptr = &cave[char_row][char_col];
	i_ptr = &i_list[c_ptr->i_idx];

	/* Try to convert up-stairs into down-stairs */
	if (i_ptr->tval == TV_UP_STAIR) {
	    if (create_down_stair) {
		invcopy(i_ptr, OBJ_DOWN_STAIR);
		i_ptr->iy = char_row;
		i_ptr->ix = char_col;
	    }
	}

	/* Try to convert down-stairs into stairs */
	else if (i_ptr->tval == TV_DOWN_STAIR) {
	    if (create_up_stair) {
		invcopy(i_ptr, OBJ_UP_STAIR);
		i_ptr->iy = char_row;
		i_ptr->ix = char_col;
	    }
	}

	/* Normal objects are obliterated  */
	else if (valid_grid(char_row, char_col)) {
	    delete_object(char_row, char_col);
	    cur_pos = i_pop();
	    i_ptr = &i_list[cur_pos];
	    c_ptr->i_idx = cur_pos;
	    if (create_up_stair) {
		invcopy(i_ptr, OBJ_UP_STAIR);
		i_ptr->iy = char_row;
		i_ptr->ix = char_col;
	    }
	    else if (create_down_stair) {
		invcopy(i_ptr, OBJ_DOWN_STAIR);
		i_ptr->iy = char_row;
		i_ptr->ix = char_col;
	    }
	}

	/* Cancel the stair request */
	create_down_stair = FALSE;
	create_up_stair = FALSE;
    }


    /* Get new panel, draw map */
    (void)get_panel(char_row, char_col, TRUE);

    /* Update the view */
    update_view();

    /* Update the lite */
    update_lite();

    /* Update the monsters */
    update_monsters();

    /* Check the view */
    check_view();


    /* Print the depth */
    prt_depth();

    /* Announce (or repeat) the feeling, unless in town */
    if (dun_level) do_cmd_feeling();


    /* Check the mana occasionally just to stay in touch */
    if (class[p_ptr->pclass].spell == MAGE) {
	calc_spells(A_INT);
	calc_mana(A_INT);
    }
    else if (class[p_ptr->pclass].spell == PRIEST) {
	calc_spells(A_WIS);
	calc_mana(A_WIS);
    }

    /* Calculate bonuses */
    calc_bonuses();


    /*** Process this dungeon level ***/


    /* Loop until the character, level, or game, dies */

    while (1) {


	/* Notice death and new levels */
	if (death || new_level_flag) break;


	/*** Process the player ***/
	
	/* Give the player some energy */
	p_ptr->energy += extract_energy(p_ptr->pspeed);
	
	/* Hack -- ignore the player when he has no energy */
	if (p_ptr->energy < 100) {

	    /* Just process the monsters */
	    process_monsters();
	    
	    /* And wait until the next loop */
	    continue;
	}
	

	/*** Handle real turns for the player ***/
	
	/* Advance the turn counter */
	turn++;

	/* Hack -- small "energy boost" (see "creature.c") */
	p_ptr->energy += rand_int(5);



	/*** Check the Load ***/

#ifdef CHECK_HOURS
	/* Check for game hours			       */
	if (((turn % 100) == 1) && !check_time()) {
	    if (closing_flag > 2) {
		msg_print("The gates to ANGBAND are now closed.");
		(void)strcpy(died_from, "(closing gate: saved)");
		if (save_player()) quit(NULL);
		(void)strcpy(died_from, "a slammed gate");
		death = TRUE;
		exit_game();
	    }
	    else {
		disturb(0, 0);
		closing_flag++;
		msg_print("The gates to ANGBAND are closing due to high load.");
		msg_print("Please finish up or save your game.");
	    }
	}
#endif

	/*** Update the Stores ***/

	/* Hack -- Daybreak/Nighfall in town */
	if (!dun_level && (turn % (TOWN_DAWN / 2) == 0)) {

	    int y, x;
	    bool dawn;

	    /* Check for dawn */
	    dawn = (!(turn % TOWN_DAWN));

	    /* Night falls */
	    if (dawn) {
		msg_print("The sun has risen.");
	    }

	    /* Day breaks */
	    else {
		msg_print("The sun has fallen.");
	    }

	    /* Hack -- Scan the town, switch the visibility */
	    for (y = 0; y < cur_height; y++) {
		for (x = 0; x < cur_width; x++) {

		    /* Get the cave grid */
		    c_ptr = &cave[y][x];

		    /* Assume lit and marked */
		    c_ptr->info |= CAVE_PL;
		    c_ptr->info |= CAVE_FM;

		    /* All done if dawn */
		    if (dawn) continue;

		    /* Skip real walls */
		    if (!floor_grid_bold(y, x)) continue;

		    /* Skip the doors/stairs */
		    i_ptr = &i_list[c_ptr->i_idx];
		    if (i_ptr->tval == TV_STORE_DOOR) continue;
		    if (i_ptr->tval == TV_DOWN_STAIR) continue;

		    /* Hack -- Make everything else dark */
		    c_ptr->info &= ~CAVE_PL;
		    c_ptr->info &= ~CAVE_FM;
		}
	    }

	    /* Redraw the map */
	    prt_map();
	}


	/*** Update the Stores ***/
	
	/* Perhaps only/always do this at dawn? */
	/* Update the stores once a day */
	if ((dun_level) && ((turn % STORE_TURNS) == 0)) {

	    if (wizard || peek) msg_print("Updating Stores...");

	    store_maint();

	    if (shuffle_owners && randint(STORE_SHUFFLE) == 1) {
		if (wizard || peek) msg_print("Shuffling a Store...");
		store_shuffle();
	    }

	    if (wizard || peek) msg_print("Done.");
	}


	/*** Make, and Heal, the Monsters ***/

	/* Check for creature generation */
	if (randint(MAX_M_ALLOC_CHANCE) == 1) {
	    alloc_monster(1, MAX_SIGHT + 5, FALSE);
	}

	/* Check for creature regeneration */
	if (!(turn % 20)) regen_monsters();


	/*** Handle the Lights ***/

	/* Check for light being wielded */
	i_ptr = &inventory[INVEN_LITE];

	/* Extract "max radius" from current lite */
	if (i_ptr->tval == TV_LITE) {

	    /* Use some fuel (unless "permanent") */
	    if (!(i_ptr->flags3 & TR3_LITE) && (i_ptr->pval > 0)) {

		/* Decrease life-span */
		i_ptr->pval--;

		/* Hack -- Special treatment when blind */
		if (p_ptr->blind > 0) {
		    /* Hack -- save some light for later */
		    if (i_ptr->pval == 0) i_ptr->pval++;
		}

		/* The light is now out */
		else if (i_ptr->pval == 0) {
		    disturb(0, 0);
		    msg_print("Your light has gone out!");
		}

		/* The light is getting dim */
		else if ((i_ptr->pval < 40) && (randint(5) == 1)) {
		    disturb(0, 0);
		    msg_print("Your light is growing faint.");
		}
	    }
	}

	/* Update the lite (and the lite radius) */
	update_lite();

	/* Any "view/lite" change should induce "update_monsters()" */
	if (old_lite != cur_lite) {

	    /* Update the monsters */
	    update_monsters();
	}

	/* Sudden loss of lite (can happen from "blind" too) */
	if ((old_lite > 0) && (cur_lite <= 0)) {

	    /* Is this grid dark now? */
	    if (no_lite()) {

		/* Notice the darkness */
		msg_print("You can no longer see!");

		/* That is disturbing */
		disturb(0, 0);
	    }
	}

	/* Remember the old lite */
	old_lite = cur_lite;


	/*** Check the Food, and Regenerate ***/

	/* Default regeneration */
	regen_amount = PLAYER_REGEN_NORMAL;

	/* Check food status	       */
	if (p_ptr->food < PLAYER_FOOD_ALERT) {
	    if (p_ptr->food < PLAYER_FOOD_WEAK) {

		if (p_ptr->food < 0) {
		    regen_amount = 0;
		}
		else if (p_ptr->food < PLAYER_FOOD_FAINT) {
		    regen_amount = PLAYER_REGEN_FAINT;
		}
		else if (p_ptr->food < PLAYER_FOOD_WEAK) {
		    regen_amount = PLAYER_REGEN_WEAK;
		}

		if ((PY_WEAK & p_ptr->status) == 0) {
		    p_ptr->status |= PY_WEAK;
		    msg_print("You are getting weak from hunger.");
		    disturb(0, 0);
		    prt_hunger();
		}
		if ((p_ptr->food < PLAYER_FOOD_FAINT) && (randint(8) == 1)) {
		    p_ptr->paralysis += randint(5);
		    msg_print("You faint from the lack of food.");
		    disturb(1, 0);
		}
	    }
	    else if ((PY_HUNGRY & p_ptr->status) == 0) {
		p_ptr->status |= PY_HUNGRY;
		msg_print("You are getting hungry.");
		disturb(0, 0);
		prt_hunger();
	    }
	}

	/* Food consumption */
	/* Note: Speeded up characters really burn up the food!  */
	/* now summation, not square, since spd less powerful -CFT */
	/* Hack -- Note that Speed is different now (2.7.3) */
	
	/* Fast players consume slightly more food */
	if (p_ptr->pspeed > 110) {
	    int ospeed = (110 - p_ptr->pspeed) / 10;
	    p_ptr->food -=  (ospeed * ospeed - ospeed) / 2;
	}

	/* Digest some food */
	p_ptr->food -= p_ptr->food_digested;

	/* Starve to death */
	if (p_ptr->food < 0) {
	    take_hit(-p_ptr->food / 16, "starvation");	/* -CJS- */
	    disturb(1, 0);
	}

	/* Regeneration ability */
	if (p_ptr->regenerate) {
	    regen_amount = regen_amount * 3 / 2;
	}

	/* Searching or Resting */
	if (p_ptr->searching || p_ptr->rest) {
	    regen_amount = regen_amount * 2;
	}

	/* Regenerate the mana (even if poisoned or cut (?)) */
	if (p_ptr->cmana < p_ptr->mana) {
	    regenmana(regen_amount);
	}

	/* Poisoned or cut yields no healing */
	if (p_ptr->poisoned > 0) regen_amount = 0;
	if (p_ptr->cut > 0) regen_amount = 0;

	/* Regenerate Hit Points if needed */
	if (p_ptr->chp < p_ptr->mhp) {
	    regenhp(regen_amount);
	}


	/*** Assorted Maladies ***/

	/* Paralysis -- player cannot see monster movement */
	if (p_ptr->paralysis > 0) {
	    p_ptr->paralysis--;
	    disturb(1, 0);
	}

	/* Blindness */
	if (p_ptr->blind > 0) {
	    if (!(PY_BLIND & p_ptr->status)) {
		p_ptr->status |= PY_BLIND;
		update_lite();
		draw_cave();
		disturb(0, 0);
	    }
	    p_ptr->blind--;
	    if (!p_ptr->blind) {
		p_ptr->status &= ~PY_BLIND;
		msg_print("The veil of darkness lifts.");
		draw_cave();
		update_lite();
		disturb(0, 0);
	    }
	}

	/* Hallucinating? */
	if (p_ptr->image > 0) {
	    p_ptr->image--;
	    prt_map();
	    disturb(1, 0);
	}

	/* Confusion */
	if (p_ptr->confused > 0) {
	    if ((PY_CONFUSED & p_ptr->status) == 0) {
		p_ptr->status |= PY_CONFUSED;
		prt_confused();
	    }
	    p_ptr->confused--;
	    if (!p_ptr->confused) {
		p_ptr->status &= ~PY_CONFUSED;
		msg_print("You feel less confused now.");
		prt_confused();

		/* Hack -- Allow "mega-rest" to continue */
		if (p_ptr->rest != -2) disturb(0, 0);
	    }
	}

	/* Stun */
	if (p_ptr->stun > 0) {
	    int adjust = (con_adj() / 2 + 1);
	    if (adjust < 1) adjust = 1;
	    p_ptr->stun -= adjust;
	    p_ptr->status |= PY_STR_WGT;
	}

	/* Hack -- Always redraw the stun */
	prt_stun();

	/* Cut */
	if (p_ptr->cut > 0) {
	    int damage = 1;
	    int adjust = con_adj() + 1;
	    if (adjust < 1) adjust = 1;
	    if (p_ptr->cut > 1000) {
		adjust = 0;
		damage = 3;
	    }
	    else if (p_ptr->cut > 200) {
		damage = 3;
	    }
	    else if (p_ptr->cut > 100) {
		damage = 2;
	    }
	    take_hit(damage, "a fatal wound");
	    disturb(1, 0);
	    p_ptr->cut -= adjust;
	    if (p_ptr->cut < 0) p_ptr->cut = 0;
	    if (!p_ptr->cut) {
		if (p_ptr->chp >= 0) msg_print("Your wound heals.");
	    }
	}

	/* Always redraw "cuts" */
	prt_cut();

	/* Poisoned */
	if (p_ptr->poisoned > 0) {
	    if ((PY_POISONED & p_ptr->status) == 0) {
		p_ptr->status |= PY_POISONED;
		prt_poisoned();
	    }
	    p_ptr->poisoned--;
	    if (p_ptr->immune_pois ||
		p_ptr->resist_pois ||
		p_ptr->oppose_pois) {
		p_ptr->poisoned = 0;
	    }
	    if (p_ptr->poisoned == 0) {
		p_ptr->status &= ~PY_POISONED;
		prt_poisoned();
		msg_print("You feel better.");
		disturb(0, 0);
	    }
	    else {
		switch (con_adj()) {
		  case -4: i = 4; break;
		  case -3:
		  case -2: i = 3; break;
		  case -1: i = 2; break;
		  case 0: i = 1; break;
		  case 1:
		  case 2:
		  case 3: i = ((turn % 2) == 0); break;
		  case 4:
		  case 5: i = ((turn % 3) == 0); break;
		  case 6: i = ((turn % 4) == 0); break;
		  case 7: i = ((turn % 5) == 0); break;
		  default: i = ((turn % 6) == 0); break;
		}
		take_hit(i, "poison");
		disturb(1, 0);
	    }
	}

	/* Afraid */
	if (p_ptr->afraid > 0) {
	    if ((PY_FEAR & p_ptr->status) == 0) {
		p_ptr->status |= PY_FEAR;
		prt_afraid();
	    }
	    p_ptr->afraid--;
	    if (p_ptr->shero || p_ptr->hero) p_ptr->afraid = 0;
	    if (p_ptr->resist_fear) p_ptr->afraid = 0;
	    if (!p_ptr->afraid) {
		p_ptr->status &= ~PY_FEAR;
		prt_afraid();
		msg_print("You feel bolder now.");

		/* Hack -- Allow Mega-Rest to continue */
		if (p_ptr->rest != -2) disturb(0, 0);
	    }
	}


	/*** Check the Speed ***/

	/* Fast */
	if (p_ptr->fast > 0) {
	    if (!(PY_FAST & p_ptr->status)) {
		p_ptr->status |= PY_FAST;
		msg_print("You feel yourself moving faster.");
		p_ptr->status |= PY_STR_WGT;
		disturb(0, 0);
	    }
	    p_ptr->fast--;
	    if (!p_ptr->fast) {
		p_ptr->status &= ~PY_FAST;
		msg_print("You feel yourself slow down.");
		p_ptr->status |= PY_STR_WGT;

		/* Hack -- Allow Mega-Rest to continue */
		if (p_ptr->rest != -2) disturb(0, 0);
	    }
	}

	/* Slow */
	if (p_ptr->slow > 0) {
	    if (!(PY_SLOW & p_ptr->status)) {
		p_ptr->status |= PY_SLOW;
		msg_print("You feel yourself moving slower.");
		p_ptr->status |= PY_STR_WGT;
		disturb(0, 0);
	    }
	    p_ptr->slow--;
	    if (!p_ptr->slow) {
		p_ptr->status &= ~PY_SLOW;
		msg_print("You feel yourself speed up.");
		p_ptr->status |= PY_STR_WGT;

		/* Hack -- allow Mega-Rest to continue */
		if (p_ptr->rest != -2) disturb(0, 0);
	    }
	}


	/*** All good things must come to an end... ***/

	/* Protection from evil counter */
	if (p_ptr->protevil > 0) {
	    p_ptr->protevil--;
	    if (!p_ptr->protevil) {
		msg_print("You no longer feel safe from evil.");
	    }
	}

	/* Invulnerability */
	if (p_ptr->invuln > 0) {
	    if ((PY_INVULN & p_ptr->status) == 0) {
		p_ptr->status |= PY_INVULN;
		msg_print("Your skin turns to steel!");
		disturb(0, 0);
		p_ptr->status |= PY_STR_WGT;
	    }
	    p_ptr->invuln--;
	    if (p_ptr->invuln == 0) {
		p_ptr->status &= ~PY_INVULN;
		msg_print("Your skin returns to normal.");
		disturb(0, 0);
		p_ptr->status |= PY_STR_WGT;
	    }
	}

	/* Heroism */
	if (p_ptr->hero > 0) {
	    if (!(PY_HERO & p_ptr->status)) {
		p_ptr->status |= PY_HERO;
		msg_print("You feel like a HERO!");
		disturb(0, 0);
		calc_hitpoints();
		p_ptr->status |= PY_STR_WGT;
		p_ptr->status |= PY_HP;
	    }
	    p_ptr->hero--;
	    if (!p_ptr->hero) {
		p_ptr->status &= ~PY_HERO;
		msg_print("The heroism wears off.");
		disturb(0, 0);
		calc_hitpoints();
		p_ptr->status |= PY_STR_WGT;
		p_ptr->status |= PY_HP;
	    }
	}

	/* Super Heroism */
	if (p_ptr->shero > 0) {
	    if (!(PY_SHERO & p_ptr->status)) {
		p_ptr->status |= PY_SHERO;
		msg_print("You feel like a killing machine!");
		disturb(0, 0);
		calc_hitpoints();
		p_ptr->status |= PY_STR_WGT;
		p_ptr->status |= PY_HP;
	    }
	    p_ptr->shero--;
	    if (!p_ptr->shero) {
		p_ptr->status &= ~PY_SHERO;
		msg_print("You feel less Berserk.");
		disturb(0, 0);
		calc_hitpoints();
		p_ptr->status |= PY_STR_WGT;
		p_ptr->status |= PY_HP;
	    }
	}

	/* Blessed */
	if (p_ptr->blessed > 0) {
	    if (!(PY_BLESSED & p_ptr->status)) {
		p_ptr->status |= PY_BLESSED;
		msg_print("You feel righteous!");
		disturb(0, 0);
		p_ptr->status |= PY_STR_WGT;
	    }
	    p_ptr->blessed--;
	    if (!p_ptr->blessed) {
		p_ptr->status &= ~PY_BLESSED;
		msg_print("The prayer has expired.");
		disturb(0, 0);
		p_ptr->status |= PY_STR_WGT;
	    }
	}

	/* Shield */
	if (p_ptr->shield > 0) {
	    p_ptr->shield--;
	    if (!p_ptr->shield) {
		msg_print("Your mystic shield crumbles away.");
		disturb(0, 0);
		p_ptr->status |= PY_STR_WGT;
	    }
	}

	/* Detect Invisible */
	if (p_ptr->detect_inv > 0) {
	    if (!(PY_DET_INV & p_ptr->status)) {
		p_ptr->status |= PY_DET_INV;
		p_ptr->status |= PY_STR_WGT;
		update_monsters();
	    }
	    p_ptr->detect_inv--;
	    if (!p_ptr->detect_inv) {
		p_ptr->status &= ~PY_DET_INV;
		p_ptr->status |= PY_STR_WGT;
		update_monsters();
	    }
	}

	/* Timed infra-vision */
	if (p_ptr->tim_infra > 0) {
	    if (!(PY_TIM_INFRA & p_ptr->status)) {
		p_ptr->status |= PY_TIM_INFRA;
		p_ptr->status |= PY_STR_WGT;
		update_monsters();
	    }
	    p_ptr->tim_infra--;
	    if (!p_ptr->tim_infra) {
		p_ptr->status &= ~PY_TIM_INFRA;
		p_ptr->status |= PY_STR_WGT;
		update_monsters();
	    }
	}


	/*** Timed resistance must end eventually ***/

	/* XXX Technically, should check "immune" flag as well */

	if (p_ptr->oppose_fire) {
	    p_ptr->oppose_fire--;
	    if (!p_ptr->oppose_fire && !p_ptr->resist_fire) {
		msg_print("You no longer feel safe from flame.");
	    }
	}

	if (p_ptr->oppose_cold) {
	    p_ptr->oppose_cold--;
	    if (!p_ptr->oppose_cold && !p_ptr->resist_cold) {
		msg_print("You no longer feel safe from cold.");
	    }
	}

	if (p_ptr->oppose_acid) {
	    p_ptr->oppose_acid--;
	    if (!p_ptr->oppose_acid && !p_ptr->resist_acid) {
		msg_print("You no longer feel safe from acid.");
	    }
	}

	if (p_ptr->oppose_elec) {
	    p_ptr->oppose_elec--;
	    if (!p_ptr->oppose_elec && !p_ptr->resist_elec) {
		msg_print("You no longer feel safe from lightning.");
	    }
	}

	if (p_ptr->oppose_pois) {
	    p_ptr->oppose_pois--;
	    if (!p_ptr->oppose_pois && !p_ptr->resist_pois) {
		msg_print("You no longer feel safe from poison.");
	    }
	}


	/*** Involuntary Movement ***/

	/* Word-of-Recall -- Note: Word-of-Recall is a delayed action */
	if (p_ptr->word_recall > 0) {
	    if (p_ptr->word_recall == 1) {
		new_level_flag = TRUE;
		p_ptr->paralysis++;
		p_ptr->word_recall = 0;
		if (dun_level) {
		    dun_level = 0;
		    msg_print("You feel yourself yanked upwards! ");
		}
		else if (p_ptr->max_dlv) {
		    dun_level = p_ptr->max_dlv;
		    msg_print("You feel yourself yanked downwards! ");
		}
		else {
		    msg_print("You feel tension leave the air. ");
		}
	    }
	    else {
		p_ptr->word_recall--;
	    }
	}


	/*** Handle Resting ***/

	/* Check "Resting" status */
	if (p_ptr->rest != 0) {

	    /* +n -> rest for n turns */
	    if (p_ptr->rest > 0) {
		p_ptr->rest--;
		if (p_ptr->rest == 0) {
		    rest_off();
		}
	    }

	    /* -1 -> rest until HP/mana restored */
	    else if (p_ptr->rest == -1) {
		if ((p_ptr->chp == p_ptr->mhp) &&
		    (p_ptr->cmana == p_ptr->mana)) {

		    p_ptr->rest = 0;
		    rest_off();
		}
	    }

	    /* -2 -> like -1, plus stone/blind/conf/fear/stun/halluc/recall/slow */
	    /* Note: stop (via "disturb") as soon as blind or recall is done */
	    else if (p_ptr->rest == -2) {
		if ((p_ptr->blind < 1) && (p_ptr->confused < 1) &&
		    (p_ptr->afraid < 1) && (p_ptr->stun < 1) &&
		    (p_ptr->image < 1) && (p_ptr->word_recall < 1) &&
		    (p_ptr->slow < 1) && (p_ptr->paralysis < 1) &&
		    (p_ptr->chp == p_ptr->mhp) &&
		    (p_ptr->cmana == p_ptr->mana)) {

		    p_ptr->rest = 0;
		    rest_off();
		}
	    }
	}


        /*** Hack -- delay bonus calculations ***/
        
        /* Delayed calculation */
	if (p_ptr->status & PY_STR_WGT) {
	    calc_bonuses();
	}

	
	/*** Display some things ***/

	if (p_ptr->status & PY_STUDY) {
	    prt_study();
	}

	if (p_ptr->status & PY_SPEED) {
	    p_ptr->status &= ~PY_SPEED;
	    prt_speed();
	}

	if ((p_ptr->status & PY_PARALYSED) && (p_ptr->paralysis < 1)) {
	    p_ptr->status &= ~PY_PARALYSED;
	    prt_state();
	}
	else if (p_ptr->paralysis > 0) {
	    p_ptr->status |= PY_PARALYSED;
	    prt_state();
	}
	else if (p_ptr->rest != 0) {
	    prt_state();
	}

	if (p_ptr->status & PY_STATS) {
	    if (p_ptr->status & PY_STR) prt_stat(A_STR);
	    if (p_ptr->status & PY_INT) prt_stat(A_INT);
	    if (p_ptr->status & PY_WIS) prt_stat(A_WIS);
	    if (p_ptr->status & PY_DEX) prt_stat(A_DEX);
	    if (p_ptr->status & PY_CON) prt_stat(A_CON);
	    if (p_ptr->status & PY_CHR) prt_stat(A_CHR);
	    p_ptr->status &= ~PY_STATS;
	}

	if (p_ptr->status & PY_ARMOR) {
	    p_ptr->status &= ~PY_ARMOR;
	    prt_pac();
	}

	if (p_ptr->status & PY_HP) {
	    p_ptr->status &= ~PY_HP;
	    prt_mhp();
	    prt_chp();
	}

	if (p_ptr->status & PY_MANA) {
	    p_ptr->status &= ~PY_MANA;
	    prt_cmana();
	}


	/*** Process Inventory ***/

	/* Process equipment */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {

	    /* Get the object */
	    i_ptr = &inventory[i];

	    /* Skip fake objects */
	    if (i_ptr->tval == TV_NOTHING) continue;

	    /* Let activatable objects recharge */
	    if (i_ptr->timeout > 0) i_ptr->timeout--;

	    /* Hack -- Process "Drain Experience" flag */
	    if (i_ptr->flags3 & TR3_DRAIN_EXP) {
		if ((randint(20) == 1) && (p_ptr->exp > 0)) {
		    p_ptr->exp--;
		    p_ptr->max_exp--;
		    prt_experience();
		}
	    }
	}

	/* Timeout rods (backwards) */
	for (i = inven_ctr-1; i >= 0; i--) {

	    i_ptr = &inventory[i];

	    /* Examine ALL rods */
	    if (i_ptr->tval == TV_ROD) {

		/* Charge it a little */
		if (i_ptr->pval > 0) {

		    /* Charge it */
		    i_ptr->pval--;

		    /* Notice when done (and recombine) */
		    if (i_ptr->pval == 0) {
			/* message("Your rod has recharged.", 0); */
			i = combine(i);
		    }
		}
	    }
	}


	/*** Auto-Detect-Enchantment ***/

	/* Have some "feelings" about the inventory */
	if (!p_ptr->confused && !p_ptr->blind) {

	    /* Feel the inventory */
	    sense_inventory();
	}


	/*** Compact the Monsters ***/

	/* "Tighten" up the monster list */
	tighten_m_list();


	/*** Update the Display ***/

#ifdef USE_MULTIHUED

	/* Visually Cycle the Multi-Hued things */
	mh_cycle();

#endif


	/*** Handle actual user input ***/

	/* Check for interrupts to repetition, or running, or resting. */
	if (command_rep || find_flag || p_ptr->rest) {

	    /* Hack -- move the cursor onto the player */
	    move_cursor_relative(char_row, char_col);

	    /* Flush output, check for keypress */
	    if (Term_kbhit()) {

		/* Flush input */
		flush();

		/* Disturb the resting, running, and repeating */
		disturb(0, 0);

		/* XXX Message */
		msg_print("Cancelled.");
	    }
	}

#if 0
	/* Hack -- Wait for a bit */
	if (command_rep || p_ptr->rest) delay(10);
#endif

	/* Resting -- Voluntary trade of moves for regeneration */
	/* Mega-Stunned -- Unable to do anything but stagger */
	/* Paralyzed -- Not allowed to do anything */
	/* Dead -- Not really able to do anything */

	if ((p_ptr->rest) ||
	    (p_ptr->stun >= 100) ||
	    (p_ptr->paralysis > 0) ||
	    (death) ) {

	    /* Hilite the player */
	    move_cursor_relative(char_row, char_col);

	    /* Flush output */
	    Term_fresh();
	}


	/* Get (and execute) one or more user commands */
	else {

	    /* Hack -- Notice "disturbances" */
	    if (screen_change) {
		
		/* Forget the interuption */
		screen_change = FALSE;

		/* Verify "continuing" command */
		if (command_new) {

		    /* Allow clean "cancel" */		
		    if (!get_check("Continue with inventory command?")) {

			/* Cancel command */
			command_new = 0;

			/* Cancel "list" mode */
			command_see = FALSE;
		    }
		}

		/* Just flush the changes */
		else {
		
		    /* Hilite the player */
		    move_cursor_relative(char_row, char_col);

		    /* Flush the changes */
		    Term_fresh();
		}	
	    }


	    /* Repeat until the turn ends */
	    while (1) {

		/* Commands are assumed to take time */
		free_turn_flag = FALSE;

		/* Update the "repeat" count */
		if (p_ptr->status & PY_REPEAT) prt_state();

		/* Hack -- If running, run some more */
		if (find_flag) {

		    /* Hack -- Hilite the player */
		    move_cursor_relative(char_row, char_col);

		    /* Hack -- flush old output */
		    Term_fresh();
		    
		    /* Take a step */
		    find_step();
		}

		/* Else, get a command from the user */
		else {

		    /* XXX XXX Hack -- update choice window */
		    choice_inven(0, inven_ctr - 1);
		    
		    /* Get a command (new or old) */
		    request_command();

		    /* Process the command */
		    process_command();
		}


		/* Hack -- handle over-flow of the pack */
		if (inventory[INVEN_PACK].tval) {
    
		    int amt;
		    vtype prt1;

		    /* Choose an item to spill */
		    i = INVEN_PACK;
		    
		    /* Access the slot to be dropped */
		    i_ptr = &inventory[i];

		    /* Drop all of that item */
		    amt = i_ptr->number;
		    
		    /* Warning */
		    msg_print("Your pack overflows!");

		    /* Message */
		    objdes(prt1, i_ptr, TRUE);
		    msg_print(format("You drop %s.", prt1));

		    /* Drop it (carefully) near the player */
		    drop_near(i_ptr, 0, char_row, char_col);

		    /* Decrease the item, optimize. */
		    inven_item_increase(i, -amt);
		    inven_item_optimize(i);
		}
    

		/* Exit when a "real" command is executed */
		if (!free_turn_flag) break;
		
		/* Notice death and new levels */
		if (death || new_level_flag) break;
	    }
	}


	/* Hack -- notice death and new levels */
	if (death || new_level_flag) break;


	/* Random teleportation */
	if ((p_ptr->teleport) && (randint(100) == 1)) teleport(40);

	/* Mega-Hack -- process teleport traps */
	if (teleport_flag) teleport(100);


	/* Mega-Hack -- take a full turn from the player */
	p_ptr->energy -= 100;


	/* Process all of the monsters */
	process_monsters();
    }


    /* Forget the old lite */
    forget_lite();

    /* Forget the old view */
    forget_view();
}


