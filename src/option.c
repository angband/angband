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
		OPT_use_sound,
		OPT_rogue_like_commands,
		OPT_use_old_target,
		OPT_pickup_always,
		OPT_pickup_inven,
		OPT_pickup_detail,
		OPT_hide_squelchable,
		OPT_squelch_worthless,
		OPT_easy_alter,
		OPT_easy_open,
		OPT_show_lists,
		OPT_mouse_movement,
		OPT_mouse_buttons,
		OPT_xchars_to_file,
		OPT_NONE,
		OPT_NONE,
	},

	/* Display */
	{
		OPT_hp_changes_color,
		OPT_highlight_player,
		OPT_center_player,
		OPT_show_piles,
		OPT_show_flavors,
		OPT_show_labels,
		OPT_view_yellow_light,
		OPT_view_bright_light,
		OPT_view_granite_light,
		OPT_view_special_light,
		OPT_view_perma_grids,
		OPT_view_torch_grids,
		OPT_animate_flicker,
		OPT_purple_uniques,
		OPT_NONE,
		OPT_NONE,
	},

	/* Warning */
	{
		OPT_disturb_move,
		OPT_disturb_near,
		OPT_disturb_detect,
		OPT_disturb_state,
		OPT_quick_messages,
		OPT_auto_more,
		OPT_ring_bell,
		OPT_flush_failure,
		OPT_flush_disturb,
		OPT_notify_recharge,
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
		OPT_birth_ai_sound,
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
		OPT_birth_feelings,
		OPT_NONE
	},

	/* Cheat */
	{
		OPT_cheat_peek,
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
	}
};


typedef struct
{
	const char *name;
	const char *description;
	bool normal;
} option_entry;

