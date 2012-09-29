/* File: files.c */ 

/* Purpose: misc code to access files used by Moria */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "angband.h"


/*
 * Check the hours file vs. what time it is	-Doc
 */
void read_times(void)
{

#ifdef CHECK_HOURS

    register int i;
    vtype        in_line, buf;
    FILE        *file1;

/* Attempt to read hours.dat.	 If it does not exist,	   */
/* inform the user so he can tell the wizard about it	 */

    file1 = my_tfopen(ANGBAND_HOURS, "r");
    if (file1) {
	while (fgets(in_line, 80, file1)) {
	    if (strlen(in_line) > 3) {
		if (!strncmp(in_line, "SUN:", 4))
		    (void)strcpy(days[0], in_line);
		else if (!strncmp(in_line, "MON:", 4))
		    (void)strcpy(days[1], in_line);
		else if (!strncmp(in_line, "TUE:", 4))
		    (void)strcpy(days[2], in_line);
		else if (!strncmp(in_line, "WED:", 4))
		    (void)strcpy(days[3], in_line);
		else if (!strncmp(in_line, "THU:", 4))
		    (void)strcpy(days[4], in_line);
		else if (!strncmp(in_line, "FRI:", 4))
		    (void)strcpy(days[5], in_line);
		else if (!strncmp(in_line, "SAT:", 4))
		    (void)strcpy(days[6], in_line);
	    }
	}
	(void)fclose(file1);
    }

    else {
	sprintf(buf,"There is no hours file \"%s\".\n", ANGBAND_HOURS);
	message(buf, 0);
	sprintf(buf,"Please inform the head wizard, %s!\n", WIZARD);
	message(buf, 0);
	message(NULL,0);
	exit_game();
    }

/* Check the hours, if closed	then exit. */
    if (!check_time()) {
	file1 = my_tfopen(ANGBAND_HOURS, "r");
	if (file1) {
	    clear_screen();
	    for (i = 0; fgets(in_line, 80, file1); i++) {
		put_str(in_line, i, 0);
	    }
	    (void)fclose(file1);
	    pause_line(23);
	}
	exit_game();
    }
#endif				   /* CHECK_HOURS */

}


/*
 * Attempt to open, and display, the intro "news" file		-RAK-
 */
void show_news(void)
{
    register int i;
    vtype        in_line, buf;
    FILE        *file1;


    /* Try to open the News file */
    file1 = my_tfopen(ANGBAND_NEWS, "r");

    /* Error? */
    if (!file1) {

	sprintf(buf, "Cannot read '%s'!\n", ANGBAND_NEWS);
	message(buf, 0);
	message(NULL,0);

	quit("cannot open 'news' file");
    }

    /* Clear the screen */    
    clear_screen();

    /* Dump the file (nuking newlines) */
    for (i = 0; (i < 24) && fgets(in_line, 80, file1); i++) {
	in_line[strlen(in_line)-1]=0;
	put_str(in_line, i, 0);
    }

    /* Flush it */
    Term_fresh();

    /* Close */
    (void)fclose(file1);
}


/*
 * File perusal.	    -CJS- primitive, but portable 
 */
void helpfile(cptr filename)
{
    bigvtype tmp_str;
    FILE    *file;
    char     input;
    int      i;

    file = my_tfopen(filename, "r");
    if (!file) {
	(void)sprintf(tmp_str, "Can not find help file \"%s\".\n", filename);
	message(tmp_str, 0);
	return;
    }

    save_screen();

    while (!feof(file))
    {
	clear_screen();
	for (i = 0; i < 23; i++) {
	    if (fgets(tmp_str, BIGVTYPESIZ - 1, file)) {
		tmp_str[strlen(tmp_str)-1] = '\0';
		put_str(tmp_str, i, 0);
	    }
	}

	prt("[Press any key to continue.]", 23, 23);
	input = inkey();
	if (input == ESCAPE) break;
    }

    (void)fclose(file);

    restore_screen();
}





/*
 * Print the character to a file or device.
 */
