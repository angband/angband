/* File: borg-ext.c */

/* Purpose: helper file for "borg-ben.c" -BEN- */

#include "angband.h"

#ifdef ALLOW_BORG

#include "borg.h"

#include "borg-obj.h"

#include "borg-map.h"

#include "borg-ext.h"



/*
 * See "borg.h" and "borg-ben.c" for general information.
 */



/*
 * Some information
 */

int goal;			/* Current "goal" */

int goal_rising;		/* Currently fleeing to town */

int stair_less;			/* Use the next "up" staircase */
int stair_more;			/* Use the next "down" staircase */

int count_floor;		/* Number of floor grids */
int count_less;			/* Number of stairs (up) */
int count_more;			/* Number of stairs (down) */
int count_kill;			/* Number of monsters */
int count_take;			/* Number of objects */

int town_stair_x;		/* Hack -- Stairs in town */
int town_stair_y;		/* Hack -- Stairs in town */

int last_visit;			/* Last purchase visit */

s32b auto_began;		/* When this level began */

s32b auto_shock;		/* When last "shocked" */



/*
 * Some tables
 */
 
byte auto_char[256];		/* Analysis of various char codes */



/*
 * Wipe things
 */
void borg_ext_wipe(void)
{
    /* Hack -- Clear the key buffer */
    borg_flush();

    /* Restart the clock */
    c_t = 10000L;

    /* Start a new level */
    auto_began = c_t;

    /* Shocking */
    auto_shock = c_t;

    /* No goal yet */
    goal = 0;

    /* Do not use any stairs */
    stair_less = stair_more = FALSE;

    /* Hack -- cannot rise past town */
    if (!auto_depth) goal_rising = FALSE;

    /* No known grids yet */
    count_floor = count_less = count_more = 0;

    /* Nothing to chase yet */
    count_kill = count_take = 0;

    /* Nothing bought yet */
    last_visit = 0;

    /* Forget the map */
    borg_forget_map();
}



/*
 * Look at the screen and update the borg
 *
 * Uses the "panel" info (w_x, w_y) obtained earlier
 *
 * Note that the "c_t" variable corresponds *roughly* to player turns,
 * except that resting and "repeated" commands count as a single turn,
 * and "free" moves (including "illegal" moves, such as attempted moves
 * into walls, or tunneling into monsters) are counted as turns.
 */
void borg_update(void)
{
    int x, y, dx, dy;

    auto_grid *ag;


    byte old_attr[SCREEN_HGT][SCREEN_WID];
    char old_char[SCREEN_HGT][SCREEN_WID];


    /* Memorize the "current" (66x22 grid) map sector */
    for (dy = 0; dy < SCREEN_HGT; dy++) {
        for (dx = 0; dx < SCREEN_WID; dx++) {

            /* Obtain the map location */
            x = w_x + dx;
            y = w_y + dy;

            /* Get the auto_grid */
            ag = grid(x, y);

            /* Save the current knowledge */
            old_attr[dy][dx] = ag->o_a;
            old_char[dy][dx] = ag->o_c;
        }
    }


    /* Analyze the map */
    borg_update_map();

    /* Paranoia -- make sure we exist */
    if (do_image) {
        borg_oops("Hallucinating (?)");
        return;
    }


#ifdef BORG_ROOMS

    /* Make "fake" rooms for all viewable unknown grids */
    for (n = 0; n < view_n; n++) {

        /* Access the location */
        y = view_y[n];
        x = view_x[n];

        /* Access the grid */
        ag = grid(x,y);

        /* Skip walls */
        if (ag->info & BORG_WALL) continue;

        /* Skip "known" grids */
        if (ag->o_c != ' ') continue;

        /* Make "fake" rooms as needed */
        if (!ag->room) {

            auto_room *ar;

            /* Get a new room */
            ar = borg_free_room();

            /* Initialize the room */
            ar->x = ar->x1 = ar->x2 = x;
            ar->y = ar->y1 = ar->y2 = y;

            /* Save the room */
            ag->room = ar->self;
        }
    }

#endif


    /* Analyze the current (66x22 grid) map sector */
    for (dy = 0; dy < SCREEN_HGT; dy++) {
        for (dx = 0; dx < SCREEN_WID; dx++) {

            /* Obtain the map location */
            x = w_x + dx;
            y = w_y + dy;

            /* Get the auto_grid */
            ag = grid(x, y);

            /* Skip unknown grids */
            if (ag->o_c == ' ') continue;

            /* Notice first "knowledge" */
            if (old_char[dy][dx] == ' ') {

                /* We are shocked */
                auto_shock = c_t;
            }

            /* Count certain "changes" */
            if (ag->o_c != old_char[dy][dx]) {

                char oc = old_char[dy][dx];
                char nc = ag->o_c;
                
                /* Lost old "floor" grids */
                if (oc == '.') count_floor--;

                /* Lost old "permanent" landmarks */
                if (oc == '<') count_less--;
                if (oc == '>') count_more--;

                /* Lost old "temporary" info */
                if (auto_char[(byte)(oc)] == AUTO_CHAR_KILL) count_kill--;
                if (auto_char[(byte)(oc)] == AUTO_CHAR_TAKE) count_take--;

                /* Found new "floor" grids */		
                if (nc == '.') count_floor++;

                /* Found new "permanent" landmarks */
                if (nc == '<') count_less++;
                if (nc == '>') count_more++;

                /* Found new "temporary" info */
                if (auto_char[(byte)(nc)] == AUTO_CHAR_KILL) count_kill++;
                if (auto_char[(byte)(nc)] == AUTO_CHAR_TAKE) count_take++;
                
                /* Track stairs in town */
                if (!auto_depth && (nc == '>')) {
                    town_stair_x = x;
                    town_stair_y = y;
                }
            }


#ifdef BORG_ROOMS

            /* Clear all "broken" rooms */
            if (auto_char[(byte)(ag->o_c)] == AUTO_CHAR_WALL) {

                /* Clear all rooms containing walls */
                if (ag->room) {

                    /* Clear all rooms containing walls */
                    if (borg_clear_room(x, y)) goal = 0;
                }
            }

            /* Create "fake" rooms as needed */
            else {

                /* Mega-Hack -- super-fake rooms */
                if (!ag->room) {

                    /* Acquire a new room */
                    auto_room *ar = borg_free_room();

                    /* Initialize the room */
                    ar->x = ar->x1 = ar->x2 = x;
                    ar->y = ar->y1 = ar->y2 = y;

                    /* Save the room */
                    ag->room = ar->self;
                }	
            }

#endif

            /* Skip non-viewable grids */
            if (!(ag->info & BORG_VIEW)) continue;

            /* Hack -- Notice all (viewable) changes */
            if ((ag->o_c != old_char[dy][dx]) ||
                (ag->o_a != old_attr[dy][dx])) {

                /* Important -- Cancel goals */
                goal = 0;
            }
        }
    }	


#ifdef BORG_ROOMS

    /* Paranoia -- require a self room */
    if (!grid(c_x,c_y)->room) {

        /* Acquire a new room */
        auto_room *ar = borg_free_room();

        /* Initialize the room */
        ar->x = ar->x1 = ar->x2 = c_x;
        ar->y = ar->y1 = ar->y2 = c_y;

        /* Save the room */
        grid(c_x,c_y)->room = ar->self;
    }

    /* Build a "bigger" room around the player */
    if (borg_build_room(c_x, c_y)) goal = 0;

    /* Hack */
    if (TRUE) {
    
        auto_room *ar;
        
        /* Mark all the "containing rooms" as visited. */
        for (ar = room(1,c_x,c_y); ar; ar = room(0,0,0)) {

            /* Note the visit */
            ar->when = c_t;
        }
    }

#else

    /* Note the visit */
    /* grid(c_x,c_y)->when = c_t; */

#endif

    /* Hack -- Count visits to this grid */
    /* if (grid(c_x,c_y)->visits < 100) grid(c_x,c_y)->visits++; */
}









