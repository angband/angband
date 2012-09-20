/* File: spells2.c */

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
 * Hack -- apply a hacked "projection()" to all viewable monsters
 */
static bool project_hack(int typ, int dam)
{
    int		i;
    bool	obvious = FALSE;

    /* Affect all (nearby) monsters */
    for (i = 1; i < m_max; i++) {

        monster_type *m_ptr = &m_list[i];

        int y = m_ptr->fy;
        int x = m_ptr->fx;

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Require line of sight */
        if (!player_has_los_bold(y, x)) continue;

        /* Hack -- apply the effect to the monster in that grid */
        if (project_m(1, 0, y, x, dam, typ, 0)) obvious = TRUE;
    }

    /* Result */
    return (obvious);
}


/*
 * Hack -- apply a "projection()" in a direction (or at the target)
 */
static bool project_hook(int typ, int dir, int dam, int flg)
{
    int tx, ty;

    /* Pass through the target if needed */
    flg |= (PROJECT_THRU);

    /* Use the given direction */
    tx = px + ddx[dir];
    ty = py + ddy[dir];

    /* Use an actual "target" */
    if ((dir == 5) && target_okay()) {
        tx = target_col;
        ty = target_row;
    }

    /* Analyze the "dir" and the "target", do NOT explode */
    return (project(0, 0, ty, tx, dam, typ, flg));
}

/*
 * Cast a bolt spell
 */
bool fire_bolt(int typ, int dir, int dam)
{
    int flg = PROJECT_STOP;
    return (project_hook(typ, dir, dam, flg));
}

/*
 * Cast a beam spell
 */
bool fire_beam(int typ, int dir, int dam)
{
    /* Go until we have to stop, do "beam" damage to everyone */
    /* Also, affect all grids (NOT objects) we pass through */
    int flg = PROJECT_BEAM | PROJECT_GRID;
    return (project_hook(typ, dir, dam, flg));
}

/*
 * Cast a ball spell
 */
bool fire_ball(int typ, int dir, int dam, int rad)
{
    int tx, ty;

    int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_STOP;

    /* Use the given direction */
    tx = px + 99 * ddx[dir];
    ty = py + 99 * ddy[dir];

    /* Use an actual "target" */
    if ((dir == 5) && target_okay()) {
        flg &= ~PROJECT_STOP;
        tx = target_col;
        ty = target_row;
    }

    /* Analyze the "dir" and the "target".  Hurt items on floor. */
    return (project(0, rad, ty, tx, dam, typ, flg));
}

/*
 * Cast a bolt spell, or rarely, a beam spell
 */
bool fire_bolt_or_beam(int prob, int typ, int dir, int dam)
{
    if (rand_int(100) < prob) {
        return (fire_beam(typ, dir, dam));
    }
    else {
        return (fire_bolt(typ, dir, dam));
    }
}




/*
 * Wake up all monsters, and speed up "los" monsters.
 */
void aggravate_monsters(int who)
{
    int i;

    bool sleep = FALSE;
    bool speed = FALSE;

    /* Aggravate everyone nearby */
    for (i = 1; i < m_max; i++) {

        monster_type	*m_ptr = &m_list[i];
        monster_race	*r_ptr = &r_info[m_ptr->r_idx];

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Skip aggravating monster (or player) */
        if (i == who) continue;

        /* Wake up nearby sleeping monsters */
        if (m_ptr->cdis < MAX_SIGHT * 2) {

            /* Wake up */
            if (m_ptr->csleep) {

                /* Wake up */
                m_ptr->csleep = 0;
                sleep = TRUE;
            }
        }

        /* Speed up monsters in line of sight */	
        if (player_has_los_bold(m_ptr->fy, m_ptr->fx)) {

            /* Speed up (instantly) to racial base + 10 */
            if (m_ptr->mspeed < r_ptr->speed + 10) {

                /* Speed up */
                m_ptr->mspeed = r_ptr->speed + 10;
                speed = TRUE;
            }
        }
    }

    /* Messages */
    if (speed) msg_print("You feel a sudden stirring nearby!");
    else if (sleep) msg_print("You hear a sudden stirring in the distance!");
}



/*
 * Helper function -- return a "nearby" race for polymorphing
 */
s16b poly_r_idx(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];

    int i, r, lev1, lev2;

    /* Hack -- Uniques never polymorph */
    if (r_ptr->flags1 & RF1_UNIQUE) return (r_idx);

    /* Pick a (possibly new) non-unique race */
    for (i = 0; i < 1000; i++) {

        /* Allowable range of "levels" for resulting monster */
        lev1 = r_ptr->level - ((randint(20)/randint(9))+1);
        lev2 = r_ptr->level + ((randint(20)/randint(9))+1);

        /* Increase monster depth */
        monster_level = (dun_level + r_ptr->level) / 2 + 5;

        /* Hack -- Pick a new race */
        r = get_mon_num(monster_level);

        /* Restore monster depth */
        monster_level = dun_level;

        /* Extract that monster */
        r_ptr = &r_info[r];

        /* Skip uniques */
        if (r_ptr->flags1 & RF1_UNIQUE) continue;

        /* Accept valid monsters */
        if ((r_ptr->level < lev1) && (r_ptr->level > lev2)) continue;

        /* Use that index */
        r_idx = r;
        
        /* Done */
        break;
    }

    /* Use the original */
    return (r_idx);
}


/*
 * Teleport a monster, normally up to "dis" grids away.
 *
 * Attempt to move the monster at least "dis/2" grids away.
 *
 * But allow variation to prevent infinite loops.
 */
void teleport_away(int m_idx, int dis)
{
    int			ny, nx, oy, ox, d, i, min;

    bool		look = TRUE;

    monster_type	*m_ptr = &m_list[m_idx];


    /* Paranoia */
    if (!m_ptr->r_idx) return;

    /* Save the old location */
    oy = m_ptr->fy;
    ox = m_ptr->fx;

    /* Minimum distance */
    min = dis / 2;

    /* Look until done */
    while (look) {

        /* Verify max distance */
        if (dis > 200) dis = 200;

        /* Try several locations */
        for (i = 0; i < 500; i++) {

            /* Pick a (possibly illegal) location */
            while (1) {
                ny = rand_spread(oy, dis);
                nx = rand_spread(ox, dis);
                d = distance(oy, ox, ny, nx);
                if ((d >= min) && (d <= dis)) break;
            }

            /* Ignore illegal locations */
            if (!in_bounds(ny, nx)) continue;

            /* Require "naked" floor space */
            if (!naked_grid_bold(ny, nx)) continue;

            /* No teleporting into vaults and such */
            /* if (cave[ny][nx].info & CAVE_ICKY) continue; */

            /* This grid looks good */
            look = FALSE;

            /* Stop looking */
            break;
        }

        /* Increase the maximum distance */
        dis = dis * 2;

        /* Decrease the minimum distance */
        min = min / 2;
    }
    
    /* Update the new location */
    cave[ny][nx].m_idx = m_idx;

    /* Update the old location */
    cave[oy][ox].m_idx = 0;

    /* Move the monster */
    m_ptr->fy = ny;
    m_ptr->fx = nx;

    /* Update the monster (new location) */
    update_mon(m_idx, TRUE);

    /* Redraw the old grid */
    lite_spot(oy, ox);
            
    /* Redraw the new grid */
    lite_spot(ny, nx);
}




/*
 * Delete all non-unique monsters of a given "type" from the level
 */
bool genocide(void)
{
    int		i;

    char	typ;

    bool	result = FALSE;
    
    /* Mega-Hack -- Get a monster symbol */
    (void)(get_com("Choose a monster race (by symbol) to genocide: ", &typ));

    /* Delete the monsters of that "type" */
    for (i = 1; i < m_max; i++) {

        monster_type	*m_ptr = &m_list[i];
        monster_race	*r_ptr = &r_info[m_ptr->r_idx];

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Hack -- Skip Unique Monsters */
        if (r_ptr->flags1 & RF1_UNIQUE) continue;

        /* Skip "wrong" monsters */
        if (r_ptr->r_char != typ) continue;

        /* Delete the monster */
        delete_monster_idx(i);

        /* Take damage */
        take_hit(randint(4), "the strain of casting Genocide");

        /* Visual feedback */
        move_cursor_relative(py, px);
        p_ptr->redraw |= (PR_HP);
        handle_stuff();
        Term_fresh();
        delay(20 * delay_spd);
        
        /* Take note */
        result = TRUE;
    }

    return (result);
}


/*
 * Delete all nearby (non-unique) monsters
 */
bool mass_genocide(void)
{
    int		i;
    
    bool	result = FALSE;

    /* Delete the (nearby) monsters */
    for (i = 1; i < m_max; i++) {

        monster_type	*m_ptr = &m_list[i];
        monster_race	*r_ptr = &r_info[m_ptr->r_idx];

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Hack -- Skip unique monsters */
        if (r_ptr->flags1 & RF1_UNIQUE) continue;

        /* Skip distant monsters */	
        if (m_ptr->cdis > MAX_SIGHT) continue;

        /* Delete the monster */
        delete_monster_idx(i);

        /* Hack -- visual feedback */
        take_hit(randint(3), "the strain of casting Mass Genocide");
        move_cursor_relative(py, px);
        p_ptr->redraw |= (PR_HP);
        handle_stuff();
        Term_fresh();
        delay(20 * delay_spd);

        /* Note effect */
        result = TRUE;
    }

    return (result);
}



/*
 * Increase players hit points, notice effects
 */
bool hp_player(int num)
{
    if (p_ptr->chp < p_ptr->mhp) {

        p_ptr->chp += num;

        if (p_ptr->chp > p_ptr->mhp) {
            p_ptr->chp = p_ptr->mhp;
            p_ptr->chp_frac = 0;
        }

        p_ptr->redraw |= (PR_HP);

        num = num / 5;
        if (num < 3) {
            if (num == 0) {
                msg_print("You feel a little better.");
            }
            else {
                msg_print("You feel better.");
            }
        }
        else {
            if (num < 7) {
                msg_print("You feel much better.");
            }
            else {
                msg_print("You feel very good.");
            }
        }
        
        return (TRUE);
    }

    return (FALSE);
}




/*
 * Increase "protection from evil" counter
 */
bool protect_evil()
{
    bool res = FALSE;
    if (!p_ptr->protevil) res = TRUE;
    p_ptr->protevil += randint(25) + 3 * p_ptr->lev;
    if (p_ptr->protevil > 30000) p_ptr->protevil = 30000;
    return (res);
}


/*
 * Make the player no longer hungry
 */
void satisfy_hunger(void)
{
    /* Message */
    msg_print("You feel full!");

    /* No longer hungry */
    p_ptr->food = PY_FOOD_MAX - 1;
}


/*
 * Banish evil monsters
 */
bool banish_evil(int dist)
{
    int           i;
    
    bool result = FALSE;

    /* Banish all (nearby) monsters */
    for (i = 1; i < m_max; i++) {

        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Require line of sight */
        if (!player_has_los_bold(m_ptr->fy, m_ptr->fx)) continue;

        /* Teleport evil monsters away */
        if (r_ptr->flags3 & RF3_EVIL) {

            /* Memorize the flag */
            r_ptr->r_flags3 |= RF3_EVIL;

            /* Teleport it away */
            (void)teleport_away(i, dist);

            /* Note the result */
            result = TRUE;
        }
    }

    return (result);
}


/*
 * Probe nearby monsters
 */
