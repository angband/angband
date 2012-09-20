/* File: cmd1.c */

/* Purpose: Movement commands (part 1) */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"



/*
 * Determine if the player "hits" a monster (normal combat).
 * Note -- Always miss 5%, always hit 5%, otherwise random.
 */
bool test_hit_fire(int chance, int ac, int vis)
{
    int k;

    /* Percentile dice */
    k = rand_int(100);

    /* Hack -- Instant miss or hit */
    if (k < 10) return (k < 5);

    /* Never hit */
    if (chance <= 0) return (FALSE);

    /* Invisible monsters are harder to hit */
    if (!vis) chance = (chance + 1) / 2;

    /* Power competes against armor */
    if (rand_int(chance) < (ac * 3 / 4)) return (FALSE);

    /* Assume hit */
    return (TRUE);
}



/*
 * Determine if the player "hits" a monster (normal combat).
 *
 * Note -- Always miss 5%, always hit 5%, otherwise random.
 */
bool test_hit_norm(int chance, int ac, int vis)
{
    int k;

    /* Percentile dice */
    k = rand_int(100);

    /* Hack -- Instant miss or hit */
    if (k < 10) return (k < 5);

    /* Wimpy attack never hits */
    if (chance <= 0) return (FALSE);

    /* Penalize invisible targets */
    if (!vis) chance = (chance + 1) / 2;

    /* Power must defeat armor */
    if (rand_int(chance) < (ac * 3 / 4)) return (FALSE);

    /* Assume hit */
    return (TRUE);
}



/*
 * Critical hits (from objects thrown by player)
 * Factor in item weight, total plusses, and player level.
 */
s16b critical_shot(int weight, int plus, int dam)
{
    int i, k;

    /* Extract "shot" power */
    i = (weight + ((p_ptr->to_h + plus) * 4) + (p_ptr->lev * 2));

    /* Critical hit */
    if (randint(5000) <= i)
    {
        k = weight + randint(500);

        if (k < 500)
        {
            msg_print("It was a good hit!");
            dam = 2 * dam + 5;
        }
        else if (k < 1000)
        {
            msg_print("It was a great hit!");
            dam = 2 * dam + 10;
        }
        else
        {
            msg_print("It was a superb hit!");
            dam = 3 * dam + 15;
        }
    }

    return (dam);
}



/*
 * Critical hits (by player)
 *
 * Factor in weapon weight, total plusses, player level.
 */
s16b critical_norm(int weight, int plus, int dam)
{
    int i, k;

    /* Extract "blow" power */
    i = (weight + ((p_ptr->to_h + plus) * 5) + (p_ptr->lev * 3));

    /* Chance */
    if (randint(5000) <= i)
    {
        k = weight + randint(650);

        if (k < 400)
        {
            msg_print("It was a good hit!");
            dam = 2 * dam + 5;
        }
        else if (k < 700)
        {
            msg_print("It was a great hit!");
            dam = 2 * dam + 10;
        }
        else if (k < 900)
        {
            msg_print("It was a superb hit!");
            dam = 3 * dam + 15;
        }
        else if (k < 1300)
        {
            msg_print("It was a *GREAT* hit!");
            dam = 3 * dam + 20;
        }
        else
        {
            msg_print("It was a *SUPERB* hit!");
            dam = ((7 * dam) / 2) + 25;
        }
    }

    return (dam);
}



/*
 * Extract the "total damage" from a given object hitting a given monster.
 *
 * Note that "flasks of oil" do NOT do fire damage, although they
 * certainly could be made to do so.  XXX XXX
 *
 * Note that most brands and slays are x3, except Slay Animal (x2),
 * Slay Evil (x2), and Kill dragon (x5).
 */