/*
 * Attempt to guess what type of monster is at the given location.
 *
 * Use type "0" for "no known monster".
 *
 * This guess can be improved by the judicious use of a specialized
 * "attr/char" mapping, especially for unique monsters.  Note that
 * we cheat by directly accessing the attr/char tables.
 *
 * We will never correctly identify player ghosts
 * We will usually fail to identify multi-hued monsters
 * We will usually fail for very out of depth monsters
 * We will attempt to err on the side of caution
 *
 * We treat the "town" level specially, and unless a player ghost
 * appears in town, our guess will always be correct.
 */
static int borg_guess_race(int x, int y)
{
    int i;

    auto_grid *ag = grid(x, y);


    /* Special treatment of "town" monsters */
    if (!auto_depth) {

        /* Hack -- Find the first "acceptable" monster */
        for (i = 1; i < MAX_R_IDX-1; i++) {

            monster_race *r_ptr = &r_list[i];
            monster_lore *l_ptr = &l_list[i];

            if (r_ptr->level) break;

            if (ag->o_c != l_ptr->l_char) continue;
            if (ag->o_a != l_ptr->l_attr) continue;

            return (i);
        }
    }

    /* Guess what monster it might be */
    else {

        /* Hack -- Find the highest "acceptable" monster */
        for (i = (MAX_R_IDX-1) - 1; i > 0; i--) {

            monster_race *r_ptr = &r_list[i];
            monster_lore *l_ptr = &l_list[i];

            if (r_ptr->level > auto_depth + 5) continue;

            if (ag->o_c != l_ptr->l_char) continue;
            if (ag->o_a != l_ptr->l_attr) continue;

            return (i);
        }
    }


    /* Oops */
    return (0);
}



/*
 * Guess how much danger a monster implies
 */
static int borg_danger_monster(int r)
{
    int i, f, p = 0;
    
    monster_race *r_ptr = &r_list[r];
    
    /* Hack -- Never attacks */
    if (r_ptr->rflags1 & RF1_NEVER_BLOW) return (0);

    /* Analyze each attack */
    for (i = 0; i < 4; i++) {

        /* Done */
        if (!r_ptr->blow[i].method) break;
        
        /* Hack -- Add in some danger for each attack */
        p += (r_ptr->blow[i].d_dice * r_ptr->blow[i].d_side);

        /* Analyze the attack */
        switch (r_ptr->blow[i].effect) {

            /* Theft is dangerous! */
            case RBE_EAT_ITEM:  p += 20; break;
            case RBE_EAT_GOLD:  p += 10; break;

            /* Special attacks are annoying */
            case RBE_BLIND:	p += 10; break;
            case RBE_CONFUSE:	p += 10; break;
            case RBE_TERRIFY:	p += 10; break;
            case RBE_PARALYZE:	p += 10; break;
            case RBE_POISON:	p += 10; break;

            /* Painful attacks */
            case RBE_ACID:	p += 40; break;
            case RBE_ELEC:	p += 20; break;
            case RBE_FIRE:	p += 40; break;
            case RBE_COLD:	p += 20; break;

            /* Stat draining is very annoying */
            case RBE_LOSE_STR:	p += 50; break;
            case RBE_LOSE_DEX:	p += 50; break;
            case RBE_LOSE_CON:	p += 50; break;
            case RBE_LOSE_INT:	p += 50; break;
            case RBE_LOSE_WIS:	p += 50; break;
            case RBE_LOSE_CHR:	p += 50; break;

            /* Experience draining sucks big time */
            case RBE_EXP_10:	p += 100; break;
            case RBE_EXP_20:	p += 100; break;
            case RBE_EXP_40:	p += 100; break;
            case RBE_EXP_80:	p += 100; break;
            
            /* Really annoying attacks */
            case RBE_LOSE_ALL:	p += 500; break;
            case RBE_UN_BONUS:	p += 500; break;
        }
    }
    
    /* Skip non-danger */
    if (p <= 0) return (0);
        
    /* Hack -- Add in some danger for hit-points */
    /* p += (r_ptr->hdice * r_ptr->hside) / 8; */

    /* Extract monster speed */
    f = (r_ptr->speed - 110);

    /* Hack -- Increase the danger by the monster speed */
    if (f > auto_speed) p += (p * (f - auto_speed) / 10);
    
    /* Total */
    return (p);
}



/*
 * Count the "danger" of the given location.
 * Currently based on the "hp" and "blows" of adjacent monsters.
 * Should also reflect spell attacks, and special blow effects.
 * Should also consider non-adjacent monsters with spell attacks.
 * Should take monster/player speed into account.
 */
static int borg_danger(int x0, int y0)
{
    int i1, n = 0;

    auto_grid *ag = grid(x0, y0);


    /* Hack -- traps are dangerous */
    if (ag->o_c == '^') {

        int i;
        int n1 = 0;
        
        /* Hack -- Find an "acceptable"  */
        for (i = OBJ_TRAP_LIST; i < OBJ_TRAP_LIST + MAX_TRAP; i++) {

            int n2 = 0;
            
            inven_xtra *x_ptr = &x_list[i];

            if (ag->o_c != x_ptr->x_char) continue;
            if (ag->o_a != x_ptr->x_attr) continue;

            /* Some traps are dangerous */
            switch (k_list[i].sval) {

                case SV_TRAP_PIT: n2 = 10; break;
                case SV_TRAP_SPIKED_PIT: n2 = 40; break;
                case SV_TRAP_TRAP_DOOR: n2 = 50; break;
                case SV_TRAP_ARROW: n2 = 10; break;
                case SV_TRAP_DART_SLOW: n2 = 10; break;
                case SV_TRAP_DART_DEX: n2 = 50; break;
                case SV_TRAP_DART_STR: n2 = 50; break;
                case SV_TRAP_DART_CON: n2 = 50; break;
                case SV_TRAP_GAS_POISON: n2 = 25; break;
                case SV_TRAP_GAS_BLIND: n2 = 25; break;
                case SV_TRAP_GAS_CONFUSE: n2 = 25; break;
                case SV_TRAP_GAS_SLEEP: n2 = 25; break;
                case SV_TRAP_FIRE: n2 = 50; break;
                case SV_TRAP_ACID: n2 = 50; break;
                case SV_TRAP_TELEPORT: n2 = 5; break;
                case SV_TRAP_SUMMON: n2 = 20; break;
                case SV_TRAP_FALLING_ROCK: n2 = 20; break;
            }

            /* Maintain max danger */
            if (n1 < n2) n1 = n2;
        }
        
        /* Add in the danger */
        n += n1;
    }


    /* Look around */
    for (i1 = 0; i1 < 8; i1++) {

        int n1 = 0;
        
        int d1 = ddd[i1];
        int x1 = x0 + ddx[d1];
        int y1 = y0 + ddy[d1];

        auto_grid *ag1 = grid(x1, y1);

        /* Ignore "dark" grids */
        if (!(ag1->info & BORG_OKAY)) {

            /* No danger (?) */
        }
        
        /* Monsters are dangerous */
        else if (auto_char[(byte)(ag1->o_c)] == AUTO_CHAR_KILL) {

            /* Access the monster */
            int r1 = borg_guess_race(x1, y1);

            /* Monsters are dangerous */
            n1 += borg_danger_monster(r1);
        }
        
        /* Open space can be dangerous */
        else if (!(ag1->info & BORG_WALL)) {

            int i2, n2 = 0;

            /* Check around */
            for (i2 = 0; i2 < 8; i2++) {

                int d2 = ddd[i2];
                int x2 = x1 + ddx[d2];
                int y2 = y1 + ddy[d2];

                auto_grid *ag2 = grid(x2, y2);

                /* Ignore "dark" grids */
                if (!(ag2->info & BORG_OKAY)) {

                    /* No danger (?) */
                }
        
                /* Monsters are dangerous (if mobile) */
                else if (auto_char[(byte)(ag2->o_c)] == AUTO_CHAR_KILL) {

                    /* Access the monster */
                    int r2 = borg_guess_race(x2, y2);

                    /* Skip non-mobile monsters */
                    if (r_list[r2].rflags1 & RF1_NEVER_MOVE) continue;
                    
                    /* Monsters are (somewhat) dangerous */
                    n2 += borg_danger_monster(r2);
                }
            }
            
            /* Add in the danger */
            n1 += n2 / 10;
        }

        /* Add in the danger */
        n += n1;
    }

    /* Return the danger */
    return (n);
}



