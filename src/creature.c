/* File: creature.c */

/* Purpose: handle monster movement and attacks */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "angband.h"


/* Lets do all prototypes correctly.... -CWS */
#ifndef NO_LINT_ARGS
#ifdef __STDC__
static void get_moves(int, int *);
static int  monster_critical(int, int, int);
static void make_attack(int);
static void make_move(int, int *);
static void mon_cast_spell(int, int *);
static void mon_move(int);
static void shatter_quake(int, int);
static void br_wall(int, int);
#endif
#endif



/*
 * This function updates the monster record of the given monster
 * This involves extracting the distance to the player, checking
 * for visibility (natural, infravision, see-invis, telepathy),
 * updating the monster visibility flag, redrawing or erasing the
 * monster, and taking note of any "visual" features of the monster.
 * The only monster fields that are changed here are "cdis" and "ml".
 *
 * It is slightly inefficient to calculate "distance from player" (cdis)
 * in this function, especially for monsters that never move, but it will
 * prevent the calculation from being done in multiple places.
 *
 * Perhaps we should take a parameter...
 *
 * We need to verify that "update_mon()" is called after every change to
 * a monster's location, and that "update_monsters()" is called after
 * every change to the player location.
 *
 * But then those will be the ONLY times such checks are needed.
 * And even if we miss one, we will "catch up" when the player "moves".
 */
void update_mon(int m_idx)
{
    register cave_type     *c_ptr;
    register monster_type  *m_ptr;
    register monster_race  *r_ptr;
    register monster_lore  *l_ptr;

    /* The current monster location */
    int fy, fx;

    /* Can the monster be sensed in any way? */
    int flag = FALSE;

    /* Does the monster noticably avoid telepathy? */
    int no_mind = FALSE;

    /* Does the monster noticably avoid infravision? */
    int no_infra = FALSE;

    /* Is the monster noticably invisible? */
    int is_invis = FALSE;


    /* Get the monster */
    m_ptr = &m_list[m_idx];
    
    /* Get the monster race (to check flags) */
    r_ptr = &r_list[m_ptr->r_idx];

    /* Get the monster lore (to memorize flags) */
    l_ptr = &l_list[m_ptr->r_idx];


    /* Get the monster location */
    fy = m_ptr->fy;
    fx = m_ptr->fx;

    /* XXX XXX Mega-Hack -- notice "missing" monsters */
    if (cave[fy][fx].m_idx != m_idx) {

	/* Debugging -- should only happen with 2.7.0/2.7.1 savefiles */
	message("You hear the fabric of the world ripping...", 0);
	message(NULL, 0);

	/* Pick a "new" location */
	while ((cave[fy][fx].m_idx > 0) ||
	       (!clean_grid(fy, fx))) {
	    fy = rand_int(cur_height-2)+1;
	    fx = rand_int(cur_width-2)+1;
	}

	/* Forget the "old" location */
	cave[m_ptr->fy][m_ptr->fx].m_idx = 0;

	/* Save the new location */
	m_ptr->fy = fy;
	m_ptr->fx = fx;
	
	/* Use the new location instead */
	cave[fy][fx].m_idx = m_idx;
    }
    
    
    /* Re-Calculate the "distance from player" field */
    m_ptr->cdis = distance(char_row, char_col, fy, fx);

    /* Determine if there is "line of sight" from player to monster */
    m_ptr->los = player_has_los(fy, fx);


    /* The monster is "nearby", and on the current "panel", */
    /* and the player has "vision" (sight or telepathy or wiz-sight) */
    if ((m_ptr->cdis <= MAX_SIGHT) && (panel_contains(fy, fx)) &&
	(!(p_ptr->blind) || p_ptr->telepathy || wizard)) {

	/* Telepathy can see all monsters with "minds" */
	if (p_ptr->telepathy) {

	    char c = r_ptr->r_char;
	    cptr n = r_ptr->name;

	    /* Never sense monsters with "bizarre minds" */
	    /* (elementals & golems & vorticies & xorns) -CFT */
	    if (strchr("EMXgjvz.", c));

	    /* Once in a while, sense these "almost mindless" insects... -CFT */
	    else if (strchr("FKaclt", c)) {
		if (randint(5)==1) flag = TRUE;
	    }

	    /* Once in a while, sense spiders  -CFT*/
	    /* But always sense Driders and Uniques (Shelob and Ungol) */
	    else if (c=='S' && !prefix(n, "Drider") &&
		     !(r_ptr->cflags2 & MF2_UNIQUE)) {
		if (randint(5)==1) flag = TRUE;
	    }

	    /* Once in a while, sense worms -CFT */
	    /* But always sense Wereworms and Giant Purple Worms */
	    else if (c=='w' && !prefix(n, "Were") && !prefix(n, "Gian")) {
		if (randint(5)==1) flag = TRUE;
	    }

	    /* Never sense normal molds.  But always sense Death molds. */
	    else if (c=='m' && strncmp(n, "Death", 5));

	    /* Never sense skeletons.  But always sense Druj and Cantoras */
	    else if (c=='s' && !strstr(n, "ruj") && !prefix(n, "Cantor"));

	    /* Never sense icky things.  But always sense Blue icky things.. */
	    else if (c=='i' && strncmp(n, "Blue", 4));

	    /* Never sense 'shrooms.  But always sense magic 'shrooms */
	    else if (c==',' && !prefix(n, "Magic"));

	    /* Never sense mindless monsters. */
	    else if (r_ptr->cflags2 & MF2_MINDLESS);

	    /* Finally, sense anything not explicitly denied above. */
	    else flag = TRUE;

	    /* Take note of telepathy failure */
	    if (!flag) no_mind = TRUE;
	}

	/* There is line of sight from the player to the monster */
	/* So unless the creature is invisible or dark, it gets seen */
	if (m_ptr->los) {

	    /* Get the cave grid (to check light) */
	    c_ptr = &cave[fy][fx];

	    /* Infravision is able to see "nearby" monsters */
	    if ((p_ptr->see_infra > 0) &&
		(m_ptr->cdis <= (unsigned)(p_ptr->see_infra))) {

		/* Infravision only works on "warm" creatures */
		if (!(r_ptr->cflags2 & MF2_NO_INFRA)) flag = TRUE;

		/* Below, we will need to know that infravision failed */
		else no_infra = TRUE;
	    }

	    /* Check for "illumination" of the monster grid */
	    if ((c_ptr->info & CAVE_PL) || (c_ptr->info & CAVE_TL)) {

		/* Unless the monster is invisible, we see it */
		if (!(r_ptr->cflags1 & MF1_MV_INVIS)) flag = TRUE;

		/* Take note of invisible monsters */
		else is_invis = TRUE;

		/* Invisible monsters can be seen by some players */
		if (is_invis && p_ptr->see_inv) flag = TRUE;
	    }
	}

	/* Wizards have "wizard sight" */
	if (wizard) flag = TRUE;
    }


    /* The monster is now visible */
    if (flag) {

	/* It was previously unseen */
	if (!m_ptr->ml) {

	    /* Appearing monsters can disturb the player a lot */
	    if (disturb_enter) disturb(1, 0);

	    /* Mark Monster as visible */
	    m_ptr->ml = TRUE;
	}

	/* XXX Mindless monsters can be verified as such */
	if (no_mind) l_ptr->r_cflags2 |= MF2_MINDLESS;

	/* Infravision can be verified as failing */
	if (no_infra) l_ptr->r_cflags2 |= MF2_NO_INFRA;

	/* Invisible monsters can be verified as such */
	if (is_invis) l_ptr->r_cflags1 |= MF1_MV_INVIS;
    }

    /* The monster has disappeared */
    else {

	/* It was previously seen */
	if (m_ptr->ml) {

	    /* Mark monster as hidden */
	    m_ptr->ml = FALSE;

	    /* Disappearing monsters can "disturb" player. */
	    if (disturb_leave) disturb(0, 0);
	}
    }

    /* Display (or erase) the monster */
    lite_spot(fy, fx);
}




/*
 * This function simply updates all the monsters (see above).
 * Note that this may include a few "pending monster deaths",
 * but that is a very minor inconvenience (and rare, too).
 */
void update_monsters(void)
{
    register int          i;

    /* Process the monsters (backwards, for historical reasons) */
    for (i = m_max - 1; i >= MIN_M_IDX; i--) update_mon(i);
}




/*
 * Given speed,  returns number of moves this turn.     -RAK-
 *
 * NOTE: Player must always move at least once per iteration,
 *       a slowed player is handled by moving monsters faster
 * m_idx = index in m_list[] now passed in, so (turn+m_idx) can
 *          be used to vary when monsters move. -CFT
 */
int movement_rate(int m_idx)
{
    register int ps, ms, tm, i;

    /* Player Speed: fast->2, normal->1, slow->0, etc. -CFT */
    ps = 1 - p_ptr->speed;

    /* The monster speed is already in that format */
    ms = m_list[m_idx].cspeed;

    /* Player and monster have same speeds -CFT */
    if (ps == ms) return 1;

    /* 0xFF to prevent negative values -CFT */
    i = m_idx + (int)(turn & 0xFF);

    if (ps<1 && ms<1) {		/* both slow, swap "reciprocals" -CFT */
	tm = 2 - ps;
	ps = 2 - ms;
	ms = tm;
    }

    /* then mon must be fast, or above would have happened -CFT */
    if (ps < 1)	return ms * (2 - ps);	

    /* then player fast... move once in a while -CFT */
    if (ms < 1) return !(i % (ps * (2 - ms)));

/*
 * player faster.         
 * This formula is not intuitive, but it effectively uses the turn counter
 * (offset by the monster index, so not every monster moves at same time)
 * to compute factional parts of movement ratios.. so that a monster 2/3 the
 * player's spd will move twice every 3 turns.  An earlier version of this
 * equation performed the same result on average, but it was prone to "clumps"
 * of speed... if the player was spd 4, and the monster spd 2, then for each
 * 4 turn cycle, the monster would move 0,0,1,1.  This equation will result
 * in 0,1,0,1, which is better. -CFT
 */

    if (ps>ms) return (((i*ms) % ps) < ms);

    /* divides evenly, simple case -CFT */
    if (!(tm = (ms % ps))) return (ms / ps);

  /*
   * Like the player-faster formula, this is NOT intuitive.  However, it
   * effectively uses the turn counter & monster index to decide when a
   * monster should get an "extra" move.  It also prevents "clumps". -CFT
   */

    return ((ms / ps) + (((i*tm) % ps) < tm));
}



/*
 * Returns whether a given monster will try to run from the player.
 *
 * Monsters will attempt to avoid very powerful players.  See below for details.
 *
 * A wounded, level 50 player will terrify:
 *   9/10 of all healthy level 11 monsters.  1/10 of all healthy level 19 monsters.
 *   9/10 of all wounded level 16 monsters.  1/10 of all wounded level 24 monsters.
 *
 * A healthy, level 50 player will terrify:
 *   9/10 of all healthy level 16 monsters.  1/10 of all healthy level 24 monsters.
 *   9/10 of all wounded level 21 monsters.  1/10 of all wounded level 29 monsters.
 */
static int mon_will_run(int m_idx)
{
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_list[m_ptr->r_idx];

    int power, morale;

    /* Keep immobile monsters from trying to run away */
    if (r_ptr->cflags1 & MF1_MV_ONLY_ATT) return (FALSE);

    /* Keep monsters from running too far away */
    if (m_ptr->cdis > MAX_SIGHT + 5) return FALSE;

    /* Afraid monsters are afraid */
    if (m_ptr->monfear) return TRUE;

    /* If the monster is "close" assume too late to run away */
    if (m_ptr->cdis <= 5) return (FALSE);

    /* The player's power is based on his "level" and "health" */
    power = p_ptr->lev + (5 * p_ptr->chp / p_ptr->mhp);

    /* The monster's morale is based on his "level" and "health" */
    morale = r_ptr->level + (5 * m_ptr->hp / m_ptr->maxhp);

    /* Hack -- every monster gets a "boldness" bonus */
    /* This bonus used to be based on "(m_ptr->maxhp % 8)". */
    morale += (m_idx % 10);

    /* Strong players terrify weak monsters */
    if (power > morale + 25) return (TRUE);

    /* Assume not scared */
    return (FALSE);
}


/*
 * Choose correct directions for monster movement	-RAK-	 
 *
 * Perhaps monster fear should only work when player can be seen?
 */
