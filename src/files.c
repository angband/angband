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
 * Extract the first few "tokens" from a buffer
 *
 * This function uses "colon" and "slash" as the delimeter characters.
 *
 * We never extract more than "num" tokens.  The "last" token may include
 * "delimeter" characters, allowing the buffer to include a "string" token.
 *
 * We save pointers to the tokens in "tokens", and return the number found.
 *
 * Hack -- Attempt to handle the 'c' character formalism
 *
 * Hack -- An empty buffer, or a final delimeter, yields an "empty" token.
 *
 * Hack -- We will always extract at least one token
 */
s16b tokenize(char *buf, s16b num, char **tokens)
{
    int i = 0;

    char *s = buf;


    /* Process */
    while (i < num - 1) {

        char *t;
        
        /* Scan the string */
        for (t = s; *t; t++) {

            /* Found a delimiter */
            if ((*t == ':') || (*t == '/')) break;

            /* Handle single quotes */
            if (*t == '\'') {

                /* Advance */
                t++;

                /* Handle backslash */
                if (*t == '\\') t++;

                /* Require a character */
                if (!*t) break;

                /* Advance */
                t++;

                /* Hack -- Require a close quote */
                if (*t != '\'') *t = '\'';
            }

            /* Handle back-slash */
            if (*t == '\\') t++;
        }

        /* Nothing left */
        if (!*t) break;

        /* Nuke and advance */
        *t++ = '\0';

        /* Save the token */
        tokens[i++] = s;

        /* Advance */
        s = t;
    }

    /* Save the token */
    tokens[i++] = s;

    /* Number found */
    return (i);
}



/*
 * Parse a sub-file of the "extra info" (format shown below)
 *
 * Each "action" line has an "action symbol" in the first column,
 * followed by a colon, followed by some command specific info,
 * usually in the form of "tokens" separated by colons or slashes.
 *
 * Blank lines, lines starting with white space, and lines starting
 * with pound signs ("#") are ignored (as comments).
 *
 * Note the use of "tokenize()" to allow the use of both colons and
 * slashes as delimeters, while still allowing final tokens which
 * may contain any characters including "delimiters".
 *
 * Note the use of "strtol()" to allow all "integers" to be encoded
 * in decimal, hexidecimal, or octal form.
 *
 * Note that "monster zero" is used for the "player" attr/char, "object
 * zero" will be used for the "stack" attr/char, and "feature zero" is
 * used for the "nothing" attr/char.
 *
 * Parse another file recursively, see below for details
 *   %:<filename>
 *
 * Specify the attr/char values for "monsters" by race index
 *   R:<num>:<a>:<c>
 *
 * Specify the attr/char values for "objects" by kind index
 *   K:<num>:<a>:<c>
 *
 * Specify the attr/char values for "features" by feat index
 *   F:<num>:<a>:<c>
 *
 * Specify the attr/char values for unaware "objects" by kind tval
 *   U:<tv>:<a>:<c>
 *
 * Specify the attr/char values for inventory "objects" by kind tval
 *   E:<tv>:<a>:<c>
 *
 * Define a macro action, given an encoded macro action
 *   A:<str>
 *
 * Create a normal macro, given an encoded macro trigger
 *   P:<str>
 *
 * Create a command macro, given an encoded macro trigger
 *   C:<str>
 *
 * Create a keyset mapping
 *   S:<key>:<key>:<dir>
 *
 * Turn an option off, given its name
 *   X:<str>
 *
 * Turn an option on, given its name
 *   Y:<str>
 *
 * Specify visual information, given an index, and some data
 *   V:<num>:<kv>:<rv>:<gv>:<bv>
 */
