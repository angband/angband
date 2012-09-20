/* File: load2.c */

/* Purpose: support for loading savefiles */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"


/*
 * This file is responsible for loading all "old" savefiles
 *
 * Note that 2.7.0 - 2.7.2 savefiles are obsolete and will not work.
 *
 * Note that pre-2.5.1 savefiles are non-standardized and will not work.
 *
 * Note that pre-2.7.0 savefiles lose a lot of information when imported.
 *
 * We attempt to prevent corrupt savefiles from inducing memory errors.
 *
 * Note that Angband 2.7.9 encodes "terrain features" in the savefile
 * using the old 2.7.8 method.  Angband 2.8.0 will use the same method
 * to read pre-2.8.0 savefiles, but will use a new method to save them,
 * which will only affect "save.c".
 *
 * Note that Angband 2.8.0 will use a VERY different savefile method,
 * which will use "blocks" of information which can be ignored or parsed,
 * and which will not use a silly "protection" scheme on the savefiles,
 * but which may still use some form of "checksums" to prevent the use
 * of "corrupt" savefiles, which might cause nasty weirdness.
 *
 * Note that this file should not use the random number generator, the
 * object flavors, the visual attr/char mappings, or anything else which
 * is initialized *after* or *during* the "load character" function.
 */



/*
 * New "cave grid" flags -- saved in savefile
 */
#define OLD_GRID_W_01	0x0001	/* Wall type (bit 1) */
#define OLD_GRID_W_02	0x0002	/* Wall type (bit 2) */
#define OLD_GRID_PERM	0x0004	/* Wall type is permanent */
#define OLD_GRID_QQQQ	0x0008	/* Unused */
#define OLD_GRID_MARK	0x0010	/* Grid is memorized */
#define OLD_GRID_GLOW	0x0020	/* Grid is illuminated */
#define OLD_GRID_ROOM	0x0040	/* Grid is part of a room */
#define OLD_GRID_ICKY	0x0080	/* Grid is anti-teleport */

/*
 * Masks for the new grid types
 */
#define OLD_GRID_WALL_MASK	0x0003	/* Wall type */

/*
 * Legal results of OLD_GRID_WALL_MASK
 */
#define OLD_GRID_WALL_NONE		0x0000	/* No wall */
#define OLD_GRID_WALL_MAGMA		0x0001	/* Magma vein */
#define OLD_GRID_WALL_QUARTZ	0x0002	/* Quartz vein */
#define OLD_GRID_WALL_GRANITE	0x0003	/* Granite wall */




/*
 * This save package was brought to by		-JWT- and -RAK-
 * and has been completely rewritten for UNIX by	-JEW-
 *
 * and has been completely rewritten again by	 -CJS-
 * and completely rewritten again! for portability by -JEW-
 *
 * Much of this file is used only for pre-2.7.0 savefiles.  The relevant
 * functions often contain the suffix "_old" to indicate them.  This entire
 * file was rewritten by -BEN- and most of it was built from scratch.
 *
 * Note that the "current" savefile format is indicated in "save.c",
 * where the savefiles are actually created.
 *
 * This file is rather huge, mostly to handle pre-2.7.0 savefiles.
 */



/*
 * Local "loading" parameters, to cut down on local parameters
 */

static FILE	*fff;		/* Current save "file" */

static byte	xor_byte;	/* Simple encryption */

static byte	sf_major;	/* Savefile's "version_major" */
static byte	sf_minor;	/* Savefile's "version_minor" */
static byte	sf_patch;	/* Savefile's "version_patch" */

static u32b	v_check = 0L;	/* A simple "checksum" on the actual values */
static u32b	x_check = 0L;	/* A simple "checksum" on the encoded bytes */



/*
 * This function determines if the version of the savefile
 * currently being read is older than version "x.y.z".
 */
static bool older_than(byte x, byte y, byte z)
{
    /* Much older, or much more recent */
    if (sf_major < x) return (TRUE);
    if (sf_major > x) return (FALSE);

    /* Distinctly older, or distinctly more recent */
    if (sf_minor < y) return (TRUE);
    if (sf_minor > y) return (FALSE);

    /* Barely older, or barely more recent */
    if (sf_patch < z) return (TRUE);
    if (sf_patch > z) return (FALSE);

    /* Identical versions */
    return (FALSE);
}


/*
 * Show information on the screen, one line at a time.
 * Avoid the top two lines, to avoid interference with "msg_print()".
 */
static void note(cptr msg)
{
    static int y = 2;

    /* Draw the message */
    prt(msg, y, 0);

    /* Advance one line (wrap if needed) */
    if (++y >= 24) y = 2;

    /* Flush it */
    Term_fresh();
}


/*
 * Hack -- determine if an item is "wearable" (or a missile)
 */
static bool wearable_p(inven_type *i_ptr)
{
    /* Valid "tval" codes */
    switch (i_ptr->tval) {
        case TV_SHOT:
        case TV_ARROW:
        case TV_BOLT:
        case TV_BOW:
        case TV_DIGGING:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_SWORD:
        case TV_BOOTS:
        case TV_GLOVES:
        case TV_HELM:
        case TV_CROWN:
        case TV_SHIELD:
        case TV_CLOAK:
        case TV_SOFT_ARMOR:
        case TV_HARD_ARMOR:
        case TV_DRAG_ARMOR:
        case TV_LITE:
        case TV_AMULET:
        case TV_RING:
            return (TRUE);
    }
    
    /* Nope */
    return (FALSE);
}


/*
 * The following functions are used to load the basic building blocks
 * of savefiles.  They also maintain the "checksum" info for 2.7.0+
 */

static byte sf_get(void)
{
    byte c, v;

    /* Get a character, decode the value */
    c = getc(fff) & 0xFF;
    v = c ^ xor_byte;
    xor_byte = c;

    /* Maintain the checksum info */
    v_check += v;
    x_check += xor_byte;

    /* Return the value */
    return (v);
}

static void rd_byte(byte *ip)
{
    *ip = sf_get();
}

static void rd_char(char *ip)
{
    rd_byte((byte*)ip);
}

static void rd_u16b(u16b *ip)
{
    (*ip) = sf_get();
    (*ip) |= ((u16b)(sf_get()) << 8);
}

static void rd_s16b(s16b *ip)
{
    rd_u16b((u16b*)ip);
}

static void rd_u32b(u32b *ip)
{
    (*ip) = sf_get();
    (*ip) |= ((u32b)(sf_get()) << 8);
    (*ip) |= ((u32b)(sf_get()) << 16);
    (*ip) |= ((u32b)(sf_get()) << 24);
}

static void rd_s32b(s32b *ip)
{
    rd_u32b((u32b*)ip);
}


/*
 * Hack -- read a string
 */
static void rd_string(char *str, int max)
{
    int i;

    /* Read the string */    
    for (i = 0; TRUE; i++) {

        byte tmp8u;

        /* Read a byte */
        rd_byte(&tmp8u);

        /* Collect string while legal */
        if (i < max) str[i] = tmp8u;

        /* End of string */
        if (!tmp8u) break;
    }

    /* Terminate */
    str[max-1] = '\0';
}


/*
 * Hack -- strip some bytes
 */
static void strip_bytes(int n)
{
    int i;
    byte tmp8u;

    /* Strip the bytes */
    for (i = 0; i < n; i++) rd_byte(&tmp8u);
}


/*
 * Owner Conversion -- pre-2.7.8 to 2.7.8
 * Shop is column, Owner is Row, see "tables.c"
 */
static byte convert_owner[24] = {
    1, 3, 1, 0, 2, 3, 2, 0,
    0, 1, 3, 1, 0, 1, 1, 0,
    3, 2, 0, 2, 1, 2, 3, 0
};


/*
 * Old inventory slot values (pre-2.7.3)
 */
#define OLD_INVEN_WIELD     22
#define OLD_INVEN_HEAD      23
#define OLD_INVEN_NECK      24
#define OLD_INVEN_BODY      25
#define OLD_INVEN_ARM       26
#define OLD_INVEN_HANDS     27
#define OLD_INVEN_RIGHT     28
#define OLD_INVEN_LEFT      29
#define OLD_INVEN_FEET      30
#define OLD_INVEN_OUTER     31
#define OLD_INVEN_LITE      32
#define OLD_INVEN_AUX       33

/*
 * Analyze pre-2.7.3 inventory slots
 */
static s16b convert_slot(int old)
{
    /* Move slots */
    switch (old)
    {
        case OLD_INVEN_WIELD: return (INVEN_WIELD);
        case OLD_INVEN_HEAD: return (INVEN_HEAD);
        case OLD_INVEN_NECK: return (INVEN_NECK);
        case OLD_INVEN_BODY: return (INVEN_BODY);
        case OLD_INVEN_ARM: return (INVEN_ARM);
        case OLD_INVEN_HANDS: return (INVEN_HANDS);
        case OLD_INVEN_RIGHT: return (INVEN_RIGHT);
        case OLD_INVEN_LEFT: return (INVEN_LEFT);
        case OLD_INVEN_FEET: return (INVEN_FEET);
        case OLD_INVEN_OUTER: return (INVEN_OUTER);
        case OLD_INVEN_LITE: return (INVEN_LITE);

        /* Hack -- "hold" old aux items */
        case OLD_INVEN_AUX: return (INVEN_WIELD - 1);
    }

    /* Default */
    return (old);
}