s16b tot_dam_aux(object_type *i_ptr, int tdam, monster_type *m_ptr)
{
    int mult = 1;

    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    u32b f1, f2, f3;

    /* Extract the flags */
    object_flags(i_ptr, &f1, &f2, &f3);

    /* Some "weapons" and "ammo" do extra damage */
    switch (i_ptr->tval)
    {
        case TV_SHOT:
        case TV_ARROW:
        case TV_BOLT:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_SWORD:
        case TV_DIGGING:

            /* Slay Animal */
            if ((f1 & TR1_SLAY_ANIMAL) &&
                (r_ptr->flags3 & RF3_ANIMAL))
            {
                if (m_ptr->ml) r_ptr->r_flags3 |= RF3_ANIMAL;

                if (mult < 2) mult = 2;
            }

            /* Slay Evil */
            if ((f1 & TR1_SLAY_EVIL) &&
                (r_ptr->flags3 & RF3_EVIL))
            {
                if (m_ptr->ml) r_ptr->r_flags3 |= RF3_EVIL;

                if (mult < 2) mult = 2;
            }

            /* Slay Undead */
            if ((f1 & TR1_SLAY_UNDEAD) &&
                (r_ptr->flags3 & RF3_UNDEAD))
            {
                if (m_ptr->ml) r_ptr->r_flags3 |= RF3_UNDEAD;

                if (mult < 3) mult = 3;
            }

            /* Slay Demon */
            if ((f1 & TR1_SLAY_DEMON) &&
                (r_ptr->flags3 & RF3_DEMON))
            {
                if (m_ptr->ml) r_ptr->r_flags3 |= RF3_DEMON;

                if (mult < 3) mult = 3;
            }

            /* Slay Orc */
            if ((f1 & TR1_SLAY_ORC) &&
                (r_ptr->flags3 & RF3_ORC))
            {
                if (m_ptr->ml) r_ptr->r_flags3 |= RF3_ORC;

                if (mult < 3) mult = 3;
            }

            /* Slay Troll */
            if ((f1 & TR1_SLAY_TROLL) &&
                (r_ptr->flags3 & RF3_TROLL))
            {
                if (m_ptr->ml) r_ptr->r_flags3 |= RF3_TROLL;

                if (mult < 3) mult = 3;
            }

            /* Slay Giant */
            if ((f1 & TR1_SLAY_GIANT) &&
                (r_ptr->flags3 & RF3_GIANT))
            {
                if (m_ptr->ml) r_ptr->r_flags3 |= RF3_GIANT;

                if (mult < 3) mult = 3;
            }

            /* Slay Dragon  */
            if ((f1 & TR1_SLAY_DRAGON) &&
                (r_ptr->flags3 & RF3_DRAGON))
            {
                if (m_ptr->ml) r_ptr->r_flags3 |= RF3_DRAGON;

                if (mult < 3) mult = 3;
            }

            /* Execute Dragon */
            if ((f1 & TR1_KILL_DRAGON) &&
                (r_ptr->flags3 & RF3_DRAGON))
            {
                if (m_ptr->ml) r_ptr->r_flags3 |= RF3_DRAGON;

                if (mult < 5) mult = 5;
            }


            /* Brand (Acid) */
            if (f1 & TR1_BRAND_ACID)
            {
                /* Notice immunity */
                if (r_ptr->flags3 & RF3_IM_ACID)
                {
                    if (m_ptr->ml) r_ptr->r_flags3 |= RF3_IM_ACID;
                }

                /* Otherwise, take the damage */
                else
                {
                    if (mult < 3) mult = 3;
                }
            }

            /* Brand (Elec) */
            if (f1 & TR1_BRAND_ELEC)
            {
                /* Notice immunity */
                if (r_ptr->flags3 & RF3_IM_ELEC)
                {
                    if (m_ptr->ml) r_ptr->r_flags3 |= RF3_IM_ELEC;
                }

                /* Otherwise, take the damage */
                else
                {
                    if (mult < 3) mult = 3;
                }
            }

            /* Brand (Fire) */
            if (f1 & TR1_BRAND_FIRE)
            {
                /* Notice immunity */
                if (r_ptr->flags3 & RF3_IM_FIRE)
                {
                    if (m_ptr->ml) r_ptr->r_flags3 |= RF3_IM_FIRE;
                }

                /* Otherwise, take the damage */
                else
                {
                    if (mult < 3) mult = 3;
                }
            }

            /* Brand (Cold) */
            if (f1 & TR1_BRAND_COLD)
            {
                /* Notice immunity */
                if (r_ptr->flags3 & RF3_IM_COLD)
                {
                    if (m_ptr->ml) r_ptr->r_flags3 |= RF3_IM_COLD;
                }

                /* Otherwise, take the damage */
                else
                {
                    if (mult < 3) mult = 3;
                }
          }
    }


    /* Return the total damage */
    return (tdam * mult);
}


/*
 * Searches for hidden things.			-RAK-	
 */
void search(void)
{
    int           y, x, chance;

    cave_type    *c_ptr;
    object_type  *i_ptr;


    /* Start with base search ability */
    chance = p_ptr->skill_srh;

    /* Penalize various conditions */
    if (p_ptr->blind || no_lite()) chance = chance / 10;
    if (p_ptr->confused || p_ptr->image) chance = chance / 10;

    /* Search the nearby grids, which are always in bounds */
    for (y = (py - 1); y <= (py + 1); y++)
    {
        for (x = (px - 1); x <= (px + 1); x++)
        {
            /* Sometimes, notice things */
            if (rand_int(100) < chance)
            {
                /* Access the grid */
                c_ptr = &cave[y][x];

                /* Access the object */
                i_ptr = &i_list[c_ptr->i_idx];

                /* Invisible trap */
                if (c_ptr->ftyp == 0x02)
                {
                    /* Pick a trap */
                    pick_trap(y, x);

                    /* Message */
                    msg_print("You have found a trap.");

                    /* Disturb */
                    disturb(0, 0);
                }

                /* Secret door */
                else if (c_ptr->ftyp == 0x30)
                {
                    /* Message */
                    msg_print("You have found a secret door.");

                    /* Pick a door XXX XXX XXX */
                    c_ptr->ftyp = 0x20;

                    /* Notice */
                    note_spot(y, x);

                    /* Redraw */
                    lite_spot(y, x);

                    /* Disturb */
                    disturb(0, 0);
                }

                /* Search chests */
                else if (i_ptr->tval == TV_CHEST)
                {
                    /* Examine chests for traps */
                    if (!object_known_p(i_ptr) && (chest_traps[i_ptr->pval]))
                    {

                        /* Message */
                        msg_print("You have discovered a trap on the chest!");

                        /* Know the trap */
                        object_known(i_ptr);

                        /* Notice it */
                        disturb(0, 0);
                    }
                }
            }
        }
    }
}




/*
 * Player "wants" to pick up an object or gold.
 * Note that we ONLY handle things that can be picked up.
 * See "move_player()" for handling of other things.
 */
