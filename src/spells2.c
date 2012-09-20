/* File: spells.c */

/* Purpose: player and creature spells, breaths, wands, scrolls, etc. */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "angband.h"






/*
 * Helper function -- return a "nearby" race for polymorphing
 */
int poly_r_idx(int r_idx)
{
    register monster_race *r_ptr;

    int lev1, lev2;

    /* Get that monster race */
    r_ptr = &r_list[r_idx];

    /* Uniques never polymorph */
    if (r_ptr->cflags2 & MF2_UNIQUE) return (r_idx);


    /* Allowable range of "levels" for resulting monster */
    lev1 = r_ptr->level - ((randint(20)/randint(9))+1);
    lev2 = r_ptr->level + ((randint(20)/randint(9))+1);


    /* Pick a (possibly new) non-unique race */
    while (1) {

	/* Pick a random race */
	r_idx = rand_int(MAX_R_IDX-1);

	/* Extract that monster */
	r_ptr = &r_list[r_idx];

	/* Skip uniques */
	if (r_ptr->cflags2 & MF2_UNIQUE) continue;

	/* XXX Apply "rarity" */

	/* Accept valid monsters */
	if ((r_ptr->level >= lev1) && (r_ptr->level <= lev2)) break;
    }

    /* Return the result */
    return (r_idx);
}


/*
 * polymorph is now uniform for poly/mass poly/choas poly, and only
 * as deadly as chaos poly is.  This still makes polymorphing a bad
 * idea, but it won't be automatically fatal. -CFT 
 */
static int poly(int mnum)
{
    register monster_type *m_ptr;
    register monster_race *r_ptr;

    int y, x, r_idx;

    /* Get the initial monster and race */
    m_ptr = &m_list[mnum];
    r_ptr = &r_list[m_ptr->r_idx];

    /* Pick a "polymorph" destination */
    r_idx = poly_r_idx(m_ptr->r_idx);

    /* Sometimes nothing happens (see above) */
    if (r_idx == m_ptr->r_idx) return (FALSE);

    /* Save the monster location */
    y = m_ptr->fy;
    x = m_ptr->fx;

    /* "Kill" the monster */
    delete_monster_idx(mnum);

    /* Place the new monster where the old one was */
    place_monster(y,x,r_idx,FALSE);

    /* Success */
    return (TRUE);
}



/*
 * Create a wall.		-RAK-	 
 *
 * XXX COnvert this to "project()" function.
 */
int build_wall(int dir, int y, int x)
{
    int                 build, damage, dist, flag;
    cave_type		*c_ptr;
    monster_type	*m_ptr;
    monster_race	*r_ptr;
    vtype               out_val;
    char		m_name[80];

    build = FALSE;
    dist = 0;

    /* Go till done */
    for (flag = FALSE; !flag; ) {
	(void)mmove(dir, &y, &x);
	dist++;
	c_ptr = &cave[y][x];
	if ((dist > OBJ_BOLT_RANGE) || !floor_grid_bold(y,x)) {
	    flag = TRUE;
	}
	else {
	    if (c_ptr->m_idx > 1) {

		m_ptr = &m_list[c_ptr->m_idx];
		r_ptr = &r_list[m_ptr->r_idx];

		/* stop the wall building */
		flag = TRUE;

		if (!(r_ptr->cflags1 & MF1_THRO_WALL)) {

		    /* monster does not move, can't escape the wall */
		    if (r_ptr->cflags1 & MF1_MV_ONLY_ATT) {
			damage = 250;
		    }
		    else {
			damage = damroll(4, 8);
		    }

		    monster_desc(m_name, m_ptr, 0);
		    (void)sprintf(out_val, "%s wails out in pain!", m_name);
		    message(out_val,0x01);

		    if (mon_take_hit((int)c_ptr->m_idx, damage, TRUE)) {
			(void)sprintf(out_val, "%s is embedded in the rock.",
				      m_name);
			message(out_val,0x01);
			/* prt_experience(); */
		    }
		}

		/* Earth elementals and Xorn's actually GAIN hitpoints */
		else if (r_ptr->r_char == 'E' || r_ptr->r_char == 'X') {
		    m_ptr->hp += damroll(4, 8);
		}
	    }

	    if (!valid_grid(y,x)) continue;

	    delete_object(y, x);
	    c_ptr->fval = MAGMA_WALL;
	    c_ptr->info &= ~CAVE_FM;
	    lite_spot(y, x);
	    build = TRUE;
	}
    }

    /* Update the view, etc */
    update_view();
    update_lite();
    update_monsters();

    /* Success? */    
    return (build);
}



/*
 * Move the creature record to a new location		-RAK-	 
 */
void teleport_away(int m_idx, int dis)
{
    register int           yn, xn, ctr;
    register monster_type *m_ptr;

    m_ptr = &m_list[m_idx];

    /* Find a location */    
    for (ctr = 1; TRUE; ctr++) {

	/* Pick a nearby "legal" location */
	while (1) {
	    yn = rand_spread(m_ptr->fy, dis);
	    xn = rand_spread(m_ptr->fx, dis);
	    if (in_bounds(yn, xn)) break;
	}

	/* Accept "empty" floor grids */
	if (empty_grid_bold(yn,xn)) break;

	/* Occasionally try further away */
	if (ctr % 10 == 0) dis += 5;
    }


    /* Move the monster */
    move_rec((int)m_ptr->fy, (int)m_ptr->fx, yn, xn);

    /* Pretend we were previously "unseen" */
    m_ptr->ml = FALSE;

    /* Update the monster */
    update_mon(m_idx);
}


/*
 * Teleport player to spell casting creature		-RAK-	 
 */
void teleport_to(int ny, int nx)
{
    int dis, ctr, y, x;

    dis = 1;
    ctr = 0;

    /* Pick a usable location */    
    while (1) {

	/* Pick a legal location */
	while (1) {

	    /* Pick a location */
	    y = rand_spread(ny, dis);
	    x = rand_spread(nx, dis);

	    /* Verify it */
	    if (in_bounds(y, x)) break;
	}

	/* Accept "naked" floor grids */
	if (naked_grid_bold(y, x)) break;

	/* Count */
	ctr++;

	/* Try farther away */
	if (ctr > (4 * dis * dis + 4 * dis + 1)) {
	    ctr = 0;
	    dis++;
	}
    }


    /* Move the player */
    move_rec(char_row, char_col, y, x);

    /* Check for new panel (redraw map) */
    (void)get_panel(char_row, char_col, FALSE);

    /* Update the view/lite */
    update_view();
    update_lite();

    /* Update creatures */
    update_monsters();


    /* Check the view */
    check_view();
}


/*
 * Delete all creatures within max_sight distance	-RAK-
 * NOTE : Winning creatures cannot be genocided			
 * Wizards can genocide anything they can see
 */
int mass_genocide(int spell)
{
    register int        i, result;
    register monster_type *m_ptr;
    register monster_race *r_ptr;

    result = FALSE;
    for (i = m_max - 1; i >= MIN_M_IDX; i--) {

	m_ptr = &m_list[i];
	r_ptr = &r_list[m_ptr->r_idx];

	if (((m_ptr->cdis <= MAX_SIGHT) &&
	     (!(r_ptr->cflags2 & MF2_UNIQUE))) ||
	    (wizard && (m_ptr->cdis <= MAX_SIGHT))) {

	    /* Delete the monster */
	    delete_monster_idx(i);

	    /* Cute visual feedback as the player slowly dies */
	    if (spell) {
		take_hit(randint(3), "the strain of casting Mass Genocide");
		prt_chp();
		move_cursor_relative(char_row, char_col);
		Term_fresh();
		delay(20 * delay_spd);
	    }

	    result = TRUE;
	}
    }

    return (result);
}

/*
 * Delete (not kill) all creatures of a given type from level.	-RAK-
 * NOTE : Winning creatures can not be genocided.
 */
int genocide(int spell)
{
    register int		i, killed;
    char			typ;
    register monster_type	*m_ptr;
    register monster_race	*r_ptr;
    char			m_name[80];

    killed = FALSE;

    if (get_com("Which type of creature do you wish exterminated?", &typ)) {

	for (i = m_max - 1; i >= MIN_M_IDX; i--) {

	    m_ptr = &m_list[i];
	    r_ptr = &r_list[m_ptr->r_idx];

	    if (r_list[m_ptr->r_idx].r_char == typ) {

		/* Cannot genocide a Quest Monster */
		if (r_ptr->cflags2 & MF2_QUESTOR) {

		    /* Hack -- only if visible */
		    if (m_ptr->ml) {
			/* genocide is a powerful spell, so we will let the */
			/* player know the names of the creatures he did not */
			/* destroy, this message makes no sense otherwise */
			monster_desc(m_name, m_ptr, 0x80);
			message(m_name, 0x03);
			message(" is unaffected!", 0);
		    }
		}

		/* Genocide it */
		else {
		    /* Delete the monster */
		    delete_monster_idx(i);

		    if (spell) {
			take_hit(randint(4), "the strain of casting Genocide");
			prt_chp();
			move_cursor_relative(char_row, char_col);
			Term_fresh();
			delay(20 * delay_spd);
		    }
		    killed = TRUE;
		}
	    }
	}
    }

    return (killed);
}


/*
 * Speed nearby creatures.
 */
int speed_monsters()
{
    register int        i, speed;
    register monster_type *m_ptr;
    register monster_race *r_ptr;
    vtype               out_val;
    char		m_name[80];

    speed = FALSE;
    
    for (i = m_max - 1; i >= MIN_M_IDX; i--) {

	m_ptr = &m_list[i];

	if (!m_ptr->los) continue;

	r_ptr = &r_list[m_ptr->r_idx];
	monster_desc(m_name, m_ptr, 0);

	m_ptr->mspeed += 10;

	m_ptr->csleep = 0;

	if (m_ptr->ml) {
	    speed = TRUE;
	    (void)sprintf(out_val, "%s starts moving faster.", m_name);
	    message(out_val,0x01);
	}
    }

    return (speed);
}


/*
 * Slow all nearby creatures.
 * NOTE: cannot slow a winning creature (BALROG)
 */
int slow_monsters()
{
    register int        i, speed;
    register monster_type *m_ptr;
    register monster_race *r_ptr;
    vtype               out_val;
    char		m_name[80];

    speed = FALSE;

    for (i = m_max - 1; i >= MIN_M_IDX; i--) {

	m_ptr = &m_list[i];

	if (!m_ptr->los) continue;

	r_ptr = &r_list[m_ptr->r_idx];
	monster_desc(m_name, m_ptr, 0);

	if ((r_ptr->level <
	    randint((p_ptr->lev - 10) < 1 ? 1 : (p_ptr->lev - 10)) + 10) &&
		   !(r_ptr->cflags2 & MF2_UNIQUE)) {

	    m_ptr->mspeed -= 10;

	    m_ptr->csleep = 0;

	    if (m_ptr->ml) {
		(void)sprintf(out_val, "%s starts moving slower.", m_name);
		message(out_val,0x01);
		speed = TRUE;
	    }
	}
	else if (m_ptr->ml) {
	    (void)sprintf(out_val, "%s is unaffected.", m_name);
	    message(out_val,0x01);
	}
    }

    return (speed);
}


