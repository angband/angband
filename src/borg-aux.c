/* File: borg-aux.c */

/* Purpose: Helper file for "borg-ben.c" -BEN- */

#include "angband.h"

#ifdef ALLOW_BORG

#include "borg.h"

#include "borg-obj.h"

#include "borg-map.h"

#include "borg-ext.h"

#include "borg-aux.h"



/*
 * See "borg-ben.c" for more information.
 *
 * Problems:
 *   Use "time stamps" (and not "random" checks) for several routines,
 *   including "kill junk" and "wear stuff", and maybe even "free space".
 *   But be careful with the "free space" routine, wear stuff first.
 *   Make sure nothing is "destroyed" if we do not do them every turn.
 *   Consider some special routines in stores (and in the home).
 *
 * Note that we assume that any item with quantity zero does not exist,
 * thus, when simulating possible worlds, we do not actually have to
 * "optimize" empty slots.
 *
 * Hack -- We should perhaps consider wearing "harmless" items into empty
 * slots when in the dungeon, to allow rings/amulets to be brought back up
 * to town to be sold.
 *
 * We should take account of possible combinations of equipment.  This may
 * be a potentially expensive computation, but could be done occasionally.
 * It is important to use a "state-less" formula to allow the exchange to
 * be spread over multiple turns.
 *
 * Hack -- We should attempt to only collect non-discounted items, at least
 * for the "expensive" slots, such as books, since we do not want to lose
 * value due to stacking.  We seem to sell the non-discounted items first,
 * and to buy the discounted items first, since they are cheap.  Oh well,
 * we may just be stuck with using discounted books.  Unless we actually
 * do correct "combining" in the simulations, and reward free slots.  Ick!
 *
 * XXX XXX XXX We really need a better "twitchy" function.
 *
 * XXX XXX XXX We need a better "flee this level" function
 *
 * XXX XXX XXX We need to stockpile possible useful items at home.
 *
 * XXX XXX XXX Perhaps we could simply maintain a list of abilities
 * that we might need at some point, such as the ability to identify, and
 * simply allow the Borg to "sell" items to the home which satisfy this
 * desire for "abilities".
 *
 * XXX XXX XXX Also, we should probably attempt to track the "best" item
 * in the home for each equipment slot, using some form of heuristic, and
 * reward that item based on its power, so that the Borg would always
 * have a "backup" item in case of disenchantment.
 *
 * XXX XXX XXX Also, we could reward equipment based on possible enchantment,
 * up to the maximal amount available in the home, which would induce item
 * switching when the item could be enchanted sufficiently.
 *
 * XXX XXX XXX One problem is that if the Borg suddenly becomes "unprepared"
 * for the current level, and decides to "flee" the level by taking stairs
 * down to a deeper level, then he will get stuck in a possibly infinite
 * "fleeing" loop.  We should also attempt to allow "fleeing" to a location
 * which may be safe for the short term, to allow the Borg to rest.  Also,
 * fleeing from fast spell-casters is probably not a very smart idea.  Also,
 * fleeing from fast monsters in general is rather stupid.  Watching the Borg
 * flee from an Ochre Jelly is a good example of this stupidity.  XXX XXX XXX
 */



/*
 * Safety arrays for simulating possible worlds
 */
 
auto_item *safe_items;		/* Safety "inventory" */

auto_shop *safe_shops;		/* Safety "shops" */


/*
 * Hack -- importance of the various "level feelings"
 * Try to explore the level for at least this many turns
 */
static s16b value_feeling[] = {
    500,
    8000,
    8000,
    6000,
    4000,
    2000,
    1000,
    800,
    600,
    400,
    200,
    0
};




/*
 * The "notice" functions examine various aspects of the player inventory,
 * the player equipment, or the home contents, and extract various numerical
 * quantities based on those aspects, adjusting them for various "abilities",
 * such as the ability to cast certain spells, etc.
 *
 * The "power" functions use the numerical quantities described above, and
 * use them to do two different things:  (1) rank the "value" of having
 * various abilities relative to the possible "money" reward of carrying
 * sellable items instead, and (2) rank the value of various abilities
 * relative to each other, which is used to determine what to wear/buy,
 * and in what order to wear/buy those items.
 *
 * These functions use some very heuristic values, by the way...
 *
 * We should probably take account of things like possible enchanting
 * (especially when in town), and items which may be found soon.
 *
 * We consider several things:
 *   (1) the actual "power" of the current weapon and bow
 *   (2) the various "flags" imparted by the equipment
 *   (3) the various abilities imparted by the equipment
 *   (4) the penalties induced by heavy armor or gloves or edged weapons
 *   (5) the abilities required to enter the "max_depth" dungeon level
 *   (6) the various abilities of some useful inventory items
 *
 * Note the use of special "item counters" for evaluating the value of
 * a collection of items of the given type.  Basically, the first item
 * of the given type is always the most valuable, with subsequent items
 * being worth less, until the "limit" is reached, after which point any
 * extra items are only worth as much as they can be sold for.
 */
 
 
 
/*
 * Helper function -- notice the player equipment
 */
static void borg_notice_aux1(void)
{
    int			i, hold;

    int			extra_blows = 0;

    int			extra_shots = 0;
    int			extra_might = 0;
    
    int			stat_add[6];
        
    auto_item		*item;


    /* Clear the stat modifiers */
    for (i = 0; i < 6; i++) stat_add[i] = 0;


    /* Clear the armor class */
    my_ac = 0;
    
    /* Clear the bonuses */
    my_to_hit = my_to_dam = my_to_ac = 0;


    /* Start with a single blow per turn */
    my_num_blow = 1;

    /* Start with a single shot per turn */
    my_num_fire = 1;


    /* Assume normal view radius */
    my_cur_view = MAX_SIGHT;
    
    /* Assume normal lite radius */
    my_cur_lite = 0;


    /* Assume normal speed */
    my_speed = 110;
    
    /* Assume normal other */
    my_other = 0;


    /* Reset the "ammo" tval */
    my_ammo_tval = 0;

    /* Reset the "ammo" sides */
    my_ammo_sides = 0;
    
    /* Reset the shooting power */
    my_ammo_power = 0;
    
    /* Reset the shooting range */
    my_ammo_range = 0;


    /* Clear all the flags */
    my_see_inv = FALSE;
    my_teleport = FALSE;
    my_free_act = FALSE;
    my_slow_digest = FALSE;
    my_aggravate = FALSE;
    my_regenerate = FALSE;
    my_ffall = FALSE;
    my_hold_life = FALSE;
    my_telepathy = FALSE;
    my_lite = FALSE;
    
    my_immune_acid = FALSE;
    my_immune_elec = FALSE;
    my_immune_fire = FALSE;
    my_immune_cold = FALSE;

    my_resist_acid = FALSE;
    my_resist_elec = FALSE;
    my_resist_fire = FALSE;
    my_resist_cold = FALSE;
    my_resist_pois = FALSE;
    my_resist_conf = FALSE;
    my_resist_sound = FALSE;
    my_resist_lite = FALSE;
    my_resist_dark = FALSE;
    my_resist_chaos = FALSE;
    my_resist_disen = FALSE;
    my_resist_shard = FALSE;
    my_resist_nexus = FALSE;
    my_resist_blind = FALSE;
    my_resist_neth = FALSE;
    my_resist_fear = FALSE;
    
    my_sustain_str = FALSE;
    my_sustain_int = FALSE;
    my_sustain_wis = FALSE;
    my_sustain_con = FALSE;
    my_sustain_dex = FALSE;
    my_sustain_chr = FALSE;
    

    /* Base infravision (purely racial) */
    my_see_infra = rb_ptr->infra;


    /* Base skill -- disarming */
    my_skill_dis = rb_ptr->r_dis + cb_ptr->c_dis;

    /* Base skill -- magic devices */
    my_skill_dev = rb_ptr->r_dev + cb_ptr->c_dev;

    /* Base skill -- saving throw */
    my_skill_sav = rb_ptr->r_sav + cb_ptr->c_sav;

    /* Base skill -- stealth */
    my_skill_stl = rb_ptr->r_stl + cb_ptr->c_stl;

    /* Base skill -- searching ability */
    my_skill_srh = rb_ptr->r_srh + cb_ptr->c_srh;

    /* Base skill -- searching frequency */
    my_skill_fos = rb_ptr->r_fos + cb_ptr->c_fos;

    /* Base skill -- combat (normal) */
    my_skill_thn = rb_ptr->r_thn + cb_ptr->c_thn;

    /* Base skill -- combat (shooting) */
    my_skill_thb = rb_ptr->r_thb + cb_ptr->c_thb;

    /* Base skill -- combat (throwing) */
    my_skill_tht = rb_ptr->r_thb + cb_ptr->c_thb;

    /* Base skill -- Digging */
    my_skill_dig = 0;


    /* Elf */
    if (auto_race == RACE_ELF) my_resist_lite = TRUE;

    /* Hobbit */
    if (auto_race == RACE_HOBBIT) my_sustain_dex = TRUE;

    /* Gnome */
    if (auto_race == RACE_GNOME) my_free_act = TRUE;

    /* Dwarf */
    if (auto_race == RACE_DWARF) my_resist_blind = TRUE;

    /* Half-Orc */
    if (auto_race == RACE_HALF_ORC) my_resist_dark = TRUE;

    /* Half-Troll */
    if (auto_race == RACE_HALF_TROLL) my_sustain_str = TRUE;

    /* Dunadan */
    if (auto_race == RACE_DUNADAN) my_sustain_con = TRUE;

    /* High Elf */
    if (auto_race == RACE_HIGH_ELF) my_resist_lite = TRUE;
    if (auto_race == RACE_HIGH_ELF) my_see_inv = TRUE;


    /* Scan the usable inventory */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {

        item = &auto_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;
        
        /* Affect stats */
        if (item->flags1 & TR1_STR) stat_add[A_STR] += item->pval;
        if (item->flags1 & TR1_INT) stat_add[A_INT] += item->pval;
        if (item->flags1 & TR1_WIS) stat_add[A_WIS] += item->pval;
        if (item->flags1 & TR1_DEX) stat_add[A_DEX] += item->pval;
        if (item->flags1 & TR1_CON) stat_add[A_CON] += item->pval;
        if (item->flags1 & TR1_CHR) stat_add[A_CHR] += item->pval;

        /* Affect infravision */
        if (item->flags1 & TR1_INFRA) my_see_infra += item->pval;

        /* Affect stealth */
        if (item->flags1 & TR1_STEALTH) my_skill_stl += item->pval;

        /* Affect searching ability (factor of five) */
        if (item->flags1 & TR1_SEARCH) my_skill_srh += (item->pval * 5);

        /* Affect searching frequency (factor of five) */
        if (item->flags1 & TR1_SEARCH) my_skill_fos += (item->pval * 5);

        /* Affect digging (factor of 20) */
        if (item->flags1 & TR1_TUNNEL) p_ptr->skill_dig += (item->pval * 20);

        /* Affect speed */
        if (item->flags1 & TR1_SPEED) my_speed += item->pval;

        /* Affect blows */
        if (item->flags1 & TR1_BLOWS) extra_blows += item->pval;

        /* Boost shots */
        if (item->flags3 & TR3_XTRA_SHOTS) extra_shots++;

        /* Boost might */
        if (item->flags3 & TR3_XTRA_MIGHT) extra_might++;

        /* Various flags */
        if (item->flags3 & TR3_SLOW_DIGEST) my_slow_digest = TRUE;
        if (item->flags3 & TR3_AGGRAVATE) my_aggravate = TRUE;
        if (item->flags3 & TR3_TELEPORT) my_teleport = TRUE;
        if (item->flags3 & TR3_REGEN) my_regenerate = TRUE;
        if (item->flags3 & TR3_TELEPATHY) my_telepathy = TRUE;
        if (item->flags3 & TR3_LITE) my_lite = TRUE;
        if (item->flags3 & TR3_SEE_INVIS) my_see_inv = TRUE;
        if (item->flags3 & TR3_FEATHER) my_ffall = TRUE;
        if (item->flags2 & TR2_FREE_ACT) my_free_act = TRUE;
        if (item->flags2 & TR2_HOLD_LIFE) my_hold_life = TRUE;

        /* Immunity flags */
        if (item->flags2 & TR2_IM_FIRE) my_immune_fire = TRUE;
        if (item->flags2 & TR2_IM_ACID) my_immune_acid = TRUE;
        if (item->flags2 & TR2_IM_COLD) my_immune_cold = TRUE;
        if (item->flags2 & TR2_IM_ELEC) my_immune_elec = TRUE;

        /* Resistance flags */
        if (item->flags2 & TR2_RES_ACID) my_resist_acid = TRUE;
        if (item->flags2 & TR2_RES_ELEC) my_resist_elec = TRUE;
        if (item->flags2 & TR2_RES_FIRE) my_resist_fire = TRUE;
        if (item->flags2 & TR2_RES_COLD) my_resist_cold = TRUE;
        if (item->flags2 & TR2_RES_POIS) my_resist_pois = TRUE;
        if (item->flags2 & TR2_RES_CONF) my_resist_conf = TRUE;
        if (item->flags2 & TR2_RES_SOUND) my_resist_sound = TRUE;
        if (item->flags2 & TR2_RES_LITE) my_resist_lite = TRUE;
        if (item->flags2 & TR2_RES_DARK) my_resist_dark = TRUE;
        if (item->flags2 & TR2_RES_CHAOS) my_resist_chaos = TRUE;
        if (item->flags2 & TR2_RES_DISEN) my_resist_disen = TRUE;
        if (item->flags2 & TR2_RES_SHARDS) my_resist_shard = TRUE;
        if (item->flags2 & TR2_RES_NEXUS) my_resist_nexus = TRUE;
        if (item->flags2 & TR2_RES_BLIND) my_resist_blind = TRUE;
        if (item->flags2 & TR2_RES_NETHER) my_resist_neth = TRUE;

        /* Sustain flags */
        if (item->flags2 & TR2_SUST_STR) my_sustain_str = TRUE;
        if (item->flags2 & TR2_SUST_INT) my_sustain_int = TRUE;
        if (item->flags2 & TR2_SUST_WIS) my_sustain_wis = TRUE;
        if (item->flags2 & TR2_SUST_DEX) my_sustain_dex = TRUE;
        if (item->flags2 & TR2_SUST_CON) my_sustain_con = TRUE;
        if (item->flags2 & TR2_SUST_CHR) my_sustain_chr = TRUE;

        /* Modify the base armor class */
        my_ac += item->ac;

        /* Apply the bonuses to armor class */
        my_to_ac += item->to_a;

        /* Hack -- do not apply "weapon" bonuses */
        if (i == INVEN_WIELD) continue;

        /* Hack -- do not apply "bow" bonuses */
        if (i == INVEN_BOW) continue;

        /* Apply the bonuses to hit/damage */
        my_to_hit += item->to_h;
        my_to_dam += item->to_d;
    }

    /* Hack -- Chaos / Confusion */
    if (my_resist_chaos) my_resist_conf = TRUE;


    /* Update "stats" */
    for (i = 0; i < 6; i++) {

        int use, ind;
        
        /* Extract the new "use_stat" value for the stat */
        use = modify_stat_value(my_stat_cur[i], stat_add[i]);

        /* Save the stat */
        my_stat_use[i] = use;

        /* Values: 3, ..., 17 */
        if (use <= 17) ind = (use - 3);

        /* Ranges: 18/00-18/09, ..., 18/210-18/219 */
        else if (use <= 18+219) ind = (15 + (use - 18) / 10);

        /* Range: 18/220+ */
        else ind = (37);

        /* Save the index */
        my_stat_ind[i] = ind;
    }


#if 0

    /* XXX XXX XXX */
    
    /* Extract the current weight (in tenth pounds) */
    j = inven_weight;

    /* Extract the "weight limit" (in tenth pounds) */
    i = weight_limit();

    /* XXX Hack -- Apply "encumbrance" from weight */
    if (j > i/2) my_speed -= ((j - (i/2)) / (i / 10));

#endif

    /* Bloating slows the player down (a little) */
    if (do_gorged) my_speed -= 10;



    /* Actual Modifier Bonuses (Un-inflate stat bonuses) */
    my_to_ac += ((int)(adj_dex_ta[my_stat_ind[A_DEX]]) - 128);
    my_to_dam += ((int)(adj_str_td[my_stat_ind[A_STR]]) - 128);
    my_to_hit += ((int)(adj_dex_th[my_stat_ind[A_DEX]]) - 128);
    my_to_hit += ((int)(adj_str_th[my_stat_ind[A_STR]]) - 128);


    /* Examine the lite */
    item = &auto_items[INVEN_LITE];

    /* Glowing player has light */
    if (my_lite) my_cur_lite = 1;
    
    /* Lite */
    if (item->tval == TV_LITE) {

        /* Torches -- radius one */
        if (item->sval == SV_LITE_TORCH) my_cur_lite = 1;

        /* Lanterns -- radius two */
        if (item->sval == SV_LITE_LANTERN) my_cur_lite = 2;

        /* No fuel means no radius */
        if (!item->pval) my_cur_lite = 0;

        /* Artifact lites -- radius three */
        if (item->name1) my_cur_lite = 3;

        /* Artifact lites -- assume glowing */
        if (item->name1) my_lite = TRUE;
    }


    /* Obtain the "hold" value */
    hold = adj_str_hold[my_stat_ind[A_STR]];


    /* Examine the "current bow" */
    item = &auto_items[INVEN_BOW];

    /* It is hard to carholdry a heavy bow */
    if (hold < item->weight / 10) {

        /* Hard to wield a heavy bow */
        my_to_hit += 2 * (hold - item->weight / 10);
    }

    /* Compute "extra shots" if needed */
    if (item->iqty && (hold >= item->weight / 10)) {

        /* Take note of required "tval" for missiles */
        switch (item->sval) {

            case SV_SLING:
                my_ammo_tval = TV_SHOT;
                my_ammo_sides = 3;
                my_ammo_power = 2;
                break;

            case SV_SHORT_BOW:
                my_ammo_tval = TV_ARROW;
                my_ammo_sides = 4;
                my_ammo_power = 2;
                break;

            case SV_LONG_BOW:
                my_ammo_tval = TV_ARROW;
                my_ammo_sides = 4;
                my_ammo_power = 3;
                break;

            case SV_LIGHT_XBOW:
                my_ammo_tval = TV_BOLT;
                my_ammo_sides = 5;
                my_ammo_power = 3;
                break;

            case SV_HEAVY_XBOW:
                my_ammo_tval = TV_BOLT;
                my_ammo_sides = 5;
                my_ammo_power = 4;
                break;
        }

        /* Add in extra power */
        my_ammo_power += extra_might;
        
        /* Calculate total range */
        my_ammo_range = 10 + my_ammo_power * 5;
        
        /* Hack -- High Level Rangers shoot arrows quickly */
        if ((auto_class == 4) && (my_ammo_tval == TV_ARROW)) {

            /* Extra shot at level 20 */
            if (auto_max_level >= 20) my_num_fire++;

            /* Extra shot at level 40 */
            if (auto_max_level >= 40) my_num_fire++;
        }

        /* Add in the "bonus shots" */
        my_num_fire += extra_shots;

        /* Require at least one shot */
        if (my_num_fire < 1) my_num_fire = 1;
    }



    /* Examine the "main weapon" */
    item = &auto_items[INVEN_WIELD];

    /* It is hard to hold a heavy weapon */
    if (hold < item->weight / 10) {

        /* Hard to wield a heavy weapon */
        my_to_hit += 2 * (hold - item->weight / 10);
    }

    /* Normal weapons */
    if (item->iqty && (hold >= item->weight / 10)) {

        int str_index, dex_index;

        int num = 0, wgt = 0, mul = 0, div = 0;

        /* Analyze the class */
        switch (auto_class) {

            /* Warrior */
            case 0: num = 6; wgt = 30; mul = 5; break;

            /* Mage */
            case 1: num = 4; wgt = 40; mul = 2; break;

            /* Priest (was mul = 3.5) */
            case 2: num = 5; wgt = 35; mul = 3; break;

            /* Rogue */
            case 3: num = 5; wgt = 30; mul = 3; break;

            /* Ranger */
            case 4: num = 5; wgt = 35; mul = 4; break;

            /* Paladin */
            case 5: num = 5; wgt = 30; mul = 4; break;
        }

        /* Enforce a minimum "weight" */
        div = ((item->weight < wgt) ? wgt : item->weight);

        /* Access the strength vs weight */
        str_index = (adj_str_blow[my_stat_ind[A_STR]] * mul / div);

        /* Maximal value */
        if (str_index > 11) str_index = 11;

        /* Index by dexterity */
        dex_index = (adj_dex_blow[my_stat_ind[A_DEX]]);

        /* Maximal value */
        if (dex_index > 11) dex_index = 11;

        /* Use the blows table */
        my_num_blow = blows_table[str_index][dex_index];

        /* Maximal value */
        if (my_num_blow > num) my_num_blow = num;

        /* Add in the "bonus blows" */
        my_num_blow += extra_blows;

        /* Require at least one blow */
        if (my_num_blow < 1) my_num_blow = 1;

        /* Boost digging skill by weapon weight */
        my_skill_dig += (item->weight / 10);
    }

    /* Priest weapon penalty for non-blessed edged weapons */
    if ((auto_class == 2) &&
        ((item->tval == TV_SWORD) || (item->tval == TV_POLEARM)) &&
        (!(item->flags3 & TR3_BLESSED))) {

        /* Reduce the real bonuses */
        my_to_hit -= 2;
        my_to_dam -= 2;
    }


    /* Affect Skill -- stealth (bonus one) */
    my_skill_stl += 1;

    /* Affect Skill -- disarming (DEX and INT) */
    my_skill_dis += adj_dex_dis[my_stat_ind[A_DEX]];
    my_skill_dis += adj_int_dis[my_stat_ind[A_INT]];

    /* Affect Skill -- magic devices (INT) */
    my_skill_dev += adj_int_dev[my_stat_ind[A_INT]];

    /* Affect Skill -- saving throw (WIS) */
    my_skill_sav += adj_wis_sav[my_stat_ind[A_WIS]];

    /* Affect Skill -- digging (STR) */
    my_skill_dig += adj_str_dig[my_stat_ind[A_STR]];


    /* Affect Skill -- disarming (Level, by Class) */
    my_skill_dis += (cb_ptr->x_dis * auto_max_level / 10);

    /* Affect Skill -- magic devices (Level, by Class) */
    my_skill_dev += (cb_ptr->x_dev * auto_max_level / 10);

    /* Affect Skill -- saving throw (Level, by Class) */
    my_skill_sav += (cb_ptr->x_sav * auto_max_level / 10);

    /* Affect Skill -- stealth (Level, by Class) */
    my_skill_stl += (cb_ptr->x_stl * auto_max_level / 10);

    /* Affect Skill -- search ability (Level, by Class) */
    my_skill_srh += (cb_ptr->x_srh * auto_max_level / 10);

    /* Affect Skill -- search frequency (Level, by Class) */
    my_skill_fos += (cb_ptr->x_fos * auto_max_level / 10);

    /* Affect Skill -- combat (normal) (Level, by Class) */
    my_skill_thn += (cb_ptr->x_thn * auto_max_level / 10);

    /* Affect Skill -- combat (shooting) (Level, by Class) */
    my_skill_thb += (cb_ptr->x_thb * auto_max_level / 10);

    /* Affect Skill -- combat (throwing) (Level, by Class) */
    my_skill_tht += (cb_ptr->x_thb * auto_max_level / 10);

    /* Limit Skill -- stealth from 0 to 30 */
    if (my_skill_stl > 30) my_skill_stl = 30;
    if (my_skill_stl < 0) my_skill_stl = 0;

    /* Limit Skill -- digging from 1 up */
    if (my_skill_dig < 1) my_skill_dig = 1;


    /*** Count needed enchantment ***/

    /* Assume no enchantment needed */
    auto_need_enchant_to_a = 0;
    auto_need_enchant_to_h = 0;
    auto_need_enchant_to_d = 0;
    
    /* Hack -- enchant all the equipment (weapons) */
    for (i = INVEN_WIELD; i <= INVEN_BOW; i++) {

        item = &auto_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;
        
        /* Skip "unknown" items */
        if (!item->able) continue;

        /* Enchant all weapons (to hit) */
        if (item->to_h < 8) {
            auto_need_enchant_to_h += (8 - item->to_h);
        }

        /* Enchant all weapons (to damage) */
        if (item->to_d < 8) {
            auto_need_enchant_to_d += (8 - item->to_d);
        }
    }
    
    /* Hack -- enchant all the equipment (armor) */
    for (i = INVEN_BODY; i <= INVEN_FEET; i++) {

        item = &auto_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;
        
        /* Skip "unknown" items */
        if (!item->able) continue;

        /* Note need for enchantment */
        if (item->to_a < 8) {
            auto_need_enchant_to_a += (8 - item->to_a);
        }
    }
}


