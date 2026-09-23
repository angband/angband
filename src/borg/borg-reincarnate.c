/**
 * \file borg-reincarnate.c
 * \brief Resurrect the borg with new stats and class
 *
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

#include "borg-reincarnate.h"

#ifdef ALLOW_BORG

#include "../ui-menu.h"

#include "borg-init.h"
#include "borg-io.h"
#include "borg-messages-react.h"
#include "borg-trait.h"
#include "borg.h"

/* Name segments for random player names */

/* Dwarves */
static const char *dwarf_syllable1[] =
{
    "B", "D", "F", "G", "Gl", "H", "K", "L",
    "M", "N", "R", "S", "T", "Th", "V",
};

static const char *dwarf_syllable2[] =
{
    "a", "e", "i", "o", "oi", "u",
};

static const char *dwarf_syllable3[] =
{
    "bur", "fur", "gan", "gnus", "gnar", "li", "lin", "lir", "mli", "nar",
    "nus", "rin", "ran", "sin", "sil", "sur",
};

/* Elves */
static const char *elf_syllable1[] =
{
    "Al", "An", "Bal", "Bel", "Cal", "Cel", "El", "Elr", "Elv", "Eow", "Ear",
    "F", "Fal", "Fel", "Fin", "G", "Gal", "Gel", "Gl", "Is", "Lan", "Leg",
    "Lom", "N", "Nal", "Nel",  "S", "Sal", "Sel", "T", "Tal", "Tel", "Thr",
    "Tin",
};

static const char *elf_syllable2[] =
{
    "a", "adrie", "ara", "e", "ebri", "ele", "ere", "i", "io", "ithra", "ilma",
    "il-Ga", "ili", "o", "orfi", "u", "y",
};

static const char *elf_syllable3[] =
{
    "l", "las", "lad", "ldor", "ldur", "linde", "lith", "mir", "n", "nd",
    "ndel", "ndil", "ndir", "nduil", "ng", "mbor", "r", "rith", "ril", "riand",
    "rion", "s", "thien", "viel", "wen", "wyn",
};

/* Gnomes */
static const char *gnome_syllable1[] =
{
    "Aar", "An", "Ar", "As", "C", "H", "Han", "Har", "Hel", "Iir", "J", "Jan",
    "Jar", "K", "L", "M", "Mar", "N", "Nik", "Os", "Ol", "P", "R", "S", "Sam",
    "San", "T", "Ter", "Tom", "Ul", "V", "W", "Y",
};

static const char *gnome_syllable2[] =
{
    "a", "aa",  "ai", "e", "ei", "i", "o", "uo", "u", "uu",
};

static const char *gnome_syllable3[] =
{
    "ron", "re", "la", "ki", "kseli", "ksi", "ku", "ja", "ta", "na", "namari",
    "neli", "nika", "nikki", "nu", "nukka", "ka", "ko", "li", "kki", "rik",
    "po", "to", "pekka", "rjaana", "rjatta", "rjukka", "la", "lla", "lli",
    "mo", "nni",
};

/* Hobbit */
static const char *hobbit_syllable1[] =
{
    "B", "Ber", "Br", "D", "Der", "Dr", "F", "Fr", "G", "H", "L", "Ler", "M",
    "Mer", "N", "P", "Pr", "Per", "R", "S", "T", "W",
};

static const char *hobbit_syllable2[] =
{
    "a", "e", "i", "ia", "o", "oi", "u",
};

static const char *hobbit_syllable3[] =
{
    "bo", "ck", "decan", "degar", "do", "doc", "go", "grin", "lba", "lbo",
    "lda", "ldo", "lla", "ll", "lo", "m", "mwise", "nac", "noc", "nwise", "p",
    "ppin", "pper", "tho", "to",
};

