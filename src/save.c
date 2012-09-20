/* save.c: save and restore games and monster memory info

   Copyright (c) 1989 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

#include <stdio.h>

#include "constant.h"
#include "config.h"
#include "types.h"
#include "externs.h"

#ifndef USG
/* stuff only needed for Berkeley UNIX */
#include <sys/types.h>
#include <sys/file.h>
#include <sys/param.h>
#endif

#ifdef USG
#ifndef ATARIST_MWC
#include <string.h>
#include <fcntl.h>
#else
#include "string.h"
#endif
#else
#include <strings.h>
#endif

static int sv_write();
static void wr_byte();
static void wr_short();
static void wr_long();
static void wr_bytes();
static void wr_string();
static void wr_shorts();
static void wr_item();
static void wr_monster();
static void rd_byte();
static void rd_short();
static void rd_long();
static void rd_bytes();
static void rd_string();
static void rd_shorts();
static void rd_item();
static void rd_monster();

#if !defined(ATARIST_MWC)
#ifdef MAC
#include <time.h>
#else
long time();
#endif
#else
char *malloc();
#endif

#ifndef SET_UID
#include <sys/stat.h>
#endif

/* these are used for the save file, to avoid having to pass them to every
   procedure */
static FILE *fileptr;
static int8u xor_byte;
static int from_savefile;	/* can overwrite old savefile when save */
static int32u start_time;	/* time that play started */

/* This save package was brought to by			-JWT-
   and							-RAK-
   and has been completely rewritten for UNIX by	-JEW-  */
/* and has been completely rewritten again by	 -CJS-	*/
/* and completely rewritten again! for portability by -JEW- */


static char *basename(a)
  char *a;
{
  char *b;
  char *strrchr();

  if ((b=strrchr(a, (int)'/'))==(char *)0)
    return a;
  return b;
}

static void wr_unique(item)
register struct unique_mon *item;
{
  wr_long((int32u)item->exist);
  wr_long((int32u)item->dead);
}

static void rd_unique(item)
register struct unique_mon *item;
{
  rd_long((int32u *)&item->exist);
  rd_long((int32u *)&item->dead);
}