/*
 * Helper function -- notice the player inventory
 */
static void borg_notice_aux2(void)
{
    int i;

    auto_item *item;
    

    /*** Reset counters ***/
    
    /* Reset basic */
    amt_fuel = 0;
    amt_food = 0;
    amt_ident = 0;
    amt_recall = 0;
    amt_escape = 0;
    amt_teleport = 0;
    
    /* Reset healing */
    amt_cure_critical = 0;
    amt_cure_serious = 0;

    /* Reset detection */
    amt_detect_trap = 0;
    amt_detect_door = 0;

    /* Reset missiles */
    amt_missile = 0;

    /* Reset books */
    amt_book[0] = 0;
    amt_book[1] = 0;
    amt_book[2] = 0;
    amt_book[3] = 0;
    amt_book[4] = 0;
    amt_book[5] = 0;
    amt_book[6] = 0;
    amt_book[7] = 0;
    amt_book[8] = 0;
    
    /* Reset various */
    amt_add_stat[A_STR] = 0;
    amt_add_stat[A_INT] = 0;
    amt_add_stat[A_WIS] = 0;
    amt_add_stat[A_DEX] = 0;
    amt_add_stat[A_CON] = 0;
    amt_add_stat[A_CHR] = 0;
    amt_fix_stat[A_STR] = 0;
    amt_fix_stat[A_INT] = 0;
    amt_fix_stat[A_WIS] = 0;
    amt_fix_stat[A_DEX] = 0;
    amt_fix_stat[A_CON] = 0;
    amt_fix_stat[A_CHR] = 0;
    amt_fix_exp = 0;
    
    /* Reset enchantment */
    amt_enchant_to_a = 0;
    amt_enchant_to_d = 0;
    amt_enchant_to_h = 0;


    /*** Process the inventory ***/

    /* Scan the inventory */
    for (i = 0; i < INVEN_PACK; i++) {

        item = &auto_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Hack -- skip un-aware items */
        if (!item->kind) continue;


        /* Analyze the item */
        switch (item->tval) {

            /* Books */
            case TV_MAGIC_BOOK:
            case TV_PRAYER_BOOK:
            
                /* Skip incorrect books */
                if (item->tval != mb_ptr->spell_book) break;
                
                /* Count the books */
                amt_book[item->sval] += item->iqty;
    
                break;
                        

            /* Food */
            case TV_FOOD:

                /* Analyze */
                switch (item->sval) {

                    case SV_FOOD_RATION:
                        amt_food += item->iqty;
                        break;
                }

                break;


            /* Potions */
            case TV_POTION:

                /* Analyze */
                switch (item->sval) {

                    case SV_POTION_CURE_CRITICAL:
                        amt_cure_critical += item->iqty;
                        break;
                        
                    case SV_POTION_CURE_SERIOUS:
                        amt_cure_serious += item->iqty;
                        break;
                        
                    case SV_POTION_INC_STR:
                        amt_add_stat[A_STR] += item->iqty;
                        break;

                    case SV_POTION_INC_INT:
                        amt_add_stat[A_INT] += item->iqty;
                        break;

                    case SV_POTION_INC_WIS:
                        amt_add_stat[A_WIS] += item->iqty;
                        break;

                    case SV_POTION_INC_DEX:
                        amt_add_stat[A_DEX] += item->iqty;
                        break;

                    case SV_POTION_INC_CON:
                        amt_add_stat[A_CON] += item->iqty;
                        break;

                    case SV_POTION_INC_CHR:
                        amt_add_stat[A_CHR] += item->iqty;
                        break;

                    case SV_POTION_RES_STR:
                        amt_fix_stat[A_STR] += item->iqty;
                        break;

                    case SV_POTION_RES_INT:
                        amt_fix_stat[A_INT] += item->iqty;
                        break;

                    case SV_POTION_RES_WIS:
                        amt_fix_stat[A_WIS] += item->iqty;
                        break;

                    case SV_POTION_RES_DEX:
                        amt_fix_stat[A_DEX] += item->iqty;
                        break;

                    case SV_POTION_RES_CON:
                        amt_fix_stat[A_CON] += item->iqty;
                        break;

                    case SV_POTION_RES_CHR:
                        amt_fix_stat[A_CHR] += item->iqty;
                        break;

                    case SV_POTION_RESTORE_EXP:
                        amt_fix_exp += item->iqty;
                        break;
                }

                break;


            /* Scrolls */
            case TV_SCROLL:

                /* Analyze the scroll */
                switch (item->sval) {

                    case SV_SCROLL_IDENTIFY:
                        amt_ident += item->iqty;
                        break;

                    case SV_SCROLL_TELEPORT:
                        amt_escape += item->iqty;
                        break;

                    case SV_SCROLL_WORD_OF_RECALL:
                        amt_recall += item->iqty;
                        break;

                    case SV_SCROLL_ENCHANT_ARMOR:
                        amt_enchant_to_a += item->iqty;
                        break;

                    case SV_SCROLL_ENCHANT_WEAPON_TO_HIT:
                        amt_enchant_to_h += item->iqty;
                        break;

                    case SV_SCROLL_ENCHANT_WEAPON_TO_DAM:
                        amt_enchant_to_d += item->iqty;
                        break;
                }

                break;
            
            
            /* Rods */
            case TV_ROD:

                /* Analyze */
                switch (item->sval) {

                    case SV_ROD_IDENTIFY:
                        amt_ident += item->iqty * 100;
                        break;

                    case SV_ROD_RECALL:
                        amt_recall += item->iqty * 100;
                        break;

                    case SV_ROD_DETECT_TRAP:
                        amt_detect_trap += item->iqty * 100;
                        break;

                    case SV_ROD_DETECT_DOOR:
                        amt_detect_door += item->iqty * 100;
                        break;
                }

                break;


            /* Staffs */
            case TV_STAFF:

                /* Analyze */
                switch (item->sval) {

                    case SV_STAFF_IDENTIFY:
                        amt_ident += item->iqty * item->pval;
                        break;
                        
                    case SV_STAFF_TELEPORTATION:
                        amt_teleport += item->iqty * item->pval;
                        break;

#if 0
                    case SV_STAFF_DETECT_TRAP:
                        amt_detect_trap += item->iqty * item->pval;
                        break;

                    case SV_STAFF_DETECT_DOOR:
                        amt_detect_door += item->iqty * item->pval;
                        break;
#endif

                }

                break;


            /* Flasks */
            case TV_FLASK:

                /* Use as fuel for lanterns */
                amt_fuel += item->iqty;
                
                /* XXX Consider use as missiles */

                break;


            /* Missiles */
            case TV_SHOT:
            case TV_ARROW:
            case TV_BOLT:

                /* Hack -- ignore invalid missiles */
                if (item->tval != my_ammo_tval) break;

                /* Hack -- ignore worthless missiles */
                if (item->value <= 0) break;
                
                /* Count them */
                amt_missile += item->iqty;

                break;
        }
    }


    /*** Process the Spells ***/

    /* Handle "satisfy hunger" -> infinite food */    
    if (borg_spell_okay(2,0) || borg_prayer_okay(1,5)) amt_food += 1000;

    /* Handle "identify" -> infinite identifies */
    if (borg_spell_okay(2,4) || borg_prayer_okay(5,2)) amt_ident += 1000;

    /* Handle "detect traps" */
    if (borg_spell_okay(0,7) || borg_prayer_okay(0,5)) amt_detect_trap += 1000;

    /* Handle "detect doors" */
    if (borg_spell_okay(0,7) || borg_prayer_okay(0,6)) amt_detect_door += 1000;


    /*** Process the Needs ***/

    /* No need for fuel */
    if (auto_items[INVEN_LITE].name1) amt_fuel += 1000;

    /* No need for stats */
    if (my_stat_cur[A_STR] >= 18+100) amt_add_stat[A_STR] += 1000;
    if (my_stat_cur[A_INT] >= 18+100) amt_add_stat[A_INT] += 1000;
    if (my_stat_cur[A_WIS] >= 18+100) amt_add_stat[A_WIS] += 1000;
    if (my_stat_cur[A_DEX] >= 18+100) amt_add_stat[A_DEX] += 1000;
    if (my_stat_cur[A_CON] >= 18+100) amt_add_stat[A_CON] += 1000;
    if (my_stat_cur[A_CHR] >= 18+100) amt_add_stat[A_CHR] += 1000;

    /* No need for stat repair */
    if (!do_fix_stat[A_STR]) amt_fix_stat[A_STR] += 1000;
    if (!do_fix_stat[A_INT]) amt_fix_stat[A_INT] += 1000;
    if (!do_fix_stat[A_WIS]) amt_fix_stat[A_WIS] += 1000;
    if (!do_fix_stat[A_DEX]) amt_fix_stat[A_DEX] += 1000;
    if (!do_fix_stat[A_CON]) amt_fix_stat[A_CON] += 1000;
    if (!do_fix_stat[A_CHR]) amt_fix_stat[A_CHR] += 1000;

    /* No need for experience repair */
    if (!do_fix_exp) amt_fix_exp += 1000;
}


/*
 * Helper function -- notice the contents of the home
 */
