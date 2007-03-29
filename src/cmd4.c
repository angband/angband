/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 *
 * UnAngband (c) 2001-6 Andrew Doull. Modifications to the Angband 2.9.1
 * source code are released under the Gnu Public License. See www.fsf.org
 * for current GPL license details. Addition permission granted to
 * incorporate modifications in all Angband variants as defined in the
 * Angband variants FAQ. See rec.games.roguelike.angband for FAQ.
 *
 */

/*
 * Code cleanup -- Pete Mack 02/2007 (No copyright) 
 * Use proper function tables and database methodology.
 * Tables are now tables, not multiline conditionals.
 * Joins are now relational, not ad hoc.
 * Function tables are used for iteration where reasonable. (C-style class model)
 */

#include "angband.h"
#include "ui.h"



typedef struct {
		int maxnum; /* Maximum possible item count for this class */
		bool easy_know; /* Items don't need to be IDed to recognize membership */

		const char *(*name)(int gid); /* name of this group */

		/* Compare, in group and display order */
		/* Optional, if already sorted */
		int (*gcomp)(const void *, const void *); /* Compare GroupIDs of two oids */
		int (*group)(int oid); /* Group ID for of an oid */
		bool (*aware)(object_type *obj); /* Object is known sufficiently for group */

} group_funcs;

typedef struct {

		/* Display object label (possibly with cursor) at given screen location. */
		void (*display_label)(int col, int row, bool cursor, int oid);

		void (*lore)(int oid); /* Dump known lore to screen*/

		/* Required only for objects with modifiable display attributes */
		/* Unknown 'flavors' return flavor attributes */
		char *(*xchar)(int oid); /* get character attr for OID (by address) */
		byte *(*xattr)(int oid); /* get color attr for OID (by address) */

		/* Required only for manipulable (ordinary) objects */
		/* Address of inscription.  Unknown 'flavors' return null  */
		u16b *(*note)(int oid);

} member_funcs;


/* Helper class for generating joins */
typedef struct join {
		int oid;
		int gid;
} join_t;

/* A default group-by */
static join_t *default_join;
static int default_join_cmp(const void *a, const void *b)
{
		join_t *ja = &default_join[*(int*)a];
		join_t *jb = &default_join[*(int*)b];
		int c = ja->gid - jb->gid;
		if (c) return c;
		return ja->oid - jb->oid;
}
static int default_group(int oid) { return default_join[oid].gid; }


static int *obj_group_order;
/*
 * Description of each monster group.
 */
static struct {
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
	{ "$?!_",	"Mimics" },
	{ "m",		"Molds" },
	{ ",",		"Mushroom Patches" },
	{ "n",		"Nagas" },
	{ "o",		"Orcs" },
	{ "tp",		"Humans" },
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

static void browser_mouse(key_event ke, int *column, int *grp_cur, int grp_cnt, 
				int *list_cur, int list_cnt, int col0, int row0,
				int grp0, int list0, int *delay);

static void browser_cursor(char ch, int *column, int *grp_cur, int grp_cnt, 
				int *list_cur, int list_cnt);

static bool visual_mode_command(key_event ke, bool *visual_list_ptr, 
				int height, int width, 
				byte *attr_top_ptr, char *char_left_ptr, 
				byte *cur_attr_ptr, char *cur_char_ptr,
				int col, int row, int *delay);

static void place_visual_list_cursor(int col, int row, byte a,
				byte c, byte attr_top, byte char_left);

static void dump_pref_file(void (*dump)(FILE*), const char *title);

/*
 *  Clipboard variables for copy&paste in visual mode
 */
static byte attr_idx = 0;
static byte char_idx = 0;

/*
 * Return a specific ordering for the features
 */
int feat_order(int feat)
{
		feature_type *f_ptr = &f_info[feat];
		switch(f_ptr->d_char) {
				case '.': 				return 0;
				case '^': 				return 1;
				case '\'': case '+': 	return 2;
				case '<': case '>':		return 3;
				case '#':				return 4;
				case '*': case '%' :	return 5;
				case ';': case ':' :	return 6;
				default:
					if(isdigit(f_ptr->d_char)) return 7;
					return 8;
		}
};


/* HACK */
static const int use_dbltile = 0;
static const int use_trptile = 0;

/* Emit a 'graphical' symbol and a padding character if appropriate */
static void big_pad(int col, int row, byte a, byte c)
{
		Term_putch(col, row, a, c);
		if(!use_bigtile) return;
		if (a &0x80) Term_putch(col+1, row, 255, -1);
		else Term_putch(col+1, row, 1, ' ');
}

static int actual_width(int width) {
		if (use_trptile) width = width * 3;
		else if(use_dbltile) width *= 2;
		if(use_bigtile) width *= 2;
		return width;
}
static int actual_height(int height) {
		if(use_bigtile) height *= 2;
		if (use_trptile) height = height * 3 / 2;
		else if(use_dbltile) height *= 2;
		return height;
}

static int logical_width(int width)
{
		int div = 1;
		if(use_trptile) div = 3;
		else if(use_dbltile) div *= 2;
		if(use_bigtile) div *= 2;
		return width / div;
}

static int logical_height(int height)
{
		int div = 1;
		if(use_trptile) {
				height *= 2;
				div = 3;
		}
		else if(use_dbltile) div = 2;
		if(use_bigtile) div *= 2;
		return height / div;
}

/*
 * Interact with inscriptions.
 * Create a copy of an existing quark, except if the quark has '=x' in it, 
 * If an quark has '=x' in it remove it from the copied string, otherwise append it where 'x' is ch.
 * Return the new quark location.
 */
static int auto_note_modify(int note, char ch)
{
		char tmp[80];

		cptr s;

		/* Paranoia */
		if (!ch) return(note);

		/* Null length string to start */
		tmp[0] = '\0';

		/* Inscription */
		if (note)
		{

				/* Get the inscription */
				s = quark_str(note);

				/* Temporary copy */
				my_strcpy(tmp,s,80);

				/* Process inscription */
				while (s)
				{

						/* Auto-pickup on "=g" */
						if (s[1] == ch)
						{

								/* Truncate string */
								tmp[strlen(tmp)-strlen(s)] = '\0';

								/* Overwrite shorter string */
								my_strcat(tmp,s+2,80);

								/* Create quark */
								return(quark_add(tmp));
						}

						/* Find another '=' */
						s = strchr(s + 1, '=');
				}
		}

		/* Append note */
		my_strcat(tmp,format("=%c",ch),80);

		/* Create quark */
		return(quark_add(tmp));
}

/*
 * Display a list with a cursor
 */
static void display_group_list(int col, int row, int wid, int per_page,
				int start, int max, int cursor, const cptr group_text[])
{
		int i, pos;

		/* Display lines until done */
		for (i = 0, pos = start; i < per_page && pos < max; i++, pos++)
		{
				char buffer[21];
				byte attr = curs_attrs[CURS_KNOWN][cursor == pos];

				/* Erase the line */
				Term_erase(col, row + i, wid);

				/* Display it (width should not exceed 20) */
				strncpy(buffer, group_text[pos], 20);
				buffer[20] = 0;
				c_put_str(attr, buffer, row + i, col);
		}
		/* Wipe the rest? */
}

/*
 * Display the members of a list.
 * Aware of inscriptions, wizard information, and string-formatted visual data.
 * label function must display actual visuals, to handle illumination, etc
 */
static void display_member_list(int col, int row, int wid, int per_page,
				int start, int o_count, int cursor, int object_idx [],
				member_funcs o_funcs)
{
		int i, pos;

		for(i = 0, pos = start; i < per_page && pos < o_count; i++, pos++) {
				int oid = object_idx[pos];
				byte attr = curs_attrs[CURS_KNOWN][cursor == oid];

				/* Print basic label */
				o_funcs.display_label(col, row + i, pos == cursor, oid);

				/* Show inscription, if applicable, aware and existing */
				if(o_funcs.note && o_funcs.note(oid) && *o_funcs.note(oid)) {
						c_put_str(TERM_YELLOW,quark_str(*o_funcs.note(oid)), row+i, 65);
				}

				if (p_ptr->wizard)
						c_put_str(attr, format("%d", oid), row, 60);

				/* Do visual mode */
				if(per_page == 1 && o_funcs.xattr) {
						char c = *o_funcs.xchar(oid);
						byte a = *o_funcs.xattr(oid);
						c_put_str(attr, format((c & 0x80) ? "%02x/%02x" : "%02x/%d", a, c), row + i, 60);
				}
		}

		/* Clear remaining lines */
		for (; i < per_page; i++)
		{
				Term_erase(col, row + i, 255);
		}
}


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