bool probing(void)
{
    int            i;

    bool	probe = FALSE;


    /* Probe all (nearby) monsters */
    for (i = 1; i < m_max; i++) {

        monster_type *m_ptr = &m_list[i];

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Require line of sight */
        if (!player_has_los_bold(m_ptr->fy, m_ptr->fx)) continue;

        /* Probe visible monsters */
        if (m_ptr->ml) {

            char m_name[80];

            /* Start the message */
            if (!probe) msg_print("Probing...");

            /* Get "the monster" or "something" */
            monster_desc(m_name, m_ptr, 0x04);

            /* Describe the monster */
            msg_format("%^s has %d hit points.", m_name, m_ptr->hp);

            /* Learn all of the non-spell, non-treasure flags */
            lore_do_probe(i);

            /* Probe worked */
            probe = TRUE;
        }
    }

    /* Done */
    if (probe) {
        msg_print("That's all.");
    }

    /* Result */
    return (probe);
}


/*
 * Hack -- Dispel a monster, induce fear
 */
static bool dispel_monster(int m_idx, int dam)
{
    monster_type *m_ptr = &m_list[m_idx];

    char m_name[80];

    bool fear = FALSE;


    /* Paranoia */
    if (dam < 1) return (FALSE);
    
    /* Get the name */
    monster_desc(m_name, m_ptr, 0);

    /* Apply the damage, check for fear and death */
    if (mon_take_hit(m_idx, randint(dam), &fear, " dissolves!")) {

        /* Dead monster */
    }

    /* Damaged monster */
    else {

        /* Message */
        msg_format("%^s shudders.", m_name);

        /* Take note */
        if (fear && (m_ptr->ml)) {

            /* Sound */
            sound(SOUND_FLEE);

            /* Message */
            msg_format("%^s flees in terror!", m_name);
        }
    }

    /* Note the effect */
    return (TRUE);
}


/*
 * Apply damage to all nearby (evil) monsters.
 */
bool dispel_evil(int dam)
{
    int	i;
    bool dispel = FALSE;

    /* Check all nearby monsters within line of sight */
    for (i = 1; i < m_max; i++) {

        monster_type	*m_ptr = &m_list[i];
        monster_race	*r_ptr = &r_info[m_ptr->r_idx];

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Require line of sight */
        if (!player_has_los_bold(m_ptr->fy, m_ptr->fx)) continue;

        /* Only affect "evil" monsters */
        if (!(r_ptr->flags3 & RF3_EVIL)) continue;

        /* Memorize the susceptibility */
        r_ptr->r_flags3 |= RF3_EVIL;

        /* Dispel the monster, note effects */
        if (dispel_monster(i, dam)) dispel = TRUE;
    }

    /* Note effect */
    return (dispel);
}


/*
 * Apply damage to all nearby (undead) monsters.
 */
bool dispel_undead(int dam)
{
    int	i;
    bool dispel = FALSE;

    /* Check all nearby monsters within line of sight */
    for (i = 1; i < m_max; i++) {

        monster_type	*m_ptr = &m_list[i];
        monster_race	*r_ptr = &r_info[m_ptr->r_idx];

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Require line of sight */
        if (!player_has_los_bold(m_ptr->fy, m_ptr->fx)) continue;

        /* Only affect "undead" monsters */
        if (!(r_ptr->flags3 & RF3_UNDEAD)) continue;

        /* Memorize the susceptibility */
        r_ptr->r_flags3 |= RF3_UNDEAD;

        /* Dispel the monster, note effects */
        if (dispel_monster(i, dam)) dispel = TRUE;
    }

    /* Note effect */
    return (dispel);
}


/*
 * Apply damage to all nearby monsters.
 */
bool dispel_monsters(int dam)
{
    int	i;
    bool dispel = FALSE;

    /* Affect all nearby monsters within line of sight */
    for (i = 1; i < m_max; i++) {

        monster_type	*m_ptr = &m_list[i];

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Require line of sight */
        if (!player_has_los_bold(m_ptr->fy, m_ptr->fx)) continue;

        /* Dispel the monster, note effects */
        if (dispel_monster(i, dam)) dispel = TRUE;
    }

    /* Note effect */
    return (dispel);
}


/*
 * Attempt to turn (scare) undead creatures.	-RAK-	
 */
bool turn_undead(void)
{
    int		i;
    bool	result = FALSE;

    char	m_name[80];


    /* Turn undead monsters */
    for (i = 1; i < m_max; i++) {

        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Require line of sight */
        if (!player_has_los_bold(m_ptr->fy, m_ptr->fx)) continue;

        /* Only affect undead */
        if (!(r_ptr->flags3 & RF3_UNDEAD)) continue;

        /* Check saving throw */
        if (((p_ptr->lev + 1) > r_ptr->level) ||
            (rand_int(100) < 20)) {

            if (m_ptr->ml) {

                monster_desc(m_name, m_ptr, 0);
                msg_format("%^s flees in terror!", m_name);

                r_ptr->r_flags3 |= RF3_UNDEAD;

                result = TRUE;
            }

            /* Increase fear */
            m_ptr->monfear += randint(p_ptr->lev) * 2;
        }

        /* Hmmm... */
        else if (m_ptr->ml) {

            monster_desc(m_name, m_ptr, 0);
            msg_format("%^s is unaffected.", m_name);
        }
    }

    return (result);
}


/*
 * Leave a "glyph of warding" which prevents monster movement
 */
void warding_glyph(void)
{
    cave_type *c_ptr;
    
    /* Require clean space */
    if (!clean_grid_bold(py, px)) return;

    /* Access the player grid */
    c_ptr = &cave[py][px];

    /* Create a glyph of warding */
    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x03);
}




/*
 * Array of stat "descriptions"
 */
static cptr desc_stat_pos[] = {
    "strong",
    "smart",
    "wise",
    "dextrous",
    "healthy",
    "cute"
};


/*
 * Array of stat "descriptions"
 */
static cptr desc_stat_neg[] = {
    "weak",
    "stupid",
    "naive",
    "clumsy",
    "sickly",
    "ugly"
};


/*
 * Lose a "point"
 */
bool do_dec_stat(int stat)
{
    bool sust = FALSE;

    /* Access the "sustain" */
    switch (stat) {
        case A_STR: if (p_ptr->sustain_str) sust = TRUE; break;
        case A_INT: if (p_ptr->sustain_int) sust = TRUE; break;
        case A_WIS: if (p_ptr->sustain_wis) sust = TRUE; break;
        case A_DEX: if (p_ptr->sustain_dex) sust = TRUE; break;
        case A_CON: if (p_ptr->sustain_con) sust = TRUE; break;
        case A_CHR: if (p_ptr->sustain_chr) sust = TRUE; break;
    }

    /* Sustain */
    if (sust) {

        /* Message */
        msg_format("You feel %s for a moment, but the feeling passes.",
                   desc_stat_neg[stat]);

        /* Notice effect */
        return (TRUE);
    }

    /* Attempt to reduce the stat */
    if (dec_stat(stat, 10, FALSE)) {

        /* Message */
        msg_format("You feel very %s.", desc_stat_neg[stat]);

        /* Notice effect */
        return (TRUE);
    }

    /* Nothing obvious */
    return (FALSE);
}


/*
 * Restore lost "points" in a stat
 */
bool do_res_stat(int stat)
{
    /* Attempt to increase */
    if (res_stat(stat)) {

        /* Message */
        msg_format("You feel less %s.", desc_stat_neg[stat]);

        /* Notice */
        return (TRUE);
    }

    /* Nothing obvious */
    return (FALSE);
}


/*
 * Gain a "point" in a stat
 */
bool do_inc_stat(int stat)
{
    bool res;

    /* Restore strength */
    res = res_stat(stat);

    /* Attempt to increase */
    if (inc_stat(stat)) {

        /* Message */
        msg_format("Wow!  You feel very %s!", desc_stat_pos[stat]);

        /* Notice */
        return (TRUE);
    }

    /* Restoration worked */
    if (res) {

        /* Message */
        msg_format("You feel less %s.", desc_stat_neg[stat]);

        /* Notice */
        return (TRUE);
    }

    /* Nothing obvious */
    return (FALSE);
}



/*
 * Identify everything being carried.
 * Done by a potion of "self knowledge".
 */
void identify_pack(void)
{
    int                 i;
    inven_type         *i_ptr;

    /* Simply identify and know every item */
    for (i = 0; i < INVEN_TOTAL; i++) {
        i_ptr = &inventory[i];
        if (i_ptr->k_idx) {
            inven_aware(i_ptr);
            inven_known(i_ptr);
        }
    }
}


/*
 * Add to the players food time				-RAK-	
 *
 * Eating a food ration (5000 food) while totally full will
 * gorge you by 5000/50 = 100 food.  While gorged, your speed
 * is lowered by 10 points, and you digest at normal speed even
 * if resting.  A normal speed player (digestion 2) will thus
 * be slow for about 50*10 game turns, in which time he will get
 * about 25 full player moves.  Faster players will spend fewer
 * game turns gorged, but will have more player moves in that
 * same amount of time.
 */
void add_food(int num)
{
    int old, extra;

    /* Cancel starvation */
    if (p_ptr->food < 0) p_ptr->food = 0;

    /* Save the old value */
    old = p_ptr->food;

    /* Add the food */
    p_ptr->food += num;

    /* Paranoia -- Overflow check */
    if ((num > 0) && (p_ptr->food <= 0)) p_ptr->food = 32000;

    /* Check for bloating */
    if (p_ptr->food >= PY_FOOD_MAX) {

        /* Message */
        msg_print("You have gorged yourself into a bloated state!");

        /* Obtain the "bloat value" */
        extra = p_ptr->food - PY_FOOD_MAX;
        if (extra > num) extra = num;

        /* Do not get "full" effect from bloat food */
        p_ptr->food = ((old > PY_FOOD_MAX) ? old : PY_FOOD_MAX);

        /* Get a little bit bloated (slow down player) */
        p_ptr->food += extra / 50;
    }

    /* Check for "full" */
    else if (p_ptr->food >= PY_FOOD_FULL) {

        /* Message */
        msg_print("You are full.");
    }
}






/*
 * Increase "Timed Infravision"
 */
bool add_tim_infra(int amount)
{
    if (p_ptr->tim_infra < 30000) p_ptr->tim_infra += amount;
    return (TRUE);
}

/*
 * Increase "Timed See-Invisible"
 */
bool add_tim_invis(int amount)
{
    if (p_ptr->tim_invis < 30000) p_ptr->tim_invis += amount;
    return (TRUE);
}

/*
 * Increase "Timed Bless"
 */
bool add_bless(int amount)
{
    if (p_ptr->blessed < 30000) p_ptr->blessed += amount;
    return (TRUE);
}

/*
 * Increase "Timed Shield"
 */
bool add_shield(int amount)
{
    if (p_ptr->shield < 30000) p_ptr->shield += amount;
    return (TRUE);
}

/*
 * Increase "Timed Slowness"
 */
bool add_slow(int amount)
{
    if (p_ptr->slow < 30000) p_ptr->slow += amount;
    return (TRUE);
}

/*
 * Increase "Timed Speed"
 */
bool add_fast(int amount)
{
    if (p_ptr->fast < 30000) p_ptr->fast += amount;
    return (TRUE);
}

/*
 * Increase "Timed Blindness" (if allowed)
 */
bool add_blind(int amount)
{
    if (p_ptr->resist_blind) return (FALSE);
    if (p_ptr->blind < 30000) p_ptr->blind += amount;
    return (TRUE);
}

/*
 * Increase "Timed Hallucination" (if allowed)
 */
bool add_image(int amount)
{
    if (p_ptr->image < 30000) p_ptr->image += amount;
    return (TRUE);
}

/*
 * Increase "Timed Poison" (if allowed)
 */
bool add_poisoned(int amount)
{
    if (p_ptr->immune_pois) return (FALSE);
    if (p_ptr->resist_pois) return (FALSE);
    if (p_ptr->oppose_pois) return (FALSE);
    if (p_ptr->poisoned < 30000) p_ptr->poisoned += amount;
    return (TRUE);
}

/*
 * Increase "Timed Confusion" (if allowed)
 */
