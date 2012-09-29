/* File: arrays.c */ 

/* Purpose: initialize various arrays */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "angband.h"



/*
 * Prepare the ANGBAND_xxx filepath "constants".
 *
 * First, we'll look for the ANGBAND_PATH environment variable,
 * and then look for the files in there.  If that doesn't work,
 * we'll try the DEFAULT_PATH constant.  So be sure that one of
 * these two things works...
 *
 * The code is now a lot cleaner, with no realloc calls. -BEN-
 * And it is no longer optional, everyone must use it.  If you
 * wish to reinstate "constant paths", please do so by giving
 * the variables below constant "initial values".
 */

cptr ANGBAND_DIR_FILES = NULL;		/* Dir: ascii files  */
cptr ANGBAND_DIR_BONES = NULL;		/* Dir: ascii bones files */
cptr ANGBAND_DIR_SAVE = NULL;		/* Dir: binary save files */
cptr ANGBAND_DIR_DATA = NULL;		/* Dir: system dependant files */

cptr ANGBAND_NEWS = NULL;		/* News file */
cptr ANGBAND_WELCOME = NULL;		/* Player generation help */
cptr ANGBAND_VERSION = NULL;		/* Version information */

cptr ANGBAND_WIZ = NULL;		/* Acceptable wizard uid's */
cptr ANGBAND_HOURS = NULL;		/* Hours of operation */
cptr ANGBAND_LOAD = NULL;		/* Load information */
cptr ANGBAND_LOG = NULL;		/* Log file of some form */

cptr ANGBAND_R_HELP = NULL;		/* Roguelike command help */
cptr ANGBAND_O_HELP = NULL;		/* Original command help */
cptr ANGBAND_RWIZ_HELP = NULL;		/* Roguelike Wiz-cmd help */
cptr ANGBAND_OWIZ_HELP = NULL;		/* Original Wiz-cmd help */

cptr ANGBAND_K_LIST = NULL;		/* Ascii item kind file */
cptr ANGBAND_R_LIST = NULL;		/* Ascii monster race file */



/*
 * Find the paths to all of our important files and directories...
 * Use the ANGBAND_PATH environment var if possible, else use DEFAULT_PATH,
 * and then branch off appropriately from there (see below).
 *
 * Note that this function is called BEFORE Term_init().
 *
 * If your system can't do "getenv()", you'll have to kludge this.  [cjh]
 * For example, on "MACINTOSH", we set a global "folder pointer" based on
 * where the "lib" folder was last time, and if we cannot use that one,
 * we ask the user to interactively "reset" it.  See "main-mac.c".
 */
void get_file_paths()
{
    /* The current path (and "workspace") */
    char path[1024];

    /* Pointer to the "end" of the workspace */
    char *tail;

    /* Grab the base "path" */
    char *angband_path = NULL;


#ifdef MACINTOSH

    /* Hack -- The Macintosh uses a "system" path-prefix */
    strcpy(path, PATH_SEP);

#else

    /* Get the environment variable */
    angband_path = getenv("ANGBAND_PATH");

    /* Use the angband_path, or a default */
    strcpy(path, angband_path ? angband_path : DEFAULT_PATH);

    /* Be sure not to duplicate any "Path separator" */
    if (!suffix(path,PATH_SEP)) strcat(path, PATH_SEP);

#endif


    /* Prepare to append to the Base Path */
    tail = path + strlen(path);

    /* Find some directory names */
    strcpy(tail, "data");
    ANGBAND_DIR_DATA = string_make(path);
    strcpy(tail, "save");
    ANGBAND_DIR_SAVE = string_make(path);
    strcpy(tail, "bones");
    ANGBAND_DIR_BONES = string_make(path);
    strcpy(tail, "files");
    ANGBAND_DIR_FILES = string_make(path);

    /* Add a path separator */
    strcat(tail, PATH_SEP);

    /* Use the "files" directory (from above) */
    tail = tail + strlen(tail);

    /* The basic info files */
    strcpy(tail, "news.hlp");
    ANGBAND_NEWS = string_make(path);
    strcpy(tail, "welcome.hlp");
    ANGBAND_WELCOME = string_make(path);
    strcpy(tail, "version.hlp");
    ANGBAND_VERSION = string_make(path);

    /* The command help files */
    strcpy(tail, "rstdcmds.hlp");
    ANGBAND_R_HELP = string_make(path);
    strcpy(tail, "ostdcmds.hlp");
    ANGBAND_O_HELP = string_make(path);
    strcpy(tail, "rwizcmds.hlp");
    ANGBAND_RWIZ_HELP = string_make(path);
    strcpy(tail, "owizcmds.hlp");
    ANGBAND_OWIZ_HELP = string_make(path);

    /* Some parsable text files */
    strcpy(tail, "wizards.txt");
    ANGBAND_WIZ = string_make(path);
    strcpy(tail, "hours.txt");
    ANGBAND_HOURS = string_make(path);
    strcpy(tail, "load.txt");
    ANGBAND_LOAD = string_make(path);

    /* Parsable Item/Monster template files */
    strcpy(tail, "k_list.txt");
    ANGBAND_K_LIST = string_make(path);
    strcpy(tail, "r_list.txt");
    ANGBAND_R_LIST = string_make(path);
}







/*
 * Grab one flag in a monster_race from a textual string
 *
 * The following substitutions were used to extract the
 * function below from the "dump" function (elsewhere).
 *
 *   s:^.*(\(flags.\) \& \(.*\)).* " \| \(.*\)");.*$:\1 \2 \3:g
 *   s:\(.*\) \(.*\) \(.*\):    else if (streq(what,"\3")) \1 |= \2;:g
 */