	int g_cur = 0, grp_old = -1, grp_top = 0; /* group list positions */
	int o_cur = 0, object_top = 0; /* object list positions */
	int g_o_count = 0; /* object count for group */
	int o_first = 0, g_o_max = 0; /* group limits in object list */
	int oid = -1, old_oid = -1;  /* object identifiers */

	/* display state variables */
	bool visual_list = FALSE;
	byte attr_top = 0;
	char char_left = 0;
	int note_idx = 0;

	int delay = 0;
	int column = 0;

	bool flag = FALSE;
	bool redraw = TRUE;

	int browser_rows;
	int wid, hgt;
	int i;
	int prev_g = -1;

	/* Get size */
	Term_get_size(&wid, &hgt);
	browser_rows = hgt - 8;

	/* Do the group by. ang_sort only works on (void **) */
	/* Maybe should make this a precondition? */
	if(g_funcs.gcomp)
			qsort(obj_list, o_count, sizeof(*obj_list), g_funcs.gcomp);

	C_MAKE(g_list, max_group+1, int);
	C_MAKE(g_offset, max_group+1, int);

	for(i = 0; i < o_count; i++) {
		if(prev_g != g_funcs.group(obj_list[i])) {
			prev_g = g_funcs.group(obj_list[i]);
			g_offset[grp_cnt] = i;
			g_list[grp_cnt++] = prev_g;
		}
	}
	g_offset[grp_cnt] = o_count;
	g_list[grp_cnt] = -1;


	/* The compact set of group names, in display order */
	C_MAKE(g_names, grp_cnt, const char **);
	for (i = 0; i < grp_cnt; i++) {
		int len;
		g_names[i] = g_funcs.name(g_list[i]);
		len = strlen(g_names[i]);
		if(len > g_name_len) g_name_len = len;
	}
	if(g_name_len >= 20) g_name_len = 20;

