/* File: save-old.c */

/* Purpose: support for loading pre-2.7.0 savefiles */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "angband.h"


/*
 * This save package was brought to by		-JWT- and -RAK-
 * and has been completely rewritten for UNIX by	-JEW-  
 *
 * and has been completely rewritten again by	 -CJS-
 * and completely rewritten again! for portability by -JEW-
 *
 * This file is only used to parse pre-2.7.0 savefiles.
 * See the file "save.c" for more recent methods.
 */


/*
 * these are used for the save file, to avoid having to pass them to every
 * procedure 
 */

static FILE	*fff;		/* Current save "file" */

static byte	xor_byte;	/* Simple encryption */

static byte	version_maj;	/* Major version */
static byte	version_min;	/* Minor version */
static byte	patch_level;	/* Patch level */

static bool say;		/* Debugging */


/*
 * This function determines if the version of the savefile
 * currently being read is older than version "x.y.z".
 */
static bool older_than(int x, int y, int z)
{
    /* Much older, or much more recent */
    if (version_maj < x) return (TRUE);
    if (version_maj > x) return (FALSE);

    /* Distinctly older, or distinctly more recent */
    if (version_min < y) return (TRUE);
    if (version_min > y) return (FALSE);

    /* Barely older, or barely more recent */
    if (patch_level < z) return (TRUE);
    if (patch_level > z) return (FALSE);

    /* Identical versions */
    return (FALSE);
}


/*
 * Show information on the screen, one line at a time.
 * If "where" is negative, advance "-where" lines from last location.
 */
static void prt_note(int where, cptr msg)
{
    static int y = 0;

    /* Accept line number, Remember the line */
    y = (where < 0) ? (y - where) : where;

    /* Attempt to "wrap" if forced to */
    if (y >= 24) y = 0;

    /* Draw the message */
    prt(msg, y, 0);

    /* Flush it */
    Term_fresh();
}




/*
 * The basic Input function for old savefiles
 * All information is read one byte at a time
 * Note that this was stolen from "save.c"
 */

static byte sf_get(void)
{
    register byte c, v;

    /* Get a character, decode the value */
    c = getc(fff) & 0xFF;
    v = c ^ xor_byte;
    xor_byte = c;

    /* Return the value */    
    return (v);
}




/*
 * Write/Read various "byte sized" objects
 */

static void rd_byte(byte *ip)
{
    *ip = sf_get();
}


static void rd_char(char *ip)
{
    rd_byte((byte*)ip);
}


/*
 * Write/Read various "short" objects
 */

static void rd_u16b(u16b *ip)
{
    (*ip) = sf_get();
    (*ip) |= ((u16b)(sf_get()) << 8);
}


static void rd_s16b(s16b *ip)
{
    rd_u16b((u16b*)ip);
}



/*
 * Write/Read various "long" objects
 */

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
 * Strings
 */

static void rd_string(char *str)
{
    while (1) {
	byte tmp;
	rd_byte(&tmp);
	*str = tmp;
	if (!*str) break;
	str++;
    }
}



/*
 * Mega-Hack -- convert the old "name2" fields into the new
 * name1/name2 fields.  Note that the entries below appear
 * in the same order as the old "SN_xxx" defines.
 */

#define CHE_EMPTY		1	/* Hack -- see flags1 */
#define CHE_DISARMED		2	/* Hack -- see below */
#define CHE_UNLOCKED		3	/* Hack -- see flags2 */
#define CHE_LOCKED		4	/* Hack -- see flags2 */

#define CHE_POISON_NEEDLE	11	/* Hack -- see flags2 */
#define CHE_GAS_TRAP		12	/* Hack -- see flags2 */
#define CHE_EXPLOSION_DEVICE	13	/* Hack -- see flags2 */
#define CHE_SUMMONING_RUNES	14	/* Hack -- see flags2 */
#define CHE_MULTIPLE_TRAPS	15	/* Hack -- see flags2 */