static int sv_write()
{
  int32u l;
  register int i, j;
  int count;
  int8u char_tmp, prev_char;
  register cave_type *c_ptr;
  register recall_type *r_ptr;
  struct stats *s_ptr;
#ifdef MSDOS
  inven_type *t_ptr;
#endif
  register struct flags *f_ptr;
  store_type *st_ptr;
  struct misc *m_ptr;

  /* clear the death flag when creating a HANGUP save file, so that player
     can see tombstone when restart */

  if (eof_flag)
    death = FALSE;

  l = 0;
  if (find_cut)
    l |= 1;
  if (find_examine)
    l |= 2;
  if (find_prself)
    l |= 4;
  if (find_bound)
    l |= 8;
  if (prompt_carry_flag)
    l |= 16;
  if (rogue_like_commands)
    l |= 32;
  if (show_weight_flag)
    l |= 64;
  if (highlight_seams)
    l |= 128;
  if (find_ignore_doors)
    l |= 256;
/*   if (no_haggle_flag)
    l |= 0x200L; */
  if (!carry_query_flag)
    l |= 0x400L;
  if (unfelt)
    l |= 0x0001000L;
  l |= ((delay_spd & 0xf) << 13);
  l |= ((hitpoint_warn & 0xf) << 17);
  if (death)
    l |= 0x80000000L;	/* Sign bit */
  wr_long(GROND);
  wr_long(RINGIL);
  wr_long(AEGLOS);
  wr_long(ARUNRUTH);
  wr_long(MORMEGIL);
  wr_long(ANGRIST);
  wr_long(GURTHANG);
  wr_long(CALRIS);
  wr_long(ANDURIL);
  wr_long(STING);
  wr_long(ORCRIST);
  wr_long(GLAMDRING);
  wr_long(DURIN);
  wr_long(AULE);
  wr_long(THUNDERFIST);
  wr_long(BLOODSPIKE);
  wr_long(DOOMCALLER);
  wr_long(NARTHANC);
  wr_long(NIMTHANC);
  wr_long(DETHANC);
  wr_long(GILETTAR);
  wr_long(RILIA);
  wr_long(BELANGIL);
  wr_long(BALLI);
  wr_long(LOTHARANG);
  wr_long(FIRESTAR);
  wr_long(ERIRIL);
  wr_long(CUBRAGOL);
  wr_long(BARD);
  wr_long(COLLUIN);
  wr_long(HOLCOLLETH);
  wr_long(TOTILA);
  wr_long(PAIN);
  wr_long(ELVAGIL);
  wr_long(AGLARANG);
  wr_long(EORLINGAS);
  wr_long(BARUKKHELED);
  wr_long(WRATH);
  wr_long(HARADEKKET);
  wr_long(MUNDWINE);
  wr_long(GONDRICAM);
  wr_long(ZARCUTHRA);
  wr_long(CARETH);
  wr_long(FORASGIL);
  wr_long(CRISDURIAN);
  wr_long(COLANNON);
  wr_long(HITHLOMIR);
  wr_long(THALKETTOTH);
  wr_long(ARVEDUI);
  wr_long(THRANDUIL);
  wr_long(THENGEL);
  wr_long(HAMMERHAND);
  wr_long(CELEFARN);
  wr_long(THROR);
  wr_long(MAEDHROS);
  wr_long(OLORIN);
  wr_long(ANGUIREL);
  wr_long(OROME);
  wr_long(EONWE);
  wr_long(THEODEN);
  wr_long(ULMO);
  wr_long(OSONDIR);
  wr_long(TURMIL);
  wr_long(CASPANION);
  wr_long(TIL);
  wr_long(DEATHWREAKER);
  wr_long(AVAVIR);
  wr_long(TARATOL);

  wr_long(DOR_LOMIN);
  wr_long(NENYA);
  wr_long(NARYA);
  wr_long(VILYA);
  wr_long(BELEGENNON);
  wr_long(FEANOR);
  wr_long(ISILDUR);
  wr_long(SOULKEEPER);
  wr_long(FINGOLFIN);
  wr_long(ANARION);
  wr_long(POWER);
  wr_long(PHIAL);
  wr_long(BELEG);
  wr_long(DAL);
  wr_long(PAURHACH);
  wr_long(PAURNIMMEN);
  wr_long(PAURAEGEN);
  wr_long(PAURNEN);
  wr_long(CAMMITHRIM);
  wr_long(CAMBELEG);
  wr_long(INGWE);
  wr_long(CARLAMMAS);
  wr_long(HOLHENNETH);
  wr_long(AEGLIN);
  wr_long(CAMLOST);
  wr_long(NIMLOTH);
  wr_long(NAR);
  wr_long(BERUTHIEL);
  wr_long(GORLIM);
  wr_long(ELENDIL);
  wr_long(THORIN);
  wr_long(CELEBORN);
  wr_long(THRAIN);
  wr_long(GONDOR);
  wr_long(THINGOL);
  wr_long(THORONGIL);
  wr_long(LUTHIEN);
  wr_long(TUOR);
  wr_long(ROHAN);
  wr_long(TULKAS);
  wr_long(NECKLACE);
  wr_long(BARAHIR);
  wr_long(RAZORBACK);
  wr_long(BLADETURNER);

  for (i=0; i<MAX_QUESTS; i++)
    wr_long(quests[i]);

  for (i=0; i<MAX_CREATURES; i++)
    wr_unique(&u_list[i]);

  for (i = 0; i < MAX_CREATURES; i++)
    {
      r_ptr = &c_recall[i];
      if (r_ptr->r_cmove || r_ptr->r_cdefense || r_ptr->r_kills ||
          r_ptr->r_spells2 || r_ptr->r_spells3 || r_ptr->r_spells ||
	  r_ptr->r_deaths || r_ptr->r_attacks[0] || r_ptr->r_attacks[1] ||
	  r_ptr->r_attacks[2] || r_ptr->r_attacks[3])
	{
	  wr_short((int16u)i);
	  wr_long(r_ptr->r_cmove);
	  wr_long(r_ptr->r_spells);
	  wr_long(r_ptr->r_spells2);
	  wr_long(r_ptr->r_spells3);
	  wr_short(r_ptr->r_kills);
	  wr_short(r_ptr->r_deaths);
	  wr_long(r_ptr->r_cdefense);
	  wr_byte(r_ptr->r_wake);
	  wr_byte(r_ptr->r_ignore);
	  wr_bytes(r_ptr->r_attacks, MAX_MON_NATTACK);
	}
    }
  wr_short((int16u)0xFFFF); /* sentinel to indicate no more monster info */

  wr_short((int16u)log_index);
  wr_long(l);

  m_ptr = &py.misc;
  wr_string(m_ptr->name);
  wr_byte(m_ptr->male);
  wr_long((int32u)m_ptr->au);
  wr_long((int32u)m_ptr->max_exp);
  wr_long((int32u)m_ptr->exp);
  wr_short(m_ptr->exp_frac);
  wr_short(m_ptr->age);
  wr_short(m_ptr->ht);
  wr_short(m_ptr->wt);
  wr_short(m_ptr->lev);
  wr_short(m_ptr->max_dlv);
  wr_short((int16u)m_ptr->srh);
  wr_short((int16u)m_ptr->fos);
  wr_short((int16u)m_ptr->bth);
  wr_short((int16u)m_ptr->bthb);
  wr_short((int16u)m_ptr->mana);
  wr_short((int16u)m_ptr->mhp);
  wr_short((int16u)m_ptr->ptohit);
  wr_short((int16u)m_ptr->ptodam);
  wr_short((int16u)m_ptr->pac);
  wr_short((int16u)m_ptr->ptoac);
  wr_short((int16u)m_ptr->dis_th);
  wr_short((int16u)m_ptr->dis_td);
  wr_short((int16u)m_ptr->dis_ac);
  wr_short((int16u)m_ptr->dis_tac);
  wr_short((int16u)m_ptr->disarm);
  wr_short((int16u)m_ptr->save);
  wr_short((int16u)m_ptr->sc);
  wr_short((int16u)m_ptr->stl);
  wr_byte(m_ptr->pclass);
  wr_byte(m_ptr->prace);
  wr_byte(m_ptr->hitdie);
  wr_byte(m_ptr->expfact);
  wr_short((int16u)m_ptr->cmana);
  wr_short(m_ptr->cmana_frac);
  wr_short((int16u)m_ptr->chp);
  wr_short(m_ptr->chp_frac);
  for (i = 0; i < 4; i++)
    wr_string (m_ptr->history[i]);

  s_ptr = &py.stats;
  wr_shorts(s_ptr->max_stat, 6);
  wr_shorts(s_ptr->cur_stat, 6);
  wr_shorts((int16u *)s_ptr->mod_stat, 6);
  wr_shorts(s_ptr->use_stat, 6);

  f_ptr = &py.flags;
  wr_long(f_ptr->status);
  wr_short((int16u)f_ptr->rest);
  wr_short((int16u)f_ptr->blind);
  wr_short((int16u)f_ptr->paralysis);
  wr_short((int16u)f_ptr->confused);
  wr_short((int16u)f_ptr->food);
  wr_short((int16u)f_ptr->food_digested);
  wr_short((int16u)f_ptr->protection);
  wr_short((int16u)f_ptr->speed);
  wr_short((int16u)f_ptr->fast);
  wr_short((int16u)f_ptr->slow);
  wr_short((int16u)f_ptr->afraid);
  wr_short((int16u)f_ptr->cut);
  wr_short((int16u)f_ptr->stun);
  wr_short((int16u)f_ptr->poisoned);
  wr_short((int16u)f_ptr->image);
  wr_short((int16u)f_ptr->protevil);
  wr_short((int16u)f_ptr->invuln);
  wr_short((int16u)f_ptr->hero);
  wr_short((int16u)f_ptr->shero);
  wr_short((int16u)f_ptr->shield);
  wr_short((int16u)f_ptr->blessed);
  wr_short((int16u)f_ptr->resist_heat);
  wr_short((int16u)f_ptr->resist_cold);
  wr_short((int16u)f_ptr->resist_acid);
  wr_short((int16u)f_ptr->resist_light);
  wr_short((int16u)f_ptr->resist_poison);
  wr_short((int16u)f_ptr->detect_inv);
  wr_short((int16u)f_ptr->word_recall);
  wr_short((int16u)f_ptr->see_infra);
  wr_short((int16u)f_ptr->tim_infra);
  wr_byte(f_ptr->see_inv);
  wr_byte(f_ptr->teleport);
  wr_byte(f_ptr->free_act);
  wr_byte(f_ptr->slow_digest);
  wr_byte(f_ptr->aggravate);
  wr_byte(f_ptr->fire_resist);
  wr_byte(f_ptr->cold_resist);
  wr_byte(f_ptr->acid_resist);
  wr_byte(f_ptr->regenerate);
  wr_byte(f_ptr->lght_resist);
  wr_byte(f_ptr->ffall);
  wr_byte(f_ptr->sustain_str);
  wr_byte(f_ptr->sustain_int);
  wr_byte(f_ptr->sustain_wis);
  wr_byte(f_ptr->sustain_con);
  wr_byte(f_ptr->sustain_dex);
  wr_byte(f_ptr->sustain_chr);
  wr_byte(f_ptr->confuse_monster);
  wr_byte(f_ptr->new_spells);
  wr_byte(f_ptr->poison_resist);
  wr_byte(f_ptr->hold_life);
  wr_byte(f_ptr->telepathy);
  wr_byte(f_ptr->fire_im);
  wr_byte(f_ptr->acid_im);
  wr_byte(f_ptr->poison_im);
  wr_byte(f_ptr->cold_im);
  wr_byte(f_ptr->light_im);
  wr_byte(f_ptr->light);
  wr_byte(f_ptr->confusion_resist);
  wr_byte(f_ptr->sound_resist);
  wr_byte(f_ptr->light_resist);
  wr_byte(f_ptr->dark_resist);
  wr_byte(f_ptr->chaos_resist);
  wr_byte(f_ptr->disenchant_resist);
  wr_byte(f_ptr->shards_resist);
  wr_byte(f_ptr->nexus_resist);
  wr_byte(f_ptr->blindness_resist);
  wr_byte(f_ptr->nether_resist);

  wr_short((int16u)missile_ctr);
  wr_long((int32u)turn);
  wr_short((int16u)inven_ctr);
  for (i = 0; i < inven_ctr; i++)
    wr_item(&inventory[i]);
  for (i = INVEN_WIELD; i < INVEN_ARRAY_SIZE; i++)
    wr_item(&inventory[i]);
  wr_short((int16u)inven_weight);
  wr_short((int16u)equip_ctr);
  wr_long(spell_learned);
  wr_long(spell_worked);
  wr_long(spell_forgotten);
  wr_long(spell_learned2);
  wr_long(spell_worked2);
  wr_long(spell_forgotten2);
  wr_bytes(spell_order, 64);
  wr_bytes(object_ident, OBJECT_IDENT_SIZE);
  wr_long(randes_seed);
  wr_long(town_seed);
  wr_short((int16u)last_msg);
  for (i = 0; i < MAX_SAVE_MSG; i++)
    wr_string(old_msg[i]);

  /* this indicates 'cheating' if it is a one */
  wr_short((int16u)panic_save);
  wr_short((int16u)total_winner);
  wr_short((int16u)noscore);
  wr_shorts(player_hp, MAX_PLAYER_LEVEL);


  for (i = 0; i < MAX_STORES; i++)
    {
      st_ptr = &store[i];
      wr_long((int32u)st_ptr->store_open);
      wr_short((int16u)st_ptr->insult_cur);
      wr_byte(st_ptr->owner);
      wr_byte(st_ptr->store_ctr);
      wr_short(st_ptr->good_buy);
      wr_short(st_ptr->bad_buy);

      for (j = 0; j < st_ptr->store_ctr; j++)
	{
	    wr_long((int32u)st_ptr->store_inven[j].scost);
	    wr_item(&st_ptr->store_inven[j].sitem);
	}
  }

  /* save the current time in the savefile */
  l = time((long *)0);
/*  if (l < start_time)
    {
        someone is messing with the clock!, assume that we have been
	 playing for 1 day
      l = start_time + 86400L;
    }
*/
  wr_long(l);

  /* starting with 5.2, put died_from string in savefile */
  wr_string(died_from);

  /* only level specific info follows, this allows characters to be
     resurrected, the dungeon level info is not needed for a resurrection */
  if (death)
    {
      if (ferror(fileptr) || fflush(fileptr) == EOF)
	return FALSE;
      return TRUE;
    }
  wr_short((int16u)dun_level);
  wr_short((int16u)char_row);
  wr_short((int16u)char_col);
  wr_short((int16u)mon_tot_mult);
  wr_short((int16u)cur_height);
  wr_short((int16u)cur_width);
  wr_short((int16u)max_panel_rows);
  wr_short((int16u)max_panel_cols);

  for (i = 0; i < MAX_HEIGHT; i++)
    for (j = 0; j < MAX_WIDTH; j++)
      {
	c_ptr = &cave[i][j];
	if (c_ptr->cptr != 0)
	  {
	    wr_byte((int8u)i);
	    wr_byte((int8u)j);
	    wr_byte(c_ptr->cptr);
	  }
      }
  wr_byte((int8u)0xFF); /* marks end of cptr info */
  for (i = 0; i < MAX_HEIGHT; i++)
    for (j = 0; j < MAX_WIDTH; j++)
      {
	c_ptr = &cave[i][j];
	if (c_ptr->tptr != 0)
	  {
	    wr_byte((int8u)i);
	    wr_byte((int8u)j);
	    wr_short((int16u)c_ptr->tptr);
	  }
      }
  wr_byte(0xFF); /* marks end of tptr info */
  /* must set counter to zero, note that code may write out two bytes
     unnecessarily */
  count = 0;
  prev_char = 0;
  for (i = 0; i < MAX_HEIGHT; i++)
    for (j = 0; j < MAX_WIDTH; j++)
      {
	c_ptr = &cave[i][j];
	char_tmp = c_ptr->fval | (c_ptr->lr << 4) | (c_ptr->fm << 5) |
	  (c_ptr->pl << 6) | (c_ptr->tl << 7);
	if (char_tmp != prev_char || count == MAX_UCHAR)
	  {
	    wr_byte((int8u)count);
	    wr_byte(prev_char);
	    prev_char = char_tmp;
	    count = 1;
	  }
	else
	  count++;
      }
  /* save last entry */
  wr_byte((int8u)count);
  wr_byte(prev_char);

#ifdef MSDOS
  /* must change graphics symbols for walls and floors back to default chars,
     this is necessary so that if the user changes the graphics line, the
     program will be able change all existing walls/floors to the new symbol */
  t_ptr = &t_list[tcptr - 1];
  for (i = tcptr - 1; i >= MIN_TRIX; i--)
    {
      if (t_ptr->tchar == wallsym)
	t_ptr->tchar = '#';
      t_ptr--;
    }
#endif
  wr_short((int16u)tcptr);
  for (i = MIN_TRIX; i < tcptr; i++)
    wr_item(&t_list[i]);
  wr_short((int16u)mfptr);
  for (i = MIN_MONIX; i < mfptr; i++)
    wr_monster(&m_list[i]);

  /* Save ghost names & stats etc... */
  wr_bytes(c_list[MAX_CREATURES - 1].name, 100);
  wr_long((int32u)c_list[MAX_CREATURES - 1].cmove);
  wr_long((int32u)c_list[MAX_CREATURES - 1].spells);
  wr_long((int32u)c_list[MAX_CREATURES - 1].cdefense);
  {  int16u temp;  /* fix player ghost's exp bug.  The mexp field is
       			  really an int32u, but the savefile was writing/
      			  reading an int16u.  Since I don't want to change
      			  the savefile format, this insures that the low
      			  bits of mexp are written (No ghost should be
      			  worth more than 64K (Melkor is only worth 60k!),
			  but we check anyway).  Using temp insures that
			  the low bits are all written, and works perfectly
			  with a similar fix when loading a character. -CFT */
    if (c_list[MAX_CREATURES-1].mexp > (int32u)0xff00)
      temp = (int16u)0xff00;
    else
      temp = (int16u)c_list[MAX_CREATURES-1].mexp;
    wr_short((int16u)temp);
  }
  wr_byte((int8u)c_list[MAX_CREATURES - 1].sleep);
  wr_byte((int8u)c_list[MAX_CREATURES - 1].aaf);
  wr_byte((int8u)c_list[MAX_CREATURES - 1].ac);
  wr_byte((int8u)c_list[MAX_CREATURES - 1].speed);
  wr_byte((int8u)c_list[MAX_CREATURES - 1].cchar);
  wr_bytes(c_list[MAX_CREATURES - 1].hd, 2);
  wr_bytes(c_list[MAX_CREATURES - 1].damage, sizeof(attid)*4);
  wr_short((int16u)c_list[MAX_CREATURES - 1].level);

  if (ferror(fileptr) || (fflush(fileptr) == EOF))
    return FALSE;
  return TRUE;
}