	while ((!flag) && (grp_cnt))
	{
		key_event ke;

		if (redraw)
		{
			clear_from(0);
			/* Hack: This could be cleaner */
			prt( format("Knowledge - %s", title), 2, 0);
			prt( "Group", 4, 0);
			prt("Name", 4, g_name_len + 3);
			move_cursor(4, 65);
			if(o_funcs.note)
				Term_addstr(-1, TERM_WHITE, "Inscribed ");
			if(otherfields)
				Term_addstr(-1, TERM_WHITE, otherfields);

			for (i = 0; i < 78; i++)
			{
				Term_putch(i, 5, TERM_WHITE, '=');
			}

			for (i = 0; i < browser_rows; i++)
			{
				Term_putch(g_name_len + 1, 6 + i, TERM_WHITE, '|');
			}

			redraw = FALSE;
		}

		/* Scroll group list */
		if (g_cur < grp_top) grp_top = g_cur;
		if (g_cur >= grp_top + browser_rows) grp_top = g_cur - browser_rows + 1;
		if (grp_top + browser_rows >= grp_cnt) grp_top = grp_cnt - browser_rows;
		if(grp_top < 0) grp_top = 0;

		if(g_cur != grp_old) {
			o_first = o_cur = g_offset[g_cur];
			object_top = o_first;
			g_o_count = g_offset[g_cur+1] - g_offset[g_cur];
			g_o_max = g_offset[g_cur+1];
			grp_old = g_cur;
			old_oid = -1;
		}

		/* Display a scrollable list of groups */
		display_group_list(0, 6, g_name_len, browser_rows,
											grp_top, grp_cnt, g_cur, g_names);

		/* Scroll object list */
		if(o_cur >= g_o_max) o_cur = g_o_max-1;
		if(o_cur < o_first) o_cur = o_first;
		if (o_cur < object_top) object_top = o_cur;
		if (o_cur >= object_top + browser_rows)
			object_top = o_cur - browser_rows + 1;
		if (object_top + browser_rows >= g_o_max)
			object_top = g_o_max - browser_rows;
		if(object_top < o_first) object_top = o_first;

		oid = obj_list[o_cur];

		if(!visual_list) {
			/* Display a list of objects in the current group */
			display_member_list(g_name_len + 3, 6, g_name_len, browser_rows, 
										object_top, g_o_max, o_cur, obj_list, o_funcs);
		}
		else
		{
			/* Edit 1 group member */
			object_top = o_cur;
			/* Display a single-row list */
			display_member_list(g_name_len + 3, 6, g_name_len, 1, 
								o_cur, g_o_max, o_cur, obj_list, o_funcs);
			/* Display visual list below first object */
			display_visual_list(g_name_len + 3, 7, browser_rows-1,
										wid - (g_name_len + 3), attr_top, char_left);
		}
		/* Prompt */
		{
			const char *pedit = (!o_funcs.xattr) ? "" :
					(!(attr_idx|char_idx) ? ", 'c' to copy" : ", 'c', 'p' to paste");

			const char *pnote = (!o_funcs.note || !o_funcs.note(oid)) ?
							"" : ((note_idx) ? ", '\\' to re-inscribe"  :  "");

			const char *pnote1 = (!o_funcs.note || !o_funcs.note(oid)) ?
									"" : ", '{', '}', 'k', 'g', ...";

			const char *pvs = (!o_funcs.xattr) ? "" : ", 'v' for visuals";

			if(visual_list)
				prt(format("<dir>, 'r' to recall, ENTER to accept%s, ESC", pedit), hgt-1, 0);
			else 
				prt(format("<dir>, 'r' to recall%s ESC%s%s%s",
										pvs, pedit, pnote, pnote1), hgt-1, 0);
		}

		handle_stuff();

		if (visual_list)
		{
			place_visual_list_cursor(g_name_len + 3, 7, *o_funcs.xattr(oid), 
										*o_funcs.xchar(oid), attr_top, char_left);
		}
		else if (!column)
		{
			Term_gotoxy(0, 6 + (g_cur - grp_top));
		}
		else
		{
			Term_gotoxy(g_name_len + 3, 6 + (o_cur - object_top));
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

		switch (ke.key)
		{

			case ESCAPE:
			{
				flag = TRUE;
				break;
			}

			case '\xff':
			{
				/* Move the cursor */
				browser_mouse(ke, &column, &g_cur, grp_cnt,
								&o_cur, g_o_max, g_name_len + 3, 6,
								grp_top, object_top, &delay);
				if (!ke.index) break;
				if(oid != obj_list[o_cur])
						break;
			}

			case 'R':
			case 'r':
			{
				/* Recall on screen */
				if(oid >= 0)
					o_funcs.lore(oid);

				redraw = TRUE;
				break;
			}

			/* HACK: make this a function.  Replace g_funcs.aware() */
			case '{':
			case '}':
			case '\\':
				/* precondition -- valid object */
				if (o_funcs.note == NULL || o_funcs.note(oid) == 0)
						break;
			{
				char note_text[80] = "";
				u16b *note = o_funcs.note(oid);
				u16b old_note = *note;

				if (ke.key == '{')
				{
					/* Prompt */
					prt("Inscribe with: ", hgt, 0);

					/* Default note */
					if (old_note)
						sprintf(note_text, "%s", quark_str(old_note));

					/* Get a filename */
					if (!askfor_aux(note_text, sizeof note_text)) continue;

						/* Set the inscription */
						*note = quark_add(note_text);

					/* Set the current default note */
					note_idx = *note;
				}
				else if (ke.key == '}')
				{
					*note = 0;
				}
				else
				{
					/* '\\' */
					*note = note_idx;
				}

				/* Process existing objects */
				for (i = 1; i < o_max; i++)
				{
					/* Get the object */
					object_type *i_ptr = &o_list[i];

					/* Skip dead or differently sourced items */
					if (!i_ptr->k_idx || i_ptr->note != old_note) continue;

					/* Not matching item */
					if (!g_funcs.group(oid) != i_ptr->tval) continue;

					/* Auto-inscribe */
					if (g_funcs.aware(i_ptr) || cheat_peek)
					i_ptr->note = note_idx;
				}

				break;
			}

			default:
			{
				/* Move the cursor; disable roguelike keyset. */
				int omode = rogue_like_commands;
				rogue_like_commands = FALSE;
				if(target_dir(ke.key)) {
					browser_cursor(ke.key, &column, &g_cur, grp_cnt,
									&o_cur, g_o_max);
				}
				else if(o_funcs.note && o_funcs.note(oid)) {
					note_idx = auto_note_modify(*o_funcs.note(oid), ke.key);
					*o_funcs.note(oid) = note_idx;
				}
				rogue_like_commands = omode;
				break;
			}
		}
	}

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
		{
				Term_erase(col, row + i, width);
		}

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
static bool visual_mode_command(key_event ke, bool *visual_list_ptr, 
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
			if (*visual_list_ptr)
			{
				/* Cancel change */
				*cur_attr_ptr = attr_old;
				*cur_char_ptr = char_old;
				*visual_list_ptr = FALSE;

				return TRUE;
			}
			break;

		case '\n':
		case '\r':
			if (*visual_list_ptr)
			{
				/* Accept change */
				*visual_list_ptr = FALSE;

				return TRUE;
			}
			break;

		case 'V':
		case 'v':
			if (!*visual_list_ptr)
			{
				*visual_list_ptr = TRUE;

				*attr_top_ptr = (byte)MAX(0, (int)*cur_attr_ptr - frame_top);
				*char_left_ptr = (char)MAX(-128, (int)*cur_char_ptr - frame_left);

				attr_old = *cur_attr_ptr;
				char_old = *cur_char_ptr;

				return TRUE;
			}
			else
			{
				/* Cancel change */
				*cur_attr_ptr = attr_old;
				*cur_char_ptr = char_old;
				*visual_list_ptr = FALSE;

				return TRUE;
			}
			break;

		case 'C':
		case 'c':
			/* Set the visual */
			attr_idx = *cur_attr_ptr;
			char_idx = *cur_char_ptr;

			return TRUE;

		case 'P':
		case 'p':
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
							|| (c != *char_left_ptr + mx)) )
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
					if ((c == -128) && (ddx[d] < 0)) d = 0;
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

	if (use_dbltile || use_trptile)
		return;

	/* Display symbol */
	big_pad(66, row, a, c);

	/* Display kills */
	if (r_ptr->flags1 & (RF1_UNIQUE))
		put_str(format("%s", (r_ptr->max_num == 0)?  " dead" : "alive"), row, 70);
	else put_str(format("%5d", l_ptr->pkills), row, 70);
}


static int m_cmp_race(const void *a, const void *b) {
	monster_race *r_a = &r_info[default_join[*(int*)a].oid];
	monster_race *r_b = &r_info[default_join[*(int*)b].oid];
	int gid = default_join[*(int*)a].gid;
	/* group by */
	int c = gid - default_join[*(int*)b].gid;
	if(c) return c;
	/* order results */
	c = r_a->d_char - r_b->d_char;
	if(c && gid != 0) {
		/* UNIQUE group is ordered by level & name only */
		/* Others by order they appear in the group symbols */
		return strchr(monster_group[gid].chars, r_a->d_char)
			- strchr(monster_group[gid].chars, r_b->d_char);
	}
	c = r_a->level - r_b->level;
	if(c) return c;
	return strcmp(r_name + r_a->name, r_name + r_b->name);
}

static char *m_xchar(int oid) 
{ return &r_info[default_join[oid].oid].x_char; }
static byte *m_xattr(int oid)
{ return &r_info[default_join[oid].oid].x_attr; }
static const char *race_name(int gid) { return monster_group[gid].name; }
static void mon_lore(int oid) { screen_roff(default_join[oid].oid); inkey_ex(); }

/*
 * Display known monsters.
 */
