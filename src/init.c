/* File: init.c */

/* Purpose: initialize various arrays from files -BEN- */

#include "angband.h"

/*
 * This file is used to initialize various variables and arrays for the
 * Angband game.  Note the use of "fd_read()" and "fd_write()" to bypass
 * the common limitation of "read()" and "write()" to only 32767 bytes
 * at a time.
 *
 * Several of the arrays for Angband are built from "template" files in
 * the "lib/file" directory, from which quick-load binary "image" files
 * are constructed whenever they are not present in the "lib/data"
 * directory, or if those files become obsolete, if we are allowed.
 *
 * Warning -- the "ascii" file parsers use a minor hack to collect the
 * name and text information in a single pass.  Thus, the game will not
 * be able to load any template file with more than 20K of names or 60K
 * of text, even though technically, up to 64K should be legal.
 *
 * Note that if "ALLOW_TEMPLATES" is not defined, then a lot of the code
 * in this file is compiled out, and the game will not run unless valid
 * "binary template files" already exist in "lib/data".  Thus, one can
 * compile Angband with ALLOW_TEMPLATES defined, run once to create the
 * "*.raw" files in "lib/data", and then quit, and recompile without
 * defining ALLOW_TEMPLATES, which will both save 20K and prevent people
 * from changing the ascii template files in potentially dangerous ways.
 *
 * The code could actually be removed and placed into a "stand-alone"
 * program, but that feels a little silly, especially considering some
 * of the platforms that we currently support.
 */


/*
 * Find the default paths to all of our important sub-directories.
 *
 * The purpose of each sub-directory is described in "variables.c".
 *
 * All of the sub-directories should, by default, be located inside
 * the main "lib" directory, whose location is very system dependant.
 *
 * This function takes a writable buffer, initially containing the
 * "path" to the "lib" directory, for example, "/pkg/lib/angband/",
 * or a system dependant string, for example, ":lib:".  The buffer
 * must be large enough to contain at least 32 more characters.
 *
 * Note that the initial path must end in the appropriate PATH_SEP
 * string (if necessary), or we may create meaningless paths.
 *
 * Various command line options may allow some of the important
 * directories to be changed to user-specified directories, most
 * importantly, the "info" and "user" and "save" directories.
 *
 * All of the "sub-directory" paths created below will end in
 * the "PATH_SEP" string, and any replacement paths specified
 * by the user should also end in the "PATH_SEP" string.
 *
 * Note that this function attempts to verify the "news" file,
 * and the game aborts (cleanly) on failure.
 *
 * Note that this function attempts to verify (or create) the
 * "high score" file, and the game aborts (cleanly) on failure.
 *
 * Note that one of the most common "extraction" errors involves
 * failing to extract all sub-directories (even empty ones), such
 * as by failing to use the "-d" option of "pkunzip", or failing
 * to use the "save empty directories" option with "Compact Pro".
 *
 * This error will be caught by the "high score" verification (or
 * creation) code below, since the "lib/apex" directory should be
 * distributed empty, and thus the ability to verify (or create)
 * a "high score" file in that directory normally indicates that
 * all of the sub-directories of "lib" exist.
 *
 * Note that this function is called BEFORE we initialize "term.c",
 * and so none of the basic "message" routines will work here!
 */
void init_file_paths(char *path)
{
    char *tail;

    int fd = -1;

    int mode = 0644;


    /*** Prepare the "path" ***/

    /* Hack -- save the main directory */
    ANGBAND_DIR = string_make(path);

    /* Prepare to append to the Base Path */
    tail = path + strlen(path);


    /*** Build the sub-directory names ***/

    /* Build a path name */
    sprintf(tail, "apex%s", PATH_SEP);
    ANGBAND_DIR_APEX = string_make(path);

    /* Build a path name */
    sprintf(tail, "bone%s", PATH_SEP);
    ANGBAND_DIR_BONE = string_make(path);

    /* Build a path name */
    sprintf(tail, "data%s", PATH_SEP);
    ANGBAND_DIR_DATA = string_make(path);

    /* Build a path name */
    sprintf(tail, "edit%s", PATH_SEP);
    ANGBAND_DIR_EDIT = string_make(path);

    /* Build a path name */
    sprintf(tail, "file%s", PATH_SEP);
    ANGBAND_DIR_FILE = string_make(path);

    /* Build a path name */
    sprintf(tail, "help%s", PATH_SEP);
    ANGBAND_DIR_HELP = string_make(path);

    /* Build a path name */
    sprintf(tail, "info%s", PATH_SEP);
    ANGBAND_DIR_INFO = string_make(path);

    /* Build a path name */
    sprintf(tail, "save%s", PATH_SEP);
    ANGBAND_DIR_SAVE = string_make(path);

    /* Build a path name */
    sprintf(tail, "user%s", PATH_SEP);
    ANGBAND_DIR_USER = string_make(path);

    /* Build a path name */
    sprintf(tail, "xtra%s", PATH_SEP);
    ANGBAND_DIR_XTRA = string_make(path);


    /*** Verify the "news" file ***/

    /* Hack -- attempt to access the "news" file */
    sprintf(path, "%s%s", ANGBAND_DIR_FILE, "news.txt");

    /* Attempt to open the file */
    fd = fd_open(path, O_RDONLY, 0);

    /* Hack -- display a complex message */
    if (fd < 0) {
    
        /* Message */
        plog_fmt("Cannot access the '%s' file!", path);
    
        /* Message */
        plog_fmt("Assuming defective 'lib' directory!");

        /* Error */
        quit("Fatal Error.");
    }
    
    /* Close it */
    (void)fd_close(fd);
    

    /*** Verify (or create) the "high score" file ***/

    /* Hack -- attempt to access the "high scores" file */
    sprintf(path, "%s%s", ANGBAND_DIR_APEX, "scores.raw");

    /* Attempt to open the high score file */
    fd = fd_open(path, O_RDONLY | O_BINARY, 0);

    /* Failure */
    if (fd < 0) {

#if defined(MACINTOSH) && !defined(applec)
        /* Global -- "data file" */
        _ftype = 'DATA';
#endif

        /* Create a new high score file (unless it exists) */
        fd = fd_open(path, O_WRONLY | O_CREAT | O_EXCL | O_BINARY, mode);

        /* Failure */
        if (fd < 0) {

            /* Message */
            plog_fmt("Cannot create the '%s' file!", path);
    
            /* Message */
            plog_fmt("Assuming defective 'lib' directory!");

            /* Error */
            quit("Fatal Error.");
        }
    }

    /* Close it */
    (void)fd_close(fd);
}




/*
 * Hack -- take notes on line 23
 */
static void note(cptr str)
{
    Term_erase(0, 23, 80, 1);
    Term_putstr(20, 23, -1, TERM_WHITE, str);
    Term_fresh();
}



#ifdef ALLOW_TEMPLATES


/*
 * Hack -- help give useful error messages
 */
static int error_idx = -1;
static int error_line = -1;


/*
 * Hack -- help initialize the fake "name" and "text" arrays when
 * parsing an "ascii" template file.
 */
static u16b fake_name_size;
static u16b fake_text_size;


/*
 * Standard error message text
 */
static cptr err_str[] = {
    NULL,
    "parse error",
    "obsolete file",
    "missing record header",
    "non-sequential records",
    "invalid flag specification",
    "undefined directive",
    "out of memory"
};



/*** Helper arrays for parsing ascii template files ***/

/*
 * Monster Blow Methods
 */
static cptr r_info_blow_method[] = {

    "",
    "HIT",
    "TOUCH",
    "PUNCH",
    "KICK",
    "CLAW",
    "BITE",
    "STING",
    "XXX1",
    "BUTT",
    "CRUSH",
    "ENGULF",
    "XXX2",
    "CRAWL",
    "DROOL",
    "SPIT",
    "XXX3",
    "GAZE",
    "WAIL",
    "SPORE",
    "XXX4",
    "BEG",
    "INSULT",
    "MOAN",
    "XXX5",
    NULL
};


/*
 * Monster Blow Effects
 */
static cptr r_info_blow_effect[] = {

    "",
    "HURT",
    "POISON",
    "UN_BONUS",
    "UN_POWER",
    "EAT_GOLD",
    "EAT_ITEM",
    "EAT_FOOD",
    "EAT_LITE",
    "ACID",
    "ELEC",
    "FIRE",
    "COLD",
    "BLIND",
    "CONFUSE",
    "TERRIFY",
    "PARALYZE",
    "LOSE_STR",
    "LOSE_INT",
    "LOSE_WIS",
    "LOSE_DEX",
    "LOSE_CON",
    "LOSE_CHR",
    "LOSE_ALL",
    "SHATTER",
    "EXP_10",
    "EXP_20",
    "EXP_40",
    "EXP_80",
    NULL
};


/*
 * Monster race flags
 */
static cptr r_info_flags1[] = {

    "UNIQUE",
    "QUESTOR",
    "MALE",
    "FEMALE",
    "CHAR_CLEAR",
    "CHAR_MULTI",
    "ATTR_CLEAR",
    "ATTR_MULTI",
    "FORCE_DEPTH",
    "FORCE_MAXHP",
    "FORCE_SLEEP",
    "FORCE_EXTRA",
    "FRIEND",
    "FRIENDS",
    "ESCORT",
    "ESCORTS",
    "NEVER_BLOW",
    "NEVER_MOVE",
    "RAND_25",
    "RAND_50",
    "ONLY_GOLD",
    "ONLY_ITEM",
    "DROP_60",
    "DROP_90",
    "DROP_1D2",
    "DROP_2D2",
    "DROP_3D2",
    "DROP_4D2",
    "DROP_GOOD",
    "DROP_GREAT",
    "DROP_USEFUL",
    "DROP_CHOSEN"
};

/*
 * Monster race flags
 */
static cptr r_info_flags2[] = {

    "STUPID",
    "SMART",
    "XXX1X2",
    "XXX2X2",
    "INVISIBLE",
    "COLD_BLOOD",
    "EMPTY_MIND",
    "WEIRD_MIND",
    "MULTIPLY",
    "REGENERATE",
    "XXX3X2",
    "XXX4X2",
    "POWERFUL",
    "XXX5X2",
    "XXX7X2",
    "XXX6X2",
    "OPEN_DOOR",
    "BASH_DOOR",
    "PASS_WALL",
    "KILL_WALL",
    "MOVE_BODY",
    "KILL_BODY",
    "TAKE_ITEM",
    "KILL_ITEM",
    "BRAIN_1",
    "BRAIN_2",
    "BRAIN_3",
    "BRAIN_4",
    "BRAIN_5",
    "BRAIN_6",
    "BRAIN_7",
    "BRAIN_8"
};

/*
 * Monster race flags
 */
static cptr r_info_flags3[] = {

    "ORC",
    "TROLL",
    "GIANT",
    "DRAGON",
    "DEMON",
    "UNDEAD",
    "EVIL",
    "ANIMAL",
    "XXX1X3",
    "XXX2X3",
    "XXX3X3",
    "XXX4X3",
    "HURT_LITE",
    "HURT_ROCK",
    "HURT_FIRE",
    "HURT_COLD",
    "IM_ACID",
    "IM_ELEC",
    "IM_FIRE",
    "IM_COLD",
    "IM_POIS",
    "XXX5X3",
    "RES_NETH",
    "RES_WATE",
    "RES_PLAS",
    "RES_NEXU",
    "RES_DISE",
    "XXX6X3",
    "NO_FEAR",
    "NO_STUN",
    "NO_CONF",
    "NO_SLEEP"
};

/*
 * Monster race flags
 */
static cptr r_info_flags4[] = {

    "SHRIEK",
    "XXX2X4",
    "XXX3X4",
    "XXX4X4",
    "ARROW_1",
    "ARROW_2",
    "ARROW_3",
    "ARROW_4",
    "BR_ACID",
    "BR_ELEC",
    "BR_FIRE",
    "BR_COLD",
    "BR_POIS",
    "BR_NETH",
    "BR_LITE",
    "BR_DARK",
    "BR_CONF",
    "BR_SOUN",
    "BR_CHAO",
    "BR_DISE",
    "BR_NEXU",
    "BR_TIME",
    "BR_INER",
    "BR_GRAV",
    "BR_SHAR",
    "BR_PLAS",
    "BR_WALL",
    "BR_MANA",
    "XXX5X4",
    "XXX6X4",
    "XXX7X4",
    "XXX8X4"
};

/*
 * Monster race flags
 */
static cptr r_info_flags5[] = {

    "BA_ACID",
    "BA_ELEC",
    "BA_FIRE",
    "BA_COLD",
    "BA_POIS",
    "BA_NETH",
    "BA_WATE",
    "BA_MANA",
    "BA_DARK",
    "DRAIN_MANA",
    "MIND_BLAST",
    "BRAIN_SMASH",
    "CAUSE_1",
    "CAUSE_2",
    "CAUSE_3",
    "CAUSE_4",
    "BO_ACID",
    "BO_ELEC",
    "BO_FIRE",
    "BO_COLD",
    "BO_POIS",
    "BO_NETH",
    "BO_WATE",
    "BO_MANA",
    "BO_PLAS",
    "BO_ICEE",
    "MISSILE",
    "SCARE",
    "BLIND",
    "CONF",
    "SLOW",
    "HOLD"
};

/*
 * Monster race flags
 */
static cptr r_info_flags6[] = {

    "HASTE",
    "XXX1X6",
    "HEAL",
    "XXX2X6",
    "BLINK",
    "TPORT",
    "XXX3X6",
    "XXX4X6",
    "TELE_TO",
    "TELE_AWAY",
    "TELE_LEVEL",
    "XXX5",
    "DARKNESS",
    "TRAPS",
    "FORGET",
    "XXX6X6",
    "XXX7X6",
    "XXX8X6",
    "S_MONSTER",
    "S_MONSTERS",
    "S_ANT",
    "S_SPIDER",
    "S_HOUND",
    "S_REPTILE",
    "S_ANGEL",
    "S_DEMON",
    "S_UNDEAD",
    "S_DRAGON",
    "S_HI_UNDEAD",
    "S_HI_DRAGON",
    "S_WRAITH",
    "S_UNIQUE"
};


/*
 * Object flags
 */
static cptr k_info_flags1[] = {

    "STR",
    "INT",
    "WIS",
    "DEX",
    "CON",
    "CHR",
    "XXX1",
    "XXX2",
    "STEALTH",
    "SEARCH",
    "INFRA",
    "TUNNEL",
    "SPEED",
    "BLOWS",
    "XXX3",
    "XXX4",
    "SLAY_ANIMAL",
    "SLAY_EVIL",
    "SLAY_UNDEAD",
    "SLAY_DEMON",
    "SLAY_ORC",
    "SLAY_TROLL",
    "SLAY_GIANT",
    "SLAY_DRAGON",
    "KILL_DRAGON",
    "XXX5",
    "IMPACT",
    "XXX6",
    "BRAND_ACID",
    "BRAND_ELEC",
    "BRAND_FIRE",
    "BRAND_COLD"
};

