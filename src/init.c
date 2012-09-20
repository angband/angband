/* File: init.c */

/* Purpose: initialize various arrays from files -BEN- */

#include "angband.h"


/*
 * Find the paths to all of our important files and directories...
 * Use the ANGBAND_PATH environment var if possible, else use
 * DEFAULT_PATH, and in either case, branch off appropriately.
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
    /* Pointer to the "end" of the workspace */
    char *tail;

    /* The current path (and "workspace") */
    char path[1024];


#ifdef MACINTOSH

    /* Hack -- prepare for Macintosh */
    strcpy(path, ":lib:");

#else

# ifdef WINDOWS

    /* Hack -- prepare for Windows */
    get_lib_path(path);

# else

#  ifdef AMIGA

    /* Hack -- prepare for Amiga */
    strcpy(path, "Angband:");

#  else

    /* Get the environment variable */
    tail = getenv("ANGBAND_PATH");

    /* Use the angband_path, or a default */
    strcpy(path, tail ? tail : DEFAULT_PATH);

    /* Add a path separator (only if needed) */
    if (!suffix(path,PATH_SEP)) strcat(path, PATH_SEP);

#  endif

# endif

#endif


    /* Hack -- save the main directory */
    ANGBAND_DIR = string_make(path);


    /* Prepare to append to the Base Path */
    tail = path + strlen(path);

    /* Make some directory names */
    strcpy(tail, "file");
    ANGBAND_DIR_FILE = string_make(path);
    strcpy(tail, "help");
    ANGBAND_DIR_HELP = string_make(path);
    strcpy(tail, "bone");
    ANGBAND_DIR_BONE = string_make(path);
    strcpy(tail, "data");
    ANGBAND_DIR_DATA = string_make(path);
    strcpy(tail, "save");
    ANGBAND_DIR_SAVE = string_make(path);
    strcpy(tail, "pref");
    ANGBAND_DIR_PREF = string_make(path);
    strcpy(tail, "info");
    ANGBAND_DIR_INFO = string_make(path);


    /* Hack -- access the "news" file */
    sprintf(path, "%s%s%s", ANGBAND_DIR_FILE, PATH_SEP, "news.txt");
    ANGBAND_NEWS = string_make(path);
}




/*
 * Hack -- take notes on line 23
 */
static void note(cptr str)
{
    Term_erase(0, 23, 79, 23);
    Term_putstr(20, 23, -1, TERM_WHITE, str);
    Term_fresh();
}



/*
 * Hack -- let the various systems "breathe"
 */
static void gasp()
{

#if defined(MACINTOSH) || defined(WINDOWS)

    /* Don't hog the processor */
    Term_xtra(TERM_XTRA_CHECK, -999);

#endif

}



/*
 * Hack -- return the "buffer length" of a string
 * This is "zero" for NULL and "strlen(str)+1" otherwise.
 */
static uint string_size(cptr str)
{
    if (!str) return (0);
    return (strlen(str) + 1);
}


/*
 * Grab one (basic) flag in a monster_race from a textual string
 *
 * Note the strange "if then else" blocks to prevent memory errors
 * during compilation.  The speed of this function is irrelevant.
 */
static errr grab_one_basic_flag(monster_race *r_ptr, cptr what)
{
    u32b flags1 = 0L;
    u32b flags2 = 0L;
    u32b flags3 = 0L;


         if (streq(what,"UNIQUE"))		flags1 |= RF1_UNIQUE;
    else if (streq(what,"QUESTOR"))		flags1 |= RF1_QUESTOR;
    
    else if (streq(what,"MALE"))		flags1 |= RF1_MALE;
    else if (streq(what,"FEMALE"))		flags1 |= RF1_FEMALE;
    
    else if (streq(what,"CHAR_CLEAR"))		flags1 |= RF1_CHAR_CLEAR;
    else if (streq(what,"CHAR_MULTI"))		flags1 |= RF1_CHAR_MULTI;
    else if (streq(what,"ATTR_CLEAR"))		flags1 |= RF1_ATTR_CLEAR;
    else if (streq(what,"ATTR_MULTI"))		flags1 |= RF1_ATTR_MULTI;
    
    else if (streq(what,"FORCE_DEPTH"))		flags1 |= RF1_FORCE_DEPTH;
    else if (streq(what,"FORCE_MAXHP"))		flags1 |= RF1_FORCE_MAXHP;
    else if (streq(what,"FORCE_SLEEP"))		flags1 |= RF1_FORCE_SLEEP;
    else if (streq(what,"FORCE_EXTRA"))		flags1 |= RF1_FORCE_EXTRA;
    
    else if (streq(what,"FRIEND"))		flags1 |= RF1_FRIEND;
    else if (streq(what,"FRIENDS"))		flags1 |= RF1_FRIENDS;
    else if (streq(what,"ESCORT"))		flags1 |= RF1_ESCORT;
    else if (streq(what,"ESCORTS"))		flags1 |= RF1_ESCORTS;
    
    else if (streq(what,"NEVER_BLOW"))		flags1 |= RF1_NEVER_BLOW;
    else if (streq(what,"NEVER_MOVE"))		flags1 |= RF1_NEVER_MOVE;
    else if (streq(what,"RAND_25"))		flags1 |= RF1_RAND_25;
    else if (streq(what,"RAND_50"))		flags1 |= RF1_RAND_50;
    
    else if (streq(what,"ONLY_GOLD"))		flags1 |= RF1_ONLY_GOLD;
    else if (streq(what,"ONLY_ITEM"))		flags1 |= RF1_ONLY_ITEM;
    else if (streq(what,"DROP_60"))		flags1 |= RF1_DROP_60;
    else if (streq(what,"DROP_90"))		flags1 |= RF1_DROP_90;
    else if (streq(what,"DROP_1D2"))		flags1 |= RF1_DROP_1D2;
    else if (streq(what,"DROP_2D2"))		flags1 |= RF1_DROP_2D2;
    else if (streq(what,"DROP_3D2"))		flags1 |= RF1_DROP_3D2;
    else if (streq(what,"DROP_4D2"))		flags1 |= RF1_DROP_4D2;
    else if (streq(what,"DROP_GOOD"))		flags1 |= RF1_DROP_GOOD;
    else if (streq(what,"DROP_GREAT"))		flags1 |= RF1_DROP_GREAT;
    else if (streq(what,"DROP_USEFUL"))		flags1 |= RF1_DROP_USEFUL;
    else if (streq(what,"DROP_CHOSEN"))		flags1 |= RF1_DROP_CHOSEN;


         if (streq(what,"STUPID"))		flags2 |= RF2_STUPID;
    else if (streq(what,"SMART"))		flags2 |= RF2_SMART;
    else if (streq(what,"XXX1X2"))		flags2 |= RF2_XXX1X2;
    else if (streq(what,"XXX2X2"))		flags2 |= RF2_XXX2X2;
    else if (streq(what,"INVISIBLE"))		flags2 |= RF2_INVISIBLE;
    else if (streq(what,"COLD_BLOOD"))		flags2 |= RF2_COLD_BLOOD;
    else if (streq(what,"EMPTY_MIND"))		flags2 |= RF2_EMPTY_MIND;
    else if (streq(what,"WEIRD_MIND"))		flags2 |= RF2_WEIRD_MIND;
    
    else if (streq(what,"MULTIPLY"))		flags2 |= RF2_MULTIPLY;
    else if (streq(what,"REGENERATE"))		flags2 |= RF2_REGENERATE;
    else if (streq(what,"XXX3X2"))		flags2 |= RF2_XXX3X2;
    else if (streq(what,"XXX4X2"))		flags2 |= RF2_XXX4X2;
    else if (streq(what,"POWERFUL"))		flags2 |= RF2_POWERFUL;
    else if (streq(what,"XXX5X2"))		flags2 |= RF2_XXX5X2;
    else if (streq(what,"DESTRUCT"))		flags2 |= RF2_DESTRUCT;
    else if (streq(what,"XXX6X2"))		flags2 |= RF2_XXX6X2;
    
    else if (streq(what,"OPEN_DOOR"))		flags2 |= RF2_OPEN_DOOR;
    else if (streq(what,"BASH_DOOR"))		flags2 |= RF2_BASH_DOOR;
    else if (streq(what,"PASS_WALL"))		flags2 |= RF2_PASS_WALL;
    else if (streq(what,"KILL_WALL"))		flags2 |= RF2_KILL_WALL;
    else if (streq(what,"MOVE_BODY"))		flags2 |= RF2_MOVE_BODY;
    else if (streq(what,"KILL_BODY"))		flags2 |= RF2_KILL_BODY;
    else if (streq(what,"TAKE_ITEM"))		flags2 |= RF2_TAKE_ITEM;
    else if (streq(what,"KILL_ITEM"))		flags2 |= RF2_KILL_ITEM;
    
    else if (streq(what,"BRAIN_1"))		flags2 |= RF2_BRAIN_1;
    else if (streq(what,"BRAIN_2"))		flags2 |= RF2_BRAIN_2;
    else if (streq(what,"BRAIN_3"))		flags2 |= RF2_BRAIN_3;
    else if (streq(what,"BRAIN_4"))		flags2 |= RF2_BRAIN_4;
    else if (streq(what,"BRAIN_5"))		flags2 |= RF2_BRAIN_5;
    else if (streq(what,"BRAIN_6"))		flags2 |= RF2_BRAIN_6;
    else if (streq(what,"BRAIN_7"))		flags2 |= RF2_BRAIN_7;
    else if (streq(what,"BRAIN_8"))		flags2 |= RF2_BRAIN_8;

         if (streq(what,"ORC"))			flags3 |= RF3_ORC;
    else if (streq(what,"TROLL"))		flags3 |= RF3_TROLL;
    else if (streq(what,"GIANT"))		flags3 |= RF3_GIANT;
    else if (streq(what,"DRAGON"))		flags3 |= RF3_DRAGON;
    else if (streq(what,"DEMON"))		flags3 |= RF3_DEMON;
    else if (streq(what,"UNDEAD"))		flags3 |= RF3_UNDEAD;
    else if (streq(what,"EVIL"))		flags3 |= RF3_EVIL;
    else if (streq(what,"ANIMAL"))		flags3 |= RF3_ANIMAL;
    
    else if (streq(what,"XXX1X3"))		flags3 |= RF3_XXX1X3;
    else if (streq(what,"XXX2X3"))		flags3 |= RF3_XXX2X3;
    else if (streq(what,"XXX3X3"))		flags3 |= RF3_XXX3X3;
    else if (streq(what,"XXX4X3"))		flags3 |= RF3_XXX4X3;
    
    else if (streq(what,"HURT_LITE"))		flags3 |= RF3_HURT_LITE;
    else if (streq(what,"HURT_ROCK"))		flags3 |= RF3_HURT_ROCK;
    else if (streq(what,"HURT_FIRE"))		flags3 |= RF3_HURT_FIRE;
    else if (streq(what,"HURT_COLD"))		flags3 |= RF3_HURT_COLD;
    
    else if (streq(what,"IM_ACID"))		flags3 |= RF3_IM_ACID;
    else if (streq(what,"IM_ELEC"))		flags3 |= RF3_IM_ELEC;
    else if (streq(what,"IM_FIRE"))		flags3 |= RF3_IM_FIRE;
    else if (streq(what,"IM_COLD"))		flags3 |= RF3_IM_COLD;
    else if (streq(what,"IM_POIS"))		flags3 |= RF3_IM_POIS;
    else if (streq(what,"XXX5X3"))		flags3 |= RF3_XXX5X3;
    else if (streq(what,"RES_NETH"))		flags3 |= RF3_RES_NETH;
    else if (streq(what,"RES_WATE"))		flags3 |= RF3_RES_WATE;
    else if (streq(what,"RES_PLAS"))		flags3 |= RF3_RES_PLAS;
    else if (streq(what,"RES_NEXU"))		flags3 |= RF3_RES_NEXU;
    else if (streq(what,"RES_DISE"))		flags3 |= RF3_RES_DISE;
    else if (streq(what,"XXX6X3"))		flags3 |= RF3_XXX6X3;
    else if (streq(what,"NO_FEAR"))		flags3 |= RF3_NO_FEAR;
    else if (streq(what,"NO_STUN"))		flags3 |= RF3_NO_STUN;
    else if (streq(what,"NO_CONF"))		flags3 |= RF3_NO_CONF;
    else if (streq(what,"NO_SLEEP"))		flags3 |= RF3_NO_SLEEP;


    /* Failure */
    if (!flags1 && !flags2 && !flags3) return (1);

    /* Apply flags */
    if (flags1) r_ptr->rflags1 |= flags1;
    if (flags2) r_ptr->rflags2 |= flags2;
    if (flags3) r_ptr->rflags3 |= flags3;

    /* Success */
    return (0);
}


