
#ifndef INCLUDED_GAME_EVENT_H
#define INCLUDED_GAME_EVENT_H

enum birth_stage
{
	BIRTH_METHOD_CHOICE = 0,
	BIRTH_SEX_CHOICE,
	BIRTH_RACE_CHOICE,
	BIRTH_CLASS_CHOICE,
	BIRTH_ROLLER_CHOICE,
	BIRTH_POINTBASED,
	BIRTH_AUTOROLLER,
	BIRTH_ROLLER,
	BIRTH_NAME_CHOICE,
	BIRTH_FINAL_CONFIRM,
	BIRTH_COMPLETE
};

/* The various events we can send signals about. */
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
	EVENT_STATE,		/* The three 'R's: Resting, Repeating and
				   Searching */
	EVENT_MOUSEBUTTONS,     /* Displayed mouse buttons need changing */

	EVENT_PLAYERMOVED,

	EVENT_INVENTORY,
	EVENT_EQUIPMENT,
	EVENT_MONSTERLIST,
	EVENT_MONSTERTARGET,
	EVENT_MESSAGE,

	EVENT_INITSTATUS,	/* New status message for initialisation */

	EVENT_BIRTHSTAGE,	/* Change the point we're up to in birth */
	EVENT_BIRTHSTATS,	/* Change in the birth points */
	EVENT_BIRTHAUTOROLLER,  /* Autoroller updates. */

	/* Changing of the game state/context. */
	EVENT_ENTER_INIT,
	EVENT_LEAVE_INIT,
	EVENT_ENTER_BIRTH,
	EVENT_LEAVE_BIRTH,
	EVENT_ENTER_GAME,
	EVENT_LEAVE_GAME,
	EVENT_ENTER_STORE,
	EVENT_LEAVE_STORE,
	EVENT_ENTER_DEATH,
	EVENT_LEAVE_DEATH,

	EVENT_END  /* Can be sent at the end of a series of events */
} game_event_type;

#define  N_GAME_EVENTS EVENT_END + 1

typedef union
{
	struct 
	{
		int x;
		int y;
	} point;
		
	const char *string;

	struct
	{
		enum birth_stage stage;
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
		int *limits;
		int *matches;
		int *current;
		unsigned long round;
	} birthautoroll;

} game_event_data;


/* 
 * A function called when a game event occurs - these are registered to be
 * called by event_add_handler or event_add_handler_set, and deregistered
 * when they should no longer be called through event_remove_handler or
 * event_remove_handler_set.
 */
typedef void game_event_handler(game_event_type type, game_event_data *data, void *user);

void event_add_handler(game_event_type type, game_event_handler *fn, void *user);
void event_remove_handler(game_event_type type, game_event_handler *fn, void *user);
void event_add_handler_set(game_event_type *type, size_t n_types, game_event_handler *fn, void *user);
void event_remove_handler_set(game_event_type *type, size_t n_types, game_event_handler *fn, void *user);

void event_signal_birthstage_question(enum birth_stage stage, const char *hint, int n_choices, int initial_choice, const char *choices[], const char *helptexts[]);
void event_signal_birthstage(enum birth_stage stage, void *xtra);
void event_signal_birthstats(int stats[6], int remaining);
void event_signal_birthautoroller(int limits[6], int matches[6], int current[6], unsigned long round);

void event_signal_point(game_event_type, int x, int y);
void event_signal_string(game_event_type, const char *s);
void event_signal(game_event_type);

#endif /* INCLUDED_GAME_EVENT_H */
