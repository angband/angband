/* File: moria3.c */ 

/* Purpose: high level command processing */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "angband.h"




/*
 * An enhanced look, with peripheral vision. Looking all 8	-CJS-
 * directions will see everything which ought to be visible.
 *
 * Can specify direction 5, which looks in all directions. 
 *
 * For the purpose of hindering vision, each place is regarded as a diamond just
 * touching its four immediate neighbours. A diamond is opaque if it is a
 * wall, or shut door, or something like that. A place is visible if any part
 * of its diamond is visible: i.e. there is a line from the view point to
 * part of the diamond which does not pass through any opaque diamonds. 
 *
 * Consider the following situation: 
 *
 * @....			    X	X   X	X   X .##..			   /
 * \ / \ / \ / \ / \ .....			  X @ X . X . X 1 X . X \ / \
 * / \ / \ / \ / X	X   X	X   X Expanded view, with	   / \ / \ /
 * \ / \ / \ diamonds inscribed	  X . X # X # X 2 X . X about each point,	  
 * \ / \ / \ / \ / \ / and some locations	    X	X   X	X   X
 * numbered.		   / \ / \ / \ / \ / \ X . X . X . X 3 X 4 X \ / \ /
 * \ / \ / \ / X	X   X	X   X - Location 1 is fully visible. -
 * Location 2 is visible, even though partially obscured. - Location 3 is
 * invisible, but if either # were transparent, it would be visible. -
 * Location 4 is completely obscured by a single #. 
 *
 * The function which does the work is look_ray. It sets up its own co-ordinate
 * frame (global variables map back to the dungeon frame) and looks for
 * everything between two angles specified from a central line. It is
 * recursive, and each call looks at stuff visible along a line parallel to
 * the center line, and a set distance away from it. A diagonal look uses
 * more extreme peripheral vision from the closest horizontal and vertical
 * directions; horizontal or vertical looks take a call for each side of the
 * central line. 
 */


/*
 * Globally accessed variables: gl_nseen counts the number of places where
 * something is seen. gl_rock indicates a look for rock or objects. 
 *
 * The others map co-ords in the ray frame to dungeon co-ords. 
 *
 * dungeon y = char_row	 + gl_fyx * (ray x)  + gl_fyy * (ray y)
 * dungeon x = char_col	 + gl_fxx * (ray x)  + gl_fxy * (ray y) 
 */
static int gl_fxx, gl_fxy, gl_fyx, gl_fyy;
static int gl_nseen, gl_noquery;
static int gl_rock;

/*
 * Intended to be indexed by dir/2, since is only relevant to horizontal or
 * vertical directions. 
 */
static int set_fxy[] = {0, 1, 0, 0, -1};
static int set_fxx[] = {0, 0, -1, 1, 0};
static int set_fyy[] = {0, 0, 1, -1, 0};
static int set_fyx[] = {0, 1, 0, 0, -1};

/*
 * Map diagonal-dir/2 to a normal-dir/2. 
 */
static int map_diag1[] = {1, 3, 0, 2, 4};
static int map_diag2[] = {2, 1, 0, 4, 3};

/*
 * Any sufficiently big number will do
 */
#define GRADF	10000



/*
 * Look at a monster
 */
static cptr look_mon_desc(int mnum)
{
    monster_type *m_ptr = &m_list[mnum];
    monster_race *r_ptr = &r_list[m_ptr->r_idx];

    bool          living = TRUE;
    int           perc;


    /* Determine if the monster is "living" (vs "undead") */
    if (r_ptr->cflags2 & MF2_UNDEAD) living = FALSE;
    if (r_ptr->cflags2 & MF2_DEMON) living = FALSE;    
    if (strchr("EgvX", r_ptr->r_char)) living = FALSE;    


    /* Healthy monsters */
    if (m_ptr->hp >= m_ptr->maxhp) {

	/* Paranoia */
	m_ptr->hp = m_ptr->maxhp;

	/* No damage */
	return (living ? "unhurt" : "undamaged");
    }


    /* Notice the "long arithmetic" */
    perc = (100L * m_ptr->hp) / m_ptr->maxhp;

    if (perc > 60) {
	return (living ? "somewhat wounded" : "somewhat damaged");
    }

    if (perc > 25) {
	return (living ? "wounded" : "damaged");
    }

    if (perc > 10) {
	return (living ? "badly wounded" : "badly damaged");
    }

    return (living ? "almost dead" : "almost destroyed");
}




static int look_see(int x, int y, int *transparent)
{
    const char         *dstring,*string;
    char               query = 'a';
    register cave_type *c_ptr;
    register int        i, j;
    bigvtype            out_val, tmp_str;
    monster_type	*m_ptr;
    inven_type		*i_ptr;
    char		m_name[80];

    /* Assume not transparent */
    *transparent = FALSE;

    /* Paranoia */    
    if (x < 0 || y < 0 || y > x) {
	(void)sprintf(tmp_str, "Illegal call to look_see(%d, %d)", x, y);
	msg_print(tmp_str);
	return FALSE;
    }

    /* Default to looking at an object */
    dstring = "You see";

    /* Looking under the player */
    if (!x && !y) dstring = "You are on";

    /* Something */
    j = char_col + gl_fxx * x + gl_fxy * y;
    i = char_row + gl_fyx * x + gl_fyy * y;
    x = j;
    y = i;

    /* Off screen, stop looking, nothing to see */
    if (!panel_contains(y, x)) {
	return FALSE;
    }

    /* Get the cave */
    c_ptr = &cave[y][x];

    /* Get the monster */
    m_ptr = &m_list[c_ptr->m_idx];

    /* Floor grids are transparent */
    if (floor_grid_bold(y,x)) *transparent = TRUE;

    /* Hack -- Don't look at a direct line of sight. */
    if (gl_noquery) return FALSE;

    /* Start the description */        
    out_val[0] = 0;

    /* Examine visible monsters */
    if (gl_rock == 0 && c_ptr->m_idx > 1 && m_list[c_ptr->m_idx].ml) {

	j = m_list[c_ptr->m_idx].r_idx;

	/* Get the monster name ("a kobold") */
	monster_desc(m_name, &m_list[c_ptr->m_idx], 0x08);

	/* Auto-recall */
	if (use_recall_win && term_recall) {

	    /* Describe */
	    (void)sprintf(out_val, "%s %s (%s).  --pause--",
			  dstring, m_name, look_mon_desc(c_ptr->m_idx));

	    /* Auto-recall */
	    roff_recall(j);
	}

	/* Use prompt */
	else {

	    /* Describe, and prompt for recall */
	    (void)sprintf(out_val, "%s %s (%s) [(r)ecall]",
			  dstring, m_name, look_mon_desc(c_ptr->m_idx));
	}


	prt(out_val, 0, 0);
	move_cursor_relative(y, x);
	query = inkey();


	/* Use a recall window */
	if (!(use_recall_win && term_recall)) {
	    if (query == 'r' || query == 'R') {
		erase_line(0,0);
		save_screen();
		query = roff_recall(j);
		restore_screen();
	    }
	}


	/* Now look "under" the monster */
	if (r_list[m_ptr->r_idx].cflags1 & MF1_PLURAL) dstring = "They are on";
	else if (r_list[m_ptr->r_idx].cflags1 & MF1_FEMALE) dstring = "She is on";
	else if (r_list[m_ptr->r_idx].cflags1 & MF1_MALE) dstring = "He is on";
	else dstring = "It is on";
    }


    /* Check for illumination */
    if (test_lite(y, x)) {

	/* Is there an object there? */
	if (c_ptr->i_idx) {

	    i_ptr = &i_list[c_ptr->i_idx];

	    /* No rock, yes visible object */
	    if (!gl_rock &&
		(i_ptr->tval != TV_INVIS_TRAP) &&
		(i_ptr->tval != TV_SECRET_DOOR)) {
		
		objdes(tmp_str, &i_list[c_ptr->i_idx], TRUE);
		(void)sprintf(out_val, "%s %s.  ---pause---", dstring, tmp_str);
		prt(out_val, 0, 0);
		move_cursor_relative(y, x);
		query = inkey();
		dstring = "It is in";
	    }
	}

	/* Examine rocks.  Note that "granite" is only seen if non-empty. */
	/* This is important, because "secret doors" can then pretend to */
	/* be "empty" granite walls (and thus have no description) */
	if ((gl_rock || out_val[0]) && (c_ptr->fval >= MIN_WALL)) {

	    switch (c_ptr->fval) {
	      case MAGMA_WALL:
		string = "some dark rock";
		break;
	      case QUARTZ_WALL:
		string = "a quartz vein";
		break;
	      default:
		string = "a granite wall";
		if (!out_val[0]) string = NULL;
		break;
	    }

	    if (string) {
		(void)sprintf(out_val, "%s %s.  ---pause---", dstring, string);
		prt(out_val, 0, 0);
		move_cursor_relative(y, x);
		query = inkey();
	    }
	}
    }

    if (out_val[0]) {
	gl_nseen++;
	if (query == ESCAPE) return TRUE;
    }

    return FALSE;
}


