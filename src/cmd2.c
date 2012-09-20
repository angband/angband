/* File: cmd2.c */

/* Purpose: Movement commands (part 2) */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"



/*
 * Go up one level					-RAK-	
 */
void do_cmd_go_up(void)
{
    cave_type *c_ptr;

    /* Player grid */
    c_ptr = &cave[py][px];

    /* Verify stairs */
    if ((c_ptr->feat & 0x3F) != 0x06) {
        msg_print("I see no up staircase here.");
        return;
    }

    /* Hack -- take a turn */
    energy_use = 100;
    
    /* Success */
    msg_print("You enter a maze of up staircases.");

    /* Go up the stairs */
    dun_level--;
    new_level_flag = TRUE;

    /* Create a way back */
    create_down_stair = TRUE;
}


/*
 * Go down one level
 */
void do_cmd_go_down(void)
{
    cave_type *c_ptr;

    /* Player grid */
    c_ptr = &cave[py][px];

    /* Verify stairs */
    if ((c_ptr->feat & 0x3F) != 0x07) {
        msg_print("I see no down staircase here.");
        return;
    }

    /* Hack -- take a turn */
    energy_use = 100;
    
    /* Success */
    msg_print("You enter a maze of down staircases.");

    /* Go down */
    dun_level++;
    new_level_flag = TRUE;

    /* Create a way back */
    create_up_stair = TRUE;
}



/*
 * Simple command to "search" for one turn
 */
void do_cmd_search(void)
{
    /* Allow repeated command */
    if (command_arg) {

        /* Set repeat count */
        command_rep = command_arg - 1;

        /* Redraw the state */
        p_ptr->redraw |= (PR_STATE);

        /* Cancel the arg */
        command_arg = 0;
    }

    /* Take a turn */
    energy_use = 100;
    
    /* Search */
    search();
}


/*
 * Hack -- toggle search mode
 */
void do_cmd_toggle_search(void)
{
    /* Stop searching */
    if (p_ptr->searching) {

        /* Clear the searching flag */
        p_ptr->searching = FALSE;

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);

        /* Redraw the state */
        p_ptr->redraw |= (PR_STATE);
    }

    /* Start searching */
    else {

        /* Set the searching flag */
        p_ptr->searching = TRUE;

        /* Update stuff */
        p_ptr->update |= (PU_BONUS);

        /* Redraw stuff */
        p_ptr->redraw |= (PR_STATE | PR_SPEED);
    }
}



/*
 * Allocates objects upon opening a chest    -BEN-
 * Disperse treasures from the chest "i_ptr", centered at (x,y).
 */
static void chest_death(int y, int x, inven_type *i_ptr)
{
    int		i, d, ny, nx;
    int		number, small;


    /* Must be a chest */
    if (i_ptr->tval != TV_CHEST) return;

    /* Determine if the chest is small */
    small = (i_ptr->sval < SV_CHEST_MIN_LARGE);

    /* Determine how many items to drop */
    number = (i_ptr->sval % SV_CHEST_MIN_LARGE);

    /* Generate some treasure */
    if (i_ptr->pval && (number > 0)) {

        /* Drop some objects (non-chests) */
        for ( ; number > 0; --number) {

            /* Try 20 times per item */
            for (i = 0; i < 20; ++i) {

                /* Pick a distance */
                d = ((i + 15) / 15);

                /* Pick a location */
                scatter(&ny, &nx, y, x, d, 0);

                /* Must be a clean floor grid */
                if (!clean_grid_bold(ny, nx)) continue;

                /* Opening a chest */
                opening_chest = TRUE;

                /* The "pval" of a chest is how "good" it is */
                object_level = ABS(i_ptr->pval);

                /* Small chests often drop gold */
                if (small && (rand_int(100) < 75)) {
                    place_gold(ny, nx);
                }

                /* Otherwise drop an item */
                else {
                    place_object(ny, nx, FALSE, FALSE);
                }

                /* Reset the object level */
                object_level = dun_level;

                /* No longer opening a chest */
                opening_chest = FALSE;

                /* Notice it */
                note_spot(ny, nx);
                
                /* Display it */
                lite_spot(ny, nx);

                /* Under the player */
                if ((ny == py) && (nx == px)) {
                    msg_print("You feel something roll beneath your feet.");
                }

                /* Successful placement */
                break;
            }
        }
    }

    /* Empty */
    i_ptr->pval = 0;

    /* Known */
    inven_known(i_ptr);
}


/*
 * Chests have traps too.
 *
 * Exploding chest destroys contents (and traps).
 * Note that the chest itself is never destroyed.
 */
static void chest_trap(int y, int x, inven_type *i_ptr)
{
    int  i, trap;


    /* Only analyze chests */
    if (i_ptr->tval != TV_CHEST) return;

    /* Ignore disarmed chests */
    if (i_ptr->pval <= 0) return;

    /* Obtain the traps */
    trap = chest_traps[i_ptr->pval];

    /* Lose strength */
    if (trap & CHEST_LOSE_STR) {
        msg_print("A small needle has pricked you!");
        take_hit(damroll(1, 4), "a poison needle");
        (void)do_dec_stat(A_STR);
    }

    /* Lose constitution */
    if (trap & CHEST_LOSE_CON) {
        msg_print("A small needle has pricked you!");
        take_hit(damroll(1, 4), "a poison needle");
        (void)do_dec_stat(A_CON);
    }

    /* Poison */
    if (trap & CHEST_POISON) {
        msg_print("A puff of green gas surrounds you!");
        if (!(p_ptr->resist_pois || p_ptr->oppose_pois)) {
            (void)set_poisoned(p_ptr->poisoned + 10 + randint(20));
        }
    }

    /* Paralyze */
    if (trap & CHEST_PARALYZE) {
        msg_print("A puff of yellow gas surrounds you!");
        if (!p_ptr->free_act) {
            (void)set_paralyzed(p_ptr->paralyzed + 10 + randint(20));
        }
    }

    /* Summon monsters */
    if (trap & CHEST_SUMMON) {
        int num = 2 + randint(3);
        msg_print("You are enveloped in a cloud of smoke!");
        for (i = 0; i < num; i++) {
            (void)summon_specific(y, x, dun_level, 0);
        }
    }

    /* Explode */
    if (trap & CHEST_EXPLODE) {
        msg_print("There is a sudden explosion!");
        msg_print("Everything inside the chest is destroyed!");
        i_ptr->pval = 0;
        take_hit(damroll(5, 8), "an exploding chest");
    }
}