static void borg_notice_aux3(void)
{
    int i;

    auto_item *item;
    
    auto_shop *shop = &auto_shops[7];

    
    /*** Reset counters ***/

    /* Reset basic */
    num_fuel = 0;
    num_food = 0;
    num_ident = 0;
    num_recall = 0;
    num_escape = 0;
    num_teleport = 0;
    
    /* Reset healing */
    num_cure_critical = 0;
    num_cure_serious = 0;

    /* Reset missiles */
    num_missile = 0;

    /* Reset books */
    num_book[0] = 0;
    num_book[1] = 0;
    num_book[2] = 0;
    num_book[3] = 0;
    num_book[4] = 0;
    num_book[5] = 0;
    num_book[6] = 0;
    num_book[7] = 0;
    num_book[8] = 0;
    
    /* Reset various */
    num_fix_stat[A_STR] = 0;
    num_fix_stat[A_INT] = 0;
    num_fix_stat[A_WIS] = 0;
    num_fix_stat[A_DEX] = 0;
    num_fix_stat[A_CON] = 0;
    num_fix_stat[A_CHR] = 0;
    num_fix_exp = 0;
    
    /* Reset enchantment */
    num_enchant_to_a = 0;
    num_enchant_to_d = 0;
    num_enchant_to_h = 0;


    /*** Process the inventory ***/

    /* Scan the home */
    for (i = 0; i < 24; i++) {

        item = &shop->ware[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Hack -- skip un-aware items */
        if (!item->kind) continue;


        /* Analyze the item */
        switch (item->tval) {

            /* Books */
            case TV_MAGIC_BOOK:
            case TV_PRAYER_BOOK:
            
                /* Skip incorrect books */
                if (item->tval != mb_ptr->spell_book) break;
                
                /* Count the books */
                num_book[item->sval] += item->iqty;
    
                break;
                        

            /* Food */
            case TV_FOOD:

                /* Analyze */
                switch (item->sval) {

                    case SV_FOOD_RATION:
                        num_food += item->iqty;
                        break;
                }

                break;


            /* Potions */
            case TV_POTION:

                /* Analyze */
                switch (item->sval) {

                    case SV_POTION_CURE_CRITICAL:
                        num_cure_critical += item->iqty;
                        break;
                        
                    case SV_POTION_CURE_SERIOUS:
                        num_cure_serious += item->iqty;
                        break;

                    case SV_POTION_RES_STR:
                        num_fix_stat[A_STR] += item->iqty;
                        break;

                    case SV_POTION_RES_INT:
                        num_fix_stat[A_INT] += item->iqty;
                        break;

                    case SV_POTION_RES_WIS:
                        num_fix_stat[A_WIS] += item->iqty;
                        break;

                    case SV_POTION_RES_DEX:
                        num_fix_stat[A_DEX] += item->iqty;
                        break;

                    case SV_POTION_RES_CON:
                        num_fix_stat[A_CON] += item->iqty;
                        break;

                    case SV_POTION_RES_CHR:
                        num_fix_stat[A_CHR] += item->iqty;
                        break;

                    case SV_POTION_RESTORE_EXP:
                        num_fix_exp += item->iqty;
                        break;
                }

                break;


            /* Scrolls */
            case TV_SCROLL:

                /* Analyze the scroll */
                switch (item->sval) {

                    case SV_SCROLL_IDENTIFY:
                        num_ident += item->iqty;
                        break;

                    case SV_SCROLL_TELEPORT:
                        num_escape += item->iqty;
                        break;

                    case SV_SCROLL_WORD_OF_RECALL:
                        num_recall += item->iqty;
                        break;

                    case SV_SCROLL_ENCHANT_ARMOR:
                        num_enchant_to_a += item->iqty;
                        break;

                    case SV_SCROLL_ENCHANT_WEAPON_TO_HIT:
                        num_enchant_to_h += item->iqty;
                        break;

                    case SV_SCROLL_ENCHANT_WEAPON_TO_DAM:
                        num_enchant_to_d += item->iqty;
                        break;
                }

                break;
            
            
            /* Rods */
            case TV_ROD:

                /* Analyze */
                switch (item->sval) {

                    case SV_ROD_IDENTIFY:
                        num_ident += item->iqty * 100;
                        break;

                    case SV_ROD_RECALL:
                        num_recall += item->iqty * 100;
                        break;
                }

                break;


            /* Staffs */
            case TV_STAFF:

                /* Analyze */
                switch (item->sval) {

                    case SV_STAFF_IDENTIFY:
                        num_ident += item->iqty * item->pval;
                        break;
                        
                    case SV_STAFF_TELEPORTATION:
                        num_teleport += item->iqty * item->pval;
                        break;
                }

                break;


            /* Flasks */
            case TV_FLASK:

                /* Use as fuel for lanterns */
                num_fuel += item->iqty;
                
                break;


            /* Missiles */
            case TV_SHOT:
            case TV_ARROW:
            case TV_BOLT:

                /* Hack -- ignore invalid missiles */
                if (item->tval != my_ammo_tval) break;

                /* Hack -- ignore worthless missiles */
                if (item->value <= 0) break;
                
                /* Count them */
                num_missile += item->iqty;

                break;
        }
    }


    /*** Process the Spells ***/

    /* Handle "satisfy hunger" -> infinite food */    
    if (borg_spell_okay(2,0) || borg_prayer_okay(1,5)) num_food += 1000;

    /* Handle "identify" -> infinite identifies */
    if (borg_spell_okay(2,4) || borg_prayer_okay(5,2)) num_ident += 1000;


    /*** Process the Needs ***/

    /* No need for fuel */
    if (auto_items[INVEN_LITE].name1) num_fuel += 1000;

    /* Hack -- No need for stat repair */
    if (my_sustain_str) num_fix_stat[A_STR] += 1000;
    if (my_sustain_int) num_fix_stat[A_INT] += 1000;
    if (my_sustain_wis) num_fix_stat[A_WIS] += 1000;
    if (my_sustain_dex) num_fix_stat[A_DEX] += 1000;
    if (my_sustain_con) num_fix_stat[A_CON] += 1000;
    if (my_sustain_chr) num_fix_stat[A_CHR] += 1000;
}





/*
 * Extract various bonuses
 */
static void borg_notice(void)
{
    /* Notice the equipment */
    borg_notice_aux1();
    
    /* Notice the inventory */
    borg_notice_aux2();
    
    /* Notice the home */
    /* borg_notice_aux3(); */
}



/*
 * Helper function -- calculate "power" of equipment
 */
static s32b borg_power_aux1(void)
{
    int			hold;
    int			damage;

    int			cur_wgt = 0;
    int			max_wgt = 0;
        
    s32b		value = 0L;

    auto_item		*item;


    /* Obtain the "hold" value */
    hold = adj_str_hold[my_stat_ind[A_STR]];


    /*** Analyze weapon ***/
    
    /* Examine current weapon */
    item = &auto_items[INVEN_WIELD];
    
    /* Calculate "average" damage per "normal" blow (times 2) */
    damage = (item->dd * (item->ds + 1) + ((my_to_dam + item->to_d) * 2));

    /* XXX XXX XXX reward "extra" damage from "slaying" flags */
    
    /* Reward "damage" */
    value += (my_num_blow * damage * 500L);
    
    /* Reward "bonus to hit" */
    value += ((my_to_hit + item->to_h) * 100L);

    /* Hack -- It is hard to hold a heavy weapon */
    if (hold < item->weight / 10) value -= 500000L;


    /*** Analyze bow ***/
    
    /* Examine current bow */
    item = &auto_items[INVEN_BOW];
    
    /* Calculate "average" damage per "normal" shot (times 2) */
    damage = ((my_ammo_sides + 1) + (item->to_d * 2)) * my_ammo_power;

    /* Reward "damage" */
    value += (my_num_fire * damage * 500L);

    /* Reward "bonus to hit" */
    value += ((my_to_hit + item->to_h) * 100L);

    /* Mega-Hack -- reward use of "bolts" */
    if (my_ammo_tval == TV_BOLT) value += 200000L;

    /* Mega-Hack -- reward use of "arrows" */
    if (my_ammo_tval == TV_ARROW) value += 100000L;

    /* Hack -- It is hard to hold a heavy weapon */
    if (hold < item->weight / 10) value -= 500000L;


    /*** Reward various things ***/
    
    /* Hack -- Reward light radius */
    value += (my_cur_lite * 1000000L);

    /* Hack -- Reward speed */
    value += ((my_speed - 100) * 100000L);

    /* Hack -- Reward strength bonus */
    value += (my_stat_ind[A_STR] * 500L);

    /* Hack -- Reward intelligence bonus */
    if (mb_ptr->spell_book == TV_MAGIC_BOOK) {
        value += (my_stat_ind[A_INT] * 200L);
    }
    
    /* Hack -- Reward wisdom bonus */
    if (mb_ptr->spell_book == TV_PRAYER_BOOK) {
        value += (my_stat_ind[A_WIS] * 200L);
    }
    
    /* Hack -- Reward charisma bonus */
    value += (my_stat_ind[A_CHR] * 50L);


    /*** Reward current skills ***/

    /* Hack -- tiny rewards */
    value += (my_skill_dis * 1L);
    value += (my_skill_dev * 1L);
    value += (my_skill_sav * 1L);
    value += (my_skill_stl * 1L);
    value += (my_skill_srh * 1L);
    value += (my_skill_fos * 1L);
    value += (my_skill_thn * 1L);
    value += (my_skill_thb * 1L);
    value += (my_skill_tht * 1L);
    value += (my_skill_dig * 1L);

    
    /*** Reward current flags ***/

    /* Various flags */
    if (my_see_inv) value += 5000L;
    if (my_free_act) value += 10000L;
    if (my_slow_digest) value += 10L;
    if (my_regenerate) value += 50000L;
    if (my_ffall) value += 10L;
    if (my_hold_life) value += 10000L;
    if (my_telepathy) value += 50000L;
    if (my_lite) value += 2000L;
    
    /* Immunity flags */
    if (my_immune_acid) value += 80000L;
    if (my_immune_elec) value += 40000L;
    if (my_immune_fire) value += 60000L;
    if (my_immune_cold) value += 30000L;

    /* Resistance flags */
    if (my_resist_acid) value += 8000L;
    if (my_resist_elec) value += 4000L;
    if (my_resist_fire) value += 6000L;
    if (my_resist_cold) value += 3000L;
    if (my_resist_pois) value += 20000L;
    if (my_resist_conf) value += 5000L;
    if (my_resist_sound) value += 100L;
    if (my_resist_lite) value += 200L;
    if (my_resist_dark) value += 200L;
    if (my_resist_chaos) value += 2000L;
    if (my_resist_disen) value += 5000L;
    if (my_resist_shard) value += 100L;
    if (my_resist_nexus) value += 100L;
    if (my_resist_blind) value += 5000L;
    if (my_resist_neth) value += 2000L;
    if (my_resist_fear) value += 2000L;
    
    /* Sustain flags */
    if (my_sustain_str) value += 50L;
    if (my_sustain_int) value += 50L;
    if (my_sustain_wis) value += 50L;
    if (my_sustain_con) value += 50L;
    if (my_sustain_dex) value += 50L;
    if (my_sustain_chr) value += 50L;


    /*** XXX XXX XXX Reward "necessary" flags ***/
    
    /* Mega-Hack -- See invisible (level 10) */
    if (my_see_inv && (auto_max_depth+1 >= 10)) value += 100000L;
    
    /* Mega-Hack -- Free action (level 20) */
    if (my_free_act && (auto_max_depth+1 >= 20)) value += 100000L;
    

    /*** Reward powerful armor ***/

    /* Reward armor */
    value += ((my_ac + my_to_ac) * 300L);
    

    /*** Penalize various things ***/
 
    /* Penalize various flags */
    if (my_teleport) value -= 100000L;
    if (my_aggravate) value -= 50000L;


    /*** Penalize armor weight ***/
    
    /* Compute the total armor weight */
    cur_wgt += auto_items[INVEN_BODY].weight;
    cur_wgt += auto_items[INVEN_HEAD].weight;
    cur_wgt += auto_items[INVEN_ARM].weight;
    cur_wgt += auto_items[INVEN_OUTER].weight;
    cur_wgt += auto_items[INVEN_HANDS].weight;
    cur_wgt += auto_items[INVEN_FEET].weight;

    /* Determine the weight allowance */
    max_wgt = mb_ptr->spell_weight;

    /* Hack -- heavy armor hurts magic */
    if ((mb_ptr->spell_book) &&
        (((cur_wgt - max_wgt) / 10) > 0) &&
        (auto_max_level < 20)) {

        /* Mega-Hack -- Penalize heavy armor which hurts mana */
        value -= (((cur_wgt - max_wgt) / 10) * (20 - auto_max_level) * 10L);
    }


    /*** Penalize bad magic ***/
    
    /* Hack -- most gloves hurt magic for spell-casters */
    if (mb_ptr->spell_book == TV_MAGIC_BOOK) {
    
        item = &auto_items[INVEN_HANDS];

        /* Penalize non-usable gloves */
        if (item->iqty &&
            (!(item->flags2 & TR2_FREE_ACT)) &&
            (!((item->flags1 & TR1_DEX) && (item->pval > 0)))) {

            /* Hack -- Major penalty */
            value -= 50000L;
        }
    }

    /* Hack -- most edged weapons hurt magic for priests */
    if (auto_class == CLASS_PRIEST) {

        item = &auto_items[INVEN_WIELD];

        /* Penalize non-blessed edged weapons */
        if (((item->tval == TV_SWORD) || (item->tval == TV_POLEARM)) &&
            (!(item->flags3 & TR3_BLESSED))) {

            /* Hack -- Major penalty */
            value -= 50000L;
        }
    }

    /* Result */
    return (value);
}



/*
 * Helper function -- calculate power of inventory
 */
static s32b borg_power_aux2(void)
{
    int			k, book;
    
    s32b		value = 0L;


    /*** Basic abilities ***/

    /* Reward fuel */
    k = 0;
    for ( ; k < 10 && k < amt_fuel; k++) value += 60000L;
    for ( ; k < 20 && k < amt_fuel; k++) value += 6000L;

    /* Reward food */
    k = 0;
    for ( ; k < 10 && k < amt_food; k++) value += 50000L;
    for ( ; k < 20 && k < amt_food; k++) value += 5000L;

    /* Reward ident */
    k = 0;
    for ( ; k < 20 && k < amt_ident; k++) value += 6000L;
    for ( ; k < 40 && k < amt_ident; k++) value += 600L;

    /* Reward recall */
    k = 0;
    for ( ; k < 3 && k < amt_recall; k++) value += 50000L;
    for ( ; k < 5 && k < amt_recall; k++) value += 5000L;

    /* Reward escape */
    k = 0;
    for ( ; k < 5 && k < amt_escape; k++) value += 10000L;

    /* Reward teleport */
    k = 0;
    for ( ; k < 5 && k < amt_teleport; k++) value += 10000L;


    /*** Healing ***/

    /* Reward cure critical */
    k = 0;
    for ( ; k <  5 && k < amt_cure_critical; k++) value += 5000L;
    for ( ; k < 20 && k < amt_cure_critical; k++) value += 500L;
    
    /* Reward cure serious */
    k = 0;
    for ( ; k <  5 && k < amt_cure_serious; k++) value += 500L;
    for ( ; k < 10 && k < amt_cure_serious; k++) value += 50L;


    /*** Detection ***/

    /* Reward detect trap */
    k = 0;
    for ( ; k < 1 && k < amt_detect_trap; k++) value += 4000L;
    
    /* Reward detect door */
    k = 0;
    for ( ; k < 1 && k < amt_detect_door; k++) value += 2000L;

    
    /*** Missiles ***/
    
    /* Reward missiles */
    k = 0;
    for ( ; k < 10 && k < amt_missile; k++) value += 1000L;
    for ( ; k < 30 && k < amt_missile; k++) value += 100L;


    /*** Various ***/
    
    /* Hack -- Reward add stat */
    if (amt_add_stat[A_STR]) value += 50000;
    if (amt_add_stat[A_INT]) value += 50000;
    if (amt_add_stat[A_WIS]) value += 50000;
    if (amt_add_stat[A_DEX]) value += 50000;
    if (amt_add_stat[A_CON]) value += 50000;
    if (amt_add_stat[A_CHR]) value += 50000;
    
    /* Hack -- Reward fix stat */
    if (amt_fix_stat[A_STR]) value += 10000;
    if (amt_fix_stat[A_INT]) value += 10000;
    if (amt_fix_stat[A_WIS]) value += 10000;
    if (amt_fix_stat[A_DEX]) value += 10000;
    if (amt_fix_stat[A_CON]) value += 10000;
    if (amt_fix_stat[A_CHR]) value += 10000;
    
    /* Hack -- Restore experience */
    if (amt_fix_exp) value += 500000;


    /*** Enchantment ***/
    
    /* Reward enchant armor */
    if (amt_enchant_to_a && auto_need_enchant_to_a) value += 300L;

    /* Reward enchant weapon to hit */
    if (amt_enchant_to_h && auto_need_enchant_to_h) value += 100L;

    /* Reward enchant weapon to damage */
    if (amt_enchant_to_d && auto_need_enchant_to_d) value += 500L;


    /*** Hack -- books ***/

    /* Reward books */
    for (book = 0; book < 9; book++) {

        /* No copies */
        if (!amt_book[book]) continue;

        /* The "hard" books */
        if (book >= 4) {

            /* Reward the book */
            k = 0;
            for ( ; k < 1 && k < amt_book[book]; k++) value += 300000L;
        }

        /* The "easy" books */
        else {

            int what, when = 99;

            /* Scan the spells */
            for (what = 0; what < 9; what++) {

                auto_magic *as = &auto_magics[book][what];

                /* Track minimum level */
                if (as->level < when) when = as->level;
            }

            /* Hack -- Ignore "difficult" normal books */
            if ((when > 5) && (when >= auto_max_level + 2)) continue;

            /* Reward the book */
            k = 0;
            for ( ; k < 1 && k < amt_book[book]; k++) value += 500000L;
            for ( ; k < 2 && k < amt_book[book]; k++) value += 100000L;
        }
    }


    /* Return the value */
    return (value);
}


/*
 * Helper function -- calculate power of items in the home
 */
static s32b borg_power_aux3(void)
{
    int			k, book;
    
    s32b		value = 0L;


    /*** Basic abilities ***/

    /* Collect fuel */
    for (k = 0; k < 50 && k < num_fuel; k++) value += 1000L - k*10L;

    /* Collect food */
    for (k = 0; k < 50 && k < num_food; k++) value += 800L - k*10L;

    /* Collect ident */
    for (k = 0; k < 20 && k < num_ident; k++) value += 200L - k*10L;

    /* Collect recall */
    for (k = 0; k < 10 && k < num_recall; k++) value += 300L - k*10L;

    /* Collect escape */
    for (k = 0; k < 10 && k < num_escape; k++) value += 200L - k*10L;

    /* Collect teleport */
    for (k = 0; k < 10 && k < num_teleport; k++) value += 200L - k*10L;


    /*** Healing ***/

    /* Collect cure critical */
    for (k = 0; k < 10 && k < num_cure_critical; k++) value += 150L - k*10L;
    
    /* Collect cure serious */
    for (k = 0; k < 10 && k < num_cure_serious; k++) value += 140L - k*10L;


    /*** Missiles ***/
    
    /* Collect missiles */
    for (k = 0; k < 50 && k < num_missile; k++) value += 10L;


    /*** Various ***/
    
    /* Hack -- Collect restore life levels */
    for (k = 0; k < 5 && k < num_fix_exp; k++) value += 500L - k*10L;


    /*** Enchantment ***/
    
    /* Reward enchant armor */
    for (k = 0; k < 20 && k < num_enchant_to_a; k++) value += 20L;

    /* Reward enchant weapon to hit */
    for (k = 0; k < 20 && k < num_enchant_to_h; k++) value += 15L;

    /* Reward enchant weapon to damage */
    for (k = 0; k < 20 && k < num_enchant_to_d; k++) value += 25L;


    /*** Hack -- books ***/

    /* Reward books */
    for (book = 0; book < 4; book++) {

        /* Collect up to 5 copies of each normal book */
        for (k = 0; k < 5 && k < num_book[book]; k++) value += 1000L - k*100L;
    }

    /* Reward books */
    for (book = 4; book < 9; book++) {

        /* Collect up to 2 copies of each special book */
        for (k = 0; k < 2 && k < num_book[book]; k++) value += 1000L - k*100L;
    }

    
    /* Return the value */
    return (value);
}


/*
 * Calculate the "power" of the Borg
 */
static s32b borg_power(void)
{
    s32b value = 0L;
    
    /* Process the equipment */
    value += borg_power_aux1();
    
    /* Process the inventory */
    value += borg_power_aux2();
    
    /* Process the home */
    /* value += borg_power_aux3(); */

    /* Return the value */
    return (value);
}




/*
 * Determine if an item is "probably" worthless
 *
 * This (very heuristic) function is a total hack, designed only to prevent
 * a very specific annoying situation described below.
 *
 * Note that a "cautious" priest (or low level mage/ranger) will leave town
 * with a few identify scrolls, wander around dungeon level 1 for a few turns,
 * and use all of the scrolls on leather gloves and broken daggers, and must
 * then return to town for more scrolls.  This may repeat indefinitely.
 *
 * The problem is that some characters (priests, mages, rangers) never get an
 * "average" feeling about items, and have no way to keep track of how long
 * they have been holding a given item for, so they cannot even attempt to
 * gain knowledge from the lack of "good" or "cursed" feelings.  But they
 * cannot afford to just identify everything they find by using scrolls of
 * identify, because, in general, some items are, on average, "icky", and
 * not even worth the price of a new scroll of identify.
 *
 * Even worse, the current algorithm refuses to sell un-identified items, so
 * the poor character will throw out all his good stuff to make room for crap.
 *
 * This function simply examines the item and assumes that certain items are
 * "icky", which is probably a total hack.  Perhaps we could do something like
 * compare the item to the item we are currently wearing, or perhaps we could
 * analyze the expected value of the item, or guess at the likelihood that the
 * item might be a blessed, or something.
 *
 * Currently, only characters who do not get "average" feelings are allowed
 * to decide that something is "icky", others must wait for an "average"
 * feeling.
 */
static bool borg_item_icky(auto_item *item)
{
    /* Mega-Hack -- allow "icky" items */
    if ((auto_class != CLASS_PRIEST) &&
        (auto_class != CLASS_RANGER) &&
        (auto_class != CLASS_MAGE)) return (FALSE);
            
    /* Broken dagger/sword, Filthy rag */
    if (((item->tval == TV_SWORD) && (item->sval == SV_BROKEN_DAGGER)) ||
        ((item->tval == TV_SWORD) && (item->sval == SV_BROKEN_SWORD)) ||
        ((item->tval == TV_SOFT_ARMOR) && (item->sval == SV_FILTHY_RAG))) {
        return (TRUE);
    }

    /* Dagger, Sling */
    if (((item->tval == TV_SWORD) && (item->sval == SV_DAGGER)) ||
        ((item->tval == TV_BOW) && (item->sval == SV_SLING))) {
        return (TRUE);
    }
        
    /* Cloak, Robe */
    if (((item->tval == TV_CLOAK) && (item->sval == SV_CLOAK)) ||
        ((item->tval == TV_SOFT_ARMOR) && (item->sval == SV_ROBE))) {
        return (TRUE);
    }

    /* Leather Gloves */
    if ((item->tval == TV_GLOVES) &&
        (item->sval == SV_SET_OF_LEATHER_GLOVES)) {
        return (TRUE);
    }

    /* Hack -- Diggers */
    if (item->tval == TV_DIGGING) return (TRUE);

    /* Assume okay */
    return (FALSE);
}






/*
 * Read a scroll of recall (if "allowed")
 */
static bool borg_recall(void)
{
    /* Multiple "recall" fails */
    if (!goal_recalling) {

        /* Try to invoke "recall" (scroll is "better") */
        if (borg_read_scroll(SV_SCROLL_WORD_OF_RECALL) ||
            borg_zap_rod(SV_ROD_RECALL)) {

            /* Success */
            return (TRUE);
        }
    }

    /* Nothing */
    return (FALSE);
}






/*
 * Use things in a useful, but non-essential, manner
 */
static bool borg_use_things(void)
{
    int i;


    /* Quaff experience restoration potion */
    if (do_fix_exp &&
        borg_quaff_potion(SV_POTION_RESTORE_EXP)) {
        
        return (TRUE);
    }


    /* Quaff potions of "increase" stat if needed */
    if (((my_stat_cur[A_STR] < 18+100) &&
         borg_quaff_potion(SV_POTION_INC_STR)) ||
        ((my_stat_cur[A_INT] < 18+100) &&
         borg_quaff_potion(SV_POTION_INC_INT)) ||
        ((my_stat_cur[A_WIS] < 18+100) &&
         borg_quaff_potion(SV_POTION_INC_WIS)) ||
        ((my_stat_cur[A_DEX] < 18+100) &&
         borg_quaff_potion(SV_POTION_INC_DEX)) ||
        ((my_stat_cur[A_CON] < 18+100) &&
         borg_quaff_potion(SV_POTION_INC_CON)) ||
        ((my_stat_cur[A_CHR] < 18+100) &&
         borg_quaff_potion(SV_POTION_INC_CHR))) {

        return (TRUE);
    }

    /* Quaff potions of "restore" stat if needed */
    if (((do_fix_stat[A_STR]) &&
         borg_quaff_potion(SV_POTION_RES_STR)) ||
        ((do_fix_stat[A_INT]) &&
         borg_quaff_potion(SV_POTION_RES_INT)) ||
        ((do_fix_stat[A_WIS]) &&
         borg_quaff_potion(SV_POTION_RES_WIS)) ||
        ((do_fix_stat[A_DEX]) &&
         borg_quaff_potion(SV_POTION_RES_DEX)) ||
        ((do_fix_stat[A_CON]) &&
         borg_quaff_potion(SV_POTION_RES_CON)) ||
        ((do_fix_stat[A_CHR]) &&
         borg_quaff_potion(SV_POTION_RES_CHR))) {

        return (TRUE);
    }


    /* Use some items right away */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;
        
        /* Process "force" items */
        switch (item->tval) {

            case TV_POTION:

                /* Check the scroll */
                switch (item->sval) {

                    case SV_POTION_ENLIGHTENMENT:

                        /* Never quaff these in town */
                        if (!auto_depth) break;

                    case SV_POTION_AUGMENTATION:
                    case SV_POTION_EXPERIENCE:

                        /* Try quaffing the potion */
                        if (borg_quaff_potion(item->sval)) return (TRUE);
                }

                break;

            case TV_SCROLL:

                /* Hack -- check Blind/Confused */
                if (do_blind || do_confused) break;

                /* Check the scroll */
                switch (item->sval) {

                    case SV_SCROLL_MAPPING:
                    case SV_SCROLL_DETECT_TRAP:
                    case SV_SCROLL_DETECT_DOOR:
                    case SV_SCROLL_ACQUIREMENT:
                    case SV_SCROLL_STAR_ACQUIREMENT:
                    case SV_SCROLL_PROTECTION_FROM_EVIL:

                        /* Never read these in town */
                        if (!auto_depth) break;

                        /* Try reading the scroll */
                        if (borg_read_scroll(item->sval)) return (TRUE);
                }

                break;
        }
    }


    /* Eat food */
    if (do_hungry) {

        /* Attempt to satisfy hunger */
        if (borg_spell(2,0) ||
            borg_prayer(1,5) ||
            borg_eat_food(SV_FOOD_RATION)) {

            return (TRUE);
        }
    }


    /* Nothing to do */
    return (FALSE);
}