/* Human */
static const char *human_syllable1[] =
{
    "Ab", "Ac", "Ad", "Af", "Agr", "Ast", "As", "Al", "Adw", "Adr", "Ar", "B",
    "Br", "C", "Cr", "Ch", "Cad", "D", "Dr", "Dw", "Ed", "Eth", "Et", "Er",
    "El", "Eow", "F", "Fr", "G", "Gr", "Gw", "Gal", "Gl", "H", "Ha", "Ib",
    "Jer", "K", "Ka", "Ked", "L", "Loth", "Lar", "Leg", "M", "Mir", "N", "Nyd",
    "Ol", "Oc", "On", "P", "Pr", "R", "Rh", "S", "Sev", "T", "Tr", "Th", "V",
    "Y", "Z", "W", "Wic",
};

static const char *human_syllable2[] =
{
    "a", "ae", "au", "ao", "are", "ale", "ali", "ay", "ardo", "e", "ei", "ea",
    "eri", "era", "ela", "eli", "enda", "erra", "i", "ia", "ie", "ire", "ira",
    "ila", "ili", "ira", "igo", "o", "oa", "oi", "oe", "ore", "u", "y",
};

static const char *human_syllable3[] =
{
    "a", "and", "b", "bwyn", "baen", "bard", "c", "ctred", "cred", "ch", "can",
    "d", "dan", "don", "der", "dric", "dfrid", "dus", "f", "g", "gord", "gan",
    "l", "li", "lgrin", "lin", "lith", "lath", "loth", "ld", "ldric", "ldan",
    "m", "mas", "mos", "mar", "mond", "n", "nydd", "nidd", "nnon", "nwan",
    "nyth", "nad", "nn", "nnor", "nd", "p", "r", "ron", "rd", "s", "sh",
    "seth", "sean", "t", "th", "tha", "tlan", "trem", "tram", "v", "vudd",
    "w", "wan", "win", "wyn", "wyr", "wyr", "wyth",
};

/* Orc */
static const char *orc_syllable1[] =
{
    "B", "Er", "G", "Gr", "H", "P", "Pr", "R", "V", "Vr", "T", "Tr", "M", "Dr",
};

static const char *orc_syllable2[] =
{
    "a", "i", "o", "oo", "u", "ui",
};

static const char *orc_syllable3[] =
{
    "dash", "dish", "dush", "gar", "gor", "gdush", "lo", "gdish", "k", "lg",
    "nak", "rag", "rbag", "rg", "rk", "ng", "nk", "rt", "ol", "urk", "shnak",
    "mog", "mak", "rak",
};

/*
 * Random Name Generator
 */
static void create_random_name(int race, char *name, size_t name_len)
{
    /* Paranoia */
    if (!name)
        return;

    /* Select the monster type */
    switch (race) {
        /* Create the monster name */
    case RACE_DWARF:
        my_strcpy(name,
            dwarf_syllable1[randint0(sizeof(dwarf_syllable1) / sizeof(char *))],
            name_len);
        my_strcat(name,
            dwarf_syllable2[randint0(sizeof(dwarf_syllable2) / sizeof(char *))],
            name_len);
        my_strcat(name,
            dwarf_syllable3[randint0(sizeof(dwarf_syllable3) / sizeof(char *))],
            name_len);
        break;
    case RACE_ELF:
    case RACE_HALF_ELF:
    case RACE_HIGH_ELF:
        my_strcpy(name,
            elf_syllable1[randint0(sizeof(elf_syllable1) / sizeof(char *))],
            name_len);
        my_strcat(name,
            elf_syllable2[randint0(sizeof(elf_syllable2) / sizeof(char *))],
            name_len);
        my_strcat(name,
            elf_syllable3[randint0(sizeof(elf_syllable3) / sizeof(char *))],
            name_len);
        break;
    case RACE_GNOME:
        my_strcpy(name,
            gnome_syllable1[randint0(sizeof(gnome_syllable1) / sizeof(char *))],
            name_len);
        my_strcat(name,
            gnome_syllable2[randint0(sizeof(gnome_syllable2) / sizeof(char *))],
            name_len);
        my_strcat(name,
            gnome_syllable3[randint0(sizeof(gnome_syllable3) / sizeof(char *))],
            name_len);
        break;
    case RACE_HOBBIT:
        my_strcpy(name,
            hobbit_syllable1[randint0(
                sizeof(hobbit_syllable1) / sizeof(char *))],
            name_len);
        my_strcat(name,
            hobbit_syllable2[randint0(
                sizeof(hobbit_syllable2) / sizeof(char *))],
            name_len);
        my_strcat(name,
            hobbit_syllable3[randint0(
                sizeof(hobbit_syllable3) / sizeof(char *))],
            name_len);
        break;
    case RACE_HUMAN:
    case RACE_DUNADAN:
        my_strcpy(name,
            human_syllable1[randint0(sizeof(human_syllable1) / sizeof(char *))],
            name_len);
        my_strcat(name,
            human_syllable2[randint0(sizeof(human_syllable2) / sizeof(char *))],
            name_len);
        my_strcat(name,
            human_syllable3[randint0(sizeof(human_syllable3) / sizeof(char *))],
            name_len);
        break;
    case RACE_HALF_ORC:
    case RACE_HALF_TROLL:
    case RACE_KOBOLD:
        my_strcpy(name,
            orc_syllable1[randint0(sizeof(orc_syllable1) / sizeof(char *))],
            name_len);
        my_strcat(name,
            orc_syllable2[randint0(sizeof(orc_syllable2) / sizeof(char *))],
            name_len);
        my_strcat(name,
            orc_syllable3[randint0(sizeof(orc_syllable3) / sizeof(char *))],
            name_len);
        break;
    /* Create an empty name */
    default:
        name[0] = '\0';
        break;
    }
}