static void get_moves(int m_idx, int *mm)
{
    int y, ay, x, ax, move_val;

    /* Extract the "standard" direction to move */
    y = m_list[m_idx].fy - char_row;
    x = m_list[m_idx].fx - char_col;

    /* Apply fear if possible and necessary */
    if (mon_will_run(m_idx)) {
	y = (-y);
	x = (-x);
    }

    /* Apply the move */
    if (y < 0) {
	move_val = 8;
	ay = (-y);
    }
    else {
	move_val = 0;
	ay = y;
    }
    if (x > 0) {
	move_val += 4;
	ax = x;
    }
    else {
	ax = (-x);
    }

    /* this has the advantage of preventing the diamond maneuvre, also faster */
    if (ay > (ax << 1)) {
	move_val += 2;
    }
    else if (ax > (ay << 1)) {
	move_val++;
    }

    switch (move_val) {
      case 0:
	mm[0] = 9;
	if (ay > ax) {
	    mm[1] = 8;
	    mm[2] = 6;
	    mm[3] = 7;
	    mm[4] = 3;
	}
	else {
	    mm[1] = 6;
	    mm[2] = 8;
	    mm[3] = 3;
	    mm[4] = 7;
	}
	break;
      case 1:
      case 9:
	mm[0] = 6;
	if (y < 0) {
	    mm[1] = 3;
	    mm[2] = 9;
	    mm[3] = 2;
	    mm[4] = 8;
	}
	else {
	    mm[1] = 9;
	    mm[2] = 3;
	    mm[3] = 8;
	    mm[4] = 2;
	}
	break;
      case 2:
      case 6:
	mm[0] = 8;
	if (x < 0) {
	    mm[1] = 9;
	    mm[2] = 7;
	    mm[3] = 6;
	    mm[4] = 4;
	}
	else {
	    mm[1] = 7;
	    mm[2] = 9;
	    mm[3] = 4;
	    mm[4] = 6;
	}
	break;
      case 4:
	mm[0] = 7;
	if (ay > ax) {
	    mm[1] = 8;
	    mm[2] = 4;
	    mm[3] = 9;
	    mm[4] = 1;
	}
	else {
	    mm[1] = 4;
	    mm[2] = 8;
	    mm[3] = 1;
	    mm[4] = 9;
	}
	break;
      case 5:
      case 13:
	mm[0] = 4;
	if (y < 0) {
	    mm[1] = 1;
	    mm[2] = 7;
	    mm[3] = 2;
	    mm[4] = 8;
	}
	else {
	    mm[1] = 7;
	    mm[2] = 1;
	    mm[3] = 8;
	    mm[4] = 2;
	}
	break;
      case 8:
	mm[0] = 3;
	if (ay > ax) {
	    mm[1] = 2;
	    mm[2] = 6;
	    mm[3] = 1;
	    mm[4] = 9;
	}
	else {
	    mm[1] = 6;
	    mm[2] = 2;
	    mm[3] = 9;
	    mm[4] = 1;
	}
	break;
      case 10:
      case 14:
	mm[0] = 2;
	if (x < 0) {
	    mm[1] = 3;
	    mm[2] = 1;
	    mm[3] = 6;
	    mm[4] = 4;
	}
	else {
	    mm[1] = 1;
	    mm[2] = 3;
	    mm[3] = 4;
	    mm[4] = 6;
	}
	break;
      case 12:
	mm[0] = 1;
	if (ay > ax) {
	    mm[1] = 2;
	    mm[2] = 4;
	    mm[3] = 3;
	    mm[4] = 7;
	}
	else {
	    mm[1] = 4;
	    mm[2] = 2;
	    mm[3] = 7;
	    mm[4] = 3;
	}
	break;
    }
}

static int monster_critical(int dice, int sides, int dam)
{
    int total = dice * sides;
    int max = 0;

    if (dam == total && dam > 20)
	max = 1;
    if ((dam > (19 * total) / 20) && ((dam < 20) ? randint(20) == 1 : TRUE)) {
	if (dam > 20)
	    while (randint(50) == 1)
		max++;
	if (dam > 45)
	    return 6 + max;
	if (dam > 33)
	    return 5 + max;
	if (dam > 25)
	    return 4 + max;
	if (dam > 18)
	    return 3 + max;
	if (dam > 11)
	    return 2 + max;
	return 1 + max;
    }
    return 0;
}

/*
 * Make an attack on the player (chuckle.)		-RAK-	 
 */
