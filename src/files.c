/* files.c: misc code to access files used by Moria

   Copyright (c) 1989 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

#include <stdio.h>
#include <errno.h>
#include <sys/param.h>

#include "constant.h"
#include "config.h"
#include "types.h"
#include "externs.h"

#ifdef MSDOS
extern int errno;
#endif

#if defined(GEMDOS) && (__STDC__ == 0)
#include <access.h>
char *strcat();
#endif

#ifdef USG
#ifndef ATARIST_MWC
#include <string.h>
#include <fcntl.h>
#endif
#else
#include <strings.h>
#ifndef VMS
#include <sys/file.h>
#else
#include <file.h>
#endif
#endif

#ifndef VMS
#if defined(ultrix) || defined(USG)
void exit();
#endif
#endif

/* Not touched for Mac port */
/*
 *  init_scorefile
 *  Open the score file while we still have the setuid privileges.  Later
 *  when the score is being written out, you must be sure to flock the file
 *  so we don't have multiple people trying to write to it at the same time.
 *  Craig Norborg (doc)		Mon Aug 10 16:41:59 EST 1987
 */
init_scorefile()
{
#ifdef SET_UID
  if (1 > (highscore_fd = open(ANGBAND_TOP, O_RDWR | O_CREAT, 0644)))
#else
  if (1 > (highscore_fd = open(ANGBAND_TOP, O_RDWR | O_CREAT, 0666)))
#endif
    {
      (void) fprintf (stderr, "Can't open score file \"%s\"\n", ANGBAND_TOP);
      exit(1);
    }
}


/* Attempt to open the intro file			-RAK-	 */
/* This routine also checks the hours file vs. what time it is	-Doc */
void read_times()
{
  register int i;
  vtype in_line;
  FILE *file1;

#ifdef ANGBAND_HOU
  /* Attempt to read hours.dat.	 If it does not exist,	   */
  /* inform the user so he can tell the wizard about it	 */
  if ((file1 = fopen(ANGBAND_HOU, "r")) != NULL)
    {
      while (fgets(in_line, 80, file1) != NULL)
	if (strlen(in_line) > 3)
	  {
	    if (!strncmp(in_line, "SUN:", 4))
	      (void) strcpy(days[0], in_line);
	    else if (!strncmp(in_line, "MON:", 4))
	      (void) strcpy(days[1], in_line);
	    else if (!strncmp(in_line, "TUE:", 4))
	      (void) strcpy(days[2], in_line);
	    else if (!strncmp(in_line, "WED:", 4))
	      (void) strcpy(days[3], in_line);
	    else if (!strncmp(in_line, "THU:", 4))
	      (void) strcpy(days[4], in_line);
	    else if (!strncmp(in_line, "FRI:", 4))
	      (void) strcpy(days[5], in_line);
	    else if (!strncmp(in_line, "SAT:", 4))
	      (void) strcpy(days[6], in_line);
	  }
      (void) fclose(file1);
    }
  else
    {
      restore_term();
      (void) fprintf(stderr, "There is no hours file \"%s\".\nPlease inform the wizard, %s, so he can correct this!\n", ANGBAND_HOU, WIZARD);
      exit(1);
    }

  /* Check the hours, if closed	then exit. */
  if (!check_time())
    {
      if ((file1 = fopen(ANGBAND_HOU, "r")) != NULL)
	{
	  clear_screen();
	  for (i = 0; fgets(in_line, 80, file1) != NULL; i++)
	    put_buffer(in_line, i, 0);
	  (void) fclose(file1);
	  pause_line(23);
	}
      exit_game();
    }

  /* Print the introduction message, news, etc.		 */
  if ((file1 = fopen(ANGBAND_MOR, "r")) != NULL)
    {
      clear_screen();
      for (i = 0; fgets(in_line, 80, file1) != NULL; i++)
	put_buffer(in_line, i, 0);
      pause_line(23);
      (void) fclose(file1);
    }
#endif
}

/* File perusal.	    -CJS-
   primitive, but portable */