/*
 * Kill the borg and force a reincarnation.
 */
void borg_force_reincarnate(void)
{
    /* Kill the borg */
    borg_keypress('Q');
    borg_keypress('y');
    borg_keypress('@');

    /* Respawn the borg */
    borg_reincarnate_start();
}


/*
 * Wrap up the reincarnation process
 */
void borg_reincarnate_end(void)
{
    borg.player = player;

    /* Notice the new race and class */
    borg_prepare_race_class_info();

    borg_notice_player();

    /* clear out the old reactions again in case more messages came after */
    /* the borg died */
    borg_clear_reactions();

    /* set the old depth so we know we are on a new level */
    borg.status.old_depth = 128;

    /* Mark savefile as borg cheater */
    if (!(player->noscore & NOSCORE_BORG))
        player->noscore |= NOSCORE_BORG;

    borg_reinit_options();

    /* We need the borg to keep playing after he dies */
    option_set("cheat_live", true);

    /* Message */
    borg_note("# Done respawning");
}

/*
 * Allow the borg to play continuously.  Reset all values,
 */
void borg_reincarnate_start(void)
{
    char full_name[PLAYER_NAME_LEN];
    unsigned int  race;
    unsigned int  class;

    borg.status.respawning = true;

    /* clear out the old reactions */
    borg_clear_reactions();

    borg_note("# Respawning");

    /* New character */
    borg_keypress('n');
    borg_keypress('y');

    /* Not based on old character  */
    borg_keypress('n');

    if (borg_cfg[BORG_RESPAWN_RACE] != -1)
        race = borg_cfg[BORG_RESPAWN_RACE];
    else
        race = randint0(MAX_RACES);
    if (borg_cfg[BORG_RESPAWN_CLASS] != -1)
        class = borg_cfg[BORG_RESPAWN_CLASS];
    else
        class = randint0(MAX_CLASSES);

    /* pick race */
    if (race != player->race->ridx) {
        borg_keypress(all_letters[race]);
    }
    borg_keypress(KC_ENTER);

    /* pick class */
    if (class != player->class->cidx) {
        borg_keypress(all_letters[class]);
    }
    borg_keypress(KC_ENTER);

    /* point based */
    borg_keypress(KC_ENTER);

    /* accept points */
    borg_keypress(KC_ENTER);

    /* Get a random name */
    create_random_name(race, full_name, sizeof(full_name));

    borg_keypresses(full_name);
    borg_keypress(KC_ENTER);

    /* accept history */
    borg_keypress(KC_ENTER);

    /* continue */
    borg_keypress(KC_ENTER);
}

#endif