/*
 * Hack -- check a location for "dark room"
 */
static bool borg_light_room_aux(int x, int y)
{
    auto_grid *ag;

    int x1, y1, x2, y2;

    /* Illegal location */
    if (!grid_legal(x,y)) return (FALSE);

    /* Get grid */
    ag = grid(x,y);

    /* Location must be dark */
    if (ag->info & BORG_OKAY) return (FALSE);

    /* XXX XXX XXX Location must be on panel */

    /* Location must be in view */
    if (!(ag->info & BORG_VIEW)) return (FALSE);

    /* Build the rectangle */
    x1 = MIN(c_x, x);
    x2 = MAX(c_x, x);
    y1 = MIN(c_y, y);
    y2 = MAX(c_y, y);

    /* Scan the rectangle */
    for (y = y1; y <= y2; y++) {
        for (x = x1; x <= x2; x++) {

            /* Get grid */
            ag = grid(x,y);

            /* Location must be known */
            if (ag->o_c == ' ') return (FALSE);

            /* Location must not be a wall */
            if (ag->info & BORG_WALL) return (FALSE);
        }
    }

    /* Okay */
    return (TRUE);
}



/*
 * Light up the room (if possible and useful)
 */
static bool borg_check_lite(void)
{
    bool okay = FALSE;

    auto_item *item = &auto_items[INVEN_LITE];


    /* Refuel current torch */
    if ((item->tval == TV_LITE) && (item->sval == SV_LITE_TORCH)) {

        /* Refuel the torch if needed */
        if ((item->pval < 2500) && borg_refuel_torch()) return (TRUE);
    }

    /* Refuel current lantern */
    if ((item->tval == TV_LITE) && (item->sval == SV_LITE_LANTERN)) {

        /* Refuel the lantern if needed */
        if ((item->pval < 5000) && borg_refuel_lantern()) return (TRUE);
    }


    /* Mega-Hack -- Artifact -- Wizard Light */
    if (item->name1 == ART_THRAIN) {

        /* Hack -- Allow for "recharge" */
        if (!when_art_lite || (c_t - when_art_lite >= 1000)) {

            /* Note the time */
            when_art_lite = c_t;

            /* Note */
            borg_note("# Activating the Arkenstone of Thrain.");

            /* Activate the light */
            borg_keypress('A');
            borg_keypress('f');

            /* Success */
            return (TRUE);
        }
    }


    /* Mega-Hack -- Artifact -- Magic Mapping */
    if (item->name1 == ART_ELENDIL) {

        /* Hack -- Allow for "recharge" */
        if (!when_art_lite || (c_t - when_art_lite >= 200)) {

            /* Note the time */
            when_art_lite = c_t;

            /* Note */
            borg_note("# Activating the Star of Elendil.");

            /* Activate the light */
            borg_keypress('A');
            borg_keypress('f');

            /* Success */
            return (TRUE);
        }
    }


    /* Never in town */
    if (!auto_depth) return (FALSE);

    /* Never when blind or hallucinating */
    if (do_blind || do_image) return (FALSE);


    /* Mega-Hack -- find traps and doors */
    if (!when_traps || (c_t - when_traps >= 123) ||
        !when_doors || (c_t - when_doors >= 207)) {

        /* Check for traps and doors */
        if (borg_spell(0,7)) {

            when_traps = c_t;
            when_doors = c_t;

            borg_note("# Checking for traps and doors.");

            return (TRUE);
        }
    }
    
    /* Mega-Hack -- find traps */
    if (!when_traps || (c_t - when_traps >= 123)) {

        /* Check for traps */
        if (borg_zap_rod(SV_ROD_DETECT_TRAP) ||
            borg_prayer(0,5)) {

            when_traps = c_t;

            borg_note("# Checking for traps.");

            return (TRUE);
        }
    }

    /* Mega-Hack -- find doors */
    if (!when_doors || (c_t - when_doors >= 207)) {

        /* Check for traps */
        if (borg_zap_rod(SV_ROD_DETECT_DOOR) ||
            borg_prayer(0,6)) {

            when_doors = c_t;

            borg_note("# Checking for doors.");

            return (TRUE);
        }
    }


    /* Prevent infinite loops and such */
    if (!when_call_lite || (c_t - when_call_lite >= 10)) {

        /* Check for "need light" */
        if (item->name1) {

            /* Check "Artifact Corners" */
            if (borg_light_room_aux(c_x + 3, c_y + 2) ||
                borg_light_room_aux(c_x + 3, c_y - 2) ||
                borg_light_room_aux(c_x - 3, c_y + 2) ||
                borg_light_room_aux(c_x - 3, c_y - 2) ||
                borg_light_room_aux(c_x + 2, c_y + 3) ||
                borg_light_room_aux(c_x + 2, c_y - 3) ||
                borg_light_room_aux(c_x - 2, c_y + 3) ||
                borg_light_room_aux(c_x - 2, c_y - 3)) {

                /* Go for it */
                okay = TRUE;
            }
        }

        /* Check for "need light" */
        else {

            /* Check "Lantern Corners" */
            if (borg_light_room_aux(c_x + 2, c_y + 2) ||
                borg_light_room_aux(c_x + 2, c_y - 2) ||
                borg_light_room_aux(c_x - 2, c_y + 2) ||
                borg_light_room_aux(c_x - 2, c_y - 2)) {

                /* Go for it */
                okay = TRUE;
            }
        }
    }

    /* Not okay */
    if (!okay) return (FALSE);


    /* Mega-Hack -- Artifact -- Call Light */
    if (item->name1 == ART_GALADRIEL) {

        /* Hack -- Allow for recharge */
        if (!when_art_lite || (c_t - when_art_lite >= 50)) {

            /* Reset the call light counter */
            when_call_lite = c_t;

            /* Note the time */
            when_art_lite = c_t;

            /* Note */
            borg_note("# Activating the Phial.");

            /* Activate the light */
            borg_keypress('A');
            borg_keypress('f');

            /* Success */
            return (TRUE);
        }
    }


    /* Go for it */
    if (borg_spell(0,3) ||
        borg_prayer(0,4) ||
        borg_zap_rod(SV_ROD_ILLUMINATION) ||
        borg_use_staff(SV_STAFF_LITE) ||
        borg_read_scroll(SV_SCROLL_LIGHT)) {

        /* Reset the call light counter */
        when_call_lite = c_t;

        /* Note */
        borg_note("# Illuminating the room.");

        /* Success */
        return (TRUE);
    }


    /* No light */
    return (FALSE);
}



/*
 * Enchant armor
 */
static bool borg_enchant_to_a(void)
{
    int i, b, a;


    /* Nothing to enchant */
    if (!auto_need_enchant_to_a) return (FALSE);
    
    /* Need "enchantment" ability */
    if (!amt_enchant_to_a) return (FALSE);

    /* Forbid blind/confused */
    if (do_blind || do_confused) return (FALSE);

    /* Look for armor that needs enchanting */
    for (b = 0, a = 99, i = INVEN_BODY; i <= INVEN_FEET; i++) {

        auto_item *item = &auto_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;
        
        /* Skip non-identified items */
        if (!item->able) continue;

        /* Find the least enchanted item */
        if (item->to_a > a) continue;

        /* Save the info */
        b = i; a = item->to_a;
    }

    /* Enchant that item if "possible" */
    if (b && (a < 8) && (borg_read_scroll(SV_SCROLL_ENCHANT_ARMOR))) {

        /* Choose from equipment */
        borg_keypress('/');

        /* Choose that item */
        borg_keypress(I2A(b - INVEN_WIELD));

        /* Success */
        return (TRUE);
    }

    /* Nothing to do */
    return (FALSE);
}



/*
 * Enchant weapons to hit
 */
static bool borg_enchant_to_h(void)
{
    int i, b, a;


    /* Nothing to enchant */
    if (!auto_need_enchant_to_h) return (FALSE);
    
    /* Need "enchantment" ability */
    if (!amt_enchant_to_h) return (FALSE);

    /* Forbid blind/confused */
    if (do_blind || do_confused) return (FALSE);

    /* Look for a weapon that needs enchanting */
    for (b = 0, a = 99, i = INVEN_WIELD; i <= INVEN_BOW; i++) {

        auto_item *item = &auto_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;
        
        /* Skip non-identified items */
        if (!item->able) continue;

        /* Find the least enchanted item */
        if (item->to_h > a) continue;

        /* Save the info */
        b = i; a = item->to_h;
    }

    /* Enchant that item if "possible" */
    if (b && (a < 8) && (borg_read_scroll(SV_SCROLL_ENCHANT_WEAPON_TO_HIT))) {

        /* Choose from equipment */
        borg_keypress('/');

        /* Choose that item */
        borg_keypress(I2A(b - INVEN_WIELD));

        /* Success */
        return (TRUE);
    }

    /* Nothing to do */
    return (FALSE);
}


/*
 * Enchant weapons
 */
static bool borg_enchant_to_d(void)
{
    int i, b, a;

    /* Nothing to enchant */
    if (!auto_need_enchant_to_d) return (FALSE);
    
    /* Need "enchantment" ability */
    if (!amt_enchant_to_d) return (FALSE);

    /* Forbid blind/confused */
    if (do_blind || do_confused) return (FALSE);

    /* Look for a weapon that needs enchanting */
    for (b = 0, a = 99, i = INVEN_WIELD; i <= INVEN_BOW; i++) {

        auto_item *item = &auto_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;
        
        /* Skip non-identified items */
        if (!item->able) continue;

        /* Find the least enchanted item */
        if (item->to_d > a) continue;

        /* Save the info */
        b = i; a = item->to_d;
    }

    /* Enchant that item if "possible" */
    if (b && (a < 8) && (borg_read_scroll(SV_SCROLL_ENCHANT_WEAPON_TO_DAM))) {

        /* Choose from equipment */
        borg_keypress('/');

        /* Choose that item */
        borg_keypress(I2A(b - INVEN_WIELD));

        /* Success */
        return (TRUE);
    }


    /* Nothing to do */
    return (FALSE);
}


/*
 * Attempt to destroy an item by any means possible
 */
static bool borg_destroy(int i)
{
    auto_item *item = &auto_items[i];


    /* Special destruction */
    switch (item->tval) {

        case TV_POTION:

            /* Check the potion */
            switch (item->sval) {

                case SV_POTION_WATER:
                case SV_POTION_APPLE_JUICE:
                case SV_POTION_SLIME_MOLD:
                case SV_POTION_CURE_LIGHT:
                case SV_POTION_CURE_SERIOUS:
                case SV_POTION_CURE_CRITICAL:
                case SV_POTION_HEALING:
                case SV_POTION_STAR_HEALING:
                case SV_POTION_LIFE:
                case SV_POTION_RES_STR:
                case SV_POTION_RES_INT:
                case SV_POTION_RES_WIS:
                case SV_POTION_RES_DEX:
                case SV_POTION_RES_CON:
                case SV_POTION_RES_CHR:
                case SV_POTION_RESTORE_EXP:
                case SV_POTION_RESTORE_MANA:
                case SV_POTION_HEROISM:
                case SV_POTION_BESERK_STRENGTH:
                case SV_POTION_RESIST_HEAT:
                case SV_POTION_RESIST_COLD:
                case SV_POTION_INFRAVISION:
                case SV_POTION_DETECT_INVIS:
                case SV_POTION_SLOW_POISON:
                case SV_POTION_CURE_POISON:
                case SV_POTION_SPEED:

                    /* Try quaffing the potion */
                    if (borg_quaff_potion(item->sval)) return (TRUE);
            }

            break;

        case TV_SCROLL:

            /* Check the scroll */
            switch (item->sval) {

                case SV_SCROLL_REMOVE_CURSE:
                case SV_SCROLL_LIGHT:
                case SV_SCROLL_MONSTER_CONFUSION:
                case SV_SCROLL_RUNE_OF_PROTECTION:
                case SV_SCROLL_STAR_REMOVE_CURSE:
                case SV_SCROLL_DETECT_GOLD:
                case SV_SCROLL_DETECT_ITEM:
                case SV_SCROLL_TRAP_DOOR_DESTRUCTION:
                case SV_SCROLL_SATISFY_HUNGER:
                case SV_SCROLL_DISPEL_UNDEAD:
                case SV_SCROLL_BLESSING:
                case SV_SCROLL_HOLY_CHANT:
                case SV_SCROLL_HOLY_PRAYER:

                    /* Try reading the scroll */
                    if (borg_read_scroll(item->sval)) return (TRUE);
            }

            break;

        case TV_FOOD:

            /* Check the scroll */
            switch (item->sval) {

                case SV_FOOD_CURE_POISON:
                case SV_FOOD_CURE_BLINDNESS:
                case SV_FOOD_CURE_PARANOIA:
                case SV_FOOD_CURE_CONFUSION:
                case SV_FOOD_CURE_SERIOUS:
                case SV_FOOD_RESTORE_STR:
                case SV_FOOD_RESTORE_CON:
                case SV_FOOD_RESTORING:
                case SV_FOOD_BISCUIT:
                case SV_FOOD_JERKY:
                case SV_FOOD_RATION:
                case SV_FOOD_SLIME_MOLD:
                case SV_FOOD_WAYBREAD:
                case SV_FOOD_PINT_OF_ALE:
                case SV_FOOD_PINT_OF_WINE:

                    /* Try eating the food (unless Bloated) */
                    if (!do_full && borg_eat_food(item->sval)) return (TRUE);
            }

            break;
    }


    /* Message */
    borg_note(format("# Destroying %s.", item->desc));

    /* Destroy that item */
    borg_keypress('k');
    borg_keypress(I2A(i));

    /* Hack -- destroy a single item */
    if (item->iqty > 1) borg_keypress('\n');

    /* Mega-Hack -- verify destruction */
    borg_keypress('y');

    /* Did something */
    return (TRUE);
}




