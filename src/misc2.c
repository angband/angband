/* File: misc2.c */

/* Purpose: misc code for monsters */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "angband.h"


/*
 * Ludwig's Brainstorm
 */
static int test_place(int y, int x)
{
    if (!floor_grid(y, x)) return (FALSE);

    if (cave[y][x].fval == NULL_WALL) return (FALSE);

    if (cave[y][x].m_idx) return (FALSE);

    if ((y == char_row) && (x == char_col)) return (FALSE);

    return TRUE;
}



/*
 * Deletes a monster entry from the level.
 */
static void delete_monster_fully(int j)
{
    register cave_type *c_ptr;
    register monster_type *m_ptr;
    register int fx, fy;

    /* Paranoia */
    if (j < MIN_M_IDX) return;

    /* Get the monster */
    m_ptr = &m_list[j];

    /* Get the monster location */
    fx = m_ptr->fx;
    fy = m_ptr->fy;

    /* One less of this monster on this level */
    l_list[m_ptr->r_idx].cur_num--;

    /* Forget that the monster is here (if it actually was) */
    if (cave[fy][fx].m_idx == j) cave[fy][fx].m_idx = 0;

    /* Visual update */
    lite_spot(m_ptr->fy, m_ptr->fx);

#ifdef TARGET
    /* This targetting code stolen from Morgul -CFT */
    /* Targetted monster dead or compacted.      CDW */
    if (j == target_mon) target_mode = FALSE;
#endif

    /* One less monster */
    m_max--;

    /* Do structure dumping */
    if (j != m_max) {

#ifdef TARGET
	/* Targetted monster moved to replace dead or compacted monster -CDW */
	if (target_mon == (int)(m_max)) target_mon = j;
#endif

	/* Slide the last monster into the dead monster's slot */
	m_ptr = &m_list[m_max];
	c_ptr = &cave[m_ptr->fy][m_ptr->fx];
	c_ptr->m_idx = j;
	m_list[j] = m_list[m_max];
    }

    /* Wipe the monster record */
    m_list[m_max] = m_list[0];

    /* Mega-Hack -- slow down reproduction */
    if (mon_tot_mult > 0) mon_tot_mult--;
}




/*
 * This routine "marks" a monster to be killed later.  The obvious method
 * is to set the hitpoints to be negative, though this is not the best method,
 * since negative hitpoints can be "cured".  We should really have a "nuke" flag.
 *
 * But anyway, this routine marks a monster to be deleted "later".  Currently,
 * the algorithm involves setting "hack_m_idx" to the index of the highest
 * monster that should be left alone.  This is done mostly in "process_monsters()",
 * where the monsters are processed from the last monster down, and we do not
 * want monsters that have already had a turn to replace monsters that have not
 * had a turn yet, or they will get two turns.  So we mark monsters as "dead",
 * and then, when it is actually their turn, we remove them.
 *
 * Note that we will be in trouble if the last monster in a full monster list
 * attempts to summon monsters, since compaction will fail, and so will the
 * monster creation.  There are several solutions for this, none of which seem
 * to have been done.  The easiest is to put a limit on "births" per turn, and
 * to always compact before each turn to be sure at least that many spaces are
 * available.  This can induce thrashing, but "mon_tot_mult" should cut down
 * on stuff like that.
 *
 * In fact, in general, it is a bad idea to delete "monsters" or any other type
 * of record "mid-processing", the standard technique is to mark things as dead,
 * and then to "garbage collect" at the end of each turn.
 *
 * I suppose somebody was really desperate for a few bytes of space...
 *
 * It appears that the clearing of the cave grid is purely syntactic sugar
 * for the "polymorph" routine and "monster eating" routines.  No big deal.
 * It still makes me paranoid.  Maybe it would be better to actually set some
 * form of "polymorph at the end of this turn" flag for the monster.  Call
 * me paranoid, but SOMETHING is creating "invisible monsters" again...
 */
static void delete_monster_later(int j)
{
    register monster_type *m_ptr;
    register int fx, fy;

    /* Paranoia */
    if (j < MIN_M_IDX) return;

    /* Get the monster */
    m_ptr = &m_list[j];

    /* Get the cave */
    fy = m_ptr->fy;
    fx = m_ptr->fx;

    /* Forget that the monster is here */
    if (cave[fy][fx].m_idx == j) cave[fy][fx].m_idx = 0;

    /* Visual update */
    lite_spot(m_ptr->fy, m_ptr->fx);

    /* Mark the monster as "dead" (non-optimal method) */
    m_ptr->hp = (-1);
}


/*
 * Delete a monster, now if possible, later if necessary
 *
 * We use "hack_m_idx" to determine if the monster can be safely deleted now.
 * The "hack_m_idx" is set only in "process_monsters()" and is always "-1"
 * except during the execution of process_monsters().
 */
void delete_monster_idx(int i)
{
    /* Paranoia */
    if (i < MIN_M_IDX) return;

    /* This monster will be processed later */
    if (i < hack_m_idx) {

	/* Mark the monster as dead */
	delete_monster_later(i);
    }

    /* This monster might as well be deleted now */
    else {

	/* Actually delete the monster */
	delete_monster_fully(i);
    }
}


/*
 * Delete the monster, if any, at a given location
 */
void delete_monster(int y, int x)
{
    cave_type *c_ptr;

    /* Paranoia */
    if (!in_bounds(y,x)) return;

    /* Check the grid */
    c_ptr = &cave[y][x];

    /* Hack -- no such monster */
    if (c_ptr->m_idx < MIN_M_IDX) return;

    /* Delete the monster */
    delete_monster_idx(c_ptr->m_idx);
}




/*
 * Wipe the monster list (for a new level)
 */
void wipe_m_list()
{
    register int i;

    /* Delete the existing monsters (backwards!) */
    for (i = m_max-1; i >= MIN_M_IDX; i--) delete_monster_idx(i);

    /* XXX Should already be done */
    m_max = MIN_M_IDX;
}


/*
 * Attempt to Compact some monsters (safely)	-RAK-
 *
 * XXX Base the saving throw on a combination of
 * monster level and current "desperation".
 */
static void compact_monsters(void)
{
    register int           i;
    int                    cur_dis, orig;
    monster_type	*m_ptr;
    monster_race	*r_ptr;

    msg_print("Compacting monsters...");

    /* Remember how many monsters we started with */
    orig = m_max;

    /* Start 66 (that is, 72-6) units away */
    cur_dis = 72;

    /* Keep going until someone is deleted */
    while (m_max == orig) {

	/* Nothing to compact (!!!) */
	if (cur_dis < 0) return;

	/* Come closer to the player */
	cur_dis -= 6;

	/* Check all the monsters */
	for (i = m_max - 1; i >= MIN_M_IDX; i--) {

	    m_ptr = &m_list[i];
	    r_ptr = &r_list[m_ptr->r_idx];

	    /* Ignore nearby monsters */
	    if (m_ptr->cdis < cur_dis) continue;

	    /* Never compact "Quest" Monsters */
	    if (r_ptr->cflags2 & MF2_QUESTOR) continue;

	    /* XXX Try not to compact Unique Monsters */
	    /* if ((r_ptr->cflags2 & MF2_UNIQUE) && ???) continue; */

	    /* All monsters get a saving throw */
	    if (randint(3) == 1) {

		/* Delete the monster */
		delete_monster_idx(i);
	    }
	}
    }

    /* redraw */
    prt_map();
}


/*
 * Allow "dungeon.c" to pre-emptively compact the monster list
 */
