/*
 * misc2.c: misc code for maintaining the dungeon, printing player info 
 *
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "monster.h"

/* include before constant.h because param.h defines NULL incorrectly */
#ifndef USG
#include <sys/types.h>
#include <sys/param.h>
#endif

#include "constant.h"
#include "config.h"
#include "types.h"
#include "externs.h"

#include <ctype.h>

#ifdef USG
#ifndef ATARIST_MWC
#include <string.h>
#else
char               *index();
#endif
#else
#include <strings.h>
#endif

/* Lets do all prototypes correctly.... -CWS */
#ifndef NO_LINT_ARGS
#ifdef __STDC__
static void prt_lnum(const char *, int32, int, int);
static void prt_num(const char *, int, int, int);
static void prt_long(int32, int, int);
static void prt_int(int, int, int);
#else
static void prt_lnum();
static void prt_num();
static void prt_long();
static void prt_int();
#endif
static void gain_level();
#endif

static const char *stat_names[] = { "STR: ", "INT: ", "WIS: ",
					"DEX: ", "CON: ", "CHR: "};

#define BLANK_LENGTH	24
static char blank_string[] = "                        ";

extern int rating;

/* Places a particular trap at location y, x		-RAK-	 */
void 
place_trap(y, x, subval)
int y, x, subval;
{
    register int cur_pos;

    if (!in_bounds(y, x))
	return;	       /* abort! -CFT */
    if (cave[y][x].cptr >= MIN_MONIX)
	return;	       /* don't put rubble under monsters, it's annoying -CFT */

    if (cave[y][x].tptr != 0)
	if ((t_list[cave[y][x].tptr].tval == TV_STORE_DOOR) ||
	    (t_list[cave[y][x].tptr].tval == TV_UP_STAIR) ||
	    (t_list[cave[y][x].tptr].tval == TV_DOWN_STAIR) ||
	    ((t_list[cave[y][x].tptr].tval >= TV_MIN_WEAR) &&
	     (t_list[cave[y][x].tptr].tval <= TV_MAX_WEAR) &&
	     (t_list[cave[y][x].tptr].flags2 & TR_ARTIFACT)))
	    return;		   /* don't replace stairs, stores, artifacts */
	else
	    delete_object(y, x);
    cur_pos = popt();
    cave[y][x].tptr = cur_pos;
    invcopy(&t_list[cur_pos], OBJ_TRAP_LIST + subval);
}


/* Places rubble at location y, x			-RAK-	 */
void 
place_rubble(y, x)
int y, x;
{
    register int        cur_pos;
    register cave_type *cave_ptr;

    if (!in_bounds(y, x))
	return;			   /* abort! -CFT */
    if (cave[y][x].tptr != 0)
	if ((t_list[cave[y][x].tptr].tval == TV_STORE_DOOR) ||
	    (t_list[cave[y][x].tptr].tval == TV_UP_STAIR) ||
	    (t_list[cave[y][x].tptr].tval == TV_DOWN_STAIR) ||
	    ((t_list[cave[y][x].tptr].tval >= TV_MIN_WEAR) &&
	     (t_list[cave[y][x].tptr].tval <= TV_MAX_WEAR) &&
	     (t_list[cave[y][x].tptr].flags2 & TR_ARTIFACT)))
	    return;		   /* don't replace stairs, stores, artifacts */
	else
	    delete_object(y, x);
    cur_pos = popt();
    cave_ptr = &cave[y][x];
    cave_ptr->tptr = cur_pos;
    cave_ptr->fval = BLOCKED_FLOOR;
    invcopy(&t_list[cur_pos], OBJ_RUBBLE);
}

/* if killed a 'Creeping _xxx_ coins'... -CWS */
void
get_coin_type(c_ptr)
creature_type *c_ptr;
{
    if (!stricmp(c_ptr->name, "Creeping copper coins")) {
	coin_type = 2;
    }

    if (!stricmp(c_ptr->name, "Creeping silver coins")) {
	coin_type = 5;
    }

    if (!stricmp(c_ptr->name, "Creeping gold coins")) {
	coin_type = 10;
    }

    if (!stricmp(c_ptr->name, "Creeping mithril coins")) {
	coin_type = 16;
    }

    if (!stricmp(c_ptr->name, "Creeping adamantite coins")) {
	coin_type = 17;
    }
}

/* Places a treasure (Gold or Gems) at given row, column -RAK-	 */
void 
place_gold(y, x)
int y, x;
{
    register int        i, cur_pos;
    register inven_type *t_ptr;

    if (!in_bounds(y, x))
	return;			   /* abort! -CFT */
    if (cave[y][x].tptr != 0)
	if ((t_list[cave[y][x].tptr].tval == TV_STORE_DOOR) ||
	    (t_list[cave[y][x].tptr].tval == TV_UP_STAIR) ||
	    (t_list[cave[y][x].tptr].tval == TV_DOWN_STAIR) ||
	    ((t_list[cave[y][x].tptr].tval >= TV_MIN_WEAR) &&
	     (t_list[cave[y][x].tptr].tval <= TV_MAX_WEAR) &&
	     (t_list[cave[y][x].tptr].flags2 & TR_ARTIFACT)))
	    return;		   /* don't replace stairs, stores, artifacts */
	else
	    delete_object(y, x);
    cur_pos = popt();
    i = ((randint(object_level + 2) + 2) / 2) - 1;
    if (randint(OBJ_GREAT) == 1)
	i += randint(object_level + 1);
    if (i >= MAX_GOLD)
	i = MAX_GOLD - 1;
    if (coin_type) {			/* if killed a Creeping _xxx_ coins... */
	if (coin_type > MAX_GOLD - 1)
	    coin_type = 0;		/* safety check -CWS */
	i = coin_type;
    }
    cave[y][x].tptr = cur_pos;
    invcopy(&t_list[cur_pos], OBJ_GOLD_LIST + i);
    t_ptr = &t_list[cur_pos];
    t_ptr->cost += (8L * (long)randint((int)t_ptr->cost)) + randint(8);

/* average the values to make Creeping _xxx_ coins not give too great treasure drops */
    if (coin_type)
	t_ptr->cost = ((8L * (long)randint((int)object_list[OBJ_GOLD_LIST + i].cost))
		       + (t_ptr->cost)) >> 1;
					  
    if (cave[y][x].cptr == 1)
	msg_print("You feel something roll beneath your feet.");
}


/* Returns the array number of a random object		-RAK-	 */
int 
get_obj_num(level, good)
int level, good;
{
    register int i, j;

    do {
	if (level == 0)
	    i = randint(t_level[0]) - 1;
	else {
	    if (level >= MAX_OBJ_LEVEL)
		level = MAX_OBJ_LEVEL;
	    else if (randint(OBJ_GREAT) == 1) {
		level = level * MAX_OBJ_LEVEL / randint(MAX_OBJ_LEVEL) + 1;
		if (level > MAX_OBJ_LEVEL)
		    level = MAX_OBJ_LEVEL;
	    }
	/*
	 * This code has been added to make it slightly more likely to get
	 * the higher level objects.	Originally a uniform distribution
	 * over all objects less than or equal to the dungeon level.  This
	 * distribution makes a level n objects occur approx 2/n% of the time
	 * on level n, and 1/2n are 0th level. 
	 */

	    if (randint(2) == 1)
		i = randint(t_level[level]) - 1;
	    else {		   /* Choose three objects, pick the highest level. */
		i = randint(t_level[level]) - 1;
		j = randint(t_level[level]) - 1;
		if (i < j)
		    i = j;
		j = randint(t_level[level]) - 1;
		if (i < j)
		    i = j;
		j = object_list[sorted_objects[i]].level;
		if (j == 0)
		    i = randint(t_level[0]) - 1;
		else
		    i = randint(t_level[j] - t_level[j - 1]) - 1 + t_level[j - 1];
	    }
	}
    } while (((object_list[sorted_objects[i]].rare ?
	       (randint(object_list[sorted_objects[i]].rare) - 1) : 0) && !good)
	     || (object_list[sorted_objects[i]].rare == 255));
    return (i);
}



int 
special_place_object(y, x)
int y, x;
{
    register int cur_pos, tmp;
    char         str[100];
    int          done = 0;

    if (!in_bounds(y, x))
	return 0;		   /* abort! -CFT */
    if (cave[y][x].tptr != 0)
	if ((t_list[cave[y][x].tptr].tval == TV_STORE_DOOR) ||
	    (t_list[cave[y][x].tptr].tval == TV_UP_STAIR) ||
	    (t_list[cave[y][x].tptr].tval == TV_DOWN_STAIR) ||
	    ((t_list[cave[y][x].tptr].tval >= TV_MIN_WEAR) &&
	     (t_list[cave[y][x].tptr].tval <= TV_MAX_WEAR) &&
	     (t_list[cave[y][x].tptr].flags2 & TR_ARTIFACT)))
	    return 0;		   /* don't replace stairs, stores, artifacts */
	else
	    delete_object(y, x);
    str[0] = 0;

again:
    if (done > 20)
	return 0;
    tmp = randint(MAX_OBJECTS - (SPECIAL_OBJ - 1)) + (SPECIAL_OBJ - 1) - 1;
    switch (tmp) {
      case (SPECIAL_OBJ - 1):
	done++;
	if (randint(30) > 1)
	    goto again;
	if (NARYA)
	    goto again;
	if ((object_list[tmp].level - 40) > object_level)
	    goto again;
	if ((object_list[tmp].level > object_level) && (randint(50) > 1))
	    goto again;
	if ((wizard || peek))
	    sprintf(str, "Narya");
	else
	    good_item_flag = TRUE;
	NARYA = TRUE;
	break;
      case (SPECIAL_OBJ):
	done++;
	if (randint(35) > 1)
	    goto again;
	if (NENYA)
	    goto again;
	if ((object_list[tmp].level - 40) > object_level)
	    goto again;
	if ((object_list[tmp].level > object_level) && (randint(60) > 1))
	    goto again;
	if ((wizard || peek))
	    sprintf(str, "Nenya");
	else
	    good_item_flag = TRUE;
	NENYA = TRUE;
	break;
      case (SPECIAL_OBJ + 1):
	done++;
	if (randint(40) > 1)
	    goto again;
	if (VILYA)
	    goto again;
	if ((object_list[tmp].level - 40) > object_level)
	    goto again;
	if ((object_list[tmp].level > object_level) && (randint(70) > 1))
	    goto again;
	if ((wizard || peek))
	    sprintf(str, "Vilya");
	else
	    good_item_flag = TRUE;
	VILYA = TRUE;
	break;
      case (SPECIAL_OBJ + 2):
	done++;
	if (randint(60) > 1)
	    goto again;
	if (POWER)
	    goto again;
	if ((object_list[tmp].level - 40) > object_level)
	    goto again;
	if ((object_list[tmp].level > object_level) && (randint(100) > 1))
	    goto again;
	if ((wizard || peek))
	    sprintf(str, "Power (The One Ring)");
	else
	    good_item_flag = TRUE;
	POWER = TRUE;
	break;
      case (SPECIAL_OBJ + 3):
	done++;
	if (PHIAL)
	    goto again;
	if ((object_list[tmp].level - 40) > object_level)
	    goto again;
	if ((object_list[tmp].level > object_level) && (randint(30) > 1))
	    goto again;
	if ((wizard || peek))
	    sprintf(str, "Phial of Galadriel");
	else
	    good_item_flag = TRUE;
	PHIAL = TRUE;
	break;
      case (SPECIAL_OBJ + 4):
	done++;
	if (randint(10) > 1)
	    goto again;
	if (INGWE)
	    goto again;
	if ((object_list[tmp].level - 40) > object_level)
	    goto again;
	if ((object_list[tmp].level > object_level) && (randint(50) > 1))
	    goto again;
	if ((wizard || peek))
	    sprintf(str, "Amulet of Ingwe");
	else
	    good_item_flag = TRUE;
	INGWE = TRUE;
	break;
      case (SPECIAL_OBJ + 5):
	done++;
	if (randint(6) > 1)
	    goto again;
	if (CARLAMMAS)
	    goto again;
	if ((object_list[tmp].level - 40) > object_level)
	    goto again;
	if ((object_list[tmp].level > object_level) && (randint(35) > 1))
	    goto again;
	if ((wizard || peek))
	    sprintf(str, "Amulet of Carlammas");
	else
	    good_item_flag = TRUE;
	CARLAMMAS = TRUE;
	break;
      case (SPECIAL_OBJ + 6):
	done++;
	if (randint(8) > 1)
	    goto again;
	if (ELENDIL)
	    goto again;
	if ((object_list[tmp].level - 40) > object_level)
	    goto again;
	if ((object_list[tmp].level > object_level) && (randint(30) > 1))
	    goto again;
	if ((wizard || peek))
	    sprintf(str, "Star of Elendil");
	else
	    good_item_flag = TRUE;
	ELENDIL = TRUE;
	break;
      case (SPECIAL_OBJ + 7):
	done++;
	if (randint(18) > 1)
	    goto again;
	if (THRAIN)
	    goto again;
	if ((object_list[tmp].level - 40) > object_level)
	    goto again;
	if ((object_list[tmp].level > object_level) && (randint(60) > 1))
	    goto again;
	if ((wizard || peek))
	    sprintf(str, "Arkenstone of Thrain");
	else
	    good_item_flag = TRUE;
	THRAIN = TRUE;
	break;
      case (SPECIAL_OBJ + 8):
	done++;
	if (randint(25) > 1)
	    goto again;
	if (TULKAS)
	    goto again;
	if ((object_list[tmp].level - 40) > object_level)
	    goto again;
	if ((object_list[tmp].level > object_level) && (randint(65) > 1))
	    goto again;
	if ((wizard || peek))
	    sprintf(str, "Ring of Tulkas");
	else
	    good_item_flag = TRUE;
	TULKAS = TRUE;
	break;
      case (SPECIAL_OBJ + 9):
	done++;
	if (randint(25) > 1)
	    goto again;
	if (NECKLACE)
	    goto again;
	if ((object_list[tmp].level - 40) > object_level)
	    goto again;
	if ((object_list[tmp].level > object_level) && (randint(60) > 1))
	    goto again;
	if ((wizard || peek))
	    sprintf(str, "Necklace of the Dwarves");
	else
	    good_item_flag = TRUE;
	NECKLACE = TRUE;
	break;
      case (SPECIAL_OBJ + 10):
	done++;
	if (randint(20) > 1)
	    goto again;
	if (BARAHIR)
	    goto again;
	if ((object_list[tmp].level - 40) > object_level)
	    goto again;
	if ((object_list[tmp].level > object_level) && (randint(50) > 1))
	    goto again;
	if ((wizard || peek))
	    sprintf(str, "Ring of Barahir");
	else
	    good_item_flag = TRUE;
	BARAHIR = TRUE;
	break;
    }
    if (strlen(str) > 0 && (wizard || peek))
	msg_print(str);
    cur_pos = popt();
    cave[y][x].tptr = cur_pos;
    invcopy(&t_list[cur_pos], tmp);
    t_list[cur_pos].timeout = 0;
    t_list[cur_pos].ident |= ID_NOSHOW_TYPE; /* don't show (+x of yyy) for these */
    if (object_list[tmp].level > object_level) {
	rating += 2 * (object_list[sorted_objects[tmp]].level - object_level);
    }
    if (cave[y][x].cptr == 1)
	msg_print("You feel something roll beneath your feet.");
    return (-1);
}