/*
 * Hack -- convert 2.7.X ego-item indexes into 2.7.9 ego-item indexes
 */

static byte convert_ego_item[128] = {

    0				/* 0 */,
    EGO_RESISTANCE		/* 1 = EGO_RESIST (XXX) */,
    EGO_RESIST_ACID		/* 2 = EGO_RESIST_A (XXX) */,
    EGO_RESIST_FIRE		/* 3 = EGO_RESIST_F (XXX) */,
    EGO_RESIST_COLD		/* 4 = EGO_RESIST_C (XXX) */,
    EGO_RESIST_ELEC		/* 5 = EGO_RESIST_E (XXX) */,
    EGO_HA			/* 6 = EGO_HA */,
    EGO_DF			/* 7 = EGO_DF */,
    EGO_SLAY_ANIMAL		/* 8 = EGO_SLAY_ANIMAL */,
    EGO_SLAY_DRAGON		/* 9 = EGO_SLAY_DRAGON */,
    EGO_SLAY_EVIL		/* 10 = EGO_SLAY_EVIL (XXX) */,
    EGO_SLAY_UNDEAD		/* 11 = EGO_SLAY_UNDEAD (XXX) */,
    EGO_BRAND_FIRE		/* 12 = EGO_FT */,
    EGO_BRAND_COLD		/* 13 = EGO_FB */,
    EGO_FREE_ACTION		/* 14 = EGO_FREE_ACTION (XXX) */,
    EGO_SLAYING			/* 15 = EGO_SLAYING */,
    0				/* 16 */,
    0				/* 17 */,
    EGO_SLOW_DESCENT		/* 18 = EGO_SLOW_DESCENT */,
    EGO_SPEED			/* 19 = EGO_SPEED */,
    EGO_STEALTH			/* 20 = EGO_STEALTH (XXX) */,
    0				/* 21 */,
    0				/* 22 */,
    0				/* 23 */,
    EGO_INTELLIGENCE		/* 24 = EGO_INTELLIGENCE */,
    EGO_WISDOM			/* 25 = EGO_WISDOM */,
    EGO_INFRAVISION		/* 26 = EGO_INFRAVISION */,
    EGO_MIGHT			/* 27 = EGO_MIGHT */,
    EGO_LORDLINESS		/* 28 = EGO_LORDLINESS */,
    EGO_MAGI			/* 29 = EGO_MAGI (XXX) */,
    EGO_BEAUTY			/* 30 = EGO_BEAUTY */,
    EGO_SEEING			/* 31 = EGO_SEEING (XXX) */,
    EGO_REGENERATION		/* 32 = EGO_REGENERATION */,
    0				/* 33 */,
    0				/* 34 */,
    0				/* 35 */,
    0				/* 36 */,
    0				/* 37 */,
    EGO_PERMANENCE		/* 38 = EGO_ROBE_MAGI */,
    EGO_PROTECTION		/* 39 = EGO_PROTECTION */,
    0				/* 40 */,
    0				/* 41 */,
    0				/* 42 */,
    EGO_BRAND_FIRE		/* 43 = EGO_FIRE (XXX) */,
    EGO_HURT_EVIL		/* 44 = EGO_AMMO_EVIL */,
    EGO_HURT_DRAGON		/* 45 = EGO_AMMO_DRAGON */,
    0				/* 46 */,
    0				/* 47 */,
    0				/* 48 */,
    0				/* 49 */,
    EGO_FLAME			/* 50 = EGO_AMMO_FIRE */,
    0				/* 51 */,	/* oops */
    EGO_FROST			/* 52 = EGO_AMMO_SLAYING */,
    0				/* 53 */,
    0				/* 54 */,
    EGO_HURT_ANIMAL		/* 55 = EGO_AMMO_ANIMAL */,
    0				/* 56 */,
    0				/* 57 */,
    0				/* 58 */,
    0				/* 59 */,
    EGO_EXTRA_MIGHT		/* 60 = EGO_EXTRA_MIGHT */,
    EGO_EXTRA_SHOTS		/* 61 = EGO_EXTRA_SHOTS */,
    0				/* 62 */,
    0				/* 63 */,
    EGO_VELOCITY		/* 64 = EGO_VELOCITY */,
    EGO_ACCURACY		/* 65 = EGO_ACCURACY */,
    0				/* 66 */,
    EGO_SLAY_ORC		/* 67 = EGO_SLAY_ORC */,
    EGO_POWER			/* 68 = EGO_POWER */,
    0				/* 69 */,
    0				/* 70 */,
    EGO_WEST			/* 71 = EGO_WEST */,
    EGO_BLESS_BLADE		/* 72 = EGO_BLESS_BLADE */,
    EGO_SLAY_DEMON		/* 73 = EGO_SLAY_DEMON */,
    EGO_SLAY_TROLL		/* 74 = EGO_SLAY_TROLL */,
    0				/* 75 */,
    0				/* 76 */,
    EGO_WOUNDING		/* 77 = EGO_AMMO_WOUNDING */,
    0				/* 78 */,
    0				/* 79 */,
    0				/* 80 */,
    EGO_LITE			/* 81 = EGO_LITE */,
    EGO_AGILITY			/* 82 = EGO_AGILITY */,
    0				/* 83 */,
    0				/* 84 */,
    EGO_SLAY_GIANT		/* 85 = EGO_SLAY_GIANT */,
    EGO_TELEPATHY		/* 86 = EGO_TELEPATHY */,
    EGO_ELVENKIND		/* 87 = EGO_ELVENKIND (XXX) */,
    0				/* 88 */,
    0				/* 89 */,
    EGO_ATTACKS			/* 90 = EGO_ATTACKS */,
    EGO_AMAN			/* 91 = EGO_AMAN */,
    0				/* 92 */,
    0				/* 93 */,
    0				/* 94 */,
    0				/* 95 */,
    0				/* 96 */,
    0				/* 97 */,
    0				/* 98 */,
    0				/* 99 */,
    0				/* 100 */,
    0				/* 101 */,
    0				/* 102 */,
    0				/* 103 */,
    EGO_WEAKNESS		/* 104 = EGO_WEAKNESS */,
    EGO_STUPIDITY		/* 105 = EGO_STUPIDITY */,
    EGO_NAIVETY			/* 106 = EGO_DULLNESS */,
    EGO_SICKLINESS		/* 107 = EGO_SICKLINESS */,
    EGO_CLUMSINESS		/* 108 = EGO_CLUMSINESS */,
    EGO_UGLINESS		/* 109 = EGO_UGLINESS */,
    EGO_TELEPORTATION		/* 110 = EGO_TELEPORTATION */,
    0				/* 111 */,
    EGO_IRRITATION		/* 112 = EGO_IRRITATION */,
    EGO_VULNERABILITY		/* 113 = EGO_VULNERABILITY */,
    EGO_ENVELOPING		/* 114 = EGO_ENVELOPING */,
    0				/* 115 */,
    EGO_SLOWNESS		/* 116 = EGO_SLOWNESS */,
    EGO_NOISE			/* 117 = EGO_NOISE */,
    EGO_ANNOYANCE		/* 118 = EGO_GREAT_MASS */,
    0				/* 119 */,
    EGO_BACKBITING		/* 120 = EGO_BACKBITING */,
    0				/* 121 */,
    0				/* 122 */,
    0				/* 123 */,
    EGO_MORGUL			/* 124 = EGO_MORGUL */,
    0				/* 125 */,
    EGO_SHATTERED		/* 126 = EGO_SHATTERED */,
    EGO_BLASTED			/* 127 = EGO_BLASTED (XXX) */
};


/*
 * Read an item (2.7.0 or later)
 *
 * Note that savefiles from 2.7.0 and 2.7.1 are obsolete.
 *
 * Note that pre-2.7.9 savefiles (from Angband 2.5.1 onward anyway)
 * can be read using the code above.
 *
 * This function attempts to "repair" old savefiles, and to extract
 * the most up to date values for various object fields.
 *
 * Note that Angband 2.7.9 introduced a new method for object "flags"
 * in which the "flags" on an object are actually extracted when they
 * are needed from the object kind, artifact index, ego-item index,
 * and two special "xtra" fields which are used to encode any "extra"
 * power of certain ego-items.  This had the side effect that items
 * imported from pre-2.7.9 savefiles will lose any "extra" powers they
 * may have had, and also, all "uncursed" items will become "cursed"
 * again, including Calris, even if it is being worn at the time.  As
 * a complete hack, items which are inscribed with "uncursed" will be
 * "uncursed" when imported from pre-2.7.9 savefiles.
 */