int save_char()
{
  vtype temp;
  char *tmp2;

#ifdef SECURE
  bePlayer();
#endif
  if (death && NO_SAVE) return TRUE;

  if (_save_char(savefile)) {

    tmp2=basename(savefile);

    (void) sprintf(temp, "%s/p.%s", ANGBAND_SAV, (tmp2+1));

    unlink(temp);
  } else {
    return FALSE;
  }
#ifdef SECURE
  beGames();
#endif
  return TRUE;
}

int _save_char(fnam)
char *fnam;
{
  vtype temp;
  int ok, fd;
  int8u char_tmp;

  if (log_index < 0)
    return TRUE;	/* Nothing to save. */

  nosignals();
  put_qio();
  disturb (1, 0);		/* Turn off resting and searching. */
  change_speed(-pack_heavy);	/* Fix the speed */
  pack_heavy = 0;
  ok = FALSE;
#ifndef ATARIST_MWC
  fd = -1;
  fileptr = NULL;		/* Do not assume it has been init'ed */
#ifdef SET_UID
  fd = open(fnam, O_RDWR|O_CREAT|O_EXCL, 0600);
#else
  fd = open(fnam, O_RDWR|O_CREAT|O_EXCL, 0666);
#endif
  if (fd < 0 && access(fnam, 0) >= 0 &&
      (from_savefile ||
       (wizard && get_check("Can't make new savefile. Overwrite old?"))))
    {
#ifdef SET_UID
      (void) chmod(fnam, 0600);
      fd = open(fnam, O_RDWR|O_TRUNC, 0600);
#else
      (void) chmod(fnam, 0666);
      fd = open(fnam, O_RDWR|O_TRUNC, 0666);
#endif
  }
  if (fd >= 0)
    {
      (void) close(fd);
#endif /* !ATARIST_MWC */
      /* GCC for atari st defines atarist */
#if defined(atarist) || defined(ATARIST_MWC) || defined(MSDOS)
      fileptr = fopen(savefile, "wb");
#else
      fileptr = fopen(savefile, "w");
#endif
#ifndef ATARIST_MWC
    }
#endif
  if (fileptr != NULL)
    {
#ifdef MSDOS
      (void) setmode(fileno(fileptr), O_BINARY);
#endif
      xor_byte = 0;
      wr_byte((int8u)CUR_VERSION_MAJ);
      xor_byte = 0;
      wr_byte((int8u)CUR_VERSION_MIN);
      xor_byte = 0;
      wr_byte((int8u)PATCH_LEVEL);
      xor_byte = 0;
      char_tmp = randint(256) - 1;
      wr_byte(char_tmp);
      /* Note that xor_byte is now equal to char_tmp */

      ok = sv_write();
      if (fclose(fileptr) == EOF)
	ok = FALSE;
    }

  if (!ok)
    {
      if (fd >= 0)
	(void) unlink(fnam);
      signals();
      if (fd >= 0)
	(void) sprintf(temp, "Error writing to savefile");
      else
/* here? */
	(void) sprintf(temp, "Can't create new savefile");
      msg_print(temp);
      return FALSE;
    }
  else
    character_saved = 1;

  turn = -1;
  log_index = -1;

  signals();

  return TRUE;
}


