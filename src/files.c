/* File: files.c */

/* Purpose: code dealing with files (and death) */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"



/*
 * You may or may not want to use the following "#undef".
 */
/* #undef _POSIX_SAVED_IDS */


/*
 * Hack -- drop permissions
 */
void safe_setuid_drop(void)
{

#ifdef SET_UID

# ifdef SAFE_SETUID

#  ifdef SAFE_SETUID_POSIX

    if (setuid(getuid()) != 0) {
        quit("setuid(): cannot set permissions correctly!");
    }
    if (setgid(getgid()) != 0) {
        quit("setgid(): cannot set permissions correctly!");
    }

#  else

    if (setreuid(geteuid(), getuid()) != 0) {
        quit("setreuid(): cannot set permissions correctly!");
    }
    if (setregid(getegid(), getgid()) != 0) {
        quit("setregid(): cannot set permissions correctly!");
    }

#  endif

# endif

#endif

}


/*
 * Hack -- grab permissions
 */
void safe_setuid_grab(void)
{

#ifdef SET_UID

# ifdef SAFE_SETUID

#  ifdef SAFE_SETUID_POSIX

    if (setuid(player_euid) != 0) {
        quit("setuid(): cannot set permissions correctly!");
    }
    if (setgid(player_egid) != 0) {
        quit("setgid(): cannot set permissions correctly!");
    }

#  else

    if (setreuid(geteuid(), getuid()) != 0) {
        quit("setreuid(): cannot set permissions correctly!");
    }
    if (setregid(getegid(), getgid()) != 0) {
        quit("setregid(): cannot set permissions correctly!");
    }

#  endif

# endif

#endif

}


/*
 * Parse a sub-file of the "extra info" (format shown below)
 *
 * Each "action" line has an "action symbol" in the first column,
 * followed by a colon, followed by some command specific info,
 * usually with "fields" separated by colons.  Blank lines and
 * lines starting with pound signs ("#") are ignored (as comments).
 *
 * Note that <a>, <c>, <num>, <tv>, <sv>, <f> must be integers.
 * Note that <str> must be an "encoded" string (see elsewhere).
 * Note that <key> must be a keypress (a number from 1 to 255) or zero.
 * Note that <dir> must be a direction (a number from 1 to 9) or zero.
 *
 * Parse another file (recursively):
 *   %:<filename>		<-- see next function
 *
 * Specify the attr/char values for "monsters":
 *   R:<num>:<a>/<c>		<-- attr/char by race index
 *   M:<a>/<c>:<a>/<c>		<-- attr/char by race attr/char
 *
 * Specify the attr/char values for "objects":
 *   K:<num>:<a>/<c>		<-- attr/char by kind index
 *   I:<a>/<c>:<a>/<c>		<-- attr/char by kind attr/char
 *   T:<tv>,<sv>:<a>/<c>	<-- attr/char by kind tval/sval
 *
 * Specify the attr/char values for unaware "objects":
 *   U:<tv>:<a>/<c>		<-- attr/char by kind tval
 *
 * Specify the attr/char values for inventory "objects":
 *   E:<tv>:<a>/<c>		<-- attr/char by kind tval
 *
 * Specify the attr/char values for terrain features:
 *   F:<f>:<a>/<c>		<-- attr/char by feature index
 *
 * Specify macros and command-macros:
 *   A:<str>			<-- macro action (encoded)
 *   P:<str>			<-- macro pattern (encoded)
 *   C:<str>			<-- command macro pattern (encoded)
 *
 * Specify keyset mappings:
 *   S:<key>:<key>:<dir>	<-- keyset mapping
 *
 * Specify option settings:
 *   X:<str>			<-- Set an option to NO/OFF/FALSE
 *   Y:<str>			<-- Set an option to YES/ON/TRUE
 */
static errr process_pref_file_aux(cptr name)
{
    int i, n, i1, i2, n1, n2;

    FILE *fp;

    /* Current input line */
    char buf[1024];

    /* Current macro data */
    char pat[1024] = "";
    char act[1024] = "";


    /* Open the file */
    fp = my_fopen(name, "r");

    /* Catch errors */
    if (!fp) return (-1);

    /* Process the file */
    while (1) {

        /* Read a line from the file, stop when done */
        if (!fgets(buf, 1024, fp)) break;

        /* Skip comments */
        if (buf[0] == '#') continue;

        /* See how long the input is */
        i = strlen(buf);

        /* Strip the final newline (and spaces) */
        while (i && isspace(buf[i-1])) buf[--i] = '\0';

        /* Skip blank lines */
        if (!buf[0]) continue;

        /* The line better have a colon and such */
        if (buf[1] != ':') {
            msg_format("Bad command <%s> in file <%s>.", buf, name);
            continue;
        }

        /* Process "%:<fname>" */
        if (buf[0] == '%') {

            /* Attempt to Process the given file */
            (void)process_pref_file(buf + 2);
        }

        /* Process "R:<num>:<a>/<c>" */
        else if (buf[0] == 'R') {
            monster_race *r_ptr;
            if (sscanf(buf, "R:%d:%d/%d", &i, &n1, &n2) != 3) {
                msg_format("Bad command <%s> in file <%s>.", buf, name);
                continue;
            }
            r_ptr = &r_info[i];
            if (n1) r_ptr->l_attr = n1;
            if (n2) r_ptr->l_char = n2;
        }

        /* Process "M:<a>/<c>:<a>/<c>" */
        else if (buf[0] == 'M') {
            if (sscanf(buf, "M:%d/%d:%d/%d", &i1, &i2, &n1, &n2) != 4) {
                msg_format("Bad command <%s> in file <%s>.", buf, name);
                continue;
            }
            for (i = 1; i < MAX_R_IDX; i++) {
                monster_race *r_ptr = &r_info[i];
                if ((!i1 || r_ptr->r_attr == i1) &&
                    (!i2 || r_ptr->r_char == i2)) {
                    if (n1) r_ptr->l_attr = n1;
                    if (n2) r_ptr->l_char = n2;
                }
            }
        }

        /* Process "K:<num>:<a>/<c>" */
        else if (buf[0] == 'K') {
            if (sscanf(buf, "K:%d:%d/%d", &i, &n1, &n2) != 3) {
                msg_format("Bad command <%s> in file <%s>.", buf, name);
                continue;
            }
            if (n1) k_info[i].x_attr = n1;
            if (n2) k_info[i].x_char = n2;
        }

        /* Process "I:<a>/<c>:<a>/<c>" */
        else if (buf[0] == 'I') {
            if (sscanf(buf, "I:%d/%d:%d/%d", &i1, &i2, &n1, &n2) != 4) {
                msg_format("Bad command <%s> in file <%s>.", buf, name);
                continue;
            }
            for (i = 0; i < MAX_K_IDX; i++) {
                if ((!i1 || k_info[i].k_attr == i1) &&
                    (!i2 || k_info[i].k_char == i2)) {
                    if (n1) k_info[i].x_attr = n1;
                    if (n2) k_info[i].x_char = n2;
                }
            }
        }

        /* Process "T:<tv>,<sv>:<a>/<c>" */
        else if (buf[0] == 'T') {
            if (sscanf(buf, "T:%d,%d:%d/%d", &i1, &i2, &n1, &n2) != 4) {
                msg_format("Bad command <%s> in file <%s>.", buf, name);
                continue;
            }
            for (i = 0; i < MAX_K_IDX; i++) {
                if ((!i1 || k_info[i].tval == i1) &&
                    (!i2 || k_info[i].sval == i2)) {
                    if (n1) k_info[i].x_attr = n1;
                    if (n2) k_info[i].x_char = n2;
                }
            }
        }

        /* Process "U:<tv>:<a>/<c>" -- attr/char for unaware items */
        else if (buf[0] == 'U') {
            if (sscanf(buf, "U:%d:%d/%d", &i1, &n1, &n2) != 3) {
                msg_format("Bad command <%s> in file <%s>.", buf, name);
                continue;
            }
            for (i = 0; i < MAX_K_IDX; i++) {
                if (!i1 || (k_info[i].tval == i1)) {
                    if (n1) k_info[i].i_attr = n1;
                    if (n2) k_info[i].i_char = n2;
                }
            }
        }

        /* Process "E:<tv>:<a>/<c>" -- attr/char for equippy chars */
        else if (buf[0] == 'E') {
            if (sscanf(buf, "E:%d:%d/%d", &i1, &n1, &n2) != 3) {
                msg_format("Bad command <%s> in file <%s>.", buf, name);
                continue;
            }
            if (n1) tval_to_attr[i1] = n1;
            if (n2) tval_to_char[i1] = n2;
        }

        /* Process "F:<f>:<a>/<c>" -- attr/char for terrain features */
        else if (buf[0] == 'F') {
            if (sscanf(buf, "F:%d:%d/%d", &i1, &n1, &n2) != 3) {
                msg_format("Bad command <%s> in file <%s>.", buf, name);
                continue;
            }
            if (n1) f_info[i1].z_attr = n1;
            if (n2) f_info[i1].z_char = n2;
        }

        /* Process "A:<str>" -- save an "action" for later */
        else if (buf[0] == 'A') {
            text_to_ascii(act, buf+2);
        }

        /* Process "P:<str>" -- normal-macro trigger */
        else if (buf[0] == 'P') {
            text_to_ascii(pat, buf+2);
            macro_add(pat, act, FALSE);
        }

        /* Process "C:<str>" -- command-macro trigger */
        else if (buf[0] == 'C') {
            text_to_ascii(pat, buf+2);
            macro_add(pat, act, TRUE);
        }

        /* Process "S:<key>:<key>:<dir>" -- keymap */
        else if (buf[0] == 'S') {
            if (sscanf(buf, "S:%d:%d:%d", &i1, &n1, &n2) != 3) {
                msg_format("Bad command <%s> in file <%s>.", buf, name);
                continue;
            }
            if ((i1 < 0) || (i1 > 255)) continue;
            if ((n1 < 0) || (n1 > 255)) n1 = 0;
            if ((n2 < 1) || (n2 > 9) || (n2 == 5)) n2 = 0;
            keymap_cmds[i1] = n1;
            keymap_dirs[i1] = n2;
        }

        /* Process "X:<str>" -- turn option off */
        else if (buf[0] == 'X') {
            for (n = 0, i = 0; options[i].o_desc; i++) {
                if (options[i].o_var &&
                    options[i].o_text &&
                    streq(options[i].o_text, buf + 2)) {
                    (*options[i].o_var) = FALSE;
                    n = i;
                    break;
                }
            }
            if (!n) {
                msg_format("Unknown option <%s> in file <%s>.", buf, name);
                continue;
            }
        }

        /* Process "Y:<str>" -- turn option on */
        else if (buf[0] == 'Y') {
            for (n = 0, i = 0; options[i].o_desc; i++) {
                if (options[i].o_var &&
                    options[i].o_text &&
                    streq(options[i].o_text, buf + 2)) {
                    (*options[i].o_var) = TRUE;
                    n = i;
                    break;
                }
            }
            if (!n) {
                msg_format("Unknown option <%s> in file <%s>.", buf, name);
                continue;
            }
        }

        /* Illegal command */
        else {
            msg_format("Unknown command <%s> in file <%s>.", buf, name);
            continue;
        }
    }

    /* Close the file */
    my_fclose(fp);


    /* Hack -- note use of "cheat" options */
    if (cheat_peek) noscore |= 0x0100;
    if (cheat_hear) noscore |= 0x0200;
    if (cheat_room) noscore |= 0x0400;
    if (cheat_xtra) noscore |= 0x0800;
    if (cheat_know) noscore |= 0x1000;
    if (cheat_live) noscore |= 0x2000;


    /* Success */
    return (0);
}