/*
 * Mega-Hack -- evaluate the "freedom" of the given location
 *
 * This may help avoid getting stuck in corners
 *
 * XXX XXX XXX We need a better method (!)
 */
static int borg_freedom(int x, int y)
{
    int i, d, f = 0;
    
    /* Mega-Hack -- stay in center of town */
    if (!auto_depth) {

        /* Love the stairs! */
        d = double_distance(x, y, town_stair_x, town_stair_y);
        
        /* Distance is a bad thing */
        f += (1000 - d);

        /* Zero distance yields much freedom */
        if (!d) f += 1000;
        
        /* Small distance yields some freedom */
        if (d <= 2) f += 500;
    }

    /* Desire free space */
    for (i = 0; i < 8; i++) {

        int x1 = x + ddx[i];
        int y1 = y + ddy[i];

        auto_grid *ag = grid(x1, y1);

        /* Count open space */
        if (!(ag->info & BORG_WALL)) f++;
    }
    
    /* Freedom */
    return (f);
}




/*
 * Be "cautious".  This routine assumes that all damage comes
 * from dangerous monsters, which is good when invisible monsters
 * are around, but perhaps silly when poisoned or lightly wounded.
 *
 * A big problem is that we only check danger when we are already
 * standing next to it.  We should actually try to flee from it.
 * Or at least not walk into it.  This is now (sort of) done.
 *
 * Unfortunately, in the town, the Borg tends to flee into a
 * corner and get slaughtered (just like the damn monsters!)
 */
bool borg_caution(void)
{
    int x, y;
    int i, d, p, k;

    int g_p = -1;
    int g_d = -1;

    auto_grid *ag;

    byte old_mushroom = auto_char[(byte)(',')];
    byte new_mushroom = auto_char[(byte)(',')];


    /* XXX XXX XXX Mega-Hack -- attack mushrooms */
    if ((rand_int(100) < 10) ||
        (do_afraid && (rand_int(100) < 10)) ||
        ((auto_chp < auto_mhp) && (rand_int(100) < 50))) {

        /* Kill mushrooms */
        new_mushroom = AUTO_CHAR_KILL;
    }


    /* Handle mushrooms */
    auto_char[(byte)(',')] = new_mushroom;

    /* Look around */
    p = borg_danger(c_x, c_y);

    /* Handle mushrooms */
    auto_char[(byte)(',')] = old_mushroom;



    /* Attempt to teleport to safety */
    if (auto_chp < auto_mhp / 4) {

        /* Prefer "fastest" methods */
        if (borg_read_scroll(SV_SCROLL_TELEPORT) ||
            borg_use_staff(SV_STAFF_TELEPORTATION) ||
            borg_spell(1,5) ||
            borg_spell(0,2) ||
            borg_prayer(1,1) ||
            borg_read_scroll(SV_SCROLL_PHASE_DOOR)) {

            return (TRUE);
        }
    }


    /* Attempt to teleport to safety */
    if ((auto_chp < auto_mhp / 2) && (p > auto_chp / 2)) {

        /* Prefer "cheapest" methods */
        if (borg_spell(0,2) ||
            borg_spell(1,5) ||
            borg_prayer(1,1) ||
            borg_read_scroll(SV_SCROLL_PHASE_DOOR) ||
            borg_read_scroll(SV_SCROLL_TELEPORT) ||
            borg_use_staff(SV_STAFF_TELEPORTATION)) {
            
            return (TRUE);
        }
    }


    /* Ignore "wimpy" monsters */
    if (p < auto_chp / 4) return (FALSE);

    /* Hack -- Sometimes ignore "medium" monsters */
    if ((p < auto_chp / 2) && (rand_int(100) < 50)) return (FALSE);

    /* Hack -- Sometimes ignore "medium" monsters */
    if ((p < auto_chp) && (rand_int(100) < 10)) return (FALSE);


    /* Hack -- Check for stairs */
    if (p > auto_chp) {
    
        ag = grid(c_x,c_y);
        
        /* Mega-Hack */
        if (ag->o_c == '<') {

            /* Flee! */
            borg_note("Fleeing up some stairs!");
            borg_keypress('<');
            return (TRUE);
        }
    
        /* Mega-Hack */
        if (ag->o_c == '>') {

            /* Flee! */
            borg_note("Fleeing down some stairs!");
            borg_keypress('>');
            return (TRUE);
        }
    }
    

    /* Handle mushrooms */
    auto_char[(byte)(',')] = new_mushroom;

    /* Attempt to find a better grid */
    for (i = 0; i < 8; i++) {

        /* Access the grid */
        d = ddd[i];
        x = c_x + ddx[d];
        y = c_y + ddy[d];
        ag = grid(x, y);

        /* Skip walls (!) */
        if (ag->info & BORG_WALL) continue;
        
        /* Skip unknown grids (?) */
        if (ag->o_c == ' ') continue;

        /* Hack -- Skip monsters */
        if (auto_char[(byte)(ag->o_c)] == AUTO_CHAR_KILL) continue;

        /* Mega-Hack -- Skip stores (dead ends) */
        if (auto_char[(byte)(ag->o_c)] == AUTO_CHAR_SHOP) continue;

        /* Extract the danger there */
        k = borg_danger(x, y);

        /* Ignore "worse" choices */
        if ((g_d >= 0) && (k > g_p)) continue;

        /* Hack -- Compare "equal" choices */
        if ((g_d >= 0) && (k == g_p)) {

            int new = borg_freedom(x,y);
            int old = borg_freedom(c_x+ddx[g_d], c_y+ddy[g_d]);
    
            /* Hack -- run for freedom */
            if (new < old) continue;
        }
        
        /* Save the info */
        g_p = k; g_d = d;
    }

    /* Handle mushrooms */
    auto_char[(byte)(',')] = old_mushroom;


    /* Somewhere "useful" to flee */
    if ((g_d >= 0) && (p > g_p + g_p / 4)) {

        /* Note */
        borg_note(format("Caution (%d > %d).", p, g_p));

        /* Hack -- Flee! */
        borg_keypress('0' + g_d);

        /* Success */
        return (TRUE);
    }

    /* Note */
    borg_note(format("Cornered (danger %d).", p));

    
    /* Nothing */
    return (FALSE);
}


/*
 * Attack monsters (before firing)
 */