static int convert_name2[] = {

    0				/* 0 = SN_NULL */,
    2000+EGO_R			/* 1 = SN_R */,
    2000+EGO_RESIST_A		/* 2 = SN_RA */,
    2000+EGO_RESIST_F		/* 3 = SN_RF */,
    2000+EGO_RESIST_C		/* 4 = SN_RC */,
    2000+EGO_RESIST_E		/* 5 = SN_RL */,
    2000+EGO_HA			/* 6 = SN_HA */,
    2000+EGO_DF			/* 7 = SN_DF */,
    2000+EGO_SLAY_A		/* 8 = SN_SA */,
    2000+EGO_SLAY_D		/* 9 = SN_SD */,
    2000+EGO_SLAY_E		/* 10 = SN_SE */,
    2000+EGO_SLAY_U		/* 11 = SN_SU */,
    2000+EGO_FT			/* 12 = SN_FT */,
    2000+EGO_FB			/* 13 = SN_FB */,
    2000+EGO_FREE_ACTION	/* 14 = SN_FREE_ACTION */,
    2000+EGO_SLAYING		/* 15 = SN_SLAYING */,
    2000+EGO_CLUMSINESS		/* 16 = SN_CLUMSINESS */,
    2000+EGO_WEAKNESS		/* 17 = SN_WEAKNESS */,
    2000+EGO_SLOW_DESCENT	/* 18 = SN_SLOW_DESCENT */,
    2000+EGO_SPEED		/* 19 = SN_SPEED */,
    2000+EGO_STEALTH		/* 20 = SN_STEALTH */,
    2000+EGO_SLOWNESS		/* 21 = SN_SLOWNESS */,
    2000+EGO_NOISE		/* 22 = SN_NOISE */,
    2000+EGO_GREAT_MASS		/* 23 = SN_GREAT_MASS */,
    2000+EGO_INTELLIGENCE	/* 24 = SN_INTELLIGENCE */,
    2000+EGO_WISDOM		/* 25 = SN_WISDOM */,
    2000+EGO_INFRAVISION	/* 26 = SN_INFRAVISION */,
    2000+EGO_MIGHT		/* 27 = SN_MIGHT */,
    2000+EGO_LORDLINESS		/* 28 = SN_LORDLINESS */,
    2000+EGO_MAGI		/* 29 = SN_MAGI */,
    2000+EGO_BEAUTY		/* 30 = SN_BEAUTY */,
    2000+EGO_SEEING		/* 31 = SN_SEEING */,
    2000+EGO_REGENERATION	/* 32 = SN_REGENERATION */,
    2000+EGO_STUPIDITY		/* 33 = SN_STUPIDITY */,
    2000+EGO_DULLNESS		/* 34 = SN_DULLNESS */,
    0				/* 35 = SN_BLINDNESS */,
    0				/* 36 = SN_TIMIDNESS */,
    0				/* 37 = SN_TELEPORTATION */,
    2000+EGO_UGLINESS		/* 38 = SN_UGLINESS */,
    2000+EGO_PROTECTION		/* 39 = SN_PROTECTION */,
    2000+EGO_IRRITATION		/* 40 = SN_IRRITATION */,
    2000+EGO_VULNERABILITY	/* 41 = SN_VULNERABILITY */,
    2000+EGO_ENVELOPING		/* 42 = SN_ENVELOPING */,
    2000+EGO_FIRE		/* 43 = SN_FIRE */,
    2000+EGO_SLAY_EVIL		/* 44 = SN_SLAY_EVIL */,
    2000+EGO_DRAGON_SLAYING	/* 45 = SN_DRAGON_SLAYING */,
    9000+CHE_EMPTY		/* 46 = SN_EMPTY */,
    9000+CHE_LOCKED		/* 47 = SN_LOCKED */,
    9000+CHE_POISON_NEEDLE	/* 48 = SN_POISON_NEEDLE */,
    9000+CHE_GAS_TRAP		/* 49 = SN_GAS_TRAP */,
    9000+CHE_EXPLOSION_DEVICE	/* 50 = SN_EXPLOSION_DEVICE */,
    9000+CHE_SUMMONING_RUNES	/* 51 = SN_SUMMONING_RUNES */,
    9000+CHE_MULTIPLE_TRAPS	/* 52 = SN_MULTIPLE_TRAPS */,
    9000+CHE_DISARMED		/* 53 = SN_DISARMED */,
    9000+CHE_UNLOCKED		/* 54 = SN_UNLOCKED */,
    2000+EGO_SLAY_ANIMAL	/* 55 = SN_SLAY_ANIMAL */,
    1000+ART_GROND		/* 56 = SN_GROND */,
    1000+ART_RINGIL		/* 57 = SN_RINGIL */,
    1000+ART_AEGLOS		/* 58 = SN_AEGLOS */,
    1000+ART_ARUNRUTH		/* 59 = SN_ARUNRUTH */,
    1000+ART_MORMEGIL		/* 60 = SN_MORMEGIL */,
    2000+EGO_MORGUL		/* 61 = SN_MORGUL */,
    1000+ART_ANGRIST		/* 62 = SN_ANGRIST */,
    1000+ART_GURTHANG		/* 63 = SN_GURTHANG */,
    1000+ART_CALRIS		/* 64 = SN_CALRIS */,
    2000+EGO_ACCURACY		/* 65 = SN_ACCURACY */,
    1000+ART_ANDURIL		/* 66 = SN_ANDURIL */,
    2000+EGO_SLAY_O		/* 67 = SN_SO */,
    2000+EGO_POWER		/* 68 = SN_POWER */,
    1000+ART_DURIN		/* 69 = SN_DURIN */,
    1000+ART_AULE		/* 70 = SN_AULE */,
    2000+EGO_WEST		/* 71 = SN_WEST */,
    2000+EGO_BLESS_BLADE	/* 72 = SN_BLESS_BLADE */,
    2000+EGO_SLAY_DEMON		/* 73 = SN_SDEM */,
    2000+EGO_SLAY_T		/* 74 = SN_ST */,
    1000+ART_BLOODSPIKE		/* 75 = SN_BLOODSPIKE */,
    1000+ART_THUNDERFIST	/* 76 = SN_THUNDERFIST */,
    2000+EGO_WOUNDING		/* 77 = SN_WOUNDING */,
    1000+ART_ORCRIST		/* 78 = SN_ORCRIST */,
    1000+ART_GLAMDRING		/* 79 = SN_GLAMDRING */,
    1000+ART_STING		/* 80 = SN_STING */,
    2000+EGO_LITE		/* 81 = SN_LITE */,
    2000+EGO_AGILITY		/* 82 = SN_AGILITY */,
    2000+EGO_BACKBITING		/* 83 = SN_BACKBITING */,
    1000+ART_DOOMCALLER		/* 84 = SN_DOOMCALLER */,
    2000+EGO_SLAY_G		/* 85 = SN_SG */,
    2000+EGO_TELEPATHY		/* 86 = SN_TELEPATHY */,
    0				/* 87 = SN_DRAGONKIND */,
    0				/* 88 = SN_NENYA */,
    0				/* 89 = SN_NARYA */,
    0				/* 90 = SN_VILYA */,
    2000+EGO_AMAN		/* 91 = SN_AMAN */,
    1000+ART_BELEGENNON		/* 92 = SN_BELEGENNON */,
    1000+ART_FEANOR		/* 93 = SN_FEANOR */,
    1000+ART_ANARION		/* 94 = SN_ANARION */,
    1000+ART_ISILDUR		/* 95 = SN_ISILDUR */,
    1000+ART_FINGOLFIN		/* 96 = SN_FINGOLFIN */,
    2000+EGO_ELVENKIND		/* 97 = SN_ELVENKIND */,
    1000+ART_SOULKEEPER		/* 98 = SN_SOULKEEPER */,
    1000+ART_DOR		/* 99 = SN_DOR_LOMIN */,
    1000+ART_MORGOTH		/* 100 = SN_MORGOTH */,
    1000+ART_BELTHRONDING	/* 101 = SN_BELTHRONDING */,
    1000+ART_DAL		/* 102 = SN_DAL */,
    1000+ART_PAURHACH		/* 103 = SN_PAURHACH */,
    1000+ART_PAURNIMMEN		/* 104 = SN_PAURNIMMEN */,
    1000+ART_PAURAEGEN		/* 105 = SN_PAURAEGEN */,
    1000+ART_CAMMITHRIM		/* 106 = SN_CAMMITHRIM */,
    1000+ART_CAMBELEG		/* 107 = SN_CAMBELEG */,
    1000+ART_HOLHENNETH		/* 108 = SN_HOLHENNETH */,
    1000+ART_PAURNEN		/* 109 = SN_PAURNEN */,
    1000+ART_AEGLIN		/* 110 = SN_AEGLIN */,
    1000+ART_CAMLOST		/* 111 = SN_CAMLOST */,
    1000+ART_NIMLOTH		/* 112 = SN_NIMLOTH */,
    1000+ART_NAR		/* 113 = SN_NAR */,
    1000+ART_BERUTHIEL		/* 114 = SN_BERUTHIEL */,
    1000+ART_GORLIM		/* 115 = SN_GORLIM */,
    1000+ART_NARTHANC		/* 116 = SN_NARTHANC */,
    1000+ART_NIMTHANC		/* 117 = SN_NIMTHANC */,
    1000+ART_DETHANC		/* 118 = SN_DETHANC */,
    1000+ART_GILETTAR		/* 119 = SN_GILETTAR */,
    1000+ART_RILIA		/* 120 = SN_RILIA */,
    1000+ART_BELANGIL		/* 121 = SN_BELANGIL */,
    1000+ART_BALLI		/* 122 = SN_BALLI */,
    1000+ART_LOTHARANG		/* 123 = SN_LOTHARANG */,
    1000+ART_FIRESTAR		/* 124 = SN_FIRESTAR */,
    1000+ART_ERIRIL		/* 125 = SN_ERIRIL */,
    1000+ART_CUBRAGOL		/* 126 = SN_CUBRAGOL */,
    1000+ART_BARD		/* 127 = SN_BARD */,
    1000+ART_COLLUIN		/* 128 = SN_COLLUIN */,
    1000+ART_HOLCOLLETH		/* 129 = SN_HOLCOLLETH */,
    1000+ART_TOTILA		/* 130 = SN_TOTILA */,
    1000+ART_PAIN		/* 131 = SN_PAIN */,
    1000+ART_ELVAGIL		/* 132 = SN_ELVAGIL */,
    1000+ART_AGLARANG		/* 133 = SN_AGLARANG */,
    1000+ART_ROHIRRIM		/* 134 = SN_ROHIRRIM */,
    1000+ART_EORLINGAS		/* 135 = SN_EORLINGAS */,
    1000+ART_BARUKKHELED	/* 136 = SN_BARUKKHELED */,
    1000+ART_WRATH		/* 137 = SN_WRATH */,
    1000+ART_HARADEKKET		/* 138 = SN_HARADEKKET */,
    1000+ART_MUNDWINE		/* 139 = SN_MUNDWINE */,
    1000+ART_GONDRICAM		/* 140 = SN_GONDRICAM */,
    1000+ART_ZARCUTHRA		/* 141 = SN_ZARCUTHRA */,
    1000+ART_CARETH		/* 142 = SN_CARETH */,
    1000+ART_FORASGIL		/* 143 = SN_FORASGIL */,
    1000+ART_CRISDURIAN		/* 144 = SN_CRISDURIAN */,
    1000+ART_COLANNON		/* 145 = SN_COLANNON */,
    1000+ART_HITHLOMIR		/* 146 = SN_HITHLOMIR */,
    1000+ART_THALKETTOTH	/* 147 = SN_THALKETTOTH */,
    1000+ART_ARVEDUI		/* 148 = SN_ARVEDUI */,
    1000+ART_THRANDUIL		/* 149 = SN_THRANDUIL */,
    1000+ART_THENGEL		/* 150 = SN_THENGEL */,
    1000+ART_HAMMERHAND		/* 151 = SN_HAMMERHAND */,
    1000+ART_CELEGORM		/* 152 = SN_CELEGORM */,
    1000+ART_THROR		/* 153 = SN_THROR */,
    1000+ART_MAEDHROS		/* 154 = SN_MAEDHROS */,
    1000+ART_OLORIN		/* 155 = SN_OLORIN */,
    1000+ART_ANGUIREL		/* 156 = SN_ANGUIREL */,
    1000+ART_THORIN		/* 157 = SN_THORIN */,
    1000+ART_CELEBORN		/* 158 = SN_CELEBORN */,
    1000+ART_OROME		/* 159 = SN_OROME */,
    1000+ART_EONWE		/* 160 = SN_EONWE */,
    1000+ART_GONDOR		/* 161 = SN_GONDOR */,
    1000+ART_THEODEN		/* 162 = SN_THEODEN */,
    1000+ART_THINGOL		/* 163 = SN_THINGOL */,
    1000+ART_THORONGIL		/* 164 = SN_THORONGIL */,
    1000+ART_LUTHIEN		/* 165 = SN_LUTHIEN */,
    1000+ART_TUOR		/* 166 = SN_TUOR */,
    1000+ART_ULMO		/* 167 = SN_ULMO */,
    1000+ART_OSONDIR		/* 168 = SN_OSONDIR */,
    1000+ART_TURMIL		/* 169 = SN_TURMIL */,
    1000+ART_CASPANION		/* 170 = SN_CASPANION */,
    1000+ART_TIL		/* 171 = SN_TIL */,
    1000+ART_DEATHWREAKER	/* 172 = SN_DEATHWREAKER */,
    1000+ART_AVAVIR		/* 173 = SN_AVAVIR */,
    1000+ART_TARATOL		/* 174 = SN_TARATOL */,
    1000+ART_RAZORBACK		/* 175 = SN_RAZORBACK */,
    1000+ART_BLADETURNER	/* 176 = SN_BLADETURNER */,
    2000+EGO_SHATTERED		/* 177 = SN_SHATTERED */,
    2000+EGO_BLASTED		/* 178 = SN_BLASTED */,
    2000+EGO_ATTACKS		/* 179 = SN_ATTACKS */,
};



/*
 * Convert the old savefile "flags" into the new ones
 * This just converts the flags that are the same, see
 * "rd_item_old()" for total conversion method.
 */