void carry(int pickup)
{
    cave_type  *c_ptr = &cave[py][px];

    object_type *i_ptr;

    char	i_name[80];


    /* Hack -- nothing here to pick up */
    if (!(c_ptr->i_idx)) return;

    /* Get the object */
    i_ptr = &i_list[c_ptr->i_idx];

    /* Describe the object */
    object_desc(i_name, i_ptr, TRUE, 3);

    /* Pick up gold */
    if (i_ptr->tval == TV_GOLD)
    {
        /* Disturb */
        disturb(0, 0);

        /* Message */
        msg_format("You have found %ld gold pieces worth of %s.",
                   (long)i_ptr->pval, i_name);

        /* Collect the gold */
        p_ptr->au += i_ptr->pval;

        /* Redraw gold */
        p_ptr->redraw |= (PR_GOLD);

        /* Delete gold */
        delete_object(py, px);
    }

    /* Pick it up */
    else
    {
        /* Hack -- disturb */
        disturb(0, 0);

        /* Describe the object */
        if (!pickup)
        {
            msg_format("You see %s.", i_name);
        }

        /* Note that the pack is too full */
        else if (!inven_carry_okay(i_ptr))
        {
            msg_format("You have no room for %s.", i_name);
        }

        /* Pick up the item (if requested and allowed) */
        else
        {
            int okay = TRUE;

            /* Hack -- query every item */
            if (carry_query_flag)
            {	
                char out_val[160];
                sprintf(out_val, "Pick up %s? ", i_name);
                okay = get_check(out_val);
            }

            /* Attempt to pick up an object. */
            if (okay)
            {
                int slot;

                /* Carry the item */
                slot = inven_carry(i_ptr);

                /* Get the item again */
                i_ptr = &inventory[slot];

                /* Describe the object */
                object_desc(i_name, i_ptr, TRUE, 3);

                /* Message */
                msg_format("You have %s (%c).", i_name, index_to_label(slot));

                /* Delete original */
                delete_object(py, px);
            }
        }
    }
}





/*
 * Determine if a trap affects the player.
 * Always miss 5% of the time, Always hit 5% of the time.
 * Otherwise, match trap power against player armor.
 */
static int check_hit(int power)
{
    int k, ac;

    /* Percentile dice */
    k = rand_int(100);

    /* Hack -- 5% hit, 5% miss */
    if (k < 10) return (k < 5);

    /* Paranoia -- No power */
    if (power <= 0) return (FALSE);

    /* Total armor */
    ac = p_ptr->ac + p_ptr->to_a;

    /* Power competes against Armor */
    if (randint(power) > ((ac * 3) / 4)) return (TRUE);

    /* Assume miss */
    return (FALSE);
}



/*
 * Handle player hitting a real trap
 */
static void hit_trap(void)
{
    int			i, num, dam;

    cave_type		*c_ptr;

    cptr		name = "a trap";


    /* Disturb the player */
    disturb(0, 0);

    /* Get the cave grid */
    c_ptr = &cave[py][px];

    /* Examine the trap sub-val */
    switch (c_ptr->ftyp)
    {
        case 0x10:
            msg_print("You fell through a trap door!");
            if (p_ptr->ffall)
            {
                msg_print("You float gently down to the next level.");
            }
            else
            {
                dam = damroll(2,8);
                take_hit(dam, name);
            }
            new_level_flag = TRUE;
            dun_level++;
            break;

        case 0x11:
            msg_print("You fell into a pit!");
            if (p_ptr->ffall)
            {
                msg_print("You float gently to the bottom of the pit.");
            }
            else
            {
                dam = damroll(2, 6);
                take_hit(dam, name);
            }
            break;

        case 0x12:

            msg_print("You fall into a spiked pit!");

            if (p_ptr->ffall)
            {
                msg_print("You float gently to the floor of the pit.");
                msg_print("You carefully avoid touching the spikes.");
            }

            else
            {
                /* Base damage */
                dam = damroll(2,6);

                /* Extra spike damage */
                if (rand_int(100) < 50)
                {

                    msg_print("You are impaled!");

                    dam = dam * 2;
                    (void)set_cut(p_ptr->cut + randint(dam));
                }

                /* Take the damage */
                take_hit(dam, name);
            }
            break;

        case 0x13:

            msg_print("You fall into a spiked pit!");

            if (p_ptr->ffall)
            {
                msg_print("You float gently to the floor of the pit.");
                msg_print("You carefully avoid touching the spikes.");
            }

            else
            {
                /* Base damage */
                dam = damroll(2,6);

                /* Extra spike damage */
                if (rand_int(100) < 50)
                {
                    msg_print("You are impaled on poisonous spikes!");

                    dam = dam * 2;
                    (void)set_cut(p_ptr->cut + randint(dam));

                    if (p_ptr->resist_pois || p_ptr->oppose_pois)
                    {
                        msg_print("The poison does not affect you!");
                    }

                    else
                    {
                        dam = dam * 2;
                        (void)set_poisoned(p_ptr->poisoned + randint(dam));
                    }
                }

                /* Take the damage */
                take_hit(dam, name);
            }

            break;

        case 0x14:
            msg_print("You are enveloped in a cloud of smoke!");
            c_ptr->ftyp = 0x01;
            c_ptr->fdat &= ~CAVE_MARK;
            note_spot(py,px);
            lite_spot(py,px);
            num = 2 + randint(3);
            for (i = 0; i < num; i++)
            {
                (void)summon_specific(py, px, dun_level, 0);
            }
            break;

        case 0x15:
            msg_print("You hit a teleport trap!");
            teleport_player(100);
            break;

        case 0x16:
            msg_print("You are enveloped in flames!");
            dam = damroll(4, 6);
            fire_dam(dam, "a fire trap");
            break;

        case 0x17:
            msg_print("You are splashed with acid!");
            dam = damroll(4, 6);
            acid_dam(dam, "an acid trap");
            break;

        case 0x18:
            if (check_hit(125))
            {
                msg_print("A small dart hits you!");
                dam = damroll(1,4);
                take_hit(dam, name);
                (void)set_slow(p_ptr->slow + rand_int(20) + 20);
            }
            else
            {
                msg_print("A small dart barely misses you.");
            }
            break;

        case 0x19:
            if (check_hit(125))
            {
                msg_print("A small dart hits you!");
                dam = damroll(1,4);
                take_hit(dam, name);
                (void)do_dec_stat(A_STR);
            }
            else
            {
                msg_print("A small dart barely misses you.");
            }
            break;

        case 0x1A:
            if (check_hit(125))
            {
                msg_print("A small dart hits you!");
                dam = damroll(1,4);
                take_hit(dam, name);
                (void)do_dec_stat(A_DEX);
            }
            else
            {
                msg_print("A small dart barely misses you.");
            }
            break;

        case 0x1B:
            if (check_hit(125))
            {
                msg_print("A small dart hits you!");
                dam = damroll(1,4);
                take_hit(dam, name);
                (void)do_dec_stat(A_CON);
            }
            else
            {
                msg_print("A small dart barely misses you.");
            }
            break;

        case 0x1C:
            msg_print("A black gas surrounds you!");
            if (!p_ptr->resist_blind)
            {
                (void)set_blind(p_ptr->blind + rand_int(50) + 25);
            }
            break;

        case 0x1D:
            msg_print("A gas of scintillating colors surrounds you!");
            if (!p_ptr->resist_conf)
            {
                (void)set_confused(p_ptr->confused + rand_int(20) + 10);
            }
            break;

        case 0x1E:
            msg_print("A pungent green gas surrounds you!");
            if (!p_ptr->resist_pois && !p_ptr->oppose_pois)
            {
                (void)set_poisoned(p_ptr->poisoned + rand_int(20) + 10);
            }
            break;

        case 0x1F:
            msg_print("A strange white mist surrounds you!");
            if (!p_ptr->free_act)
            {
                (void)set_paralyzed(p_ptr->paralyzed + rand_int(10) + 5);
            }
            break;
    }
}



