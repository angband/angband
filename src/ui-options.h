#ifndef INCLUDED_UI_OPTIONS_H
#define INCLUDED_UI_OPTIONS_H

#include "obj-ignore.h"

void do_cmd_options_birth(void);
int ego_item_name(char *buf, size_t buf_size, ego_desc *desc);
bool ignore_tval(int tval);
void do_cmd_options_item(const char *title, int row);
void do_cmd_options(void);

#endif