errr process_pref_file_aux(char *buf)
{
    int i, j, k, n1, n2;

    char *zz[16];


    /* Skip "empty" lines */
    if (!buf[0]) return (0);

    /* Skip "blank" lines */
    if (isspace(buf[0])) return (0);

    /* Skip comments */
    if (buf[0] == '#') return (0);


    /* Require "?:*" format */
    if (buf[1] != ':') return (1);


    /* Process "%:<fname>" */
    if (buf[0] == '%') {

        /* Attempt to Process the given file */
        return (process_pref_file(buf + 2));
    }


    /* Process "R:<num>:<a>/<c>" -- attr/char for monster races */
    if (buf[0] == 'R') {
        if (tokenize(buf+2, 3, zz) == 3) {
            monster_race *r_ptr;
            i = (huge)strtol(zz[0], NULL, 0);
            n1 = strtol(zz[1], NULL, 0);
            n2 = strtol(zz[2], NULL, 0);
            if (i >= MAX_R_IDX) return (1);
            r_ptr = &r_info[i];
            if (n1) r_ptr->l_attr = n1;
            if (n2) r_ptr->l_char = n2;
            return (0);
        }
    }


    /* Process "K:<num>:<a>/<c>"  -- attr/char for object kinds */
    else if (buf[0] == 'K') {
        if (tokenize(buf+2, 3, zz) == 3) {
            inven_kind *k_ptr;
            i = (huge)strtol(zz[0], NULL, 0);
            n1 = strtol(zz[1], NULL, 0);
            n2 = strtol(zz[2], NULL, 0);
            if (i >= MAX_K_IDX) return (1);
            k_ptr = &k_info[i];
            if (n1) k_ptr->x_attr = n1;
            if (n2) k_ptr->x_char = n2;
            return (0);
        }
    }


    /* Process "F:<num>:<a>/<c>" -- attr/char for terrain features */
    else if (buf[0] == 'F') {
        if (tokenize(buf+2, 3, zz) == 3) {
            feature_type *f_ptr;
            i = (huge)strtol(zz[0], NULL, 0);
            n1 = strtol(zz[1], NULL, 0);
            n2 = strtol(zz[2], NULL, 0);
            if (i >= MAX_F_IDX) return (1);
            f_ptr = &f_info[i];
            if (n1) f_ptr->z_attr = n1;
            if (n2) f_ptr->z_char = n2;
            return (0);
        }
    }


    /* Process "U:<tv>:<a>/<c>" -- attr/char for unaware items */
    else if (buf[0] == 'U') {
        if (tokenize(buf+2, 3, zz) == 3) {
            j = (huge)strtol(zz[0], NULL, 0);
            n1 = strtol(zz[1], NULL, 0);
            n2 = strtol(zz[2], NULL, 0);
            for (i = 1; i < MAX_K_IDX; i++) {
                inven_kind *k_ptr = &k_info[i];
                if (k_ptr->tval == j) {
                    if (n1) k_ptr->i_attr = n1;
                    if (n2) k_ptr->i_char = n2;
                }
            }
            return (0);
        }
    }


    /* Process "E:<tv>:<a>/<c>" -- attr/char for equippy chars */
    else if (buf[0] == 'E') {
        if (tokenize(buf+2, 3, zz) == 3) {
            j = (huge)strtol(zz[0], NULL, 0);
            n1 = strtol(zz[1], NULL, 0);
            n2 = strtol(zz[2], NULL, 0);
            if (j >= 128) return (1);
            if (n1) tval_to_attr[j] = n1;
            if (n2) tval_to_char[j] = n2;
            return (0);
        }
    }


    /* Process "A:<str>" -- save an "action" for later */
    else if (buf[0] == 'A') {
        text_to_ascii(macro__buf, buf+2);
        return (0);
    }

    /* Process "P:<str>" -- create normal macro */
    else if (buf[0] == 'P') {
        char tmp[1024];
        text_to_ascii(tmp, buf+2);
        macro_add(tmp, macro__buf, FALSE);
        return (0);
    }

    /* Process "C:<str>" -- create command macro */
    else if (buf[0] == 'C') {
        char tmp[1024];
        text_to_ascii(tmp, buf+2);
        macro_add(tmp, macro__buf, TRUE);
        return (0);
    }


    /* Process "S:<key>:<key>:<dir>" -- keymap */
    else if (buf[0] == 'S') {
        if (tokenize(buf+2, 3, zz) == 3) {
            i = (byte)strtol(zz[0], NULL, 0);
            j = (byte)strtol(zz[0], NULL, 0);
            k = (byte)strtol(zz[0], NULL, 0);
            if ((k > 9) || (k == 5)) k = 0;
            keymap_cmds[i] = j;
            keymap_dirs[i] = k;
            return (0);
        }
    }


    /* Process "V:<num>:<kv>:<rv>:<gv>:<bv>" -- visual info */
    else if (buf[0] == 'V') {
        if (tokenize(buf+2, 5, zz) == 5) {
            i = (byte)strtol(zz[0], NULL, 0);
            color_table[i][0] = (byte)strtol(zz[1], NULL, 0);
            color_table[i][1] = (byte)strtol(zz[2], NULL, 0);
            color_table[i][2] = (byte)strtol(zz[3], NULL, 0);
            color_table[i][3] = (byte)strtol(zz[4], NULL, 0);
            return (0);
        }
    }


    /* Process "X:<str>" -- turn option off */
    else if (buf[0] == 'X') {
        for (i = 0; options[i].o_desc; i++) {
            if (options[i].o_var &&
                options[i].o_text &&
                (options[i].o_page <= 8) &&
                streq(options[i].o_text, buf + 2)) {
                (*options[i].o_var) = FALSE;
                return (0);
            }
        }
    }

    /* Process "Y:<str>" -- turn option on */
    else if (buf[0] == 'Y') {
        for (i = 0; options[i].o_desc; i++) {
            if (options[i].o_var &&
                options[i].o_text &&
                (options[i].o_page <= 8) &&
                streq(options[i].o_text, buf + 2)) {
                (*options[i].o_var) = TRUE;
                return (0);
            }
        }
    }


    /* Failure */
    return (1);
}


/*
 * Process the "user pref file" with the given name
 *
 * See the function above for a list of legal "commands".
 */