/*
 * Find a pref file with the given name and process it
 * Looks in the current directory, and the "USER" directories
 */
errr process_pref_file(cptr name)
{
    char tmp[1024];

    /* XXX XXX XXX Hack -- Try the given file */
    if (0 == process_pref_file_aux(name)) return (0);

    /* Look in the "user" directory */
    sprintf(tmp, "%s%s", ANGBAND_DIR_USER, name);

    /* Attempt to process that file */
    if (0 == process_pref_file_aux(tmp)) return (0);

    /* Oh well */
    return (1);
}







#ifdef CHECK_TIME

/*
 * Operating hours for ANGBAND (defaults to non-work hours)
 */
static char days[7][29] = {
    "SUN:XXXXXXXXXXXXXXXXXXXXXXXX",
    "MON:XXXXXXXX.........XXXXXXX",
    "TUE:XXXXXXXX.........XXXXXXX",
    "WED:XXXXXXXX.........XXXXXXX",
    "THU:XXXXXXXX.........XXXXXXX",
    "FRI:XXXXXXXX.........XXXXXXX",
    "SAT:XXXXXXXXXXXXXXXXXXXXXXXX"
};

/*
 * Restict usage (defaults to no restrictions)
 */
static bool check_time_flag = FALSE;

#endif


/*
 * Handle CHECK_TIME
 */
errr check_time(void)
{

#ifdef CHECK_TIME

    time_t              c;
    struct tm		*tp;

    /* No restrictions */
    if (!check_time_flag) return (0);

    /* Check for time violation */
    c = time((time_t *)0);
    tp = localtime(&c);

    /* Violation */
    if (days[tp->tm_wday][tp->tm_hour + 4] != 'X') return (1);

#endif

    /* Success */
    return (0);
}



/*
 * Initialize CHECK_TIME
 */
errr check_time_init(void)
{

#ifdef CHECK_TIME

    FILE        *fp;

    char	buf[1024];


    /* Access the "hours" file */
    sprintf(buf, "%s%s", ANGBAND_DIR_FILE, "time.txt");

    /* Open the file */
    fp = my_fopen(buf, "r");

    /* No file, no restrictions */
    if (!fp) return (0);

    /* Assume restrictions */
    check_time_flag = TRUE;

    /* Parse the file */
    while (fgets(buf, 80, fp)) {

        /* Skip comments */
        if (buf[0] == '#') continue;

        /* Skip "blank" lines */
        if (!buf[0] || !isprint(buf[0]) || isspace(buf[0])) continue;

        /* Chop the buffer */
        buf[29] = '\0';

        /* Extract the info */
        if (prefix(buf, "SUN:")) strcpy(days[0], buf);
        if (prefix(buf, "MON:")) strcpy(days[1], buf);
        if (prefix(buf, "TUE:")) strcpy(days[2], buf);
        if (prefix(buf, "WED:")) strcpy(days[3], buf);
        if (prefix(buf, "THU:")) strcpy(days[4], buf);
        if (prefix(buf, "FRI:")) strcpy(days[5], buf);
        if (prefix(buf, "SAT:")) strcpy(days[6], buf);
    }

    /* Close it */
    my_fclose(fp);

#endif

    /* Success */
    return (0);
}



#ifdef CHECK_LOAD

#ifndef MAXHOSTNAMELEN
# define MAXHOSTNAMELEN  64
#endif

typedef struct statstime {

    int                 cp_time[4];
    int                 dk_xfer[4];
    unsigned int        v_pgpgin;
    unsigned int        v_pgpgout;
    unsigned int        v_pswpin;
    unsigned int        v_pswpout;
    unsigned int        v_intr;
    int                 if_ipackets;
    int                 if_ierrors;
    int                 if_opackets;
    int                 if_oerrors;
    int                 if_collisions;
    unsigned int        v_swtch;
    long                avenrun[3];
    struct timeval      boottime;
    struct timeval      curtime;

} statstime;

/*
 * Maximal load (if any).
 */
static int check_load_value = 0;

#endif


/*
 * Handle CHECK_LOAD
 */
errr check_load(void)
{

#ifdef CHECK_LOAD

    struct statstime    st;

    /* Success if not checking */
    if (!check_load_value) return (0);

    /* Check the load */
    if (0 == rstat("localhost", &st)) {

        long val1 = (long)(st.avenrun[2]);
        long val2 = (long)(check_load_value) * FSCALE;

        /* Check for violation */
        if (val1 >= val2) return (1);
    }

#endif

    /* Success */
    return (0);
}


/*
 * Initialize CHECK_LOAD
 */