/*
 * Object flags
 */
static cptr k_info_flags2[] = {
    
    "SUST_STR",
    "SUST_INT",
    "SUST_WIS",
    "SUST_DEX",
    "SUST_CON",
    "SUST_CHR",
    "XXX1",
    "XXX2",
    "IM_ACID",
    "IM_ELEC",
    "IM_FIRE",
    "IM_COLD",
    "IM_POIS",
    "XXX3",
    "FREE_ACT",
    "HOLD_LIFE",
    "RES_ACID",
    "RES_ELEC",
    "RES_FIRE",
    "RES_COLD",
    "RES_POIS",
    "XXX4",
    "RES_LITE",
    "RES_DARK",
    "RES_BLIND",
    "RES_CONF",
    "RES_SOUND",
    "RES_SHARDS",
    "RES_NETHER",
    "RES_NEXUS",
    "RES_CHAOS",
    "RES_DISEN"
};

/*
 * Object flags
 */
static cptr k_info_flags3[] = {

    "XXX1",
    "XXX2",
    "XXX3",
    "XXX4",
    "XXX5",
    "XXX6",
    "XXX7",
    "XXX8",
    "EASY_KNOW",
    "HIDE_TYPE",
    "SHOW_MODS",
    "INSTA_ART",
    "FEATHER",
    "LITE",
    "SEE_INVIS",
    "TELEPATHY",
    "SLOW_DIGEST",
    "REGEN",
    "XTRA_MIGHT",
    "XTRA_SHOTS",
    "IGNORE_ACID",
    "IGNORE_ELEC",
    "IGNORE_FIRE",
    "IGNORE_COLD",
    "ACTIVATE",
    "DRAIN_EXP",
    "TELEPORT",
    "AGGRAVATE",
    "BLESSED",
    "CURSED",
    "HEAVY_CURSE",
    "PERMA_CURSE"
};


/*
 * Convert a "color letter" into an "actual" color
 * The colors are: dwsorgbuDWvyRGBU, as shown below
 */
static int color_char_to_attr(char c)
{
    switch (c) {

        case 'd': return (TERM_DARK);
        case 'w': return (TERM_WHITE);
        case 's': return (TERM_SLATE);
        case 'o': return (TERM_ORANGE);
        case 'r': return (TERM_RED);
        case 'g': return (TERM_GREEN);
        case 'b': return (TERM_BLUE);
        case 'u': return (TERM_UMBER);

        case 'D': return (TERM_L_DARK);
        case 'W': return (TERM_L_WHITE);
        case 'v': return (TERM_VIOLET);
        case 'y': return (TERM_YELLOW);
        case 'R': return (TERM_L_RED);
        case 'G': return (TERM_L_GREEN);
        case 'B': return (TERM_L_BLUE);
        case 'U': return (TERM_L_UMBER);
    }

    return (-1);
}



/*** Initialize from ascii template files ***/


/*
 * Initialize the "v_info" array, by parsing an ascii "template" file
 */
static errr init_v_info_txt(FILE *fp, char *buf)
{
    int i;

    char *s;

    /* Not ready yet */
    bool okay = FALSE;

    /* Current entry */
    vault_type *v_ptr = NULL;


    /* Just before the first record */
    error_idx = -1;

    /* Just before the first line */
    error_line = -1;


    /* Prepare the "fake" stuff */
    v_head->name_size = 0;
    v_head->text_size = 0;

    /* Parse */
    while (TRUE) {


        /* Advance the line number */
        error_line++;

        /* Read a line from the file, stop when done */
        if (!fgets(buf, 1000, fp)) break;


        /* Skip comments */
        if (buf[0] == '#') continue;

        /* Advance to "weirdness" (including the final newline) */
        for (s = buf; isprint(*s); ++s) ;
        
        /* Nuke "weirdness" */
        *s = '\0';

        /* Skip blank lines */
        if (!buf[0] || (buf[0] == ' ')) continue;

        /* Verify correct "colon" format */
        if (buf[1] != ':') return (1);


        /* Hack -- Process 'V' for "Version" */
        if (buf[0] == 'V') {

            int v1, v2, v3;

            /* Scan for the values */
            if ((3 != sscanf(buf, "V:%d.%d.%d", &v1, &v2, &v3)) ||
                (v1 != v_head->v_major) ||
                (v2 != v_head->v_minor) ||
                (v3 != v_head->v_patch)) {

                return (2);
            }

            /* Okay to proceed */
            okay = TRUE;

            /* Continue */
            continue;
        }

        /* No version yet */
        if (!okay) return (2);


        /* Process 'N' for "New/Number/Name" */
        if (buf[0] == 'N') {

            /* Find, verify, and nuke the colon before the name */
            if (!(s = strchr(buf+2, ':'))) return (1);

            /* Nuke the colon, advance to the name */
            *s++ = '\0';

            /* Paranoia -- require a name */
            if (!*s) return (1);

            /* Get the index */
            i = atoi(buf+2);

            /* Verify information */
            if (i <= error_idx) return (4);

            /* Verify information */
            if (i >= v_head->info_num) return (2);

            /* Save the index */
            error_idx = i;

            /* Point at the "info" */
            v_ptr = &v_info[i];

            /* Hack -- Verify space */
            if (v_head->name_size + strlen(s) > fake_name_size - 8) return (7);

            /* Advance and Save the name index */
            if (!v_ptr->name) v_ptr->name = ++v_head->name_size;

            /* Append chars to the name */
            strcpy(v_name + v_head->name_size, s);

            /* Advance the index */
            v_head->name_size += strlen(s);

            /* Next... */
            continue;
        }

        /* There better be a current v_ptr */
        if (!v_ptr) return (3);


        /* Process 'D' for "Description" */
        if (buf[0] == 'D') {

            /* Acquire the text */
            s = buf+2;

            /* Hack -- Verify space */
            if (v_head->text_size + strlen(s) > fake_text_size - 8) return (7);

            /* Advance and Save the text index */
            if (!v_ptr->text) v_ptr->text = ++v_head->text_size;

            /* Append chars to the name */
            strcpy(v_text + v_head->text_size, s);

            /* Advance the index */
            v_head->text_size += strlen(s);

            /* Next... */
            continue;
        }


        /* Process 'X' for "Extra info" (one line only) */
        if (buf[0] == 'X') {

            int typ, rat, hgt, wid;
            
            /* Scan for the values */
            if (4 != sscanf(buf+2, "%d:%d:%d:%d",
                &typ, &rat, &hgt, &wid)) return (1);

            /* Save the values */
            v_ptr->typ = typ;
            v_ptr->rat = rat;
            v_ptr->hgt = hgt;
            v_ptr->wid = wid;

            /* Next... */
            continue;
        }


        /* Oops */
        return (6);
    }


    /* Complete the "name" and "text" sizes */
    ++v_head->name_size;
    ++v_head->text_size;


    /* Success */
    return (0);
}



/*
 * Initialize the "f_info" array, by parsing an ascii "template" file
 */
static errr init_f_info_txt(FILE *fp, char *buf)
{
    int i;

    char *s;

    /* Not ready yet */
    bool okay = FALSE;

    /* Current entry */
    feature_type *f_ptr = NULL;


    /* Just before the first record */
    error_idx = -1;

    /* Just before the first line */
    error_line = -1;


    /* Prepare the "fake" stuff */
    f_head->name_size = 0;
    f_head->text_size = 0;

    /* Parse */
    while (TRUE) {


        /* Advance the line number */
        error_line++;

        /* Read a line from the file, stop when done */
        if (!fgets(buf, 1000, fp)) break;


        /* Skip comments */
        if (buf[0] == '#') continue;

        /* Advance to "weirdness" (including the final newline) */
        for (s = buf; isprint(*s); ++s) ;
        
        /* Nuke "weirdness" */
        *s = '\0';

        /* Skip blank lines */
        if (!buf[0] || (buf[0] == ' ')) continue;

        /* Verify correct "colon" format */
        if (buf[1] != ':') return (1);


        /* Hack -- Process 'V' for "Version" */
        if (buf[0] == 'V') {

            int v1, v2, v3;

            /* Scan for the values */
            if ((3 != sscanf(buf, "V:%d.%d.%d", &v1, &v2, &v3)) ||
                (v1 != f_head->v_major) ||
                (v2 != f_head->v_minor) ||
                (v3 != f_head->v_patch)) {

                return (2);
            }

            /* Okay to proceed */
            okay = TRUE;

            /* Continue */
            continue;
        }

        /* No version yet */
        if (!okay) return (2);


        /* Process 'N' for "New/Number/Name" */
        if (buf[0] == 'N') {

            /* Find, verify, and nuke the colon before the name */
            if (!(s = strchr(buf+2, ':'))) return (1);

            /* Nuke the colon, advance to the name */
            *s++ = '\0';

            /* Paranoia -- require a name */
            if (!*s) return (1);

            /* Get the index */
            i = atoi(buf+2);

            /* Verify information */
            if (i <= error_idx) return (4);

            /* Verify information */
            if (i >= f_head->info_num) return (2);

            /* Save the index */
            error_idx = i;

            /* Point at the "info" */
            f_ptr = &f_info[i];

            /* Hack -- Verify space */
            if (f_head->name_size + strlen(s) > fake_name_size - 8) return (7);

            /* Advance and Save the name index */
            if (!f_ptr->name) f_ptr->name = ++f_head->name_size;

            /* Append chars to the name */
            strcpy(f_name + f_head->name_size, s);

            /* Advance the index */
            f_head->name_size += strlen(s);

            /* Next... */
            continue;
        }

        /* There better be a current f_ptr */
        if (!f_ptr) return (3);


#if 0

        /* Process 'D' for "Description" */
        if (buf[0] == 'D') {

            /* Acquire the text */
            s = buf+2;

            /* Hack -- Verify space */
            if (f_head->text_size + strlen(s) > fake_text_size - 8) return (7);

            /* Advance and Save the text index */
            if (!f_ptr->text) f_ptr->text = ++f_head->text_size;

            /* Append chars to the name */
            strcpy(f_text + f_head->text_size, s);

            /* Advance the index */
            f_head->text_size += strlen(s);

            /* Next... */
            continue;
        }

#endif


        /* Process 'I' for "Info" (one line only) */
        if (buf[0] == 'I') {

            char sym, col;
            int tmp;

            /* Scan for the values */
            if (2 != sscanf(buf+2, "%c:%c",
                &sym, &col)) return (1);

            /* Extract the color */
            tmp = color_char_to_attr(col);
            if (tmp < 0) return (1);

            /* Save the values */
            f_ptr->f_char = sym;
            f_ptr->f_attr = tmp;

            /* Next... */
            continue;
        }


        /* Oops */
        return (6);
    }


    /* Complete the "name" and "text" sizes */
    ++f_head->name_size;
    ++f_head->text_size;


    /* Success */
    return (0);
}



/*
 * Grab one flag in an inven_kind from a textual string
 */
static errr grab_one_kind_flag(inven_kind *k_ptr, cptr what)
{
    int i;
    
    /* Check flags1 */
    for (i = 0; i < 32; i++) {
        if (streq(what, k_info_flags1[i])) {
            k_ptr->flags1 |= (1L << i);
            return (0);
        }
    }
    
    /* Check flags2 */
    for (i = 0; i < 32; i++) {
        if (streq(what, k_info_flags2[i])) {
            k_ptr->flags2 |= (1L << i);
            return (0);
        }
    }
    
    /* Check flags3 */
    for (i = 0; i < 32; i++) {
        if (streq(what, k_info_flags3[i])) {
            k_ptr->flags3 |= (1L << i);
            return (0);
        }
    }

    /* Error */
    return (1);
}



/*
 * Initialize the "k_info" array, by parsing an ascii "template" file
 */
