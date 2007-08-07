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
const byte option_page[OPT_PAGE_MAX][OPT_PAGE_PER] =
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
		OPT_NONE,
		OPT_NONE,
		OPT_NONE,
	},

	/* Display */
	{
		OPT_hp_changes_color,
		OPT_depth_in_feet,
		OPT_hilite_player,
 		OPT_center_player,
		OPT_show_piles,
		OPT_show_flavors,
		OPT_show_labels,
		OPT_view_yellow_lite,
		OPT_view_bright_lite,
		OPT_view_granite_lite,
		OPT_view_special_lite,
		OPT_view_perma_grids,
		OPT_view_torch_grids,
		OPT_NONE,
		OPT_NONE,
	},

	/* Warning */
	{
		OPT_disturb_move,
		OPT_disturb_near,
		OPT_disturb_detect,
		OPT_disturb_state,
		OPT_disturb_minor,
		OPT_quick_messages,
		OPT_auto_more,
		OPT_ring_bell,
		OPT_flush_failure,
		OPT_flush_disturb,
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
		OPT_birth_autoscum,
		OPT_birth_ironman,
		OPT_birth_no_stores,
		OPT_birth_no_artifacts,
		OPT_birth_no_stacking,
		OPT_birth_no_preserve,
		OPT_birth_no_stairs,
		OPT_birth_ai_sound,
		OPT_birth_ai_smell,
		OPT_birth_ai_packs,
		OPT_birth_ai_learn,
		OPT_birth_ai_cheat,
		OPT_birth_ai_smart,
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
	}
};


/*
 * Options -- textual names (where defined)
 */
