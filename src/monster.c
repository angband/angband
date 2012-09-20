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
 * Hack -- reorder the monster list for efficiency
 */
void reorder_monsters(void)
{
    int i;

    /* Excise dead monsters (backwards!) */
    for (i = m_max - 1; i >= 1; i--) {

        /* Get the i'th object */
        monster_type *m_ptr = &m_list[i];

        /* Skip real monsters */
        if (m_ptr->r_idx) continue;

        /* One less monster */
        m_max--;

        /* Reorder */
        if (i != m_max) {

            int ny = m_list[m_max].fy;
            int nx = m_list[m_max].fx;

            /* Update the cave */
            cave[ny][nx].m_idx = i;

            /* Hack -- Update the target */
            if (target_who == (int)(m_max)) target_who = i;

            /* Hack -- Update the health bar */
            if (health_who == (int)(m_max)) health_track(i);

            /* Structure copy */
            m_list[i] = m_list[m_max];

            /* Wipe the hole */
            WIPE(&m_list[m_max], monster_type);
        }
    }

    /* Hack -- reset "m_nxt" */
    m_nxt = m_max;
}


/*
 * Delete a monster by index.
 *
 * This function causes the given monster to cease to exist for
 * all intents and purposes.  The monster record is left in place
 * but the record is wiped, marking it as "dead" (no race index)
 * so that it can be "skipped" when scanning the monster array.
 *
 * Thus, anyone who makes direct reference to the "m_list[]" array
 * using monster indexes that may have become invalid should be sure
 * to verify that the "r_idx" field is non-zero.  All references
 * to "m_list[c_ptr->m_idx]" are guaranteed to be valid, see below.
 */
void delete_monster_idx(int i)
{
    int x, y;
    
    monster_type *m_ptr = &m_list[i];

    monster_race *r_ptr = &r_info[m_ptr->r_idx];


    /* Get location */
    y = m_ptr->fy;
    x = m_ptr->fx;


    /* Hack -- Reduce the racial counter */
    r_ptr->cur_num--;

    /* Hack -- count the number of "reproducers" */
    if (r_ptr->flags2 & RF2_MULTIPLY) num_repro--;


    /* Hack -- remove target monster */
    if (i == target_who) target_who = 0;

    /* Hack -- remove tracked monster */
    if (i == health_who) health_track(0);


    /* Monster is gone */
    cave[y][x].m_idx = 0;


    /* Visual update */
    lite_spot(y, x);


    /* Wipe the Monster */
    WIPE(m_ptr, monster_type);


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
    if (c_ptr->m_idx) delete_monster_idx(c_ptr->m_idx);
}


/*
 * Compact the monster list because it is getting too full.
 *
 * This function is very dangerous, use with extreme caution!
 *
 * XXX Base the saving throw on a combination of monster level,
 * distance from player, and current "desperation".
 */
