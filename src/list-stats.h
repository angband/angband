/**
 * \file src/list-stats.h
 * \brief player stats
 *
 * Changing stat order or making new ones will break savefiles. Stats
 * below start from 0 on line 14, so a stat's sequence number is its line
 * number minus 14.
 *
 * Each stat has a matching sustain in src/list-object-flags.h, which should
 * be at the same index in that file as the stat in this file.
 *
 * Stat properties are defined in lib/gamedata/object_property.txt
 */
STAT(STR)
STAT(INT)
STAT(WIS)
STAT(DEX)
STAT(CON)