void tighten_m_list(void)
{
    /* If not much space left, try compacting */
    if (MAX_M_IDX - m_max < 10) compact_monsters();
}


/*
 * Returns a pointer to next free space			-RAK-
 */
int m_pop(void)
{
    /* Out of space?  Compact. */
    if (m_max == MAX_M_IDX) compact_monsters();

    /* Still no slots?  XXX Abort! */    
    if (m_max == MAX_M_IDX) return (-1);

    /* Return (and increase) free slot */
    return (m_max++);
}


/*
 * Check for the presence of a breath weapon
 */
static int has_breath(int z)
{
    monster_race *r_ptr = &r_list[z];

    if ((r_ptr->spells1 &
		(MS1_CAUSE_1 | MS1_CAUSE_2 | MS1_HOLD |
		MS1_BLIND | MS1_CONF | MS1_FEAR | MS1_SLOW | MS1_BR_ELEC |
		MS1_BR_POIS | MS1_BR_ACID | MS1_BR_COLD | MS1_BR_FIRE |
		MS1_BO_FIRE | MS1_BO_COLD | MS1_BO_ACID | MS1_ARROW_1 |
		MS1_CAUSE_3 | MS1_BA_FIRE | MS1_BA_COLD | MS1_BO_MANA)) ||
	(r_ptr->spells2 &
		(MS2_BR_CHAO | MS2_BR_SHAR | MS2_BR_SOUN | MS2_BR_CONF |
		MS2_BR_DISE | MS2_BR_LIFE | MS2_BO_ELEC | MS2_BA_ELEC |
		MS2_BA_ACID | MS2_TRAP_CREATE | MS2_RAZOR | MS2_MIND_BLAST |
		MS2_ARROW_2 | MS2_BO_PLAS | MS2_BO_NETH | MS2_BO_ICEE |
		MS2_FORGET | MS2_BRAIN_SMASH | MS2_BA_POIS | MS2_TELE_LEVEL |
		MS2_BO_WATE | MS2_BA_WATE | MS2_BA_NETH | MS2_BR_NETH)) ||
	(r_ptr->spells3 &
		(MS3_BR_WALL | MS3_BR_SLOW | MS3_BR_LITE | MS3_BR_TIME |
		MS3_BR_GRAV | MS3_BR_DARK | MS3_BR_PLAS | MS3_ARROW_3 |
		MS3_DARK_STORM | MS3_MANA_STORM)) ) {

	return TRUE;
    }

    /* No power-spells */
    return FALSE;
}


/*
 * Places a monster at given location
 *
 * Refuses to place out-of-depth Quest Monsters.
 */
int place_monster(int y, int x, int r_idx, int slp)
{
    register int           cur_pos, j, z, ny, nx, count;
    register monster_type *m_ptr;
    register monster_race *r_ptr;
    char                   buf[100];

    /* Verify monster race */
    if ((r_idx < 0) || (r_idx >= MAX_R_IDX)) return FALSE;

    /* Verify location */
    if (!test_place(y, x)) return FALSE;

    /* Get the race */
    r_ptr = &r_list[r_idx];

    /* See if we can make any more of them */
    if (l_list[r_idx].cur_num >= l_list[r_idx].max_num) {

	/* Note for wizard */
	if (wizard) {
	    (void)sprintf(buf, "Ignoring %s '%s' monster.",
			  (l_list[r_idx].max_num ? "excessive" : "dead"),
			  r_ptr->name);
	    msg_print(buf);
	}

	/* Cannot create */
	return FALSE;
    }

    /* Quest monsters may NOT be created out of depth */
    if ((r_ptr->cflags2 & MF2_QUESTOR) && (dun_level < r_ptr->level)) {

	/* Note for wizard */
	if (wizard) {
	    (void)sprintf(buf, "Ignoring shallow '%s' monster.", r_ptr->name);
	    msg_print(buf);
	}

	/* Cannot create */
	return FALSE;
    }

    /* Count the monsters on the level */
    l_list[r_idx].cur_num++;

    /* Get the next monster record */
    cur_pos = m_pop();

    /* Mega-Paranoia */
    if (cur_pos == -1) return FALSE;

    /* Powerful monster */
    if (r_ptr->level > dun_level) {

	j = r_ptr->level - dun_level;
	rating += ((j > 30) ? 15 : (j / 2));

	/* XXX Uniques get a double rating bonus? */
	if (r_ptr->cflags2 & MF2_UNIQUE) rating += (j / 2);
    }

    /* Get a new monster record */
    m_ptr = &m_list[cur_pos];

    /* Update the cave */
    cave[y][x].m_idx = cur_pos;

    /* Place the monster at the location */
    m_ptr->fy = y;
    m_ptr->fx = x;

    /* Save the race */
    m_ptr->r_idx = r_idx;

    /* Assign maximal hitpoints */
    if (r_ptr->cflags2 & MF2_MAX_HP) {
	m_ptr->maxhp = max_hp(r_ptr->hd);
    }
    else {
	m_ptr->maxhp = pdamroll(r_ptr->hd);
    }

    /* And start out fully healthy */
    m_ptr->hp = m_ptr->maxhp;

    /* Hack -- speed saved in compact form */
    m_ptr->cspeed = r_ptr->speed - 10;

    /* No "damage" yet */
    m_ptr->stunned = 0;
    m_ptr->confused = 0;
    m_ptr->monfear = 0;

    /* Default to invisible */
    m_ptr->ml = FALSE;

    /* Update the monster. */
    update_mon(cur_pos);


    /* Update the monster sleep info */
    if (slp) {
	if (r_ptr->sleep == 0) {
	    m_ptr->csleep = 0;
	}
	else {
	    m_ptr->csleep = ((int)r_ptr->sleep * 2) +
			     randint((int)r_ptr->sleep * 10);
	}
    }

    /* to give the player a sporting chance, any monster that appears in */
    /* line-of-sight and can cast spells or breathe, should be asleep.   */
    /* This is an extension of Um55's sleeping dragon code...            */

    /* Sleep just long enough for player to react */
    else if (has_breath(r_idx) && los(y, x, char_row, char_col)) {
	m_ptr->csleep = randint(4);
    }

    /* Wake up... */
    else {
	m_ptr->csleep = 0;
    }



    /* Unique kobolds, Liches, orcs, Ogres, Trolls, yeeks, and demons */
    /* get a "following" of escorts.  -DGK-    But not skeletons, */
    /* which include druj, which would make Cantoras amazingly tough -CFT */

    if (r_ptr->cflags2 & MF2_UNIQUE) {

	j = r_ptr->r_char;

	/* Monsters with escorts */
	if (strchr("kLoOTyI&", j)) {

	    /* Try for the highest level monster we can get */
	    for (z = MAX_R_IDX-1; z>=0; z--) {

		/* Find a similar, lower level, non-unique, monster */
		if ((r_list[z].r_char == j) &&
		    (r_list[z].level <= r_list[z].level) &&
		    !(r_list[z].cflags2 & MF2_UNIQUE)) {

		    /* Try up to 50 nearby places */
		    count = 0;
		    do {
			ny = rand_range(y-3,y+3);
			nx = rand_range(x-3,x+3);
			count++;
		    } while (!test_place(ny,nx) && (count<51));

		    /* Certain monsters come in groups */
		    if ((j=='k') || (j=='y') || (j=='&') ||
			(r_list[z].cflags2 & MF2_GROUP)) {
			place_group(ny,nx,z,slp);
		    }

		    /* Otherwise, just use a single escort */
		    else {
			place_monster(ny,nx,z,slp);
		    }
		}
	    }
	}
    }

    /* Success */
    return TRUE;
}