/* Places an object at given row, column co-ordinate    -RAK-   */
void
place_object(y, x)
int y, x;
{
    register int cur_pos, tmp;

    if (!in_bounds(y,x)) return; /* abort! -CFT */
    if (cave[y][x].tptr != 0)
	if ((t_list[cave[y][x].tptr].tval == TV_STORE_DOOR) ||
	    (t_list[cave[y][x].tptr].tval == TV_UP_STAIR) ||
	    (t_list[cave[y][x].tptr].tval == TV_DOWN_STAIR) ||
	    ((t_list[cave[y][x].tptr].tval >= TV_MIN_WEAR) &&
	     (t_list[cave[y][x].tptr].tval <= TV_MAX_WEAR) &&
	     (t_list[cave[y][x].tptr].flags2 & TR_ARTIFACT)))
	    return; /* don't replace stairs, stores, artifacts */
	else
	    delete_object(y,x);
    
    if (randint(MAX_OBJECTS)>SPECIAL_OBJ && randint(10)==1)
	if (special_place_object(y,x)==(-1))
	    return;
    cur_pos = popt();
    cave[y][x].tptr = cur_pos;

    do {	   /* don't generate another chest if opening_chest is true -CWS */
	tmp = get_obj_num(dun_level, FALSE);
    } while (opening_chest && (object_list[sorted_objects[tmp]].tval == TV_CHEST));
	
    invcopy(&t_list[cur_pos], sorted_objects[tmp]);
    magic_treasure(cur_pos, dun_level, FALSE, 0);
    if (object_list[sorted_objects[tmp]].level > dun_level)
	rating += object_list[sorted_objects[tmp]].level - dun_level;
    if (peek) {
	if (object_list[sorted_objects[tmp]].level > dun_level) {
	    char buf[200];
	    int8u temp;
	    
	    temp=t_list[cur_pos].ident;
	    t_list[cur_pos].ident |= ID_STOREBOUGHT;
	    objdes(buf, &t_list[cur_pos], TRUE);
	    t_list[cur_pos].ident = temp;
	    msg_print(buf);
	}
    }
    if (cave[y][x].cptr == 1)
	msg_print ("You feel something roll beneath your feet.");
}

/* Places a GOOD-object at given row, column co-ordinate ~Ludwig */
void 
place_special(y, x, good)
int    y, x;
int32u good;
{
    register int cur_pos, tmp;
    int          tv, is_good = FALSE;

    if (!in_bounds(y, x))
	return;			   /* abort! -CFT */
    if (cave[y][x].tptr != 0)
	if ((t_list[cave[y][x].tptr].tval == TV_STORE_DOOR) ||
	    (t_list[cave[y][x].tptr].tval == TV_UP_STAIR) ||
	    (t_list[cave[y][x].tptr].tval == TV_DOWN_STAIR) ||
	    ((t_list[cave[y][x].tptr].tval >= TV_MIN_WEAR) &&
	     (t_list[cave[y][x].tptr].tval <= TV_MAX_WEAR) &&
	     (t_list[cave[y][x].tptr].flags2 & TR_ARTIFACT)))
	    return;		   /* don't replace stairs, stores, artifacts */
	else
	    delete_object(y, x);

    if (randint(10) == 1)
	if (special_place_object(y, x) == (-1))
	    return;
    cur_pos = popt();
    cave[y][x].tptr = cur_pos;
    do {
	tmp = get_obj_num((object_level + 10), TRUE);
	tv = object_list[sorted_objects[tmp]].tval;
	if ((tv == TV_HELM) || (tv == TV_SHIELD) ||
	    (tv == TV_CLOAK) || (tv == TV_HAFTED) || (tv == TV_POLEARM) ||
	    (tv == TV_BOW) || (tv == TV_BOLT) || (tv == TV_ARROW) ||
	    (tv == TV_BOOTS) || (tv == TV_GLOVES))
	    is_good = TRUE;
	if ((tv == TV_SWORD) &&
	    strncmp("& Broken", object_list[sorted_objects[tmp]].name, 8))
	    is_good = TRUE;	   /* broken swords/daggers are NOT good!
				    * -CFT */
	if ((tv == TV_HARD_ARMOR) &&
	    strncmp("Rusty", object_list[sorted_objects[tmp]].name, 5))
	    is_good = TRUE;	   /* rusty chainmail is NOT good! -CFT */
	if ((tv == TV_SOFT_ARMOR) &&
	 stricmp("some filthy rags", object_list[sorted_objects[tmp]].name))
	    is_good = TRUE;	   /* nor are rags! -CFT */
	if ((tv == TV_MAGIC_BOOK) &&	/* if book, good must be one of the
					 * deeper, special must be Raal's */
	    (object_list[sorted_objects[tmp]].subval > ((good & SPECIAL) ? 71 : 67)))
	    is_good = TRUE;
	if ((tv == TV_PRAYER_BOOK) &&	/* if book, good must be one of the
					 * deeper, special must be Wrath of
					 * God */
	    (object_list[sorted_objects[tmp]].subval > ((good & SPECIAL) ? 71 : 67)))
	    is_good = TRUE;
    } while (!is_good);
    invcopy(&t_list[cur_pos], sorted_objects[tmp]);
    magic_treasure(cur_pos, object_level, (good & SPECIAL) ? 666 : 1, 0);
    if (peek) {
	if (object_list[sorted_objects[tmp]].level > object_level) {
	    char                buf[200];
	    int8u               t;

	    t = t_list[cur_pos].ident;
	    t_list[cur_pos].ident |= ID_STOREBOUGHT;
	    objdes(buf, &t_list[cur_pos], TRUE);
	    t_list[cur_pos].ident = t;
	    msg_print(buf);
	}
    }
    if (cave[y][x].cptr == 1)
	msg_print("You feel something roll beneath your feet.");
}


/* Allocates an object for tunnels and rooms		-RAK-	 */
void 
alloc_object(alloc_set, typ, num)
int (*alloc_set) ();
int typ, num;
{
    register int i, j, k;

    for (k = 0; k < num; k++) {
	do {
	    i = randint(cur_height) - 1;
	    j = randint(cur_width) - 1;
	}
    /*
     * don't put an object beneath the player, this could cause problems if
     * player is standing under rubble, or on a trap 
     */
	while ((!(*alloc_set) (cave[i][j].fval)) ||
	       (cave[i][j].tptr != 0) || (i == char_row && j == char_col));
	if (typ < 4) {		   /* typ == 2 not used, used to be visible
				    * traps */
	    if (typ == 1)
		place_trap(i, j, randint(MAX_TRAP) - 1);	/* typ == 1 */
	    else
		place_rubble(i, j);/* typ == 3 */
	} else {
	    object_level = dun_level;
	    if (typ == 4)
		place_gold(i, j);  /* typ == 4 */
	    else
		place_object(i, j);/* typ == 5 */
	}
    }
}


/* Creates objects nearby the coordinates given		-RAK-	 */
void 
random_object(y, x, num)
int y, x, num;
{
    register int        i, j, k;
    register cave_type *cave_ptr;

    do {
	i = 0;
	do {
	    do {
		j = y - 3 + randint(5);
		k = x - 4 + randint(7);
	    } while (!in_bounds(j, k));
	    cave_ptr = &cave[j][k];
	    if ((cave_ptr->fval <= MAX_CAVE_FLOOR) && (cave_ptr->tptr == 0)) {
		object_level = dun_level;
		if (randint(100) < 75)
		    place_object(j, k);
		else
		    place_gold(j, k);
		i = 9;
	    }
	    i++;
	}
	while (i <= 10);
	num--;
    }
    while (num != 0);
}

void 
special_random_object(y, x, num)
int y, x, num;
{
    register int        i, j, k;
    register cave_type *cave_ptr;

    object_level = dun_level;
    do {
	i = 0;
	do {
	    j = y - 3 + randint(5);
	    k = x - 4 + randint(7);
	    cave_ptr = &cave[j][k];
	    if ((cave_ptr->fval <= MAX_CAVE_FLOOR) && (cave_ptr->tptr == 0)) {
		if (randint(5) == 1) {
		    if (!special_place_object(j, k))
			place_special(j, k, SPECIAL);
		} else {
		    place_special(j, k, SPECIAL);
		}
		i = 9;
	    }
	    i++;
	}
	while (i <= 10);
	num--;
    }
    while (num != 0);
}

/* Converts stat num into string			-RAK-	 */
void 
cnv_stat(my_stat, out_val)
int   my_stat;
char *out_val;
{
    register int16u stat = my_stat;
    register int    part1, part2;

    if (stat > 18) {
	part1 = 18;
	part2 = stat - 18;
	if (part2 >= 220)
	    (void)sprintf(out_val, "%2d/*** ", part1);
	else if (part2 >= 100)
	    (void)sprintf(out_val, "%2d/%03d ", part1, part2);
	else
	    (void)sprintf(out_val, " %2d/%02d ", part1, part2);
    } else
	(void)sprintf(out_val, "%6d ", stat);
}


/* Print character stat in given row, column		-RAK-	 */
void 
prt_stat(stat)
int stat;
{
    vtype out_val1;

    cnv_stat(py.stats.use_stat[stat], out_val1);
    put_buffer(stat_names[stat], 5 + stat, STAT_COLUMN);
    put_buffer(out_val1, 5 + stat, STAT_COLUMN + 6);
}


/* Print character info in given row, column		-RAK-	 */
/* the longest title is 13 characters, so only pad to 13 */
void 
prt_field(info, row, column)
const char *info;
int         row, column;
{
    put_buffer(&blank_string[BLANK_LENGTH - 13], row, column);
    put_buffer(info, row, column);
}

/* Print long number with header at given row, column */
static void 
prt_lnum(header, num, row, column)
const char *header;
int32       num;
int         row, column;
{
    vtype out_val;

    (void)sprintf(out_val, "%s%9ld", header, (long)num);
    put_buffer(out_val, row, column);
}

/* Print number with header at given row, column	-RAK-	 */
static void 
prt_num(header, num, row, column)
const char *header;
int         num, row, column;
{
    vtype out_val;

    (void)sprintf(out_val, "%s   %6d", header, num);
    put_buffer(out_val, row, column);
}

/* Print long number at given row, column */
static void 
prt_long(num, row, column)
int32 num;
int   row, column;
{
    vtype out_val;

    (void)sprintf(out_val, "%9ld", (long)num);
    put_buffer(out_val, row, column);
}

/* Print number at given row, column	-RAK-	 */
static void 
prt_int(num, row, column)
    int num, row, column;
{
    vtype out_val;

    (void)sprintf(out_val, "%6d", num);
    put_buffer(out_val, row, column);
}


/* Adjustment for wisdom/intelligence				-JWT-	 */
int 
stat_adj(stat)
    int stat;
{
    register int value;

    value = py.stats.use_stat[stat];
    if (value > 228)
	return (20);
    else if (value > 218)
	return (18);
    else if (value > 198)
	return (16);
    else if (value > 188)
	return (15);
    else if (value > 178)
	return (14);
    else if (value > 168)
	return (13);
    else if (value > 158)
	return (12);
    else if (value > 148)
	return (11);
    else if (value > 138)
	return (10);
    else if (value > 128)
	return (9);
    else if (value > 118)
	return (8);
    else if (value == 118)
	return (7);
    else if (value > 107)
	return (6);
    else if (value > 87)
	return (5);
    else if (value > 67)
	return (4);
    else if (value > 17)
	return (3);
    else if (value > 14)
	return (2);
    else if (value > 7)
	return (1);
    else
	return (0);
}


/* Adjustment for charisma				-RAK-	 */
/* Percent decrease or increase in price of goods		 */
int 
chr_adj()
{
    register int charisma;

    charisma = py.stats.use_stat[A_CHR];
    if (charisma > 217)
	return (80);
    else if (charisma > 187)
	return (86);
    else if (charisma > 147)
	return (88);
    else if (charisma > 117)
	return (90);
    else if (charisma > 107)
	return (92);
    else if (charisma > 87)
	return (94);
    else if (charisma > 67)
	return (96);
    else if (charisma > 18)
	return (98);
    else
	switch (charisma) {
	  case 18:
	    return (100);
	  case 17:
	    return (101);
	  case 16:
	    return (102);
	  case 15:
	    return (103);
	  case 14:
	    return (104);
	  case 13:
	    return (106);
	  case 12:
	    return (108);
	  case 11:
	    return (110);
	  case 10:
	    return (112);
	  case 9:
	    return (114);
	  case 8:
	    return (116);
	  case 7:
	    return (118);
	  case 6:
	    return (120);
	  case 5:
	    return (122);
	  case 4:
	    return (125);
	  case 3:
	    return (130);
	  default:
	    return (140);
	}
}


