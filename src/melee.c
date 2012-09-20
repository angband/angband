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
 * Critical blow.  All hits that do 95% of total possible damage,
 * and which also do at least 20 damage, or, sometimes, N damage.
 * This is used only to determine "cuts" and "stuns".
 */
static int monster_critical(int dice, int sides, int dam)
{
    int max = 0;
    int total = dice * sides;

    /* Must do at least 95% of perfect */
    if (dam < total * 19 / 20) return (0);

    /* Weak blows rarely work */
    if ((dam < 20) && (rand_int(100) >= dam)) return (0);

    /* Perfect damage */
    if (dam == total) max++;

    /* Super-charge */
    if (dam >= 20) {
        while (rand_int(100) < 2) max++;
    }

    /* Critical damage */
    if (dam > 45) return (6 + max);
    if (dam > 33) return (5 + max);
    if (dam > 25) return (4 + max);
    if (dam > 18) return (3 + max);
    if (dam > 11) return (2 + max);
    return (1 + max);
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

    monster_race *r_ptr = &r_info[m_ptr->r_idx];


    /* Not allowed to learn */
    if (!smart_learn) return;

    /* Too stupid to learn anything */
    if (r_ptr->flags2 & RF2_STUPID) return;

    /* Not intelligent, only learn sometimes */
    if (!(r_ptr->flags2 & RF2_SMART) && (rand_int(100) < 50)) return;


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
        if (!p_ptr->msp) m_ptr->smart |= SM_IMM_MANA;
        break;
    }
}






/*
 * Internal probablility routine
 */
static bool int_outof(monster_race *r_ptr, int prob)
{
    /* Non-Smart monsters are half as "smart" */
    if (!(r_ptr->flags2 & RF2_SMART)) prob = prob / 2;

    /* Roll the dice */
    return (rand_int(100) < prob);
}



/*
 * Remove the "bad" spells from a spell list
 */