/*
 * Places a "winning" monster at given location	    -RAK-
 */
int place_win_monster()
{
    register int y, x;

    /* Hack -- caught by place_monster() */
    if (total_winner) return (FALSE);

    /* Attempt to place */
    if (wizard || peek) {
	msg_print("Placing win monster");
    }

    /* Find a legal, distant, unoccupied, space */
    while (1) {
	y = randint(cur_height - 2);
	x = randint(cur_width - 2);
	if (!floor_grid(y,x)) continue;
	if (cave[y][x].m_idx || cave[y][x].i_idx) continue;
	if (distance(y, x, char_row, char_col) <= MAX_SIGHT) continue;
	break;
    }

    /* Hack -- Attempt to place him (sleeping) */
    return (place_monster(y, x, MAX_R_IDX - 2, TRUE));
}



/*
 * XXX Note that g->name is set during "init_r_list()"
 *
 * I am a little concerned about the reliance of "ghost fields"
 * In particular, shouldn't we clear most of them first?
 * I worry, because "cflags2" is set pretty high in the
 * default initializations.
 */

static cptr ghost_race_names[] = {
    "human", "elf", "elf", "hobbit", "gnome",
    "dwarf", "orc", "troll", "human", "elf"
};

static cptr ghost_class_names[] = {
    "warrior", "mage", "priest",
    "rogue", "ranger", "paladin"
};

static int8u ghost_class_colors[] = {
    COLOR_L_BLUE, COLOR_RED, COLOR_L_GREEN,
    COLOR_BLUE, COLOR_GREEN, COLOR_WHITE
};


/*
 * Prepare the "ghost" monster_race info
 *
 * XXX This code has only recently been debugged,
 * so it may have brand new bugs now.
 *
 * Even if not, it really needs to be re-writtem, there are redundancies
 * and incorrectnesses everywhere.  And the savefile ruins everything.
 * Actually, the new savefile is "much better".  It may fix the problems.
 */
