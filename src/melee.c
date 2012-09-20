/* File: melee.c */

/* Purpose: handle monster attacks and spells */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"



/*
 * Critical blow
 */
static int monster_critical(int dice, int sides, int dam)
{
    int total = dice * sides;

    if ((dam > total * 19 / 20) && ((dam >= 20) || (rand_int(20) == 0))) {

        int max = 0;

        if (dam > 20) {
            if (dam == total) max++;
            while (rand_int(50) == 0) max++;
        }

        if (dam > 45) return (6 + max);
        if (dam > 33) return (5 + max);
        if (dam > 25) return (4 + max);
        if (dam > 18) return (3 + max);
        if (dam > 11) return (2 + max);
        return (1 + max);
    }

    return 0;
}



#ifdef DRS_SMART_OPTIONS


/*
 * And now for Intelligent monster attacks (including spells).
 *
 * Original idea and code by "DRS" (David Reeves Sward).
 * Major modifications by "BEN" (Ben Harrison).
 *
 * Give monsters more intelligent attack/spell selection based on
 * observations of previous attacks on the player, and/or by allowing
 * the monster to "cheat" and know the player status.
 *
 * Maintain an idea of the player status, and use that information
 * to occasionally eliminate "ineffective" spell attacks.  We could
 * also eliminate ineffective normal attacks, but there is no reason
 * for the monster to do this, since he gains no benefit.
 * Note that MINDLESS monsters are not allowed to use this code.
 * And non-INTELLIGENT monsters only use it partially effectively.
 *
 * Actually learn what the player resists, and use that information
 * to remove attacks or spells before using them.  This will require
 * much less space, if I am not mistaken.  Thus, each monster gets a
 * set of 32 bit flags, "smart", build from the various "SM_*" flags.
 *
 * This has the added advantage that attacks and spells are related.
 * The "smart_learn" option means that the monster "learns" the flags
 * that should be set, and "smart_cheat" means that he "knows" them.
 * So "smart_cheat" means that the "smart" field is always up to date,
 * while "smart_learn" means that the "smart" field is slowly learned.
 * Both of them have the same effect on the "choose spell" routine.
 */




/*
 * Learn about an "observed" resistance.
 */
static void update_smart_learn(int m_idx, int what)
{
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_list[m_ptr->r_idx];


    /* Not allowed to learn */
    if (!smart_learn) return;

    /* Too stupid to learn anything */
    if (r_ptr->rflags2 & RF2_STUPID) return;

    /* Not intelligent, only learn sometimes */
    if (!(r_ptr->rflags2 & RF2_SMART) && (randint(100) < 50)) return;

    
    /* Analyze the knowledge */
    switch (what) {

      case DRS_ACID:
        if (p_ptr->resist_acid) m_ptr->smart |= SM_RES_ACID;
        if (p_ptr->oppose_acid) m_ptr->smart |= SM_OPP_ACID;
        if (p_ptr->immune_acid) m_ptr->smart |= SM_IMM_ACID;
        break;

      case DRS_ELEC:
        if (p_ptr->resist_elec) m_ptr->smart |= SM_RES_ELEC;
        if (p_ptr->oppose_elec) m_ptr->smart |= SM_OPP_ELEC;
        if (p_ptr->immune_elec) m_ptr->smart |= SM_IMM_ELEC;
        break;

      case DRS_FIRE:
        if (p_ptr->resist_fire) m_ptr->smart |= SM_RES_FIRE;
        if (p_ptr->oppose_fire) m_ptr->smart |= SM_OPP_FIRE;
        if (p_ptr->immune_fire) m_ptr->smart |= SM_IMM_FIRE;
        break;

      case DRS_COLD:
        if (p_ptr->resist_cold) m_ptr->smart |= SM_RES_COLD;
        if (p_ptr->oppose_cold) m_ptr->smart |= SM_OPP_COLD;
        if (p_ptr->immune_cold) m_ptr->smart |= SM_IMM_COLD;
        break;

      case DRS_POIS:
        if (p_ptr->resist_pois) m_ptr->smart |= SM_RES_POIS;
        if (p_ptr->oppose_pois) m_ptr->smart |= SM_OPP_POIS;
        if (p_ptr->immune_pois) m_ptr->smart |= SM_IMM_POIS;
        break;


      case DRS_NETH:
        if (p_ptr->resist_neth) m_ptr->smart |= SM_RES_NETH;
        break;
        
      case DRS_LITE:
        if (p_ptr->resist_lite) m_ptr->smart |= SM_RES_LITE;
        break;

      case DRS_DARK:
        if (p_ptr->resist_dark) m_ptr->smart |= SM_RES_DARK;
        break;

      case DRS_FEAR:
        if (p_ptr->resist_fear) m_ptr->smart |= SM_RES_FEAR;
        break;

      case DRS_CONF:
        if (p_ptr->resist_conf) m_ptr->smart |= SM_RES_CONF;
        break;

      case DRS_CHAOS:
        if (p_ptr->resist_chaos) m_ptr->smart |= SM_RES_CHAOS;
        break;

      case DRS_DISEN:
        if (p_ptr->resist_disen) m_ptr->smart |= SM_RES_DISEN;
        break;

      case DRS_BLIND:
        if (p_ptr->resist_blind) m_ptr->smart |= SM_RES_BLIND;
        break;

      case DRS_NEXUS:
        if (p_ptr->resist_nexus) m_ptr->smart |= SM_RES_NEXUS;
        break;

      case DRS_SOUND:
        if (p_ptr->resist_sound) m_ptr->smart |= SM_RES_SOUND;
        break;

      case DRS_SHARD:
        if (p_ptr->resist_shard) m_ptr->smart |= SM_RES_SHARD;
        break;


      case DRS_FREE:
        if (p_ptr->free_act) m_ptr->smart |= SM_IMM_FREE;
        break;

      case DRS_MANA:
        if (!p_ptr->mana) m_ptr->smart |= SM_IMM_MANA;
        break;
    }
}






/*
 * Internal probablility routine
 *
 * If i or j is positive, use "1 in i" or "1 in j".
 * If i or j is negative, use "(i-1) in i" or "(j-1) in j".
 *
 * Ex: i = 3, use 1 in 3.  i = -3, use 2 in 3.
 */
static bool int_outof(monster_race *r_ptr, int prob)
{
    /* Non-Smart monsters are half as "smart" */
    if (!(r_ptr->rflags2 & RF2_SMART)) prob = prob / 2;

    /* Roll the dice */
    return (rand_int(100) < prob);
}



/*
 * Remove the "bad" spells from a spell list
 */