bool add_confused(int amount)
{
    if (p_ptr->resist_conf) return (FALSE);
    if (p_ptr->resist_chaos) return (FALSE);
    if (p_ptr->confused < 30000) p_ptr->confused += amount;
    return (TRUE);
}

/*
 * Increase "Timed Fear" (if allowed)
 */
bool add_fear(int amount)
{
    if (p_ptr->hero || p_ptr->shero) return (FALSE);
    if (p_ptr->resist_fear) return (FALSE);
    if (p_ptr->fear < 30000) p_ptr->fear += amount;
    return (TRUE);
}

/*
 * Increase "Timed Paralysis" (if allowed)
 */
bool add_paralysis(int amount)
{
    if (p_ptr->free_act) return (FALSE);
    if (p_ptr->paralysis < 30000) p_ptr->paralysis += amount;
    return (TRUE);
}





/*
 * Used by the "enchant" function (chance of failure)
 */
static int enchant_table[16] = {
   0, 10,  50, 100, 200,
   300, 400, 500, 700, 950,
   990, 992, 995, 997, 999,
   1000
};


/*
 * Dump a message describing a monster's reaction to damage
 *
 * Technically should attempt to treat "Beholder"'s as jelly's
 */
void message_pain(int m_idx, int dam)
{
    long			oldhp, newhp, tmp;
    int				percentage;

    monster_type		*m_ptr = &m_list[m_idx];
    monster_race		*r_ptr = &r_info[m_ptr->r_idx];

    char			m_name[80];


    /* Get the monster name */
    monster_desc(m_name, m_ptr, 0);

    /* Notice non-damage */
    if (dam == 0) {
        msg_format("%^s is unharmed.", m_name);
        return;
    }

    /* Note -- subtle fix -CFT */
    newhp = (long)(m_ptr->hp);
    oldhp = newhp + (long)(dam);
    tmp = (newhp * 100L) / oldhp;
    percentage = (int)(tmp);


    /* Jelly's and Mold's and Quthl's */
    if (strchr("jmvQ", r_ptr->r_char)) {

        if (percentage > 95)
            msg_format("%^s barely notices.", m_name);
        else if (percentage > 75)
            msg_format("%^s flinches.", m_name);
        else if (percentage > 50)
            msg_format("%^s squelches.", m_name);
        else if (percentage > 35)
            msg_format("%^s quivers in pain.", m_name);
        else if (percentage > 20)
            msg_format("%^s writhes about.", m_name);
        else if (percentage > 10)
            msg_format("%^s writhes in agony.", m_name);
        else
            msg_format("%^s jerks limply.", m_name);
    }

    /* Dogs and Hounds */
    else if (strchr("CZ", r_ptr->r_char)) {

        if (percentage > 95)
            msg_format("%^s shrugs off the attack.", m_name);
        else if (percentage > 75)
            msg_format("%^s snarls with pain.", m_name);
        else if (percentage > 50)
            msg_format("%^s yelps in pain.", m_name);
        else if (percentage > 35)
            msg_format("%^s howls in pain.", m_name);
        else if (percentage > 20)
            msg_format("%^s howls in agony.", m_name);
        else if (percentage > 10)
            msg_format("%^s writhes in agony.", m_name);
        else
            msg_format("%^s yelps feebly.", m_name);
    }

    /* One type of monsters (ignore,squeal,shriek) */
    else if (strchr("KcaUqRXbFJlrsSt", r_ptr->r_char)) {

        if (percentage > 95)
            msg_format("%^s ignores the attack.", m_name);
        else if (percentage > 75)
            msg_format("%^s grunts with pain.", m_name);
        else if (percentage > 50)
            msg_format("%^s squeals in pain.", m_name);
        else if (percentage > 35)
            msg_format("%^s shrieks in pain.", m_name);
        else if (percentage > 20)
            msg_format("%^s shrieks in agony.", m_name);
        else if (percentage > 10)
            msg_format("%^s writhes in agony.", m_name);
        else
            msg_format("%^s cries out feebly.", m_name);
    }

    /* Another type of monsters (shrug,cry,scream) */
    else {

        if (percentage > 95)
            msg_format("%^s shrugs off the attack.", m_name);
        else if (percentage > 75)
            msg_format("%^s grunts with pain.", m_name);
        else if (percentage > 50)
            msg_format("%^s cries out in pain.", m_name);
        else if (percentage > 35)
            msg_format("%^s screams in pain.", m_name);
        else if (percentage > 20)
            msg_format("%^s screams in agony.", m_name);
        else if (percentage > 10)
            msg_format("%^s writhes in agony.", m_name);
        else
            msg_format("%^s cries out feebly.", m_name);
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
    int		i, cnt = 0;

    /* Attempt to uncurse items being worn */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {

        u32b f1, f2, f3;

        inven_type *i_ptr = &inventory[i];

        /* Uncursed already */
        if (!cursed_p(i_ptr)) continue;

        /* Extract the flags */
        inven_flags(i_ptr, &f1, &f2, &f3);

        /* Heavily Cursed Items need a special spell */
        if (!all && (f3 & TR3_HEAVY_CURSE)) continue;

        /* Perma-Cursed Items can NEVER be uncursed */
        if (f3 & TR3_PERMA_CURSE) continue;

        /* Uncurse it */
        i_ptr->ident &= ~ID_CURSED;

        /* Hack -- Assume felt */
        i_ptr->ident |= ID_SENSE;

        /* Take note */
        i_ptr->note = quark_add("uncursed");

        /* Redraw the choice window */
        p_ptr->redraw |= (PR_CHOOSE);

        /* Recalculate the bonuses */
        p_ptr->update |= (PU_BONUS);

        /* Count the uncursings */
        cnt++;
    }

    /* Return "something uncursed" */
    return (cnt);
}


/*
 * Remove most curses
 */
bool remove_curse()
{
    return (remove_curse_aux(FALSE));
}

/*
 * Remove all curses
 */
bool remove_all_curse()
{
    return (remove_curse_aux(TRUE));
}



/*
 * Restores any drained experience
 */
bool restore_level()
{
    /* Restore experience */
    if (p_ptr->exp < p_ptr->max_exp) {

        /* Message */
        msg_print("You feel your life energies returning.");

        /* Restore the experience */
        p_ptr->exp = p_ptr->max_exp;

        /* Check the experience */
        check_experience();

        /* Did something */
        return (TRUE);
    }

    /* No effect */
    return (FALSE);
}


/*
 * self-knowledge... idea from nethack.  Useful for determining powers and
 * resistences of items.  It saves the screen, clears it, then starts listing
 * attributes, a screenful at a time.  (There are a LOT of attributes to
 * list.  It will probably take 2 or 3 screens for a powerful character whose
 * using several artifacts...) -CFT
 *
 * It is now a lot more efficient. -BEN-
 *
 * See also "identify_fully()".
 */
void self_knowledge()
{
    int		i = 0, j, k;

    u32b	f1 = 0L, f2 = 0L, f3 = 0L;

    inven_type	*i_ptr;

    cptr	info[128];


    /* Acquire item flags from equipment */
    for (k = INVEN_WIELD; k < INVEN_TOTAL; k++) {

        u32b t1, t2, t3;

        i_ptr = &inventory[k];

        /* Skip empty items */
        if (!i_ptr->k_idx) continue;

        /* Extract the flags */
        inven_flags(i_ptr, &t1, &t2, &t3);

        /* Extract flags */
        f1 |= t1;
        f2 |= t2;
        f3 |= t3;
    }


    if (p_ptr->blind) {
        info[i++] = "You cannot see.";
    }
    if (p_ptr->confused) {
        info[i++] = "You are confused.";
    }
    if (p_ptr->fear) {
        info[i++] = "You are terrified.";
    }
    if (p_ptr->cut) {
        info[i++] = "You are bleeding.";
    }
    if (p_ptr->stun) {
        info[i++] = "You are stunned.";
    }
    if (p_ptr->poisoned) {
        info[i++] = "You are poisoned.";
    }
    if (p_ptr->image) {
        info[i++] = "You are hallucinating.";
    }

    if (p_ptr->aggravate) {
        info[i++] = "You aggravate monsters.";
    }
    if (p_ptr->teleport) {
        info[i++] = "Your position is very uncertain.";
    }

    if (p_ptr->blessed) {
        info[i++] = "You feel rightous.";
    }
    if (p_ptr->hero) {
        info[i++] = "You feel heroic.";
    }
    if (p_ptr->shero) {
        info[i++] = "You are in a battle rage.";
    }
    if (p_ptr->protevil) {
        info[i++] = "You are protected from evil.";
    }
    if (p_ptr->shield) {
        info[i++] = "You are protected by a mystic shield.";
    }
    if (p_ptr->invuln) {
        info[i++] = "You are temporarily invulnerable.";
    }
    if (p_ptr->confusing) {
        info[i++] = "Your hands are glowing dull red.";
    }
    if (p_ptr->searching) {
        info[i++] = "You are looking around very carefully.";
    }
    if (p_ptr->new_spells) {
        info[i++] = "You can learn some more spells.";
    }
    if (p_ptr->word_recall) {
        info[i++] = "You will soon be recalled.";
    }
    if (p_ptr->see_infra) {
        info[i++] = "Your eyes are sensitive to infrared light.";
    }
    if (p_ptr->see_inv) {
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
    if (p_ptr->resist_shard) {
        info[i++] = "You are resistant to blasts of shards.";
    }
    if (p_ptr->resist_nexus) {
        info[i++] = "You are resistant to nexus attacks.";
    }
    if (p_ptr->resist_neth) {
        info[i++] = "You are resistant to nether forces.";
    }
    if (p_ptr->resist_fear) {
        info[i++] = "You are completely fearless.";
    }
    if (p_ptr->resist_blind) {
        info[i++] = "Your eyes are resistant to blindness.";
    }

    if (p_ptr->sustain_str) {
        info[i++] = "Your strength is sustained.";
    }
    if (p_ptr->sustain_int) {
        info[i++] = "Your intelligence is sustained.";
    }
    if (p_ptr->sustain_wis) {
        info[i++] = "Your wisdom is sustained.";
    }
    if (p_ptr->sustain_con) {
        info[i++] = "Your constitution is sustained.";
    }
    if (p_ptr->sustain_dex) {
        info[i++] = "Your dexterity is sustained.";
    }
    if (p_ptr->sustain_chr) {
        info[i++] = "Your charisma is sustained.";
    }

    if (f1 & TR1_STR) {
        info[i++] = "Your strength is affected by your equipment.";
    }
    if (f1 & TR1_INT) {
        info[i++] = "Your intelligence is affected by your equipment.";
    }
    if (f1 & TR1_WIS) {
        info[i++] = "Your wisdom is affected by your equipment.";
    }
    if (f1 & TR1_DEX) {
        info[i++] = "Your dexterity is affected by your equipment.";
    }
    if (f1 & TR1_CON) {
        info[i++] = "Your constitution is affected by your equipment.";
    }
    if (f1 & TR1_CHR) {
        info[i++] = "Your charisma is affected by your equipment.";
    }

    if (f1 & TR1_STEALTH) {
        info[i++] = "Your stealth is affected by your equipment.";
    }
    if (f1 & TR1_SEARCH) {
        info[i++] = "Your searching ability is affected by your equipment.";
    }
    if (f1 & TR1_INFRA) {
        info[i++] = "Your infravision is affected by your equipment.";
    }
    if (f1 & TR1_TUNNEL) {
        info[i++] = "Your digging ability is affected by your equipment.";
    }
    if (f1 & TR1_SPEED) {
        info[i++] = "Your speed is affected by your equipment.";
    }
    if (f1 & TR1_BLOWS) {
        info[i++] = "Your attack speed is affected by your equipment.";
    }


    /* Access the current weapon */
    i_ptr = &inventory[INVEN_WIELD];

    /* Analyze the weapon */
    if (i_ptr->k_idx) {

        /* Indicate Blessing */
        if (f3 & TR3_BLESSED) {
            info[i++] = "Your weapon has been blessed by the gods.";
        }

        /* Hack */
        if (f1 & TR1_IMPACT) {
            info[i++] = "The unbelievable impact of your weapon can cause earthquakes.";
        }

        /* Special "Attack Bonuses" */
        if (f1 & TR1_BRAND_ACID) {
            info[i++] = "Your weapon melts your foes.";
        }
        if (f1 & TR1_BRAND_ELEC) {
            info[i++] = "Your weapon shocks your foes.";
        }
        if (f1 & TR1_BRAND_FIRE) {
            info[i++] = "Your weapon burns your foes.";
        }
        if (f1 & TR1_BRAND_COLD) {
            info[i++] = "Your weapon freezes your foes.";
        }
        
        /* Special "slay" flags */
        if (f1 & TR1_SLAY_ANIMAL) {
            info[i++] = "Your weapon strikes at animals with extra force.";
        }
        if (f1 & TR1_SLAY_EVIL) {
            info[i++] = "Your weapon strokes at evil with extra force.";
        }
        if (f1 & TR1_SLAY_UNDEAD) {
            info[i++] = "Your weapon strikes at undead with holy wrath.";
        }
        if (f1 & TR1_SLAY_DEMON) {
            info[i++] = "Your weapon strikes at demons with holy wrath.";
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
        if (f1 & TR1_SLAY_DRAGON) {
            info[i++] = "Your weapon is especially deadly against dragons.";
        }

        /* Special "kill" flags */
        if (f1 & TR1_KILL_DRAGON) {
            info[i++] = "Your weapon is a great bane of dragons.";
        }
    }


    /* Save the screen */
    Term_save();

    /* Erase the screen */
    for (k = 1; k < 24; k++) prt("", k, 13);

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
            for ( ; k > 2; k--) prt("", k, 15);
        }
    }

    /* Pause */
    prt("[Press any key to continue]", k, 13);
    inkey();

    /* Restore the screen */
    Term_load();
}





/*
 * Teleport the player one level up or down (random when legal)
 */
void tele_level()
{
    if (!dun_level) {
        msg_print("You sink through the floor.");
        dun_level++;
        new_level_flag = TRUE;
    }
    else if (is_quest(dun_level) || (dun_level >= MAX_DEPTH-1)) {
        msg_print("You rise up through the ceiling.");
        dun_level--;
        new_level_flag = TRUE;
    }
    else if (rand_int(100) < 50) {
        msg_print("You rise up through the ceiling.");
        dun_level--;
        new_level_flag = TRUE;
    }
    else {
        msg_print("You sink through the floor.");
        dun_level++;
        new_level_flag = TRUE;
    }
}





/*
 * Forget everything
 */
bool lose_all_info(void)
{
    int                 i;

    /* Forget info about objects */
    for (i = 0; i < INVEN_TOTAL; i++) {

        inven_type *i_ptr = &inventory[i];

        /* Skip non-items */
        if (!i_ptr->k_idx) continue;

        /* Allow "protection" by the MENTAL flag */
        if (i_ptr->ident & ID_MENTAL) continue;

        /* Hack -- Clear the "empty" flag */
        i_ptr->ident &= ~ID_EMPTY;

        /* Hack -- Clear the "known" flag */
        i_ptr->ident &= ~ID_KNOWN;

        /* Hack -- Clear the "felt" flag */
        i_ptr->ident &= ~ID_SENSE;
    }

    /* Redraw the choice window */
    p_ptr->redraw |= (PR_CHOOSE);

    /* Mega-Hack -- Forget the map */
    wiz_dark();

    /* It worked */
    return (TRUE);
}


/*
 * Detect any treasure on the current panel		-RAK-	
 *
 * We do not yet create any "hidden gold" features XXX XXX XXX
 */
bool detect_treasure(void)
{
    int		y, x;
    bool	detect = FALSE;
    cave_type	*c_ptr;

    /* Scan the current panel */
    for (y = panel_row_min; y <= panel_row_max; y++) {
        for (x = panel_col_min; x <= panel_col_max; x++) {

            /* Access the grid */
            c_ptr = &cave[y][x];

            /* Magma/Quartz + Known Gold */
            if (((c_ptr->feat & 0x3F) == 0x36) ||
                ((c_ptr->feat & 0x3F) == 0x37)) {

                /* Notice detected gold */
                if (!test_lite_bold(y, x)) detect = TRUE;

                /* Hack -- memorize the feature */
                c_ptr->feat |= CAVE_MARK;

                /* Redraw */
                lite_spot(y, x);
            }

#if 0

            /* Notice embedded gold */
            if (((c_ptr->feat & 0x3F) == 0x34) ||
                ((c_ptr->feat & 0x3F) == 0x35)) {

                /* Expose the gold */
                c_ptr->feat += 0x02;

                /* Detect */
                detect = TRUE;

                /* Hack -- memorize the item */
                c_ptr->feat |= CAVE_MARK;

                /* Redraw */
                lite_spot(y, x);
            }
#endif

            /* Notice gold */
            if (i_list[c_ptr->i_idx].tval == TV_GOLD) {

                /* Notice new items */
                if (!test_lite_bold(y, x)) detect = TRUE;

                /* Hack -- memorize the item */
                c_ptr->feat |= CAVE_MARK;

                /* Redraw */
                lite_spot(y, x);
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
bool detect_magic()
{
    int		i, j, tv;
    bool	detect = FALSE;

    cave_type	*c_ptr;
    inven_type	*i_ptr;


    /* Scan the current panel */
    for (i = panel_row_min; i <= panel_row_max; i++) {
        for (j = panel_col_min; j <= panel_col_max; j++) {

            /* Access the grid and object */
            c_ptr = &cave[i][j];
            i_ptr = &i_list[c_ptr->i_idx];

            /* Nothing there */
            if (!(c_ptr->i_idx)) continue;

            /* Examine the tval */
            tv = i_ptr->tval;

            /* Artifacts, misc magic items, or enchanted wearables */
            if (artifact_p(i_ptr) ||
                (tv == TV_AMULET) || (tv == TV_RING) ||
                (tv == TV_STAFF) || (tv == TV_WAND) || (tv == TV_ROD) ||
                (tv == TV_SCROLL) || (tv == TV_POTION) ||
                (tv == TV_MAGIC_BOOK) || (tv == TV_PRAYER_BOOK) ||
                ((i_ptr->to_a > 0) || (i_ptr->to_h + i_ptr->to_d > 0))) {

                /* Note new items */
                if (!test_lite_bold(i, j)) detect = TRUE;

                /* Hack -- memorize the item */
                c_ptr->feat |= CAVE_MARK;

                /* Redraw */
                lite_spot(i, j);
            }
        }
    }

    /* Return result */
    return (detect);
}





/*
 * Locates and displays all invisible creatures on current panel -RAK-
 */
bool detect_invisible()
{
    int		i;
    bool	flag = FALSE;


    /* Detect all invisible monsters */
    for (i = 1; i < m_max; i++) {

        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        int fy = m_ptr->fy;
        int fx = m_ptr->fx;

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Skip visible monsters */
        if (m_ptr->ml) continue;

        /* Detect all invisible monsters */
        if (panel_contains(fy, fx) && (r_ptr->flags2 & RF2_INVISIBLE)) {

            /* Take note that they are invisible */
            r_ptr->r_flags2 |= RF2_INVISIBLE;

            /* Mega-Hack -- Show the monster */
            m_ptr->ml = TRUE;
            lite_spot(fy, fx);
            flag = TRUE;
        }
    }

    /* Describe result, and clean up */
    if (flag) {

        /* Describe, and wait for acknowledgement */
        msg_print("You sense the presence of invisible creatures!");
        msg_print(NULL);

        /* Mega-Hack -- Fix the monsters */
        update_monsters(FALSE);
    }

    /* Result */
    return (flag);
}



/*
 * Display evil creatures on current panel		-RAK-	
 */
bool detect_evil(void)
{
    int		i;
    bool	flag = FALSE;


    /* Display all the evil monsters */
    for (i = 1; i < m_max; i++) {

        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        int fy = m_ptr->fy;
        int fx = m_ptr->fx;

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Skip visible monsters */
        if (m_ptr->ml) continue;

        /* Detect evil monsters */
        if (panel_contains(fy, fx) && (r_ptr->flags3 & RF3_EVIL)) {

            /* Mega-Hack -- Show the monster */
            m_ptr->ml = TRUE;
            lite_spot(fy, fx);
            flag = TRUE;
        }
    }

    /* Note effects and clean up */
    if (flag) {

        /* Describe, and wait for acknowledgement */
        msg_print("You sense the presence of evil!");
        msg_print(NULL);

        /* Mega-Hack -- Fix the monsters */
        update_monsters(FALSE);
    }

    /* Result */
    return (flag);
}



/*
 * Display all non-invisible monsters on the current panel
 */
bool detect_monsters(void)
{
    int		i;
    bool	flag = FALSE;


    /* Detect non-invisible monsters */
    for (i = 1; i < m_max; i++) {

        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        int fy = m_ptr->fy;
        int fx = m_ptr->fx;

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Skip visible monsters */
        if (m_ptr->ml) continue;

        /* Detect all non-invisible monsters */
        if (panel_contains(fy, fx) && (!(r_ptr->flags2 & RF2_INVISIBLE))) {

            /* Mega-Hack -- Show the monster */
            m_ptr->ml = TRUE;
            lite_spot(fy, fx);
            flag = TRUE;
        }
    }

    /* Describe and clean up */
    if (flag) {

        /* Describe, and wait for acknowledgement */
        msg_print("You sense the presence of monsters!");
        msg_print(NULL);

        /* Mega-Hack -- Fix the monsters */
        update_monsters(FALSE);
    }

    /* Result */
    return (flag);
}


/*
 * Detect everything
 */
bool detection(void)
{
    int		i;
    bool	flag = FALSE;
    bool	detect = FALSE;


    /* Detect the easy things */
    if (detect_treasure()) detect = TRUE;
    if (detect_object()) detect = TRUE;
    if (detect_trap()) detect = TRUE;
    if (detect_sdoor()) detect = TRUE;


    /* Detect all monsters in the current panel */
    for (i = 1; i < m_max; i++) {

        monster_type *m_ptr = &m_list[i];

        int fy = m_ptr->fy;
        int fx = m_ptr->fx;

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Skip visible monsters */
        if (m_ptr->ml) continue;

        /* Detect all monsters */
        if (panel_contains(fy, fx)) {

            /* Mega-Hack -- Show the monster */
            m_ptr->ml = TRUE;
            lite_spot(fy, fx);
            flag = detect = TRUE;
        }
    }

    /* Describe the result, then fix the monsters */
    if (flag) {

        /* Describe, and wait for acknowledgement */
        msg_print("You sense the presence of monsters!");
        msg_print(NULL);

        /* Mega-Hack -- Fix the monsters */
        update_monsters(FALSE);
    }

    /* Result */
    return (detect);
}


/*
 * Detect all objects on the current panel		-RAK-	
 */
bool detect_object(void)
{
    int		i, j;
    bool	detect = FALSE;

    /* Scan the current panel */
    for (i = panel_row_min; i <= panel_row_max; i++) {
        for (j = panel_col_min; j <= panel_col_max; j++) {

            /* Access the grid */
            cave_type *c_ptr = &cave[i][j];

            /* Nothing here */
            if (!(c_ptr->i_idx)) continue;

	    /* Do not detect "gold" */
            if (i_list[c_ptr->i_idx].tval == TV_GOLD) continue;

            /* Note new objects */
            if (!test_lite_bold(i,j)) detect = TRUE;

            /* Hack -- memorize it */
            c_ptr->feat |= CAVE_MARK;

            /* Redraw */
            lite_spot(i, j);
        }
    }

    return (detect);
}


/*
 * Locates and displays traps on current panel		-RAK-	
 */
bool detect_trap(void)
{
    int		i, j;
    bool	detect = FALSE;

    cave_type  *c_ptr;
    inven_type *i_ptr;


    /* Scan the current panel */
    for (i = panel_row_min; i <= panel_row_max; i++) {
        for (j = panel_col_min; j <= panel_col_max; j++) {

            /* Access the grid and object */
            c_ptr = &cave[i][j];

	    /* Get the object */
            i_ptr = &i_list[c_ptr->i_idx];

            /* Detect invisible traps */
            if ((c_ptr->feat & 0x3F) == 0x02) {

                /* Hack -- Pick a trap XXX XXX XXX */
                c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x10) + rand_int(16);
                
                /* Hack -- memorize it */
                c_ptr->feat |= CAVE_MARK;

                /* Redraw */
                lite_spot(i, j);

                /* Obvious */
                detect = TRUE;
            }

            /* Hack -- Identify chests */
            else if (i_ptr->tval == TV_CHEST) {

                /* Identify the chest */
                inven_known(i_ptr);
            }
        }
    }

    return (detect);
}



/*
 * Locates and displays all stairs and secret doors on current panel -RAK-	
 */
bool detect_sdoor()
{
    int		i, j;
    bool	detect = FALSE;

    cave_type *c_ptr;


    /* Scan the panel */
    for (i = panel_row_min; i <= panel_row_max; i++) {
        for (j = panel_col_min; j <= panel_col_max; j++) {

            /* Access the grid and object */
            c_ptr = &cave[i][j];

            /* Hack -- detect secret doors */
            if ((c_ptr->feat & 0x3F) == 0x30) {
            
                /* Find the door XXX XXX XXX */
                c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x20);
                
                /* Memorize the door */
                c_ptr->feat |= CAVE_MARK;

                /* Redraw */
                lite_spot(i, j);

                /* Obvious */
                detect = TRUE;
            }

            /* Ignore known grids */
            if (c_ptr->feat & CAVE_MARK) continue;
                                        
            /* Hack -- detect stairs */
            if (((c_ptr->feat & 0x3F) == 0x06) ||
                ((c_ptr->feat & 0x3F) == 0x07)) {

                /* Memorize the stairs */
                c_ptr->feat |= CAVE_MARK;

                /* Redraw */
                lite_spot(i, j);

                /* Obvious */
                detect = TRUE;
            }
        }
    }

    return (detect);
}


/*
 * Create stairs at the player location
 */
void stair_creation()
{
    /* Access the grid */
    cave_type *c_ptr;
    
    /* Access the player grid */
    c_ptr = &cave[py][px];

    /* XXX XXX XXX */
    if (!valid_grid(py, px)) {
        msg_print("The object resists the spell.");
        return;
    }

    /* Hack -- Delete old contents */
    delete_object(py, px);

    /* Create a staircase */
    if (!dun_level) {
        c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x07);
    }
    else if (is_quest(dun_level) || (dun_level >= MAX_DEPTH-1)) {
        c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x06);
    }
    else if (rand_int(100) < 50) {
        c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x06);
    }
    else {
        c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x07);
    }
}




