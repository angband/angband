
#include "game-cmd.h"

/* From xtra3.c */

/* Ask the textui core for a game command. */
extern errr textui_get_cmd(cmd_context context, bool wait);

/* Set up game event handlers for the textui. */
void init_display(void);

/* From cmd0.c */

/* Get a command through the text UI */
void textui_process_command(bool no_request);