static errr init_k_info_txt(FILE *fp, char *buf)
{
    int i;

    char *s, *t;

    /* Not ready yet */
    bool okay = FALSE;

    /* Current entry */
    inven_kind *k_ptr = NULL;


    /* Just before the first record */
    error_idx = -1;

    /* Just before the first line */
    error_line = -1;


    /* Prepare the "fake" stuff */
    k_head->name_size = 0;
    k_head->text_size = 0;

    /* Parse */
    while (TRUE) {


        /* Advance the line number */
        error_line++;

        /* Read a line from the file, stop when done */
        if (!fgets(buf, 1000, fp)) break;


        /* Skip comments */
        if (buf[0] == '#') continue;

        /* Advance to "weirdness" (including the final newline) */
        for (s = buf; isprint(*s); ++s) ;
        
        /* Nuke "weirdness" */
        *s = '\0';

        /* Skip blank lines */
        if (!buf[0] || (buf[0] == ' ')) continue;

        /* Verify correct "colon" format */
        if (buf[1] != ':') return (1);


        /* Hack -- Process 'V' for "Version" */
        if (buf[0] == 'V') {

            int v1, v2, v3;

            /* Scan for the values */
            if ((3 != sscanf(buf, "V:%d.%d.%d", &v1, &v2, &v3)) ||
                (v1 != k_head->v_major) ||
                (v2 != k_head->v_minor) ||
                (v3 != k_head->v_patch)) {

                return (2);
            }

            /* Okay to proceed */
            okay = TRUE;

            /* Continue */
            continue;
        }

        /* No version yet */
        if (!okay) return (2);


        /* Process 'N' for "New/Number/Name" */
        if (buf[0] == 'N') {

            /* Find, verify, and nuke the colon before the name */
            if (!(s = strchr(buf+2, ':'))) return (1);

            /* Nuke the colon, advance to the name */
            *s++ = '\0';

            /* Paranoia -- require a name */
            if (!*s) return (1);

            /* Get the index */
            i = atoi(buf+2);

            /* Verify information */
            if (i <= error_idx) return (4);

            /* Verify information */
            if (i >= k_head->info_num) return (2);

            /* Save the index */
            error_idx = i;

            /* Point at the "info" */
            k_ptr = &k_info[i];

            /* Hack -- Verify space */
            if (k_head->name_size + strlen(s) > fake_name_size - 8) return (7);

            /* Advance and Save the name index */
            if (!k_ptr->name) k_ptr->name = ++k_head->name_size;

            /* Append chars to the name */
            strcpy(k_name + k_head->name_size, s);

            /* Advance the index */
            k_head->name_size += strlen(s);

            /* Next... */
            continue;
        }

        /* There better be a current k_ptr */
        if (!k_ptr) return (3);


#if 0

        /* Process 'D' for "Description" */
        if (buf[0] == 'D') {

            /* Acquire the text */
            s = buf+2;

            /* Hack -- Verify space */
            if (k_head->text_size + strlen(s) > fake_text_size - 8) return (7);

            /* Advance and Save the text index */
            if (!k_ptr->text) k_ptr->text = ++k_head->text_size;

            /* Append chars to the name */
            strcpy(k_text + k_head->text_size, s);

            /* Advance the index */
            k_head->text_size += strlen(s);

            /* Next... */
            continue;
        }

#endif


        /* Process 'I' for "Info" (one line only) */
        if (buf[0] == 'I') {

            char sym, col;
            int tmp, tval, sval, pval;

            /* Scan for the values */
            if (5 != sscanf(buf+2, "%c:%c:%d:%d:%d",
                &sym, &col, &tval, &sval, &pval)) return (1);

            /* Extract the color */
            tmp = color_char_to_attr(col);
            if (tmp < 0) return (1);

            /* Save the values */
            k_ptr->k_char = sym;
            k_ptr->k_attr = tmp;
            k_ptr->tval = tval;
            k_ptr->sval = sval;
            k_ptr->pval = pval;

            /* Next... */
            continue;
        }

        /* Process 'W' for "More Info" (one line only) */
        if (buf[0] == 'W') {

            int level, extra, wgt;
            long cost;

            /* Scan for the values */
            if (4 != sscanf(buf+2, "%d:%d:%d:%ld",
                &level, &extra, &wgt, &cost)) return (1);

            /* Save the values */
            k_ptr->level = level;
            k_ptr->extra = extra;
            k_ptr->weight = wgt;
            k_ptr->cost = cost;

            /* Next... */
            continue;
        }

        /* Process 'A' for "Allocation" (one line only) */
        if (buf[0] == 'A') {

            int i;

            /* Simply read each number following a colon */
            for (i = 0, s = buf+1; s && (s[0] == ':') && s[1]; ++i) {

                /* Default chance */
                k_ptr->chance[i] = 1;

                /* Store the attack damage index */
                k_ptr->locale[i] = atoi(s+1);

                /* Find the slash */
                t = strchr(s+1, '/');

                /* Find the next colon */
                s = strchr(s+1, ':');

                /* If the slash is "nearby", use it */
                if (t && (!s || t < s)) {
                    int chance = atoi(t+1);
                    if (chance > 0) k_ptr->chance[i] = chance;
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
                &ac, &hd1, &hd2, &th, &td, &ta)) return (1);

            k_ptr->ac = ac;
            k_ptr->dd = hd1;
            k_ptr->ds = hd2;
            k_ptr->to_h = th;
            k_ptr->to_d = td;
            k_ptr->to_a =  ta;

            /* Next... */
            continue;
        }

        /* Hack -- Process 'F' for flags */
        if (buf[0] == 'F') {

            /* Parse every entry textually */
            for (s = buf + 2; *s; ) {

                /* Find the end of this entry */
                for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) ;

                /* Nuke and skip any dividers */
                if (*t) {
                    *t++ = '\0';
                    while (*t == ' ' || *t == '|') t++;
                }

                /* Parse this entry */
                if (0 != grab_one_kind_flag(k_ptr, s)) return (5);

                /* Start the next entry */
                s = t;
            }

            /* Next... */
            continue;
        }


        /* Oops */
        return (6);
    }


    /* Complete the "name" and "text" sizes */
    ++k_head->name_size;
    ++k_head->text_size;


    /* Success */
    return (0);
}


/*
 * Grab one flag in an artifact_type from a textual string
 */
static errr grab_one_artifact_flag(artifact_type *a_ptr, cptr what)
{
    int i;
    
    /* Check flags1 */
    for (i = 0; i < 32; i++) {
        if (streq(what, k_info_flags1[i])) {
            a_ptr->flags1 |= (1L << i);
            return (0);
        }
    }
    
    /* Check flags2 */
    for (i = 0; i < 32; i++) {
        if (streq(what, k_info_flags2[i])) {
            a_ptr->flags2 |= (1L << i);
            return (0);
        }
    }
    
    /* Check flags3 */
    for (i = 0; i < 32; i++) {
        if (streq(what, k_info_flags3[i])) {
            a_ptr->flags3 |= (1L << i);
            return (0);
        }
    }

    /* Error */
    return (1);
}




/*
 * Initialize the "a_info" array, by parsing an ascii "template" file
 */
static errr init_a_info_txt(FILE *fp, char *buf)
{
    int i;

    char *s, *t;

    /* Not ready yet */
    bool okay = FALSE;

    /* Current entry */
    artifact_type *a_ptr = NULL;


    /* Just before the first record */
    error_idx = -1;

    /* Just before the first line */
    error_line = -1;


    /* Parse */
    while (TRUE) {


        /* Advance the line number */
        error_line++;

        /* Read a line from the file, stop when done */
        if (!fgets(buf, 1000, fp)) break;


        /* Skip comments */
        if (buf[0] == '#') continue;

        /* Advance to "weirdness" (including the final newline) */
        for (s = buf; isprint(*s); ++s) ;
        
        /* Nuke "weirdness" */
        *s = '\0';

        /* Skip blank lines */
        if (!buf[0] || (buf[0] == ' ')) continue;

        /* Verify correct "colon" format */
        if (buf[1] != ':') return (1);


        /* Hack -- Process 'V' for "Version" */
        if (buf[0] == 'V') {

            int v1, v2, v3;

            /* Scan for the values */
            if ((3 != sscanf(buf, "V:%d.%d.%d", &v1, &v2, &v3)) ||
                (v1 != a_head->v_major) ||
                (v2 != a_head->v_minor) ||
                (v3 != a_head->v_patch)) {

                return (2);
            }

            /* Okay to proceed */
            okay = TRUE;

            /* Continue */
            continue;
        }

        /* No version yet */
        if (!okay) return (2);


        /* Process 'N' for "New/Number/Name" */
        if (buf[0] == 'N') {

            /* Find, verify, and nuke the colon before the name */
            if (!(s = strchr(buf+2, ':'))) return (1);

            /* Nuke the colon, advance to the name */
            *s++ = '\0';

            /* Paranoia -- require a name */
            if (!*s) return (1);

            /* Get the index */
            i = atoi(buf+2);

            /* Verify information */
            if (i < error_idx) return (4);

            /* Verify information */
            if (i >= a_head->info_num) return (2);

            /* Save the index */
            error_idx = i;

            /* Point at the "info" */
            a_ptr = &a_info[i];

            /* Hack -- Verify space */
            if (a_head->name_size + strlen(s) > fake_name_size - 8) return (7);

            /* Advance and Save the name index */
            if (!a_ptr->name) a_ptr->name = ++a_head->name_size;

            /* Append chars to the name */
            strcpy(a_name + a_head->name_size, s);

            /* Advance the index */
            a_head->name_size += strlen(s);

            /* Next... */
            continue;
        }

        /* There better be a current a_ptr */
        if (!a_ptr) return (3);


#if 0

        /* Process 'D' for "Description" */
        if (buf[0] == 'D') {

            /* Acquire the text */
            s = buf+2;

            /* Hack -- Verify space */
            if (a_head->text_size + strlen(s) > fake_text_size - 8) return (7);

            /* Advance and Save the text index */
            if (!a_ptr->text) a_ptr->text = ++a_head->text_size;

            /* Append chars to the name */
            strcpy(a_text + a_head->text_size, s);

            /* Advance the index */
            a_head->text_size += strlen(s);

            /* Next... */
            continue;
        }

#endif

        /* Process 'I' for "Info" (one line only) */
        if (buf[0] == 'I') {

            int tval, sval, pval;

            /* Scan for the values */
            if (3 != sscanf(buf+2, "%d:%d:%d",
                &tval, &sval, &pval)) return (1);

            /* Save the values */
            a_ptr->tval = tval;
            a_ptr->sval = sval;
            a_ptr->pval = pval;

            /* Next... */
            continue;
        }

        /* Process 'W' for "More Info" (one line only) */
        if (buf[0] == 'W') {

            int level, rarity, wgt;
            long cost;

            /* Scan for the values */
            if (4 != sscanf(buf+2, "%d:%d:%d:%ld",
                &level, &rarity, &wgt, &cost)) return (1);

            /* Save the values */
            a_ptr->level = level;
            a_ptr->rarity = rarity;
            a_ptr->weight = wgt;
            a_ptr->cost = cost;

            /* Next... */
            continue;
        }

        /* Hack -- Process 'P' for "power" and such */
        if (buf[0] == 'P') {

            int ac, hd1, hd2, th, td, ta;

            /* Scan for the values */
            if (6 != sscanf(buf+2, "%d:%dd%d:%d:%d:%d",
                &ac, &hd1, &hd2, &th, &td, &ta)) return (1);

            a_ptr->ac = ac;
            a_ptr->dd = hd1;
            a_ptr->ds = hd2;
            a_ptr->to_h = th;
            a_ptr->to_d = td;
            a_ptr->to_a =  ta;

            /* Next... */
            continue;
        }

        /* Hack -- Process 'F' for flags */
        if (buf[0] == 'F') {

            /* Parse every entry textually */
            for (s = buf + 2; *s; ) {

                /* Find the end of this entry */
                for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) ;

                /* Nuke and skip any dividers */
                if (*t) {
                    *t++ = '\0';
                    while ((*t == ' ') || (*t == '|')) t++;
                }

                /* Parse this entry */
                if (0 != grab_one_artifact_flag(a_ptr, s)) return (5);

                /* Start the next entry */
                s = t;
            }

            /* Next... */
            continue;
        }


        /* Oops */
        return (6);
    }


    /* Complete the "name" and "text" sizes */
    ++a_head->name_size;
    ++a_head->text_size;


    /* Hack -- extract the "ignore" flags */
    for (i = 0; i < MAX_A_IDX; i++) {

        a_ptr = &a_info[i];

        /* Skip non-artifacts */
        if (!a_ptr->name) continue;

        /* Ignore everything */
        a_ptr->flags3 |= (TR3_IGNORE_ACID);
        a_ptr->flags3 |= (TR3_IGNORE_ELEC);
        a_ptr->flags3 |= (TR3_IGNORE_FIRE);
        a_ptr->flags3 |= (TR3_IGNORE_COLD);
    }


    /* Success */
    return (0);
}


/*
 * Grab one flag in a ego-item_type from a textual string
 */
static bool grab_one_ego_item_flag(ego_item_type *e_ptr, cptr what)
{
    int i;
    
    /* Check flags1 */
    for (i = 0; i < 32; i++) {
        if (streq(what, k_info_flags1[i])) {
            e_ptr->flags1 |= (1L << i);
            return (0);
        }
    }
    
    /* Check flags2 */
    for (i = 0; i < 32; i++) {
        if (streq(what, k_info_flags2[i])) {
            e_ptr->flags2 |= (1L << i);
            return (0);
        }
    }
    
    /* Check flags3 */
    for (i = 0; i < 32; i++) {
        if (streq(what, k_info_flags3[i])) {
            e_ptr->flags3 |= (1L << i);
            return (0);
        }
    }

    /* Error */
    return (1);
}




/*
 * Initialize the "e_info" array, by parsing an ascii "template" file
 */
static errr init_e_info_txt(FILE *fp, char *buf)
{
    int i;

    char *s, *t;

    /* Not ready yet */
    bool okay = FALSE;

    /* Current entry */
    ego_item_type *e_ptr = NULL;


    /* Just before the first record */
    error_idx = -1;

    /* Just before the first line */
    error_line = -1;


    /* Parse */
    while (TRUE) {


        /* Advance the line number */
        error_line++;

        /* Read a line from the file, stop when done */
        if (!fgets(buf, 1000, fp)) break;


        /* Skip comments */
        if (buf[0] == '#') continue;

        /* Skip to "weirdness" (including the final newline) */
        for (s = buf; isprint(*s); ++s) ;

        /* Nuke "weirdness" */
        *s = '\0';

        /* Skip blank lines */
        if (!buf[0] || (buf[0] == ' ')) continue;

        /* Verify correct "colon" format */
        if (buf[1] != ':') return (1);


        /* Hack -- Process 'V' for "Version" */
        if (buf[0] == 'V') {

            int v1, v2, v3;

            /* Scan for the values */
            if ((3 != sscanf(buf, "V:%d.%d.%d", &v1, &v2, &v3)) ||
                (v1 != e_head->v_major) ||
                (v2 != e_head->v_minor) ||
                (v3 != e_head->v_patch)) {

                return (2);
            }

            /* Okay to proceed */
            okay = TRUE;

            /* Continue */
            continue;
        }

        /* No version yet */
        if (!okay) return (2);


        /* Process 'N' for "New/Number/Name" */
        if (buf[0] == 'N') {

            /* Find, verify, and nuke the colon before the name */
            if (!(s = strchr(buf+2, ':'))) return (1);

            /* Nuke the colon, advance to the name */
            *s++ = '\0';

            /* Paranoia -- require a name */
            if (!*s) return (1);

            /* Get the index */
            i = atoi(buf+2);

            /* Verify information */
            if (i < error_idx) return (4);

            /* Verify information */
            if (i >= e_head->info_num) return (2);

            /* Save the index */
            error_idx = i;

            /* Point at the "info" */
            e_ptr = &e_info[i];

            /* Hack -- Verify space */
            if (e_head->name_size + strlen(s) > fake_name_size - 8) return (7);

            /* Advance and Save the name index */
            if (!e_ptr->name) e_ptr->name = ++e_head->name_size;

            /* Append chars to the name */
            strcpy(e_name + e_head->name_size, s);

            /* Advance the index */
            e_head->name_size += strlen(s);

            /* Next... */
            continue;
        }

        /* There better be a current e_ptr */
        if (!e_ptr) return (3);


#if 0

        /* Process 'D' for "Description" */
        if (buf[0] == 'D') {

            /* Acquire the text */
            s = buf+2;

            /* Hack -- Verify space */
            if (e_head->text_size + strlen(s) > fake_text_size - 8) return (7);

            /* Advance and Save the text index */
            if (!e_ptr->text) e_ptr->text = ++e_head->text_size;

            /* Append chars to the name */
            strcpy(e_text + e_head->text_size, s);

            /* Advance the index */
            e_head->text_size += strlen(s);

            /* Next... */
            continue;
        }

#endif

        /* Process 'X' for "Xtra" (one line only) */
        if (buf[0] == 'X') {

            int slot, rating;

            /* Scan for the values */
            if (2 != sscanf(buf+2, "%d:%d",
                &slot, &rating)) return (1);

            /* Save the values */
            e_ptr->slot = slot;
            e_ptr->rating = rating;

            /* Next... */
            continue;
        }

        /* Process 'W' for "More Info" (one line only) */
        if (buf[0] == 'W') {

            int level, rarity, pad2;
            long cost;

            /* Scan for the values */
            if (4 != sscanf(buf+2, "%d:%d:%d:%ld",
                &level, &rarity, &pad2, &cost)) return (1);

            /* Save the values */
            e_ptr->level = level;
            e_ptr->rarity = rarity;
            /* e_ptr->weight = wgt; */
            e_ptr->cost = cost;

            /* Next... */
            continue;
        }

        /* Hack -- Process 'C' for "creation" */
        if (buf[0] == 'C') {

            int th, td, ta, pv;

            /* Scan for the values */
            if (4 != sscanf(buf+2, "%d:%d:%d:%d",
                &th, &td, &ta, &pv)) return (1);

            e_ptr->max_to_h = th;
            e_ptr->max_to_d = td;
            e_ptr->max_to_a = ta;
            e_ptr->max_pval = pv;

            /* Next... */
            continue;
        }

        /* Hack -- Process 'F' for flags */
        if (buf[0] == 'F') {

            /* Parse every entry textually */
            for (s = buf + 2; *s; ) {

                /* Find the end of this entry */
                for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) ;

                /* Nuke and skip any dividers */
                if (*t) {
                    *t++ = '\0';
                    while ((*t == ' ') || (*t == '|')) t++;
                }

                /* Parse this entry */
                if (0 != grab_one_ego_item_flag(e_ptr, s)) return (5);

                /* Start the next entry */
                s = t;
            }

            /* Next... */
            continue;
        }

        /* Oops */
        return (6);
    }


    /* Complete the "name" and "text" sizes */
    ++e_head->name_size;
    ++e_head->text_size;