static void do_cmd_knowledge_monsters(void)
{
		group_funcs r_funcs = {N_ELEMENTS(monster_group), FALSE, race_name,
							m_cmp_race, default_group, 0};

	member_funcs m_funcs = {display_monster, mon_lore, m_xchar, m_xattr, 0};
	
	int *monsters;
	int m_count = 0;
	int i;
	size_t j;

	for(i = 0; i < z_info->r_max; i++) {
		monster_race *r_ptr = &r_info[i];
		if(!cheat_know && !l_list[i].sights) continue;
		if(!r_ptr->name) continue;

		if(r_ptr->flags1 & RF1_UNIQUE) m_count++;
		for(j = 1; j < N_ELEMENTS(monster_group)-1; j++) {
			const char *pat = monster_group[j].chars;
			if(strchr(pat, r_ptr->d_char)) m_count++;
		}
	}

	C_MAKE(default_join, m_count, join_t);
	C_MAKE(monsters, m_count, int);

	m_count = 0;
	for(i = 0; i < z_info->r_max; i++) {
		monster_race *r_ptr = &r_info[i];
		if(!cheat_know && !l_list[i].sights) continue;
		if(!r_ptr->name) continue;
	
		for(j = 0; j < N_ELEMENTS(monster_group)-1; j++) {
			const char *pat = monster_group[j].chars;
			if(j == 0 && !(r_ptr->flags1 & RF1_UNIQUE)) 
				continue;
			else if(j > 0 && !strchr(pat, r_ptr->d_char))
				continue;

			monsters[m_count] = m_count;
			default_join[m_count].oid = i;
			default_join[m_count++].gid = j;
		}
	}

	display_knowledge("monsters", monsters, m_count, r_funcs, m_funcs,
						"Sym  Kills");
	FREE(monsters);
	FREE(default_join);
	default_join = 0;
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
	o_ptr->ident |= IDENT_STORE | IDENT_KNOWN;
	if(cheat_xtra) o_ptr->ident |= IDENT_MENTAL;

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
	if(c) return c;

	/* order by */
	c = a_a->sval - a_b->sval;
	if(c) return c;
	return strcmp(a_name+a_a->name, a_name+a_b->name);
}

static const char *kind_name(int gid)
{ return object_text_order[gid].name; }
static int art2tval(int oid) { return a_info[oid].tval; }

/*
 * Display known artifacts
 */