/*
 * Sleep any creature.		-RAK-	 
 */
int sleep_monsters2(void)
{
    register int        i, sleep;
    register monster_type *m_ptr;
    register monster_race *r_ptr;
    vtype               out_val;
    char		m_name[80];

    sleep = FALSE;
    for (i = m_max - 1; i >= MIN_M_IDX; i--) {
	m_ptr = &m_list[i];
	r_ptr = &r_list[m_ptr->r_idx];
	monster_desc(m_name, m_ptr, 0);

	if (!m_ptr->los) continue;

	if ((r_ptr->level >
	    randint((p_ptr->lev - 10) < 1 ? 1 : (p_ptr->lev - 10)) + 10) ||
	    (r_ptr->cflags2 & MF2_UNIQUE) || (r_ptr->cflags2 & MF2_CHARM_SLEEP)) {

	    if (m_ptr->ml) {
		if (r_ptr->cflags2 & MF2_CHARM_SLEEP) {
		    l_list[m_ptr->r_idx].r_cflags2 |= MF2_CHARM_SLEEP;
		}
		(void)sprintf(out_val, "%s is unaffected.", m_name);
		message(out_val,0x01);
	    }
	}
	else {
	    m_ptr->csleep = 500;
	    if (m_ptr->ml) {
		(void)sprintf(out_val, "%s falls asleep.", m_name);
		message(out_val,0x01);
		sleep = TRUE;
	    }
	}
    }

    return (sleep);
}


/*
 * Crazy function -- polymorph all visible creatures.
 */
int mass_poly()
{
    register int i;
    int mass;
    register monster_type  *m_ptr;
    register monster_race *r_ptr;

    mass = FALSE;
    for (i = m_max - 1; i >= MIN_M_IDX; i--) {
	m_ptr = &m_list[i];
	if (m_ptr->cdis <= MAX_SIGHT) {
	    r_ptr = &r_list[m_ptr->r_idx];
	    if (!(r_ptr->cflags2 & MF2_UNIQUE)) {
		if (poly(i)) mass = TRUE;
	    }
	}
    }

    return (mass);
}


/*
 * Display evil creatures on current panel		-RAK-	 
 */
int detect_evil(void)
{
    register int        i, flag;
    register monster_type *m_ptr;

    flag = FALSE;
    for (i = m_max - 1; i >= MIN_M_IDX; i--) {
	m_ptr = &m_list[i];
	if (panel_contains((int)m_ptr->fy, (int)m_ptr->fx) &&
	    (MF2_EVIL & r_list[m_ptr->r_idx].cflags2)) {

	    /* Draw the monster (even if invisible) */
	    m_ptr->ml = TRUE;
	    lite_monster(m_ptr);

	    flag = TRUE;
	}
    }

    if (flag) {
	msg_print("You sense the presence of evil!");
	msg_print(NULL);

	/* Fix the monsters */
	update_monsters();
    }

    return (flag);
}


/*
 * Change players hit points in some manner		-RAK-	 
 */
int hp_player(int num)
{
    register int          res;

    res = FALSE;

    if (p_ptr->chp < p_ptr->mhp) {
	p_ptr->chp += num;
	if (p_ptr->chp > p_ptr->mhp) {
	    p_ptr->chp = p_ptr->mhp;
	    p_ptr->chp_frac = 0;
	}
	prt_chp();

	num = num / 5;
	if (num < 3) {
	    if (num == 0)
		msg_print("You feel a little better.");
	    else
		msg_print("You feel better.");
	}
	else {
	    if (num < 7)
		msg_print("You feel much better.");
	    else
		msg_print("You feel very good.");
	}
	res = TRUE;
    }
    
    return (res);
}


/*
 * Cure players confusion				-RAK-	 
 */
int cure_confusion()
{
    register int           cure;

    cure = FALSE;

    if (p_ptr->confused > 1) {
	p_ptr->confused = 1;
	cure = TRUE;
    }
    return (cure);
}


/*
 * Cure players blindness				-RAK-	 
 */
int cure_blindness(void)
{
    register int           cure;

    cure = FALSE;

    if (p_ptr->blind > 1) {
	p_ptr->blind = 1;
	cure = TRUE;
    }
    return (cure);
}


/*
 * Cure poisoning					-RAK-	 
 */
int cure_poison(void)
{
    register int           cure;

    cure = FALSE;

    if (p_ptr->poisoned > 1) {
	p_ptr->poisoned = 1;
	cure = TRUE;
    }
    return (cure);
}


/*
 * Cure the players fear				-RAK-	 
 */
int remove_fear()
{
    register int           result;

    result = FALSE;

    if (p_ptr->afraid > 1) {
	p_ptr->afraid = 1;
	result = TRUE;
    }
    return (result);
}




/*
 * This is a fun one.  In a given block, pick some walls and
 * turn them into open spots.  Pick some open spots and turn
 * them into walls.  An "Earthquake" effect.	       -RAK-  
 */
void earthquake(void)
{
    register int        i, j;
    register cave_type *c_ptr;
    register monster_type *m_ptr;
    register monster_race *r_ptr;
    int                 kill, damage, tmp, y, x;
    vtype               out_val;
    char		m_name[80];

    /* Shatter the terrain around the player */
    for (i = char_row - 10; i <= char_row + 10; i++) {
	for (j = char_col - 10; j <= char_col + 10; j++) {

	    /* Verify legality and distance */
	    if (!in_bounds(i, j)) continue;
	    if (distance(char_row, char_col, i, j) > 10) continue;

	    /* Skip the player himself */
	    if ((i == char_row) && (j == char_col)) continue;

	    /* Artifacts and Stairs and Stores resist */
	    if (!valid_grid(i,j)) continue;

	    /* Shatter some of the terrain */	    
	    if (randint(8) == 1) {

		c_ptr = &cave[i][j];

		/* Delete the object */
		delete_object(i, j);

		/* Hurt the monster, if any */
		if (c_ptr->m_idx > 1) {

		    m_ptr = &m_list[c_ptr->m_idx];
		    r_ptr = &r_list[m_ptr->r_idx];

		    if (!(r_ptr->cflags1 & MF1_THRO_WALL) &&
			!(r_ptr->cflags2 & MF2_BREAK_WALL)) {

			/* monster can not move to escape the wall */
			if (r_ptr->cflags1 & MF1_MV_ONLY_ATT) {
			    kill = TRUE;
			}

			/* Monster MAY have somewhere to flee */
			else {
			    kill = TRUE;
			    for (y = i - 1; y <= i + 1; y++) {
				for (x = j - 1; x <= j + 1; x++) {
				    if (!in_bounds(y,x)) continue;
				    if (empty_grid_bold(y, x)) kill = FALSE;
				}
			    }
			}

			if (kill) {
			    damage = 320;
			}
			else {
			    damage = damroll(3 + randint(3), 8+randint(5));
			}

			monster_desc(m_name, m_ptr, 0);
			(void)sprintf(out_val, "%s wails out in pain!", m_name);
			message(out_val,0x01);

			/* Do damage, get experience (?) */
			if (mon_take_hit(c_ptr->m_idx, damage, TRUE)) {
			    sprintf(out_val, "%s is embedded in the rock.",
				    m_name);
			    message(out_val,0x01);
			    prt_experience();
			}
		    }
		}

		/* Make walls, if none there */
		if (c_ptr->fval < MIN_WALL) {
		    tmp = randint(10);
		    if (tmp < 6) c_ptr->fval = QUARTZ_WALL;
		    else if (tmp < 9) c_ptr->fval = MAGMA_WALL;
		    else c_ptr->fval = GRANITE_WALL;
		    c_ptr->info &= ~CAVE_PL;
		    c_ptr->info &= ~CAVE_FM;
		}

		/* Else Destroy (NON-PERMANENT) walls */
		else {
		    c_ptr->fval = CORR_FLOOR;
		    c_ptr->info &= ~CAVE_LR;
		    c_ptr->info &= ~CAVE_PL;
		    c_ptr->info &= ~CAVE_FM;
		}

		/* Redraw */
		lite_spot(i, j);
	    }
	}
    }

    /* Update the view, etc */
    update_view();
    update_lite();
    update_monsters();
}


/*
 * Evil creatures don't like this.		       -RAK-   
 */
int protect_evil()
{
    register int           res;

    res = !p_ptr->protevil;

    p_ptr->protevil += randint(25) + 3 * p_ptr->lev;

    return (res);
}


/*
 * Make the player no longer hungry
 */
void satisfy_hunger(void)
{
    msg_print("You feel full!");
    msg_print(NULL);

    /* No longer hungry */
    p_ptr->food = PLAYER_FOOD_MAX;

    /* Hack -- update the display */
    p_ptr->status &= ~(PY_WEAK | PY_HUNGRY);
    prt_hunger();
}


/*
 * Banish monsters -- a hack.
 */
int banish_evil(int dist)
{
    register int           i;
    int                    dispel;
    register monster_type *m_ptr;

    dispel = FALSE;
    for (i = m_max - 1; i >= MIN_M_IDX; i--) {
	m_ptr = &m_list[i];
	if (!m_ptr->los) continue;
	if (r_list[m_ptr->r_idx].cflags2 & MF2_EVIL) {
	    l_list[m_ptr->r_idx].r_cflags2 |= MF2_EVIL;
	    (void)teleport_away(i, dist);
	    dispel = TRUE;
	}
    }

    return (dispel);
}

int probing(void)
{
    register int            i;
    int                     probe;
    register monster_type  *m_ptr;
    register monster_race *r_ptr;
    register monster_lore   *l_ptr;
    vtype                   out_val;
    char		    m_name[80];

    msg_print("Probing...");
    probe = FALSE;
    for (i = m_max - 1; i >= MIN_M_IDX; i--) {
	m_ptr = &m_list[i];

	if (m_ptr->los && m_ptr->ml) {

	    r_ptr = &r_list[m_ptr->r_idx];
	    l_ptr = &l_list[m_ptr->r_idx];

	    /* Get "the monster" or "something" */
	    monster_desc(m_name, m_ptr, 0x04);
	    sprintf(out_val, "%s has %d hit points.", m_name, m_ptr->hp);
	    move_cursor_relative(m_ptr->fy, m_ptr->fx);
	    message(out_val,0x01);

	    /* Learn all of the non-spell, non-treasure flags */
	    lore_do_probe(m_ptr);

	    /* Probe worked */
	    probe = TRUE;
	}
    }

    if (probe) {
	msg_print("That's all.");
    }
    else {
	msg_print("You find nothing to probe.");
    }

    move_cursor_relative(char_row, char_col);
    return (probe);
}