/* Certain checks are ommitted for the wizard. -CJS- */

int get_char(generate)
int *generate;
{
  register int i, j;
  int fd, c, ok, total_count;
  int32u l, age, time_saved;
  vtype temp;
  int16u int16u_tmp;
  register cave_type *c_ptr;
  register recall_type *r_ptr;
  struct misc *m_ptr;
  struct stats *s_ptr;
  register struct flags *f_ptr;
  store_type *st_ptr;
  int8u char_tmp, ychar, xchar, count;
  int8u version_maj, version_min, patch_level;

  free_turn_flag = TRUE;/* So a feeling isn't generated upon reloading -DGK*/
  nosignals();
  *generate = TRUE;
  fd = -1;

  if (access(savefile, 0) < 0)
    {
      signals();
      msg_print("Savefile does not exist.");
      return FALSE;
    }

  clear_screen();

  (void) sprintf(temp, "Restoring Character.");
  put_buffer(temp, 23, 0);
  sleep(1);

  if (turn >= 0)
    msg_print("IMPOSSIBLE! Attempt to restore while still alive!");

  /* Allow restoring a file belonging to someone else - if we can delete it. */
  /* Hence first try to read without doing a chmod. */

  else if ((fd = open(savefile, O_RDONLY, 0)) < 0)
    msg_print("Can't open file for reading.");
  else
    {
#ifndef SET_UID
      struct stat statbuf;
#endif
      turn = -1;
      log_index = -1;
      ok = TRUE;

#ifndef SET_UID
      (void) fstat(fd, &statbuf);
#endif
      (void) close(fd);
      /* GCC for atari st defines atarist */
      fileptr = fopen(savefile, "r");
      if (fileptr == NULL)
	goto error;

      prt("Restoring Memory...", 0, 0);
      put_qio();

      xor_byte = 0;
      rd_byte(&version_maj);
      xor_byte = 0;
      rd_byte(&version_min);
      xor_byte = 0;
      rd_byte(&patch_level);
      xor_byte = 0;
      rd_byte(&xor_byte);

      /* COMPAT support savefiles from 5.0.14 to 5.0.17 */
      /* support savefiles from 5.1.0 to present */
      if ((version_maj != CUR_VERSION_MAJ)
	  || (version_min > CUR_VERSION_MIN)
	  || (version_min == CUR_VERSION_MIN && patch_level > PATCH_LEVEL)
	  || (version_min == 0 && patch_level < 14))
	{
	  prt("Sorry. This savefile is from a different version of angband.",
	      2, 0);
	  goto error;
	}

      put_qio();
      rd_long(&GROND);
      rd_long(&RINGIL);
      rd_long(&AEGLOS);
      rd_long(&ARUNRUTH);
      rd_long(&MORMEGIL);
      rd_long(&ANGRIST);
      rd_long(&GURTHANG);
      rd_long(&CALRIS);
      rd_long(&ANDURIL);
      rd_long(&STING);
      rd_long(&ORCRIST);
      rd_long(&GLAMDRING);
      rd_long(&DURIN);
      rd_long(&AULE);
      rd_long(&THUNDERFIST);
      rd_long(&BLOODSPIKE);
      rd_long(&DOOMCALLER);
      rd_long(&NARTHANC);
      rd_long(&NIMTHANC);
      rd_long(&DETHANC);
      rd_long(&GILETTAR);
      rd_long(&RILIA);
      rd_long(&BELANGIL);
      rd_long(&BALLI);
      rd_long(&LOTHARANG);
      rd_long(&FIRESTAR);
      rd_long(&ERIRIL);
      rd_long(&CUBRAGOL);
      rd_long(&BARD);
      rd_long(&COLLUIN);
      rd_long(&HOLCOLLETH);
      rd_long(&TOTILA);
      rd_long(&PAIN);
      rd_long(&ELVAGIL);
      rd_long(&AGLARANG);
      rd_long(&EORLINGAS);
      rd_long(&BARUKKHELED);
      rd_long(&WRATH);
      rd_long(&HARADEKKET);
      rd_long(&MUNDWINE);
      rd_long(&GONDRICAM);
      rd_long(&ZARCUTHRA);
      rd_long(&CARETH);
      rd_long(&FORASGIL);
      rd_long(&CRISDURIAN);
      rd_long(&COLANNON);
      rd_long(&HITHLOMIR);
      rd_long(&THALKETTOTH);
      rd_long(&ARVEDUI);
      rd_long(&THRANDUIL);
      rd_long(&THENGEL);
      rd_long(&HAMMERHAND);
      rd_long(&CELEFARN);
      rd_long(&THROR);
      rd_long(&MAEDHROS);
      rd_long(&OLORIN);
      rd_long(&ANGUIREL);
      rd_long(&OROME);
      rd_long(&EONWE);
      rd_long(&THEODEN);
      rd_long(&ULMO);
      rd_long(&OSONDIR);
      rd_long(&TURMIL);
      rd_long(&CASPANION);
      rd_long(&TIL);
      rd_long(&DEATHWREAKER);
      rd_long(&AVAVIR);
      rd_long(&TARATOL);
      if (to_be_wizard) prt("Loaded Weapon Artifacts", 2, 0);
      put_qio();


      rd_long(&DOR_LOMIN);
      rd_long(&NENYA);
      rd_long(&NARYA);
      rd_long(&VILYA);
      rd_long(&BELEGENNON);
      rd_long(&FEANOR);
      rd_long(&ISILDUR);
      rd_long(&SOULKEEPER);
      rd_long(&FINGOLFIN);
      rd_long(&ANARION);
      rd_long(&POWER);
      rd_long(&PHIAL);
      rd_long(&BELEG);
      rd_long(&DAL);
      rd_long(&PAURHACH);
      rd_long(&PAURNIMMEN);
      rd_long(&PAURAEGEN);
      rd_long(&PAURNEN);
      rd_long(&CAMMITHRIM);
      rd_long(&CAMBELEG);
      rd_long(&INGWE);
      rd_long(&CARLAMMAS);
      rd_long(&HOLHENNETH);
      rd_long(&AEGLIN);
      rd_long(&CAMLOST);
      rd_long(&NIMLOTH);
      rd_long(&NAR);
      rd_long(&BERUTHIEL);
      rd_long(&GORLIM);
      rd_long(&ELENDIL);
      rd_long(&THORIN);
      rd_long(&CELEBORN);
      rd_long(&THRAIN);
      rd_long(&GONDOR);
      rd_long(&THINGOL);
      rd_long(&THORONGIL);
      rd_long(&LUTHIEN);
      rd_long(&TUOR);
      rd_long(&ROHAN);
      rd_long(&TULKAS);
      rd_long(&NECKLACE);
      rd_long(&BARAHIR);
      rd_long(&RAZORBACK);
      rd_long(&BLADETURNER);
      if (to_be_wizard) prt("Loaded Armour Artifacts", 3, 0);
      put_qio();

      for (i=0; i<MAX_QUESTS; i++)
	rd_long(&quests[i]);
      if (to_be_wizard) prt("Loaded Quests", 4, 0);

      for (i=0; i<MAX_CREATURES; i++)
	rd_unique(&u_list[i]);
      if (to_be_wizard) prt("Loaded Unique Beasts", 5, 0);
      put_qio();

      rd_short(&int16u_tmp);
      while (int16u_tmp != 0xFFFF)
	{
	  if (int16u_tmp >= MAX_CREATURES)
	    goto error;
	  r_ptr = &c_recall[int16u_tmp];
	  rd_long(&r_ptr->r_cmove);
	  rd_long(&r_ptr->r_spells);
	  rd_long(&r_ptr->r_spells2);
	  rd_long(&r_ptr->r_spells3);
	  rd_short(&r_ptr->r_kills);
	  rd_short(&r_ptr->r_deaths);
	  rd_long(&r_ptr->r_cdefense);
	  rd_byte(&r_ptr->r_wake);
	  rd_byte(&r_ptr->r_ignore);
	  rd_bytes(r_ptr->r_attacks, MAX_MON_NATTACK);
	  rd_short(&int16u_tmp);
	}
      if (to_be_wizard) prt("Loaded Recall Memory", 6, 0);
      put_qio();
      rd_short((int16u *)&log_index);
      rd_long(&l);
      if (to_be_wizard) prt("Loaded Options Memory", 7, 0);
      put_qio();

      if (l & 1)
	find_cut = TRUE;
      else
	find_cut = FALSE;
      if (l & 2)
	find_examine = TRUE;
      else
	find_examine = FALSE;
      if (l & 4)
	find_prself = TRUE;
      else
	find_prself = FALSE;
      if (l & 8)
	find_bound = TRUE;
      else
	find_bound = FALSE;
      if (l & 16)
	prompt_carry_flag = TRUE;
      else
	prompt_carry_flag = FALSE;
      if (l & 32)
	rogue_like_commands = TRUE;
      else
	rogue_like_commands = FALSE;
      if (l & 64)
	show_weight_flag = TRUE;
      else
	show_weight_flag = FALSE;
      if (l & 128)
	highlight_seams = TRUE;
      else
	highlight_seams = FALSE;
      if (l & 256)
	find_ignore_doors = TRUE;
      else
	find_ignore_doors = FALSE;
/*    if (l & 0x200L)
        no_haggle_flag = TRUE;
      else
        no_haggle_flag = FALSE;
*/
      if (l & 0x400L)
        carry_query_flag = FALSE;
      else
        carry_query_flag = TRUE;
      if (l & 0x1000L)
        unfelt = TRUE;
      else
        unfelt = FALSE;
      delay_spd = ((l >> 13) & 0xf);
      hitpoint_warn = ((l >> 17) & 0xf);

      if (to_be_wizard && (l & 0x80000000L)
	  && get_check("Resurrect a dead character?"))
	l &= ~0x80000000L;
      if ((l & 0x80000000L) == 0)
	{
	  m_ptr = &py.misc;
	  rd_string(m_ptr->name);
	  rd_byte(&m_ptr->male);
	  rd_long((int32u *)&m_ptr->au);
	  rd_long((int32u *)&m_ptr->max_exp);
	  rd_long((int32u *)&m_ptr->exp);
	  rd_short(&m_ptr->exp_frac);
	  rd_short(&m_ptr->age);
	  rd_short(&m_ptr->ht);
	  rd_short(&m_ptr->wt);
	  rd_short(&m_ptr->lev);
	  rd_short(&m_ptr->max_dlv);
	  rd_short((int16u *)&m_ptr->srh);
	  rd_short((int16u *)&m_ptr->fos);
	  rd_short((int16u *)&m_ptr->bth);
	  rd_short((int16u *)&m_ptr->bthb);
	  rd_short((int16u *)&m_ptr->mana);
	  rd_short((int16u *)&m_ptr->mhp);
	  rd_short((int16u *)&m_ptr->ptohit);
	  rd_short((int16u *)&m_ptr->ptodam);
	  rd_short((int16u *)&m_ptr->pac);
	  rd_short((int16u *)&m_ptr->ptoac);
	  rd_short((int16u *)&m_ptr->dis_th);
	  rd_short((int16u *)&m_ptr->dis_td);
	  rd_short((int16u *)&m_ptr->dis_ac);
	  rd_short((int16u *)&m_ptr->dis_tac);
	  rd_short((int16u *)&m_ptr->disarm);
	  rd_short((int16u *)&m_ptr->save);
	  rd_short((int16u *)&m_ptr->sc);
	  rd_short((int16u *)&m_ptr->stl);
	  rd_byte(&m_ptr->pclass);
	  rd_byte(&m_ptr->prace);
	  rd_byte(&m_ptr->hitdie);
	  rd_byte(&m_ptr->expfact);
	  rd_short((int16u *)&m_ptr->cmana);
	  rd_short(&m_ptr->cmana_frac);
	  rd_short((int16u *)&m_ptr->chp);
	  rd_short(&m_ptr->chp_frac);
	  for (i = 0; i < 4; i++)
	    rd_string (m_ptr->history[i]);

	  s_ptr = &py.stats;
	  rd_shorts(s_ptr->max_stat, 6);
	  rd_shorts(s_ptr->cur_stat, 6);
	  rd_shorts((int16u *)s_ptr->mod_stat, 6);
	  rd_shorts(s_ptr->use_stat, 6);

	  f_ptr = &py.flags;
	  rd_long(&f_ptr->status);
	  rd_short((int16u *)&f_ptr->rest);
	  rd_short((int16u *)&f_ptr->blind);
	  rd_short((int16u *)&f_ptr->paralysis);
	  rd_short((int16u *)&f_ptr->confused);
	  rd_short((int16u *)&f_ptr->food);
	  rd_short((int16u *)&f_ptr->food_digested);
	  rd_short((int16u *)&f_ptr->protection);
	  rd_short((int16u *)&f_ptr->speed);
	  rd_short((int16u *)&f_ptr->fast);
	  rd_short((int16u *)&f_ptr->slow);
	  rd_short((int16u *)&f_ptr->afraid);
	  rd_short((int16u *)&f_ptr->cut);
	  rd_short((int16u *)&f_ptr->stun);
	  rd_short((int16u *)&f_ptr->poisoned);
	  rd_short((int16u *)&f_ptr->image);
	  rd_short((int16u *)&f_ptr->protevil);
	  rd_short((int16u *)&f_ptr->invuln);
	  rd_short((int16u *)&f_ptr->hero);
	  rd_short((int16u *)&f_ptr->shero);
	  rd_short((int16u *)&f_ptr->shield);
	  rd_short((int16u *)&f_ptr->blessed);
	  rd_short((int16u *)&f_ptr->resist_heat);
	  rd_short((int16u *)&f_ptr->resist_cold);
	  rd_short((int16u *)&f_ptr->resist_acid);
	  rd_short((int16u *)&f_ptr->resist_light);
	  rd_short((int16u *)&f_ptr->resist_poison);
	  rd_short((int16u *)&f_ptr->detect_inv);
	  rd_short((int16u *)&f_ptr->word_recall);
	  rd_short((int16u *)&f_ptr->see_infra);
	  rd_short((int16u *)&f_ptr->tim_infra);
	  rd_byte(&f_ptr->see_inv);
	  rd_byte(&f_ptr->teleport);
	  rd_byte(&f_ptr->free_act);
	  rd_byte(&f_ptr->slow_digest);
	  rd_byte(&f_ptr->aggravate);
	  rd_byte(&f_ptr->fire_resist);
	  rd_byte(&f_ptr->cold_resist);
	  rd_byte(&f_ptr->acid_resist);
	  rd_byte(&f_ptr->regenerate);
	  rd_byte(&f_ptr->lght_resist);
	  rd_byte(&f_ptr->ffall);
	  rd_byte(&f_ptr->sustain_str);
	  rd_byte(&f_ptr->sustain_int);
	  rd_byte(&f_ptr->sustain_wis);
	  rd_byte(&f_ptr->sustain_con);
	  rd_byte(&f_ptr->sustain_dex);
	  rd_byte(&f_ptr->sustain_chr);
	  rd_byte(&f_ptr->confuse_monster);
	  rd_byte(&f_ptr->new_spells);
	  rd_byte(&f_ptr->poison_resist);
	  rd_byte(&f_ptr->hold_life);
	  rd_byte(&f_ptr->telepathy);
	  rd_byte(&f_ptr->fire_im);
	  rd_byte(&f_ptr->acid_im);
	  rd_byte(&f_ptr->poison_im);
	  rd_byte(&f_ptr->cold_im);
	  rd_byte(&f_ptr->light_im);
	  rd_byte(&f_ptr->light);
	  rd_byte(&f_ptr->confusion_resist);
  	  rd_byte(&f_ptr->sound_resist);
	  rd_byte(&f_ptr->light_resist);
 	  rd_byte(&f_ptr->dark_resist);
	  rd_byte(&f_ptr->chaos_resist);
	  rd_byte(&f_ptr->disenchant_resist);
	  rd_byte(&f_ptr->shards_resist);
	  rd_byte(&f_ptr->nexus_resist);
	  rd_byte(&f_ptr->blindness_resist);
	  rd_byte(&f_ptr->nether_resist);

	  rd_short((int16u *)&missile_ctr);
	  rd_long((int32u *)&turn);
	  rd_short((int16u *)&inven_ctr);
	  if (inven_ctr > INVEN_WIELD) {
            prt("ERROR in inven_ctr", 8, 0);
	    goto error;
          }
	  for (i = 0; i < inven_ctr; i++)
	    rd_item(&inventory[i]);
	  for (i = INVEN_WIELD; i < INVEN_ARRAY_SIZE; i++)
	    rd_item(&inventory[i]);
	  rd_short((int16u *)&inven_weight);
	  rd_short((int16u *)&equip_ctr);
	  rd_long(&spell_learned);
	  rd_long(&spell_worked);
	  rd_long(&spell_forgotten);
	  rd_long(&spell_learned2);
	  rd_long(&spell_worked2);
	  rd_long(&spell_forgotten2);
	  rd_bytes(spell_order, 64);
	  rd_bytes(object_ident, OBJECT_IDENT_SIZE);
	  rd_long(&randes_seed);
	  rd_long(&town_seed);
	  rd_short((int16u *)&last_msg);
	  for (i = 0; i < MAX_SAVE_MSG; i++)
	    rd_string(old_msg[i]);

	  rd_short((int16u *)&panic_save);
	  rd_short((int16u *)&total_winner);
	  rd_short((int16u *)&noscore);
	  rd_shorts(player_hp, MAX_PLAYER_LEVEL);

	  if ((version_min >= 2)
	      || (version_min == 1 && patch_level >= 3))
	    for (i = 0; i < MAX_STORES; i++)
	      {
		st_ptr = &store[i];
		rd_long((int32u *)&st_ptr->store_open);
		rd_short((int16u *)&st_ptr->insult_cur);
		rd_byte(&st_ptr->owner);
		rd_byte(&st_ptr->store_ctr);
		rd_short(&st_ptr->good_buy);
		rd_short(&st_ptr->bad_buy);
		if (st_ptr->store_ctr > STORE_INVEN_MAX) {
                  prt("ERROR in store_ctr", 9, 0);
		  goto error;
                }
		for (j = 0; j < st_ptr->store_ctr; j++)
		  {
		    rd_long((int32u *)&st_ptr->store_inven[j].scost);
		    rd_item(&st_ptr->store_inven[j].sitem);
		  }
	      }

	  if ((version_min >= 2)
	      || (version_min == 1 && patch_level >= 3)) {
	    rd_long(&time_saved);
#ifndef SET_UID
	    if (!to_be_wizard) {
	      if (time_saved > (statbuf.st_ctime+100) ||
		  time_saved < (statbuf.st_ctime-100)) {
		    prt("Fiddled save file", 10, 0);
		    goto error;
		  }
	    }
#endif
	  }

	  if (version_min >= 2)
	    rd_string(died_from);
	}
      if ((c = getc(fileptr)) == EOF || (l & 0x80000000L))
	{
	  if ((l & 0x80000000L) == 0)
	    {
	      if (!to_be_wizard || turn < 0) {
                prt("ERROR in to_be_wizard", 10, 0);
		goto error;
              }
	      prt("Attempting a resurrection!", 0, 0);
	      if (py.misc.chp < 0)
		{
		  py.misc.chp =	 0;
		  py.misc.chp_frac = 0;
		}
	      /* don't let him starve to death immediately */
	      if (py.flags.food < 5000)
		py.flags.food = 5000;
	      cure_poison();
	      cure_blindness();
	      cure_confusion();
	      remove_fear();
	      if (py.flags.image > 0)
		py.flags.image = 0;
	      if (py.flags.cut > 0)
		py.flags.cut = 0;
	      if (py.flags.stun>0) {
	        if (py.flags.stun>50) {
		  py.misc.ptohit+=20;
		  py.misc.ptodam+=20;
	        } else {
		  py.misc.ptohit+=5;
		  py.misc.ptodam+=5;
	        }
	        py.flags.stun=0;
	      }
	      if (py.flags.word_recall > 0)
	        py.flags.word_recall = 0;
	      dun_level = 0; /* Resurrect on the town level. */
	      character_generated = 1;
	      /* set noscore to indicate a resurrection, and don't enter
		 wizard mode */
	      to_be_wizard = FALSE;
	      noscore |= 0x1;
	    }
	  else
	    {
	      prt("Restoring Memory of a departed spirit...", 0, 0);
	      turn = -1;
	    }
	  put_qio();
	  /* The log_index of the previous incarnation is here if later version
	     want to use it. For now, throw it away and get a new log. */
	  log_index = -1;
	  goto closefiles;
	}
      if (ungetc(c, fileptr) == EOF) {
        prt("ERROR in ungetc", 11, 0);
	goto error;
      }
      prt("Restoring Character...", 0, 0);
      put_qio();

      /* only level specific info should follow, not present for dead
         characters */

      rd_short((int16u *)&dun_level);
      rd_short((int16u *)&char_row);
      rd_short((int16u *)&char_col);
      rd_short((int16u *)&mon_tot_mult);
      rd_short((int16u *)&cur_height);
      rd_short((int16u *)&cur_width);
      rd_short((int16u *)&max_panel_rows);
      rd_short((int16u *)&max_panel_cols);

      /* read in the creature ptr info */
      rd_byte(&char_tmp);
      while (char_tmp != 0xFF)
	{
	  ychar = char_tmp;
	  rd_byte(&xchar);
	  rd_byte(&char_tmp);
	  if (xchar > MAX_WIDTH || ychar > MAX_HEIGHT) {
	    vtype temp;
	    sprintf(temp,
		"Error in creature ptr info: x=%x, y=%x, char_tmp=%x",
		xchar, ychar, char_tmp);
            prt(temp, 11, 0);
	  }
	  cave[ychar][xchar].cptr = char_tmp;
	  rd_byte(&char_tmp);
	}
      /* read in the treasure ptr info */
      rd_byte(&char_tmp);
      while (char_tmp != 0xFF)
	{
	  ychar = char_tmp;
	  rd_byte(&xchar);
	  rd_short((int16u *)&int16u_tmp);
	  if (xchar > MAX_WIDTH || ychar > MAX_HEIGHT) {
            prt("Error in treasure pointer info", 12, 0);
	    goto error;
	  }
	  cave[ychar][xchar].tptr = int16u_tmp;
	  rd_byte(&char_tmp);
	}
      /* read in the rest of the cave info */
      c_ptr = &cave[0][0];
      total_count = 0;
      while (total_count != MAX_HEIGHT*MAX_WIDTH)
	{
	  rd_byte(&count);
	  rd_byte(&char_tmp);
	  for (i = count; i > 0; i--)
	    {
#ifndef ATARIST_MWC
	      if (c_ptr >= &cave[MAX_HEIGHT][0]) {
		prt("ERROR in cave size", 13, 0);
		goto error;
	      }
#endif
	      c_ptr->fval = char_tmp & 0xF;
	      c_ptr->lr = (char_tmp >> 4) & 0x1;
	      c_ptr->fm = (char_tmp >> 5) & 0x1;
	      c_ptr->pl = (char_tmp >> 6) & 0x1;
	      c_ptr->tl = (char_tmp >> 7) & 0x1;
	      c_ptr++;
	    }
	  total_count += count;
	}

      rd_short((int16u *)&tcptr);
      if (tcptr > MAX_TALLOC) {
	prt("ERROR in MAX_TALLOC", 14, 0);
	goto error;
      }
      for (i = MIN_TRIX; i < tcptr; i++)
	rd_item(&t_list[i]);
      rd_short((int16u *)&mfptr);
      if (mfptr > MAX_MALLOC) {
	prt("ERROR in MAX_MALLOC", 15, 0);
	goto error;
      }
      for (i = MIN_MONIX; i < mfptr; i++)
	rd_monster(&m_list[i]);

#ifdef MSDOS
      /* change walls and floors to graphic symbols */
      t_ptr = &t_list[tcptr - 1];
      for (i = tcptr - 1; i >= MIN_TRIX; i--)
	{
	  if (t_ptr->tchar == '#')
	    t_ptr->tchar = wallsym;
	  t_ptr--;
	}
#endif

      /* Restore ghost names & stats etc... */
      c_list[MAX_CREATURES - 1].name[0]='A';
      rd_bytes((int8u *)(c_list[MAX_CREATURES - 1].name), 100);
      rd_long((int32u *)&(c_list[MAX_CREATURES - 1].cmove));
      rd_long((int32u *)&(c_list[MAX_CREATURES - 1].spells));
      rd_long((int32u *)&(c_list[MAX_CREATURES - 1].cdefense));
      {  int16u temp;  /* fix player ghost's exp bug.  The mexp field is
      			  really an int32u, but the savefile was writing/
      			  reading an int16u.  Since I don't want to change
      			  the savefile format, this insures that the mexp
      			  field is loaded, and that the "high bits" of
      			  mexp do not contain garbage values which could
      			  mean that player ghost are worth millions of
			  exp. -CFT */
        rd_short((int16u *)&temp);
        c_list[MAX_CREATURES-1].mexp = (int32u)temp;
      }
      rd_byte((int8u *)&(c_list[MAX_CREATURES - 1].sleep));
      rd_byte((int8u *)&(c_list[MAX_CREATURES - 1].aaf));
      rd_byte((int8u *)&(c_list[MAX_CREATURES - 1].ac));
      rd_byte((int8u *)&(c_list[MAX_CREATURES - 1].speed));
      rd_byte((int8u *)&(c_list[MAX_CREATURES - 1].cchar));
      rd_bytes((int8u *)(c_list[MAX_CREATURES - 1].hd), 2);
      rd_bytes((int8u *)(c_list[MAX_CREATURES - 1].damage), sizeof(attid)*4);
      rd_short((int16u *)&(c_list[MAX_CREATURES - 1].level));
      *generate = FALSE;  /* We have restored a cave - no need to generate. */

      if ((version_min == 1 && patch_level < 3)
	  || (version_min == 0))
	for (i = 0; i < MAX_STORES; i++)
	  {
	    st_ptr = &store[i];
	    rd_long((int32u *)&st_ptr->store_open);
	    rd_short((int16u *)&st_ptr->insult_cur);
	    rd_byte(&st_ptr->owner);
	    rd_byte(&st_ptr->store_ctr);
	    rd_short(&st_ptr->good_buy);
	    rd_short(&st_ptr->bad_buy);
	    if (st_ptr->store_ctr > STORE_INVEN_MAX) {
	      prt("ERROR in STORE_INVEN_MAX", 16, 0);
	      goto error;
	    }
	    for (j = 0; j < st_ptr->store_ctr; j++)
	      {
		rd_long((int32u *)&st_ptr->store_inven[j].scost);
		rd_item(&st_ptr->store_inven[j].sitem);
	      }
	  }

      /* read the time that the file was saved */
      if (version_min == 0 && patch_level < 16)
	time_saved = 0; /* no time in file, clear to zero */
      else if (version_min == 1 && patch_level < 3)
	rd_long(&time_saved);

      if (ferror(fileptr)) {
	prt("FILE ERROR", 17, 0);
	goto error;
      }

      if (turn < 0) {
        prt("Error = turn < 0", 7, 0);
      error:
	ok = FALSE;	/* Assume bad data. */
        }
      else
	{
	  /* don't overwrite the killed by string if character is dead */
	  if (py.misc.chp >= 0)
	    (void) strcpy(died_from, "(alive and well)");
	  character_generated = 1;
	}

    closefiles:

      if (fileptr != NULL)
	{
	  if (fclose(fileptr) < 0)
	    ok = FALSE;
	}
      if (fd >= 0)
	(void) close(fd);

      if (!ok)
	msg_print("Error during reading of file.");
      else if (turn >= 0 && !_new_log())
	msg_print("Can't log player in the log file.");
      else
	{
	  /* let the user overwrite the old savefile when save/quit */
	  from_savefile = 1;

	  signals();

	  if (turn >= 0)
	    {	/* Only if a full restoration. */
	      weapon_heavy = FALSE;
	      pack_heavy = 0;
	      check_strength();

	      /* rotate store inventory, depending on how old the save file */
	      /* is foreach day old (rounded up), call store_maint */
	      /* calculate age in seconds */
	      start_time = time((long *)0);
	      /* check for reasonable values of time here ... */
	      if (start_time < time_saved)
		age = 0;
	      else
		age = start_time - time_saved;

	      age = (age + 43200L) / 86400L;  /* age in days */
	      if (age > 10) age = 10; /* in case savefile is very old */
	      for (i = 0; i < age; i++)
		store_maint();
	    }
	  /*
	  if (noscore)
	    msg_print("This save file cannot be used to get on the score board.");
	  */

	  if (version_maj != CUR_VERSION_MAJ
	      || version_min != CUR_VERSION_MIN)
	    {
	      (void) sprintf(temp,
			     "Save file version %d.%d %s on game version %d.%d.",
			     version_maj, version_min,
			     version_maj == CUR_VERSION_MAJ
			     ? "accepted" : "very risky" ,
			     CUR_VERSION_MAJ, CUR_VERSION_MIN);
	      msg_print(temp);
	    }

	  if (turn >= 0) {
	    char *tmp2;

	    tmp2=basename(savefile);

	    (void) sprintf(temp, "%s/p.%s", ANGBAND_SAV, (tmp2+1));

	    link(savefile, temp);
	    unlink(savefile);
	    return TRUE;
	  } else {
	    return FALSE;	/* Only restored options and monster memory. */
	  }
	}
    }
  turn = -1;
  log_index = -1;
  prt("Please try again without that savefile.", 1, 0);
  signals();
#ifdef MAC
  *exit_flag = TRUE;
#else
  exit_game();
#endif

  return FALSE;	/* not reached, unless on mac */
}

