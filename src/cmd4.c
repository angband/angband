/*
 * File: cmd4.c
 * Purpose: Various kinds of browsing functions.
 *
 * Copyright (c) 1997-2007 Robert A. Koeneke, James E. Wilson, Ben Harrison,
 * Eytan Zweig, Andrew Doull, Pete Mack.
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
#include "angband.h"
#include "ui.h"



static void do_cmd_pref_file_hack(long row);



/* Flag value for missing array entry */
#define MISSING -17

#define APP_MACRO	101
#define ASK_MACRO	103
#define DEL_MACRO	104
#define NEW_MACRO	105
#define APP_KEYMAP	106
#define ASK_KEYMAP	107
#define DEL_KEYMAP	108
#define NEW_KEYMAP	109
#define ENTER_ACT	110
#define LOAD_PREF	111
#define DUMP_MON	112
#define DUMP_OBJ	113
#define DUMP_FEAT	114
#define DUMP_FLAV	115
#define MOD_MON		116
#define	MOD_OBJ		117
#define MOD_FEAT	118
#define MOD_FLAV	119
#define DUMP_COL	120
#define MOD_COL		121
#define RESET_VIS	122

#define OPTION_MENU 140
#define VISUAL_MENU 141
#define COLOR_MENU	142
#define KNOWLEDGE_MENU 143
#define MACRO_MENU	144


#define INFO_SCREENS 2 /* Number of screens in character info mode */



typedef struct
{
	int maxnum;          /* Maximum possible item count for this class */
	bool easy_know;      /* Items don't need to be IDed to recognize membership */

	const char *(*name)(int gid);               /* Name of this group */

	/* Compare, in group and display order (optional if already sorted) */
	int (*gcomp)(const void *, const void *);   /* Compares gids of two oids */
	int (*group)(int oid);                      /* Returns gid for an oid */

	/* Summary function for the "object" information. */
	void (*summary)(int gid, const int *object_list, int n, int top, int row, int col);

} group_funcs;

typedef struct
{
	/* Displays an entry at specified location, including kill-count and graphics */
	void (*display_member)(int col, int row, bool cursor, int oid);

	void (*lore)(int oid);       /* Displays lore for an oid */


	/* Required only for objects with modifiable display attributes */
	/* Unknown 'flavors' return flavor attributes */
	char *(*xchar)(int oid);     /* Get character attr for OID (by address) */
	byte *(*xattr)(int oid);     /* Get color attr for OID (by address) */

	const char *(*xtra_prompt)(int oid);  /* Returns optional extra prompt */
	void (*xtra_act)(char ch, int oid);   /* Handles optional extra actions */

	bool is_visual;                       /* Does this kind have visual editing? */

} member_funcs;


/* Helper class for generating joins */
typedef struct join
{
		int oid;
		int gid;
} join_t;

/* A default group-by */
static join_t *default_join;
#if 0
static int default_join_cmp(const void *a, const void *b)
{
		join_t *ja = &default_join[*(int*)a];
		join_t *jb = &default_join[*(int*)b];
		int c = ja->gid - jb->gid;
		if (c) return c;
		return ja->oid - jb->oid;
}
#endif
static int default_group(int oid) { return default_join[oid].gid; }


static int *obj_group_order;

/*
 * Description of each monster group.
 */
static struct
{
		cptr chars;
		cptr name;
} monster_group[] =
{
	{ (cptr)-1,	"Uniques" },
	{ "A",		"Angels" },
	{ "a",		"Ants" },
	{ "b",		"Bats" },
	{ "B",		"Birds" },
	{ "C",		"Canines" },
	{ "c",		"Centipedes" },
	{ "uU",		"Demons" },
	{ "dD",		"Dragons" },
	{ "vE",		"Elementals/Vortices" },
	{ "e",		"Eyes/Beholders" },
	{ "f",		"Felines" },
	{ "G",		"Ghosts" },
	{ "OP",		"Giants/Ogres" },
	{ "g",		"Golems" },
	{ "H",		"Harpies/Hybrids" },
	{ "h",		"Hominids (Elves, Dwarves)" },
	{ "H",		"Hydras" },
	{ "i",		"Icky Things" },
	{ "FI",		"Insects" },
	{ "j",		"Jellies" },
	{ "K",		"Killer Beetles" },
	{ "k",		"Kobolds" },
	{ "L",		"Lichs" },
	{ "tp",		"Men" },
	{ "$?!_",	"Mimics" },
	{ "m",		"Molds" },
	{ ",",		"Mushroom Patches" },
	{ "n",		"Nagas" },
	{ "o",		"Orcs" },
	{ "q",		"Quadrupeds" },
	{ "Q",		"Quylthulgs" },
	{ "R",		"Reptiles/Amphibians" },
	{ "r",		"Rodents" },
	{ "S",		"Scorpions/Spiders" },
	{ "s",		"Skeletons/Drujs" },
	{ "J",		"Snakes" },
	{ "T",		"Trolls" },
	{ "V",		"Vampires" },
	{ "W",		"Wights/Wraiths" },
	{ "w",		"Worms/Worm Masses" },
	{ "X",		"Xorns/Xarens" },
	{ "Y",		"Yeti" },
	{ "Z",		"Zephyr Hounds" },
	{ "z",		"Zombies" },
	{ NULL,		NULL }
};

/*
 * Description of each feature group.
 */
const char *feature_group_text[] = 
{
	"Floors",
	"Traps",
	"Doors",
	"Stairs",
	"Walls",
	"Streamers",
	"Obstructions",
	"Stores",
	"Other",
	NULL
};



/* Useful method declarations */
static void display_visual_list(int col, int row, int height, int width,
				byte attr_top, char char_left);

static bool visual_mode_command(event_type ke, bool *visual_list_ptr, 
				int height, int width, 
				byte *attr_top_ptr, char *char_left_ptr, 
				byte *cur_attr_ptr, char *cur_char_ptr,
				int col, int row, int *delay);

static void place_visual_list_cursor(int col, int row, byte a,
				byte c, byte attr_top, byte char_left);

static void dump_pref_file(void (*dump)(FILE*), const char *title, int row);

/*
 * Clipboard variables for copy&paste in visual mode
 */
static byte attr_idx = 0;
static byte char_idx = 0;

/*
 * Return a specific ordering for the features
 */
int feat_order(int feat)
{
	feature_type *f_ptr = &f_info[feat];

	switch (f_ptr->d_char)
	{
		case '.': 				return 0;
		case '^': 				return 1;
		case '\'': case '+': 	return 2;
		case '<': case '>':		return 3;
		case '#':				return 4;
		case '*': case '%' :	return 5;
		case ';': case ':' :	return 6;

		default:
		{
			if (isdigit(f_ptr->d_char)) return 7;
			return 8;
		}
	}
}


/* Emit a 'graphical' symbol and a padding character if appropriate */
static void big_pad(int col, int row, byte a, byte c)
{
	Term_putch(col, row, a, c);
	if (!use_bigtile) return;

	if (a & 0x80)
		Term_putch(col + 1, row, 255, -1);
	else
		Term_putch(col + 1, row, 1, ' ');
}

/* Return the actual width of a symbol */
static int actual_width(int width)
{
#ifdef UNANGBAND
	if (use_trptile) width *= 3;
	else if (use_dbltile) width *= 2;
#endif

	if (use_bigtile) width *= 2;

	return width;
}

/* Return the actual height of a symbol */
static int actual_height(int height)
{
#ifdef UNANGBAND
	if (use_trptile) height = height * 3 / 2;
	else if (use_dbltile) height *= 2;
#endif

	if (use_bigtile) height *= 2;

	return height;
}


/* From an actual width, return the logical width */
static int logical_width(int width)
{
	int div = 1;

#ifdef UNANGBAND
	if (use_trptile) div = 3;
	else if (use_dbltile) div = 2;
#endif

	if (use_bigtile) div *= 2;

	return width / div;
}

/* From an actual height, return the logical height */
static int logical_height(int height)
{
	int div = 1;

#ifdef UNANGBAND
	if (use_trptile)
	{
		height *= 2;
		div = 3;
	}
	else if (use_dbltile) div = 2;
#endif

	if (use_bigtile) div *= 2;

	return height / div;
}


static void display_group_member(menu_type *menu, int oid,
						bool cursor, int row, int col, int wid)
{
	const member_funcs *o_funcs = menu->menu_data;
	byte attr = curs_attrs[CURS_KNOWN][cursor == oid];

	/* Print the interesting part */
	o_funcs->display_member(col, row, cursor, oid);

#if 0 /* Debugging code */
	c_put_str(attr, format("%d", oid), row, 60);
#endif

	/* Do visual mode */
	if (o_funcs->is_visual && o_funcs->xattr)
	{
		char c = *o_funcs->xchar(oid);
		byte a = *o_funcs->xattr(oid);

		c_put_str(attr, format((c & 0x80) ? "%02x/%02x" : "%02x/%d", a, c), row, 60);
	}
}


#define swap(a, b) (swapspace = (void*)(a)), ((a) = (b)), ((b) = swapspace)

/*
 * Interactive group by. 
 * Recognises inscriptions, graphical symbols, lore
 */
static void display_knowledge(const char *title, int *obj_list, int o_count,
				group_funcs g_funcs, member_funcs o_funcs,
				const char *otherfields)
{
	/* maximum number of groups to display */
	int max_group = g_funcs.maxnum < o_count ? g_funcs.maxnum : o_count ;

	/* This could (should?) be (void **) */
	int *g_list, *g_offset;

	const char **g_names;

	int g_name_len = 8;  /* group name length, minumum is 8 */

	int grp_cnt = 0; /* total number groups */

	int g_cur = 0, grp_old = -1; /* group list positions */
	int o_cur = 0;					/* object list positions */
	int g_o_count = 0;				 /* object count for group */
	int oid = -1;  				/* object identifiers */

	region title_area = { 0, 0, 0, 4 };
	region group_region = { 0, 6, MISSING, -2 };
	region object_region = { MISSING, 6, 0, -2 };

	/* display state variables */
	bool visual_list = FALSE;
	byte attr_top = 0;
	char char_left = 0;

	int delay = 0;

	menu_type group_menu;
	menu_type object_menu;
	menu_iter object_iter;

	/* Panel state */
	/* These are swapped in parallel whenever the actively browsing " */
	/* changes */
	int *active_cursor = &g_cur, *inactive_cursor = &o_cur;
	menu_type *active_menu = &group_menu, *inactive_menu = &object_menu;
	int panel = 0;

	void *swapspace;
	bool do_swap = FALSE;

	bool flag = FALSE;
	bool redraw = TRUE;

	int browser_rows;
	int wid, hgt;
	int i;
	int prev_g = -1;

	int omode = rogue_like_commands;


	/* Get size */
	Term_get_size(&wid, &hgt);
	browser_rows = hgt - 8;

	/* Disable the roguelike commands for the duration */
	rogue_like_commands = FALSE;



	/* Do the group by. ang_sort only works on (void **) */
	/* Maybe should make this a precondition? */
	if (g_funcs.gcomp)
		qsort(obj_list, o_count, sizeof(*obj_list), g_funcs.gcomp);


	/* Sort everything into group order */
	C_MAKE(g_list, max_group + 1, int);
	C_MAKE(g_offset, max_group + 1, int);

	for (i = 0; i < o_count; i++)
	{
		if (prev_g != g_funcs.group(obj_list[i]))
		{
			prev_g = g_funcs.group(obj_list[i]);
			g_offset[grp_cnt] = i;
			g_list[grp_cnt++] = prev_g;
		}
	}

	g_offset[grp_cnt] = o_count;
	g_list[grp_cnt] = -1;


	/* The compact set of group names, in display order */
	C_MAKE(g_names, grp_cnt, const char **);

	for (i = 0; i < grp_cnt; i++)
	{
		int len;
		g_names[i] = g_funcs.name(g_list[i]);
		len = strlen(g_names[i]);
		if (len > g_name_len) g_name_len = len;
	}

	/* Reasonable max group name len */
	if (g_name_len >= 20) g_name_len = 20;

	object_region.col = g_name_len + 3;
	group_region.width = g_name_len;


	/* Leave room for the group summary information */
	if (g_funcs.summary) object_region.page_rows = -3;


	/* Set up the two menus */
	WIPE(&group_menu, menu_type);
	group_menu.count = grp_cnt;
	group_menu.cmd_keys = "\n\r6\x8C";  /* Ignore these as menu commands */
	group_menu.menu_data = g_names;

	WIPE(&object_menu, menu_type);
	object_menu.menu_data = &o_funcs;
	WIPE(&object_iter, object_iter);
	object_iter.display_row = display_group_member;

	o_funcs.is_visual = FALSE;

	menu_init(&group_menu, MN_SCROLL, MN_STRING, &group_region);
	menu_init2(&object_menu, find_menu_skin(MN_SCROLL), &object_iter, &object_region);


	/* This is the event loop for a multi-region panel */
	/* Panels are -- text panels, two menus, and visual browser */
	/* with "pop-up menu" for lore */
	while ((!flag) && (grp_cnt))
	{
		event_type ke, ke0;

		if (redraw)
		{
			/* Print the title bits */
			region_erase(&title_area);
			prt(format("Knowledge - %s", title), 2, 0);
			prt("Group", 4, 0);
			prt("Name", 4, g_name_len + 3);

			if (otherfields)
				prt(otherfields, 4, 55);


			/* Print dividers: horizontal and vertical */
			for (i = 0; i < 79; i++)
				Term_putch(i, 5, TERM_WHITE, '=');

			for (i = 0; i < browser_rows; i++)
				Term_putch(g_name_len + 1, 6 + i, TERM_WHITE, '|');


			/* Reset redraw flag */
			redraw = FALSE;
		}

		if (g_cur != grp_old)
		{
			grp_old = g_cur;
			o_cur = 0;
			g_o_count = g_offset[g_cur+1] - g_offset[g_cur];
			menu_set_filter(&object_menu, obj_list + g_offset[g_cur], g_o_count);
			group_menu.cursor = g_cur;
			object_menu.cursor = 0;
		}

		/* HACK ... */
		if (!visual_list)
		{
			/* ... The object menu may be browsing the entire group... */
			o_funcs.is_visual = FALSE;
			menu_set_filter(&object_menu, obj_list + g_offset[g_cur], g_o_count);
			object_menu.cursor = o_cur;
		}
		else
		{
			/* ... or just a single element in the group. */
			o_funcs.is_visual = TRUE;
			menu_set_filter(&object_menu, obj_list + o_cur + g_offset[g_cur], 1);
			object_menu.cursor = 0;
		}

		oid = obj_list[g_offset[g_cur]+o_cur];

		/* Print prompt */
		{
			const char *pedit = (!o_funcs.xattr) ? "" :
					(!(attr_idx|char_idx) ? ", 'c' to copy" : ", 'c', 'p' to paste");
			const char *xtra = o_funcs.xtra_prompt ? o_funcs.xtra_prompt(oid) : "";
			const char *pvs = "";

			if (visual_list) pvs = ", ENTER to accept";
			else if (o_funcs.xattr) pvs = ", 'v' for visuals";

			prt(format("<dir>, 'r' to recall%s%s%s, ESC", pvs, pedit, xtra), hgt - 1, 0);
		}

		if (do_swap)
		{
			do_swap = FALSE;
			swap(active_menu, inactive_menu);
			swap(active_cursor, inactive_cursor);
			panel = 1 - panel;
		}

		if (g_funcs.summary && !visual_list)
		{
			g_funcs.summary(g_cur, obj_list, g_o_count, g_offset[g_cur],
			                object_menu.boundary.row + object_menu.boundary.page_rows,
			                object_region.col);
		}

		menu_refresh(inactive_menu);
		menu_refresh(active_menu);

		handle_stuff();

		if (visual_list)
		{
			display_visual_list(g_name_len + 3, 7, browser_rows-1,
			                             wid - (g_name_len + 3), attr_top, char_left);
			place_visual_list_cursor(g_name_len + 3, 7, *o_funcs.xattr(oid), 
										*o_funcs.xchar(oid), attr_top, char_left);
		}

		if (delay)
		{
			/* Force screen update */
			Term_fresh();

			/* Delay */
			Term_xtra(TERM_XTRA_DELAY, delay);

			delay = 0;
		}

		ke = inkey_ex();

		/* Do visual mode command if needed */
		if (o_funcs.xattr && o_funcs.xchar &&
					visual_mode_command(ke, &visual_list,
					browser_rows-1, wid - (g_name_len + 3),
					&attr_top, &char_left,
					o_funcs.xattr(oid), o_funcs.xchar(oid),
					g_name_len + 3, 7, &delay))
		{
			continue;
		}

		if (ke.type == EVT_MOUSE)
		{
			/* Change active panels */
			if (region_inside(&inactive_menu->boundary, &ke))
			{
				swap(active_menu, inactive_menu);
				swap(active_cursor, inactive_cursor);
				panel = 1-panel;
			}
		}

		ke0 = run_event_loop(&active_menu->target, 0, &ke);
		if (ke0.type != EVT_AGAIN) ke = ke0;

		switch (ke.type)
		{
			case EVT_KBRD:
			{
				break;
			}

			case ESCAPE:
			{
				flag = TRUE;
				continue;
			}

			case EVT_SELECT:
			{
				if (panel == 1 && oid >= 0 && o_cur == active_menu->cursor)
				{
					o_funcs.lore(oid);
					redraw = TRUE;
				}
			}

			case EVT_MOVE:
			{
				*active_cursor = active_menu->cursor;
				continue;
			}

			case EVT_BACK:
			{
				if (panel == 1)
					do_swap = TRUE;
			}

			/* XXX Handle EVT_RESIZE */

			default:
			{
				continue;
			}
		}

		switch (ke.key)
		{
			case ESCAPE:
			{
				flag = TRUE;
				break;
			}

			case 'R':
			case 'r':
			{
				/* Recall on screen */
				if (oid >= 0)
					o_funcs.lore(oid);

				redraw = TRUE;
				break;
			}

			default:
			{
				int d = target_dir(ke.key);

				/* Handle key-driven motion between panels */
				if (ddx[d] && ((ddx[d] < 0) == (panel == 1)))
				{
					/* Silly hack -- diagonal arithmetic */
					*inactive_cursor += ddy[d];
					if (*inactive_cursor < 0) *inactive_cursor = 0;
					else if (g_cur >= grp_cnt) g_cur = grp_cnt -1;
					else if (o_cur >= g_o_count) o_cur = g_o_count-1;
					do_swap = TRUE;
				}
				else if (o_funcs.xtra_act)
				{
					o_funcs.xtra_act(ke.key, oid);
				}

				break;
			}
		}
	}

	/* Restore roguelike option */
	rogue_like_commands = omode;

	/* Prompt */
	if (!grp_cnt)
		prt(format("No %s known.", title), 15, 0);

	FREE(g_names);
	FREE(g_offset);
	FREE(g_list);
}