/* Returns a character's adjustment to hit points	 -JWT-	 */
int 
con_adj()
{
    register int con;

    con = py.stats.use_stat[A_CON];
    if (con < 7)
	return (con - 7);
    else if (con < 17)
	return (0);
    else if (con == 17)
	return (1);
    else if (con < 94)
	return (2);
    else if (con < 117)
	return (3);
    else if (con < 119)
	return (4);
    else if (con < 128)
	return (5);
    else if (con < 138)
	return (6);
    else if (con < 158)
	return (7);
    else if (con < 168)
	return (8);
    else if (con < 178)
	return (9);
    else if (con < 188)
	return (10);
    else if (con < 198)
	return (11);
    else if (con < 208)
	return (12);
    else if (con < 228)
	return (13);
    else
	return (14);
}


const char *
title_string()
{
    const char *p;

    if (py.misc.lev < 1)
	p = "Babe in arms";
    else if (py.misc.lev <= MAX_PLAYER_LEVEL)
	p = player_title[py.misc.pclass][py.misc.lev - 1];
    else if (py.misc.male)
	p = "**KING**";
    else
	p = "**QUEEN**";
    return p;
}


/* Prints title of character				-RAK-	 */
void 
prt_title()
{
    prt_field(title_string(), 3, STAT_COLUMN);
}


/* Prints level						-RAK-	 */
void 
prt_level()
{
    prt_int((int)py.misc.lev, 12, STAT_COLUMN + 6);
}


/* Prints players current mana points.		 -RAK-	 */
void 
prt_cmana()
{
    prt_int(py.misc.cmana, 14, STAT_COLUMN + 6);
}


/* Prints Max hit points				-RAK-	 */
void 
prt_mhp()
{
    prt_int(py.misc.mhp, 15, STAT_COLUMN + 6);
}


/* Prints players current hit points			-RAK-	 */
void 
prt_chp()
{
    prt_int(py.misc.chp, 16, STAT_COLUMN + 6);
}


/* prints current AC					-RAK-	 */
void 
prt_pac()
{
    prt_int(py.misc.dis_ac, 18, STAT_COLUMN + 6);
}


/* Prints current gold					-RAK-	 */
void 
prt_gold()
{
    prt_long(py.misc.au, 19, STAT_COLUMN + 3);
}


/* Prints depth in stat area				-RAK-	 */
void 
prt_depth()
{
    vtype               depths;
    register int        depth;

    depth = dun_level * 50;
    if (depth == 0)
	(void)strcpy(depths, "Town    ");
    else
	(void)sprintf(depths, "%d ft", depth);
    prt(depths, 23, 70);
}


/* Prints status of hunger				-RAK-	 */
void 
prt_hunger()
{
    if (PY_WEAK & py.flags.status)
	put_buffer("Weak  ", 23, 0);
    else if (PY_HUNGRY & py.flags.status)
	put_buffer("Hungry", 23, 0);
    else
	put_buffer("      ", 23, 0);
}


/* Prints Blind status					-RAK-	 */
void 
prt_blind()
{
    if (PY_BLIND & py.flags.status)
	put_buffer("Blind", 23, 7);
    else
	put_buffer("     ", 23, 7);
}


/* Prints Confusion status				-RAK-	 */
void 
prt_confused()
{
    if (PY_CONFUSED & py.flags.status)
	put_buffer("Confused", 23, 13);
    else
	put_buffer("        ", 23, 13);
}


/* Prints Fear status					-RAK-	 */
void 
prt_afraid()
{
    if (PY_FEAR & py.flags.status)
	put_buffer("Afraid", 23, 22);
    else
	put_buffer("      ", 23, 22);
}


/* Prints Poisoned status				-RAK-	 */
void 
prt_poisoned()
{
    if (PY_POISONED & py.flags.status)
	put_buffer("Poisoned", 23, 29);
    else
	put_buffer("        ", 23, 29);
}


/* Prints Searching, Resting, Paralysis, or 'count' status	-RAK-	 */
void 
prt_state()
{
    char tmp[16];

    py.flags.status &= ~PY_REPEAT;
    if (py.flags.paralysis > 1)
	put_buffer("Paralysed ", 23, 38);
    else if (PY_REST & py.flags.status) {
	if (py.flags.rest > 0)
	    (void)sprintf(tmp, "Rest %-5d", py.flags.rest);
	else if (py.flags.rest == -1)
	    (void)sprintf(tmp, "Rest *****");
	else if (py.flags.rest == -2)
	    (void)sprintf(tmp, "Rest &&&&&");
	put_buffer(tmp, 23, 38);
    } else if (command_count > 0) {
	(void)sprintf(tmp, "Repeat %-3d", command_count);
	py.flags.status |= PY_REPEAT;
	put_buffer(tmp, 23, 38);
	if (PY_SEARCH & py.flags.status)
	    put_buffer("Search    ", 23, 38);
    } else if (PY_SEARCH & py.flags.status)
	put_buffer("Searching ", 23, 38);
    else			   /* "repeat 999" is 10 characters */
	put_buffer("          ", 23, 38);
}


/* Prints the speed of a character.			-CJS- */
void 
prt_speed()
{
    register int i;

    i = py.flags.speed;
    if (PY_SEARCH & py.flags.status)	/* Search mode. */
	i--;
    if (i > 2)
	put_buffer("Extremely Slow", 23, 49);
    else if (i == 2)
	put_buffer("Very Slow     ", 23, 49);
    else if (i == 1)
	put_buffer("Slow          ", 23, 49);
    else if (i == 0)
	put_buffer("              ", 23, 49);
    else if (i == -1)
	put_buffer("Fast          ", 23, 49);
    else if (i == -2)
	put_buffer("Very Fast     ", 23, 49);
    else if (i == -3)
	put_buffer("Extremely Fast", 23, 49);
    else if (i == -4)
	put_buffer("Deadly Speed  ", 23, 49);
    else
	put_buffer("Light Speed   ", 23, 49);
}


void 
prt_study()
{
    py.flags.status &= ~PY_STUDY;
    if (py.flags.new_spells != 0)
	put_buffer("Study", 23, 64);
    else
	put_buffer("     ", 23, 64);
}

void 
cut_player(c)
int c;
{
    py.flags.cut += c;
    c = py.flags.cut;
    if (c > 5000)
	msg_print("You have been given a mortal wound.");
    else if (c > 900)
	msg_print("You have been given a deep gash.");
    else if (c > 200)
	msg_print("You have been given a severe cut.");
    else if (c > 100)
	msg_print("You have been given a nasty cut.");
    else if (c > 50)
	msg_print("You have been given a bad cut.");
    else if (c > 10)
	msg_print("You have been given a light cut.");
    else if (c > 0)
	msg_print("You have been given a graze.");
}

void 
prt_cut()
{
    int c = py.flags.cut;

    if (c > 900)
	put_buffer("Mortal wound", 21, 0);
    else if (c > 300)
	put_buffer("Deep gash   ", 21, 0);
    else if (c > 200)
	put_buffer("Severe cut  ", 21, 0);
    else if (c > 45)
	put_buffer("Nasty cut   ", 21, 0);
    else if (c > 15)
	put_buffer("Bad cut     ", 21, 0);
    else if (c > 5)
	put_buffer("Light cut   ", 21, 0);
    else if (c > 0)
	put_buffer("Graze       ", 21, 0);
    else
	put_buffer("            ", 21, 0);
}

void 
stun_player(s)
int s;
{
    int t;

    if (!py.flags.sound_resist) {
	t = py.flags.stun;
	py.flags.stun += s;
	s = py.flags.stun;
	if (s > 100) {
	    msg_print("You have been knocked out.");
	    if (t == 0) {
		py.misc.ptohit -= 20;
		py.misc.ptodam -= 20;
		py.misc.dis_th -= 20;
		py.misc.dis_td -= 20;
	    } else if (t <= 50) {
		py.misc.ptohit -= 15;
		py.misc.ptodam -= 15;
		py.misc.dis_th -= 15;
		py.misc.dis_td -= 15;
	    }
	} else if (s > 50) {
	    msg_print("You've been heavily stunned.");
	    if (t == 0) {
		py.misc.ptohit -= 20;
		py.misc.ptodam -= 20;
		py.misc.dis_th -=20;
		py.misc.dis_td -=20;
	    } else if (t <= 50) {
		py.misc.ptohit -= 15;
		py.misc.ptodam -= 15;
                py.misc.dis_th -= 15;
                py.misc.dis_td -= 15;
	    }
	} else if (s > 0) {
	    msg_print("You've been stunned.");
	    if (t == 0) {
		py.misc.ptohit -= 5;
		py.misc.ptodam -= 5;
                py.misc.dis_th -= 5;
                py.misc.dis_td -= 5;
	    }
	}
    }
}

void 
prt_stun()
{
    int s = py.flags.stun;

    if (!py.flags.sound_resist) {
	if (s > 100)
	    put_buffer("Knocked out ", 22, 0);
	else if (s > 50)
	    put_buffer("Heavy stun  ", 22, 0);
	else if (s > 0)
	    put_buffer("Stun        ", 22, 0);
	else
	    put_buffer("            ", 22, 0);
    }
}

/* Prints winner status on display			-RAK-	 */
void 
prt_winner()
{
    if (wizard)
	put_buffer("Wizard", 20, 0);
    else if (total_winner)
	put_buffer("Winner", 20, 0);
    else
	put_buffer("       ", 20, 0);
}


int16u 
modify_stat(stat, amount)
int stat;
int amount;
{
    register int    loop, i;
    register int16u tmp_stat;

    tmp_stat = py.stats.cur_stat[stat];
    loop = (amount < 0 ? -amount : amount);
    for (i = 0; i < loop; i++) {
	if (amount > 0) {
	    if (tmp_stat < 18)
		tmp_stat++;
	    else
		tmp_stat += 10;
	} else {
	    if (tmp_stat > 27)
		tmp_stat -= 10;
	    else if (tmp_stat > 18)
		tmp_stat = 18;
	    else if (tmp_stat > 3)
		tmp_stat--;
	}
    }
    return tmp_stat;
}


/* Set the value of the stat which is actually used.	 -CJS- */
void 
set_use_stat(stat)
int stat;
{
    py.stats.use_stat[stat] = modify_stat(stat, py.stats.mod_stat[stat]);

    if (stat == A_STR) {
	py.flags.status |= PY_STR_WGT;
	calc_bonuses();
    } else if (stat == A_DEX)
	calc_bonuses();
    else if (stat == A_INT && class[py.misc.pclass].spell == MAGE) {
	calc_spells(A_INT);
	calc_mana(A_INT);
    } else if (stat == A_WIS && class[py.misc.pclass].spell == PRIEST) {
	calc_spells(A_WIS);
	calc_mana(A_WIS);
    } else if (stat == A_CON)
	calc_hitpoints();
}


/* Increases a stat by one randomized level		-RAK-	 */
int 
inc_stat(stat)
register int stat;
{
    register int tmp_stat, gain;

    res_stat(stat);
    tmp_stat = py.stats.cur_stat[stat];
    if (tmp_stat < 118) {
	if (tmp_stat < 18) {	   
	    gain = randint(2);		/* let's be able to monitor the increase -CWS */
	    tmp_stat += gain;
	} else if (tmp_stat < 116) {
	/* stat increases by 1/6 to 1/3 of difference from max */
	    gain = ((118 - tmp_stat) / 2 + 3) >> 1;
	    tmp_stat += randint(gain) + gain / 2;
	    if (tmp_stat > 117)
		tmp_stat = 117;
	} else
	    tmp_stat++;

	py.stats.cur_stat[stat] = tmp_stat;
	if (tmp_stat > py.stats.max_stat[stat])
	    py.stats.max_stat[stat] = tmp_stat;
	set_use_stat(stat);
	prt_stat(stat);
	return TRUE;
    } else
	return FALSE;
}


/* Decreases a stat by one randomized level		-RAK-	 */
int 
dec_stat(stat)
register int stat;
{
    register int tmp_stat, loss;

    tmp_stat = py.stats.cur_stat[stat];
    if (tmp_stat > 3) {
	if (tmp_stat < 19)
	    tmp_stat--;
	else if (tmp_stat < 117) {
	    loss = (((118 - tmp_stat) >> 1) + 1) >> 1;
	    tmp_stat += -randint(loss) - loss;
	    if (tmp_stat < 18)
		tmp_stat = 18;
	} else
	    tmp_stat--;

	py.stats.cur_stat[stat] = tmp_stat;
	set_use_stat(stat);
	prt_stat(stat);
	return TRUE;
    } else
	return FALSE;
}


/* Restore a stat.  Return TRUE only if this actually makes a difference. */
int 
res_stat(stat)
int stat;
{
    register int i;

    i = py.stats.max_stat[stat] - py.stats.cur_stat[stat];
    if (i) {
	py.stats.cur_stat[stat] += i;
	set_use_stat(stat);
	prt_stat(stat);
	return TRUE;
    }
    return FALSE;
}

/*
 * Boost a stat artificially (by wearing something). If the display argument
 * is TRUE, then increase is shown on the screen. 
 */
void 
bst_stat(stat, amount)
int stat, amount;
{
    py.stats.mod_stat[stat] += amount;

    set_use_stat(stat);
/* can not call prt_stat() here, may be in store, may be in inven_command */
    py.flags.status |= (PY_STR << stat);
}


/* Returns a character's adjustment to hit.		 -JWT-	 */
int 
tohit_adj()
{
    register int total, stat;

    stat = py.stats.use_stat[A_DEX];
    if      (stat <   4)  total = -3;
    else if (stat <   6)  total = -2;
    else if (stat <   8)  total = -1;
    else if (stat <  16)  total =  0;
    else if (stat <  17)  total =  1;
    else if (stat <  18)  total =  2;
    else if (stat <  69)  total =  3;
    else if (stat < 108)  total =  4; /* 18/51 to 18/89 -CFT */
    else if (stat < 118)  total =  5; /* 18/90 to 18/99 -CFT */
    else if (stat < 128)  total =  6; /* 18/100 to 18/109 -CFT */
    else if (stat < 138)  total =  7;
    else if (stat < 148)  total =  8;
    else if (stat < 158)  total =  9;
    else if (stat < 168)  total = 10;
    else if (stat < 178)  total = 11;
    else if (stat < 188)  total = 12;
    else if (stat < 198)  total = 13;
    else if (stat < 218)  total = 14;
    else if (stat < 228)  total = 15;
    else total = 17;
    stat = py.stats.use_stat[A_STR];
    if      (stat <   4)  total -= 3;
    else if (stat <   5)  total -= 2;
    else if (stat <   7)  total -= 1;
    else if (stat <  18)  total -= 0;
    else if (stat <  88)  total += 1; /* 18 to 18/69 -CFT */
    else if (stat <  98)  total += 2; /* 18/70 to 18/79 -CFT */
    else if (stat < 108)  total += 3; /* 18/80 to 18/89 -CFT */
    else if (stat < 118)  total += 4; /* 18/90 to 18/99 -CFT */
    else if (stat < 128)  total += 5; /* 18/100 to 18/109 -CFT */
    else if (stat < 138)  total += 6;
    else if (stat < 148)  total += 7;
    else if (stat < 158)  total += 8;
    else if (stat < 168)  total += 9;
    else if (stat < 178)  total +=10;
    else if (stat < 188)  total +=11;
    else if (stat < 198)  total +=12;
    else if (stat < 218)  total +=13;
    else if (stat < 228)  total +=14;
    else total += 16;
    return (total);
}