/*
 * Attempts to destroy a type of creature.  Success depends on
 * the creatures level VS. the player's level		 -RAK-	 
 *
 * XXX Hack -- cflag must be a set of monster flags
 */
int dispel_creature(u32b cflag, int damage)
{
    register int	i;
    int			dispel;
    monster_type	*m_ptr;
    monster_race	*r_ptr;
    monster_lore	*l_ptr;
    vtype		out_val;
    char		m_name[80];

    dispel = FALSE;

    /* Affect all nearby monsters within line of sight */
    for (i = m_max - 1; i >= MIN_M_IDX; i--) {

	/* Get the monster */
	m_ptr = &m_list[i];

	/* Too far away */
	if (!m_ptr->los) continue;

	/* Get the race */
	r_ptr = &r_list[m_ptr->r_idx];

	/* Hurt it if possible */
	if (r_ptr->cflags2 & cflag) {

	    /* Get the lore */
	    l_ptr = &l_list[m_ptr->r_idx];

	    /* Memorize the susceptibility */
	    l_ptr->r_cflags2 |= cflag;

	    /* Get the name */
	    monster_desc(m_name, m_ptr, 0);

	    if (mon_take_hit(i, randint(damage), FALSE)) {
		(void)sprintf(out_val, "%s dissolves!", m_name);
		message(out_val,0x01);
		prt_experience();
	    }
	    else {
		(void)sprintf(out_val, "%s shudders.", m_name);
		message(out_val,0x01);
	    }

	    dispel = TRUE;
	}
    }

    return (dispel);
}


/*
 * Attempt to turn (confuse) undead creatures.	-RAK-	 
 */
int turn_undead(void)
{
    register int            i, turn_und;
    register monster_type  *m_ptr;
    register monster_race *r_ptr;
    vtype                   out_val;
    char		    m_name[80];

    turn_und = FALSE;
    for (i = m_max - 1; i >= MIN_M_IDX; i--) {

	m_ptr = &m_list[i];
	if (!m_ptr->los) continue;

	r_ptr = &r_list[m_ptr->r_idx];
	if (r_ptr->cflags2 & MF2_UNDEAD) {
	    monster_desc(m_name, m_ptr, 0);
	    if (((p_ptr->lev + 1) > r_ptr->level) ||
		(randint(5) == 1)) {
		if (m_ptr->ml) {
		    (void)sprintf(out_val, "%s runs frantically!", m_name);
		    message(out_val,0x01);
		    turn_und = TRUE;
		    l_list[m_ptr->r_idx].r_cflags2 |= MF2_UNDEAD;
		}
		m_ptr->monfear = randint(p_ptr->lev) * 2;
	    }
	    else if (m_ptr->ml) {
		(void)sprintf(out_val, "%s is unaffected.", m_name);
		message(out_val,0x01);
	    }
	}
    }

    return (turn_und);
}


/*
 * Leave a glyph of warding. Creatures will not pass over it. -RAK- 
 * 
 */
void warding_glyph(void)
{
    register int        i;
    register cave_type *c_ptr;

    c_ptr = &cave[char_row][char_col];
    if (c_ptr->i_idx == 0) {
	i = i_pop();
	invcopy(&i_list[i], OBJ_GLYPH);
	i_list[i].iy = char_row;
	i_list[i].ix = char_col;
	c_ptr->i_idx = i;
    }
}




/*
 * Lose experience					-RAK-	 
 */
void lose_exp(s32b amount)
{
    register int          i;

    if (amount > p_ptr->exp) {
	p_ptr->exp = 0;
    }
    else {
	p_ptr->exp -= amount;
    }

    prt_experience();

    i = 0;
    while (((player_exp[i] * p_ptr->expfact / 100L) <= p_ptr->exp)
	   && (i < MAX_PLAYER_LEVEL)) {
	i++;
    }

    /* increment i once more, because level 1 exp is stored in player_exp[0] */
    i++;

    if (i > MAX_PLAYER_LEVEL) i = MAX_PLAYER_LEVEL;

    if (p_ptr->lev != i) {

	p_ptr->lev = i;

	calc_hitpoints();

	if (class[p_ptr->pclass].spell == MAGE) {
	    calc_spells(A_INT);
	    calc_mana(A_INT);
	}
	else if (class[p_ptr->pclass].spell == PRIEST) {
	    calc_spells(A_WIS);
	    calc_mana(A_WIS);
	}

	prt_level();
	prt_title();
    }
}


/*
 * Slow Poison						-RAK-	 
 */
int slow_poison()
{
    register int           slow;

    slow = FALSE;

    if (p_ptr->poisoned > 0) {
	p_ptr->poisoned = p_ptr->poisoned / 2;
	if (p_ptr->poisoned < 1) p_ptr->poisoned = 1;
	slow = TRUE;
	msg_print("The effect of the poison has been reduced.");
    }
    return (slow);
}


/*
 * Bless						-RAK-	 
 */
void bless(int amount)
{
    p_ptr->blessed += amount;
}


/*
 * Detect Invisible for period of time			-RAK-	 
 */
void detect_inv2(int amount)
{
    p_ptr->detect_inv += amount;
}


/*
 * Routine used by "earthquakes"
 * Those routines will update the view, etc.
 */
static void replace_spot(int y, int x, int typ)
{
    register cave_type *c_ptr;

    c_ptr = &cave[y][x];
    switch (typ) {
      case 1:
      case 2:
      case 3:
	c_ptr->fval = CORR_FLOOR;
	break;
      case 4:
      case 7:
      case 10:
	c_ptr->fval = GRANITE_WALL;
	break;
      case 5:
      case 8:
      case 11:
	c_ptr->fval = MAGMA_WALL;
	break;
      case 6:
      case 9:
      case 12:
	c_ptr->fval = QUARTZ_WALL;
	break;
    }

    /* No longer part of a room */
    c_ptr->info &= ~CAVE_LR;
    c_ptr->info &= ~CAVE_FM;
    c_ptr->info &= ~CAVE_PL;

    /* Delete the object, if any */
    delete_object(y, x);

    /* Delete the monster (if any) */
    delete_monster(y, x);
}


/*
 * The spell of destruction.				-RAK-
 * NOTE : Winning creatures that are deleted will be considered
 * as teleporting to another level.  This will NOT win the game.
 */
void destroy_area(int y, int x)
{
    register int i, j, k;

    msg_print("There is a searing blast of light!");

    if (!p_ptr->resist_blind && !p_ptr->resist_lite) {
	p_ptr->blind += 10 + randint(10);
    }

    /* No destroying the town */
    if (dun_level) {
	for (i = (y - 15); i <= (y + 15); i++) {
	    for (j = (x - 15); j <= (x + 15); j++) {
		if (valid_grid(i, j)) {
		    k = distance(i, j, y, x);
		    /* clear player's spot... from um55 -CFT */
		    if (k == 0) replace_spot(i, j, 1);
		    else if (k < 13) replace_spot(i, j, randint(6));
		    else if (k < 16) replace_spot(i, j, randint(9));
		}
	    }
	}
    }

    /* Hack -- update the view */
    update_view();
    update_lite();
    update_monsters();

    /* Hack -- redraw the cave */
    draw_cave();
}


/*
 * Revamped!  Now takes item pointer, number of times to try enchanting,
 * and a flag of what to try enchanting.  Artifacts resist enchantment
 * some of the time, and successful enchantment to at least +0 might
 * break a curse on the item.  -CFT
 *
 * Enchants a plus onto an item.                        -RAK-   
 */
int enchant(inven_type *i_ptr, int n, int eflag)
{
    register int chance, res = FALSE, i;
    int table[13] = {  10,  50, 100, 200, 300, 400,
			   500, 700, 950, 990, 992, 995, 997 };

    /* Artifacts resist enchantment */
    int a = artifact_p(i_ptr);

    /* Try "n" times */
    for (i=0; i<n; i++) {

	chance = 0;

	if (eflag & ENCH_TOHIT) {

	    if (i_ptr->tohit < 1) chance = 0;
	    else if (i_ptr->tohit > 13) chance = 1000;
	    else chance = table[i_ptr->tohit-1];

	    if ((randint(1000)>chance) && (!a || randint(7)>3)) {

		i_ptr->tohit++;
		res = TRUE;

		/* only when you get it above -1 -CFT */
		if (cursed_p(i_ptr) &&
		    (i_ptr->tohit >= 0) && (randint(4)==1)) {
		    msg_print("The curse is broken! ");
		    i_ptr->flags3 &= ~TR3_CURSED;
		    i_ptr->flags3 &= ~TR3_HEAVY_CURSE;
		    i_ptr->ident &= ~ID_FELT;
		    inscribe(i_ptr, "uncursed");
		}
	    }
	}

	if (eflag & ENCH_TODAM) {

	    if (i_ptr->todam < 1) chance = 0;
	    else if (i_ptr->todam > 13) chance = 1000;
	    else chance = table[i_ptr->todam-1];

	    if ((randint(1000)>chance) && (!a || randint(7)>3)) {

		i_ptr->todam++;
		res = TRUE;

		/* only when you get it above -1 -CFT */
		if (cursed_p(i_ptr) &&
		    (i_ptr->todam >= 0) && (randint(4)==1)) {
		    msg_print("The curse is broken! ");
		    i_ptr->flags3 &= ~TR3_CURSED;
		    i_ptr->flags3 &= ~TR3_HEAVY_CURSE;
		    i_ptr->ident &= ~ID_FELT;
		    inscribe(i_ptr, "uncursed");
		}
	    }
	}

	if (eflag & ENCH_TOAC) {

	    if (i_ptr->toac < 1) chance = 0;
	    else if (i_ptr->toac > 13) chance = 1000;
	    else chance = table[i_ptr->toac-1];

	    if ((randint(1000)>chance) && (!a || randint(7)>3)) {

		i_ptr->toac++;
		res = TRUE;

		/* only when you get it above -1 -CFT */
		if (cursed_p(i_ptr) &&
		    (i_ptr->toac >= 0) && (randint(4)==1)) {
		    msg_print("The curse is broken! ");
		    i_ptr->flags3 &= ~TR3_CURSED;
		    i_ptr->flags3 &= ~TR3_HEAVY_CURSE;
		    i_ptr->ident &= ~ID_FELT;
		    inscribe(i_ptr, "uncursed");
		}
	    }
	}
    }

    if (res) calc_bonuses();

    return (res);
}


/*
 * Hack -- generate a "formattable" pain message
 * Should convert this to take a monster name
 */