static void make_attack(int m_idx)
{
    int                    attype, adesc, adice, asides;
    int                    i, j, tmp, damage, flag, attackn, notice, visible;
    int                    shatter = FALSE;
    int                    do_cut, do_stun;
    int32                  gold;
    int16u                  *ap, *ap_orig;
    vtype                  ddesc;
    char		   m_name[80];

    register monster_race	*r_ptr;
    monster_type		*m_ptr;
    register inven_type		*i_ptr;

    /* flag to see if blinked away (after steal) -CFT */
    int8u                  blinked = 0;


    /* don't beat a dead body! */
    if (death) return;

    m_ptr = &m_list[m_idx];
    r_ptr = &r_list[m_ptr->r_idx];

    /* Get the monster name (or "it") */
    monster_desc(m_name, m_ptr, 0);

    /* Get the "died from" information (i.e. "a kobold") */
    monster_desc(ddesc, m_ptr, 0x88);

    if (r_ptr->cflags2 & MF2_DESTRUCT) shatter = TRUE;

    attackn = 0;
    ap = r_ptr->damage;
    ap_orig = ap;

    /* if has no attacks (*ap starts off 0), still loop once */
    /* to accumulate notices that it has no attacks - dbd */
    while ((*ap != 0 || ap == ap_orig) && !death && !blinked) {
	attype = a_list[*ap].attack_type;
	adesc = a_list[*ap].attack_desc;
	adice = a_list[*ap].attack_dice;
	asides = a_list[*ap].attack_sides;
	ap++;
	flag = FALSE;

	/* Random (100) + level > 50 chance for stop any attack added */
	if (((p_ptr->protevil > 0) && (r_ptr->cflags2 & MF2_EVIL) &&
	     ((p_ptr->lev + 1) > r_ptr->level)) &&
	    (randint(100) + (p_ptr->lev) > 50)) {

	    if (m_ptr->ml) l_list[m_ptr->r_idx].r_cflags2 |= MF2_EVIL;
	    attype = 99;
	    adesc = 99;
	}

	switch (attype) {
	  case 1:		   /* Normal attack  */
	    if (test_hit(60, (int)r_ptr->level, 0, p_ptr->pac + p_ptr->ptoac,
			 CLA_MISC_HIT))
		flag = TRUE;
	    break;
	  case 2:		   /* Lose Strength */
	    if (test_hit(-3, (int)r_ptr->level, 0, p_ptr->pac + p_ptr->ptoac,
			 CLA_MISC_HIT))
		flag = TRUE;
	    break;
	  case 3:		   /* Confusion attack */
	    if (test_hit(10, (int)r_ptr->level, 0, p_ptr->pac + p_ptr->ptoac,
			 CLA_MISC_HIT))
		flag = TRUE;
	    break;
	  case 4:		   /* Fear attack    */
	    if (test_hit(10, (int)r_ptr->level, 0, p_ptr->pac + p_ptr->ptoac,
			 CLA_MISC_HIT))
		flag = TRUE;
	    break;
	  case 5:		   /* Fire attack    */
	    if (test_hit(10, (int)r_ptr->level, 0, p_ptr->pac + p_ptr->ptoac,
			 CLA_MISC_HIT))
		flag = TRUE;
	    break;
	  case 6:		   /* Acid attack    */
	    if (test_hit(0, (int)r_ptr->level, 0, p_ptr->pac + p_ptr->ptoac,
			 CLA_MISC_HIT))
		flag = TRUE;
	    break;
	  case 7:		   /* Cold attack    */
	    if (test_hit(10, (int)r_ptr->level, 0, p_ptr->pac + p_ptr->ptoac,
			 CLA_MISC_HIT))
		flag = TRUE;
	    break;
	  case 8:		   /* Lightning attack */
	    if (test_hit(10, (int)r_ptr->level, 0, p_ptr->pac + p_ptr->ptoac,
			 CLA_MISC_HIT))
		flag = TRUE;
	    break;
	  case 9:		   /* Corrosion attack */
	    if (test_hit(0, (int)r_ptr->level, 0, p_ptr->pac + p_ptr->ptoac,
			 CLA_MISC_HIT))
		flag = TRUE;
	    break;
	  case 10:		   /* Blindness attack */
	    if (test_hit(2, (int)r_ptr->level, 0, p_ptr->pac + p_ptr->ptoac,
			 CLA_MISC_HIT))
		flag = TRUE;
	    break;
	  case 11:		   /* Paralysis attack */
	    if (test_hit(2, (int)r_ptr->level, 0, p_ptr->pac + p_ptr->ptoac,
			 CLA_MISC_HIT))
		flag = TRUE;
	    break;
	  case 12:		   /* Steal Money    */
	    if ((test_hit(5, (int)r_ptr->level, 0, p_ptr->lev,
			  CLA_MISC_HIT))
		&& (p_ptr->au > 0))
		flag = TRUE;
	    break;
	  case 13:		   /* Steal Object   */
	    if ((test_hit(2, (int)r_ptr->level, 0, p_ptr->lev,
			  CLA_MISC_HIT))
		&& (inven_ctr > 0))
		flag = TRUE;
	    break;
	  case 14:		   /* Poison	       */
	    if (test_hit(5, (int)r_ptr->level, 0, p_ptr->pac + p_ptr->ptoac,
			 CLA_MISC_HIT))
		flag = TRUE;
	    break;
	  case 15:		   /* Lose dexterity */
	    if (test_hit(0, (int)r_ptr->level, 0, p_ptr->pac + p_ptr->ptoac,
			 CLA_MISC_HIT))
		flag = TRUE;
	    break;
	  case 16:		   /* Lose constitution */
	    if (test_hit(0, (int)r_ptr->level, 0, p_ptr->pac + p_ptr->ptoac,
			 CLA_MISC_HIT))
		flag = TRUE;
	    break;
	  case 17:		   /* Lose intelligence */
	    if (test_hit(2, (int)r_ptr->level, 0, p_ptr->pac + p_ptr->ptoac,
			 CLA_MISC_HIT))
		flag = TRUE;
	    break;
	  case 18:		   /* Lose wisdom */
	    if (test_hit(2, (int)r_ptr->level, 0, p_ptr->pac + p_ptr->ptoac,
			 CLA_MISC_HIT))
		flag = TRUE;
	    break;
	  case 19:		   /* Lose experience */
	    if (test_hit(5, (int)r_ptr->level, 0, p_ptr->pac + p_ptr->ptoac,
			 CLA_MISC_HIT))
		flag = TRUE;
	    break;
	  case 20:		   /* Aggravate monsters */
	    flag = TRUE;
	    break;
	  case 21:		   /* Disenchant	  */
	    if (test_hit(20, (int)r_ptr->level, 0, p_ptr->pac + p_ptr->ptoac,
			 CLA_MISC_HIT))
		flag = TRUE;
	    break;
	  case 22:		   /* Eat food	  */
	    if (test_hit(5, (int)r_ptr->level, 0, p_ptr->pac + p_ptr->ptoac,
			 CLA_MISC_HIT))
		flag = TRUE;
	    break;
	  case 23:		   /* Eat light	  */
	    if (test_hit(5, (int)r_ptr->level, 0, p_ptr->pac + p_ptr->ptoac,
			 CLA_MISC_HIT))
		flag = TRUE;
	    break;
	  case 24:		   /* Eat charges	  */
	    if ((test_hit(15, (int)r_ptr->level, 0, p_ptr->pac + p_ptr->ptoac,
			  CLA_MISC_HIT)))
		/* check to make sure an object (XXX drainable?) exists */
		if (inven_ctr > 0) flag = TRUE;
	    break;
	  case 25:		   /* Drain all stats   */
	    if ((test_hit(2, (int)r_ptr->level, 0, p_ptr->pac + p_ptr->ptoac,
			  CLA_MISC_HIT)))
		flag = TRUE;
	    break;

	  /* Repelled */
	  case 99:
	    flag = TRUE;
	    break;

	  /* Monster has no physical attacks - dbd */
	  case 0:
	    flag = TRUE;
	    break;

	  default:
	    break;
	}

	/* Describe the attack */
	if (flag) {

	    disturb(1, 0);

	    /* No cut or stun yet */
	    do_cut = do_stun = 0;

	    switch (adesc) {
	      case 1:
		message(m_name, 0x03);
		message(" hits you.", 0);
		if (randint(2) == 1) {
		    /* Cut, but not stunned */
		    do_cut = 1;
		}
		else {
		    /* Stunned, but not cut */
		    do_stun = 1;
		}
		break;
	      case 2:
		message(m_name, 0x03);
		message(" bites you.", 0);
		do_cut = 1;
		break;
	      case 3:
		message(m_name, 0x03);
		message(" claws you.", 0);
		do_cut = 1;
		break;
	      case 4:
		message(m_name, 0x03);
		message(" stings you.", 0);
		break;
	      case 5:
		message(m_name, 0x03);
		message(" touches you.", 0);
		break;
	      case 6:
		message(m_name, 0x03);
		message(" kicks you.", 0);
		break;
	      case 7:
		message(m_name, 0x03);
		message(" gazes at you.", 0);
		break;
	      case 8:
		message(m_name, 0x03);
		message(" breathes on you.", 0);
		break;
	      case 9:
		message(m_name, 0x03);
		message(" spits on you.", 0);
		break;
	      case 10:
		message(m_name, 0x03);
		message(" makes a horrible wail.", 0);
		break;
	      case 11:
		message(m_name, 0x03);
		message(" embraces you.", 0);
		break;
	      case 12:
		message(m_name, 0x03);
		message(" crawls on you.", 0);
		break;
	      case 13:
		message(m_name, 0x03);
		message(" releases a cloud of spores.", 0);
		break;
	      case 14:
		message(m_name, 0x03);
		message(" begs you for money.", 0);
		break;
	      case 15:
		/* Careful!  No monster name here! */
		msg_print("You've been slimed!");
		break;
	      case 16:
		message(m_name, 0x03);
		message(" crushes you.", 0);
		break;
	      case 17:
		message(m_name, 0x03);
		message(" tramples you.", 0);
		do_stun = 1;
		break;
	      case 18:
		message(m_name, 0x03);
		message(" drools on you.", 0);
		break;
	      case 19:
		message(m_name, 0x03);
		switch (randint(9)) {
		  case 1:
		    message(" insults you!", 0);
		    break;
		  case 2:
		    message(" insults your mother!", 0);
		    break;
		  case 3:
		    message(" gives you the finger!", 0);
		    break;
		  case 4:
		    message(" humiliates you!", 0);
		    break;
		  case 5:
		    message(" wets on your leg!", 0);
		    break;
		  case 6:
		    message(" defiles you!", 0);
		    break;
		  case 7:
		    message(" dances around you!", 0);
		    break;
		  case 8:
		    message(" makes obscene gestures!", 0);
		    break;
		  case 9:
		    message(" moons you!!!", 0);
		    break;
		}
		break;
	      case 20:
		message(m_name, 0x03);
		message(" butts you.", 0);
		do_stun = 1;
		break;
	      case 21:
		message(m_name, 0x03);
		message(" charges you.", 0);
		do_stun = 1;
		break;
	      case 22:
		message(m_name, 0x03);
		message(" engulfs you.", 0);
		break;
	      case 23:
		message(m_name, 0x03);
		switch (randint(5)) {
		  case 1:
		    message(" wants his mushrooms back. ", 0);
		    break;
		  case 2:
		    message(" tells you to get off his land. ", 0);
		    break;
		  case 3:
		    message(" looks for his dogs. ", 0);
		    break;
		  case 4:
		    message(" says 'Did you kill my Fang?' ", 0);
		    break;
		  case 5:
		    message(" asks 'Do you want to buy any mushrooms?' ", 0);
		    break;
		}
		break;
	      case 99:
		message(m_name, 0x03);
		message(" is repelled.", 0);
		break;
	      case 0:
		/* no message for case 0 because no attacks - dbd */
	      default:
		break;
	    }

	    notice = TRUE;
	/*
	 * always fail to notice attack if creature invisible, set notice and
	 * visible here since creature may be visible when attacking and then
	 * teleport afterwards (becoming effectively invisible) 
	 */
	    if (!m_ptr->ml) {
		visible = FALSE;
		notice = FALSE;
	    }
	    else {
		visible = TRUE;
	    }

	    /* Roll out the damage */
	    damage = damroll(adice, asides);

	    switch (attype) {

	      /* No physical attacks */
	      case 0:
		/* notice if monster *has* no physical attacks -CWS */
		if (!randint(10)) notice = TRUE;
		break;

	      /* Normal attack	 */
	      case 1:
		/* round half-way case down */
		damage -= ((((((p_ptr->pac + p_ptr->ptoac) > 150) ? 150 :
		     (p_ptr->pac + p_ptr->ptoac)) * 3) / 4) * damage) / 200;
		take_hit(damage, ddesc);
		if ((damage > 23) && shatter) {
		    /* Earthquake centered at the monster */
		    shatter_quake(m_ptr->fy, m_ptr->fx);
		}
		break;

	      /* Lose Strength */
	      case 2:
		take_hit(damage, ddesc);
		if (p_ptr->sustain_str) {
		    msg_print("You feel weaker for a moment, but it passes.");
		}
		else if (randint(2) == 1) {
		    msg_print("You feel weaker.");
		    (void)dec_stat(A_STR, 15, FALSE);
		}
		else {
		    notice = FALSE;
		}
		break;

	      /* Confusion attack */
	      case 3:
		take_hit(damage, ddesc);
		if ((!p_ptr->resist_conf) && (!p_ptr->resist_chaos)) {
		    if (randint(2) == 1) {
			if (p_ptr->confused < 1) {
			    msg_print("You feel confused.");
			    p_ptr->confused += randint((int)r_ptr->level);
			}
			else {
			    notice = FALSE;
			}
			p_ptr->confused += 3;
		    }
		    else {
			notice = FALSE;
		    }
		}
		break;

	      /* Fear attack */		
	      case 4:
		take_hit(damage, ddesc);
		if (player_saves() || (p_ptr->pclass == 1 && randint(3) == 1)
		    || p_ptr->resist_fear) {
		    msg_print("You stand your ground!");
		}
		else if (p_ptr->afraid < 1) {
		    msg_print("You are suddenly afraid!");
		    p_ptr->afraid += 3 + randint((int)r_ptr->level);
		}
		else {
		    p_ptr->afraid += 3;
		    notice = FALSE;
		}
		break;

	      /* Fire attack */
	      case 5:
		msg_print("You are enveloped in flames!");
		fire_dam(damage, ddesc);
		break;

	      /* Acid attack */
	      case 6:
		msg_print("You are covered in acid!");
		acid_dam(damage, ddesc);
		break;

	      /* Cold attack */
	      case 7:
		msg_print("You are covered with frost!");
		cold_dam(damage, ddesc);
		break;

	      /* Lightning attack */
	      case 8:
		msg_print("Lightning strikes you!");
		elec_dam(damage, ddesc);
		break;

	      /* Corrosion attack */
	      case 9:
		msg_print("A stinging red gas swirls about you.");
		corrode_gas(ddesc);
		take_hit(damage, ddesc);
		break;

	      /* Blindness attack */
	      case 10:
		take_hit(damage, ddesc);
		if (!p_ptr->resist_blind) {
		    if (p_ptr->blind < 1) {
			p_ptr->blind += 10 + randint((int)r_ptr->level);
			msg_print("Your eyes begin to sting.");
		    }
		    else {
			p_ptr->blind += 5;
			notice = FALSE;
		    }
		}
		break;

	      /* Paralysis attack */
	      case 11:
		take_hit(damage, ddesc);
		if (player_saves()) {
		    msg_print("You resist the effects!");
		}
		else if (p_ptr->paralysis < 1) {
		    if (p_ptr->free_act)
			msg_print("You are unaffected.");
		    else {
			p_ptr->paralysis = randint((int)r_ptr->level) + 3;
			msg_print("You are paralysed.");
		    }
		}
		else {
		    notice = FALSE;
		}
		break;

	      /* Steal Money */
	      case 12:
		/* immune to steal at 18/150 */
		if ((p_ptr->paralysis < 1) &&
		    (randint(168) < p_ptr->use_stat[A_DEX])) {
		    msg_print("You quickly protect your money pouch!");
		}
		else {
		    vtype               t1;
		    gold = (p_ptr->au / 10) + randint(25);
		    if (gold < 2) gold = 2;
		    if (gold > 5000) gold = 2000 + randint(1000) + (p_ptr->au / 20);
		    if (gold > p_ptr->au) gold = p_ptr->au;
		    if (gold <= 0) {
			message("Nothing was stolen.", 0);
		    }
		    if (p_ptr->au) {
			msg_print("Your purse feels lighter.");
			sprintf(t1, "%ld coins were stolen!", (long)gold);
			message(t1, 0);
		    }
		    else {
			message("All of your coins were stolen!", 0);
		    }
		    p_ptr->au -= gold;
		    prt_gold();
		}
		if (randint(2) == 1) {
		    msg_print("There is a puff of smoke!");
		    blinked = 1;   /* added -CFT */
		    teleport_away(m_idx, MAX_SIGHT + 5);
		}
		break;

	      /* Steal Object */
	      case 13:
		/* immune to steal at 18/150 dexterity */
		if ((p_ptr->paralysis < 1) &&
		    (randint(168) < p_ptr->use_stat[A_DEX])) {
		    msg_print("You grab hold of your backpack!");
		}
		else {
		    int			amt;
		    vtype               t1, t2;

		    /* Steal a single item from the pack */
		    i = randint(inven_ctr) - 1;

		    /* Get the item */
		    i_ptr = &inventory[i];

		    /* Don't steal artifacts  -CFT */
		    if (artifact_p(i_ptr)) break;

		    /* Steal some of the items */
		    amt = randint(i_ptr->number);

		    /* XXX Hack -- only one item at a time */
		    amt = 1;

		    /* Get a description */
		    objdes(t1, i_ptr, FALSE);

		    /* Message */
		    sprintf(t2, "%sour %s (%c) %s stolen!",
			    ((i_ptr->number > 1) ? 
			     ((amt == i_ptr->number) ? "All of y" :
			     (amt > 1 ? "Some of y" : "One of y")) : "Y"),
			    t1, index_to_label(i),
			    ((amt > 1) ? "were" : "was"));
		    message(t2, 0);

		    /* Steal the items */
		    inven_item_increase(i,-amt);
		    inven_item_optimize(i);
		}

		/* Allow monster to "blink" away */
		if (randint(3) == 1) {
		    msg_print("There is a puff of smoke!");
		    blinked = 1;   /* added -CFT */
		    teleport_away(m_idx, MAX_SIGHT + 5);
		}

		break;

	      /* Poison	 */
	      case 14:
		take_hit(damage, ddesc);
		if (!(p_ptr->immune_pois ||
		      p_ptr->resist_pois ||
		      p_ptr->oppose_pois)) {
		    msg_print("You feel very sick.");
		    p_ptr->poisoned += randint((int)r_ptr->level) + 5;
		}
		else {
		    msg_print("The poison has no effect.");
		}
		break;

	      /* Lose dexterity */
	      case 15:
		take_hit(damage, ddesc);
		if (p_ptr->sustain_dex) {
		    msg_print("You feel clumsy for a moment, but it passes.");
		}
		else {
		    msg_print("You feel more clumsy.");
		    (void)dec_stat(A_DEX, 15, FALSE);
		}
		break;

	      /* Lose constitution */
	      case 16:
		take_hit(damage, ddesc);
		if (p_ptr->sustain_con) {
		    msg_print("Your body resists the effects of the disease.");
		}
		else {
		    msg_print("Your health is damaged!");
		    (void)dec_stat(A_CON, 15, FALSE);
		}
		break;

	      /* Lose intelligence */
	      case 17:
		take_hit(damage, ddesc);
		msg_print("You have trouble thinking clearly.");
		if (p_ptr->sustain_int) {
		    msg_print("But your mind quickly clears.");
		}
		else {
		    (void)dec_stat(A_INT, 15, FALSE);
		}
		break;

	      /* Lose wisdom */
	      case 18:
		take_hit(damage, ddesc);
		if (p_ptr->sustain_wis) {
		    msg_print("Your wisdom is sustained.");
		}
		else {
		    msg_print("Your wisdom is drained.");
		    (void)dec_stat(A_WIS, 15, FALSE);
		}
		break;

	      /* Lose experience  */
	      case 19:
		if (p_ptr->hold_life && randint(5) > 1) {
		    msg_print("You keep hold of your life force!");
		}
		else {
		    if (p_ptr->hold_life) {
			msg_print("You feel your life slipping away!");
			lose_exp(damage + (p_ptr->exp/1000) * MON_DRAIN_LIFE);
		    }
		    else {
			msg_print("You feel your life draining away!");
			lose_exp(damage + (p_ptr->exp/100) * MON_DRAIN_LIFE);
		    }
		}
		break;

	      /* Aggravate monster */
	      case 20:
		(void)aggravate_monster(20);
		break;

	      /* Disenchant */
	      case 21:

		/* Allow complete resist */
		if (!p_ptr->resist_disen) {

		    /* Take some damage */
		    take_hit(damage, ddesc);

		    /* Apply disenchantment */
		    if (!apply_disenchant(0)) notice = FALSE;
		}
		break;

	      /* Eat food */
	      case 22:
		if (find_range(TV_FOOD, TV_NEVER, &i, &j)) {
		    int arg = rand_range(i,j);
		    msg_print("It got at your rations!");
		    inven_item_increase(arg, -1);
		    inven_item_optimize(arg);
		}
		else {
		    notice = FALSE;
		}
		break;

	      /* Eat light */
	      case 23:
		i_ptr = &inventory[INVEN_LITE];
		if ((i_ptr->pval > 0) && (!artifact_p(i_ptr))) {
		    i_ptr->pval -= (250 + randint(250));
		    if (i_ptr->pval < 1) i_ptr->pval = 1;
		    if (p_ptr->blind < 1) {
			msg_print("Your light dims.");
		    }
		    else {
			notice = FALSE;
		    }
		}
		else {
		    notice = FALSE;
		}
		break;

	      /* Eat charges */
	      case 24:
		i = randint(inven_ctr) - 1;
		j = r_ptr->level;
		i_ptr = &inventory[i];
		if (((i_ptr->tval == TV_STAFF) || (i_ptr->tval == TV_WAND)) &&
		    (i_ptr->pval > 0)) {
		    m_ptr->hp += j * i_ptr->pval;
		    i_ptr->pval = 0;
		    if (!known2_p(i_ptr)) {
			i_ptr->ident |= ID_EMPTY;
		    }
		    msg_print("Energy drains from your pack!");
		}
		else {
		    notice = FALSE;
		}
		break;

	      /* Drain all stats. Haha! SM */
	      case 25:
		take_hit(damage, ddesc);
		if (p_ptr->sustain_str) {
		    msg_print("You feel weaker for a moment, but it passes.");
		}
		else {
		    msg_print("You feel weaker.");
		    (void)dec_stat(A_STR, 15, FALSE);
		}
		if (p_ptr->sustain_dex) {
		    msg_print("You feel clumsy for a moment, but it passes.");
		}
		else {
		    msg_print("You feel more clumsy.");
		    (void)dec_stat(A_DEX, 15, FALSE);
		}
		if (p_ptr->sustain_con) {
		    msg_print("Your body resists the effects of the disease.");
		}
		else {
		    msg_print("Your health is damaged!");
		    (void)dec_stat(A_CON, 15, FALSE);
		}
		msg_print("You have trouble thinking clearly.");
		if (p_ptr->sustain_int) {
		    msg_print("But your mind quickly clears.");
		}
		else {
		    (void)dec_stat(A_INT, 15, FALSE);
		}
		if (p_ptr->sustain_wis) {
		    msg_print("Your wisdom is sustained.");
		}
		else {
		    msg_print("Your wisdom is drained.");
		    (void)dec_stat(A_WIS, 15, FALSE);
		}
		if (p_ptr->sustain_chr) {
		    msg_print("You keep your good looks.");
		}
		else {
		    msg_print("Your features are twisted.");
		    (void)dec_stat(A_CHR, 15, FALSE);
		}
		break;

	      case 99:
		notice = FALSE;
		break;

	      default:
		notice = FALSE;
		break;
	    }

	    /* The next few lines deal with "critical" cuts/stunning */
	    /* Note that no attack sets both do_cut AND do_stun */
	    /* Note that "tmp" is unused if (!do_tmp && !do_stun) */

	    /* Critical hit (zero if non-critical) */
	    tmp = monster_critical(adice, asides, damage);

	    /* Critical Cut (note check for "do_cut==0") */
	    switch (do_cut * tmp) {
	      case 0: break;
	      case 1: cut_player(randint(5)); break;
	      case 2: cut_player(randint(5) + 5); break;
	      case 3: cut_player(randint(30) + 20); break;
	      case 4: cut_player(randint(70) + 30); break;
	      case 5: cut_player(randint(250) + 50); break;
	      case 6: cut_player(300); break;
	      default: cut_player(5000); break;
	    }

	    /* Critical Stun (note check for "do_stun==0") */
	    switch (do_stun * tmp) {
	      case 0: break;
	      case 1: stun_player(randint(5)); break;
	      case 2: stun_player(randint(5) + 5); break;
	      case 3: stun_player(randint(20) + 10); break;
	      case 4: stun_player(randint(40) + 30); break;
	      case 5: stun_player(randint(50) + 40); break;
	      case 6: stun_player(randint(60) + 57); break;
	      default: stun_player(100 + randint(10)); break;
	    }

	    /* moved here from mon_move, so that monster only */
	    /* confused if it actually hits */
	    /* if no attacks, monster can't get confused -dbd */
	    if (!attype) {
		if (p_ptr->confusing && p_ptr->protevil <= 0) {
		    msg_print("Your hands stop glowing.");
		    p_ptr->confusing = FALSE;
		    if ((randint(100) < r_ptr->level) ||
			(MF2_CHARM_SLEEP & r_ptr->cflags2)) {
			message(m_name, 0x03);
			message(" is unaffected.", 0);
		    }
		    else {
			message(m_name, 0x03);
			message(" appears confused.", 0);
			m_ptr->confused = TRUE;
		    }

		    if (visible && !death && randint(4) == 1) {
			l_list[m_ptr->r_idx].r_cflags2 |=
			    r_ptr->cflags2 & MF2_CHARM_SLEEP;
		    }
		}
	    }

	/*
	 * increase number of attacks if notice true, or if had previously
	 * noticed the attack (in which case all this does is help player
	 * learn damage), note that in the second case do not increase
	 * attacks if creature repelled (no damage done) 
	 */
	    if ((notice ||
		 (l_list[m_ptr->r_idx].r_attacks[attackn] != 0 &&
		  attype != 99))
		&& l_list[m_ptr->r_idx].r_attacks[attackn] < MAX_UCHAR) {
		l_list[m_ptr->r_idx].r_attacks[attackn]++;
	    }
	    if (visible && death && l_list[m_ptr->r_idx].r_deaths < MAX_SHORT) {
		l_list[m_ptr->r_idx].r_deaths++;
	    }
	}
	else {
	    if ((adesc >= 1 && adesc <= 3) || (adesc == 6)) {
		disturb(1, 0);
		message(m_name, 0x03);
		message(" misses you.", 0);
	    }
	}

	/* Can we learn another attack */
	if (attackn < 4 - 1) {
	    attackn++;
	}
	else {
	    break;
	}
    }
}



