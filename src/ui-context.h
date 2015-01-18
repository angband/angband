#ifndef UI_CONTEXT_H
#define UI_CONTEXT_H

#include "cave.h"
#include "ui-input.h"

int context_menu_player(int mx, int my);
int context_menu_cave(struct chunk *c, int y, int x, int adjacent, int mx, int my);
int context_menu_object(struct object *obj);
int context_menu_command(int mx, int my);
void textui_process_click(ui_event e);
struct cmd_info *textui_action_menu_choose(void);

#endif /* UI_CONTEXT_H */
