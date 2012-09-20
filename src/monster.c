/* File: monster.c */

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
 * Actually remove a monster record.
 * Always call "delete_monster()" first.
 *
 * Currently, only called by "process_monsters()" and
 * "tighten_m_list()" and "wipe_m_list()".
 *
 * Note the careful use of the "m_idx" field in cave grids.  This is
 * necessary to prevent problems during "polymorph" attacks and when
 * one monster "eats" another and even if one monster dies and another
 * "blinks" into the grid it used to occupy.  See below.
 */
void remove_monster_idx(int i)
{
    monster_type *m_ptr = &m_list[i];
    monster_lore *l_ptr = &l_list[m_ptr->r_idx];


    /* One less of this monster on this level */
    l_ptr->cur_num--;

    /* Hack -- remove target monster */
    if (i == target_who) target_who = 0;

    /* Hack -- remove tracked monster */
    if (i == health_who) health_track(0);

    /* One less monster */
    m_max--;

    /* Do structure dumping */
    if (i != m_max) {

        int ny = m_list[m_max].fy;
        int nx = m_list[m_max].fx;

        /* Hack -- prepare to slide the monster */
        if (cave[ny][nx].m_idx == m_max) cave[ny][nx].m_idx = i;

        /* Hack -- Sliding target monster */
        if (target_who == (int)(m_max)) target_who = i;

        /* Hack -- Sliding tracked monster */
        if (health_who == (int)(m_max)) health_track(i);

        /* Structure copy the final monster onto the dead monster */
        m_list[i] = m_list[m_max];
    }

    /* Wipe the monster record */
    m_list[m_max] = m_list[0];
}


/*
 * Delete a monster by index.
 *
 * The actual "removal" is done by "remove_monster_idx()" which is
 * only called from "process_monsters()".  This prevents any chance
 * of nasty dangling pointer references.
 *
 * But note that until that happens, there will be monsters floating
 * around marked as "dead".  In order to avoid constant checks on
 * "m_ptr->dead", we mark the cave grid as empty below.
 *
 * Note that compaction should probably not be attempted during the
 * "process_monsters()" function, since it will fail.  This is handled
 * by only allowing about 30 monster reproductions per game round, by
 * use of the "tighten_m_list()" function called from "dungeon.c".
 */
void delete_monster_idx(int i)
{
    monster_type *m_ptr = &m_list[i];

    int fy = m_ptr->fy;
    int fx = m_ptr->fx;

    cave_type *c_ptr = &cave[fy][fx];


    /* Monster is dead */
    m_ptr->dead = TRUE;


    /* Hack -- Monster is gone */
    c_ptr->m_idx = 0;

    /* Hack -- Visual update */
    lite_spot(fy, fx);
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

    /* Delete the monster (if any) */
    if (c_ptr->m_idx > MIN_M_IDX) delete_monster_idx(c_ptr->m_idx);
}


/*
 * Attempt to Compact some monsters.
 *
 * This will usually only work correctly from "tighten_m_list()"
 *
 * XXX Base the saving throw on a combination of monster level,
 * distance from player, and current "desperation".
 */
static void compact_monsters(void)
{
    int           i, cur_dis, num = 0;

    msg_print("Compacting monsters...");

    /* Start 66 (that is, 72-6) units away */
    cur_dis = 72;

    /* Keep going until someone is deleted */
    while (!num) {

        /* Nothing to compact (!!!) */
        if (cur_dis < 0) return;

        /* Come closer to the player */
        cur_dis -= 6;

        /* Check all the monsters */
        for (i = MIN_M_IDX; i < m_max; i++) {

            monster_type *m_ptr = &m_list[i];
            monster_race *r_ptr = &r_list[m_ptr->r_idx];

            /* Paranoia -- skip "dead" monsters */
            if (m_ptr->dead) continue;

            /* Ignore nearby monsters */
            if (m_ptr->cdis < cur_dis) continue;

            /* Never compact "Quest" Monsters */
            if (r_ptr->rflags1 & RF1_QUESTOR) continue;

            /* XXX Try not to compact Unique Monsters */
            /* if ((r_ptr->rflags1 & RF1_UNIQUE) && ???) continue; */

            /* All monsters get a saving throw */
            if (rand_int(3) != 0) continue;

            /* Delete the monster */
            delete_monster_idx(i);

            /* Count the monster */
            num++;
        }
    }
}


/*
 * Require some breathing space.  This function should never
 * be called during the main loop of "process_monsters()".
 * Thus it is called at the top and bottom of that function.
 * Note that running out of space is NOT fatal, just annoying.
 */
void tighten_m_list()
{
    int i;

    /* Require some free records */
    if (m_max + 30 > MAX_M_IDX) {

        /* Compact some monsters */
        compact_monsters();

        /* Hack -- Remove dead monsters (backwards!) */
        for (i = m_max - 1; i >= MIN_M_IDX; i--) {

            /* Get the i'th monster */
            monster_type *m_ptr = &m_list[i];

            /* Hack -- Remove dead monsters. */
            if (m_ptr->dead) remove_monster_idx(i);
        }
    }
}



/*
 * Returns a new monster index (or zero)
 */
int m_pop(void)
{
    /* Normal allocation */
    if (m_max < MAX_M_IDX) return (m_max++);

    /* Warning */
    msg_print("Unable to create new monster!");

    /* Hack -- try not to crash */
    return (0);
}







/*
 * Delete/Remove all the monsters when the player leaves the level
 */
void wipe_m_list()
{
    int i;

    /* Delete all the monsters */
    for (i = MIN_M_IDX; i < m_max; i++) {

        monster_type *m_ptr = &m_list[i];

        /* Paranoia -- skip dead monsters */
        if (m_ptr->dead) continue;

        /* Delete the monster */
        delete_monster_idx(i);
    }

    /* Remove all the monsters (backwards!) */
    for (i = m_max - 1; i >= MIN_M_IDX; i--) {

        /* Remove the monster */
        remove_monster_idx(i);
    }

    /* Paranoia */
    m_max = MIN_M_IDX;
}



