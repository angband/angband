/* File: help.c */

/* Purpose: identify a symbol */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "angband.h"


/*
 * XXX Add more power to this file?
 * For example, a real, interactive "help"?
 */



/*
 * The table of "symbol info" -- each entry is a string of the form
 * "X:desc" where "X" is the trigger, and "desc" is the "info".
 */
static cptr ident_info[] = {
    " :An open pit",
    "!:A potion", 
    "\":An amulet, necklace, or periapt", 
    "#:A stone wall", 
    "$:Treasure", 
    "%:A magma or quartz vein", 
    "&:Demon (Oh dear!)", 
    "':An open door", 
    "(:Soft armor", 
    "):A shield", 
    "*:Gems", 
    "+:A closed door", 
    ",:Food or mushroom patch", 
    "-:A wand", 
    ".:Floor", 
    "/:A pole weapon", 
	/* "0:unused", */
    "1:Entrance to General Store", 
    "2:Entrance to Armory", 
    "3:Entrance to Weaponsmith", 
    "4:Entrance to Temple", 
    "5:Entrance to Alchemy shop", 
    "6:Entrance to Magic store", 
    "7:Entrance to Black Market", 
    "8:Entrance to your home", 
	/* "9:unused", */
    "::Rubble", 
    ";:A loose rock", 
    "<:An up staircase", 
    "=:A ring", 
    ">:A down staircase", 
    "?:A scroll", 
    "@:You", 
    "A:Angel", 
    "B:Birds", 
    "C:Canine", 
    "D:Ancient Dragon (Beware)", 
    "E:Elemental", 
    "F:Giant Fly", 
    "G:Ghost", 
    "H:Hybrid", 
    "I:Minor Demon", 
    "J:Jabberwock", 
    "K:Killer Beetle", 
    "L:Lich", 
    "M:Mummy", 
	/* "N:unused", */
    "O:Ogre", 
    "P:Giant humanoid", 
    "Q:Quylthulg (Pulsing Flesh Mound)", 
    "R:Reptile/Amphibian", 
    "S:Giant Scorpion/Spider", 
    "T:Troll", 
    "U:Umber Hulk", 
    "V:Vampire", 
    "W:Wight/Wraith", 
    "X:Xorn/Xaren", 
    "Y:Yeti", 
    "Z:Zepher (Elemental) hound", 
    "[:Hard armor", 
    "\\:A hafted weapon", 
    "]:Misc. armor", 
    "^:A trap", 
    "_:A staff", 
	/* "`:unused", */
    "a:Giant Ant/Ant Lion", 
    "b:Giant Bat", 
    "c:Giant Centipede", 
    "d:Dragon", 
    "e:Floating Eye", 
    "f:Feline", 
    "g:Golem", 
    "h:Humanoid (Dwarf/Elf/Halfling)", 
    "i:Icky Thing", 
    "j:Jelly", 
    "k:Kobold", 
    "l:Giant Louse", 
    "m:Mold", 
    "n:Naga", 
    "o:Orc", 
    "p:Person (Humanoid)", 
    "q:Quadruped", 
    "r:Rodent", 
    "s:Skeleton", 
    "t:Giant Tick", 
	/* "u:unused", */
    "v:Vortex", 
    "w:Worm/Worm Mass", 
	/* "x:unused", */
    "y:Yeek", 
    "z:Zombie", 
    "{:Missile (Arrow/bolt/bullet)", 
    "|:A sword (or dagger)", 
    "}:Launcher (Bow/crossbow/sling)", 
    "~:Tool (or miscellaneous item)",
    NULL
};



/*
 * Identify a character, allow recall of monnsters
 */
void ident_char(void)
{
    char         command, query;
    register int i;
    int n;

    /* The turn is free */
    free_turn_flag = TRUE;

    /* Get a character, or abort */
    if (!get_com("Enter character to be identified :", &command)) return;

    /* Find that character info, and describe it */
    for (i = 0; ident_info[i]; ++i) {
	if (command == ident_info[i][0]) {
	    char buf[128];
	    sprintf(buf, "%c - %s.", command, ident_info[i] + 2);
	    prt(buf, 0, 0);
	    break;
	}
    }

    /* Abort if nothing found */
    if (!ident_info[i]) return;


    /* No monsters recalled yet */
    n = 0;

    /* Allow access to monster memory. */
    for (i = MAX_R_IDX - 1; i >= 0; i--) {

	/* Did they ask about this monster? */
	if ((r_list[i].r_char == command) && bool_roff_recall(i))
	{
	    /* Hack -- assume no response */
	    query = 'n';

	    /* First monster gets a query */
	    if (!n) {

		put_str("You recall those details? [y/n]", 0, 40);
		query = inkey();
		erase_line(0, 40);

		if (query != 'y' && query != 'Y') break;
	    }

	    /* Count the monsters shown */
	    n++;

	    /* Graphic Recall window */
	    if (use_recall_win) {
		roff_recall(i);
		prt("--pause--", 0, 70);
		query = inkey();
	    }

	    /* On-screen recall */
	    else {

		/* Did they quit? */
		if (query == ESCAPE) break;

		/* Save the screen */
		save_screen();

		/* Describe the monster */
		query = roff_recall(i);

		/* Restore the screen */
		restore_screen();
	    }

	    /* Let the user stop it */
	    if (query == ESCAPE) break;
	}
    }

    /* Erase any "Recall" prompts */
    if (n) erase_line(0, 70);
}