/*
 * Open a closed door or closed chest.
 *
 * Note unlocking a locked door/chest is worth one experience point.
 */
void do_cmd_open()
{
    int				y, x, i, j, dir;
    int				flag;

    cave_type		*c_ptr;
    inven_type		*i_ptr;

    bool more = FALSE;


    /* Allow repeated command */
    if (command_arg) {

        /* Set repeat count */
        command_rep = command_arg - 1;

        /* Redraw the state */
        p_ptr->redraw |= (PR_STATE);

        /* Cancel the arg */
        command_arg = 0;
    }

    /* Get a "repeated" direction */
    if (get_rep_dir(&dir)) {

        /* Get requested location */
        y = py + ddy[dir];
        x = px + ddx[dir];

        /* Get requested grid */
        c_ptr = &cave[y][x];

        /* Get the object (if any) */
        i_ptr = &i_list[c_ptr->i_idx];

        /* Nothing useful */
        if ((((c_ptr->feat & 0x3F) < 0x20) ||
             ((c_ptr->feat & 0x3F) > 0x2F)) &&
            (i_ptr->tval != TV_CHEST)) {

            /* Message */
            msg_print("You see nothing there to open.");
        }

        /* Monster in the way */
        else if (c_ptr->m_idx) {

            /* Take a turn */
            energy_use = 100;
            
            /* Message */
            msg_print("There is a monster in the way!");

            /* Attack */
            py_attack(y, x);
        }

        /* Open a closed chest. */
        else if (i_ptr->tval == TV_CHEST) {

            /* Take a turn */
            energy_use = 100;
            
            /* Assume opened successfully */
            flag = TRUE;

            /* Attempt to unlock it */
            if (i_ptr->pval > 0) {

                /* Assume locked, and thus not open */
                flag = FALSE;

                /* Get the "disarm" factor */
                i = p_ptr->skill_dis;

                /* Penalize some conditions */
                if (p_ptr->blind || no_lite()) i = i / 10;
                if (p_ptr->confused || p_ptr->image) i = i / 10;

                /* Extract the difficulty */
                j = i - i_ptr->pval;

                /* Always have a small chance of success */
                if (j < 2) j = 2;

                /* Success -- May still have traps */
                if (rand_int(100) < j) {
                    msg_print("You have picked the lock.");
                    gain_exp(1);
                    flag = TRUE;
                }

                /* Failure -- Keep trying */
                else {
                    /* We may continue repeating */
                    more = TRUE;
                    if (flush_failure) flush();
                    msg_print("You failed to pick the lock.");
                }
            }

            /* Allowed to open */
            if (flag) {

                /* Apply chest traps, if any */
                chest_trap(y, x, i_ptr);

                /* Let the Chest drop items */
                chest_death(y, x, i_ptr);
            }
        }

        /* Jammed door */
        else if ((c_ptr->feat & 0x3F) >= 0x28) {

            /* Take a turn */
            energy_use = 100;
            
            /* Stuck */
            msg_print("The door appears to be stuck.");
        }

        /* Locked door */
        else if ((c_ptr->feat & 0x3F) >= 0x21) {

            /* Take a turn */
            energy_use = 100;
            
            /* Disarm factor */
            i = p_ptr->skill_dis;

            /* Penalize some conditions */
            if (p_ptr->blind || no_lite()) i = i / 10;
            if (p_ptr->confused || p_ptr->image) i = i / 10;

            /* Extract the difficulty */
            j = i - ((int)(c_ptr->feat & 0x07)) * 4;

            /* Always have a small chance of success */
            if (j < 2) j = 2;

            /* Success */
            if (rand_int(100) < j) {

                /* Message */
                msg_print("You have picked the lock.");

                /* Experience */
                gain_exp(1);

                /* Open the door */
                c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x04);
            
                /* Notice */
                note_spot(y, x);

                /* Redraw */
                lite_spot(y, x);

                /* Update some things */
                p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
            }

            /* Failure */
            else {

                /* Failure */
                if (flush_failure) flush();

                /* Message */
                msg_print("You failed to pick the lock.");

                /* We may keep trying */
                more = TRUE;
            }
        }

        /* Closed door */
        else {

            /* Open the door */
            c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x04);
            
            /* Notice */
            note_spot(y, x);

            /* Redraw */
            lite_spot(y, x);

            /* Update some things */
            p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
        }
    }

    /* Cancel repeat unless we may continue */
    if (!more) disturb(0, 0);
}


/*
 * Close an open door.
 */
void do_cmd_close()
{
    int			y, x, dir;
    cave_type		*c_ptr;

    bool more = FALSE;


    /* Allow repeated command */
    if (command_arg) {

        /* Set repeat count */
        command_rep = command_arg - 1;

        /* Redraw the state */
        p_ptr->redraw |= (PR_STATE);

        /* Cancel the arg */
        command_arg = 0;
    }

    /* Get a "repeated" direction */
    if (get_rep_dir(&dir)) {

        /* Get requested location */
        y = py + ddy[dir];
        x = px + ddx[dir];

        /* Get grid and contents */
        c_ptr = &cave[y][x];

        /* Broken door */
        if ((c_ptr->feat & 0x3F) == 0x05) {
        
            /* Message */            
            msg_print("The door appears to be broken.");
        }
        
        /* Require open door */
        else if ((c_ptr->feat & 0x3F) != 0x04) {

            /* Message */
            msg_print("You see nothing there to close.");
        }

        /* Monster in the way */
        else if (c_ptr->m_idx) {

            /* Take a turn */
            energy_use = 100;
            
            /* Message */
            msg_print("There is a monster in the way!");

            /* Attack */
            py_attack(y, x);
        }

        /* Close the door */
        else {

            /* Take a turn */
            energy_use = 100;

            /* Close the door */
            c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x20);

            /* Notice */
            note_spot(y, x);

            /* Redraw */
            lite_spot(y, x);

            /* Update some things */
            p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
        }
    }

    /* Cancel repeat unless we may continue */
    if (!more) disturb(0, 0);
}



/*
 * Tunnel through wall.  Assumes valid location.
 *
 * Note that it is impossible to "extend" rooms past their
 * outer walls (which are actually part of the room).
 *
 * This will, however, produce grids which are NOT illuminated
 * (or darkened) along with the rest of the room.
 */