cptr pain_message(int m_idx, int dam)
{
    register monster_type	*m_ptr;
    monster_race		*r_ptr;
    cptr			name;
    long			oldhp, newhp, tmp;
    int				percentage;

    /* avoid potential div by 0 */
    if (dam == 0) {
	return "%s is unharmed.";
    }

    m_ptr = &m_list[m_idx];
    r_ptr = &r_list[m_ptr->r_idx];

    name = r_ptr->name;

    /* subtle fix -CFT */
    newhp = (long)(m_ptr->hp);
    oldhp = newhp + (long)(dam);
    tmp = (newhp * 100L) / oldhp;
    percentage = (int)(tmp);

    /* Non-verbal creatures like molds */
    if (strchr("jmvQ", r_ptr->r_char) ||
	((r_ptr->r_char == 'e') && stricmp(name, "Beholder"))) {

	if (percentage > 95)
	    return "%s barely notices.";
	if (percentage > 75)
	    return "%s flinches.";
	if (percentage > 50)
	    return "%s squelches.";
	if (percentage > 35)
	    return "%s quivers in pain.";
	if (percentage > 20)
	    return "%s writhes about.";
	if (percentage > 10)
	    return "%s writhes in agony.";
	return "%s jerks limply.";
    }

    /* Dogs and Hounds */
    else if (strchr("CZ", r_ptr->r_char)) {

	if (percentage > 95)
	    return "%s shrugs off the attack.";
	if (percentage > 75)
	    return "%s snarls with pain.";
	if (percentage > 50)
	    return "%s yelps in pain.";
	if (percentage > 35)
	    return "%s howls in pain.";
	if (percentage > 20)
	    return "%s howls in agony.";
	if (percentage > 10)
	    return "%s writhes in agony.";
	return "%s yelps feebly.";
    }

    /* One type of monsters (ignore,squeal,shriek) */
    else if (strchr("KcaUqRXbFJlrsSt", r_ptr->r_char)) {

	if (percentage > 95)
	    return "%s ignores the attack.";
	if (percentage > 75)
	    return "%s grunts with pain.";
	if (percentage > 50)
	    return "%s squeals in pain.";
	if (percentage > 35)
	    return "%s shrieks in pain.";
	if (percentage > 20)
	    return "%s shrieks in agony.";
	if (percentage > 10)
	    return "%s writhes in agony.";
	return "%s cries out feebly.";
    }

    /* Another type of monsters (shrug,cry,scream) */
    else {

	if (percentage > 95)
	    return "%s shrugs off the attack.";
	if (percentage > 75)
	    return "%s grunts with pain.";
	if (percentage > 50)
	    return "%s cries out in pain.";
	if (percentage > 35)
	    return "%s screams in pain.";
	if (percentage > 20)
	    return "%s screams in agony.";
	if (percentage > 10)
	    return "%s writhes in agony.";
	return "%s cries out feebly.";
    }
}



/*
 * Removes curses from items in inventory
 *
 * Note that Items which are "Perma-Cursed" (The One Ring,
 * The Crown of Morgoth) can NEVER be uncursed.
 *
 * Note that if "all" is FALSE, then Items which are
 * "Heavy-Cursed" (Mormegil, Calris, and Weapons of Morgul)
 * will not be uncursed.
 */
static int remove_curse_aux(int all)
{
    register inven_type		*i_ptr;
    register int		i, cnt;

    /* Attempt to uncurse items being worn */
    for (cnt = 0, i = INVEN_WIELD; i < INVEN_TOTAL; i++) {

	i_ptr = &inventory[i];

	/* Not wearable */
	if (!wearable_p(i_ptr)) continue;

	/* Uncursed already */
	if (!cursed_p(i_ptr)) continue;

	/* Heavily Cursed Items need a special spell */
	if (!all && (i_ptr->flags3 & TR3_HEAVY_CURSE)) continue;

	/* Perma-Cursed Items can NEVER be uncursed */
	if (i_ptr->flags3 & TR3_PERMA_CURSE) continue;

	/* Uncurse it, and take note */
	i_ptr->flags3 &= ~TR3_CURSED;
	i_ptr->flags3 &= ~TR3_HEAVY_CURSE;

	/* Hack -- assume felt */
	i_ptr->ident |= ID_FELT;

	/* Take note */
	inscribe(i_ptr, "uncursed");

	/* Count the uncursings */
	cnt++;
    }

    /* Recalculate the bonuses if needed */
    if (cnt) calc_bonuses();

    /* Return "something uncursed" */
    return (cnt);
}


/*
 * Remove most curses
 */
int remove_curse()
{
    return (remove_curse_aux(FALSE));
}

/*
 * Remove all curses
 */
int remove_all_curse()
{
    return (remove_curse_aux(TRUE));
}



/*
 * Restores any drained experience			-RAK-	 
 */
int restore_level()
{
    register int          restore;

    restore = FALSE;

    if (p_ptr->max_exp > p_ptr->exp) {
	restore = TRUE;
	msg_print("You feel your life energies returning.");
    /* this while loop is not redundant, ptr_exp may reduce the exp level */
	while (p_ptr->exp < p_ptr->max_exp) {
	    p_ptr->exp = p_ptr->max_exp;
	    prt_experience();
	}
    }
    return (restore);
}


/*
 * self-knowledge... idea from nethack.  Useful for determining powers and
 * resistences of items.  It saves the screen, clears it, then starts listing
 * attributes, a screenful at a time.  (There are a LOT of attributes to
 * list.  It will probably take 2 or 3 screens for a powerful character whose
 * using several artifacts...) -CFT 
 *
 * It is now a lot more efficient. -BEN-
 */