static void wr_byte(c)
int8u c;
{
  xor_byte ^= c;
  (void) putc((int)xor_byte, fileptr);
}

static void wr_short(s)
int16u s;
{
  xor_byte ^= (s & 0xFF);
  (void) putc((int)xor_byte, fileptr);
  xor_byte ^= ((s >> 8) & 0xFF);
  (void) putc((int)xor_byte, fileptr);
}

static void wr_long(l)
register int32u l;
{
  xor_byte ^= (l & 0xFF);
  (void) putc((int)xor_byte, fileptr);
  xor_byte ^= ((l >> 8) & 0xFF);
  (void) putc((int)xor_byte, fileptr);
  xor_byte ^= ((l >> 16) & 0xFF);
  (void) putc((int)xor_byte, fileptr);
  xor_byte ^= ((l >> 24) & 0xFF);
  (void) putc((int)xor_byte, fileptr);
}

static void wr_bytes(c, count)
int8u *c;
register int count;
{
  register int i;
  register int8u *ptr;

  ptr = c;
  for (i = 0; i < count; i++)
    {
      xor_byte ^= *ptr++;
      (void) putc((int)xor_byte, fileptr);
    }
}

static void wr_string(str)
register char *str;
{
  while (*str != '\0')
    {
      xor_byte ^= *str++;
      (void) putc((int)xor_byte, fileptr);
    }
  xor_byte ^= *str;
  (void) putc((int)xor_byte, fileptr);
}