static void rd_item(inven_type *i_ptr)
{
    byte old_dd;
    byte old_ds;

    s32b old_cost;

    u32b f1, f2, f3;

    u16b tmp16u;

    inven_kind *k_ptr;

    char note[128];


    invwipe(i_ptr);

    rd_s16b(&i_ptr->k_idx);

    rd_byte(&i_ptr->iy);
    rd_byte(&i_ptr->ix);

    rd_byte(&i_ptr->tval);
    rd_byte(&i_ptr->sval);
    rd_s16b(&i_ptr->pval);

    /* Old method */
    if (older_than(2,7,8)) {

        rd_byte(&i_ptr->name1);
        rd_byte(&i_ptr->name2);
        rd_byte(&i_ptr->ident);
        rd_byte(&i_ptr->number);
        rd_s16b(&i_ptr->weight);
        rd_s16b(&i_ptr->timeout);

        rd_s16b(&i_ptr->to_h);
        rd_s16b(&i_ptr->to_d);
        rd_s16b(&i_ptr->to_a);
        
        rd_s16b(&i_ptr->ac);
        
        rd_byte(&old_dd);
        rd_byte(&old_ds);

        strip_bytes(2);

        rd_s32b(&old_cost);

        strip_bytes(4);
    }

    /* New method */
    else {

        rd_byte(&i_ptr->discount);
        rd_byte(&i_ptr->number);
        rd_s16b(&i_ptr->weight);

        rd_byte(&i_ptr->name1);
        rd_byte(&i_ptr->name2);
        rd_s16b(&i_ptr->timeout);

        rd_s16b(&i_ptr->to_h);
        rd_s16b(&i_ptr->to_d);
        rd_s16b(&i_ptr->to_a);
        
        rd_s16b(&i_ptr->ac);
        
        rd_byte(&old_dd);
        rd_byte(&old_ds);

        rd_byte(&i_ptr->ident);

        rd_byte(&i_ptr->marked);
    }

    strip_bytes(12);

    rd_u16b(&tmp16u);
    
    rd_byte(&i_ptr->xtra1);
    rd_byte(&i_ptr->xtra2);

    rd_string(note, 128);

    /* Save the inscription */
    if (note[0]) i_ptr->note = quark_add(note);


    /* XXX XXX Mega-Hack -- handle "dungeon objects" later */
    if ((i_ptr->k_idx >= 445) && (i_ptr->k_idx <= 479)) return;


    /* Obtain the "kind" template */
    k_ptr = &k_info[i_ptr->k_idx];

    /* Obtain tval/sval from k_info */
    i_ptr->tval = k_ptr->tval;
    i_ptr->sval = k_ptr->sval;


    /* Hack -- notice "broken" items */
    if (k_ptr->cost <= 0) i_ptr->ident |= ID_BROKEN;

    /* Hack -- assume "cursed" items */
    if (older_than(2,7,9)) {

        /* Hack -- assume cursed */
        if (k_ptr->flags3 & TR3_CURSED) i_ptr->ident |= ID_CURSED;
        
        /* Hack -- apply "uncursed" incription */
        if (streq(note, "uncursed")) i_ptr->ident &= ~ID_CURSED;
    }


    /* Hack -- the "gold" values changed in 2.7.8 */
    if (older_than(2,7,8) && (i_ptr->tval == TV_GOLD)) {

        /* Extract the value */
        i_ptr->pval = old_cost;

        /* Done */
        return;
    }


    /* Repair non "wearable" items */
    if (!wearable_p(i_ptr)) {

        /* Acquire correct fields */
        i_ptr->to_h = k_ptr->to_h;
        i_ptr->to_d = k_ptr->to_d;
        i_ptr->to_a = k_ptr->to_a;

        /* Acquire correct fields */
        i_ptr->ac = k_ptr->ac;
        i_ptr->dd = k_ptr->dd;
        i_ptr->ds = k_ptr->ds;

        /* Acquire correct weight */
        i_ptr->weight = k_ptr->weight;

        /* Paranoia */
        i_ptr->name1 = i_ptr->name2 = 0;

        /* All done */
        return;
    }


    /* Extract the flags */
    inven_flags(i_ptr, &f1, &f2, &f3);
    
    /* The ego item indexes changed in 2.7.9 */
    if (older_than(2,7,9) && i_ptr->name2) {

        /* Convert the ego-item names */
        i_ptr->name2 = convert_ego_item[i_ptr->name2];

        /* Hack -- fix some "Ammo" */
        if ((i_ptr->tval == TV_BOLT) ||
            (i_ptr->tval == TV_ARROW) ||
            (i_ptr->tval == TV_SHOT)) {

            /* Special ego-item indexes */
            if (i_ptr->name2 == EGO_BRAND_FIRE) {
                i_ptr->name2 = EGO_FLAME;
            }
            else if (i_ptr->name2 == EGO_SLAYING) {
                i_ptr->name2 = EGO_FROST;
            }
            else if (i_ptr->name2 == EGO_SLAY_ANIMAL) {
                i_ptr->name2 = EGO_HURT_ANIMAL;
            }
            else if (i_ptr->name2 == EGO_SLAY_EVIL) {
                i_ptr->name2 = EGO_HURT_EVIL;
            }
            else if (i_ptr->name2 == EGO_SLAY_DRAGON) {
                i_ptr->name2 = EGO_HURT_DRAGON;
            }
        }

        /* Hack -- fix some "Bows" */
        if (i_ptr->tval == TV_BOW) {

            /* Special ego-item indexes */
            if (i_ptr->name2 == EGO_MIGHT) {
                i_ptr->name2 = EGO_VELOCITY;
            }
        }

        /* Hack -- fix some "Robes" */
        if (i_ptr->tval == TV_SOFT_ARMOR) {

            /* Special ego-item indexes */
            if (i_ptr->name2 == EGO_MAGI) {
                i_ptr->name2 = EGO_PERMANENCE;
            }
        }

        /* Hack -- fix some "Boots" */
        if (i_ptr->tval == TV_BOOTS) {

            /* Special ego-item indexes */
            if (i_ptr->name2 == EGO_STEALTH) {
                i_ptr->name2 = EGO_QUIET;
            }
            else if (i_ptr->name2 == EGO_FREE_ACTION) {
                i_ptr->name2 = EGO_MOTION;
            }
        }

        /* Hack -- fix some "Shields" */
        if (i_ptr->tval == TV_SHIELD) {

            /* Special ego-item indexes */
            if (i_ptr->name2 == EGO_RESIST_ACID) {
                i_ptr->name2 = EGO_ENDURE_ACID;
            }
            else if (i_ptr->name2 == EGO_RESIST_ELEC) {
                i_ptr->name2 = EGO_ENDURE_ELEC;
            }
            else if (i_ptr->name2 == EGO_RESIST_FIRE) {
                i_ptr->name2 = EGO_ENDURE_FIRE;
            }
            else if (i_ptr->name2 == EGO_RESIST_COLD) {
                i_ptr->name2 = EGO_ENDURE_COLD;
            }
            else if (i_ptr->name2 == EGO_RESISTANCE) {
                i_ptr->name2 = EGO_ENDURANCE;
            }
            else if (i_ptr->name2 == EGO_ELVENKIND) {
                i_ptr->name2 = EGO_ENDURANCE;
            }
        }
    }
    
    /* Hack -- the "searching" bonuses changed in 2.7.6 */
    if (older_than(2,7,6)) {

        /* Reduce the "pval" bonus on "search" */
        if (f1 & TR1_SEARCH) {

            /* Paranoia -- do not lose any search bonuses */
            i_ptr->pval = (i_ptr->pval + 4) / 5;
        }
    }

    /* Paranoia */
    if (i_ptr->name1) {

        artifact_type *a_ptr;
        
        /* Obtain the artifact info */
        a_ptr = &a_info[i_ptr->name1];
        
        /* Verify that artifact */
        if (!a_ptr->name) i_ptr->name1 = 0;
    }

    /* Paranoia */
    if (i_ptr->name2) {

        ego_item_type *e_ptr;
        
        /* Obtain the ego-item info */
        e_ptr = &e_info[i_ptr->name2];
        
        /* Verify that ego-item */
        if (!e_ptr->name) i_ptr->name2 = 0;
    }


    /* Acquire standard fields */
    i_ptr->ac = k_ptr->ac;
    i_ptr->dd = k_ptr->dd;
    i_ptr->ds = k_ptr->ds;

    /* Acquire standard weight */
    i_ptr->weight = k_ptr->weight;
    
    /* Acquire standard pval (if needed) */
    if (!i_ptr->pval) i_ptr->pval = k_ptr->pval;


    /* Artifacts */
    if (i_ptr->name1) {

        artifact_type *a_ptr;
        
        /* Obtain the artifact info */
        a_ptr = &a_info[i_ptr->name1];

        /* Acquire new artifact "pval" */
        i_ptr->pval = a_ptr->pval;

        /* Acquire new artifact fields */
        i_ptr->ac = a_ptr->ac;
        i_ptr->dd = a_ptr->dd;
        i_ptr->ds = a_ptr->ds;
        
        /* Acquire new artifact weight */
        i_ptr->weight = a_ptr->weight;

        /* Hack -- assume "curse" */
        if (older_than(2,7,9)) {

            /* Hack -- assume cursed */
            if (a_ptr->flags3 & TR3_CURSED) i_ptr->ident |= ID_CURSED;
        }
    }

    /* Ego items */
    if (i_ptr->name2) {

        ego_item_type *e_ptr;
        
        /* Obtain the ego-item info */
        e_ptr = &e_info[i_ptr->name2];

        /* Hack -- keep some old fields */
        if ((i_ptr->dd < old_dd) && (i_ptr->ds == old_ds)) {

            /* Keep old boosted damage dice */        
            i_ptr->dd = old_dd;
        }

        /* Hack -- assume "curse" */
        if (older_than(2,7,9)) {

            /* Hack -- assume cursed */
            if (e_ptr->flags3 & TR3_CURSED) i_ptr->ident |= ID_CURSED;
        }
    }
}