static bool twall(int y, int x)
{
    cave_type	*c_ptr = &cave[y][x];

    /* Paranoia -- Require a wall or door or some such */
    if ((c_ptr->feat & 0x3F) < 0x20) return (FALSE);

    /* Remove the feature */
    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x01);

    /* Forget the "field mark" */
    c_ptr->feat &= ~CAVE_MARK;

    /* Notice */
    note_spot(y, x);

    /* Redisplay the grid */
    lite_spot(y, x);

    /* Update some things */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);

    /* Result */
    return (TRUE);
}



/*
 * Tunnels through "walls" (including rubble and closed doors)
 *
 * Note that tunneling almost always takes time, since otherwise
 * you can use tunnelling to find monsters.  Also note that you
 * must tunnel in order to hit invisible monsters in walls (etc).
 *
 * Digging is very difficult without a "digger" weapon, but can be
 * accomplished by strong players using heavy weapons.
 */
void do_cmd_tunnel()
{
    int			y, x, dir;

    cave_type		*c_ptr;

    bool old_floor = FALSE;

    bool more = FALSE;


    /* Allow repeated command */
    if (command_arg) {

        /* Set repeat count */
        command_rep = command_arg - 1;

        /* Redraw the state */
        p_ptr->redraw |= (PR_STATE);

        /* Cancel the arg */
        command_arg = 0;
    }

    /* Get a direction to tunnel, or Abort */
    if (get_rep_dir(&dir)) {

        /* Get location */
        y = py + ddy[dir];
        x = px + ddx[dir];

        /* Get grid */
        c_ptr = &cave[y][x];

        /* Check the floor-hood */
        old_floor = floor_grid_bold(y, x);

        /* No tunnelling through emptiness */
        if ((c_ptr->feat & 0x3F) < 0x20) {

            /* Message */
            msg_print("You see nothing there to tunnel through.");
        }

        /* No tunnelling through doors */
        else if ((c_ptr->feat & 0x3F) < 0x30) {

            /* Message */
            msg_print("You cannot tunnel through doors.");
        }

        /* A monster is in the way */
        else if (c_ptr->m_idx) {

            /* Take a turn */
            energy_use = 100;
            
            /* Message */
            msg_print("There is a monster in the way!");

            /* Attack */
            py_attack(y, x);
        }

        /* Okay, try digging */
        else {

            /* Take a turn */
            energy_use = 100;
            
            /* Titanium */
            if ((c_ptr->feat & 0x3F) >= 0x3C) {

                msg_print("This seems to be permanent rock.");
            }

            /* Granite */
            else if ((c_ptr->feat & 0x3F) >= 0x38) {

                /* Tunnel */
                if ((p_ptr->skill_dig > 40 + rand_int(1600)) && twall(y, x)) {
                    msg_print("You have finished the tunnel.");
                }

                /* Keep trying */
                else {
                    /* We may continue tunelling */
                    msg_print("You tunnel into the granite wall.");
                    more = TRUE;
                }
            }

            /* Quartz / Magma */
            else if ((c_ptr->feat & 0x3F) >= 0x32) {

                bool okay = FALSE;
                bool gold = FALSE;
                bool hard = FALSE;
                
                /* Found gold */
                if ((c_ptr->feat & 0x3F) >= 0x34) gold = TRUE;
                
                /* Quartz */
                if (c_ptr->feat & 0x01) hard = TRUE;

                /* Quartz */
                if (hard) {

                    okay = (p_ptr->skill_dig > 20 + rand_int(800));
                }
                
                /* Magma */
                else {

                    okay = (p_ptr->skill_dig > 10 + rand_int(400));
                }

                /* Success */
                if (okay && twall(y,x)) {

                    /* Found treasure */
                    if (gold) {

                        /* Place some gold */
                        place_gold(y, x);

                        /* Notice it */
                        note_spot(y, x);
                        
                        /* Display it */
                        lite_spot(y, x);

                        /* Message */
                        msg_print("You have found something!");
                    }

                    /* Found nothing */
                    else {

                        /* Message */
                        msg_print("You have finished the tunnel.");
                    }
                }

                /* Failure (quartz) */
                else if (hard) {

                    /* Message, continue digging */
                    msg_print("You tunnel into the quartz vein.");
                    more = TRUE;
                }

                /* Failure (magma) */
                else {

                    /* Message, continue digging */
                    msg_print("You tunnel into the magma vein.");
                    more = TRUE;
                }
            }

            /* Rubble */
            else if ((c_ptr->feat & 0x3F) == 0x31) {

                /* Remove the rubble */
                if ((p_ptr->skill_dig > rand_int(200)) && twall(y,x)) {

                    /* Message */
                    msg_print("You have removed the rubble.");

                    /* Hack -- place an object */
                    if (rand_int(100) < 10) {
                        place_object(y, x, FALSE, FALSE);
                        if (player_can_see_bold(y, x)) {
                             msg_print("You have found something!");
                        }
                    }

                    /* Notice */
                    note_spot(y, x);
                    
                    /* Display */
                    lite_spot(y, x);
                }

                else {

                    /* Message, keep digging */
                    msg_print("You dig in the rubble.");
                    more = TRUE;
                }
            }

            /* Default to secret doors */
            else {

                /* Message, keep digging */
                msg_print("You tunnel into the granite wall.");
                more = TRUE;

                /* Hack -- Search */
                search();
            }
        }

        /* Notice "blockage" changes */
        if (old_floor != floor_grid_bold(y, x)) {

            /* Update some things */
            p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
        }
    }

    /* Cancel repetition unless we can continue */
    if (!more) disturb(0, 0);
}


/*
 * Disarms a trap, or chest	-RAK-	
 */