/*
 * Look at everything within a cone of vision between two ray
 * lines emanating from the player, and y or more places away
 * from the direct line of view. This is recursive.
 *
 * Rays are specified by gradients, y over x, multiplied by
 * 2*GRADF. This is ONLY called with gradients between 2*GRADF
 * (45 degrees) and 1 (almost horizontal).
 *
 * (y axis)/ angle from
 *   ^	  /	    ___ angle to
 *   |	 /	 ___
 *...|../.....___.................... parameter y (look at things in the
 *   | /   ___			      cone, and on or above this line)
 *   |/ ___
 *   @-------------------->   direction in which you are looking. (x axis)
 *   |
 *   | 
 */
static int look_ray(int y, int from, int to)
{
    register int        max_x, x;
    int                 transparent;

/*
 * from is the larger angle of the ray (larger than "to") since
 * we scan towards the center line.  If "from" is smaller than "to",
 * the ray does not exist
 */

    if (from <= to || y > MAX_SIGHT) return FALSE;

/*
 * Find first visible location along this line. Minimum x such that (2x-1)/x
 * < from/GRADF <=> x > GRADF(2x-1)/from. This may be called with y=0 whence
 * x will be set to 0. Thus we need a special fix. 
 */

    x = (long)GRADF * (2 * y - 1) / from + 1;

    if (x <= 0) x = 1;

/*
 * Find last visible location along this line. Maximum x such that (2x+1)/x >
 * to/GRADF <=> x < GRADF(2x+1)/to 
 */

    max_x = ((long)GRADF * (2 * y + 1) - 1) / to;
    if (max_x > MAX_SIGHT) max_x = MAX_SIGHT;
    if (max_x < x) return FALSE;

/*
 * Hack -- gl_noquery prevents doubling up on direct lines of sight. If
 * 'to' is greater than 1, we do not really look at stuff along the
 * direct line of sight, but we do have to see what is opaque for the
 * purposes of obscuring other objects. 
 */

    if ((y == 0 && to > 1) || (y == x && from < GRADF * 2)) {
	gl_noquery = TRUE;
    }
    else {
	gl_noquery = FALSE;
    }

    if (look_see(x, y, &transparent)) {
	return TRUE;
    }

    if (y == x) {
	gl_noquery = FALSE;
    }

    /* Hack */    
    if (transparent) goto init_transparent;

    /* Go until done */
    for (;;) {

	/* Look down the window we've found, allow abort */
	if (look_ray(y + 1, from, (int)((2 * y + 1) * (long)GRADF / x))) {
	    return TRUE;
	}

	/* Find the start of next window. */
	do {

	    /* All done (?) */
	    if (x == max_x) return FALSE;

	    /* See if this seals off the scan. (If y is zero, then it will.) */
	    from = (2 * y - 1) * (long)GRADF / x;
	    if (from <= to) return FALSE;

	    x++;

	    if (look_see(x, y, &transparent)) {
		return TRUE;
	    }
	}
	while (!transparent);

init_transparent:

	/* Find the end of this window of visibility. */
	do {
	    if (x == max_x) {
	    /* The window is trimmed by an earlier limit. */
		return look_ray(y + 1, from, to);
	    }

	    x++;

	    if (look_see(x, y, &transparent)) {
		return TRUE;
	    }
	}
	while (transparent);
    }
}



/*
 * Look at what we can see. This is a free move. 
 *
 * Prompts for a direction, and then looks at every object in turn within a cone
 * of vision in that direction. For each object, the cursor is moved over the
 * object, a description is given, and we wait for the user to type
 * something. Typing ESCAPE will abort the entire look. 
 *
 * Looks first at real objects and monsters, and looks at rock types only after
 * all other things have been seen.  Only looks at rock types if the
 * notice_seams option is set. 
 */
void do_cmd_look()
{
    register int        i, abort_look;
    int                 dir, dummy;

    /* This is a free move */
    free_turn_flag = TRUE;

    /* Blind */
    if (p_ptr->blind > 0) {
	msg_print("You can't see a damn thing!");
    }

    /* Hallucinating */
    else if (p_ptr->image > 0) {
	msg_print("You can't believe what you are seeing! It's like a dream!");
    }

    /* Get a direction (or "5"), ignoring target and confusion */
    else if (get_a_dir("Look which direction? ", &dir, 0x04)) {

	abort_look = FALSE;
	gl_nseen = 0;
	gl_rock = 0;
	gl_noquery = FALSE;	   /* Have to set this up for the look_see */

	if (look_see(0, 0, &dummy)) {
	    abort_look = TRUE;
	}
	else {
	    do {
		abort_look = FALSE;

		/* Look at everything */
		if (dir == 5) {

		    for (i = 1; i <= 4; i++) {
			gl_fxx = set_fxx[i];
			gl_fyx = set_fyx[i];
			gl_fxy = set_fxy[i];
			gl_fyy = set_fyy[i];
			if (look_ray(0, 2 * GRADF - 1, 1)) {
			    abort_look = TRUE;
			    break;
			}
			gl_fxy = (-gl_fxy);
			gl_fyy = (-gl_fyy);
			if (look_ray(0, 2 * GRADF, 2)) {
			    abort_look = TRUE;
			    break;
			}
		    }
		}

		/* Straight directions */
		else if ((dir & 1) == 0) {
		    i = dir >> 1;
		    gl_fxx = set_fxx[i];
		    gl_fyx = set_fyx[i];
		    gl_fxy = set_fxy[i];
		    gl_fyy = set_fyy[i];
		    if (look_ray(0, GRADF, 1)) {
			abort_look = TRUE;
		    }
		    else {
			gl_fxy = (-gl_fxy);
			gl_fyy = (-gl_fyy);
			abort_look = look_ray(0, GRADF, 2);
		    }
		}

		/* Diagonals */
		else {
		    i = map_diag1[dir >> 1];
		    gl_fxx = set_fxx[i];
		    gl_fyx = set_fyx[i];
		    gl_fxy = (-set_fxy[i]);
		    gl_fyy = (-set_fyy[i]);
		    if (look_ray(1, 2 * GRADF, GRADF)) {
			abort_look = TRUE;
		    }
		    else {
			i = map_diag2[dir >> 1];
			gl_fxx = set_fxx[i];
			gl_fyx = set_fyx[i];
			gl_fxy = set_fxy[i];
			gl_fyy = set_fyy[i];
			abort_look = look_ray(1, 2 * GRADF - 1, GRADF);
		    }
		}
	    }
	    while (abort_look == FALSE && notice_seams && (++gl_rock < 2));

	    if (abort_look) {
		msg_print("Aborting look.");
	    }
	    else {
		if (gl_nseen) {
		    if (dir == 5) {
			msg_print("That's all you see.");
		    }
		    else {
			msg_print("That's all you see in that direction.");
		    }
		}
		else if (dir == 5) {
		    msg_print("You see nothing of interest.");
		}
		else {
		    msg_print("You see nothing of interest in that direction.");
		}
	    }
	}
    }
}





/*
 * Support code for the "Locate ourself on the Map" command
 */