/*
 * Forward declare
 */
typedef struct _race_entry race_entry;


/*
 * An entry for the monster allocator below
 */
struct _race_entry {
    u16b r_idx;		/* Object kind index */
    byte locale;	/* Base dungeon level */
    byte chance;	/* Rarity of occurance */
};


/*
 * Return a monster race index suitable to the requested dungeon level.
 *
 * There is a small chance (1/50) of using a monster from even deeper
 * than the requested level (up to four levels deeper), in which case
 * the monster will almost definitely be from that level.
 *
 * It is slightly more likely to acquire monsters of the given level
 * that of lower level monsters.  This is done by choosing two monsters
 * and using the "hardest" of the two monsters.
 *
 * The new distribution makes a level n monster occur approx 2/n% of
 * the time on level n, and 1/n*n% are 1st level.
 *
 * Only two functions (this one and the next, which is almost identical)
 * use the "r_level" array, and they both assume that the "r_list" array
 * is sorted by level, which may or may not actually be true.
 *
 * This version (2.7.0) enforces the "rarity" information for monsters.
 * But note that several functions bypass us and use the race list directly,
 * and they also assume that the list is sorted.  XXX XXX XXX
 *
 * XXX XXX XXX This code currently assumes that the monster list is sorted,
 * and even then assumes that "later" monsters are "harder".
 *
 * The old "get_nmons_num()" function was only used in "greater vaults" to
 * acquire extremely out of depth monsters.  We now use this function always.
 *
 * This function still feels a little bit "hacky" to me...
 *
 * Note that monster race zero is now "illegal".
 */

/*
 * Returns the array number of a random object
 * Uses the locale/chance info for distribution.
 */
int get_mon_num(int level)
{
    int		i, try, r_idx;

    monster_race *r_ptr;


    /* Number of entries in the "k_sort" table */
    static u16b size = 0;

    /* The actual table of entries */
    static race_entry *table = NULL;

    /* Number of entries at each locale */
    static u16b t_lev[256];


    /* Initialize the table */
    if (!size) {

        u16b aux[256];

        /* Clear the level counter and the aux array */
        for (i = 0; i < 256; i++) t_lev[i] = aux[i] = 0;

        /* Scan the monsters (not the ghost) */
        for (i = 1; i < MAX_R_IDX-1; i++) {

            /* Get the i'th race */
            r_ptr = &r_list[i];

            /* Process "real" monsters */
            if (r_ptr->rarity) {

                /* Count the total entries */
                size++;

                /* Count the entries at each level */
                t_lev[r_ptr->level]++;
            }
        }

        /* Combine the "t_lev" entries */
        for (i = 1; i < 256; i++) t_lev[i] += t_lev[i-1];

        /* Allocate the table */
        C_MAKE(table, size, race_entry);

        /* Scan the monsters (not the ghost) */
        for (i = 1; i < MAX_R_IDX-1; i++) {

            /* Get the i'th race */
            r_ptr = &r_list[i];

            /* Count valid pairs */
            if (r_ptr->rarity) {

                int r, x, y, z;

                /* Extract the level/rarity */
                x = r_ptr->level;
                r = r_ptr->rarity;

                /* Skip entries preceding our locale */
                y = (x > 0) ? t_lev[x-1] : 0;

                /* Skip previous entries at this locale */
                z = y + aux[x];

                /* Load the table entry */
                table[z].r_idx = i;
                table[z].locale = x;
                table[z].chance = r;

                /* Another entry complete for this locale */
                aux[x]++;
            }
        }
    }


    /* Pick a monster */
    while (1) {

        /* Town level is easy */
        if (level == 0) {

            /* Pick a level 0 entry */
            i = rand_int(t_lev[0]);
        }

        /* Other levels */
        else {

            /* Obtain "desired" level */
            try = level;

            /* Occasionally Make a Nasty Monster */
            if (rand_int(NASTY_MON) == 0) {

                /* Pick a level bonus */
                int d = try / 4 + 2;

                /* Require a harder level */
                try += ((d < 5) ? d : 5);
            }

            /* Only examine legal levels */
            if (try > MAX_R_LEV) try = MAX_R_LEV;

            /* Pick any entry at or below the given level */
            i = rand_int(t_lev[try]);

            /* Always try for a "harder" monster */
            if (TRUE) {

                /* Pick another object at or below the given level */
                int j = rand_int(t_lev[try]);

                /* Keep it if it is "better" */
                if (table[i].locale < table[j].locale) i = j;
            }

            /* Hack -- Never make town monsters */
            if (table[i].locale == 0) continue;
        }

        /* Play the "chance game" */
        if (rand_int(table[i].chance) != 0) continue;


        /* Access the "r_idx" of the chosen monster */
        r_idx = table[i].r_idx;

        /* Access the actual race */
        r_ptr = &r_list[r_idx];

        /* Uniques never appear out of "requested" depth */
        if ((r_ptr->level > level) && (r_ptr->rflags1 & RF1_UNIQUE)) {
            continue;
        }

        /* Depth Monsters never appear out of depth */
        if ((r_ptr->level > dun_level) && (r_ptr->rflags1 & RF1_FORCE_DEPTH)) {
            continue;
        }

        /* Sounds good */
        return (r_idx);
    }

    /* Paranoia */
    return (0);
}



/*
 * Places a monster at given location
 *
 * Refuses to place out-of-depth Quest Monsters.
 *
 * To give the player a sporting chance, any monster that appears in
 * line-of-sight and can cast spells or breathe, should be asleep.
 * This is an extension of Um55's sleeping dragon code...
 *
 * Note that once a monster has been killed 30000 times during a single
 * life, that monster will no longer be created during that life.
 *
 * Note that monster race "zero" is no longer a valid race index!
 *
 * Unique kobolds, Liches, orcs, Ogres, Trolls, yeeks, and demons
 * (but not skeletons/drujs) get a "following" of escorts.  -DGK-
 *
 * XXX XXX XXX Hack --
 * Note that we assume that the higher a monster race index is,
 * the higher the monster level.  We use this to choose the hardest
 * possible escorts for the unique by traversing the array backwards.
 */