/*
 * Display visuals.
 */
static void display_visual_list(int col, int row, int height, int width, byte attr_top, char char_left)
{
	int i, j;

	/* Clear the display lines */
	for (i = 0; i < height; i++)
			Term_erase(col, row + i, width);

	width = logical_width(width);

	/* Display lines until done */
	for (i = 0; i < height; i++)
	{
		/* Display columns until done */
		for (j = 0; j < width; j++)
		{
			byte a;
			char c;
			int x = col + actual_width(j);
			int y = row + actual_width(i);
			int ia, ic;

			ia = attr_top + i;
			ic = char_left + j;

			a = (byte)ia;
			c = (char)ic;

			/* Display symbol */
			big_pad(x, y, a, c);
		}
	}
}


/*
 * Place the cursor at the collect position for visual mode
 */
static void place_visual_list_cursor(int col, int row, byte a, byte c, byte attr_top, byte char_left)
{
	int i = a - attr_top;
	int j = c - char_left;

	int x = col + actual_width(j);
	int y = row + actual_height(i);

	/* Place the cursor */
	Term_gotoxy(x, y);
}


/*
 *  Do visual mode command -- Change symbols
 */
static bool visual_mode_command(event_type ke, bool *visual_list_ptr, 
				int height, int width, 
				byte *attr_top_ptr, char *char_left_ptr, 
				byte *cur_attr_ptr, char *cur_char_ptr,
				int col, int row, int *delay)
{
	static byte attr_old = 0;
	static char char_old = 0;

	int frame_left = logical_width(10);
	int frame_right = logical_width(10);
	int frame_top = logical_height(4);
	int frame_bottom = logical_height(4);

	switch (ke.key)
	{
		case ESCAPE:
		{
			if (*visual_list_ptr)
			{
				/* Cancel change */
				*cur_attr_ptr = attr_old;
				*cur_char_ptr = char_old;
				*visual_list_ptr = FALSE;

				return TRUE;
			}

			break;
		}

		case '\n':
		case '\r':
		{
			if (*visual_list_ptr)
			{
				/* Accept change */
				*visual_list_ptr = FALSE;
				return TRUE;
			}

			break;
		}

		case 'V':
		case 'v':
		{
			if (!*visual_list_ptr)
			{
				*visual_list_ptr = TRUE;

				*attr_top_ptr = (byte)MAX(0, (int)*cur_attr_ptr - frame_top);
				*char_left_ptr = (char)MAX(-128, (int)*cur_char_ptr - frame_left);

				attr_old = *cur_attr_ptr;
				char_old = *cur_char_ptr;
			}
			else
			{
				/* Cancel change */
				*cur_attr_ptr = attr_old;
				*cur_char_ptr = char_old;
				*visual_list_ptr = FALSE;
			}

			return TRUE;
		}

		case 'C':
		case 'c':
		{
			/* Set the visual */
			attr_idx = *cur_attr_ptr;
			char_idx = *cur_char_ptr;

			return TRUE;
		}

		case 'P':
		case 'p':
		{
			if (attr_idx)
			{
				/* Set the char */
				*cur_attr_ptr = attr_idx;
				*attr_top_ptr = (byte)MAX(0, (int)*cur_attr_ptr - frame_top);
			}

			if (char_idx)
			{
				/* Set the char */
				*cur_char_ptr = char_idx;
				*char_left_ptr = (char)MAX(-128, (int)*cur_char_ptr - frame_left);
			}

			return TRUE;
		}

		default:
		{
			if (*visual_list_ptr)
			{
				int eff_width = actual_width(width);
				int eff_height = actual_height(height);
				int d = target_dir(ke.key);
				byte a = *cur_attr_ptr;
				char c = *cur_char_ptr;

				/* Get mouse movement */
				if (ke.key == '\xff')
				{
					int my = ke.mousey - row;
					int mx = ke.mousex - col;

					my = logical_height(my);
					mx = logical_width(mx);

					if ((my >= 0) && (my < eff_height) && (mx >= 0) && (mx < eff_width)
						&& ((ke.index) || (a != *attr_top_ptr + my)
							|| (c != *char_left_ptr + mx)))
					{
						/* Set the visual */
						*cur_attr_ptr = a = *attr_top_ptr + my;
						*cur_char_ptr = c = *char_left_ptr + mx;

						/* Move the frame */
						if (*char_left_ptr > MAX(-128, (int)c - frame_left))
							(*char_left_ptr)--;
						if (*char_left_ptr + eff_width < MIN(127, (int)c + frame_right))
							(*char_left_ptr)++;
						if (*attr_top_ptr > MAX(0, (int)a - frame_top))
							(*attr_top_ptr)--;
						if (*attr_top_ptr + eff_height < MIN(255, (int)a + frame_bottom))
							(*attr_top_ptr)++;

						/* Delay */
						*delay = 100;

						/* Accept change */
						if (ke.index) *visual_list_ptr = FALSE;

						return TRUE;
					}

					/* Cancel change */
					else if (ke.index)
					{
						*cur_attr_ptr = attr_old;
						*cur_char_ptr = char_old;
						*visual_list_ptr = FALSE;

						return TRUE;
					}
				}
				else
				{
					/* Restrict direction */
					if ((a == 0) && (ddy[d] < 0)) d = 0;
					if ((c == (char) -128) && (ddx[d] < 0)) d = 0;
					if ((a == 255) && (ddy[d] > 0)) d = 0;
					if ((c == 127) && (ddx[d] > 0)) d = 0;

					a += ddy[d];
					c += ddx[d];

					/* Set the visual */
					*cur_attr_ptr = a;
					*cur_char_ptr = c;

					/* Move the frame */
					if ((ddx[d] < 0) && *char_left_ptr > MAX(-128, (int)c - frame_left))
						(*char_left_ptr)--;
					if ((ddx[d] > 0) && *char_left_ptr + eff_width <
														MIN(127, (int)c + frame_right))
					(*char_left_ptr)++;

					if ((ddy[d] < 0) && *attr_top_ptr > MAX(0, (int)a - frame_top))
						(*attr_top_ptr)--;
					if ((ddy[d] > 0) && *attr_top_ptr + eff_height <
													MIN(255, (int)a + frame_bottom))
						(*attr_top_ptr)++;

					if (d != 0) return TRUE;
				}
			}
		}
	}

	/* Visual mode command is not used */
	return FALSE;
}


/* The following sections implement "subclasses" of the
 * abstract classes represented by member_funcs and group_funcs
 */

/* =================== MONSTERS ==================================== */
/* Many-to-many grouping - use default auxiliary join */

/*
 * Display a monster
 */
static void display_monster(int col, int row, bool cursor, int oid)
{
	/* HACK Get the race index. (Should be a wrapper function) */
	int r_idx = default_join[oid].oid;

	/* Access the race */
	monster_race *r_ptr = &r_info[r_idx];
	monster_lore *l_ptr = &l_list[r_idx];

	/* Choose colors */
	byte attr = curs_attrs[CURS_KNOWN][(int)cursor];
	byte a = r_ptr->x_attr;
	byte c = r_ptr->x_char;

	/* Display the name */
	c_prt(attr, r_name + r_ptr->name, row, col);

#ifdef UNANGBAND
	if (use_dbltile || use_trptile)
		return;
#endif

	/* Display symbol */
	big_pad(66, row, a, c);

	/* Display kills */
	if (r_ptr->flags1 & (RF1_UNIQUE))
		put_str(format("%s", (r_ptr->max_num == 0)?  " dead" : "alive"), row, 70);
	else
		put_str(format("%5d", l_ptr->pkills), row, 70);
}


static int m_cmp_race(const void *a, const void *b)
{
	monster_race *r_a = &r_info[default_join[*(int*)a].oid];
	monster_race *r_b = &r_info[default_join[*(int*)b].oid];
	int gid = default_join[*(int*)a].gid;

	/* Group by */
	int c = gid - default_join[*(int*)b].gid;
	if (c) return c;

	/* Order results */
	c = r_a->d_char - r_b->d_char;
	if (c && gid != 0)
	{
		/* UNIQUE group is ordered by level & name only */
		/* Others by order they appear in the group symbols */
		return strchr(monster_group[gid].chars, r_a->d_char)
			- strchr(monster_group[gid].chars, r_b->d_char);
	}
	c = r_a->level - r_b->level;
	if (c) return c;

	return strcmp(r_name + r_a->name, r_name + r_b->name);
}

static char *m_xchar(int oid) { return &r_info[default_join[oid].oid].x_char; }
static byte *m_xattr(int oid) { return &r_info[default_join[oid].oid].x_attr; }
static const char *race_name(int gid) { return monster_group[gid].name; }
static void mon_lore(int oid) { screen_roff(default_join[oid].oid); inkey_ex(); }

static void mon_summary(int gid, const int *object_list, int n, int top, int row, int col)
{
	int i;
	int kills = 0;

	/* Access the race */
	
	for (i = 0; i < n; i++)
	{
		int oid = default_join[object_list[i+top]].oid;
		kills += l_list[oid].pkills;
	}

	/* Different display for the first item if we've got uniques to show */
	if (gid == 0 && ((&r_info[default_join[object_list[0]].oid])->flags1 & (RF1_UNIQUE)))
	{
		c_prt(TERM_L_BLUE, format("%d known uniques, %d slain.", n, kills),
					row, col);
	}
	else
	{
		int tkills = 0;

		for (i = 0; i < z_info->r_max; i++) 
			tkills += l_list[i].pkills;

		c_prt(TERM_L_BLUE, format("Creatures slain: %d/%d (in group/in total)", kills, tkills), row, col);
	}
}

static int count_known_monsters(void)
{
	int m_count = 0;
	int i;
	size_t j;

	for (i = 0; i < z_info->r_max; i++)
	{
		monster_race *r_ptr = &r_info[i];
		if (!cheat_know && !l_list[i].sights) continue;
		if (!r_ptr->name) continue;

		if (r_ptr->flags1 & RF1_UNIQUE) m_count++;

		for (j = 1; j < N_ELEMENTS(monster_group) - 1; j++)
		{
			const char *pat = monster_group[j].chars;
			if (strchr(pat, r_ptr->d_char)) m_count++;
		}
	}

	return m_count;
}