/*
 * Read a monster
 */
static void rd_monster(monster_type *m_ptr)
{
    byte tmp8u;

    /* Read the monster race */
    rd_s16b(&m_ptr->r_idx);

    /* Read the other information */
    rd_byte(&m_ptr->fy);
    rd_byte(&m_ptr->fx);
    rd_s16b(&m_ptr->hp);
    rd_s16b(&m_ptr->maxhp);
    rd_s16b(&m_ptr->csleep);
    rd_byte(&m_ptr->mspeed);
    rd_byte(&m_ptr->energy);
    rd_byte(&m_ptr->stunned);
    rd_byte(&m_ptr->confused);
    rd_byte(&m_ptr->monfear);
    rd_byte(&tmp8u);
}





/*
 * Read the monster lore
 */
static void rd_lore(int r_idx)
{
    byte tmp8u;

    monster_race *r_ptr = &r_info[r_idx];


    /* Pre-2.7.7 */
    if (older_than(2,7,7)) {

        /* Strip old flags */
        strip_bytes(20);

        /* Kills during this life */
        rd_s16b(&r_ptr->r_pkills);

        /* Strip something */
        strip_bytes(2);

        /* Count observations of attacks */
        rd_byte(&r_ptr->r_blows[0]);
        rd_byte(&r_ptr->r_blows[1]);
        rd_byte(&r_ptr->r_blows[2]);
        rd_byte(&r_ptr->r_blows[3]);

        /* Count some other stuff */
        rd_byte(&r_ptr->r_wake);
        rd_byte(&r_ptr->r_ignore);

        /* Strip something */
        strip_bytes(2);

        /* Count kills by player */
        rd_s16b(&r_ptr->r_tkills);

        /* Count deaths of player */
        rd_s16b(&r_ptr->r_deaths);

        /* Read the "Racial" monster limit per level */
        rd_byte(&r_ptr->max_num);

        /* Strip something */
        strip_bytes(1);

        /* Hack -- guess at "sights" */
        r_ptr->r_sights = MAX(r_ptr->r_tkills, r_ptr->r_deaths);
    }

    /* Current */
    else {

        /* Count sights/deaths/kills */
        rd_s16b(&r_ptr->r_sights);
        rd_s16b(&r_ptr->r_deaths);
        rd_s16b(&r_ptr->r_pkills);
        rd_s16b(&r_ptr->r_tkills);

        /* Count wakes and ignores */
        rd_byte(&r_ptr->r_wake);
        rd_byte(&r_ptr->r_ignore);

        /* Extra stuff */
        rd_byte(&r_ptr->r_xtra1);
        rd_byte(&r_ptr->r_xtra2);

        /* Count drops */
        rd_byte(&r_ptr->r_drop_gold);
        rd_byte(&r_ptr->r_drop_item);

        /* Count spells */
        rd_byte(&r_ptr->r_cast_inate);
        rd_byte(&r_ptr->r_cast_spell);

        /* Count blows of each type */
        rd_byte(&r_ptr->r_blows[0]);
        rd_byte(&r_ptr->r_blows[1]);
        rd_byte(&r_ptr->r_blows[2]);
        rd_byte(&r_ptr->r_blows[3]);

        /* Memorize flags */
        rd_u32b(&r_ptr->r_flags1);
        rd_u32b(&r_ptr->r_flags2);
        rd_u32b(&r_ptr->r_flags3);
        rd_u32b(&r_ptr->r_flags4);
        rd_u32b(&r_ptr->r_flags5);
        rd_u32b(&r_ptr->r_flags6);


        /* Read the "Racial" monster limit per level */
        rd_byte(&r_ptr->max_num);

        /* Later (?) */
        rd_byte(&tmp8u);
        rd_byte(&tmp8u);
        rd_byte(&tmp8u);
    }
    
    /* Repair the lore flags */
    r_ptr->r_flags1 &= r_ptr->flags1;
    r_ptr->r_flags2 &= r_ptr->flags2;
    r_ptr->r_flags3 &= r_ptr->flags3;
    r_ptr->r_flags4 &= r_ptr->flags4;
    r_ptr->r_flags5 &= r_ptr->flags5;
    r_ptr->r_flags6 &= r_ptr->flags6;
}




/*
 * Read a store
 */
static errr rd_store(int n)
{
    store_type *st_ptr = &store[n];

    int j;

    byte own, num;

    /* Read the basic info */
    rd_s32b(&st_ptr->store_open);
    rd_s16b(&st_ptr->insult_cur);
    rd_byte(&own);
    rd_byte(&num);
    rd_s16b(&st_ptr->good_buy);
    rd_s16b(&st_ptr->bad_buy);

    /* Extract the owner (see above) */
    st_ptr->owner = (older_than(2,7,8) ? convert_owner[own] : own);

    /* Read the items */
    for (j = 0; j < num; j++) {

        inven_type forge;

        /* Read the item */
        rd_item(&forge);

        /* Acquire valid items */
        if (st_ptr->stock_num < STORE_INVEN_MAX) {

            /* Acquire the item */
            st_ptr->stock[st_ptr->stock_num++] = forge;
        }
    }

    /* Success */
    return (0);
}





/*
 * Read options (ignore most pre-2.7.9 options)
 *
 * XXX XXX XXX Angband 2.8.0 will not only throw out all pre-2.8.0
 * options, and will save at least 8*32 option flags, but will also
 * save an "option mask" indicating which option flags are legal.
 * This will allow easy "adapting" to older savefiles.
 *
 * We should also make the "cheating" options official flags, and
 * move the "byte" options to a different part of the code, perhaps
 * with a few more (for variety).
 *
 * We should perhaps attempt to parse a few of the pre-2.8.0 flags,
 * such as "rogue_like_commands" or "use_color".  Or not...
 *
 * XXX XXX XXX Remember to rename all the options for 2.8.0 with
 * nicer names, and to reorganize them in the option flag sets in
 * some more or less intuitive order (perhaps simply in order?).
 *
 * Implement simple "savefile extenders" using some form of "sized"
 * chunks of bytes, with a {size,type,data} format, so everyone can
 * know the size, interested people can know the type, and the actual
 * data is available to the parsing routines that acknowledge the type.
 *
 * Consider changing the "globe of invulnerability" code so that it
 * takes some form of "maximum damage to protect from" in addition to
 * the existing "number of turns to protect for", and where each hit
 * by a monster will reduce the shield by that amount.
 */
static void rd_options(void)
{
    int i;

    byte b;

    u16b c;

    u32b opt[4];


    /*** Normal options ***/

    /* Read the options */
    rd_u32b(&opt[0]);
    rd_u32b(&opt[1]);
    rd_u32b(&opt[2]);
    rd_u32b(&opt[3]);


    /*** Special options ***/

    /* Read "delay_spd" */
    rd_byte(&b);
    delay_spd = b;

    /* Read "hitpoint_warn" */
    rd_byte(&b);
    hitpoint_warn = b;


    /*** Cheating options ***/

    rd_u16b(&c);

    if (c & 0x0002) wizard = TRUE;

    cheat_peek = (c & 0x0100) ? TRUE : FALSE;
    cheat_hear = (c & 0x0200) ? TRUE : FALSE;
    cheat_room = (c & 0x0400) ? TRUE : FALSE;
    cheat_xtra = (c & 0x0800) ? TRUE : FALSE;
    cheat_know = (c & 0x1000) ? TRUE : FALSE;
    cheat_live = (c & 0x2000) ? TRUE : FALSE;


    /*** Normal Options ***/

    /* Hack -- ignore pre-2.7.9 options */
    if (older_than(2,7,9)) return;

    /* Analyze the options */
    for (i = 0; options[i].o_desc; i++) {

        int os = options[i].o_set;
        int ob = options[i].o_bit;

        /* Extract a variable setting, if possible */
        if (options[i].o_var && os) {
            (*options[i].o_var) = (opt[os-1] & (1L << ob)) ? TRUE : FALSE;
        }
    }
}





/*
 * Hack -- read the ghost info
 *
 * XXX XXX XXX This is such a nasty hack it hurts.
 */