static void do_cmd_knowledge_artifacts(void)
{
	/* HACK -- should be TV_MAX */
	group_funcs obj_f = {TV_GOLD, FALSE, kind_name, a_cmp_tval, art2tval, 0};
	member_funcs art_f = {display_artifact, desc_art_fake, 0, 0, 0};


	int *artifacts;
	int a_count = 0;
	int i, j;

	C_MAKE(artifacts, z_info->a_max, int);
	
	/* Collect valid artifacts */
	for(i = 0; i < z_info->a_max; i++) {
		if((cheat_xtra || a_info[i].cur_num) && a_info[i].name)
			artifacts[a_count++] = i;
	}
	for(i = 0; !cheat_xtra && i < z_info->o_max; i++) {
		int a = o_list[i].name1;
		if(a && !object_known_p(&o_list[i])) {
			for(j = 0; j < a_count && a != artifacts[j]; j++);
			a_count -= 1;
			for(; j < a_count; j++) 
				artifacts[j] = artifacts[j+1];
		}
	}

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
	const char *cursed [] = {"permanently cursed", "heavily cursed", "cursed"};
	const char *xtra [] = {"sustain", "higher resistance", "ability"};
	object_type dummy;
	WIPE(&dummy, dummy);

	int f3, i;
	int e_idx = default_join[oid].oid;
	ego_item_type *e_ptr = &e_info[e_idx];

	/* Save screen */
	screen_save();

	/* Set text_out hook */
	text_out_hook = text_out_to_screen;

	/* Dump the name */
	c_prt(TERM_L_BLUE, format("%s %s", ego_grp_name(default_group(oid)),
										e_name+e_ptr->name), 0, 0);

	/* Begin recall */
	Term_gotoxy(0, 1);
	if(e_ptr->text) {
		int x, y;
		text_out(e_text + e_ptr->text);
		Term_locate(&x, &y);
		Term_gotoxy(0, y+1);
	}

	/* List ego flags */
	dummy.name2 = e_idx;
	object_info_out_flags = object_flags;
	object_info_out(&dummy);

	if(e_ptr->xtra) {
		text_out(format("It provides one random %s.", xtra[e_ptr->xtra - 1]));
	}

	for(i = 0, f3 = TR3_PERMA_CURSE; i < 3 ; f3 >>= 1, i++) {
		if(e_ptr->flags3 & f3) {
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
	/*group by */
	int c = default_join[*(int*)a].gid - default_join[*(int*)b].gid;
	if(c) return c;
	/* order by */
	return strcmp(e_name + ea->name, e_name + eb->name);
}

/*
 * Display known ego_items
 */
static void do_cmd_knowledge_ego_items(void)
{
	group_funcs obj_f =
		{TV_GOLD, FALSE, ego_grp_name, e_cmp_tval, default_group, 0};

	member_funcs ego_f = {display_ego_item, desc_ego_fake, 0, 0, 0/*e_note */ };

	int *egoitems;
	int e_count = 0;
	int i, j;

	/* HACK: currently no more than 3 tvals for one ego type */
	C_MAKE(egoitems, z_info->e_max*EGO_TVALS_MAX, int);
	C_MAKE(default_join, z_info->e_max*EGO_TVALS_MAX, join_t);
	for(i = 0; i < z_info->e_max; i++) {
		if(e_info[i].everseen || cheat_xtra) {
			for(j = 0; j < EGO_TVALS_MAX && e_info[i].tval[j]; j++)
			{
				int gid = obj_group_order[e_info[i].tval[j]];
				/* Ignore duplicate gids */
				if(j > 0 && gid == default_join[e_count-1].gid)
					continue;
				egoitems[e_count] = e_count;
				default_join[e_count].oid = i;
				default_join[e_count++].gid = gid; 
			}
		}
	}

	display_knowledge("ego items", egoitems, e_count, obj_f, ego_f, "");
	FREE(default_join);
	default_join = 0;
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
	
	/* Access the object */
	object_kind *k_ptr = &k_info[k_idx];

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
	strip_name(o_name, k_idx, FALSE);

	/* Display the name */
	c_prt(attr, o_name, row, col);


	/* Hack - don't use if double tile */
	if (use_dbltile || use_trptile)
		return;

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

	Term_gotoxy(0,0);
	/* Describe */
	object_info_screen(o_ptr);

	/* Load the screen */
	screen_load();

	(void)inkey_ex();
}

static int o_cmp_tval(const void *a, const void *b)
{
	object_kind *k_a = &k_info[*(int*)a];
	object_kind *k_b = &k_info[*(int*)b];
	/*group by */
	int ta = obj_group_order[k_a->tval];
	int tb = obj_group_order[k_b->tval];
	int c = ta - tb;
	if(c) return c;
	/* order by */
	c = k_a->aware - k_b->aware;
	if(c) return -c; /* aware has low sort weight */
	if(!k_a->aware) {
		return strcmp(flavor_text + flavor_info[k_a->flavor].text,
									flavor_text +flavor_info[k_b->flavor].text);
	}
	c = k_a->cost - k_b->cost;
	if(c) return c;
	return strcmp(k_name + k_a->name, k_name + k_b->name);
}
static int obj2gid(int oid) { return obj_group_order[k_info[oid].tval]; }
static char *o_xchar(int oid) {
	object_kind *k_ptr = &k_info[oid];
	if(!k_ptr->flavor || k_ptr->aware) return &k_ptr->x_char;
	else return &flavor_info[k_ptr->flavor].x_char;
}
static byte *o_xattr(int oid) {
	object_kind *k_ptr = &k_info[oid];
	if(!k_ptr->flavor || k_ptr->aware) return &k_ptr->x_attr;
	else return &flavor_info[k_ptr->flavor].x_attr;
}
#if 0
static u16b *o_note(int oid) {
	object_kind *k_ptr = &k_info[oid];
	if(!k_ptr->flavor || k_ptr->aware) return &k_ptr->note;
	else return 0;
}
#endif

/*
 * Display known objects
 */
static void do_cmd_knowledge_objects(void)
{
	group_funcs kind_f =
		{TV_GOLD, FALSE, kind_name, o_cmp_tval, obj2gid, 0};
	member_funcs obj_f =
		{display_object, desc_obj_fake, o_xchar, o_xattr, 0 /*o_note*/};

	int *objects;
	int o_count = 0;
	int i;

	C_MAKE(objects, z_info->k_max, int);

	for(i = 0; i < z_info->k_max; i++) {
		if(k_info[i].everseen || k_info[i].flavor || cheat_xtra) {
			int c = obj_group_order[k_info[i].tval];
			if(c >= 0) objects[o_count++] = i;
		}
	}
	display_knowledge("known objects", objects, o_count, kind_f, obj_f, "         Sym");
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


	if (use_dbltile || use_trptile) return;

	/* Display symbol */
	big_pad(68, row, f_ptr->x_attr, f_ptr->x_char);

	/* ILLUMINATION AND DARKNESS GO HERE */

}


static int f_cmp_fkind(const void *a, const void *b) {
	feature_type *fa = &f_info[*(int*)a];
	feature_type *fb = &f_info[*(int*)b];
	/* group by */
	int c = feat_order(*(int*)a) - feat_order(*(int*)b);
	if(c) return c;
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
static void do_cmd_knowledge_features(void)
{
	group_funcs fkind_f = {N_ELEMENTS(feature_group_text), FALSE,
							fkind_name, f_cmp_fkind, feat_order, 0};

	member_funcs feat_f = {display_feature, feat_lore, f_xchar, f_xattr, 0};

	int *features;
	int f_count = 0;
	int i;
	C_MAKE(features, z_info->f_max, int);

	for(i = 0; i < z_info->f_max; i++) {
		if(f_info[i].name == 0) continue;
		features[f_count++] = i; /* Currently no filter for features */
	}

	display_knowledge("features", features, f_count, fkind_f, feat_f, " Sym");
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
	
    if(0 != character_dungeon)
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
	if(0 != character_dungeon)
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
	key_event ke;

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
			get_name();
		}

		/* File dump */
		else if (ke.key == 'f')
		{
			char ftmp[80];

			sprintf(ftmp, "%s.txt", op_ptr->base_name);

			if (get_string("File name: ", ftmp, 80))
			{
				if (ftmp[0] && (ftmp[0] != ' '))
				{
					if (file_character(ftmp, FALSE))
					{
						msg_print("Character dump failed!");
					}
					else
					{
						msg_print("Character dump successful.");
					}
				}
			}
		}

		/* Toggle mode */
		else if ((ke.key == 'h') || (ke.key == '\xff') || ke.key == '4')
		{
			mode = (mode + 1) % 6;
		}

		/* Toggle mode */
		else if ((ke.key == 'l') || ke.key == '6')
		{
			mode = (mode + 6 - 1) % 6;
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
	key_event ke;

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
			if (!askfor_aux(shower, sizeof shower)) continue;

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
			if (!askfor_aux(finder, sizeof finder)) continue;

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
	sprintf(ftmp, "%s.prf", op_ptr->base_name);

	/* Ask for a file (or cancel) */
	if (!askfor_aux(ftmp, sizeof ftmp)) return;

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


static void display_option(menu_type *menu, int oid, bool cursor,
							int row, int col, int width)
{
	byte attr = curs_attrs[CURS_KNOWN][(int)cursor];
	c_prt(attr, format("%-48s: %s  (%s)", 
						option_desc[oid],
						op_ptr->opt[oid] ? "yes" : "no ",
						option_text[oid]),
			row, col);
}

static bool update_option(char key, const void *pgdb, int oid)
{
	switch(toupper(key))
	{
	case 'Y': case '6':
		op_ptr->opt[oid] = TRUE;
		break;
	case 'N': case '4':
		op_ptr->opt[oid] = FALSE;
		break;
	case 'T': case '5':
		op_ptr->opt[oid] = !op_ptr->opt[oid];
		break;
	case 0xff:
		op_ptr->opt[oid] = !op_ptr->opt[oid];
		break;
	case '?':
		show_file(format("option.txt#%s", option_text[oid]), NULL, 0, 0);
		break;
	}
	return TRUE;
}


/*
 * Interact with some options
 */
static void do_cmd_options_aux(void *vpage, cptr info)
{
	int page = (int)vpage;
	int opt[OPT_PAGE_PER];
	int i, n = 0;
	int cursor_pos = 0;

	/* TODO: make an initializer that takes a few common args */
	/* Title, prompt, cmd, choices, and flags -- everything else is ignored */
	/* for MN_ONCE  */
	menu_type menu = {
		0, 					/* menu_id */
		info,				/* title */
		"Set option (y/n/t) '?' for information", /* prompt */
		"?5YyNnTt",			/* cmd_keys */
		default_choice,		/* selections */
		MN_REL_TAGS|MN_SCROLL|MN_NO_ACT|MN_ONCE,	/* flags */
		0,					/* count */

		vpage,				/* menu data */
		update_option,		/* updater */
		NULL,				/* browse */
		display_option,		/* label */
		0,					/* tagger */
		0,					/* skin */
	};
	
	screen_save();
	Term_clear();

	/* Filter the options for this page */
	for (i = 0; i < OPT_PAGE_PER; i++)
	{
		if (option_page[page][i] != 255)
		{
			opt[n++] = option_page[page][i];
		}
	}
	menu.count = n;
	for(;;)
	{
		key_event cx;
		cx = menu_select(&menu, opt, n, &cursor_pos, SCREEN_REGION);
		if (ESCAPE == cx.key) break;
		if(cx.type == EVT_MOVE) cursor_pos = cx.index;
		else if(cx.key != '\xff') {
			update_option(cx.key, vpage, opt[cursor_pos]);
			if(strchr("YNyn", cx.key)) cursor_pos++;
		}
		cursor_pos = (cursor_pos+n)%n;
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

	key_event ke;

	u32b old_flag[ANGBAND_TERM_MAX];


	/* Memorize old flags */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		old_flag[j] = op_ptr->window_flag[j];
	}


	/* Clear screen */
	Term_clear();

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
		if ((ke.key == '5') || (ke.key == 't') || ((ke.key == '\xff') && (ke.index)))
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
static void remove_old_dump(cptr orig_file, cptr mark)
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

		strnfmt(expected_line, sizeof(expected_line),
						"%s begin %s", dump_separator, mark);

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
		{
			fprintf(orig_fff, "%s\n", buf);
		}

		my_fclose(orig_fff);
		my_fclose(tmp_fff);
	}

	/* Kill the temporary file */
	fd_kill(tmp_file);
}


/*
 * Output the header of a pref-file dump
 */
static void pref_header(FILE *fff, cptr mark)
{
	/* Start of dump */
	fprintf(fff, "%s begin %s\n", dump_separator, mark);

	fprintf(fff, "# *Warning!*  The lines below are an automatic dump.\n");
	fprintf(fff, "# Don't edit them; changes will be deleted and replaced automatically.\n");
}

/*
 * Output the footer of a pref-file dump
 */
static void pref_footer(FILE *fff, cptr mark)
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
 * - dump(FILE*) needs to emit only the raw data for the dump.
 *   Comments are generated automatically
 * - row is where on the screen to place the prompt
 */
static void dump_pref_file(void (*dump)(FILE*), const char *title)
{
	char ftmp[80];
	char buf[1025];
	FILE *fff;

	int row = 15;

	/* Prompt */
	prt(format("%s to a pref file", title), row, 0);

	/* Prompt */
	prt("File: ", row + 2, 0);

	/* Default filename */
	sprintf(ftmp, "%s.prf", op_ptr->base_name);

	/* Get a filename */
	if (!askfor_aux(ftmp, sizeof ftmp)) return;

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, ftmp);

	FILE_TYPE(FILE_TYPE_TEXT);

	/* Remove old macros */
	remove_old_dump(buf, title);

	/* Append to the file */
	fff = my_fopen(buf, "a");

	/* Failure */
	if (!fff) {
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
		for (j = 0; j < 32; j++)
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
}

/* Hack -- Base Delay Factor */
void do_cmd_delay(void)
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

		cx = inkey();
		if (cx == ESCAPE) break;
		if (isdigit(cx)) op_ptr->delay_factor = D2I(cx);
		else bell("Illegal delay factor!");
	}
}

/* Hack -- hitpoint warning factor */
void do_cmd_hp_warn(void)
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

		cx = inkey();
		if (cx == ESCAPE) break;
		if (isdigit(cx)) op_ptr->hitpoint_warn = D2I(cx);
		else bell("Illegal hitpoint warning!");
	}
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

	/* Read the pattern */
	while (ch != '\0')
	{
		/* Save the key */
		buf[n++] = ch;

		/* Do not process macros */
		inkey_base = TRUE;

		/* Do not wait for keys */
		inkey_scan = TRUE;

		/* Attempt to read a key */
		ch = inkey();
	}

	/* Terminate */
	buf[n] = '\0';

	/* Flush */
	flush();


	/* Convert the trigger */
	ascii_to_text(tmp, sizeof(tmp), buf);

	/* Hack -- display the trigger */
	Term_addstr(-1, TERM_WHITE, tmp);
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
	{'lupf', "Load a user pref file", 0},