static void repair_item_flags_old(inven_type *i_ptr)
{
    int i = 0;
    u32b f1 = i_ptr->flags1;
    u32b f2 = i_ptr->flags2;


    /* Wipe the flags */
    i_ptr->flags1 = i_ptr->flags2 = i_ptr->flags3 = 0L;


    /*** Old Flag Set #1 ***/

    if (f1 & 0x00000001L) i_ptr->flags1 |= TR1_STR;
    if (f1 & 0x00000002L) i_ptr->flags1 |= TR1_INT;
    if (f1 & 0x00000004L) i_ptr->flags1 |= TR1_WIS;
    if (f1 & 0x00000008L) i_ptr->flags1 |= TR1_DEX;
    if (f1 & 0x00000010L) i_ptr->flags1 |= TR1_CON;
    if (f1 & 0x00000020L) i_ptr->flags1 |= TR1_CHR;
    if (f1 & 0x00000040L) i_ptr->flags1 |= TR1_SEARCH;
    if (f1 & 0x00000080L) i_ptr->flags3 |= TR3_SLOW_DIGEST;
    if (f1 & 0x00000100L) i_ptr->flags1 |= TR1_STEALTH;
    if (f1 & 0x00000200L) i_ptr->flags3 |= TR3_AGGRAVATE;
    if (f1 & 0x00000400L) i_ptr->flags3 |= TR3_TELEPORT;
    if (f1 & 0x00000800L) i_ptr->flags3 |= TR3_REGEN;
    if (f1 & 0x00001000L) i_ptr->flags1 |= TR1_SPEED;
    if (f1 & 0x00002000L) i_ptr->flags1 |= TR1_SLAY_DRAGON;
    if (f1 & 0x00004000L) i_ptr->flags1 |= TR1_SLAY_ANIMAL;
    if (f1 & 0x00008000L) i_ptr->flags1 |= TR1_SLAY_EVIL;
    if (f1 & 0x00010000L) i_ptr->flags1 |= TR1_SLAY_UNDEAD;
    if (f1 & 0x00020000L) i_ptr->flags1 |= TR1_BRAND_COLD;
    if (f1 & 0x00040000L) i_ptr->flags1 |= TR1_BRAND_FIRE;
    if (f1 & 0x00080000L) i_ptr->flags2 |= TR2_RES_FIRE;
    if (f1 & 0x00100000L) i_ptr->flags2 |= TR2_RES_ACID;
    if (f1 & 0x00200000L) i_ptr->flags2 |= TR2_RES_COLD;
    if (f1 & 0x00400000L) i = i; /* SUST_STAT extracted already */
    if (f1 & 0x00800000L) i_ptr->flags2 |= TR2_FREE_ACT;
    if (f1 & 0x01000000L) i_ptr->flags3 |= TR3_SEE_INVIS;
    if (f1 & 0x02000000L) i_ptr->flags2 |= TR2_RES_ELEC;
    if (f1 & 0x04000000L) i_ptr->flags3 |= TR3_FEATHER;
    if (f1 & 0x08000000L) i_ptr->flags1 |= TR1_KILL_DRAGON;
    if (f1 & 0x10000000L) i_ptr->flags2 |= TR2_RES_POIS;
    if (f1 & 0x20000000L) i_ptr->flags1 |= TR1_TUNNEL;
    if (f1 & 0x40000000L) i_ptr->flags1 |= TR1_INFRA;
    if (f1 & 0x80000000L) i_ptr->flags3 |= TR3_CURSED;


    /*** Old Flag Set #2 ***/

    if (f2 & 0x00000001L) i_ptr->flags1 |= TR1_SLAY_DEMON;
    if (f2 & 0x00000002L) i_ptr->flags1 |= TR1_SLAY_TROLL;
    if (f2 & 0x00000004L) i_ptr->flags1 |= TR1_SLAY_GIANT;
    if (f2 & 0x00000008L) i_ptr->flags2 |= TR2_HOLD_LIFE;
    if (f2 & 0x00000010L) i_ptr->flags1 |= TR1_SLAY_ORC;
    if (f2 & 0x00000020L) i_ptr->flags3 |= TR3_TELEPATHY;
    if (f2 & 0x00000040L) i_ptr->flags2 |= TR2_IM_FIRE;
    if (f2 & 0x00000080L) i_ptr->flags2 |= TR2_IM_COLD;
    if (f2 & 0x00000100L) i_ptr->flags2 |= TR2_IM_ACID;
    if (f2 & 0x00000200L) i_ptr->flags2 |= TR2_IM_ELEC;
    if (f2 & 0x00000400L) i_ptr->flags3 |= TR3_LITE;
    if (f2 & 0x00000800L) i_ptr->flags3 |= TR3_ACTIVATE;
    if (f2 & 0x00001000L) i_ptr->flags1 |= TR1_BRAND_ELEC;
    if (f2 & 0x00002000L) i_ptr->flags1 |= TR1_IMPACT;
    if (f2 & 0x00004000L) i_ptr->flags2 |= TR2_IM_POIS;
    if (f2 & 0x00008000L) i_ptr->flags2 |= TR2_RES_CONF;
    if (f2 & 0x00010000L) i_ptr->flags2 |= TR2_RES_SOUND;
    if (f2 & 0x00020000L) i_ptr->flags2 |= TR2_RES_LITE;
    if (f2 & 0x00040000L) i_ptr->flags2 |= TR2_RES_DARK;
    if (f2 & 0x00080000L) i_ptr->flags2 |= TR2_RES_CHAOS;
    if (f2 & 0x00100000L) i_ptr->flags2 |= TR2_RES_DISEN;
    if (f2 & 0x00200000L) i_ptr->flags2 |= TR2_RES_SHARDS;
    if (f2 & 0x00400000L) i_ptr->flags2 |= TR2_RES_NEXUS;
    if (f2 & 0x00800000L) i_ptr->flags2 |= TR2_RES_BLIND;
    if (f2 & 0x01000000L) i_ptr->flags2 |= TR2_RES_NETHER;
    if (f2 & 0x02000000L) i = i; /* Artifact Flag extracted into "name1" */
    if (f2 & 0x04000000L) i_ptr->flags3 |= TR3_BLESSED;
    if (f2 & 0x08000000L) i_ptr->flags1 |= TR1_ATTACK_SPD;
    if (f2 & 0x10000000L) i = i; /* TR2_RES_FEAR no longer used */
}


/*
 * Read an old-version "item" structure
 */
