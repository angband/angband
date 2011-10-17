/* File: borg4.c */
/*  Purpose: Notice and Power code for the Borg -BEN- */

#include "angband.h"
#include "object/tvalsval.h"
#include "cave.h"
#include "files.h"

#ifdef ALLOW_BORG
#include "borg1.h"
#include "borg2.h"
#include "borg3.h"
#include "borg4.h"



/*
 * Note that we assume that any item with quantity zero does not exist,
 * thus, when simulating possible worlds, we do not actually have to
 * "optimize" empty slots.
 *
 */


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
    int         i, hold;

    int         extra_blows = 0;

    int         extra_shots = 0;
    int         extra_might = 0;
    int         my_num_fire;
	bitflag f[OF_SIZE];

    borg_item       *item;

    /* Recalc some Variables */
    borg_skill[BI_ARMOR] = 0;
    borg_skill[BI_SPEED] = 110;

    /* Start with a single blow per turn */
    borg_skill[BI_BLOWS] = 1;

    /* Start with a single shot per turn */
    my_num_fire = 1;

    /* Reset the "ammo" tval */
    my_ammo_tval = -1;

    /* Reset the "ammo" sides for darts*/
    my_ammo_sides = 4;

    /* Reset the shooting power */
    my_ammo_power = 0;

	/* Reset the count of ID needed immediately */
	my_need_id = 0;

	/* Base infravision (purely racial) */
    borg_skill[BI_INFRA] = rb_ptr->infra;

    /* Base skill -- disarming */
    borg_skill[BI_DIS] = rb_ptr->r_skills[SKILL_DISARM] + cb_ptr->c_skills[SKILL_DISARM];

    /* Base skill -- magic devices */
    borg_skill[BI_DEV] = rb_ptr->r_skills[SKILL_DEVICE] + cb_ptr->c_skills[SKILL_DEVICE];

    /* Base skill -- saving throw */
    borg_skill[BI_SAV] = rb_ptr->r_skills[SKILL_SAVE] + cb_ptr->c_skills[SKILL_SAVE];

    /* Base skill -- stealth */
    borg_skill[BI_STL] = rb_ptr->r_skills[SKILL_STEALTH] + cb_ptr->c_skills[SKILL_STEALTH];

    /* Base skill -- searching ability */
    borg_skill[BI_SRCH] = rb_ptr->r_skills[SKILL_SEARCH] + cb_ptr->c_skills[SKILL_SEARCH];

    /* Base skill -- searching frequency */
    borg_skill[BI_SRCHFREQ] = rb_ptr->r_skills[SKILL_SEARCH_FREQUENCY] + cb_ptr->c_skills[SKILL_SEARCH_FREQUENCY];

    /* Base skill -- combat (normal) */
    borg_skill[BI_THN] = rb_ptr->r_skills[SKILL_TO_HIT_MELEE] + cb_ptr->c_skills[SKILL_TO_HIT_MELEE];

    /* Base skill -- combat (shooting) */
    borg_skill[BI_THB] = rb_ptr->r_skills[SKILL_TO_HIT_BOW] + cb_ptr->c_skills[SKILL_TO_HIT_BOW];

    /* Base skill -- combat (throwing) */
    borg_skill[BI_THT] = rb_ptr->r_skills[SKILL_TO_HIT_THROW] + cb_ptr->c_skills[SKILL_TO_HIT_THROW];

    /** Racial Skills **/

    /* Extract the player flags */
    player_flags(f);

    /* Good flags */
    if (rf_has(f, OF_SLOW_DIGEST)) borg_skill[BI_SDIG] = TRUE;
    if (rf_has(f, OF_FEATHER)) borg_skill[BI_FEATH] = TRUE;
    if (rf_has(f, OF_REGEN)) borg_skill[BI_REG] = TRUE;
    if (rf_has(f, OF_TELEPATHY)) borg_skill[BI_ESP] = TRUE;
    if (rf_has(f, OF_SEE_INVIS)) borg_skill[BI_SINV] = TRUE;
    if (rf_has(f, OF_FREE_ACT)) borg_skill[BI_FRACT] = TRUE;
    if (rf_has(f, OF_HOLD_LIFE)) borg_skill[BI_HLIFE] = TRUE;

    /* Weird flags */

    /* Bad flags */
    if (rf_has(f, OF_IMPACT)) borg_skill[BI_W_IMPACT] = TRUE;
    if (rf_has(f, OF_AGGRAVATE)) borg_skill[BI_CRSAGRV] = TRUE;
    if (rf_has(f, OF_TELEPORT)) borg_skill[BI_CRSTELE] = TRUE;
	if (rf_has(f, OF_IMPAIR_HP)) borg_skill[BI_CRSHPIMP] = TRUE;
	if (rf_has(f, OF_IMPAIR_MANA)) borg_skill[BI_CRSMPIMP] = TRUE;
	if (rf_has(f, OF_AFRAID))
	{
		borg_skill[BI_CRSFEAR] = TRUE;
	}
	if (rf_has(f, OF_VULN_FIRE)) borg_skill[BI_CRSFVULN] = TRUE;
	if (rf_has(f, OF_VULN_ACID)) borg_skill[BI_CRSAVULN] = TRUE;
	if (rf_has(f, OF_VULN_COLD)) borg_skill[BI_CRSCVULN] = TRUE;
	if (rf_has(f, OF_VULN_ELEC)) borg_skill[BI_CRSEVULN] = TRUE;

    /* Immunity flags */
    if (rf_has(f, OF_IM_FIRE)) borg_skill[BI_IFIRE] = TRUE;
    if (rf_has(f, OF_IM_ACID)) borg_skill[BI_IACID] = TRUE;
    if (rf_has(f, OF_IM_COLD)) borg_skill[BI_ICOLD] = TRUE;
    if (rf_has(f, OF_IM_ELEC)) borg_skill[BI_IELEC] = TRUE;

    /* Resistance flags */
    if (rf_has(f, OF_RES_ACID)) borg_skill[BI_RACID] = TRUE;
    if (rf_has(f, OF_RES_ELEC)) borg_skill[BI_RELEC] = TRUE;
    if (rf_has(f, OF_RES_FIRE)) borg_skill[BI_RFIRE] = TRUE;
    if (rf_has(f, OF_RES_COLD)) borg_skill[BI_RCOLD] = TRUE;
    if (rf_has(f, OF_RES_POIS)) borg_skill[BI_RPOIS] = TRUE;
    if (rf_has(f, OF_RES_FEAR)) borg_skill[BI_RFEAR] = TRUE;
    if (rf_has(f, OF_RES_LIGHT)) borg_skill[BI_RLITE] = TRUE;
    if (rf_has(f, OF_RES_DARK)) borg_skill[BI_RDARK] = TRUE;
    if (rf_has(f, OF_RES_BLIND)) borg_skill[BI_RBLIND] = TRUE;
    if (rf_has(f, OF_RES_CONFU)) borg_skill[BI_RCONF] = TRUE;
    if (rf_has(f, OF_RES_SOUND)) borg_skill[BI_RSND] = TRUE;
    if (rf_has(f, OF_RES_SHARD)) borg_skill[BI_RSHRD] = TRUE;
    if (rf_has(f, OF_RES_NEXUS)) borg_skill[BI_RNXUS] = TRUE;
    if (rf_has(f, OF_RES_NETHR)) borg_skill[BI_RNTHR] = TRUE;
    if (rf_has(f, OF_RES_CHAOS)) borg_skill[BI_RKAOS] = TRUE;
    if (rf_has(f, OF_RES_DISEN)) borg_skill[BI_RDIS] = TRUE;

    /* Sustain flags */
    if (rf_has(f, OF_SUST_STR)) borg_skill[BI_SSTR] = TRUE;
    if (rf_has(f, OF_SUST_INT)) borg_skill[BI_SINT] = TRUE;
    if (rf_has(f, OF_SUST_WIS)) borg_skill[BI_SWIS] = TRUE;
    if (rf_has(f, OF_SUST_DEX)) borg_skill[BI_SDEX] = TRUE;
    if (rf_has(f, OF_SUST_CON)) borg_skill[BI_SCON] = TRUE;
    if (rf_has(f, OF_SUST_CHR)) borg_skill[BI_SCHR] = TRUE;

	/* I am pretty sure the CF_flags will be caught by the
	 * code above when the player flags are checked
	 */

    /* Clear the stat modifiers */
    for (i = 0; i < 6; i++) my_stat_add[i] = 0;

    /* Scan the usable inventory */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
    {
        item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

		/* Does the borg need to get an ID for it? */
		if (streq(item->note, "magical") ||
            streq(item->note, "ego") ||
            streq(item->note, "splendid") ||
			streq(item->note, "excellent")) my_need_id ++;

        /* track number of items the borg has on him */
        /* Count up how many artifacts the borg has on him */
        borg_has_on[item->kind] += item->iqty;
        if (item->name1)
            borg_artifact[item->name1] = item->iqty;

        /* Affect stats */
        if (of_has(item->flags, OF_STR)) my_stat_add[A_STR] += item->pval;
        if (of_has(item->flags, OF_INT)) my_stat_add[A_INT] += item->pval;
        if (of_has(item->flags, OF_WIS)) my_stat_add[A_WIS] += item->pval;
        if (of_has(item->flags, OF_DEX)) my_stat_add[A_DEX] += item->pval;
        if (of_has(item->flags, OF_CON)) my_stat_add[A_CON] += item->pval;
        if (of_has(item->flags, OF_CHR)) my_stat_add[A_CHR] += item->pval;

        /* various slays */
        if (of_has(item->flags, OF_SLAY_ANIMAL)) borg_skill[BI_WS_ANIMAL] = TRUE;
        if (of_has(item->flags, OF_SLAY_EVIL))   borg_skill[BI_WS_EVIL] = TRUE;
        if (of_has(item->flags, OF_SLAY_UNDEAD)) borg_skill[BI_WS_UNDEAD] = TRUE;
        if (of_has(item->flags, OF_SLAY_DEMON))  borg_skill[BI_WS_DEMON] = TRUE;
        if (of_has(item->flags, OF_SLAY_ORC))    borg_skill[BI_WS_ORC] = TRUE;
        if (of_has(item->flags, OF_SLAY_TROLL))  borg_skill[BI_WS_TROLL] = TRUE;
        if (of_has(item->flags, OF_SLAY_GIANT))  borg_skill[BI_WS_GIANT] = TRUE;
        if (of_has(item->flags, OF_SLAY_DRAGON)) borg_skill[BI_WS_DRAGON] = TRUE;
        if (of_has(item->flags, OF_KILL_UNDEAD)) borg_skill[BI_WK_UNDEAD] = TRUE;
        if (of_has(item->flags, OF_KILL_DEMON))  borg_skill[BI_WK_DEMON] = TRUE;
        if (of_has(item->flags, OF_KILL_DRAGON)) borg_skill[BI_WK_DRAGON] = TRUE;
        if (of_has(item->flags, OF_IMPACT))      borg_skill[BI_W_IMPACT] = TRUE;
        if (of_has(item->flags, OF_BRAND_ACID))  borg_skill[BI_WB_ACID] = TRUE;
        if (of_has(item->flags, OF_BRAND_ELEC))  borg_skill[BI_WB_ELEC] = TRUE;
        if (of_has(item->flags, OF_BRAND_FIRE))  borg_skill[BI_WB_FIRE] = TRUE;
        if (of_has(item->flags, OF_BRAND_COLD))  borg_skill[BI_WB_COLD] = TRUE;
        if (of_has(item->flags, OF_BRAND_POIS))  borg_skill[BI_WB_POIS] = TRUE;

        /* Affect infravision */
        if (of_has(item->flags, OF_INFRA)) borg_skill[BI_INFRA] += item->pval;

        /* Affect stealth */
        if (of_has(item->flags, OF_STEALTH)) borg_skill[BI_STL] += item->pval;

        /* Affect searching ability (factor of five) */
        if (of_has(item->flags, OF_SEARCH)) borg_skill[BI_SRCH] += (item->pval * 5);

        /* Affect searching frequency (factor of five) */
        if (of_has(item->flags, OF_SEARCH)) borg_skill[BI_SRCHFREQ] += (item->pval * 5);

        /* Affect digging (factor of 20) */
        if (of_has(item->flags, OF_TUNNEL)) borg_skill[BI_DIG] += (item->pval * 20);

        /* Affect speed */
        if (of_has(item->flags, OF_SPEED)) borg_skill[BI_SPEED] += item->pval;

        /* Affect blows */
        if (of_has(item->flags, OF_BLOWS)) extra_blows += item->pval;

        /* Boost shots */
        if (of_has(item->flags, OF_SHOTS)) extra_shots++;

        /* Boost might */
        if (of_has(item->flags, OF_MIGHT)) extra_might++;

        /* Various flags */
        if (of_has(item->flags, OF_SLOW_DIGEST)) borg_skill[BI_SDIG] = TRUE;
        if (of_has(item->flags, OF_AGGRAVATE)) borg_skill[BI_CRSAGRV] = TRUE;
        if (of_has(item->flags, OF_TELEPORT)) borg_skill[BI_CRSTELE] = TRUE;
		if (of_has(item->flags, OF_IMPAIR_HP)) borg_skill[BI_CRSHPIMP] = TRUE;
		if (of_has(item->flags, OF_IMPAIR_MANA)) borg_skill[BI_CRSMPIMP] = TRUE;
		if (of_has(item->flags, OF_AFRAID))
		{
			borg_skill[BI_CRSFEAR] = TRUE;
		}
		if (of_has(item->flags, OF_VULN_FIRE)) borg_skill[BI_CRSFVULN] = TRUE;
		if (of_has(item->flags, OF_VULN_ACID)) borg_skill[BI_CRSAVULN] = TRUE;
		if (of_has(item->flags, OF_VULN_COLD)) borg_skill[BI_CRSCVULN] = TRUE;
		if (of_has(item->flags, OF_VULN_ELEC)) borg_skill[BI_CRSEVULN] = TRUE;


        if (of_has(item->flags, OF_REGEN)) borg_skill[BI_REG] = TRUE;
        if (of_has(item->flags, OF_TELEPATHY)) borg_skill[BI_ESP] = TRUE;
        if (of_has(item->flags, OF_SEE_INVIS)) borg_skill[BI_SINV] = TRUE;
        if (of_has(item->flags, OF_FEATHER)) borg_skill[BI_FEATH] = TRUE;
        if (of_has(item->flags, OF_FREE_ACT)) borg_skill[BI_FRACT] = TRUE;
        if (of_has(item->flags, OF_HOLD_LIFE)) borg_skill[BI_HLIFE] = TRUE;

		/* Item makes player glow or has a light radius  */
		if (of_has(item->flags, OF_LIGHT))
		{
			/* Special case for Torches/Lantern of Brightness, they are not perm. */
			if (item->tval != TV_LIGHT &&
				item->sval != SV_LIGHT_TORCH &&
				item->sval != SV_LIGHT_LANTERN) borg_skill[BI_LIGHT] ++;
		}

		/* Artifact-- borgs do not gain the knowlege of some flags until
		 * after the *ID*. So we allow the borg to know that the
		 * item does have lite even if its not *ID*
		 */
		if ((item->name1 >=1 && item->name1 <= 3) || /* Phial, Star, Arkenstone */
			  item->name1 == 7 || /* Planatir */
			  item->name1 == 14 || /* Elfstone */
			  item->name1 == 15)  /* Jewel */
			 borg_skill[BI_LIGHT] ++;

        /* Immunity flags */
        /* if you are immune you automaticly resist */
        if (of_has(item->flags, OF_IM_FIRE))
        {
            borg_skill[BI_IFIRE] = TRUE;
            borg_skill[BI_RFIRE] = TRUE;
            borg_skill[BI_TRFIRE] = TRUE;
        }
        if (of_has(item->flags, OF_IM_ACID))
        {
            borg_skill[BI_IACID] = TRUE;
            borg_skill[BI_RACID] = TRUE;
            borg_skill[BI_TRACID] = TRUE;
        }
        if (of_has(item->flags, OF_IM_COLD))
        {
            borg_skill[BI_ICOLD] = TRUE;
            borg_skill[BI_RCOLD] = TRUE;
            borg_skill[BI_TRCOLD] = TRUE;
        }
        if (of_has(item->flags, OF_IM_ELEC))
        {
            borg_skill[BI_IELEC] = TRUE;
            borg_skill[BI_RELEC] = TRUE;
            borg_skill[BI_TRELEC] = TRUE;
        }

        /* Resistance flags */
        if (of_has(item->flags, OF_RES_ACID)) borg_skill[BI_RACID] = TRUE;
        if (of_has(item->flags, OF_RES_ELEC)) borg_skill[BI_RELEC] = TRUE;
        if (of_has(item->flags, OF_RES_FIRE)) borg_skill[BI_RFIRE] = TRUE;
        if (of_has(item->flags, OF_RES_COLD)) borg_skill[BI_RCOLD] = TRUE;
        if (of_has(item->flags, OF_RES_POIS)) borg_skill[BI_RPOIS] = TRUE;
        if (of_has(item->flags, OF_RES_CONFU)) borg_skill[BI_RCONF] = TRUE;
        if (of_has(item->flags, OF_RES_SOUND)) borg_skill[BI_RSND] = TRUE;
        if (of_has(item->flags, OF_RES_LIGHT)) borg_skill[BI_RLITE] = TRUE;
        if (of_has(item->flags, OF_RES_DARK)) borg_skill[BI_RDARK] = TRUE;
        if (of_has(item->flags, OF_RES_CHAOS)) borg_skill[BI_RKAOS] = TRUE;
        if (of_has(item->flags, OF_RES_DISEN)) borg_skill[BI_RDIS] = TRUE;
        if (of_has(item->flags, OF_RES_SHARD)) borg_skill[BI_RSHRD] = TRUE;
        if (of_has(item->flags, OF_RES_NEXUS)) borg_skill[BI_RNXUS] = TRUE;
        if (of_has(item->flags, OF_RES_BLIND)) borg_skill[BI_RBLIND] = TRUE;
        if (of_has(item->flags, OF_RES_NETHR)) borg_skill[BI_RNTHR] = TRUE;

        /* Sustain flags */
        if (of_has(item->flags, OF_SUST_STR)) borg_skill[BI_SSTR] = TRUE;
        if (of_has(item->flags, OF_SUST_INT)) borg_skill[BI_SINT] = TRUE;
        if (of_has(item->flags, OF_SUST_WIS)) borg_skill[BI_SWIS] = TRUE;
        if (of_has(item->flags, OF_SUST_DEX)) borg_skill[BI_SDEX] = TRUE;
        if (of_has(item->flags, OF_SUST_CON)) borg_skill[BI_SCON] = TRUE;
        if (of_has(item->flags, OF_SUST_CHR)) borg_skill[BI_SCHR] = TRUE;


        /* Hack -- Net-zero The borg will miss read acid damaged items such as
         * Leather Gloves [2,-2] and falsely assume they help his power.
         * this hack rewrites the bonus to an extremely negative value
         * thus encouraging him to remove the non-helpful-non-harmful but
         * heavy-none-the-less item.
         */
        if ((!item->name1 && !item->name2) &&
             item->ac >= 1 && item->to_a + item->ac <= 0)
        {
            item->to_a = -20;
        }

        /* Modify the base armor class */
        borg_skill[BI_ARMOR] += item->ac;

        /* Apply the bonuses to armor class */
        borg_skill[BI_ARMOR] += item->to_a;

        /* Hack -- do not apply "weapon" bonuses */
        if (i == INVEN_WIELD) continue;

        /* Hack -- do not apply "bow" bonuses */
        if (i == INVEN_BOW) continue;

        /* Apply the bonuses to hit/damage */
        borg_skill[BI_TOHIT] += item->to_h;
        borg_skill[BI_TODAM] += item->to_d;
    }

    /* Update "stats" */
    for (i = 0; i < 6; i++)
    {
        int add, use, ind;

        add = my_stat_add[i];

        if (op_ptr->opt[OPT_birth_maximize])
        {
            /* Modify the stats for race/class */
            add += (p_ptr->race->r_adj[i] + p_ptr->class->c_adj[i]);
        }
        /* Extract the new "use_stat" value for the stat */
        use = modify_stat_value(my_stat_cur[i], add);

        /* Save the stat */
        my_stat_use[i] = use;

        /* Values: 3, ..., 17 */
        if (use <= 18) ind = (use - 3);

        /* Ranges: 18/00-18/09, ..., 18/210-18/219 */
        else if (use <= 18+219) ind = (15 + (use - 18) / 10);

        /* Range: 18/220+ */
        else ind = (37);

        /* Save the index */
        if (ind > 37)
            my_stat_ind[i] = 37;
        else
            my_stat_ind[i] = ind;
        borg_skill[BI_STR+i] = my_stat_ind[i];
        borg_skill[BI_CSTR+i] = borg_stat[i];
    }


    /* 'Mana' is actually the 'mana adjustment' */
    if (p_ptr->class->spell_book == TV_PRAYER_BOOK)
    {
        borg_skill[BI_SP_ADJ] =
            ((adj_mag_mana[my_stat_ind[A_WIS]] * borg_skill[BI_CLEVEL]) / 2);
        borg_skill[BI_FAIL1] = adj_mag_stat[my_stat_ind[A_WIS]];
        borg_skill[BI_FAIL2] = adj_mag_fail[my_stat_ind[A_WIS]];
    }
    if (p_ptr->class->spell_book == TV_MAGIC_BOOK)
    {
        borg_skill[BI_SP_ADJ] =
            ((adj_mag_mana[my_stat_ind[A_INT]] * borg_skill[BI_CLEVEL]) / 2);
        borg_skill[BI_FAIL1] = adj_mag_stat[my_stat_ind[A_INT]];
        borg_skill[BI_FAIL2] = adj_mag_fail[my_stat_ind[A_INT]];
    }


    /* Bloating slows the player down (a little) */
    if (borg_skill[BI_ISGORGED]) borg_skill[BI_SPEED] -= 10;


    /* Actual Modifier Bonuses (Un-inflate stat bonuses) */
    borg_skill[BI_ARMOR] += ((int)(adj_dex_ta[my_stat_ind[A_DEX]]) - 128);
    borg_skill[BI_TODAM] += ((int)(adj_str_td[my_stat_ind[A_STR]]) - 128);
    borg_skill[BI_TOHIT] += ((int)(adj_dex_th[my_stat_ind[A_DEX]]) - 128);
    borg_skill[BI_TOHIT] += ((int)(adj_str_th[my_stat_ind[A_STR]]) - 128);


    /* Obtain the "hold" value */
    hold = adj_str_hold[my_stat_ind[A_STR]];


    /** Examine the "current bow" **/
    item = &borg_items[INVEN_BOW];

    /* attacking with bare hands */
    if (item->iqty == 0)
    {
        item->ds = 0;
        item->dd = 0;
        item->to_d = 0;
        item->to_h = 0;
        item->weight = 0;
    }

	/* Real bonuses */
    borg_skill[BI_BTOHIT] = item->to_h;
    borg_skill[BI_BTODAM] = item->to_d;

    /* It is hard to carholdry a heavy bow */
    if (hold < item->weight / 10)
    {
        borg_skill[BI_HEAVYBOW] = TRUE;
        /* Hard to wield a heavy bow */
        borg_skill[BI_TOHIT] += 2 * (hold - item->weight / 10);
    }

    /* Compute "extra shots" if needed */
    if (item->iqty && (hold >= item->weight / 10))
    {
        /* Take note of required "tval" for missiles */
        switch (item->sval)
        {
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

		/* Hack -- Reward High Level Rangers using Bows */
        if (player_has(PF_EXTRA_SHOT) && (my_ammo_tval == TV_ARROW))
        {
            /* Extra shot at level 20 */
            if (borg_skill[BI_CLEVEL] >= 20) my_num_fire++;

            /* Extra shot at level 40 */
            if (borg_skill[BI_CLEVEL] >= 40) my_num_fire++;
        }

        /* Add in the "bonus shots" */
        my_num_fire += extra_shots;

        /* Require at least one shot */
        if (my_num_fire < 1) my_num_fire = 1;
    }
    borg_skill[BI_SHOTS] = my_num_fire;

    /* Calculate "average" damage per "normal" shot (times 2) */
    borg_skill[BI_BMAXDAM] = (my_ammo_sides + borg_skill[BI_BTODAM]) * my_ammo_power;
    borg_skill[BI_BMAXDAM] *= borg_skill[BI_SHOTS];

    /* Examine the "main weapon" */
    item = &borg_items[INVEN_WIELD];

    /* attacking with bare hands */
    if (item->iqty == 0)
    {
        item->ds = 0;
        item->dd = 0;
        item->to_d = 0;
        item->to_h = 0;
        item->weight = 0;
    }

	/* Real values */
    borg_skill[BI_WTOHIT] = item->to_h;
    borg_skill[BI_WTODAM] = item->to_d;

    /* It is hard to hold a heavy weapon */
    if (hold < item->weight / 10)
    {
        borg_skill[BI_HEAVYWEPON] = TRUE;

        /* Hard to wield a heavy weapon */
        borg_skill[BI_TOHIT] += 2 * (hold - item->weight / 10);
    }

    /* Normal weapons */
    if (item->iqty && (hold >= item->weight / 10))
    {
        int str_index, dex_index;
        int div;

        /* Enforce a minimum "weight" (tenth pounds) */
        div = ((item->weight < p_ptr->class->min_weight) ? p_ptr->class->min_weight : item->weight);

        /* Get the strength vs weight */
        str_index = (adj_str_blow[my_stat_ind[A_STR]] * p_ptr->class->att_multiply / div);

        /* Maximal value */
        if (str_index > 11) str_index = 11;

        /* Index by dexterity */
        dex_index = (adj_dex_blow[my_stat_ind[A_DEX]]);

        /* Maximal value */
        if (dex_index > 11) dex_index = 11;

        /* Use the blows table */
        borg_skill[BI_BLOWS] = blows_table[str_index][dex_index];

        /* Maximal value */
        if (borg_skill[BI_BLOWS] > p_ptr->class->max_attacks) borg_skill[BI_BLOWS] = p_ptr->class->max_attacks;

        /* Add in the "bonus blows" */
        borg_skill[BI_BLOWS] += extra_blows;

        /* Require at least one blow */
        if (borg_skill[BI_BLOWS] < 1) borg_skill[BI_BLOWS] = 1;

        /* Boost digging skill by weapon weight */
        borg_skill[BI_DIG] += (item->weight / 10);

    }

    /* Calculate "max" damage per "normal" blow  */
    /* and assume we can enchant up to +8 if borg_skill[BI_CLEVEL] > 25 */
    borg_skill[BI_WMAXDAM] =
        (item->dd * item->ds + borg_skill[BI_TODAM] + borg_skill[BI_WTODAM]);
    /* Calculate base damage, used to calculating slays */
    borg_skill[BI_WBASEDAM] =
        (item->dd * item->ds);

     /* Hack -- Reward High Level Warriors with Res Fear */
     if (player_has(PF_BRAVERY_30))
     {
         /* Resist fear at level 30 */
         if (borg_skill[BI_CLEVEL] >= 30) borg_skill[BI_RFEAR] = TRUE;
     }

    /* Affect Skill -- stealth (bonus one) */
    borg_skill[BI_STL] += 1;

    /* Affect Skill -- disarming (DEX and INT) */
    borg_skill[BI_DIS] += adj_dex_dis[my_stat_ind[A_DEX]];
    borg_skill[BI_DIS] += adj_int_dis[my_stat_ind[A_INT]];

    /* Affect Skill -- magic devices (INT) */
    borg_skill[BI_DEV] += adj_int_dev[my_stat_ind[A_INT]];

    /* Affect Skill -- saving throw (WIS) */
    borg_skill[BI_SAV] += adj_wis_sav[my_stat_ind[A_WIS]];

    /* Affect Skill -- digging (STR) */
    borg_skill[BI_DIG] += adj_str_dig[my_stat_ind[A_STR]];


    /* Affect Skill -- disarming (Level, by Class) */
    borg_skill[BI_DIS] += (cb_ptr->x_skills[SKILL_DISARM] * borg_skill[BI_MAXCLEVEL] / 10);

    /* Affect Skill -- magic devices (Level, by Class) */
    borg_skill[BI_DEV] += (cb_ptr->x_skills[SKILL_DEVICE] * borg_skill[BI_MAXCLEVEL] / 10);

    /* Affect Skill -- saving throw (Level, by Class) */
    borg_skill[BI_SAV] += (cb_ptr->x_skills[SKILL_SAVE] * borg_skill[BI_MAXCLEVEL] / 10);

    /* Affect Skill -- stealth (Level, by Class) */
    borg_skill[BI_STL] += (cb_ptr->x_skills[SKILL_STEALTH] * borg_skill[BI_MAXCLEVEL] / 10);

    /* Affect Skill -- search ability (Level, by Class) */
    borg_skill[BI_SRCH] += (cb_ptr->x_skills[SKILL_SEARCH] * borg_skill[BI_MAXCLEVEL] / 10);

    /* Affect Skill -- search frequency (Level, by Class) */
    borg_skill[BI_SRCHFREQ] += (cb_ptr->x_skills[SKILL_SEARCH_FREQUENCY] * borg_skill[BI_MAXCLEVEL] / 10);

    /* Affect Skill -- combat (normal) (Level, by Class) */
    borg_skill[BI_THN] += (cb_ptr->x_skills[SKILL_TO_HIT_MELEE] * borg_skill[BI_MAXCLEVEL] / 10);

    /* Affect Skill -- combat (shooting) (Level, by Class) */
    borg_skill[BI_THB] += (cb_ptr->x_skills[SKILL_TO_HIT_BOW] * borg_skill[BI_MAXCLEVEL] / 10);

    /* Affect Skill -- combat (throwing) (Level, by Class) */
    borg_skill[BI_THT] += (cb_ptr->x_skills[SKILL_TO_HIT_THROW] * borg_skill[BI_MAXCLEVEL] / 10);

    /* Limit Skill -- stealth from 0 to 30 */
    if (borg_skill[BI_STL] > 30) borg_skill[BI_STL] = 30;
    if (borg_skill[BI_STL] < 0) borg_skill[BI_STL] = 0;

    /* Limit Skill -- digging from 1 up */
    if (borg_skill[BI_DIG] < 1) borg_skill[BI_DIG] = 1;


	/*** Some penalties to consider ***/

	/* Fear from spell or effect or flag */
	if (borg_skill[BI_ISAFRAID] || borg_skill[BI_CRSFEAR])
	{
		borg_skill[BI_TOHIT] -= 20;
		borg_skill[BI_ARMOR] += 8;
		borg_skill[BI_DEV] = borg_skill[BI_DEV] * 95 / 100;
	}

    /* priest weapon penalty for non-blessed edged weapons */
    if (player_has(PF_BLESS_WEAPON) &&
        ((item->tval == TV_SWORD || item->tval == TV_POLEARM) &&
        !of_has(item->flags, OF_BLESSED)))
    {
        /* Reduce the real bonuses */
        borg_skill[BI_TOHIT] -= 2;
        borg_skill[BI_TODAM] -= 2;
    }

	/*** Count needed enchantment ***/

    /* Assume no enchantment needed */
    my_need_enchant_to_a = 0;
    my_need_enchant_to_h = 0;
    my_need_enchant_to_d = 0;
    my_need_brand_weapon = 0;

    /* Hack -- enchant all the equipment (weapons) */
    for (i = INVEN_WIELD; i <= INVEN_BOW; i++)
    {
        item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip "unknown" items */
        if (!item->ident) continue;

		/* Most classes store the enchants until they get
		 * a 3x shooter (like a long bow).
		 * --Important: Also look in borg7.c for the enchanting.
		 * --We do not want the bow enchanted by mistake.
		 */
		if (i == INVEN_BOW &&  /* bow */
			my_ammo_power < 3 && /* 3x shooter */
			(!item->name1 && !item->name2)) /* Not Ego or Artifact */
			continue;

        /* Enchant all weapons (to hit) */
        if ((borg_prayer_legal_fail(7, 3, 65) ||
        	 borg_spell_legal_fail(7, 3, 65) ||
             amt_enchant_weapon >=1 ) )
        {
            if (item->to_h < borg_enchant_limit)
            {
                my_need_enchant_to_h += (borg_enchant_limit - item->to_h);
            }

            /* Enchant all weapons (to damage) */
            if (item->to_d < borg_enchant_limit)
            {
                my_need_enchant_to_d += (borg_enchant_limit - item->to_d);
            }
        }
        else /* I dont have the spell or *enchant* */
        {
            if (item->to_h < 8)
            {
                my_need_enchant_to_h += (8 - item->to_h);
            }

            /* Enchant all weapons (to damage) */
            if (item->to_d < 8)
            {
                my_need_enchant_to_d += (8 - item->to_d);
            }
        }
    }

    /* Hack -- enchant all the equipment (armor) */
    for (i = INVEN_BODY; i <= INVEN_FEET; i++)
    {
        item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip "unknown" items */
        if (!item->ident) continue;

        /* Note need for enchantment */
        if ((borg_prayer_legal_fail(7, 4, 65) ||
            borg_spell_legal_fail(7, 2, 65) ||
            amt_enchant_armor >=1 ))
        {
            if (item->to_a < borg_enchant_limit)
            {
                my_need_enchant_to_a += (borg_enchant_limit - item->to_a);
            }
        }
        else
        {
            if (item->to_a < 8)
            {
                my_need_enchant_to_a += (8 - item->to_a);
            }
        }
    }


    /* Examine the lite */
    item = &borg_items[INVEN_LIGHT];

    /* Assume normal lite radius */
    borg_skill[BI_CURLITE] = 0;

    /* Glowing player has light */
    if (borg_skill[BI_LIGHT]) borg_skill[BI_CURLITE] = borg_skill[BI_LIGHT];

    /* Lite */
    if (item->tval == TV_LIGHT)
    {
        /* Torches Bright -- extra radius two */
        if (item->sval == SV_LIGHT_TORCH && item->timeout >= 1501) borg_skill[BI_CURLITE] = borg_skill[BI_CURLITE] + 2;

        /* Torches -- extra radius one */
        if (item->sval == SV_LIGHT_TORCH &&
			item->timeout <= 1501 && item->timeout >= 1) borg_skill[BI_CURLITE] = borg_skill[BI_CURLITE] + 1;

		/* Torch of Brightness -- extra radius one */
        if (item->sval == SV_LIGHT_TORCH &&
			of_has(item->flags, OF_LIGHT) &&
			item->timeout >= 1) borg_skill[BI_CURLITE] = borg_skill[BI_CURLITE] + 1;

        /* Lanterns -- radius two */
        if (item->sval == SV_LIGHT_LANTERN && item->timeout >= 1) borg_skill[BI_CURLITE] = borg_skill[BI_CURLITE] + 2;

        /* Artifact lites -- radius three */
        /* HACK assume non-torch/non lantern lite is artifact */
        if ((item->sval != SV_LIGHT_TORCH) &&
            (item->sval != SV_LIGHT_LANTERN))
        {
			/* We grant +2 here even though the artifact is +3 because the other +1 was granted
			 * higher up in a special handle for non *ID* artifacts in the form of BI_LIGHT
			 */
            borg_skill[BI_CURLITE] = borg_skill[BI_CURLITE] + 2;
        }
    }

	/* Special way to handle See Inv */
	if (borg_see_inv >= 1) borg_skill[BI_SINV] = TRUE;
	if (borg_skill[BI_CDEPTH] == 0 && /* only in town.  Allow him to recall down */
        (borg_prayer_legal(2, 3) ||
         borg_spell_legal(2, 6))) borg_skill[BI_SINV] = TRUE;

	/* Very special handling of Free Action.
	 * If the person has perfect Savings throw, he can be
	 * considered ok on Free Action.  This can free up an
	 * equipment slot.
	 */
    if (borg_skill[BI_SAV] >= 100) borg_skill[BI_FRACT] = TRUE;

	/* Special case for RBlindness.  Perfect saves and the
	 * resistances for light and dark are good enough for RBlind
	 */
	if (borg_skill[BI_SAV] >= 100 && borg_skill[BI_RDARK] &&
	    borg_skill[BI_RLITE]) borg_skill[BI_RBLIND] = TRUE;

	/*** Quiver needs to be evaluated ***/

	/* Hack -- ignore invalid missiles */
    for (i = QUIVER_START; i < QUIVER_END; i++)
    {
        item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) break;

		if (item->tval != my_ammo_tval) continue;

		/* Count missiles */
		borg_skill[BI_AMISSILES] += item->iqty;

		/* Only enchant ammo if we have a good shooter,
		 * otherwise, store the enchants in the home.
		 */
		if (my_ammo_power >= 3)
		{

			if ((borg_equips_artifact(EFF_FIREBRAND, INVEN_BOW) ||
				 borg_spell_legal_fail(7, 5, 65)) &&
			  item->iqty >=5 &&
			  /* Skip artifacts and ego-items */
			  !item->name2 &&
			  !item->name1 &&
			  item->ident &&
			  item->tval == my_ammo_tval)
			  {
				my_need_brand_weapon +=10L;
			  }

			/* if we have loads of cash (as we will at level 35),  */
			/* enchant missiles */
			if (borg_skill[BI_CLEVEL] > 35)
			{
				if ((borg_spell_legal_fail(7, 3, 65) || borg_prayer_legal_fail(7, 3, 65))
					&& item->iqty >= 5)
				{
					if (item->to_h < 10)
					{
						my_need_enchant_to_h += (10 - item->to_h);
					}

					if (item->to_d < 10)
					{
						my_need_enchant_to_d += (10 - item->to_d);
					}
				}
				else
				{
					if (item->to_h < 8)
					{
						my_need_enchant_to_h += (8 - item->to_h);
					}

					if (item->to_d < 8)
					{
						my_need_enchant_to_d += (8 - item->to_d);
					}
				}
			}
		} /* Ammo Power > 3 */
	} /* Quiver */

}


/*
 * Helper function -- notice the player inventory
 */
static void borg_notice_aux2(void)
{
    int i;

    borg_item *item;


    /*** Reset counters ***/


    /* Reset basic */
    amt_food_lowcal = 0;
    amt_food_hical = 0;

    /* Reset healing */
    amt_slow_poison =0;
    amt_cure_confusion = 0;
    amt_cure_blind = 0;

	/* Reset stat potions */
    for (i = 0; i < 6; i++) amt_inc_stat[i] = 0;

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
    amt_fix_stat[6] = 0;

    amt_fix_exp = 0;
    amt_cool_staff = 0;
	amt_cool_wand = 0;
    amt_digger = 0;

    /* Reset enchantment */
    amt_enchant_to_a = 0;
    amt_enchant_to_d = 0;
    amt_enchant_to_h = 0;

    amt_brand_weapon = 0;
    amt_enchant_weapon = 0;
    amt_enchant_armor = 0;

	/* Reset number of Ego items needing *ID* */
	amt_ego = 0;

    /*** Process the inventory ***/

    /* Scan the inventory */
    for (i = 0; i < INVEN_MAX_PACK; i++)
    {
        item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

		/* Does the borg need to get an ID for it? */
		if (strstr(item->note, "magical") || strstr(item->note, "special") ||
			strstr(item->note, "ego") ||streq(item->note, "splendid") ||streq(item->note, "excellent")) my_need_id ++;

        /* Hack -- skip un-aware items */
        if (!item->kind) continue;

		/* count up the items on the borg (do not count artifacts  */
        /* that are not being wielded) */
        borg_has[item->kind] += item->iqty;

        /* Analyze the item */
        switch (item->tval)
        {
            /* Books */
            case TV_MAGIC_BOOK:
            case TV_PRAYER_BOOK:
            /* Skip incorrect books */
            if (item->tval != p_ptr->class->spell_book) break;
            /* Count the books */
            amt_book[item->sval] += item->iqty;
            break;


            /* Food */
            case TV_FOOD:
            /* Analyze */
            switch (item->sval)
            {
                case SV_FOOD_WAYBREAD:
                    amt_food_hical += item->iqty;
                    break;
                case SV_FOOD_RATION:
                    amt_food_hical += item->iqty;
                    break;
                case SV_FOOD_SLIME_MOLD:
                    amt_food_lowcal += item->iqty;
                    break;
                case SV_FOOD_PURGING:
					/* We don't count the shrooms until level 10, they clutter the inventory too much.
					 * The borg will store them in the house.
					 */
					if (borg_munchkin_start && borg_skill[BI_MAXCLEVEL] >= borg_munchkin_level)
					{
						amt_fix_stat[A_STR] += item->iqty;
						amt_fix_stat[A_CON] += item->iqty;
						borg_skill[BI_ACUREPOIS] += item->iqty;
					}
                    break;
                case SV_FOOD_RESTORING:
					if (borg_munchkin_start && borg_skill[BI_MAXCLEVEL] >= borg_munchkin_level)
					{
						amt_fix_stat[A_STR] += item->iqty;
						amt_fix_stat[A_INT] += item->iqty;
						amt_fix_stat[A_WIS] += item->iqty;
						amt_fix_stat[A_DEX] += item->iqty;
						amt_fix_stat[A_CON] += item->iqty;
						amt_fix_stat[A_CHR] += item->iqty;
						amt_fix_stat[6]     += item->iqty;
					}
                    break;

                case SV_FOOD_CURE_MIND:
					if (borg_munchkin_start && borg_skill[BI_MAXCLEVEL] >= borg_munchkin_level)
					{
						amt_cure_confusion += item->iqty;
					}
					break;

                case SV_FOOD_FAST_RECOVERY:
					if (borg_munchkin_start && borg_skill[BI_MAXCLEVEL] >= borg_munchkin_level)
					{
						amt_cure_blind += item->iqty;
						borg_skill[BI_ACUREPOIS] += item->iqty;
					}
					break;

				case SV_FOOD_SECOND_SIGHT:
				case SV_FOOD_EMERGENCY:
				case SV_FOOD_TERROR:
				case SV_FOOD_STONESKIN:
				case SV_FOOD_DEBILITY:
				case SV_FOOD_SPRINTING:
					if (borg_munchkin_start && borg_skill[BI_MAXCLEVEL] >= borg_munchkin_level)
					{
						borg_skill[BI_ASHROOM] +=item->iqty;
					}
					break;
            }
            break;


            /* Potions */
            case TV_POTION:
            /* Analyze */
            switch (item->sval)
            {
                case SV_POTION_HEALING:
                borg_skill[BI_AHEAL] += item->iqty;
                break;
                case SV_POTION_STAR_HEALING:
                borg_skill[BI_AEZHEAL] += item->iqty;
                break;
                case SV_POTION_LIFE:
                borg_skill[BI_ALIFE] += item->iqty;
                break;
                case SV_POTION_CURE_CRITICAL:
                borg_skill[BI_ACCW] += item->iqty;
                break;
                case SV_POTION_CURE_SERIOUS:
                borg_skill[BI_ACSW] += item->iqty;
                break;
                case SV_POTION_CURE_LIGHT:
                borg_skill[BI_ACLW] += item->iqty;
                break;
                case SV_POTION_CURE_POISON:
                borg_skill[BI_ACUREPOIS] += item->iqty;
                break;

                case SV_POTION_RESIST_HEAT:
                borg_skill[BI_ARESHEAT] += item->iqty;
                break;
                case SV_POTION_RESIST_COLD:
                borg_skill[BI_ARESCOLD] += item->iqty;
                break;
                case SV_POTION_RESIST_POIS:
                borg_skill[BI_ARESPOIS] += item->iqty;
                break;

				case SV_POTION_INC_STR:
                amt_inc_stat[A_STR] += item->iqty;
                break;
                case SV_POTION_INC_INT:
                amt_inc_stat[A_INT] += item->iqty;
                break;
                case SV_POTION_INC_WIS:
                amt_inc_stat[A_WIS] += item->iqty;
                break;
                case SV_POTION_INC_DEX:
                amt_inc_stat[A_DEX] += item->iqty;
                break;
                case SV_POTION_INC_CON:
                amt_inc_stat[A_CON] += item->iqty;
                break;
                case SV_POTION_INC_CHR:
                amt_inc_stat[A_CHR] += item->iqty;
                break;
                case SV_POTION_INC_ALL:
                amt_inc_stat[A_STR] += item->iqty;
                amt_inc_stat[A_INT] += item->iqty;
                amt_inc_stat[A_WIS] += item->iqty;
                amt_inc_stat[A_DEX] += item->iqty;
                amt_inc_stat[A_CON] += item->iqty;
                amt_inc_stat[A_CHR] += item->iqty;
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

                case SV_POTION_SPEED:
                borg_skill[BI_ASPEED] += item->iqty;
                break;

  				case SV_POTION_DETONATIONS:
  				borg_skill[BI_ADETONATE] += item->iqty;
  				break;
  			}

            break;



            /* Scrolls */
            case TV_SCROLL:


            /* Analyze the scroll */
            switch (item->sval)
            {
                case SV_SCROLL_IDENTIFY:
                borg_skill[BI_AID] += item->iqty;
                break;

                case SV_SCROLL_RECHARGING:
                borg_skill[BI_ARECHARGE] += item->iqty;
                break;

                case SV_SCROLL_PHASE_DOOR:
                borg_skill[BI_APHASE] += item->iqty;
                break;

                case SV_SCROLL_TELEPORT:
                borg_skill[BI_ATELEPORT] += item->iqty;
                break;

                case SV_SCROLL_WORD_OF_RECALL:
                borg_skill[BI_RECALL] += item->iqty;
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

                case SV_SCROLL_STAR_ENCHANT_WEAPON:
                amt_enchant_weapon += item->iqty;
                break;

                case SV_SCROLL_PROTECTION_FROM_EVIL:
                borg_skill[BI_APFE] += item->iqty;
                break;

                case SV_SCROLL_STAR_ENCHANT_ARMOR:
                amt_enchant_armor += item->iqty;
                break;

                case SV_SCROLL_RUNE_OF_PROTECTION:
                borg_skill[BI_AGLYPH] += item->iqty;
                break;

                case SV_SCROLL_TELEPORT_LEVEL:
                borg_skill[BI_ATELEPORTLVL] += item->iqty;
                borg_skill[BI_ATELEPORT] += 1;
                break;

				case SV_SCROLL_MASS_BANISHMENT:
                borg_skill[BI_AMASSBAN] += item->iqty;
                break;
            }
            break;


            /* Rods */
            case TV_ROD:


            /* Analyze */
            switch (item->sval)
            {
                case SV_ROD_IDENTIFY:
				if (borg_activate_failure(item->tval, item->sval) < 500)
                {
                    borg_skill[BI_AID] += item->iqty * 100;
                }
                else
                {
                    borg_skill[BI_AID] += item->iqty;
                }
                break;

                case SV_ROD_RECALL:
                /* Don't count on it if I suck at activations */
				if (borg_activate_failure(item->tval, item->sval) < 500)
                {
                    borg_skill[BI_RECALL] += item->iqty * 100;
                }
                else
                {
                    borg_skill[BI_RECALL] += item->iqty;
                }
                break;

                case SV_ROD_DETECT_TRAP:
                borg_skill[BI_ADETTRAP] += item->iqty * 100;
                break;

                case SV_ROD_DETECT_DOOR:
                borg_skill[BI_ADETDOOR] += item->iqty * 100;
                break;

                case SV_ROD_DETECTION:
                borg_skill[BI_ADETTRAP] += item->iqty * 100;
                borg_skill[BI_ADETDOOR] += item->iqty * 100;
                borg_skill[BI_ADETEVIL] += item->iqty * 100;
                break;

                case SV_ROD_ILLUMINATION:
                borg_skill[BI_ALITE] += item->iqty * 100;
                break;

                case SV_ROD_SPEED:
                /* Don't count on it if I suck at activations */
				if (borg_activate_failure(item->tval, item->sval) < 500)
                {
                    borg_skill[BI_ASPEED] += item->iqty * 100;
                }
                else
                {
                    borg_skill[BI_ASPEED] += item->iqty;
                }
                break;

                case SV_ROD_MAPPING:
                borg_skill[BI_AMAGICMAP] += item->iqty * 100;
                break;

                case SV_ROD_HEALING:
                /* only +2 per rod because of long charge time. */
                /* Don't count on it if I suck at activations */
				if (borg_activate_failure(item->tval, item->sval) < 500)
                {
                    borg_skill[BI_AHEAL] += item->iqty * 3;
                }
                else
                {
                    borg_skill[BI_AHEAL] += item->iqty + 1;
                }
                break;

				case SV_ROD_LIGHT:
				case SV_ROD_FIRE_BOLT:
				case SV_ROD_ELEC_BOLT:
				case SV_ROD_COLD_BOLT:
				case SV_ROD_ACID_BOLT:
				{
					borg_skill[BI_AROD1] += item->iqty;
					break;
				}
				case SV_ROD_DRAIN_LIFE:
				case SV_ROD_FIRE_BALL:
				case SV_ROD_ELEC_BALL:
				case SV_ROD_COLD_BALL:
				case SV_ROD_ACID_BALL:
				{
					borg_skill[BI_AROD2] += item->iqty;
					break;
				}

            }

            break;

			/* Wands */
			case TV_WAND:

				/* Analyze each */
				if (item->sval == SV_WAND_TELEPORT_AWAY)
				{
					borg_skill[BI_ATPORTOTHER] += item->pval;
				}

				if (item->sval == SV_WAND_STINKING_CLOUD &&
					borg_skill[BI_MAXDEPTH] < 30)
				{
					amt_cool_wand += item->pval;
				}

				if (item->sval == SV_WAND_MAGIC_MISSILE &&
					borg_skill[BI_MAXDEPTH] < 30)
				{
					amt_cool_wand += item->pval;
				}

				if (item->sval == SV_WAND_ANNIHILATION)
				{
					amt_cool_wand += item->pval;
				}

				break;


			/* Staffs */
            case TV_STAFF:
            /* Analyze */
            switch (item->sval)
            {
                case SV_STAFF_IDENTIFY:
				if (borg_skill[BI_MAXDEPTH] <= 95)
				{
					borg_skill[BI_AID] += item->pval * item->iqty;
				}
                break;

                case SV_STAFF_TELEPORTATION:
				if (borg_skill[BI_MAXDEPTH] <= 95)
				{
						borg_skill[BI_AESCAPE] += (item->iqty);
					if (borg_activate_failure(item->tval, item->sval) < 500)
					{
						borg_skill[BI_AESCAPE] += item->pval;
					}
				}
                break;

                case SV_STAFF_SPEED:
                if (borg_skill[BI_MAXDEPTH] <= 95) borg_skill[BI_ASPEED] += item->pval;
                break;

                case SV_STAFF_HEALING:
                borg_skill[BI_AHEAL] += item->pval;
                break;

                case SV_STAFF_THE_MAGI:
                borg_skill[BI_ASTFMAGI] += item->pval;
                break;

                case SV_STAFF_DESTRUCTION:
                borg_skill[BI_ASTFDEST] +=item->pval;
                break;

                case SV_STAFF_POWER:
                amt_cool_staff +=item->iqty;
                break;

                case SV_STAFF_HOLINESS:
                amt_cool_staff +=item->iqty;
                borg_skill[BI_AHEAL] +=item->pval;
                break;
            }

            break;


            /* Flasks */
            case TV_FLASK:

            /* Use as fuel if we equip a lantern */
            if (borg_items[INVEN_LIGHT].sval == SV_LIGHT_LANTERN) borg_skill[BI_AFUEL] += item->iqty;

            /* Count as Missiles */
            if (borg_skill[BI_CLEVEL] < 15 ) borg_skill[BI_AMISSILES] += item->iqty;
            break;


            /* Torches */
            case TV_LIGHT:

            /* Use as fuel if it is a torch and we carry a torch */
            if ((item->sval == SV_LIGHT_TORCH && item->timeout >= 1) &&
                (borg_items[INVEN_LIGHT].sval == SV_LIGHT_TORCH) && borg_items[INVEN_LIGHT].iqty)
            {
                borg_skill[BI_AFUEL] += item->iqty;
            }

            break;


            /* Weapons */
            case TV_HAFTED:
            case TV_POLEARM:
            case TV_SWORD:
                /* These items are checked a bit later in a sub routine
                 * to notice the flags.  It is done outside this switch.
                 */
                 break;

            /* Shovels and such */
            case TV_DIGGING:

                /* Hack -- ignore worthless ones (including cursed) */
                if (item->value <= 0) break;
                if (item->cursed) break;

                /* Do not carry if weak, won't be able to dig anyway */
                if (borg_skill[BI_DIG] < BORG_DIG) break;

				amt_digger += item->iqty;
               break;

            /* Missiles */
            case TV_SHOT:
            case TV_ARROW:
            case TV_BOLT:
            /* Hack -- ignore invalid missiles */
            if (item->tval != my_ammo_tval) break;

            /* Hack -- ignore worthless missiles */
            if (item->value <= 0) break;

            /* Count plain missiles */
            if (!item->name2) borg_skill[BI_AMISSILES] += item->iqty;

			/* Only enchant ammo if we have a good shooter,
			 * otherwise, store the enchants in the home.
			 */
			if (my_ammo_power < 3) break;

            if ((borg_equips_artifact(EFF_FIREBRAND, INVEN_BOW) ||
                 borg_spell_legal_fail(7, 5, 65)) &&
              item->iqty >=5 &&
              /* Skip artifacts and ego-items */
              !item->name1 &&
              !item->name2 &&
              item->ident &&
              item->tval == my_ammo_tval)
              {
                my_need_brand_weapon +=10L;
              }

            /* if we have loads of cash (as we will at level 35),  */
            /* enchant missiles */
            if (borg_skill[BI_CLEVEL] > 35)
            {
                if ((borg_spell_legal_fail(7, 3, 65) || borg_prayer_legal_fail(7, 3, 65))
                    && item->iqty >= 5)
                {
                    if (item->to_h < 10)
                    {
                        my_need_enchant_to_h += (10 - item->to_h);
                    }

                    if (item->to_d < 10)
                    {
                        my_need_enchant_to_d += (10 - item->to_d);
                    }
                }
                else
                {
                    if (item->to_h < 8)
                    {
                        my_need_enchant_to_h += (8 - item->to_h);
                    }

                    if (item->to_d < 8)
                    {
                        my_need_enchant_to_d += (8 - item->to_d);
                    }
                }
            }

            break;
        }
    }


    /*** Process the Spells and Prayers ***/
    /*    artifact activations are accounted here
     *  But some artifacts are not counted for two reasons .
     *  1.  Some spells-powers are needed instantly and are considered in
     *  the borg preparation code.  An artifact maybe non-charged at the
     *  moment he needes it.  Then he would need the spell and not be able
     *  to cast it. (ie. teleport, phase)
     *  2.  An artifact may grant a power then he assumes he has infinite
     *  amounts.  He then sells off his scrolls with the duplicate power.
     *  When it comes time to upgrade and swap out the artifact, he wont
     *  because his power drops since he does not have the scrolls anymore.
     *  and he does not buy items first.
     *
     *  A possible solution would be to have him keep a few scrolls as a
     *  back-up, or to remove the bonus for level preparation from borg_power.
     *  Right now I think it is better that he not consider the artifacts
     *  Whose powers are considered in borg_prep.
     */

    /* Handle "satisfy hunger" -> infinite food */
    if (borg_spell_legal_fail(2, 0, 80) || borg_prayer_legal_fail(1, 5, 80))
    {
        borg_skill[BI_FOOD] += 1000;
    }

    /* Handle "identify" -> infinite identifies */
    if (borg_spell_legal(2, 5) || borg_prayer_legal(5, 2) ||
        borg_equips_artifact(EFF_IDENTIFY, INVEN_WIELD))
    {
        borg_skill[BI_AID] += 1000;
    }

    /* Handle "detect traps" */
    if (borg_prayer_legal(0, 5))
    {
        borg_skill[BI_ADETTRAP] = 1000;
    }

    /* Handle "detect doors" */
    if (borg_prayer_legal(0, 6))
    {
        borg_skill[BI_ADETDOOR] = 1000;
    }

    /* Handle "detect evil & monsters" */
    if (borg_prayer_legal(0, 0) ||
        borg_spell_legal(0, 1))
    {
        borg_skill[BI_ADETEVIL] = 1000;
    }

    /* Handle "detection" */
    if (borg_prayer_legal(5, 1) ||
    borg_equips_artifact(EFF_DETECT_ALL, INVEN_HEAD))
    {
        borg_skill[BI_ADETDOOR] = 1000;
        borg_skill[BI_ADETTRAP] = 1000;
        borg_skill[BI_ADETEVIL] = 1000;

    }

    /* Handle "See Invisible" in a special way. */
    if (borg_prayer_legal(2, 3)/* ||
        borg_spell_legal(2, 6)*/)
    {
		borg_skill[BI_DINV] = TRUE;
    }

    /* Handle "magic mapping" */
    if (borg_prayer_legal(2, 6) ||
    borg_equips_artifact(EFF_MAPPING, INVEN_LIGHT))
    {
        borg_skill[BI_ADETDOOR] = 1000;
        borg_skill[BI_ADETTRAP] = 1000;
        borg_skill[BI_AMAGICMAP] = 1000;
    }

    /* Handle "call lite" */
    if (borg_prayer_legal(0, 4) ||
    	borg_equips_artifact(EFF_ILLUMINATION, INVEN_LIGHT) ||
    	borg_equips_artifact(EFF_CLAIRVOYANCE, INVEN_LIGHT) ||
    	borg_spell_legal(0, 3))
    {
        borg_skill[BI_ALITE] += 1000;
    }

    /* Handle "protection from evil" */
    if (borg_prayer_legal(2, 4) ||
    borg_equips_artifact(EFF_PROTEVIL, INVEN_HEAD))
    {
        borg_skill[BI_APFE] += 1000;
    }

    /* Handle "rune of protection" glyph" */
    if (borg_prayer_legal(3, 4) ||
    	borg_spell_legal(6, 4))
    {
        borg_skill[BI_AGLYPH] += 1000;
    }

    /* Handle "detect traps/doors" */
    if (borg_spell_legal(0, 7))
    {
        borg_skill[BI_ADETDOOR] = 1000;
        borg_skill[BI_ADETTRAP] = 1000;
    }

    /* Handle "enchant weapon" */
    if (borg_spell_legal_fail(7, 3, 65) ||
        borg_prayer_legal_fail(7, 3, 65))
    {
        amt_enchant_to_h += 1000;
        amt_enchant_to_d += 1000;
        amt_enchant_weapon +=1000;
    }

    /* Handle "Brand Weapon (bolts)" */
    if (borg_equips_artifact(EFF_FIREBRAND, INVEN_BOW) ||
        borg_spell_legal_fail(7, 5, 65))
    {
        amt_brand_weapon += 1000;
    }

    /* Handle "enchant armor" */
    if (borg_spell_legal_fail(7, 2, 65) ||
        borg_prayer_legal_fail(7, 4, 65))
    {
        amt_enchant_to_a += 1000;
        amt_enchant_armor +=1000;
    }

    /* Handle Diggers (stone to mud) */
    if (borg_spell_legal_fail(2, 2, 40) ||
    	borg_equips_artifact(EFF_STONE_TO_MUD, INVEN_WIELD) ||
		borg_equips_ring(SV_RING_DELVING))
    {
        amt_digger += 1;
    }

    /* Handle recall */
    if (borg_prayer_legal_fail(4, 4, 40) || borg_spell_legal_fail(6, 3,40) ||
        (borg_skill[BI_CDEPTH] == 100 && (borg_prayer_legal(4, 4) || borg_spell_legal(6, 3))))
    {
        borg_skill[BI_RECALL] += 1000;
    }
	if (borg_equips_artifact(EFF_RECALL, INVEN_WIELD))
	{
		borg_skill[BI_RECALL] += 1;
	}

    /* Handle teleport_level */
    if (borg_prayer_legal_fail(4, 3, 20) || borg_spell_legal_fail(6, 2, 20))
    {
        borg_skill[BI_ATELEPORTLVL] += 1000;
    }

    /* Handle PhaseDoor spell carefully */
    if (borg_prayer_legal_fail(4, 0, 3) ||
        borg_spell_legal_fail(0, 2, 3))
    {
        borg_skill[BI_APHASE] += 1000;
    }
	if (borg_equips_artifact(EFF_TELE_PHASE, INVEN_WIELD))
	{
		borg_skill[BI_APHASE] += 1;
	}

    /* Handle teleport spell carefully */
    if (borg_prayer_legal_fail(1, 1, 1) ||
        borg_prayer_legal_fail(4, 1, 1) ||
        borg_spell_legal_fail(1, 5, 1))
    {
        borg_skill[BI_ATELEPORT] += 1000;
    }
	if (borg_equips_artifact(EFF_TELE_LONG, INVEN_RIGHT))
	{
		borg_skill[BI_AESCAPE] += 1;
        borg_skill[BI_ATELEPORT] += 1;
	}

    /* Handle teleport away */
    if (borg_prayer_legal_fail(4, 2, 40) || borg_spell_legal_fail(3, 1,40))
    {
        borg_skill[BI_ATPORTOTHER] += 1000;
    }

    /* Handle Holy Word prayer just to see if legal */
    if (borg_prayer_legal(3, 5))
    {
        borg_skill[BI_AHWORD] += 1000;
    }

    /* speed spells HASTE*/
    if ( borg_spell_legal( 3, 2 ) ||
         borg_equips_artifact(EFF_HASTE1, INVEN_RIGHT) ||
         borg_equips_artifact(EFF_HASTE2, INVEN_RIGHT))
    {
        borg_skill[BI_ASPEED] += 1000;
    }

    /* Handle "cure light wounds" */
    if (borg_equips_artifact(EFF_CURE_SERIOUS, INVEN_WIELD))
    {
        borg_skill[BI_ACSW] += 1000;
    }


    /* Handle "heal" */
    if (borg_equips_artifact(EFF_HEAL1,INVEN_BODY) ||
        borg_equips_artifact(EFF_HEAL2,INVEN_HEAD) ||
        borg_prayer_legal(3, 2) ||
        borg_prayer_legal(6, 2))
    {
        borg_skill[BI_AHEAL] += 1000;
    }

    /* Handle "fix exp" */
    if (borg_equips_artifact(EFF_RESTORE_LIFE, INVEN_OUTER))
    {
        amt_fix_exp += 1000;
    }

	/* Handle "Remembrance" -- is just as good as Hold Life */
	if (borg_prayer_legal(6, 4) ||
	    borg_equips_artifact(EFF_RESTORE_LIFE, INVEN_WIELD))
	{
		borg_skill[BI_HLIFE] = TRUE;
	}

    /* Handle "recharge" */
    if (borg_equips_artifact(EFF_RECHARGE, INVEN_OUTER) ||
        borg_spell_legal(7,4) ||
        borg_prayer_legal(7,1) ||
        borg_spell_legal(2, 1))
    {
        borg_skill[BI_ARECHARGE] += 1000;
    }

    /*** Process the Needs ***/

    /* No need for fuel if its not a torch or lantern */
    if ((borg_items[INVEN_LIGHT].sval != SV_LIGHT_TORCH) &&
        (borg_items[INVEN_LIGHT].sval != SV_LIGHT_LANTERN)) borg_skill[BI_AFUEL] += 1000;

    /* No need to *buy* stat increase potions */
    if (my_stat_cur[A_STR] >= (18+100) + 10 * op_ptr->opt[OPT_birth_maximize] *
        (p_ptr->race->r_adj[A_STR] + p_ptr->class->c_adj[A_STR]))
        amt_add_stat[A_STR] += 1000;

    if (my_stat_cur[A_INT] >= (18+100) + 10 * op_ptr->opt[OPT_birth_maximize] *
        (p_ptr->race->r_adj[A_INT] + p_ptr->class->c_adj[A_INT]))
         amt_add_stat[A_INT] += 1000;

    if (my_stat_cur[A_WIS] >= (18+100) + 10 * op_ptr->opt[OPT_birth_maximize] *
        (p_ptr->race->r_adj[A_WIS] + p_ptr->class->c_adj[A_WIS]))
        amt_add_stat[A_WIS] += 1000;

    if (my_stat_cur[A_DEX] >= (18+100) + 10 * op_ptr->opt[OPT_birth_maximize] *
        (p_ptr->race->r_adj[A_DEX] + p_ptr->class->c_adj[A_DEX]))
         amt_add_stat[A_DEX] += 1000;

    if (my_stat_cur[A_CON] >= (18+100) + 10 * op_ptr->opt[OPT_birth_maximize] *
        (p_ptr->race->r_adj[A_CON] + p_ptr->class->c_adj[A_CON]))
        amt_add_stat[A_CON] += 1000;

    if (my_stat_cur[A_CHR] >= (18+100) + 10 * op_ptr->opt[OPT_birth_maximize] *
        (p_ptr->race->r_adj[A_CHR] + p_ptr->class->c_adj[A_CHR]))
         amt_add_stat[A_CHR] += 1000;

    /* No need to *buy* stat repair potions */
    if (!borg_skill[BI_ISFIXSTR]) amt_fix_stat[A_STR] += 1000;
    if (!borg_skill[BI_ISFIXINT]) amt_fix_stat[A_INT] += 1000;
    if (!borg_skill[BI_ISFIXWIS]) amt_fix_stat[A_WIS] += 1000;
    if (!borg_skill[BI_ISFIXDEX]) amt_fix_stat[A_DEX] += 1000;
    if (!borg_skill[BI_ISFIXCON]) amt_fix_stat[A_CON] += 1000;
    if (!borg_skill[BI_ISFIXCHR]) amt_fix_stat[A_CHR] += 1000;


    /* No need for experience repair */
    if (!borg_skill[BI_ISFIXEXP]) amt_fix_exp += 1000;

    /* Correct the high and low calorie foods */
    borg_skill[BI_FOOD] += amt_food_hical;
    if (amt_food_hical <= 3) borg_skill[BI_FOOD] += amt_food_lowcal;

    /* If weak, do not count food spells */
   if (borg_skill[BI_ISWEAK] && (borg_skill[BI_FOOD] >= 1000))
        borg_skill[BI_FOOD] -= 1000;
}


/*
 * Helper function -- notice the player swap weapon
 */
void borg_notice_weapon_swap(void)
{
    int i;
    int b_i = 0;

    s32b v =-1L;
    s32b b_v = 0L;

    int dam, damage;
    borg_item *item;

    weapon_swap =0;

    /*** Process the inventory ***/
    for (i = 0; i < INVEN_MAX_PACK; i++)
    {
        item = &borg_items[i];

        /* reset counter */
        v= -1L;
        dam =0;
        damage =0;

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Hack -- skip un-aware items */
        if (!item->kind) continue;

		/* Skip non-wearable items */
		if (borg_slot(item->tval, item->sval) == -1) continue;

		/* Dont carry swaps until dlevel 50.  They are heavy.
           Unless the item is a digger, then carry it */
        if (borg_skill[BI_MAXDEPTH] < 50 && item->tval !=TV_DIGGING) continue;

        /* priest weapon penalty for non-blessed edged weapons */
        if (player_has(PF_BLESS_WEAPON) &&
            (item->tval == TV_SWORD || item->tval == TV_POLEARM) &&
            !of_has(item->flags, OF_BLESSED)) continue;

        /* Require ID, "known" (or average, good, etc) */
        if (!item->ident &&
            !streq(item->note, "magical") &&
            !streq(item->note, "excellent") &&
            !streq(item->note, "ego") &&
            !streq(item->note, "splendid") &&
            !streq(item->note, "terrible") &&
            !streq(item->note, "special")) continue;

       /* Clear all the swap weapon flags as I look at each one. */
        weapon_swap_digger = 0;
        weapon_swap_slay_animal = FALSE;
        weapon_swap_slay_evil = FALSE;
        weapon_swap_slay_undead = FALSE;
        weapon_swap_slay_demon = FALSE;
        weapon_swap_slay_orc = FALSE;
        weapon_swap_slay_troll = FALSE;
        weapon_swap_slay_giant = FALSE;
        weapon_swap_slay_dragon = FALSE;
        weapon_swap_kill_undead = FALSE;
        weapon_swap_kill_demon = FALSE;
        weapon_swap_kill_dragon = FALSE;
        weapon_swap_impact = FALSE;
        weapon_swap_brand_acid = FALSE;
        weapon_swap_brand_elec = FALSE;
        weapon_swap_brand_fire = FALSE;
        weapon_swap_brand_cold = FALSE;
        weapon_swap_brand_pois = FALSE;
        weapon_swap_see_infra = FALSE;
        weapon_swap_slow_digest = FALSE;
        weapon_swap_aggravate = FALSE;
        weapon_swap_teleport = FALSE;
        weapon_swap_regenerate = FALSE;
        weapon_swap_telepathy = FALSE;
        weapon_swap_LIGHT = FALSE;
        weapon_swap_see_invis = FALSE;
        weapon_swap_ffall = FALSE;
        weapon_swap_free_act = FALSE;
        weapon_swap_hold_life = FALSE;
        weapon_swap_immune_fire = FALSE;
        weapon_swap_immune_acid = FALSE;
        weapon_swap_immune_cold = FALSE;
        weapon_swap_immune_elec = FALSE;
        weapon_swap_resist_acid = FALSE;
        weapon_swap_resist_elec = FALSE;
        weapon_swap_resist_fire = FALSE;
        weapon_swap_resist_cold = FALSE;
        weapon_swap_resist_pois = FALSE;
        weapon_swap_resist_conf = FALSE;
        weapon_swap_resist_sound = FALSE;
        weapon_swap_resist_LIGHT = FALSE;
        weapon_swap_resist_dark = FALSE;
        weapon_swap_resist_chaos = FALSE;
        weapon_swap_resist_disen = FALSE;
        weapon_swap_resist_shard = FALSE;
        weapon_swap_resist_nexus = FALSE;
        weapon_swap_resist_blind = FALSE;
        weapon_swap_resist_neth = FALSE;
        decurse_weapon_swap =-1;

        /* Analyze the item */
        switch (item->tval)
        {

            /* weapons */
            case TV_HAFTED:
            case TV_POLEARM:
            case TV_SWORD:
            case TV_DIGGING:
            {

            /* Digging */
            if (of_has(item->flags, OF_TUNNEL))
            {
                /* Don't notice digger if we can turn stone to mud,
                 * or I am using one.
                 */
                /* Hack -- ignore worthless ones (including cursed) */
                if (item->value <= 0) break;
                if (item->cursed) break;
                if (!borg_spell_legal_fail(2, 2, 40) &&
                    !of_has(borg_items[INVEN_WIELD].flags, OF_TUNNEL))
                weapon_swap_digger = item->pval;
            }

            /* various slays */
            if (of_has(item->flags, OF_SLAY_ANIMAL)) weapon_swap_slay_animal = TRUE;
            if (of_has(item->flags, OF_SLAY_EVIL))   weapon_swap_slay_evil = TRUE;
            if (of_has(item->flags, OF_SLAY_UNDEAD)) weapon_swap_slay_undead = TRUE;
            if (of_has(item->flags, OF_SLAY_DEMON))  weapon_swap_slay_demon = TRUE;
            if (of_has(item->flags, OF_SLAY_ORC))    weapon_swap_slay_orc = TRUE;
            if (of_has(item->flags, OF_SLAY_TROLL))  weapon_swap_slay_troll = TRUE;
            if (of_has(item->flags, OF_SLAY_GIANT))  weapon_swap_slay_giant = TRUE;
            if (of_has(item->flags, OF_SLAY_DRAGON)) weapon_swap_slay_dragon = TRUE;
            if (of_has(item->flags, OF_KILL_UNDEAD)) weapon_swap_slay_undead = TRUE;
            if (of_has(item->flags, OF_KILL_DEMON))  weapon_swap_slay_demon = TRUE;
            if (of_has(item->flags, OF_KILL_DRAGON)) weapon_swap_kill_dragon = TRUE;
            if (of_has(item->flags, OF_IMPACT))      weapon_swap_impact = TRUE;
            if (of_has(item->flags, OF_BRAND_ACID))  weapon_swap_brand_acid = TRUE;
            if (of_has(item->flags, OF_BRAND_ELEC))  weapon_swap_brand_elec = TRUE;
            if (of_has(item->flags, OF_BRAND_FIRE))  weapon_swap_brand_fire = TRUE;
            if (of_has(item->flags, OF_BRAND_COLD))  weapon_swap_brand_cold = TRUE;
            if (of_has(item->flags, OF_BRAND_POIS))  weapon_swap_brand_pois = TRUE;

            /* Affect infravision */
            if (of_has(item->flags, OF_INFRA)) weapon_swap_see_infra += item->pval;

            /* Affect speed */

            /* Various flags */
            if (of_has(item->flags, OF_SLOW_DIGEST)) weapon_swap_slow_digest = TRUE;
            if (of_has(item->flags, OF_AGGRAVATE)) weapon_swap_aggravate = TRUE;
            if (of_has(item->flags, OF_TELEPORT)) weapon_swap_teleport = TRUE;
            if (of_has(item->flags, OF_REGEN)) weapon_swap_regenerate = TRUE;
            if (of_has(item->flags, OF_TELEPATHY)) weapon_swap_telepathy = TRUE;
            if (of_has(item->flags, OF_LIGHT)) weapon_swap_LIGHT = TRUE;
            if (of_has(item->flags, OF_SEE_INVIS)) weapon_swap_see_invis = TRUE;
            if (of_has(item->flags, OF_FEATHER)) weapon_swap_ffall = TRUE;
            if (of_has(item->flags, OF_FREE_ACT)) weapon_swap_free_act = TRUE;
            if (of_has(item->flags, OF_HOLD_LIFE)) weapon_swap_hold_life = TRUE;

            /* Immunity flags */
            /* if you are immune you automaticly resist */
            if (of_has(item->flags, OF_IM_FIRE))
            {
                weapon_swap_immune_fire = TRUE;
                weapon_swap_resist_fire = TRUE;
            }
            if (of_has(item->flags, OF_IM_ACID))
            {
                weapon_swap_immune_acid = TRUE;
                weapon_swap_resist_acid = TRUE;
            }
            if (of_has(item->flags, OF_IM_COLD))
            {
                weapon_swap_immune_cold = TRUE;
                weapon_swap_resist_cold = TRUE;
            }
            if (of_has(item->flags, OF_IM_ELEC))
            {
                weapon_swap_immune_elec = TRUE;
                weapon_swap_resist_elec = TRUE;
            }

            /* Resistance flags */
            if (of_has(item->flags, OF_RES_ACID)) weapon_swap_resist_acid = TRUE;
            if (of_has(item->flags, OF_RES_ELEC)) weapon_swap_resist_elec = TRUE;
            if (of_has(item->flags, OF_RES_FIRE)) weapon_swap_resist_fire = TRUE;
            if (of_has(item->flags, OF_RES_COLD)) weapon_swap_resist_cold = TRUE;
            if (of_has(item->flags, OF_RES_POIS)) weapon_swap_resist_pois = TRUE;
            if (of_has(item->flags, OF_RES_CONFU)) weapon_swap_resist_conf = TRUE;
            if (of_has(item->flags, OF_RES_SOUND)) weapon_swap_resist_sound = TRUE;
            if (of_has(item->flags, OF_RES_LIGHT)) weapon_swap_resist_LIGHT = TRUE;
            if (of_has(item->flags, OF_RES_DARK)) weapon_swap_resist_dark = TRUE;
            if (of_has(item->flags, OF_RES_CHAOS)) weapon_swap_resist_chaos = TRUE;
            if (of_has(item->flags, OF_RES_DISEN)) weapon_swap_resist_disen = TRUE;
            if (of_has(item->flags, OF_RES_SHARD)) weapon_swap_resist_shard = TRUE;
            if (of_has(item->flags, OF_RES_NEXUS)) weapon_swap_resist_nexus = TRUE;
            if (of_has(item->flags, OF_RES_BLIND)) weapon_swap_resist_blind = TRUE;
            if (of_has(item->flags, OF_RES_NETHR)) weapon_swap_resist_neth = TRUE;
            if (item->cursed) decurse_weapon_swap = 0;
            if (of_has(item->flags, OF_HEAVY_CURSE)) decurse_weapon_swap = 1;

			/* Sustain flags */

            /* calculating the value of the swap weapon. */
            damage = (item->dd * (item->ds) *25L);

            /* Reward "damage" and increased blows per round*/
            v += damage * (borg_skill[BI_BLOWS]+1);

            /* Reward "bonus to hit" */
            v += ((borg_skill[BI_TOHIT] + item->to_h)*100L);

            /* Reward "bonus to dam" */
            v += ((borg_skill[BI_TODAM] + item->to_d)*75L);

            dam = damage * borg_skill[BI_BLOWS];

            /* assume 2x base damage for x% of creatures */
            dam = damage * 2 * borg_skill[BI_BLOWS];
            /* rewared SAnimal if no electric brand */
            if (!borg_skill[BI_WS_ANIMAL] && !borg_skill[BI_WB_ELEC] && weapon_swap_slay_animal) v += (dam*2) /2;
            if (!borg_skill[BI_WS_EVIL] && weapon_swap_slay_evil) v +=  (dam*7) /2;

            /* assume 3x base damage for x% of creatures */
            dam = damage *3*borg_skill[BI_BLOWS];

            /* half of the reward now for SOrc and STroll*/
            if (!borg_skill[BI_WS_ORC] && weapon_swap_slay_orc) v += (dam*1) /2;
            if (!borg_skill[BI_WS_TROLL] && weapon_swap_slay_troll) v += (dam*2) /2;

            if (!borg_skill[BI_WS_UNDEAD] && weapon_swap_slay_undead) v += (dam*5) /2;
            if (!borg_skill[BI_WS_DEMON] && weapon_swap_slay_demon) v += (dam*3) /2;
            if (!borg_skill[BI_WS_GIANT] && weapon_swap_slay_giant) v += (dam*4) /2;
            if (!borg_skill[BI_WS_DRAGON] && !borg_skill[BI_WK_DRAGON] && weapon_swap_slay_dragon) v += (dam*6) /2;
            if (!borg_skill[BI_WB_ACID] && weapon_swap_brand_acid) v += (dam*4) /2;
            if (!borg_skill[BI_WB_ELEC] && weapon_swap_brand_elec) v += (dam*5) /2;
            if (!borg_skill[BI_WB_FIRE] && weapon_swap_brand_fire) v += (dam*3) /2;
            if (!borg_skill[BI_WB_COLD] && weapon_swap_brand_cold) v += (dam*3) /2;
            if (!borg_skill[BI_WB_POIS] && weapon_swap_brand_pois) v += (dam*3) /2;
            /* Orcs and Trolls get the second half of the reward if SEvil is not possesed. */
            if (!borg_skill[BI_WS_ORC] && !borg_skill[BI_WS_EVIL] && weapon_swap_slay_orc) v += (dam*1) /2;
            if (!borg_skill[BI_WS_TROLL] && !borg_skill[BI_WS_EVIL] && weapon_swap_slay_troll) v += (dam*1) /2;

            /* assume 5x base damage for x% of creatures */
            dam = damage  * 5 * borg_skill[BI_BLOWS];
            if (!borg_skill[BI_WK_UNDEAD] && weapon_swap_kill_undead) v += (dam*5) /2;
            if (!borg_skill[BI_WK_DEMON] && weapon_swap_kill_demon) v += (dam*5) /2;
            if (!borg_skill[BI_WK_DRAGON] && weapon_swap_kill_dragon) v += (dam*5) /2;

            /* reward the Tunnel factor when low level */
            if (borg_skill[BI_MAXDEPTH] <= 40 && borg_skill[BI_MAXDEPTH] >= 25 && borg_gold < 100000 && weapon_swap_digger) v += (weapon_swap_digger * 3500L) + 1000L;

            /* Other Skills */
            if (!borg_skill[BI_SDIG] && weapon_swap_slow_digest) v += 10L;
            if (weapon_swap_aggravate) v -= 8000L;
            if (weapon_swap_teleport) v -= 100000L;
            if (decurse_weapon_swap != -1) v -= 5000L;
            if (!borg_skill[BI_REG] && weapon_swap_regenerate) v += 2000L;
            if (!borg_skill[BI_ESP] && weapon_swap_telepathy) v += 5000L;
            if (!borg_skill[BI_LIGHT] && weapon_swap_LIGHT) v += 2000L;
            if (!borg_skill[BI_SINV] && weapon_swap_see_invis) v += 50000L;
            if (!borg_skill[BI_FEATH] && weapon_swap_ffall) v += 10L;
            if (!borg_skill[BI_FRACT] && weapon_swap_free_act) v += 10000L;
            if (!borg_skill[BI_HLIFE] && (borg_skill[BI_MAXCLEVEL] < 50) && weapon_swap_hold_life) v += 2000L;
            if (!borg_skill[BI_IFIRE] && weapon_swap_immune_fire) v += 70000L;
            if (!borg_skill[BI_IACID] && weapon_swap_immune_acid) v += 30000L;
            if (!borg_skill[BI_ICOLD] && weapon_swap_immune_cold) v += 50000L;
            if (!borg_skill[BI_IELEC] && weapon_swap_immune_elec) v += 25000L;
            if (!borg_skill[BI_RFIRE] && weapon_swap_resist_fire) v += 8000L;
            if (!borg_skill[BI_RACID] && weapon_swap_resist_acid) v += 6000L;
            if (!borg_skill[BI_RCOLD] && weapon_swap_resist_cold) v += 4000L;
            if (!borg_skill[BI_RELEC] && weapon_swap_resist_elec) v += 3000L;
            /* extra bonus for getting all basic resist */
            if (weapon_swap_resist_fire &&
                weapon_swap_resist_acid &&
                weapon_swap_resist_elec &&
                weapon_swap_resist_cold) v +=  10000L;
            if (!borg_skill[BI_RPOIS] && weapon_swap_resist_pois) v += 20000L;
            if (!borg_skill[BI_RCONF] && weapon_swap_resist_conf) v += 5000L;
            if (!borg_skill[BI_RSND] && weapon_swap_resist_sound) v += 2000L;
            if (!borg_skill[BI_RLITE] && weapon_swap_resist_LIGHT) v += 800L;
            if (!borg_skill[BI_RDARK] && weapon_swap_resist_dark) v += 800L;
            if (!borg_skill[BI_RKAOS] && weapon_swap_resist_chaos) v += 8000L;
            if (!borg_skill[BI_RDIS] && weapon_swap_resist_disen) v += 5000L;
            if (!borg_skill[BI_RSHRD] && weapon_swap_resist_shard) v += 100L;
            if (!borg_skill[BI_RNXUS] && weapon_swap_resist_nexus) v += 100L;
            if (!borg_skill[BI_RBLIND] && weapon_swap_resist_blind) v += 5000L;
            if (!borg_skill[BI_RNTHR] && weapon_swap_resist_neth) v += 5500L;
            if (!borg_skill[BI_RFEAR] && weapon_swap_resist_fear) v += 5500L;

            /* Special concern if Tarrasque is alive */
            if (borg_skill[BI_MAXDEPTH] >= 75 &&
               ((!borg_skill[BI_ICOLD] && weapon_swap_immune_cold) ||
                (!borg_skill[BI_IFIRE] && weapon_swap_immune_fire)))
            {
               /* If Tarraseque is alive */
               if (borg_race_death[539] == 0)
               {
                   if (!borg_skill[BI_ICOLD] && weapon_swap_immune_cold) v  += 90000L;
                   if (!borg_skill[BI_IFIRE] && weapon_swap_immune_fire) v  += 90000L;
               }

            }


            /*  Mega-Hack -- resists (level 60) */
            /* its possible that he will get a sword and a cloak
             * both with the same high resist and keep each based
             * on that resist.  We want him to check to see
             * that the other swap does not already have the high resist.
             */
            if (!borg_skill[BI_RNTHR]  && (borg_skill[BI_MAXDEPTH]+1 >= 55) &&
                weapon_swap_resist_neth) v += 100000L;
            if (!borg_skill[BI_RKAOS] && (borg_skill[BI_MAXDEPTH]+1 >= 60) &&
                weapon_swap_resist_chaos) v += 100000L;
            if (!borg_skill[BI_RDIS] && (borg_skill[BI_MAXDEPTH]+1 >= 60) &&
                weapon_swap_resist_disen) v += 100000L;

            /* some artifacts would make good back ups for their activation */


            /* skip usless ones */
            if (v <= 1000) continue;

            /* collect the best one */
            if (v < b_v) continue;

            /* track it */
            b_i = i;
            b_v = v;
        }


        }
    }
    /* mark the swap item and its value */
    weapon_swap_value = b_v;
    weapon_swap = b_i;

    /* Now that we know who the best swap is lets set our swap
     * flags and get a move on
     */
    /*** Process the best inven item ***/

    item = &borg_items[b_i];

   /* Clear all the swap weapon flags as I look at each one. */
    weapon_swap_slay_animal = FALSE;
    weapon_swap_slay_evil = FALSE;
    weapon_swap_slay_undead = FALSE;
    weapon_swap_slay_demon = FALSE;
    weapon_swap_slay_orc = FALSE;
    weapon_swap_slay_troll = FALSE;
    weapon_swap_slay_giant = FALSE;
    weapon_swap_slay_dragon = FALSE;
    weapon_swap_kill_undead = FALSE;
    weapon_swap_kill_demon = FALSE;
    weapon_swap_kill_dragon = FALSE;
    weapon_swap_impact = FALSE;
    weapon_swap_brand_acid = FALSE;
    weapon_swap_brand_elec = FALSE;
    weapon_swap_brand_fire = FALSE;
    weapon_swap_brand_cold = FALSE;
    weapon_swap_brand_pois = FALSE;
    weapon_swap_see_infra = FALSE;
    weapon_swap_slow_digest = FALSE;
    weapon_swap_aggravate = FALSE;
    weapon_swap_teleport = FALSE;
    weapon_swap_regenerate = FALSE;
    weapon_swap_telepathy = FALSE;
    weapon_swap_LIGHT = FALSE;
    weapon_swap_see_invis = FALSE;
    weapon_swap_ffall = FALSE;
    weapon_swap_free_act = FALSE;
    weapon_swap_hold_life = FALSE;
    weapon_swap_immune_fire = FALSE;
    weapon_swap_immune_acid = FALSE;
    weapon_swap_immune_cold = FALSE;
    weapon_swap_immune_elec = FALSE;
    weapon_swap_resist_acid = FALSE;
    weapon_swap_resist_elec = FALSE;
    weapon_swap_resist_fire = FALSE;
    weapon_swap_resist_cold = FALSE;
    weapon_swap_resist_pois = FALSE;
    weapon_swap_resist_conf = FALSE;
    weapon_swap_resist_sound = FALSE;
    weapon_swap_resist_LIGHT = FALSE;
    weapon_swap_resist_dark = FALSE;
    weapon_swap_resist_chaos = FALSE;
    weapon_swap_resist_disen = FALSE;
    weapon_swap_resist_shard = FALSE;
    weapon_swap_resist_nexus = FALSE;
    weapon_swap_resist_blind = FALSE;
    weapon_swap_resist_neth = FALSE;
    decurse_weapon_swap = -1;

    /* Assume no enchantment needed */
    enchant_weapon_swap_to_h = 0;
    enchant_weapon_swap_to_d = 0;

    /* Enchant swap weapons (to hit) */
    if ((borg_prayer_legal_fail(7, 3, 65) ||
         borg_spell_legal_fail(7, 3, 65) ||
         amt_enchant_weapon >=1 ) && item->tval != TV_DIGGING)
    {
        if (item->to_h < 10)
        {
            enchant_weapon_swap_to_h += (10 - item->to_h);
        }

        /* Enchant my swap (to damage) */
        if (item->to_d < 10)
        {
            enchant_weapon_swap_to_d += (10 - item->to_d);
        }
    }
    else if (item->tval != TV_DIGGING)
    {
        if (item->to_h < 8)
        {
            enchant_weapon_swap_to_h += (8 - item->to_h);
        }

        /* Enchant my swap (to damage) */
        if (item->to_d < 8)
        {
            enchant_weapon_swap_to_d += (8 - item->to_d);
        }
    }

    /* various slays */
    if (of_has(item->flags, OF_SLAY_ANIMAL)) weapon_swap_slay_animal = TRUE;
    if (of_has(item->flags, OF_SLAY_EVIL))   weapon_swap_slay_evil = TRUE;
    if (of_has(item->flags, OF_SLAY_UNDEAD)) weapon_swap_slay_undead = TRUE;
    if (of_has(item->flags, OF_SLAY_DEMON))  weapon_swap_slay_demon = TRUE;
    if (of_has(item->flags, OF_SLAY_ORC))    weapon_swap_slay_orc = TRUE;
    if (of_has(item->flags, OF_SLAY_TROLL))  weapon_swap_slay_troll = TRUE;
    if (of_has(item->flags, OF_SLAY_GIANT))  weapon_swap_slay_giant = TRUE;
    if (of_has(item->flags, OF_SLAY_DRAGON)) weapon_swap_slay_dragon = TRUE;
    if (of_has(item->flags, OF_KILL_UNDEAD)) weapon_swap_kill_undead = TRUE;
    if (of_has(item->flags, OF_KILL_DEMON))  weapon_swap_kill_demon = TRUE;
    if (of_has(item->flags, OF_KILL_DRAGON)) weapon_swap_kill_dragon = TRUE;
    if (of_has(item->flags, OF_IMPACT))      weapon_swap_impact = TRUE;
    if (of_has(item->flags, OF_BRAND_ACID))  weapon_swap_brand_acid = TRUE;
    if (of_has(item->flags, OF_BRAND_ELEC))  weapon_swap_brand_elec = TRUE;
    if (of_has(item->flags, OF_BRAND_FIRE))  weapon_swap_brand_fire = TRUE;
    if (of_has(item->flags, OF_BRAND_COLD))  weapon_swap_brand_cold = TRUE;
    if (of_has(item->flags, OF_BRAND_POIS))  weapon_swap_brand_pois = TRUE;

    /* Affect infravision */
    if (of_has(item->flags, OF_INFRA)) weapon_swap_see_infra += item->pval;
    /* Affect various skills */
    /* Affect speed */

    /* Various flags */
    if (of_has(item->flags, OF_SLOW_DIGEST)) weapon_swap_slow_digest = TRUE;
    if (of_has(item->flags, OF_AGGRAVATE)) weapon_swap_aggravate = TRUE;
    if (of_has(item->flags, OF_TELEPORT)) weapon_swap_teleport = TRUE;
    if (of_has(item->flags, OF_REGEN)) weapon_swap_regenerate = TRUE;
    if (of_has(item->flags, OF_TELEPATHY)) weapon_swap_telepathy = TRUE;
    if (of_has(item->flags, OF_LIGHT)) weapon_swap_LIGHT = TRUE;
    if (of_has(item->flags, OF_SEE_INVIS)) weapon_swap_see_invis = TRUE;
    if (of_has(item->flags, OF_FEATHER)) weapon_swap_ffall = TRUE;
    if (of_has(item->flags, OF_FREE_ACT)) weapon_swap_free_act = TRUE;
    if (of_has(item->flags, OF_HOLD_LIFE)) weapon_swap_hold_life = TRUE;

    /* Immunity flags */
    /* if you are immune you automaticly resist */
    if (of_has(item->flags, OF_IM_FIRE))
    {
        weapon_swap_immune_fire = TRUE;
        weapon_swap_resist_fire = TRUE;
    }
    if (of_has(item->flags, OF_IM_ACID))
    {
        weapon_swap_immune_acid = TRUE;
        weapon_swap_resist_acid = TRUE;
    }
    if (of_has(item->flags, OF_IM_COLD))
    {
        weapon_swap_immune_cold = TRUE;
        weapon_swap_resist_cold = TRUE;
    }
    if (of_has(item->flags, OF_IM_ELEC))
    {
        weapon_swap_immune_elec = TRUE;
        weapon_swap_resist_elec = TRUE;
    }

    /* Resistance flags */
    if (of_has(item->flags, OF_RES_ACID)) weapon_swap_resist_acid = TRUE;
    if (of_has(item->flags, OF_RES_ELEC)) weapon_swap_resist_elec = TRUE;
    if (of_has(item->flags, OF_RES_FIRE)) weapon_swap_resist_fire = TRUE;
    if (of_has(item->flags, OF_RES_COLD)) weapon_swap_resist_cold = TRUE;
    if (of_has(item->flags, OF_RES_POIS)) weapon_swap_resist_pois = TRUE;
    if (of_has(item->flags, OF_RES_CONFU)) weapon_swap_resist_conf = TRUE;
    if (of_has(item->flags, OF_RES_SOUND)) weapon_swap_resist_sound = TRUE;
    if (of_has(item->flags, OF_RES_LIGHT)) weapon_swap_resist_LIGHT = TRUE;
    if (of_has(item->flags, OF_RES_DARK)) weapon_swap_resist_dark = TRUE;
    if (of_has(item->flags, OF_RES_CHAOS)) weapon_swap_resist_chaos = TRUE;
    if (of_has(item->flags, OF_RES_DISEN)) weapon_swap_resist_disen = TRUE;
    if (of_has(item->flags, OF_RES_SHARD)) weapon_swap_resist_shard = TRUE;
    if (of_has(item->flags, OF_RES_NEXUS)) weapon_swap_resist_nexus = TRUE;
    if (of_has(item->flags, OF_RES_BLIND)) weapon_swap_resist_blind = TRUE;
    if (of_has(item->flags, OF_RES_NETHR)) weapon_swap_resist_neth = TRUE;
    if (item->cursed) decurse_weapon_swap = 0;
    if (of_has(item->flags, OF_HEAVY_CURSE)) decurse_weapon_swap = 1;
}

/*
 * Helper function -- notice the player swap armour
 */
void borg_notice_armour_swap(void)
{
    int i;
    int b_i = 0;
    s32b v = -1L;
    s32b b_v = 0L;
    int dam, damage;

    borg_item *item;

    armour_swap = 0;

    /* borg option to not use them */
    if (!borg_uses_swaps) return;

    /*** Process the inventory ***/
    for (i = 0; i < INVEN_MAX_PACK; i++)
    {
        item = &borg_items[i];

        /* reset counter */
        v= -1L;
        dam =0;
        damage =0;

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Hack -- skip un-aware items */
        if (!item->kind) continue;

		/* Skip non-wearable items */
		if (borg_slot(item->tval, item->sval) == -1) continue;

		/* Dont carry swaps until dlevel 50.  They are heavy */
        if (borg_skill[BI_MAXDEPTH] < 50) continue;

        /* Require "known" (or average, good, etc) */
        if (!item->ident &&
            !strstr(item->note, "magical") &&
            !strstr(item->note, "ego") &&
            !strstr(item->note, "splendid") &&
            !strstr(item->note, "excellent") &&
            !strstr(item->note, "terrible") &&
            !strstr(item->note, "special")) continue;

        /* One Ring is not a swap */
        if (item->activation == EFF_BIZARRE) continue;

        /* Clear all the swap weapon flags as I look at each one. */
        armour_swap_slay_animal = FALSE;
        armour_swap_slay_evil = FALSE;
        armour_swap_slay_undead = FALSE;
        armour_swap_slay_demon = FALSE;
        armour_swap_slay_orc = FALSE;
        armour_swap_slay_troll = FALSE;
        armour_swap_slay_giant = FALSE;
        armour_swap_slay_dragon = FALSE;
        armour_swap_kill_undead = FALSE;
        armour_swap_kill_demon = FALSE;
        armour_swap_kill_dragon = FALSE;
        armour_swap_impact = FALSE;
        armour_swap_brand_acid = FALSE;
        armour_swap_brand_elec = FALSE;
        armour_swap_brand_fire = FALSE;
        armour_swap_brand_cold = FALSE;
        armour_swap_brand_pois = FALSE;
        armour_swap_see_infra = FALSE;
        armour_swap_slow_digest = FALSE;
        armour_swap_aggravate = FALSE;
        armour_swap_teleport = FALSE;
        armour_swap_regenerate = FALSE;
        armour_swap_telepathy = FALSE;
        armour_swap_LIGHT = FALSE;
        armour_swap_see_invis = FALSE;
        armour_swap_ffall = FALSE;
        armour_swap_free_act = FALSE;
        armour_swap_hold_life = FALSE;
        armour_swap_immune_fire = FALSE;
        armour_swap_immune_acid = FALSE;
        armour_swap_immune_cold = FALSE;
        armour_swap_immune_elec = FALSE;
        armour_swap_resist_acid = FALSE;
        armour_swap_resist_elec = FALSE;
        armour_swap_resist_fire = FALSE;
        armour_swap_resist_cold = FALSE;
        armour_swap_resist_pois = FALSE;
        armour_swap_resist_conf = FALSE;
        armour_swap_resist_sound = FALSE;
        armour_swap_resist_LIGHT = FALSE;
        armour_swap_resist_dark = FALSE;
        armour_swap_resist_chaos = FALSE;
        armour_swap_resist_disen = FALSE;
        armour_swap_resist_shard = FALSE;
        armour_swap_resist_nexus = FALSE;
        armour_swap_resist_blind = FALSE;
        armour_swap_resist_neth = FALSE;
        decurse_armour_swap = -1;

        /* Analyze the item */
        switch (item->tval)
        {
            /* ARMOUR TYPE STUFF */
            case TV_RING:
            case TV_AMULET:
            case TV_BOOTS:
            case TV_HELM:
            case TV_CROWN:
            case TV_SHIELD:
            case TV_CLOAK:
            case TV_SOFT_ARMOR:
            case TV_HARD_ARMOR:
            case TV_DRAG_ARMOR:
            {
            /* various slays */
            /* as of 280, armours dont have slays but random artifacts might.
             */
            if (of_has(item->flags, OF_SLAY_ANIMAL)) armour_swap_slay_animal = TRUE;
            if (of_has(item->flags, OF_SLAY_EVIL))   armour_swap_slay_evil = TRUE;
            if (of_has(item->flags, OF_SLAY_UNDEAD)) armour_swap_slay_undead = TRUE;
            if (of_has(item->flags, OF_SLAY_DEMON))  armour_swap_slay_demon = TRUE;
            if (of_has(item->flags, OF_SLAY_ORC))    armour_swap_slay_orc = TRUE;
            if (of_has(item->flags, OF_SLAY_TROLL))  armour_swap_slay_troll = TRUE;
            if (of_has(item->flags, OF_SLAY_GIANT))  armour_swap_slay_giant = TRUE;
            if (of_has(item->flags, OF_SLAY_DRAGON)) armour_swap_slay_dragon = TRUE;
            if (of_has(item->flags, OF_KILL_UNDEAD)) armour_swap_kill_undead = TRUE;
            if (of_has(item->flags, OF_KILL_DEMON))  armour_swap_kill_demon = TRUE;
            if (of_has(item->flags, OF_KILL_DRAGON)) armour_swap_kill_dragon = TRUE;
            if (of_has(item->flags, OF_IMPACT))      armour_swap_impact = TRUE;
            if (of_has(item->flags, OF_BRAND_ACID))  armour_swap_brand_acid = TRUE;
            if (of_has(item->flags, OF_BRAND_ELEC))  armour_swap_brand_elec = TRUE;
            if (of_has(item->flags, OF_BRAND_FIRE))  armour_swap_brand_fire = TRUE;
            if (of_has(item->flags, OF_BRAND_COLD))  armour_swap_brand_cold = TRUE;
            if (of_has(item->flags, OF_BRAND_POIS))  armour_swap_brand_pois = TRUE;

            /* Affect infravision */
            if (of_has(item->flags, OF_INFRA)) armour_swap_see_infra += item->pval;
            /* Affect various skills */
            /* Affect speed */

            /* Various flags */
            if (of_has(item->flags, OF_SLOW_DIGEST)) armour_swap_slow_digest = TRUE;
            if (of_has(item->flags, OF_AGGRAVATE)) armour_swap_aggravate = TRUE;
            if (of_has(item->flags, OF_TELEPORT)) armour_swap_teleport = TRUE;
            if (of_has(item->flags, OF_REGEN)) armour_swap_regenerate = TRUE;
            if (of_has(item->flags, OF_TELEPATHY)) armour_swap_telepathy = TRUE;
            if (of_has(item->flags, OF_LIGHT)) armour_swap_LIGHT = TRUE;
            if (of_has(item->flags, OF_SEE_INVIS)) armour_swap_see_invis = TRUE;
            if (of_has(item->flags, OF_FEATHER)) armour_swap_ffall = TRUE;
            if (of_has(item->flags, OF_FREE_ACT)) armour_swap_free_act = TRUE;
            if (of_has(item->flags, OF_HOLD_LIFE)) armour_swap_hold_life = TRUE;

            /* Immunity flags */
            /* if you are immune you automaticly resist */
            if (of_has(item->flags, OF_IM_FIRE))
            {
                armour_swap_immune_fire = TRUE;
                armour_swap_resist_fire = TRUE;
            }
            if (of_has(item->flags, OF_IM_ACID))
            {
                armour_swap_immune_acid = TRUE;
                armour_swap_resist_acid = TRUE;
            }
            if (of_has(item->flags, OF_IM_COLD))
            {
                armour_swap_immune_cold = TRUE;
                armour_swap_resist_cold = TRUE;
            }
            if (of_has(item->flags, OF_IM_ELEC))
            {
                armour_swap_immune_elec = TRUE;
                armour_swap_resist_elec = TRUE;
            }

            /* Resistance flags */
            if (of_has(item->flags, OF_RES_ACID)) armour_swap_resist_acid = TRUE;
            if (of_has(item->flags, OF_RES_ELEC)) armour_swap_resist_elec = TRUE;
            if (of_has(item->flags, OF_RES_FIRE)) armour_swap_resist_fire = TRUE;
            if (of_has(item->flags, OF_RES_COLD)) armour_swap_resist_cold = TRUE;
            if (of_has(item->flags, OF_RES_POIS)) armour_swap_resist_pois = TRUE;
            if (of_has(item->flags, OF_RES_CONFU)) armour_swap_resist_conf = TRUE;
            if (of_has(item->flags, OF_RES_SOUND)) armour_swap_resist_sound = TRUE;
            if (of_has(item->flags, OF_RES_LIGHT)) armour_swap_resist_LIGHT = TRUE;
            if (of_has(item->flags, OF_RES_DARK)) armour_swap_resist_dark = TRUE;
            if (of_has(item->flags, OF_RES_CHAOS)) armour_swap_resist_chaos = TRUE;
            if (of_has(item->flags, OF_RES_DISEN)) armour_swap_resist_disen = TRUE;
            if (of_has(item->flags, OF_RES_SHARD)) armour_swap_resist_shard = TRUE;
            if (of_has(item->flags, OF_RES_NEXUS)) armour_swap_resist_nexus = TRUE;
            if (of_has(item->flags, OF_RES_BLIND)) armour_swap_resist_blind = TRUE;
            if (of_has(item->flags, OF_RES_NETHR)) armour_swap_resist_neth = TRUE;
            if (item->cursed) decurse_armour_swap = 0;
            if (of_has(item->flags, OF_HEAVY_CURSE)) decurse_armour_swap = 1;

            /* Sustain flags */

            /* calculating the value of the swap weapon. */
            damage = (item->dd * item->ds *35L);

            /* Reward "damage" and increased blows per round*/
            v += damage * (borg_skill[BI_BLOWS]+1);

            /* Reward "bonus to hit" */
            v += ((borg_skill[BI_TOHIT] + item->to_h)*100L);

            /* Reward "bonus to dam" */
            v += ((borg_skill[BI_TODAM] + item->to_d)*35L);

            dam = damage * borg_skill[BI_BLOWS];

            /* assume 2x base damage for x% of creatures */
            dam = damage * 2 * borg_skill[BI_BLOWS];

            if (!borg_skill[BI_WS_ANIMAL] && !borg_skill[BI_WB_ELEC] && armour_swap_slay_animal) v += (dam*2) /2;
            if (!borg_skill[BI_WS_EVIL] && armour_swap_slay_evil) v +=  (dam*7) /2;
            /* assume 3x base damage for x% of creatures */
            dam = damage *3*borg_skill[BI_BLOWS];

            if (!borg_skill[BI_WS_UNDEAD] && armour_swap_slay_undead) v += (dam*5) /2;
            if (!borg_skill[BI_WS_DEMON] && armour_swap_slay_demon) v += (dam*3) /2;
            if (!borg_skill[BI_WS_GIANT] && armour_swap_slay_giant) v += (dam*4) /2;
            if (!borg_skill[BI_WS_DRAGON] && !borg_skill[BI_WK_DRAGON] && armour_swap_slay_dragon) v += (dam*6) /2;
            if (!borg_skill[BI_WB_ACID] && armour_swap_brand_acid) v += (dam*4) /2;
            if (!borg_skill[BI_WB_ELEC] && armour_swap_brand_elec) v += (dam*5) /2;
            if (!borg_skill[BI_WB_FIRE] && armour_swap_brand_fire) v += (dam*3) /2;
            if (!borg_skill[BI_WB_COLD] && armour_swap_brand_cold) v += (dam*3) /2;
            if (!borg_skill[BI_WB_POIS] && armour_swap_brand_pois) v += (dam*3) /2;
            /* SOrc and STroll get 1/2 reward now */
            if (!borg_skill[BI_WS_ORC] && armour_swap_slay_orc) v += (dam*1) /2;
            if (!borg_skill[BI_WS_TROLL] && armour_swap_slay_troll) v += (dam*2) /2;
            /* SOrc and STroll get 2/2 reward if slay evil not possesed */
            if (!borg_skill[BI_WS_ORC] && !borg_skill[BI_WS_EVIL] && armour_swap_slay_orc) v += (dam*1) /2;
            if (!borg_skill[BI_WS_TROLL] && !borg_skill[BI_WS_EVIL] && armour_swap_slay_troll) v += (dam*1) /2;

            /* assume 5x base damage for x% of creatures */
            dam = damage  * 5 * borg_skill[BI_BLOWS];
            if (!borg_skill[BI_WK_UNDEAD] && armour_swap_kill_undead) v += (dam*5) /2;
            if (!borg_skill[BI_WK_DEMON] && armour_swap_kill_demon) v += (dam*3) /2;
            if (!borg_skill[BI_WK_DRAGON] && armour_swap_kill_dragon) v += (dam*5) /2;


            if (!borg_skill[BI_SDIG] && armour_swap_slow_digest) v += 10L;
            if (armour_swap_aggravate) v -= 8000L;
            if (armour_swap_teleport) v -= 100000L;
            if (decurse_armour_swap != -1) v -= 5000L;
            if (!borg_skill[BI_REG] && armour_swap_regenerate) v += 2000L;
            if (!borg_skill[BI_ESP] && armour_swap_telepathy) v += 5000L;
            if (!borg_skill[BI_LIGHT] && armour_swap_LIGHT) v += 2000L;
            if (!borg_skill[BI_SINV] && armour_swap_see_invis) v += 50000L;
            if (!borg_skill[BI_FEATH] && armour_swap_ffall) v += 10L;
            if (!borg_skill[BI_FRACT] && armour_swap_free_act) v += 10000L;
            if (!borg_skill[BI_HLIFE] && (borg_skill[BI_MAXCLEVEL] < 50) && armour_swap_hold_life) v += 2000L;
            if (!borg_skill[BI_IFIRE] && armour_swap_immune_fire) v += 70000L;
            if (!borg_skill[BI_IACID] && armour_swap_immune_acid) v += 30000L;
            if (!borg_skill[BI_ICOLD] && armour_swap_immune_cold) v += 50000L;
            if (!borg_skill[BI_IELEC] && armour_swap_immune_elec) v += 25000L;
            if (!borg_skill[BI_RFIRE] && armour_swap_resist_fire) v += 8000L;
            if (!borg_skill[BI_RACID] && armour_swap_resist_acid) v += 6000L;
            if (!borg_skill[BI_RCOLD] && armour_swap_resist_cold) v += 4000L;
            if (!borg_skill[BI_RELEC] && armour_swap_resist_elec) v += 3000L;
            /* extra bonus for getting all basic resist */
            if (armour_swap_resist_fire &&
                armour_swap_resist_acid &&
                armour_swap_resist_elec &&
                armour_swap_resist_cold) v +=  10000L;
            if (!borg_skill[BI_RPOIS] && armour_swap_resist_pois) v += 20000L;
            if (!borg_skill[BI_RCONF] && armour_swap_resist_conf) v += 5000L;
            if (!borg_skill[BI_RSND] && armour_swap_resist_sound) v += 2000L;
            if (!borg_skill[BI_RLITE] && armour_swap_resist_LIGHT) v += 800L;
            if (!borg_skill[BI_RDARK] && armour_swap_resist_dark) v += 800L;
            if (!borg_skill[BI_RKAOS] && armour_swap_resist_chaos) v += 8000L;
            if (!borg_skill[BI_RDIS] && armour_swap_resist_disen) v += 5000L;
            if (!borg_skill[BI_RSHRD] && armour_swap_resist_shard) v += 100L;
            if (!borg_skill[BI_RNXUS] && armour_swap_resist_nexus) v += 100L;
            if (!borg_skill[BI_RBLIND] && armour_swap_resist_blind) v += 5000L;
            if (!borg_skill[BI_RNTHR] && armour_swap_resist_neth) v += 5500L;
            /* Special concern if Tarraseque is alive */
            if (borg_skill[BI_MAXDEPTH] >= 75 &&
               ((!borg_skill[BI_ICOLD] && armour_swap_immune_cold) ||
                (!borg_skill[BI_IFIRE] && armour_swap_immune_fire)))
            {
               /* If Tarraseque is alive */
               if (borg_race_death[539] == 0)
               {
                  if (!borg_skill[BI_ICOLD] && armour_swap_immune_cold) v  += 90000L;
                  if (!borg_skill[BI_IFIRE] && armour_swap_immune_fire) v  += 90000L;
               }

            }



            /*  Mega-Hack -- resists (level 60) */
            /* Its possible that he will get a sword and a cloak
             * both with the same high resist and keep each based
             * on that resist.  We want him to check to see
             * that the other swap does not already have the high resist.
             */
            if (!borg_skill[BI_RNTHR]  && borg_skill[BI_MAXDEPTH]+1 >= 55  &&
                !weapon_swap_resist_neth &&
                armour_swap_resist_neth) v += 105000L;
            if (!borg_skill[BI_RKAOS] && borg_skill[BI_MAXDEPTH]+1 >= 60 &&
                !weapon_swap_resist_chaos &&
                armour_swap_resist_chaos) v += 104000L;
            if (!borg_skill[BI_RDIS] && borg_skill[BI_MAXDEPTH]+1 >= 60 &&
                !weapon_swap_resist_disen &&
                armour_swap_resist_disen) v += 100000L;

            /* some artifacts would make good back ups for their activation */

            }

            /* skip usless ones */
            if (v <= 1000) continue;

            /* collect the best one */
            if ((b_i >=0) && (v < b_v)) continue;

            /* track it */
            b_i = i;
            b_v = v;
            armour_swap_value = v;
            armour_swap = i;
        }
    }

        /* Now that we know who the best swap is lets set our swap
         * flags and get a move on
         */
        /*** Process the best inven item ***/

        item = &borg_items[b_i];

       /* Clear all the swap weapon flags as I look at each one. */
        armour_swap_slay_animal = FALSE;
        armour_swap_slay_evil = FALSE;
        armour_swap_slay_undead = FALSE;
        armour_swap_slay_demon = FALSE;
        armour_swap_slay_orc = FALSE;
        armour_swap_slay_troll = FALSE;
        armour_swap_slay_giant = FALSE;
        armour_swap_slay_dragon = FALSE;
        armour_swap_kill_dragon = FALSE;
        armour_swap_impact = FALSE;
        armour_swap_brand_acid = FALSE;
        armour_swap_brand_elec = FALSE;
        armour_swap_brand_fire = FALSE;
        armour_swap_brand_cold = FALSE;
        armour_swap_brand_pois = FALSE;
        armour_swap_see_infra = FALSE;
        armour_swap_slow_digest = FALSE;
        armour_swap_aggravate = FALSE;
        armour_swap_teleport = FALSE;
        armour_swap_regenerate = FALSE;
        armour_swap_telepathy = FALSE;
        armour_swap_LIGHT = FALSE;
        armour_swap_see_invis = FALSE;
        armour_swap_ffall = FALSE;
        armour_swap_free_act = FALSE;
        armour_swap_hold_life = FALSE;
        armour_swap_immune_fire = FALSE;
        armour_swap_immune_acid = FALSE;
        armour_swap_immune_cold = FALSE;
        armour_swap_immune_elec = FALSE;
        armour_swap_resist_acid = FALSE;
        armour_swap_resist_elec = FALSE;
        armour_swap_resist_fire = FALSE;
        armour_swap_resist_cold = FALSE;
        armour_swap_resist_pois = FALSE;
        armour_swap_resist_conf = FALSE;
        armour_swap_resist_sound = FALSE;
        armour_swap_resist_LIGHT = FALSE;
        armour_swap_resist_dark = FALSE;
        armour_swap_resist_chaos = FALSE;
        armour_swap_resist_disen = FALSE;
        armour_swap_resist_shard = FALSE;
        armour_swap_resist_nexus = FALSE;
        armour_swap_resist_blind = FALSE;
        armour_swap_resist_neth = FALSE;
        decurse_armour_swap = -1;

        /* various slays */
            if (of_has(item->flags, OF_SLAY_ANIMAL)) armour_swap_slay_animal = TRUE;
            if (of_has(item->flags, OF_SLAY_EVIL))   armour_swap_slay_evil = TRUE;
            if (of_has(item->flags, OF_SLAY_UNDEAD)) armour_swap_slay_undead = TRUE;
            if (of_has(item->flags, OF_SLAY_DEMON))  armour_swap_slay_demon = TRUE;
            if (of_has(item->flags, OF_SLAY_ORC))    armour_swap_slay_orc = TRUE;
            if (of_has(item->flags, OF_SLAY_TROLL))  armour_swap_slay_troll = TRUE;
            if (of_has(item->flags, OF_SLAY_GIANT))  armour_swap_slay_giant = TRUE;
            if (of_has(item->flags, OF_SLAY_DRAGON)) armour_swap_slay_dragon = TRUE;
            if (of_has(item->flags, OF_KILL_UNDEAD)) armour_swap_kill_undead = TRUE;
            if (of_has(item->flags, OF_KILL_DEMON)) armour_swap_kill_demon = TRUE;
            if (of_has(item->flags, OF_KILL_DRAGON)) armour_swap_kill_dragon = TRUE;
            if (of_has(item->flags, OF_IMPACT))      armour_swap_impact = TRUE;
            if (of_has(item->flags, OF_BRAND_ACID))  armour_swap_brand_acid = TRUE;
            if (of_has(item->flags, OF_BRAND_ELEC))  armour_swap_brand_elec = TRUE;
            if (of_has(item->flags, OF_BRAND_FIRE))  armour_swap_brand_fire = TRUE;
            if (of_has(item->flags, OF_BRAND_COLD))  armour_swap_brand_cold = TRUE;
            if (of_has(item->flags, OF_BRAND_POIS))  armour_swap_brand_pois = TRUE;

            /* Affect infravision */
            if (of_has(item->flags, OF_INFRA)) armour_swap_see_infra += item->pval;
            /* Affect various skills */
            /* Affect speed */

            /* Various flags */
            if (of_has(item->flags, OF_SLOW_DIGEST)) armour_swap_slow_digest = TRUE;
            if (of_has(item->flags, OF_AGGRAVATE)) armour_swap_aggravate = TRUE;
            if (of_has(item->flags, OF_TELEPORT)) armour_swap_teleport = TRUE;
            if (of_has(item->flags, OF_REGEN)) armour_swap_regenerate = TRUE;
            if (of_has(item->flags, OF_TELEPATHY)) armour_swap_telepathy = TRUE;
            if (of_has(item->flags, OF_LIGHT)) armour_swap_LIGHT = TRUE;
            if (of_has(item->flags, OF_SEE_INVIS)) armour_swap_see_invis = TRUE;
            if (of_has(item->flags, OF_FEATHER)) armour_swap_ffall = TRUE;
            if (of_has(item->flags, OF_FREE_ACT)) armour_swap_free_act = TRUE;
            if (of_has(item->flags, OF_HOLD_LIFE)) armour_swap_hold_life = TRUE;

            /* Immunity flags */
            /* if you are immune you automaticly resist */
            if (of_has(item->flags, OF_IM_FIRE))
            {
                armour_swap_immune_fire = TRUE;
                armour_swap_resist_fire = TRUE;
            }
            if (of_has(item->flags, OF_IM_ACID))
            {
                armour_swap_immune_acid = TRUE;
                armour_swap_resist_acid = TRUE;
            }
            if (of_has(item->flags, OF_IM_COLD))
            {
                armour_swap_immune_cold = TRUE;
                armour_swap_resist_cold = TRUE;
            }
            if (of_has(item->flags, OF_IM_ELEC))
            {
                armour_swap_immune_elec = TRUE;
                armour_swap_resist_elec = TRUE;
            }

            /* Resistance flags */
            if (of_has(item->flags, OF_RES_ACID)) armour_swap_resist_acid = TRUE;
            if (of_has(item->flags, OF_RES_ELEC)) armour_swap_resist_elec = TRUE;
            if (of_has(item->flags, OF_RES_FIRE)) armour_swap_resist_fire = TRUE;
            if (of_has(item->flags, OF_RES_COLD)) armour_swap_resist_cold = TRUE;
            if (of_has(item->flags, OF_RES_POIS)) armour_swap_resist_pois = TRUE;
            if (of_has(item->flags, OF_RES_CONFU)) armour_swap_resist_conf = TRUE;
            if (of_has(item->flags, OF_RES_SOUND)) armour_swap_resist_sound = TRUE;
            if (of_has(item->flags, OF_RES_LIGHT)) armour_swap_resist_LIGHT = TRUE;
            if (of_has(item->flags, OF_RES_DARK)) armour_swap_resist_dark = TRUE;
            if (of_has(item->flags, OF_RES_CHAOS)) armour_swap_resist_chaos = TRUE;
            if (of_has(item->flags, OF_RES_DISEN)) armour_swap_resist_disen = TRUE;
            if (of_has(item->flags, OF_RES_SHARD)) armour_swap_resist_shard = TRUE;
            if (of_has(item->flags, OF_RES_NEXUS)) armour_swap_resist_nexus = TRUE;
            if (of_has(item->flags, OF_RES_BLIND)) armour_swap_resist_blind = TRUE;
            if (of_has(item->flags, OF_RES_NETHR)) armour_swap_resist_neth = TRUE;
            if (item->cursed) decurse_armour_swap = 0;
            if (of_has(item->flags, OF_HEAVY_CURSE)) decurse_armour_swap = 1;

        enchant_armour_swap_to_a = 0;

        /* dont look for enchantment on non armours */
        if (item->tval >= TV_LIGHT) return;

        /* Hack -- enchant the swap equipment (armor) */
        /* Note need for enchantment */
        if ((borg_prayer_legal_fail(7, 4, 65) ||
             borg_spell_legal_fail(7, 2, 65) ||
             amt_enchant_armor >=1 ))
        {
            if (item->to_a < 10)
            {
                enchant_armour_swap_to_a += (10 - item->to_a);
            }
        }
        else
        {
            if (item->to_a < 8)
            {
                enchant_armour_swap_to_a += (8 - item->to_a);
            }
        }

}

/*
 * Analyze the equipment and inventory
 */
void borg_notice(bool notice_swap)
{
	int inven_weight;
	int carry_capacity;
	int i;

    /* Clear out 'has' array */
    memset(borg_has, 0, size_obj*sizeof(int));

    /* Many of our variables are tied to borg_skill[], which is erased at the
     * the start of borg_notice().  So we must update the frame the cheat in
     * all the non inventory skills.
     */
    borg_update_frame();

	/* The borg needs to update his base stat points */
	for (i =0; i < 6; i++)
	{
		/* Cheat the exact number from the game.  This number is available to the player
		 * on the extra term window.
		 */
		my_stat_cur[i] = p_ptr->stat_cur[i];

        /* Max stat is the max that the cur stat ever is. */
        if (my_stat_cur[i] > my_stat_max[i])
            my_stat_max[i] = my_stat_cur[i];
	}

    /* Notice the equipment */
    borg_notice_aux1();

    /* Notice the inventory */
    borg_notice_aux2();

    /* Notice and locate my swap weapon */
    if (notice_swap)
    {
        borg_notice_weapon_swap();
        borg_notice_armour_swap();
    }
    borg_skill[BI_SRACID] = borg_skill[BI_RACID]
                            || armour_swap_resist_acid
                            || weapon_swap_resist_acid
                            || borg_spell_legal_fail(4, 3, 15) /* Res FECAP */
							|| borg_spell_legal_fail(4, 3, 15); /* Res A */
    borg_skill[BI_SRELEC] = borg_skill[BI_RELEC]
                            || armour_swap_resist_elec
                            || weapon_swap_resist_elec
                            || borg_spell_legal_fail(4, 3, 15); /* Res FECAP */
    borg_skill[BI_SRFIRE] = borg_skill[BI_RFIRE]
                            || armour_swap_resist_fire
                            || weapon_swap_resist_fire
                            || borg_spell_legal_fail(4, 3, 15) /* Res FECAP */
							|| borg_prayer_legal_fail(1, 7, 15) /* Res FC */
							|| borg_spell_legal_fail(4, 1, 15); /* Res F */
	borg_skill[BI_SRCOLD] = borg_skill[BI_RCOLD]
                            || armour_swap_resist_cold
                            || weapon_swap_resist_cold
                            || borg_spell_legal_fail(4, 3, 15) /* Res FECAP */
							|| borg_prayer_legal_fail(1, 7, 15) /* Res FC */
							|| borg_spell_legal_fail(4, 0, 15); /* Res C */
    borg_skill[BI_SRPOIS] = borg_skill[BI_RPOIS]
                            || armour_swap_resist_pois
                            || weapon_swap_resist_pois
                            || borg_spell_legal_fail(4, 3, 15) /* Res FECAP */
							|| borg_spell_legal_fail(4, 2, 15); /* Res P */
    borg_skill[BI_SRFEAR] = borg_skill[BI_RFEAR]
                            || armour_swap_resist_fear
                            || weapon_swap_resist_fear;
    borg_skill[BI_SRLITE] = borg_skill[BI_RLITE]
                            || armour_swap_resist_LIGHT
                            || weapon_swap_resist_LIGHT;
    borg_skill[BI_SRDARK] = borg_skill[BI_RDARK]
                            || armour_swap_resist_dark
                            || weapon_swap_resist_dark;
    borg_skill[BI_SRBLIND] = borg_skill[BI_RBLIND]
                            || armour_swap_resist_blind
                            || weapon_swap_resist_blind;
    borg_skill[BI_SRCONF] = borg_skill[BI_RCONF]
                            || armour_swap_resist_conf
                            || weapon_swap_resist_conf;
    borg_skill[BI_SRSND] = borg_skill[BI_RSND]
                            || armour_swap_resist_sound
                            || weapon_swap_resist_sound;
    borg_skill[BI_SRSHRD] = borg_skill[BI_RSHRD]
                            || armour_swap_resist_shard
                            || weapon_swap_resist_shard;
    borg_skill[BI_SRNXUS] = borg_skill[BI_RNXUS]
                            || armour_swap_resist_nexus
                            || weapon_swap_resist_nexus;
    borg_skill[BI_SRNTHR] = borg_skill[BI_RNTHR]
                            || armour_swap_resist_neth
                            || weapon_swap_resist_neth;
    borg_skill[BI_SRKAOS] = borg_skill[BI_RKAOS]
                            || armour_swap_resist_chaos
                            || weapon_swap_resist_chaos;
    borg_skill[BI_SRDIS] = borg_skill[BI_RDIS]
                            || armour_swap_resist_disen
                            || weapon_swap_resist_disen;
    borg_skill[BI_SHLIFE] = borg_skill[BI_HLIFE]
                            || armour_swap_hold_life
                            || weapon_swap_hold_life;
    borg_skill[BI_SFRACT] = borg_skill[BI_FRACT]
                            || armour_swap_free_act
                            || weapon_swap_free_act;


    /* Hack -- Apply "encumbrance" from weight */
    /* Extract the current weight (in tenth pounds) */
    inven_weight = p_ptr->total_weight;

    /* Extract the "weight limit" (in tenth pounds) */
    carry_capacity = adj_str_wgt[my_stat_ind[A_STR]] * 100;

    /* Apply "encumbrance" from weight */
    if (inven_weight > carry_capacity/2) borg_skill[BI_SPEED] -= ((inven_weight - (carry_capacity/2)) / (carry_capacity / 10));

}

/*
 * Helper function -- notice the home equipment
 */
static void borg_notice_home_aux1(borg_item *in_item, bool no_items)
{

    /*** Reset counters ***/

    /* Reset basic */
    num_food = 0;
    num_fuel = 0;
    num_mold = 0;
    num_ident = 0;
    num_recall = 0;
    num_phase = 0;
    num_escape = 0;
	num_tele_staves = 0;
    num_teleport = 0;
    num_teleport_level =0;
	num_recharge = 0;

    num_artifact = 0;
    num_ego = 0;

    num_invisible = 0;
    num_pfe =0;
    num_glyph = 0;
    num_genocide = 0;
    num_mass_genocide = 0;
    num_berserk = 0;
    num_pot_rheat = 0;
    num_pot_rcold = 0;
    num_speed = 0;
	num_detonate = 0;

    num_slow_digest = 0;
    num_regenerate = 0;
    num_telepathy = 0;
    num_see_inv = 0;
    num_ffall = 0;
    num_free_act = 0;
    num_hold_life = 0;
    num_immune_acid = 0;
    num_immune_elec = 0;
    num_immune_fire = 0;
    num_immune_cold = 0;
    num_resist_acid = 0;
    num_resist_elec = 0;
    num_resist_fire = 0;
    num_resist_cold = 0;
    num_resist_pois = 0;
    num_resist_conf = 0;
    num_resist_sound = 0;
    num_resist_LIGHT = 0;
    num_resist_dark = 0;
    num_resist_chaos = 0;
    num_resist_disen = 0;
    num_resist_shard = 0;
    num_resist_nexus = 0;
    num_resist_blind = 0;
    num_resist_neth = 0;
    num_sustain_str = 0;
    num_sustain_int = 0;
    num_sustain_wis = 0;
    num_sustain_dex =0;
    num_sustain_con = 0;
    num_sustain_all = 0;

    home_stat_add[A_STR] = 0;
    home_stat_add[A_INT] = 0;
    home_stat_add[A_WIS] = 0;
    home_stat_add[A_DEX] = 0;
    home_stat_add[A_CON] = 0;
    home_stat_add[A_CHR] = 0;

    num_weapons = 0;

    num_bow =0;
    num_rings = 0;
    num_neck = 0;
    num_armor = 0;
    num_cloaks = 0;
    num_shields = 0;
    num_hats = 0;
    num_gloves = 0;
    num_boots = 0;
    num_LIGHT = 0;
    num_speed = 0;
    num_edged_weapon = 0;
    num_bad_gloves= 0;

    /* Reset healing */
    num_cure_critical = 0;
    num_cure_serious = 0;
    num_fix_exp = 0;
    num_mana = 0;
    num_heal = 0;
    num_ezheal = 0;
	num_life = 0;
    if (!in_item && !no_items) num_ezheal_true = 0;
    if (!in_item && !no_items) num_heal_true = 0;
    if (!in_item && !no_items) num_life_true = 0;

	/* Mushrooms */
	num_mush_second_sight = 0;		/* esp */
	num_mush_fast_recovery = 0;		/* cure stun, cut, pois, blind */
	num_mush_restoring = 0;			/* Restore All */
	num_mush_cure_mind = 0;			/* Cure confustion, Halluc, fear, tmp resist Conf */
	num_mush_emergency = 0;			/* Hallucinate, Oppose Fire, Oppose Cold, Heal 200 */
	num_mush_terror = 0;			/* Terror --give +5 speed boost */
	num_mush_stoneskin = 0;			/* StoneSkin */
	num_mush_debility = 0;			/* Mana Restore, temp loss of a stat (str/con) */
	num_mush_sprinting = 0;			/* Sprinting (speed +10) */
	num_mush_purging = 0;			/* Purging --Makes hungry, restore Str/Con, Cure Pois */

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
    num_fix_stat[6] = 0;

    /* Reset enchantment */
    num_enchant_to_a = 0;
    num_enchant_to_d = 0;
    num_enchant_to_h = 0;

    home_slot_free = 0;
    home_damage = 0;

    num_duplicate_items = 0;
}


/*
 * This checks for duplicate items in the home
 */
static void borg_notice_home_dupe(borg_item *item, bool check_sval, int i)
{
/* eventually check for power overlap... armor of resistence is same as weak elvenkind.*/
/*  two armors of elvenkind that resist poison is a dupe.  AJG*/

    int dupe_count, x;
    borg_item *item2;
	ego_item_type *e_ptr = &e_info[item->name2];

    /* check for a duplicate.  */
    /* be carefull about extra powers (elvenkind/magi) */
    if (e_ptr->xtra == OBJECT_XTRA_TYPE_RESIST || e_ptr->xtra == OBJECT_XTRA_TYPE_POWER) return;

    /* if this is a stack of items then all after the first are a */
    /* duplicate */
    dupe_count = item->iqty-1;

    /* Look for other items before this one that are the same */
    for (x = 0; x < i; x++)
    {
        if (x < STORE_INVEN_MAX)
            item2 = &borg_shops[7].ware[x];
        else
            /* Check what the borg has on as well.*/
            item2 = &borg_items[((x-STORE_INVEN_MAX)+INVEN_WIELD)];

        /* if everything matches it is a duplicate item */
        /* Note that we only check sval on certain items.  This */
        /* is because, for example, two pairs of dragon armor */
        /* are not the same unless thier subtype (color) matches */
        /* but a defender is a defender even if one is a dagger and */
        /* one is a mace */
        if ( (item->tval == item2->tval) &&
             (check_sval ? (item->sval == item2->sval) : TRUE) &&
             (item->name1 == item2->name1) &&
             (item->name2 == item2->name2) )
        {
            dupe_count++;
        }
    }

    /* there can be one dupe of rings because there are two ring slots. */
    if (item->tval == TV_RING && dupe_count)
        dupe_count--;

    /* Add this items count to the total duplicate count */
    num_duplicate_items += dupe_count;
}

/*
 * Helper function -- notice the home inventory
 */
static void borg_notice_home_aux2(borg_item *in_item, bool no_items)
{
    int i;

    borg_item *item;

    borg_shop *shop = &borg_shops[7];
    bitflag f[OF_SIZE];

    /*** Process the inventory ***/

    /* Scan the home */
    for (i = 0; i < (STORE_INVEN_MAX+(INVEN_TOTAL-INVEN_WIELD)); i++)
    {
        if (no_items) break;

        if (!in_item)
            if (i < STORE_INVEN_MAX)
                item = &shop->ware[i];
            else
                item = &borg_items[((i-STORE_INVEN_MAX)+INVEN_WIELD)];
        else
            item = in_item;

        /* Skip empty items */
        if (!item->iqty)
        {
            home_slot_free++;
            continue;
        }

        /* Hack -- skip un-aware items */
        if (!item->kind)
        {
            home_slot_free++;
            continue;
        }

        if (of_has(item->flags, OF_SLOW_DIGEST)) num_slow_digest += item->iqty;
        if (of_has(item->flags, OF_REGEN)) num_regenerate += item->iqty;
        if (of_has(item->flags, OF_TELEPATHY)) num_telepathy += item->iqty;
        if (of_has(item->flags, OF_SEE_INVIS)) num_see_inv += item->iqty;
        if (of_has(item->flags, OF_FEATHER)) num_ffall += item->iqty;
        if (of_has(item->flags, OF_FREE_ACT)) num_free_act += item->iqty;
        if (of_has(item->flags, OF_HOLD_LIFE)) num_hold_life += item->iqty;
        if (of_has(item->flags, OF_IM_FIRE))
        {
            num_immune_fire += item->iqty;
            num_resist_fire += item->iqty;
        }
        if (of_has(item->flags, OF_IM_ACID))
        {
            num_immune_acid += item->iqty;
            num_resist_acid += item->iqty;
        }
        if (of_has(item->flags, OF_IM_COLD))
        {
            num_immune_cold += item->iqty;
            num_resist_cold += item->iqty;
        }
        if (of_has(item->flags, OF_IM_ELEC))
        {
            num_immune_elec += item->iqty;
            num_resist_elec += item->iqty;
        }
        if (of_has(item->flags, OF_RES_ACID)) num_resist_acid += item->iqty;
        if (of_has(item->flags, OF_RES_ELEC)) num_resist_elec += item->iqty;
        if (of_has(item->flags, OF_RES_FIRE)) num_resist_fire += item->iqty;
        if (of_has(item->flags, OF_RES_COLD)) num_resist_cold += item->iqty;
        if (of_has(item->flags, OF_RES_POIS)) num_resist_pois += item->iqty;
        if (of_has(item->flags, OF_RES_SOUND)) num_resist_sound += item->iqty;
        if (of_has(item->flags, OF_RES_LIGHT)) num_resist_LIGHT += item->iqty;
        if (of_has(item->flags, OF_RES_DARK)) num_resist_dark += item->iqty;
        if (of_has(item->flags, OF_RES_CHAOS)) num_resist_chaos += item->iqty;
        if (of_has(item->flags, OF_RES_CONFU)) num_resist_conf += item->iqty;
        if (of_has(item->flags, OF_RES_DISEN)) num_resist_disen += item->iqty;
        if (of_has(item->flags, OF_RES_SHARD)) num_resist_shard += item->iqty;
        if (of_has(item->flags, OF_RES_NEXUS)) num_resist_nexus += item->iqty;
        if (of_has(item->flags, OF_RES_BLIND)) num_resist_blind += item->iqty;
        if (of_has(item->flags, OF_RES_NETHR)) num_resist_neth += item->iqty;

        /* Count Sustains */
        if (of_has(item->flags, OF_SUST_STR)) num_sustain_str += item->iqty;
        if (of_has(item->flags, OF_SUST_INT)) num_sustain_str += item->iqty;
        if (of_has(item->flags, OF_SUST_WIS)) num_sustain_str += item->iqty;
        if (of_has(item->flags, OF_SUST_DEX)) num_sustain_str += item->iqty;
        if (of_has(item->flags, OF_SUST_CON)) num_sustain_str += item->iqty;
        if (of_has(item->flags, OF_SUST_STR) &&
           of_has(item->flags, OF_SUST_INT)  &&
           of_has(item->flags, OF_SUST_WIS)  &&
           of_has(item->flags, OF_SUST_DEX)  &&
           of_has(item->flags, OF_SUST_CON)) num_sustain_all +=item->iqty;

        /* count up bonus to stats */
        /* HACK only collect stat rings above +3 */
        if (of_has(item->flags, OF_STR))
        {
            if (item->tval != TV_RING || item->pval > 3)
                home_stat_add[A_STR] += item->pval * item->iqty;
        }
        if (of_has(item->flags, OF_INT))
        {
            if (item->tval != TV_RING || item->pval > 3)
                home_stat_add[A_INT] += item->pval * item->iqty;
        }
        if (of_has(item->flags, OF_WIS))
        {
            if (item->tval != TV_RING || item->pval > 3)
                home_stat_add[A_WIS] += item->pval * item->iqty;
        }
        if (of_has(item->flags, OF_DEX))
        {
            if (item->tval != TV_RING || item->pval > 3)
                home_stat_add[A_DEX] += item->pval * item->iqty;
        }
        if (of_has(item->flags, OF_CON))
        {
            if (item->tval != TV_RING || item->pval > 3)
                home_stat_add[A_CON] += item->pval * item->iqty;
        }
        if (of_has(item->flags, OF_CHR))
        {
            if (item->tval != TV_RING || item->pval > 3)
                home_stat_add[A_CHR] += item->pval * item->iqty;
        }

        /* count up bonus to speed */
        if (of_has(item->flags, OF_SPEED)) num_speed += item->pval * item->iqty;

        /* count artifacts */
        if (item->name1)
        {
            num_artifact += item->iqty;
        }
        /* count egos that need *ID* */
        if ((e_info[item->name2].xtra  == OBJECT_XTRA_TYPE_RESIST || e_info[item->name2].xtra == OBJECT_XTRA_TYPE_POWER) &&
			!item->fully_identified)
        {
            num_ego += item->iqty;
        }

        /* Analyze the item */
        switch (item->tval)
        {
            case TV_SOFT_ARMOR:
            case TV_HARD_ARMOR:
                num_armor += item->iqty;

                /* see if this item is duplicated */
                borg_notice_home_dupe( item, FALSE, i );
                break;

            case TV_DRAG_ARMOR:
                num_armor += item->iqty;

                /* see if this item is duplicated */
                borg_notice_home_dupe( item, TRUE, i );
                break;

            case TV_CLOAK:
                num_cloaks += item->iqty;

                /* see if this item is duplicated */
                borg_notice_home_dupe( item, FALSE, i );

                break;

            case TV_SHIELD:
                num_shields += item->iqty;

                /* see if this item is duplicated */
                borg_notice_home_dupe( item, FALSE, i );
                break;

            case TV_HELM:
            case TV_CROWN:
                num_hats += item->iqty;

                /* see if this item is duplicated */
                borg_notice_home_dupe( item, FALSE, i );

                break;

            case TV_GLOVES:
                num_gloves += item->iqty;

                /* most gloves hurt magic for spell-casters */
                if (player_has(PF_CUMBER_GLOVE) && borg_skill[BI_MAXSP] > 3)
                {
                    /* Penalize non-usable gloves */
                    if (item->iqty &&
                        (!of_has(item->flags, OF_FREE_ACT) &&
                        !of_has(item->flags, OF_DEX) && item->pval > 0))
                    {
                        num_bad_gloves += item->iqty;
                    }
                }

                /* gloves of slaying give a damage bonus */
                home_damage += item->to_d * 3;

                /* see if this item is duplicated */
                borg_notice_home_dupe( item, FALSE, i );

                break;

	        case TV_FLASK:
	            /* Use as fuel if we equip a lantern */
	            if (borg_items[INVEN_LIGHT].sval == SV_LIGHT_LANTERN)
	            {
					num_fuel += item->iqty;
					/* borg_note(format("1.num_fuel=%d",num_fuel)); */
				}
				break;

            case TV_LIGHT:
				/* Fuel */
	            if (borg_items[INVEN_LIGHT].sval == SV_LIGHT_TORCH)
	            {
					num_fuel += item->iqty;
				}

		        /* Artifacts */
		        if (item->name1)
                {
                    num_LIGHT += item->iqty;
                }
                break;

            case TV_BOOTS:
                num_boots += item->iqty;

                /* see if this item is duplicated */
                borg_notice_home_dupe( item, FALSE, i );
                break;

            case TV_SWORD:
            case TV_POLEARM:
            case TV_HAFTED:
            /* case TV_DIGGING: */
            {
                s16b num_blow;

                num_weapons += item->iqty;
                /*  most edged weapons hurt magic for priests */
                if (player_has(PF_BLESS_WEAPON))
                {
                    /* Penalize non-blessed edged weapons */
                    if ((item->tval == TV_SWORD || item->tval == TV_POLEARM) &&
                        !of_has(item->flags, OF_BLESSED))
                    {
                        num_edged_weapon += item->iqty;
                    }
                }


                /* NOTE:  This damage does not take slays into account. */
                /* it is just a rough estimate to make sure the glave of pain*/
                /* is kept if it is found */
                /* It is hard to hold a heavy weapon */
                num_blow = 1;
                if (adj_str_hold[my_stat_ind[A_STR]] >= item->weight / 10)
                {
                    int str_index, dex_index;
                    int num = 0, wgt = 0, mul = 0, div = 0;

                    /* Analyze the class */
                    switch (borg_class)
                    {
                        /* Warrior */
                        case CLASS_WARRIOR: num = 6; wgt = 30; mul = 5; break;

                        /* Mage */
                        case CLASS_MAGE: num = 4; wgt = 40; mul = 2; break;

                        /* Priest (was mul = 3.5) */
                        case CLASS_PRIEST: num = 5; wgt = 35; mul = 3; break;

                        /* Rogue */
                        case CLASS_ROGUE: num = 5; wgt = 30; mul = 3; break;

                        /* Ranger */
                        case CLASS_RANGER: num = 5; wgt = 35; mul = 4; break;

                        /* Paladin */
                        case CLASS_PALADIN: num = 5; wgt = 30; mul = 4; break;

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
                    num_blow = blows_table[str_index][dex_index];

                    /* Maximal value */
                    if (num_blow > num) num_blow = num;

                }

                /* Require at least one blow */
                if (num_blow < 1) num_blow = 1;

                if (of_has(item->flags, OF_BLOWS)) num_blow += item->pval;
                num_blow *= item->iqty;
                if ( item->to_d > 8 || borg_skill[BI_CLEVEL] < 15 )
                {
                    home_damage += num_blow * (item->dd * (item->ds) +
                                         (borg_skill[BI_TODAM] + item->to_d));
                }
                else
                {
                    home_damage += num_blow * (item->dd * (item->ds) +
                                        (borg_skill[BI_TODAM] + 8));
                }

                /* see if this item is a duplicate */
                borg_notice_home_dupe( item, FALSE, i );
                break;
            }

            case TV_BOW:
                num_bow += item->iqty;

                /* see if this item is a duplicate */
                borg_notice_home_dupe( item, FALSE, i );
                break;

            case TV_RING:
                num_rings += item->iqty;

                /* see if this item is a duplicate */
                borg_notice_home_dupe( item, TRUE, i );

                break;

            case TV_AMULET:
                num_neck += item->iqty;

                /* see if this item is a duplicate */
                borg_notice_home_dupe( item, TRUE, i );
                break;


            /* Books */
            case TV_MAGIC_BOOK:
            case TV_PRAYER_BOOK:

            /* Skip incorrect books */
            if (item->tval != p_ptr->class->spell_book) break;

            /* Count the books */
            num_book[item->sval] += item->iqty;

            break;


            /* Food */
            case TV_FOOD:

            /* Analyze */
            switch (item->sval)
            {
                case SV_FOOD_WAYBREAD:
                num_food += item->iqty;
                break;

                case SV_FOOD_RATION:
                num_food += item->iqty;
                break;

                case SV_FOOD_SLIME_MOLD:
                num_mold += item->iqty;
                break;


				case SV_FOOD_PURGING:
                num_fix_stat[A_CON] += item->iqty;
                num_fix_stat[A_STR] += item->iqty;
				num_mush_purging += item->iqty;
                break;

                case SV_FOOD_RESTORING:
                num_fix_stat[A_STR] += item->iqty;
                num_fix_stat[A_INT] += item->iqty;
                num_fix_stat[A_WIS] += item->iqty;
                num_fix_stat[A_DEX] += item->iqty;
                num_fix_stat[A_CON] += item->iqty;
                num_fix_stat[A_CHR] += item->iqty;
                num_fix_stat[6]     += item->iqty;
                break;

				/* Mushrooms added in 312 */
				case SV_FOOD_SECOND_SIGHT:
					num_mush_second_sight +=item->iqty;
					break;
				case SV_FOOD_EMERGENCY:
					num_mush_emergency +=item->iqty;
					break;
				case SV_FOOD_TERROR:
					num_mush_terror +=item->iqty;
					break;
				case SV_FOOD_STONESKIN:
					num_mush_stoneskin +=item->iqty;
					break;
				case SV_FOOD_DEBILITY:
					num_mush_debility +=item->iqty;
					break;
				case SV_FOOD_SPRINTING:
					num_mush_sprinting +=item->iqty;
					break;
				case SV_FOOD_FAST_RECOVERY:
					num_mush_fast_recovery +=item->iqty;
					break;
				case SV_FOOD_CURE_MIND:
					num_mush_cure_mind +=item->iqty;
					break;
			}

            break;


            /* Potions */
            case TV_POTION:

            /* Analyze */
            switch (item->sval)
            {
                case SV_POTION_CURE_CRITICAL:
                num_cure_critical += item->iqty;
                break;

                case SV_POTION_CURE_SERIOUS:
                num_cure_serious += item->iqty;
                break;

                case SV_POTION_RESIST_HEAT:
                num_pot_rheat += item->iqty;
                break;
                case SV_POTION_RESIST_COLD:
                num_pot_rcold += item->iqty;
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

                case SV_POTION_RESTORE_MANA:
                num_mana += item->iqty;
                break;

                case SV_POTION_HEALING:
                num_heal += item->iqty;
                if (!in_item && !no_items) num_heal_true += item->iqty;
                break;

                case SV_POTION_STAR_HEALING:
                num_ezheal += item->iqty;
                if (!in_item && !no_items) num_ezheal_true += item->iqty;
                break;

                case SV_POTION_LIFE:
                num_life += item->iqty;
                if (!in_item && !no_items) num_life_true += item->iqty;
                break;

                case SV_POTION_BERSERK_STRENGTH:
                num_berserk += item->iqty;
                break;

                case SV_POTION_SPEED:
                num_speed += item->iqty;
                break;

				case SV_POTION_DETONATIONS:
				num_detonate +=item->iqty;
				break;

            }

            break;


            /* Scrolls */
            case TV_SCROLL:

            /* Analyze the scroll */
            switch (item->sval)
            {
                case SV_SCROLL_IDENTIFY:
                num_ident += item->iqty;
                break;

                case SV_SCROLL_PHASE_DOOR:
                num_phase += item->iqty;
                break;

                case SV_SCROLL_TELEPORT:
                num_teleport += item->iqty;
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

                case SV_SCROLL_PROTECTION_FROM_EVIL:
                num_pfe += item->iqty;
                break;

                case SV_SCROLL_RUNE_OF_PROTECTION:
                num_glyph += item->iqty;
                break;

                case SV_SCROLL_TELEPORT_LEVEL:
                num_teleport_level += item->iqty;
                break;

                case SV_SCROLL_RECHARGING:
                num_recharge += item->iqty;
                break;

				case SV_SCROLL_MASS_BANISHMENT:
				num_mass_genocide += item->iqty;
				break;
            }

            break;


            /* Rods */
            case TV_ROD:

            /* Analyze */
            switch (item->sval)
            {
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

            /* only collect staves with more than 3 charges at high level */
            if (item->pval <= 3 && borg_skill[BI_CLEVEL] > 30)
                break;

            /* Analyze */
            switch (item->sval)
            {
                case SV_STAFF_IDENTIFY:
                num_ident += item->pval * item->iqty;
                break;

                case SV_STAFF_TELEPORTATION:
                num_escape += item->pval * item->iqty;
                num_tele_staves ++;
                break;
            }

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

        /* if only doing one item, break. */
        if (in_item) break;
    }


    /*** Process the Spells and Prayers ***/

    /* Handle "satisfy hunger" -> infinite food */
    if (borg_spell_legal(2, 0) || borg_prayer_legal(1, 5))
    {
        num_food += 1000;
    }

    /* Handle "identify" -> infinite identifies */
    if (borg_spell_legal(2, 5) ||
	    borg_prayer_legal(5, 2))
    {
        num_ident += 1000;
    }

    /* Handle "enchant weapon" */
    if (borg_spell_legal_fail(7, 3, 65) ||
        borg_prayer_legal_fail(7, 3, 65))
    {
        num_enchant_to_h += 1000;
        num_enchant_to_d += 1000;
    }

    /*  Handle "protection from evil" */
    if (borg_prayer_legal(2, 4))
    {
        num_pfe += 1000;
    }

    /*  Handle "rune of protection" glyph */
    if (borg_prayer_legal(3, 4) ||
    	borg_spell_legal(6, 4))
    {
        num_glyph += 1000;
    }

    /* handle restore */

    /* Handle recall */
    if (borg_prayer_legal(4, 4) ||
        borg_spell_legal(6, 3))
    {
        num_recall += 1000;
    }

    /* Handle teleport_level */
    if (borg_prayer_legal(4, 3) ||
        borg_spell_legal(6, 2))
    {
        num_teleport_level += 1000;
    }

    /* Handle recharge */
    if (borg_prayer_legal(7, 1) ||
        borg_spell_legal(2, 1) ||
        borg_spell_legal(7, 4))
    {
        num_recharge += 1000;
    }

    /*** Process the Needs ***/

    /* Hack -- No need for stat repair */
    if (borg_skill[BI_SSTR]) num_fix_stat[A_STR] += 1000;
    if (borg_skill[BI_SINT]) num_fix_stat[A_INT] += 1000;
    if (borg_skill[BI_SWIS]) num_fix_stat[A_WIS] += 1000;
    if (borg_skill[BI_SDEX]) num_fix_stat[A_DEX] += 1000;
    if (borg_skill[BI_SCON]) num_fix_stat[A_CON] += 1000;
    if (borg_skill[BI_SCHR]) num_fix_stat[A_CHR] += 1000;

    /* Extract the player flags */
    player_flags(f);

    /* Good flags */
    if (rf_has(f, OF_SLOW_DIGEST)) num_slow_digest = TRUE;
    if (rf_has(f, OF_FEATHER)) num_ffall = TRUE;
    if (rf_has(f, OF_LIGHT)) num_LIGHT = TRUE;
    if (rf_has(f, OF_REGEN)) num_regenerate = TRUE;
    if (rf_has(f, OF_TELEPATHY)) num_telepathy = TRUE;
    if (rf_has(f, OF_SEE_INVIS)) num_see_inv = TRUE;
    if (rf_has(f, OF_FREE_ACT)) num_free_act = TRUE;
    if (rf_has(f, OF_HOLD_LIFE)) num_hold_life = TRUE;

    /* Weird flags */

    /* Bad flags */

    /* Immunity flags */
    if (rf_has(f, OF_IM_FIRE)) num_immune_fire = TRUE;
    if (rf_has(f, OF_IM_ACID)) num_immune_acid = TRUE;
    if (rf_has(f, OF_IM_COLD)) num_immune_cold = TRUE;
    if (rf_has(f, OF_IM_ELEC)) num_immune_elec = TRUE;

    /* Resistance flags */
    if (rf_has(f, OF_RES_ACID)) num_resist_acid = TRUE;
    if (rf_has(f, OF_RES_ELEC)) num_resist_elec = TRUE;
    if (rf_has(f, OF_RES_FIRE)) num_resist_fire = TRUE;
    if (rf_has(f, OF_RES_COLD)) num_resist_cold = TRUE;
    if (rf_has(f, OF_RES_POIS)) num_resist_pois = TRUE;
    if (rf_has(f, OF_RES_LIGHT)) num_resist_LIGHT = TRUE;
    if (rf_has(f, OF_RES_DARK)) num_resist_dark = TRUE;
    if (rf_has(f, OF_RES_BLIND)) num_resist_blind = TRUE;
    if (rf_has(f, OF_RES_CONFU)) num_resist_conf = TRUE;
    if (rf_has(f, OF_RES_SOUND)) num_resist_sound = TRUE;
    if (rf_has(f, OF_RES_SHARD)) num_resist_shard = TRUE;
    if (rf_has(f, OF_RES_NEXUS)) num_resist_nexus = TRUE;
    if (rf_has(f, OF_RES_NETHR)) num_resist_neth = TRUE;
    if (rf_has(f, OF_RES_CHAOS)) num_resist_chaos = TRUE;
    if (rf_has(f, OF_RES_DISEN)) num_resist_disen = TRUE;

    /* Sustain flags */
    if (rf_has(f, OF_SUST_STR)) num_sustain_str = TRUE;
    if (rf_has(f, OF_SUST_INT)) num_sustain_int = TRUE;
    if (rf_has(f, OF_SUST_WIS)) num_sustain_wis = TRUE;
    if (rf_has(f, OF_SUST_DEX)) num_sustain_dex = TRUE;
    if (rf_has(f, OF_SUST_CON)) num_sustain_con = TRUE;

}

/*
 * Extract the bonuses for items in the home.
 *
 * in_item is passed in if you want to pretend that in_item is
 *          the only item in the home.
 * no_items is passed in as TRUE if you want to pretend that the
 *          home is empty.
 */
void borg_notice_home(borg_item *in_item, bool no_items)
{
    /* Notice the home equipment */
    borg_notice_home_aux1(in_item, no_items);

    /* Notice the home inventory */
    borg_notice_home_aux2(in_item, no_items);
}


/*
 * Helper function -- calculate "power" of equipment
 */
static s32b borg_power_aux1(void)
{
    int         hold;
    int         damage, dam;

    int         i;

    int         cur_wgt = 0;
    int         max_wgt = 0;

    s32b        value = 0L;

    borg_item       *item;


    /* Obtain the "hold" value (weight limit for weapons) */
    hold = adj_str_hold[my_stat_ind[A_STR]];

    /*** Analyze weapon ***/

    /* Examine current weapon */
    item = &borg_items[INVEN_WIELD];

	/* We give a bonus to wearing an unID'D sword in order to use it and
	 * garner a pseudoID from it.  We do not do this late in the game though
	 * because our weapon often has traits that we need in order to be deep (FA, SeeInvis)
	 */
	if (borg_skill[BI_CDEPTH] < 10 && borg_skill[BI_MAXCLEVEL] < 15 &&
		item->iqty && item->ident != TRUE) value += 1000000;

    /* Calculate "average" damage per "normal" blow  */
    /* and assume we can enchant up to +8 if borg_skill[BI_CLEVEL] > 25 */
    damage = (item->dd * item->ds * 20L);


    /* Reward "damage" and increased blows per round*/
    value += damage * (borg_skill[BI_BLOWS]+1);

    /* Reward "bonus to hit" */
    value += ((borg_skill[BI_TOHIT] + item->to_h)*100L );

    /* Reward "bonus to dam" */
    value += ((borg_skill[BI_TODAM] + item->to_d)*30L);

    /* extra damage for some */
    if (borg_worships_damage)
    {
        value += ((borg_skill[BI_TOHIT] + item->to_h)*15L);
    }

    /* extra boost for deep dungeon */
    if (borg_skill[BI_MAXDEPTH] >= 75)
    {
        value += ((borg_skill[BI_TOHIT] + item->to_h)*15L);

        value += item->dd *
                 item->ds * 20L *
                 2 * borg_skill[BI_BLOWS];
    }

    /* assume 2x base damage for x% of creatures */
    dam = damage * 2 * borg_skill[BI_BLOWS];
    if (borg_skill[BI_WS_ANIMAL]) value += (dam * 2) / 2;
    if (borg_skill[BI_WS_EVIL])   value += (dam * 7) / 2;

    /* extra damage for some */
    if (borg_worships_damage)
    {
        value += (dam);
    }

    /* assume 3x base damage for x% of creatures */
    dam = damage  * 3 * borg_skill[BI_BLOWS];
    if (borg_skill[BI_WS_UNDEAD] && (!borg_skill[BI_WK_UNDEAD])) value += (dam * 5) / 2;
    if (borg_skill[BI_WS_DEMON] && (!borg_skill[BI_WK_DEMON]))  value += (dam * 3) / 2;
    if (borg_skill[BI_WS_DRAGON] && (!borg_skill[BI_WK_DRAGON])) value += (dam * 6) / 2;
    if (borg_skill[BI_WS_GIANT])  value += (dam * 4) / 2;
    if (borg_skill[BI_WB_ACID])  value += (dam * 4) / 2;
    if (borg_skill[BI_WB_ELEC])  value += (dam * 5) / 2;
    if (borg_skill[BI_WB_FIRE])  value += (dam * 3) / 2;
    if (borg_skill[BI_WB_COLD])  value += (dam * 3) / 2;
    /* SOrc and STroll get 1/2 of reward now */
    if (borg_skill[BI_WS_ORC] )    value += (dam * 1) / 2;
    if (borg_skill[BI_WS_TROLL] )  value += (dam * 2) / 2;
    /* and the other 2/2 if SEvil not possesed */
    if (borg_skill[BI_WS_ORC] && !borg_skill[BI_WS_EVIL])    value += (dam * 1) / 2;
    if (borg_skill[BI_WS_TROLL] && !borg_skill[BI_WS_EVIL])  value += (dam * 1) / 2;

    /* extra damage for some */
    if (borg_worships_damage)
    {
        value += (dam);
    }

    /* assume 5x base damage for x% of creatures */
    dam = damage  * 5 * borg_skill[BI_BLOWS];
    if (borg_skill[BI_WK_UNDEAD]) value += (dam * 5) / 2;
    if (borg_skill[BI_WK_DEMON]) value += (dam * 5) / 2;
    if (borg_skill[BI_WK_DRAGON]) value += (dam * 5) / 2;
    /* extra damage for some */
    if (borg_worships_damage)
    {
        value += (dam);
    }

    /* It used to be only on Grond */
    if (borg_skill[BI_W_IMPACT]) value += 50L;

    /* Hack -- It is hard to hold a heavy weapon */
    if (borg_skill[BI_HEAVYWEPON]) value -= 500000L;

    /* HACK -- Borg worships num_blow, even on broken swords. */
    /* kind 47 is a broken sword usually 1d2 in damage */
    /* if (item->kind == 47 || item->kind == 30 ||item->kind == 390 ) value -=90000L; */


	/* We want low level borgs to have high blows (dagger, whips) */
	if (borg_skill[BI_CLEVEL] <= 10) value += borg_skill[BI_BLOWS] * 45000L;

    /*** Analyze bow ***/

    /* Examine current bow */
    item = &borg_items[INVEN_BOW];

	/* We give a bonus to wearing an unID'D bow in order to use it and
	 * garner a pseudoID from it.  We do not do this late in the game though
	 * because our weapon often has traits that we need in order to be deep (FA, SeeInvis)
	 */
	if (borg_skill[BI_CDEPTH] < 10 && borg_skill[BI_MAXCLEVEL] < 15 &&
		item->iqty && item->ident != TRUE) value += 6000000;

	/* Calculate "average" damage per "normal" shot (times 2) */
    if ( item->to_d > 8 || borg_skill[BI_CLEVEL] < 25 )
        damage = ((my_ammo_sides) + (item->to_d)) * my_ammo_power;
    else
        damage = (my_ammo_sides + 8) * my_ammo_power;

    /* Reward "damage" */
    if (borg_worships_damage)
    {
        value += (borg_skill[BI_SHOTS] * damage * 11L);
    }
    else
    {
        value += (borg_skill[BI_SHOTS] * damage * 9L);
    }

	/* Extra bonus for low levels, they need a ranged weap */
	if (borg_skill[BI_CLEVEL] < 15) value += (borg_skill[BI_SHOTS] * damage * 200L);


    /* slings force you to carry heavy ammo.  Penalty for that unles you have lots of str  */
    if (item->sval == SV_SLING &&
        !item->name1 &&
        my_stat_ind[A_STR] < 9)
    {
        value -= 5000L;
    }

	/* Bonus if level 1 to buy a sling, they are cheap ranged weapons */
	if (item->sval == SV_SLING && borg_skill[BI_CLEVEL] ==1 && my_stat_ind[A_STR] >= 9) value += 8000;


    /* Reward "bonus to hit" */
    value += ((borg_skill[BI_TOHIT] + item->to_h)*100L);;

    /* extra damage for some */
    if (borg_worships_damage)
    {
        value += ((borg_skill[BI_TOHIT] + item->to_h) * 25L);
    }


    /* Prefer bows */
    if (player_has(PF_EXTRA_SHOT) && my_ammo_tval == TV_ARROW) value += 30000L;

    /* Hack -- It is hard to hold a heavy weapon */
    if (hold < item->weight / 10) value -= 500000L;



    /***  Analyze dragon armour  ***/

    /* Examine current armor */
    item = &borg_items[INVEN_BODY];

    if (item->tval == TV_DRAG_ARMOR && !item->name1)
      {
          switch( item->sval)
          {
              case SV_DRAGON_BLACK:
              case SV_DRAGON_BLUE:
              case SV_DRAGON_WHITE:
              case SV_DRAGON_RED:
                  value += 1100;
                  break;
              case SV_DRAGON_GREEN:
                  value += 2750;
                  break;
              case SV_DRAGON_MULTIHUED:
                  value += 3250;
                  break;
              case SV_DRAGON_SHINING:
              case SV_DRAGON_LAW:
              case SV_DRAGON_BRONZE:
              case SV_DRAGON_GOLD:
              case SV_DRAGON_CHAOS:
              case SV_DRAGON_BALANCE:
              case SV_DRAGON_POWER:
                  value += 5150;
          }
      }

	/*** Examine the Rings for special types ***/
	for (i = INVEN_LEFT; i <= INVEN_RIGHT; i++)
	{
	    /* Obtain the item */
		item = &borg_items[i];

		/* Reward the [Elemental] protection rings for their activation */
		if (item->kind == RING_FLAMES)	value += 25000;
		if (item->kind == RING_ACID)	value += 10000;
		if (item->kind == RING_ICE)		value += 15000;
		if (item->kind == RING_LIGHTNING)	value += 10000;
	}

    /*** Reward various things ***/

    /* Hack -- Reward light radius */
    if (borg_skill[BI_CURLITE] <= 3) value += (borg_skill[BI_CURLITE] * 100000L);
    if (borg_skill[BI_CURLITE] > 3) value += (300000L) + (borg_skill[BI_CURLITE] * 1000);



    /* Hack -- Reward speed
     * see if speed can be a bonus if good speed; not +3.
     * reward higher for +10 than +50 speed (decreased return).
     */
    if (borg_worships_speed)
    {
        if (borg_skill[BI_SPEED] >= 150)
            value += (((borg_skill[BI_SPEED] - 120) * 1500L) + 185000L);

        if (borg_skill[BI_SPEED] >= 145 && borg_skill[BI_SPEED] <= 149)
            value += (((borg_skill[BI_SPEED] - 120) * 1500L) + 180000L);

        if (borg_skill[BI_SPEED] >= 140 && borg_skill[BI_SPEED] <= 144)
            value += (((borg_skill[BI_SPEED] - 120) * 1500L) + 175000L);

        if (borg_skill[BI_SPEED] >= 135 && borg_skill[BI_SPEED] <= 139)
            value += (((borg_skill[BI_SPEED] - 120) * 1500L) + 175000L);

        if (borg_skill[BI_SPEED] >= 130 && borg_skill[BI_SPEED] <= 134)
            value += (((borg_skill[BI_SPEED] - 120) * 1500L) + 160000L);

        if (borg_skill[BI_SPEED] >= 125 && borg_skill[BI_SPEED] <= 129)
            value += (((borg_skill[BI_SPEED] - 110) * 1500L) + 135000L);

        if (borg_skill[BI_SPEED] >= 120 && borg_skill[BI_SPEED] <= 124)
            value += (((borg_skill[BI_SPEED] - 110) * 1500L) + 110000L);

        if (borg_skill[BI_SPEED] >= 115 && borg_skill[BI_SPEED] <= 119)
            value += (((borg_skill[BI_SPEED] - 110) * 1500L) +  85000L);

        if (borg_skill[BI_SPEED] >= 110 && borg_skill[BI_SPEED] <= 114)
            value += (((borg_skill[BI_SPEED] - 110) * 1500L) +  65000L);
        else
            value += (((borg_skill[BI_SPEED] -110) *2500L));
    }
    else
    {
        if (borg_skill[BI_SPEED] >= 140)
            value += (((borg_skill[BI_SPEED] - 120) * 1000L) + 175000L);

        if (borg_skill[BI_SPEED] >= 135 && borg_skill[BI_SPEED] <= 139)
            value += (((borg_skill[BI_SPEED] - 120) * 1000L) + 165000L);

        if (borg_skill[BI_SPEED] >= 130 && borg_skill[BI_SPEED] <= 134)
            value += (((borg_skill[BI_SPEED] - 120) * 1000L) + 150000L);

        if (borg_skill[BI_SPEED] >= 125 && borg_skill[BI_SPEED] <= 129)
            value += (((borg_skill[BI_SPEED] - 110) * 1000L) + 125000L);

        if (borg_skill[BI_SPEED] >= 120 && borg_skill[BI_SPEED] <= 124)
            value += (((borg_skill[BI_SPEED] - 110) * 1000L) + 100000L);

        if (borg_skill[BI_SPEED] >= 115 && borg_skill[BI_SPEED] <= 119)
            value += (((borg_skill[BI_SPEED] - 110) * 1000L) +  75000L);

        if (borg_skill[BI_SPEED] >= 110 && borg_skill[BI_SPEED] <= 114)
            value += (((borg_skill[BI_SPEED] - 110) * 1000L) +  55000L);
        else
            value += (((borg_skill[BI_SPEED] -110) *2500L));
    }


    /* Hack -- Reward strength bonus */
    value += (my_stat_ind[A_STR] * 100L);

    /* Hack -- Reward intelligence bonus */
    if ((p_ptr->class->spell_book == TV_MAGIC_BOOK) &&
        (my_stat_ind[A_INT] <= 37 ))
    {
        value += (my_stat_ind[A_INT] * 500L);

        /* Bonus for sp. */
        if (borg_worships_mana)
        {
            value += ((adj_mag_mana[my_stat_ind[A_INT]] * borg_skill[BI_CLEVEL]) / 2)  * 255L;
        }
        else
        {
            value += ((adj_mag_mana[my_stat_ind[A_INT]] * borg_skill[BI_CLEVEL]) / 2)  * 155L;
        }

        /* bonus for fail rate */
        value += adj_mag_stat[my_stat_ind[A_INT]] * 5010L;

        /* mage should try to get min fail to 0 */
        if (player_has(PF_ZERO_FAIL))
        {
            /* other fail rates */
            if (adj_mag_fail[my_stat_ind[A_INT]] < 1)
                value += 90000L;
        }
    }

    /* Hack -- Reward wisdom bonus */
    if ((p_ptr->class->spell_book == TV_PRAYER_BOOK) &&
        (my_stat_ind[A_WIS] <= 37 ))
    {
        value += (my_stat_ind[A_WIS] * 200L);

        /* Bonus for sp. */
        value += ((adj_mag_mana[my_stat_ind[A_WIS]] * borg_skill[BI_CLEVEL]) / 2)  * 150L;

        /* bonus for fail rate */
        value += adj_mag_stat[my_stat_ind[A_WIS]] * 3000L;

        /* priest should try to get min fail to 0 */
        if (player_has(PF_ZERO_FAIL))
        {
            /* Bonus for priests to in order to keep Holy Word fail rate down */
            if (borg_prayer_legal(3, 5)) value += my_stat_ind[A_WIS] * 35000L;

            if (adj_mag_fail[my_stat_ind[A_WIS]] < 1)
                value += 70000L;
        }

    }


    /* Dexterity Bonus --good for attacking and ac*/
    if (my_stat_ind[A_DEX] <= 37 )
    {
        /* Hack -- Reward bonus */
        value += (my_stat_ind[A_DEX] * 120L);
    }

    /* Constitution Bonus */
    if (my_stat_ind[A_CON] <= 37 )
    {
        int bonus_hp = p_ptr->player_hp[p_ptr->lev-1] + adj_con_mhp[my_stat_ind[A_CON]] * borg_skill[BI_CLEVEL] / 100;

        if (borg_worships_hp)
        {
            value += (my_stat_ind[A_CON] * 250L);
            /* Hack -- Reward hp bonus */
            /*         This is a bit wierd because we are not really giving a bonus for */
            /*         what hp you have, but for the 'bonus' hp you get */
            /*         getting over 800hp is very important. */
            if (bonus_hp < 800)
                value += bonus_hp * 450L;
            else
                value += (bonus_hp-800) * 100L + (350L * 500);
        }
        else /*does not worship hp */
        {
            value += (my_stat_ind[A_CON] * 150L);
            /* Hack -- Reward hp bonus */
            /*         This is a bit wierd because we are not really giving a bonus for */
            /*         what hp you have, but for the 'bonus' hp you get */
            /*         getting over 500hp is very important. */
            if (bonus_hp < 500)
                value += bonus_hp * 350L;
            else
                value += (bonus_hp-500) * 100L + (350L * 500);
        }

    }


    /* Hack -- Reward charisma bonus up to level 25 */
    if (borg_skill[BI_CLEVEL] < 25)
        value += (my_stat_ind[A_CHR] * 2L);



    /* HACK - a small bonus for adding to stats even above max. */
    /*        This will allow us to swap a ring of int +6 for */
    /*        our ring of int +2 even though we are at max int because */
    /*        we are wielding a weapon that has +4 int */
    /*        later it might be nice to swap to a weapon that does not */
    /*        have an int bonus */
    for (i = 0; i < 6; i++) value += my_stat_add[i];


    /*** Reward current skills ***/

    /* Hack -- tiny rewards */
    value += (borg_skill[BI_DIS] * 2L);
    value += (borg_skill[BI_DEV] * 25L);
    value += (borg_skill[BI_SAV] * 25L);
    /* perfect saves are very nice */
    if (borg_skill[BI_SAV] > 99)
        value += 10000;
    value += (borg_skill[BI_STL] * 2L);
    value += (borg_skill[BI_SRCH] * 1L);
    value += (borg_skill[BI_SRCHFREQ] * 1L);
    value += (borg_skill[BI_THN] * 5L);
    value += (borg_skill[BI_THB] * 35L);
    value += (borg_skill[BI_THT] * 2L);
    value += (borg_skill[BI_DIG] * 2L);


    /*** Reward current flags ***/

    /* Various flags */
    if (borg_skill[BI_SDIG]) value  += 750L;
    if (borg_skill[BI_SDIG] && borg_skill[BI_ISHUNGRY]) value  += 7500L;
    if (borg_skill[BI_SDIG] && borg_skill[BI_ISWEAK]) value  += 7500L;

    /* Feather Fall if low level is nice */
    if (borg_skill[BI_MAXDEPTH] < 20)
    {
        if (borg_skill[BI_FEATH]) value    += 500L;
    }
    else
    {
        if (borg_skill[BI_FEATH]) value     +=50;
    }

    if (borg_skill[BI_LIGHT]) value         += 2000L;

    if (borg_skill[BI_ESP])
    {
        if (borg_skill[BI_SINV]) value      += 500L;
    }

    if (!borg_skill[BI_DINV])
    {
		if (borg_skill[BI_SINV]) value      += 5000L;
	}

    if (borg_skill[BI_FRACT]) value     += 10000L;

    /* after you max out you are pretty safe from drainers.*/
    if (borg_skill[BI_MAXCLEVEL] < 50)
    {
        if (borg_skill[BI_HLIFE]) value    += 2000L;
    }
    else
    {
        if (borg_skill[BI_HLIFE]) value    += 200L;
    }
    if (borg_skill[BI_REG]) value   += 2000L;
    if (borg_skill[BI_ESP]) value    += 80000L;

    /* Immunity flags */
    if (borg_skill[BI_ICOLD]) value  += 65000L;
    if (borg_skill[BI_IELEC]) value  += 40000L;
    if (borg_skill[BI_IFIRE]) value  += 80000L;
    if (borg_skill[BI_IACID]) value  += 50000L;

    /* Resistance flags */
    if (borg_skill[BI_RCOLD]) value  += 3000L;
    if (borg_skill[BI_RELEC]) value  += 4000L;
    if (borg_skill[BI_RACID]) value  += 6000L;
    if (borg_skill[BI_RFIRE]) value  += 8000L;
    /* extra bonus for getting all basic resist */
    if (borg_skill[BI_RFIRE] &&
        borg_skill[BI_RACID] &&
        borg_skill[BI_RELEC] &&
        borg_skill[BI_RCOLD]) value +=  10000L;
    if (borg_skill[BI_RPOIS]) value  += 20000L;
    if (borg_skill[BI_RSND]) value += 3500L;
    if (borg_skill[BI_RLITE]) value  += 800L;
    if (borg_skill[BI_RDARK]) value  += 800L;
    if (borg_skill[BI_RKAOS]) value += 5000L;

    /* this is way boosted to avoid carrying stuff you don't need */
    if (borg_skill[BI_RCONF]) value  += 80000L;

    /* mages need a slight boost for this */
    if (borg_class == CLASS_MAGE && borg_skill[BI_RCONF]) value +=2000L;
    if (borg_skill[BI_RDIS]) value += 5000L;
    if (borg_skill[BI_RSHRD]) value += 100L;
    if (borg_skill[BI_RNXUS]) value += 100L;
    if (borg_skill[BI_RBLIND]) value += 5000L;
    if (borg_skill[BI_RNTHR]) value  += 5500L;
    if (borg_skill[BI_RFEAR]) value  += 2000L;

    /* Sustain flags */
    if (borg_skill[BI_SSTR]) value += 50L;
    if (borg_skill[BI_SINT]) value += 50L;
    if (borg_skill[BI_SWIS]) value += 50L;
    if (borg_skill[BI_SCON]) value += 50L;
    if (borg_skill[BI_SDEX]) value += 50L;
    /* boost for getting them all */
    if (borg_skill[BI_SSTR] &&
        borg_skill[BI_SINT] &&
        borg_skill[BI_SWIS] &&
        borg_skill[BI_SDEX] &&
        borg_skill[BI_SCON])  value += 1000L;


    /*** XXX XXX XXX Reward "necessary" flags ***/

    /* Mega-Hack -- See invisible (level 10) */
    if ((borg_skill[BI_SINV] || borg_skill[BI_ESP]) && (borg_skill[BI_MAXDEPTH]+1 >= 10)) value += 100000L;


    /* Mega-Hack -- Free action (level 20) */
    if (borg_skill[BI_FRACT] && (borg_skill[BI_MAXDEPTH]+1 >= 20)) value += 100000L;


    /*  Mega-Hack -- resists (level 25) */
    if (borg_skill[BI_RFIRE] && (borg_skill[BI_MAXDEPTH]+1 >= 25)) value += 100000L;


    /*  Mega-Hack -- resists (level 40) */
    if (borg_skill[BI_RPOIS] && (borg_skill[BI_MAXDEPTH]+1 >= 40)) value += 100000L;
    if (borg_skill[BI_RELEC] && (borg_skill[BI_MAXDEPTH]+1 >= 40)) value += 100000L;
    if (borg_skill[BI_RACID] && (borg_skill[BI_MAXDEPTH]+1 >= 40)) value += 100000L;
    if (borg_skill[BI_RCOLD] && (borg_skill[BI_MAXDEPTH]+1 >= 40)) value += 100000L;


    /*  Mega-Hack -- Speed / Hold Life (level 46) and maxed out */
    if ((borg_skill[BI_HLIFE] && (borg_skill[BI_MAXDEPTH]+1 >= 46) && (borg_skill[BI_MAXCLEVEL] < 50))) value += 100000L;
    if ((borg_skill[BI_SPEED] >= 115) && (borg_skill[BI_MAXDEPTH]+1 >=46)) value +=100000L;
    if (borg_skill[BI_RCONF] && (borg_skill[BI_MAXDEPTH]+1 >= 46)) value += 100000L;

    /*  Mega-Hack -- resist Nether is -very- nice to have at level 50 */
    if (borg_skill[BI_RNTHR]  && (borg_skill[BI_MAXDEPTH]+1 >= 50)) value += 55000L;

    /*  Mega-Hack -- resist Sound to avoid being KO'd */
    if (borg_skill[BI_RSND]  && (borg_skill[BI_MAXDEPTH]+1 >= 50)) value += 100000L;

    /*  Mega-Hack -- resists & Telepathy (level 55) */
    if (borg_skill[BI_RBLIND] && (borg_skill[BI_MAXDEPTH]+1 >= 55)) value += 100000L;
    if (borg_skill[BI_ESP] && (borg_skill[BI_MAXDEPTH]+1 >= 55)) value += 100000L;
    if (borg_skill[BI_RNTHR]  && (borg_skill[BI_MAXDEPTH]+1 >= 60)) value += 55000L;


    /*  Mega-Hack -- resists & +10 speed (level 60) */
    if (borg_skill[BI_RKAOS] && (borg_skill[BI_MAXDEPTH]+1 >= 60)) value += 104000L;
    if (borg_skill[BI_RDIS] && (borg_skill[BI_MAXDEPTH]+1 >= 60)) value += 90000L;
    if ((borg_skill[BI_SPEED] >= 120) && (borg_skill[BI_MAXDEPTH]+1 >=60)) value +=100000L;

    /*  Must have +20 speed (level 80) */
    if ((borg_skill[BI_SPEED] >= 130) && (borg_skill[BI_MAXDEPTH]+1 >=80)) value +=100000L;

    /* Not Req, but a good idea:
     * Extra boost to Nether deeper down
     * RDark for deeper uniques
     * Good to have +30 speed
     */
    if (borg_skill[BI_RNTHR] && (borg_skill[BI_MAXDEPTH]+1 >= 80)) value += 15000L;
    if (borg_skill[BI_RDARK] && (borg_skill[BI_MAXDEPTH]+1 >= 80)) value += 25000L;
    if ((borg_skill[BI_SPEED] >= 140) && (borg_skill[BI_MAXDEPTH]+1 >=80) &&
        borg_class == CLASS_WARRIOR)                value += 100000L;


    /*** Reward powerful armor ***/

    /* Reward armor */
    if (borg_worships_ac)
    {
        if (borg_skill[BI_ARMOR] < 15) value += ((borg_skill[BI_ARMOR]) * 2500L);
        if (borg_skill[BI_ARMOR] >= 15 && borg_skill[BI_ARMOR] < 75) value += ((borg_skill[BI_ARMOR]) * 2000L) + 28250L;
        if (borg_skill[BI_ARMOR] >= 75) value += ((borg_skill[BI_ARMOR]) * 1500L) + 73750L;
    }
    else
    {
        if (borg_skill[BI_ARMOR] < 15)   value += ((borg_skill[BI_ARMOR]) * 2000L);
        if (borg_skill[BI_ARMOR] >= 15 &&
            borg_skill[BI_ARMOR] < 75)   value += ((borg_skill[BI_ARMOR]) * 500L) + 28350L;
        if (borg_skill[BI_ARMOR] >= 75) value += ((borg_skill[BI_ARMOR]) * 100L) + 73750L;
    }


	/* Hack-- Reward the borg for carrying a NON*ID* item
	 * this way he will pull it from the house and *ID* it.
	 */
	if (amt_ego ||
	    ((e_info[borg_items[INVEN_OUTER].name2].xtra == OBJECT_XTRA_TYPE_RESIST ||
	     e_info[borg_items[INVEN_OUTER].name2].xtra == OBJECT_XTRA_TYPE_POWER)) &&
	     !borg_items[INVEN_OUTER].fully_identified) value += 999999L;

    /*** Penalize various things ***/

    /* Penalize various flags */
    if (borg_skill[BI_CRSTELE]) value -= 100000L;
    if (borg_class == CLASS_MAGE && borg_skill[BI_CRSAGRV]) value -= 800000L;
    else if (borg_skill[BI_CRSAGRV]) value -= 800000L;
	if (borg_skill[BI_CRSHPIMP]) value -=35000;
	if (borg_class != CLASS_WARRIOR && borg_skill[BI_CRSMPIMP]) value -=15000;
	if ((borg_class == CLASS_MAGE || borg_class == CLASS_PRIEST) &&
		borg_skill[BI_CRSMPIMP]) value -=15000;
    if (borg_skill[BI_CRSFEAR]) value -= 400000L;
    if (borg_skill[BI_CRSFEAR] && borg_class != CLASS_MAGE) value -= 200000L;
	if (borg_skill[BI_CRSFVULN]) value -=30000;
	if (borg_skill[BI_CRSEVULN]) value -=30000;
	if (borg_skill[BI_CRSCVULN]) value -=30000;
	if (borg_skill[BI_CRSAVULN]) value -=30000;


    /*** Penalize armor weight ***/
    if (my_stat_ind[A_STR] < 15)
    {
        if (borg_items[INVEN_BODY].weight > 200)
            value -= (borg_items[INVEN_BODY].weight - 200) * 15;
        if (borg_items[INVEN_HEAD].weight > 30)
            value -= 250;
        if (borg_items[INVEN_ARM].weight > 10)
            value -= 250;
        if (borg_items[INVEN_FEET].weight > 50)
            value -= 250;
    }

    /* Compute the total armor weight */
    cur_wgt += borg_items[INVEN_BODY].weight;
    cur_wgt += borg_items[INVEN_HEAD].weight;
    cur_wgt += borg_items[INVEN_ARM].weight;
    cur_wgt += borg_items[INVEN_OUTER].weight;
    cur_wgt += borg_items[INVEN_HANDS].weight;
    cur_wgt += borg_items[INVEN_FEET].weight;

    /* Determine the weight allowance */
    max_wgt = p_ptr->class->spell_weight;

    /* Hack -- heavy armor hurts magic */
    if (p_ptr->class->spell_book && ((cur_wgt - max_wgt) / 10) > 0)
    {
        /* Mega-Hack -- Penalize heavy armor which hurts mana */
        if (borg_skill[BI_MAXSP] >= 300 && borg_skill[BI_MAXSP] <= 350)
            value -= (((cur_wgt - max_wgt) / 10) * 400L);
        if (borg_skill[BI_MAXSP] >= 200 && borg_skill[BI_MAXSP] <= 299)
            value -= (((cur_wgt - max_wgt) / 10) * 800L);
        if (borg_skill[BI_MAXSP] >= 100 && borg_skill[BI_MAXSP] <= 199)
            value -= (((cur_wgt - max_wgt) / 10) * 1600L);
        if (borg_skill[BI_MAXSP] >= 1 && borg_skill[BI_MAXSP] <= 99)
            value -= (((cur_wgt - max_wgt) / 10) * 3200L);
    }


    /*** Penalize bad magic ***/

    /* Hack -- most gloves hurt magic for spell-casters */
    if (player_has(PF_CUMBER_GLOVE) && borg_class == CLASS_MAGE)
    {
        item = &borg_items[INVEN_HANDS];

        /* Penalize non-usable gloves */
        if (item->iqty &&
            (!of_has(item->flags, OF_FREE_ACT) &&
             !(of_has(item->flags, OF_DEX) && item->pval > 0)) &&
		     !(item->sval == SV_SET_OF_ALCHEMISTS_GLOVES))

		{
            /* Hack -- Major penalty */
            value -= 275000L;
        }
    }

    /*  Hack -- most edged weapons hurt magic for priests */
    if (player_has(PF_BLESS_WEAPON))
    {
        item = &borg_items[INVEN_WIELD];

        /* Penalize non-blessed edged weapons */
        if ((item->tval == TV_SWORD || item->tval == TV_POLEARM) &&
            !of_has(item->flags, OF_BLESSED))
        {
            /* Hack -- Major penalty */
            value -= 75000L;
        }
    }


#if 0 /* I wonder if this causes the borg to change his gear so radically at depth 99 */
	/* HUGE MEGA MONDO HACK! prepare for the big fight */
    /* go after Morgoth new priorities. */
    if ((borg_skill[BI_MAXDEPTH]+1 == 100 || borg_skill[BI_CDEPTH] == 100) && (!borg_skill[BI_KING]))
    {
        /* protect from stat drain */
        if (borg_skill[BI_SSTR]) value += 35000L;
        /* extra bonus for spell casters */
        if (p_ptr->class->spell_book == TV_MAGIC_BOOK && borg_skill[BI_SINT]) value += 45000L;
        /* extra bonus for spell casters */
        if (p_ptr->class->spell_book == TV_PRAYER_BOOK && borg_skill[BI_SWIS]) value += 35000L;
        if (borg_skill[BI_SCON]) value += 55000L;
        if (borg_skill[BI_SDEX]) value += 15000L;
        if (borg_skill[BI_WS_EVIL])  value += 15000L;

        /* Another bonus for resist nether, poison and base four */
        if (borg_skill[BI_RNTHR]) value +=  15000L;
        if (borg_skill[BI_RDIS]) value += 15000L;

        /* to protect against summoned baddies */
        if (borg_skill[BI_RPOIS]) value +=  100000L;
        if (borg_skill[BI_RFIRE] &&
            borg_skill[BI_RACID] &&
            borg_skill[BI_RELEC] &&
            borg_skill[BI_RCOLD]) value += 100000L;
    }
#endif

    /* Reward for activatable Artifacts in inventory */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
    {
        int multibonus = 0;
        item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Good to have one item with multiple high resists */
        multibonus = (of_has(item->flags, OF_RES_POIS) +
               of_has(item->flags, OF_RES_BLIND) +
               of_has(item->flags, OF_RES_CONFU) +
               of_has(item->flags, OF_RES_SOUND) +
               of_has(item->flags, OF_RES_SHARD) +
               of_has(item->flags, OF_RES_NEXUS) +
               of_has(item->flags, OF_RES_NETHR) +
               of_has(item->flags, OF_RES_CHAOS) +
               of_has(item->flags, OF_RES_DISEN) +
			   (of_has(item->flags, OF_RES_FIRE) &&
			    of_has(item->flags, OF_RES_COLD) &&
			    of_has(item->flags, OF_RES_ELEC) &&
			    of_has(item->flags, OF_RES_ACID)) +
			   (of_has(item->flags, OF_SUST_STR) &&
			    of_has(item->flags, OF_SUST_INT) &&
			    of_has(item->flags, OF_SUST_WIS) &&
			    of_has(item->flags, OF_SUST_DEX) &&
			    of_has(item->flags, OF_SUST_CON)));

        if (multibonus >= 2) value += 3000 * multibonus;

        /* This needs to be changed */
        switch (item->activation)
        {
        /* Artifact -- Narthanc- fire bolt 9d8*/
        /* Artifact -- Paurhach- fire bolt 9d8 */
        case EFF_FIRE_BOLT:
        value +=(500+(9*(8+1)/2));
        break;

        /* Artifact -- Nimthanc- frost bolt 6d8*/
        /* Artifact -- Paurnimmen- frost bolt 6d8 */
        case EFF_COLD_BOLT:
        value +=(500+(6*(8+1)/2));
        break;

        /* Artifact -- Dethanc- electric bolt 4d8*/
        /* Artifact -- Pauraegen- lightning bolt 4d8 */
        case EFF_ELEC_BOLT:
        value +=(500+(4*(8+1)/2));
        break;


        /* Artifact -- Rilia- poison gas 12*/
        case EFF_STINKING_CLOUD:
        value +=(500+(24));
        break;

        /* Artifact -- Belangil- frost ball 48*/
        case EFF_COLD_BALL50:
        value +=(500+(96));
        break;


        /* Artifact -- Arunruth- frost bolt 12d8*/
        case EFF_COLD_BOLT2:
        value +=(500+(12*(8+1)/2));
        break;


        /* Artifact -- Ringil- frost ball 100*/
        /* Artifact -- Aeglos- frost ball 100*/
        case EFF_COLD_BALL100:
        value +=(500+(200));
        /* extra boost for speed */
        if (!op_ptr->opt[OPT_birth_randarts] &&
            !op_ptr->opt[OPT_birth_randarts])
            value +=25000L;
        break;


        /* Artifact -- Anduril- fire ball 72*/
        /* Artifact -- Firestar- fire ball 72 */
        case EFF_FIRE_BALL:
        value +=(500+(144));
        break;

		/* Artifact -- NARYA- FIRE BALL 120 */
        case EFF_FIRE_BALL2:
        value +=(500+(240));
        break;

        /* Artifact -- Theoden- drain Life 120*/
        case EFF_DRAIN_LIFE2:
        value +=(500+120);
        break;


        /* Artifact -- Totila- confusion */
        case EFF_CONFUSE2:
        value +=(500+(200));
        break;

        /* Artifact -- TURMIL- drain life 90 */
        case EFF_DRAIN_LIFE1:
        value +=(500+90);
        break;


        /* Artifact -- Razorback- spikes 150 */
        /* Artifact -- FINGOLFIN- MISSILE 150 (bonus for TH TD)*/
        case EFF_ARROW:
        value +=(500+(300));
        break;


        /* Artifact -- Cammithrim- Magic Missile 2d6 */
        case EFF_MISSILE:
        value +=(500+(2*(6+1)/2));
        break;

        /* Artifact -- PaurNEN- ACID bolt 5d8 */
        case EFF_ACID_BOLT:
        value +=(500+(5*(8+1)/2));
        break;

        /* Artifact -- INGWE- DISPEL EVIL X5 */
        case EFF_DISPEL_EVIL:
        value +=(500+(10 + (borg_skill[BI_CLEVEL]*5)/2));
        break;


        /* Artifact -- NENYA- COLD BALL 200 */
        case EFF_COLD_BALL2:
        value +=(500+(400));
        break;

        /* Artifact -- VILYA- ELEC BALL 250 */
        case EFF_ELEC_BALL2:
        value +=(500+(500));
        break;

        /* Artifact -- POWER One Ring-*/
        case EFF_BIZARRE:
        value +=(999999);
        break;

        /* Artifact -- Ulmo- tele way */
        case EFF_TELE_OTHER:
        if (borg_class == CLASS_MAGE)
        {
            value +=500;
        }
        else
            value +=(500+(500));
        break;

        /* Artifact -- Colluin - bladturner Resistance */
        case EFF_RESIST_ALL:
        value +=(500+(150));
        /* extra bonus for the non spell guys */
        if (borg_class == CLASS_WARRIOR || borg_class == CLASS_ROGUE ||
            borg_class == CLASS_PALADIN) value +=25000;
        break;

        /* Artifact -- Holcolleth -- Sleep II */
        case EFF_SLEEPII:
        if ((borg_class == CLASS_MAGE) || (borg_class == CLASS_PRIEST) )
        {
            value +=500;
        }
        else
            value +=(500+(200));
        break;

        /* Artifact -- Thingol recharge */
        case EFF_RECHARGE:
        if (borg_class == CLASS_MAGE)
        {
            value +=500;
        }
        else
            value +=(500+(100));
        break;

        /* Artifact -- Holehenth detection */

        /* Artifact -- Dal fear and poison */
        case EFF_REM_FEAR_POIS:
        if (borg_class == CLASS_MAGE || borg_class == CLASS_PRIEST)
        {
            value +=500;
        }
        else
            value +=(500+(200));
        break;

        /* Artifact -- Carlammas PFE*/

        /* Artifact -- Lotharang- cure light */

        /* Artifact -- Eriril id */

        /* Artifact -- Cubragol brand bolts, bonus for speed */
        case EFF_FIREBRAND:
        value +=(500+(300));
        /* extra boost for speed */
        if (!op_ptr->opt[OPT_birth_randarts] &&
            !op_ptr->opt[OPT_birth_randarts])
            value +=25000L;
        break;

        /* Artifact -- Avavir WoR */

        /* Artifact -- Taratol, feanor, tulkas speed */

        /* Artifact -- Soulkeeper, Gondor heal */

        /* Artifact -- Belegonnon   phase */

        /* Artifact -- Colannon teleport */

        /* Artifact -- Luthien RLL */

        /* Artifact -- Phial */

        /* Artifact -- Star */
        case EFF_MAPPING:
        value +=(1200);
        break;

        /* Artifact -- Arkstone */
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
    int         k, carry_capacity, inven_weight, book;

    s32b        value = 0L;


    /*** Basic abilities ***/

    /*
     * In here, we must subtract out the bonus granted from certain
     * Artifacts.  They grant amt_x = 1000 then the power is increased
     * by 1000 times whatever bonus.  In the case of Gondor.  This is
     * 1000 heals times 4000 points per heal.
     *
     */

    /* Reward fuel */
    k = 0;
    for (; k < 5 && k < borg_skill[BI_AFUEL]; k++) value += 60000L;
    if (borg_skill[BI_STR] >= 15)
    {
		for (; k < 10 && k < borg_skill[BI_AFUEL]; k++) value += 6000L - (k * 100);
	}

    /* Reward Food */
    /* if hungry, food is THE top priority */
    if ((borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK]) && borg_skill[BI_FOOD]) value += 100000;
    k = 0;
    for (; k < 7 && k < borg_skill[BI_FOOD]; k++) value += 50000L;
    if (borg_skill[BI_STR] >= 15)
    {
	    for (; k < 10 && k < borg_skill[BI_FOOD]; k++) value += 200L;
	}
    if (borg_skill[BI_REG] && borg_skill[BI_CLEVEL] <=15)
    {
        k=0;
        for (; k < 15 && k < borg_skill[BI_FOOD]; k++) value += 700L;
    }
    /* Prefere to buy HiCalorie foods over LowCalorie */
    if (amt_food_hical <= 5) value += amt_food_hical * 50;


	/* Reward Cure Poison and Cuts*/
    if ((borg_skill[BI_ISCUT] || borg_skill[BI_ISPOISONED]) && borg_skill[BI_ACCW]) value +=100000;
    if ((borg_skill[BI_ISCUT] || borg_skill[BI_ISPOISONED]) && borg_skill[BI_AHEAL]) value +=50000;
    if ((borg_skill[BI_ISCUT] || borg_skill[BI_ISPOISONED]) && borg_skill[BI_ACSW])
    {   /* usually takes more than one */
        k = 0;
        for (; k < 5 && k < borg_skill[BI_ACSW]; k++) value += 25000L;
    }
    if (borg_skill[BI_ISPOISONED] && borg_skill[BI_ACUREPOIS]) value +=15000;
    if (borg_skill[BI_ISPOISONED] && amt_slow_poison) value +=5000;

	/* collect Resistance pots if not immune -- All Classes */
    if (!borg_skill[BI_IPOIS] && borg_skill[BI_ACUREPOIS] <= 20)
	{
		/* Not if munchkin starting */
		if (borg_munchkin_start && borg_skill[BI_MAXCLEVEL] < borg_munchkin_level)
		{
			/* Do not carry these until later */
		}
		else
		{
			for (; k <  4 && k < borg_skill[BI_ARESPOIS]; k++) value += 300L;
		}
	}

    /* Reward Resistance Potions for Warriors */
    if (borg_class == CLASS_WARRIOR && borg_skill[BI_MAXDEPTH] > 20)
    {
        k = 0;
		/* Not if munchkin starting */
		if (borg_munchkin_start && borg_skill[BI_MAXCLEVEL] < borg_munchkin_level)
		{
			/* Do not carry these until later */
		}
		else
		{
			/* collect pots if not immune */
			if (!borg_skill[BI_IFIRE])
			{
				for (; k <  4 && k < borg_skill[BI_ARESHEAT]; k++) value += 500L;
			}
			k = 0;
			/* collect pots if not immune */
			if (!borg_skill[BI_ICOLD])
			{
				for (; k <  4 && k < borg_skill[BI_ARESCOLD]; k++) value += 500L;
			}
			/* collect pots if not immune */
			if (!borg_skill[BI_IPOIS])
			{
				for (; k <  4 && k < borg_skill[BI_ARESPOIS]; k++) value += 500L;
			}
		}
    }

    /* Reward ident */
    k = 0;
	if (borg_skill[BI_CLEVEL] >= 10)
	{
		for (; k < 5 && k < borg_skill[BI_AID]; k++) value += 6000L;
		if (borg_skill[BI_STR] >= 15)
		{
			for (; k < 15 && k < borg_skill[BI_AID]; k++) value += 600L;
		}
	}
	/* Reward ID if I am carrying a {magical} or {excellent} item */
	if (my_need_id)
	{
		k = 0;
		for (; k < my_need_id && k < borg_skill[BI_AID]; k++) value += 6000L;
	}

    /*  Reward PFE  carry lots of these*/
    k = 0;
	/* Not if munchkin starting */
	if (borg_munchkin_start && borg_skill[BI_MAXCLEVEL] < borg_munchkin_level)
	{
		/* Do not carry these until later */
	}
	else
	{
		for (; k < 10 && k < borg_skill[BI_APFE]; k++) value += 10000L;
		for (; k < 25 && k < borg_skill[BI_APFE]; k++) value += 2000L;
	}
    /*  Reward Glyph- Rune of Protection-  carry lots of these*/
    k = 0;
    for (; k < 10 && k < borg_skill[BI_AGLYPH]; k++) value += 10000L;
    for (; k < 25 && k < borg_skill[BI_AGLYPH]; k++) value += 2000L;
    if (borg_skill[BI_MAXDEPTH] >= 100)
    {
        k = 0;
        for (; k < 10 && k < borg_skill[BI_AGLYPH]; k++) value += 2500L;
        for (; k < 75 && k < borg_skill[BI_AGLYPH]; k++) value += 2500L;
    }

	/* Reward Scroll of Mass Genocide, only when fighting Morgoth */
    if (borg_skill[BI_MAXDEPTH] >= 100)
    {
        k = 0;
        for (; k < 99 && k < borg_skill[BI_AMASSBAN]; k++) value += 2500L;
	}

    /* Reward recall */
    k = 0;
	if (borg_skill[BI_CLEVEL] > 7)
	{
		/* Not if munchkin starting */
		if (borg_munchkin_start && borg_skill[BI_MAXCLEVEL] < borg_munchkin_level)
		{
			/* Do not carry these until later */
		}
		else
		{
			for (; k < 3 && k < borg_skill[BI_RECALL]; k++) value += 50000L;
			if (borg_skill[BI_STR] >= 15)
			{
				for (; k < 7 && k < borg_skill[BI_RECALL]; k++) value += 5000L;
			}
			/* Deep borgs need the rod to avoid low mana traps */
			if (borg_skill[BI_MAXDEPTH] >= 50 && borg_has[ROD_RECALL]) value +=12000;
		}
	}

	/* Reward phase */
    k = 1;
    /* first phase door is very important */
    if (borg_skill[BI_APHASE]) value += 50000;
	/* Not if munchkin starting */
	if (borg_munchkin_start && borg_skill[BI_MAXCLEVEL] < borg_munchkin_level)
	{
		/* Do not carry these until later */
	}
	else
	{
		for (; k < 8 && k < borg_skill[BI_APHASE]; k++) value += 500L;
		if (borg_skill[BI_STR] >= 15)
		{
			for (; k < 15 && k < borg_skill[BI_APHASE]; k++) value += 500L;
		}
	}

    /* Reward escape (staff of teleport or artifact */
    k = 0;
	if (borg_munchkin_start && borg_skill[BI_MAXCLEVEL] < borg_munchkin_level)
	{
		/* Do not carry these until later */
	}
	else
	{
		for (; k < 2 && k < borg_skill[BI_AESCAPE]; k++) value += 10000L;
		if (borg_skill[BI_CDEPTH] > 70)
		{
			k = 0;
			for (; k < 3 && k < borg_skill[BI_AESCAPE]; k++) value += 10000L;
		}
	}

    /* Reward teleport scroll*/
    k = 0;
	if (borg_skill[BI_CLEVEL] >=3)
	{
		if (borg_skill[BI_ATELEPORT]) value += 10000L;
	}
	if (borg_skill[BI_CLEVEL] >= 7)
	{
		for (; k < 3 && k < borg_skill[BI_ATELEPORT]; k++) value += 10000L;
	}
	if (borg_skill[BI_CLEVEL] >=30)
	{
		for (; k < 10 && k < borg_skill[BI_ATELEPORT]; k++) value += 10000L;
	}

    /* Reward Teleport Level scrolls */
    k = 0;
	if (borg_skill[BI_CLEVEL] >= 15)
	{
		for (; k <  5 && k < borg_skill[BI_ATELEPORTLVL]; k++) value += 5000L;
	}


    /*** Healing ***/
    if (borg_class == CLASS_WARRIOR || borg_class == CLASS_ROGUE)
    {
        k = 0;
        for (; k < 15 && k < borg_skill[BI_AHEAL]; k++) value += 8000L;

        k = 0; /* carry a couple for emergency. Store the rest. */
		if (borg_skill[BI_MAXDEPTH] >= 46)
		{
			if (borg_scumming_pots)
			{
				for (; k < 1 && k < borg_skill[BI_AEZHEAL]; k++) value +=10000L;
			}
			else
			{
				for (; k < 2 && k < borg_skill[BI_AEZHEAL]; k++) value +=10000L;
			}
		}

        /* These guys need to carry the rods more, town runs low on supply. */
        k = 0;
        for (; k < 6 && k < borg_has[ROD_HEAL]; k++) value +=20000L;
    }
    else if (borg_class == CLASS_RANGER || borg_class == CLASS_PALADIN ||
        borg_class == CLASS_MAGE)
    {
        k = 0;
        for (; k < 10 && k < borg_skill[BI_AHEAL]; k++) value += 4000L;

        k = 0; /* carry a couple for emergency. Store the rest. */
		if (borg_skill[BI_MAXDEPTH] >= 46)
		{
			if (borg_scumming_pots)
			{
				for (; k < 1 && k < borg_skill[BI_AEZHEAL]; k++) value +=10000L;
			}
			else
			{
				for (; k < 2 && k < borg_skill[BI_AEZHEAL]; k++) value +=10000L;
			}
		}

        if (borg_class == CLASS_PALADIN)
        {
            /* Reward heal potions */
            k = 0;
            for (; k < 3 && k < borg_has[POTION_HEAL]; k++) value += 5000L;
        }

        /* These guys need to carry the rods more, town runs low on supply. */
        k = 0;
        for (; k < 4 && k < borg_has[ROD_HEAL]; k++) value +=20000L;

    }
    else if (borg_class == CLASS_PRIEST)
    {
        /* Level 1 priests are given a Potion of Healing.  It is better
         * for them to sell that potion and buy equipment or several
         * Cure Crits with it.
         */
        if (borg_skill[BI_CLEVEL] == 1)
        {
            k = 0;
            for (; k < 10 && k < borg_has[POTION_HEAL]; k++) value -= 2000L;
        }
        /* Reward heal potions */
        k = 0;
        for (; k < 5 && k < borg_has[POTION_HEAL]; k++) value += 2000L;

        k = 0; /* carry a couple for emergency. Store the rest. */
		if (borg_skill[BI_MAXDEPTH] >= 46)
		{
			if (borg_scumming_pots)
			{
				for (; k < 1 && k < borg_skill[BI_AEZHEAL]; k++) value +=10000L;
			}
			else
			{
				for (; k < 2 && k < borg_skill[BI_AEZHEAL]; k++) value +=10000L;
			}
		}
    }


    /* Collecting Potions, prepping for Morgoth/Sauron fight */
    if (borg_skill[BI_MAXDEPTH] >= 99)
    {
        /* Sauron is alive -- carry them all*/
        if (borg_race_death[546] == 0)
        {
            k = 0;
            for (; k < 99 && k < borg_has[POTION_HEAL]; k++) value += 8000L;
            k = 0;
            for (; k < 99 && k < borg_skill[BI_AEZHEAL]; k++) value +=10000L;
            k = 0;
            for (; k < 99 && k < borg_skill[BI_ASPEED]; k++) value += 8000L;
            k = 0;
            for (; k < 99 && k < borg_skill[BI_ALIFE]; k++) value +=10000L;
            k = 0;
            if (borg_class != CLASS_WARRIOR)
			{
				for (; k < 99 && k < borg_has[POTION_RES_MANA]; k++) value += 5000L;
			}
            k = 0;
            for (; k < 40 && k < borg_has[SHROOM_STONESKIN]; k++) value += 5000L;

			/* No need to store extras in home */
			borg_scumming_pots = FALSE;

        }
        /* Sauron is dead -- store them unless I have enough */
        if (borg_race_death[546] != 0)
        {
            /* Must scum for more pots */
            if ((num_heal_true + borg_has[POTION_HEAL] + num_ezheal_true + borg_skill[BI_AEZHEAL] < 30) ||
                (num_ezheal_true + borg_skill[BI_AEZHEAL] < 20) ||
                (num_speed + borg_skill[BI_ASPEED] < 15))
            {
                /* leave pots at home so they dont shatter */
				borg_scumming_pots = TRUE;
            }
            /* I have enough, carry all pots, and other good stuff. */
            else
            {
                k = 0;
                for (; k < 99 && k < borg_has[POTION_HEAL]; k++) value += 8000L;
                k = 0;
                for (; k < 99 && k < borg_skill[BI_AEZHEAL]; k++) value +=10000L;
				k = 0;
				for (; k < 99 && k < borg_skill[BI_ALIFE]; k++) value +=10000L;
                k = 0;
                for (; k < 99 && k < borg_skill[BI_ASPEED]; k++) value +=8000L;
				k = 0;
				for (; k < 40 && k < borg_has[SHROOM_STONESKIN]; k++) value += 5000L;
                k = 0;
				if (borg_class != CLASS_WARRIOR)
				{
					for (; k < 99 && k < borg_has[POTION_RES_MANA]; k++) value += 5000L;
				}
                /* Shrooms of Restoring */
                k = 0;
                for (; k < 35 && k < amt_fix_stat[6]; k++) value += 5000L;
				/* Reward Scroll of Mass Genocide, only when fighting Morgoth */
				k = 0;
				for (; k < 99 && k < borg_skill[BI_AMASSBAN]; k++) value += 2500L;

				/* No need to store extras in home */
				borg_scumming_pots = FALSE;
            }
        }
    }

    /* Restore Mana */
    if (borg_skill[BI_MAXSP] > 100)
    {
        k = 0;
        for (; k < 10 && k < borg_has[POTION_RES_MANA]; k++) value += 4000L;
        k = 0;
        for (; k < 100 && k < borg_skill[BI_ASTFMAGI]; k++) value += 4000L;
    }

    /* Reward Cure Critical.  Heavy reward on first 5 */
    if (borg_skill[BI_CLEVEL] < 35 && borg_skill[BI_CLEVEL] > 10)
    {
        k = 0;
        for (; k < 10 && k < borg_skill[BI_ACCW]; k++) value += 5000L;
    }
    else if (borg_skill[BI_CLEVEL] > 35)
    {
        /* Reward cure critical.  Later on in game. */
        k = 0;
        for (; k <  10 && k < borg_skill[BI_ACCW]; k++) value += 5000L;
		if (borg_skill[BI_STR] > 15)
		{
			for (; k < 15 && k < borg_skill[BI_ACCW]; k++) value += 500L;
		}
    }

    /* Reward cure serious -- only reward serious if low on crits */
    if (borg_skill[BI_ACCW] < 5 && borg_skill[BI_MAXCLEVEL] > 10 && (borg_skill[BI_CLEVEL] < 35 || !borg_skill[BI_RCONF]))
    {
        k = 0;
        for (; k <  7 && k < borg_skill[BI_ACSW]; k++) value += 50L;
		if (borg_skill[BI_STR] > 15)
		{
	        for (; k < 10 && k < borg_skill[BI_ACSW]; k++) value += 5L;
		}
    }

    /* Reward cure light -- Low Level Characters */
    if ((borg_skill[BI_ACCW] + borg_skill[BI_ACSW] < 5) && borg_skill[BI_CLEVEL] < 8)
    {
        k = 0;
        for (; k <  5 && k < borg_skill[BI_ACLW]; k++) value += 550L;
    }

	/* Reward Cures */
    if (!borg_skill[BI_RCONF])
    {
		if (borg_munchkin_start && borg_skill[BI_MAXCLEVEL] < 10)
		{
			/* Do not carry these until later */
		}
		else
		{
			k = 0;
			for (; k < 10 && k < amt_cure_confusion; k++) value += 400L;
		}
    }
    if (!borg_skill[BI_RBLIND])
    {
        k = 0;
		if (borg_munchkin_start && borg_skill[BI_MAXCLEVEL] < borg_munchkin_level)
		{
			/* Do not carry these until later */
		}
		else
		{
	        for (; k < 5 && k < amt_cure_blind; k++) value += 300L;
		}
    }
    if (!borg_skill[BI_RPOIS])
    {
        k = 0;
		if (borg_munchkin_start && borg_skill[BI_MAXCLEVEL] < borg_munchkin_level)
		{
			/* Do not carry these until later */
		}
		else
		{
			for (; k < 5 && k < borg_skill[BI_ACUREPOIS]; k++) value += 250L;
		}
    }
    /*** Detection ***/

    /* Reward detect trap */
    k = 0;
    for (; k < 1 && k < borg_skill[BI_ADETTRAP]; k++) value += 4000L;

    /* Reward detect door */
    k = 0;
    for (; k < 1 && k < borg_skill[BI_ADETDOOR]; k++) value += 2000L;

    /* Reward detect evil for non spell caster guys */
    if (!borg_skill[BI_ESP] && !borg_prayer_legal(0, 0) &&
        !borg_spell_legal(0, 1))
    {
        k = 0;
        for (; k < 1 && k < borg_skill[BI_ADETEVIL]; k++) value += 1000L;
    }

    /* Reward magic mapping */
    k = 0;
    for (; k < 1 && k < borg_skill[BI_AMAGICMAP]; k++) value += 4000L;

    /* Reward call lite */
    k = 0;
    for (; k < 1 && k < borg_skill[BI_ALITE]; k++) value += 1000L;

    /* Genocide scrolls. Just scrolls, mainly used for Morgoth */
    if (borg_skill[BI_MAXDEPTH] >= 100)
    {
        k = 0;
        for (; k < 10 && k < borg_has[SCROLL_GENOCIDE]; k++) value += 10000L;
        for (; k < 25 && k < borg_has[SCROLL_GENOCIDE]; k++) value += 2000L;
    }

    /* Reward speed potions/rods/staves (but no staves deeper than depth 95) */
    k = 0;
	if (borg_munchkin_start && borg_skill[BI_MAXCLEVEL] < borg_munchkin_level)
	{
		/* Do not carry these until later */
	}
	else
	{
        for (; k < 20 && k < borg_skill[BI_ASPEED]; k++) value += 5000L;
	}

    /* Reward Recharge ability */
	if (borg_skill[BI_ARECHARGE] && borg_skill[BI_MAXDEPTH] < 99) value += 5000L;

    /*** Missiles ***/

    /* Reward missiles */
    if (borg_class == CLASS_RANGER || borg_class == CLASS_WARRIOR)
    {
        k = 0;
        for (; k < 40 && k < borg_skill[BI_AMISSILES]; k++) value += 1000L;
		if (borg_skill[BI_STR] > 15 && borg_skill[BI_STR] < 18)
		{
	        for (; k < 80 && k < borg_skill[BI_AMISSILES]; k++) value += 100L;
		}
		if (borg_skill[BI_STR] > 18)
		{
	        for (; k < 180 && k < borg_skill[BI_AMISSILES]; k++) value += 80L;
		}
    }
    else
    {
        k = 0;
		for (; k < 20 && k < borg_skill[BI_AMISSILES] && k < 99; k++) value += 1000L;
		if (borg_skill[BI_STR] > 15)
		{
	        for (; k < 50 && k < borg_skill[BI_AMISSILES] && k < 99; k++) value += 100L;
		}
		/* Don't carry too many */
		if (borg_skill[BI_STR] <= 15 && borg_skill[BI_AMISSILES] > 20) value -= 1000L;
	}

	/* Potions of Detonations */
    k = 0;
    for (; k < 15 && k < borg_skill[BI_ADETONATE]; k++) value += 2000L;

    /*** Various ***/

    /*  -- Reward carrying a staff of destruction. */
    if (borg_skill[BI_ASTFDEST]) value += 5000L;
    k=0;
    for (; k < 9 && k < borg_skill[BI_ASTFDEST]; k++) value += 200L;

    /*  -- Reward carrying a wand of Teleport Other. */
    if (borg_skill[BI_ATPORTOTHER]) value += 5000L;
    /* Much greater reward for Warrior */
    if (borg_class == CLASS_WARRIOR && borg_skill[BI_ATPORTOTHER]) value += 50000L;
    /* reward per charge */
    k=0;
    for (; k < 15 && k < borg_skill[BI_ATPORTOTHER]; k++) value += 5000L;

    /*  -- Reward carrying an attack wand.
	 */
    if ((borg_has[WAND_MM] || borg_has[WAND_SCLOUD]) && borg_skill[BI_MAXDEPTH] < 30) value += 5000L;
    if (borg_has[WAND_ANNILATION] && borg_skill[BI_CDEPTH] < 30) value += 5000L;
    /* Much greater reward for Warrior or lower level  */
    if ((borg_class == CLASS_WARRIOR || borg_skill[BI_CLEVEL] <= 20) &&
		(borg_has[WAND_MM] || borg_has[WAND_ANNILATION] || borg_has[WAND_SCLOUD])) value += 10000L;
    /* reward per charge */
    value += amt_cool_wand * 50L;

    /* These staves are great but do not clutter inven with them */
    /*  -- Reward carrying a staff of holiness/power */
    if (amt_cool_staff) value += 2500L;
    k=0;
    for (; k < 3 && k < amt_cool_staff; k++) value += 500L;

	/* Rods of attacking are good too */
	k = 0;
	for (; k < 6 && k < borg_skill[BI_AROD1]; k++) value += 8000;
	k = 0;
	for (; k < 6 && k < borg_skill[BI_AROD2]; k++) value += 12000;

	/* Hack -- Reward add stat */
    if (amt_add_stat[A_STR]) value += 50000;
    if (amt_add_stat[A_INT]) value += 20000;
    if (p_ptr->class->spell_book == TV_MAGIC_BOOK)
        if (amt_add_stat[A_INT]) value += 50000;

    if (amt_add_stat[A_WIS]) value += 20000;
    if (p_ptr->class->spell_book == TV_PRAYER_BOOK)
        if (amt_add_stat[A_WIS]) value += 50000;
    if (amt_add_stat[A_DEX]) value += 50000;
    if (amt_add_stat[A_CON]) value += 50000;
    if (amt_add_stat[A_CHR]) value += 10000;

    /* Hack -- Reward stat potions */
    if (amt_inc_stat[A_STR] && my_stat_cur[A_STR] < (18+100)) value += 550000;
    if (amt_inc_stat[A_INT] && my_stat_cur[A_INT] < (18+100)) value += 520000;
    if (p_ptr->class->spell_book == TV_MAGIC_BOOK)
        if (amt_inc_stat[A_INT] && my_stat_cur[A_INT] < (18+100)) value += 575000;
    if (amt_inc_stat[A_WIS] && my_stat_cur[A_WIS] < (18+100)) value += 520000;
    if (p_ptr->class->spell_book == TV_PRAYER_BOOK)
        if (amt_inc_stat[A_WIS] && my_stat_cur[A_WIS] < (18+100)) value += 575000;
    if (amt_inc_stat[A_DEX] && my_stat_cur[A_DEX] < (18+100)) value += 550000;
    if (amt_inc_stat[A_CON] && my_stat_cur[A_CON] < (18+100)) value += 550000;
    if (amt_inc_stat[A_CHR] && my_stat_cur[A_CHR] < (18+100)) value += 510000;

    /* Hack -- Reward fix stat */
    if (amt_fix_stat[A_STR]) value += 10000;
    if (amt_fix_stat[A_INT]) value += 10000;
    if (amt_fix_stat[A_WIS]) value += 10000;
    if (amt_fix_stat[A_DEX]) value += 10000;
    if (amt_fix_stat[A_CON]) value += 10000;
    if (amt_fix_stat[A_CHR]) value += 10000;

    /* Reward Remove Curse */
    if (borg_wearing_cursed)
    {
        if (borg_has[SCROLL_STAR_CURSE]) value += 90000;
        if (borg_has[SCROLL_CURSE]) value += 90000;
    }

    /* Hack -- Restore experience */
    if (amt_fix_exp) value += 50000;

    /*** Enchantment ***/

    /* Reward enchant armor */
    if (amt_enchant_to_a < 1000 && my_need_enchant_to_a) value += 540L;

    /* Reward enchant weapon to hit */
    if (amt_enchant_to_h < 1000 && my_need_enchant_to_h) value += 540L;

    /* Reward enchant weapon to damage */
    if (amt_enchant_to_d < 1000 && my_need_enchant_to_d) value += 500L;

    /* Reward *enchant weapon* to damage */
    if (amt_enchant_weapon) value += 5000L;

    /* Reward *enchant armour*  */
    if (amt_enchant_armor) value += 5000L;

    /* Reward carrying a shovel if low level */
    if (borg_skill[BI_MAXDEPTH] <= 40 && borg_skill[BI_MAXDEPTH] >= 25 && borg_gold < 100000 && borg_items[INVEN_WIELD].tval != TV_DIGGING   &&
        amt_digger == 1) value += 5000L;


    /*** Hack -- books ***/

    /* Reward books */
    for (book = 0; book < 9; book++)
    {
        /* No copies */
        if (!amt_book[book]) continue;

        /* The "hard" books */
        if (book >= 4)
        {
            /* Reward the book */
            k = 0;
            for (; k < 1 && k < amt_book[book]; k++) value += 300000L;
        }

        /* The "easy" books */
        else
        {
            int what, when = 99;

            /* Scan the spells */
            for (what = 0; what < 9; what++)
            {
                borg_magic *as = &borg_magics[book][what];

                /* Track minimum level */
                if (as->level < when) when = as->level;
            }

            /* Hack -- Ignore "difficult" normal books */
            if ((when > 5) && (when >= borg_skill[BI_MAXCLEVEL] + 2)) continue;

            /* Reward the book */
            k = 0;
            for (; k < 1 && k < amt_book[book]; k++) value += 500000L;
            if (borg_skill[BI_STR] > 5)
                for (; k < 2 && k < amt_book[book]; k++) value += 10000L;
            if (borg_skill[BI_MAXDEPTH] > 50)
                for (; k < 3 && k < amt_book[book]; k++) value += 2500L;
        }
    }

    /*  Hack -- Apply "encumbrance" from weight */
    /* Extract the current weight (in tenth pounds) */
    inven_weight = p_ptr->total_weight;

    /* Extract the "weight limit" (in tenth pounds) */
    carry_capacity = adj_str_wgt[my_stat_ind[A_STR]] * 100;

    /* XXX XXX XXX Apply "encumbrance" from weight */
    if (inven_weight > carry_capacity/2)
   	{
		borg_item *item = &borg_items[INVEN_MAX_PACK-1];

		/* Some items will be used immediately and should not contribute to encumbrance */
		if (item->iqty &&
			((item->tval == TV_SCROLL &&
			 ((item->sval == SV_SCROLL_ENCHANT_ARMOR && amt_enchant_to_a < 1000 && my_need_enchant_to_a) ||
			  (item->sval == SV_SCROLL_ENCHANT_WEAPON_TO_HIT && amt_enchant_to_h < 1000 && my_need_enchant_to_h) ||
			  (item->sval == SV_SCROLL_ENCHANT_WEAPON_TO_DAM && amt_enchant_to_d < 1000 && my_need_enchant_to_d) ||
			   item->sval == SV_SCROLL_STAR_ENCHANT_WEAPON ||
			   item->sval == SV_SCROLL_STAR_ENCHANT_ARMOR)) ||
			(item->tval == TV_POTION &&
			 ((item->sval == SV_POTION_RES_STR && borg_skill[BI_ISFIXSTR]) ||
			  (item->sval == SV_POTION_RES_INT && borg_skill[BI_ISFIXINT]) ||
			  (item->sval == SV_POTION_RES_WIS && borg_skill[BI_ISFIXWIS]) ||
			  (item->sval == SV_POTION_RES_DEX && borg_skill[BI_ISFIXDEX]) ||
			  (item->sval == SV_POTION_RES_CON && borg_skill[BI_ISFIXCON]) ||
			  (item->sval == SV_POTION_RES_CHR && borg_skill[BI_ISFIXCHR]) ||
			  item->sval == SV_POTION_INC_STR ||
			  item->sval == SV_POTION_INC_INT ||
			  item->sval == SV_POTION_INC_WIS ||
			  item->sval == SV_POTION_INC_DEX ||
			  item->sval == SV_POTION_INC_CON ||
			  item->sval == SV_POTION_INC_CHR ||
			  item->sval == SV_POTION_INC_ALL))))
		{
			/* No encumbrance penalty for purchasing these items */
		}
		else
		{
			value -= ((inven_weight - (carry_capacity/2)) / (carry_capacity / 10) *1000L);
		}
	}

    /* Reward empty slots (up to 5) */
    k = 1;
        for (; k < 6; k++)
            if (!borg_items[INVEN_MAX_PACK-k].iqty)
                value += 40L;

    /* Return the value */
    return (value);
}


/*
 * Calculate the "power" of the Borg
 */
s32b borg_power(void)
{
    int i = 1;
    s32b value = 0L;

    /* Process the equipment */
    value += borg_power_aux1();

    /* Process the inventory */
    value += borg_power_aux2();

    /* Add a bonus for deep level prep */
    /* Dump prep codes */
   /* Scan from surface to deep , stop when not preped */
    for (i = 1; i <= borg_skill[BI_MAXDEPTH]+50; i++)
    {
        /* Dump fear code*/
        if ((cptr)NULL != borg_prepared(i)) break;
    }
   	value +=((i-1) * 40000L);

    /* Add the value for the swap items */
    value += weapon_swap_value;
    value += armour_swap_value;


    /* Return the value */
    return (value);
}

/*
 * Helper function -- calculate power of equipment in the home
 */
static s32b borg_power_home_aux1(void)
{
    s32b        value = 0L;

    /* This would be better seperated by item type (so 1 bonus for resist cold armor */
    /*   1 bonus for resist cold shield... but that would take a bunch more code. */

    /* try to collect at least 2 of each resist/power (for swapping) */
    /* This can be used to get rid of extra artifacts... */

    /* spare lite sources.  Artifacts only */
    if (num_LIGHT == 1)
        value += 150L;
    else
        if (num_LIGHT == 2)
            value += 170L;
        else
            if (num_LIGHT > 2)
                value += 170L + (num_LIGHT - 2) * 5L;

    if (num_slow_digest == 1)
        value += 50L;
    else
        if (num_slow_digest == 2)
            value += 70L;
        else
            if (num_slow_digest > 2)
                value += 70L + (num_slow_digest - 2) * 5L;

    if (num_regenerate == 1)
        value += 75L;
    else
        if (num_regenerate == 2)
            value += 100L;
        else
            if (num_regenerate > 2)
                value += 100L + (num_regenerate - 2) * 10L;

    if (num_telepathy == 1)
        value += 1000L;
    else
        if (num_telepathy == 2)
            value += 1500L;
        else
            if (num_telepathy > 2)
                value += 1500L + (num_telepathy - 2) * 10L;

    if (num_see_inv == 1)
        value += 800L;
    else
        if (num_see_inv == 2)
            value += 1200L;
        else
            if (num_see_inv > 2)
                value += 1200L + (num_see_inv - 2) * 10L;

    if (num_ffall == 1)
        value += 10L;
    else
        if (num_ffall == 2)
            value += 15L;
        else
            if (num_ffall > 2)
                value += 15L + (num_ffall - 2) * 1L;


    if (num_free_act == 1)
        value += 1000L;
    else
        if (num_free_act == 2)
            value += 1500L;
        else
            if (num_free_act > 2)
                value += 1500L + (num_free_act - 2) * 10L;

    if (num_hold_life == 1)
        value += 1000L;
    else
        if (num_hold_life == 2)
            value += 1500L;
        else
            if (num_hold_life > 2)
                value += 1500L + (num_hold_life - 2) * 10L;

    if (num_resist_acid == 1)
        value += 1000L;
    else
        if (num_resist_acid == 2)
            value += 1500L;
        else
            if (num_resist_acid > 2)
                value += 1500L + (num_resist_acid - 2) * 1L;
    if (num_immune_acid == 1)
        value += 3000L;
    else
        if (num_immune_acid == 2)
            value += 5000L;
        else
            if (num_immune_acid > 2)
                value += 5000L + (num_immune_acid - 2) * 30L;

    if (num_resist_elec == 1)
        value += 1000L;
    else
        if (num_resist_elec == 2)
            value += 1500L;
        else
            if (num_resist_elec > 2)
                value += 1500L + (num_resist_elec - 2) * 1L;
    if (num_immune_elec == 1)
        value += 3000L;
    else
        if (num_immune_elec == 2)
            value += 5000L;
        else
            if (num_immune_elec > 2)
                value += 5000L + (num_immune_elec - 2) * 30L;

    if (num_resist_fire == 1)
        value += 1000L;
    else
        if (num_resist_fire == 2)
            value += 1500L;
        else
            if (num_resist_fire > 2)
                value += 1500L + (num_resist_fire - 2) * 1L;
    if (num_immune_fire == 1)
        value += 3000L;
    else
        if (num_immune_fire == 2)
            value += 5000L;
        else
            if (num_immune_fire > 2)
                value += 5000L + (num_immune_fire - 2) * 30L;

    if (num_resist_cold == 1)
        value += 1000L;
    else
        if (num_resist_cold == 2)
            value += 1500L;
        else
            if (num_resist_cold > 2)
                value += 1500L + (num_resist_cold - 2) * 1L;
    if (num_immune_cold == 1)
        value += 3000L;
    else
        if (num_immune_cold == 2)
            value += 5000L;
        else
            if (num_immune_cold > 2)
                value += 5000L + (num_immune_cold - 2) * 30L;

    if (num_resist_pois == 1)
        value += 5000L;
    else
        if (num_resist_pois == 2)
             value += 9000L;
        else
            if (num_resist_pois > 2)
                value += 9000L + (num_resist_pois - 2) * 40L;

    if (num_resist_conf == 1)
        value += 2000L;
    else
        if (num_resist_conf == 2)
            value += 8000L;
        else
            if (num_resist_conf > 2)
                value += 8000L + (num_resist_conf - 2) * 45L;

    if (num_resist_sound == 1)
        value += 500L;
    else
        if (num_resist_sound == 2)
            value += 700L;
        else
            if (num_resist_sound > 2)
                value += 700L + (num_resist_sound - 2) * 30L;

    if (num_resist_LIGHT == 1)
        value += 100L;
    else
        if (num_resist_LIGHT == 2)
            value += 150L;
        else
            if (num_resist_LIGHT > 2)
                value += 150L + (num_resist_LIGHT - 2) * 1L;

    if (num_resist_dark == 1)
        value += 100L;
    else
        if (num_resist_dark == 2)
            value += 150L;
        else
            if (num_resist_dark > 2)
                value += 150L + (num_resist_dark - 2) * 1L;

    if (num_resist_chaos == 1)
        value += 1000L;
    else
        if (num_resist_chaos == 2)
            value += 1500L;
        else
            if (num_resist_chaos > 2)
                value += 1500L + (num_resist_chaos - 2) * 10L;

    if (num_resist_disen == 1)
        value += 5000L;
    else
        if (num_resist_disen == 2)
            value += 7000L;
        else
            if (num_resist_disen > 2)
                value += 7000L + (num_resist_disen - 2) * 35L;

    if (num_resist_shard == 1)
        value += 100L;
    else
        if (num_resist_shard == 2)
            value += 150L;
        else
            if (num_resist_shard > 2)
                value += 150L + (num_resist_shard - 2) * 1L;

    if (num_resist_nexus == 1)
        value += 200L;
    else
        if (num_resist_nexus == 2)
            value += 300L;
        else
            if (num_resist_nexus > 2)
                value += 300L + (num_resist_nexus - 2) * 2L;

    if (num_resist_blind == 1)
        value += 500L;
    else
        if (num_resist_blind == 2)
            value += 1000L;
        else
            if (num_resist_blind > 2)
                value += 1000L + (num_resist_blind - 2) * 5L;

    if (num_resist_neth == 1)
        value += 3000L;
    else
        if (num_resist_neth == 2)
            value += 4000L;
        else
            if (num_resist_neth > 2)
                value += 4000L + (num_resist_neth - 2) * 45L;

    /* stat gain items as well...(good to carry ring of dex +6 in */
    /*                            house even if I don't need it right now) */
    if (home_stat_add[A_STR] < 9)
        value += home_stat_add[A_STR] * 300L;
    else
        if (home_stat_add[A_STR] < 15)
            value += 9 * 300L + (home_stat_add[A_STR] - 9) * 200L;
        else
            value += 9 * 300L + 6 * 200L +
                      (home_stat_add[A_STR] - 15) * 1L;

    if (home_stat_add[A_DEX] < 9)
        value += home_stat_add[A_DEX] * 300L;
    else
        if (home_stat_add[A_DEX] < 15)
            value += 9 * 300L + (home_stat_add[A_DEX] - 9) * 200L;
        else
            value += 9 * 300L + 6 * 200L +
                      (home_stat_add[A_DEX] - 15) * 1L;

    /* HACK extra con for thorin and other such things */
    if (home_stat_add[A_CON] < 15)
        value += home_stat_add[A_CON] * 300L;
    else
        if (home_stat_add[A_CON] < 21)
            value += 15 * 300L + (home_stat_add[A_CON] - 15) * 200L;
        else
            value += 15 * 300L + 6 * 200L + (home_stat_add[A_CON] - 21) * 1L;

    /* int and wis are only bonused for spell casters. */
    if (p_ptr->class->spell_book == TV_MAGIC_BOOK)
    {
        if (home_stat_add[A_INT] < 20)
            value += home_stat_add[A_INT] * 400L;
        else
            if (home_stat_add[A_INT] < 26)
                value += 20 * 400L + (home_stat_add[A_INT] - 20) * 300L;
            else
                value += 20 * 100L + 6 * 300L +
                         (home_stat_add[A_INT] - 26) * 5L;
    }

    if (p_ptr->class->spell_book == TV_PRAYER_BOOK)
    {
        if (home_stat_add[A_WIS] < 20)
            value += home_stat_add[A_WIS] * 400L;
        else
            if (home_stat_add[A_WIS] < 26)
                value += 20 * 400L + (home_stat_add[A_WIS] - 20) * 300L;
            else
                value += 20 * 400L + 6 * 300L +
                        (home_stat_add[A_WIS] - 26) * 3L;
    }

    /* Sustains */
    if (num_sustain_str == 1)
        value += 200L;
    else
        if (num_sustain_str == 2)
            value += 250L;
        else
            if (num_sustain_str > 2)
                value += 250L + (num_sustain_str - 2) * 1L;

    if (num_sustain_int == 1)
        value += 200L;
    else
        if (num_sustain_int == 2)
            value += 250L;
        else
            if (num_sustain_int > 2)
                value += 250L + (num_sustain_int - 2) * 1L;

    if (num_sustain_wis == 1)
        value += 200L;
    else
        if (num_sustain_wis == 2)
            value += 250L;
        else
            if (num_sustain_wis > 2)
                value += 250L + (num_sustain_wis - 2) * 1L;

    if (num_sustain_con == 1)
        value += 200L;
    else
        if (num_sustain_con == 2)
            value += 250L;
        else
            if (num_sustain_con > 2)
                value += 250L + (num_sustain_con - 2) * 1L;

    if (num_sustain_dex == 1)
        value += 200L;
    else
        if (num_sustain_dex == 2)
            value += 250L;
        else
            if (num_sustain_dex > 2)
                value += 250L + (num_sustain_dex - 2) * 1L;

    if (num_sustain_all == 1)
        value += 1000L;
    else
        if (num_sustain_all == 2)
            value += 1500L;
        else
            if (num_sustain_all > 2)
                value += 1500L + (num_sustain_all - 2) * 1L;

    /* do a minus for too many duplicates.  This way we do not store */
    /* useless items and spread out types of items. */
    if (num_weapons > 5)
        value -= (num_weapons - 5) * 2000L;
    else
        if (num_weapons > 1)
            value -= (num_weapons - 1) * 100L;
    if (num_bow > 2)
        value -= (num_bow - 2) * 1000L;
    if (num_rings > 6)
        value -= (num_rings - 6) * 4000L;
    else
        if (num_rings > 4)
            value -= (num_rings - 4) * 2000L;
    if (num_neck > 3)
        value -= (num_neck - 3) * 1500L;
    else
        if (num_neck > 3)
            value -= (num_neck - 3) * 700L;
    if (num_armor > 6)
        value -= (num_armor - 6) * 1000L;
    if (num_cloaks > 3)
        value -= (num_cloaks - 3) * 1000L;
    if (num_shields > 3)
        value -= (num_shields - 3) * 1000L;
    if (num_hats > 4)
        value -= (num_hats - 4) * 1000L;
    if (num_gloves > 3)
        value -= (num_gloves - 3) * 1000L;
    if (num_boots > 3)
        value -= (num_boots - 3) * 1000L;


    value += home_damage;

    /* if edged and priest, dump it   */
    value -= num_edged_weapon * 3000L;

    /* if gloves and mage or ranger and not FA/Dex, dump it. */
    value -= num_bad_gloves * 3000L;

    /* do not allow duplication of items. */
    value -= num_duplicate_items * 50000L;


    /* Return the value */
    return (value);
}


/*
 * Helper function -- calculate power of items in the home
 *
 * The weird calculations help spread out the purchase order
 */
static s32b borg_power_home_aux2(void)
{
    int         k, book;

    s32b        value = 0L;


    /*** Basic abilities ***/

    /* Collect food */
	if (borg_skill[BI_MAXCLEVEL] < 10)
	{
		for (k = 0; k < 25 && k < num_food; k++) value += 8000L - k*10L;
	}
	else if (borg_skill[BI_MAXCLEVEL] < 35)
	{
		for (k = 0; k < 90 && k < num_food; k++) value += 8000L - k*10L;
	}
	else
	{
		for (k = 0; k < 90 && k < num_food; k++) value += 8000L - k*10L;
	}
	if (borg_skill[BI_MAXCLEVEL] > 35 && borg_class == CLASS_WARRIOR)
	{
		for (k = 0; k < 89 && k < num_food; k++) value += 8000L - k*2L;
	}
#if 0
    /* Collect fuel */
	if (borg_skill[BI_MAXCLEVEL] < 10)
	{
		for (k = 0; k < 25 && k < num_fuel; k++) value += 8000L - k*10L;
	}
	else if (borg_skill[BI_MAXCLEVEL] < 35)
	{
		for (k = 0; k < 50 && k < num_fuel; k++) value += 8000L - k*10L;
	}
	else
	{
		for (k = 0; k < 90 && k < num_fuel; k++) value += 8000L - k*10L;
	}

    /* Collect Molds as pets */
    for (k = 0; k < 10 && k < num_mold; k++) value += 10L - k;
#endif
    /* Collect ident */
    for (k = 0; k < 50 && k < num_ident; k++) value += 2000L - k*10L;

    /* Collect enchantments armour */
	if (borg_skill[BI_CLEVEL] < 45)
	{
    	for (k = 0; k < 90 && k < num_enchant_to_a; k++) value += 500L - k*10L;
	}
    /* Collect enchantments to hit */
	if (borg_skill[BI_CLEVEL] < 45)
    {
    	for (k = 0; k < 90 && k < num_enchant_to_h; k++) value += 500L - k*10L;
	}
    /* Collect enchantments to dam */
	if (borg_skill[BI_CLEVEL] < 45)
    {
		for (k = 0; k < 90 && k < num_enchant_to_d; k++) value += 500L - k*10L;
	}

    /* Collect pfe */
    for (k = 0; k < 90 && k < num_pfe; k++) value += 500L - k*10L;

    /* Collect glyphs */
    for (k = 0; k < 90 && k < num_glyph; k++) value += 500L - k*10L;

    /* Reward Genocide scrolls. Just scrolls, mainly used for Morgoth */
    for (k = 0; k < 90 && k < num_genocide; k++) value += 500L - k*10L;

    /* Reward Mass Genocide scrolls. Just scrolls, mainly used for Morgoth */
    for (k = 0; k < 90 && k < num_mass_genocide; k++) value += 500L;

    /* Collect Recharge ability */
    for (k = 0; k < 90 && k < num_recharge; k++) value += 500L - k*10L;

    /* Reward Resistance Potions for Warriors */
    if (borg_class == CLASS_WARRIOR && borg_skill[BI_MAXDEPTH] > 20 && borg_skill[BI_MAXDEPTH] < 80)
    {
        k = 0;
        for (; k <  90 && k < num_pot_rheat; k++) value += 100L - k*10L;
        for (; k <  90 && k < num_pot_rcold; k++) value +=  100L - k*10L;
    }

    /* Collect recall */
    for (k = 0; k < 90 && k < num_recall; k++) value += 1000L;

    /* Collect escape  (staff of teleport) */
	if (borg_skill[BI_MAXCLEVEL] < 40)
	{
	    for (k = 0; k < 85 && k < num_escape; k++) value += 2000L - k*10L;
	}

    /* Collect a maximal number of staves in the home */
    for (k = 0; k < 90 && k < num_tele_staves; k++) value -= 50000L;

    /* Collect teleport */
    for (k = 0; k < 85 && k < num_teleport; k++) value += 5000L;

    /* Collect phase */
	if (borg_skill[BI_MAXCLEVEL] < 10)
	{
	    for (k = 0; k < 90 && k < num_phase; k++) value += 5000L;
	}

    /* Collect teleport level scrolls*/
    /* for (k = 0; k < 85 && k < num_teleport_level; k++) value += 5000L - k*8L; */

    /* Collect Speed */
    /* for (k = 0; k < 85 && k < num_speed; k++) value += 5000L - k*10L; */

	/* Collect Potions of Detonations */
    for (k = 0; k < 85 && k < num_detonate; k++) value += 5000L;

    /* collect mana/ */
    if (borg_skill[BI_MAXSP] > 1)
    {
        for (k = 0; k < 90 && k < num_mana; k++) value += 6000L - k*8L;
    }

    /* Level 1 priests are given a Potion of Healing.  It is better
     * for them to sell that potion and buy equipment or several
     * Cure Crits with it.
     */
    if (borg_skill[BI_CLEVEL] == 1)
    {
        k = 0;
        for (; k < 10 && k < num_heal; k++) value -= 5000L;
    }

    /*** Healing ***/

    /* Collect cure critical */
    for (k = 0; k < 90 && k < num_cure_critical; k++) value += 1500L-k*10L;

    /* Collect heal, *Heal*, Life */
    for (k = 0; k < 90 && k < num_heal; k++) value += 3000L;
    for (k = 0; k < 198 && k < num_ezheal; k++) value += 8000L;
    for (k = 0; k < 198 && k < num_life; k++) value += 9000L;

	/* junk cure serious if we have some in the home */
    if (borg_skill[BI_CLEVEL] > 35)    /* dont bother keeping them if high level */
        for (k = 0; k < 90 && k < num_cure_serious; k++) value -= 1500L-k*10L;

    /*** Various ***/

    /* Fixing Stats */
    if (borg_skill[BI_CLEVEL] == 50 && num_fix_exp) value -= 7500L;
    if (borg_skill[BI_CLEVEL] > 35 && borg_skill[BI_CLEVEL] <= 49)
       for (k = 0; k < 70 && k < num_fix_exp; k++) value += 1000L - k*10L;
    else if (borg_skill[BI_CLEVEL] <= 35)
       for (k = 0; k < 5 && k < num_fix_exp; k++) value += 1000L - k*10L;

    /* Keep shrooms in the house */
#if 0
    for (k = 0; k < 90 && k < num_fix_stat[6]; k++) value += 5000L;
    for (k = 0; k < 90 && k < num_mush_second_sight; k++) value += 5000L;
    for (k = 0; k < 90 && k < num_mush_fast_recovery; k++) value += 5000L;
    for (k = 0; k < 90 && k < num_mush_cure_mind; k++) value += 5000L;
    for (k = 0; k < 90 && k < num_mush_emergency; k++) value += 5000L;
    /* for (k = 0; k < 90 && k < num_mush_terror; k++) value += 5000L; */
    for (k = 0; k < 90 && k < num_mush_stoneskin; k++) value += 5000L;
    for (k = 0; k < 90 && k < num_mush_debility; k++) value += 5000L;
    for (k = 0; k < 90 && k < num_mush_sprinting; k++) value += 5000L;
    /* for (k = 0; k < 90 && k < num_mush_purging; k++) value += 5000L; */
#endif

    /*** Hack -- books ***/

    /* Reward books */
    for (book = 0; book < 4; book++)
    {

        if (borg_skill[BI_CLEVEL] > 35)
            /* Collect up to 20 copies of each normal book */
            for (k = 0; k < 20 && k < num_book[book]; k++)
            {
                /* Hack -- only stockpile useful books */
                if (num_book[book]) value += 5000L - k*10L;
            }
        else
            /* Collect up to 5 copies of each normal book */
            for (k = 0; k < 5 && k < num_book[book]; k++)
            {
                /* Hack -- only stockpile useful books */
                if (num_book[book]) value += 5000L - k*10L;
            }
    }

    /* Reward artifacts in the home */
    value += num_artifact * 500L;

    /* Reward certain types of egos in the home */
    value += num_ego * 5000L;

    /* Return the value */
    return (value);
}


/*
 * Calculate the "power" of the home
 */
s32b borg_power_home(void)
{
    s32b value = 0L;

    /* Process the home equipment */
    value += borg_power_home_aux1();

    /* Process the home inventory */
    value += borg_power_home_aux2();

    /* Return the value */
    return (value);
}


/*
 * Calculate base danger from a monster's physical attacks
 *
 * We attempt to take account of various resistances, both in
 * terms of actual damage, and special effects, as appropriate.
 *
 * We reduce the danger from distant "sleeping" monsters.
 * PFE reduces my fear of an area.
 *
 */
static int borg_danger_aux1(int i, bool full_damage)
{
    int k, n = 0;
    int pfe = 0;
    int power, chance;

    s16b ac = borg_skill[BI_ARMOR];

    borg_kill *kill = &borg_kills[i];

    monster_race *r_ptr = &r_info[kill->r_idx];

    /* shields gives +50 to ac and deflects some missiles and balls*/
    if (borg_shield)
        ac += 50;

    /*  PFE gives a protection.  */
        /* Hack -- Apply "protection from evil" */
        if ( (borg_prot_from_evil) &&
            (rf_has(r_ptr->flags, RF_EVIL)) &&
            ((borg_skill[BI_CLEVEL]) >= r_ptr->level) )
        {
            pfe = 1;
        }


    /* Mega-Hack -- unknown monsters */
    if (kill->r_idx >= z_info->r_max) return (1000);

    /* Analyze each physical attack */
    for (k = 0; k < 4; k++)
    {
        int z = 0;

		/* Extract the attack infomation */
		int effect = r_ptr->blow[k].effect;
		int method = r_ptr->blow[k].method;
		int d_dice = r_ptr->blow[k].d_dice;
		int d_side = r_ptr->blow[k].d_side;

        power = 0;

        /* Done */
        if (!method) break;

        /* Analyze the attack */
        switch (effect)
        {
            case RBE_HURT:
            z = (d_dice * d_side);
            /* stun */
            if ((d_side < 3) && (z > d_dice * d_side))
            {
				n += 200;
            }
            /* fudge- only mystics kick and they tend to KO.  Avoid close */
            /* combat like the plauge */
            if (method == RBM_KICK)
            {
                n += 400;
			}
            power = 60;
            if ((pfe) && !borg_attacking)
            {
                z /= 2;
			}
            break;

            case RBE_POISON:
            z = (d_dice * d_side);
            power = 5;
            if (borg_skill[BI_RPOIS]) break;
            if (borg_skill[BI_TRPOIS]) break;
            /* Add fear for the effect */
                z += 10;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_UN_BONUS:
            z = (d_dice * d_side);
            power = 20;
            if (borg_skill[BI_RDIS]) break;
            /* Add fear for the effect */
                z += 500;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_UN_POWER:
            z = (d_dice * d_side);
            /* Add fear for the effect */
                z += 20;
            power = 15;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_EAT_GOLD:
            z = (d_dice * d_side);
            /* if in town and low level avoid them stupid urchins */
            if (borg_skill[BI_CLEVEL] < 5) z += 50;
            power = 5;
            if (100 <= adj_dex_safe[my_stat_ind[A_DEX]] + borg_skill[BI_CLEVEL]) break;
            if (borg_gold < 100) break;
            if (borg_gold > 100000) break;
            /* Add fear for the effect */
                z += 5;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_EAT_ITEM:
            z = (d_dice * d_side);
            power = 5;
            if (100 <= adj_dex_safe[my_stat_ind[A_DEX]] + borg_skill[BI_CLEVEL]) break;
            /* Add fear for the effect */
                z += 5;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_EAT_FOOD:
            z = (d_dice * d_side);
            power = 5;
            if (borg_skill[BI_FOOD] > 5) break;
            /* Add fear for the effect */
                z += 5;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_EAT_LIGHT:
            z = (d_dice * d_side);
            power = 5;
            if (borg_skill[BI_CURLITE] == 0) break;
            if (borg_skill[BI_AFUEL] > 5) break;
            /* Add fear for the effect */
                z += 5;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_ACID:
            if (borg_skill[BI_IACID]) break;
            z = (d_dice * d_side);
            if (borg_skill[BI_RACID]) z = (z + 2) / 3;
            if (borg_skill[BI_TRACID]) z = (z + 2) / 3;
            /* Add fear for the effect */
                z += 200; /* We dont want our armour corroded. */
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_ELEC:
            if (borg_skill[BI_IELEC]) break;
            z = (d_dice * d_side);
            power = 10;
            if (borg_skill[BI_RELEC]) z = (z + 2) / 3;
            if (borg_skill[BI_TRELEC]) z = (z + 2) / 3;
            /* Add fear for the effect */
                z = z * 2;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_FIRE:
            if (borg_skill[BI_IFIRE]) break;
            z = (d_dice * d_side);
            power = 10;
            if (borg_skill[BI_RFIRE]) z = (z + 2) / 3;
            if (borg_skill[BI_TRFIRE]) z = (z + 2) / 3;
            /* Add fear for the effect */
                z = z * 2;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_COLD:
            if (borg_skill[BI_ICOLD]) break;
            z = (d_dice * d_side);
            power = 10;
            if (borg_skill[BI_RCOLD]) z = (z + 2) / 3;
            if (borg_skill[BI_TRCOLD]) z = (z + 2) / 3;
            /* Add fear for the effect */
                z = z * 2;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_BLIND:
            z = (d_dice * d_side);
            power = 2;
            if (borg_skill[BI_RBLIND]) break;
            /* Add fear for the effect */
                z += 10;
			if (borg_class == CLASS_MAGE) z +=75;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_CONFUSE:
            z = (d_dice * d_side);
            power = 10;
            if (borg_skill[BI_RCONF]) break;
            /* Add fear for the effect */
                z += 200;
			if (borg_class == CLASS_MAGE) z +=200;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_TERRIFY:
            z = (d_dice * d_side);
            power = 10;
            if (borg_skill[BI_RFEAR]) break;
            /* Add fear for the effect */
                z = z * 2;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_PARALYZE:
            z = (d_dice * d_side);
            power = 2;
            if (borg_skill[BI_FRACT]) break;
            z += 200;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_LOSE_STR:
            z = (d_dice * d_side);
            if (borg_skill[BI_SSTR]) break;
            if (borg_stat[A_STR] <= 3) break;
            if (borg_prayer_legal(6, 3)) break;
            z += 150;
            /* extra scary to have str drain below 10 */
            if (borg_stat[A_STR] < 10)
                z += 100;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_LOSE_DEX:
            z = (d_dice * d_side);
            if (borg_skill[BI_SDEX]) break;
            if (borg_stat[A_DEX] <= 3) break;
            if (borg_prayer_legal(6, 3)) break;
            z += 150;
            /* extra scary to have drain below 10 */
            if (borg_stat[A_DEX] < 10)
                z += 100;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_LOSE_CON:
            z = (d_dice * d_side);
            if (borg_skill[BI_SCON]) break;
            if (borg_stat[A_CON] <= 3) break;
            if (borg_prayer_legal(6, 3)) break;
            /* Add fear for the effect */
            z += 150;
            /* extra scary to have con drain below 8 */
            if (borg_stat[A_STR] < 8)
                z += 100;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_LOSE_INT:
            z = (d_dice * d_side);
            if (borg_skill[BI_SINT]) break;
            if (borg_stat[A_INT] <= 3) break;
            if (borg_prayer_legal(6, 3)) break;
            z += 150;
            /* extra scary for spell caster */
            if (p_ptr->class->spell_book == TV_MAGIC_BOOK)
                z += 50;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_LOSE_WIS:
            z = (d_dice * d_side);
            if (borg_skill[BI_SWIS]) break;
            if (borg_stat[A_WIS] <= 3) break;
            if (borg_prayer_legal(6, 3)) break;
            z += 150;
            /* extra scary for pray'er */
            if (p_ptr->class->spell_book == TV_PRAYER_BOOK)
                z += 50;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_LOSE_CHR:
            z = (d_dice * d_side);
            if (borg_skill[BI_SCHR]) break;
            if (borg_stat[A_CHR] <= 3) break;
            if (borg_prayer_legal(6, 3)) break;
            z += 5;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_LOSE_ALL:
            z = (d_dice * d_side);
            power = 2;
            /* only morgoth. HACK to make it easier to fight him */
            break;

            case RBE_SHATTER:
            z = (d_dice * d_side);
            z -= (z * ((ac < 150) ? ac : 150) / 250);
            power = 60;
            /* Add fear for the effect */
                z += 150;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_EXP_10:
            z = (d_dice * d_side);
            if (borg_skill[BI_HLIFE]) break;
            /* do not worry about drain exp after level 50 */
            if (borg_skill[BI_CLEVEL] == 50) break;
            if (borg_prayer_legal(6, 4)) break;
            /* Add fear for the effect */
                z += 100;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_EXP_20:
            z = (d_dice * d_side);
            if (borg_skill[BI_HLIFE]) break;
            /* do not worry about drain exp after level 50 */
            if (borg_skill[BI_CLEVEL] >= 50) break;
            if (borg_prayer_legal(6, 4)) break;
            /* Add fear for the effect */
                z += 150;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_EXP_40:
            z = (d_dice * d_side);
            if (borg_skill[BI_HLIFE]) break;
            /* do not worry about drain exp after level 50 */
            if (borg_skill[BI_CLEVEL] >= 50) break;
            if (borg_prayer_legal(6, 4)) break;
            /* Add fear for the effect */
                z += 200;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

            case RBE_EXP_80:
            z = (d_dice * d_side);
            if (borg_skill[BI_HLIFE]) break;
            /* do not worry about drain exp after level 50 */
            if (borg_skill[BI_CLEVEL] >= 50) break;
            if (borg_prayer_legal(6, 4)) break;
            /* Add fear for the effect */
                z += 250;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

			case RBE_HALLU:
            z = (d_dice * d_side);
            /* Add fear for the effect */
                z += 250;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;
        }

        /* if we are doing partial damage reduce for % chance that it will */
        /* hit you. */
        if (!full_damage)
        {
            /* figure out chance that monster will hit you. */
            /* add a 50% bonus in to account for bad luck. */
            if (borg_fighting_unique || (r_ptr->level + power) > 0)
                chance  = 150 - (((ac * 300) / 4) / 1+((r_ptr->level + power) * 3));
            else
                chance = -1;

            /* always have a 5% chance of hitting. */
            if (chance < 5) chance = 5;

			/* Modify the damage by the chance of getting hit */
            z = (z * chance) / 100;
        }

        /* Add in damage */
        n += z;
    }

    /* Danger */
    return (n);
}


/*
 * Calculate base danger from a monster's spell attacks
 *
 * We attempt to take account of various resistances, both in
 * terms of actual damage, and special effects, as appropriate.
 *
 * We reduce the danger from distant "sleeping" monsters.
 *
 * We reduce the danger if the monster is immobile or not LOS
 */
static int borg_danger_aux2(int i, int y, int x, bool average, bool full_damage)
{
    int q, n= 0, pfe =0, glyph= 0, glyph_check =0;

    int spot_x, spot_y, spot_safe=1;

    int lev, hp, total_dam = 0, av;

	bool bolt = FALSE;

    borg_kill *kill = &borg_kills[i];

    borg_grid   *ag;

    monster_race *r_ptr = &r_info[kill->r_idx];

    /*  PFE gives a protection.  */
        /* Hack -- Apply "protection from evil" */
        if ( (borg_prot_from_evil) &&
            (rf_has(r_ptr->flags, RF_EVIL)) &&
            ((borg_skill[BI_CLEVEL] ) >= r_ptr->level) )
        {
            pfe = 1;
        }

    /* glyph of warding rune of protection provides some small
     * protection with some ranged atacks; mainly summon attacks.
     * We should reduce the danger commensurate to the probability of the
     * monster breaking the glyph as defined by melee2.c
     */
    if (borg_on_glyph)
    {
        glyph = 1;
    }
    else if (track_glyph_num)
    {
        /* Check all existing glyphs */
        for (glyph_check = 0; glyph_check < track_glyph_num; glyph_check++)
        {
            if ((track_glyph_y[glyph_check] == y) && (track_glyph_x[glyph_check] == x))
            {
                /* Reduce the danger */
                glyph = 1;
            }
        }
    }

    /* Mega-Hack -- unknown monsters */
    if (kill->r_idx >= z_info->r_max) return (1000);


    /* Paranoia -- Nothing to cast */
	if (!kill->ranged_attack) return (0);


    /* Extract the level */
    lev = r_ptr->level;

    /* Extract hit-points */
    hp = kill->power;

    /* Analyze the spells */
	for (q = 0; q < kill->ranged_attack; q++)
    {
        int p = 0;

        int z = 0;

        /* Cast the spell. */
        switch (kill->spell[q])
        {
            case 32+0:    /* RF4_SHRIEK */
            /* if looking at full damage, things that are just annoying */
            /* do not count.*/
            /* Add fear for the effect */
                p += 5;
            break;

            case 32+1:    /* RF4_XXX2X4 */
            /* this is now a failed spell attempt for monsters */
            /* used to recognize invisible/ hidden monsters */
                p += 10;
            break;

            case 32+2:    /* RF4_XXX3X4 */
            break;

            case 32+3:    /* RF4_XXX4X4 */
            break;

            case 32+4:    /* RF4_ARROW_1 */
            z = (1 * 6);
            break;

            case 32+5:    /* RF4_ARROW_2 */
            z = (3 * 6);
            break;

            case 32+6:    /* RF4_ARROW_3 */
            z = (5 * 6);
            break;

            case 32+7:    /* RF4_ARROW_4 */
            z = (7 * 6);
            break;

            case 32+8:    /* RF4_BR_ACID */
            if (borg_skill[BI_IACID]) break;
            z = (hp / 3);
            /* max damage */
            if (z > 1600)
                z = 1600;
            if (borg_skill[BI_RACID]) z = (z + 2) / 3;
            if (borg_skill[BI_TRACID]) z = (z + 2) / 3;
            /* if looking at full damage, things that are just annoying */
            /* do not count.*/
            /* Add fear for the effect */
                p += 40;
            break;

            case 32+9:    /* RF4_BR_ELEC */
            if (borg_skill[BI_IELEC]) break;
            z = (hp / 3);
            /* max damage */
            if (z > 1600)
                z = 1600;
            if (borg_skill[BI_RELEC]) z = (z + 2) / 3;
            if (borg_skill[BI_TRELEC]) z = (z + 2) / 3;
            /* if looking at full damage, things that are just annoying */
            /* do not count.*/
            /* Add fear for the effect */
                p += 20;
            break;

            case 32+10:    /* RF4_BR_FIRE */
            if (borg_skill[BI_IFIRE]) break;
            z = (hp / 3);
            /* max damage */
            if (z > 1600)
                z = 1600;
            if (borg_skill[BI_RFIRE]) z = (z + 2) / 3;
            if (borg_skill[BI_TRFIRE]) z = (z + 2) / 3;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 40;
            break;

            case 32+11:    /* RF4_BR_COLD */
            if (borg_skill[BI_ICOLD]) break;
            z = (hp / 3);
            /* max damage */
            if (z > 1600)
                z = 1600;
            if (borg_skill[BI_RCOLD]) z = (z + 2) / 3;
            if (borg_skill[BI_TRCOLD]) z = (z + 2) / 3;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 20;
            break;

            case 32+12:    /* RF4_BR_POIS */
            z = (hp / 3);
            /* max damage */
            if (z > 800)
                z = 800;
            if (borg_skill[BI_RPOIS]) z = (z + 2) / 3;
            if (borg_skill[BI_TRPOIS]) z = (z + 2) / 3;
            if (borg_skill[BI_TRPOIS]) break;
            if (borg_skill[BI_RPOIS]) break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 20;
            break;

            case 32+13:    /* RF4_BR_NETH */
            z = (hp / 6);
            /* max damage */
            if (z > 350)
                z = 550;
            if (borg_skill[BI_RNTHR])
            {
                z = (z*6)/9;
                break;
            }
            /* Add fear for the effect */
                p += 125;
            break;

            case 32+14:    /* RF4_BR_LIGHT */
            z = (hp / 6);
            /* max damage */
            if (z > 400)
                z = 400;
            if (borg_skill[BI_RLITE])
            {
                z = (z*2)/3;
                break;
            }
            if (borg_skill[BI_RBLIND]) break;
            p += 20;
			if (borg_class == CLASS_MAGE) p +=20;
            break;

            case 32+15:    /* RF4_BR_DARK */
            z = (hp / 6);
            /* max damage */
            if (z > 400)
                z = 400;
            if (borg_skill[BI_RDARK]) z = (z*2)/ 3;
            if (borg_skill[BI_RDARK]) break;
            if (borg_skill[BI_RBLIND]) break;
            p += 20;
			if (borg_class == CLASS_MAGE) p +=20;
            break;

            case 32+16:    /* RF4_BR_CONF */
            z = (hp / 6);
            /* max damage */
            if (z > 400)
                z = 400;
            if (borg_skill[BI_RCONF]) z = z / 2;
            if (borg_skill[BI_RCONF]) break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 300;
			if (borg_class == CLASS_MAGE) p +=200;
            break;

            case 32+17:    /* RF4_BR_SOUN */
            z = (hp / 6);
            /* max damage */
            if (z > 500)
                z = 500;
            if (borg_skill[BI_RSND]) z = (z*5)/9;
            if (borg_skill[BI_RSND]) break;
            /* if already stunned be REALLY nervous dangerousabout this */
            if (borg_skill[BI_ISSTUN])
                z += 500;
            if (borg_skill[BI_ISHEAVYSTUN])
                z += 1000;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 50;
            break;

            case 32+18:    /* RF4_BR_CHAO */
            z = (hp / 6);
            /* max damage */
            if (z > 500)
                z = 500;
            if (borg_skill[BI_RKAOS]) z = (z*6)/9;
            /* Add fear for the effect */
                p += 100;
            if (borg_skill[BI_RKAOS]) break;
            p += 200;
            break;

            case 32+19:    /* RF4_BR_DISE */
            z = (hp / 6);
            /* max damage */
            if (z > 500)
                z = 500;
            if (borg_skill[BI_RDIS]) z = (z*6)/10;
            if (borg_skill[BI_RDIS]) break;
            p += 500;
            break;

            case 32+20:    /* RF4_BR_NEXU */
            z = (hp / 3);
            /* max damage */
            if (z > 400)
                z = 400;
            if (borg_skill[BI_RNXUS]) z = (z*6)/10;
            if (borg_skill[BI_RNXUS]) break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 100;
            break;

            case 32+21:    /* RF4_BR_TIME */
            z = (hp / 3);
            /* max damage */
            if (z > 150)
                z = 150;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 250;
            break;

            case 32+22:    /* RF4_BR_INER */
            z = (hp / 6);
            /* max damage */
            if (z > 200)
                z = 200;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 100;
            break;

            case 32+23:    /* RF4_BR_GRAV */
            z = (hp / 3);
            /* max damage */
            if (z > 200)
                z = 200;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 100;
            if (borg_skill[BI_RSND]) break;
            /* if already stunned be REALLY nervous about this */
            if (borg_skill[BI_ISSTUN])
                z += 500;
            if (borg_skill[BI_ISHEAVYSTUN])
                z += 1000;
            break;

            case 32+24:    /* RF4_BR_SHAR */
            z = (hp / 6);
            /* max damage */
            if (z > 500)
                z = 500;
            if (borg_skill[BI_RSHRD]) z = (z*6)/9;
            if (borg_skill[BI_RSHRD]) break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 50;
            break;

            case 32+25:    /* RF4_BR_PLAS */
            z = (hp / 6);
            /* max damage */
            if (z > 150)
                z = 150;
            if (borg_skill[BI_RSND]) break;
            /* Pump this up if you have goi so that the borg is sure */
            /* to be made nervous */
            p += 100;
            /* if already stunned be REALLY nervous about this */
            if (borg_skill[BI_ISSTUN])
                z += 500;
            if (borg_skill[BI_ISHEAVYSTUN])
                z += 1000;
            break;

            case 32+26:    /* RF4_BR_WALL */
            z = (hp / 6);
            /* max damage */
            if (z > 200)
                z = 200;
            if (borg_skill[BI_RSND]) break;
            /* if already stunned be REALLY nervous about this */
            if (borg_skill[BI_ISSTUN])
                z += 100;
            if (borg_skill[BI_ISHEAVYSTUN])
                z += 500;
            /* Add fear for the effect */
                p += 50;
            break;

            case 32+27:    /* RF4_BR_MANA */
            /* XXX XXX XXX */
            break;

            case 32+28:    /* RF4_XXX5X4 */
            break;

            case 32+29:    /* RF4_XXX6X4 */
            break;

            case 32+30:    /* RF4_XXX7X4 */
            break;

            case 32+31:    /* RF4_BOULDER */
            z = (1 + lev / 7) * 12 / 2;
            bolt = TRUE;
            break;

            case 64+0:    /* RF5_BA_ACID */
            if (borg_skill[BI_IACID]) break;
            z = (lev * 3) / 2 + 15;
            if (borg_skill[BI_RACID]) z = (z + 2) / 3;
            if (borg_skill[BI_TRACID]) z = (z + 2) / 3;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 40;
            break;

            case 64+1:    /* RF5_BA_ELEC */
            if (borg_skill[BI_IELEC]) break;
            z = (lev * 3) / 2 + 8;
            if (borg_skill[BI_RELEC]) z = (z + 2) / 3;
            if (borg_skill[BI_TRELEC]) z = (z + 2) / 3;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 20;
            break;

            case 64+2:    /* RF5_BA_FIRE */
            if (borg_skill[BI_IFIRE]) break;
            z = (lev * 7) / 2 + 10;
            if (borg_skill[BI_RFIRE]) z = (z + 2) / 3;
            if (borg_skill[BI_TRFIRE]) z = (z + 2) / 3;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 40;
            break;

            case 64+3:    /* RF5_BA_COLD */
            if (borg_skill[BI_ICOLD]) break;
            z = (lev * 3) / 2 + 10;
            if (borg_skill[BI_RCOLD]) z = (z + 2) / 3;
            if (borg_skill[BI_TRCOLD]) z = (z + 2) / 3;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 20;
            break;

            case 64+4:    /* RF5_BA_POIS */
            z = (12 * 2);
            if (borg_skill[BI_RPOIS]) z = (z + 2) / 3;
            if (borg_skill[BI_TRPOIS]) z = (z + 2) / 3;
            if (borg_skill[BI_TRPOIS]) break;
            if (borg_skill[BI_RPOIS]) break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 20;
            break;

            case 64+5:    /* RF5_BA_NETH */
            z = (50 +50 + (10 * 10) + lev);
            if (borg_skill[BI_RNTHR]) z = (z*6)/8;
            if (borg_skill[BI_RNTHR]) break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 250;
            break;

            case 64+6:    /* RF5_BA_WATE */
            z = ((lev * 5) / 2) + 50;
            if (borg_skill[BI_RSND]) break;
            /* if already stunned be REALLY nervous about this */
            if (borg_skill[BI_ISSTUN])
                p += 500;
            if (borg_skill[BI_ISHEAVYSTUN])
                p += 1000;
            if (borg_skill[BI_RCONF]) break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 50;
			if (borg_class == CLASS_MAGE) p +=20;
            break;

            case 64+7:    /* RF5_BA_MANA */
            z = ((lev * 5) + 150);
            /* Add fear for the effect */
                p += 50;
            break;

            case 64+8:    /* RF5_BA_DARK */
            z = (((lev * 5)) + (50));
            if (borg_skill[BI_RDARK]) z = (z*6)/9;
            if (borg_skill[BI_RDARK]) break;
            if (borg_skill[BI_RBLIND]) break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 20;
			if (borg_class == CLASS_MAGE) p +=20;
            break;

            case 64+9:    /* RF5_DRAIN_MANA */
            if (borg_skill[BI_MAXSP]) p += 10;
            break;

            case 64+10:    /* RF5_MIND_BLAST */
            if (borg_skill[BI_SAV] < 100)
                z = 20;
            break;

            case 64+11:    /* RF5_BRAIN_SMASH */
            z = (12 * 15);
            p += 200 - 2 * borg_skill[BI_SAV];
            if (p < 0) p =0;
            break;

            case 64+12:    /* RF5_CAUSE_1 */
            if (borg_skill[BI_SAV] >= 100) break;
            z = (3 * 8);
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                /* reduce by % chance of save  (add 20% for fudge) */
                z = z * (120 - borg_skill[BI_SAV]) / 100;
            break;

            case 64+13:    /* RF5_CAUSE_2 */
            if (borg_skill[BI_SAV] >= 100) break;
            z = (8 * 8);
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                /* reduce by % chance of save  (add 20% for fudge) */
                z = z * (120 - borg_skill[BI_SAV]) / 100;
            break;

            case 64+14:    /* RF5_CAUSE_3 */
            if (borg_skill[BI_SAV] >= 100) break;
            z = (10 * 15);
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                /* reduce by % chance of save  (add 20% for fudge) */
                z = z * (120 - borg_skill[BI_SAV]) / 100;
            break;

            case 64+15:    /* RF5_CAUSE_4 */
            if (borg_skill[BI_SAV] >= 100) break;
            z = (15 * 15);
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 20;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                /* reduce by % chance of save  (add 40% for fudge) */
                z = z * (120 - borg_skill[BI_SAV]) / 100;
            break;

            case 64+16:    /* RF5_BO_ACID */
            bolt = TRUE;
            if (borg_skill[BI_IACID]) break;
            z = ((7 * 8) + (lev / 3));
            if (borg_skill[BI_RACID]) z = (z + 2) / 3;
            if (borg_skill[BI_TRACID]) z = (z + 2) / 3;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 40;
            break;

            case 64+17:    /* RF5_BO_ELEC */
            if (borg_skill[BI_IELEC]) break;
            bolt = TRUE;
            z = ((4 * 8) + (lev / 3));
            if (borg_skill[BI_RELEC]) z = (z + 2) / 3;
            if (borg_skill[BI_TRELEC]) z = (z + 2) / 3;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 20;
            break;

            case 64+18:    /* RF5_BO_FIRE */
            if (borg_skill[BI_IFIRE]) break;
            bolt = TRUE;
            z = ((9 * 8) + (lev / 3));
            if (borg_skill[BI_RFIRE]) z = (z + 2) / 3;
            if (borg_skill[BI_TRFIRE]) z = (z + 2) / 3;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 40;
            break;

            case 64+19:    /* RF5_BO_COLD */
            if (borg_skill[BI_ICOLD]) break;
            bolt = TRUE;
            z = ((6 * 8) + (lev / 3));
            if (borg_skill[BI_RCOLD]) z = (z + 2) / 3;
            if (borg_skill[BI_TRCOLD]) z = (z + 2) / 3;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 20;
            break;

            case 64+20:    /* RF5_BO_POIS */
            /* XXX XXX XXX */
            bolt = TRUE;
            break;

            case 64+21:    /* RF5_BO_NETH */
            bolt = TRUE;
            z = (50 + 30 + (5 * 5) + (lev * 3) / 2);
            if (borg_skill[BI_RNTHR]) z = (z*6)/8;
            if (borg_skill[BI_RNTHR]) break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 200;
            break;

            case 64+22:    /* RF5_BO_WATE */
            z = ((10 * 10) + (lev));
            bolt = TRUE;
            if (borg_skill[BI_RSND]) break;
            /* if already stunned be REALLY nervous about this */
            if (borg_skill[BI_ISSTUN])
                p += 500;
            if (borg_skill[BI_ISHEAVYSTUN])
                p += 1000;
            if (borg_skill[BI_RCONF]) break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 20;
			if (borg_class == CLASS_MAGE) p +=20;
            break;

            case 64+23:    /* RF5_BO_MANA */
            z = ((lev * 7) / 2) + 50;
            bolt = TRUE;
            /* Add fear for the effect */
                p += 50;
            break;

            case 64+24:    /* RF5_BO_PLAS */
            z = (10 + (8 * 7) + (lev));
            bolt = TRUE;
            if (borg_skill[BI_RSND]) break;
            /* if already stunned be REALLY nervous about this */
            if (borg_skill[BI_ISSTUN])
                z += 500;
            if (borg_skill[BI_ISHEAVYSTUN])
                z += 1000;
            break;

            case 64+25:    /* RF5_BO_ICEE */
            z = ((6 * 6) + (lev));
            bolt = TRUE;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 20;
            if (borg_skill[BI_RSND]) break;
            /* if already stunned be REALLY nervous about this */
            if (borg_skill[BI_ISSTUN])
                z += 50;
            if (borg_skill[BI_ISHEAVYSTUN])
                z += 1000;
            break;

            case 64+26:    /* RF5_MISSILE */
            z = ((2 * 6) + (lev / 3));
            bolt = TRUE;
            break;

            case 64+27:    /* RF5_SCARE */
            if (borg_skill[BI_SAV] >= 100) break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 10;
            break;

            case 64+28:    /* RF5_BLIND */
            if (borg_skill[BI_SAV] >= 100) break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 10;
            break;

            case 64+29:    /* RF5_CONF */
            if (borg_skill[BI_SAV] >= 100) break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 10;
            break;

            case 64+30:    /* RF5_SLOW */
            if (borg_skill[BI_FRACT]) break;
            if (borg_skill[BI_SAV] >= 100) break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 5;
            break;

            case 64+31:    /* RF5_HOLD */
            if (borg_skill[BI_FRACT]) break;
            if (borg_skill[BI_SAV] >= 100) break;
            p += 150;
            break;

            case 96+0:    /* RF6_HASTE */
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 10;
            break;

            case 96+1:    /* RF6_XXX1X6 */
            break;

            case 96+2:    /* RF6_HEAL */
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 10;
            break;

            case 96+3:    /* RF6_XXX2X6 */
            break;

            case 96+4:    /* RF6_BLINK */
            break;

            case 96+5:    /* RF6_TPORT */
            break;

            case 96+6:    /* RF6_XXX3X6 */
            break;

            case 96+7:    /* RF6_XXX4X6 */
            break;

            case 96+8:    /* RF6_TELE_TO */
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 20;
            break;

            case 96+9:    /* RF6_TELE_AWAY */
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 10;
            break;

            case 96+10:    /* RF6_TELE_LEVEL */
            if (borg_skill[BI_SAV] >= 100) break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 50;
            break;

            case 96+11:    /* RF6_XXX5 */
            break;

            case 96+12:    /* RF6_DARKNESS */
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 5;
            break;

            case 96+13:    /* RF6_TRAPS */
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
                p += 50;
            break;

            case 96+14:    /* RF6_FORGET */
            if (borg_skill[BI_SAV] >= 100) break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            {
                /* if you are a spell caster, this is a big issue */
                if (borg_skill[BI_CURSP] < 15)
                {
                    p += 500;
                }
                else
                {
                    p += 30;
                }
            }
            break;

            case 96+15:    /* RF6_XXX6X6 */
            break;

            /* Summoning is only as dangerous as the monster that is
             * actually summoned but the monsters that summon are a priority
             * to kill.  PFE reduces danger from some evil summoned monsters
             * One Problem with GOI and Create Door is that the GOI reduces
             * the fear so much that the borg won't cast the Create Door,
             * eventhough it would be a good idea.
             */

            case 96+16:    /* S_KIN */
			/* This is used to calculate the free squares next to us.
			 * This is important when dealing with summoners.
			 */
			for (spot_x = -1; spot_x <= 1; spot_x++)
			{
				for (spot_y = -1; spot_y <= 1; spot_y++)
				{
					/* Acquire location */
					x = spot_x + kill->x;
					y = spot_y + kill->y;

					ag = &borg_grids[y][x];

					/* skip our own spot */
					if (x == kill->x && y == kill->y) continue;

					/* track spaces already protected */
					if ( ag->feat == FEAT_GLYPH || ag->kill ||
					   ((ag->feat >= FEAT_DOOR_HEAD) && (ag->feat <= FEAT_PERM_SOLID)))
					{   /* track the safe areas for calculating danger */
						spot_safe ++;

						/* Just in case */
						if (spot_safe == 0) spot_safe = 1;
						if (spot_safe == 8) spot_safe = 100;
						if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
					}

				}
			}
            if (pfe )
            {    p +=(lev);
                p = p / spot_safe;
            }
            else if (glyph || borg_create_door || borg_fighting_unique)
            {    p +=(lev) * 3;
                p = p / spot_safe;
            }
            else
            {    p += (lev) * 7;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75/100;

            break;

            case 96+17:    /* S_HI_DEMON */
			/* This is used to calculate the free squares next to us.
			 * This is important when dealing with summoners.
			 */
			for (spot_x = -1; spot_x <= 1; spot_x++)
			{
				for (spot_y = -1; spot_y <= 1; spot_y++)
				{
					/* Acquire location */
					x = spot_x + kill->x;
					y = spot_y + kill->y;

					ag = &borg_grids[y][x];

					/* skip our own spot */
					if (x == kill->x && y == kill->y) continue;

					/* track spaces already protected */
					if ( ag->feat == FEAT_GLYPH || ag->kill ||
					   ((ag->feat >= FEAT_DOOR_HEAD) && (ag->feat <= FEAT_PERM_SOLID)))
					{   /* track the safe areas for calculating danger */
						spot_safe ++;

						/* Just in case */
						if (spot_safe == 0) spot_safe = 1;
						if (spot_safe == 8) spot_safe = 100;
						if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
					}

				}
			}
            if (pfe )
            {    p +=(lev);
                p = p / spot_safe;
            }
            else if (glyph || borg_create_door || borg_fighting_unique)
            {    p +=(lev) * 6;
                p = p / spot_safe;
            }
            else
            {    p += (lev) * 12;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75/100;
            break;


            case 96+18:    /* RF6_S_MONSTER */
			/* This is used to calculate the free squares next to us.
			 * This is important when dealing with summoners.
			 */
			for (spot_x = -1; spot_x <= 1; spot_x++)
			{
				for (spot_y = -1; spot_y <= 1; spot_y++)
				{
					/* Acquire location */
					x = spot_x + kill->x;
					y = spot_y + kill->y;

					ag = &borg_grids[y][x];

					/* skip our own spot */
					if (x == kill->x && y == kill->y) continue;

					/* track spaces already protected */
					if ( ag->feat == FEAT_GLYPH || ag->kill ||
					   ((ag->feat >= FEAT_DOOR_HEAD) && (ag->feat <= FEAT_PERM_SOLID)))
					{   /* track the safe areas for calculating danger */
						spot_safe ++;

						/* Just in case */
						if (spot_safe == 0) spot_safe = 1;
						if (spot_safe == 8) spot_safe = 100;
						if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
					}

				}
			}
            if (pfe || glyph || borg_create_door || borg_fighting_unique)
                p +=0;
            else
            {    p += (lev) * 5;
                p = p / spot_safe;
            }
            break;

            case 96+19:    /* RF6_S_MONSTERS */
			/* This is used to calculate the free squares next to us.
			 * This is important when dealing with summoners.
			 */
			for (spot_x = -1; spot_x <= 1; spot_x++)
			{
				for (spot_y = -1; spot_y <= 1; spot_y++)
				{
					/* Acquire location */
					x = spot_x + kill->x;
					y = spot_y + kill->y;

					ag = &borg_grids[y][x];

					/* skip our own spot */
					if (x == kill->x && y == kill->y) continue;

					/* track spaces already protected */
					if ( ag->feat == FEAT_GLYPH || ag->kill ||
					   ((ag->feat >= FEAT_DOOR_HEAD) && (ag->feat <= FEAT_PERM_SOLID)))
					{   /* track the safe areas for calculating danger */
						spot_safe ++;

						/* Just in case */
						if (spot_safe == 0) spot_safe = 1;
						if (spot_safe == 8) spot_safe = 100;
						if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
					}

				}
			}
            if (pfe || glyph || borg_create_door || borg_fighting_unique)
                p +=0;
            else
            {    p += (lev) * 7;
                 p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75/100;
            break;

            case 96+20:   /* RF6_S_ANIMAL */
			/* This is used to calculate the free squares next to us.
			 * This is important when dealing with summoners.
			 */
			for (spot_x = -1; spot_x <= 1; spot_x++)
			{
				for (spot_y = -1; spot_y <= 1; spot_y++)
				{
					/* Acquire location */
					x = spot_x + kill->x;
					y = spot_y + kill->y;

					ag = &borg_grids[y][x];

					/* skip our own spot */
					if (x == kill->x && y == kill->y) continue;

					/* track spaces already protected */
					if ( ag->feat == FEAT_GLYPH || ag->kill ||
					   ((ag->feat >= FEAT_DOOR_HEAD) && (ag->feat <= FEAT_PERM_SOLID)))
					{   /* track the safe areas for calculating danger */
						spot_safe ++;

						/* Just in case */
						if (spot_safe == 0) spot_safe = 1;
						if (spot_safe == 8) spot_safe = 100;
						if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
					}

				}
			}
            if (pfe || glyph || borg_create_door || borg_fighting_unique)
                p +=0;
            else
            {   p += (lev) * 5;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75/100;
            break;

            case 96+21:    /* RF6_S_SPIDER */
			/* This is used to calculate the free squares next to us.
			 * This is important when dealing with summoners.
			 */
			for (spot_x = -1; spot_x <= 1; spot_x++)
			{
				for (spot_y = -1; spot_y <= 1; spot_y++)
				{
					/* Acquire location */
					x = spot_x + kill->x;
					y = spot_y + kill->y;

					ag = &borg_grids[y][x];

					/* skip our own spot */
					if (x == kill->x && y == kill->y) continue;

					/* track spaces already protected */
					if ( ag->feat == FEAT_GLYPH || ag->kill ||
					   ((ag->feat >= FEAT_DOOR_HEAD) && (ag->feat <= FEAT_PERM_SOLID)))
					{   /* track the safe areas for calculating danger */
						spot_safe ++;

						/* Just in case */
						if (spot_safe == 0) spot_safe = 1;
						if (spot_safe == 8) spot_safe = 100;
						if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
					}

				}
			}
            if (pfe || glyph || borg_create_door || borg_fighting_unique)
                p +=0;
            else
            {   p += (lev) * 5;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75/100;
            break;

            case 96+22:    /* RF6_S_HOUND */
			/* This is used to calculate the free squares next to us.
			 * This is important when dealing with summoners.
			 */
			for (spot_x = -1; spot_x <= 1; spot_x++)
			{
				for (spot_y = -1; spot_y <= 1; spot_y++)
				{
					/* Acquire location */
					x = spot_x + kill->x;
					y = spot_y + kill->y;

					ag = &borg_grids[y][x];

					/* skip our own spot */
					if (x == kill->x && y == kill->y) continue;

					/* track spaces already protected */
					if ( ag->feat == FEAT_GLYPH || ag->kill ||
					   ((ag->feat >= FEAT_DOOR_HEAD) && (ag->feat <= FEAT_PERM_SOLID)))
					{   /* track the safe areas for calculating danger */
						spot_safe ++;

						/* Just in case */
						if (spot_safe == 0) spot_safe = 1;
						if (spot_safe == 8) spot_safe = 100;
						if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
					}

				}
			}
            if (pfe || glyph || borg_create_door || borg_fighting_unique)
                p +=0;
            else
            {    p += (lev) * 5;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75/100;
            break;

            case 96+23:    /* RF6_S_HYDRA */
			/* This is used to calculate the free squares next to us.
			 * This is important when dealing with summoners.
			 */
			for (spot_x = -1; spot_x <= 1; spot_x++)
			{
				for (spot_y = -1; spot_y <= 1; spot_y++)
				{
					/* Acquire location */
					x = spot_x + kill->x;
					y = spot_y + kill->y;

					ag = &borg_grids[y][x];

					/* skip our own spot */
					if (x == kill->x && y == kill->y) continue;

					/* track spaces already protected */
					if ( ag->feat == FEAT_GLYPH || ag->kill ||
					   ((ag->feat >= FEAT_DOOR_HEAD) && (ag->feat <= FEAT_PERM_SOLID)))
					{   /* track the safe areas for calculating danger */
						spot_safe ++;

						/* Just in case */
						if (spot_safe == 0) spot_safe = 1;
						if (spot_safe == 8) spot_safe = 100;
						if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
					}

				}
			}
            if (pfe )
            {    p +=(lev);
                p = p / spot_safe;
            }
            else if (glyph || borg_create_door || borg_fighting_unique)
            {   p +=(lev) * 2;
                p = p / spot_safe;
            }
            else
            {   p += (lev) * 5;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75/100;
            break;

            case 96+24:    /* RF6_S_ANGEL */
			/* This is used to calculate the free squares next to us.
			 * This is important when dealing with summoners.
			 */
			for (spot_x = -1; spot_x <= 1; spot_x++)
			{
				for (spot_y = -1; spot_y <= 1; spot_y++)
				{
					/* Acquire location */
					x = spot_x + kill->x;
					y = spot_y + kill->y;

					ag = &borg_grids[y][x];

					/* skip our own spot */
					if (x == kill->x && y == kill->y) continue;

					/* track spaces already protected */
					if ( ag->feat == FEAT_GLYPH || ag->kill ||
					   ((ag->feat >= FEAT_DOOR_HEAD) && (ag->feat <= FEAT_PERM_SOLID)))
					{   /* track the safe areas for calculating danger */
						spot_safe ++;

						/* Just in case */
						if (spot_safe == 0) spot_safe = 1;
						if (spot_safe == 8) spot_safe = 100;
						if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
					}

				}
			}
            if (pfe  || borg_fighting_unique)
            {    p +=(lev);
                p = p / spot_safe;
            }
            else if (glyph || borg_create_door || borg_fighting_unique)
            {    p +=(lev)* 3;
                p = p / spot_safe;
            }
            else
            {    p += (lev) * 7;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75/100;
            break;

            case 96+25:    /* RF6_S_DEMON */
			/* This is used to calculate the free squares next to us.
			 * This is important when dealing with summoners.
			 */
			for (spot_x = -1; spot_x <= 1; spot_x++)
			{
				for (spot_y = -1; spot_y <= 1; spot_y++)
				{
					/* Acquire location */
					x = spot_x + kill->x;
					y = spot_y + kill->y;

					ag = &borg_grids[y][x];

					/* skip our own spot */
					if (x == kill->x && y == kill->y) continue;

					/* track spaces already protected */
					if ( ag->feat == FEAT_GLYPH || ag->kill ||
					   ((ag->feat >= FEAT_DOOR_HEAD) && (ag->feat <= FEAT_PERM_SOLID)))
					{   /* track the safe areas for calculating danger */
						spot_safe ++;

						/* Just in case */
						if (spot_safe == 0) spot_safe = 1;
						if (spot_safe == 8) spot_safe = 100;
						if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
					}

				}
			}
            if (pfe )
            {    p +=(lev);
                p = p / spot_safe;
            }
            else if (glyph || borg_create_door || borg_fighting_unique)
            {    p +=(lev) * 3;
                p = p / spot_safe;
            }
            else
            {    p += (lev) * 7;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75/100;
            break;

            case 96+26:    /* RF6_S_UNDEAD */
			/* This is used to calculate the free squares next to us.
			 * This is important when dealing with summoners.
			 */
			for (spot_x = -1; spot_x <= 1; spot_x++)
			{
				for (spot_y = -1; spot_y <= 1; spot_y++)
				{
					/* Acquire location */
					x = spot_x + kill->x;
					y = spot_y + kill->y;

					ag = &borg_grids[y][x];

					/* skip our own spot */
					if (x == kill->x && y == kill->y) continue;

					/* track spaces already protected */
					if ( ag->feat == FEAT_GLYPH || ag->kill ||
					   ((ag->feat >= FEAT_DOOR_HEAD) && (ag->feat <= FEAT_PERM_SOLID)))
					{   /* track the safe areas for calculating danger */
						spot_safe ++;

						/* Just in case */
						if (spot_safe == 0) spot_safe = 1;
						if (spot_safe == 8) spot_safe = 100;
						if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
					}

				}
			}
            if (pfe )
            {    p +=(lev);
                p = p / spot_safe;
            }
            else if (glyph || borg_create_door || borg_fighting_unique)
            {    p +=(lev) * 3;
                p = p / spot_safe;
            }
            else
            {    p += (lev) * 7;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75/100;
            break;

            case 96+27:    /* RF6_S_DRAGON */
			/* This is used to calculate the free squares next to us.
			 * This is important when dealing with summoners.
			 */
			for (spot_x = -1; spot_x <= 1; spot_x++)
			{
				for (spot_y = -1; spot_y <= 1; spot_y++)
				{
					/* Acquire location */
					x = spot_x + kill->x;
					y = spot_y + kill->y;

					ag = &borg_grids[y][x];

					/* skip our own spot */
					if (x == kill->x && y == kill->y) continue;

					/* track spaces already protected */
					if ( ag->feat == FEAT_GLYPH || ag->kill ||
					   ((ag->feat >= FEAT_DOOR_HEAD) && (ag->feat <= FEAT_PERM_SOLID)))
					{   /* track the safe areas for calculating danger */
						spot_safe ++;

						/* Just in case */
						if (spot_safe == 0) spot_safe = 1;
						if (spot_safe == 8) spot_safe = 100;
						if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
					}

				}
			}
            if (pfe )
            {    p +=(lev);
                p = p / spot_safe;
            }
            else if (glyph || borg_create_door || borg_fighting_unique)
            {    p +=(lev) * 3;
                p = p / spot_safe;
            }
            else
            {    p += (lev) * 7;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75/100;
            break;

            case 96+28:    /* RF6_S_HI_UNDEAD */
			/* This is used to calculate the free squares next to us.
			 * This is important when dealing with summoners.
			 */
			for (spot_x = -1; spot_x <= 1; spot_x++)
			{
				for (spot_y = -1; spot_y <= 1; spot_y++)
				{
					/* Acquire location */
					x = spot_x + kill->x;
					y = spot_y + kill->y;

					ag = &borg_grids[y][x];

					/* skip our own spot */
					if (x == kill->x && y == kill->y) continue;

					/* track spaces already protected */
					if ( ag->feat == FEAT_GLYPH || ag->kill ||
					   ((ag->feat >= FEAT_DOOR_HEAD) && (ag->feat <= FEAT_PERM_SOLID)))
					{   /* track the safe areas for calculating danger */
						spot_safe ++;

						/* Just in case */
						if (spot_safe == 0) spot_safe = 1;
						if (spot_safe == 8) spot_safe = 100;
						if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
					}

				}
			}
            if (pfe )
            {    p +=(lev);
                p = p / spot_safe;
            }
            else if (glyph || borg_create_door || borg_fighting_unique)
            {    p +=(lev) * 6;
                p = p / spot_safe;
            }
            else
            {    p += (lev) * 12;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75/100;
            break;

            case 96+29:    /* RF6_S_HI_DRAGON */
			/* This is used to calculate the free squares next to us.
			 * This is important when dealing with summoners.
			 */
			for (spot_x = -1; spot_x <= 1; spot_x++)
			{
				for (spot_y = -1; spot_y <= 1; spot_y++)
				{
					/* Acquire location */
					x = spot_x + kill->x;
					y = spot_y + kill->y;

					ag = &borg_grids[y][x];

					/* skip our own spot */
					if (x == kill->x && y == kill->y) continue;

					/* track spaces already protected */
					if ( ag->feat == FEAT_GLYPH || ag->kill ||
					   ((ag->feat >= FEAT_DOOR_HEAD) && (ag->feat <= FEAT_PERM_SOLID)))
					{   /* track the safe areas for calculating danger */
						spot_safe ++;

						/* Just in case */
						if (spot_safe == 0) spot_safe = 1;
						if (spot_safe == 8) spot_safe = 100;
						if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
					}

				}
			}
            if (pfe )
            {
                p = p / spot_safe;
            }
            else if (glyph || borg_create_door || borg_fighting_unique)
            {    p +=(lev) * 6;
                p = p / spot_safe;
            }
            else
            {    p += (lev) * 12;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75/100;
            break;

            case 96+30:    /* RF6_S_WRAITH */
			/* This is used to calculate the free squares next to us.
			 * This is important when dealing with summoners.
			 */
			for (spot_x = -1; spot_x <= 1; spot_x++)
			{
				for (spot_y = -1; spot_y <= 1; spot_y++)
				{
					/* Acquire location */
					x = spot_x + kill->x;
					y = spot_y + kill->y;

					ag = &borg_grids[y][x];

					/* skip our own spot */
					if (x == kill->x && y == kill->y) continue;

					/* track spaces already protected */
					if ( ag->feat == FEAT_GLYPH || ag->kill ||
					   ((ag->feat >= FEAT_DOOR_HEAD) && (ag->feat <= FEAT_PERM_SOLID)))
					{   /* track the safe areas for calculating danger */
						spot_safe ++;

						/* Just in case */
						if (spot_safe == 0) spot_safe = 1;
						if (spot_safe == 8) spot_safe = 100;
						if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
					}

				}
			}
            if (pfe )
            {    p +=(lev);
                p = p / spot_safe;
            }
            else if (glyph || borg_create_door || borg_fighting_unique)
            {    p +=(lev) * 6;
                p = p / spot_safe;
            }
            else
            {    p += (lev) * 12;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75/100;
            break;

            case 96+31:    /* RF6_S_UNIQUE */
			/* This is used to calculate the free squares next to us.
			 * This is important when dealing with summoners.
			 */
			for (spot_x = -1; spot_x <= 1; spot_x++)
			{
				for (spot_y = -1; spot_y <= 1; spot_y++)
				{
					/* Acquire location */
					x = spot_x + kill->x;
					y = spot_y + kill->y;

					ag = &borg_grids[y][x];

					/* skip our own spot */
					if (x == kill->x && y == kill->y) continue;

					/* track spaces already protected */
					if ( ag->feat == FEAT_GLYPH || ag->kill ||
					   ((ag->feat >= FEAT_DOOR_HEAD) && (ag->feat <= FEAT_PERM_SOLID)))
					{   /* track the safe areas for calculating danger */
						spot_safe ++;

						/* Just in case */
						if (spot_safe == 0) spot_safe = 1;
						if (spot_safe == 8) spot_safe = 100;
						if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
					}

				}
			}
            if (pfe )
            {    p +=(lev);
                p = p / spot_safe;
            }
            else if (glyph || borg_create_door)
            {    p +=(lev) * 3;    /* slightly reduced danger for unique */
                p = p / spot_safe;
            }
            else
            {    p += (lev) * 6;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75/100;
            break;
        }

		/* A bolt spell cannot jump monsters to hit the borg. */
		if (bolt == TRUE && !borg_projectable_pure(kill->y, kill->x, c_y, c_x)) z = 0;

		/* Some borgs are concerned with the 'effects' of an attack.  ie, cold attacks shatter potions,
		 * fire attacks burn scrolls, electric attacks zap rings.
		 */
		if (borg_skill[BI_MAXDEPTH] >= 75) p = 0;

        /* Notice damage */
        p += z;

        /* Track the most dangerous spell */
        if (p > n) n = p;

        /* Track the damage of all the spells, used in averaging */
        total_dam +=p;
    }

	/* Slightly decrease the danger if the borg is sitting in
	 * a sea of runes.
	 */
	if (borg_morgoth_position || borg_as_position) total_dam = total_dam * 7 / 10;

    /* Average damage of all the spells & compare to most dangerous spell */
	av = total_dam / kill->ranged_attack;

    /* If the most dangerous spell is alot bigger than the average,
     * then return the dangerous one.
     *
     * There is a problem when dealing with defence manuevers.
     * If the borg is considering casting a spell like
     * Resistance and the monster also has a non
     * resistable attack (like Disenchant) then the damage
     * returned will be for that spell, since the danger of the
     * others (like fire, cold) will be greatly reduced by the
     * proposed defence spell.  The result will be the borg will
     * not cast the resistance spell eventhough it may be a very
     * good idea.
     *
     * Example: a monster has three breath attacks (Fire, Ice,
     * Disenchant) and each hits for 800 pts of damage.  The
     * borg currently resists all three, so the danger would be
     * 500. If the borg were to use a Res Heat Potion that would
     * decrease the danger to:
     * Fire:  333
     * Ice:   500
     * Disen: 500
     * Now the Average is 444.  Not really worth it, nominal change.
     * But if the resistance spell was both Fire and Ice, then
     * it would be:
     * Fire:  333
     * Ice:   333
     * Disen: 500
     * With an average of 388. Probably worth it, but the borg
     * would see that the Disen attack is a quite dangerous and
     * would return the result of 500.
     *
     * To fix this, the flag 'average' is added to the
     * borg_danger() to skip this check and return the average
     * damage.  If the flag is FALSE then the formula below is
     * SKIPPED and the value returned with be the average.
     * If the flag is TRUE, then the formula below will be used
     * to determine the returned value.  Currently the elemental
     * resistance spells and PFE have the flag set as FALSE.
     *
     */
    if (!average) return (av);
    if (n >= av * 15/10 || n > borg_skill[BI_CURHP] * 8/10) return (n);
    else
    /* Average Danger */
    return (av);
}


/*
 * Calculate the danger to a grid from a monster  XXX XXX XXX
 *
 * Note that we are paranoid, especially about "monster speed",
 * since even if a monster is slower than us, it will occasionally
 * get one full turn to attack us.
 *
 * Note that we assume that monsters can walk through walls and
 * other monsters to get to the player.  XXX XXX XXX
 *
 * This function attempts to consider possibilities such as movement plus
 * spell attacks, physical attacks and spell attacks together,
 * and other similar situations.
 *
 * Currently we assume that "sleeping" monsters are less dangerous
 * unless you get near them, which may wake them up.
 *
 * We attempt to take into account things like monsters which sometimes
 * "stumble", and monsters which only "sometimes" use powerful spells.
 */
int borg_danger_aux(int y, int x, int c, int i, bool average, bool full_damage)
{
    borg_kill *kill = &borg_kills[i];

    monster_race *r_ptr = &r_info[kill->r_idx];

    int x9 = kill->x;
    int y9 = kill->y;
	int y_temp, x_temp;

    int ax, ay, d;

    int q=0, r, p, v1=0, v2=0, b_v2 = 0, b_v1 = 0;

    int glyph =0;

    int fake_speed = borg_skill[BI_SPEED];
    int monster_speed = kill->speed;
	int monster_moves = kill->moves;
    int t, e;

	int ii;
    int chance;

    /* Paranoia */
    if (!kill->r_idx) return (0);

	/* Skip certain monster indexes.
	 * These have been listed mainly in Teleport Other
	 * checks in borg6.c in the defence maneuvers.
	 */
	if (borg_tp_other_n)
	{
		for (ii = 1; ii <= borg_tp_other_n; ii++)
		{
			/* Is the current danger check same as a saved monster index? */
			if (i == borg_tp_other_index[ii])
			{
				return (0);
			}
		}
	}


    /* Distance components */
    ax = (x9 > x) ? (x9 - x) : (x - x9);
    ay = (y9 > y) ? (y9 - y) : (y - y9);

    /* Distance */
    d = MAX(ax, ay);

    /* Minimal distance */
    if (d < 1) d = 1;

    /* Minimal distance */
    if (d > 20) return (0);

    /* A very speedy borg will miscalculate danger of some monsters */
    if (borg_skill[BI_SPEED] >=135) fake_speed = (borg_fighting_unique ? 120 : 125);

    /* Consider the character haste and slow monster spells */
    if (borg_speed)
        fake_speed += 10;
    if (borg_slow_spell)
        monster_speed -= 10;

    /* Assume monsters are a little fast when you are low level */
    if (borg_skill[BI_MAXHP] < 20 && borg_skill[BI_CDEPTH])
        monster_speed += 3;


    /* Player energy per game turn  */
    e = extract_energy[(fake_speed)];

    /* Game turns per player move  */
    t = (100 + (e - 1)) / e;

    /*  Monster energy per game turn  */
    e = extract_energy[monster_speed];

    /* Monster moves */
    q = c * ((t * e) / 10);

    /* allow partial hits when not caculating full possible damage */
    if (full_damage)
        q = (int)((q+9)/10)*10;

    /* Minimal energy.  Monsters get at least some energy.
     * If the borg is very fast relative to a monster, then the
     * monster danger is artifically low due to the way the borg
     * will calculate the danger and energy.  So the monsters must
     * be given some base energy to equate the borg's.
     * ie:  the borg with speed +40 (speed = 150) is attacking
     * a monster with normal speed (speed = 110).  One would
     * think that the borg gets 4 attacks per turn over the monster.
     * and this does happen.  What if the monster can deal out
     * 1000 damage pts per monster attack turn?  The borg will
     * reduce the danger to 250 because the borg is 4x faster
     * than the monster.  But eventually the borg will get hit
     * by that 1000 pt damage attack.  And when it does, its
     * going to hurt.
     * So we make sure the monster is at least as fast as us.
     * But the monster is allowed to be faster than us.
     */
	if (q <= 10) q = 10;

    /** Danger from physical attacks **/

    /* Physical attacks */
    v1 = borg_danger_aux1(i, full_damage);

    /* Hack -- Under Stressful Situation.
     */
    if (time_this_panel > 1200 || borg_t > 25000)
    {
        /* he might be stuck and could overflow */
        v1 = v1 / 5;
    }

    /* No attacks for some monsters */
    if (rf_has(r_ptr->flags, RF_NEVER_BLOW))
    {
        v1 = 0;
    }

    /* No movement for some monsters */
    if ((rf_has(r_ptr->flags, RF_NEVER_MOVE)) && (d > 1))
    {
        v1 = 0;
    }

    /* multipliers yeild some trouble when I am weak */
    if ((rf_has(r_ptr->flags, RF_MULTIPLY)) && (borg_skill[BI_CLEVEL] < 20))
    {   /* extra 50% */
        v1 = v1 + (v1 *15/10);
    }

    /* Friends yeild some trouble when I am weak */
    if ((rf_has(r_ptr->flags, RF_FRIENDS) || rf_has(r_ptr->flags, RF_ESCORTS)) &&
        (borg_skill[BI_CLEVEL] < 20))
    {
        if (borg_skill[BI_CLEVEL] < 15)
        {
            /* extra 80% */
            v1 = v1 + (v1 *18/10);
        }
        else
        {
            /* extra 30% */
            v1 = v1 + (v1 *13/10);
        }

    }

    /* Reduce danger from sleeping monsters */
    if (!kill->awake)
    {
        int inc = r_ptr->sleep + 5;
		/* Reduce the fear if Borg is higher level */
        if (borg_skill[BI_CLEVEL] >= 25 )
        {
             v1 = v1 / 2;
        }
        else
        {
            /* low clevel weaklings should still fear alot*/
            v1 = v1;
        }

		/* Tweak danger based on the "alertness" of the monster */
        /* increase the danger for light sleepers */
		v1 = v1 + (v1*inc/100);
	}
     /* Reduce danger from sleeping monsters with the sleep 2 spell*/
    if (borg_sleep_spell_ii)
    {
        if  ( (d == 1) &&
             (kill->awake) &&
             (!(rf_has(r_ptr->flags, RF_NO_SLEEP))) &&
             (!(rf_has(r_ptr->flags, RF_UNIQUE))) &&
             (kill->level <= (borg_skill[BI_CLEVEL] - 15)))
        {
			/* Under special circumstances force the damage to 0 */
			if (borg_skill[BI_CLEVEL] < 20 &&
				borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2)
			{
				v1 = 0;
			}
			else
			{
				v1 = v1 / 3;
			}
        }
    }
     /* Reduce danger from sleeping monsters with the sleep 1,3 spell*/
    if (borg_sleep_spell)
    {
		if (kill->awake &&
           (!(rf_has(r_ptr->flags, RF_NO_SLEEP))) &&
            (!(rf_has(r_ptr->flags, RF_UNIQUE))) &&
            (kill->level <= (borg_skill[BI_CLEVEL] - 15)))
		{
			/* Under special circumstances force the damage to 0 */
			if (borg_skill[BI_CLEVEL] < 20 &&
				borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2)
			{
				v1 = 0;
			}
			else
			{
		        v1 = v1 / (d+2);
			}
		}
    }
    /* Reduce danger from confused monsters */
    if (kill->confused)
    {
       v1 = v1 / 2;
    }
    if (kill->stunned)
    {
       v1 = v1 * 10 / 13;
    }
     if (borg_confuse_spell)
    {
		if (kill->awake &&
			!kill->confused &&
           (!(rf_has(r_ptr->flags, RF_NO_SLEEP))) &&
            (!(rf_has(r_ptr->flags, RF_UNIQUE))) &&
            (kill->level <= (borg_skill[BI_CLEVEL] - 15)))
		{
			/* Under special circumstances force the damage to 0 */
			if (borg_skill[BI_CLEVEL] < 20 &&
				borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2)
			{
				v1 = 0;
			}
			else
			{
		        v1 = v1 / (d+2);
			}
		}
    }
     /* Perceive a reduce danger from scared monsters */
    if (borg_fear_mon_spell)
    {
        v1 = 0;
    }

    /* Hack -- Physical attacks require proximity
	 *
	 * Note that we do try to consider a fast monster moving and attacking
	 * in the same round.  We should consider monsters that have a speed 2 or 3 classes
	 * higher than ours, but most times, the borg will only encounter monsters with a single
	 * catagory higher speed.
	 */
    if (q > 10 && d != 1 && !(rf_has(r_ptr->flags, RF_NEVER_MOVE)))
    {
		b_v1 = 0;

		/* Check for a single grid movement, simulating the monster's move action. */
		for (ii = 0; ii < 8; ii++)
		{
			/* Obtain a grid to which the monster might move */
			y_temp = y9 + ddy_ddd[ii];
			x_temp = x9 + ddx_ddd[ii];

			/* Check for legality */
			if (!in_bounds_fully(y_temp,x_temp)) continue;

			/* Cannot occupy another monster's grid */
			if (borg_grids[y_temp][x_temp].kill) continue;

			/* Cannot occupy a closed door */
			if (borg_grids[y_temp][x_temp].feat >= FEAT_DOOR_HEAD &&
				borg_grids[y_temp][x_temp].feat <= FEAT_DOOR_TAIL) continue;

			/* Cannot occupy a perma-wall */
			if (borg_grids[y_temp][x_temp].feat >= FEAT_PERM_EXTRA) continue;

			/* Cannot occupy a wall/seam grid (unless pass_wall or kill_wall) */
			if ((borg_grids[y_temp][x_temp].feat >= FEAT_WALL_EXTRA &&
				  borg_grids[y_temp][x_temp].feat <= FEAT_WALL_SOLID) ||
				 (borg_grids[y_temp][x_temp].feat == FEAT_MAGMA ||
				  borg_grids[y_temp][x_temp].feat == FEAT_QUARTZ ||
				  borg_grids[y_temp][x_temp].feat == FEAT_MAGMA_K ||
				  borg_grids[y_temp][x_temp].feat == FEAT_QUARTZ_K ||
				  borg_grids[y_temp][x_temp].feat == FEAT_RUBBLE))
			{
				/* legally on a wall of some sort, check for closeness*/
			    if (rf_has(r_ptr->flags, RF_PASS_WALL))
				{
					if (distance(y_temp, x_temp, y, x) == 1) b_v1 = v1;
				}
				if (rf_has(r_ptr->flags, RF_KILL_WALL))
				{
					if (distance(y_temp, x_temp, y, x) == 1) b_v1 = v1;
				}
			}

			/* Is this grid being considered adjacent to the grid for which the borg_danger() was called? */
			if (distance(y_temp, x_temp, y, x) >1) continue;

			/* A legal floor grid */
			if (borg_cave_floor_bold(y_temp, x_temp))
			{
				/* Really fast monster can hit me more than once after it's move */
				b_v1 = v1 * (q/(d*10));
			}
		}

		/* Monster is not able to move and threaten me in the same round */
		v1 = b_v1;
	}

	/* Consider a monster that is fast and can strike more than once per round */
	if (q > 10 && d == 1)
	{
		v1= v1 * q / 10;
	}

	/* Need to be close if you are normal speed */
	if (q == 10 && d > 1)
	{
		v1 = 0;
	}

    /** Ranged Attacks **/
    v2 = borg_danger_aux2(i,y,x,average, full_damage);

   /* Never cast spells */
    if (!r_ptr->freq_innate && !r_ptr->freq_spell)
    {
        v2 = 0;
    }

    /* Hack -- verify distance */
    if (distance(y9, x9, y, x) > MAX_RANGE)
    {
        v2 = 0;
    }

    /* Hack -- verify line of sight (both ways) for monsters who can only move 1 grid. */
    if (q <= 10 && !borg_projectable(y9, x9, y, x) && !borg_projectable(y, x, y9, x9))
    {
        v2 = 0;
    }

	/* Hack -- verify line of sight (both ways) for monsters who can only move > 1 grid.
	 * Some fast monsters can take a move action and range attack in the same round.
	 * Basically, we see how many grids the monster can move and check LOS from each of
	 * those grids to determine the relative danger.  We need to make sure that the monster
	 * is not passing through walls unless he has that ability.
	     * Consider a fast monster who can move and cast a spell in the same round.
		 * This is important for a borg looking for a safe grid from a ranged attacker.
		 * If the attacker is fast then he might be able to move to a grid which does have LOS
		 * to the grid the borg is considering.
		 *
		 * ##############
		 * #.....#.#.1#D#   Grids marked 2 are threatened by the D currently.
		 * #####.#..##@##	Grids marked 1 are safe currently, but the fast D will be able
		 * #####.#..1221#	to move to the borg's grid after he moves and the D will be able
		 * ##############	to use a ranged attack to grids 1, all in the same round.
		 *					The borg should not consider grid 1 as safe.
	*/
	if (q >= 20)
	{
		int b_q = q;
		b_v2 = 0;

		/* Maximal speed check */
		if (q > 20) q = 20;

		/* Check for a single grid movement, simulating the monster's move action. */
		for (ii = 0; ii < 8; ii++)
		{
			/* Obtain a grid to which the monster might move */
			y_temp = y9 + ddy_ddd[ii];
			x_temp = x9 + ddx_ddd[ii];

			/* Check for legality */
			if (!in_bounds_fully(y_temp,x_temp)) continue;

			/* Cannot occupy another monster's grid */
			if (borg_grids[y_temp][x_temp].kill) continue;

			/* Cannot occupy a closed door */
			if (borg_grids[y_temp][x_temp].feat >= FEAT_DOOR_HEAD &&
				borg_grids[y_temp][x_temp].feat <= FEAT_DOOR_TAIL) continue;

			/* Cannot occupy a perma-wall */
			if (borg_grids[y_temp][x_temp].feat >= FEAT_PERM_EXTRA) continue;

			/* Cannot occupy a wall/seam grid (unless pass_wall or kill_wall) */
			if ((borg_grids[y_temp][x_temp].feat >= FEAT_WALL_EXTRA &&
				  borg_grids[y_temp][x_temp].feat <= FEAT_WALL_SOLID) ||
				 (borg_grids[y_temp][x_temp].feat == FEAT_MAGMA ||
				  borg_grids[y_temp][x_temp].feat == FEAT_QUARTZ ||
				  borg_grids[y_temp][x_temp].feat == FEAT_MAGMA_K ||
				  borg_grids[y_temp][x_temp].feat == FEAT_QUARTZ_K ||
				  borg_grids[y_temp][x_temp].feat == FEAT_RUBBLE))
			{
				/* legally on a wall of some sort, check for LOS*/
			    if (rf_has(r_ptr->flags, RF_PASS_WALL))
				{
					if (borg_projectable(y_temp, x_temp, y, x)) b_v2 = v2 * b_q / 10;
				}
				if (rf_has(r_ptr->flags, RF_KILL_WALL))
				{
					if (borg_projectable(y_temp, x_temp, y, x)) b_v2 = v2 * b_q / 10;
				}
			}

			/* Monster on a legal floor grid.  Can he see me? */
			else if (borg_projectable(y_temp, x_temp, y, x)) b_v2 = v2 * b_q / 10;
		}

		/* Monster is not able to move and threaten me in the same round */
		v2 = b_v2;
	}

		/* Hack -- Under Stressful Situation.
         */
        if (time_this_panel > 1200 || borg_t > 25000)
        {
            /* he might be stuck and could overflow */
            v2 = v2 / 5;
        }

        /* multipliers yeild some trouble when I am weak */
        if ((rf_has(r_ptr->flags, RF_MULTIPLY)) && (borg_skill[BI_CLEVEL] < 20))
        {
            v2 = v2 + (v2 *12/10);
        }

        /* Friends yeild some trouble when I am weak */
        if ((rf_has(r_ptr->flags, RF_FRIENDS) || rf_has(r_ptr->flags, RF_ESCORTS)) &&
            (borg_skill[BI_CLEVEL] < 20))
        {
            v2 = v2 + (v2 *12/10);
        }

        /* Reduce danger from sleeping monsters */
        if (!kill->awake)
        {
            int inc = r_ptr->sleep + 5;
			/* weaklings and should still fear */
            if (borg_skill[BI_CLEVEL] >= 25 )
            {
                 v2 = v2 / 2;
            }
            else
            {
                /* only subract 50% of the danger */
                v2 = v2;
            }

			/* Tweak danger based on the "alertness" of the monster */
            /* increase the danger for light sleepers */
            v2 = v2 + (v2*inc/100);
		}

        /* Reduce danger from sleeping monsters with the sleep 2 spell*/
        if (borg_sleep_spell_ii)
        {

            if  ( (d == 1) &&
                  (kill->awake) &&
                  (!(rf_has(r_ptr->flags, RF_NO_SLEEP))) &&
                  (!(rf_has(r_ptr->flags, RF_UNIQUE))) &&
                  (kill->level <= ((borg_skill[BI_CLEVEL] < 15)  ? borg_skill[BI_CLEVEL] : (((borg_skill[BI_CLEVEL]-10)/4)*3) + 10) ))
            {
                  v2 = v2 / 3;
            }
        }

        /* Reduce danger from sleeping monsters with the sleep 1,3 spell*/
        if (borg_sleep_spell)
        {
            v2 = v2 / (d+2);
        }
        /* Reduce danger from confused monsters */
        if (kill->confused)
        {
           v2 = v2 / 2;
        }
        /* Reduce danger from stunnned monsters  */
        if (kill->stunned)
        {
           v2 = v2 *10/13;
        }
        if (borg_confuse_spell)
        {
            v2 = v2 / 6;
        }

#if 0 /* They still cast spells, they are still dangerous */
        /* Reduce danger from scared monsters */
        if (borg_fear_mon_spell)
        {
            v2 = v2 * 8/10;
        }
        if (kill->afraid)
        {
            v2 = v2 * 8/10;
        }
#endif
        if (!full_damage)
        {
            /* reduce for frequency. */
            chance = (r_ptr->freq_innate + r_ptr->freq_spell)/2;
            if (chance < 11)
                v2 = ((v2 * 4) / 10);
            else
            if (chance < 26)
                v2 = ((v2 * 6) / 10);
            else
            if (chance < 51)
                v2 = ((v2 * 8) / 10) ;
        }

        /* Danger */
        if (v2)
        {
            /* Full power */
            r = q;

            /* Total danger */
            v2 = v2 * r / 10;
        }

    /* Maximal danger */
    p = MAX(v1, v2);
	if (p > 2000) p = 2000;

    /* Result */
    return (p);
}


/*
 * Hack -- Calculate the "danger" of the given grid.
 *
 * Currently based on the physical power of nearby monsters, as well
 * as the spell power of monsters which can target the given grid.
 *
 * This function is extremely expensive, mostly due to the number of
 * times it is called, and also to the fact that it calls its helper
 * functions about thirty times each per call.
 *
 * We need to do more intelligent processing with the "c" parameter,
 * since currently the Borg does not realize that backing into a
 * hallway is a good idea, since as far as he can tell, many of
 * the nearby monsters can "squeeze" into a single grid.
 *
 * Note that we also take account of the danger of the "region" in
 * which the grid is located, which allows us to apply some "fear"
 * of invisible monsters and things of that nature.
 *
 * Generally bool Average is TRUE.
 */
int borg_danger(int y, int x, int c, bool average, bool full_damage)
{
    int i, p=0;

    /* Base danger (from regional fear) but not within a vault.  Cheating the floor grid */
	if (!(cave->info[y][x] & (CAVE_ICKY)) && borg_skill[BI_CDEPTH] <= 80)
	{
		p += borg_fear_region[y/11][x/11] * c;
	}

	/* Reduce regional fear on Depth 100 */
	if (borg_skill[BI_CDEPTH] == 100 && p >= 300) p = 300;

    /* Added danger (from a lot of monsters).
     * But do not add it if we have been sitting on
     * this panel for too long, or monster's in a vault.  The fear_monsters[][]
     * can induce some bouncy behavior.
     */
    if (time_this_panel <= 200 &&
		!(cave->info[y][x] & (CAVE_ICKY))) p += borg_fear_monsters[y][x] * c;

    full_damage = TRUE;

    /* Examine all the monsters */
    for (i = 1; i < borg_kills_nxt; i++)
    {
        borg_kill *kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Collect danger from monster */
        p += borg_danger_aux(y, x, c, i, average, full_damage);
    }

    /* Return the danger */
    return (p > 2000 ? 2000 : p);
}




/*
 * Determine if the Borg is out of "crucial" supplies.
 *
 * Note that we ignore "restock" issues for the first several turns
 * on each level, to prevent repeated "level bouncing".
 */
cptr borg_restock(int depth)
{

    /* We are now looking at our preparedness */
    if ( -1 == borg_ready_morgoth)
        borg_ready_morgoth = 0;

    /* Always ready for the town */
    if (!depth) return ((cptr)NULL);

	/* Always Ready to leave town */
	if (borg_skill[BI_CDEPTH] == 0) return ((cptr)NULL);

    /* Always spend time on a level unless 100*/
    if (borg_t - borg_began < 100 && borg_skill[BI_CDEPTH] != 100) return ((cptr)NULL);


    /*** Level 1 ***/

    /* Must have some lite */
    if (borg_skill[BI_CURLITE] < 1) return ("rs my_CURLITE");

    /* Must have "fuel" */
    if (borg_skill[BI_AFUEL] < 1 && !borg_skill[BI_LIGHT]) return ("rs amt_fuel");

    /* Must have "food" */
    if (borg_skill[BI_FOOD] < 1) return ("rs amt_food");

    /* Assume happy at level 1 */
    if (depth <= 1) return ((cptr)NULL);

    /*** Level 2 and 3 ***/

    /* Must have good lite */
    if (borg_skill[BI_CURLITE] < 2) return ("rs lite+1");

    /* Must have "fuel" */
    if (borg_skill[BI_AFUEL] < 2 && !borg_skill[BI_LIGHT]) return ("rs fuel+2");

    /* Must have "food" */
    if (borg_skill[BI_FOOD] < 3) return ("rs food+2");

    /* Must have "recall" */
    /* if (borg_skill[BI_RECALL] < 2) return ("rs recall"); */

    /* Assume happy at level 3 */
    if (depth <= 3) return ((cptr)NULL);

    /*** Level 3 to 5 ***/

    if (depth <= 5) return ((cptr)NULL);

    /*** Level 6 to 9 ***/

    /* Must have "phase" */
    if (borg_skill[BI_APHASE] < 1) return ("rs phase");

    /* Potions of Cure Wounds */
    if ((borg_skill[BI_MAXCLEVEL] < 30) && borg_skill[BI_ACLW] + borg_skill[BI_ACSW] + borg_skill[BI_ACCW]< 1) return ("rs clw/csw");

    /* Assume happy at level 9 */
    if (depth <= 9) return ((cptr)NULL);


    /*** Level 10 - 19  ***/

    /* Must have "cure" */
    if ((borg_skill[BI_MAXCLEVEL] < 30) && borg_skill[BI_ACLW] + borg_skill[BI_ACSW] + borg_skill[BI_ACCW] < 2) return ("rs cure");

    /* Must have "teleport" */
    if (borg_skill[BI_ATELEPORT] + borg_skill[BI_AESCAPE] < 2) return ("rs tele&esc(1)");

    /* Assume happy at level 19 */
    if (depth <= 19) return ((cptr)NULL);


    /*** Level 20 - 35  ***/

    /* Must have "cure" */
    if ((borg_skill[BI_MAXCLEVEL] < 30) && borg_skill[BI_ACSW] + borg_skill[BI_ACCW] < 4) return ("rs cure");

    /* Must have "teleport" or Staff */
    if (borg_skill[BI_ATELEPORT] + borg_skill[BI_AESCAPE] < 4) return ("rs tele&esc(4)");

    /* Assume happy at level 44 */
    if (depth <= 35) return ((cptr)NULL);


    /*** Level 36 - 45  ***/

    /* Must have Scroll of Teleport (or good 2nd choice) */
    if (borg_skill[BI_ATELEPORT] + borg_skill[BI_ATELEPORTLVL] < 2) return ("rs teleport(1)");

    /* Assume happy at level 44 */
    if (depth <= 45) return ((cptr)NULL);


    /*** Level 46 - 64  ***/

    /* Assume happy at level 65 */
    if (depth <= 64) return ((cptr)NULL);

    /*** Level 65 - 99  ***/

    /* Must have "Heal" */
    if (borg_skill[BI_AHEAL] + borg_has[ROD_HEAL] + borg_skill[BI_AEZHEAL] < 1) return ("rs heal");

    /* Assume happy at level 99 */
    if (depth <= 99) return ((cptr)NULL);

    /*** Level 100  ***/

    /* Must have "Heal" */
    /* If I just got to dlevel 100 and low on heals, get out now. */
    if (borg_t - borg_began < 10 && borg_skill[BI_AEZHEAL] < 15) return ("rs *heal*");

    /* Assume happy */
    return ((cptr)NULL);
}


/*
 * Determine if the Borg meets the "minimum" requirements for a level
 */
static cptr borg_prepared_aux(int depth)
{
    if ( -1 == borg_ready_morgoth)
        borg_ready_morgoth = 0;
    if (borg_skill[BI_KING])
        {
            borg_ready_morgoth = 1;
            return ((cptr)NULL);
        }

    /* Always ready for the town */
    if (!depth) return ((cptr)NULL);


    /*** Essential Items for Level 1 ***/

    /* Require lite (any) */
    if (borg_skill[BI_CURLITE] < 1) return ("1 Lite");

    /* Require food */
    if (borg_skill[BI_FOOD] < 5) return ("5 Food");

    /* Usually ready for level 1 */
    if (depth <= 1) return ((cptr)NULL);


    /*** Essential Items for Level 2 ***/

    /* Require lite (radius two) */
    if (borg_skill[BI_CURLITE] < 2) return ("2 Lite");

    /* Require fuel */
    if (borg_skill[BI_AFUEL] < 5 && !borg_skill[BI_LIGHT]) return ("5 Fuel");

    /* Require recall */
    /* if (borg_skill[BI_RECALL] < 1) return ("1 recall"); */

    if (!borg_plays_risky)
    {
        /* Require 30 hp */
        if (borg_skill[BI_MAXHP] < 30) return ("30 hp");
    }

    /* Usually ready for level 2 */
    if (depth <= 2) return ((cptr)NULL);

    /*** Essential Items for Level 3 and 4 ***/

    if (!borg_plays_risky)
    {
        /* class specific requirement */
        switch (borg_class)
        {
            case CLASS_WARRIOR:
                if (borg_skill[BI_MAXHP] < 50) return ("50 hp");
                if (borg_skill[BI_MAXCLEVEL] < 4) return ("4 clevel");
                break;
            case CLASS_ROGUE:
                if (borg_skill[BI_MAXHP] < 50) return ("50 hp");
                if (borg_skill[BI_MAXCLEVEL] < 8) return ("8 clevel");
                break;
            case CLASS_PRIEST:
                if (borg_skill[BI_MAXHP] < 40) return ("40 hp");
                if (borg_skill[BI_MAXCLEVEL] < 9) return ("9 level");
                break;
            case CLASS_PALADIN:
                if (borg_skill[BI_MAXHP] < 50) return ("50 hp");
                if (borg_skill[BI_MAXCLEVEL] < 4) return ("4 clevel");
                break;
            case CLASS_RANGER:
                if (borg_skill[BI_MAXHP] < 50) return ("50 hp");
                if (borg_skill[BI_MAXCLEVEL] < 4) return ("4 clevel");
                break;
            case CLASS_MAGE:
                if (borg_skill[BI_MAXHP] < 60) return ("60 hp");
                if (borg_skill[BI_MAXCLEVEL] < 11) return ("11 clevel");
                break;
        }
    }

    /* Potions of Cure Serious Wounds */
    if ((borg_skill[BI_MAXCLEVEL] < 30) && borg_skill[BI_ACLW] + borg_skill[BI_ACSW] + borg_skill[BI_ACCW] < 2) return ("2 cure");
#if 0
    /* Scrolls of Identify */
    if (amt_ident < 2 && (borg_skill[BI_CDEPTH])) return ("2 ident");
#endif
    /* Usually ready for level 3 and 4 */
    if (depth <= 4) return ((cptr)NULL);


    /*** Essential Items for Level 5 to 9 ***/

    if (!borg_plays_risky)
    {
        /* class specific requirement */
        if (borg_skill[BI_CDEPTH])
        {
            switch (borg_class)
            {
                case CLASS_WARRIOR:
                    if (borg_skill[BI_MAXHP] < 60) return ("60 hp");
                    if (borg_skill[BI_MAXCLEVEL] < 6) return ("6 clevel");
                    break;
                case CLASS_ROGUE:
                    if (borg_skill[BI_MAXHP] < 60) return ("60 hp");
                    if (borg_skill[BI_MAXCLEVEL] < 10) return ("10 clevel");
                    break;
                case CLASS_PRIEST:
                    if (borg_skill[BI_MAXHP] < 60) return ("60 hp");
                    if (borg_skill[BI_MAXCLEVEL] < 15) return ("15 clevel");
                    break;
                case CLASS_PALADIN:
                    if (borg_skill[BI_MAXHP] < 60) return ("60 hp");
                    if (borg_skill[BI_MAXCLEVEL] < 6) return ("6 clevel");
                    break;
                case CLASS_RANGER:
                    if (borg_skill[BI_MAXHP] < 60) return ("60 hp");
                    if (borg_skill[BI_MAXCLEVEL] < 6) return ("6 clevel");
                    break;
                case CLASS_MAGE:
                    if (borg_skill[BI_MAXHP] < 80) return ("80 hp");
                    if (borg_skill[BI_MAXCLEVEL] < 15) return ("15 level");
                    break;
            }
        }
    }
#if 0
    /* Scrolls of Identify */
    if (amt_ident < 5 && (borg_skill[BI_CDEPTH])) return ("5 idents");
#endif
    /* Potions of Cure Serious/Critical Wounds */
    if ((borg_skill[BI_MAXCLEVEL] < 30) && borg_skill[BI_ACLW] + borg_skill[BI_ACSW] + borg_skill[BI_ACCW] < 2) return ("2 cures");

    /* Scrolls of Word of Recall */
    if (borg_skill[BI_RECALL] < 1) return ("1 recall");

    /* Usually ready for level 5 to 9 */
    if (depth <= 9) return ((cptr)NULL);


    /*** Essential Items for Level 10 to 19 ***/


    /* Escape or Teleport */
    if (borg_skill[BI_ATELEPORT] + borg_skill[BI_AESCAPE] < 2) return ("2 tele&esc");

    if (!borg_plays_risky)
    {
        /* class specific requirement */
        switch (borg_class)
        {
            case CLASS_WARRIOR:
                if (borg_skill[BI_MAXCLEVEL] < (depth - 4) && depth <= 19)
                    return ("dlevel - 4 >= clevel");
                break;
            case CLASS_ROGUE:
                if (borg_skill[BI_MAXCLEVEL] < depth && depth <= 19) return ("dlevel >= clevel" );
                break;
            case CLASS_PRIEST:
                if (borg_skill[BI_MAXCLEVEL] < depth && depth <= 19) return ("dlevel >= clevel" );
                break;
            case CLASS_PALADIN:
                if (borg_skill[BI_MAXCLEVEL] < depth && depth <= 19) return ("dlevel >= clevel" );
                break;
            case CLASS_RANGER:
                if (borg_skill[BI_MAXCLEVEL] < depth && depth <= 19) return ("dlevel >= clevel" );
                break;
            case CLASS_MAGE:
                if (borg_skill[BI_MAXCLEVEL] < (depth + 5) && borg_skill[BI_MAXCLEVEL] <= 28)
                    return ("dlevel + 5 > = clevel" );
                break;
        }
    }
#if 0
    /* Identify */
    if (amt_ident < 10) return ("ident10");
#endif
    /* Potions of Cure Critical Wounds */
    if ((borg_skill[BI_MAXCLEVEL] < 30) && borg_skill[BI_ACCW] < 3) return ("cure crit3");

    /* See invisible */
    /* or telepathy */
    if ((!borg_skill[BI_SINV] && !borg_skill[BI_DINV] &&
         !borg_skill[BI_ESP])) return ("See Invis : ESP");

    /* Usually ready for level 10 to 19 */
    if (depth <= 19) return ((cptr)NULL);


    /*** Essential Items for Level 20 ***/


    /* Free action */
    if (!borg_skill[BI_FRACT]) return ("FA");

    /* ready for level 20 */
    if (depth <= 20) return ((cptr)NULL);


    /*** Essential Items for Level 25 ***/

    /* must have fire + 2 other basic resists */
    if (!borg_skill[BI_SRFIRE]) return ("RF");
    {
        int basics = borg_skill[BI_RACID] + borg_skill[BI_RCOLD] + borg_skill[BI_RELEC];

        if (basics < 2) return ("basic resist2");
    }
    /* have some minimal stats */
    if (borg_stat[A_STR] < 7) return ("low STR");

    if (p_ptr->class->spell_book == TV_MAGIC_BOOK)
    {
        if (borg_stat[A_INT] < 7) return ("low INT");
    }
    if (p_ptr->class->spell_book == TV_PRAYER_BOOK)
    {
        if (borg_stat[A_WIS] < 7) return ("low WIS");
    }
    if (borg_stat[A_DEX] < 7) return ("low DEX");
    if (borg_stat[A_CON] < 7) return ("low CON");

    if (!borg_plays_risky)
    {
        /* class specific requirement */
        switch (borg_class)
        {
            case CLASS_WARRIOR:
                if (borg_skill[BI_MAXCLEVEL] < (depth + 5) && borg_skill[BI_MAXCLEVEL] <= 38)
                    return ("dlevel + 5 >= clevel" );
                break;
            case CLASS_ROGUE:
                if (borg_skill[BI_MAXCLEVEL] < (depth + 10) && borg_skill[BI_MAXCLEVEL] <= 43)
                    return ("dlevel + 10 >= clevel" );
                break;
            case CLASS_PRIEST:
                if (borg_skill[BI_MAXCLEVEL] < (depth + 13) && borg_skill[BI_MAXCLEVEL] <= 46)
                    return ("dlevel + 13 >= clevel" );
                break;
            case CLASS_PALADIN:
                if (borg_skill[BI_MAXCLEVEL] < (depth + 7) && borg_skill[BI_MAXCLEVEL] <= 40)
                    return ("dlevel + 7 >= clevel" );
                break;
            case CLASS_RANGER:
                if (borg_skill[BI_MAXCLEVEL] < (depth + 8) && borg_skill[BI_MAXCLEVEL] <= 41 && borg_skill[BI_MAXCLEVEL] > 28)
                    return ("dlevel + 8 >= clevel" );
                break;
            case CLASS_MAGE:
                if (borg_skill[BI_MAXCLEVEL] < (depth + 8) && borg_skill[BI_MAXCLEVEL] <= 38)
                    return ("dlevel + 8 >= clevel" );
                if (((borg_skill[BI_MAXCLEVEL]-38) * 2 + 30) < depth &&
                    borg_skill[BI_MAXCLEVEL] <= 44 &&
                    borg_skill[BI_MAXCLEVEL] > 38)
                    return ("(clevel-38)*2+30 < dlevel" );
                break;
        }
    }

    /* Ready for level 25 */
    if (depth <= 25) return ((cptr)NULL);


/*** Essential Items for Level 25 to 39 ***/

    /* All Basic resistance*/
    if (!borg_skill[BI_SRCOLD]) return ("RC");
    if (!borg_skill[BI_SRELEC]) return ("RE");
    if (!borg_skill[BI_SRACID]) return ("RA");

    /* Escape and Teleport */
    if (borg_skill[BI_ATELEPORT] + borg_skill[BI_AESCAPE] < 6) return ("tell&esc6");

    /* Cure Critical Wounds */
    if ((borg_skill[BI_MAXCLEVEL] < 30) && (borg_skill[BI_ACCW] + borg_skill[BI_ACSW]) < 10) return ("cure10");

    /* Ready for level 33 */
    if (depth <= 33) return ((cptr)NULL);

    /* Minimal level */
    if (borg_skill[BI_MAXCLEVEL] < 40  && !borg_plays_risky) return ("level 40");

    /* Usually ready for level 20 to 39 */
    if (depth <= 39) return ((cptr)NULL);



/*** Essential Items for Level 40 to 45 ***/

    /* Resist */
    if (!borg_skill[BI_SRPOIS]) return ("RPois");
    if (!borg_skill[BI_SRCONF])  return ("RConf");

    if (borg_stat[A_STR] < 16) return ("low STR");

    if (p_ptr->class->spell_book == TV_MAGIC_BOOK)
    {
        if (borg_stat[A_INT] < 16) return ("low INT");
    }
    if (p_ptr->class->spell_book == TV_PRAYER_BOOK)
    {
        if (borg_stat[A_WIS] < 16) return ("low WIS");
    }
    if (borg_stat[A_DEX] < 16) return ("low DEX");
    if (borg_stat[A_CON] < 16) return ("low CON");


	/* Ok to continue */
    if (depth <= 45) return ((cptr)NULL);


/*** Essential Items for Level 46 to 55 ***/

    /*  Must have +5 speed after level 46 */
    if (borg_skill[BI_SPEED] < 115) return ("+5 speed");

    /* Potions of heal */
    if (borg_skill[BI_AHEAL] < 1 && (borg_skill[BI_AEZHEAL] < 1) ) return ("1heal");

    if (!borg_plays_risky)
    {
        /* Minimal hitpoints */
        if (borg_skill[BI_MAXHP] < 500) return ("HP 500");
    }

    /* High stats XXX XXX XXX */
    if (borg_stat[A_STR] < 18+40) return ("low STR");

    if (p_ptr->class->spell_book == TV_MAGIC_BOOK)
    {
        if (borg_stat[A_INT] < 18+100) return ("low INT");
    }
    if (p_ptr->class->spell_book == TV_PRAYER_BOOK)
    {
        if (borg_stat[A_WIS] < 18+100) return ("low WIS");
    }
    if (borg_stat[A_DEX] < 18+60) return ("low DEX");
    if (borg_stat[A_CON] < 18+60) return ("low CON");

    /* Hold Life */
    if ((!borg_skill[BI_HLIFE] && !weapon_swap_hold_life &&
        !armour_swap_hold_life) && (borg_skill[BI_MAXCLEVEL] < 50) ) return ("hold life");

    /* Usually ready for level 46 to 55 */
    if (depth <= 55) return ((cptr)NULL);

/*** Essential Items for Level 55 to 59 ***/

    /* Potions of heal */
    if (borg_skill[BI_AHEAL] < 2 && borg_skill[BI_AEZHEAL] < 1) return ("2heal");

    /* Resists */
    if (!borg_skill[BI_SRBLIND]) return ("RBlind");

    /* Must have resist nether */
/*    if (!borg_plays_risky && !borg_skill[BI_SRNTHR]) return ("RNeth"); */


    /* Telepathy, better have it by now */
    if (!borg_skill[BI_ESP]) return ("ESP");

    /* Usually ready for level 55 to 59 */
    if (depth <= 59) return ((cptr)NULL);



/*** Essential Items for Level 61 to 80 ***/

    /* Must have +10 speed */
    if (borg_skill[BI_SPEED] < 120) return ("+10 speed");


    /* Resists */
    if (!borg_skill[BI_SRKAOS]) return ("RChaos");
    if (!borg_skill[BI_SRDIS]) return ("RDisen");

    /* Usually ready for level 61 to 80 */
    if (depth <= 80) return ((cptr)NULL);

/*** Essential Items for Level 81-85 ***/
    /* Minimal Speed */
    if (borg_skill[BI_SPEED] < 130) return ("+20 Speed");

    /* Usually ready for level 81 to 85 */
    if (depth <= 85) return ((cptr)NULL);


/*** Essential Items for Level 86-99 ***/


    /* Usually ready for level 86 to 99 */
    if (depth <= 99) return ((cptr)NULL);

/*** Essential Items for Level 100 ***/

    /* must have lots of restore mana to go after MORGOTH */
    if (!borg_skill[BI_KING])
    {
        if ((borg_skill[BI_MAXSP] > 100) && (borg_has[POTION_RES_MANA] < 15)) return ("10ResMana");

        /* must have lots of heal */
        if (borg_has[POTION_HEAL] < 5) return ("5Heal");

        /* must have lots of ez-heal */
        if (borg_skill[BI_AEZHEAL] < 15) return ("15EZHeal");

        /* must have lots of speed */
        if (borg_skill[BI_ASPEED] < 10) return ("10Speed");

      }

    /* Its good to be the king */
    if (depth <= 127) return ((cptr)NULL);

    /* all bases covered */
    return ((cptr)NULL);
}

/* buffer for borg_prepared mesage
 */
#define MAX_REASON 1024
static char borg_prepared_buffer[MAX_REASON];

/*
 * Determine if the Borg is "prepared" for the given level
 *
 * This routine does not help him decide how to get ready for the
 * given level, so it must work closely with "borg_power()".
 *
 * Note that we ignore any "town fear", and we allow fear of one
 * level up to and including the relevant depth.
 *
 * This now returns a string with the reason you are not prepared.
 *
 */
cptr borg_prepared(int depth)
{
    cptr reason;

    /* Town and First level */
    if (depth == 1) return ((cptr)NULL);

    /* Not prepared if I need to restock */
    if ((reason = borg_restock(depth)))	return (reason);

	/*** Require his Clevel to be greater than or equal to Depth */
	if (borg_skill[BI_MAXCLEVEL] < depth && borg_skill[BI_MAXCLEVEL] < 50) return ("Clevel < depth");

	/* Must meet minimal requirements */
	if (depth <= 99)
	{
   	    if ((reason = borg_prepared_aux(depth))) return (reason);
	}

	/* Not if No_Deeper is set */
	if (depth >= borg_no_deeper)
	{
       	strnfmt(borg_prepared_buffer, MAX_REASON, "No deeper %d.", borg_no_deeper);
       	return (borg_prepared_buffer);
	}


    /* Once Morgoth is dead */
    if (borg_skill[BI_KING])
    {
        return ((cptr)NULL);
    }

    /* Always okay from town */
    if (!borg_skill[BI_CDEPTH])	return (reason);

	/* Scum on depth 80-81 for some *heal* potions */
	if (depth >= 82 && (num_ezheal < 10 && borg_skill[BI_AEZHEAL] < 10))
	{
        /* Must know exact number of Potions  in home */
        borg_notice_home(NULL, FALSE);

       	strnfmt(borg_prepared_buffer, MAX_REASON, "Scumming *Heal* potions (%d to go).", 10-num_ezheal);
       	return (borg_prepared_buffer);
	}

    /* Scum on depth 80-81 for lots of *Heal* potions preparatory for Endgame */
    if (depth >= 82 && borg_skill[BI_MAXDEPTH] >= 97)
    {
        /* Must know exact number of Potions  in home */
        borg_notice_home(NULL, FALSE);

        /* Scum for 30*/
        if (num_ezheal_true + borg_skill[BI_AEZHEAL] < 30)
        {
       		strnfmt(borg_prepared_buffer, MAX_REASON, "Scumming *Heal* potions (%d to go).", 30-
       		        (num_ezheal_true + borg_skill[BI_AEZHEAL]));
       		return (borg_prepared_buffer);
        }

        /* Return to town to get your stock from the home*/
        if (num_ezheal_true + borg_skill[BI_AEZHEAL] >= 30 && /* Enough combined EZ_HEALS */
        	num_ezheal_true >=1 && borg_skill[BI_MAXDEPTH] >= 99) /* Still some sitting in the house */
        {
       		strnfmt(borg_prepared_buffer, MAX_REASON, "Collect from house (%d potions).", num_ezheal_true);
       		return (borg_prepared_buffer);
        }
	}

    /* Check to make sure the borg does not go below where 3 living */
    /* uniques are. */
    if (borg_skill[BI_MAXDEPTH] <= 98)
    {
          monster_race *r_ptr = &r_info[borg_living_unique_index];

		/* are too many uniques alive */
        if (borg_numb_live_unique < 3 || borg_plays_risky ||
        	borg_skill[BI_CLEVEL] == 50 || borg_kills_uniques == FALSE) return ((cptr)NULL);

		/* Check for the dlevel of the unique */
		if (depth < borg_unique_depth) return ((cptr)NULL);

       	/* To avoid double calls to format() */
 	   	/* Reset our description for not diving */
       	strnfmt(borg_prepared_buffer, MAX_REASON, "Must kill %s.", r_ptr->name);
       	return (borg_prepared_buffer);

    }
    else if (borg_skill[BI_MAXDEPTH] >= 98 || depth >= 98)
      /* check to make sure the borg does not go to level 100 */
      /* unless all the uniques are dead. */
    {
          monster_race *r_ptr;

		  /* Access the living unique obtained from borg_update() */
          r_ptr = &r_info[borg_living_unique_index];

			/* -1 is unknown. */
			borg_ready_morgoth = -1;

          if (borg_numb_live_unique < 1 ||
              borg_living_unique_index == 547) /* Morgoth */
          {
            	if (depth >= 99) borg_ready_morgoth = 1;
            	return ((cptr)NULL);
          }

          /* Under special cases allow the borg to dive to 99 then quickly
           * get his butt to dlevel 98
           */
          if (borg_skill[BI_MAXDEPTH] == 99 && depth <= 98 &&
              (borg_prayer_legal_fail(4, 3, 20) || /* Teleport Level */
               borg_spell_legal_fail(6, 2, 20) || /* Teleport Level */
               borg_skill[BI_ATELEPORTLVL] >= 1)) /* Teleport Level scroll */
          {
              return ((cptr)NULL);
          }

      	/* To avoid double calls to format() */
      	strnfmt(borg_prepared_buffer, MAX_REASON, "%s still alive!", r_ptr->name);
      	return (borg_prepared_buffer);

    }

}

/*
 * Initialize this file
 */
void borg_init_4(void)
{
    /* Do nothing? */
}



#ifdef MACINTOSH
static int HACK = 0;
#endif
#endif /* ALLOW_BORG */