/*
 * Grab one (spell) flag in a monster_race from a textual string
 *
 * Note the strange "if then else" blocks to prevent memory errors
 * during compilation.  The speed of this function is irrelevant.
 */
static errr grab_one_spell_flag(monster_race *r_ptr, cptr what)
{
    int i;

    u32b flags4 = 0L;
    u32b flags5 = 0L;
    u32b flags6 = 0L;


    /* Start checking */
    if (!what) what = what;

    /* XXX XXX XXX Hack -- Read spell frequency */
    else if (1 == sscanf(what, "1_IN_%d", &i)) {

        /* Extract a "frequency" */
        r_ptr->freq_spell = r_ptr->freq_inate = 100 / i;

        /* Success */
        return (0);
    }

         if (streq(what,"SHRIEK"))		flags4 |= RF4_SHRIEK;
    else if (streq(what,"XXX2X4"))		flags4 |= RF4_XXX2X4;
    else if (streq(what,"XXX3X4"))		flags4 |= RF4_XXX3X4;
    else if (streq(what,"XXX4X4"))		flags4 |= RF4_XXX4X4;
    else if (streq(what,"ARROW_1"))		flags4 |= RF4_ARROW_1;
    else if (streq(what,"ARROW_2"))		flags4 |= RF4_ARROW_2;
    else if (streq(what,"ARROW_3"))		flags4 |= RF4_ARROW_3;
    else if (streq(what,"ARROW_4"))		flags4 |= RF4_ARROW_4;
    else if (streq(what,"BR_ACID"))		flags4 |= RF4_BR_ACID;
    else if (streq(what,"BR_ELEC"))		flags4 |= RF4_BR_ELEC;
    else if (streq(what,"BR_FIRE"))		flags4 |= RF4_BR_FIRE;
    else if (streq(what,"BR_COLD"))		flags4 |= RF4_BR_COLD;
    else if (streq(what,"BR_POIS"))		flags4 |= RF4_BR_POIS;
    else if (streq(what,"BR_NETH"))		flags4 |= RF4_BR_NETH;
    else if (streq(what,"BR_LITE"))		flags4 |= RF4_BR_LITE;
    else if (streq(what,"BR_DARK"))		flags4 |= RF4_BR_DARK;
    else if (streq(what,"BR_CONF"))		flags4 |= RF4_BR_CONF;
    else if (streq(what,"BR_SOUN"))		flags4 |= RF4_BR_SOUN;
    else if (streq(what,"BR_CHAO"))		flags4 |= RF4_BR_CHAO;
    else if (streq(what,"BR_DISE"))		flags4 |= RF4_BR_DISE;
    else if (streq(what,"BR_NEXU"))		flags4 |= RF4_BR_NEXU;
    else if (streq(what,"BR_TIME"))		flags4 |= RF4_BR_TIME;
    else if (streq(what,"BR_INER"))		flags4 |= RF4_BR_INER;
    else if (streq(what,"BR_GRAV"))		flags4 |= RF4_BR_GRAV;
    else if (streq(what,"BR_SHAR"))		flags4 |= RF4_BR_SHAR;
    else if (streq(what,"BR_PLAS"))		flags4 |= RF4_BR_PLAS;
    else if (streq(what,"BR_WALL"))		flags4 |= RF4_BR_WALL;
    else if (streq(what,"BR_MANA"))		flags4 |= RF4_BR_MANA;
    else if (streq(what,"XXX5X4"))		flags4 |= RF4_XXX5X4;
    else if (streq(what,"XXX6X4"))		flags4 |= RF4_XXX6X4;
    else if (streq(what,"XXX7X4"))		flags4 |= RF4_XXX7X4;
    else if (streq(what,"XXX8X4"))		flags4 |= RF4_XXX8X4;
 
         if (streq(what,"BA_ACID"))		flags5 |= RF5_BA_ACID;
    else if (streq(what,"BA_ELEC"))		flags5 |= RF5_BA_ELEC;
    else if (streq(what,"BA_FIRE"))		flags5 |= RF5_BA_FIRE;
    else if (streq(what,"BA_COLD"))		flags5 |= RF5_BA_COLD;
    else if (streq(what,"BA_POIS"))		flags5 |= RF5_BA_POIS;
    else if (streq(what,"BA_NETH"))		flags5 |= RF5_BA_NETH;
    else if (streq(what,"BA_WATE"))		flags5 |= RF5_BA_WATE;
    else if (streq(what,"BA_MANA"))		flags5 |= RF5_BA_MANA;
    else if (streq(what,"BA_DARK"))		flags5 |= RF5_BA_DARK;
    else if (streq(what,"DRAIN_MANA"))		flags5 |= RF5_DRAIN_MANA;
    else if (streq(what,"MIND_BLAST"))		flags5 |= RF5_MIND_BLAST;
    else if (streq(what,"BRAIN_SMASH"))		flags5 |= RF5_BRAIN_SMASH;
    else if (streq(what,"CAUSE_1"))		flags5 |= RF5_CAUSE_1;
    else if (streq(what,"CAUSE_2"))		flags5 |= RF5_CAUSE_2;
    else if (streq(what,"CAUSE_3"))		flags5 |= RF5_CAUSE_3;
    else if (streq(what,"CAUSE_4"))		flags5 |= RF5_CAUSE_4;
    else if (streq(what,"BO_ACID"))		flags5 |= RF5_BO_ACID;
    else if (streq(what,"BO_ELEC"))		flags5 |= RF5_BO_ELEC;
    else if (streq(what,"BO_FIRE"))		flags5 |= RF5_BO_FIRE;
    else if (streq(what,"BO_COLD"))		flags5 |= RF5_BO_COLD;
    else if (streq(what,"BO_POIS"))		flags5 |= RF5_BO_POIS;
    else if (streq(what,"BO_NETH"))		flags5 |= RF5_BO_NETH;
    else if (streq(what,"BO_WATE"))		flags5 |= RF5_BO_WATE;
    else if (streq(what,"BO_MANA"))		flags5 |= RF5_BO_MANA;
    else if (streq(what,"BO_PLAS"))		flags5 |= RF5_BO_PLAS;
    else if (streq(what,"BO_ICEE"))		flags5 |= RF5_BO_ICEE;
    else if (streq(what,"MISSILE"))		flags5 |= RF5_MISSILE;
    else if (streq(what,"SCARE"))		flags5 |= RF5_SCARE;
    else if (streq(what,"BLIND"))		flags5 |= RF5_BLIND;
    else if (streq(what,"CONF"))		flags5 |= RF5_CONF;
    else if (streq(what,"SLOW"))		flags5 |= RF5_SLOW;
    else if (streq(what,"HOLD"))		flags5 |= RF5_HOLD;

         if (streq(what,"HASTE"))		flags6 |= RF6_HASTE;
    else if (streq(what,"XXX1X6"))		flags6 |= RF6_XXX1X6;
    else if (streq(what,"HEAL"))		flags6 |= RF6_HEAL;
    else if (streq(what,"XXX2X6"))		flags6 |= RF6_XXX2X6;
    else if (streq(what,"BLINK"))		flags6 |= RF6_BLINK;
    else if (streq(what,"TPORT"))		flags6 |= RF6_TPORT;
    else if (streq(what,"XXX3X6"))		flags6 |= RF6_XXX3X6;
    else if (streq(what,"XXX4X6"))		flags6 |= RF6_XXX4X6;
    else if (streq(what,"TELE_TO"))		flags6 |= RF6_TELE_TO;
    else if (streq(what,"TELE_AWAY"))		flags6 |= RF6_TELE_AWAY;
    else if (streq(what,"TELE_LEVEL"))		flags6 |= RF6_TELE_LEVEL;
    else if (streq(what,"XXX5"))		flags6 |= RF6_XXX5;
    else if (streq(what,"DARKNESS"))		flags6 |= RF6_DARKNESS;
    else if (streq(what,"TRAPS"))		flags6 |= RF6_TRAPS;
    else if (streq(what,"FORGET"))		flags6 |= RF6_FORGET;
    else if (streq(what,"XXX6X6"))		flags6 |= RF6_XXX6X6;
    else if (streq(what,"XXX7X6"))		flags6 |= RF6_XXX7X6;
    else if (streq(what,"XXX8X6"))		flags6 |= RF6_XXX8X6;
    else if (streq(what,"S_MONSTER"))		flags6 |= RF6_S_MONSTER;
    else if (streq(what,"S_MONSTERS"))		flags6 |= RF6_S_MONSTERS;
    else if (streq(what,"S_ANT"))		flags6 |= RF6_S_ANT;
    else if (streq(what,"S_SPIDER"))		flags6 |= RF6_S_SPIDER;
    else if (streq(what,"S_HOUND"))		flags6 |= RF6_S_HOUND;
    else if (streq(what,"S_REPTILE"))		flags6 |= RF6_S_REPTILE;
    else if (streq(what,"S_ANGEL"))		flags6 |= RF6_S_ANGEL;
    else if (streq(what,"S_DEMON"))		flags6 |= RF6_S_DEMON;
    else if (streq(what,"S_UNDEAD"))		flags6 |= RF6_S_UNDEAD;
    else if (streq(what,"S_DRAGON"))		flags6 |= RF6_S_DRAGON;
    else if (streq(what,"S_HI_UNDEAD"))		flags6 |= RF6_S_HI_UNDEAD;
    else if (streq(what,"S_HI_DRAGON"))		flags6 |= RF6_S_HI_DRAGON;
    else if (streq(what,"S_WRAITH"))		flags6 |= RF6_S_WRAITH;
    else if (streq(what,"S_UNIQUE"))		flags6 |= RF6_S_UNIQUE;


    /* Failure */
    if (!flags4 && !flags5 && !flags6) return (1);

    /* Apply flags */
    if (flags4) r_ptr->rflags4 |= flags4;
    if (flags5) r_ptr->rflags5 |= flags5;
    if (flags6) r_ptr->rflags6 |= flags6;

    /* Success */
    return (0);
}



