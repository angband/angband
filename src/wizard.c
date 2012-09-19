/* wizard.c: Version history and info, and wizard mode debugging aids.

   Copyright (c) 1989 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

#include <stdio.h>
#include "constant.h"
#include "config.h"
#include "types.h"
#include "externs.h"

#ifdef USG
#ifndef ATARIST_MWC
#include <string.h>
#endif
#else
#include <strings.h>
#endif

long atol();

int is_wizard(uid)
  int uid;
{
  FILE *fp;
  char buf[100];
  int test;

  if ((fp=fopen(ANGBAND_WIZ, "r"))==NULL) {
    fprintf(stderr, "Can't get wizard check...");
    exit_game();
  }
  do {
    (void) fgets(buf, sizeof buf, fp);
    if (sscanf(buf, "%d", &test)) {
      if (test==uid && buf[0]!='#') {
	fclose(fp);
	return TRUE;
      }
    }
  } while (!feof(fp));
  fclose(fp);
  return FALSE;
}

/* Check to see which artifacts have been seen		*/
void artifact_check()
{
  FILE *file1;
  vtype filename;
  {
    prt("Checking for artifacts that have been seen... ", 0, 0);
    prt("File name: ", 0, 0);
    if (get_string(filename, 0, 11, 64))
      {
	if (strlen(filename) == 0)
	  return;
	if ((file1 = fopen(filename, "w")) != NULL)
	  {
	    (void) fprintf(file1, "Artifacts that have been seen\n");
	    (void) fprintf(file1, "\n");
	    if (GROND) fprintf(file1, "Grond\n");
	    if (RINGIL) fprintf(file1, "Ringil\n");
	    if (AEGLOS) fprintf(file1, "Aeglos\n");
	    if (ARUNRUTH) fprintf(file1, "Arunruth\n");
	    if (MORMEGIL) fprintf(file1, "Mormegil\n");
	    if (ANGRIST) fprintf(file1, "Angrist\n");
	    if (GURTHANG) fprintf(file1, "Gurthang\n");
	    if (CALRIS) fprintf(file1, "Calris\n");
	    if (ANDURIL) fprintf(file1, "Anduril\n");
	    if (STING) fprintf(file1, "Sting\n");
	    if (ORCRIST) fprintf(file1, "Orcrist\n");
	    if (GLAMDRING) fprintf(file1, "Glamdring\n");
	    if (DURIN) fprintf(file1, "Durin\n");
	    if (AULE) fprintf(file1, "Aule\n");
	    if (THUNDERFIST) fprintf(file1, "Thunderfist\n");
	    if (BLOODSPIKE) fprintf(file1, "Bloodspike\n");
	    if (DOOMCALLER) fprintf(file1, "Doomcaller\n");
	    if (NARTHANC) fprintf(file1, "Narthanc\n");
	    if (NIMTHANC) fprintf(file1, "Nimthanc\n");
	    if (DETHANC) fprintf(file1, "Dethanc\n");
	    if (GILETTAR) fprintf(file1, "Gilettar\n");
	    if (RILIA) fprintf(file1, "Rilia\n");
	    if (BELANGIL) fprintf(file1, "Belangil\n");
	    if (BALLI) fprintf(file1, "Balli Stonehand\n");
	    if (LOTHARANG) fprintf(file1, "Lotharang\n");
	    if (FIRESTAR) fprintf(file1, "Firestar\n");
	    if (ERIRIL) fprintf(file1, "Eriril\n");
	    if (CUBRAGOL) fprintf(file1, "Cubragol\n");
	    if (BARD) fprintf(file1, "Longbow of Bard\n");
	    if (COLLUIN) fprintf(file1, "Colluin\n");
	    if (HOLCOLLETH) fprintf(file1, "Holcolleth\n");
	    if (TOTILA) fprintf(file1, "Totila\n");
	    if (PAIN) fprintf(file1, "Glaive of Pain\n");
	    if (ELVAGIL) fprintf(file1, "Elvagil\n");
	    if (AGLARANG) fprintf(file1, "Aglarang\n");
	    if (EORLINGAS) fprintf(file1, "Eorlingas\n");
	    if (BARUKKHELED) fprintf(file1, "Barukkheled\n");
	    if (WRATH) fprintf(file1, "Trident of Wrath\n");
	    if (HARADEKKET) fprintf(file1, "Haradekket\n");
	    if (MUNDWINE) fprintf(file1, "Mundwine\n");
	    if (GONDRICAM) fprintf(file1, "Gondricam\n");
	    if (ZARCUTHRA) fprintf(file1, "Zarcuthra\n");
	    if (CARETH) fprintf(file1, "Careth Asdriag\n");
	    if (FORASGIL) fprintf(file1, "Forasgil\n");
	    if (CRISDURIAN) fprintf(file1, "Crisdurian\n");
	    if (COLANNON) fprintf(file1, "Colannon\n");
	    if (HITHLOMIR) fprintf(file1, "Hithlomir\n");
	    if (THALKETTOTH) fprintf(file1, "Thalkettoth\n");
	    if (ARVEDUI) fprintf(file1, "Arvedui\n");
	    if (THRANDUIL) fprintf(file1, "Thranduil\n");
	    if (THENGEL) fprintf(file1, "Thengel\n");
	    if (HAMMERHAND) fprintf(file1, "Hammerhand\n");
       	    if (CELEFARN) fprintf(file1, "Celefarn\n");
	    if (THROR) fprintf(file1, "Thror\n");
	    if (MAEDHROS) fprintf(file1, "Maedhros\n");
	    if (OLORIN) fprintf(file1, "Olorin\n");
	    if (ANGUIREL) fprintf(file1, "Anguirel\n");
	    if (OROME) fprintf(file1, "Orome\n");
	    if (EONWE) fprintf(file1, "Eonwe\n");
	    if (THEODEN) fprintf(file1, "Theoden\n");
	    if (ULMO) fprintf(file1, "Trident of Ulmo\n");
	    if (OSONDIR) fprintf(file1, "Osondir\n");
	    if (TURMIL) fprintf(file1, "Turmil\n");
	    if (TIL) fprintf(file1, "Til-i-arc\n");
	    if (DEATHWREAKER) fprintf(file1, "Deathwreaker\n");
	    if (AVAVIR) fprintf(file1, "Avavir\n");
	    if (TARATOL) fprintf(file1, "Taratol\n");
	    if (DOR_LOMIN) fprintf(file1, "Dor-Lomin\n");
	    if (BELEGENNON) fprintf(file1, "Belegennon\n");
	    if (FEANOR) fprintf(file1, "Feanor\n");
	    if (ISILDUR) fprintf(file1, "Isildur\n");
	    if (SOULKEEPER) fprintf(file1, "Soulkeeper\n");
	    if (FINGOLFIN) fprintf(file1, "Fingolfin\n");
	    if (ANARION) fprintf(file1, "Anarion\n");
	    if (BELEG) fprintf(file1, "Beleg Cuthalion\n");
	    if (DAL) fprintf(file1, "Dal-i-thalion\n");
	    if (PAURHACH) fprintf(file1, "Paurhach\n");
	    if (PAURNIMMEN) fprintf(file1, "Paurnimmen\n");
	    if (PAURAEGEN) fprintf(file1, "Pauragen\n");
	    if (PAURNEN) fprintf(file1, "Paurnen\n");
	    if (CAMMITHRIM) fprintf(file1, "Cammithrin\n");
	    if (CAMBELEG) fprintf(file1, "Cambeleg\n");
	    if (HOLHENNETH) fprintf(file1, "Holhenneth\n");
	    if (AEGLIN) fprintf(file1, "Aeglin\n");
	    if (CAMLOST) fprintf(file1, "Camlost\n");
	    if (NIMLOTH) fprintf(file1, "Nimloth\n");
	    if (NAR) fprintf(file1, "Nar-i-vagil\n");
	    if (BERUTHIEL) fprintf(file1, "Beruthiel\n");
	    if (GORLIM) fprintf(file1, "Gorlim\n");
	    if (THORIN) fprintf(file1, "Thorin\n");
	    if (CELEBORN) fprintf(file1, "Celeborn\n");
	    if (GONDOR) fprintf(file1, "Gondor\n");
	    if (THINGOL) fprintf(file1, "Thingol\n");
	    if (THORONGIL) fprintf(file1, "Thorongil\n");
	    if (LUTHIEN) fprintf(file1, "Luthien\n");
	    if (TUOR) fprintf(file1, "Tuor\n");
	    if (ROHAN) fprintf(file1, "Rohan\n");
	    if (CASPANION) fprintf(file1, "Caspanion\n");
	    if (NARYA) fprintf(file1, "Narya\n");
	    if (NENYA) fprintf(file1, "Nenya\n");
	    if (VILYA) fprintf(file1, "Vilya\n");
	    if (POWER) fprintf(file1, "The One Ring\n");
	    if (PHIAL) fprintf(file1, "The Phial of Galadriel\n");
	    if (INGWE) fprintf(file1, "The Amulet of Ingwe\n");
	    if (CARLAMMAS) fprintf(file1, "The Amulet of Carlammas\n");
	    if (TULKAS) fprintf(file1, "The Ring of Tulkas\n");
	    if (NECKLACE) fprintf(file1, "The Amulet of the Dwarves\n");
	    if (BARAHIR) fprintf(file1, "The Ring of Barahir\n");
	    if (ELENDIL) fprintf(file1, "The Star of Elendil\n");
	    if (THRAIN) fprintf(file1, "The Arkenstone of Thrain\n");
	    if (RAZORBACK) fprintf(file1, "Razorback\n");
	    if (BLADETURNER) fprintf(file1, "Bladeturner\n");
          }
	(void) fclose(file1);
	prt("Done...", 0, 0);
      }
    else
      prt("File could not be opened.", 0, 0);
  }
}

