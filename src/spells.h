/*
 * File: src/spells.h
 * Purpose: spell implementations and helpers
 */

#ifndef SPELLS_H
#define SPELLS_H

#include "monster/monster.h"

/*
 * Spell types used by project(), and related functions.
 */
enum
{
    #define GF(a, b, c, d, e, f, g, h, i, j, k, l, m) GF_##a,
    #include "list-gf-types.h"
    #undef GF
};

/**
 * Structure for GF types and their resistances/immunities/vulnerabilities
 */
struct gf_type {
	u16b name;      	/* numerical index (GF_#) */
	const char *desc;			/* text description (if blind) */
	int resist;			/* object flag for resistance */
	int num;			/* numerator for resistance */
	random_value denom;	/* denominator for resistance */
	int opp;			/* timed flag for temporary resistance ("opposition") */
	int immunity;		/* object flag for total immunity */
	bool side_immune;	/* whether immunity protects from ALL side effects */
	int vuln;			/* object flag for vulnerability */
	int mon_res;		/* monster flag for resistance */
	int mon_vuln;		/* monster flag for vulnerability */
	int obj_hates;		/* object flag for object vulnerability */
	int obj_imm;		/* object flag for object immunity */
};


/**
 * Bolt motion (used in prefs.c, spells1.c)
 */
enum
{
    BOLT_NO_MOTION,
    BOLT_0,
    BOLT_45,
    BOLT_90,
    BOLT_135,
    BOLT_MAX
};


/* TODO: these descriptions are somewhat wrong/misleading */
/*
 * Bit flags for the "project()" function
 *
 *   NONE: No flags
 *   JUMP: Jump directly to the target location (this is a hack)
 *   BEAM: Work as a beam weapon (affect every grid passed through)
 *   THRU: Continue "through" the target (used for "bolts"/"beams")
 *   STOP: Stop as soon as we hit a monster (used for "bolts")
 *   GRID: Affect each grid in the "blast area" in some way
 *   ITEM: Affect each object in the "blast area" in some way
 *   KILL: Affect each monster in the "blast area" in some way
 *   HIDE: Hack -- disable "visual" feedback from projection
 *   AWARE: Effects are already obvious to the player
 */
#define PROJECT_NONE  0x000
#define PROJECT_JUMP  0x001
#define PROJECT_BEAM  0x002
#define PROJECT_THRU  0x004
#define PROJECT_STOP  0x008
#define PROJECT_GRID  0x010
#define PROJECT_ITEM  0x020
#define PROJECT_KILL  0x040
#define PROJECT_HIDE  0x080
#define PROJECT_AWARE 0x100


/*
 * Bit flags for the "enchant()" function
 */
#define ENCH_TOHIT   0x01
#define ENCH_TODAM   0x02
#define ENCH_TOAC    0x04


/** Functions **/

/* spells1.c */
s16b poly_r_idx(int r_idx);
void teleport_away(struct monster *m, int dis);
void teleport_player(int dis);
void teleport_player_to(int ny, int nx);
void teleport_player_level(void);
int gf_name_to_idx(const char *name);
const char *gf_idx_to_name(int type);
void take_hit(struct player *p, int dam, const char *kb_str);
void acid_dam(int dam, const char *kb_str);
void elec_dam(int dam, const char *kb_str);
void fire_dam(int dam, const char *kb_str);
void cold_dam(int dam, const char *kb_str);
bool res_stat(int stat);
bool apply_disenchant(int mode);
bool project(int who, int rad, int y, int x, int dam, int typ, int flg);
int check_for_resist(struct player *p, int type, bitflag *flags, bool real);
bool check_side_immune(int type);
int inven_damage(struct player *p, int type, int cperc);
int adjust_dam(struct player *p, int type, int dam, aspect dam_aspect, int resist);
void monster_learn_resists(struct monster *m, struct player *p, int type);

/* spells2.c */
extern bool hp_player(int num);
extern bool heal_player(int perc, int min);
extern bool warding_glyph(void);
extern void warding_glyph_spell(void);
extern bool do_dec_stat(int stat, bool perma);
extern bool do_res_stat(int stat);
extern bool do_inc_stat(int stat);
extern void identify_pack(void);
extern bool remove_curse(void);
extern bool remove_all_curse(void);
extern bool restore_item(void);
extern bool restore_level(void);
extern bool lose_all_info(void);
extern void set_recall(void);
extern bool detect_traps(bool aware);
extern bool detect_doorstairs(bool aware);
extern bool detect_treasure(bool aware);
extern bool detect_close_buried_treasure(void);
extern bool detect_objects_magic(bool aware);
extern bool detect_monsters_normal(bool aware);
extern bool detect_monsters_invis(bool aware);
extern bool detect_monsters_evil(bool aware);
extern bool detect_all(bool aware);
extern void stair_creation(void);
extern bool enchant(object_type *o_ptr, int n, int eflag);
extern bool enchant_spell(int num_hit, int num_dam, int num_ac);
extern void do_ident_item(int item, object_type *o_ptr);
extern bool ident_spell(void);
extern bool recharge(int num);
extern bool speed_monsters(void);
extern bool slow_monsters(void);
extern bool confuse_monsters(bool aware);
extern bool sleep_monsters(bool aware);
extern bool banish_evil(int dist);
extern bool turn_undead(bool aware);
extern bool dispel_undead(int dam);
extern bool dispel_evil(int dam);
extern bool dispel_monsters(int dam);
extern void aggravate_monsters(int who);
extern bool banishment(void);
extern bool mass_banishment(void);
extern bool probing(void);
extern void destroy_area(int y1, int x1, int r, bool full);
extern void earthquake(int cy, int cx, int r);
extern bool light_area(int dam, int rad);
extern bool unlight_area(int dam, int rad);
extern bool fire_ball(int typ, int dir, int dam, int rad);
extern bool fire_swarm(int num, int typ, int dir, int dam, int rad);
extern bool fire_bolt(int typ, int dir, int dam);
extern bool fire_beam(int typ, int dir, int dam);
extern bool fire_bolt_or_beam(int prob, int typ, int dir, int dam);
extern bool project_los(int typ, int dam, bool obvious);
extern bool light_line(int dir);
extern bool strong_light_line(int dir);
extern bool drain_life(int dir, int dam);
extern bool wall_to_mud(int dir);
extern bool destroy_door(int dir);
extern bool disarm_trap(int dir);
extern bool heal_monster(int dir);
extern bool speed_monster(int dir);
extern bool slow_monster(int dir);
extern bool sleep_monster(int dir, bool aware);
extern bool confuse_monster(int dir, int plev, bool aware);
extern bool poly_monster(int dir);
extern bool clone_monster(int dir);
extern bool fear_monster(int dir, int plev, bool aware);
extern bool teleport_monster(int dir);
extern bool door_creation(void);
extern bool trap_creation(void);
extern bool destroy_doors_touch(void);
extern bool sleep_monsters_touch(bool aware);
extern bool curse_armor(void);
extern bool curse_weapon(void);
extern void brand_object(object_type *o_ptr, int brand_type);
extern void brand_weapon(void);
extern bool brand_ammo(void);
extern bool brand_bolts(void);
extern void ring_of_power(int dir);

/* x-spell.c */
extern int get_spell_index(const object_type *o_ptr, int index);
extern const char *get_spell_name(int tval, int index);
extern void get_spell_info(int tval, int index, char *buf, size_t len);
extern bool cast_spell(int tval, int index, int dir);
extern bool spell_needs_aim(int tval, int spell);

#endif /* !SPELLS_H */