/* Returns a character's adjustment to armor class	 -JWT-	 */
int 
toac_adj()
{
    register int stat;

    stat = py.stats.use_stat[A_DEX];
    if      (stat <   4)  return(-4);
    else if (stat ==  4)  return(-3);
    else if (stat ==  5)  return(-2);
    else if (stat ==  6)  return(-1);
    else if (stat <  15)  return( 0);
    else if (stat <  18)  return( 1);
    else if (stat <  58)  return( 2); /* 18 to 18/49 -CFT */
    else if (stat <  98)  return( 3); /* 18/50 to 18/79 -CFT */
    else if (stat < 108)  return( 4); /* 18/80 to 18/89 -CFT */
    else if (stat < 118)  return( 5); /* 18/90 to /99 -CFT */
    else if (stat < 128)  return( 6); /* /100 to /109 -CFT */
    else if (stat < 138)  return( 7);
    else if (stat < 148)  return( 8);
    else if (stat < 158)  return( 9);
    else if (stat < 168)  return(10);
    else if (stat < 178)  return(11);
    else if (stat < 188)  return(12);
    else if (stat < 198)  return(13);
    else if (stat < 218)  return(14);
    else if (stat < 228)  return(15);
    else                  return(17);
}


/* Returns a character's adjustment to disarm		 -RAK-	 */
int 
todis_adj()
{
    register int stat;

    stat = py.stats.use_stat[A_DEX];
    if      (stat <=  3)  return(-8);
    else if (stat ==  4)  return(-6);
    else if (stat ==  5)  return(-4);
    else if (stat ==  6)  return(-2);
    else if (stat ==  7)  return(-1);
    else if (stat <  13)  return( 0);
    else if (stat <  16)  return( 1);
    else if (stat <  18)  return( 2);
    else if (stat <  58)  return( 4); /* 18 to 18/49 -CFT */
    else if (stat <  88)  return( 5); /* 18/50 to 18/69 -CFT */
    else if (stat < 108)  return( 6); /* 18/70 to 18/89 -CFT */
    else if (stat < 118)  return( 7); /* 18/90 to 18/99 -CFT */
    else                  return( 8); /* 18/100 and over -CFT */
}


/* Returns a character's adjustment to damage		 -JWT-	 */
int 
todam_adj()
{
    register int stat;

    stat = py.stats.use_stat[A_STR];
    if      (stat <   4)  return(-2);
    else if (stat <   5)  return(-1);
    else if (stat <  16)  return( 0);
    else if (stat <  17)  return( 1);
    else if (stat <  18)  return( 2);
    else if (stat <  88)  return( 3); /* 18 to 18/69 -CFT */
    else if (stat <  98)  return( 4); /* 18/70 to 18/79 -CFT */
    else if (stat < 108)  return( 5); /* 18/80 to 18/89 -CFT */
    else if (stat < 118)  return( 5); /* 18/90 to 18/99 -CFT */
    else if (stat < 128)  return( 6); /* 18/100 to /109 -CFT */
    else if (stat < 138)  return( 7);
    else if (stat < 148)  return( 8);
    else if (stat < 158)  return( 9);
    else if (stat < 168)  return(10);
    else if (stat < 178)  return(11);
    else if (stat < 188)  return(12);
    else if (stat < 198)  return(13);
    else if (stat < 218)  return(14);
    else if (stat < 228)  return(16);
    else                  return(20);
}


/* Prints character-screen info				-RAK-	 */
void 
prt_stat_block()
{
    register int32u       status;
    register struct misc *m_ptr;
    register int          i;

    m_ptr = &py.misc;
    prt_field(race[py.misc.prace].trace, 1, STAT_COLUMN);
    prt_field(class[py.misc.pclass].title, 2, STAT_COLUMN);
    prt_field(title_string(), 3, STAT_COLUMN);
    for (i = 0; i < 6; i++)
	prt_stat(i);
    prt_num("LEV", (int)m_ptr->lev, 12, STAT_COLUMN);
    prt_lnum("EXP", m_ptr->exp, 13, STAT_COLUMN);
    prt_num("MNA", m_ptr->cmana, 14, STAT_COLUMN);
    prt_num("MHP", m_ptr->mhp, 15, STAT_COLUMN);
    prt_num("CHP", m_ptr->chp, 16, STAT_COLUMN);
    prt_chp();			   /* this will overwrite hp, in color, if
				    * needed. -CFT */
    prt_num("AC ", m_ptr->dis_ac, 18, STAT_COLUMN);
    prt_lnum("AU ", m_ptr->au, 19, STAT_COLUMN);
    prt_winner();
    prt_cut();
    prt_stun();
    prt_study();
    status = py.flags.status;
    if ((PY_HUNGRY | PY_WEAK) & status)
	prt_hunger();
    if (PY_BLIND & status)
	prt_blind();
    if (PY_CONFUSED & status)
	prt_confused();
    if (PY_FEAR & status)
	prt_afraid();
    if (PY_POISONED & status)
	prt_poisoned();
    if ((PY_SEARCH | PY_REST) & status)
	prt_state();
/* if speed non zero, print it, modify speed if Searching */
    if (py.flags.speed - ((PY_SEARCH & status) >> 8) != 0)
	prt_speed();

	prt_equippy_chars();
}

/* EQUIPMENT CHARACTER HANDLER  - DGK */
void
#ifdef __STDC__
prt_equippy_chars(void)
#else
prt_equippy_chars()
#endif                                    
{                                        
    int i, j;                              
    inven_type *i_ptr;                     
    vtype out_val;                           
                                          
    out_val[1]='\0';                       
    for (i = INVEN_WIELD; i < INVEN_ARRAY_SIZE; i++) {
	i_ptr = &inventory[i];                           
	j = i - INVEN_WIELD;                             
	
	if (!equippy_chars || (i_ptr->tval == TV_NOTHING))
	    out_val[0] = ' ';                                
	else                                             
	    out_val[0] = (int)(i_ptr->tchar);                
	
	put_buffer(out_val, 4, j);
    }
}


/* Draws entire screen					-RAK-	 */
void 
draw_cave()
{
    clear_screen();
    prt_stat_block();
    prt_map();
    prt_depth();
}


/* Prints the following information on the screen.	-JWT-	 */
void 
put_character()
{
    register struct misc *m_ptr;

    m_ptr = &py.misc;
    clear_screen();
    put_buffer("Name        :", 2, 1);
    put_buffer("Race        :", 3, 1);
    put_buffer("Sex         :", 4, 1);
    put_buffer("Class       :", 5, 1);
    if (character_generated) {
	put_buffer(m_ptr->name, 2, 15);
	put_buffer(race[m_ptr->prace].trace, 3, 15);
	put_buffer((m_ptr->male ? "Male" : "Female"), 4, 15);
	put_buffer(class[m_ptr->pclass].title, 5, 15);
    }
}


/* Prints the following information on the screen.	-JWT-	 */
void 
put_stats()
{
    register struct misc *m_ptr;
    register int          i, temp;
    vtype                 buf;

    m_ptr = &py.misc;
    for (i = 0; i < 6; i++) {
	cnv_stat(py.stats.use_stat[i], buf);
	put_buffer(stat_names[i], 2 + i, 61);
	put_buffer(buf, 2 + i, 66);
	if (py.stats.max_stat[i] > py.stats.cur_stat[i]) {
	    /* this looks silly, but it happens because modify_stat() only
	     * looks at cur_stat -CFT */
	    temp = py.stats.cur_stat[i];
	    py.stats.cur_stat[i] = py.stats.max_stat[i];
	    cnv_stat (modify_stat(i,py.stats.mod_stat[i]), buf);
	    py.stats.cur_stat[i] = temp; /* DON'T FORGET! -CFT */
	    put_buffer(buf, 2 + i, 73);
	}
    }
    prt_num("+ To Hit    ", m_ptr->dis_th, 9, 1);
    prt_num("+ To Damage ", m_ptr->dis_td, 10, 1);
    prt_num("+ To AC     ", m_ptr->dis_tac, 11, 1);
    prt_num("  Total AC  ", m_ptr->dis_ac, 12, 1);
}


/* Returns a rating of x depending on y			-JWT-	 */
const char *
likert(x, y)
int x, y;
{
    if ((x/y) < 0)
	return ("Very Bad");

    switch ((x / y)) {
      case 0:
      case 1:
	return ("Bad");
      case 2:
	return ("Poor");
      case 3:
      case 4:
	return ("Fair");
      case 5:
	return ("Good");
      case 6:
	return ("Very Good");
      case 7:
      case 8:
	return ("Excellent");
      case 9:
      case 10:
      case 11:
      case 12:
      case 13:
	return ("Superb");
      case 14:
      case 15:
      case 16:
      case 17:
	return ("Heroic");
      default:
	return ("Legendary");
    }
}


/* Prints age, height, weight, and SC			-JWT-	 */
void 
put_misc1()
{
    register struct misc *m_ptr;

    m_ptr = &py.misc;
    prt_num("Age          ", (int)m_ptr->age, 2, 32);
    prt_num("Height       ", (int)m_ptr->ht, 3, 32);
    prt_num("Weight       ", (int)m_ptr->wt, 4, 32);
    prt_num("Social Class ", (int)m_ptr->sc, 5, 32);
}


/* Prints the following information on the screen.	-JWT-	 */
void 
put_misc2()
{
    register struct misc *m_ptr;

    m_ptr = &py.misc;
    prt_num("Level      ", (int)m_ptr->lev, 9, 28);
    prt_lnum("Experience ", m_ptr->exp, 10, 28);
    prt_lnum("Max Exp    ", m_ptr->max_exp, 11, 28);
    if (m_ptr->lev >= MAX_PLAYER_LEVEL) {
	char                buf[40];	/* for this to look right, the
					 * following should be spaced the
					 * same as in the prt_lnum code...
					 * -CFT */

	sprintf(buf, "%s%9s", "Exp to Adv.", "****");
	put_buffer(buf, 12, 28);
    } else
	prt_lnum("Exp to Adv.", (int32) (player_exp[m_ptr->lev - 1] *
					 m_ptr->expfact / 100), 12, 28);
    prt_lnum("Gold       ", m_ptr->au, 13, 28);
    prt_num("Max Hit Points ", m_ptr->mhp, 9, 52);
    prt_num("Cur Hit Points ", m_ptr->chp, 10, 52);
    prt_num("Max Mana       ", m_ptr->mana, 11, 52);
    prt_num("Cur Mana       ", m_ptr->cmana, 12, 52);
}


/* Prints ratings on certain abilities			-RAK-	 */
void 
put_misc3()
{
    int                   xbth, xbthb, xfos, xsrh, xstl, xdis, xsave, xdev;
    vtype                 xinfra;
    register struct misc *p_ptr;

    clear_from(14);
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

    (void)sprintf(xinfra, "%d feet", py.flags.see_infra * 10);

    put_buffer("(Miscellaneous Abilities)", 15, 25);
    put_buffer("Fighting    :", 16, 1);
    put_buffer(likert(xbth, 12), 16, 15);
    put_buffer("Bows/Throw  :", 17, 1);
    put_buffer(likert(xbthb, 12), 17, 15);
    put_buffer("Saving Throw:", 18, 1);
    put_buffer(likert(xsave, 6), 18, 15);

    put_buffer("Stealth     :", 16, 28);
    put_buffer(likert(xstl, 1), 16, 42);
    put_buffer("Disarming   :", 17, 28);
    put_buffer(likert(xdis, 8), 17, 42);
    put_buffer("Magic Device:", 18, 28);
    put_buffer(likert(xdev, 6), 18, 42);

    put_buffer("Perception  :", 16, 55);
    put_buffer(likert(xfos, 3), 16, 69);
    put_buffer("Searching   :", 17, 55);
    put_buffer(likert(xsrh, 6), 17, 69);
    put_buffer("Infra-Vision:", 18, 55);
    put_buffer(xinfra, 18, 69);
}


/* Used to display the character on the screen.		-RAK-	 */
void 
display_char()
{
    put_character();
    put_misc1();
    put_stats();
    put_misc2();
    put_misc3();
}


/* Gets a name for the character			-JWT-	 */
void 
get_name()
{
    char tmp[100];

    strcpy(tmp, py.misc.name);
    prt("Enter your player's name  [press <RETURN> when finished]", 21, 2);
    put_buffer(&blank_string[BLANK_LENGTH - 15], 2, 15);
#ifdef MAC
/*
 * Force player to give a name, would be nice to get name from chooser (STR
 * -16096), but that name might be too long 
 */
    while (!get_string(py.misc.name, 2, 15, 15) || py.misc.name[0] == 0);
#else
    if (!get_string(py.misc.name, 2, 15, 15) || py.misc.name[0] == 0) {
	strcpy(py.misc.name, tmp);
	put_buffer(tmp, 2, 15);
    }
#endif
    clear_from(20);
#ifdef MAC
/* Use the new name to set save file default name. */
    initsavedefaults();
#endif
}