static int grab_one_flag(monster_race *r_ptr, cptr what)
{
    int32u flags1 = 0L, flags2 = 0L;

    if (!what) what = what;

    else if (streq(what,"WINNER"))	flags1 |= MF1_WINNER;
    else if (streq(what,"QUESTOR"))	flags2 |= MF2_QUESTOR;
    else if (streq(what,"UNIQUE"))	flags2 |= MF2_UNIQUE;
    else if (streq(what,"MAX_HP"))	flags2 |= MF2_MAX_HP;
    else if (streq(what,"INTELLIGENT"))	flags2 |= MF2_INTELLIGENT;

    else if (streq(what,"SPECIAL"))	flags2 |= MF2_SPECIAL;
    else if (streq(what,"GOOD"))	flags2 |= MF2_GOOD;
    else if (streq(what,"CARRY_GOLD"))	flags1 |= MF1_CARRY_GOLD;
    else if (streq(what,"CARRY_OBJ"))	flags1 |= MF1_CARRY_OBJ;
    else if (streq(what,"PICK_UP"))	flags1 |= MF1_PICK_UP;

    else if (streq(what,"MV_ONLY_ATT"))	flags1 |= MF1_MV_ONLY_ATT;
    else if (streq(what,"MV_ATT_NORM"))	flags1 |= MF1_MV_ATT_NORM;
    else if (streq(what,"MV_20"))	flags1 |= MF1_MV_20;
    else if (streq(what,"MV_40"))	flags1 |= MF1_MV_40;
    else if (streq(what,"MV_75"))	flags1 |= MF1_MV_75;
    else if (streq(what,"MV_INVIS"))	flags1 |= MF1_MV_INVIS;
    else if (streq(what,"NO_INFRA"))	flags2 |= MF2_NO_INFRA;
    else if (streq(what,"MINDLESS"))	flags2 |= MF2_MINDLESS;
    else if (streq(what,"THRO_DR"))	flags1 |= MF1_THRO_DR;
    else if (streq(what,"THRO_WALL"))	flags1 |= MF1_THRO_WALL;
    else if (streq(what,"THRO_CREAT"))	flags1 |= MF1_THRO_CREAT;
    else if (streq(what,"MULTIPLY"))	flags1 |= MF1_MULTIPLY;
    else if (streq(what,"GROUP"))	flags2 |= MF2_GROUP;
    else if (streq(what,"HAS_60"))	flags1 |= MF1_HAS_60;
    else if (streq(what,"HAS_90"))	flags1 |= MF1_HAS_90;
    else if (streq(what,"HAS_1D2"))	flags1 |= MF1_HAS_1D2;
    else if (streq(what,"HAS_2D2"))	flags1 |= MF1_HAS_2D2;
    else if (streq(what,"HAS_4D2"))	flags1 |= MF1_HAS_4D2;

    else if (streq(what,"ANIMAL"))	flags2 |= MF2_ANIMAL;
    else if (streq(what,"EVIL"))	flags2 |= MF2_EVIL;
    else if (streq(what,"ORC"))	flags2 |= MF2_ORC;
    else if (streq(what,"TROLL"))	flags2 |= MF2_TROLL;
    else if (streq(what,"GIANT"))	flags2 |= MF2_GIANT;
    else if (streq(what,"DRAGON"))	flags2 |= MF2_DRAGON;
    else if (streq(what,"DEMON"))	flags2 |= MF2_DEMON;
    else if (streq(what,"UNDEAD"))	flags2 |= MF2_UNDEAD;

    else if (streq(what,"IM_ACID"))	flags2 |= MF2_IM_ACID;
    else if (streq(what,"IM_FIRE"))	flags2 |= MF2_IM_FIRE;
    else if (streq(what,"IM_COLD"))	flags2 |= MF2_IM_COLD;
    else if (streq(what,"IM_ELEC"))	flags2 |= MF2_IM_ELEC;
    else if (streq(what,"IM_POIS"))	flags2 |= MF2_IM_POIS;

    else if (streq(what,"HURT_LITE"))	flags2 |= MF2_HURT_LITE;
    else if (streq(what,"HURT_ROCK"))	flags2 |= MF2_HURT_ROCK;
    else if (streq(what,"CHARM_SLEEP"))	flags2 |= MF2_CHARM_SLEEP;
    else if (streq(what,"BREAK_WALL"))	flags2 |= MF2_BREAK_WALL;
    else if (streq(what,"DESTRUCT"))	flags2 |= MF2_DESTRUCT;

    else if (streq(what,"MALE"))	flags1 |= MF1_MALE;
    else if (streq(what,"FEMALE"))	flags1 |= MF1_FEMALE;
    else if (streq(what,"PLURAL"))	flags1 |= MF1_PLURAL;
    
    else if (streq(what,"CHAR_CLEAR"))	flags1 |= MF1_CHAR_CLEAR;
    else if (streq(what,"CHAR_MULTI"))	flags1 |= MF1_CHAR_MULTI;
    else if (streq(what,"ATTR_CLEAR"))	flags1 |= MF1_ATTR_CLEAR;
    else if (streq(what,"ATTR_MULTI"))	flags1 |= MF1_ATTR_MULTI;

    if (!flags1 && !flags2) return (0);

    if (flags1) r_ptr->cflags1 |= flags1;
    if (flags2) r_ptr->cflags2 |= flags2;

    return (1);
}


/*
 * Grab one spell in a monster_race from a textual string
 */