errr check_load_init(void)
{

#ifdef CHECK_LOAD

    FILE        *fp;

    char	buf[1024];

    char	temphost[MAXHOSTNAMELEN+1];
    char	thishost[MAXHOSTNAMELEN+1];


    /* Access the "load" file */
    sprintf(buf, "%s%s", ANGBAND_DIR_FILE, "load.txt");

    /* Open the "load" file */
    fp = my_fopen(buf, "r");

    /* No file, no restrictions */
    if (!fp) return (0);

    /* Default load */
    check_load_value = 100;

    /* Get the host name */
    (void)gethostname(thishost, (sizeof thishost) - 1);

    /* Parse it */
    while (fgets(buf, sizeof(buf), fp)) {

        int value;

        /* Skip comments */
        if (buf[0] == '#') continue;

        /* Skip "blank" lines */
        if (!buf[0] || !isprint(buf[0]) || isspace(buf[0])) continue;

        /* Parse, or ignore */
        if (sscanf(buf, "%s%d", temphost, &value) != 2) continue;

        /* Skip other hosts */
        if (!streq(temphost,thishost) && !streq(temphost,"localhost")) continue;

        /* Use that value */
        check_load_value = value;
        break;
    }

    /* Close the file */
    my_fclose(fp);

#endif

    /* Success */
    return (0);
}


/*
 * Attempt to open, and display, the "lib/file/news.txt" file.
 *
 * This function is called before "init_some_arrays()".
 */
void show_news(void)
{
    int i;

    FILE        *fp;

    char	*s;

    char	buf[1024];


    /* Construct the name of the "news" file */
    sprintf(buf, "%s%s", ANGBAND_DIR_FILE, "news.txt");

    /* Open the News file */
    fp = my_fopen(buf, "r");

    /* No 'news.txt' file? */
    if (!fp) quit_fmt("Cannot open the '%s' file!", buf);

    /* Clear the screen */
    clear_screen();

    /* Dump the file to the screen */
    for (i = 0; (i < 20) && fgets(buf, 80, fp); i++) {

        /* Advance to "weirdness" (including the final newline) */
        for (s = buf; isprint(*s); ++s) ;
        
        /* Nuke "weirdness" */
        *s = '\0';

        /* Display the line */
        put_str(buf, i+2, 0);

        /* Flush it */
        Term_fresh();
    }

    /* Flush it */
    Term_fresh();

    /* Close */
    my_fclose(fp);
}



/*
 * Recursive "help file" perusal.  Return FALSE on "ESCAPE".
 */
static bool do_cmd_help_aux(cptr name, int line)
{
    int		i, k, n;

    /* Number of "real" lines passed by */
    int		next = 0;

    /* Number of "real" lines in the file */
    int		size = 0;

    /* Backup value for "line" */
    int		back = 0;

    /* This screen has sub-screens */
    bool	menu = FALSE;

    /* Current help file */
    FILE	*fff = NULL;

    /* Find this string (if any) */
    cptr	find = NULL;

    /* Hold a string to find */
    char	finder[128] = "";

    /* Path buffer */
    char	path[1024];

    /* General buffer */
    char	buf[1024];

    /* Sub-menu information */
    char	hook[10][32];


    /* Wipe the hooks */
    for (i = 0; i < 10; i++) hook[i][0] = '\0';


    /* Build the standard file name */
    sprintf(path, "%s%s", ANGBAND_DIR_HELP, name);

    /* Open the file */
    fff = my_fopen(path, "r");

    /* Hack -- try the alternative directory */
    if (!fff) {

        /* Build the alternate file name */
        sprintf(path, "%s%s", ANGBAND_DIR_INFO, name);

        /* Open the file */
        fff = my_fopen(path, "r");
    }

    /* Oops */
    if (!fff) {

        /* Message */
        msg_format("Cannot open help file '%s'.", name);
        msg_print(NULL);

        /* Oops */
        return (TRUE);
    }


    /* Pre-Parse the file */
    while (TRUE) {

        /* Read a line from the file */
        if (!fgets(buf, 128, fff)) break;

        /* Hack -- verify the file */
        for (n = 0; buf[n]; n++) {

            /* Invalid character (stop reading) */
            if (!isprint(buf[n])) break;
        }

        /* Strip trailing spaces and stuff */
        for (n = strlen(buf) - 1; (n > 0) && (isspace(buf[n-1])); n--) ;

        /* Truncate the text */
        buf[n] = '\0';

        /* XXX Parse "menu" items */
        if (prefix(buf, "***** ")) {

            char b1 = '[', b2 = ']';

            /* Notice "menu" requests */
            if ((buf[6] == b1) && isdigit(buf[7]) &&
                (buf[8] == b2) && (buf[9] == ' ')) {

                /* This is a menu file */
                menu = TRUE;

                /* Extract the menu item */
                k = buf[7] - '0';

                /* Extract the menu item */
                strcpy(hook[k], buf + 10);
            }

            /* Skip this */
            continue;
        }

        /* Count the "real" lines */
        next++;
    }

    /* Save the number of "real" lines */
    size = next;



    /* Display the file */
    while (TRUE) {


        /* Clear the screen */
        clear_screen();


        /* Restart when necessary */
        if (line >= size) line = 0;


        /* Re-open the file if needed */
        if (next > line) {

            /* Close it */
            my_fclose(fff);

            /* Hack -- Re-Open the file */
            fff = my_fopen(path, "r");

            /* Oops */
            if (!fff) return (FALSE);

            /* File has been restarted */
            next = 0;
        }

        /* Skip lines if needed */
        for ( ; next < line; next++) {

            /* Skip a line */
            if (!fgets(buf, 128, fff)) break;
        }


        /* Dump the next 20 lines of the file */
        for (i = 0; i < 20; ) {

            /* Hack -- track the "first" line */
            if (!i) line = next;

            /* Get a line of the file */
            if (!fgets(buf, 128, fff)) break;

            /* Check the length of the line */
            n = strlen(buf) - 1;

            /* Never more than 80 characters */
            if (n > 80) n = 80;

            /* Terminate the line */
            buf[n] = '\0';

            /* Hack -- skip "special" lines */
            if (prefix(buf, "***** ")) continue;

            /* Count the "real" lines */
            next++;

            /* Hack -- keep searching */
            if (find && !i && !strstr(buf, find)) continue;

            /* Hack -- stop searching */
            find = NULL;

            /* Dump the lines */
            put_str(buf, i+2, 0);

            /* Count the printed lines */
            i++;
        }

        /* Hack -- failed search */
        if (find) {
            bell();
            line = back;
            find = NULL;
            continue;
        }


        /* Show a general "title" */
        prt(format("[Angband %d.%d.%d, Helpfile '%s', Line %d/%d]",
                   VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,
                   name, line, size), 0, 0);


        /* Prompt -- menu screen */
        if (menu) {

            /* Wait for it */
            prt("[Press a Number, or ESC to exit.]", 23, 0);
        }

        /* Prompt -- small files */
        else if (size <= 20) {

            /* Wait for it */
            prt("[Press ESC to exit.]", 23, 0);
        }

        /* Prompt -- large files */
        else {

            /* Wait for it */
            prt("[Press Return, Space, -, /, or ESC to exit.]", 23, 0);
        }

        /* Get a keypress */
        k = inkey();

        /* Hack -- return to last screen */
        if (k == '?') break;

        /* Hack -- try searching */
        if (k == '/') {
            prt("Find: ", 23, 0);
            if (askfor_aux(finder, 80)) {
                find = finder;
                back = line;
                line = line + 1;
            }
        }

        /* Hack -- go to a specific line */
        if (k == '#') {
            char tmp[80];
            prt("Goto Line: ", 23, 0);
            strcpy(tmp, "0");
            if (askfor_aux(tmp, 80)) {
                line = atoi(tmp);
            }
        }

        /* Hack -- go to a specific file */
        if (k == '%') {
            char tmp[80];
            prt("Goto File: ", 23, 0);
            strcpy(tmp, "help.hlp");
            if (askfor_aux(tmp, 80)) {
                if (!do_cmd_help_aux(tmp, 0)) k = ESCAPE;
            }
        }

        /* Hack -- Allow backing up */
        if (k == '-') {
            line = line - 10;
            if (line < 0) line = 0;
        }

        /* Hack -- Advance a single line */
        if ((k == '\n') || (k == '\r')) {
            line = line + 1;
        }

        /* Advance one page */
        if (k == ' ') {
            line = line + 20;
        }

        /* Recurse on numbers */
        if (menu && isdigit(k) && hook[k-'0'][0]) {

            /* Recurse on that file */
            if (!do_cmd_help_aux(hook[k-'0'], 0)) k = ESCAPE;
        }

        /* Exit on escape */
        if (k == ESCAPE) break;
    }

    /* Close the file */
    my_fclose(fff);

    /* Escape */
    if (k == ESCAPE) return (FALSE);

    /* Normal return */
    return (TRUE);
}