/*
 * Determine the "Activation" (if any) for an artifact
 * Return a string, or NULL for "no activation"
 */
cptr item_activation(inven_type *i_ptr)
{
    u32b f1, f2, f3;

    /* Extract the flags */
    inven_flags(i_ptr, &f1, &f2, &f3);
    
    /* Require activation ability */
    if (!(f3 & TR3_ACTIVATE)) return (NULL);

    /* Some artifacts can be activated */
    switch (i_ptr->name1) {

        case ART_NARTHANC:
            return "fire bolt (9d8) every 8+d8 turns";
        case ART_NIMTHANC:
            return "frost bolt (6d8) every 7+d7 turns";
        case ART_DETHANC:
            return "lightning bolt (4d8) every 6+d6 turns";
        case ART_RILIA:
            return "stinking cloud (12) every 4+d4 turns";
        case ART_BELANGIL:
            return "frost ball (48) every 5+d5 turns";
        case ART_DAL:
            return "remove fear and cure poison every 5 turns";
        case ART_RINGIL:
            return "frost ball (100) every 300 turns";
        case ART_ANDURIL:
            return "fire ball (72) every 400 turns";
        case ART_FIRESTAR:
            return "large fire ball (72) every 100 turns";
        case ART_FEANOR:
            return "haste self (20+d20 turns) every 200 turns";
        case ART_THEODEN:
            return "drain life (120) every 400 turns";
        case ART_TURMIL:
            return "drain life (90) every 70 turns";
        case ART_CASPANION:
            return "door and trap destruction every 10 turns";
        case ART_AVAVIR:
            return "word of recall every 200 turns";
        case ART_TARATOL:
            return "haste self (20+d20 turns) every 100+d100 turns";
        case ART_ERIRIL:
            return "identify every 10 turns";
        case ART_OLORIN:
            return "probing every 20 turns";
        case ART_EONWE:
            return "mass genocide every 1000 turns";
        case ART_LOTHARANG:
            return "cure wounds (4d7) every 3+d3 turns";
        case ART_CUBRAGOL:
            return "fire branding of bolts every 999 turns";
        case ART_ARUNRUTH:
            return "frost bolt (12d8) every 500 turns";
        case ART_AEGLOS:
            return "frost ball (100) every 500 turns";
        case ART_OROME:
            return "stone to mud every 5 turns";
        case ART_SOULKEEPER:
            return "heal (1000) every 888 turns";
        case ART_BELEGENNON:
            return "phase door every 2 turns";
        case ART_CELEBORN:
            return "genocide every 500 turns";
        case ART_LUTHIEN:
            return "restore life levels every 450 turns";
        case ART_ULMO:
            return "teleport away every 150 turns";
        case ART_COLLUIN:
            return "resistance (20+d20 turns) every 111 turns";
        case ART_HOLCOLLETH:
            return "Sleep II every 55 turns";
        case ART_THINGOL:
            return "recharge item I every 70 turns";
        case ART_COLANNON:
            return "teleport every 45 turns";
        case ART_TOTILA:
            return "confuse monster every 15 turns";
        case ART_CAMMITHRIM:
            return "magic missile (2d6) every 2 turns";
        case ART_PAURHACH:
            return "fire bolt (9d8) every 8+d8 turns";
        case ART_PAURNIMMEN:
            return "frost bolt (6d8) every 7+d7 turns";
        case ART_PAURAEGEN:
            return "lightning bolt (4d8) every 6+d6 turns";
        case ART_PAURNEN:
            return "acid bolt (5d8) every 5+d5 turns";
        case ART_FINGOLFIN:
            return "a magical arrow (150) every 90+d90 turns";
        case ART_HOLHENNETH:
            return "detection every 55+d55 turns";
        case ART_GONDOR:
            return "heal (500) every 500 turns";
        case ART_RAZORBACK:
            return "star ball (150) every 1000 turns";
        case ART_BLADETURNER:
            return "berserk rage, bless, and resistance every 400 turns";
        case ART_GALADRIEL:
            return "illumination every 10+d10 turns";
        case ART_ELENDIL:
            return "magic mapping every 50+d50 turns";
        case ART_THRAIN:
            return "clairvoyance every 100+d100 turns";
        case ART_INGWE:
            return "dispel evil (x5) every 300+d300 turns";
        case ART_CARLAMMAS:
            return "protection from evil every 225+d225 turns";
        case ART_TULKAS:
            return "haste self (75+d75 turns) every 150+d150 turns";
        case ART_NARYA:
            return "large fire ball (120) every 225+d225 turns";
        case ART_NENYA:
            return "large frost ball (200) every 325+d325 turns";
        case ART_VILYA:
            return "large lightning ball (250) every 425+d425 turns";
        case ART_POWER:
            return "bizarre things every 450+d450 turns";
    }


    /* Require dragon scale mail */
    if (i_ptr->tval != TV_DRAG_ARMOR) return (NULL);

    /* Branch on the sub-type */
    switch (i_ptr->sval) {

        case SV_DRAGON_BLUE:
            return "breathe lightning (100) every 450+d450 turns";
        case SV_DRAGON_WHITE:
            return "breathe frost (110) every 450+d450 turns";
        case SV_DRAGON_BLACK:
            return "breathe acid (130) every 450+d450 turns";
        case SV_DRAGON_GREEN:
            return "breathe poison gas (150) every 450+d450 turns";
        case SV_DRAGON_RED:
            return "breathe fire (200) every 450+d450 turns";
        case SV_DRAGON_MULTIHUED:
            return "breathe multi-hued (250) every 225+d225 turns";
        case SV_DRAGON_BRONZE:
            return "breathe confusion (120) every 450+d450 turns";
        case SV_DRAGON_GOLD:
            return "breathe sound (130) every 450+d450 turns";
        case SV_DRAGON_CHAOS:
            return "breathe chaos/disenchant (220) every 300+d300 turns";
        case SV_DRAGON_LAW:
            return "breathe sound/shards (230) every 300+d300 turns";
        case SV_DRAGON_BALANCE:
            return "You breathe balance (250) every 300+d300 turns";
        case SV_DRAGON_SHINING:
            return "breathe light/darkness (200) every 300+d300 turns";
        case SV_DRAGON_POWER:
            return "breathe the elements (300) every 300+d300 turns";
    }


    /* Oops */
    return NULL;
}