bool borg_attack(void)
{
    int i, r, n = 0, g_d = -1, g_r = -1;

    /* Check adjacent grids */
    for (i = 0; i < 8; i++) {

        /* Direction */
        int d = ddd[i];

        /* Location */
        int x = c_x + ddx[d];
        int y = c_y + ddy[d];

        /* Grid */
        auto_grid *ag = grid(x, y);

        /* Skip non-monsters */
        if (auto_char[(byte)(ag->o_c)] != AUTO_CHAR_KILL) continue;

        /* Count monsters */
        n++;

        /* Guess the monster */
        r = borg_guess_race(x, y);

        /* Mega-Hack -- Use the "hardest" monster */
        if ((g_d >= 0) && (r < g_r)) continue;

        /* Save the info */
        g_d = d; g_r = r;
    }

    /* Nothing to attack */
    if (!n) return (FALSE);

    /* Do not attack when afraid */
    if (do_afraid) {
        borg_note(format("Fearing (%s).", r_list[g_r].name));
        return (FALSE);
    }

    /* Mention monster */
    if (n > 1) {
        borg_note("Selecting an adjacent monster.");
    }
    else {
        borg_note("Attacking an adjacent monster.");
    }
    
    /* Hack -- Attack! */
    borg_keypress('0' + g_d);

    /* Success */
    return (TRUE);
}




/*
 * Fire a missile, if possible
 */
static bool borg_fire_missile(void)
{
    int i, n = -1;

    /* Scan the pack */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Skip bad missiles */
        if (item->tval != auto_tval_ammo) continue;

        /* Skip un-identified, non-average, missiles */
        if (!item->able && !streq(item->note, "{average}")) continue;

        /* Find the smallest pile */
        if ((n < 0) || (item->iqty < auto_items[n].iqty)) n = i;
    }

    /* Use that missile */
    if (n >= 0) {
    
        borg_note("Firing standard missile");
        
        borg_keypress('f');
        borg_keypress('a' + n);
        
        return (TRUE);
    }

    /* Nope */
    return (FALSE);
}


/*
 * Fire anything, if possible
 */
static bool borg_fire_anything(void)
{
    int i, n = -1;

    /* Use any missile */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Acceptable missiles */
        if ((item->tval == TV_BOLT) ||
            (item->tval == TV_ARROW) ||
            (item->tval == TV_SHOT)) {

            /* Skip un-identified, non-average, missiles */
            if (!item->able && !streq(item->note, "{average}")) continue;

            /* Find the smallest pile */
            if ((n < 0) || (item->iqty < auto_items[n].iqty)) n = i;
        }
    }

    /* Use that missile */
    if (n >= 0) {
    
        borg_note("Firing left-over missile");
        
        borg_keypress('f');
        borg_keypress('a' + n);
        
        return (TRUE);
    }

    /* Nope */
    return (FALSE);
}





/*
 * Attempt to fire at monsters
 */
bool borg_play_fire(void)
{
    int i, d, r, p;

    int f_r = 0;
    int f_p = -1;
    int f_d = -1;
    int f_x = c_x;
    int f_y = c_y;

    byte old_mushroom = auto_char[(byte)(',')];
    

    /* XXX XXX XXX Mega-Hack -- attack mushrooms */
    if ((rand_int(100) < 10) ||
        (do_afraid && (rand_int(100) < 10)) ||
        ((auto_chp < auto_mhp) && (rand_int(100) < 50))) {

        /* Kill mushrooms */
        auto_char[(byte)(',')] = AUTO_CHAR_KILL;
    }

    /* Look for something to kill */
    for (i = 0; i < view_n; i++) {

        /* Access the "view grid" */
        int y = view_y[i];
        int x = view_x[i];

        /* Get the auto_grid */
        auto_grid *ag = grid(x, y);


        /* Skip the player grid */
        if ((c_x == x) && (c_y == y)) continue;

        /* Never shoot walls */
        if (ag->info & BORG_WALL) continue;

        /* Never shoot at "dark" grids */
        if (!(ag->info & BORG_OKAY)) continue;

        /* Never shoot at off-panel grids */
        /* if (!(ag->info & BORG_HERE)) continue; */

        /* Skip unknown grids */
        /* if (ag->o_c == ' ') continue; */

        /* Notice monsters */
        if (auto_char[(byte)(ag->o_c)] != AUTO_CHAR_KILL) continue;

        /* Check distance */
        d = distance(c_y, c_x, y, x);

        /* Must be within "range" */
        if (d > 8) continue;

        /* Must have "missile line of sight" */
        if (!borg_projectable(c_x, c_y, x, y)) continue;

        /* Acquire monster type */
        r = borg_guess_race(x, y);

        /* Acquire monster power */
        p = borg_danger_monster(r);

        /* Hack -- ignore "easy" town monsters */
        if (!auto_depth && (p < auto_chp)) continue;
        
        /* Hack -- Lower power by distance */
        p -= d;
                
        /* Choose hardest monster */
        if (p < f_p) continue;

        /* Save the distance */
        f_r = r; f_p = p; f_d = d; f_x = x; f_y = y;
    }

    /* Restore Mushroom flags */
    auto_char[(byte)(',')] = old_mushroom;

    /* Nothing to attack */
    if (f_p < 0) return (FALSE);


    /* Unless "worried", do not fire a lot */
    if (!do_afraid && (f_p < auto_chp / 2)) {
    
        /* Skip close monsters, and most monsters */
        if ((f_d <= 1) || (rand_int(100) < 20)) return (FALSE);
    }


    /* Magic Missile */
    if (borg_spell(0,0)) {

        /* Note */
        borg_note(format("Targetting (%s).", r_list[f_r].name));

        /* Target the monster (always works) */
        if (!borg_target(f_x, f_y)) borg_flush();

        /* Success */
        return (TRUE);
    }
    
    /* Physical missile */
    else if (borg_fire_missile() ||
             borg_fire_anything()) {

        /* Note */
        borg_note(format("Targetting (%s).", r_list[f_r].name));

        /* Target the monster (always works) */
        if (!borg_target(f_x, f_y)) borg_flush();

        /* Success */
        return (TRUE);
    }


    /* Oh well */
    return (FALSE);
}










/*
 * Process a "goto" goal, return "TRUE" if goal is still okay.
 */