/* Changes the name of the character			-JWT-	 */
void 
change_name()
{
    register char c;
    register int  flag;

#ifndef MAC
    vtype         temp;

#endif

    flag = FALSE;
    display_char();
    do {
	prt("<f>ile character description. <c>hange character name.", 21, 2);
	c = inkey();
	switch (c) {
	  case 'c':
	    get_name();
	    flag = TRUE;
	    break;
	  case 'f':
#ifdef MAC
	/* On mac, file_character() gets filename with std file dialog. */
	    if (file_character())
		flag = TRUE;
#else
	    prt("File name:", 0, 0);
	    if (get_string(temp, 0, 10, 60) && temp[0])
		if (file_character(temp))
		    flag = TRUE;
#endif
	    break;
	  case ESCAPE:
	  case ' ':
	  case '\n':
	  case '\r':
	    flag = TRUE;
	    break;
	  default:
	    bell();
	    break;
	}
    }
    while (!flag);
}


/* Destroy an item in the inventory			-RAK-	 */
void 
inven_destroy(item_val)
int item_val;
{
    register int         j;
    register inven_type *i_ptr;

    i_ptr = &inventory[item_val];
    if ((i_ptr->number > 1) && (i_ptr->subval <= ITEM_SINGLE_STACK_MAX)) {
	i_ptr->number--;
	inven_weight -= i_ptr->weight;
    } else {
	inven_weight -= i_ptr->weight * i_ptr->number;
	for (j = item_val; j < inven_ctr - 1; j++)
	    inventory[j] = inventory[j + 1];
	invcopy(&inventory[inven_ctr - 1], OBJ_NOTHING);
	inven_ctr--;
    }
    py.flags.status |= PY_STR_WGT;
}


/*
 * Copies the object in the second argument over the first argument. However,
 * the second always gets a number of one except for ammo etc. 
 */
void 
take_one_item(s_ptr, i_ptr)
register inven_type *s_ptr, *i_ptr;
{
    *s_ptr = *i_ptr;
    if ((s_ptr->number > 1) && (s_ptr->subval >= ITEM_SINGLE_STACK_MIN)
	&& (s_ptr->subval <= ITEM_SINGLE_STACK_MAX))
	s_ptr->number = 1;
}


/* Drops an item from inventory to given location	-RAK-	 */
void 
inven_drop(item_val, drop_all)
register int item_val, drop_all;
{
    int                  i;
    register inven_type *i_ptr;
    vtype                prt2;
    bigvtype             prt1;

    i_ptr = &inventory[item_val];
    if (cave[char_row][char_col].tptr != 0)
	(void)delete_object(char_row, char_col);
    i = popt();
    t_list[i] = *i_ptr;
    cave[char_row][char_col].tptr = i;

    if (item_val >= INVEN_WIELD)
	takeoff(item_val, -1);
    else {
	if (drop_all || i_ptr->number == 1) {
	    inven_weight -= i_ptr->weight * i_ptr->number;
	    inven_ctr--;
	    while (item_val < inven_ctr) {
		inventory[item_val] = inventory[item_val + 1];
		item_val++;
	    }
	    invcopy(&inventory[inven_ctr], OBJ_NOTHING);
	} else {
	    t_list[i].number = 1;
	    inven_weight -= i_ptr->weight;
	    i_ptr->number--;
	}
	objdes(prt1, &t_list[i], TRUE);
	(void)sprintf(prt2, "Dropped %s.", prt1);
	msg_print(prt2);
    }
    py.flags.status |= PY_STR_WGT;
}


/* Destroys a type of item on a given percent chance	-RAK-	 */
int 
inven_damage(typ, perc)
#ifdef MSDOS
int (*typ) (inven_type *);

#else
int (*typ) ();

#endif
register int perc;

{
    register int index, i, j, offset;
    vtype        tmp_str, out_val;

    j = 0;
    offset = randint(inven_ctr);
    for (index = 0; index < inven_ctr; index++) {
	i = (index + offset) % inven_ctr; /* be clever and not destroy the first item */
	if ((*typ) (&inventory[i]) && (randint(100) < perc)) {
	    objdes(tmp_str, &inventory[i], FALSE);
	    sprintf(out_val, "%sour %s (%c) %s destroyed!",
		    ((inventory[i].subval <= ITEM_SINGLE_STACK_MAX) &&
		     (inventory[i].number > 1))	/* stacked single items */
		    ? "One of y" : "Y",
		    tmp_str, i + 'a',
		    ((inventory[i].subval > ITEM_SINGLE_STACK_MAX) &&
		     (inventory[i].number > 1))	/* stacked group items */
		    ? "were" : "was");
	    msg_print(out_val);
	    inven_destroy(i);
	    j++;
	}
    }
    return (j);
}


/* Computes current weight limit			-RAK-	 */
int 
weight_limit()
{
    register int32 weight_cap;

    weight_cap = (long)py.stats.use_stat[A_STR] * (long)PLAYER_WEIGHT_CAP
	+ (long)py.misc.wt;
    if (weight_cap > 3000L)
	weight_cap = 3000L;
    return ((int)weight_cap);
}


/* this code must be identical to the inven_carry() code below */
int 
inven_check_num(t_ptr)
register inven_type *t_ptr;
{
    register int i;

    if (inven_ctr < INVEN_WIELD)
	return TRUE;
    else if (t_ptr->subval >= ITEM_SINGLE_STACK_MIN)
	for (i = 0; i < inven_ctr; i++)
	    if (inventory[i].tval == t_ptr->tval &&
		inventory[i].subval == t_ptr->subval &&
	/* make sure the number field doesn't overflow */
		((int)inventory[i].number + (int)t_ptr->number < 256) &&
	/* they always stack (subval < 192), or else they have same p1 */
		((t_ptr->subval < ITEM_GROUP_MIN) || (inventory[i].p1 == t_ptr->p1))
	/* only stack if both or neither are identified */
		&& (known1_p(&inventory[i]) == known1_p(t_ptr)))
		return TRUE;
    return FALSE;
}

/* return FALSE if picking up an object would change the players speed */
int 
inven_check_weight(i_ptr)
register inven_type *i_ptr;
{
    register int i, new_inven_weight;

    i = weight_limit();
    new_inven_weight = i_ptr->number * i_ptr->weight + inven_weight;
    if (i < new_inven_weight)
	i = new_inven_weight / (i + 1);
    else
	i = 0;

    if (pack_heavy != i)
	return FALSE;
    else
	return TRUE;
}


/* Are we strong enough for the current pack and weapon?  -CJS-	 */
void 
check_strength()
{
    register int         i;
    register inven_type *i_ptr;
    static int           notlike = FALSE;

    i_ptr = &inventory[INVEN_WIELD];
    if (i_ptr->tval != TV_NOTHING
	&& (py.stats.use_stat[A_STR] * 15 < i_ptr->weight)) {
	if (weapon_heavy == FALSE) {
	    msg_print("You have trouble wielding such a heavy weapon.");
	    weapon_heavy = TRUE;
	    calc_bonuses();
	}
    } else if (weapon_heavy == TRUE) {
	weapon_heavy = FALSE;
	if (i_ptr->tval != TV_NOTHING)
	    msg_print("You are strong enough to wield your weapon.");
	else
	    msg_print("You feel relieved to put down your heavy weapon.");
	calc_bonuses();
    }
    i = weight_limit();
    if (i < inven_weight)
	i = inven_weight / (i + 1);
    else
	i = 0;
    if (pack_heavy != i) {
	if (pack_heavy < i)
	    msg_print("Your pack is so heavy that it slows you down.");
	else
	    msg_print("You move more easily under the weight of your pack.");
	change_speed(i - pack_heavy);
	pack_heavy = i;
    }
    py.flags.status &= ~PY_STR_WGT;

    if (py.misc.pclass == 2 && !notlike) {
        if ((i_ptr->tval == TV_SWORD || i_ptr->tval == TV_POLEARM)
            && ((i_ptr->flags2 & TR_BLESS_BLADE) == 0)) {
            notlike = TRUE;
            msg_print("You do not feel comfortable with your weapon.");
        }
    } else if (py.misc.pclass == 2 && notlike) {
        if (i_ptr->tval == TV_NOTHING) {
            notlike = FALSE;
            msg_print("You feel comfortable again after removing that weapon.");
        } else if (!(i_ptr->tval == TV_SWORD || i_ptr->tval == TV_POLEARM)
		   || !((i_ptr->flags2 & TR_BLESS_BLADE) == 0)) {
            notlike = FALSE;
            msg_print("You feel comfortable with your weapon once more.");
        }
    }
}


/*
 * Add an item to players inventory.  Return the item position for a
 * description if needed.	       -RAK- this code must be identical to
 * the inven_check_num() code above 
 */

/*
 * Okay, here's my inven_carry() function (from misc2.c, I think).  Just
 * replace the existing inven_carry() with this one, and items will sort into
 * place, with mage spellbooks coming first for mages, rangers, and rogues.
 * Also, this will make Tenser's book sort after all the mage books except
 * Raals, instead of in the middle of them (which always seemed strange to
 * me). -CFT 
 */

int 
inven_carry(i_ptr)
register inven_type *i_ptr;
{
    register int         locn = 0, i;
    register int         typ, subt;
    register inven_type *t_ptr;
    int                  known1p, always_known1p;
    int                  tval_tmp;  /* used to make magic books before pray
				    * books if magicuser */
    int                  stacked = FALSE;

    typ = i_ptr->tval;
    subt = i_ptr->subval;
    known1p = known1_p(i_ptr);
    always_known1p = (object_offset(i_ptr) == -1);

    if (inven_ctr >= INVEN_WIELD) /* sanity checking to prevent the inv from */
	inven_ctr = INVEN_WIELD;  /* running over the equipment list -CWS */

/*
 * to prevent nasty losses of objects, we first look through entire inven for
 * a place to stack, w/o assuming the inventory is sorted. -CFT 
 */
    if (subt >= ITEM_SINGLE_STACK_MIN) {
	for (locn = 0; locn < inven_ctr; locn++) {
	    t_ptr = &inventory[locn];
	    if (t_ptr->tval == typ &&
		t_ptr->subval == subt &&
	/* make sure the number field doesn't overflow */
		((int)t_ptr->number + (int)i_ptr->number < 256) &&
	/* they always stack (subval < 192), or else they have same p1 */
		((subt < ITEM_GROUP_MIN) || (t_ptr->p1 == i_ptr->p1))
	/* only stack if both or neither are identified */
		&& (known1_p(&inventory[locn]) == known1p)) {
		stacked = TRUE;	   /* note that we did process the item -CFT */
		t_ptr->number += i_ptr->number;

	/* if player bought at bargin price, then make sure he can't sell back
	 * for normal value.  This is unfair, since it robs the value from items,
	 * but it does prevent the player from "milking" the stores for cash.
	 */
		if (i_ptr->cost < t_ptr->cost)
		    t_ptr->cost = i_ptr->cost;
		break;
	    } /* if it stacks here */
	} /* for loop */
    } /* if it stacks, try to stack it... */

    if (!stacked) {
    /* either it doesn't stack anyway, or it didn't match anything in the inventory.
     * Now try to insert. -CFT */

	for (locn = 0;; locn++) {
	    t_ptr = &inventory[locn];

	/* For items which are always known1p, i.e. never have a 'color',
	 * insert them into the inventory in sorted order.  
	 */
	    if ((typ == TV_PRAYER_BOOK) && (class[py.misc.pclass].spell == MAGE))
		typ = TV_MAGIC_BOOK - 1;
	/* sort is in descending, so this will be immediately after magic books.
	 * It helps that there is no tval that uses this. -CFT
	 */
	    tval_tmp = t_ptr->tval;
	    if ((tval_tmp == TV_PRAYER_BOOK) &&
		(class[py.misc.pclass].spell == MAGE))
		tval_tmp = TV_MAGIC_BOOK - 1;
	/* sort is in descending, so this will be immediately after magic books.
	 * It helps that there is no tval that uses this. -CFT
	 */
	    if ((typ > tval_tmp) ||     /* sort by desc tval */
		(always_known1p &&      /* if always known, then sort by inc level, */
		 (typ == tval_tmp) &&	/* then by inc subval */
		 ((i_ptr->level < t_ptr->level) ||
	     ((i_ptr->level == t_ptr->level) && (subt < t_ptr->subval))))) {
		for (i = inven_ctr - 1; i >= locn; i--)
		    inventory[i + 1] = inventory[i];
		inventory[locn] = *i_ptr;
		inven_ctr++;
		break;
	    }
	}
    }
    inven_weight += i_ptr->number * i_ptr->weight;
    py.flags.status |= PY_STR_WGT;
    return locn;
}


/* Returns spell chance of failure for spell		-RAK-	 */
int 
spell_chance(spell)
int spell;
{
    register spell_type *s_ptr;
    register int         chance;
    register int         stat;
    int                  minfail;

    s_ptr = &magic_spell[py.misc.pclass - 1][spell];
    chance = s_ptr->sfail - 3 * (py.misc.lev - s_ptr->slevel);
    if (class[py.misc.pclass].spell == MAGE)
	stat = A_INT;
    else
	stat = A_WIS;
    chance -= 3 * (stat_adj(stat) - 1);
    if (s_ptr->smana > py.misc.cmana)
	chance += 5 * (s_ptr->smana - py.misc.cmana);
    switch (stat_adj(stat)) {
      case 0:
	minfail = 50;
	break;			   /* I doubt can cast spells with stat this
				    * low, anyways... */
      case 1:
	minfail = 12;
	break;			   /* 8-14 stat */
      case 2:
	minfail = 8;
	break;			   /* 15-17 stat */
      case 3:
	minfail = 5;
	break;			   /* 18-18/49 stat */
      case 4:
	minfail = 4;
	break;			   /* 18/50-18/69 */
      case 5:
	minfail = 4;
	break;			   /* 18/70-18/89 */
      case 6:
	minfail = 3;
	break;			   /* 18/90-18/99 */
      case 7:
	minfail = 3;
	break;			   /* 18/100 */
      case 8:
      case 9:
      case 10:
	minfail = 2;
	break;			   /* 18/101 - /130 */
      case 11:
      case 12:
	minfail = 2;
	break;			   /* /131 - /150 */
      case 13:
      case 14:
	minfail = 1;
	break;			   /* /151 - /170 */
      case 15:
      case 16:
	minfail = 1;
	break;			   /* /171 - /200 */
      default:
	minfail = 0;
	break;			   /* > 18/200 */
    }
    if ((minfail < 5) && (py.misc.pclass != 1) && (py.misc.pclass != 2))
	minfail = 5;		   /* only mages/priests can get best
				    * chances... */
    if (py.misc.pclass == 2) {	   /* Big prayer penalty for edged weapons
				    * -DGK */
	register inven_type *i_ptr = &inventory[INVEN_WIELD];

	if ((i_ptr->tval == TV_SWORD) || (i_ptr->tval == TV_POLEARM))
	    if ((i_ptr->flags2 & TR_BLESS_BLADE) == 0)
		chance += 25;
    }
    if (chance > 95)
	chance = 95;
    else if (chance < minfail)
	chance = minfail;
    return chance;
}


