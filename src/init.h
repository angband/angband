/**
 * \file init.h
 * \brief initialization
 *
 * Copyright (c) 2000 Robert Ruehlmann
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

#ifndef INCLUDED_INIT_H
#define INCLUDED_INIT_H

#include "h-basic.h"
#include "z-bitflag.h"
#include "z-file.h"
#include "z-rand.h"
#include "datafile.h"
#include "object.h"

/**
 * Information about maximal indices of certain arrays.
 *
 * This will become a list of "all" the game constants - NRM
 */
struct angband_constants
{
	/* Array bounds etc, set on parsing edit files */
	u16b f_max;			/**< Maximum number of terrain features */
	u16b trap_max;		/**< Maximum number of trap kinds */
	u16b k_max;			/**< Maximum number of object base kinds */
	u16b a_max;			/**< Maximum number of artifact kinds */
	u16b e_max;			/**< Maximum number of ego-item kinds */
	u16b r_max;			/**< Maximum number of monster races */
	u16b mp_max;		/**< Maximum number of monster pain message sets */
	u16b s_max;			/**< Maximum number of magic spells */
	u16b pit_max;		/**< Maximum number of monster pit types */
	u16b act_max;		/**< Maximum number of activations for randarts */
	u16b curse_max;		/**< Maximum number of curses */
	u16b slay_max;		/**< Maximum number of slays */
	u16b brand_max;		/**< Maximum number of brands */
	u16b mon_blows_max;	/**< Maximum number of monster blows */
	u16b blow_methods_max;	/**< Maximum number of monster blow methods */
	u16b blow_effects_max;	/**< Maximum number of monster blow effects */
	u16b equip_slots_max;	/**< Maximum number of player equipment slots */
	u16b profile_max;	/**< Maximum number of cave_profiles */
	u16b quest_max;		/**< Maximum number of quests */
	u16b projection_max;	/**< Maximum number of projection types */
	u16b calculation_max;	/**< Maximum number of object power calculations */
	u16b property_max;	/**< Maximum number of object properties */
	u16b ordinary_kind_max;	/**< Maximum number of objects in object.txt */
	u16b shape_max;		/**< Maximum number of player shapes */

	/* Maxima of things on a given level, read from constants.txt */
	u16b level_monster_max;	/**< Maximum number of monsters on a given level */

	/* Monster generation constants, read from constants.txt */
	u16b alloc_monster_chance;	/**< 1/per-turn-chance of generation */
	u16b level_monster_min;		/**< Minimum number generated */
	u16b town_monsters_day;		/**< Townsfolk generated - day */
	u16b town_monsters_night;	/**< Townsfolk generated  - night */
	u16b repro_monster_max;		/**< Maximum breeders on a level */
	u16b ood_monster_chance;	/**< Chance of OoD monster is 1 in this */
	u16b ood_monster_amount;	/**< Max number of levels OoD */
	u16b monster_group_max;		/**< Maximum size of a group */
	u16b monster_group_dist;	/**< Max dist of a group from a related group */

	/* Monster gameplay constants, read from constants.txt */
	u16b glyph_hardness;		/**< How hard for a monster to break a glyph */
	u16b repro_monster_rate;	/**< Monster reproduction rate-slower */
	u16b life_drain_percent;	/**< Percent of player life drained */
	u16b flee_range;			/**< Monsters run this many grids out of view */
	u16b turn_range;			/**< Monsters turn to fight closer than this */

	/* Dungeon generation constants, read from constants.txt */
	u16b level_room_max;	/**< Maximum number of rooms on a level */
	u16b level_door_max;	/**< Maximum number of potential doors on a level */
	u16b wall_pierce_max;	/**< Maximum number of potential wall piercings */
	u16b tunn_grid_max;		/**< Maximum number of tunnel grids */
	u16b room_item_av;		/**< Average number of items in rooms */
	u16b both_item_av;		/**< Average number of items in random places */
	u16b both_gold_av;		/**< Average number of money items */
	u16b level_pit_max;		/**< Maximum number of pits on a level */