static void set_ghost(monster_race *g, cptr pn, int gr, int gc, int lev)
{
    int  i;

    char name[20];
    char gr_name[20];
    char gc_name[20];

    /* Extract the basic ghost name */
    strcpy(name, pn);

    /* Extract the race and class names */
    strcpy(gr_name, ghost_race_names[gr]);
    strcpy(gc_name, ghost_class_names[gc]);

    /* Capitalize the name */
    if (islower(name[0])) name[0] = toupper(name[0]);

    /* Capitalize the race/class */
    if (islower(gr_name[0])) gr_name[0] = toupper(gr_name[0]);
    if (islower(gc_name[0])) gc_name[0] = toupper(gc_name[0]);

    /* Forget any flags a previous ghost had */
    g->cflags1 = g->cflags2 = 0L;

    /* Forget any spells a previous ghost had */
    g->spells1 = g->spells2 = g->spells3 = 0L;

    /* Save the level, extract the experience */
    g->level = lev;
    g->mexp = lev * 5 + 5;

    /* Never asleep (?) */
    g->sleep = 0;

    /* Very attentive (?) */
    g->aaf = 100;


    /* Initialize some of the flags */
    g->cflags1 |= (MF1_MV_ATT_NORM | MF1_CARRY_OBJ);
    g->cflags2 |= (MF2_GOOD);
    g->cflags2 |= (MF2_UNIQUE | MF2_CHARM_SLEEP | MF2_EVIL);


    /* Town ghost */
    if (!dun_level) {

	/* Save the color */
	g->r_attr = ghost_class_colors[gc];

	/* A wanderer in the town */
	sprintf(ghost_name, "%s, the %s %s",
		name, gr_name, gc_name);

	g->cflags1 |= (MF1_THRO_DR | MF1_HAS_90 | MF1_HAS_60);

	if (lev > 10) g->cflags1 |= (MF1_HAS_1D2);
	if (lev > 18) g->cflags1 |= (MF1_HAS_2D2);
	if (lev > 23) g->cflags1 |= (MF1_HAS_4D2);
	if (lev > 40) g->cflags1 |= (MF2_SPECIAL);
	if (lev > 40) g->cflags1 &= (~MF1_HAS_4D2);

	/* Add some random resists -DGK */
	for (i = 0; i <= (lev / 5); i++) {
	    switch ((int) randint(13)) {
	      case 1:
	      case 2:
	      case 3:
		g->cflags2 |= (MF2_IM_FIRE);
	      case 4:
	      case 5:
	      case 6:
		g->cflags2 |= (MF2_IM_ACID);
	      case 7:
	      case 8:
	      case 9:
		g->cflags2 |= (MF2_IM_COLD);
	      case 10:
	      case 11:
	      case 12:
		g->cflags2 |= (MF2_IM_ELEC);
	      case 13:
		g->cflags2 |= (MF2_IM_POIS);
	    }
	}

	switch (gc) {
	  case 0:		   /* Warrior */
	    break;
	  case 1:		   /* Mage */
	    g->spells1 |= (0x3L | MS1_BLINK | MS1_ARROW_1 |
			   MS1_SLOW | MS1_CONF);
	    if (lev > 5) g->spells2 |= MS2_BA_POIS;
	    if (lev > 7) g->spells2 |= MS2_BO_ELEC;
	    if (lev > 10) g->spells1 |= MS1_BO_COLD;
	    if (lev > 12) g->spells1 |= MS1_TELEPORT;
	    if (lev > 15) g->spells1 |= MS1_BO_ACID;
	    if (lev > 20) g->spells1 |= MS1_BO_FIRE;
	    if (lev > 25) g->spells1 |= MS1_BA_COLD;
	    if (lev > 25) g->spells2 |= MS2_HASTE;
	    if (lev > 30) g->spells1 |= MS1_BA_FIRE;
	    if (lev > 40) g->spells1 |= MS1_BO_MANA;
	    break;
	  case 3:		   /* Rogue */
	    g->spells1 |= (0x5L | MS1_BLINK);
	    if (lev > 10) g->spells1 |= MS1_CONF;
	    if (lev > 18) g->spells1 |= MS1_SLOW;
	    if (lev > 25) g->spells1 |= MS1_TELEPORT;
	    if (lev > 30) g->spells1 |= MS1_HOLD;
	    if (lev > 35) g->spells1 |= MS1_TELE_TO;
	    break;
	  case 4:		   /* Ranger */
	    g->spells1 |= (0x8L | MS1_ARROW_1);
	    if (lev > 5) g->spells2 |= MS2_BA_POIS;
	    if (lev > 7) g->spells2 |= MS2_BO_ELEC;
	    if (lev > 10) g->spells1 |= MS1_BO_COLD;
	    if (lev > 18) g->spells1 |= MS1_BO_ACID;
	    if (lev > 25) g->spells1 |= MS1_BO_FIRE;
	    if (lev > 30) g->spells1 |= MS1_BA_COLD;
	    if (lev > 35) g->spells1 |= MS1_BA_FIRE;
	    break;
	  case 2:		   /* Priest */
	  case 5:		   /* Paladin */
	    g->spells1 |= (0x4L | MS1_CAUSE_1 | MS1_FEAR);
	    if (lev > 5) g->spells2 |= MS2_HEAL;
	    if (lev > 10) g->spells1 |= (MS1_CAUSE_2 | MS1_BLIND);
	    if (lev > 18) g->spells1 |= MS1_HOLD;
	    if (lev > 25) g->spells1 |= MS1_CONF;
	    if (lev > 30) g->spells1 |= MS1_CAUSE_3;
	    if (lev > 35) g->spells1 |= MS1_MANA_DRAIN;
	    break;
	}

	if (gr == 6) g->cflags2 |= MF2_ORC;
	if (gr == 7) g->cflags2 |= MF2_TROLL;

	g->ac = 15 + randint(15);
	if (gc == 0 || gc >= 3) g->ac += randint(60);

	/* Default speed (encoded) */
	g->speed = 11;

	/* High level mages and rogues are fast... */
	if ((gc == 1 || gc == 3) && lev > 25) g->speed++;

	/* Use the letter 'p' */
	g->r_char = 'p';

	/* XXX */
	g->hd[1] = 1;

	g->damage[0] = 5 + ((lev > 18) ? 18 : lev);
	g->damage[1] = g->damage[0];

	switch (gc) {
	  case 0:
	    g->damage[2] = ((lev < 30) ? (5 + ((lev > 18) ? 18 : lev)) : 235);
	    g->damage[3] = g->damage[2];
	    break;
	  case 1:
	  case 2:
	    g->damage[2] = 0;
	    g->damage[3] = 0;
	    break;
	  case 3:
	    g->damage[2] = g->damage[3] = ((lev < 30) ? 149 : 232);
	    break;
	  case 4:
	  case 5:
	    g->damage[2] = g->damage[3] = g->damage[1];
	    break;
	}

	return;
    }


    /* Initialize some more of the flags */    
    g->cflags2 |= (MF2_UNDEAD | MF2_NO_INFRA | MF2_IM_POIS);


    /* Make a ghost with power based on the ghost level */
    switch ((int) (g->level / 4) + randint(3)) {

      case 1:
      case 2:
      case 3:
	sprintf(ghost_name, "%s, the Skeleton %s", name, gr_name);
	g->cflags1 |= (MF1_THRO_DR | MF1_HAS_90);
	g->cflags2 |= (MF2_IM_COLD);
	if (gr == 6) g->cflags2 |= MF2_ORC;
	if (gr == 7) g->cflags2 |= MF2_TROLL;
	g->ac = 26;
	g->speed = 11;
	g->r_char = 's';
	g->r_attr = COLOR_WHITE;
	g->hd[1] = 1;
	g->damage[0] = 5;
	g->damage[1] = 5;
	g->damage[2] = 0;
	g->damage[3] = 0;
	break;

      case 4:
      case 5:
	sprintf(ghost_name, "%s, the %s zombie", name, gr_name);
	g->cflags1 |= (MF1_THRO_DR | MF1_HAS_60 | MF1_HAS_90);
	if (gr == 6) g->cflags2 |= MF2_ORC;
	if (gr == 7) g->cflags2 |= MF2_TROLL;
	g->ac = 30;
	g->speed = 11;
	g->r_char = 'z';
	g->r_attr = COLOR_GRAY;
	g->hd[1] *= 2;
	g->damage[0] = 8;
	g->damage[1] = 0;
	g->damage[2] = 0;
	g->damage[3] = 0;
	break;

      case 6:
	sprintf(ghost_name, "%s, the Poltergeist", name);
	g->cflags1 |= (MF1_MV_INVIS | MF1_HAS_1D2 | MF1_MV_75 | MF1_THRO_WALL);
	g->cflags2 |= (MF2_IM_COLD);
	g->ac = 20;
	g->speed = 13;
	g->r_char = 'G';
	g->r_attr = COLOR_WHITE;
	g->damage[0] = 5;
	g->damage[1] = 5;
	g->damage[2] = 93;
	g->damage[3] = 93;
	g->mexp = (g->mexp * 3) / 2;
	break;

      case 7:
      case 8:
	sprintf(ghost_name, "%s, the Mummified %s", name, gr_name);
	g->cflags1 |= (MF1_HAS_1D2);
	if (gr == 6) g->cflags2 |= MF2_ORC;
	if (gr == 7) g->cflags2 |= MF2_TROLL;
	g->ac = 35;
	g->speed = 11;
	g->r_char = 'M';
	g->r_attr = COLOR_GRAY;
	g->hd[1] *= 2;
	g->damage[0] = 16;
	g->damage[1] = 16;
	g->damage[2] = 16;
	g->damage[3] = 0;
	g->mexp = (g->mexp * 3) / 2;
	break;

      case 9:
      case 10:
	sprintf(ghost_name, "%s%s spirit", name,
		(name[strlen(name) - 1] == 's') ? "'" : "'s");
	g->cflags1 |= (MF1_MV_INVIS | MF1_THRO_WALL | MF1_HAS_1D2);
	g->cflags2 |= (MF2_IM_COLD);
	g->ac = 20;
	g->speed = 11;
	g->r_char = 'G';
	g->r_attr = COLOR_WHITE;
	g->hd[1] *= 2;
	g->damage[0] = 19;
	g->damage[1] = 185;
	g->damage[2] = 99;
	g->damage[3] = 178;
	g->mexp = g->mexp * 3;
	break;

      case 11:
	sprintf(ghost_name, "%s%s ghost", name,
		(name[strlen(name) - 1] == 's') ? "'" : "'s");
	g->cflags1 |= (MF1_MV_INVIS | MF1_THRO_WALL | MF1_HAS_1D2);
	g->cflags2 |= (MF2_IM_COLD);
	g->spells1 |= (0xFL | MS1_HOLD | MS1_MANA_DRAIN | MS1_BLIND);
	g->ac = 40;
	g->speed = 12;
	g->r_char = 'G';
	g->r_attr = COLOR_WHITE;
	g->hd[1] *= 2;
	g->damage[0] = 99;
	g->damage[1] = 99;
	g->damage[2] = 192;
	g->damage[3] = 184;
	g->mexp = (g->mexp * 7) / 2;
	break;

      case 12:
	sprintf(ghost_name, "%s, the Vampire", name);
	g->cflags1 |= (MF1_THRO_DR | MF1_HAS_2D2);
	g->cflags2 |= (MF2_HURT_LITE);
	g->spells1 |= (0x8L | MS1_HOLD | MS1_FEAR | MS1_TELE_TO | MS1_CAUSE_2);
	g->ac = 40;
	g->speed = 11;
	g->r_char = 'V';
	g->r_attr = COLOR_VIOLET;
	g->hd[1] *= 3;
	g->damage[0] = 20;
	g->damage[1] = 20;
	g->damage[2] = 190;
	g->damage[3] = 0;
	g->mexp = g->mexp * 3;
	break;

      case 13:
	sprintf(ghost_name, "%s%s Wraith", name,
		(name[strlen(name) - 1] == 's') ? "'" : "'s");
	g->cflags1 |= (MF1_THRO_DR | MF1_HAS_4D2 | MF1_HAS_2D2);
	g->cflags2 |= (MF2_IM_COLD | MF2_HURT_LITE);
	g->spells1 |= (0x7L | MS1_HOLD | MS1_FEAR | MS1_BLIND | MS1_CAUSE_3);
	g->spells2 |= (MS2_BO_NETH);
	g->ac = 60;
	g->speed = 12;
	g->r_char = 'W';
	g->r_attr = COLOR_WHITE;
	g->hd[1] *= 3;
	g->damage[0] = 20;
	g->damage[1] = 20;
	g->damage[2] = 190;
	g->damage[3] = 0;
	g->mexp = g->mexp * 5;
	break;

      case 14:
	sprintf(ghost_name, "%s, the Vampire Lord", name);
	g->cflags1 |= (MF1_THRO_DR | MF1_HAS_1D2);
	g->cflags2 |= (MF2_HURT_LITE | MF2_SPECIAL);
	g->spells1 |= (0x8L | MS1_HOLD | MS1_FEAR | MS1_TELE_TO | MS1_CAUSE_3);
	g->spells2 |= (MS2_BO_NETH);
	g->ac = 80;
	g->speed = 11;
	g->r_char = 'V';
	g->r_attr = COLOR_BLUE;
	g->hd[1] *= 2;
	g->hd[0] = (g->hd[0] * 5) / 2;
	g->damage[0] = 20;
	g->damage[1] = 20;
	g->damage[2] = 20;
	g->damage[3] = 198;
	g->mexp = g->mexp * 20;
	break;

      case 15:
	sprintf(ghost_name, "%s%s ghost", name,
		 (name[strlen(name) - 1] == 's') ? "'" : "'s");
	g->cflags1 |= (MF1_MV_INVIS | MF1_THRO_WALL | MF1_HAS_2D2);
	g->cflags2 |= (MF2_SPECIAL | MF2_IM_COLD);
	g->spells1 |= (0x5L | MS1_HOLD | MS1_MANA_DRAIN | MS1_BLIND | MS1_CONF);
	g->ac = 90;
	g->speed = 13;
	g->r_char = 'G';
	g->r_attr = COLOR_WHITE;
	g->hd[1] *= 3;
	g->damage[0] = 99;
	g->damage[1] = 99;
	g->damage[2] = 192;
	g->damage[3] = 184;
	g->mexp = g->mexp * 20;
	break;

      case 17:
	sprintf(ghost_name, "%s, the Lich", name);
	g->cflags1 |= (MF1_THRO_DR | MF1_HAS_2D2 | MF1_HAS_1D2);
	g->cflags2 |= (MF2_SPECIAL | MF2_IM_COLD | MF2_INTELLIGENT);
	g->spells1 |= (0x3L | MS1_FEAR | MS1_CAUSE_3 | MS1_TELE_TO | MS1_BLINK |
		       MS1_S_UNDEAD | MS1_BA_FIRE | MS1_BA_COLD | MS1_HOLD |
		       MS1_MANA_DRAIN | MS1_BLIND | MS1_CONF | MS1_TELEPORT);
	g->spells2 |= (MS2_BRAIN_SMASH | MS2_RAZOR);
	g->ac = 120;
	g->speed = 12;
	g->r_char = 'L';
	g->r_attr = COLOR_ORANGE;
	g->hd[1] *= 3;
	g->hd[0] *= 2;
	g->damage[0] = 181;
	g->damage[1] = 201;
	g->damage[2] = 214;
	g->damage[3] = 181;
	g->mexp = g->mexp * 50;
	break;

      default:
	sprintf(ghost_name, "%s%s ghost", name,
		(name[strlen(name) - 1] == 's') ? "'" : "'s");
	g->cflags1 |= (MF1_MV_INVIS | MF1_THRO_WALL |
		       MF1_HAS_1D2 | MF1_HAS_2D2);
	g->cflags2 |= (MF2_SPECIAL | MF2_IM_COLD | MF2_INTELLIGENT);
	g->spells1 |= (0x2L | MS1_HOLD | MS1_MANA_DRAIN | 
		       MS1_BLIND | MS1_CONF | MS1_TELE_TO);
	g->spells2 |= (MS2_BO_NETH | MS2_BA_NETH | MS2_BRAIN_SMASH |
		       MS2_TELE_LEVEL);
	g->ac = 130;
	g->speed = 13;
	g->r_char = 'G';
	g->r_attr = COLOR_WHITE;
	g->hd[1] *= 2;
	g->hd[0] = (g->hd[0] * 5) / 2;
	g->damage[0] = 99;
	g->damage[1] = 99;
	g->damage[2] = 192;
	g->damage[3] = 184;
	g->mexp = g->mexp * 30;
	break;
    }
}