static bool borg_play_step(int x2, int y2)
{
    auto_grid *ag;

    int dir, x, y, p1, p;


    /* We have arrived */
    if ((c_x == x2) && (c_y == y2)) return (FALSE);

    /* Get a direction (may be a wall there) */
    dir = borg_goto_dir(c_x, c_y, x2, y2);

    /* We are confused */
    if ((dir == 0) || (dir == 5)) return (FALSE);

    /* Obtain the destination */
    x = c_x + ddx[dir];
    y = c_y + ddy[dir];

    /* Access the grid we are stepping on */
    ag = grid(x, y);


    /* Must "disarm" traps */
    if (ag->o_c == '^') {
        borg_info("Disarming a trap.");
        borg_keypress('D');
        borg_keypress('0' + dir);
        return (TRUE);
    }

    /* Occasionally "bash" doors */
    if ((ag->o_c == '+') && (rand_int(10) == 0)) {
        borg_info("Bashing a door.");
        borg_keypress('B');
        borg_keypress('0' + dir);
        return (TRUE);
    }

    /* Must "open" (or "bash") doors */
    if (ag->o_c == '+') {
        borg_info("Opening a door.");
        borg_keypress('0');
        borg_keypress('9');
        borg_keypress('o');
        borg_keypress('0' + dir);
        return (TRUE);
    }

    /* Tunnel through rubble */
    if (ag->o_c == ':') {
        borg_info("Digging rubble.");
        borg_keypress('0');
        borg_keypress('9');
        borg_keypress('9');
        borg_keypress('T');
        borg_keypress('0' + dir);
        return (TRUE);
    }

    /* Walls -- Tunnel (or give up) */
    if ((ag->o_c == '#') || (ag->o_c == '%')) {

        /* XXX XXX XXX Mega-Hack -- prevent infinite loops */
        if (rand_int(100) < 10) return (FALSE);

        /* Tunnel into it */
        borg_info("Digging a wall.");
        borg_keypress('0');
        borg_keypress('9');
        borg_keypress('9');
        borg_keypress('T');
        borg_keypress('0' + dir);
        return (TRUE);
    }

    /* Gold -- sometimes tunnel */
    if ((ag->o_c == '$') || (ag->o_c == '*')) {

        /* XXX XXX XXX Mega-Hack -- Assume walls */
        if (rand_int(100) < 10) {

            /* Note */
            borg_note("Hack -- Assuming embedded gold/gems!");
            
            /* Assume the grid is a wall */
            ag->info |= BORG_WALL;
            
            /* Cancel all goals */
            goal = 0;

            /* Move anyway */
            borg_keypress('0' + dir);
            return (TRUE);
        }

        /* XXX XXX XXX Mega-Hack -- Handle walls */
        if ((ag->info & BORG_WALL) && (rand_int(100) < 90)) {

            /* Tunnel for it */
            borg_info("Digging for embedded gold/gems.");
            borg_keypress('0');
            borg_keypress('9');
            borg_keypress('9');
            borg_keypress('T');
            borg_keypress('0' + dir);
            return (TRUE);
        }

        borg_info("Taking gold/gems.");
        borg_keypress('0' + dir);
        return (TRUE);
    }

    /* Attack a monster (?) */
    if (auto_char[(byte)(ag->o_c)] == AUTO_CHAR_KILL) {
        borg_note("Accidental attack!");
        borg_keypress('0' + dir);
        return (TRUE);
    }

    /* Enter a shop */
    if (auto_char[(byte)(ag->o_c)] == AUTO_CHAR_SHOP) {
        borg_note("Entering a shop.");
        borg_keypress('0' + dir);
        return (TRUE);
    }

    /* Get something */
    if (auto_char[(byte)(ag->o_c)] == AUTO_CHAR_TAKE) {
        borg_note("Taking something.");
        borg_keypress('0' + dir);
        return (TRUE);
    }



    /* Hack -- check danger here */
    p1 = borg_danger(c_x,c_y);

    /* Hack -- check danger there */
    p = borg_danger(x,y);

    /* Hack -- be paranoid */
    if ((p > p1) &&
        ((p > auto_chp) ||
         ((p > auto_chp / 2) && (rand_int(100) < 50)))) {

        /* Note */
        borg_note(format("Paranoia (%d > %d)", p, p1));

        /* Stay still */
        borg_keypress('5');

        /* Success */
        return (TRUE);
    }


    /* Walk (or tunnel or open or bash) in that direction */
    borg_keypress('0' + dir);

    /* Mega-Hack -- prepare to take stairs if desired */
    if (stair_less && (ag->o_c == '<')) borg_keypress('<');
    if (stair_more && (ag->o_c == '>')) borg_keypress('>');

    /* Did something */
    return (TRUE);
}




/*
 * Take a step towards the current goal location
 * Return TRUE if this goal is still "okay".
 * Otherwise, cancel the goal and return FALSE.
 */
bool borg_play_old_goal(void)
{
    int x, y;
    
    /* Flow towards the goal */
    if (goal) {

        /* Get the best direction */
        borg_flow_best(&x, &y, c_x, c_y);

        /* Attempt to take one step in the path */
        if (borg_play_step(x, y)) return (TRUE);
    }

    /* Cancel goals */
    goal = 0;

    /* Nothing to do */
    return (FALSE);
}





/*
 * Determine if a grid touches unknown grids
 */
static bool borg_interesting(int x1, int y1)
{
    int i, x, y;

    auto_grid *ag;
    
    /* Scan all eight neighbors */
    for (i = 0; i < 8; i++) {

        /* Get the location */
        x = x1 + ddx[ddd[i]];
        y = y1 + ddy[ddd[i]];

        /* Get the grid */
        ag = grid(x, y);

        /* Unknown grids are interesting */	
        if (ag->o_c == ' ') return (TRUE);

#ifdef BORG_ROOMS

        /* Known walls are boring */
        if (ag->info & BORG_WALL) continue;

        /* Unroomed grids are interesting */
        if (!ag->room) return (TRUE);

#endif

    }

    /* Assume not */
    return (FALSE);
}







/*
 * Prepare to "flow" towards monsters to "kill"
 */
bool borg_flow_kill()
{
    int i, x, y, r, p;

    auto_grid *ag;


    /* Nothing found */
    seen_n = 0;

    /* Look for something to kill */
    for (i = 0; i < view_n; i++) {

        /* Access the "view grid" */
        y = view_y[i];
        x = view_x[i];

        /* Get the auto_grid */
        ag = grid(x, y);

        /* Skip the player grid */
        if ((c_x == x) && (c_y == y)) continue;

        /* Skip unknown grids */
        if (ag->o_c == ' ') continue;

        /* Notice monsters */
        if (auto_char[(byte)(ag->o_c)] == AUTO_CHAR_KILL) {

            /* Obtain the "monster race" */
            r = borg_guess_race(x,y);
            
            /* Obtain the "monster power" */
            p = borg_danger_monster(r);

            /* Skip deadly monsters */
            if (p > auto_chp) continue;

            /* Remember it */
            if (seen_n < SEEN_MAX) {
                seen_x[seen_n] = x;
                seen_y[seen_n] = y;
                seen_n++;
            }
        }
    }

    /* Nothing to kill */
    if (!seen_n) return (FALSE);


    /* Clear the flow codes */
    borg_flow_clear();

    /* Look for something to kill */
    for (i = 0; i < seen_n; i++) {

        /* Enqueue the grid */
        borg_flow_enqueue_grid(seen_x[i], seen_y[i]);
    }

    /* Clear the seen array */
    seen_n = 0;

    /* Spread the flow a little */
    borg_flow_spread(TRUE);

    /* Extract the "best" cost */
    i = borg_flow_cost(c_x, c_y);

    /* No good path */
    if (i >= 9999) return (FALSE);

    /* Note (happens a LOT) */
    if (auto_fff) borg_info(format("Flowing toward monsters, at cost %d", i));

    /* Set the "goal" */
    goal = GOAL_KILL;

    /* Success */
    return (TRUE);
}


/*
 * Prepare to "flow" towards objects to "take"
 */
bool borg_flow_take()
{
    int i, x, y;

    auto_grid *ag;


    /* Nothing yet */
    seen_n = 0;

    /* Look for something to take */
    for (i = 0; i < view_n; i++) {

        /* Access the "view grid" */
        y = view_y[i];
        x = view_x[i];

        /* Get the auto_grid */
        ag = grid(x, y);

        /* Skip the player grid */
        if ((c_x == x) && (c_y == y)) continue;

        /* Skip unknown grids */
        if (ag->o_c == ' ') continue;

        /* Only notice objects */
        if (auto_char[(byte)(ag->o_c)] == AUTO_CHAR_TAKE) {

            /* XXX XXX Hack -- avoid gold in walls when weak */
            if (((ag->o_c == '$') || (ag->o_c == '*')) &&
                (ag->info & BORG_WALL) &&
                (auto_stat[A_STR] < 18 + 20)) continue;

            /* Remember it */
            if (seen_n < SEEN_MAX) {
                seen_x[seen_n] = x;
                seen_y[seen_n] = y;
                seen_n++;
            }
        }
    }

    /* Nothing to take */
    if (!seen_n) return (FALSE);


    /* Clear the flow codes */
    borg_flow_clear();

    /* Look for something to take */
    for (i = 0; i < seen_n; i++) {

        /* Enqueue the grid */
        borg_flow_enqueue_grid(seen_x[i], seen_y[i]);
    }

    /* Clear the seen array */
    seen_n = 0;

    /* Spread the flow (a little) */
    borg_flow_spread(TRUE);

    /* Extract the "best" cost */
    i = borg_flow_cost(c_x, c_y);

    /* No good path */
    if (i >= 9999) return (FALSE);

    /* Note (happens a LOT!) */
    if (auto_fff) borg_info(format("Flowing toward objects, at cost %d", i));

    /* Set the "goal" */
    goal = GOAL_TAKE;

    /* Success */
    return (TRUE);
}


