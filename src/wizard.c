/*
 * wizard.c: Version history and info, and wizard mode debugging aids. 
 *
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include <stdio.h>
#include "constant.h"
#include "config.h"
#include "types.h"
#include "externs.h"
#include "monster.h"

#ifdef USG
#ifndef ATARIST_MWC
#include <string.h>
#endif
#else
#include <strings.h>
#endif

#ifndef linux
long                atol();
#endif

int
is_wizard(uid)
    int                 uid;
{
    FILE               *fp;
    char                buf[100];
    int                 test;

    if ((fp = my_tfopen(ANGBAND_WIZ, "r")) == NULL) {
	fprintf(stderr, "Can't get wizard check...");
	exit_game();
    }
    do {
	(void)fgets(buf, sizeof buf, fp);
	if (sscanf(buf, "%d", &test)) {
	    if (test == uid && buf[0] != '#') {
		fclose(fp);
		return TRUE;
	    }
	}
    } while (!feof(fp));
    fclose(fp);
    return FALSE;
}

/* Check to see which artifacts have been seen		 */
void
artifact_check()
{
    FILE               *file1;
    vtype               filename;

    prt("Checking for artifacts that have been seen... ", 0, 0);
    prt("File name: ", 0, 0);
    if (get_string(filename, 0, 11, 64)) {
	if (strlen(filename) == 0)
	    return;
	if ((file1 = my_tfopen(filename, "w")) != NULL) {
	    (void)fprintf(file1, "Artifacts that have been seen\n");
	    (void)fprintf(file1, "\n");
	    if (GROND)
		fprintf(file1, "Grond\n");
	    if (RINGIL)
		fprintf(file1, "Ringil\n");
	    if (AEGLOS)
		fprintf(file1, "Aeglos\n");
	    if (ARUNRUTH)
		fprintf(file1, "Arunruth\n");
	    if (MORMEGIL)
		fprintf(file1, "Mormegil\n");
	    if (ANGRIST)
		fprintf(file1, "Angrist\n");
	    if (GURTHANG)
		fprintf(file1, "Gurthang\n");
	    if (CALRIS)
		fprintf(file1, "Calris\n");
	    if (ANDURIL)
		fprintf(file1, "Anduril\n");
	    if (STING)
		fprintf(file1, "Sting\n");
	    if (ORCRIST)
		fprintf(file1, "Orcrist\n");
	    if (GLAMDRING)
		fprintf(file1, "Glamdring\n");
	    if (DURIN)
		fprintf(file1, "Durin\n");
	    if (AULE)
		fprintf(file1, "Aule\n");
	    if (THUNDERFIST)
		fprintf(file1, "Thunderfist\n");
	    if (BLOODSPIKE)
		fprintf(file1, "Bloodspike\n");
	    if (DOOMCALLER)
		fprintf(file1, "Doomcaller\n");
	    if (NARTHANC)
		fprintf(file1, "Narthanc\n");
	    if (NIMTHANC)
		fprintf(file1, "Nimthanc\n");
	    if (DETHANC)
		fprintf(file1, "Dethanc\n");
	    if (GILETTAR)
		fprintf(file1, "Gilettar\n");
	    if (RILIA)
		fprintf(file1, "Rilia\n");
	    if (BELANGIL)
		fprintf(file1, "Belangil\n");
	    if (BALLI)
		fprintf(file1, "Balli Stonehand\n");
	    if (LOTHARANG)
		fprintf(file1, "Lotharang\n");
	    if (FIRESTAR)
		fprintf(file1, "Firestar\n");
	    if (ERIRIL)
		fprintf(file1, "Eriril\n");
	    if (CUBRAGOL)
		fprintf(file1, "Cubragol\n");
	    if (BARD)
		fprintf(file1, "Longbow of Bard\n");
	    if (COLLUIN)
		fprintf(file1, "Colluin\n");
	    if (HOLCOLLETH)
		fprintf(file1, "Holcolleth\n");
	    if (TOTILA)
		fprintf(file1, "Totila\n");
	    if (PAIN)
		fprintf(file1, "Glaive of Pain\n");
	    if (ELVAGIL)
		fprintf(file1, "Elvagil\n");
	    if (AGLARANG)
		fprintf(file1, "Aglarang\n");
	    if (EORLINGAS)
		fprintf(file1, "Eorlingas\n");
	    if (BARUKKHELED)
		fprintf(file1, "Barukkheled\n");
	    if (WRATH)
		fprintf(file1, "Trident of Wrath\n");
	    if (HARADEKKET)
		fprintf(file1, "Haradekket\n");
	    if (MUNDWINE)
		fprintf(file1, "Mundwine\n");
	    if (GONDRICAM)
		fprintf(file1, "Gondricam\n");
	    if (ZARCUTHRA)
		fprintf(file1, "Zarcuthra\n");
	    if (CARETH)
		fprintf(file1, "Careth Asdriag\n");
	    if (FORASGIL)
		fprintf(file1, "Forasgil\n");
	    if (CRISDURIAN)
		fprintf(file1, "Crisdurian\n");
	    if (COLANNON)
		fprintf(file1, "Colannon\n");
	    if (HITHLOMIR)
		fprintf(file1, "Hithlomir\n");
	    if (THALKETTOTH)
		fprintf(file1, "Thalkettoth\n");
	    if (ARVEDUI)
		fprintf(file1, "Arvedui\n");
	    if (THRANDUIL)
		fprintf(file1, "Thranduil\n");
	    if (THENGEL)
		fprintf(file1, "Thengel\n");
	    if (HAMMERHAND)
		fprintf(file1, "Hammerhand\n");
	    if (CELEGORM)
		fprintf(file1, "Celegorm\n");
	    if (THROR)
		fprintf(file1, "Thror\n");
	    if (MAEDHROS)
		fprintf(file1, "Maedhros\n");
	    if (OLORIN)
		fprintf(file1, "Olorin\n");
	    if (ANGUIREL)
		fprintf(file1, "Anguirel\n");
	    if (OROME)
		fprintf(file1, "Orome\n");
	    if (EONWE)
		fprintf(file1, "Eonwe\n");
	    if (THEODEN)
		fprintf(file1, "Theoden\n");
	    if (ULMO)
		fprintf(file1, "Trident of Ulmo\n");
	    if (OSONDIR)
		fprintf(file1, "Osondir\n");
	    if (TURMIL)
		fprintf(file1, "Turmil\n");
	    if (TIL)
		fprintf(file1, "Til-i-arc\n");
	    if (DEATHWREAKER)
		fprintf(file1, "Deathwreaker\n");
	    if (AVAVIR)
		fprintf(file1, "Avavir\n");
	    if (TARATOL)
		fprintf(file1, "Taratol\n");
	    if (DOR_LOMIN)
		fprintf(file1, "Dor-Lomin\n");
	    if (BELEGENNON)
		fprintf(file1, "Belegennon\n");
	    if (FEANOR)
		fprintf(file1, "Feanor\n");
	    if (ISILDUR)
		fprintf(file1, "Isildur\n");
	    if (SOULKEEPER)
		fprintf(file1, "Soulkeeper\n");
	    if (FINGOLFIN)
		fprintf(file1, "Fingolfin\n");
	    if (ANARION)
		fprintf(file1, "Anarion\n");
	    if (BELEG)
		fprintf(file1, "Belthronding\n");
	    if (DAL)
		fprintf(file1, "Dal-i-thalion\n");
	    if (PAURHACH)
		fprintf(file1, "Paurhach\n");
	    if (PAURNIMMEN)
		fprintf(file1, "Paurnimmen\n");
	    if (PAURAEGEN)
		fprintf(file1, "Pauragen\n");
	    if (PAURNEN)
		fprintf(file1, "Paurnen\n");
	    if (CAMMITHRIM)
		fprintf(file1, "Cammithrin\n");
	    if (CAMBELEG)
		fprintf(file1, "Cambeleg\n");
	    if (HOLHENNETH)
		fprintf(file1, "Holhenneth\n");
	    if (AEGLIN)
		fprintf(file1, "Aeglin\n");
	    if (CAMLOST)
		fprintf(file1, "Camlost\n");
	    if (NIMLOTH)
		fprintf(file1, "Nimloth\n");
	    if (NAR)
		fprintf(file1, "Nar-i-vagil\n");
	    if (BERUTHIEL)
		fprintf(file1, "Beruthiel\n");
	    if (GORLIM)
		fprintf(file1, "Gorlim\n");
	    if (THORIN)
		fprintf(file1, "Thorin\n");
	    if (CELEBORN)
		fprintf(file1, "Celeborn\n");
	    if (GONDOR)
		fprintf(file1, "Gondor\n");
	    if (THINGOL)
		fprintf(file1, "Thingol\n");
	    if (THORONGIL)
		fprintf(file1, "Thorongil\n");
	    if (LUTHIEN)
		fprintf(file1, "Luthien\n");
	    if (TUOR)
		fprintf(file1, "Tuor\n");
	    if (ROHAN)
		fprintf(file1, "Rohan\n");
	    if (CASPANION)
		fprintf(file1, "Caspanion\n");
	    if (NARYA)
		fprintf(file1, "Narya\n");
	    if (NENYA)
		fprintf(file1, "Nenya\n");
	    if (VILYA)
		fprintf(file1, "Vilya\n");
	    if (POWER)
		fprintf(file1, "The One Ring\n");
	    if (PHIAL)
		fprintf(file1, "The Phial of Galadriel\n");
	    if (INGWE)
		fprintf(file1, "The Amulet of Ingwe\n");
	    if (CARLAMMAS)
		fprintf(file1, "The Amulet of Carlammas\n");
	    if (TULKAS)
		fprintf(file1, "The Ring of Tulkas\n");
	    if (NECKLACE)
		fprintf(file1, "The Amulet of the Dwarves\n");
	    if (BARAHIR)
		fprintf(file1, "The Ring of Barahir\n");
	    if (ELENDIL)
		fprintf(file1, "The Star of Elendil\n");
	    if (THRAIN)
		fprintf(file1, "The Arkenstone of Thrain\n");
	    if (RAZORBACK)
		fprintf(file1, "Razorback\n");
	    if (BLADETURNER)
		fprintf(file1, "Bladeturner\n");
	    (void)fclose(file1);
	    prt("Done...", 0, 0);
	} else
	    prt("File could not be opened.", 0, 0);
    } else
	prt("File could not be opened.", 0, 0);
}

