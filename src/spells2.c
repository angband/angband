/* File: spells2.c */

/* Purpose: Spell code (part 2) */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"





/*
 * Increase players hit points, notice effects
 */
bool hp_player(int num)
{
    if (p_ptr->chp < p_ptr->mhp)
    {
        p_ptr->chp += num;

        if (p_ptr->chp > p_ptr->mhp)
        {
            p_ptr->chp = p_ptr->mhp;
            p_ptr->chp_frac = 0;
        }

        p_ptr->redraw |= (PR_HP);

        num = num / 5;
        if (num < 3)
        {
            if (num == 0)
            {
                msg_print("You feel a little better.");
            }
            else
            {
                msg_print("You feel better.");
            }
        }
        else
        {
            if (num < 7)
            {
                msg_print("You feel much better.");
            }
            else
            {
                msg_print("You feel very good.");
            }
        }

        return (TRUE);
    }

    return (FALSE);
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
    c_ptr->ftyp = 0x03;
}




/*
 * Array of stat "descriptions"
 */
static cptr desc_stat_pos[] =
{
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
static cptr desc_stat_neg[] =
{
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
    switch (stat)
    {
        case A_STR: if (p_ptr->sustain_str) sust = TRUE; break;
        case A_INT: if (p_ptr->sustain_int) sust = TRUE; break;
        case A_WIS: if (p_ptr->sustain_wis) sust = TRUE; break;
        case A_DEX: if (p_ptr->sustain_dex) sust = TRUE; break;
        case A_CON: if (p_ptr->sustain_con) sust = TRUE; break;
        case A_CHR: if (p_ptr->sustain_chr) sust = TRUE; break;
    }

    /* Sustain */
    if (sust)
    {
        /* Message */
        msg_format("You feel %s for a moment, but the feeling passes.",
                   desc_stat_neg[stat]);

        /* Notice effect */
        return (TRUE);
    }

    /* Attempt to reduce the stat */
    if (dec_stat(stat, 10, FALSE))
    {
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
    if (res_stat(stat))
    {
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
    if (inc_stat(stat))
    {
        /* Message */
        msg_format("Wow!  You feel very %s!", desc_stat_pos[stat]);

        /* Notice */
        return (TRUE);
    }

    /* Restoration worked */
    if (res)
    {
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
    object_type        *i_ptr;

    /* Simply identify and know every item */
    for (i = 0; i < INVEN_TOTAL; i++)
    {
        i_ptr = &inventory[i];
        if (i_ptr->k_idx)
        {
            object_aware(i_ptr);
            object_known(i_ptr);
        }
    }
}






/*
 * Used by the "enchant" function (chance of failure)
 */
static int enchant_table[16] =
{
    0, 10,  50, 100, 200,
    300, 400, 500, 700, 950,
    990, 992, 995, 997, 999,
    1000
};


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
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
    {
        u32b f1, f2, f3;

        object_type *i_ptr = &inventory[i];

        /* Uncursed already */
        if (!cursed_p(i_ptr)) continue;

        /* Extract the flags */
        object_flags(i_ptr, &f1, &f2, &f3);

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
    if (p_ptr->exp < p_ptr->max_exp)
    {
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
 *
 * XXX XXX XXX Use the "show_file()" method, perhaps.
 */
void self_knowledge()
{
    int		i = 0, j, k;

    u32b	f1 = 0L, f2 = 0L, f3 = 0L;

    object_type	*i_ptr;

    cptr	info[128];


    /* Acquire item flags from equipment */
    for (k = INVEN_WIELD; k < INVEN_TOTAL; k++)
    {
        u32b t1, t2, t3;

        i_ptr = &inventory[k];

        /* Skip empty items */
        if (!i_ptr->k_idx) continue;

        /* Extract the flags */
        object_flags(i_ptr, &t1, &t2, &t3);

        /* Extract flags */
        f1 |= t1;
        f2 |= t2;
        f3 |= t3;
    }


    if (p_ptr->blind)
    {
        info[i++] = "You cannot see.";
    }
    if (p_ptr->confused)
    {
        info[i++] = "You are confused.";
    }
    if (p_ptr->afraid)
    {
        info[i++] = "You are terrified.";
    }
    if (p_ptr->cut)
    {
        info[i++] = "You are bleeding.";
    }
    if (p_ptr->stun)
    {
        info[i++] = "You are stunned.";
    }
    if (p_ptr->poisoned)
    {
        info[i++] = "You are poisoned.";
    }
    if (p_ptr->image)
    {
        info[i++] = "You are hallucinating.";
    }

    if (p_ptr->aggravate)
    {
        info[i++] = "You aggravate monsters.";
    }
    if (p_ptr->teleport)
    {
        info[i++] = "Your position is very uncertain.";
    }

    if (p_ptr->blessed)
    {
        info[i++] = "You feel rightous.";
    }
    if (p_ptr->hero)
    {
        info[i++] = "You feel heroic.";
    }
    if (p_ptr->shero)
    {
        info[i++] = "You are in a battle rage.";
    }
    if (p_ptr->protevil)
    {
        info[i++] = "You are protected from evil.";
    }
    if (p_ptr->shield)
    {
        info[i++] = "You are protected by a mystic shield.";
    }
    if (p_ptr->invuln)
    {
        info[i++] = "You are temporarily invulnerable.";
    }
    if (p_ptr->confusing)
    {
        info[i++] = "Your hands are glowing dull red.";
    }
    if (p_ptr->searching)
    {
        info[i++] = "You are looking around very carefully.";
    }
    if (p_ptr->new_spells)
    {
        info[i++] = "You can learn some more spells.";
    }
    if (p_ptr->word_recall)
    {
        info[i++] = "You will soon be recalled.";
    }
    if (p_ptr->see_infra)
    {
        info[i++] = "Your eyes are sensitive to infrared light.";
    }
    if (p_ptr->see_inv)
    {
        info[i++] = "You can see invisible creatures.";
    }
    if (p_ptr->ffall)
    {
        info[i++] = "You land gently.";
    }
    if (p_ptr->free_act)
    {
        info[i++] = "You have free action.";
    }
    if (p_ptr->regenerate)
    {
        info[i++] = "You regenerate quickly.";
    }
    if (p_ptr->slow_digest)
    {
        info[i++] = "Your appetite is small.";
    }
    if (p_ptr->telepathy)
    {
        info[i++] = "You have ESP.";
    }
    if (p_ptr->hold_life)
    {
        info[i++] = "You have a firm hold on your life force.";
    }
    if (p_ptr->lite)
    {
        info[i++] = "You are carrying a permanent light.";
    }

    if (p_ptr->immune_acid)
    {
        info[i++] = "You are completely immune to acid.";
    }
    else if ((p_ptr->resist_acid) && (p_ptr->oppose_acid))
    {
        info[i++] = "You resist acid exceptionally well.";
    }
    else if ((p_ptr->resist_acid) || (p_ptr->oppose_acid))
    {
        info[i++] = "You are resistant to acid.";
    }

    if (p_ptr->immune_elec)
    {
        info[i++] = "You are completely immune to lightning.";
    }
    else if ((p_ptr->resist_elec) && (p_ptr->oppose_elec))
    {
        info[i++] = "You resist lightning exceptionally well.";
    }
    else if ((p_ptr->resist_elec) || (p_ptr->oppose_elec))
    {
        info[i++] = "You are resistant to lightning.";
    }

    if (p_ptr->immune_fire)
    {
        info[i++] = "You are completely immune to fire.";
    }
    else if ((p_ptr->resist_fire) && (p_ptr->oppose_fire))
    {
        info[i++] = "You resist fire exceptionally well.";
    }
    else if ((p_ptr->resist_fire) || (p_ptr->oppose_fire))
    {
        info[i++] = "You are resistant to fire.";
    }

    if (p_ptr->immune_cold)
    {
        info[i++] = "You are completely immune to cold.";
    }
    else if ((p_ptr->resist_cold) && (p_ptr->oppose_cold))
    {
        info[i++] = "You resist cold exceptionally well.";
    }
    else if ((p_ptr->resist_cold) || (p_ptr->oppose_cold))
    {
        info[i++] = "You are resistant to cold.";
    }

    if ((p_ptr->resist_pois) && (p_ptr->oppose_pois))
    {
        info[i++] = "You resist poison exceptionally well.";
    }
    else if ((p_ptr->resist_pois) || (p_ptr->oppose_pois))
    {
        info[i++] = "You are resistant to poison.";
    }

    if (p_ptr->resist_lite)
    {
        info[i++] = "You are resistant to bright light.";
    }
    if (p_ptr->resist_dark)
    {
        info[i++] = "You are resistant to darkness.";
    }
    if (p_ptr->resist_conf)
    {
        info[i++] = "You are resistant to confusion.";
    }
    if (p_ptr->resist_sound)
    {
        info[i++] = "You are resistant to sonic attacks.";
    }
    if (p_ptr->resist_disen)
    {
        info[i++] = "You are resistant to disenchantment.";
    }
    if (p_ptr->resist_chaos)
    {
        info[i++] = "You are resistant to chaos.";
    }
    if (p_ptr->resist_shard)
    {
        info[i++] = "You are resistant to blasts of shards.";
    }
    if (p_ptr->resist_nexus)
    {
        info[i++] = "You are resistant to nexus attacks.";
    }
    if (p_ptr->resist_neth)
    {
        info[i++] = "You are resistant to nether forces.";
    }
    if (p_ptr->resist_fear)
    {
        info[i++] = "You are completely fearless.";
    }
    if (p_ptr->resist_blind)
    {
        info[i++] = "Your eyes are resistant to blindness.";
    }

    if (p_ptr->sustain_str)
    {
        info[i++] = "Your strength is sustained.";
    }
    if (p_ptr->sustain_int)
    {
        info[i++] = "Your intelligence is sustained.";
    }
    if (p_ptr->sustain_wis)
    {
        info[i++] = "Your wisdom is sustained.";
    }
    if (p_ptr->sustain_con)
    {
        info[i++] = "Your constitution is sustained.";
    }
    if (p_ptr->sustain_dex)
    {
        info[i++] = "Your dexterity is sustained.";
    }
    if (p_ptr->sustain_chr)
    {
        info[i++] = "Your charisma is sustained.";
    }

    if (f1 & TR1_STR)
    {
        info[i++] = "Your strength is affected by your equipment.";
    }
    if (f1 & TR1_INT)
    {
        info[i++] = "Your intelligence is affected by your equipment.";
    }
    if (f1 & TR1_WIS)
    {
        info[i++] = "Your wisdom is affected by your equipment.";
    }
    if (f1 & TR1_DEX)
    {
        info[i++] = "Your dexterity is affected by your equipment.";
    }
    if (f1 & TR1_CON)
    {
        info[i++] = "Your constitution is affected by your equipment.";
    }
    if (f1 & TR1_CHR)
    {
        info[i++] = "Your charisma is affected by your equipment.";
    }

    if (f1 & TR1_STEALTH)
    {
        info[i++] = "Your stealth is affected by your equipment.";
    }
    if (f1 & TR1_SEARCH)
    {
        info[i++] = "Your searching ability is affected by your equipment.";
    }
    if (f1 & TR1_INFRA)
    {
        info[i++] = "Your infravision is affected by your equipment.";
    }
    if (f1 & TR1_TUNNEL)
    {
        info[i++] = "Your digging ability is affected by your equipment.";
    }
    if (f1 & TR1_SPEED)
    {
        info[i++] = "Your speed is affected by your equipment.";
    }
    if (f1 & TR1_BLOWS)
    {
        info[i++] = "Your attack speed is affected by your equipment.";
    }


    /* Access the current weapon */
    i_ptr = &inventory[INVEN_WIELD];

    /* Analyze the weapon */
    if (i_ptr->k_idx)
    {
        /* Indicate Blessing */
        if (f3 & TR3_BLESSED)
        {
            info[i++] = "Your weapon has been blessed by the gods.";
        }

        /* Hack */
        if (f1 & TR1_IMPACT)
        {
            info[i++] = "The impact of your weapon can cause earthquakes.";
        }

        /* Special "Attack Bonuses" */
        if (f1 & TR1_BRAND_ACID)
        {
            info[i++] = "Your weapon melts your foes.";
        }
        if (f1 & TR1_BRAND_ELEC)
        {
            info[i++] = "Your weapon shocks your foes.";
        }
        if (f1 & TR1_BRAND_FIRE)
        {
            info[i++] = "Your weapon burns your foes.";
        }
        if (f1 & TR1_BRAND_COLD)
        {
            info[i++] = "Your weapon freezes your foes.";
        }

        /* Special "slay" flags */
        if (f1 & TR1_SLAY_ANIMAL)
        {
            info[i++] = "Your weapon strikes at animals with extra force.";
        }
        if (f1 & TR1_SLAY_EVIL)
        {
            info[i++] = "Your weapon strikes at evil with extra force.";
        }
        if (f1 & TR1_SLAY_UNDEAD)
        {
            info[i++] = "Your weapon strikes at undead with holy wrath.";
        }
        if (f1 & TR1_SLAY_DEMON)
        {
            info[i++] = "Your weapon strikes at demons with holy wrath.";
        }
        if (f1 & TR1_SLAY_ORC)
        {
            info[i++] = "Your weapon is especially deadly against orcs.";
        }
        if (f1 & TR1_SLAY_TROLL)
        {
            info[i++] = "Your weapon is especially deadly against trolls.";
        }
        if (f1 & TR1_SLAY_GIANT)
        {
            info[i++] = "Your weapon is especially deadly against giants.";
        }
        if (f1 & TR1_SLAY_DRAGON)
        {
            info[i++] = "Your weapon is especially deadly against dragons.";
        }

        /* Special "kill" flags */
        if (f1 & TR1_KILL_DRAGON)
        {
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
    for (k = 2, j = 0; j < i; j++)
    {
        /* Show the info */
        prt(info[j], k++, 15);

        /* Every 20 entries (lines 2 to 21), start over */
        if ((k == 22) && (j+1 < i))
        {
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
 * Forget everything
 */
bool lose_all_info(void)
{
    int                 i;

    /* Forget info about objects */
    for (i = 0; i < INVEN_TOTAL; i++)
    {
        object_type *i_ptr = &inventory[i];

        /* Skip non-items */
        if (!i_ptr->k_idx) continue;

        /* Allow "protection" by the MENTAL flag */
        if (i_ptr->ident & ID_MENTAL) continue;

        /* Remove "default inscriptions" */
        if (i_ptr->note && (i_ptr->ident & ID_SENSE))
        {
            /* Access the inscription */
            cptr q = quark_str(i_ptr->note);

            /* Hack -- Remove auto-inscriptions */
            if ((streq(q, "cursed")) ||
                (streq(q, "broken")) ||
                (streq(q, "good")) ||
                (streq(q, "average")) ||
                (streq(q, "excellent")) ||
                (streq(q, "worthless")) ||
                (streq(q, "special")) ||
                (streq(q, "terrible")))
            {
                /* Forget the inscription */
                i_ptr->note = 0;
            }
        }

        /* Hack -- Clear the "empty" flag */
        i_ptr->ident &= ~ID_EMPTY;

        /* Hack -- Clear the "known" flag */
        i_ptr->ident &= ~ID_KNOWN;

        /* Hack -- Clear the "felt" flag */
        i_ptr->ident &= ~ID_SENSE;
    }

    /* Redraw the choice window */
    p_ptr->redraw |= (PR_CHOOSE);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Combine / Reorder the pack (later) */
    p_ptr->notice |= (PN_COMBINE | PN_REORDER);

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

    object_type	*i_ptr;


    /* Scan the current panel */
    for (y = panel_row_min; y <= panel_row_max; y++)
    {
        for (x = panel_col_min; x <= panel_col_max; x++)
        {
            c_ptr = &cave[y][x];

            i_ptr = &i_list[c_ptr->i_idx];

            /* Magma/Quartz + Known Gold */
            if ((c_ptr->ftyp == 0x36) ||
                (c_ptr->ftyp == 0x37))
            {
                /* Notice detected gold */
                if (!(c_ptr->fdat & CAVE_MARK))
                {
                    /* Detect */
                    detect = TRUE;

                    /* Hack -- memorize the feature */
                    c_ptr->fdat |= CAVE_MARK;

                    /* Redraw */
                    lite_spot(y, x);
                }
            }

#if 0

            /* Notice embedded gold */
            if ((c_ptr->ftyp == 0x34) ||
                (c_ptr->ftyp == 0x35))
            {
                /* Expose the gold XXX XXX XXX */
                c_ptr->ftyp += 0x02;

                /* Detect */
                detect = TRUE;

                /* Hack -- memorize the item */
                c_ptr->fdat |= CAVE_MARK;

                /* Redraw */
                lite_spot(y, x);
            }
#endif

            /* Notice gold */
            if (i_ptr->tval == TV_GOLD)
            {
                /* Notice new items */
                if (!(i_ptr->marked))
                {
                    /* Detect */
                    detect = TRUE;

                    /* Hack -- memorize the item */
                    i_ptr->marked = TRUE;

                    /* Redraw */
                    lite_spot(y, x);
                }
            }
        }
    }

    return (detect);
}



/*
 * Detect magic items.
 *
 * This will light up all spaces with "magic" items, including artifacts,
 * ego-items, potions, scrolls, books, rods, wands, staves, amulets, rings,
 * and "enchanted" items of the "good" variety.
 *
 * It can probably be argued that this function is now too powerful.
 */
bool detect_magic()
{
    int		i, j, tv;
    bool	detect = FALSE;

    cave_type	*c_ptr;
    object_type	*i_ptr;


    /* Scan the current panel */
    for (i = panel_row_min; i <= panel_row_max; i++)
    {
        for (j = panel_col_min; j <= panel_col_max; j++)
        {
            /* Access the grid and object */
            c_ptr = &cave[i][j];
            i_ptr = &i_list[c_ptr->i_idx];

            /* Nothing there */
            if (!(c_ptr->i_idx)) continue;

            /* Examine the tval */
            tv = i_ptr->tval;

            /* Artifacts, misc magic items, or enchanted wearables */
            if (artifact_p(i_ptr) || ego_item_p(i_ptr) ||
                (tv == TV_AMULET) || (tv == TV_RING) ||
                (tv == TV_STAFF) || (tv == TV_WAND) || (tv == TV_ROD) ||
                (tv == TV_SCROLL) || (tv == TV_POTION) ||
                (tv == TV_MAGIC_BOOK) || (tv == TV_PRAYER_BOOK) ||
                ((i_ptr->to_a > 0) || (i_ptr->to_h + i_ptr->to_d > 0)))
            {
                /* Note new items */
                if (!(i_ptr->marked))
                {
                    /* Detect */
                    detect = TRUE;

                    /* Memorize the item */
                    i_ptr->marked = TRUE;

                    /* Redraw */
                    lite_spot(i, j);
                }
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
    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        int fy = m_ptr->fy;
        int fx = m_ptr->fx;

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Skip visible monsters */
        if (m_ptr->ml) continue;

        /* Detect all invisible monsters */
        if (panel_contains(fy, fx) && (r_ptr->flags2 & RF2_INVISIBLE))
        {
            /* Take note that they are invisible */
            r_ptr->r_flags2 |= RF2_INVISIBLE;

            /* Mega-Hack -- Show the monster */
            m_ptr->ml = TRUE;
            lite_spot(fy, fx);
            flag = TRUE;
        }
    }

    /* Describe result, and clean up */
    if (flag)
    {
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
    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        int fy = m_ptr->fy;
        int fx = m_ptr->fx;

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Skip visible monsters */
        if (m_ptr->ml) continue;

        /* Detect evil monsters */
        if (panel_contains(fy, fx) && (r_ptr->flags3 & RF3_EVIL))
        {
            /* Mega-Hack -- Show the monster */
            m_ptr->ml = TRUE;
            lite_spot(fy, fx);
            flag = TRUE;
        }
    }

    /* Note effects and clean up */
    if (flag)
    {
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
    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        int fy = m_ptr->fy;
        int fx = m_ptr->fx;

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Skip visible monsters */
        if (m_ptr->ml) continue;

        /* Detect all non-invisible monsters */
        if (panel_contains(fy, fx) && (!(r_ptr->flags2 & RF2_INVISIBLE)))
        {
            /* Mega-Hack -- Show the monster */
            m_ptr->ml = TRUE;
            lite_spot(fy, fx);
            flag = TRUE;
        }
    }

    /* Describe and clean up */
    if (flag)
    {
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
    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];

        int fy = m_ptr->fy;
        int fx = m_ptr->fx;

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Skip visible monsters */
        if (m_ptr->ml) continue;

        /* Detect all monsters */
        if (panel_contains(fy, fx))
        {
            /* Mega-Hack -- Show the monster */
            m_ptr->ml = TRUE;
            lite_spot(fy, fx);
            flag = detect = TRUE;
        }
    }

    /* Describe the result, then fix the monsters */
    if (flag)
    {
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

    cave_type	*c_ptr;

    object_type	*i_ptr;


    /* Scan the current panel */
    for (i = panel_row_min; i <= panel_row_max; i++)
    {
        for (j = panel_col_min; j <= panel_col_max; j++)
        {
            c_ptr = &cave[i][j];

            i_ptr = &i_list[c_ptr->i_idx];

            /* Nothing here */
            if (!(c_ptr->i_idx)) continue;

	    /* Do not detect "gold" */
            if (i_ptr->tval == TV_GOLD) continue;

            /* Note new objects */
            if (!(i_ptr->marked))
            {
                /* Detect */
                detect = TRUE;

                /* Hack -- memorize it */
                i_ptr->marked = TRUE;

                /* Redraw */
                lite_spot(i, j);
            }
        }
    }

    return (detect);
}


/*
 * Locates and displays traps on current panel
 */
bool detect_trap(void)
{
    int		i, j;

    bool	detect = FALSE;

    cave_type  *c_ptr;


    /* Scan the current panel */
    for (i = panel_row_min; i <= panel_row_max; i++)
    {
        for (j = panel_col_min; j <= panel_col_max; j++)
        {
            /* Access the grid */
            c_ptr = &cave[i][j];

            /* Detect invisible traps */
            if (c_ptr->ftyp == 0x02)
            {
                /* Pick a trap */
                pick_trap(i, j);

                /* Hack -- memorize it */
                c_ptr->fdat |= CAVE_MARK;

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
 * Locates and displays all stairs and secret doors on current panel -RAK-	
 */
bool detect_sdoor()
{
    int		i, j;
    bool	detect = FALSE;

    cave_type *c_ptr;


    /* Scan the panel */
    for (i = panel_row_min; i <= panel_row_max; i++)
    {
        for (j = panel_col_min; j <= panel_col_max; j++)
        {
            /* Access the grid and object */
            c_ptr = &cave[i][j];

            /* Hack -- detect secret doors */
            if (c_ptr->ftyp == 0x30)
            {
                /* Find the door XXX XXX XXX */
                c_ptr->ftyp = 0x20;

                /* Memorize the door */
                c_ptr->fdat |= CAVE_MARK;

                /* Redraw */
                lite_spot(i, j);

                /* Obvious */
                detect = TRUE;
            }

            /* Ignore known grids */
            if (c_ptr->fdat & CAVE_MARK) continue;

            /* Hack -- detect stairs */
            if ((c_ptr->ftyp == 0x06) ||
                (c_ptr->ftyp == 0x07))
            {
                /* Memorize the stairs */
                c_ptr->fdat |= CAVE_MARK;

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
    if (!valid_grid(py, px))
    {
        msg_print("The object resists the spell.");
        return;
    }

    /* Hack -- Delete old contents */
    delete_object(py, px);

    /* Create a staircase */
    if (!dun_level)
    {
        c_ptr->ftyp = 0x07;
    }
    else if (is_quest(dun_level) || (dun_level >= MAX_DEPTH-1))
    {
        c_ptr->ftyp = 0x06;
    }
    else if (rand_int(100) < 50)
    {
        c_ptr->ftyp = 0x06;
    }
    else
    {
        c_ptr->ftyp = 0x07;
    }

    /* Notice */
    note_spot(py, px);

    /* Redraw */
    lite_spot(py, px);
}




/*
 * Hook to specify "weapon"
 */
static bool item_tester_hook_weapon(object_type *i_ptr)
{
    switch (i_ptr->tval)
    {
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
static bool item_tester_hook_armour(object_type *i_ptr)
{
    switch (i_ptr->tval)
    {
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
bool enchant(object_type *i_ptr, int n, int eflag)
{
    int i, chance, prob;

    bool res = FALSE;

    bool a = artifact_p(i_ptr);

    u32b f1, f2, f3;

    /* Extract the flags */
    object_flags(i_ptr, &f1, &f2, &f3);


    /* Large piles resist enchantment */
    prob = i_ptr->number * 100;

    /* Missiles are easy to enchant */
    if ((i_ptr->tval == TV_BOLT) ||
        (i_ptr->tval == TV_ARROW) ||
        (i_ptr->tval == TV_SHOT))
    {
        prob = prob / 20;
    }

    /* Try "n" times */
    for (i=0; i<n; i++)
    {
        /* Hack -- Roll for pile resistance */
        if (rand_int(prob) >= 100) continue;

        /* Enchant to hit */
        if (eflag & ENCH_TOHIT)
        {
            if (i_ptr->to_h < 0) chance = 0;
            else if (i_ptr->to_h > 15) chance = 1000;
            else chance = enchant_table[i_ptr->to_h];

            if ((randint(1000) > chance) && (!a || (rand_int(100) < 50)))
            {
                i_ptr->to_h++;
                res = TRUE;

                /* only when you get it above -1 -CFT */
                if (cursed_p(i_ptr) &&
                    (!(f3 & TR3_PERMA_CURSE)) &&
                    (i_ptr->to_h >= 0) && (rand_int(100) < 25))
                {
                    msg_print("The curse is broken!");
                    i_ptr->ident &= ~ID_CURSED;
                    i_ptr->ident |= ID_SENSE;
                    i_ptr->note = quark_add("uncursed");
                }
            }
        }

        /* Enchant to damage */
        if (eflag & ENCH_TODAM)
        {
            if (i_ptr->to_d < 0) chance = 0;
            else if (i_ptr->to_d > 15) chance = 1000;
            else chance = enchant_table[i_ptr->to_d];

            if ((randint(1000) > chance) && (!a || (rand_int(100) < 50)))
            {
                i_ptr->to_d++;
                res = TRUE;

                /* only when you get it above -1 -CFT */
                if (cursed_p(i_ptr) &&
                    (!(f3 & TR3_PERMA_CURSE)) &&
                    (i_ptr->to_d >= 0) && (rand_int(100) < 25))
                {
                    msg_print("The curse is broken!");
                    i_ptr->ident &= ~ID_CURSED;
                    i_ptr->ident |= ID_SENSE;
                    i_ptr->note = quark_add("uncursed");
                }
            }
        }

        /* Enchant to armor class */
        if (eflag & ENCH_TOAC)
        {
            if (i_ptr->to_a < 0) chance = 0;
            else if (i_ptr->to_a > 15) chance = 1000;
            else chance = enchant_table[i_ptr->to_a];

            if ((randint(1000) > chance) && (!a || (rand_int(100) < 50)))
            {
                i_ptr->to_a++;
                res = TRUE;

                /* only when you get it above -1 -CFT */
                if (cursed_p(i_ptr) &&
                    (!(f3 & TR3_PERMA_CURSE)) &&
                    (i_ptr->to_a >= 0) && (rand_int(100) < 25))
                {
                    msg_print("The curse is broken!");
                    i_ptr->ident &= ~ID_CURSED;
                    i_ptr->ident |= ID_SENSE;
                    i_ptr->note = quark_add("uncursed");
                }
            }
        }
    }

    /* Failure */
    if (!res) return (FALSE);

    /* Redraw the choice window */
    p_ptr->redraw |= (PR_CHOOSE);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Combine / Reorder the pack (later) */
    p_ptr->notice |= (PN_COMBINE | PN_REORDER);

    /* Success */
    return (TRUE);
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

    object_type		*i_ptr;

    char		i_name[80];


    /* Assume enchant weapon */
    item_tester_hook = item_tester_hook_weapon;

    /* Enchant armor if requested */
    if (num_ac) item_tester_hook = item_tester_hook_armour;

    /* Get an item (from equip or inven or floor) */
    if (!get_item(&item, "Enchant which item? ", TRUE, TRUE, TRUE))
    {
        if (item == -2) msg_print("You have nothing to enchant.");
        return (FALSE);
    }

    /* Get the item (in the pack) */
    if (item >= 0)
    {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else
    {
        i_ptr = &i_list[0 - item];
    }


    /* Description */
    object_desc(i_name, i_ptr, FALSE, 0);

    /* Describe */
    msg_format("%s %s glow%s brightly!",
               ((item >= 0) ? "Your" : "The"), i_name,
               ((i_ptr->number > 1) ? "" : "s"));

    /* Enchant */
    if (enchant(i_ptr, num_hit, ENCH_TOHIT)) okay = TRUE;
    if (enchant(i_ptr, num_dam, ENCH_TODAM)) okay = TRUE;
    if (enchant(i_ptr, num_ac, ENCH_TOAC)) okay = TRUE;

    /* Failure */
    if (!okay)
    {
        /* Flush */
        if (flush_failure) flush();

        /* Message */
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

    object_type		*i_ptr;

    char		i_name[80];


    /* Get an item (from equip or inven or floor) */
    if (!get_item(&item, "Identify which item? ", TRUE, TRUE, TRUE))
    {
        if (item == -2) msg_print("You have nothing to identify.");
        return (FALSE);
    }

    /* Get the item (in the pack) */
    if (item >= 0)
    {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else
    {
        i_ptr = &i_list[0 - item];
    }


    /* Identify it fully */
    object_aware(i_ptr);
    object_known(i_ptr);

    /* Redraw the choice window */
    p_ptr->redraw |= (PR_CHOOSE);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);	

    /* Combine / Reorder the pack (later) */
    p_ptr->notice |= (PN_COMBINE | PN_REORDER);

    /* Description */
    object_desc(i_name, i_ptr, TRUE, 3);

    /* Describe */
    if (item >= INVEN_WIELD)
    {
        msg_format("%^s: %s (%c).",
                   describe_use(item), i_name, index_to_label(item));
    }
    else if (item >= 0)
    {
        msg_format("In your pack: %s (%c).",
                   i_name, index_to_label(item));
    }
    else
    {
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

    object_type		*i_ptr;

    char		i_name[80];


    /* Get an item (from equip or inven or floor) */
    if (!get_item(&item, "Identify which item? ", TRUE, TRUE, TRUE))
    {
        if (item == -2) msg_print("You have nothing to identify.");
        return (FALSE);
    }

    /* Get the item (in the pack) */
    if (item >= 0)
    {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else
    {
        i_ptr = &i_list[0 - item];
    }


    /* Identify it fully */
    object_aware(i_ptr);
    object_known(i_ptr);

    /* Mark the item as fully known */
    i_ptr->ident |= (ID_MENTAL);

    /* Redraw choice window */
    p_ptr->redraw |= (PR_CHOOSE);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Combine / Reorder the pack (later) */
    p_ptr->notice |= (PN_COMBINE | PN_REORDER);

    /* Handle stuff */
    handle_stuff();

    /* Description */
    object_desc(i_name, i_ptr, TRUE, 3);

    /* Describe */
    if (item >= INVEN_WIELD)
    {
        msg_format("%^s: %s (%c).",
                   describe_use(item), i_name, index_to_label(item));
    }
    else if (item >= 0)
    {
        msg_format("In your pack: %s (%c).",
                   i_name, index_to_label(item));
    }
    else
    {
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
static bool item_tester_hook_recharge(object_type *i_ptr)
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
 *
 * XXX XXX XXX Perhaps we should auto-unstack recharging stacks.
 */
bool recharge(int num)
{
    int                 i, t, item, lev;

    object_type		*i_ptr;


    /* Only accept legal items */
    item_tester_hook = item_tester_hook_recharge;

    /* Get an item (from inven or floor) */
    if (!get_item(&item, "Recharge which item? ", FALSE, TRUE, TRUE))
    {
        if (item == -2) msg_print("You have nothing to recharge.");
        return (FALSE);
    }

    /* Get the item (in the pack) */
    if (item >= 0)
    {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else
    {
        i_ptr = &i_list[0 - item];
    }


    /* Extract the object "level" */
    lev = k_info[i_ptr->k_idx].level;

    /* Recharge a rod */
    if (i_ptr->tval == TV_ROD)
    {

        /* Extract a recharge power */
        i = (100 - lev + num) / 5;

        /* Paranoia -- prevent crashes */
        if (i < 1) i = 1;

        /* Back-fire */
        if (rand_int(i) == 0)
        {
            /* Hack -- backfire */
            msg_print("The recharge backfires, draining the rod further!");

            /* Hack -- decharge the rod */
            if (i_ptr->pval < 10000) i_ptr->pval = (i_ptr->pval + 100) * 2;
        }

        /* Recharge */
        else
        {
            /* Rechange amount */
            t = (num * damroll(2, 4));

            /* Recharge by that amount */
            if (i_ptr->pval > t)
            {
                i_ptr->pval -= t;
            }

            /* Fully recharged */
            else
            {
                i_ptr->pval = 0;
            }
        }
    }

    /* Recharge wand/staff */
    else
    {
        /* Recharge power */
        i = (num + 100 - lev - (10 * i_ptr->pval)) / 15;

        /* Paranoia -- prevent crashes */
        if (i < 1) i = 1;

        /* Back-fire XXX XXX XXX */
        if (rand_int(i) == 0)
        {
            /* Dangerous Hack -- Destroy the item */
            msg_print("There is a bright flash of light.");

            /* Reduce and describe inventory */
            if (item >= 0)
            {
                inven_item_increase(item, -999);
                inven_item_describe(item);
                inven_item_optimize(item);
            }

            /* Reduce and describe floor item */
            else
            {
                floor_item_increase(0 - item, -999);
                floor_item_describe(0 - item);
                floor_item_optimize(0 - item);
            }
        }

        /* Recharge */
        else
        {

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

    /* Combine / Reorder the pack (later) */
    p_ptr->notice |= (PN_COMBINE | PN_REORDER);

    /* Something was done */
    return (TRUE);
}








/*
 * Apply a "project()" directly to all viewable monsters
 */
static bool project_hack(int typ, int dam)
{
    int		i, x, y;

    int		flg = PROJECT_JUMP | PROJECT_KILL | PROJECT_HIDE;

    bool	obvious = FALSE;


    /* Affect all (nearby) monsters */
    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Location */
        y = m_ptr->fy;
        x = m_ptr->fx;

        /* Require line of sight */
        if (!player_has_los_bold(y, x)) continue;

        /* Jump directly to the target monster */
        if (project(0, 0, y, x, dam, typ, flg)) obvious = TRUE;
    }

    /* Result */
    return (obvious);
}


/*
 * Speed monsters
 */
bool speed_monsters(void)
{
    return (project_hack(GF_OLD_SPEED, p_ptr->lev));
}

/*
 * Slow monsters
 */
bool slow_monsters(void)
{
    return (project_hack(GF_OLD_SLOW, p_ptr->lev));
}

/*
 * Sleep monsters
 */
bool sleep_monsters(void)
{
    return (project_hack(GF_OLD_SLEEP, p_ptr->lev));
}


/*
 * Banish evil monsters
 */
bool banish_evil(int dist)
{
    return (project_hack(GF_AWAY_EVIL, dist));
}


/*
 * Turn undead
 */
bool turn_undead(void)
{
    return (project_hack(GF_TURN_UNDEAD, p_ptr->lev));
}


/*
 * Dispel undead monsters
 */
bool dispel_undead(int dam)
{
    return (project_hack(GF_DISP_UNDEAD, dam));
}

/*
 * Dispel evil monsters
 */
bool dispel_evil(int dam)
{
    return (project_hack(GF_DISP_EVIL, dam));
}

/*
 * Dispel all monsters
 */
bool dispel_monsters(int dam)
{
    return (project_hack(GF_DISP_ALL, dam));
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
    for (i = 1; i < m_max; i++)
    {
        monster_type	*m_ptr = &m_list[i];
        monster_race	*r_ptr = &r_info[m_ptr->r_idx];

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Skip aggravating monster (or player) */
        if (i == who) continue;

        /* Wake up nearby sleeping monsters */
        if (m_ptr->cdis < MAX_SIGHT * 2)
        {
            /* Wake up */
            if (m_ptr->csleep)
            {
                /* Wake up */
                m_ptr->csleep = 0;
                sleep = TRUE;
            }
        }

        /* Speed up monsters in line of sight */	
        if (player_has_los_bold(m_ptr->fy, m_ptr->fx))
        {
            /* Speed up (instantly) to racial base + 10 */
            if (m_ptr->mspeed < r_ptr->speed + 10)
            {
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
    for (i = 1; i < m_max; i++)
    {
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
    for (i = 1; i < m_max; i++)
    {
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
 * Probe nearby monsters
 */
bool probing(void)
{
    int            i;

    bool	probe = FALSE;


    /* Probe all (nearby) monsters */
    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Require line of sight */
        if (!player_has_los_bold(m_ptr->fy, m_ptr->fx)) continue;

        /* Probe visible monsters */
        if (m_ptr->ml)
        {
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
    if (probe)
    {
        msg_print("That's all.");
    }

    /* Result */
    return (probe);
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


    /* XXX XXX */
    full = full ? full : 0;

    /* Big area of affect */
    for (y = (y1 - r); y <= (y1 + r); y++)
    {
        for (x = (x1 - r); x <= (x1 + r); x++)
        {
            /* Skip illegal grids */
            if (!in_bounds(y, x)) continue;

            /* Extract the distance */
            k = distance(y1, x1, y, x);

            /* Stay in the circle of death */
            if (k > r) continue;

            /* Access the grid */
            c_ptr = &cave[y][x];

            /* Lose room and vault */
            c_ptr->fdat &= ~(CAVE_ROOM | CAVE_ICKY);

            /* Lose light and knowledge */
            c_ptr->fdat &= ~(CAVE_MARK | CAVE_GLOW);

            /* Hack -- Notice player affect */
            if ((x == px) && (y == py))
            {
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
            if (valid_grid(y, x))
            {

                /* Delete the object (if any) */
                delete_object(y, x);

                /* Wall (or floor) type */
                t = rand_int(200);

                /* Granite */
                if (t < 20)
                {
                    /* Clear previous contents, add granite wall */
                    c_ptr->ftyp = 0x38;
                }

                /* Quartz */
                else if (t < 70)
                {
                    /* Clear previous contents, add quartz vein */
                    c_ptr->ftyp = 0x33;
                }

                /* Magma */
                else if (t < 100)
                {
                    /* Clear previous contents, add magma vein */
                    c_ptr->ftyp = 0x32;
                }

                /* Floor */
                else
                {
                    /* Clear previous contents, add floor */
                    c_ptr->ftyp = 0x01;
                }
            }
        }
    }


    /* Hack -- Affect player */
    if (flag)
    {
        /* Message */
        msg_print("There is a searing blast of light!");

        /* Blind the player */
        if (!p_ptr->resist_blind && !p_ptr->resist_lite)
        {
            /* Become blind */
            (void)set_blind(p_ptr->blind + 10 + randint(10));
        }
    }


    /* Mega-Hack -- Forget the view and lite */
    p_ptr->update |= (PU_UN_VIEW | PU_UN_LITE);

    /* Update stuff */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);

    /* Update the monsters */
    p_ptr->update |= (PU_MONSTERS);

    /* Redraw map */
    p_ptr->redraw |= (PR_MAP | PR_AROUND);
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
    for (y = 0; y < 32; y++)
    {
        for (x = 0; x < 32; x++)
        {
            map[y][x] = FALSE;
        }
    }

    /* Check around the epicenter */
    for (dy = -r; dy <= r; dy++)
    {
        for (dx = -r; dx <= r; dx++)
        {
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
            c_ptr->fdat &= ~(CAVE_ROOM | CAVE_ICKY);

            /* Lose light and knowledge */
            c_ptr->fdat &= ~(CAVE_GLOW | CAVE_MARK);

            /* Skip the epicenter */
            if (!dx && !dy) continue;

            /* Skip most grids */
            if (rand_int(100) < 85) continue;

            /* Damage this grid */
            map[16+yy-cy][16+xx-cx] = TRUE;

            /* Hack -- Take note of player damage */
            if ((yy == py) && (xx == px)) hurt = TRUE;
        }
    }

    /* First, affect the player (if necessary) */
    if (hurt)
    {
        /* Check around the player */
        for (i = 0; i < 8; i++)
        {
            /* Access the location */
            y = py + ddy[i];
            x = px + ddx[i];

            /* Skip non-empty grids */
            if (!empty_grid_bold(y, x)) continue;

            /* Important -- Skip "quake" grids */
            if (map[16+y-cy][16+x-cx]) continue;

            /* Count "safe" grids */
            sn++;

            /* Randomize choice */
            if (rand_int(sn) > 0) continue;

            /* Save the safe location */
            sy = y; sx = x;
        }

        /* Random message */
        switch (randint(3))
        {
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
        if (!sn)
        {
            /* Message and damage */
            msg_print("You are severely crushed!");
            damage = 300;
        }

        /* Destroy the grid, and push the player to safety */
        else
        {
            /* Calculate results */
            switch (randint(3))
            {
                case 1:
                    msg_print("You nimbly dodge the blast!");
                    damage = 0;
                    break;
                case 2:
                    msg_print("You are bashed by rubble!");
                    damage = damroll(10, 4);
                    (void)set_stun(p_ptr->stun + randint(50));
                    break;
                case 3:
                    msg_print("You are crushed between the floor and ceiling!");
                    damage = damroll(10, 4);
                    (void)set_stun(p_ptr->stun + randint(50));
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

        /* Important -- no wall on player */
        map[16+py-cy][16+px-cx] = FALSE;

        /* Take some damage */
        if (damage) take_hit(damage, "an earthquake");
    }


    /* Examine the quaked region */
    for (dy = -r; dy <= r; dy++)
    {
        for (dx = -r; dx <= r; dx++)
        {
            /* Extract the location */
            yy = cy + dy;
            xx = cx + dx;

            /* Skip unaffected grids */
            if (!map[16+yy-cy][16+xx-cx]) continue;

            /* Access the grid */
            c_ptr = &cave[yy][xx];

            /* Process monsters */
            if (c_ptr->m_idx)
            {
                monster_type *m_ptr = &m_list[c_ptr->m_idx];
                monster_race *r_ptr = &r_info[m_ptr->r_idx];

                /* Most monsters cannot co-exist with rock */
                if (!(r_ptr->flags2 & RF2_KILL_WALL) &&
                    !(r_ptr->flags2 & RF2_PASS_WALL))
                {
                    char m_name[80];

                    /* Assume not safe */
                    sn = 0;

                    /* Monster can move to escape the wall */
                    if (!(r_ptr->flags1 & RF1_NEVER_MOVE))
                    {
                        /* Look for safety */
                        for (i = 0; i < 8; i++)
                        {
                            /* Access the grid */
                            y = yy + ddy[i];
                            x = xx + ddx[i];

                            /* Skip non-empty grids */
                            if (!empty_grid_bold(y, x)) continue;

                            /* Hack -- no safety on glyph of warding */
                            if (cave[y][x].ftyp == 0x03) continue;

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
                    if (m_ptr->hp < 0)
                    {
                        /* Message */
                        msg_format("%^s is embedded in the rock!", m_name);

                        /* Delete the monster */
                        delete_monster(yy, xx);

                        /* No longer safe */
                        sn = 0;
                    }

                    /* Hack -- Escape from the rock */
                    if (sn)
                    {
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
    for (dy = -r; dy <= r; dy++)
    {
        for (dx = -r; dx <= r; dx++)
        {
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
            if (valid_grid(yy, xx))
            {
                bool floor = floor_grid_bold(yy, xx);

                /* Delete any object that is still there */
                delete_object(yy, xx);

                /* Wall (or floor) type */
                t = (floor ? rand_int(100) : 200);

                /* Granite */
                if (t < 20)
                {
                    /* Clear previous contents, add granite wall */
                    c_ptr->ftyp = 0x38;
                }

                /* Quartz */
                else if (t < 70)
                {
                    /* Clear previous contents, add quartz vein */
                    c_ptr->ftyp = 0x33;
                }

                /* Magma */
                else if (t < 100)
                {
                    /* Clear previous contents, add magma vein */
                    c_ptr->ftyp = 0x32;
                }

                /* Floor */
                else
                {
                    /* Clear previous contents, add floor */
                    c_ptr->ftyp = 0x01;
                }
            }
        }
    }


    /* Mega-Hack -- Forget the view and lite */
    p_ptr->update |= (PU_UN_VIEW | PU_UN_LITE);

    /* Update stuff */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);

    /* Update the monsters */
    p_ptr->update |= (PU_DISTANCE);

    /* Update the health bar */
    p_ptr->redraw |= (PR_HEALTH);

    /* Redraw map */
    p_ptr->redraw |= (PR_MAP | PR_AROUND);
}



/*
 * This routine clears the entire "temp" set.
 *
 * This routine will Perma-Lite all "temp" grids.
 *
 * This routine is used (only) by "lite_room()"
 *
 * Dark grids are illuminated.
 *
 * Also, process all affected monsters.
 *
 * SMART monsters always wake up when illuminated
 * NORMAL monsters wake up 1/4 the time when illuminated
 * STUPID monsters wake up 1/10 the time when illuminated
 */
static void cave_temp_room_lite(void)
{
    int i;

    /* Clear them all */
    for (i = 0; i < temp_n; i++)
    {
        int y = temp_y[i];
        int x = temp_x[i];

        cave_type *c_ptr = &cave[y][x];

        /* No longer in the array */
        c_ptr->fdat &= ~CAVE_TEMP;

        /* Update only non-CAVE_GLOW grids */
        /* if (c_ptr->fdat & CAVE_GLOW) continue; */

        /* Perma-Lite */
        c_ptr->fdat |= CAVE_GLOW;

        /* Process affected monsters */
        if (c_ptr->m_idx)
        {
            monster_type	*m_ptr = &m_list[c_ptr->m_idx];

            monster_race	*r_ptr = &r_info[m_ptr->r_idx];

            /* Update the monster */
            update_mon(c_ptr->m_idx, FALSE);

            /* Sometimes monsters wake up */
            if (m_ptr->csleep &&
                (((r_ptr->flags2 & RF2_STUPID) && (rand_int(100) < 10)) ||
                 (rand_int(100) < 25) ||
                 (r_ptr->flags2 & RF2_SMART)))
            {
                /* Wake up! */
                m_ptr->csleep = 0;

                /* Notice the "waking up" */
                if (m_ptr->ml)
                {
                    char m_name[80];

                    /* Acquire the monster name */
                    monster_desc(m_name, m_ptr, 0);

                    /* Dump a message */
                    msg_format("%^s wakes up.", m_name);
                }
            }
        }

        /* Note */
        note_spot(y, x);

        /* Redraw */
        lite_spot(y, x);
    }

    /* None left */
    temp_n = 0;
}



/*
 * This routine clears the entire "temp" set.
 *
 * This routine will "darken" all "temp" grids.
 *
 * In addition, some of these grids will be "unmarked".
 *
 * This routine is used (only) by "unlite_room()"
 *
 * Also, process all affected monsters
 */
static void cave_temp_room_unlite(void)
{
    int i;

    /* Clear them all */
    for (i = 0; i < temp_n; i++)
    {
        int y = temp_y[i];
        int x = temp_x[i];

        cave_type *c_ptr = &cave[y][x];

        /* No longer in the array */
        c_ptr->fdat &= ~CAVE_TEMP;

        /* Darken the grid */
        c_ptr->fdat &= ~CAVE_GLOW;

        /* Hack -- Forget "boring" grids */
        if (c_ptr->ftyp <= 0x02)
        {
            /* Forget the grid */
            c_ptr->fdat &= ~CAVE_MARK;

            /* Notice */
            note_spot(y, x);
        }

        /* Process affected monsters */
        if (c_ptr->m_idx)
        {
            /* Update the monster */
            update_mon(c_ptr->m_idx, FALSE);
        }

        /* Redraw */
        lite_spot(y, x);
    }

    /* None left */
    temp_n = 0;
}




/*
 * Aux function -- see below
 */
static void cave_temp_room_aux(int y, int x)
{
    cave_type *c_ptr = &cave[y][x];

    /* Avoid infinite recursion */
    if (c_ptr->fdat & CAVE_TEMP) return;

    /* Do not "leave" the current room */
    if (!(c_ptr->fdat & CAVE_ROOM)) return;

    /* Paranoia -- verify space */
    if (temp_n == TEMP_MAX) return;

    /* Mark the grid as "seen" */
    c_ptr->fdat |= CAVE_TEMP;

    /* Add it to the "seen" set */
    temp_y[temp_n] = y;
    temp_x[temp_n] = x;
    temp_n++;
}




/*
 * Illuminate any room containing the given location.
 */
void lite_room(int y1, int x1)
{
    int i, x, y;

    /* Add the initial grid */
    cave_temp_room_aux(y1, x1);

    /* While grids are in the queue, add their neighbors */
    for (i = 0; i < temp_n; i++)
    {
        x = temp_x[i], y = temp_y[i];

        /* Walls get lit, but stop light */
        if (!floor_grid_bold(y, x)) continue;

        /* Spread adjacent */
        cave_temp_room_aux(y + 1, x);
        cave_temp_room_aux(y - 1, x);
        cave_temp_room_aux(y, x + 1);
        cave_temp_room_aux(y, x - 1);

        /* Spread diagonal */
        cave_temp_room_aux(y + 1, x + 1);
        cave_temp_room_aux(y - 1, x - 1);
        cave_temp_room_aux(y - 1, x + 1);
        cave_temp_room_aux(y + 1, x - 1);
    }

    /* Now, lite them all up at once */
    cave_temp_room_lite();
}


/*
 * Darken all rooms containing the given location
 */
void unlite_room(int y1, int x1)
{
    int i, x, y;

    /* Add the initial grid */
    cave_temp_room_aux(y1, x1);

    /* Spread, breadth first */
    for (i = 0; i < temp_n; i++)
    {
        x = temp_x[i], y = temp_y[i];

        /* Walls get dark, but stop darkness */
        if (!floor_grid_bold(y, x)) continue;

        /* Spread adjacent */
        cave_temp_room_aux(y + 1, x);
        cave_temp_room_aux(y - 1, x);
        cave_temp_room_aux(y, x + 1);
        cave_temp_room_aux(y, x - 1);

        /* Spread diagonal */
        cave_temp_room_aux(y + 1, x + 1);
        cave_temp_room_aux(y - 1, x - 1);
        cave_temp_room_aux(y - 1, x + 1);
        cave_temp_room_aux(y + 1, x - 1);
    }

    /* Now, darken them all at once */
    cave_temp_room_unlite();
}



/*
 * Hack -- call light around the player
 * Affect all monsters in the projection radius
 */
bool lite_area(int dam, int rad)
{
    int flg = PROJECT_GRID | PROJECT_KILL;

    /* Hack -- Message */
    if (!p_ptr->blind)
    {
        msg_print("You are surrounded by a white light.");
    }

    /* Hook into the "project()" function */
    (void)project(0, rad, py, px, dam, GF_LITE_WEAK, flg);

    /* Lite up the room */
    lite_room(py, px);

    /* Assume seen */
    return (TRUE);
}


/*
 * Hack -- call darkness around the player
 * Affect all monsters in the projection radius
 */
bool unlite_area(int dam, int rad)
{
    int flg = PROJECT_GRID | PROJECT_KILL;

    /* Hack -- Message */
    if (!p_ptr->blind)
    {
        msg_print("Darkness surrounds you.");
    }

    /* Hook into the "project()" function */
    (void)project(0, rad, py, px, dam, GF_DARK_WEAK, flg);

    /* Lite up the room */
    unlite_room(py, px);

    /* Assume seen */
    return (TRUE);
}



/*
 * Cast a ball spell
 * Stop if we hit a monster, act as a "ball"
 * Allow "target" mode to pass over monsters
 * Affect grids, objects, and monsters
 */
bool fire_ball(int typ, int dir, int dam, int rad)
{
    int tx, ty;

    int flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

    /* Use the given direction */
    tx = px + 99 * ddx[dir];
    ty = py + 99 * ddy[dir];

    /* Hack -- Use an actual "target" */
    if ((dir == 5) && target_okay())
    {
        flg &= ~PROJECT_STOP;
        tx = target_col;
        ty = target_row;
    }

    /* Analyze the "dir" and the "target".  Hurt items on floor. */
    return (project(0, rad, ty, tx, dam, typ, flg));
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

    /* Hack -- Use an actual "target" */
    if ((dir == 5) && target_okay())
    {
        tx = target_col;
        ty = target_row;
    }

    /* Analyze the "dir" and the "target", do NOT explode */
    return (project(0, 0, ty, tx, dam, typ, flg));
}


/*
 * Cast a bolt spell
 * Stop if we hit a monster, as a "bolt"
 * Affect monsters (not grids or objects)
 */
bool fire_bolt(int typ, int dir, int dam)
{
    int flg = PROJECT_STOP | PROJECT_KILL;
    return (project_hook(typ, dir, dam, flg));
}

/*
 * Cast a beam spell
 * Pass through monsters, as a "beam"
 * Affect monsters (not grids or objects)
 */
bool fire_beam(int typ, int dir, int dam)
{
    int flg = PROJECT_BEAM | PROJECT_KILL;
    return (project_hook(typ, dir, dam, flg));
}

/*
 * Cast a bolt spell, or rarely, a beam spell
 */
bool fire_bolt_or_beam(int prob, int typ, int dir, int dam)
{
    if (rand_int(100) < prob)
    {
        return (fire_beam(typ, dir, dam));
    }
    else
    {
        return (fire_bolt(typ, dir, dam));
    }
}


/*
 * Some of the old functions
 */

bool lite_line(int dir)
{
    int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_KILL;
    return (project_hook(GF_LITE_WEAK, dir, damroll(6, 8), flg));
}

bool drain_life(int dir, int dam)
{
    int flg = PROJECT_STOP | PROJECT_KILL;
    return (project_hook(GF_OLD_DRAIN, dir, dam, flg));
}

bool wall_to_mud(int dir)
{
    int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
    return (project_hook(GF_KILL_WALL, dir, 20 + randint(30), flg));
}

bool destroy_door(int dir)
{
    int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM;
    return (project_hook(GF_KILL_DOOR, dir, 0, flg));
}

bool disarm_trap(int dir)
{
    int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM;
    return (project_hook(GF_KILL_TRAP, dir, 0, flg));
}

bool heal_monster(int dir)
{
    int flg = PROJECT_STOP | PROJECT_KILL;
    return (project_hook(GF_OLD_HEAL, dir, damroll(4, 6), flg));
}

bool speed_monster(int dir)
{
    int flg = PROJECT_STOP | PROJECT_KILL;
    return (project_hook(GF_OLD_SPEED, dir, p_ptr->lev, flg));
}

bool slow_monster(int dir)
{
    int flg = PROJECT_STOP | PROJECT_KILL;
    return (project_hook(GF_OLD_SLOW, dir, p_ptr->lev, flg));
}

bool sleep_monster(int dir)
{
    int flg = PROJECT_STOP | PROJECT_KILL;
    return (project_hook(GF_OLD_SLEEP, dir, p_ptr->lev, flg));
}

bool confuse_monster(int dir, int plev)
{
    int flg = PROJECT_STOP | PROJECT_KILL;
    return (project_hook(GF_OLD_CONF, dir, plev, flg));
}

bool poly_monster(int dir)
{
    int flg = PROJECT_STOP | PROJECT_KILL;
    return (project_hook(GF_OLD_POLY, dir, p_ptr->lev, flg));
}

bool clone_monster(int dir)
{
    int flg = PROJECT_STOP | PROJECT_KILL;
    return (project_hook(GF_OLD_CLONE, dir, 0, flg));
}

bool fear_monster(int dir, int plev)
{
    int flg = PROJECT_STOP | PROJECT_KILL;
    return (project_hook(GF_TURN_ALL, dir, plev, flg));
}

bool teleport_monster(int dir)
{
    int flg = PROJECT_BEAM | PROJECT_KILL;
    return (project_hook(GF_AWAY_ALL, dir, MAX_SIGHT * 5, flg));
}



/*
 * Hooks -- affect adjacent grids (radius 1 ball attack)
 */

bool door_creation()
{
    int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
    return (project(0, 1, py, px, 0, GF_MAKE_DOOR, flg));
}

bool trap_creation()
{
    int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
    return (project(0, 1, py, px, 0, GF_MAKE_TRAP, flg));
}

bool destroy_doors_touch()
{
    int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
    return (project(0, 1, py, px, 0, GF_KILL_DOOR, flg));
}

bool sleep_monsters_touch(void)
{
    int flg = PROJECT_KILL | PROJECT_HIDE;
    return (project(0, 1, py, px, p_ptr->lev, GF_OLD_SLEEP, flg));
}


