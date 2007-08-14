
#ifndef INCLUDED_UI_EVENT_H
#define INCLUDED_UI_EVENT_H

/* The various events we can send signals about. */
typedef enum ui_event_type
{
	ui_MAP_CHANGED,		/* Some part of the map has changed. */

	ui_STATS_CHANGED,  	/* One or more of the stats. */
	ui_HP_CHANGED,	   	/* HP or MaxHP. */
	ui_MANA_CHANGED,	/* Mana or MaxMana. */
	ui_AC_CHANGED,		/* Armour Class. */
	ui_EXPERIENCE_CHANGED,	/* Experience or MaxExperience. */
	ui_LEVEL_CHANGED,	/* Player's level has changed */
	ui_TITLE_CHANGED,	/* Player's title has changed */
	ui_GOLD_CHANGED,	/* Player's gold amount. */
	ui_HEALTH_CHANGED,	/* Observed monster's health level. */
	ui_DEPTH_CHANGED,	/* Dungeon depth */
	ui_SPEED_CHANGED,	/* Player's speed */
	ui_RACE_CLASS_CHANGED,	/* Race or Class */
	ui_STUDY_CHANGED,	/* "Study" availability */
	ui_STATUS_CHANGED,	/* Status */
	ui_DETECT_TRAPS_CHANGED,/* Trap detection status */
	ui_STATE_CHANGED,	/* The three 'R's: Resting, Repeating and
				   Searching */

	ui_PLAYER_MOVED,

	ui_INVENTORY_CHANGED,
	ui_EQUIPMENT_CHANGED,
	ui_MONSTERLIST_CHANGED,
	ui_MONSTER_TARGET_CHANGED,
	ui_MESSAGES_CHANGED,

	ui_event_REDRAW		/* It's the end of a "set" of events, so safe to update */
} ui_event_type;

#define  N_UI_EVENTS ui_event_REDRAW

typedef union
{
	struct 
	{
		int x;
		int y;
	} point;

} ui_event_data;

typedef void ui_event_handler(ui_event_type type, ui_event_data *data, void *user);

void ui_event_register(ui_event_type type, ui_event_handler *fn, void *user);
void ui_event_deregister(ui_event_type type, ui_event_handler *fn, void *user);
void ui_event_register_set(ui_event_type *type, size_t n_types, ui_event_handler *fn, void *user);
void ui_event_deregister_set(ui_event_type *type, size_t n_types, ui_event_handler *fn, void *user);

void ui_event_signal_point(ui_event_type, int x, int y);
void ui_event_signal(ui_event_type);

#endif /* INCLUDED_UI_EVENT_H */