/*
 * Convert a "color letter" into an actual color
 * The colors are: dwsorgbuDWvyRGBU, see below
 * No longer includes MULTI or CLEAR
 */
static int color_char_to_attr(char c)
{
    switch (c) {

        case 'd': return (TERM_BLACK);
        case 'w': return (TERM_WHITE);
        case 's': return (TERM_GRAY);
        case 'o': return (TERM_ORANGE);
        case 'r': return (TERM_RED);
        case 'g': return (TERM_GREEN);
        case 'b': return (TERM_BLUE);
        case 'u': return (TERM_UMBER);

        case 'D': return (TERM_D_GRAY);
        case 'W': return (TERM_L_GRAY);
        case 'v': return (TERM_VIOLET);
        case 'y': return (TERM_YELLOW);
        case 'R': return (TERM_L_RED);
        case 'G': return (TERM_L_GREEN);
        case 'B': return (TERM_L_BLUE);
        case 'U': return (TERM_L_UMBER);
    }

    return (-1);
}



/*
 * Extract a method
 */
static errr analyze_blow(monster_race *r_ptr, int i, cptr s1, cptr s2, cptr s3)
{
    int method = 0, effect = 0, d_dice = 0, d_side = 0;

    /* Analyze method */
    if (streq(s1, "")) method = 0;
    else if (streq(s1, "HIT")) method = RBM_HIT;
    else if (streq(s1, "TOUCH")) method = RBM_TOUCH;
    else if (streq(s1, "PUNCH")) method = RBM_PUNCH;
    else if (streq(s1, "KICK")) method = RBM_KICK;
    else if (streq(s1, "CLAW")) method = RBM_CLAW;
    else if (streq(s1, "BITE")) method = RBM_BITE;
    else if (streq(s1, "STING")) method = RBM_STING;
    else if (streq(s1, "XXX1")) method = RBM_XXX1;
    else if (streq(s1, "BUTT")) method = RBM_BUTT;
    else if (streq(s1, "CRUSH")) method = RBM_CRUSH;
    else if (streq(s1, "ENGULF")) method = RBM_ENGULF;
    else if (streq(s1, "XXX2")) method = RBM_XXX2;
    else if (streq(s1, "CRAWL")) method = RBM_CRAWL;
    else if (streq(s1, "DROOL")) method = RBM_DROOL;
    else if (streq(s1, "SPIT")) method = RBM_SPIT;
    else if (streq(s1, "XXX3")) method = RBM_XXX3;
    else if (streq(s1, "GAZE")) method = RBM_GAZE;
    else if (streq(s1, "WAIL")) method = RBM_WAIL;
    else if (streq(s1, "SPORE")) method = RBM_SPORE;
    else if (streq(s1, "XXX4")) method = RBM_XXX4;
    else if (streq(s1, "BEG")) method = RBM_BEG;
    else if (streq(s1, "INSULT")) method = RBM_INSULT;
    else if (streq(s1, "MOAN")) method = RBM_MOAN;
    else if (streq(s1, "XXX5")) method = RBM_XXX5;
    else return (-11);
    
    /* Analyze effect */
    if (streq(s2, "")) effect = 0;
    else if (streq(s2, "HURT")) effect = RBE_HURT;
    else if (streq(s2, "POISON")) effect = RBE_POISON;
    else if (streq(s2, "UN_BONUS")) effect = RBE_UN_BONUS;
    else if (streq(s2, "UN_POWER")) effect = RBE_UN_POWER;
    else if (streq(s2, "EAT_GOLD")) effect = RBE_EAT_GOLD;
    else if (streq(s2, "EAT_ITEM")) effect = RBE_EAT_ITEM;
    else if (streq(s2, "EAT_FOOD")) effect = RBE_EAT_FOOD;
    else if (streq(s2, "EAT_LITE")) effect = RBE_EAT_LITE;
    else if (streq(s2, "ACID")) effect = RBE_ACID;
    else if (streq(s2, "ELEC")) effect = RBE_ELEC;
    else if (streq(s2, "FIRE")) effect = RBE_FIRE;
    else if (streq(s2, "COLD")) effect = RBE_COLD;
    else if (streq(s2, "BLIND")) effect = RBE_BLIND;
    else if (streq(s2, "CONFUSE")) effect = RBE_CONFUSE;
    else if (streq(s2, "TERRIFY")) effect = RBE_TERRIFY;
    else if (streq(s2, "PARALYZE")) effect = RBE_PARALYZE;
    else if (streq(s2, "LOSE_STR")) effect = RBE_LOSE_STR;
    else if (streq(s2, "LOSE_INT")) effect = RBE_LOSE_INT;
    else if (streq(s2, "LOSE_WIS")) effect = RBE_LOSE_WIS;
    else if (streq(s2, "LOSE_DEX")) effect = RBE_LOSE_DEX;
    else if (streq(s2, "LOSE_CON")) effect = RBE_LOSE_CON;
    else if (streq(s2, "LOSE_CHR")) effect = RBE_LOSE_CHR;
    else if (streq(s2, "LOSE_ALL")) effect = RBE_LOSE_ALL;
    else if (streq(s2, "XXX1")) effect = RBE_XXX1;
    else if (streq(s2, "EXP_10")) effect = RBE_EXP_10;
    else if (streq(s2, "EXP_20")) effect = RBE_EXP_20;
    else if (streq(s2, "EXP_40")) effect = RBE_EXP_40;
    else if (streq(s2, "EXP_80")) effect = RBE_EXP_80;
    else return (-12);

    /* Analyze damage */
    if (*s3 && (sscanf(s3, "%dd%d", &d_dice, &d_side) != 2)) return (-13);

    /* Save the info */
    r_ptr->blow[i].method = method;
    r_ptr->blow[i].effect = effect;
    r_ptr->blow[i].d_dice = d_dice;
    r_ptr->blow[i].d_side = d_side;

    /* Success */
    return (0);
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
    char *s, *t;

    /* Not ready yet */
    bool okay = FALSE;

    /* No monster yet */
    int m = -1;

    /* No r_ptr yet */
    monster_race *r_ptr = NULL;

    /* The "monsters" file */
    FILE *fp;

    /* General buffer */
    char buf[1024];

    /* Current race description */
    char desc[24*80];


    /* Access the "r_list.txt" file */
    sprintf(buf, "%s%s%s", ANGBAND_DIR_FILE, PATH_SEP, "r_list.txt");

    /* Open the monster file */
    fp = fopen(buf, "r");

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

            /* Don't hog the processor */
            gasp();

            /* Next... */
            continue;
        }

        /* The line better have a colon and such */
        if (buf[1] != ':') return (1);

        /* Process 'V' for "Version" */
        if (buf[0] == 'V') {

            int v1, v2, v3;

            /* Scan for the values */
            if ((3 != sscanf(buf, "V:%d.%d.%d", &v1, &v2, &v3)) ||
                (v1 != CUR_VERSION_MAJ) ||
                (v2 != CUR_VERSION_MIN) ||
                (v3 != CUR_PATCH_LEVEL)) {

                msg_print("Warning: your 'm_list.txt' is obsolete!");
                msg_print(NULL);

                return (99);
            }

            /* Okay to proceed */
            okay = TRUE;

            /* Continue */
            continue;
        }

        /* Process 'N' for "New/Number/Name" */
        if (buf[0] == 'N') {

            /* No version yet */
            if (!okay) return (99);

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

            /* Verify (no ghost definition!) */
            if ((m < 0) || (m >= MAX_R_IDX-1)) return (7);

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
            int tmp, spd, hp1, hp2, aaf, ac, slp;

            /* Scan for the values */
            if (8 != sscanf(buf, "I:%c:%c:%d:%dd%d:%d:%d:%d",
                &chr, &att, &spd, &hp1, &hp2, &aaf, &ac, &slp)) return (11);

            /* Extract the color */
            tmp = color_char_to_attr(att);
            if (tmp < 0) return (12);

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

            int lev, rar, xxx;
            long exp;

            /* Scan for the values */
            if (4 != sscanf(buf, "W:%d:%d:%d:%ld",
                &lev, &rar, &xxx, &exp)) return (11);

            /* Save the values */
            r_ptr->level = lev;
            r_ptr->rarity = rar;
            r_ptr->extra = xxx;
            r_ptr->mexp = exp;

            /* Next... */
            continue;
        }

        /* Process 'B' for "Blows" (up to four lines) */
        if (buf[0] == 'B') {

            int i;

            char *s1, *s2;
            
            /* Find the next empty blow slot */
            for (i = 0; (i < 3) && (r_ptr->blow[i].method != 0); i++);

            /* Analyze the first field */
            for (s1 = s = buf+2, t = s; *t && (*t != ':'); t++);

            /* Terminate the field (if necessary) */
            if (*t == ':') *t++ = '\0';

            /* Analyze the second field */
            for (s2 = s = t; *t && (*t != ':'); t++);

            /* Terminate the field (if necessary) */
            if (*t == ':') *t++ = '\0';

            /* Analyze the blow */
            if (0 != analyze_blow(r_ptr, i, s1, s2, t)) return (16);

            /* Next... */
            continue;
        }

        /* Process 'F' for "Basic Flags" (multiple lines) */
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
                if (0 != grab_one_basic_flag(r_ptr, s)) return (18);

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
                for (t = s; *t && *t != ' ' && *t != '|'; ++t);

                /* Nuke and skip any dividers */
                if (*t) {
                    *t++ = '\0';
                    while (*t == ' ' || *t == '|') t++;
                }

                /* Parse this entry */
                if (0 != grab_one_spell_flag(r_ptr, s)) return (18);

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