void do_cmd_view_map()
{
    /* Free move */
    free_turn_flag = TRUE;

    /* Are we blind? */
    if ((p_ptr->blind > 0) || no_lite()) {
	msg_print("You can't see your map.");
	return;
    }

    /* Look at the map */
    screen_map();

    /* We should probably get a key, but screen_map does it */
}


/*
 * Given an row (y) and col (x), recenter the "panel".
 * The map is reprinted if necessary, and "TRUE" is returned.
 */
static bool do_cmd_locate_aux(int y, int x)
{
    int prow = panel_row;
    int pcol = panel_col;

    prow = ((y - SCREEN_HEIGHT / 4) / (SCREEN_HEIGHT / 2));
    if (prow > max_panel_rows) prow = max_panel_rows;
    else if (prow < 0) prow = 0;

    pcol = ((x - SCREEN_WIDTH / 4) / (SCREEN_WIDTH / 2));
    if (pcol > max_panel_cols) pcol = max_panel_cols;
    else if (pcol < 0) pcol = 0;

    /* Check for "no change" */
    if ((prow == panel_row) && (pcol == panel_col)) return (FALSE);

    /* Save the new panel info */
    panel_row = prow;
    panel_col = pcol;

    /* Recalculate the boundaries */
    panel_bounds();

    /* Redraw the map */
    prt_map();

    /* The map was redrawn */
    return (TRUE);
}



/*
 * Support code for the "Locate ourself on the Map" command
 */

void do_cmd_locate()
{
    int dir_val;
    int y, x;
    vtype out_val;
    vtype tmp_str;

    int cy, cx, p_y, p_x;


    /* Free move */
    free_turn_flag = TRUE;

    /* Are we blind?  Ignore "no_lite()" */
    if (p_ptr->blind > 0) {
	msg_print("You can't see your map.");
	return;
    }


    /* Save character location */
    y = char_row;
    x = char_col;

    /* Move to a new panel */
    (void)do_cmd_locate_aux(y, x);

    /* Extract (original) panel info */
    cy = panel_row;
    cx = panel_col;


    /* Show panels until done */
    while (1) {

	p_y = panel_row;
	p_x = panel_col;

	/* Describe the location */
	if ((p_y == cy) && (p_x == cx)) {
	    tmp_str[0] = '\0';
	}
	else {
	    (void)sprintf(tmp_str, "%s%s of",
		(p_y < cy) ? " North" : (p_y > cy) ? " South" : "",
		(p_x < cx) ? " West" : (p_x > cx) ? " East" : "");
	}


	/* Prepare to ask which way to look */
	(void)sprintf(out_val,
	    "Map sector [%d,%d], which is%s your sector. Look which direction?",
	    p_y, p_x, tmp_str);

	/* Get a direction (or Escape) */
	if (!get_a_dir(out_val, &dir_val, 0)) break;


	/* Keep "moving" until the panel changes */
	while (1) {

	    /* Apply the direction */
	    x += ((dir_val - 1) % 3 - 1) * SCREEN_WIDTH / 2;
	    y -= ((dir_val - 1) / 3 - 1) * SCREEN_HEIGHT / 2;

	    /* No motion off map */
	    if (x < 0 || y < 0 || x >= cur_width || y >= cur_width) {
		msg_print("You've gone past the end of your map.");
		x -= ((dir_val - 1) % 3 - 1) * SCREEN_WIDTH / 2;
		y += ((dir_val - 1) / 3 - 1) * SCREEN_HEIGHT / 2;
		break;
	    }

	    /* Hack -- keep sliding until done (?) */
	    if (do_cmd_locate_aux(y, x)) break;
	}
    }


    /* Paranoia -- make sure the screen is okay */

    /* Get new panel, redraw the map */
    (void)get_panel(char_row, char_col, FALSE);

    /* Update the view/lite */
    update_view();
    update_lite(); 

    /* Update the monsters */
    update_monsters(); 

    /* Check the view */
    check_view();
}




/*
 * Allocates objects upon opening a chest    -BEN-
 *
 * Disperse treasures from the chest "i_ptr", centered at (x,y).
 * Adapted from "monster_death()", but much simpler...
 */
static void chest_death(int y, int x, inven_type *i_ptr)
{
    int			i, y1, x1, number;

    bool	do_item = (i_ptr->flags1 & CH1_CARRY_OBJ) ? TRUE : FALSE;
    bool	do_gold = (i_ptr->flags1 & CH1_CARRY_GOLD) ? TRUE : FALSE;


    /* Must be a chest */
    if (i_ptr->tval != TV_CHEST) return;

    /* Count how many objects */
    number = 0;
    if ((i_ptr->flags1 & CH1_HAS_60) && (randint(100) < 60)) number++;
    if ((i_ptr->flags1 & CH1_HAS_90) && (randint(100) < 90)) number++;
    if (i_ptr->flags1 & CH1_HAS_1D2) number += randint(2);
    if (i_ptr->flags1 & CH1_HAS_2D2) number += damroll(2, 2);
    if (i_ptr->flags1 & CH1_HAS_4D2) number += damroll(4, 2);

    /* Summon some objects */
    if (number > 0) {

	/* Drop some objects (non-chests) */    
	for ( ; number > 0; --number) {

	    /* Try 20 times per item */
	    for (i = 0; i < 20; ++i) {

		/* Pick a location */
		y1 = rand_spread(y, 2);
		x1 = rand_spread(x, 2);

		/* Must be a legal grid */
		if (!in_bounds(y1, x1)) continue;
		
		/* Must be a clean floor grid */
		if (!clean_grid_bold(y1, x1)) continue;

		/* Must be legal, must be visible */
		if (!los(y, x, y1, x1)) continue;

		/* Opening a chest */
		opening_chest = TRUE;

		/* The "pval" of a chest is how "good" it is */
		object_level = i_ptr->pval;

		/* Place an Item or Gold */
		if (do_gold && (randint(2) == 1)) {
		    place_gold(y1, x1);
		}
		else if (do_item) {
		    place_object(y1, x1);
		}
		else if (do_gold) {
		    place_gold(y1, x1);
		}

		/* Reset the object level */
		object_level = dun_level;
		
		/* No longer opening a chest */
		opening_chest = FALSE;

		/* Actually display the object's grid */
		lite_spot(y1, x1);

		/* Successful placement */
		break;
	    }
	}
    }

    /* The chest is now identified */
    known2(i_ptr);

    /* The chest is "dead" */
    i_ptr->cost = 0L;
    i_ptr->flags1 = 0L;
    i_ptr->flags2 = 0L;
    i_ptr->pval = 0;
}


/*
 * Chests have traps too.
 * Note: Chests now use "flags2" for their traps
 * Exploding chest destroys contents, and traps.
 * Note that the chest itself is never destroyed.
 */
static void chest_trap(int y, int x, inven_type *i_ptr)
{
    register int        i, j, k;

    if (i_ptr->tval != TV_CHEST) return;

    if (i_ptr->flags2 & CH2_LOSE_STR) {
	msg_print("A small needle has pricked you!");
	if (!p_ptr->sustain_str) {
	    (void)dec_stat(A_STR, 10, FALSE);
	    take_hit(damroll(1, 4), "a poison needle");
	    msg_print("You feel weakened!");
	}
	else {
	    msg_print("You are unaffected.");
	}
    }

    if (i_ptr->flags2 & CH2_POISON) {
	msg_print("A small needle has pricked you!");
	take_hit(damroll(1, 6), "a poison needle");
	if (!(p_ptr->resist_pois ||
	      p_ptr->oppose_pois ||
	      p_ptr->immune_pois)) {
	    p_ptr->poisoned += 10 + randint(20);
	}
    }

    if (i_ptr->flags2 & CH2_PARALYSED) {
	msg_print("A puff of yellow gas surrounds you!");
	if (p_ptr->free_act) {
	    msg_print("You are unaffected.");
	}
	else {
	    msg_print("You choke and pass out.");
	    p_ptr->paralysis = 10 + randint(20);
	}
    }

    if (i_ptr->flags2 & CH2_SUMMON) {
	for (i = 0; i < 3; i++) {
	    j = y;
	    k = x;
	    (void)summon_monster(&j, &k, FALSE);
	}
    }

    if (i_ptr->flags2 & CH2_EXPLODE) {
	msg_print("There is a sudden explosion!");
	msg_print("Everything inside the chest is destroyed!");
	i_ptr->flags1 = 0L;
	i_ptr->flags2 = 0L;
	take_hit(damroll(5, 8), "an exploding chest");
    }
}