errr process_pref_file(cptr name)
{
    FILE *fp;

    char buf[1024];


    /* Access the file */
    strcpy(buf, ANGBAND_DIR_USER);
    strcat(buf, name);

    /* Open the file */
    fp = my_fopen(buf, "r");

    /* Catch errors */
    if (!fp) return (-1);

    /* Process the file */
    while (0 == my_fgets(fp, buf, 1024)) {

        /* Process the line */
        if (process_pref_file_aux(buf)) {

            /* Useful error message */
            msg_format("Error in '%s' parsing '%s'.", buf, name);
        }
    }

    /* Close the file */
    my_fclose(fp);

    /* Success */
    return (0);
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


    /* Access the "time" file */
    strcpy(buf, ANGBAND_DIR_FILE);
    strcat(buf, "time.txt");

    /* Open the file */
    fp = my_fopen(buf, "r");

    /* No file, no restrictions */
    if (!fp) return (0);

    /* Assume restrictions */
    check_time_flag = TRUE;

    /* Parse the file */
    while (0 == my_fgets(fp, buf, 80)) {

        /* Skip comments and blank lines */
        if (!buf[0] || (buf[0] == '#')) continue;

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
    strcpy(buf, ANGBAND_DIR_FILE);
    strcat(buf, "load.txt");

    /* Open the "load" file */
    fp = my_fopen(buf, "r");

    /* No file, no restrictions */
    if (!fp) return (0);

    /* Default load */
    check_load_value = 100;

    /* Get the host name */
    (void)gethostname(thishost, (sizeof thishost) - 1);

    /* Parse it */
    while (0 == my_fgets(fp, buf, 1024)) {

        int value;

        /* Skip comments and blank lines */
        if (!buf[0] || (buf[0] == '#')) continue;

        /* Parse, or ignore */
        if (sscanf(buf, "%s%d", temphost, &value) != 2) continue;

        /* Skip other hosts */
        if (!streq(temphost,thishost) &&
            !streq(temphost,"localhost")) continue;

        /* Use that value */
        check_load_value = value;

        /* Done */
        break;
    }

    /* Close the file */
    my_fclose(fp);

#endif

    /* Success */
    return (0);
}




/*
 * Print long number with header at given row, column
 * Use the color for the number, not the header
 */
static void prt_lnum(cptr header, s32b num, int row, int col, byte color)
{
    int len = strlen(header);
    char out_val[32];
    put_str(header, row, col);
    (void)sprintf(out_val, "%9ld", (long)num);
    c_put_str(color, out_val, row, col + len);
}

/*
 * Print number with header at given row, column
 */
static void prt_num(cptr header, int num, int row, int col, byte color)
{
    int len = strlen(header);
    char out_val[32];
    put_str(header, row, col);
    put_str("   ", row, col + len);
    (void)sprintf(out_val, "%6ld", (long)num);
    c_put_str(color, out_val, row, col + len + 3);
}



/*
 * Prints the following information on the screen.
 *
 * For this to look right, the following should be spaced the
 * same as in the prt_lnum code... -CFT
 */
static void display_player_middle(void)
{
    int show_tohit = p_ptr->dis_to_h;
    int show_todam = p_ptr->dis_to_d;

    inven_type *i_ptr = &inventory[INVEN_WIELD];

    /* Hack -- add in weapon info if known */
    if (inven_known_p(i_ptr)) show_tohit += i_ptr->to_h;
    if (inven_known_p(i_ptr)) show_todam += i_ptr->to_d;

    /* Dump the bonuses to hit/dam */
    prt_num("+ To Hit    ", show_tohit, 9, 1, TERM_L_BLUE);
    prt_num("+ To Damage ", show_todam, 10, 1, TERM_L_BLUE);

    /* Dump the armor class bonus */
    prt_num("+ To AC     ", p_ptr->dis_to_a, 11, 1, TERM_L_BLUE);

    /* Dump the total armor class */
    prt_num("  Base AC   ", p_ptr->dis_ac, 12, 1, TERM_L_BLUE);

    prt_num("Level      ", (int)p_ptr->lev, 9, 28, TERM_L_GREEN);

    if (p_ptr->exp >= p_ptr->max_exp) {
        prt_lnum("Experience ", p_ptr->exp, 10, 28, TERM_L_GREEN);
    }
    else {
        prt_lnum("Experience ", p_ptr->exp, 10, 28, TERM_YELLOW);
    }

    prt_lnum("Max Exp    ", p_ptr->max_exp, 11, 28, TERM_L_GREEN);

    if (p_ptr->lev >= PY_MAX_LEVEL) {
        put_str("Exp to Adv.", 12, 28);
        c_put_str(TERM_L_GREEN, "    *****", 12, 28+11);
    }
    else {
        prt_lnum("Exp to Adv.",
                 (s32b)(player_exp[p_ptr->lev - 1] * p_ptr->expfact / 100L),
                 12, 28, TERM_L_GREEN);
    }

    prt_lnum("Gold       ", p_ptr->au, 13, 28, TERM_L_GREEN);

    prt_num("Max Hit Points ", p_ptr->mhp, 9, 52, TERM_L_GREEN);

    if (p_ptr->chp >= p_ptr->mhp) {
        prt_num("Cur Hit Points ", p_ptr->chp, 10, 52, TERM_L_GREEN);
    }
    else if (p_ptr->chp > (p_ptr->mhp * hitpoint_warn) / 10) {
        prt_num("Cur Hit Points ", p_ptr->chp, 10, 52, TERM_YELLOW);
    }
    else {
        prt_num("Cur Hit Points ", p_ptr->chp, 10, 52, TERM_RED);
    }

    prt_num("Max SP (Mana)  ", p_ptr->msp, 11, 52, TERM_L_GREEN);

    if (p_ptr->csp >= p_ptr->msp) {
        prt_num("Cur SP (Mana)  ", p_ptr->csp, 12, 52, TERM_L_GREEN);
    }
    else if (p_ptr->csp > (p_ptr->msp * hitpoint_warn) / 10) {
        prt_num("Cur SP (Mana)  ", p_ptr->csp, 12, 52, TERM_YELLOW);
    }
    else {
        prt_num("Cur SP (Mana)  ", p_ptr->csp, 12, 52, TERM_RED);
    }
}




/*
 * Hack -- pass color info around this file
 */
static byte likert_color = TERM_WHITE;


/*
 * Returns a "rating" of x depending on y
 */
static cptr likert(int x, int y)
{
    /* Paranoia */
    if (y <= 0) y = 1;

    /* Negative value */
    if (x < 0) {
        likert_color = TERM_RED;
        return ("Very Bad");
    }

    /* Analyze the value */
    switch ((x / y)) {
      case 0:
      case 1:
        likert_color = TERM_RED;
        return ("Bad");
      case 2:
        likert_color = TERM_RED;
        return ("Poor");
      case 3:
      case 4:
        likert_color = TERM_YELLOW;
        return ("Fair");
      case 5:
        likert_color = TERM_YELLOW;
        return ("Good");
      case 6:
        likert_color = TERM_YELLOW;
        return ("Very Good");
      case 7:
      case 8:
        likert_color = TERM_L_GREEN;
        return ("Excellent");
      case 9:
      case 10:
      case 11:
      case 12:
      case 13:
        likert_color = TERM_L_GREEN;
        return ("Superb");
      case 14:
      case 15:
      case 16:
      case 17:
        likert_color = TERM_L_GREEN;
        return ("Heroic");
      default:
        likert_color = TERM_L_GREEN;
        return ("Legendary");
    }
}


/*
 * Prints ratings on certain abilities
 *
 * This code is "imitated" elsewhere to "dump" a character sheet.
 */
static void display_player_various(void)
{
    int			tmp;
    int			xthn, xthb, xfos, xsrh;
    int			xdis, xdev, xsav, xstl;
    cptr		desc;

    inven_type		*i_ptr;
    

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


    put_str("Fighting    :", 16, 1);
    desc = likert(xthn, 12);
    c_put_str(likert_color, desc, 16, 15);

    put_str("Bows/Throw  :", 17, 1);
    desc = likert(xthb, 12);
    c_put_str(likert_color, desc, 17, 15);

    put_str("Saving Throw:", 18, 1);
    desc = likert(xsav, 6);
    c_put_str(likert_color, desc, 18, 15);

    put_str("Stealth     :", 19, 1);
    desc = likert(xstl, 1);
    c_put_str(likert_color, desc, 19, 15);


    put_str("Perception  :", 16, 28);
    desc = likert(xfos, 6);
    c_put_str(likert_color, desc, 16, 42);

    put_str("Searching   :", 17, 28);
    desc = likert(xsrh, 6);
    c_put_str(likert_color, desc, 17, 42);

    put_str("Disarming   :", 18, 28);
    desc = likert(xdis, 8);
    c_put_str(likert_color, desc, 18, 42);

    put_str("Magic Device:", 19, 28);
    desc = likert(xdev, 6);
    c_put_str(likert_color, desc, 19, 42);


    put_str("Blows/Round:", 16, 55);
    put_str(format("%d", p_ptr->num_blow), 16, 69);
    
    put_str("Shots/Round:", 17, 55);
    put_str(format("%d", p_ptr->num_fire), 17, 69);
    
    put_str("Infra-Vision:", 19, 55);
    put_str(format("%d feet", p_ptr->see_infra * 10), 19, 69);
}



/*
 * Display the character on the screen (with optional history)
 *
 * The top two and bottom two lines are left blank.
 */
void display_player(bool do_hist)
{
    int i;

    char	buf[80];


    /* Clear the screen */
    clear_screen();

    /* Name, Sex, Race, Class */
    put_str("Name        :", 2, 1);
    put_str("Sex         :", 3, 1);
    put_str("Race        :", 4, 1);
    put_str("Class       :", 5, 1);

    c_put_str(TERM_L_BLUE, player_name, 2, 15);
    c_put_str(TERM_L_BLUE, (p_ptr->male ? "Male" : "Female"), 3, 15);
    c_put_str(TERM_L_BLUE, rp_ptr->title, 4, 15);
    c_put_str(TERM_L_BLUE, cp_ptr->title, 5, 15);
    
    /* Age, Height, Weight, Social */
    prt_num("Age          ", (int)p_ptr->age, 2, 32, TERM_L_BLUE);
    prt_num("Height       ", (int)p_ptr->ht, 3, 32, TERM_L_BLUE);
    prt_num("Weight       ", (int)p_ptr->wt, 4, 32, TERM_L_BLUE);
    prt_num("Social Class ", (int)p_ptr->sc, 5, 32, TERM_L_BLUE);

    /* Display the stats */
    for (i = 0; i < 6; i++) {

        /* Special treatment of "injured" stats */
        if (p_ptr->stat_cur[i] < p_ptr->stat_max[i]) {

            int value;
            
            /* Use lowercase stat name */
            put_str(stat_names_reduced[i], 2 + i, 61);

            /* Get the current stat */
            value = p_ptr->stat_use[i];

            /* Obtain the current stat (modified) */
            cnv_stat(value, buf);

            /* Display the current stat (modified) */
            c_put_str(TERM_YELLOW, buf, 2 + i, 66);

            /* Acquire the max stat */
            value = p_ptr->stat_top[i];

            /* Obtain the maximum stat (modified) */
            cnv_stat(value, buf);

            /* Display the maximum stat (modified) */
            c_put_str(TERM_L_GREEN, buf, 2 + i, 73);
        }

        /* Normal treatment of "normal" stats */
        else {

            /* Assume uppercase stat name */
            put_str(stat_names[i], 2 + i, 61);

            /* Obtain the current stat (modified) */
            cnv_stat(p_ptr->stat_use[i], buf);

            /* Display the current stat (modified) */
            c_put_str(TERM_L_GREEN, buf, 2 + i, 66);
        }
    }

    /* Extra info */
    display_player_middle();

    /* Display "history" info */
    if (do_hist) {

        put_str("(Character Background)", 15, 25);

        for (i = 0; i < 4; i++) {
            put_str(history[i], i + 16, 10);
        }
    }
    
    /* Display "various" info */
    else {

        put_str("(Miscellaneous Abilities)", 15, 25);

        display_player_various();
    }
}




/*
 * Hack -- Dump a character description file
 *
 * XXX XXX XXX Allow the "full" flag to dump additional info,
 * and trigger its usage from various places in the code.
 */
errr file_character(cptr name, bool full)
{
    int			i, x, y;

    byte		a;
    char		c;

#if 0
    cptr		other = "(";
#endif

    cptr		paren = ")";

    int			fd = -1;

    FILE		*fff = NULL;

    store_type		*st_ptr = &store[7];

    char		i_name[80];

    char		buf[1024];


    /* Drop priv's */
    safe_setuid_drop();

    /* Access the help file */
    strcpy(buf, ANGBAND_DIR_USER);
    strcat(buf, name);

#if defined(MACINTOSH) && !defined(applec)
    /* Global -- "text file" */
    _ftype = 'TEXT';
#endif

    /* Check for existing file */
    fd = fd_open(buf, O_RDONLY, 0);
    
    /* Existing file */
    if (fd >= 0) {

        char out_val[160];

        /* Close the file */
        (void)fd_close(fd);

        /* Build query */
        (void)sprintf(out_val, "Replace existing file %s? ", buf);

        /* Ask */
        if (get_check(out_val)) fd = -1;
    }

    /* Open the non-existing file */
    if (fd < 0) fff = my_fopen(buf, "w");

    /* Grab priv's */
    safe_setuid_grab();


    /* Invalid file */
    if (!fff) {

        /* Message */
        msg_format("Character dump failed!");
        msg_print(NULL);

        /* Error */
        return (-1);
    }


    /* Begin dump */
    fprintf(fff, "  [Angband %d.%d.%d Character Dump]\n\n",
            VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
            

    /* Save screen */
    Term_save();

    /* Display the player (with various) */
    display_player(FALSE);

    /* Dump part of the screen */
    for (y = 2; y < 22; y++) {

        /* Dump each row */
        for (x = 0; x < 79; x++) {

            /* Get the attr/char */
            (void)(Term_what(x, y, &a, &c));

            /* Dump it */
            buf[x] = c;
        }

        /* Terminate */
        buf[x] = '\0';
        
        /* End the row */
        fprintf(fff, "%s\n", buf);
    }

    /* Display the player (with history) */
    display_player(TRUE);

    /* Dump part of the screen */
    for (y = 15; y < 20; y++) {

        /* Dump each row */
        for (x = 0; x < 79; x++) {

            /* Get the attr/char */
            (void)(Term_what(x, y, &a, &c));

            /* Dump it */
            buf[x] = c;
        }

        /* Terminate */
        buf[x] = '\0';
        
        /* End the row */
        fprintf(fff, "%s\n", buf);
    }

    /* Restore screen */
    Term_load();

    /* Skip some lines */
    fprintf(fff, "\n\n");


    /* Dump the equipment */
    if (equip_cnt) {
        fprintf(fff, "  [Character Equipment]\n\n");
        for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
            objdes(i_name, &inventory[i], TRUE, 3);
            fprintf(fff, "%c%s %s\n",
                    index_to_label(i), paren, i_name);
        }
        fprintf(fff, "\n\n");
    }

    /* Dump the inventory */
    fprintf(fff, "  [Character Inventory]\n\n");
    for (i = 0; i < INVEN_PACK; i++) {
        objdes(i_name, &inventory[i], TRUE, 3);
        fprintf(fff, "%c%s %s\n",
                index_to_label(i), paren, i_name);
    }
    fprintf(fff, "\n\n");


    /* Dump the Home (page 1) */
    fprintf(fff, "  [Home Inventory (page 1)]\n\n");	
    for (i = 0; i < 12; i++) {
        objdes(i_name, &st_ptr->stock[i], TRUE, 3);
        fprintf(fff, "%c%s %s\n", I2A(i%12), paren, i_name);
    }
    fprintf(fff, "\n\n");

    /* Dump the Home (page 2) */
    fprintf(fff, "  [Home Inventory (page 2)]\n\n");	
    for (i = 12; i < 24; i++) {
        objdes(i_name, &st_ptr->stock[i], TRUE, 3);
        fprintf(fff, "%c%s %s\n", I2A(i%12), paren, i_name);
    }
    fprintf(fff, "\n\n");


    /* Close it */
    my_fclose(fff);


    /* Message */
    msg_print("Character dump successful.");
    msg_print(NULL);

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
    FILE        *fp;

    char	buf[1024];


    /* Clear the screen */
    clear_screen();

    /* Access the "news" file */
    strcpy(buf, ANGBAND_DIR_FILE);
    strcat(buf, "news.txt");

    /* Open the News file */
    fp = my_fopen(buf, "r");

    /* Dump */
    if (fp) {

        int i = 0;

        /* Dump the file to the screen */
        while (0 == my_fgets(fp, buf, 1024)) {

            /* Display and advance */
            put_str(buf, i++, 0);
        }

        /* Close */
        my_fclose(fp);
    }

    /* Flush it */
    Term_fresh();
}



/*
 * Recursive "help file" perusal.  Return FALSE on "ESCAPE".
 *
 * XXX XXX XXX Consider using a temporary file.
 */
static bool do_cmd_help_aux(cptr name, cptr what, int line)
{
    int		i, k;

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
    char	finder[128];

    /* Describe this thing */
    char	caption[128];

    /* Path buffer */
    char	path[1024];

    /* General buffer */
    char	buf[1024];

    /* Sub-menu information */
    char	hook[10][32];


    /* Wipe finder */
    strcpy(finder, "");

    /* Wipe caption */
    strcpy(caption, "");

    /* Wipe the hooks */
    for (i = 0; i < 10; i++) hook[i][0] = '\0';


    /* Hack XXX XXX XXX */
    if (what) {

        /* Caption */
        strcpy(caption, what);

        /* Access the "file" */
        strcpy(path, name);

        /* Open */
        fff = my_fopen(path, "r");
    }

    /* Look in "help" */
    if (!fff) {
        
        /* Caption */
        sprintf(caption, "Help file '%s'", name);

        /* Access the "help" file */
        strcpy(path, ANGBAND_DIR_HELP);
        strcat(path, name);

        /* Open the file */
        fff = my_fopen(path, "r");
    }

    /* Look in "info" */
    if (!fff) {
        
        /* Caption */
        sprintf(caption, "Info file '%s'", name);

        /* Access the "info" file */
        strcpy(path, ANGBAND_DIR_INFO);
        strcat(path, name);

        /* Open the file */
        fff = my_fopen(path, "r");
    }

    /* Oops */
    if (!fff) {

        /* Message */
        msg_format("Cannot open '%s'.", name);
        msg_print(NULL);

        /* Oops */
        return (TRUE);
    }


    /* Pre-Parse the file */
    while (TRUE) {

        /* Read a line or stop */
        if (my_fgets(fff, buf, 1024)) break;

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
            if (my_fgets(fff, buf, 1024)) break;
        }


        /* Dump the next 20 lines of the file */
        for (i = 0; i < 20; ) {

            /* Hack -- track the "first" line */
            if (!i) line = next;

            /* Get a line of the file or stop */
            if (my_fgets(fff, buf, 1024)) break;

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
        prt(format("[Angband %d.%d.%d, %s, Line %d/%d]",
                   VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,
                   caption, line, size), 0, 0);


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
                if (!do_cmd_help_aux(tmp, NULL, 0)) k = ESCAPE;
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
            if (!do_cmd_help_aux(hook[k-'0'], NULL, 0)) k = ESCAPE;
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
    (void)do_cmd_help_aux(name, NULL, 0);

    /* Restore the screen */
    Term_load();

    /* Leave "icky" mode */
    character_icky = FALSE;
}



/*
 * Hack -- display the contents of a file on the screen
 *
 * XXX XXX XXX Use this function for commands such as the
 * "examine object" command.
 */
errr show_file(cptr name, cptr what)
{
    /* Enter "icky" mode */
    character_icky = TRUE;

    /* Save the screen */
    Term_save();

    /* Peruse the requested file */
    (void)do_cmd_help_aux(name, what, 0);

    /* Restore the screen */
    Term_load();

    /* Leave "icky" mode */
    character_icky = FALSE;

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
            quit_fmt("The name '%s' contains control chars!", player_name);
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

#ifdef VM
        /* Hack -- support "flat directory" usage on VM/ESA */
        (void)sprintf(savefile, "%s%s.sv",
                        ANGBAND_DIR_SAVE, player_base);
#endif /* VM */

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
 * Hack -- commit suicide
 */
void do_cmd_suicide(void)
{
    int i;

    /* Flush input */
    flush();

    /* Verify Retirement */
    if (total_winner) {

        /* Verify */
        if (!get_check("Do you want to retire? ")) return;
    }

    /* Verify Suicide */
    else {

        /* Verify */
        if (!get_check("Do you really want to quit? ")) return;

        /* Special Verify */
        prt("Please verify QUITTING by typing the '@' sign: ", 0, 0);
        flush();
        i = inkey();
        prt("", 0, 0);
        if (i != '@') return;
    }

    /* Stop playing */
    alive = FALSE;

    /* Kill the player */
    death = TRUE;

    /* Cause of death */
    (void)strcpy(died_from, "Quitting");
}



/*
 * Save the game
 */
void do_cmd_save_game(void)
{
    /* Disturb the player */
    disturb(1, 0);

    /* Clear messages */
    msg_print(NULL);

    /* Handle stuff */
    handle_stuff();

    /* Message */
    prt("Saving game...", 0, 0);

    /* Refresh */
    Term_fresh();

    /* The player is not dead */
    (void)strcpy(died_from, "(saved)");

    /* Forbid suspend */
    signals_ignore_tstp();

    /* Save the player */
    if (save_player()) {
        prt("Saving game... done.", 0, 0);
    }

    /* Save failed (oops) */
    else {
        prt("Saving game... failed!", 0, 0);
    }

    /* Allow suspend again */
    signals_handle_tstp();
    
    /* Refresh */
    Term_fresh();

    /* Hack -- Forget that the player was saved */
    character_saved = FALSE;

    /* Note that the player is not dead */
    (void)strcpy(died_from, "(alive and well)");
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

            /* XXX XXX XXX Get the proper "Bones File" name */
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
 * Display a "tomb-stone"
 */
static void print_tomb()
{
    cptr	p;

    char	tmp[160];

    char	buf[1024];

    FILE        *fp;

    time_t	ct = time((time_t)0);


    /* Clear the screen */
    clear_screen();

    /* Access the "dead" file */
    strcpy(buf, ANGBAND_DIR_FILE);
    strcat(buf, "dead.txt");

    /* Open the News file */
    fp = my_fopen(buf, "r");

    /* Dump */
    if (fp) {

        int i = 0;

        /* Dump the file to the screen */
        while (0 == my_fgets(fp, buf, 1024)) {

            /* Display and advance */
            put_str(buf, i++, 0);
        }

        /* Close */
        my_fclose(fp);
    }


    /* King or Queen */
    if (total_winner || (p_ptr->lev > PY_MAX_LEVEL)) {
        p = "Magnificent";
    }

#ifdef ALLOW_TITLES

    /* Hack -- Novice */
    else if (p_ptr->lev < 1) {
        p = "Novice";
    }
    
    /* Normal */
    else {
        p =  player_title[p_ptr->pclass][p_ptr->lev - 1];
    }

#else

    /* Oops */
    else {
        p = "Player";
    }

#endif


    center_string(buf, player_name);
    put_str(buf, 6, 11);

    center_string(buf, "the");
    put_str(buf, 7, 11);

    center_string(buf, p);
    put_str(buf, 8, 11);


    center_string(buf, cp_ptr->title);
    put_str(buf, 10, 11);

    (void)sprintf(tmp, "Level: %d", (int)p_ptr->lev);
    center_string(buf, tmp);
    put_str(buf, 11, 11);

    (void)sprintf(tmp, "Exp: %ld", (long)p_ptr->exp);
    center_string(buf, tmp);
    put_str(buf, 12, 11);

    (void)sprintf(tmp, "AU: %ld", (long)p_ptr->au);
    center_string(buf, tmp);
    put_str(buf, 13, 11);

    (void)sprintf(tmp, "Killed on Level %d", dun_level);
    center_string(buf, tmp);
    put_str(buf, 14, 11);

    (void)sprintf(tmp, "by %s.", died_from);
    center_string(buf, tmp);
    put_str(buf, 15, 11);


    (void)sprintf(tmp, "%-.24s", ctime(&ct));
    center_string(buf, tmp);
    put_str(buf, 17, 11);
}


/*
 * Display some character info
 */
static void show_info(void)
{
    int			i, j, k;

    inven_type		*i_ptr;

    store_type		*st_ptr = &store[7];

    char		p1 = '(', p2 = ')';


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

        /* Prompt */
        put_str("Filename: ", 23, 0);

        /* Ask for filename (or abort) */
        if (!askfor(out_val, 60)) return;

        /* Return means "show on screen" */
        if (!out_val[0]) break;

        /* Dump a character file */
        (void)file_character(out_val, FALSE);
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
        prt("You are using: -more-", 0, 0);
        if (inkey() == ESCAPE) return;
    }

    /* Inventory -- if any */
    if (inven_cnt) {
        clear_screen();
        item_tester_full = TRUE;
        show_inven();
        prt("You are carrying: -more-", 0, 0);
        if (inkey() == ESCAPE) return;
    }



    /* Home -- if anything there */
    if (st_ptr->stock_num) {

        /* Display contents of the home */
        for (k = 0, i = 0; i < st_ptr->stock_num; k++) {

            /* Clear the screen */
            clear_screen();

            /* Show 12 items */
            for (j = 0; (j < 12) && (i < st_ptr->stock_num); j++, i++) {

                char i_name[80];
                char tmp_val[80];

                /* Acquire item */
                i_ptr = &st_ptr->stock[i];

                /* Print header, clear line */
                sprintf(tmp_val, "%c) ", I2A(j));
                prt(tmp_val, j+2, 4);

                /* Display object description */
                objdes(i_name, i_ptr, TRUE, 3);
                c_put_str(tval_to_attr[i_ptr->tval], i_name, j+2, 7);
            }

            /* Caption */
            prt(format("Your home contains (page %d): -more-", k+1), 0, 0);

            /* Wait for it */
            if (inkey() == ESCAPE) return;
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

        /* Title */
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
            sprintf(out_val,
                    "               (User %s, Date %s, Gold %s, Turn %s).",
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

    /* Access the high score file */
    strcpy(buf, ANGBAND_DIR_APEX);
    strcat(buf, "scores.raw");

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
        msg_print(NULL);
        display_scores_aux(0, 10, -1, NULL);
        return (0);
    }
#endif

#ifndef SCORE_BORGS
    /* Borg-mode pre-empts scoring */
    if (noscore & 0x00F0) {
        msg_print("Score not registered for borgs.");
        msg_print(NULL);
        display_scores_aux(0, 10, -1, NULL);
        return (0);
    }
#endif

#ifndef SCORE_CHEATERS
    /* Cheaters are not scored */
    if (noscore & 0xFF00) {
        msg_print("Score not registered for cheaters.");
        msg_print(NULL);
        display_scores_aux(0, 10, -1, NULL);
        return (0);
    }
#endif

    /* Interupted */
    if (!total_winner && streq(died_from, "Interrupting")) {
        msg_print("Score not registered due to interruption.");
        msg_print(NULL);
        display_scores_aux(0, 10, -1, NULL);
        return (0);
    }

    /* Quitter */
    if (!total_winner && streq(died_from, "Quitting")) {
        msg_print("Score not registered due to quitting.");
        msg_print(NULL);
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

    /* Restore the experience */
    p_ptr->exp = p_ptr->max_exp;

    /* Restore the level */
    p_ptr->lev = p_ptr->max_plv;
    
    /* Check the experience */
    /* check_experience(); */

    /* Mega-Hack -- Instant Experience */
    p_ptr->exp += 5000000L;
    p_ptr->max_exp += 5000000L;

    /* Mega-Hack -- Instant Gold */
    p_ptr->au += 250000L;

    /* Automatic level 50 */
    /* p_ptr->lev = 50; */

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


    /* Access the high score file */
    strcpy(buf, ANGBAND_DIR_APEX);
    strcat(buf, "scores.raw");

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

    /* Forbid suspend */
    signals_ignore_tstp();

    /* Indicate panic save */
    (void)strcpy(died_from, "(panic save)");

    /* Panic save, or get worried */
    if (!save_player()) quit("panic save failed!");

    /* Successful panic save */
    quit("panic save succeeded!");
}



#ifdef HANDLE_SIGNALS


#include <signal.h>


/*
 * Handle signals -- suspend
 *
 * Actually suspend the game, and then resume cleanly
 */
static void handle_signal_suspend(int sig)
{
    /* Disable handler */
    (void)signal(sig, SIG_IGN);

#ifdef SIGSTOP

    /* Flush output */
    Term_fresh();

    /* Suspend the "Term" */
    Term_xtra(TERM_XTRA_ALIVE, 0);

    /* Suspend ourself */
    (void)kill(0, SIGSTOP);

    /* Resume the "Term" */
    Term_xtra(TERM_XTRA_ALIVE, 1);

    /* Redraw the term */
    Term_redraw();

    /* Flush the term */
    Term_fresh();

#endif

    /* Restore handler */
    (void)signal(sig, handle_signal_suspend);
}


/*
 * Handle signals -- simple (interrupt and quit)
 *
 * This function was causing a *huge* number of problems, so it has
 * been simplified greatly.  We keep a global variable which counts
 * the number of times the user attempts to kill the process, and
 * we commit suicide if the user does this a certain number of times.
 *
 * We attempt to give "feedback" to the user as he approaches the
 * suicide thresh-hold, but without penalizing accidental keypresses.
 *
 * To prevent messy accidents, we should reset this global variable
 * whenever the user enters a keypress, or something like that.
 */
static void handle_signal_simple(int sig)
{
    /* Disable handler */
    (void)signal(sig, SIG_IGN);


    /* Nothing to save, just quit */
    if (!character_generated || character_saved) quit(NULL);


    /* Count the signals */
    signal_count++;

    
    /* Terminate dead characters */
    if (death) {

        /* Mark the savefile */
        (void)strcpy(died_from, "Abortion");

        /* Close stuff */
        close_game();

        /* Quit */
        quit("interrupt");
    }

    /* Allow suicide (after 5) */
    else if (signal_count >= 5) {
    
        /* Cause of "death" */
        (void)strcpy(died_from, "Interrupting");

        /* Stop playing */
        alive = FALSE;

        /* Suicide */
        death = TRUE;

        /* Close stuff */
        close_game();

        /* Quit */
        quit("interrupt");
    }

    /* Give warning (after 4) */
    else if (signal_count >= 4) {
    
        /* Make a noise */
        Term_xtra(TERM_XTRA_NOISE, 0);
    
        /* Clear the top line */
        Term_erase(0, 0, 80, 1);
        
        /* Display the cause */
        Term_putstr(0, 0, -1, TERM_WHITE, "Contemplating suicide!");

        /* Flush */
        Term_fresh();
    }

    /* Give warning (after 2) */
    else if (signal_count >= 2) {
    
        /* Make a noise */
        Term_xtra(TERM_XTRA_NOISE, 0);
    }

    /* Restore handler */
    (void)signal(sig, handle_signal_simple);
}


/*
 * Handle signal -- abort, kill, etc
 */
static void handle_signal_abort(int sig)
{
    /* Disable handler */
    (void)signal(sig, SIG_IGN);


    /* Nothing to save, just quit */
    if (!character_generated || character_saved) quit(NULL);

    
    /* Clear the bottom line */
    Term_erase(0, 23, 80, 1);
    
    /* Give a warning */
    Term_putstr(0, 23, -1, TERM_RED,
                "A gruesome software bug LEAPS out at you!");

    /* Message */
    Term_putstr(45, 23, -1, TERM_RED, "Panic save...");

    /* Flush output */
    Term_fresh();

    /* Panic Save */
    panic_save = 1;

    /* Panic save */
    (void)strcpy(died_from, "(panic save)");

    /* Forbid suspend */
    signals_ignore_tstp();

    /* Attempt to save */
    if (save_player()) {
        Term_putstr(45, 23, -1, TERM_RED, "Panic save succeeded!");
    }
    
    /* Save failed */
    else {
        Term_putstr(45, 23, -1, TERM_RED, "Panic save failed!");
    }
    
    /* Flush output */
    Term_fresh();
    
    /* Quit */
    quit("software bug");
}




/*
 * Ignore SIGTSTP signals (keyboard suspend)
 */
void signals_ignore_tstp(void)
{

#ifdef SIGTSTP
    (void)signal(SIGTSTP, SIG_IGN);
#endif

}

/*
 * Handle SIGTSTP signals (keyboard suspend)
 */
void signals_handle_tstp(void)
{

#ifdef SIGTSTP
    (void)signal(SIGTSTP, handle_signal_suspend);
#endif

}


/*
 * Prepare to handle the relevant signals
 */
void signals_init()
{

#ifdef SIGHUP
    (void)signal(SIGHUP, SIG_IGN);
#endif


#ifdef SIGTSTP
    (void)signal(SIGTSTP, handle_signal_suspend);
#endif


#ifdef SIGINT
    (void)signal(SIGINT, handle_signal_simple);
#endif

#ifdef SIGQUIT
    (void)signal(SIGQUIT, handle_signal_simple);
#endif


#ifdef SIGFPE
    (void)signal(SIGFPE, handle_signal_abort);
#endif

#ifdef SIGILL
    (void)signal(SIGILL, handle_signal_abort);
#endif

#ifdef SIGTRAP
    (void)signal(SIGTRAP, handle_signal_abort);
#endif

#ifdef SIGIOT
    (void)signal(SIGIOT, handle_signal_abort);
#endif

#ifdef SIGKILL
    (void)signal(SIGKILL, handle_signal_abort);
#endif

#ifdef SIGBUS
    (void)signal(SIGBUS, handle_signal_abort);
#endif

#ifdef SIGSEGV
    (void)signal(SIGSEGV, handle_signal_abort);
#endif

#ifdef SIGTERM
    (void)signal(SIGTERM, handle_signal_abort);
#endif

#ifdef SIGPIPE
    (void)signal(SIGPIPE, handle_signal_abort);
#endif

#ifdef SIGEMT
    (void)signal(SIGEMT, handle_signal_abort);
#endif

#ifdef SIGDANGER
    (void)signal(SIGDANGER, handle_signal_abort);
#endif

#ifdef SIGSYS
    (void)signal(SIGSYS, handle_signal_abort);
#endif

#ifdef SIGXCPU
    (void)signal(SIGXCPU, handle_signal_abort);
#endif

#ifdef SIGPWR
    (void)signal(SIGPWR, handle_signal_abort);
#endif

}


#else	/* HANDLE_SIGNALS */


/*
 * Do nothing
 */
void signals_ignore_tstp(void)
{
}

/*
 * Do nothing
 */
void signals_handle_tstp(void)
{
}

/*
 * Do nothing
 */
void signals_init(void)
{
}


#endif	/* HANDLE_SIGNALS */