void do_cmd_disarm()
{
    int                 y, x, i, j, dir, power;

    cave_type		*c_ptr;
    inven_type		*i_ptr;

    bool		more = FALSE;


    /* Allow repeated command */
    if (command_arg) {

        /* Set repeat count */
        command_rep = command_arg - 1;

        /* Redraw the state */
        p_ptr->redraw |= (PR_STATE);

        /* Cancel the arg */
        command_arg = 0;
    }

    /* Get a direction (or abort) */
    if (get_rep_dir(&dir)) {

        /* Get location */
        y = py + ddy[dir];
        x = px + ddx[dir];

        /* Get grid and contents */
        c_ptr = &cave[y][x];

        /* Access the item */
        i_ptr = &i_list[c_ptr->i_idx];

        /* Nothing useful */
        if ((i_ptr->tval != TV_CHEST) &&
            (((c_ptr->feat & 0x3F) < 0x10) ||
             ((c_ptr->feat & 0x3F) > 0x1F))) {

            /* Message */
            msg_print("You see nothing there to disarm.");
        }

        /* Monster in the way */
        else if (c_ptr->m_idx) {

            /* Take a turn */
            energy_use = 100;
            
            /* Message */
            msg_print("There is a monster in the way!");

            /* Attack */
            py_attack(y, x);
        }

        /* Normal disarm */
        else if (i_ptr->tval == TV_CHEST) {

            /* Take a turn */
            energy_use = 100;
            
            /* Get the "disarm" factor */
            i = p_ptr->skill_dis;

            /* Penalize some conditions */
            if (p_ptr->blind || no_lite()) i = i / 10;
            if (p_ptr->confused || p_ptr->image) i = i / 10;

            /* Extract the difficulty */
            j = i - i_ptr->pval;

            /* Always have a small chance of success */
            if (j < 2) j = 2;

            /* Must find the trap first. */
            if (!inven_known_p(i_ptr)) {
                msg_print("I don't see any traps.");
            }

            /* Already disarmed/unlocked */
            else if (i_ptr->pval <= 0) {
                msg_print("The chest is not trapped.");
            }

            /* No traps to find. */
            else if (!chest_traps[i_ptr->pval]) {
                msg_print("The chest is not trapped.");
            }

            /* Success (get a lot of experience) */
            else if (rand_int(100) < j) {
                msg_print("You have disarmed the chest.");
                gain_exp(i_ptr->pval);
                i_ptr->pval = (0 - i_ptr->pval);
            }

            /* Failure -- Keep trying */
            else if ((i > 5) && (randint(i) > 5)) {
                /* We may keep trying */
                more = TRUE;
                if (flush_failure) flush();
                msg_print("You failed to disarm the chest.");
            }

            /* Failure -- Set off the trap */
            else {
                msg_print("You set off a trap!");
                chest_trap(y, x, i_ptr);
            }
        }

        /* Disarm a trap */
        else {

            cptr name = (f_name + f_info[c_ptr->feat & 0x3F].name);
            
            /* Take a turn */
            energy_use = 100;
            
            /* Get the "disarm" factor */
            i = p_ptr->skill_dis;

            /* Penalize some conditions */
            if (p_ptr->blind || no_lite()) i = i / 10;
            if (p_ptr->confused || p_ptr->image) i = i / 10;

            /* XXX XXX XXX Variable power? */

            /* Extract trap "power" */
            power = 5;
            
            /* Extract the difficulty */
            j = i - power;

            /* Always have a small chance of success */
            if (j < 2) j = 2;

            /* Success */
            if (rand_int(100) < j) {
            
                /* Message */
                msg_format("You have disarmed the %s.", name);

                /* Reward */
                gain_exp(power);

                /* Remove the trap */
                c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x01);
                
                /* Forget the "field mark" */
                c_ptr->feat &= ~CAVE_MARK;

                /* Notice */
                note_spot(y, x);

                /* Redisplay the grid */
                lite_spot(y, x);
                
                /* move the player onto the trap grid */
                move_player(dir, FALSE);
            }

            /* Failure -- Keep trying */
            else if ((i > 5) && (randint(i) > 5)) {

                /* Failure */
                if (flush_failure) flush();

                /* Message */
                msg_format("You failed to disarm the %s.", name);

                /* We may keep trying */
                more = TRUE;
            }

            /* Failure -- Set off the trap */
            else {

                /* Message */
                msg_format("You set off the %s!", name);

                /* Move the player onto the trap */
                move_player(dir, FALSE);
            }
        }
    }

    /* Cancel repeat unless told not to */
    if (!more) disturb(0, 0);
}


/*
 * Bash open a door, success based on character strength
 *
 * For a closed door, pval is positive if locked; negative if stuck.
 *
 * For an open door, pval is positive for a broken door.
 *
 * A closed door can be opened - harder if locked. Any door might be
 * bashed open (and thereby broken). Bashing a door is (potentially)
 * faster! You move into the door way. To open a stuck door, it must
 * be bashed. A closed door can be jammed (see do_cmd_spike()).
 *
 * Creatures can also open or bash doors, see elsewhere.
 *
 * We need to use character body weight for something, or else we need
 * to no longer give female characters extra starting gold.
 */
void do_cmd_bash()
{
    int                 y, x, dir;
    
    int			bash, temp;

    cave_type		*c_ptr;

    bool		more = FALSE;


    /* Allow repeated command */
    if (command_arg) {

        /* Set repeat count */
        command_rep = command_arg - 1;

        /* Redraw the state */
        p_ptr->redraw |= (PR_STATE);

        /* Cancel the arg */
        command_arg = 0;
    }

    /* Get a "repeated" direction */
    if (get_rep_dir(&dir)) {

        /* Bash location */
        y = py + ddy[dir];
        x = px + ddx[dir];

        /* Get grid */
        c_ptr = &cave[y][x];

        /* Nothing useful */
        if (((c_ptr->feat & 0x3F) < 0x20) ||
            ((c_ptr->feat & 0x3F) > 0x2F)) {

            /* Message */
            msg_print("You see nothing there to bash.");
        }

        /* Monster in the way */
        else if (c_ptr->m_idx) {

            /* Take a turn */
            energy_use = 100;
            
            /* Message */
            msg_print("There is a monster in the way!");

            /* Attack */
            py_attack(y, x);
        }

        /* Bash a closed door */
        else {

            /* Take a turn */
            energy_use = 100;
            
            /* Message */
            msg_print("You smash into the door!");

            /* Hack -- Bash power based on strength */
            /* (Ranges from 3 to 20 to 100 to 200) */
            bash = adj_str_blow[p_ptr->stat_ind[A_STR]];

            /* Compare the bash power to the door power */
            temp = (bash - ((int)(c_ptr->feat & 0x07)) * 10);

            /* Hack -- always have a chance */
            if (temp < 1) temp = 1;
            
            /* Hack -- attempt to bash down the door */
            if (rand_int(100) < temp) {

                /* Message */
                msg_print("The door crashes open!");

                /* Break down the door */
                if (rand_int(100) < 50) {
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x05);
                }
                
                /* Open the door */
                else {
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x04);
                }

                /* Notice */
                note_spot(y, x);

                /* Redraw */
                lite_spot(y, x);

                /* Hack -- Fall through the door */
                move_player(dir, FALSE);

                /* Update some things */
                p_ptr->update |= (PU_VIEW | PU_LITE);
                p_ptr->update |= (PU_DISTANCE);
            }

            /* Saving throw against stun */
            else if (rand_int(100) < adj_dex_safe[p_ptr->stat_ind[A_DEX]] +
                                     p_ptr->lev) {

                /* Message */
                msg_print("The door holds firm.");

                /* Allow repeated bashing */
                more = TRUE;
            }

            /* High dexterity yields coolness */
            else {

                /* Message */
                msg_print("You are off-balance.");

                /* Hack -- Lose balance ala paralysis */
                (void)set_paralyzed(p_ptr->paralyzed + 2 + rand_int(2));
            }
        }
    }

    /* Unless valid action taken, cancel bash */
    if (!more) disturb(0, 0);
}