/*
 * Display known monsters.
 */
static void do_cmd_knowledge_monsters(void *obj, const char *name)
{
	group_funcs r_funcs = {N_ELEMENTS(monster_group), FALSE, race_name,
							m_cmp_race, default_group, mon_summary};

	member_funcs m_funcs = {display_monster, mon_lore, m_xchar, m_xattr, 0, 0, 0};

	
	int *monsters;
	int m_count = 0;
	int i;
	size_t j;

	for (i = 0; i < z_info->r_max; i++)
	{
		monster_race *r_ptr = &r_info[i];
		if (!cheat_know && !l_list[i].sights) continue;
		if (!r_ptr->name) continue;

		if (r_ptr->flags1 & RF1_UNIQUE) m_count++;

		for (j = 1; j < N_ELEMENTS(monster_group) - 1; j++)
		{
			const char *pat = monster_group[j].chars;
			if (strchr(pat, r_ptr->d_char)) m_count++;
		}
	}

	C_MAKE(default_join, m_count, join_t);
	C_MAKE(monsters, m_count, int);

	m_count = 0;
	for (i = 0; i < z_info->r_max; i++)
	{
		monster_race *r_ptr = &r_info[i];
		if (!cheat_know && !l_list[i].sights) continue;
		if (!r_ptr->name) continue;
	
		for (j = 0; j < N_ELEMENTS(monster_group)-1; j++)
		{
			const char *pat = monster_group[j].chars;
			if (j == 0 && !(r_ptr->flags1 & RF1_UNIQUE)) 
				continue;
			else if (j > 0 && !strchr(pat, r_ptr->d_char))
				continue;

			monsters[m_count] = m_count;
			default_join[m_count].oid = i;
			default_join[m_count++].gid = j;
		}
	}

	display_knowledge("monsters", monsters, m_count, r_funcs, m_funcs,
						"          Sym  Kills");
	KILL(default_join);
	FREE(monsters);
}

/* =================== ARTIFACTS ==================================== */
/* Many-to-one grouping */

/*
 * Display an artifact label
 */
static void display_artifact(int col, int row, bool cursor, int oid)
{
	char o_name[80];
	object_type object_type_body;
	object_type *o_ptr = &object_type_body;

	/* Choose a color */
	byte attr = curs_attrs[CURS_KNOWN][(int)cursor];

	/* Get local object */
	o_ptr = &object_type_body;

	/* Wipe the object */
	object_wipe(o_ptr);

	/* Make fake artifact */
	make_fake_artifact(o_ptr, oid);

	/* Get its name */
	object_desc_spoil(o_name, sizeof(o_name), o_ptr, TRUE, 0);

	/* Display the name */
	c_prt(attr, o_name, row, col);
}

/*
 * Show artifact lore
 */
static void desc_art_fake(int a_idx)
{
	object_type *o_ptr;
	object_type object_type_body;

	/* Get local object */
	o_ptr = &object_type_body;

	/* Wipe the object */
	object_wipe(o_ptr);

	/* Make fake artifact */
	make_fake_artifact(o_ptr, a_idx);
	o_ptr->ident |= (IDENT_STORE | IDENT_KNOWN);
	if (cheat_xtra) o_ptr->ident |= IDENT_MENTAL;

	/* Hack -- Handle stuff */
	handle_stuff();

	/* Save the screen */
	screen_save();

	object_info_screen(o_ptr);

	/* Load the screen */
	screen_load();
}

static int a_cmp_tval(const void *a, const void *b)
{
	artifact_type *a_a = &a_info[*(int*)a];
	artifact_type *a_b = &a_info[*(int*)b];
	/*group by */
	int ta = obj_group_order[a_a->tval];
	int tb = obj_group_order[a_b->tval];
	int c = ta - tb;
	if (c) return c;

	/* order by */
	c = a_a->sval - a_b->sval;
	if (c) return c;
	return strcmp(a_name+a_a->name, a_name+a_b->name);
}

static const char *kind_name(int gid) { return object_text_order[gid].name; }
static int art2gid(int oid) { return obj_group_order[a_info[oid].tval]; }


/* If 'artifacts' is NULL, it counts the number of known artifacts, otherwise
   it collects the list of known artifacts into 'artifacts' as well. */
static int collect_known_artifacts(int *artifacts, size_t artifacts_len)
{
	int a_count = 0;
	int i, j;

	if (artifacts)
		assert(artifacts_len >= z_info->a_max);

	for (j = 0; j < z_info->a_max; j++)
	{
		/* If the artifact has been created (or we're cheating) */
		if ((cheat_xtra || a_info[j].cur_num) && a_info[j].name)
		{
			bool valid = TRUE;

			for (i = 0; !cheat_xtra && i < z_info->o_max; i++)
			{
				int a = o_list[i].name1;

				/* If we haven't actually identified the artifact yet */
				if (a && a == j && !object_known_p(&o_list[i]))
				{
					valid = FALSE;
				}
			}

			if (valid)
			{
				if (artifacts)
					artifacts[a_count++] = j;
				else
					a_count++;
			}
		}
	}

	return a_count;
}

/*
 * Display known artifacts
 */
static void do_cmd_knowledge_artifacts(void *obj, const char *name)
{
	/* HACK -- should be TV_MAX */
	group_funcs obj_f = {TV_GOLD, FALSE, kind_name, a_cmp_tval, art2gid, 0};
	member_funcs art_f = {display_artifact, desc_art_fake, 0, 0, 0, 0, 0};

	int *artifacts;
	int a_count = 0;

	C_MAKE(artifacts, z_info->a_max, int);

	/* Collect valid artifacts */
	a_count = collect_known_artifacts(artifacts, z_info->a_max);

	display_knowledge("artifacts", artifacts, a_count, obj_f, art_f, 0);
	FREE(artifacts);
}

/* =================== EGO ITEMS  ==================================== */
/* Many-to-many grouping (uses default join) */

/* static u16b *e_note(int oid) {return &e_info[default_join[oid].oid].note;} */
static const char *ego_grp_name(int gid) { return object_text_order[gid].name; }

static void display_ego_item(int col, int row, bool cursor, int oid)
{
	/* HACK: Access the object */
	ego_item_type *e_ptr = &e_info[default_join[oid].oid];

	/* Choose a color */
	byte attr = curs_attrs[0 != (int)e_ptr->everseen][0 != (int)cursor];

	/* Display the name */
	c_prt(attr, e_name + e_ptr->name, row, col);
}

/*
 * Describe fake ego item "lore"
 */
static void desc_ego_fake(int oid)
{
	/* Hack: dereference the join */
	const char *cursed[] = { "permanently cursed", "heavily cursed", "cursed" };
	const char *xtra[] = { "sustain", "higher resistance", "ability" };
	int f3, i;

	int e_idx = default_join[oid].oid;
	ego_item_type *e_ptr = &e_info[e_idx];

	object_type dummy;
	WIPE(&dummy, dummy);

	/* Save screen */
	screen_save();

	/* Set text_out hook */
	text_out_hook = text_out_to_screen;

	/* Dump the name */
	c_prt(TERM_L_BLUE, format("%s %s", ego_grp_name(default_group(oid)),
	                                   e_name + e_ptr->name), 0, 0);

	/* Begin recall */
	Term_gotoxy(0, 1);
	if (e_ptr->text)
	{
		int x, y;
		text_out(e_text + e_ptr->text);
		Term_locate(&x, &y);
		Term_gotoxy(0, y+1);
	}

	/* List ego flags */
	dummy.name2 = e_idx;
	object_info_out_flags = object_flags;
	object_info_out(&dummy);

	if (e_ptr->xtra)
		text_out(format("It provides one random %s.", xtra[e_ptr->xtra - 1]));

	for (i = 0, f3 = TR3_PERMA_CURSE; i < 3 ; f3 >>= 1, i++)
	{
		if (e_ptr->flags3 & f3)
		{
			text_out_c(TERM_RED, format("It is %s.", cursed[i]));
			break;
		}
	}

	Term_flush();

	(void)inkey_ex();

	screen_load();
}

/* TODO? Currently ego items will order by e_idx */
static int e_cmp_tval(const void *a, const void *b)
{
	ego_item_type *ea = &e_info[default_join[*(int*)a].oid];
	ego_item_type *eb = &e_info[default_join[*(int*)b].oid];

	/* Group by */
	int c = default_join[*(int*)a].gid - default_join[*(int*)b].gid;
	if (c) return c;

	/* Order by */
	return strcmp(e_name + ea->name, e_name + eb->name);
}

/*
 * Display known ego_items
 */
static void do_cmd_knowledge_ego_items(void *obj, const char *name)
{
	group_funcs obj_f =
		{TV_GOLD, FALSE, ego_grp_name, e_cmp_tval, default_group, 0};

	member_funcs ego_f = {display_ego_item, desc_ego_fake, 0, 0, 0, 0, 0};

	int *egoitems;
	int e_count = 0;
	int i, j;

	/* HACK: currently no more than 3 tvals for one ego type */
	C_MAKE(egoitems, z_info->e_max * EGO_TVALS_MAX, int);
	C_MAKE(default_join, z_info->e_max * EGO_TVALS_MAX, join_t);

	for (i = 0; i < z_info->e_max; i++)
	{
		if (e_info[i].everseen || cheat_xtra)
		{
			for (j = 0; j < EGO_TVALS_MAX && e_info[i].tval[j]; j++)
			{
				int gid = obj_group_order[e_info[i].tval[j]];

				/* Ignore duplicate gids */
				if (j > 0 && gid == default_join[e_count - 1].gid) continue;

				egoitems[e_count] = e_count;
				default_join[e_count].oid = i;
				default_join[e_count++].gid = gid; 
			}
		}
	}

	display_knowledge("ego items", egoitems, e_count, obj_f, ego_f, "");

	KILL(default_join);
	FREE(egoitems);
}

/* =================== ORDINARY OBJECTS  ==================================== */
/* Many-to-one grouping */

/*
 * Display the objects in a group.
 */
static void display_object(int col, int row, bool cursor, int oid)
{
	int k_idx = oid;
	
	object_kind *k_ptr = &k_info[k_idx];
	const char *inscrip = get_autoinscription(oid);

	char o_name[80];

	/* Choose a color */
	bool aware = (k_ptr->flavor == 0) || (k_ptr->aware);
	byte a = (aware && k_ptr->x_attr) ?
				k_ptr->x_attr : flavor_info[k_ptr->flavor].x_attr;
	byte c = aware ? k_ptr->x_char : flavor_info[k_ptr->flavor].x_char;
	byte attr = curs_attrs[(int)k_ptr->flavor == 0 || k_ptr->aware][(int)cursor];

	/* Symbol is unknown.  This should never happen.*/	
	if (!k_ptr->aware && !k_ptr->flavor && !p_ptr->wizard)
	{
		assert(0);
		c = ' ';
		a = TERM_DARK;
	}

	/* Tidy name */
        object_kind_name(o_name, sizeof o_name, k_idx, cheat_know);

	/* Display the name */
	c_prt(attr, o_name, row, col);

	/* Show autoinscription if around */
	if (aware && inscrip)
		c_put_str(TERM_YELLOW, inscrip, row, 55);

#ifdef UNANGBAND
	/* Hack - don't use if double tile */
	if (use_dbltile || use_trptile)
		return;
#endif

	/* Display symbol */
	big_pad(76, row, a, c);
}

/*
 * Describe fake object
 */
static void desc_obj_fake(int k_idx)
{
	object_type object_type_body;
	object_type *o_ptr = &object_type_body;

	/* Wipe the object */
	object_wipe(o_ptr);

	/* Create the artifact */
	object_prep(o_ptr, k_idx);

	/* Hack -- its in the store */
	if (k_info[k_idx].aware) o_ptr->ident |= (IDENT_STORE);

	/* It's fully know */
	if (!k_info[k_idx].flavor) object_known(o_ptr);

	/* Hack -- Handle stuff */
	handle_stuff();

	/* Save the screen */
	screen_save();

	/* Describe */
	Term_gotoxy(0,0);
	object_info_screen(o_ptr);

	/* Load the screen */
	screen_load();
}

static int o_cmp_tval(const void *a, const void *b)
{
	object_kind *k_a = &k_info[*(int*)a];
	object_kind *k_b = &k_info[*(int*)b];

	/* Group by */
	int ta = obj_group_order[k_a->tval];
	int tb = obj_group_order[k_b->tval];
	int c = ta - tb;
	if (c) return c;

	/* Order by */
	c = k_a->aware - k_b->aware;
	if (c) return -c; /* aware has low sort weight */
	if (!k_a->aware)
	{
		return strcmp(flavor_text + flavor_info[k_a->flavor].text,
									flavor_text +flavor_info[k_b->flavor].text);
	}
	c = k_a->cost - k_b->cost;
	if (c) return c;

	return strcmp(k_name + k_a->name, k_name + k_b->name);
}

static int obj2gid(int oid) { return obj_group_order[k_info[oid].tval]; }

static char *o_xchar(int oid)
{
	object_kind *k_ptr = &k_info[oid];

	if (!k_ptr->flavor || k_ptr->aware)
		return &k_ptr->x_char;
	else
		return &flavor_info[k_ptr->flavor].x_char;
}

static byte *o_xattr(int oid)
{
	object_kind *k_ptr = &k_info[oid];

	if (!k_ptr->flavor || k_ptr->aware)
		return &k_ptr->x_attr;
	else
		return &flavor_info[k_ptr->flavor].x_attr;
}

/*
 * Display special prompt for object inscription.
 */
static const char *o_xtra_prompt(int oid)
{
	object_kind *k_ptr = &k_info[oid];
	s16b idx = get_autoinscription_index(oid);

	const char *no_insc = ", '{'";
	const char *with_insc = ", '{', '}'";


	/* Forget it if we've never seen the thing */
	if (k_ptr->flavor && !k_ptr->aware)
		return "";

	/* If it's already inscribed */
	if (idx != -1)
		return with_insc;

	return no_insc;
}

/*
 * Special key actions for object inscription.
 */
