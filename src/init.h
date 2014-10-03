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
#include "parser.h"

/**
 * Information about maximal indices of certain arrays.
 *
 * This will become a list of "all" the game constants - NRM
 */
struct angband_constants
{
	/* Array bounds, set on parsing edit files */
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
	u16b mon_blows_max;	/**< Maximum number of monster blows */

	/* Maxima of things on a given level, read from constants.txt */
	u16b level_object_max;	/**< Maximum number of objects on a given level */
	u16b level_monster_max;	/**< Maximum number of monsters on a given level */
	u16b level_trap_max;	/**< Maximum number of traps on a given level */

	/* Monster generation constants, read from constants.txt */
	u16b alloc_monster_chance;	/**< 1/per-turn-chance of generation */
	u16b level_monster_min;		/**< Minimum number generated */
	u16b town_monsters_day;		/**< Townsfolk generated - day */
	u16b town_monsters_night;	/**< Townsfolk generated  - night */
	u16b repro_monster_max;		/**< Maximum breeders on a level */
	u16b ood_monster_chance;	/**< Chance of OoD monster is 1 in this */
	u16b ood_monster_amount;	/**< Max number of levels OoD */

	/* Monster gameplay constants, read from constants.txt */
	u16b glyph_hardness;		/**< How hard for a monster to break a glyph */
	u16b repro_monster_rate;	/**< Monster reproduction rate-slower */
	u16b life_drain_percent;	/**< Percent of player life drained */
	u16b max_flow_depth;		/**< Maximum depth for flow calculation */
};

struct init_module {
	const char *name;
	void (*init)(void);
	void (*cleanup)(void);
};

struct angband_constants *z_info;

extern const char *ANGBAND_SYS;
extern const char *ANGBAND_GRAF;

extern char *ANGBAND_DIR_APEX;
extern char *ANGBAND_DIR_EDIT;
extern char *ANGBAND_DIR_FILE;
extern char *ANGBAND_DIR_HELP;
extern char *ANGBAND_DIR_INFO;
extern char *ANGBAND_DIR_SAVE;
extern char *ANGBAND_DIR_PREF;
extern char *ANGBAND_DIR_USER;
extern char *ANGBAND_DIR_XTRA;

extern char *ANGBAND_DIR_XTRA_FONT;
extern char *ANGBAND_DIR_XTRA_GRAF;
extern char *ANGBAND_DIR_XTRA_SOUND;
extern char *ANGBAND_DIR_XTRA_ICON;

extern struct parser *init_parse_artifact(void);
extern struct parser *init_parse_c(void);
extern struct parser *init_parse_e(void);
extern struct parser *init_parse_f(void);
extern struct parser *init_parse_h(void);
extern struct parser *init_parse_k(void);
extern struct parser *init_parse_kb(void);
extern struct parser *init_parse_mp(void);
extern struct parser *init_parse_p(void);
extern struct parser *init_parse_pit(void);
extern struct parser *init_parse_r(void);
extern struct parser *init_parse_s(void);
extern struct parser *init_parse_v(void);
extern struct parser *init_parse_z(void);
extern struct parser *init_parse_flavor(void);
extern struct parser *init_parse_names(void);
extern struct parser *init_parse_hints(void);
extern struct parser *init_parse_trap(void);

extern void init_file_paths(const char *config, const char *lib, const char *data);
extern void init_arrays(void);
extern void create_needed_dirs(void);
extern bool init_angband(void);
extern void cleanup_angband(void);

#endif /* INCLUDED_INIT_H */
