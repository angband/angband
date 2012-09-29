/* File: wizard.c */ 

/* Purpose: Version history and info, and wizard mode debugging aids. */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "angband.h"




/*
 * Wizard routine for gaining on stats                  -RAK-    
 */
static void change_character()
{
    register int          tmp_val;
    register int32        tmp_lval;

    vtype                 tmp_str;

    prt("(3 - 118) Strength     = ", 0, 0);
    if (!askfor(tmp_str, 3)) return;

    tmp_val = atoi(tmp_str);
    if ((tmp_val > 2) && (tmp_val < 119)) {
	p_ptr->max_stat[A_STR] = tmp_val;
	(void)res_stat(A_STR);
    }

    prt("(3 - 118) Intelligence = ", 0, 0);
    if (!askfor(tmp_str, 3)) return;

    tmp_val = atoi(tmp_str);
    if ((tmp_val > 2) && (tmp_val < 119)) {
	p_ptr->max_stat[A_INT] = tmp_val;
	(void)res_stat(A_INT);
    }

    prt("(3 - 118) Wisdom       = ", 0, 0);
    if (!askfor(tmp_str, 3)) return;

    tmp_val = atoi(tmp_str);
    if ((tmp_val > 2) && (tmp_val < 119)) {
	p_ptr->max_stat[A_WIS] = tmp_val;
	(void)res_stat(A_WIS);
    }

    prt("(3 - 118) Dexterity    = ", 0, 0);
    if (!askfor(tmp_str, 3)) return;

    tmp_val = atoi(tmp_str);
    if ((tmp_val > 2) && (tmp_val < 119)) {
	p_ptr->max_stat[A_DEX] = tmp_val;
	(void)res_stat(A_DEX);
    }

    prt("(3 - 118) Constitution = ", 0, 0);
    if (!askfor(tmp_str, 3)) return;

    tmp_val = atoi(tmp_str);
    if ((tmp_val > 2) && (tmp_val < 119)) {
	p_ptr->max_stat[A_CON] = tmp_val;
	(void)res_stat(A_CON);
    }

    prt("(3 - 118) Charisma     = ", 0, 0);
    if (!askfor(tmp_str, 3)) return;

    tmp_val = atoi(tmp_str);
    if ((tmp_val > 2) && (tmp_val < 119)) {
	p_ptr->max_stat[A_CHR] = tmp_val;
	(void)res_stat(A_CHR);
    }

    prt("(1 - 32767) Max Hit Points = ", 0, 0);
    if (!askfor(tmp_str, 5)) return;

    tmp_val = atoi(tmp_str);
    if (tmp_str[0] && (tmp_val > 0) && (tmp_val <= MAX_SHORT)) {
	p_ptr->mhp = tmp_val;
	p_ptr->chp = tmp_val;
	p_ptr->chp_frac = 0;
	prt_mhp();
	prt_chp();
    }

    prt("(0 - 32767) Max Mana = ", 0, 0);
    if (!askfor(tmp_str, 5)) return;

    tmp_val = atoi(tmp_str);
    if (tmp_str[0] && (tmp_val >= 0) && (tmp_val <= MAX_SHORT)) {
	p_ptr->mana = tmp_val;
	p_ptr->cmana = tmp_val;
	p_ptr->cmana_frac = 0;
	prt_cmana();
    }

    (void)sprintf(tmp_str, "Current=%ld  Gold = ", (long)p_ptr->au);
    prt(tmp_str, 0, 0);
    if (!askfor(tmp_str, 7)) return;

    tmp_lval = atol(tmp_str);
    if (tmp_str[0] && (tmp_lval >= 0)) {
	p_ptr->au = tmp_lval;
	prt_gold();
    }

    (void)sprintf(tmp_str, "Current=%ld  Max Exp = ", (long)p_ptr->max_exp);
    prt(tmp_str, 0, 0);
    if (!askfor(tmp_str, 7)) return;

    tmp_lval = atol(tmp_str);
    if (tmp_lval > -1 && (*tmp_str != '\0')) {
	p_ptr->max_exp = tmp_lval;
	prt_experience();
    }

    (void)sprintf(tmp_str, "Current=%d  (0-200) Searching = ", p_ptr->srh);
    prt(tmp_str, 0, 0);
    if (!askfor(tmp_str, 3)) return;

    tmp_val = atoi(tmp_str);
    if ((tmp_val >= 0) && (tmp_val < 201) && (*tmp_str != '\0')) {
	p_ptr->srh = tmp_val;
    }

    (void)sprintf(tmp_str, "Current=%d  (-1-18) Stealth = ", p_ptr->stl);
    prt(tmp_str, 0, 0);
    if (!askfor(tmp_str, 3)) return;

    tmp_val = atoi(tmp_str);
    if ((tmp_val >= -1) && (tmp_val < 19) && (*tmp_str != '\0')) {
	p_ptr->stl = tmp_val;
    }

    (void)sprintf(tmp_str, "Current=%d  (0-200) Disarming = ", p_ptr->disarm);
    prt(tmp_str, 0, 0);
    if (!askfor(tmp_str, 3)) return;

    tmp_val = atoi(tmp_str);
    if ((tmp_val >= 0) && (tmp_val < 201) && (*tmp_str != '\0')) {
	p_ptr->disarm = tmp_val;
    }

    (void)sprintf(tmp_str, "Current=%d  (0-100) Save = ", p_ptr->save);
    prt(tmp_str, 0, 0);
    if (!askfor(tmp_str, 3)) return;

    tmp_val = atoi(tmp_str);
    if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0')) {
	p_ptr->save = tmp_val;
    }

    (void)sprintf(tmp_str, "Current=%d  (0-200) Base to hit = ", p_ptr->bth);
    prt(tmp_str, 0, 0);
    if (!askfor(tmp_str, 3)) return;

    tmp_val = atoi(tmp_str);
    if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0')) {
	p_ptr->bth = tmp_val;
    }

    (void)sprintf(tmp_str, "Current=%d  (0-200) Bows/Throwing = ", p_ptr->bthb);
    prt(tmp_str, 0, 0);
    if (!askfor(tmp_str, 3)) return;

    tmp_val = atoi(tmp_str);
    if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0')) {
	p_ptr->bthb = tmp_val;
    }

    (void)sprintf(tmp_str, "Current=%d  Weight = ", p_ptr->wt);
    prt(tmp_str, 0, 0);
    if (!askfor(tmp_str, 3)) return;

    tmp_val = atoi(tmp_str);
    if (tmp_val > -1 && (*tmp_str != '\0')) {
	p_ptr->wt = tmp_val;
    }

    while (get_com("Alter base speed? (+/-)", tmp_str)) {
	if (*tmp_str == '+')
	    change_speed(-1);
	else if (*tmp_str == '-')
	    change_speed(1);
	else
	    break;
	prt_speed();
    }
}


