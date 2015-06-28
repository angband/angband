#ifndef INCLUDED_UI_OPTIONS_H
#define INCLUDED_UI_OPTIONS_H

extern void do_cmd_options_birth(void);
extern bool squelch_tval(int tval);
extern void do_cmd_options_item(const char *title, int row);
extern void cleanup_options(void);

/* already in cmds.h
 * extern void do_cmd_options(void); */

#endif