/*
 * Destroy "junk" when slow or full.
 *
 * We penalize the loss of both power and monetary value, and reward
 * the loss of weight that may be slowing us down.  The weight loss
 * is worth one gold per tenth of a pound.  This causes things like
 * lanterns and chests and spikes to be considered "annoying".
 */
static bool borg_crush_junk(void)
{
    int i, b_i = -1;
    s32b p, b_p = 0L;

    s32b greed;
    s32b price;


    /* Check normal slots */
    for (i = 0; i < INVEN_PACK; i++) {
    
        auto_item *item = &auto_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip non "worthless" items */
        if (item->value > 0) continue;

        /* Hack -- Skip artifacts */
        if (item->name1) continue;

        /* Hack -- skip "artifacts" */
        if (streq(item->note, "{special}")) continue;
        if (streq(item->note, "{terrible}")) continue;

        /* Message */
        borg_note(format("# Junking junk (junk)"));

        /* Attempt to destroy it */
        if (borg_destroy(i)) return (TRUE);
    }


    /* Check normal slots */
    for (i = 0; i < INVEN_PACK; i++) {
    
        auto_item *item = &auto_items[i];

        /* Notice empty slots */
        if (!item->iqty) break;
    }

    /* Do not destroy junk if not "forced" and not "slow" */
    if ((i < INVEN_PACK) && (auto_speed >= 110)) return (FALSE);


    /* Calculate "greed" factor */
    greed = (auto_gold / 100L) + 100L;

    /* Minimal greed */
    if (greed > 10000L) greed = 10000L;


    /* Scan for junk */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip "good" unknown items (unless "icky") */
        if (!item->able && !streq(item->note, "{average}") &&
            (item->value > 0) && !borg_item_icky(item)) continue;
        
        /* Hack -- Skip artifacts */
        if (item->name1) continue;

        /* Save the item */
        COPY(&safe_items[i], &auto_items[i], auto_item);

        /* Destroy one of the items */
        auto_items[i].iqty--;

        /* Examine the inventory */
        borg_notice();
                
        /* Evaluate the inventory */
        p = borg_power();

        /* Restore the item */
        COPY(&auto_items[i], &safe_items[i], auto_item);

        /* Obtain the base price */
        price = ((item->value < 30000L) ? item->value : 30000L);

        /* Hack -- ignore very cheap items */
        if (price < greed) price = 0L;

        /* Penalize money loss, reward weight loss */
        p -= (price - item->weight);

        /* Ignore "bad" swaps */
        if ((b_i >= 0) && (p < b_p)) continue;

        /* Maintain the "best" */
        b_i = i; b_p = p;
    }

    /* Examine the inventory */
    borg_notice();

    /* Evaluate */
    p = borg_power();

    /* Destroy "useless" things */
    if ((b_i >= 0) && (b_p >= p)) {

        /* Message */
        borg_note(format("# Junking %ld gold (junk)", p - b_p));

        /* Attempt to destroy it */
        if (borg_destroy(b_i)) return (TRUE);
    }

    /* Nothing to destroy */
    return (FALSE);
}



/*
 * Make sure we have at least one free inventory slot.
 *
 * Acquire an empty inventory slot, by any means possible.
 *
 * This function evaluates the possible worlds that result from
 * the destruction of each inventory slot, and attempts to destroy
 * that slot which causes the best possible resulting world.
 *
 * We attempt to avoid destroying unknown items by "rewarding" the
 * presence of unknown items by a massively heuristic value.
 *
 * If the Borg cannot find something to destroy, which should only
 * happen if he fills up with artifacts, then he will probably act
 * rather twitchy for a while.
 */
static bool borg_free_space(void)
{
    int i, b_i = -1;
    s32b p, b_p = 0L;

    s32b price;
    s32b greed;


    /* Check normal slots */
    for (i = 0; i < INVEN_PACK; i++) {
    
        auto_item *item = &auto_items[i];

        /* Found an empty slot */
        if (!item->iqty) return (FALSE);
    }


    /* Calculate "greed" factor */
    greed = (auto_gold / 100L) + 100L;

    /* Minimal greed */
    if (greed > 10000L) greed = 10000L;


    /* Scan the inventory */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Hack -- skip "artifacts" */
        if (item->name1) continue;

        /* Hack -- skip "artifacts" */
        if (streq(item->note, "{special}")) continue;
        if (streq(item->note, "{terrible}")) continue;

        /* Save the item */
        COPY(&safe_items[i], &auto_items[i], auto_item);

        /* Destroy one of the items */
        auto_items[i].iqty--;

        /* Examine the inventory */
        borg_notice();
                
        /* Evaluate the inventory */
        p = borg_power();

        /* Restore the item */
        COPY(&auto_items[i], &safe_items[i], auto_item);

        /* Obtain the base price */
        price = ((item->value < 30000L) ? item->value : 30000L);

        /* Hack -- ignore very cheap items */
        if (price < greed) price = 0L;

        /* Penalize money loss, reward weight loss */
        p -= (price - item->weight);

        /* Hack -- try not to destroy "unaware" items */
        if (!item->kind && (item->value > 0)) {

            /* Hack -- Reward "unaware" items */
            switch (item->tval) {

                case TV_RING:
                case TV_AMULET:
                    p -= (auto_max_depth * 5000L);
                    break;

                case TV_ROD:
                    p -= (auto_max_depth * 3000L);
                    break;

                case TV_STAFF:
                case TV_WAND:
                    p -= (auto_max_depth * 2000L);
                    break;

                case TV_SCROLL:
                case TV_POTION:
                    p -= (auto_max_depth * 500L);
                    break;

                case TV_FOOD:
                    p -= (auto_max_depth * 10L);
                    break;
            }
        }

        /* Hack -- try not to destroy "unknown" items (unless "icky") */
        if (!item->able && !streq(item->note, "{average}") &&
            (item->value > 0) && !borg_item_icky(item)) {

            /* Reward "unknown" items */
            switch (item->tval) {
            
                case TV_SHOT:
                case TV_ARROW:
                case TV_BOLT:
                    p -= 100L;
                    break;

                case TV_BOW:
                    p -= 20000L;
                    break;

                case TV_DIGGING:
                    p -= 10L;
                    break;

                case TV_HAFTED:
                case TV_POLEARM:
                case TV_SWORD:
                    p -= 10000L;
                    break;

                case TV_BOOTS:
                case TV_GLOVES:
                case TV_HELM:
                case TV_CROWN:
                case TV_SHIELD:
                case TV_CLOAK:
                    p -= 15000L;
                    break;

                case TV_SOFT_ARMOR:
                case TV_HARD_ARMOR:
                case TV_DRAG_ARMOR:
                    p -= 15000L;
                    break;

                case TV_AMULET:
                case TV_RING:
                    p -= 5000L;
                    break;

                case TV_STAFF:
                case TV_WAND:
                    p -= 1000L;
                    break;
            }
        }

        /* Ignore "bad" swaps */
        if ((b_i >= 0) && (p < b_p)) continue;

        /* Maintain the "best" */
        b_i = i; b_p = p;
    }

    /* Examine the inventory */
    borg_notice();

    /* Evaluate the inventory */
    p = borg_power();

    /* Hack -- panic before destroying good stuff */
    if (panic_stuff && (b_i >= 0) && ((p - b_p) > greed / 2)) {

        /* Panic */
        borg_oops("panic_stuff");

        /* Oops */
        return (TRUE);
    }
    
    /* Attempt to destroy it */
    if (b_i >= 0) {

        /* Debug */
        borg_note(format("# Junking %ld gold (full)", p - b_p));

        /* Try to throw away the junk */
        if (borg_destroy(b_i)) return (TRUE);
    }

    /* Paranoia */
    return (FALSE);
}





/*
 * Count the number of items worth "selling"
 *
 * This determines the choice of stairs.
 *
 * XXX XXX XXX Total hack, by the way...
 */
static int borg_count_sell(void)
{
    int i, k = 0;

    s32b price;
    s32b greed;


    /* Calculate "greed" factor */
    greed = (auto_gold / 100L) + 100L;

    /* Minimal greed */
    if (greed > 10000L) greed = 10000L;
    

    /* Count "sellable" items */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;	

        /* Skip "crappy" items */
        if (item->value <= 0) continue;

        /* Obtain the base price */
        price = ((item->value < 30000L) ? item->value : 30000L);

        /* Skip cheap "known" (or "average") items */
        if ((price * item->iqty < greed) &&
            (item->able || streq(item->note, "{average}"))) continue;

        /* Count remaining items */
        k++;
    }

    /* Result */
    return (k);
}




/*
 * Identify items if possible
 *
 * Note that "borg_parse()" will "cancel" the identification if it
 * detects a "You failed..." message.  This is VERY important!!!
 * Otherwise the "identify" might induce bizarre actions by sending
 * the "index" of an item as a command.
 *
 * Hack -- recover from mind blanking by re-identifying the equipment.
 *
 * We instantly identify items known to be "good" (or "terrible").
 *
 * We identify most un-aware items as soon as possible.
 *
 * We identify most un-known items as soon as possible.
 * 
 * We play games with items that get "feelings" to try and wait for
 * "sensing" to take place if possible.
 *
 * XXX XXX XXX Make sure not to sell "non-aware" items, unless
 * we are really sure we want to lose them.  For example, we should
 * wait for feelings on (non-icky) wearable items or else make sure
 * that we identify them before we try and sell them.
 *
 * Mega-Hack -- the whole "sometimes identify things" code is a total
 * hack.  Slightly less bizarre would be some form of "occasionally,
 * pick a random item and identify it if necessary", which might lower
 * the preference for identifying items that appear early in the pack.
 * Also, preventing inventory motion would allow proper time-stamping.
 */
static bool borg_test_stuff(void)
{
    int i, b_i = -1;
    s32b v, b_v = -1;
    

    /* Look for an item to identify (equipment) */
    for (i = INVEN_WIELD; i <= INVEN_FEET; i++) {

        auto_item *item = &auto_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip known items */
        if (item->able) continue;

        /* Hack -- never identify "average" things */
        /* if (streq(item->note, "{average}")) continue; */

        /* Get the value */
        v = item->value + 100000L;

        /* Track the best */
        if (v < b_v) continue;
        
        /* Track it */
        b_i = i; b_v = v;
    }

    /* Look for an item to identify (inventory) */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip known items */
        if (item->able) continue;

        /* Assume nothing */
        v = 0;
        
        /* Identify "good" (and "terrible") items */
        if (streq(item->note, "{good}")) v = item->value + 10000L;
        else if (streq(item->note, "{excellent}")) v = item->value + 20000L;
        else if (streq(item->note, "{special}")) v = item->value + 50000L;
        else if (streq(item->note, "{terrible}")) v = item->value + 50000L;

        /* Nothing */
        if (!v) continue;
        
        /* Track the best */
        if (v < b_v) continue;
        
        /* Track it */
        b_i = i; b_v = v;
    }

    /* Look for an item to identify (inventory) */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;        

        /* Skip known items */
        if (item->able) continue;

        /* Hack -- never identify "average" things */
        if (streq(item->note, "{average}")) continue;

        /* Hack -- assume no value */
        v = 0;

        /* Hack -- reward "unaware" items */
        if (!item->kind) {

            /* Analyze the type */
            switch (item->tval) {

                case TV_RING:
                case TV_AMULET:

                    /* Hack -- reward depth */
                    v += (auto_max_depth * 5000L);

                    break;

                case TV_ROD:

                    /* Hack -- reward depth */
                    v += (auto_max_depth * 3000L);

                    break;

                case TV_WAND:
                case TV_STAFF:

                    /* Hack -- reward depth */
                    v += (auto_max_depth * 2000L);

                    break;

                case TV_POTION:
                case TV_SCROLL:

                    /* Hack -- boring levels */
                    if (auto_max_depth < 5) break;

                    /* Hack -- reward depth */
                    v += (auto_max_depth * 500L);

                    break;

                case TV_FOOD:

                    /* Hack -- reward depth */
                    v += (auto_max_depth * 10L);

                    break;
            }
        }

        /* Analyze the type */
        switch (item->tval) {
        
            case TV_CHEST:

                /* Hack -- Always identify chests */
                v = item->value;
                break;
                
            case TV_WAND:
            case TV_STAFF:

                /* Hack -- Always identify (get charges) */
                v = item->value;
                break;

            case TV_RING:
            case TV_AMULET:

                /* Hack -- Always identify (get information) */
                v = item->value;
                break;

            case TV_LITE:

                /* Hack -- Always identify (get artifact info) */
                v = item->value;
                break;

	    case TV_SHOT:
	    case TV_ARROW:
	    case TV_BOLT:
            case TV_BOW:
            case TV_DIGGING:
            case TV_HAFTED:
            case TV_POLEARM:
            case TV_SWORD:
            case TV_BOOTS:
            case TV_GLOVES:
            case TV_HELM:
            case TV_CROWN:
            case TV_SHIELD:
            case TV_CLOAK:
            case TV_SOFT_ARMOR:
            case TV_HARD_ARMOR:
            case TV_DRAG_ARMOR:

                /* Mega-Hack -- use identify spell */
                if (borg_spell_okay(2,4) && (rand_int(100) < 50)) {
                    v = item->value;
                }

                /* Mega-Hack -- use identify prayer */
                if (borg_prayer_okay(5,2) && (rand_int(100) < 50)) {
                    v = item->value;
                }

                /* Mega-Hack -- mages get bored */
                if ((auto_class == 1) && (rand_int(1000) < auto_level)) {

                    /* Mega-Hack -- ignore "icky" items */
                    if (!borg_item_icky(item)) v = item->value;
                }
                
                /* Mega-Hack -- rangers get bored */
                if ((auto_class == 4) && (rand_int(3000) < auto_level)) {

                    /* Mega-Hack -- ignore "icky" items */
                    if (!borg_item_icky(item)) v = item->value;
                }
                
                /* Mega-Hack -- priests get bored */
                if ((auto_class == 2) && (rand_int(5000) < auto_level)) {

                    /* Mega-Hack -- ignore "icky" items */
                    if (!borg_item_icky(item)) v = item->value;
                }
                
                break;
        }

        /* Ignore */
        if (!v) continue;

        /* Track the best */
        if (v < b_v) continue;
        
        /* Track it */
        b_i = i; b_v = v;
    }


    /* Found something */
    if (b_i >= 0) {

        auto_item *item = &auto_items[b_i];
            
        /* Use a Spell/Prayer/Rod/Staff/Scroll of Identify */
        if (borg_spell(2,4) ||
            borg_prayer(5,2) ||
            borg_zap_rod(SV_ROD_IDENTIFY) ||
            borg_use_staff(SV_STAFF_IDENTIFY) ||
            borg_read_scroll(SV_SCROLL_IDENTIFY)) {

            /* Log -- may be cancelled */
            borg_note(format("# Identifying %s.", item->desc));

            /* Equipment */
            if (b_i >= INVEN_WIELD) {

                /* Select the equipment */
                borg_keypress('/');

                /* Select the item */
                borg_keypress(I2A(b_i - INVEN_WIELD));
            }

            /* Inventory */            
            else {

                /* Select the item */
                borg_keypress(I2A(b_i));
            }
            
            /* Success */
            return (TRUE);
        }
    }

    /* Nothing to do */
    return (FALSE);
}




/*
 * Attempt to "swap" rings, for efficiency purposes
 *
 * Note that the "INVEN_RIGHT" ring should always be "better" than the
 * "INVEN_LEFT" ring since it is more "tightly" held by the player.
 *
 * This routine is only called in shops, to allow us to "safely" play
 * the ring shuffling game.  Note that on any one trip to the dungeon,
 * we may collect several useful rings, but only actually keep the best,
 * since we may sell them before we shuffle rings.  Oh well.
 */
static bool borg_swap_rings(void)
{
    int hole;
    int icky;
    
    s32b v, v1, v2;


    /* Look for an empty slot */
    for (hole = 0; hole < INVEN_PACK; hole++) {
    
        auto_item *item = &auto_items[hole];
        
        /* Notice empty slot */
        if (!item->iqty) break;
    }
    
    /* Need an empty slot */
    if (hole == INVEN_PACK) return (FALSE);

    /* Look for an empty slot */
    for (icky = hole + 1; icky < INVEN_PACK; icky++) {
    
        auto_item *item = &auto_items[icky];

        /* Notice empty slot */
        if (!item->iqty) break;
    }
    
    /* Need an empty slot */
    if (icky == INVEN_PACK) return (FALSE);


    /*** Consider taking off the "left" ring ***/

    /* Save the hole */
    COPY(&safe_items[hole], &auto_items[hole], auto_item);

    /* Save the ring */
    COPY(&safe_items[INVEN_LEFT], &auto_items[INVEN_LEFT], auto_item);

    /* Take off the ring */
    COPY(&auto_items[hole], &auto_items[INVEN_LEFT], auto_item);

    /* Erase left ring */
    WIPE(&auto_items[INVEN_LEFT], auto_item);

    /* Examine the inventory */
    borg_notice();
                
    /* Evaluate the inventory */
    v1 = borg_power();

    /* Restore the ring */
    COPY(&auto_items[INVEN_LEFT], &safe_items[INVEN_LEFT], auto_item);

    /* Restore the hole */
    COPY(&auto_items[hole], &safe_items[hole], auto_item);


    /*** Consider taking off the "right" ring ***/

    /* Save the hole */
    COPY(&safe_items[hole], &auto_items[hole], auto_item);

    /* Save the ring */
    COPY(&safe_items[INVEN_RIGHT], &auto_items[INVEN_RIGHT], auto_item);

    /* Take off the ring */
    COPY(&auto_items[hole], &auto_items[INVEN_RIGHT], auto_item);

    /* Erase left ring */
    WIPE(&auto_items[INVEN_RIGHT], auto_item);

    /* Examine the inventory */
    borg_notice();
                
    /* Evaluate the inventory */
    v2 = borg_power();

    /* Restore the ring */
    COPY(&auto_items[INVEN_RIGHT], &safe_items[INVEN_RIGHT], auto_item);

    /* Restore the hole */
    COPY(&auto_items[hole], &safe_items[hole], auto_item);

    
    /*** Swap rings if necessary ***/

    /* Examine the inventory */
    borg_notice();

    /* Evaluate the inventory */
    v = borg_power();

    /* Prepare to "swap" if needed */
    if (v2 > v1) {
    
        /* Log */
        borg_note("# Taking off both rings.");

        /* Take off "tight"/"right" ring */
        if (auto_items[INVEN_RIGHT].iqty) {
            borg_keypress('t');
            borg_keypress(I2A(INVEN_RIGHT - INVEN_WIELD));
        }
        
        /* Take off "loose"/"left" ring */
        if (auto_items[INVEN_LEFT].iqty) {
            borg_keypress('t');
            borg_keypress(I2A(INVEN_LEFT - INVEN_WIELD));
        }

        /* Success */
        return (TRUE);
    }
    
    /* Nope */
    return (FALSE);
}