static int grab_one_spell(monster_race *r_ptr, cptr what)
{
    int32u flags1 = 0L, flags2 = 0L, flags3 = 0L;

    int chance;

    if (!what) what = what;

    /* Hack -- store the "frequency" in the spell flags */
    else if (1 == sscanf(what, "1_IN_%d", &chance)) {
    
	/* Hack -- frequency stored as "flags" */
	flags1 |= (chance & CS1_FREQ);
    }

    else if (streq(what,"HEAL"))		flags2 |= MS2_HEAL;
    else if (streq(what,"HASTE"))		flags2 |= MS2_HASTE;
    else if (streq(what,"BLINK"))		flags1 |= MS1_BLINK;
    else if (streq(what,"TELEPORT"))		flags1 |= MS1_TELEPORT;

    else if (streq(what,"TELE_TO"))		flags1 |= MS1_TELE_TO;
    else if (streq(what,"TELE_AWAY"))		flags2 |= MS2_TELE_AWAY;
    else if (streq(what,"TELE_LEVEL"))	flags2 |= MS2_TELE_LEVEL;
    else if (streq(what,"BLIND"))		flags1 |= MS1_BLIND;
    else if (streq(what,"HOLD"))		flags1 |= MS1_HOLD;
    else if (streq(what,"SLOW"))		flags1 |= MS1_SLOW;
    else if (streq(what,"CONF"))		flags1 |= MS1_CONF;
    else if (streq(what,"FEAR"))		flags1 |= MS1_FEAR;

    else if (streq(what,"CAUSE_1"))		flags1 |= MS1_CAUSE_1;
    else if (streq(what,"CAUSE_2"))		flags1 |= MS1_CAUSE_2;
    else if (streq(what,"CAUSE_3"))		flags1 |= MS1_CAUSE_3;
    else if (streq(what,"ARROW_1"))		flags1 |= MS1_ARROW_1;
    else if (streq(what,"ARROW_2"))		flags2 |= MS2_ARROW_2;
    else if (streq(what,"ARROW_3"))		flags3 |= MS3_ARROW_3;
    else if (streq(what,"RAZOR"))		flags2 |= MS2_RAZOR;

    else if (streq(what,"MANA_DRAIN"))	flags1 |= MS1_MANA_DRAIN;
    else if (streq(what,"MIND_BLAST"))	flags2 |= MS2_MIND_BLAST;
    else if (streq(what,"BRAIN_SMASH"))	flags2 |= MS2_BRAIN_SMASH;
    else if (streq(what,"FORGET"))		flags2 |= MS2_FORGET;
    else if (streq(what,"TRAP_CREATE"))	flags2 |= MS2_TRAP_CREATE;
    else if (streq(what,"DARKNESS"))		flags2 |= MS2_DARKNESS;
    else if (streq(what,"DARK_STORM"))	flags3 |= MS3_DARK_STORM;
    else if (streq(what,"MANA_STORM"))	flags3 |= MS3_MANA_STORM;

    else if (streq(what,"BO_ACID"))		flags1 |= MS1_BO_ACID;
    else if (streq(what,"BO_FIRE"))		flags1 |= MS1_BO_FIRE;
    else if (streq(what,"BO_COLD"))		flags1 |= MS1_BO_COLD;
    else if (streq(what,"BO_ELEC"))		flags2 |= MS2_BO_ELEC;
    else if (streq(what,"BO_ICEE"))		flags2 |= MS2_BO_ICEE;
    else if (streq(what,"BO_WATE"))		flags2 |= MS2_BO_WATE;
    else if (streq(what,"BO_MANA"))		flags1 |= MS1_BO_MANA;
    else if (streq(what,"BO_PLAS"))		flags2 |= MS2_BO_PLAS;
    else if (streq(what,"BO_NETH"))		flags2 |= MS2_BO_NETH;

    else if (streq(what,"BA_ACID"))		flags2 |= MS2_BA_ACID;
    else if (streq(what,"BA_FIRE"))		flags1 |= MS1_BA_FIRE;
    else if (streq(what,"BA_COLD"))		flags1 |= MS1_BA_COLD;
    else if (streq(what,"BA_ELEC"))		flags2 |= MS2_BA_ELEC;
    else if (streq(what,"BA_WATE"))		flags2 |= MS2_BA_WATE;
    else if (streq(what,"BA_POIS"))		flags2 |= MS2_BA_POIS;
    else if (streq(what,"BA_NETH"))		flags2 |= MS2_BA_NETH;

    else if (streq(what,"BR_ACID"))		flags1 |= MS1_BR_ACID;
    else if (streq(what,"BR_FIRE"))		flags1 |= MS1_BR_FIRE;
    else if (streq(what,"BR_COLD"))		flags1 |= MS1_BR_COLD;
    else if (streq(what,"BR_ELEC"))		flags1 |= MS1_BR_ELEC;
    else if (streq(what,"BR_POIS"))		flags1 |= MS1_BR_POIS;
    else if (streq(what,"BR_LITE"))		flags3 |= MS3_BR_LITE;
    else if (streq(what,"BR_DARK"))		flags3 |= MS3_BR_DARK;
    else if (streq(what,"BR_SOUN"))		flags2 |= MS2_BR_SOUN;
    else if (streq(what,"BR_CONF"))		flags2 |= MS2_BR_CONF;
    else if (streq(what,"BR_CHAO"))		flags2 |= MS2_BR_CHAO;
    else if (streq(what,"BR_SHAR"))		flags2 |= MS2_BR_SHAR;
    else if (streq(what,"BR_LIFE"))		flags2 |= MS2_BR_LIFE;
    else if (streq(what,"BR_DISE"))		flags2 |= MS2_BR_DISE;
    else if (streq(what,"BR_WALL"))		flags3 |= MS3_BR_WALL;
    else if (streq(what,"BR_SLOW"))		flags3 |= MS3_BR_SLOW;
    else if (streq(what,"BR_TIME"))		flags3 |= MS3_BR_TIME;
    else if (streq(what,"BR_GRAV"))		flags3 |= MS3_BR_GRAV;
    else if (streq(what,"BR_PLAS"))		flags3 |= MS3_BR_PLAS;
    else if (streq(what,"BR_NETH"))		flags2 |= MS2_BR_NETH;

    else if (streq(what,"S_MONSTER"))		flags1 |= MS1_S_MONSTER;
    else if (streq(what,"S_SUMMON"))		flags2 |= MS2_S_SUMMON;
    else if (streq(what,"S_UNDEAD"))		flags1 |= MS1_S_UNDEAD;
    else if (streq(what,"S_DEMON"))		flags1 |= MS1_S_DEMON;
    else if (streq(what,"S_DRAGON"))		flags1 |= MS1_S_DRAGON;
    else if (streq(what,"S_ANGEL"))		flags2 |= MS2_S_ANGEL;
    else if (streq(what,"S_REPTILE"))		flags3 |= MS3_S_REPTILE;
    else if (streq(what,"S_SPIDER"))		flags2 |= MS2_S_SPIDER;
    else if (streq(what,"S_ANT"))		flags3 |= MS3_S_ANT;
    else if (streq(what,"S_HOUND"))		flags2 |= MS2_S_HOUND;
    else if (streq(what,"S_UNIQUE"))		flags3 |= MS3_S_UNIQUE;
    else if (streq(what,"S_WRAITH"))		flags3 |= MS3_S_WRAITH;
    else if (streq(what,"S_GUNDEAD"))		flags3 |= MS3_S_GUNDEAD;
    else if (streq(what,"S_ANCIENTD"))	flags3 |= MS3_S_ANCIENTD;

    if (!flags1 && !flags2 && !flags3) return (0);

    if (flags1) r_ptr->spells1 |= flags1;
    if (flags2) r_ptr->spells2 |= flags2;
    if (flags3) r_ptr->spells3 |= flags3;

    return (1);
}


/*
 * Convert a "color letter" into an actual color
 * The colors are: dwsorgbuDWvyRGBU, see below
 * No longer includes MULTI or CLEAR
 */
static int color_char_to_attr(char c)
{
    switch (c) {

	case 'd': return (COLOR_BLACK);
	case 'w': return (COLOR_WHITE);
	case 's': return (COLOR_GRAY);
	case 'o': return (COLOR_ORANGE);
	case 'r': return (COLOR_RED);
	case 'g': return (COLOR_GREEN);
	case 'b': return (COLOR_BLUE);
	case 'u': return (COLOR_BROWN);

	case 'D': return (COLOR_D_GRAY);
	case 'W': return (COLOR_L_GRAY);
	case 'v': return (COLOR_VIOLET);
	case 'y': return (COLOR_YELLOW);
	case 'R': return (COLOR_L_RED);
	case 'G': return (COLOR_L_GREEN);
	case 'B': return (COLOR_L_BLUE);
	case 'U': return (COLOR_L_BROWN);
    }

    return (-1);
}



