/*
 * File: src/spells.h
 * Purpose: spell implementations and helpers
 */

#ifndef SPELLS_H
#define SPELLS_H

#include "monster.h"

/*
 * Bit flags for the "enchant()" function
 */
#define ENCH_TOHIT   0x01
#define ENCH_TODAM   0x02
#define ENCH_TOAC    0x04


/** Functions **/

/* spells.c */
extern bool hp_player(int num);
extern bool heal_player(int perc, int min);
extern bool warding_glyph(void);
extern void warding_glyph_spell(void);
bool res_stat(int stat);
extern bool do_dec_stat(int stat, bool perma);
extern bool do_res_stat(int stat);
extern bool do_inc_stat(int stat);
extern void identify_pack(void);
extern bool remove_curse(void);
extern bool remove_all_curse(void);
extern bool restore_level(void);
extern bool lose_all_info(void);
extern bool set_recall(void);
extern bool detect_traps(bool aware);
extern bool detect_doorstairs(bool aware);
extern bool detect_treasure(bool aware, bool full);
extern bool detect_close_buried_treasure(void);
extern bool detect_monsters_normal(bool aware);
extern bool detect_monsters_invis(bool aware);
extern bool detect_monsters_evil(bool aware);
extern bool detect_monsters_entire_level(void);
extern bool detect_all(bool aware);
extern void stair_creation(void);
bool apply_disenchant(int mode);
extern bool enchant(object_type *o_ptr, int n, int eflag);
extern bool enchant_spell(int num_hit, int num_dam, int num_ac);
extern void do_ident_item(object_type *o_ptr);
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
extern void aggravate_monsters(struct monster *who);
extern bool banishment(void);
extern bool mass_banishment(void);
extern bool probing(void);
void teleport_away(struct monster *m, int dis);
void teleport_player(int dis);
void teleport_player_to(int ny, int nx);
void teleport_player_level(void);
extern void destroy_area(int y1, int x1, int r, bool full);
extern void earthquake(int cy, int cx, int r);
void light_room(int y1, int x1, bool light);
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
extern void brand_object(object_type *o_ptr, const char *name);
extern void brand_weapon(void);
extern bool brand_ammo(void);
extern bool brand_bolts(void);
extern void ring_of_power(int dir);
extern bool spell_identify_unknown_available(void);

#endif /* !SPELLS_H */
