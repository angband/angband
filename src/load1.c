/* File: load1.c */

/*
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"


/*
 * This file handles "old" Angband savefiles (pre-2.7.0)
 *
 * All "old" savefiles lose a lot of information when imported, including
 * all monster memory, some object memory, all non-standard object flags,
 * all player ghosts, and a lot of "map" information.
 *
 * A long time ago, before the official Angband version scheme started,
 * Angband savefiles used the "Moria" version numbers, so a savefile might
 * be marked as "5.2.2".  For consistancy, any savefile marked "5.2.Z" is
 * mentally converted to "2.5.Z", and any other savefile marked "5.Y.Z" is
 * mentally changed to "1.Y.Z" (not that any have been encountered yet).
 *
 * This file will correctly parse all known savefiles, and can be modified
 * to handle previously unknown savefiles with a little effort.
 *
 * There are three special flags to assist in the parsing of extremely old
 * savefiles, "arg_stupid", which allows parsing of certain "compressed"
 * fields, "arg_colour", which allows parsing of some obsolete "colour"
 * info, and "arg_crappy", which induces "maximize" mode, since some old
 * versions used something similar to this special mode.
 *
 * Old "PCAngband 1.4" savefiles need "arg_stupid" and "arg_colour" and
 * "arg_crappy".  Old "MacAngband 1.0" and "MacAngband 2.0.3" savefiles
 * need "arg_stupid" and "arg_colour".  Old "Archimedes Angband" savefiles
 * need "arg_stupid" and "arg_crappy".  These flags are extracted for the
 * savefiles which *might* need them by asking the user to identify the
 * origin of the savefile, if the savefile is extremely obsolete.
 *
 * We attempt to prevent corrupt savefiles from inducing memory errors.
 *
 * Note that this file should not use the random number generator, the
 * object flavors, the visual attr/char mappings, or anything else which
 * is initialized *after* or *during* the "load character" function.
 *
 * This file assumes that many global variables, including the "dungeon"
 * arrays, are initialized to all zeros.
 */


#ifdef ALLOW_OLD_SAVEFILES



/*
 * Handle for the savefile
 */
static FILE *fff;

/*
 * Hack -- simple encryption byte
 */
static byte xor_byte;

/*
 * Hack -- parse old "compressed" fields
 */
static bool arg_stupid;

/*
 * Hack -- parse old "colour" info
 */
static bool arg_colour;

/*
 * Hack -- force "maximize" mode
 */
static bool arg_crappy;



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
 * Hack -- Show information on the screen, one line at a time.
 *
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
static bool wearable_p(const object_type *o_ptr)
{
	/* Valid "tval" codes */
	switch (o_ptr->tval)
	{
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
		{
			return (TRUE);
		}
	}

	/* Nope */
	return (FALSE);
}


/*
 * These functions load the basic building blocks of savefiles.
 */

static byte sf_get(void)
{
	byte c, v;

	/* Get a character, decode the value */
	c = getc(fff) & 0xFF;
	v = c ^ xor_byte;
	xor_byte = c;

	/* Hack */
	if (feof(fff)) v = 0;

	/* Return the value */
	return (v);
}

static void rd_byte(byte *ip)
{
	*ip = sf_get();
}

static void rd_u16b(u16b *ip)
{
	u16b t1, t2;
	t1 = sf_get();
	t2 = sf_get();
	(*ip) = (t1 | (t2 << 8));
}

static void rd_s16b(s16b *ip)
{
	rd_u16b((u16b*)ip);
}