/*
 * Attempt to take off useless equipment
 */
static bool borg_takeoff_stuff(void)
{
    int hole;

    int i, b_i = -1;
        
    s32b v, b_v = 0L;


    /* Look for an empty slot */
    for (hole = 0; hole < INVEN_PACK; hole++) {
    
        auto_item *item = &auto_items[hole];
        
        /* Notice empty slot */
        if (!item->iqty) break;
    }
    
    /* Need an empty slot */
    if (hole == INVEN_PACK) return (FALSE);


    /* Scan equipment */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {

        auto_item *item = &auto_items[i];
        
        /* Skip empty slots */
        if (!item->iqty) continue;
            
        /* Save the hole */
        COPY(&safe_items[hole], &auto_items[hole], auto_item);

        /* Save the item */
        COPY(&safe_items[i], &auto_items[i], auto_item);

        /* Take off the item */
        COPY(&auto_items[hole], &safe_items[i], auto_item);

        /* Erase the item */
        WIPE(&auto_items[i], auto_item);

        /* Examine the inventory */
        borg_notice();
                
        /* Evaluate the inventory */
        v = borg_power();

        /* Restore the item */
        COPY(&auto_items[i], &safe_items[i], auto_item);

        /* Restore the hole */
        COPY(&auto_items[hole], &safe_items[hole], auto_item);

        /* Track best */
        if ((b_i >= 0) && (b_v >= v)) continue;

        /* Track best */
        b_i = i; b_v = v;
    }

    /* Examine the inventory */
    borg_notice();

    /* Evaluate the inventory */
    v = borg_power();

    /* Prepare to "swap" if needed */
    if ((b_i >= 0) && (b_v >= v)) {

        auto_item *item = &auto_items[b_i];
        
        /* Log */
        borg_note(format("# Taking off %s.", item->desc));

        /* Take off item */
        borg_keypress('t');
        borg_keypress(I2A(b_i - INVEN_WIELD));

        /* Success */
        return (TRUE);
    }
    
    /* Nope */
    return (FALSE);
}




/*
 * Helper function (see below)
 */
static void borg_best_stuff_aux(int n, byte *test, byte *best, s32b *vp)
{
    int i;
    
    int slot;


    /* All done */
    if (n >= 12) {

        s32b p;

        /* Examine */
        borg_notice();

        /* Evaluate */
        p = borg_power();

        /* Track best */
        if (p > *vp) {

            /* Save the results */
            for (i = 0; i < n; i++) best[i] = test[i];

            /* Use it */
            *vp = p;
        }

        /* Success */
        return;
    }


    /* Extract the slot XXX XXX XXX */
    slot = INVEN_WIELD + n;


    /* Note the attempt */
    test[n] = slot;

    /* Evaluate the default item */
    borg_best_stuff_aux(n + 1, test, best, vp);


    /* Try other possible objects */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;
        
        /* Require "aware" */
        if (!item->kind) continue;
        
        /* Require "known" (or average) */
        if (!item->able && !streq(item->note, "{average}")) continue;

        /* Hack -- ignore "worthless" items */
        if (!item->value) continue;


        /* Make sure it goes in this slot */
        if (slot != borg_wield_slot(item)) continue;

        /* Take off old item */
        COPY(&auto_items[i], &safe_items[slot], auto_item);

        /* Wear the new item */
        COPY(&auto_items[slot], &safe_items[i], auto_item);

        /* Note the attempt */
        test[n] = i;

        /* Evaluate the possible item */
        borg_best_stuff_aux(n + 1, test, best, vp);

        /* Restore equipment */
        COPY(&auto_items[slot], &safe_items[slot], auto_item);

        /* Restore inventory */
        COPY(&auto_items[i], &safe_items[i], auto_item);
    }
}



/*
 * Attempt to instantiate the *best* possible equipment.
 */
static bool borg_best_stuff(void)
{
    int k;

    int hole;
    int slot;

    s32b value;

    int i, b_i = -1;
    s32b p, b_p = 0L;

    byte test[12];
    byte best[12];
    
    auto_item *item;


    /* Look for an empty slot */
    for (hole = 0; hole < INVEN_PACK; hole++) {
    
        item = &auto_items[hole];
        
        /* Found an empty slot */
        if (!item->iqty) break;
    }
    
    /* No empty slots */
    if (hole == INVEN_PACK) return (FALSE);


    /* Hack -- Initialize */
    for (k = 0; k < 12; k++) best[k] = 255;

    /* Hack -- Copy all the slots */
    for (i = 0; i < INVEN_TOTAL; i++) {

        /* Save the item */
        COPY(&safe_items[i], &auto_items[i], auto_item);
    }
        

    /* Examine the inventory */
    borg_notice();

    /* Evaluate the inventory */
    value = borg_power();

    /* Determine the best possible equipment */
    (void)borg_best_stuff_aux(0, test, best, &value);


    /* Find "easiest" step */
    for (k = 0; k < 12; k++) {

        /* Get choice */
        i = best[k];
        
        /* Ignore non-changes */
        if (i >= INVEN_WIELD) continue;

        /* Access the item */
        item = &auto_items[i];

        /* Access the slot */
        slot = borg_wield_slot(item);

        /* Save the old item */
        COPY(&safe_items[slot], &auto_items[slot], auto_item);

        /* Save the new item */
        COPY(&safe_items[i], &auto_items[i], auto_item);

        /* Save the hole */
        COPY(&safe_items[hole], &auto_items[hole], auto_item);

        /* Take off old item */
        COPY(&auto_items[hole], &safe_items[slot], auto_item);

        /* Wear new item */
        COPY(&auto_items[slot], &safe_items[i], auto_item);

        /* Only a single item */
        auto_items[slot].iqty = 1;

        /* Reduce the inventory quantity by one */
        auto_items[i].iqty--;
        
        /* Examine the inventory */
        borg_notice();
                
        /* Evaluate the inventory */
        p = borg_power();

        /* Restore the old item */
        COPY(&auto_items[slot], &safe_items[slot], auto_item);

        /* Restore the new item */
        COPY(&auto_items[i], &safe_items[i], auto_item);

        /* Restore the hole */
        COPY(&auto_items[hole], &safe_items[hole], auto_item);

        /* Ignore "bad" swaps */
        if ((b_i >= 0) && (p < b_p)) continue;

        /* Maintain the "best" */
        b_i = i; b_p = p;
    }

    /* Restore bonuses */
    borg_notice();
                
    /* Evaluate the inventory */
    p = borg_power();


    /* Start changing */
    if (b_i >= 0) {

        /* Get the item */
        auto_item *item = &auto_items[b_i];

        /* Log */
        borg_note(format("# Besting %s.", item->desc));

        /* Wear it */
        borg_keypress('w');
        borg_keypress(I2A(b_i));

        /* Did something */
        return (TRUE);
    }
    

    /* Nope */
    return (FALSE);
}






/*
 * Wear useful equipment.
 *
 * Look through the inventory for equipment that is better than
 * the current equipment, and wear it, in an optimal order.
 *
 * Basically, we evaluate the world both with the current set of
 * equipment, and in the alternate world in which various items
 * are used instead of the items they would replace, and we take
 * one step towards the world in which we have the most "power".
 *
 * Although a player can actually wear two rings, we pretend that only
 * the "loose" ring can be removed, which is the semantics induced by
 * the "wear" command.
 *
 * The "borg_swap_rings()" code above occasionally allows us to remove
 * both rings, at which point this function will replace the "best" ring
 * on the "tight" finger, and the second "best" ring on the "loose" finger.
 */
static bool borg_wear_stuff(void)
{
    int slot;
    int hole;

    s32b p, b_p = 0L;

    int i, b_i = -1;

    auto_item *item;


    /* Look for an empty slot */
    for (hole = 0; hole < INVEN_PACK; hole++) {
    
        item = &auto_items[hole];
        
        /* Found an empty slot */
        if (!item->iqty) break;
    }
    
    /* No empty slots */
    if (hole == INVEN_PACK) return (FALSE);


    /* Scan inventory */
    for (i = 0; i < INVEN_PACK; i++) {

        item = &auto_items[i];


        /* Skip empty items */
        if (!item->iqty) continue;
        

        /* Require "aware" */
        if (!item->kind) continue;
        
        /* Require "known" (or average) */
        if (!item->able && !streq(item->note, "{average}")) continue;

        /* Hack -- ignore "worthless" items */
        if (!item->value) continue;


        /* Where does it go */
        slot = borg_wield_slot(item);
    
        /* Cannot wear this item */
        if (slot < 0) continue;

        /* Save the old item */
        COPY(&safe_items[slot], &auto_items[slot], auto_item);

        /* Save the new item */
        COPY(&safe_items[i], &auto_items[i], auto_item);

        /* Save the hole */
        COPY(&safe_items[hole], &auto_items[hole], auto_item);

        /* Take off old item */
        COPY(&auto_items[hole], &safe_items[slot], auto_item);

        /* Wear new item */
        COPY(&auto_items[slot], &safe_items[i], auto_item);

        /* Only a single item */
        auto_items[slot].iqty = 1;

        /* Reduce the inventory quantity by one */
        auto_items[i].iqty--;
        
        /* Examine the inventory */
        borg_notice();
                
        /* Evaluate the inventory */
        p = borg_power();

        /* Restore the old item */
        COPY(&auto_items[slot], &safe_items[slot], auto_item);

        /* Restore the new item */
        COPY(&auto_items[i], &safe_items[i], auto_item);

        /* Restore the hole */
        COPY(&auto_items[hole], &safe_items[hole], auto_item);

        /* Ignore "bad" swaps */
        if ((b_i >= 0) && (p < b_p)) continue;

        /* XXX XXX XXX Consider if slot is empty */

        /* Hack -- Ignore "equal" swaps */
        if ((b_i >= 0) && (p == b_p)) continue;
        
        /* Maintain the "best" */
        b_i = i; b_p = p;
    }

    /* Restore bonuses */
    borg_notice();
                
    /* Evaluate the inventory */
    p = borg_power();

    /* No item */
    if ((b_i >= 0) && (b_p > p)) {
    
        /* Get the item */
        item = &auto_items[b_i];

        /* Log */
        borg_note(format("# Wearing %s.", item->desc));

        /* Wear it */
        borg_keypress('w');
        borg_keypress(I2A(b_i));

        /* Did something */
        return (TRUE);
    }
    
    /* Nope */
    return (FALSE);
}


/*
 * Study important spells as soon as possible
 */
static bool borg_study_magic(void)
{
    /* Study spells/prayers */
    if (borg_study_okay()) {

        /* Mages */
        if (mb_ptr->spell_book == TV_MAGIC_BOOK) {

            /* Hack -- try for magic missile */
            if (borg_study_spell(0,0)) return (TRUE);

            /* Hack -- try for teleport */
            if (borg_study_spell(1,5)) return (TRUE);

            /* Hack -- try for satisfy hunger */
            if (borg_study_spell(2,0)) return (TRUE);

            /* Hack -- try for identify */
            if (borg_study_spell(2,4)) return (TRUE);

            /* Hack -- try for phase door */
            if (borg_study_spell(0,2)) return (TRUE);

            /* Hack -- try for call light */
            if (borg_study_spell(0,3)) return (TRUE);

            /* Hack -- try for stone to mud */
            if (borg_study_spell(1,8)) return (TRUE);

            /* Hack -- try for detect traps/doors */
            if (borg_study_spell(0,7)) return (TRUE);

            /* Hack -- try for cure poison */
            if (borg_study_spell(1,4)) return (TRUE);
        }

        /* Priests */
        if (mb_ptr->spell_book == TV_PRAYER_BOOK) {

            /* Hack -- try for perceptions */
            if (borg_study_prayer(5,2)) return (TRUE);

            /* Hack -- try for portal */
            if (borg_study_prayer(1,1)) return (TRUE);

            /* Hack -- try for satisfy hunger */
            if (borg_study_prayer(1,5)) return (TRUE);

            /* Hack -- try for orb of draining */
            if (borg_study_prayer(2,1)) return (TRUE);

            /* Hack -- try for detect traps */
            if (borg_study_prayer(0,5)) return (TRUE);

            /* Hack -- try for detect doors */
            if (borg_study_prayer(0,6)) return (TRUE);

            /* Hack -- try for cure poison */
            if (borg_study_prayer(2,0)) return (TRUE);

            /* Hack -- try for remove fear */
            if (borg_study_prayer(0,3)) return (TRUE);

            /* Hack -- try for call light */
            if (borg_study_prayer(0,4)) return (TRUE);
        }
    }

    /* Nope */
    return (FALSE);
}


/*
 * Notice "icky" spells/prayers
 */
static bool borg_play_magic_okay(int book, int what)
{
    /* Mages */
    if (mb_ptr->spell_book == TV_MAGIC_BOOK) {

        /* Ignore "Word of Destruction" */
        if ((book == 3) && (what == 5)) return (FALSE);
        
        /* Ignore "Earthquake" */
        if ((book == 5) && (what == 3)) return (FALSE);

        /* Ignore "Word of Recall" */
        if ((book == 5) && (what == 4)) return (FALSE);
    }
    
    /* Priests */
    if (mb_ptr->spell_book == TV_PRAYER_BOOK) {
    
        /* Ignore "Earthquake" */
        if ((book == 2) && (what == 5)) return (FALSE);
        
        /* Ignore "Word of Recall" */
        if ((book == 4) && (what == 4)) return (FALSE);
        
        /* Ignore "Word of Destruction" */
        if ((book == 8) && (what == 3)) return (FALSE);
    }

    /* Okay */
    return (TRUE);
}


/*
 * Study/use "extra" magic to gain levels
 */
static bool borg_play_magic(void)
{
    int i, book, what;


    /* Hack -- only in town */
    if (auto_depth) return (FALSE);


    /* Hack -- must use magic or prayers */
    if (!mb_ptr->spell_book) return (FALSE);

    /* Hack -- blind/confused */
    if (do_blind || do_confused) return (FALSE);


    /*** Use every (safe) spell once ***/

    /* Check each book (backwards) */
    for (book = 9 - 1; book >= 0; book--) {

        /* Look for the book */
        i = borg_book(book);

        /* Make sure we have the book */
        if (i < 0) continue;

        /* Check every spell */
        for (what = 9 - 1; what >= 0; what--) {

            auto_magic *as = &auto_magics[book][what];

            /* Only try "untried" spells/prayers */
            if (as->status != BORG_MAGIC_TEST) continue;

            /* Ignore "bizarre" spells/prayers */
            if (as->method == BORG_MAGIC_OBJ) continue;
            if (as->method == BORG_MAGIC_WHO) continue;

            /* Ignore "dangerous" or "confusing" spells */
            if (!borg_play_magic_okay(book, what)) continue;

            /* Note */
            borg_note("# Testing 'silly' spell/prayer");

            /* Hack -- Use spell or prayer */
            if (borg_spell(book, what) || borg_prayer(book, what)) {

                /* Hack -- Allow attack spells */
                if (as->method == BORG_MAGIC_AIM) {

                    /* Hack -- target self */
                    borg_keypress('*');
                    borg_keypress('p');
                    borg_keypress('t');
                }

                /* Success */
                return (TRUE);
            }
        }
    }


    /*** Learn every spell ***/

    /* Require "study" flag */
    if (!do_study) return (FALSE);
    
    /* Check each book (backwards) */
    for (book = 9 - 1; book >= 0; book--) {

        /* Look for the book */
        i = borg_book(book);

        /* Make sure we have the book */
        if (i < 0) continue;

        /* Check for spells */
        for (what = 9 - 1; what >= 0; what--) {

            auto_magic *as = &auto_magics[book][what];

            /* Only study "learnable" spells */
            if (as->status != BORG_MAGIC_OKAY) continue;

            /* Note */
            borg_note("# Studying 'silly' spell/prayer");

            /* Hack -- Learn it */
            if (borg_study_spell(book, what)) return (TRUE);
            if (borg_study_prayer(book, what)) return (TRUE);
        }
    }

    /* Nope */
    return (FALSE);
}




/*
 * Determine if the Borg is out of "crucial" supplies.
 */
static bool borg_restock(void)
{
    /* Always ready for the town */
    if (!auto_depth) return (FALSE);


    /*** Level 1 ***/
    
    /* Must have some lite */
    if (my_cur_lite < 1) return (TRUE);

    /* Must have "fuel" */
    if (amt_fuel < 1) return (TRUE);

    /* Must have "food" */
    if (amt_food < 1) return (TRUE);

    /* Assume happy at level 1 */
    if (auto_depth <= 1) return (FALSE);


    /*** Level 2 and below ***/

    /* Must have good lite */
    if (my_cur_lite < 2) return (TRUE);

    /* Must have "fuel" */
    if (amt_fuel < 5) return (TRUE);

    /* Must have "food" */
    if (amt_food < 5) return (TRUE);

    /* Must have "recall" */
    if (amt_recall < 2) return (TRUE);

    /* Assume happy */
    return (FALSE);
}


/*
 * Help the Borg decide if he is "prepared" for the given level
 *
 * This routine does not help him decide how to get ready for the
 * given level, so it must work closely with "borg_power()".
 */