static void wr_shorts(s, count)
int16u *s;
register int count;
{
  register int i;
  register int16u *sptr;

  sptr = s;
  for (i = 0; i < count; i++)
    {
      xor_byte ^= (*sptr & 0xFF);
      (void) putc((int)xor_byte, fileptr);
      xor_byte ^= ((*sptr++ >> 8) & 0xFF);
      (void) putc((int)xor_byte, fileptr);
    }
}

static void wr_item(item)
register inven_type *item;
{
  wr_short(item->index);
  wr_byte(item->name2);
  wr_string(item->inscrip);
  wr_long(item->flags);
  wr_byte(item->tval);
  wr_byte(item->tchar);
  wr_short((int16u)item->p1);
  wr_long((int32u)item->cost);
  wr_byte(item->subval);
  wr_byte(item->number);
  wr_short(item->weight);
  wr_short((int16u)item->tohit);
  wr_short((int16u)item->todam);
  wr_short((int16u)item->ac);
  wr_short((int16u)item->toac);
  wr_bytes(item->damage, 2);
  wr_byte(item->level);
  wr_byte(item->ident);
  wr_long(item->flags2);
  wr_short((int16u)item->timeout);
}

static void wr_monster(mon)
register monster_type *mon;
{
  wr_short((int16u)mon->hp);
  wr_short((int16u)mon->csleep);
  wr_short((int16u)mon->cspeed);
  wr_short(mon->mptr);
  wr_byte(mon->fy);
  wr_byte(mon->fx);
  wr_byte(mon->cdis);
  wr_byte(mon->ml);
  wr_byte(mon->stunned);
  wr_byte(mon->confused);
}

