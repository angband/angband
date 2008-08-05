#ifndef INCLUDED_OPTIONS_H
#define INCLUDED_OPTIONS_H


const char *option_name(int opt);
const char *option_desc(int opt);

void option_set(int opt, bool on);
void option_set_defaults(void);


/*** Option display definitions ***/

/*
 * Information for "do_cmd_options()".
 */
#define OPT_PAGE_MAX				5
#define OPT_PAGE_PER				16

/* The option data structures */
extern const byte option_page[OPT_PAGE_MAX][OPT_PAGE_PER];



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

#define OPT_hilite_player			59
#define OPT_view_yellow_lite		60
#define OPT_view_bright_lite		61
#define OPT_view_granite_lite		62
#define OPT_view_special_lite		63
#define OPT_easy_open 				64
#define OPT_easy_alter 				65
#define OPT_show_piles				67
#define OPT_center_player			68
#define OPT_auto_more			71
#define OPT_hp_changes_color		74
#define OPT_hide_squelchable		75
#define OPT_squelch_worthless		76
#define OPT_mouse_movement		77
#define OPT_mouse_buttons		78


#define OPT_birth_maximize          (OPT_BIRTH+0)
#define OPT_birth_randarts          (OPT_BIRTH+1)
#define OPT_birth_money             (OPT_BIRTH+2)
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


/*
 * Hack -- Option symbols
 *
 * These shouldn't even be here.
 */
#define OPT(opt_name)	op_ptr->opt[OPT_##opt_name]

#define rogue_like_commands		OPT(rogue_like_commands)
#define quick_messages			OPT(quick_messages)
#define use_sound				OPT(use_sound)
#define pickup_detail			OPT(pickup_detail)
#define use_old_target			OPT(use_old_target)
#define pickup_always			OPT(pickup_always)
#define pickup_inven			OPT(pickup_inven)
#define show_labels				OPT(show_labels)
#define ring_bell				OPT(ring_bell)
#define show_flavors			OPT(show_flavors)
#define run_ignore_doors		OPT(run_ignore_doors)
#define disturb_move			OPT(disturb_move)
#define disturb_near			OPT(disturb_near)
#define disturb_detect			OPT(disturb_detect)
#define disturb_state			OPT(disturb_state)
#define disturb_minor			OPT(disturb_minor)
#define view_perma_grids		OPT(view_perma_grids)
#define view_torch_grids		OPT(view_torch_grids)
#define flush_failure			OPT(flush_failure)
#define flush_disturb			OPT(flush_disturb)
#define hilite_player			OPT(hilite_player)
#define view_yellow_lite		OPT(view_yellow_lite)
#define view_bright_lite		OPT(view_bright_lite)
#define view_granite_lite		OPT(view_granite_lite)
#define view_special_lite		OPT(view_special_lite)
#define easy_open				OPT(easy_open)
#define easy_alter				OPT(easy_alter)
#define show_piles				OPT(show_piles)
#define center_player			OPT(center_player)
#define auto_more				OPT(auto_more)
#define hp_changes_color		OPT(hp_changes_color)
#define hide_squelchable		OPT(hide_squelchable)
#define mouse_movement			OPT(mouse_movement)
#define mouse_buttons			OPT(mouse_buttons)

#define birth_maximize			OPT(birth_maximize)
#define birth_randarts			OPT(birth_randarts)
#define birth_ironman			OPT(birth_ironman)
#define birth_no_stores			OPT(birth_no_stores)
#define birth_no_artifacts		OPT(birth_no_artifacts)
#define birth_no_stacking       OPT(birth_no_stacking)
#define birth_no_preserve       OPT(birth_no_preserve)
#define birth_no_stairs			OPT(birth_no_stairs)
#define birth_ai_sound			OPT(birth_ai_sound)
#define birth_ai_smell			OPT(birth_ai_smell)
#define birth_ai_packs			OPT(birth_ai_packs)
#define birth_ai_learn			OPT(birth_ai_learn)
#define birth_ai_cheat			OPT(birth_ai_cheat)
#define birth_ai_smart			OPT(birth_ai_smart)

#define cheat_peek				OPT(cheat_peek)
#define cheat_hear				OPT(cheat_hear)
#define cheat_room				OPT(cheat_room)
#define cheat_xtra				OPT(cheat_xtra)
#define cheat_know				OPT(cheat_know)
#define cheat_live				OPT(cheat_live)

#define adult_maximize			OPT(adult_maximize)
#define adult_randarts			OPT(adult_randarts)
#define adult_ironman			OPT(adult_ironman)
#define adult_no_stores			OPT(adult_no_stores)
#define adult_no_artifacts		OPT(adult_no_artifacts)
#define adult_no_stacking		OPT(adult_no_stacking)
#define adult_no_preserve		OPT(adult_no_preserve)
#define adult_no_stairs			OPT(adult_no_stairs)
#define adult_ai_sound			OPT(adult_ai_sound)
#define adult_ai_smell			OPT(adult_ai_smell)
#define adult_ai_packs			OPT(adult_ai_packs)
#define adult_ai_learn			OPT(adult_ai_learn)
#define adult_ai_cheat			OPT(adult_ai_cheat)
#define adult_ai_smart			OPT(adult_ai_smart)

#define score_peek				OPT(score_peek)
#define score_hear				OPT(score_hear)
#define score_room				OPT(score_room)
#define score_xtra				OPT(score_xtra)
#define score_know				OPT(score_know)
#define score_live				OPT(score_live)

#endif /* !INCLUDED_OPTIONS_H */