/*
 * Given five choices of moves, make the first legal one. -RAK-
 *
 * We are guaranteed that update_mon() has been called recently, and we must
 * also guarantee to call update_mon() if we change the monster location.
 *
 * Note that we can directly update the monster lore if needed.
 */
static void make_move(int m_idx, int *mm)
{
    register cave_type    *c_ptr;
    register inven_type   *i_ptr;
    register monster_type *m_ptr;
    register monster_race *r_ptr;
    register monster_lore *l_ptr;

    int                   i, newy, newx;

    bool stuck_door = FALSE;
    
    bool do_turn = FALSE;
    bool do_move = FALSE;
    bool do_view = FALSE;
    
    char		  m_name[80];


    /* Access the monster */
    m_ptr = &m_list[m_idx];
    r_ptr = &r_list[m_ptr->r_idx];
    l_ptr = &l_list[m_ptr->r_idx];


    /* Take an array of five directions to try */
    /* XXX We should terminate the array with zero */
    /* Or pass the number of directions as an argument */
    for (i = 0; !do_turn && (i < 5); i++) {

	/* Get the position of the i'th move */
	newy = m_ptr->fy;
	newx = m_ptr->fx;
	(void)mmove(mm[i], &newy, &newx);

	/* Access that cave grid */
	c_ptr = &cave[newy][newx];

	/* Access that cave grid's contents */
	i_ptr = &i_list[c_ptr->i_idx];


	/* Ignore requests to move through boundary walls */
	if (c_ptr->fval == BOUNDARY_WALL) break;


	/* XXX Technically, need to check for monster in the way */
	/* combined with that monster being in a wall (or door?) */

	/* Floor is open? */
	if (floor_grid(newy, newx)) {
	    do_move = TRUE;
	}

	/* Creature moves through walls? */
	else if (r_ptr->cflags1 & MF1_THRO_WALL) {
	    do_move = TRUE;
	    if (m_ptr->ml) l_ptr->r_cflags1 |= MF1_THRO_WALL;
	}

	/* Crunch up those Walls Morgoth and Umber Hulks!!!! */
	else if (r_ptr->cflags2 & MF2_BREAK_WALL) {

	    do_move = TRUE;

	    /* XXX Hack -- assume the player can see it */
	    l_ptr->r_cflags2 |= MF2_BREAK_WALL;

	    /* Hack -- break open doors */
	    if ((i_ptr->tval == TV_CLOSED_DOOR) ||
		(i_ptr->tval == TV_SECRET_DOOR)) {

		/* Hack -- break the door */
		invcopy(i_ptr, OBJ_OPEN_DOOR);
		i_ptr->iy = newy;
		i_ptr->ix = newx;
		i_ptr->pval = (-1);

		/* Redraw door */
		lite_spot(newy, newx);

		/* Message */
		msg_print("You hear a door burst open!");
		disturb(1, 0);
	    }

	    /* Smash through walls */
	    else {
		(void)twall(newy, newx, 1, 0);
	    }

	    /* Note changes to viewable region */
	    if (player_has_los(newy, newx)) do_view = TRUE;
	}

	/* Creature can open doors? */
	else if (c_ptr->i_idx) {

	    /* Creature can open doors. */
	    if (r_ptr->cflags1 & MF1_THRO_DR) {

		stuck_door = FALSE;

		if (i_ptr->tval == TV_CLOSED_DOOR) {

		    do_turn = TRUE;

		    /* XXX Hack -- scared monsters can open locked/stuck doors */
		    if ((m_ptr->monfear) && randint(2) == 1) {
			i_ptr->pval = 0;
		    }

		    /* Open doors */
		    if (i_ptr->pval == 0) {
			do_move = TRUE;
		    }

		    /* Locked doors -- take a turn to unlock it */
		    else if (i_ptr->pval > 0) {
			if (randint((m_ptr->hp + 1) * (50 + i_ptr->pval)) <
			    40 * (m_ptr->hp - 10 - i_ptr->pval)) {
			    i_ptr->pval = 0;
			}
		    }

		    /* Stuck doors */
		    else if (i_ptr->pval < 0) {
			if (randint((m_ptr->hp + 1) * (50 - i_ptr->pval)) <
			    40 * (m_ptr->hp - 10 + i_ptr->pval)) {
			    msg_print("You hear a door burst open!");
			    disturb(1, 0);
			    stuck_door = TRUE;
			    do_move = TRUE;
			}
		    }
		}

		/* Hack -- monsters open secret doors */
		else if (i_ptr->tval == TV_SECRET_DOOR) {
		    do_turn = TRUE;
		    do_move = TRUE;
		}


		/* Deal with doors in the way */
		if (do_move) {
		
		    /* XXX Should create a new object XXX */
		    invcopy(i_ptr, OBJ_OPEN_DOOR);

		    /* Place it in the dungeon */
		    i_ptr->iy = newy;
		    i_ptr->ix = newx;

		    /* 50% chance of breaking door */
		    if (stuck_door) i_ptr->pval = 1 - randint(2);

		    /* Redraw door */
		    lite_spot(newy, newx);

		    /* XXX Should have to see the door too */
		    if (m_ptr->ml) l_ptr->r_cflags1 |= MF1_THRO_DR;

		    /* Hack -- should move into open doorway */
		    do_move = FALSE;

		    /* Update visibility only if needed */
		    if (player_has_los(newy, newx)) do_view = TRUE;
		}
	    }

	    /* Creature can not open doors, must bash them   */
	    else {

		if (i_ptr->tval == TV_CLOSED_DOOR) {
		    do_turn = TRUE;
		    if (randint((m_ptr->hp + 1) * (80 + MY_ABS(i_ptr->pval))) <
			40 * (m_ptr->hp - 20 - MY_ABS(i_ptr->pval))) {

			/* XXX Should create a new object XXX */
			invcopy(i_ptr, OBJ_OPEN_DOOR);

			/* Place it in the dungeon */
			i_ptr->iy = newy;
			i_ptr->ix = newx;
			
			/* 50% chance of breaking door */
			i_ptr->pval = 1 - randint(2);

			/* Redraw */
			lite_spot(newy, newx);
			msg_print("You hear a door burst open!");
			disturb(1, 0);

			/* Update visibility when needed */
			if (player_has_los(newy, newx)) do_view = TRUE;
		    }
		}
	    }
	}


	/* Hack -- check for Glyph of Warding */
	if (do_move && (c_ptr->i_idx != 0) &&
	    (i_ptr->tval == TV_VIS_TRAP) && (i_ptr->sval == SV_TRAP_GLYPH)) {

	    /* Break the ward */
	    if (randint(OBJ_BREAK_GLYPH) < r_list[m_ptr->r_idx].level) {
		if ((newy == char_row) && (newx == char_col)) {
		    msg_print("The rune of protection is broken!");
		}
		delete_object(newy, newx);
	    }

	    /* Still warded */
	    else {
		do_move = FALSE;

		/* If the creature moves only to attack, don't let */
		/* it move if the glyph prevents it from attacking */
		if (r_ptr->cflags1 & MF1_MV_ONLY_ATT) do_turn = TRUE;
	    }
	}


	/* Process player or OTHER monster in the way */
	if (do_move && c_ptr->m_idx && c_ptr->m_idx != m_idx) {

	    /* Is the player in the next grid? */
	    if (c_ptr->m_idx == 1) {

		/* Do the attack */
		make_attack(m_idx);

		/* No move to do, but turn is done */
		do_move = FALSE;
		do_turn = TRUE;
	    }

	    /* Then there must be another monster there */
	    else {

		/* Can we "eat" that other monster? */
		if ((r_ptr->cflags1 & MF1_THRO_CREAT) &&
		    (r_list[m_ptr->r_idx].mexp >
		     r_list[m_list[c_ptr->m_idx].r_idx].mexp)) {

		    /* If the OTHER monster is visible... */
		    if (m_list[c_ptr->m_idx].ml) {

			/* XXX Technically, should move first */
			if (m_ptr->ml) l_ptr->r_cflags1 |= MF1_THRO_CREAT;
		    }

		    /* Eat the monster */
		    delete_monster_idx(c_ptr->m_idx);
		}

		else {
		    do_move = FALSE;
		}
	    }
	}


	/* XXX Hack -- make sure molds never "run away" */
	if (r_ptr->cflags1 & MF1_MV_ONLY_ATT) do_move = FALSE;


	/* Creature has been allowed move. */
	if (do_move) {

	    /* Move the creature */
	    move_rec(m_ptr->fy, m_ptr->fx, newy, newx);

	    /* Update the monster */
	    update_mon(m_idx);

	    /* XXX Moving monsters disturb the player */
	    if (m_ptr->ml &&
	        (disturb_move || (disturb_near && (m_ptr->cdis < 5)))) {
	        disturb(0, 0);
            }
            

	    /* XXX XXX Update l_ptr (eating others, crushing walls, etc) */

	    /* A turn was taken */
	    do_turn = TRUE;

	    /* Pick up or eat an object	*/
	    if (r_ptr->cflags1 & MF1_PICK_UP) {

		/* used in code to prevent orcs from picking up */
		/* Slay Orc weapons, artifacts, etc -CFT */
		int32u flg = 0L;

		c_ptr = &cave[newy][newx];
		i_ptr = &i_list[c_ptr->i_idx];

		/* Is there a (non-gold) object the monster can pick up? */
		if ((c_ptr->i_idx != 0) && (i_ptr->tval <= TV_MAX_OBJECT)) {

		    /* The monster TRIES to pick things up */
		    if (m_ptr->ml) l_ptr->r_cflags1 |= MF1_PICK_UP;

		    /* Analyze "monster hurting" flags on the object */
		    if (wearable_p(&i_list[c_ptr->i_idx])) {

			/* React to objects that hurt the monster */
			if (i_ptr->flags1 & TR1_SLAY_DRAGON) flg |= MF2_DRAGON;
			if (i_ptr->flags1 & TR1_KILL_DRAGON) flg |= MF2_DRAGON;                            
			if (i_ptr->flags1 & TR1_SLAY_UNDEAD) flg |= MF2_UNDEAD;
			if (i_ptr->flags1 & TR1_SLAY_DEMON) flg |= MF2_DEMON;
			if (i_ptr->flags1 & TR1_SLAY_TROLL) flg |= MF2_TROLL;
			if (i_ptr->flags1 & TR1_SLAY_GIANT) flg |= MF2_GIANT;
			if (i_ptr->flags1 & TR1_SLAY_ORC) flg |= MF2_ORC;
		    }

		    /* The object cannot be picked up by the monster */
		    if (artifact_p(i_ptr) || (r_ptr->cflags2 & flg)) {

			vtype               i_name;

			/* Can the player "see" the grid? */
			if (m_ptr->los) {

			    /* Acquire the object name */
			    objdes(i_name, i_ptr, TRUE);

			    /* Acquire "The monster" (or "Something") */
			    monster_desc(m_name, m_ptr, 0x04);

			    /* Compose, split, and dump a message */
			    message(m_name, 0x03);
			    message(" tries to pick up ", 0x02);
			    message(i_name, 0x02);
			    message(", but stops suddenly!", 0x04);
			}
		    }

		    /* Let the creature "eat" it */
		    else {
			delete_object(newy, newx);
		    }
		}
	    }
	}
    }


    /* XXX Hack -- If no move or turn taken, cancel fear */
    if (!do_turn && !do_move && m_ptr->monfear) {

	/* No longer afraid */
	m_ptr->monfear = 0;

	/* Message if seen */
	if (m_ptr->ml) {
	    monster_desc(m_name, m_ptr, 0);
	    message(m_name, 0x03);
	    message(" turns to fight!", 0);
	}
    }


    /* Hack -- update view if necessary */
    if (do_view) {
	update_view();
	update_lite();
	update_monsters();
    }
}



/*
 * Creatures can cast spells (and breathe) too.   -RAK-
 *
 * cast_spell = true if creature changes position
 * took_turn  = true if creature casts a spell		 
 *
 * Assumes update_mon() has been called, and will call it again if we move.
 *
 * Blindness check is really not correct.  What if the monster is out of sight?
 */
