/* prefs.h - prefs interface */

#ifndef PREFS_H
#define PREFS_H

void autoinsc_dump(ang_file *fff);
void squelch_dump(ang_file *fff);
void option_dump(ang_file *fff);
void macro_dump(ang_file *fff);
void keymap_dump(ang_file *fff);
void dump_monsters(ang_file *fff);
void dump_objects(ang_file *fff);
void dump_features(ang_file *fff);
void dump_flavors(ang_file *fff);
void dump_colors(ang_file *fff);
bool prefs_save(const char *path, void (*dump)(ang_file *), const char *title);
s16b tokenize(char *buf, s16b num, char **tokens);
errr process_pref_file_command(char *buf);
errr process_pref_file(cptr name);

#endif /* !PREFS_H */
