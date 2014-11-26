#ifndef INCLUDED_UI_INPUT_H
#define INCLUDED_UI_INPUT_H

#include "angband.h"
#include "ui-event.h"
#include "cmd-core.h"

#define SCAN_INSTANT ((u32b) -1)
#define SCAN_OFF 0

/* ui-input.c */
extern struct keypress *inkey_next;
extern u32b inkey_scan;
extern bool inkey_flag;
extern u16b lazymove_delay;
extern bool msg_flag;

extern void flush(void);
extern void flush_fail(void);
extern struct keypress inkey(void);
extern ui_event inkey_m(void);
extern ui_event inkey_ex(void);
extern void anykey(void);
extern void bell(const char *reason);
extern void message_flush(void);
extern void display_message(game_event_type type, game_event_data *data, void *user);
extern void clear_from(int row);
extern bool askfor_aux_keypress(char *buf, size_t buflen, size_t *curs, size_t *len, struct keypress keypress, bool firsttime);
extern bool askfor_aux(char *buf, size_t len, bool (*keypress_h)(char *, size_t, size_t *, size_t *, struct keypress, bool));
extern bool get_name(char *buf, size_t buflen);
extern bool get_string(const char *prompt, char *buf, size_t len);
extern s16b get_quantity(const char *prompt, int max);
extern char get_char(const char *prompt, const char *options, size_t len, char fallback);
extern bool get_check(const char *prompt);
extern bool (*get_file)(const char *suggested_name, char *path, size_t len);
extern bool get_com(const char *prompt, struct keypress *command);
extern bool get_com_ex(const char *prompt, ui_event *command);
extern void pause_line(struct term *term);

/* ui-spell.c -- just for now */
int get_spell_from_book(const char *verb, struct object *book,
		const char *error, bool (*spell_filter)(int spell));
int get_spell(const char *verb, item_tester book_filter,
		cmd_code cmd, const char *error, bool (*spell_filter)(int spell));
void textui_book_browse(const object_type *o_ptr);
void textui_spell_browse(void);


#endif /* INCLUDED_UI_INPUT_H */
