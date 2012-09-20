/* recall.c: print out monster memory info			-CJS-

   Copyright (c) 1989 James E. Wilson, Christopher J. Stuart

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

#include "constant.h"
#include "config.h"
#include "monster.h"
#include "types.h"
#include "externs.h"

#include <stdio.h>

static void roff(ARG_CHAR_PTR);
int strlen(ARG_CHAR_PTR);


static char *desc_atype[] = {
  "do something undefined",
  "attack",
  "weaken",
  "confuse",
  "terrify",
  "burn",
  "shoot acid",
  "freeze",
  "electrify",
  "corrode",
  "blind",
  "paralyse",
  "steal money",
  "steal things",
  "poison",
  "reduce dexterity",
  "reduce constitution",
  "drain intelligence",
  "drain wisdom",
  "lower experience",
  "call for help",
  "disenchant",
  "eat your food",
  "absorb light",
  "absorb charges",
  "reduce all stats" };
static char *desc_amethod[] = {
  "make an undefined advance",
  "hit",
  "bite",
  "claw",
  "sting",
  "touch",
  "kick",
  "gaze",
  "breathe",
  "spit",
  "wail",
  "embrace",
  "crawl on you",
  "release spores",
  "beg",
  "slime you",
  "crush",
  "trample",
  "drool",
  "insult",
  "butt",
  "charge",
  "engulf",
  "moan"
};
static char *desc_howmuch[] = {
  " not at all",
  " a bit",
  "",
  " quite",
  " very",
  " most",
  " highly",
  " extremely" };

static char *desc_move[] = {
  "move invisibly",
  "open doors",
  "pass through walls",
  "kill weaker creatures",
  "pick up objects",
  "breed explosively" };
static char *desc_spell[] = {
  "1",
  "2",
  "4",
  "8", /* 1+2+4+8=0xF = CS_FREQ... freaky huh? */
  "teleport short distances",
  "teleport long distances",
  "teleport to its prey",
  "cause light wounds",
  "cause serious wounds",
  "paralyse its prey",
  "induce blindness",
  "confuse",
  "terrify",
  "summon a monster",
  "summon the undead",
  "slow its prey",
  "drain mana",
  "summon demonkind",
  "summon dragonkind",
  "lightning",        /* breaths 1 */
  "poison gases",
  "acid",
  "frost",
  "fire",             /* end */
  "cast fire bolts",
  "cast frost bolts",
  "cast acid bolts",
  "cast magic missiles",
  "cause critical wounds",
  "cast fire balls",
  "cast frost balls",
  "cast mana bolts",
  "chaos",          /* breaths2 */
  "shards",
  "sound",
  "confusion",
  "disenchantment",
  "nether",         /* end of part1 */
  "cast lightning bolts",
  "cast lightning balls",
  "cast acid balls",
  "create traps",
  "wound its prey",
  "cast mind blasting",
  "teleport away its prey",
  "heal itself",
  "haste itself",
  "fire missiles",
  "cast plasma bolts",
  "summon many creatures",
  "cast nether bolts",
  "cast ice bolts",
  "cast darkness",
  "cause amnesia",
  "sear your mind",
  "cast stinking clouds",
  "teleport its preys level",
  "cast water bolts",
  "cast whirlpools",
  "cast nether balls",
  "summon an angel",
  "summon spiders",
  "summon hounds",
  "nexus",            /* part 2... */
  "elemental force",  /* breaths 3 */
  "inertia",
  "light",
  "time",
  "gravity",
  "darkness",
  "plasma",           /* end */
  "fire arrows",
  "summon Ringwraiths",
  "cast darkness storms",
  "cast mana storms",
  "summon reptiles",
  "summon ants",
  "summon unique creatures",
  "summon greater undead",
  "summon ancient dragons",
};
static char *desc_weakness[] = {
  "bright light",
  "rock remover"
};
static char *desc_immune[] = {
  "frost",
  "fire",
  "lightning",
  "poison",
  "acid"
};