void self_knowledge()
{
    int    i = 0, j, k;
    u32b f1 = 0L, f2 = 0L, f3 = 0L;
    inven_type *i_ptr;
    cptr info[128];


    /* Acquire item flags (from worn items) */
    for (k = INVEN_WIELD; k < INVEN_TOTAL; k++) {
	i_ptr = &inventory[k];

	/* Only examine real items */        
	if (i_ptr->tval) {

	    /* Certain fields depend on a positive "pval" */
	    if (i_ptr->pval > 0) {
		f1 |= i_ptr->flags1;
	    }
	    else {
		/* Mask out the "inactive" flags */
		f1 |= (i_ptr->flags1 & ~TR1_PVAL_MASK);
	    }
	    f2 |= i_ptr->flags2;
	    f3 |= i_ptr->flags3;
	}
    }


    if (p_ptr->blind > 0) {
	info[i++] = "You cannot see.";
    }
    if (p_ptr->confused > 0) {
	info[i++] = "You are confused.";
    }
    if (p_ptr->afraid > 0) {
	info[i++] = "You are terrified.";
    }
    if (p_ptr->cut > 0) {
	info[i++] = "You are bleeding.";
    }
    if (p_ptr->stun > 0) {
	info[i++] = "You are stunned and reeling.";
    }
    if (p_ptr->poisoned > 0) {
	info[i++] = "You are poisoned.";
    }
    if (p_ptr->image > 0) {
	info[i++] = "You are hallucinating.";
    }
    if (p_ptr->aggravate) {
	info[i++] = "You aggravate monsters.";
    }
    if (p_ptr->teleport) {
	info[i++] = "Your position is very uncertain.";
    }
    if (p_ptr->blessed > 0) {
	info[i++] = "You feel rightous.";
    }
    if (p_ptr->hero > 0) {
	info[i++] = "You feel heroic.";
    }
    if (p_ptr->shero > 0) {
	info[i++] = "You are in a battle rage.";
    }
    if (p_ptr->protevil > 0) {
	info[i++] = "You are protected from evil.";
    }
    if (p_ptr->shield > 0) {
	info[i++] = "You are protected by a mystic shield.";
    }
    if (p_ptr->invuln > 0) {
	info[i++] = "You are temporarily invulnerable.";
    }
    if (p_ptr->confusing) {
	info[i++] = "Your hands are glowing dull red.";
    }
    if (p_ptr->searching) {
	info[i++] = "You are looking around very carefully.";
    }
    if (p_ptr->new_spells > 0) {
	info[i++] = "You can learn some more spells.";
    }
    if (p_ptr->word_recall > 0) {
	info[i++] = "You will soon be recalled.";
    }
    if ((p_ptr->see_infra) || (p_ptr->tim_infra)) {
	info[i++] = "Your eyes are sensitive to infrared light.";
    }
    if ((p_ptr->see_inv) || (p_ptr->detect_inv)) {
	info[i++] = "You can see invisible creatures.";
    }
    if (p_ptr->ffall) {
	info[i++] = "You land gently.";
    }
    if (p_ptr->free_act) {
	info[i++] = "You have free action.";
    }
    if (p_ptr->regenerate) {
	info[i++] = "You regenerate quickly.";
    }
    if (p_ptr->slow_digest) {
	info[i++] = "Your appetite is small.";
    }
    if (p_ptr->telepathy) {
	info[i++] = "You have ESP.";
    }
    if (p_ptr->hold_life) {
	info[i++] = "You have a firm hold on your life force.";
    }
    if (p_ptr->lite) {
	info[i++] = "You are carrying a permanent light.";
    }

    if (p_ptr->immune_acid) {
	info[i++] = "You are completely immune to acid.";
    }
    else if ((p_ptr->resist_acid) && (p_ptr->oppose_acid)) {
	info[i++] = "You resist acid exceptionally well.";
    }
    else if ((p_ptr->resist_acid) || (p_ptr->oppose_acid)) {
	info[i++] = "You are resistant to acid.";
    }

    if (p_ptr->immune_elec) {
	info[i++] = "You are completely immune to lightning.";
    }
    else if ((p_ptr->resist_elec) && (p_ptr->oppose_elec)) {
	info[i++] = "You resist lightning exceptionally well.";
    }
    else if ((p_ptr->resist_elec) || (p_ptr->oppose_elec)) {
	info[i++] = "You are resistant to lightning.";
    }

    if (p_ptr->immune_fire) {
	info[i++] = "You are completely immune to fire.";
    }
    else if ((p_ptr->resist_fire) && (p_ptr->oppose_fire)) {
	info[i++] = "You resist fire exceptionally well.";
    }
    else if ((p_ptr->resist_fire) || (p_ptr->oppose_fire)) {
	info[i++] = "You are resistant to fire.";
    }

    if (p_ptr->immune_cold) {
	info[i++] = "You are completely immune to cold.";
    }
    else if ((p_ptr->resist_cold) && (p_ptr->oppose_cold)) {
	info[i++] = "You resist cold exceptionally well.";
    }
    else if ((p_ptr->resist_cold) || (p_ptr->oppose_cold)) {
	info[i++] = "You are resistant to cold.";
    }

    if (p_ptr->immune_pois) {
	info[i++] = "You are completely immune to poison.";
    }
    else if ((p_ptr->resist_pois) && (p_ptr->oppose_pois)) {
	info[i++] = "You resist poison exceptionally well.";
    }
    else if ((p_ptr->resist_pois) || (p_ptr->oppose_pois)) {
	info[i++] = "You are resistant to poison.";
    }

    if (p_ptr->resist_lite) {
	info[i++] = "You are resistant to bright light.";
    }
    if (p_ptr->resist_dark) {
	info[i++] = "You are resistant to darkness.";
    }
    if (p_ptr->resist_conf) {
	info[i++] = "You are resistant to confusion.";
    }
    if (p_ptr->resist_sound) {
	info[i++] = "You are resistant to sonic attacks.";
    }
    if (p_ptr->resist_disen) {
	info[i++] = "You are resistant to disenchantment.";
    }
    if (p_ptr->resist_chaos) {
	info[i++] = "You are resistant to chaos.";
    }
    if (p_ptr->resist_shards) {
	info[i++] = "You are resistant to blasts of shards.";
    }
    if (p_ptr->resist_nexus) {
	info[i++] = "You are resistant to nexus attacks.";
    }
    if (p_ptr->resist_nether) {
	info[i++] = "You are resistant to nether forces.";
    }
    if (p_ptr->resist_fear) {
	info[i++] = "You are completely fearless.";
    }
    if (p_ptr->resist_blind) {
	info[i++] = "Your eyes are resistant to blindness.";
    }

    if (p_ptr->sustain_str) {
	info[i++] = "You will not become weaker.";
    }
    if (p_ptr->sustain_int) {
	info[i++] = "You will not become dumber.";
    }
    if (p_ptr->sustain_wis) {
	info[i++] = "You will not become less wise.";
    }
    if (p_ptr->sustain_con) {
	info[i++] = "You will not become out of shape.";
    }
    if (p_ptr->sustain_dex) {
	info[i++] = "You will not become clumsy.";
    }
    if (p_ptr->sustain_chr) {
	info[i++] = "You will not become less popular.";
    }


    if (f1 & TR1_STR) {
	info[i++] = "You are magically strong.";
    }
    if (f1 & TR1_INT) {
	info[i++] = "You are magically intelligent.";
    }
    if (f1 & TR1_WIS) {
	info[i++] = "You are magically wise.";
    }
    if (f1 & TR1_DEX) {
	info[i++] = "You are magically agile.";
    }
    if (f1 & TR1_CON) {
	info[i++] = "You are magically tough.";
    }
    if (f1 & TR1_CHR) {
	info[i++] = "You are magically popular.";
    }

    if (f1 & TR1_STEALTH) {
	info[i++] = "You are magically stealthy.";
    }
    if (f1 & TR1_SEARCH) {
	info[i++] = "You are magically perceptive.";
    }
    if (f1 & TR1_ATTACK_SPD) {
	info[i++] = "You can strike at your foes with uncommon speed.";
    }


    /* Access the current weapon */
    i_ptr = &inventory[INVEN_WIELD];

    /* Analyze the weapon */
    if (i_ptr->tval != TV_NOTHING) {

	/* Be sure not to check certain fields unless they matter */
	if (i_ptr->pval > 0) {
	    f1 |= i_ptr->flags1;
	}
	else {
	    /* Certain flags depend on a positive "pval" */
	    f1 |= (i_ptr->flags1 & ~TR1_PVAL_MASK);
	}
	f2 |= i_ptr->flags2;
	f3 |= i_ptr->flags3;


	/* Indicate various curses */
	if (f3 & TR3_HEAVY_CURSE) {
	    info[i++] = "Your weapon is truly foul.";
	}
	else if (f3 & TR3_CURSED) {
	    info[i++] = "Your weapon is accursed.";
	}

	/* Indicate Blessing */
	if (f3 & TR3_BLESSED) {
	    info[i++] = "Your weapon has been blessed by the gods.";
	}

	/* Special "Attack Bonuses" */
	if (f1 & TR1_TUNNEL) {
	    info[i++] = "Your weapon is an effective digging tool.";
	}
	if (f1 & TR1_ATTACK_SPD) {
	    info[i++] = "Your weapon strikes with uncommon speed.";
	}
	if (f1 & TR1_BRAND_COLD) {
	    info[i++] = "Your frigid weapon freezes your foes.";
	}
	if (f1 & TR1_BRAND_FIRE) {
	    info[i++] = "Your flaming weapon burns your foes.";
	}
	if (f1 & TR1_BRAND_ELEC) {
	    info[i++] = "Your weapon electrocutes your foes.";
	}
	if (f1 & TR1_IMPACT) {
	    info[i++] = "The unbelievable impact of your weapon can cause earthquakes.";
	}

	if (f1 & TR1_KILL_DRAGON) {
	    info[i++] = "Your weapon is a great bane of dragons.";
	}
	else if (f1 & TR1_SLAY_DRAGON) {
	    info[i++] = "Your weapon is especially deadly against dragons.";
	}
	if (f1 & TR1_SLAY_ORC) {
	    info[i++] = "Your weapon is especially deadly against orcs.";
	}
	if (f1 & TR1_SLAY_TROLL) {
	    info[i++] = "Your weapon is especially deadly against trolls.";
	}
	if (f1 & TR1_SLAY_GIANT) {
	    info[i++] = "Your weapon is especially deadly against giants.";
	}
	if (f1 & TR1_SLAY_ANIMAL) {
	    info[i++] = "Your weapon is especially deadly against natural creatures.";
	}
	if (f1 & TR1_SLAY_DEMON) {
	    info[i++] = "Your weapon strikes at demons with holy wrath.";
	}
	if (f1 & TR1_SLAY_UNDEAD) {
	    info[i++] = "Your weapon strikes at undead with holy wrath.";
	}
	if (f1 & TR1_SLAY_EVIL) {
	    info[i++] = "Your weapon fights against evil with holy fury.";
	}
    }


    /* Save the screen */
    save_screen();

    /* Erase the screen */
    for (k = 1; k < 24; k++) erase_line(k, 13);

    /* Label the information */
    prt("     Your Attributes:", 1, 15);

    /* We will print on top of the map (column 13) */
    for (k = 2, j = 0; j < i; j++) {

	/* Show the info */
	prt(info[j], k++, 15);

	/* Every 20 entries (lines 2 to 21), start over */
	if ((k == 22) && (j+1 < i)) {
	    prt("-- more --", k, 15);
	    inkey();
	    for ( ; k > 2; k--) erase_line(k, 15);
	}
    }

    /* Pause */
    prt("[Press any key to continue]", k, 13);
    inkey();

    /* Restore the screen */
    restore_screen();
}





/*
 * Teleport the player one level up or down (random when legal)
 */
void tele_level()
{
    if (!dun_level) {
	dun_level++;
	msg_print("You sink through the floor.");
    }
    else if (is_quest(dun_level)) {
	dun_level--;
	msg_print("You rise up through the ceiling.");
    }
    else if (randint(2) == 1) {
	dun_level--;
	msg_print("You rise up through the ceiling.");
    }
    else {
	dun_level++;
	msg_print("You sink through the floor.");
    }

    /* New level */
    new_level_flag = TRUE;
}



/*
 * Sleep creatures adjacent to player			-RAK-	 
 * Could be done as a "radius one" ball attack via "project()".
 */
int sleep_monsters1(int y, int x)
{
    register int            i, j;
    register cave_type     *c_ptr;
    register monster_type  *m_ptr;
    register monster_race *r_ptr;
    int                     sleep;
    vtype                   out_val, m_name;

    sleep = FALSE;
    for (i = y - 1; i <= y + 1; i++)
	for (j = x - 1; j <= x + 1; j++) {
	    c_ptr = &cave[i][j];
	    if (c_ptr->m_idx > 1) {

		m_ptr = &m_list[c_ptr->m_idx];
		r_ptr = &r_list[m_ptr->r_idx];
		monster_desc(m_name, m_ptr, 0);

		if ((r_ptr->level >
		     randint((p_ptr->lev - 10) < 1 ? 1 : (p_ptr->lev - 10)) + 10) ||
		    (MF2_CHARM_SLEEP & r_ptr->cflags2) || (r_ptr->cflags2 & MF2_UNIQUE)) {
		    if (m_ptr->ml && (r_ptr->cflags2 & MF2_CHARM_SLEEP))
			l_list[m_ptr->r_idx].r_cflags2 |= MF2_CHARM_SLEEP;
		    (void)sprintf(out_val, "%s is unaffected.", m_name);
		}
		else {
		    sleep = TRUE;
		    m_ptr->csleep = 500;
		    (void)sprintf(out_val, "%s falls asleep.", m_name);
		}

		message(out_val, 0x01);
	    }
	}
    return (sleep);
}



/*
 * Forget everything
 */
int lose_all_info(void)
{
    int                 i;
    inven_type          *i_ptr;

    /* Forget info about objects */
    for (i = 0; i < INVEN_TOTAL; i++) {

	i_ptr = &inventory[i];

	/* Skip non-items */
	if (!i_ptr->tval) continue;

	/* Now forget about the item */
	if (known2_p(i_ptr)) {

	    /* Clear the "known" flag */
	    i_ptr->ident &= ~ID_KNOWN;

	    /* XXX Hack -- remember "cursed" items */
	    if (cursed_p(i_ptr)) {
		i_ptr->ident |= ID_FELT;
		inscribe(i_ptr, "cursed");
	    }
	}
    }

    /* Forget the map */
    wiz_dark();

    return (0);
}

/*
 * Detect any treasure on the current panel		-RAK-	 
 */
int detect_treasure(void)
{
    register int        i, j, detect;
    register cave_type *c_ptr;

    detect = FALSE;
    for (i = panel_row_min; i <= panel_row_max; i++) {
	for (j = panel_col_min; j <= panel_col_max; j++) {
	    c_ptr = &cave[i][j];
	    if ((c_ptr->i_idx != 0) &&
		(i_list[c_ptr->i_idx].tval == TV_GOLD) &&
		!test_lite(i, j)) {

		/* Hack -- memorize the item */
		c_ptr->info |= CAVE_FM;

		/* Redraw */
		lite_spot(i, j);
		detect = TRUE;
	    }
	}
    }

    return (detect);
}



/*
 * Detect magic items.
 *
 * This will light up all spaces with "magic" items, including potions, scrolls,
 * books, rods, wands, staves, amulets, rings, artifacts, and "enchanted" items.
 *
 * It can probably be argued that this function is now too powerful.
 */
int detect_magic()
{
    register int i, j, detect, tv;
    register cave_type *c_ptr;
    register inven_type *i_ptr;

    detect = FALSE;
    for (i = panel_row_min; i <= panel_row_max; i++) {
	for (j = panel_col_min; j <= panel_col_max; j++) {
	    c_ptr = &cave[i][j];

	    /* Nothing there */
	    if (c_ptr->i_idx == 0) continue;

	    /* Already visible */
	    if (test_lite(i, j)) continue;

	    /* Get the item */
	    i_ptr = &i_list[c_ptr->i_idx];

	    /* Examine the tval */            
	    tv = i_ptr->tval;

	    /* artifacts */
	    /* misc items (including amulets/rings) */
	    /* magic books (both kinds) */
	    /* wearables which have a positive modifier */
	    if (artifact_p(i_ptr) ||
		((tv >= TV_AMULET) && (tv < TV_FLASK)) ||
		((tv == TV_MAGIC_BOOK) || (tv == TV_PRAYER_BOOK)) ||
		(wearable_p(i_ptr) &&
		 ((i_ptr->toac>0) || (i_ptr->tohit>0) || (i_ptr->todam>0)))) {

		/* Hack -- memorize the item */
		c_ptr->info |= CAVE_FM;

		/* Redraw */
		lite_spot(i, j);
		detect = TRUE;
	    }
	}
    }

    /* Return result */    
    return (detect);
}