static void o_xtra_act(char ch, int oid)
{
	object_kind *k_ptr = &k_info[oid];
	s16b idx = get_autoinscription_index(oid);

	/* Forget it if we've never seen the thing */
	if (k_ptr->flavor && !k_ptr->aware)
		return;

	/* Uninscribe */
	if (ch == '}')
	{
		if (idx) remove_autoinscription(oid);
		return;
	}

	/* Inscribe */
	else if (ch == '{')
	{
		char note_text[80] = "";

		/* Avoid the prompt getting in the way */
		screen_save();

		/* Prompt */
		prt("Inscribe with: ", 0, 0);

		/* Default note */
		if (idx != -1)
			strnfmt(note_text, sizeof(note_text), "%s", get_autoinscription(oid));

		/* Get an inscription */
		if (askfor_aux(note_text, sizeof(note_text), NULL))
		{
			/* Remove old inscription if existent */
			if (idx != -1)
				remove_autoinscription(oid);

			/* Add the autoinscription */
			add_autoinscription(oid, note_text);

			/* Notice stuff (later) */
			p_ptr->notice |= (PN_AUTOINSCRIBE);
			p_ptr->window |= (PW_INVEN | PW_EQUIP);
		}

		/* Reload the screen */
		screen_load();
	}
}



/*
 * Display known objects
 */
static void do_cmd_knowledge_objects(void *obj, const char *name)
{
	group_funcs kind_f = {TV_GOLD, FALSE, kind_name, o_cmp_tval, obj2gid, 0};
	member_funcs obj_f = {display_object, desc_obj_fake, o_xchar, o_xattr, o_xtra_prompt, o_xtra_act, 0};

	int *objects;
	int o_count = 0;
	int i;

	C_MAKE(objects, z_info->k_max, int);

	for (i = 0; i < z_info->k_max; i++)
	{
		if (k_info[i].everseen || k_info[i].flavor || cheat_xtra)
		{
			int c = obj_group_order[k_info[i].tval];
			if (c >= 0) objects[o_count++] = i;
		}
	}

	display_knowledge("known objects", objects, o_count, kind_f, obj_f, "Inscribed          Sym");

	FREE(objects);
}

/* =================== TERRAIN FEATURES ==================================== */
/* Many-to-one grouping */

/*
 * Display the features in a group.
 */
static void display_feature(int col, int row, bool cursor, int oid )
{
	/* Get the feature index */
	int f_idx = oid;

	/* Access the feature */
	feature_type *f_ptr = &f_info[f_idx];

	/* Choose a color */
	byte attr = curs_attrs[CURS_KNOWN][(int)cursor];

	/* Display the name */
	c_prt(attr, f_name + f_ptr->name, row, col);

#ifdef UNANGBAND
	if (use_dbltile || use_trptile) return;
#endif

	/* Display symbol */
	big_pad(68, row, f_ptr->x_attr, f_ptr->x_char);

	/* ILLUMINATION AND DARKNESS GO HERE */

}


static int f_cmp_fkind(const void *a, const void *b)
{
	feature_type *fa = &f_info[*(int*)a];
	feature_type *fb = &f_info[*(int*)b];
	/* group by */
	int c = feat_order(*(int*)a) - feat_order(*(int*)b);
	if (c) return c;
	/* order by feature name */
	return strcmp(f_name + fa->name, f_name + fb->name);
}

static const char *fkind_name(int gid) { return feature_group_text[gid]; }
static byte *f_xattr(int oid) { return &f_info[oid].x_attr; }
static char *f_xchar(int oid) { return &f_info[oid].x_char; }
static void feat_lore(int oid) { /* noop */ }

/*
 * Interact with feature visuals.
 */
static void do_cmd_knowledge_features(void *obj, const char *name)
{
	group_funcs fkind_f = {N_ELEMENTS(feature_group_text), FALSE,
							fkind_name, f_cmp_fkind, feat_order, 0};

	member_funcs feat_f = {display_feature, feat_lore, f_xchar, f_xattr, 0, 0, 0};

	int *features;
	int f_count = 0;
	int i;
	C_MAKE(features, z_info->f_max, int);

	for (i = 0; i < z_info->f_max; i++)
	{
		if (f_info[i].name == 0) continue;
		features[f_count++] = i; /* Currently no filter for features */
	}

	display_knowledge("features", features, f_count, fkind_f, feat_f, "           Sym");
	FREE(features);
}

/* =================== HOMES AND STORES ==================================== */



void do_cmd_knowledge_home() 
{
	/* TODO */
}


/* =================== END JOIN DEFINITIONS ================================ */


/*
 * Hack -- redraw the screen
 *
 * This command performs various low level updates, clears all the "extra"
 * windows, does a total redraw of the main window, and requests all of the
 * interesting updates and redraws that I can think of.
 *
 * This command is also used to "instantiate" the results of the user
 * selecting various things, such as graphics mode, so it must call
 * the "TERM_XTRA_REACT" hook before redrawing the windows.
 *
 */
void do_cmd_redraw(void)
{
	int j;

	term *old = Term;


	/* Low level flush */
	Term_flush();

	/* Reset "inkey()" */
	flush();
	
	if (character_dungeon)
		verify_panel();


	/* Hack -- React to changes */
	Term_xtra(TERM_XTRA_REACT, 0);


	/* Combine and Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);


	/* Update torch */
	p_ptr->update |= (PU_TORCH);

	/* Update stuff */
	p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

	/* Fully update the visuals */
	p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);

	/* Redraw everything */
	p_ptr->redraw |= (PR_BASIC | PR_EXTRA | PR_MAP | PR_EQUIPPY);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1 |
	                  PW_MESSAGE | PW_OVERHEAD | PW_MONSTER | PW_OBJECT |
	                  PW_MAP | PW_MONLIST);

	/* Clear screen */
	Term_clear();

	/* Hack -- update */
	handle_stuff();

	/* Place the cursor on the player */
	if (0 != character_dungeon)
		move_cursor_relative(p_ptr->px, p_ptr->py);


	/* Redraw every window */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		/* Dead window */
		if (!angband_term[j]) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* Redraw */
		Term_redraw();

		/* Refresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Hack -- change name
 */
void do_cmd_change_name(void)
{
	event_type ke;

	int mode = 0;

	cptr p;

	/* Prompt */
	p = "['c' to change name, 'f' to file, 'h' to change mode, or ESC]";

	/* Save screen */
	screen_save();

	/* Forever */
	while (1)
	{
		/* Display the player */
		display_player(mode);

		/* Prompt */
		Term_putstr(2, 23, -1, TERM_WHITE, p);

		/* Query */
		ke = inkey_ex();

		/* Exit */
		if (ke.key == ESCAPE) break;

		/* Change name */
		if ((ke.key == 'c') || ((ke.key == '\xff') && (ke.mousey == 2) && (ke.mousex < 26)))
		{
			get_name(FALSE);
		}

		/* File dump */
		else if (ke.key == 'f')
		{
			char ftmp[80];

			strnfmt(ftmp, sizeof ftmp, "%s.txt", op_ptr->base_name);

			if (get_string("File name: ", ftmp, 80))
			{
				if (ftmp[0] && (ftmp[0] != ' '))
				{
					if (file_character(ftmp, FALSE))
						msg_print("Character dump failed!");
					else
						msg_print("Character dump successful.");
				}
			}
		}

		/* Toggle mode */
		else if ((ke.key == 'h') || (ke.key == '\xff') ||
		         (ke.key == ARROW_LEFT) || (ke.key == ' '))
		{
			mode = (mode + 1) % INFO_SCREENS;
		}

		/* Toggle mode */
		else if ((ke.key == 'l') || ke.key == ARROW_RIGHT)
		{
			mode = (mode - 1) % INFO_SCREENS;
		}


		/* Oops */
		else
		{
			bell(NULL);
		}

		/* Flush messages */
		message_flush();
	}

	/* Load screen */
	screen_load();
}


/*
 * Recall the most recent message
 */
void do_cmd_message_one(void)
{
	/* Recall one message XXX XXX XXX */
	c_prt(message_color(0), format( "> %s", message_str(0)), 0, 0);
}


/*
 * Show previous messages to the user
 *
 * The screen format uses line 0 and 23 for headers and prompts,
 * skips line 1 and 22, and uses line 2 thru 21 for old messages.
 *
 * This command shows you which commands you are viewing, and allows
 * you to "search" for strings in the recall.
 *
 * Note that messages may be longer than 80 characters, but they are
 * displayed using "infinite" length, with a special sub-command to
 * "slide" the virtual display to the left or right.
 *
 * Attempt to only hilite the matching portions of the string.
 */
void do_cmd_messages(void)
{
	event_type ke;

	int i, j, n, q;
	int wid, hgt;

	char shower[80];
	char finder[80];


	/* Wipe finder */
	my_strcpy(finder, "", sizeof(shower));

	/* Wipe shower */
	my_strcpy(shower, "", sizeof(finder));


	/* Total messages */
	n = message_num();

	/* Start on first message */
	i = 0;

	/* Start at leftmost edge */
	q = 0;

	/* Get size */
	Term_get_size(&wid, &hgt);

	/* Save screen */
	screen_save();

	/* Process requests until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Dump messages */
		for (j = 0; (j < hgt - 4) && (i + j < n); j++)
		{
			cptr msg = message_str((s16b)(i+j));
			byte attr = message_color((s16b)(i+j));

			/* Apply horizontal scroll */
			msg = ((int)strlen(msg) >= q) ? (msg + q) : "";

			/* Dump the messages, bottom to top */
			Term_putstr(0, hgt - 3 - j, -1, attr, msg);

			/* Hilite "shower" */
			if (shower[0])
			{
				cptr str = msg;

				/* Display matches */
				while ((str = strstr(str, shower)) != NULL)
				{
					int len = strlen(shower);

					/* Display the match */
					Term_putstr(str-msg, hgt - 3 - j, len, TERM_YELLOW, shower);

					/* Advance */
					str += len;
				}
			}
		}

		/* Display header XXX XXX XXX */
		prt(format("Message Recall (%d-%d of %d), Offset %d",
			   i, i + j - 1, n, q), 0, 0);

		/* Display prompt (not very informative) */
		prt("[Press 'p' for older, 'n' for newer, ..., or ESCAPE]", hgt - 1, 0);

		/* Get a command */
		ke = inkey_ex();

		/* Exit on Escape */
		if (ke.key == ESCAPE) break;

		/* Hack -- Save the old index */
		j = i;

		/* Horizontal scroll */
		if (ke.key == '4')
		{
			/* Scroll left */
			q = (q >= wid / 2) ? (q - wid / 2) : 0;

			/* Success */
			continue;
		}

		/* Horizontal scroll */
		if (ke.key == '6')
		{
			/* Scroll right */
			q = q + wid / 2;

			/* Success */
			continue;
		}

		/* Hack -- handle show */
		if (ke.key == '=')
		{
			/* Prompt */
			prt("Show: ", hgt - 1, 0);

			/* Get a "shower" string, or continue */
			if (!askfor_aux(shower, sizeof shower, NULL)) continue;

			/* Okay */
			continue;
		}

		/* Hack -- handle find */
		if (ke.key == '/')
		{
			s16b z;

			/* Prompt */
			prt("Find: ", hgt - 1, 0);

			/* Get a "finder" string, or continue */
			if (!askfor_aux(finder, sizeof finder, NULL)) continue;

			/* Show it */
			my_strcpy(shower, finder, sizeof(shower));

			/* Scan messages */
			for (z = i + 1; z < n; z++)
			{
				cptr msg = message_str(z);

				/* Search for it */
				if (strstr(msg, finder))
				{
					/* New location */
					i = z;

					/* Done */
					break;
				}
			}
		}

		/* Recall 20 older messages */
		if ((ke.key == 'p') || (ke.key == KTRL('P')) || (ke.key == ' '))
		{
			/* Go older if legal */
			if (i + 20 < n) i += 20;
		}

		/* Recall 10 older messages */
		if (ke.key == '+')
		{
			/* Go older if legal */
			if (i + 10 < n) i += 10;
		}

		/* Recall 1 older message */
		if ((ke.key == '8') || (ke.key == '\n') || (ke.key == '\r'))
		{
			/* Go older if legal */
			if (i + 1 < n) i += 1;
		}

		/* Recall 20 newer messages */
		if ((ke.key == 'n') || (ke.key == KTRL('N')))
		{
			/* Go newer (if able) */
			i = (i >= 20) ? (i - 20) : 0;
		}

		/* Recall 10 newer messages */
		if (ke.key == '-')
		{
			/* Go newer (if able) */
			i = (i >= 10) ? (i - 10) : 0;
		}

		/* Recall 1 newer messages */
		if (ke.key == '2')
		{
			/* Go newer (if able) */
			i = (i >= 1) ? (i - 1) : 0;
		}

		/* Scroll forwards or backwards using mouse clicks */
		if (ke.key == '\xff')
		{
			if (ke.index)
			{
				if (ke.mousey <= hgt / 2)
				{
					/* Go older if legal */
					if (i + 20 < n) i += 20;
				}
				else
				{
					/* Go newer (if able) */
					i = (i >= 20) ? (i - 20) : 0;
				}
			}
		}

		/* Hack -- Error of some kind */
		if (i == j) bell(NULL);
	}

	/* Load screen */
	screen_load();
}




/*** Options display and setting ***/

/*
 * Displays an option entry.
 */
static void display_option(menu_type *menu, int oid,
							bool cursor, int row, int col, int width)
{
	byte attr = curs_attrs[CURS_KNOWN][(int)cursor];

	c_prt(attr, format("%-45s: %s  (%s)", option_desc[oid],
	                   op_ptr->opt[oid] ? "yes" : "no ", option_text[oid]),
	                   row, col);
}

/*
 * Handle keypresses for an option entry.
 */
static bool update_option(char key, void *pgdb, int oid)
{
	/* Ignore arrow events */
	if (key == ARROW_LEFT || key == ARROW_RIGHT)
		return;

	switch (toupper((unsigned char) key))
	{
		case 'Y':
		{
			op_ptr->opt[oid] = TRUE;
			break;
		}

		case 'N':
		{
			op_ptr->opt[oid] = FALSE;
			break;
		}

		case '?':
		{
			show_file(format("option.txt#%s", option_text[oid]), NULL, 0, 0);
			break;
		}

		default:
		{
			op_ptr->opt[oid] = !op_ptr->opt[oid];
			break;
		}

	}

	return TRUE;
}

static const menu_iter options_toggle_iter =
{
	0,
	NULL,
	NULL,
	display_option,		/* label */
	update_option		/* updater */
};

static menu_type option_toggle_menu;


/*
 * Interact with some options
 */