static void rd_ghost()
{
    int i;

    monster_race *r_ptr = &r_info[MAX_R_IDX-1];

    char *name = (r_name + r_ptr->name);


    /* Pre-2.7.7 ghosts */
    if (older_than(2,7,7)) {

        char buf[64];
        
        /* Read the old name */
        rd_string(buf, 64);

        /* Strip old data */
        strip_bytes(52);
    }

    /* Newer method */
    else {

        /* Read the old name */
        rd_string(name, 64);

        /* Visuals */
        rd_char(&r_ptr->r_char);
        rd_byte(&r_ptr->r_attr);

        /* Level/Rarity */
        rd_byte(&r_ptr->level);
        rd_byte(&r_ptr->rarity);

        /* Misc info */
        rd_byte(&r_ptr->hdice);
        rd_byte(&r_ptr->hside);
        rd_s16b(&r_ptr->ac);
        rd_s16b(&r_ptr->sleep);
        rd_byte(&r_ptr->aaf);
        rd_byte(&r_ptr->speed);

        /* Experience */
        rd_s32b(&r_ptr->mexp);

        /* Extra */
        rd_s16b(&r_ptr->extra);

        /* Frequency */
        rd_byte(&r_ptr->freq_inate);
        rd_byte(&r_ptr->freq_spell);

        /* Flags */
        rd_u32b(&r_ptr->flags1);
        rd_u32b(&r_ptr->flags2);
        rd_u32b(&r_ptr->flags3);
        rd_u32b(&r_ptr->flags4);
        rd_u32b(&r_ptr->flags5);
        rd_u32b(&r_ptr->flags6);

        /* Attacks */
        for (i = 0; i < 4; i++) {
            rd_byte(&r_ptr->blow[i].method);
            rd_byte(&r_ptr->blow[i].effect);
            rd_byte(&r_ptr->blow[i].d_dice);
            rd_byte(&r_ptr->blow[i].d_side);
        }
    }


    /* Hack -- set the "graphic" info */
    r_ptr->l_attr = r_ptr->r_attr;
    r_ptr->l_char = r_ptr->r_char;
}




/*
 * Read/Write the "extra" information
 */

static void rd_extra()
{
    int i;

    byte tmp8u;

    rd_string(player_name, 32);

    rd_string(died_from, 80);

    for (i = 0; i < 4; i++) {
        rd_string(history[i], 60);
    }

    /* Class/Race/Gender/Spells */
    rd_byte(&p_ptr->prace);
    rd_byte(&p_ptr->pclass);
    rd_byte(&p_ptr->male);
    rd_byte(&tmp8u);	/* oops */

    /* Special Race/Class info */
    rd_byte(&p_ptr->hitdie);
    rd_byte(&p_ptr->expfact);

    /* Age/Height/Weight */
    rd_s16b(&p_ptr->age);
    rd_s16b(&p_ptr->ht);
    rd_s16b(&p_ptr->wt);

    /* Read the stat info */
    for (i = 0; i < 6; i++) rd_s16b(&p_ptr->stat_max[i]);
    for (i = 0; i < 6; i++) rd_s16b(&p_ptr->stat_cur[i]);

    strip_bytes(24);	/* oops */

    rd_s32b(&p_ptr->au);

    rd_s32b(&p_ptr->max_exp);
    rd_s32b(&p_ptr->exp);
    rd_u16b(&p_ptr->exp_frac);

    rd_s16b(&p_ptr->lev);

    rd_s16b(&p_ptr->mhp);
    rd_s16b(&p_ptr->chp);
    rd_u16b(&p_ptr->chp_frac);

    rd_s16b(&p_ptr->msp);
    rd_s16b(&p_ptr->csp);
    rd_u16b(&p_ptr->csp_frac);

    rd_s16b(&p_ptr->max_plv);
    rd_s16b(&p_ptr->max_dlv);

    /* More info */
    strip_bytes(8);
    rd_s16b(&p_ptr->sc);
    strip_bytes(2);

    /* Ignore old redundant info */
    if (older_than(2,7,7)) strip_bytes(24);

    /* Read the flags */
    strip_bytes(2);	/* Old "rest" */
    rd_s16b(&p_ptr->blind);
    rd_s16b(&p_ptr->paralyzed);
    rd_s16b(&p_ptr->confused);
    rd_s16b(&p_ptr->food);
    strip_bytes(4);	/* Old "food_digested" / "protection" */
    rd_s16b(&p_ptr->energy);
    rd_s16b(&p_ptr->fast);
    rd_s16b(&p_ptr->slow);
    rd_s16b(&p_ptr->afraid);
    rd_s16b(&p_ptr->cut);
    rd_s16b(&p_ptr->stun);
    rd_s16b(&p_ptr->poisoned);
    rd_s16b(&p_ptr->image);
    rd_s16b(&p_ptr->protevil);
    rd_s16b(&p_ptr->invuln);
    rd_s16b(&p_ptr->hero);
    rd_s16b(&p_ptr->shero);
    rd_s16b(&p_ptr->shield);
    rd_s16b(&p_ptr->blessed);
    rd_s16b(&p_ptr->tim_invis);
    rd_s16b(&p_ptr->word_recall);
    rd_s16b(&p_ptr->see_infra);
    rd_s16b(&p_ptr->tim_infra);
    rd_s16b(&p_ptr->oppose_fire);
    rd_s16b(&p_ptr->oppose_cold);
    rd_s16b(&p_ptr->oppose_acid);
    rd_s16b(&p_ptr->oppose_elec);
    rd_s16b(&p_ptr->oppose_pois);

    /* Old redundant flags */
    if (older_than(2,7,7)) strip_bytes(34);

    rd_byte(&p_ptr->confusing);
    rd_byte(&tmp8u);	/* oops */
    rd_byte(&tmp8u);	/* oops */
    rd_byte(&tmp8u);	/* oops */
    rd_byte(&p_ptr->searching);
    rd_byte(&p_ptr->maximize);
    rd_byte(&p_ptr->preserve);
    rd_byte(&tmp8u);

    /* Future use */
    for (i = 0; i < 48; i++) rd_byte(&tmp8u);

    /* Skip the flags */
    strip_bytes(12);


    /* Hack -- the two "special seeds" */
    rd_u32b(&seed_flavor);
    rd_u32b(&seed_town);


    /* Special stuff */
    rd_u16b(&panic_save);
    rd_u16b(&total_winner);
    rd_u16b(&noscore);


    /* Important -- Read "death" */
    rd_byte(&tmp8u);
    death = tmp8u;

    /* Read "feeling" */
    rd_byte(&tmp8u);
    feeling = tmp8u;

    /* Turn of last "feeling" */
    rd_s32b(&old_turn);

    /* Current turn */
    rd_s32b(&turn);
}




/*
 * Read the player inventory
 *
 * Note that the inventory changed in Angband 2.7.4.  Two extra
 * pack slots were added and the equipment was rearranged.  Note
 * that these two features combine when parsing old save-files, in
 * which items from the old "aux" slot are "carried", perhaps into
 * one of the two new "inventory" slots.
 *
 * Note that the inventory is "re-sorted" later by "dungeon()".
 */
static errr rd_inventory()
{
    int slot = 0;

    inven_type forge;

    /* No weight */
    total_weight = 0;

    /* No items */
    inven_cnt = 0;
    equip_cnt = 0;

    /* Read until done */
    while (1) {

        u16b n;

        /* Get the next item index */
        rd_u16b(&n);

        /* Nope, we reached the end */
        if (n == 0xFFFF) break;

        /* Read the item */
        rd_item(&forge);

        /* Hack -- verify item */
        if (!forge.k_idx) return (53);

        /* Hack -- convert old slot numbers */
        if (older_than(2,7,4)) n = convert_slot(n);

        /* Wield equipment */
        if (n >= INVEN_WIELD) {

            /* Structure copy */
            inventory[n] = forge;

            /* Add the weight */
            total_weight += (forge.number * forge.weight);

            /* One more item */
            equip_cnt++;
        }

        /* Warning -- backpack is full */
        else if (inven_cnt == INVEN_PACK) {

            /* Oops */
            note("Too many items in the inventory!");

            /* Fail */
            return (54);
        }

        /* Carry inventory */
        else {

            /* Get a slot */
            n = slot++;
            
            /* Structure copy */
            inventory[n] = forge;

            /* Add the weight */
            total_weight += (forge.number * forge.weight);

            /* One more item */
            inven_cnt++;
        }
    }

    /* Success */
    return (0);
}



/*
 * Read the saved messages
 */
static void rd_messages()
{
    int i;
    char buf[128];

    s16b num;

    /* Hack -- old method used circular queue */
    rd_s16b(&num);

    /* Read the messages */
    for (i = 0; i < num; i++) {

        /* Read the message */
        rd_string(buf, 128);

        /* Save the message */
        message_add(buf);
    }
}


/*
 * Read the dungeon (new method)
 *
 * XXX XXX XXX Angband 2.8.0 will totally change the dungeon info
 *
 * XXX XXX XXX Try to be more flexible about "too many monsters"
 *
 * XXX XXX XXX Mega-Hack -- attempt to convert pre-2.8.0 savefile
 * format into 2.8.0 internal format, by extracting the new cave
 * grid terrain feature flags.  Note that we may have to move the
 * terrain feature extractors into the "rd_item()" function.
 */