/* Light up the dungeon					-RAK-	*/
void wizard_light(light)
  int light;
{
  register cave_type *c_ptr;
  register int k, l, i, j;
  int flag;

  if (!light) {
    if (cave[char_row][char_col].pl)
      flag = FALSE;
    else
      flag = TRUE;
  } else {
    flag = (light>0)?1:0;
  }
  for (i = 0; i < cur_height; i++)
    for (j = 0; j < cur_width; j++)
      if (cave[i][j].fval <= MAX_CAVE_FLOOR)
	for (k = i-1; k <= i+1; k++)
	  for (l = j-1; l <= j+1; l++)
	    {
	      c_ptr = &cave[k][l];
	      c_ptr->pl = flag;
	      if (!flag)
		c_ptr->fm = FALSE;
	    }
  prt_map();
}


/* Wizard routine for gaining on stats			-RAK-	*/
void change_character()
{
  register int tmp_val;
  register int32 tmp_lval;
  int16u *a_ptr;
  vtype tmp_str;
  register struct misc *m_ptr;

  a_ptr = py.stats.max_stat;
  prt("(3 - 118) Strength     = ", 0, 0);
  if (get_string(tmp_str, 0, 25, 3))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > 2) && (tmp_val < 119))
	{
	  a_ptr[A_STR] = tmp_val;
	  (void) res_stat(A_STR);
	}
    }
  else
    return;

  prt("(3 - 118) Intelligence = ", 0, 0);
  if (get_string(tmp_str, 0, 25, 3))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > 2) && (tmp_val < 119))
	{
	  a_ptr[A_INT] = tmp_val;
	  (void) res_stat(A_INT);
	}
    }
  else
    return;

  prt("(3 - 118) Wisdom       = ", 0, 0);
  if (get_string(tmp_str, 0, 25, 3))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > 2) && (tmp_val < 119))
	{
	  a_ptr[A_WIS] = tmp_val;
	  (void) res_stat(A_WIS);
	}
    }
  else
    return;

  prt("(3 - 118) Dexterity    = ", 0, 0);
  if (get_string(tmp_str, 0, 25, 3))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > 2) && (tmp_val < 119))
	{
	  a_ptr[A_DEX] = tmp_val;
	  (void) res_stat(A_DEX);
	}
    }
  else
    return;

  prt("(3 - 118) Constitution = ", 0, 0);
  if (get_string(tmp_str, 0, 25, 3))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > 2) && (tmp_val < 119))
	{
	  a_ptr[A_CON] = tmp_val;
	  (void) res_stat(A_CON);
	}
    }
  else
    return;

  prt("(3 - 118) Charisma     = ", 0, 0);
  if (get_string(tmp_str, 0, 25, 3))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > 2) && (tmp_val < 119))
	{
	  a_ptr[A_CHR] = tmp_val;
	  (void) res_stat(A_CHR);
	}
    }
  else
    return;

  m_ptr = &py.misc;
  prt("(1 - 32767) Hit points = ", 0, 0);
  if (get_string(tmp_str, 0, 25, 5))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > 0) && (tmp_val <= MAX_SHORT))
	{
	  m_ptr->mhp  = tmp_val;
	  m_ptr->chp  = tmp_val;
	  m_ptr->chp_frac = 0;
	  prt_mhp();
	  prt_chp();
	}
    }
  else
    return;

  prt("(0 - 32767) Mana       = ", 0, 0);
  if (get_string(tmp_str, 0, 25, 5))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > -1) && (tmp_val <= MAX_SHORT) && (*tmp_str != '\0'))
	{
	  m_ptr->mana  = tmp_val;
	  m_ptr->cmana = tmp_val;
	  m_ptr->cmana_frac = 0;
	  prt_cmana();
	}
    }
  else
    return;

  (void) sprintf(tmp_str, "Current=%ld  Gold = ", m_ptr->au);
  tmp_val = strlen(tmp_str);
  prt(tmp_str, 0, 0);
  if (get_string(tmp_str, 0, tmp_val, 7))
    {
      tmp_lval = atol(tmp_str);
      if (tmp_lval > -1 && (*tmp_str != '\0'))
	{
	  m_ptr->au = tmp_lval;
	  prt_gold();
	}
    }
  else
    return;
  (void) sprintf(tmp_str, "Current=%ld  Max Exp = ", m_ptr->max_exp);
  tmp_val = strlen(tmp_str);
  prt(tmp_str, 0, 0);
  if (get_string(tmp_str, 0, tmp_val, 7))
    {
      tmp_lval = atol(tmp_str);
      if (tmp_lval > -1 && (*tmp_str != '\0'))
	{
	  m_ptr->max_exp = tmp_lval;
	  prt_experience();
	}
    }
  else
    return;

  (void) sprintf(tmp_str, "Current=%d  (0-200) Searching = ", m_ptr->srh);
  tmp_val = strlen(tmp_str);
  prt(tmp_str, 0, 0);
  if (get_string(tmp_str, 0, tmp_val, 3))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0'))
	m_ptr->srh  = tmp_val;
    }
  else
    return;

  (void) sprintf(tmp_str, "Current=%d  (-1-18) Stealth = ", m_ptr->stl);
  tmp_val = strlen(tmp_str);
  prt(tmp_str, 0, 0);
  if (get_string(tmp_str, 0, tmp_val, 3))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > -2) && (tmp_val < 19) && (*tmp_str != '\0'))
	m_ptr->stl  = tmp_val;
    }
  else
    return;

  (void) sprintf(tmp_str, "Current=%d  (0-200) Disarming = ", m_ptr->disarm);
  tmp_val = strlen(tmp_str);
  prt(tmp_str, 0, 0);
  if (get_string(tmp_str, 0, tmp_val, 3))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0'))
	m_ptr->disarm = tmp_val;
    }
  else
    return;

  (void) sprintf(tmp_str, "Current=%d  (0-100) Save = ", m_ptr->save);
  tmp_val = strlen(tmp_str);
  prt(tmp_str, 0, 0);
  if (get_string(tmp_str, 0, tmp_val, 3))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0'))
	m_ptr->save = tmp_val;
    }
  else
    return;

  (void) sprintf(tmp_str, "Current=%d  (0-200) Base to hit = ", m_ptr->bth);
  tmp_val = strlen(tmp_str);
  prt(tmp_str, 0, 0);
  if (get_string(tmp_str, 0, tmp_val, 3))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0'))
	m_ptr->bth  = tmp_val;
    }
  else
    return;

  (void) sprintf(tmp_str, "Current=%d  (0-200) Bows/Throwing = ", m_ptr->bthb);
  tmp_val = strlen(tmp_str);
  prt(tmp_str, 0, 0);
  if (get_string(tmp_str, 0, tmp_val, 3))
    {
      tmp_val = atoi(tmp_str);
      if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0'))
	m_ptr->bthb = tmp_val;
    }
  else
    return;

  (void) sprintf(tmp_str, "Current=%d  Weight = ", m_ptr->wt);
  tmp_val = strlen(tmp_str);
  prt(tmp_str, 0, 0);
  if (get_string(tmp_str, 0, tmp_val, 3))
    {
      tmp_val = atoi(tmp_str);
      if (tmp_val > -1 && (*tmp_str != '\0'))
	m_ptr->wt = tmp_val;
    }
  else
    return;

  while(get_com("Alter speed? (+/-)", tmp_str))
    {
      if (*tmp_str == '+')
	change_speed(-1);
      else if (*tmp_str == '-')
	change_speed(1);
      else
	break;
      prt_speed();
    }
}