#ifdef BINARY_ARRAY_IMAGES

/*
 * Attempt to "dump" a "quick-load binary image" for "r_list"
 *
 * But we cannot actually "read()" the whole chunk at once because
 * the piece of shit Macintosh "read()" function expects to read()
 * only 64K at a time.  Brain damaged personal computers piss me off.
 * We might even be able to squeeze by if we forbid expansion of the
 * records, but that is too heavy a price to pay...
 *
 * Thus: three separate files:
 *   r_list.raw = (Intro) + (N records)
 *   r_name.raw = (N name sizes) + (N names)
 *   r_desc.raw = (N desc sizes) + (N descs)
 *
 * The "r_name" and "r_desc" files start with an array of "lengths"
 * (including terminators), where "zero" means "no string" (NULL).
 * Then the files contain each non-NULL string in order.
 */
static errr dump_r_list_raw()
{
    int i, fd;

    int mode = 0644;

    char tmp[1024];


    /* Hack -- sizes */
    uint len[MAX_R_IDX-1];

    /* Size of the "lengths" array */
    uint slen = sizeof(len);

    /* Size of one normal record */
    uint size = sizeof(monster_race);


#ifdef MACINTOSH
    _ftype = 'DATA';
#endif


    /* File "r_list.raw" -- raw records */
    sprintf(tmp, "%s%s%s", ANGBAND_DIR_DATA, PATH_SEP, "r_list.raw");
    fd = my_topen(tmp, O_RDWR | O_CREAT | O_BINARY, mode);
    if (fd < 0) return (-1);

    /* Dump the version info */
    tmp[0] = CUR_VERSION_MAJ;
    tmp[1] = CUR_VERSION_MIN;
    tmp[2] = CUR_PATCH_LEVEL;
    tmp[3] = size;

    /* Dump it */
    write(fd, tmp, 4);

    /* Attempt to dump the (normal) monster races */
    for (i = 0; i < MAX_R_IDX-1; i++) {
        if (size != write(fd, (char*)(&r_list[i]), size)) break;
    }

    /* Close and verify */
    gasp();
    close(fd);
    if (i < MAX_R_IDX-1) return (1);


    /* File "r_name.raw" -- race names */
    sprintf(tmp, "%s%s%s", ANGBAND_DIR_DATA, PATH_SEP, "r_name.raw");
    fd = my_topen(tmp, O_RDWR | O_CREAT | O_BINARY, mode);
    if (fd < 0) return (-1);

    /* Prepare the "lengths" array */
    for (i = 0; i < MAX_R_IDX-1; i++) {
        monster_race *r_ptr = &r_list[i];
        len[i] = string_size(r_ptr->name);
    }

    /* Dump the array */
    write(fd, (char*)(len), slen);

    /* Dump the names */
    for (i = 0; i < MAX_R_IDX-1; i++) {
        monster_race *r_ptr = &r_list[i];
        char *str = (char*)(r_ptr->name);
        if (len[i] && (len[i] != write(fd, str, len[i]))) break;
    }

    /* Close and verify */
    gasp();
    close(fd);
    if (i < MAX_R_IDX-1) return (1);


    /* File "r_desc.raw" -- race names */
    sprintf(tmp, "%s%s%s", ANGBAND_DIR_DATA, PATH_SEP, "r_desc.raw");
    fd = my_topen(tmp, O_RDWR | O_CREAT | O_BINARY, mode);
    if (fd < 0) return (-1);

    /* Prepare the "lengths" array */
    for (i = 0; i < MAX_R_IDX-1; i++) {
        monster_race *r_ptr = &r_list[i];
        len[i] = string_size(r_ptr->desc);
    }

    /* Dump the array */
    write(fd, (char*)(len), slen);

    /* Dump the names */
    for (i = 0; i < MAX_R_IDX-1; i++) {
        monster_race *r_ptr = &r_list[i];
        char *str = (char*)(r_ptr->desc);
        if (len[i] && (len[i] != write(fd, str, len[i]))) break;
    }

    /* Close and verify */
    gasp();
    close(fd);
    if (i < MAX_R_IDX-1) return (1);


    /* Success */
    return (0);
}



/*
 * Attempt to "quick-load" a binary image for "r_list"
 */
static errr init_r_list_raw()
{
    int i, fd;

    int mode = 0644;

    char tmp[1024];


    /* Hack -- sizes */
    uint len[MAX_R_IDX-1];

    /* Size of the "lengths" array */
    uint slen = sizeof(len);

    /* Size of one normal record */
    uint size = sizeof(monster_race);


#ifdef MACINTOSH
    _ftype = 'DATA';
#endif


    /* File "r_list.raw" -- raw records */
    sprintf(tmp, "%s%s%s", ANGBAND_DIR_DATA, PATH_SEP, "r_list.raw");
    fd = my_topen(tmp, O_RDONLY | O_BINARY, mode);
    if (fd < 0) return (-1);

    /* Read and Verify the version info */
    if ((4 != read(fd, tmp, 4)) ||
        (tmp[0] != CUR_VERSION_MAJ) ||
        (tmp[1] != CUR_VERSION_MIN) ||
        (tmp[2] != CUR_PATCH_LEVEL) ||
        (tmp[3] != size)) {

        /* Hack -- message */
        note("Ignoring old file 'lib/file/m_list.raw'...");

        /* Close */
        close(fd);

        /* Fail */
        return (-1);
    }

    /* Attempt to read the (normal) monster races */
    for (i = 0; i < MAX_R_IDX-1; i++) {
        monster_race *r_ptr = &r_list[i];
        if (size != read(fd, (char*)(r_ptr), size)) break;
        r_ptr->name = NULL;
        r_ptr->desc = NULL;
    }

    /* Close and verify */
    gasp();
    close(fd);
    if (i < MAX_R_IDX-1) return (1);


    /* File "r_name.raw" -- race names */
    sprintf(tmp, "%s%s%s", ANGBAND_DIR_DATA, PATH_SEP, "r_name.raw");
    fd = my_topen(tmp, O_RDONLY | O_BINARY, mode);
    if (fd < 0) return (-1);

    /* Read the size array */
    if (slen != read(fd, (char*)(len), slen)) return (1);

    /* Attempt to read the (normal) monster race names */
    for (i = 0; i < MAX_R_IDX-1; i++) {
        monster_race *r_ptr = &r_list[i];
        if (len[i] != read(fd, tmp, len[i])) break;
        if (len[i]) r_ptr->name = string_make(tmp);
    }

    /* Close and verify */
    gasp();
    close(fd);
    if (i < MAX_R_IDX-1) return (1);


    /* File "r_desc.raw" -- descriptions */
    sprintf(tmp, "%s%s%s", ANGBAND_DIR_DATA, PATH_SEP, "r_desc.raw");
    fd = my_topen(tmp, O_RDONLY | O_BINARY, mode);
    if (fd < 0) return (-1);

    /* Read the size array */
    if (slen != read(fd, (char*)(len), slen)) return (1);

    /* Attempt to read the (normal) monster race descs */
    for (i = 0; i < MAX_R_IDX-1; i++) {
        monster_race *r_ptr = &r_list[i];
        if (len[i] != read(fd, tmp, len[i])) break;
        if (len[i]) r_ptr->desc = string_make(tmp);
    }

    /* Close and verify */
    gasp();
    close(fd);
    if (i < MAX_R_IDX-1) return (1);


    /* Success */
    return (0);
}