static errr rd_dungeon()
{
    int i, y, x;
    byte count;
    byte ychar, xchar;
    byte tmp8u;
    u16b tmp16u;
    int ymax, xmax;
    int total_count;

    cave_type *c_ptr;

    inven_type *i_ptr;


    /* Header info */
    rd_s16b(&dun_level);
    rd_s16b(&num_repro);
    rd_s16b(&py);
    rd_s16b(&px);
    rd_s16b(&cur_hgt);
    rd_s16b(&cur_wid);
    rd_s16b(&max_panel_rows);
    rd_s16b(&max_panel_cols);

    /* Only read as necessary */
    ymax = cur_hgt;
    xmax = cur_wid;

    /* Read in the actual "cave" data */
    total_count = 0;
    xchar = ychar = 0;

    /* Read until done */
    while (total_count < ymax * xmax) {

        /* Extract some RLE info */
        rd_byte(&count);
        rd_byte(&tmp8u);

        /* Apply the RLE info */
        for (i = count; i > 0; i--) {

            /* Prevent over-run */
            if ((ychar >= ymax) || (xchar >= xmax)) {
                note("Illegal location!");
                return (81);
            }

            /* Access the cave */
            c_ptr = &cave[ychar][xchar];

            /* Hack -- Clear all the flags */
            c_ptr->feat = 0x0000;

            /* Old method */
            if (older_than(2,7,5)) {

                /* Extract the old "info" flags */
                if ((tmp8u >> 4) & 0x1) c_ptr->feat |= CAVE_ROOM;
                if ((tmp8u >> 5) & 0x1) c_ptr->feat |= CAVE_MARK;
                if ((tmp8u >> 6) & 0x1) c_ptr->feat |= CAVE_GLOW;

                /* Hack -- process old style "light" */
                if (c_ptr->feat & CAVE_GLOW) {
                    c_ptr->feat |= CAVE_MARK;
                }

                /* Mega-Hack -- light all walls */
                else if ((tmp8u & 0x0F) >= 12) {
                    c_ptr->feat |= CAVE_GLOW;
                }

                /* Process the "floor type" */
                switch (tmp8u & 0x0F) {

                    /* Lite Room Floor */
                    case 2:
                        c_ptr->feat |= CAVE_GLOW;

                    /* Dark Room Floor */
                    case 1:
                        c_ptr->feat |= CAVE_ROOM;
                        break;

                    /* Lite Vault Floor */
                    case 4:
                        c_ptr->feat |= CAVE_GLOW;

                    /* Dark Vault Floor */
                    case 3:
                        c_ptr->feat |= CAVE_ROOM;
                        c_ptr->feat |= CAVE_ICKY;
                        break;

                    /* Corridor Floor */
                    case 5:
                        break;

                    /* Perma-wall (assume "solid") */
                    case 15:
                        c_ptr->feat |= 0x3F;
                        break;

                    /* Granite wall (assume "basic") */
                    case 12:
                        c_ptr->feat |= 0x38;
                        break;

                    /* Quartz vein */
                    case 13:
                        c_ptr->feat |= 0x33;
                        break;

                    /* Magma vein */
                    case 14:
                        c_ptr->feat |= 0x32;
                        break;
                }
            }

            /* Newer method */
            else {

                /* The old "vault" flag */
                if (tmp8u & OLD_GRID_ICKY) c_ptr->feat |= CAVE_ICKY;

                /* The old "room" flag */
                if (tmp8u & OLD_GRID_ROOM) c_ptr->feat |= CAVE_ROOM;

                /* The old "glow" flag */
                if (tmp8u & OLD_GRID_GLOW) c_ptr->feat |= CAVE_GLOW;

                /* The old "mark" flag */
                if (tmp8u & OLD_GRID_MARK) c_ptr->feat |= CAVE_MARK;

                /* The old "wall" flags -- granite wall */
                if ((tmp8u & OLD_GRID_WALL_MASK) ==
                    OLD_GRID_WALL_GRANITE) {

                    /* Permanent wall (assume "solid") */
                    if (tmp8u & OLD_GRID_PERM) c_ptr->feat |= 0x3F;

                    /* Normal wall (assume "basic") */
                    else c_ptr->feat |= 0x38;
                }

                /* The old "wall" flags -- quartz vein */
                else if ((tmp8u & OLD_GRID_WALL_MASK) ==
                         OLD_GRID_WALL_QUARTZ) {

                    /* Assume no treasure */
                    c_ptr->feat |= 0x33;
                }
                
                /* The old "wall" flags -- magma vein */
                else if ((tmp8u & OLD_GRID_WALL_MASK) ==
                         OLD_GRID_WALL_MAGMA) {

                    /* Assume no treasure */
                    c_ptr->feat |= 0x32;
                }
            }

            /* Advance the cave pointers */
            xchar++;

            /* Wrap to the next line */
            if (xchar >= xmax) {
                xchar = 0;
                ychar++;
            }
        }

        /* Advance the count */
        total_count += count;
    }


    /* Read the item count */
    rd_u16b(&tmp16u);

    /* Read the dungeon items */
    for (i = 1; i < tmp16u; i++) {

        int i_idx;
        int k = 0;
        
        inven_type forge;
        
        /* Point at it */
        i_ptr = &forge;
        
        /* Read the item */
        rd_item(i_ptr);

        /* Access the item location */
        c_ptr = &cave[i_ptr->iy][i_ptr->ix];

        /* Skip dead objects */
        if (!i_ptr->k_idx) continue;

        /* Hack -- convert old "dungeon" objects */
        if ((i_ptr->k_idx >= 445) && (i_ptr->k_idx <= 479)) {

            bool invis = FALSE;
            
            /* Hack -- catch "invisible traps" */
            if (i_ptr->tval == 101) invis = TRUE;
            
            /* Analyze the "dungeon objects" */
            switch (i_ptr->k_idx) {
            
                /* Rubble */
                case 445:
                    k = 0x31;
                    break;
                    
                /* Open Door */
                case 446:

                    /* Broken door */
                    if (i_ptr->pval) {
                        k = 0x05;
                    }
                    
                    /* Open door */
                    else {
                        k = 0x04;
                    }

                    break;
                    
                /* Closed Door */
                case 447:

                    /* Jammed door */
                    if (i_ptr->pval < 0) {
                        k = (0 - i_ptr->pval) / 2;
                        if (k > 7) k = 7;
                        k = 0x28 + k;
                    }

                    /* Locked door */
                    else {
                        k = i_ptr->pval / 2;
                        if (k > 7) k = 7;
                        k = 0x20 + k;
                    }

                    break;
                    
                /* Secret Door */
                case 448:
                    k = 0x30;
                    break;
                    
                /* Up Stairs */
                case 449:
                    k = 0x06;
                    break;
                    
                /* Down Stairs */
                case 450:
                    k = 0x07;
                    break;

                /* Store '1' */
                case 451:
                    k = 0x08;
                    break;

                /* Store '2' */
                case 452:
                    k = 0x09;
                    break;
                    
                /* Store '3' */
                case 453:
                    k = 0x0A;
                    break;
                    
                /* Store '4' */
                case 454:
                    k = 0x0B;
                    break;
                    
                /* Store '5' */
                case 455:
                    k = 0x0C;
                    break;
                    
                /* Store '6' */
                case 456:
                    k = 0x0D;
                    break;
                    
                /* Store '7' */
                case 457:
                    k = 0x0E;
                    break;
                    
                /* Store '8' */
                case 458:
                    k = 0x0F;
                    break;
                    
                /* Glyph of Warding */
                case 459:
                    k = 0x03;
                    break;
                    
                /* Trap -- Pit */
                case 460:
                    k = 0x11;
                    break;

                /* Trap -- Spiked Pit */
                case 461:
                    k = 0x12;
                    break;

                /* Trap -- Trap Door */
                case 462:
                    k = 0x10;
                    break;

                /* Trap -- Gas -- Sleep */
                case 463:
                    k = 0x1F;
                    break;

                /* Trap -- Loose rock */
                case 464:
                    k = 0x11;
                    break;

                /* Trap -- Dart -- lose str */
                case 465:
                    k = 0x19;
                    break;

                /* Trap -- Teleport */
                case 466:
                    k = 0x15;
                    break;

                /* Trap -- Falling rock */
                case 467:
                    k = 0x13;
                    break;

                /* Trap -- Dart -- lose dex */
                case 468:
                    k = 0x1A;
                    break;

                /* Trap -- Summoning */
                case 469:
                    k = 0x14;
                    break;

                /* Trap -- Fire */
                case 470:
                    k = 0x16;
                    break;
                    
                /* Trap -- Acid */
                case 471:
                    k = 0x17;
                    break;
                    
                /* Trap -- Gas -- poison */
                case 472:
                    k = 0x1E;
                    break;
                    
                /* Trap -- Gas -- blind */
                case 473:
                    k = 0x1C;
                    break;
                    
                /* Trap -- Gas -- confuse */
                case 474:
                    k = 0x1D;
                    break;
                    
                /* Trap -- Dart -- slow */
                case 475:
                    k = 0x18;
                    break;
                    
                /* Trap -- Dart -- lose con */
                case 476:
                    k = 0x1B;
                    break;
                    
                /* Trap -- Arrow */
                case 477:
                    k = 0x18;
                    break;
            }                    

            /* Hack -- handle "invisible traps" */
            if (invis) k = 0x02;
        }
        
        /* Hack -- treasure in walls */
        else if (i_ptr->tval == TV_GOLD) {
        
            /* Quartz + treasure */
            if ((c_ptr->feat & 0x3F) == 0x33) k = 0x37;

            /* Magma + treasure */
            if ((c_ptr->feat & 0x3F) == 0x32) k = 0x36;
        }

        /* Hack -- use the feature */
        if (k) {

            /* Set new bits */
            c_ptr->feat = ((c_ptr->feat & ~0x3F) | k);

            /* Skip it */
            continue;
        }

        /* Get a new record */
        i_idx = i_pop();
        
        /* Oops */
        if (!i_idx) {
            note(format("Too many (%d) objects!", i_max));
            return (92);
        }

        /* Acquire place */
        i_ptr = &i_list[i_idx];

        /* Copy the item */
        (*i_ptr) = forge;

        /* Mark the location */
        c_ptr->i_idx = i_idx;
    }


    /* Read the monster count */
    rd_s16b(&m_max);
    
    /* Validate XXX XXX XXX */
    if (m_max > MAX_M_IDX) {
        note(format("Too many (%d) monsters!", m_max));
        return (93);
    }

    /* Slightly older method */
    if (older_than(2,7,7)) {

        /* Read the monsters (starting at record 2) */
        for (i = 2; i < m_max; i++) {

            monster_type *m_ptr;
            monster_race *r_ptr;

            /* Access the monster */
            m_ptr = &m_list[i];

            /* Read the monster */
            rd_monster(m_ptr);


            /* Mega-Hack -- eliminate old urchins and ghosts */
            if ((m_ptr->r_idx <= 0) || (m_ptr->r_idx == MAX_R_IDX-1)) {

                /* Just kill the monster */
                WIPE(m_ptr, monster_type);
            }


            /* Process real monsters */
            if (m_ptr->r_idx) {

                /* Access the location */
                c_ptr = &cave[m_ptr->fy][m_ptr->fx];

                /* Note the location */
                c_ptr->m_idx = i;

                /* Access the lore */
                r_ptr = &r_info[m_ptr->r_idx];

                /* Hack -- Count the monsters */
                r_ptr->cur_num++;

                /* Count the monsters */
                m_cnt++;
            }
        }
    }

    /* Slightly newer method */
    else {

        /* Read the monsters */
        for (i = 1; i < m_max; i++) {

            monster_type *m_ptr;
            monster_race *r_ptr;

            /* Access the monster */
            m_ptr = &m_list[i];

            /* Read the monster */
            rd_monster(m_ptr);

            /* Process real monsters */
            if (m_ptr->r_idx) {
            
                /* Hack -- Access the location */
                c_ptr = &cave[m_ptr->fy][m_ptr->fx];

                /* Mark the location */
                c_ptr->m_idx = i;

                /* Hack -- Access the lore */
                r_ptr = &r_info[m_ptr->r_idx];

                /* Hack -- Count the monsters */
                r_ptr->cur_num++;

                /* Count the monsters */
                m_cnt++;
            }
        }
    }


    /* Hack -- clean up the dungeon */
    for (y = 0; y < cur_hgt; y++) {
        for (x = 0; x < cur_wid; x++) {

            cave_type *c_ptr = &cave[y][x];

            /* Hack -- convert nothing-ness into floors */
            if ((c_ptr->feat & 0x3F) == 0x00) c_ptr->feat |= 0x01;            
        }
    }

    
    /* Success */
    return (0);
}