static void do_cmd_options_aux(void *vpage, cptr info)
{
	int page = (int)vpage;
	int opt[OPT_PAGE_PER];
	int i, n = 0;
	int cursor_pos = 0;

	menu_type *menu = &option_toggle_menu;
	menu->title = info;
	menu_layout(menu, &SCREEN_REGION);

	screen_save();
	clear_from(0);

	/* Filter the options for this page */
	for (i = 0; i < OPT_PAGE_PER; i++)
	{
		if (option_page[page][i] != OPT_NONE)
			opt[n++] = option_page[page][i];
	}

	menu_set_filter(menu, opt, n);
	menu->menu_data = vpage;

	menu_layout(menu, &SCREEN_REGION);

	while (TRUE)
	{
		event_type cx;

		cx = menu_select(menu, &cursor_pos, EVT_MOVE);

		if (cx.key == ESCAPE)
			break;
		else if (cx.type == EVT_MOVE)
			cursor_pos = cx.index;
		else if (cx.type == EVT_SELECT && strchr("YN", toupper(cx.key)))
			cursor_pos++;

		cursor_pos = (cursor_pos+n) % n;
	}

	/* Hack -- Notice use of any "cheat" options */
	for (i = OPT_CHEAT; i < OPT_ADULT; i++)
	{
		if (op_ptr->opt[i])
		{
			/* Set score option */
			op_ptr->opt[OPT_SCORE + (i - OPT_CHEAT)] = TRUE;
		}
	}

	screen_load();
}


/*
 * Modify the "window" options
 */
static void do_cmd_options_win(void)
{
	int i, j, d;

	int y = 0;
	int x = 0;

	event_type ke;

	u32b old_flag[ANGBAND_TERM_MAX];


	/* Memorize old flags */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		old_flag[j] = op_ptr->window_flag[j];
	}


	/* Clear screen */
	clear_from(0);

	/* Interact */
	while (1)
	{
		/* Prompt */
		prt("Window flags (<dir> to move, 't' to toggle, or ESC)", 0, 0);

		/* Display the windows */
		for (j = 0; j < ANGBAND_TERM_MAX; j++)
		{
			byte a = TERM_WHITE;

			cptr s = angband_term_name[j];

			/* Use color */
			if (j == x) a = TERM_L_BLUE;

			/* Window name, staggered, centered */
			Term_putstr(35 + j * 5 - strlen(s) / 2, 2 + j % 2, -1, a, s);
		}

		/* Display the options */
		for (i = 0; i < PW_MAX_FLAGS; i++)
		{
			byte a = TERM_WHITE;

			cptr str = window_flag_desc[i];

			/* Use color */
			if (i == y) a = TERM_L_BLUE;

			/* Unused option */
			if (!str) str = "(Unused option)";

			/* Flag name */
			Term_putstr(0, i + 5, -1, a, str);

			/* Display the windows */
			for (j = 0; j < ANGBAND_TERM_MAX; j++)
			{
				byte a = TERM_WHITE;

				char c = '.';

				/* Use color */
				if ((i == y) && (j == x)) a = TERM_L_BLUE;

				/* Active flag */
				if (op_ptr->window_flag[j] & (1L << i)) c = 'X';

				/* Flag value */
				Term_putch(35 + j * 5, i + 5, a, c);
			}
		}

		/* Place Cursor */
		Term_gotoxy(35 + x * 5, y + 5);

		/* Get key */
		ke = inkey_ex();

		/* Allow escape */
		if ((ke.key == ESCAPE) || (ke.key == 'q')) break;

		/* Mouse interaction */
		if (ke.key == '\xff')
		{
			int choicey = ke.mousey - 5;
			int choicex = (ke.mousex - 35)/5;

			if ((choicey >= 0) && (choicey < PW_MAX_FLAGS)
				&& (choicex > 0) && (choicex < ANGBAND_TERM_MAX)
				&& !(ke.mousex % 5))
			{
				y = choicey;
				x = (ke.mousex - 35)/5;
			}

			/* Toggle using mousebutton later */
			if (!ke.index) continue;
		}

		/* Toggle */
		if ((ke.key == '5') || (ke.key == 't') || (ke.key == '\n') || (ke.key == '\r') || ((ke.key == '\xff') && (ke.index)))
		{
			/* Hack -- ignore the main window */
			if (x == 0)
			{
				bell("Cannot set main window flags!");
			}

			/* Toggle flag (off) */
			else if (op_ptr->window_flag[x] & (1L << y))
			{
				op_ptr->window_flag[x] &= ~(1L << y);
			}

			/* Toggle flag (on) */
			else
			{
				op_ptr->window_flag[x] |= (1L << y);
			}

			/* Continue */
			continue;
		}

		/* Extract direction */
		d = target_dir(ke.key);

		/* Move */
		if (d != 0)
		{
			x = (x + ddx[d] + 8) % ANGBAND_TERM_MAX;
			y = (y + ddy[d] + 16) % PW_MAX_FLAGS;
		}

		/* Oops */
		else
		{
			bell("Illegal command for window options!");
		}
	}

	/* Notice changes */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		term *old = Term;

		/* Dead window */
		if (!angband_term[j]) continue;

		/* Ignore non-changes */
		if (op_ptr->window_flag[j] == old_flag[j]) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* Erase */
		Term_clear();

		/* Refresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 *  Header and footer marker string for pref file dumps
 */
static cptr dump_separator = "#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#";


/*
 * Remove old lines from pref files
 */
static void remove_old_dump(const char *orig_file, const char *mark)
{
	FILE *tmp_fff, *orig_fff;

	char tmp_file[1024];
	char buf[1024];
	bool between_marks = FALSE;
	bool changed = FALSE;
	char expected_line[1024];


	/* Open an old dump file in read-only mode */
	orig_fff = my_fopen(orig_file, "r");

	/* If original file does not exist, nothing to do */
	if (!orig_fff) return;

	/* Open a new temporary file */
	tmp_fff = my_fopen_temp(tmp_file, sizeof(tmp_file));

	if (!tmp_fff)
	{
			msg_format("Failed to create temporary file %s.", tmp_file);
			msg_print(NULL);
			return;
	}

	/* Work out what we expect to find */
	strnfmt(expected_line, sizeof(expected_line), "%s begin %s", dump_separator, mark);

	/* Loop for every line */
	while (TRUE)
	{
		/* Read a line */
		if (my_fgets(orig_fff, buf, sizeof(buf)))
		{
			/* End of file but no end marker */
			if (between_marks) changed = FALSE;

			break;
		}

		/* Is this line a header/footer? */
		if (strncmp(buf, dump_separator, strlen(dump_separator)) == 0)
		{
			/* Found the expected line? */
			if (strcmp(buf, expected_line) == 0)
			{
				if (!between_marks)
				{
					/* Expect the footer next */
					strnfmt(expected_line, sizeof(expected_line),
					        "%s end %s", dump_separator, mark);

					between_marks = TRUE;

					/* There are some changes */
					changed = TRUE;
				}
				else
				{
					/* Expect a header next - XXX shouldn't happen */
					strnfmt(expected_line, sizeof(expected_line),
					        "%s begin %s", dump_separator, mark);

					between_marks = FALSE;

					/* Next line */
					continue;
				}
			}

			/* Found a different line */
			else
			{
				/* Expected a footer and got something different? */
				if (between_marks)
				{
					/* Abort */
					changed = FALSE;
					break;
				}
			}
		}

		if (!between_marks)
		{
			/* Copy orginal line */
			fprintf(tmp_fff, "%s\n", buf);
		}
	}

	/* Close files */
	my_fclose(orig_fff);
	my_fclose(tmp_fff);

	/* If there are changes, overwrite the original file with the new one */
	if (changed)
	{
		/* Copy contents of temporary file */
		tmp_fff = my_fopen(tmp_file, "r");
		orig_fff = my_fopen(orig_file, "w");

		while (!my_fgets(tmp_fff, buf, sizeof(buf)))
			fprintf(orig_fff, "%s\n", buf);

		my_fclose(orig_fff);
		my_fclose(tmp_fff);
	}

	/* Kill the temporary file */
	fd_kill(tmp_file);
}


/*
 * Output the header of a pref-file dump
 */
static void pref_header(FILE *fff, const char *mark)
{
	/* Start of dump */
	fprintf(fff, "%s begin %s\n", dump_separator, mark);

	fprintf(fff, "# *Warning!*  The lines below are an automatic dump.\n");
	fprintf(fff, "# Don't edit them; changes will be deleted and replaced automatically.\n");
}

/*
 * Output the footer of a pref-file dump
 */
static void pref_footer(FILE *fff, const char *mark)
{
	fprintf(fff, "# *Warning!*  The lines above are an automatic dump.\n");
	fprintf(fff, "# Don't edit them; changes will be deleted and replaced automatically.\n");

	/* End of dump */
	fprintf(fff, "%s end %s\n", dump_separator, mark);
}


/*
 * Interactively dump preferences to a file.
 *
 * - Title must have the form "Dump <pref-type>"
 * - dump(FILE *) needs to emit only the raw data for the dump.
 *   Comments are generated automatically
 */
static void dump_pref_file(void (*dump)(FILE*), const char *title, int row)
{
	char ftmp[80];
	char buf[1025];
	FILE *fff;

	/* Prompt */
	prt(format("%s to a pref file", title), row, 0);

	/* Prompt */
	prt("File: ", row + 2, 0);

	/* Default filename */
	strnfmt(ftmp, sizeof ftmp, "%s.prf", op_ptr->base_name);

	/* Get a filename */
	if (!askfor_aux(ftmp, sizeof ftmp, NULL)) return;

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, ftmp);

	FILE_TYPE(FILE_TYPE_TEXT);

	/* Remove old macros */
	remove_old_dump(buf, title);

	/* Append to the file */
	fff = my_fopen(buf, "a");

	/* Failure */
	if (!fff)
	{
		msg_print("Failed");
		return;
	}

	/* Output header */
	pref_header(fff, title);

	/* Skip some lines */
	fprintf(fff, "\n\n");

	/* Start dumping */
	fprintf(fff, "# %s definitions\n\n", strstr(title, " "));
	
	dump(fff);

	/* All done */
	fprintf(fff, "\n\n\n");

	/* Output footer */
	pref_footer(fff, title);

	/* Close */
	my_fclose(fff);

	/* Message */
	msg_print(format("Dumped %s", strstr(title, " ")+1));
}

/*
 * Save autoinscription data to a pref file.
 */
static void autoinsc_dump(FILE *fff)
{
	int i;

	if (!inscriptions)
		return;

	/* Start dumping */
	fprintf(fff, "# Autoinscription settings");
	fprintf(fff, "# B:item kind:inscription\n\n");

	for (i = 0; i < inscriptions_count; i++)
	{
		object_kind *k_ptr = &k_info[inscriptions[i].kind_idx];

		/* Describe and write */
		fprintf(fff, "# Autoinscription for %s\n", k_name + k_ptr->name);
		fprintf(fff, "B:%d:%s\n\n", inscriptions[i].kind_idx,
		        quark_str(inscriptions[i].inscription_idx));
	}

	/* All done */
	fprintf(fff, "\n");
}

/*
 * Save squelch data to a pref file.
 */
static void squelch_dump(FILE *fff)
{
	int i;
	int tval, sval;
	bool squelch;

	/* Start dumping */
	fprintf(fff, "\n\n");
	fprintf(fff, "# Squelch bits\n\n");

	/* Dump squelch bits */
	for (i = 1; i < z_info->k_max; i++)
	{
		tval = k_info[i].tval;
		sval = k_info[i].sval;
		squelch = k_info[i].squelch;

		/* Dump the squelch info */
		if (tval || sval)
			fprintf(fff, "Q:%d:%d:%d:%d\n", i, tval, sval, squelch);
	}

	/* All done */
	fprintf(fff, "\n");
}

/*
 * Write all current options to a user preference file.
 */
static void option_dump(FILE *fff)
{
	int i, j;

	/* Dump options (skip cheat, adult, score) */
	for (i = 0; i < OPT_CHEAT; i++)
	{
		/* Require a real option */
		if (!option_text[i]) continue;

		/* Comment */
		fprintf(fff, "# Option '%s'\n", option_desc[i]);

		/* Dump the option */
		if (op_ptr->opt[i])
		{
			fprintf(fff, "Y:%s\n", option_text[i]);
		}
		else
		{
			fprintf(fff, "X:%s\n", option_text[i]);
		}

		/* Skip a line */
		fprintf(fff, "\n");
	}

	/* Dump window flags */
	for (i = 1; i < ANGBAND_TERM_MAX; i++)
	{
		/* Require a real window */
		if (!angband_term[i]) continue;

		/* Check each flag */
		for (j = 0; j < (int)N_ELEMENTS(window_flag_desc); j++)
		{
			/* Require a real flag */
			if (!window_flag_desc[j]) continue;

			/* Comment */
			fprintf(fff, "# Window '%s', Flag '%s'\n",
				angband_term_name[i], window_flag_desc[j]);

			/* Dump the flag */
			if (op_ptr->window_flag[i] & (1L << j))
			{
				fprintf(fff, "W:%d:%d:1\n", i, j);
			}
			else
			{
				fprintf(fff, "W:%d:%d:0\n", i, j);
			}

			/* Skip a line */
			fprintf(fff, "\n");
		}
	}

	autoinsc_dump(fff);
	squelch_dump(fff);
}



#ifdef ALLOW_MACROS

/*
 * append all current macros to the given file
 */
static void macro_dump(FILE *fff)
{
	int i;
	char buf[1024];

	/* Dump them */
	for (i = 0; i < macro__num; i++)
	{
		/* Start the macro */
		fprintf(fff, "# Macro '%d'\n\n", i);

		/* Extract the macro action */
		ascii_to_text(buf, sizeof(buf), macro__act[i]);

		/* Dump the macro action */
		fprintf(fff, "A:%s\n", buf);

		/* Extract the macro pattern */
		ascii_to_text(buf, sizeof(buf), macro__pat[i]);

		/* Dump the macro pattern */
		fprintf(fff, "P:%s\n", buf);

		/* End the macro */
		fprintf(fff, "\n\n");
	}
}


/*
 * Hack -- ask for a "trigger" (see below)
 *
 * Note the complex use of the "inkey()" function from "util.c".
 *
 * Note that both "flush()" calls are extremely important.  This may
 * no longer be true, since "util.c" is much simpler now.  XXX XXX XXX
 */
static void do_cmd_macro_aux(char *buf)
{
	char ch;

	int n = 0;

	char tmp[1024];


	/* Flush */
	flush();


	/* Do not process macros */
	inkey_base = TRUE;

	/* First key */
	ch = inkey();

	text_out_hook = text_out_to_screen;

	/* Read the pattern */
	while (ch != 0 && ch != '\xff')
	{
		/* Save the key */
		buf[n++] = ch;
		buf[n] = 0;

		/* echo */
		ascii_to_text(tmp, sizeof(tmp), buf+n-1);
		text_out(tmp);

		/* Do not process macros */
		inkey_base = TRUE;

		/* Do not wait for keys */
		inkey_scan = TRUE;

		/* Attempt to read a key */
		ch = inkey();
	}

	/* Convert the trigger */
	ascii_to_text(tmp, sizeof(tmp), buf);
}