	/* World shape constants, read from constants.txt */
	u16b max_depth;		/* Maximum dungeon level */
	u16b day_length;	/* Number of turns from dawn to dawn */
	u16b dungeon_hgt;	/**< Maximum number of vertical grids on a level */
	u16b dungeon_wid;	/**< Maximum number of horizontical grids on a level */
	u16b town_hgt;	/**< Maximum number of vertical grids in the town */
	u16b town_wid;	/**< Maximum number of horizontical grids in the town */
	u16b feeling_total;	/* Total number of feeling squares per level */
	u16b feeling_need;	/* Squares needed to see to get first feeling */
    u16b stair_skip;    /* Number of levels to skip for each down stair */
	u16b move_energy;	/* Energy the player or monster needs to move */

	/* Carrying capacity constants, read from constants.txt */
	u16b pack_size;		/**< Maximum number of pack slots */
	u16b quiver_size;	/**< Maximum number of quiver slots */
	u16b quiver_slot_size;	/**< Maximum number of missiles per quiver slot */
	u16b floor_size;	/**< Maximum number of items per floor grid */

	/* Store parameters, read from constants.txt */
	u16b store_inven_max;	/**< Maximum number of objects in store inventory */
	u16b store_turns;		/**< Number of turns between turnovers */
	u16b store_shuffle;		/**< 1/per-day-chance of owner changing */
	u16b store_magic_level;	/**< Level for apply_magic() in normal stores */

	/* Object creation constants, read from constants.txt */
	u16b max_obj_depth;	/* Maximum depth used in object allocation */
	u16b great_obj;		/* 1/chance of inflating the requested object level */
	u16b great_ego;		/* 1/chance of inflating the requested ego item level */
	u16b fuel_torch;	/* Maximum amount of fuel in a torch */
	u16b fuel_lamp;		/* Maximum amount of fuel in a lantern */
	u16b default_lamp;	/* Default amount of fuel in a lantern  */

	/* Player constants, read from constants.txt */
	u16b max_sight;		/* Maximum visual range */
	u16b max_range;		/* Maximum missile and spell range */
	u16b start_gold;	/* Amount of gold the player starts with */
	u16b food_value;	/* Number of turns 1% of food lasts */
};

struct init_module {
	const char *name;
	void (*init)(void);
	void (*cleanup)(void);
};

extern bool play_again;

extern const char *list_element_names[];
extern const char *list_obj_flag_names[];

extern struct angband_constants *z_info;

extern const char *ANGBAND_SYS;

extern char *ANGBAND_DIR_GAMEDATA;
extern char *ANGBAND_DIR_CUSTOMIZE;
extern char *ANGBAND_DIR_HELP;
extern char *ANGBAND_DIR_SCREENS;
extern char *ANGBAND_DIR_FONTS;
extern char *ANGBAND_DIR_TILES;
extern char *ANGBAND_DIR_SOUNDS;
extern char *ANGBAND_DIR_ICONS;
extern char *ANGBAND_DIR_USER;
extern char *ANGBAND_DIR_SAVE;
extern char *ANGBAND_DIR_SCORES;
extern char *ANGBAND_DIR_INFO;
extern char *ANGBAND_DIR_ARCHIVE;

extern struct parser *init_parse_artifact(void);
extern struct parser *init_parse_class(void);
extern struct parser *init_parse_ego(void);
extern struct parser *init_parse_feat(void);
extern struct parser *init_parse_history(void);
extern struct parser *init_parse_object(void);
extern struct parser *init_parse_object_base(void);
extern struct parser *init_parse_pain(void);
extern struct parser *init_parse_p_race(void);
extern struct parser *init_parse_pit(void);
extern struct parser *init_parse_monster(void);
extern struct parser *init_parse_vault(void);
extern struct parser *init_parse_constants(void);
extern struct parser *init_parse_flavor(void);
extern struct parser *init_parse_names(void);
extern struct parser *init_parse_hints(void);
extern struct parser *init_parse_trap(void);
extern struct parser *init_parse_chest_trap(void);
extern struct parser *init_parse_quest(void);

extern struct file_parser flavor_parser;

errr grab_effect_data(struct parser *p, struct effect *effect);
extern void init_file_paths(const char *config, const char *lib, const char *data);
extern void init_game_constants(void);
extern void init_arrays(void);
extern void create_needed_dirs(void);
extern bool init_angband(void);
extern void cleanup_angband(void);

#endif /* INCLUDED_INIT_H */