/*
 * Player attacks a (poor, defenseless) creature	-RAK-	
 *
 * If no "weapon" is available, then "punch" the monster one time.
 */
void py_attack(int y, int x)
{
    int			num = 0, k, bonus, chance;

    cave_type		*c_ptr = &cave[y][x];

    monster_type	*m_ptr = &m_list[c_ptr->m_idx];
    monster_race	*r_ptr = &r_info[m_ptr->r_idx];

    object_type		*i_ptr;

    char		m_name[80];

    bool		fear = FALSE;

    bool		do_quake = FALSE;


    /* Disturb the player */
    disturb(0, 0);


    /* Disturb the monster */
    m_ptr->csleep = 0;


    /* Extract monster name (or "it") */
    monster_desc(m_name, m_ptr, 0);


    /* Auto-Recall if possible and visible */
    if (m_ptr->ml) recent_track(m_ptr->r_idx);

    /* Track a new monster */
    if (m_ptr->ml) health_track(c_ptr->m_idx);


    /* Handle player fear */
    if (p_ptr->afraid)
    {
        /* Message */
        msg_format("You are too afraid to attack %s!", m_name);

        /* Done */
        return;
    }


    /* Access the weapon */
    i_ptr = &inventory[INVEN_WIELD];

    /* Calculate the "attack quality" */
    bonus = p_ptr->to_h + i_ptr->to_h;
    chance = (p_ptr->skill_thn + (bonus * BTH_PLUS_ADJ));


    /* Attack once for each legal blow */
    while (num++ < p_ptr->num_blow)
    {
        /* Test for hit */
        if (test_hit_norm(chance, r_ptr->ac, m_ptr->ml))
        {
            /* Sound */
            sound(SOUND_HIT);

            /* Message */
            msg_format("You hit %s.", m_name);

            /* Hack -- bare hands do one damage */
            k = 1;

            /* Handle normal weapon */
            if (i_ptr->k_idx)
            {
                k = damroll(i_ptr->dd, i_ptr->ds);
                k = tot_dam_aux(i_ptr, k, m_ptr);
                if (p_ptr->impact && (k > 50)) do_quake = TRUE;
                k = critical_norm(i_ptr->weight, i_ptr->to_h, k);
                k += i_ptr->to_d;
            }

            /* Apply the player damage bonuses */
            k += p_ptr->to_d;

            /* No negative damage */
            if (k < 0) k = 0;

            /* Complex message */
            if (wizard)
            {
                msg_format("You do %d (out of %d) damage.", k, m_ptr->hp);
            }

            /* Damage, check for fear and death */
            if (mon_take_hit(c_ptr->m_idx, k, &fear, NULL)) break;

            /* Confusion attack */
            if (p_ptr->confusing)
            {
                /* Cancel glowing hands */
                p_ptr->confusing = FALSE;

                /* Message */
                msg_print("Your hands stop glowing.");

                /* Confuse the monster */
                if (r_ptr->flags3 & RF3_NO_CONF)
                {
                    if (m_ptr->ml) r_ptr->r_flags3 |= RF3_NO_CONF;
                    msg_format("%^s is unaffected.", m_name);
                }
                else if (rand_int(100) < r_ptr->level)
                {
                    msg_format("%^s is unaffected.", m_name);
                }
                else
                {
                    msg_format("%^s appears confused.", m_name);
                    m_ptr->confused += 10 + rand_int(p_ptr->lev) / 5;
                }
            }
        }

        /* Player misses */
        else
        {
            /* Sound */
            sound(SOUND_MISS);

            /* Message */
            msg_format("You miss %s.", m_name);
        }
    }


    /* Hack -- delay fear messages */
    if (fear && m_ptr->ml)
    {
        /* Sound */
        sound(SOUND_FLEE);

        /* Message */
        msg_format("%^s flees in terror!", m_name);
    }


    /* Mega-Hack -- apply earthquake brand */
    if (do_quake) earthquake(py, px, 10);
}





