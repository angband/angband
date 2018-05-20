/**
 * \file cmd-core.h
 * \brief Handles the queueing of game commands.
 *
 * Copyright (c) 2008-9 Antony Sidwell
 * Copyright (c) 2014 Andi Sidwell
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#ifndef INCLUDED_CMD_CORE_H
#define INCLUDED_CMD_CORE_H

#include "object.h"
#include "z-type.h"

/**
 * All valid game commands.  Not all implemented yet.
 */
typedef enum cmd_code {
	CMD_NULL = 0,	/* A "do nothing" command so that there's something
					   UIs can use as a "no command yet" sentinel. */
	/* 
	 * Splash screen commands 
	 */
	CMD_LOADFILE,
	CMD_NEWGAME,

	/* 
	 * Birth commands 
	 */
	CMD_BIRTH_INIT,
	CMD_BIRTH_RESET,
	CMD_CHOOSE_RACE,
	CMD_CHOOSE_CLASS,
	CMD_BUY_STAT,
	CMD_SELL_STAT,
	CMD_RESET_STATS,
	CMD_ROLL_STATS,
	CMD_PREV_STATS,
	CMD_NAME_CHOICE,
	CMD_HISTORY_CHOICE,
	CMD_ACCEPT_CHARACTER,

	/* 
	 * The main game commands
	 */
	CMD_GO_UP,
	CMD_GO_DOWN,
	CMD_WALK,
	CMD_JUMP,
	CMD_PATHFIND,

	CMD_INSCRIBE,
	CMD_UNINSCRIBE,
	CMD_AUTOINSCRIBE,
	CMD_TAKEOFF,
	CMD_WIELD,
	CMD_DROP,
	CMD_BROWSE_SPELL,
	CMD_STUDY,
	CMD_CAST, /* Casting a spell /or/ praying. */
	CMD_USE_STAFF,
	CMD_USE_WAND,
	CMD_USE_ROD,
	CMD_ACTIVATE,
	CMD_EAT,
	CMD_QUAFF,
	CMD_READ_SCROLL,
	CMD_REFILL,
	CMD_USE,
	CMD_FIRE,
	CMD_THROW,
	CMD_PICKUP,
	CMD_AUTOPICKUP,
	CMD_IGNORE,
	CMD_DISARM,
	CMD_REST,
	CMD_TUNNEL,
	CMD_OPEN,
	CMD_CLOSE,
	CMD_RUN,
	CMD_HOLD,
	CMD_ALTER,
	CMD_SLEEP,

    /* Store commands */	
	CMD_SELL,
	CMD_BUY,
	CMD_STASH,
	CMD_RETRIEVE,

	/* Hors categorie Commands */
	CMD_SUICIDE,

	CMD_HELP,
	CMD_REPEAT,
	CMD_COMMAND_MONSTER
} cmd_code;

typedef enum cmd_context {
	CMD_INIT,
	CMD_BIRTH,
	CMD_GAME,
	CMD_STORE,
	CMD_DEATH
} cmd_context;

enum {
	DIR_UNKNOWN = 0,
	DIR_NW = 7,
	DIR_N = 8,
	DIR_NE = 9,
	DIR_W = 4,
	DIR_TARGET = 5,
	DIR_NONE = 5,
	DIR_E = 6,
	DIR_SW = 1,
	DIR_S = 2,
	DIR_SE = 3,
};

/**
 * ------------------------------------------------------------------------
 * Argument structures
 * ------------------------------------------------------------------------ */

/**
 * The data of the argument
 */
union cmd_arg_data {
	const char *string;
	
	int choice;
	struct object *obj;
	int number;
	int direction;
	
	struct loc point;
};

/**
 * The type of the data
 */
enum cmd_arg_type {
	arg_NONE = 0,
	arg_STRING = 1,
	arg_CHOICE,
	arg_ITEM,
	arg_NUMBER,
	arg_DIRECTION,
	arg_TARGET,
	arg_POINT
};

/**
 * A single argument
 */
struct cmd_arg {
	enum cmd_arg_type type;
	union cmd_arg_data data;
	char name[20];		/* Better than dynamic allocation */
};


/**
 * Maximum number of arguments a command needs to take.
 */
#define CMD_MAX_ARGS 4



/**
 * The struct command type is used to return details of the command the
 * game should carry out.
 *
 * 'command' should always have a valid cmd_code value, the other entries
 * may or may not be significant depending on the command being returned.
 */