/*
 * Describe a "fully identified" item
 */
bool identify_fully_aux(inven_type *i_ptr)
{
    int			i = 0, j, k;

    u32b f1, f2, f3;

    cptr		info[128];


    /* Extract the flags */
    inven_flags(i_ptr, &f1, &f2, &f3);
        

    /* Mega-Hack -- describe activation */
    if (f3 & TR3_ACTIVATE) {
        info[i++] = "It can be activated for...";
        info[i++] = item_activation(i_ptr);
        info[i++] = "...if it is being worn.";
    }


    /* Hack -- describe lite's */
    if (i_ptr->tval == TV_LITE) {

        if (artifact_p(i_ptr)) {
            info[i++] = "It provides light (radius 3) forever.";
        }
        else if (i_ptr->sval == SV_LITE_LANTERN) {
            info[i++] = "It provides light (radius 2) when fueled.";
        }
        else {
            info[i++] = "It provides light (radius 1) when fueled.";
        }
    }


    /* And then describe it fully */

    if (f1 & TR1_STR) {
        info[i++] = "It affects your strength.";
    }
    if (f1 & TR1_INT) {
        info[i++] = "It affects your intelligence.";
    }
    if (f1 & TR1_WIS) {
        info[i++] = "It affects your wisdom.";
    }
    if (f1 & TR1_DEX) {
        info[i++] = "It affects your dexterity.";
    }
    if (f1 & TR1_CON) {
        info[i++] = "It affects your constitution.";
    }
    if (f1 & TR1_CHR) {
        info[i++] = "It affects your charisma.";
    }

    if (f1 & TR1_STEALTH) {
        info[i++] = "It affects your stealth.";
    }
    if (f1 & TR1_SEARCH) {
        info[i++] = "It affects your searching.";
    }
    if (f1 & TR1_INFRA) {
        info[i++] = "It affects your infravision.";
    }
    if (f1 & TR1_TUNNEL) {
        info[i++] = "It affects your ability to tunnel.";
    }
    if (f1 & TR1_SPEED) {
        info[i++] = "It affects your speed.";
    }
    if (f1 & TR1_BLOWS) {
        info[i++] = "It affects your attack speed.";
    }

    if (f1 & TR1_BRAND_ACID) {
        info[i++] = "It does extra damage from acid.";
    }
    if (f1 & TR1_BRAND_ELEC) {
        info[i++] = "It does extra damage from electricity.";
    }
    if (f1 & TR1_BRAND_FIRE) {
        info[i++] = "It does extra damage from fire.";
    }
    if (f1 & TR1_BRAND_COLD) {
        info[i++] = "It does extra damage from frost.";
    }

    if (f1 & TR1_IMPACT) {
        info[i++] = "It can cause earthquakes.";
    }

    if (f1 & TR1_KILL_DRAGON) {
        info[i++] = "It is a great bane of dragons.";
    }
    else if (f1 & TR1_SLAY_DRAGON) {
        info[i++] = "It is especially deadly against dragons.";
    }
    if (f1 & TR1_SLAY_ORC) {
        info[i++] = "It is especially deadly against orcs.";
    }
    if (f1 & TR1_SLAY_TROLL) {
        info[i++] = "It is especially deadly against trolls.";
    }
    if (f1 & TR1_SLAY_GIANT) {
        info[i++] = "It is especially deadly against giants.";
    }
    if (f1 & TR1_SLAY_DEMON) {
        info[i++] = "It strikes at demons with holy wrath.";
    }
    if (f1 & TR1_SLAY_UNDEAD) {
        info[i++] = "It strikes at undead with holy wrath.";
    }
    if (f1 & TR1_SLAY_EVIL) {
        info[i++] = "It fights against evil with holy fury.";
    }
    if (f1 & TR1_SLAY_ANIMAL) {
        info[i++] = "It is especially deadly against natural creatures.";
    }

    if (f2 & TR2_SUST_STR) {
        info[i++] = "It sustains your strength.";
    }
    if (f2 & TR2_SUST_INT) {
        info[i++] = "It sustains your intelligence.";
    }
    if (f2 & TR2_SUST_WIS) {
        info[i++] = "It sustains your wisdom.";
    }
    if (f2 & TR2_SUST_DEX) {
        info[i++] = "It sustains your dexterity.";
    }
    if (f2 & TR2_SUST_CON) {
        info[i++] = "It sustains your constitution.";
    }
    if (f2 & TR2_SUST_CHR) {
        info[i++] = "It sustains your charisma.";
    }

    if (f2 & TR2_IM_ACID) {
        info[i++] = "It provides immunity to acid.";
    }
    if (f2 & TR2_IM_ELEC) {
        info[i++] = "It provides immunity to electricity.";
    }
    if (f2 & TR2_IM_FIRE) {
        info[i++] = "It provides immunity to fire.";
    }
    if (f2 & TR2_IM_COLD) {
        info[i++] = "It provides immunity to cold.";
    }
    if (f2 & TR2_IM_POIS) {
        info[i++] = "It provides immunity to poison.";
    }

    if (f2 & TR2_FREE_ACT) {
        info[i++] = "It provides immunity to paralysis.";
    }
    if (f2 & TR2_HOLD_LIFE) {
        info[i++] = "It provides resistance to life draining.";
    }

    if (f2 & TR2_RES_ACID) {
        info[i++] = "It provides resistance to acid.";
    }
    if (f2 & TR2_RES_ELEC) {
        info[i++] = "It provides resistance to electricity.";
    }
    if (f2 & TR2_RES_FIRE) {
        info[i++] = "It provides resistance to fire.";
    }
    if (f2 & TR2_RES_COLD) {
        info[i++] = "It provides resistance to cold.";
    }
    if (f2 & TR2_RES_POIS) {
        info[i++] = "It provides resistance to poison.";
    }

    if (f2 & TR2_RES_LITE) {
        info[i++] = "It provides resistance to light.";
    }
    if (f2 & TR2_RES_DARK) {
        info[i++] = "It provides resistance to dark.";
    }

    if (f2 & TR2_RES_BLIND) {
        info[i++] = "It provides resistance to blindness.";
    }
    if (f2 & TR2_RES_CONF) {
        info[i++] = "It provides resistance to confusion.";
    }
    if (f2 & TR2_RES_SOUND) {
        info[i++] = "It provides resistance to sound.";
    }
    if (f2 & TR2_RES_SHARDS) {
        info[i++] = "It provides resistance to shards.";
    }

    if (f2 & TR2_RES_NETHER) {
        info[i++] = "It provides resistance to nether.";
    }
    if (f2 & TR2_RES_NEXUS) {
        info[i++] = "It provides resistance to nexus.";
    }
    if (f2 & TR2_RES_CHAOS) {
        info[i++] = "It provides resistance to chaos.";
    }
    if (f2 & TR2_RES_DISEN) {
        info[i++] = "It provides resistance to disenchantment.";
    }

    if (f3 & TR3_FEATHER) {
        info[i++] = "It induces feather falling.";
    }
    if (f3 & TR3_LITE) {
        info[i++] = "It provides permanent light.";
    }
    if (f3 & TR3_SEE_INVIS) {
        info[i++] = "It allows you to see invisible monsters.";
    }
    if (f3 & TR3_TELEPATHY) {
        info[i++] = "It gives telepathic powers.";
    }
    if (f3 & TR3_SLOW_DIGEST) {
        info[i++] = "It slows your metabolism.";
    }
    if (f3 & TR3_REGEN) {
        info[i++] = "It speeds your regenerative powers.";
    }

    if (f3 & TR3_XTRA_MIGHT) {
        info[i++] = "It fires missiles with extra might.";
    }
    if (f3 & TR3_XTRA_SHOTS) {
        info[i++] = "It fires missiles excessively fast.";
    }

    if (f3 & TR3_DRAIN_EXP) {
        info[i++] = "It drains experience.";
    }
    if (f3 & TR3_TELEPORT) {
        info[i++] = "It induces random teleportation.";
    }
    if (f3 & TR3_AGGRAVATE) {
        info[i++] = "It aggravates nearby creatures.";
    }

    if (f3 & TR3_BLESSED) {
        info[i++] = "It has been blessed by the gods.";
    }

    if (cursed_p(i_ptr)) {
        if (f3 & TR3_PERMA_CURSE) {
            info[i++] = "It is permanently cursed.";
        }
        else if (f3 & TR3_HEAVY_CURSE) {
            info[i++] = "It is heavily cursed.";
        }
        else {
            info[i++] = "It is cursed.";
        }
    }


    if (f3 & TR3_IGNORE_ACID) {
        info[i++] = "It cannot be harmed by acid.";
    }
    if (f3 & TR3_IGNORE_ELEC) {
        info[i++] = "It cannot be harmed by electricity.";
    }
    if (f3 & TR3_IGNORE_FIRE) {
        info[i++] = "It cannot be harmed by fire.";
    }
    if (f3 & TR3_IGNORE_COLD) {
        info[i++] = "It cannot be harmed by cold.";
    }


    /* No special effects */
    if (!i) return (FALSE);


    /* Save the screen */
    Term_save();

    /* Erase the screen */
    for (k = 1; k < 24; k++) prt("", k, 13);

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
            for ( ; k > 2; k--) prt("", k, 15);
        }
    }

    /* Wait for it */
    prt("[Press any key to continue]", k, 15);
    inkey();

    /* Restore the screen */
    Term_load();

    /* Gave knowledge */
    return (TRUE);
}