/*
 * Hack -- indicate errors during parsing of "r_list.txt"
 */
static int error_r_idx = -1;

/*
 * Initialize the "r_list" array by parsing an ascii file
 */
static errr init_r_list_txt(void)
{
    register char *s, *t;

    /* No monster yet */
    int m = -1;

    /* No r_ptr yet */
    monster_race *r_ptr = NULL;

    /* The "monsters" file */
    FILE *fp;

    /* No line should be more than 80 chars */
    char buf[160];

    /* Current race description */
    char desc[24*80];

    /* Open the monster file */
    fp = fopen(ANGBAND_R_LIST, "r");

    /* Failure */
    if (!fp) return (-1);

    /* Load the monster descriptions from the file */
    while (1) {

	/* Read a line from the file, stop when done */
	if (!fgets(buf, 160, fp)) break;

	/* Skip comments */
	if (buf[0] == '#') continue;

	/* Strip the final newline */
	for (s = buf; isprint(*s); ++s); *s = '\0';

	/* Blank lines terminate monsters */
	if (!buf[0]) {

	    /* No current r_ptr */
	    if (!r_ptr) continue;

	    /* Save the decription */
	    if (desc[0]) r_ptr->desc = string_make(desc);

	    /* Now there is no current r_ptr */
	    r_ptr = NULL;

	    /* Next... */
	    continue;
	}

	/* The line better have a colon and such */
	if (buf[1] != ':') return (1);

	/* Process 'N' for "New/Number/Name" */
	if (buf[0] == 'N') {

	    /* Not done the previous one */
	    if (r_ptr) return (11);

	    /* Find, verify, and nuke the colon before the name */
	    if (!(s = strchr(buf+2, ':'))) return (2);

	    /* Nuke the colon, advance to the name */
	    *s++ = '\0';

	    /* Require non-empty names */
	    if (!*s) return (3);

	    /* Get the index */
	    m = atoi(buf+2);

	    /* Save the index */
	    error_r_idx = m;

	    /* Verify */
	    if ((m < 0) || (m >= MAX_R_IDX)) return (7);

	    /* Start a new r_ptr */
	    r_ptr = &r_list[m];

	    /* Make sure we have not done him yet */
	    if (r_ptr->name) return (8);

	    /* Save the name */
	    r_ptr->name = string_make(s);

	    /* No desc yet */
	    desc[0] = '\0';

	    /* Next... */
	    continue;
	}

	/* There better be a current r_ptr */
	if (!r_ptr) return (10);

	/* Process 'I' for "Info" (one line only) */
	if (buf[0] == 'I') {

	    char chr, att;
	    int tmp, spd, hp1, hp2, aaf, ac, slp, rar, lev;
	    long exp;

	    /* Scan for the values */
	    if (11 != sscanf(buf, "I:%c:%c:%d:%dd%d:%d:%d:%d:%d:%d:%ld",
		&chr, &att, &spd, &hp1, &hp2,
		&aaf, &ac, &slp, &rar, &lev, &exp)) return (11);

	    /* Extract the color */
	    tmp = color_char_to_attr(att);
	    if (tmp < 0) return (12);
	    r_ptr->r_attr = tmp;

	    /* Save the values */
	    r_ptr->r_char = chr;
	    r_ptr->speed = 10 + spd;	/* Hack */
	    r_ptr->hd[0] = hp1;
	    r_ptr->hd[1] = hp2;
	    r_ptr->aaf = aaf;
	    r_ptr->ac = ac;
	    r_ptr->sleep = slp;
	    r_ptr->rarity = rar;
	    r_ptr->level = lev;
	    r_ptr->mexp = exp;

	    /* Next... */
	    continue;
	}

	/* Process 'A' for "Attacks" (one line only) */
	if (buf[0] == 'A') {

	    int i;

	    /* Simply read each number following a colon */
	    for (i = 0, s = buf+1; s && (s[0] == ':') && s[1]; ++i) {

		/* Store the attack damage index */
		r_ptr->damage[i] = atoi(s+1);

		/* Find the next colon */
		s = strchr(s+1, ':');
	    }

	    /* Next... */
	    continue;
	}

	/* Process 'F' for "Flags" (multiple lines) */
	if (buf[0] == 'F') {

	    /* Parse every entry */
	    for (s = buf + 2; *s; ) {

		/* Find the end of this entry */
		for (t = s; *t && *t != ' ' && *t != '|'; ++t);

		/* Nuke and skip any dividers */
		if (*t) {
		    *t++ = '\0';
		    while (*t == ' ' || *t == '|') t++;
		}

		/* Parse this entry */
		if (!grab_one_flag(r_ptr, s)) return (18);

		/* Start the next entry */
		s = t;
	    }

	    /* Next... */
	    continue;
	}

	/* Process 'S' for "Spells" (multiple lines) */
	if (buf[0] == 'S') {

	    /* Parse every entry */
	    for (s = buf + 2; *s; ) {

		/* Find the end of this entry */
		for (t = s; *t && *t != ' ' && *t != '|'; ++t);

		/* Nuke and skip any dividers */
		if (*t) {
		    *t++ = '\0';
		    while (*t == ' ' || *t == '|') t++;
		}

		/* Parse this entry */
		if (!grab_one_spell(r_ptr, s)) return (19);

		/* Start the next entry */
		s = t;
	    }

	    /* Next... */
	    continue;
	}

	/* Process 'D' for "Description" */
	if (buf[0] == 'D') {

	    /* Collect the description, allocated later */
	    strcat(desc,buf+2);

	    /* Next... */
	    continue;
	}
    }

    /* Close the file */
    fclose(fp);


    /* Success */
    return (0);
}


/*
 * Attempt to "dump" a "quick-load binary image" for "r_list"
 */