/* Light up the dungeon					-RAK-	 */
void
wizard_light(light)
    int                 light;
{
    register cave_type *c_ptr;
    register int        k, l, i, j;
    int                 flag;

    if (!light) {
	if (cave[char_row][char_col].pl)
	    flag = FALSE;
	else
	    flag = TRUE;
    } else {
	flag = (light > 0) ? 1 : 0;
    }
    for (i = 0; i < cur_height; i++)
	for (j = 0; j < cur_width; j++)
	    if (cave[i][j].fval <= MAX_CAVE_FLOOR)
		for (k = i - 1; k <= i + 1; k++)
		    for (l = j - 1; l <= j + 1; l++) {
			c_ptr = &cave[k][l];
			c_ptr->pl = flag;
			if (!flag)
			    c_ptr->fm = FALSE;
		    }
    prt_map();
}


/* Wizard routine for gaining on stats			-RAK-	 */
void
change_character()
{
    register int        tmp_val;
    register int32      tmp_lval;
    int16u             *a_ptr;
    vtype               tmp_str;
    register struct misc *m_ptr;

    a_ptr = py.stats.max_stat;
    prt("(3 - 118) Strength     = ", 0, 0);
    if (get_string(tmp_str, 0, 25, 3)) {
	tmp_val = atoi(tmp_str);
	if ((tmp_val > 2) && (tmp_val < 119)) {
	    a_ptr[A_STR] = tmp_val;
	    (void)res_stat(A_STR);
	}
    } else
	return;

    prt("(3 - 118) Intelligence = ", 0, 0);
    if (get_string(tmp_str, 0, 25, 3)) {
	tmp_val = atoi(tmp_str);
	if ((tmp_val > 2) && (tmp_val < 119)) {
	    a_ptr[A_INT] = tmp_val;
	    (void)res_stat(A_INT);
	}
    } else
	return;

    prt("(3 - 118) Wisdom       = ", 0, 0);
    if (get_string(tmp_str, 0, 25, 3)) {
	tmp_val = atoi(tmp_str);
	if ((tmp_val > 2) && (tmp_val < 119)) {
	    a_ptr[A_WIS] = tmp_val;
	    (void)res_stat(A_WIS);
	}
    } else
	return;

    prt("(3 - 118) Dexterity    = ", 0, 0);
    if (get_string(tmp_str, 0, 25, 3)) {
	tmp_val = atoi(tmp_str);
	if ((tmp_val > 2) && (tmp_val < 119)) {
	    a_ptr[A_DEX] = tmp_val;
	    (void)res_stat(A_DEX);
	}
    } else
	return;

    prt("(3 - 118) Constitution = ", 0, 0);
    if (get_string(tmp_str, 0, 25, 3)) {
	tmp_val = atoi(tmp_str);
	if ((tmp_val > 2) && (tmp_val < 119)) {
	    a_ptr[A_CON] = tmp_val;
	    (void)res_stat(A_CON);
	}
    } else
	return;

    prt("(3 - 118) Charisma     = ", 0, 0);
    if (get_string(tmp_str, 0, 25, 3)) {
	tmp_val = atoi(tmp_str);
	if ((tmp_val > 2) && (tmp_val < 119)) {
	    a_ptr[A_CHR] = tmp_val;
	    (void)res_stat(A_CHR);
	}
    } else
	return;

    m_ptr = &py.misc;
    prt("(1 - 32767) Hit points = ", 0, 0);
    if (get_string(tmp_str, 0, 25, 5)) {
	tmp_val = atoi(tmp_str);
	if ((tmp_val > 0) && (tmp_val <= MAX_SHORT)) {
	    m_ptr->mhp = tmp_val;
	    m_ptr->chp = tmp_val;
	    m_ptr->chp_frac = 0;
	    prt_mhp();
	    prt_chp();
	}
    } else
	return;

    prt("(0 - 32767) Mana       = ", 0, 0);
    if (get_string(tmp_str, 0, 25, 5)) {
	tmp_val = atoi(tmp_str);
	if ((tmp_val > -1) && (tmp_val <= MAX_SHORT) && (*tmp_str != '\0')) {
	    m_ptr->mana = tmp_val;
	    m_ptr->cmana = tmp_val;
	    m_ptr->cmana_frac = 0;
	    prt_cmana();
	}
    } else
	return;

    (void)sprintf(tmp_str, "Current=%ld  Gold = ", m_ptr->au);
    tmp_val = strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 7)) {
	tmp_lval = atol(tmp_str);
	if (tmp_lval > -1 && (*tmp_str != '\0')) {
	    m_ptr->au = tmp_lval;
	    prt_gold();
	}
    } else
	return;
    (void)sprintf(tmp_str, "Current=%ld  Max Exp = ", m_ptr->max_exp);
    tmp_val = strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 7)) {
	tmp_lval = atol(tmp_str);
	if (tmp_lval > -1 && (*tmp_str != '\0')) {
	    m_ptr->max_exp = tmp_lval;
	    prt_experience();
	}
    } else
	return;

    (void)sprintf(tmp_str, "Current=%d  (0-200) Searching = ", m_ptr->srh);
    tmp_val = strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 3)) {
	tmp_val = atoi(tmp_str);
	if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0'))
	    m_ptr->srh = tmp_val;
    } else
	return;

    (void)sprintf(tmp_str, "Current=%d  (-1-18) Stealth = ", m_ptr->stl);
    tmp_val = strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 3)) {
	tmp_val = atoi(tmp_str);
	if ((tmp_val > -2) && (tmp_val < 19) && (*tmp_str != '\0'))
	    m_ptr->stl = tmp_val;
    } else
	return;

    (void)sprintf(tmp_str, "Current=%d  (0-200) Disarming = ", m_ptr->disarm);
    tmp_val = strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 3)) {
	tmp_val = atoi(tmp_str);
	if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0'))
	    m_ptr->disarm = tmp_val;
    } else
	return;

    (void)sprintf(tmp_str, "Current=%d  (0-100) Save = ", m_ptr->save);
    tmp_val = strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 3)) {
	tmp_val = atoi(tmp_str);
	if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0'))
	    m_ptr->save = tmp_val;
    } else
	return;

    (void)sprintf(tmp_str, "Current=%d  (0-200) Base to hit = ", m_ptr->bth);
    tmp_val = strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 3)) {
	tmp_val = atoi(tmp_str);
	if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0'))
	    m_ptr->bth = tmp_val;
    } else
	return;

    (void)sprintf(tmp_str, "Current=%d  (0-200) Bows/Throwing = ", m_ptr->bthb);
    tmp_val = strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 3)) {
	tmp_val = atoi(tmp_str);
	if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0'))
	    m_ptr->bthb = tmp_val;
    } else
	return;

    (void)sprintf(tmp_str, "Current=%d  Weight = ", m_ptr->wt);
    tmp_val = strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 3)) {
	tmp_val = atoi(tmp_str);
	if (tmp_val > -1 && (*tmp_str != '\0'))
	    m_ptr->wt = tmp_val;
    } else
	return;

    while (get_com("Alter speed? (+/-)", tmp_str)) {
	if (*tmp_str == '+')
	    change_speed(-1);
	else if (*tmp_str == '-')
	    change_speed(1);
	else
	    break;
	prt_speed();
    }
}


