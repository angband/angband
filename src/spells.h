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
extern bool do_dec_stat(int stat, bool perma);
extern bool remove_curse(void);
extern bool remove_all_curse(void);
extern bool enchant(object_type *o_ptr, int n, int eflag);
extern bool enchant_spell(int num_hit, int num_dam, int num_ac);
extern void do_ident_item(object_type *o_ptr);
void teleport_player(int dis);
void teleport_player_to(int ny, int nx);
void teleport_player_level(void);
extern void brand_object(object_type *o_ptr, const char *name);
extern bool spell_identify_unknown_available(void);

#endif /* !SPELLS_H */