/*
 * Opens a closed door or closed chest.		-RAK-
 * Note that failed opens take time, or ghosts could be found
 * Note unlocking a door is worth one XP, and unlocking a chest
 * is worth as many XP as the chest had "levels".
 */
void do_cmd_open()
{
    int				y, x, i, dir;
    int				flag;
    register cave_type		*c_ptr;
    register inven_type		*i_ptr;
    char			m_name[80];


    /* Assume we will not continue repeating this command */
    int more = FALSE;

    /* Get a direction (or Escape) */
    if (!get_a_dir(NULL, &command_dir, 0)) {
	/* Graceful exit */
	free_turn_flag = TRUE;
    }

    else {

	/* Apply partial confusion */
	dir = command_dir;
	confuse_dir(&dir, 0x02);

	/* Get requested grid */
	y = char_row;
	x = char_col;
	(void)mmove(dir, &y, &x);
	c_ptr = &cave[y][x];

	/* Get the object (if it exists) */
	i_ptr = &i_list[c_ptr->i_idx];

	/* Nothing is there */
	if ((c_ptr->i_idx == 0) ||
	    ((i_ptr->tval != TV_CLOSED_DOOR) &&
	     (i_ptr->tval != TV_CHEST))) {
	    msg_print("I do not see anything you can open there.");
	    free_turn_flag = TRUE;
	}

	/* Monster in the way */
	else if (c_ptr->m_idx > 1) {

	    /* Acquire "Monster" (or "Something") */
	    monster_desc(m_name, &m_list[c_ptr->m_idx], 0x04);
	    message(m_name, 0x03);
	    message(" is in your way!", 0);
	}

	/* Closed door */
	else if (i_ptr->tval == TV_CLOSED_DOOR) {

	    /* Stuck */
	    if (i_ptr->pval < 0) {
		msg_print("It appears to be stuck.");
	    }

	    /* Locked */
	    else if (i_ptr->pval > 0) {

		i = p_ptr->disarm + 2 * todis_adj() + stat_adj(A_INT)
		    + (class_level_adj[p_ptr->pclass][CLA_DISARM]
		       * p_ptr->lev / 3);

		/* give a 1/50 chance of opening anything, anyway -CWS */
		if ((i - i_ptr->pval) < 2) i = i_ptr->pval + 2;

		if (p_ptr->confused > 0) {
		    msg_print("You are too confused to pick the lock.");
		}
		else if ((i - i_ptr->pval) > randint(100)) {
		    msg_print("You have picked the lock.");
		    p_ptr->exp++;
		    prt_experience();
		    i_ptr->pval = 0;
		}
		else {
		    /* We may keep trying */
		    more = TRUE;
		    msg_print("You failed to pick the lock.");
		}
	    }

	    /* In any case, if the door is unlocked, open it */
	    if (i_ptr->pval == 0) {

		invcopy(i_ptr, OBJ_OPEN_DOOR);
		i_ptr->iy = y;
		i_ptr->ix = x;

		/* The door is in a "corridor" */
		c_ptr->fval = CORR_FLOOR;

		/* Draw the door */
		lite_spot(y, x);

		/* A "blockage" is now gone */
		forget_lite();
		update_view();
		update_lite();

		/* Monsters may have appeared */
		update_monsters();

		/* Check the view */
		check_view();
	    }
	}

	/* Open a closed chest. */
	else if (i_ptr->tval == TV_CHEST) {

	    i = p_ptr->disarm + 2 * todis_adj() + stat_adj(A_INT) +
		(class_level_adj[p_ptr->pclass][CLA_DISARM] *
		p_ptr->lev / 3);

	    /* give a 1/50 chance of opening anything, anyway -CWS */
	    if ((i - i_ptr->pval) < 2) i = i_ptr->pval + 2;

	    /* Assume opened successfully */
	    flag = TRUE;

	    /* Attempt to unlock it */
	    if (i_ptr->flags2 & CH2_LOCKED) {

		/* Assume locked, and thus not open */
		flag = FALSE;

		/* Too confused */
		if (p_ptr->confused > 0) {
		    msg_print("You are too confused to pick the lock.");
		}

		/* Pick the lock, leave the traps */
		else if ((i - i_ptr->pval) > randint(100)) {
		    msg_print("You have picked the lock.");
		    i_ptr->flags2 &= ~CH2_LOCKED;
		    p_ptr->exp += i_ptr->pval;
		    prt_experience();
		    flag = TRUE;
		}

		/* Keep trying */
		else {
		    /* We may continue repeating */
		    more = TRUE;
		    msg_print("You failed to pick the lock.");
		}
	    }

	    /* Allowed to open */
	    if (flag) {

		/* Apply chest traps, if any */
		if (i_ptr->flags2) chest_trap(y, x, i_ptr);

		/* Let the Chest drop items */
		chest_death(y, x, i_ptr);
	    }
	}
    }

    /* Cancel repeat unless we may continue */
    if (!more) command_rep = 0;
}


/*
 * Close an open door.
 */
void do_cmd_close()
{
    int                    y, x, dir;
    vtype                  m_name;
    register cave_type    *c_ptr;
    inven_type		  *i_ptr;

    /* Get a "desired" direction, or Abort */
    if (!get_a_dir(NULL, &command_dir, 0)) {
	/* Abort gracefully */
	free_turn_flag = TRUE;
    }

    else {

	/* Apply partial confusion */
	dir = command_dir;
	confuse_dir(&dir, 0x02);

	y = char_row;
	x = char_col;
	(void)mmove(dir, &y, &x);

	c_ptr = &cave[y][x];
	i_ptr = &i_list[c_ptr->i_idx];


	if ((c_ptr->i_idx == 0) ||
	    (i_ptr->tval != TV_OPEN_DOOR)) {

	    msg_print("I do not see anything you can close there.");
	    free_turn_flag = TRUE;
	}

	/* Handle broken doors */
	else if (i_ptr->pval) {
	    msg_print("The door appears to be broken.");
	    free_turn_flag = TRUE;
	}

	/* Monster in the way */
	else if (c_ptr->m_idx > 1) {
	    /* Acquire "Monster" (or "Something") */
	    monster_desc(m_name, &m_list[c_ptr->m_idx], 0x04);
	    message(m_name, 0x03);
	    message(" is in your way!", 0);
	}

	/* Close it */
	else {

	    /* Hack -- kill the old object */
	    i_ptr = &i_list[c_ptr->i_idx];
	    invcopy(i_ptr, OBJ_CLOSED_DOOR);

	    /* Place it in the dungeon */
	    i_ptr->iy = y;
	    i_ptr->ix = x;

	    /* Redisplay */
	    lite_spot(y, x);

	    /* Update the view */
	    update_view();
	    update_lite();
	    update_monsters();
	}
    }
}


/*
 * Tunneling through wall
 * Used by TUNNEL and WALL_TO_MUD
 * Both those routines do "update_view()" for us.				
 */
int twall(int y, int x, int t1, int t2)
{
    int		i, j;
    int		res, found;
    cave_type	*c_ptr;

    res = FALSE;

    /* Allow chaining of "probability" calls */
    if (t1 > t2) {

	c_ptr = &cave[y][x];


	/* Forget the "field mark" and "perma-lite" */
	c_ptr->info &= ~CAVE_PL;
	c_ptr->info &= ~CAVE_FM;


	/* Set to TRUE when we have acquired a "floor type" */
	found = FALSE;

	/* Hack -- Now make the grid a floor space */
	if (c_ptr->info & CAVE_LR) {

	    /* Try to "extend" rooms (prevents teleportation into vaults) */
	    for (i = y - 1; !found && (i <= y + 1); i++) {
		for (j = x - 1; !found && (j <= x + 1); j++) {

		    /* Notice when we are touching a "room floor" */
		    if (floor_grid_bold(i, j) &&
			(cave[i][j].fval != CORR_FLOOR)) {

			/* Steal the floor type */
			c_ptr->fval = cave[i][j].fval;

			/* XXX XXX Hack -- Obtain perma-lite from that room */
			if (cave[i][j].info & CAVE_PL) c_ptr->info |= CAVE_PL;

			/* Double break; */
			found = TRUE;
		    }
		}
	    }
	}


	/* Otherwise, make it a corridor */
	if (!found) {
	    c_ptr->fval = CORR_FLOOR;
	}


	/* Redisplay the grid */
	lite_spot(y, x);

	/* Worked */
	res = TRUE;
    }

    return (res);
}


