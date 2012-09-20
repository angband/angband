/* File: melee2.c */

/* Purpose: Monster spells and movement */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"



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

        /* Know poison info */
        if (p_ptr->resist_pois) smart |= SM_RES_POIS;
        if (p_ptr->oppose_pois) smart |= SM_OPP_POIS;

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


    if ((smart & SM_OPP_POIS) && (smart & SM_RES_POIS)) {
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


#endif


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
 * XXX XXX XXX Note that several effects should really not be "seen"
 * if the player is blind.  See also "effects.c" for other "mistakes".
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
            take_hit(damroll(15, 15), ddesc);
            (void)set_cut(p_ptr->cut + damroll(10, 10));
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
        teleport_player_to(m_ptr->fy, m_ptr->fx);
        break;

      case 160+9:    /* RF6_TELE_AWAY */
        if (!direct) break;
        disturb(1, 0);
        msg_format("%^s teleports you away.", m_name);
        teleport_player(100);
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
                    /* XXX XXX XXX XXX Old test (pval 10 to 20) */
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
                /* XXX XXX XXX XXX Old test (pval 10 to 20) */
                if (randint((m_ptr->hp + 1) * (50 + i_ptr->pval)) <
                    40 * (m_ptr->hp - 10 - i_ptr->pval))
#endif

                /* Attempt to Bash */
                if (rand_int(m_ptr->hp / 10) > (c_ptr->feat & 0x07)) {

                    /* Message */
                    msg_print("You hear a door burst open!");
                        
                    /* Disturb (sometimes) */
                    if (disturb_other) disturb(0, 0);

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
                if (c_ptr->feat & CAVE_MARK) {
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

            /* Possible disturb */
            if (m_ptr->ml &&
                (disturb_move ||
                 (m_ptr->los &&
                  disturb_near))) {

                /* Disturb */
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
                            msg_format("%^s tries to pick up %s, but fails.",
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



