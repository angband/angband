/**
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2007-9 Andi Sidwell, Chris Carr, Ed Graham, Erik Osheim
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband License":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#ifndef INCLUDED_BORG_TRAIT_SWAP_H
#define INCLUDED_BORG_TRAIT_SWAP_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

extern int weapon_swap; /* location of my swap weapon (+1 so zero is none) */
extern int armour_swap; /* my swap of armour (+1 so zero is none) */

extern int32_t weapon_swap_value;
extern int32_t armour_swap_value;

extern bool decurse_weapon_swap; /* my swap is great, except its cursed */
extern int  enchant_weapon_swap_to_h; /* my swap is great, except its cursed */
extern int  enchant_weapon_swap_to_d; /* my swap is great, except its cursed */
extern bool decurse_armour_swap; /* my swap is great, except its cursed */
extern int  enchant_armour_swap_to_a; /* my swap is great, except its cursed */

extern uint8_t weapon_swap_free_act;
extern uint8_t weapon_swap_resist_acid;
extern uint8_t weapon_swap_resist_elec;
extern uint8_t weapon_swap_resist_fire;
extern uint8_t weapon_swap_resist_cold;
extern uint8_t weapon_swap_resist_pois;
extern uint8_t weapon_swap_resist_conf;
extern uint8_t weapon_swap_resist_sound;
extern uint8_t weapon_swap_resist_light;
extern uint8_t weapon_swap_resist_dark;
extern uint8_t weapon_swap_resist_chaos;
extern uint8_t weapon_swap_resist_disen;
extern uint8_t weapon_swap_resist_shard;
extern uint8_t weapon_swap_resist_nexus;
extern uint8_t weapon_swap_resist_blind;
extern uint8_t weapon_swap_resist_neth;
extern uint8_t weapon_swap_resist_fear;
extern uint8_t weapon_swap_hold_life;

extern uint8_t armour_swap_free_act;
extern uint8_t armour_swap_resist_acid;
extern uint8_t armour_swap_resist_elec;
extern uint8_t armour_swap_resist_fire;
extern uint8_t armour_swap_resist_cold;
extern uint8_t armour_swap_resist_pois;
extern uint8_t armour_swap_resist_conf;
extern uint8_t armour_swap_resist_sound;
extern uint8_t armour_swap_resist_light;
extern uint8_t armour_swap_resist_dark;
extern uint8_t armour_swap_resist_chaos;
extern uint8_t armour_swap_resist_disen;
extern uint8_t armour_swap_resist_shard;
extern uint8_t armour_swap_resist_nexus;
extern uint8_t armour_swap_resist_blind;
extern uint8_t armour_swap_resist_neth;
extern uint8_t armour_swap_resist_fear;
extern uint8_t armour_swap_hold_life;

extern void borg_notice_weapon_swap(void);
extern void borg_notice_armour_swap(void);

#endif
#endif