/*
 * Places a ghost somewhere.
 * Probably not the best possible algorithm.
 */
int place_ghost()
{
    register int           y, x, cur_pos;
    register monster_type  *m_ptr;
    monster_race           *r_ptr;
    char                   name[100];
    int                    i, j, level;
    int                    gr;
    int                    gc;
    FILE		   *fp;

    char                   tmp[1024];


    /* The race is convenient */
    r_ptr = &r_list[MAX_R_IDX - 1];


    /* In the town, ghosts have the same level as the player */
    if (!dun_level) {

	/* You have to be level 5, and even then its only 90% */
	if (p_ptr->lev < 5 || randint(10) > 1) return 0;

	/* Look for a proper bones file */
	sprintf(tmp, "%s%s%d", ANGBAND_DIR_BONES, PATH_SEP, p_ptr->lev);
	fp = my_tfopen(tmp, "r");
	if (!fp) return (0);

	/* Read the bones info */
	if (fscanf(fp, "%[^\n]\n%d\n%d\n%d", name, &i, &gr, &gc) < 4) {
	    fclose(fp);
	    if (wizard) msg_print("Town:Failed to scan in info properly!");
	    return 0;
	}

	fclose(fp);

	level = p_ptr->lev;
    }

    /* In the dungeon, ghosts have the same level as the level */    
    else {

	/* And even then, it only happens sometimes */
	if (14 > randint((dun_level / 2) + 11)) return 0;

	/* Or rather, 1/3 of that often :-) */
	if (randint(3) != 1) return (0);

	/* Open the bones file */
	sprintf(tmp, "%s%s%d", ANGBAND_DIR_BONES, PATH_SEP, dun_level);
	fp = my_tfopen(tmp, "r");
	if (!fp) return (0);

	if (fscanf(fp, "%[^\n]\n%d\n%d\n%d", name, &i, &gr, &gc) < 4) {
	    fclose(fp);
	    if (wizard) msg_print("Ghost:Failed to scan in info properly!");
	    return 0;
	}
	fclose(fp);

	level = dun_level;
    }


    /* Break up the hitpoints */
    j = 1;
    if (i > 255) {
	j = i / 32;
	i = 32;
    }

    /* set_ghost may adj for race/class/lv */
    r_ptr->hd[0] = i;
    r_ptr->hd[1] = j;

    /* Set up the ghost */
    set_ghost(r_ptr, name, gr, gc, level);

    /* Note for wizard */
    if (wizard || peek) msg_print(r_ptr->name);

    cur_pos = m_pop();
    m_ptr = &m_list[cur_pos];

    /* Hack -- pick a nice (far away) location */
    do {
	y = randint(cur_height - 2);
	x = randint(cur_width - 2);
    } while (!floor_grid(y,x) ||
	     (cave[y][x].m_idx != 0) || (cave[y][x].i_idx != 0) ||
	     (distance(y, x, char_row, char_col) <= MAX_SIGHT));


    /*** Place the Ghost by Hand (so no-one else does it accidentally) ***/

    m_ptr->fy = y;
    m_ptr->fx = x;

    m_ptr->r_idx = MAX_R_IDX-1;

    m_ptr->hp = (int16) r_ptr->hd[0] * (int16) r_ptr->hd[1];

    /* the r_list speed value is 10 greater, so that it can be a int8u */
    m_ptr->cspeed = r_list[m_ptr->r_idx].speed - 10;

    m_ptr->stunned = 0;
    m_ptr->csleep = 0;

    cave[y][x].m_idx = cur_pos;

    /* Update the monster */
    update_mon(cur_pos);

    return TRUE;
}




/*
 * Mega-Hack -- allocation helper
 *
 * Number of monsters with level 0-N 
 */
static int16 r_level[MAX_R_LEV+1];



/*
 * Major Hack -- Initializes r_level array
 * Note that the GHOST is not part of this.
 * But ALL other monsters are, even Morgoth.
 */