static errr rd_item_old(inven_type *i_ptr)
{
    byte tmp8u;
    u16b tmp16u;
    char note[128];

    bool invis_trap = FALSE;

    inven_kind *k_ptr;

    rd_u16b(&i_ptr->k_idx);
    rd_byte(&i_ptr->name2);
    rd_string(note);
    inscribe(i_ptr, note);
    rd_u32b(&i_ptr->flags1);
    rd_byte(&i_ptr->tval);

    /* Was: i_ptr->tchar */
    rd_byte(&tmp8u);

    rd_s16b(&i_ptr->pval);
    rd_s32b(&i_ptr->cost);
    rd_byte(&i_ptr->sval);
    rd_byte(&i_ptr->number);

    rd_u16b(&tmp16u);
    i_ptr->weight = tmp16u;

    rd_s16b(&i_ptr->tohit);
    rd_s16b(&i_ptr->todam);
    rd_s16b(&i_ptr->ac);
    rd_s16b(&i_ptr->toac);
    rd_byte(&i_ptr->dd);
    rd_byte(&i_ptr->ds);

    /* Forget old "level" */
    rd_byte(&tmp8u);

    rd_byte(&i_ptr->ident);
    rd_u32b(&i_ptr->flags2);

    /* Read and forget the old timeout */    
    rd_u16b(&tmp16u);

    /* Clear the timeout */
    i_ptr->timeout = 0;

    /* Harmless Hack -- clear scost */
    i_ptr->scost = 0;

    /* Default to blank "flags3" */
    i_ptr->flags3 = 0L;


    /* XXX Potions are now a single tval, but use both flags */
    if (i_ptr->tval == TV_POTION + 1) {
	i_ptr->tval = TV_POTION;
	i_ptr->flags2 = i_ptr->flags1;
	i_ptr->flags1 = 0L;
    }

    /* XXX Scrolls are now a single tval, but use both flags */
    if (i_ptr->tval == TV_SCROLL + 1) {
	i_ptr->tval = TV_SCROLL;
	i_ptr->flags2 = i_ptr->flags1;
	i_ptr->flags1 = 0L;
    }


    /* Several objects now have new "locations" */
    switch (i_ptr->k_idx) {

	/* Items map to their "base" */
	case 0: i_ptr->k_idx = 15; break;
	case 13: i_ptr->k_idx = 12; break;
	case 14: i_ptr->k_idx = 12; break;
	case 22: i_ptr->k_idx = 21; break;
	case 23: i_ptr->k_idx = 21; break;
	case 26: i_ptr->k_idx = 25; break;
	case 27: i_ptr->k_idx = 25; break;

	/* Cleaning of Edged Blades */
	case 28: i_ptr->k_idx = 38; break;
	case 29: i_ptr->k_idx = 43; break;

	/* Items map to their "base" */
	case 35: i_ptr->k_idx = 34; break;
	case 38: i_ptr->k_idx = 37; break;
	case 43: i_ptr->k_idx = 42; break;

	/* Separate Hafted from Polearm */
	case 58: i_ptr->k_idx = 62; break;
	case 62: i_ptr->k_idx = 58; break;

	/* Spikes, Lantern, Torches */
	case 84: i_ptr->k_idx = 345; break;
	case 85: i_ptr->k_idx = 347; break;
	case 86: i_ptr->k_idx = 346; break;

	/* Dungeon Shovels and Picks */
	case 87: i_ptr->k_idx = 88; break;
	case 88: i_ptr->k_idx = 89; break;
	case 89: i_ptr->k_idx = 85; break;
	case 90: i_ptr->k_idx = 86; break;

	/* More items mapping to their "base" */        
	case 102: i_ptr->k_idx = 101; break;
	case 144: i_ptr->k_idx = 78; break;
	case 170: i_ptr->k_idx = 80; break;
	case 177: i_ptr->k_idx = 176; break;
	case 178: i_ptr->k_idx = 176; break;
	case 179: i_ptr->k_idx = 176; break;
	case 182: i_ptr->k_idx = 181; break;
	case 183: i_ptr->k_idx = 181; break;
	case 191: i_ptr->k_idx = 190; break;
	case 195: i_ptr->k_idx = 194; break;

	/* Random Rod */
	case 196: i_ptr->k_idx = 352; break;

	/* More items linking to "base" */
	case 198: i_ptr->k_idx = 197; break;
	case 199: i_ptr->k_idx = 197; break;
	case 205: i_ptr->k_idx = 214; break;
	case 238: i_ptr->k_idx = 237; break;
	case 239: i_ptr->k_idx = 237; break;
	case 256: i_ptr->k_idx = 249; break;
	case 284: i_ptr->k_idx = 283; break;

	/* Flask of oil */
	case 268: i_ptr->k_idx = 348; break;

	/* Random Staff */
	case 293: i_ptr->k_idx = 306; break;

	/* Random Staff */
	case 299: i_ptr->k_idx = 316; break;

	/* More items linking to "base" */
	case 306: i_ptr->k_idx = 305; break;
	case 316: i_ptr->k_idx = 307; break;
	case 321: i_ptr->k_idx = 322; break;

	/* Miscellaneous Junk */
	case 345: i_ptr->k_idx = 389; break;
	case 346: i_ptr->k_idx = 390; break;

	/* Skeletons */
	case 353: i_ptr->k_idx = 391; break;
	case 354: i_ptr->k_idx = 392; break;
	case 347: i_ptr->k_idx = 393; break;
	case 348: i_ptr->k_idx = 394; break;
	case 349: i_ptr->k_idx = 395; break;
	case 350: i_ptr->k_idx = 396; break;
	case 351: i_ptr->k_idx = 397; break;
	case 352: i_ptr->k_idx = 398; break;

	/* Empty bottle */
	case 355: i_ptr->k_idx = 349; break;

	/* Random Scroll */
	case 378: i_ptr->k_idx = 191; break;

	/* Dragon Scale Mail */
	case 389: i_ptr->k_idx = 401; break;
	case 390: i_ptr->k_idx = 402; break;
	case 391: i_ptr->k_idx = 400; break;
	case 392: i_ptr->k_idx = 404; break;
	case 393: i_ptr->k_idx = 403; break;
	case 394: i_ptr->k_idx = 405; break;

	/* Random Daggers */
	case 395: i_ptr->k_idx = 43; break;
	case 396: i_ptr->k_idx = 43; break;
	case 397: i_ptr->k_idx = 43; break;

	/* Random Short Sword */
	case 398: i_ptr->k_idx = 35; break;

	/* Random leftover potions */
	case 399: i_ptr->k_idx = 422; break;
	case 400: i_ptr->k_idx = 417; break;
	case 401: i_ptr->k_idx = 415; break;
	case 402: i_ptr->k_idx = 420; break;
	case 403: i_ptr->k_idx = 418; break;
	case 404: i_ptr->k_idx = 416; break;

	/* Random Rods */  
	case 405: i_ptr->k_idx = 355; break;
	case 406: i_ptr->k_idx = 353; break;

	/* Random Staff */
	case 407: i_ptr->k_idx = 321; break;

	/* Dragon Scale Mail */
	case 408: i_ptr->k_idx = 408; break;
	case 409: i_ptr->k_idx = 409; break;

	/* Random Rod */
	case 410: i_ptr->k_idx = 354; break;

	/* Random Cloak */
	case 411: i_ptr->k_idx = 123; break;

	/* Random Scrolls */
	case 412: i_ptr->k_idx = 198; break;
	case 413: i_ptr->k_idx = 199; break;

	/* Random Ring */
	case 414: i_ptr->k_idx = 144; break;

	/* Dragon Scale Mail */
	case 415: i_ptr->k_idx = 410; break;
	case 416: i_ptr->k_idx = 407; break;
	case 417: i_ptr->k_idx = 411; break;
	case 418: i_ptr->k_idx = 406; break;
	case 419: i_ptr->k_idx = 412; break;

	/* More random potions */
	case 420: i_ptr->k_idx = 256; break;
	case 421: i_ptr->k_idx = 421; break;
	case 422: i_ptr->k_idx = 419; break;

	/* Store bought food */
	case 423: i_ptr->k_idx = 21; break;
	case 424: i_ptr->k_idx = 22; break;
	case 425: i_ptr->k_idx = 23; break;
	case 426: i_ptr->k_idx = 26; break;
	case 427: i_ptr->k_idx = 27; break;

	/* Storebought Shovels/Picks */
	case 428: i_ptr->k_idx = 87; break;
	case 429: i_ptr->k_idx = 84; break;

	/* Many old shop items mapped to real items */
	case 430: i_ptr->k_idx = 176; break;
	case 431: i_ptr->k_idx = 181; break;
	case 432: i_ptr->k_idx = 185; break;
	case 433: i_ptr->k_idx = 189; break;
	case 434: i_ptr->k_idx = 192; break;
	case 435: i_ptr->k_idx = 193; break;
	case 436: i_ptr->k_idx = 201; break;
	case 437: i_ptr->k_idx = 217; break;
	case 438: i_ptr->k_idx = 220; break;
	case 439: i_ptr->k_idx = 237; break;
	case 440: i_ptr->k_idx = 257; break;
	case 441: i_ptr->k_idx = 259; break;
	case 442: i_ptr->k_idx = 264; break;

	/* Old shop items -- lantern, torch, oil */
	case 443: i_ptr->k_idx = 347; break;
	case 444: i_ptr->k_idx = 346; break;
	case 445: i_ptr->k_idx = 348; break;

	/* Clean up the traps, the mush, and the rubble */
	case 459: i_ptr->k_idx = 460; break;
	case 460: i_ptr->k_idx = 477; break;
	case 477: i_ptr->k_idx = 445; break;
	case 478: i_ptr->k_idx = 21; break;
	case 479: i_ptr->k_idx = 459; break;

	/* The old "nothing" object */
	case 498: i_ptr->k_idx = 0; break;

	/* Ruined Chest */
	case 499: i_ptr->k_idx = 344; break;

	/* Special objects */
	case 501: i_ptr->k_idx = 508; break;
	case 502: i_ptr->k_idx = 509; break;
	case 503: i_ptr->k_idx = 510; break;
	case 504: i_ptr->k_idx = 511; break;
	case 505: i_ptr->k_idx = 500; break;
	case 506: i_ptr->k_idx = 504; break;
	case 507: i_ptr->k_idx = 503; break;
	case 508: i_ptr->k_idx = 501; break;
	case 509: i_ptr->k_idx = 502; break;
	case 510: i_ptr->k_idx = 507; break;
	case 511: i_ptr->k_idx = 505; break;
	case 512: i_ptr->k_idx = 506; break;
    }


    /* The "Special Objects" now have Artifact Names */
    switch (i_ptr->k_idx) {
	case OBJ_GALADRIEL: i_ptr->name1 = ART_GALADRIEL; break;
	case OBJ_ELENDIL: i_ptr->name1 = ART_ELENDIL; break;
	case OBJ_THRAIN: i_ptr->name1 = ART_THRAIN; break;
	case OBJ_CARLAMMAS: i_ptr->name1 = ART_CARLAMMAS; break;
	case OBJ_INGWE: i_ptr->name1 = ART_INGWE; break;
	case OBJ_DWARVES: i_ptr->name1 = ART_DWARVES; break;
	case OBJ_BARAHIR: i_ptr->name1 = ART_BARAHIR; break;
	case OBJ_TULKAS: i_ptr->name1 = ART_TULKAS; break;
	case OBJ_NARYA: i_ptr->name1 = ART_NARYA; break;
	case OBJ_NENYA: i_ptr->name1 = ART_NENYA; break;
	case OBJ_VILYA: i_ptr->name1 = ART_VILYA; break;
	case OBJ_POWER: i_ptr->name1 = ART_POWER; break;
    }

    /* Artifact Names Dominate Ego-Item Names */
    if (i_ptr->name1) i_ptr->name2 = 0;


    /* The Old Special Names induce Artifact Names */
    if (i_ptr->name2) {

	int hack;

	/* Analyze the old "special name" */
	hack = convert_name2[i_ptr->name2];

	/* Old "Chest" names */
	if (hack > 9000) {

	    /* Extract a couple meaningful names */
	    if (hack == CHE_DISARMED) {
		i_ptr->flags2 |= CH2_DISARMED;
	    }

	    /* Forget the old name */
	    i_ptr->name2 = 0;
	}

	/* It is an ego-item */        
	else if (hack > 2000) {

	    /* Move it elsewhere in the table */
	    i_ptr->name2 = (hack - 2000);
	}

	/* It is an artifact */        
	else if (hack > 1000) {

	    /* Move it into the artifact table */
	    i_ptr->name1 = (hack - 1000);

	    /* Forget the old name */
	    i_ptr->name2 = 0;
	}

	/* Oops.  That name no longer exists... */
	else {
	    message("Ignoring illegal 'name2' field", 0);
	    i_ptr->name2 = 0;
	}
    }


    /* Access the (possibly new) item template */
    k_ptr = &k_list[i_ptr->k_idx];

    /* Take note if the object was an invisible trap */
    if (i_ptr->tval == TV_INVIS_TRAP) invis_trap = TRUE;

    /* Hack -- repair "tval" */
    i_ptr->tval = k_ptr->tval;

    /* Hack -- repair "sval" */
    i_ptr->sval = k_ptr->sval;

    /* XXX Hack -- un-repair invisible traps */
    if (invis_trap) i_ptr->tval = TV_INVIS_TRAP;


    /* XXX Wands, Staffs, and Rods have no flags */
    if ((i_ptr->tval == TV_WAND) ||
	(i_ptr->tval == TV_STAFF) ||
	(i_ptr->tval == TV_ROD)) {
	i_ptr->flags1 = i_ptr->flags2 = 0L;
    }


    /* Chests now put the traps in flags2 */
    if (i_ptr->tval == TV_CHEST) {

	i_ptr->flags2 = i_ptr->flags1 & CH2_TRAP_MASK;
	i_ptr->flags1 &= ~CH2_TRAP_MASK;

	if (i_ptr->flags1 & CH2_LOCKED) {
	    i_ptr->flags2 |= CH2_LOCKED;
	    i_ptr->flags1 &= ~CH2_LOCKED;
	}
    }


    /* Forget about missile_ctr */
    if ((i_ptr->tval == TV_SPIKE) || (i_ptr->tval == TV_SHOT) ||
	(i_ptr->tval == TV_ARROW) || (i_ptr->tval == TV_BOLT)) {

	/* Forget "missile_ctr" */
	i_ptr->pval = 0;
    }


    /* Update Bows (XXX Handle Bows of Extra Might) */
    if (i_ptr->tval == TV_BOW) {

	/* Hack -- repair "pval" (ancient "bug") */
	i_ptr->pval = 0;

	/* XXX XXX Normal bows of extra might */
	/* i_ptr->flags3 |= (TR3_XTRA_MIGHT); */
    }



    /* Repair the "flags" in wearable objects */
    if (wearable_p(i_ptr)) {


	/* Save the "Sustain Stat flags" */
	u32b sustain2 = 0L;


	/* Extract old "TR_SUST_STAT" flag, save it for later */
	if (i_ptr->flags1 & 0x00400000L) {

	    /* Hack -- multi-sustain2 */
	    if (i_ptr->pval == 10) {

		/* Sustain everything */
		sustain2 |= (TR2_SUST_STR | TR2_SUST_DEX | TR2_SUST_CON |
			     TR2_SUST_INT | TR2_SUST_WIS | TR2_SUST_CHR);

		/* Forget the bogus pval */
		i_ptr->pval = 0;
	    }

	    /* Give a normal sustain2, keep the pval */
	    switch (i_ptr->pval) {
		case 1: sustain2 |= (TR2_SUST_STR); break;
		case 2: sustain2 |= (TR2_SUST_INT); break;
		case 3: sustain2 |= (TR2_SUST_WIS); break;
		case 4: sustain2 |= (TR2_SUST_DEX); break;
		case 5: sustain2 |= (TR2_SUST_CON); break;
		case 6: sustain2 |= (TR2_SUST_CHR); break;
	    }

	    /* Hack -- If the "pval" was "hidden", forget it */
	    if (i_ptr->ident & 0x40) i_ptr->pval = 0;
	}

	/* Completely repair old flags */
	repair_item_flags_old(i_ptr);

	/* Drop in the new "Sustain Stat" flags */
	i_ptr->flags2 |= sustain2;
    }



    /*** Analyze the old "ident" flags (and friends) ***/

    /* Some of the old "ident" flags only apply to wearable's */    
    if (wearable_p(i_ptr)) {

	/* Convert "ID_SHOW_HITDAM" to "TR3_SHOW_MODS" */
	if (i_ptr->ident & 0x20) i_ptr->flags3 |= TR3_SHOW_MODS;

	/* Convert "ID_NOSHOW_TYPE" to "TR3_HIDE_TYPE" */
	if (i_ptr->ident & 0x80) i_ptr->flags3 |= TR3_HIDE_TYPE;

	/* Hack -- Inherit "EASY_KNOW" from parent */
	if (k_ptr->flags3 & TR3_EASY_KNOW) i_ptr->flags3 |= TR3_EASY_KNOW;
    }

    /* Convert old "ID_DAMD" flag into new "ID_FELT" method */
    if (i_ptr->ident & 0x02) {
	i_ptr->ident |= ID_FELT;
	i_ptr->ident &= ~0x02;
    }

    /* Forget old "ID_STOREBOUGHT" flag, if any */
    i_ptr->ident &= ~0x10;

    /* Forget old "ID_SHOW_HITDAM" flag, if any */
    i_ptr->ident &= ~0x20;

    /* Forget old "ID_NOSHOW_P1" flag, if any */
    i_ptr->ident &= ~0x40;   

    /* Forget old "ID_HIDE_TYPE" flag, if any */
    i_ptr->ident &= ~0x80;


    /*** Miscelaneous "Cleanup" ***/

    /* Hack -- And "identify" a few things */
    if ((i_ptr->tval == TV_SKELETON) ||
	(i_ptr->tval == TV_BOTTLE) ||
	(i_ptr->tval == TV_JUNK) ||
	(i_ptr->tval == TV_FLASK) ||
	(i_ptr->tval == TV_SPIKE)) {

	/* Fully know the item */
	known2(i_ptr);
    }


    /*** Analyze wearable items some more ***/

    /* Stop processing unless wearable */
    if (!wearable_p(i_ptr)) return (0);

    /* Dangerous Hack -- update speed */
    if (i_ptr->flags1 & TR1_SPEED) {
	i_ptr->pval = i_ptr->pval * 10;
    }
    
    /* Hack -- Ego-Items of Morgul */
    if (i_ptr->name2 == EGO_MORGUL) {
	i_ptr->flags3 |= TR3_HEAVY_CURSE;
    }

    /* Update artifacts */
    if (i_ptr->name1) {

	/* Repair -- Acquire new artifact flags/fields */
	i_ptr->flags1 = v_list[i_ptr->name1].flags1;
	i_ptr->flags2 = v_list[i_ptr->name1].flags2;
	i_ptr->flags3 = v_list[i_ptr->name1].flags3;
	i_ptr->pval = v_list[i_ptr->name1].pval;
	i_ptr->ac = v_list[i_ptr->name1].ac;
	i_ptr->dd = v_list[i_ptr->name1].dd;
	i_ptr->ds = v_list[i_ptr->name1].ds;
	i_ptr->weight = v_list[i_ptr->name1].weight;
	i_ptr->cost = v_list[i_ptr->name1].cost;
    }

    /* Update dragon scale mail */
    else if (i_ptr->tval == TV_DRAG_ARMOR) {

	/* Repair -- Acquire new artifact flags/fields */
	i_ptr->flags1 = k_list[i_ptr->k_idx].flags1;
	i_ptr->flags2 = k_list[i_ptr->k_idx].flags2;
	i_ptr->flags3 = k_list[i_ptr->k_idx].flags3;
	i_ptr->pval = k_list[i_ptr->k_idx].pval;
	i_ptr->ac = k_list[i_ptr->k_idx].ac;
	i_ptr->dd = k_list[i_ptr->k_idx].dd;
	i_ptr->ds = k_list[i_ptr->k_idx].ds;
	i_ptr->weight = k_list[i_ptr->k_idx].weight;
    }

    /* Extract new "ignore" settings */
    else {

	/* Extract new "ignore" settings from "Resist" */
	if (i_ptr->flags2 & TR2_RES_ACID) i_ptr->flags3 |= TR3_IGNORE_ACID;
	if (i_ptr->flags2 & TR2_RES_ELEC) i_ptr->flags3 |= TR3_IGNORE_ELEC;
	if (i_ptr->flags2 & TR2_RES_FIRE) i_ptr->flags3 |= TR3_IGNORE_FIRE;
	if (i_ptr->flags2 & TR2_RES_COLD) i_ptr->flags3 |= TR3_IGNORE_COLD;

	/* Extract new "ignore" settings from "Immune" */
	if (i_ptr->flags2 & TR2_IM_ACID) i_ptr->flags3 |= TR3_IGNORE_ACID;
	if (i_ptr->flags2 & TR2_IM_ELEC) i_ptr->flags3 |= TR3_IGNORE_ELEC;
	if (i_ptr->flags2 & TR2_IM_FIRE) i_ptr->flags3 |= TR3_IGNORE_FIRE;
	if (i_ptr->flags2 & TR2_IM_COLD) i_ptr->flags3 |= TR3_IGNORE_COLD;
    }


    /* Success */
    return (0);
}