/*
 * Peruse the On-Line-Help, starting at the given file.
 */
void do_cmd_help(cptr name)
{
    /* Hack -- default file */
    if (!name) name = "help.hlp";

    /* Enter "icky" mode */
    character_icky = TRUE;

    /* Save the screen */
    Term_save();

    /* Peruse the main help file */
    (void)do_cmd_help_aux(name, 0);

    /* Restore the screen */
    Term_load();

    /* Leave "icky" mode */
    character_icky = FALSE;
}



/*
 * Determine a "title" for the player
 */
static cptr title_string()
{
    int plev = p_ptr->lev;

    /* King or Queen */
    if (total_winner || (plev > PY_MAX_LEVEL)) {
        return (p_ptr->male ? "**KING**" : "**QUEEN**");
    }

#ifdef ALLOW_TITLES

    /* Hack -- Novice */
    if (plev < 1) return ("Novice");

    /* Use the actual title */
    return (player_title[p_ptr->pclass][plev - 1]);

#else

    /* No title */
    return ("Player");

#endif

}



/*
 * Print the character to a file or device.
 */
errr file_character(cptr name)
{
    int			i;

    cptr		p;

    cptr		colon = ":";
    cptr		blank = " ";

#if 0
    cptr		other = "(";
#endif

    cptr		paren = ")";

    int			fd = -1;

    FILE		*fp = NULL;

    int			tmp;
    int                 xthn, xthb, xfos, xsrh;
    int			xdis, xdev, xsav, xstl;
    char                xinfra[32];

    int			show_to_h, show_to_d;

    inven_type		*i_ptr;

    store_type		*st_ptr = &store[7];

    char		i_name[80];

    char		buf[160];


    /* Drop priv's */
    safe_setuid_drop();

#if defined(MACINTOSH) && !defined(applec)
    /* Global -- "text file" */
    _ftype = 'TEXT';
#endif

    /* Check for existing file */
    fd = fd_open(name, O_RDONLY, 0);
    
    /* Existing file */
    if (fd >= 0) {

        char out_val[160];

        /* Close the file */
        (void)fd_close(fd);

        /* Build query */
        (void)sprintf(out_val, "Replace existing file %s? ", name);

        /* Ask */
        if (get_check(out_val)) fd = -1;
    }

    /* Open the non-existing file */
    if (fd < 0) fp = my_fopen(name, "w");

    /* Grab priv's */
    safe_setuid_grab();


    /* Invalid file */
    if (!fp) {

        /* Message */
        msg_format("Cannot open file '%s'.", name);
        msg_print(NULL);

        /* Error */
        return (-1);
    }


    /* Fighting Skill (with current weapon) */
    i_ptr = &inventory[INVEN_WIELD];
    tmp = p_ptr->to_h + i_ptr->to_h;
    xthn = p_ptr->skill_thn + (tmp * BTH_PLUS_ADJ);

    /* Shooting Skill (with current bow and normal missile) */
    i_ptr = &inventory[INVEN_BOW];
    tmp = p_ptr->to_h + i_ptr->to_h;
    xthb = p_ptr->skill_thb + (tmp * BTH_PLUS_ADJ);

    /* Basic abilities */
    xdis = p_ptr->skill_dis;
    xdev = p_ptr->skill_dev;
    xsav = p_ptr->skill_sav;
    xstl = p_ptr->skill_stl;
    xsrh = p_ptr->skill_srh;
    xfos = p_ptr->skill_fos;

    /* Infravision string */
    (void)sprintf(xinfra, "%d feet", p_ptr->see_infra * 10);

    /* Basic to hit/dam bonuses */
    show_to_h = p_ptr->dis_to_h;
    show_to_d = p_ptr->dis_to_d;

    /* Check the weapon */
    i_ptr = &inventory[INVEN_WIELD];

    /* Hack -- add in weapon info if known */
    if (inven_known_p(i_ptr)) show_to_h += i_ptr->to_h;
    if (inven_known_p(i_ptr)) show_to_d += i_ptr->to_d;


    /* Notify user */
    prt(format("Dumping character to '%s'...", name), 0, 0);

    /* Flush */
    Term_fresh();

    colon = ":";
    blank = " ";

    fprintf(fp, " Name%9s %-23s", colon, player_name);
    fprintf(fp, "Age%11s %6d ", colon, (int)p_ptr->age);
    cnv_stat(p_ptr->stat_use[A_STR], buf);
    fprintf(fp, "   STR : %s\n", buf);
    fprintf(fp, " Race%9s %-23s", colon, rp_ptr->title);
    fprintf(fp, "Height%8s %6d ", colon, (int)p_ptr->ht);
    cnv_stat(p_ptr->stat_use[A_INT], buf);
    fprintf(fp, "   INT : %s\n", buf);
    fprintf(fp, " Sex%10s %-23s", colon,
                  (p_ptr->male ? "Male" : "Female"));
    fprintf(fp, "Weight%8s %6d ", colon, (int)p_ptr->wt);
    cnv_stat(p_ptr->stat_use[A_WIS], buf);
    fprintf(fp, "   WIS : %s\n", buf);
    fprintf(fp, " Class%8s %-23s", colon,
                  cp_ptr->title);
    fprintf(fp, "Social Class : %6d ", p_ptr->sc);
    cnv_stat(p_ptr->stat_use[A_DEX], buf);
    fprintf(fp, "   DEX : %s\n", buf);
    fprintf(fp, " Title%8s %-23s", colon, title_string());
    fprintf(fp, "%22s", blank);
    cnv_stat(p_ptr->stat_use[A_CON], buf);
    fprintf(fp, "   CON : %s\n", buf);
    fprintf(fp, "%34s", blank);
    fprintf(fp, "%26s", blank);
    cnv_stat(p_ptr->stat_use[A_CHR], buf);
    fprintf(fp, "   CHR : %s\n\n", buf);

    fprintf(fp, " + To Hit    : %6d", show_to_h);
    fprintf(fp, "%7sLevel      :%9d", blank, (int)p_ptr->lev);
    fprintf(fp, "   Max Hit Points : %6d\n", p_ptr->mhp);
    fprintf(fp, " + To Damage : %6d", show_to_d);
    fprintf(fp, "%7sExperience :%9ld", blank, (long)p_ptr->exp);
    fprintf(fp, "   Cur Hit Points : %6d\n", p_ptr->chp);
    fprintf(fp, " + To AC     : %6d", p_ptr->dis_to_a);
    fprintf(fp, "%7sMax Exp    :%9ld", blank, (long)p_ptr->max_exp);
    fprintf(fp, "   Max Mana%8s %6d\n", colon, p_ptr->msp);
    fprintf(fp, "   Base AC   : %6d", p_ptr->dis_ac);

    if (p_ptr->lev >= PY_MAX_LEVEL) {
        fprintf(fp, "%7sExp to Adv.:%9s", blank, "****");
    }
    else {
        fprintf(fp, "%7sExp to Adv.:%9ld", blank,
                      (long)(player_exp[p_ptr->lev - 1] *
                             p_ptr->expfact / 100L));
    }

    fprintf(fp, "   Cur Mana%8s %6d\n", colon, p_ptr->csp);
    fprintf(fp, "%28sGold%8s%9ld\n", blank, colon, (long)p_ptr->au);


    /* Dump the misc. abilities */
    fprintf(fp, "\n(Miscellaneous Abilities)\n");
    fprintf(fp, " Fighting    : %-10s", likert(xthn, 12));
    fprintf(fp, "   Stealth     : %-10s", likert(xstl, 1));
    fprintf(fp, "   Perception  : %s\n", likert(xfos, 6));
    fprintf(fp, " Bows/Throw  : %-10s", likert(xthb, 12));
    fprintf(fp, "   Disarming   : %-10s", likert(xdis, 8));
    fprintf(fp, "   Searching   : %s\n", likert(xsrh, 6));
    fprintf(fp, " Saving Throw: %-10s", likert(xsav, 6));
    fprintf(fp, "   Magic Device: %-10s", likert(xdev, 6));
    fprintf(fp, "   Infra-Vision: %s\n\n", xinfra);

    /* Write out the character's history     */
    fprintf(fp, "Character Background\n");
    for (i = 0; i < 4; i++) {
        fprintf(fp, " %s\n", history[i]);
    }

    fprintf(fp, "\n\n");

    /* Dump the equipment */
    if (equip_cnt) {
        fprintf(fp, "  [Character's Equipment List]\n\n");
        for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
            p = mention_use(i);
            objdes(i_name, &inventory[i], TRUE, 3);
            fprintf(fp, "  %c%s %-19s: %s\n",
                    index_to_label(i), paren, p, i_name);
        }
        fprintf(fp, "\n\n");
    }


    /* Dump the inventory */
    if (inven_cnt) {
        fprintf(fp, "  [General Inventory List]\n\n");
        for (i = 0; i < INVEN_PACK; i++) {
            objdes(i_name, &inventory[i], TRUE, 3);
            fprintf(fp, "%c%s %s\n",
                    index_to_label(i), paren, i_name);
        }
        fprintf(fp, "\n\n");
    }


    /* Dump the Home */
    fprintf(fp, "  [Home Inventory]\n\n");	
    if (!st_ptr->stock_num) {
        fprintf(fp, "  Character has no objects at home.\n");
    }
    else {
        for (i = 0; i < st_ptr->stock_num; i++) {
            if (i == 12) fprintf(fp, "\n");
            objdes(i_name, &st_ptr->stock[i], TRUE, 3);
            fprintf(fp, "%c%s %s\n", (i%12)+'a', paren, i_name);
        }
    }

    /* Terminate it */
    fprintf(fp, "\n");

    /* Close it */
    my_fclose(fp);

    /* Message */
    prt(format("Dumping character to '%s'... done.", name), 0, 0);

    /* Dump */
    Term_fresh();

    /* Success */
    return (0);
}