/*
 * Prepare to "flow" towards "dark" or "unknown" grids
 */
bool borg_flow_dark()
{
    int i, x, y;

    auto_grid *ag;


    /* Nothing yet */
    seen_n = 0;

    /* Look for something unknown */
    for (i = 0; i < view_n; i++) {

        /* Access the "view grid" */
        y = view_y[i];
        x = view_x[i];

        /* Get the auto_grid */
        ag = grid(x, y);

        /* Skip the player grid */
        if ((c_x == x) && (c_y == y)) continue;

        /* Skip unknown grids */
        if (ag->o_c == ' ') continue;

        /* Cannot explore walls */
        if (ag->info & BORG_WALL) continue;

        /* Notice interesting grids */
        if (borg_interesting(x, y)) {

            /* Remember it */
            if (seen_n < SEEN_MAX) {
                seen_x[seen_n] = x;
                seen_y[seen_n] = y;
                seen_n++;
            }
        }
    }

    /* Nothing dark */
    if (!seen_n) return (FALSE);


    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < seen_n; i++) {

        /* Enqueue the grid */
        borg_flow_enqueue_grid(seen_x[i], seen_y[i]);
    }

    /* Clear the seen array */
    seen_n = 0;

    /* Spread the flow (a little) */
    borg_flow_spread(TRUE);

    /* Extract the "best" cost */
    i = borg_flow_cost(c_x, c_y);

    /* No good path */
    if (i >= 9999) return (FALSE);

    /* Hack */
    if (!auto_depth) borg_note(format("Flowing toward unknown grids, at cost %d", i));

    /* Note (happens a LOT!) */
    if (auto_fff) borg_info(format("Flowing toward unknown grids, at cost %d", i));

    /* Set the "goal" */
    goal = GOAL_DARK;

    /* Success */
    return (TRUE);
}


/*
 * Prepare to "flow" towards "interesting" things
 */
bool borg_flow_explore(void)
{
    int x, y, i;

    auto_grid *ag;


    /* Nothing yet */
    seen_n = 0;

    /* Examine every legal grid */
    for (y = 1; y < AUTO_MAX_Y-1; y++) {
        for (x = 1; x < AUTO_MAX_X-1; x++) {

            /* Get the grid */
            ag = grid(x, y);

            /* Skip the player grid */
            if ((c_x == x) && (c_y == y)) continue;

            /* Skip stuff */
            if (ag->o_c == ' ') continue;

#ifdef BORG_ROOMS
            if (!ag->room) continue;
#endif

            /* Cannot explore walls */
            if (ag->info & BORG_WALL) continue;

            /* Only examine "interesting" grids */
            if (!borg_interesting(x, y)) continue;

            /* Remember it */
            if (seen_n < SEEN_MAX) {
                seen_x[seen_n] = x;
                seen_y[seen_n] = y;
                seen_n++;
            }
        }
    }

    /* Nothing useful */
    if (!seen_n) return (FALSE);


    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < seen_n; i++) {

        /* Enqueue the grid */
        borg_flow_enqueue_grid(seen_x[i], seen_y[i]);
    }

    /* Clear the seen array */
    seen_n = 0;

    /* Spread the flow (a little) */
    borg_flow_spread(TRUE);

    /* Extract the "best" cost */
    i = borg_flow_cost(c_x, c_y);

    /* No good path */
    if (i >= 9999) return (FALSE);

    /* Note */
    borg_note(format("Chasing unknowns, at cost %d", i));

    /* Set the "goal" */
    goal = GOAL_DARK;

    /* Success */
    return (TRUE);
}


/*
 * Prepare to "flow" towards "old" monsters
 */
bool borg_flow_kill_any(void)
{
    int i, x, y, r, p;

    auto_grid *ag;


    /* Efficiency -- Nothing to kill */
    if (!count_kill) return (FALSE);


    /* Nothing yet */
    seen_n = 0;

    /* Examine every legal grid */
    for (y = 1; y < AUTO_MAX_Y-1; y++) {
        for (x = 1; x < AUTO_MAX_X-1; x++) {

            /* Get the grid */
            ag = grid(x, y);

            /* Skip the player grid */
            if ((c_x == x) && (c_y == y)) continue;

            /* Skip stuff */
            if (ag->o_c == ' ') continue;

#ifdef BORG_ROOMS
            if (!ag->room) continue;
#endif

            /* Skip non-monsters */
            if (auto_char[(byte)(ag->o_c)] != AUTO_CHAR_KILL) continue;

            /* Obtain the "monster race" */
            r = borg_guess_race(x,y);
            
            /* Obtain the "monster power" */
            p = borg_danger_monster(r);

            /* Skip deadly monsters */
            if (p > auto_chp) continue;

            /* Remember it */
            if (seen_n < SEEN_MAX) {
                seen_x[seen_n] = x;
                seen_y[seen_n] = y;
                seen_n++;
            }
        }
    }

    /* Nothing useful */
    if (!seen_n) return (FALSE);


    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < seen_n; i++) {

        /* Enqueue the grid */
        borg_flow_enqueue_grid(seen_x[i], seen_y[i]);
    }

    /* Clear the "seen" array */
    seen_n = 0;

    /* Spread the flow (a little) */
    borg_flow_spread(TRUE);

    /* Extract the "best" cost */
    i = borg_flow_cost(c_x, c_y);

    /* No good path */
    if (i >= 9999) return (FALSE);

    /* Note */
    borg_note(format("Chasing monsters, at cost %d", i));

    /* Set the "goal" */
    goal = GOAL_KILL;

    /* Success */
    return (TRUE);
}


/*
 * Prepare to "flow" towards "old" objects
 */
bool borg_flow_take_any(void)
{
    int i, x, y;

    auto_grid *ag;


    /* Efficiency -- Nothing to take */
    if (!count_take) return (FALSE);


    /* Nothing yet */
    seen_n = 0;

    /* Examine every legal grid */
    for (y = 1; y < AUTO_MAX_Y-1; y++) {
        for (x = 1; x < AUTO_MAX_X-1; x++) {

            /* Get the grid */
            ag = grid(x, y);

            /* Skip the player grid */
            if ((c_x == x) && (c_y == y)) continue;

            /* Skip stuff */
            if (ag->o_c == ' ') continue;

#ifdef BORG_ROOMS
            if (!ag->room) continue;
#endif

            /* Skip non-objects */
            if (auto_char[(byte)(ag->o_c)] != AUTO_CHAR_TAKE) continue;

            /* XXX XXX Hack -- avoid gold in walls when weak */
            if (((ag->o_c == '$') || (ag->o_c == '*')) &&
                (ag->info & BORG_WALL) &&
                (auto_stat[A_STR] < 18 + 20)) continue;

            /* Remember it */
            if (seen_n < SEEN_MAX) {
                seen_x[seen_n] = x;
                seen_y[seen_n] = y;
                seen_n++;
            }
        }
    }

    /* Nothing useful */
    if (!seen_n) return (FALSE);


    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < seen_n; i++) {

        /* Enqueue the grid */
        borg_flow_enqueue_grid(seen_x[i], seen_y[i]);
    }

    /* Clear the "seen" array */
    seen_n = 0;

    /* Spread the flow (a little) */
    borg_flow_spread(TRUE);

    /* Extract the "best" cost */
    i = borg_flow_cost(c_x, c_y);

    /* No good path */
    if (i >= 9999) return (FALSE);

    /* Note */
    borg_note(format("Chasing objects, at cost %d", i));

    /* Set the "goal" */
    goal = GOAL_TAKE;

    /* Success */
    return (TRUE);
}





