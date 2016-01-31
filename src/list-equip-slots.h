/**
 * \file list-equip-slots.h
 * \brief types of slot for equipment
 *
 * Fields:
 * slot - The index name of the slot
 * acid_v - whether equipment in the slot needs checking for acid damage
 * name - whether the actual item name is mentioned when things happen to it
 * mention - description for when the slot is mentioned briefly
 * heavy describe - description for when the slot item is too heavy
 * describe - description for when the slot is described at length
 */
/* slot				acid_v	name	mention			heavy decribe	describe */
EQUIP(NONE,			false,	false,	"",				"",				"")
EQUIP(WEAPON,		false,	false,	"Wielding",		"just lifting",	"attacking monsters with")
EQUIP(BOW,			false,	false,	"Shooting",		"just holding",	"shooting missiles with")
EQUIP(RING,			false,	true,	"On %s",		"",				"wearing on your %s")
EQUIP(AMULET,		false,	true,	"Around %s",	"",				"wearing around your %s")
EQUIP(LIGHT,		false,	false,	"Light source",	"",				"using to light your way")
EQUIP(BODY_ARMOR,	true,	true,	"On %s",		"",				"wearing on your %s")
EQUIP(CLOAK,		true,	true,	"On %s",		"",				"wearing on your %s")
EQUIP(SHIELD,		true,	true,	"On %s",		"",				"wearing on your %s")
EQUIP(HAT,			true,	true,	"On %s",		"",				"wearing on your %s")
EQUIP(GLOVES,		true,	true,	"On %s",		"",				"wearing on your %s")
EQUIP(BOOTS,		true,	true,	"On %s",		"",				"wearing on your %s")