/*
 * Process the player name.
 * Extract a clean "base name".
 * Build the savefile name if needed.
 */
void process_player_name(bool sf)
{
    int i, k = 0;


    /* Cannot be too long */
    if (strlen(player_name) > 15) {

        /* Name too long */
        quit_fmt("The name '%s' is too long!", player_name);
    }

    /* Cannot contain "icky" characters */
    for (i = 0; player_name[i]; i++) {

        /* No control characters */
        if (iscntrl(player_name[i])) {

            /* Illegal characters */
            quit_fmt("The name '%s' contains control characters!", player_name);
        }
    }


#ifdef MACINTOSH

    /* Extract "useful" letters */
    for (i = 0; player_name[i]; i++) {

        char c = player_name[i];

        /* Convert "dot" to "underscore" */
        if (c == '.') c = '_';

        /* Accept all the letters */
        player_base[k++] = c;
    }

#else

    /* Extract "useful" letters */
    for (i = 0; player_name[i]; i++) {

        char c = player_name[i];

        /* Accept some letters */
        if (isalpha(c) || isdigit(c)) player_base[k++] = c;

        /* Convert space, dot, and underscore to underscore */
        else if (strchr(". _", c)) player_base[k++] = '_';
    }

#endif


#if defined(WINDOWS) || defined(MSDOS)

    /* Hack -- max length */
    if (k > 8) k = 8;

#endif

    /* Terminate */
    player_base[k] = '\0';

    /* Require a "base" name */
    if (!player_base[0]) strcpy(player_base, "PLAYER");


#ifdef SAVEFILE_MUTABLE

    /* Accept */
    sf = TRUE;
    
#endif

    /* Change the savefile name */
    if (sf) {

#ifdef SAVEFILE_USE_UID
        /* Rename the savefile, using the player_uid and player_base */
        (void)sprintf(savefile, "%s%d.%s",
                        ANGBAND_DIR_SAVE, player_uid, player_base);
#else
        /* Rename the savefile, using the player_base */
        (void)sprintf(savefile, "%s%s",
                        ANGBAND_DIR_SAVE, player_base);
#endif

    }
}


/*
 * Gets a name for the character, reacting to name changes.
 *
 * Assumes that "display_player()" has just been called
 * XXX Perhaps we should NOT ask for a name (at "birth()") on Unix?
 */
void get_name()
{
    char tmp[32];

    /* Prompt and ask */
    prt("Enter your player's name above [press <RETURN> when finished]", 21, 2);

    /* Ask until happy */
    while (1) {

        /* Go to the "name" field */
        move_cursor(2, 15);

        /* Save the player name */
        strcpy(tmp, player_name);

        /* Get an input, ignore "Escape" */
        if (askfor_aux(tmp, 15)) strcpy(player_name, tmp);

        /* Process the player name */
        process_player_name(FALSE);

        /* All done */
        break;
    }

    /* Pad the name (to clear junk) */
    sprintf(tmp, "%-15.15s", player_name);

    /* Re-Draw the name (in light blue) */
    c_put_str(TERM_L_BLUE, tmp, 2, 15);

    /* Erase the prompt, etc */
    clear_from(20);
}




/*
 * Hack -- Calculates the total number of points earned		-JWT-	
 */
long total_points(void)
{
    return (p_ptr->max_exp + (100 * p_ptr->max_dlv));
}



/*
 * Centers a string within a 31 character string		-JWT-	
 */
static void center_string(char *buf, cptr str)
{
    int i, j;

    /* Total length */
    i = strlen(str);

    /* Necessary border */
    j = 15 - i / 2;

    /* Mega-Hack */
    (void)sprintf(buf, "%*s%s%*s", j, "", str, 31 - i - j, "");
}


/*
 * Save a "bones" file for a dead character
 *
 * Should probably attempt some form of locking...
 */
static void make_bones(void)
{
    FILE                *fp;

    char                str[1024];


    /* Ignore wizards and borgs */
    if (!(noscore & 0x00FF)) {

        /* Ignore people who die in town */
        if (dun_level) {

            /* XXX XXX Perhaps the player's level should be taken into account */

            /* Get the proper "Bones File" name */
            sprintf(str, "%sbone.%03d", ANGBAND_DIR_BONE, dun_level);

            /* Attempt to open the bones file */
            fp = my_fopen(str, "r");

            /* Close it right away */
            if (fp) my_fclose(fp);

            /* Do not over-write a previous ghost */
            if (fp) return;

#if defined(MACINTOSH) && !defined(applec)
            /* Global -- "text file" */
            _ftype = 'TEXT';
#endif

            /* Try to write a new "Bones File" */
            fp = my_fopen(str, "w");

            /* Not allowed to write it?  Weird. */
            if (!fp) return;

            /* Save the info */
            fprintf(fp, "%s\n", player_name);
            fprintf(fp, "%d\n", p_ptr->mhp);
            fprintf(fp, "%d\n", p_ptr->prace);
            fprintf(fp, "%d\n", p_ptr->pclass);

            /* Close and save the Bones file */
            my_fclose(fp);
        }
    }
}


/*
 * Silly string (unbalanced) representing the "grass"
 */
static cptr grass = "________)/\\\\_)_/___(\\/___(//_\\)/_\\//__\\\\(/_|_)_______";

/*
 * Prints the gravestone of the character  -RAK-
 */