/* Wizard routine for creating objects			-RAK-	*/
void wizard_create()
{
  register int tmp_val;
  int flag, i, j, k;
  int32 tmp_lval;
  char tmp_str[100];
  register inven_type *i_ptr;
  treasure_type t_type, *t_ptr;
  inven_type forge;
  register cave_type *c_ptr;
  char pattern[4];
  char ch;
  int more = FALSE, where;

  t_ptr = &t_type;
  i_ptr = &forge;
  i_ptr->name2 = 0;
  i_ptr->ident = ID_KNOWN2|ID_STOREBOUGHT;

  save_screen();
  prt("What type of item?    : ", 0, 0);
  prt("[W]eapon, [A]rmour, [O]thers.", 1, 0);
  if (!get_com((char *)0, &ch))
    {restore_screen();return;}
  switch (ch) {
  case 'W':
  case 'w':
    prt("What type of Weapon?    : ", 0, 0);
    prt("[S]word, [H]afted, [P]olearm, [B]ow, [A]mmo.", 1, 0);
    if (!get_com((char *)0, &ch))
      {restore_screen();return;}
    switch (ch) {
    case 'S':
    case 's':
      i_ptr->tval=TV_SWORD;
      break;
    case 'H':
    case 'h':
      i_ptr->tval=TV_HAFTED;
      break;
    case 'P':
    case 'p':
      i_ptr->tval=TV_POLEARM;
      break;
    case 'B':
    case 'b':
      i_ptr->tval=TV_BOW;
      break;
    case 'A':
    case 'a':
      prt("What type of Ammo?    : ", 0, 0);
      prt("[A]rrow, [B]olt, [P]ebble.", 1, 0);
      if (!get_com((char *)0, &ch))
	{restore_screen();return;}
      switch (ch) {
      case 'A':
      case 'a':
	i_ptr->tval=TV_ARROW;
	break;
      case 'B':
      case 'b':
	i_ptr->tval=TV_BOLT;
	break;
      case 'P':
      case 'p':
	i_ptr->tval=TV_SLING_AMMO;
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
    if (!get_com((char *)0, &ch))
      {restore_screen();return;}
    switch (ch) {
    case 'S':
    case 's':
      i_ptr->tval=TV_SHIELD;
      break;
    case 'H':
    case 'h':
      i_ptr->tval=TV_HELM;
      break;
    case 'G':
    case 'g':
      i_ptr->tval=TV_GLOVES;
      break;
    case 'B':
    case 'b':
      i_ptr->tval=TV_BOOTS;
      break;
    case 'C':
    case 'c':
      i_ptr->tval=TV_CLOAK;
      break;
    case 'A':
    case 'a':
      prt("What type of Armour?    : ", 0, 0);
      prt("[H]ard armour, [S]oft armour.", 1, 0);
      if (!get_com((char *)0, &ch))
	{restore_screen();return;}
      switch (ch) {
      case 'H':
      case 'h':
	i_ptr->tval=TV_HARD_ARMOR;
	break;
      case 'S':
      case 's':
	i_ptr->tval=TV_SOFT_ARMOR;
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
    if (!get_com((char *)0, &ch))
      {restore_screen();return;}
    switch (ch) {
    case 'R':
    case 'r':
      i_ptr->tval=TV_RING;
      break;
    case 'P':
    case 'p':
      i_ptr->tval=TV_POTION1;
      break;
    case 'S':
    case 's':
      i_ptr->tval=TV_SCROLL1;
      break;
    case 'A':
    case 'a':
      i_ptr->tval=TV_AMULET;
      break;
    case 'W':
    case 'w':
      prt("Wand, Staff or Rod?    : ", 0, 0);
      prt("[W]and, [S]taff, [R]od.", 1, 0);
      if (!get_com((char *)0, &ch))
	{restore_screen();return;}
      switch (ch) {
      case 'W':
      case 'w':
	i_ptr->tval=TV_WAND;
	break;
      case 'S':
      case 's':
	i_ptr->tval=TV_STAFF;
	break;
      case 'R':
      case 'r':
	i_ptr->tval=TV_ROD;
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
      if (!get_com((char *)0, &ch))
	{restore_screen();return;}
      switch (ch) {
      case 'P':
      case 'p':
	i_ptr->tval=TV_PRAYER_BOOK;
	break;
      case 'S':
      case 's':
	i_ptr->tval=TV_MAGIC_BOOK;
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
      if (!get_com((char *)0, &ch))
	{restore_screen();return;}
      switch (ch) {
      case 'S':
      case 's':
	i_ptr->tval=TV_SPIKE;
	break;
      case 'd':
      case 'D':
	i_ptr->tval=TV_DIGGING;
	break;
      case 'C':
      case 'c':
	i_ptr->tval=TV_CHEST;
	break;
      case 'L':
      case 'l':
	i_ptr->tval=TV_LIGHT;
	break;
      case 'F':
      case 'f':
	i_ptr->tval=TV_FOOD;
	break;
      case 'O':
      case 'o':
	i_ptr->tval=TV_FLASK;
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

  j=0;
  i=0;
  k=0;
 again:
  restore_screen();
  save_screen();
  prt("Which Item?  : ", 0, 0);
  for (; i<MAX_DUNGEON_OBJ; i++) {
    switch (i_ptr->tval) {
    case TV_POTION1:
      if ((object_list[i].tval == TV_POTION1) ||
          (object_list[i].tval == TV_POTION2)) {
         sprintf(tmp_str, "%c) %s", 'a'+j, object_list[i].name);
         prt(tmp_str, 1+j, 0);
         j++;
      }
      break;
    case TV_SCROLL1:
      if ((object_list[i].tval == TV_SCROLL1) ||
          (object_list[i].tval == TV_SCROLL2)) {
         sprintf(tmp_str, "%c) %s", 'a'+j, object_list[i].name);
         prt(tmp_str, 1+j, 0);
         j++;
      }
      break;
    default:
      if (object_list[i].tval == i_ptr->tval) {
         sprintf(tmp_str, "%c) %s", 'a'+j, object_list[i].name);
         prt(tmp_str, 1+j, 0);
         j++;
      }
      break;
    }
    if (j==21) {
      more = TRUE;
      break;
    }
  }
  if (j<21) {
    for (i=(i-(MAX_DUNGEON_OBJ-1))+(SPECIAL_OBJ-1); i<MAX_OBJECTS; i++) {
      switch (i_ptr->tval) {
      case TV_POTION1:
        if ((object_list[i].tval == TV_POTION1) ||
            (object_list[i].tval == TV_POTION2)) {
           sprintf(tmp_str, "%c) %s", 'a'+j, object_list[i].name);
           prt(tmp_str, 1+j, 0);
           j++;
        }
        break;
      case TV_SCROLL1:
        if ((object_list[i].tval == TV_SCROLL1) ||
            (object_list[i].tval == TV_SCROLL2)) {
           sprintf(tmp_str, "%c) %s", 'a'+j, object_list[i].name);
           prt(tmp_str, 1+j, 0);
           j++;
        }
        break;
      default:
        if (object_list[i].tval == i_ptr->tval) {
           sprintf(tmp_str, "%c) %s", 'a'+j, object_list[i].name);
           prt(tmp_str, 1+j, 0);
           j++;
        }
        break;
      }
      if (j==21) {
	more=TRUE;
	break;
      }
    }
  }
  if (more) prt("v) NEXT PAGE", 22, 0);

  do {
    if (!get_com((char *)0, &ch))
      {restore_screen();return;}
  } while ((ch<'a' && ch>('a'+j))||(more && ch<'a' && ch>('a'+j+1)));

  if ((ch=='v')&&more) {
    more=FALSE;
    k+=(j-1);
    j=0;
    goto again;
  }

  k+=(ch-'a'+1);

  j=0;
  for (i=0; i<MAX_DUNGEON_OBJ; i++) {
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
    if (j==k) break;
  }
  if (j!=k) {
    for (i=(SPECIAL_OBJ-1); i<MAX_OBJECTS; i++) {
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
      if (j==k) break;
    }
  }

  if (j!=k) {restore_screen();return;}

  invcopy(i_ptr, i);
  i_ptr->timeout=0;
  restore_screen();
  save_screen();

  prt("Number of items? [return=1]: ", 0, 0);
  if (!get_string(tmp_str, 0, 33, 5)) goto end;
  tmp_val = atoi(tmp_str);
  if (tmp_val!=0) i_ptr->number = tmp_val;

  prt("Weight of item? [return=default]: ", 0, 0);
  if (!get_string(tmp_str, 0, 35, 5)) goto end;
  tmp_val = atoi(tmp_str);
  if (tmp_val!=0) i_ptr->weight = tmp_val;

  if ((i_ptr->tval==TV_SWORD) ||
      (i_ptr->tval==TV_HAFTED) ||
      (i_ptr->tval==TV_POLEARM) ||
      (i_ptr->tval==TV_ARROW) ||
      (i_ptr->tval==TV_BOLT) ||
      (i_ptr->tval==TV_SLING_AMMO) ||
      (i_ptr->tval==TV_DIGGING)) {
	i_ptr->ident|=ID_SHOW_HITDAM;
	prt("Damage (dice): ", 0, 0);
	if (!get_string(tmp_str, 0, 15, 3)) goto end;
	tmp_val = atoi(tmp_str);
	if (tmp_val!=0) i_ptr->damage[0] = tmp_val;
	prt("Damage (sides): ", 0, 0);
	if (!get_string(tmp_str, 0, 16, 3)) goto end;
	tmp_val = atoi(tmp_str);
	if (tmp_val!=0) i_ptr->damage[1] = tmp_val;
      }

  prt("+To hit: ", 0, 0);
  if (!get_string(tmp_str, 0, 9, 3)) goto end;
  tmp_val = atoi(tmp_str);
  if (tmp_val!=0) i_ptr->tohit = tmp_val;

  prt("+To dam: ", 0, 0);
  if (!get_string(tmp_str, 0, 9, 3)) goto end;
  tmp_val = atoi(tmp_str);
  if (tmp_val!=0) i_ptr->todam = tmp_val;

  if ((i_ptr->tval==TV_SOFT_ARMOR) ||
      (i_ptr->tval==TV_HARD_ARMOR) ||
      (i_ptr->tval==TV_HELM) ||
      (i_ptr->tval==TV_CLOAK) ||
      (i_ptr->tval==TV_BOOTS) ||
      (i_ptr->tval==TV_GLOVES) ||
      (i_ptr->tval==TV_SHIELD)) {
    prt("Base AC : ", 0, 0);
    if (!get_string(tmp_str, 0, 10, 3)) goto end;
    tmp_val = atoi(tmp_str);
    if (tmp_val!=0) i_ptr->ac = tmp_val;
  }

  prt("+To AC : ", 0, 0);
  if (!get_string(tmp_str, 0, 9, 3)) goto end;
  tmp_val = atoi(tmp_str);
  if (tmp_val!=0) i_ptr->toac = tmp_val;

  prt("Magic Plus Flag  : ", 0, 0);
  if (!get_string(tmp_str, 0, 20, 5)) goto end;
  tmp_val = atoi(tmp_str);
  if (tmp_val!=0) i_ptr->p1 = tmp_val;


  save_screen();

  if ((i_ptr->tval==TV_SWORD) ||
      (i_ptr->tval==TV_HAFTED) ||
      (i_ptr->tval==TV_POLEARM) ||
      (i_ptr->tval==TV_ARROW) ||
      (i_ptr->tval==TV_BOLT) ||
      (i_ptr->tval==TV_SLING_AMMO) ||
      (i_ptr->tval==TV_DIGGING)) {
	if (get_com("Slay Evil? [yn]: ", &ch)) {
	  if (ch=='y'||ch=='Y') i_ptr->flags |= TR_SLAY_EVIL;
	} else if (ch=='\033') goto end;
	if (get_com("Slay Animal? [yn]: ", &ch)) {
	  if (ch=='y'||ch=='Y') i_ptr->flags |= TR_SLAY_ANIMAL;
	} else if (ch=='\033') goto end;
	if (get_com("Slay Undead? [yn]: ", &ch)) {
	  if (ch=='y'||ch=='Y') i_ptr->flags |= TR_SLAY_UNDEAD;
	} else if (ch=='\033') goto end;
	if (get_com("Slay Giant? [yn]: ", &ch)) {
	  if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_SLAY_GIANT;
	} else if (ch=='\033') goto end;
	if (get_com("Slay Demon? [yn]: ", &ch)) {
	  if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_SLAY_DEMON;
	} else if (ch=='\033') goto end;
	if (get_com("Slay Troll? [yn]: ", &ch)) {
	  if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_SLAY_TROLL;
	} else if (ch=='\033') goto end;
	if (get_com("Slay Orc? [yn]: ", &ch)) {
	  if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_SLAY_ORC;
	} else if (ch=='\033') goto end;
	if (get_com("Slay Dragon? [yn]: ", &ch)) {
	  if (ch=='y'||ch=='Y') i_ptr->flags |= TR_SLAY_DRAGON;
	} else if (ch=='\033') goto end;
	if (get_com("Execute Dragon? [yn]: ", &ch)) {
	  if (ch=='y'||ch=='Y') i_ptr->flags |= TR_SLAY_X_DRAGON;
	} else if (ch=='\033') goto end;
	if (get_com("Frost Brand? [yn]: ", &ch)) {
	  if (ch=='y'||ch=='Y') i_ptr->flags |= TR_FROST_BRAND;
	} else if (ch=='\033') goto end;
	if (get_com("Fire Brand? [yn]: ", &ch)) {
	  if (ch=='y'||ch=='Y') i_ptr->flags |= TR_FLAME_TONGUE;
	} else if (ch=='\033') goto end;
	if (get_com("Lightning Brand? [yn]: ", &ch)) {
	  if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_LIGHTNING;
	} else if (ch=='\033') goto end;
	if (get_com("Earthquake Brand? [yn]: ", &ch)) {
	  if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_IMPACT;
	} else if (ch=='\033') goto end;
      }

  if (get_com("Affect Strength? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags |= TR_STR;
  } else if (ch=='\033') goto end;
  if (get_com("Affect Intelligence? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags |= TR_INT;
  } else if (ch=='\033') goto end;
  if (get_com("Affect Wisdom? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags |= TR_WIS;
  } else if (ch=='\033') goto end;
  if (get_com("Affect Dexterity? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags |= TR_DEX;
  } else if (ch=='\033') goto end;
  if (get_com("Affect Constitution? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags |= TR_CON;
  } else if (ch=='\033') goto end;
  if (get_com("Affect Charisma? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags |= TR_CHR;
  } else if (ch=='\033') goto end;
  if (get_com("Automatic Searching? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags |= TR_SEARCH;
  } else if (ch=='\033') goto end;
  if (get_com("Slow Digestion? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags |= TR_SLOW_DIGEST;
  } else if (ch=='\033') goto end;
  if (get_com("Stealth? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags |= TR_STEALTH;
  } else if (ch=='\033') goto end;
  if (get_com("Aggravate Monsters? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags |= TR_AGGRAVATE;
  } else if (ch=='\033') goto end;
  if (get_com("Regeneration? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags |= TR_REGEN;
  } else if (ch=='\033') goto end;
  if (get_com("Speed? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags |= TR_SPEED;
  } else if (ch=='\033') goto end;
  if (get_com("Resist Fire? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags |= TR_RES_FIRE;
  } else if (ch=='\033') goto end;
  if (get_com("Resist Cold? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags |= TR_RES_COLD;
  } else if (ch=='\033') goto end;
  if (get_com("Resist Acid? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags |= TR_RES_ACID;
  } else if (ch=='\033') goto end;
  if (get_com("Resist Lightning? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags |= TR_RES_LIGHT;
  } else if (ch=='\033') goto end;
  if (get_com("Resist Poison? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags |= TR_POISON;
  } else if (ch=='\033') goto end;
  if (get_com("Resist Confusion? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_RES_CONF;
  } else if (ch=='\033') goto end;
  if (get_com("Resist Sound? [yn]: ",&ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_RES_SOUND;
  } else if (ch=='\033') goto end;
  if (get_com("Resist Light? [yn]: ",&ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_RES_LT;
  } else if (ch=='\033') goto end;
  if (get_com("Resist Dark? [yn]: ",&ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_RES_DARK;
  } else if (ch=='\033') goto end;
  if (get_com("Resist Chaos? [yn]: ",&ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_RES_CHAOS;
  } else if (ch=='\033') goto end;
  if (get_com("Resist Disenchantment? [yn]: ",&ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_RES_DISENCHANT;
  } else if (ch=='\033') goto end;
  if (get_com("Resist Shards? [yn]: ",&ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_RES_SHARDS;
  } else if (ch=='\033') goto end;
  if (get_com("Resist Nexus? [yn]: ",&ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_RES_NEXUS;
  } else if (ch=='\033') goto end;
  if (get_com("Resist Nether? [yn]: ",&ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_RES_NETHER;
  } else if (ch=='\033') goto end;
  if (get_com("Resist Blindness? [yn]: ",&ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_RES_BLIND;
  } else if (ch=='\033') goto end;
  if (get_com("Sustain a stat (Magic value 10 for all stats)? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags |= TR_SUST_STAT;
  } else if (ch=='\033') goto end;
  if (get_com("See invisible? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags |= TR_SEE_INVIS;
  } else if (ch=='\033') goto end;
  if (get_com("Free Action? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags |= TR_FREE_ACT;
  } else if (ch=='\033') goto end;
  if (get_com("Feather Falling? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags |= TR_FFALL;
  } else if (ch=='\033') goto end;
  if (get_com("Tunneling? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags |= TR_TUNNEL;
  } else if (ch=='\033') goto end;
  if (get_com("Infra-vision? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags |= TR_INFRA;
  } else if (ch=='\033') goto end;
  if (get_com("Resist life level loss? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_HOLD_LIFE;
  } else if (ch=='\033') goto end;
  if (get_com("Telepathy? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_TELEPATHY;
  } else if (ch=='\033') goto end;
  if (get_com("Immune to Fire? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_IM_FIRE;
  } else if (ch=='\033') goto end;
  if (get_com("Immune to Cold? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_IM_COLD;
  } else if (ch=='\033') goto end;
  if (get_com("Immune to Acid? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_IM_ACID;
  } else if (ch=='\033') goto end;
  if (get_com("Immune to Lightning? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_IM_LIGHT;
  } else if (ch=='\033') goto end;
  if (get_com("Immune to Poison? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_IM_POISON;
  } else if (ch=='\033') goto end;
  if (get_com("Give off Light? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_LIGHT;
  } else if (ch=='\033') goto end;
  if (get_com("Is it an Artifact? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_ARTIFACT;
  } else if (ch=='\033') goto end;
  if (get_com("Active Artifact? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags2 |= TR_ACTIVATE;
  } else if (ch=='\033') goto end;
  if (get_com("Cursed? [yn]: ", &ch)) {
    if (ch=='y'||ch=='Y') i_ptr->flags |= TR_CURSED;
  } else if (ch=='\033') goto end;

  prt("Cost : ", 0, 0);
  if (!get_string(tmp_str, 0, 9, 8)) {restore_screen();return;}
  tmp_lval = atol(tmp_str);
  if (tmp_val!=0) i_ptr->cost = tmp_lval;

  prt("Dungeon Level on which it is found : ", 0, 0);
  if (!get_string(tmp_str, 0, 39, 3)) {restore_screen();return;}
  tmp_val = atoi(tmp_str);
  if (tmp_val!=0) i_ptr->level = tmp_val;

  j=0;
  i=0;
  k=0;
  more=FALSE;
 SNagain:
  restore_screen();
  save_screen();
  for (; i<SN_ARRAY_SIZE; i++) {
    sprintf(tmp_str, "%c) %s", 'a'+j, special_names[i]);
    prt(tmp_str, 1+j, 0);
    j++;
    if (j==21) {
      more = TRUE;
      break;
    }
  }
  if (more) prt("v) NEXT PAGE", 22, 0);

  do {
    if (!get_com("Please choose a secondary name for the item : ", &ch))
      {restore_screen();return;}
  } while ((ch<'a' && ch>('a'+j))||(more && ch<'a' && ch>('a'+j+1)));

  if ((ch=='v')&&more) {
    more=FALSE;
    k+=(j-1);
    j=0;
    goto SNagain;
  } else {
    i_ptr->name2=k+(ch-'a');
  }
  restore_screen();
  save_screen();

 end:
  if (get_check("Allocate?")) {
    /* delete object first if any, before call popt */
      c_ptr = &cave[char_row][char_col];
      if (c_ptr->tptr != 0)
	(void) delete_object(char_row, char_col);

      store_bought(i_ptr);
      tmp_val = popt();
      t_list[tmp_val] = forge;
      c_ptr->tptr = tmp_val;
      msg_print("Allocated.");
    }
  else
    msg_print("Aborted.");
  restore_screen();
}