/*
 * Find the index of some "spikes", if possible.
 *
 * XXX XXX XXX Let user choose a pile of spikes, perhaps?
 */
static bool get_spike(int *ip)
{
    int i;

    /* Check every item in the pack */
    for (i = 0; i < INVEN_PACK; i++) {

        inven_type *i_ptr = &inventory[i];

        /* Check the "tval" code */
        if (i_ptr->tval == TV_SPIKE) {

            /* Save the spike index */
            (*ip) = i;

            /* Success */
            return (TRUE);
        }
    }

    /* Oops */
    return (FALSE);
}



/*
 * Jam a closed door with a spike
 *
 * This command may NOT be repeated
 */
void do_cmd_spike()
{
    int                  y, x, dir, item;

    cave_type		*c_ptr;


    /* Get a "repeated" direction */
    if (get_rep_dir(&dir)) {

        /* Get location */
        y = py + ddy[dir];
        x = px + ddx[dir];

        /* Get grid and contents */
        c_ptr = &cave[y][x];

        /* Require closed door */
        if (((c_ptr->feat & 0x3F) < 0x20) ||
            ((c_ptr->feat & 0x3F) > 0x2F)) {

            /* Message */
            msg_print("You see nothing there to spike.");
        }

        /* Get a spike */
        else if (!get_spike(&item)) {

            /* Message */
            msg_print("You have no spikes!");
        }

        /* Is a monster in the way? */
        else if (c_ptr->m_idx) {

            /* Take a turn */
            energy_use = 100;

            /* Message */
            msg_print("There is a monster in the way!");

            /* Attack */
            py_attack(y, x);
        }

        /* Go for it */
        else {

            /* Take a turn */
            energy_use = 100;

            /* Successful jamming */
            msg_print("You jam the door with a spike.");

            /* Convert "locked" to "stuck" */
            if ((c_ptr->feat & 0x3F) < 0x28) c_ptr->feat |= 0x08;
            
            /* Add one spike to the door */
            if ((c_ptr->feat & 0x07) < 0x07) c_ptr->feat++;
            
            /* Use up, and describe, a single spike, from the bottom */
            inven_item_increase(item, -1);
            inven_item_describe(item);
            inven_item_optimize(item);
        }
    }
}



/*
 * Support code for the "Walk" and "Jump" commands
 */
void do_cmd_walk(int pickup)
{
    int dir;

    bool more = FALSE;


    /* Allow repeated command */
    if (command_arg) {

        /* Set repeat count */
        command_rep = command_arg - 1;

        /* Redraw the state */
        p_ptr->redraw |= (PR_STATE);

        /* Cancel the arg */
        command_arg = 0;
    }

    /* Get a "repeated" direction */
    if (get_rep_dir(&dir)) {

        /* Take a turn */
        energy_use = 100;

        /* Actually move the character */
        move_player(dir, pickup);

        /* Allow more walking */
        more = TRUE;
    }

    /* Cancel repeat unless we may continue */
    if (!more) disturb(0, 0);
}



/*
 * Start running.
 */
void do_cmd_run(void)
{
    int dir;

    /* Get a "repeated" direction */
    if (get_rep_dir(&dir)) {

        /* Hack -- Set the run counter */
        running = (command_arg ? command_arg : 1000);

        /* First step */
        run_step(dir);
    }
}



/*
 * Stay still.  Search.  Enter stores.
 * Pick up treasure if "pickup" is true.
 */
void do_cmd_stay(int pickup)
{
    cave_type *c_ptr = &cave[py][px];


    /* Allow repeated command */
    if (command_arg) {

        /* Set repeat count */
        command_rep = command_arg - 1;

        /* Redraw the state */
        p_ptr->redraw |= (PR_STATE);

        /* Cancel the arg */
        command_arg = 0;
    }


    /* Take a turn */
    energy_use = 100;
    

    /* Spontaneous Searching */
    if ((p_ptr->skill_fos >= 50) || (0 == rand_int(50 - p_ptr->skill_fos))) {
        search();
    }

    /* Continuous Searching */
    if (p_ptr->searching) {
        search();
    }


    /* Hack -- enter a store if we are on one */
    if (((c_ptr->feat & 0x3F) >= 0x08) &&
        ((c_ptr->feat & 0x3F) <= 0x0F)) {

        /* Disturb */
        disturb(0, 0);

        /* Hack -- enter store */
        command_new = '_';
    }


    /* Try to Pick up anything under us */
    carry(pickup);
}






/*
 * Resting allows a player to safely restore his hp	-RAK-	
 */
void do_cmd_rest(void)
{
    char out_val[80];


    /* Prompt for time if needed */
    if (command_arg <= 0) {

        /* Ask the question (perhaps a "prompt" routine would be good) */
        prt("Rest for how long? ('*' for HP/SP, '&' as needed): ", 0, 0);

        /* Default to "until done" */
        strcpy(out_val, "&");
        
        /* Ask for how long */
        if (askfor_aux(out_val, 4)) {

            if (out_val[0] == '*') {
                command_arg = (-1);
            }
            else if (out_val[0] == '&') {
                command_arg = (-2);
            }
            else {
                command_arg = atoi(out_val);
                if (command_arg < 0) command_arg = 0;
            }
        }

        /* Nuke prompt */
        prt("", 0, 0);

        /* Handle "cancel" */
        if (!command_arg) return;
    }

    /* Hack -- maximum rest time */
    if (command_arg > 9999) command_arg = 9999;


    /* Hack -- take a turn */
    energy_use = 100;

    /* Save the rest code */
    resting = command_arg;

    /* Cancel searching */
    p_ptr->searching = FALSE;

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Redraw the state */
    p_ptr->redraw |= (PR_STATE);

    /* Handle stuff */
    handle_stuff();

    /* Refresh */
    Term_fresh();
}