static void remove_bad_spells(int m_idx, u32b *f4p, u32b *f5p, u32b *f6p)
{
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    u32b f4 = (*f4p);
    u32b f5 = (*f5p);
    u32b f6 = (*f6p);

    u32b smart = 0L;


    /* Too stupid to know anything */
    if (r_ptr->flags2 & RF2_STUPID) return;


    /* Must be cheating or learning */
    if (!smart_cheat && !smart_learn) return;


    /* Update acquired knowledge */
    if (smart_learn) {

        /* Hack -- Occasionally forget player status */
        if (m_ptr->smart && (rand_int(100) < 1)) m_ptr->smart = 0L;

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
        if (!p_ptr->msp) smart |= SM_IMM_MANA;
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


    /* XXX XXX XXX No spells left? */
    /* if (!f4 && !f5 && !f6) ... */


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
 * Determine if a monster attack against the player succeeds.
 * Always miss 5% of the time, Always hit 5% of the time.
 * Otherwise, match monster power against player armor.
 */
static int check_hit(int power, int level)
{
    int i, k, ac;

    /* Percentile dice */
    k = rand_int(100);

    /* Hack -- Always miss or hit */
    if (k < 10) return (k < 5);

    /* Calculate the "attack quality" */
    i = (power + (level * 3));

    /* Total armor */
    ac = p_ptr->ac + p_ptr->to_a;
    
    /* Power and Level compete against Armor */
    if ((i > 0) && (randint(i) > ((ac * 3) / 4))) return (TRUE);

    /* Assume miss */
    return (FALSE);
}



/*
 * Hack -- possible "insult" messages
 */
static cptr desc_insult[] = {
    "insults you!",
    "insults your mother!",
    "gives you the finger!",
    "humiliates you!",
    "defiles you!",
    "dances around you!",
    "makes obscene gestures!",
    "moons you!!!"
};



/*
 * Hack -- possible "insult" messages
 */
static cptr desc_moan[] = {
    "seems sad about something.",
    "asks if you have seen his dogs.",
    "tells you to get off his land.",
    "mumbles something about mushrooms."
};


/*
 * Attack the player via physical attacks.
 */
bool make_attack_normal(int m_idx)
{
    monster_type	*m_ptr = &m_list[m_idx];

    monster_race	*r_ptr = &r_info[m_ptr->r_idx];

    int			ap_cnt;

    int			i, j, k, tmp, ac, rlev;
    int			do_cut, do_stun;
    
    s32b		gold;

    inven_type		*i_ptr;

    char		i_name[80];

    char		m_name[80];

    char		ddesc[80];

    bool		blinked;



    /* Not allowed to attack */
    if (r_ptr->flags1 & RF1_NEVER_BLOW) return (FALSE);


    /* Total armor */
    ac = p_ptr->ac + p_ptr->to_a;
    
    /* Extract the effective monster level */
    rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);
    

    /* Get the monster name (or "it") */
    monster_desc(m_name, m_ptr, 0);

    /* Get the "died from" information (i.e. "a kobold") */
    monster_desc(ddesc, m_ptr, 0x88);


    /* Assume no blink */
    blinked = FALSE;

    /* Scan through all four blows */
    for (ap_cnt = 0; ap_cnt < 4; ap_cnt++) {

        bool visible = FALSE;
        bool obvious = FALSE;

        int power = 0;
        int damage = 0;

        cptr act = NULL;

        /* Extract the attack infomation */
        int effect = r_ptr->blow[ap_cnt].effect;
        int method = r_ptr->blow[ap_cnt].method;
        int d_dice = r_ptr->blow[ap_cnt].d_dice;
        int d_side = r_ptr->blow[ap_cnt].d_side;


        /* Hack -- no more attacks */
        if (!method) break;


        /* Extract visibility (before blink) */
        if (m_ptr->ml) visible = TRUE;



        /* Extract the attack "power" */
        switch (effect) {

            case RBE_HURT:	power = 60; break;
            case RBE_POISON:	power =  5; break;
            case RBE_UN_BONUS:	power = 20; break;
            case RBE_UN_POWER:	power = 15; break;
            case RBE_EAT_GOLD:	power =  5; break;
            case RBE_EAT_ITEM:	power =  5; break;
            case RBE_EAT_FOOD:	power =  5; break;
            case RBE_EAT_LITE:	power =  5; break;
            case RBE_ACID:	power =  0; break;
            case RBE_ELEC:	power = 10; break;
            case RBE_FIRE:	power = 10; break;
            case RBE_COLD:	power = 10; break;
            case RBE_BLIND:	power =  2; break;
            case RBE_CONFUSE:	power = 10; break;
            case RBE_TERRIFY:	power = 10; break;
            case RBE_PARALYZE:	power =  2; break;
            case RBE_LOSE_STR:	power =  0; break;
            case RBE_LOSE_DEX:	power =  0; break;
            case RBE_LOSE_CON:	power =  0; break;
            case RBE_LOSE_INT:	power =  0; break;
            case RBE_LOSE_WIS:	power =  0; break;
            case RBE_LOSE_CHR:	power =  0; break;
            case RBE_LOSE_ALL:	power =  2; break;
            case RBE_SHATTER:	power = 60; break;
            case RBE_EXP_10:	power =  5; break;
            case RBE_EXP_20:	power =  5; break;
            case RBE_EXP_40:	power =  5; break;
            case RBE_EXP_80:	power =  5; break;
        }


        /* Monster hits player */
        if (!effect || check_hit(power, rlev)) {


            /* Always disturbing */
            disturb(1, 0);


            /* Hack -- Apply "protection from evil" */
            if ((p_ptr->protevil > 0) &&
                (r_ptr->flags3 & RF3_EVIL) &&
                (p_ptr->lev >= rlev) &&
                ((rand_int(100) + p_ptr->lev) > 50)) {

                /* Remember the Evil-ness */
                if (m_ptr->ml) r_ptr->r_flags3 |= RF3_EVIL;

                /* Message */
                msg_format("%^s is repelled.", m_name);

                /* Hack -- Next attack */
                continue;
            }


            /* Assume no cut or stun */
            do_cut = do_stun = 0;

            /* Describe the attack method */
            switch (method) {

              case RBM_HIT:
                act = "hits you.";
                do_cut = do_stun = 1;
                break;

              case RBM_TOUCH:
                act = "touches you.";
                break;

              case RBM_PUNCH:
                act = "punches you.";
                do_stun = 1;
                break;

              case RBM_KICK:
                act = "kicks you.";
                do_stun = 1;
                break;

              case RBM_CLAW:
                act = "claws you.";
                do_cut = 1;
                break;

              case RBM_BITE:
                act = "bites you.";
                do_cut = 1;
                break;

              case RBM_STING:
                act = "stings you.";
                break;

              case RBM_XXX1:
                act = "XXX1's you.";
                break;

              case RBM_BUTT:
                act = "butts you.";
                do_stun = 1;
                break;

              case RBM_CRUSH:
                act = "crushes you.";
                do_stun = 1;
                break;

              case RBM_ENGULF:
                act = "engulfs you.";
                break;

              case RBM_XXX2:
                act = "XXX2's you.";
                break;

              case RBM_CRAWL:
                act = "crawls on you.";
                break;

              case RBM_DROOL:
                act = "drools on you.";
                break;

              case RBM_SPIT:
                act = "spits on you.";
                break;

              case RBM_XXX3:
                act = "XXX3's on you.";
                break;

              case RBM_GAZE:
                act = "gazes at you.";
                break;

              case RBM_WAIL:
                act = "wails at you.";
                break;

              case RBM_SPORE:
                act = "releases spores at you.";
                break;

              case RBM_XXX4:
                act = "projects XXX4's at you.";
                break;

              case RBM_BEG:
                act = "begs you for money.";
                break;

              case RBM_INSULT:
                act = desc_insult[rand_int(8)];
                break;

              case RBM_MOAN:
                act = desc_moan[rand_int(4)];
                break;

              case RBM_XXX5:
                act = "XXX5's you.";
                break;
            }

            /* Message */
            if (act) msg_format("%^s %s", m_name, act);


            /* Hack -- assume all attacks are obvious */
            obvious = TRUE;

            /* Roll out the damage */
            damage = damroll(d_dice, d_side);

            /* Apply appropriate damage */
            switch (effect) {

              case 0:

                /* Hack -- Assume obvious */
                obvious = TRUE;

                /* Hack -- No damage */
                damage = 0;

                break;

              case RBE_HURT:

                /* Obvious */
                obvious = TRUE;

                /* Hack -- Reduce damage based on the player armor class */
                damage -= damage * (((ac < 150) ? ac : 150) * 3 / 4) / 200;

                /* Take damage */
                take_hit(damage, ddesc);

                break;

              case RBE_POISON:

                /* Take some damage */
                take_hit(damage, ddesc);

                /* Take "poison" effect */
                if (!(p_ptr->resist_pois ||
                      p_ptr->oppose_pois ||
                      p_ptr->immune_pois)) {

                    if (set_poisoned(p_ptr->poisoned + randint(rlev) + 5)) {
                        msg_print("You feel very sick.");
                        obvious = TRUE;
                    }
                }

                /* Learn about the player */
                update_smart_learn(m_idx, DRS_POIS);

                break;

              case RBE_UN_BONUS:

                /* Take some damage */
                take_hit(damage, ddesc);

                /* Allow complete resist */
                if (!p_ptr->resist_disen) {

                    /* Apply disenchantment */
                    if (apply_disenchant(0)) obvious = TRUE;
                }

                /* Learn about the player */
                update_smart_learn(m_idx, DRS_DISEN);

                break;

              case RBE_UN_POWER:

                /* Take some damage */
                take_hit(damage, ddesc);

                /* Find an item */
                for (k = 0; k < 10; k++) {

                    /* Pick an item */
                    i = rand_int(INVEN_PACK);

                    /* Obtain the item */
                    i_ptr = &inventory[i];

                    /* Drain charged wands/staffs */
                    if (((i_ptr->tval == TV_STAFF) ||
                         (i_ptr->tval == TV_WAND)) &&
                        (i_ptr->pval)) {

                        /* Message */
                        msg_print("Energy drains from your pack!");

                        /* Obvious */
                        obvious = TRUE;

                        /* Uncharge */
                        i_ptr->pval = 0;

                        /* Redraw the choice window */
                        p_ptr->redraw |= (PR_CHOOSE);

                        /* Heal */
                        j = rlev;
                        m_ptr->hp += j * i_ptr->pval * i_ptr->number;
                        if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

                        /* Redraw (later) if needed */
                        if (health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

                        /* Done */
                        break;
                    }
                }

                break;

              case RBE_EAT_GOLD:

                /* Take some damage */
                take_hit(damage, ddesc);

                /* Obvious */
                obvious = TRUE;

                /* Saving throw (unless paralyzed) based on dex and level */
                if (!p_ptr->paralyzed &&
                    (rand_int(100) < (adj_dex_safe[p_ptr->stat_ind[A_DEX]] +
                                      p_ptr->lev))) {

                    /* Saving throw message */
                    msg_print("You quickly protect your money pouch!");

                    /* Occasional blink anyway */
                    if (rand_int(3)) blinked = TRUE;
                }

                /* Eat gold */
                else {

                    gold = (p_ptr->au / 10) + randint(25);
                    if (gold < 2) gold = 2;
                    if (gold > 5000) gold = (p_ptr->au / 20) + randint(3000);
                    if (gold > p_ptr->au) gold = p_ptr->au;
                    p_ptr->au -= gold;
                    if (gold <= 0) {
                        msg_print("Nothing was stolen.");
                    }
                    else if (p_ptr->au) {
                        msg_print("Your purse feels lighter.");
                        msg_format("%ld coins were stolen!", (long)gold);
                    }
                    else {
                        msg_print("Your purse feels lighter.");
                        msg_print("All of your coins were stolen!");
                    }
                    p_ptr->redraw |= (PR_GOLD);
                    blinked = TRUE;
                }

                break;

              case RBE_EAT_ITEM:

                /* Take some damage */
                take_hit(damage, ddesc);

                /* Saving throw (unless paralyzed) based on dex and level */
                if (!p_ptr->paralyzed &&
                    (rand_int(100) < (adj_dex_safe[p_ptr->stat_ind[A_DEX]] +
                                      p_ptr->lev))) {

                    /* Saving throw message */
                    msg_print("You grab hold of your backpack!");

                    /* Occasional "blink" anyway */
                    blinked = TRUE;

                    /* Obvious */
                    obvious = TRUE;

                    /* Done */
                    break;
                }

                /* Find an item */
                for (k = 0; k < 10; k++) {

                    /* Pick an item */
                    i = rand_int(INVEN_PACK);

                    /* Obtain the item */
                    i_ptr = &inventory[i];

                    /* Accept real items */
                    if (!i_ptr->k_idx) continue;
                    
                    /* Don't steal artifacts  -CFT */
                    if (artifact_p(i_ptr)) continue;

                    /* Get a description */
                    objdes(i_name, i_ptr, FALSE, 3);

                    /* Message */
                    msg_format("%sour %s (%c) was stolen!",
                               ((i_ptr->number > 1) ? "One of y" : "Y"),
                               i_name, index_to_label(i));

                    /* Steal the items */
                    inven_item_increase(i,-1);
                    inven_item_optimize(i);

                    /* Redraw the choice window */
                    p_ptr->redraw |= (PR_CHOOSE);

                    /* Obvious */
                    obvious = TRUE;
                    
                    /* Blink away */
                    blinked = TRUE;

                    /* Done */
                    break;
                }

                break;

              case RBE_EAT_FOOD:

                /* Take some damage */
                take_hit(damage, ddesc);

                /* Steal some food */
                for (k = 0; k < 10; k++) {

                    /* Pick an item from the pack */
                    i = rand_int(INVEN_PACK);

                    /* Get the item */
                    i_ptr = &inventory[i];

                    /* Accept real items */
                    if (!i_ptr->k_idx) continue;
                    
                    /* Only eat food */
                    if (i_ptr->tval != TV_FOOD) continue;

                    /* Get a description */
                    objdes(i_name, i_ptr, FALSE, 0);

                    /* Message */
                    msg_format("%sour %s (%c) was eaten!",
                               ((i_ptr->number > 1) ? "One of y" : "Y"),
                               i_name, index_to_label(i));

                    /* Steal the items */
                    inven_item_increase(i,-1);
                    inven_item_optimize(i);

                    /* Redraw the choice window */
                    p_ptr->redraw |= (PR_CHOOSE);

                    /* Obvious */
                    obvious = TRUE;
                    
                    /* Done */
                    break;
                }

                break;

              case RBE_EAT_LITE:

                /* Take some damage */
                take_hit(damage, ddesc);

                /* Access the lite */
                i_ptr = &inventory[INVEN_LITE];

                /* Drain fuel */
                if ((i_ptr->pval > 0) && (!artifact_p(i_ptr))) {

                    /* Reduce fuel */
                    i_ptr->pval -= (250 + randint(250));
                    if (i_ptr->pval < 1) i_ptr->pval = 1;

                    /* Notice */
                    if (!p_ptr->blind) {
                        msg_print("Your light dims.");
                        obvious = TRUE;
                    }

                    /* Redraw the choice window */
                    p_ptr->redraw |= (PR_CHOOSE);
                }

                break;

              case RBE_ACID:

                /* Obvious */
                obvious = TRUE;

                /* Message */
                msg_print("You are covered in acid!");

                /* Special damage */
                acid_dam(damage, ddesc);

                /* Learn about the player */
                update_smart_learn(m_idx, DRS_ACID);

                break;

              case RBE_ELEC:

                /* Obvious */
                obvious = TRUE;

                /* Message */
                msg_print("You are struck by electricity!");

                /* Special damage */
                elec_dam(damage, ddesc);

                /* Learn about the player */
                update_smart_learn(m_idx, DRS_ELEC);

                break;

              case RBE_FIRE:

                /* Obvious */
                obvious = TRUE;

                /* Message */
                msg_print("You are enveloped in flames!");

                /* Special damage */
                fire_dam(damage, ddesc);

                /* Learn about the player */
                update_smart_learn(m_idx, DRS_FIRE);

                break;

              case RBE_COLD:

                /* Obvious */
                obvious = TRUE;

                /* Message */
                msg_print("You are covered with frost!");

                /* Special damage */
                cold_dam(damage, ddesc);

                /* Learn about the player */
                update_smart_learn(m_idx, DRS_COLD);

                break;

              case RBE_BLIND:

                /* Take damage */
                take_hit(damage, ddesc);

                /* Increase "blind" */
                if (!p_ptr->resist_blind) {
                    if (set_blind(p_ptr->blind + 10 + randint(rlev))) {
                        msg_print("Your eyes begin to sting.");
                        obvious = TRUE;
                    }
                }

                /* Learn about the player */
                update_smart_learn(m_idx, DRS_BLIND);

                break;

              case RBE_CONFUSE:

                /* Take damage */
                take_hit(damage, ddesc);

                /* Increase "confused" */
                if (!p_ptr->resist_conf) {
                    if (set_confused(p_ptr->confused + 3 + randint(rlev))) {
                        msg_print("You feel confused.");
                        obvious = TRUE;
                    }
                }

                /* Learn about the player */
                update_smart_learn(m_idx, DRS_CONF);

                break;

              case RBE_TERRIFY:

                /* Take damage */
                take_hit(damage, ddesc);

                /* Increase "afraid" */
                if (p_ptr->resist_fear) {
                    msg_print("You stand your ground!");
                    obvious = TRUE;
                }
                else if (rand_int(100) < p_ptr->skill_sav) {
                    msg_print("You stand your ground!");
                    obvious = TRUE;
                }
                else {
                    if (set_afraid(p_ptr->afraid + 3 + randint(rlev))) {
                        msg_print("You are suddenly afraid!");
                        obvious = TRUE;
                    }
                }

                /* Learn about the player */
                update_smart_learn(m_idx, DRS_FEAR);

                break;

              case RBE_PARALYZE:

                /* Take damage */
                take_hit(damage, ddesc);
                
                /* Increase "paralyzed" */
                if (p_ptr->free_act) {
                    msg_print("You are unaffected!");
                    obvious = TRUE;
                }
                else if (rand_int(100) < p_ptr->skill_sav) {
                    msg_print("You resist the effects!");
                    obvious = TRUE;
                }
                else {
                    if (set_paralyzed(p_ptr->paralyzed + 3 + randint(rlev))) {
                        msg_print("You are paralyzed!");
                        obvious = TRUE;
                    }
                }

                /* Learn about the player */
                update_smart_learn(m_idx, DRS_FREE);

                break;

              case RBE_LOSE_STR:

                /* Damage (physical) */
                take_hit(damage, ddesc);

                /* Damage (stat) */
                if (do_dec_stat(A_STR)) obvious = TRUE;

                break;

              case RBE_LOSE_INT:

                /* Damage (physical) */
                take_hit(damage, ddesc);

                /* Damage (stat) */
                if (do_dec_stat(A_INT)) obvious = TRUE;

                break;

              case RBE_LOSE_WIS:

                /* Damage (physical) */
                take_hit(damage, ddesc);

                /* Damage (stat) */
                if (do_dec_stat(A_WIS)) obvious = TRUE;

                break;

              case RBE_LOSE_DEX:

                /* Damage (physical) */
                take_hit(damage, ddesc);

                /* Damage (stat) */
                if (do_dec_stat(A_DEX)) obvious = TRUE;

                break;

              case RBE_LOSE_CON:

                /* Damage (physical) */
                take_hit(damage, ddesc);

                /* Damage (stat) */
                if (do_dec_stat(A_CON)) obvious = TRUE;

                break;

              case RBE_LOSE_CHR:

                /* Damage (physical) */
                take_hit(damage, ddesc);

                /* Damage (stat) */
                if (do_dec_stat(A_CHR)) obvious = TRUE;

                break;

              case RBE_LOSE_ALL:

                /* Damage (physical) */
                take_hit(damage, ddesc);

                /* Damage (stats) */
                if (do_dec_stat(A_STR)) obvious = TRUE;
                if (do_dec_stat(A_DEX)) obvious = TRUE;
                if (do_dec_stat(A_CON)) obvious = TRUE;
                if (do_dec_stat(A_INT)) obvious = TRUE;
                if (do_dec_stat(A_WIS)) obvious = TRUE;
                if (do_dec_stat(A_CHR)) obvious = TRUE;

                break;

              case RBE_SHATTER:

                /* Obvious */
                obvious = TRUE;

                /* Hack -- Reduce damage based on the player armor class */
                damage -= damage * (((ac < 150) ? ac : 150) * 3 / 4) / 200;

                /* Take damage */
                take_hit(damage, ddesc);

                /* Radius 8 earthquake centered at the monster */
                if (damage > 23) earthquake(m_ptr->fy, m_ptr->fx, 8);

                break;

              case RBE_EXP_10:

                /* Obvious */
                obvious = TRUE;

                /* Take damage */
                take_hit(damage, ddesc);
                
                if (p_ptr->hold_life && (rand_int(100) < 95)) {
                    msg_print("You keep hold of your life force!");
                }
                else {
                    s32b d = damroll(10,6) + (p_ptr->exp/100) * MON_DRAIN_LIFE;
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

              case RBE_EXP_20:

                /* Obvious */
                obvious = TRUE;

                /* Take damage */
                take_hit(damage, ddesc);

                if (p_ptr->hold_life && (rand_int(100) < 90)) {
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

                /* Obvious */
                obvious = TRUE;

                /* Take damage */
                take_hit(damage, ddesc);

                if (p_ptr->hold_life && (rand_int(100) < 75)) {
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

                /* Obvious */
                obvious = TRUE;

                /* Take damage */
                take_hit(damage, ddesc);

                if (p_ptr->hold_life && (rand_int(100) < 50)) {
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
            }


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
              case 4: cut_player(randint(50) + 50); break;
              case 5: cut_player(randint(100) + 100); break;
              case 6: cut_player(300); break;
              default: cut_player(500); break;
            }

            /* Critical Stun (note check for "do_stun==0") */
            switch (do_stun * tmp) {
              case 0: break;
              case 1: stun_player(randint(5)); break;
              case 2: stun_player(randint(10) + 10); break;
              case 3: stun_player(randint(20) + 20); break;
              case 4: stun_player(randint(30) + 30); break;
              case 5: stun_player(randint(40) + 40); break;
              case 6: stun_player(randint(50) + 50); break;
              default: stun_player(randint(75) + 75); break;
            }
        }

        /* Monster missed player */
        else {

            /* Analyze failed attacks */
            switch (method) {

                case RBM_HIT:
                case RBM_TOUCH:
                case RBM_PUNCH:
                case RBM_KICK:
                case RBM_CLAW:
                case RBM_BITE:
                case RBM_STING:
                case RBM_XXX1:
                case RBM_BUTT:
                case RBM_CRUSH:
                case RBM_ENGULF:
                case RBM_XXX2:

                    /* Visible monsters */
                    if (m_ptr->ml) {

                        /* Disturbing */
                        disturb(1, 0);

                        /* Message */
                        msg_format("%^s misses you.", m_name);
                    }

                    break;
            }
        }


        /* Analyze "visible" monsters only */
        if (visible) {

            /* Count "obvious" attacks (and ones that cause damage) */
            if (obvious || damage || (r_ptr->r_blows[ap_cnt] > 10)) {

                /* Count attacks of this type */
                if (r_ptr->r_blows[ap_cnt] < MAX_UCHAR) {
                    r_ptr->r_blows[ap_cnt]++;
                }
            }
        }


        /* Hack -- Player is dead */
        if (death) break;
    }


    /* Blink away */
    if (blinked) {
        msg_print("There is a puff of smoke!");
        teleport_away(m_idx, MAX_SIGHT * 2 + 5);
    }


    /* Always notice cause of death */
    if (death && (r_ptr->r_deaths < MAX_SHORT)) r_ptr->r_deaths++;


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
    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    /* Determine the radius of the blast */
    max_dis = (r_ptr->flags2 & RF2_POWERFUL) ? 3 : 2;

    /* Go towards player, do not hit anyone else, hurt items on ground. */
    (void)project(m_idx, max_dis, y, x, dam_hp, typ, flg);
}



/*
 * Creatures can cast spells, shoot missiles, and breathe.
 *
 * Returns "TRUE" if a spell (or whatever) was (successfully) cast.
 *
 * XXX XXX XXX This function could use some work, but remember to
 * keep it as optimized as possible, while retaining generic code.
 *
 * Verify the various "blind-ness" checks in the code.
 *
 * Perhaps monsters should breathe at locations *near* the player,
 * since this would allow them to inflict "partial" damage.
 *
 * Perhaps smart monsters should decline to use "bolt" spells if
 * there is a monster in the way, unless they wish to kill it.
 *
 * Note that, to allow the use of the "track_target" option at some
 * later time, certain non-optimal things are done in the code below,
 * including explicit checks against the "direct" variable, which is
 * currently always true by the time it is checked, but which should
 * really be set according to an explicit "projectable()" test, and
 * the use of generic "x,y" locations instead of the player location,
 * with those values being initialized with the player location.
 *
 * It will not be possible to "correctly" handle the case in which a
 * monster attempts to attack a location which is thought to contain
 * the player, but which in fact is nowhere near the player, since this
 * might induce all sorts of messages about the attack itself, and about
 * the effects of the attack, which the player might or might not be in
 * a position to observe.  Thus, for simplicity, it is probably best to
 * only allow "faulty" attacks by a monster if one of the important grids
 * (probably the initial or final grid) is in fact in view of the player.
 * It may be necessary to actually prevent spell attacks except when the
 * monster actually has line of sight to the player.  Note that a monster
 * could be left in a bizarre situation after the player ducked behind a
 * pillar and then teleported away, for example.
 *
 * Note that certain spell attacks do not use the "project()" function
 * but "simulate" it via the "direct" variable, which is always at least
 * as restrictive as the "project()" function.  This is necessary to
 * prevent "blindness" attacks and such from bending around walls, etc,
 * and to allow the use of the "track_target" option in the future.
 *
 * Note that this function attempts to optimize the use of spells for the
 * cases in which the monster has no spells, or has spells but cannot use
 * them, or has spells but they will have no "useful" effect.  Note that
 * this function has been an efficiency bottleneck in the past.
 */
bool make_attack_spell(int m_idx)
{
    int			k, chance, thrown_spell, rlev;

    byte		spell[96], num = 0;

    u32b		f4, f5, f6;

    monster_type	*m_ptr = &m_list[m_idx];
    monster_race	*r_ptr = &r_info[m_ptr->r_idx];

    char		m_name[80];
    char		m_poss[80];

    char		ddesc[80];


    /* Target location */
    int x = px;
    int y = py;

    /* Summon count */
    int count = 0;


    /* Extract the blind-ness */
    bool blind = (p_ptr->blind ? TRUE : FALSE);

    /* Extract the "see-able-ness" */
    bool seen = (!blind && m_ptr->ml);


    /* Assume "normal" target */
    bool normal = TRUE;

    /* Assume "projectable" */
    bool direct = TRUE;


    /* Hack -- Extract the spell probability */
    chance = (r_ptr->freq_inate + r_ptr->freq_spell) / 2;

    /* Not allowed to cast spells */
    if (!chance) return (FALSE);

    /* Cannot cast spells when confused */
    if (m_ptr->confused) return (FALSE);

    /* Only do spells occasionally */
    if (rand_int(100) >= chance) return (FALSE);


    /* XXX XXX XXX Handle "track_target" option (?) */


    /* Hack -- require projectable player */
    if (normal) {

        /* Check range */    
        if (m_ptr->cdis > MAX_RANGE) return (FALSE);

        /* Check path */
        if (!projectable(m_ptr->fy, m_ptr->fx, py, px)) return (FALSE);
    }


    /* Extract the monster level */
    rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);


    /* Extract the racial spell flags */
    f4 = r_ptr->flags4;
    f5 = r_ptr->flags5;
    f6 = r_ptr->flags6;


    /* Hack -- allow "desperate" spells */
    if ((r_ptr->flags2 & RF2_SMART) &&
        (m_ptr->hp < m_ptr->maxhp / 10) &&
        (rand_int(100) < 50)) {

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

    /* No spells left */
    if (!f4 && !f5 && !f6) return (FALSE);

#endif


    /* Extract the "inate" spells */
    for (k = 0; k < 32; k++) {
        if (f4 & (1L << k)) spell[num++] = k + 32 * 3;
    }
    
    /* Extract the "normal" spells */
    for (k = 0; k < 32; k++) {
        if (f5 & (1L << k)) spell[num++] = k + 32 * 4;
    }
    
    /* Extract the "bizarre" spells */
    for (k = 0; k < 32; k++) {
        if (f6 & (1L << k)) spell[num++] = k + 32 * 5;
    }

    /* No spells left */
    if (!num) return (FALSE);


    /* Hack -- No need to attack */
    if (!alive) return (FALSE);


    /* Get the monster name (or "it") */
    monster_desc(m_name, m_ptr, 0x00);

    /* Get the monster possessive ("his"/"her"/"its") */
    monster_desc(m_poss, m_ptr, 0x22);

    /* Hack -- Get the "died from" name */
    monster_desc(ddesc, m_ptr, 0x88);


    /* Choose a spell to cast */
    thrown_spell = spell[rand_int(num)];


    /* Cast the spell. */
    switch (thrown_spell) {

      case 96+0:    /* RF4_SHRIEK */
        if (!direct) break;
        disturb(1, 0);
        msg_format("%^s makes a high pitched shriek.", m_name);
        aggravate_monsters(m_idx);
        break;

      case 96+1:    /* RF4_XXX2X4 */
        break;

      case 96+2:    /* RF4_XXX3X4 */
        break;

      case 96+3:    /* RF4_XXX4X4 */
        break;

      case 96+4:    /* RF4_ARROW_1 */
        disturb(1, 0);
        if (blind) msg_format("%^s makes a strange noise.", m_name);
        else msg_format("%^s fires an arrow.", m_name);
        bolt(m_idx, GF_ARROW, damroll(1, 6));
        break;

      case 96+5:    /* RF4_ARROW_2 */
        disturb(1, 0);
        if (blind) msg_format("%^s makes a strange noise.", m_name);
        else msg_format("%^s fires an arrow!", m_name);
        bolt(m_idx, GF_ARROW, damroll(3, 6));
        break;

      case 96+6:    /* RF4_ARROW_3 */
        disturb(1, 0);
        if (blind) msg_format("%^s makes a strange noise.", m_name);
        else msg_format("%^s fires a missile.", m_name);
        bolt(m_idx, GF_ARROW, damroll(5, 6));
        break;

      case 96+7:    /* RF4_ARROW_4 */
        disturb(1, 0);
        if (blind) msg_format("%^s makes a strange noise.", m_name);
        else msg_format("%^s fires a missile!", m_name);
        bolt(m_idx, GF_ARROW, damroll(7, 6));
        break;

      case 96+8:    /* RF4_BR_ACID */
        disturb(1, 0);
        if (blind) msg_format("%^s breathes.", m_name);
        else msg_format("%^s breathes acid.", m_name);
        breath(m_idx, GF_ACID,
               ((m_ptr->hp / 3) > 1600 ? 1600 : (m_ptr->hp / 3)));
        update_smart_learn(m_idx, DRS_ACID);
        break;

      case 96+9:    /* RF4_BR_ELEC */
        disturb(1, 0);
        if (blind) msg_format("%^s breathes.", m_name);
        else msg_format("%^s breathes lightning.", m_name);
        breath(m_idx, GF_ELEC,
               ((m_ptr->hp / 3) > 1600 ? 1600 : (m_ptr->hp / 3)));
        update_smart_learn(m_idx, DRS_ELEC);
        break;

      case 96+10:    /* RF4_BR_FIRE */
        disturb(1, 0);
        if (blind) msg_format("%^s breathes.", m_name);
        else msg_format("%^s breathes fire.", m_name);
        breath(m_idx, GF_FIRE,
               ((m_ptr->hp / 3) > 1600 ? 1600 : (m_ptr->hp / 3)));
        update_smart_learn(m_idx, DRS_FIRE);
        break;

      case 96+11:    /* RF4_BR_COLD */
        disturb(1, 0);
        if (blind) msg_format("%^s breathes.", m_name);
        else msg_format("%^s breathes frost.", m_name);
        breath(m_idx, GF_COLD,
               ((m_ptr->hp / 3) > 1600 ? 1600 : (m_ptr->hp / 3)));
        update_smart_learn(m_idx, DRS_COLD);
        break;

      case 96+12:    /* RF4_BR_POIS */
        disturb(1, 0);
        if (blind) msg_format("%^s breathes.", m_name);
        else msg_format("%^s breathes gas.", m_name);
        breath(m_idx, GF_POIS,
               ((m_ptr->hp / 3) > 800 ? 800 : (m_ptr->hp / 3)));
        update_smart_learn(m_idx, DRS_POIS);
        break;

      case 96+13:    /* RF4_BR_NETH */
        disturb(1, 0);
        if (blind) msg_format("%^s breathes.", m_name);
        else msg_format("%^s breathes nether.", m_name);
        breath(m_idx, GF_NETHER,
               ((m_ptr->hp / 6) > 550 ? 550 : (m_ptr->hp / 6)) );
        update_smart_learn(m_idx, DRS_NETH);
        break;

      case 96+14:    /* RF4_BR_LITE */
        disturb(1, 0);
        if (blind) msg_format("%^s breathes.", m_name);
        else msg_format("%^s breathes light.", m_name);
        breath(m_idx, GF_LITE,
            ((m_ptr->hp / 6) > 400 ? 400 : (m_ptr->hp / 6)));
        update_smart_learn(m_idx, DRS_LITE);
        break;

      case 96+15:    /* RF4_BR_DARK */
        disturb(1, 0);
        if (blind) msg_format("%^s breathes.", m_name);
        else msg_format("%^s breathes darkness.", m_name);
        breath(m_idx, GF_DARK,
            ((m_ptr->hp / 6) > 400 ? 400 : (m_ptr->hp / 6)));
        update_smart_learn(m_idx, DRS_DARK);
        break;

      case 96+16:    /* RF4_BR_CONF */
        disturb(1, 0);
        if (blind) msg_format("%^s breathes.", m_name);
        else msg_format("%^s breathes confusion.", m_name);
        breath(m_idx, GF_CONFUSION,
            ((m_ptr->hp / 6) > 400 ? 400 : (m_ptr->hp / 6)));
        update_smart_learn(m_idx, DRS_CONF);
        break;

      case 96+17:    /* RF4_BR_SOUN */
        disturb(1, 0);
        if (blind) msg_format("%^s breathes.", m_name);
        else msg_format("%^s breathes sound.", m_name);
        breath(m_idx, GF_SOUND,
            ((m_ptr->hp / 6) > 400 ? 400 : (m_ptr->hp / 6)));
        update_smart_learn(m_idx, DRS_SOUND);
        break;

      case 96+18:    /* RF4_BR_CHAO */
        disturb(1, 0);
        if (blind) msg_format("%^s breathes.", m_name);
        else msg_format("%^s breathes chaos.", m_name);
        breath(m_idx, GF_CHAOS,
               ((m_ptr->hp / 6) > 600 ? 600 : (m_ptr->hp / 6)));
        update_smart_learn(m_idx, DRS_CHAOS);
        break;

      case 96+19:    /* RF4_BR_DISE */
        disturb(1, 0);
        if (blind) msg_format("%^s breathes.", m_name);
        else msg_format("%^s breathes disenchantment.", m_name);
        breath(m_idx, GF_DISENCHANT,
            ((m_ptr->hp / 6) > 500 ? 500 : (m_ptr->hp / 6)));
        update_smart_learn(m_idx, DRS_DISEN);
        break;

      case 96+20:    /* RF4_BR_NEXU */
        disturb(1, 0);
        if (blind) msg_format("%^s breathes.", m_name);
        else msg_format("%^s breathes nexus.", m_name);
        breath(m_idx, GF_NEXUS,
            ((m_ptr->hp / 3) > 250 ? 250 : (m_ptr->hp / 3)));
        update_smart_learn(m_idx, DRS_NEXUS);
        break;

      case 96+21:    /* RF4_BR_TIME */
        disturb(1, 0);
        if (blind) msg_format("%^s breathes.", m_name);
        else msg_format("%^s breathes time.", m_name);
        breath(m_idx, GF_TIME,
            ((m_ptr->hp / 3) > 150 ? 150 : (m_ptr->hp / 3)));
        break;

      case 96+22:    /* RF4_BR_INER */
        disturb(1, 0);
        if (blind) msg_format("%^s breathes.", m_name);
        else msg_format("%^s breathes inertia.", m_name);
        breath(m_idx, GF_INERTIA,
               ((m_ptr->hp / 6) > 200 ? 200 : (m_ptr->hp / 6)));
        break;

      case 96+23:    /* RF4_BR_GRAV */
        disturb(1, 0);
        if (blind) msg_format("%^s breathes.", m_name);
        else msg_format("%^s breathes gravity.", m_name);
        breath(m_idx, GF_GRAVITY,
            ((m_ptr->hp / 3) > 200 ? 200 : (m_ptr->hp / 3)));
        break;

      case 96+24:    /* RF4_BR_SHAR */
        disturb(1, 0);
        if (blind) msg_format("%^s breathes.", m_name);
        else msg_format("%^s breathes shards.", m_name);
        breath(m_idx, GF_SHARDS,
            ((m_ptr->hp / 6) > 400 ? 400 : (m_ptr->hp / 6)));
        update_smart_learn(m_idx, DRS_SHARD);
        break;

      case 96+25:    /* RF4_BR_PLAS */
        disturb(1, 0);
        if (blind) msg_format("%^s breathes.", m_name);
        else msg_format("%^s breathes plasma.", m_name);
        breath(m_idx, GF_PLASMA,
            ((m_ptr->hp / 6) > 150 ? 150 : (m_ptr->hp / 6)));
        break;

      case 96+26:    /* RF4_BR_WALL */
        disturb(1, 0);
        if (blind) msg_format("%^s breathes.", m_name);
        else msg_format("%^s breathes force.", m_name);
        breath(m_idx, GF_FORCE,
               ((m_ptr->hp / 6) > 200 ? 200 : (m_ptr->hp / 6)));
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
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s casts an acid ball.", m_name);
        breath(m_idx, GF_ACID,
               randint(rlev * 3) + 15);
        update_smart_learn(m_idx, DRS_ACID);
        break;

      case 128+1:    /* RF5_BA_ELEC */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s casts a lightning ball.", m_name);
        breath(m_idx, GF_ELEC,
            randint(rlev * 3 / 2) + 8);
        update_smart_learn(m_idx, DRS_ELEC);
        break;

      case 128+2:    /* RF5_BA_FIRE */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s casts a fire ball.", m_name);
        breath(m_idx, GF_FIRE,
               randint(rlev * 7 / 2) + 10);
        update_smart_learn(m_idx, DRS_FIRE);
        break;

      case 128+3:    /* RF5_BA_COLD */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s casts a frost ball.", m_name);
        breath(m_idx, GF_COLD,
               randint(rlev * 3 / 2) + 10);
        update_smart_learn(m_idx, DRS_COLD);
        break;

      case 128+4:    /* RF5_BA_POIS */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s casts a stinking cloud.", m_name);
        breath(m_idx, GF_POIS,
               damroll(12, 2));
        update_smart_learn(m_idx, DRS_POIS);
        break;

      case 128+5:    /* RF5_BA_NETH */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s casts a nether ball.", m_name);
        breath(m_idx, GF_NETHER,
               (50 + damroll(10, 10) + rlev));
        update_smart_learn(m_idx, DRS_NETH);
        break;

      case 128+6:    /* RF5_BA_WATE */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s gestures fluidly.", m_name);
        msg_print("You are engulfed in a whirlpool.");
        breath(m_idx, GF_WATER,
               randint(rlev * 5 / 2) + 50);
        break;

      case 128+7:    /* RF5_BA_MANA */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles powerfully.", m_name);
        else msg_format("%^s invokes a mana storm.", m_name);
        breath(m_idx, GF_MANA,
               (rlev * 5) + damroll(10, 10));
        break;

      case 128+8:    /* RF5_BA_DARK */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles powerfully.", m_name);
        else msg_format("%^s invokes a darkness storm.", m_name);
        breath(m_idx, GF_DARK,
               (rlev * 5) + damroll(10, 10));
        update_smart_learn(m_idx, DRS_DARK);
        break;

      case 128+9:    /* RF5_DRAIN_MANA */
        if (!direct) break;
        if (p_ptr->csp) {

            int r1;

            /* Disturb if legal */
            disturb(1, 0);

            /* Basic message */
            msg_format("%^s draws psychic energy from you!", m_name);

            /* Attack power */
            r1 = (randint(rlev) / 2) + 1;

            /* Full drain */
            if (r1 >= p_ptr->csp) {
                r1 = p_ptr->csp;
                p_ptr->csp = 0;
                p_ptr->csp_frac = 0;
            }

            /* Partial drain */
            else {
                p_ptr->csp -= r1;
            }

            /* Redraw mana */
            p_ptr->redraw |= (PR_MANA);

            /* Heal the monster */
            if (m_ptr->hp < m_ptr->maxhp) {

                /* Heal */
                m_ptr->hp += (6 * r1);
                if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

                /* Redraw (later) if needed */
                if (health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

                /* Special message */
                if (seen) {
                    msg_format("%^s appears healthier.", m_name);
                }
            }
        }
        update_smart_learn(m_idx, DRS_MANA);
        break;

      case 128+10:    /* RF5_MIND_BLAST */
        if (!direct) break;
        disturb(1, 0);
        if (!seen) {
            msg_print("You feel something focusing on your mind.");
        }
        else {
            msg_format("%^s gazes deep into your eyes.", m_name);
        }

        if (rand_int(100) < p_ptr->skill_sav) {
            msg_print("You resist the effects!");
        }
        else {
            msg_print("Your mind is blasted by psionic energy.");
            if (!p_ptr->resist_conf) {
                (void)set_confused(p_ptr->confused + rand_int(4) + 4);
            }
            take_hit(damroll(8, 8), ddesc);
        }
        break;

      case 128+11:    /* RF5_BRAIN_SMASH */
        if (!direct) break;
        disturb(1, 0);
        if (!seen) {
            msg_print("You feel something focusing on your mind.");
        }
        else {
            msg_format("%^s looks deep into your eyes.", m_name);
        }
        if (rand_int(100) < p_ptr->skill_sav) {
            msg_print("You resist the effects!");
        }
        else {
            msg_print("Your mind is blasted by psionic energy.");
            take_hit(damroll(12, 15), ddesc);
            if (!p_ptr->resist_blind) {
                (void)set_blind(p_ptr->blind + 8 + rand_int(8));
            }
            if (!p_ptr->resist_conf) {
                (void)set_confused(p_ptr->confused + rand_int(4) + 4);
            }
            if (!p_ptr->free_act) {
                (void)set_paralyzed(p_ptr->paralyzed + rand_int(4) + 4);
            }
            (void)set_slow(p_ptr->slow + rand_int(4) + 4);
        }
        break;

      case 128+12:    /* RF5_CAUSE_1 */
        if (!direct) break;
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s points at you and curses.", m_name);
        if (rand_int(100) < p_ptr->skill_sav) {
            msg_print("You resist the effects!");
        }
        else {
            take_hit(damroll(3, 8), ddesc);
        }
        break;

      case 128+13:    /* RF5_CAUSE_2 */
        if (!direct) break;
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s points at you and curses horribly.", m_name);
        if (rand_int(100) < p_ptr->skill_sav) {
            msg_print("You resist the effects!");
        }
        else {
            take_hit(damroll(8, 8), ddesc);
        }
        break;

      case 128+14:    /* RF5_CAUSE_3 */
        if (!direct) break;
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles loudly.", m_name);
        else msg_format("%^s points at you, incanting terribly!", m_name);
        if (rand_int(100) < p_ptr->skill_sav) {
            msg_print("You resist the effects!");
        }
        else {
            take_hit(damroll(10, 15), ddesc);
        }
        break;

      case 128+15:    /* RF5_CAUSE_4 */
        if (!direct) break;
        disturb(1, 0);
        if (blind) msg_format("%^s screams the word 'DIE!'", m_name);
        else msg_format("%^s points at you, screaming the word DIE!", m_name);
        if (rand_int(100) < p_ptr->skill_sav) {
            msg_print("You resist the effects!");
        }
        else {
            msg_print("You start to bleed!");
            take_hit(damroll(15, 15), ddesc);
            cut_player(damroll(10, 10));
        }
        break;

      case 128+16:    /* RF5_BO_ACID */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s casts a acid bolt.", m_name);
        bolt(m_idx, GF_ACID,
             damroll(7, 8) + (rlev / 3));
        update_smart_learn(m_idx, DRS_ACID);
        break;

      case 128+17:    /* RF5_BO_ELEC */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s casts a lightning bolt.", m_name);
        bolt(m_idx, GF_ELEC,
             damroll(4, 8) + (rlev / 3));
        update_smart_learn(m_idx, DRS_ELEC);
        break;

      case 128+18:    /* RF5_BO_FIRE */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s casts a fire bolt.", m_name);
        bolt(m_idx, GF_FIRE,
             damroll(9, 8) + (rlev / 3));
        update_smart_learn(m_idx, DRS_FIRE);
        break;

      case 128+19:    /* RF5_BO_COLD */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s casts a frost bolt.", m_name);
        bolt(m_idx, GF_COLD,
             damroll(6, 8) + (rlev / 3));
        update_smart_learn(m_idx, DRS_COLD);
        break;

      case 128+20:    /* RF5_BO_POIS */
        /* XXX XXX XXX */
        break;

      case 128+21:    /* RF5_BO_NETH */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s casts a nether bolt.", m_name);
        bolt(m_idx, GF_NETHER,
             30 + damroll(5, 5) + (rlev * 3) / 2);
        update_smart_learn(m_idx, DRS_NETH);
        break;

      case 128+22:    /* RF5_BO_WATE */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s casts a water bolt.", m_name);
        bolt(m_idx, GF_WATER,
             damroll(10, 10) + (rlev));
        break;

      case 128+23:    /* RF5_BO_MANA */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s casts a mana bolt.", m_name);
        bolt(m_idx, GF_MANA,
             randint(rlev * 7 / 2) + 50);
        break;

      case 128+24:    /* RF5_BO_PLAS */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s casts a plasma bolt.", m_name);
        bolt(m_idx, GF_PLASMA,
             10 + damroll(8, 7) + (rlev));
        break;

      case 128+25:    /* RF5_BO_ICEE */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s casts an ice bolt.", m_name);
        bolt(m_idx, GF_ICE,
             damroll(6, 6) + (rlev));
        update_smart_learn(m_idx, DRS_COLD);
        break;

      case 128+26:    /* RF5_MISSILE */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s casts a magic missile.", m_name);
        bolt(m_idx, GF_MISSILE,
             damroll(2, 6) + (rlev / 3));
        break;

      case 128+27:    /* RF5_SCARE */
        if (!direct) break;
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles, and you hear scary noises.", m_name);
        else msg_format("%^s casts a fearful illusion.", m_name);
        if (p_ptr->resist_fear) {
            msg_print("You refuse to be frightened.");
        }
        else if (rand_int(100) < p_ptr->skill_sav) {
            msg_print("You refuse to be frightened.");
        }
        else {
            (void)set_afraid(p_ptr->afraid + rand_int(4) + 4);
        }
        update_smart_learn(m_idx, DRS_FEAR);
        break;

      case 128+28:    /* RF5_BLIND */
        if (!direct) break;
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s casts a spell, burning your eyes!", m_name);
        if (p_ptr->resist_blind) {
            msg_print("You are unaffected!");
        }
        else if (rand_int(100) < p_ptr->skill_sav) {
            msg_print("You resist the effects!");
        }
        else {
            (void)set_blind(12 + rand_int(4));
        }
        update_smart_learn(m_idx, DRS_BLIND);
        break;

      case 128+29:    /* RF5_CONF */
        if (!direct) break;
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles, and you hear puzzling noises.", m_name);
        else msg_format("%^s creates a mesmerising illusion.", m_name);
        if (p_ptr->resist_conf) {
            msg_print("You disbelieve the feeble spell.");
        }
        else if (rand_int(100) < p_ptr->skill_sav) {
            msg_print("You disbelieve the feeble spell.");
        }
        else {
            (void)set_confused(p_ptr->confused + rand_int(4) + 4);
        }
        update_smart_learn(m_idx, DRS_CONF);
        break;

      case 128+30:    /* RF5_SLOW */
        if (!direct) break;
        disturb(1, 0);
        msg_format("%^s drains power from your muscles!", m_name);
        if (p_ptr->free_act) {
            msg_print("You are unaffected!");
        }
        else if (rand_int(100) < p_ptr->skill_sav) {
            msg_print("You resist the effects!");
        }
        else {
            (void)set_slow(p_ptr->slow + rand_int(4) + 4);
        }
        update_smart_learn(m_idx, DRS_FREE);
        break;

      case 128+31:    /* RF5_HOLD */
        if (!direct) break;
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s stares deep into your eyes!", m_name);
        if (p_ptr->free_act) {
            msg_print("You are unaffected!");
        }
        else if (rand_int(100) < p_ptr->skill_sav) {
            msg_format("You resist the effects!");
        }
        else {
            (void)set_paralyzed(p_ptr->paralyzed + rand_int(4) + 4);
        }
        update_smart_learn(m_idx, DRS_FREE);
        break;



      case 160+0:    /* RF6_HASTE */
        disturb(1, 0);
        if (blind) {
            msg_format("%^s mumbles.", m_name);
        }
        else {
            msg_format("%^s concentrates on %s body.", m_name, m_poss);
        }

        /* Allow quick speed increases to base+10 */
        if (m_ptr->mspeed < r_ptr->speed + 10) {
            msg_format("%^s starts moving faster.", m_name);
            m_ptr->mspeed += 10;
        }

        /* Allow small speed increases to base+20 */
        else if (m_ptr->mspeed < r_ptr->speed + 20) {
            msg_format("%^s starts moving faster.", m_name);
            m_ptr->mspeed += 2;
        }

        break;

      case 160+1:    /* RF6_XXX1X6 */
        break;

      case 160+2:    /* RF6_HEAL */

        disturb(1, 0);

        /* Message */
        if (blind) {
            msg_format("%^s mumbles.", m_name);
        }
        else {
            msg_format("%^s concentrates on %s wounds.", m_name, m_poss);
        }

        /* Heal some */
        m_ptr->hp += (rlev * 6);

        /* Fully healed */
        if (m_ptr->hp >= m_ptr->maxhp) {

            /* Fully healed */
            m_ptr->hp = m_ptr->maxhp;

            /* Message */
            if (seen) {
                msg_format("%^s looks REALLY healthy!", m_name);
            }
            else {
                msg_format("%^s sounds REALLY healthy!", m_name);
            }
        }

        /* Partially healed */
        else {

            /* Message */
            if (seen) {
                msg_format("%^s looks healthier.", m_name);
            }
            else {
                msg_format("%^s sounds healthier.", m_name);
            }
        }

        /* Redraw (later) if needed */
        if (health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

        /* Cancel fear */
        if (m_ptr->monfear) {

            /* Cancel fear */
            m_ptr->monfear = 0;

            /* Message */
            msg_format("%^s recovers %s courage.", m_name, m_poss);
        }

        break;

      case 160+3:    /* RF6_XXX2X6 */
        break;

      case 160+4:    /* RF6_BLINK */
        disturb(1, 0);
        msg_format("%^s blinks away.", m_name);
        teleport_away(m_idx, 10);
        break;

      case 160+5:    /* RF6_TPORT */
        disturb(1, 0);
        msg_format("%^s teleports away.", m_name);
        teleport_away(m_idx, MAX_SIGHT * 2 + 5);
        break;

      case 160+6:    /* RF6_XXX3X6 */
        break;

      case 160+7:    /* RF6_XXX4X6 */
        break;

      case 160+8:    /* RF6_TELE_TO */
        if (!direct) break;
        disturb(1, 0);
        msg_format("%^s commands you to return.", m_name);
        teleport_flag = TRUE;
        teleport_dist = 0;
        teleport_to_y = m_ptr->fy;
        teleport_to_x = m_ptr->fx;
        break;

      case 160+9:    /* RF6_TELE_AWAY */
        if (!direct) break;
        disturb(1, 0);
        msg_format("%^s teleports you away.", m_name);
        teleport_flag = TRUE;
        teleport_dist = 100;
        break;

      case 160+10:    /* RF6_TELE_LEVEL */
        if (!direct) break;
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles strangely.", m_name);
        else msg_format("%^s gestures at your feet.", m_name);
        if (p_ptr->resist_nexus) {
            msg_print("You are unaffected!");
        }
        else if (rand_int(100) < p_ptr->skill_sav) {
            msg_print("You resist the effects!");
        }
        else {
            tele_level();
        }
        update_smart_learn(m_idx, DRS_NEXUS);
        break;

      case 160+11:    /* RF6_XXX5 */
        break;

      case 160+12:    /* RF6_DARKNESS */
        if (!direct) break;
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s gestures in shadow.", m_name);
        (void)unlite_area(0, 3);
        break;

      case 160+13:    /* RF6_TRAPS */
        if (!direct) break;
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles, and then cackles evilly.", m_name);
        else msg_format("%^s casts a spell and cackles evilly.", m_name);
        (void)trap_creation();
        break;

      case 160+14:    /* RF6_FORGET */
        if (!direct) break;
        disturb(1, 0);
        msg_format("%^s tries to blank your mind.", m_name);

        if (rand_int(100) < p_ptr->skill_sav) {
            msg_print("You resist the effects!");
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
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s magically summons help!", m_name);
        for (k = 0; k < 1; k++) {
            count += summon_specific(y, x, rlev, 0);
        }
        if (blind && count) msg_print("You hear something appear nearby.");
        break;

      case 160+19:    /* RF6_S_MONSTERS */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s magically summons monsters!", m_name);
        for (k = 0; k < 8; k++) {
            count += summon_specific(y, x, rlev, 0);
        }
        if (blind && count) msg_print("You hear many things appear nearby.");
        break;

      case 160+20:    /* RF6_S_ANT */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s magically summons ants.", m_name);
        for (k = 0; k < 6; k++) {
            count += summon_specific(y, x, rlev, SUMMON_ANT);
        }
        if (blind && count) msg_print("You hear many things appear nearby.");
        break;

      case 160+21:    /* RF6_S_SPIDER */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s magically summons spiders.", m_name);
        for (k = 0; k < 6; k++) {
            count += summon_specific(y, x, rlev, SUMMON_SPIDER);
        }
        if (blind && count) msg_print("You hear many things appear nearby.");
        break;

      case 160+22:    /* RF6_S_HOUND */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s magically summons hounds.", m_name);
        for (k = 0; k < 6; k++) {
            count += summon_specific(y, x, rlev, SUMMON_HOUND);
        }
        if (blind && count) msg_print("You hear many things appear nearby.");
        break;

      case 160+23:    /* RF6_S_HYDRA */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s magically summons hydras.", m_name);
        for (k = 0; k < 6; k++) {
            count += summon_specific(y, x, rlev, SUMMON_HYDRA);
        }
        if (blind && count) msg_print("You hear many things appear nearby.");
        break;

      case 160+24:    /* RF6_S_ANGEL */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s magically summons an angel!", m_name);
        for (k = 0; k < 1; k++) {
            count += summon_specific(y, x, rlev, SUMMON_ANGEL);
        }
        if (blind && count) msg_print("You hear something appear nearby.");
        break;

      case 160+25:    /* RF6_S_DEMON */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s magically summons a hellish adversary!", m_name);
        for (k = 0; k < 1; k++) {
            count += summon_specific(y, x, rlev, SUMMON_DEMON);
        }
        if (blind && count) msg_print("You hear something appear nearby.");
        break;

      case 160+26:    /* RF6_S_UNDEAD */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s magically summons an undead adversary!", m_name);
        for (k = 0; k < 1; k++) {
            count += summon_specific(y, x, rlev, SUMMON_UNDEAD);
        }
        if (blind && count) msg_print("You hear something appear nearby.");
        break;

      case 160+27:    /* RF6_S_DRAGON */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s magically summons a dragon!", m_name);
        for (k = 0; k < 1; k++) {
            count += summon_specific(y, x, rlev, SUMMON_DRAGON);
        }
        if (blind && count) msg_print("You hear something appear nearby.");
        break;

      case 160+28:    /* RF6_S_HI_UNDEAD */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s magically summons greater undead!", m_name);
        for (k = 0; k < 8; k++) {
            count += summon_specific(y, x, rlev, SUMMON_HI_UNDEAD);
        }
        if (blind && count) {
            msg_print("You hear many creepy things appear nearby.");
        }
        break;

      case 160+29:    /* RF6_S_HI_DRAGON */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s magically summons ancient dragons!", m_name);
        for (k = 0; k < 8; k++) {
            count += summon_specific(y, x, rlev, SUMMON_HI_DRAGON);
        }
        if (blind && count) {
            msg_print("You hear many powerful things appear nearby.");
        }
        break;

      case 160+30:    /* RF6_S_WRAITH */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s magically summons mighty undead opponents!", m_name);
        for (k = 0; k < 8; k++) {
            count += summon_specific(y, x, rlev, SUMMON_WRAITH);
        }
        for (k = 0; k < 8; k++) {
            count += summon_specific(y, x, rlev, SUMMON_HI_UNDEAD);
        }
        if (blind && count) {
            msg_print("You hear many creepy things appear nearby.");
        }
        break;

      case 160+31:    /* RF6_S_UNIQUE */
        disturb(1, 0);
        if (blind) msg_format("%^s mumbles.", m_name);
        else msg_format("%^s magically summons special opponents!", m_name);
        for (k = 0; k < 8; k++) {
            count += summon_specific(y, x, rlev, SUMMON_UNIQUE);
        }
        for (k = 0; k < 8; k++) {
            count += summon_specific(y, x, rlev, SUMMON_HI_UNDEAD);
        }
        if (blind && count) {
            msg_print("You hear many powerful things appear nearby.");
        }
        break;


      default:
        disturb(1, 0);
        msg_format("Oops. %^s casts a buggy spell.", m_name);
    }


    /* Remember what the monster did to us */
    if (seen) {

        /* Inate spell */
        if (thrown_spell < 32*4) {
            r_ptr->r_flags4 |= (1L << (thrown_spell - 32*3));
            if (r_ptr->r_cast_inate < MAX_UCHAR) r_ptr->r_cast_inate++;
        }

        /* Bolt or Ball */
        else if (thrown_spell < 32*5) {
            r_ptr->r_flags5 |= (1L << (thrown_spell - 32*4));
            if (r_ptr->r_cast_spell < MAX_UCHAR) r_ptr->r_cast_spell++;
        }

        /* Special spell */
        else if (thrown_spell < 32*6) {
            r_ptr->r_flags6 |= (1L << (thrown_spell - 32*5));
            if (r_ptr->r_cast_spell < MAX_UCHAR) r_ptr->r_cast_spell++;
        }
    }


    /* Always take note of monsters that kill you */
    if (death && (r_ptr->r_deaths < MAX_SHORT)) r_ptr->r_deaths++;


    /* A spell was cast */
    return (TRUE);
}