void compact_monsters(int size)
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
        for (i = 1; i < m_max; i++) {

            monster_type *m_ptr = &m_list[i];

            monster_race *r_ptr = &r_info[m_ptr->r_idx];

            /* Paranoia -- skip "dead" monsters */
            if (!m_ptr->r_idx) continue;

            /* Hack -- High level monsters start out "immune" */
            if (r_ptr->level > cur_lev) continue;

            /* Ignore nearby monsters */
            if ((cur_dis > 0) && (m_ptr->cdis < cur_dis)) continue;

            /* Saving throw chance */
            chance = 90;

            /* Only compact "Quest" Monsters in emergencies */
            if ((r_ptr->flags1 & RF1_QUESTOR) && (cnt < 1000)) chance = 100;

            /* Try not to compact Unique Monsters */
            if (r_ptr->flags1 & RF1_UNIQUE) chance = 99;

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
 * Delete/Remove all the monsters when the player leaves the level
 *
 * This is an efficient method of simulating multiple calls to the
 * "delete_monster()" function, with no visual effects.
 */
void wipe_m_list()
{
    int i;

    /* Delete all the monsters */
    for (i = m_max - 1; i >= 1; i--) {

        monster_type *m_ptr = &m_list[i];

        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        /* Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Mega-Hack -- preserve Unique's XXX XXX XXX */

        /* Hack -- Reduce the racial counter */
        r_ptr->cur_num--;

        /* Monster is gone */
        cave[m_ptr->fy][m_ptr->fx].m_idx = 0;

        /* Wipe the Monster */
        WIPE(m_ptr, monster_type);
    }

    /* Reset the monster array */
    m_nxt = m_max = 1;

    /* No more monsters */
    m_cnt = 0;


    /* Hack -- reset "reproducer" count */
    num_repro = 0;

    /* Hack -- no more target */
    target_who = 0;

    /* Hack -- no more tracking */
    health_track(0);
}


/*
 * Acquires and returns the index of a "free" monster.
 *
 * This routine should almost never fail, but it *can* happen.
 */
s16b m_pop(void)
{
    int i, n;

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

    /* Check for some space */
    for (n = 1; n < MAX_M_IDX; n++) {

        /* Get next space */
        i = m_nxt;

        /* Advance (and wrap) the "next" pointer */
        if (++m_nxt >= MAX_M_IDX) m_nxt = 1;

        /* Skip monsters in use */
        if (m_list[i].r_idx) continue;

        /* Count the monsters */
        m_cnt++;

        /* Use this monster */
        return (i);
    }

    /* XXX XXX XXX Warning */
    if (character_dungeon) msg_print("Too many monsters!");

    /* Try not to crash */
    return (0);
}





/*
 * Choose a monster race that seems "appropriate" to the given level
 *
 * This function uses the "allocation table" built in "init.c".
 *
 * Note that "town" monsters will *only* be created in the town,
 * and "normal" monsters will *never* be created in the town.
 *
 * There is a small chance (1/50) of "boosting" the given depth by
 * a small amount (up to four levels), except in the town.
 *
 * It is (slightly) more likely to acquire a monster of the given level
 * than one of a lower level.  This is done by choosing several monsters
 * appropriate to the given level and keeping the "hardest" one.
 *
 * Note that we only pick 10000 different monsters before failing.
 * This may prevent "summon unique monsters" from crashing.
 *
 * As far as I can tell, this routine will never return a monster
 * which cannot be placed in the dungeon at the current time.  But
 * if we are given an overly restrictive hook, we may not terminate.
 *
 * Thus the "place_monster()" routine should never fail unless it is
 * given an illegal location, or an insufficient level, or an overly
 * restrictive hook.
 */
s16b get_mon_num(int level)
{
    int		i, p, k;

    int		r_idx;

    monster_race *r_ptr;

    /* Obtain the table */
    s16b *t_lev = alloc_race_index;
    race_entry *table = alloc_race_table;


    /* Hack -- sometimes "boost" level */
    if (level > 0) {
    
        /* Occasionally Make a Nasty Monster */
        if (rand_int(NASTY_MON) == 0) {

            /* Pick a level bonus */
            int d = level / 4 + 2;

            /* Boost the level */
            level += ((d < 5) ? d : 5);
        }

        /* Only examine legal levels */
        if (level > MAX_DEPTH - 1) level = MAX_DEPTH - 1;
    }
    

    /* Hack -- Pick a monster */
    for (k = 0; k < 10000; k++) {

        /* Town level is easy */
        if (level <= 0) {

            /* Pick a level 0 entry */
            i = rand_int(t_lev[0]);
        }

        /* Other levels */
        else {

            /* Roll for rerolls */
            p = rand_int(100);
            
            /* Pick any "appropriate" monster */
            i = rand_int(t_lev[level]);

            /* Try for a "harder" monster twice (10%) */
            if (p < 10) {

                /* Pick another monster at or below the given level */
                int j = rand_int(t_lev[level]);

                /* Keep it if it is "better" */
                if (table[i].locale < table[j].locale) i = j;
            }

            /* Try for a "harder" monster once (50%) */
            if (p < 60) {

                /* Pick another monster at or below the given level */
                int j = rand_int(t_lev[level]);

                /* Keep it if it is "better" */
                if (table[i].locale < table[j].locale) i = j;
            }

            /* Hack -- Never make town monsters */
            if (table[i].locale == 0) continue;
        }


        /* Access the "r_idx" of the chosen monster */
        r_idx = table[i].r_idx;


        /* Hack -- apply the hook (if needed) */
        if (get_mon_num_hook && (!(*get_mon_num_hook)(r_idx))) continue;
        
        
        /* Access the actual race */
        r_ptr = &r_info[r_idx];


        /* Hack -- "unique" monsters must be "unique" */
        if ((r_ptr->flags1 & RF1_UNIQUE) && (r_ptr->cur_num >= r_ptr->max_num)) {
            continue;
        } 


        /* Depth Monsters never appear out of depth */
        if ((r_ptr->flags1 & RF1_FORCE_DEPTH) && (r_ptr->level > dun_level)) {
            continue;
        }


        /* Hack -- Roll for "rarity" */
        if (rand_int(table[i].chance) != 0) continue;


        /* Use that monster */
        return (r_idx);
    }


    /* Oops */
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
 * If no r_ptr arg is given, it is extracted from m_ptr and r_info
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
    cptr		res;

    monster_race	*r_ptr = &r_info[m_ptr->r_idx];

    cptr		name = (r_name + r_ptr->name);

    bool		seen, pron;


    /* Can we "see" it (exists + forced, or visible + not unforced) */
    seen = (m_ptr && ((mode & 0x80) || (!(mode & 0x40) && m_ptr->ml)));

    /* Sexed Pronouns (seen and allowed, or unseen and allowed) */
    pron = (m_ptr && ((seen && (mode & 0x20)) || (!seen && (mode & 0x10))));


    /* First, try using pronouns, or describing hidden monsters */
    if (!seen || pron) {

        /* an encoding of the monster "sex" */
        int kind = 0x00;

        /* Extract the gender (if applicable) */
        if (r_ptr->flags1 & RF1_FEMALE) kind = 0x20;
        else if (r_ptr->flags1 & RF1_MALE) kind = 0x10;

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
        if (r_ptr->flags1 & RF1_FEMALE) strcpy(desc, "herself");
        else if (r_ptr->flags1 & RF1_MALE) strcpy(desc, "himself");
        else strcpy(desc, "itself");
    }


    /* Handle all other visible monster requests */
    else {

        /* It could be a Unique */
        if (r_ptr->flags1 & RF1_UNIQUE) {

            /* Start with the name (thus nominative and objective) */
            (void)strcpy(desc, name);
        }

        /* It could be an indefinite monster */
        else if (mode & 0x08) {

            /* XXX Check plurality for "some" */

            /* Indefinite monsters need an indefinite article */
            (void)strcpy(desc, is_a_vowel(name[0]) ? "an " : "a ");
            (void)strcat(desc, name);
        }

        /* It could be a normal, definite, monster */
        else {

            /* Definite monsters need a definite article */
            (void)strcpy(desc, "the ");
            (void)strcat(desc, name);
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
 * Learn about a monster (by "probing" it)
 */
void lore_do_probe(int m_idx)
{
    monster_type *m_ptr = &m_list[m_idx];

    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    /* Hack -- Memorize some flags */
    r_ptr->r_flags1 = r_ptr->flags1;
    r_ptr->r_flags2 = r_ptr->flags2;
    r_ptr->r_flags3 = r_ptr->flags3;

    /* Redraw the recall window */
    p_ptr->redraw |= (PR_RECENT);
}


/*
 * Take note that the given monster just dropped some treasure
 *
 * Note that learning the "GOOD"/"GREAT" flags gives information
 * about the treasure (even when the monster is killed for the first
 * time, such as uniques, and the treasure has not been examined yet).
 *
 * This "indirect" method is used to prevent the player from learning
 * exactly how much treasure a monster can drop from observing only
 * a single example of a drop.  This method actually observes how much
 * gold and items are dropped, and remembers that information to be
 * described later by the monster recall code.
 */
void lore_treasure(int m_idx, int num_item, int num_gold)
{
    monster_type *m_ptr = &m_list[m_idx];

    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    /* Note the number of things dropped */
    if (num_item > r_ptr->r_drop_item) r_ptr->r_drop_item = num_item;
    if (num_gold > r_ptr->r_drop_gold) r_ptr->r_drop_gold = num_gold;

    /* Hack -- memorize the good/great flags */
    if (r_ptr->flags1 & RF1_DROP_GOOD) r_ptr->r_flags1 |= RF1_DROP_GOOD;
    if (r_ptr->flags1 & RF1_DROP_GREAT) r_ptr->r_flags1 |= RF1_DROP_GREAT;
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
 *
 * Note the optimized inline use of the "distance()" function.
 */
void update_mon(int m_idx, bool dist)
{
    monster_type *m_ptr = &m_list[m_idx];

    monster_race *r_ptr = &r_info[m_ptr->r_idx];

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
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        /* Telepathy can see all "nearby" monsters with "minds" */
        if (p_ptr->telepathy && (m_ptr->cdis <= MAX_SIGHT)) {

            /* Empty mind, no telepathy */
            if (r_ptr->flags2 & RF2_EMPTY_MIND) {
                do_empty_mind = TRUE;
            }

            /* Weird mind, occasional telepathy */
            else if (r_ptr->flags2 & RF2_WEIRD_MIND) {
                do_weird_mind = TRUE;
                if (rand_int(100) < 10) do_telepathy = TRUE;
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
                if (r_ptr->flags2 & RF2_SMART) r_ptr->r_flags2 |= RF2_SMART;
                if (r_ptr->flags2 & RF2_STUPID) r_ptr->r_flags2 |= RF2_STUPID;
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
                if (r_ptr->flags2 & RF2_COLD_BLOOD) do_cold_blood = TRUE;

                /* Infravision works */
                if (!do_cold_blood) flag = TRUE;
            }

            /* Check for "illumination" of the monster grid */
            if (player_can_see_bold(fy, fx)) {

                /* Take note of invisibility */
                if (r_ptr->flags2 & RF2_INVISIBLE) do_invisible = TRUE;

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

            /* Hack -- Count "fresh" sightings */
            if (r_ptr->r_sights < MAX_SHORT) r_ptr->r_sights++;

            /* Appearing monsters can disturb the player */
            if (disturb_enter) disturb(1, 0);
        }

        /* Memorize various observable flags */
        if (do_empty_mind) r_ptr->r_flags2 |= RF2_EMPTY_MIND;
        if (do_weird_mind) r_ptr->r_flags2 |= RF2_WEIRD_MIND;
        if (do_cold_blood) r_ptr->r_flags2 |= RF2_COLD_BLOOD;
        if (do_invisible) r_ptr->r_flags2 |= RF2_INVISIBLE;

        /* Efficiency -- Notice multi-hued monsters */
        if (r_ptr->flags1 & RF1_ATTR_MULTI) scan_monsters = TRUE;
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
    for (i = 1; i < m_max; i++) {

        monster_type *m_ptr = &m_list[i];

        /* Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Update the monster */
        update_mon(i, dist);
    }
}




/*
 * Attempt to place a monster of the given race at the given location.
 *
 * To give the player a sporting chance, any monster that appears in
 * line-of-sight and is extremely dangerous can be marked as
 * "FORCE_SLEEP", which will cause them to be placed with low energy,
 * which often (but not always) lets the player move before they do.
 *
 * This routine refuses to place out-of-depth "FORCE_DEPTH" monsters.
 *
 * XXX XXX XXX Use special "here" and "dead" flags for unique monsters,
 * remove old "cur_num" and "max_num" fields.  XXX XXX XXX XXX
 *
 * XXX XXX XXX Actually, do something similar for artifacts, to simplify
 * the "preserve" mode, and to make the "what artifacts" flag more useful.
 */
static bool place_monster_one(int y, int x, int r_idx, bool slp)
{
    int			i;

    cave_type		*c_ptr;

    monster_type	*m_ptr;

    monster_race	*r_ptr = &r_info[r_idx];

    cptr		name = (r_name + r_ptr->name);


    /* Verify location */
    if (!in_bounds(y,x)) return (FALSE);

    /* Require empty space */
    if (!empty_grid_bold(y, x)) return (FALSE);

    /* Hack -- no creation on glyph of warding */
    if ((cave[y][x].feat & 0x3F) == 0x03) return (FALSE);
        

    /* Paranoia */
    if (!r_ptr->name) return (FALSE);


    /* Hack -- "unique" monsters must be "unique" */
    if ((r_ptr->flags1 & RF1_UNIQUE) && (r_ptr->cur_num >= r_ptr->max_num)) {

        /* Cannot create */
        return (FALSE);
    }


    /* Depth monsters may NOT be created out of depth */
    if ((r_ptr->flags1 & RF1_FORCE_DEPTH) && (dun_level < r_ptr->level)) {

        /* Cannot create */
        return (FALSE);
    }


    /* Powerful monster */
    if (r_ptr->level > dun_level) {

        /* Uniques get rating based on "out of depth" amount */
        if (r_ptr->flags1 & RF1_UNIQUE) {
            if (cheat_hear) msg_format("Deep Unique (%s).", name);
            rating += (r_ptr->level - dun_level);
        }

        /* Normal monsters are worth "half" as much */
        else {
            if (cheat_hear) msg_format("Deep Monster (%s).", name);
            rating += (r_ptr->level - dun_level) / 2;
        }
    }

    /* Note the monster */
    else if (r_ptr->flags1 & RF1_UNIQUE) {

        /* Unique monsters induce message */
        if (cheat_hear) msg_format("Unique (%s).", name);
    }


    /* Access the location */
    c_ptr = &cave[y][x];

    /* Make a new monster */
    c_ptr->m_idx = m_pop();

    /* Mega-Hack -- catch "failure" */
    if (!c_ptr->m_idx) return (FALSE);


    /* Get a new monster record */
    m_ptr = &m_list[c_ptr->m_idx];

    /* Save the race */
    m_ptr->r_idx = r_idx;

    /* Place the monster at the location */
    m_ptr->fy = y;
    m_ptr->fx = x;


    /* Hack -- Count the monsters on the level */
    r_ptr->cur_num++;


    /* Hack -- count the number of "reproducers" */
    if (r_ptr->flags2 & RF2_MULTIPLY) num_repro++;


    /* Assign maximal hitpoints */
    if (r_ptr->flags1 & RF1_FORCE_MAXHP) {
        m_ptr->maxhp = maxroll(r_ptr->hdice, r_ptr->hside);
    }
    else {
        m_ptr->maxhp = damroll(r_ptr->hdice, r_ptr->hside);
    }

    /* And start out fully healthy */
    m_ptr->hp = m_ptr->maxhp;


    /* Extract the monster base speed */
    m_ptr->mspeed = r_ptr->speed;

    /* Hack -- small racial variety */
    if (!(r_ptr->flags1 & RF1_UNIQUE)) {
    
        /* Allow some small variation per monster */
        i = extract_energy[r_ptr->speed] / 10;
        if (i) m_ptr->mspeed += rand_spread(0, i);
    }


    /* Give a random starting energy */
    m_ptr->energy = rand_int(100);

    /* Hack -- Reduce risk of "instant death by breath weapons" */
    if (r_ptr->flags1 & RF1_FORCE_SLEEP) {

        /* Start out with minimal energy */
        m_ptr->energy = rand_int(10);
    }


    /* No "damage" yet */
    m_ptr->stunned = 0;
    m_ptr->confused = 0;
    m_ptr->monfear = 0;
    m_ptr->xtra = 0;

    /* Update the monster (correctly) */
    m_ptr->ml = FALSE;
    update_mon(c_ptr->m_idx, TRUE);


    /* Assume no sleeping */
    m_ptr->csleep = 0;
    
    /* Enforce sleeping if needed */
    if (slp && r_ptr->sleep) {
        int val = r_ptr->sleep;
        m_ptr->csleep = ((val * 2) + randint(val * 10));
    }


    /* Success */
    return (TRUE);
}


/*
 * Maximum size of a group of monsters
 */
#define GROUP_MAX	32


/*
 * Attempt to place a "group" of monsters around the given location
 */
static bool place_monster_group(int y, int x, int r_idx, bool slp)
{
    monster_race *r_ptr = &r_info[r_idx];

    int old, n, i;
    int total = 0, extra = 0;

    int hack_n = 0;

    byte hack_y[GROUP_MAX];
    byte hack_x[GROUP_MAX];


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

            int mx = hx + ddx_ddd[i];
            int my = hy + ddy_ddd[i];

            /* Walls and Monsters block flow */
            if (!empty_grid_bold(my, mx)) continue;

            /* Attempt to place another monster */
            if (place_monster_one(my, mx, r_idx, slp)) {

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
 * Hack -- help pick an escort type
 */
static int place_monster_idx = 0;

/*
 * Hack -- help pick an escort type
 */
static bool place_monster_okay(int r_idx)
{
    monster_race *r_ptr = &r_info[place_monster_idx];

    monster_race *z_ptr = &r_info[r_idx];
    
    /* Require similar "race" */
    if (z_ptr->r_char != r_ptr->r_char) return (FALSE);

    /* Skip more advanced monsters */
    if (z_ptr->level > r_ptr->level) return (FALSE);

    /* Skip unique monsters */
    if (z_ptr->flags1 & RF1_UNIQUE) return (FALSE);

    /* Paranoia -- Skip identical monsters */
    if (place_monster_idx == r_idx) return (FALSE);

    /* Okay */
    return (TRUE);
}


/*
 * Attempt to place a monster of the given rave at the given location
 *
 * Note that certain monsters are now marked as requiring "friends".
 * These monsters, if successfully placed, and if the "grp" parameter
 * is TRUE, will be surrounded by a "group" of identical monsters.
 *
 * Note that certain monsters are now marked as requiring an "escort",
 * which is a collection of monsters with similar "race" but lower level.
 *
 * Some monsters induce a fake "group" flag on their escorts.
 *
 * Note the "bizarre" use of non-recursion to prevent annoying output
 * when running a code profiler.
 */
bool place_monster_aux(int y, int x, int r_idx, bool slp, bool grp)
{
    int			i;

    monster_race	*r_ptr = &r_info[r_idx];


    /* Place one monster, or fail */
    if (!place_monster_one(y, x, r_idx, slp)) return (FALSE);


    /* Require the "group" flag */
    if (!grp) return (TRUE);


    /* Friends for certain monsters */
    if (r_ptr->flags1 & RF1_FRIENDS) {

        /* Attempt to place a group */
        place_monster_group(y, x, r_idx, slp);
    }
    

    /* Escorts for certain monsters */
    if (r_ptr->flags1 & RF1_ESCORT) {

        /* Try to place several "escorts" */
        for (i = 0; i < 50; i++) {

            int nx, ny, z, d = 3;

            /* Pick a location */
            scatter(&ny, &nx, y, x, d, 0);

            /* Require empty grids */
            if (!empty_grid_bold(ny, nx)) continue;
                        
            /* Set the escort index */
            place_monster_idx = r_idx;

            /* Set the escort hook */
            get_mon_num_hook = place_monster_okay;

            /* Pick a random race */
            z = get_mon_num(r_ptr->level);

            /* Remove restriction */
            get_mon_num_hook = NULL;

            /* Place an escort (allow groups) */
            place_monster_one(ny, nx, z, slp);

            /* Friends for certain monsters */
            if ((r_info[z].flags1 & RF1_FRIENDS) ||
                (r_ptr->flags1 & RF1_ESCORTS)) {
                place_monster_group(ny, nx, z, slp);
            }
        }
    }


    /* Success */
    return (TRUE);
}


/*
 * Hack -- attempt to place a monster at the given location
 *
 * Attempt to find a monster appropriate to the "monster_level"
 */
bool place_monster(int y, int x, bool slp, bool grp)
{
    int r_idx;
    
    /* Pick a monster */
    r_idx = get_mon_num(monster_level);
    
    /* Attempt to place the monster */
    if (place_monster_aux(y, x, r_idx, slp, grp)) return (TRUE);
    
    /* Oops */
    return (FALSE);
}




/*
 * XXX XXX XXX Player Ghosts are such a hack XXX XXX XXX
 *
 * Angband 2.8.0 will simplify this situation by creating a small number
 * of "unique" monsters which will serve as the "player ghosts".  Each
 * will have a place holder for the "name" of a deceased player, which
 * will be extracted from a "bone" file, or replaced with a "default"
 * name if a real name is not available.  Each ghost will appear exactly
 * once and will not induce a special feeling.  The code below attempts
 * to simulate the expected player ghost templates, more or less.
 */



/*
 * Set a "blow" record for the ghost
 */
static void ghost_blow(int i, int m, int e, int d, int s)
{
    monster_race *r_ptr = &r_info[MAX_R_IDX-1];

    /* Save the data */
    r_ptr->blow[i].method = m;
    r_ptr->blow[i].effect = e;
    r_ptr->blow[i].d_dice = d;
    r_ptr->blow[i].d_side = s;
}



/*
 * XXX XXX XXX Prepare the "ghost" race
 *
 * This will change completely for Angband 2.8.0
 *
 * We are given a "name" of a dead player, and a race/class (by index)
 * which we should probably ignore, or at most verify, and a dungeon
 * level (the current level).
 *
 * Current methods:
 *   (s) 1 Skeleton
 *   (z) 1 Zombie
 *   (M) 1 Mummy
 *   (G) 1 Polterguiest, 1 Spirit, 1 Ghost, 1 Shadow, 1 Phantom
 *   (W) 1 Wraith
 *   (V) 1 Vampire, 1 Vampire Lord
 *   (L) 1 Lich
 *
 * Possible change: Lose 1 ghost, Add "Master Lich"
 *
 * Possible change: Lose 2 ghosts, Add "Wraith", Add "Master Lich"
 *
 * Possible change: Lose 4 ghosts, lose 1 vampire lord
 */
static void set_ghost(cptr pn)
{
    int i;

    monster_race *r_ptr = &r_info[MAX_R_IDX-1];

    char *name = (r_name + r_ptr->name);

    char gb_name[32];


    /* Extract the basic ghost name */
    strcpy(gb_name, pn);

    /* Find the first comma, or end of string */
    for (i = 0; (i < 16) && (gb_name[i]) && (gb_name[i] != ','); i++) ;

    /* Terminate the name */
    gb_name[i] = '\0';

    /* Force a name */
    if (!gb_name[0]) strcpy(gb_name, "Nobody");

    /* Capitalize the name */
    if (islower(gb_name[0])) gb_name[0] = toupper(gb_name[0]);


    /* Clear the normal flags */
    r_ptr->flags1 = r_ptr->flags2 = r_ptr->flags3 = 0L;

    /* Clear the spell flags */
    r_ptr->flags4 = r_ptr->flags5 = r_ptr->flags6 = 0L;


    /* Clear the attacks */
    ghost_blow(0, 0, 0, 0, 0);
    ghost_blow(1, 0, 0, 0, 0);
    ghost_blow(2, 0, 0, 0, 0);
    ghost_blow(3, 0, 0, 0, 0);


    /* The ghost never sleeps */
    r_ptr->sleep = 0;

    /* The ghost is very attentive */
    r_ptr->aaf = 100;


    /* Save the level */
    r_ptr->level = dun_level;


    /* The ghost is unique with maximized hitpoints */
    r_ptr->flags1 |= (RF1_UNIQUE | RF1_FORCE_MAXHP);

    /* The ghost only drops good items */
    r_ptr->flags1 |= (RF1_ONLY_ITEM | RF1_DROP_GOOD);

    /* The ghost is cold blooded */
    r_ptr->flags2 |= (RF2_COLD_BLOOD);

    /* The ghost is evil and undead and poison immune */
    r_ptr->flags3 |= (RF3_EVIL | RF3_UNDEAD | RF3_IM_POIS);

    /* Cannot be slept or confused or scared */
    r_ptr->flags3 |= (RF3_NO_SLEEP | RF3_NO_CONF | RF3_NO_FEAR);


    /* Make a ghost */
    switch ((dun_level / 4) + randint(3)) {

      case 1:
      case 2:
      case 3:
        sprintf(name, "%s, the Skeleton", gb_name);
        r_ptr->r_char = 's';
        r_ptr->r_attr = TERM_WHITE;
        r_ptr->flags2 |= (RF2_OPEN_DOOR | RF2_BASH_DOOR);
        r_ptr->flags3 |= (RF3_IM_COLD);
        r_ptr->ac = 26;
        r_ptr->speed = 110;
        r_ptr->hdice = 5;
        r_ptr->hside = 10;
        r_ptr->mexp = dun_level * 5 + 5;
        ghost_blow(0, RBM_HIT, RBE_HURT, 1, 6);
        ghost_blow(1, RBM_HIT, RBE_HURT, 1, 6);
        break;

      case 4:
      case 5:
        sprintf(name, "%s, the Zombie", gb_name);
        r_ptr->r_char = 'z';
        r_ptr->r_attr = TERM_SLATE;
        r_ptr->flags1 |= (RF1_DROP_60 | RF1_DROP_90);
        r_ptr->flags2 |= (RF2_OPEN_DOOR | RF2_BASH_DOOR);
        r_ptr->flags3 |= (RF3_IM_COLD);
        r_ptr->ac = 30;
        r_ptr->speed = 110;
        r_ptr->hdice = 10;
        r_ptr->hside = 10;
        r_ptr->mexp = dun_level * 5 + 5;
        ghost_blow(0, RBM_HIT, RBE_HURT, 1, 8);
        ghost_blow(0, RBM_HIT, RBE_HURT, 1, 8);
        break;

      case 6:
      case 7:
        sprintf(name, "%s, the Mummy", gb_name);
        r_ptr->r_char = 'M';
        r_ptr->r_attr = TERM_SLATE;
        r_ptr->flags1 |= (RF1_DROP_1D2);
        r_ptr->flags2 |= (RF2_OPEN_DOOR | RF2_BASH_DOOR);
        r_ptr->flags3 |= (RF3_IM_COLD);
        r_ptr->ac = 35;
        r_ptr->speed = 110;
        r_ptr->hdice = 15;
        r_ptr->hside = 10;
        r_ptr->mexp = dun_level * 6 + 10;
        ghost_blow(0, RBM_HIT, RBE_HURT, 2, 6);
        ghost_blow(1, RBM_HIT, RBE_HURT, 2, 6);
        break;

      case 8:
        sprintf(name, "%s, the Poltergeist", gb_name);
        r_ptr->r_char = 'G';
        r_ptr->r_attr = TERM_WHITE;
        r_ptr->flags1 |= (RF1_RAND_50 | RF1_RAND_25);
        r_ptr->flags1 |= (RF1_DROP_60 | RF1_DROP_90);
        r_ptr->flags2 |= (RF2_INVISIBLE | RF2_PASS_WALL);
        r_ptr->flags3 |= (RF3_IM_COLD);
        r_ptr->ac = 20;
        r_ptr->speed = 130;
        r_ptr->hdice = 10;
        r_ptr->hside = 10;
        r_ptr->mexp = dun_level * 6 + 10;
        ghost_blow(0, RBM_HIT, RBE_HURT, 1, 6);
        ghost_blow(1, RBM_HIT, RBE_HURT, 1, 6);
        ghost_blow(2, RBM_TOUCH, RBE_TERRIFY, 0, 0);
        ghost_blow(3, RBM_TOUCH, RBE_TERRIFY, 0, 0);
        break;

      case 9:
      case 10:
        sprintf(name, "%s, the Spirit", gb_name);
        r_ptr->r_char = 'G';
        r_ptr->r_attr = TERM_UMBER;
        r_ptr->flags1 |= (RF1_RAND_25 | RF1_DROP_1D2);
        r_ptr->flags2 |= (RF2_INVISIBLE | RF2_PASS_WALL);
        r_ptr->flags3 |= (RF3_IM_COLD | RF3_IM_ELEC);
        r_ptr->flags5 |= (RF5_BLIND | RF5_SCARE);
        r_ptr->freq_inate = r_ptr->freq_spell = 100 / 10;
        r_ptr->ac = 20;
        r_ptr->speed = 110;
        r_ptr->hdice = 20;
        r_ptr->hside = 10;
        r_ptr->mexp = dun_level * 15 + 10;
        ghost_blow(0, RBM_HIT, RBE_HURT, 3, 6);
        ghost_blow(1, RBM_HIT, RBE_HURT, 3, 6);
        ghost_blow(2, RBM_TOUCH, RBE_LOSE_DEX, 1, 8);
        ghost_blow(3, RBM_WAIL, RBE_TERRIFY, 0, 0);
        break;

      case 11:
        sprintf(name, "%s, the Ghost", gb_name);
        r_ptr->r_char = 'G';
        r_ptr->r_attr = TERM_WHITE;
        r_ptr->flags1 |= (RF1_RAND_25 | RF1_DROP_1D2);
        r_ptr->flags2 |= (RF2_INVISIBLE | RF2_PASS_WALL);
        r_ptr->flags2 |= (RF2_TAKE_ITEM);
        r_ptr->flags3 |= (RF3_IM_COLD);
        r_ptr->flags5 |= (RF5_BLIND | RF5_HOLD | RF5_DRAIN_MANA);
        r_ptr->freq_inate = r_ptr->freq_spell = 100 / 10;
        r_ptr->ac = 40;
        r_ptr->speed = 120;
        r_ptr->hdice = 20;
        r_ptr->hside = 10;
        r_ptr->mexp = dun_level * 20 + 10;
        ghost_blow(0, RBM_WAIL, RBE_TERRIFY, 0, 0);
        ghost_blow(1, RBM_TOUCH, RBE_EXP_20, 0, 0);
        ghost_blow(2, RBM_CLAW, RBE_LOSE_INT, 1, 6);
        ghost_blow(3, RBM_CLAW, RBE_LOSE_WIS, 1, 6);
        break;

      case 12:
        sprintf(name, "%s, the Vampire", gb_name);
        r_ptr->r_char = 'V';
        r_ptr->r_attr = TERM_VIOLET;
        r_ptr->flags1 |= (RF1_DROP_2D2);
        r_ptr->flags2 |= (RF2_OPEN_DOOR | RF2_BASH_DOOR);
        r_ptr->flags3 |= (RF3_HURT_LITE);
        r_ptr->flags5 |= (RF5_SCARE | RF5_HOLD | RF5_CAUSE_2);
        r_ptr->flags6 |= (RF6_TELE_TO);
        r_ptr->freq_inate = r_ptr->freq_spell = 100 / 8;
        r_ptr->ac = 40;
        r_ptr->speed = 110;
        r_ptr->hdice = 50;
        r_ptr->hside = 10;
        r_ptr->mexp = dun_level * 20 + 100;
        ghost_blow(0, RBM_HIT, RBE_HURT, 3, 8);
        ghost_blow(1, RBM_HIT, RBE_HURT, 3, 8);
        ghost_blow(2, RBM_BITE, RBE_EXP_40, 0, 0);
        break;

      case 13:
        sprintf(name, "%s, the Wraith", gb_name);
        r_ptr->r_char = 'W';
        r_ptr->r_attr = TERM_WHITE;
        r_ptr->flags1 |= (RF1_DROP_2D2 | RF1_DROP_4D2);
        r_ptr->flags2 |= (RF2_OPEN_DOOR | RF2_BASH_DOOR);
        r_ptr->flags3 |= (RF3_IM_COLD | RF3_HURT_LITE);
        r_ptr->flags5 |= (RF5_BLIND | RF5_SCARE | RF5_HOLD);
        r_ptr->flags5 |= (RF5_CAUSE_3 | RF5_BO_NETH);
        r_ptr->freq_inate = r_ptr->freq_spell = 100 / 8;
        r_ptr->ac = 60;
        r_ptr->speed = 120;
        r_ptr->hdice = 80;
        r_ptr->hside = 10;
        r_ptr->mexp = dun_level * 30 + 100;
        ghost_blow(0, RBM_HIT, RBE_HURT, 3, 8);
        ghost_blow(1, RBM_HIT, RBE_HURT, 3, 8);
        ghost_blow(2, RBM_TOUCH, RBE_EXP_20, 0, 0);
        break;

      case 14:
        sprintf(name, "%s, the Vampire Lord", gb_name);
        r_ptr->r_char = 'V';
        r_ptr->r_attr = TERM_BLUE;
        r_ptr->flags1 |= (RF1_DROP_1D2 | RF1_DROP_GREAT);
        r_ptr->flags2 |= (RF2_OPEN_DOOR | RF2_BASH_DOOR);
        r_ptr->flags3 |= (RF3_HURT_LITE);
        r_ptr->flags5 |= (RF5_SCARE | RF5_HOLD | RF5_CAUSE_3 | RF5_BO_NETH);
        r_ptr->flags6 |= (RF6_TELE_TO);
        r_ptr->freq_inate = r_ptr->freq_spell = 100 / 8;
        r_ptr->ac = 80;
        r_ptr->speed = 110;
        r_ptr->hdice = 15;
        r_ptr->hside = 100;
        r_ptr->mexp = dun_level * 100 + 100;
        ghost_blow(0, RBM_HIT, RBE_HURT, 3, 8);
        ghost_blow(1, RBM_HIT, RBE_HURT, 3, 8);
        ghost_blow(2, RBM_HIT, RBE_HURT, 3, 8);
        ghost_blow(3, RBM_BITE, RBE_EXP_80, 0, 0);
        break;

      case 15:
        sprintf(name, "%s, the Shadow", gb_name);
        r_ptr->r_char = 'G';
        r_ptr->r_attr = TERM_L_DARK;
        r_ptr->flags1 |= (RF1_DROP_2D2 | RF1_DROP_GREAT);
        r_ptr->flags2 |= (RF2_INVISIBLE | RF2_PASS_WALL);
        r_ptr->flags3 |= (RF3_IM_COLD);
        r_ptr->flags5 |= (RF5_BLIND | RF5_CONF | RF5_HOLD | RF5_DRAIN_MANA);
        r_ptr->freq_inate = r_ptr->freq_spell = 100 / 4;
        r_ptr->ac = 90;
        r_ptr->speed = 130;
        r_ptr->hdice = 20;
        r_ptr->hside = 100;
        r_ptr->mexp = dun_level * 100 + 100;
        ghost_blow(0, RBM_TOUCH, RBE_EXP_40, 0, 0);
        ghost_blow(1, RBM_TOUCH, RBE_EXP_40, 0, 0);
        ghost_blow(2, RBM_CLAW, RBE_LOSE_INT, 2, 8);
        ghost_blow(3, RBM_CLAW, RBE_LOSE_WIS, 2, 8);
        break;

      case 16:
        sprintf(name, "%s, the Phantom", gb_name);
        r_ptr->r_char = 'G';
        r_ptr->r_attr = TERM_VIOLET;
        r_ptr->flags1 |= (RF1_DROP_1D2 | RF1_DROP_2D2 | RF1_DROP_GREAT);
        r_ptr->flags2 |= (RF2_SMART | RF2_INVISIBLE | RF2_PASS_WALL);
        r_ptr->flags3 |= (RF3_IM_COLD | RF3_IM_ELEC);
        r_ptr->flags5 |= (RF5_BLIND | RF5_CONF | RF5_HOLD | RF5_DRAIN_MANA);
        r_ptr->flags5 |= (RF5_BRAIN_SMASH);
        r_ptr->flags5 |= (RF5_BA_NETH | RF5_BO_NETH);
        r_ptr->flags6 |= (RF6_TELE_TO | RF6_TELE_LEVEL);
        r_ptr->freq_inate = r_ptr->freq_spell = 100 / 4;
        r_ptr->ac = 130;
        r_ptr->speed = 130;
        r_ptr->hdice = 30;
        r_ptr->hside = 100;
        r_ptr->mexp = dun_level * 200 + 100;
        ghost_blow(0, RBM_TOUCH, RBE_EXP_80, 0, 0);
        ghost_blow(1, RBM_TOUCH, RBE_EXP_80, 0, 0);
        ghost_blow(2, RBM_CLAW, RBE_LOSE_INT, 2, 8);
        ghost_blow(3, RBM_CLAW, RBE_LOSE_WIS, 2, 8);
        break;

      default:
        sprintf(name, "%s, the Lich", gb_name);
        r_ptr->r_char = 'L';
        r_ptr->r_attr = TERM_ORANGE;
        r_ptr->flags1 |= (RF1_DROP_2D2 | RF1_DROP_1D2 | RF1_DROP_GREAT);
        r_ptr->flags2 |= (RF2_SMART | RF2_OPEN_DOOR | RF2_BASH_DOOR);
        r_ptr->flags3 |= (RF3_IM_COLD);
        r_ptr->flags5 |= (RF5_BLIND | RF5_SCARE | RF5_CONF | RF5_HOLD);
        r_ptr->flags5 |= (RF5_DRAIN_MANA | RF5_BA_FIRE | RF5_BA_COLD);
        r_ptr->flags5 |= (RF5_CAUSE_3 | RF5_CAUSE_4 | RF5_BRAIN_SMASH);
        r_ptr->flags6 |= (RF6_BLINK | RF6_TPORT | RF6_TELE_TO);
        r_ptr->flags6 |= (RF6_S_UNDEAD);
        r_ptr->freq_inate = r_ptr->freq_spell = 100 / 4;
        r_ptr->ac = 120;
        r_ptr->speed = 120;
        r_ptr->hdice = 40;
        r_ptr->hside = 100;
        r_ptr->mexp = dun_level * 300 + 100;
        ghost_blow(0, RBM_TOUCH, RBE_LOSE_DEX, 2, 12);
        ghost_blow(1, RBM_TOUCH, RBE_LOSE_DEX, 2, 12);
        ghost_blow(2, RBM_TOUCH, RBE_UN_POWER, 0, 0);
        ghost_blow(3, RBM_TOUCH, RBE_EXP_80, 0, 0);
        break;
    }
}



/*
 * Attempt to place a ghost somewhere.
 *
 * Hack -- this routine must also "prepare" the "ghost" info
 *
 * XXX XXX XXX This will change for Angband 2.8.0.
 *
 * Although we extract the "name", "hitpoints, "race", "class" of
 * the "dead player" from the "bone" file, only the "name" is used.
 */
bool alloc_ghost(void)
{
    int			y, x;
    
    int			hp, gr, gc;

    cave_type		*c_ptr;
    monster_type	*m_ptr;

    monster_race	*r_ptr = &r_info[MAX_R_IDX-1];

    FILE		*fp;

    bool		err = FALSE;

    char		name[100];
    char		tmp[1024];


    /* Hack -- no ghosts in the town */
    if (!dun_level) return (FALSE);


    /* Hack -- "ghosts" must be "unique" */
    if (r_ptr->cur_num >= r_ptr->max_num) return (FALSE);


    /* Only a 25% chance */
    if (rand_int(100) < 75) return (FALSE);

    /* And even then, it only happens sometimes */
    if (randint((dun_level / 2) + 11) < 14) return (FALSE);


    /* Choose a bones file */
    sprintf(tmp, "%sbone.%03d", ANGBAND_DIR_BONE, dun_level);

    /* Open the bones file */
    fp = my_fopen(tmp, "r");

    /* No bones file to use */
    if (!fp) return (FALSE);

    /* XXX XXX XXX Scan the file */
    err = (fscanf(fp, "%[^\n]\n%d\n%d\n%d", name, &hp, &gr, &gc) != 4);

    /* Close the file */
    my_fclose(fp);

    /* Delete the bones file */
    remove(tmp);

    /* Catch errors */
    if (err) {
        msg_print("Warning -- deleted corrupt 'ghost' file!");
        return (FALSE);
    }


    /* Hack -- Set up the ghost */
    set_ghost(name);


    /* Note for wizard (special ghost name) */
    if (cheat_hear) msg_print("Unique (Ghost)");

    /* Hack -- pick a nice (far away) location */
    while (1) {

        /* Pick a location */
        y = rand_int(cur_hgt);
        x = rand_int(cur_wid);

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

    /* Mega-Hack -- catch "failure" */
    if (!c_ptr->m_idx) return (FALSE);


    /* Access the monster */
    m_ptr = &m_list[c_ptr->m_idx];

    m_ptr->fy = y;
    m_ptr->fx = x;

    /* Hack -- the monster is a ghost */
    m_ptr->r_idx = MAX_R_IDX-1;


    /* Hack -- Count the ghosts (only one) */
    r_ptr->cur_num++;


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
    r_ptr->l_attr = r_ptr->r_attr;
    r_ptr->l_char = r_ptr->r_char;


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
 * Use "monster_level" for the monster level
 */
bool alloc_monster(int dis, int slp)
{
    int			y, x;

    /* Find a legal, distant, unoccupied, space */
    while (1) {

        /* Pick a location */
        y = rand_int(cur_hgt);
        x = rand_int(cur_wid);

        /* Require "naked" floor grid */
        if (!naked_grid_bold(y,x)) continue;

        /* Accept far away grids */
        if (distance(y, x, py, px) > dis) break;
    }

    /* Attempt to place the monster, allow groups */
    if (place_monster(y, x, slp, TRUE)) return (TRUE);

    /* Nope */
    return (FALSE);
}




/*
 * Hack -- the "type" of the current "summon specific"
 */
static int summon_specific_type = 0;


/*
 * Hack -- help decide if a monster race is "okay" to summon
 */
static bool summon_specific_okay(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];

    bool okay = FALSE;


    /* Hack -- no specific type specified */
    if (!summon_specific_type) return (TRUE);


    /* Check our requirements */
    switch (summon_specific_type) {

        case SUMMON_ANT:
            okay = ((r_ptr->r_char == 'a') &&
                    !(r_ptr->flags1 & RF1_UNIQUE));
            break;

        case SUMMON_SPIDER:
            okay = ((r_ptr->r_char == 'S') &&
                    !(r_ptr->flags1 & RF1_UNIQUE));
            break;

        case SUMMON_HOUND:
            okay = (((r_ptr->r_char == 'C') || (r_ptr->r_char == 'Z')) &&
                    !(r_ptr->flags1 & RF1_UNIQUE));
            break;

        case SUMMON_REPTILE:
            okay = ((r_ptr->r_char == 'R') &&
                    !(r_ptr->flags1 & RF1_UNIQUE));
            break;

        case SUMMON_ANGEL:
            okay = ((r_ptr->r_char == 'A') &&
                    !(r_ptr->flags1 & RF1_UNIQUE));
            break;

        case SUMMON_DEMON:
            okay = ((r_ptr->flags3 & RF3_DEMON) &&
                !(r_ptr->flags1 & RF1_UNIQUE));
            break;

        case SUMMON_UNDEAD:
            okay = ((r_ptr->flags3 & RF3_UNDEAD) &&
                    !(r_ptr->flags1 & RF1_UNIQUE));
            break;

        case SUMMON_DRAGON:
            okay = ((r_ptr->flags3 & RF3_DRAGON) &&
                    !(r_ptr->flags1 & RF1_UNIQUE));
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
                    (r_ptr->flags1 & RF1_UNIQUE));
            break;

        case SUMMON_UNIQUE:
            okay = (r_ptr->flags1 & RF1_UNIQUE);
            break;
    }

    /* Result */
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
 * Note that the summoning of Unique monsters may not succeed (!)
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
bool summon_specific(int y1, int x1, int lev, int type)
{
    int i, x, y;

    bool result = FALSE;
    
    /* Try to place it */
    for (i = 0; i < 20; ++i) {

        /* Pick a distance */
        int d = (i / 15) + 1;

        /* Pick a location */
        scatter(&y, &x, y1, x1, d, 0);

        /* Require "empty" floor grid */
        if (!empty_grid_bold(y, x)) continue;

        /* Hack -- no summon on glyph of warding */
        if ((cave[y][x].feat & 0x3F) == 0x03) continue;
        
        /* Save the "summon" type */
        summon_specific_type = type;

        /* Require "okay" monsters */
        get_mon_num_hook = summon_specific_okay;

        /* Choose a monster */
        monster_level = (dun_level + lev) / 2 + 5;

        /* Attempt to place the monster (allow groups) */	
        result = place_monster(y, x, FALSE, TRUE);

        /* Restore dungoen level */
        monster_level = dun_level;

        /* Remove restriction */
        get_mon_num_hook = NULL;

        /* Forget "summon" type */
        summon_specific_type = 0;

        /* Done */
        break;
    }

    /* Failure */
    return (result);
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

    monster_race *r_ptr = &r_info[m_ptr->r_idx];

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
 *
 * Note that if "smell" is turned on, all monsters get vicious.
 *
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
    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    /* Monster flowing disabled */
    if (!flow_by_sound) return (FALSE);

    /* Monster can go through rocks */
    if (r_ptr->flags2 & RF2_PASS_WALL) return (FALSE);
    if (r_ptr->flags2 & RF2_KILL_WALL) return (FALSE);

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
        y = y1 + ddy_ddd[i];
        x = x1 + ddx_ddd[i];

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
        (*yp) = py + 16 * ddy_ddd[i];
        (*xp) = px + 16 * ddx_ddd[i];
    }

    /* No legal move (?) */
    if (!when) return (FALSE);

    /* Success */
    return (TRUE);
}

#endif


/*
 * Choose "logical" directions for monster movement
 */
static void get_moves(int m_idx, int *mm)
{
    monster_type *m_ptr = &m_list[m_idx];

    int y, ay, x, ax;

    int move_val = 0;

    int y2 = py;
    int x2 = px;


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
 * Let the given monster attempt to reproduce.
 *
 * Note that "reproduction" REQUIRES empty space.
 */
bool multiply_monster(int m_idx)
{
    monster_type	*m_ptr = &m_list[m_idx];

    int			i, y, x;

    /* Try up to 18 times */
    for (i = 0; i < 18; i++) {

        int d = 1;

        /* Pick a location */
        scatter(&y, &x, m_ptr->fy, m_ptr->fx, d, 0);

        /* Require an "empty" floor grid */
        if (!empty_grid_bold(y, x)) continue;

        /* Create a new monster (no groups) */
        if (place_monster_aux(y, x, m_ptr->r_idx, FALSE, FALSE)) return (TRUE);
    }

    /* Result */
    return (FALSE);
}




/*
 * Hack -- local "player stealth" value (see below)
 */
static u32b noise = 0L;


/*
 * Process a monster
 *
 * The monster is known to be within 100 grids of the player
 *
 * In several cases, we directly update the monster lore
 *
 * Note that a monster is only allowed to "reproduce" if there
 * are a limited number of "reproducing" monsters on the current
 * level.  This should prevent the level from being "swamped" by
 * reproducing monsters.  It also allows a large mass of mice to
 * prevent a louse from multiplying, but this is a small price to
 * pay for a simple multiplication method.
 *
 * XXX Monster fear is slightly odd, in particular, monsters will
 * fixate on opening a door even if they cannot open it.  Actually,
 * the same thing happens to normal monsters when they hit a door
 *
 * XXX XXX XXX In addition, monsters which *cannot* open or bash
 * down a door will still stand there trying to open it.  :-(
 *
 * XXX Technically, need to check for monster in the way
 * combined with that monster being in a wall (or door?)
 *
 * A "direction" of "5" means "pick a random direction".
 */
static void process_monster(int m_idx)
{
    monster_type	*m_ptr = &m_list[m_idx];
    monster_race	*r_ptr = &r_info[m_ptr->r_idx];

    int			i, d, oy, ox, ny, nx;

    int			mm[8];
    
    cave_type    	*c_ptr;
    inven_type  	*i_ptr;
    monster_type	*y_ptr;

    bool		do_turn;
    bool		do_move;
    bool		do_view;

    bool		did_open_door;
    bool		did_bash_door;
    bool		did_take_item;
    bool		did_kill_item;
    bool		did_move_body;
    bool		did_kill_body;
    bool		did_pass_wall;
    bool		did_kill_wall;


    /* Handle "sleep" */
    if (m_ptr->csleep) {

        u32b notice = 0;
        
        /* Hack -- handle non-aggravation */
        if (!p_ptr->aggravate) notice = rand_int(1024);

        /* Hack -- See if monster "notices" player */
        if ((notice * notice * notice) <= noise) {

            /* Hack -- amount of "waking" */
            int d = 1;

            /* Wake up faster near the player */
            if (m_ptr->cdis < 50) d = (100 / m_ptr->cdis);

            /* Hack -- handle aggravation */
            if (p_ptr->aggravate) d = m_ptr->csleep;

            /* Still asleep */
            if (m_ptr->csleep > d) {

                /* Monster wakes up "a little bit" */
                m_ptr->csleep -= d;

                /* Notice the "not waking up" */
                if (m_ptr->ml) {

                    /* Hack -- Count the ignores */
                    if (r_ptr->r_ignore < MAX_UCHAR) r_ptr->r_ignore++;
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

                    /* Hack -- Count the wakings */
                    if (r_ptr->r_wake < MAX_UCHAR) r_ptr->r_wake++;
                }
            }
        }

        /* Still sleeping */
        if (m_ptr->csleep) return;
    }


    /* Handle "stun" */
    if (m_ptr->stunned) {

        int d = 1;
        
        /* Make a "saving throw" against stun */
        if (rand_int(5000) <= r_ptr->level * r_ptr->level) {
        
            /* Recover fully */
            d = m_ptr->stunned;
        }
        
        /* Hack -- Recover from stun */
        if (m_ptr->stunned > d) {

            /* Recover somewhat */
            m_ptr->stunned -= d;
        }
        
        /* Fully recover */
        else {
        
            /* Recover fully */
            m_ptr->stunned = 0;
            
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
        int d = randint(r_ptr->level / 10 + 1);

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
        int d = randint(r_ptr->level / 10 + 1);

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


    /* Get the origin */
    oy = m_ptr->fy;
    ox = m_ptr->fx;
        

    /* Attempt to "mutiply" if able and allowed */
    if ((r_ptr->flags2 & RF2_MULTIPLY) && (num_repro < MAX_REPRO)) {

        int k, y, x;

        /* Count the adjacent monsters */
        for (k = 0, y = oy - 1; y <= oy + 1; y++) {
            for (x = ox - 1; x <= ox + 1; x++) {
                if (cave[y][x].m_idx) k++;
            }
        }

        /* Hack -- multiply slower in crowded areas */
        if ((k < 4) && (!k || !rand_int(k * MON_MULT_ADJ))) {

            /* Try to multiply */
            if (multiply_monster(m_idx)) {

                /* Take note if visible */
                if (m_ptr->ml) r_ptr->r_flags2 |= RF2_MULTIPLY;

                /* Multiplying takes energy */
                return;
            }
        }
    }


    /* Attempt to cast a spell */
    if (make_attack_spell(m_idx)) return;


    /* Hack -- Assume no movement */
    mm[0] = mm[1] = mm[2] = mm[3] = 0;
    mm[4] = mm[5] = mm[6] = mm[7] = 0;


    /* Confused -- 100% random */
    if (m_ptr->confused) {

        /* Try four "random" directions */
        mm[0] = mm[1] = mm[2] = mm[3] = 5;
    }

    /* 75% random movement */
    else if ((r_ptr->flags1 & RF1_RAND_50) &&
             (r_ptr->flags1 & RF1_RAND_25) &&
             (rand_int(100) < 75)) {

        /* Memorize flags */
        if (m_ptr->ml) r_ptr->r_flags1 |= RF1_RAND_50;
        if (m_ptr->ml) r_ptr->r_flags1 |= RF1_RAND_25;

        /* Try four "random" directions */
        mm[0] = mm[1] = mm[2] = mm[3] = 5;
    }

    /* 50% random movement */
    else if ((r_ptr->flags1 & RF1_RAND_50) &&
             (rand_int(100) < 50)) {

        /* Memorize flags */
        if (m_ptr->ml) r_ptr->r_flags1 |= RF1_RAND_50;

        /* Try four "random" directions */
        mm[0] = mm[1] = mm[2] = mm[3] = 5;
    }

    /* 25% random movement */
    else if ((r_ptr->flags1 & RF1_RAND_25) &&
             (rand_int(100) < 25)) {

        /* Memorize flags */
        if (m_ptr->ml) r_ptr->r_flags1 |= RF1_RAND_25;

        /* Try four "random" directions */
        mm[0] = mm[1] = mm[2] = mm[3] = 5;
    }

    /* Normal movement */
    else {

        /* Logical moves */
        get_moves(m_idx, mm);
    }


    /* Assume nothing */
    do_turn = FALSE;
    do_move = FALSE;
    do_view = FALSE;

    /* Assume nothing */
    did_open_door = FALSE;
    did_bash_door = FALSE;
    did_take_item = FALSE;
    did_kill_item = FALSE;
    did_move_body = FALSE;
    did_kill_body = FALSE;
    did_pass_wall = FALSE;
    did_kill_wall = FALSE;


    /* Take a zero-terminated array of "directions" */
    for (i = 0; mm[i]; i++) {

        /* Get the direction */
        d = mm[i];
        
        /* Hack -- allow "randomized" motion */
        if (d == 5) d = ddd[rand_int(8)];

        /* Get the destination */
        ny = oy + ddy[d];
        nx = ox + ddx[d];

        /* Access that cave grid */
        c_ptr = &cave[ny][nx];

        /* Access that cave grid's contents */
        i_ptr = &i_list[c_ptr->i_idx];

        /* Access that cave grid's contents */
        y_ptr = &m_list[c_ptr->m_idx];


        /* Floor is open? */
        if (floor_grid_bold(ny, nx)) {

            /* Go ahead and move */
            do_move = TRUE;
        }

        /* Permanent wall */
        else if ((c_ptr->feat & 0x3F) >= 0x3C) {

            /* Nothing */
        }

        /* Monster moves through walls (and doors) */
        else if (r_ptr->flags2 & RF2_PASS_WALL) {

            /* Pass through walls/doors/rubble */
            do_move = TRUE;

            /* Monster went through a wall */
            did_pass_wall = TRUE;
        }

        /* Monster destroys walls (and doors) */
        else if (r_ptr->flags2 & RF2_KILL_WALL) {

            /* Eat through walls/doors/rubble */
            do_move = TRUE;

            /* Monster destroyed a wall */
            did_kill_wall = TRUE;

            /* Clear the wall code, if any */
            c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x01);

            /* Forget the "field mark", if any */
            c_ptr->feat &= ~CAVE_MARK;

            /* Notice */
            note_spot(ny, nx);

            /* Redraw */
            lite_spot(ny, nx);

            /* Note changes to viewable region */
            if (player_has_los_bold(ny, nx)) do_view = TRUE;
        }

        /* Handle doors and secret doors */
        else if ((c_ptr->feat & 0x3F) <= 0x30) {

            bool may_bash = TRUE;

            /* Take a turn */
            do_turn = TRUE;

            /* Creature can open doors. */
            if (r_ptr->flags2 & RF2_OPEN_DOOR) {

                /* Closed doors and secret doors */
                if (((c_ptr->feat & 0x3F) == 0x20) ||
                    ((c_ptr->feat & 0x3F) == 0x30)) {

                    /* The door is open */
                    did_open_door = TRUE;

                    /* Do not bash the door */
                    may_bash = FALSE;
                }

                /* Locked doors (not jammed) */
                else if ((c_ptr->feat & 0x3F) < 0x28) {

#if 0
                    /* XXX XXX XXX XXX XXX Old test (pval 10 to 20) */
                    if (randint((m_ptr->hp + 1) * (50 + i_ptr->pval)) <
                        40 * (m_ptr->hp - 10 - i_ptr->pval))
#endif

                    /* Try to unlock it */
                    if (rand_int(m_ptr->hp / 10) > (c_ptr->feat & 0x07)) {

                        /* Unlock the door */
                        c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x20);

                        /* Do not bash the door */
                        may_bash = FALSE;
                    }
                }
            }

            /* Stuck doors -- attempt to bash them down if allowed */
            if (may_bash && (r_ptr->flags2 & RF2_BASH_DOOR)) {

#if 0
                /* XXX XXX XXX XXX XXX Old test (pval 10 to 20) */
                if (randint((m_ptr->hp + 1) * (50 + i_ptr->pval)) <
                    40 * (m_ptr->hp - 10 - i_ptr->pval))
#endif

                /* Attempt to Bash */
                if (rand_int(m_ptr->hp / 10) > (c_ptr->feat & 0x07)) {

                    /* Message */
                    msg_print("You hear a door burst open!");
                        
                    /* Disturb (sometimes) */
                    if (disturb_other) disturb(1, 0);

                    /* The door was bashed open */
                    did_bash_door = TRUE;

                    /* Hack -- fall into doorway */
                    do_move = TRUE;
                }
            }


            /* Deal with doors in the way */
            if (did_open_door || did_bash_door) {

                /* Break down the door */
                if (did_bash_door && (rand_int(100) < 50)) {
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x05);
                }
                
                /* Open the door */
                else {
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x04);
                }

                /* Notice */
                note_spot(ny, nx);

                /* Redraw */
                lite_spot(ny, nx);

                /* Handle viewable doors */
                if (player_has_los_bold(ny, nx)) do_view = TRUE;
            }
        }


        /* Hack -- check for Glyph of Warding */
        if (do_move && ((c_ptr->feat & 0x3F) == 0x03)) {

            /* Assume no move allowed */
            do_move = FALSE;

            /* Break the ward */
            if (randint(BREAK_GLYPH) < r_ptr->level) {

                /* Describe observable breakage */
                if (player_can_see_bold(ny, nx)) {
                    msg_print("The rune of protection is broken!");
                }

                /* Break the rune */
                c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x01);

                /* Allow movement */
                do_move = TRUE;
            }
        }

        /* Some monsters never attack */
        if (do_move && (ny == py) && (nx == px) &&
            (r_ptr->flags1 & RF1_NEVER_BLOW)) {

            /* Hack -- memorize lack of attacks */
            /* if (m_ptr->ml) r_ptr->r_flags1 |= RF1_NEVER_BLOW; */

            /* Do not move */
            do_move = FALSE;
        }


        /* The player is in the way.  Attack him. */
        if (do_move && (ny == py) && (nx == px)) {

            /* Do the attack */
            (void)make_attack_normal(m_idx);

            /* Do not move */
            do_move = FALSE;

            /* Took a turn */
            do_turn = TRUE;
        }


        /* Some monsters never move */
        if (do_move && (r_ptr->flags1 & RF1_NEVER_MOVE)) {

            /* Hack -- memorize lack of attacks */
            /* if (m_ptr->ml) r_ptr->r_flags1 |= RF1_NEVER_MOVE; */

            /* Do not move */
            do_move = FALSE;
        }


        /* A monster is in the way */
        if (do_move && c_ptr->m_idx) {

            monster_race *z_ptr = &r_info[y_ptr->r_idx];
            
            /* Assume no movement */
            do_move = FALSE;

            /* Kill weaker monsters */
            if ((r_ptr->flags2 & RF2_KILL_BODY) &&
                (r_ptr->mexp > z_ptr->mexp)) {

                /* Allow movement */
                do_move = TRUE;

                /* Monster ate another monster */
                did_kill_body = TRUE;

                /* XXX XXX XXX Message */

                /* Kill the monster */
                delete_monster(ny, nx);

                /* Hack -- get the empty monster */
                y_ptr = &m_list[c_ptr->m_idx];
            }

            /* Push past weaker monsters (unless leaving a wall) */
            if ((r_ptr->flags2 & RF2_MOVE_BODY) &&
                (r_ptr->mexp > z_ptr->mexp) &&
                (floor_grid_bold(m_ptr->fy, m_ptr->fx))) {

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

            /* Hack -- Update the old location */
            cave[oy][ox].m_idx = c_ptr->m_idx;

            /* Mega-Hack -- move the old monster, if any */
            if (c_ptr->m_idx) {
    
                /* Move the old monster */        
                y_ptr->fy = oy;
                y_ptr->fx = ox;

                /* Update the old monster */
                update_mon(c_ptr->m_idx, TRUE);
            }

            /* Hack -- Update the new location */
            c_ptr->m_idx = m_idx;

            /* Move the monster */
            m_ptr->fy = ny;
            m_ptr->fx = nx;
            
            /* Update the monster */
            update_mon(m_idx, TRUE);

            /* Redraw the old grid */
            lite_spot(oy, ox);
            
            /* Redraw the new grid */
            lite_spot(ny, nx);

            /* Hack -- Moving monsters can disturb the player */
            if (m_ptr->ml &&
                (disturb_move ||
                 (disturb_near && (m_ptr->cdis < 5)))) {
                 
                disturb(0, 0);
            }


            /* XXX XXX XXX Change for Angband 2.8.0 */
            
            /* Take or Kill objects (not "gold") on the floor */
            if (i_ptr->k_idx && (i_ptr->tval != TV_GOLD) &&
                ((r_ptr->flags2 & RF2_TAKE_ITEM) ||
                 (r_ptr->flags2 & RF2_KILL_ITEM))) {

                u32b f1, f2, f3;

                u32b flg3 = 0L;

                char m_name[80];
                char i_name[80];

                /* Check the grid */
                i_ptr = &i_list[c_ptr->i_idx];

                /* Extract some flags */
                inven_flags(i_ptr, &f1, &f2, &f3);
                
                /* Acquire the object name */
                objdes(i_name, i_ptr, TRUE, 3);

                /* Acquire the monster name */
                monster_desc(m_name, m_ptr, 0x04);

                /* React to objects that hurt the monster */
                if (f1 & TR1_KILL_DRAGON) flg3 |= RF3_DRAGON;
                if (f1 & TR1_SLAY_DRAGON) flg3 |= RF3_DRAGON;
                if (f1 & TR1_SLAY_TROLL) flg3 |= RF3_TROLL;
                if (f1 & TR1_SLAY_GIANT) flg3 |= RF3_GIANT;
                if (f1 & TR1_SLAY_ORC) flg3 |= RF3_ORC;
                if (f1 & TR1_SLAY_DEMON) flg3 |= RF3_DEMON;
                if (f1 & TR1_SLAY_UNDEAD) flg3 |= RF3_UNDEAD;
                if (f1 & TR1_SLAY_ANIMAL) flg3 |= RF3_ANIMAL;
                if (f1 & TR1_SLAY_EVIL) flg3 |= RF3_EVIL;

                /* The object cannot be picked up by the monster */
                if (artifact_p(i_ptr) || (r_ptr->flags3 & flg3)) {

                    /* Only give a message for "take_item" */
                    if (r_ptr->flags2 & RF2_TAKE_ITEM) {

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
                else if (r_ptr->flags2 & RF2_TAKE_ITEM) {

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

        /* Stop when done */
        if (do_turn) break;
    }


    /* Notice changes in view */
    if (do_view) {

        /* Update some things */
        p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
    }


    /* Learn things from observable monster */
    if (m_ptr->ml) {

        /* Monster opened a door */
        if (did_open_door) r_ptr->r_flags2 |= RF2_OPEN_DOOR;

        /* Monster bashed a door */
        if (did_bash_door) r_ptr->r_flags2 |= RF2_BASH_DOOR;

        /* Monster tried to pick something up */
        if (did_take_item) r_ptr->r_flags2 |= RF2_TAKE_ITEM;

        /* Monster tried to crush something */
        if (did_kill_item) r_ptr->r_flags2 |= RF2_KILL_ITEM;

        /* Monster pushed past another monster */
        if (did_move_body) r_ptr->r_flags2 |= RF2_MOVE_BODY;

        /* Monster ate another monster */
        if (did_kill_body) r_ptr->r_flags2 |= RF2_KILL_BODY;

        /* Monster passed through a wall */
        if (did_pass_wall) r_ptr->r_flags2 |= RF2_PASS_WALL;

        /* Monster destroyed a wall */
        if (did_kill_wall) r_ptr->r_flags2 |= RF2_KILL_WALL;
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

        /* XXX XXX XXX Actually do something now (?) */
    }
}




/*
 * Process all the monsters, once per game turn.
 *
 * During each game turn, energize every "real" monster, and allow fully
 * energized monsters to move, attack, etc.
 *
 * Note that a monster can ONLY move in the monster array when someone
 * calls "reorder_monsters()", which should be called very rarely.
 *
 * This function is responsible for at least half of the processor time
 * on a normal system with a "normal" amount of monsters and a player doing
 * normal things.
 *
 * When the player is resting, virtually 90% of the processor time is spent
 * in this function, and its children, "process_monster()" and "make_move()".
 *
 * Actually, when the player is resting, more than half of the processor
 * time is spent just scanning through the monster list.  So make sure that
 * the main loop in "dungeon()" attempts to call "reorder_monsters()" at
 * least once every few thousand game turns if needed.  XXX XXX XXX
 *
 * Most of the rest of the time is spent in "update_view()" and "lite_spot()",
 * especially when the player is running.
 */
void process_monsters(void)
{
    int			i, e;
    int			fx, fy;

    bool		test;
    
    monster_type	*m_ptr;
    monster_race	*r_ptr;


    /* Hack -- calculate the "player noise" */
    noise = (1L << (30 - p_ptr->skill_stl));


    /* Process the monsters */
    for (i = 1; i < m_max; i++) {

        m_ptr = &m_list[i];

        /* Skip dead monsters */
        if (!m_ptr->r_idx) continue;


        /* Obtain the energy boost */
        e = extract_energy[m_ptr->mspeed];

        /* Give this monster some energy */
        m_ptr->energy += e;


        /* Not enough energy to move */
        if (m_ptr->energy < 100) continue;

        /* Use up "some" energy */
        m_ptr->energy -= 100;


        /* Hack -- Require proximity */
        if (m_ptr->cdis >= 100) continue;


        /* Access the race */
        r_ptr = &r_info[m_ptr->r_idx];

        /* Access the location */
        fx = m_ptr->fx;
        fy = m_ptr->fy;


        /* Assume no move */
        test = FALSE;
        
        /* Handle "sensing radius" */
        if (m_ptr->cdis <= r_ptr->aaf) {

            /* We can "sense" the player */
            test = TRUE;
        }

        /* Handle "sight" and "aggravation" */
        else if ((m_ptr->cdis <= MAX_SIGHT) &&
                 (player_has_los_bold(fy, fx) ||
                  p_ptr->aggravate)) {

            /* We can "see" or "feel" the player */
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


        /* Do nothing */
        if (!test) continue;


        /* Process the monster */
        process_monster(i);

        /* Hack -- notice death or departure */
        if (!alive || new_level_flag) break;
    }


    /* Handle stuff */
    handle_stuff();
}