/*
 * Hack -- ask for a keymap "trigger" (see below)
 *
 * Note that both "flush()" calls are extremely important.  This may
 * no longer be true, since "util.c" is much simpler now.  XXX XXX XXX
 */
static void do_cmd_macro_aux_keymap(char *buf)
{
	char tmp[1024];


	/* Flush */
	flush();


	/* Get a key */
	buf[0] = inkey();
	buf[1] = '\0';


	/* Convert to ascii */
	ascii_to_text(tmp, sizeof(tmp), buf);

	/* Hack -- display the trigger */
	Term_addstr(-1, TERM_WHITE, tmp);


	/* Flush */
	flush();
}


/*
 * Hack -- Append all keymaps to the given file.
 *
 * Hack -- We only append the keymaps for the "active" mode.
 */
static void keymap_dump(FILE *fff)
{
	int i;
	int mode;
	char buf[1024];

	/* Roguelike */
	if (rogue_like_commands)
	{
		mode = KEYMAP_MODE_ROGUE;
	}

	/* Original */
	else
	{
		mode = KEYMAP_MODE_ORIG;
	}

	for (i = 0; i < (int)N_ELEMENTS(keymap_act[mode]); i++)
	{
		char key[2] = "?";

		cptr act;

		/* Loop up the keymap */
		act = keymap_act[mode][i];

		/* Skip empty keymaps */
		if (!act) continue;

		/* Encode the action */
		ascii_to_text(buf, sizeof(buf), act);

		/* Dump the keymap action */
		fprintf(fff, "A:%s\n", buf);

		/* Convert the key into a string */
		key[0] = i;

		/* Encode the key */
		ascii_to_text(buf, sizeof(buf), key);

		/* Dump the keymap pattern */
		fprintf(fff, "C:%d:%s\n", mode, buf);

		/* Skip a line */
		fprintf(fff, "\n");
	}

}

#endif


/*
 * Interact with "macros"
 *
 * Could use some helpful instructions on this page.  XXX XXX XXX
 * CLEANUP
 */

static event_action macro_actions[] =
{
	{LOAD_PREF, "Load a user pref file", 0},
#ifdef ALLOW_MACROS
	{APP_MACRO, "Append macros to a file", 0},
	{ASK_MACRO, "Query a macro", 0},
	{NEW_MACRO, "Create a macro", 0},
	{DEL_MACRO, "Remove a macro", 0},
	{APP_KEYMAP, "Append keymaps to a file", 0},
	{ASK_KEYMAP, "Query a keymap", 0},
	{NEW_KEYMAP, "Create a keymap", 0},
	{DEL_KEYMAP, "Remove a keymap", 0},
	{ENTER_ACT, "Enter a new action", 0}
#endif /* ALLOW_MACROS */
};

static menu_type macro_menu;


void do_cmd_macros(void)
{

	char tmp[1024];

	char pat[1024];

	int mode;
	int cursor = 0;

	region loc = {0, 0, 0, 12};

	/* Roguelike */
	if (rogue_like_commands)
	{
		mode = KEYMAP_MODE_ROGUE;
	}

	/* Original */
	else
	{
		mode = KEYMAP_MODE_ORIG;
	}


	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	screen_save();

	menu_layout(&macro_menu, &loc);

	/* Process requests until done */
	while (1)
	{
		event_type c;
		int evt;

		/* Clear screen */
		clear_from(0);

		/* Describe current action */
		prt("Current action (if any) shown below:", 13, 0);

		/* Analyze the current action */
		ascii_to_text(tmp, sizeof(tmp), macro_buffer);

		/* Display the current action */
		prt(tmp, 14, 0);
		c = menu_select(&macro_menu, &cursor, EVT_CMD);


		if (ESCAPE == c.key) break;
		if (c.key == ARROW_LEFT || c.key == ARROW_RIGHT) continue;
		evt = macro_actions[cursor].id;

		switch(evt)
		{
		case LOAD_PREF:
		{
			do_cmd_pref_file_hack(16);
			break;
		}

#ifdef ALLOW_MACROS
		case APP_MACRO:
		{
			/* Dump the macros */
			(void)dump_pref_file(macro_dump, "Dump Macros", 15);

			break;
		}

		case ASK_MACRO:
		{
			int k;

			/* Prompt */
			prt("Command: Query a macro", 16, 0);

			/* Prompt */
			prt("Trigger: ", 18, 0);

			/* Get a macro trigger */
			do_cmd_macro_aux(pat);

			/* Get the action */
			k = macro_find_exact(pat);

			/* Nothing found */
			if (k < 0)
			{
				/* Prompt */
				msg_print("Found no macro.");
			}

			/* Found one */
			else
			{
				/* Obtain the action */
				my_strcpy(macro_buffer, macro__act[k], sizeof(macro_buffer));

				/* Analyze the current action */
				ascii_to_text(tmp, sizeof(tmp), macro_buffer);

				/* Display the current action */
				prt(tmp, 22, 0);

				/* Prompt */
				msg_print("Found a macro.");
			}
			break;
		}

		case NEW_MACRO:
		{
			/* Prompt */
			prt("Command: Create a macro", 16, 0);

			/* Prompt */
			prt("Trigger: ", 18, 0);

			/* Get a macro trigger */
			do_cmd_macro_aux(pat);

			/* Clear */
			clear_from(20);

			/* Prompt */
			prt("Action: ", 20, 0);

			/* Convert to text */
			ascii_to_text(tmp, sizeof(tmp), macro_buffer);

			/* Get an encoded action */
			if (askfor_aux(tmp, sizeof tmp, NULL))
			{
				/* Convert to ascii */
				text_to_ascii(macro_buffer, sizeof(macro_buffer), tmp);

				/* Link the macro */
				macro_add(pat, macro_buffer);

				/* Prompt */
				msg_print("Added a macro.");
			}
			break;
		}

		case DEL_MACRO:
		{
			/* Prompt */
			prt("Command: Remove a macro", 16, 0);

			/* Prompt */
			prt("Trigger: ", 18, 0);

			/* Get a macro trigger */
			do_cmd_macro_aux(pat);

			/* Link the macro */
			macro_add(pat, pat);

			/* Prompt */
			msg_print("Removed a macro.");
			break;
		}
		case APP_KEYMAP:
		{
			/* Dump the keymaps */
			(void)dump_pref_file(keymap_dump, "Dump Keymaps", 15);
			break;
		}
		case ASK_KEYMAP:
		{
			cptr act;

			/* Prompt */
			prt("Command: Query a keymap", 16, 0);

			/* Prompt */
			prt("Keypress: ", 18, 0);

			/* Get a keymap trigger */
			do_cmd_macro_aux_keymap(pat);

			/* Look up the keymap */
			act = keymap_act[mode][(byte)(pat[0])];

			/* Nothing found */
			if (!act)
			{
				/* Prompt */
				msg_print("Found no keymap.");
			}

			/* Found one */
			else
			{
				/* Obtain the action */
				my_strcpy(macro_buffer, act, sizeof(macro_buffer));

				/* Analyze the current action */
				ascii_to_text(tmp, sizeof(tmp), macro_buffer);

				/* Display the current action */
				prt(tmp, 22, 0);

				/* Prompt */
				msg_print("Found a keymap.");
			}
			break;
		}
		case NEW_KEYMAP:
		{
			/* Prompt */
			prt("Command: Create a keymap", 16, 0);

			/* Prompt */
			prt("Keypress: ", 18, 0);

			/* Get a keymap trigger */
			do_cmd_macro_aux_keymap(pat);

			/* Clear */
			clear_from(20);

			/* Prompt */
			prt("Action: ", 20, 0);

			/* Convert to text */
			ascii_to_text(tmp, sizeof(tmp), macro_buffer);

			/* Get an encoded action */
			if (askfor_aux(tmp, sizeof tmp, NULL))
			{
				/* Convert to ascii */
				text_to_ascii(macro_buffer, sizeof(macro_buffer), tmp);

				/* Free old keymap */
				string_free(keymap_act[mode][(byte)(pat[0])]);

				/* Make new keymap */
				keymap_act[mode][(byte)(pat[0])] = string_make(macro_buffer);

				/* Prompt */
				msg_print("Added a keymap.");
			}
			break;
		}
		case DEL_KEYMAP:
		{
			/* Prompt */
			prt("Command: Remove a keymap", 16, 0);

			/* Prompt */
			prt("Keypress: ", 18, 0);

			/* Get a keymap trigger */
			do_cmd_macro_aux_keymap(pat);

			/* Free old keymap */
			string_free(keymap_act[mode][(byte)(pat[0])]);

			/* Make new keymap */
			keymap_act[mode][(byte)(pat[0])] = NULL;

			/* Prompt */
			msg_print("Removed a keymap.");
			break;
		}
		case ENTER_ACT: /* Enter a new action */
		{
			/* Prompt */
			prt("Command: Enter a new action", 16, 0);

			/* Go to the correct location */
			Term_gotoxy(0, 22);

			/* Analyze the current action */
			ascii_to_text(tmp, sizeof(tmp), macro_buffer);

			/* Get an encoded action */
			if (askfor_aux(tmp, sizeof tmp, NULL))
			{
				/* Extract an action */
				text_to_ascii(macro_buffer, sizeof(macro_buffer), tmp);
			}
			break;
		}
#endif /* ALLOW_MACROS */
		}

		/* Flush messages */
		message_flush();
	}

	/* Load screen */
	screen_load();
}


/* Dump monsters */
static void dump_monsters(FILE *fff)
{
	int i;
	for (i = 0; i < z_info->r_max; i++)
	{
		monster_race *r_ptr = &r_info[i];

		/* Skip non-entries */
		if (!r_ptr->name) continue;

		/* Dump a comment */
		fprintf(fff, "# %s\n", (r_name + r_ptr->name));

		/* Dump the monster attr/char info */
		fprintf(fff, "R:%d:0x%02X:0x%02X\n\n", i,
			(byte)(r_ptr->x_attr), (byte)(r_ptr->x_char));
	}
}

/* Dump objects */
static void dump_objects(FILE *fff)
{
	int i;
	for (i = 0; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Skip non-entries */
		if (!k_ptr->name) continue;

		/* Dump a comment */
		fprintf(fff, "# %s\n", (k_name + k_ptr->name));

		/* Dump the object attr/char info */
		fprintf(fff, "K:%d:0x%02X:0x%02X\n\n", i,
					(byte)(k_ptr->x_attr), (byte)(k_ptr->x_char));
	}
}

/* Dump features */
static void dump_features(FILE *fff)
{
	int i;
	for (i = 0; i < z_info->f_max; i++)
	{
		feature_type *f_ptr = &f_info[i];

		/* Skip non-entries */
		if (!f_ptr->name) continue;

		/* Skip mimic entries -- except invisible trap */
		if ((f_ptr->mimic != i) && (i != FEAT_INVIS)) continue;

		/* Dump a comment */
		fprintf(fff, "# %s\n", (f_name + f_ptr->name));

		/* Dump the feature attr/char info */
		/* Dump the feature attr/char info */
		fprintf(fff, "F:%d:0x%02X:0x%02X\n\n", i,
						(byte)(f_ptr->x_attr), (byte)(f_ptr->x_char));


	}
}

/* Dump flavors */
static void dump_flavors(FILE *fff)
{
	int i;
	for (i = 0; i < z_info->flavor_max; i++)
	{
		flavor_type *x_ptr = &flavor_info[i];

		/* Dump a comment */
		fprintf(fff, "# %s\n", (flavor_text + x_ptr->text));

		/* Dump the flavor attr/char info */
		fprintf(fff, "L:%d:0x%02X:0x%02X\n\n", i,
			(byte)(x_ptr->x_attr), (byte)(x_ptr->x_char));
	}
}

/* Dump colors */
static void dump_colors(FILE *fff)
{
	int i;
	for (i = 0; i < MAX_COLORS; i++)
	{
		int kv = angband_color_table[i][0];
		int rv = angband_color_table[i][1];
		int gv = angband_color_table[i][2];
		int bv = angband_color_table[i][3];

		cptr name = "unknown";

		/* Skip non-entries */
		if (!kv && !rv && !gv && !bv) continue;

		/* Extract the color name */
		if (i < BASIC_COLORS) name = color_names[i];

		/* Dump a comment */
		fprintf(fff, "# Color '%s'\n", name);

	}
}


int modify_attribute(const char *clazz, int oid, const char *name,
							byte da, char dc, byte *pca, char *pcc)
{
	int cx;
	const char *empty_symbol = "<< ? >>";
#ifdef UNANGBAND
	const char *empty_symbol2 = "\0";
	const char *empty_symbol3 = "\0";
#endif

	byte ca = (byte)*pca;
	byte cc = (byte)*pcc;

#ifdef UNANGBAND
	int linec = (use_trptile ? 22 : (use_dbltile ? 21 : 20));

	if (use_trptile && use_bigtile)
	{
		empty_symbol = "// ?????? \\\\";
		empty_symbol2 = "   ??????   ";
		empty_symbol3 = "\\\\ ?????? //";
	}
	else if (use_dbltile && use_bigtile)
	{
		empty_symbol = "// ???? \\\\";
		empty_symbol2 = "\\\\ ???? //";
	}
	else if (use_trptile)
	{
		empty_symbol = "// ??? \\\\";
		empty_symbol2 = "   ???   ";
		empty_symbol3 = "\\\\ ??? //";
	}
	else if (use_dbltile)
	{
		empty_symbol = "// ?? \\\\";
		empty_symbol2 = "\\\\ ?? //";
	}
	else
#else
	int linec = 20;
#endif
	if (use_bigtile) empty_symbol = "<< ?? >>";

	/* Prompt */
	prt(format("Command: Change %s attr/chars", clazz), 15, 0);

	/* Label the object */
	Term_putstr(5, 17, -1, TERM_WHITE, format("%s = %d, Name = %-40.40s", clazz, oid, name));
							
	/* Label the Default values */
	Term_putstr(10, 19, -1, TERM_WHITE,
			    format("Default attr/char = %3u / %3u", da, dc));
	Term_putstr(40, 19, -1, TERM_WHITE, empty_symbol);

#ifdef UNANGBAND
	if (use_dbltile || use_trptile) Term_putstr (40, 20, -1, TERM_WHITE, empty_symbol2);
	if (use_trptile) Term_putstr (40, 20, -1, TERM_WHITE, empty_symbol3);
#endif

	big_pad(43, 19, da, dc);

	/* Label the Current values */
	Term_putstr(10, linec, -1, TERM_WHITE,
			    format("Current attr/char = %3u / %3u", ca, cc));
	Term_putstr(40, linec, -1, TERM_WHITE, empty_symbol);

#ifdef UNANGBAND
	if (use_dbltile || use_trptile) Term_putstr (40, linec+1, -1, TERM_WHITE, empty_symbol2); 
	if (use_trptile) Term_putstr (40, linec+2, -1, TERM_WHITE, empty_symbol3); 
#endif

	big_pad(43, linec, ca, cc);

#ifdef UNANGBAND
	if (use_trptile) linec++;
#endif

	/* Prompt */
	Term_putstr(0, linec + 2, -1, TERM_WHITE, "Command (n/N/a/A/c/C): ");

	/* Get a command */
	cx = inkey();

	/* All done */
	if (cx == ESCAPE) return cx;

	/* Analyze */
	if (cx == 'a') *pca = (byte)(ca + 1);
	if (cx == 'A') *pca = (byte)(ca - 1);
	if (cx == 'c') *pcc = (byte)(cc + 1);
	if (cx == 'C') *pcc = (byte)(cc - 1);

	return cx;
}

