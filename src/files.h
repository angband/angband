/* files.h - game file interface */

#ifndef FILES_H
#define FILES_H

extern void html_screenshot(const char *name, int mode);
extern void player_flags(bitflag f[OF_SIZE]);
extern void display_player(int mode);
extern void display_player_stat_info(void);
extern void display_player_xtra_info(void);
extern errr file_character(const char *name, bool full);
extern bool show_file(const char *name, const char *what, int line, int mode);
extern void do_cmd_help(void);
extern void process_player_name(bool sf);
extern bool get_name(char *buf, size_t buflen);
extern void save_game(void);
extern void close_game(void);
extern void exit_game_panic(void);

#endif /* !FILES_H */
