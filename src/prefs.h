/* prefs.h - prefs interface */

#ifndef PREFS_H
#define PREFS_H

void autoinsc_dump(ang_file *fff);
void squelch_dump(ang_file *fff);
void option_dump(ang_file *fff);
void dump_monsters(ang_file *fff);
void dump_objects(ang_file *fff);
void dump_features(ang_file *fff);
void dump_flavors(ang_file *fff);
void dump_colors(ang_file *fff);
bool prefs_save(const char *path, void (*dump)(ang_file *), const char *title);
errr process_pref_file_command(const char *buf);
bool process_pref_file(const char *name, bool quiet, bool user);

#endif /* !PREFS_H */