static void print_tomb()
{
    int		i;
    cptr	p;

    char	day[32];

    char	out_val[160];
    char	tmp_val[160];

    time_t	ct = time((time_t)0);


    /* Draw a tombstone */
    clear_screen();

    put_str("_______________________", 1, 15);
    put_str("/", 2, 14);
    put_str("\\         ___", 2, 38);
    put_str("/", 3, 13);
    put_str("\\ ___   /   \\      ___", 3, 39);
    put_str("/            RIP            \\   \\  :   :     /   \\", 4, 12);
    put_str("/", 5, 11);
    put_str("\\  : _;,,,;_    :   :", 5, 41);

    center_string(tmp_val, player_name);
    (void)sprintf(out_val, "/%s\\,;_          _;,,,;_", tmp_val);

    put_str(out_val, 6, 10);

    put_str("|               the               |   ___", 7, 9);

    p = title_string();
    if (total_winner) p = "Magnificent";
    center_string(tmp_val, p);
    (void)sprintf(out_val, "| %s |  /   \\", tmp_val);
    put_str(out_val, 8, 9);

    put_str("|", 9, 9);
    put_str("|  :   :", 9, 43);

    if (!total_winner) {
        p = cp_ptr->title;
    }
    else if (p_ptr->male) {
        p = "*King*";
    }
    else {
        p = "*Queen*";
    }

    center_string(tmp_val, p);
    (void)sprintf(out_val, "| %s | _;,,,;_   ____", tmp_val);
    put_str(out_val, 10, 9);

    (void)sprintf(out_val, "Level : %d", (int)p_ptr->lev);
    center_string(tmp_val, out_val);
    (void)sprintf(out_val, "| %s |          /    \\", tmp_val);
    put_str(out_val, 11, 9);

    (void)sprintf(out_val, "%ld Exp", (long)p_ptr->exp);
    center_string(tmp_val, out_val);
    (void)sprintf(out_val, "| %s |          :    :", tmp_val);
    put_str(out_val, 12, 9);

    (void)sprintf(out_val, "%ld Au", (long)p_ptr->au);
    center_string(tmp_val, out_val);
    (void)sprintf(out_val, "| %s |          :    :", tmp_val);
    put_str(out_val, 13, 9);

    (void)sprintf(out_val, "Died on Level : %d", dun_level);
    center_string(tmp_val, out_val);
    (void)sprintf(out_val, "| %s |         _;,,,,;_", tmp_val);
    put_str(out_val, 14, 9);

    put_str("|            killed by            |", 15, 9);

    p = died_from;
    i = strlen(p);
    died_from[i] = '.';			   /* add a trailing period */
    died_from[i + 1] = '\0';
    center_string(tmp_val, p);
    (void)sprintf(out_val, "| %s |", tmp_val);
    put_str(out_val, 16, 9);

    died_from[i] = '\0';		   /* strip off the period */
    sprintf(day, "%-.24s", ctime(&ct));
    center_string(tmp_val, day);
    (void)sprintf(out_val, "| %s |", tmp_val);
    put_str(out_val, 17, 9);

    put_str("*|   *     *     *    *   *     *  | *", 18, 8);

    put_str(grass, 19, 0);
}


/*
 * Display some character info
 */
static void show_info(void)
{
    int			i, j, k;

    inven_type		*i_ptr;

    store_type		*st_ptr = &store[7];;


    /* Hack -- Know everything in the inven/equip */
    for (i = 0; i < INVEN_TOTAL; i++) {
        i_ptr = &inventory[i];
        if (i_ptr->k_idx) {
            inven_aware(i_ptr);
            inven_known(i_ptr);
        }
    }

    /* Hack -- Know everything in the home */
    for (i = 0; i < st_ptr->stock_num; i++) {
        i_ptr = &st_ptr->stock[i];
        if (i_ptr->k_idx) {
            inven_aware(i_ptr);
            inven_known(i_ptr);
        }
    }

    /* Hack -- Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Handle stuff */
    handle_stuff();

    /* Flush all input keys */
    flush();

    /* Flush messages */
    msg_print(NULL);


    /* Describe options */
    prt("You may now dump a character record to one or more files.", 21, 0);
    prt("Then, hit RETURN to see the character, or ESC to abort.", 22, 0);

    /* Dump character records as requested */
    while (TRUE) {

        char out_val[160];

        /* Get a filename (or escape) */
        put_str("Filename: ", 23, 0);
        if (!askfor(out_val, 60)) return;

        /* Return means "show on screen" */
        if (!out_val[0]) break;

        /* Dump a character file */
        (void)file_character(out_val);
    }


    /* Show player on screen */
    display_player(FALSE);

    /* Prompt for inventory */
    prt("Hit any key to see more information (ESC to abort): ", 23, 0);

    /* Allow abort at this point */
    if (inkey() == ESCAPE) return;


    /* Show equipment and inventory */

    /* Equipment -- if any */
    if (equip_cnt) {
        clear_screen();
        item_tester_full = TRUE;
        show_equip();
        msg_print("You are using:");
        msg_print(NULL);
    }

    /* Inventory -- if any */
    if (inven_cnt) {
        clear_screen();
        item_tester_full = TRUE;
        show_inven();
        msg_print("You are carrying:");
        msg_print(NULL);
    }



    /* Home -- if anything there */
    if (st_ptr->stock_num) {

        /* show home's inventory... */
        for (k = 0, i = 0; i < st_ptr->stock_num; k++) {

            /* Clear the screen */
            clear_screen();

            /* Show 12 items */
            for (j = 0; (j < 12) && (i < st_ptr->stock_num); j++, i++) {

                char i_name[80];
                char tmp_val[80];

                i_ptr = &st_ptr->stock[i];

                sprintf(tmp_val, "%c) ", 'a'+j);
                prt(tmp_val, j+2, 4);

                objdes(i_name, i_ptr, TRUE, 3);
                c_prt(tval_to_attr[i_ptr->tval], i_name, j+2, 7);
            }

            /* Caption */
            msg_format("You have stored at your house (page %d):", k+1);
            msg_print(NULL);
        }
    }
}





/*
 * Semi-Portable High Score List Entry (128 bytes) -- BEN
 *
 * All fields listed below are null terminated ascii strings.
 *
 * In addition, the "number" fields are right justified, and
 * space padded, to the full available length (minus the "null").
 *
 * Note that "string comparisons" are thus valid on "pts".
 */

typedef struct high_score high_score;

struct high_score {

  char what[8];		/* Version info (string) */

  char pts[10];		/* Total Score (number) */

  char gold[10];	/* Total Gold (number) */

  char turns[10];	/* Turns Taken (number) */

  char day[10];		/* Time stamp (string) */

  char who[16];		/* Player Name (string) */

  char uid[8];		/* Player UID (number) */

  char sex[2];		/* Player Sex (string) */
  char p_r[3];		/* Player Race (number) */
  char p_c[3];		/* Player Class (number) */

  char cur_lev[4];	/* Current Player Level (number) */
  char cur_dun[4];	/* Current Dungeon Level (number) */
  char max_lev[4];	/* Max Player Level (number) */
  char max_dun[4];	/* Max Dungeon Level (number) */

  char how[32];		/* Method of death (string) */
};



/*
 * The "highscore" file descriptor, if available.
 */
static int highscore_fd = -1;


/*
 * Seek score 'i' in the highscore file
 */
static int highscore_seek(int i)
{
    /* Seek for the requested record */
    return (fd_seek(highscore_fd, (huge)(i) * sizeof(high_score)));
}


/*
 * Read one score from the highscore file
 */
static errr highscore_read(high_score *score)
{
    /* Read the record, note failure */
    return (fd_read(highscore_fd, (char*)(score), sizeof(high_score)));
}


/*
 * Write one score to the highscore file
 */
static int highscore_write(high_score *score)
{
    /* Write the record, note failure */
    return (fd_write(highscore_fd, (char*)(score), sizeof(high_score)));
}




/*
 * Just determine where a new score *would* be placed
 * Return the location (0 is best) or -1 on failure
 */
static int highscore_where(high_score *score)
{
    int			i;

    high_score		the_score;

    /* Paranoia -- it may not have opened */
    if (highscore_fd < 0) return (-1);

    /* Go to the start of the highscore file */
    if (highscore_seek(0)) return (-1);

    /* Read until we get to a higher score */
    for (i = 0; i < MAX_HISCORES; i++) {
        if (highscore_read(&the_score)) return (i);
        if (strcmp(the_score.pts, score->pts) < 0) return (i);
    }

    /* The "last" entry is always usable */
    return (MAX_HISCORES - 1);
}


