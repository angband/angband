/**
 * \file game-event.h
 * \brief Allows the registering of handlers to be told about game events.
 *
 * Copyright (c) 2007 Antony Sidwell
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

#ifndef INCLUDED_GAME_EVENT_H
#define INCLUDED_GAME_EVENT_H

#include "z-type.h"

/**
 * The various events we can send signals about.
 */
typedef enum game_event_type
{
	EVENT_MAP = 0,		/* Some part of the map has changed. */

	EVENT_STATS,  		/* One or more of the stats. */
	EVENT_HP,	   	/* HP or MaxHP. */
	EVENT_MANA,		/* Mana or MaxMana. */
	EVENT_AC,		/* Armour Class. */
	EVENT_EXPERIENCE,	/* Experience or MaxExperience. */
	EVENT_PLAYERLEVEL,	/* Player's level has changed */
	EVENT_PLAYERTITLE,	/* Player's title has changed */
	EVENT_GOLD,		/* Player's gold amount. */
	EVENT_MONSTERHEALTH,	/* Observed monster's health level. */
	EVENT_DUNGEONLEVEL,	/* Dungeon depth */
	EVENT_PLAYERSPEED,	/* Player's speed */
	EVENT_RACE_CLASS,	/* Race or Class */
	EVENT_STUDYSTATUS,	/* "Study" availability */
	EVENT_STATUS,		/* Status */
	EVENT_DETECTIONSTATUS,	/* Trap detection status */
	EVENT_FEELING,		/* Object level feeling */
	EVENT_STATE,		/* The three 'R's: Resting, Repeating and
				   Searching */

	EVENT_PLAYERMOVED,
	EVENT_SEEFLOOR,         /* When the player would "see" floor objects */
	EVENT_EXPLOSION,
	EVENT_BOLT,
	EVENT_MISSILE,

	EVENT_INVENTORY,
	EVENT_EQUIPMENT,
	EVENT_ITEMLIST,
	EVENT_MONSTERLIST,
	EVENT_MONSTERTARGET,
	EVENT_OBJECTTARGET,
	EVENT_MESSAGE,
	EVENT_SOUND,
	EVENT_BELL,
	EVENT_USE_STORE,
	EVENT_STORECHANGED,	/* Triggered on a successful buy/retrieve or sell/drop */

	EVENT_INPUT_FLUSH,
	EVENT_MESSAGE_FLUSH,
	EVENT_CHECK_INTERRUPT,
	EVENT_REFRESH,
	EVENT_NEW_LEVEL_DISPLAY,
	EVENT_COMMAND_REPEAT,
	EVENT_ANIMATE,
	EVENT_CHEAT_DEATH,

	EVENT_INITSTATUS,	/* New status message for initialisation */
	EVENT_BIRTHPOINTS,	/* Change in the birth points */

	/* Changing of the game state/context. */
	EVENT_ENTER_INIT,
	EVENT_LEAVE_INIT,
	EVENT_ENTER_BIRTH,
	EVENT_LEAVE_BIRTH,
	EVENT_ENTER_GAME,
	EVENT_LEAVE_GAME,
	EVENT_ENTER_WORLD,
	EVENT_LEAVE_WORLD,
	EVENT_ENTER_STORE,
	EVENT_LEAVE_STORE,
	EVENT_ENTER_DEATH,
	EVENT_LEAVE_DEATH,

	EVENT_END  /* Can be sent at the end of a series of events */
} game_event_type;

#define  N_GAME_EVENTS EVENT_END + 1

typedef union
{
	struct loc point;

	const char *string;

	bool flag;

	struct {
		const char *msg;
		int type;
	} message;

	struct
	{
		bool reset;
		const char *hint;
		int n_choices;
		int initial_choice;
		const char **choices;
		const char **helptexts;
		void *xtra;
	} birthstage;

  	struct
	{
		int *stats;
		int remaining;
	} birthstats;

	struct
	{
		int gf_type;
		int num_grids;
		int *distance_to_grid;
		bool drawing;
		bool *player_sees_grid;
		struct loc *blast_grid;
		struct loc centre;
	} explosion;

	struct
	{
		int gf_type;
		bool drawing;
		bool seen;
		bool beam;
		int oy;
		int ox;
		int y;
		int x;
	} bolt;

	struct
	{
		struct object *obj;
		bool seen;
		int y;
		int x;
	} missile;

} game_event_data;


/**
 * A function called when a game event occurs - these are registered to be
 * called by event_add_handler or event_add_handler_set, and deregistered
 * when they should no longer be called through event_remove_handler or
 * event_remove_handler_set.
 */
typedef void game_event_handler(game_event_type type, game_event_data *data, void *user);

void event_add_handler(game_event_type type, game_event_handler *fn, void *user);
void event_remove_handler(game_event_type type, game_event_handler *fn, void *user);
void event_remove_handler_type(game_event_type type);
void event_remove_all_handlers(void);
void event_add_handler_set(game_event_type *type, size_t n_types, game_event_handler *fn, void *user);
void event_remove_handler_set(game_event_type *type, size_t n_types, game_event_handler *fn, void *user);

void event_signal_birthpoints(int stats[6], int remaining);

void event_signal_point(game_event_type, int x, int y);
void event_signal_string(game_event_type, const char *s);
void event_signal_message(game_event_type type, int t, const char *s);
void event_signal_flag(game_event_type type, bool flag);
void event_signal(game_event_type);
void event_signal_blast(game_event_type type,
						int gf_type,
						int num_grids,
						int *distance_to_grid,
						bool seen,
						bool *player_sees_grid,
						struct loc *blast_grid,
						struct loc centre);
void event_signal_bolt(game_event_type type,
					   int gf_type,
					   bool drawing,
					   bool seen,
					   bool beam,
					   int oy,
					   int ox,
					   int y,
					   int x);
void event_signal_missile(game_event_type type,
						  struct object *obj,
						  bool seen,
						  int y,
						  int x);

#endif /* INCLUDED_GAME_EVENT_H */