static void rd_u32b(u32b *ip)
{
	u32b t1, t2, t3, t4;
	t1 = sf_get();
	t2 = sf_get();
	t3 = sf_get();
	t4 = sf_get();
	(*ip) = (t1 | (t2 << 8) | (t3 << 16) | (t4 << 24));
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
	for (i = 0; TRUE; i++)
	{
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
	byte tmp8u;

	/* Strip the bytes */
	while (n--) rd_byte(&tmp8u);
}


/*
 * Owner Conversion -- pre-2.7.8 to 2.7.8
 * Shop is column, Owner is Row, see "tables.c"
 */
static const byte convert_owner[24] =
{
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
 * Hack -- convert the old "name2" fields into the new name1/name2 fields.
 *
 * Note that the entries below must map one-to-one onto the old "SN_*"
 * defines, shown in the comments after the new values.
 *
 * Note that this code relies on the fact that there are only 128 ego-items
 * and only 128 artifacts in the new system.
 */
static const byte convert_old_names[180] =
{
	0,						/* 0 = SN_NULL */
	EGO_RESISTANCE,			/* 1 = SN_R */
	EGO_RESIST_ACID,		/* 2 = SN_RA */
	EGO_RESIST_FIRE,		/* 3 = SN_RF */
	EGO_RESIST_COLD,		/* 4 = SN_RC */
	EGO_RESIST_ELEC,		/* 5 = SN_RL */
	EGO_HA,					/* 6 = SN_HA */
	EGO_DF,					/* 7 = SN_DF */
	EGO_SLAY_ANIMAL,		/* 8 = SN_SA */
	EGO_SLAY_DRAGON,		/* 9 = SN_SD */
	EGO_SLAY_EVIL,			/* 10 = SN_SE */
	EGO_SLAY_UNDEAD,		/* 11 = SN_SU */
	EGO_BRAND_FIRE,			/* 12 = SN_FT */
	EGO_BRAND_COLD,			/* 13 = SN_FB */
	EGO_FREE_ACTION,		/* 14 = SN_FREE_ACTION */
	EGO_SLAYING,			/* 15 = SN_SLAYING */
	EGO_CLUMSINESS,			/* 16 = SN_CLUMSINESS */
	EGO_WEAKNESS,			/* 17 = SN_WEAKNESS */
	EGO_SLOW_DESCENT,		/* 18 = SN_SLOW_DESCENT */
	EGO_SPEED,				/* 19 = SN_SPEED */
	EGO_STEALTH,			/* 20 = SN_STEALTH */
	EGO_SLOWNESS,			/* 21 = SN_SLOWNESS */
	EGO_NOISE,				/* 22 = SN_NOISE */
	EGO_ANNOYANCE,			/* 23 = SN_GREAT_MASS */
	EGO_INTELLIGENCE,		/* 24 = SN_INTELLIGENCE */
	EGO_WISDOM,				/* 25 = SN_WISDOM */
	EGO_INFRAVISION,		/* 26 = SN_INFRAVISION */
	EGO_MIGHT,				/* 27 = SN_MIGHT */
	EGO_LORDLINESS,			/* 28 = SN_LORDLINESS */
	EGO_MAGI,				/* 29 = SN_MAGI */
	EGO_BEAUTY,				/* 30 = SN_BEAUTY */
	EGO_SEEING,				/* 31 = SN_SEEING */
	EGO_REGENERATION,		/* 32 = SN_REGENERATION */
	EGO_STUPIDITY,			/* 33 = SN_STUPIDITY */
	EGO_NAIVETY,			/* 34 = SN_DULLNESS */
	0,						/* 35 = SN_BLINDNESS */
	0,						/* 36 = SN_TIMIDNESS */
	0,						/* 37 = SN_TELEPORTATION */
	EGO_UGLINESS,			/* 38 = SN_UGLINESS */
	EGO_PROTECTION,			/* 39 = SN_PROTECTION */
	EGO_IRRITATION,			/* 40 = SN_IRRITATION */
	EGO_VULNERABILITY,		/* 41 = SN_VULNERABILITY */
	EGO_ENVELOPING,			/* 42 = SN_ENVELOPING */
	EGO_BRAND_FIRE,			/* 43 = SN_FIRE */
	EGO_HURT_EVIL,			/* 44 = SN_SLAY_EVIL */
	EGO_HURT_DRAGON,		/* 45 = SN_DRAGON_SLAYING */
	0,						/* 46 = SN_EMPTY */
	0,						/* 47 = SN_LOCKED */
	0,						/* 48 = SN_POISON_NEEDLE */
	0,						/* 49 = SN_GAS_TRAP */
	0,						/* 50 = SN_EXPLOSION_DEVICE */
	0,						/* 51 = SN_SUMMONING_RUNES */
	0,						/* 52 = SN_MULTIPLE_TRAPS */
	0,						/* 53 = SN_DISARMED */
	0,						/* 54 = SN_UNLOCKED */
	EGO_HURT_ANIMAL,		/* 55 = SN_SLAY_ANIMAL */
	ART_GROND + 128,		/* 56 = SN_GROND */
	ART_RINGIL + 128,		/* 57 = SN_RINGIL */
	ART_AEGLOS + 128,		/* 58 = SN_AEGLOS */
	ART_ARUNRUTH + 128,		/* 59 = SN_ARUNRUTH */
	ART_MORMEGIL + 128,		/* 60 = SN_MORMEGIL */
	EGO_MORGUL,				/* 61 = SN_MORGUL */
	ART_ANGRIST + 128,		/* 62 = SN_ANGRIST */
	ART_GURTHANG + 128,		/* 63 = SN_GURTHANG */
	ART_CALRIS + 128,		/* 64 = SN_CALRIS */
	EGO_ACCURACY,			/* 65 = SN_ACCURACY */
	ART_ANDURIL + 128,		/* 66 = SN_ANDURIL */
	EGO_SLAY_ORC,			/* 67 = SN_SO */
	EGO_POWER,				/* 68 = SN_POWER */
	ART_DURIN + 128,		/* 69 = SN_DURIN */
	ART_AULE + 128,			/* 70 = SN_AULE */
	EGO_WEST,				/* 71 = SN_WEST */
	EGO_BLESS_BLADE,		/* 72 = SN_BLESS_BLADE */
	EGO_SLAY_DEMON,			/* 73 = SN_SDEM */
	EGO_SLAY_TROLL,			/* 74 = SN_ST */
	ART_BLOODSPIKE + 128,	/* 75 = SN_BLOODSPIKE */
	ART_THUNDERFIST + 128,	/* 76 = SN_THUNDERFIST */
	EGO_WOUNDING,			/* 77 = SN_WOUNDING */
	ART_ORCRIST + 128,		/* 78 = SN_ORCRIST */
	ART_GLAMDRING + 128,	/* 79 = SN_GLAMDRING */
	ART_STING + 128,		/* 80 = SN_STING */
	EGO_LITE,				/* 81 = SN_LITE */
	EGO_AGILITY,			/* 82 = SN_AGILITY */
	EGO_BACKBITING,			/* 83 = SN_BACKBITING */
	ART_DOOMCALLER + 128,	/* 84 = SN_DOOMCALLER */
	EGO_SLAY_GIANT,			/* 85 = SN_SG */
	EGO_TELEPATHY,			/* 86 = SN_TELEPATHY */
	0,						/* 87 = SN_DRAGONKIND */
	0,						/* 88 = SN_NENYA */
	0,						/* 89 = SN_NARYA */
	0,						/* 90 = SN_VILYA */
	EGO_AMAN,				/* 91 = SN_AMAN */
	ART_BELEGENNON + 128,	/* 92 = SN_BELEGENNON */
	ART_FEANOR + 128,		/* 93 = SN_FEANOR */
	ART_ANARION + 128,		/* 94 = SN_ANARION */
	ART_ISILDUR + 128,		/* 95 = SN_ISILDUR */
	ART_FINGOLFIN + 128,	/* 96 = SN_FINGOLFIN */
	EGO_ELVENKIND,			/* 97 = SN_ELVENKIND */
	ART_SOULKEEPER + 128,	/* 98 = SN_SOULKEEPER */
	ART_DOR + 128,			/* 99 = SN_DOR_LOMIN */
	ART_MORGOTH + 128,		/* 100 = SN_MORGOTH */
	ART_BELTHRONDING + 128,	/* 101 = SN_BELTHRONDING */
	ART_DAL + 128,			/* 102 = SN_DAL */
	ART_PAURHACH + 128,		/* 103 = SN_PAURHACH */
	ART_PAURNIMMEN + 128,	/* 104 = SN_PAURNIMMEN */
	ART_PAURAEGEN + 128,	/* 105 = SN_PAURAEGEN */
	ART_CAMMITHRIM + 128,	/* 106 = SN_CAMMITHRIM */
	ART_CAMBELEG + 128,		/* 107 = SN_CAMBELEG */
	ART_HOLHENNETH + 128,	/* 108 = SN_HOLHENNETH */
	ART_PAURNEN + 128,		/* 109 = SN_PAURNEN */
	ART_AEGLIN + 128,		/* 110 = SN_AEGLIN */
	ART_CAMLOST + 128,		/* 111 = SN_CAMLOST */
	ART_NIMLOTH + 128,		/* 112 = SN_NIMLOTH */
	ART_NAR + 128,			/* 113 = SN_NAR */
	ART_BERUTHIEL + 128,	/* 114 = SN_BERUTHIEL */
	ART_GORLIM + 128,		/* 115 = SN_GORLIM */
	ART_NARTHANC + 128,		/* 116 = SN_NARTHANC */
	ART_NIMTHANC + 128,		/* 117 = SN_NIMTHANC */
	ART_DETHANC + 128,		/* 118 = SN_DETHANC */
	ART_GILETTAR + 128,		/* 119 = SN_GILETTAR */
	ART_RILIA + 128,		/* 120 = SN_RILIA */
	ART_BELANGIL + 128,		/* 121 = SN_BELANGIL */
	ART_BALLI + 128,		/* 122 = SN_BALLI */
	ART_LOTHARANG + 128,	/* 123 = SN_LOTHARANG */
	ART_FIRESTAR + 128,		/* 124 = SN_FIRESTAR */
	ART_ERIRIL + 128,		/* 125 = SN_ERIRIL */
	ART_CUBRAGOL + 128,		/* 126 = SN_CUBRAGOL */
	ART_BARD + 128,			/* 127 = SN_BARD */
	ART_COLLUIN + 128,		/* 128 = SN_COLLUIN */
	ART_HOLCOLLETH + 128,	/* 129 = SN_HOLCOLLETH */
	ART_TOTILA + 128,		/* 130 = SN_TOTILA */
	ART_PAIN + 128,			/* 131 = SN_PAIN */
	ART_ELVAGIL + 128,		/* 132 = SN_ELVAGIL */
	ART_AGLARANG + 128,		/* 133 = SN_AGLARANG */
	ART_ROHIRRIM + 128,		/* 134 = SN_ROHIRRIM */
	ART_EORLINGAS + 128,	/* 135 = SN_EORLINGAS */
	ART_BARUKKHELED + 128,	/* 136 = SN_BARUKKHELED */
	ART_WRATH + 128,		/* 137 = SN_WRATH */
	ART_HARADEKKET + 128,	/* 138 = SN_HARADEKKET */
	ART_MUNDWINE + 128,		/* 139 = SN_MUNDWINE */
	ART_GONDRICAM + 128,	/* 140 = SN_GONDRICAM */
	ART_ZARCUTHRA + 128,	/* 141 = SN_ZARCUTHRA */
	ART_CARETH + 128,		/* 142 = SN_CARETH */
	ART_FORASGIL + 128,		/* 143 = SN_FORASGIL */
	ART_CRISDURIAN + 128,	/* 144 = SN_CRISDURIAN */
	ART_COLANNON + 128,		/* 145 = SN_COLANNON */
	ART_HITHLOMIR + 128,	/* 146 = SN_HITHLOMIR */
	ART_THALKETTOTH + 128,	/* 147 = SN_THALKETTOTH */
	ART_ARVEDUI + 128,		/* 148 = SN_ARVEDUI */
	ART_THRANDUIL + 128,	/* 149 = SN_THRANDUIL */
	ART_THENGEL + 128,		/* 150 = SN_THENGEL */
	ART_HAMMERHAND + 128,	/* 151 = SN_HAMMERHAND */
	ART_CELEGORM + 128,		/* 152 = SN_CELEGORM */
	ART_THROR + 128,		/* 153 = SN_THROR */
	ART_MAEDHROS + 128,		/* 154 = SN_MAEDHROS */
	ART_OLORIN + 128,		/* 155 = SN_OLORIN */
	ART_ANGUIREL + 128,		/* 156 = SN_ANGUIREL */
	ART_THORIN + 128,		/* 157 = SN_THORIN */
	ART_CELEBORN + 128,		/* 158 = SN_CELEBORN */
	ART_OROME + 128,		/* 159 = SN_OROME */
	ART_EONWE + 128,		/* 160 = SN_EONWE */
	ART_GONDOR + 128,		/* 161 = SN_GONDOR */
	ART_THEODEN + 128,		/* 162 = SN_THEODEN */
	ART_THINGOL + 128,		/* 163 = SN_THINGOL */
	ART_THORONGIL + 128,	/* 164 = SN_THORONGIL */
	ART_LUTHIEN + 128,		/* 165 = SN_LUTHIEN */
	ART_TUOR + 128,			/* 166 = SN_TUOR */
	ART_ULMO + 128,			/* 167 = SN_ULMO */
	ART_OSONDIR + 128,		/* 168 = SN_OSONDIR */
	ART_TURMIL + 128,		/* 169 = SN_TURMIL */
	ART_CASPANION + 128,	/* 170 = SN_CASPANION */
	ART_TIL + 128,			/* 171 = SN_TIL */
	ART_DEATHWREAKER + 128,	/* 172 = SN_DEATHWREAKER */
	ART_AVAVIR + 128,		/* 173 = SN_AVAVIR */
	ART_TARATOL + 128,		/* 174 = SN_TARATOL */
	ART_RAZORBACK + 128,	/* 175 = SN_RAZORBACK */
	ART_BLADETURNER + 128,	/* 176 = SN_BLADETURNER */
	0,						/* 177 = SN_SHATTERED */
	0,						/* 178 = SN_BLASTED */
	EGO_ATTACKS				/* 179 = SN_ATTACKS */
};


/*
 * Convert old kinds into normal kinds
 *
 * Note the hard-coded use of the new object kind indexes.  XXX XXX XXX
 */
static const s16b convert_old_kinds_normal[501] =
{
	15,			/* Move: Mushroom of poison */
	1,			/* a Mushroom of Blindness */
	2,			/* a Mushroom of Paranoia */
	3,			/* a Mushroom of Confusion */
	4,			/* a Mushroom of Hallucination */
	5,			/* a Mushroom of Cure Poison */
	6,			/* a Mushroom of Cure Blindness */
	7,			/* a Mushroom of Cure Paranoia */
	8,			/* a Mushroom of Cure Confusion */
	9,			/* a Mushroom of Weakness */
	10,			/* a Mushroom of Unhealth */
	11,			/* a Mushroom of Restore Constitution */
	12,			/* a Mushroom of Restoring */
	12,			/* Move: Mushrooms of restoring (extra) */
	12,			/* Move: Mushrooms of restoring (extra) */
	15,			/* a Mushroom of Poison */
	15,			/* Move: Hairy Mold of poison */
	17,			/* a Mushroom of Paralysis */
	18,			/* a Mushroom of Restore Strength */
	19,			/* a Mushroom of Disease */
	20,			/* a Mushroom of Cure Serious Wounds */
	21,			/* a Ration of Food */
	21,			/* Move: Ration of Food (extra) */
	21,			/* Move: Ration of Food (extra) */
	24,			/* a Slime Mold */
	25,			/* a Piece of Elvish Waybread */
	25,			/* Move: Piece of Elvish Waybread (extra) */
	25,			/* Move: Piece of Elvish Waybread (extra) */
	38,			/* Move: Main Gauche */
	43,			/* Move: Dagger */
	30,			/* a Broken Dagger */
	31,			/* a Bastard Sword */
	32,			/* a Scimitar */
	33,			/* a Tulwar */
	34,			/* a Broad Sword */
	34,			/* Move: Broad-sword */
	36,			/* a Blade of Chaos */
	37,			/* a Two-Handed Sword */
	37,			/* Move: Two-Handed Sword */
	39,			/* a Cutlass */
	40,			/* an Executioner's Sword */
	41,			/* a Katana */
	42,			/* a Long Sword */
	42,			/* Move: Long sword */
	44,			/* a Rapier */
	45,			/* a Sabre */
	46,			/* a Small Sword */
	47,			/* a Broken Sword */
	48,			/* a Ball-and-Chain */
	49,			/* a Whip */
	50,			/* a Flail */
	51,			/* a Two-Handed Flail */
	52,			/* a Morning Star */
	53,			/* a Mace */
	54,			/* a Quarterstaff */
	55,			/* a War Hammer */
	56,			/* a Lead-Filled Mace */
	57,			/* a Mace of Disruption */
	62,			/* Move: Awl-Pike */
	59,			/* a Beaked Axe */
	60,			/* a Glaive */
	61,			/* a Halberd */
	58,			/* Move: Lucern Hammer */
	63,			/* a Pike */
	64,			/* a Spear */
	65,			/* a Trident */
	66,			/* a Lance */
	67,			/* a Great Axe */
	68,			/* a Battle Axe */
	69,			/* a Lochaber Axe */
	70,			/* a Broad Axe */
	71,			/* a Scythe */
	72,			/* a Scythe of Slicing */
	73,			/* a Short Bow */
	74,			/* a Long Bow */
	75,			/* a Light Crossbow */
	76,			/* a Heavy Crossbow */
	77,			/* a Sling */
	78,			/* an Arrow */
	79,			/* a Seeker Arrow */
	80,			/* a Bolt */
	81,			/* a Seeker Bolt */
	82,			/* a Rounded Pebble */
	83,			/* an Iron Shot */
	345,		/* Move: Spike */
	347,		/* Move: Lantern */
	346,		/* Move: Torch */
	88,			/* Move: Orcish Pick */
	89,			/* Move: Dwarven Pick */
	85,			/* Move: Gnomish Shovel */
	86,			/* Move: Dwarven shovel */
	91,			/* a Pair of Soft Leather Boots */
	92,			/* a Pair of Hard Leather Boots */
	93,			/* a Pair of Metal Shod Boots */
	94,			/* a Hard Leather Cap */
	95,			/* a Metal Cap */
	96,			/* an Iron Helm */
	97,			/* a Steel Helm */
	98,			/* an Iron Crown */
	99,			/* a Golden Crown */
	100,		/* a Jewel Encrusted Crown */
	101,		/* a Robe */
	101,		/* Move: Robe */
	103,		/* Soft Leather Armour */
	104,		/* Soft Studded Leather */
	105,		/* Hard Leather Armour */
	106,		/* Hard Studded Leather */
	107,		/* Leather Scale Mail */
	108,		/* Metal Scale Mail */
	109,		/* Chain Mail */
	110,		/* Rusty Chain Mail */
	111,		/* Augmented Chain Mail */
	112,		/* Bar Chain Mail */
	113,		/* Metal Brigandine Armour */
	114,		/* Partial Plate Armour */
	115,		/* Metal Lamellar Armour */
	116,		/* Full Plate Armour */
	117,		/* Ribbed Plate Armour */
	118,		/* Adamantite Plate Mail */
	119,		/* Mithril Plate Mail */
	120,		/* Mithril Chain Mail */
	121,		/* Double Chain Mail */
	122,		/* a Shield of Deflection */
	123,		/* a Cloak */
	124,		/* a Shadow Cloak */
	125,		/* a Set of Leather Gloves */
	126,		/* a Set of Gauntlets */
	127,		/* a Set of Cesti */
	128,		/* a Small Leather Shield */
	129,		/* a Large Leather Shield */
	130,		/* a Small Metal Shield */
	131,		/* a Large Metal Shield */
	132,		/* a Ring of Strength */
	133,		/* a Ring of Dexterity */
	134,		/* a Ring of Constitution */
	135,		/* a Ring of Intelligence */
	136,		/* a Ring of Speed */
	137,		/* a Ring of Searching */
	138,		/* a Ring of Teleportation */
	139,		/* a Ring of Slow Digestion */
	140,		/* a Ring of Resist Fire */
	141,		/* a Ring of Resist Cold */
	142,		/* a Ring of Feather Falling */
	143,		/* a Ring of Poison Resistance */
	78,			/* Move: Arrow */
	145,		/* a Ring of Weakness */
	146,		/* a Ring of Flames */
	147,		/* a Ring of Acid */
	148,		/* a Ring of Ice */
	149,		/* a Ring of Woe */
	150,		/* a Ring of Stupidity */
	151,		/* a Ring of Damage */
	152,		/* a Ring of Accuracy */
	153,		/* a Ring of Protection */
	154,		/* a Ring of Aggravate Monster */
	155,		/* a Ring of See Invisible */
	156,		/* a Ring of Sustain Strength */
	157,		/* a Ring of Sustain Intelligence */
	158,		/* a Ring of Sustain Wisdom */
	159,		/* a Ring of Sustain Constitution */
	160,		/* a Ring of Sustain Dexterity */
	161,		/* a Ring of Sustain Charisma */
	162,		/* a Ring of Slaying */
	163,		/* an Amulet of Wisdom */
	164,		/* an Amulet of Charisma */
	165,		/* an Amulet of Searching */
	166,		/* an Amulet of Teleportation */
	167,		/* an Amulet of Slow Digestion */
	168,		/* an Amulet of Resist Acid */
	169,		/* an Amulet of Adornment */
	80,			/* Move: Bolt */
	171,		/* an Amulet of the Magi */
	172,		/* an Amulet of DOOM */
	173,		/* a Scroll of Enchant Weapon To-Hit */
	174,		/* a Scroll of Enchant Weapon To-Dam */
	175,		/* a Scroll of Enchant Armor */
	176,		/* a Scroll of Identify */
	176,		/* Move: Scroll of Identify (extra) */
	176,		/* Move: Scroll of Identify (extra) */
	176,		/* Move: Scroll of Identify (extra) */
	180,		/* a Scroll of Remove Curse */
	181,		/* a Scroll of Light */
	181,		/* Move: Scroll of Light (extra) */
	181,		/* Move: Scroll of Light (extra) */
	184,		/* a Scroll of Summon Monster */
	185,		/* a Scroll of Phase Door */
	186,		/* a Scroll of Teleportation */
	187,		/* a Scroll of Teleport Level */
	188,		/* a Scroll of Monster Confusion */
	189,		/* a Scroll of Magic Mapping */
	190,		/* a Scroll of Rune of Protection */
	190,		/* Move: Scroll of Rune of Protection */
	192,		/* a Scroll of Treasure Detection */
	193,		/* a Scroll of Object Detection */
	194,		/* a Scroll of Trap Detection */
	194,		/* Move: Scroll of Trap Detection (extra) */
	352,		/* Move: Rod of Trap Location (Hack!) */
	197,		/* a Scroll of Door/Stair Location */
	197,		/* Move: Scroll of Door/Stair Location (extra) */
	197,		/* Move: Scroll of Door/Stair Location (extra) */
	200,		/* a Scroll of Mass Genocide */
	201,		/* a Scroll of Detect Invisible */
	202,		/* a Scroll of Aggravate Monster */
	203,		/* a Scroll of Trap Creation */
	204,		/* a Scroll of Trap/Door Destruction */
	214,		/* Move: Scroll of *Enchant Armor* */
	206,		/* a Scroll of Recharging */
	207,		/* a Scroll of Genocide */
	208,		/* a Scroll of Darkness */
	209,		/* a Scroll of Protection from Evil */
	210,		/* a Scroll of Satisfy Hunger */
	211,		/* a Scroll of Dispel Undead */
	212,		/* a Scroll of *Enchant Weapon* */
	213,		/* a Scroll of Curse Weapon */
	214,		/* a Scroll of *Enchant Armor* */
	215,		/* a Scroll of Curse Armor */
	216,		/* a Scroll of Summon Undead */
	217,		/* a Scroll of Blessing */
	218,		/* a Scroll of Holy Chant */
	219,		/* a Scroll of Holy Prayer */
	220,		/* a Scroll of Word of Recall */
	221,		/* a Scroll of *Destruction* */
	222,		/* a Potion of Slime Mold Juice */
	223,		/* a Potion of Apple Juice */
	224,		/* a Potion of Water */
	225,		/* a Potion of Strength */
	226,		/* a Potion of Weakness */
	227,		/* a Potion of Restore Strength */
	228,		/* a Potion of Intelligence */
	229,		/* a Potion of Stupidity */
	230,		/* a Potion of Restore Intelligence */
	231,		/* a Potion of Wisdom */
	232,		/* a Potion of Naivety */
	233,		/* a Potion of Restore Wisdom */
	234,		/* a Potion of Charisma */
	235,		/* a Potion of Ugliness */
	236,		/* a Potion of Restore Charisma */
	237,		/* a Potion of Cure Light Wounds */
	237,		/* Move: Potion of Cure Light Wounds (extra) */
	237,		/* Move: Potion of Cure Light Wounds (extra) */
	240,		/* a Potion of Cure Serious Wounds */
	241,		/* a Potion of Cure Critical Wounds */
	242,		/* a Potion of Healing */
	243,		/* a Potion of Constitution */
	244,		/* a Potion of Experience */
	245,		/* a Potion of Sleep */
	246,		/* a Potion of Blindness */
	247,		/* a Potion of Confusion */
	248,		/* a Potion of Poison */
	249,		/* a Potion of Speed */
	250,		/* a Potion of Slowness */
	251,		/* a Potion of Dexterity */
	252,		/* a Potion of Restore Dexterity */
	253,		/* a Potion of Restore Constitution */
	254,		/* a Potion of Lose Memories */
	255,		/* a Potion of Salt Water */
	249,		/* Move: Potion of Speed (extra) */
	257,		/* a Potion of Heroism */
	258,		/* a Potion of Berserk Strength */
	259,		/* a Potion of Boldness */
	260,		/* a Potion of Restore Life Levels */
	261,		/* a Potion of Resist Heat */
	262,		/* a Potion of Resist Cold */
	263,		/* a Potion of Detect Invisible */
	264,		/* a Potion of Slow Poison */
	265,		/* a Potion of Neutralize Poison */
	266,		/* a Potion of Restore Mana */
	267,		/* a Potion of Infra-vision */
	348,		/* Move: Flask of oil */
	269,		/* a Wand of Light */
	270,		/* a Wand of Lightning Bolts */
	271,		/* a Wand of Frost Bolts */
	272,		/* a Wand of Fire Bolts */
	273,		/* a Wand of Stone to Mud */
	274,		/* a Wand of Polymorph */
	275,		/* a Wand of Heal Monster */
	276,		/* a Wand of Haste Monster */
	277,		/* a Wand of Slow Monster */
	278,		/* a Wand of Confuse Monster */
	279,		/* a Wand of Sleep Monster */
	280,		/* a Wand of Drain Life */
	281,		/* a Wand of Trap/Door Destruction */
	282,		/* a Wand of Magic Missile */
	283,		/* a Wand of Clone Monster */
	283,		/* Move: Wand of Clone Monster */
	285,		/* a Wand of Teleport Other */
	286,		/* a Wand of Disarming */
	287,		/* a Wand of Lightning Balls */
	288,		/* a Wand of Cold Balls */
	289,		/* a Wand of Fire Balls */
	290,		/* a Wand of Stinking Cloud */
	291,		/* a Wand of Acid Balls */
	292,		/* a Wand of Wonder */
	306,		/* Move: Staff of Light */
	294,		/* a Wand of Acid Bolts */
	295,		/* a Wand of Dragon's Flame */
	296,		/* a Wand of Dragon's Frost */
	297,		/* a Wand of Dragon's Breath */
	298,		/* a Wand of Annihilation */
	316,		/* Move: Staff of Door/Stair Location */
	300,		/* a Staff of Trap Location */
	301,		/* a Staff of Treasure Location */
	302,		/* a Staff of Object Location */
	303,		/* a Staff of Teleportation */
	304,		/* a Staff of Earthquakes */
	305,		/* a Staff of Summoning */
	305,		/* Move: Staff of Summoning (extra) */
	307,		/* a Staff of *Destruction* */
	308,		/* a Staff of Starlight */
	309,		/* a Staff of Haste Monsters */
	310,		/* a Staff of Slow Monsters */
	311,		/* a Staff of Sleep Monsters */
	312,		/* a Staff of Cure Light Wounds */
	313,		/* a Staff of Detect Invisible */
	314,		/* a Staff of Speed */
	315,		/* a Staff of Slowness */
	307,		/* Move: Staff of *Destruction* */
	317,		/* a Staff of Remove Curse */
	318,		/* a Staff of Detect Evil */
	319,		/* a Staff of Curing */
	320,		/* a Staff of Dispel Evil */
	322,		/* Move: Staff of Darkness */
	322,		/* a Staff of Darkness */
	323,		/* a Staff of Genocide */
	324,		/* a Staff of Power */
	325,		/* a Staff of the Magi */
	326,		/* a Staff of Perception */
	327,		/* a Staff of Holiness */
	328,		/* a Staff of Enlightenment */
	329,		/* a Staff of Healing */
	330,		/* a Book of Magic Spells [Magic for Beginners] */
	331,		/* a Book of Magic Spells [Conjurings and Tricks] */
	332,		/* a Book of Magic Spells [Incantations and Illusions] */
	333,		/* a Book of Magic Spells [Sorcery and Evocations] */
	334,		/* a Holy Book of Prayers [Beginners Handbook] */
	335,		/* a Holy Book of Prayers [Words of Wisdom] */
	336,		/* a Holy Book of Prayers [Chants and Blessings] */
	337,		/* a Holy Book of Prayers [Exorcism and Dispelling] */
	344,		/* Move: Chests become ruined chests */
	344,		/* Move: Chests become ruined chests */
	344,		/* Move: Chests become ruined chests */
	344,		/* Move: Chests become ruined chests */
	344,		/* Move: Chests become ruined chests */
	344,		/* Move: Chests become ruined chests */
	394,		/* Move: Junk and Skeletons */
	393,		/* Move: Junk and Skeletons */
	102,		/* Move: Filthy Rag */
	349,		/* Move: Junk and Skeletons */
	389,		/* Move: Junk and Skeletons */
	395,		/* Move: Junk and Skeletons */
	396,		/* Move: Junk and Skeletons */
	397,		/* Move: Junk and Skeletons */
	398,		/* Move: Junk and Skeletons */
	391,		/* Move: Junk and Skeletons */
	392,		/* Move: Junk and Skeletons */
	390,		/* Move: Junk and Skeletons */
	356,		/* a Rod of Light */
	357,		/* a Rod of Lightning Bolts */
	358,		/* a Rod of Frost Bolts */
	359,		/* a Rod of Fire Bolts */
	360,		/* a Rod of Polymorph */
	361,		/* a Rod of Slow Monster */
	362,		/* a Rod of Sleep Monster */
	363,		/* a Rod of Drain Life */
	364,		/* a Rod of Teleport Other */
	365,		/* a Rod of Disarming */
	366,		/* a Rod of Lightning Balls */
	367,		/* a Rod of Cold Balls */
	368,		/* a Rod of Fire Balls */
	369,		/* a Rod of Acid Balls */
	370,		/* a Rod of Acid Bolts */
	371,		/* a Rod of Enlightenment */
	372,		/* a Rod of Perception */
	373,		/* a Rod of Curing */
	374,		/* a Rod of Healing */
	375,		/* a Rod of Detection */
	376,		/* a Rod of Restoration */
	377,		/* a Rod of Speed */
	191,		/* Move: Scroll of *Remove Curse* */
	379,		/* a Book of Magic Spells [Resistance of Scarabtarices] */
	380,		/* a Book of Magic Spells [Mordenkainen's Escapes] */
	381,		/* a Book of Magic Spells [Kelek's Grimoire of Power] */
	382,		/* a Book of Magic Spells [Tenser's Transformations] */
	383,		/* a Book of Magic Spells [Raal's Tome of Destruction] */
	384,		/* a Holy Book of Prayers [Ethereal Openings] */
	385,		/* a Holy Book of Prayers [Godly Insights] */
	386,		/* a Holy Book of Prayers [Purifications and Healing] */
	387,		/* a Holy Book of Prayers [Holy Infusions] */
	388,		/* a Holy Book of Prayers [Wrath of God] */
	401,		/* Move: Blue Dragon Scale Mail */
	402,		/* Move: White Dragon Scale Mail */
	400,		/* Move: Black Dragon Scale Mail */
	404,		/* Move: Green Dragon Scale Mail */
	403,		/* Move: Red Dragon Scale Mail */
	405,		/* Move: Multi-Hued Dragon Scale Mail */
	43,			/* Move: Daggers (ancient artifacts?) */
	43,			/* Move: Daggers (ancient artifacts?) */
	43,			/* Move: Daggers (ancient artifacts?) */
	35,			/* Move: Short Sword */
	422,		/* Move: Potion of *Enlightenment* */
	417,		/* Move: Potion of Detonations */
	415,		/* Move: Potion of Death */
	420,		/* Move: Potion of Life */
	418,		/* Move: Potion of Augmentation */
	416,		/* Move: Potion of Ruination */
	355,		/* Move: Rod of Illumination */
	353,		/* Move: Rod of Probing */
	321,		/* Move: Staff of Probing */
	408,		/* Move: Bronze Dragon Scale Mail */
	409,		/* Move: Gold Dragon Scale Mail */
	354,		/* Move: Rod of Recall */
	123,		/* Move: Cloak (extra) */
	198,		/* Move: Scroll of Acquirement */
	199,		/* Move: Scroll of *Acquirement* */
	144,		/* Move: Ring of Free Action */
	410,		/* Move: Chaos Dragon Scale Mail */
	407,		/* Move: Law Dragon Scale Mail */
	411,		/* Move: Balance Dragon Scale Mail */
	406,		/* Move: Shining Dragon Scale Mail */
	412,		/* Move: Power Dragon Scale Mail */
	256,		/* Move: Potion of Enlightenment */
	421,		/* Move: Potion of Self Knowledge */
	419,		/* Move: Postion of *Healing* */
	21,			/* Move: Food ration (extra) */
	22,			/* Move: Hard Biscuit */
	23,			/* Move: Strip of Beef Jerky */
	26,			/* Move: Pint of Fine Ale */
	27,			/* Move: Pint of Fine Wine */
	87,			/* Move: Pick */
	84,			/* Move: Shovel */
	176,		/* Move: Scrolls of Identify (store) */
	181,		/* Move: Scrolls of Light (store) */
	185,		/* Move: Scrolls of Phase Door (store) */
	189,		/* Move: Scrolls of Mapping (store) */
	192,		/* Move: Scrolls of Treasure Detection (store) */
	193,		/* Move: Scrolls of Object Detection (store) */
	201,		/* Move: Scrolls of Detect Invisible (store) */
	217,		/* Move: Scrolls of Blessing (store) */
	220,		/* Move: Scrolls of Word of Recall (store) */
	237,		/* Move: Potions of Cure Light Wounds (store) */
	257,		/* Move: Potions of Heroism (store) */
	259,		/* Move: Potions of Boldness (store) */
	264,		/* Move: Potions of Slow Poison (store) */
	347,		/* Move: Lantern (store) */
	346,		/* Move: Torches (store) */
	348,		/* Move: Flasks of Oil (store) */
	446,		/* Hack: TERRAIN */
	447,		/* Hack: TERRAIN */
	448,		/* Hack: TERRAIN */
	449,		/* Hack: TERRAIN */
	450,		/* Hack: TERRAIN */
	451,		/* Hack: TERRAIN */
	452,		/* Hack: TERRAIN */
	453,		/* Hack: TERRAIN */
	454,		/* Hack: TERRAIN */
	455,		/* Hack: TERRAIN */
	456,		/* Hack: TERRAIN */
	457,		/* Hack: TERRAIN */
	458,		/* Hack: TERRAIN */
	0,			/* Move: Traps -- delete them all */
	0,			/* Move: Traps -- delete them all */
	0,			/* Move: Traps -- delete them all */
	0,			/* Move: Traps -- delete them all */
	0,			/* Move: Traps -- delete them all */
	0,			/* Move: Traps -- delete them all */
	0,			/* Move: Traps -- delete them all */
	0,			/* Move: Traps -- delete them all */
	0,			/* Move: Traps -- delete them all */
	0,			/* Move: Traps -- delete them all */
	0,			/* Move: Traps -- delete them all */
	0,			/* Move: Traps -- delete them all */
	0,			/* Move: Traps -- delete them all */
	0,			/* Move: Traps -- delete them all */
	0,			/* Move: Traps -- delete them all */
	0,			/* Move: Traps -- delete them all */
	0,			/* Move: Traps -- delete them all */
	0,			/* Move: Traps -- delete them all */
	445,		/* Move: Rubble */
	21,			/* Move: Mush -> Food Ration */
	459,		/* Move: Glyph of Warding */
	480,		/* copper */
	481,		/* copper */
	482,		/* copper */
	483,		/* silver */
	484,		/* silver */
	485,		/* silver */
	486,		/* garnets */
	487,		/* garnets */
	488,		/* gold */
	489,		/* gold */
	490,		/* gold */
	491,		/* opals */
	492,		/* sapphires */
	493,		/* rubies */
	494,		/* diamonds */
	495,		/* emeralds */
	496,		/* mithril */
	497,		/* adamantite */
	0,			/* Move: Nothing -- delete it */
	344,		/* Move: Ruined Chest */
	0			/* Move: Broken object -- delete it */
};


/*
 * Convert old kinds (501-512) into special artifacts
 */
static const byte convert_old_kinds_special[12] =
{
	ART_NARYA,		/* Old 501 */
	ART_NENYA,		/* Old 502 */
	ART_VILYA,		/* Old 503 */
	ART_POWER,		/* Old 504 */
	ART_GALADRIEL,	/* Old 505 */
	ART_INGWE,		/* Old 506 */
	ART_CARLAMMAS,	/* Old 507 */
	ART_ELENDIL,	/* Old 508 */
	ART_THRAIN,		/* Old 509 */
	ART_TULKAS,		/* Old 510 */
	ART_DWARVES,	/* Old 511 */
	ART_BARAHIR		/* Old 512 */
};



/*
 * Read an old-version "item" structure
 */
static errr rd_item_old(object_type *o_ptr)
{
	byte old_ident;
	byte old_names;

	s16b old_k_idx;

	s16b old_pval;

	s32b old_cost;

	u32b f1, f2, f3;

	object_kind *k_ptr;

	char old_note[128];


	/* Hack -- wipe */
	(void)WIPE(o_ptr, object_type);

	/* Old kind index */
	rd_s16b(&old_k_idx);

	/* Old Ego/Art index */
	rd_byte(&old_names);

	/* Old inscription */
	rd_string(old_note, 128);

	/* Save the inscription */
	if (old_note[0]) o_ptr->note = quark_add(old_note);

	/* Ignore "f1", "tval", "tchar" */
	strip_bytes(6);

	/* Old pval */
	rd_s16b(&old_pval);

	/* Old cost */
	rd_s32b(&old_cost);

	/* Ignore "sval" */
	strip_bytes(1);

	/* Quantity */
	rd_byte(&o_ptr->number);

	/* Ignore "weight" */
	strip_bytes(2);

	/* Bonuses */
	rd_s16b(&o_ptr->to_h);
	rd_s16b(&o_ptr->to_d);

	/* Ignore "ac" */
	strip_bytes(2);

	/* Bonuses */
	rd_s16b(&o_ptr->to_a);

	/* Ignore "dd", "ds", "level" */
	strip_bytes(3);

	/* Old special flags */
	rd_byte(&old_ident);

	/* Ignore "f2", "timeout" */
	strip_bytes(6);

	/* Ignore "color" data */
	if (arg_colour) strip_bytes(1);


	/* Paranoia */
	if ((old_k_idx < 0) || (old_k_idx >= 513) || (old_names >= 180))
	{
		note("Illegal object!");
		return (1);
	}

	/* Normal objects */
	if (old_k_idx < 501)
	{
		/* Convert */
		o_ptr->k_idx = convert_old_kinds_normal[old_k_idx];

		/* Mega-Hack -- handle "dungeon objects" later */
		if ((o_ptr->k_idx >= 445) && (o_ptr->k_idx <= 479)) return (0);
	}

	/* Special objects */
	else
	{
		artifact_type *a_ptr;

		/* Convert */
		o_ptr->name1 = convert_old_kinds_special[old_k_idx - 501];

		/* Artifact */
		a_ptr = &a_info[o_ptr->name1];

		/* Lookup the real "kind" */
		o_ptr->k_idx = lookup_kind(a_ptr->tval, a_ptr->sval);

		/* Ignore old_names */
		old_names = 0;
	}


	/* Analyze the old "special name" */
	old_names = convert_old_names[old_names];

	/* It is an artifact */
	if (old_names >= 128)
	{
		/* Extract the artifact index */
		o_ptr->name1 = (old_names - 128);
	}

	/* It is an ego-item (or a normal item) */
	else
	{
		/* Extract the ego-item index */
		o_ptr->name2 = (old_names);
	}


	/*** Analyze the item ***/

	/* Get the item kind */
	k_ptr = &k_info[o_ptr->k_idx];

	/* Extract "tval" and "sval" */
	o_ptr->tval = k_ptr->tval;
	o_ptr->sval = k_ptr->sval;

	/* Hack -- notice "broken" items */
	if (k_ptr->cost <= 0) o_ptr->ident |= (IDENT_BROKEN);

	/* Hack -- assume "cursed" items */
	if (k_ptr->flags3 & (TR3_LIGHT_CURSE)) o_ptr->ident |= (IDENT_CURSED);


	/*** Hack -- repair ego-item names ***/

	/* Repair ego-item names */
	if (o_ptr->name2)
	{
		/* Hack -- fix some "ego-missiles" */
		if ((o_ptr->tval == TV_BOLT) ||
		    (o_ptr->tval == TV_ARROW) ||
		    (o_ptr->tval == TV_SHOT))
		{
			/* Special ego-item indexes */
			if (o_ptr->name2 == EGO_BRAND_FIRE)
			{
				o_ptr->name2 = EGO_FLAME;
			}
			else if (o_ptr->name2 == EGO_SLAYING)
			{
				o_ptr->name2 = EGO_FROST;
			}
			else if (o_ptr->name2 == EGO_SLAY_ANIMAL)
			{
				o_ptr->name2 = EGO_HURT_ANIMAL;
			}
			else if (o_ptr->name2 == EGO_SLAY_EVIL)
			{
				o_ptr->name2 = EGO_HURT_EVIL;
			}
			else if (o_ptr->name2 == EGO_SLAY_DRAGON)
			{
				o_ptr->name2 = EGO_HURT_DRAGON;
			}
		}

		/* Hack -- fix "Bows" */
		if (o_ptr->tval == TV_BOW)
		{
			/* Special ego-item indexes */
			if (o_ptr->name2 == EGO_MIGHT) o_ptr->name2 = EGO_VELOCITY;
		}

		/* Hack -- fix "Robes" */
		if (o_ptr->tval == TV_SOFT_ARMOR)
		{
			/* Special ego-item indexes */
			if (o_ptr->name2 == EGO_MAGI) o_ptr->name2 = EGO_PERMANENCE;
		}

		/* Hack -- fix "Boots" */
		if (o_ptr->tval == TV_BOOTS)
		{
			/* Special ego-item indexes */
			if (o_ptr->name2 == EGO_STEALTH)
			{
				o_ptr->name2 = EGO_QUIET;
			}
			else if (o_ptr->name2 == EGO_FREE_ACTION)
			{
				o_ptr->name2 = EGO_MOTION;
			}
		}

		/* Hack -- fix "Shields" */
		if (o_ptr->tval == TV_SHIELD)
		{
			/* Special ego-item indexes */
			if (o_ptr->name2 == EGO_RESIST_ACID)
			{
				o_ptr->name2 = EGO_ENDURE_ACID;
			}
			else if (o_ptr->name2 == EGO_RESIST_ELEC)
			{
				o_ptr->name2 = EGO_ENDURE_ELEC;
			}
			else if (o_ptr->name2 == EGO_RESIST_FIRE)
			{
				o_ptr->name2 = EGO_ENDURE_FIRE;
			}
			else if (o_ptr->name2 == EGO_RESIST_COLD)
			{
				o_ptr->name2 = EGO_ENDURE_COLD;
			}
			else if (o_ptr->name2 == EGO_RESISTANCE)
			{
				o_ptr->name2 = EGO_ENDURANCE;
			}
			else if (o_ptr->name2 == EGO_ELVENKIND)
			{
				o_ptr->name2 = EGO_ENDURANCE;
			}
		}
	}


	/*** Convert old flags ***/

	/* Convert "store-bought" to "aware" */
	if (old_ident & 0x10) k_ptr->aware = TRUE;

	/* Convert "identified" to "aware" */
	if (old_ident & 0x08) k_ptr->aware = TRUE;

	/* Convert "store-bought" to "known" */
	if (old_ident & 0x10) o_ptr->ident |= (IDENT_KNOWN);

	/* Convert "identified" to "known" */
	if (old_ident & 0x08) o_ptr->ident |= (IDENT_KNOWN);

	/* Convert "ID_DAMD" to "ID_SENSE" */
	if (old_ident & 0x02) o_ptr->ident |= (IDENT_SENSE);

	/* Convert "ID_FELT" to "ID_SENSE" */
	if (old_ident & 0x01) o_ptr->ident |= (IDENT_SENSE);


	/*** Get the standard values ***/

	/* Get the standard fields */
	o_ptr->ac = k_ptr->ac;
	o_ptr->dd = k_ptr->dd;
	o_ptr->ds = k_ptr->ds;

	/* Get the standard weight */
	o_ptr->weight = k_ptr->weight;


	/*** Convert non-wearable items ***/

	/* Fix non-wearable items */
	if (!wearable_p(o_ptr))
	{
		/* Paranoia */
		o_ptr->name1 = o_ptr->name2 = 0;

		/* Assume normal bonuses */
		o_ptr->to_h = k_ptr->to_h;
		o_ptr->to_d = k_ptr->to_d;
		o_ptr->to_a = k_ptr->to_a;

		/* Get the normal pval */
		o_ptr->pval = k_ptr->pval;

		/* Hack -- wands/staffs use "pval" for "charges" */
		if (o_ptr->tval == TV_WAND) o_ptr->pval = old_pval;
		if (o_ptr->tval == TV_STAFF) o_ptr->pval = old_pval;

		/* Hack -- Gold uses "pval" for "value" */
		if (o_ptr->tval == TV_GOLD) o_ptr->pval = (s16b)old_cost;

		/* Success */
		return (0);
	}


	/*** Convert wearable items ***/

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3);

	/* Hack -- Rings/Amulets */
	if ((o_ptr->tval == TV_RING) || (o_ptr->tval == TV_AMULET))
	{
		/* Hack -- Adapt to the new "speed" code */
		if (f1 & (TR1_SPEED))
		{
			/* Paranoia -- only expand small bonuses */
			if (old_pval < 3) old_pval = old_pval * 10;
		}

		/* Hack -- Adapt to the new "searching" code */
		if (f1 & (TR1_SEARCH))
		{
			/* Paranoia -- only reduce large bonuses */
			old_pval = (old_pval + 4) / 5;
		}

		/* Hack -- Useful pval codes */
		if (f1 & (TR1_PVAL_MASK))
		{
			/* Require a pval code */
			o_ptr->pval = (old_pval ? old_pval : 1);
		}
	}

	/* Hack -- Lites */
	else if (o_ptr->tval == TV_LITE)
	{
		/* Hack -- keep old pval */
		o_ptr->pval = old_pval;
	}

	/* Update artifacts */
	if (o_ptr->name1)
	{
		artifact_type *a_ptr = &a_info[o_ptr->name1];

		/* Get the "broken" code */
		if (!a_ptr->cost) o_ptr->ident |= (IDENT_BROKEN);

		/* Get the artifact pval */
		o_ptr->pval = a_ptr->pval;

		/* Get the artifact fields */
		o_ptr->ac = a_ptr->ac;
		o_ptr->dd = a_ptr->dd;
		o_ptr->ds = a_ptr->ds;

		/* Get the artifact weight */
		o_ptr->weight = a_ptr->weight;

		/* Assume current "curse" */
		if (a_ptr->flags3 & (TR3_LIGHT_CURSE)) o_ptr->ident |= (IDENT_CURSED);
	}

	/* Update ego-items */
	if (o_ptr->name2)
	{
		ego_item_type *e_ptr = &e_info[o_ptr->name2];

		/* Get the "broken" code */
		if (!e_ptr->cost) o_ptr->ident |= (IDENT_BROKEN);

		/* Hack -- Adapt to the new "speed" code */
		if (f1 & (TR1_SPEED))
		{
			/* Paranoia -- only expand small bonuses */
			if (old_pval < 3) old_pval = old_pval * 10;
		}

		/* Hack -- Adapt to the new "searching" code */
		if (f1 & (TR1_SEARCH))
		{
			/* Paranoia -- only reduce large bonuses */
			old_pval = (old_pval + 4) / 5;
		}

		/* Hack -- Useful pval codes */
		if (f1 & (TR1_PVAL_MASK))
		{
			/* Require a pval code */
			o_ptr->pval = (old_pval ? old_pval : 1);
		}

		/* Assume current "curse" */
		if (e_ptr->flags3 & (TR3_LIGHT_CURSE)) o_ptr->ident |= (IDENT_CURSED);
	}


	/* Success */
	return (0);
}


/*
 * Read the old lore
 *
 * Hack -- Assume all kills were by the player
 *
 * Hack -- The "max_num" field is extracted later
 */
static void rd_lore_old(int r_idx)
{
	monster_lore *l_ptr = &l_list[r_idx];

	/* Forget old flags */
	strip_bytes(16);

	/* Read kills and deaths */
	rd_s16b(&l_ptr->r_pkills);
	rd_s16b(&l_ptr->r_deaths);

	/* Forget old info */
	strip_bytes(10);

	/* Guess at "sights" */
	l_ptr->r_sights = MAX(l_ptr->r_pkills, l_ptr->r_deaths);
}


/*
 * Read an old store
 */
static errr rd_store_old(int n)
{
	store_type *st_ptr = &store[n];

	int j;

	byte own, num;

	/* Read some things */
	rd_s32b(&st_ptr->store_open);
	rd_s16b(&st_ptr->insult_cur);
	rd_byte(&own);
	rd_byte(&num);
	rd_s16b(&st_ptr->good_buy);
	rd_s16b(&st_ptr->bad_buy);

	/* Paranoia */
	if ((own >= 24) || ((own % 8) != n))
	{
		note("Illegal store owner!");
		return (1);
	}

	/* Extract the owner */
	st_ptr->owner = convert_owner[own];

	/* Read the items */
	for (j = 0; j < num; j++)
	{
		object_type *i_ptr;
		object_type object_type_body;

		/* Strip the old "fixed cost" */
		strip_bytes(4);

		/* Get local object */
		i_ptr = &object_type_body;

		/* Wipe the object */
		object_wipe(i_ptr);

		/* Load the item */
		rd_item_old(i_ptr);

		/* Forget the inscription */
		i_ptr->note = 0;

		/* Save "valid" items */
		if (st_ptr->stock_num < STORE_INVEN_MAX)
		{
			int k = st_ptr->stock_num++;

			/* Add that item to the store */
			object_copy(&st_ptr->stock[k], i_ptr);
		}
	}

	/* Success */
	return (0);
}



/*
 * Hack -- array of old artifact index order
 */
static const byte old_art_order[] =
{
	ART_GROND,
	ART_RINGIL,
	ART_AEGLOS,
	ART_ARUNRUTH,
	ART_MORMEGIL,
	ART_ANGRIST,
	ART_GURTHANG,
	ART_CALRIS,
	ART_ANDURIL,
	ART_STING,
	ART_ORCRIST,
	ART_GLAMDRING,
	ART_DURIN,
	ART_AULE,
	ART_THUNDERFIST,
	ART_BLOODSPIKE,
	ART_DOOMCALLER,
	ART_NARTHANC,
	ART_NIMTHANC,
	ART_DETHANC,
	ART_GILETTAR,
	ART_RILIA,
	ART_BELANGIL,
	ART_BALLI,
	ART_LOTHARANG,
	ART_FIRESTAR,
	ART_ERIRIL,
	ART_CUBRAGOL,
	ART_BARD,
	ART_COLLUIN,
	ART_HOLCOLLETH,
	ART_TOTILA,
	ART_PAIN,
	ART_ELVAGIL,
	ART_AGLARANG,
	ART_EORLINGAS,
	ART_BARUKKHELED,
	ART_WRATH,
	ART_HARADEKKET,
	ART_MUNDWINE,
	ART_GONDRICAM,
	ART_ZARCUTHRA,
	ART_CARETH,
	ART_FORASGIL,
	ART_CRISDURIAN,
	ART_COLANNON,
	ART_HITHLOMIR,
	ART_THALKETTOTH,
	ART_ARVEDUI,
	ART_THRANDUIL,
	ART_THENGEL,
	ART_HAMMERHAND,
	ART_CELEGORM,
	ART_THROR,
	ART_MAEDHROS,
	ART_OLORIN,
	ART_ANGUIREL,
	ART_OROME,
	ART_EONWE,
	ART_THEODEN,
	ART_ULMO,
	ART_OSONDIR,
	ART_TURMIL,
	ART_CASPANION,
	ART_TIL,
	ART_DEATHWREAKER,
	ART_AVAVIR,
	ART_TARATOL,
	ART_DOR,
	ART_NENYA,
	ART_NARYA,
	ART_VILYA,
	ART_BELEGENNON,
	ART_FEANOR,
	ART_ISILDUR,
	ART_SOULKEEPER,
	ART_FINGOLFIN,
	ART_ANARION,
	ART_POWER,
	ART_GALADRIEL,
	ART_BELTHRONDING,
	ART_DAL,
	ART_PAURHACH,
	ART_PAURNIMMEN,
	ART_PAURAEGEN,
	ART_PAURNEN,
	ART_CAMMITHRIM,
	ART_CAMBELEG,
	ART_INGWE,
	ART_CARLAMMAS,
	ART_HOLHENNETH,
	ART_AEGLIN,
	ART_CAMLOST,
	ART_NIMLOTH,
	ART_NAR,
	ART_BERUTHIEL,
	ART_GORLIM,
	ART_ELENDIL,
	ART_THORIN,
	ART_CELEBORN,
	ART_THRAIN,
	ART_GONDOR,
	ART_THINGOL,
	ART_THORONGIL,
	ART_LUTHIEN,
	ART_TUOR,
	ART_ROHIRRIM,
	ART_TULKAS,
	ART_DWARVES,
	ART_BARAHIR,
	ART_RAZORBACK,
	ART_BLADETURNER,
	0
};


/*
 * Read the artifacts -- old version
 */
static void rd_artifacts_old(void)
{
	int i, a_idx;

	u32b tmp32u;

	/* Read the old artifact existance flags */
	for (i = 0; TRUE; i++)
	{
		/* Process next artifact */
		a_idx = old_art_order[i];

		/* All done */
		if (!a_idx) break;

		/* Read "created" flag */
		if (arg_stupid)
		{
			byte tmp8u;
			rd_byte(&tmp8u);
			tmp32u = tmp8u;
		}
		else
		{
			rd_u32b(&tmp32u);
		}

		/* Process the flag */
		if (tmp32u) a_info[a_idx].cur_num = 1;
	}
}





/*
 * Read the old "extra" information
 *
 * There were several "bugs" with the saving of the "current stat value"
 * array, which is handled by simply "restoring" all the player stats,
 * and then "stripping" the old "current stat" information, depending
 * on the version, which determines the number of bytes used.
 */
static void rd_extra_old(void)
{
	int i;


	rd_string(op_ptr->full_name, 32);

	rd_byte(&p_ptr->psex);
	rd_s32b(&p_ptr->au);
	rd_s32b(&p_ptr->max_exp);
	rd_s32b(&p_ptr->exp);
	rd_u16b(&p_ptr->exp_frac);
	rd_s16b(&p_ptr->age);
	rd_s16b(&p_ptr->ht);
	rd_s16b(&p_ptr->wt);
	rd_s16b(&p_ptr->lev);
	rd_s16b(&p_ptr->max_depth);
	strip_bytes(8);
	rd_s16b(&p_ptr->msp);
	rd_s16b(&p_ptr->mhp);
	strip_bytes(20);
	rd_s16b(&p_ptr->sc);
	strip_bytes(2);
	rd_byte(&p_ptr->pclass);
	rd_byte(&p_ptr->prace);
	rd_byte(&p_ptr->hitdie);
	rd_byte(&p_ptr->expfact);
	rd_s16b(&p_ptr->csp);
	rd_u16b(&p_ptr->csp_frac);
	rd_s16b(&p_ptr->chp);
	rd_u16b(&p_ptr->chp_frac);

	/* Hack -- Repair maximum player level */
	if (p_ptr->max_lev < p_ptr->lev) p_ptr->max_lev = p_ptr->lev;

	/* Hack -- Repair maximum dungeon level */
	if (p_ptr->max_depth < 0) p_ptr->max_depth = 1;

	/* Read the history */
	for (i = 0; i < 4; i++)
	{
		rd_string(p_ptr->history[i], 60);
	}

	/* Read the "maximum" stats */
	for (i = 0; i < A_MAX; i++)
	{
		/* Read the maximal stat */
		rd_s16b(&p_ptr->stat_max[i]);

		/* Paranoia -- Make sure the stat is legal */
		if (p_ptr->stat_max[i] > 18+100) p_ptr->stat_max[i] = 18+100;

		/* Fully restore the stat */
		p_ptr->stat_cur[i] = p_ptr->stat_max[i];

		/* Hack -- use that stat */
		p_ptr->stat_use[i] = p_ptr->stat_cur[i];
	}

	/* Strip the old "current stats" */
	if (older_than(2, 5, 7))
	{
		strip_bytes(12);
	}
	else
	{
		strip_bytes(6);
	}

	/* Strip the old "stat modifier" values */
	strip_bytes(12);

	/* Strip the old "resulting stat" values */
	strip_bytes(12);

	/* Read some stuff */
	strip_bytes(6); /* old "status" and "rest" */
	rd_s16b(&p_ptr->blind);
	rd_s16b(&p_ptr->paralyzed);
	rd_s16b(&p_ptr->confused);
	rd_s16b(&p_ptr->food);
	strip_bytes(6); /* old "digestion", "protection", "speed" */
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
	rd_s16b(&p_ptr->oppose_fire);
	rd_s16b(&p_ptr->oppose_cold);
	rd_s16b(&p_ptr->oppose_acid);
	rd_s16b(&p_ptr->oppose_elec);
	rd_s16b(&p_ptr->oppose_pois);
	rd_s16b(&p_ptr->tim_invis);
	rd_s16b(&p_ptr->word_recall);
	rd_s16b(&p_ptr->see_infra);
	rd_s16b(&p_ptr->tim_infra);
	strip_bytes(38); /* temporary stuff */

	/* Oops -- old "resist fear" code */
	if (!older_than(2, 6, 0)) strip_bytes(1);

	/* Old "missile counter */
	strip_bytes(2);

	/* Current turn */
	rd_s32b(&turn);
	if (turn < 0) turn = 0;

	/* Last turn */
	if (older_than(2, 6, 0))
	{
		old_turn = turn;
	}
	else
	{
		rd_s32b(&old_turn);
		if (old_turn < 0) old_turn = 0;
	}
}



/*
 * Read the saved messages
 */
static void rd_messages_old(void)
{
	int i, m = 22;

	char buf[128];

	u16b last_msg;

	/* Hack -- old method used circular queue */
	rd_u16b(&last_msg);

	/* Minor Hack -- may lose some old messages */
	for (i = 0; i < m; i++)
	{
		/* Read the message */
		rd_string(buf, 128);

		/* Save (most of) the messages */
		if (buf[0] && (i <= last_msg)) message_add(buf, MSG_GENERIC);
	}
}




/*
 * Read a pre-2.7.0 dungeon
 *
 * All sorts of information is lost from pre-2.7.0 savefiles, because they
 * were not saved in a very intelligent manner.
 *
 * Where important, we attempt to recover lost information, or at least to
 * simulate the presence of such information.
 *
 * Old savefiles may only contain 512 objects and 1024 monsters.
 *
 * Old savefiles encode some "terrain" information in "fake objects".
 */
static errr rd_dungeon_old(void)
{
	int i, y, x;
	byte count;
	byte ychar, xchar;
	s16b depth;
	s16b py, px;
	s16b ymax, xmax;
	int total_count;
	byte tmp8u;
	u16b tmp16u;
	u16b limit;

	byte ix[512];
	byte iy[512];


	/* Header info */
	rd_s16b(&depth);
	rd_s16b(&py);
	rd_s16b(&px);
	rd_u16b(&tmp16u);
	rd_s16b(&ymax);
	rd_s16b(&xmax);
	rd_u16b(&tmp16u);
	rd_u16b(&tmp16u);


	/* Ignore illegal dungeons */
	if ((depth < 0) || (depth >= MAX_DEPTH))
	{
		note(format("Ignoring illegal dungeon depth (%d)", depth));
		return (0);
	}

	/* Ignore illegal dungeons */
	if ((ymax != DUNGEON_HGT) || (xmax != DUNGEON_WID))
	{
		note(format("Ignoring illegal dungeon size (%d,%d).", ymax, xmax));
		return (0);
	}

	/* Ignore illegal dungeons */
	if (!in_bounds(py, px))
	{
		note(format("Ignoring illegal player location (%d,%d).", py, px));
		return (1);
	}


	/* Strip the monster indexes */
	while (1)
	{
		/* Read location */
		rd_byte(&tmp8u);
		if (tmp8u == 0xFF) break;
		ychar = tmp8u;
		rd_byte(&xchar);

		/* Location */
		y = ychar;
		x = xchar;

		/* Invalid cave location */
		if (!in_bounds(y, x))
		{
			note("Illegal monster location!");
			return (71);
		}

		/* Strip the index */
		if (older_than(2, 6, 0))
		{
			strip_bytes(1);
		}
		else
		{
			strip_bytes(2);
		}
	}


	/* Clear location data */
	for (i = 0; i < 512; i++)
	{
		ix[i] = iy[i] = 0;
	}

	/* Read the object indexes */
	while (1)
	{
		/* Read location */
		rd_byte(&tmp8u);
		if (tmp8u == 0xFF) break;
		ychar = tmp8u;
		rd_byte(&xchar);

		/* Location */
		y = ychar;
		x = xchar;

		/* Invalid cave location */
		if (!in_bounds(y, x))
		{
			note("Illegal object location!");
			return (72);
		}

		/* Read the item index */
		rd_u16b(&tmp16u);

		/* Ignore silly locations */
		if (tmp16u >= 512)
		{
			note("Illegal object index!");
			return (181);
		}

		/* Remember the location */
		iy[tmp16u] = y;
		ix[tmp16u] = x;
	}


	/* Read in the actual "cave" data */
	total_count = 0;
	x = y = 0;

	/* Read until done */
	while (total_count < DUNGEON_HGT * DUNGEON_WID)
	{
		/* Extract some RLE info */
		rd_byte(&count);
		rd_byte(&tmp8u);

		/* Apply the RLE info */
		for (i = count; i > 0; i--)
		{
			byte info = 0x00;
			byte feat = FEAT_FLOOR;

			/* Invalid cave location */
			if ((x >= DUNGEON_WID) || (y >= DUNGEON_HGT))
			{
				note("Dungeon too large!");
				return (81);
			}

			/* Extract the old "info" flags */
			if ((tmp8u >> 4) & 0x1) info |= (CAVE_ROOM);
			if ((tmp8u >> 5) & 0x1) info |= (CAVE_MARK);
			if ((tmp8u >> 6) & 0x1) info |= (CAVE_GLOW);

			/* Hack -- process old style "light" */
			if (info & (CAVE_GLOW))
			{
				info |= (CAVE_MARK);
			}

			/* Mega-Hack -- light all walls */
			else if ((tmp8u & 0xF) >= 12)
			{
				info |= (CAVE_GLOW);
			}

			/* Process the "floor type" */
			switch (tmp8u & 0xF)
			{
				/* Lite Room Floor */
				case 2:
				{
					info |= (CAVE_GLOW);
				}

				/* Dark Room Floor */
				case 1:
				{
					info |= (CAVE_ROOM);
					break;
				}

				/* Lite Vault Floor */
				case 4:
				{
					info |= (CAVE_GLOW);
				}

				/* Dark Vault Floor */
				case 3:
				{
					info |= (CAVE_ROOM);
					info |= (CAVE_ICKY);
					break;
				}

				/* Corridor Floor */
				case 5:
				{
					break;
				}

				/* Perma-wall */
				case 15:
				{
					feat = FEAT_PERM_SOLID;
					break;
				}

				/* Granite wall */
				case 12:
				{
					feat = FEAT_WALL_EXTRA;
					break;
				}

				/* Quartz vein */
				case 13:
				{
					feat = FEAT_QUARTZ;
					break;
				}

				/* Magma vein */
				case 14:
				{
					feat = FEAT_MAGMA;
					break;
				}
			}

			/* Save the info */
			cave_info[y][x] = info;

			/* Save the feat */
			cave_set_feat(y, x, feat);

			/* Advance the cave pointers */
			x++;

			/* Wrap to the next line */
			if (x >= DUNGEON_WID)
			{
				x = 0;
				y++;
			}
		}

		/* Increase the count */
		total_count += count;
	}


	/*** Player ***/

	/* Save depth */
	p_ptr->depth = depth;

	/* Place player in dungeon */
	if (!player_place(py, px))
	{
		note(format("Cannot place player (%d,%d)!", py, px));
		return (162);
	}


	/*** Objects ***/

	/* Read the item count */
	rd_u16b(&limit);

	/* Hack -- verify */
	if (limit >= 512)
	{
		note(format("Too many (%d) object entries!", limit));
		return (151);
	}

	/* Read the dungeon items */
	for (i = 1; i < limit; i++)
	{
		object_type *i_ptr;
		object_type object_type_body;


		/* Get local object */
		i_ptr = &object_type_body;

		/* Wipe the object */
		object_wipe(i_ptr);

		/* Read the item */
		rd_item_old(i_ptr);

		/* Skip dead objects */
		if (!i_ptr->k_idx) continue;


		/* Save location */
		i_ptr->iy = y = iy[i];
		i_ptr->ix = x = ix[i];

		/* Invalid cave location */
		if ((y >= DUNGEON_HGT) || (x >= DUNGEON_WID))
		{
			note("Illegal object location!!!");
			return (72);
		}


		/* Hack -- convert old "dungeon" objects */
		if ((i_ptr->k_idx >= 445) && (i_ptr->k_idx <= 479))
		{
			byte feat = 0;

			/* Analyze the "dungeon objects" */
			switch (i_ptr->k_idx)
			{
				/* Rubble */
				case 445:
				{
					feat = FEAT_RUBBLE;
					break;
				}

				/* Open Door */
				case 446:
				{
					feat = FEAT_OPEN;
					break;
				}

				/* Closed Door */
				case 447:
				{
					feat = FEAT_DOOR_HEAD;
					break;
				}

				/* Secret Door */
				case 448:
				{
					feat = FEAT_SECRET;
					break;
				}

				/* Up Stairs */
				case 449:
				{
					feat = FEAT_LESS;
					break;
				}

				/* Down Stairs */
				case 450:
				{
					feat = FEAT_MORE;
					break;
				}

				/* Store '1' */
				case 451:
				{
					feat = FEAT_SHOP_HEAD + 0x00;
					break;
				}

				/* Store '2' */
				case 452:
				{
					feat = FEAT_SHOP_HEAD + 0x01;
					break;
				}

				/* Store '3' */
				case 453:
				{
					feat = FEAT_SHOP_HEAD + 0x02;
					break;
				}

				/* Store '4' */
				case 454:
				{
					feat = FEAT_SHOP_HEAD + 0x03;
					break;
				}

				/* Store '5' */
				case 455:
				{
					feat = FEAT_SHOP_HEAD + 0x04;
					break;
				}

				/* Store '6' */
				case 456:
				{
					feat = FEAT_SHOP_HEAD + 0x05;
					break;
				}

				/* Store '7' */
				case 457:
				{
					feat = FEAT_SHOP_HEAD + 0x06;
					break;
				}

				/* Store '8' */
				case 458:
				{
					feat = FEAT_SHOP_HEAD + 0x07;
					break;
				}

				/* Glyph of Warding */
				case 459:
				{
					feat = FEAT_GLYPH;
					break;
				}
			}

			/* Hack -- use the feature */
			cave_set_feat(y, x, feat);

			/* Done */
			continue;
		}


		/* Hack -- treasure in walls */
		if (i_ptr->tval == TV_GOLD)
		{
			/* Quartz or Magma with treasure */
			if ((cave_feat[y][x] == FEAT_QUARTZ) ||
			    (cave_feat[y][x] == FEAT_MAGMA))
			{
				/* Add treasure */
				cave_set_feat(y, x, cave_feat[y][x] + 0x04);

				/* Done */
				continue;
			}
		}


		/* Give the item to the floor */
		if (!floor_carry(i_ptr->iy, i_ptr->ix, i_ptr))
		{
			note(format("Cannot place object %d!", o_max));
			return (152);
		}
	}


	/*** Monster ***/

	/* Read the monster count */
	rd_u16b(&limit);

	/* Hack -- verify */
	if (limit >= 1024)
	{
		note(format("Too many (%d) monster entries!", limit));
		return (161);
	}

	/* Read the monsters (starting at 2) */
	for (i = 2; i < limit; i++)
	{
		monster_race *r_ptr;

		monster_type *n_ptr;
		monster_type monster_type_body;


		/* Get local monster */
		n_ptr = &monster_type_body;

		/* Hack -- wipe */
		(void)WIPE(n_ptr, monster_type);

		/* Read the current hitpoints */
		rd_s16b(&n_ptr->hp);

		/* Strip max hitpoints */
		if (!older_than(2, 6, 0)) strip_bytes(2);

		/* Current sleep counter */
		rd_s16b(&n_ptr->csleep);

		/* Strip speed */
		strip_bytes(2);

		/* Read race */
		rd_s16b(&n_ptr->r_idx);

		/* Read location */
		rd_byte(&n_ptr->fy);
		rd_byte(&n_ptr->fx);

		/* Extract location */
		y = n_ptr->fy;
		x = n_ptr->fx;

		/* Strip confusion, etc */
		strip_bytes(4);

		/* Fear */
		if (!older_than(2, 6, 0)) strip_bytes(1);

		/* Old "color" data */
		if (arg_colour) strip_bytes(1);


		/* Hack -- ignore "broken" monsters */
		if (n_ptr->r_idx <= 0) continue;

		/* Hack -- ignore "player ghosts" */
		if (n_ptr->r_idx >= z_info->r_max-1) continue;


		/* Invalid cave location */
		if ((x >= DUNGEON_WID) || (y >= DUNGEON_HGT))
		{
			note("Illegal monster location!!!");
			return (71);
		}


		/* Get the race */
		r_ptr = &r_info[n_ptr->r_idx];

		/* Hack -- recalculate speed */
		n_ptr->mspeed = r_ptr->speed;

		/* Hack -- fake energy */
		n_ptr->energy = i % 100;

		/* Hack -- maximal hitpoints */
		n_ptr->maxhp = r_ptr->hdice * r_ptr->hside;


		/* Place monster in dungeon */
		if (!monster_place(n_ptr->fy, n_ptr->fx, n_ptr))
		{
			note(format("Cannot place monster %d!", i));
			return (162);
		}
	}


	/* The dungeon is ready */
	character_dungeon = TRUE;


	/* Success */
	return (0);
}




/*
 * Read pre-2.7.0 options
 */
static void rd_options_old(void)
{
	u32b tmp32u;

	/* Unused */
	strip_bytes(2);

	/* Standard options */
	rd_u32b(&tmp32u);

	/* Mega-Hack -- Extract death */
	p_ptr->is_dead = (tmp32u & 0x80000000) ? TRUE : FALSE;

	/* Hack -- Unused options */
	if (!older_than(2, 6, 0)) strip_bytes(12);
}


/*
 * Read the pre-2.7.0 inventory
 */
static errr rd_inventory_old(void)
{
	int i, n;
	int slot = 0;
	s16b ictr;

	object_type *i_ptr;
	object_type object_type_body;

	/* Count the items */
	rd_s16b(&ictr);

	/* Verify */
	if (ictr >= INVEN_PACK)
	{
		note("Too many items in the inventory!");
		return (15);
	}

	/* Normal pack items */
	for (i = 0; i < ictr; i++)
	{
		/* Get local object */
		i_ptr = &object_type_body;

		/* Wipe the object */
		object_wipe(i_ptr);

		/* Read the item */
		rd_item_old(i_ptr);

		/* Assume aware */
		object_aware(i_ptr);

		/* Get the next slot */
		n = slot++;

		/* Copy the object */
		object_copy(&inventory[n], i_ptr);

		/* Add the weight */
		p_ptr->total_weight += (i_ptr->number * i_ptr->weight);

		/* One more item */
		p_ptr->inven_cnt++;
	}

	/* Old "normal" equipment */
	for (i = OLD_INVEN_WIELD; i < OLD_INVEN_AUX; i++)
	{
		/* Get local object */
		i_ptr = &object_type_body;

		/* Wipe the object */
		object_wipe(i_ptr);

		/* Read the item */
		rd_item_old(i_ptr);

		/* Assume aware */
		object_aware(i_ptr);

		/* Skip "empty" slots */
		if (!i_ptr->tval) continue;

		/* Hack -- convert old slot numbers */
		n = convert_slot(i);

		/* Copy the object */
		object_copy(&inventory[n], i_ptr);

		/* Add the weight */
		p_ptr->total_weight += (i_ptr->number * i_ptr->weight);

		/* One more item */
		p_ptr->equip_cnt++;
	}

	/* Old "aux" item */
	for (i = OLD_INVEN_AUX; i <= OLD_INVEN_AUX; i++)
	{
		/* Get local object */
		i_ptr = &object_type_body;

		/* Wipe the object */
		object_wipe(i_ptr);

		/* Read the item */
		rd_item_old(i_ptr);

		/* Assume aware */
		object_aware(i_ptr);

		/* Skip "empty" slots */
		if (!i_ptr->tval) continue;

		/* Get the next slot */
		n = slot++;

		/* Copy the object */
		object_copy(&inventory[n], i_ptr);

		/* Add the weight */
		p_ptr->total_weight += (i_ptr->number * i_ptr->weight);

		/* One more item */
		p_ptr->inven_cnt++;
	}

	/* Forget old weight */
	strip_bytes(4);

	/* Success */
	return (0);
}


/*
 * Read a pre-2.7.0 savefile
 *
 * Note that some information from older savefiles is ignored,
 * converted, or simulated, to keep the process simple but valid.
 */
static errr rd_savefile_old_aux(void)
{
	int i;

	u16b tmp16u;
	u32b tmp32u;


	/* Mention the savefile version */
	note(format("Loading a %d.%d.%d savefile...",
	            sf_major, sf_minor, sf_patch));


	/* Mega-Hack */
	if (older_than(2, 5, 2))
	{
		/* Allow use of old MacAngband 1.0 and 2.0.3 savefiles */
		if (get_check("Are you using an old Macintosh savefile? "))
		{
			/* Set a flag */
			arg_stupid = arg_colour = TRUE;
		}

		/* Allow use of old PC Angband 1.4 savefiles */
		else if (get_check("Are you using an old PC savefile? "))
		{
			/* Set a flag */
			arg_stupid = arg_colour = arg_crappy = TRUE;
		}

		/* Allow use of old Archimedes Angband 1.2 savefiles */
		else if (get_check("Are you using an old Archimedes savefile? "))
		{
			/* Set a flag */
			arg_stupid = arg_crappy = TRUE;
		}
	}


	/* Strip the version bytes */
	strip_bytes(4);

	/* Hack -- decrypt */
	xor_byte = sf_extra;


	/* Read the artifacts */
	rd_artifacts_old();
	note("Loaded Artifacts");

	/* Strip "quest" data */
	if (arg_stupid)
	{
		strip_bytes(8);
	}
	else
	{
		strip_bytes(16);
	}
	note("Loaded Quests");


	/* Load the old "Uniques" flags */
	for (i = 0; i < z_info->r_max; i++)
	{
		monster_race *r_ptr = &r_info[i];

		/* Read "exist" and "dead" flags */
		if (arg_stupid)
		{
			byte tmp8u;
			rd_byte(&tmp8u);
			rd_byte(&tmp8u);
			tmp32u = tmp8u;
		}
		else
		{
			rd_u32b(&tmp32u);
			rd_u32b(&tmp32u);
		}

		/* Paranoia */
		r_ptr->cur_num = 0;

		/* This value is unused */
		r_ptr->max_num = 100;

		/* Only one unique monster */
		if (r_ptr->flags1 & (RF1_UNIQUE)) r_ptr->max_num = 1;

		/* Hack -- No ghosts */
		if (i == z_info->r_max-1) r_ptr->max_num = 0;

		/* Note death */
		if (tmp32u) r_ptr->max_num = 0;
	}
	note("Loaded Unique Beasts");


	/* Hack -- assume previous lives */
	sf_lives = 1;

	/* Load the recall memory */
	while (1)
	{
		monster_race *r_ptr;
		monster_lore *l_ptr;

		/* Read some info, check for sentinal */
		rd_u16b(&tmp16u);
		if (tmp16u == 0xFFFF) break;

		/* Incompatible save files */
		if (tmp16u >= z_info->r_max)
		{
			note(format("Too many (%u) monster races!", tmp16u));
			return (21);
		}

		/* Get the monster */
		r_ptr = &r_info[tmp16u];
		l_ptr = &l_list[tmp16u];

		/* Extract the monster lore */
		rd_lore_old(tmp16u);

		/* Hack -- Prevent fake kills */
		if (r_ptr->flags1 & (RF1_UNIQUE))
		{
			/* Hack -- Note living uniques */
			if (r_ptr->max_num != 0) l_ptr->r_pkills = 0;
		}
	}
	note("Loaded Monster Memory");


	/* Read the old options */
	rd_options_old();
	note("Loaded options");

	/* Read the extra stuff */
	rd_extra_old();
	note("Loaded extra information");


	/* Initialize the sex */
	sp_ptr = &sex_info[p_ptr->psex];

	/* Initialize the race/class */
	rp_ptr = &p_info[p_ptr->prace];
	cp_ptr = &c_info[p_ptr->pclass];

	/* Initialize the magic */
	mp_ptr = &cp_ptr->spells;


	/* Fake some "item awareness" */
	for (i = 1; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Hack -- learn about "obvious" items */
		if (k_ptr->level < p_ptr->max_depth) k_ptr->aware = TRUE;
	}


	/* Read the inventory */
	rd_inventory_old();


	/* Read spell flags */
	rd_u32b(&p_ptr->spell_learned1);
	rd_u32b(&p_ptr->spell_worked1);
	rd_u32b(&p_ptr->spell_forgotten1);
	rd_u32b(&p_ptr->spell_learned2);
	rd_u32b(&p_ptr->spell_worked2);
	rd_u32b(&p_ptr->spell_forgotten2);

	/* Read spell order */
	for (i = 0; i < 64; i++)
	{
		rd_byte(&p_ptr->spell_order[i]);
	}

	note("Loaded spell information");


	/* Ignore old aware/tried flags */
	strip_bytes(1024);

	/* Old seeds */
	rd_u32b(&seed_flavor);
	rd_u32b(&seed_town);

	/* Old messages */
	rd_messages_old();

	/* Some leftover info */
	rd_u16b(&p_ptr->panic_save);
	rd_u16b(&p_ptr->total_winner);
	rd_u16b(&p_ptr->noscore);

	/* Read the player_hp array */
	for (i = 0; i < 50; i++) rd_s16b(&p_ptr->player_hp[i]);

	/* Strip silly hitpoint information */
	if (!older_than(2, 6, 2)) strip_bytes(98);

	note("Loaded some more information");


	/* Read the stores */
	for (i = 0; i < MAX_STORES; i++)
	{
		if (rd_store_old(i))
		{
			note("Error loading a store!");
			return (32);
		}
	}


	/* Time at which file was saved */
	rd_u32b(&sf_when);

	/* Read the cause of death, if any */
	rd_string(p_ptr->died_from, 80);

	note("Loaded all player info");


	/* Read dungeon */
	if (!p_ptr->is_dead)
	{
		/* Dead players have no dungeon */
		if (rd_dungeon_old())
		{
			note("Error loading dungeon!");
			return (25);
		}
		note("Loaded dungeon");
	}


	/* Hack -- no ghosts */
	r_info[z_info->r_max-1].max_num = 0;


	/* Hack -- reset morgoth */
	r_info[R_INFO_MORGOTH].max_num = 1;

	/* Hack -- reset sauron */
	r_info[R_INFO_SAURON].max_num = 1;


	/* Hack -- reset morgoth */
	l_list[R_INFO_MORGOTH].r_pkills = 0;

	/* Hack -- reset sauron */
	l_list[R_INFO_SAURON].r_pkills = 0;


	/* Add first quest */
	q_list[0].level = 99;

	/* Add second quest */
	q_list[1].level = 100;

	/* Reset third quest */
	q_list[2].level = 0;

	/* Reset fourth quest */
	q_list[3].level = 0;


	/* Hack -- maximize mode */
	if (arg_crappy) adult_maximize = TRUE;


	/* Assume success */
	return (0);
}


#endif	/* ALLOW_OLD_SAVEFILES */


/*
 * Attempt to load a pre-2.7.0 savefile
 */
errr rd_savefile_old(void)
{
	errr err = -1;

#ifdef ALLOW_OLD_SAVEFILES

	/* Grab permissions */
	safe_setuid_grab();

	/* The savefile is a binary file */
	fff = my_fopen(savefile, "rb");

	/* Drop permissions */
	safe_setuid_drop();

	/* Paranoia */
	if (!fff) return (-1);

	/* Call the sub-function */
	err = rd_savefile_old_aux();

	/* Check for errors */
	if (ferror(fff)) err = -1;

	/* Close the file */
	my_fclose(fff);

#endif /* ALLOW_OLD_SAVEFILES */

	/* Result */
	return (err);
}