/*
 * Actually place an entry into the high score file
 * Return the location (0 is best) or -1 on "failure"
 */
static int highscore_add(high_score *score)
{
    int			i, slot;
    bool		done = FALSE;

    high_score		the_score, tmpscore;


    /* Paranoia -- it may not have opened */
    if (highscore_fd < 0) return (-1);

    /* Determine where the score should go */
    slot = highscore_where(score);

    /* Hack -- Not on the list */
    if (slot < 0) return (-1);

    /* Hack -- prepare to dump the new score */
    the_score = (*score);

    /* Slide all the scores down one */
    for (i = slot; !done && (i < MAX_HISCORES); i++) {

        /* Read the old guy, note errors */
        if (highscore_seek(i)) return (-1);
        if (highscore_read(&tmpscore)) done = TRUE;

        /* Back up and dump the score we were holding */
        if (highscore_seek(i)) return (-1);
        if (highscore_write(&the_score)) return (-1);

        /* Hack -- Save the old score, for the next pass */
        the_score = tmpscore;
    }

    /* Return location used */
    return (slot);
}



/*
 * Display the scores in a given range.
 * Assumes the high score list is already open.
 * Only five entries per line, too much info.
 *
 * Mega-Hack -- allow "fake" entry at the given position.
 */
static void display_scores_aux(int from, int to, int note, high_score *score)
{
    int		i, j, k, n, attr, place;

    high_score	the_score;

    char	out_val[256];
    char	tmp_val[160];


    /* Paranoia -- it may not have opened */
    if (highscore_fd < 0) return;


    /* Assume we will show the first 10 */
    if (from < 0) from = 0;
    if (to < 0) to = 10;
    if (to > MAX_HISCORES) to = MAX_HISCORES;


    /* Seek to the beginning */
    if (highscore_seek(0)) return;

    /* Hack -- Count the high scores */
    for (i = 0; i < MAX_HISCORES; i++) {
        if (highscore_read(&the_score)) break;
    }

    /* Hack -- allow "fake" entry to be last */
    if ((note == i) && score) i++;

    /* Forget about the last entries */
    if (i > to) i = to;


    /* Show 5 per page, until "done" */
    for (k = from, place = k+1; k < i; k += 5) {

        /* Clear those */
        clear_screen();

        put_str("                Angband Hall of Fame", 0, 0);

        /* Indicate non-top scores */
        if (k > 0) {
            sprintf(tmp_val, "(from position %d)", k + 1);
            put_str(tmp_val, 0, 40);
        }

        /* Dump 5 entries */
        for (j = k, n = 0; j < i && n < 5; place++, j++, n++) {

            int pr, pc, clev, mlev, cdun, mdun;

            cptr user, gold, when, aged;


            /* Hack -- indicate death in yellow */
            attr = (j == note) ? TERM_YELLOW : TERM_WHITE;


            /* Mega-Hack -- insert a "fake" record */
            if ((note == j) && score) {
                the_score = (*score);
                attr = TERM_L_GREEN;
                score = NULL;
                note = -1;
                j--;
            }

            /* Read a normal record */
            else {
                /* Read the proper record */
                if (highscore_seek(j)) break;
                if (highscore_read(&the_score)) break;
            }

            /* Extract the race/class */
            pr = atoi(the_score.p_r);
            pc = atoi(the_score.p_c);

            /* Extract the level info */
            clev = atoi(the_score.cur_lev);
            mlev = atoi(the_score.max_lev);
            cdun = atoi(the_score.cur_dun);
            mdun = atoi(the_score.max_dun);

            /* Hack -- extract the gold and such */
            for (user = the_score.uid; isspace(*user); user++) ;
            for (when = the_score.day; isspace(*when); when++) ;
            for (gold = the_score.gold; isspace(*gold); gold++) ;
            for (aged = the_score.turns; isspace(*aged); aged++) ;

            /* Dump some info */
            sprintf(out_val, "%3d.%9s  %s the %s %s, Level %d",
                    place, the_score.pts, the_score.who,
                    race_info[pr].title, class_info[pc].title,
                    clev);

            /* Append a "maximum level" */
            if (mlev > clev) strcat(out_val, format(" (Max %d)", mlev));

            /* Dump the first line */
            c_put_str(attr, out_val, n*4 + 2, 0);

            /* Another line of info */
            sprintf(out_val, "               Killed by %s on %s %d",
                    the_score.how, "Dungeon Level", cdun);

            /* Hack -- some people die in the town */
            if (!cdun) {
                sprintf(out_val, "               Killed by %s in the Town",
                        the_score.how);
            }

            /* Append a "maximum level" */
            if (mdun > cdun) strcat(out_val, format(" (Max %d)", mdun));

            /* Dump the info */
            c_put_str(attr, out_val, n*4 + 3, 0);

            /* And still another line of info */
            sprintf(out_val, "               (User %s, Date %s, Gold %s, Turn %s).",
                    user, when, gold, aged);
            c_put_str(attr, out_val, n*4 + 4, 0);
        }


        /* Wait for response. */
        prt("[Press ESC to quit, any other key to continue.]", 23, 17);
        j = inkey();
        prt("", 23, 0);

        /* Hack -- notice Escape */
        if (j == ESCAPE) break;
    }
}


/*
 * Hack -- Display the scores in a given range and quit.
 *
 * This function is only called from "main.c" when the user asks
 * to see the "high scores".
 */
void display_scores(int from, int to)
{
    char buf[1024];

    /* Extract the name of the High Score File */
    sprintf(buf, "%s%s", ANGBAND_DIR_APEX, "scores.raw");

    /* Open the binary high score file, for reading */
    highscore_fd = fd_open(buf, O_RDONLY | O_BINARY, 0);

    /* Paranoia -- No score file */
    if (highscore_fd < 0) quit("Score file unavailable.");

    /* Clear screen */
    clear_screen();

    /* Display the scores */
    display_scores_aux(from, to, -1, NULL);

    /* Shut the high score file */
    (void)fd_close(highscore_fd);

    /* Forget the high score fd */
    highscore_fd = -1;

    /* Quit */
    quit(NULL);
}



/*
 * Enters a players name on a hi-score table, if "legal", and in any
 * case, displays some relevant portion of the high score list.
 *
 * Assumes "signals_ignore_tstp()" has been called.
 */
