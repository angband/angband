/**
 * \file list-options.h
 * \brief options
 *
 * Currently, if there are more than 21 of any option type, the later ones
 * will be ignored
 * Cheat options need to be followed by corresponding score options
 */

/* name                   description
type     normal */
OP(none,                  "",
SPECIAL, false)
OP(rogue_like_commands,   "Use the roguelike command keyset",
INTERFACE, false)
OP(use_sound,             "Use sound",
INTERFACE, false)
OP(show_damage,           "Show damage player deals to monsters",
INTERFACE, false)
OP(use_old_target,        "Use old target by default",
INTERFACE, false)
OP(pickup_always,         "Always pickup items",
INTERFACE, false)
OP(pickup_inven,          "Always pickup items matching inventory",
INTERFACE, true)
OP(show_flavors,          "Show flavors in object descriptions",
INTERFACE, false)
OP(show_target,           "Highlight target with cursor",
INTERFACE, true)
OP(highlight_player,      "Highlight player with cursor between turns",
INTERFACE, false)
OP(disturb_near,          "Disturb whenever viewable monster moves",
INTERFACE, true)
OP(solid_walls,           "Show walls as solid blocks",
INTERFACE, false)
OP(hybrid_walls,          "Show walls with shaded background",
INTERFACE, false)
OP(view_yellow_light,     "Color: Illuminate torchlight in yellow",
INTERFACE, false)
OP(animate_flicker,       "Color: Shimmer multi-colored things",
INTERFACE, false)
OP(center_player,         "Center map continuously",
INTERFACE, false)
OP(purple_uniques,        "Color: Show unique monsters in purple",
INTERFACE, false)
OP(auto_more,             "Automatically clear '-more-' prompts",
INTERFACE, false)
OP(hp_changes_color,      "Color: Player color indicates % hit points",
INTERFACE, true)
OP(mouse_movement,        "Allow mouse clicks to move the player",
INTERFACE, true)
OP(notify_recharge,       "Notify on object recharge",
INTERFACE, false)
OP(effective_speed,       "Show effective speed as multiplier",
INTERFACE, false)
OP(cheat_hear,            "Cheat: Peek into monster creation",
CHEAT, false)
OP(score_hear,            "Score: Peek into monster creation",
SCORE, false)
OP(cheat_room,            "Cheat: Peek into dungeon creation",
CHEAT, false)
OP(score_room,            "Score: Peek into dungeon creation",
SCORE, false)
OP(cheat_xtra,            "Cheat: Peek into something else",
CHEAT, false)
OP(score_xtra,            "Score: Peek into something else",
SCORE, false)
OP(cheat_live,            "Cheat: Allow player to avoid death",
CHEAT, false)
OP(score_live,            "Score: Allow player to avoid death",
SCORE, false)
OP(birth_randarts,        "Generate a new, random artifact set",
BIRTH, false)
OP(birth_connect_stairs,  "Generate connected stairs",
BIRTH, true)
OP(birth_force_descend,   "Force player descent (never make up stairs)",
BIRTH, false)
OP(birth_no_recall,       "Word of Recall has no effect",
BIRTH, false)
OP(birth_no_artifacts,    "Restrict creation of artifacts",
BIRTH, false)
OP(birth_stacking,        "Stack objects on the floor",
BIRTH, true)
OP(birth_lose_arts,       "Lose artifacts when leaving level",
BIRTH, false)
OP(birth_feelings,        "Show level feelings",
BIRTH, true)
OP(birth_no_selling,      "Increase gold drops but disable selling",
BIRTH, true)
OP(birth_start_kit,       "Start with a kit of useful gear",
BIRTH, true)
OP(birth_ai_learn,        "Monsters learn from their mistakes",
BIRTH, true)
OP(birth_know_runes,      "Know all runes on birth",
BIRTH, false)
OP(birth_know_flavors,    "Know all flavors on birth",
BIRTH, false)
OP(birth_levels_persist,  "Persistent levels (experimental)",
BIRTH, false)
OP(birth_percent_damage,  "To-damage is a percentage of dice (experimental)",
BIRTH, false)

