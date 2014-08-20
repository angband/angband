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
bool res_stat(int stat);
extern bool do_dec_stat(int stat, bool perma);
extern bool do_res_stat(int stat);
extern void identify_pack(void);
extern bool remove_curse(void);
extern bool remove_all_curse(void);
extern bool restore_level(void);
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
bool apply_disenchant(int mode);
extern bool enchant(object_type *o_ptr, int n, int eflag);
extern bool enchant_spell(int num_hit, int num_dam, int num_ac);
extern void do_ident_item(object_type *o_ptr);
extern bool ident_spell(void);
void teleport_away(struct monster *m, int dis);
void teleport_player(int dis);
void teleport_player_to(int ny, int nx);
void teleport_player_level(void);
extern void destroy_area(int y1, int x1, int r, bool full);
extern void earthquake(int cy, int cx, int r);
void light_room(int y1, int x1, bool light);
extern void brand_object(object_type *o_ptr, const char *name);
extern bool spell_identify_unknown_available(void);

#endif /* !SPELLS_H */