/*
 * Actually read the savefile
 *
 * Angband 2.8.0 will completely replace this code, see "save.c",
 * though this code will be kept to read pre-2.8.0 savefiles.
 */
static errr rd_savefile()
{
    int i;

    byte tmp8u;
    u16b tmp16u;
    u32b tmp32u;

    char buf[128];


#ifdef VERIFY_CHECKSUMS
    u32b n_x_check, n_v_check;
    u32b o_x_check, o_v_check;
#endif


    /* Get the version info */
    xor_byte = 0;
    rd_byte(&sf_major);
    xor_byte = 0;
    rd_byte(&sf_minor);
    xor_byte = 0;
    rd_byte(&sf_patch);
    xor_byte = 0;
    rd_byte(&xor_byte);


    /* Handle stupidity from Angband 2.4 / 2.5 */
    if ((sf_major == 5) && (sf_minor == 2)) {
        sf_major = 2;
        sf_minor = 5;
    }


    /* Build a message */
    sprintf(buf, "Loading a %d.%d.%d savefile...",
            sf_major, sf_minor, sf_patch);

    /* Display the message */
    note(buf);


    /* Clear the checksums */
    v_check = 0L;
    x_check = 0L;


    /* We cannot load savefiles from Angband 1.0 */
    if (sf_major != VERSION_MAJOR) {

        note("This savefile is from a different version of Angband.");
        return (11);
    }


    /* We cannot load savefiles from newer versions of Angband */
    if ((sf_minor > VERSION_MINOR) ||
        ((sf_minor == VERSION_MINOR) && (sf_patch > VERSION_PATCH))) {

        note("This savefile is from a more recent version of Angband.");
        return (12);
    }


    /* Hack -- parse really old savefiles */
    if (older_than(2,7,0)) {
    
#ifdef ALLOW_OLD_SAVEFILES
        /* Load it */
        return (rd_savefile_old());
#else
        /* Message */
        note("This savefile is from an unsupported version of Angband.");
#endif

        /* Oops */
        return (11);
    }
    

    /* Hack -- Warn about "obsolete" versions */
    if (older_than(2,7,4)) {
        note("Warning -- converting obsolete save file.");
    }

    
    /* Operating system info */
    rd_u32b(&sf_xtra);

    /* Time of savefile creation */
    rd_u32b(&sf_when);

    /* Number of resurrections */
    rd_u16b(&sf_lives);

    /* Number of times played */
    rd_u16b(&sf_saves);


    /* Later use (always zero) */
    rd_u32b(&tmp32u);

    /* Later use (always zero) */
    rd_u32b(&tmp32u);


    /* Then the options */
    rd_options();
    if (arg_fiddle) note("Loaded Option Flags");


    /* Then the "messages" */
    rd_messages();
    if (arg_fiddle) note("Loaded Messages");


    /* Monster Memory */
    rd_u16b(&tmp16u);

    /* Incompatible save files */
    if (tmp16u > MAX_R_IDX) {
        note(format("Too many (%u) monster races!", tmp16u));
        return (21);
    }

    /* Read the available records */
    for (i = 0; i < tmp16u; i++) {

        monster_race *r_ptr;
        
        /* Read the lore */
        rd_lore(i);

        /* Access that monster */
        r_ptr = &r_info[i];
        
        /* XXX XXX Hack -- repair old savefiles */
        if (older_than(2,7,6)) {

            /* Assume no kills */
            r_ptr->r_pkills = 0;

            /* Hack -- no previous lives */
            if (sf_lives == 0) {

                /* All kills by this life */
                r_ptr->r_pkills = r_ptr->r_tkills;
            }

            /* Hack -- handle uniques */
            if (r_ptr->flags1 & RF1_UNIQUE) {

                /* Assume no kills */
                r_ptr->r_pkills = 0;

                /* Handle dead uniques */
                if (r_ptr->max_num == 0) r_ptr->r_pkills = 1;
            }
        }
    }
    if (arg_fiddle) note("Loaded Monster Memory");


    /* Object Memory */
    rd_u16b(&tmp16u);

    /* Incompatible save files */
    if (tmp16u > MAX_K_IDX) {
        note(format("Too many (%u) object kinds!", tmp16u));
        return (22);
    }

    /* Read the object memory */
    for (i = 0; i < tmp16u; i++) {

        byte tmp8u;

        inven_kind *k_ptr = &k_info[i];

        rd_byte(&tmp8u);

        k_ptr->aware = (tmp8u & 0x01) ? TRUE: FALSE;
        k_ptr->tried = (tmp8u & 0x02) ? TRUE: FALSE;
    }
    if (arg_fiddle) note("Loaded Object Memory");


    /* Load the Quests */
    rd_u16b(&tmp16u);

    /* Incompatible save files */
    if (tmp16u > 4) {
        note(format("Too many (%u) quests!", tmp16u));
        return (23);
    }

    /* Load the Quests */
    for (i = 0; i < tmp16u; i++) {
        rd_byte(&tmp8u);
        q_list[i].level = tmp8u;
        rd_byte(&tmp8u);
        rd_byte(&tmp8u);
        rd_byte(&tmp8u);
    }
    if (arg_fiddle) note("Loaded Quests");


    /* Load the Artifacts */
    rd_u16b(&tmp16u);

    /* Incompatible save files */
    if (tmp16u > MAX_A_IDX) {
        note(format("Too many (%u) artifacts!", tmp16u));
        return (24);
    }

    /* Read the artifact flags */
    for (i = 0; i < tmp16u; i++) {
        rd_byte(&tmp8u);
        a_info[i].cur_num = tmp8u;
        rd_byte(&tmp8u);
        rd_byte(&tmp8u);
        rd_byte(&tmp8u);
    }
    if (arg_fiddle) note("Loaded Artifacts");


    /* Read the extra stuff */
    rd_extra();
    if (arg_fiddle) note("Loaded extra information");


    /* Read the player_hp array */
    rd_u16b(&tmp16u);

    /* Incompatible save files */
    if (tmp16u > PY_MAX_LEVEL) {
        note(format("Too many (%u) hitpoint entries!", tmp16u));
        return (25);
    }

    /* Read the player_hp array */
    for (i = 0; i < tmp16u; i++) {
        rd_s16b(&player_hp[i]);
    }


    /* Important -- Initialize the race/class */
    rp_ptr = &race_info[p_ptr->prace];
    cp_ptr = &class_info[p_ptr->pclass];

    /* Important -- Choose the magic info */
    mp_ptr = &magic_info[p_ptr->pclass];


    /* Read spell info */
    rd_u32b(&spell_learned1);
    rd_u32b(&spell_learned2);
    rd_u32b(&spell_worked1);
    rd_u32b(&spell_worked2);
    rd_u32b(&spell_forgotten1);
    rd_u32b(&spell_forgotten2);

    for (i = 0; i < 64; i++) {
        rd_byte(&spell_order[i]);
    }


    /* Read the inventory */
    if (rd_inventory()) {
        note("Unable to read inventory");
        return (21);
    }


    /* Read the stores */
    rd_u16b(&tmp16u);
    for (i = 0; i < tmp16u; i++) {
        if (rd_store(i)) return (22);
    }


    /* I'm not dead yet... */
    if (!death) {

        /* Dead players have no dungeon */
        note("Restoring Dungeon...");
        if (rd_dungeon()) {
            note("Error reading dungeon data");
            return (34);
        }

        /* Read the ghost info */
        rd_ghost();
    }


#ifdef VERIFY_CHECKSUMS

    /* Save the checksum */
    n_v_check = v_check;

    /* Read the old checksum */
    rd_u32b(&o_v_check);

    /* Verify */
    if (o_v_check != n_v_check) {
        note("Invalid checksum");
        return (11);
    }


    /* Save the encoded checksum */
    n_x_check = x_check;

    /* Read the checksum */
    rd_u32b(&o_x_check);


    /* Verify */
    if (o_x_check != n_x_check) {
        note("Invalid encoded checksum");
        return (11);
    }

#endif


    /* Success */
    return (0);
}