static vtype roffbuf;		/* Line buffer. */
static char *roffp;		/* Pointer into line buffer. */
static int roffpline;		/* Place to print line now being loaded. */

#define plural(c, ss, sp)	((c) == 1 ? ss : sp)

/* Number of kills needed for information. */

/* the higher the level of the monster, the fewer the kills you need */
#define knowarmor(l,d)		((d) > 304 / (4 + (l)))
/* the higher the level of the monster, the fewer the attacks you need,
   the more damage an attack does, the more attacks you need */
#define knowdamage(l,a,d)	((4 + (l))*(a) > 80 * (d))

/* use slightly different tests for unique monsters to hasten learning -CFT */
#define knowuniqarmor(l,d)	((d) > 304 / (38 + (5*(l))/4))
#define knowuniqdamage(l,a,d)	((4 +(long)(l))*(2*(long)(a)) > 80 * (d))

/* Do we know anything about this monster? */
int bool_roff_recall(mon_num)
int mon_num;
{
  register recall_type *mp;
  register int i;

  if (wizard)
    return TRUE;
  mp = &c_recall[mon_num];
  if (mp->r_cmove || mp->r_cdefense || mp->r_kills || mp->r_spells ||
      mp->r_spells2 || mp->r_spells3 || mp->r_deaths)
    return TRUE;
  for (i = 0; i < 4; i++)
    if (mp->r_attacks[i])
      return TRUE;
  return FALSE;
}