#if 0

    /* Hack -- extract the "ignore" flags */
    for (i = 0; i < MAX_E_IDX; i++) {

        e_ptr = &e_info[i];

        /* Skip non-artifacts */
        if (!e_ptr->name) continue;

        /* Ignore when resistant */
        if (e_ptr->flags2 & TR2_RES_ACID) e_ptr->flags3 |= (TR3_IGNORE_ACID);
        if (e_ptr->flags2 & TR2_RES_ELEC) e_ptr->flags3 |= (TR3_IGNORE_ELEC);
        if (e_ptr->flags2 & TR2_RES_FIRE) e_ptr->flags3 |= (TR3_IGNORE_FIRE);
        if (e_ptr->flags2 & TR2_RES_COLD) e_ptr->flags3 |= (TR3_IGNORE_COLD);

        /* Ignore when immune */
        if (e_ptr->flags2 & TR2_IM_ACID) e_ptr->flags3 |= (TR3_IGNORE_ACID);
        if (e_ptr->flags2 & TR2_IM_ELEC) e_ptr->flags3 |= (TR3_IGNORE_ELEC);
        if (e_ptr->flags2 & TR2_IM_FIRE) e_ptr->flags3 |= (TR3_IGNORE_FIRE);
        if (e_ptr->flags2 & TR2_IM_COLD) e_ptr->flags3 |= (TR3_IGNORE_COLD);
    }

#endif


    /* Success */
    return (0);
}


/*
 * Grab one (basic) flag in a monster_race from a textual string
 */
static errr grab_one_basic_flag(monster_race *r_ptr, cptr what)
{
    int i;
    
    /* Scan flags1 */
    for (i = 0; i < 32; i++) {
        if (streq(what, r_info_flags1[i])) {
            r_ptr->flags1 |= (1L << i);
            return (0);
        }
    }
    
    /* Scan flags2 */
    for (i = 0; i < 32; i++) {
        if (streq(what, r_info_flags2[i])) {
            r_ptr->flags2 |= (1L << i);
            return (0);
        }
    }
    
    /* Scan flags1 */
    for (i = 0; i < 32; i++) {
        if (streq(what, r_info_flags3[i])) {
            r_ptr->flags3 |= (1L << i);
            return (0);
        }
    }
    
    /* Failure */
    return (1);
}


/*
 * Grab one (spell) flag in a monster_race from a textual string
 */
static errr grab_one_spell_flag(monster_race *r_ptr, cptr what)
{
    int i;
    
    /* Scan flags4 */
    for (i = 0; i < 32; i++) {
        if (streq(what, r_info_flags4[i])) {
            r_ptr->flags4 |= (1L << i);
            return (0);
        }
    }
    
    /* Scan flags5 */
    for (i = 0; i < 32; i++) {
        if (streq(what, r_info_flags5[i])) {
            r_ptr->flags5 |= (1L << i);
            return (0);
        }
    }
    
    /* Scan flags6 */
    for (i = 0; i < 32; i++) {
        if (streq(what, r_info_flags6[i])) {
            r_ptr->flags6 |= (1L << i);
            return (0);
        }
    }
    
    /* Failure */
    return (1);
}




/*
 * Initialize the "r_info" array, by parsing an ascii "template" file
 */
static errr init_r_info_txt(FILE *fp, char *buf)
{
    int i;

    char *s, *t;

    /* Not ready yet */
    bool okay = FALSE;

    /* Current entry */
    monster_race *r_ptr = NULL;


    /* Just before the first record */
    error_idx = -1;

    /* Just before the first line */
    error_line = -1;


    /* Start the "fake" stuff */
    r_head->name_size = 0;
    r_head->text_size = 0;

    /* Parse */
    while (TRUE) {


        /* Advance the line number */
        error_line++;

        /* Read a line from the file, stop when done */
        if (!fgets(buf, 1000, fp)) break;


        /* Skip comments */
        if (buf[0] == '#') continue;

        /* Advance to "weirdness" (including the final newline) */
        for (s = buf; isprint(*s); ++s) ;
        
        /* Nuke "weirdness" */
        *s = '\0';

        /* Skip blank lines */
        if (!buf[0] || (buf[0] == ' ')) continue;

        /* Verify correct "colon" format */
        if (buf[1] != ':') return (1);


        /* Hack -- Process 'V' for "Version" */
        if (buf[0] == 'V') {

            int v1, v2, v3;

            /* Scan for the values */
            if ((3 != sscanf(buf, "V:%d.%d.%d", &v1, &v2, &v3)) ||
                (v1 != r_head->v_major) ||
                (v2 != r_head->v_minor) ||
                (v3 != r_head->v_patch)) {

                return (2);
            }

            /* Okay to proceed */
            okay = TRUE;

            /* Continue */
            continue;
        }

        /* No version yet */
        if (!okay) return (2);


        /* Process 'N' for "New/Number/Name" */
        if (buf[0] == 'N') {

            /* Find, verify, and nuke the colon before the name */
            if (!(s = strchr(buf+2, ':'))) return (1);

            /* Nuke the colon, advance to the name */
            *s++ = '\0';

            /* Paranoia -- require a name */
            if (!*s) return (1);

            /* Get the index */
            i = atoi(buf+2);

            /* Verify information */
            if (i < error_idx) return (4);

            /* Verify information */
            if (i >= r_head->info_num) return (2);

            /* Save the index */
            error_idx = i;

            /* Point at the "info" */
            r_ptr = &r_info[i];

            /* Hack -- Verify space */
            if (r_head->name_size + strlen(s) > fake_name_size - 8) return (7);

            /* Advance and Save the name index */
            if (!r_ptr->name) r_ptr->name = ++r_head->name_size;

            /* Append chars to the name */
            strcpy(r_name + r_head->name_size, s);

            /* Advance the index */
            r_head->name_size += strlen(s);

            /* Next... */
            continue;
        }

        /* There better be a current r_ptr */
        if (!r_ptr) return (3);


        /* Process 'D' for "Description" */
        if (buf[0] == 'D') {

            /* Acquire the text */
            s = buf+2;

            /* Hack -- Verify space */
            if (r_head->text_size + strlen(s) > fake_text_size - 8) return (7);

            /* Advance and Save the text index */
            if (!r_ptr->text) r_ptr->text = ++r_head->text_size;

            /* Append chars to the name */
            strcpy(r_text + r_head->text_size, s);

            /* Advance the index */
            r_head->text_size += strlen(s);

            /* Next... */
            continue;
        }

        /* Process 'I' for "Info" (one line only) */
        if (buf[0] == 'I') {

            char chr, att;
            int tmp, spd, hp1, hp2, aaf, ac, slp;

            /* Scan for the values */
            if (8 != sscanf(buf, "I:%c:%c:%d:%dd%d:%d:%d:%d",
                &chr, &att, &spd, &hp1, &hp2, &aaf, &ac, &slp)) return (1);

            /* Extract the color */
            tmp = color_char_to_attr(att);
            if (tmp < 0) return (1);

            /* Save the values */
            r_ptr->r_char = chr;
            r_ptr->r_attr = tmp;
            r_ptr->speed = spd;
            r_ptr->hdice = hp1;
            r_ptr->hside = hp2;
            r_ptr->aaf = aaf;
            r_ptr->ac = ac;
            r_ptr->sleep = slp;

            /* Next... */
            continue;
        }

        /* Process 'W' for "More Info" (one line only) */
        if (buf[0] == 'W') {

            int lev, rar, pad;
            long exp;

            /* Scan for the values */
            if (4 != sscanf(buf, "W:%d:%d:%d:%ld",
                &lev, &rar, &pad, &exp)) return (1);

            /* Save the values */
            r_ptr->level = lev;
            r_ptr->rarity = rar;
            r_ptr->extra = pad;
            r_ptr->mexp = exp;

            /* Next... */
            continue;
        }

        /* Process 'B' for "Blows" (up to four lines) */
        if (buf[0] == 'B') {

            int n1, n2;

            /* Find the next empty blow slot (if any) */
            for (i = 0; i < 4; i++) if (!r_ptr->blow[i].method) break;

            /* Oops, no more slots */
            if (i == 4) return (1);
            
            /* Analyze the first field */
            for (s = t = buf+2; *t && (*t != ':'); t++) ;

            /* Terminate the field (if necessary) */
            if (*t == ':') *t++ = '\0';

            /* Analyze the method */
            for (n1 = 0; r_info_blow_method[n1]; n1++) {
                if (streq(s, r_info_blow_method[n1])) break;
            }
            
            /* Invalid method */
            if (!r_info_blow_method[n1]) return (1);

            /* Analyze the second field */
            for (s = t; *t && (*t != ':'); t++) ;

            /* Terminate the field (if necessary) */
            if (*t == ':') *t++ = '\0';

            /* Analyze effect */
            for (n2 = 0; r_info_blow_effect[n2]; n2++) {
                if (streq(s, r_info_blow_effect[n2])) break;
            }
    
            /* Invalid effect */
            if (!r_info_blow_effect[n2]) return (1);

            /* Analyze the third field */
            for (s = t; *t && (*t != 'd'); t++) ;

            /* Terminate the field (if necessary) */
            if (*t == 'd') *t++ = '\0';

            /* Save the method */
            r_ptr->blow[i].method = n1;

            /* Save the effect */
            r_ptr->blow[i].effect = n2;

            /* Extract the damage dice and sides */
            r_ptr->blow[i].d_dice = atoi(s);
            r_ptr->blow[i].d_side = atoi(t);

            /* Next... */
            continue;
        }

        /* Process 'F' for "Basic Flags" (multiple lines) */
        if (buf[0] == 'F') {

            /* Parse every entry */
            for (s = buf + 2; *s; ) {

                /* Find the end of this entry */
                for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) ;

                /* Nuke and skip any dividers */
                if (*t) {
                    *t++ = '\0';
                    while (*t == ' ' || *t == '|') t++;
                }

                /* Parse this entry */
                if (0 != grab_one_basic_flag(r_ptr, s)) return (5);

                /* Start the next entry */
                s = t;
            }

            /* Next... */
            continue;
        }

        /* Process 'S' for "Spell Flags" (multiple lines) */
        if (buf[0] == 'S') {

            /* Parse every entry */
            for (s = buf + 2; *s; ) {

                /* Find the end of this entry */
                for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) ;

                /* Nuke and skip any dividers */
                if (*t) {
                    *t++ = '\0';
                    while ((*t == ' ') || (*t == '|')) t++;
                }

                /* XXX XXX XXX Hack -- Read spell frequency */
                if (1 == sscanf(s, "1_IN_%d", &i)) {

                    /* Extract a "frequency" */
                    r_ptr->freq_spell = r_ptr->freq_inate = 100 / i;

                    /* Start at next entry */
                    s = t;
                    
                    /* Continue */
                    continue;
                }

                /* Parse this entry */
                if (0 != grab_one_spell_flag(r_ptr, s)) return (5);

                /* Start the next entry */
                s = t;
            }

            /* Next... */
            continue;
        }

        /* Oops */
        return (6);
    }


    /* Complete the "name" and "text" sizes */
    ++r_head->name_size;
    ++r_head->text_size;


    /* XXX XXX XXX XXX XXX */

    /* Mega-Hack -- acquire "ghost" */
    r_ptr = &r_info[MAX_R_IDX-1];

    /* Acquire the next index */
    r_ptr->name = r_head->name_size;
    r_ptr->text = r_head->text_size;

    /* Save some space for the ghost info */
    r_head->name_size += 64;
    r_head->text_size += 64;

    /* Hack -- Default name/text for the ghost */
    strcpy(r_name + r_ptr->name, "Nobody, the Unknown Player Ghost");
    strcpy(r_text + r_ptr->text, "It seems strangely familiar...");

    /* Hack -- set the char/attr info */
    r_ptr->r_attr = r_ptr->l_attr = TERM_WHITE;
    r_ptr->r_char = r_ptr->l_char = 'G';

    /* Hack -- Try to prevent a few "potential" bugs */
    r_ptr->flags1 |= (RF1_UNIQUE);

    /* Hack -- Try to prevent a few "potential" bugs */
    r_ptr->flags1 |= (RF1_NEVER_MOVE | RF1_NEVER_BLOW);

    /* Hack -- Try to prevent a few "potential" bugs */
    r_ptr->hdice = r_ptr->hside = 1;

    /* Hack -- Try to prevent a few "potential" bugs */
    r_ptr->mexp = 1L;


    /* Success */
    return (0);
}


#endif	/* ALLOW_TEMPLATES */



/*** Initialize from binary image files ***/


/*
 * Initialize the "f_info" array, by parsing a binary "image" file
 */