struct command {
	/* What context this is happening in */
	cmd_context context;

	/* A valid command code. */
	cmd_code code;

	/* Number of times to attempt to repeat command. */
	int nrepeats;

	/* Arguments */
	struct cmd_arg arg[CMD_MAX_ARGS];
};


/**
 * Return codes for cmd_get_arg()
 */
enum cmd_return_codes {
	CMD_OK = 0,
	CMD_ARG_NOT_PRESENT = -1,
	CMD_ARG_WRONG_TYPE = -2,
	CMD_ARG_ABORTED = -3
};


/**
 * Command handlers will take a pointer to the command structure
 * so that they can access any arguments supplied.
 */
typedef void (*cmd_handler_fn)(struct command *cmd);


/**
 * ------------------------------------------------------------------------
 * Command type functions
 * ------------------------------------------------------------------------ */

/* Return the verb that goes alongside the given command. */
const char *cmd_verb(cmd_code cmd);


/**
 * ------------------------------------------------------------------------
 * Command queue functions
 * ------------------------------------------------------------------------ */

/**
 * Returns the top command on the queue.
 */
struct command *cmdq_peek(void);

/**
 * A function called by the game to get a command from the UI.
 */
extern errr (*cmd_get_hook)(cmd_context c);

/**
 * Gets the next command from the queue and processes it
 */
bool cmdq_pop(cmd_context c);

/**
 * Insert commands in the queue.
 */
errr cmdq_push_copy(struct command *cmd);
errr cmdq_push_repeat(cmd_code c, int nrepeats);
errr cmdq_push(cmd_code c);


/**
 * Process all commands presently in the queue.
 */
void cmdq_execute(cmd_context ctx);

/**
 * ------------------------------------------------------------------------
 * Command repeat manipulation
 * ------------------------------------------------------------------------ */

/**
 * Remove any pending repeats from the current command.
 */
void cmd_cancel_repeat(void);

/**
 * Update the number of repeats pending for the current command.
 */
void cmd_set_repeat(int nrepeats);

/**
 * Call to disallow the current command from being repeated with the
 * "Repeat last command" command.
 */
void cmd_disable_repeat(void);

/**
 * Returns the number of repeats left for the current command.
 * i.e. zero if not repeating.
 */
int cmd_get_nrepeats(void);



/**
 * ------------------------------------------------------------------------
 * Command type functions
 * ------------------------------------------------------------------------ */

/**
 * Set the args of a command.
 */
void cmd_set_arg_choice(struct command *cmd, const char *arg, int choice);
void cmd_set_arg_string(struct command *cmd, const char *arg, const char *str);
void cmd_set_arg_direction(struct command *cmd, const char *arg, int dir);
void cmd_set_arg_target(struct command *cmd, const char *arg, int target);
void cmd_set_arg_point(struct command *cmd, const char *arg, int x, int y);
void cmd_set_arg_item(struct command *cmd, const char *arg, struct object *obj);
void cmd_set_arg_number(struct command *cmd, const char *arg, int amt);


/**
 * Get the args of a command.
 */
int cmd_get_arg_choice(struct command *cmd, const char *arg, int *choice);
int cmd_get_arg_string(struct command *cmd, const char *arg, const char **str);
int cmd_get_arg_direction(struct command *cmd, const char *arg, int *dir);
int cmd_get_arg_target(struct command *cmd, const char *arg, int *target);
int cmd_get_arg_point(struct command *cmd, const char *arg, int *x, int *y);
int cmd_get_arg_item(struct command *cmd, const char *arg, struct object **obj);
int cmd_get_arg_number(struct command *cmd, const char *arg, int *amt);

/**
 * Try a bit harder.
 */
int cmd_get_direction(struct command *cmd, const char *arg, int *dir,
					  bool allow_5);
int cmd_get_target(struct command *cmd, const char *arg, int *target);
int cmd_get_item(struct command *cmd, const char *arg, struct object **obj,
				 const char *prompt, const char *reject, item_tester filter,
				 int mode);
int cmd_get_quantity(struct command *cmd, const char *arg, int *amt, int max);
int cmd_get_string(struct command *cmd, const char *arg, const char **str,
				   const char *initial, const char *title, const char *prompt);
int cmd_get_spell(struct command *cmd, const char *arg, int *spell,
				  const char *verb, item_tester book_filter, const char *error,
				  bool (*spell_filter)(int spell));

#endif