/*
 * Tunnels through rubble and walls			-RAK-
 * Must take into account: secret doors,  special tools
 *
 * Note that tunneling almost always takes time, since otherwise
 * you can use tunnelling to find monsters.  Also note that you
 * must tunnel in order to hit monsters in walls or on closed doors.
 */
void do_cmd_tunnel()
{
    register int        i, tabil;
    register cave_type *c_ptr;
    register inven_type *i_ptr;
    int                 y, x, dir;
    char		m_name[80];

    /* Assume we cannot continue */
    int more = FALSE;

    /* Get a direction to tunnel, or Abort */
    if (!get_a_dir (NULL, &command_dir, 0)) {

	/* Abort the tunnel, be graceful */
	free_turn_flag = TRUE;
    }

    else {

	/* Notice visibility changes */
	bool old_floor = FALSE;

	/* Take partial confusion into account */
	dir = command_dir;
	confuse_dir(&dir, 0x02);

	y = char_row;
	x = char_col;
	(void)mmove(dir, &y, &x);
	c_ptr = &cave[y][x];

	/* Check the floor-hood */
	old_floor = floor_grid_bold(y, x);

	/* And then apply the current weapon type */
	i_ptr = &inventory[INVEN_WIELD];

	/* Check the validity */
	if (old_floor) {
	    free_turn_flag = TRUE;
	    msg_print("You see nothing there to tunnel through.");
	}

	/* A monster is in the way */
	else if (c_ptr->m_idx > 1) {

	    /* Acquire "Monster" (or "Something") */
	    monster_desc(m_name, &m_list[c_ptr->m_idx], 0x04);
	    message(m_name, 0x03);
	    message(" is in your way!", 0);

	    /* Attempt an attack */
	    if (p_ptr->afraid < 1) py_attack(y, x);
	    else msg_print("You are too afraid!");
	}

	/* Hack -- no tunnelling through doors */
	else if (i_list[c_ptr->i_idx].tval == TV_CLOSED_DOOR) {
	    msg_print("You cannot tunnel through doors.");
	}

	/* You cannot dig without a weapon */        
	else if (i_ptr->tval == TV_NOTHING) {
	    msg_print("You dig with your hands, making no progress.");
	}

	/* Hack -- Penalize heavy weapon */
	else if (p_ptr->use_stat[A_STR] * 15 < i_ptr->weight) {
	    msg_print("Your weapon is too heavy for you to dig with.");
	}

	/* Okay, try digging */
	else {

	    /* Compute the digging ability of player based on strength */
	    tabil = p_ptr->use_stat[A_STR];

	    /* Special diggers (includes all shovels, etc) */
	    if (i_ptr->flags1 & TR1_TUNNEL) {

		/* The "pval" is really important */
		tabil += 25 + i_ptr->pval * 50;
	    }

	    /* Normal weapon */
	    else {

		/* Good weapons make digging easier */
		tabil += (i_ptr->dd * i_ptr->ds);

		/* The weapon bonuses help too */
		tabil += (i_ptr->tohit + i_ptr->todam);

		/* But without a shovel, digging is hard */
		tabil = tabil / 2;
	    }

	    /* Regular walls; Granite, magma intrusion, quartz vein  */
	    /* Don't forget the boundary walls, made of titanium (255) */

	    if (c_ptr->fval == BOUNDARY_WALL) {
		msg_print("This seems to be permanent rock.");
	    }

	    else if (c_ptr->fval == GRANITE_WALL) {

		i = randint(1200) + 80;
		if (twall(y, x, tabil, i)) {
		    if (c_ptr->i_idx && player_can_see(y, x)) {
			msg_print("You have found something!");
		    }
		    else {
			msg_print("You have finished the tunnel.");
		    }
		}
		else {
		    /* We may continue tunelling */
		    msg_print("You tunnel into the granite wall.");
		    more = TRUE;
		}
	    }

	    else if (c_ptr->fval == MAGMA_WALL) {

		i = randint(600) + 10;
		if (twall(y, x, tabil, i)) {
		    if (c_ptr->i_idx && player_can_see(y, x)) {
			msg_print("You have found something!");
		    }
		    else {
			msg_print("You have finished the tunnel.");
		    }
		}
		else {
		    /* We may continue tunelling */
		    msg_print("You tunnel into the magma intrusion.");
		    more = TRUE;
		}
	    }

	    else if (c_ptr->fval == QUARTZ_WALL) {

		i = randint(400) + 10;
		if (twall(y, x, tabil, i)) {
		    if (c_ptr->i_idx && player_can_see(y, x)) {
			msg_print("You have found something!");
		    }
		    else {
			msg_print("You have finished the tunnel.");
		    }
		}
		else {
		    /* We may continue tunelling */
		    msg_print("You tunnel into the quartz vein.");
		    more = TRUE;
		}
	    }

	    /* Empty air */
	    else if (c_ptr->i_idx == 0) {
		msg_print("Tunnel through what?  Empty air?!?");
	    }

	    /* Secret doors. */
	    else if (i_list[c_ptr->i_idx].tval == TV_SECRET_DOOR) {
		/* We may continue tunelling */
		msg_print("You tunnel into the granite wall.");
		search(char_row, char_col, p_ptr->srh);
		more = TRUE;
	    }

	    /* Rubble */
	    else if (i_list[c_ptr->i_idx].tval == TV_RUBBLE) {
		if (tabil > randint(180)) {
		    delete_object(y, x);
		    msg_print("You have removed the rubble.");
		    if (randint(10) == 1) {
			place_object(y, x);
			if (test_lite(y, x)) {
			     msg_print("You have found something!");
			}
		    }
		    lite_spot(y, x);
		}
		else {
		    /* We may continue tunelling */
		    more = TRUE;
		    msg_print("You dig in the rubble.");
		}
	    }

	    /* Anything else is illegal */
	    else {
		msg_print("You can't tunnel through that.");
	    }
	}

	/* Notice "blockage" changes */
	if (old_floor != floor_grid_bold(y, x)) {

	    /* Update the view, lite, monsters */
	    update_view();
	    update_lite();
	    update_monsters();

	    /* Check the view */
	    check_view();
	}
    }

    /* Cancel repetition unless we can continue */
    if (!more) command_rep = 0;
}


/*
 * Disarms a trap, or chest	-RAK-	
 */