event_action visual_menu_items [] =
{
	{ LOAD_PREF, "Load a user pref file", 0, 0},
	{ DUMP_MON,  "Dump monster attr/chars", 0, 0},
	{ DUMP_OBJ,  "Dump object attr/chars", 0, 0 },
	{ DUMP_FEAT, "Dump feature attr/chars", 0, 0 },
	{ DUMP_FLAV, "Dump flavor attr/chars", 0, 0 },
	{ MOD_MON,   "Change monster attr/chars", 0, 0 },
	{ MOD_OBJ,   "Change object attr/chars", 0, 0 },
	{ MOD_FEAT,  "Change feature attr/chars", 0, 0 },
	{ MOD_FLAV,  "Change flavor attr/chars", 0, 0 },
	{ RESET_VIS, "Reset visuals", 0, 0 },
};

static menu_type visual_menu;


/*
 * Interact with "visuals"
 */
void do_cmd_visuals(void)
{
	int cursor = 0;

	/* Save screen */
	screen_save();

	menu_layout(&visual_menu, &SCREEN_REGION);

	/* Interact until done */
	while (1)
	{
		event_type key;
		int evt = -1;
		clear_from(0);
		key = menu_select(&visual_menu, &cursor, EVT_CMD);
		if (key.key == ESCAPE) 
			break;

		if (key.key == ARROW_LEFT || key.key == ARROW_RIGHT)
			continue;

		assert(cursor >= 0 && cursor < visual_menu.count);

		evt = visual_menu_items[cursor].id;

		if (evt == LOAD_PREF)
		{
			/* Ask for and load a user pref file */
			do_cmd_pref_file_hack(15);
		}

#ifdef ALLOW_VISUALS

		else if (evt == DUMP_MON)
		{
			dump_pref_file(dump_monsters, "Dump Monster attr/chars", 15);
		}

		else if (evt == DUMP_OBJ)
		{
			dump_pref_file(dump_objects, "Dump Object attr/chars", 15);
		}

		else if (evt == DUMP_FEAT)
		{
			dump_pref_file(dump_features, "Dump Feature attr/chars", 15);
		}

		/* Dump flavor attr/chars */
		else if (evt == DUMP_FLAV) 
		{
			dump_pref_file(dump_flavors, "Dump Flavor attr/chars", 15);
		}

		/* Modify monster attr/chars */
		else if (evt == MOD_MON)
		{
			static int r = 0;

			/* Prompt */
			prt("Command: Change monster attr/chars", 15, 0);

			/* Hack -- query until done */
			while (1)
			{
				int cx;
				monster_race *r_ptr = &r_info[r];

				cx = modify_attribute("Monster", r, r_name + r_ptr->name,
									r_ptr->d_attr, r_ptr->d_char,
									&r_ptr->x_attr, &r_ptr->x_char);

				if (cx == ESCAPE) break;
				/* Analyze */
				if (cx == 'n') r = (r + z_info->r_max + 1) % z_info->r_max;
				if (cx == 'N') r = (r + z_info->r_max - 1) % z_info->r_max;
			}
		}

		/* Modify object attr/chars */
		else if (evt == MOD_OBJ)
		{
			static int k = 0;

			/* Hack -- query until done */
			while (1)
			{
				int cx;
				object_kind *k_ptr = &k_info[k];
				cx = modify_attribute("Object", k, k_name + k_ptr->name,
									k_ptr->d_attr, k_ptr->d_char,
									&k_ptr->x_attr, &k_ptr->x_char);

				if (cx == ESCAPE) break;
				if (cx == 'n') k = (k + z_info->k_max + 1) % z_info->k_max;
				if (cx == 'N') k = (k + z_info->k_max - 1) % z_info->k_max;
			}
		}

		/* Modify feature attr/chars */
		else if (evt == MOD_FEAT)
		{
			static int f = 0;

			/* Hack -- query until done */
			while (1)
			{
				feature_type *f_ptr = &f_info[f];
				int cx = modify_attribute("Feature", f, f_name + f_ptr->name,
									f_ptr->d_attr, f_ptr->d_char,
									&f_ptr->x_attr, &f_ptr->x_char);

				if (cx == ESCAPE) break;
				if (cx == 'n') f = (f + z_info->f_max + 1) % z_info->f_max;
				if (cx == 'N') f = (f + z_info->f_max - 1) % z_info->f_max;
			}
		}
		/* Modify flavor attr/chars */
		else if (evt == MOD_FLAV)
		{
			static int f = 0;

			/* Hack -- query until done */
			while (1)
			{
				flavor_type *x_ptr = &flavor_info[f];
				int cx = modify_attribute("Flavor", f, flavor_text + x_ptr->text,
									x_ptr->d_attr, x_ptr->d_char,
									&x_ptr->x_attr, &x_ptr->x_char);

				if (cx == ESCAPE) break;
				if (cx == 'n') f = (f + z_info->flavor_max + 1) % z_info->flavor_max;
				if (cx == 'N') f = (f + z_info->flavor_max - 1) % z_info->flavor_max;
			}
		}

#endif /* ALLOW_VISUALS */

		/* Reset visuals */
		else if (evt == RESET_VIS)
		{
			/* Reset */
			reset_visuals(TRUE);

			/* Message */
			msg_print("Visual attr/char tables reset.");
		}
	}

	/* Load screen */
	screen_load();
}


static event_action color_events [] =
{
	{LOAD_PREF, "Load a user pref file", 0, 0},
#ifdef ALLOW_COLORS
	{DUMP_COL, "Dump colors", 0, 0},
	{MOD_COL, "Modify colors", 0, 0}
#endif
};

static menu_type color_menu;


/*
 * Interact with "colors"
 */
void do_cmd_colors(void)
{
	int i;
	int cx;
	int cursor = 0;

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Save screen */
	screen_save();

	menu_layout(&color_menu, &SCREEN_REGION);

	/* Interact until done */
	while (1)
	{
		event_type key;
		int evt;
		clear_from(0);
		key = menu_select(&color_menu, &cursor, EVT_CMD);

		/* Done */
		if (key.key == ESCAPE) break;

		if (key.key == ARROW_RIGHT || key.key == ARROW_LEFT) continue;

		evt = color_events[cursor].id;

		/* Load a user pref file */
		if (evt == LOAD_PREF)
		{
			/* Ask for and load a user pref file */
			do_cmd_pref_file_hack(8);

			/* Could skip the following if loading cancelled XXX XXX XXX */

			/* Mega-Hack -- React to color changes */
			Term_xtra(TERM_XTRA_REACT, 0);

			/* Mega-Hack -- Redraw physical windows */
			Term_redraw();
		}

#ifdef ALLOW_COLORS

		/* Dump colors */
		else if (evt == DUMP_COL)
		{
			dump_pref_file(dump_colors, "Dump Colors", 15);
		}

		/* Edit colors */
		else if (evt == MOD_COL)
		{
			static byte a = 0;

			/* Prompt */
			prt("Command: Modify colors", 8, 0);

			/* Hack -- query until done */
			while (1)
			{
				cptr name;

				/* Clear */
				clear_from(10);

				/* Exhibit the normal colors */
				for (i = 0; i < BASIC_COLORS; i++)
				{
					/* Exhibit this color */
					Term_putstr(i*4, 20, -1, a, "###");

					/* Exhibit all colors */
					Term_putstr(i*4, 22, -1, (byte)i, format("%3d", i));
				}

				/* Describe the color */
				name = ((a < BASIC_COLORS) ? color_names[a] : "undefined");

				/* Describe the color */
				Term_putstr(5, 10, -1, TERM_WHITE,
					    format("Color = %d, Name = %s", a, name));

				/* Label the Current values */
				Term_putstr(5, 12, -1, TERM_WHITE,
					    format("K = 0x%02x / R,G,B = 0x%02x,0x%02x,0x%02x",
						   angband_color_table[a][0],
						   angband_color_table[a][1],
						   angband_color_table[a][2],
						   angband_color_table[a][3]));

				/* Prompt */
				Term_putstr(0, 14, -1, TERM_WHITE,
					    "Command (n/N/k/K/r/R/g/G/b/B): ");

				/* Get a command */
				cx = inkey();

				/* All done */
				if (cx == ESCAPE) break;

				/* Analyze */
				if (cx == 'n') a = (byte)(a + 1);
				if (cx == 'N') a = (byte)(a - 1);
				if (cx == 'k') angband_color_table[a][0] = (byte)(angband_color_table[a][0] + 1);
				if (cx == 'K') angband_color_table[a][0] = (byte)(angband_color_table[a][0] - 1);
				if (cx == 'r') angband_color_table[a][1] = (byte)(angband_color_table[a][1] + 1);
				if (cx == 'R') angband_color_table[a][1] = (byte)(angband_color_table[a][1] - 1);
				if (cx == 'g') angband_color_table[a][2] = (byte)(angband_color_table[a][2] + 1);
				if (cx == 'G') angband_color_table[a][2] = (byte)(angband_color_table[a][2] - 1);
				if (cx == 'b') angband_color_table[a][3] = (byte)(angband_color_table[a][3] + 1);
				if (cx == 'B') angband_color_table[a][3] = (byte)(angband_color_table[a][3] - 1);

				/* Hack -- react to changes */
				Term_xtra(TERM_XTRA_REACT, 0);

				/* Hack -- redraw */
				Term_redraw();
			}
		}

#endif /* ALLOW_COLORS */

		/* Clear screen */
		clear_from(0);
	}


	/* Load screen */
	screen_load();
}



/*** Non-complex menu actions ***/

/*
 * Set base delay factor
 */
static void do_cmd_delay(void)
{
	/* Prompt */
	prt("Command: Base Delay Factor", 20, 0);

	/* Get a new value */
	while (1)
	{
		char cx;
		int msec = op_ptr->delay_factor * op_ptr->delay_factor;
		prt(format("Current base delay factor: %d (%d msec)",
					   op_ptr->delay_factor, msec), 22, 0);
		prt("New base delay factor (0-9 or ESC to accept): ", 21, 0);

		/* Get input */
		cx = inkey();

		/* Process input */
		if (cx == ESCAPE)
			break;
		if (isdigit((unsigned char) cx))
			op_ptr->delay_factor = D2I(cx);
		else
			bell("Illegal delay factor!");
	}
}


/*
 * Set hitpoint warning level
 */
static void do_cmd_hp_warn(void)
{
	/* Prompt */
	prt("Command: Hitpoint Warning", 20, 0);

	/* Get a new value */
	while (1)
	{
		char cx;
		prt(format("Current hitpoint warning: %2d%%",
			   op_ptr->hitpoint_warn * 10), 22, 0);
		prt("New hitpoint warning (0-9 or ESC to accept): ", 21, 0);

		/* Get input */
		cx = inkey();

		/* Process input */
		if (cx == ESCAPE)
			break;
		if (isdigit((unsigned char) cx))
			op_ptr->hitpoint_warn = D2I(cx);
		else
			bell("Illegal hitpoint warning!");
	}
}


/*
 * Ask for a "user pref file" and process it.
 *
 * This function should only be used by standard interaction commands,
 * in which a standard "Command:" prompt is present on the given row.
 *
 * Allow absolute file names?  XXX XXX XXX
 */
static void do_cmd_pref_file_hack(long row)
{
	char ftmp[80];

	/* Prompt */
	prt("Command: Load a user pref file", row, 0);

	/* Prompt */
	prt("File: ", row + 2, 0);

	/* Default filename */
	strnfmt(ftmp, sizeof ftmp, "%s.prf", op_ptr->base_name);

	/* Ask for a file (or cancel) */
	if (!askfor_aux(ftmp, sizeof ftmp, NULL)) return;

	/* Process the given filename */
	if (process_pref_file(ftmp))
	{
		/* Mention failure */
		msg_format("Failed to load '%s'!", ftmp);
	}
	else
	{
		/* Mention success */
		msg_format("Loaded '%s'.", ftmp);
	}

	inkey_ex();
}


/*
 * Write options to a file.
 */
static void do_dump_options(void *unused, const char *title)
{
	dump_pref_file(option_dump, title, 20);
}



/*** Main menu definitions and display ***/

/*
 * Definition of the options menu.
 *
 * XXX Too many entries.
 */

static event_action option_actions [] = 
{
	{'1', "Interface options", do_cmd_options_aux, (void*)0}, 
	{'2', "Display options", do_cmd_options_aux, (void*)1},
	{'3', "Warning and disturbance options", do_cmd_options_aux, (void*)2}, 
	{'4', "Birth (difficulty) options", do_cmd_options_aux, (void*)3}, 
	{'5', "Cheat options", do_cmd_options_aux, (void*)4}, 
	{0, 0, 0, 0}, /* Load and append */
	{'W', "Subwindow display settings", (action_f) do_cmd_options_win, 0}, 
	{'S', "Item squelch settings", (action_f) do_cmd_options_item, 0}, 
	{'D', "Set base delay factor", (action_f) do_cmd_delay, 0}, 
	{'H', "Set hitpoint warning", (action_f) do_cmd_hp_warn, 0}, 
	{0, 0, 0, 0}, /* Special choices */
	{'L', "Load a user pref file", (action_f) do_cmd_pref_file_hack, (void*)20},
	{'A', "Dump options", do_dump_options, 0}, 
	{0, 0, 0, 0}, /* Interact with */	
	{'M', "Interact with macros (advanced)", (action_f) do_cmd_macros, 0},
	{'V', "Interact with visuals (advanced)", (action_f) do_cmd_visuals, 0},
	{'C', "Interact with colours (advanced)", (action_f) do_cmd_colors, 0},
};