/*
 * Hook to specify "weapon"
 */
static bool item_tester_hook_weapon(inven_type *i_ptr)
{
    switch (i_ptr->tval) {
        case TV_SWORD:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_DIGGING:
        case TV_BOW:
        case TV_BOLT:
        case TV_ARROW:
        case TV_SHOT:
            return (TRUE);
    }

    return (FALSE);
}


/*
 * Hook to specify "armour"
 */
static bool item_tester_hook_armour(inven_type *i_ptr)
{
    switch (i_ptr->tval) {
        case TV_DRAG_ARMOR:
        case TV_HARD_ARMOR:
        case TV_SOFT_ARMOR:
        case TV_SHIELD:
        case TV_CLOAK:
        case TV_CROWN:
        case TV_HELM:
        case TV_BOOTS:
        case TV_GLOVES:
            return (TRUE);
    }

    return (FALSE);
}



/*
 * Enchants a plus onto an item.                        -RAK-
 *
 * Revamped!  Now takes item pointer, number of times to try enchanting,
 * and a flag of what to try enchanting.  Artifacts resist enchantment
 * some of the time, and successful enchantment to at least +0 might
 * break a curse on the item.  -CFT
 *
 * Note that an item can technically be enchanted all the way to +15 if
 * you wait a very, very, long time.  Going from +9 to +10 only works
 * about 5% of the time, and from +10 to +11 only about 1% of the time.
 *
 * Note that this function can now be used on "piles" of items, and
 * the larger the pile, the lower the chance of success.
 */
bool enchant(inven_type *i_ptr, int n, int eflag)
{
    int i, chance, prob;

    bool res = FALSE;

    bool a = artifact_p(i_ptr);

    u32b f1, f2, f3;

    /* Extract the flags */
    inven_flags(i_ptr, &f1, &f2, &f3);
    
    
    /* Large piles resist enchantment */
    prob = i_ptr->number * 100;

    /* Missiles are easy to enchant */
    if ((i_ptr->tval == TV_BOLT) ||
        (i_ptr->tval == TV_ARROW) ||
        (i_ptr->tval == TV_SHOT)) {

        prob = prob / 20;
    }

    /* Try "n" times */
    for (i=0; i<n; i++) {

        /* Hack -- Roll for pile resistance */
        if (rand_int(prob) >= 100) continue;

        /* Enchant to hit */
        if (eflag & ENCH_TOHIT) {

            if (i_ptr->to_h < 0) chance = 0;
            else if (i_ptr->to_h > 15) chance = 1000;
            else chance = enchant_table[i_ptr->to_h];

            if ((randint(1000) > chance) && (!a || (rand_int(100) < 50))) {

                i_ptr->to_h++;
                res = TRUE;

                /* only when you get it above -1 -CFT */
                if (cursed_p(i_ptr) &&
                    (!(f3 & TR3_PERMA_CURSE)) &&
                    (i_ptr->to_h >= 0) && (rand_int(100) < 25)) {
                    msg_print("The curse is broken!");
                    i_ptr->ident &= ~ID_CURSED;
                    i_ptr->ident |= ID_SENSE;
                    i_ptr->note = quark_add("uncursed");
                }
            }
        }

        /* Enchant to damage */
        if (eflag & ENCH_TODAM) {

            if (i_ptr->to_d < 0) chance = 0;
            else if (i_ptr->to_d > 15) chance = 1000;
            else chance = enchant_table[i_ptr->to_d];

            if ((randint(1000) > chance) && (!a || (rand_int(100) < 50))) {

                i_ptr->to_d++;
                res = TRUE;

                /* only when you get it above -1 -CFT */
                if (cursed_p(i_ptr) &&
                    (!(f3 & TR3_PERMA_CURSE)) &&
                    (i_ptr->to_d >= 0) && (rand_int(100) < 25)) {
                    msg_print("The curse is broken!");
                    i_ptr->ident &= ~ID_CURSED;
                    i_ptr->ident |= ID_SENSE;
                    i_ptr->note = quark_add("uncursed");
                }
            }
        }

        /* Enchant to armor class */
        if (eflag & ENCH_TOAC) {

            if (i_ptr->to_a < 0) chance = 0;
            else if (i_ptr->to_a > 15) chance = 1000;
            else chance = enchant_table[i_ptr->to_a];

            if ((randint(1000) > chance) && (!a || (rand_int(100) < 50))) {

                i_ptr->to_a++;
                res = TRUE;

                /* only when you get it above -1 -CFT */
                if (cursed_p(i_ptr) &&
                    (!(f3 & TR3_PERMA_CURSE)) &&
                    (i_ptr->to_a >= 0) && (rand_int(100) < 25)) {
                    msg_print("The curse is broken!");
                    i_ptr->ident &= ~ID_CURSED;
                    i_ptr->ident |= ID_SENSE;
                    i_ptr->note = quark_add("uncursed");
                }
            }
        }
    }

    /* Redraw the choice window */
    p_ptr->redraw |= (PR_CHOOSE);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Return result */
    return (res);
}