int place_monster(int y, int x, int r_idx, int slp)
{
    int			i, z, ny, nx, count;

    cave_type		*c_ptr;

    monster_type	*m_ptr;

    monster_race	*r_ptr = &r_list[r_idx];
    monster_lore	*l_ptr = &l_list[r_idx];


    /* Verify location */
    if (!in_bounds(y,x)) return (FALSE);

    /* Require empty space */
    if (!empty_grid_bold(y, x)) return (FALSE);


    /* Paranoia -- prevent illegal monster race index */
    if (r_idx == 0) return (FALSE);


    /* Depth monsters may NOT be created out of depth */
    if ((r_ptr->rflags1 & RF1_FORCE_DEPTH) && (dun_level < r_ptr->level)) {

        /* Cannot create */
        return (FALSE);
    }


    /* Monster limit per level */
    if (l_ptr->cur_num >= l_ptr->max_num) {

        /* Cannot create */
        return (FALSE);
    }

    /* Monster limit per life */
    if (l_ptr->pkills >= 30000) {

        /* Cannot create */
        return (FALSE);
    }


    /* Note the monster */
    if (cheat_hear && (r_ptr->rflags1 & RF1_UNIQUE)) {
        msg_print(format("Unique (%s)", r_ptr->name));
    }

    /* Powerful monster */
    if (r_ptr->level > dun_level) {

        /* Uniques get rating based on "out of depth" amount */
        if (r_ptr->rflags1 & RF1_UNIQUE) {
            rating += (r_ptr->level - dun_level);
        }

        /* Normal monsters are worth "half" as much */
        else {
            rating += (r_ptr->level - dun_level) / 2;
        }
    }


    /* Access the location */
    c_ptr = &cave[y][x];

    /* Make a new monster */
    c_ptr->m_idx = m_pop();

    /* Hack -- catch failure */
    if (c_ptr->m_idx == 0) return (FALSE);


    /* Get a new monster record */
    m_ptr = &m_list[c_ptr->m_idx];

    /* Save the race */
    m_ptr->r_idx = r_idx;

    /* Place the monster at the location */
    m_ptr->fy = y;
    m_ptr->fx = x;


    /* Count the monsters on the level */
    l_ptr->cur_num++;


    /* Assign maximal hitpoints */
    if (r_ptr->rflags1 & RF1_FORCE_MAXHP) {
        m_ptr->maxhp = maxroll(r_ptr->hdice, r_ptr->hside);
    }
    else {
        m_ptr->maxhp = damroll(r_ptr->hdice, r_ptr->hside);
    }

    /* And start out fully healthy */
    m_ptr->hp = m_ptr->maxhp;

    /* Extract the monster base speed */
    m_ptr->mspeed = r_ptr->speed;

    /* Allow some small variation per monster */
    i = extract_energy[r_ptr->speed] / 10;
    if (i) m_ptr->mspeed += rand_spread(0, i);

    /* Give a random starting energy */
    m_ptr->energy = rand_int(100);

    /* No "damage" yet */
    m_ptr->stunned = 0;
    m_ptr->confused = 0;
    m_ptr->monfear = 0;

    /* Not dead */
    m_ptr->dead = FALSE;
    
    /* Update the monster (correctly) */
    m_ptr->ml = FALSE;
    update_mon(c_ptr->m_idx, TRUE);


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

    /* Hack -- Prevent instant death by breath weapons */
    else if ((r_ptr->rflags1 & RF1_FORCE_SLEEP) &&
             projectable(y, x, py, px)) {
        m_ptr->csleep = 1;
    }

    /* Wake up... */
    else {
        m_ptr->csleep = 0;
    }



    /* Escorts for certain unique monsters */
    if (r_ptr->rflags1 & RF1_UNIQUE) {

        /* Unique monsters with escorts */
        if (strchr("kLoOTyI&", r_ptr->r_char)) {

            /* XXX XXX XXX XXX Hack -- Find some escorts */
            for (z = MAX_R_IDX-1; z > 0; z--) {

                monster_race *z_ptr = &r_list[z];

                /* Skip Unique Monsters */
                if (z_ptr->rflags1 & RF1_UNIQUE) continue;
                
                /* Find a similar, lower level, non-unique, monster */
                if ((z_ptr->r_char == r_ptr->r_char) &&
                    (z_ptr->level <= r_ptr->level)) {

                    /* Try up to 50 nearby places */
                    for (count = 0; count < 50; count++) {

                        int d = 3;

                        /* Pick a place */
                        while (1) {
                            ny = rand_spread(y, d);
                            nx = rand_spread(x, d);
                            if (!in_bounds(ny,nx)) continue;
                            if (distance(y, x, ny, nx) > d) continue;
                            if (los(y, x, ny, nx)) break;
                        }

                        /* Use first empty space */
                        if (empty_grid_bold(ny, nx)) break;
                    }

                    /* Hack -- Nowhere to go */
                    if (count == 50) break;

                    /* Certain monsters come in groups */
                    if ((z_ptr->r_char == 'k') ||
                        (z_ptr->r_char == 'y') ||
                        (z_ptr->r_char == '&') ||
                        (z_ptr->rflags1 & RF1_FRIENDS)) {
                        place_group(ny, nx, z, slp);
                    }

                    /* Otherwise, just use a single escort */
                    else {
                        place_monster(ny, nx, z, slp);
                    }
                }
            }
        }
    }

    /* Success */
    return TRUE;
}




#if 0

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

static byte ghost_class_colors[] = {
    TERM_L_BLUE, TERM_RED, TERM_L_GREEN,
    TERM_BLUE, TERM_GREEN, TERM_WHITE
};