static errr init_f_info_raw(int fd)
{
    header test;


    /* Read the header */
    fd_read(fd, (char*)(&test), sizeof(header));

    /* Read and Verify the version info */
    if ((test.v_major != f_head->v_major) ||
        (test.v_minor != f_head->v_minor) ||
        (test.v_patch != f_head->v_patch) ||
        (test.v_extra != f_head->v_extra) ||
        (test.info_num != f_head->info_num) ||
        (test.info_len != f_head->info_len) ||
        (test.head_size != f_head->head_size) ||
        (test.info_size != f_head->info_size)) {

        /* Error */
        return (-1);
    }


    /* Accept the header */
    (*f_head) = test;


    /* Allocate the "f_info" array */
    C_MAKE(f_info, f_head->info_num, feature_type);

    /* Allocate the "k_name" array */
    C_MAKE(f_name, f_head->name_size, char);

    /* Allocate the "k_text" array */
    C_MAKE(f_text, f_head->text_size, char);


    /* Read the "f_info" array */
    fd_read(fd, (char*)(f_info), f_head->info_size);

    /* Read the "f_name" array */
    fd_read(fd, (char*)(f_name), f_head->name_size);

    /* Read the "f_text" array */
    fd_read(fd, (char*)(f_text), f_head->text_size);


    /* Success */
    return (0);
}



/*
 * Initialize the "f_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_f_info(void)
{
    int fd;

    int mode = 0644;

    errr err;

    FILE *fp;

    /* General buffer */
    char buf[1024];


    /*** Make the header ***/

    /* Allocate the "header" */
    MAKE(f_head, header);

    /* Save the "version" */
    f_head->v_major = VERSION_MAJOR;
    f_head->v_minor = VERSION_MINOR;
    f_head->v_patch = VERSION_PATCH;
    f_head->v_extra = 0;

    /* Save the "record" information */
    f_head->info_num = MAX_F_IDX;
    f_head->info_len = sizeof(feature_type);

    /* Save the size of "k_head" and "k_info" */
    f_head->head_size = sizeof(header);
    f_head->info_size = f_head->info_num * f_head->info_len;


#ifdef ALLOW_TEMPLATES

    /*** Load the binary image file ***/

    /* Construct the name of the "raw" file */
    sprintf(buf, "%s%s", ANGBAND_DIR_DATA, "f_info.raw");

    /* Attempt to open the "raw" file */
    fd = fd_open(buf, O_RDONLY | O_BINARY, 0);

    /* Process existing "raw" file */
    if (fd >= 0) {

        /* Attempt to parse the "raw" file */
        err = init_f_info_raw(fd);

        /* Close it */
        (void)fd_close(fd);

        /* Success */
        if (!err) return (0);

        /* Information */
        msg_print("Ignoring obsolete/defective 'f_info.raw' file.");
        msg_print(NULL);
    }


    /*** Make the fake arrays ***/

    /* Fake the size of "f_name" and "f_text" */
    fake_name_size = 20 * 1024L;
    fake_text_size = 60 * 1024L;

    /* Allocate the "k_info" array */
    C_MAKE(f_info, f_head->info_num, feature_type);

    /* Hack -- make "fake" arrays */
    C_MAKE(f_name, fake_name_size, char);
    C_MAKE(f_text, fake_text_size, char);


    /*** Load the ascii template file ***/

    /* Access the "k_info.txt" file */
    sprintf(buf, "%s%s", ANGBAND_DIR_EDIT, "f_info.txt");

    /* Open the file */
    fp = my_fopen(buf, "r");

    /* Parse it */
    if (!fp) quit("Cannot open 'f_info.txt' file.");

    /* Parse the file */
    err = init_f_info_txt(fp, buf);

    /* Close it */
    my_fclose(fp);

    /* Errors */
    if (err) {

        cptr oops;
        
        /* Error string */
        oops = ((err > 0) ? err_str[err] : "unknown");

        /* Oops */
        msg_format("Error at line %d of 'f_info.txt' file.", error_line);
        msg_format("Record %d contains a '%s' error.", error_idx, oops);
        msg_format("Parsing '%s'.", buf);
        msg_print(NULL);

        /* Quit */
        quit("Error in 'f_info.txt' file.");
    }


    /*** Dump the binary image file ***/

#if defined(MACINTOSH) && !defined(applec)
    /* Global -- "data file" */
    _ftype = 'DATA';
#endif

    /* Construct the name of the raw file */
    sprintf(buf, "%s%s", ANGBAND_DIR_DATA, "f_info.raw");

    /* Attempt to create the raw file */
    fd = fd_open(buf, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, mode);

    /* Dump to the file */
    if (fd >= 0) {

        /* Dump it */
        fd_write(fd, (char*)(f_head), f_head->head_size);

        /* Dump the "f_info" array */
        fd_write(fd, (char*)(f_info), f_head->info_size);

        /* Dump the "f_name" array */
        fd_write(fd, (char*)(f_name), f_head->name_size);

        /* Dump the "f_text" array */
        fd_write(fd, (char*)(f_text), f_head->text_size);

        /* Close */
        (void)fd_close(fd);
    }


    /*** Kill the fake arrays ***/

    /* Free the "f_info" array */
    C_KILL(f_info, f_head->info_num, feature_type);

    /* Hack -- Free the "fake" arrays */
    C_KILL(f_name, fake_name_size, char);
    C_KILL(f_text, fake_text_size, char);

    /* Forget the array sizes */
    fake_name_size = 0;
    fake_text_size = 0;

#endif	/* ALLOW_TEMPLATES */


    /*** Load the binary image file ***/

    /* Construct the name of the "raw" file */
    sprintf(buf, "%s%s", ANGBAND_DIR_DATA, "f_info.raw");

    /* Attempt to open the "raw" file */
    fd = fd_open(buf, O_RDONLY | O_BINARY, 0);

    /* Process existing "raw" file */
    if (fd < 0) quit("Cannot load 'f_info.raw' file.");

    /* Attempt to parse the "raw" file */
    err = init_f_info_raw(fd);

    /* Close it */
    (void)fd_close(fd);

    /* Error */
    if (err) quit("Cannot parse 'f_info.raw' file.");

    /* Success */
    return (0);
}



/*
 * Initialize the "k_info" array, by parsing a binary "image" file
 */
static errr init_k_info_raw(int fd)
{
    header test;


    /* Read the header */
    fd_read(fd, (char*)(&test), sizeof(header));

    /* Read and Verify the version info */
    if ((test.v_major != k_head->v_major) ||
        (test.v_minor != k_head->v_minor) ||
        (test.v_patch != k_head->v_patch) ||
        (test.v_extra != k_head->v_extra) ||
        (test.info_num != k_head->info_num) ||
        (test.info_len != k_head->info_len) ||
        (test.head_size != k_head->head_size) ||
        (test.info_size != k_head->info_size)) {

        /* Error */
        return (-1);
    }


    /* Accept the header */
    (*k_head) = test;


    /* Allocate the "k_info" array */
    C_MAKE(k_info, k_head->info_num, inven_kind);

    /* Allocate the "k_name" array */
    C_MAKE(k_name, k_head->name_size, char);

    /* Allocate the "k_text" array */
    C_MAKE(k_text, k_head->text_size, char);


    /* Read the "k_info" array */
    fd_read(fd, (char*)(k_info), k_head->info_size);

    /* Read the "k_name" array */
    fd_read(fd, (char*)(k_name), k_head->name_size);

    /* Read the "k_text" array */
    fd_read(fd, (char*)(k_text), k_head->text_size);


    /* Success */
    return (0);
}



/*
 * Initialize the "k_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_k_info(void)
{
    int fd;

    int mode = 0644;

    errr err;

    FILE *fp;

    /* General buffer */
    char buf[1024];


    /*** Make the header ***/

    /* Allocate the "header" */
    MAKE(k_head, header);

    /* Save the "version" */
    k_head->v_major = VERSION_MAJOR;
    k_head->v_minor = VERSION_MINOR;
    k_head->v_patch = VERSION_PATCH;
    k_head->v_extra = 0;

    /* Save the "record" information */
    k_head->info_num = MAX_K_IDX;
    k_head->info_len = sizeof(inven_kind);

    /* Save the size of "k_head" and "k_info" */
    k_head->head_size = sizeof(header);
    k_head->info_size = k_head->info_num * k_head->info_len;


#ifdef ALLOW_TEMPLATES

    /*** Load the binary image file ***/

    /* Construct the name of the "raw" file */
    sprintf(buf, "%s%s", ANGBAND_DIR_DATA, "k_info.raw");

    /* Attempt to open the "raw" file */
    fd = fd_open(buf, O_RDONLY | O_BINARY, 0);

    /* Process existing "raw" file */
    if (fd >= 0) {

        /* Attempt to parse the "raw" file */
        err = init_k_info_raw(fd);

        /* Close it */
        (void)fd_close(fd);

        /* Success */
        if (!err) return (0);

        /* Information */
        msg_print("Ignoring obsolete/defective 'k_info.raw' file.");
        msg_print(NULL);
    }


    /*** Make the fake arrays ***/

    /* Fake the size of "k_name" and "k_text" */
    fake_name_size = 20 * 1024L;
    fake_text_size = 60 * 1024L;

    /* Allocate the "k_info" array */
    C_MAKE(k_info, k_head->info_num, inven_kind);

    /* Hack -- make "fake" arrays */
    C_MAKE(k_name, fake_name_size, char);
    C_MAKE(k_text, fake_text_size, char);


    /*** Load the ascii template file ***/

    /* Access the "k_info.txt" file */
    sprintf(buf, "%s%s", ANGBAND_DIR_EDIT, "k_info.txt");

    /* Open the file */
    fp = my_fopen(buf, "r");

    /* Parse it */
    if (!fp) quit("Cannot open 'k_info.txt' file.");

    /* Parse the file */
    err = init_k_info_txt(fp, buf);

    /* Close it */
    my_fclose(fp);

    /* Errors */
    if (err) {

        cptr oops;
        
        /* Error string */
        oops = ((err > 0) ? err_str[err] : "unknown");

        /* Oops */
        msg_format("Error at line %d of 'k_info.txt' file.", error_line);
        msg_format("Record %d contains a '%s' error.", error_idx, oops);
        msg_format("Parsing '%s'.", buf);
        msg_print(NULL);

        /* Quit */
        quit("Error in 'k_info.txt' file.");
    }


    /*** Dump the binary image file ***/

#if defined(MACINTOSH) && !defined(applec)
    /* Global -- "data file" */
    _ftype = 'DATA';
#endif

    /* Construct the name of the raw file */
    sprintf(buf, "%s%s", ANGBAND_DIR_DATA, "k_info.raw");

    /* Attempt to create the raw file */
    fd = fd_open(buf, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, mode);

    /* Dump to the file */
    if (fd >= 0) {

        /* Dump it */
        fd_write(fd, (char*)(k_head), k_head->head_size);

        /* Dump the "k_info" array */
        fd_write(fd, (char*)(k_info), k_head->info_size);

        /* Dump the "k_name" array */
        fd_write(fd, (char*)(k_name), k_head->name_size);

        /* Dump the "k_text" array */
        fd_write(fd, (char*)(k_text), k_head->text_size);

        /* Close */
        (void)fd_close(fd);
    }


    /*** Kill the fake arrays ***/

    /* Free the "k_info" array */
    C_KILL(k_info, k_head->info_num, inven_kind);

    /* Hack -- Free the "fake" arrays */
    C_KILL(k_name, fake_name_size, char);
    C_KILL(k_text, fake_text_size, char);

    /* Forget the array sizes */
    fake_name_size = 0;
    fake_text_size = 0;

#endif	/* ALLOW_TEMPLATES */


    /*** Load the binary image file ***/

    /* Construct the name of the "raw" file */
    sprintf(buf, "%s%s", ANGBAND_DIR_DATA, "k_info.raw");

    /* Attempt to open the "raw" file */
    fd = fd_open(buf, O_RDONLY | O_BINARY, 0);

    /* Process existing "raw" file */
    if (fd < 0) quit("Cannot load 'k_info.raw' file.");

    /* Attempt to parse the "raw" file */
    err = init_k_info_raw(fd);

    /* Close it */
    (void)fd_close(fd);

    /* Error */
    if (err) quit("Cannot parse 'k_info.raw' file.");

    /* Success */
    return (0);
}



/*
 * Initialize the "a_info" array, by parsing a binary "image" file
 */
static errr init_a_info_raw(int fd)
{
    header test;


    /* Read the header */
    fd_read(fd, (char*)(&test), sizeof(header));

    /* Read and Verify the version info */
    if ((test.v_major != a_head->v_major) ||
        (test.v_minor != a_head->v_minor) ||
        (test.v_patch != a_head->v_patch) ||
        (test.v_extra != a_head->v_extra) ||
        (test.info_num != a_head->info_num) ||
        (test.info_len != a_head->info_len) ||
        (test.head_size != a_head->head_size) ||
        (test.info_size != a_head->info_size)) {

        /* Error */
        return (-1);
    }


    /* Accept the header */
    (*a_head) = test;


    /* Allocate the "a_info" array */
    C_MAKE(a_info, a_head->info_num, artifact_type);

    /* Allocate the "a_name" array */
    C_MAKE(a_name, a_head->name_size, char);

    /* Allocate the "a_text" array */
    C_MAKE(a_text, a_head->text_size, char);


    /* Read the "a_info" array */
    fd_read(fd, (char*)(a_info), a_head->info_size);

    /* Read the "a_name" array */
    fd_read(fd, (char*)(a_name), a_head->name_size);

    /* Read the "a_text" array */
    fd_read(fd, (char*)(a_text), a_head->text_size);


    /* Success */
    return (0);
}



/*
 * Initialize the "a_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_a_info(void)
{
    int fd;

    int mode = 0644;

    errr err;

    FILE *fp;

    /* General buffer */
    char buf[1024];


    /*** Make the "header" ***/

    /* Allocate the "header" */
    MAKE(a_head, header);

    /* Save the "version" */
    a_head->v_major = VERSION_MAJOR;
    a_head->v_minor = VERSION_MINOR;
    a_head->v_patch = VERSION_PATCH;
    a_head->v_extra = 0;

    /* Save the "record" information */
    a_head->info_num = MAX_A_IDX;
    a_head->info_len = sizeof(artifact_type);

    /* Save the size of "a_head" and "a_info" */
    a_head->head_size = sizeof(header);
    a_head->info_size = a_head->info_num * a_head->info_len;