/*
 * Detect everything
 */
int detection(void)
{
    register int           i, detect;
    register monster_type *m_ptr;

    /* Detect the easy things */
    detect_treasure();
    detect_object();
    detect_trap();
    detect_sdoor();

    /* Illuminate all monsters in the current panel */
    detect = FALSE;
    for (i = MIN_M_IDX; i < m_max; i++) {
	m_ptr = &m_list[i];
	if (panel_contains((int)m_ptr->fy, (int)m_ptr->fx)) {

	    /* Draw the monster (even if invisible) */
	    m_ptr->ml = TRUE;
	    lite_monster(m_ptr);

	    detect = TRUE;
	}
    }

    /* Describe the result, then fix the monsters */
    if (detect) {

	msg_print("You sense the presence of monsters!");
	msg_print(NULL);

	/* Fix the monsters */
	update_monsters();
    }

    /* XXX Only returns true if monsters were detected */
    return (detect);
}

/*
 * Detect all objects on the current panel		-RAK-	 
 */
int detect_object(void)
{
    register int        i, j, detect;
    register cave_type *c_ptr;

    detect = FALSE;
    for (i = panel_row_min; i <= panel_row_max; i++) {
	for (j = panel_col_min; j <= panel_col_max; j++) {

	    c_ptr = &cave[i][j];
	    if (c_ptr->i_idx == 0) continue;
	    if (test_lite(i,j)) continue;
	    if (i_list[c_ptr->i_idx].tval >= TV_MAX_OBJECT) continue;

	    /* Hack -- memorize it */
	    c_ptr->info |= CAVE_FM;

	    /* Redraw */
	    lite_spot(i, j);
	    detect = TRUE;
	}
    }

    return (detect);
}


/*
 * Locates and displays traps on current panel		-RAK-	 
 */
int detect_trap(void)
{
    register int         i, j;
    int                  detect;
    register cave_type  *c_ptr;
    register inven_type *i_ptr;

    detect = FALSE;
    for (i = panel_row_min; i <= panel_row_max; i++) {
	for (j = panel_col_min; j <= panel_col_max; j++) {
	    c_ptr = &cave[i][j];
	    if (c_ptr->i_idx == 0) continue;
	    i_ptr = &i_list[c_ptr->i_idx];

	    /* Notice traps */
	    if (i_ptr->tval == TV_INVIS_TRAP) {

		/* Change it */
		i_ptr->tval = TV_VIS_TRAP;

		/* Hack -- memorize it */
		c_ptr->info |= CAVE_FM;

		/* Redraw */
		lite_spot(i, j);
		detect = TRUE;
	    }

	    /* Identify chests */
	    else if (i_ptr->tval == TV_CHEST) {
		known2(i_ptr);
	    }
	}
    }

    return (detect);
}



/*
 * Create stairs
 * Assume the player grid is never blocked.
 */
void stair_creation()
{
    register cave_type *c_ptr;
    register int        cur_pos;

    c_ptr = &cave[char_row][char_col];

    /* Do not destroy useful stuff */
    if (valid_grid(char_row, char_col)) {

	delete_object(char_row, char_col);

	cur_pos = i_pop();
	if (!dun_level) {
	    invcopy(&i_list[cur_pos], OBJ_DOWN_STAIR);
	}
	else if (is_quest(dun_level)) {
	    invcopy(&i_list[cur_pos], OBJ_UP_STAIR);
	}
	else if (randint(2) == 1) {
	    invcopy(&i_list[cur_pos], OBJ_UP_STAIR);
	}
	else {
	    invcopy(&i_list[cur_pos], OBJ_DOWN_STAIR);
	}
	i_list[cur_pos].iy = char_row;
	i_list[cur_pos].ix = char_col;
	c_ptr->i_idx = cur_pos;
    }
    else {
	msg_print("The object resists the spell.");
    }
}


/*
 * Locates and displays all stairs and secret doors on current panel -RAK-	
 */
int detect_sdoor()
{
    register int        i, j, detect;
    register cave_type *c_ptr;
    register inven_type *i_ptr;

    detect = FALSE;
    for (i = panel_row_min; i <= panel_row_max; i++) {
	for (j = panel_col_min; j <= panel_col_max; j++) {
	    c_ptr = &cave[i][j];
	    if (c_ptr->i_idx == 0) continue;
	    i_ptr = &i_list[c_ptr->i_idx];

	    /* Secret doors  */
	    if (i_ptr->tval == TV_SECRET_DOOR) {

		/* Hack -- make a closed door */
		invcopy(i_ptr, OBJ_CLOSED_DOOR);

		/* Place it in the dungeon */
		i_ptr->iy = i;
		i_ptr->ix = j;

		/* Hack -- memorize it */
		c_ptr->info |= CAVE_FM;

		/* Redraw */
		lite_spot(i, j);
		detect = TRUE;
	    }

	    /* Staircases */
	    else if (((i_ptr->tval == TV_UP_STAIR) ||
		      (i_ptr->tval == TV_DOWN_STAIR)) &&
		     !(c_ptr->info & CAVE_FM)) {

		/* Hack -- memorize it */
		c_ptr->info |= CAVE_FM;

		/* Redraw */
		lite_spot(i, j);
		detect = TRUE;
	    }
	}
    }

    return (detect);
}


/*
 * Locates and displays all invisible creatures on current panel -RAK-
 */
int detect_invisible()
{
    register int           i, flag;
    register monster_type *m_ptr;
    register monster_race *r_ptr;
    register monster_lore *l_ptr;

    flag = FALSE;
    for (i = m_max - 1; i >= MIN_M_IDX; i--) {
	m_ptr = &m_list[i];
	r_ptr = &r_list[m_ptr->r_idx];
	l_ptr = &l_list[m_ptr->r_idx];
	if (panel_contains(m_ptr->fy, m_ptr->fx) &&
	    (r_ptr->cflags1 & MF1_MV_INVIS)) {

	    /* Draw the monster (even if invisible) */
	    m_ptr->ml = TRUE;
	    lite_monster(m_ptr);

	    /* Take note that they are invisible */
	    l_ptr->r_cflags1 |= MF1_MV_INVIS;

	    /* Something was detected */
	    flag = TRUE;
	}
    }

    if (flag) {

	msg_print("You sense the presence of invisible creatures!");
	msg_print(NULL);

	/* Fix the monsters */
	update_monsters();
    }

    return (flag);
}






/*
 * Allow identification of an object below the player
 */
bool ident_floor(void)
{
    register inven_type *i_ptr;
    cave_type		*c_ptr;
    cptr prt = "You are standing on something.  Identify it?";
    bigvtype            out_val, tmp_str;

    /* Hack -- allow "on-floor" identifications */
    c_ptr = &cave[char_row][char_col];

    /* Nothing there */
    if (c_ptr->i_idx == 0) return (FALSE);

    /* Get the item */
    i_ptr = &i_list[c_ptr->i_idx];

    /* Be sure the item is "legal" */
    if (i_ptr->tval == TV_NOTHING) return (FALSE);

    /* Be sure the item can be picked up */
    if (i_ptr->tval >= TV_MAX_PICK_UP) return (FALSE);

    /* Already identified */
    if (known2_p(i_ptr)) return (FALSE);
    
    /* See if the user wants to identify it */
    if (!get_check(prt)) return (FALSE);

    /* Identify it fully */
    inven_aware(i_ptr);
    known2(i_ptr);

    /* Describe it */
    objdes(tmp_str, i_ptr, TRUE);
    (void) sprintf(out_val, "On the ground: %s", tmp_str);
    message(out_val, 0);

    /* Success */
    return (TRUE);
}


/*
 * Identify an object in the inventory	-RAK-	 
 * This routine no longer automatically combines objects
 * This routine now returns the index of the identified object
 * Use "ident_floor()" to allow identification of items on floor,
 * as in "used_up = (ident_floor() || (ident_spell() >= 0));"
 * Use "combine()" as in "combine(ident_spell())" to do combining.
 */
int ident_spell()
{
    int			item_val;
    register inven_type *i_ptr;
    bigvtype            out_val, tmp_str;

    cptr pmt = "Identify which item? ";

    /* Get an item to identify */
    if (!get_item(&item_val, pmt, 0, INVEN_TOTAL-1)) return (-1);

    /* Get the item */
    i_ptr = &inventory[item_val];

    /* Identify it fully */
    inven_aware(i_ptr);
    known2(i_ptr);

    /* Description */
    objdes(tmp_str, i_ptr, TRUE);
    if (item_val >= INVEN_WIELD) {
	calc_bonuses();		
	(void)sprintf(out_val, "%s: %s. ",
		      describe_use(item_val), tmp_str);
    }
    else {
	(void)sprintf(out_val, "(%c) %s. ",
		      index_to_label(item_val), tmp_str);
    }
    message(out_val, 0);

    return (item_val);
}


/*
 * Identify everything being carried.
 * Done by a potion of "self knowledge".
 */
void identify_pack()
{
    int                 i;
    inven_type         *i_ptr;

    /* Simply identify and know every item */
    for (i = 0; i < INVEN_TOTAL; i++) {
	i_ptr = &inventory[i];
	if (i_ptr->tval != TV_NOTHING) {
	    inven_aware(i_ptr);
	    known2(i_ptr);
	}
    }
}


/*
 * Fully "identify" an object in the inventory	-BEN-
 * This routine now returns the index of the identified object
 */