/* Print list of spells					-RAK-	 */
/*
 * if nonconsec is -1: spells numbered consecutively from 'a' to 'a'+num >=0:
 * spells numbered by offset from nonconsec 
 */
void 
print_spells(spell, num, comment, nonconsec)
int         *spell;
register int num;
int          comment, nonconsec;
{
    register int         i, j;
    vtype                out_val;
    register spell_type *s_ptr;
    int                  col, offset;
    const char          *p;
    char                 spell_char;

    if (comment)
	col = 22;
    else
	col = 31;
    offset = (class[py.misc.pclass].spell == MAGE ? SPELL_OFFSET : PRAYER_OFFSET);
    erase_line(1, col);
    put_buffer("Name", 1, col + 5);
    put_buffer("Lv Mana Fail", 1, col + 35);
/* only show the first 22 choices */
    if (num > 22)
	num = 22;
    for (i = 0; i < num; i++) {
	j = spell[i];
	s_ptr = &magic_spell[py.misc.pclass - 1][j];
	if (comment == FALSE)
	    p = "";
	else if (j >= 32 ? ((spell_forgotten2 & (1L << (j - 32))) != 0)
		 : ((spell_forgotten & (1L << j)) != 0))
	    p = " forgotten";
	else if (j >= 32 ? ((spell_learned2 & (1L << (j - 32))) == 0)
		 : ((spell_learned & (1L << j)) == 0))
	    p = " unknown";
	else if (j >= 32 ? ((spell_worked2 & (1L << (j - 32))) == 0)
		 : ((spell_worked & (1L << j)) == 0))
	    p = " untried";
	else
	    p = "";
    /* determine whether or not to leave holes in character choices,
     * nonconsec -1 when learning spells, consec offset>=0 when asking which
     * spell to cast 
     */
	if (nonconsec == -1)
	    spell_char = 'a' + i;
	else
	    spell_char = 'a' + j - nonconsec;
	(void)sprintf(out_val, "  %c) %-30s%2d %4d %3d%%%s", spell_char,
		      spell_names[j + offset], s_ptr->slevel, s_ptr->smana,
		      spell_chance(j), p);
	prt(out_val, 2 + i, col);
    }
}


/* Returns spell pointer				-RAK-	 */
int 
get_spell(spell, num, sn, sc, prompt, first_spell)
int          *spell;
register int  num;
register int *sn, *sc;
const char   *prompt;
int           first_spell;
{
    register spell_type *s_ptr;
    int                  flag, redraw, offset, i;
    char                 choice;
    vtype                out_str, tmp_str;

    *sn = (-1);
    flag = FALSE;
    (void)sprintf(out_str, "(Spells %c-%c, *=List, <ESCAPE>=exit) %s",
	   spell[0] + 'a' - first_spell, spell[num - 1] + 'a' - first_spell,
		  prompt);
    redraw = FALSE;
    offset = (class[py.misc.pclass].spell == MAGE ? SPELL_OFFSET : PRAYER_OFFSET);
    while (flag == FALSE && get_com(out_str, &choice)) {
	if (isupper((int)choice)) {
	    *sn = choice - 'A' + first_spell;
	/* verify that this is in spell[], at most 22 entries in spell[] */
	    for (i = 0; i < num; i++)
		if (*sn == spell[i])
		    break;
	    if (i == num)
		*sn = (-2);
	    else {
		s_ptr = &magic_spell[py.misc.pclass - 1][*sn];
		(void)sprintf(tmp_str, "Cast %s (%d mana, %d%% fail)?",
			      spell_names[*sn + offset], s_ptr->smana,
			      spell_chance(*sn));
		if (get_check(tmp_str))
		    flag = TRUE;
		else
		    *sn = (-1);
	    }
	} else if (islower((int)choice)) {
	    *sn = choice - 'a' + first_spell;
	/* verify that this is in spell[], at most 22 entries in spell[] */
	    for (i = 0; i < num; i++)
		if (*sn == spell[i])
		    break;
	    if (i == num)
		*sn = (-2);
	    else
		flag = TRUE;
	} else if (choice == '*') {
	/* only do this drawing once */
	    if (!redraw) {
		save_screen();
		redraw = TRUE;
		print_spells(spell, num, FALSE, first_spell);
	    }
	} else if (isalpha((int)choice))
	    *sn = (-2);
	else {
	    *sn = (-1);
	    bell();
	}
	if (*sn == -2) {
	    sprintf(tmp_str, "You don't know that %s.",
		(class[py.misc.pclass].spell == MAGE) ? "spell" : "prayer");
	    msg_print(tmp_str);
	}
    }
    if (redraw)
	restore_screen();

    erase_line(MSG_LINE, 0);
    if (flag)
	*sc = spell_chance(*sn);

    return (flag);
}


/* calculate number of spells player should have, and learn forget spells
 * until that number is met -JEW- 
 */
void 
calc_spells(stat)
int stat;
{
    register int    i;
    register int32u mask;
    int32u          spell_flag;
    int             j, offset;
    int             num_allowed, new_spells, num_known, levels;
    vtype           tmp_str;
    const char           *p;
    register struct misc *p_ptr;
    register spell_type  *msp_ptr;

    p_ptr = &py.misc;
    msp_ptr = &magic_spell[p_ptr->pclass - 1][0];
    if (stat == A_INT) {
	p = "spell";
	offset = SPELL_OFFSET;
    } else {
	p = "prayer";
	offset = PRAYER_OFFSET;
    }

/* check to see if know any spells greater than level, eliminate them */
    for (i = 31, mask = 0x80000000L; mask; mask >>= 1, i--) {
	if (mask & spell_learned) {
	    if (msp_ptr[i].slevel > p_ptr->lev) {
		spell_learned &= ~mask;
		spell_forgotten |= mask;
		(void)sprintf(tmp_str, "You have forgotten the %s of %s.", p,
			      spell_names[i + offset]);
		msg_print(tmp_str);
	    }
	}
	if (mask & spell_learned2) {
	    if (msp_ptr[i + 32].slevel > p_ptr->lev) {
		spell_learned2 &= ~mask;
		spell_forgotten2 |= mask;
		(void)sprintf(tmp_str, "You have forgotten the %s of %s.", p,
			      spell_names[i + offset + 32]);
		msg_print(tmp_str);
	    }
	}
    }

/* calc number of spells allowed */
    levels = p_ptr->lev - class[p_ptr->pclass].first_spell_lev + 1;
    switch (stat_adj(stat)) {
      case 0:
	num_allowed = 0;
	break;
      case 1:
      case 2:
      case 3:
	num_allowed = 1 * levels;
	break;
      case 4:
      case 5:
	num_allowed = 3 * levels / 2;
	break;
      case 6:
	num_allowed = 2 * levels;
	break;
      default:
	num_allowed = 5 * levels / 2;
	break;
    }

    num_known = 0;
    for (mask = 0x1; mask; mask <<= 1) {
	if (mask & spell_learned)
	    num_known++;
	if (mask & spell_learned2)
	    num_known++;
    }

    new_spells = num_allowed - num_known;

    if (new_spells > 0) {

    /* remember forgotten spells while forgotten spells exist of new_spells
     * positive, remember the spells in the order that they were learned 
     */
	for (i = 0; ((spell_forgotten | spell_forgotten2) && new_spells
		     && (i < num_allowed) && (i < 64)); i++) {
	/* j is (i+1)th spell learned */
	    j = spell_order[i];

	/* shifting by amounts greater than number of bits in long gives an
	 * undefined result, so don't shift for unknown spells 
	 */
	    if (j == 99)
		continue;	   /* don't process unknown spells... -CFT */

	    if (j < 32) {	   /* use spell_learned, spell_forgotten...
				    * -CFT */
		mask = 1L << j;	   /* bit in spell fields */
		if (mask & spell_forgotten) {
		    if (msp_ptr[j].slevel <= p_ptr->lev) {
			spell_forgotten &= ~mask;
			spell_learned |= mask;
			new_spells--;
			(void)sprintf(tmp_str, "You have remembered the %s of %s.", p,
				      spell_names[j + offset]);
			msg_print(tmp_str);
		    } else
			num_allowed++;	/* if was too high lv to remember */
		} /* if mask&spell_forgotten */
	    }
	     /* j < 32 */ 
	    else {		   /* j > 31, use spell_learned2,
				    * spell_forgotten2... -CFT */
		mask = 1L << (j - 32);	/* bit in spell fields */
		if (mask & spell_forgotten2) {
		    if (msp_ptr[j].slevel <= p_ptr->lev) {
			spell_forgotten2 &= ~mask;
			spell_learned2 |= mask;
			new_spells--;
			(void)sprintf(tmp_str, "You have remembered the %s of %s.", p,
				      spell_names[j + offset]);
			msg_print(tmp_str);
		    } else
			num_allowed++;	/* if was too high lv to remember */
		} /* if mask&spell_forgotten2 */
	    } /* j > 31 */
	} /* for loop... */

	if (new_spells > 0) {
	/* determine which spells player can learn */
	/*
	 * must check all spells here, in gain_spell() we actually check if
	 * the books are present 
	 */
	/* only bother with spells learnable by class -CFT */
	    spell_flag = spellmasks[py.misc.pclass][0] & ~spell_learned;
	    mask = 0x1;
	    i = 0;
	    for (j = 0, mask = 0x1; spell_flag; mask <<= 1, j++)
		if (spell_flag & mask) {
		    spell_flag &= ~mask;
		    if (msp_ptr[j].slevel <= p_ptr->lev)
			i++;
		}
	/* only bother with spells learnable by class -CFT */
	    spell_flag = spellmasks[py.misc.pclass][1] & ~spell_learned2;
	    mask = 0x1;
	    for (j = 0, mask = 0x1; spell_flag; mask <<= 1, j++)
		if (spell_flag & mask) {
		    spell_flag &= ~mask;
		    if (msp_ptr[j + 32].slevel <= p_ptr->lev)
			i++;
		}
	    if (new_spells > i)
		new_spells = i;
	}
    } else if (new_spells < 0) {

    /* forget spells until new_spells zero or no more spells know, spells are
     * forgotten in the opposite order that they were learned 
     */
	for (i = 63; new_spells && (spell_learned | spell_learned2); i--) {
	/* j is the (i+1)th spell learned */
	    j = spell_order[i];

	/* shifting by amounts greater than number of bits in long gives an
	 * undefined result, so don't shift for unknown spells 
	 */
	    if (j == 99)
		continue;	   /* don't process unknown spells... -CFT */

	    if (j < 32) {	   /* use spell_learned, spell_forgotten...
				    * -CFT */
		mask = 1L << j;	   /* bit in spell fields */
		if (mask & spell_learned) {
		    spell_learned &= ~mask;
		    spell_forgotten |= mask;
		    new_spells++;
		    (void)sprintf(tmp_str, "You have forgotten the %s of %s.", p,
				  spell_names[j + offset]);
		    msg_print(tmp_str);
		} /* if mask&spell_learned */
	    }
	     /* j < 32 */ 
	    else {		   /* j > 31, use spell_learned2,
				    * spell_forgotten2... -CFT */
		mask = 1L << (j - 32);	/* bit in spell fields */
		if (mask & spell_learned2) {
		    spell_learned2 &= ~mask;
		    spell_forgotten2 |= mask;
		    new_spells++;
		    (void)sprintf(tmp_str, "You have forgotten the %s of %s.", p,
				  spell_names[j + offset]);
		    msg_print(tmp_str);
		} /* if mask&spell_learned2 */
	    } /* j > 31 */
	} /* for loop... */
	new_spells = 0;		   /* we've forgotten, so we shouldn't be
				    * learning any... */
    }
    if (new_spells != py.flags.new_spells) {
	if (new_spells > 0 && py.flags.new_spells == 0) {
	    (void)sprintf(tmp_str, "You can learn some new %ss now.", p);
	    msg_print(tmp_str);
	}
	py.flags.new_spells = new_spells;
	py.flags.status |= PY_STUDY;
    }
}