#ifdef ALLOW_TEMPLATES

    /*** Load the binary image file ***/

    /* Construct the name of the "raw" file */
    sprintf(buf, "%s%s", ANGBAND_DIR_DATA, "a_info.raw");

    /* Attempt to open the "raw" file */
    fd = fd_open(buf, O_RDONLY | O_BINARY, 0);

    /* Process existing "raw" file */
    if (fd >= 0) {

        /* Attempt to parse the "raw" file */
        err = init_a_info_raw(fd);

        /* Close it */
        (void)fd_close(fd);

        /* Success */
        if (!err) return (0);

        /* Information */
        msg_print("Ignoring obsolete/defective 'a_info.raw' file.");
        msg_print(NULL);
    }


    /*** Make the fake arrays ***/

    /* Fake the size of "a_name" and "a_text" */
    fake_name_size = 20 * 1024L;
    fake_text_size = 60 * 1024L;

    /* Allocate the "a_info" array */
    C_MAKE(a_info, a_head->info_num, artifact_type);

    /* Hack -- make "fake" arrays */
    C_MAKE(a_name, fake_name_size, char);
    C_MAKE(a_text, fake_text_size, char);


    /*** Load the ascii template file ***/

    /* Access the "a_info.txt" file */
    sprintf(buf, "%s%s", ANGBAND_DIR_EDIT, "a_info.txt");

    /* Open the file */
    fp = my_fopen(buf, "r");

    /* Parse it */
    if (!fp) quit("Cannot open 'a_info.txt' file.");

    /* Parse the file */
    err = init_a_info_txt(fp, buf);

    /* Close it */
    my_fclose(fp);

    /* Errors */
    if (err) {

        cptr oops;
        
        /* Error string */
        oops = ((err > 0) ? err_str[err] : "unknown");

        /* Oops */
        msg_format("Error at line %d of 'a_info.txt' file.", error_line);
        msg_format("Record %d contains a '%s' error.", error_idx, oops);
        msg_format("Parsing '%s'.", buf);
        msg_print(NULL);

        /* Quit */
        quit("Error in 'a_info.txt' file.");
    }


    /*** Dump the binary image file ***/

#if defined(MACINTOSH) && !defined(applec)
    /* Global -- "data file" */
    _ftype = 'DATA';
#endif

    /* Construct the name of the raw file */
    sprintf(buf, "%s%s", ANGBAND_DIR_DATA, "a_info.raw");

    /* Attempt to create the raw file */
    fd = fd_open(buf, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, mode);

    /* Dump to the file */
    if (fd >= 0) {

        /* Dump it */
        fd_write(fd, (char*)(a_head), a_head->head_size);

        /* Dump the "a_info" array */
        fd_write(fd, (char*)(a_info), a_head->info_size);

        /* Dump the "a_name" array */
        fd_write(fd, (char*)(a_name), a_head->name_size);

        /* Dump the "a_text" array */
        fd_write(fd, (char*)(a_text), a_head->text_size);

        /* Close */
        (void)fd_close(fd);
    }


    /*** Kill the fake arrays ***/

    /* Free the "a_info" array */
    C_KILL(a_info, a_head->info_num, artifact_type);

    /* Hack -- Free the "fake" arrays */
    C_KILL(a_name, fake_name_size, char);
    C_KILL(a_text, fake_text_size, char);

    /* Forget the array sizes */
    fake_name_size = 0;
    fake_text_size = 0;

#endif	/* ALLOW_TEMPLATES */


    /*** Load the binary image file ***/

    /* Construct the name of the "raw" file */
    sprintf(buf, "%s%s", ANGBAND_DIR_DATA, "a_info.raw");

    /* Attempt to open the "raw" file */
    fd = fd_open(buf, O_RDONLY | O_BINARY, 0);

    /* Process existing "raw" file */
    if (fd < 0) quit("Cannot open 'a_info.raw' file.");

    /* Attempt to parse the "raw" file */
    err = init_a_info_raw(fd);

    /* Close it */
    (void)fd_close(fd);

    /* Error */
    if (err) quit("Cannot parse 'a_info.raw' file.");

    /* Success */
    return (0);
}



/*
 * Initialize the "e_info" array, by parsing a binary "image" file
 */
static errr init_e_info_raw(int fd)
{
    header test;


    /* Read the header */
    fd_read(fd, (char*)(&test), sizeof(header));

    /* Read and Verify the version info */
    if ((test.v_major != e_head->v_major) ||
        (test.v_minor != e_head->v_minor) ||
        (test.v_patch != e_head->v_patch) ||
        (test.v_extra != e_head->v_extra) ||
        (test.info_num != e_head->info_num) ||
        (test.info_len != e_head->info_len) ||
        (test.head_size != e_head->head_size) ||
        (test.info_size != e_head->info_size)) {

        /* Error */
        return (-1);
    }


    /* Accept the header */
    (*e_head) = test;


    /* Allocate the "e_info" array */
    C_MAKE(e_info, e_head->info_num, ego_item_type);

    /* Allocate the "e_name" array */
    C_MAKE(e_name, e_head->name_size, char);

    /* Allocate the "e_text" array */
    C_MAKE(e_text, e_head->text_size, char);


    /* Read the "e_info" array */
    fd_read(fd, (char*)(e_info), e_head->info_size);

    /* Read the "e_name" array */
    fd_read(fd, (char*)(e_name), e_head->name_size);

    /* Read the "e_text" array */
    fd_read(fd, (char*)(e_text), e_head->text_size);


    /* Success */
    return (0);
}



/*
 * Initialize the "e_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_e_info(void)
{
    int fd;

    int mode = 0644;

    errr err;

    FILE *fp;

    /* General buffer */
    char buf[1024];


    /*** Make the "header" ***/

    /* Allocate the "header" */
    MAKE(e_head, header);

    /* Save the "version" */
    e_head->v_major = VERSION_MAJOR;
    e_head->v_minor = VERSION_MINOR;
    e_head->v_patch = VERSION_PATCH;
    e_head->v_extra = 0;

    /* Save the "record" information */
    e_head->info_num = MAX_E_IDX;
    e_head->info_len = sizeof(ego_item_type);

    /* Save the size of "e_head" and "e_info" */
    e_head->head_size = sizeof(header);
    e_head->info_size = e_head->info_num * e_head->info_len;


#ifdef ALLOW_TEMPLATES

    /*** Load the binary image file ***/

    /* Construct the name of the "raw" file */
    sprintf(buf, "%s%s", ANGBAND_DIR_DATA, "e_info.raw");

    /* Attempt to open the "raw" file */
    fd = fd_open(buf, O_RDONLY | O_BINARY, 0);

    /* Process existing "raw" file */
    if (fd >= 0) {

        /* Attempt to parse the "raw" file */
        err = init_e_info_raw(fd);

        /* Close it */
        (void)fd_close(fd);

        /* Success */
        if (!err) return (0);

        /* Information */
        msg_print("Ignoring obsolete/defective 'e_info.raw' file.");
        msg_print(NULL);
    }


    /*** Make the fake arrays ***/

    /* Fake the size of "e_name" and "e_text" */
    fake_name_size = 20 * 1024L;
    fake_text_size = 60 * 1024L;

    /* Allocate the "e_info" array */
    C_MAKE(e_info, e_head->info_num, ego_item_type);

    /* Hack -- make "fake" arrays */
    C_MAKE(e_name, fake_name_size, char);
    C_MAKE(e_text, fake_text_size, char);


    /*** Load the ascii template file ***/

    /* Access the "e_info.txt" file */
    sprintf(buf, "%s%s", ANGBAND_DIR_EDIT, "e_info.txt");

    /* Open the file */
    fp = my_fopen(buf, "r");

    /* Parse it */
    if (!fp) quit("Cannot open 'e_info.txt' file.");

    /* Parse the file */
    err = init_e_info_txt(fp, buf);

    /* Close it */
    my_fclose(fp);

    /* Errors */
    if (err) {

        cptr oops;
        
        /* Error string */
        oops = ((err > 0) ? err_str[err] : "unknown");

        /* Oops */
        msg_format("Error at line %d of 'e_info.txt' file.", error_line);
        msg_format("Record %d contains a '%s' error.", error_idx, oops);
        msg_format("Parsing '%s'.", buf);
        msg_print(NULL);

        /* Quit */
        quit("Error in 'e_info.txt' file.");
    }


    /*** Dump the binary image file ***/

#if defined(MACINTOSH) && !defined(applec)
    /* Global -- "data file" */
    _ftype = 'DATA';
#endif

    /* Construct the name of the raw file */
    sprintf(buf, "%s%s", ANGBAND_DIR_DATA, "e_info.raw");

    /* Attempt to create the raw file */
    fd = fd_open(buf, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, mode);

    /* Dump to the file */
    if (fd >= 0) {

        /* Dump it */
        fd_write(fd, (char*)(e_head), e_head->head_size);

        /* Dump the "e_info" array */
        fd_write(fd, (char*)(e_info), e_head->info_size);

        /* Dump the "e_name" array */
        fd_write(fd, (char*)(e_name), e_head->name_size);

        /* Dump the "e_text" array */
        fd_write(fd, (char*)(e_text), e_head->text_size);

        /* Close */
        (void)fd_close(fd);
    }


    /*** Kill the fake arrays ***/

    /* Free the "e_info" array */
    C_KILL(e_info, e_head->info_num, ego_item_type);

    /* Hack -- Free the "fake" arrays */
    C_KILL(e_name, fake_name_size, char);
    C_KILL(e_text, fake_text_size, char);

    /* Forget the array sizes */
    fake_name_size = 0;
    fake_text_size = 0;

#endif	/* ALLOW_TEMPLATES */


    /*** Load the binary image file ***/

    /* Construct the name of the "raw" file */
    sprintf(buf, "%s%s", ANGBAND_DIR_DATA, "e_info.raw");

    /* Attempt to open the "raw" file */
    fd = fd_open(buf, O_RDONLY | O_BINARY, 0);

    /* Process existing "raw" file */
    if (fd < 0) quit("Cannot load 'e_info.raw' file.");

    /* Attempt to parse the "raw" file */
    err = init_e_info_raw(fd);

    /* Close it */
    (void)fd_close(fd);

    /* Error */
    if (err) quit("Cannot parse 'e_info.raw' file.");

    /* Success */
    return (0);
}



/*
 * Initialize the "r_info" array, by parsing a binary "image" file
 */
static errr init_r_info_raw(int fd)
{
    header test;

    /* Read the header */
    fd_read(fd, (char*)(&test), sizeof(header));

    /* Verify the version info */
    if ((test.v_major != r_head->v_major) ||
        (test.v_minor != r_head->v_minor) ||
        (test.v_patch != r_head->v_patch) ||
        (test.v_extra != r_head->v_extra) ||
        (test.info_num != r_head->info_num) ||
        (test.info_len != r_head->info_len) ||
        (test.head_size != r_head->head_size) ||
        (test.info_size != r_head->info_size)) {

        /* Error */
        return (-1);
    }


    /* Accept the header */
    (*r_head) = test;


    /* Allocate the "r_info" array */
    C_MAKE(r_info, r_head->info_num, monster_race);

    /* Allocate the "r_name" array */
    C_MAKE(r_name, r_head->name_size, char);

    /* Allocate the "r_text" array */
    C_MAKE(r_text, r_head->text_size, char);


    /* Read the "r_info" array */
    fd_read(fd, (char*)(r_info), r_head->info_size);

    /* Read the "r_name" array */
    fd_read(fd, (char*)(r_name), r_head->name_size);

    /* Read the "r_text" array */
    fd_read(fd, (char*)(r_text), r_head->text_size);


    /* Success */
    return (0);
}



/*
 * Initialize the "r_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_r_info(void)
{
    int fd;

    int mode = 0644;

    errr err;

    FILE *fp;

    /* General buffer */
    char buf[1024];


    /*** Make the header ***/

    /* Allocate the "header" */
    MAKE(r_head, header);

    /* Save the "version" */
    r_head->v_major = VERSION_MAJOR;
    r_head->v_minor = VERSION_MINOR;
    r_head->v_patch = VERSION_PATCH;
    r_head->v_extra = 0;

    /* Save the "record" information */
    r_head->info_num = MAX_R_IDX;
    r_head->info_len = sizeof(monster_race);

    /* Save the size of "r_head" and "r_info" */
    r_head->head_size = sizeof(header);
    r_head->info_size = r_head->info_num * r_head->info_len;


#ifdef ALLOW_TEMPLATES

    /*** Load the binary image file ***/

    /* Construct the name of the "raw" file */
    sprintf(buf, "%s%s", ANGBAND_DIR_DATA, "r_info.raw");

    /* Attempt to open the "raw" file */
    fd = fd_open(buf, O_RDONLY | O_BINARY, 0);

    /* Process existing "raw" file */
    if (fd >= 0) {

        /* Attempt to parse the "raw" file */
        err = init_r_info_raw(fd);

        /* Close it */
        (void)fd_close(fd);

        /* Success */
        if (!err) return (0);

        /* Information */
        msg_print("Ignoring obsolete/defective 'r_info.raw' file.");
        msg_print(NULL);
    }


    /*** Make the fake arrays ***/

    /* Assume the size of "r_name" and "r_text" */
    fake_name_size = 20 * 1024L;
    fake_text_size = 60 * 1024L;

    /* Allocate the "r_info" array */
    C_MAKE(r_info, r_head->info_num, monster_race);

    /* Hack -- make "fake" arrays */
    C_MAKE(r_name, fake_name_size, char);
    C_MAKE(r_text, fake_text_size, char);


    /*** Load the ascii template file ***/

    /* Access the "r_info.txt" file */
    sprintf(buf, "%s%s", ANGBAND_DIR_EDIT, "r_info.txt");

    /* Open the file */
    fp = my_fopen(buf, "r");

    /* Parse it */
    if (!fp) quit("Cannot open 'r_info.txt' file.");

    /* Parse the file */
    err = init_r_info_txt(fp, buf);

    /* Close it */
    my_fclose(fp);

    /* Errors */
    if (err) {

        cptr oops;
        
        /* Error string */
        oops = ((err > 0) ? err_str[err] : "unknown");

        /* Oops */
        msg_format("Error at line %d of 'r_info.txt' file.", error_line);
        msg_format("Record %d contains a '%s' error.", error_idx, oops);
        msg_format("Parsing '%s'.", buf);
        msg_print(NULL);

        /* Quit */
        quit("Error in 'r_info.txt' file.");
    }


    /*** Dump the binary image file ***/

#if defined(MACINTOSH) && !defined(applec)
    /* Global -- "data file" */
    _ftype = 'DATA';