int identify_fully()
{
    int    i = 0, j, k;
    
    int			item_val;
    inven_type *i_ptr;
    bigvtype            out_val, tmp_str;

    cptr pmt = "Fully *identify* which item? ";
    
    cptr info[128];


    /* Get an item to identify */
    if (!get_item(&item_val, pmt, 0, INVEN_TOTAL-1)) return (-1);

    /* Get the item */
    i_ptr = &inventory[item_val];

    /* Identify it fully */
    inven_aware(i_ptr);
    known2(i_ptr);

    /* Description */
    objdes(tmp_str, i_ptr, TRUE);
    if (item_val >= INVEN_WIELD) {
	calc_bonuses();		
	(void)sprintf(out_val, "%s: %s. ",
		      describe_use(item_val), tmp_str);
    }
    else {
	(void)sprintf(out_val, "(%c) %s. ",
		      index_to_label(item_val), tmp_str);
    }
    message(out_val, 0);

    /* All done if not wearable */
    if (!wearable_p(i_ptr)) return (item_val);
    

    /* And then describe it fully */

    if (i_ptr->flags1 & TR1_STR) {
	info[i++] = "It affects your strength.";
    }
    if (i_ptr->flags1 & TR1_INT) {
	info[i++] = "It affects your intelligence.";
    }
    if (i_ptr->flags1 & TR1_WIS) {
	info[i++] = "It affects your wisdom.";
    }
    if (i_ptr->flags1 & TR1_DEX) {
	info[i++] = "It affects your dexterity.";
    }
    if (i_ptr->flags1 & TR1_CON) {
	info[i++] = "It affects your constitution.";
    }
    if (i_ptr->flags1 & TR1_CHR) {
	info[i++] = "It affects your charisma.";
    }

    if (i_ptr->flags1 & TR1_STEALTH) {
	info[i++] = "It affects your stealth.";
    }
    if (i_ptr->flags1 & TR1_SEARCH) {
	info[i++] = "It affects your searching.";
    }
    if (i_ptr->flags1 & TR1_INFRA) {
	info[i++] = "It affects your infravision.";
    }
    if (i_ptr->flags1 & TR1_ATTACK_SPD) {
	info[i++] = "It affects your attack speed.";
    }
    if (i_ptr->flags1 & TR1_SPEED) {
	info[i++] = "It affects your speed.";
    }
    if (i_ptr->flags1 & TR1_TUNNEL) {
	info[i++] = "It affects your ability to tunnel.";
    }

    if (i_ptr->flags1 & TR1_BRAND_ACID) {
	info[i++] = "It does extra damage from acid.";
    }
    if (i_ptr->flags1 & TR1_BRAND_ELEC) {
	info[i++] = "It does extra damage from electricity.";
    }
    if (i_ptr->flags1 & TR1_BRAND_FIRE) {
	info[i++] = "It does extra damage from fire.";
    }
    if (i_ptr->flags1 & TR1_BRAND_COLD) {
	info[i++] = "It does extra damage from frost.";
    }

    if (i_ptr->flags1 & TR1_IMPACT) {
	info[i++] = "It can cause earthquakes.";
    }

    if (i_ptr->flags1 & TR1_KILL_DRAGON) {
	info[i++] = "It is a great bane of dragons.";
    }
    else if (i_ptr->flags1 & TR1_SLAY_DRAGON) {
	info[i++] = "It is especially deadly against dragons.";
    }
    if (i_ptr->flags1 & TR1_SLAY_ORC) {
	info[i++] = "It is especially deadly against orcs.";
    }
    if (i_ptr->flags1 & TR1_SLAY_TROLL) {
	info[i++] = "It is especially deadly against trolls.";
    }
    if (i_ptr->flags1 & TR1_SLAY_GIANT) {
	info[i++] = "It is especially deadly against giants.";
    }
    if (i_ptr->flags1 & TR1_SLAY_DEMON) {
	info[i++] = "It strikes at demons with holy wrath.";
    }
    if (i_ptr->flags1 & TR1_SLAY_UNDEAD) {
	info[i++] = "It strikes at undead with holy wrath.";
    }
    if (i_ptr->flags1 & TR1_SLAY_EVIL) {
	info[i++] = "It fights against evil with holy fury.";
    }
    if (i_ptr->flags1 & TR1_SLAY_ANIMAL) {
	info[i++] = "It is especially deadly against natural creatures.";
    }

    if (i_ptr->flags2 & TR2_SUST_STR) {
	info[i++] = "It sustains your strength.";
    }
    if (i_ptr->flags2 & TR2_SUST_INT) {
	info[i++] = "It sustains your intelligence.";
    }
    if (i_ptr->flags2 & TR2_SUST_WIS) {
	info[i++] = "It sustains your wisdom.";
    }
    if (i_ptr->flags2 & TR2_SUST_DEX) {
	info[i++] = "It sustains your dexterity.";
    }
    if (i_ptr->flags2 & TR2_SUST_CON) {
	info[i++] = "It sustains your constitution.";
    }
    if (i_ptr->flags2 & TR2_SUST_CHR) {
	info[i++] = "It sustains your charisma.";
    }

    if (i_ptr->flags2 & TR2_IM_ACID) {
	info[i++] = "It provides immunity to acid.";
    }
    if (i_ptr->flags2 & TR2_IM_ELEC) {
	info[i++] = "It provides immunity to electricity.";
    }
    if (i_ptr->flags2 & TR2_IM_FIRE) {
	info[i++] = "It provides immunity to fire.";
    }
    if (i_ptr->flags2 & TR2_IM_COLD) {
	info[i++] = "It provides immunity to cold.";
    }
    if (i_ptr->flags2 & TR2_IM_POIS) {
	info[i++] = "It provides immunity to poison.";
    }

    if (i_ptr->flags2 & TR2_FREE_ACT) {
	info[i++] = "It provides immunity to paralysis.";
    }
    if (i_ptr->flags2 & TR2_HOLD_LIFE) {
	info[i++] = "It provides immunity to life draining.";
    }

    if (i_ptr->flags2 & TR2_RES_ACID) {
	info[i++] = "It provides resistance to acid.";
    }
    if (i_ptr->flags2 & TR2_RES_ELEC) {
	info[i++] = "It provides resistance to electricity.";
    }
    if (i_ptr->flags2 & TR2_RES_FIRE) {
	info[i++] = "It provides resistance to fire.";
    }
    if (i_ptr->flags2 & TR2_RES_COLD) {
	info[i++] = "It provides resistance to cold.";
    }
    if (i_ptr->flags2 & TR2_RES_POIS) {
	info[i++] = "It provides resistance to poison.";
    }

    if (i_ptr->flags2 & TR2_RES_LITE) {
	info[i++] = "It provides resistance to light.";
    }
    if (i_ptr->flags2 & TR2_RES_DARK) {
	info[i++] = "It provides resistance to dark.";
    }

    if (i_ptr->flags2 & TR2_RES_BLIND) {
	info[i++] = "It provides resistance to blindness.";
    }
    if (i_ptr->flags2 & TR2_RES_CONF) {
	info[i++] = "It provides resistance to confusion.";
    }
    if (i_ptr->flags2 & TR2_RES_SOUND) {
	info[i++] = "It provides resistance to sound.";
    }
    if (i_ptr->flags2 & TR2_RES_SHARDS) {
	info[i++] = "It provides resistance to shards.";
    }

    if (i_ptr->flags2 & TR2_RES_NETHER) {
	info[i++] = "It provides resistance to nether.";
    }
    if (i_ptr->flags2 & TR2_RES_NEXUS) {
	info[i++] = "It provides resistance to nexus.";
    }
    if (i_ptr->flags2 & TR2_RES_CHAOS) {
	info[i++] = "It provides resistance to chaos.";
    }
    if (i_ptr->flags2 & TR2_RES_DISEN) {
	info[i++] = "It provides resistance to disenchantment.";
    }

    if (i_ptr->flags3 & TR3_FEATHER) {
	info[i++] = "It induces feather falling.";
    }
    if (i_ptr->flags3 & TR3_LITE) {
	info[i++] = "It provides permanent light.";
    }
    if (i_ptr->flags3 & TR3_SEE_INVIS) {
	info[i++] = "It allows you to see invisible monsters.";
    }
    if (i_ptr->flags3 & TR3_TELEPATHY) {
	info[i++] = "It gives telepathic powers.";
    }
    if (i_ptr->flags3 & TR3_SLOW_DIGEST) {
	info[i++] = "It slows your metabolism.";
    }
    if (i_ptr->flags3 & TR3_REGEN) {
	info[i++] = "It speeds your regenerative powers.";
    }

    if (i_ptr->flags3 & TR3_XTRA_MIGHT) {
	info[i++] = "It fires missiles with extra might.";
    }
    if (i_ptr->flags3 & TR3_XTRA_SHOTS) {
	info[i++] = "It fires missiles excessively fast.";
    }

    if (i_ptr->flags3 & TR3_ACTIVATE) {
	info[i++] = "It can be activated.";
    }
    if (i_ptr->flags3 & TR3_DRAIN_EXP) {
	info[i++] = "It drains experience.";
    }
    if (i_ptr->flags3 & TR3_TELEPORT) {
	info[i++] = "It induces random teleportation.";
    }
    if (i_ptr->flags3 & TR3_AGGRAVATE) {
	info[i++] = "It aggravates nearby creatures.";
    }

    if (i_ptr->flags3 & TR3_BLESSED) {
	info[i++] = "It has been blessed by the gods.";
    }

    if (i_ptr->flags3 & TR3_PERMA_CURSE) {
	info[i++] = "It is permanently cursed.";
    }
    else if (i_ptr->flags3 & TR3_HEAVY_CURSE) {
	info[i++] = "It is heavily cursed.";
    }
    else if (i_ptr->flags3 & TR3_CURSED) {
	info[i++] = "It is cursed.";
    }

    if (i_ptr->flags3 & TR3_IGNORE_ACID) {
	info[i++] = "It cannot be harmed by acid.";
    }
    if (i_ptr->flags3 & TR3_IGNORE_ELEC) {
	info[i++] = "It cannot be harmed by electricity.";
    }
    if (i_ptr->flags3 & TR3_IGNORE_FIRE) {
	info[i++] = "It cannot be harmed by fire.";
    }
    if (i_ptr->flags3 & TR3_IGNORE_COLD) {
	info[i++] = "It cannot be harmed by cold.";
    }


    /* No special effects */
    if (!i) return (item_val);
    

    /* Save the screen */
    save_screen();

    /* Erase the screen */
    for (k = 1; k < 24; k++) erase_line(k, 13);

    /* Label the information */
    prt("     Item Attributes:", 1, 15);

    /* We will print on top of the map (column 13) */
    for (k = 2, j = 0; j < i; j++) {

	/* Show the info */
	prt(info[j], k++, 15);

	/* Every 20 entries (lines 2 to 21), start over */
	if ((k == 22) && (j+1 < i)) {
	    prt("-- more --", k, 15);
	    inkey();
	    for ( ; k > 2; k--) erase_line(k, 15);
	}
    }

    /* Wait for it */
    prt("[Press any key to continue]", k, 15);
    inkey();
    
    /* Restore the screen */
    restore_screen();


    /* Return the index of the item */
    return (item_val);
}




/*
 * Get all the monsters on the level pissed off.	-RAK-	 
 */
int aggravate_monster(int dis_affect)
{
    register int           i, aggravate;
    register monster_type *m_ptr;

    aggravate = FALSE;
    for (i = m_max - 1; i >= MIN_M_IDX; i--) {
	m_ptr = &m_list[i];
	m_ptr->csleep = 0;
	if ((m_ptr->cdis <= dis_affect) && (m_ptr->mspeed < 120)) {
	    m_ptr->mspeed += 10;
	    aggravate = TRUE;
	}
    }
    if (aggravate) {
	msg_print("You hear a sudden stirring in the distance!");
    }
    return (aggravate);
}