/*
 * Prepare to "flow" towards "interesting" things
 */
bool borg_flow_symbol(char what)
{
    int i, x, y;

    auto_grid *ag;


    /* Nothing yet */
    seen_n = 0;

    /* Examine every legal grid */
    for (y = 1; y < AUTO_MAX_Y-1; y++) {
        for (x = 1; x < AUTO_MAX_X-1; x++) {

            /* Get the grid */
            ag = grid(x, y);

            /* Skip the player grid */
            if ((c_x == x) && (c_y == y)) continue;

            /* Skip stuff */
            if (ag->o_c == ' ') continue;

#ifdef BORG_ROOMS
            if (!ag->room) continue;
#endif

            /* Skip incorrect symbols */
            if (ag->o_c != what) continue;

            /* Remember it */
            if (seen_n < SEEN_MAX) {
                seen_x[seen_n] = x;
                seen_y[seen_n] = y;
                seen_n++;
            }
        }
    }

    /* Nothing useful */
    if (!seen_n) return (FALSE);


    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < seen_n; i++) {

        /* Enqueue the grid */
        borg_flow_enqueue_grid(seen_x[i], seen_y[i]);
    }

    /* Clear the "seen" array */
    seen_n = 0;

    /* Spread the flow (a little) */
    borg_flow_spread(TRUE);

    /* Extract the "best" cost */
    i = borg_flow_cost(c_x, c_y);

    /* No good path */
    if (i >= 9999) return (FALSE);

    /* Note */
    borg_note(format("Chasing symbols (%c), at cost %d", what, i));

    /* Set the "goal" */
    goal = GOAL_XTRA;

    /* Success */
    return (TRUE);
}







/*
 * Walk around the dungeon looking for monsters
 */
bool borg_flow_revisit(void)
{
    int x, y, i;

    int r_n = -1;
    
#ifdef BORG_ROOMS

    int r_x, r_y;

    s32b r_age = 0L;

    s32b age;


    auto_room *ar;

    /* First find the reachable spaces */
    borg_flow_reverse();

    /* Re-visit "old" rooms */
    for (i = 1; i < auto_room_max; i++) {

        /* Access the "room" */
        ar = &auto_rooms[i];

        /* Skip "dead" rooms */
        if (ar->free) continue;

        /* Skip "unreachable" rooms */
        if (ar->cost >= 9999) continue;

        /* Hack -- skip "boring" rooms */
        /* if ((ar->x1 == ar->x2) || (ar->y1 == ar->y2)) continue; */

        /* Reward "age" and "distance" and "luck" */
        age = (c_t - ar->when) + (ar->cost / 2);

        /* Skip "recent" rooms */
        if ((r_n >= 0) && (age < r_age)) continue;

        /* Save the index, and the age */
        r_n = i; r_age = age;
    }

    /* Clear the flow codes */
    borg_flow_clear();

    /* Hack -- No rooms to visit (!) */
    if (r_n < 0) return (FALSE);

    /* Get the room */
    ar = &auto_rooms[r_n];

    /* Visit a random grid of that room */
    r_x = rand_range(ar->x1, ar->x2);
    r_y = rand_range(ar->y1, ar->y2);

    /* Enqueue the room */
    borg_flow_enqueue_grid(r_x, r_y);

    /* Spread the flow */
    borg_flow_spread(TRUE);

    /* Extract the "best" cost */
    i = borg_flow_cost(c_x, c_y);

    /* No good path */
    if (i >= 9999) return (FALSE);

    /* Note */
    borg_note(format("Revisiting (%d, %d) with age %ld, at cost %d",
                     r_x, r_y, r_age, i));

    /* Set the "goal" */
    goal = GOAL_XTRA;

#else	/* BORG_ROOMS */

    /* Clear */
    seen_n = 0;
    
    /* First find the reachable spaces */
    borg_flow_reverse();

    /* Examine every legal grid */
    for (y = 1; y < AUTO_MAX_Y-1; y++) {
        for (x = 1; x < AUTO_MAX_X-1; x++) {

            /* Get the grid */
            auto_grid *ag = grid(x, y);

            /* Skip the player grid */
            if ((c_x == x) && (c_y == y)) continue;

            /* Skip "unreachable" grids */
            if (ag->cost >= 9999) continue;

            /* Hack -- Skip "nearby" grids */
            if (ag->cost < 50) continue;

            /* Hack -- Skip most of the grids */
            if (rand_int(100) < 90) continue;

            /* Remember it */
            if (seen_n < SEEN_MAX) {
                seen_x[seen_n] = x;
                seen_y[seen_n] = y;
                seen_n++;
            }
        }
    }

    /* Clear the flow codes */
    borg_flow_clear();

    /* Save the number of grids */
    r_n = seen_n;
    
    /* Nothing to revisit */
    if (!seen_n) return (FALSE);
    
    /* Clear the flow codes */
    borg_flow_clear();

    /* Look for something to kill */
    for (i = 0; i < seen_n; i++) {

        /* Enqueue the grid */
        borg_flow_enqueue_grid(seen_x[i], seen_y[i]);
    }

    /* Clear the seen array */
    seen_n = 0;

    /* Spread the flow a little */
    borg_flow_spread(TRUE);

    /* Extract the "best" cost */
    i = borg_flow_cost(c_x, c_y);

    /* No good path */
    if (i >= 9999) return (FALSE);

    /* Note */
    borg_note(format("Revisiting %d random grids, at cost %d", r_n, i));

    /* Set the "goal" */
    goal = GOAL_XTRA;

#endif

    /* Success */
    return (TRUE);
}


/*
 * Heuristic -- Help locate secret doors (see below)
 *
 * Determine the "likelihood" of a secret door touching "(x,y)",
 * which is "d" grids away from the player.
 *
 * Assume grid is legal, non-wall, and reachable.
 * Assume grid neighbors are legal and known.
 */
static int borg_secrecy(int x, int y, int d)
{
    int		i, v;
    int		wall = 0, supp = 0, diag = 0;

    char	cc[8];

    auto_grid	*ag;


    /* No secret doors in town */
    if (!auto_depth) return (0);


    /* Get the central grid */
    ag = grid(x, y);

    /* Tweak -- Limit total searches */
    if (ag->xtra > 50) return (0);


    /* Extract adjacent locations */
    for (i = 0; i < 8; i++) {

        /* Extract the location */
        int xx = x + ddx[ddd[i]];
        int yy = y + ddy[ddd[i]];

        /* Get the grid */
        cc[i] = grid(xx,yy)->o_c;
    }


    /* Count possible door locations */
    for (i = 0; i < 4; i++) if (cc[i] == '#') wall++;

    /* No possible secret doors */
    if (wall <= 0) return (0);


    /* Count supporting evidence for secret doors */
    for (i = 0; i < 4; i++) {
        if ((cc[i] == '#') || (cc[i] == '%')) supp++;
        else if ((cc[i] == '+') || (cc[i] == '\'')) supp++;
    }

    /* Count supporting evidence for secret doors */
    for (i = 4; i < 8; i++) {
        if ((cc[i] == '#') || (cc[i] == '%')) diag++;
    }


    /* Tweak -- Reward walls, punish visitation and distance */
    v = (supp * 300) + (diag * 100) - (ag->xtra * 20) - (d * 1);

    /* Result */
    return (v);
}



/*
 * Search carefully for secret doors and such
 */
