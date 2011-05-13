#ifndef INCLUDED_OPTIONS_H
#define INCLUDED_OPTIONS_H

/*** Functions ***/

/** Given an option index, return its name */
const char *option_name(int opt);

/** Given an option index, return its description */
const char *option_desc(int opt);

/** Set an an option, return TRUE if successful */
bool option_set(const char *opt, bool on);

/** Reset options to defaults */
void option_set_defaults(void);


/*** Option display definitions ***/

/*
 * Information for "do_cmd_options()".
 */
#define OPT_PAGE_MAX				4
#define OPT_PAGE_PER				16

#define OPT_PAGE_BIRTH				2

/* The option data structures */
extern const int option_page[OPT_PAGE_MAX][OPT_PAGE_PER];



/*** Option definitions ***/

/*
 * Option indexes (offsets)
 *
 * These values are hard-coded by savefiles (and various pieces of code).  Ick.
 */
#define OPT_CHEAT					30
#define OPT_SCORE					40
#define OPT_BIRTH					50

#define OPT_NONE					89
#define OPT_MAX						90

#define N_OPTS_CHEAT				6
#define N_OPTS_BIRTH				16

/*
 * Option indexes (hard-coded by savefiles)
 */
#define OPT_rogue_like_commands		0
#define OPT_use_sound               1
#define OPT_use_old_target			3
#define OPT_pickup_always			4
#define OPT_pickup_inven			5
#define OPT_show_flavors			6
#define OPT_disturb_move			7
#define OPT_disturb_near			8
#define OPT_disturb_detect			9
#define OPT_disturb_state			10
#define OPT_view_yellow_light		13
#define OPT_easy_open 				14
#define OPT_animate_flicker         15
#define OPT_center_player			16
#define OPT_purple_uniques			17
#define OPT_xchars_to_file			18
#define OPT_auto_more				19
#define OPT_hp_changes_color		20
#define OPT_mouse_movement			21
#define OPT_mouse_buttons			22
#define OPT_notify_recharge			23

#define OPT_cheat_hear				(OPT_CHEAT+1)
#define OPT_cheat_room				(OPT_CHEAT+2)
#define OPT_cheat_xtra				(OPT_CHEAT+3)
#define OPT_cheat_know				(OPT_CHEAT+4)
#define OPT_cheat_live				(OPT_CHEAT+5)

#define OPT_score_hear				(OPT_SCORE+1)
#define OPT_score_room				(OPT_SCORE+2)
#define OPT_score_xtra				(OPT_SCORE+3)
#define OPT_score_know				(OPT_SCORE+4)
#define OPT_score_live				(OPT_SCORE+5)

#define OPT_birth_maximize          (OPT_BIRTH+0)
#define OPT_birth_randarts          (OPT_BIRTH+1)
#define OPT_birth_ironman           (OPT_BIRTH+2)
#define OPT_birth_no_stores         (OPT_BIRTH+3)
#define OPT_birth_no_artifacts      (OPT_BIRTH+4)
#define OPT_birth_no_stacking       (OPT_BIRTH+5)
#define OPT_birth_no_preserve       (OPT_BIRTH+6)
#define OPT_birth_no_stairs			(OPT_BIRTH+7)
#define OPT_birth_no_feelings	    (OPT_BIRTH+8)
#define OPT_birth_no_selling 	    (OPT_BIRTH+9)
#define OPT_birth_keep_randarts		(OPT_BIRTH+10)
#define OPT_birth_ai_smell			(OPT_BIRTH+11)
#define OPT_birth_ai_packs			(OPT_BIRTH+12)
#define OPT_birth_ai_learn			(OPT_BIRTH+13)
#define OPT_birth_ai_cheat			(OPT_BIRTH+14)
#define OPT_birth_ai_smart			(OPT_BIRTH+15)


#define OPT(opt_name)	op_ptr->opt[OPT_##opt_name]

#endif /* !INCLUDED_OPTIONS_H */