static void rd_byte(ptr)
int8u *ptr;
{
  int8u c;

  c = getc(fileptr) & 0xFF;
  *ptr = c ^ xor_byte;
  xor_byte = c;
}

static void rd_short(ptr)
int16u *ptr;
{
  int8u c;
  int16u s;

  c = (getc(fileptr) & 0xFF);
  s = c ^ xor_byte;
  xor_byte = (getc(fileptr) & 0xFF);
  s |= (int16u)(c ^ xor_byte) << 8;
  *ptr = s;
}

static void rd_long(ptr)
int32u *ptr;
{
  register int32u l;
  register int8u c;

  c = (getc(fileptr) & 0xFF);
  l = c ^ xor_byte;
  xor_byte = (getc(fileptr) & 0xFF);
  l |= (int32u)(c ^ xor_byte) << 8;
  c = (getc(fileptr) & 0xFF);
  l |= (int32u)(c ^ xor_byte) << 16;
  xor_byte = (getc(fileptr) & 0xFF);
  l |= (int32u)(c ^ xor_byte) << 24;
  *ptr = l;
}

static void rd_bytes(ptr, count)
int8u *ptr;
int count;
{
  int i;
  int8u c;
  int8u nc;

  for (i = 0; i < count; i++)
    {
      c = (getc(fileptr) & 0xFF);
      nc = c ^ xor_byte;
      *ptr = nc;
      ptr++;
      xor_byte = c;
    }
}