void helpfile(filename)
char *filename;
{
  bigvtype tmp_str;
  FILE *file;
  char input;
  int i;

  file = fopen(filename, "r");
  if (file == NULL)
    {
      (void) sprintf (tmp_str, "Can not find help file \"%s\".\n", filename);
      prt (tmp_str, 0, 0);
      return;
    }

  save_screen();

  while (!feof(file))
    {
      clear_screen();
      for (i = 0; i < 23; i++)
	if (fgets (tmp_str, BIGVTYPESIZ-1, file) != NULL)
	  put_buffer (tmp_str, i, 0);
      prt("[Press any key to continue.]", 23, 23);
      input = inkey();
      if (input == ESCAPE)
	break;
    }

  (void) fclose(file);
  restore_screen();
}

/* Prints a list of random objects to a file.  Note that -RAK-	 */
/* the objects produced is a sampling of objects which		 */
/* be expected to appear on that level.				 */
void print_objects()
{
  register int i;
  int nobj, j, level;
  vtype filename1; bigvtype tmp_str;
  register FILE *file1;
  register inven_type *i_ptr;

  prt("Produce objects on what level?: ", 0, 0);
  level = 0;
  if (!get_string(tmp_str, 0, 32, 10))
    return;
  level = atoi(tmp_str);
  prt("Produce how many objects?: ", 0, 0);
  nobj = 0;
  if (!get_string(tmp_str, 0, 27, 10))
    return;
  nobj = atoi(tmp_str);
  if ((nobj > 0) && (level > -1) && (level < 1201))
    {
      if (nobj > 10000)
	nobj = 10000;
      prt("File name: ", 0, 0);
      if (get_string(filename1, 0, 11, 64))
	{
	  if (strlen(filename1) == 0)
	    return;
	  if ((file1 = fopen(filename1, "w")) != NULL)
	    {
	      (void) sprintf(tmp_str, "%d", nobj);
	      prt(strcat(tmp_str, " random objects being produced..."), 0, 0);
	      put_qio();
	      (void) fprintf(file1, "*** Random Object Sampling:\n");
	      (void) fprintf(file1, "*** %d objects\n", nobj);
	      (void) fprintf(file1, "*** For Level %d\n", level);
	      (void) fprintf(file1, "\n");
	      (void) fprintf(file1, "\n");
	      j = popt();
	      for (i = 0; i < nobj; i++)
		{
		  if (randint(MAX_DUNGEON_OBJ)==1) {
		    int tmp;
		  again:
		    tmp=randint(MAX_OBJECTS-(SPECIAL_OBJ-1))+(SPECIAL_OBJ-1)-1;
		    switch (tmp) {
		    case (SPECIAL_OBJ-1):
		      if (object_list[tmp].level <= level) msg_print("*Remove Curse*");
		      break;
		    case (SPECIAL_OBJ):
		      if (object_list[tmp].level <= level) msg_print("[Resistance of Scarabtarices]");
		      break;
		    case (SPECIAL_OBJ+1):
		      if (object_list[tmp].level <= level) msg_print("[Mordenkainen's Escapes]");
		      break;
		    case (SPECIAL_OBJ+2):
		      if (object_list[tmp].level <= level) msg_print("[Kelek's Grimoire of Power]");
		      break;
		    case (SPECIAL_OBJ+3):
		      if (randint(2)==1) goto again;
		      if (object_list[tmp].level <= level) msg_print("[Tenser's Transformations]");
		      break;
		    case (SPECIAL_OBJ+4):
		      if (randint(2)==1) goto again;
		      if (object_list[tmp].level <= level) msg_print("[Raal's Tome of Destruction]");
		      break;
		    case (SPECIAL_OBJ+5):
		      if (object_list[tmp].level <= level) msg_print("[Ethereal Openings]");
		      break;
		    case (SPECIAL_OBJ+6):
		      if (object_list[tmp].level <= level) msg_print("[Godly Insights]");
		      break;
		    case (SPECIAL_OBJ+7):
		      if (object_list[tmp].level <= level) msg_print("[Purifications and Healing]");
		      break;
		    case (SPECIAL_OBJ+8):
		      if (object_list[tmp].level <= level) msg_print("[Wrath of God]");
		      break;
		    case (SPECIAL_OBJ+9):
		      if (randint(2)>1) goto again;
		      if (object_list[tmp].level <= level) msg_print("[Holy Infusions]");
		      break;               /* End of Books */
		    case (SPECIAL_OBJ+10): /* Narya */
		      if (randint(4)>1) goto again;
		      if (NARYA) goto again;
		      if (object_list[tmp].level <= level) msg_print("Narya");
		      NARYA=TRUE;
		      break;
		    case (SPECIAL_OBJ+11): /* Nenya */
		      if (randint(5)>1) goto again;
		      if (NENYA) goto again;
		      if (object_list[tmp].level <= level) msg_print("Nenya");
		      NENYA=TRUE;
		      break;
		    case (SPECIAL_OBJ+12):/* Vilya */
		      if (randint(6)>1) goto again;
		      if (VILYA) goto again;
		      if (object_list[tmp].level <= level) msg_print("Vilya");
		      VILYA=TRUE;
		      break;
		    case (SPECIAL_OBJ+13):/* The Ring of Power */
		      if (randint(10)>1) goto again;
		      if (POWER) goto again;
		      if (object_list[tmp].level <= level) msg_print("Power (The One Ring)");
		      POWER=TRUE;
		      break;
		    case (SPECIAL_OBJ+14):/* Dragon Scale mails */
		    case (SPECIAL_OBJ+15):
		    case (SPECIAL_OBJ+16):
		    case (SPECIAL_OBJ+17):
		    case (SPECIAL_OBJ+18):
		    case (SPECIAL_OBJ+19):
		    case (SPECIAL_OBJ+20):
		      if (randint(2)>1) goto again;
		      if (object_list[tmp].level <= level) msg_print("Dragon Scale Mail");
		      break;
		    case (SPECIAL_OBJ+23):/* Multi-Hued Dragon Scale Mail */
		      if (randint(3)>1) goto again;
		      if (object_list[tmp].level <= level) msg_print("Multi-Hued Dragon Scale Mail");
		      break;
		    }
		    if (object_list[tmp].level > level)
		      continue;
		    invcopy(&t_list[j], tmp);
		    if (object_list[t_list[j].index].tval==TV_HARD_ARMOR)
		      t_list[j].toac+=10+randint(10);
		    i_ptr = &t_list[j];
		    store_bought(i_ptr);
		    objdes(tmp_str, i_ptr, TRUE);
		  } else {
		    invcopy(&t_list[j],
			    sorted_objects[get_obj_num(level, FALSE)]);
		    magic_treasure(j, level, TRUE, FALSE);
		    i_ptr = &t_list[j];
		    store_bought(i_ptr);
		    if (i_ptr->flags & TR_CURSED)
		      add_inscribe(i_ptr, ID_DAMD);
		    objdes(tmp_str, i_ptr, TRUE);
		  }
		  (void) fprintf(file1, "%d %s\n", i_ptr->level, tmp_str);
		}
	      pusht((int8u)j);
	      (void) fclose(file1);
	      prt("Completed.", 0, 0);
	    }
	  else
	    prt("File could not be opened.", 0, 0);
	}
    }
  else
    prt ("Parameters no good.", 0, 0);
}


