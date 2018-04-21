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
EQUIP(NONE,			FALSE,	FALSE,	"",				"",				"")
EQUIP(WEAPON,		FALSE,	FALSE,	"Wielding",		"just lifting",	"attacking monsters with")
EQUIP(BOW,			FALSE,	FALSE,	"Shooting",		"just holding",	"shooting missiles with")
EQUIP(RING,			FALSE,	TRUE,	"On %s",		"",				"wearing on your %s")
EQUIP(AMULET,		FALSE,	TRUE,	"Around %s",	"",				"wearing around your %s")
EQUIP(LIGHT,		FALSE,	FALSE,	"Light source",	"",				"using to light your way")
EQUIP(BODY_ARMOR,	TRUE,	TRUE,	"On %s",		"",				"wearing on your %s")
EQUIP(CLOAK,		TRUE,	TRUE,	"On %s",		"",				"wearing on your %s")
EQUIP(SHIELD,		TRUE,	TRUE,	"On %s",		"",				"wearing on your %s")
EQUIP(HAT,			TRUE,	TRUE,	"On %s",		"",				"wearing on your %s")
EQUIP(GLOVES,		TRUE,	TRUE,	"On %s",		"",				"wearing on your %s")
EQUIP(BOOTS,		TRUE,	TRUE,	"On %s",		"",				"wearing on your %s")
