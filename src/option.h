#ifndef INCLUDED_OPTIONS_H
#define INCLUDED_OPTIONS_H


const char *option_name(int opt);
const char *option_desc(int opt);

bool option_set(const char *opt, bool on);
void option_set_defaults(void);


/*** Option display definitions ***/

/*
 * Information for "do_cmd_options()".
 */
#define OPT_PAGE_MAX				5
#define OPT_PAGE_PER				16

/* The option data structures */
extern const int option_page[OPT_PAGE_MAX][OPT_PAGE_PER];



/*** Option definitions ***/

/*
 * Option indexes (offsets)
 *
 * These values are hard-coded by savefiles (and various pieces of code).  Ick.
 */
#define OPT_BIRTH					128
#define OPT_CHEAT					160
#define OPT_ADULT					192
#define OPT_SCORE					224
#define OPT_NONE					255
#define OPT_MAX						256


/*
 * Option indexes (hard-coded by savefiles)
 */
#define OPT_rogue_like_commands		0
#define OPT_quick_messages			1
#define OPT_use_sound               2
#define OPT_pickup_detail			3
#define OPT_use_old_target			4
#define OPT_pickup_always			5
#define OPT_pickup_inven			6

#define OPT_show_labels				10
#define OPT_show_lists              11

#define OPT_ring_bell				14
#define OPT_show_flavors			15

#define OPT_disturb_move			20
#define OPT_disturb_near			21
#define OPT_disturb_detect			22
#define OPT_disturb_state			23
#define OPT_disturb_minor			24

#define OPT_view_perma_grids		38
#define OPT_view_torch_grids		39

#define OPT_flush_failure			52
#define OPT_flush_disturb			53

#define OPT_highlight_player			59
#define OPT_view_yellow_light		60
#define OPT_view_bright_light		61
#define OPT_view_granite_light		62
#define OPT_view_special_light		63
#define OPT_easy_open 				64
#define OPT_easy_alter 				65
#define OPT_animate_flicker         66
#define OPT_show_piles				67
#define OPT_center_player			68
#define OPT_purple_uniques			69
#define OPT_xchars_to_file			70
#define OPT_auto_more			71
#define OPT_hp_changes_color		74
#define OPT_hide_squelchable		75
#define OPT_squelch_worthless		76
#define OPT_mouse_movement		77
#define OPT_mouse_buttons		78
#define OPT_notify_recharge			79


#define OPT_birth_maximize          (OPT_BIRTH+0)
#define OPT_birth_randarts          (OPT_BIRTH+1)
/* #define OPT_birth_money             (OPT_BIRTH+2) */
#define OPT_birth_ironman           (OPT_BIRTH+3)
#define OPT_birth_no_stores         (OPT_BIRTH+4)
#define OPT_birth_no_artifacts      (OPT_BIRTH+5)
#define OPT_birth_no_stacking       (OPT_BIRTH+6)
#define OPT_birth_no_preserve       (OPT_BIRTH+7)
#define OPT_birth_no_stairs			(OPT_BIRTH+8)
#define OPT_birth_feelings				(OPT_BIRTH+9)
/* leave four spaces for future */
#define OPT_birth_ai_sound			(OPT_BIRTH+13)
#define OPT_birth_ai_smell			(OPT_BIRTH+14)
#define OPT_birth_ai_packs			(OPT_BIRTH+15)
#define OPT_birth_ai_learn			(OPT_BIRTH+16)
#define OPT_birth_ai_cheat			(OPT_BIRTH+17)
#define OPT_birth_ai_smart			(OPT_BIRTH+18)

#define OPT_cheat_peek				(OPT_CHEAT+0)
#define OPT_cheat_hear				(OPT_CHEAT+1)
#define OPT_cheat_room				(OPT_CHEAT+2)
#define OPT_cheat_xtra				(OPT_CHEAT+3)
#define OPT_cheat_know				(OPT_CHEAT+4)
#define OPT_cheat_live				(OPT_CHEAT+5)

#define OPT_adult_maximize          (OPT_ADULT+0)
#define OPT_adult_randarts          (OPT_ADULT+1)
#define OPT_adult_money             (OPT_ADULT+2)
#define OPT_adult_ironman           (OPT_ADULT+3)
#define OPT_adult_no_stores         (OPT_ADULT+4)
#define OPT_adult_no_artifacts      (OPT_ADULT+5)
#define OPT_adult_no_stacking       (OPT_ADULT+6)
#define OPT_adult_no_preserve       (OPT_ADULT+7)
#define OPT_adult_no_stairs			(OPT_ADULT+8)
#define OPT_adult_feelings				(OPT_ADULT+9)
/* leave four spaces for future */
#define OPT_adult_ai_sound			(OPT_ADULT+13)
#define OPT_adult_ai_smell			(OPT_ADULT+14)
#define OPT_adult_ai_packs			(OPT_ADULT+15)
#define OPT_adult_ai_learn			(OPT_ADULT+16)
#define OPT_adult_ai_cheat			(OPT_ADULT+17)
#define OPT_adult_ai_smart			(OPT_ADULT+18)

#define OPT_score_peek				(OPT_SCORE+0)
#define OPT_score_hear				(OPT_SCORE+1)
#define OPT_score_room				(OPT_SCORE+2)
#define OPT_score_xtra				(OPT_SCORE+3)
#define OPT_score_know				(OPT_SCORE+4)
#define OPT_score_live				(OPT_SCORE+5)


#define OPT(opt_name)	op_ptr->opt[OPT_##opt_name]

#endif /* !INCLUDED_OPTIONS_H */
