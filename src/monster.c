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
 * Actually "remove" a monster record by "excision".
 *
 * Always call "delete_monster()" first (!)
 *
 * Currently, only called by "process_monsters()" and
 * "tighten_m_list()" and "wipe_m_list()".
 *
 * Note the careful use of the "m_idx" field in cave grids.  This is
 * necessary to prevent problems during "polymorph" attacks and when
 * one monster "eats" another and even if one monster dies and another
 * "blinks" into the grid it used to occupy.  See below.
 */
void remove_monster_idx(int m_idx)
{
    /* One less monster */
    m_max--;

    /* Do structure dumping */
    if (m_idx != m_max) {

        int ny = m_list[m_max].fy;
        int nx = m_list[m_max].fx;

        /* Hack -- prepare to slide the monster */
        if (cave[ny][nx].m_idx == m_max) cave[ny][nx].m_idx = m_idx;

        /* Hack -- Sliding target monster */
        if (target_who == (int)(m_max)) target_who = m_idx;

        /* Hack -- Sliding tracked monster */
        if (health_who == (int)(m_max)) health_track(m_idx);

        /* Structure copy the final monster onto the dead monster */
        m_list[m_idx] = m_list[m_max];
    }

    /* Wipe the hole (again) */
    WIPE(&m_list[m_max], monster_type);
}


/*
 * Delete a monster by index.
 *
 * This function causes the given monster to cease to exist for
 * all intents and purposes.  The monster record is left in place
 * (for no specific reason) but the record is wiped, marking it
 * as "dead" (no race index) so that it can be "skipped" when
 * scanning the monster array and "excised" when necessary.
 *
 * The actual "removal" is done by "remove_monster_idx()" which is
 * only called from "process_monsters()".  This prevents any chance
 * of nasty dangling pointer references.
 *
 * Thus, anyone who makes direct reference to the "m_list[]" array
 * using monster indexes that may have become invalid should be sure
 * to verify that the "r_idx" field is non-zero.  All references
 * to "m_list[c_ptr->m_idx]" are guaranteed to be valid, see below.
 *
 * Note that compaction should probably not be attempted during the
 * "process_monsters()" function, since it will fail.  This is handled
 * by only allowing about 30 monster reproductions per game round, by
 * use of the "tighten_m_list()" function called from "dungeon.c".
 */
