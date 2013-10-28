/*
 * File: options.c
 * Purpose: Options table and definitions.
 *
 * Copyright (c) 1997 Ben Harrison
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
#include "option.h"

/*
 * Option screen interface
 */
const int option_page[OPT_PAGE_MAX][OPT_PAGE_PER] =
{
	/* Interface */
	{
		OPT_rogue_like_commands,
		OPT_use_old_target,
		OPT_pickup_always,
		OPT_pickup_inven,
		OPT_easy_open,
		OPT_center_player,
		OPT_view_yellow_light,
		OPT_hp_changes_color,
		OPT_animate_flicker,
		OPT_purple_uniques,
		OPT_show_flavors,
		OPT_mouse_movement,
		OPT_mouse_buttons,
		OPT_use_sound,
		OPT_xchars_to_file,
		OPT_NONE,
	},

	/* Warning */
	{
		OPT_disturb_move,
		OPT_disturb_near,
		OPT_disturb_detect,
		OPT_disturb_state,
		OPT_auto_more,
		OPT_notify_recharge,
		OPT_NONE,
		OPT_NONE,
		OPT_NONE,
		OPT_NONE,
		OPT_NONE,
		OPT_NONE,
		OPT_NONE,
		OPT_NONE,
		OPT_NONE,
		OPT_NONE,
	},

	/* Birth/Difficulty */
	{
		OPT_birth_maximize,
		OPT_birth_randarts,
		OPT_birth_keep_randarts,
		OPT_birth_ai_smell,
		OPT_birth_ai_packs,
		OPT_birth_ai_learn,
		OPT_birth_ai_cheat,
		OPT_birth_ai_smart,
		OPT_birth_ironman,
		OPT_birth_no_stores,
		OPT_birth_no_artifacts,
		OPT_birth_no_stacking,
		OPT_birth_no_preserve,
		OPT_birth_no_stairs,
		OPT_birth_no_feelings,
		OPT_birth_no_selling,
	},

	/* Cheat */
	{
		OPT_cheat_hear,
		OPT_cheat_room,
		OPT_cheat_xtra,
		OPT_cheat_know,
		OPT_cheat_live,
		OPT_NONE,
		OPT_NONE,
		OPT_NONE,
		OPT_NONE,
		OPT_NONE,
		OPT_NONE,
		OPT_NONE,
		OPT_NONE,
		OPT_NONE,
		OPT_NONE,
		OPT_NONE,
	}
};


struct option
{
	const char *name;
	const char *description;
	bool normal;
};