static errr dump_r_list_raw()
{

#ifdef BINARY_ARRAY_IMAGES

    int i, fd;

    /* One race at a time */
    monster_race *r_ptr;

    /* Do not save the name or desc fields */
    uint size = sizeof(monster_race) - 2 * sizeof(cptr);
    
    int mode = 0666;
    
    int16u tmp2;

    char tmp[1024];
    char buf[2048];

    /* Use the "fast" binary version whenever possible */
    sprintf(tmp, "%s%s%s", ANGBAND_DIR_DATA, PATH_SEP, "r_list.raw");

#ifdef SET_UID
    mode = 0644;
#endif

#ifdef MACINTOSH
    _ftype = 'DATA';
#endif

    /* Create a new file */
    fd = my_topen(tmp, O_RDWR | O_CREAT | O_BINARY, mode);

#ifdef MACINTOSH
    _ftype = 'SAVE';
#endif

    /* Failure to make */
    if (fd < 0) return (-1);


    /* Dump the version info */
    buf[0] = CUR_VERSION_MAJ;
    buf[1] = CUR_VERSION_MIN; 
    buf[2] = CUR_PATCH_LEVEL;
    buf[3] = 0; /* Platform */

    /* Dump it, ignore errors */
    write(fd, buf, 4);

           
    /* Attempt to dump the (normal) monster races */
    for (i = 0; i < MAX_R_IDX-1; i++) {

	/* Access the monster race */
	r_ptr = &r_list[i];

	/* Load the main record */
	if (size != write(fd, (char*)(r_ptr), size)) break;

	/* Dump the monster name (if any) */
	if (r_ptr->name) {
	    strcpy(buf, r_ptr->name);
	    tmp2 = strlen(buf) + 1;
	    if (2 != write(fd, (char*)(&tmp2), 2)) break;
	    if (tmp2 != write(fd, buf, tmp2)) break;
	}
	else {
	    tmp2 = 0;
	    if (2 != write(fd, (char*)(&tmp2), 2)) break;
	}

	/* Dump the monster desc (if any) */
	if (r_ptr->desc) {
		strcpy(buf, r_ptr->desc);
		tmp2 = strlen(buf) + 1;
		if (2 != write(fd, (char*)(&tmp2), 2)) break;
		if (tmp2 != write(fd, buf, tmp2)) break;
	}
	else {
	    tmp2 = 0;
	    if (2 != write(fd, (char*)(&tmp2), 2)) break;
	}
    }
    
    /* Close it */
    close(fd);

    /* Failure */
    if (i < MAX_R_IDX-1) {
	message("Warning: failure writing 'r_list.raw'", 0);
	message(NULL, 0);
    }
    
#endif

    /* Success */
    return (0);
}



 
/*
 * Attempt to "quick-load" a binary image for "r_list"
 */
static errr init_r_list_raw()
{

#ifdef BINARY_ARRAY_IMAGES

    int i, fd;

    /* One race at a time */
    monster_race *r_ptr;

    /* Do not save the name or desc fields */
    uint size = sizeof(monster_race) - 2 * sizeof(cptr);
    
    int mode = 0666;
    
    int16u tmp2;

    char tmp[1024];
    char buf[2048];


    /* Use the "fast" binary version whenever possible */
    sprintf(tmp, "%s%s%s", ANGBAND_DIR_DATA, PATH_SEP, "r_list.raw");

#ifdef SET_UID
    mode = 0644;
#endif

#ifdef MACINTOSH
    _ftype = 'DATA';
#endif

    /* Open a read-only copy */
    fd = my_topen(tmp, O_RDONLY | O_BINARY, mode);

#ifdef MACINTOSH
    _ftype = 'SAVE';
#endif

    /* Failure to open */
    if (fd < 0) return (-1);

    /* Read the version info */
    if ((4 != read(fd, buf, 4)) ||
	(buf[0] != CUR_VERSION_MAJ) ||
	(buf[1] != CUR_VERSION_MIN) ||
	(buf[2] != CUR_PATCH_LEVEL) ||
	(buf[0] != 0)) {

	/* Close and Fail (cleanly) */
	close(fd);
	return (-1);
    }
        
    /* Attempt to read the (normal) monster races */
    for (i = 0; i < MAX_R_IDX-1; i++) {

	/* Access the monster race */
	r_ptr = &r_list[i];

	/* Load the main record */
	if (size != read(fd, (char*)(r_ptr), size)) break;

	/* Hack -- Load the monster name */
	r_ptr->name = NULL;
	if (2 != read(fd, (char*)(&tmp2), 2)) break;
	if (tmp2) {
	    if (tmp2 != read(fd, buf, tmp2)) break;
	    r_ptr->name = string_make(buf);
	}

	/* Hack -- Load the monster desc */
	r_ptr->desc = NULL;
	if (2 != read(fd, (char*)(&tmp2), 2)) break;
	if (tmp2) {
	    if (tmp2 != read(fd, buf, tmp2)) break;
	    r_ptr->desc = string_make(buf);
	}
    }

    /* Close the file */
    close(fd);

    /* Aborted load */
    if (i < MAX_R_IDX-1) {
	message("Major error loading 'r_list.raw'", 0);
	message("Try removing it from the 'data' directory", 0);
	message(NULL, 0);
	quit("cannot load 'r_list.raw'");
    }

#endif

    /* Success */
    return (0);
}


/*
 * Note that r_list now includes a description
 * Note that "r_list" starts out totally cleared
 */
static void init_r_list()
{
    errr err;

    /* Ghost */
    monster_race *r_ptr;

    char tmp[1024];


    /* XXX Hack -- prepare "ghost" race */

    /* Prepare the "player ghost" */
    r_ptr = &r_list[MAX_R_IDX-1];

    /* Hack -- Give the ghost monster a "fake" name */
    r_ptr->name = ghost_name;

    /* Hack -- Prepare a fake ghost name */
    strcpy(ghost_name, "Someone's Ghost");

    /* Hack -- Try to prevent a few "potential" bugs */
    r_ptr->cflags2 |= MF2_UNIQUE;


    /* Try to load a "binary image" */
    err = init_r_list_raw();

    /* Otherwise try the text version */
    if (err) err = init_r_list_txt();

    /* Still no luck? Fail! */
    if (err) {
	sprintf(tmp, "Fatal error #%d parsing 'r_list.txt', record %d",
		err, error_r_idx);
	message(tmp, 0);
	message(NULL,0);
	quit("cannot load 'r_list.txt'");
    }

    /* Attempt to dump a "quick-load" version */
    err = dump_r_list_raw();
    if (err) {
	message("Warning: unable to create 'r_list.raw'", 0);
    }
}






/*
 * Grab one flag in a inven_kind from a textual string
 */