static bool borg_prepared(int depth)
{
    /* Always ready for the town */
    if (!depth) return (TRUE);


    /*** Essential Items for Level 1 ***/

    /* Require lite (any) */
    if (my_cur_lite < 1) return (FALSE);

    /* Require food */
    if (amt_food < 10) return (FALSE);

    /* Usually ready for level 1 */
    if (depth <= 1) return (TRUE);


    /*** Essential Items for Level 2 ***/

    /* Require lite (radius two) */
    if (my_cur_lite < 2) return (FALSE);

    /* Require fuel */
    if (amt_fuel < 10) return (FALSE);

    /* Require recall */
    if (amt_recall < 2) return (FALSE);

    /* Scrolls of Identify (for identification) */
    if (amt_ident < 5) return (FALSE);

    /* Usually ready for level 2 */
    if (depth <= 2) return (TRUE);


    /*** Essential Items for Level 3 and 4 ***/

    /* Scrolls of Word of Recall */
    if (amt_recall < 3) return (FALSE);

    /* Scrolls of Identify */
    if (amt_ident < 10) return (FALSE);

    /* Potions of Cure Serious Wounds */
    if (amt_cure_serious + amt_cure_critical < 2) return (FALSE);

    /* Usually ready for level 3 and 4 */
    if (depth <= 4) return (TRUE);


    /*** Essential Items for Level 5 to 9 ***/

    /* Scrolls of Word of Recall */
    if (amt_recall < 4) return (FALSE);

    /* Scrolls of Identify */
    if (amt_ident < 15) return (FALSE);

    /* Potions of Cure Serious/Critical Wounds */
    if (amt_cure_serious + amt_cure_critical < 5) return (FALSE);

    /* Usually ready for level 5 to 9 */
    if (depth <= 9) return (TRUE);


    /*** Essential Items for Level 10 to 19 ***/

    /* Teleport */
    if (amt_teleport + amt_escape < 2) return (FALSE);

    /* Identify */
    if (amt_ident < 20) return (FALSE);

    /* Potions of Cure Critical Wounds */
    if (amt_cure_critical < 10) return (FALSE);

    /* XXX XXX See invisible */
    if (!my_see_inv) return (FALSE);

    /* Usually ready for level 10 to 19 */
    if (depth <= 19) return (TRUE);


    /*** Essential Items for Level 20 to 39 ***/

    /* Escape */
    if (amt_escape < 2) return (FALSE);
    if (amt_teleport < 2) return (FALSE);

    /* Cure Critical Wounds */
    if (amt_cure_critical < 15) return (FALSE);

    /* XXX XXX Free action */
    if (!my_free_act) return (FALSE);

    /* Usually ready for level 20 to 39 */
    if (depth <= 39) return (TRUE);


    /*** Essential Items for Level 40 to 99 ***/

    /* Minimal level */
    if (auto_level < 40) return (FALSE);

    /* Minimal hitpoints */
    if (auto_chp < 400) return (FALSE);

    /* High stats */
    if (auto_stat[A_STR] < 18+70) return (FALSE);
    if (auto_stat[A_INT] < 18+70) return (FALSE);
    if (auto_stat[A_WIS] < 18+70) return (FALSE);
    if (auto_stat[A_DEX] < 18+70) return (FALSE);
    if (auto_stat[A_CON] < 18+70) return (FALSE);
    if (auto_stat[A_CHR] < 18+70) return (FALSE);

#if 0
    /* XXX XXX XXX Hold Life */
    if (!my_hold_life) return (FALSE);

    /* XXX XXX XXX Resist Disenchantment */
    if (!my_resist_disen) return (FALSE);
#endif

    /* Usually ready for level 40 to 99 */
    if (depth <= 99) return (TRUE);


    /*** Essential Items for Level 100 ***/

    /* Minimal level */
    if (auto_level < 50) return (FALSE);

    /* Minimal hitpoints */
    if (auto_chp < 750) return (FALSE);

    /* Assume ready */
    return (TRUE);
}




/*
 * Leave the level if necessary (or bored)
 */
static bool borg_leave_level(bool bored)
{
    int k, g = 0;


    /* Cancel stair requests */
    stair_less = stair_more = FALSE;


    /* Hack -- already leaving via "recall" */
    if (goal_recalling) return (FALSE);


    /* Count sellable items */
    k = borg_count_sell();


    /* Power-dive (if legal) */
    if (auto_depth && (auto_level > auto_depth * 2) &&
        borg_prepared(auto_depth+1)) g = 1;

    /* Dive when bored */
    if (auto_depth && bored && (c_t - auto_shock >= 1000)) g = 1;

    /* Mega-Hack -- Stay on a level for a minimal amount of time */
    if (c_t - auto_began < value_feeling[auto_feeling]) g = 0;

    /* Do not dive when "full" of items */
    if (g && (k >= 18)) g = 0;

    /* Hack -- do not dive when drained */
    if (do_fix_exp) g = 0;


    /* Return to town (immediately) if we are worried */
    if (!borg_prepared(auto_depth)) goal_rising = TRUE;

    /* Return to town instead of entering a dangerous level */
    if ((g > 0) && !borg_prepared(auto_depth+1)) goal_rising = TRUE;

    /* Return to town to sell stuff */
    if (bored && (k >= 18)) goal_rising = TRUE;

    /* Return to town when level drained */
    if (do_fix_lev) goal_rising = TRUE;

    /* Hack -- return to town to restore experience */
    if (bored && do_fix_exp) goal_rising = TRUE;

    /* Hack -- Never rise from town */
    if (!auto_depth) goal_rising = FALSE;

    /* Return to town */
    if (goal_rising) g = -1;

    /* Mega-Hack -- allow "stair scumming" at 50 feet to rotate stores */
    if ((auto_depth == 1) && (c_t - auto_began < 200) && (g < 0)) g = 0;


    /* Bored and in town, go down */
    if (bored && !auto_depth) {

        /* Mega-Hack -- Recall into dungeon */
        if (auto_max_depth && (auto_max_depth >= 5) &&
            (amt_recall > 4) &&
            borg_prepared(auto_max_depth) &&
            borg_recall()) {

            /* Note */
            borg_note("# Recalling into dungeon (for fun)...");

            /* Give it a shot */
            return (TRUE);
        }

        /* Go down */
        g = 1;
    }


    /* Use random stairs when really bored */
    if (auto_depth && bored && (c_t - auto_shock >= 3000)) {

        /* Note */
        borg_note("# Choosing random stairs.");

        /* Use random stairs */
        g = ((rand_int(100) < 50) ? -1 : 1);
    }


    /* Go Up */
    if (g < 0) {

        /* Hack -- use scroll of recall if able */
        if (goal_rising && (auto_depth > 4) &&
            (amt_recall > 3)) {
            if (borg_recall()) {
                borg_note("# Recalling to town (for fun)");
                return (TRUE);
            }
        }

        /* Attempt to use stairs */
        if (borg_flow_stair_less()) {
            stair_less = TRUE;
            if (borg_play_old_goal()) return (TRUE);
        }

        /* Cannot find any stairs, try word of recall */
        if (goal_rising && bored && auto_depth) {
            if (borg_recall()) {
                borg_note("# Recalling to town (no stairs)");
                return (TRUE);
            }
        }
    }


    /* Go Down */
    if (g > 0) {

        /* Attempt to use those stairs */
        if (borg_flow_stair_more()) {
            stair_more = TRUE;
            if (borg_play_old_goal()) return (TRUE);
        }
    }


    /* Failure */
    return (FALSE);
}




/*
 * Determine if an item can be sold in the given store
 *
 * XXX XXX XXX Consider use of "icky" test on items
 */
static bool borg_good_sell(auto_item *item, int who)
{
    /* Never sell worthless items */
    if (item->value <= 0) return (FALSE);

    /* Analyze the type */
    switch (item->tval) {

        case TV_POTION:
        case TV_SCROLL:
        
            /* Never sell if not "known" and interesting */
            if (!item->able && (auto_max_depth > 5)) return (FALSE);
            break;

        case TV_FOOD:
        case TV_ROD:
        case TV_WAND:
        case TV_STAFF:
        case TV_RING:
        case TV_AMULET:
        case TV_LITE:

            /* Never sell if not "known" */
            if (!item->able) return (FALSE);
            break;

        case TV_BOW:
        case TV_DIGGING:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_SWORD:
        case TV_BOOTS:
        case TV_GLOVES:
        case TV_HELM:
        case TV_CROWN:
        case TV_SHIELD:
        case TV_CLOAK:
        case TV_SOFT_ARMOR:
        case TV_HARD_ARMOR:
        case TV_DRAG_ARMOR:

            /* Only sell "known" (or "average") items (unless "icky") */
            if (!item->able && !streq(item->note, "{average}") &&
                !borg_item_icky(item)) return (FALSE);

            break;
    }


    /* Switch on the store */
    switch (who + 1) {

      /* General Store */
      case 1:

        /* Analyze the type */
        switch (item->tval) {
          case TV_DIGGING:
          case TV_CLOAK:
          case TV_FOOD:
          case TV_FLASK:
          case TV_LITE:
          case TV_SPIKE:
            return (TRUE);
        }
        break;

      /* Armoury */
      case 2:

        /* Analyze the type */
        switch (item->tval) {
          case TV_BOOTS:
          case TV_GLOVES:
#if 0
          case TV_CLOAK:
#endif
          case TV_HELM:
          case TV_CROWN:
          case TV_SHIELD:
          case TV_SOFT_ARMOR:
          case TV_HARD_ARMOR:
          case TV_DRAG_ARMOR:
            return (TRUE);
        }
        break;

      /* Weapon Shop */
      case 3:

        /* Analyze the type */
        switch (item->tval) {
          case TV_SHOT:
          case TV_BOLT:
          case TV_ARROW:
          case TV_BOW:
#if 0
          case TV_DIGGING:
          case TV_HAFTED:
#endif
          case TV_POLEARM:
          case TV_SWORD:
            return (TRUE);
        }
        break;

      /* Temple */
      case 4:

        /* Analyze the type */
        switch (item->tval) {
          case TV_HAFTED:
#if 0
          case TV_SCROLL:
          case TV_POTION:
#endif
          case TV_PRAYER_BOOK:
            return (TRUE);
        }
        break;

      /* Alchemist */
      case 5:

        /* Analyze the type */
        switch (item->tval) {
          case TV_SCROLL:
          case TV_POTION:
            return (TRUE);
        }
        break;

      /* Magic Shop */
      case 6:

        /* Analyze the type */
        switch (item->tval) {
          case TV_MAGIC_BOOK:
          case TV_AMULET:
          case TV_RING:
#if 0
          case TV_SCROLL:
          case TV_POTION:
#endif
          case TV_STAFF:
          case TV_WAND:
          case TV_ROD:
            return (TRUE);
        }
        break;


      /* Black Market */
      case 7:

        /* Analyze the type */
        switch (item->tval) {
          case TV_CHEST:
            return (TRUE);
        }
        break;
    }

    /* Assume not */
    return (FALSE);
}



/*
 * Hack -- find something to sell to the home
 */
static bool borg_think_home_sell_aux(void)
{
    int icky = 23;

    int i, b_i = -1;
    s32b p, b_p = 0L;
    s32b s, b_s = 0L;


    /* Hack -- the home is full */
    if (auto_shops[7].ware[icky].iqty) return (FALSE);


    /* Examine the player */
    borg_notice();

    /* Evaluate the player */
    b_p = borg_power();


    /* Examine the home */
    borg_notice_aux3();

    /* Evaluate the home */
    b_s = borg_power_aux3();


    /* Save the store hole */
    COPY(&safe_shops[7].ware[icky], &auto_shops[7].ware[icky], auto_item);

    /* Sell stuff */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Save the item */
        COPY(&safe_items[i], &auto_items[i], auto_item);

        /* Give the item to the shop */
        COPY(&auto_shops[7].ware[icky], &safe_items[i], auto_item);

        /* Give a single item */
        auto_shops[7].ware[icky].iqty = 1;

        /* Lose a single item */
        auto_items[i].iqty--;

        /* Examine the inventory */
        borg_notice();

        /* Evaluate the inventory */
        p = borg_power();

        /* Restore the item */
        COPY(&auto_items[i], &safe_items[i], auto_item);

        /* Ignore "bad" sales */
        if (p < b_p) continue;

        /* Examine the home */
        borg_notice_aux3();

        /* Evaluate the home */
        s = borg_power_aux3();

        /* Ignore "silly" sales (except artifacts) XXX XXX XXX */
        if ((p == b_p) && (s <= b_s) && !item->name1) continue;

        /* Maintain the "best" */
        b_i = i; b_p = p; b_s = s;
    }

    /* Restore the store hole */
    COPY(&auto_shops[7].ware[icky], &safe_shops[7].ware[icky], auto_item);
    
    /* Examine the player */
    borg_notice();

    /* Evaluate the player */
    p = borg_power();

    /* Examine the home */
    borg_notice_aux3();

    /* Evaluate the home */
    s = borg_power_aux3();

    /* Stockpile */
    if (b_i >= 0) {

        /* Visit the home */
        goal_shop = 7;

        /* Sell that item */
        goal_item = b_i;

        /* Success */
        return (TRUE);
    }

    /* Assume not */
    return (FALSE);
}


/*
 * Hack -- find something to sell in a shop
 */
static bool borg_think_shop_sell_aux(void)
{
    int icky = 23;

    int k, b_k = -1;
    int i, b_i = -1;
    s32b p, b_p = 0L;
    s32b c, b_c = 0L;
    

    /* Examine the inventory */
    borg_notice();

    /* Evaluate */
    b_p = borg_power();


    /* Check each shop */
    for (k = 0; k < 7; k++) {

        /* Hack -- Skip "full" shops */
        if (auto_shops[k].ware[icky].iqty) continue;

        /* Save the store hole */
        COPY(&safe_shops[k].ware[icky], &auto_shops[k].ware[icky], auto_item);

        /* Sell stuff */
        for (i = 0; i < INVEN_PACK; i++) {

            auto_item *item = &auto_items[i];

            /* Skip empty items */
            if (!item->iqty) continue;

            /* Hack -- never sell artifacts */
            if (item->name1) continue;

            /* Skip "bad" sales */
            if (!borg_good_sell(item, k)) continue;

            /* Save the item */
            COPY(&safe_items[i], &auto_items[i], auto_item);

            /* Give the item to the shop */
            COPY(&auto_shops[k].ware[icky], &safe_items[i], auto_item);

            /* Give a single item */
            auto_shops[k].ware[icky].iqty = 1;

            /* Lose a single item */
            auto_items[i].iqty--;

            /* Examine the inventory */
            borg_notice();

            /* Evaluate the inventory */
            p = borg_power();

            /* Restore the item */
            COPY(&auto_items[i], &safe_items[i], auto_item);

            /* Ignore "bad" sales */
            if (p < b_p) continue;

            /* Extract the "price" */
            c = ((item->value < 30000L) ? item->value : 30000L);

            /* Ignore "cheaper" items */
            if ((p == b_p) && (c <= b_c)) continue;

            /* Maintain the "best" */
            b_k = k; b_i = i; b_p = p; b_c = c;
        }

        /* Restore the store hole */
        COPY(&auto_shops[k].ware[icky], &safe_shops[k].ware[icky], auto_item);
    }
    
    /* Examine the inventory */
    borg_notice();

    /* Evaluate */
    p = borg_power();

    /* Sell something (if useless) */
    if ((b_k >= 0) && (b_i >= 0)) {

        /* Visit that shop */
        goal_shop = b_k;

        /* Sell that item */
        goal_item = b_i;

        /* Success */
        return (TRUE);
    }

    /* Assume not */
    return (FALSE);
}



/*
 * Hack -- find something to purchase from the shops
 */
static bool borg_think_shop_buy_aux(void)
{
    int hole;
    int slot;

    int k, b_k = -1;
    int n, b_n = -1;
    s32b p, b_p = 0L;
    s32b c, b_c = 0L;


    /* Look for an empty slot */
    for (hole = 0; hole < INVEN_PACK; hole++) {
    
        auto_item *item = &auto_items[hole];
        
        /* Found an empty slot */
        if (!item->iqty) break;
    }
    
    /* No empty slots */
    if (hole == INVEN_PACK) return (FALSE);


    /* Examine the inventory */
    borg_notice();
    
    /* Extract the "power" */
    b_p = borg_power();


    /* Check the shops */
    for (k = 0; k < 7; k++) {

        /* Scan the wares */
        for (n = 0; n < 24; n++) {

            auto_item *item = &auto_shops[k].ware[n];

            /* Skip empty items */
            if (!item->iqty) continue;

            /* Hack -- Require "sufficient" cash */
            if (auto_gold < item->cost * 12 / 10) continue;

            /* Save shop item */
            COPY(&safe_shops[k].ware[n], &auto_shops[k].ware[n], auto_item);

            /* Save hole */
            COPY(&safe_items[hole], &auto_items[hole], auto_item);

            /* Remove one item from shop */
            auto_shops[k].ware[n].iqty--;

            /* Obtain "slot" */
            slot = borg_wield_slot(item);

            /* Consider new equipment */
            if (slot >= 0) {

                /* Save old item */
                COPY(&safe_items[slot], &auto_items[slot], auto_item);
                
                /* Move equipment into inventory */
                COPY(&auto_items[hole], &safe_items[slot], auto_item);

                /* Move new item into equipment */
                COPY(&auto_items[slot], &safe_shops[k].ware[n], auto_item);

                /* Only a single item */
                auto_items[slot].iqty = 1;

                /* Examine the inventory */
                borg_notice();
                
                /* Evaluate the inventory */
                p = borg_power();

                /* Restore old item */
                COPY(&auto_items[slot], &safe_items[slot], auto_item);
            }

            /* Consider new inventory */
            else {

                /* Move new item into inventory */
                COPY(&auto_items[hole], &safe_shops[k].ware[n], auto_item);

                /* Only a single item */
                auto_items[hole].iqty = 1;

                /* Examine the inventory */
                borg_notice();
                
                /* Evaluate the equipment */
                p = borg_power();
            }

            /* Restore hole */
            COPY(&auto_items[hole], &safe_items[hole], auto_item);
                
            /* Restore shop item */
            COPY(&auto_shops[k].ware[n], &safe_shops[k].ware[n], auto_item);

            /* Obtain the "cost" of the item */
            c = item->cost;
            
            /* Penalize the cost of expensive items */
            if (c > auto_gold / 10) p -= c;
            
            /* Ignore "bad" purchases */
            if (p < b_p) continue;

            /* Ignore "expensive" purchases */
            if ((p == b_p) && (c >= b_c)) continue;

            /* Save the item and cost */
            b_k = k; b_n = n; b_p = p; b_c = c;
        }
    }

    /* Examine the inventory */
    borg_notice();
    
    /* Extract the "power" */
    p = borg_power();

    /* Buy something */
    if ((b_k >= 0) && (b_n >= 0)) {

        /* Visit that shop */
        goal_shop = b_k;

        /* Buy that item */
        goal_ware = b_n;

        /* Success */
        return (TRUE);
    }

    /* Nope */
    return (FALSE);
}


/*
 * Hack -- find something to purchase from the home
 */