static void init_r_level()
{
    register int i;
    static bool done = FALSE;

    /* Only initialize once */    
    if (done) return;

    /* Start with no monsters per level */
    for (i = 0; i <= MAX_R_LEV; i++) r_level[i] = 0;

    /* Count the NORMAL monsters on each level */
    for (i = 0; i < MAX_R_IDX-1; i++) r_level[r_list[i].level]++;

    /* Deduce the monsters on or below each level */
    for (i = 1; i <= MAX_R_LEV; i++) r_level[i] += r_level[i-1];

    /* Only do this once */
    done = TRUE;
}





/*
 * Get a monster race index.  Method 1.
 *
 * Return a monster suitable to be placed at a given level.  This makes high
 * level monsters (up to the given level) slightly more common than low level
 * monsters at any given level.   -CJS- 
 *
 * Code has been added to make it slightly more likely to get the higher level
 * monsters at higher dungeon levels.  Originally a uniform distribution over
 * all monsters of level less than or equal to the dungeon level.  The new
 * distribution makes a level n monster occur approx 2/n% of the time on
 * level n, and 1/n*n% are 1st level.
 *
 * Code has been added to be a little more civilized about monster depths
 * for the first levels -CWS
 *
 * Only two functions (this one and the next, which is almost identical)
 * use the "r_level" array, and they both assume that the "r_list" array
 * is sorted by level, which may or may not actually be true.
 *
 * This version (2.7.0) enforces the "rarity" information for monsters.
 * But note that several functions bypass us and use the race list directly,
 * and they also assume that the list is sorted.
 */
int get_mons_num(int level)
{
    register int i, j, num;

    int          old = level;

    init_r_level();

    while (1) {

	if (level == 0) {
	    i = rand_int(r_level[0]);
	}

	else {

	    if (level > MAX_R_LEV) level = MAX_R_LEV;

	    /* Make a Nasty Monster */
	    if (randint(MON_NASTY) == 1) {
		/* Make low level monsters more likely at low levels */
		i = level / 4 + 1;
		if (i > 4) i = 4;
		level = level + MY_ABS(i) + 1;
		if (level > MAX_R_LEV) level = MAX_R_LEV;
	    }
	    else {
		/* Make high level monsters more likely at high levels */
		num = r_level[level] - r_level[0];
		i = randint(num) - 1;
		j = randint(num) - 1;
		if (j > i) i = j;
		level = r_list[i + r_level[0]].level;
	    }

	    /* Bizarre function */            
	    i = r_level[level] - r_level[level - 1];
	    if (i == 0) i++;
	    i = randint(i) - 1 + r_level[level - 1];
	}

	/* Uniques never appear out of "modified" depth */
	if ((r_list[i].level > old) &&
	    (r_list[i].cflags2 & MF2_UNIQUE)) {
	    continue;
	}

	/* Quest Monsters always appear on their Quest Level */
	if ((r_list[i].level > dun_level) &&
	    (r_list[i].cflags2 & MF2_QUESTOR)) {
	    continue;
	}

	/* The GHOST is NEVER a legal response */
	if (i == MAX_R_IDX - 1) continue;

	/* No rarity */
	if (!r_list[i].rarity) break;

	/* Rarity check passed */
	if (randint(r_list[i].rarity) == 1) break;
    }

    /* Accept the monster */
    return i;
}


/*
 * Get a monster race index.  Method 2.
 */
int get_nmons_num(int level)
{
    register int i, j, num;
    int          old;

    old = level;

    init_r_level();

    while (1) {

	if (level == 0) {
	    i = rand_int(r_level[0]);
	}

	else {

	    if (level > MAX_R_LEV) level = MAX_R_LEV;

	    num = r_level[level] - r_level[0];

	    i = rand_int(num);
	    i += 15;
	    if (i >= num) i = num - 1;

	    j = rand_int(num);
	    if (j > i) i = j;

	    j = rand_int(num);
	    if (j > i) i = j;

	    level = r_list[i + r_level[0]].level;
	    i = r_level[level] - r_level[level - 1];
	    if (i == 0) i = 1;

	    i = randint(i) - 1 + r_level[level - 1];
	}

	if ((r_list[i].level > old) && (r_list[i].cflags2 & MF2_UNIQUE)) {
	    continue;
	}

	if ((r_list[i].level > dun_level) &&
	    (r_list[i].cflags2 & MF2_QUESTOR)) {
	    continue;
	}

	/* The GHOST is NEVER a legal response */
	if (i == MAX_R_IDX - 1) continue;    

	/* No rarity */
	if (!r_list[i].rarity) break;

	/* Rarity check passed */
	if (randint(r_list[i].rarity) == 1) break;
    }

    /* Accept the monster */
    return i;
}


void place_group(int y, int x, int r_idx, int slp)
{
    /* prevent level rating from skyrocketing if they are out of depth... */
    int old = rating;
    int extra = 0;

    /* reduce size of group if out-of-depth */
    if (r_list[r_idx].level > (unsigned) dun_level) {
	extra = (-randint(r_list[r_idx].level - dun_level));
    }

    /* if monster is deeper than normal, then travel in bigger packs -CFT */
    else if (r_list[r_idx].level < (unsigned) dun_level) {
	extra = randint(dun_level - r_list[r_idx].level);
    }

    /* put an upper bounds on it... -CFT */
    if (extra > 12) extra = 12;

    switch (randint(13) + extra) {
      case 25:
	place_monster(y, x - 3, r_idx, 0);
      case 24:
	place_monster(y, x + 3, r_idx, 0);
      case 23:
	place_monster(y - 3, x, r_idx, 0);
      case 22:
	place_monster(y + 3, x, r_idx, 0);
      case 21:
	place_monster(y - 2, x + 1, r_idx, 0);
      case 20:
	place_monster(y + 2, x - 1, r_idx, 0);
      case 19:
	place_monster(y + 2, x + 1, r_idx, 0);
      case 18:
	place_monster(y - 2, x - 1, r_idx, 0);
      case 17:
	place_monster(y + 1, x + 2, r_idx, 0);
      case 16:
	place_monster(y - 1, x - 2, r_idx, 0);
      case 15:
	place_monster(y + 1, x - 2, r_idx, 0);
      case 14:
	place_monster(y - 1, x + 2, r_idx, 0);
      case 13:
	place_monster(y, x - 2, r_idx, 0);
      case 12:
	place_monster(y, x + 2, r_idx, 0);
      case 11:
	place_monster(y + 2, x, r_idx, 0);
      case 10:
	place_monster(y - 2, x, r_idx, 0);
      case 9:
	place_monster(y + 1, x + 1, r_idx, 0);
      case 8:
	place_monster(y + 1, x - 1, r_idx, 0);
      case 7:
	place_monster(y - 1, x - 1, r_idx, 0);
      case 6:
	place_monster(y - 1, x + 1, r_idx, 0);
      case 5:
	place_monster(y, x + 1, r_idx, 0);
      case 4:
	place_monster(y, x - 1, r_idx, 0);
      case 3:
	place_monster(y + 1, x, r_idx, 0);
      case 2:
	place_monster(y - 1, x, r_idx, 0);
	rating = old;
      case 1:
      default:			   /* just in case I screwed up -CFT */
	place_monster(y, x, r_idx, 0);
    }
}


/*
 * Allocates some random monsters   -RAK-	 
 * Place the monsters at least "dis" distance from the player.
 * Use "slp" to choose the initial "sleep" status
 */