/* Print the character to a file or device		-RAK-	 */
int file_character(filename1)
char *filename1;
{
  register int i;
  int j, xbth, xbthb, xfos, xsrh, xstl, xdis, xsave, xdev;
  vtype xinfra;
  int fd;
  register FILE *file1;
  bigvtype prt2;
  register struct misc *p_ptr;
  register inven_type *i_ptr;
  vtype out_val, prt1;
  char *p, *colon, *blank;

  fd = open (filename1, O_WRONLY|O_CREAT|O_EXCL, 0644);
  if (fd < 0 && errno == EEXIST)
    {
      (void) sprintf(out_val, "Replace existing file %s?", filename1);
      if (get_check(out_val))
	fd = open(filename1, O_WRONLY, 0644);
    }
  if (fd >= 0)
    {
      /* on some non-unix machines, fdopen() is not reliable, hence must call
	 close() and then fopen() */
      (void) close(fd);
      file1 = fopen(filename1, "w");
    }
  else
    file1 = NULL;

  if (file1 != NULL)
    {
      prt("Writing character sheet...", 0, 0);
      put_qio();
      colon = ":";
      blank = " ";
      (void) fprintf(file1, "%c\n\n", CTRL('L'));
      (void) fprintf(file1, " Name%9s %-23s", colon, py.misc.name);
      (void) fprintf(file1, " Age%11s %6d", colon, (int)py.misc.age);
      cnv_stat(py.stats.use_stat[A_STR], prt1);
      (void) fprintf(file1, "   STR : %s\n", prt1);
      (void) fprintf(file1, " Race%9s %-23s", colon,race[py.misc.prace].trace);
      (void) fprintf(file1, " Height%8s %6d", colon, (int)py.misc.ht);
      cnv_stat(py.stats.use_stat[A_INT], prt1);
      (void) fprintf(file1, "   INT : %s\n", prt1);
      (void) fprintf(file1, " Sex%10s %-23s", colon,
		     (py.misc.male ? "Male" : "Female"));
      (void) fprintf(file1, " Weight%8s %6d", colon, (int)py.misc.wt);
      cnv_stat(py.stats.use_stat[A_WIS], prt1);
      (void) fprintf(file1, "   WIS : %s\n", prt1);
      (void) fprintf(file1, " Class%8s %-23s", colon,
		     class[py.misc.pclass].title);
      (void) fprintf(file1, " Social Class : %6d", py.misc.sc);
      cnv_stat(py.stats.use_stat[A_DEX], prt1);
      (void) fprintf(file1, "   DEX : %s\n", prt1);
      (void) fprintf(file1, " Title%8s %-23s", colon, title_string());
      (void) fprintf(file1, "%22s", blank);
      cnv_stat(py.stats.use_stat[A_CON], prt1);
      (void) fprintf(file1, "   CON : %s\n", prt1);
      (void) fprintf(file1, "%34s", blank);
      (void) fprintf(file1, "%26s", blank);
      cnv_stat(py.stats.use_stat[A_CHR], prt1);
      (void) fprintf(file1, "   CHR : %s\n\n", prt1);

      (void) fprintf(file1, " + To Hit    : %6d", py.misc.dis_th);
      (void) fprintf(file1, "%8sLevel      : %6d", blank, (int)py.misc.lev);
      (void) fprintf(file1, "    Max Hit Points : %6d\n", py.misc.mhp);
      (void) fprintf(file1, " + To Damage : %6d", py.misc.dis_td);
      (void) fprintf(file1, "%8sExperience : %6ld", blank, py.misc.exp);
      (void) fprintf(file1, "    Cur Hit Points : %6d\n", py.misc.chp);
      (void) fprintf(file1, " + To AC     : %6d", py.misc.dis_tac);
      (void) fprintf(file1, "%8sGold%8s %6ld", blank, colon, py.misc.au);
      (void) fprintf(file1, "    Max Mana%8s %6d\n", colon, py.misc.mana);
      (void) fprintf(file1, "   Total AC  : %6d", py.misc.dis_ac);
      (void) fprintf(file1, "%27s", blank);
      (void) fprintf(file1, "    Cur Mana%8s %6d\n\n", colon, py.misc.cmana);

      p_ptr = &py.misc;
      xbth = p_ptr->bth + p_ptr->ptohit * BTH_PLUS_ADJ
	+ (class_level_adj[p_ptr->pclass][CLA_BTH] * p_ptr->lev);
      xbthb = p_ptr->bthb + p_ptr->ptohit * BTH_PLUS_ADJ
	+ (class_level_adj[p_ptr->pclass][CLA_BTHB] * p_ptr->lev);
      /* this results in a range from 0 to 29 */
      xfos = 40 - p_ptr->fos;
      if (xfos < 0)
	xfos = 0;
      xsrh = p_ptr->srh;
      /* this results in a range from 0 to 9 */
      xstl = p_ptr->stl + 1;
      xdis = p_ptr->disarm + 2 * todis_adj() + stat_adj(A_INT)
	+ (class_level_adj[p_ptr->pclass][CLA_DISARM] * p_ptr->lev / 3);
      xsave = p_ptr->save + stat_adj(A_WIS)
	+ (class_level_adj[p_ptr->pclass][CLA_SAVE] * p_ptr->lev / 3);
      xdev = p_ptr->save + stat_adj(A_INT)
	+ (class_level_adj[p_ptr->pclass][CLA_DEVICE] * p_ptr->lev / 3);

      (void) sprintf(xinfra, "%d feet", py.flags.see_infra * 10);

      (void) fprintf(file1, "(Miscellaneous Abilities)\n\n");
      (void) fprintf(file1, " Fighting    : %-10s", likert(xbth, 12));
      (void) fprintf(file1, "   Stealth     : %-10s", likert(xstl, 1));
      (void) fprintf(file1, "   Perception  : %s\n", likert(xfos, 3));
      (void) fprintf(file1, " Bows/Throw  : %-10s", likert(xbthb, 12));
      (void) fprintf(file1, "   Disarming   : %-10s", likert(xdis, 8));
      (void) fprintf(file1, "   Searching   : %s\n", likert(xsrh, 6));
      (void) fprintf(file1, " Saving Throw: %-10s", likert(xsave, 6));
      (void) fprintf(file1, "   Magic Device: %-10s", likert(xdev, 6));
      (void) fprintf(file1, "   Infra-Vision: %s\n\n", xinfra);
      /* Write out the character's history     */
      (void) fprintf(file1, "Character Background\n");
      for (i = 0; i < 4; i++)
	(void) fprintf(file1, " %s\n", py.misc.history[i]);
      /* Write out the equipment list.	     */
      j = 0;
      (void) fprintf(file1, "\n  [Character's Equipment List]\n\n");
      if (equip_ctr == 0)
	(void) fprintf(file1, "  Character has no equipment in use.\n");
      else
	for (i = INVEN_WIELD; i < INVEN_ARRAY_SIZE; i++)
	  {
	    i_ptr = &inventory[i];
	    if (i_ptr->tval != TV_NOTHING)
	      {
		switch (i)
		  {
		  case INVEN_WIELD:	p = "You are wielding";	break;
		  case INVEN_HEAD:	p = "Worn on head";	break;
		  case INVEN_NECK:	p = "Worn around neck";	break;
		  case INVEN_BODY:	p = "Worn on body";	break;
		  case INVEN_ARM:	p = "Worn on shield arm";break;
		  case INVEN_HANDS:	p = "Worn on hands";	break;
		  case INVEN_RIGHT:	p = "Right ring finger";break;
		  case INVEN_LEFT:	p = "Left  ring finger";break;
		  case INVEN_FEET:	p = "Worn on feet";	break;
		  case INVEN_OUTER:	p = "Worn about body";	break;
		  case INVEN_LIGHT:	p = "Light source is";	break;
		  case INVEN_AUX:	p = "Secondary weapon";	break;
		  default: p = "*Unknown value*";     break;
		  }
		objdes(prt2, &inventory[i], TRUE);
		(void) fprintf(file1, "  %c) %-19s: %s\n", j+'a', p, prt2);
		j++;
	      }
	  }

      /* Write out the character's inventory.	     */
      (void) fprintf(file1, "%c\n\n", CTRL('L'));
      (void) fprintf(file1, "  [General Inventory List]\n\n");
      if (inven_ctr == 0)
	(void) fprintf(file1, "  Character has no objects in inventory.\n");
      else
	{
	  for (i = 0; i < inven_ctr; i++)
	    {
	      objdes(prt2, &inventory[i], TRUE);
	      (void) fprintf(file1, "%c) %s\n", i+'a', prt2);
	    }
	}
      (void) fprintf(file1, "%c", CTRL('L'));
      (void) fclose(file1);
      prt("Completed.", 0, 0);
      return TRUE;
    }
  else
    {
      if (fd >= 0)
	(void) close (fd);
      (void) sprintf (out_val, "Can't open file %s:", filename1);
      msg_print(out_val);
      return FALSE;
    }
}