bool borg_flow_spastic(void)
{
    int i, x, y, v;

    int g_x = c_x;
    int g_y = c_y;
    int g_v = -1;


    int boredom;

#ifdef BORG_ROOMS
    auto_room *ar;
#endif


    /* Hack -- Tweak -- Determine boredom */
    boredom = count_floor - 500;
    if (boredom < 200) boredom = 200;
    if (boredom > 800) boredom = 800;

    /* Tweak -- Search based on dungeon knowledge */
    if (c_t - auto_shock < boredom) return (FALSE);


    /* Hack -- first find the reachable spaces */
    borg_flow_reverse();

    /* Scan the entire map */
    for (y = 1; y < AUTO_MAX_Y-1; y++) {
        for (x = 1; x < AUTO_MAX_X-1; x++) {

            auto_grid *ag = grid(x, y);

            /* Skip stuff */
            if (ag->o_c == ' ') continue;

            /* Cannot search inside walls */
            if (ag->info & BORG_WALL) continue;

#ifdef BORG_ROOMS

            /* Require a room */
            if (!ag->room) continue;

            /* Scan the rooms */
            for (ar = room(1,x,y); ar; ar = room(0,0,0)) {

                /* Skip "unreachable" rooms */
                if (ar->cost >= 9999) continue;

                /* Get the "searchability" */
                v = borg_secrecy(x, y, ar->cost);

                /* The grid is not searchable */
                if (v <= 0) continue;

                /* Skip non-perfect grids */
                if ((g_v >= 0) && (v < g_v)) continue;

                /* Save the data */
                g_v = v; g_x = x; g_y = y;
            }

#else	/* BORG_ROOMS */

            /* Skip "unreachable" grids */
            if (ag->cost >= 9999) continue;

            /* Get the "searchability" */
            v = borg_secrecy(x, y, ag->cost);

            /* The grid is not searchable */
            if (v <= 0) continue;

            /* Skip non-perfect grids */
            if ((g_v >= 0) && (v < g_v)) continue;

            /* Save the data */
            g_v = v; g_x = x; g_y = y;

#endif

        }
    }

    /* Clear the flow codes */
    borg_flow_clear();

    /* Hack -- Nothing found */
    if (g_v < 0) return (FALSE);


    /* We have arrived */
    if ((c_x == g_x) && (c_y == g_y)) {

        /* Take note */
        borg_note("Spastic Searching...");

        /* Tweak -- Remember the search */
        if (grid(c_x,c_y)->xtra < 100) grid(c_x,c_y)->xtra += 9;

        /* Tweak -- Search a little */
        borg_keypress('0');
        borg_keypress('9');
        borg_keypress('s');

        /* Success */
        return (TRUE);
    }


    /* Enqueue the grid */
    borg_flow_enqueue_grid(g_x, g_y);

    /* Spread the flow a little */
    borg_flow_spread(TRUE);

    /* Extract the "best" cost */
    i = borg_flow_cost(c_x, c_y);

    /* No good path */
    if (i >= 9999) return (FALSE);

    /* Note */
    borg_note(format("Spastic twitch towards (%d,%d)", g_x, g_y));

    /* Set the "goal" */
    goal = GOAL_XTRA;

    /* Success */
    return (TRUE);
}




/*
 * Hack -- set the options the way we like them
 */
static void borg_play_options(void)
{
    /* The Borg uses the original keypress codes */
    rogue_like_commands = FALSE;

    /* Use color to identify monsters and such */
    use_color = TRUE;

    /* Pick up items when stepped on */
    always_pickup = TRUE;

    /* Require explicit target request */
    use_old_target = FALSE;

    /* Do NOT query "throw" commands */
    always_throw = TRUE;

    /* Do NOT query "pick up" commands */
    carry_query_flag = FALSE;

    /* Do NOT query "various" commands */
    other_query_flag = FALSE;

    /* Require explicit repeat commands */
    always_repeat = FALSE;

    /* Do not get confused by extra info */
    plain_descriptions = TRUE;

    /* Maximize space for information */
    show_inven_weight = FALSE;
    show_equip_weight = FALSE;
    show_store_weight = FALSE;

    /* Buy/Sell without haggling */
    no_haggle_flag = TRUE;

    /* Maximize screen info */
    fresh_before = TRUE;
    fresh_after = TRUE;

    /* Read the level directly (not in feet) */
    depth_in_feet = FALSE;

    /* Use the health bar (later) */
    show_health_bar = TRUE;

    /* Do not get confused by extra spell info */
    show_spell_info = FALSE;
    
    /* Do not get confused by equippy chars */
    equippy_chars = FALSE;


    /* XXX Hack -- notice "command" mode */
    hilite_player = FALSE;


    /* XXX Mega-Hack -- pre-set "preserve" mode */
    p_ptr->preserve = TRUE;
}




/*
 * Initialize this file
 */
void borg_ext_init(void)
{
    int i;

    byte t_a;
    
    char buf[80];
    

    /*** Init the lowest module ***/

    /* Init "borg.c" */
    borg_init();
    

    /*** Init the medium modules ***/

    /* Init "borg-map.c" */
    borg_map_init();

    /* Init "borg-obj.c" */
    borg_obj_init();




    /*** Hack -- initialize options ***/
    
    /* Hack -- Verify options */
    borg_play_options();



    /*** Hack -- Extract race ***/
    
    /* Check for textual race */
    if (0 == borg_what_text(COL_RACE, ROW_RACE, -12, &t_a, buf)) {

        /* Scan the races */
        for (i = 0; i < 10; i++) {
    
            /* Check the race */
            if (prefix(buf, race_info[i].trace)) {

                /* We got one */
                auto_race = i;
                break;
            }
        }
    }

    /* Extract the race pointer */
    rb_ptr = &race_info[auto_race];
    

    /*** Hack -- Extract class ***/
    
    /* Check for textual class */
    if (0 == borg_what_text(COL_CLASS, ROW_CLASS, -12, &t_a, buf)) {
    
        /* Scan the classes */
        for (i = 0; i < 6; i++) {
    
            /* Check the race */
            if (prefix(buf, class_info[i].title)) {

                /* We got one */
                auto_class = i;
                break;
            }
        }
    }

    /* Extract the class pointer */
    cb_ptr = &class_info[auto_class];
    
    /* Extract the magic pointer */
    mb_ptr = &magic_info[auto_class];
    

    /*** Hack -- Analyze the Frame ***/
    
    /* Analyze the "frame" */
    borg_update_frame();
    

    /*** Hack -- react to race and class ***/
    
    /* Notice the new race and class */
    prepare_race_class_info();


    /*** Hack -- analyze some characters ***/
    
    /* Analyze various characters */
    for (i = 33; i < 128; i++) {

        /* Shops -- '1' to '8' */
        if (strchr("12345678", i)) {

            auto_char[i] = AUTO_CHAR_SHOP;
        }
        
        /* Okay -- Floors, open doors, stairs */
        else if (strchr(".'<>", i)) {
        
            auto_char[i] = AUTO_CHAR_OKAY;
        }

        /* Take -- Objects */
        else if (strchr("?!_-\\|/\"=~{([])},", i)) {
        
            auto_char[i] = AUTO_CHAR_TAKE;
        }

        /* Take -- Gold */
        else if (strchr("$*", i)) {
        
            auto_char[i] = AUTO_CHAR_TAKE;
        }

        /* Take -- Traps, Doors, Rubble */
        else if (strchr("^+:;", i)) {
        
            auto_char[i] = AUTO_CHAR_TAKE;
        }

        /* Walls -- Walls and Veins */
        else if (strchr("#%", i)) {
        
            auto_char[i] = AUTO_CHAR_WALL;
        }

        /* Extra -- The player */
        else if (i == '@') {

            auto_char[i] = AUTO_CHAR_XTRA;
        }

        /* Monsters -- Letters (and '&') */
        else if (isalpha(i) || (i == '&')) {
        
            auto_char[i] = AUTO_CHAR_KILL;
        }
    }
}




#endif