/*
 * Read and Write monsters
 */

static void rd_monster_old(monster_type *m_ptr)
{
    byte	tmp8u;
    u16b	tmp16u;
    
    /* Read the current hitpoints */
    rd_s16b(&m_ptr->hp);

    if (older_than(2,6,0)) {
	/* Hack -- see below as well */
	m_ptr->maxhp = m_ptr->hp;
    }
    else {
	/* Read the maximal hitpoints */
	rd_s16b(&m_ptr->maxhp);
    }

    rd_s16b(&m_ptr->csleep);

    rd_u16b(&tmp16u);	/* Old speed */

    rd_u16b(&m_ptr->r_idx);

    rd_byte(&m_ptr->fy);
    rd_byte(&m_ptr->fx);

    /* Verify the bug fix above */
    if (m_ptr->maxhp <= 0) {

	if (r_list[m_ptr->r_idx].cflags2 & MF2_MAX_HP) {
	    m_ptr->maxhp = max_hp(r_list[m_ptr->r_idx].hd);
	}
	else {
	    m_ptr->maxhp = pdamroll(r_list[m_ptr->r_idx].hd);
	}
    }

    rd_byte(&tmp8u); /* ignore saved "m_ptr->cdis" */
    rd_byte(&tmp8u); /* ignore saved "m_ptr->ml" */

    rd_byte(&m_ptr->stunned);
    rd_byte(&m_ptr->confused);

    if (older_than(2,6,0)) {
	/* Clear the monster fear value */
	m_ptr->monfear = 0;
    }
    else {
	/* Read the monster fear */
	rd_byte(&m_ptr->monfear);
    }

    /* Hack -- recalculate speed */
    m_ptr->mspeed = r_list[m_ptr->r_idx].speed;
    m_ptr->energy = rand_int(100);
}