void delete_monster_idx(int i)
{
    monster_type *m_ptr = &m_list[i];

    monster_lore *l_ptr = &l_list[m_ptr->r_idx];

    int y = m_ptr->fy;
    int x = m_ptr->fx;

    cave_type *c_ptr = &cave[y][x];


    /* One less of this monster on this level */
    l_ptr->cur_num--;

    /* Hack -- remove target monster */
    if (i == target_who) target_who = 0;

    /* Hack -- remove tracked monster */
    if (i == health_who) health_track(0);


    /* Wipe the Monster */
    WIPE(m_ptr, monster_type);


    /* Monster is gone */
    c_ptr->m_idx = 0;

    /* Visual update */
    lite_spot(y, x);
    
    
    /* Count the monsters */
    m_cnt--;
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
static void compact_monsters(int size)
{
    int		i, num, cnt;
    int		cur_lev, cur_dis, chance;


    /* Message */
    msg_print("Compacting monsters...");

    /* Compact at least 'size' objects */
    for (num = 0, cnt = 1; num <= size; cnt++) {

        /* Get more vicious each iteration */
        cur_lev = 5 * cnt;

        /* Get closer each iteration */
        cur_dis = 5 * (20 - cnt);

        /* Check all the monsters */
        for (i = MIN_M_IDX; i < m_max; i++) {

            monster_type *m_ptr = &m_list[i];
            monster_race *r_ptr = &r_list[m_ptr->r_idx];

            /* Paranoia -- skip "dead" monsters */
            if (!m_ptr->r_idx) continue;

            /* Hack -- High level monsters start out "immune" */
            if (r_ptr->level > cur_lev) continue;

            /* Ignore nearby monsters */
            if ((cur_dis > 0) && (m_ptr->cdis < cur_dis)) continue;

            /* Saving throw chance */
            chance = 90;
            
            /* Only compact "Quest" Monsters in emergencies */
            if ((r_ptr->rflags1 & RF1_QUESTOR) && (cnt < 1000)) chance = 100;

            /* Try not to compact Unique Monsters */
            if (r_ptr->rflags1 & RF1_UNIQUE) chance = 99;

            /* All monsters get a saving throw */
            if (rand_int(100) < chance) continue;

            /* Delete the monster */
            delete_monster_idx(i);

            /* Count the monster */
            num++;
        }
    }
}



/*
 * Returns a new monster index (or zero)
 */
int m_pop(void)
{
    int i;
    
    /* Normal allocation */
    if (m_max < MAX_M_IDX) {

        /* Access the next hole */
        i = m_max;
        
        /* Expand the array */
        m_max++;

        /* Count the monsters */
        m_cnt++;
        
        /* Return the index */            
        return (i);
    }
    
    /* XXX XXX XXX Mega-Warning */
    msg_print("Unable to create new monster!");

    /* Try not to crash */
    return (0);
}







/*
 * Delete/Remove all the monsters when the player leaves the level
 * Note -- we do NOT visually reflect these (irrelevant) changes
 */
void wipe_m_list()
{
    int i;

    /* Delete all the monsters */
    for (i = MIN_M_IDX; i < m_max; i++) {

        monster_type *m_ptr = &m_list[i];

        monster_lore *l_ptr = &l_list[m_ptr->r_idx];

        int y = m_ptr->fy;
        int x = m_ptr->fx;

        cave_type *c_ptr = &cave[y][x];

        /* Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* One less of this monster on this level */
        l_ptr->cur_num--;

        /* Hack -- remove target monster */
        if (i == target_who) target_who = 0;

        /* Hack -- remove tracked monster */
        if (i == health_who) health_track(0);

        /* Wipe the Monster */
        WIPE(m_ptr, monster_type);

        /* Monster is gone */
        c_ptr->m_idx = 0;
    }

    /* Reset the monster array */
    m_nxt = m_max = MIN_M_IDX;
    
    /* No more monsters */
    m_cnt = 0;
}




/*
 * Choose a monster race that seems "appropriate" to the given level
 *
 * This function uses the "allocation table" built in "init.c".
 *
 * There is a small chance (1/50) of "boosting" the given depth by
 * a small amount (up to four levels).
 *
 * It is (slightly) more likely to acquire a monster of the given level
 * than one of a lower level.  This is done by choosing several monsters
 * appropriate to the given level and keeping the "hardest" one.
 */
int get_mon_num(int level)
{
    int		i, try, r_idx;

    monster_race *r_ptr;

    /* Obtain the table */
    s16b *t_lev = alloc_race_index;
    race_entry *table = alloc_race_table;


    /* Pick a monster */
    while (1) {

        /* Town level is easy */
        if (level <= 0) {

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
            if (try > MAX_R_LEV - 1) try = MAX_R_LEV - 1;

            /* Pick any monster at or below the given level */
            i = rand_int(t_lev[try]);

            /* Sometimes try for a "harder" monster */
            if (rand_int(3) != 0) {

                /* Pick another monster at or below the given level */
                int j = rand_int(t_lev[try]);

                /* Keep it if it is "better" */
                if (table[i].locale < table[j].locale) i = j;
            }

            /* Sometimes try for a "harder" monster */
            if (rand_int(3) != 0) {

                /* Pick another monster at or below the given level */
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
 *   0x23 --> Reflexive, genderized if visable ("himself") or "itself"
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





/*
 * This function updates the monster record of the given monster
 *
 * This involves extracting the distance to the player, checking
 * for visibility (natural, infravision, see-invis, telepathy),
 * updating the monster visibility flag, redrawing or erasing the
 * monster when the visibility changes, and taking note of any
 * "visual" features of the monster (cold-blooded, invisible, etc).
 *
 * The only monster fields that are changed here are "cdis" and "ml".
 *
 * There are a few cases where the calling routine knows that the
 * distance from the player to the monster has not changed, and so
 * we have a special parameter to request distance computation.
 * This lets many calls to this function run very quickly.
 *
 * Note that it is important to call this function every time a monster
 * moves, for that monster, and every time the player moves, for all
 * monsters, and every time the player state changes, for all monsters.
 * But we only need to recalculate distance for the first two situations.
 *
 * Note in particular that any change in the player's "state" that
 * affects visibility *must* call the update_monsters() function.
 * This includes blindness, infravision, and see invisibile.
 *
 * The routines that actually move the monsters call this routine
 * directly, and the ones that move the player call update_monsters().
 *
 * Routines that light/darken rooms, and cause player blindness, do
 * it too.  Keep it in mind if a monster ever acquires the ability
 * to make itself invisible without moving.
 *
 * Note that this function is called once per monster every time the
 * player moves, so it is important to optimize it for monsters which
 * are far away.  Note the optimization which skips monsters which
 * are far away and were invisible last turn.
 */
void update_mon(int m_idx, bool dist)
{
    monster_type *m_ptr = &m_list[m_idx];

    monster_race *r_ptr = &r_list[m_ptr->r_idx];
    monster_lore *l_ptr = &l_list[m_ptr->r_idx];

    /* The current monster location */
    int fy = m_ptr->fy;
    int fx = m_ptr->fx;

    /* Can the monster be sensed in any way? */
    bool flag = FALSE;

    /* Various extra flags */
    bool do_viewable = FALSE;
    bool do_telepathy = FALSE;
    bool do_empty_mind = FALSE;
    bool do_weird_mind = FALSE;
    bool do_invisible = FALSE;
    bool do_cold_blood = FALSE;


    /* Calculate distance */
    if (dist) {

        /* Calculate the approximate distance from player */
        int dy = (py > fy) ? (py - fy) : (fy - py);
        int dx = (px > fx) ? (px - fx) : (fx - px);
        int d = (dy > dx) ? (dy + (dx>>1)) : (dx + (dy>>1));

        /* Save the distance (in a byte) */
        m_ptr->cdis = (d < 255) ? d : 255;
    }


    /* Efficiency -- Ignore distant unseen monsters */
    if ((m_ptr->cdis > MAX_SIGHT) && !m_ptr->ml) return;


    /* The monster is on the current "panel" */
    if (panel_contains(fy, fx)) {

        /* Get the monster race (to check flags) */
        monster_race *r_ptr = &r_list[m_ptr->r_idx];

        /* Telepathy can see all "nearby" monsters with "minds" */
        if (p_ptr->telepathy && (m_ptr->cdis <= MAX_SIGHT)) {

            /* Empty mind, no telepathy */
            if (r_ptr->rflags2 & RF2_EMPTY_MIND) {
                do_empty_mind = TRUE;
            }
            
            /* Weird mind, occasional telepathy */
            else if (r_ptr->rflags2 & RF2_WEIRD_MIND) {
                do_weird_mind = TRUE;
                if (rand_int(10) == 0) do_telepathy = TRUE;
            }
            
            /* Normal mind, allow telepathy */
            else {
                do_telepathy = TRUE;
            }
            
            /* Apply telepathy */
            if (do_telepathy) {

                /* Apply telepathy */
                flag = TRUE;

                /* Hack -- Memorize mental flags */
                if (r_ptr->rflags2 & RF2_SMART) l_ptr->flags2 |= RF2_SMART;
                if (r_ptr->rflags2 & RF2_STUPID) l_ptr->flags2 |= RF2_STUPID;
            }
        }

        /* Normal line of sight, and player is not blind */
        if (player_has_los_bold(fy, fx) && (!p_ptr->blind)) {

            /* Remember line of sight */
            do_viewable = TRUE;
            
            /* Infravision is able to see "nearby" monsters */
            if (p_ptr->see_infra &&
                (m_ptr->cdis <= (unsigned)(p_ptr->see_infra))) {

                /* Infravision only works on "warm" creatures */
                /* Below, we will need to know that infravision failed */
                if (r_ptr->rflags2 & RF2_COLD_BLOOD) do_cold_blood = TRUE;

                /* Infravision works */
                if (!do_cold_blood) flag = TRUE;
            }

            /* Check for "illumination" of the monster grid */
            if (player_can_see_bold(fy, fx)) {

                /* Take note of invisibility */
                if (r_ptr->rflags2 & RF2_INVISIBLE) do_invisible = TRUE;

                /* Visible, or detectable, monsters get seen */
                if (!do_invisible || p_ptr->see_inv) flag = TRUE;
            }
        }

        /* Hack -- Wizards have "perfect telepathy" */
        if (wizard && (m_ptr->cdis <= MAX_SIGHT)) flag = TRUE;
    }


    /* The monster is now visible */
    if (flag) {

        /* It was previously unseen */
        if (!m_ptr->ml) {

            /* Mark Monster as visible */
            m_ptr->ml = TRUE;

            /* Draw the monster */
            lite_spot(fy, fx);

            /* Update health bar as needed */
            if (health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

            /* Count "fresh" sightings */
            if (l_ptr->sights < MAX_SHORT) l_ptr->sights++;

            /* Appearing monsters can disturb the player */
            if (disturb_enter) disturb(1, 0);
        }

#ifdef SHIMMER_MONSTERS

        /* XXX XXX XXX Hack -- make multi-hued monsters shimmer */
        else if (r_ptr->rflags1 & RF1_ATTR_MULTI) {

            /* Draw the monster */
            lite_spot(fy, fx);
        }

#endif
        
        /* Memorize various observable flags */
        if (do_empty_mind) l_ptr->flags2 |= RF2_EMPTY_MIND;
        if (do_weird_mind) l_ptr->flags2 |= RF2_WEIRD_MIND;
        if (do_cold_blood) l_ptr->flags2 |= RF2_COLD_BLOOD;
        if (do_invisible) l_ptr->flags2 |= RF2_INVISIBLE;
    }

    /* The monster has disappeared */
    else {

        /* It was previously seen */
        if (m_ptr->ml) {

            /* Mark monster as hidden */
            m_ptr->ml = FALSE;

            /* Erase the monster */
            lite_spot(fy, fx);     

            /* Update health bar as needed */
            if (health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

            /* Disappearing monsters can "disturb" player. */
            if (disturb_leave) disturb(0, 0);
        }
    }
}




/*
 * This function simply updates all the (non-dead) monsters (see above).
 */
void update_monsters(bool dist)
{
    int          i;

    /* Update each (live) monster */
    for (i = MIN_M_IDX; i < m_max; i++) {

        monster_type *m_ptr = &m_list[i];
        
        /* Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Update the monster */
        update_mon(i, dist);
    }
}





/*
 * Attempt to place the given monster at the given location.
 *
 * Note that monster race "zero" is no longer a valid race index!
 *
 * To give the player a sporting chance, any monster that appears in
 * line-of-sight and is extremely dangerous can be marked as
 * "FORCE_SLEEP", which will cause them to be placed with low energy,
 * which often (but not always) lets the player move before they do.
 *
 * Refuses to place out-of-depth Quest Monsters.
 *
 * Note that once a monster has been killed 30000 times during a single
 * life, that monster will no longer be created during that life.
 *
 * Note that certain monsters are now marked as requiring an "escort",
 * which is a collection of monsters with similar "race" but lower level.
 * Some monsters induce a fake "group" flag on their escorts.
 */
bool place_monster(int y, int x, int r_idx, int slp)
{
    int			i;

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


    /* Powerful monster */
    if (r_ptr->level > dun_level) {

        /* Uniques get rating based on "out of depth" amount */
        if (r_ptr->rflags1 & RF1_UNIQUE) {
            if (cheat_hear) msg_format("Deep Unique (%s).", r_ptr->name);
            rating += (r_ptr->level - dun_level);
        }

        /* Normal monsters are worth "half" as much */
        else {
            if (cheat_hear) msg_format("Deep Monster (%s).", r_ptr->name);
            rating += (r_ptr->level - dun_level) / 2;
        }
    }

    /* Note the monster */
    else if (r_ptr->rflags1 & RF1_UNIQUE) {

        /* Unique monsters induce message */
        if (cheat_hear) msg_format("Unique (%s).", r_ptr->name);
    }


    /* Access the location */
    c_ptr = &cave[y][x];

    /* Make a new monster */
    c_ptr->m_idx = m_pop();

    /* Mega-Hack -- catch failure */
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
    m_ptr->xtra = 0;
    
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

    /* Wake up... */
    else {
        m_ptr->csleep = 0;
    }


    /* Hack -- Reduce risk of "instant death by breath weapons" */
    if (r_ptr->rflags1 & RF1_FORCE_SLEEP) {

        /* Start out with minimal energy */
        m_ptr->energy = rand_int(10);
    }


    /* Escorts for certain monsters */
    if (r_ptr->rflags1 & RF1_ESCORT) {

        /* Try to place several "escorts" */
        for (i = 0; i < 50; i++) {

            int n, z, nx, ny, d = 3;

            /* Pick a location */
            scatter(&ny, &nx, y, x, d, 0);

            /* Require empty grids */
            if (!empty_grid_bold(ny, nx)) continue;

            /* Attempt to pick an escort race */
            for (n = 0; n < 10000; n++) {
            
                monster_race *z_ptr;
            
                /* Pick a random race */
                z = get_mon_num(r_ptr->level);
                
                /* Access the race */
                z_ptr = &r_list[z];

                /* Require similar "race" */
                if (z_ptr->r_char != r_ptr->r_char) continue;

                /* Skip more advanced monsters */
                if (z_ptr->level > r_ptr->level) continue;
            
                /* Skip Unique Monsters */
                if (z_ptr->rflags1 & RF1_UNIQUE) continue;

                /* Skip identical monsters */
                if (z_ptr == r_ptr) continue;
            
                /* Certain monsters have large escorts */
                if (r_ptr->rflags1 & RF1_ESCORTS) {
                    if (place_group(ny, nx, z, slp)) break;
                }
            
                /* Certain monsters come in groups */
                else if (z_ptr->rflags1 & RF1_FRIENDS) {
                    if (place_group(ny, nx, z, slp)) break;
                }

                /* Otherwise, just use a single escort */
                else {
                    if (place_monster(ny, nx, z, slp)) break;
                }
            }
        }
    }

    /* Success */
    return (TRUE);
}




/*
 * Maximum size of a group of monsters
 */
#define GROUP_MAX	64



/*
 * Attempt to place the given monster at the given location,
 * and if successful, place some friends there with him.
 */
bool place_group(int y, int x, int r_idx, int slp)
{
    monster_race *r_ptr = &r_list[r_idx];

    int old, n, i;
    int total = 0, extra = 0;

    int hack_n = 0;

    byte hack_y[GROUP_MAX];
    byte hack_x[GROUP_MAX];


    /* Place the first monster */
    if (!place_monster(y, x, r_idx, slp)) return (FALSE);
    

    /* Pick a group size */
    total = randint(13);
    
    /* Hard monsters, small groups */
    if (r_ptr->level > dun_level) {
        extra = r_ptr->level - dun_level;
        extra = 0 - randint(extra);
    }

    /* Easy monsters, large groups */
    else if (r_ptr->level < dun_level) {
        extra = dun_level - r_ptr->level;
        extra = randint(extra);
    }

    /* Hack -- limit group reduction */
    if (extra > 12) extra = 12;

    /* Modify the group size */
    total += extra;
 
    /* Minimum size */
    if (total < 1) total = 1;

    /* Maximum size */
    if (total > GROUP_MAX) total = GROUP_MAX;
        

    /* Save the rating */
    old = rating;

    /* Start on the monster */
    hack_n = 1;
    hack_x[0] = x;
    hack_y[0] = y;

    /* Puddle monsters, breadth first, up to total */
    for (n = 0; (n < hack_n) && (hack_n < total); n++) {

        /* Grab the location */
        int hx = hack_x[n];
        int hy = hack_y[n];

        /* Check each direction, up to total */
        for (i = 0; (i < 8) && (hack_n < total); i++) {
        
            int mx = hx + ddx[ddd[i]];
            int my = hy + ddy[ddd[i]];

            /* Walls and Monsters block flow */
            if (!empty_grid_bold(my, mx)) continue;
            
            /* Attempt to place another monster */
            if (place_monster(my, mx, r_idx, slp)) {

                /* Add it to the "hack" set */
                hack_y[hack_n] = my;
                hack_x[hack_n] = mx;
                hack_n++;
            }
        }
    }

    /* Hack -- restore the rating */
    rating = old;


    /* Success */
    return (TRUE);
}





 
/*
 * Ghost generation info
 */
 
static cptr ghost_race_names[] = {
    "Human", "Elf", "Elf", "Hobbit", "Gnome",
    "Dwarf", "Orc", "Troll", "Human", "Elf"
};

static cptr ghost_class_names[] = {
    "Warrior", "Mage", "Priest",
    "Rogue", "Ranger", "Paladin"
};

static byte ghost_class_colors[] = {
    TERM_L_BLUE, TERM_RED, TERM_L_GREEN,
    TERM_BLUE, TERM_GREEN, TERM_WHITE
};

static int ghost_race;
static int ghost_class;

static char gb_name[32];



/*
 * Set a "blow" record for the ghost
 */
static void ghost_blow(int i, int m, int e, int d, int s)
{
    monster_race *g = &r_list[MAX_R_IDX-1];

    /* Save the data */
    g->blow[i].method = m;
    g->blow[i].effect = e;
    g->blow[i].d_dice = d;
    g->blow[i].d_side = s;
}


/*
 * Prepare the "ghost" race (method 1)
 */
static void set_ghost_aux_1(void)
{
    monster_race *g = &r_list[MAX_R_IDX-1];

    int i, d1, d2;
    
    int lev = g->level;

    int gr = ghost_race;
    int gc = ghost_class;

    cptr gr_name = ghost_race_names[gr];
    cptr gc_name = ghost_class_names[gc];
        
    
    /* XXX Indent */
    if (TRUE) {
    
        /* A wanderer from the town */
        sprintf(ghost_name, "%s, the %s %s",
                gb_name, gr_name, gc_name);


        /* Use a "player" symbol */
        g->r_char = 'p';

        /* Use a "player" color */
        g->r_attr = ghost_class_colors[gc];


        /* Open doors, bash doors */
        g->rflags1 |= (RF2_OPEN_DOOR | RF2_BASH_DOOR);


        /* Treasure drops */
        g->rflags1 |= (RF1_DROP_60 | RF1_DROP_90);

        /* Treasure drops */
        if (lev >= 10) g->rflags1 |= (RF1_DROP_1D2);
        if (lev >= 20) g->rflags1 |= (RF1_DROP_2D2);
        if (lev >= 30) g->rflags1 |= (RF1_DROP_4D2);

        /* Treasure drops */
        if (lev >= 40) g->rflags1 &= ~(RF1_DROP_4D2);
        if (lev >= 40) g->rflags1 |= (RF1_DROP_GREAT);


        /* Extract an "immunity power" */
        i = (lev / 5) + randint(5);
        
        /* Immunity (by level) */
        switch ((i > 12) ? 12 : i) {
        
            case 12:
                g->rflags3 |= (RF3_IM_POIS);
                
            case 11:
            case 10:
                g->rflags3 |= (RF3_IM_ACID);
                
            case 9:
            case 8:
            case 7:
                g->rflags3 |= (RF3_IM_FIRE);
                
            case 6:
            case 5:
            case 4:
                g->rflags3 |= (RF3_IM_COLD);
                
            case 3:
            case 2:
            case 1:
                g->rflags3 |= (RF3_IM_ELEC);
        }


        /* Extract some spells */
        switch (gc) {

          case 0:	/* Warrior */
            break;

          case 1:	/* Mage */
            g->freq_inate = g->freq_spell = 100 / 2;
            g->rflags4 |= (RF4_ARROW_1);
            g->rflags5 |= (RF5_SLOW | RF5_CONF);
            g->rflags6 |= (RF6_BLINK);
            if (lev > 5) g->rflags5 |= (RF5_BA_POIS);
            if (lev > 7) g->rflags5 |= (RF5_BO_ELEC);
            if (lev > 10) g->rflags5 |= (RF5_BO_COLD);
            if (lev > 12) g->rflags6 |= (RF6_TPORT);
            if (lev > 15) g->rflags5 |= (RF5_BO_ACID);
            if (lev > 20) g->rflags5 |= (RF5_BO_FIRE);
            if (lev > 25) g->rflags5 |= (RF5_BA_COLD);
            if (lev > 25) g->rflags6 |= (RF6_HASTE);
            if (lev > 30) g->rflags5 |= (RF5_BA_FIRE);
            if (lev > 40) g->rflags5 |= (RF5_BO_MANA);
            break;

          case 2:	/* Priest */
            g->freq_inate = g->freq_spell = 100 / 4;
            g->rflags5 |= (RF5_CAUSE_1 | RF5_SCARE);
            if (lev > 5) g->rflags6 |= (RF6_HEAL);
            if (lev > 10) g->rflags5 |= (RF5_BLIND);
            if (lev > 12) g->rflags5 |= (RF5_CAUSE_2);
            if (lev > 18) g->rflags5 |= (RF5_HOLD);
            if (lev > 25) g->rflags5 |= (RF5_CONF);
            if (lev > 30) g->rflags5 |= (RF5_CAUSE_3);
            if (lev > 35) g->rflags5 |= (RF5_DRAIN_MANA);
            break;

          case 3:	/* Rogue */
            g->freq_inate = g->freq_spell = 100 / 6;
            g->rflags6 |= (RF6_BLINK);
            if (lev > 10) g->rflags5 |= (RF5_CONF);
            if (lev > 18) g->rflags5 |= (RF5_SLOW);
            if (lev > 25) g->rflags6 |= (RF6_TPORT);
            if (lev > 30) g->rflags5 |= (RF5_HOLD);
            if (lev > 35) g->rflags6 |= (RF6_TELE_TO);
            break;

          case 4:	/* Ranger */
            g->freq_inate = g->freq_spell = 100 / 6;
            g->rflags4 |= (RF4_ARROW_1);
            if (lev > 5) g->rflags5 |= (RF5_BA_POIS);
            if (lev > 7) g->rflags5 |= (RF5_BO_ELEC);
            if (lev > 10) g->rflags5 |= (RF5_BO_COLD);
            if (lev > 18) g->rflags5 |= (RF5_BO_ACID);
            if (lev > 25) g->rflags5 |= (RF5_BO_FIRE);
            if (lev > 30) g->rflags5 |= (RF5_BA_COLD);
            if (lev > 35) g->rflags5 |= (RF5_BA_FIRE);
            break;

          case 5:	/* Paladin */
            g->freq_inate = g->freq_spell = 100 / 8;
            g->rflags5 |= (RF5_CAUSE_1 | RF5_SCARE);
            if (lev > 5) g->rflags6 |= (RF6_HEAL);
            if (lev > 10) g->rflags5 |= (RF5_BLIND);
            if (lev > 12) g->rflags5 |= (RF5_CAUSE_2);
            if (lev > 18) g->rflags5 |= (RF5_HOLD);
            if (lev > 25) g->rflags5 |= (RF5_CONF);
            if (lev > 30) g->rflags5 |= (RF5_CAUSE_3);
            if (lev > 35) g->rflags5 |= (RF5_DRAIN_MANA);
            break;
        }


        /* Racial properties */
        if (gr == 6) g->rflags3 |= (RF3_ORC);
        if (gr == 7) g->rflags3 |= (RF3_TROLL);


        /* Armor class */
        g->ac = 15 + randint(15);
        
        /* Non mage/priest gets extra armor */
        if ((gc != 1) && (gc != 2)) g->ac += randint(60);


        /* Default speed (normal) */
        g->speed = 110;

        /* High level mages are fast... */
        if ((gc == 1) && (lev >= 20)) g->speed += 10;

        /* High level rogues are fast... */
        if ((gc == 3) && (lev >= 30)) g->speed += 10;


        /* Base damage */
        d1 = 1; d2 = lev + 5;

        /* Break up the damage */
        while ((d1 * 8) < d2) {
            d1 = d1 * 2;
            d2 = d2 / 2;
        }


        /* Extract attacks */
        switch (gc) {

          case 0:	/* Warrior */

            /* Sometimes increase damage */
            if (lev >= 30) d2 = d2 * 2;
            
            /* Normal attacks (four) */
            ghost_blow(0, RBM_HIT, RBE_HURT, d1, d2);
            ghost_blow(1, RBM_HIT, RBE_HURT, d1, d2);
            ghost_blow(2, RBM_HIT, RBE_HURT, d1, d2);
            ghost_blow(3, RBM_HIT, RBE_HURT, d1, d2);
            
            break;

          case 1:	/* Mage */

            /* Sometimes increase damage */
            if (lev >= 30) d2 = d2 * 3 / 2;
            
            /* Normal attacks (one) */
            ghost_blow(0, RBM_HIT, RBE_HURT, d1, d2);

            break;

          case 2:	/* Priest */
          
            /* Sometimes increase damage */
            if (lev >= 30) d2 = d2 * 3 / 2;
            
            /* Normal attacks (two) */
            ghost_blow(0, RBM_HIT, RBE_HURT, d1, d2);
            ghost_blow(0, RBM_HIT, RBE_HURT, d1, d2);

            break;

          case 3:	/* Rogue */

            /* Sometimes increase damage */
            if (lev >= 30) d2 = d2 * 2;
            
            /* Normal attacks */
            ghost_blow(0, RBM_HIT, RBE_HURT, d1, d2);
            ghost_blow(1, RBM_HIT, RBE_HURT, d1, d2);

            /* Special attacks -- Touch to steal */
            ghost_blow(2, RBM_TOUCH, RBE_EAT_ITEM, 0, 0);
            ghost_blow(3, RBM_TOUCH, RBE_EAT_ITEM, 0, 0);
            
            break;

          case 4:	/* Ranger */

            /* Sometimes increase damage */
            if (lev >= 30) d2 = d2 * 2;

            /* Normal attacks (three) */
            ghost_blow(0, RBM_HIT, RBE_HURT, d1, d2);
            ghost_blow(1, RBM_HIT, RBE_HURT, d1, d2);
            ghost_blow(2, RBM_HIT, RBE_HURT, d1, d2);

            break;

          case 5:	/* Paladin */

            /* Sometimes increase damage */
            if (lev >= 30) d2 = d2 * 2;
            
            /* Normal attacks (three) */
            ghost_blow(0, RBM_HIT, RBE_HURT, d1, d2);
            ghost_blow(1, RBM_HIT, RBE_HURT, d1, d2);
            ghost_blow(2, RBM_HIT, RBE_HURT, d1, d2);

            break;
        }
    }
}



/*
 * Prepare the ghost -- method 2
 */
static void set_ghost_aux_2(void)
{
    monster_race *g = &r_list[MAX_R_IDX-1];

    int lev = g->level;

    int gr = ghost_race;

    cptr gr_name = ghost_race_names[gr];
        

    /* The ghost is cold blooded */
    g->rflags2 |= (RF2_COLD_BLOOD);

    /* The ghost is undead */
    g->rflags3 |= (RF3_UNDEAD);

    /* The ghost is immune to poison */
    g->rflags3 |= (RF3_IM_POIS);
    

    /* Make a ghost */
    switch ((lev / 4) + randint(3)) {

      case 1:
      case 2:
      case 3:
        sprintf(ghost_name, "%s, the Skeleton %s", gb_name, gr_name);
        g->r_char = 's';
        g->r_attr = TERM_WHITE;
        g->rflags2 |= (RF2_OPEN_DOOR | RF2_BASH_DOOR);
        g->rflags3 |= (RF3_IM_COLD);
        if (gr == 6) g->rflags3 |= (RF3_ORC);
        if (gr == 7) g->rflags3 |= (RF3_TROLL);
        g->ac = 26;
        g->speed = 110;
        ghost_blow(0, RBM_HIT, RBE_HURT, 1, 6);
        ghost_blow(1, RBM_HIT, RBE_HURT, 1, 6);
        break;

      case 4:
      case 5:
        sprintf(ghost_name, "%s, the Zombified %s", gb_name, gr_name);
        g->r_char = 'z';
        g->r_attr = TERM_GRAY;
        g->rflags1 |= (RF1_DROP_60 | RF1_DROP_90);
        g->rflags2 |= (RF2_OPEN_DOOR | RF2_BASH_DOOR);
        if (gr == 6) g->rflags3 |= (RF3_ORC);
        if (gr == 7) g->rflags3 |= (RF3_TROLL);
        g->ac = 30;
        g->speed = 110;
        g->hside *= 2;
        ghost_blow(0, RBM_HIT, RBE_HURT, 1, 9);
        break;

      case 6:
      case 7:
        sprintf(ghost_name, "%s, the Mummified %s", gb_name, gr_name);
        g->r_char = 'M';
        g->r_attr = TERM_GRAY;
        g->rflags1 |= (RF1_DROP_1D2);
        g->rflags2 |= (RF2_OPEN_DOOR | RF2_BASH_DOOR);
        if (gr == 6) g->rflags3 |= (RF3_ORC);
        if (gr == 7) g->rflags3 |= (RF3_TROLL);
        g->ac = 35;
        g->speed = 110;
        g->hside *= 2;
        g->mexp = (g->mexp * 3) / 2;
        ghost_blow(0, RBM_HIT, RBE_HURT, 2, 8);
        ghost_blow(1, RBM_HIT, RBE_HURT, 2, 8);
        ghost_blow(2, RBM_HIT, RBE_HURT, 2, 8);
        break;

      case 8:
        sprintf(ghost_name, "%s, the Poltergeist", gb_name);
        g->r_char = 'G';
        g->r_attr = TERM_WHITE;
        g->rflags1 |= (RF1_RAND_50 | RF1_RAND_25 | RF1_DROP_1D2);
        g->rflags2 |= (RF2_INVISIBLE | RF2_PASS_WALL);
        g->rflags3 |= (RF3_IM_COLD);
        g->ac = 20;
        g->speed = 130;
        g->mexp = (g->mexp * 3) / 2;
        ghost_blow(0, RBM_HIT, RBE_HURT, 1, 6);
        ghost_blow(1, RBM_HIT, RBE_HURT, 1, 6);
        ghost_blow(2, RBM_TOUCH, RBE_TERRIFY, 0, 0);
        ghost_blow(3, RBM_TOUCH, RBE_TERRIFY, 0, 0);
        break;

      case 9:
      case 10:
        sprintf(ghost_name, "%s, the Spirit", gb_name);
        g->r_char = 'G';
        g->r_attr = TERM_WHITE;
        g->rflags1 |= (RF1_DROP_1D2);
        g->rflags2 |= (RF2_INVISIBLE | RF2_PASS_WALL);
        g->rflags3 |= (RF3_IM_COLD);
        g->ac = 20;
        g->speed = 110;
        g->hside *= 2;
        g->mexp = g->mexp * 3;
        ghost_blow(0, RBM_TOUCH, RBE_LOSE_WIS, 1, 6);
        ghost_blow(1, RBM_TOUCH, RBE_LOSE_DEX, 1, 6);
        ghost_blow(2, RBM_HIT, RBE_HURT, 3, 6);
        ghost_blow(3, RBM_WAIL, RBE_TERRIFY, 0, 0);
        break;

      case 11:
        sprintf(ghost_name, "%s, the Ghost", gb_name);
        g->r_char = 'G';
        g->r_attr = TERM_WHITE;
        g->rflags1 |= (RF1_DROP_1D2);
        g->rflags2 |= (RF2_INVISIBLE | RF2_PASS_WALL);
        g->rflags3 |= (RF3_IM_COLD);
        g->rflags5 |= (RF5_BLIND | RF5_HOLD | RF5_DRAIN_MANA);
        g->freq_inate = g->freq_spell = 100 / 15;
        g->ac = 40;
        g->speed = 120;
        g->hside *= 2;
        g->mexp = (g->mexp * 7) / 2;
        ghost_blow(0, RBM_WAIL, RBE_TERRIFY, 0, 0);
        ghost_blow(1, RBM_TOUCH, RBE_EXP_20, 0, 0);
        ghost_blow(2, RBM_CLAW, RBE_LOSE_INT, 1, 6);
        ghost_blow(3, RBM_CLAW, RBE_LOSE_WIS, 1, 6);
        break;

      case 12:
        sprintf(ghost_name, "%s, the Vampire", gb_name);
        g->r_char = 'V';
        g->r_attr = TERM_VIOLET;
        g->rflags1 |= (RF1_DROP_2D2);
        g->rflags2 |= (RF2_OPEN_DOOR | RF2_BASH_DOOR);
        g->rflags3 |= (RF3_HURT_LITE);
        g->rflags5 |= (RF5_SCARE | RF5_HOLD | RF5_CAUSE_2);
        g->rflags6 |= (RF6_TELE_TO);
        g->freq_inate = g->freq_spell = 100 / 8;
        g->ac = 40;
        g->speed = 110;
        g->hside *= 3;
        g->mexp = g->mexp * 3;
        ghost_blow(0, RBM_HIT, RBE_HURT, 3, 8);
        ghost_blow(1, RBM_HIT, RBE_HURT, 3, 8);
        ghost_blow(2, RBM_BITE, RBE_EXP_40, 0, 0);
        break;

      case 13:
        sprintf(ghost_name, "%s, the Wraith", gb_name);
        g->r_char = 'W';
        g->r_attr = TERM_WHITE;
        g->rflags1 |= (RF1_DROP_2D2 | RF1_DROP_4D2);
        g->rflags2 |= (RF2_OPEN_DOOR | RF2_BASH_DOOR);
        g->rflags3 |= (RF3_IM_COLD | RF3_HURT_LITE);
        g->rflags5 |= (RF5_BLIND | RF5_SCARE | RF5_HOLD);
        g->rflags5 |= (RF5_CAUSE_3 | RF5_BO_NETH);
        g->freq_inate = g->freq_spell = 100 / 7;
        g->ac = 60;
        g->speed = 120;
        g->hside *= 3;
        g->mexp = g->mexp * 5;
        ghost_blow(0, RBM_HIT, RBE_HURT, 3, 8);
        ghost_blow(1, RBM_HIT, RBE_HURT, 3, 8);
        ghost_blow(2, RBM_TOUCH, RBE_EXP_20, 0, 0);
        break;

      case 14:
        sprintf(ghost_name, "%s, the Vampire Lord", gb_name);
        g->r_char = 'V';
        g->r_attr = TERM_BLUE;
        g->rflags1 |= (RF1_DROP_1D2 | RF1_DROP_GREAT);
        g->rflags2 |= (RF2_OPEN_DOOR | RF2_BASH_DOOR);
        g->rflags3 |= (RF3_HURT_LITE);
        g->rflags5 |= (RF5_SCARE | RF5_HOLD | RF5_CAUSE_3 | RF5_BO_NETH);
        g->rflags6 |= (RF6_TELE_TO);
        g->freq_inate = g->freq_spell = 100 / 8;
        g->ac = 80;
        g->speed = 110;
        g->hside *= 2;
        g->hdice *= 2;
        g->mexp = g->mexp * 20;
        ghost_blow(0, RBM_HIT, RBE_HURT, 3, 8);
        ghost_blow(1, RBM_HIT, RBE_HURT, 3, 8);
        ghost_blow(2, RBM_HIT, RBE_HURT, 3, 8);
        ghost_blow(3, RBM_BITE, RBE_EXP_80, 0, 0);
        break;

      case 15:
        sprintf(ghost_name, "%s, the Ghost", gb_name);
        g->r_char = 'G';
        g->r_attr = TERM_WHITE;
        g->rflags1 |= (RF1_DROP_2D2 | RF1_DROP_GREAT);
        g->rflags2 |= (RF2_INVISIBLE | RF2_PASS_WALL);
        g->rflags3 |= (RF3_IM_COLD);
        g->rflags5 |= (RF5_BLIND | RF5_CONF | RF5_HOLD | RF5_DRAIN_MANA);
        g->freq_inate = g->freq_spell = 100 / 5;
        g->ac = 90;
        g->speed = 130;
        g->hside *= 3;
        g->mexp = g->mexp * 20;
        ghost_blow(0, RBM_WAIL, RBE_TERRIFY, 0, 0);
        ghost_blow(1, RBM_TOUCH, RBE_EXP_20, 0, 0);
        ghost_blow(2, RBM_CLAW, RBE_LOSE_INT, 1, 6);
        ghost_blow(3, RBM_CLAW, RBE_LOSE_WIS, 1, 6);
        break;

      case 17:
        sprintf(ghost_name, "%s, the Lich", gb_name);
        g->r_char = 'L';
        g->r_attr = TERM_ORANGE;
        g->rflags1 |= (RF1_DROP_2D2 | RF1_DROP_1D2 | RF1_DROP_GREAT);
        g->rflags2 |= (RF2_SMART | RF2_OPEN_DOOR | RF2_BASH_DOOR);
        g->rflags3 |= (RF3_IM_COLD);
        g->rflags5 |= (RF5_BLIND | RF5_SCARE | RF5_CONF | RF5_HOLD);
        g->rflags5 |= (RF5_DRAIN_MANA | RF5_BA_FIRE | RF5_BA_COLD);
        g->rflags5 |= (RF5_CAUSE_3 | RF5_CAUSE_4 | RF5_BRAIN_SMASH);
        g->rflags6 |= (RF6_BLINK | RF6_TPORT | RF6_TELE_TO | RF6_S_UNDEAD);
        g->freq_inate = g->freq_spell = 100 / 3;
        g->ac = 120;
        g->speed = 120;
        g->hside *= 3;
        g->hdice *= 2;
        g->mexp = g->mexp * 50;
        ghost_blow(0, RBM_TOUCH, RBE_LOSE_DEX, 2, 12);
        ghost_blow(1, RBM_TOUCH, RBE_LOSE_DEX, 2, 12);
        ghost_blow(2, RBM_TOUCH, RBE_UN_POWER, 0, 0);
        ghost_blow(3, RBM_TOUCH, RBE_EXP_40, 0, 0);
        break;

      default:
        sprintf(ghost_name, "%s, the Ghost", gb_name);
        g->r_char = 'G';
        g->r_attr = TERM_WHITE;
        g->rflags1 |= (RF1_DROP_1D2 | RF1_DROP_2D2 | RF1_DROP_GREAT);
        g->rflags2 |= (RF2_SMART | RF2_INVISIBLE | RF2_PASS_WALL);
        g->rflags3 |= (RF3_IM_COLD);
        g->rflags5 |= (RF5_BLIND | RF5_CONF | RF5_HOLD | RF5_BRAIN_SMASH);
        g->rflags5 |= (RF5_DRAIN_MANA | RF5_BA_NETH | RF5_BO_NETH);
        g->rflags6 |= (RF6_TELE_TO | RF6_TELE_LEVEL);
        g->freq_inate = g->freq_spell = 100 / 2;
        g->ac = 130;
        g->speed = 130;
        g->hside *= 2;
        g->hdice *= 2;
        g->mexp = g->mexp * 30;
        ghost_blow(0, RBM_WAIL, RBE_TERRIFY, 0, 0);
        ghost_blow(1, RBM_TOUCH, RBE_EXP_20, 0, 0);
        ghost_blow(2, RBM_CLAW, RBE_LOSE_INT, 1, 6);
        ghost_blow(3, RBM_CLAW, RBE_LOSE_WIS, 1, 6);
        break;
    }
}



/*
 * Hack -- Prepare the "ghost" race
 *
 * XXX Note that g->name always points to "ghost_name"
 *
 * We are given a "name" of the form "Bob" (or "Bob, the xxx"), and
 * a race/class (by index), and a level (usually the dungeon level),
 * and a special "town" flag (which chooses the major ghost "type").
 *
 * Note that "town" ghosts are always level 1 to 50, and other ghosts
 * are always level 1 to 100 (or deeper?)
 *
 * Currently we save the current "ghost race info" in the savefile.
 * Note that ghosts from pre-2.7.7 savefiles are always ignored.
 *
 * Eventually we should probably save the ghost in such a way as
 * to allow it to be "re-extracted" from a small amount of info,
 * such as the "base name", the "race", the "class", the base "hp",
 * the "level", the "town" flag, and the "random seed".  This would
 * make the savefile impervious to changes in the race format.
 *
 * Thus we would need to save "pn", "hp", "gr", "gc", and "lev",
 * plus the "town" flag, plus a random seed of some form.  Note that
 * we already save the "pn" value, followed by a "comma" and "title",
 * and we have the "lev" field as the actual ghost level.  But it is
 * probably best to ignore this storage method for a few versions.
 *
 * We "could" extract the "hp" from the ghost name and current hp's.
 * We "could" extract the "town" flag from the ghost race symbol.
 *
 * Note that each new ghost needs a new "random seed".  And actually,
 * we do not really need a "full" random seed, we could just use a
 * random value from which random numbers can be extracted.  (?)
 */
static void set_ghost(cptr pn, int hp, int gr, int gc, int lev, bool town)
{
    int i;

    monster_race *g = &r_list[MAX_R_IDX-1];


    /* Extract the basic ghost name */
    strcpy(gb_name, pn);

    /* Find the first comma, or end of string */
    for (i = 0; (i < 16) && (gb_name[i]) && (gb_name[i] != ','); i++);

    /* Terminate the name */
    gb_name[i] = '\0';
        
    /* Force a name */
    if (!gb_name[0]) strcpy(gb_name, "Nobody");
    
    /* Capitalize the name */
    if (islower(gb_name[0])) gb_name[0] = toupper(gb_name[0]);


    /* Clear the normal flags */
    g->rflags1 = g->rflags2 = g->rflags3 = 0L;

    /* Clear the spell flags */
    g->rflags4 = g->rflags5 = g->rflags6 = 0L;
    
    
    /* Clear the attacks */
    ghost_blow(0, 0, 0, 0, 0);
    ghost_blow(1, 0, 0, 0, 0);
    ghost_blow(2, 0, 0, 0, 0);
    ghost_blow(3, 0, 0, 0, 0);
    

    /* The ghost never sleeps */
    g->sleep = 0;

    /* The ghost is very attentive */
    g->aaf = 100;


    /* Save the level */
    g->level = lev;

    /* Extract the default experience */
    g->mexp = lev * 5 + 5;


    /* Hack -- Break up the hitpoints */
    for (i = 1; i * i < hp; i++);

    /* Extract the basic hit dice and sides */
    g->hdice = g->hside = i;


    /* Unique monster */
    g->rflags1 |= (RF1_UNIQUE);

    /* Only carry good items */
    g->rflags1 |= (RF1_ONLY_ITEM | RF1_DROP_GOOD);

    /* The ghost is always evil */
    g->rflags3 |= (RF3_EVIL);

    /* Cannot be slept or confused */
    g->rflags3 |= (RF3_NO_SLEEP | RF3_NO_CONF);


    /* Save the race and class */
    ghost_race = gr;
    ghost_class = gc;


    /* Prepare the ghost (method 1) */
    if (town) {

        /* Method 1 */
        set_ghost_aux_1();
    }

    /* Prepare the ghost (method 2) */
    else {

        /* Method 2 */
        set_ghost_aux_2();
    }
}



/*
 * Attempt to place a ghost somewhere.
 */
bool alloc_ghost(void)
{
    int			y, x, hp, level, gr, gc;

    cave_type		*c_ptr;
    monster_type	*m_ptr;

    monster_race	*r_ptr = &r_list[MAX_R_IDX-1];
    monster_lore	*l_ptr = &l_list[MAX_R_IDX-1];

    FILE		*fp;

    bool		err = FALSE;
    bool		town = FALSE;
    
    char		name[100];
    char		tmp[1024];


    /* Hack -- no ghosts in the town */
    if (!dun_level) return (FALSE);

    /* Already have a ghost */
    if (l_ptr->cur_num >= l_ptr->max_num) return (FALSE);


    /* Town -- Use Player Level */
    if (!dun_level) {

#if 0
        /* Assume minimum level */
        if (p_ptr->lev < 5) return (FALSE);

        /* And even then, it only happens sometimes */
        /* if (14 > randint((dun_level / 2) + 11)) return (FALSE); */

        /* Only a 10% chance */
        if (rand_int(10) != 0) return (FALSE);

        /* Level is player level */
        level = p_ptr->lev;
#endif

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
    sprintf(tmp, "%s%sbone.%03d", ANGBAND_DIR_BONE, PATH_SEP, level);

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


    /* Extract "town" flag */
    if (level == p_ptr->lev) town = TRUE;

    /* Set up the ghost */
    set_ghost(name, hp, gr, gc, level, town);


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


    /* Assign the max hitpoints */
    m_ptr->maxhp = maxroll(r_ptr->hdice, r_ptr->hside);

    /* Start out fully healed */
    m_ptr->hp = m_ptr->maxhp;

    /* Extract the base speed */
    m_ptr->mspeed = r_ptr->speed;

    /* Pick a random energy */
    m_ptr->energy = rand_int(100);

    /* Not stunned or sleeping */
    m_ptr->stunned = 0;
    m_ptr->csleep = 0;
    m_ptr->monfear = 0;
    m_ptr->xtra = 0;
    

    /* Mega-Hack -- update the graphic info */
    l_ptr->l_attr = r_ptr->r_attr;
    l_ptr->l_char = r_ptr->r_char;


    /* Update the monster (correctly) */
    m_ptr->ml = FALSE;
    update_mon(c_ptr->m_idx, TRUE);


    /* Hack -- increase the rating */
    rating += 10;

    /* A ghost makes the level special */
    good_item_flag = TRUE;


    /* Success */
    return (TRUE);
}





/*
 * Attempt to allocate a random monster in the dungeon.
 * Place the monster at least "dis" distance from the player.
 * Use "slp" to choose the initial "sleep" status
 */
bool alloc_monster(int dis, int slp)
{
    int			y, x, r_idx;

    monster_race	*r_ptr;


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
        if (place_group(y, x, r_idx, slp)) return (TRUE);
    }

    /* Place a single monster */	
    else {
        if (place_monster(y, x, r_idx, slp)) return (TRUE);
    }

    /* Nope */
    return (FALSE);
}


/*
 * Summon a monster of the given level near the given location.
 * We return TRUE if anything was summoned.
 */
int summon_monster(int y1, int x1, int lev)
{
    int			i, y, x, r_idx;

    monster_race	*r_ptr;


    /* Try twenty locations */
    for (i = 0; i < 20; i++) {

        int d = (i / 15) + 1;

        /* Pick a nearby location */
        scatter(&y, &x, y1, x1, d, 0);

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
 * Place a monster (of the specified "type") near the given
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
    int		r_idx, i, x, y;

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


    /* Try to place it */
    for (i = 0; i < 20; ++i) {

        /* Pick a distance */
        int d = (i / 15) + 1;

        /* Pick a location */
        scatter(&y, &x, y1, x1, d, 0);

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
 * Returns whether a given monster will try to run from the player.
 *
 * Monsters will attempt to avoid very powerful players.  See below.
 *
 * Because this function is called so often, little details are important
 * for efficiency.  Like not using "mod" or "div" when possible.  And
 * attempting to check the conditions in an optimal order.  Note that
 * "(x << 2) == (x * 4)" if "x" has enough bits to hold the result.
 *
 * Note that this function is responsible for about one to five percent
 * of the processor use in normal conditions...
 */
static int mon_will_run(int m_idx)
{
    monster_type *m_ptr = &m_list[m_idx];

#ifdef ALLOW_TERROR

    monster_race *r_ptr = &r_list[m_ptr->r_idx];

    u16b p_lev, m_lev;
    u16b p_chp, p_mhp;
    u16b m_chp, m_mhp;
    u32b p_val, m_val;

#endif

    /* Keep monsters from running too far away */
    if (m_ptr->cdis > MAX_SIGHT + 5) return (FALSE);

    /* All "afraid" monsters will run away */
    if (m_ptr->monfear) return (TRUE);

#ifdef ALLOW_TERROR

    /* Nearby monsters will not become terrified */
    if (m_ptr->cdis <= 5) return (FALSE);

    /* Examine player power (level) */
    p_lev = p_ptr->lev;

    /* Examine monster power (level plus morale) */
    m_lev = r_ptr->level + (m_idx & 0x08) + 25;

    /* Optimize extreme cases below */
    if (m_lev > p_lev + 4) return (FALSE);
    if (m_lev + 4 <= p_lev) return (TRUE);

    /* Examine player health */
    p_chp = p_ptr->chp;
    p_mhp = p_ptr->mhp;

    /* Examine monster health */
    m_chp = m_ptr->hp;
    m_mhp = m_ptr->maxhp;

    /* Prepare to optimize the calculation */
    p_val = (p_lev * p_mhp) + (p_chp << 2);	/* div p_mhp */
    m_val = (m_lev * m_mhp) + (m_chp << 2);	/* div m_mhp */

    /* Strong players scare strong monsters */
    if (p_val * m_mhp > m_val * p_mhp) return (TRUE);

#endif

    /* Assume no terror */
    return (FALSE);
}


#ifdef WDT_TRACK_OPTIONS

/*
 * Set the "track" field for a monster
 */
static void set_t_bit(int m_idx, bool player, bool direct)
{
    monster_type *m_ptr = &m_list[m_idx];
    
    m_ptr->t_bit = 0;
    if (player) m_ptr->t_bit |= MTB_PLAYER;
    if (direct) m_ptr->t_bit |= MTB_DIRECT;
}


/*
 * Update the target.  This is a slightly silly function.
 */
static void get_target(int m_idx)
{
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_list[m_ptr->r_idx];


    /* Hack -- Certain monsters can "see" the player */
    if (player_has_los_bold(m_ptr->fy, m_ptr->fx)) {

        /* Target the player */
        m_ptr->ty = py;
        m_ptr->tx = px;

        /* Remember for a few turns */
        m_ptr->t_dur = 10;
        
        /* Direct knowledge of the player */
        set_t_bit(m_idx, TRUE, TRUE);
        
        /* Done */
        return;
    }
    
    
    /* Hack -- Certain monsters "know" where the player is */
    if (((r_ptr->rflags2 & RF2_PASS_WALL) ||
         (r_ptr->rflags2 & RF2_KILL_WALL)) &&
        (m_ptr->cdis < r_ptr->aaf)) {

        /* Target the player */
        m_ptr->ty = py;
        m_ptr->tx = px;

        /* But only for one turn */
        m_ptr->t_dur = 1;

        /* Indirect knowledge of the player */
        set_t_bit(m_idx, TRUE, FALSE);

        /* Done */
        return;
    }
    
    
    /* Check to see if we used to know where the player was... */
    if (m_ptr->t_bit & MTB_PLAYER) {
        
        /* We used to have direct knowledge, remember it */
        if (m_ptr->t_bit & MTB_DIRECT) {
            m_ptr->t_dur = 2 + r_ptr->aaf / 3;
            set_t_bit(m_idx, TRUE, FALSE);
        }

        /* We have been relying on old information, age it */
        else {
            if (m_ptr->t_dur) m_ptr->t_dur--;
            set_t_bit(m_idx, TRUE, FALSE);
        }

        /* Done (unless arrived) */
        if (m_ptr->fy != m_ptr->ty) return;
        if (m_ptr->fx != m_ptr->tx) return;
    }


    /* Follow footsteps */
    if (TRUE) {

        int i, y, x, n, best, here;

        int y1 = m_ptr->fy;
        int x1 = m_ptr->fx;
            
        cave_type *c_ptr;

        /* Monster grid */
        c_ptr = &cave[y1][x1];

        /* No "good" choices so far */
        n = 0;
        
        /* Best so far */
        here = best = c_ptr->track;

        /* Target ourself */
        m_ptr->ty = y1;
        m_ptr->tx = x1;

        /* Only for one turn */
        m_ptr->t_dur = 1;

        /* Not targeting player */
        set_t_bit(m_idx, FALSE, FALSE);


        /* Check adjacent grids, diagonals last */
        for (i = 0; i < 8; i++) {

            /* Get the location */
            y = y1 + ddy[ddd[i]];
            x = x1 + ddx[ddd[i]];

            /* Get the grid */
            c_ptr = &cave[y][x];

            /* Don't even look at walls */
            if (c_ptr->info & GRID_WALL_MASK) continue;

            /* Ignore less trampled locations */
            if (c_ptr->track < best) continue;

            /* Count valid locations */
            n++;
            
            /* Hack -- Choose between "equally valid" locations */
            if ((c_ptr->track == best) && (rand_int(n) > 0)) continue;
            
            /* Save the new best location */
            best = c_ptr->track;

            /* Target that location */
            m_ptr->ty = y;
            m_ptr->tx = x;

            /* But only for one turn */
            m_ptr->t_dur = 1;
        
            /* No direct knowledge */
            set_t_bit(m_idx, FALSE, FALSE);
        }

        /* Hack -- do not wander aimlessly */
        if (best == here) m_ptr->t_dur = 0;
    }
}

#endif



#ifdef MONSTER_FLOW

/*
 * Choose the "best" direction for "flowing"
 *
 * Note that ghosts and rock-eaters are never allowed to "flow"
 *
 * Prefer "non-diagonal" directions, but twiddle them a little
 * to angle slightly towards the player's actual location.
 *
 * Allow very perceptive monsters to track old "spoor" left by
 * previous locations occupied by the player.  This will tend
 * to have monsters end up either near the player or on a grid
 * recently occupied by the player (and left via "teleport").
 * Note that if "smell" is turned on, all monsters get vicious.
 * Also note that teleporting away from a location will cause
 * the monsters who were chasing you to converge on that location
 * as long as you are still near enough to "annoy" them without
 * being close enough to chase directly.  I have no idea what will
 * happen if you combine "smell" with low "aaf" values.
 */
static bool get_moves_aux(int m_idx, int *yp, int *xp)
{
    int i, y, x, y1, x1, when = 0, cost = 999;

    cave_type *c_ptr;

    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_list[m_ptr->r_idx];

    /* Monster flowing disabled */
    if (!flow_by_sound) return (FALSE);

    /* Monster can go through rocks */
    if (r_ptr->rflags2 & RF2_PASS_WALL) return (FALSE);
    if (r_ptr->rflags2 & RF2_KILL_WALL) return (FALSE);

    /* Monster location */
    y1 = m_ptr->fy;
    x1 = m_ptr->fx;

    /* Monster grid */
    c_ptr = &cave[y1][x1];

    /* The player is not currently near the monster grid */
    if (c_ptr->when < cave[py][px].when) {

        /* The player has never been near the monster grid */
        if (!c_ptr->when) return (FALSE);

        /* The monster is not allowed to track the player */
        if (!flow_by_smell) return (FALSE);
    }

    /* Monster is too far away to notice the player */
    if (c_ptr->cost > MONSTER_FLOW_DEPTH) return (FALSE);
    if (c_ptr->cost > r_ptr->aaf) return (FALSE);

    /* Hack -- Player can see us, run towards him */
    if (player_has_los_bold(y1, x1)) return (FALSE);

    /* Check nearby grids, diagonals first */
    for (i = 7; i >= 0; i--) {

        /* Get the location */
        y = y1 + ddy[ddd[i]];
        x = x1 + ddx[ddd[i]];

        /* Ignore illegal locations */
        if (!cave[y][x].when) continue;

        /* Ignore ancient locations */
        if (cave[y][x].when < when) continue;

        /* Ignore distant locations */
        if (cave[y][x].cost > cost) continue;

        /* Save the cost and time */
        when = cave[y][x].when;
        cost = cave[y][x].cost;

        /* Hack -- Save the "twiddled" location */
        (*yp) = py + 100 * ddy[ddd[i]];
        (*xp) = px + 100 * ddx[ddd[i]];
    }

    /* No legal move (?) */
    if (!when) return (FALSE);

    /* Success */
    return (TRUE);
}

#endif


/*
 * Choose correct directions for monster movement	-RAK-	
 *
 * Perhaps monster fear should only work when player can be seen?
 *
 * This function does not have to worry about monster mobility
 */
static void get_moves(int m_idx, int *mm)
{
    monster_type *m_ptr = &m_list[m_idx];

    int y, ay, x, ax;

    int move_val = 0;

    int y2 = py;
    int x2 = px;


#ifdef WDT_TRACK_OPTIONS
    /* Follow the player */
    if (track_follow) {
    
        /* Invalid target */
        if (m_ptr->t_dur <= 0) return;
        
        /* Approach the target */
        y2 = m_ptr->ty;
        x2 = m_ptr->tx;
    }
#endif

#ifdef MONSTER_FLOW
    /* Flow towards the player */
    if (flow_by_sound) {

        /* Flow towards the player */
        (void)get_moves_aux(m_idx, &y2, &x2);
    }
#endif

    /* Extract the "pseudo-direction" */
    y = m_ptr->fy - y2;
    x = m_ptr->fx - x2;


    /* Apply fear if possible and necessary */
    if (mon_will_run(m_idx)) {

        /* XXX XXX Not very "smart" */
        y = (-y), x = (-x);
    }


    /* Extract the "absolute distances" */
    ax = ABS(x);
    ay = ABS(y);

    /* Do something weird */
    if (y < 0) move_val += 8;
    if (x > 0) move_val += 4;

    /* Prevent the diamond maneuvre */
    if (ay > (ax << 1)) {
        move_val++;
        move_val++;
    }
    else if (ax > (ay << 1)) {
        move_val++;
    }

    /* Extract some directions */
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





/*
 * Given some move choices, make the first "legal" one.
 *
 * In several cases, we directly update the monster lore.
 *
 * XXX Monster fear is slightly odd, in particular, monsters will
 * fixate on opening a door even if they cannot open it.  Actually,
 * the same thing happens to normal monsters when they hit a door.
 *
 * XXX Technically, need to check for monster in the way
 * combined with that monster being in a wall (or door?)
 *
 * This routine needs to make sure (it currently does) that immobile
 * monsters do not move (whether from fear or confusion).
 *
 * Note that the "direction" of "5" means "pick a random direction".
 *
 * Assume that the move array is terminated with a zero.
 */
static void make_move(int m_idx, int *mm)
{
    int			i, ny, nx;

    cave_type    	*c_ptr;
    inven_type  	*i_ptr;

    monster_type	*m_ptr = &m_list[m_idx];
    monster_race	*r_ptr = &r_list[m_ptr->r_idx];

    bool		do_turn = FALSE;
    bool		do_move = FALSE;
    bool		do_view = FALSE;

    bool		did_open_door = FALSE;
    bool		did_bash_door = FALSE;
    bool		did_take_item = FALSE;
    bool		did_kill_item = FALSE;
    bool		did_move_body = FALSE;
    bool		did_kill_body = FALSE;
    bool		did_pass_wall = FALSE;
    bool		did_kill_wall = FALSE;


    /* Skip "dead" monsters */
    if (!m_ptr->r_idx) return;
    
    
    /* Take a zero-terminated array of "directions" */
    for (i = 0; !do_turn && (i < 8) && mm[i]; i++) {

        /* Refuse to not move */
        if (mm[i] == 5) mm[i] = ddd[rand_int(8)];
        
        /* Get the destination */
        ny = m_ptr->fy + ddy[mm[i]];
        nx = m_ptr->fx + ddx[mm[i]];

        /* Access that cave grid */
        c_ptr = &cave[ny][nx];

        /* Access that cave grid's contents */
        i_ptr = &i_list[c_ptr->i_idx];


        /* Floor is open? */
        if (floor_grid_bold(ny, nx)) {

            /* Go ahead and move */
            do_move = TRUE;
        }

        /* Permanent blockage */
        else if (c_ptr->info & GRID_PERM) {

            /* Nothing */
        }

        /* Creature moves through walls? */
        else if (r_ptr->rflags2 & RF2_PASS_WALL) {

            /* Pass through walls/doors/rubble */
            do_move = TRUE;

            /* Monster went through a wall */
            did_pass_wall = TRUE;
        }

        /* Crunch up those Walls Morgoth and Umber Hulks!!!! */
        else if (r_ptr->rflags2 & RF2_KILL_WALL) {

            /* Eat through walls/doors/rubble */
            do_move = TRUE;

            /* Monster destroyed a wall */
            did_kill_wall = TRUE;

            /* Hack -- destroy doors and rubble */
            if ((i_ptr->tval == TV_CLOSED_DOOR) ||
                (i_ptr->tval == TV_SECRET_DOOR) ||
                (i_ptr->tval == TV_RUBBLE)) {

                /* Delete the door/rubble */
                delete_object(ny, nx);
            }

            /* Clear the wall code, if any */
            c_ptr->info &= ~GRID_WALL_MASK;

            /* Forget the "field mark", if any */
            c_ptr->info &= ~GRID_MARK;

            /* Redisplay the grid */
            lite_spot(ny, nx);

            /* Note changes to viewable region */
            if (player_has_los_bold(ny, nx)) do_view = TRUE;
        }

        /* Creature can open doors? */
        else if (c_ptr->i_idx) {

            /* Open closed/secret doors */
            if ((i_ptr->tval == TV_CLOSED_DOOR) ||
                (i_ptr->tval == TV_SECRET_DOOR)) {

                bool may_bash = TRUE;

                /* Take a turn */
                do_turn = TRUE;

                /* Creature can open doors. */
                if (r_ptr->rflags2 & RF2_OPEN_DOOR) {

                    /* Normal doors */
                    if (i_ptr->pval == 0) {

                        /* The door is open */
                        did_open_door = TRUE;

                        /* Do not bash the door */
                        may_bash = FALSE;
                    }

                    /* Locked doors -- take a turn to unlock it */
                    else if (i_ptr->pval > 0) {

                        /* Try to unlock it */
                        if (randint((m_ptr->hp + 1) * (50 + i_ptr->pval)) <
                            40 * (m_ptr->hp - 10 - i_ptr->pval)) {

                            /* The door is unlocked */
                            i_ptr->pval = 0;

                            /* Do not bash the door */
                            may_bash = FALSE;
                        }
                    }
                }

                /* Stuck doors -- attempt to bash them down if allowed */
                if (may_bash && (r_ptr->rflags2 & RF2_BASH_DOOR)) {

                    /* Attempt to Bash */
                    if (randint((m_ptr->hp + 1) * (50 - i_ptr->pval)) <
                        40 * (m_ptr->hp - 10 + i_ptr->pval)) {

                        /* Message */
                        msg_print("You hear a door burst open!");
                        disturb(1, 0);

                        /* The door was bashed open */
                        did_bash_door = TRUE;

                        /* Hack -- fall into doorway */
                        do_move = TRUE;
                    }
                }


                /* Deal with doors in the way */
                if (did_open_door || did_bash_door) {

                    /* XXX Should create a new object XXX */
                    invcopy(i_ptr, OBJ_OPEN_DOOR);

                    /* Place it in the dungeon */
                    i_ptr->iy = ny;
                    i_ptr->ix = nx;

                    /* 50% chance of breaking door during bash */
                    if (did_bash_door) i_ptr->pval = 0 - rand_int(2);

                    /* Redraw door */
                    lite_spot(ny, nx);

                    /* Handle viewable doors */
                    if (player_has_los_bold(ny, nx)) do_view = TRUE;
                }
            }
        }


        /* Hack -- check for Glyph of Warding */
        if (do_move &&
            (c_ptr->i_idx) &&
            (i_ptr->tval == TV_VIS_TRAP) &&
            (i_ptr->sval == SV_TRAP_GLYPH)) {

            /* Assume no move allowed */
            do_move = FALSE;

            /* Break the ward */
            if (randint(BREAK_GLYPH) < r_ptr->level) {

                /* Describe observable breakage */
                if (player_can_see_bold(ny, nx)) {
                    msg_print("The rune of protection is broken!");
                }

                /* Delete the rune */
                delete_object(ny, nx);

                /* Allow movement */
                do_move = TRUE;
            }
        }


        /* Some monsters never attack */
        if (do_move && (r_ptr->rflags1 & RF1_NEVER_BLOW)) {
        
            /* Never attack the player */
            if (c_ptr->m_idx == 1) {

                /* Hack -- memorize lack of attacks */
                /* if (m_ptr->ml) l_ptr->flags1 |= RF1_NEVER_BLOW; */
                
                /* Do not move */
                do_move = FALSE;
            }
        }


        /* The player is in the way.  Attack him. */
        if (do_move && (c_ptr->m_idx == 1)) {

            /* Do the attack */
            (void)make_attack_normal(m_idx);

            /* Do not move */
            do_move = FALSE;

            /* Took a turn */
            do_turn = TRUE;
        }


        /* Some monsters never move */
        if (do_move && (r_ptr->rflags1 & RF1_NEVER_MOVE)) {

            /* Hack -- memorize lack of attacks */
            /* if (m_ptr->ml) l_ptr->r_flags1 |= RF1_NEVER_MOVE; */

            /* Do not move */
            do_move = FALSE;
        }


        /* A monster is in the way */
        if (do_move && (c_ptr->m_idx > 1)) {

            /* Assume no movement */
            do_move = FALSE;

            /* Kill weaker monsters */
            if ((r_ptr->rflags2 & RF2_KILL_BODY) &&
                (r_ptr->mexp > r_list[m_list[c_ptr->m_idx].r_idx].mexp)) {

                /* Allow movement */
                do_move = TRUE;

                /* Monster ate another monster */
                did_kill_body = TRUE;

                /* XXX XXX XXX Message */
                
                /* Kill the monster */
                delete_monster(ny, nx);
            }

            /* Push past weaker monsters */
            if ((r_ptr->rflags2 & RF2_MOVE_BODY) &&
                (r_ptr->mexp > r_list[m_list[c_ptr->m_idx].r_idx].mexp)) {

                /* Allow movement */
                do_move = TRUE;

                /* Monster pushed past another monster */
                did_move_body = TRUE;

                /* XXX XXX XXX Message */                
            }
        }


        /* Creature has been allowed move */
        if (do_move) {

            /* Take a turn */
            do_turn = TRUE;

            /* Move the creature */
            move_rec(m_ptr->fy, m_ptr->fx, ny, nx);

            /* Update the monster */
            update_mon(m_idx, TRUE);

#ifdef WDT_TRACK_OPTIONS
            /* Erase the player footprints */
            if (c_ptr->track > 4) c_ptr->track -= 2;
            if (c_ptr->track > -7) c_ptr->track -= 1;
#endif

            /* Hack -- Moving monsters can disturb the player */
            if (m_ptr->ml &&
                (disturb_move ||
                 (disturb_near && (m_ptr->cdis < 5)))) {
                disturb(0, 0);
            }


            /* Take or Kill "normal" objects on the floor */
            if (i_ptr->k_idx && (i_ptr->tval <= TV_MAX_OBJECT) &&
                ((r_ptr->rflags2 & RF2_TAKE_ITEM) ||
                 (r_ptr->rflags2 & RF2_KILL_ITEM))) {

                u32b flg3 = 0L;

                char m_name[80];
                char i_name[80];

                /* Check the grid */
                c_ptr = &cave[ny][nx];
                i_ptr = &i_list[c_ptr->i_idx];

                /* Acquire the object name */
                objdes(i_name, i_ptr, TRUE, 3);

                /* Acquire the monster name */
                monster_desc(m_name, m_ptr, 0x04);

                /* Analyze "monster hurting" flags on the object */
                if (wearable_p(i_ptr)) {

                    /* React to objects that hurt the monster */
                    if (i_ptr->flags1 & TR1_KILL_DRAGON) flg3 |= RF3_DRAGON;
                    if (i_ptr->flags1 & TR1_SLAY_DRAGON) flg3 |= RF3_DRAGON;
                    if (i_ptr->flags1 & TR1_SLAY_TROLL) flg3 |= RF3_TROLL;
                    if (i_ptr->flags1 & TR1_SLAY_GIANT) flg3 |= RF3_GIANT;
                    if (i_ptr->flags1 & TR1_SLAY_ORC) flg3 |= RF3_ORC;
                    if (i_ptr->flags1 & TR1_SLAY_DEMON) flg3 |= RF3_DEMON;
                    if (i_ptr->flags1 & TR1_SLAY_UNDEAD) flg3 |= RF3_UNDEAD;
                    if (i_ptr->flags1 & TR1_SLAY_ANIMAL) flg3 |= RF3_ANIMAL;
                    if (i_ptr->flags1 & TR1_SLAY_EVIL) flg3 |= RF3_EVIL;
                }

                /* The object cannot be picked up by the monster */
                if (artifact_p(i_ptr) || (r_ptr->rflags3 & flg3)) {

                    /* Only give a message for "take_item" */
                    if (r_ptr->rflags2 & RF2_TAKE_ITEM) {
                    
                        /* Take note */
                        did_take_item = TRUE;

                        /* Describe observable situations */
                        if (m_ptr->ml && player_has_los_bold(ny, nx)) {

                            /* Dump a message */
                            msg_format("%^s tries to pick up %s, but stops suddenly!",
                                       m_name, i_name);
                        }
                    }
                }
                
                /* Pick up the item */
                else if (r_ptr->rflags2 & RF2_TAKE_ITEM) {

                    /* Take note */
                    did_take_item = TRUE;

                    /* Describe observable situations */
                    if (player_has_los_bold(ny, nx)) {
                     
                        /* Dump a message */
                        msg_format("%^s picks up %s.", m_name, i_name);
                    }
                        
                    /* Delete the object */
                    delete_object(ny, nx);
                }

                /* Destroy the item */
                else {

                    /* Take note */
                    did_kill_item = TRUE;

                    /* Describe observable situations */
                    if (player_has_los_bold(ny, nx)) {

                        /* Dump a message */
                        msg_format("%^s crushes %s.", m_name, i_name);
                    }
                        
                    /* Delete the object */
                    delete_object(ny, nx);
                }
            }
        }
    }


    /* Notice changes in view */
    if (do_view) {

        /* Update some things */
        p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
    }


    /* Learn things from observable monster */
    if (m_ptr->ml) {

        monster_lore *l_ptr = &l_list[m_ptr->r_idx];

        /* Monster opened a door */
        if (did_open_door) l_ptr->flags2 |= RF2_OPEN_DOOR;

        /* Monster bashed a door */
        if (did_bash_door) l_ptr->flags2 |= RF2_BASH_DOOR;

        /* Monster tried to pick something up */
        if (did_take_item) l_ptr->flags2 |= RF2_TAKE_ITEM;

        /* Monster tried to crush something */
        if (did_kill_item) l_ptr->flags2 |= RF2_KILL_ITEM;

        /* Monster pushed past another monster */
        if (did_move_body) l_ptr->flags2 |= RF2_MOVE_BODY;

        /* Monster ate another monster */
        if (did_kill_body) l_ptr->flags2 |= RF2_KILL_BODY;

        /* Monster passed through a wall */
        if (did_pass_wall) l_ptr->flags2 |= RF2_PASS_WALL;

        /* Monster destroyed a wall */
        if (did_kill_wall) l_ptr->flags2 |= RF2_KILL_WALL;
    }


    /* Hack -- get "bold" if out of options */
    if (!do_turn && !do_move && m_ptr->monfear) {

        /* No longer afraid */
        m_ptr->monfear = 0;

        /* Message if seen */
        if (m_ptr->ml) {

            char m_name[80];

            /* Acquire the monster name */
            monster_desc(m_name, m_ptr, 0);

            /* Dump a message */
            msg_format("%^s turns to fight!", m_name);
        }
    }
}



/*
 * Hack -- get some random moves
 */
static void get_moves_random(int *mm)
{
    /* Try four random directions */
    mm[0] = mm[1] = mm[2] = mm[3] = 5;
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
        scatter(&y, &x, y1, x1, d, 0);

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
 * Move a critter about the dungeon			-RAK-
 *
 * Note that this routine is called ONLY from "process_monster()".
 */
static void mon_move(int m_idx)
{
    int			mm[8];

    monster_type	*m_ptr = &m_list[m_idx];
    monster_race	*r_ptr = &r_list[m_ptr->r_idx];
    monster_lore	*l_ptr = &l_list[m_ptr->r_idx];

    /* Get the monster location */
    int fx = m_ptr->fx;
    int fy = m_ptr->fy;

    int stagger;


    /* Assume no movement */
    mm[0] = mm[1] = mm[2] = mm[3] = 0;
    mm[4] = mm[5] = mm[6] = mm[7] = 0;


    /* Does the critter multiply?  Are creatures allowed to multiply? */
    /* Efficiency -- pre-empt multiplying if place_monster() will fail */
    /* Mega-Hack -- If the player is resting, only multiply occasionally */
    if ((r_ptr->rflags2 & RF2_MULTIPLY) &&
        (l_ptr->cur_num < l_ptr->max_num) &&
        (l_ptr->pkills < 30000) &&
        (!p_ptr->rest || (!rand_int(MON_MULT_ADJ))) ) {

        int k, y, x;
        
        /* Count the adjacent monsters */
        for (k = 0, y = fy - 1; y <= fy + 1; y++) {
            for (x = fx - 1; x <= fx + 1; x++) {
                if (cave[y][x].m_idx > 1) k++;
            }
        }

        /* Hack -- multiply slower in crowded rooms */
        if ((k < 4) && (!k || !rand_int(k * MON_MULT_ADJ))) {

            /* Try to multiply, take note if kid is visible */
            if (multiply_monster(m_idx)) {

                /* Take note if mom is visible too */
                if (m_ptr->ml) l_ptr->flags2 |= RF2_MULTIPLY;
            }
        }
    }


    /* Attempt to cast a spell */
    if (make_attack_spell(m_idx)) return;


    /* Extract the stagger values */
    stagger = 0;
    if (r_ptr->rflags1 & RF1_RAND_50) stagger += 50;
    if (r_ptr->rflags1 & RF1_RAND_25) stagger += 25;
    
    
    /* Confused -- 100% random */
    if (m_ptr->confused) {
        get_moves_random(mm);
    }

    /* 75% random movement */
    else if ((stagger == 75) && (rand_int(100) < 75)) {
        if (m_ptr->ml) l_ptr->flags1 |= RF1_RAND_50;
        if (m_ptr->ml) l_ptr->flags1 |= RF1_RAND_25;
        get_moves_random(mm);
    }

    /* 50% random movement */
    else if ((stagger == 50) && (rand_int(100) < 50)) {
        if (m_ptr->ml) l_ptr->flags1 |= RF1_RAND_50;
        get_moves_random(mm);
    }

    /* 25% random movement */
    else if ((stagger == 25) && (rand_int(100) < 25)) {
        if (m_ptr->ml) l_ptr->flags1 |= RF1_RAND_25;
        get_moves_random(mm);
    }

    /* Normal movement */
    else {
        get_moves(m_idx, mm);
    }

    /* Apply the movement */
    make_move(m_idx, mm);
}




/*
 * Heal the monsters (once per 200 game turns)
 * XXX XXX XXX Should probably be by monster turn.
 */
static void regen_monsters(void)
{
    int i, frac;

    /* Regenerate everyone */
    for (i = MIN_M_IDX; i < m_max; i++) {

        /* Check the i'th monster */
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_list[m_ptr->r_idx];
        
        /* Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Allow regeneration (if needed) */
        if (m_ptr->hp < m_ptr->maxhp) {

            /* Base regeneration */
            frac = m_ptr->maxhp / 50;

            /* Minimal regeneration rate */
            if (!frac) frac = 1;

            /* Some monsters regenerate quickly */
            if (r_ptr->rflags2 & RF2_REGENERATE) frac *= 2;

            /* Regenerate */
            m_ptr->hp += frac;

            /* Do not over-regenerate */
            if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

            /* Redraw (later) if needed */
            if (health_who == i) p_ptr->redraw |= (PR_HEALTH);
        }
    }
}


/*
 * Hack -- local "player stealth" calculation.
 * Passed "process_monsters()" to "process_monster()"
 */
static u32b noise = 0L;


/*
 * Process a monster (that is, give him a turn)
 * Note that only monsters within 100 grids are processed
 * This limits the "effective" range of "aaf" to 100.
 */
static void process_monster(int i)
{
    monster_type *m_ptr = &m_list[i];
    monster_race *r_ptr = &r_list[m_ptr->r_idx];

    monster_lore *l_ptr;

    int fx = m_ptr->fx;
    int fy = m_ptr->fy;

    bool test = FALSE;


    /* Monsters "sense" the player */
    if (m_ptr->cdis <= r_ptr->aaf) {

        /* We can "sense" the player */
        test = TRUE;
    }

    /* For efficiency, pretend line of sight is reflexive */
    else if (player_has_los_bold(fy, fx)) {

        /* We can "see" the player */
        test = TRUE;
    }

    /* Hack -- Aggravation "wakes up" nearby monsters (below) */
    else if (p_ptr->aggravate && (m_ptr->cdis <= MAX_SIGHT)) {

        /* We can "feel" the player */
        test = TRUE;
    }

#ifdef MONSTER_FLOW
    /* Hack -- Monsters can "smell" the player from far away */
    /* Note that most monsters have "aaf" of "20" or so */
    else if (flow_by_sound &&
            (cave[py][px].when == cave[fy][fx].when) &&
            (cave[fy][fx].cost < MONSTER_FLOW_DEPTH) &&
            (cave[fy][fx].cost < r_ptr->aaf)) {

        /* We can "smell" the player */
        test = TRUE;
    }
#endif

#ifdef WDT_TRACK_OPTIONS
    /* Hack -- Monsters can "follow" the player */
    else if (track_follow || track_target) {

        /* Re-acquire the target */
        get_target(i);

        /* We can "track" the player */
        test = TRUE;
    }
#endif


    /* Not allowed to do anything */	    	
    if (!test) return;


    /* Access the lore */
    l_ptr = &l_list[m_ptr->r_idx];


    /* Hack -- handle aggravation */
    if (p_ptr->aggravate) m_ptr->csleep = 0;

    /* Handle "sleep" */
    if (m_ptr->csleep) {

        /* Hack -- If the player is resting or paralyzed, then only */
        /* run this block about one turn in 50, since he is "quiet". */
        /* Hack -- nothing can hear the player from 50 grids away */
        if ((!p_ptr->rest && !p_ptr->paralysis) || (!rand_int(50))) {

            u32b notice = rand_int(1024);

            /* XXX See if monster "notices" player */
            if ((notice * notice * notice) <= noise) {

                /* Hack -- amount of "waking" */
                int d = (m_ptr->cdis < 50) ? (100 / m_ptr->cdis) : 1;

                /* Still asleep */
                if (m_ptr->csleep > d) {

                    /* Monster wakes up "a little bit" */
                    m_ptr->csleep -= d;

                    /* Notice the "not waking up" */
                    if (m_ptr->ml) {

                        /* Count the ignores */
                        if (l_ptr->ignore < MAX_UCHAR) l_ptr->ignore++;
                    }
                }

                /* Just woke up */
                else {

                    /* Reset sleep counter */
                    m_ptr->csleep = 0;

                    /* Notice the "waking up" */
                    if (m_ptr->ml) {

                        char m_name[80];

                        /* Acquire the monster name */
                        monster_desc(m_name, m_ptr, 0);

                        /* Dump a message */
                        msg_format("%^s wakes up.", m_name);

                        /* Count the wakings */
                        if (l_ptr->wake < MAX_UCHAR) l_ptr->wake++;
                    }
                }
            }
        }

        /* Still sleeping */
        if (m_ptr->csleep) return;
    }


    /* Handle "stun" */
    if (m_ptr->stunned) {

        /* Recover a little bit */
        m_ptr->stunned--;

        /* Make a "saving throw" against stun */
        if (rand_int(5000) <= r_ptr->level * r_ptr->level) {
            m_ptr->stunned = 0;
        }

        /* Hack -- Recover from stun */
        if (!m_ptr->stunned) {

            /* Message if visible */
            if (m_ptr->ml) {

                char m_name[80];

                /* Acquire the monster name */
                monster_desc(m_name, m_ptr, 0);

                /* Dump a message */
                msg_format("%^s is no longer stunned.", m_name);
            }
        }

        /* Still stunned */
        if (m_ptr->stunned) return;
    }


    /* Handle confusion */
    if (m_ptr->confused) {

        /* Amount of "boldness" */
        int d = randint(1 + r_ptr->level / 10);

        /* Still confused */
        if (m_ptr->confused > d) {

            /* Reduce the confusion */
            m_ptr->confused -= d;
        }

        /* Recovered */
        else {

            /* No longer confused */
            m_ptr->confused = 0;

            /* Message if visible */
            if (m_ptr->ml) {

                char m_name[80];

                /* Acquire the monster name */
                monster_desc(m_name, m_ptr, 0);

                /* Dump a message */
                msg_format("%^s is no longer confused.", m_name);
            }
        }
    }


    /* Handle "fear" */
    if (m_ptr->monfear) {

        /* Amount of "boldness" */
        int d = randint(1 + r_ptr->level / 10);

        /* Still afraid */
        if (m_ptr->monfear > d) {

            /* Reduce the fear */
            m_ptr->monfear -= d;
        }

        /* Recover from fear, take note if seen */
        else {

            /* No longer afraid */
            m_ptr->monfear = 0;

            /* Visual note */
            if (m_ptr->ml) {

                char m_name[80];
                char m_poss[80];

                /* Acquire the monster name/poss */
                monster_desc(m_name, m_ptr, 0);
                monster_desc(m_poss, m_ptr, 0x22);

                /* Dump a message */
                msg_format("%^s recovers %s courage.", m_name, m_poss);
            }
        }
    }


    /* Let it move */
    mon_move(i);
}


/*
 * Process all the monsters.
 *
 * Give monsters some energy, and when allowed, try waking up, moving,
 * attacking, etc.  We also remove "dead" monsters here.
 *
 * Note that a monster can ONLY move in the monster array when someone
 * calls "remove_monster_idx()".  And (basically) the only function that
 * does that is this one, and we only do it for monsters that are dead,
 * inducing the "movement" of a monster that was already processed into
 * a slot which will not be processed again until next time.
 *
 * Poly-morph is extremely dangerous, cause it changes the monster race.
 * Actually, it is done by killing the original monster and creating a
 * new monster, so we should be okay, except that the "dead" monster may
 * not be removed until the next time through the loop.
 *
 * Note that this routine ASSUMES that "update_monsters()" has been called
 * every time the player changes his view or lite, and that "update_mon()"
 * has been, and will be, called every time a monster moves.  Likewise, we
 * rely on the lite/view/etc being "verified" EVERY time a door/wall appears
 * or disappears.  This will then require minimal scans of the "monster" array.
 *
 * It is very important to process the array "backwards", as "newly created"
 * monsters should not get a turn until the next round of processing.  Else
 * there is a risk of multiple summons breaking the machine.  As it is, there
 * once the monster array fills up, only the first few "summons" per round
 * will succeed, but luckily, this will (on average) require about 5 * 20
 * or 100 summons during one player turn.
 *
 * Note that monster race zero is used to hold the "empty" monster race.
 *
 * Note also the new "speed code" which allows "better" distribution of events.
 *
 * There is a (hackish) "speed boost" of rand_int(5) energy units applied each
 * time the monster attempt to move.  This means that fast monsters get more
 * "boosts".  On average, for a normal monster, this produces one "extra" move
 * every 50 moves, barely noticable, but also provides some random spread to
 * the monster clumpings.  Calling "random()" so often is very expensive.
 * Thus, this code must be explicitly chosen at compile time.
 *
 * This function is responsible for at least half of the processor time
 * on a normal system with a "normal" amount of monsters and a player doing
 * normal things.  Most of this time is in process_monster() and make_move().
 *
 * Most of the rest of the time is spent in "update_view()" and "lite_spot()".
 */
void process_monsters(void)
{
    int		i, e;


    /* Calculate the "noisiness" of the player */
    noise = (1L << (30 - p_ptr->skill_stl));


    /* Require some free records */
    if (MIN_M_IDX + m_cnt + 30 > MAX_M_IDX) {

        /* Compact some monsters */
        compact_monsters(30);

        /* Hack -- Remove dead monsters (backwards!) */
        for (i = m_max - 1; i >= MIN_M_IDX; i--) {

            /* Get the i'th monster */
            monster_type *m_ptr = &m_list[i];

            /* Hack -- Remove dead monsters. */
            if (!m_ptr->r_idx) remove_monster_idx(i);
        }
    }


    /* Check for creature generation */
    if (!(turn % 10) && (rand_int(MAX_M_ALLOC_CHANCE) == 0)) {
        (void)alloc_monster(MAX_SIGHT + 5, FALSE);
    }

    /* XXX XXX XXX XXX Check for creature regeneration */
    if (!(turn % 200)) regen_monsters();


    /* Process the monsters (backwards!) */
    for (i = m_max - 1; i >= MIN_M_IDX; i--) {


        /* Get the i'th monster */
        monster_type *m_ptr = &m_list[i];


        /* Excise dead monsters. */
        if (!m_ptr->r_idx) {

            /* Remove the monster */
            remove_monster_idx(i);

            /* Do NOT process it */
            continue;
        }


        /* Obtain the energy boost */
        e = extract_energy[m_ptr->mspeed];

#if 0
#ifdef WDT_TRACK_OPTIONS
        /* Reduce energy boost if far away and out of sight */
        if (track_follow &&
            (m_ptr->cdis > 20) &&
            !player_has_los_bold(m_ptr->fy, m_ptr->fx)) {

            /* Reduce the energy boost */
            e >>= 2;
        }
#endif
#endif

        /* Give this monster some energy */
        m_ptr->energy += e;

        /* Not enough energy to move */
        if (m_ptr->energy < 100) continue;

#ifdef RANDOM_BOOST
        /* Hack -- small "energy boost" (see "dungeon.c") */
        m_ptr->energy += rand_int(5);
#endif

        /* Use up "some" energy */
        m_ptr->energy -= 100;


        /* Process "nearby" monsters */
        if ((m_ptr->cdis < 100) || track_follow) process_monster(i);


        /* Hack -- notice player death or departure */
        if (death || new_level_flag) break;
    }


    /* Require some free records */
    if (MIN_M_IDX + m_max + 30 > MAX_M_IDX) {

        /* Compact some monsters */
        compact_monsters(30);

        /* Hack -- Remove dead monsters (backwards!) */
        for (i = m_max - 1; i >= MIN_M_IDX; i--) {

            /* Get the i'th monster */
            monster_type *m_ptr = &m_list[i];

            /* Hack -- Remove dead monsters. */
            if (!m_ptr->r_idx) remove_monster_idx(i);
        }
    }


    /* Handle stuff */
    handle_stuff();
}