static void remove_bad_spells(int m_idx, u32b *f4p, u32b *f5p, u32b *f6p)
{
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_list[m_ptr->r_idx];

    u32b f4 = (*f4p);
    u32b f5 = (*f5p);
    u32b f6 = (*f6p);
    
    u32b smart = 0L;
    

    /* Too stupid to know anything */
    if (r_ptr->rflags2 & RF2_STUPID) return;


    /* Must be cheating or learning */
    if (!smart_cheat && !smart_learn) return;


    /* Update acquired knowledge */
    if (smart_learn) {

        /* Hack -- Occasionally forget player status */
        if (m_ptr->smart && (rand_int(100) == 0)) m_ptr->smart = 0L;

        /* Use the memorized flags */
        smart = m_ptr->smart;
    }


    /* Cheat if requested */
    if (smart_cheat) {

        /* Know basic info */    
        if (p_ptr->resist_acid) smart |= SM_RES_ACID;
        if (p_ptr->oppose_acid) smart |= SM_OPP_ACID;
        if (p_ptr->immune_acid) smart |= SM_IMM_ACID;
        if (p_ptr->resist_elec) smart |= SM_RES_ELEC;
        if (p_ptr->oppose_elec) smart |= SM_OPP_ELEC;
        if (p_ptr->immune_elec) smart |= SM_IMM_ELEC;
        if (p_ptr->resist_fire) smart |= SM_RES_FIRE;
        if (p_ptr->oppose_fire) smart |= SM_OPP_FIRE;
        if (p_ptr->immune_fire) smart |= SM_IMM_FIRE;
        if (p_ptr->resist_cold) smart |= SM_RES_COLD;
        if (p_ptr->oppose_cold) smart |= SM_OPP_COLD;
        if (p_ptr->immune_cold) smart |= SM_IMM_COLD;
        if (p_ptr->resist_pois) smart |= SM_RES_POIS;
        if (p_ptr->oppose_pois) smart |= SM_OPP_POIS;
        if (p_ptr->immune_pois) smart |= SM_IMM_POIS;
    
        /* Know special resistances */
        if (p_ptr->resist_neth) smart |= SM_RES_NETH;
        if (p_ptr->resist_lite) smart |= SM_RES_LITE;
        if (p_ptr->resist_dark) smart |= SM_RES_DARK;
        if (p_ptr->resist_fear) smart |= SM_RES_FEAR;
        if (p_ptr->resist_conf) smart |= SM_RES_CONF;
        if (p_ptr->resist_chaos) smart |= SM_RES_CHAOS;
        if (p_ptr->resist_disen) smart |= SM_RES_DISEN;
        if (p_ptr->resist_blind) smart |= SM_RES_BLIND;
        if (p_ptr->resist_nexus) smart |= SM_RES_NEXUS;
        if (p_ptr->resist_sound) smart |= SM_RES_SOUND;
        if (p_ptr->resist_shard) smart |= SM_RES_SHARD;
    
        /* Know bizarre "resistances" */
        if (p_ptr->free_act) smart |= SM_IMM_FREE;
        if (!p_ptr->mana) smart |= SM_IMM_MANA;
    }


    /* Nothing known */
    if (!smart) return;
    

    if (smart & SM_IMM_ACID) {
        if (int_outof(r_ptr, 100)) f4 &= ~RF4_BR_ACID;
        if (int_outof(r_ptr, 100)) f5 &= ~RF5_BA_ACID;
        if (int_outof(r_ptr, 100)) f5 &= ~RF5_BO_ACID;
    }
    else if ((smart & SM_OPP_ACID) && (smart & SM_RES_ACID)) {
        if (int_outof(r_ptr, 80)) f4 &= ~RF4_BR_ACID;
        if (int_outof(r_ptr, 80)) f5 &= ~RF5_BA_ACID;
        if (int_outof(r_ptr, 80)) f5 &= ~RF5_BO_ACID;
    }
    else if ((smart & SM_OPP_ACID) || (smart & SM_RES_ACID)) {
        if (int_outof(r_ptr, 30)) f4 &= ~RF4_BR_ACID;
        if (int_outof(r_ptr, 30)) f5 &= ~RF5_BA_ACID;
        if (int_outof(r_ptr, 30)) f5 &= ~RF5_BO_ACID;
    }

    
    if (smart & SM_IMM_ELEC) {
        if (int_outof(r_ptr, 100)) f4 &= ~RF4_BR_ELEC;
        if (int_outof(r_ptr, 100)) f5 &= ~RF5_BA_ELEC;
        if (int_outof(r_ptr, 100)) f5 &= ~RF5_BO_ELEC;
    }
    else if ((smart & SM_OPP_ELEC) && (smart & SM_RES_ELEC)) {
        if (int_outof(r_ptr, 80)) f4 &= ~RF4_BR_ELEC;
        if (int_outof(r_ptr, 80)) f5 &= ~RF5_BA_ELEC;
        if (int_outof(r_ptr, 80)) f5 &= ~RF5_BO_ELEC;
    }
    else if ((smart & SM_OPP_ELEC) || (smart & SM_RES_ELEC)) {
        if (int_outof(r_ptr, 30)) f4 &= ~RF4_BR_ELEC;
        if (int_outof(r_ptr, 30)) f5 &= ~RF5_BA_ELEC;
        if (int_outof(r_ptr, 30)) f5 &= ~RF5_BO_ELEC;
    }

    
    if (smart & SM_IMM_FIRE) {
        if (int_outof(r_ptr, 100)) f4 &= ~RF4_BR_FIRE;
        if (int_outof(r_ptr, 100)) f5 &= ~RF5_BA_FIRE;
        if (int_outof(r_ptr, 100)) f5 &= ~RF5_BO_FIRE;
    }
    else if ((smart & SM_OPP_FIRE) && (smart & SM_RES_FIRE)) {
        if (int_outof(r_ptr, 80)) f4 &= ~RF4_BR_FIRE;
        if (int_outof(r_ptr, 80)) f5 &= ~RF5_BA_FIRE;
        if (int_outof(r_ptr, 80)) f5 &= ~RF5_BO_FIRE;
    }
    else if ((smart & SM_OPP_FIRE) || (smart & SM_RES_FIRE)) {
        if (int_outof(r_ptr, 30)) f4 &= ~RF4_BR_FIRE;
        if (int_outof(r_ptr, 30)) f5 &= ~RF5_BA_FIRE;
        if (int_outof(r_ptr, 30)) f5 &= ~RF5_BO_FIRE;
    }
    
    
    if (smart & SM_IMM_COLD) {
        if (int_outof(r_ptr, 100)) f4 &= ~RF4_BR_COLD;
        if (int_outof(r_ptr, 100)) f5 &= ~RF5_BA_COLD;
        if (int_outof(r_ptr, 100)) f5 &= ~RF5_BO_COLD;
        if (int_outof(r_ptr, 100)) f5 &= ~RF5_BO_ICEE;
    }
    else if ((smart & SM_OPP_COLD) && (smart & SM_RES_COLD)) {
        if (int_outof(r_ptr, 80)) f4 &= ~RF4_BR_COLD;
        if (int_outof(r_ptr, 80)) f5 &= ~RF5_BA_COLD;
        if (int_outof(r_ptr, 80)) f5 &= ~RF5_BO_COLD;
        if (int_outof(r_ptr, 80)) f5 &= ~RF5_BO_ICEE;
    }
    else if ((smart & SM_OPP_COLD) || (smart & SM_RES_COLD)) {
        if (int_outof(r_ptr, 30)) f4 &= ~RF4_BR_COLD;
        if (int_outof(r_ptr, 30)) f5 &= ~RF5_BA_COLD;
        if (int_outof(r_ptr, 30)) f5 &= ~RF5_BO_COLD;
        if (int_outof(r_ptr, 30)) f5 &= ~RF5_BO_ICEE;
    }
    
    
    if (smart & SM_IMM_POIS) {
        if (int_outof(r_ptr, 100)) f4 &= ~RF4_BR_POIS;
        if (int_outof(r_ptr, 100)) f5 &= ~RF5_BA_POIS;
    }
    else if ((smart & SM_OPP_POIS) && (smart & SM_RES_POIS)) {
        if (int_outof(r_ptr, 80)) f4 &= ~RF4_BR_POIS;
        if (int_outof(r_ptr, 80)) f5 &= ~RF5_BA_POIS;
    }
    else if ((smart & SM_OPP_POIS) || (smart & SM_RES_POIS)) {
        if (int_outof(r_ptr, 30)) f4 &= ~RF4_BR_POIS;
        if (int_outof(r_ptr, 30)) f5 &= ~RF5_BA_POIS;
    }


    if (smart & SM_RES_NETH) {
        if (int_outof(r_ptr, 50)) f4 &= ~RF4_BR_NETH;
        if (int_outof(r_ptr, 50)) f5 &= ~RF5_BA_NETH;
        if (int_outof(r_ptr, 50)) f5 &= ~RF5_BO_NETH;
    }

    if (smart & SM_RES_LITE) {
        if (int_outof(r_ptr, 50)) f4 &= ~RF4_BR_LITE;
    }

    if (smart & SM_RES_DARK) {
        if (int_outof(r_ptr, 50)) f4 &= ~RF4_BR_DARK;
        if (int_outof(r_ptr, 50)) f5 &= ~RF5_BA_DARK;
    }

    if (smart & SM_RES_FEAR) {
        if (int_outof(r_ptr, 100)) f5 &= ~RF5_SCARE;
    }

    if (smart & SM_RES_CONF) {
        if (int_outof(r_ptr, 100)) f5 &= ~RF5_CONF;
        if (int_outof(r_ptr, 50)) f4 &= ~RF4_BR_CONF;
    }

    if (smart & SM_RES_CHAOS) {
        if (int_outof(r_ptr, 100)) f5 &= ~RF5_CONF;
        if (int_outof(r_ptr, 50)) f4 &= ~RF4_BR_CONF;
        if (int_outof(r_ptr, 50)) f4 &= ~RF4_BR_CHAO;
    }

    if (smart & SM_RES_DISEN) {
        if (int_outof(r_ptr, 100)) f4 &= ~RF4_BR_DISE;
    }

    if (smart & SM_RES_BLIND) {
        if (int_outof(r_ptr, 100)) f5 &= ~RF5_BLIND;
    }

    if (smart & SM_RES_NEXUS) {
        if (int_outof(r_ptr, 50)) f4 &= ~RF4_BR_NEXU;
        if (int_outof(r_ptr, 50)) f6 &= ~RF6_TELE_LEVEL;
    }

    if (smart & SM_RES_SOUND) {
        if (int_outof(r_ptr, 50)) f4 &= ~RF4_BR_SOUN;
    }

    if (smart & SM_RES_SHARD) {
        if (int_outof(r_ptr, 50)) f4 &= ~RF4_BR_SHAR;
    }


    if (smart & SM_IMM_FREE) {
        if (int_outof(r_ptr, 100)) f5 &= ~RF5_HOLD;
        if (int_outof(r_ptr, 100)) f5 &= ~RF5_SLOW;
    }

    if (smart & SM_IMM_MANA) {
        if (int_outof(r_ptr, 100)) f5 &= ~RF5_DRAIN_MANA;
    }


    /* XXX XXX XXX if (!f4 && !f5 && !f6) ... */


    (*f4p) = f4;
    (*f5p) = f5;
    (*f6p) = f6;
}



#else


/*
 * Learn nothing about the player
 */
#define update_smart_learn(M,W)		/* nothing */


#endif





/*
 * Apply an "earthquake" to the player's location
 * Take the "cause" of the quake as a paramater
 *
 * This function ALWAYS "re-updates" the view/lite/monsters
 */