static bool borg_think_home_buy_aux(void)
{
    int hole;
    int slot;

    int n, b_n = -1;
    s32b p, b_p = 0L;


    /* Look for an empty slot */
    for (hole = 0; hole < INVEN_PACK; hole++) {
    
        auto_item *item = &auto_items[hole];
        
        /* Found an empty slot */
        if (!item->iqty) break;
    }
    
    /* No empty slots */
    if (hole == INVEN_PACK) return (FALSE);


    /* Examine the inventory */
    borg_notice();
    
    /* Extract the "power" */
    b_p = borg_power();


    /* Scan the home */
    for (n = 0; n < 24; n++) {

        auto_item *item = &auto_shops[7].ware[n];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Save shop item */
        COPY(&safe_shops[7].ware[n], &auto_shops[7].ware[n], auto_item);

        /* Save hole */
        COPY(&safe_items[hole], &auto_items[hole], auto_item);

        /* Remove one item from shop */
        auto_shops[7].ware[n].iqty--;

        /* Obtain "slot" */
        slot = borg_wield_slot(item);

        /* Consider new equipment */
        if (slot >= 0) {

            /* Save old item */
            COPY(&safe_items[slot], &auto_items[slot], auto_item);
                
            /* Move equipment into inventory */
            COPY(&auto_items[hole], &safe_items[slot], auto_item);

            /* Move new item into equipment */
            COPY(&auto_items[slot], &safe_shops[7].ware[n], auto_item);

            /* Only a single item */
            auto_items[slot].iqty = 1;

            /* Examine the inventory */
            borg_notice();
                
            /* Evaluate the inventory */
            p = borg_power();

            /* Restore old item */
            COPY(&auto_items[slot], &safe_items[slot], auto_item);
        }

        /* Consider new inventory */
        else {

            /* Move new item into inventory */
            COPY(&auto_items[hole], &safe_shops[7].ware[n], auto_item);

            /* Only a single item */
            auto_items[hole].iqty = 1;

            /* Examine the inventory */
            borg_notice();
                
            /* Evaluate the equipment */
            p = borg_power();
        }

        /* Restore hole */
        COPY(&auto_items[hole], &safe_items[hole], auto_item);
                
        /* Restore shop item */
        COPY(&auto_shops[7].ware[n], &safe_shops[7].ware[n], auto_item);

        /* Ignore "silly" purchases */
        if (p <= b_p) continue;

        /* Save the item and cost */
        b_n = n; b_p = p;
    }

    /* Examine the inventory */
    borg_notice();
    
    /* Extract the "power" */
    p = borg_power();

    /* Buy something */
    if ((b_n >= 0) && (b_p > p)) {

        /* Go to the home */
        goal_shop = 7;

        /* Buy that item */
        goal_ware = b_n;

        /* Success */
        return (TRUE);
    }

    /* Nope */
    return (FALSE);
}





/*
 * Choose a shop to visit.
 *
 * (1) Sell items to the home (for later use)
 *     We stockpile items which may be useful later
 *
 * (2) Sell items to the shops (for money)
 *     We sell anything we do not actually need
 *
 * (3) Buy items from the shops (for the player)
 *     We buy things that we actually need
 *
 * (4) Buy items from the home (for the player)
 *     We buy things that we actually need
 *
 * (5) Buy items from the shops (for stockpile) XXX XXX XXX
 *     We buy things we may need later
 *
 * (6) Buy items from the home (for random use) XXX XXX XXX
 *     We attempt to remove items which are no longer useful
 *
 * The basic principle is that we should always act to improve our
 * "status", and we should sometimes act to "maintain" our status,
 * especially if there is a monetary reward.  But first we should
 * attempt to use the home as a "stockpile", even though that is
 * not worth any money, since it may save us money eventually.
 */
static bool borg_choose_shop(void)
{
    int i;


    /* Must be in town */
    if (auto_depth) return (FALSE);

    /* Must have complete information */
    for (i = 0; i < 8; i++) {

        auto_shop *shop = &auto_shops[i];

        /* Skip "visited" shops */
        if (!shop->when) return (FALSE);
    }


    /* Assume no important shop */
    goal_shop = goal_ware = goal_item = -1;


    /* Step 1 -- Sell items to the home */
    if (borg_think_home_sell_aux()) {

        /* Message */
        borg_note(format("# Selling '%s' to the home",
                         auto_items[goal_item].desc));

        /* Success */
        return (TRUE);
    }


    /* Step 2 -- Sell items to the shops */
    if (borg_think_shop_sell_aux()) {

        /* Message */
        borg_note(format("# Selling '%s' at '%s'",
                         auto_items[goal_item].desc,
                         (f_name + f_info[0x08+goal_shop].name)));

        /* Success */
        return (TRUE);
    }


    /* Step 3 -- Buy items from the shops */
    if (borg_think_shop_buy_aux()) {

        /* Message */
        borg_note(format("# Buying '%s' at '%s'",
                         auto_shops[goal_shop].ware[goal_ware].desc,
                         (f_name + f_info[0x08+goal_shop].name)));

        /* Success */
        return (TRUE);
    }


    /* Step 4 -- Buy items from the home */
    if (borg_think_home_buy_aux()) {

        /* Message */
        borg_note(format("# Buying '%s' from the home",
                         auto_shops[goal_shop].ware[goal_ware].desc));

        /* Success */
        return (TRUE);
    }


    /* Failure */
    return (FALSE);
}




/*
 * Prevent starvation by any means possible
 */
static bool borg_eat_food_any(void)
{
    int i;

    /* Scan the inventory for "normal" food */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip unknown food */
        if (!item->kind) continue;

        /* Skip non-food */
        if (item->tval != TV_FOOD) continue;

        /* Skip "flavored" food */
        if (item->sval < SV_FOOD_MIN_FOOD) continue;

        /* Eat something of that type */
        if (borg_eat_food(item->sval)) return (TRUE);
    }

    /* Scan the inventory for "okay" food */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip unknown food */
        if (!item->kind) continue;

        /* Skip non-food */
        if (item->tval != TV_FOOD) continue;

        /* Skip "icky" food */
        if (item->sval < SV_FOOD_MIN_OKAY) continue;

        /* Eat something of that type */
        if (borg_eat_food(item->sval)) return (TRUE);
    }

    /* Nothing */
    return (FALSE);
}



/*
 * Perform an action in the dungeon
 *
 * Return TRUE if a "meaningful" action was performed
 * Otherwise, return FALSE so we will be called again
 *
 * Strategy:
 *   Make sure we are happy with our "status" (see above)
 *   Attack and kill visible monsters, if near enough
 *   Open doors, disarm traps, tunnel through rubble
 *   Pick up (or tunnel to) gold and useful objects
 *   Explore "interesting" grids, to expand the map
 *   Explore the dungeon and revisit old grids
 *
 * Fleeing:
 *   Use word of recall when level is "scary"
 *   Flee to stairs when there is a chance of death
 *   Avoid "stair bouncing" if at all possible
 */
bool borg_think_dungeon(void)
{
    auto_grid *ag = grid(c_x,c_y);

    
    /* Hack -- use "recall" to flee if possible */
    if (goal_fleeing && (auto_level > 0) && (borg_recall())) {
    
        /* Note */
        borg_note("# Fleeing the level (recall)");
        
        /* Success */
        return (TRUE);
    }

    /* Hack -- take stairs up */
    if (stair_less && (ag->o_c == '<')) {

        /* Take the stairs */
        borg_keypress('<');

        /* Success */
        return (TRUE);
    }
    
    /* Hack -- take stairs down */
    if (stair_more && (ag->o_c == '>')) {

        /* Take the stairs */
        borg_keypress('>');

        /* Success */
        return (TRUE);
    }

    /* Hack -- prevent clock wrapping */
    if (c_t >= 30000) {
    
        /* Panic */
        borg_oops("clock overflow");

        /* Oops */
        return (TRUE);
    }

    /* Panic -- avoid possible death */
    if (panic_death &&
        (auto_chp < 100) &&
        (auto_chp < auto_mhp / 4)) {

        /* Panic */
        borg_oops("panic_death");

        /* Oops */
        return (TRUE);
    }


    /*** Deal with crucial stuff ***/

    /* Examine the inventory */
    borg_notice();
    
    /* Must have light -- Refuel current torch */
    if (!my_lite) {

        auto_item *item = &auto_items[INVEN_LITE];

        /* Must have light -- Refuel current torch */
        if ((item->tval == TV_LITE) && (item->sval == SV_LITE_TORCH)) {

            /* Try to refuel the torch */
            if ((item->pval < 250) && borg_refuel_torch()) return (TRUE);
        }

        /* Must have light -- Refuel current lantern */
        if ((item->tval == TV_LITE) && (item->sval == SV_LITE_LANTERN)) {

            /* Try to refill the lantern */
            if ((item->pval < 500) && borg_refuel_lantern()) return (TRUE);
        }
    }

    /* Hack -- Handle starvation */
    if (do_weak) {

        /* Note */
        borg_note("# Starving");

        /* Attempt to satisfy hunger */
        if (borg_eat_food_any() ||
            borg_spell(2,0) ||
            borg_prayer(1,5)) {

            /* Success */
            return (TRUE);
        }

        /* Hack -- use "recall" to flee if possible */
        if ((auto_level > 0) && (borg_recall())) return (TRUE);

        /* Hack -- return to town */
        goal_rising = TRUE;
    }

    /* Hack -- Crucial supplies */
    if (borg_restock()) {
    
        /* Note */
        borg_note("# Need to restock!");

        /* Hack -- use "recall" to flee if possible */
        if ((auto_level > 0) && (borg_recall())) return (TRUE);

        /* Return to town */
        goal_rising = TRUE;
    }

    /* Try not to die */
    if (borg_caution()) return (TRUE);

    /* Wear things that need to be worn */
    if (borg_wear_stuff()) return (TRUE);

    /* Learn useful spells as soon as possible */
    if (borg_study_magic()) return (TRUE);


    /*** Attempt to leave the level ***/

    /* Hack -- Flee to town -- low hit points */
    if ((auto_chp < auto_mhp / 2) &&
        (amt_cure_critical < 5) &&
        (rand_int(100) < 25)) {

        /* Note */
        borg_note("# Low on hit-points");
        
        /* Flee this level */
        goal_fleeing = TRUE;
    }

    /* Hack -- prevent clock overflow */
    if (c_t - auto_began >= 20000) {

        /* Note */
        borg_note("# Extremely bored!");

        /* Flee this level */
        goal_fleeing = TRUE;
        
        /* Hack -- ignore monsters */
        goal_ignoring = TRUE;
    }

    /* Hack -- use "recall" to flee if possible */
    if (goal_fleeing && (auto_level > 0) && (borg_recall())) {
    
        /* Note */
        borg_note("# Fleeing the level (recall)");
        
        /* Success */
        return (TRUE);
    }


    /*** Deal with monsters ***/

    /* Attack neighboring monsters */
    if (borg_attack()) return (TRUE);

    /* Fire at nearby monsters */
    if (borg_launch()) return (TRUE);

    /* Recover from damage */
    if (borg_recover()) return (TRUE);

    /* Continue flowing towards monsters */
    if (goal == GOAL_KILL) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Find a (viewable) monster */
    if (borg_flow_kill(TRUE)) {
        if (borg_play_old_goal()) return (TRUE);
    }


    /*** Deal with inventory objects ***/

    /* Check the light */
    if (borg_check_lite()) return (TRUE);

    /* Use things */
    if (borg_use_things()) return (TRUE);

    /* Identify unknown things */
    if (borg_test_stuff()) return (TRUE);

    /* Enchant things */
    if (borg_enchant_to_d()) return (TRUE);
    if (borg_enchant_to_a()) return (TRUE);
    if (borg_enchant_to_h()) return (TRUE);

    /* XXX Recharge things */

    /* Throw away junk */
    if (borg_crush_junk()) return (TRUE);

    /* Acquire free space */
    if (borg_free_space()) return (TRUE);


    /*** Flow towards terrain ***/

    /* Continue flowing towards terrain */
    if (goal == GOAL_WANK) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Find a (viewable) terrain */
    if (borg_flow_wank(TRUE)) {
        if (borg_play_old_goal()) return (TRUE);
    }


    /*** Flow towards objects ***/

    /* Continue flowing towards objects */
    if (goal == GOAL_TAKE) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Find a (viewable) object */
    if (borg_flow_take(TRUE)) {
        if (borg_play_old_goal()) return (TRUE);
    }


    /*** Leave the level ***/

    /* Hack -- Flee the level */
    if (goal_fleeing) {

        /* Hack -- Take the next stairs */
        stair_less = stair_more = TRUE;

        /* Continue fleeing the level */
        if (goal == GOAL_FLEE) {
            if (borg_play_old_goal()) return (TRUE);
        }

        /* Try to find some stairs */
        if (borg_flow_stair_both()) {

            /* Note */
            borg_note("# Fleeing this level (stairs)");
            
            /* Step towards the stairs */
            if (borg_play_old_goal()) return (TRUE);
        }


        /* Note */
        borg_note("# Boosting bravery!");

        /* Hack -- ignore some danger */
        avoidance = (auto_chp * 2);

        /* Forget the danger fields */
        auto_danger_wipe = TRUE;

        /* Hack -- try to go up */
        if (borg_flow_stair_both()) {

            /* Note */
            borg_note("# Fleeing this level (stairs)");
            
            /* Step towards the stairs */
            if (borg_play_old_goal()) return (TRUE);
        }


        /* Note */
        borg_note("# Infinite bravery!");

        /* Hack -- ignore all danger */
        avoidance = 30000;

        /* Forget the danger fields */
        auto_danger_wipe = TRUE;

        /* Hack -- try to go up */
        if (borg_flow_stair_both()) {

            /* Note */
            borg_note("# Fleeing this level (stairs)");
            
            /* Step towards the stairs */
            if (borg_play_old_goal()) return (TRUE);
        }

        /* Hack -- restore avoidance */
        avoidance = auto_chp;

        /* Forget the danger fields */
        auto_danger_wipe = TRUE;
    }
    

    /*** Exploration ***/

    /* Continue flowing (see below) */
    if (goal == GOAL_MISC) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Continue flowing (see below) */
    if (goal == GOAL_DARK) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Continue flowing (see below) */
    if (goal == GOAL_XTRA) {
        if (borg_play_old_goal()) return (TRUE);
    }


    /*** Try grids that are farther away ***/

    /* Chase old terrain */
    if (borg_flow_wank(FALSE)) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Chase old monsters */
    if (borg_flow_kill(FALSE)) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Chase old objects */
    if (borg_flow_take(FALSE)) {
        if (borg_play_old_goal()) return (TRUE);
    }


    /*** Wander around ***/

    /* Leave the level (if needed) */
    if (borg_leave_level(FALSE)) return (TRUE);

    /* Explore nearby interesting grids */
    if (borg_flow_dark()) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Explore interesting grids */
    if (borg_flow_explore()) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Hack -- visit all the shops */
    if (borg_flow_shop_visit()) {
        if (borg_play_old_goal()) return (TRUE);
    }
    
    /* Hack -- Visit the shops */
    if (borg_choose_shop()) {

        /* Hack -- re-enter a shop if needed */
        if (grid(c_x,c_y)->o_c == '1' + goal_shop) {

            /* Note */
            borg_note("# Re-entering a shop");

            /* Enter the store */
            borg_keypress('5');

            /* Success */
            return (TRUE);
        }

        /* Try and visit a shop, if so desired */
        if (borg_flow_shop_entry(goal_shop)) {
            if (borg_play_old_goal()) return (TRUE);
        }
    }

    /* Hack -- Learn left-over spells */
    if (borg_play_magic()) return (TRUE);

    /* Leave the level (if possible) */
    if (borg_leave_level(TRUE)) return (TRUE);

    /* Search for secret doors */
    if (borg_flow_spastic(FALSE)) {
        if (!goal) return (TRUE);
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Re-visit old rooms */
    if (borg_flow_revisit()) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Search for secret doors (bored) */
    if (borg_flow_spastic(TRUE)) {
        if (!goal) return (TRUE);
        if (borg_play_old_goal()) return (TRUE);
    }


    /*** Oops ***/

    /* Mega-Hack -- wait for recall to kick in */
    if (goal_recalling) {

        /* Take note */
        borg_note("# Waiting for Recall...");

        /* Rest until done */
        borg_keypress('R');
        borg_keypress('&');
        borg_keypress('\n');

        /* Done */
        return (TRUE);
    }

    /* Twitch around */
    if (borg_twitchy()) return (TRUE);

    /* Oops */
    return (FALSE);
}




/*
 * Sell items to the current shop, if desired
 */
static bool borg_think_shop_sell(void)
{
    /* Sell something if requested */
    if ((goal_shop == shop_num) && (goal_item >= 0)) {

        auto_shop *shop = &auto_shops[goal_shop];
        
        auto_item *item = &auto_items[goal_item];

        /* Log */
        borg_note(format("# Selling %s", item->desc));

        /* Buy an item */
        borg_keypress('s');

        /* Buy the desired item */
        borg_keypress(I2A(goal_item));

        /* Hack -- Sell a single item */
        if (item->iqty > 1) borg_keypress('\n');

        /* Mega-Hack -- Accept the price */
        borg_keypress('\n');
        borg_keypress('\n');
        borg_keypress('\n');
        borg_keypress('\n');

        /* The purchase is complete */
        goal_shop = goal_item = -1;

        /* Success */
        return (TRUE);
    }

    /* Nope */
    return (FALSE);
}


/*
 * Buy items from the current shop, if desired
 */
static bool borg_think_shop_buy(void)
{
    /* Buy something if requested */
    if ((goal_shop == shop_num) && (goal_ware >= 0)) {

        auto_shop *shop = &auto_shops[goal_shop];
        
        auto_item *item = &shop->ware[goal_ware];

        /* Minor Hack -- Go to the correct page */
        if ((goal_ware / 12) != shop->page) borg_keypress(' ');

        /* Log */
        borg_note(format("# Buying %s.", item->desc));

        /* Buy an item */
        borg_keypress('p');

        /* Buy the desired item */
        borg_keypress(I2A(goal_ware % 12));

        /* Hack -- Buy a single item */
        if (item->iqty > 1) borg_keypress('\n');

        /* Mega-Hack -- Accept the price */
        borg_keypress('\n');
        borg_keypress('\n');
        borg_keypress('\n');
        borg_keypress('\n');

        /* The purchase is complete */
        goal_shop = goal_ware = -1;

        /* Success */
        return (TRUE);
    }

    /* Nothing to buy */
    return (FALSE);
}


/*
 * Deal with being in a store
 */
bool borg_think_store(void)
{
    /* Examine the inventory */
    borg_notice();

    /* Hack -- best stuff */
    if (borg_best_stuff()) return (TRUE);

    /* Wear things */
    if (borg_wear_stuff()) return (TRUE);

    /* Take off things */
    if (borg_takeoff_stuff()) return (TRUE);

    /* Hack -- swap rings */
    if (borg_swap_rings()) return (TRUE);

    /* Choose a shop to visit */
    if (borg_choose_shop()) {

        /* Try to sell stuff */
        if (borg_think_shop_sell()) return (TRUE);

        /* Try to buy stuff */
        if (borg_think_shop_buy()) return (TRUE);
    }

    /* Stamp the shop with a time stamp */
    auto_shops[shop_num].when = c_t;

    /* No shop */
    shop_num = -1;

    /* Leave the store */
    borg_keypress(ESCAPE);

    /* Done */
    return (TRUE);
}



/*
 * Initialize this file
 */
void borg_aux_init(void)
{
    /* Make the "safe" inventory array */
    C_MAKE(safe_items, INVEN_TOTAL, auto_item);

    /* Make the "safe" stores in the town */
    C_MAKE(safe_shops, 8, auto_shop);
}



#else

#ifdef MACINTOSH
static int i = 0;
#endif

#endif