/*
 * Determines the odds of an object breaking when thrown
 * Note that "impact" is true if the object hit a monster
 * Artifacts never break, see the "drop_near()" function.
 * Assume the object has NOT hit a wall or monster
 * Hitting a monster doubles the breakage chance
 * Hitting a wall less than 3 grids away does too.
 */
static int breakage_chance(inven_type *i_ptr)
{
    /* Examine the item type */
    switch (i_ptr->tval) {

      /* Burning flasks */
      case TV_FLASK:
        return (100);

      /* Very breakable objects */
      case TV_POTION:
      case TV_BOTTLE:
      case TV_FOOD:
      case TV_JUNK:
        return (50);

      /* Somewhat breakable objects */
      case TV_LITE:
      case TV_SCROLL:
      case TV_ARROW:
      case TV_SKELETON:
        return (30);

      /* Slightly breakable objects */
      case TV_WAND:
      case TV_SHOT:
      case TV_BOLT:
      case TV_SPIKE:
        return (20);
    }

    /* Normal objects */
    return (10);
}


/*
 * Fire an object from the pack or floor.
 *
 * You may only fire items that "match" your missile launcher.
 *
 * You must use slings + pebbles/shots, bows + arrows, xbows + bolts.
 *
 * See "calc_bonuses()" for more calculations and such.
 *
 * Note that "firing" a missile is MUCH better than "throwing" it.
 *
 * Note: "unseen" monsters are very hard to hit.
 *
 * Objects are more likely to break if they "attempt" to hit a monster.
 *
 * Rangers (with Bows) and Anyone (with "Extra Shots") get extra shots.
 *
 * The "extra shot" code works by decreasing the amount of energy
 * required to make each shot, spreading the shots out over time.
 *
 * Note that when firing missiles, the launcher multiplier is applied
 * after all the bonuses are added in, making multipliers very useful.
 *
 * Note that Bows of "Extra Might" get extra range and an extra bonus
 * for the damage multiplier.
 *
 * Note that Bows of "Extra Shots" give an extra shot.
 */
void do_cmd_fire(void)
{
    int			dir, item;
    int			j, y, x, ny, nx, ty, tx;
    int			tdam, tdis, thits, tmul;
    int			bonus, chance;
    int			cur_dis, visible;

    inven_type          throw_obj;
    inven_type		*i_ptr;

    inven_type		*j_ptr;

    bool		hit_body = FALSE;

    int			missile_attr;
    int			missile_char;

    char		i_name[80];


    /* Get the "bow" (if any) */
    j_ptr = &inventory[INVEN_BOW];

    /* Require a launcher */
    if (!j_ptr->tval) {
        msg_print("You have nothing to fire with.");
        return;
    }


    /* Require proper missile */
    item_tester_tval = p_ptr->tval_ammo;

    /* Get an item (from inven or floor) */
    if (!get_item(&item, "Fire which item? ", FALSE, TRUE, TRUE)) {
        if (item == -2) msg_print("You have nothing to fire.");
        return;
    }

    /* Access the item (if in the pack) */
    if (item >= 0) {
        i_ptr = &inventory[item];
    }
    else {
        i_ptr = &i_list[0 - item];
    }


    /* Get a direction (or cancel) */
    if (!get_aim_dir(&dir)) return;


    /* Create a "local missile object" */
    throw_obj = *i_ptr;
    throw_obj.number = 1;

    /* Reduce and describe inventory */
    if (item >= 0) {
        inven_item_increase(item, -1);
        inven_item_describe(item);
        inven_item_optimize(item);
    }

    /* Reduce and describe floor item */
    else {
        floor_item_increase(0 - item, -1);
        floor_item_optimize(0 - item);
    }

    /* Use the missile object */
    i_ptr = &throw_obj;

    /* Describe the object */
    objdes(i_name, i_ptr, FALSE, 3);

    /* Find the color and symbol for the object for throwing */
    missile_attr = inven_attr(i_ptr);
    missile_char = inven_char(i_ptr);


    /* Use the proper number of shots */
    thits = p_ptr->num_fire;

    /* Use a base distance */
    tdis = 10;

    /* Base damage from thrown object plus launcher bonus */
    tdam = damroll(i_ptr->dd, i_ptr->ds) + i_ptr->to_d + j_ptr->to_d;

    /* Actually "fire" the object */
    bonus = (p_ptr->to_h + i_ptr->to_h + j_ptr->to_h);
    chance = (p_ptr->skill_thb + (bonus * BTH_PLUS_ADJ));

    /* Assume a base multiplier */
    tmul = 1;

    /* Analyze the launcher */
    switch (j_ptr->sval) {

        /* Sling and ammo */
        case SV_SLING:
            tmul = 2;
            break;

        /* Short Bow and Arrow */
        case SV_SHORT_BOW:
            tmul = 2;
            break;

        /* Long Bow and Arrow */
        case SV_LONG_BOW:
            tmul = 3;
            break;

        /* Light Crossbow and Bolt */
        case SV_LIGHT_XBOW:
            tmul = 3;
            break;

        /* Heavy Crossbow and Bolt */
        case SV_HEAVY_XBOW:
            tmul = 4;
            break;
    }

    /* Get extra "power" from "extra might" */
    if (p_ptr->xtra_might) tmul++;
    
    /* Boost the damage */
    tdam *= tmul;

    /* Base range */
    tdis = 10 + 5 * tmul;


    /* Take a (partial) turn */
    energy_use = (100 / thits);


    /* Start at the player */
    y = py;
    x = px;

    /* Predict the "target" location */
    tx = px + 99 * ddx[dir];
    ty = py + 99 * ddy[dir];

    /* Check for "target request" */
    if ((dir == 5) && target_okay()) {
        tx = target_col;
        ty = target_row;
    }


    /* Hack -- Handle stuff */
    handle_stuff();


    /* Travel until stopped */
    for (cur_dis = 0; cur_dis < tdis; ) {

        /* Hack -- Stop at the target */
        if ((y == ty) && (x == tx)) break;

        /* Calculate the new location (see "project()") */
        ny = y;
        nx = x;
        mmove2(&ny, &nx, py, px, ty, tx);

        /* Stopped by walls/doors */
        if (!floor_grid_bold(ny,nx)) break;
	
        /* Advance the distance */
        cur_dis++;

        /* Save the new location */
        x = nx;
        y = ny;


        /* The player can see the (on screen) missile */
        if (panel_contains(y, x) && player_can_see_bold(y, x)) {

            /* Draw, Hilite, Fresh, Pause, Erase */
            print_rel(missile_char, missile_attr, y, x);
            move_cursor_relative(y, x);
            Term_fresh();
            delay(10 * delay_spd);
            lite_spot(y, x);
            Term_fresh();
        }

        /* The player cannot see the missile */
        else {

            /* Pause anyway, for consistancy */
            delay(10 * delay_spd);
        }


        /* Monster here, Try to hit it */
        if (cave[y][x].m_idx) {

            cave_type *c_ptr = &cave[y][x];
            
            monster_type *m_ptr = &m_list[c_ptr->m_idx];
            monster_race *r_ptr = &r_info[m_ptr->r_idx];

            /* Check the visibility */
            visible = m_ptr->ml;

            /* Did we hit it (penalize range) */
            if (test_hit_fire(chance - cur_dis, r_ptr->ac, m_ptr->ml)) {

                bool fear = FALSE;
                
                /* Assume a default death */
                cptr note_dies = " dies.";

                /* Some monsters get "destroyed" */
                if ((r_ptr->flags3 & RF3_DEMON) ||
                    (r_ptr->flags3 & RF3_UNDEAD) ||
                    (r_ptr->flags2 & RF2_STUPID) ||
                    (strchr("Evg", r_ptr->r_char))) {

                    /* Special note at death */
                    note_dies = " is destroyed.";
                }


                /* Note the collision */
                hit_body = TRUE;


                /* Handle unseen monster */
                if (!visible) {

                    /* Invisible monster */
                    msg_format("The %s finds a mark.", i_name);
                }

                /* Handle visible monster */
                else {

                    char m_name[80];

                    /* Get "the monster" or "it" */
                    monster_desc(m_name, m_ptr, 0);

                    /* Message */
                    msg_format("The %s hits %s.", i_name, m_name);

                    /* Hack -- Track this monster race */
                    if (m_ptr->ml) recent_track(m_ptr->r_idx);

                    /* Hack -- Track this monster */
                    if (m_ptr->ml) health_track(c_ptr->m_idx);
                }

                /* Apply special damage XXX XXX XXX */
                tdam = tot_dam_aux(i_ptr, tdam, m_ptr);
                tdam = critical_shot(i_ptr->weight, i_ptr->to_h, tdam);

                /* No negative damage */
                if (tdam < 0) tdam = 0;

                /* Complex message */
                if (wizard) {
                    msg_format("You do %d (out of %d) damage.",
                               tdam, m_ptr->hp);
                }

                /* Hit the monster, check for death */
                if (mon_take_hit(c_ptr->m_idx, tdam, &fear, note_dies)) {

                    /* Dead monster */
                }

                /* No death */
                else {

                    /* Message */
                    message_pain(c_ptr->m_idx, tdam);

                    /* Take note */
                    if (fear && m_ptr->ml) {

                        char m_name[80];

                        /* Sound */
                        sound(SOUND_FLEE);

                        /* Get the monster name (or "it") */
                        monster_desc(m_name, m_ptr, 0);

                        /* Message */
                        msg_format("%^s flees in terror!", m_name);
                    }
                }
            }

            /* Stop looking */
            break;
        }
    }

    /* Chance of breakage */
    j = breakage_chance(i_ptr);

    /* Double the chance if we hit a monster */
    if (hit_body) j = j * 2;

    /* Paranoia -- maximum breakage chance */
    if (j > 100) j = 100;

    /* Drop (or break) near that location */
    drop_near(i_ptr, j, y, x);
}



