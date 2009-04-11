
#include "angband.h"
#include "game-cmd.h"

/*
 * A function called by the game to get a command from the UI.
 * Just a hook, with the real function supplied by the UI.
 */
game_command (*get_game_command)(void);