/*
 * Wizard routine for creating objects		-RAK-	 
 *
 * Note that wizards can nuke artifacts / stairs this way
 */

static void wizard_create_aux1(inven_type *i_ptr)
{
    int                  i, j, k_idx;
    int			 tval = 0;
    int			 page, num;
    char                 ch;
    int			 option[24];
    char                 tmp_str[100];


    prt("What type of item?", 0, 0);
    prt("[W]eapon, [A]rmour, [O]thers.", 1, 0);
    if (!get_com(NULL, &ch)) return;

    switch (ch) {

      case 'W': case 'w':
	prt("What type of Weapon?", 0, 0);
	prt("[S]word, [H]afted, [P]olearm, [B]ow, [A]mmo.", 1, 0);
	if (!get_com(NULL, &ch)) return;

	switch (ch) {
	  case 'S': case 's':
	    tval = TV_SWORD;
	    break;

	  case 'H': case 'h':
	    tval = TV_HAFTED;
	    break;

	  case 'P': case 'p':
	    tval = TV_POLEARM;
	    break;

	  case 'B': case 'b':
	    tval = TV_BOW;
	    break;

	  case 'A': case 'a':
	    prt("What type of Ammo?", 0, 0);
	    prt("[A]rrow, [B]olt, [P]ebble.", 1, 0);
	    if (!get_com(NULL, &ch)) return;

	    switch (ch) {
	      case 'A': case 'a':
		tval = TV_ARROW;
		break;
	      case 'B': case 'b':
		tval = TV_BOLT;
		break;
	      case 'P': case 'p':
		tval = TV_SHOT;
		break;
	      default:
		break;
	    }
	    break;

	  default:
	    return;
	}
	break;

      case 'A': case 'a':
	prt("What type of Armour?", 0, 0);
	prt("[A]rmour, [G]loves, [B]oots, [S]hields, [H]elms, [C]loaks.", 1, 0);
	if (!get_com(NULL, &ch)) return;

	switch (ch) {

	  case 'S': case 's':
	    tval = TV_SHIELD;
	    break;

	  case 'H': case 'h':
	    tval = TV_HELM;
	    break;

	  case 'G': case 'g':
	    tval = TV_GLOVES;
	    break;

	  case 'B': case 'b':
	    tval = TV_BOOTS;
	    break;

	  case 'C': case 'c':
	    tval = TV_CLOAK;
	    break;

	  case 'A': case 'a':
	    prt("What type of Armour?    : ", 0, 0);
	    prt("[D]ragon Scale, [H]ard armour, [S]oft armour.", 1, 0);
	    if (!get_com(NULL, &ch)) return;

	    switch (ch) {
	      case 'D': case 'd':
		tval = TV_DRAG_ARMOR;
		break;
	      case 'H': case 'h':
		tval = TV_HARD_ARMOR;
		break;
	      case 'S': case 's':
		tval = TV_SOFT_ARMOR;
		break;
	      default:
		break;
	    }
	    break;

	  default:
	    return;
	}
	break;

      case 'O': case 'o':
	prt("What type of Object?", 0, 0);
	prt("[R]ing, [P]otion, [W]and, [S]croll, [B]ook, [A]mulet, [T]ool.", 1, 0);
	if (!get_com(NULL, &ch)) return;

	switch (ch) {

	  case 'R': case 'r':
	    tval = TV_RING;
	    break;

	  case 'P': case 'p':
	    tval = TV_POTION;
	    break;

	  case 'S': case 's':
	    tval = TV_SCROLL;
	    break;

	  case 'A': case 'a':
	    tval = TV_AMULET;
	    break;

	  case 'W': case 'w':
	    prt("Wand, Staff or Rod?", 0, 0);
	    prt("[W]and, [S]taff, [R]od.", 1, 0);
	    if (!get_com(NULL, &ch)) return;
	    switch (ch) {
	      case 'W': case 'w':
		tval = TV_WAND;
		break;
	      case 'S': case 's':
		tval = TV_STAFF;
		break;
	      case 'R': case 'r':
		tval = TV_ROD;
		break;
	      default:
		return;
	    }
	    break;

	  case 'B': case 'b':
	    prt("Spellbook or Prayerbook?", 0, 0);
	    prt("[S]pellbook, [P]rayerbook.", 1, 0);
	    if (!get_com(NULL, &ch)) return;

	    switch (ch) {
	      case 'P': case 'p':
		tval = TV_PRAYER_BOOK;
		break;
	      case 'S': case 's':
		tval = TV_MAGIC_BOOK;
		break;
	      default:
		return;
	    }
	    break;

	  case 'T': case 't':
	    prt("Which Type of Tool?", 0, 0);
	    prt("[S]pike, [D]igger, [C]hest, [L]ight, [F]ood, [O]il.", 1, 0);
	    if (!get_com(NULL, &ch)) return;

	    switch (ch) {
	      case 'S': case 's':
		tval = TV_SPIKE;
		break;
	      case 'D': case 'd':
		tval = TV_DIGGING;
		break;
	      case 'C': case 'c':
		tval = TV_CHEST;
		break;
	      case 'L': case 'l':
		tval = TV_LITE;
		break;
	      case 'F': case 'f':
		tval = TV_FOOD;
		break;
	      case 'O': case 'o':
		tval = TV_FLASK;
		break;
	      default:
		return;
	    }
	    break;

	  default:
	    return;
	}
	break;

      default:
	return;
    }


    /*** Base object type chosen ***/

    /* Show pages until a legal value is chosen for "k" */
    for (page = i = 0, k_idx = -1; k_idx < 0; ) {

	/* Clear the screen */
	clear_screen();

	/* Show up to twenty options at a time.  Hack -- wrap when legal */
	for (num = 0; (num < 20) && (page || (i < MAX_K_IDX)); i++) {

	    /* Hack -- reset on "overflow" */
	    if (i == MAX_K_IDX) i = 0;

	    /* Save (and count) this option */
	    if (k_list[i].tval == tval) option[num++] = i;
	}

	/* Notice a full page */
	if (num >= 20) page++;

	/* List the options extracted above */
	for (j = 0; j < num; j++) {

	    char /* p1 = '(', */ p2 = ')';
	    int opt = option[j];

	    sprintf(tmp_str, "  %c%c [%d] %s",
		   'a' + j, p2, opt, k_list[opt].name);

	    prt(tmp_str, j+1, 0);
	}

	/* Build a prompt */
	sprintf(tmp_str, "Choose an item (%sESC to cancel): ",
		(num<20) ? "" : "Space for more choices, ");

	/* Get a command */
	if (!get_com(tmp_str, &ch)) return;

	/* Analyze the choice */
	j = (ch - 'a');

	/* Process "Legal" Answers */
	if ((j >= 0) && (j < num)) k_idx = option[j];
    }


    /* Build an object using that template */
    invcopy(i_ptr, k_idx);
}