/*
 * Read the old lore
 */
static void rd_lore_old(monster_lore *l_ptr)
{
    rd_u32b(&l_ptr->r_cflags1);
    rd_u32b(&l_ptr->r_spells1);
    rd_u32b(&l_ptr->r_spells2);
    rd_u32b(&l_ptr->r_spells3);
    rd_u16b(&l_ptr->r_kills);
    rd_u16b(&l_ptr->r_deaths);
    rd_u32b(&l_ptr->r_cflags2);
    rd_byte(&l_ptr->r_wake);
    rd_byte(&l_ptr->r_ignore);
    rd_byte(&l_ptr->r_attacks[0]);
    rd_byte(&l_ptr->r_attacks[1]);
    rd_byte(&l_ptr->r_attacks[2]);
    rd_byte(&l_ptr->r_attacks[3]);

    /* XXX Remember to "extract" max_num later */

    /* Hack -- Extract (and clear) the "treasure count" bits */
    l_ptr->r_drop = (l_ptr->r_cflags1 & CM1_TREASURE) >> CM1_TR_SHIFT;
    l_ptr->r_cflags1 &= ~CM1_TREASURE;

    /* Hack -- Extract (and clear) the "spell count" bits */        
    l_ptr->r_cast = (l_ptr->r_spells1 & CS1_FREQ);
    l_ptr->r_spells1 &= ~CS1_FREQ;
}




static errr rd_store_old(store_type *st_ptr)
{
    int j;

    rd_s32b(&st_ptr->store_open);
    rd_s16b(&st_ptr->insult_cur);
    rd_byte(&st_ptr->owner);
    rd_byte(&st_ptr->store_ctr);
    rd_u16b(&st_ptr->good_buy);
    rd_u16b(&st_ptr->bad_buy);

    /* Too many items */    
    if (st_ptr->store_ctr > STORE_INVEN_MAX) {
	prt_note(-2, "Too many items in store");
	return (10);
    }

    /* Read the items (and costs) */
    for (j = 0; j < st_ptr->store_ctr; j++) {
	s32b scost;
	rd_s32b(&scost);
	rd_item_old(&st_ptr->store_item[j]);
	st_ptr->store_item[j].scost = scost;
    }

    /* Success */
    return (0);
}



/*
 * Read the artifacts -- old version
 */

static void rd_artifacts_old()
{
    u32b tmp32u;

    v_list[ART_MORGOTH].cur_num = 0;

    rd_u32b(&tmp32u); v_list[ART_GROND].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_RINGIL].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_AEGLOS].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_ARUNRUTH].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_MORMEGIL].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_ANGRIST].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_GURTHANG].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_CALRIS].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_ANDURIL].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_STING].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_ORCRIST].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_GLAMDRING].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_DURIN].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_AULE].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_THUNDERFIST].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_BLOODSPIKE].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_DOOMCALLER].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_NARTHANC].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_NIMTHANC].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_DETHANC].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_GILETTAR].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_RILIA].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_BELANGIL].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_BALLI].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_LOTHARANG].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_FIRESTAR].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_ERIRIL].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_CUBRAGOL].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_BARD].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_COLLUIN].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_HOLCOLLETH].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_TOTILA].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_PAIN].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_ELVAGIL].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_AGLARANG].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_EORLINGAS].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_BARUKKHELED].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_WRATH].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_HARADEKKET].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_MUNDWINE].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_GONDRICAM].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_ZARCUTHRA].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_CARETH].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_FORASGIL].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_CRISDURIAN].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_COLANNON].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_HITHLOMIR].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_THALKETTOTH].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_ARVEDUI].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_THRANDUIL].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_THENGEL].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_HAMMERHAND].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_CELEGORM].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_THROR].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_MAEDHROS].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_OLORIN].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_ANGUIREL].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_OROME].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_EONWE].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_THEODEN].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_ULMO].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_OSONDIR].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_TURMIL].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_CASPANION].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_TIL].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_DEATHWREAKER].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_AVAVIR].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_TARATOL].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_DOR].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_NENYA].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_NARYA].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_VILYA].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_BELEGENNON].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_FEANOR].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_ISILDUR].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_SOULKEEPER].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_FINGOLFIN].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_ANARION].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_POWER].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_GALADRIEL].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_BELTHRONDING].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_DAL].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_PAURHACH].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_PAURNIMMEN].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_PAURAEGEN].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_PAURNEN].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_CAMMITHRIM].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_CAMBELEG].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_INGWE].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_CARLAMMAS].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_HOLHENNETH].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_AEGLIN].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_CAMLOST].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_NIMLOTH].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_NAR].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_BERUTHIEL].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_GORLIM].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_ELENDIL].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_THORIN].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_CELEBORN].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_THRAIN].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_GONDOR].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_THINGOL].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_THORONGIL].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_LUTHIEN].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_TUOR].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_ROHIRRIM].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_TULKAS].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_DWARVES].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_BARAHIR].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_RAZORBACK].cur_num = tmp32u;
    rd_u32b(&tmp32u); v_list[ART_BLADETURNER].cur_num = tmp32u;
}





/*
 * Read/Write the "ghost" information
 *
 * Note -- old savefiles do this VERY badly...
 */

static void rd_ghost_old()
{
    monster_race *r_ptr = &r_list[MAX_R_IDX-1];

    int i;
    byte tmp8u;
    u16b tmp16u;
    u32b tmp32u;


    /* A buffer for the ghost name */
    char gname[128];

    /* Hack -- read the name as bytes */
    for (i = 0; i < 100; i++) rd_char(&gname[i]);
    strcpy(ghost_name, gname);

    /* Restore ghost names & stats etc... */
    /* XXX Argh, all of the neat spells are lost */
    /* XXX Stupid order anyway */

    rd_u32b(&tmp32u);
    r_ptr->cflags1 = tmp32u;

    rd_u32b(&tmp32u);
    r_ptr->spells1 = tmp32u;

    rd_u32b(&tmp32u);
    r_ptr->cflags2 = tmp32u;


/*
 * fix player ghost's exp bug.  The mexp field is really an u32b, but the
 * savefile was writing/ reading an u16b.  Since I don't want to change
 * the savefile format, this insures that the mexp field is loaded, and that
 * the "high bits" of mexp do not contain garbage values which could mean that
 * player ghost are worth millions of exp. -CFT
 */

    rd_u16b(&tmp16u);
    r_ptr->mexp = (u32b)(tmp16u);

/*
 * more stupid size bugs that would've never been needed if these variables
 * had been given enough space in the first place -CWS
 */

    if (older_than(2,6,0)) {
	rd_byte(&tmp8u);
	r_ptr->sleep = tmp8u;
    }
    else {
	rd_u16b(&tmp16u);
	r_ptr->sleep = tmp16u;
    }

    rd_byte(&tmp8u);
    r_ptr->aaf = tmp8u;

    if (older_than(2,6,0)) {
	rd_byte(&tmp8u);
	r_ptr->ac = tmp8u;
    }
    else {
	rd_u16b(&tmp16u);
	r_ptr->ac = tmp16u;
    }

    /* Read the speed */
    rd_byte(&tmp8u);
    r_ptr->speed = (tmp8u - 10) * 10 + 100;

    rd_byte(&tmp8u);
    r_ptr->r_char = tmp8u;

    rd_byte(&r_ptr->hd[0]);
    rd_byte(&r_ptr->hd[1]);

    /* Hack -- read the attacks */
    for (i = 0; i < 4; i++) {
	rd_u16b(&r_ptr->damage[i]);
    }

    rd_u16b(&tmp16u);
    r_ptr->level = tmp16u;

    /* Hack -- a white ghost (plus shimmer) */
    r_ptr->r_attr = TERM_WHITE;    
    r_ptr->cflags1 |= MF1_ATTR_MULTI;

    /* Hack -- set the "graphic" info */
    r_attr[MAX_R_IDX-1] = r_ptr->r_attr;
    r_char[MAX_R_IDX-1] = r_ptr->r_char;
}



/*
 * Read the OLD extra information
 */