#endif

    /* Construct the name of the raw file */
    sprintf(buf, "%s%s", ANGBAND_DIR_DATA, "r_info.raw");

    /* Attempt to create the raw file */
    fd = fd_open(buf, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, mode);

    /* Dump to the file */
    if (fd >= 0) {

        /* Dump it */
        fd_write(fd, (char*)(r_head), r_head->head_size);

        /* Dump the "r_info" array */
        fd_write(fd, (char*)(r_info), r_head->info_size);

        /* Dump the "r_name" array */
        fd_write(fd, (char*)(r_name), r_head->name_size);

        /* Dump the "r_text" array */
        fd_write(fd, (char*)(r_text), r_head->text_size);

        /* Close */
        (void)fd_close(fd);
    }


    /*** Kill the fake arrays ***/

    /* Free the "r_info" array */
    C_KILL(r_info, r_head->info_num, monster_race);

    /* Hack -- Free the "fake" arrays */
    C_KILL(r_name, fake_name_size, char);
    C_KILL(r_text, fake_text_size, char);

    /* Forget the array sizes */
    fake_name_size = 0;
    fake_text_size = 0;

#endif	/* ALLOW_TEMPLATES */


    /*** Load the binary image file ***/

    /* Construct the name of the "raw" file */
    sprintf(buf, "%s%s", ANGBAND_DIR_DATA, "r_info.raw");

    /* Attempt to open the "raw" file */
    fd = fd_open(buf, O_RDONLY | O_BINARY, 0);

    /* Process existing "raw" file */
    if (fd < 0) quit("Cannot load 'r_info.raw' file.");

    /* Attempt to parse the "raw" file */
    err = init_r_info_raw(fd);

    /* Close it */
    (void)fd_close(fd);

    /* Error */
    if (err) quit("Cannot parse 'r_info.raw' file.");
    
    /* Success */
    return (0);
}



/*
 * Initialize the "v_info" array, by parsing a binary "image" file
 */
static errr init_v_info_raw(int fd)
{
    header test;


    /* Read the header */
    fd_read(fd, (char*)(&test), sizeof(header));

    /* Read and Verify the version info */
    if ((test.v_major != v_head->v_major) ||
        (test.v_minor != v_head->v_minor) ||
        (test.v_patch != v_head->v_patch) ||
        (test.v_extra != v_head->v_extra) ||
        (test.info_num != v_head->info_num) ||
        (test.info_len != v_head->info_len) ||
        (test.head_size != v_head->head_size) ||
        (test.info_size != v_head->info_size)) {

        /* Error */
        return (-1);
    }


    /* Accept the header */
    (*v_head) = test;


    /* Allocate the "v_info" array */
    C_MAKE(v_info, v_head->info_num, vault_type);

    /* Allocate the "k_name" array */
    C_MAKE(v_name, v_head->name_size, char);

    /* Allocate the "k_text" array */
    C_MAKE(v_text, v_head->text_size, char);


    /* Read the "v_info" array */
    fd_read(fd, (char*)(v_info), v_head->info_size);

    /* Read the "v_name" array */
    fd_read(fd, (char*)(v_name), v_head->name_size);

    /* Read the "v_text" array */
    fd_read(fd, (char*)(v_text), v_head->text_size);


    /* Success */
    return (0);
}



/*
 * Initialize the "v_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_v_info(void)
{
    int fd;

    int mode = 0644;

    errr err;

    FILE *fp;

    /* General buffer */
    char buf[1024];


    /*** Make the header ***/

    /* Allocate the "header" */
    MAKE(v_head, header);

    /* Save the "version" */
    v_head->v_major = VERSION_MAJOR;
    v_head->v_minor = VERSION_MINOR;
    v_head->v_patch = VERSION_PATCH;
    v_head->v_extra = 0;

    /* Save the "record" information */
    v_head->info_num = MAX_V_IDX;
    v_head->info_len = sizeof(feature_type);

    /* Save the size of "v_head" and "v_info" */
    v_head->head_size = sizeof(header);
    v_head->info_size = v_head->info_num * v_head->info_len;


#ifdef ALLOW_TEMPLATES

    /*** Load the binary image file ***/

    /* Construct the name of the "raw" file */
    sprintf(buf, "%s%s", ANGBAND_DIR_DATA, "v_info.raw");

    /* Attempt to open the "raw" file */
    fd = fd_open(buf, O_RDONLY | O_BINARY, 0);

    /* Process existing "raw" file */
    if (fd >= 0) {

        /* Attempt to parse the "raw" file */
        err = init_v_info_raw(fd);

        /* Close it */
        (void)fd_close(fd);

        /* Success */
        if (!err) return (0);

        /* Information */
        msg_print("Ignoring obsolete/defective 'v_info.raw' file.");
        msg_print(NULL);
    }


    /*** Make the fake arrays ***/

    /* Fake the size of "v_name" and "v_text" */
    fake_name_size = 20 * 1024L;
    fake_text_size = 60 * 1024L;

    /* Allocate the "k_info" array */
    C_MAKE(v_info, v_head->info_num, vault_type);

    /* Hack -- make "fake" arrays */
    C_MAKE(v_name, fake_name_size, char);
    C_MAKE(v_text, fake_text_size, char);


    /*** Load the ascii template file ***/

    /* Access the "v_info.txt" file */
    sprintf(buf, "%s%s", ANGBAND_DIR_EDIT, "v_info.txt");

    /* Open the file */
    fp = my_fopen(buf, "r");

    /* Parse it */
    if (!fp) quit("Cannot open 'v_info.txt' file.");

    /* Parse the file */
    err = init_v_info_txt(fp, buf);

    /* Close it */
    my_fclose(fp);

    /* Errors */
    if (err) {

        cptr oops;
        
        /* Error string */
        oops = ((err > 0) ? err_str[err] : "unknown");

        /* Oops */
        msg_format("Error at line %d of 'v_info.txt' file.", error_line);
        msg_format("Record %d contains a '%s' error.", error_idx, oops);
        msg_format("Parsing '%s'.", buf);
        msg_print(NULL);

        /* Quit */
        quit("Error in 'v_info.txt' file.");
    }


    /*** Dump the binary image file ***/

#if defined(MACINTOSH) && !defined(applec)
    /* Global -- "data file" */
    _ftype = 'DATA';
#endif

    /* Construct the name of the raw file */
    sprintf(buf, "%s%s", ANGBAND_DIR_DATA, "v_info.raw");

    /* Attempt to create the raw file */
    fd = fd_open(buf, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, mode);

    /* Dump to the file */
    if (fd >= 0) {

        /* Dump it */
        fd_write(fd, (char*)(v_head), v_head->head_size);

        /* Dump the "v_info" array */
        fd_write(fd, (char*)(v_info), v_head->info_size);

        /* Dump the "v_name" array */
        fd_write(fd, (char*)(v_name), v_head->name_size);

        /* Dump the "v_text" array */
        fd_write(fd, (char*)(v_text), v_head->text_size);

        /* Close */
        (void)fd_close(fd);
    }


    /*** Kill the fake arrays ***/

    /* Free the "v_info" array */
    C_KILL(v_info, v_head->info_num, vault_type);

    /* Hack -- Free the "fake" arrays */
    C_KILL(v_name, fake_name_size, char);
    C_KILL(v_text, fake_text_size, char);

    /* Forget the array sizes */
    fake_name_size = 0;
    fake_text_size = 0;

#endif	/* ALLOW_TEMPLATES */


    /*** Load the binary image file ***/

    /* Construct the name of the "raw" file */
    sprintf(buf, "%s%s", ANGBAND_DIR_DATA, "v_info.raw");

    /* Attempt to open the "raw" file */
    fd = fd_open(buf, O_RDONLY | O_BINARY, 0);

    /* Process existing "raw" file */
    if (fd < 0) quit("Cannot load 'v_info.raw' file.");

    /* Attempt to parse the "raw" file */
    err = init_v_info_raw(fd);

    /* Close it */
    (void)fd_close(fd);

    /* Error */
    if (err) quit("Cannot parse 'v_info.raw' file.");

    /* Success */
    return (0);
}




/*** Initialize others ***/



/*
 * Hack -- Objects sold in the stores -- by tval/sval pair.
 */
static byte store_table[MAX_STORES-2][STORE_CHOICES][2] = {

  {
    /* General Store */
    { TV_FOOD, SV_FOOD_RATION },
    { TV_FOOD, SV_FOOD_RATION },
    { TV_FOOD, SV_FOOD_RATION },
    { TV_FOOD, SV_FOOD_RATION },
    { TV_FOOD, SV_FOOD_RATION },
    { TV_FOOD, SV_FOOD_RATION },
    { TV_FOOD, SV_FOOD_BISCUIT },
    { TV_FOOD, SV_FOOD_JERKY },
    { TV_FOOD, SV_FOOD_PINT_OF_WINE },
    { TV_FOOD, SV_FOOD_PINT_OF_ALE },
    { TV_DIGGING, SV_SHOVEL },
    { TV_DIGGING, SV_PICK },
    { TV_CLOAK, SV_CLOAK },
    { TV_CLOAK, SV_CLOAK },
    { TV_CLOAK, SV_CLOAK },
    { TV_POTION, SV_POTION_WATER },
    { TV_POTION, SV_POTION_APPLE_JUICE },
    { TV_SPIKE, 0 },
    { TV_SPIKE, 0 },
    { TV_SPIKE, 0 },
    { TV_LITE, SV_LITE_TORCH },
    { TV_LITE, SV_LITE_TORCH },
    { TV_LITE, SV_LITE_TORCH },
    { TV_LITE, SV_LITE_TORCH },
    { TV_LITE, SV_LITE_LANTERN },
    { TV_FLASK, 0 },
    { TV_FLASK, 0 },
    { TV_FLASK, 0 },
    { TV_FLASK, 0 },
    { TV_FLASK, 0 }
  },

  {
    /* Armoury */
    { TV_BOOTS, SV_PAIR_OF_SOFT_LEATHER_BOOTS },
    { TV_BOOTS, SV_PAIR_OF_SOFT_LEATHER_BOOTS },
    { TV_BOOTS, SV_PAIR_OF_HARD_LEATHER_BOOTS },
    { TV_HELM, SV_HARD_LEATHER_CAP },
    { TV_HELM, SV_HARD_LEATHER_CAP },
    { TV_HELM, SV_METAL_CAP },
    { TV_HELM, SV_IRON_HELM },
    { TV_SOFT_ARMOR, SV_ROBE },
    { TV_SOFT_ARMOR, SV_ROBE },
    { TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR },
    { TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR },
    { TV_SOFT_ARMOR, SV_HARD_LEATHER_ARMOR },
    { TV_SOFT_ARMOR, SV_HARD_STUDDED_LEATHER },
    { TV_SOFT_ARMOR, SV_LEATHER_SCALE_MAIL },
    { TV_SOFT_ARMOR, SV_LEATHER_SCALE_MAIL },
    { TV_HARD_ARMOR, SV_METAL_SCALE_MAIL },
    { TV_HARD_ARMOR, SV_CHAIN_MAIL },
    { TV_HARD_ARMOR, SV_CHAIN_MAIL },
    { TV_HARD_ARMOR, SV_AUGMENTED_CHAIN_MAIL },
    { TV_HARD_ARMOR, SV_BAR_CHAIN_MAIL },
    { TV_HARD_ARMOR, SV_METAL_BRIGANDINE_ARMOUR },
    { TV_HARD_ARMOR, SV_DOUBLE_CHAIN_MAIL },
    { TV_GLOVES, SV_SET_OF_LEATHER_GLOVES },
    { TV_GLOVES, SV_SET_OF_LEATHER_GLOVES },
    { TV_GLOVES, SV_SET_OF_GAUNTLETS },
    { TV_SHIELD, SV_SMALL_LEATHER_SHIELD },
    { TV_SHIELD, SV_SMALL_LEATHER_SHIELD },
    { TV_SHIELD, SV_LARGE_LEATHER_SHIELD },
    { TV_SHIELD, SV_SMALL_METAL_SHIELD }
  },

  {
    /* Weaponsmith */
    { TV_SWORD, SV_DAGGER },
    { TV_SWORD, SV_MAIN_GAUCHE },
    { TV_SWORD, SV_RAPIER },
    { TV_SWORD, SV_SMALL_SWORD },
    { TV_SWORD, SV_SHORT_SWORD },
    { TV_SWORD, SV_SABRE },
    { TV_SWORD, SV_CUTLASS },
    { TV_SWORD, SV_TULWAR },
    { TV_SWORD, SV_BROAD_SWORD },
    { TV_SWORD, SV_LONG_SWORD },
    { TV_SWORD, SV_SCIMITAR },
    { TV_SWORD, SV_KATANA },
    { TV_SWORD, SV_BASTARD_SWORD },
    { TV_POLEARM, SV_SPEAR },
    { TV_POLEARM, SV_AWL_PIKE },
    { TV_POLEARM, SV_TRIDENT },
    { TV_POLEARM, SV_PIKE },
    { TV_POLEARM, SV_BEAKED_AXE },
    { TV_POLEARM, SV_BROAD_AXE },
    { TV_POLEARM, SV_LANCE },
    { TV_POLEARM, SV_BATTLE_AXE },
    { TV_HAFTED, SV_WHIP },
    { TV_BOW, SV_SLING },
    { TV_BOW, SV_SHORT_BOW },
    { TV_BOW, SV_LONG_BOW },
    { TV_BOW, SV_LIGHT_XBOW },
    { TV_SHOT, SV_AMMO_LIGHT },
    { TV_SHOT, SV_AMMO_NORMAL },
    { TV_ARROW, SV_AMMO_NORMAL },
    { TV_BOLT, SV_AMMO_NORMAL },
  },

  {
    /* Temple */
    { TV_HAFTED, SV_WHIP },
    { TV_HAFTED, SV_QUARTERSTAFF },
    { TV_HAFTED, SV_MACE },
    { TV_HAFTED, SV_BALL_AND_CHAIN },
    { TV_HAFTED, SV_WAR_HAMMER },
    { TV_HAFTED, SV_LUCERN_HAMMER },
    { TV_HAFTED, SV_MORNING_STAR },
    { TV_HAFTED, SV_FLAIL },
    { TV_HAFTED, SV_LEAD_FILLED_MACE },
    { TV_SCROLL, SV_SCROLL_REMOVE_CURSE },
    { TV_SCROLL, SV_SCROLL_BLESSING },
    { TV_SCROLL, SV_SCROLL_HOLY_CHANT },
    { TV_POTION, SV_POTION_BOLDNESS },
    { TV_POTION, SV_POTION_HEROISM },
    { TV_POTION, SV_POTION_CURE_LIGHT },
    { TV_POTION, SV_POTION_CURE_SERIOUS },
    { TV_POTION, SV_POTION_CURE_SERIOUS },
    { TV_POTION, SV_POTION_CURE_CRITICAL },
    { TV_POTION, SV_POTION_CURE_CRITICAL },
    { TV_POTION, SV_POTION_RESTORE_EXP },
    { TV_POTION, SV_POTION_RESTORE_EXP },
    { TV_POTION, SV_POTION_RES_WIS },
    { TV_PRAYER_BOOK, 0 },
    { TV_PRAYER_BOOK, 0 },
    { TV_PRAYER_BOOK, 0 },
    { TV_PRAYER_BOOK, 1 },
    { TV_PRAYER_BOOK, 1 },
    { TV_PRAYER_BOOK, 2 },
    { TV_PRAYER_BOOK, 2 },
    { TV_PRAYER_BOOK, 3 }
  },

  {
    /* Alchemy shop */
    { TV_SCROLL, SV_SCROLL_ENCHANT_WEAPON_TO_HIT },
    { TV_SCROLL, SV_SCROLL_ENCHANT_WEAPON_TO_DAM },
    { TV_SCROLL, SV_SCROLL_ENCHANT_ARMOR },
    { TV_SCROLL, SV_SCROLL_IDENTIFY },
    { TV_SCROLL, SV_SCROLL_IDENTIFY },
    { TV_SCROLL, SV_SCROLL_IDENTIFY },
    { TV_SCROLL, SV_SCROLL_IDENTIFY },
    { TV_SCROLL, SV_SCROLL_LIGHT },
    { TV_SCROLL, SV_SCROLL_PHASE_DOOR },
    { TV_SCROLL, SV_SCROLL_PHASE_DOOR },
    { TV_SCROLL, SV_SCROLL_MONSTER_CONFUSION },
    { TV_SCROLL, SV_SCROLL_MAPPING },
    { TV_SCROLL, SV_SCROLL_DETECT_GOLD },
    { TV_SCROLL, SV_SCROLL_DETECT_ITEM },
    { TV_SCROLL, SV_SCROLL_DETECT_TRAP },
    { TV_SCROLL, SV_SCROLL_DETECT_DOOR },
    { TV_SCROLL, SV_SCROLL_DETECT_INVIS },
    { TV_SCROLL, SV_SCROLL_RECHARGING },
    { TV_SCROLL, SV_SCROLL_SATISFY_HUNGER },
    { TV_SCROLL, SV_SCROLL_WORD_OF_RECALL },
    { TV_SCROLL, SV_SCROLL_WORD_OF_RECALL },
    { TV_SCROLL, SV_SCROLL_WORD_OF_RECALL },
    { TV_POTION, SV_POTION_RESIST_HEAT },
    { TV_POTION, SV_POTION_RESIST_COLD },
    { TV_POTION, SV_POTION_RES_STR },
    { TV_POTION, SV_POTION_RES_INT },
    { TV_POTION, SV_POTION_RES_WIS },
    { TV_POTION, SV_POTION_RES_DEX },
    { TV_POTION, SV_POTION_RES_CON },
    { TV_POTION, SV_POTION_RES_CHR }
  },

  {
    /* Magic-User store */
    { TV_RING, SV_RING_SEARCHING },
    { TV_RING, SV_RING_FEATHER_FALL },
    { TV_RING, SV_RING_PROTECTION },
    { TV_AMULET, SV_AMULET_CHARISMA },
    { TV_AMULET, SV_AMULET_SLOW_DIGEST },
    { TV_AMULET, SV_AMULET_RESIST_ACID },
    { TV_WAND, SV_WAND_SLOW_MONSTER },
    { TV_WAND, SV_WAND_CONFUSE_MONSTER },
    { TV_WAND, SV_WAND_SLEEP_MONSTER },
    { TV_WAND, SV_WAND_MAGIC_MISSILE },
    { TV_WAND, SV_WAND_STINKING_CLOUD },
    { TV_WAND, SV_WAND_WONDER },
    { TV_STAFF, SV_STAFF_LITE },
    { TV_STAFF, SV_STAFF_MAPPING },
    { TV_STAFF, SV_STAFF_DETECT_TRAP },
    { TV_STAFF, SV_STAFF_DETECT_DOOR },
    { TV_STAFF, SV_STAFF_DETECT_GOLD },
    { TV_STAFF, SV_STAFF_DETECT_ITEM },
    { TV_STAFF, SV_STAFF_DETECT_INVIS },
    { TV_STAFF, SV_STAFF_DETECT_EVIL },
    { TV_STAFF, SV_STAFF_TELEPORTATION },
    { TV_STAFF, SV_STAFF_IDENTIFY },
    { TV_MAGIC_BOOK, 0 },
    { TV_MAGIC_BOOK, 0 },
    { TV_MAGIC_BOOK, 0 },
    { TV_MAGIC_BOOK, 1 },
    { TV_MAGIC_BOOK, 1 },
    { TV_MAGIC_BOOK, 2 },
    { TV_MAGIC_BOOK, 2 },
    { TV_MAGIC_BOOK, 3 }
  }
};