#endif


/*
 * Initialize the "r_list" array by parsing various files.
 */
static void init_r_list()
{
    errr err;


    /* XXX Hack -- prepare "ghost" race */
    monster_race *r_ptr = &r_list[MAX_R_IDX-1];


    /* Hack -- Give the ghost monster a "fake" name */
    r_ptr->name = ghost_name;

    /* Hack -- Give the ghost monster a "default" name */
    r_ptr->desc = "It seems familiar...";
    
    /* Hack -- set the char/attr info */
    r_ptr->r_attr = TERM_WHITE;
    r_ptr->r_char = 'G';

    /* Hack -- Prepare a fake ghost name */
    strcpy(ghost_name, "Nobody, the Ghost");

    /* Hack -- Try to prevent a few "potential" bugs */
    r_ptr->rflags1 |= RF1_UNIQUE;

    /* XXX XXX XXX We should really "recreate" the ghost */
    

#ifdef BINARY_ARRAY_IMAGES

    /* Try to load a "binary image" */
    err = init_r_list_raw();

    /* Otherwise try the text version */
    if (!err) return;

#endif

    /* Try the text version */
    err = init_r_list_txt();

    /* Still no luck? Fail! */
    if (err) {

        /* Warning */
        msg_format("Fatal error #%d parsing 'r_list.txt', record %d!",
                   err, error_r_idx);
        msg_print(NULL);

        /* Quit */
        quit("cannot load 'r_list.txt'");
    }

#ifdef BINARY_ARRAY_IMAGES

    /* Attempt to dump a "quick-load" version */
    err = dump_r_list_raw();

    /* Warn on errors */
    if (err) {
        msg_print("Warning: unable to create binary monster race images!");
        msg_print(NULL);
    }

#endif

}






/*
 * Grab one flag in a inven_kind from a textual string
 *
 * Note the strange "if then else" blocks to prevent memory errors
 * during compilation.  The speed of this function is irrelevant.
 */