#ifdef ALLOW_MACROS
	{'amac', "Append macros to a file", 0},
	{'qmac', "Query a macro", 0},
	{'cmac', "Create a macro", 0},
	{'jmac', "Remove a macro", 0},
	{'akm', "Append keymaps to a file", 0},
	{'qkm', "Query a keymap", 0},
	{'ckm', "Create a keymap", 0},
	{'rkm', "Remove a keymap", 0},
	{'emac', "Enter a new action", 0}
#endif /* ALLOW_MACROS */
};

static menu_type macro_menu = {
	'mcro',
	0,
	0,
	0,
	default_choice,

	MN_EVT,
	N_ELEMENTS(macro_actions),
	macro_actions
	/* ,0,0,0,0*/
};


void do_cmd_macros(void)
{

	char tmp[1024];

	char pat[1024];

	int mode;
	int cursor = 0;

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

	/* Process requests until done */
	while (1)
	{
		region loc = {0, 1, 0, 11};
		key_event c;
		int evt;
		/* Clear screen */
		Term_clear();

		/* No title -- this is a complex menu. */
		prt("Interact with macros", 0, 0);
		/* Describe current action */
		prt("Current action (if any) shown below:", 12, 0);
		/* Analyze the current action */
		ascii_to_text(tmp, sizeof(tmp), macro_buffer);
		/* Display the current action */
		prt(tmp, 13, 0);
		c = menu_select(&macro_menu, 0, macro_menu.count, &cursor, loc);

		if(ESCAPE == c.key) 
			break;
		evt = macro_actions[cursor].id;

		switch(evt) {
		case 'lupf':
			do_cmd_pref_file_hack(16);
			break;

#ifdef ALLOW_MACROS
		case 'amac':
		{
			/* Dump the macros */
			(void)dump_pref_file(macro_dump, "Dump Macros");

			break;
		}

		case 'qmac':
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

		case 'cmac':
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
			if (askfor_aux(tmp, sizeof tmp))
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
		case 'dmac':
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
		case 'akm':
		{
			/* Dump the keymaps */
			(void)dump_pref_file(keymap_dump, "Dump Keymaps");
			break;
		}
		case 'qkm':
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
		case 'ckm':
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
			if (askfor_aux(tmp, sizeof tmp))
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
		case 'dkm':
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
		case 'emac': /* Enter a new action */
		{
			/* Prompt */
			prt("Command: Enter a new action", 16, 0);

			/* Go to the correct location */
			Term_gotoxy(0, 22);

			/* Analyze the current action */
			ascii_to_text(tmp, sizeof(tmp), macro_buffer);

			/* Get an encoded action */
			if (askfor_aux(tmp, sizeof tmp))
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
	for (i = 0; i < 256; i++)
	{
		int kv = angband_color_table[i][0];
		int rv = angband_color_table[i][1];
		int gv = angband_color_table[i][2];
		int bv = angband_color_table[i][3];

		cptr name = "unknown";

		/* Skip non-entries */
		if (!kv && !rv && !gv && !bv) continue;

		/* Extract the color name */
		if (i < 16) name = color_names[i];

		/* Dump a comment */
		fprintf(fff, "# Color '%s'\n", name);

	}
}


int modify_attribute(const char *clazz, int oid, const char *name,
							byte da, char dc, byte *pca, char *pcc)
{
	int cx;
	const char *empty_symbol = "<< ? >>";
	const char *empty_symbol2 = "\0";
	const char *empty_symbol3 = "\0";

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
	else if (use_bigtile) empty_symbol = "<< ?? >>";

	byte ca = (byte)*pca;
	byte cc = (byte)*pcc;

	int linec = (use_trptile ? 22: (use_dbltile ? 21 : 20));

	/* Prompt */
	prt(format("Command: Change %s attr/chars", clazz), 15, 0);

	/* Label the object */
	Term_putstr(5, 17, -1, TERM_WHITE, format("%s = %d, Name = %-40.40s", clazz, oid, name));
							
	/* Label the Default values */
	Term_putstr(10, 19, -1, TERM_WHITE,
			    format("Default attr/char = %3u / %3u", da, dc));
	Term_putstr(40, 19, -1, TERM_WHITE, empty_symbol);

	if (use_dbltile || use_trptile) Term_putstr (40, 20, -1, TERM_WHITE, empty_symbol2);
	if (use_trptile) Term_putstr (40, 20, -1, TERM_WHITE, empty_symbol3);


	big_pad(43, 19, da, dc);

	/* Label the Current values */
	Term_putstr(10, linec, -1, TERM_WHITE,
			    format("Current attr/char = %3u / %3u", ca, cc));
	Term_putstr(40, linec, -1, TERM_WHITE, empty_symbol);
	if (use_dbltile || use_trptile) Term_putstr (40, linec+1, -1, TERM_WHITE, empty_symbol2); 
	if (use_trptile) Term_putstr (40, linec+2, -1, TERM_WHITE, empty_symbol3); 

	big_pad(43, linec, ca, cc);

	if (use_trptile) linec++;

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
	{'lupf', "Load a user pref file", 0, 0},
	{'vdmx',  "Dump monster attr/chars", 0, 0},
	{'vdox',  "Dump object attr/chars", 0, 0},
	{'vdfx',  "Dump feature attr/chars", 0, 0},
	{'vdxx',  "Dump flavor attr/chars", 0, 0},
	{'vemx',  "Change monster attr/chars", 0, 0},
	{'veox',  "Change object attr/chars", 0, 0},
	{'vefx',  "Change feature attr/chars", 0, 0},
	{'vexx',  "Change flavor attr/chars", 0, 0},
	{'vrst', "Reset visuals", 0, 0},
};

static menu_type visual_menu = {
	'visu',
	"Interact with visuals",
	"",
	"Command: ",
	default_choice,
	MN_EVT,
	N_ELEMENTS(visual_menu_items),
	visual_menu_items
	/* ,0,0,0,0 */
};


/*
 * Interact with "visuals"
 */
void do_cmd_visuals(void)
{
	int cursor = 0;

	/* Save screen */
	screen_save();


	/* Interact until done */
	while (1)
	{
		key_event key;
		int evt = -1;
		Term_clear();
		key = menu_select(&visual_menu, 0, visual_menu.count, &cursor, SCREEN_REGION);
		if(key.key == ESCAPE) 
			break;

		assert(cursor >= 0 && cursor < visual_menu.count);

		evt = visual_menu_items[cursor].id;

		if (evt == 'lupf')
		{
			/* Ask for and load a user pref file */
			do_cmd_pref_file_hack(15);
		}

#ifdef ALLOW_VISUALS

		else if (evt == 'vdmx')
		{
			dump_pref_file(dump_monsters, "Dump Monster attr/chars");
		}

		else if (evt == 'vdox')
		{
			dump_pref_file(dump_objects, "Dump Object attr/chars");
		}

		else if (evt == 'vdfx')
		{
			dump_pref_file(dump_features, "Dump Feature attr/chars");
		}

		/* Dump flavor attr/chars */
		else if (evt == 'vdxx')
		{
			dump_pref_file(dump_flavors, "Dump Flavor attr/chars");
		}

		/* Modify monster attr/chars */
		else if (evt == 'vemx')
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
		else if (evt == 'veox')
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
		else if (evt == 'vefx')
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
		else if (evt == 'vexx')
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
		else if (evt == 'vrst')
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
	{'lupf', "Load a user pref file", 0, 0},
#ifdef ALLOW_COLORS
	{'vdco', "Dump colors", 0, 0},
	{'veco', "Modify colors", 0, 0}
#endif
};

static menu_type color_menu = {
	'colr',
	"Interact with colors",
	"Command: ",
	0,
	default_choice,
	MN_EVT,
	N_ELEMENTS(color_events),
	color_events
	/* 0,0,0,0 */
};



/*
 * Interact with "colors"
 */
void do_cmd_colors(void)
{
	int i;
	int cx;
	int cursor = -1;

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Save screen */
	screen_save();

	/* Interact until done */
	while (1)
	{
		key_event key;
		int evt;
		Term_clear();
		key = menu_select(&color_menu, 0, color_menu.count, &cursor, SCREEN_REGION);

		/* Done */
		if (key.key == ESCAPE) break;
		evt = color_events[cursor].id;

		/* Load a user pref file */
		if (evt == 'lupf')
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
		else if (evt == 'vdco')
		{
			dump_pref_file(dump_colors, "Dump Colors");
		}

		/* Edit colors */
		else if (evt == 'veco')
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
				for (i = 0; i < 16; i++)
				{
					/* Exhibit this color */
					Term_putstr(i*4, 20, -1, a, "###");

					/* Exhibit all colors */
					Term_putstr(i*4, 22, -1, (byte)i, format("%3d", i));
				}

				/* Describe the color */
				name = ((a < 16) ? color_names[a] : "undefined");

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
		Term_clear();
	}


	/* Load screen */
	screen_load();
}




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
 * Array of feeling strings
 */
static cptr do_cmd_feeling_text[11] =
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
	if (feeling > 10) feeling = 10;

	/* No useful feeling in town */
	if (!p_ptr->depth)
	{
		msg_print("Looks like a typical town.");
		return;
	}

	/* Display the feeling */
	msg_print(do_cmd_feeling_text[feeling]);
}

/*
 * Encode the screen colors
 */
static const char hack[17] = "dwsorgbuDWvyRGBU";


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
			for (i = 0; i < 16; i++)
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
 * Hack -- save a screen dump to a file
 */
void do_cmd_save_screen(void)
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
			buf[x] = hack[a&0x0F];
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
void do_cmd_save_screen_html(void)
{
	int i;

	FILE *fff;
	char file_name[1024];
	char tmp_val[256];

	typedef void (*dump_func)(FILE*);
	dump_func dump_visuals [] = 
		{dump_monsters, dump_features, dump_objects, dump_flavors, dump_colors};

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);


	/* Ask for a file */
	my_strcpy(tmp_val, "dump.html", sizeof(tmp_val));
	if (!get_string("File: ", tmp_val, sizeof(tmp_val))) return;

	/* Save current preferences */
	path_build(file_name, 1024, ANGBAND_DIR_USER, "dump.prf");
	fff = my_fopen(file_name, "w");

	if (!fff) {
		msg_print("Screen dump failed.");
		message_flush();
		return;
	}
	for(i = 0; i < N_ELEMENTS(dump_visuals); i++)
		dump_visuals[i](fff);
	my_fclose(fff);


	/* Dump the screen with raw character attributes */
	reset_visuals(FALSE);
	do_cmd_redraw();
	html_screenshot(tmp_val);

	/* Recover current graphics settings */
	reset_visuals(TRUE);
	process_pref_file(file_name);
	fd_kill(file_name);
	do_cmd_redraw();

	msg_print("Html screen dump saved.");
	message_flush();
}


/*
 * Move the cursor using the mouse in a browser window
 */
static void browser_mouse(key_event ke, int *column, int *grp_cur,
							int grp_cnt, int *list_cur, int list_cnt,
							int col0, int row0, int grp0, int list0,
							int *delay)
{
	int my = ke.mousey - row0;
	int mx = ke.mousex;
	int wid;
	int hgt;

	int grp = *grp_cur;
	int list = *list_cur;

	/* Get size */
	Term_get_size(&wid, &hgt);

	if (mx < col0)
	{
		int old_grp = grp;

		*column = 0;
		if ((my >= 0) && (my < grp_cnt - grp0) && (my < hgt - row0 - 2)) grp = my + grp0;
		else if (my < 0) { grp--; *delay = 100; }
		else if (my >= hgt - row0 - 2) { grp++; *delay = 50; }

		/* Verify */
		if (grp >= grp_cnt)	grp = grp_cnt - 1;
		if (grp < 0) grp = 0;
		if (grp != old_grp)	list = 0;

	}
	else
	{
		*column = 1;
		if ((my >= 0) && (my < list_cnt - list0) && (my < hgt - row0 - 2)) list = my + list0;
		else if (my < 0) { list--; *delay = 100; }
		else if (my >= hgt - row0 - 2) { list++; *delay = 50; }

		/* Verify */
		if (list >= list_cnt) list = list_cnt - 1;
		if (list < 0) list = 0;
	}

	(*grp_cur) = grp;
	(*list_cur) = list;
} 

/* 
 * Move the cursor in a browser window 
 */
static void browser_cursor(char ch, int *column, int *grp_cur, int grp_cnt, 
						   int *list_cur, int list_cnt)
{
	int d;
	int col = *column;
	int grp = *grp_cur;
	int list = *list_cur;

	/* Extract direction */
	d = target_dir(ch);

	if (!d) return;

	/* Diagonals - hack */
	if ((ddx[d] > 0) && ddy[d])
	{
		int browser_rows;
		int wid, hgt;

		/* Get size */
		Term_get_size(&wid, &hgt);

		browser_rows = hgt - 8;

		/* Browse group list */
		if (!col)
		{
			int old_grp = grp;

			/* Move up or down */
			grp += ddy[d] * browser_rows;

			/* Verify */
			if (grp >= grp_cnt)	grp = grp_cnt - 1;
			if (grp < 0) grp = 0;
			if (grp != old_grp)	list = 0;
		}

		/* Browse sub-list list */
		else
		{
			/* Move up or down */
			list += ddy[d] * browser_rows;

			/* Verify */
			if (list >= list_cnt) list = list_cnt - 1;
			if (list < 0) list = 0;
		}

		(*grp_cur) = grp;
		(*list_cur) = list;

		return;
	}

	if (ddx[d])
	{
		col += ddx[d];
		if (col < 0) col = 0;
		if (col > 1) col = 1;

		(*column) = col;

		return;
	}

	/* Browse group list */
	if (!col)
	{
		int old_grp = grp;

		/* Move up or down */
		grp += ddy[d];

		/* Verify */
		if (grp < 0) grp = 0;
		if (grp >= grp_cnt)	grp = grp_cnt - 1;
		if (grp != old_grp)	list = 0;
	}

	/* Browse sub-list list */
	else
	{
		/* Move up or down */
		list += ddy[d];

		/* Verify */
		if (list >= list_cnt) list = list_cnt - 1;
		if (list < 0) list = 0;
	}

	(*grp_cur) = grp;
	(*list_cur) = list;
}


/* ========================= MENU DEFINITIONS ========================== */


static menu_item option_actions [] =
{
	{{0, "User Interface Options", do_cmd_options_aux, (void*)0}, '1'},
	{{0, "Disturbance Options", do_cmd_options_aux, (void*)1}, '2'},
	{{0, "Game-Play Options", do_cmd_options_aux, (void*)2}, '3'},
	{{0, "Efficiency Options", do_cmd_options_aux, (void*)3}, '4'},
	{{0, "Display Options", do_cmd_options_aux, (void*)4}, '5'},
	{{0, "Birth Options", do_cmd_options_aux, (void*)5}, '6'},
	{{0, "Cheat Options", do_cmd_options_aux, (void*)6}, '7'},
	{{0, 0, 0, 0}}, /* Load and append */
	{{0, "Window Flags", (action_f) do_cmd_options_win, 0}, 'W'},
	{{0, "Item Squelch and Autoinscribe Menus", (action_f) do_cmd_squelch_autoinsc, 0}, 'S'},
	{{0, "Load a user pref file", (action_f) do_cmd_pref_file_hack, (void*)20}, 'L'},
	{{0, "Dump Options", (action_f) dump_pref_file, option_dump}, 'A'},
	{{0, 0, 0,}, 0}, /* Special choices */
	{{0, "Base Delay Factor", (action_f) do_cmd_delay, 0}, 'D'},
	{{0, "Hitpoint Warning", (action_f) do_cmd_hp_warn, 0}, 'H'}
};

static menu_type option_menu = {
	'opti',
	"Display Options",
	"Prompt: ",
	0,
	0,
	MN_ACT,
	N_ELEMENTS(option_actions),
	option_actions
	/* ,0,0,0,0 */
}; 

static menu_item knowledge_actions[] =
{
	{{0, "Display artifact knowledge", (action_f)do_cmd_knowledge_artifacts, 0}, '1'},
	{{0, "Display monster knowledge", (action_f)do_cmd_knowledge_monsters, 0}, '2'},
	{{0, "Display ego item knowledge", (action_f)do_cmd_knowledge_ego_items, 0}, '3'},
	{{0, "Display object knowledge", (action_f)do_cmd_knowledge_objects, 0}, '4'},
	{{0, "Display feature knowledge", (action_f)do_cmd_knowledge_features, 0}, '5'},
	{{0, "Display self-knowledge", (action_f)self_knowledge, 0}, '6'},
	{{0, 0, 0}, 0, 0}, /* other stuff */
	{{0, "Load a user pref file", (action_f) do_cmd_pref_file_hack, (void*) 20}, 'L'},
	{{0, "Interact with visuals", (action_f) do_cmd_visuals, 0}, 'V'},
};

static menu_type knowledge_menu = {
	'know',
	"Display current options",
	"Prompt: ",
	0,
	0,
	MN_ACT,
	N_ELEMENTS(knowledge_actions),
	knowledge_actions
}; 

/* Keep macro counts happy. */
static void cleanup_cmds () {
	FREE(obj_group_order);
}

void do_cmd_options()
{
	int cursor = -1;

	screen_save();

	for(;;) {
		key_event c;
		Term_clear();
		c = menu_select(&option_menu, 0, option_menu.count, &cursor, SCREEN_REGION);
		if(ESCAPE == c.key) break;
	}

	screen_load();
}

void do_cmd_knowledge()
{
	int cursor = -1;

	/* initialize static variables */
	if(!obj_group_order) {
		int i;
		int gid = -1;
		C_MAKE(obj_group_order, TV_GOLD+1, int);
		atexit(cleanup_cmds);
		for(i = 0; i <= TV_GOLD; i++) /* allow for missing values */
			obj_group_order[i] = -1;
		for(i = 0; 0 != object_text_order[i].tval; i++) {
			if(object_text_order[i].name) gid = i;
			obj_group_order[object_text_order[i].tval] = gid;
		}
	}

	screen_save();

	for(;;) {
		key_event c;
		Term_clear();
		c = menu_select(&knowledge_menu, 0, knowledge_menu.count, &cursor, SCREEN_REGION);
		if(ESCAPE == c.key) break;
	}

	screen_load();
}