/*
 * Enchant an item (in the inventory or on the floor)
 * Note that "num_ac" requires armour, else weapon
 * Returns TRUE if attempted, FALSE if cancelled
 */
bool enchant_spell(int num_hit, int num_dam, int num_ac)
{
    int			item;
    bool		okay = FALSE;

    inven_type		*i_ptr;

    char		i_name[80];


    /* Assume enchant weapon */
    item_tester_hook = item_tester_hook_weapon;

    /* Enchant armor if requested */
    if (num_ac) item_tester_hook = item_tester_hook_armour;

    /* Get an item (from equip or inven or floor) */
    if (!get_item(&item, "Enchant which item? ", TRUE, TRUE, TRUE)) {
        if (item == -2) msg_print("You have nothing to enchant.");
        return (FALSE);
    }

    /* Get the item (in the pack) */
    if (item >= 0) {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else {
        i_ptr = &i_list[0 - item];
    }


    /* Description */
    objdes(i_name, i_ptr, FALSE, 0);

    /* Describe */
    msg_format("%s %s glow%s brightly!",
               ((item >= 0) ? "Your" : "The"), i_name,
               ((i_ptr->number > 1) ? "" : "s"));

    /* Enchant */
    if (enchant(i_ptr, num_hit, ENCH_TOHIT)) okay = TRUE;
    if (enchant(i_ptr, num_dam, ENCH_TODAM)) okay = TRUE;
    if (enchant(i_ptr, num_ac, ENCH_TOAC)) okay = TRUE;

    /* Redraw choice window */
    p_ptr->redraw |= (PR_CHOOSE);

    /* Detect "failure" */
    if (!okay) {
        if (flush_failure) flush();
        msg_print("The enchantment failed.");
    }

    /* Something happened */
    return (TRUE);
}


/*
 * Identify an object in the inventory (or on the floor)
 * This routine does *not* automatically combine objects.
 * Returns TRUE if something was identified, else FALSE.
 */
bool ident_spell()
{
    int			item;

    inven_type		*i_ptr;

    char		i_name[80];


    /* Get an item (from equip or inven or floor) */
    if (!get_item(&item, "Identify which item? ", TRUE, TRUE, TRUE)) {
        if (item == -2) msg_print("You have nothing to identify.");
        return (FALSE);
    }

    /* Get the item (in the pack) */
    if (item >= 0) {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else {
        i_ptr = &i_list[0 - item];
    }


    /* Identify it fully */
    inven_aware(i_ptr);
    inven_known(i_ptr);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);	

    /* Redraw choice window */
    p_ptr->redraw |= (PR_CHOOSE);

    /* Combine pack */
    p_ptr->update |= (PU_COMBINE | PU_REORDER);

    /* Description */
    objdes(i_name, i_ptr, TRUE, 3);

    /* Describe */
    if (item >= INVEN_WIELD) {
        msg_format("%^s: %s (%c).",
                   describe_use(item), i_name, index_to_label(item));
    }
    else if (item >= 0) {
        msg_format("In your pack: %s (%c).",
                   i_name, index_to_label(item));
    }
    else {
        msg_format("On the ground: %s.",
                   i_name);
    }

    /* Something happened */
    return (TRUE);
}



/*
 * Fully "identify" an object in the inventory	-BEN-
 * This routine returns TRUE if an item was identified.
 */
bool identify_fully()
{
    int			item;

    inven_type		*i_ptr;

    char		i_name[80];


    /* Get an item (from equip or inven or floor) */
    if (!get_item(&item, "Fully *identify* which item? ", TRUE, TRUE, TRUE)) {
        if (item == -2) msg_print("You have nothing to identify.");
        return (FALSE);
    }

    /* Get the item (in the pack) */
    if (item >= 0) {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else {
        i_ptr = &i_list[0 - item];
    }


    /* Identify it fully */
    inven_aware(i_ptr);
    inven_known(i_ptr);

    /* Mark the item as fully known */
    i_ptr->ident |= (ID_MENTAL);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Redraw choice window */
    p_ptr->redraw |= (PR_CHOOSE);

    /* Handle stuff */
    handle_stuff();

    /* Description */
    objdes(i_name, i_ptr, TRUE, 3);

    /* Describe */
    if (item >= INVEN_WIELD) {
        msg_format("%^s: %s (%c).",
                   describe_use(item), i_name, index_to_label(item));
    }
    else if (item >= 0) {
        msg_format("In your pack: %s (%c).",
                   i_name, index_to_label(item));
    }
    else {
        msg_format("On the ground: %s.",
                   i_name);
    }

    /* Describe it fully */
    identify_fully_aux(i_ptr);

    /* Success */
    return (TRUE);
}




/*
 * Hook for "get_item()".  Determine if something is rechargable.
 */
static bool item_tester_hook_recharge(inven_type *i_ptr)
{
    /* Recharge staffs */
    if (i_ptr->tval == TV_STAFF) return (TRUE);

    /* Recharge wands */
    if (i_ptr->tval == TV_WAND) return (TRUE);

    /* Hack -- Recharge rods */
    if (i_ptr->tval == TV_ROD) return (TRUE);

    /* Nope */
    return (FALSE);
}


/*
 * Recharge a wand/staff/rod from the pack or on the floor.
 *
 * Mage -- Recharge I --> recharge(5)
 * Mage -- Recharge II --> recharge(40)
 * Mage -- Recharge III --> recharge(100)
 *
 * Priest -- Recharge --> recharge(15)
 *
 * Scroll of recharging --> recharge(60)
 *
 * recharge(20) = 1/6 failure for empty 10th level wand
 * recharge(60) = 1/10 failure for empty 10th level wand
 *
 * It is harder to recharge high level, and highly charged wands.
 *
 * XXX XXX XXX Beware of "sliding index errors".
 *
 * Should probably not "destroy" over-charged items, unless we
 * "replace" them by, say, a broken stick or some such.  The only
 * reason this is okay is because "scrolls of recharging" appear
 * BEFORE all staffs/wands/rods in the inventory.  Note that the
 * new "auto_sort_pack" option would correctly handle replacing
 * the "broken" wand with any other item (i.e. a broken stick).
 */
bool recharge(int num)
{
    int                 i, t, item, lev;

    inven_type		*i_ptr;


    /* Only accept legal items */
    item_tester_hook = item_tester_hook_recharge;

    /* Get an item (from inven or floor) */
    if (!get_item(&item, "Recharge which item? ", FALSE, TRUE, TRUE)) {
        if (item == -2) msg_print("You have nothing to recharge.");
        return (FALSE);
    }

    /* Get the item (in the pack) */
    if (item >= 0) {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else {
        i_ptr = &i_list[0 - item];
    }


    /* Extract the object "level" */
    lev = k_info[i_ptr->k_idx].level;

    /* Recharge a rod */
    if (i_ptr->tval == TV_ROD) {

        /* Extract a recharge power */
        i = (100 - lev + num) / 5;

        /* Paranoia -- prevent crashes */
        if (i < 1) i = 1;

        /* Back-fire */
        if (rand_int(i) == 0) {

            /* Hack -- backfire */
            msg_print("The recharge backfires, draining the rod further!");

            /* Hack -- decharge the rod */
            if (i_ptr->pval < 10000) i_ptr->pval = (i_ptr->pval + 100) * 2;
        }

        /* Recharge */
        else {

            /* Rechange amount */
            t = (num * damroll(2, 4));

            /* Recharge by that amount */
            if (i_ptr->pval > t) {
                i_ptr->pval -= t;
            }

            /* Fully recharged */
            else {
                i_ptr->pval = 0;
            }
        }
    }

    /* Recharge wand/staff */
    else {

        /* Recharge power */
        i = (num + 100 - lev - (10 * i_ptr->pval)) / 15;

        /* Paranoia -- prevent crashes */
        if (i < 1) i = 1;

        /* Back-fire XXX XXX XXX */
        if (rand_int(i) == 0) {

            /* Dangerous Hack -- Destroy the item */
            msg_print("There is a bright flash of light.");

            /* Reduce and describe inventory */
            if (item >= 0) {
                inven_item_increase(item, -999);
                inven_item_describe(item);
                inven_item_optimize(item);
            }

            /* Reduce and describe floor item */
            else {
                floor_item_increase(0 - item, -999);
                floor_item_describe(0 - item);
                floor_item_optimize(0 - item);
            }
        }

        /* Recharge */
        else {

            /* Extract a "power" */
            t = (num / (lev + 2)) + 1;

            /* Recharge based on the power */
            if (t > 0) i_ptr->pval += 2 + randint(t);

            /* Hack -- we no longer "know" the item */
            i_ptr->ident &= ~ID_KNOWN;

            /* Hack -- we no longer think the item is empty */
            i_ptr->ident &= ~ID_EMPTY;
        }
    }

    /* Redraw choice window */
    p_ptr->redraw |= (PR_CHOOSE);

    /* Something was done */
    return (TRUE);
}




/*
 * Some of the old functions
 */


bool lite_line(int dir)
{
    return (fire_beam(GF_LITE_WEAK, dir, damroll(6, 8)));
}

bool drain_life(int dir, int dam)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_DRAIN, dir, dam, flg));
}

bool wall_to_mud(int dir)
{
    int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM;
    return (project_hook(GF_KILL_WALL, dir, 20 + randint(30), flg));
}

bool destroy_door(int dir)
{
    int flg = PROJECT_BEAM | PROJECT_ITEM;
    return (project_hook(GF_KILL_DOOR, dir, 0, flg));
}

bool disarm_trap(int dir)
{
    int flg = PROJECT_BEAM | PROJECT_ITEM;
    return (project_hook(GF_KILL_TRAP, dir, 0, flg));
}

bool heal_monster(int dir)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_HEAL, dir, damroll(4, 6), flg));
}

bool speed_monster(int dir)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_SPEED, dir, p_ptr->lev, flg));
}

bool slow_monster(int dir)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_SLOW, dir, p_ptr->lev, flg));
}

bool sleep_monster(int dir)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_SLEEP, dir, p_ptr->lev, flg));
}

bool confuse_monster(int dir, int plev)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_CONF, dir, plev, flg));
}

bool fear_monster(int dir, int plev)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_SCARE, dir, plev, flg));
}

bool poly_monster(int dir)
{
    int flg = PROJECT_BEAM;
    return (project_hook(GF_OLD_POLY, dir, p_ptr->lev, flg));
}

bool clone_monster(int dir)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_CLONE, dir, 0, flg));
}

bool teleport_monster(int dir)
{
    int flg = PROJECT_BEAM;
    return (project_hook(GF_OLD_TPORT, dir, MAX_SIGHT * 5, flg));
}



/*
 * Hooks -- affect adjacent grids (radius 1 ball attack)
 */

bool door_creation()
{
    int flg = PROJECT_ITEM | PROJECT_GRID | PROJECT_HIDE;
    return (project(0, 1, py, px, 0, GF_MAKE_DOOR, flg));
}

bool trap_creation()
{
    int flg = PROJECT_ITEM | PROJECT_GRID | PROJECT_HIDE;
    return (project(0, 1, py, px, 0, GF_MAKE_TRAP, flg));
}

bool destroy_doors_touch()
{
    int flg = PROJECT_ITEM | PROJECT_HIDE;
    return (project(0, 1, py, px, 0, GF_KILL_DOOR, flg));
}

bool sleep_monsters_touch(void)
{
    int flg = PROJECT_HIDE;
    return (project(0, 1, py, px, p_ptr->lev, GF_OLD_SLEEP, flg));
}



/*
 * Hooks -- affect all nearby monsters
 */