static void quake_player(cptr what)
{
    int		x, y;
    int		cx, cy;
    cave_type	*c_ptr;
    int		tmp, damage = 0;
    int		sy = 0, sx = 0, safe = 0;


    /* Get the player location */
    cx = py;
    cy = px;

    /* See if we can find a "safe" location to "move" to */
    for (y = cy - 1; !safe && y <= cy + 1; y++) {
        for (x = cx - 1; !safe && x <= cx + 1; x++) {
            if ((y == cy) && (x == cx)) continue;
            if (!in_bounds(y, x)) continue;
            if (empty_grid_bold(y, x)) {
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

        /* Destroy location (if valid) */
        if (valid_grid(cy, cx)) {
            tmp = randint(10);
            if (tmp < 6) {
                c_ptr->info &= ~GRID_WALL_MASK;
                c_ptr->info |= GRID_WALL_QUARTZ;
            }
            else if (tmp < 9) {
                c_ptr->info &= ~GRID_WALL_MASK;
                c_ptr->info |= GRID_WALL_MAGMA;
            }
            else {
                c_ptr->info &= ~GRID_WALL_MASK;
                c_ptr->info |= GRID_WALL_GRANITE;
            }
            delete_object(cy, cx);
            c_ptr->info &= ~GRID_GLOW;
            c_ptr->info &= ~GRID_MARK;
            lite_spot(cy, cx);
        }


        /* Move the player to the safe location */
        move_rec(py, px, sy, sx);

        /* Check for new panel (redraw map) */
        verify_panel();

        /* Update some things */
        p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);
        p_ptr->update |= (PU_MONSTERS);
    }



    /* Take some damage */
    if (damage) take_hit(damage, what);
}




/*
 * Hack -- Drop a "wall" on the player, who must be at the given location.
 *
 * XXX XXX Perhaps replace this with a "beam" of rock breathing.
 */
static void br_wall(int cy, int cx)
{
    /* Hack -- Verify location */
    if ((cx != py) || (cy != px)) return;

    /* Take some damage */
    quake_player("a breathed wall");
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
    int				i, j, kill, y, x;
    int				tmp, damage = 0;


    /* Check around the epicenter */
    for (i = cy - 8; i <= cy + 8; i++) {
        for (j = cx - 8; j <= cx + 8; j++) {

            /* Skip the epicenter */
            if ((i == cy) && (j == cx)) continue;

            /* Skip illegal grids */
            if (!in_bounds(i, j)) continue;

            /* Sometimes, shatter that grid */
            if (rand_int(8) == 0) {

                cave_type *c_ptr = &cave[i][j];

                monster_type *m_ptr = &m_list[c_ptr->m_idx];
                monster_race *r_ptr = &r_list[m_ptr->r_idx];


                /* Treat the player grid specially */
                if (c_ptr->m_idx == 1) {

                    /* Quake the player */
                    quake_player("an earthquake");

                    /* Continue */
                    continue;
                }


                /* Embed a (non-player) monster */
                if (c_ptr->m_idx > 1) {

                    char m_name[80];

                    /* Describe the monster */
                    monster_desc(m_name, m_ptr, 0);

                    /* Ignore "rock eaters" */
                    if (!(r_ptr->rflags2 & RF2_PASS_WALL) &&
                        !(r_ptr->rflags2 & RF2_KILL_WALL)) {

                        /* Monster can not move to escape the wall */
                        if (r_ptr->rflags1 & RF1_NEVER_MOVE) {
                            kill = TRUE;
                        }

                        /* The monster MAY be able to dodge the walls */
                        else {

                            /* Assume surrounded by rocks */
                            kill = TRUE;

                            /* Look for non-rock space (dodge later) */
                            for (y = i - 1; y <= i + 1; y++) {
                                for (x = j - 1; x <= j + 1; x++) {
                                    if ((y == i) && (x == j)) continue;
                                    if (!in_bounds(y, x)) continue;
                                    if (empty_grid_bold(y, x)) kill = FALSE;
                                }
                            }
                        }

                        /* Scream in pain */
                        message(m_name, 0x03);
                        message(" wails out in pain!", 0);

                        /* Take damage from the quake */
                        damage = damroll(4, 8);

                        /* Monster is certainly awake */
                        m_ptr->csleep = 0;

                        /* This is NOT player induced damage */
                        m_ptr->hp -= damage;

                        /* Hack -- If totally embedded, die instantly */
                        if (kill) m_ptr->hp = -1;

                        /* Unique monsters will not quite die */
                        if ((r_ptr->rflags1 & RF1_UNIQUE) && (m_ptr->hp < 0)) {
                            m_ptr->hp = 0;
                        }

                        /* Handle "dead monster" */
                        if (m_ptr->hp < 0) {

                            message(m_name, 0x03);
                            message(" is embedded in the rock.", 0);

                            /* Kill the monster */
                            monster_death(m_ptr, FALSE, m_ptr->ml);

                            /* Delete it */
                            delete_monster_idx(c_ptr->m_idx);
                        }
                    }
                }

                /* Destroy location (if valid) */
                if (valid_grid(i, j)) {

                    bool floor = floor_grid_bold(i, j);

                    /* Forget the current walls */
                    c_ptr->info &= ~GRID_WALL_MASK;

                    /* Floor grids turn into rubble.  */
                    if (floor) {
                        tmp = randint(10);
                        if (tmp < 6) c_ptr->info |= GRID_WALL_QUARTZ;
                        else if (tmp < 9) c_ptr->info |= GRID_WALL_MAGMA;
                        else c_ptr->info |= GRID_WALL_GRANITE;
                    }

                    /* Delete any object that is still there */
                    delete_object(i, j);

                    /* Hack -- not lit, not known, not in a room */
                    c_ptr->info &= ~GRID_ROOM;
                    c_ptr->info &= ~GRID_GLOW;
                    c_ptr->info &= ~GRID_MARK;
                }
            }
        }
    }


    /* Update stuff */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
    
    /* Redraw stuff */
    p_ptr->redraw |= (PR_MAP | PR_BLOCK);
}


/*
 * Attack the player via physical attacks.
 */
bool make_attack_normal(int m_idx)
{
    int			ap_cnt;
    int			method, effect, d_dice, d_side;

    int			i, j, tmp, armor, damage;
    int			do_cut, do_stun;

    s32b		gold;

    monster_type	*m_ptr;
    monster_race	*r_ptr;
    monster_lore	*l_ptr;

    inven_type		*i_ptr;

    char		m_name[80];

    char		ddesc[80];

    /* Blink after theft */
    bool blinked = FALSE;


    /* Note player death */
    if (death) return (FALSE);

    /* Extract the effective armor */
    armor = p_ptr->pac + p_ptr->ptoac;


    /* Acquire monster info */
    m_ptr = &m_list[m_idx];
    r_ptr = &r_list[m_ptr->r_idx];
    l_ptr = &l_list[m_ptr->r_idx];

    /* Not allowed to attack */
    if (r_ptr->rflags1 & RF1_NEVER_BLOW) return (FALSE);
    

    /* Get the monster name (or "it") */
    monster_desc(m_name, m_ptr, 0);

    /* Get the "died from" information (i.e. "a kobold") */
    monster_desc(ddesc, m_ptr, 0x88);


    /* Scan through all four blows */
    for (ap_cnt = 0; ap_cnt < 4; ap_cnt++) {

        bool visible = FALSE;
        bool notice = FALSE;

        bool flag = FALSE;

        
        /* Extract the attack infomation */
        effect = r_ptr->blow[ap_cnt].effect;
        method = r_ptr->blow[ap_cnt].method;
        d_dice = r_ptr->blow[ap_cnt].d_dice;
        d_side = r_ptr->blow[ap_cnt].d_side;


        /* Extract visibility (before blink) */
        if (m_ptr->ml) visible = TRUE;

        /* Assume we will notice visible attacks */
        if (visible) notice = TRUE;


        /* Hack -- no more attacks */
        if (!method) break;
        

        /* Apply "protection from evil" */
        if ((p_ptr->protevil > 0) && 
            (r_ptr->rflags3 & RF3_EVIL) &&
            ((p_ptr->lev + 1) > r_ptr->level) &&
            (randint(100) + (p_ptr->lev) > 50)) {

            /* Remember the Evil-ness */
            if (m_ptr->ml) l_ptr->flags3 |= RF3_EVIL;

            /* Hack -- Repelled */
            effect = 99;

            /* Hack -- Repelled */
            method = 99;
        }


        /* See if the attack "succeeds" */
        switch (effect) {
        
          case 0:
            flag = TRUE;
            break;

          case RBE_HURT:
            if (test_hit(60, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_POISON:
            if (test_hit(5, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_UN_BONUS:
            if (test_hit(20, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_UN_POWER:
            if (test_hit(15, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_EAT_GOLD:
            if (test_hit(5, r_ptr->level, 0, p_ptr->lev, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_EAT_ITEM:
            if (test_hit(2, r_ptr->level, 0, p_ptr->lev, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_EAT_FOOD:
            if (test_hit(5, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_EAT_LITE:
            if (test_hit(5, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_ACID:
            if (test_hit(0, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_ELEC:
            if (test_hit(10, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_FIRE:
            if (test_hit(10, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_COLD:
            if (test_hit(10, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_BLIND:
            if (test_hit(2, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_CONFUSE:
            if (test_hit(10, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_TERRIFY:
            if (test_hit(10, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_PARALYZE:
            if (test_hit(2, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_LOSE_STR:
            if (test_hit(0, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_LOSE_DEX:
            if (test_hit(0, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_LOSE_CON:
            if (test_hit(0, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_LOSE_INT:
            if (test_hit(0, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_LOSE_WIS:
            if (test_hit(0, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_LOSE_CHR:
            if (test_hit(0, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_LOSE_ALL:
            if (test_hit(2, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;

          case RBE_XXX1:
            flag = TRUE;
            break;
            
          case RBE_EXP_10:
            if (test_hit(5, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_EXP_20:
            if (test_hit(5, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_EXP_40:
            if (test_hit(5, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case RBE_EXP_80:
            if (test_hit(5, r_ptr->level, 0, armor, CLA_MISC_HIT))
                flag = TRUE;
            break;
            
          case 99:		/* Repelled */
            flag = TRUE;
            break;

          default:
            /* XXX icky attack */
            break;
        }


        /* Monster hit player */
        if (flag) {

            /* Always disturbing */
            disturb(1, 0);

            /* No cut or stun yet */
            do_cut = do_stun = 0;

            /* Describe the attack method */
            switch (method) {

              case 0:
                break;
                
              case RBM_HIT:
                message(m_name, 0x03);
                message(" hits you.", 0);
                do_cut = do_stun = 1;
                break;

              case RBM_TOUCH:
                message(m_name, 0x03);
                message(" touches you.", 0);
                break;

              case RBM_PUNCH:
                message(m_name, 0x03);
                message(" punches you.", 0);
                break;

              case RBM_KICK:
                message(m_name, 0x03);
                message(" kicks you.", 0);
                break;

              case RBM_CLAW:
                message(m_name, 0x03);
                message(" claws you.", 0);
                do_cut = 1;
                break;

              case RBM_BITE:
                message(m_name, 0x03);
                message(" bites you.", 0);
                do_cut = 1;
                break;

              case RBM_STING:
                message(m_name, 0x03);
                message(" stings you.", 0);
                break;

              case RBM_XXX1:
                break;
                
              case RBM_BUTT:
                message(m_name, 0x03);
                message(" butts you.", 0);
                do_stun = 1;
                break;
                
              case RBM_CRUSH:
                message(m_name, 0x03);
                message(" crushes you.", 0);
                break;
                
              case RBM_ENGULF:
                message(m_name, 0x03);
                message(" engulfs you.", 0);
                break;
                
              case RBM_XXX2:
                break;
                
              case RBM_CRAWL:
                message(m_name, 0x03);
                message(" crawls on you.", 0);
                break;
                
              case RBM_DROOL:
                message(m_name, 0x03);
                message(" drools on you.", 0);
                break;
                
              case RBM_SPIT:
                message(m_name, 0x03);
                message(" spits on you.", 0);
                break;
                
              case RBM_XXX3:
                break;
                
              case RBM_GAZE:
                message(m_name, 0x03);
                message(" gazes at you.", 0);
                break;

              case RBM_WAIL:
                message(m_name, 0x03);
                message(" makes a horrible wail.", 0);
                break;
                
              case RBM_SPORE:
                message(m_name, 0x03);
                message(" releases a cloud of spores.", 0);
                break;
                
              case RBM_XXX4:
                break;
                
              case RBM_BEG:
                message(m_name, 0x03);
                message(" begs you for money.", 0);
                break;
                
              case RBM_INSULT:
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
                
              case RBM_MOAN:
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
                
              case RBM_XXX5:
                break;
                
              case 99:
                message(m_name, 0x03);
                message(" is repelled.", 0);
                break;
                
              default:
                /* XXX icky attack */
                break;
            }


            /* Roll out the damage */
            damage = damroll(d_dice, d_side);

            /* Apply appropriate damage */
            switch (effect) {

              case 0:
                break;

              case RBE_HURT:

                /* Reduce damage based on the player armor class */
                damage -= damage * (((armor > 150) ? 150 : armor) * 3 / 4) / 200;

                /* Take damage */
                take_hit(damage, ddesc);

                /* XXX XXX XXX Mega-Hack -- cause earthquakes */
                if ((damage > 23) && (r_ptr->rflags2 & RF2_DESTRUCT)) {

                    /* Earthquake centered at the monster */
                    shatter_quake(m_ptr->fy, m_ptr->fx);
                }

                break;

              case RBE_POISON:

                take_hit(damage, ddesc);
                if (!p_ptr->immune_pois &&
                    !p_ptr->resist_pois &&
                    !p_ptr->oppose_pois) {
                    msg_print("You feel very sick.");
                    p_ptr->poisoned += randint((int)r_ptr->level) + 5;
                }
                else {
                    msg_print("The poison has no effect.");
                }
                break;

              case RBE_UN_BONUS:

                /* Allow complete resist */
                if (!p_ptr->resist_disen) {

                    /* Take some damage */
                    take_hit(damage, ddesc);

                    /* Apply disenchantment */
                    if (!apply_disenchant(0)) notice = FALSE;
                }
                update_smart_learn(m_idx, DRS_DISEN);
                break;

              case RBE_UN_POWER:

                i = rand_int(inven_ctr);
                j = r_ptr->level;
                i_ptr = &inventory[i];
                if (((i_ptr->tval == TV_STAFF) || (i_ptr->tval == TV_WAND)) &&
                    (i_ptr->pval > 0)) {
                    m_ptr->hp += j * i_ptr->pval;
                    i_ptr->pval = 0;
                    if (!inven_known_p(i_ptr)) {
                        i_ptr->ident |= ID_EMPTY;
                    }
                    msg_print("Energy drains from your pack!");
                }
                else {
                    notice = FALSE;
                }
                break;

              case RBE_EAT_GOLD:

                /* immune to steal at 18/150 */
                if ((p_ptr->paralysis < 1) &&
                    (randint(168) < p_ptr->use_stat[A_DEX])) {
                    msg_print("You quickly protect your money pouch!");
                }
                else {

                    char t1[80];

                    gold = (p_ptr->au / 10) + randint(25);
                    if (gold < 2) gold = 2;
                    if (gold > 5000) gold = 2000 + randint(1000) + (p_ptr->au / 20);
                    if (gold > p_ptr->au) gold = p_ptr->au;
                    p_ptr->au -= gold;
                    if (gold <= 0) {
                        message("Nothing was stolen.", 0);
                    }
                    else if (p_ptr->au) {
                        msg_print("Your purse feels lighter.");
                        sprintf(t1, "%ld coins were stolen!", (long)gold);
                        message(t1, 0);
                    }
                    else {
                        msg_print("Your purse feels lighter.");
                        message("All of your coins were stolen!", 0);
                    }
                    p_ptr->redraw |= PR_BLOCK;
                    blinked = TRUE;
                }

                /* Occasional blink anyway */
                if (rand_int(3)) blinked = TRUE;

                break;

              case RBE_EAT_ITEM:

                /* immune to steal at 18/150 dexterity */
                if ((p_ptr->paralysis < 1) &&
                    (randint(168) < p_ptr->use_stat[A_DEX])) {
                    msg_print("You grab hold of your backpack!");
                }
                else {

                    int		amt;
                    char	t1[160];
                    char	t2[160];

                    /* Steal a single item from the pack */
                    i = rand_int(inven_ctr);

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
                    
                    /* Blink away */
                    blinked = TRUE;
                }

                /* Occasional "blink" anyway */
                if (rand_int(3) == 0) blinked = TRUE;
                
                break;

              case RBE_EAT_FOOD:

                if (find_range(TV_FOOD, &i, &j)) {
                    int arg = rand_range(i,j);
                    inven_item_increase(arg, -1);
                    inven_item_optimize(arg);
                    msg_print("It got at your rations!");
                }
                else {
                    notice = FALSE;
                }
                break;

              case RBE_EAT_LITE:

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

              case RBE_ACID:

                msg_print("You are covered in acid!");
                acid_dam(damage, ddesc);
                update_smart_learn(m_idx, DRS_ACID);
                break;

              case RBE_ELEC:

                msg_print("You are struck by electricity!");
                elec_dam(damage, ddesc);
                update_smart_learn(m_idx, DRS_ELEC);
                break;

              case RBE_FIRE:

                msg_print("You are enveloped in flames!");
                fire_dam(damage, ddesc);
                update_smart_learn(m_idx, DRS_FIRE);
                break;

              case RBE_COLD:

                msg_print("You are covered with frost!");
                cold_dam(damage, ddesc);
                update_smart_learn(m_idx, DRS_COLD);
                break;

              case RBE_BLIND:

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

              case RBE_CONFUSE:

                take_hit(damage, ddesc);
                if ((!p_ptr->resist_conf) && (!p_ptr->resist_chaos)) {
                    if (rand_int(2) == 0) {
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

              case RBE_TERRIFY:

                take_hit(damage, ddesc);
                if (player_saves() ||
                    ((p_ptr->pclass == 1) && (rand_int(3) == 0)) ||
                    p_ptr->resist_fear) {
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

              case RBE_PARALYZE:

                take_hit(damage, ddesc);
                if (player_saves()) {
                    msg_print("You resist the effects!");
                }
                else if (p_ptr->paralysis < 1) {
                    if (p_ptr->free_act) {
                        msg_print("You are unaffected.");
                    }
                    else {
                        p_ptr->paralysis = randint((int)r_ptr->level) + 3;
                        msg_print("You are paralysed.");
                    }
                }
                else {
                    notice = FALSE;
                }
                break;

              case RBE_LOSE_STR:

                take_hit(damage, ddesc);
                if (p_ptr->sustain_str) {
                    msg_print("You feel weaker for a moment, but it passes.");
                }
                else {
                    msg_print("You feel weaker.");
                    (void)dec_stat(A_STR, 15, FALSE);
                }
                break;

              case RBE_LOSE_INT:

                take_hit(damage, ddesc);
                msg_print("You have trouble thinking clearly.");
                if (p_ptr->sustain_int) {
                    msg_print("But your mind quickly clears.");
                }
                else {
                    (void)dec_stat(A_INT, 15, FALSE);
                }
                break;

              case RBE_LOSE_WIS:

                take_hit(damage, ddesc);
                if (p_ptr->sustain_wis) {
                    msg_print("Your wisdom is sustained.");
                }
                else {
                    msg_print("Your wisdom is drained.");
                    (void)dec_stat(A_WIS, 15, FALSE);
                }
                break;

              case RBE_LOSE_DEX:

                take_hit(damage, ddesc);
                if (p_ptr->sustain_dex) {
                    msg_print("You feel clumsy for a moment, but it passes.");
                }
                else {
                    msg_print("You feel more clumsy.");
                    (void)dec_stat(A_DEX, 15, FALSE);
                }
                break;

              case RBE_LOSE_CON:

                take_hit(damage, ddesc);
                if (p_ptr->sustain_con) {
                    msg_print("Your body resists the effects of the disease.");
                }
                else {
                    msg_print("Your health is damaged!");
                    (void)dec_stat(A_CON, 15, FALSE);
                }
                break;

              case RBE_LOSE_CHR:

                take_hit(damage, ddesc);
                if (p_ptr->sustain_chr) {
                    msg_print("You keep your good looks.");
                }
                else {
                    msg_print("Your features are twisted.");
                    (void)dec_stat(A_CHR, 15, FALSE);
                }
                break;

              case RBE_LOSE_ALL:

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

              case RBE_XXX1:

                break;

              case RBE_EXP_10:

                take_hit(damage, ddesc);
                if (p_ptr->hold_life) {
                    msg_print("You keep hold of your life force!");
                }
                else {
                    s32b d = damroll(10,6) + (p_ptr->exp/100) * MON_DRAIN_LIFE;
                    msg_print("You feel your life draining away!");
                    lose_exp(d);
                }
                break;

              case RBE_EXP_20:

                take_hit(damage, ddesc);
                if (p_ptr->hold_life) {
                    msg_print("You keep hold of your life force!");
                }
                else {
                    s32b d = damroll(20,6) + (p_ptr->exp/100) * MON_DRAIN_LIFE;
                    if (p_ptr->hold_life) {
                        msg_print("You feel your life slipping away!");
                        lose_exp(d/10);
                    }
                    else {
                        msg_print("You feel your life draining away!");
                        lose_exp(d);
                    }
                }
                break;

              case RBE_EXP_40:

                take_hit(damage, ddesc);
                if (p_ptr->hold_life && rand_int(4)) {
                    msg_print("You keep hold of your life force!");
                }
                else {
                    s32b d = damroll(40,6) + (p_ptr->exp/100) * MON_DRAIN_LIFE;
                    if (p_ptr->hold_life) {
                        msg_print("You feel your life slipping away!");
                        lose_exp(d/10);
                    }
                    else {
                        msg_print("You feel your life draining away!");
                        lose_exp(d);
                    }
                }
                break;

              case RBE_EXP_80:

                take_hit(damage, ddesc);
                if (p_ptr->hold_life && rand_int(4)) {
                    msg_print("You keep hold of your life force!");
                }
                else {
                    s32b d = damroll(80,6) + (p_ptr->exp/100) * MON_DRAIN_LIFE;
                    if (p_ptr->hold_life) {
                        msg_print("You feel your life slipping away!");
                        lose_exp(d/10);
                    }
                    else {
                        msg_print("You feel your life draining away!");
                        lose_exp(d);
                    }
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
            /* Note that "tmp" is unused if (!do_cut && !do_stun) */

            /* Hack -- only one of cut or stun */
            if (do_cut && do_stun) {

                /* Cancel cut */
                if (rand_int(100) < 50) {
                    do_cut = 0;
                }

                /* Cancel stun */
                else {
                    do_stun = 0;
                }
            }
            
            /* Critical hit (zero if non-critical) */
            tmp = monster_critical(d_dice, d_side, damage);

            /* Critical Cut (note check for "do_cut==0") */
            switch (do_cut * tmp) {
              case 0: break;
              case 1: cut_player(randint(5)); break;
              case 2: cut_player(randint(5) + 5); break;
              case 3: cut_player(randint(30) + 20); break;
              case 4: cut_player(randint(70) + 30); break;
              case 5: cut_player(randint(250) + 50); break;
              case 6: cut_player(300); break;
              default: cut_player(500); break;
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


            /* Hack -- Apply "glowing hands" */
            if (p_ptr->confusing) {

                msg_print("Your hands stop glowing.");
                p_ptr->confusing = FALSE;

                if (randint(100) < r_ptr->level) {
                    message(m_name, 0x03);
                    message(" is unaffected.", 0);
                }
                else if (r_ptr->rflags3 & RF3_NO_CONF) {
                    if (visible) l_ptr->flags3 |= RF3_NO_CONF;
                    message(m_name, 0x03);
                    message(" is unaffected.", 0);
                }
                else {
                    message(m_name, 0x03);
                    message(" appears confused.", 0);
                    m_ptr->confused = TRUE;
                }
            }


            /* Notice attack if noticed, or previously noticed */
            if (notice || (l_ptr->blows[ap_cnt] && (effect != 99))) {

                /* Count attacks of this type */
                if (l_ptr->blows[ap_cnt] < MAX_UCHAR) {
                    l_ptr->blows[ap_cnt]++;
                }
            }
        }

        /* Monster missed player */
        else {

            /* XXX XXX XXX Notice invisible monsters? */

            /* XXX XXX XXX Take notes on failed attacks? */

            /* Only describe some failed attacks */
            if ((method == 1) || (method == 2) ||
                (method == 3) || (method == 6)) {

                disturb(1, 0);
                
                message(m_name, 0x03);
                message(" misses you.", 0);
            }
        }

        
        /* Player is dead */
        if (death) break;
    }


    /* Blink away */
    if (blinked) {
        msg_print("There is a puff of smoke!");
        teleport_away(m_idx, MAX_SIGHT + 5);
    }


    /* Always notice cause of death */
    if (death && (l_ptr->deaths < MAX_SHORT)) l_ptr->deaths++;


    /* Assume we attacked */
    return (TRUE);
}


/*
 * Cast a bolt at the player
 */
static void bolt(int m_idx, int typ, int dam_hp)
{
    int y = py, x = px;

    int flg = PROJECT_STOP;

    monster_type *m_ptr = &m_list[m_idx];
    
#ifdef WDT_TRACK_OPTIONS

    /* Tracking a target */
    if (track_target) {

        /* Use the target */
        y = m_ptr->ty;
        x = m_ptr->tx;

        /* Verify target */
        if (!(m_ptr->t_bit & MTB_DIRECT)) return;
    }

#endif

    /* Go towards player, hit people in the way */
    (void)project(m_idx, 0, y, x, dam_hp, typ, flg);
}


/*
 * Cast a breath (or ball) attack
 */
static void breath(int m_idx, int typ, int dam_hp)
{
    int max_dis, y = py, x = px;

    int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_XTRA;

    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_list[m_ptr->r_idx];

    /* Determine the radius of the blast */
    max_dis = (r_ptr->rflags2 & RF2_POWERFUL) ? 3 : 2;

#ifdef WDT_TRACK_OPTIONS

    /* Tracking a target */
    if (track_target) {

        /* Use the target */
        y = m_ptr->ty;
        x = m_ptr->tx;

        /* Verify target */
        if (!(m_ptr->t_bit & MTB_DIRECT)) return;
    }

#endif

    /* Go towards player, do not hit anyone else, hurt items on ground. */
    (void)project(m_idx, max_dis, y, x, dam_hp, typ, flg);
}





/*
 * Creatures can cast spells (and breathe) too.
 * Returns "TRUE" if a spell was (successfully) cast
 *
 * Blindness check is iffy.  What if the monster is out of sight?
 * XXX We should probably use "if (!seen)" instead of "if (blind)"
 *
 * Total rewrite needed for new monster race flags
 */
bool make_attack_spell(int m_idx)
{
    u32b		i, f4, f5, f6;
    int			k, chance, thrown_spell, r1;
    int			spell_choice[64];

    bool		recovered = FALSE;

    monster_type	*m_ptr = &m_list[m_idx];
    monster_race	*r_ptr = &r_list[m_ptr->r_idx];

    monster_lore	*l_ptr;

    char		m_name[80];
    char		m_poss[80];
    char		m_self[80];

    char		ddesc[80];


    /* Extract the blind-ness -CFT */
    int blind = (p_ptr->blind);

    /* Extract the "see-able-ness" */
    int seen = (!blind && m_ptr->ml);

    /* Assume line of sight */
    int direct = TRUE;

    /* Summon variables */
    int x, y, count = 0;


    /* Target the Player location */
    x = px;
    y = py;


#ifdef WDT_TRACK_OPTIONS

    /* Target a different location */
    if (track_target) {

        /* Use the target */
        x = m_ptr->tx;
        y = m_ptr->ty;

        /* Verify "duration" */
        if (!m_ptr->t_dur) return (FALSE);

#if 0
        /* Hack -- Ignore spell if player cannot see end-points */
        if (!player_has_los_bold(m_ptr->fy, m_ptr->fx) &&
            !player_has_los_bold(y, x)) {
            
            /* Who cares */
            return (FALSE);
        }
#endif

        /* Allow "direct tracking" */
        if ((m_ptr->t_bit & MTB_PLAYER) &&
            (m_ptr->t_bit & MTB_DIRECT)) {
            direct = TRUE;
        }
     }

#endif


    /* Cannot cast spells when confused */
    if (m_ptr->confused) return (FALSE);

    /* Must be within certain range */
    if (m_ptr->cdis > MAX_RANGE) return (FALSE);

    /* Hack -- Extract the spell probability */
    chance = (r_ptr->freq_inate + r_ptr->freq_spell) / 2;

    /* Not allowed to cast spells */
    if (!chance) return (FALSE);

    /* Only do spells occasionally */
    if (rand_int(100) >= chance) return (FALSE);

    /* Hack -- make sure spell could arrive as a "bolt" */
    if (!projectable(m_ptr->fy, m_ptr->fx, y, x)) return (FALSE);

    /* Hack -- Ignore dead player */
    if (death) return (FALSE);


    /* Get the monster lore */
    l_ptr = &l_list[m_ptr->r_idx];

    /* Get the monster name (or "it") */
    monster_desc(m_name, m_ptr, 0x00);

    /* Get the monster's possessive and reflexive (sexed if visible) */
    monster_desc(m_poss, m_ptr, 0x22);
    monster_desc(m_self, m_ptr, 0x23);

    /* Get the "died from" name */
    monster_desc(ddesc, m_ptr, 0x88);

    /* Extract the racial spell flags */
    f4 = r_ptr->rflags4;
    f5 = r_ptr->rflags5;
    f6 = r_ptr->rflags6;

    /* Get desperate when intelligent and about to die */
    if ((r_ptr->rflags2 & RF2_SMART) &&
        (m_ptr->hp < ((r_ptr->hdice * r_ptr->hside) / 10)) &&
        rand_int(2)) {

        /* Require intelligent spells */
        f4 &= RF4_INT_MASK;
        f5 &= RF5_INT_MASK;
        f6 &= RF6_INT_MASK;

        /* No spells left */
        if (!f4 && !f5 && !f6) return (FALSE);
    }


#ifdef DRS_SMART_OPTIONS

    /* Remove the "ineffective" spells */
    remove_bad_spells(m_idx, &f4, &f5, &f6);

#endif


    /* No spells yet */
    k = 0;

    /* Extract the spells */
    for (i = f4; i; ) spell_choice[k++] = bit_pos(&i) + 32*3;
    for (i = f5; i; ) spell_choice[k++] = bit_pos(&i) + 32*4;
    for (i = f6; i; ) spell_choice[k++] = bit_pos(&i) + 32*5;

    /* Nothing to cast */
    if (!k) return (FALSE);


    /* Choose a spell to cast */
    thrown_spell = spell_choice[rand_int(k)];

    /* Hack -- disturb the player */
    disturb(1, 0);


    /* Cast the spell. */
    switch (thrown_spell) {

      case 96+0:    /* RF4_SHRIEK */
        message(m_name, 0x03);
        message(" makes a high pitched shriek.", 0);
        aggravate_monsters();
        break;
        
      case 96+1:    /* RF4_XXX2X4 */
        break;
        
      case 96+2:    /* RF4_XXX3X4 */
        break;
        
      case 96+3:    /* RF4_XXX4X4 */
        break;
        
      case 96+4:    /* RF4_ARROW_1 */
        message(m_name, 0x03);
        if (blind) message(" makes a strange noise.", 0);
        else message(" fires an arrow at you.", 0);
        bolt(m_idx, GF_ARROW, damroll(1, 6));
        break;
        
      case 96+5:    /* RF4_ARROW_2 */
        message(m_name, 0x03);
        if (blind) message(" makes a strange noise.", 0);
        else message(" fires an arrow at you.", 0);
        bolt(m_idx, GF_ARROW, damroll(3, 6));
        break;
        
      case 96+6:    /* RF4_ARROW_3 */
        message(m_name, 0x03);
        if (blind) message(" makes a strange noise.", 0);
        else message(" fires missiles at you.", 0);
        bolt(m_idx, GF_ARROW, damroll(5, 6));
        break;
        
      case 96+7:    /* RF4_ARROW_4 */
        message(m_name, 0x03);
        if (blind) message(" makes a strange noise.", 0);
        else message(" fires missiles at you.", 0);
        bolt(m_idx, GF_ARROW, damroll(7, 6));
        break;
        
      case 96+8:    /* RF4_BR_ACID */
        message(m_name, 0x03);
        if (blind) message(" breathes.", 0);
        else message(" breathes acid.", 0);
        breath(m_idx, GF_ACID,
               ((m_ptr->hp / 3) > 1600 ? 1600 : (m_ptr->hp / 3)));
        update_smart_learn(m_idx, DRS_ACID);
        break;
        
      case 96+9:    /* RF4_BR_ELEC */
        message(m_name, 0x03);
        if (blind) message(" breathes.", 0);
        else message(" breathes lightning.", 0);
        breath(m_idx, GF_ELEC,
               ((m_ptr->hp / 3) > 1600 ? 1600 : (m_ptr->hp / 3)));
        update_smart_learn(m_idx, DRS_ELEC);
        break;
        
      case 96+10:    /* RF4_BR_FIRE */
        message(m_name, 0x03);
        if (blind) message(" breathes.", 0);
        else message(" breathes fire.", 0);
        breath(m_idx, GF_FIRE,
               ((m_ptr->hp / 3) > 1600 ? 1600 : (m_ptr->hp / 3)));
        update_smart_learn(m_idx, DRS_FIRE);
        break;
        
      case 96+11:    /* RF4_BR_COLD */
        message(m_name, 0x03);
        if (blind) message(" breathes.", 0);
        else message(" breathes frost.", 0);
        breath(m_idx, GF_COLD,
               ((m_ptr->hp / 3) > 1600 ? 1600 : (m_ptr->hp / 3)));
        update_smart_learn(m_idx, DRS_COLD);
        break;
        
      case 96+12:    /* RF4_BR_POIS */
        message(m_name, 0x03);
        if (blind) message(" breathes.", 0);
        else message(" breathes gas.", 0);
        breath(m_idx, GF_POIS,
               ((m_ptr->hp / 3) > 800 ? 800 : (m_ptr->hp / 3)));
        update_smart_learn(m_idx, DRS_POIS);
        break;
        
      case 96+13:    /* RF4_BR_NETH */
        message(m_name, 0x03);
        if (blind) message(" breathes.", 0);
        else message(" breathes nether.", 0);
        breath(m_idx, GF_NETHER,
               ((m_ptr->hp / 6) > 550 ? 550 : (m_ptr->hp / 6)) );
        update_smart_learn(m_idx, DRS_NETH);
        break;
        
      case 96+14:    /* RF4_BR_LITE */
        message(m_name, 0x03);
        if (blind) message(" breathes.", 0);
        else message(" breathes light.", 0);
        breath(m_idx, GF_LITE,
            ((m_ptr->hp / 6) > 400 ? 400 : (m_ptr->hp / 6)));
        update_smart_learn(m_idx, DRS_LITE);
        break;
        
      case 96+15:    /* RF4_BR_DARK */
        message(m_name, 0x03);
        if (blind) message(" breathes.", 0);
        else message(" breathes darkness.", 0);
        breath(m_idx, GF_DARK,
            ((m_ptr->hp / 6) > 400 ? 400 : (m_ptr->hp / 6)));
        update_smart_learn(m_idx, DRS_DARK);
        break;
        
      case 96+16:    /* RF4_BR_CONF */
        message(m_name, 0x03);
        if (blind) message(" breathes.", 0);
        else message(" breathes confusion.", 0);
        breath(m_idx, GF_CONFUSION,
            ((m_ptr->hp / 6) > 400 ? 400 : (m_ptr->hp / 6)));
        update_smart_learn(m_idx, DRS_CONF);
        update_smart_learn(m_idx, DRS_CHAOS);
        break;
        
      case 96+17:    /* RF4_BR_SOUN */
        message(m_name, 0x03);
        if (blind) message(" breathes.", 0);
        else message(" breathes sound.", 0);
        breath(m_idx, GF_SOUND,
            ((m_ptr->hp / 6) > 400 ? 400 : (m_ptr->hp / 6)));
        update_smart_learn(m_idx, DRS_SOUND);
        break;
        
      case 96+18:    /* RF4_BR_CHAO */
        message(m_name, 0x03);
        if (blind) message(" breathes.", 0);
        else message(" breathes chaos.", 0);
        breath(m_idx, GF_CHAOS,
               ((m_ptr->hp / 6) > 600 ? 600 : (m_ptr->hp / 6)));
        update_smart_learn(m_idx, DRS_CONF);
        update_smart_learn(m_idx, DRS_CHAOS);
        break;
        
      case 96+19:    /* RF4_BR_DISE */
        message(m_name, 0x03);
        if (blind) message(" breathes.", 0);
        else message(" breathes disenchantment.", 0);
        breath(m_idx, GF_DISENCHANT,
            ((m_ptr->hp / 6) > 500 ? 500 : (m_ptr->hp / 6)));
        update_smart_learn(m_idx, DRS_DISEN);
        break;
        
      case 96+20:    /* RF4_BR_NEXU */
        message(m_name, 0x03);
        if (blind) message(" breathes.", 0);
        else message(" breathes Nexus.", 0);
        breath(m_idx, GF_NEXUS,
            ((m_ptr->hp / 3) > 250 ? 250 : (m_ptr->hp / 3)));
        update_smart_learn(m_idx, DRS_NEXUS);
        break;
        
      case 96+21:    /* RF4_BR_TIME */
        message(m_name, 0x03);
        if (blind) message(" breathes.", 0);
        else message(" breathes time.", 0);
        breath(m_idx, GF_TIME,
            ((m_ptr->hp / 3) > 150 ? 150 : (m_ptr->hp / 3)));
        break;
        
      case 96+22:    /* RF4_BR_INER */
        message(m_name, 0x03);
        if (blind) message(" breathes.", 0);
        else message(" breathes inertia.", 0);
        breath(m_idx, GF_INERTIA,
               ((m_ptr->hp / 6) > 200 ? 200 : (m_ptr->hp / 6)));
        break;
        
      case 96+23:    /* RF4_BR_GRAV */
        message(m_name, 0x03);
        if (blind) message(" breathes.", 0);
        else message(" breathes gravity.", 0);
        breath(m_idx, GF_GRAVITY,
            ((m_ptr->hp / 3) > 200 ? 200 : (m_ptr->hp / 3)));
        break;
        
      case 96+24:    /* RF4_BR_SHAR */
        message(m_name, 0x03);
        if (blind) message(" breathes.", 0);
        else message(" breathes shards.", 0);
        breath(m_idx, GF_SHARDS,
            ((m_ptr->hp / 6) > 400 ? 400 : (m_ptr->hp / 6)));
        update_smart_learn(m_idx, DRS_SHARD);
        break;
        
      case 96+25:    /* RF4_BR_PLAS */
        message(m_name, 0x03);
        if (blind) message(" breathes.", 0);
        else message(" breathes plasma.", 0);
        breath(m_idx, GF_PLASMA,
            ((m_ptr->hp / 6) > 150 ? 150 : (m_ptr->hp / 6)));
        break;
        
      case 96+26:    /* RF4_BR_WALL */
        message(m_name, 0x03);
        if (blind) message(" breathes.", 0);
        else message(" breathes elemental force.", 0);

        /* Breath "walls", at PLAYER location */
        /* XXX XXX XXX This should be done as a "beam" via "project()" */
        if (direct && !rand_int(10)) {
            br_wall(y, x);
        }

        /* Normal breath */
        else {
            breath(m_idx, GF_FORCE,
                   ((m_ptr->hp / 6) > 200 ? 200 : (m_ptr->hp / 6)));
        }
        break;
        
      case 96+27:    /* RF4_BR_MANA */
        /* XXX XXX XXX */
        break;
        
      case 96+28:    /* RF4_XXX5X4 */
        break;
        
      case 96+29:    /* RF4_XXX6X4 */
        break;
        
      case 96+30:    /* RF4_XXX7X4 */
        break;
        
      case 96+31:    /* RF4_XXX8X4 */
        break;
        


      case 128+0:    /* RF5_BA_ACID */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" casts an Acid Ball.", 0);
        breath(m_idx, GF_ACID,
               randint(r_ptr->level * 3) + 15 );
        update_smart_learn(m_idx, DRS_ACID);
        break;
        
      case 128+1:    /* RF5_BA_ELEC */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" casts a Lightning Ball.", 0);
        breath(m_idx, GF_ELEC,
            randint((r_ptr->level * 3) / 2) + 8);
        update_smart_learn(m_idx, DRS_ELEC);
        break;
        
      case 128+2:    /* RF5_BA_FIRE */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" casts a Fire ball.", 0);
        breath(m_idx, GF_FIRE,
               randint((r_ptr->level * 7) / 2) + 10);
        update_smart_learn(m_idx, DRS_FIRE);
        break;
        
      case 128+3:    /* RF5_BA_COLD */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" casts a Frost ball.", 0);
        breath(m_idx, GF_COLD,
               randint((r_ptr->level * 3) / 2) + 10);
        update_smart_learn(m_idx, DRS_COLD);
        break;
        
      case 128+4:    /* RF5_BA_POIS */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" casts a Stinking Cloud.", 0);
        breath(m_idx, GF_POIS,
               damroll(12, 2));
        update_smart_learn(m_idx, DRS_POIS);
        break;
        
      case 128+5:    /* RF5_BA_NETH */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" casts a Nether Ball.", 0);
        breath(m_idx, GF_NETHER,
               (50 + damroll(10, 10) + (r_ptr->level)));
        update_smart_learn(m_idx, DRS_NETH);
        break;
        
      case 128+6:    /* RF5_BA_WATE */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" gestures fluidly.", 0);
        msg_print("You are engulfed in a whirlpool.");
        breath(m_idx, GF_WATER,
               randint((r_ptr->level * 5) / 2) + 50);
        break;
        
      case 128+7:    /* RF5_BA_MANA */
        message(m_name, 0x03);
        if (blind) message(" mumbles powerfully.", 0);
        else message(" invokes a Mana Storm.", 0);
        breath(m_idx, GF_MANA,
               (r_ptr->level * 5) + damroll(10, 10));
        break;
        
      case 128+8:    /* RF5_BA_DARK */
        message(m_name, 0x03);
        if (blind) message(" mumbles powerfully.", 0);
        else message(" casts a Darkness Storm.", 0);
        breath(m_idx, GF_DARK,
            ((m_ptr->hp / 6) > 500 ? 500 : (m_ptr->hp / 6)));
        update_smart_learn(m_idx, DRS_DARK);
        break;
        
      case 128+9:    /* RF5_DRAIN_MANA */
        if (!direct) break;
        if (p_ptr->cmana) {
            disturb(1, 0);
            message(m_name, 0x03);
            message(" draws psychic energy from you!", 0);
            if (seen) {
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
            p_ptr->redraw |= PR_MANA;
            m_ptr->hp += 6 * (r1);
        }
        update_smart_learn(m_idx, DRS_MANA);
        break;
        
      case 128+10:    /* RF5_MIND_BLAST */
        if (!direct) break;
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
                if (p_ptr->confused) {
                    p_ptr->confused += 2;
                }
                else {
                    p_ptr->confused = rand_int(5) + 4;
                }
            }
            take_hit(damroll(8, 8), ddesc);
        }
        break;
        
      case 128+11:    /* RF5_BRAIN_SMASH */
        if (!direct) break;
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
                if (p_ptr->confused) {
                    p_ptr->confused += 2;
                }
                else {
                    p_ptr->confused = rand_int(5) + 4;
                }
            }
            if (!p_ptr->free_act) {
                if (p_ptr->paralysis) {
                    p_ptr->paralysis += 2;
                }
                else {
                    p_ptr->paralysis = rand_int(5) + 4;
                }
                if (p_ptr->slow) {
                    p_ptr->slow += 2;
                }
                else {
                    p_ptr->slow = rand_int(5) + 4;
                }
            }
            if (!p_ptr->resist_blind) {
                if (p_ptr->blind) {
                    p_ptr->blind += 6;
                }
                else {
                    p_ptr->blind += rand_int(3) + 13;
                }
            }
        }
        break;
        
      case 128+12:    /* RF5_CAUSE_1 */
        if (!direct) break;
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" points at you and curses.", 0);
        if (player_saves()) msg_print("You resist the effects of the spell.");
        else take_hit(damroll(3, 8), ddesc);
        break;
        
      case 128+13:    /* RF5_CAUSE_2 */
        if (!direct) break;
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" points at you and curses horribly.", 0);
        if (player_saves()) msg_print("You resist the effects of the spell.");
        else take_hit(damroll(8, 8), ddesc);
        break;
        
      case 128+14:    /* RF5_CAUSE_3 */
        if (!direct) break;
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
        
      case 128+15:    /* RF5_CAUSE_4 */
        if (!direct) break;
        message(m_name, 0x03);
        if (blind) message(" screams the word 'DIE!'", 0);
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
        
      case 128+16:    /* RF5_BO_ACID */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" casts a Acid bolt.", 0);
        bolt(m_idx, GF_ACID,
             damroll(7, 8) + (r_ptr->level / 3));
        update_smart_learn(m_idx, DRS_ACID);
        break;
        
      case 128+17:    /* RF5_BO_ELEC */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" casts a Lightning bolt.", 0);
        bolt(m_idx, GF_ELEC,
             damroll(4, 8) + (r_ptr->level / 3));
        update_smart_learn(m_idx, DRS_ELEC);
        break;
        
      case 128+18:    /* RF5_BO_FIRE */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" casts a Fire bolt.", 0);
        bolt(m_idx, GF_FIRE,
             damroll(9, 8) + (r_ptr->level / 3));
        update_smart_learn(m_idx, DRS_FIRE);
        break;
        
      case 128+19:    /* RF5_BO_COLD */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" casts a Frost bolt.", 0);
        bolt(m_idx, GF_COLD,
             damroll(6, 8) + (r_ptr->level / 3));
        update_smart_learn(m_idx, DRS_COLD);
        break;
        
      case 128+20:    /* RF5_BO_POIS */
        /* XXX XXX XXX */
        break;
        
      case 128+21:    /* RF5_BO_NETH */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" casts a Nether Bolt.", 0);
        bolt(m_idx, GF_NETHER,
             30 + damroll(5, 5) + (r_ptr->level * 3) / 2);
        update_smart_learn(m_idx, DRS_NETH);
        break;
        
      case 128+22:    /* RF5_BO_WATE */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" casts a Water Bolt.", 0);
        bolt(m_idx, GF_WATER,
             damroll(10, 10) + (r_ptr->level));
        break;
        
      case 128+23:    /* RF5_BO_MANA */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" casts a Mana bolt.", 0);
        bolt(m_idx, GF_MISSILE,
             randint((r_ptr->level * 7) / 2) + 50);
        break;
        
      case 128+24:    /* RF5_BO_PLAS */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" casts a Plasma Bolt.", 0);
        bolt(m_idx, GF_PLASMA,
             10 + damroll(8, 7) + (r_ptr->level));
        break;
        
      case 128+25:    /* RF5_BO_ICEE */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" casts an Ice Bolt.", 0);
        bolt(m_idx, GF_COLD,
             damroll(6, 6) + (r_ptr->level));
        update_smart_learn(m_idx, DRS_COLD);
        break;
        
      case 128+26:    /* RF5_MISSILE */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" casts a magic missile.", 0);
        bolt(m_idx, GF_MISSILE,
             damroll(2, 6) + (r_ptr->level / 3));
        break;
        
      case 128+27:    /* RF5_SCARE */
        if (!direct) break;
        message(m_name, 0x03);
        if (blind) message(" mumbles, and you hear scary noises.", 0);
        else message(" casts a fearful illusion.", 0);
        if (player_saves() || p_ptr->resist_fear) {
            msg_print("You refuse to be frightened.");
        }
        else if (p_ptr->afraid) {
            p_ptr->afraid += 2;
        }
        else {
            p_ptr->afraid = rand_int(5) + 4;
        }
        update_smart_learn(m_idx, DRS_FEAR);
        break;
        
      case 128+28:    /* RF5_BLIND */
        if (!direct) break;
        message(m_name, 0x03);
        if (blind) message(" mumbles, and your eyes burn even more.", 0);
        else message(" casts a spell, burning your eyes!", 0);
        if ((player_saves()) || (p_ptr->resist_blind)) {
            if (blind) {
                msg_print("But the extra burning quickly fades away.");
            }
            else {
                msg_print("You blink and your vision clears.");
            }
        }
        else if (p_ptr->blind) {
            p_ptr->blind += 6;
        }
        else {
            p_ptr->blind += rand_int(3) + 13;
        }
        update_smart_learn(m_idx, DRS_BLIND);
        break;

      case 128+29:    /* RF5_CONF */
        if (!direct) break;
        message(m_name, 0x03);
        if (blind) message(" mumbles, and you hear puzzling noises.", 0);
        else message(" creates a mesmerising illusion.", 0);
        if ((player_saves()) ||
            (p_ptr->resist_conf) ||
            (p_ptr->resist_chaos)) {
            msg_print("You disbelieve the feeble spell.");
        }
        else if (p_ptr->confused) {
            p_ptr->confused += 2;
        }
        else {
            p_ptr->confused = rand_int(5) + 4;
        }
        update_smart_learn(m_idx, DRS_CONF);
        update_smart_learn(m_idx, DRS_CHAOS);
        break;
        
      case 128+30:    /* RF5_SLOW */
        if (!direct) break;
        message(m_name, 0x03);
        message(" drains power from your muscles!", 0);
        if (p_ptr->free_act) {
            msg_print("You are unaffected.");
        }
        else if (player_saves()) {
            msg_print("Your body resists the spell.");
        }
        else if (p_ptr->slow) {
            p_ptr->slow += 2;
        }
        else {
            p_ptr->slow = rand_int(5) + 4;
        }
        update_smart_learn(m_idx, DRS_FREE);
        break;
        
      case 128+31:    /* RF5_HOLD */
        if (!direct) break;
        message(m_name, 0x03);
        if (blind) message(" mumbles, and you feel something holding you!", 0);
        else message(" gazes deep into your eyes!", 0);
        if (p_ptr->free_act) {
            msg_print("You are unaffected.");
        }
        else if (player_saves()) {
            if (blind) message(" You resist!", 0);
            else message(" You stare back unafraid!", 0);
        }
        else if (p_ptr->paralysis) {
            p_ptr->paralysis += 2;
        }
        else {
            p_ptr->paralysis = rand_int(5) + 5;
        }
        update_smart_learn(m_idx, DRS_FREE);
        break;
        


      case 160+0:    /* RF6_HASTE */
        message(m_name, 0x03);
        if (blind) {
            message(" mumbles to ", 0x02);
            message(m_self, 0x02);
            message(".", 0);
        }
        else {
            message(" casts a spell.", 0);
        }

        /* Hack -- allow one "speed up" above racial max */
        if ((m_ptr->mspeed) <= (r_ptr->speed)) {
            message(m_name, 0x03);
            message(" starts moving faster.", 0);
            m_ptr->mspeed += 10;
        }

        break;
        
      case 160+1:    /* RF6_XXX1X6 */
        break;
        
      case 160+2:    /* RF6_HEAL */
        if (blind) {
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
            message(seen ? " looks" : " sounds", 0x02);
            message(" as healthy as can be.", 0);

            /* can't be afraid at max hp's */
            if (m_ptr->monfear) {
                m_ptr->monfear = 0;
                recovered = TRUE;
            }
        }

        /* Get more healthy */
        else {

            m_ptr->hp += (r_ptr->level) * 6;
            if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

            message(m_name, 0x03);
            message(seen ? " looks" : " sounds", 0x02);

            if (m_ptr->hp == m_ptr->maxhp) {
                message(" REALLY healthy!", 0);

                /* can't be afraid at max hp's */
                if (m_ptr->monfear) {
                    m_ptr->monfear = 0;
                    recovered = TRUE;
                }
            }
            else {
                message(" healthier.", 0);

                /* has recovered 33% of it's hit points */
                if ((m_ptr->monfear) &&
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
        
      case 160+3:    /* RF6_XXX2X6 */
        break;
        
      case 160+4:    /* RF6_BLINK */
        message(m_name, 0x03);
        message(" blinks away.", 0);
        teleport_away(m_idx, 5);
        break;
        
      case 160+5:    /* RF6_TPORT */
        message(m_name, 0x03);
        message(" teleports away.", 0);
        teleport_away(m_idx, MAX_SIGHT + 5);
        break;
        
      case 160+6:    /* RF6_XXX3X6 */
        break;
        
      case 160+7:    /* RF6_XXX4X6 */
        break;
        
      case 160+8:    /* RF6_TELE_TO */
        if (!direct) break;
        message(m_name, 0x03);
        message(" commands you to return!", 0);
        teleport_flag = TRUE;
        teleport_dist = 0;
        teleport_to_y = m_ptr->fy;
        teleport_to_x = m_ptr->fx;
        break;
        
      case 160+9:    /* RF6_TELE_AWAY */
        if (!direct) break;
        message(m_name, 0x03);
        message(" teleports you away.", 0);
        teleport_flag = TRUE;
        teleport_dist = 100;
        break;
        
      case 160+10:    /* RF6_TELE_LEVEL */
        if (!direct) break;
        message(m_name, 0x03);
        if (blind) message(" mumbles strangely.", 0);
        else message(" gestures at you.", 0);
        if (p_ptr->resist_nexus ||
            player_saves() ||
            rand_int(3)) {
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
            else if (rand_int(2)) {
                msg_print("You rise up through the ceiling.");
                dun_level--;
            }
            else {
                msg_print("You sink through the floor.");
                dun_level++;
            }
            new_level_flag = TRUE;
        }
        update_smart_learn(m_idx, DRS_NEXUS);
        break;
        
      case 160+11:    /* RF6_XXX5 */
        break;
        
      case 160+12:    /* RF6_DARKNESS */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" gestures in shadow.", 0);
        (void)unlite_area(y, x);
        break;
        
      case 160+13:    /* RF6_TRAPS */
        if (!direct) break;
        message(m_name, 0x03);
        if (blind) message(" mumbles, and then cackles evilly.", 0);
        else message(" casts a spell and cackles evilly.", 0);
        (void)trap_creation();
        break;
        
      case 160+14:    /* RF6_FORGET */
        if (!direct) break;
        message(m_name, 0x03);
        message(" tries to blank your mind.", 0);

        if (player_saves() || rand_int(2)) {
            msg_print("You resist the spell.");
        }
        else if (lose_all_info()) {
            msg_print("Your memories fade away.");
        }
        break;
        
      case 160+15:    /* RF6_XXX6X6 */
        break;

      case 160+16:    /* RF6_XXX7X6 */
        break;
        
      case 160+17:    /* RF6_XXX8X6 */
        break;
        
      case 160+18:    /* RF6_S_MONSTER */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" magically summons help!", 0);
        for (k = 0; k < 1; k++) {
            count += summon_monster(y, x, dun_level + MON_SUMMON_ADJ);
        }
        if (blind && count) message("You hear something appear nearby.", 0);
        break;
        
      case 160+19:    /* RF6_S_MONSTERS */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" magically summons monsters!", 0);
        for (k = 0; k < 8; k++) {
            count += summon_monster(y, x, dun_level + MON_SUMMON_ADJ);
        }
        if (blind && count) message("You here many things appear nearby.", 0);
        break;
        
      case 160+20:    /* RF6_S_ANT */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" magically summons ants.", 0);
        for (k = 0; k < 7; k++) {
            count += summon_specific(y, x, r_ptr->level, SUMMON_ANT);
        }
        if (blind && count) message("You hear many things appear nearby.", 0);
        break;
        
      case 160+21:    /* RF6_S_SPIDER */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" magically summons Spiders.", 0);
        for (k = 0; k < 6; k++) {
            count += summon_specific(y, x, r_ptr->level, SUMMON_SPIDER);
        }
        if (blind && count) message("You hear many things appear nearby.", 0);
        break;
        
      case 160+22:    /* RF6_S_HOUND */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" magically summons Hounds.", 0);
        for (k = 0; k < 8; k++) {
            count += summon_specific(y, x, r_ptr->level, SUMMON_HOUND);
        }
        if (blind && count) message("You hear many things appear nearby.", 0);
        break;
        
      case 160+23:    /* RF6_S_REPTILE */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" magically summons reptiles.", 0);
        for (k = 0; k < 8; k++) {
            count += summon_specific(y, x, r_ptr->level, SUMMON_REPTILE);
        }
        if (blind && count) message("You hear many things appear nearby.", 0);
        break;
        
      case 160+24:    /* RF6_S_ANGEL */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" summons an Angel.", 0);
        for (k = 0; k < 1; k++) {
            count += summon_specific(y, x, r_ptr->level, SUMMON_ANGEL);
        }
        if (blind && count) message("You hear something appear nearby.", 0);
        break;
        
      case 160+25:    /* RF6_S_DEMON */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" summons a hellish adversary!", 0);
        for (k = 0; k < 1; k++) {
            count += summon_specific(y, x, r_ptr->level, SUMMON_DEMON);
        }
        if (blind && count) {
            message("You smell fire and brimstone nearby.", 0);
        }
        break;
        
      case 160+26:    /* RF6_S_UNDEAD */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" magically summons help from beyond the grave!", 0);
        for (k = 0; k < 1; k++) {
            count += summon_specific(y, x, r_ptr->level, SUMMON_UNDEAD);
        }
        if (blind && count) {
            message("You hear something creepy appear nearby.", 0);
        }
        break;
        
      case 160+27:    /* RF6_S_DRAGON */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" magically summons a Dragon!", 0);
        for (k = 0; k < 1; k++) {
            count += summon_specific(y, x, r_ptr->level, SUMMON_DRAGON);
        }
        if (blind && count) {
            message("You hear something large appear nearby.", 0);
        }
        break;
        
      case 160+28:    /* RF6_S_HI_UNDEAD */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" summons the DEAD!", 0);
        for (k = 0; k < 8; k++) {
            count += summon_specific(y, x, r_ptr->level, SUMMON_HI_UNDEAD);
        }
        if (blind && count) message("A chill runs down your spine.", 0);
        break;
        
      case 160+29:    /* RF6_S_HI_DRAGON */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" summons ancient dragons.", 0);
        for (k = 0; k < 5; k++) {
            count += summon_specific(y, x, r_ptr->level, SUMMON_HI_DRAGON);
        }
        if (blind && count) {
            message("You hear many huge things appear nearby.", 0);
        }
        break;
        
      case 160+30:    /* RF6_S_WRAITH */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" magically summons mighty undead opponents.", 0);
        for (k = 0; k < 7; k++) {
            count += summon_specific(y, x, r_ptr->level, SUMMON_WRAITH);
        }
        for (k = 0; k < 7; k++) {
            count += summon_specific(y, x, r_ptr->level, SUMMON_HI_UNDEAD);
        }
        if (blind && count) {
            message("You hear many creepy things appear nearby.", 0);
        }
        break;
        
      case 160+31:    /* RF6_S_UNIQUE */
        message(m_name, 0x03);
        if (blind) message(" mumbles.", 0);
        else message(" summons special opponents!", 0);
        for (k = 0; k < 4; k++) {
            count += summon_specific(y, x, r_ptr->level, SUMMON_UNIQUE);
        }
        for (k = 0; k < 4; k++) {
            count += summon_specific(y, x, r_ptr->level, SUMMON_HI_UNDEAD);
        }
        if (blind && count) {
            message("You are worried by what you hear nearby.", 0);
        }
        break;


      default:
        message("Oops. ", 0x02);
        message(m_name, 0x03);
        message(" casts a buggy spell.", 0);
    }


    /* Remember what the monster did to us */
    if (seen) {

        /* Inate spell */
        if (thrown_spell < 32*4) {
            l_ptr->flags4 |= 1L << (thrown_spell - 32*3);
            if (l_ptr->cast_inate < MAX_UCHAR) l_ptr->cast_inate++;
        }

        /* Bolt or Ball */
        else if (thrown_spell < 32*5) {
            l_ptr->flags5 |= 1L << (thrown_spell - 32*4);
            if (l_ptr->cast_spell < MAX_UCHAR) l_ptr->cast_spell++;
        }

        /* Special spell */
        else if (thrown_spell < 32*6) {
            l_ptr->flags6 |= 1L << (thrown_spell - 32*5);
            if (l_ptr->cast_spell < MAX_UCHAR) l_ptr->cast_spell++;
        }
    }


    /* Always take note of monsters that kill you */
    if (death && (l_ptr->deaths < MAX_SHORT)) l_ptr->deaths++;


    /* A spell was cast */
    return (TRUE);
}