/*
 * Move player in the given direction, with the given "pickup" flag.
 *
 * This routine should (probably) always induce energy expenditure.
 *
 * Note that moving will *always* take a turn, and will *always* hit
 * any monster which might be in the destination grid.  Previously,
 * moving into walls was "free" and did NOT hit invisible monsters.
 */
void move_player(int dir, int do_pickup)
{
    int			y, x;

    cave_type		*c_ptr;
    object_type		*i_ptr;
    monster_type	*m_ptr;


    /* Find the result of moving */
    y = py + ddy[dir];
    x = px + ddx[dir];

    /* Examine the destination */
    c_ptr = &cave[y][x];

    /* Get the object */
    i_ptr = &i_list[c_ptr->i_idx];

    /* Get the monster */
    m_ptr = &m_list[c_ptr->m_idx];


    /* Hack -- attack monsters */
    if (c_ptr->m_idx)
    {
        /* Attack */
        py_attack(y, x);
    }

    /* Player can not walk through "walls" */
    else if (!floor_grid_bold(y,x))
    {
        /* Disturb the player */
        disturb(0, 0);

        /* Notice things in the dark */
        if (!(c_ptr->fdat & CAVE_MARK) &&
            (p_ptr->blind || !(c_ptr->fdat & CAVE_LITE)))
        {
            /* Rubble */
            if (c_ptr->ftyp == 0x31)
            {
                msg_print("You feel some rubble blocking your way.");
                c_ptr->fdat |= CAVE_MARK;
                lite_spot(y, x);
            }

            /* Closed door */
            else if (c_ptr->ftyp < 0x30)
            {
                msg_print("You feel a closed door blocking your way.");
                c_ptr->fdat |= CAVE_MARK;
                lite_spot(y, x);
            }

            /* Wall (or secret door) */
            else
            {
                msg_print("You feel a wall blocking your way.");
                c_ptr->fdat |= CAVE_MARK;
                lite_spot(y, x);
            }
        }

        /* Notice things */
        else
        {
            /* Rubble */
            if (c_ptr->ftyp == 0x31)
            {
                msg_print("There is rubble blocking your way.");
            }

            /* Closed doors */
            else if (c_ptr->ftyp < 0x30)
            {
                msg_print("There is a closed door blocking your way.");
            }

            /* Wall (or secret door) */
            else
            {
                msg_print("There is a wall blocking your way.");
            }
        }
    }

    /* Normal movement */
    else
    {
        int oy, ox;

        /* Save old location */
        oy = py;
        ox = px;

        /* Move the player */
        py = y;
        px = x;

        /* Redraw new spot */
        lite_spot(py, px);

        /* Redraw old spot */
        lite_spot(oy, ox);

        /* Check for new panel (redraw map) */
        verify_panel();

        /* Update stuff */
        p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);

        /* Update the monsters */
        p_ptr->update |= (PU_DISTANCE);


        /* Spontaneous Searching */
        if ((p_ptr->skill_fos >= 50) ||
            (0 == rand_int(50 - p_ptr->skill_fos)))
        {
            search();
        }

        /* Continuous Searching */
        if (p_ptr->searching)
        {
            search();
        }

        /* Handle "objects" */
        if (c_ptr->i_idx) carry(do_pickup);

        /* Handle "store doors" */
        if ((c_ptr->ftyp >= 0x08) &&
            (c_ptr->ftyp <= 0x0F))
        {
            /* Disturb */
            disturb(0, 0);

            /* Hack -- Enter store */
            command_new = '_';
        }

        /* Discover invisible traps */
        else if (c_ptr->ftyp == 0x02)
        {
            /* Disturb */
            disturb(0, 0);

            /* Message */
            msg_print("You found a trap!");

            /* Pick a trap */
            pick_trap(py, px);

            /* Hit the trap */
            hit_trap();
        }

        /* Set off an visible trap */
        else if ((c_ptr->ftyp >= 0x10) &&
                 (c_ptr->ftyp <= 0x1F))
        {
            /* Disturb */
            disturb(0, 0);

            /* Hit the trap */
            hit_trap();
        }
    }
}


/*
 * Hack -- Check for a "motion blocker" (see below)
 */