static errr top_twenty(void)
{
    int          j;

    high_score   the_score;

    time_t ct = time((time_t*)0);


    /* Wipe screen */
    clear_screen();

    /* No score file */
    if (highscore_fd < 0) {
        msg_print("Score file unavailable.");
        msg_print(NULL);
        return (0);
    }

#ifndef SCORE_WIZARDS
    /* Wizard-mode pre-empts scoring */
    if (noscore & 0x000F) {
        msg_print("Score not registered for wizards.");
        display_scores_aux(0, 10, -1, NULL);
        return (0);
    }
#endif

#ifndef SCORE_BORGS
    /* Borg-mode pre-empts scoring */
    if (noscore & 0x00F0) {
        msg_print("Score not registered for borgs.");
        display_scores_aux(0, 10, -1, NULL);
        return (0);
    }
#endif

#ifndef SCORE_CHEATERS
    /* Cheaters are not scored */
    if (noscore & 0xFF00) {
        msg_print("Score not registered for cheaters.");
        display_scores_aux(0, 10, -1, NULL);
        return (0);
    }
#endif

    /* Interupted */
    if (!total_winner && streq(died_from, "Interrupting")) {
        msg_print("Score not registered due to interruption.");
        display_scores_aux(0, 10, -1, NULL);
        return (0);
    }

    /* Quitter */
    if (!total_winner && streq(died_from, "Quitting")) {
        msg_print("Score not registered due to quitting.");
        display_scores_aux(0, 10, -1, NULL);
        return (0);
    }


    /* Clear the record */
    WIPE(&the_score, high_score);

    /* Save the version */
    sprintf(the_score.what, "%u.%u.%u",
            VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

    /* Calculate and save the points */
    sprintf(the_score.pts, "%9lu", (long)total_points());
    the_score.pts[9] = '\0';

    /* Save the current gold */
    sprintf(the_score.gold, "%9lu", (long)p_ptr->au);
    the_score.gold[9] = '\0';

    /* Save the current turn */
    sprintf(the_score.turns, "%9lu", (long)turn);
    the_score.turns[9] = '\0';

#ifdef HIGHSCORE_DATE_HACK
    /* Save the date in a hacked up form (9 chars) */
    sprintf(the_score.day, "%-.6s %-.2s", ctime(&ct) + 4, ctime(&ct) + 22);
#else
    /* Save the date in standard form (8 chars) */
    strftime(the_score.day, 9, "%m/%d/%y", localtime(&ct));
#endif

    /* Save the player name (15 chars) */
    sprintf(the_score.who, "%-.15s", player_name);

    /* Save the player info */
    sprintf(the_score.uid, "%7u", player_uid);
    sprintf(the_score.sex, "%c", (p_ptr->male ? 'm' : 'f'));
    sprintf(the_score.p_r, "%2d", p_ptr->prace);
    sprintf(the_score.p_c, "%2d", p_ptr->pclass);

    /* Save the level and such */
    sprintf(the_score.cur_lev, "%3d", p_ptr->lev);
    sprintf(the_score.cur_dun, "%3d", dun_level);
    sprintf(the_score.max_lev, "%3d", p_ptr->max_plv);
    sprintf(the_score.max_dun, "%3d", p_ptr->max_dlv);

    /* Save the cause of death (31 chars) */
    sprintf(the_score.how, "%-.31s", died_from);


    /* Lock (for writing) the highscore file, or fail */
    if (fd_lock(highscore_fd, F_WRLCK)) return (1);

    /* Add a new entry to the score list, see where it went */
    j = highscore_add(&the_score);

    /* Unlock the highscore file, or fail */
    if (fd_lock(highscore_fd, F_UNLCK)) return (1);


    /* Hack -- Display the top fifteen scores */
    if (j < 10) {
        display_scores_aux(0, 15, j, NULL);
    }

    /* Display the scores surrounding the player */
    else {
        display_scores_aux(0, 5, j, NULL);
        display_scores_aux(j - 2, j + 7, j, NULL);
    }


    /* Success */
    return (0);
}


/*
 * Predict the players location, and display it.
 */
static errr predict_score(void)
{
    int          j;

    high_score   the_score;


    /* No score file */
    if (highscore_fd < 0) {
        msg_print("Score file unavailable.");
        msg_print(NULL);
        return (0);
    }


    /* Save the version */
    sprintf(the_score.what, "%u.%u.%u",
            VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

    /* Calculate and save the points */
    sprintf(the_score.pts, "%9lu", (long)total_points());

    /* Save the current gold */
    sprintf(the_score.gold, "%9lu", (long)p_ptr->au);

    /* Save the current turn */
    sprintf(the_score.turns, "%9lu", (long)turn);

    /* Hack -- no time needed */
    strcpy(the_score.day, "TODAY");

    /* Save the player name (15 chars) */
    sprintf(the_score.who, "%-.15s", player_name);

    /* Save the player info */
    sprintf(the_score.uid, "%7u", player_uid);
    sprintf(the_score.sex, "%c", (p_ptr->male ? 'm' : 'f'));
    sprintf(the_score.p_r, "%2d", p_ptr->prace);
    sprintf(the_score.p_c, "%2d", p_ptr->pclass);

    /* Save the level and such */
    sprintf(the_score.cur_lev, "%3d", p_ptr->lev);
    sprintf(the_score.cur_dun, "%3d", dun_level);
    sprintf(the_score.max_lev, "%3d", p_ptr->max_plv);
    sprintf(the_score.max_dun, "%3d", p_ptr->max_dlv);

    /* Hack -- no cause of death */
    strcpy(the_score.how, "nobody (yet!)");


    /* See where the entry would be placed */
    j = highscore_where(&the_score);


    /* Hack -- Display the top fifteen scores */
    if (j < 10) {
        display_scores_aux(0, 15, j, &the_score);
    }

    /* Display some "useful" scores */
    else {
        display_scores_aux(0, 5, -1, NULL);
        display_scores_aux(j - 2, j + 7, j, &the_score);
    }


    /* Success */
    return (0);
}








/*
 * Change the player into a King!			-RAK-	
 */
static void kingly()
{
    /* Hack -- retire in town */
    dun_level = 0;

    /* Fake death */
    (void)strcpy(died_from, "Ripe Old Age");

    /* Restore the level */
    (void)restore_level();

    /* Mega-Hack -- Instant Experience */
    p_ptr->exp += 5000000L;
    p_ptr->max_exp += 5000000L;

    /* Mega-Hack -- Instant Gold */
    p_ptr->au += 250000L;

    /* Display a crown */
    clear_screen();
    put_str("#", 1, 34);
    put_str("#####", 2, 32);
    put_str("#", 3, 34);
    put_str(",,,  $$$  ,,,", 4, 28);
    put_str(",,=$   \"$$$$$\"   $=,,", 5, 24);
    put_str(",$$        $$$        $$,", 6, 22);
    put_str("*>         <*>         <*", 7, 22);
    put_str("$$         $$$         $$", 8, 22);
    put_str("\"$$        $$$        $$\"", 9, 22);
    put_str("\"$$       $$$       $$\"", 10, 23);
    put_str("*#########*#########*", 11, 24);
    put_str("*#########*#########*", 12, 24);

    /* Display a message */
    put_str("Veni, Vidi, Vici!", 15, 26);
    put_str("I came, I saw, I conquered!", 16, 21);
    if (p_ptr->male) {
        put_str("All Hail the Mighty King!", 17, 22);
    }
    else {
        put_str("All Hail the Mighty Queen!", 17, 22);
    }

    /* Flush input */
    flush();

    /* Wait for response */
    pause_line(23);
}


/*
 * Close up the current game (player may or may not be dead)
 *
 * This function is called only from "main.c" and "signals.c".
 */
void close_game(void)
{
    char buf[1024];


    /* Handle stuff */
    handle_stuff();

    /* Flush the messages */
    msg_print(NULL);

    /* Flush the input */
    flush();


    /* No suspending now */
    signals_ignore_tstp();


    /* Hack -- Character is now "icky" */
    character_icky = TRUE;


    /* Extract the name of the High Score File */
    sprintf(buf, "%s%s", ANGBAND_DIR_APEX, "scores.raw");

    /* Open the binary high score file, for reading/writing */
    highscore_fd = fd_open(buf, O_RDWR | O_BINARY, 0);


    /* Handle death */
    if (death) {

        /* Handle retirement */
        if (total_winner) kingly();

        /* Save memories */
        if (!save_player()) msg_print("death save failed!");
        
        /* Dump bones file */
        make_bones();

        /* You are dead */
        print_tomb();

        /* Show more info */
        show_info();

        /* Handle score, show Top scores */
        top_twenty();
    }

    /* Still alive */
    else {

        /* Save the game */
        do_cmd_save_game();

        /* Prompt for score prediction */
        prt("Press Return (or Escape).", 0, 40);

        /* Predict score (or ESCAPE) */
        if (inkey() != ESCAPE) predict_score();	
    }


    /* Shut the high score file */
    (void)fd_close(highscore_fd);

    /* Forget the high score fd */
    highscore_fd = -1;


    /* Allow suspending now */
    signals_handle_tstp();
}


/*
 * Handle abrupt death of the visual system
 *
 * This routine is called only in very rare situations, and only
 * by certain visual systems, when they experience fatal errors.
 *
 * XXX XXX Hack -- clear the death flag when creating a HANGUP
 * save file so that player can see tombstone when restart.
 */
void exit_game_panic(void)
{
    /* If nothing important has happened, just quit */
    if (!character_generated || character_saved) quit("panic");

    /* Mega-Hack -- see "msg_print()" */
    msg_flag = FALSE;

    /* Hack -- turn off some things */
    disturb(1, 0);

    /* Mega-Hack -- Delay death */
    if (p_ptr->chp < 0) death = FALSE;

    /* Hardcode panic save */
    panic_save = 1;

    /* Indicate panic save */
    (void)strcpy(died_from, "(panic save)");

    /* Panic save, or get worried */
    if (!save_player()) quit("panic save failed!");

    /* Successful panic save */
    quit("panic save succeeded!");
}