bool speed_monsters(void)
{
    return (project_hack(GF_OLD_SPEED, p_ptr->lev));
}

bool slow_monsters(void)
{
    return (project_hack(GF_OLD_SLOW, p_ptr->lev));
}

bool sleep_monsters(void)
{
    return (project_hack(GF_OLD_SLEEP, p_ptr->lev));
}




/*
 * Hack -- call light around the player
 */
bool lite_area(int dam, int rad)
{
    /* Hack -- Message */
    if (!p_ptr->blind) {
        msg_print("You are surrounded by a white light.");
    }

    /* Hook into the "project()" function */
    (void)project(0, rad, py, px, dam, GF_LITE_WEAK, PROJECT_GRID);

    /* Lite up the room */
    lite_room(py, px);

    /* Assume seen */
    return (TRUE);
}


/*
 * Hack -- call darkness around the player
 */
bool unlite_area(int dam, int rad)
{
    /* Hack -- Message */
    if (!p_ptr->blind) {
        msg_print("Darkness surrounds you.");
    }

    /* Hook into the "project()" function */
    (void)project(0, rad, py, px, dam, GF_DARK_WEAK, PROJECT_GRID);

    /* Lite up the room */
    unlite_room(py, px);

    /* Assume seen */
    return (TRUE);
}







/*
 * The spell of destruction
 *
 * This spell "deletes" monsters (instead of "killing" them).
 *
 * Later we may use one function for both "destruction" and
 * "earthquake" by using the "full" to select "destruction".
 */
void destroy_area(int y1, int x1, int r, bool full)
{
    int y, x, k, t;

    cave_type *c_ptr;

    bool flag = FALSE;


    /* Big area of affect */
    for (y = (y1 - r); y <= (y1 + r); y++) {
        for (x = (x1 - r); x <= (x1 + r); x++) {

            /* Skip illegal grids */
            if (!in_bounds(y, x)) continue;

            /* Extract the distance */
            k = distance(y1, x1, y, x);

            /* Stay in the circle of death */
            if (k > r) continue;

            /* Access the grid */
            c_ptr = &cave[y][x];

            /* Lose room and vault */
            c_ptr->feat &= ~(CAVE_ROOM | CAVE_ICKY);

            /* Lose light and knowledge */
            c_ptr->feat &= ~(CAVE_MARK | CAVE_GLOW);

            /* Hack -- Notice player affect */
            if ((x == px) && (y == py)) {
                
                /* Hurt the player later */
                flag = TRUE;

                /* Do not hurt this grid */
                continue;
            }

            /* Hack -- Skip the epicenter */
            if ((y == y1) && (x == x1)) continue;

            /* Delete the monster (if any) */
            delete_monster(y, x);
		
            /* Destroy "valid" grids */
            if (valid_grid(y, x)) {

                /* Delete the object (if any) */
                delete_object(y, x);

                /* Access the grid */
                c_ptr = &cave[y][x];

                /* Wall (or floor) type */
                t = rand_int(200);

                /* Granite */
                if (t < 20) {

                    /* Clear previous contents, add granite wall */
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x38);
                }
                
                /* Quartz */
                else if (t < 70) {

                    /* Clear previous contents, add quartz vein */
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x33);
                }
                
                /* Magma */
                else if (t < 100) {

                    /* Clear previous contents, add magma vein */
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x32);
                }

                /* Floor */
                else {

                    /* Clear previous contents, add floor */
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x01);
                }                    
            }
        }
    }


    /* Hack -- Affect player */
    if (flag) {
    
        /* Message */
        msg_print("There is a searing blast of light!");

        /* Blind the player */
        if (!p_ptr->resist_blind && !p_ptr->resist_lite) {

            /* Become blind */
            add_blind(10 + randint(10));
        }
    }
    

    /* Update stuff */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);

    /* Redraw stuff */
    p_ptr->redraw |= (PR_MAP);
}


/*
 * Induce an "earthquake" of the given radius at the given location.
 *
 * This will turn some walls into floors and some floors into walls.
 *
 * The player will take damage and "jump" into a safe grid if possible,
 * otherwise, he will "tunnel" through the rubble instantaneously.
 *
 * Monsters will take damage, and "jump" into a safe grid if possible,
 * otherwise they will be "buried" in the rubble, disappearing from
 * the level in the same way that they do when genocided.
 *
 * Note that thus the player and monsters (except eaters of walls and
 * passers through walls) will never occupy the same grid as a wall.
 * Note that as of now (2.7.8) no monster may occupy a "wall" grid, even
 * for a single turn, unless that monster can pass_walls or kill_walls.
 * This has allowed massive simplification of the "monster" code.
 */
void earthquake(int cy, int cx, int r)
{
    int		i, t, y, x, yy, xx, dy, dx, oy, ox;

    int		damage = 0;

    int		sn = 0, sy = 0, sx = 0;

    bool	hurt = FALSE;

    cave_type	*c_ptr;

    bool	map[32][32];


    /* Paranoia -- Enforce maximum range */
    if (r > 12) r = 12;

    /* Clear the "maximal blast" area */
    for (y = 0; y < 32; y++) {
        for (x = 0; x < 32; x++) {
            map[y][x] = FALSE;
        }
    }

    /* Check around the epicenter */
    for (dy = -r; dy <= r; dy++) {
        for (dx = -r; dx <= r; dx++) {

            /* Extract the location */
            yy = cy + dy;
            xx = cx + dx;

            /* Skip illegal grids */
            if (!in_bounds(yy, xx)) continue;

            /* Skip distant grids */
            if (distance(cy, cx, yy, xx) > r) continue;

            /* Access the grid */
            c_ptr = &cave[yy][xx];

            /* Lose room and vault */
            c_ptr->feat &= ~(CAVE_ROOM | CAVE_ICKY);

            /* Lose light and knowledge */
            c_ptr->feat &= ~(CAVE_GLOW | CAVE_MARK);

            /* Skip the epicenter */
            if (!dx && !dy) continue;

            /* Skip most grids */
            if (rand_int(100) < 85) continue;

            /* Hack -- Take note of player damage */
            if ((yy == py) && (xx == px)) hurt = TRUE;

            /* Damage this grid */
            map[16+yy-cy][16+xx-cx] = TRUE;
        }
    }

    /* First, affect the player (if necessary) */
    if (hurt) {

        /* Check around the player */
        for (i = 0; i < 8; i++) {

            /* Access the location */
            y = py + ddy[i];
            x = px + ddx[i];

            /* Skip non-empty grids */
            if (!empty_grid_bold(y, x)) continue;

            /* Count "safe" grids */
            sn++;

            /* Randomize choice */
            if (rand_int(sn) > 0) continue;

            /* Save the safe location */
            sy = y; sx = x;
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

        /* Hurt the player a lot */
        if (!sn) {

            /* Message and damage */
            msg_print("You are severely crushed!");
            damage = 300;

            /* Important -- no wall on player */
            map[16+py-cy][16+px-cx] = FALSE;
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

            /* Save the old location */
            oy = py;
            ox = px;
    
            /* Move the player to the safe location */
            py = sy;
            px = sx;
    
            /* Redraw the old spot */
            lite_spot(oy, ox);
    
            /* Redraw the new spot */
            lite_spot(py, px);

            /* Check for new panel */
            verify_panel();
        }

        /* Take some damage */
        if (damage) take_hit(damage, "an earthquake");
    }


    /* Examine the quaked region */
    for (dy = -r; dy <= r; dy++) {
        for (dx = -r; dx <= r; dx++) {

            /* Extract the location */
            yy = cy + dy;
            xx = cx + dx;

            /* Skip unaffected grids */
            if (!map[16+yy-cy][16+xx-cx]) continue;

            /* Access the grid */
            c_ptr = &cave[yy][xx];

            /* Process monsters */
            if (c_ptr->m_idx) {

                monster_type *m_ptr = &m_list[c_ptr->m_idx];
                monster_race *r_ptr = &r_info[m_ptr->r_idx];

                /* Most monsters cannot co-exist with rock */
                if (!(r_ptr->flags2 & RF2_KILL_WALL) &&
                    !(r_ptr->flags2 & RF2_PASS_WALL)) {

                    char m_name[80];

                    /* Assume not safe */
                    sn = 0;

                    /* Monster can move to escape the wall */
                    if (!(r_ptr->flags1 & RF1_NEVER_MOVE)) {

                        /* Look for safety */
                        for (i = 0; i < 8; i++) {

                            /* Access the grid */
                            y = yy + ddy[i];
                            x = xx + ddx[i];

                            /* Skip non-empty grids */
                            if (!empty_grid_bold(y, x)) continue;

                            /* Important -- Skip "quake" grids */
                            if (map[16+y-cy][16+x-cx]) continue;

                            /* Count "safe" grids */
                            sn++;

                            /* Randomize choice */
                            if (rand_int(sn) > 0) continue;

                            /* Save the safe grid */
                            sy = y; sx = x;
                        }
                    }

                    /* Describe the monster */
                    monster_desc(m_name, m_ptr, 0);

                    /* Scream in pain */
                    msg_format("%^s wails out in pain!", m_name);

                    /* Take damage from the quake */
                    damage = (sn ? damroll(4, 8) : 200);

                    /* Monster is certainly awake */
                    m_ptr->csleep = 0;

                    /* Apply damage directly */
                    m_ptr->hp -= damage;

                    /* Delete (not kill) "dead" monsters */
                    if (m_ptr->hp < 0) {

                        /* Message */
                        msg_format("%^s is embedded in the rock!", m_name);

                        /* Delete the monster */
                        delete_monster(yy, xx);

                        /* No longer safe */
                        sn = 0;
                    }

                    /* Hack -- Escape from the rock */
                    if (sn) {

                        int m_idx = cave[yy][xx].m_idx;
                        
                        /* Update the new location */
                        cave[sy][sx].m_idx = m_idx;

                        /* Update the old location */
                        cave[yy][xx].m_idx = 0;

                        /* Move the monster */
                        m_ptr->fy = sy;
                        m_ptr->fx = sx;
            
                        /* Update the monster (new location) */
                        update_mon(m_idx, TRUE);

                        /* Redraw the old grid */
                        lite_spot(yy, xx);
            
                        /* Redraw the new grid */
                        lite_spot(sy, sx);
                    }
                }
            }
        }
    }


    /* Examine the quaked region */
    for (dy = -r; dy <= r; dy++) {
        for (dx = -r; dx <= r; dx++) {

            /* Extract the location */
            yy = cy + dy;
            xx = cx + dx;

            /* Skip unaffected grids */
            if (!map[16+yy-cy][16+xx-cx]) continue;

            /* Access the cave grid */
            c_ptr = &cave[yy][xx];

            /* Paranoia -- never affect player */
            if ((yy == py) && (xx == px)) continue;

            /* Destroy location (if valid) */
            if (valid_grid(yy, xx)) {

                bool floor = floor_grid_bold(yy, xx);

                /* Delete any object that is still there */
                delete_object(yy, xx);

                /* Access the grid */
                c_ptr = &cave[y][x];

                /* Wall (or floor) type */
                t = (floor ? rand_int(100) : 200);

                /* Granite */
                if (t < 20) {

                    /* Clear previous contents, add granite wall */
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x38);
                }
                
                /* Quartz */
                else if (t < 70) {

                    /* Clear previous contents, add quartz vein */
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x33);
                }
                
                /* Magma */
                else if (t < 100) {

                    /* Clear previous contents, add magma vein */
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x32);
                }

                /* Floor */
                else {

                    /* Clear previous contents, add floor */
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x01);
                }
            }
        }
    }


    /* Update stuff */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);

    /* Update the monsters */
    p_ptr->update |= (PU_DISTANCE);

    /* Update the health bar */
    p_ptr->redraw |= (PR_HEALTH);

    /* Redraw stuff */
    p_ptr->redraw |= (PR_MAP);
}


