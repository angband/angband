/**
 * \file ui-wizard.h
 * \brief Declarations for menus and ui-game.c shims related to debug commands
 */
#ifndef INCLUDED_UI_WIZARD_H
#define INCLUDED_UI_WIZARD_H

void wiz_acquire_good(void);
void wiz_acquire_great(void);
void wiz_confirm_quit_no_save(void);
void wiz_create_all_for_tval(void);
void wiz_create_artifact(void);
void wiz_create_item(bool art);
void wiz_create_nonartifact(void);
void wiz_learn_all_object_kinds(void);
void wiz_phase_door(void);
void wiz_proj_demo(void);
void wiz_teleport(void);

#endif /* INCLUDED_UI_WIZARD_H */