/* Wizard routine for creating objects			-RAK-	 */
void
wizard_create()
{
    register int        tmp_val;
    int                 i, j, k;
    int32               tmp_lval;
    char                tmp_str[100];
    register inven_type *i_ptr;
    treasure_type       t_type, *t_ptr;
    inven_type          forge;
    register cave_type *c_ptr;
    char                ch;
    int                 more = FALSE;

    t_ptr = &t_type;
    i_ptr = &forge;
    i_ptr->name2 = 0;
    i_ptr->ident = ID_KNOWN2 | ID_STOREBOUGHT;

    save_screen();
    prt("What type of item?    : ", 0, 0);
    prt("[W]eapon, [A]rmour, [O]thers.", 1, 0);
    if (!get_com((char *)0, &ch)) {
	restore_screen();
	return;
    }
    switch (ch) {
      case 'W':
      case 'w':
	prt("What type of Weapon?    : ", 0, 0);
	prt("[S]word, [H]afted, [P]olearm, [B]ow, [A]mmo.", 1, 0);
	if (!get_com((char *)0, &ch)) {
	    restore_screen();
	    return;
	}
	switch (ch) {
	  case 'S':
	  case 's':
	    i_ptr->tval = TV_SWORD;
	    break;
	  case 'H':
	  case 'h':
	    i_ptr->tval = TV_HAFTED;
	    break;
	  case 'P':
	  case 'p':
	    i_ptr->tval = TV_POLEARM;
	    break;
	  case 'B':
	  case 'b':
	    i_ptr->tval = TV_BOW;
	    break;
	  case 'A':
	  case 'a':
	    prt("What type of Ammo?    : ", 0, 0);
	    prt("[A]rrow, [B]olt, [P]ebble.", 1, 0);
	    if (!get_com((char *)0, &ch)) {
		restore_screen();
		return;
	    }
	    switch (ch) {
	      case 'A':
	      case 'a':
		i_ptr->tval = TV_ARROW;
		break;
	      case 'B':
	      case 'b':
		i_ptr->tval = TV_BOLT;
		break;
	      case 'P':
	      case 'p':
		i_ptr->tval = TV_SLING_AMMO;
		break;
	      default:
		break;
	    }
	    break;
	  default:
	    restore_screen();
	    return;
	}
	break;
      case 'A':
      case 'a':
	prt("What type of Armour?    : ", 0, 0);
	prt("[A]rmour, [G]loves, [B]oots, [S]hields, [H]elms, [C]loaks.", 1, 0);
	if (!get_com((char *)0, &ch)) {
	    restore_screen();
	    return;
	}
	switch (ch) {
	  case 'S':
	  case 's':
	    i_ptr->tval = TV_SHIELD;
	    break;
	  case 'H':
	  case 'h':
	    i_ptr->tval = TV_HELM;
	    break;
	  case 'G':
	  case 'g':
	    i_ptr->tval = TV_GLOVES;
	    break;
	  case 'B':
	  case 'b':
	    i_ptr->tval = TV_BOOTS;
	    break;
	  case 'C':
	  case 'c':
	    i_ptr->tval = TV_CLOAK;
	    break;
	  case 'A':
	  case 'a':
	    prt("What type of Armour?    : ", 0, 0);
	    prt("[H]ard armour, [S]oft armour.", 1, 0);
	    if (!get_com((char *)0, &ch)) {
		restore_screen();
		return;
	    }
	    switch (ch) {
	      case 'H':
	      case 'h':
		i_ptr->tval = TV_HARD_ARMOR;
		break;
	      case 'S':
	      case 's':
		i_ptr->tval = TV_SOFT_ARMOR;
		break;
	      default:
		break;
	    }
	    break;
	  default:
	    restore_screen();
	    return;
	}
	break;
      case 'O':
      case 'o':
	prt("What type of Object?    : ", 0, 0);
	prt(
	    "[R]ing, [P]otion, [W]and/staff, [S]croll, [M]agicbook, [A]mulet, [T]ool.",
	    1, 0);
	if (!get_com((char *)0, &ch)) {
	    restore_screen();
	    return;
	}
	switch (ch) {
	  case 'R':
	  case 'r':
	    i_ptr->tval = TV_RING;
	    break;
	  case 'P':
	  case 'p':
	    i_ptr->tval = TV_POTION1;
	    break;
	  case 'S':
	  case 's':
	    i_ptr->tval = TV_SCROLL1;
	    break;
	  case 'A':
	  case 'a':
	    i_ptr->tval = TV_AMULET;
	    break;
	  case 'W':
	  case 'w':
	    prt("Wand, Staff or Rod?    : ", 0, 0);
	    prt("[W]and, [S]taff, [R]od.", 1, 0);
	    if (!get_com((char *)0, &ch)) {
		restore_screen();
		return;
	    }
	    switch (ch) {
	      case 'W':
	      case 'w':
		i_ptr->tval = TV_WAND;
		break;
	      case 'S':
	      case 's':
		i_ptr->tval = TV_STAFF;
		break;
	      case 'R':
	      case 'r':
		i_ptr->tval = TV_ROD;
		break;
	      default:
		restore_screen();
		return;
	    }
	    break;
	  case 'M':
	  case 'm':
	    prt("Spellbook or Prayerbook?    : ", 0, 0);
	    prt("[S]pellbook, [P]rayerbook.", 1, 0);
	    if (!get_com((char *)0, &ch)) {
		restore_screen();
		return;
	    }
	    switch (ch) {
	      case 'P':
	      case 'p':
		i_ptr->tval = TV_PRAYER_BOOK;
		break;
	      case 'S':
	      case 's':
		i_ptr->tval = TV_MAGIC_BOOK;
		break;
	      default:
		restore_screen();
		return;
	    }
	    break;
	  case 'T':
	  case 't':
	    prt("Which Tool etc...?  : ", 0, 0);
	    prt("[S]pike, [D]igger, [C]hest, [L]ight, [F]ood, [O]il.", 1, 0);
	    if (!get_com((char *)0, &ch)) {
		restore_screen();
		return;
	    }
	    switch (ch) {
	      case 'S':
	      case 's':
		i_ptr->tval = TV_SPIKE;
		break;
	      case 'd':
	      case 'D':
		i_ptr->tval = TV_DIGGING;
		break;
	      case 'C':
	      case 'c':
		i_ptr->tval = TV_CHEST;
		break;
	      case 'L':
	      case 'l':
		i_ptr->tval = TV_LIGHT;
		break;
	      case 'F':
	      case 'f':
		i_ptr->tval = TV_FOOD;
		break;
	      case 'O':
	      case 'o':
		i_ptr->tval = TV_FLASK;
		break;
	      default:
		restore_screen();
		return;
	    }
	    break;
	  default:
	    restore_screen();
	    return;
	}
	break;
      default:
	restore_screen();
	return;
    }

    j = 0;
    i = 0;
    k = 0;
again:
    restore_screen();
    save_screen();
    prt("Which Item?  : ", 0, 0);
    for (; i < MAX_DUNGEON_OBJ; i++) {
	switch (i_ptr->tval) {
	  case TV_POTION1:
	    if ((object_list[i].tval == TV_POTION1) ||
		(object_list[i].tval == TV_POTION2)) {
		sprintf(tmp_str, "%c) %s", 'a' + j, object_list[i].name);
		prt(tmp_str, 1 + j, 0);
		j++;
	    }
	    break;
	  case TV_SCROLL1:
	    if ((object_list[i].tval == TV_SCROLL1) ||
		(object_list[i].tval == TV_SCROLL2)) {
		sprintf(tmp_str, "%c) %s", 'a' + j, object_list[i].name);
		prt(tmp_str, 1 + j, 0);
		j++;
	    }
	    break;
	  default:
	    if (object_list[i].tval == i_ptr->tval) {
		sprintf(tmp_str, "%c) %s", 'a' + j, object_list[i].name);
		prt(tmp_str, 1 + j, 0);
		j++;
	    }
	    break;
	}
	if (j == 21) {
	    more = TRUE;
	    break;
	}
    }
    if (j < 21) {
	for (i = (i - (MAX_DUNGEON_OBJ - 1)) + (SPECIAL_OBJ - 1); i < MAX_OBJECTS; i++) {
	    switch (i_ptr->tval) {
	      case TV_POTION1:
		if ((object_list[i].tval == TV_POTION1) ||
		    (object_list[i].tval == TV_POTION2)) {
		    sprintf(tmp_str, "%c) %s", 'a' + j, object_list[i].name);
		    prt(tmp_str, 1 + j, 0);
		    j++;
		}
		break;
	      case TV_SCROLL1:
		if ((object_list[i].tval == TV_SCROLL1) ||
		    (object_list[i].tval == TV_SCROLL2)) {
		    sprintf(tmp_str, "%c) %s", 'a' + j, object_list[i].name);
		    prt(tmp_str, 1 + j, 0);
		    j++;
		}
		break;
	      default:
		if (object_list[i].tval == i_ptr->tval) {
		    sprintf(tmp_str, "%c) %s", 'a' + j, object_list[i].name);
		    prt(tmp_str, 1 + j, 0);
		    j++;
		}
		break;
	    }
	    if (j == 21) {
		more = TRUE;
		break;
	    }
	}
    }
    if (more)
	prt("v) NEXT PAGE", 22, 0);

    do {
	if (!get_com((char *)0, &ch)) {
	    restore_screen();
	    return;
	}
    } while ((ch < 'a' && ch > ('a' + j)) || (more && ch < 'a' && ch > ('a' + j + 1)));

    if ((ch == 'v') && more) {
	more = FALSE;
	k += (j - 1);
	j = 0;
	goto again;
    }
    k += (ch - 'a' + 1);

    j = 0;
    for (i = 0; i < MAX_DUNGEON_OBJ; i++) {
	switch (i_ptr->tval) {
	  case TV_POTION1:
	    if ((object_list[i].tval == TV_POTION1) ||
		(object_list[i].tval == TV_POTION2)) {
		j++;
	    }
	    break;
	  case TV_SCROLL1:
	    if ((object_list[i].tval == TV_SCROLL1) ||
		(object_list[i].tval == TV_SCROLL2)) {
		j++;
	    }
	    break;
	  default:
	    if (object_list[i].tval == i_ptr->tval) {
		j++;
	    }
	    break;
	}
	if (j == k)
	    break;
    }
    if (j != k) {
	for (i = (SPECIAL_OBJ - 1); i < MAX_OBJECTS; i++) {
	    switch (i_ptr->tval) {
	      case TV_POTION1:
		if ((object_list[i].tval == TV_POTION1) ||
		    (object_list[i].tval == TV_POTION2)) {
		    j++;
		}
		break;
	      case TV_SCROLL1:
		if ((object_list[i].tval == TV_SCROLL1) ||
		    (object_list[i].tval == TV_SCROLL2)) {
		    j++;
		}
		break;
	      default:
		if (object_list[i].tval == i_ptr->tval) {
		    j++;
		}
		break;
	    }
	    if (j == k)
		break;
	}
    }
    if (j != k) {
	restore_screen();
	return;
    }
    invcopy(i_ptr, i);
    i_ptr->timeout = 0;
    restore_screen();
    save_screen();

    prt("Number of items? [return=1]: ", 0, 0);
    if (!get_string(tmp_str, 0, 33, 5))
	goto end;
    tmp_val = atoi(tmp_str);
    if (tmp_val != 0)
	i_ptr->number = tmp_val;

    prt("Weight of item? [return=default]: ", 0, 0);
    if (!get_string(tmp_str, 0, 35, 5))
	goto end;
    tmp_val = atoi(tmp_str);
    if (tmp_val != 0)
	i_ptr->weight = tmp_val;

    if ((i_ptr->tval == TV_SWORD) ||
	(i_ptr->tval == TV_HAFTED) ||
	(i_ptr->tval == TV_POLEARM) ||
	(i_ptr->tval == TV_ARROW) ||
	(i_ptr->tval == TV_BOLT) ||
	(i_ptr->tval == TV_SLING_AMMO) ||
	(i_ptr->tval == TV_DIGGING)) {
	i_ptr->ident |= ID_SHOW_HITDAM;
	prt("Damage (dice): ", 0, 0);
	if (!get_string(tmp_str, 0, 15, 3))
	    goto end;
	tmp_val = atoi(tmp_str);
	if (tmp_val != 0)
	    i_ptr->damage[0] = tmp_val;
	prt("Damage (sides): ", 0, 0);
	if (!get_string(tmp_str, 0, 16, 3))
	    goto end;
	tmp_val = atoi(tmp_str);
	if (tmp_val != 0)
	    i_ptr->damage[1] = tmp_val;
    }
    prt("+To hit: ", 0, 0);
    if (!get_string(tmp_str, 0, 9, 3))
	goto end;
    tmp_val = atoi(tmp_str);
    if (tmp_val != 0)
	i_ptr->tohit = tmp_val;

    prt("+To dam: ", 0, 0);
    if (!get_string(tmp_str, 0, 9, 3))
	goto end;
    tmp_val = atoi(tmp_str);
    if (tmp_val != 0)
	i_ptr->todam = tmp_val;

    if ((i_ptr->tval == TV_SOFT_ARMOR) ||
	(i_ptr->tval == TV_HARD_ARMOR) ||
	(i_ptr->tval == TV_HELM) ||
	(i_ptr->tval == TV_CLOAK) ||
	(i_ptr->tval == TV_BOOTS) ||
	(i_ptr->tval == TV_GLOVES) ||
	(i_ptr->tval == TV_SHIELD)) {
	prt("Base AC : ", 0, 0);
	if (!get_string(tmp_str, 0, 10, 3))
	    goto end;
	tmp_val = atoi(tmp_str);
	if (tmp_val != 0)
	    i_ptr->ac = tmp_val;
    }
    prt("+To AC : ", 0, 0);
    if (!get_string(tmp_str, 0, 9, 3))
	goto end;
    tmp_val = atoi(tmp_str);
    if (tmp_val != 0)
	i_ptr->toac = tmp_val;

    prt("Magic Plus Flag  : ", 0, 0);
    if (!get_string(tmp_str, 0, 20, 5))
	goto end;
    tmp_val = atoi(tmp_str);
    if (tmp_val != 0)
	i_ptr->p1 = tmp_val;


    save_screen();
    if ((i_ptr->tval <= TV_MAX_WEAR) && (i_ptr->tval >= TV_MIN_WEAR)) {
    /*
     * only then bother with TR_* flags, since otherwise they are
     * meaningless... -CFT 
     */

	if ((i_ptr->tval == TV_SWORD) ||
	    (i_ptr->tval == TV_HAFTED) ||
	    (i_ptr->tval == TV_POLEARM) ||
	    (i_ptr->tval == TV_ARROW) ||
	    (i_ptr->tval == TV_BOLT) ||
	    (i_ptr->tval == TV_SLING_AMMO) ||
	    (i_ptr->tval == TV_DIGGING)) {
	    if (get_com("Slay Evil? [yn]: ", &ch)) {
		if (ch == 'y' || ch == 'Y')
		    i_ptr->flags |= TR_SLAY_EVIL;
	    } else if (ch == '\033')
		goto end;
	    if (get_com("Slay Animal? [yn]: ", &ch)) {
		if (ch == 'y' || ch == 'Y')
		    i_ptr->flags |= TR_SLAY_ANIMAL;
	    } else if (ch == '\033')
		goto end;
	    if (get_com("Slay Undead? [yn]: ", &ch)) {
		if (ch == 'y' || ch == 'Y')
		    i_ptr->flags |= TR_SLAY_UNDEAD;
	    } else if (ch == '\033')
		goto end;
	    if (get_com("Slay Giant? [yn]: ", &ch)) {
		if (ch == 'y' || ch == 'Y')
		    i_ptr->flags2 |= TR_SLAY_GIANT;
	    } else if (ch == '\033')
		goto end;
	    if (get_com("Slay Demon? [yn]: ", &ch)) {
		if (ch == 'y' || ch == 'Y')
		    i_ptr->flags2 |= TR_SLAY_DEMON;
	    } else if (ch == '\033')
		goto end;
	    if (get_com("Slay Troll? [yn]: ", &ch)) {
		if (ch == 'y' || ch == 'Y')
		    i_ptr->flags2 |= TR_SLAY_TROLL;
	    } else if (ch == '\033')
		goto end;
	    if (get_com("Slay Orc? [yn]: ", &ch)) {
		if (ch == 'y' || ch == 'Y')
		    i_ptr->flags2 |= TR_SLAY_ORC;
	    } else if (ch == '\033')
		goto end;
	    if (get_com("Slay Dragon? [yn]: ", &ch)) {
		if (ch == 'y' || ch == 'Y')
		    i_ptr->flags |= TR_SLAY_DRAGON;
	    } else if (ch == '\033')
		goto end;
	    if (get_com("Execute Dragon? [yn]: ", &ch)) {
		if (ch == 'y' || ch == 'Y')
		    i_ptr->flags |= TR_SLAY_X_DRAGON;
	    } else if (ch == '\033')
		goto end;
	    if (get_com("Frost Brand? [yn]: ", &ch)) {
		if (ch == 'y' || ch == 'Y')
		    i_ptr->flags |= TR_FROST_BRAND;
	    } else if (ch == '\033')
		goto end;
	    if (get_com("Fire Brand? [yn]: ", &ch)) {
		if (ch == 'y' || ch == 'Y')
		    i_ptr->flags |= TR_FLAME_TONGUE;
	    } else if (ch == '\033')
		goto end;
	    if (get_com("Lightning Brand? [yn]: ", &ch)) {
		if (ch == 'y' || ch == 'Y')
		    i_ptr->flags2 |= TR_LIGHTNING;
	    } else if (ch == '\033')
		goto end;
	    if (get_com("Earthquake Brand? [yn]: ", &ch)) {
		if (ch == 'y' || ch == 'Y')
		    i_ptr->flags2 |= TR_IMPACT;
	    } else if (ch == '\033')
		goto end;
	}
	if (get_com("Affect Strength? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags |= TR_STR;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Affect Intelligence? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags |= TR_INT;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Affect Wisdom? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags |= TR_WIS;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Affect Dexterity? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags |= TR_DEX;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Affect Constitution? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags |= TR_CON;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Affect Charisma? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags |= TR_CHR;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Automatic Searching? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags |= TR_SEARCH;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Slow Digestion? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags |= TR_SLOW_DIGEST;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Stealth? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags |= TR_STEALTH;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Aggravate Monsters? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags |= TR_AGGRAVATE;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Regeneration? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags |= TR_REGEN;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Speed? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags |= TR_SPEED;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Resist Fire? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags |= TR_RES_FIRE;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Resist Cold? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags |= TR_RES_COLD;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Resist Acid? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags |= TR_RES_ACID;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Resist Lightning? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags |= TR_RES_LIGHT;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Resist Poison? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags |= TR_POISON;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Resist Confusion? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags2 |= TR_RES_CONF;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Resist Sound? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags2 |= TR_RES_SOUND;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Resist Light? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags2 |= TR_RES_LT;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Resist Dark? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags2 |= TR_RES_DARK;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Resist Chaos? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags2 |= TR_RES_CHAOS;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Resist Disenchantment? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags2 |= TR_RES_DISENCHANT;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Resist Shards? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags2 |= TR_RES_SHARDS;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Resist Nexus? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags2 |= TR_RES_NEXUS;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Resist Nether? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags2 |= TR_RES_NETHER;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Resist Blindness? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags2 |= TR_RES_BLIND;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Sustain a stat (Magic value 10 for all stats)? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags |= TR_SUST_STAT;
	} else if (ch == '\033')
	    goto end;
	if (get_com("See invisible? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags |= TR_SEE_INVIS;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Free Action? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags |= TR_FREE_ACT;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Feather Falling? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags |= TR_FFALL;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Tunneling? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags |= TR_TUNNEL;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Infra-vision? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags |= TR_INFRA;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Resist life level loss? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags2 |= TR_HOLD_LIFE;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Telepathy? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags2 |= TR_TELEPATHY;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Immune to Fire? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags2 |= TR_IM_FIRE;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Immune to Cold? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags2 |= TR_IM_COLD;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Immune to Acid? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags2 |= TR_IM_ACID;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Immune to Lightning? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags2 |= TR_IM_LIGHT;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Immune to Poison? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags2 |= TR_IM_POISON;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Give off Light? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags2 |= TR_LIGHT;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Activatable Item? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags2 |= TR_ACTIVATE;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Is it an Artifact? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags2 |= TR_ARTIFACT;
	} else if (ch == '\033')
	    goto end;
	if (get_com("Cursed? [yn]: ", &ch)) {
	    if (ch == 'y' || ch == 'Y')
		i_ptr->flags |= TR_CURSED;
	} else if (ch == '\033')
	    goto end;
    } /* end if TV_MAX_WEAR >= i_ptr->tval >= TV_MIN_WEAR -CFT */
    prt("Cost : ", 0, 0);
    if (!get_string(tmp_str, 0, 9, 8)) {
	restore_screen();
	return;
    }
    tmp_lval = atol(tmp_str);
    if (tmp_val != 0)
	i_ptr->cost = tmp_lval;

    prt("Dungeon Level on which it is found : ", 0, 0);
    if (!get_string(tmp_str, 0, 39, 3)) {
	restore_screen();
	return;
    }
    tmp_val = atoi(tmp_str);
    if (tmp_val != 0)
	i_ptr->level = tmp_val;

    j = 0;
    i = 0;
    k = 0;
    more = FALSE;
SNagain:
    restore_screen();
    save_screen();
    for (; i < SN_ARRAY_SIZE; i++) {
	sprintf(tmp_str, "%c) %s", 'a' + j, special_names[i]);
	prt(tmp_str, 1 + j, 0);
	j++;
	if (j == 21) {
	    more = TRUE;
	    break;
	}
    }
    if (more)
	prt("v) NEXT PAGE", 22, 0);

    do {
	if (!get_com("Please choose a secondary name for the item : ", &ch)) {
	    restore_screen();
	    return;
	}
    } while ((ch < 'a' && ch > ('a' + j)) || (more && ch < 'a' && ch > ('a' + j + 1)));

    if ((ch == 'v') && more) {
	more = FALSE;
	k += (j - 1);
	j = 0;
	goto SNagain;
    } else {
	i_ptr->name2 = k + (ch - 'a');
    }
    restore_screen();
    save_screen();

end:
    if (get_check("Allocate?")) {
    /* delete object first if any, before call popt */
	c_ptr = &cave[char_row][char_col];
	if (c_ptr->tptr != 0)
	    (void)delete_object(char_row, char_col);

	store_bought(i_ptr);
	tmp_val = popt();
	t_list[tmp_val] = forge;
	c_ptr->tptr = tmp_val;
	msg_print("Allocated.");
    } else
	msg_print("Aborted.");
    restore_screen();
}


/* pause if screen fills up while printint up artifacts - cba */

void
#ifdef __STDC__
artifact_screen_full(int *i, int j)
#else
artifact_screen_full(i, j)
    int                *i;
    int                 j;

#endif
{
    int                 t;

/* is screen full? */
    if (*i == 22) {
	prt("-- more --", *i, j);
	inkey();
	for (t = 2; t < 23; t++)
	    erase_line(t, j);	   /* don't forget to erase extra */
	prt("Artifacts seen: (continued)", 1, j + 5);
	*i = 2;
    }
}

/* Print out the artifacts seen without using a file - cba */

void
#ifdef __STDC__
artifact_check_no_file(void)
#else
artifact_check_no_file()
#endif
{
    int                 i, j;

    save_screen();
    j = 15;

    for (i = 1; i < 23; i++)
	erase_line(i, j - 2);

    i = 1;
    prt("Artifacts Seen:", i++, j + 5);

/* weapons */

    if (AEGLIN) {
	prt("The Broadsword 'Aeglin'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (AEGLOS) {
	prt("The Spear 'Aeglos'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (AGLARANG) {
	prt("The Katana 'Aglarang'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (ANDURIL) {
	prt("The Longsword 'Anduril'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (ANGRIST) {
	prt("The Dagger 'Angrist'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (ANGUIREL) {
	prt("The Longsword 'Anguirel'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (ARUNRUTH) {
	prt("The Broadsword 'Arunruth'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (AULE) {
	prt("The War Hammer of Aule", i++, j);
	artifact_screen_full(&i, j);
    }
    if (AVAVIR) {
	prt("The Scythe 'Avavir'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (BALLI) {
	prt("The Battle Axe of Balli Stonehand", i++, j);
	artifact_screen_full(&i, j);
    }
    if (BARUKKHELED) {
	prt("The Broad Axe 'Barukkheled'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (BELANGIL) {
	prt("The Dagger 'Belangil'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (BLOODSPIKE) {
	prt("The Morningstar 'Bloodspike'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (CALRIS) {
	prt("The Bastard Sword 'Calris'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (CARETH) {
	prt("The Saber 'Careth Asdriag'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (CRISDURIAN) {
	prt("The Executioner's Sword 'Crisdurian'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (DEATHWREAKER) {
	prt("The Mace of Disruption 'Deathwreaker'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (DETHANC) {
	prt("The Dagger 'Dethanc'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (DOOMCALLER) {
	prt("The Blade of Chaos 'Doomcaller'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (DURIN) {
	prt("The Great Axe of Durin", i++, j);
	artifact_screen_full(&i, j);
    }
    if (ELVAGIL) {
	prt("The Longsword 'Elvagil'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (EONWE) {
	prt("The Great Axe of Eonwe", i++, j);
	artifact_screen_full(&i, j);
    }
    if (EORLINGAS) {
	prt("The Lance of Eorlingas", i++, j);
	artifact_screen_full(&i, j);
    }
    if (ERIRIL) {
	prt("The Quarterstaff 'Eriril'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (FIRESTAR) {
	prt("The Morningstar 'Firestar'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (FORASGIL) {
	prt("The Rapier 'Forasgil'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (GILETTAR) {
	prt("The Short Sword 'Gilettar'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (GLAMDRING) {
	prt("The Broadsword 'Glamdring'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (GONDRICAM) {
	prt("The Cutlass 'Gondricam'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (GURTHANG) {
	prt("The Two-Handed Sword 'Gurthang'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (HARADEKKET) {
	prt("The Scimitar 'Haradekket'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (LOTHARANG) {
	prt("The Battle Axe 'Lotharang'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (MORMEGIL) {
	prt("The Two-Handed Sword 'Mormegil'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (MUNDWINE) {
	prt("The Lochaber Axe 'Mundwine'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (NAR) {
	prt("The Quarterstaff 'Nar-i-vagil'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (NARTHANC) {
	prt("The Dagger 'Narthanc'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (NIMLOTH) {
	prt("The Spear 'Nimloth'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (NIMTHANC) {
	prt("The Dagger 'Nimthanc'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (ORCRIST) {
	prt("The Broadsword 'Orcrist'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (OROME) {
	prt("The Spear 'Orome'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (OSONDIR) {
	prt("The Halberd 'Osondir'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (PAIN) {
	prt("The Glaive of Pain", i++, j);
	artifact_screen_full(&i, j);
    }
    if (RILIA) {
	prt("The Dagger of Rilia", i++, j);
	artifact_screen_full(&i, j);
    }
    if (RINGIL) {
	prt("The Longsword 'Ringil'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (STING) {
	prt("The Short Sword 'Sting'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (TARATOL) {
	prt("The Mace 'Taratol'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (THEODEN) {
	prt("The Beaked Axe of Theoden", i++, j);
	artifact_screen_full(&i, j);
    }
    if (THUNDERFIST) {
	prt("The Two-Handed Great Flail 'Thunderfist'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (TIL) {
	prt("The Pike 'Til-i-arc'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (TOTILA) {
	prt("The Flail 'Totila'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (TURMIL) {
	prt("The Lucerne Hammer 'Turmil'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (ULMO) {
	prt("The Trident of Ulmo", i++, j);
	artifact_screen_full(&i, j);
    }
    if (WRATH) {
	prt("The Trident of Wrath", i++, j);
	artifact_screen_full(&i, j);
    }
    if (ZARCUTHRA) {
	prt("The Two-Handed Sword 'Zarcuthra'", i++, j);
	artifact_screen_full(&i, j);
    }
/* missle weapons */

    if (BARD) {
	prt("The Longbow of Bard", i++, j);
	artifact_screen_full(&i, j);
    }
    if (BELEG) {
	prt("The Longbow of Beleg Cuthalion", i++, j);
	artifact_screen_full(&i, j);
    }
    if (CUBRAGOL) {
	prt("The Light Crossbow 'Cubragol'", i++, j);
	artifact_screen_full(&i, j);
    }

/* cloaks */

    if (COLANNON) {
	prt("The Cloak 'Colannon'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (COLLUIN) {
	prt("The Cloak 'Colluin'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (HOLCOLLETH) {
	prt("The Cloak 'Holcolleth'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (LUTHIEN) {
	prt("The Shadow Cloak of Luthien", i++, j);
	artifact_screen_full(&i, j);
    }
    if (THINGOL) {
	prt("The Cloak of Thingol", i++, j);
	artifact_screen_full(&i, j);
    }
    if (THORONGIL) {
	prt("The Cloak of Thorongil", i++, j);
	artifact_screen_full(&i, j);
    }
    if (TUOR) {
	prt("The Shadow Cloak of Tuor", i++, j);
	artifact_screen_full(&i, j);
    }

/* armor */

    if (ANARION) {
	prt("The Large Metal Shield of Anarion", i++, j);
	artifact_screen_full(&i, j);
    }
    if (ARVEDUI) {
	prt("The Chain Mail of Arvedui", i++, j);
	artifact_screen_full(&i, j);
    }
    if (BELEGENNON) {
	prt("The Mithril Chain Mail 'Belegennon'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (BERUTHIEL) {
	prt("The Iron Crown of Beruthiel", i++, j);
	artifact_screen_full(&i, j);
    }
    if (BLADETURNER) {
	prt("The Power Dragon Scale Mail 'Bladeturner'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (CAMBELEG) {
	prt("The Leather Gloves 'Cambeleg'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (CAMLOST) {
	prt("The Gauntlets 'Camlost'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (CAMMITHRIM) {
	prt("The Leather Gloves 'Cammithrin'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (CASPANION) {
	prt("The Augmented Chain Mail of Caspanion", i++, j);
	artifact_screen_full(&i, j);
    }
    if (CELEBORN) {
	prt("The Mithril Plate Mail of Celeborn", i++, j);
	artifact_screen_full(&i, j);
    }
    if (CELEGORM) {
	prt("The Large Leather Shield of Celegorm", i++, j);
	artifact_screen_full(&i, j);
    }
    if (DAL) {
	prt("The Soft Leather Boots 'Dal-i-thalion'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (DOR_LOMIN) {
	prt("The Iron Helm of Dor-Lomin", i++, j);
	artifact_screen_full(&i, j);
    }
    if (FEANOR) {
	prt("The Hard Leather Boots of Feanor", i++, j);
	artifact_screen_full(&i, j);
    }
    if (FINGOLFIN) {
	prt("The Set of Cesti of Fingolfin", i++, j);
	artifact_screen_full(&i, j);
    }
    if (GONDOR) {
	prt("The Golden Crown of Gondor", i++, j);
	artifact_screen_full(&i, j);
    }
    if (GORLIM) {
	prt("The Iron Helm of Gorlim", i++, j);
	artifact_screen_full(&i, j);
    }
    if (HAMMERHAND) {
	prt("The Steel Helm 'Hammerhand'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (HITHLOMIR) {
	prt("The Soft Leather Armor 'Hithlomir'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (HOLHENNETH) {
	prt("The Iron Helm 'Holhenneth'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (ISILDUR) {
	prt("The Full Plate Armor of Isildur", i++, j);
	artifact_screen_full(&i, j);
    }
    if (PAURAEGEN) {
	prt("The Gauntlets 'Pauragen'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (PAURHACH) {
	prt("The Gauntlets 'Paurhach'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (PAURNEN) {
	prt("The Gauntlets 'Paurnen'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (PAURNIMMEN) {
	prt("The Gauntlets 'Paurnimmen'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (RAZORBACK) {
	prt("The Multi-Hued Dragon Scale Mail 'Razorback'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (ROHAN) {
	prt("The Metal Brigandine Armor of Rohan", i++, j);
	artifact_screen_full(&i, j);
    }
    if (SOULKEEPER) {
	prt("The Adamantine Plate Mail 'Soulkeeper'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (THALKETTOTH) {
	prt("The Leather Scale Mail 'Thalkettoth'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (THENGEL) {
	prt("The Metal Cap of Thengel", i++, j);
	artifact_screen_full(&i, j);
    }
    if (THORIN) {
	prt("The Small Metal Shield of Thorin", i++, j);
	artifact_screen_full(&i, j);
    }
    if (THRANDUIL) {
	prt("The Hard Leather Cap of Thranduil", i++, j);
	artifact_screen_full(&i, j);
    }
    if (THROR) {
	prt("The Pair of Metal Shod Boots of Thror", i++, j);
	artifact_screen_full(&i, j);
    }

/* amulets and necklaces */

    if (CARLAMMAS) {
	prt("The Amulet of Carlammas", i++, j);
	artifact_screen_full(&i, j);
    }
    if (INGWE) {
	prt("The Amulet of Ingwe", i++, j);
	artifact_screen_full(&i, j);
    }
    if (NECKLACE) {
	prt("The Amulet of the Dwarves", i++, j);
	artifact_screen_full(&i, j);
    }

/* light sources */

    if (PHIAL) {
	prt("The Phial of Galadriel", i++, j);
	artifact_screen_full(&i, j);
    }
    if (ELENDIL) {
	prt("The Star of Elendil", i++, j);
	artifact_screen_full(&i, j);
    }
    if (THRAIN) {
	prt("The Arkenstone of Thrain", i++, j);
	artifact_screen_full(&i, j);
    }

/* rings */

    if (BARAHIR) {
	prt("The Ring of Barahir", i++, j);
	artifact_screen_full(&i, j);
    }
    if (TULKAS) {
	prt("The Ring of Tulkas", i++, j);
	artifact_screen_full(&i, j);
    }
    if (NARYA) {
	prt("The Ring of Power 'Narya'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (NENYA) {
	prt("The Ring of Power 'Nenya'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (VILYA) {
	prt("The Ring of Power 'Vilya'", i++, j);
	artifact_screen_full(&i, j);
    }
    if (POWER) {
	prt("The One Ring", i++, j);
	artifact_screen_full(&i, j);
    }

/* morgoth's stuff */

    if (GROND) {
	prt("The Lead-Filled Mace 'Grond'", i++, j);
	artifact_screen_full(&i, j);
    }
    pause_line(i);
    restore_screen();
}


/* print out the status of uniques - cba */

void
#ifdef __STDC__
unique_screen_full(int *i, int j)
#else
unique_screen_full(i, j)
    int                *i;
    int                 j;

#endif
{
    int                 t;

/* is screen full? */
    if (*i == 22) {
	prt("-- more --", *i, j);
	inkey();
	for (t = 2; t < 23; t++)
	    erase_line(t, j);	   /* don't forget to erase extra */
	prt("Uniques: (continued)", 1, j + 5);
	*i = 2;
    }
}

void
#ifdef __STDC__
check_uniques(void)
#else
check_uniques()
#endif
{
    int                 i, j, k;
    bigvtype            msg;

    save_screen();
    j = 15;

    for (i = 1; i < 23; i++)
	erase_line(i, j - 2);

    i = 1;
    prt("Uniques:", i++, j + 5);

    for (k = 0; k < MAX_CREATURES; k++) {
	if (strlen(c_list[k].name) > 0) {
	    if (wizard) {
		sprintf(msg, "%s is %s", c_list[k].name,
			(u_list[k].dead) ? "DEAD" : "ALIVE");
		prt(msg, i++, j);
		unique_screen_full(&i, j);
	    } else if (u_list[k].dead) {
		sprintf(msg, "%s is DEAD", c_list[k].name);
		prt(msg, i++, j);
		unique_screen_full(&i, j);
	    }
	}
    }
    pause_line(i);
    restore_screen();
}