static int see_wall(int dir, int y, int x)
{
    /* Get the new location */
    y += ddy[dir];
    x += ddx[dir];

    /* Illegal grids are blank */
    if (!in_bounds2(y, x)) return (FALSE);

    /* Must actually block motion */
    if (cave[y][x].ftyp < 0x20) return (FALSE);

    /* Must be known to the player */
    if (!(cave[y][x].fdat & CAVE_MARK)) return (FALSE);

    /* Default */
    return (TRUE);
}


/*
 * Hack -- Check for an "unknown corner" (see below)
 */
static int see_nothing(int dir, int y, int x)
{
    /* Get the new location */
    y += ddy[dir];
    x += ddx[dir];

    /* Illegal grids are unknown */
    if (!in_bounds2(y,x)) return (TRUE);

    /* Memorized grids are known */
    if (cave[y][x].fdat & CAVE_MARK) return (FALSE);

    /* Non-floor grids are unknown */
    if (!floor_grid_bold(y,x)) return (TRUE);

    /* Viewable grids are known */
    if (player_can_see_bold(y,x)) return (FALSE);

    /* Default */
    return (TRUE);
}






/*
   The running algorithm:			-CJS-

   In the diagrams below, the player has just arrived in the
   grid marked as '@', and he has just come from a grid marked
   as 'o', and he is about to enter the grid marked as 'x'.

   Of course, if the "requested" move was impossible, then you
   will of course be blocked, and will stop.

   Overview: You keep moving until something interesting happens.
   If you are in an enclosed space, you follow corners. This is
   the usual corridor scheme. If you are in an open space, you go
   straight, but stop before entering enclosed space. This is
   analogous to reaching doorways. If you have enclosed space on
   one side only (that is, running along side a wall) stop if
   your wall opens out, or your open space closes in. Either case
   corresponds to a doorway.

   What happens depends on what you can really SEE. (i.e. if you
   have no light, then running along a dark corridor is JUST like
   running in a dark room.) The algorithm works equally well in
   corridors, rooms, mine tailings, earthquake rubble, etc, etc.

   These conditions are kept in static memory:
        find_openarea	 You are in the open on at least one
                         side.
        find_breakleft	 You have a wall on the left, and will
                         stop if it opens
        find_breakright	 You have a wall on the right, and will
                         stop if it opens

   To initialize these conditions, we examine the grids adjacent
   to the grid marked 'x', two on each side (marked 'L' and 'R').
   If either one of the two grids on a given side is seen to be
   closed, then that side is considered to be closed. If both
   sides are closed, then it is an enclosed (corridor) run.

         LL		L
        @x	       LxR
         RR	       @R

   Looking at more than just the immediate squares is
   significant. Consider the following case. A run along the
   corridor will stop just before entering the center point,
   because a choice is clearly established. Running in any of
   three available directions will be defined as a corridor run.
   Note that a minor hack is inserted to make the angled corridor
   entry (with one side blocked near and the other side blocked
   further away from the runner) work correctly. The runner moves
   diagonally, but then saves the previous direction as being
   straight into the gap. Otherwise, the tail end of the other
   entry would be perceived as an alternative on the next move.

           #.#
          ##.##
          .@x..
          ##.##
           #.#

   Likewise, a run along a wall, and then into a doorway (two
   runs) will work correctly. A single run rightwards from @ will
   stop at 1. Another run right and down will enter the corridor
   and make the corner, stopping at the 2.

        #@x	  1
        ########### ######
        2	    #
        #############
        #

   After any move, the function area_affect is called to
   determine the new surroundings, and the direction of
   subsequent moves. It examines the current player location
   (at which the runner has just arrived) and the previous
   direction (from which the runner is considered to have come).

   Moving one square in some direction places you adjacent to
   three or five new squares (for straight and diagonal moves
   respectively) to which you were not previously adjacent,
   marked as '!' in the diagrams below.

       ...!	  ...	
       .o@!	  .o.!	
       ...!	  ..@!	
                   !!!

   You STOP if any of the new squares are interesting in any way:
   for example, if they contain visible monsters or treasure.

   You STOP if any of the newly adjacent squares seem to be open,
   and you are also looking for a break on that side. (that is,
   find_openarea AND find_break).

   You STOP if any of the newly adjacent squares do NOT seem to be
   open and you are in an open area, and that side was previously
   entirely open.

   Corners: If you are not in the open (i.e. you are in a corridor)
   and there is only one way to go in the new squares, then turn in
   that direction. If there are more than two new ways to go, STOP.
   If there are two ways to go, and those ways are separated by a
   square which does not seem to be open, then STOP.

   Otherwise, we have a potential corner. There are two new open
   squares, which are also adjacent. One of the new squares is
   diagonally located, the other is straight on (as in the diagram).
   We consider two more squares further out (marked below as ?).

   We assign "option" to the straight-on grid, and "option2" to the
   diagonal grid, and "check_dir" to the grid marked 's'.

          .s
         @x?
          #?

   If they are both seen to be closed, then it is seen that no
   benefit is gained from moving straight. It is a known corner.
   To cut the corner, go diagonally, otherwise go straight, but
   pretend you stepped diagonally into that next location for a
   full view next time. Conversely, if one of the ? squares is
   not seen to be closed, then there is a potential choice. We check
   to see whether it is a potential corner or an intersection/room entrance.
   If the square two spaces straight ahead, and the space marked with 's'
   are both blank, then it is a potential corner and enter if find_examine
   is set, otherwise must stop because it is not a corner.
*/