int file_character(cptr filename1)
{
    register int		i;
    int				j;
    int				xbth, xbthb, xfos, xsrh;
    int				xstl, xdis, xsave, xdev;
    vtype			xinfra;
    int				fd = -1;
    bigvtype			prt2;
    register inven_type		*i_ptr;
    vtype			out_val, prt1;
    cptr			p;
    cptr			colon = ":";
    cptr			blank = " ";

    register FILE		*file1;

#ifdef MACINTOSH

    /* Open the file (already verified by mac_file_character) */
    file1 = fopen(filename1, "w");

#else

    fd = my_topen(filename1, O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (fd < 0 && errno == EEXIST) {
	(void)sprintf(out_val, "Replace existing file %s?", filename1);
	if (get_check(out_val)) {
	    fd = my_topen(filename1, O_WRONLY, 0644);
	}
    }
    if (fd >= 0) {
	/* on some non-unix machines, fdopen() is not reliable, */
	/* hence must call close() and then fopen()  */
	(void)close(fd);
	file1 = my_tfopen(filename1, "w");
    }
    else {
	file1 = NULL;
    }

#endif

    if (file1) {

	prt("Writing character sheet...", 0, 0);
	Term_fresh();

	colon = ":";
	blank = " ";
	/* (void)fprintf(file1, "%c\n\n", CTRL('L')); */
	(void)fprintf(file1, " Name%9s %-23s", colon, player_name);
	(void)fprintf(file1, "Age%11s %6d ", colon, (int)p_ptr->age);
	cnv_stat(p_ptr->use_stat[A_STR], prt1);
	(void)fprintf(file1, "   STR : %s\n", prt1);
	(void)fprintf(file1, " Race%9s %-23s", colon, race[p_ptr->prace].trace);
	(void)fprintf(file1, "Height%8s %6d ", colon, (int)p_ptr->ht);
	cnv_stat(p_ptr->use_stat[A_INT], prt1);
	(void)fprintf(file1, "   INT : %s\n", prt1);
	(void)fprintf(file1, " Sex%10s %-23s", colon,
		      (p_ptr->male ? "Male" : "Female"));
	(void)fprintf(file1, "Weight%8s %6d ", colon, (int)p_ptr->wt);
	cnv_stat(p_ptr->use_stat[A_WIS], prt1);
	(void)fprintf(file1, "   WIS : %s\n", prt1);
	(void)fprintf(file1, " Class%8s %-23s", colon,
		      class[p_ptr->pclass].title);
	(void)fprintf(file1, "Social Class : %6d ", p_ptr->sc);
	cnv_stat(p_ptr->use_stat[A_DEX], prt1);
	(void)fprintf(file1, "   DEX : %s\n", prt1);
	(void)fprintf(file1, " Title%8s %-23s", colon, title_string());
	(void)fprintf(file1, "%22s", blank);
	cnv_stat(p_ptr->use_stat[A_CON], prt1);
	(void)fprintf(file1, "   CON : %s\n", prt1);
	(void)fprintf(file1, "%34s", blank);
	(void)fprintf(file1, "%26s", blank);
	cnv_stat(p_ptr->use_stat[A_CHR], prt1);
	(void)fprintf(file1, "   CHR : %s\n\n", prt1);

	(void)fprintf(file1, " + To Hit    : %6d", p_ptr->dis_th);
	(void)fprintf(file1, "%7sLevel      :%9d", blank, (int)p_ptr->lev);
	(void)fprintf(file1, "   Max Hit Points : %6d\n", p_ptr->mhp);
	(void)fprintf(file1, " + To Damage : %6d", p_ptr->dis_td);
	(void)fprintf(file1, "%7sExperience :%9ld", blank, (long)p_ptr->exp);
	(void)fprintf(file1, "   Cur Hit Points : %6d\n", p_ptr->chp);
	(void)fprintf(file1, " + To AC     : %6d", p_ptr->dis_tac);
	(void)fprintf(file1, "%7sMax Exp    :%9ld", blank, (long)p_ptr->max_exp);
	(void)fprintf(file1, "   Max Mana%8s %6d\n", colon, p_ptr->mana);
	(void)fprintf(file1, "   Total AC  : %6d", p_ptr->dis_ac);
	if (p_ptr->lev >= MAX_PLAYER_LEVEL) {
	    (void)fprintf(file1, "%7sExp to Adv.:%9s", blank, "****");
	}
	else {
	    (void)fprintf(file1, "%7sExp to Adv.:%9ld", blank,
			  (long) (player_exp[p_ptr->lev - 1] *
				   p_ptr->expfact / 100L));
	}
	(void)fprintf(file1, "   Cur Mana%8s %6d\n", colon, p_ptr->cmana);
	(void)fprintf(file1, "%28sGold%8s%9ld\n", blank, colon, (long)p_ptr->au);

	xbth = p_ptr->bth + p_ptr->ptohit * BTH_PLUS_ADJ
	    + (class_level_adj[p_ptr->pclass][CLA_BTH] * p_ptr->lev);
	xbthb = p_ptr->bthb + p_ptr->ptohit * BTH_PLUS_ADJ
	    + (class_level_adj[p_ptr->pclass][CLA_BTHB] * p_ptr->lev);
    /* this results in a range from 0 to 29 */
	xfos = 40 - p_ptr->fos;
	if (xfos < 0) xfos = 0;
	xsrh = p_ptr->srh;
    /* this results in a range from 0 to 9 */
	xstl = p_ptr->stl + 1;
	xdis = p_ptr->disarm + 2 * todis_adj() + stat_adj(A_INT)
	    + (class_level_adj[p_ptr->pclass][CLA_DISARM] * p_ptr->lev / 3);
	xsave = p_ptr->save + stat_adj(A_WIS)
	    + (class_level_adj[p_ptr->pclass][CLA_SAVE] * p_ptr->lev / 3);
	xdev = p_ptr->save + stat_adj(A_INT)
	    + (class_level_adj[p_ptr->pclass][CLA_DEVICE] * p_ptr->lev / 3);

	(void)sprintf(xinfra, "%d feet", p_ptr->see_infra * 10);

	(void)fprintf(file1, "\n(Miscellaneous Abilities)\n");
	(void)fprintf(file1, " Fighting    : %-10s", likert(xbth, 12));
	(void)fprintf(file1, "   Stealth     : %-10s", likert(xstl, 1));
	(void)fprintf(file1, "   Perception  : %s\n", likert(xfos, 3));
	(void)fprintf(file1, " Bows/Throw  : %-10s", likert(xbthb, 12));
	(void)fprintf(file1, "   Disarming   : %-10s", likert(xdis, 8));
	(void)fprintf(file1, "   Searching   : %s\n", likert(xsrh, 6));
	(void)fprintf(file1, " Saving Throw: %-10s", likert(xsave, 6));
	(void)fprintf(file1, "   Magic Device: %-10s", likert(xdev, 6));
	(void)fprintf(file1, "   Infra-Vision: %s\n\n", xinfra);
    /* Write out the character's history     */
	(void)fprintf(file1, "Character Background\n");
	for (i = 0; i < 4; i++)
	    (void)fprintf(file1, " %s\n", history[i]);
    /* Write out the equipment list.	     */
	j = 0;
	(void)fprintf(file1, "\n  [Character's Equipment List]\n\n");
	if (equip_ctr == 0) {
	    (void)fprintf(file1, "  Character has no equipment in use.\n");
	}
	else {
	    for (i = INVEN_WIELD; i < INVEN_ARRAY_SIZE; i++) {
		i_ptr = &inventory[i];
		if (i_ptr->tval != TV_NOTHING) {
		    p = mention_use(i);
		    objdes(prt2, &inventory[i], TRUE);
		    (void)fprintf(file1, "  %c) %-19s: %s\n", j + 'a', p, prt2);
		    j++;
		}
	    }
	}

    /* Write out the character's inventory.	     */
	(void)fprintf(file1, "\n\n");
	(void)fprintf(file1, "  [General Inventory List]\n\n");
	if (inven_ctr == 0) {
	    (void)fprintf(file1, "  Character has no objects in inventory.\n");
	}
	else {
	    for (i = 0; i < inven_ctr; i++) {
		objdes(prt2, &inventory[i], TRUE);
		(void)fprintf(file1, "%c) %s\n", i + 'a', prt2);
	    }
	}
	(void)fprintf(file1, "\n\n");

	fprintf(file1, "  [%s%s Home Inventory]\n\n", player_name,
		(toupper(player_name[strlen(player_name)-1]) == 'S' ? "'" : "'s"));
	if (store[MAX_STORES-1].store_ctr == 0) {
	    (void) fprintf(file1, "  Character has no objects at home.\n");
	}
	else {
	    for (i = 0; i < store[MAX_STORES-1].store_ctr; i++) {
		if (i==12) fprintf(file1, "\n");  
		objdes_store(prt2, &store[MAX_STORES-1].store_item[i], TRUE);
		(void) fprintf(file1, "%c) %s\n", (i%12)+'a', prt2);
	    }
	}
	(void) fprintf(file1, "\n");

	(void)fclose(file1);
	message("Completed.", 0);
	return TRUE;
    }
    else {

	(void)sprintf(out_val, "Can't open file: %s", filename1);
	message(out_val, 0);
	return FALSE;
    }
}



