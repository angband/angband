/* button.h - button interface */

#ifndef BUTTON_H
#define BUTTON_H

int button_add_text(const char *label, keycode_t keypress);
int button_add(const char *label, keycode_t keypress);
void button_backup_all(void);
void button_restore(void);
int button_kill_text(keycode_t keypress);
int button_kill(keycode_t keypress);
void button_kill_all(void);
void button_init(button_add_f add, button_kill_f kill);
void button_hook(button_add_f add, button_kill_f kill);
void button_free(void);
char button_get_key(int x, int y);
size_t button_print(int row, int col);

#endif /* !BUTTON_H */