/*
 * Hack -- allow quick "cycling" through the legal directions
 */
static byte cycle[] = { 1, 2, 3, 6, 9, 8, 7, 4, 1, 2, 3, 6, 9, 8, 7, 4, 1 };

/*
 * Hack -- map each direction into the "middle" of the "cycle[]" array
 */
static byte chome[] = { 0, 8, 9, 10, 7, 0, 11, 6, 5, 4 };

/*
 * The direction we are running
 */
static byte find_current;

/*
 * The direction we came from
 */
static byte find_prevdir;

/*
 * We are looking for open area
 */
static bool find_openarea;

/*
 * We are looking for a break
 */
static bool find_breakright;
static bool find_breakleft;



/*
 * Initialize the running algorithm for a new direction.
 *
 * Diagonal Corridor -- allow diaginal entry into corridors.
 *
 * Blunt Corridor -- If there is a wall two spaces ahead and
 * we seem to be in a corridor, then force a turn into the side
 * corridor, must be moving straight into a corridor here. ???
 *
 * Diagonal Corridor    Blunt Corridor (?)
 *       # #                  #
 *       #x#                 @x#
 *       @p.                  p
 */
static void run_init(int dir)
{
    int		row, col, deepleft, deepright;
    int		i, shortleft, shortright;


    /* Save the direction */
    find_current = dir;

    /* Assume running straight */
    find_prevdir = dir;

    /* Assume looking for open area */
    find_openarea = TRUE;

    /* Assume not looking for breaks */
    find_breakright = find_breakleft = FALSE;

    /* Assume no nearby walls */
    deepleft = deepright = FALSE;
    shortright = shortleft = FALSE;

    /* Find the destination grid */
    row = py + ddy[dir];
    col = px + ddx[dir];

    /* Extract cycle index */
    i = chome[dir];

    /* Check for walls */
    if (see_wall(cycle[i+1], py, px))
    {
        find_breakleft = TRUE;
        shortleft = TRUE;
    }
    else if (see_wall(cycle[i+1], row, col))
    {
        find_breakleft = TRUE;
        deepleft = TRUE;
    }

    /* Check for walls */
    if (see_wall(cycle[i-1], py, px))
    {
        find_breakright = TRUE;
        shortright = TRUE;
    }
    else if (see_wall(cycle[i-1], row, col))
    {
        find_breakright = TRUE;
        deepright = TRUE;
    }

    /* Looking for a break */
    if (find_breakleft && find_breakright)
    {
        /* Not looking for open area */
        find_openarea = FALSE;

        /* Hack -- allow angled corridor entry */
        if (dir & 0x01)
        {
            if (deepleft && !deepright)
            {
                find_prevdir = cycle[i - 1];
            }
            else if (deepright && !deepleft)
            {
                find_prevdir = cycle[i + 1];
            }
        }

        /* Hack -- allow blunt corridor entry */
        else if (see_wall(cycle[i], row, col))
        {
            if (shortleft && !shortright)
            {
                find_prevdir = cycle[i - 2];
            }
            else if (shortright && !shortleft)
            {
                find_prevdir = cycle[i + 2];
            }
        }
    }
}


/*
 * Update the current "run" path
 *
 * Return TRUE if the running should be stopped
 */
