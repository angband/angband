/**
 * \file list-options.h
 * \brief options
 *
 * Currently, if there are more than 20 of any option type, the later ones
 * will be ignored
 * Cheat options need to be followed by corresponding score options
 */

/* name                   description
type     normal */
OP(none,                  "",
SPECIAL, FALSE)
OP(rogue_like_commands,   "Use the roguelike command keyset",
INTERFACE, FALSE)
OP(use_sound,             "Use sound",
INTERFACE, FALSE)
OP(show_damage,           "Show damage player deals to monsters",
INTERFACE, FALSE)
OP(use_old_target,        "Use old target by default",
INTERFACE, FALSE)
OP(pickup_always,         "Always pickup items",
INTERFACE, FALSE)
OP(pickup_inven,          "Always pickup items matching inventory",
INTERFACE, TRUE)
OP(show_flavors,          "Show flavors in object descriptions",
INTERFACE, FALSE)
OP(show_target,           "Highlight target with cursor",
INTERFACE, TRUE)
OP(disturb_near,          "Disturb whenever viewable monster moves",
INTERFACE, TRUE)
OP(solid_walls,           "Show walls as solid blocks",
INTERFACE, FALSE)
OP(hybrid_walls,          "Show walls with shaded background",
INTERFACE, FALSE)
OP(view_yellow_light,     "Color: Illuminate torchlight in yellow",
INTERFACE, FALSE)
OP(animate_flicker,       "Color: Shimmer multi-colored things",
INTERFACE, FALSE)
OP(center_player,         "Center map continuously",
INTERFACE, FALSE)
OP(purple_uniques,        "Color: Show unique monsters in purple",
INTERFACE, FALSE)
OP(auto_more,             "Automatically clear '-more-' prompts",
INTERFACE, FALSE)
OP(hp_changes_color,      "Color: Player color indicates % hit points",
INTERFACE, TRUE)
OP(mouse_movement,        "Allow mouse clicks to move the player",
INTERFACE, TRUE)
OP(notify_recharge,       "Notify on object recharge",
INTERFACE, FALSE)
OP(cheat_hear,            "Cheat: Peek into monster creation",
CHEAT, FALSE)
OP(score_hear,            "Score: Peek into monster creation",
SCORE, FALSE)
OP(cheat_room,            "Cheat: Peek into dungeon creation",
CHEAT, FALSE)
OP(score_room,            "Score: Peek into dungeon creation",
SCORE, FALSE)
OP(cheat_xtra,            "Cheat: Peek into something else",
CHEAT, FALSE)
OP(score_xtra,            "Score: Peek into something else",
SCORE, FALSE)
OP(cheat_know,            "Cheat: Know complete monster info",
CHEAT, FALSE)
OP(score_know,            "Score: Know complete monster info",
SCORE, FALSE)
OP(cheat_live,            "Cheat: Allow player to avoid death",
CHEAT, FALSE)
OP(score_live,            "Score: Allow player to avoid death",
SCORE, FALSE)
OP(birth_randarts,        "Randomise the artifacts (except a very few)",
BIRTH, FALSE)
OP(birth_no_recall,       "Word of Recall has no effect",
BIRTH, FALSE)
OP(birth_no_artifacts,    "Restrict creation of artifacts",
BIRTH, FALSE)
OP(birth_no_stacking,     "Don't stack objects on the floor",
BIRTH, FALSE)
OP(birth_no_preserve,     "Lose artifacts when leaving level",
BIRTH, FALSE)
OP(birth_no_stairs,       "Don't generate connected stairs",
BIRTH, FALSE)
OP(birth_no_feelings,     "Don't show level feelings",
BIRTH, FALSE)
OP(birth_no_selling,      "Increase gold drops but disable selling",
BIRTH, TRUE)
OP(birth_keep_randarts,   "Use previous set of randarts",
BIRTH, TRUE)
OP(birth_start_kit,       "Start with a kit of useful gear",
BIRTH, TRUE)
OP(birth_ai_learn,        "Monsters learn from their mistakes",
BIRTH, FALSE)
OP(birth_force_descend,   "Force player descent",
BIRTH, FALSE)