static const char *names[OPT_MAX] =
{
	"rogue_like_commands",		/* OPT_rogue_like_commands */
	"quick_messages",			/* OPT_quick_messages */
	"use_sound",				/* OPT_use_sound */
	"pickup_detail",			/* OPT_pickup_detail */
	"use_old_target",			/* OPT_use_old_target */
	"pickup_always",			/* OPT_pickup_always */
	"pickup_inven",				/* OPT_pickup_inven */
	"depth_in_feet",			/* OPT_depth_in_feet */
	NULL,						/* xxx stack_force_notes */
	NULL,						/* xxx stack_force_costs */
	"show_labels",				/* OPT_show_labels */
	"show_lists",				/* OPT_show_lists */
	NULL,						/* xxx show_choices */
	NULL,						/* xxx show_details */
	"ring_bell",				/* OPT_ring_bell */
	"show_flavors",				/* OPT_flavors */
	NULL,						/* xxx run_ignore_stairs */
	NULL,						/* xxx run_ignore_doors */
	NULL,						/* xxx run_cut_corners */
	NULL,						/* xxx run_use_corners */
	"disturb_move",				/* OPT_disturb_move */
	"disturb_near",				/* OPT_disturb_near */
	"disturb_detect",			/* OPT_disturb_detect */
	"disturb_state",			/* OPT_disturb_state */
	"disturb_minor",			/* OPT_disturb_minor */
	NULL,						/* xxx next_xp */
	NULL,						/* xxx alert_hitpoint */
	NULL,						/* xxx alert_failure */
	NULL,						/* xxx verify_destroy */
	NULL,						/* xxx verify_special */
	NULL,						/* xxx allow_quantity */
	NULL,						/* xxx */
	NULL,						/* xxx auto_haggle */
	NULL,						/* xxx auto_scum */
	NULL,						/* xxx testing_stack */
	NULL,						/* xxx testing_carry */
	NULL,						/* xxx expand_look */
	NULL,						/* xxx expand_list */
	"view_perma_grids",			/* OPT_view_perma_grids */
	"view_torch_grids",			/* OPT_view_torch_grids */
	NULL,						/* xxx dungeon_align */
	NULL,						/* xxx dungeon_stair */
	NULL,						/* xxx flow_by_sound */
	NULL,						/* xxx flow_by_smell */
	NULL,						/* xxx track_follow */
	NULL,						/* xxx track_target */
	NULL,						/* xxx smart_learn */
	NULL,						/* xxx smart_cheat */
	NULL,						/* xxx view_reduce_lite */
	NULL,						/* xxx hidden_player */
	NULL,						/* xxx avoid_abort */
	NULL,						/* xxx avoid_other */
	"flush_failure",			/* OPT_flush_failure */
	"flush_disturb",			/* OPT_flush_disturb */
	NULL,						/* xxx flush_command */
	NULL,						/* xxx fresh_before */
	NULL,						/* xxx fresh_after */
	NULL,						/* xxx fresh_message */
	NULL,						/* xxx compress_savefile */
	"hilite_player",			/* OPT_hilite_player */
	"view_yellow_lite",			/* OPT_view_yellow_lite */
	"view_bright_lite",			/* OPT_view_bright_lite */
	"view_granite_lite",		/* OPT_view_granite_lite */
	"view_special_lite",		/* OPT_view_special_lite */
	"easy_open",				/* OPT_easy_open */
	"easy_alter",				/* OPT_easy_alter */
	NULL,						/* xxx easy_floor */
	"show_piles",				/* OPT_show_piles */
	"center_player",			/* OPT_center_player */
	NULL,						/* xxx run_avoid_center */
	NULL,						/* xxx scroll_target */
	"auto_more",				/* OPT_auto_more */
	NULL,						/* xxx smart_monsters */
	NULL,						/* xxx smart_packs */
	"hp_changes_color",			/* OPT_hp_changes_color */
	"hide_squelchable",			/* OPT_hide_squelchable */
	"squelch_worthless",			/* OPT_squelch_worthless */
	"mouse_movement",			/* OPT_mouse_movement */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	"birth_maximize",
	"birth_randarts",
	"birth_autoscum",
	"birth_ironman",
	"birth_no_stores",
	"birth_no_artifacts",
	"birth_no_stacking",
	"birth_no_preserve",
	"birth_no_stairs",
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	"birth_ai_sound",
	"birth_ai_smell",
	"birth_ai_packs",
	"birth_ai_learn",
	"birth_ai_cheat",
	"birth_ai_smart",
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	"cheat_peek",				/* OPT_cheat_peek */
	"cheat_hear",				/* OPT_cheat_hear */
	"cheat_room",				/* OPT_cheat_room */
	"cheat_xtra",				/* OPT_cheat_xtra */
	"cheat_know",				/* OPT_cheat_know */
	"cheat_live",				/* OPT_cheat_live */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	"adult_maximize",
	"adult_randarts",
	"adult_autoscum",
	"adult_ironman",
	"adult_no_stores",
	"adult_no_artifacts",
	"adult_no_stacking",
	"adult_no_preserve",
	"adult_no_stairs",
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	"adult_ai_sound",
	"adult_ai_smell",
	"adult_ai_packs",
	"adult_ai_learn",
	"adult_ai_cheat",
	"adult_ai_smart",
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	"score_peek",				/* OPT_score_peek */
	"score_hear",				/* OPT_score_hear */
	"score_room",				/* OPT_score_room */
	"score_xtra",				/* OPT_score_xtra */
	"score_know",				/* OPT_score_know */
	"score_live",				/* OPT_score_live */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL,						/* xxx */
	NULL						/* xxx */
};


/*
 * Options -- descriptions (where defined)
 */