static bool grab_one_kind_flag(inven_kind *k_ptr, cptr what)
{
    int32u flags1 = 0L, flags2 = 0L, flags3 = 0L;

    if (!what) what = what;

    else if (streq(what, "STR"))		flags1 |= TR1_STR;
    else if (streq(what, "INT"))		flags1 |= TR1_INT;
    else if (streq(what, "WIS"))		flags1 |= TR1_WIS;
    else if (streq(what, "DEX"))		flags1 |= TR1_DEX;
    else if (streq(what, "CON"))		flags1 |= TR1_CON;
    else if (streq(what, "CHR"))		flags1 |= TR1_CHR;

    else if (streq(what, "SEARCH"))		flags1 |= TR1_SEARCH;
    else if (streq(what, "SPEED"))		flags1 |= TR1_SPEED;
    else if (streq(what, "STEALTH"))		flags1 |= TR1_STEALTH;
    else if (streq(what, "TUNNEL"))		flags1 |= TR1_TUNNEL;
    else if (streq(what, "INFRA"))		flags1 |= TR1_INFRA;
    else if (streq(what, "ATTACK_SPD"))		flags1 |= TR1_ATTACK_SPD;

    else if (streq(what, "KILL_DRAGON"))	flags1 |= TR1_KILL_DRAGON;
    else if (streq(what, "SLAY_DRAGON"))	flags1 |= TR1_SLAY_DRAGON;
    else if (streq(what, "SLAY_ANIMAL"))	flags1 |= TR1_SLAY_ANIMAL;
    else if (streq(what, "SLAY_EVIL"))		flags1 |= TR1_SLAY_EVIL;

    else if (streq(what, "IMPACT"))		flags1 |= TR1_IMPACT;
    else if (streq(what, "BRAND_COLD"))		flags1 |= TR1_BRAND_COLD;
    else if (streq(what, "BRAND_FIRE"))		flags1 |= TR1_BRAND_FIRE;
    else if (streq(what, "BRAND_ELEC"))		flags1 |= TR1_BRAND_ELEC;

    else if (streq(what, "SLAY_UNDEAD"))	flags1 |= TR1_SLAY_UNDEAD;
    else if (streq(what, "SLAY_DEMON"))		flags1 |= TR1_SLAY_DEMON;

    else if (streq(what, "SLAY_TROLL"))		flags1 |= TR1_SLAY_TROLL;
    else if (streq(what, "SLAY_GIANT"))		flags1 |= TR1_SLAY_GIANT;
    else if (streq(what, "SLAY_ORC"))		flags1 |= TR1_SLAY_ORC;

    else if (streq(what, "FREE_ACT"))		flags2 |= TR2_FREE_ACT;
    else if (streq(what, "HOLD_LIFE"))		flags2 |= TR2_HOLD_LIFE;


    else if (streq(what, "IM_FIRE"))		flags2 |= TR2_IM_FIRE;
    else if (streq(what, "IM_COLD"))		flags2 |= TR2_IM_COLD;
    else if (streq(what, "IM_ACID"))		flags2 |= TR2_IM_ACID;
    else if (streq(what, "IM_ELEC"))		flags2 |= TR2_IM_ELEC;
    else if (streq(what, "IM_POIS"))		flags2 |= TR2_IM_POIS;


    else if (streq(what, "RES_CONF"))		flags2 |= TR2_RES_CONF;
    else if (streq(what, "RES_SOUND"))		flags2 |= TR2_RES_SOUND;
    else if (streq(what, "RES_LITE"))		flags2 |= TR2_RES_LITE;
    else if (streq(what, "RES_DARK"))		flags2 |= TR2_RES_DARK;
    else if (streq(what, "RES_CHAOS"))		flags2 |= TR2_RES_CHAOS;
    else if (streq(what, "RES_DISEN"))		flags2 |= TR2_RES_DISEN;
    else if (streq(what, "RES_SHARDS"))		flags2 |= TR2_RES_SHARDS;
    else if (streq(what, "RES_NEXUS"))		flags2 |= TR2_RES_NEXUS;
    else if (streq(what, "RES_BLIND"))		flags2 |= TR2_RES_BLIND;
    else if (streq(what, "RES_NETHER"))		flags2 |= TR2_RES_NETHER;

    else if (streq(what, "RES_POIS"))		flags2 |= TR2_RES_POIS;
    else if (streq(what, "RES_FIRE"))		flags2 |= TR2_RES_FIRE;
    else if (streq(what, "RES_COLD"))		flags2 |= TR2_RES_COLD;
    else if (streq(what, "RES_ELEC"))		flags2 |= TR2_RES_ELEC;
    else if (streq(what, "RES_ACID"))		flags2 |= TR2_RES_ACID;

    else if (streq(what, "SUST_STR"))		flags2 |= TR2_SUST_STR;
    else if (streq(what, "SUST_INT"))		flags2 |= TR2_SUST_INT;
    else if (streq(what, "SUST_WIS"))		flags2 |= TR2_SUST_WIS;
    else if (streq(what, "SUST_DEX"))		flags2 |= TR2_SUST_DEX;
    else if (streq(what, "SUST_CON"))		flags2 |= TR2_SUST_CON;
    else if (streq(what, "SUST_CHR"))		flags2 |= TR2_SUST_CHR;

    else if (streq(what, "EASY_KNOW"))		flags3 |= TR3_EASY_KNOW;
    else if (streq(what, "HIDE_TYPE"))		flags3 |= TR3_HIDE_TYPE;
    else if (streq(what, "SHOW_MODS"))		flags3 |= TR3_SHOW_MODS;
    else if (streq(what, "SHOW_BASE"))		flags3 |= TR3_SHOW_BASE;

    else if (streq(what, "FEATHER"))		flags3 |= TR3_FEATHER;
    else if (streq(what, "LITE"))		flags3 |= TR3_LITE;
    else if (streq(what, "SEE_INVIS"))		flags3 |= TR3_SEE_INVIS;
    else if (streq(what, "TELEPATHY"))		flags3 |= TR3_TELEPATHY;

    else if (streq(what, "SLOW_DIGEST"))	flags3 |= TR3_SLOW_DIGEST;
    else if (streq(what, "REGEN"))		flags3 |= TR3_REGEN;

    else if (streq(what, "XTRA_MIGHT"))		flags3 |= TR3_XTRA_MIGHT;
    else if (streq(what, "XTRA_SHOTS"))		flags3 |= TR3_XTRA_SHOTS;

    else if (streq(what, "IGNORE_FIRE"))	flags3 |= TR3_IGNORE_FIRE;
    else if (streq(what, "IGNORE_COLD"))	flags3 |= TR3_IGNORE_COLD;
    else if (streq(what, "IGNORE_ELEC"))	flags3 |= TR3_IGNORE_ELEC;
    else if (streq(what, "IGNORE_ACID"))	flags3 |= TR3_IGNORE_ACID;

    else if (streq(what, "ACTIVATE"))		flags3 |= TR3_ACTIVATE;
    else if (streq(what, "DRAIN_EXP"))		flags3 |= TR3_DRAIN_EXP;
    else if (streq(what, "TELEPORT"))		flags3 |= TR3_TELEPORT;
    else if (streq(what, "AGGRAVATE"))		flags3 |= TR3_AGGRAVATE;

    else if (streq(what, "BLESSED"))		flags3 |= TR3_BLESSED;
    else if (streq(what, "CURSED"))		flags3 |= TR3_CURSED;
    else if (streq(what, "HEAVY_CURSE"))	flags3 |= TR3_HEAVY_CURSE;
    else if (streq(what, "PERMA_CURSE"))	flags3 |= TR3_PERMA_CURSE;

    if (!flags1 && !flags2 && !flags3) return (FALSE);

    if (flags1) k_ptr->flags1 |= flags1;
    if (flags2) k_ptr->flags2 |= flags2;
    if (flags3) k_ptr->flags3 |= flags3;

    return (TRUE);
}


