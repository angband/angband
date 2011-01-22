#ifndef KEYMAP_H
#define KEYMAP_H

const char *keymap_find(int keymap, keycode_t kc);
void keymap_add(int keymap, keycode_t trigger, char *actions);
bool keymap_remove(int keymap, keycode_t trigger);
void keymap_free(void);
void keymap_dump(ang_file *fff);

#endif /* KEYMAP_H */