void do_cmd_disarm()
{
    int                 y, x, dir;
    register int        tot;
    register cave_type *c_ptr;
    register inven_type *i_ptr;
    char                m_name[80];

    /* Assume we cannot continue repeating */
    int more = FALSE;

    /* Get a direction (or abort) */
    if (!get_a_dir(NULL, &command_dir, 0)) {
	/* Abort Gracefully */
	free_turn_flag = TRUE;
    }

    else {

	dir = command_dir;
	confuse_dir(&dir, 0x02);

	y = char_row;
	x = char_col;
	(void)mmove(dir, &y, &x);
	c_ptr = &cave[y][x];

	i_ptr = &i_list[c_ptr->i_idx];


	/* Nothing useful there */
	if ((c_ptr->i_idx == 0) ||
	    ((i_ptr->tval != TV_VIS_TRAP) &&
	     (i_ptr->tval != TV_CHEST))) {

	    msg_print("I do not see anything there to disarm.");
	    free_turn_flag = TRUE;
	}

	/* Monster in the way */
	else if (c_ptr->m_idx > 1) {
	    /* Acquire "Monster" (or "Something") */
	    monster_desc(m_name, &m_list[c_ptr->m_idx], 0x04);
	    message(m_name, 0x03);
	    message(" is in your way!", 0);
	}

	/* Normal disarm */
	else {

	    tot = p_ptr->disarm + 2 * todis_adj() + stat_adj(A_INT) +
		  (class_level_adj[p_ptr->pclass][CLA_DISARM] *
		  p_ptr->lev / 3);

	    if ((p_ptr->blind > 0) || (no_lite())) {
		tot = tot / 10;
	    }
	    if (p_ptr->confused > 0) {
		tot = tot / 10;
	    }
	    if (p_ptr->image > 0) {
		tot = tot / 10;
	    }

	    /* Floor trap */
	    if (i_ptr->tval == TV_VIS_TRAP) {

		/* Success */
		if ((tot + 100 - i_ptr->pval) > randint(100)) {
		    msg_print("You have disarmed the trap.");
		    p_ptr->exp += i_ptr->pval;
		    delete_object(y, x);
		    /* move the player onto the trap */
		    move_player(dir, FALSE);
		    prt_experience();
		}

		/* Keep trying */
		else if ((tot > 5) && (randint(tot) > 5)) {
		    /* We may keep trying */
		    more = TRUE;
		    msg_print("You failed to disarm the trap.");
		}

		/* Oops */
		else {
		    msg_print("You set the trap off!");
		    /* Move the player onto the trap */
		    move_player(dir, FALSE);
		}
	    }

	    /* Disarm chest */
	    else if (i_ptr->tval == TV_CHEST) {

		/* Must find the trap first. */
		if (!known2_p(i_ptr)) {
		    msg_print("I don't see any traps.");
		    free_turn_flag = TRUE;
		}

		/* No traps to find. */
		else if (!(i_ptr->flags2 & CH2_TRAP_MASK)) {
		    msg_print("The chest is not trapped.");
		    free_turn_flag = TRUE;
		}

		/* Successful Disarm */
		else if ((tot - i_ptr->pval) > randint(100)) {
		    i_ptr->flags2 &= ~CH2_TRAP_MASK;
		    i_ptr->flags2 |= CH2_DISARMED;
		    msg_print("You have disarmed the chest.");
		    p_ptr->exp += i_ptr->pval;
		    prt_experience();
		}

		/* Keep trying */
		else if ((tot > 5) && (randint(tot) > 5)) {
		    /* We may keep trying */
		    more = TRUE;
		    msg_print("You failed to disarm the chest.");
		}

		/* Oops */
		else {
		    msg_print("You set a trap off!");
		    chest_trap(y, x, i_ptr);
		}
	    }
	}
    }


    /* Cancel repeat unless told not to */
    if (!more) command_rep = 0;
}


/*
 * Bash open a door or chest				-RAK-
 *
 * Note: Affected by strength and weight of character 
 *
 * For a closed door, pval is positive if locked; negative if stuck.
 * A disarm spell unlocks and unjams doors! 
 *
 * For an open door, pval is positive for a broken door. 
 *
 * A closed door can be opened - harder if locked. Any door might be 
 * bashed open (and thereby broken). Bashing a door is (potentially)
 * faster! You move into the door way. To open a stuck door, it must 
 * be bashed. A closed door can be jammed (see do_cmd_spike()).
 *
 * Creatures can also open doors. A creature with open door ability will
 * (if not in the line of sight) move though a closed or secret door with
 * no changes.  If in the line of sight, closed door are openned, & secret
 * door revealed.  Whether in the line of sight or not, such a creature may
 * unlock or unstick a door.  That is, creatures shut doors behind them,
 * and repair ones they break (oops).  A creature with no such ability
 * will attempt to bash a non-secret door. 
 *
 * Note that all forms of bashing now take time, even if silly, so that
 * no information is given away by bashing at invisible creatures.
 */
void do_cmd_bash()
{
    int                 y, x, tmp, dir;
    register cave_type  *c_ptr;
    register inven_type *i_ptr;

    /* Assume we cannot keep bashing */
    int more = FALSE;

    /* Get a direction (or Escape) */
    if (!get_a_dir(NULL, &command_dir, 0)) {
	/* Graceful abort */
	free_turn_flag = TRUE;
    }

    /* Execute the bash */
    else {

	/* Extract bash direction, apply partial confusion */
	dir = command_dir;
	confuse_dir(&dir, 0x02);

	/* Bash location */
	y = char_row;
	x = char_col;
	(void)mmove(dir, &y, &x);
	c_ptr = &cave[y][x];

	/* Request to bash a monster */
	if (c_ptr->m_idx > 1) {
	    if (p_ptr->afraid > 0) {
		msg_print("You are too afraid!");
	    }
	    else {
		py_bash(y, x);
	    }
	}

	/* Request to bash something */
	else if (c_ptr->i_idx != 0) {

	    /* What is there */
	    i_ptr = &i_list[c_ptr->i_idx];

	    /* Bash a closed door */
	    if (i_ptr->tval == TV_CLOSED_DOOR) {

		msg_print("You smash into the door!");

		tmp = p_ptr->use_stat[A_STR] + p_ptr->wt / 2;

		/* Use (roughly) similar method as for monsters. */
		if (randint(tmp * (20 + MY_ABS(i_ptr->pval))) <
			10 * (tmp - MY_ABS(i_ptr->pval))) {

		    msg_print("The door crashes open!");

		    /* Hack -- drop on the old object */
		    invcopy(i_ptr, OBJ_OPEN_DOOR);

		    /* Place it in the dungeon */
		    i_ptr->iy = y;
		    i_ptr->ix = x;

		    /* 50% chance of breaking door */
		    i_ptr->pval = 1 - randint(2);
		    c_ptr->fval = CORR_FLOOR;

		    /* Show the door */
		    lite_spot(y, x);

		    /* If not confused, fall through the door */
		    if (p_ptr->confused == 0) {
			move_player(dir, FALSE);
		    }

		    /* Update the view, lite, and monsters */
		    update_view();
		    update_lite();
		    update_monsters();

		    /* Check the view */
		    check_view();
		}

		else if (randint(150) > p_ptr->use_stat[A_DEX]) {
		    /* Note: this will cancel "repeat" */
		    p_ptr->paralysis = 1 + randint(2);
		    msg_print("You are off-balance.");
		}

		else {
		    /* Allow repeated bashing until dizzy */
		    more = TRUE;
		    msg_print("The door holds firm.");
		}
	    }

	    /* Semi-Hack -- Bash a Chest */
	    else if (i_ptr->tval == TV_CHEST) {
		if (randint(10) == 1) {
		    int tmp_iy = i_ptr->iy;
		    int tmp_ix = i_ptr->ix;
		    msg_print("You have destroyed the chest and its contents!");
		    invcopy(i_ptr, OBJ_RUINED_CHEST);
		    i_ptr->iy = tmp_iy;
		    i_ptr->ix = tmp_ix;
		}
		else if ((i_ptr->flags2 & CH2_LOCKED) && (randint(10) == 1)) {
		    msg_print("The lock breaks open!");
		    i_ptr->flags2 &= ~CH2_LOCKED;
		}
		else {
		    /* We may continue */
		    more = TRUE;
		    msg_print("The chest holds firm.");
		}
	    }

	    /* Bash something else (including secret doors) */
	    else {
		msg_print("You bash it, but nothing interesting happens.");
	    }
	}

	/* Empty Air */
	else if (c_ptr->fval < MIN_WALL) {
	    msg_print("You bash at empty space.");
	}

	/* Walls and secret doors yield same message */
	else {
	    msg_print("You bash it, but nothing interesting happens.");
	}
    }

    /* Unless valid action taken, cancel bash */
    if (!more) command_rep = 0;
}


/*
 * Jam a closed door with a spike -RAK-
 *
 * Be sure not to allow the user to "find" ghosts by spiking
 * Thus, anything that could indicate a monster takes a turn
 */
