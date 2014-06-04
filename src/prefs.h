/* prefs.h - prefs interface */

#ifndef PREFS_H
#define PREFS_H

extern int use_graphics;
extern bool arg_wizard;
extern bool arg_rebalance;
extern int arg_graphics;
extern bool arg_graphics_nice;

void autoinsc_dump(ang_file *fff);
void dump_options_and_keymaps(ang_file *fff);
void dump_monsters(ang_file *fff);
void dump_objects(ang_file *fff);
void dump_autoinscriptions(ang_file *f);
void dump_features(ang_file *fff);
void dump_flavors(ang_file *fff);
void dump_colors(ang_file *fff);
bool prefs_save(const char *path, void (*dump)(ang_file *), const char *title);
errr process_pref_file_command(const char *buf);
bool process_pref_file(const char *name, bool quiet, bool user);
void reset_visuals(bool load_prefs);

#endif /* !PREFS_H */