static void rd_extra_old()
{
    int i;
    byte tmp8u;
    u16b tmp16u;
    s16b tmp16s;
    s32b tmp32s;

    rd_string(player_name);
    rd_byte(&p_ptr->male);
    rd_s32b(&p_ptr->au);
    rd_s32b(&p_ptr->max_exp);
    rd_s32b(&p_ptr->exp);
    rd_u16b(&p_ptr->exp_frac);
    rd_u16b(&p_ptr->age);
    rd_u16b(&p_ptr->ht);
    rd_u16b(&p_ptr->wt);
    rd_s16b(&p_ptr->lev);
    rd_s16b(&p_ptr->max_dlv);
    rd_s16b(&p_ptr->srh);
    rd_s16b(&p_ptr->fos);
    rd_s16b(&p_ptr->bth);
    rd_s16b(&p_ptr->bthb);
    rd_s16b(&p_ptr->mana);
    rd_s16b(&p_ptr->mhp);
    
    /* Ignore old bonuses */
    rd_s16b(&tmp16s);
    rd_s16b(&tmp16s);
    rd_s16b(&tmp16s);
    rd_s16b(&tmp16s);

    /* Ignore old displayed bonuses */
    rd_s16b(&tmp16s);
    rd_s16b(&tmp16s);
    rd_s16b(&tmp16s);
    rd_s16b(&tmp16s);

    rd_s16b(&p_ptr->disarm);
    rd_s16b(&p_ptr->save);
    rd_s16b(&p_ptr->sc);
    rd_s16b(&p_ptr->stl);
    rd_byte(&p_ptr->pclass);
    rd_byte(&p_ptr->prace);
    rd_byte(&p_ptr->hitdie);
    rd_byte(&p_ptr->expfact);
    rd_s16b(&p_ptr->cmana);
    rd_u16b(&p_ptr->cmana_frac);
    rd_s16b(&p_ptr->chp);
    rd_u16b(&p_ptr->chp_frac);

    for (i = 0; i < 4; i++) {
	rd_string(history[i]);
    }

    /* Read the stats */    
    for (i = 0; i < 6; i++) rd_s16b(&p_ptr->max_stat[i]);
    if (older_than(2,5,7)) {
	for (i = 0; i < 6; i++) rd_s16b(&p_ptr->cur_stat[i]);
    }
    else {
	for (i = 0; i < 6; i++) {
	    rd_byte(&tmp8u);
	    p_ptr->cur_stat[i] = tmp8u;
	}
    }
    for (i = 0; i < 6; i++) rd_s16b(&p_ptr->mod_stat[i]);
    for (i = 0; i < 6; i++) rd_s16b(&p_ptr->use_stat[i]);

    /* Read the flags */
    rd_u32b(&p_ptr->status);
    rd_s16b(&p_ptr->rest);
    rd_s16b(&p_ptr->blind);
    rd_s16b(&p_ptr->paralysis);
    rd_s16b(&p_ptr->confused);
    rd_s16b(&p_ptr->food);
    rd_s16b(&p_ptr->food_digested);
    rd_s16b(&p_ptr->protection);
    rd_u16b(&tmp16u);
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
    rd_s16b(&p_ptr->detect_inv);
    rd_s16b(&p_ptr->word_recall);
    rd_s16b(&p_ptr->see_infra);
    rd_s16b(&p_ptr->tim_infra);
    rd_byte(&p_ptr->see_inv);
    rd_byte(&p_ptr->teleport);
    rd_byte(&p_ptr->free_act);
    rd_byte(&p_ptr->slow_digest);
    rd_byte(&p_ptr->aggravate);
    rd_byte(&p_ptr->resist_fire);
    rd_byte(&p_ptr->resist_cold);
    rd_byte(&p_ptr->resist_acid);
    rd_byte(&p_ptr->regenerate);
    rd_byte(&p_ptr->resist_elec);
    rd_byte(&p_ptr->ffall);
    rd_byte(&p_ptr->sustain_str);
    rd_byte(&p_ptr->sustain_int);
    rd_byte(&p_ptr->sustain_wis);
    rd_byte(&p_ptr->sustain_con);
    rd_byte(&p_ptr->sustain_dex);
    rd_byte(&p_ptr->sustain_chr);
    rd_byte(&p_ptr->confusing);
    rd_byte(&p_ptr->new_spells);
    rd_byte(&p_ptr->resist_pois);
    rd_byte(&p_ptr->hold_life);
    rd_byte(&p_ptr->telepathy);
    rd_byte(&p_ptr->immune_fire);
    rd_byte(&p_ptr->immune_acid);
    rd_byte(&p_ptr->immune_pois);
    rd_byte(&p_ptr->immune_cold);
    rd_byte(&p_ptr->immune_elec);
    rd_byte(&p_ptr->lite);
    rd_byte(&p_ptr->resist_conf);
    rd_byte(&p_ptr->resist_sound);
    rd_byte(&p_ptr->resist_lite);
    rd_byte(&p_ptr->resist_dark);
    rd_byte(&p_ptr->resist_chaos);
    rd_byte(&p_ptr->resist_disen);
    rd_byte(&p_ptr->resist_shards);
    rd_byte(&p_ptr->resist_nexus);
    rd_byte(&p_ptr->resist_blind);
    rd_byte(&p_ptr->resist_nether);

    if (older_than(2,6,0)) {
	p_ptr->resist_fear = 0;	/* sigh */
    }
    else {
	rd_byte(&p_ptr->resist_fear);
    }

    /* This is not that important any more */
    p_ptr->searching = 0;
    p_ptr->status &= ~PY_SEARCH;

    /* Old "missile_ctr" */
    rd_u16b(&tmp16u);

    /* Current turn */
    rd_s32b(&tmp32s);
    turn = (tmp32s < 0) ? 0 : tmp32s;

    /* Last turn */
    if (older_than(2,6,0)) {
	old_turn = turn;
    }
    else {
	rd_s32b(&tmp32s);
	old_turn = (tmp32s < 0) ? 0 : tmp32s;
    }
}



/*
 * Read the saved messages
 */
static void rd_messages_old()
{
    int i;
    char buf[1024];

    u16b last_msg;

    /* Hack -- old method used circular queue */
    rd_u16b(&last_msg);

    /* Minor Hack -- may lose some old messages */
    for (i = 0; i < 22; i++) {

	/* Read the message */
	rd_string(buf);

	/* Save (most of) the messages */
	if (buf[0] && (i <= last_msg)) message_new(buf, -1);
    }
}





/*
 * Old method
 */
static errr rd_dungeon_old()
{
    int i;
    byte count;
    byte ychar, xchar;
    byte tmp8u;
    u16b tmp16u;
    s16b mon_tot_mult;
    int total_count;
    cave_type *c_ptr;


    /* Header info */            
    rd_s16b(&dun_level);
    rd_s16b(&char_row);
    rd_s16b(&char_col);
    rd_s16b(&mon_tot_mult);
    rd_s16b(&cur_height);
    rd_s16b(&cur_width);
    rd_s16b(&max_panel_rows);
    rd_s16b(&max_panel_cols);


    /* Set the dungeon to indicate the player location */
    cave[char_row][char_col].m_idx = 1;


    /* read in the creature ptr info */
    while (1) {

	rd_byte(&tmp8u);
	if (tmp8u == 0xFF) break;

	ychar = tmp8u;
	rd_byte(&xchar);

	/* Invalid cave location */
	if (xchar >= MAX_WIDTH || ychar >= MAX_HEIGHT) return (71);

	/* let's correctly fix the invisible monster bug  -CWS */
	if (older_than(2,6,0)) {
	    rd_byte(&tmp8u);
	    cave[ychar][xchar].m_idx = tmp8u;
	}
	else {
	    rd_u16b(&tmp16u);
	    cave[ychar][xchar].m_idx = tmp16u;
	}
    }

    /* read in the treasure ptr info */
    while (1) { 
	rd_byte(&tmp8u);
	if (tmp8u == 0xFF) break;
	ychar = tmp8u;
	rd_byte(&xchar);
	rd_u16b(&tmp16u);
	if (xchar >= MAX_WIDTH || ychar >= MAX_HEIGHT) return (72);
	cave[ychar][xchar].i_idx = tmp16u;
    }


    /* Read in the actual "cave" data */
    total_count = 0;
    xchar = ychar = 0;

    /* Read until done */
    while (total_count < MAX_HEIGHT * MAX_WIDTH) {

	/* Extract some RLE info */
	rd_byte(&count);
	rd_byte(&tmp8u);

	/* Apply the RLE info */
	for (i = count; i > 0; i--) {

	    /* Prevent over-run */
	    if (ychar >= MAX_HEIGHT) {
		prt_note(-2, "Dungeon too big!");
		return (81);
	    }

	    /* Access the cave */
	    c_ptr = &cave[ychar][xchar];

	    /* Extract the "wall data" */
	    c_ptr->fval = tmp8u & 0xF;

	    /* Extract the "info" */
	    c_ptr->info = 0;
	    if ((tmp8u >> 4) & 0x1) c_ptr->info |= CAVE_LR;
	    if ((tmp8u >> 5) & 0x1) c_ptr->info |= CAVE_FM;
	    if ((tmp8u >> 6) & 0x1) c_ptr->info |= CAVE_PL;

	    /* Handle old style "light" */
	    if (c_ptr->info & CAVE_PL) {
		c_ptr->info |= CAVE_FM;
		if (c_ptr->info & CAVE_LR) c_ptr->info |= CAVE_INIT;
	    }

	    /* Hack -- handle old style "room codes" */
	    if (c_ptr->fval == ROOM_FLOOR + 1) {
		c_ptr->fval = ROOM_FLOOR;
		c_ptr->info |= CAVE_PL;
	    }
	    	    
	    /* Hack -- handle old style "room codes" */
	    else if (c_ptr->fval == VAULT_FLOOR + 1) {
		c_ptr->fval = VAULT_FLOOR;
		c_ptr->info |= CAVE_PL;
	    }
	    	    

	    /* Mega-Hack -- light all walls */
	    else if (c_ptr->fval >= MIN_WALL) {
		c_ptr->info |= CAVE_PL;
	    }

	    /* Advance the cave pointers */
	    xchar++;

	    /* Wrap to the next line */
	    if (xchar >= MAX_WIDTH) {
		xchar = 0;
		ychar++;
	    }
	}

	total_count += count;
    }


    /* Read the item count */
    rd_s16b(&i_max);
    if (i_max > MAX_I_IDX) {
	prt_note(-2, "Too many objects");
	return (92);
    }

    /* Read the dungeon items */
    for (i = MIN_I_IDX; i < i_max; i++) {
	inven_type *i_ptr = &i_list[i];

	/* Read the item */
	rd_item_old(i_ptr);

	/* XXX Hack -- No trapdoors */
	if (((i_ptr->tval == TV_INVIS_TRAP) ||
	     (i_ptr->tval == TV_VIS_TRAP)) &&
	    (i_ptr->sval == SV_TRAP_TRAP_DOOR)) {

	    /* Make it a spiked pit */
	    i_ptr->sval = SV_TRAP_SPIKED_PIT;
	}
    }


    /* Read the monster count */        
    rd_s16b(&m_max);
    if (m_max > MAX_M_IDX) {
	prt_note(-2, "Too many monsters");
	return (93);
    }

    /* Read the monsters */
    for (i = MIN_M_IDX; i < m_max; i++) {
	monster_type *m_ptr = &m_list[i];
	rd_monster_old(m_ptr);
	cave[m_ptr->fy][m_ptr->fx].m_idx = i;
    }


    /* Read the ghost info */
    rd_ghost_old();


    /* Success */
    return (0);
}