static void rd_string(str)
char *str;
{
  register int8u c;

  do
    {
      c = (getc(fileptr) & 0xFF);
      *str = c ^ xor_byte;
      xor_byte = c;
    }
  while (*str++ != '\0');
}

static void rd_shorts(ptr, count)
int16u *ptr;
register int count;
{
  register int i;
  register int16u *sptr;
  register int16u s;
  int8u c;

  sptr = ptr;
  for (i = 0; i < count; i++)
    {
      c = (getc(fileptr) & 0xFF);
      s = c ^ xor_byte;
      xor_byte = (getc(fileptr) & 0xFF);
      s |= (int16u)(c ^ xor_byte) << 8;
      *sptr++ = s;
    }
}

static void rd_item(item)
register inven_type *item;
{
  rd_short(&item->index);
  rd_byte(&item->name2);
  rd_string(item->inscrip);
  rd_long(&item->flags);
  rd_byte(&item->tval);
  rd_byte(&item->tchar);
  rd_short((int16u *)&item->p1);
  rd_long((int32u *)&item->cost);
  rd_byte(&item->subval);
  rd_byte(&item->number);
  rd_short(&item->weight);
  rd_short((int16u *)&item->tohit);
  rd_short((int16u *)&item->todam);
  rd_short((int16u *)&item->ac);
  rd_short((int16u *)&item->toac);
  rd_bytes(item->damage, 2);
  rd_byte(&item->level);
  rd_byte(&item->ident);
  rd_long(&item->flags2);
  rd_short((int16u *)&item->timeout);
}

static void rd_monster(mon)
register monster_type *mon;
{
  rd_short((int16u *)&mon->hp);
  rd_short((int16u *)&mon->csleep);
  rd_short((int16u *)&mon->cspeed);
  rd_short(&mon->mptr);
  rd_byte(&mon->fy);
  rd_byte(&mon->fx);
  rd_byte(&mon->cdis);
  rd_byte(&mon->ml);
  rd_byte(&mon->stunned);
  rd_byte(&mon->confused);
}
