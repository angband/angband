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

#include "../cmd-core.h"
#include "../game-world.h"
#include "../obj-gear.h"
#include "../obj-init.h"
#include "../obj-knowledge.h"
#include "../obj-make.h"
#include "../obj-pile.h"
#include "../obj-power.h"
#include "../obj-randart.h"
#include "../obj-tval.h"
#include "../obj-util.h"
#include "../player-birth.h"
#include "../player-spell.h"
#include "../player-timed.h"
#include "../store.h"
#include "../ui-term.h"

#include "borg-flow-kill.h"
#include "borg-flow.h"
#include "borg-init.h"
#include "borg-io.h"
#include "borg-magic.h"
#include "borg-messages-react.h"
#include "borg-messages.h"
#include "borg-trait.h"
#include "borg.h"

/*
  * Name segments for random player names
  * Copied Cth by DvE
  * Copied from borgband by APW
  */

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
 * based on a Javascript by Michael Hensley
 * "http://geocities.com/timessquare/castle/6274/"
 * Copied from Cth by DvE
 * Copied from borgband by APW
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
 * Init players with some belongings
 *
 * Having an item makes the player "aware" of its purpose.
 */
static void borg_outfit_player(struct player *p)
{
    int                      i;
    const struct start_item *si;
    struct object           *obj, *known_obj;

    /* Currently carrying nothing */
    p->upkeep->total_weight = 0;

    /* Give the player obvious object knowledge */
    p->obj_k->dd = 1;
    p->obj_k->ds = 1;
    p->obj_k->ac = 1;
    for (i = 1; i < OF_MAX; i++) {
        struct obj_property *prop = lookup_obj_property(OBJ_PROPERTY_FLAG, i);
        if (prop->subtype == OFT_LIGHT)
            of_on(p->obj_k->flags, i);
        if (prop->subtype == OFT_DIG)
            of_on(p->obj_k->flags, i);
        if (prop->subtype == OFT_THROW)
            of_on(p->obj_k->flags, i);
    }

    /* Give the player starting equipment */
    for (si = p->class->start_items; si; si = si->next) {
        int                 num  = rand_range(si->min, si->max);
        struct object_kind *kind = lookup_kind(si->tval, si->sval);
        assert(kind);

        /* Without start_kit, only start with 1 food and 1 light */
        if (!OPT(p, birth_start_kit)) {
            if (!tval_is_food_k(kind) && !tval_is_light_k(kind))
                continue;

            num = 1;
        }

        /* Exclude if configured to do so based on birth options. */
        if (si->eopts) {
            bool included = true;
            int  eind     = 0;

            while (si->eopts[eind] && included) {
                if (si->eopts[eind] > 0) {
                    if (p->opts.opt[si->eopts[eind]]) {
                        included = false;
                    }
                } else {
                    if (!p->opts.opt[-si->eopts[eind]]) {
                        included = false;
                    }
                }
                ++eind;
            }
            if (!included)
                continue;
        }

        /* Prepare a new item */
        obj = object_new();
        object_prep(obj, kind, 0, MINIMISE);
        obj->number = num;
        obj->origin = ORIGIN_BIRTH;

        known_obj   = object_new();
        obj->known  = known_obj;
        object_set_base_known(p, obj);
        object_flavor_aware(p, obj);
        obj->known->pval   = obj->pval;
        obj->known->effect = obj->effect;
        obj->known->notice |= OBJ_NOTICE_ASSESSED;

        /* Deduct the cost of the item from starting cash */
        p->au -= object_value_real(obj, obj->number);

        /* Carry the item */
        inven_carry(p, obj, true, false);
        kind->everseen = true;
    }

    /* Sanity check */
    if (p->au < 0)
        p->au = 0;

    /* Now try wielding everything */
    wield_all(p);

    /* Update knowledge */
    update_player_object_knowledge(p);
}

/*
 * Init players with some hp
 */
static void borg_roll_hp(void)
{
    int i, j, min_value, max_value;

    /* Minimum hitpoints at highest level */
    min_value = (PY_MAX_LEVEL * (player->hitdie - 1) * 3) / 8;
    min_value += PY_MAX_LEVEL;

    /* Maximum hitpoints at highest level */
    max_value = (PY_MAX_LEVEL * (player->hitdie - 1) * 5) / 8;
    max_value += PY_MAX_LEVEL;

    /* Roll out the hitpoints */
    while (true) {
        /* Roll the hitpoint values */
        for (i = 1; i < PY_MAX_LEVEL; i++) {
            j                    = randint1(player->hitdie);
            player->player_hp[i] = player->player_hp[i - 1] + j;
        }

        /* XXX Could also require acceptable "mid-level" hitpoints */

        /* Require "valid" hitpoints at highest level */
        if (player->player_hp[PY_MAX_LEVEL - 1] < min_value)
            continue;
        if (player->player_hp[PY_MAX_LEVEL - 1] > max_value)
            continue;

        /* Acceptable */
        break;
    }
}

/*
 * Allow the borg to play continuously.  Reset all values,
 */