void do_cmd_spike()
{
    int                  y, x, dir, i, j;
    register cave_type  *c_ptr;
    register inven_type *i_ptr;
    char		m_name[80];

    /* Assume we will not continue with the repeating */
    int			more = FALSE;

    /* Get a direction (or cancel) */
    if (!get_a_dir(NULL, &command_dir, 0)) {    
	/* Abort gracefully */
	free_turn_flag = TRUE;
    }

    else {

	/* Confuse the direction (partially) */
	dir = command_dir;
	confuse_dir (&dir, 0x02);

	/* Get the grid of the thing to jam */
	y = char_row;
	x = char_col;
	(void)mmove(dir, &y, &x);
	c_ptr = &cave[y][x];

	/* What is it? */
	i_ptr = &i_list[c_ptr->i_idx];

	/* Nothing there? */
	if ((c_ptr->i_idx == 0) ||
	    ((i_ptr->tval != TV_OPEN_DOOR) &&
	     (i_ptr->tval != TV_CLOSED_DOOR))) {

	    msg_print("I see no door there.");
	    free_turn_flag = TRUE;
	}

	/* Open doors must be closed */
	else if (i_ptr->tval == TV_OPEN_DOOR) {
	    msg_print("The door must be closed first.");
	    free_turn_flag = TRUE;
	}

	/* Make sure the player has spikes */
	else if (!find_range(TV_SPIKE, TV_NEVER, &i, &j)) {
	    msg_print("But you have no spikes.");
	    free_turn_flag = TRUE;
	}

	/* Is a monster in the way? */
	else if (c_ptr->m_idx != 0) {
	    /* Acquire "Monster" (or "Something") */
	    monster_desc(m_name, &m_list[c_ptr->m_idx], 0x04);
	    message(m_name, 0x03);
	    message(" is in your way!", 0);
	}

	/* Go for it */
	else {

	    /* We may continue spiking (unless out of spikes) */
	    more = TRUE;

	    /* Successful jamming */
	    msg_print("You jam the door with a spike.");

	    /* Make locked to stuck. */
	    if (i_ptr->pval > 0) i_ptr->pval = (-i_ptr->pval);

	    /* Successive spikes have a progressively smaller effect. */
	    /* Series is: 0 20 30 37 43 48 52 56 60 64 67 70 ... */
	    i_ptr->pval -= 1 + 190 / (10 - i_ptr->pval);

	    /* Use up, and describe, a single spike, from the bottom */
	    inven_item_increase(j, -1);
	    inven_item_describe(j);
	    inven_item_optimize(j);
	}
    }

    /* Cancel repetition unless it worked */
    if (!more) command_rep = 0;
}


/*
 * Throw an object across the dungeon.
 * Flasks of oil do "fire damage" (is this still true?)
 * Extra damage and chance of hitting when missiles are used
 * with correct weapon (xbow + bolt, bow + arrow, sling + shot).
 * Rangers (with Bows) and Anyone (with "Extra Shots") get extra shots.
 */
void do_cmd_fire()
{
    int dir, item_val;
    int ok_throw = FALSE;
    inven_type *i_ptr;


    /* Assume free turn */
    free_turn_flag = TRUE;

    if (inven_ctr == 0) {
	msg_print("But you are not carrying anything.");
	return;
    }

    /* Hack -- allow auto-see */
    command_wrk = TRUE;
    
    /* Get an item */
    if (!get_item(&item_val, "Fire/Throw which item? ", 0, inven_ctr-1)) return;

    /* Cancel auto-see */
    command_see = FALSE;
    

    i_ptr = &inventory[item_val];

    /* User can request throw */
    if (always_throw) {

	ok_throw = TRUE;
    }

    /* Some things are just meant to be thrown */
    else if ((i_ptr->tval == TV_FLASK) || (i_ptr->tval == TV_SHOT) ||
	     (i_ptr->tval == TV_ARROW) || (i_ptr->tval == TV_BOLT) ||
	     (i_ptr->tval == TV_SPIKE) || (i_ptr->tval == TV_JUNK) ||
	     (i_ptr->tval == TV_BOTTLE) || (i_ptr->tval == TV_SKELETON)) {

	ok_throw = TRUE;
    }

    /* Player knows (or feels) that it is cursed */
    else if (cursed_p(i_ptr) &&
	     (known2_p(i_ptr) || (i_ptr->ident & ID_FELT))) {
	ok_throw = TRUE;
    }

    /* If the player knows that it is junk, throw it */
    else if (known2_p(i_ptr) && (i_ptr->cost <= 0)) {
	ok_throw = TRUE;
    }

    /* Known Artifacts and ego objects are never "okay" */
    else if (known2_p(i_ptr) &&
	     ((i_ptr->name1) || (i_ptr->name2) || artifact_p(i_ptr))) {
	ok_throw = FALSE;
    }

    /* Normal weapons are okay to throw, since they do damage */
    /* (Moral of story: wield your weapon if you're worried */
    /* that you might throw it away!).  Anyway, there is */
    /* usually only a 1/5 chance of disappearing */
    else if ((i_ptr->tval >= TV_HAFTED) && (i_ptr->tval <= TV_DIGGING)) {
	ok_throw = TRUE;
    }

    /* Most food/potions do 1d1 damage when thrown.  I want the code	*/
    /* to ask before throwing away potions of DEX, *Healing*, etc.	*/
    /* This also means it will ask before throwing potions of slow	*/
    /* poison, and other cheap items that the player is likely to	*/
    /* not care about.  This test means that mushrooms/molds of	*/
    /* unhealth, potions of detonations and death are the only	*/
    /* always-throwable food/potions (but see "bad items" below)	*/
    else if (((i_ptr->tval == TV_FOOD) || (i_ptr->tval == TV_POTION)) &&
	     (inven_aware_p(i_ptr)) &&
	     ((i_ptr->dd * i_ptr->ds) > 1)) {
	ok_throw = TRUE;
    }


    /* If the object looks "not okay", verify it */
    if (!ok_throw) {
	char tmp_str[128], out_val[128];
	objdes(tmp_str, i_ptr, TRUE);
	sprintf(out_val, "Really throw %s?", tmp_str);
	ok_throw = get_check(out_val);
    }


    /* If they really want to, get a direction and throw it */
    if (ok_throw) {

	/* Get a direction (or Abort), apply confusion */
	if (get_dir_c(NULL, &dir)) {

	    /* Take a turn and shoot the object */
	    free_turn_flag = FALSE;
	    shoot(item_val, dir);
	}
    }
}


/*
 * Support code for the "Walk" and "Jump" commands
 */
void do_cmd_walk(int pickup)
{
    int dir;

    /* Get the initial direction (or Abort) */
    if (!get_a_dir (NULL, &command_dir, 0)) {
	/* Graceful abort */
	free_turn_flag = TRUE;
	command_rep = 0;
    }

    /* Attempt to walk */
    else {

	/* Apply partial confusion */
	dir = command_dir;
	confuse_dir(&dir, 0x02);

	/* Actually move the character */
	move_player(dir, pickup);
    }
}


/*
 * Stay still (but check for treasure, traps, re-light, etc)
 */

void do_cmd_stay(int pickup)
{
    /* Actually "move" the character (overkill, but it works) */
    move_player(5, pickup);
}



/*
 * Do the first (or next) step of the run (given max distance)
 * If distance is non positive, assume max distance of 1000.
 *
 * We must erase the player symbol '@' here, because sub3_move_lite() does
 * not erase the previous location of the player when in find mode and when
 * find_prself is FALSE.  The player symbol is not draw at all in this case
 * while moving, so the only problem is on the first turn of find mode, when
 * the initial position of the character must be erased. Hence we must do the
 * erasure here.
 */

void do_cmd_run()
{
    /* Get the initial direction (or Abort) */
    if (!get_a_dir (NULL, &command_dir, 0)) {
	/* Graceful abort */
	free_turn_flag = TRUE;
    }

    else {

	/* Prepare to Run */
	find_init();

	/* Save the max run-distance (default to 1000 steps) */
	if (command_arg <= 0) command_arg = 1000;

	/* Take the first step */
	find_step();
    }
}


/*
 * Simple command to "search" for one turn
 */
void do_cmd_search(void)
{
    /* Use the current location, and ability */
    search(char_row, char_col, p_ptr->srh);
}




/*
 * Resting allows a player to safely restore his hp	-RAK-	 
 */