static const char *descs[OPT_MAX] =
{
	"Rogue-like commands",						/* OPT_rogue_like_commands */
	"Activate quick messages",					/* OPT_quick_messages */
	"Use sound",								/* OPT_use_sound */
	"Be verbose when picking things up",		/* OPT_pickup_detail */
	"Use old target by default",				/* OPT_use_old_target */
	"Always pickup items",						/* OPT_pickup_always */
	"Always pickup items matching inventory",	/* OPT_pickup_inven */
	"Show dungeon level in feet",				/* OPT_depth_in_feet */
	NULL,										/* xxx stack_force_notes */
	NULL,										/* xxx stack_force_costs */
	"Show labels in equipment listings",		/* OPT_show_labels */
	"Always show lists",						/* OPT_show_lists */
	NULL,										/* xxx show_choices */
	NULL,										/* xxx show_details */
	"Audible bell (on errors, etc)",			/* OPT_ring_bell */
	"Show flavors in object descriptions",		/* OPT_show_flavors */
	NULL,										/* xxx run_ignore_stairs */
	NULL,										/* xxx run_ignore_doors */
	NULL,										/* xxx run_cut_corners */
	NULL,										/* xxx run_use_corners */
	"Disturb whenever any monster moves",		/* OPT_disturb_move */
	"Disturb whenever viewable monster moves",	/* OPT_disturb_near */
	"Disturb whenever leaving trap detected area", /* OPT_disturb_detect */
	"Disturb whenever player state changes",	/* OPT_disturb_state */
	"Disturb whenever boring things happen",	/* OPT_disturb_minor */
	NULL,										/* xxx next_xp */
	NULL,										/* xxx alert_hitpoint */
	NULL,										/* xxx alert_failure */
	NULL,										/* xxx verify_destroy */
	NULL,										/* xxx verify_special */
	NULL,										/* xxx allow_quantity */
	NULL,										/* xxx */
	NULL,										/* xxx auto_haggle */
	NULL,										/* xxx auto_scum */
	NULL,										/* xxx testing_stack */
	NULL,										/* xxx testing_carry */
	NULL,										/* xxx expand_look */
	NULL, 										/* xxx expand_list */
	"Map remembers all perma-lit grids",		/* OPT_view_perma_grids */
	"Map remembers all torch-lit grids",		/* OPT_view_torch_grids */
	"Generate dungeons with aligned rooms",		/* OPT_dungeon_align */
	"Generate dungeons with connected stairs",	/* OPT_dungeon_stair */
	"Monsters chase current location (slow)",	/* OPT_adult_ai_sound */
	"Monsters chase recent locations (slow)",	/* OPT_adult_ai_smell */
	NULL,										/* xxx track_follow */
	NULL,										/* xxx track_target */
	NULL,										/* xxx smart_learn */
	NULL,										/* xxx smart_cheat */
	NULL,										/* xxx view_reduce_lite */
	NULL,										/* xxx hidden_player */
	NULL,										/* xxx avoid_abort */
	NULL,										/* xxx avoid_other */
	"Flush input on various failures",			/* OPT_flush_failure */
	"Flush input whenever disturbed",			/* OPT_flush_disturb */
	NULL,										/* xxx */
	NULL,										/* xxx fresh_before */
	NULL,										/* xxx fresh_after */
	NULL,										/* xxx */
	NULL,										/* xxx compress_savefile */
	"Hilite the player with the cursor",		/* OPT_hilite_player */
	"Use special colors for torch lite",		/* OPT_view_yellow_lite */
	"Use special colors for field of view",		/* OPT_view_bright_lite */
	"Use special colors for wall grids",		/* OPT_view_granite_lite */
	"Use special colors for floor grids",		/* OPT_view_special_lite */
	"Open/Disarm/Close without direction",		/* OPT_easy_open */
	"Open/Disarm doors/traps on movement",		/* OPT_easy_alter */
	NULL,										/* xxx easy_floor */
	"Show stacks using special attr/char",		/* OPT_show_piles */
	"Center map continuously",					/* OPT_center_player */
	NULL,										/* xxx run_avoid_center */
	NULL,										/* xxx scroll_target */
	"Automatically clear '-more-' prompts",		/* OPT_auto_more */
	NULL,										/* xxx smart_monsters */
	NULL,										/* xxx smart_packs */
	"Player color indicates low hit points",	/* OPT_hp_changes_color */
	"Hide items set as squelchable",			/* OPT_hide_squelchable */
	"Automatically squelch worthless items",	/* OPT_squelch_worthless */
	"Allow mouse clicks to move the player",	/* OPT_mouse_movement */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	"Maximize effect of race/class bonuses",	/* OPT_birth_maximize */
	"Randomize some of the artifacts (alpha)",	/* OPT_birth_randarts */
	"Auto-scum for good levels",				/* OPT_birth_autoscum */
	"Restrict the use of stairs/recall",		/* OPT_birth_ironman */
	"Restrict the use of stores/home",			/* OPT_birth_no_stores */
	"Restrict creation of artifacts",			/* OPT_birth_no_artifacts */
	"Don't stack objects on the floor",			/* OPT_birth_no_stacking */
	"Don't preserve artifacts when leaving level",	/* OPT_birth_no_preserve */
	"Don't generate connected stairs",			/* OPT_birth_no_stairs */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	"Monsters chase current location",
	"Monsters chase recent locations",
	"Monsters act smarter in groups",
	"Monsters learn from their mistakes",
	"Monsters exploit player's weaknesses",
	"Monsters behave more intelligently (broken)",
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	"Cheat: Peek into object creation",			/* OPT_cheat_peek */
	"Cheat: Peek into monster creation",		/* OPT_cheat_hear */
	"Cheat: Peek into dungeon creation",		/* OPT_cheat_room */
	"Cheat: Peek into something else",			/* OPT_cheat_xtra */
	"Cheat: Know complete monster info",		/* OPT_cheat_know */
	"Cheat: Allow player to avoid death",		/* OPT_cheat_live */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	"Adult: Maximize effect of race/class bonuses",	/* OPT_adult_maximize */
	"Adult: Randomize some of the artifacts (beta)",/* OPT_adult_randarts */
	"Adult: Auto-scum for good levels",				/* OPT_adult_autoscum */
	"Adult: Restrict the use of stairs/recall",	/* OPT_adult_ironman */
	"Adult: Restrict the use of stores/home",	/* OPT_adult_no_stores */
	"Adult: Restrict creation of artifacts",	/* OPT_adult_no_artifacts */
	"Adult: Don't stack objects on the floor",	/* OPT_adult_no_stacking */
	"Adult: Preserve artifacts when leaving level",	/* OPT_adult_no_preserve */
	"Adult: Don't generate connected stairs",	/* OPT_adult_no_stairs */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	"Adult: Monsters chase current location",
	"Adult: Monsters chase recent locations",
	"Adult: Monsters act smarter in groups",
	"Adult: Monsters learn from their mistakes",
	"Adult: Monsters exploit players weaknesses",
	"Adult: Monsters behave more intelligently (broken)",
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	"Score: Peek into object creation",			/* OPT_score_peek */
	"Score: Peek into monster creation",		/* OPT_score_hear */
	"Score: Peek into dungeon creation",		/* OPT_score_room */
	"Score: Peek into something else",			/* OPT_score_xtra */
	"Score: Know complete monster info",		/* OPT_score_know */
	"Score: Allow player to avoid death",		/* OPT_score_live */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL,										/* xxx */
	NULL										/* xxx */
};