static bool run_test(void)
{
    int			prev_dir, new_dir, check_dir = 0;

    int			row, col;
    int			i, max, inv;
    int			option, option2;

    cave_type		*c_ptr;


    /* No options yet */
    option = 0;
    option2 = 0;

    /* Where we came from */
    prev_dir = find_prevdir;


    /* Range of newly adjacent grids */
    max = (prev_dir & 0x01) + 1;


    /* Look at every newly adjacent square. */
    for (i = -max; i <= max; i++)
    {
        new_dir = cycle[chome[prev_dir] + i];

        row = py + ddy[new_dir];
        col = px + ddx[new_dir];

        c_ptr = &cave[row][col];


        /* Visible monsters abort running */
        if (c_ptr->m_idx)
        {
            monster_type *m_ptr = &m_list[c_ptr->m_idx];

            /* Visible monster */
            if (m_ptr->ml) return (TRUE);
        }

        /* Visible objects abort running */
        if (c_ptr->i_idx)
        {
            object_type *i_ptr = &i_list[c_ptr->i_idx];

            /* Visible object */
            if (i_ptr->marked) return (TRUE);
        }


        /* Assume unknown */
        inv = TRUE;

        /* Check memorized grids */
        if (c_ptr->fdat & CAVE_MARK)
        {
            bool notice = TRUE;

            /* Examine the terrain */
            switch (c_ptr->ftyp)
            {
                /* Floors */
                case 0x01:

                /* Invis traps */
                case 0x02:

                /* Secret doors */
                case 0x30:

                /* Normal veins */
                case 0x32:
                case 0x33:

                /* Hidden treasure */
                case 0x34:
                case 0x35:

                /* Walls */
                case 0x38:
                case 0x39:
                case 0x3A:
                case 0x3B:
                case 0x3C:
                case 0x3D:
                case 0x3E:
                case 0x3F:

                    /* Ignore */
                    notice = FALSE;

                    /* Done */
                    break;

                /* Stairs */
                case 0x06:
                case 0x07:

                    /* Option -- ignore */
                    if (find_ignore_stairs) notice = FALSE;

                    /* Done */
                    break;

                /* Open doors */
                case 0x04:
                case 0x05:

                    /* Option -- ignore */
                    if (find_ignore_doors) notice = FALSE;

                    /* Done */
                    break;
            }

            /* Interesting feature */
            if (notice) return (TRUE);

            /* The grid is "visible" */
            inv = FALSE;
        }

        /* Analyze unknown grids and floors */
        if (inv || floor_grid_bold(row, col))
        {
            /* Looking for open area */
            if (find_openarea)
            {
                /* Nothing */
            }

            /* The first new direction. */
            else if (!option)
            {
                option = new_dir;
            }

            /* Three new directions. Stop running. */
            else if (option2)
            {
                return (TRUE);
            }

            /* Two non-adjacent new directions.  Stop running. */
            else if (option != cycle[chome[prev_dir] + i - 1])
            {
                return (TRUE);
            }

            /* Two new (adjacent) directions (case 1) */
            else if (new_dir & 0x01)
            {
                check_dir = cycle[chome[prev_dir] + i - 2];
                option2 = new_dir;
            }

            /* Two new (adjacent) directions (case 2) */
            else
            {
                check_dir = cycle[chome[prev_dir] + i + 1];
                option2 = option;
                option = new_dir;
            }
        }

        /* Obstacle, while looking for open area */
        else
        {
            if (find_openarea)
            {
                if (i < 0)
                {
                    /* Break to the right */
                    find_breakright = TRUE;
                }

                else if (i > 0)
                {
                    /* Break to the left */
                    find_breakleft = TRUE;
                }
            }
        }
    }


    /* Looking for open area */
    if (find_openarea)
    {
        /* Hack -- look again */
        for (i = -max; i < 0; i++)
        {
            new_dir = cycle[chome[prev_dir] + i];

            row = py + ddy[new_dir];
            col = px + ddx[new_dir];

            /* Unknown grid or floor */
            if (!(cave[row][col].fdat & CAVE_MARK) || floor_grid_bold(row, col))
            {
                /* Looking to break right */
                if (find_breakright)
                {
                    return (TRUE);
                }
            }

            /* Obstacle */
            else
            {
                /* Looking to break left */
                if (find_breakleft)
                {
                    return (TRUE);
                }
            }
        }

        /* Hack -- look again */
        for (i = max; i > 0; i--)
        {
            new_dir = cycle[chome[prev_dir] + i];

            row = py + ddy[new_dir];
            col = px + ddx[new_dir];

            /* Unknown grid or floor */
            if (!(cave[row][col].fdat & CAVE_MARK) || floor_grid_bold(row, col))
            {
                /* Looking to break left */
                if (find_breakleft)
                {
                    return (TRUE);
                }
            }

            /* Obstacle */
            else
            {
                /* Looking to break right */
                if (find_breakright)
                {
                    return (TRUE);
                }
            }
        }
    }


    /* Not looking for open area */
    else
    {
        /* No options */
        if (!option)
        {
            return (TRUE);
        }

        /* One option */
        else if (!option2)
        {
            /* Primary option */
            find_current = option;

            /* No other options */
            find_prevdir = option;
        }

        /* Two options, examining corners */
        else if (find_examine && !find_cut)
        {
            /* Primary option */
            find_current = option;

            /* Hack -- allow curving */
            find_prevdir = option2;
        }

        /* Two options, pick one */
        else
        {
            /* Get next location */
            row = py + ddy[option];
            col = px + ddx[option];

            /* Don't see that it is closed off. */
            /* This could be a potential corner or an intersection. */
            if (!see_wall(option, row, col) ||
                !see_wall(check_dir, row, col))
            {
                /* Can not see anything ahead and in the direction we */
                /* are turning, assume that it is a potential corner. */
                if (find_examine &&
                    see_nothing(option, row, col) &&
                    see_nothing(option2, row, col))
                {
                    find_current = option;
                    find_prevdir = option2;
                }

                /* STOP: we are next to an intersection or a room */
                else
                {
                    return (TRUE);
                }
            }

            /* This corner is seen to be enclosed; we cut the corner. */
            else if (find_cut)
            {
                find_current = option2;
                find_prevdir = option2;
            }

            /* This corner is seen to be enclosed, and we */
            /* deliberately go the long way. */
            else
            {
                find_current = option;
                find_prevdir = option2;
            }
        }
    }


    /* About to hit a known wall, stop */
    if (see_wall(find_current, py, px))
    {
        return (TRUE);
    }


    /* Failure */
    return (FALSE);
}



/*
 * Take one step along the current "run" path
 */
void run_step(int dir)
{
    /* Start running */
    if (dir)
    {
        /* Hack -- do not start silly run */
        if (see_wall(dir, py, px))
        {
            /* Message */
            msg_print("You cannot run in that direction.");

            /* Disturb */
            disturb(0,0);

            /* Done */
            return;
        }

        /* Calculate torch radius */
        p_ptr->update |= (PU_TORCH);

        /* Initialize */
        run_init(dir);
    }

    /* Keep running */
    else
    {
        /* Update run */
        if (run_test())
        {
            /* Disturb */
            disturb(0,0);

            /* Done */
            return;
        }
    }

    /* Decrease the run counter */
    if (--running <= 0) return;

    /* Take time */
    energy_use = 100;

    /* Move the player, using the "pickup" flag */
    move_player(find_current, always_pickup);
}