/*
 * Display all creatures on the current panel		-RAK-	 
 */
int detect_monsters(void)
{
    register int        i, detect;
    register monster_type *m_ptr;

    detect = FALSE;
    for (i = m_max - 1; i >= MIN_M_IDX; i--) {

	m_ptr = &m_list[i];

	if (panel_contains((int)m_ptr->fy, (int)m_ptr->fx) &&
	    ((MF1_MV_INVIS & r_list[m_ptr->r_idx].cflags1) == 0)) {

	    /* Draw the monster (unless invisible) */
	    m_ptr->ml = TRUE;
	    lite_monster(m_ptr);

	    detect = TRUE;
	}
    }

    if (detect) {

	/* Describe, and wait for acknowledgement */
	msg_print("You sense the presence of monsters!");
	msg_print(NULL);

	/* Fix the monsters */
	update_monsters();
    }

    return (detect);
}



/*
 * Lightning ball in all directions    -SM-   
 */
void starball(int y, int x)
{
    register int i;

    for (i = 1; i <= 9; i++) {
	if (i != 5) {
	    fire_ball(GF_ELEC, i, y, x, 150, 3);
	}
    }
}




/*
 * Light line in all directions				-RAK-	 
 */
void starlite(int y, int x)
{
    register int i;

    if (p_ptr->blind < 1) {
	msg_print("The end of the staff bursts into a blue shimmering light.");
    }
    for (i = 1; i <= 9; i++) {
	if (i != 5) lite_line(i, y, x);
    }
}



/*
 * Recharge a wand/staff/rod.  Does not work on stacked items.
 *
 * recharge I = recharge(20) = 1/6 failure for empty 10th level wand  
 * recharge II = recharge(60) = 1/10 failure for empty 10th level wand
 * make it harder to recharge high level, and highly charged wands    
 */
int recharge(int num)
{
    int                 i, j, i1, i2, item_val, lev;
    inven_type		*i_ptr;

    /* No range found yet */
    i1 = 999, i2 = -1;

    /* Check for wands */
    if (find_range(TV_WAND, TV_NEVER, &i, &j)) {
	if (i < i1) i1 = i;
	if (j > i2) i2 = j;
    }

    /* Check for staffs */
    if (find_range(TV_STAFF, TV_NEVER, &i, &j)) {
	if (i < i1) i1 = i;
	if (j > i2) i2 = j;
    }

    /* Hack -- Check for rods */
    if (find_range(TV_ROD, TV_NEVER, &i, &j)) {
	if (i < i1) i1 = i;
	if (j > i2) i2 = j;
    }

    /* Quick check */
    if (i1 > i2) {
	msg_print("You have nothing to recharge.");
	return (FALSE);
    }

    /* Ask for it */
    if (!get_item(&item_val, "Recharge which item? ", i1, i2)) return (FALSE);

    /* Get the item */
    i_ptr = &inventory[item_val];

    /* Verify item */
    if ((i_ptr->tval != TV_WAND) &&
	(i_ptr->tval != TV_STAFF) &&
	(i_ptr->tval != TV_ROD)) {

	message("Oops.  That item cannot be recharged.", 0);
	return (FALSE);
    }


    /* Hack -- refuse to recharge stacked items */
    if (i_ptr->number > 1) {

	message("Oops.  You cannot recharge stacked items.", 0);
	return (FALSE);
    }


    /* Extract the object "level" */
    lev = k_list[i_ptr->k_idx].level;

    /* Recharge a rod */
    if (i_ptr->tval == TV_ROD) {

	u16b t, t_p = i_ptr->pval;

	/* Back-fire */
	if (randint((100 - lev + num) / 5) == 1) {
	    msg_print("The recharge backfires, draining the rod further!");
	    /* don't overflow... */
	    if (t_p < 10000) i_ptr->pval = (t_p + 100) * 2;
	}

	/* Recharge */
	else {
	    /* rechange amount */
	    t = (u16b) (num * damroll(2, 4));
	    if (t_p < t) i_ptr->pval = 0;
	    else i_ptr->pval = t_p - t;
	}
    }

    /* recharge wand/staff */
    else {

	/* Back-fire */
	if (randint((num + 100 - lev - (10 * i_ptr->pval)) / 15) == 1) {
	    msg_print("There is a bright flash of light.");
	    inven_item_increase(item_val, -1);
	    inven_item_optimize(item_val);
	}

	/* Recharge */
	else {
	    num = (num / (lev + 2)) + 1;
	    i_ptr->pval += 2 + randint(num);

	    /* Hack -- we no longer "know" the item */
	    i_ptr->ident &= ~ID_KNOWN;

	    /* Hack -- we no longer think the item is empty */
	    i_ptr->ident &= ~ID_EMPTY;
	}
    }

    /* Something was done */
    return (TRUE);
}




/*
 * Hack -- hooks for the old "beam"/"bolt"/"ball"/"breath" functions
 * These variables help convert "directions" into "destinations".
 */

static int dx[10] = { 0, -1, 0, 1, -1, 0, 1, -1, 0, 1 };
static int dy[10] = { 0, 1, 1, 1, 0, 0, 0, -1, -1, -1 };


/*
 * Hooks for the old "player spells"
 */

static bool project_hook(int typ, int dir, int dam, int flg)
{
    int tx, ty;

    /* Pass through the target if needed */
    flg |= (PROJECT_THRU);

    /* Check for "target request" */
    if ((dir == 0) && target_okay()) {
	tx = target_col;
	ty = target_row;
    }

    /* Just use the direction, go until something gets hit */
    else {
	tx = char_col + dx[dir];
	ty = char_row + dy[dir];
    }

    /* Analyze the "dir" and the "target", do NOT explode */
    return (project(1, 0, ty, tx, dam, typ, flg));
}

void fire_ball(int typ, int dir, int py, int px, int dam_hp, int max_dis)
{
    int tx, ty;

    int flg = PROJECT_GRID | PROJECT_ITEM;

    /* Check for "target request" */
    if ((dir == 0) && target_okay()) {
	tx = target_col;
	ty = target_row;
    }

    /* Just use the direction, go until something gets hit */
    else {
	flg |= (PROJECT_STOP | PROJECT_THRU);
	tx = char_col + dx[dir];
	ty = char_row + dy[dir];
    }

    /* Analyze the "dir" and the "target".  Hurt items on floor. */
    project(1, max_dis, ty, tx, dam_hp, typ, flg);
}

void fire_bolt(int typ, int dir, int y, int x, int dam)
{
    int flg = PROJECT_STOP;
    project_hook(typ, dir, dam, flg);
}

int line_spell(int typ, int dir, int y, int x, int dam)
{
    /* Go until we have to stop, do "beam" damage to everyone */
    /* Also, affect all objects and grids we pass through */
    int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM;
    return (project_hook(typ, dir, dam, flg));
}

void lite_line(int dir, int y, int x)
{
    (void)line_spell(GF_LITE_WEAK, dir, y, x, damroll(6, 8));
}

int drain_life(int dir, int y, int x, int dam)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_DRAIN, dir, dam, flg));
}

int wall_to_mud(int dir, int y, int x)
{
    int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
    return (project_hook(GF_KILL_WALL, dir, 20 + randint(30), flg));
}

int td_destroy2(int dir, int y, int x)
{
    int flg = PROJECT_BEAM | PROJECT_ITEM | PROJECT_HIDE;
    return (project_hook(GF_KILL_DOOR, dir, 0, flg));
}

int disarm_all(int dir, int y, int x)
{
    int flg = PROJECT_BEAM | PROJECT_ITEM | PROJECT_HIDE;
    return (project_hook(GF_KILL_TRAP, dir, 0, flg));
}

int td_destroy()
{
    int flg = PROJECT_ITEM | PROJECT_HIDE;
    return (project(1, 1, char_row, char_col, 0, GF_KILL_DOOR, flg));
}

int door_creation()
{
    int flg = PROJECT_ITEM | PROJECT_GRID | PROJECT_HIDE;
    return (project(1, 1, char_row, char_col, 0, GF_MAKE_DOOR, flg));
}

int trap_creation()
{
    int flg = PROJECT_ITEM | PROJECT_GRID | PROJECT_HIDE;
    return (project(1, 1, char_row, char_col, 0, GF_MAKE_TRAP, flg));
}

int heal_monster(int dir, int y, int x)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_HEAL, dir, damroll(4, 6), flg));
}

int speed_monster(int dir, int y, int x)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_SPEED, dir, 0, flg));
}

int slow_monster(int dir, int y, int x)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_SLOW, dir, p_ptr->lev, flg));
}

int sleep_monster(int dir, int y, int x)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_SLEEP, dir, p_ptr->lev, flg));
}

int confuse_monster(int dir, int y, int x, int plev)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_CONF, dir, plev, flg));
}

int fear_monster(int dir, int y, int x, int plev)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_SCARE, dir, plev, flg));
}

int poly_monster(int dir, int y, int x)
{
    int flg = PROJECT_BEAM;
    return (project_hook(GF_OLD_POLY, dir, p_ptr->lev, flg));
}

int clone_monster(int dir, int y, int x)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_CLONE, dir, 0, flg));
}

int teleport_monster(int dir, int y, int x)
{
    int flg = PROJECT_BEAM;
    return (project_hook(GF_OLD_TPORT, dir, MAX_SIGHT * 5, flg));
}

int lite_area(int y, int x, int dam, int rad)
{
    /* Hack -- Message */
    if (p_ptr->blind < 1) {
	msg_print("You are surrounded by a white light.");
    }

    /* Hook into the "project()" function */
    return (project(1, rad, char_row, char_col, dam, GF_LITE_WEAK, PROJECT_GRID));
}

int unlite_area(int y, int x)
{
    /* Hack -- Message */
    if (p_ptr->blind < 1) {
	msg_print("Darkness surrounds you.");
    }

    /* Simple "unlite_area" attack centered on player -- hurts lite hounds */
    return (project(1, 3, char_row, char_col, 10, GF_DARK_WEAK, PROJECT_GRID));
}





/*
 * Hooks for the old "monster attacks"
 */

void bolt(int m_idx, int typ, int dam_hp)
{
    int flg = PROJECT_STOP;

    /* Go towards player, hit people in the way */
    (void)project(m_idx, 0, char_row, char_col, dam_hp, typ, flg);
}

void breath(int m_idx, int typ, int dam_hp)
{
    int max_dis;
    int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_XTRA;

    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_list[m_ptr->r_idx];

    /* Determine the radius of the blast */
    max_dis = 2;
    if (strchr("vDEA&", r_ptr->r_char)) max_dis = 3;
    if ((strchr("dR", r_ptr->r_char) && (r_ptr->cflags2 & MF2_UNIQUE))) max_dis = 3;

    /* Go towards player, do not hit anyone else, hurt items on ground. */
    (void)project(m_idx, max_dis, char_row, char_col, dam_hp, typ, flg);
}