void reincarnate_borg(void)
{
    char           buf[80];
    int            i;
    struct player *p = player;

    /* save the existing dungeon.  It is cleared later but needs to */
    /* be blank when  creating the new player */
    struct chunk *sv_cave = cave;
    cave                  = NULL;

    /* Cheat death */
    borg.trait[BI_MAXDEPTH]  = 0;
    borg.trait[BI_MAXCLEVEL] = 1;

    /* Flush message buffer */
    borg_parse(NULL);
    borg_clear_reactions();

    /* flush the commands */
    borg_flush();

    /*** Wipe the player ***/
    player_init(player);

    borg.trait[BI_ISCUT] = borg.trait[BI_ISSTUN] = borg.trait[BI_ISHEAVYSTUN]
        = borg.trait[BI_ISIMAGE] = borg.trait[BI_ISSTUDY] = false;

    /* reset our panel clock */
    borg.time_this_panel = 1;

    /* reset our vault/unique check */
    vault_on_level    = false;
    unique_on_level   = 0;
    scaryguy_on_level = false;

    /* reset our breeder flag */
    breeder_level = false;

    /* Assume not leaving the level */
    borg.goal.leaving = false;

    /* Assume not fleeing the level */
    borg.goal.fleeing = false;

    /* Assume not fleeing the level */
    borg.goal.fleeing_to_town = false;

    /* Assume not ignoring monsters */
    borg.goal.ignoring = false;

    flavor_init();

    /** Roll up a new character **/
    struct player_race  *p_race  = NULL;
    struct player_class *p_class = NULL;
    if (borg_cfg[BORG_RESPAWN_RACE] != -1)
        p_race = player_id2race(borg_cfg[BORG_RESPAWN_RACE]);
    else
        p_race = player_id2race(randint0(MAX_RACES));
    if (borg_cfg[BORG_RESPAWN_CLASS] != -1)
        p_class = player_id2class(borg_cfg[BORG_RESPAWN_CLASS]);
    else
        p_class = player_id2class(randint0(MAX_CLASSES));
    player_generate(player, p_race, p_class, false);

    /* The dungeon is not ready nor is the player */
    character_dungeon   = false;
    character_generated = false;

    /* Start in town */
    player->depth = 0;

    /* Hack -- seed for flavors */
    seed_flavor = randint0(0x10000000);

    /* Embody */
    memcpy(&p->body, &bodies[p->race->body], sizeof(p->body));
    my_strcpy(buf, bodies[p->race->body].name, sizeof(buf));
    p->body.name  = string_make(buf);
    p->body.slots = mem_zalloc(p->body.count * sizeof(struct equip_slot));
    for (i = 0; i < p->body.count; i++) {
        p->body.slots[i].type = bodies[p->race->body].slots[i].type;
        my_strcpy(buf, bodies[p->race->body].slots[i].name, sizeof(buf));
        p->body.slots[i].name = string_make(buf);
    }

    /* Get a random name */
    create_random_name(
        player->race->ridx, player->full_name, sizeof(player->full_name));

    /* Give the player some money */
    player->au = player->au_birth = z_info->start_gold;

    /* Hack - need some HP */
    borg_roll_hp();

    /* Hack - player knows all combat runes.  Maybe make them not runes? NRM */
    player->obj_k->to_a = 1;
    player->obj_k->to_h = 1;
    player->obj_k->to_d = 1;

    /* Player learns innate runes */
    player_learn_innate(player);

    /* Initialise the spells */
    player_spells_init(player);

    /* outfit the player */
    borg_outfit_player(player);

    /* generate town */
    player->upkeep->generate_level = true;
    player->upkeep->playing        = true;

    struct command fake_cmd;
    /* fake up a command */
    my_strcpy(fake_cmd.arg[0].name, "choice", sizeof(fake_cmd.arg[0].name));
    fake_cmd.arg[0].data.choice = 1;
    do_cmd_reset_stats(&fake_cmd);

    /* Initialise the stores, dungeon */
    store_reset();
    chunk_list_max = 0;

    /* Restore the standard artifacts (randarts may have been loaded) */
    cleanup_parser(&randart_parser);
    deactivate_randart_file();
    run_parser(&artifact_parser);

    /* Now only randomize the artifacts if required */
    if (OPT(player, birth_randarts)) {
        seed_randart = randint0(0x10000000);
        do_randart(seed_randart, true);
        deactivate_randart_file();
    }

    /* Hack -- flush it */
    Term_fresh();

    /*** Hack -- react to race and class ***/

    /* Notice the new race and class */
    borg_prepare_race_class_info();

#if false
    /* need to check all stats */
    for (int tmp_i = 0; tmp_i < STAT_MAX; tmp_i++)
        my_need_stat_check[tmp_i] = true;
#endif

    borg_notice_player();

    /* Message */
    borg_note("# Respawning");
    borg_respawning = 5;

    /* fully healed and rested */
    player->chp = player->mhp;
    player->csp = player->msp;
    player->upkeep->energy_use = 100;

    /* don't notice or update immediately */
    p->upkeep->notice = 0;
    p->upkeep->update = 0;
    p->upkeep->redraw = 0;

    /* restore the cave */
    cave = sv_cave;

    /* the new player is now ready */
    character_generated = true;

    /* Mark savefile as borg cheater */
    if (!(player->noscore & NOSCORE_BORG))
        player->noscore |= NOSCORE_BORG;

    /* Done.  Play on */
}

#endif