/*
 * Throw an object from the pack or floor.
 *
 * Note: "unseen" monsters are very hard to hit.
 *
 * Should throwing a weapon do full damage?  Should it allow the magic
 * to hit bonus of the weapon to have an effect?  Should it ever cause
 * the item to be destroyed?  Should it do any damage at all?
 *
 * Do we really need "verification" (note the "destroy" command)?
 */
void do_cmd_throw(void)
{
    int			dir, item;
    int			j, y, x, ny, nx, ty, tx;
    int			chance, tdam, tdis;
    int			mul, div;
    int			cur_dis, visible;

    inven_type          throw_obj;
    inven_type		*i_ptr;

    bool		ok_throw = FALSE;

    bool		hit_body = FALSE;

    int			missile_attr;
    int			missile_char;

    char		i_name[80];



    /* Get an item (from inven or floor) */
    if (!get_item(&item, "Throw which item? ", FALSE, TRUE, TRUE)) {
        if (item == -2) msg_print("You have nothing to throw.");
        return;
    }

    /* Access the item (if in the pack) */
    if (item >= 0) {
        i_ptr = &inventory[item];
    }
    else {
        i_ptr = &i_list[0 - item];
    }


    /* User can request throw */
    if (always_throw) {
        ok_throw = TRUE;
    }

    /* Known/felt items which are broken/cursed are crappy */
    else if ((inven_known_p(i_ptr) || (i_ptr->ident & ID_SENSE)) &&
             (cursed_p(i_ptr) || broken_p(i_ptr))) {
        ok_throw = TRUE;
    }

    /* Some things are just meant to be thrown */
    else if ((i_ptr->tval == TV_FLASK) ||
             (i_ptr->tval == TV_SHOT) ||
             (i_ptr->tval == TV_ARROW) ||
             (i_ptr->tval == TV_BOLT) ||
             (i_ptr->tval == TV_SPIKE) ||
             (i_ptr->tval == TV_JUNK) ||
             (i_ptr->tval == TV_BOTTLE) ||
             (i_ptr->tval == TV_SKELETON)) {
        ok_throw = TRUE;
    }

    /* Known Artifacts and Ego-Items are never "okay" */
    else if (inven_known_p(i_ptr) &&
             (artifact_p(i_ptr) || ego_item_p(i_ptr))) {
        ok_throw = FALSE;
    }

#if 0

    /* Normal weapons do damage when thrown */
    else if ((i_ptr->tval == TV_SWORD) ||
             (i_ptr->tval == TV_POLEARM) ||
             (i_ptr->tval == TV_HAFTED) ||
             (i_ptr->tval == TV_DIGGING)) {
        ok_throw = TRUE;
    }

#endif

    /* Food/Potions are only okay if they do MORE than one damage */
    else if (((i_ptr->tval == TV_FOOD) || (i_ptr->tval == TV_POTION)) &&
             (inven_aware_p(i_ptr)) && ((i_ptr->dd * i_ptr->ds) > 1)) {
        ok_throw = TRUE;
    }


    /* Verify if needed */
    if (!ok_throw) {

        char out_val[160];

        inven_type temp_obj;
        
        /* Create a "local missile object" */
        temp_obj = *i_ptr;
        temp_obj.number = 1;

        /* Description */
        objdes(i_name, &temp_obj, TRUE, 3);

        /* Verify */
        sprintf(out_val, "Really throw %s? ", i_name);
        if (!get_check(out_val)) return;
    }


    /* Get a direction (or cancel) */
    if (!get_aim_dir(&dir)) return;


    /* Create a "local missile object" */
    throw_obj = *i_ptr;
    throw_obj.number = 1;

    /* Reduce and describe inventory */
    if (item >= 0) {
        inven_item_increase(item, -1);
        inven_item_describe(item);
        inven_item_optimize(item);
    }

    /* Reduce and describe floor item */
    else {
        floor_item_increase(0 - item, -1);
        floor_item_optimize(0 - item);
    }

    /* Use the local object */
    i_ptr = &throw_obj;

    /* Description */
    objdes(i_name, i_ptr, FALSE, 3);

    /* Find the color and symbol for the object for throwing */
    missile_attr = inven_attr(i_ptr);
    missile_char = inven_char(i_ptr);


    /* Extract a "distance multiplier" */
    mul = 10;

    /* Enforce a minimum "weight" of one pound */
    div = ((i_ptr->weight > 10) ? i_ptr->weight : 10);

    /* Hack -- Distance -- Reward strength, penalize weight */
    tdis = (adj_str_blow[p_ptr->stat_ind[A_STR]] + 20) * mul / div;

    /* Max distance of 10 */
    if (tdis > 10) tdis = 10;

    /* Hack -- Base damage from thrown object */
    tdam = damroll(i_ptr->dd, i_ptr->ds) + i_ptr->to_d;
    
    /* Chance of hitting */
    chance = (p_ptr->skill_tht + (p_ptr->to_h * BTH_PLUS_ADJ));


    /* Take a turn */
    energy_use = 100;


    /* Start at the player */
    y = py;
    x = px;

    /* Predict the "target" location */
    tx = px + 99 * ddx[dir];
    ty = py + 99 * ddy[dir];

    /* Check for "target request" */
    if ((dir == 5) && target_okay()) {
        tx = target_col;
        ty = target_row;
    }


    /* Hack -- Handle stuff */
    handle_stuff();


    /* Travel until stopped */
    for (cur_dis = 0; cur_dis < tdis; ) {

        /* Hack -- Stop at the target */
        if ((y == ty) && (x == tx)) break;

        /* Calculate the new location (see "project()") */
        ny = y;
        nx = x;
        mmove2(&ny, &nx, py, px, ty, tx);

        /* Stopped by walls/doors */
        if (!floor_grid_bold(ny,nx)) break;

        /* Advance the distance */
        cur_dis++;

        /* Save the new location */
        x = nx;
        y = ny;


        /* The player can see the (on screen) missile */
        if (panel_contains(y, x) && player_can_see_bold(y, x)) {

            /* Draw, Hilite, Fresh, Pause, Erase */
            print_rel(missile_char, missile_attr, y, x);
            move_cursor_relative(y, x);
            Term_fresh();
            delay(10 * delay_spd);
            lite_spot(y, x);
            Term_fresh();
        }

        /* The player cannot see the missile */
        else {

            /* Pause anyway, for consistancy */
            delay(10 * delay_spd);
        }


        /* Monster here, Try to hit it */
        if (cave[y][x].m_idx) {

            cave_type *c_ptr = &cave[y][x];
            
            monster_type *m_ptr = &m_list[c_ptr->m_idx];
            monster_race *r_ptr = &r_info[m_ptr->r_idx];

            /* Check the visibility */
            visible = m_ptr->ml;

            /* Did we hit it (penalize range) */
            if (test_hit_fire(chance - cur_dis, r_ptr->ac, m_ptr->ml)) {

                bool fear = FALSE;
                
                /* Assume a default death */
                cptr note_dies = " dies.";

                /* Some monsters get "destroyed" */
                if ((r_ptr->flags3 & RF3_DEMON) ||
                    (r_ptr->flags3 & RF3_UNDEAD) ||
                    (r_ptr->flags2 & RF2_STUPID) ||
                    (strchr("Evg", r_ptr->r_char))) {

                    /* Special note at death */
                    note_dies = " is destroyed.";
                }


                /* Note the collision */
                hit_body = TRUE;


                /* Handle unseen monster */
                if (!visible) {

                    /* Invisible monster */
                    msg_format("The %s finds a mark.", i_name);
                }

                /* Handle visible monster */
                else {

                    char m_name[80];

                    /* Get "the monster" or "it" */
                    monster_desc(m_name, m_ptr, 0);

                    /* Message */
                    msg_format("The %s hits %s.", i_name, m_name);

                    /* Hack -- Track this monster race */
                    if (m_ptr->ml) recent_track(m_ptr->r_idx);

                    /* Hack -- Track this monster */
                    if (m_ptr->ml) health_track(c_ptr->m_idx);
                }

                /* Apply special damage XXX XXX XXX */
                tdam = tot_dam_aux(i_ptr, tdam, m_ptr);
                tdam = critical_shot(i_ptr->weight, i_ptr->to_h, tdam);

                /* No negative damage */
                if (tdam < 0) tdam = 0;

                /* Complex message */
                if (wizard) {
                    msg_format("You do %d (out of %d) damage.",
                               tdam, m_ptr->hp);
                }

                /* Hit the monster, check for death */
                if (mon_take_hit(c_ptr->m_idx, tdam, &fear, note_dies)) {

                    /* Dead monster */
                }

                /* No death */
                else {

                    /* Message */
                    message_pain(c_ptr->m_idx, tdam);

                    /* Take note */
                    if (fear && m_ptr->ml) {

                        char m_name[80];

                        /* Sound */
                        sound(SOUND_FLEE);

                        /* Get the monster name (or "it") */
                        monster_desc(m_name, m_ptr, 0);

                        /* Message */
                        msg_format("%^s flees in terror!", m_name);
                    }
                }
            }

            /* Stop looking */
            break;
        }
    }

    /* Chance of breakage */
    j = breakage_chance(i_ptr);

    /* Double the chance if we hit a monster */
    if (hit_body) j = j * 2;

    /* Paranoia -- maximum breakage chance */
    if (j > 100) j = 100;

    /* Drop (or break) near that location */
    drop_near(i_ptr, j, y, x);
}



