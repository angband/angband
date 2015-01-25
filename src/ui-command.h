#ifndef UI_COMMAND_H
#define UI_COMMAND_H

void do_cmd_redraw(void);
void do_cmd_xxx_options(void);
void do_cmd_unknown(void);
void textui_cmd_suicide(void);
void textui_cmd_debug(void);
void do_cmd_load_screen(void);
void html_screenshot(const char *path, int mode);
void do_cmd_save_screen(void);
void textui_cmd_rest(void);
void textui_quit(void);

#endif /* UI_COMMAND_H */