/*
 * Hack -- location saver for error messages
 */
static int error_k_idx = -1;


/*
 * Initialize the "k_list" array by parsing a file
 * Note that "k_list" starts out totally cleared
 */
static errr init_k_list_txt()
{
    register char *s, *t;

    /* No item kind yet */
    int m = -1;

    /* No k_ptr yet */
    inven_kind *k_ptr = NULL;

    /* The "objects" file */
    FILE *fp;

    /* No line should be more than 80 chars */
    char buf[160];

    /* Open the file */
    fp = fopen(ANGBAND_K_LIST, "r");

    /* Failure */
    if (!fp) return (-1);

    /* Parse the file to initialize "k_list" */
    while (1) {

	/* Read a line from the file, stop when done */
	if (!fgets(buf, 160, fp)) break;

	/* Skip comments */
	if (buf[0] == '#') continue;

	/* Strip the final newline */
	for (s = buf; isprint(*s); ++s); *s = '\0';

	/* Blank lines terminate monsters */
	if (!buf[0]) {

	    /* No current k_ptr */
	    if (!k_ptr) continue;

	    /* Now there is no current k_ptr */
	    k_ptr = NULL;

	    /* Next... */
	    continue;
	}

	/* The line better have a colon and such */
	if (buf[1] != ':') return (1);

	/* Process 'N' for "New/Number/Name" */
	if (buf[0] == 'N') {

	    /* Not done the previous one */
	    if (k_ptr) return (2);

	    /* Find, verify, and nuke the colon before the name */
	    if (!(s = strchr(buf+2, ':'))) return (3);

	    /* Nuke the colon, advance to the name */
	    *s++ = '\0';

	    /* Do not allow empty names */
	    if (!*s) return (4);

	    /* Get the index */
	    m = atoi(buf+2);

	    /* For errors */
	    error_k_idx = m;

	    /* Verify */
	    if ((m < 0) || (m >= MAX_K_IDX)) return (5);

	    /* Start a new k_ptr */
	    k_ptr = &k_list[m];

	    /* Make sure we have not done him yet */
	    if (k_ptr->name) return (6);

	    /* Save the name */
	    k_ptr->name = string_make(s);

	    /* Next... */
	    continue;
	}

	/* There better be a current k_ptr */
	if (!k_ptr) return (10);

	/* Process 'I' for "Info" (one line only) */
	if (buf[0] == 'I') {

	    char sym, col;
	    int tmp, tval, sval, pval, num, wgt, lev;
	    long cost;

	    /* Scan for the values */
	    if (9 != sscanf(buf+2, "%c:%c:%d:%d:%d:%d:%d:%d:%ld",
		&sym, &col, &tval, &sval, &pval,
		&num, &wgt, &lev, &cost)) return (11);

	    /* Extract the color */
	    tmp = color_char_to_attr(col);
	    if (tmp < 0) return (12);

	    /* Save the values */
	    k_ptr->i_char = sym;
	    k_ptr->i_attr = tmp;
	    k_ptr->tval = tval;
	    k_ptr->sval = sval;
	    k_ptr->pval = pval;
	    k_ptr->number = num;
	    k_ptr->weight = wgt;
	    k_ptr->level = lev;
	    k_ptr->cost = cost;


	    /* Next... */
	    continue;
	}

	/* Process 'A' for "Allocation" (one line only) */
	if (buf[0] == 'A') {

	    int i;

	    /* Simply read each number following a colon */
	    for (i = 0, s = buf+1; s && (s[0] == ':') && s[1]; ++i) {

		/* Default rarity */
		k_ptr->rarity[i] = 1;

		/* Store the attack damage index */
		k_ptr->locale[i] = atoi(s+1);

		/* Find the slash */
		t = strchr(s+1, '/');

		/* Find the next colon */
		s = strchr(s+1, ':');

		/* If the slash is "nearby", use it */
		if (t && (!s || t < s)) {
		    int rarity = atoi(t+1);
		    if (rarity > 0) k_ptr->rarity[i] = rarity;
		}
	    }

	    /* Next... */
	    continue;
	}

	/* Hack -- Process 'P' for "power" and such */
	if (buf[0] == 'P') {

	    int ac, hd1, hd2, th, td, ta;

	    /* Scan for the values */
	    if (6 != sscanf(buf+2, "%d:%dd%d:%d:%d:%d",
		&ac, &hd1, &hd2, &th, &td, &ta)) return (15);

	    k_ptr->ac = ac;
	    k_ptr->damage[0] = hd1;
	    k_ptr->damage[1] = hd2;
	    k_ptr->tohit = th;
	    k_ptr->todam = td;
	    k_ptr->toac =  ta;

	    /* Next... */
	    continue;
	}

	/* Hack -- Process 'F' for flags */
	if (buf[0] == 'F') {

	    huge flags1, flags2, flags3;

	    /* XXX XXX Hack -- Scan for "pure" values */
	    /* Note that "huge" may not equal "int32u" */
	    if (3 == sscanf(buf+2, "0x%lx:0x%lx:0x%lx",
		&flags1, &flags2, &flags3)) {

		k_ptr->flags1 = flags1;
		k_ptr->flags2 = flags2;
		k_ptr->flags3 = flags3;

		continue;
	    }

	    /* Parse every entry textually */
	    for (s = buf + 2; *s; ) {

		/* Find the end of this entry */
		for (t = s; *t && *t != ' ' && *t != '|'; ++t);

		/* Nuke and skip any dividers */
		if (*t) {
		    *t++ = '\0';
		    while (*t == ' ' || *t == '|') t++;
		}

		/* Parse this entry */
		if (!grab_one_kind_flag(k_ptr, s)) return (18);

		/* Start the next entry */
		s = t;
	    }

	    /* Next... */
	    continue;
	}
    }

    /* Close the file */
    fclose(fp);

    /* Success */
    return (0);
}







/*
 * Attempt to "dump" a "quick-load binary image" for "k_list"
 */