void alloc_monster(int num, int dis, int slp)
{
    register int y, x, i;
    int          r_idx;

    for (i = 0; i < num; i++) {

	/* Pick a safe location */
	do {
	    y = randint(cur_height - 2);
	    x = randint(cur_width - 2);
	}
	while ((!floor_grid(y, x)) ||
	       (cave[y][x].m_idx != 0) ||
	       (distance(y, x, char_row, char_col) <= dis));

	/* Get a monster of the given level */
	r_idx = get_mons_num(dun_level);

    /*
     * to give the player a sporting chance, any monster that appears in
     * line-of-sight and can cast spells or breathe, should be asleep. This
     * is an extension of Um55's sleeping dragon code... 
     */

	if (has_breath(r_idx) && los(y, x, char_row, char_col)) {
	    slp = TRUE;
	}

	if (!(r_list[r_idx].cflags2 & MF2_GROUP)) {
	    place_monster(y, x, r_idx, slp);
	}
	else {
	    place_group(y, x, r_idx, slp);
	}
    }
}


/*
 * Places creature adjacent to given location		-RAK-	 
 */
int summon_monster(int *y, int *x, int slp)
{
    register int        i, j, k;
    int                 l, summon;
    register cave_type *c_ptr;

    i = 0;
    summon = FALSE;
    l = get_mons_num(dun_level + MON_SUMMON_ADJ);

    do {
	j = *y - 2 + randint(3);
	k = *x - 2 + randint(3);
	if (in_bounds(j, k)) {
	    c_ptr = &cave[j][k];
	    if (floor_grid(j, k) && (c_ptr->m_idx == 0)) {
		if (r_list[l].cflags2 & MF2_GROUP) {
		    place_group(j, k, l, slp);
		}
		else {
		    place_monster(j, k, l, slp);
		}
		summon = TRUE;
		i = 9;
		*y = j;
		*x = k;
	    }
	}
	i++;
    }
    while (i <= 9);

    return (summon);
}


/*
 * The things we can summon with the function below
 */

#define SUMMON_UNDEAD	11
#define SUMMON_DEMON	12
#define SUMMON_DRAGON	13
#define SUMMON_REPTILE	14
#define SUMMON_SPIDER	15
#define SUMMON_ANGEL	16
#define SUMMON_ANT	17
#define SUMMON_HOUND	18
#define SUMMON_JABBER	19
#define SUMMON_UNIQUE	31
#define SUMMON_WRAITH	32
#define SUMMON_GUNDEAD	51
#define SUMMON_ANCIENTD	52


/*
 * Hack -- in summon_specific() below, if the type is SUMMON_DEMON,
 * do not accept any demon whose level exceeds "summon_level"
 */
static int summon_level;


/*
 * Place a monster (of the specified "type") adjacent to the given
 * location, and re-set the given location to the location of the
 * summoned monster.  Return TRUE iff a monster was actually summoned.
 *
 * We pick random entries in the monster table until we find a "good" one.
 *
 * We will attempt to place the monster up to 10 times before giving up.
 *
 * Note: SUMMON_UNIQUE and SUMMON_WRAITH (XXX) require "Unique-ness"
 * while SUMMON_GUNDEAD and SUMMON_ANCIENTD do not care.
 * All other summons forbid "Unique-ness".
 *
 * This function has to be careful not to summon illegal monsters!
 */
static int summon_specific(int *y, int *x, int type)
{
    register int        i, m;
    bool		okay = FALSE;

    /* Repeat until a monster ("m") is found */
    while (!okay) {

	/* Try a random monster */
	m = rand_int(MAX_R_IDX-1);

	/* Check our requirements */
	switch (type) {

	    case SUMMON_UNDEAD:
		okay = ((r_list[m].cflags2 & MF2_UNDEAD) &&
		    !(r_list[m].cflags2 & MF2_UNIQUE) &&
		    (r_list[m].level < dun_level + 5));
		break;

	    case SUMMON_DEMON:
		okay = ((r_list[m].cflags2 & MF2_DEMON) &&
		    !(r_list[m].cflags2 & MF2_UNIQUE) &&
		    (r_list[m].level <= summon_level));
		break;

	    case SUMMON_DRAGON:
		okay = (r_list[m].cflags2 & MF2_DRAGON &&
		    !(r_list[m].cflags2 & MF2_UNIQUE));
		break;

	    case SUMMON_REPTILE:
		okay = (r_list[m].r_char == 'R' &&
			!(r_list[m].cflags2 & MF2_UNIQUE));
		break;

	    case SUMMON_SPIDER:
		okay = (r_list[m].r_char == 'S' &&
			!(r_list[m].cflags2 & MF2_UNIQUE));
		break;

	    case SUMMON_ANGEL:
		okay = (r_list[m].r_char == 'A' &&
			!(r_list[m].cflags2 & MF2_UNIQUE));
		break;

	    case SUMMON_ANT:
		okay = (r_list[m].r_char == 'a' &&
			!(r_list[m].cflags2 & MF2_UNIQUE));
		break;

	    case SUMMON_HOUND:
		okay = ((r_list[m].r_char == 'C' || r_list[m].r_char == 'Z') &&
			!(r_list[m].cflags2 & MF2_UNIQUE));
		break;

	    case SUMMON_JABBER:
		okay = ((r_list[m].r_char == 'J') && !(r_list[m].cflags2 & MF2_UNIQUE));
		break;

	    case SUMMON_UNIQUE:
		okay = (!(r_list[m].r_char == 'P') &&
			(r_list[m].cflags2 & MF2_UNIQUE));
		break;

	    case SUMMON_WRAITH:
		okay = ((r_list[m].r_char == 'W') &&
			(r_list[m].cflags2 & MF2_UNIQUE));
		break;

	    case SUMMON_GUNDEAD:
		okay = ((r_list[m].r_char == 'L') ||
			(r_list[m].r_char == 'V') ||
			(r_list[m].r_char == 'W'));
		break;

	    case SUMMON_ANCIENTD:
		okay = (r_list[m].r_char == 'D');
		break;

	    default:
		/* Invalid type! */
		return (FALSE);
	}
    }

    /* Try to place it 10 times */
    for (i = 0; i < 10; ++i) {

	register int mx, my;
	my = *y - 2 + randint(3);
	mx = *x - 2 + randint(3);

	if (in_bounds(my, mx)) {

	    register cave_type *c_ptr;
	    c_ptr = &cave[my][mx];
	    if (floor_grid(my, mx) && (c_ptr->m_idx == 0)) {

		/* Place the monster */
		place_monster(my, mx, m, FALSE);

		/* Save the location */
		*y = my;
		*x = mx;

		/* Successful summon */
		return (TRUE);
	    }
	}
    }

    /* Could not place it */
    return (FALSE);
}








/*
 * Summon a demon.  Hack -- enforce max-level 
 */
int summon_demon(int lev, int *y, int *x)
{
    int summon;
    summon_level = lev;
    summon = summon_specific(y, x, SUMMON_DEMON);
    return (summon);
}

/*
 * Summon things (see above)
 */

int summon_undead(int *y, int *x)
{
    int summon = summon_specific(y, x, SUMMON_UNDEAD);
    return (summon);
}

int summon_dragon(int *y, int *x)
{
    int summon = summon_specific(y, x, SUMMON_DRAGON);
    return (summon);
}

int summon_reptile(int *y, int *x)
{
    int summon = summon_specific(y, x, SUMMON_REPTILE);
    return (summon);
}

int summon_spider(int *y, int *x)
{
    int summon = summon_specific(y, x, SUMMON_SPIDER);
    return (summon);
}

int summon_angel(int *y, int *x)
{
    int summon = summon_specific(y, x, SUMMON_ANGEL);
    return (summon);
}