/*
 * Options -- normal values
 */
static const bool defaults[OPT_MAX] =
{
	FALSE,		/* OPT_rogue_like_commands */
	TRUE,		/* OPT_quick_messages */
	FALSE,		/* OPT_use_sound */
	TRUE,		/* OPT_query_floor */
	FALSE,		/* OPT_use_old_target */
	FALSE,		/* OPT_always_pickup */
	TRUE,		/* OPT_pickup_inven */
	FALSE,		/* OPT_depth_in_feet */
	FALSE,		/* OPT_stack_force_notes */
	FALSE,		/* xxx stack_force_costs */
	TRUE,		/* OPT_show_labels */
	TRUE,		/* OPT_show_weights */
	FALSE,		/* xxx show_choices */
	FALSE,		/* xxx show_details */
	TRUE,		/* OPT_ring_bell */
	TRUE,		/* OPT_show_flavors */
	FALSE,		/* xxx run_ignore_stairs */
	FALSE,		/* xxx run_ignore_doors */
	FALSE,		/* xxx run_cut_corners */
	FALSE,		/* xxx run_use_corners */
	FALSE,		/* OPT_disturb_move */
	TRUE,		/* OPT_disturb_near */
	TRUE,		/* OPT_disturb_detect */
	TRUE,		/* OPT_disturb_state */
	TRUE,		/* OPT_disturb_minor */
	FALSE,		/* xxx next_xp */
	FALSE,		/* xxx alert_hitpoint */
	FALSE,		/* xxx alert_failure */
	FALSE,		/* xxx verify_destroy */
	FALSE,		/* xxx verify_special */
	FALSE,		/* xxx allow_quantity */
	FALSE,		/* xxx */
	FALSE,		/* xxx auto_haggle */
	FALSE,		/* xxx auto_scum */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx expand_look */
	FALSE,		/* xxx expand_list */
	TRUE,		/* OPT_view_perma_grids */
	TRUE,		/* OPT_view_torch_grids */
	TRUE,		/* OPT_dungeon_align */
	TRUE,		/* OPT_dungeon_stair */
	FALSE,		/* xxx adult_ai_sound */
	FALSE,		/* xxx adult_ai_smell */
	FALSE,		/* xxx track_follow */
	FALSE,		/* xxx track_target */
	FALSE,		/* xxx smart_learn */
	FALSE,		/* xxx smart_cheat */
	FALSE,		/* xxx view_reduce_lite */
	FALSE,		/* xxx hidden_player */
	FALSE,		/* xxx avoid_abort */
	FALSE,		/* xxx avoid_other */
	TRUE,		/* OPT_flush_failure */
	FALSE,		/* OPT_flush_disturb */
	FALSE,		/* xxx */
	FALSE,		/* xxx fresh_before */
	FALSE,		/* xxx fresh_after */
	FALSE,		/* xxx */
	FALSE,		/* xxx compress_savefile */
	FALSE,		/* OPT_hilite_player */
	FALSE,		/* OPT_view_yellow_lite */
	TRUE,		/* OPT_view_bright_lite */
	FALSE,		/* OPT_view_granite_lite */
	TRUE,		/* OPT_view_special_lite */
	FALSE,		/* OPT_easy_open */
	FALSE,		/* OPT_easy_alter */
	FALSE,		/* xxx easy_floor */
	FALSE,		/* OPT_show_piles */
	FALSE,		/* OPT_center_player */
	FALSE,		/* xxx run_avoid_center */
	FALSE,		/* xxx */
	FALSE,		/* OPT_auto_more */
	FALSE,		/* xxx smart_monsters */
	FALSE,		/* xxx smart_packs */
	FALSE,		/* OPT_hp_changes_color */
	FALSE,		/* OPT_hide_squelchable */
	FALSE,		/* OPT_squelch_worthless */
	FALSE,		/* OPT_mouse_movement */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	TRUE,		/* birth_maximise */
	FALSE,		/* birth_randarts */
	FALSE,		/* birth_autoscum */
	FALSE,		/* birth_ironman */
	FALSE,		/* birth_no_stores */
	FALSE,		/* birth_no_artifacts */
	FALSE,		/* birth_no_stacking */
	FALSE,		/* birth_no_preserve */
	FALSE,		/* birth_no_stairs */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	TRUE,		/* birth_ai_sound */
	TRUE,		/* birth_ai_smell */
	TRUE,		/* birth_ai_packs */
	FALSE,		/* birth_ai_learn */
	FALSE,		/* birth_ai_cheat */
	FALSE,		/* birth_ai_smart */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* OPT_cheat_peek */
	FALSE,		/* OPT_cheat_hear */
	FALSE,		/* OPT_cheat_room */
	FALSE,		/* OPT_cheat_xtra */
	FALSE,		/* OPT_cheat_know */
	FALSE,		/* OPT_cheat_live */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	TRUE,		/* adult_maximise */
	FALSE,		/* adult_randarts */
	FALSE,		/* adult_autoscum */
	FALSE,		/* adult_ironman */
	FALSE,		/* adult_no_stores */
	FALSE,		/* adult_no_artifacts */
	FALSE,		/* adult_no_stacking */
	FALSE,		/* adult_no_preserve */
	FALSE,		/* adult_no_stairs */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	TRUE,		/* adult_ai_sound */
	TRUE,		/* adult_ai_smell */
	TRUE,		/* adult_ai_packs */
	FALSE,		/* adult_ai_learn */
	FALSE,		/* adult_ai_cheat */
	FALSE,		/* adult_ai_smart */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* OPT_score_peek */
	FALSE,		/* OPT_score_hear */
	FALSE,		/* OPT_score_room */
	FALSE,		/* OPT_score_xtra */
	FALSE,		/* OPT_score_know */
	FALSE,		/* OPT_score_live */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE,		/* xxx */
	FALSE		/* xxx */
};


/* Accessor functions */
const char *option_name(int opt)
{
	if (opt >= OPT_MAX) return NULL;
	return names[opt];
}

const char *option_desc(int opt)
{
	if (opt >= OPT_MAX) return NULL;
	return descs[opt];
}

/* Setup functions */
void option_set(int opt, bool on)
{
	op_ptr->opt[opt] = on;
}

void option_set_defaults(void)
{
	size_t opt;

	for (opt = 0; opt < OPT_MAX; opt++)
		op_ptr->opt[opt] = defaults[opt];
}