static void mon_cast_spell(int m_idx, int *took_turn)
{
    int32u		i;
    int			chance, thrown_spell, r1;
    register int	k;
    int			spell_choice[64];
    bool		desperate = FALSE;
    bool		recovered = FALSE;
    vtype		ddesc;

    register monster_type  *m_ptr;
    register monster_race *r_ptr;

    char		m_name[80];
    char		m_poss[80];
    char		m_self[80];

    /* Extract the blind-ness -CFT */
    int blind = (p_ptr->blind > 0);

    /* Extract the "see-able-ness" */
    int seen = FALSE;

    /* Count monsters that get summoned */
    int count = 0;

    /* Default: center location for summoning */
    int x = char_col;
    int y = char_row;

    /* Already dead */
    if (death) return;

    m_ptr = &m_list[m_idx];
    r_ptr = &r_list[m_ptr->r_idx];

    /* Get the monster name (or "it") */
    monster_desc(m_name, m_ptr, 0x00);

    /* Get the monster's possessive and reflexive (sexed if visible) */
    monster_desc(m_poss, m_ptr, 0x22);
    monster_desc(m_self, m_ptr, 0x23);

    /* Get the "visibility" */
    seen = !blind && m_ptr->ml;

    /* Hack -- extract the "1 in x" chance of casting spell */
    chance = (int)(r_ptr->spells1 & CS1_FREQ);

    /* Paranoia -- Just to be sure */
    if (chance == 0) {
	msg_print("CHANCE == 0");
	msg_print("caused by ....");
	msg_print(r_ptr->name);
	*took_turn = FALSE;
    }

    else if (randint(chance) != 1) {
	*took_turn = FALSE;
    }

    /* Must be within certain range */
    else if (m_ptr->cdis > MAX_SPELL_DIS) {
	*took_turn = FALSE;
    }

    /* Must have unobstructed Line-Of-Sight, from Monster to Player -CWS */
    else if (!los((int)m_ptr->fy, (int)m_ptr->fx, char_row, char_col)) {
	*took_turn = FALSE;
    }

    /* Creature is going to cast a spell	 */
    else {

	*took_turn = TRUE;

	/* Get the "died from" name */
	monster_desc(ddesc, m_ptr, 0x88);

	/* Extract all possible spells into spell_choice */
	if ((r_ptr->cflags2 & MF2_INTELLIGENT) &&
	    (m_ptr->hp < ((r_ptr->hd[0] * r_ptr->hd[1]) / 10)) &&
	    (r_ptr->spells1 & CS1_INT || r_ptr->spells2 & CS2_INT ||
	     r_ptr->spells3 & CS3_INT) && randint(2) == 1) {
	    desperate = TRUE;
	    l_list[m_ptr->r_idx].r_cflags2 |= MF2_INTELLIGENT;
	}

	k = 0;

	i = (r_ptr->spells1 & ~CS1_FREQ);
	if (desperate) i &= CS1_INT;
	while (i) spell_choice[k++] = bit_pos(&i);

	i = r_ptr->spells2;
	if (desperate) i &= CS2_INT;
	while (i) spell_choice[k++] = bit_pos(&i) + 32;

	i = r_ptr->spells3;
	if (desperate) i &= CS3_INT;
	while (i) spell_choice[k++] = bit_pos(&i) + 64;

	/* Paranoia */
	if (!k) {
	    message(ddesc, 0x03);
	    message(" had no spell to cast, tell someone NOW!", 0);
	    return;
	}

	/* Choose a spell to cast			       */
	thrown_spell = spell_choice[randint(k) - 1];
	thrown_spell++;

	/* all except teleport_away() and drain mana spells always disturb */
	if (thrown_spell > 6 && thrown_spell != 17) disturb(1, 0);

	/* Cast the spell.			     */
	switch (thrown_spell) {

	  case 5:		   /* Teleport Short */
	    if (seen) {
		message(m_name, 0x03);
		message(" blinks away.", 0);
	    }
	    teleport_away(m_idx, 5);
	    break;

	  case 6:		   /* Teleport Long */
	    if (seen) {
		message(m_name, 0x03);
		message(" teleports away.", 0);
	    }
	    teleport_away(m_idx, MAX_SIGHT + 5);
	    break;

	  case 7:		   /* Teleport To	 */
	    message(m_name, 0x03);
	    message(" commands you to return!", 0);
	    teleport_to((int)m_ptr->fy, (int)m_ptr->fx);
	    break;

	  case 8:		   /* Light Wound	 */
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" points at you and curses.", 0);
	    if (player_saves()) msg_print("You resist the effects of the spell.");
	    else take_hit(damroll(3, 8), ddesc);
	    break;

	  case 9:		   /* Serious Wound */
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" points at you and curses horribly.", 0);
	    if (player_saves()) msg_print("You resist the effects of the spell.");
	    else take_hit(damroll(8, 8), ddesc);
	    break;

	  case 10:		   /* Hold Person	  */
	    message(m_name, 0x03);
	    if (!seen) message(" mumbles, and you feel something holding you!", 0);
	    else message(" gazes deep into your eyes!", 0);
	    if (p_ptr->free_act) {
		msg_print("You are unaffected.");
	    }
	    else if (player_saves()) {
		if (!seen) message(" You resist!", 0);
		else message(" You stare back unafraid!", 0);
	    }
	    else if (p_ptr->paralysis > 0) {
		p_ptr->paralysis += 2;
	    }
	    else {
		p_ptr->paralysis = randint(5) + 4;
	    }
	    break;

	  case 11:		   /* Cause Blindness */
	    message(m_name, 0x03);
	    if (blind) message(" mumbles, and your eyes burn even more.", 0);
	    else message(" casts a spell, burning your eyes!", 0);
	    if ((player_saves()) || (p_ptr->resist_blind)) {
		if (blind) msg_print("But the extra burning quickly fades away.");
		else msg_print("You blink and your vision clears.");
	    }
	    else if (p_ptr->blind > 0) {
		p_ptr->blind += 6;
	    }
	    else {
		p_ptr->blind += 12 + randint(3);
	    }
	    break;

	  case 12:		   /* Cause Confuse */
	    message(m_name, 0x03);
	    if (blind) message(" mumbles, and you hear puzzling noises.", 0);
	    else message(" creates a mesmerising illusion.", 0);
	    if ((player_saves()) ||
		(p_ptr->resist_conf) ||
		(p_ptr->resist_chaos)) {
		msg_print("You disbelieve the feeble spell.");
	    }
	    else if (p_ptr->confused > 0) {
		p_ptr->confused += 2;
	    }
	    else {
		p_ptr->confused = randint(5) + 3;
	    }
	    break;

	  case 13:		   /* Cause Fear	  */
	    message(m_name, 0x03);
	    if (blind) message(" mumbles, and you hear scary noises.", 0);
	    else message(" casts a fearful illusion.", 0);
	    if (player_saves() || p_ptr->resist_fear) {
		msg_print("You refuse to be frightened.");
	    }
	    else if (p_ptr->afraid > 0) {
		p_ptr->afraid += 2;
	    }
	    else {
		p_ptr->afraid = randint(5) + 3;
	    }
	    break;

	  case 14:		   /* Summon Monster */
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" magically summons help!", 0);
	    count += summon_monster(&y, &x, FALSE);
	    if (blind && count) message("You hear something appear nearby.", 0);
	    break;

	  case 15:		   /* Summon Undead */
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" magically summons help from beyond the grave!", 0);
	    count += summon_undead(&y, &x);
	    if (blind && count) message("You hear something creepy appear nearby.", 0);
	    break;

	  case 16:		   /* Slow Person	 */
	    message(m_name, 0x03);
	    message(" drains power from your muscles!", 0);
	    if (p_ptr->free_act) {
		msg_print("You are unaffected.");
	    }
	    else if (player_saves()) {
		msg_print("Your body resists the spell.");
	    }
	    else if (p_ptr->slow > 0) {
		p_ptr->slow += 2;
	    }
	    else {
		p_ptr->slow = randint(5) + 3;
	    }
	    break;

	  case 17:		   /* Drain Mana	 */
	    if (p_ptr->cmana > 0) {
		disturb(1, 0);
		message(m_name, 0x03);
		message(" draws psychic energy from you!", 0);
		if (m_ptr->ml) {
		    message(m_name, 0x03);
		    message(" appears healthier.", 0);
		}
		r1 = (randint((int)r_ptr->level) >> 1) + 1;
		if (r1 > p_ptr->cmana) {
		    r1 = p_ptr->cmana;
		    p_ptr->cmana = 0;
		    p_ptr->cmana_frac = 0;
		}
		else {
		    p_ptr->cmana -= r1;
		}
		prt_cmana();
		m_ptr->hp += 6 * (r1);
	    }
	    break;

	  case 18:		   /* Summon Demon */
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" summons a hellish adversary!", 0);
	    count += summon_demon(r_list[m_ptr->r_idx].level, &y, &x);
	    if (blind && count) message("You smell fire and brimstone nearby.", 0);
	    break;

	  case 19:		   /* Summon Dragon */
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" magically summons a Dragon!", 0);
	    count += summon_dragon(&y, &x);
	    if (blind && count) message("You hear something large appear nearby.", 0);
	    break;

	  case 20:		   /* Breath Lightning */
	    message(m_name, 0x03);
	    if (blind) message(" breathes.", 0);
	    else message(" breathes lightning.", 0);
	    breath(m_idx, GF_ELEC,
		   ((m_ptr->hp / 3) > 1600 ? 1600 : (m_ptr->hp / 3)));
	    break;

	  case 21:		   /* Breath Gas	 */
	    message(m_name, 0x03);
	    if (blind) message(" breathes.", 0);
	    else message(" breathes gas.", 0);
	    breath(m_idx, GF_POIS,
		((m_ptr->hp / 3) > 800 ? 800 : (m_ptr->hp / 3)));
	    break;

	  case 22:		   /* Breath Acid	 */
	    message(m_name, 0x03);
	    if (blind) message(" breathes.", 0);
	    else message(" breathes acid.", 0);
	    breath(m_idx, GF_ACID, 
		   ((m_ptr->hp / 3) > 1600 ? 1600 : (m_ptr->hp / 3)));
	    break;

	  case 23:		   /* Breath Frost */
	    message(m_name, 0x03);
	    if (blind) message(" breathes.", 0);
	    else message(" breathes frost.", 0);
	    breath(m_idx, GF_COLD,
		   ((m_ptr->hp / 3) > 1600 ? 1600 : (m_ptr->hp / 3)));
	    break;

	  case 24:		   /* Breath Fire	 */
	    message(m_name, 0x03);
	    if (blind) message(" breathes.", 0);
	    else message(" breathes fire.", 0);
	    breath(m_idx, GF_FIRE,
		   ((m_ptr->hp / 3) > 1600 ? 1600 : (m_ptr->hp / 3)));
	    break;

	  case 25:		   /* Fire Bolt */
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" casts a Fire bolt.", 0);
	    bolt(m_idx, GF_FIRE,
		 damroll(9, 8) + (r_list[m_ptr->r_idx].level / 3));
	    break;

	  case 26:		   /* Frost Bolt */
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" casts a Frost bolt.", 0);
	    bolt(m_idx, GF_COLD,
		 damroll(6, 8) + (r_list[m_ptr->r_idx].level / 3));
	    break;

	  case 27:		   /* Acid Bolt */
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" casts a Acid bolt.", 0);
	    bolt(m_idx, GF_ACID,
		 damroll(7, 8) + (r_list[m_ptr->r_idx].level / 3));
	    break;

	  case 28:		   /* Magic Missiles */
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" casts a magic missile.", 0);
	    bolt(m_idx, GF_MISSILE,
		 damroll(2, 6) + (r_list[m_ptr->r_idx].level / 3));
	    break;

	  case 29:		   /* Critical Wound	 */
	    message(m_name, 0x03);
	    if (blind) message(" mumbles loudly.", 0);
	    else message(" points at you, incanting terribly!", 0);
	    if (player_saves()) {
		msg_print("You resist the effects of the spell.");
	    }
	    else {
		take_hit(damroll(10, 15), ddesc);
	    }
	    break;

	  case 30:		   /* Fire Ball */
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" casts a Fire ball.", 0);
	    breath(m_idx, GF_FIRE,
		   randint((r_list[m_ptr->r_idx].level * 7) / 2) + 10);
	    break;

	  case 31:		   /* Frost Ball */
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" casts a Frost ball.", 0);
	    breath(m_idx, GF_COLD,
		   randint((r_list[m_ptr->r_idx].level * 3) / 2) + 10);
	    break;

	  case 32:		   /* Mana Bolt */
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" casts a Mana bolt.", 0);
	    bolt(m_idx, GF_MISSILE,
		 randint((r_list[m_ptr->r_idx].level * 7) / 2) + 50);
	    break;

	  case 33:
	    message(m_name, 0x03);
	    if (blind) message(" breathes.", 0);
	    else message(" breathes chaos.", 0);
	    breath(m_idx, GF_CHAOS,
		   ((m_ptr->hp / 6) > 600 ? 600 : (m_ptr->hp / 6)));
	    break;

	  case 34:
	    message(m_name, 0x03);
	    if (blind) message(" breathes.", 0);
	    else message(" breathes shards.", 0);
	    breath(m_idx, GF_SHARDS,
		((m_ptr->hp / 6) > 400 ? 400 : (m_ptr->hp / 6)));
	    break;

	  case 35:
	    message(m_name, 0x03);
	    if (blind) message(" breathes.", 0);
	    else message(" breathes sound.", 0);
	    breath(m_idx, GF_SOUND,
		((m_ptr->hp / 6) > 400 ? 400 : (m_ptr->hp / 6)));
	    break;

	  case 36:
	    message(m_name, 0x03);
	    if (blind) message(" breathes.", 0);
	    else message(" breathes confusion.", 0);
	    breath(m_idx, GF_CONFUSION,
		((m_ptr->hp / 6) > 400 ? 400 : (m_ptr->hp / 6)));
	    break;

	  case 37:
	    message(m_name, 0x03);
	    if (blind) message(" breathes.", 0);
	    else message(" breathes disenchantment.", 0);
	    breath(m_idx, GF_DISENCHANT,
		((m_ptr->hp / 6) > 500 ? 500 : (m_ptr->hp / 6)));
	    break;

	  case 38:
	    message(m_name, 0x03);
	    if (blind) message(" breathes.", 0);
	    else message(" breathes nether.", 0);
	    breath(m_idx, GF_NETHER,
		   ((m_ptr->hp / 6) > 550 ? 550 : (m_ptr->hp / 6)) );
	    break;

	  case 39:
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" casts a Lightning bolt.", 0);
	    bolt(m_idx, GF_ELEC, 
		 damroll(4, 8) + (r_list[m_ptr->r_idx].level / 3));
	    break;

	  case 40:
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" casts a Lightning Ball.", 0);
	    breath(m_idx, GF_ELEC,
		randint((r_list[m_ptr->r_idx].level * 3) / 2) + 8);
	    break;

	  case 41:
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" casts an Acid Ball.", 0);
	    breath(m_idx, GF_ACID,
		   randint(r_list[m_ptr->r_idx].level * 3) + 15 );
	    break;

	  case 42:
	    message(m_name, 0x03);
	    if (blind) message(" mumbles, and then cackles evilly.", 0);
	    else message(" casts a spell and cackles evilly.", 0);
	    (void)trap_creation();
	    break;

	  case 43:
	    message(m_name, 0x03);
	    if (blind) message(" mumbles, and then screams 'DIE!'.", 0);
	    else message(" points at you, screaming the word DIE!", 0);
	    if (player_saves()) {
		msg_print("You laugh at the feeble spell.");
	    }
	    else {
		msg_print("You start to bleed!");
		take_hit(damroll(15, 15), ddesc);
		cut_player(m_ptr->hp);
	    }
	    break;

	  case 44:
	    if (!seen) {
		message("You feel something focusing on your mind.", 0);
	    }
	    else {
		message(m_name, 0x03);
		message(" stares at you.", 0);
	    }

	    if (player_saves()) {
		msg_print("You resist the effects.");
	    }
	    else {
		msg_print("Your mind is blasted by psionic energy.");
		if ((!p_ptr->resist_conf) && (!p_ptr->resist_chaos)) {
		    if (p_ptr->confused > 0) {
			p_ptr->confused += 2;
		    }
		    else {
			p_ptr->confused = randint(5) + 3;
		    }
		}
		take_hit(damroll(8, 8), ddesc);
	    }
	    break;

	  case 45:
	    message(m_name, 0x03);
	    message(" teleports you away.", 0);
	    (void)teleport(100);
	    break;

	  case 46:

	    if (!blind) {
		message(m_name, 0x03);
		message(" mumbles to ", 0x02);
		message(m_self, 0x02);
		message(".", 0);
	    }
	    else {
		message(m_name, 0x03);
		message(" concentrates on ", 0x02);
		message(m_poss, 0x02);
		message(" wounds.", 0);
	    }

	    /* Hack -- Already fully healed */
	    if (m_ptr->hp >= m_ptr->maxhp) {

		message(m_name, 0x03);
		message(seen ? " sounds" : " looks", 0x02);
		message(" as healthy as can be.", 0);

		/* can't be afraid at max hp's */
		if (m_ptr->monfear > 0) {
		    m_ptr->monfear = 0;
		    recovered = TRUE;
		}
	    }
	    else {

		m_ptr->hp += (r_list[m_ptr->r_idx].level) * 6;
		if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

		message(m_name, 0x03);
		message(seen ? " looks" : " sounds", 0x02);

		if (m_ptr->hp == m_ptr->maxhp) {
		    message(" REALLY healthy!", 0);

		    /* can't be afraid at max hp's */
		    if (m_ptr->monfear > 0) {
			m_ptr->monfear = 0;
			recovered = TRUE;
		    }
		}
		else {
		    message(" healthier.", 0);

		    /* has recovered 33% of it's hit points */
		    if ((m_ptr->monfear > 0) &&
		       (m_ptr->maxhp / (m_ptr->hp + 1) < 3)) {
			m_ptr->monfear = 0;
			recovered = TRUE;
		    }
		}

		/* no longer afraid -CWS */
		if (recovered) {
		    message(m_name, 0x03);
		    message(" recovers ", 0x02);
		    message(m_poss, 0x02);
		    message(" courage.", 0);
		}
	    }
	    break;

	  case 47:
	    message(m_name, 0x03);
	    if (blind) {
		message(" mumbles to ", 0x02);
		message(m_self, 0x02);
		message(".", 0);
	    }
	    else {
		message(" casts a spell.", 0);
	    }

	    if ((m_ptr->cspeed) <= ((int)(r_list[m_ptr->r_idx].speed) - 10)) {
		if ((r_list[m_ptr->r_idx].speed) <= 15) {
		    message(m_name, 0x03);
		    message(" starts moving faster.", 0);
		    m_ptr->cspeed += 1;
		}
	    }
	    break;

	  case 48:
	    message(m_name, 0x03);
	    if (blind) message(" sounds like it threw something.", 0);
	    else message(" fires missiles at you.", 0);
	    bolt(m_idx, GF_ARROW, damroll(6, 7));
	    break;

	  case 49:
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" casts a Plasma Bolt.", 0);
	    bolt(m_idx, GF_PLASMA,
		 10 + damroll(8, 7) + (r_list[m_ptr->r_idx].level));
	    break;

	  case 50:
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" magically summons monsters!", 0);
	    for (k = 0; k < 8; k++) count += summon_monster(&y, &x, FALSE);
	    if (blind && count) message("You here many things appear nearby.", 0);
	    break;

	  case 51:
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" casts a Nether Bolt.", 0);
	    bolt(m_idx, GF_NETHER,
		 30 + damroll(5, 5) + (r_list[m_ptr->r_idx].level * 3) / 2);
	    break;

	  case 52:
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" casts an Ice Bolt.", 0);
	    bolt(m_idx, GF_COLD,
		 damroll(6, 6) + (r_list[m_ptr->r_idx].level));
	    break;

	  case 53:
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" gestures in shadow.", 0);
	    (void)unlite_area(char_row, char_col);
	    break;

	  case 54:
	    message(m_name, 0x03);
	    message(" tries to blank your mind.", 0);

	    if (player_saves() || randint(2) == 1) {
		msg_print("You resist the spell.");
	    }
	    else if (lose_all_info()) {
		msg_print("Your memories fade away.");
	    }
	    break;

	  case 55:
	    if (!seen) {
		msg_print("You feel something focusing on your mind.");
	    }
	    else {
		message(m_name, 0x03);
		message(" concentrates and ", 0x02);
		message(m_poss, 0x02);
		message(" eyes glow red.", 0);
	    }
	    if (player_saves()) {
		if (!seen) msg_print("You resist the effects.");
		else msg_print("You avert your gaze!");
	    }
	    else {
		msg_print("Your mind is blasted by psionic energy.");
		take_hit(damroll(12, 15), ddesc);
		if ((!p_ptr->resist_conf) && (!p_ptr->resist_chaos)) {
		    if (p_ptr->confused > 0) {
			p_ptr->confused += 2;
		    }
		    else {
			p_ptr->confused = randint(5) + 3;
		    }
		}
		if (!p_ptr->free_act) {
		    if (p_ptr->paralysis > 0) {
			p_ptr->paralysis += 2;
		    }
		    else {
			p_ptr->paralysis = randint(5) + 4;
		    }
		    if (p_ptr->slow > 0) {
			p_ptr->slow += 2;
		    }
		    else {
			p_ptr->slow = randint(5) + 3;
		    }
		}
		if (!p_ptr->resist_blind) {
		    if (p_ptr->blind > 0) {
			p_ptr->blind += 6;
		    }
		    else {
			p_ptr->blind += 12 + randint(3);
		    }
		}
	    }
	    break;

	  case 56:
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" casts a Stinking Cloud.", 0);
	    breath(m_idx, GF_POIS,
		   damroll(12, 2));
	    break;

	  case 57:
	    message(m_name, 0x03);
	    if (blind) message(" mumbles strangely.", 0);
	    else message(" gestures at you.", 0);
	    if ((player_saves()) || (randint(3) != 1) ||
		(p_ptr->resist_nexus)) {
		msg_print("You keep your feet firmly on the ground.");
	    }
	    else {
		if (!dun_level) {
		    msg_print("You sink through the floor.");
		    dun_level++;
		}
		else if (is_quest(dun_level)) {
		    msg_print("You rise up through the ceiling.");
		    dun_level--;
		}
		else if (randint(2) == 1) {
		    msg_print("You rise up through the ceiling.");
		    dun_level--;
		}
		else {
		    msg_print("You sink through the floor.");
		    dun_level++;
		}
		new_level_flag = TRUE;
	    }
	    break;

	  case 58:
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" casts a Water Bolt.", 0);
	    bolt(m_idx, GF_WATER,
		 damroll(10, 10) + (r_list[m_ptr->r_idx].level));
	    break;

	  case 59:
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" gestures fluidly.", 0);
	    msg_print("You are engulfed in a whirlpool.");
	    breath(m_idx, GF_WATER,
		   randint((r_list[m_ptr->r_idx].level * 5) / 2) + 50);
	    break;

	  case 60:
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" casts a Nether Ball.", 0);
	    breath(m_idx, GF_NETHER,
		   (50 + damroll(10, 10) + (r_list[m_ptr->r_idx].level)));
	    break;

	  case 61:
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" summons an Angel.", 0);
	    count += summon_angel(&y, &x);
	    if (blind && count) message("You hear something appear nearby.", 0);
	    break;

	  case 62:
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" magically summons Spiders.", 0);
	    for (k = 0; k < 6; k++) count += summon_spider(&y, &x);
	    if (blind && count) message("You hear many things appear nearby.", 0);
	    break;

	  case 63:
	    message(m_name, 0x03);
	    if (!blind) message(" mumbles.", 0);
	    else message(" magically summons Hounds.", 0);
	    for (k = 0; k < 8; k++) count += summon_hound(&y, &x);
	    if (blind && count) message("You hear many things appear nearby.", 0);
	    break;

	  case 64:
	    message(m_name, 0x03);
	    if (blind) message(" breathes.", 0);
	    else message(" breathes Nexus.", 0);
	    breath(m_idx, GF_NEXUS,
		((m_ptr->hp / 3) > 250 ? 250 : (m_ptr->hp / 3)));
	    break;

	  case 65:
	    message(m_name, 0x03);
	    if (blind) message(" breathes.", 0);
	    else message(" breathes elemental force.", 0);

	    /* Breath "walls", at PLAYER location */
	    /* XXX This should be done as a "beam" */
	    if (randint(10) == 1) {
		br_wall(char_row, char_col);
	    }

	    /* Normal breath */
	    else {
		breath(m_idx, GF_FORCE,
		       ((m_ptr->hp / 6) > 200 ? 200 : (m_ptr->hp / 6)));
	    }
	    break;

	  case 66:
	    message(m_name, 0x03);
	    if (blind) message(" breathes.", 0);
	    else message(" breathes inertia.", 0);
	    breath(m_idx, GF_INERTIA,
		   ((m_ptr->hp / 6) > 200 ? 200 : (m_ptr->hp / 6)));
	    break;

	  case 67:
	    message(m_name, 0x03);
	    if (blind) message(" breathes.", 0);
	    else message(" breathes light.", 0);
	    breath(m_idx, GF_LITE,
		((m_ptr->hp / 6) > 400 ? 400 : (m_ptr->hp / 6)));
	    break;

	  case 68:
	    message(m_name, 0x03);
	    if (blind) message(" breathes.", 0);
	    else message(" breathes time.", 0);
	    breath(m_idx, GF_TIME,
		((m_ptr->hp / 3) > 150 ? 150 : (m_ptr->hp / 3)));
	    break;

	  case 69:		   /* gravity */
	    message(m_name, 0x03);
	    if (blind) message(" breathes.", 0);
	    else message(" breathes gravity.", 0);
	    breath(m_idx, GF_GRAVITY,
		((m_ptr->hp / 3) > 200 ? 200 : (m_ptr->hp / 3)));
	    break;

	  case 70:		   /* darkness */
	    message(m_name, 0x03);
	    if (blind) message(" breathes.", 0);
	    else message(" breathes darkness.", 0);
	    breath(m_idx, GF_DARK,
		((m_ptr->hp / 6) > 400 ? 400 : (m_ptr->hp / 6)));
	    break;

	  case 71:		   /* plasma */
	    message(m_name, 0x03);
	    if (blind) message(" breathes.", 0);
	    else message(" breathes plasma.", 0);
	    breath(m_idx, GF_PLASMA,
		((m_ptr->hp / 6) > 150 ? 150 : (m_ptr->hp / 6)));
	    break;

	  case 72:
	    if (blind) {
		message("You hear the 'twang' of a bowstring.", 0);
	    }
	    else {
		message(m_name, 0x03);
		message(" fires an arrow at you.", 0);
	    }
	    bolt(m_idx, GF_ARROW, damroll(1, 6));
	    break;

	  case 73:
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" magically summons mighty undead opponents.", 0);
	    for (k = 0; k < 10; k++) count += summon_wraith(&y, &x);
	    for (k = 0; k < 7; k++) count += summon_gundead(&y, &x);
	    if (blind && count) message("You hear many creepy things appear nearby.", 0);
	    break;

	  case 74:		   /* Big darkness storm */
	    message(m_name, 0x03);
	    if (blind) message(" mumbles powerfully.", 0);
	    else message(" casts a Darkness Storm.", 0);
	    breath(m_idx, GF_DARK,
		((m_ptr->hp / 6) > 500 ? 500 : (m_ptr->hp / 6)));
	    break;

	  case 75:		   /* Mana storm */
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" invokes a Mana Storm.", 0);
	    breath(m_idx, GF_MANA,
		   (r_list[m_ptr->r_idx].level * 5) + damroll(10, 10));
	    break;

	  case 76:		   /* Summon reptiles */
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" magically summons reptiles.", 0);
	    for (k = 0; k < 8; k++) count += summon_reptile(&y, &x);
	    if (blind && count) message("You hear many things appear nearby.", 0);
	    break;

	  case 77:		   /* Summon ants */
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" magically summons ants.", 0);
	    for (k = 0; k < 7; k++) count += summon_ant(&y, &x);
	    if (blind && count) message("You hear many things appear nearby.", 0);
	    break;

	  case 78:		   /* Summon unique monsters */
	    message(m_name, 0x03);
	    if (!blind) message(" mumbles.", 0);
	    else message(" summons special opponents!", 0);
	    for (k = 0; k < 5; k++) count += summon_unique(&y, &x);
	    for (k = 0; k < 4; k++) count += summon_jabberwock(&y, &x);
	    if (blind && count) message("You are worried by what you hear nearby.", 0);
	    break;

	  case 79:		   /* Summon greater undead */
	    message(m_name, 0x03);
	    if (!blind) message(" mumbles.", 0);
	    else message(" summons the DEAD!", 0);
	    for (k = 0; k < 8; k++) count += summon_gundead(&y, &x);
	    if (blind && count) message("A chill runs down your spine.", 0);
	    break;

	  case 80:		   /* Summon ancient dragons */
	    message(m_name, 0x03);
	    if (blind) message(" mumbles.", 0);
	    else message(" summons ancient dragons.", 0);
	    for (k = 0; k < 5; k++) count += summon_ancientd(&y, &x);
	    if (blind && count) message("You hear many huge things appear nearby.", 0);
	    break;

	  default:
	    message("Oops. ", 0x02);
	    message(m_name, 0x03);
	    message(" casts a buggy spell.", 0);
	}

	/* Remember what the monster did to us */
	/* If we can see him, or he cast "teleport away" or "teleport level" */
	if ((m_ptr->ml)	|| (thrown_spell == 45) || (thrown_spell == 57)) {

	    if (thrown_spell < 33) {
		l_list[m_ptr->r_idx].r_spells1 |= 1L << (thrown_spell - 1);
	    }
	    else if (thrown_spell < 65) {
		l_list[m_ptr->r_idx].r_spells2 |= 1L << (thrown_spell - 33);
	    }
	    else if (thrown_spell < 97) {
		l_list[m_ptr->r_idx].r_spells3 |= 1L << (thrown_spell - 65);
	    }

	    /* Count the number of castings of this spell */
	    if ((l_list[m_ptr->r_idx].r_spells1 & CS1_FREQ) != CS1_FREQ) {
		l_list[m_ptr->r_idx].r_spells1++;
	    }
	    
	    /* Count the player deaths, if dead */
	    if (death && l_list[m_ptr->r_idx].r_deaths < MAX_SHORT) {
		l_list[m_ptr->r_idx].r_deaths++;
	    }
	}
    }
}