/*
 * Prepare the "ghost" monster_race info
 *
 * XXX XXX This code has only recently been changed (and debugged),
 * so it may have brand new bugs now.
 *
 * Even if not, it really needs to be re-writtem, there are redundancies
 * and incorrectnesses everywhere.  And the savefile ruins everything.
 *
 * Actually, the new savefile is "much better".  It may fix the problems.
 */
static void set_ghost(cptr pn, int hp, int gr, int gc, int lev)
{
    int  i;

    monster_race *g = &r_list[MAX_R_IDX-1];

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

    /* Very attentive */
    g->aaf = 100;


    /* Hack -- Break up the hitpoints */
    for (i = 1; i * i < hp; i++);

    /* Extract the basic hit dice and sides */
    g->hdice = g->hside = i;

XXX XXX XXX XXX XXX Broken

    /* Initialize some of the flags */
    g->cflags1 |= (MF1_MV_ATT_NORM | MF1_CARRY_OBJ);
    g->cflags2 |= (MF2_GOOD);
    g->cflags2 |= (MF2_UNIQUE | MF2_CHARM_SLEEP | MF2_EVIL);


    /* The "ghost" is an "adventurer" */
    if (g->level == p_ptr->lev) {

        /* A wanderer from the town */
        sprintf(ghost_name, "%s, the %s %s",
                name, gr_name, gc_name);

        /* Use a "player" symbol */
        g->r_char = 'p';

        /* Use a "player" color */
        g->r_attr = ghost_class_colors[gc];

        /* Basic flags */
        g->cflags1 |= (MF1_THRO_DR | MF1_HAS_90 | MF1_HAS_60);

        /* Treasure drops */
        if (lev > 10) g->cflags1 |= (MF1_HAS_1D2);
        if (lev > 18) g->cflags1 |= (MF1_HAS_2D2);
        if (lev > 23) g->cflags1 |= (MF1_HAS_4D2);
        if (lev > 40) g->cflags1 |= (MF2_SPECIAL);
        if (lev > 40) g->cflags1 &= (~MF1_HAS_4D2);

        /* Add some random resists -DGK */
        for (i = 0; i <= (lev / 5); i++) {
            switch (randint(13)) {
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

        /* Analyze the class */
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

        /* Racial properties */
        if (gr == 6) g->cflags2 |= MF2_ORC;
        if (gr == 7) g->cflags2 |= MF2_TROLL;

        /* Armor class */
        g->ac = 15 + randint(15);
        if (gc == 0 || gc >= 3) g->ac += randint(60);

        /* Default speed (normal) */
        g->speed = 110;

        /* High level mages and rogues are fast... */
        if ((gc == 1 || gc == 3) && lev > 25) g->speed += 10;

        /* Obtain some attacks */
        g->damage[0] = 5 + ((lev > 18) ? 18 : lev);
        g->damage[1] = g->damage[0];

        /* Obtain more attacks */
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

        /* All done */
        return;
    }


    /* Initialize some more of the flags */
    g->cflags2 |= (MF2_UNDEAD | MF2_NO_INFRA | MF2_IM_POIS);


    /* Make a ghost with power based on the ghost level */
    switch ((g->level / 4) + randint(3)) {

      case 1:
      case 2:
      case 3:
        sprintf(ghost_name, "%s, the Skeleton %s", name, gr_name);
        g->r_char = 's';
        g->r_attr = TERM_WHITE;
        g->cflags1 |= (MF1_THRO_DR | MF1_HAS_90);
        g->cflags2 |= (MF2_IM_COLD);
        if (gr == 6) g->cflags2 |= MF2_ORC;
        if (gr == 7) g->cflags2 |= MF2_TROLL;
        g->ac = 26;
        g->speed = 110;
        g->damage[0] = 5;
        g->damage[1] = 5;
        g->damage[2] = 0;
        g->damage[3] = 0;
        break;

      case 4:
      case 5:
        sprintf(ghost_name, "%s, the Zombified %s", name, gr_name);
        g->r_char = 'z';
        g->r_attr = TERM_GRAY;
        g->cflags1 |= (MF1_THRO_DR | MF1_HAS_60 | MF1_HAS_90);
        if (gr == 6) g->cflags2 |= MF2_ORC;
        if (gr == 7) g->cflags2 |= MF2_TROLL;
        g->ac = 30;
        g->speed = 110;
        g->hside *= 2;
        g->damage[0] = 8;
        g->damage[1] = 0;
        g->damage[2] = 0;
        g->damage[3] = 0;
        break;

      case 6:
        sprintf(ghost_name, "%s, the Poltergeist", name);
        g->r_char = 'G';
        g->r_attr = TERM_WHITE;
        g->cflags1 |= (MF1_MV_INVIS | MF1_HAS_1D2 | MF1_MV_75 | MF1_THRO_WALL);
        g->cflags2 |= (MF2_IM_COLD);
        g->ac = 20;
        g->speed = 130;
        g->damage[0] = 5;
        g->damage[1] = 5;
        g->damage[2] = 93;
        g->damage[3] = 93;
        g->mexp = (g->mexp * 3) / 2;
        break;

      case 7:
      case 8:
        sprintf(ghost_name, "%s, the Mummified %s", name, gr_name);
        g->r_char = 'M';
        g->r_attr = TERM_GRAY;
        g->cflags1 |= (MF1_HAS_1D2);
        if (gr == 6) g->cflags2 |= MF2_ORC;
        if (gr == 7) g->cflags2 |= MF2_TROLL;
        g->ac = 35;
        g->speed = 110;
        g->hside *= 2;
        g->damage[0] = 16;
        g->damage[1] = 16;
        g->damage[2] = 16;
        g->damage[3] = 0;
        g->mexp = (g->mexp * 3) / 2;
        break;

      case 9:
      case 10:
        sprintf(ghost_name, "%s's Spirit", name);
        g->r_char = 'G';
        g->r_attr = TERM_WHITE;
        g->cflags1 |= (MF1_MV_INVIS | MF1_THRO_WALL | MF1_HAS_1D2);
        g->cflags2 |= (MF2_IM_COLD);
        g->ac = 20;
        g->speed = 110;
        g->hside *= 2;
        g->damage[0] = 19;
        g->damage[1] = 185;
        g->damage[2] = 99;
        g->damage[3] = 178;
        g->mexp = g->mexp * 3;
        break;

      case 11:
        sprintf(ghost_name, "%s's Ghost", name);
        g->r_char = 'G';
        g->r_attr = TERM_WHITE;
        g->cflags1 |= (MF1_MV_INVIS | MF1_THRO_WALL | MF1_HAS_1D2);
        g->cflags2 |= (MF2_IM_COLD);
        g->spells1 |= (0xFL | MS1_HOLD | MS1_MANA_DRAIN | MS1_BLIND);
        g->ac = 40;
        g->speed = 120;
        g->hside *= 2;
        g->damage[0] = 99;
        g->damage[1] = 99;
        g->damage[2] = 192;
        g->damage[3] = 184;
        g->mexp = (g->mexp * 7) / 2;
        break;

      case 12:
        sprintf(ghost_name, "%s, the Vampire", name);
        g->r_char = 'V';
        g->r_attr = TERM_VIOLET;
        g->cflags1 |= (MF1_THRO_DR | MF1_HAS_2D2);
        g->cflags2 |= (MF2_HURT_LITE);
        g->spells1 |= (0x8L | MS1_HOLD | MS1_FEAR | MS1_TELE_TO | MS1_CAUSE_2);
        g->ac = 40;
        g->speed = 110;
        g->hside *= 3;
        g->damage[0] = 20;
        g->damage[1] = 20;
        g->damage[2] = 190;
        g->damage[3] = 0;
        g->mexp = g->mexp * 3;
        break;

      case 13:
        sprintf(ghost_name, "%s's Wraith", name);
        g->r_char = 'W';
        g->r_attr = TERM_WHITE;
        g->cflags1 |= (MF1_THRO_DR | MF1_HAS_4D2 | MF1_HAS_2D2);
        g->cflags2 |= (MF2_IM_COLD | MF2_HURT_LITE);
        g->spells1 |= (0x7L | MS1_HOLD | MS1_FEAR | MS1_BLIND | MS1_CAUSE_3);
        g->spells2 |= (MS2_BO_NETH);
        g->ac = 60;
        g->speed = 120;
        g->hside *= 3;
        g->damage[0] = 20;
        g->damage[1] = 20;
        g->damage[2] = 190;
        g->damage[3] = 0;
        g->mexp = g->mexp * 5;
        break;

      case 14:
        sprintf(ghost_name, "%s, the Vampire Lord", name);
        g->r_char = 'V';
        g->r_attr = TERM_BLUE;
        g->cflags1 |= (MF1_THRO_DR | MF1_HAS_1D2);
        g->cflags2 |= (MF2_HURT_LITE | MF2_SPECIAL);
        g->spells1 |= (0x8L | MS1_HOLD | MS1_FEAR | MS1_TELE_TO | MS1_CAUSE_3);
        g->spells2 |= (MS2_BO_NETH);
        g->ac = 80;
        g->speed = 110;
        g->hside *= 2;
        g->hdice *= 2;
        g->damage[0] = 20;
        g->damage[1] = 20;
        g->damage[2] = 20;
        g->damage[3] = 198;
        g->mexp = g->mexp * 20;
        break;

      case 15:
        sprintf(ghost_name, "%s's Ghost", name);
        g->r_char = 'G';
        g->r_attr = TERM_WHITE;
        g->cflags1 |= (MF1_MV_INVIS | MF1_THRO_WALL | MF1_HAS_2D2);
        g->cflags2 |= (MF2_SPECIAL | MF2_IM_COLD);
        g->spells1 |= (0x5L | MS1_HOLD | MS1_MANA_DRAIN | MS1_BLIND | MS1_CONF);
        g->ac = 90;
        g->speed = 130;
        g->hside *= 3;
        g->damage[0] = 99;
        g->damage[1] = 99;
        g->damage[2] = 192;
        g->damage[3] = 184;
        g->mexp = g->mexp * 20;
        break;

      case 17:
        sprintf(ghost_name, "%s, the Lich", name);
        g->r_char = 'L';
        g->r_attr = TERM_ORANGE;
        g->cflags1 |= (MF1_THRO_DR | MF1_HAS_2D2 | MF1_HAS_1D2);
        g->cflags2 |= (MF2_SPECIAL | MF2_IM_COLD | MF2_INTELLIGENT);
        g->spells1 |= (0x3L | MS1_FEAR | MS1_CAUSE_3 | MS1_TELE_TO | MS1_BLINK |
                       MS1_S_UNDEAD | MS1_BA_FIRE | MS1_BA_COLD | MS1_HOLD |
                       MS1_MANA_DRAIN | MS1_BLIND | MS1_CONF | MS1_TELEPORT);
        g->spells2 |= (MS2_BRAIN_SMASH | MS2_RAZOR);
        g->ac = 120;
        g->speed = 120;
        g->hside *= 3;
        g->hdice *= 2;
        g->damage[0] = 181;
        g->damage[1] = 201;
        g->damage[2] = 214;
        g->damage[3] = 181;
        g->mexp = g->mexp * 50;
        break;

      default:
        sprintf(ghost_name, "%s's Ghost", name);
        g->r_char = 'G';
        g->r_attr = TERM_WHITE;
        g->cflags1 |= (MF1_MV_INVIS | MF1_THRO_WALL |
                       MF1_HAS_1D2 | MF1_HAS_2D2);
        g->cflags2 |= (MF2_SPECIAL | MF2_IM_COLD | MF2_INTELLIGENT);
        g->spells1 |= (0x2L | MS1_HOLD | MS1_MANA_DRAIN |
                       MS1_BLIND | MS1_CONF | MS1_TELE_TO);
        g->spells2 |= (MS2_BO_NETH | MS2_BA_NETH | MS2_BRAIN_SMASH |
                       MS2_TELE_LEVEL);
        g->ac = 130;
        g->speed = 130;
        g->hside *= 2;
        g->hdice *= 2;
        g->damage[0] = 99;
        g->damage[1] = 99;
        g->damage[2] = 192;
        g->damage[3] = 184;
        g->mexp = g->mexp * 30;
        break;
    }
}

#endif



/*
 * Places a ghost somewhere.
 * Probably not the best possible algorithm.
 */
int place_ghost()
{

#if 0

    int			y, x, hp, level, gr, gc;

    cave_type		*c_ptr;
    monster_type	*m_ptr;

    monster_race	*r_ptr = &r_list[MAX_R_IDX-1];
    monster_lore	*l_ptr = &l_list[MAX_R_IDX-1];

    FILE		*fp;

    bool		err = FALSE;

    char		name[100];
    char		tmp[1024];

#endif

    /* XXX XXX XXX XXX Assume failure */
    return (FALSE);

#if 0

XXX XXX XXX XXX XXX Broken

    /* Hack -- no ghosts in the town */
    if (!dun_level) return (FALSE);

    /* Already have a ghost */
    if (l_ptr->cur_num >= l_ptr->max_num) return (FALSE);


    /* Town -- Use Player Level */
    if (!dun_level) {

        /* Assume minimum level */
        if (p_ptr->lev < 5) return (FALSE);

        /* And even then, it only happens sometimes */
        /* if (14 > randint((dun_level / 2) + 11)) return (FALSE); */

        /* Only a 10% chance */
        if (rand_int(10) != 0) return (FALSE);

        /* Level is player level */
        level = p_ptr->lev;

        /* Hack -- no chance */
        return (FALSE);
    }

    /* Dungeon -- Use Dungeon Level */
    else {

        /* And even then, it only happens sometimes */
        if (14 > randint((dun_level / 2) + 11)) return (FALSE);

        /* Only a 33% chance */
        if (rand_int(3) != 0) return (FALSE);

        /* Level is dungeon level */
        level = dun_level;
    }


    /* Choose a bones file */
    sprintf(tmp, "%s%s%d", ANGBAND_DIR_BONE, PATH_SEP, level);

    /* Open the bones file */
    fp = my_tfopen(tmp, "r");

    /* No bones file to use */
    if (!fp) return (FALSE);

    /* Scan the file */
    err = (fscanf(fp, "%[^\n]\n%d\n%d\n%d", name, &hp, &gr, &gc) != 4);

    /* Close the file */
    fclose(fp);

    /* Delete the bones file */
    unlink(tmp);

    /* Catch errors */
    if (err) {
        msg_print("Warning -- deleted corrupt 'ghost' file!");
        return (FALSE);
    }


    /* Set up the ghost */
    set_ghost(name, hp, gr, gc, level);

    /* Note for wizard (special ghost name) */
    if (cheat_hear) msg_print("Unique (Ghost)");

    /* Hack -- pick a nice (far away) location */
    while (1) {

        /* Pick a location */
        y = randint(cur_hgt - 2);
        x = randint(cur_wid - 2);

        /* Require "naked" floor grid */
        if (!naked_grid_bold(y,x)) continue;

        /* Accept far away grids */
        if (distance(py, px, y, x) > MAX_SIGHT + 5) break;
    }


    /*** Place the Ghost by Hand (so no-one else does it accidentally) ***/

    /* Access the location */
    c_ptr = &cave[y][x];

    /* Make a new monster */
    c_ptr->m_idx = m_pop();

    /* XXX XXX XXX Failed? */
    if (c_ptr->m_idx == 0) return (FALSE);


    /* Access the monster */
    m_ptr = &m_list[c_ptr->m_idx];

    m_ptr->fy = y;
    m_ptr->fx = x;

    /* Hack -- the monster is a ghost */
    m_ptr->r_idx = MAX_R_IDX-1;


    /* Count the ghosts (only one) */
    l_ptr->cur_num++;


    /* Assign the max hitpoints and hitpoints */
    m_ptr->maxhp = m_ptr->hp = (s16b)(r_ptr->hdice) * (s16b)(r_ptr->hside);

    /* Extract the base speed */
    m_ptr->mspeed = r_ptr->speed;

    /* Pick a random energy */
    m_ptr->energy = rand_int(100);

    m_ptr->stunned = 0;
    m_ptr->csleep = 0;


    /* Mega-Hack -- update the graphic info */
    l_ptr->l_attr = r_ptr->r_attr;
    l_ptr->l_char = r_ptr->r_char;


    /* Update the monster (correctly) */
    m_ptr->ml = FALSE;
    update_mon(c_ptr->m_idx, TRUE);

    /* Success */
    return (TRUE);

#endif

}


/*
 * Let the given monster attempt to reproduce.
 * Note that "reproduction" REQUIRES empty space.
 */
int multiply_monster(int m_idx)
{
    int        i, y, x;

    monster_type *m_ptr = &m_list[m_idx];

    int x1 = m_ptr->fx;
    int y1 = m_ptr->fy;

    int result = FALSE;


    /* Hack -- prevent insane reproduction */
    if (m_ptr->cdis > 20) return (FALSE);


    /* Try up to 18 times */
    for (i = 0; i < 18; i++) {

        int d = 1;

        /* Pick a location */
        while (1) {
            y = rand_spread(y1, d);
            x = rand_spread(x1, d);
            if (!in_bounds2(y, x)) continue;
            if (distance(y1, x1, y, x) > d) continue;
            if (los(y1, x1, y, x)) break;
        }

        /* Require an "empty" floor grid */
        if (!empty_grid_bold(y, x)) continue;

        /* Create a new monster */
        result = place_monster(y, x, m_ptr->r_idx, FALSE);

        /* Failed to create! */
        if (!result) return FALSE;

        /* Get the new monster */
        m_ptr = &m_list[cave[y][x].m_idx];

        /* Return the visibility of the created monster */
        return (m_ptr->ml);
    }


    /* Nobody got made */
    return (FALSE);
}






/*
 * Place a group of monsters near the given location
 * Hack -- note the use of "old" to prevent a group of out of
 * depth monsters from driving the rating through the roof
 */
void place_group(int y, int x, int r_idx, int slp)
{
    monster_race *r_ptr = &r_list[r_idx];

    int extra = 0;

    int old = rating;


    /* reduce size of group if out-of-depth */
    if (r_ptr->level > (unsigned) dun_level) {
        extra = 0 - randint(r_ptr->level - dun_level);
    }

    /* if monster is deeper than normal, then travel in bigger packs -CFT */
    else if (r_ptr->level < (unsigned) dun_level) {
        extra = randint(dun_level - r_ptr->level);
    }

    /* put an upper bounds on it... -CFT */
    if (extra > 12) extra = 12;

    switch (randint(13) + extra) {
      case 25:
        place_monster(y, x - 3, r_idx, FALSE);
      case 24:
        place_monster(y, x + 3, r_idx, FALSE);
      case 23:
        place_monster(y - 3, x, r_idx, FALSE);
      case 22:
        place_monster(y + 3, x, r_idx, FALSE);
      case 21:
        place_monster(y - 2, x + 1, r_idx, FALSE);
      case 20:
        place_monster(y + 2, x - 1, r_idx, FALSE);
      case 19:
        place_monster(y + 2, x + 1, r_idx, FALSE);
      case 18:
        place_monster(y - 2, x - 1, r_idx, FALSE);
      case 17:
        place_monster(y + 1, x + 2, r_idx, FALSE);
      case 16:
        place_monster(y - 1, x - 2, r_idx, FALSE);
      case 15:
        place_monster(y + 1, x - 2, r_idx, FALSE);
      case 14:
        place_monster(y - 1, x + 2, r_idx, FALSE);
      case 13:
        place_monster(y, x - 2, r_idx, FALSE);
      case 12:
        place_monster(y, x + 2, r_idx, FALSE);
      case 11:
        place_monster(y + 2, x, r_idx, FALSE);
      case 10:
        place_monster(y - 2, x, r_idx, FALSE);
      case 9:
        place_monster(y + 1, x + 1, r_idx, FALSE);
      case 8:
        place_monster(y + 1, x - 1, r_idx, FALSE);
      case 7:
        place_monster(y - 1, x - 1, r_idx, FALSE);
      case 6:
        place_monster(y - 1, x + 1, r_idx, FALSE);
      case 5:
        place_monster(y, x + 1, r_idx, FALSE);
      case 4:
        place_monster(y, x - 1, r_idx, FALSE);
      case 3:
        place_monster(y + 1, x, r_idx, FALSE);
      case 2:
        place_monster(y - 1, x, r_idx, FALSE);
        rating = old;
      default:
        place_monster(y, x, r_idx, FALSE);
    }
}


/*
 * Allocates some random monsters   -RAK-	
 * Place the monsters at least "dis" distance from the player.
 * Use "slp" to choose the initial "sleep" status
 */
void alloc_monster(int num, int dis, int slp)
{
    int			y, x, i, r_idx;

    monster_race	*r_ptr;


    /* Place the monsters */
    for (i = 0; i < num; i++) {

        /* Find a legal, distant, unoccupied, space */
        while (1) {

            /* Pick a location */
            y = randint(cur_hgt - 2);
            x = randint(cur_wid - 2);

            /* Require "naked" floor grid */
            if (!naked_grid_bold(y,x)) continue;

            /* Accept far away grids */
            if (distance(y, x, py, px) > dis) break;
        }

        /* Get a monster of the current level */
        r_idx = get_mon_num(dun_level);

        /* Get that race */
        r_ptr = &r_list[r_idx];

        /* Place a group of monsters */	
        if (r_ptr->rflags1 & RF1_FRIENDS) {
            place_group(y, x, r_idx, slp);
        }

        /* Place a single monster */	
        else {
            place_monster(y, x, r_idx, slp);
        }
    }
}


/*
 * Summon a monster of the given level near the given location.
 * We return TRUE if anything was summoned.
 */
int summon_monster(int y1, int x1, int lev)
{
    int			i, y, x, r_idx;

    monster_race	*r_ptr;


    /* Try nine locations */
    for (i = 0; i < 9; i++) {

        int d = 1;

        /* Pick a nearby location */
        while (1) {
            y = rand_spread(y1, d);
            x = rand_spread(x1, d);
            if (!in_bounds(y, x)) continue;
            if (distance(y1, x1, y, x) > d) continue;
            if (los(y1, x1, y, x)) break;
        }

        /* Require "empty" floor grids */
        if (!empty_grid_bold(y, x)) continue;

        /* Pick a monster race */
        r_idx = get_mon_num(lev);

        /* Get the race */
        r_ptr = &r_list[r_idx];

        /* Place the monster */
        if (r_ptr->rflags1 & RF1_FRIENDS) {
            place_group(y, x, r_idx, FALSE);
        }
        else {
            place_monster(y, x, r_idx, FALSE);
        }

        /* Success */
        return (TRUE);
    }

    /* Nothing summoned */
    return (FALSE);
}



/*
 * Check if monster race "m" is "okay" for summon type "type"
 */
static bool summon_specific_okay(int m, int type)
{
    monster_race *r_ptr = &r_list[m];

    bool okay = FALSE;


    /* Check our requirements */
    switch (type) {

            case SUMMON_ANT:
                okay = ((r_ptr->r_char == 'a') &&
                        !(r_ptr->rflags1 & RF1_UNIQUE));
                break;

            case SUMMON_SPIDER:
                okay = ((r_ptr->r_char == 'S') &&
                        !(r_ptr->rflags1 & RF1_UNIQUE));
                break;

            case SUMMON_HOUND:
                okay = (((r_ptr->r_char == 'C') || (r_ptr->r_char == 'Z')) &&
                        !(r_ptr->rflags1 & RF1_UNIQUE));
                break;

            case SUMMON_REPTILE:
                okay = ((r_ptr->r_char == 'R') &&
                        !(r_ptr->rflags1 & RF1_UNIQUE));
                break;

            case SUMMON_ANGEL:
                okay = ((r_ptr->r_char == 'A') &&
                        !(r_ptr->rflags1 & RF1_UNIQUE));
                break;

            case SUMMON_DEMON:
                okay = ((r_ptr->rflags3 & RF3_DEMON) &&
                    !(r_ptr->rflags1 & RF1_UNIQUE));
                break;

            case SUMMON_UNDEAD:
                okay = ((r_ptr->rflags3 & RF3_UNDEAD) &&
                        !(r_ptr->rflags1 & RF1_UNIQUE));
                break;

            case SUMMON_DRAGON:
                okay = ((r_ptr->rflags3 & RF3_DRAGON) &&
                        !(r_ptr->rflags1 & RF1_UNIQUE));
                break;

            case SUMMON_HI_UNDEAD:
                okay = ((r_ptr->r_char == 'L') ||
                        (r_ptr->r_char == 'V') ||
                        (r_ptr->r_char == 'W'));
                break;

            case SUMMON_HI_DRAGON:
                okay = (r_ptr->r_char == 'D');
                break;

            case SUMMON_WRAITH:
                okay = ((r_ptr->r_char == 'W') &&
                        (r_ptr->rflags1 & RF1_UNIQUE));
                break;

            case SUMMON_UNIQUE:
                okay = (r_ptr->rflags1 & RF1_UNIQUE);
                break;
    }

    /* Return the result */
    return (okay);
}


/*
 * Place a monster (of the specified "type") adjacent to the given
 * location.  Return TRUE iff a monster was actually summoned.
 *
 * We will attempt to place the monster up to 10 times before giving up.
 *
 * Note: SUMMON_UNIQUE and SUMMON_WRAITH (XXX) require "Unique-ness"
 * Note: SUMMON_GUNDEAD and SUMMON_ANCIENTD may summon "Unique's"
 * Note: Other summon will never summon Unique monsters.
 *
 * This function has been changed.  We now take the "monster level"
 * of the summoning monster as a parameter, and use that, along with
 * the current dungeon level, to help determine the level
 * of the desired monster.  Note that this is an upper bound, and
 * also tends to "prefer" monsters of that level.
 *
 * Currently, we use the average of the dungeon and monster levels,
 * and then add five to allow slight increases in monster power.
 */
int summon_specific(int y1, int x1, int lev, int type)
{
    int		r_idx, i, d, x, y;

    monster_race	*r_ptr;


    /* Find an acceptable monster race */
    for (r_idx = i = 0; !r_idx && (i < 30000); i++) {

        /* Choose a monster */
        monster_level = (dun_level + lev) / 2 + 5;
        r_idx = get_mon_num(monster_level);
        monster_level = dun_level;

        /* Refuse "undesired" races */
        if (!summon_specific_okay(r_idx, type)) r_idx = 0;
    }

    /* No race found (!) */
    if (!r_idx) return (FALSE);

    /* Get that race */
    r_ptr = &r_list[r_idx];


    /* Try to place it 10 times */
    for (i = 0; i < 10; ++i) {

        /* Pick a distance */
        d = 1;

        /* Pick a location */
        while (1) {
            y = rand_spread(y1, d);
            x = rand_spread(x1, d);
            if (!in_bounds(y, x)) continue;
            if (distance(y1, x1, y, x) > d) continue;
            if (los(y1, x1, y, x)) break;
        }

        /* Require "empty" floor grid */
        if (!empty_grid_bold(y, x)) continue;

        /* Place a group of monsters */	
        if (r_ptr->rflags1 & RF1_FRIENDS) {
            place_group(y, x, r_idx, FALSE);
        }

        /* Place a single monster */	
        else {
            place_monster(y, x, r_idx, FALSE);
        }

        /* Success */
        return (TRUE);
    }


    /* Failure */
    return (FALSE);
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
void monster_desc(char *desc, monster_type *m_ptr, int mode)
{
    cptr res;
    monster_race *r_ptr = &r_list[m_ptr->r_idx];

    /* Can we "see" it (exists + forced, or visible + not unforced) */
    int seen = m_ptr && ((mode & 0x80) || (!(mode & 0x40) && m_ptr->ml));

    /* Sexed Pronouns (seen and allowed, or unseen and allowed) */
    int pron = m_ptr && ((seen && (mode & 0x20)) || (!seen && (mode & 0x10)));


    /* First, try using pronouns, or describing hidden monsters */
    if (!seen || pron) {

        /* an encoding of the monster "sex" */
        int kind = 0x00;

        /* Extract the gender (if applicable) */
        if (r_ptr->rflags1 & RF1_FEMALE) kind = 0x20;
        else if (r_ptr->rflags1 & RF1_MALE) kind = 0x10;

        /* Ignore the gender (if desired) */
        if (!m_ptr || !pron) kind = 0x00;


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
        }

        /* Copy the result */
        (void)strcpy(desc, res);
    }


    /* Handle visible monsters, "reflexive" request */
    else if ((mode & 0x02) && (mode & 0x01)) {

        /* The monster is visible, so use its gender */
        if (r_ptr->rflags1 & RF1_FEMALE) strcpy(desc, "herself");
        else if (r_ptr->rflags1 & RF1_MALE) strcpy(desc, "himself");
        else strcpy(desc, "itself");
    }


    /* Handle all other visible monster requests */
    else {

        /* It could be a Unique */
        if (r_ptr->rflags1 & RF1_UNIQUE) {

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



