#ifndef INCLUDED_UI_INPUT_H
#define INCLUDED_UI_INPUT_H

#include "z-term.h"
#include "game-event.h"
#include "cmd-core.h"
#include "ui-event.h"

/**
 * Holds a generic command - if cmd is set to other than CMD_NULL 
 * it simply pushes that command to the game, otherwise the hook 
 * function will be called.
 */
struct cmd_info
{
	const char *desc;
	keycode_t key[2];
	cmd_code cmd;
	void (*hook)(void);
	bool (*prereq)(void);
};

/**
 * A categorised list of all the command lists.
 */
struct command_list
{
	const char *name;
	struct cmd_info *list;
	size_t len;
};

#define SCAN_INSTANT ((u32b) -1)
#define SCAN_OFF 0

extern struct cmd_info cmd_item[];
extern struct cmd_info cmd_action[];
extern struct cmd_info cmd_item_manage[];
extern struct cmd_info cmd_info[];
extern struct cmd_info cmd_util[];
extern struct cmd_info cmd_hidden[];
extern struct command_list cmds_all[];

extern struct keypress *inkey_next;
extern u32b inkey_scan;
extern bool inkey_flag;
extern u16b lazymove_delay;
extern bool msg_flag;

void flush(game_event_type unused, game_event_data *data, void *user);
ui_event inkey_ex(void);
void anykey(void);
struct keypress inkey(void);
ui_event inkey_m(void);
void display_message(game_event_type unused, game_event_data *data, void *user);
void bell_message(game_event_type unused, game_event_data *data, void *user);
void message_flush(game_event_type unused, game_event_data *data, void *user);
void clear_from(int row);
bool askfor_aux_keypress(char *buf, size_t buflen, size_t *curs, size_t *len, struct keypress keypress, bool firsttime);
bool askfor_aux(char *buf, size_t len, bool (*keypress_h)(char *, size_t, size_t *, size_t *, struct keypress, bool));
bool get_name(char *buf, size_t buflen);
char get_char(const char *prompt, const char *options, size_t len, char fallback);
bool (*get_file)(const char *suggested_name, char *path, size_t len);
bool get_com_ex(const char *prompt, ui_event *command);
void pause_line(struct term *term);
void textui_input_init(void);
void cmd_init(void);
unsigned char cmd_lookup_key(cmd_code lookup_cmd, int mode);
unsigned char cmd_lookup_key_unktrl(cmd_code lookup_cmd, int mode);
cmd_code cmd_lookup(unsigned char key, int mode);
ui_event textui_get_command(int *count);
bool key_confirm_command(unsigned char c);
bool textui_process_key(struct keypress kp, struct cmd_info **cmd,
						int count);

#endif /* INCLUDED_UI_INPUT_H */