/*
 * Let the given monster attempt to reproduce.
 * Note that "reproduction" REQUIRES empty space.
 */
int multiply_monster(int m_idx)
{
    register int        i, y2, x2;
    register cave_type *c_ptr;

    monster_type *m_ptr = &m_list[m_idx];

    int fx = m_ptr->fx;
    int fy = m_ptr->fy;

    int result = FALSE;


    /* Try up to 18 times */
    for (i = 0; i < 18; i++) {

	/* Pick a location near the given one */
	y2 = fy - 2 + randint(3);
	x2 = fx - 2 + randint(3);

	/* Ignore illegal ones */
	if (!floor_grid(y2, x2)) continue;

	/* Do not eat yourself */
	if ((y2 == fy) && (x2 == fx)) continue;

	/* Do not drop on the player (also checked below) */
	if ((y2 == char_row) && (x2 == char_col)) continue;

	/* Get the cave grid */
	c_ptr = &cave[y2][x2];

	/* Must have space to reproduce */
	if (c_ptr->m_idx) continue;

	/* XXX Hack -- Do not drop onto objects (such as traps) */
	if (c_ptr->i_idx) continue;


	/* Create a new monster */
	result = place_monster(y2, x2, m_ptr->r_idx, FALSE);

	/* Failed to create! */
	if (!result) return FALSE;

	/* Hack -- Made a new monster */
	mon_tot_mult++;

	/* Return the visibility of the created monster */
	return (m_list[cave[y2][x2].m_idx].ml);
    }


    /* Nobody got made */
    return FALSE;
}