int summon_ant(int *y, int *x)
{
    int summon = summon_specific(y, x, SUMMON_ANT);
    return (summon);
}

int summon_hound(int *y, int *x)
{
    int summon = summon_specific(y, x, SUMMON_HOUND);
    return (summon);
}

int summon_jabberwock(int *y, int *x)
{
    int summon = summon_specific(y, x, SUMMON_JABBER);
    return (summon);
}

int summon_unique(int *y, int *x)
{
    int summon = summon_specific(y, x, SUMMON_UNIQUE);
    return (summon);
}

int summon_wraith(int *y, int *x)
{
    int summon = summon_specific(y, x, SUMMON_WRAITH);
    return (summon);
}

int summon_gundead(int *y, int *x)
{
    int summon = summon_specific(y, x, SUMMON_GUNDEAD);
    return (summon);
}

int summon_ancientd(int *y, int *x)
{
    int summon = summon_specific(y, x, SUMMON_ANCIENTD);
    return (summon);
}




/*
 * Build a string describing a monster in some way.
 *
 * We can correctly describe monsters based on their visibility.
 * We can force all monsters to be treated as visible or invisible.
 * We can build nominatives, objectives, possessives, or reflexives.
 * We can selectively pronominalize hidden, visible, or all monsters.
 * We can use definite or indefinite descriptions for hidden monsters.
 * We can use definite or indefinite descriptions for visible monsters.
 *
 * Pronominalization involves the gender whenever possible and allowed,
 * so that by cleverly requesting pronominalization / visibility, you
 * can get messages like "You hit someone.  She screams in agony!".
 *
 * Reflexives are acquired by requesting Objective plus Possessive.
 *
 * If no m_ptr arg is given (?), the monster is assumed to be hidden,
 * unless the "Assume Visible" mode is requested.
 *
 * If no r_ptr arg is given, it is extracted from m_ptr and r_list
 * If neither m_ptr nor r_ptr is given, the monster is assumed to
 * be neuter, singular, and hidden (unless "Assume Visible" is set),
 * in which case you may be in trouble... :-)
 *
 * I am assuming that no monster name is more than 70 characters long,
 * so that "char desc[80];" is sufficiently large for any result.
 *
 * Mode Flags:
 *   0x01 --> Objective (or Reflexive)
 *   0x02 --> Possessive (or Reflexive)
 *   0x04 --> Use indefinites for hidden monsters ("something")
 *   0x08 --> Use indefinites for visible monsters ("a kobold")
 *   0x10 --> Pronominalize hidden monsters
 *   0x20 --> Pronominalize visible monsters
 *   0x40 --> Assume the monster is hidden
 *   0x80 --> Assume the monster is visible
 *
 * Useful Modes:
 *   0x00 --> Full nominative name ("the kobold") or "it"
 *   0x04 --> Full nominative name ("the kobold") or "something"
 *   0x80 --> Genocide resistance name ("the kobold")
 *   0x88 --> Killing name ("a kobold")
 *   0x22 --> Possessive, genderized if visable ("his") or "its"
 *   0x23 --> Reflexive, genderized if visable (
 */

void
monster_desc (desc, m_ptr, mode)
char               *desc;
monster_type       *m_ptr;
int		   mode;
{
    cptr res;
    monster_race *r_ptr;

    /* Can we "see" it (exists + forced, or visible + not unforced) */
    int seen = m_ptr && ((mode & 0x80) || (!(mode & 0x40) && m_ptr->ml));

    /* Sexed Pronouns (seen and allowed, or unseen and allowed) */
    int pron = m_ptr && ((seen && (mode & 0x20)) || (!seen && (mode & 0x10)));

    /* Extract the monster race */
    r_ptr = &(r_list[m_ptr->r_idx]);


    /* First, try using pronouns, or describing hidden monsters */
    if (!seen || pron) {

	/* an encoding of the monster "sex" */
	int kind;

	/* Extract the gender flag */
	if (!m_ptr || !pron) kind = 0x00;
	else if (r_ptr->cflags1 & MF1_PLURAL) kind = 0x30;
	else if (r_ptr->cflags1 & MF1_FEMALE) kind = 0x20;
	else if (r_ptr->cflags1 & MF1_MALE) kind = 0x10;
	else kind = 0x00;


	/* Assume simple result */
	res = "it";

	/* Brute force: split on the possibilities */
	switch (kind + (mode & 0x07)) {

	    /* Neuter, or unknown */
	    case 0x00: res = "it"; break;
	    case 0x01: res = "it"; break;
	    case 0x02: res = "its"; break;
	    case 0x03: res = "itself"; break;
	    case 0x04: res = "something"; break;
	    case 0x05: res = "something"; break;
	    case 0x06: res = "something's"; break;
	    case 0x07: res = "itself"; break;

	    /* Male (assume human if vague) */
	    case 0x10: res = "he"; break;
	    case 0x11: res = "him"; break;
	    case 0x12: res = "his"; break;
	    case 0x13: res = "himself"; break;
	    case 0x14: res = "someone"; break;
	    case 0x15: res = "someone"; break;
	    case 0x16: res = "someone's"; break;
	    case 0x17: res = "himself"; break;

	    /* Female (assume human if vague) */
	    case 0x20: res = "she"; break;
	    case 0x21: res = "her"; break;
	    case 0x22: res = "her"; break;
	    case 0x23: res = "herself"; break;
	    case 0x24: res = "someone"; break;
	    case 0x25: res = "someone"; break;
	    case 0x26: res = "someone's"; break;
	    case 0x27: res = "herself"; break;

	    /* Plural (assume neuter if vague) */    
	    case 0x30: res = "they"; break;
	    case 0x31: res = "them"; break;
	    case 0x32: res = "their"; break;
	    case 0x33: res = "themselves"; break;
	    case 0x34: res = "some things"; break;
	    case 0x35: res = "some things"; break;
	    case 0x36: res = "some things's"; break;
	    case 0x37: res = "themselves"; break;
	}

	/* Copy the result */
	(void)strcpy(desc, res);
    }


    /* Handle visible monsters, "reflexive" request */
    else if ((mode & 0x02) && (mode & 0x01)) {

	/* The monster is visible, so use its gender */
	if (r_ptr->cflags1 & MF1_PLURAL) (void)strcpy(desc, "themselves");
	else if (r_ptr->cflags1 & MF1_FEMALE) (void)strcpy(desc, "herself");
	else if (r_ptr->cflags1 & MF1_MALE) (void)strcpy(desc, "himself");
	else (void)strcpy(desc, "itself");
    }


    /* Handle all other visible monster requests */
    else {

	/* It could be a Unique */
	if (r_ptr->cflags2 & MF2_UNIQUE) {

	    /* Start with the name (thus nominative and objective) */
	    (void)strcpy(desc, r_ptr->name);
	}

	/* It could be an indefinite monster */
	else if (mode & 0x08) {

	    /* XXX Check plurality for "some" */

	    /* Indefinite monsters need an indefinite article */
	    (void)strcpy(desc, is_a_vowel(r_ptr->name[0]) ? "an " : "a ");
	    (void)strcat(desc, r_ptr->name);
	}

	/* It could be a normal, definite, monster */
	else {

	    /* Definite monsters need a definite article */
	    (void)strcpy(desc, "the ");
	    (void)strcat(desc, r_ptr->name);
	}

	/* Handle the Possessive as a special afterthought */
	if (mode & 0x02) {

	    /* XXX Check for trailing "s" */

	    /* Simply append "apostrophe" and "s" */
	    (void)strcat(desc, "'s");
	}
    }
}