/* gain spells when player wants to		- jw */
void 
gain_spells()
{
    char                query;
    int                 stat, diff_spells, new_spells;
    int                 spells[63], offset, last_known;
    register int        i, j;
    register int32u     spell_flag = 0, spell_flag2 = 0, mask;
    vtype               tmp_str;
    struct misc         *p_ptr;
    register spell_type *msp_ptr;

    if (!py.misc.pclass) {
	msg_print("A warrior learn magic???  HA!");
	return;
    }
    i = 0;
    if (py.flags.blind > 0)
	msg_print("You can't see to read your spell book!");
    else if (no_light())
	msg_print("You have no light to read by.");
    else if (py.flags.confused > 0)
	msg_print("You are too confused.");
    else
	i = 1;
    if (i == 0)
	return;

    new_spells = py.flags.new_spells;
    diff_spells = 0;
    p_ptr = &py.misc;
    msp_ptr = &magic_spell[p_ptr->pclass - 1][0];
    if (class[p_ptr->pclass].spell == MAGE) {
	stat = A_INT;
	offset = SPELL_OFFSET;
    } else {
	stat = A_WIS;
	offset = PRAYER_OFFSET;
    }

    for (last_known = 0; last_known < 64; last_known++)
	if (spell_order[last_known] == 99)
	    break;

    if (!new_spells) {
	(void)sprintf(tmp_str, "You can't learn any new %ss!",
		      (stat == A_INT ? "spell" : "prayer"));
	msg_print(tmp_str);
	free_turn_flag = TRUE;
    } else {
    /* determine which spells player can learn */
    /* mages need the book to learn a spell, priests do not need the book */
	spell_flag = 0;
	spell_flag2 = 0;
	for (i = 0; i < inven_ctr; i++)
	    if (((stat == A_INT) && (inventory[i].tval == TV_MAGIC_BOOK))
	    || ((stat == A_WIS) && (inventory[i].tval == TV_PRAYER_BOOK))) {
		spell_flag |= inventory[i].flags;
		spell_flag2 |= inventory[i].flags2;
	    }
    }

/* clear bits for spells already learned */
    spell_flag &= ~spell_learned;
    spell_flag2 &= ~spell_learned2;

    mask = 0x1;
    i = 0;
    for (j = 0, mask = 0x1; (spell_flag | spell_flag2); mask <<= 1, j++) {
	if (spell_flag & mask) {
	    spell_flag &= ~mask;
	    if (msp_ptr[j].slevel <= p_ptr->lev) {
		spells[i] = j;
		i++;
	    }
	}
	if (spell_flag2 & mask) {
	    spell_flag2 &= ~mask;
	    if (msp_ptr[j + 32].slevel <= p_ptr->lev) {
		spells[i] = j + 32;
		i++;
	    }
	}
    }

    if (new_spells > i) {
	msg_print("You seem to be missing a book.");
	diff_spells = new_spells - i;
	new_spells = i;
    }
    if (new_spells == 0);
    else if (stat == A_INT) {
    /* get to choose which mage spells will be learned */
	save_screen();
	print_spells(spells, i, FALSE, -1);
	while (new_spells && get_com("Learn which spell?", &query)) {
	    j = query - 'a';

	/* test j < 23 in case i is greater than 22, only 22 spells are
	 * actually shown on the screen, so limit choice to those 
	 */
	    if (j >= 0 && j < i && j < 22) {
		new_spells--;
		if (spells[j] < 32)
		    spell_learned |= 1L << spells[j];
		else
		    spell_learned2 |= 1L << (spells[j] - 32);
		spell_order[last_known++] = spells[j];
		for (; j <= i - 1; j++)
		    spells[j] = spells[j + 1];
		i--;
		erase_line(j + 1, 31);
		print_spells(spells, i, FALSE, -1);
	    } else
		bell();
	}
	restore_screen();
    } else {
    /* pick a prayer at random */
	while (new_spells) {
	    j = randint(i) - 1;
	    if (spells[j] < 32)
		spell_learned |= 1L << spells[j];
	    else
		spell_learned2 |= 1L << (spells[j] - 32);
	    spell_order[last_known++] = spells[j];
	    (void)sprintf(tmp_str,
			  "You have learned the prayer of %s.",
			  spell_names[spells[j] + offset]);
	    msg_print(tmp_str);
	    for (; j <= i - 1; j++)
		spells[j] = spells[j + 1];
	    i--;
	    new_spells--;
	}
    }
    py.flags.new_spells = new_spells + diff_spells;
    if (py.flags.new_spells == 0)
	py.flags.status |= PY_STUDY;
/* set the mana for first level characters when they learn first spell */
    if (py.misc.mana == 0)
	calc_mana(stat);
}



/* Gain some mana if you know at least one spell	-RAK-	 */
void 
calc_mana(stat)
int stat;
{
    register int          new_mana, levels;
    register struct misc *p_ptr;
    register int32        value;
    register int          i;
    register inven_type  *i_ptr;
    int                   amrwgt, maxwgt;

    p_ptr = &py.misc;
    if (spell_learned != 0 || spell_learned2 != 0) {
	levels = p_ptr->lev - class[p_ptr->pclass].first_spell_lev + 1;
	switch (stat_adj(stat)) {
	  case 0:
	    new_mana = 0;
	    break;
	  case 1:
	  case 2:
	    new_mana = 1 * levels;
	    break;
	  case 3:
	    new_mana = 3 * levels / 2;
	    break;
	  case 4:
	    new_mana = 2 * levels;
	    break;
	  case 5:
	    new_mana = 5 * levels / 2;
	    break;
	  case 6:
	    new_mana = 3 * levels;
	    break;
	  case 7:
	    new_mana = 4 * levels;
	    break;
	  case 8:
	    new_mana = 9 * levels / 2;
	    break;
	  case 9:
	    new_mana = 5 * levels;
	    break;
	  case 10:
	    new_mana = 11 * levels / 2;
	    break;
	  case 11:
	    new_mana = 6 * levels;
	    break;
	  case 12:
	    new_mana = 13 * levels / 2;
	    break;
	  case 13:
	    new_mana = 7 * levels;
	    break;
	  case 14:
	    new_mana = 15 * levels / 2;
	    break;
	  default:
	    new_mana = 8 * levels;
	    break;
	}
    /* increment mana by one, so that first level chars have 2 mana */
	if (new_mana > 0)
	    new_mana++;
	if ((inventory[INVEN_HANDS].tval != TV_NOTHING) &&
	    !((inventory[INVEN_HANDS].flags & TR_FREE_ACT) ||
	      ((inventory[INVEN_HANDS].flags & TR_DEX) &&
	       (inventory[INVEN_HANDS].p1 > 0)))
/* gauntlets of dex (or free action - DGK) can hardly interfere w/ spellcasting!
 * But cursed ones can! -CFT */

	    &&(py.misc.pclass == 1 || py.misc.pclass == 3 || py.misc.pclass == 4)) {
	    new_mana = (3 * new_mana) / 4;
	}
    /* Start of **NEW ENCUMBRANCE CALCULATION**    -DGK- */
	amrwgt = 0;
	for (i = INVEN_WIELD; i < INVEN_ARRAY_SIZE; i++) {
	    i_ptr = &inventory[i];
	    switch (i) {
	      case INVEN_HEAD:
	      case INVEN_BODY:
	      case INVEN_ARM:
	      case INVEN_HANDS:
	      case INVEN_FEET:
	      case INVEN_OUTER:
		amrwgt += i_ptr->weight;
	    }
	}
	switch (py.misc.pclass) {
	  case 1:
	    maxwgt = 300;
	    break;
	  case 2:
	    maxwgt = 350;
	    break;
	  case 3:
	    maxwgt = 350;
	    break;
	  case 4:
	    maxwgt = 400;
	    break;
	  case 5:
	    maxwgt = 400;
	    break;
	  default:
	    maxwgt = 0;
	}
	if (amrwgt > maxwgt)
	    new_mana -= ((amrwgt - maxwgt) / 10);
    /* end of new mana calc */

    /* if low int/wis, gloves, and lots of heavy armor, new_mana could be
     * negative.  This would be very unlikely, except when int/wis was high
     * enough to compensate for armor, but was severly drained by an annoying
     * monster.  Since the following code blindly assumes that new_mana is >=
     * 0, we must do the work and return here. -CFT 
     */
	if (new_mana < 1) {
	    p_ptr->cmana = p_ptr->cmana_frac = p_ptr->mana = 0;
	    py.flags.status |= PY_MANA;
	    return;		   /* now return before we reach code that
				    * assumes new_mana is positive.... */
	}
    /* mana can be zero when creating character */
	if (p_ptr->mana != new_mana) {
	    if (p_ptr->mana != 0) {
	    /*
	     * change current mana proportionately to change of max mana,
	     * divide first to avoid overflow, little loss of accuracy 
	     */
		value = (((long)p_ptr->cmana << 16) + p_ptr->cmana_frac)
		    / p_ptr->mana * new_mana;
		p_ptr->cmana = value >> 16;
		p_ptr->cmana_frac = value & 0xFFFF;
	    } else {
		p_ptr->cmana = new_mana;
		p_ptr->cmana_frac = 0;
	    }
	    p_ptr->mana = new_mana;
	/* can't print mana here, may be in store or inventory mode */
	    py.flags.status |= PY_MANA;
	}
    } else if (p_ptr->mana != 0) {
	p_ptr->mana = 0;
	p_ptr->cmana = 0;
    /* can't print mana here, may be in store or inventory mode */
	py.flags.status |= PY_MANA;
    }
}


/* Increases hit points and level			-RAK-	 */
static void 
gain_level()
{
    vtype               out_val;
    register struct misc *p_ptr;
    register class_type *c_ptr;

    p_ptr = &py.misc;
    p_ptr->lev++;
    (void)sprintf(out_val, "Welcome to level %d.", (int)p_ptr->lev);
    msg_print(out_val);
    calc_hitpoints();
    prt_level();
    prt_title();
    c_ptr = &class[p_ptr->pclass];
    if (c_ptr->spell == MAGE) {
	calc_spells(A_INT);
	calc_mana(A_INT);
    } else if (c_ptr->spell == PRIEST) {
	calc_spells(A_WIS);
	calc_mana(A_WIS);
    }
}

/* Prints experience					-RAK-	 */
void 
prt_experience()
{
    register struct misc *p_ptr;
    char out_val[100];
    
    p_ptr = &py.misc;
    if (p_ptr->exp > MAX_EXP)
	p_ptr->exp = MAX_EXP;
    if (p_ptr->lev < MAX_PLAYER_LEVEL)
    {
	while ((player_exp[p_ptr->lev-1] * p_ptr->expfact / 100) <= p_ptr->exp
	       && p_ptr->lev < MAX_PLAYER_LEVEL){
	    gain_level();
	    if (p_ptr->exp > p_ptr->max_exp) {
		/* level was actually gained, not restored:
		 * this 300 is arbitrary, but it makes human ages work okay,
		 * and I chose the other racial age adjs based on this as well -CFT
		 */
		p_ptr->age += randint((int16u)class[p_ptr->pclass].age_adj *
				      (int16u)race[p_ptr->prace].m_age)/300;
	    }
	}
    }
    if (p_ptr->exp > p_ptr->max_exp)
	p_ptr->max_exp = p_ptr->exp;
    (void) sprintf(out_val, "%8ld", (long)p_ptr->exp);
    put_buffer(out_val, 13, STAT_COLUMN+4);
}


/* Calculate the players hit points */
void 
calc_hitpoints()
{
    register int          hitpoints;
    register struct misc *p_ptr;
    register int32        value;

    p_ptr = &py.misc;
    hitpoints = player_hp[p_ptr->lev - 1] + (con_adj() * p_ptr->lev);
/* always give at least one point per level + 1 */
    if (hitpoints < (p_ptr->lev + 1))
	hitpoints = p_ptr->lev + 1;

    if (py.flags.status & PY_HERO)
	hitpoints += 10;
    if (py.flags.status & PY_SHERO)
	hitpoints += 30;

/* mhp can equal zero while character is being created */
    if ((hitpoints != p_ptr->mhp) && (p_ptr->mhp != 0)) {
    /* change current hit points proportionately to change of mhp, divide
     * first to avoid overflow, little loss of accuracy 
     */
	value = (((long)p_ptr->chp << 16) + p_ptr->chp_frac) / p_ptr->mhp
	    * hitpoints;
	p_ptr->chp = value >> 16;
	p_ptr->chp_frac = value & 0xFFFF;
	p_ptr->mhp = hitpoints;

    /* can't print hit points here, may be in store or inventory mode */
	py.flags.status |= PY_HP;
    }
}


/* Inserts a string into a string				 */
void 
insert_str(object_str, mtc_str, insert)
char       *object_str;
const char *mtc_str, *insert;
{
    int            obj_len;
    char          *bound, *pc;
    register int   i, mtc_len;
    register char *temp_obj;
    const char    *temp_mtc;
    char           out_val[80];

    mtc_len = strlen(mtc_str);
    obj_len = strlen(object_str);
    bound = object_str + obj_len - mtc_len;
    for (pc = object_str; pc <= bound; pc++) {
	temp_obj = pc;
	temp_mtc = mtc_str;
	for (i = 0; i < mtc_len; i++)
	    if (*temp_obj++ != *temp_mtc++)
		break;
	if (i == mtc_len)
	    break;
    }

    if (pc <= bound) {
	(void)strncpy(out_val, object_str, (int)(pc - object_str));
	out_val[(int)(pc - object_str)] = '\0';
	if (insert)
	    (void)strcat(out_val, insert);
	(void)strcat(out_val, (char *)(pc + mtc_len));
	(void)strcpy(object_str, out_val);
    }
}


void 
insert_lnum(object_str, mtc_str, number, show_sign)
char                *object_str;
register const char *mtc_str;
int32                number;
int                  show_sign;
{
    int            mlen;
    vtype          str1, str2;
    register char *string, *tmp_str;
    int            flag;

    flag = 1;
    mlen = strlen(mtc_str);
    tmp_str = object_str;
    do {
	string = (char *) index(tmp_str, mtc_str[0]);
	if (string == 0)
	    flag = 0;
	else {
	    flag = strncmp(string, mtc_str, mlen);
	    if (flag)
		tmp_str = string + 1;
	}
    }
    while (flag);
    if (string) {
	(void)strncpy(str1, object_str, (int)(string - object_str));
	str1[(int)(string - object_str)] = '\0';
	(void)strcpy(str2, string + mlen);
	if ((number >= 0) && (show_sign))
	    (void)sprintf(object_str, "%s+%ld%s", str1, (long)number, str2);
	else
	    (void)sprintf(object_str, "%s%ld%s", str1, (long)number, str2);
    }
}


/* lets anyone enter wizard mode after a disclaimer...		- JEW - */
int 
enter_wiz_mode()
{
    register int answer;

    if (!is_wizard(player_uid))
	return FALSE;
    if (!noscore) {
	msg_print("Wizard mode is for debugging and experimenting.");
	answer = get_Yn(
			"The game will not be scored if you enter wizard mode. Are you sure?");
    }
    if (noscore || answer) {
	noscore |= 0x2;
	wizard = TRUE;
	return (TRUE);
    }
    return (FALSE);
}