static bool grab_one_kind_flag(inven_kind *k_ptr, cptr what)
{
    u32b flags1 = 0L, flags2 = 0L, flags3 = 0L;


         if (streq(what, "STR"))		flags1 |= TR1_STR;
    else if (streq(what, "INT"))		flags1 |= TR1_INT;
    else if (streq(what, "WIS"))		flags1 |= TR1_WIS;
    else if (streq(what, "DEX"))		flags1 |= TR1_DEX;
    else if (streq(what, "CON"))		flags1 |= TR1_CON;
    else if (streq(what, "CHR"))		flags1 |= TR1_CHR;

    else if (streq(what, "SEARCH"))		flags1 |= TR1_SEARCH;
    else if (streq(what, "STEALTH"))		flags1 |= TR1_STEALTH;
    else if (streq(what, "TUNNEL"))		flags1 |= TR1_TUNNEL;
    else if (streq(what, "INFRA"))		flags1 |= TR1_INFRA;
    else if (streq(what, "SPEED"))		flags1 |= TR1_SPEED;
    else if (streq(what, "BLOWS"))		flags1 |= TR1_BLOWS;

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


         if (streq(what, "FREE_ACT"))		flags2 |= TR2_FREE_ACT;
    else if (streq(what, "HOLD_LIFE"))		flags2 |= TR2_HOLD_LIFE;

    else if (streq(what, "IM_FIRE"))		flags2 |= TR2_IM_FIRE;
    else if (streq(what, "IM_COLD"))		flags2 |= TR2_IM_COLD;
    else if (streq(what, "IM_ACID"))		flags2 |= TR2_IM_ACID;
    else if (streq(what, "IM_ELEC"))		flags2 |= TR2_IM_ELEC;
    else if (streq(what, "IM_POIS"))		flags2 |= TR2_IM_POIS;

    else if (streq(what, "RES_ACID"))		flags2 |= TR2_RES_ACID;
    else if (streq(what, "RES_ELEC"))		flags2 |= TR2_RES_ELEC;
    else if (streq(what, "RES_FIRE"))		flags2 |= TR2_RES_FIRE;
    else if (streq(what, "RES_COLD"))		flags2 |= TR2_RES_COLD;
    else if (streq(what, "RES_POIS"))		flags2 |= TR2_RES_POIS;
    else if (streq(what, "RES_LITE"))		flags2 |= TR2_RES_LITE;
    else if (streq(what, "RES_DARK"))		flags2 |= TR2_RES_DARK;

    else if (streq(what, "RES_BLIND"))		flags2 |= TR2_RES_BLIND;
    else if (streq(what, "RES_CONF"))		flags2 |= TR2_RES_CONF;
    else if (streq(what, "RES_SOUND"))		flags2 |= TR2_RES_SOUND;
    else if (streq(what, "RES_SHARDS"))		flags2 |= TR2_RES_SHARDS;

    else if (streq(what, "RES_NETHER"))		flags2 |= TR2_RES_NETHER;
    else if (streq(what, "RES_NEXUS"))		flags2 |= TR2_RES_NEXUS;
    else if (streq(what, "RES_CHAOS"))		flags2 |= TR2_RES_CHAOS;
    else if (streq(what, "RES_DISEN"))		flags2 |= TR2_RES_DISEN;

    else if (streq(what, "SUST_STR"))		flags2 |= TR2_SUST_STR;
    else if (streq(what, "SUST_INT"))		flags2 |= TR2_SUST_INT;
    else if (streq(what, "SUST_WIS"))		flags2 |= TR2_SUST_WIS;
    else if (streq(what, "SUST_DEX"))		flags2 |= TR2_SUST_DEX;
    else if (streq(what, "SUST_CON"))		flags2 |= TR2_SUST_CON;
    else if (streq(what, "SUST_CHR"))		flags2 |= TR2_SUST_CHR;


         if (streq(what, "EASY_KNOW"))		flags3 |= TR3_EASY_KNOW;
    else if (streq(what, "HIDE_TYPE"))		flags3 |= TR3_HIDE_TYPE;
    else if (streq(what, "SHOW_MODS"))		flags3 |= TR3_SHOW_MODS;
    else if (streq(what, "INSTA_ART"))		flags3 |= TR3_INSTA_ART;

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
    char *s, *t;

    /* Not ready yet */
    bool okay = FALSE;

    /* No item kind yet */
    int m = -1;

    /* No k_ptr yet */
    inven_kind *k_ptr = NULL;

    /* The "objects" file */
    FILE *fp;

    /* General buffer */
    char buf[1024];


    /* Access the "k_list.txt" file */
    sprintf(buf, "%s%s%s", ANGBAND_DIR_FILE, PATH_SEP, "k_list.txt");

    /* Open the file */
    fp = fopen(buf, "r");

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

            /* Don't hog the processor */
            gasp();

            /* Next... */
            continue;
        }

        /* The line better have a colon and such */
        if (buf[1] != ':') return (1);

        /* Process 'V' for "Version" */
        if (buf[0] == 'V') {

            int v1, v2, v3;

            /* Scan for the values */
            if ((3 != sscanf(buf, "V:%d.%d.%d", &v1, &v2, &v3)) ||
                (v1 != CUR_VERSION_MAJ) ||
                (v2 != CUR_VERSION_MIN) ||
                (v3 != CUR_PATCH_LEVEL)) {

                msg_print("Warning: your 'k_list.txt' is obsolete!");
                msg_print(NULL);

                return (99);
            }

            /* Okay to proceed */
            okay = TRUE;

            /* Continue */
            continue;
        }

        /* Process 'N' for "New/Number/Name" */
        if (buf[0] == 'N') {

            /* No version */
            if (!okay) return (99);

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
            int tmp, tval, sval, pval;

            /* Scan for the values */
            if (5 != sscanf(buf+2, "%c:%c:%d:%d:%d",
                &sym, &col, &tval, &sval, &pval)) return (11);

            /* Extract the color */
            tmp = color_char_to_attr(col);
            if (tmp < 0) return (12);

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
                &level, &extra, &wgt, &cost)) return (11);

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
                &ac, &hd1, &hd2, &th, &td, &ta)) return (15);

            k_ptr->ac = ac;
            k_ptr->dd = hd1;
            k_ptr->ds = hd2;
            k_ptr->tohit = th;
            k_ptr->todam = td;
            k_ptr->toac =  ta;

            /* Next... */
            continue;
        }

        /* Hack -- Process 'F' for flags */
        if (buf[0] == 'F') {

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






#ifdef BINARY_ARRAY_IMAGES


/*
 * Attempt to "dump" a "quick-load binary image" for "k_list"
 */
static errr dump_k_list_raw()
{
    int i, fd;

    int mode = 0644;

    char tmp[1024];


    /* Hack -- sizes */
    uint len[MAX_K_IDX];

    /* Size of the "lengths" array */
    uint slen = sizeof(len);

    /* Size of one normal record */
    uint size = sizeof(inven_kind);


#ifdef MACINTOSH
    _ftype = 'DATA';
#endif


    /* File "k_list.raw" -- raw records */
    sprintf(tmp, "%s%s%s", ANGBAND_DIR_DATA, PATH_SEP, "k_list.raw");
    fd = my_topen(tmp, O_RDWR | O_CREAT | O_BINARY, mode);
    if (fd < 0) return (-1);

    /* Dump the version info */
    tmp[0] = CUR_VERSION_MAJ;
    tmp[1] = CUR_VERSION_MIN;
    tmp[2] = CUR_PATCH_LEVEL;
    tmp[3] = size;

    /* Dump it */
    write(fd, tmp, 4);

    /* Attempt to dump the (normal) monster races */
    for (i = 0; i < MAX_K_IDX; i++) {
        if (size != write(fd, (char*)(&k_list[i]), size)) break;
    }

    /* Close and verify */
    gasp();
    close(fd);
    if (i < MAX_K_IDX) return (1);


    /* File "k_name.raw" -- item kind names */
    sprintf(tmp, "%s%s%s", ANGBAND_DIR_DATA, PATH_SEP, "k_name.raw");
    fd = my_topen(tmp, O_RDWR | O_CREAT | O_BINARY, mode);
    if (fd < 0) return (-1);

    /* Prepare the "lengths" array */
    for (i = 0; i < MAX_K_IDX; i++) {
        len[i] = string_size(k_list[i].name);
    }

    /* Dump the array */
    write(fd, (char*)(len), slen);

    /* Dump the names */
    for (i = 0; i < MAX_K_IDX; i++) {
        char *str = (char*)(k_list[i].name);
        if (len[i] && (len[i] != write(fd, str, len[i]))) break;
    }

    /* Close and verify */
    gasp();
    close(fd);
    if (i < MAX_K_IDX) return (1);


    /* Success */
    return (0);
}




/*
 * Attempt to "quick-load" a binary image for "k_list"
 */
static errr init_k_list_raw()
{
    int i, fd;

    int mode = 0644;

    char tmp[1024];


    /* Hack -- sizes */
    uint len[MAX_K_IDX];

    /* Size of the "lengths" array */
    uint slen = sizeof(len);

    /* Size of one normal record */
    uint size = sizeof(inven_kind);


#ifdef MACINTOSH
    _ftype = 'DATA';
#endif


    /* File "k_list.raw" -- raw records */
    sprintf(tmp, "%s%s%s", ANGBAND_DIR_DATA, PATH_SEP, "k_list.raw");
    fd = my_topen(tmp, O_RDONLY | O_BINARY, mode);
    if (fd < 0) return (-1);

    /* Read and Verify the version info */
    if ((4 != read(fd, tmp, 4)) ||
        (tmp[0] != CUR_VERSION_MAJ) ||
        (tmp[1] != CUR_VERSION_MIN) ||
        (tmp[2] != CUR_PATCH_LEVEL) ||
        (tmp[3] != size)) {

        /* Hack -- message */
        note("Ignoring old file 'lib/file/k_list.raw'...");

        /* Close */
        close(fd);

        /* Fail */
        return (-1);
    }

    /* Attempt to read the (normal) object kinds */
    for (i = 0; i < MAX_K_IDX; i++) {
        if (size != read(fd, (char*)(&k_list[i]), size)) break;
        k_list[i].name = NULL;
    }

    /* Close and verify */
    gasp();
    close(fd);
    if (i < MAX_K_IDX) return (1);


    /* File "k_name.raw" -- kind names */
    sprintf(tmp, "%s%s%s", ANGBAND_DIR_DATA, PATH_SEP, "k_name.raw");
    fd = my_topen(tmp, O_RDONLY | O_BINARY, mode);
    if (fd < 0) return (-1);

    /* Read the size array */
    if (slen != read(fd, (char*)(len), slen)) return (1);

    /* Attempt to read the (normal) item kind names */
    for (i = 0; i < MAX_K_IDX; i++) {
        if (len[i] != read(fd, tmp, len[i])) break;
        if (len[i]) k_list[i].name = string_make(tmp);
    }

    /* Close and verify */
    gasp();
    close(fd);
    if (i < MAX_K_IDX) return (1);


    /* Success */
    return (0);
}

#endif



/*
 * Note that "k_list" starts out totally cleared
 */
static void init_k_list()
{
    errr err;


#ifdef BINARY_ARRAY_IMAGES

    /* Try to load a "binary image" */
    err = init_k_list_raw();

    /* Otherwise try the text version */
    if (!err) return;

#endif

    /* Try a text version */
    err = init_k_list_txt();

    /* Still no luck? Fail! */
    if (err) {

        /* Warning */
        msg_format("Fatal error #%d parsing 'k_list.txt', record %d!",
                   err, error_k_idx);
        msg_print(NULL);

        /* Quit */
        quit("cannot load 'k_list.txt'");
    }

#ifdef BINARY_ARRAY_IMAGES

    /* Attempt to dump a "quick-load" version */
    err = dump_k_list_raw();

    /* Warn on errors */
    if (err) {
        msg_print("Warning: unable to create 'k_list.raw'");
        msg_print(NULL);
    }

#endif

}






/*
 * Grab one flag in a inven_very from a textual string
 *
 * Note the strange "if then else" blocks to prevent memory errors
 * during compilation.  The speed of this function is irrelevant.
 */
static bool grab_one_very_flag(inven_very *v_ptr, cptr what)
{
    u32b flags1 = 0L, flags2 = 0L, flags3 = 0L;


         if (streq(what, "STR"))		flags1 |= TR1_STR;
    else if (streq(what, "INT"))		flags1 |= TR1_INT;
    else if (streq(what, "WIS"))		flags1 |= TR1_WIS;
    else if (streq(what, "DEX"))		flags1 |= TR1_DEX;
    else if (streq(what, "CON"))		flags1 |= TR1_CON;
    else if (streq(what, "CHR"))		flags1 |= TR1_CHR;

    else if (streq(what, "SEARCH"))		flags1 |= TR1_SEARCH;
    else if (streq(what, "STEALTH"))		flags1 |= TR1_STEALTH;
    else if (streq(what, "TUNNEL"))		flags1 |= TR1_TUNNEL;
    else if (streq(what, "INFRA"))		flags1 |= TR1_INFRA;
    else if (streq(what, "SPEED"))		flags1 |= TR1_SPEED;
    else if (streq(what, "BLOWS"))		flags1 |= TR1_BLOWS;

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


         if (streq(what, "FREE_ACT"))		flags2 |= TR2_FREE_ACT;
    else if (streq(what, "HOLD_LIFE"))		flags2 |= TR2_HOLD_LIFE;

    else if (streq(what, "IM_FIRE"))		flags2 |= TR2_IM_FIRE;
    else if (streq(what, "IM_COLD"))		flags2 |= TR2_IM_COLD;
    else if (streq(what, "IM_ACID"))		flags2 |= TR2_IM_ACID;
    else if (streq(what, "IM_ELEC"))		flags2 |= TR2_IM_ELEC;
    else if (streq(what, "IM_POIS"))		flags2 |= TR2_IM_POIS;

    else if (streq(what, "RES_ACID"))		flags2 |= TR2_RES_ACID;
    else if (streq(what, "RES_ELEC"))		flags2 |= TR2_RES_ELEC;
    else if (streq(what, "RES_FIRE"))		flags2 |= TR2_RES_FIRE;
    else if (streq(what, "RES_COLD"))		flags2 |= TR2_RES_COLD;
    else if (streq(what, "RES_POIS"))		flags2 |= TR2_RES_POIS;
    else if (streq(what, "RES_LITE"))		flags2 |= TR2_RES_LITE;
    else if (streq(what, "RES_DARK"))		flags2 |= TR2_RES_DARK;

    else if (streq(what, "RES_BLIND"))		flags2 |= TR2_RES_BLIND;
    else if (streq(what, "RES_CONF"))		flags2 |= TR2_RES_CONF;
    else if (streq(what, "RES_SOUND"))		flags2 |= TR2_RES_SOUND;
    else if (streq(what, "RES_SHARDS"))		flags2 |= TR2_RES_SHARDS;

    else if (streq(what, "RES_NETHER"))		flags2 |= TR2_RES_NETHER;
    else if (streq(what, "RES_NEXUS"))		flags2 |= TR2_RES_NEXUS;
    else if (streq(what, "RES_CHAOS"))		flags2 |= TR2_RES_CHAOS;
    else if (streq(what, "RES_DISEN"))		flags2 |= TR2_RES_DISEN;

    else if (streq(what, "SUST_STR"))		flags2 |= TR2_SUST_STR;
    else if (streq(what, "SUST_INT"))		flags2 |= TR2_SUST_INT;
    else if (streq(what, "SUST_WIS"))		flags2 |= TR2_SUST_WIS;
    else if (streq(what, "SUST_DEX"))		flags2 |= TR2_SUST_DEX;
    else if (streq(what, "SUST_CON"))		flags2 |= TR2_SUST_CON;
    else if (streq(what, "SUST_CHR"))		flags2 |= TR2_SUST_CHR;


         if (streq(what, "EASY_KNOW"))		flags3 |= TR3_EASY_KNOW;
    else if (streq(what, "HIDE_TYPE"))		flags3 |= TR3_HIDE_TYPE;
    else if (streq(what, "SHOW_MODS"))		flags3 |= TR3_SHOW_MODS;
    else if (streq(what, "INSTA_ART"))		flags3 |= TR3_INSTA_ART;

    else if (streq(what, "FEATHER"))		flags3 |= TR3_FEATHER;
    else if (streq(what, "LITE"))		flags3 |= TR3_LITE;
    else if (streq(what, "SEE_INVIS"))		flags3 |= TR3_SEE_INVIS;
    else if (streq(what, "TELEPATHY"))		flags3 |= TR3_TELEPATHY;

    else if (streq(what, "SLOW_DIGEST"))	flags3 |= TR3_SLOW_DIGEST;
    else if (streq(what, "REGEN"))		flags3 |= TR3_REGEN;

    else if (streq(what, "XTRA_MIGHT"))		flags3 |= TR3_XTRA_MIGHT;
    else if (streq(what, "XTRA_SHOTS"))		flags3 |= TR3_XTRA_SHOTS;

#if 0
    else if (streq(what, "IGNORE_FIRE"))	flags3 |= TR3_IGNORE_FIRE;
    else if (streq(what, "IGNORE_COLD"))	flags3 |= TR3_IGNORE_COLD;
    else if (streq(what, "IGNORE_ELEC"))	flags3 |= TR3_IGNORE_ELEC;
    else if (streq(what, "IGNORE_ACID"))	flags3 |= TR3_IGNORE_ACID;
#endif

    else if (streq(what, "ACTIVATE"))		flags3 |= TR3_ACTIVATE;
    else if (streq(what, "DRAIN_EXP"))		flags3 |= TR3_DRAIN_EXP;
    else if (streq(what, "TELEPORT"))		flags3 |= TR3_TELEPORT;
    else if (streq(what, "AGGRAVATE"))		flags3 |= TR3_AGGRAVATE;

    else if (streq(what, "BLESSED"))		flags3 |= TR3_BLESSED;
    else if (streq(what, "CURSED"))		flags3 |= TR3_CURSED;
    else if (streq(what, "HEAVY_CURSE"))	flags3 |= TR3_HEAVY_CURSE;
    else if (streq(what, "PERMA_CURSE"))	flags3 |= TR3_PERMA_CURSE;


    if (!flags1 && !flags2 && !flags3) return (FALSE);

    if (flags1) v_ptr->flags1 |= flags1;
    if (flags2) v_ptr->flags2 |= flags2;
    if (flags3) v_ptr->flags3 |= flags3;

    return (TRUE);
}


/*
 * Hack -- location saver for error messages
 */
static int error_v_idx = -1;


/*
 * Initialize the "v_list" array by parsing a file
 * Note that "v_list" starts out totally cleared
 */
static errr init_v_list_txt()
{
    char *s, *t;

    /* Not ready */
    bool okay = FALSE;

    /* No item very yet */
    int m = -1;

    /* No v_ptr yet */
    inven_very *v_ptr = NULL;

    /* The "objects" file */
    FILE *fp;

    /* General buffer */
    char buf[1024];


    /* Access the "v_list.txt" file */
    sprintf(buf, "%s%s%s", ANGBAND_DIR_FILE, PATH_SEP, "v_list.txt");

    /* Open the file */
    fp = fopen(buf, "r");

    /* Failure */
    if (!fp) return (-1);


    /* Parse the file to initialize "v_list" */
    while (1) {

        /* Read a line from the file, stop when done */
        if (!fgets(buf, 160, fp)) break;

        /* Skip comments */
        if (buf[0] == '#') continue;

        /* Strip the final newline */
        for (s = buf; isprint(*s); ++s); *s = '\0';

        /* Blank lines terminate artifact entries */
        if (!buf[0]) {

            /* No current v_ptr */
            if (!v_ptr) continue;

            /* Now there is no current v_ptr */
            v_ptr = NULL;

            /* Don't hog the processor */
            gasp();

            /* Next... */
            continue;
        }

        /* The line better have a colon and such */
        if (buf[1] != ':') return (1);

        /* Process 'V' for "Version" */
        if (buf[0] == 'V') {

            int v1, v2, v3;

            /* Scan for the values */
            if ((3 != sscanf(buf, "V:%d.%d.%d", &v1, &v2, &v3)) ||
                (v1 != CUR_VERSION_MAJ) ||
                (v2 != CUR_VERSION_MIN) ||
                (v3 != CUR_PATCH_LEVEL)) {

                msg_print("Warning: your 'v_list.txt' is obsolete!");
                msg_print(NULL);

                return (99);
            }

            /* Okay to proceed */
            okay = TRUE;

            /* Continue */
            continue;
        }

        /* Process 'N' for "New/Number/Name" */
        if (buf[0] == 'N') {

            /* No version */
            if (!okay) return (99);

            /* Not done the previous one */
            if (v_ptr) return (2);

            /* Find, verify, and nuke the colon before the name */
            if (!(s = strchr(buf+2, ':'))) return (3);

            /* Nuke the colon, advance to the name */
            *s++ = '\0';

            /* Do not allow empty names */
            if (!*s) return (4);

            /* Get the index */
            m = atoi(buf+2);

            /* For errors */
            error_v_idx = m;

            /* Verify */
            if ((m < 0) || (m >= MAX_V_IDX)) return (5);

            /* Start a new v_ptr */
            v_ptr = &v_list[m];

            /* Make sure we have not done him yet */
            if (v_ptr->name) return (6);

            /* Save the name */
            v_ptr->name = string_make(s);

            /* Set a few flags */
            v_ptr->flags3 |= TR3_IGNORE_ACID;
            v_ptr->flags3 |= TR3_IGNORE_ELEC;
            v_ptr->flags3 |= TR3_IGNORE_FIRE;
            v_ptr->flags3 |= TR3_IGNORE_COLD;

            /* Next... */
            continue;
        }

        /* There better be a current v_ptr */
        if (!v_ptr) return (10);

        /* Process 'I' for "Info" (one line only) */
        if (buf[0] == 'I') {

            int tval, sval, pval;

            /* Scan for the values */
            if (3 != sscanf(buf+2, "%d:%d:%d",
                &tval, &sval, &pval)) return (11);

            /* Save the values */
            v_ptr->tval = tval;
            v_ptr->sval = sval;
            v_ptr->pval = pval;

            /* Next... */
            continue;
        }

        /* Process 'W' for "More Info" (one line only) */
        if (buf[0] == 'W') {

            int level, rarity, wgt;
            long cost;

            /* Scan for the values */
            if (4 != sscanf(buf+2, "%d:%d:%d:%ld",
                &level, &rarity, &wgt, &cost)) return (11);

            /* Save the values */
            v_ptr->level = level;
            v_ptr->rarity = rarity;
            v_ptr->weight = wgt;
            v_ptr->cost = cost;

            /* Next... */
            continue;
        }

        /* Hack -- Process 'P' for "power" and such */
        if (buf[0] == 'P') {

            int ac, hd1, hd2, th, td, ta;

            /* Scan for the values */
            if (6 != sscanf(buf+2, "%d:%dd%d:%d:%d:%d",
                &ac, &hd1, &hd2, &th, &td, &ta)) return (15);

            v_ptr->ac = ac;
            v_ptr->dd = hd1;
            v_ptr->ds = hd2;
            v_ptr->tohit = th;
            v_ptr->todam = td;
            v_ptr->toac =  ta;

            /* Next... */
            continue;
        }

        /* Hack -- Process 'F' for flags */
        if (buf[0] == 'F') {

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
                if (!grab_one_very_flag(v_ptr, s)) return (18);

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







#ifdef BINARY_ARRAY_IMAGES

/*
 * Attempt to "dump" a "quick-load binary image" for "v_list"
 */
static errr dump_v_list_raw()
{
    int i, fd;

    int mode = 0644;

    char tmp[1024];


    /* Hack -- sizes */
    uint len[MAX_V_IDX];

    /* Size of the "lengths" array */
    uint slen = sizeof(len);

    /* Size of one normal record */
    uint size = sizeof(inven_very);


#ifdef MACINTOSH
    _ftype = 'DATA';
#endif


    /* File "v_list.raw" -- raw records */
    sprintf(tmp, "%s%s%s", ANGBAND_DIR_DATA, PATH_SEP, "v_list.raw");
    fd = my_topen(tmp, O_RDWR | O_CREAT | O_BINARY, mode);
    if (fd < 0) return (-1);

    /* Dump the version info */
    tmp[0] = CUR_VERSION_MAJ;
    tmp[1] = CUR_VERSION_MIN;
    tmp[2] = CUR_PATCH_LEVEL;
    tmp[3] = size;

    /* Dump it */
    write(fd, tmp, 4);

    /* Attempt to dump the artifact records */
    for (i = 0; i < MAX_V_IDX; i++) {
        if (size != write(fd, (char*)(&v_list[i]), size)) break;
    }

    /* Close and verify */
    gasp();
    close(fd);
    if (i < MAX_V_IDX) return (1);


    /* File "v_name.raw" -- artifact names */
    sprintf(tmp, "%s%s%s", ANGBAND_DIR_DATA, PATH_SEP, "v_name.raw");
    fd = my_topen(tmp, O_RDWR | O_CREAT | O_BINARY, mode);
    if (fd < 0) return (-1);

    /* Prepare the "lengths" array */
    for (i = 0; i < MAX_V_IDX; i++) {
        len[i] = string_size(v_list[i].name);
    }

    /* Dump the array */
    write(fd, (char*)(len), slen);

    /* Dump the names */
    for (i = 0; i < MAX_V_IDX; i++) {
        char *str = (char*)(v_list[i].name);
        if (len[i] && (len[i] != write(fd, str, len[i]))) break;
    }

    /* Close and verify */
    gasp();
    close(fd);
    if (i < MAX_V_IDX) return (1);


    /* Success */
    return (0);
}




/*
 * Attempt to "quick-load" a binary image for "v_list"
 */
static errr init_v_list_raw()
{
    int i, fd;

    int mode = 0644;

    char tmp[1024];


    /* Hack -- sizes */
    uint len[MAX_V_IDX];

    /* Size of the "lengths" array */
    uint slen = sizeof(len);

    /* Size of one normal record */
    uint size = sizeof(inven_very);


#ifdef MACINTOSH
    _ftype = 'DATA';
#endif



    /* File "v_list.raw" -- raw records */
    sprintf(tmp, "%s%s%s", ANGBAND_DIR_DATA, PATH_SEP, "v_list.raw");
    fd = my_topen(tmp, O_RDONLY | O_BINARY, mode);
    if (fd < 0) return (-1);

    /* Read and Verify the version info */
    if ((4 != read(fd, tmp, 4)) ||
        (tmp[0] != CUR_VERSION_MAJ) ||
        (tmp[1] != CUR_VERSION_MIN) ||
        (tmp[2] != CUR_PATCH_LEVEL) ||
        (tmp[3] != size)) {

        /* Hack -- message */
        note("Ignoring old file 'lib/file/v_list.raw'...");

        /* Close */
        close(fd);

        /* Fail */
        return (-1);
    }

    /* Attempt to read the (normal) object kinds */
    for (i = 0; i < MAX_V_IDX; i++) {
        if (size != read(fd, (char*)(&v_list[i]), size)) break;
        v_list[i].name = NULL;
    }

    /* Close and verify */
    gasp();
    close(fd);
    if (i < MAX_V_IDX) return (1);


    /* File "v_name.raw" -- artifact names */
    sprintf(tmp, "%s%s%s", ANGBAND_DIR_DATA, PATH_SEP, "v_name.raw");
    fd = my_topen(tmp, O_RDONLY | O_BINARY, mode);
    if (fd < 0) return (-1);

    /* Read the size array */
    if (slen != read(fd, (char*)(len), slen)) return (1);

    /* Attempt to read the artifact names */
    for (i = 0; i < MAX_V_IDX; i++) {
        if (len[i] != read(fd, tmp, len[i])) break;
        if (len[i]) v_list[i].name = string_make(tmp);
    }

    /* Close and verify */
    gasp();
    close(fd);
    if (i < MAX_V_IDX) return (1);


    /* Success */
    return (0);
}

#endif


/*
 * Note that "v_list" starts out totally cleared
 */
static void init_v_list()
{
    errr err;


#ifdef BINARY_ARRAY_IMAGES

    /* Try to load a "binary image" */
    err = init_v_list_raw();

    /* Otherwise try the text version */
    if (!err) return;

#endif

    /* Try a text version */
    err = init_v_list_txt();

    /* Still no luck? Fail! */
    if (err) {

        /* Warning */
        msg_format("Fatal error #%d parsing 'v_list.txt', record %d!",
                   err, error_v_idx);
        msg_print(NULL);

        /* Quit */
        quit("cannot load 'v_list.txt'");
    }

#ifdef BINARY_ARRAY_IMAGES

    /* Attempt to dump a "quick-load" version */
    err = dump_v_list_raw();

    /* Warn on error */
    if (err) {
        msg_print("Warning: unable to create 'v_list.raw'");
        msg_print(NULL);
    }

#endif

}



/*
 * Objects sold in the stores -- by tval/sval pair.
 */
static byte store_choice[MAX_STORES-2][STORE_CHOICES][2] = {

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
    { TV_SCROLL, SV_SCROLL_WORD_OF_RECALL },
    { TV_POTION, SV_POTION_RES_WIS },
    { TV_POTION, SV_POTION_CURE_LIGHT },
    { TV_POTION, SV_POTION_CURE_SERIOUS },
    { TV_POTION, SV_POTION_CURE_CRITICAL },
    { TV_POTION, SV_POTION_HEROISM },
    { TV_POTION, SV_POTION_BOLDNESS },
    { TV_POTION, SV_POTION_RESTORE_EXP },
    { TV_POTION, SV_POTION_RESIST_HEAT },
    { TV_POTION, SV_POTION_RESIST_COLD },
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
    { TV_SCROLL, SV_SCROLL_LIGHT },
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
static void init_other(void)
{
    int i, j, k;

    inven_kind *k_ptr;
    monster_race *r_ptr;

    s16b aux[256];



    /*** Prepare the Object Kind Allocator ***/

    /* Clear the aux array */
    C_WIPE(&aux, MAX_K_LEV, s16b);

    /* Make the index */
    C_MAKE(alloc_kind_index, MAX_K_LEV, s16b);
        
    /* Scan all of the objects */
    for (i = 0; i < MAX_K_IDX; i++) {

        /* Get the i'th object */
        k_ptr = &k_list[i];

        /* Scan all of the locale/chance pairs */
        for (j = 0; j < 4; j++) {

            /* Count valid pairs */
            if (k_ptr->chance[j] && (k_ptr->locale[j] < MAX_K_LEV)) {

                /* Count the total entries */
                alloc_kind_size++;

                /* Count the entries at each level */
                alloc_kind_index[k_ptr->locale[j]]++;
            }
        }
    }

    /* Combine the "alloc_kind_index" entries */
    for (i = 1; i < MAX_K_LEV; i++) alloc_kind_index[i] += alloc_kind_index[i-1];

    /* Allocate the table */
    C_MAKE(alloc_kind_table, alloc_kind_size, kind_entry);

    /* Initialize the table */
    for (i = 0; i < MAX_K_IDX; i++) {

        /* Get the i'th object */
        k_ptr = &k_list[i];

        /* Scan all of the locale/chance pairs */
        for (j = 0; j < 4; j++) {

            /* Count valid pairs */
            if (k_ptr->chance[j] && (k_ptr->locale[j] < MAX_K_LEV)) {

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
    C_WIPE(&aux, MAX_R_LEV, s16b);

    /* Allocate and clear the index */
    C_MAKE(alloc_race_index, MAX_R_LEV, s16b);
        
    /* Scan the monsters (not the ghost) */
    for (i = 1; i < MAX_R_IDX-1; i++) {

        /* Get the i'th race */
        r_ptr = &r_list[i];

        /* Process "real" monsters */
        if (r_ptr->rarity && (r_ptr->level < MAX_R_LEV)) {

            /* Count the total entries */
            alloc_race_size++;

            /* Count the entries at each level */
            alloc_race_index[r_ptr->level]++;
        }
    }

    /* Combine the "alloc_race_index" entries */
    for (i = 1; i < MAX_R_LEV; i++) alloc_race_index[i] += alloc_race_index[i-1];

    /* Allocate the alloc_race_table */
    C_MAKE(alloc_race_table, alloc_race_size, race_entry);

    /* Scan the monsters (not the ghost) */
    for (i = 1; i < MAX_R_IDX-1; i++) {

        /* Get the i'th race */
        r_ptr = &r_list[i];

        /* Count valid pairs */
        if (r_ptr->rarity && (r_ptr->level < MAX_R_LEV)) {

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


    /*** Prepare the Store Catalog Array ***/
    
    /* Scan the stores */
    for (i = 0; i < MAX_STORES-2; i++) {    
        
        /* Scan the choices */
        for (k = 0; k < STORE_CHOICES; k++) {
            
            /* Extract the tval/sval codes */
            int tv = store_choice[i][k][0];
            int sv = store_choice[i][k][1];
            
            /* Locate that item type */
            store_choice_kind[i][k] = lookup_kind(tv, sv);
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


    /*** Pre-allocate space for the "format()" buffer ***/
    
    /* Hack -- Just call the "format()" function */
    (void)format("%s (%s).", "Ben Harrison", "benh@linc.cis.upenn.edu");


    /*** Pre-allocate space for save/restore screen ***/
    
    /* Save the screen */
    save_screen();
    
    /* Save the screen (embedded) */
    save_screen();
    
    /* Restore the screen (embedded) */
    restore_screen();

    /* Restore the screen */
    restore_screen();
}



/*
 * This routine allocates and prepares several large arrays.
 *
 * Note that the "C_MAKE()" macro allocates "clean" memory.
 *
 * See "desc.c" for the annoying code required to handle the
 * "flavored" objects, which do not have "fixed" attr fields.
 */
void init_some_arrays()
{
    int i;


    /* Label the task */
    note("[Initializing arrays...]");


    /* Allocate and Wipe the array of monster "race info" */
    C_MAKE(r_list, MAX_R_IDX, monster_race);

    /* Allocate and Wipe the array of monster "memories" */
    C_MAKE(l_list, MAX_R_IDX, monster_lore);


    /* Allocate and Wipe the array of object "kind info" */
    C_MAKE(k_list, MAX_K_IDX, inven_kind);

    /* Allocate and Wipe the array of object "memories" */
    C_MAKE(x_list, MAX_K_IDX, inven_xtra);


    /* Allocate and Wipe the array of artifact templates */
    C_MAKE(v_list, MAX_V_IDX, inven_very);


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



    /* Initialize k_list from a file */
    note("[Initializing arrays... (objects)]");
    init_k_list();

    /* Initialize v_list from a file */
    note("[Initializing arrays... (artifacts)]");
    init_v_list();

    /* Initialize r_list from a file of some kind */
    note("[Initializing arrays... (monsters)]");
    init_r_list();

    /* Initialize some other arrays */
    note("[Initializing arrays... (various)]");
    init_other();


    /* Hack -- all done */
    note("[Initializing arrays... done]");
}



