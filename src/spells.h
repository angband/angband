/* spells.h - spell implementations and helpers */

#ifndef SPELLS_H
#define SPELLS_H

/* spells1.c */
extern s16b poly_r_idx(int r_idx);
extern void teleport_away(int m_idx, int dis);
extern void teleport_player(int dis);
extern void teleport_player_to(int ny, int nx);
extern void teleport_player_level(void);
extern void take_hit(int dam, cptr kb_str);
extern void acid_dam(int dam, cptr kb_str);
extern void elec_dam(int dam, cptr kb_str);
extern void fire_dam(int dam, cptr kb_str);
extern void cold_dam(int dam, cptr kb_str);
extern bool res_stat(int stat);
extern bool apply_disenchant(int mode);
extern bool project(int who, int rad, int y, int x, int dam, int typ, int flg);

/* spells2.c */
extern bool hp_player(int num);
extern bool heal_player(int perc, int min);
extern void warding_glyph(void);
extern bool do_dec_stat(int stat, bool perma);
extern bool do_res_stat(int stat);
extern bool do_inc_stat(int stat);
extern void identify_pack(void);
extern bool remove_curse(void);
extern bool remove_all_curse(void);
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
extern void light_room(int y1, int x1);
extern void unlight_room(int y1, int x1);
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
extern void brand_object(object_type *o_ptr, byte brand_type);
extern void brand_weapon(void);
extern bool brand_ammo(void);
extern bool brand_bolts(void);
extern void ring_of_power(int dir);

/* x-spell.c */
extern int get_spell_index(const object_type *o_ptr, int index);
extern cptr get_spell_name(int tval, int index);
extern void get_spell_info(int tval, int index, char *buf, size_t len);
extern bool cast_spell(int tval, int index, int dir);
extern bool spell_needs_aim(int tval, int spell);

#endif /* !SPELLS_H */