/* Weapon weight VS strength and dexterity		-RAK-	 */
int 
attack_blows(weight, wtohit)
int  weight;
int *wtohit;
{
    register int adj_weight;
    register int str_index, dex_index, s, d;

    s = py.stats.use_stat[A_STR];
    d = py.stats.use_stat[A_DEX];
    if (s * 15 < weight) {
	*wtohit = s * 15 - weight;
	return 1;
    } else {
	*wtohit = 0;
	if (d < 10)
	    dex_index = 0;
	else if (d < 19)
	    dex_index = 1;
	else if (d < 68)
	    dex_index = 2;
	else if (d < 108)
	    dex_index = 3;
	else if (d < 118)
	    dex_index = 4;
	else if (d == 118)
	    dex_index = 5;
	else if (d < 128)
	    dex_index = 6;
	else if (d < 138)
	    dex_index = 7;
	else if (d < 148)
	    dex_index = 8;
	else if (d < 158)
	    dex_index = 9;
	else if (d < 168)
	    dex_index = 10;
	else
	    dex_index = 11;

	switch (py.misc.pclass) { /* new class-based weight penalties -CWS */
	case 0:				/* Warriors */
	    adj_weight = ((s * 10) / ((weight < 30) ? 30 : weight));
	    break;
	case 1:				/* Mages */
	    adj_weight = ((s * 4) / ((weight < 40) ? 40 : weight));
	    break;
	case 2:				/* Priests */
	    adj_weight = ((s * 7) / ((weight < 35) ? 35 : weight));
	    break;
	case 3:				/* Rogues */
	    adj_weight = ((s * 6) / ((weight < 30) ? 30 : weight));
	    break;
	case 4:				/* Rangers */
	    adj_weight = ((s * 8) / ((weight < 35) ? 35 : weight));
	    break;
	default:			/* Paladins */
	    adj_weight = ((s * 8) / ((weight < 30) ? 30 : weight));
	    break;
	}

	if (adj_weight < 2)
	    str_index = 0;
	else if (adj_weight < 3)
	    str_index = 1;
	else if (adj_weight < 4)
	    str_index = 2;
	else if (adj_weight < 6)
	    str_index = 3;
	else if (adj_weight < 8)
	    str_index = 4;
	else if (adj_weight < 10)
	    str_index = 5;
	else if (adj_weight < 13)
	    str_index = 6;
	else if (adj_weight < 15)
	    str_index = 7;
	else if (adj_weight < 18)
	    str_index = 8;
	else if (adj_weight < 20)
	    str_index = 9;
	else
	    str_index = 10;

	s = 0;				/* do Weapons of Speed */
	for (d = INVEN_WIELD; d < INVEN_AUX; d++)
	    if (inventory[d].flags2 & TR_ATTACK_SPD)
		s += inventory[d].p1;

	d = (int)blows_table[str_index][dex_index];
	
	if (py.misc.pclass != 0)	/* Non-warrior attack penalty */
	    if (d > 5)
		d = 5;
	if (py.misc.pclass == 1)
	    if (d > 4)
		d = 4;

	d += s;
	return ((d < 1) ? 1 : d);
    }
}


/* Special damage due to magical abilities of object	-RAK-	 */
int 
tot_dam(i_ptr, tdam, monster)
register inven_type *i_ptr;
register int         tdam;
int                  monster;
{
    register creature_type *m_ptr;
    register recall_type   *r_ptr;
    int                     reduced = FALSE;
    /* don't resist more than one thing.... -CWS */

    if ((((i_ptr->tval >= TV_SLING_AMMO) && (i_ptr->tval <= TV_ARROW)) ||
	 ((i_ptr->tval >= TV_HAFTED) && (i_ptr->tval <= TV_SWORD)) ||
	 (i_ptr->tval == TV_FLASK))) {
	m_ptr = &c_list[monster];
	r_ptr = &c_recall[monster];
    /* Mjollnir? :-> */
	if (!(m_ptr->cdefense & IM_LIGHTNING) && (i_ptr->flags2 & TR_LIGHTNING)) {
	    tdam *= 5;
	}
    /* Execute Dragon */
	else if ((m_ptr->cdefense & DRAGON) && (i_ptr->flags & TR_SLAY_X_DRAGON)) {
	    tdam *= 5;
	    r_ptr->r_cdefense |= DRAGON;
	}
    /* Slay Dragon  */
	else if ((m_ptr->cdefense & DRAGON) && (i_ptr->flags & TR_SLAY_DRAGON)) {
	    tdam *= 3;
	    r_ptr->r_cdefense |= DRAGON;
	}
    /* Slay Undead  */
	else if ((m_ptr->cdefense & UNDEAD) && (i_ptr->flags & TR_SLAY_UNDEAD)) {
	    tdam *= 3;
	    r_ptr->r_cdefense |= UNDEAD;
	}
    /* Slay ORC     */
	else if ((m_ptr->cdefense & ORC) && (i_ptr->flags2 & TR_SLAY_ORC)) {
	    tdam *= 3;
	    r_ptr->r_cdefense |= ORC;
	}
    /* Slay TROLL     */
	else if ((m_ptr->cdefense & TROLL) && (i_ptr->flags2 & TR_SLAY_TROLL)) {
	    tdam *= 3;
	    r_ptr->r_cdefense |= TROLL;
	}
    /* Slay GIANT     */
	else if ((m_ptr->cdefense & GIANT) && (i_ptr->flags2 & TR_SLAY_GIANT)) {
	    tdam *= 3;
	    r_ptr->r_cdefense |= GIANT;
	}
    /* Slay DEMON     */
	else if ((m_ptr->cdefense & DEMON) && (i_ptr->flags2 & TR_SLAY_DEMON)) {
	    tdam *= 3;
	    r_ptr->r_cdefense |= DEMON;
	}
    /* Frost	       */
	else if ((!(m_ptr->cdefense & IM_FROST))
		 && (i_ptr->flags & TR_FROST_BRAND)) {
	    tdam *= 3;
	}
    /* Fire	      */
	else if ((!(m_ptr->cdefense & IM_FIRE))
		 && (i_ptr->flags & TR_FLAME_TONGUE)) {
	    tdam *= 3;
	}
    /* Slay Evil     */
	else if ((m_ptr->cdefense & EVIL) && (i_ptr->flags & TR_SLAY_EVIL)) {
	    tdam *= 2;
	    r_ptr->r_cdefense |= EVIL;
	}
    /* Slay Animal  */
	else if ((m_ptr->cdefense & ANIMAL) && (i_ptr->flags & TR_SLAY_ANIMAL)) {
	    tdam *= 2;
	    r_ptr->r_cdefense |= ANIMAL;
	}
				/* let's do the resistances */
	if (((m_ptr->cdefense & IM_FROST)) && (i_ptr->flags & TR_FROST_BRAND)) {
	    r_ptr->r_cdefense |= IM_FROST;
	    tdam = (tdam * 3) / 4;
	    reduced = TRUE;
	}
	if (((m_ptr->cdefense & IM_FIRE)) && (i_ptr->flags & TR_FLAME_TONGUE)) {
	    r_ptr->r_cdefense |= IM_FIRE;
	    if (!reduced) {
		tdam = (tdam * 3) / 4;
		reduced = TRUE;
	    }
	}
	if (((m_ptr->cdefense & IM_LIGHTNING)) && (i_ptr->flags2 & TR_LIGHTNING)) {
	    r_ptr->r_cdefense |= IM_LIGHTNING;
	    if (!reduced) {
		tdam = (tdam * 3) / 4;
		reduced = TRUE;
	    }
	}
	if ((i_ptr->flags2 & TR_IMPACT) && (tdam > 50))
	    earthquake();
    }
    return (tdam);
}


/* Critical hits, Nasty way to die.			-RAK-	 */
int 
critical_blow(weight, plus, dam, attack_type)
register int weight, plus, dam;
int          attack_type;
{
    register int critical;

    critical = dam;
/* Weight of weapon, plusses to hit, and character level all */
/* contribute to the chance of a critical	             */
    if (randint(5000) <= (int)(weight + 5 * plus
			     + (class_level_adj[py.misc.pclass][attack_type]
				* py.misc.lev))) {
	weight += randint(650);
	if (weight < 400) {
	    critical = 2 * dam + 5;
	    msg_print("It was a good hit!");
	} else if (weight < 700) {
	    critical = 2 * dam + 10;
	    msg_print("It was an excellent hit!");
	} else if (weight < 900) {
	    critical = 3 * dam + 15;
	    msg_print("It was a superb hit!");
	} else if (weight < 1300) {
	    critical = 3 * dam + 20;
	    msg_print("It was a *GREAT* hit!");
	} else {
	    critical = ((7 * dam) / 2) + 25;
	    msg_print("It was a *SUPERB* hit!");
	}
    }
    return (critical);
}


/* Given direction "dir", returns new row, column location -RAK- */
/* targeting code stolen from Morgul -CFT */
/* 'dir=0' moves toward target				    CDW  */
int 
mmove(dir, y, x)
int                 dir;
register int       *y, *x;
{
    register int new_row = 0, new_col = 0;
    int          boolflag;

    switch (dir) {
#ifdef TARGET
      case 0:			/* targetting code stolen from Morgul -CFT */
	new_row = *y;
	new_col = *x;
	mmove2(&new_row, &new_col,
	       char_row, char_col,
	       target_row, target_col);
	break;
#endif /* TARGET */
      case 1:
	new_row = *y + 1;
	new_col = *x - 1;
	break;
      case 2:
	new_row = *y + 1;
	new_col = *x;
	break;
      case 3:
	new_row = *y + 1;
	new_col = *x + 1;
	break;
      case 4:
	new_row = *y;
	new_col = *x - 1;
	break;
      case 5:
	new_row = *y;
	new_col = *x;
	break;
      case 6:
	new_row = *y;
	new_col = *x + 1;
	break;
      case 7:
	new_row = *y - 1;
	new_col = *x - 1;
	break;
      case 8:
	new_row = *y - 1;
	new_col = *x;
	break;
      case 9:
	new_row = *y - 1;
	new_col = *x + 1;
	break;
    }
    boolflag = FALSE;
    if ((new_row >= 0) && (new_row < cur_height)
	&& (new_col >= 0) && (new_col < cur_width)) {
	*y = new_row;
	*x = new_col;
	boolflag = TRUE;
    }
    return (boolflag);
}


/* Saving throws for player character.		-RAK-	 */
int 
player_saves()
{
    /* MPW C couldn't handle the expression, so split it into two parts */
    int16 temp = class_level_adj[py.misc.pclass][CLA_SAVE];

    if (randint(100) <= (py.misc.save + stat_adj(A_WIS)
			 + (temp * py.misc.lev / 3)))
	return (TRUE);
    else
	return (FALSE);
}


/* Finds range of item in inventory list		-RAK-	 */
int 
find_range(item1, item2, j, k)
int           item1, item2;
register int *j, *k;
{
    register int         i;
    register inven_type *i_ptr;
    int                  flag;

    i = 0;
    *j = (-1);
    *k = (-1);
    flag = FALSE;
    i_ptr = &inventory[0];
    while (i < inven_ctr) {
	if (!flag) {
	    if ((i_ptr->tval == item1) || (i_ptr->tval == item2)) {
		flag = TRUE;
		*j = i;
	    }
	} else {
	    if ((i_ptr->tval != item1) && (i_ptr->tval != item2)) {
		*k = i - 1;
		break;
	    }
	}
	i++;
	i_ptr++;
    }
    if (flag && (*k == -1))
	*k = inven_ctr - 1;
    return (flag);
}


/* Teleport the player to a new location		-RAK-	 */
void 
teleport(dis)
int dis;
{
    register int y, x, count;

    do {
	count = 0;
	do {
	    count += 1;
	    y = randint(cur_height) - 1;
	    x = randint(cur_width) - 1;
	    while (distance(y, x, char_row, char_col) > dis) {
		y += ((char_row - y) / 2);
		x += ((char_col - x) / 2);
	    }
	}
	while (((cave[y][x].fval >= MIN_CLOSED_SPACE) ||
		(cave[y][x].cptr >= 2) ||
		(t_list[cave[y][x].tptr].index == OBJ_OPEN_DOOR) ||
		(cave[y][x].fval == NT_DARK_FLOOR) ||
		(cave[y][x].fval == NT_LIGHT_FLOOR)) && count < 1000);

	dis *= 2;
    } while (count == 1000);

    move_rec(char_row, char_col, y, x);

    /* unlight area teleported from */
    darken_player(char_row, char_col);

    char_row = y;
    char_col = x;
    check_view();
    creatures(FALSE);
    teleport_flag = FALSE;
}


/* Add a comment to an object description.		-CJS- */
void 
scribe_object()
{
    int   item_val, j;
    vtype out_val, tmp_str;

    if (inven_ctr > 0 || equip_ctr > 0) {
	if (get_item(&item_val, "Which one? ", 0, INVEN_ARRAY_SIZE, 0)) {
	    objdes(tmp_str, &inventory[item_val], TRUE);
	    (void)sprintf(out_val, "Inscribing %s.", tmp_str);
	    msg_print(out_val);
	    if (inventory[item_val].inscrip[0] != '\0')
		(void)sprintf(out_val, "Replace \"%s\" with the inscription: ",
			      inventory[item_val].inscrip);
	    else
		(void)strcpy(out_val, "Inscription: ");
	    j = 78 - strlen(tmp_str);
	    if (j > 12)
		j = 12;
	    prt(out_val, 0, 0);
	    if (get_string(out_val, 0, strlen(out_val), j))
		inscribe(&inventory[item_val], out_val);
	}
    } else
	msg_print("You are not carrying anything to inscribe.");
}

/* Append an additional comment to an object description.	-CJS- */
void 
add_inscribe(i_ptr, type)
inven_type *i_ptr;
int         type;
{
    i_ptr->ident |= (int8u) type;
}

/* Replace any existing comment in an object description with a new one. CJS */
void 
inscribe(i_ptr, str)
inven_type *i_ptr;
const char *str;
{
    (void)strcpy(i_ptr->inscrip, str);
}


/* We need to reset the view of things.			-CJS- */
void 
check_view()
{
    register int        i, j;
    register cave_type *c_ptr, *d_ptr;

    c_ptr = &cave[char_row][char_col];
/* Check for new panel		   */
    if (get_panel(char_row, char_col, FALSE))
	prt_map();
/* Move the light source		   */
    move_light(char_row, char_col, char_row, char_col);
/* A room of light should be lit.	 */
    if (c_ptr->fval == LIGHT_FLOOR) {
	if ((py.flags.blind < 1) && !c_ptr->pl)
	    light_room(char_row, char_col);
    }
/* In doorway of light-room?		   */
    else if (c_ptr->lr && (py.flags.blind < 1)) {
	for (i = (char_row - 1); i <= (char_row + 1); i++)
	    for (j = (char_col - 1); j <= (char_col + 1); j++) {
		d_ptr = &cave[i][j];
		if ((d_ptr->fval == LIGHT_FLOOR) && !d_ptr->pl)
		    light_room(i, j);
	    }
    }
}