/*
 * Initialize some other arrays
 */
static errr init_other(void)
{
    int i, j, k;

    inven_kind *k_ptr;
    monster_race *r_ptr;

    s16b aux[256];


    /*** Prepare the "dungeon" information ***/

    /* Allocate and Wipe the object list */
    C_MAKE(i_list, MAX_I_IDX, inven_type);

    /* Allocate and Wipe the monster list */
    C_MAKE(m_list, MAX_M_IDX, monster_type);


    /* Allocate and wipe each line of the cave */
    for (i = 0; i < MAX_HGT; i++)
    {
        /* Allocate one row of the cave */
        C_MAKE(cave[i], MAX_WID, cave_type);
    }


    /*** Prepare the various "grid" arrays ***/

    /* The "lite" array */
    C_MAKE(lite_x, LITE_MAX, byte);
    C_MAKE(lite_y, LITE_MAX, byte);

    /* The "view" array */
    C_MAKE(view_x, VIEW_MAX, byte);
    C_MAKE(view_y, VIEW_MAX, byte);

    /* The "temp" array */
    C_MAKE(temp_x, TEMP_MAX, byte);
    C_MAKE(temp_y, TEMP_MAX, byte);


    /*** Prepare the various "bizarre" arrays ***/

    /* Macro's */
    C_MAKE(macro__pat, MACRO_MAX, cptr);
    C_MAKE(macro__act, MACRO_MAX, cptr);
    C_MAKE(macro__cmd, MACRO_MAX, bool);

    /* Quark's */
    C_MAKE(quark__str, QUARK_MAX, cptr);

    /* Message's */
    C_MAKE(message__ptr, MESSAGE_MAX, u16b);
    C_MAKE(message__buf, MESSAGE_BUF, char);

    /* Hack -- No messages yet */
    message__tail = MESSAGE_BUF;


    /*** Prepare the Player inventory ***/

    /* Allocate it */
    C_MAKE(inventory, INVEN_TOTAL, inven_type);


    /*** Prepare the Object Kind Allocator ***/

    /* Clear the aux array */
    C_WIPE(&aux, MAX_DEPTH, s16b);

    /* Make the index */
    C_MAKE(alloc_kind_index, MAX_DEPTH, s16b);

    /* Scan all of the objects */
    for (i = 0; i < MAX_K_IDX; i++) {

        /* Get the i'th object */
        k_ptr = &k_info[i];

        /* Scan all of the locale/chance pairs */
        for (j = 0; j < 4; j++) {

            /* Count valid pairs */
            if (k_ptr->chance[j] && (k_ptr->locale[j] < MAX_DEPTH)) {

                /* Count the total entries */
                alloc_kind_size++;

                /* Count the entries at each level */
                alloc_kind_index[k_ptr->locale[j]]++;
            }
        }
    }

    /* Combine the "alloc_kind_index" entries */
    for (i = 1; i < MAX_DEPTH; i++) alloc_kind_index[i] += alloc_kind_index[i-1];

    /* Allocate the table */
    C_MAKE(alloc_kind_table, alloc_kind_size, kind_entry);

    /* Initialize the table */
    for (i = 0; i < MAX_K_IDX; i++) {

        /* Get the i'th object */
        k_ptr = &k_info[i];

        /* Scan all of the locale/chance pairs */
        for (j = 0; j < 4; j++) {

            /* Count valid pairs */
            if (k_ptr->chance[j] && (k_ptr->locale[j] < MAX_DEPTH)) {

                int r, x, y, z;

                /* Extract the chance/locale */
                r = k_ptr->chance[j];
                x = k_ptr->locale[j];

                /* Skip entries preceding our locale */
                y = (x > 0) ? alloc_kind_index[x-1] : 0;

                /* Skip previous entries at this locale */
                z = y + aux[x];

                /* Load the table entry */
                alloc_kind_table[z].k_idx = i;
                alloc_kind_table[z].locale = x;
                alloc_kind_table[z].chance = r;

                /* Another entry complete for this locale */
                aux[x]++;
            }
        }
    }

    /* Paranoia */
    if (!alloc_kind_index[0]) quit("No town objects!");


    /*** Prepare the Monster Race Allocator ***/

    /* Clear the aux array */
    C_WIPE(&aux, MAX_DEPTH, s16b);

    /* Allocate and clear the index */
    C_MAKE(alloc_race_index, MAX_DEPTH, s16b);

    /* Scan the monsters (not the ghost) */
    for (i = 1; i < MAX_R_IDX - 1; i++) {

        /* Get the i'th race */
        r_ptr = &r_info[i];

        /* Process "real" monsters */
        if (r_ptr->rarity && (r_ptr->level < MAX_DEPTH)) {

            /* Count the total entries */
            alloc_race_size++;

            /* Count the entries at each level */
            alloc_race_index[r_ptr->level]++;
        }
    }

    /* Combine the "alloc_race_index" entries */
    for (i = 1; i < MAX_DEPTH; i++) {
        alloc_race_index[i] += alloc_race_index[i-1];
    }

    /* Allocate the alloc_race_table */
    C_MAKE(alloc_race_table, alloc_race_size, race_entry);

    /* Scan the monsters (not the ghost) */
    for (i = 1; i < MAX_R_IDX - 1; i++) {

        /* Get the i'th race */
        r_ptr = &r_info[i];

        /* Count valid pairs */
        if (r_ptr->rarity && (r_ptr->level < MAX_DEPTH)) {

            int r, x, y, z;

            /* Extract the level/rarity */
            x = r_ptr->level;
            r = r_ptr->rarity;

            /* Skip entries preceding our locale */
            y = (x > 0) ? alloc_race_index[x-1] : 0;

            /* Skip previous entries at this locale */
            z = y + aux[x];

            /* Load the table entry */
            alloc_race_table[z].r_idx = i;
            alloc_race_table[z].locale = x;
            alloc_race_table[z].chance = r;

            /* Another entry complete for this locale */
            aux[x]++;
        }
    }

    /* Paranoia */
    if (!alloc_race_index[0]) quit("No town monsters!");


    /*** Prepare the Stores ***/

    /* Allocate the stores */
    C_MAKE(store, MAX_STORES, store_type);

    /* Fill in each store */
    for (i = 0; i < MAX_STORES; i++) {

        /* Access the store */
        store_type *st_ptr = &store[i];

        /* Assume full stock */
        st_ptr->stock_size = STORE_INVEN_MAX;

        /* Allocate the stock */
        C_MAKE(st_ptr->stock, st_ptr->stock_size, inven_type);

        /* No table for the black market or home */
        if ((i == 6) || (i == 7)) continue;

        /* Assume full table */
        st_ptr->table_size = STORE_CHOICES;

        /* Allocate the stock */
        C_MAKE(st_ptr->table, st_ptr->table_size, s16b);

        /* Scan the choices */
        for (k = 0; k < STORE_CHOICES; k++) {

            /* Extract the tval/sval codes */
            int tv = store_table[i][k][0];
            int sv = store_table[i][k][1];

            /* Locate that item type */
            int k_idx = lookup_kind(tv, sv);

            /* Skip non-existant items */
            if (!k_idx) continue;

            /* Add that item index to the table */
            st_ptr->table[st_ptr->table_num++] = k_idx;
        }
    }


    /*** Pre-allocate the basic "auto-inscriptions" ***/

    /* The "basic" feelings */
    (void)quark_add("cursed");
    (void)quark_add("broken");
    (void)quark_add("average");
    (void)quark_add("good");

    /* The "extra" feelings */
    (void)quark_add("excellent");
    (void)quark_add("worthless");
    (void)quark_add("special");
    (void)quark_add("terrible");

    /* Some extra strings */
    (void)quark_add("uncursed");
    (void)quark_add("on sale");


    /*** Set the "default" options ***/

    /* Scan the options */
    for (i = 0; options[i].o_desc; i++) {

        /* Set the "default" options */
        if (options[i].o_var) (*options[i].o_var) = options[i].o_norm;
    }


    /*** Pre-allocate space for the "format()" buffer ***/

    /* Hack -- Just call the "format()" function */
    (void)format("%s (%s).", "Ben Harrison", MAINTAINER);


    /*** Pre-allocate space for save/restore screen ***/

    /* Save the screen */
    Term_save();

    /* Save the screen (embedded) */
    Term_save();

    /* Restore the screen (embedded) */
    Term_load();

    /* Restore the screen */
    Term_load();


    /* Success */
    return (0);
}



/*
 * Initialize various Angband variables and arrays.
 *
 * This initialization involves the parsing of special files
 * in the "lib/data" and sometimes the "lib/edit" directories.
 *
 * Note that the "template" files are initialized first, since they
 * often contain errors.  This means that macros and message recall
 * and things like that are not available until after they are done.
 */
void init_some_arrays(void)
{
    /* Initialize feature info */
    note("[Initializing arrays... (features)]");
    if (init_f_info()) quit("Cannot initialize features");

    /* Initialize object info */
    note("[Initializing arrays... (objects)]");
    if (init_k_info()) quit("Cannot initialize objects");

    /* Initialize artifact info */
    note("[Initializing arrays... (artifacts)]");
    if (init_a_info()) quit("Cannot initialize artifacts");

    /* Initialize ego-item info */
    note("[Initializing arrays... (ego-items)]");
    if (init_e_info()) quit("Cannot initialize ego-items");

    /* Initialize monster info */
    note("[Initializing arrays... (monsters)]");
    if (init_r_info()) quit("Cannot initialize monsters");

    /* Initialize feature info */
    note("[Initializing arrays... (vaults)]");
    if (init_v_info()) quit("Cannot initialize vaults");

    /* Initialize some other arrays */
    note("[Initializing arrays...]");
    if (init_other()) quit("Cannot initialize arrays");

    /* Hack -- all done */
    note("[Initializing arrays... done]");
}