/*
 * Read an old savefile
 */

static void rd_options_old()
{
    u16b tmp16u;
    u32b l;

    /* "Log Index" (unused) */
    rd_u16b(&tmp16u);

    /* Standard options */
    rd_u32b(&l);

    /* Hack -- Extract death */
    death = (l & 0x80000000) ? TRUE : FALSE;

    /* Hack -- Extract keyset */
    rogue_like_commands = (l & 0x0020) ? TRUE : FALSE;

    /* Hack -- Unused options */
    if (!older_than(2,6,0)) {
	rd_u32b(&l);
	rd_u32b(&l);
	rd_u32b(&l);
    }
}


/*
 * Old inventory slot values
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
 * Hack -- help re-order the inventory
 */
static int convert_slot(int old)
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
    }

    /* Oops */
    return (INVEN_WIELD-1);
}

static errr rd_inventory_old()
{
    int i, n;
    s16b ictr;
    inven_type forge;

    /* Reset the counters */
    inven_ctr = 0;
    equip_ctr = 0;
    inven_weight = 0;

    /* Count the items */
    rd_s16b(&ictr);

    /* Verify */
    if (inven_ctr > INVEN_WIELD) {
	prt_note(-2, "Unable to read inventory");
	return (15);
    }

    /* Normal pack items */
    for (i = 0; i < ictr; i++) {

	/* Read the item */
	rd_item_old(&forge);

	/* Just carry it */
	(void)inven_carry(&forge);
    }

    /* Old "normal" equipment */
    for (i = OLD_INVEN_WIELD; i < OLD_INVEN_AUX; i++) {
	
	/* Hack -- convert old slot numbers */
	n = convert_slot(i);

	/* Read the item */
	rd_item_old(&forge);

	/* Skip "empty" slots */
	if (!forge.tval) continue;
	
	/* One more item */
	equip_ctr++;

	/* Hack -- structure copy */
	inventory[n] = forge;

	/* Hack -- Add the weight */
	inven_weight += (forge.number * forge.weight);
    }

    /* Old "aux" item */
    for (i = OLD_INVEN_AUX; i <= OLD_INVEN_AUX; i++) {

	/* Read the item */
	rd_item_old(&forge);

	/* Carry it if necessary */
	if (forge.tval) (void)inven_carry(&forge);
    }
    
    /* Forget old weight */
    rd_s16b(&ictr);

    /* Forget old equip_ctr */
    rd_s16b(&ictr);

    return (0);
}


static errr rd_savefile_old()
{
    int i;
    byte tmp8u;
    u16b tmp16u;
    u32b tmp32u;


    /* XXX Fake the system info */


    /* Read the artifacts */
    rd_artifacts_old();
    if (say) prt_note(-1,"Loaded Artifacts");


    /* Load the Quests */
    rd_u32b(&tmp32u);
    q_list[0].level = tmp32u ? 99 : 0;
    rd_u32b(&tmp32u);
    q_list[1].level = 100;
    rd_u32b(&tmp32u);
    q_list[2].level = 0;
    rd_u32b(&tmp32u);
    q_list[3].level = 0;
    if (say) prt_note(-1,"Loaded Quests");


    /* Load the old "Uniques" flags */
    for (i = 0; i < MAX_R_IDX; i++) {

	/* Ignore the "exist" info, extracted later */
	rd_u32b(&tmp32u);

	/* Already true, but do it again anyway */
	l_list[i].cur_num = 0;

	/* XXX Hack -- That unique has been killed */
	rd_u32b(&tmp32u);
	l_list[i].max_num = 100;
	if (r_list[i].cflags2 & MF2_UNIQUE) l_list[i].max_num = 1;
	if (tmp32u) l_list[i].max_num = 0;
    }
    if (say) prt_note(-1,"Loaded Unique Beasts");


    /* Load the recall memory */
    while (1) {

	/* Read some info, check for sentinal */
	rd_u16b(&tmp16u);
	if (tmp16u == 0xFFFF) break;

	/* Incompatible save files */
	if (tmp16u >= MAX_R_IDX) {
	    prt_note(-2,"Too many monsters!");
	    return (21);
	}

	/* Extract the monster lore */
	rd_lore_old(&l_list[tmp16u]);
    }
    if (say) prt_note(-1,"Loaded Monster Memory");


    /* Read the old options */
    rd_options_old();
    if (say) prt_note(-1, "Loaded options");

    /* Read the extra stuff */
    rd_extra_old();
    if (say) prt_note(-1, "Loaded extra information");


    /* Read the inventory */
    rd_inventory_old();


    /* Read spell info */
    rd_u32b(&spell_learned);
    rd_u32b(&spell_worked);
    rd_u32b(&spell_forgotten);
    rd_u32b(&spell_learned2);
    rd_u32b(&spell_worked2);
    rd_u32b(&spell_forgotten2);

    for (i = 0; i < 64; i++) {
	rd_byte(&spell_order[i]);
    }

    if (say) prt_note(-1, "Read spell information");


    /* Hack -- analyze the "object_ident" array. */
    for (i = 0; i < 1024; i++) {            

	int k, tval, sval, tried, aware;

	rd_byte(&tmp8u);
	if (!tmp8u) continue;

	/* Extract the flags */
	tried = (tmp8u & 0x01) ? 1 : 0;
	aware = (tmp8u & 0x02) ? 1 : 0;

	/* Extract the identity */
	switch (i >> 6) {
	    case 0: tval = TV_AMULET; break;
	    case 1: tval = TV_RING; break;
	    case 2: tval = TV_STAFF; break;
	    case 3: tval = TV_WAND; break;
	    case 4: tval = TV_SCROLL; break;
	    case 5: tval = TV_POTION; break;
	    case 6: tval = TV_FOOD; break;
	    case 7: tval = TV_ROD; break;
	    default: tval = TV_NOTHING;
	}

	/* No type? */
	if (tval == TV_NOTHING) continue;

	/* Extract the sub-type */
	sval = i % 64; 

	/* Find the object this refers to */
	for (k = 0; k < MAX_K_IDX; k++) {

	    inven_kind *k_ptr = &k_list[k];

	    /* Set the object info */
	    if ((tval == k_ptr->tval) &&
		(sval == k_ptr->sval % 64)) {
		x_list[k].tried = tried;
		x_list[k].aware = aware;
	    }
	}
    }
    if (say) prt_note(-1, "Parsed old 'known1' flags");

    /* Old seeds */
    rd_u32b(&randes_seed);
    rd_u32b(&town_seed);

    /* Old messages */
    rd_messages_old();

    /* Some leftover info */
    rd_s16b(&panic_save);
    rd_s16b(&total_winner);
    rd_s16b(&noscore);

    /* Read the player_hp array */
    for (i = 0; i < 50; i++) rd_u16b(&player_hp[i]);

    /* Hack -- Version 2.6.2 did silly things */
    if (!older_than(2,6,2)) {
	u16b icky;
	for (i = 50; i < 99; i++) rd_u16b(&icky);
    }

    if (say) prt_note(-1, "Read some more information.");


    /* Read the stores */
    for (i = 0; i < MAX_STORES; i++) {
	if (rd_store_old(&store[i])) {
	    prt_note(-2,"ERROR reading store");
	    return (32);
	}
    }


    /* Time at which file was saved */
    rd_u32b(&sf_when);

    /* Read the cause of death, if any */
    rd_string(died_from);

    if (say) prt_note(-1, "All player info restored");


    /* I'm not dead yet... */
    if (!death) {

	/* Dead players have no dungeon */
	prt_note(-1,"Restoring Dungeon...");
	if (rd_dungeon_old()) {
	    prt_note(-2, "Error reading dungeon data");
	    return (25);
	}

	/* Really old version -- read stores again */
	if (older_than(2,1,3)) {

	    /* Read the stores (again) */
	    for (i = 0; i < MAX_STORES; i++) {
		if (rd_store_old(&store[i])) {
		    prt_note(-2,"ERROR in STORE_INVEN_MAX");
		    return (33);
		}
	    }
	}


	/* Time goes here, too */
	rd_u32b(&sf_when);
    }



    /* Assume success */
    return (0);
}


/*
 * Hack -- forward declare
 */
extern errr rd_old_sf(FILE *fff1, int vmaj, int vmin, int vpat, int say1);

/*
 * Read an old savefile
 */
errr rd_old_sf(FILE *fff1, int vmaj, int vmin, int vpat, int say1)
{
    fff = fff1;
    
    version_maj = vmaj;
    version_min = vmin;
    patch_level = vpat;

    say = say1;
        
    /* Read the old savefile */
    return (rd_savefile_old());
}