/* Print out what we have discovered about this monster. */
int roff_recall(mon_num)
int mon_num;
{
  char *p, *q;
  attid *pu;
  vtype temp;
  register recall_type *mp;
  register creature_type *cp;
  register int32 i, k; /* changed from int, to avoid PC's 16bit ints -CFT */
  int32u j;
  int mspeed;
  int32u rcmove, rspells, rspells2, rspells3;
  int32u rcdefense;  /* this was int16u, but c_recall[] uses int32u -CFT */
  recall_type save_mem;
  int breath=FALSE, magic=FALSE;
#ifdef MSDOS
/* added to allow external description file -CFT */
  FILE *descfile;
#define MAX_DESC_LEN 1024 /* even morgorth's fits... -CFT */
  char buf[MAX_DESC_LEN];
  long skptr; /* used to seek into file with... -CFT */
#endif  

  cp = &c_list[mon_num];

  mp = &c_recall[mon_num];
  if (wizard) {
    save_mem = *mp;
    mp->r_kills = MAX_SHORT;
    mp->r_wake = mp->r_ignore = MAX_UCHAR;
    j = (((cp->cmove & CM_4D2_OBJ) != 0) * 8) +
      (((cp->cmove & CM_2D2_OBJ) != 0) * 4) +
	(((cp->cmove & CM_1D2_OBJ) != 0) * 2) +
	  ((cp->cmove & CM_90_RANDOM) != 0) +
	    ((cp->cmove & CM_60_RANDOM) != 0);
    mp->r_cmove = (cp->cmove & ~CM_TREASURE) | (j << CM_TR_SHIFT);
    mp->r_cdefense = cp->cdefense;
    mp->r_spells = cp->spells | CS_FREQ;
    mp->r_spells2 = cp->spells2;
    mp->r_spells3 = cp->spells3;
    j = 0;
    pu = cp->damage;
    while (*pu != 0 && j < 4) {
      mp->r_attacks[j] = MAX_UCHAR;
      j++;
      pu++;
    }
  }
  roffpline = 0;
  roffp = roffbuf;
  rspells = mp->r_spells & cp->spells & ~CS_FREQ;
  rspells2 = mp->r_spells2 & cp->spells2;
  rspells3 = mp->r_spells3 & cp->spells3;
  /* the CM_WIN property is always known, set it if a win monster */
  rcmove = mp->r_cmove | (CM_WIN & cp->cmove);
  rcdefense = mp->r_cdefense & cp->cdefense;
#ifdef TC_COLOR
  if (!no_color_flag && !inven_bw_flag) /* then we'll colorize the name */
    textcolor(mon_color(cp->color));
#endif
  if (cp->cdefense & UNIQUE)
    (void) sprintf(temp, "%s:\n", cp->name);
  else
    (void) sprintf(temp, "The %s:\n", cp->name);
  roff(temp);
#ifdef TC_COLOR
  textcolor(LIGHTGRAY);
#endif
  /* Conflict history. */
/* changed to act better for unique monsters -CFT */
  if (cp->cdefense & UNIQUE) { /* treat unique differently... -CFT */
    if (mp->r_deaths) { /* We've been killed... */
      (void) sprintf(temp, "It has slain %d of your ancestors", mp->r_deaths);
      roff(temp);
      if (u_list[mon_num].dead) { /* but we've also killed it */
        sprintf(temp, ", but you have avenged %s! ",
		plural(mp->r_deaths, "him", "them"));
	roff(temp);
        }
      else {
        sprintf(temp, ", who %s unavenged. ",
		plural(mp->r_deaths, "remains", "remain"));
	roff(temp);
        }
      }        
    else if (u_list[mon_num].dead) { /* we killed it w/o dying... yet! */
      roff("You have slain this foe. ");
      }
    }      
  else if (mp->r_deaths) { /* not unique.... */
  (void) sprintf(temp,
		   "%d of your ancestors %s",
		   mp->r_deaths, plural(mp->r_deaths, "has", "have") );
    roff(temp);
    roff(" been killed by this creature, and ");
    if (mp->r_kills == 0)
      roff("it is not ever known to have been defeated. ");
    else {
      (void) sprintf(temp,
		       "at least %d of the beasts %s been exterminated. ",
		       mp->r_kills, plural(mp->r_kills, "has", "have") );
      roff(temp);
      }
    }
  else if (mp->r_kills) {
    (void) sprintf(temp, "At least %d of these creatures %s",
		     mp->r_kills, plural(mp->r_kills, "has", "have") );
    roff(temp);
    roff(" been killed by you and your ancestors. ");
    }
   else roff("No battles to the death are recalled. ");
  /* Immediately obvious. */
#ifdef MSDOS
/* begin my external description code.  It'll be slow as hell if you're
   looking up a lot of monsters, but it'll fit inside PC's stupid 640K
   memory limit MUCH better... -CFT */
  if ( (descfile = fopen(ANGBAND_DESC, "rb")) == NULL ) {
    roff(" You feel you know it, and it knows you. This can only mean trouble.");
  } else { /* found desc file... */  
    fseek(descfile, (long)(mon_num * sizeof(long)), SEEK_SET);
    fread(&skptr, sizeof(skptr), 1, descfile); /* get index into file */
    fseek(descfile, skptr, SEEK_SET); /* go to desc. */
    fgets(buf, MAX_DESC_LEN, descfile); /* read it */
    fclose(descfile); /* and we're done w/ descfile... */
    buf[MAX_DESC_LEN-1] = 0; /* just-in-case null termination... -CFT */
    if (strlen(buf))
      buf[strlen(buf) - 1] = 0; /* insure null term, and rem final \n */
    roff(buf);
    roff(".");
  }    
#else /* non-MSDOS systems, just use orig code... */
/* This is the original description code.  It uses sequential search! Yuck!
   Is there some way we can better this?  Maybe make the desc file sorted
   by mon name, then use binary search?  Maybe make the desc file match
   order with monsters.c, then just use mon_num to index direct?  Both of
   those require work to re-order desc file, and I don't feel up to it
   just now... Plus they make it a pain to add new monsters, so I doubt
   I should do it in an alpha phase...  On the other hand, sequential
   search of data in memory is probably orders of magnitude faster than my
   (above) direct indexed file read, so it's not THAT bad... -CFT */
  for (k=0; k<MAX_CREATURES; k++) {
    if (!stricmp(desc_list[k].name, cp->name)) {
      if (strlen(desc_list[k].desc) != 0) {
	roff(" ");
	roff(desc_list[k].desc);
	roff(".");
      }
      break;
    }
  }
  if (k==MAX_CREATURES)
    roff(" You feel you know it, and it knows you. This can only mean trouble.");
#endif

  k = FALSE;
  if (cp->level == 0)
    {
      roff(" It lives in the town");
      k = TRUE;
    }
  else if (mp->r_kills)
    {
      (void) sprintf(temp, " It is normally found at depths of %d feet",
		     cp->level*50);
      roff(temp);
      k = TRUE;
    }
  /* the c_list speed value is 10 greater, so that it can be a int8u */
  mspeed = cp->speed - 10;
  if ((rcmove & CM_ALL_MV_FLAGS) || (rcmove & CM_RANDOM_MOVE))
    {
      if (k)
	roff(", and");
      else
	{
	  roff(" It");
	  k = TRUE;
	}
      roff(" moves");
      if (rcmove & CM_RANDOM_MOVE)
	{
	  roff(desc_howmuch[(rcmove & CM_RANDOM_MOVE) >> 3]);
	  roff(" erratically");
	}
      if (mspeed == 1)
	roff(" at normal speed");
      else
	{
	  if (rcmove & CM_RANDOM_MOVE)
	    roff(", and");
	  if (mspeed <= 0)
	    {
	      if (mspeed == -1)
		roff(" very");
	      else if (mspeed < -1)
		roff(" incredibly");
	      roff(" slowly");
	    }
	  else
	    {
	      if (mspeed == 3)
		roff(" very");
	      else if (mspeed > 3)
		roff(" unbelievably");
	      roff(" quickly");
	    }
	}
    }
  if (rcmove & CM_ATTACK_ONLY)
    {
      if(k)
	roff(", but");
      else
	{
	  roff(" It");
	  k = TRUE;
	}
      roff(" does not deign to chase intruders");
    }
  if(k)
    roff(".");
  /* Kill it once to know experience, and quality (evil, undead, monsterous).
     The quality of being a dragon is obvious. */
  if (mp->r_kills)
    {
      roff(" A kill of this");  /* elses removed because animal/evil/undead
      				   aren't mutually exclusive -CFT */
      if (cp->cdefense & ANIMAL)
	roff(" natural");
      if (cp->cdefense & EVIL)
	roff(" evil");
      if (cp->cdefense & UNDEAD)
	roff(" undead");

      if (cp->cdefense & GIANT)
	roff(" giant");
      else if (cp->cdefense & ORC)
	roff(" orc");
      else if (cp->cdefense & DRAGON)
	roff(" dragon");
      else if (cp->cdefense & DEMON)
	roff(" demon");
      else if (cp->cdefense & TROLL)
	roff(" troll");
      else roff(" creature");

      /* calculate the integer exp part */
      i = (long)cp->mexp * cp->level / py.misc.lev;
      /* calculate the fractional exp part scaled by 100,
	 must use long arithmetic to avoid overflow */
      j = (((long)cp->mexp * cp->level % py.misc.lev) * (long)1000 /
	   py.misc.lev+5) / 10;

      (void) sprintf(temp, " is worth %ld.%02ld point%s", i,
		     j, (i == 1 && j == 0 ? "" : "s"));
      roff(temp);

      if ((py.misc.lev / 10) == 1) p = "th";
      else
	{
	  i = py.misc.lev % 10;
	  if (i == 1)		p = "st";
	  else if (i == 2)	p = "nd";
	  else if (i == 3)      p = "rd";
	  else			p = "th";
	}
      i = py.misc.lev;
      if ((i == 8) || (i == 11) || (i == 18)) q = "n";
      else				q = "";
      (void) sprintf(temp, " for a%s %ld%s level character.", q, i, p);
      roff(temp);
      if (cp->cdefense & GROUP)
	roff(" It usually appears in groups.");
    }
  /* Spells known, if have been used against us. */
  k = TRUE;
  i = 0;
  if ((rspells&CS_BREATHE) || (rspells2&CS_BREATHE2) ||
      (rspells3&CS_BREATHE3)) {
    breath=TRUE;
    j = rspells & CS_BREATHE;
    rspells &= ~CS_BREATHE;
    while ((i=bit_pos(&j)) != -1) {
      if (k) {
	roff(" It can breathe ");
	k = FALSE;
      }
      else if (j || (rspells2&CS_BREATHE2) || (rspells3&CS_BREATHE3))
	roff(", ");
      else
	roff(" and ");
      roff(desc_spell[i]);
    }
    j = rspells2 & CS_BREATHE2;
    rspells2 &= ~CS_BREATHE2;
    while ((i=bit_pos(&j)) != -1) {
      if (k) {
	roff(" It can breathe ");
	k = FALSE;
      }
      else if (j || (rspells3&CS_BREATHE3))
	roff(", ");
      else
	roff(" and ");
      roff(desc_spell[i+32]);
    }
    j = rspells3 & CS_BREATHE3;
    rspells &= ~CS_BREATHE3;
    while ((i=bit_pos(&j)) != -1) {
      if (k) {
	roff(" It can breathe ");
	k = FALSE;
      }
      else if (j)
	roff(", ");
      else
	roff(" and ");
      roff(desc_spell[i+64]);
    }
  }
  k = TRUE;
  if (rspells || rspells2 || rspells3) {
    magic=TRUE;
    j=rspells & ~CS_BREATHE;
    while ((i=bit_pos(&j)) != -1) {
      if (k) {
	if (breath)
	  roff(", and is also");
	else
	  roff(" It is");
	if (mp->r_cdefense & INTELLIGENT)
	  roff(" magical, casting spells intelligently which ");
	else
	  roff(" magical, casting spells which ");
	k = FALSE;
      }
      else if ((j&CS_SPELLS) || rspells2 || rspells3)
	roff(", ");
      else
	roff(" or ");
      roff(desc_spell[i]);
    }
    j=rspells2 & ~CS_BREATHE2;
    while ((i=bit_pos(&j)) != -1) {
      if (k) {
	if (breath)
	  roff(", and is also");
	else
	  roff(" It is");
	roff(" magical, casting spells which ");
	k = FALSE;
      }
      else if (j || rspells3)
	roff(", ");
      else
	roff(" or ");
      roff(desc_spell[i+32]);
    }
    j=rspells3 & ~CS_BREATHE3;
    while ((i=bit_pos(&j)) != -1) {
      if (k) {
	if (breath)
	  roff(", and is also");
	else
	  roff(" It is");
	roff(" magical, casting spells which ");
	k = FALSE;
      }
      else if (j)
	roff(", ");
      else
	roff(" or ");
      roff(desc_spell[i+64]);
    }
  }
  if (breath || magic) {
    if ((mp->r_spells & CS_FREQ) > 5) { /* Could offset by level */
      (void) sprintf(temp, "; 1 time in %ld", cp->spells & CS_FREQ);
      roff(temp);
    }
    roff(".");
  }
  /* Do we know how hard they are to kill? Armor class, hit die. */
  if (knowarmor(cp->level, mp->r_kills) || /* hasten learning of uniques -CFT */
      ((cp->cdefense & UNIQUE) && knowuniqarmor(cp->level, mp->r_kills)))
    {
      (void) sprintf(temp, " It has an armor rating of %d", cp->ac);
      roff(temp);
      (void) sprintf(temp, " and a%s life rating of %dd%d.",
		     ((cp->cdefense & MAX_HP) ? " maximized" : ""),
		     cp->hd[0], cp->hd[1]);
      roff(temp);
    }
  /* Do we know how clever they are? Special abilities. */
  k = TRUE;
  j = rcmove;
  if (rcdefense & BREAK_WALL) { /* I wonder why this wasn't here before? -CFT */
    roff(" It can bore through rock");
    k = FALSE;
    }
  for (i = 0; j & CM_SPECIAL; i++) {
    if (j & (CM_INVISIBLE << i)) {
      j &= ~(CM_INVISIBLE << i);
      if (k) {
        roff(" It can ");
	k = FALSE;
      }
      else if (j & CM_SPECIAL)
	roff(", ");
      else
	roff(" and ");
      roff(desc_move[i]);
    }
  }
  if (!k)
    roff(".");
  /* Do we know its special weaknesses? Most cdefense flags. */
  k = TRUE;
  j = rcdefense;
  if (j & HURT_LIGHT) {
    if (k) {
	roff(" It is susceptible to ");
	roff(desc_weakness[0]);
	k = FALSE;
      }
  }
  if (j & HURT_ROCK) {
    if (k) {
      roff(" It is susceptible to ");
      roff(desc_weakness[1]);
      k = FALSE;
    }
    else {
      roff(" and ");
      roff(desc_weakness[1]);
    }
  }

  if (!k)
    roff(".");
  /* Do we know its special weaknesses? Most cdefense flags. */
  k = TRUE;
  for (i = 0; j & (IM_FROST|IM_FIRE|IM_ACID|IM_POISON|IM_LIGHTNING); i++)
    {
      if (j & (IM_FROST << i))
	{
	  j &= ~(IM_FROST << i);
	  if (k)
	    {
	      roff(" It resists ");
	      k = FALSE;
	    }
	  else if (j & (IM_FROST|IM_FIRE|IM_ACID|IM_POISON|IM_LIGHTNING))
	    roff(", ");
	  else
	    roff(" and ");
	  roff(desc_immune[i]);
	}
    }
  if (!k)
    roff(".");
  if (rcdefense & NO_INFRA)
    roff(" It is cold blooded");
  if (rcdefense & CHARM_SLEEP)
    {
      if (rcdefense & NO_INFRA)
	roff(", and");
      else
	roff(" It");
      roff(" cannot be charmed or slept");
    }
  if (rcdefense & (CHARM_SLEEP|NO_INFRA))
    roff(".");
  /* Do we know how aware it is? */
  if (((mp->r_wake * mp->r_wake) > cp->sleep) || mp->r_ignore == MAX_UCHAR ||
      (cp->sleep == 0 && mp->r_kills >= 10))
    {
      roff(" It ");
      if(cp->sleep > 200)
	roff("prefers to ignore");
      else if(cp->sleep > 95)
	roff("pays very little attention to");
      else if(cp->sleep > 75)
	roff("pays little attention to");
      else if(cp->sleep > 45)
	roff("tends to overlook");
      else if(cp->sleep > 25)
	roff("takes quite a while to see");
      else if(cp->sleep > 10)
	roff("takes a while to see");
      else if(cp->sleep > 5)
	roff("is fairly observant of");
      else if(cp->sleep > 3)
	roff("is observant of");
      else if(cp->sleep > 1)
	roff("is very observant of");
      else if(cp->sleep != 0)
	roff("is vigilant for");
      else
	roff("is ever vigilant for");
      (void) sprintf(temp, " intruders, which it may notice from %d feet.",
		     10 * cp->aaf);
      roff(temp);
    }
  /* Do we know what it might carry? */
  if (rcmove & (CM_CARRY_OBJ|CM_CARRY_GOLD))
    {
      roff(" It may");
      j = (rcmove & CM_TREASURE) >> CM_TR_SHIFT;
      if (j == 1)
	{
	  if ((cp->cmove & CM_TREASURE) == CM_60_RANDOM)
	    roff(" sometimes");
	  else
	    roff(" often");
	}
      else if ((j == 2) && ((cp->cmove & CM_TREASURE) ==
			    (CM_60_RANDOM|CM_90_RANDOM)))
	roff (" often");
      roff(" carry");
      if (cp->cdefense & SPECIAL) /* it'll tell you who has better treasure -CFT */
        p = (j==1?"n exceptional object":" exceptional objects");
      else if (cp->cdefense & GOOD)
        p = (j==1?" good object":" good objects");
      else
        p = (j==1?"n object":" objects");
      if (j == 1)
        roff(" a");
      else if (j == 2)
	roff(" one or two");
      else
	{
	  (void) sprintf(temp, " up to %ld", j);
	  roff(temp);
	}
      if (rcmove & CM_CARRY_OBJ)
	{
	  roff(p);
	  if (rcmove & CM_CARRY_GOLD)
	    {
	      roff(" or treasure");
	      if (j > 1)
		roff("s");
	    }
	  roff(".");
	}
      else if (j != 1)
	roff(" treasures.");
      else
	roff(" treasure.");
    }

  /* We know about attacks it has used on us, and maybe the damage they do. */
  /* k is the total number of known attacks, used for punctuation */
  k = 0;
  for (j = 0; j < 4; j++)
    if (mp->r_attacks[j])
      k++;
  pu = cp->damage;
  /* j counts the attacks as printed, used for punctuation */
  j = 0;
  for (i = 0; *pu != 0 && i < 4; pu++, i++)
    {
      int att_type, att_how, d1, d2;

      /* don't print out unknown attacks */
      if (!mp->r_attacks[i])
	continue;

      att_type = monster_attacks[*pu].attack_type;
      att_how = monster_attacks[*pu].attack_desc;
      d1 = monster_attacks[*pu].attack_dice;
      d2 = monster_attacks[*pu].attack_sides;

      j++;
      if (j == 1)
	roff(" It can ");
      else if (j == k)
	roff(", and ");
      else
	roff(", ");

      if (att_how > 23)
	att_how = 0;
      roff(desc_amethod[att_how]);
      if (att_type != 1 || d1 > 0 && d2 > 0)
	{
	  roff(" to ");
	  if (att_type > 25)
	    att_type = 0;
	  roff(desc_atype[att_type]);
	  if (d1 && d2)
	    {
	      if (knowdamage(cp->level, mp->r_attacks[i], (int)d1*(int)d2) ||
	         ((cp->cdefense & UNIQUE) && /* learn uniques faster -CFT */
	          knowuniqdamage(cp->level, mp->r_attacks[i], (int)d1*(int)d2)))
		{
		  if (att_type == 19)	/* Loss of experience */
		    roff(" by");
		  else
		    roff(" with damage");
		  (void) sprintf(temp, " %dd%d", d1, d2 );
		  roff(temp);
		}
	    }
	}
    }
  if (j)
    roff(".");
  else if (k > 0 && mp->r_attacks[0] >= 10)
    roff(" It has no physical attacks.");
  else
    roff(" Nothing is known about its attack.");
  /* Always know the win creature. */
  if (cp->cmove & CM_WIN)
    roff(" Killing one of these wins the game!");
  roff("\n");
  prt("--pause--", roffpline, 0);
  if (wizard)
    *mp = save_mem;
  return inkey();
}

/* Print out strings, filling up lines as we go. */
static void roff(p)
register char *p;
{
  register char *q, *r;
  register int linesize;

  linesize=sizeof(roffbuf);
  if (linesize>80) linesize=80;

  while (*p)
    {
      *roffp = *p;
      if (*p == '\n' || roffp >= roffbuf + linesize - 1)
	{
	  q = roffp;
	  if (*p != '\n')
	    while (*q != ' ')
	      q--;
	  *q = 0;
	  prt(roffbuf, roffpline, 0);
	  roffpline++;
	  r = roffbuf;
	  while (q < roffp)
	    {
	      q++;
	      *r = *q;
	      r++;
	    }
	  roffp = r;
	}
      else
	roffp++;
      p++;
    }
}