/*
 * Move a critter about the dungeon			-RAK-	 
 * Note that update_mon() has been called, so "m_ptr->ml" is correct.
 * Thus, we will just directly modify the monster lore when appropriate.
 * Note that make_move() will call update_mon() if necessary.
 *
 * Note that monsters are only allowed to reproduce "MAX_MON_MULT"
 * times per level.  The "mon_tot_mult" variable starts at zero and
 * increased once per "multiply_monster()".  It is also decreased once
 * per deleted monster, in misc.c, by delete_monster().  So monsters
 * can multiply up to 75 entities, but then they will stop until the
 * player kills some of them off.  This is a silly algorithm.
 */
static void mon_move(int m_idx)
{
    register int           i, j, t;
    int                    k, move_test, dir;
    register monster_type *m_ptr;
    register monster_race *r_ptr;
    register monster_lore *l_ptr;
    int                    mm[9];
    bigvtype               out_val;
    char		   m_name[80];
    char		   m_poss[8];

    /* Get the monster, its race, and its lore info */
    m_ptr = &m_list[m_idx];
    r_ptr = &r_list[m_ptr->r_idx];
    l_ptr = &l_list[m_ptr->r_idx];


    /* reduce fear, tough monsters can unfear faster -CFT, hacked by DGK */
    if (m_ptr->monfear) {

	/* Reduce the fear (use ints to avoid problems) */
	t = (int)m_ptr->monfear;
	t -= randint(r_list[m_ptr->r_idx].level / 10);
	if (t <= 0) t = 0;
	m_ptr->monfear = (int8u) t;

	/* Recover from fear, take note if seen */
	if (!m_ptr->monfear && m_ptr->ml) {

	    /* Get the monster name/poss */
	    monster_desc(m_name, m_ptr, 0);
	    monster_desc(m_poss, m_ptr, 0x22);
	    sprintf(out_val, "%s recovers %s courage.", m_name, m_poss);
	    message(out_val, 0x01);
	}
    }


    /* Does the critter multiply?  Are creatures allowed to multiply? */
    /* Hack -- If the player is resting, only multiply occasionally */
    if ((r_ptr->cflags1 & MF1_MULTIPLY) &&
	(mon_tot_mult <= MAX_MON_MULT) &&
	(!p_ptr->rest || (randint(MON_MULT_ADJ) == 1)) ) {

	/* Count the adjacent monsters */
	k = 0;
	for (i = (int)m_ptr->fy - 1; i <= (int)m_ptr->fy + 1; i++) {
	    for (j = (int)m_ptr->fx - 1; j <= (int)m_ptr->fx + 1; j++) {
		if (in_bounds(i, j) && (cave[i][j].m_idx > 1)) {
		    k++;
		}
	    }
	}

	/* No non-positive multiplication */
	if (k <= 0) k = 1;
	if ((k < 4) && (randint(k * MON_MULT_ADJ) == 1)) {

	    /* Try to multiply, take note if kid is visible */
	    if (multiply_monster(m_idx)) {

		/* Take note if mom is visible too */
		if (m_ptr->ml) l_ptr->r_cflags1 |= MF1_MULTIPLY;
	    }
	}
    }


    move_test = FALSE;

    /* if in wall, must immediately escape to a clear area */
    if ((cave[m_ptr->fy][m_ptr->fx].fval >= MIN_WALL) &&
	(!(r_ptr->cflags1 & MF1_THRO_WALL))) {

    /*
     * If the monster is already dead, don't kill it again! This can happen
     * for monsters moving faster than the player.  They will get multiple
     * moves, but should not if they die on the first move.  This is only a
     * problem for monsters stuck in rock.  
     */

	/* Hack -- Handle "fast dead monsters in rocks" */
	if (m_ptr->hp < 0) return;

	/* note direction of for loops matches direction of keypad from 1 to 9 */
	/* do not allow attack against the player */
	k = 0;
	dir = 1;
	for (i = m_ptr->fy + 1; i >= (int)(m_ptr->fy - 1); i--) {
	    for (j = m_ptr->fx - 1; j <= (int)(m_ptr->fx + 1); j++) {
		if ((dir != 5) && floor_grid(i,j) && (cave[i][j].m_idx != 1)) {
		    mm[k++] = dir;
		}
		dir++;
	    }
	}

	if (k != 0) {
	    /* put a random direction first */
	    dir = randint(k) - 1;
	    i = mm[0];
	    mm[0] = mm[dir];
	    mm[dir] = i;
	    make_move(m_idx, mm);
	    /* this can only fail if mm[0] has a rune of protection */
	}

	/* Hack -- if still in a wall, apply more damage, and dig out */
	if (cave[m_ptr->fy][m_ptr->fx].fval >= MIN_WALL) {

	    /* XXX XXX XXX The player may not have caused the rocks */
	    
	    /* Apply damage, check for death */
	    if (mon_take_hit(m_idx, damroll(8, 8), FALSE)) {
		msg_print("You hear a scream muffled by rock!");
	    }
	    else {
		(void)twall((int)m_ptr->fy, (int)m_ptr->fx, 1, 0);
	    }
	}

	/* monster movement finished */
	return;
    }

    /* Creature is confused?  Chance it becomes un-confused  */
    else if (m_ptr->confused) {

	mm[0] = randint(9);
	mm[1] = randint(9);
	mm[2] = randint(9);
	mm[3] = randint(9);
	mm[4] = randint(9);

	/* don't move him if he is not supposed to move! */
	if (!(r_ptr->cflags1 & MF1_MV_ONLY_ATT)) {
	    if (m_ptr->ml) l_ptr->r_cflags1 |= MF1_MV_ONLY_ATT;
	    make_move(m_idx, mm);
	}

	/* reduce conf, tough monsters can unconf faster -CFT */
	/* use int so avoid unsigned wraparound -CFT */
	t = (int)m_ptr->confused;
	t -= randint(r_list[m_ptr->r_idx].level / 10);
	if (t < 0) t = 0;
	m_ptr->confused = (int8u) t;

	move_test = TRUE;
    }

    /* Creature may cast a spell */
    else if (r_ptr->spells1 != 0) {
	mon_cast_spell(m_idx, &move_test);
    }


    /* XXX This is weird, was 75% random supposed to mean that 75% of the moves */
    /* were random, or that 75% of the time, the monster moves, always randomly? */

    if (!move_test) {

	/* 75% random movement */
	if ((r_ptr->cflags1 & MF1_MV_75) && (randint(100) < 75)) {
	    if (m_ptr->ml) l_ptr->r_cflags1 |= MF1_MV_75;
	    mm[0] = randint(9);
	    mm[1] = randint(9);
	    mm[2] = randint(9);
	    mm[3] = randint(9);
	    mm[4] = randint(9);
	    make_move(m_idx, mm);
	}

	/* 40% random movement */
	else if ((r_ptr->cflags1 & MF1_MV_40) && (randint(100) < 40)) {
	    if (m_ptr->ml) l_ptr->r_cflags1 |= MF1_MV_40;
	    mm[0] = randint(9);
	    mm[1] = randint(9);
	    mm[2] = randint(9);
	    mm[3] = randint(9);
	    mm[4] = randint(9);
	    make_move(m_idx, mm);
	}

	/* 20% random movement */
	else if ((r_ptr->cflags1 & MF1_MV_20) && (randint(100) < 20)) {
	    if (m_ptr->ml) l_ptr->r_cflags1 |= MF1_MV_20;
	    mm[0] = randint(9);
	    mm[1] = randint(9);
	    mm[2] = randint(9);
	    mm[3] = randint(9);
	    mm[4] = randint(9);
	    make_move(m_idx, mm);
	}

	/* Normal movement */
	else if (r_ptr->cflags1 & MF1_MV_ATT_NORM) {

	    if (m_ptr->ml) l_ptr->r_cflags1 |= MF1_MV_ATT_NORM;
	    get_moves(m_idx, mm);

	    /* For variety, stumble occasionally */
	    if (randint(200) == 1) {
		mm[0] = randint(9);
		mm[1] = randint(9);
		mm[2] = randint(9);
		mm[3] = randint(9);
		mm[4] = randint(9);
	    }

	    make_move(m_idx, mm);
	}

	/* Attack, but don't move */
	else if ((r_ptr->cflags1 & MF1_MV_ONLY_ATT) && (m_ptr->cdis < 2)) {
	    if (m_ptr->ml) l_ptr->r_cflags1 |= MF1_MV_ONLY_ATT;
	    get_moves(m_idx, mm);
	    make_move(m_idx, mm);
	}

	/* Little hack for Quylthulgs, so that will eventually notice */
	/* that they have no physical attacks */
	else if ((r_ptr->cflags1 & CM1_ALL_MV_FLAGS) == 0 && (m_ptr->cdis < 2)) {
	    if ((m_ptr->ml) && (l_ptr->r_attacks[0] < MAX_UCHAR)) {
		l_ptr->r_attacks[0]++;
	    }
	}
    }
}




/*
 * Creatures movement and attacking are done from here	-RAK-
 *
 * Get rid of eaten/breathed on monsters.  Note: Be sure not to process
 * these monsters. This is necessary because we can't delete monsters
 * while scanning the m_list here.  See "hack_m_idx".
 *
 * Poly-morph is extremely dangerous, cause it changes the
 * monster race.  Luckily, it seems to be done by killing
 * the old monster and making a new one.  Usually...
 *
 * Note that this routine may be called for two reasons.  If "attack" is
 * TRUE, then it is actually time for the monsters to move and stuff.  But
 * if "attack" is FALSE, we are just "updating" the monsters, say, after
 * the player casts "detection", to make sure that the monster flags are
 * correctly set, in particular, the monster visibility flag "ml".
 * In this case, we simply branch to "update_monsters()", the proper routine.
 *
 * Notice the hacks to deal with "dead" monsters that could not be deleted
 * at the time.  See "hack_m_idx" for info.  Note that we check this at the
 * beginning to deal with the monster having been killed by another monster
 * appearing later in the list, and at the end to deal with monsters that in
 * some bizarre way manage to commit suicide.
 *
 * Note that this routine ASSUMES that "update_monsters()" has been called
 * every time the player changes his view or lite, and that "update_mon()"
 * has been, and will be, called every time a monster moves.  Likewise, we
 * rely on the lite/view/etc being "verified" EVERY time a door/wall appears
 * or disappears.  This will then require minimal scans of the "monster" array.
 * Note that only "some" calls to "update_mon()" actually need to recalculate
 * the "distance" field, but they all do now for consistency.
 *
 * We assume that a monster, during its own "turn", can NEVER:
 *   (1) induce its own "death"
 *   (2) change its own "race"
 *   (3) move in the monster array
 *
 * The last one (#3) is guaranteed by the use of "hack_m_idx" to hold the
 * "current monster index", and by the "queuing" of monster deaths by monsters
 * who appear "earlier" in the array then the "hack_m_idx" monster.
 *
 * Actually, the second one is simpler, since a monster's race NEVER changes,
 * and we handle "polymorph" by creating a "new" monster after removing
 * the old one.  I think.
 *
 * It is very important to process the array "backwards", as "newly created"
 * monsters should not get a turn until the next turn.
 *
 * Note that (eventually) we should "remove" the "street urchin" monster
 * record, and use "monster race zero" as the "no-race" indicator, that is,
 * the indicator of "free space" in the monster list.  But that is of a very
 * low priority for now.
 */