/*
 * Version 2.7.0 introduced a slightly different "savefile" format from
 * older versions, requiring a completely different parsing method.
 *
 * Note that savefiles from 2.7.0 - 2.7.2 were a little flaky, and are
 * thus obsolete, and may or may not be correctly loaded.
 *
 * Some information from pre-2.7.0 savefiles is "lost", such as some
 * of the "saved messages", and all "ghost" information.
 *
 * Some information from pre-2.7.7 savefiles is "lost", including most
 * monster memory, and all player ghosts.
 *
 * Allow restoring a file belonging to someone else, but only if we
 * can delete it.  Hence first try to read without doing a chmod.
 *
 * Note that Macintosh and Windows never attempt to load non-existant
 * savefiles (hopefully!) and do not attempt the "access()" call.
 */
bool load_player(void)
{
    int		fd = -1;
    bool	ok = FALSE;


    /* Verify savefile name */
    if (!savefile[0]) return (FALSE);


#if !defined(MACINTOSH) && !defined(WINDOWS) && \
    !defined(ACORN) && !defined(VM)

    /* Verify the existance of the savefile */
    if (access(savefile, 0) < 0) {

        /* Give a message */
        msg_print("Savefile does not exist.");

        /* Nothing loaded */
        return (FALSE);
    }

#endif


#ifdef VERIFY_SAVEFILE

    /* Verify savefile usage */
    if (TRUE) {

        FILE *fkk;

        char temp[1024];

        /* Extract name of lock file */
        strcpy(temp, savefile);
        strcat(temp, ".lok");

        /* Check for lock */
        fkk = my_fopen(temp, "r");

        /* Oops, lock exists */
        if (fkk) {

            /* Close the file */
            my_fclose(fkk);

            /* Message */
            msg_print("Savefile is currently in use.");

            /* Nothing loaded */
            return (FALSE);
        }

        /* Create a lock file */
        fkk = my_fopen(temp, "w");

        /* Dump a line of info */
        fprintf(fkk, "Lock file for savefile '%s'\n", savefile);

        /* Close the lock file */
        my_fclose(fkk);
    }

#endif


    /* Forbid suspend */
    signals_ignore_tstp();


    /* Flush messages */
    msg_print(NULL);

    /* Notify the player */
    clear_screen();

    /* First note */
    note("Restoring Character.");


    /* Open the BINARY savefile */
    fd = fd_open(savefile, O_RDONLY | O_BINARY, 0);

    /* Process file */
    if (fd >= 0) {

#ifdef VERIFY_TIMESTAMP
        struct stat         statbuf;
#endif

        /* Paranoia */
        turn = 0;

        /* Assume okay */
        ok = TRUE;

#ifdef VERIFY_TIMESTAMP
        /* Get the timestamp */
        (void)fstat(fd, &statbuf);
#endif

        /* Close the file */
        (void)fd_close(fd);


        /* The savefile is a binary file */
        fff = my_fopen(savefile, "rb");

        /* Paranoia */
        if (!fff) goto error;


        /* Actually read the savefile */
        if (rd_savefile()) goto error;


#ifdef VERIFY_TIMESTAMP
        /* Allow cheating */
        if (!arg_wizard) {

            /* Hack -- Verify the timestamp */
            if (sf_when > (statbuf.st_ctime + 100) ||
                sf_when < (statbuf.st_ctime - 100)) {
                note("Invalid Timestamp");
                goto error;
            }
        }
#endif


        /* Check for errors */
        if (ferror(fff)) {
            note("FILE ERROR");
            goto error;
        }


        /* Hack -- notice when the cave is ready */
        if (!death) character_dungeon = TRUE;


        /* Process "dead" players */
        if (death) {

            /* Wizards can revive dead characters */
            if (arg_wizard && get_check("Resurrect a dead character? ")) {

                /* Revive the player */
                note("Attempting a resurrection!");

                /* Not quite dead */
                if (p_ptr->chp < 0) {
                    p_ptr->chp = 0;
                    p_ptr->chp_frac = 0;
                }

                /* Hack -- No starving */
                p_ptr->food = PY_FOOD_FULL - 1;

                /* Hack -- Cure stuff */
                p_ptr->blind = 0;
                p_ptr->confused = 0;
                p_ptr->poisoned = 0;
                p_ptr->image = 0;
                p_ptr->afraid = 0;
                p_ptr->slow = 0;
                p_ptr->cut = 0;
                p_ptr->stun = 0;

                /* Cancel word of recall */
                p_ptr->word_recall = 0;

                /* Hack -- Go back to the town */
                dun_level = 0;

                /* The character exists */
                character_generated = TRUE;

                /* Remember the resurrection */
                noscore |= 0x0001;

                /* Player is no longer "dead" */
                death = FALSE;

                /* Hack -- force legal "turn" */
                if (turn < 1) turn = 1;
            }

            /* Normal "restoration" */
            else {

                note("Restoring Memory of a departed spirit...");

                /* Count the past lives */
                sf_lives++;

                /* Forget the turn, and old_turn */
                turn = old_turn = 0;

                /* Player is no longer "dead" */
                death = FALSE;

                /* Hack -- skip file verification */
                goto closefiles;
            }
        }


        /* Weird error */
        if (!turn) {

            /* Message */
            note("Invalid turn!");

error:

            /* Assume bad data. */
            ok = FALSE;
        }

        /* Normal load */
        else {

            /* Hack -- do not "hurt" the "killed by" string */
            if (p_ptr->chp >= 0) {

                /* Reset cause of death */
                (void)strcpy(died_from, "(alive and well)");
            }

            /* The character exists */
            character_generated = TRUE;
        }

closefiles:

        /* Is there a file? */
        if (fff) {

            /* Try to close it */
            if (my_fclose(fff)) ok = FALSE;
        }


        /* Normal load */
        if (ok) {

            /* A character was loaded */
            character_loaded = TRUE;

            /* Allow suspend again */
            signals_handle_tstp();

            /* Give a warning */
            if ((sf_minor != VERSION_MINOR) ||
                (sf_patch != VERSION_PATCH)) {

                msg_format("Save file from version %d.%d.%d %s game version %d.%d.%d.",
                           sf_major, sf_minor, sf_patch,
                           ((sf_minor == VERSION_MINOR) ?
                            "accepted for" : "converted for"),
                           VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
            }

            /* No living player loaded */
            if (!turn) return (FALSE);

            /* Successful load */
            return (TRUE);
        }


        /* Mention errors */
        msg_print("Error during reading of savefile.");
        msg_print(NULL);
    }

    /* No file */
    else {

        /* Message */
        msg_print("Can't open file for reading.");
    }


    /* Oh well... */
    note("Please try again without that savefile.");

    /* No game in progress */
    turn = 0;

    /* Allow suspend again */
    signals_handle_tstp();

    /* Abort */
    quit("unusable savefile");

    /* Paranoia */
    return (FALSE);
}



