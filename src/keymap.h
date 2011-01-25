#ifndef KEYMAP_H
#define KEYMAP_H

const struct keypress *keymap_find(int keymap, struct keypress kc);
void keymap_add(int keymap, struct keypress trigger, struct keypress *actions);
bool keymap_remove(int keymap, struct keypress trigger);
void keymap_free(void);
void keymap_dump(ang_file *fff);

#endif /* KEYMAP_H */