void process_monsters(void)
{
    register int          i, k;
    register monster_type *m_ptr;
    register monster_race *r_ptr;
    monster_lore          *l_ptr;
    int			  fx, fy;
    bool                  wake, ignore;


    /* Process the monsters (backwards) */
    for (i = m_max - 1; i >= MIN_M_IDX; i--) {

	/* Hack -- This monster is "sacred" */
	hack_m_idx = i;

	/* Hack -- Remove dead monsters. */
	if (m_list[i].hp < 0) {

	    /* We are no longer sacred */
	    hack_m_idx = i - 1;

	    /* Kill us */
	    delete_monster_idx(i);

	    /* Continue */
	    continue;
	}

	/* Hack -- notice player death  */
	if (death) continue;


	/* Get the i'th monster */
	m_ptr = &m_list[i];

	/* Get the monster race */
	r_ptr = &r_list[m_ptr->r_idx];

	/* Get the monster lore */
	l_ptr = &l_list[m_ptr->r_idx];


	/* Hack -- Update the monster record.  This catches both player movement */
	/* and wall-breaking, door-opening, or teleportation by other monsters. */
	/* update_mon(i); <-- not needed if everyone calls it correctly */


	/* Determine the number of moves this turn */
	k = movement_rate(i);


	/* Get the monster location */
	fx = m_ptr->fx;
	fy = m_ptr->fy;

	/* Hack -- monsters trapped in rock get real agitated */
	if ((cave[fy][fx].fval >= MIN_WALL) &&
	    (!(r_ptr->cflags1 & MF1_THRO_WALL)) ) {

	    /* Wake up fast */
	    m_ptr->csleep = 0;
	    m_ptr->stunned = 0;

	    /* Hack -- move immediately */
	    mon_move(i);

	    /* Hack -- No normal movement */
	    k = 0;
	}


	/* Process the moves */
	while (k-- > 0) {

	    wake = FALSE;
	    ignore = FALSE;

	    /* Get the monster location */
	    fx = m_ptr->fx;
	    fy = m_ptr->fy;


	    /* Monsters have a "player sensing radius" */
	    /* Monsters have "very long range vision" */
	    if ((m_ptr->cdis <= r_ptr->aaf) ||
		((m_ptr->cdis <= MAX_SIGHT * 2) &&
		 (los(fy, fx, char_row, char_col)))) {

		/* Monsters wake up occasionally */
		if (m_ptr->csleep > 0) {

		    /* Aggravation wakes up everyone (nearby) */
		    if (p_ptr->aggravate) {
			m_ptr->csleep = 0;
		    }

		    /* Hack -- If the player is resting or paralyzed, then only */
		    /* run this block about one turn in 50, since he is "quiet". */
		    else if ((p_ptr->rest == 0 && p_ptr->paralysis < 1) ||
			     (randint(50) == 1)) {

			int32u notice;
			
			/* Bizarre computation to see if monster "notices" player */
			notice = randint(1024);
			if ((notice * notice * notice) <=
			    (1L << (29 - p_ptr->stl))) {

			    /* Monster wakes up "a little bit" */
			    m_ptr->csleep -= (100 / m_ptr->cdis);
			    if (m_ptr->csleep > 0) {
				/* Still sleeping */
				ignore = TRUE;
			    }
			    else {
				/* force it to be exactly zero */
				m_ptr->csleep = 0;
				wake = TRUE;
			    }
			}
		    }
		}

		/* Process stunned-ness */
		if (m_ptr->stunned > 0) {

		    /* Recover a little bit */
		    m_ptr->stunned--;

		    /* Make a "saving throw" against stun */
		    /* Level 70 (or above) monsters recover instantly */
		    if (randint(5000) < r_ptr->level * r_ptr->level) {
			m_ptr->stunned = 0;
		    }

		    /* Recover from stun */
		    if (m_ptr->stunned == 0) {
			char m_name[80];
			monster_desc(m_name, m_ptr, 0);
			message(m_name, 0x03);
			message(" recovers and glares at you.", 0);
		    }
		}

		/* Not sleeping, not stunned, so "move" (and update) */
		if ((m_ptr->csleep == 0) && (m_ptr->stunned == 0)) mon_move(i);
	    }


	    /* Count "waking" and "ignoring" by visible monsters */
	    if (m_ptr->ml) {
		if (wake) {
		    if (l_ptr->r_wake < MAX_UCHAR) l_ptr->r_wake++;
		}
		else if (ignore) {
		    if (l_ptr->r_ignore < MAX_UCHAR) l_ptr->r_ignore++;
		}
	    }
	}


	/* Hack -- This monster is no longer sacred */
	hack_m_idx = i - 1;


	/* XXX What monster gets killed during its own turn? */
	/* I suppose it could "attack" itself in some bizarre way */

	/* Hack -- Remove dead monsters. */
	if (m_list[i].hp < 0) delete_monster_idx(i);
    }


    /* Hack -- nobody is sacred now */
    hack_m_idx = -1;


    /* Flush the screen */
    Term_fresh();
}



/*
 * Apply an "earthquake" to the player's location
 * Take the "cause" of the quake as a paramater
 *
 * This function ALWAYS "re-updates" the view/lite/monsters
 */
static void quake_player(cptr what)
{
    register int        x, y;
    register int	cx, cy;
    register cave_type *c_ptr;
    int                 tmp;
    int			damage = 0;
    int			sy = 0, sx = 0, safe = 0;


    /* Get the player location */
    cx = char_row;
    cy = char_col;

    /* See if we can find a "safe" location to "move" to */
    for (y = cy - 1; !safe && y <= cy + 1; y++) {
	for (x = cx - 1; !safe && x <= cx + 1; x++) {
	    if ((y == cy) && (x == cx)) continue;
	    if (floor_grid(y, x) && (!cave[y][x].m_idx)) {
		sy = y, sx = x, safe = TRUE;
	    }
	}
    }

    /* Random message */
    switch (randint(3)) {
      case 1:
	msg_print("The cave ceiling collapses!");
	break;
      case 2:
	msg_print("The cave floor twists in an unnatural way!");
	break;
      default:
	msg_print("The cave quakes!  You are pummeled with debris!");
	break;
    }


    /* Hurt the player a lot (is this "fair"?) */
    if (!safe) {
	msg_print("You are trapped, crushed and cannot move!");
	damage = 300;
    }

    /* Destroy the grid, and push the player to safety */
    else {

	/* Calculate results */
	switch (randint(3)) {
	  case 1:
	    msg_print("You nimbly dodge the blast!");
	    damage = 0;
	    break;
	  case 2:
	    msg_print("You are bashed by rubble!");
	    damage = damroll(10, 4);
	    stun_player(randint(50));
	    break;
	  case 3:
	    msg_print("You are crushed between the floor and ceiling!");
	    damage = damroll(10, 4);
	    stun_player(randint(50));
	    break;
	}


	/* Destroy the player's old grid */
	c_ptr = &cave[cy][cx];

	/* Destroy location (unless artifact or stairs) */
	if (valid_grid(cy, cx)) {
	    tmp = randint(10);
	    if (tmp < 6) c_ptr->fval = QUARTZ_WALL;
	    else if (tmp < 9) c_ptr->fval = MAGMA_WALL;
	    else c_ptr->fval = GRANITE_WALL;
	    delete_object(cy, cx);
	    c_ptr->info &= ~CAVE_PL;
	    c_ptr->info &= ~CAVE_FM;
	    lite_spot(cy, cx);
	}


	/* Move the player to the safe location */
	move_rec(char_row, char_col, sy, sx);
	
	/* Check for new panel (redraw map) */
	(void)get_panel(char_row, char_col, FALSE);

	/* Update the view */
	update_view();

	/* Update the lite */
	update_lite();

	/* Update the monsters */
	update_monsters();

	/* Check the view */
	check_view();
    }

    
    
    /* Take some damage */
    if (damage) take_hit(damage, what);
}




/*
 * This is a fun one.  In a given block, pick some walls and
 * turn them into open spots.  Pick some open spots and turn
 * them into walls.  An "Earthquake" effect.	       -LVB-
 *
 * Note below that we prevent unique monster from death by other monsters.
 * It causes trouble (monster not marked as dead, quest monsters don't
 * satisfy quest, etc).  So, we let then live, but extremely wimpy.
 * This isn't great, because monster might heal itself before player's
 * next swing... -CFT
 *
 * Note that since the entire outer edge of the maze is solid rocks,
 * we can skip "in_bounds()" checks of distance "one" from any monster.
 *
 * XXX Replace this with a "radius 8" ball attack on the monster itself.
 */
static void shatter_quake(int cy, int cx)
{
    register int		i, j;
    register cave_type		*c_ptr;
    register monster_type	*m_ptr;
    register monster_race	*r_ptr;
    int				kill, y, x;
    int				tmp, damage = 0;
    char			m_name[80];


    /* Check around the epicenter */
    for (i = cy - 8; i <= cy + 8; i++) {
	for (j = cx - 8; j <= cx + 8; j++) {

	    /* Skip the epicenter */
	    if ((i == cy) && (j == cx)) continue;

	    /* Skip illegal grids */
	    if (!in_bounds(i, j)) continue;

	    /* Sometimes, shatter that grid */
	    if (randint(8) == 1) {

		c_ptr = &cave[i][j];

		/* See if the player is here */

		/* Treat the player grid specially */
		if ((i == char_row) && (j == char_col)) {

		    /* Quake the player */
		    quake_player("an earthquake");

		    /* Continue */
		    continue;
		}


		/* Embed a (non-player) monster */
		if (c_ptr->m_idx > 1) {

		    m_ptr = &m_list[c_ptr->m_idx];
		    r_ptr = &r_list[m_ptr->r_idx];

		    if (!(r_ptr->cflags1 & MF1_THRO_WALL) &&
			!(r_ptr->cflags2 & MF2_BREAK_WALL)) {

			/* Monster can not move to escape the wall */
			if ((movement_rate(c_ptr->m_idx) == 0) ||
			    (r_ptr->cflags1 & MF1_MV_ONLY_ATT)) {
			    kill = TRUE;
			}

			/* The monster MAY be able to dodge the walls */
			else {

			    /* Assume surrounded by rocks */
			    kill = TRUE;

			    /* Look for non-rock space (dodge later) */
			    for (y = i - 1; y <= i + 1; y++) {
				for (x = j - 1; x <= j + 1; x++) {
				    if (y == i && x == j) continue;
				    if (floor_grid(y, x)) kill = FALSE;
				}
			    }
			}

			/* Scream in pain */
			monster_desc(m_name, m_ptr, 0);
			message(m_name, 0x03);
			message(" wails out in pain!", 0);

			/* Take damage from the quake */
			damage = damroll(4, 8);

			/* Monster is certainly awake */
			m_ptr->csleep = 0;

			/* This is NOT player induced damage */
			m_ptr->hp -= damage;

			/* If totally embedded, die instantly */
			if (kill) m_ptr->hp = -1;

			/* Unique monsters will not quite die */
			if ((r_ptr->cflags2 & MF2_UNIQUE) && (m_ptr->hp < 0)) {
			    m_ptr->hp = 0;
			}

			/* Handle "dead monster" */
			if (m_ptr->hp < 0) {

			    message(m_name, 0x03);
			    message(" is embedded in the rock.", 0);

			    /* Average the monster and dungeon levels */
			    object_level = (dun_level + r_ptr->level) >> 1;

			    /* Kill the monster */
			    monster_death(m_ptr, FALSE, m_ptr->ml);

			    /* Delete it -- see "hack_m_idx" */
			    delete_monster_idx(c_ptr->m_idx);
			}
		    }
		}

		/* Do not hurt artifacts or stairs */
		if (valid_grid(i, j)) {

		    /* Delete any "walls" or "rubble" or "doors" */
		    if (!floor_grid(i, j)) {
			c_ptr->fval = CORR_FLOOR;
		    }

		    /* Floor grids turn into rubble.  */
		    else {
			tmp = randint(10);
			if (tmp < 6) c_ptr->fval = QUARTZ_WALL;
			else if (tmp < 9) c_ptr->fval = MAGMA_WALL;
			else c_ptr->fval = GRANITE_WALL;
		    }

		    /* Delete any object that is still there */
		    delete_object(i, j);

		    /* Hack -- not lit, not known */
		    c_ptr->info &= ~CAVE_PL;
		    c_ptr->info &= ~CAVE_FM;

		    /* Erase the grid */
		    lite_spot(i, j);
		}
	    }
	}
    }
    
    /* Hack -- always update the view/lite/etc */
    update_view();
    update_lite();
    update_monsters();
}



/*
 * Hack -- Drop a "wall" on the player, who must be at the given location.
 *
 * XXX XXX Perhaps replace this with a "beam" of rock breathing.
 */
static void br_wall(int cy, int cx)
{
    /* Hack -- Verify location */
    if (cx != char_row || cy != char_col) return;

    /* Take some damage */
    quake_player("a breathed wall");
}