static errr dump_k_list_raw()
{

#ifdef BINARY_ARRAY_IMAGES

    int i, fd;

    /* One object  at a time */
    inven_kind *k_ptr;

    /* Do not save the name or desc fields */
    uint size = sizeof(inven_kind) - sizeof(cptr);
    
    int mode = 0666;
    
    int16u tmp2;

    char tmp[1024];
    char buf[2048];

    /* Use the "fast" binary version whenever possible */
    sprintf(tmp, "%s%s%s", ANGBAND_DIR_DATA, PATH_SEP, "k_list.raw");

#ifdef SET_UID
    mode = 0644;
#endif

#ifdef MACINTOSH
    _ftype = 'DATA';
#endif

    /* Create a new file */
    fd = my_topen(tmp, O_RDWR | O_CREAT | O_BINARY, mode);

#ifdef MACINTOSH
    _ftype = 'SAVE';
#endif

    /* Failure to make */
    if (fd < 0) return (-1);


    /* Dump the version info */
    buf[0] = CUR_VERSION_MAJ;
    buf[1] = CUR_VERSION_MIN; 
    buf[2] = CUR_PATCH_LEVEL;
    buf[3] = 0; /* Platform */

    /* Dump it, ignore errors */
    write(fd, buf, 4);

           
    /* Attempt to dump the object kinds */
    for (i = 0; i < MAX_K_IDX; i++) {

	/* Access the object kind */
	k_ptr = &k_list[i];

	/* Load the main record */
	if (size != write(fd, (char*)(k_ptr), size)) break;

	/* Dump the object name (if any) */
	if (k_ptr->name) {
	    strcpy(buf, k_ptr->name);
	    tmp2 = strlen(buf) + 1;
	    if (2 != write(fd, (char*)(&tmp2), 2)) break;
	    if (tmp2 != write(fd, buf, tmp2)) break;
	}
	else {
	    tmp2 = 0;
	    if (2 != write(fd, (char*)(&tmp2), 2)) break;
	}
    }
    
    /* Close it */
    close(fd);

    /* Failure */
    if (i < MAX_K_IDX) {
	message("Warning: failure writing 'k_list.raw'", 0);
	message("Warning: you should probably remove it.", 0);
	message(NULL, 0);
    }
    
#endif

    /* Success */
    return (0);
}



 
/*
 * Attempt to "quick-load" a binary image for "k_list"
 */
static errr init_k_list_raw()
{

#ifdef BINARY_ARRAY_IMAGES

    int i, fd;

    /* One kind at a time */
    inven_kind *k_ptr;

    /* Do not save the name or desc fields */
    uint size = sizeof(inven_kind) - sizeof(cptr);
    
    int mode = 0666;
    
    int16u tmp2;

    char tmp[1024];
    char buf[2048];


    /* Use the "fast" binary version whenever possible */
    sprintf(tmp, "%s%s%s", ANGBAND_DIR_DATA, PATH_SEP, "k_list.raw");

#ifdef SET_UID
    mode = 0644;
#endif

#ifdef MACINTOSH
    _ftype = 'DATA';
#endif

    /* Open a read-only copy */
    fd = my_topen(tmp, O_RDONLY | O_BINARY, mode);

#ifdef MACINTOSH
    _ftype = 'SAVE';
#endif

    /* Failure to open */
    if (fd < 0) return (-1);

    /* Read the version info */
    if ((4 != read(fd, buf, 4)) ||
	(buf[0] != CUR_VERSION_MAJ) ||
	(buf[1] != CUR_VERSION_MIN) ||
	(buf[2] != CUR_PATCH_LEVEL) ||
	(buf[0] != 0)) {

	/* Close and Fail (cleanly) */
	close(fd);
	return (-1);
    }
        
    /* Attempt to read the (normal) object kinds */
    for (i = 0; i < MAX_K_IDX; i++) {

	/* Access the object kind */
	k_ptr = &k_list[i];

	/* Load the main record */
	if (size != read(fd, (char*)(k_ptr), size)) break;

	/* Hack -- Load the monster name */
	k_ptr->name = NULL;
	if (2 != read(fd, (char*)(&tmp2), 2)) break;
	if (tmp2) {
	    if (tmp2 != read(fd, buf, tmp2)) break;
	    k_ptr->name = string_make(buf);
	}
    }

    /* Close the file */
    close(fd);

    /* Aborted load */
    if (i < MAX_K_IDX) {
	message("Major error loading 'k_list.raw'", 0);
	message("Try removing it from the 'data' directory", 0);
	message(NULL, 0);
	quit("failed while loading 'k_list.raw'");
    }

#endif

    /* Success */
    return (0);
}


/*
 * Note that "k_list" starts out totally cleared
 */
static void init_k_list()
{
    errr err;

    /* Try to load a "binary image" */
    err = init_k_list_raw();

    /* Otherwise try the text version */
    if (err) err = init_k_list_txt();

    /* Still no luck? Fail! */
    if (err) {
	char tmp[128];
	sprintf(tmp, "Fatal error #%d parsing 'k_list.txt', record %d",
		err, error_k_idx);
	message(tmp, 0);
	message(NULL,0);
	quit("cannot load 'k_list.txt'");
    }

    /* Attempt to dump a "quick-load" version */
    err = dump_k_list_raw();
    if (err) {
	message("Warning: unable to create 'k_list.raw'", 0);
    }
}





/*
 * This routine allocates and prepares several large arrays.
 *
 * The monster descriptions are extracted from ANGBAND_MONSTERS
 *
 * Note that the "C_MAKE()" macro allocates "clean" memory.
 */
void init_some_arrays()
{
    register int i;

    /* Prepare array of monster "race info" */
    C_MAKE(r_list, MAX_R_IDX, monster_race);

    /* Prepare array of object "kind info" */
    C_MAKE(k_list, MAX_K_IDX, inven_kind);


    /* Prepare array of monster "memories" */
    C_MAKE(l_list, MAX_R_IDX, monster_lore);

    /* Prepare array of object "memories" */
    C_MAKE(x_list, MAX_K_IDX, inven_xtra);


    /* Prepare object list */
    C_MAKE(i_list, MAX_I_IDX, inven_type);


    /* Prepare monster list */
    C_MAKE(m_list, MAX_M_IDX, monster_type);

    
    /* Allocate each line of the cave */
    for (i=0; i<MAX_HEIGHT; i++)
    {
	/* Allocate one row of the cave */
	C_MAKE(cave[i], MAX_WIDTH, cave_type);
    }


    /* Initialize r_list from a file */
    init_r_list();


    /* Initialize k_list from a file */
    init_k_list();
}