static const struct option options[OPT_MAX] =
{
{ "rogue_like_commands", "Use the roguelike command keyset",            FALSE }, /* 0 */
{ "use_sound",           "Use sound",                                   FALSE }, /* 1 */
{ NULL,                  NULL,                                          FALSE }, /* 2 */
{ "use_old_target",      "Use old target by default",                   FALSE }, /* 3 */
{ "pickup_always",       "Always pickup items",                         FALSE }, /* 4 */
{ "pickup_inven",        "Always pickup items matching inventory",      TRUE },  /* 5 */
{ "show_flavors",        "Show flavors in object descriptions",         FALSE },  /* 6 */
{ "disturb_move",        "Disturb whenever any monster moves",          FALSE }, /* 7 */
{ "disturb_near",        "Disturb whenever viewable monster moves",     TRUE },  /* 8 */
{ "disturb_detect",      "Disturb whenever leaving trap detected area", TRUE },  /* 9 */
{ "disturb_state",       "Disturb whenever player state changes",       TRUE },  /* 10 */
{ NULL,                  NULL,                                          FALSE }, /* 11 */
{ NULL,                  NULL,                                          FALSE }, /* 12 */
{ "view_yellow_light",   "Color: Illuminate torchlight in yellow",      FALSE }, /* 13 */
{ "easy_open",           "Open/disarm/close without direction",         TRUE }, /* 14 */
{ "animate_flicker",     "Color: Shimmer multi-colored things",         FALSE }, /* 15 */
{ "center_player",       "Center map continuously",                     FALSE }, /* 16 */
{ "purple_uniques",      "Color: Show unique monsters in purple",       FALSE }, /* 17 */
{ "xchars_to_file",      "Allow accents in output files",               FALSE }, /* 18 */
{ "auto_more",           "Automatically clear '-more-' prompts",        FALSE }, /* 19 */
{ "hp_changes_color",    "Color: Player color indicates % hit points",  FALSE }, /* 20 */
{ "mouse_movement",      "Allow mouse clicks to move the player",       FALSE }, /* 21 */
{ "mouse_buttons",       "Show mouse status line buttons",              FALSE }, /* 22 */
{ "notify_recharge",     "Notify on object recharge",                   FALSE }, /* 23 */
{ NULL,                  NULL,                                          FALSE }, /* 24 */
{ NULL,                  NULL,                                          FALSE }, /* 25 */
{ NULL,                  NULL,                                          FALSE }, /* 26 */
{ NULL,                  NULL,                                          FALSE }, /* 27 */
{ NULL,                  NULL,                                          FALSE }, /* 28 */
{ NULL,                  NULL,                                          FALSE }, /* 29 */
{ NULL,                  NULL,                                          FALSE }, /* 30 */
{ "cheat_hear",          "Cheat: Peek into monster creation",           FALSE }, /* 31 */
{ "cheat_room",          "Cheat: Peek into dungeon creation",           FALSE }, /* 32 */
{ "cheat_xtra",          "Cheat: Peek into something else",             FALSE }, /* 33 */
{ "cheat_know",          "Cheat: Know complete monster info",           FALSE }, /* 34 */
{ "cheat_live",          "Cheat: Allow player to avoid death",          FALSE }, /* 35 */
{ NULL,                  NULL,                                          FALSE }, /* 36 */
{ NULL,                  NULL,                                          FALSE }, /* 37 */
{ NULL,                  NULL,                                          FALSE }, /* 38 */
{ NULL,                  NULL,                                          FALSE }, /* 39 */
{ NULL,                  NULL,                                          FALSE }, /* 40 */
{ "score_hear",          "Score: Peek into monster creation",           FALSE }, /* 41 */
{ "score_room",          "Score: Peek into dungeon creation",           FALSE }, /* 42 */
{ "score_xtra",          "Score: Peek into something else",             FALSE }, /* 43 */
{ "score_know",          "Score: Know complete monster info",           FALSE }, /* 44 */
{ "score_live",          "Score: Allow player to avoid death",          FALSE }, /* 45 */
{ NULL,                  NULL,                                          FALSE }, /* 46 */
{ NULL,                  NULL,                                          FALSE }, /* 47 */
{ NULL,                  NULL,                                          FALSE }, /* 48 */
{ NULL,                  NULL,                                          FALSE }, /* 49 */
{ "birth_maximize",      "Maximise effect of race/class bonuses",       TRUE },  /* 50 */
{ "birth_randarts",      "Randomise the artifacts (except a very few)", FALSE }, /* 51 */
{ "birth_ironman",       "Restrict the use of stairs/recall",           FALSE }, /* 52 */
{ "birth_no_stores",     "Restrict the use of stores/home",             FALSE }, /* 53 */
{ "birth_no_artifacts",  "Restrict creation of artifacts",              FALSE }, /* 54 */
{ "birth_no_stacking",   "Don't stack objects on the floor",            FALSE }, /* 55 */
{ "birth_no_preserve",   "Lose artifacts when leaving level",           FALSE }, /* 56 */
{ "birth_no_stairs",     "Don't generate connected stairs",             FALSE }, /* 57 */
{ "birth_no_feelings",   "Don't show level feelings",                   FALSE }, /* 58 */
{ "birth_no_selling",    "Items always sell for 0 gold",                FALSE }, /* 59 */
{ "birth_keep_randarts", "Use previous set of randarts",                TRUE },  /* 60 */
{ "birth_ai_smell",      "Monsters chase recent locations",             TRUE },  /* 61 */
{ "birth_ai_packs",      "Monsters act smarter in groups",              TRUE },  /* 62 */
{ "birth_ai_learn",      "Monsters learn from their mistakes",          FALSE }, /* 63 */
{ "birth_ai_cheat",      "Monsters exploit player's weaknesses",        FALSE }, /* 64 */
{ "birth_ai_smart",      "Monsters behave more intelligently (broken)", FALSE }, /* 65 */
{ NULL,                  NULL,                                          FALSE }, /* 66 */
{ NULL,                  NULL,                                          FALSE }, /* 67 */
{ NULL,                  NULL,                                          FALSE }, /* 68 */
{ NULL,                  NULL,                                          FALSE }, /* 69 */
};


/* Accessor functions */
const char *option_name(int opt)
{
	if (opt >= OPT_MAX) return NULL;
	return options[opt].name;
}

const char *option_desc(int opt)
{
	if (opt >= OPT_MAX) return NULL;
	return options[opt].description;
}

#if 0 /* unused so far but may be useful in future */
static bool option_is_birth(int opt) { return opt >= OPT_BIRTH && opt < (OPT_BIRTH + N_OPTS_BIRTH); }
static bool option_is_score(int opt) { return opt >= OPT_SCORE && opt < (OPT_SCORE + N_OPTS_CHEAT); }
#endif

static bool option_is_cheat(int opt) { return opt >= OPT_CHEAT && opt < (OPT_CHEAT + N_OPTS_CHEAT); }

/* Setup functions */
bool option_set(const char *name, bool on)
{
	size_t opt;
	for (opt = 0; opt < OPT_MAX; opt++)
	{
		if (!options[opt].name || !streq(options[opt].name, name))
			continue;

		op_ptr->opt[opt] = on;
		if (on && option_is_cheat(opt)) {
			op_ptr->opt[opt + (OPT_SCORE - OPT_CHEAT)] = TRUE;
		}

		return TRUE;
	}

	return FALSE;
}

void option_set_defaults(void)
{
	size_t opt;
	for (opt = 0; opt < OPT_MAX; opt++)
		op_ptr->opt[opt] = options[opt].normal;
}