static menu_type option_menu;

static char tag_opt_main(menu_type *menu, int oid)
{
	if (option_actions[oid].id)
		return option_actions[oid].id;

	return 0;
}

static int valid_opt_main(menu_type *menu, int oid)
{
	if (option_actions[oid].name)
		return 1;

	return 0;
}

static void display_opt_main(menu_type *menu, int oid, bool cursor, int row, int col, int width)
{
	byte attr = curs_attrs[CURS_KNOWN][(int)cursor];

	if (option_actions[oid].name)
		c_prt(attr, option_actions[oid].name, row, col);
}


static const menu_iter options_iter =
{
	0,
	tag_opt_main,
	valid_opt_main,
	display_opt_main,
	NULL
};


/*
 * Display the options main menu.
 */
void do_cmd_options(void)
{
	int cursor = 0;
	event_type c = EVENT_EMPTY;

	screen_save();
	menu_layout(&option_menu, &SCREEN_REGION);

	while (c.key != ESCAPE)
	{
		clear_from(0);
		c = menu_select(&option_menu, &cursor, 0);
		if (c.type == EVT_SELECT && option_actions[cursor].action)
		{
			option_actions[cursor].action(option_actions[cursor].data,
			                              option_actions[cursor].name);
		}
	}

	screen_load();
}


static void do_cmd_self_knowledge(void *obj, const char *name)
{
	/* display self knowledge we already know about. */
	self_knowledge(FALSE);
}

/*
 * Definition of the "player knowledge" menu.
 */
static menu_item knowledge_actions[] =
{
	{{0, "Display object knowledge", do_cmd_knowledge_objects, 0}, '1'},
	{{0, "Display artifact knowledge", do_cmd_knowledge_artifacts, 0}, '2'},
	{{0, "Display ego item knowledge", do_cmd_knowledge_ego_items, 0}, '3'},
	{{0, "Display monster knowledge", do_cmd_knowledge_monsters, 0}, '4'},
	{{0, "Display feature knowledge", do_cmd_knowledge_features, 0}, '5'},
	{{0, "Display self-knowledge", do_cmd_self_knowledge, 0}, '6'},
};

static menu_type knowledge_menu;


/*
 * Display the "player knowledge" menu.
 */
void do_cmd_knowledge(void)
{
	int cursor = 0;
	int i;
	event_type c = EVENT_EMPTY;
	region knowledge_region = { 0, 0, -1, 11 };

	/* Grey out menu items that won't display anything */
	if (collect_known_artifacts(NULL, 0) > 0)
		knowledge_actions[1].flags = 0;
	else
		knowledge_actions[1].flags = MN_GREYED;

	knowledge_actions[2].flags = MN_GREYED;
	for (i = 0; i < z_info->e_max; i++)
	{
		if (e_info[i].everseen || cheat_xtra)
		{
			knowledge_actions[2].flags = 0;
			break;
		}
	}

	if (count_known_monsters() > 0)
		knowledge_actions[3].flags = 0;
	else
		knowledge_actions[3].flags = MN_GREYED;

	screen_save();
	menu_layout(&knowledge_menu, &knowledge_region);

	while (c.key != ESCAPE)
	{
		clear_from(0);
		c = menu_select(&knowledge_menu, &cursor, 0);
	}

	screen_load();
}



/* Keep macro counts happy. */
static void cleanup_cmds(void)
{
	FREE(obj_group_order);
}


/*
 * Initialise all menus used here.
 */
void init_cmd4_c(void)
{
	/* some useful standard command keys */
	static const char cmd_keys[] = { ARROW_LEFT, ARROW_RIGHT, '\0' };

	/* Initialize the menus */
	menu_type *menu;                 

	/* options screen selection menu */
	menu = &option_menu;
	WIPE(menu, menu_type);
	menu_set_id(menu, OPTION_MENU);
	menu->title = "Options Menu";
	menu->menu_data = option_actions;
	menu->flags = MN_CASELESS_TAGS;
	menu->cmd_keys = cmd_keys;
	menu->count = N_ELEMENTS(option_actions);
	menu_init2(menu, find_menu_skin(MN_SCROLL), &options_iter, &SCREEN_REGION);

	/* Initialize the options toggle menu */
	menu = &option_toggle_menu;
	WIPE(menu, menu_type);
	menu->prompt = "Set option (y/n/t), '?' for information";
	menu->cmd_keys = "?Yy\n\rNnTt\x8C"; /* \x8c = ARROW_RIGHT */
	menu->selections = "abcdefghijklmopqrsuvwxz";
	menu->count = OPT_PAGE_PER;
	menu->flags = MN_DBL_TAP;
	menu_init2(menu, find_menu_skin(MN_SCROLL), &options_toggle_iter, &SCREEN_REGION);

	/* macro menu */
	menu = &macro_menu;
	WIPE(menu, menu_type);
	menu_set_id(menu, MACRO_MENU);
	menu->title = "Interact with macros";
	menu->cmd_keys = cmd_keys;
	menu->selections = default_choice;
	menu->menu_data = macro_actions;
	menu->count = N_ELEMENTS(macro_actions);
	menu_init(menu, MN_SCROLL, MN_EVT, &SCREEN_REGION);

	/* visuals menu */
	menu = &visual_menu;
	WIPE(menu, menu_type);
	menu_set_id(menu, VISUAL_MENU);
	menu->title = "Interact with visuals";
	menu->cmd_keys = cmd_keys;
	menu->selections = default_choice;
	menu->menu_data = visual_menu_items;
	menu->count = N_ELEMENTS(visual_menu_items);
	menu_init(menu, MN_SCROLL, MN_EVT, &SCREEN_REGION);

	/* colors menu */
	menu = &color_menu;
	WIPE(menu, menu_type);
	menu_set_id(menu, COLOR_MENU);
	menu->title = "Interact with colors";
	menu->cmd_keys = cmd_keys;
	menu->selections = default_choice;
	menu->menu_data = color_events;
	menu->count = N_ELEMENTS(color_events);
	menu_init(menu, MN_SCROLL, MN_EVT, &SCREEN_REGION);

	/* knowledge menu */
	menu = &knowledge_menu;
	WIPE(menu, menu_type);
	menu_set_id(menu, KNOWLEDGE_MENU);
	menu->title = "Display current knowledge";
	menu->menu_data = knowledge_actions;
	menu->count = N_ELEMENTS(knowledge_actions),
	menu_init(menu, MN_SCROLL, MN_ACT, &SCREEN_REGION);

	/* initialize other static variables */
	if (!obj_group_order)
	{
		int i;
		int gid = -1;

		C_MAKE(obj_group_order, TV_GOLD+1, int);
		atexit(cleanup_cmds);

		/* Sllow for missing values */
		for (i = 0; i <= TV_GOLD; i++)
			obj_group_order[i] = -1;

		for (i = 0; 0 != object_text_order[i].tval; i++)
		{
			if (object_text_order[i].name) gid = i;
			obj_group_order[object_text_order[i].tval] = gid;
		}
	}
}








/*** Non-knowledge/option stuff ***/

/*
 * Note something in the message recall
 */
void do_cmd_note(void)
{
	char tmp[80];

	/* Default */
	my_strcpy(tmp, "", sizeof(tmp));

	/* Input */
	if (!get_string("Note: ", tmp, 80)) return;

	/* Ignore empty notes */
	if (!tmp[0] || (tmp[0] == ' ')) return;

	/* Add the note to the message recall */
	msg_format("Note: %s", tmp);
}


/*
 * Mention the current version
 */
void do_cmd_version(void)
{
	/* Silly message */
	msg_format("You are playing %s %s.  Type '?' for more info.",
		       VERSION_NAME, VERSION_STRING);
}


/*
 * Ask for a "user pref line" and process it
 */
void do_cmd_pref(void)
{
	char tmp[80];

	/* Default */
	my_strcpy(tmp, "", sizeof(tmp));

	/* Ask for a "user pref command" */
	if (!get_string("Pref: ", tmp, 80)) return;

	/* Process that pref command */
	(void)process_pref_file_command(tmp);
}



/*
 * Array of feeling strings
 */
static const char *feeling_text[] =
{
	"Looks like any other level.",
	"You feel there is something special about this level.",
	"You have a superb feeling about this level.",
	"You have an excellent feeling...",
	"You have a very good feeling...",
	"You have a good feeling...",
	"You feel strangely lucky...",
	"You feel your luck is turning...",
	"You like the look of this place...",
	"This level can't be all bad...",
	"What a boring place..."
};


/*
 * Note that "feeling" is set to zero unless some time has passed.
 * Note that this is done when the level is GENERATED, not entered.
 */
void do_cmd_feeling(void)
{
	/* Verify the feeling */
	if (feeling >= N_ELEMENTS(feeling_text))
		feeling = N_ELEMENTS(feeling_text) - 1;

	/* No useful feeling in town */
	if (!p_ptr->depth)
	{
		msg_print("Looks like a typical town.");
		return;
	}

	/* Display the feeling */
	msg_print(feeling_text[feeling]);
}



/*** Screenshot loading/saving code ***/

/*
 * Encode the screen colors
 */
static const char hack[BASIC_COLORS+1] = "dwsorgbuDWvyRGBU";


/*
 * Hack -- load a screen dump from a file
 *
 * ToDo: Add support for loading/saving screen-dumps with graphics
 * and pseudo-graphics.  Allow the player to specify the filename
 * of the dump.
 */
void do_cmd_load_screen(void)
{
	int i, y, x;

	byte a = 0;
	char c = ' ';

	bool okay = TRUE;

	FILE *fp;

	char buf[1024];


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, "dump.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Oops */
	if (!fp) return;


	/* Save screen */
	screen_save();


	/* Clear the screen */
	Term_clear();


	/* Load the screen */
	for (y = 0; okay && (y < 24); y++)
	{
		/* Get a line of data */
		if (my_fgets(fp, buf, sizeof(buf))) okay = FALSE;


		/* Show each row */
		for (x = 0; x < 79; x++)
		{
			/* Put the attr/char */
			Term_draw(x, y, TERM_WHITE, buf[x]);
		}
	}

	/* Get the blank line */
	if (my_fgets(fp, buf, sizeof(buf))) okay = FALSE;


	/* Dump the screen */
	for (y = 0; okay && (y < 24); y++)
	{
		/* Get a line of data */
		if (my_fgets(fp, buf, sizeof(buf))) okay = FALSE;

		/* Dump each row */
		for (x = 0; x < 79; x++)
		{
			/* Get the attr/char */
			(void)(Term_what(x, y, &a, &c));

			/* Look up the attr */
			for (i = 0; i < BASIC_COLORS; i++)
			{
				/* Use attr matches */
				if (hack[i] == buf[x]) a = i;
			}

			/* Put the attr/char */
			Term_draw(x, y, a, c);
		}
	}


	/* Close it */
	my_fclose(fp);


	/* Message */
	msg_print("Screen dump loaded.");
	message_flush();


	/* Load screen */
	screen_load();
}


/*
 * Save a simple text screendump.
 */
void do_cmd_save_screen_text(void)
{
	int y, x;

	byte a = 0;
	char c = ' ';

	FILE *fff;

	char buf[1024];

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, "dump.txt");

	/* File type is "DATA" -- needs to be opened in Angband to view */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Append to the file */
	fff = my_fopen(buf, "w");

	/* Oops */
	if (!fff) return;


	/* Save screen */
	screen_save();


	/* Dump the screen */
	for (y = 0; y < 24; y++)
	{
		/* Dump each row */
		for (x = 0; x < 79; x++)
		{
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

	/* Skip a line */
	fprintf(fff, "\n");


	/* Dump the screen */
	for (y = 0; y < 24; y++)
	{
		/* Dump each row */
		for (x = 0; x < 79; x++)
		{
			/* Get the attr/char */
			(void)(Term_what(x, y, &a, &c));

			/* Dump it */
			buf[x] = hack[a & 0x0F];
		}

		/* Terminate */
		buf[x] = '\0';

		/* End the row */
		fprintf(fff, "%s\n", buf);
	}

	/* Skip a line */
	fprintf(fff, "\n");


	/* Close it */
	my_fclose(fff);


	/* Message */
	msg_print("Screen dump saved.");
	message_flush();


	/* Load screen */
	screen_load();
}


/*
 * Hack -- save a screen dump to a file in html format
 */
void do_cmd_save_screen_html(int mode)
{
	size_t i;

	FILE *fff;
	char file_name[1024];
	char tmp_val[256];

	typedef void (*dump_func)(FILE *);
	dump_func dump_visuals [] = 
		{ dump_monsters, dump_features, dump_objects, dump_flavors, dump_colors };

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Ask for a file */
	if (mode == 0) my_strcpy(tmp_val, "dump.html", sizeof(tmp_val));
	else my_strcpy(tmp_val, "dump.txt", sizeof(tmp_val));
	if (!get_string("File: ", tmp_val, sizeof(tmp_val))) return;

	/* Save current preferences */
	path_build(file_name, 1024, ANGBAND_DIR_USER, "dump.prf");
	fff = my_fopen(file_name, "w");

	/* Check for failure */
	if (!fff)
	{
		msg_print("Screen dump failed.");
		message_flush();
		return;
	}

	/* Dump all the visuals */
	for (i = 0; i < N_ELEMENTS(dump_visuals); i++)
		dump_visuals[i](fff);

	my_fclose(fff);

	/* Dump the screen with raw character attributes */
	reset_visuals(FALSE);
	do_cmd_redraw();
	html_screenshot(tmp_val, mode);

	/* Recover current graphics settings */
	reset_visuals(TRUE);
	process_pref_file(file_name);
	fd_kill(file_name);
	do_cmd_redraw();

	msg_print("HTML screen dump saved.");
	message_flush();
}


/*
 * Hack -- save a screen dump to a file
 */
void do_cmd_save_screen(void)
{
	msg_print("Dump type [(t)ext; (h)tml; (f)orum embedded html]:");

	while (TRUE)
	{
		char c = inkey();

		switch (c)
		{
			case ESCAPE:
				return;

			case 't':
				do_cmd_save_screen_text();
				return;

			case 'h':
				do_cmd_save_screen_html(0);
				return;

			case 'f':
				do_cmd_save_screen_html(1);
				return;
		}
	}
}