static void wizard_create_aux2(inven_type *i_ptr)
{
    int                  tmp_val;
    int32                tmp_lval;

    int			 i, j, page, num;

    char                 ch;

    int			 option[24];
    char                 tmp_str[100];


    msg_print("Now you may specify extra information about the object.");
    msg_print("Hit ESCAPE at any time to skip the remaining questions.");
    msg_print("Hit RETURN to accept the default response for any question.");

    prt("Number of items: ", 0, 0);
    if (!askfor(tmp_str, 5)) return;
    tmp_val = atoi(tmp_str);
    if (tmp_val) i_ptr->number = tmp_val;

    prt("Weight of item: ", 0, 0);
    if (!askfor(tmp_str, 5)) return;
    tmp_val = atoi(tmp_str);
    if (tmp_val) i_ptr->weight = tmp_val;

    if ((i_ptr->tval == TV_SWORD) ||
	(i_ptr->tval == TV_HAFTED) ||
	(i_ptr->tval == TV_POLEARM) ||
	(i_ptr->tval == TV_ARROW) ||
	(i_ptr->tval == TV_BOLT) ||
	(i_ptr->tval == TV_SHOT) ||
	(i_ptr->tval == TV_DIGGING)) {

	prt("Damage (dice): ", 0, 0);
	if (!askfor(tmp_str, 3)) return;
	tmp_val = atoi(tmp_str);
	if (tmp_val) i_ptr->damage[0] = tmp_val;

	prt("Damage (sides): ", 0, 0);
	if (!askfor(tmp_str, 3)) return;
	tmp_val = atoi(tmp_str);
	if (tmp_val) i_ptr->damage[1] = tmp_val;
    }

    prt("To hit modifier: ", 0, 0);
    if (!askfor(tmp_str, 3)) return;
    tmp_val = atoi(tmp_str);
    if (tmp_val) i_ptr->tohit = tmp_val;

    prt("To dam modifier: ", 0, 0);
    if (!askfor(tmp_str, 3)) return;
    tmp_val = atoi(tmp_str);
    if (tmp_val) i_ptr->todam = tmp_val;

    /* Extra Armor Info */
    if ((i_ptr->tval == TV_DRAG_ARMOR) ||
	(i_ptr->tval == TV_HARD_ARMOR) ||
	(i_ptr->tval == TV_SOFT_ARMOR) ||
	(i_ptr->tval == TV_HELM) ||
	(i_ptr->tval == TV_CLOAK) ||
	(i_ptr->tval == TV_BOOTS) ||
	(i_ptr->tval == TV_GLOVES) ||
	(i_ptr->tval == TV_SHIELD)) {

	prt("Base AC: ", 0, 0);
	if (!askfor(tmp_str, 3)) return;
	tmp_val = atoi(tmp_str);
	if (tmp_val) i_ptr->ac = tmp_val;
    }

    prt("To AC modifier: ", 0, 0);
    if (!askfor(tmp_str, 3)) return;
    tmp_val = atoi(tmp_str);
    if (tmp_val) i_ptr->toac = tmp_val;

    prt("Special 'pval' setting: ", 0, 0);
    if (!askfor(tmp_str, 5)) return;
    tmp_val = atoi(tmp_str);
    if (tmp_val) i_ptr->pval = tmp_val;


    /* Only do TRN_* flags for wearable objects */

    if (wearable_p(i_ptr)) {

	if ((i_ptr->tval == TV_SWORD) ||
	    (i_ptr->tval == TV_HAFTED) ||
	    (i_ptr->tval == TV_POLEARM) ||
	    (i_ptr->tval == TV_ARROW) ||
	    (i_ptr->tval == TV_BOLT) ||
	    (i_ptr->tval == TV_SHOT) ||
	    (i_ptr->tval == TV_DIGGING)) {

	    if (!get_com("Slay Something?", &ch)) return;
	    if (ch == 'y' || ch == 'Y') {

		if (!get_com("Slay Evil? ", &ch)) return;
		if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_SLAY_EVIL;

		if (!get_com("Slay Animal? ", &ch)) return;
		if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_SLAY_ANIMAL;

		if (!get_com("Slay Undead? ", &ch)) return;
		if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_SLAY_UNDEAD;

		if (!get_com("Slay Giant? ", &ch)) return;
		if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_SLAY_GIANT;

		if (!get_com("Slay Demon? ", &ch)) return;
		if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_SLAY_DEMON;

		if (!get_com("Slay Troll? ", &ch)) return;
		if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_SLAY_TROLL;

		if (!get_com("Slay Orc? ", &ch)) return;
		if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_SLAY_ORC;

		if (!get_com("Slay Dragon? ", &ch)) return;
		if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_SLAY_DRAGON;

		if (!get_com("Execute Dragon? ", &ch)) return;
		if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_KILL_DRAGON;
	    }

	    if (!get_com("Frost Brand? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_BRAND_COLD;

	    if (!get_com("Fire Brand? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_BRAND_FIRE;

	    if (!get_com("Lightning Brand? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_BRAND_ELEC;

	    if (!get_com("Earthquake Brand? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_IMPACT;
	}

	if (!get_com("Affect Any Stat (via 'pval')? ", &ch)) return;
	if (ch == 'y' || ch == 'Y') {

	    if (!get_com("Affect Strength (via 'pval')? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_STR;

	    if (!get_com("Affect Intelligence (via 'pval')? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_INT;

	    if (!get_com("Affect Wisdom (via 'pval')? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_WIS;

	    if (!get_com("Affect Dexterity (via 'pval')? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_DEX;

	    if (!get_com("Affect Constitution (via 'pval')? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_CON;

	    if (!get_com("Affect Charisma (via 'pval')? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_CHR;
	}

	if (!get_com("Affect Anything Else (via 'pval')? ", &ch)) return;
	if (ch == 'y' || ch == 'Y') {

	    if (!get_com("Affect Search (via 'pval')? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_SEARCH;

	    if (!get_com("Affect Stealth (via 'pval')? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_STEALTH;

	    if (!get_com("Affect Speed?  (via 'pval')", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_SPEED;

	    if (!get_com("Affect Attack Speed?  (via 'pval')", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_ATTACK_SPD;

	    if (!get_com("Affect Tunneling (via 'pval')? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_TUNNEL;

	    if (!get_com("Affect Infra-vision (via 'pval')? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags1 |= TR1_INFRA;
	}          

	if (!get_com("Resist Anything? ", &ch)) return;
	if (ch == 'y' || ch == 'Y') {

	    if (!get_com("Resist Acid? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_RES_ACID;

	    if (!get_com("Resist Lightning? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_RES_ELEC;

	    if (!get_com("Resist Fire? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_RES_FIRE;

	    if (!get_com("Resist Cold? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_RES_COLD;

	    if (!get_com("Resist Poison? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_RES_POIS;

	    if (!get_com("Resist Confusion? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_RES_CONF;

	    if (!get_com("Resist Sound? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_RES_SOUND;

	    if (!get_com("Resist Light? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_RES_LITE;

	    if (!get_com("Resist Dark? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_RES_DARK;

	    if (!get_com("Resist Chaos? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_RES_CHAOS;

	    if (!get_com("Resist Disenchantment? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_RES_DISEN;

	    if (!get_com("Resist Shards? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_RES_SHARDS;

	    if (!get_com("Resist Nexus? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_RES_NEXUS;

	    if (!get_com("Resist Nether? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_RES_NETHER;

	    if (!get_com("Resist Blindness? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_RES_BLIND;
	}     

	if (!get_com("Immune to Anything? ", &ch)) return;
	if (ch == 'y' || ch == 'Y') {

	    if (!get_com("Immune to Acid? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_IM_ACID;

	    if (!get_com("Immune to Lightning? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_IM_ELEC;

	    if (!get_com("Immune to Fire? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_IM_FIRE;

	    if (!get_com("Immune to Cold? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_IM_COLD;

	    if (!get_com("Immune to Poison? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_IM_POIS;
	}

	if (!get_com("Sustain Any Stats? ", &ch)) return;
	if (ch == 'y' || ch == 'Y') {

	    if (!get_com("Sustain strength? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_SUST_STR;

	    if (!get_com("Sustain intelligence? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_SUST_INT;

	    if (!get_com("Sustain wisdom? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_SUST_WIS;

	    if (!get_com("Sustain dexterity? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_SUST_DEX;

	    if (!get_com("Sustain constitution? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_SUST_CON;

	    if (!get_com("Sustain charisma? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_SUST_CHR;
	}

	if (!get_com("Ignore Anything? ", &ch)) return;
	if (ch == 'y' || ch == 'Y') {

	    if (!get_com("Ignore Acid? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags3 |= TR3_IGNORE_ACID;

	    if (!get_com("Ignore Lightning? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags3 |= TR3_IGNORE_ELEC;

	    if (!get_com("Ignore Fire? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags3 |= TR3_IGNORE_FIRE;

	    if (!get_com("Ignore Cold? ", &ch)) return;
	    if (ch == 'y' || ch == 'Y') i_ptr->flags3 |= TR3_IGNORE_COLD;
    }
    
	if (!get_com("Free Action? ", &ch)) return;
	if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_FREE_ACT;

	if (!get_com("Resist life level loss? ", &ch)) return;
	if (ch == 'y' || ch == 'Y') i_ptr->flags2 |= TR2_HOLD_LIFE;

	if (!get_com("Slow Digestion? ", &ch)) return;
	if (ch == 'y' || ch == 'Y') i_ptr->flags3 |= TR3_SLOW_DIGEST;

	if (!get_com("Aggravate Monsters? ", &ch)) return;
	if (ch == 'y' || ch == 'Y') i_ptr->flags3 |= TR3_AGGRAVATE;

	if (!get_com("Regeneration? ", &ch)) return;
	if (ch == 'y' || ch == 'Y') i_ptr->flags3 |= TR3_REGEN;

	if (!get_com("Feather Falling? ", &ch)) return;
	if (ch == 'y' || ch == 'Y') i_ptr->flags3 |= TR3_FEATHER;

	if (!get_com("Telepathy? ", &ch)) return;
	if (ch == 'y' || ch == 'Y') i_ptr->flags3 |= TR3_TELEPATHY;

	if (!get_com("See invisible? ", &ch)) return;
	if (ch == 'y' || ch == 'Y') i_ptr->flags3 |= TR3_SEE_INVIS;

	if (!get_com("Give off Light? ", &ch)) return;
	if (ch == 'y' || ch == 'Y') i_ptr->flags3 |= TR3_LITE;

	if (!get_com("Blessed by the Gods? ", &ch)) return;
	if (ch == 'y' || ch == 'Y') i_ptr->flags3 |= TR3_BLESSED;

	if (!get_com("Cursed? ", &ch)) return;
	if (ch == 'y' || ch == 'Y') i_ptr->flags3 |= TR3_CURSED;

	if (!get_com("Heavily Cursed? ", &ch)) return;
	if (ch == 'y' || ch == 'Y') i_ptr->flags3 |= TR3_HEAVY_CURSE;

	if (!get_com("Permenantly Cursed? ", &ch)) return;
	if (ch == 'y' || ch == 'Y') i_ptr->flags3 |= TR3_PERMA_CURSE;
    }

    prt("Cost [escape=make, return=default]: ", 0, 0);
    if (!askfor(tmp_str, 8)) return;
    tmp_lval = atol(tmp_str);
    if (tmp_lval < 0) tmp_lval = 0;
    if (tmp_val) i_ptr->cost = tmp_lval;


    /*** Magic/Artifact ***/

    /* Ask permission */
    if (!get_com("Apply dungeon magic to the object? ", &ch)) return;
    if (ch == 'y' || ch == 'Y') apply_magic(i_ptr, dun_level, FALSE, FALSE, FALSE);

    /* Ask permission (may interact badly with "dungeon magic") */
    if (!get_com("Attempt to turn the item into an artifact? ", &ch)) return;
    while (ch == 'y' || ch == 'Y') {
	if (make_artifact(i_ptr)) return;
	if (!get_com("Attempt failed.  Try again? ", &ch)) return;
    }


    /*** Special Name ***/

    /* Get a "ego-name" */
    msg_print("Warning: Improper use of Ego-Item Names can be fatal");
    if (!get_com("Choose a Ego-Item Name? ", &ch)) return;
    if (ch != 'y' && ch != 'Y') return;

    /* Show pages until a legal value is chosen for "k" */
    for (page = i = 0; !i_ptr->name2; ) {

	/* Clear the screen */
	clear_screen();

	/* Show up to twenty options at a time.  Hack -- wrap when legal */
	for (num = 0; (num < 20) && (page || (i < EGO_MAX)); i++) {
	    if (i == EGO_MAX) i = 0;
	    if (!ego_names[i]) continue;
	    option[num++] = i;
	}

	/* Display the options */
	for (j = 0; j < num; j++) {
	    char /* p1 = '(', */ p2 = ')';
	    int opt = option[j];
	    sprintf(tmp_str, "%c%c [%d] %s",
		    'a' + j, p2, opt, ego_names[opt]);
	    prt(tmp_str, j+1, 0);
	}

	/* Notice a full page */
	if (num>=20) page++;

	/* Build a prompt */
	sprintf(tmp_str, "Choose a special name (%sESCAPE for none): ",
		(num<20) ? "" : "Space for more choices, ");

	/* Get a command */
	if (!get_com(tmp_str, &ch)) return;

	/* Extract the index */
	j = ch - 'a';

	/* Extract the special name */
	if ((j >= 0) && (j < num)) i_ptr->name2 = option[j];
    }
}





/*
 * Wizard routine for creating objects		-RAK-	 
 *
 * Note that wizards can nuke artifacts / stairs this way
 *
 * Assume that player can never be inside a blocked grid.
 */
static void wizard_create()
{
    int cur_pos;
    cave_type  *c_ptr;

    inven_type forge;


    /* Save the screen */
    save_screen();

    /* Wipe the object */
    invcopy(&forge, TV_NOTHING);

    /* Make an object */
    wizard_create_aux1(&forge);

    /* Restore the screen */
    restore_screen();

    /* Nothing made */
    if (forge.tval == TV_NOTHING) return;


    /* Save the screen */
    save_screen();

    /* Make an object */
    wizard_create_aux2(&forge);

    /* Restore the screen */
    restore_screen();


    /* Allow the user to make the object */
    if (!get_check("Allocate?")) {
	msg_print("Aborted.");
	return;
    }


    /* Delete object (if any) below player */
    delete_object(char_row, char_col);

    /* Create a dungeon object */
    cur_pos = i_pop();
    i_list[cur_pos] = forge;
    i_list[cur_pos].iy = char_row;
    i_list[cur_pos].ix = char_col;
    c_ptr = &cave[char_row][char_col];
    c_ptr->i_idx = cur_pos;

    /* All done */
    msg_print("Allocated.");
}



/*
 * Cure everything -- will not show up until next turn.
 */
static void wizard_cure_all()
{
    (void)remove_all_curse();
    (void)cure_blindness();
    (void)cure_confusion();
    (void)cure_poison();
    (void)remove_fear();

    (void)res_stat(A_STR);
    (void)res_stat(A_INT);
    (void)res_stat(A_WIS);
    (void)res_stat(A_CON);
    (void)res_stat(A_DEX);
    (void)res_stat(A_CHR);

    (void)restore_level();
    (void)hp_player(2000);

    p_ptr->food = PLAYER_FOOD_MAX;

    /* Hack -- do not QUITE cure things, let dungeon() do it */
    if (p_ptr->slow > 1) p_ptr->slow = 1;
    if (p_ptr->image > 1) p_ptr->image = 1;
    if (p_ptr->cut > 1) p_ptr->cut = 1;
    if (p_ptr->stun > 1) p_ptr->stun = 1;
}


/*
 * Go to any level
 */
static void wizard_goto_level(int level)
{
    int i;
    vtype tmp_str;

    if (level > 0) {
	i = level;
    }
    else {
	prt("Go to which level (0-1000)? ", 0, 0);
	i = (-1);
	if (get_string(tmp_str, 0, 27, 10)) i = atoi(tmp_str);
    }

    if (i > 1000) i = 1000;

    if (i >= 0) {
	dun_level = i;
	if (dun_level > 1000) dun_level = 1000;
	new_level_flag = TRUE;
    }

    erase_line(MSG_LINE, 0);
}


/*
 * Identify a lot of objects
 */

static void wizard_identify_many()
{
    int			i;
    vtype		tmp_str;
    int                 temp;
    inven_type          inv;

    prt("Identify objects upto which level (0-200) ? ", 0, 0);
    i = (-1);
    if (get_string(tmp_str, 0, 47, 10)) i = atoi(tmp_str);
    if (i > 200) i = 200;


    /* Identify all the objects */        
    if (i > -1) {

	/* Scan every object */
	for (temp = 0; temp < MAX_K_IDX; temp++) {
	    if (k_list[temp].level <= i) {
		invcopy(&inv, temp);
		inven_aware(&inv);
	    }
	}
    }

    erase_line(MSG_LINE, 0);
}


/*
 * Wizard Help
 */
static void do_cmd_wiz_help()
{	    
    /* Dump a file */
    if (rogue_like_commands) {
	helpfile(ANGBAND_RWIZ_HELP);
    }
    else {
	helpfile(ANGBAND_OWIZ_HELP);
    }
}

/*
 * Rerate Hitpoints
 */
static void do_cmd_rerate()
{
    int         min_value, max_value, i, percent;
    char        buf[50];

    min_value = (MAX_PLAYER_LEVEL * 3 * (p_ptr->hitdie - 1)) / 8 +
	MAX_PLAYER_LEVEL;
    max_value = (MAX_PLAYER_LEVEL * 5 * (p_ptr->hitdie - 1)) / 8 +
	MAX_PLAYER_LEVEL;
    player_hp[0] = p_ptr->hitdie;
    do {
	for (i = 1; i < MAX_PLAYER_LEVEL; i++) {
	    player_hp[i] = randint((int)p_ptr->hitdie);
	    player_hp[i] += player_hp[i - 1];
	}
    }
    while ((player_hp[MAX_PLAYER_LEVEL - 1] < min_value) ||
	   (player_hp[MAX_PLAYER_LEVEL - 1] > max_value));

    percent = (int)(((long)player_hp[MAX_PLAYER_LEVEL - 1] * 200L) /
		(p_ptr->hitdie + ((MAX_PLAYER_LEVEL - 1) * p_ptr->hitdie)));

    sprintf(buf, "%d%% Life Rating", percent);
    calc_hitpoints();
    prt_stat_block();
    msg_print(buf);
}



/*
 * Attempt to parse 'command' as a wizard command.
 * Return 'TRUE' iff the command was correctly parsed.
 */
int do_wiz_command(void)
{
    int                    y, x;

    /* You must be a wizard! */
    if (!wizard) return (FALSE);

    /* All wizard commands default to "free" */
    free_turn_flag = TRUE;

    /* Analyze the command */
    switch (command_cmd) {

	/* Wizard Help */
	case '\\':
	    do_cmd_wiz_help(); break;

	/* Rerate Hitpoints */
	case '!':
	    do_cmd_rerate(); break;

	/* Check on all Artifacts */
	case '~':
	    artifact_check(); break;

	/* Check on all Uniques */
	case '|':
	    check_uniques(); break;

	/* Teleport */
	case CTRL('T'):
	    teleport(100); break;

	/* Zap Monsters (Genocide) */
	case CTRL('Z'):
	    (void)mass_genocide(FALSE); break;

	/* Magic Mapping */
	case ':':
	    map_area(); break;

	/* Create any object */
	case '@':
	    wizard_create(); break;

	/* Wizard Light the Level */
	case '$':
	    wiz_lite(); break;

	/* Self-Knowledge */
	case '%':
	    self_knowledge(); break;

	/* Identify all up to a level */
	case CTRL('^'):
	    wizard_identify_many(); break;

	/* Identify */
	case CTRL('I'):
	    if (!ident_floor()) combine(ident_spell()); break;

	/* Edit character */
	case CTRL('E'):
	    change_character(); erase_line(MSG_LINE, 0); break;

	/* Cure all maladies */
	case CTRL('A'):
	    wizard_cure_all(); break;

	/* Go up or down in the dungeon */
	case CTRL('D'):
	    wizard_goto_level(command_arg); break;

	/* Generate Treasure */
	case CTRL('G'):
		if (command_arg <= 0) command_arg = 1;
		random_object(char_row, char_col, command_arg);
		prt_map();
		break;

	/* Generate Very Good Treasure */	
	case CTRL('V'):
		if (command_arg <= 0) command_arg = 1;
		special_random_object(char_row, char_col, command_arg);
		prt_map();
		break;

	/* Summon Monster */	
	case '&':
		y = char_row;
		x = char_col;
		(void)summon_monster(&y, &x, TRUE);
		break;

	/* Increase Experience */
	case '+':
		if (command_arg > 0) {
		    p_ptr->exp = command_arg;
		}
		else if (p_ptr->exp == 0) {
		    p_ptr->exp = 1;
		}
		else {
		    p_ptr->exp = p_ptr->exp * 2;
		}
		prt_experience();
		break;

	/* Exit Wizard Mode */
	case CTRL('W'):
	    wizard = FALSE;
	    msg_print("Wizard mode off.");
	    prt_winner();
	    break;

	/* Not a Wizard Command */
	default:
	    free_turn_flag = FALSE;
	    return (FALSE);
	    break;
    }

    /* Success */
    return (TRUE);
}

