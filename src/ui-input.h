#ifndef INCLUDED_UI_INPUT_H
#define INCLUDED_UI_INPUT_H

#define SCAN_INSTANT ((u32b) -1)
#define SCAN_OFF 0

/* ui-input.c */
extern struct keypress *inkey_next;

extern void flush(void);
extern void flush_fail(void);
extern struct keypress inkey(void);
extern ui_event inkey_m(void);
extern ui_event inkey_ex(void);
extern void anykey(void);
extern void bell(const char *reason);
extern void sound(int val);
extern void msg(const char *fmt, ...);
extern void msgt(unsigned int type, const char *fmt, ...);
extern void message_flush(void);
extern void clear_from(int row);
extern bool askfor_aux_keypress(char *buf, size_t buflen, size_t *curs, size_t *len, struct keypress keypress, bool firsttime);
extern bool askfor_aux(char *buf, size_t len, bool (*keypress_h)(char *, size_t, size_t *, size_t *, struct keypress, bool));
extern bool get_string(const char *prompt, char *buf, size_t len);
extern s16b get_quantity(const char *prompt, int max);
extern char get_char(const char *prompt, const char *options, size_t len, char fallback);
extern bool get_check(const char *prompt);
extern bool (*get_file)(const char *suggested_name, char *path, size_t len);
extern bool get_com(const char *prompt, struct keypress *command);
extern bool get_com_ex(const char *prompt, ui_event *command);
extern void pause_line(struct term *term);



#endif /* INCLUDED_UI_INPUT_H */