void do_cmd_rest(void)
{
    vtype rest_str;
    char ch;

    /* Prompt for time if needed */
    if (command_arg <= 0) {

	/* Assume no rest */
	command_arg = 0;

	/* Ask the question (perhaps a "prompt" routine would be good) */
	prt("Rest for how long? ('*' for HP/mana; '&' as needed) : ", 0, 0);
	if (get_string(rest_str, 0, 54, 5)) {
	    if (sscanf(rest_str, "%c", &ch) == 1) {
		if (ch == '*') {
		    command_arg = (-1);
		}
		else if (ch == '&') {
		    command_arg = (-2);
		}
		else {
		    command_arg = atoi(rest_str);
		    if (command_arg > 30000) command_arg = 30000;
		    if (command_arg < 0) command_arg = 0;
		}
	    }
	}
    }

    /* Induce Rest */
    if (command_arg != 0) {

	search_off();

	p_ptr->rest = command_arg;
	p_ptr->status |= PY_REST;
	prt_state();

	prt("Press any key to stop resting...", 0, 0);
	Term_fresh();
    }

    /* Rest was cancelled */    
    else {
	erase_line(MSG_LINE, 0);
	free_turn_flag = TRUE;
    }    
}



/*
 * Note that "feeling" is set to zero unless some time has passed.
 * Note that this is done when the level is GENERATED, not entered.
 */
void do_cmd_feeling()
{
    /* Free move */
    free_turn_flag = TRUE; 

    /* No useful feeling in town */
    if (!dun_level) {
	msg_print("You feel there is something special about the town level.");
	return;
    }

    /* Analyze the feeling */
    switch(feeling) {
      case 0:
	msg_print("Looks like any other level.");
	break;
      case 1:
	msg_print("You feel there is something special about this level.");
	break;
      case 2:
	msg_print("You have a superb feeling about this level.");
	break;
      case 3:
	msg_print("You have an excellent feeling that your luck is turning...");
	break;
      case 4:
	msg_print("You have a very good feeling.");
	break;
      case 5:
	msg_print("You have a good feeling.");
	break;
      case 6:
	msg_print("You feel strangely lucky.");
	break;
      case 7:
	msg_print("You feel your luck is turning...");
	break;
      case 8:
	msg_print("You like the look of this place.");
	break;
      case 9:
	msg_print("This level can't be all bad...");
	break;
      default:
	msg_print("What a boring place...");
	break;
    }
}




/*
 * Give an object a textual inscription
 */
void inscribe(inven_type *i_ptr, cptr str)
{
    register int i;

    /* Add the desired comment */
    for (i = 0; str[i] && (i < INSCRIP_SIZE - 1); ++i) {
	i_ptr->inscrip[i] = str[i];
    }

    /* Hack -- zero out the extra bytes */
    for ( ; i < INSCRIP_SIZE - 1; i++) {
	i_ptr->inscrip[i] = '\0';
    }

    /* Always terminate the string */
    i_ptr->inscrip[i] = '\0';
}


/*
 * Add a comment to an object description.		-CJS- 
 */
void do_cmd_inscribe(void)
{
    int   item_val;
    inven_type *i_ptr;
    vtype out_val, tmp_str;


    /* Free move */
    free_turn_flag = TRUE; 

    /* Require some objects */
    if (!inven_ctr && !equip_ctr) {
	msg_print("You are not carrying anything to inscribe.");
	return;
    }


    /* Hack -- allow auto-see */
    command_wrk = TRUE;
    
    /* Require a choice */
    if (!get_item(&item_val, "Inscribe which item? ", 0, INVEN_TOTAL-1)) return;

    /* Cancel auto-see */
    command_see = FALSE;
    

    /* Get the item */
    i_ptr = &inventory[item_val];

    /* Describe the activity */
    objdes(tmp_str, i_ptr, TRUE);
    (void)sprintf(out_val, "Inscribing %s.", tmp_str);
    msg_print(out_val);
    msg_print(NULL);

    /* Prompt for an inscription */
    if (i_ptr->inscrip[0]) {
	(void)sprintf(out_val, "Replace \"%s\" with the inscription: ",
		      i_ptr->inscrip);
	prt(out_val, 0, 0);
    }
    else {
	prt("Inscription: ", 0, 0);
    }

    /* Get a new inscription and apply it */
    if (askfor(out_val, INSCRIP_SIZE - 1)) inscribe(i_ptr, out_val);
}



/*
 * Print out the artifacts seen.
 * This can be used to notice "missed" artifacts.
 *
 * XXX Perhaps this routine induces a blank final screen.
 */
void do_cmd_check_artifacts(void)
{
    int i, j, k, t;

    char out_val[256];


    /* Free turn */
    free_turn_flag = TRUE;


#ifndef ALLOW_CHECK_ARTIFACTS
    if (!wizard) {
	msg_print("That command was not compiled.");
	return;
    }
#endif

    /* Hack -- no checking in the dungeon */
    if (dun_level && !wizard) {
	msg_print("You need to be in town to check artifacts!");
	return;
    }


    /* Save the screen */
    save_screen();

    /* Use column 15 */
    j = 15;

    /* Erase some lines */    
    for (i = 1; i < 23; i++) erase_line(i, j - 2);

    /* Start in line 1 */
    i = 1;

    /* Title the screen */
    prt("Artifacts Seen:", i++, j + 5);

    /* Scan the artifacts */
    for (k = 0; k < ART_MAX; k++) {

	/* Hack -- Skip "illegal" artifacts */
	if (!v_list[k].name) continue;

	/* Has that artifact been created? */
	if (v_list[k].cur_num) {

	    int z;
	    char base_name[80];

	    /* Hack -- default to "Artifact" */
	    strcpy(base_name, "Artifact");

	    /* Hack -- Track down the "type" name */
	    for (z = 0; z < MAX_K_IDX; z++) {
		if ((k_list[z].tval == v_list[k].tval) &&
		    (k_list[z].sval == v_list[k].sval)) {

		    /* Hack -- Make a fake object */
		    inven_type forge;
		    invcopy(&forge, z);
		    forge.name1 = k;
		    known2(&forge);
		    objdes(base_name, &forge, FALSE);
		    break;
		}
	    }

	    /* Hack -- Build the artifact name */
	    sprintf(out_val, "The %s %s", base_name, v_list[k].name);

	    /* Dump a line */
	    prt(out_val, i++, j);

	    /* is screen full? */
	    if (i == 22) {
		prt("-- more --", i, j);
		inkey();
		for (t = 2; t < 23; t++) erase_line(t, j);
		prt("Artifacts seen: (continued)", 1, j + 5);
		i = 2;
	    }
	}
    }

    /* Pause */
    prt("[Press any key to continue]", i, j);
    inkey();


    /* Restore the screen */
    restore_screen();
}


/*
 * print out the status of uniques - cba 
 *
 * XXX This routine may induce a blank final screen.
 */
void do_cmd_check_uniques()
{
    int      i, j, k, t;
    bigvtype msg;

	
    free_turn_flag = TRUE;

#ifndef ALLOW_CHECK_UNIQUES
    if (!wizard) {
	msg_print("That command was not compiled.");
	return;
    }
#endif


    save_screen();

    j = 15;

    for (i = 1; i < 23; i++) erase_line(i, j - 2);

    i = 1;
    prt("Uniques:", i++, j + 5);

    /* Note -- skip the ghost */
    for (k = 0; k < MAX_R_IDX-1; k++) {

	/* Only print Uniques */
	if (r_list[k].cflags2 & MF2_UNIQUE) {

	    bool dead = (l_list[k].max_num == 0);

	    /* Wizards know everything, players know kills */
	    if (wizard || dead) {

		/* Print a message */            
		sprintf(msg, "%s is %s.", r_list[k].name,
			dead ? "dead" : "alive");            
		prt(msg, i++, j);

		/* is screen full? */
		if (i == 22) {
		    prt("-- more --", i, j);
		    inkey();
		    for (t = 2; t < 23; t++) erase_line(t, j);
		    prt("Uniques: (continued)", 1, j + 5);
		    i = 2;
		}
	    }
	}
    }

    /* Pause */
    prt("[Press any key to continue]", i, j);
    inkey();

    /* Restore the screen */
    restore_screen();
}