static option_entry options[OPT_MAX] =
{
{ "rogue_like_commands", "Rogue-like commands",                         FALSE }, /* 0 */
{ "quick_messages",      "Activate quick messages",                     TRUE },  /* 1 */
{ "use_sound",           "Use sound",                                   FALSE }, /* 2 */
{ "pickup_detail",       "Be verbose when picking things up",           TRUE },  /* 3 */
{ "use_old_target",      "Use old target by default",                   FALSE }, /* 4 */
{ "pickup_always",       "Always pickup items",                         FALSE }, /* 5 */
{ "pickup_inven",        "Always pickup items matching inventory",      TRUE },  /* 6 */
{ NULL,                  NULL,                                          FALSE }, /* 7 */
{ NULL,                  NULL,                                          FALSE }, /* 8 */
{ NULL,                  NULL,                                          FALSE }, /* 9 */
{ "show_labels",         "Show labels in equipment listings",           TRUE },  /* 10 */
{ "show_lists",          "Always show lists",                           TRUE },  /* 11 */
{ NULL,                  NULL,                                          FALSE }, /* 12 */
{ NULL,                  NULL,                                          FALSE }, /* 13 */
{ "ring_bell",           "Audible bell (on errors, etc)",               TRUE },  /* 14 */
{ "show_flavors",        "Show flavors in object descriptions",         FALSE },  /* 15 */
{ NULL,                  NULL,                                          FALSE }, /* 16 */
{ NULL,                  NULL,                                          FALSE }, /* 17 */
{ NULL,                  NULL,                                          FALSE }, /* 18 */
{ NULL,                  NULL,                                          FALSE }, /* 19 */
{ "disturb_move",        "Disturb whenever any monster moves",          FALSE }, /* 20 */
{ "disturb_near",        "Disturb whenever viewable monster moves",     TRUE },  /* 21 */
{ "disturb_detect",      "Disturb whenever leaving trap detected area", TRUE },  /* 22 */
{ "disturb_state",       "Disturb whenever player state changes",       TRUE },  /* 23 */
{ NULL,                  NULL,                                          FALSE }, /* 24 */
{ NULL,                  NULL,                                          FALSE }, /* 25 */
{ NULL,                  NULL,                                          FALSE }, /* 26 */
{ NULL,                  NULL,                                          FALSE }, /* 27 */
{ NULL,                  NULL,                                          FALSE }, /* 28 */
{ NULL,                  NULL,                                          FALSE }, /* 29 */
{ NULL,                  NULL,                                          FALSE }, /* 30 */
{ NULL,                  NULL,                                          FALSE }, /* 31 */
{ NULL,                  NULL,                                          FALSE }, /* 32 */
{ NULL,                  NULL,                                          FALSE }, /* 33 */
{ NULL,                  NULL,                                          FALSE }, /* 34 */
{ NULL,                  NULL,                                          FALSE }, /* 35 */
{ NULL,                  NULL,                                          FALSE }, /* 36 */
{ NULL,                  NULL,                                          FALSE }, /* 37 */
{ "view_perma_grids",    "Map remembers all perma-lit grids",           TRUE },  /* 38 */
{ "view_torch_grids",    "Map remembers all torch-lit grids",           TRUE },  /* 39 */
{ NULL,                  NULL,                                          TRUE }, /* 40 */
{ NULL,                  NULL,                                          TRUE }, /* 41 */
{ NULL,                  NULL,                                          FALSE }, /* 42 */
{ NULL,                  NULL,                                          FALSE }, /* 43 */
{ NULL,                  NULL,                                          FALSE }, /* 44 */
{ NULL,                  NULL,                                          FALSE }, /* 45 */
{ NULL,                  NULL,                                          FALSE }, /* 46 */
{ NULL,                  NULL,                                          FALSE }, /* 47 */
{ NULL,                  NULL,                                          FALSE }, /* 48 */
{ NULL,                  NULL,                                          FALSE }, /* 49 */
{ NULL,                  NULL,                                          FALSE }, /* 50 */
{ NULL,                  NULL,                                          FALSE }, /* 51 */
{ "flush_failure",       "Flush input on various failures",             TRUE },  /* 52 */
{ "flush_disturb",       "Flush input whenever disturbed",              FALSE }, /* 53 */
{ NULL,                  NULL,                                          FALSE }, /* 54 */
{ NULL,                  NULL,                                          FALSE }, /* 55 */
{ NULL,                  NULL,                                          FALSE }, /* 56 */
{ NULL,                  NULL,                                          FALSE }, /* 57 */
{ NULL,                  NULL,                                          FALSE }, /* 58 */
{ "highlight_player",    "Highlight the player with the cursor",        FALSE }, /* 59 */
{ "view_yellow_light",   "Use special colors for torch light",          FALSE }, /* 60 */
{ "view_bright_light",   "Use special colors for field of view",        TRUE },  /* 61 */
{ "view_granite_light",  "Use special colors for wall grids",           FALSE }, /* 62 */
{ "view_special_light",  "Use special colors for floor grids",          TRUE },  /* 63 */
{ "easy_open",           "Open/Disarm/Close without direction",         FALSE }, /* 64 */
{ "easy_alter",          "Open/Disarm doors/traps on movement",         FALSE }, /* 65 */
{ "animate_flicker",     "Animate multi-colored monsters and items",    FALSE }, /* 66 */
{ "show_piles",          "Show stacks using special attr/char",         FALSE }, /* 67 */
{ "center_player",       "Center map continuously",                     FALSE }, /* 68 */
{ "purple_uniques",      "Show unique monsters in a special colour",    FALSE }, /* 69 */
{ "xchars_to_file",      "Allow accents in output files",               FALSE }, /* 70 */
{ "auto_more",           "Automatically clear '-more-' prompts",        FALSE }, /* 71 */
{ NULL,                  NULL,                                          FALSE }, /* 72 */
{ NULL,                  NULL,                                          FALSE }, /* 73 */
{ "hp_changes_color",    "Player color indicates low hit points",       FALSE }, /* 74 */
{ "hide_squelchable",    "Hide items set as squelchable",               FALSE }, /* 75 */
{ "squelch_worthless",   "Squelch worthless item kinds",                FALSE }, /* 76 */
{ "mouse_movement",      "Allow mouse clicks to move the player",       FALSE }, /* 77 */
{ "mouse_buttons",        "Show mouse status line buttons",             FALSE }, /* 78 */
{ "notify_recharge",     "Notify on object recharge",                   FALSE }, /* 79 */
{ NULL,                  NULL,                                          FALSE }, /* 80 */
{ NULL,                  NULL,                                          FALSE }, /* 81 */
{ NULL,                  NULL,                                          FALSE }, /* 82 */
{ NULL,                  NULL,                                          FALSE }, /* 83 */
{ NULL,                  NULL,                                          FALSE }, /* 84 */
{ NULL,                  NULL,                                          FALSE }, /* 85 */
{ NULL,                  NULL,                                          FALSE }, /* 86 */
{ NULL,                  NULL,                                          FALSE }, /* 87 */
{ NULL,                  NULL,                                          FALSE }, /* 88 */
{ NULL,                  NULL,                                          FALSE }, /* 89 */
{ NULL,                  NULL,                                          FALSE }, /* 90 */
{ NULL,                  NULL,                                          FALSE }, /* 91 */
{ NULL,                  NULL,                                          FALSE }, /* 92 */
{ NULL,                  NULL,                                          FALSE }, /* 93 */
{ NULL,                  NULL,                                          FALSE }, /* 94 */
{ NULL,                  NULL,                                          FALSE }, /* 95 */
{ NULL,                  NULL,                                          FALSE }, /* 96 */
{ NULL,                  NULL,                                          FALSE }, /* 97 */
{ NULL,                  NULL,                                          FALSE }, /* 98 */
{ NULL,                  NULL,                                          FALSE }, /* 99 */
{ NULL,                  NULL,                                          FALSE }, /* 100 */
{ NULL,                  NULL,                                          FALSE }, /* 101 */
{ NULL,                  NULL,                                          FALSE }, /* 102 */
{ NULL,                  NULL,                                          FALSE }, /* 103 */
{ NULL,                  NULL,                                          FALSE }, /* 104 */
{ NULL,                  NULL,                                          FALSE }, /* 105 */
{ NULL,                  NULL,                                          FALSE }, /* 106 */
{ NULL,                  NULL,                                          FALSE }, /* 107 */
{ NULL,                  NULL,                                          FALSE }, /* 108 */
{ NULL,                  NULL,                                          FALSE }, /* 109 */
{ NULL,                  NULL,                                          FALSE }, /* 110 */
{ NULL,                  NULL,                                          FALSE }, /* 111 */
{ NULL,                  NULL,                                          FALSE }, /* 112 */
{ NULL,                  NULL,                                          FALSE }, /* 113 */
{ NULL,                  NULL,                                          FALSE }, /* 114 */
{ NULL,                  NULL,                                          FALSE }, /* 115 */
{ NULL,                  NULL,                                          FALSE }, /* 116 */
{ NULL,                  NULL,                                          FALSE }, /* 117 */
{ NULL,                  NULL,                                          FALSE }, /* 118 */
{ NULL,                  NULL,                                          FALSE }, /* 119 */
{ NULL,                  NULL,                                          FALSE }, /* 120 */
{ NULL,                  NULL,                                          FALSE }, /* 121 */
{ NULL,                  NULL,                                          FALSE }, /* 122 */
{ NULL,                  NULL,                                          FALSE }, /* 123 */
{ NULL,                  NULL,                                          FALSE }, /* 124 */
{ NULL,                  NULL,                                          FALSE }, /* 125 */
{ NULL,                  NULL,                                          FALSE }, /* 126 */
{ NULL,                  NULL,                                          FALSE }, /* 127 */
{ "birth_maximize",      "Maximise effect of race/class bonuses",       TRUE },  /* 128 */
{ "birth_randarts",      "Randomise the artifacts (except a very few)", FALSE }, /* 129 */
{ NULL,                  NULL,                                          FALSE }, /* 130 */
{ "birth_ironman",       "Restrict the use of stairs/recall",           FALSE }, /* 131 */
{ "birth_no_stores",     "Restrict the use of stores/home",             FALSE }, /* 132 */
{ "birth_no_artifacts",  "Restrict creation of artifacts",              FALSE }, /* 133 */
{ "birth_no_stacking",   "Don't stack objects on the floor",            FALSE }, /* 134 */
{ "birth_no_preserve",   "Lose artifacts when leaving level",           FALSE }, /* 135 */
{ "birth_no_stairs",     "Don't generate connected stairs",             FALSE }, /* 136 */
{ "birth_feelings",      "Don't show level feelings",                   FALSE }, /* 137 */
{ NULL,                  NULL,                                          FALSE }, /* 138 */
{ NULL,                  NULL,                                          FALSE }, /* 139 */
{ NULL,                  NULL,                                          FALSE }, /* 140 */
{ "birth_ai_sound",      "Monsters chase current location",             TRUE },  /* 141 */
{ "birth_ai_smell",      "Monsters chase recent locations",             TRUE },  /* 142 */
{ "birth_ai_packs",      "Monsters act smarter in groups",              TRUE },  /* 143 */
{ "birth_ai_learn",      "Monsters learn from their mistakes",          FALSE }, /* 144 */
{ "birth_ai_cheat",      "Monsters exploit player's weaknesses",        FALSE }, /* 145 */
{ "birth_ai_smart",      "Monsters behave more intelligently (broken)", FALSE }, /* 146 */
{ NULL,                  NULL,                                          FALSE }, /* 147 */
{ NULL,                  NULL,                                          FALSE }, /* 148 */
{ NULL,                  NULL,                                          FALSE }, /* 149 */
{ NULL,                  NULL,                                          FALSE }, /* 150 */
{ NULL,                  NULL,                                          FALSE }, /* 151 */
{ NULL,                  NULL,                                          FALSE }, /* 152 */
{ NULL,                  NULL,                                          FALSE }, /* 153 */
{ NULL,                  NULL,                                          FALSE }, /* 154 */
{ NULL,                  NULL,                                          FALSE }, /* 155 */
{ NULL,                  NULL,                                          FALSE }, /* 156 */
{ NULL,                  NULL,                                          FALSE }, /* 157 */
{ NULL,                  NULL,                                          FALSE }, /* 158 */
{ NULL,                  NULL,                                          FALSE }, /* 159 */
{ "cheat_peek",          "Cheat: Peek into object creation",            FALSE }, /* 160 */
{ "cheat_hear",          "Cheat: Peek into monster creation",           FALSE }, /* 161 */
{ "cheat_room",          "Cheat: Peek into dungeon creation",           FALSE }, /* 162 */
{ "cheat_xtra",          "Cheat: Peek into something else",             FALSE }, /* 163 */
{ "cheat_know",          "Cheat: Know complete monster info",           FALSE }, /* 164 */
{ "cheat_live",          "Cheat: Allow player to avoid death",          FALSE }, /* 165 */
{ NULL,                  NULL,                                          FALSE }, /* 166 */
{ NULL,                  NULL,                                          FALSE }, /* 167 */
{ NULL,                  NULL,                                          FALSE }, /* 168 */
{ NULL,                  NULL,                                          FALSE }, /* 169 */
{ NULL,                  NULL,                                          FALSE }, /* 170 */
{ NULL,                  NULL,                                          FALSE }, /* 171 */
{ NULL,                  NULL,                                          FALSE }, /* 172 */
{ NULL,                  NULL,                                          FALSE }, /* 173 */
{ NULL,                  NULL,                                          FALSE }, /* 174 */
{ NULL,                  NULL,                                          FALSE }, /* 175 */
{ NULL,                  NULL,                                          FALSE }, /* 176 */
{ NULL,                  NULL,                                          FALSE }, /* 177 */
{ NULL,                  NULL,                                          FALSE }, /* 178 */
{ NULL,                  NULL,                                          FALSE }, /* 179 */
{ NULL,                  NULL,                                          FALSE }, /* 180 */
{ NULL,                  NULL,                                          FALSE }, /* 181 */
{ NULL,                  NULL,                                          FALSE }, /* 182 */
{ NULL,                  NULL,                                          FALSE }, /* 183 */
{ NULL,                  NULL,                                          FALSE }, /* 184 */
{ NULL,                  NULL,                                          FALSE }, /* 185 */
{ NULL,                  NULL,                                          FALSE }, /* 186 */
{ NULL,                  NULL,                                          FALSE }, /* 187 */
{ NULL,                  NULL,                                          FALSE }, /* 188 */
{ NULL,                  NULL,                                          FALSE }, /* 189 */
{ NULL,                  NULL,                                          FALSE }, /* 190 */
{ NULL,                  NULL,                                          FALSE }, /* 191 */
{ "adult_maximize",      "Maximize effect of race/class bonuses",       TRUE },  /* 192 */
{ "adult_randarts",      "Randomize the artifacts (except a few)",      FALSE }, /* 193 */
{ NULL,                  NULL,                                          FALSE }, /* 194 */
{ "adult_ironman",       "Restrict the use of stairs/recall",           FALSE }, /* 195 */
{ "adult_no_stores",     "Restrict the use of stores/home",             FALSE }, /* 196 */
{ "adult_no_artifacts",  "Restrict creation of artifacts",              FALSE }, /* 197 */
{ "adult_no_stacking",   "Don't stack objects on the floor",            FALSE }, /* 198 */
{ "adult_no_preserve",   "Lose artifacts when leaving level",           FALSE }, /* 199 */
{ "adult_no_stairs",     "Don't generate connected stairs",             FALSE }, /* 200 */
{ NULL,                  NULL,                                          FALSE }, /* 201 */
{ NULL,                  NULL,                                          FALSE }, /* 202 */
{ NULL,                  NULL,                                          FALSE }, /* 203 */
{ NULL,                  NULL,                                          FALSE }, /* 204 */
{ "adult_ai_sound",      "Adult: Monsters chase current location",      TRUE },  /* 205 */
{ "adult_ai_smell",      "Adult: Monsters chase recent locations",      TRUE },  /* 206 */
{ "adult_ai_packs",      "Adult: Monsters act smarter in groups",       TRUE },  /* 207 */
{ "adult_ai_learn",      "Adult: Monsters learn from their mistakes",   FALSE }, /* 208 */
{ "adult_ai_cheat",      "Adult: Monsters exploit players weaknesses",  FALSE }, /* 209 */
{ "adult_ai_smart",      "Adult: Monsters behave more intelligently (broken)",  FALSE }, /* 210 */
{ NULL,                  NULL,                                          FALSE }, /* 211 */
{ NULL,                  NULL,                                          FALSE }, /* 212 */
{ NULL,                  NULL,                                          FALSE }, /* 213 */
{ NULL,                  NULL,                                          FALSE }, /* 214 */
{ NULL,                  NULL,                                          FALSE }, /* 215 */
{ NULL,                  NULL,                                          FALSE }, /* 216 */
{ NULL,                  NULL,                                          FALSE }, /* 217 */
{ NULL,                  NULL,                                          FALSE }, /* 218 */
{ NULL,                  NULL,                                          FALSE }, /* 219 */
{ NULL,                  NULL,                                          FALSE }, /* 220 */
{ NULL,                  NULL,                                          FALSE }, /* 221 */
{ NULL,                  NULL,                                          FALSE }, /* 222 */
{ NULL,                  NULL,                                          FALSE }, /* 223 */
{ "score_peek",          "Score: Peek into object creation",            FALSE }, /* 224 */
{ "score_hear",          "Score: Peek into monster creation",           FALSE }, /* 225 */
{ "score_room",          "Score: Peek into dungeon creation",           FALSE }, /* 226 */
{ "score_xtra",          "Score: Peek into something else",             FALSE }, /* 227 */
{ "score_know",          "Score: Know complete monster info",           FALSE }, /* 228 */
{ "score_live",          "Score: Allow player to avoid death",          FALSE }, /* 229 */
{ NULL,                  NULL,                                          FALSE }, /* 230 */
{ NULL,                  NULL,                                          FALSE }, /* 231 */
{ NULL,                  NULL,                                          FALSE }, /* 232 */
{ NULL,                  NULL,                                          FALSE }, /* 233 */
{ NULL,                  NULL,                                          FALSE }, /* 234 */
{ NULL,                  NULL,                                          FALSE }, /* 235 */
{ NULL,                  NULL,                                          FALSE }, /* 236 */
{ NULL,                  NULL,                                          FALSE }, /* 237 */
{ NULL,                  NULL,                                          FALSE }, /* 238 */
{ NULL,                  NULL,                                          FALSE }, /* 239 */
{ NULL,                  NULL,                                          FALSE }, /* 240 */
{ NULL,                  NULL,                                          FALSE }, /* 241 */
{ NULL,                  NULL,                                          FALSE }, /* 242 */
{ NULL,                  NULL,                                          FALSE }, /* 243 */
{ NULL,                  NULL,                                          FALSE }, /* 244 */
{ NULL,                  NULL,                                          FALSE }, /* 245 */
{ NULL,                  NULL,                                          FALSE }, /* 246 */
{ NULL,                  NULL,                                          FALSE }, /* 247 */
{ NULL,                  NULL,                                          FALSE }, /* 248 */
{ NULL,                  NULL,                                          FALSE }, /* 249 */
{ NULL,                  NULL,                                          FALSE }, /* 250 */
{ NULL,                  NULL,                                          FALSE }, /* 251 */
{ NULL,                  NULL,                                          FALSE }, /* 252 */
{ NULL,                  NULL,                                          FALSE }, /* 253 */
{ NULL,                  NULL,                                          FALSE }, /* 254 */
{ NULL,                  NULL,                                          FALSE }, /* 255 */
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

/* Setup functions */
bool option_set(const char *name, bool on)
{
	size_t opt;
	for (opt = 0; opt < OPT_ADULT; opt++)
	{
		if (!options[opt].name || !streq(options[opt].name, name))
			continue;

		op_ptr->opt[opt] = on;
		if (on && opt > OPT_CHEAT && opt < OPT_ADULT)
			op_ptr->opt[opt + (OPT_SCORE - OPT_CHEAT)] = TRUE;

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
