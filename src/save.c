/* File: save.c */

/* Purpose: save and restore games and monster memory info */

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
 * And then was re-written again (for 2.7.0) cause it sucked.  -BEN-
 */


#ifndef USG
# include <sys/file.h>
# include <sys/param.h>
#endif


#include <time.h>

#ifdef __MINT__
# include <stat.h>		/* chmod() */
#endif

#ifndef SET_UID
# ifdef __EMX__
#  include <sys/stat.h>
# else
#  include <stat.h>
# endif
#endif

#ifdef linux
# include <sys/types.h>
# include <sys/stat.h>
#endif


/*
 * Verify the Checksums written by Angband 2.7.0
 */
#define VERIFY_CHECKSUMS


/*
 * these are used for the save file, to avoid having to pass them to every
 * procedure 
 */

static FILE	*fff;		/* Current save "file" */

static int8u	xor_byte;	/* Simple encryption */

static int8u	version_maj;	/* Major version */
static int8u	version_min;	/* Minor version */
static int8u	patch_level;	/* Patch level */

static int32u	v_check = 0L;	/* A simple "checksum" on the actual values */
static int32u	x_check = 0L;	/* A simple "checksum" on the encoded bytes */

static int	from_savefile;	/* can overwrite old savefile when save */

static bool	say = FALSE;	/* Show "extra" messages */


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
 * The basic I/O functions for savefiles
 * All information is written/read one byte at a time
 */

static void sf_put(byte v)
{
    /* Encode the value, write a character */
    xor_byte ^= v;
    (void)putc((int)xor_byte, fff);

    /* Maintain the checksum info */
    v_check += v;
    x_check += xor_byte;
}

static byte sf_get(void)
{
    register int8u c, v;

    /* Get a character, decode the value */
    c = getc(fff) & 0xFF;
    v = c ^ xor_byte;
    xor_byte = c;

    /* Maintain the checksum info */
    v_check += v;
    x_check += xor_byte;

#ifdef SAVEFILE_VOMIT
    /* Hack -- debugging */
    if (1) {
	static int y = 15, x = 0;
	char buf[3];
	sprintf(buf, "%02x", v);
	prt(buf, y, x*3);
	x++;
	if (x >= 25) {
	    x = 0;
	    y++;
	    if (y >= 24) y = 15;
	}
    }
#endif

    /* Return the value */    
    return (v);
}




/*
 * Write/Read various "byte sized" objects
 */

static void wr_byte(byte v)
{
    sf_put(v);
}

static void rd_byte(byte *ip)
{
    *ip = sf_get();
}


static void wr_char(char v)
{
    wr_byte((byte)v);
}

static void rd_char(char *ip)
{
    rd_byte((byte*)ip);
}


/*
 * Write/Read various "short" objects
 */

static void wr_int16u(int16u v)
{
    sf_put(v & 0xFF);
    sf_put((v >> 8) & 0xFF);
}

static void rd_int16u(int16u *ip)
{
    (*ip) = sf_get();
    (*ip) |= ((int16u)(sf_get()) << 8);
}


static void wr_int16s(int16 v)
{
    wr_int16u((int16u)v);
}

static void rd_int16s(int16 *ip)
{
    rd_int16u((int16u*)ip);
}



/*
 * Write/Read various "long" objects
 */

static void wr_int32u(int32u v)
{
    sf_put(v & 0xFF);
    sf_put((v >> 8) & 0xFF);
    sf_put((v >> 16) & 0xFF);
    sf_put((v >> 24) & 0xFF);
}

static void rd_int32u(int32u *ip)
{
    (*ip) = sf_get();
    (*ip) |= ((int32u)(sf_get()) << 8);
    (*ip) |= ((int32u)(sf_get()) << 16);
    (*ip) |= ((int32u)(sf_get()) << 24);
}


static void wr_int32s(int32 v)
{
    wr_int32u((int32u)v);
}

static void rd_int32s(int32 *ip)
{
    rd_int32u((int32u*)ip);
}




/*
 * Strings
 */

static void wr_string(cptr str)
{
    while (*str) {
	wr_byte(*str);
	str++;
    }
    wr_byte(*str);
}

static void rd_string(char *str)
{
    while (1) {
	int8u tmp;
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
    int i;
    int32u f1 = i_ptr->flags1;
    int32u f2 = i_ptr->flags2;


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
    int8u tmp8u;
    int16u tmp16u;
    char note[128];
    
    bool invis_trap = FALSE;

    inven_kind *k_ptr;

    rd_int16u(&i_ptr->k_idx);
    rd_byte(&i_ptr->name2);
    rd_string(note);
    inscribe(i_ptr, note);
    rd_int32u(&i_ptr->flags1);
    rd_byte(&i_ptr->tval);

    /* Was: i_ptr->tchar */
    rd_byte(&tmp8u);

    rd_int16s(&i_ptr->pval);
    rd_int32s(&i_ptr->cost);
    rd_byte(&i_ptr->sval);
    rd_byte(&i_ptr->number);

    rd_int16u(&tmp16u);
    i_ptr->weight = tmp16u;

    rd_int16s(&i_ptr->tohit);
    rd_int16s(&i_ptr->todam);
    rd_int16s(&i_ptr->ac);
    rd_int16s(&i_ptr->toac);
    rd_byte(&i_ptr->damage[0]);
    rd_byte(&i_ptr->damage[1]);

    /* Forget old "level" */
    rd_byte(&tmp8u);

    rd_byte(&i_ptr->ident);
    rd_int32u(&i_ptr->flags2);

    /* Read and forget the old timeout */    
    rd_int16u(&tmp16u);

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

    /* Artifact Names Dominate Special Names */
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
	i_ptr->pval = k_list[i_ptr->k_idx].pval;

	/* XXX XXX Normal bows of extra might */
	/* i_ptr->flags3 |= (TR3_XTRA_MIGHT); */
    }



    /* Repair the "flags" in wearable objects */
    if (wearable_p(i_ptr)) {


	/* Save the "Sustain Stat flags" */
	int32u sustain2 = 0L;


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


    /*** Set some new flags ***/

    /* Hack -- Bard does (x4) damage */
    if (i_ptr->name1 == ART_BARD) {
       i_ptr->flags3 |= (TR3_XTRA_MIGHT);
    }

    /* Hack -- Belthronding does (x4) damage (was (x5)) */
    /* XXX Could replace "Might" with "Shots" (or add both) */
    if (i_ptr->name1 == ART_BELTHRONDING) {
       i_ptr->flags3 |= (TR3_XTRA_MIGHT);
       /* i_ptr->flags3 |= (TR3_XTRA_SHOTS); */
    }

    /* Hack -- Cubragol does (x4) damage */
    if (i_ptr->name1 == ART_CUBRAGOL) {
       i_ptr->flags3 |= (TR3_XTRA_MIGHT);
       i_ptr->pval = 1;		/* Repair "+1 speed" */
    }

    /* Hack -- Some Cursed Items are hard to uncurse */
    if ((i_ptr->name1 == ART_MORMEGIL) ||
	(i_ptr->name1 == ART_CALRIS) ||
	(i_ptr->name2 == EGO_MORGUL)) {
	i_ptr->flags3 |= TR3_HEAVY_CURSE;
    }

    /* Hack -- The Crown of Morgoth cannot be removed */
    if (i_ptr->name1 == ART_MORGOTH) {
	i_ptr->flags3 |= (TR3_HEAVY_CURSE | TR3_PERMA_CURSE);
    }

    /* Hack -- The One Ring cannot be removed, and it drains experience */
    if (i_ptr->name1 == ART_POWER) {
	i_ptr->flags3 |= (TR3_HEAVY_CURSE | TR3_PERMA_CURSE | TR3_DRAIN_EXP);
    }

    /* Hack -- artifacts cannot be destroyed */
    if (i_ptr->name1) {
	i_ptr->flags3 |= (TR3_IGNORE_FIRE | TR3_IGNORE_COLD |
			  TR3_IGNORE_ELEC | TR3_IGNORE_ACID);
    }    

    /* Extract new "resist" settings from "RESIST" */
    if (i_ptr->flags2 & TR2_RES_ACID) i_ptr->flags3 |= TR3_IGNORE_ACID;
    if (i_ptr->flags2 & TR2_RES_ELEC) i_ptr->flags3 |= TR3_IGNORE_ELEC;
    if (i_ptr->flags2 & TR2_RES_FIRE) i_ptr->flags3 |= TR3_IGNORE_FIRE;
    if (i_ptr->flags2 & TR2_RES_COLD) i_ptr->flags3 |= TR3_IGNORE_COLD;

    /* Extract new "resist" settings from "IMMUNE" */
    if (i_ptr->flags2 & TR2_IM_ACID) i_ptr->flags3 |= TR3_IGNORE_ACID;
    if (i_ptr->flags2 & TR2_IM_ELEC) i_ptr->flags3 |= TR3_IGNORE_ELEC;
    if (i_ptr->flags2 & TR2_IM_FIRE) i_ptr->flags3 |= TR3_IGNORE_FIRE;
    if (i_ptr->flags2 & TR2_IM_COLD) i_ptr->flags3 |= TR3_IGNORE_COLD;

    /* XXX Old Dragon Scale and Mithril Items will retain "Resist Acid" */


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

    /* Success */
    return (0);
}


/*
 * Read an item (2.7.0)
 */
static void rd_item(inven_type *i_ptr)
{
    int8u tmp8u;
    int32u tmp32u;
    char note[128];
    
    /* Paranoia */
    if (older_than(2,7,0)) abort();

    /* Get the kind */
    rd_int16u(&i_ptr->k_idx);

    rd_byte(&i_ptr->iy);
    rd_byte(&i_ptr->ix);

    rd_byte(&i_ptr->tval);
    rd_byte(&i_ptr->sval);
    rd_int16s(&i_ptr->pval);

    rd_byte(&i_ptr->name1);
    rd_byte(&i_ptr->name2);
    rd_byte(&i_ptr->ident);
    rd_byte(&i_ptr->number);
    rd_int16s(&i_ptr->weight);
    rd_int16s(&i_ptr->timeout);

    rd_int16s(&i_ptr->tohit);
    rd_int16s(&i_ptr->todam);
    rd_int16s(&i_ptr->toac);
    rd_int16s(&i_ptr->ac);
    rd_byte(&i_ptr->damage[0]);
    rd_byte(&i_ptr->damage[1]);
    rd_byte(&tmp8u);
    rd_byte(&tmp8u);

    rd_int32s(&i_ptr->cost);
    rd_int32s(&i_ptr->scost);

    rd_int32u(&i_ptr->flags1);
    rd_int32u(&i_ptr->flags2);
    rd_int32u(&i_ptr->flags3);
    rd_int32u(&tmp32u);

    rd_string(note);
    inscribe(i_ptr, note);
}

static void wr_item(inven_type *i_ptr)
{
    wr_int16u(i_ptr->k_idx);

    wr_byte(i_ptr->iy);
    wr_byte(i_ptr->ix);

    wr_byte(i_ptr->tval);
    wr_byte(i_ptr->sval);
    wr_int16s(i_ptr->pval);

    wr_byte(i_ptr->name1);
    wr_byte(i_ptr->name2);
    wr_byte(i_ptr->ident);
    wr_byte(i_ptr->number);
    wr_int16s(i_ptr->weight);
    wr_int16s(i_ptr->timeout);

    wr_int16s(i_ptr->tohit);
    wr_int16s(i_ptr->todam);
    wr_int16s(i_ptr->toac);
    wr_int16s(i_ptr->ac);
    wr_byte(i_ptr->damage[0]);
    wr_byte(i_ptr->damage[1]);
    wr_byte(0);
    wr_byte(0);

    wr_int32s(i_ptr->cost);
    wr_int32s(i_ptr->scost);

    wr_int32u(i_ptr->flags1);
    wr_int32u(i_ptr->flags2);
    wr_int32u(i_ptr->flags3);
    wr_int32u(0L);

    wr_string(i_ptr->inscrip);
}



/*
 * Read and Write monsters
 */

static void rd_monster_old(monster_type *m_ptr)
{
    int8u	tmp8u;

    /* Read the current hitpoints */
    rd_int16s(&m_ptr->hp);

    if (older_than(2,6,0)) {
	/* Hack -- see below as well */
	m_ptr->maxhp = m_ptr->hp;
    }
    else {
	/* Read the maximal hitpoints */
	rd_int16s(&m_ptr->maxhp);
    }

    rd_int16s(&m_ptr->csleep);
    rd_int16s(&m_ptr->cspeed);

    rd_int16u(&m_ptr->r_idx);

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
}

static void rd_monster(monster_type *m_ptr)
{
    /* Parse older versions */
    if (older_than(2,7,0)) {
	rd_monster_old(m_ptr);
	return;
    }

    rd_int16u(&m_ptr->r_idx);
    rd_byte(&m_ptr->fy);
    rd_byte(&m_ptr->fx);
    rd_int16s(&m_ptr->hp);
    rd_int16s(&m_ptr->maxhp);
    rd_int16s(&m_ptr->csleep);
    rd_int16s(&m_ptr->cspeed);
    rd_byte(&m_ptr->stunned);
    rd_byte(&m_ptr->confused);
    rd_byte(&m_ptr->monfear);
    rd_byte(&m_ptr->unused);
}

static void wr_monster(monster_type *m_ptr)
{
    wr_int16u(m_ptr->r_idx);
    wr_byte(m_ptr->fy);
    wr_byte(m_ptr->fx);
    wr_int16u(m_ptr->hp);
    wr_int16u(m_ptr->maxhp);
    wr_int16u(m_ptr->csleep);
    wr_int16u(m_ptr->cspeed);    
    wr_byte(m_ptr->stunned);
    wr_byte(m_ptr->confused);
    wr_byte(m_ptr->monfear);
    wr_byte(m_ptr->unused);
}





/*
 * Write/Read the monster lore
 */

static void rd_lore(monster_lore *l_ptr)
{
    int8u tmp8u;
    int32u tmp32u;

    if (older_than(2,7,0)) {            

	rd_int32u(&l_ptr->r_cflags1);
	rd_int32u(&l_ptr->r_spells1);
	rd_int32u(&l_ptr->r_spells2);
	rd_int32u(&l_ptr->r_spells3);
	rd_int16u(&l_ptr->r_kills);
	rd_int16u(&l_ptr->r_deaths);
	rd_int32u(&l_ptr->r_cflags2);
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

    else if (older_than(2,7,1)) {

	rd_int16u(&l_ptr->r_kills);
	rd_int16u(&l_ptr->r_deaths);

	rd_int32u(&l_ptr->r_spells1);
	rd_int32u(&l_ptr->r_spells2);
	rd_int32u(&l_ptr->r_spells3);
	rd_int32u(&l_ptr->r_cflags1);
	rd_int32u(&l_ptr->r_cflags2);
	rd_int32u(&tmp32u);

	rd_byte(&l_ptr->r_attacks[0]);
	rd_byte(&l_ptr->r_attacks[1]);
	rd_byte(&l_ptr->r_attacks[2]);
	rd_byte(&l_ptr->r_attacks[3]);

	rd_byte(&l_ptr->r_wake);
	rd_byte(&l_ptr->r_ignore);

	rd_byte(&tmp8u);		/* Old "cur_num" */
	rd_byte(&l_ptr->max_num);

	/* Hack -- Extract (and clear) the "treasure count" bits */
	l_ptr->r_drop = (l_ptr->r_cflags1 & CM1_TREASURE) >> CM1_TR_SHIFT;
	l_ptr->r_cflags1 &= ~CM1_TREASURE;

	/* Hack -- Extract (and clear) the "spell count" bits */        
	l_ptr->r_cast = (l_ptr->r_spells1 & CS1_FREQ);
	l_ptr->r_spells1 &= ~CS1_FREQ;
    }

    /* Final method */
    else {

	/* Observed flags */
	rd_int32u(&l_ptr->r_spells1);
	rd_int32u(&l_ptr->r_spells2);
	rd_int32u(&l_ptr->r_spells3);
	rd_int32u(&l_ptr->r_cflags1);
	rd_int32u(&l_ptr->r_cflags2);
	rd_int32u(&tmp32u);

	/* Count observations of attacks */
	rd_byte(&l_ptr->r_attacks[0]);
	rd_byte(&l_ptr->r_attacks[1]);
	rd_byte(&l_ptr->r_attacks[2]);
	rd_byte(&l_ptr->r_attacks[3]);

	/* Count some other stuff */
	rd_byte(&l_ptr->r_wake);
	rd_byte(&l_ptr->r_ignore);

	/* Count observed treasure drops */
	rd_byte(&l_ptr->r_drop);

	/* Count observed spell castings */
	rd_byte(&l_ptr->r_cast);

	/* Count kills by player */
	rd_int16u(&l_ptr->r_kills);

	/* Count deaths of player */
	rd_int16u(&l_ptr->r_deaths);

	/* Read the "Racial" monster limit per level */
	rd_byte(&l_ptr->max_num);

	/* Later */
	rd_byte(&tmp8u);
    }
}

static void wr_lore(monster_lore *l_ptr)
{
    /* Write the info */
    wr_int32u(l_ptr->r_spells1);
    wr_int32u(l_ptr->r_spells2);
    wr_int32u(l_ptr->r_spells3);
    wr_int32u(l_ptr->r_cflags1);
    wr_int32u(l_ptr->r_cflags2);
    wr_int32u(0L);

    /* Count attacks and other stuff */
    wr_byte(l_ptr->r_attacks[0]);
    wr_byte(l_ptr->r_attacks[1]);
    wr_byte(l_ptr->r_attacks[2]);
    wr_byte(l_ptr->r_attacks[3]);
    wr_byte(l_ptr->r_wake);
    wr_byte(l_ptr->r_ignore);
    wr_byte(l_ptr->r_drop);
    wr_byte(l_ptr->r_cast);
    
    /* Count kills/deaths */
    wr_int16u(l_ptr->r_kills);
    wr_int16u(l_ptr->r_deaths);

    /* Monster limit per level */
    wr_byte(l_ptr->max_num);

    /* Later */
    wr_byte(0);
}



/*
 * Read/Write the "xtra" info for objects
 */

static void rd_xtra(inven_xtra *xtra)
{
    int8u tmp8u;

    rd_byte(&tmp8u);

    xtra->aware = (tmp8u & 0x01) ? TRUE: FALSE;
    xtra->tried = (tmp8u & 0x02) ? TRUE: FALSE;
}

static void wr_xtra(inven_xtra *xtra)
{
    int8u tmp8u = 0;

    if (xtra->aware) tmp8u |= 0x01;
    if (xtra->tried) tmp8u |= 0x02;

    wr_byte(tmp8u);
}



/*
 * Write/Read a store
 */
static void wr_store(store_type *st_ptr)
{
    int j;

    wr_int32u(st_ptr->store_open);
    wr_int16u(st_ptr->insult_cur);
    wr_byte(st_ptr->owner);
    wr_byte(st_ptr->store_ctr);
    wr_int16u(st_ptr->good_buy);
    wr_int16u(st_ptr->bad_buy);

    /* Write the items */
    for (j = 0; j < st_ptr->store_ctr; j++) {
	wr_item(&st_ptr->store_item[j]);
    }
}

static errr rd_store(store_type *st_ptr)
{
    int j;

    rd_int32s(&st_ptr->store_open);
    rd_int16s(&st_ptr->insult_cur);
    rd_byte(&st_ptr->owner);
    rd_byte(&st_ptr->store_ctr);
    rd_int16u(&st_ptr->good_buy);
    rd_int16u(&st_ptr->bad_buy);

    /* Too many items */    
    if (st_ptr->store_ctr > STORE_INVEN_MAX) {
	prt_note(-2, "Too many items in store");
	return (10);
    }

    /* Read the items (and costs) */
    for (j = 0; j < st_ptr->store_ctr; j++) {
	if (older_than(2,7,0)) {
	    int32 scost;
	    rd_int32s(&scost);
	    rd_item_old(&st_ptr->store_item[j]);
	    st_ptr->store_item[j].scost = scost;
	}
	else {
	    rd_item(&st_ptr->store_item[j]);
	}
    }

    /* Success */
    return (0);
}



/*
 * Read the artifacts -- old version
 */

static void rd_artifacts_old()
{
    int32u tmp32u;

    v_list[ART_MORGOTH].cur_num = 0;

    rd_int32u(&tmp32u); v_list[ART_GROND].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_RINGIL].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_AEGLOS].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_ARUNRUTH].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_MORMEGIL].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_ANGRIST].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_GURTHANG].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_CALRIS].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_ANDURIL].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_STING].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_ORCRIST].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_GLAMDRING].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_DURIN].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_AULE].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_THUNDERFIST].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_BLOODSPIKE].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_DOOMCALLER].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_NARTHANC].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_NIMTHANC].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_DETHANC].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_GILETTAR].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_RILIA].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_BELANGIL].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_BALLI].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_LOTHARANG].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_FIRESTAR].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_ERIRIL].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_CUBRAGOL].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_BARD].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_COLLUIN].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_HOLCOLLETH].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_TOTILA].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_PAIN].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_ELVAGIL].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_AGLARANG].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_EORLINGAS].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_BARUKKHELED].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_WRATH].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_HARADEKKET].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_MUNDWINE].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_GONDRICAM].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_ZARCUTHRA].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_CARETH].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_FORASGIL].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_CRISDURIAN].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_COLANNON].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_HITHLOMIR].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_THALKETTOTH].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_ARVEDUI].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_THRANDUIL].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_THENGEL].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_HAMMERHAND].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_CELEGORM].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_THROR].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_MAEDHROS].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_OLORIN].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_ANGUIREL].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_OROME].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_EONWE].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_THEODEN].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_ULMO].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_OSONDIR].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_TURMIL].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_CASPANION].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_TIL].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_DEATHWREAKER].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_AVAVIR].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_TARATOL].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_DOR].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_NENYA].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_NARYA].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_VILYA].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_BELEGENNON].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_FEANOR].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_ISILDUR].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_SOULKEEPER].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_FINGOLFIN].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_ANARION].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_POWER].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_GALADRIEL].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_BELTHRONDING].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_DAL].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_PAURHACH].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_PAURNIMMEN].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_PAURAEGEN].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_PAURNEN].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_CAMMITHRIM].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_CAMBELEG].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_INGWE].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_CARLAMMAS].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_HOLHENNETH].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_AEGLIN].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_CAMLOST].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_NIMLOTH].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_NAR].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_BERUTHIEL].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_GORLIM].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_ELENDIL].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_THORIN].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_CELEBORN].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_THRAIN].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_GONDOR].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_THINGOL].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_THORONGIL].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_LUTHIEN].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_TUOR].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_ROHIRRIM].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_TULKAS].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_DWARVES].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_BARAHIR].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_RAZORBACK].cur_num = tmp32u;
    rd_int32u(&tmp32u); v_list[ART_BLADETURNER].cur_num = tmp32u;
}




/*
 * Read/Write boolean options
 */

static void rd_options()
{
    int8u tmp8u;
    int32u l;


    /*** Read some options ***/

    rd_int32u(&l);

    rogue_like_commands =  (l & 0x00000001L) ? TRUE : FALSE;
    prompt_carry_flag =    (l & 0x00000002L) ? TRUE : FALSE;
    carry_query_flag =     (l & 0x00000004L) ? TRUE : FALSE;
    always_throw =         (l & 0x00000008L) ? TRUE : FALSE;
    always_repeat =        (l & 0x00000010L) ? TRUE : FALSE;
    quick_messages =       (l & 0x00000020L) ? TRUE : FALSE;

    use_color =            (l & 0x00000100L) ? TRUE : FALSE;
    notice_seams =         (l & 0x00000400L) ? TRUE : FALSE;
    ring_bell =            (l & 0x00000800L) ? TRUE : FALSE;
    equippy_chars =        (l & 0x00001000L) ? TRUE : FALSE;
    new_screen_layout =    (l & 0x00002000L) ? TRUE : FALSE;
    depth_in_feet =	   (l & 0x00004000L) ? TRUE : FALSE;
    hilite_player =	   (l & 0x00008000L) ? TRUE : FALSE;

    plain_descriptions =   (l & 0x00010000L) ? TRUE : FALSE;
    show_inven_weight =    (l & 0x00020000L) ? TRUE : FALSE;
    show_equip_weight =    (l & 0x00040000L) ? TRUE : FALSE;
    show_store_weight =    (l & 0x00080000L) ? TRUE : FALSE;

    use_recall_win =       (l & 0x00100000L) ? TRUE : FALSE;
    use_choice_win =       (l & 0x00200000L) ? TRUE : FALSE;

    rd_int32u(&l);

    no_haggle_flag =       (l & 0x00000001L) ? TRUE : FALSE; 
    shuffle_owners =       (l & 0x00000002L) ? TRUE : FALSE;

    find_cut =             (l & 0x00000100L) ? TRUE : FALSE;
    find_examine =         (l & 0x00000200L) ? TRUE : FALSE;
    find_prself =          (l & 0x00000400L) ? TRUE : FALSE;
    find_bound =           (l & 0x00000800L) ? TRUE : FALSE;
    find_ignore_doors =    (l & 0x00001000L) ? TRUE : FALSE;
    find_ignore_stairs =   (l & 0x00002000L) ? TRUE : FALSE;

    disturb_near =         (l & 0x00010000L) ? TRUE : FALSE;
    disturb_move =         (l & 0x00020000L) ? TRUE : FALSE;
    disturb_enter =        (l & 0x00040000L) ? TRUE : FALSE;
    disturb_leave =        (l & 0x00080000L) ? TRUE : FALSE;

    view_pre_compute =     (l & 0x00100000L) ? TRUE : FALSE;
    view_reduce_view =     (l & 0x00400000L) ? TRUE : FALSE;
    view_reduce_lite =     (l & 0x00800000L) ? TRUE : FALSE;

    view_yellow_lite =     (l & 0x01000000L) ? TRUE : FALSE;
    view_bright_lite =     (l & 0x02000000L) ? TRUE : FALSE;
    view_yellow_fast =     (l & 0x04000000L) ? TRUE : FALSE;
    view_bright_fast =     (l & 0x08000000L) ? TRUE : FALSE;

    view_perma_grids =     (l & 0x10000000L) ? TRUE : FALSE;
    view_torch_grids =     (l & 0x20000000L) ? TRUE : FALSE;


    /* Future options */
    rd_int32u(&l);
    rd_int32u(&l);


    /* Read "delay_spd" */
    rd_byte(&tmp8u);
    delay_spd = tmp8u;

    /* Read "hitpoint_warn" */
    rd_byte(&tmp8u);
    hitpoint_warn = tmp8u;

    /* Future options */
    rd_byte(&tmp8u);
    rd_byte(&tmp8u);


    /* Hack -- make sure nobody gets "confused" */
    if (older_than(2,7,1)) {

	/* Necessary */
        view_perma_grids = TRUE;
	view_reduce_lite = TRUE;
	disturb_near = TRUE;
	disturb_enter = TRUE;

	/* Yuck! */
	use_choice_win = FALSE;
	use_recall_win = FALSE;
    }
}

static void wr_options()
{
    int32u l;

    l = 0L;

    if (rogue_like_commands)	l |= 0x00000001L;
    if (prompt_carry_flag)	l |= 0x00000002L;
    if (carry_query_flag)	l |= 0x00000004L;
    if (always_throw)		l |= 0x00000008L;
    if (always_repeat)		l |= 0x00000010L;
    if (quick_messages)		l |= 0x00000020L;

    if (use_color)		l |= 0x00000100L;
    if (notice_seams)		l |= 0x00000400L;
    if (ring_bell)		l |= 0x00000800L;
    if (equippy_chars)		l |= 0x00001000L;
    if (new_screen_layout)	l |= 0x00002000L;
    if (depth_in_feet)		l |= 0x00004000L;
    if (hilite_player)		l |= 0x00008000L;

    if (plain_descriptions)	l |= 0x00010000L;
    if (show_inven_weight)	l |= 0x00020000L;
    if (show_equip_weight)	l |= 0x00040000L;
    if (show_store_weight)	l |= 0x00080000L;

    wr_int32u(l);


    l = 0L;

    if (no_haggle_flag)		l |= 0x00000001L; 
    if (shuffle_owners)		l |= 0x00000002L;

    if (find_cut)		l |= 0x00000100L;
    if (find_examine)		l |= 0x00000200L;
    if (find_prself)		l |= 0x00000400L;
    if (find_bound)		l |= 0x00000800L;
    if (find_ignore_doors)	l |= 0x00001000L;
    if (find_ignore_stairs)	l |= 0x00002000L;

    if (disturb_near)		l |= 0x00010000L;
    if (disturb_move)		l |= 0x00020000L;
    if (disturb_enter)		l |= 0x00040000L;
    if (disturb_leave)		l |= 0x00080000L;

    if (view_pre_compute)	l |= 0x00100000L;
    if (view_reduce_view)	l |= 0x00400000L;
    if (view_reduce_lite)	l |= 0x00800000L;

    if (view_yellow_lite)	l |= 0x01000000L;
    if (view_bright_lite)	l |= 0x02000000L;
    if (view_yellow_fast)	l |= 0x04000000L;
    if (view_bright_fast)	l |= 0x08000000L;

    if (view_perma_grids)	l |= 0x10000000L;
    if (view_torch_grids)	l |= 0x20000000L;

    if (TRUE)			l |= 0x40000000L;	/* Pre-set options */
    if (TRUE)			l |= 0x80000000L;	/* Pre-set options */
    
    wr_int32u(l);

    /* Future options */    
    wr_int32u(0L);
    wr_int32u(0L);



    /* Write "delay_spd" */
    wr_byte(delay_spd);

    /* Write "hitpoint_warn" */
    wr_byte(hitpoint_warn);

    /* Future options */
    wr_byte(0);
    wr_byte(0);
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
    int8u tmp8u;
    int16u tmp16u;
    int32u tmp32u;


    /* A buffer for the ghost name */
    char gname[128];

    /* Hack -- read the name as bytes */
    for (i = 0; i < 100; i++) rd_char(&gname[i]);
    strcpy(ghost_name, gname);

    /* Restore ghost names & stats etc... */
    /* XXX Argh, all of the neat spells are lost */
    /* XXX Stupid order anyway */

    rd_int32u(&tmp32u);
    r_ptr->cflags1 = tmp32u;

    rd_int32u(&tmp32u);
    r_ptr->spells1 = tmp32u;

    rd_int32u(&tmp32u);
    r_ptr->cflags2 = tmp32u;


/*
 * fix player ghost's exp bug.  The mexp field is really an int32u, but the
 * savefile was writing/ reading an int16u.  Since I don't want to change
 * the savefile format, this insures that the mexp field is loaded, and that
 * the "high bits" of mexp do not contain garbage values which could mean that
 * player ghost are worth millions of exp. -CFT
 */

    rd_int16u(&tmp16u);
    r_ptr->mexp = (int32u)(tmp16u);

/*
 * more stupid size bugs that would've never been needed if these variables
 * had been given enough space in the first place -CWS
 */

    if (older_than(2,6,0)) {
	rd_byte(&tmp8u);
	r_ptr->sleep = tmp8u;
    }
    else {
	rd_int16u(&tmp16u);
	r_ptr->sleep = tmp16u;
    }

    rd_byte(&tmp8u);
    r_ptr->aaf = tmp8u;

    if (older_than(2,6,0)) {
	rd_byte(&tmp8u);
	r_ptr->ac = tmp8u;
    }
    else {
	rd_int16u(&tmp16u);
	r_ptr->ac = tmp16u;
    }

    rd_byte(&tmp8u);
    r_ptr->speed = tmp8u;

    rd_byte(&tmp8u);
    r_ptr->r_char = tmp8u;

    rd_byte(&r_ptr->hd[0]);
    rd_byte(&r_ptr->hd[1]);

    /* Hack -- read the attacks */
    for (i = 0; i < 4; i++) {
	rd_int16u(&r_ptr->damage[i]);
    }

    rd_int16u(&tmp16u);
    r_ptr->level = tmp16u;

    /* Hack -- a white ghost (plus shimmer) */
    r_ptr->r_attr = COLOR_WHITE;    
    r_ptr->cflags1 |= MF1_ATTR_MULTI;
}

static void rd_ghost()
{
    monster_race *r_ptr = &r_list[MAX_R_IDX-1];

    int8u tmp8u;
    int32u tmp32u; 


    /* Hack -- old method barely works */
    if (older_than(2,7,0)) abort();


    rd_string(ghost_name);

    rd_byte(&r_ptr->level);
    rd_byte(&r_ptr->rarity);
    rd_byte(&tmp8u);
    rd_byte(&tmp8u);

    rd_byte(&tmp8u);
    rd_char(&r_ptr->r_char);
    rd_byte(&r_ptr->r_attr);
    rd_byte(&tmp8u);

    rd_byte(&r_ptr->hd[0]);
    rd_byte(&r_ptr->hd[1]);
    rd_int16u(&r_ptr->ac);
    rd_int16u(&r_ptr->sleep);
    rd_byte(&r_ptr->aaf);
    rd_byte(&r_ptr->speed);

    rd_int32u(&r_ptr->mexp);

    rd_int16u(&r_ptr->damage[0]);
    rd_int16u(&r_ptr->damage[1]);
    rd_int16u(&r_ptr->damage[2]);
    rd_int16u(&r_ptr->damage[3]);

    rd_int32u(&r_ptr->cflags1);
    rd_int32u(&r_ptr->cflags2);
    rd_int32u(&tmp32u);

    rd_int32u(&r_ptr->spells1);
    rd_int32u(&r_ptr->spells2);
    rd_int32u(&r_ptr->spells3);
}

static void wr_ghost()
{
    monster_race *r_ptr = &r_list[MAX_R_IDX-1];

    wr_string(ghost_name);

    wr_byte(r_ptr->level);
    wr_byte(r_ptr->rarity);
    wr_byte(0);
    wr_byte(0);

    wr_char(0);		/* was "gender" in 2.7.0 */
    wr_char(r_ptr->r_char);
    wr_byte(r_ptr->r_attr);
    wr_byte(0);		/* was "gchar" in 2.7.0 */

    wr_byte(r_ptr->hd[0]);
    wr_byte(r_ptr->hd[1]);
    wr_int16u(r_ptr->ac);
    wr_int16u(r_ptr->sleep);
    wr_byte(r_ptr->aaf);
    wr_byte(r_ptr->speed);

    wr_int32u(r_ptr->mexp);

    wr_int16u(r_ptr->damage[0]);
    wr_int16u(r_ptr->damage[1]);
    wr_int16u(r_ptr->damage[2]);
    wr_int16u(r_ptr->damage[3]);

    wr_int32u(r_ptr->cflags1);
    wr_int32u(r_ptr->cflags2);
    wr_int32u(0L);

    wr_int32u(r_ptr->spells1);
    wr_int32u(r_ptr->spells2);
    wr_int32u(r_ptr->spells3);
}



/*
 * Read the OLD extra information
 */
static void rd_extra_old()
{
    int i;
    int8u tmp8u;
    int16u tmp16u; 
    int32 tmp32s;

    rd_string(player_name);
    rd_byte(&p_ptr->male);
    rd_int32s(&p_ptr->au);
    rd_int32s(&p_ptr->max_exp);
    rd_int32s(&p_ptr->exp);
    rd_int16u(&p_ptr->exp_frac);
    rd_int16u(&p_ptr->age);
    rd_int16u(&p_ptr->ht);
    rd_int16u(&p_ptr->wt);
    rd_int16u(&p_ptr->lev);
    rd_int16u(&p_ptr->max_dlv);
    rd_int16s(&p_ptr->srh);
    rd_int16s(&p_ptr->fos);
    rd_int16s(&p_ptr->bth);
    rd_int16s(&p_ptr->bthb);
    rd_int16s(&p_ptr->mana);
    rd_int16s(&p_ptr->mhp);
    rd_int16s(&p_ptr->ptohit);
    rd_int16s(&p_ptr->ptodam);
    rd_int16s(&p_ptr->pac);
    rd_int16s(&p_ptr->ptoac);
    rd_int16s(&p_ptr->dis_th);
    rd_int16s(&p_ptr->dis_td);
    rd_int16s(&p_ptr->dis_ac);
    rd_int16s(&p_ptr->dis_tac);
    rd_int16s(&p_ptr->disarm);
    rd_int16s(&p_ptr->save);
    rd_int16s(&p_ptr->sc);
    rd_int16s(&p_ptr->stl);
    rd_byte(&p_ptr->pclass);
    rd_byte(&p_ptr->prace);
    rd_byte(&p_ptr->hitdie);
    rd_byte(&p_ptr->expfact);
    rd_int16s(&p_ptr->cmana);
    rd_int16u(&p_ptr->cmana_frac);
    rd_int16s(&p_ptr->chp);
    rd_int16u(&p_ptr->chp_frac);

    for (i = 0; i < 4; i++) {
	rd_string(history[i]);
    }

    /* Read the stats */    
    for (i = 0; i < 6; i++) rd_int16s(&p_ptr->max_stat[i]);
    if (older_than(2,5,7)) {
	for (i = 0; i < 6; i++) rd_int16s(&p_ptr->cur_stat[i]);
    }
    else {
	for (i = 0; i < 6; i++) {
	    rd_byte(&tmp8u);
	    p_ptr->cur_stat[i] = tmp8u;
	}
    }
    for (i = 0; i < 6; i++) rd_int16s(&p_ptr->mod_stat[i]);
    for (i = 0; i < 6; i++) rd_int16s(&p_ptr->use_stat[i]);

    /* Read the flags */
    rd_int32u(&p_ptr->status);
    rd_int16s(&p_ptr->rest);
    rd_int16s(&p_ptr->blind);
    rd_int16s(&p_ptr->paralysis);
    rd_int16s(&p_ptr->confused);
    rd_int16s(&p_ptr->food);
    rd_int16s(&p_ptr->food_digested);
    rd_int16s(&p_ptr->protection);
    rd_int16s(&p_ptr->speed);
    rd_int16s(&p_ptr->fast);
    rd_int16s(&p_ptr->slow);
    rd_int16s(&p_ptr->afraid);
    rd_int16s(&p_ptr->cut);
    rd_int16s(&p_ptr->stun);
    rd_int16s(&p_ptr->poisoned);
    rd_int16s(&p_ptr->image);
    rd_int16s(&p_ptr->protevil);
    rd_int16s(&p_ptr->invuln);
    rd_int16s(&p_ptr->hero);
    rd_int16s(&p_ptr->shero);
    rd_int16s(&p_ptr->shield);
    rd_int16s(&p_ptr->blessed);
    rd_int16s(&p_ptr->oppose_fire);
    rd_int16s(&p_ptr->oppose_cold);
    rd_int16s(&p_ptr->oppose_acid);
    rd_int16s(&p_ptr->oppose_elec);
    rd_int16s(&p_ptr->oppose_pois);
    rd_int16s(&p_ptr->detect_inv);
    rd_int16s(&p_ptr->word_recall);
    rd_int16s(&p_ptr->see_infra);
    rd_int16s(&p_ptr->tim_infra);
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

    /* XXX XXX XXX Hack -- fix somebodies stupidity */
    if (p_ptr->status & PY_SEARCH) p_ptr->searching = 1;
    
    /* Old "missile_ctr" */
    rd_int16u(&tmp16u);

    /* Current turn */
    rd_int32s(&tmp32s);
    turn = (tmp32s < 0) ? 0 : tmp32s;

    /* Last turn */
    if (older_than(2,6,0)) {
	old_turn = turn;
    }
    else {
	rd_int32s(&tmp32s);
	old_turn = (tmp32s < 0) ? 0 : tmp32s;
    }
}


/*
 * Read/Write the "extra" information
 */

static void rd_extra()
{
    int i;
    int8u tmp8u;
    int32u tmp32u;

    rd_string(player_name);

    rd_string(died_from);

    for (i = 0; i < 4; i++) {
	rd_string(history[i]);
    }

    /* Class/Race/Gender/Spells */
    rd_byte(&p_ptr->prace);
    rd_byte(&p_ptr->pclass);
    rd_byte(&p_ptr->male);
    rd_byte(&p_ptr->new_spells);

    /* Special Race/Class info */
    rd_byte(&p_ptr->hitdie);
    rd_byte(&p_ptr->expfact);

    /* Age/Height/Weight */
    rd_int16u(&p_ptr->age);
    rd_int16u(&p_ptr->ht);
    rd_int16u(&p_ptr->wt);

    /* Read the stats, Keep it simple */    
    for (i = 0; i < 6; i++) rd_int16s(&p_ptr->max_stat[i]);
    for (i = 0; i < 6; i++) rd_int16s(&p_ptr->cur_stat[i]);
    for (i = 0; i < 6; i++) rd_int16s(&p_ptr->mod_stat[i]);
    for (i = 0; i < 6; i++) rd_int16s(&p_ptr->use_stat[i]);

    rd_int32s(&p_ptr->au);

    rd_int32s(&p_ptr->max_exp);
    rd_int32s(&p_ptr->exp);
    rd_int16u(&p_ptr->exp_frac);
    rd_int16u(&p_ptr->lev);

    rd_int16s(&p_ptr->mhp);
    rd_int16s(&p_ptr->chp);
    rd_int16u(&p_ptr->chp_frac);

    rd_int16s(&p_ptr->mana);
    rd_int16s(&p_ptr->cmana);
    rd_int16u(&p_ptr->cmana_frac);

    rd_int16u(&p_ptr->max_plv);
    rd_int16u(&p_ptr->max_dlv);

    /* More info */
    rd_int16s(&p_ptr->srh);
    rd_int16s(&p_ptr->fos);
    rd_int16s(&p_ptr->disarm);
    rd_int16s(&p_ptr->save);
    rd_int16s(&p_ptr->sc);
    rd_int16s(&p_ptr->stl);
    rd_int16s(&p_ptr->bth);
    rd_int16s(&p_ptr->bthb);
    rd_int16s(&p_ptr->ptohit);
    rd_int16s(&p_ptr->ptodam);
    rd_int16s(&p_ptr->pac);
    rd_int16s(&p_ptr->ptoac);
    rd_int16s(&p_ptr->dis_th);
    rd_int16s(&p_ptr->dis_td);
    rd_int16s(&p_ptr->dis_ac);
    rd_int16s(&p_ptr->dis_tac);


    /* Read the flags */
    rd_int32u(&p_ptr->status);
    rd_int16s(&p_ptr->rest);
    rd_int16s(&p_ptr->blind);
    rd_int16s(&p_ptr->paralysis);
    rd_int16s(&p_ptr->confused);
    rd_int16s(&p_ptr->food);
    rd_int16s(&p_ptr->food_digested);
    rd_int16s(&p_ptr->protection);
    rd_int16s(&p_ptr->speed);
    rd_int16s(&p_ptr->fast);
    rd_int16s(&p_ptr->slow);
    rd_int16s(&p_ptr->afraid);
    rd_int16s(&p_ptr->cut);
    rd_int16s(&p_ptr->stun);
    rd_int16s(&p_ptr->poisoned);
    rd_int16s(&p_ptr->image);
    rd_int16s(&p_ptr->protevil);
    rd_int16s(&p_ptr->invuln);
    rd_int16s(&p_ptr->hero);
    rd_int16s(&p_ptr->shero);
    rd_int16s(&p_ptr->shield);
    rd_int16s(&p_ptr->blessed);
    rd_int16s(&p_ptr->detect_inv);
    rd_int16s(&p_ptr->word_recall);
    rd_int16s(&p_ptr->see_infra);
    rd_int16s(&p_ptr->tim_infra);
    rd_int16s(&p_ptr->oppose_fire);
    rd_int16s(&p_ptr->oppose_cold);
    rd_int16s(&p_ptr->oppose_acid);
    rd_int16s(&p_ptr->oppose_elec);
    rd_int16s(&p_ptr->oppose_pois);
    rd_byte(&p_ptr->immune_acid);
    rd_byte(&p_ptr->immune_elec);
    rd_byte(&p_ptr->immune_fire);
    rd_byte(&p_ptr->immune_cold);
    rd_byte(&p_ptr->immune_pois);
    rd_byte(&p_ptr->resist_acid);
    rd_byte(&p_ptr->resist_elec);
    rd_byte(&p_ptr->resist_fire);
    rd_byte(&p_ptr->resist_cold);
    rd_byte(&p_ptr->resist_pois);
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
    rd_byte(&p_ptr->resist_fear);
    rd_byte(&p_ptr->see_inv);
    rd_byte(&p_ptr->teleport);
    rd_byte(&p_ptr->free_act);
    rd_byte(&p_ptr->slow_digest);
    rd_byte(&p_ptr->aggravate);
    rd_byte(&p_ptr->regenerate);
    rd_byte(&p_ptr->ffall);
    rd_byte(&p_ptr->sustain_str);
    rd_byte(&p_ptr->sustain_int);
    rd_byte(&p_ptr->sustain_wis);
    rd_byte(&p_ptr->sustain_con);
    rd_byte(&p_ptr->sustain_dex);
    rd_byte(&p_ptr->sustain_chr);
    rd_byte(&p_ptr->confusing);
    rd_byte(&p_ptr->hold_life);
    rd_byte(&p_ptr->telepathy);
    rd_byte(&p_ptr->lite);
    rd_byte(&p_ptr->searching);

    /* Future use */
    for (i = 0; i < 3; i++) rd_byte(&tmp8u);
    for (i = 0; i < 15; i++) rd_int32u(&tmp32u);


    /* XXX XXX XXX Hack -- fix somebody's stupidity */
    /* XXX This is really a hack, and it probably won't work always */
    /* And even worse, there were "Two" version 2.7.0's, both different */
    if (older_than(2,7,1) && (p_ptr->status & PY_SEARCH)) p_ptr->searching = 1;
    


    /* Hack -- the two "special seeds" */            
    rd_int32u(&randes_seed);
    rd_int32u(&town_seed);


    /* Special stuff */
    rd_int16s(&panic_save);
    rd_int16s(&total_winner);
    rd_int16s(&noscore);


    /* Important -- Read "death" */
    rd_byte(&tmp8u);
    death = tmp8u;

    /* Read "feeling" */
    rd_byte(&tmp8u);
    feeling = tmp8u;

    /* Turn of last "feeling" */
    rd_int32u(&old_turn);

    /* Current turn */
    rd_int32u(&turn);
}

static void wr_extra()
{
    int i;

    wr_string(player_name);

    wr_string(died_from);

    for (i = 0; i < 4; i++) {
	wr_string(history[i]);
    }

    /* Race/Class/Gender/Spells */
    wr_byte(p_ptr->prace);
    wr_byte(p_ptr->pclass);
    wr_byte(p_ptr->male);
    wr_byte(p_ptr->new_spells);

    wr_byte(p_ptr->hitdie);
    wr_byte(p_ptr->expfact);

    wr_int16u(p_ptr->age);
    wr_int16u(p_ptr->ht);
    wr_int16u(p_ptr->wt);

    /* Dump the stats */
    for (i = 0; i < 6; ++i) wr_int16s(p_ptr->max_stat[i]);
    for (i = 0; i < 6; ++i) wr_int16s(p_ptr->cur_stat[i]);
    for (i = 0; i < 6; ++i) wr_int16s(p_ptr->mod_stat[i]);
    for (i = 0; i < 6; ++i) wr_int16s(p_ptr->use_stat[i]);

    wr_int32u(p_ptr->au);

    wr_int32u(p_ptr->max_exp);
    wr_int32u(p_ptr->exp);
    wr_int16u(p_ptr->exp_frac);
    wr_int16u(p_ptr->lev);

    wr_int16u(p_ptr->mhp);
    wr_int16u(p_ptr->chp);
    wr_int16u(p_ptr->chp_frac);

    wr_int16u(p_ptr->mana);
    wr_int16u(p_ptr->cmana);
    wr_int16u(p_ptr->cmana_frac);

    /* Max Player and Dungeon Levels */
    wr_int16u(p_ptr->max_plv);
    wr_int16u(p_ptr->max_dlv);

    /* More info */
    wr_int16u(p_ptr->srh);
    wr_int16u(p_ptr->fos);
    wr_int16u(p_ptr->disarm);
    wr_int16u(p_ptr->save);
    wr_int16u(p_ptr->sc);
    wr_int16u(p_ptr->stl);
    wr_int16u(p_ptr->bth);
    wr_int16u(p_ptr->bthb);
    wr_int16u(p_ptr->ptohit);
    wr_int16u(p_ptr->ptodam);
    wr_int16u(p_ptr->pac);
    wr_int16u(p_ptr->ptoac);
    wr_int16u(p_ptr->dis_th);
    wr_int16u(p_ptr->dis_td);
    wr_int16u(p_ptr->dis_ac);
    wr_int16u(p_ptr->dis_tac);

    /* XXX Warning -- some of these should be signed */
    wr_int32u(p_ptr->status);
    wr_int16u(p_ptr->rest);
    wr_int16u(p_ptr->blind);
    wr_int16u(p_ptr->paralysis);
    wr_int16u(p_ptr->confused);
    wr_int16u(p_ptr->food);
    wr_int16u(p_ptr->food_digested);
    wr_int16u(p_ptr->protection);
    wr_int16u(p_ptr->speed);
    wr_int16u(p_ptr->fast);
    wr_int16u(p_ptr->slow);
    wr_int16u(p_ptr->afraid);
    wr_int16u(p_ptr->cut);
    wr_int16u(p_ptr->stun);
    wr_int16u(p_ptr->poisoned);
    wr_int16u(p_ptr->image);
    wr_int16u(p_ptr->protevil);
    wr_int16u(p_ptr->invuln);
    wr_int16u(p_ptr->hero);
    wr_int16u(p_ptr->shero);
    wr_int16u(p_ptr->shield);
    wr_int16u(p_ptr->blessed);
    wr_int16u(p_ptr->detect_inv);
    wr_int16u(p_ptr->word_recall);
    wr_int16u(p_ptr->see_infra);
    wr_int16u(p_ptr->tim_infra);
    wr_int16u(p_ptr->oppose_fire);
    wr_int16u(p_ptr->oppose_cold);
    wr_int16u(p_ptr->oppose_acid);
    wr_int16u(p_ptr->oppose_elec);
    wr_int16u(p_ptr->oppose_pois);
    wr_byte(p_ptr->immune_acid);
    wr_byte(p_ptr->immune_elec);
    wr_byte(p_ptr->immune_fire);
    wr_byte(p_ptr->immune_cold);
    wr_byte(p_ptr->immune_pois);
    wr_byte(p_ptr->resist_acid);
    wr_byte(p_ptr->resist_elec);
    wr_byte(p_ptr->resist_fire);
    wr_byte(p_ptr->resist_cold);
    wr_byte(p_ptr->resist_pois);
    wr_byte(p_ptr->resist_conf);
    wr_byte(p_ptr->resist_sound);
    wr_byte(p_ptr->resist_lite);
    wr_byte(p_ptr->resist_dark);
    wr_byte(p_ptr->resist_chaos);
    wr_byte(p_ptr->resist_disen);
    wr_byte(p_ptr->resist_shards);
    wr_byte(p_ptr->resist_nexus);
    wr_byte(p_ptr->resist_blind);
    wr_byte(p_ptr->resist_nether);
    wr_byte(p_ptr->resist_fear);
    wr_byte(p_ptr->see_inv);
    wr_byte(p_ptr->teleport);
    wr_byte(p_ptr->free_act);
    wr_byte(p_ptr->slow_digest);
    wr_byte(p_ptr->aggravate);
    wr_byte(p_ptr->regenerate);
    wr_byte(p_ptr->ffall);
    wr_byte(p_ptr->sustain_str);
    wr_byte(p_ptr->sustain_int);
    wr_byte(p_ptr->sustain_wis);
    wr_byte(p_ptr->sustain_con);
    wr_byte(p_ptr->sustain_dex);
    wr_byte(p_ptr->sustain_chr);
    wr_byte(p_ptr->confusing);
    wr_byte(p_ptr->hold_life);
    wr_byte(p_ptr->telepathy);
    wr_byte(p_ptr->lite);
    wr_byte(p_ptr->searching);


    /* Future use */
    for (i = 0; i < 3; i++) wr_byte(0);
    for (i = 0; i < 15; i++) wr_int32u(0L);


    /* Write the "object seeds" */
    wr_int32u(randes_seed);
    wr_int32u(town_seed);


    /* Special stuff */
    wr_int16s(panic_save);
    wr_int16s(total_winner);
    wr_int16s(noscore);


    /* Write death */
    wr_byte(death);

    /* Write feeling */
    wr_byte(feeling);

    /* Turn of last "feeling" */
    wr_int32u(old_turn);

    /* Current turn */
    wr_int32u(turn);
}




/*
 * Write/Read the player inventory
 */

static errr rd_inventory()
{
    inven_ctr = 0;
    equip_ctr = 0;
    inven_weight = 0;

    /* Read until done */
    while (1) {

	int16u n;

	/* More items? */
	rd_int16u(&n);
	if (n == 0xFFFF) break;

	/* Verify the entry */
	if (n < INVEN_WIELD) {

	    /* Must be read in order */
	    if (n != (unsigned)(inven_ctr)) return (51);

	    /* One more item */
	    inven_ctr++;
	}

	/* Verify the equipment entry */
	else {

	    /* Must be read at most once each */
	    if (inventory[n].tval != TV_NOTHING) return (52);

	    /* One more item */
	    equip_ctr++;
	}

	/* Read the item */
	rd_item(&inventory[n]);

	/* Hack -- verify inventory */
	if (inventory[n].tval == TV_NOTHING) return (53);

	/* Add the weight */
	inven_weight += inventory[n].weight;
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
    char buf[1024];

    /* Hack -- load old "message recall" stuff */
    if (older_than(2,7,0)) {

	int16u last_msg;

	/* Hack -- old method used circular queue */
	rd_int16u(&last_msg);

	/* Minor Hack -- may lose some old messages */
	for (i = 0; i < 22; i++) {

	    /* Read the message */
	    rd_string(buf);

	    /* Save (most of) the messages */
	    if (buf[0] && (i <= last_msg)) message_new(buf, -1);
	}
    }

    /* New method for reading messages */
    else {

	int16u num;

	/* Hack -- old method used circular queue */
	rd_int16u(&num);

	/* Read the messages */
	for (i = 0; i < num; i++) {

	    /* Read the message */
	    rd_string(buf);

	    /* Save the message */
	    message_new(buf, -1);
	}
    }   
}



/* 
 * Write/Read the actual Dungeon
 */

static void wr_dungeon()
{
    int i, j;
    byte count, prev_char;
    int8u tmp8u;
    cave_type *c_ptr;

    /* Dungeon specific info follows */
    wr_int16u(dun_level);
    wr_int16u(mon_tot_mult);
    wr_int16u(char_row);
    wr_int16u(char_col);
    wr_int16u(cur_height);
    wr_int16u(cur_width);
    wr_int16u(max_panel_rows);
    wr_int16u(max_panel_cols);


    /*** Simple "Run-Length-Encoding" of cave ***/

    /* Note that this will induce two wasted bytes */
    count = 0;
    prev_char = 0;

    /* Dump the cave */
    for (i = 0; i < cur_height; i++) {
	for (j = 0; j < cur_width; j++) {

	    int8u t_lr, t_fm, t_pl, t_xx;
	    
	    /* Get the cave */
	    c_ptr = &cave[i][j];

	    /* XXX Paranoia -- verify iy,ix for later */
	    if (c_ptr->i_idx) {
		i_list[c_ptr->i_idx].iy = i;
		i_list[c_ptr->i_idx].ix = j;
	    }

	    /* XXX Paranoia -- verify fy,fx for later */
	    if (c_ptr->m_idx > 1) {
		m_list[c_ptr->m_idx].fy = i;
		m_list[c_ptr->m_idx].fx = j;
	    }

	    /* Extract the info */
	    t_lr = (c_ptr->info & CAVE_LR) ? 1 : 0;
	    t_fm = (c_ptr->info & CAVE_FM) ? 1 : 0;
	    t_pl = (c_ptr->info & CAVE_PL) ? 1 : 0;
	    t_xx = 0;
	    
	    /* Create an encoded byte of info */            
	    tmp8u = (c_ptr->fval);
	    tmp8u |= ((t_lr << 4) | (t_fm << 5) | (t_pl << 6) | (t_xx << 7));

	    /* If the run is broken, or too full, flush it */
	    if (tmp8u != prev_char || count == MAX_UCHAR) {
		wr_byte((int8u) count);
		wr_byte(prev_char);
		prev_char = tmp8u;
		count = 1;
	    }

	    /* Continue the run */
	    else {
		count++;
	    }
	}
    }

    /* Flush the data (if any) */
    if (count) {
	wr_byte((int8u) count);
	wr_byte(prev_char);
    }


    /* Dump the items (note: starting at #1) */
    wr_int16u(i_max);
    for (i = MIN_I_IDX; i < i_max; i++) {
	wr_item(&i_list[i]);
    }


    /* Dump the monsters (note: starting at #2) */    
    wr_int16u(m_max);
    for (i = MIN_M_IDX; i < m_max; i++) {
	wr_monster(&m_list[i]);
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
    int8u tmp8u;
    int16u tmp16u;
    int total_count;
    cave_type *c_ptr;


    /* Header info */            
    rd_int16s(&dun_level);
    rd_int16s(&char_row);
    rd_int16s(&char_col);
    rd_int16s(&mon_tot_mult);
    rd_int16s(&cur_height);
    rd_int16s(&cur_width);
    rd_int16s(&max_panel_rows);
    rd_int16s(&max_panel_cols);


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
	    rd_int16u(&tmp16u);
	    cave[ychar][xchar].m_idx = tmp16u;
	}
    }

    /* read in the treasure ptr info */
    while (1) { 
	rd_byte(&tmp8u);
	if (tmp8u == 0xFF) break;
	ychar = tmp8u;
	rd_byte(&xchar);
	rd_int16u(&tmp16u);
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
	    if ((tmp8u >> 7) & 0x1) i = i; /* was: CAVE_TL */

	    /* Old savefiles need to field-mark perma-lit grids */
            if (c_ptr->info & CAVE_PL) c_ptr->info |= CAVE_FM;
            
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
    rd_int16s(&i_max);
    if (i_max > MAX_I_IDX) {
	prt_note(-2, "Too many objects");
	return (92);
    }

    /* Read the dungeon items */
    for (i = MIN_I_IDX; i < i_max; i++) {
	inven_type *i_ptr = &i_list[i];

	/* Read the item */
	rd_item_old(i_ptr);

	/* XXX Convert "covered pits" / "spiked pits" into "pits" */

	/* XXX Hack -- No trapdoors */
	if ((i_ptr->tval == TV_INVIS_TRAP ||
	     i_ptr->tval == TV_VIS_TRAP) &&
	    (i_ptr->sval == SV_TRAP_TRAP_DOOR)) {

	    /* Make it a spiked pit */
	    i_ptr->sval = SV_TRAP_SPIKED_PIT;
	}
    }


    /* Read the monster count */        
    rd_int16s(&m_max);
    if (m_max > MAX_M_IDX) {
	prt_note(-2, "Too many monsters");
	return (93);
    }

    /* Read the monsters */
    for (i = MIN_M_IDX; i < m_max; i++) {
	monster_type *m_ptr = &m_list[i];
	rd_monster(m_ptr);
	cave[m_ptr->fy][m_ptr->fx].m_idx = i;
    }


    /* Read the ghost info */
    rd_ghost_old();


    /* Success */
    return (0);
}





/* 
 * New Method
 */

static errr rd_dungeon()
{
    int i;
    byte count;
    byte ychar, xchar;
    int8u tmp8u;
    int ymax, xmax;
    int total_count;
    cave_type *c_ptr;


    /* Header info */            
    rd_int16s(&dun_level);
    rd_int16s(&mon_tot_mult);
    rd_int16s(&char_row);
    rd_int16s(&char_col);
    rd_int16s(&cur_height);
    rd_int16s(&cur_width);
    rd_int16s(&max_panel_rows);
    rd_int16s(&max_panel_cols);

    /* Only read as necessary */    
    ymax = cur_height;
    xmax = cur_width;

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
	    if (ychar >= ymax) {
		prt_note(-2, "Dungeon too big!");
		return (81);
	    }

	    /* Access the cave */
	    c_ptr = &cave[ychar][xchar];

	    /* Extract the floor type */
	    c_ptr->fval = tmp8u & 0xF;

	    /* Extract the "info" */
	    c_ptr->info = 0;
	    if ((tmp8u >> 4) & 0x1) c_ptr->info |= CAVE_LR;
	    if ((tmp8u >> 5) & 0x1) c_ptr->info |= CAVE_FM;
	    if ((tmp8u >> 6) & 0x1) c_ptr->info |= CAVE_PL;
	    if ((tmp8u >> 7) & 0x1) i = i; /* Was: CAVE_TL */

	    /* Hack -- Version 2.7.0 savefiles forgot something */
            if (older_than(2,7,1) && (c_ptr->info & CAVE_PL)) {
                
                /* Field mark grids the player can, and has, seen */
                c_ptr->info |= CAVE_FM;
	    }
	    
	    /* Advance the cave pointers */
	    xchar++;

	    /* Wrap to the next line */
	    if (xchar >= xmax) {
		xchar = 0;
		ychar++;
	    }
	}

	total_count += count;
    }


    /* XXX Note that "player inventory" and "store inventory" */
    /* are NOT kept in the "i_list" array.  Only dungeon items. */

    /* Read the item count */
    rd_int16s(&i_max);
    if (i_max > MAX_I_IDX) {
	prt_note(-2, "Too many objects");
	return (92);
    }

    /* Read the dungeon items, note locations in cave */
    for (i = MIN_I_IDX; i < i_max; i++) {
	inven_type *i_ptr = &i_list[i];
	rd_item(i_ptr);
	cave[i_ptr->iy][i_ptr->ix].i_idx = i;
    }


    /* Note the player location in the cave */
    cave[char_row][char_col].m_idx = 1;


    /* Read the monster count */        
    rd_int16s(&m_max);
    if (m_max > MAX_M_IDX) {
	prt_note(-2, "Too many monsters");
	return (93);
    }

    /* Read the monsters, note locations in cave, count by race */
    for (i = MIN_M_IDX; i < m_max; i++) {
	monster_type *m_ptr = &m_list[i];
	rd_monster(m_ptr);
	cave[m_ptr->fy][m_ptr->fx].m_idx = i;
	l_list[m_ptr->r_idx].cur_num++;
    }


    /* Success */
    return (0);
}



/*
 * Read an old savefile
 */

static void rd_options_old()
{
    int16u tmp16u;
    int32u l;

    /* "Log Index" (unused) */
    rd_int16u(&tmp16u);

    /* Standard options */
    rd_int32u(&l);

    /* Extract death */
    death = (l & 0x80000000) ? TRUE : FALSE;

    /* Extract delay speed */
    delay_spd = ((l >> 13) & 0xfL);
    if (delay_spd > 9) delay_spd = 9;
    if (delay_spd < 0) delay_spd = 0;

    /* Extract hitpoint warning */
    hitpoint_warn = ((l >> 17) & 0xfL);
    if (hitpoint_warn > 9) hitpoint_warn = 9;
    if (hitpoint_warn < 0) hitpoint_warn = 0;

    /* Extract the feeling */
    feeling = ((l >> 24) & 0xfL);
    if (feeling > 10) feeling = 0;
    if (feeling < 0) feeling = 0;

    /* Extract the other flags */
    find_cut = (l & 0x0001) ? TRUE : FALSE;
    find_examine = (l & 0x0002) ? TRUE : FALSE;
    find_prself = (l & 0x0004) ? TRUE : FALSE;
    find_bound = (l & 0x0008) ? TRUE : FALSE;
    prompt_carry_flag = (l & 0x0010) ? TRUE : FALSE;
    rogue_like_commands = (l & 0x0020) ? TRUE : FALSE;
    show_inven_weight = (l & 0x0040) ? TRUE : FALSE;
    notice_seams = (l & 0x0080) ? TRUE : FALSE;
    find_ignore_doors = (l & 0x0100L) ? TRUE : FALSE;
    no_haggle_flag = (l & 0x0200L) ? TRUE : FALSE;
    carry_query_flag = (l & 0x0400L) ? FALSE : TRUE;
    plain_descriptions = (l & 0x00400000L) ? TRUE : FALSE;
    show_equip_weight = (l & 0x00800000L) ? TRUE : FALSE;
    equippy_chars = (l & 0x20000000L) ? TRUE : FALSE;
    quick_messages = (l & 0x40000000L) ? TRUE : FALSE;

    /* More options */
    if (!older_than(2,6,0)) {


	/* Broken options (just ignore them) */
	rd_int32u(&l);

	/* Hack -- grab some options */
	always_throw = (l & 0x00000001L) ? TRUE : FALSE;
	ring_bell = (l & 0x00000002L) ? TRUE : FALSE;
	shuffle_owners = (l & 0x00000004L) ? TRUE : FALSE;
	new_screen_layout = (l & 0x00000008L) ? TRUE : FALSE;


	/* Unused options */
	rd_int32u(&l);
	rd_int32u(&l);
    }
}

static errr rd_savefile_old()
{
    int i;
    int8u tmp8u;
    int16u tmp16u;
    int32u tmp32u;


    /* XXX Fake the system info */


    /* Read the artifacts */
    rd_artifacts_old();
    if (say) prt_note(-1,"Loaded Artifacts");


    /* Load the Quests */
    rd_int32u(&tmp32u);
    q_list[0].level = tmp32u ? 99 : 0;
    rd_int32u(&tmp32u);
    q_list[1].level = 100;
    rd_int32u(&tmp32u);
    q_list[2].level = 0;
    rd_int32u(&tmp32u);
    q_list[3].level = 0;
    if (say) prt_note(-1,"Loaded Quests");


    /* Load the old "Uniques" flags */
    for (i = 0; i < MAX_R_IDX; i++) {

	/* Ignore the "exist" info, extracted later */
	rd_int32u(&tmp32u);
	
	/* Already true, but do it again anyway */
	l_list[i].cur_num = 0;

	/* XXX Hack -- That unique has been killed */
	rd_int32u(&tmp32u);
	l_list[i].max_num = 255;
	if (r_list[i].cflags2 & MF2_UNIQUE) l_list[i].max_num = 1;
	if (tmp32u) l_list[i].max_num = 0;
    }
    if (say) prt_note(-1,"Loaded Unique Beasts");


    /* Load the recall memory */
    while (1) {

	/* Read some info, check for sentinal */
	rd_int16u(&tmp16u);
	if (tmp16u == 0xFFFF) break;

	/* Incompatible save files */
	if (tmp16u >= MAX_R_IDX) {
	    prt_note(-2,"Too many monsters!");
	    return (21);
	}

	/* Extract the monster lore */
	rd_lore(&l_list[tmp16u]);
    }
    if (say) prt_note(-1,"Loaded Monster Memory");


    /* Read the old options */
    rd_options_old();
    if (say) prt_note(-1, "Loaded options");

    /* Read the extra stuff */
    rd_extra_old();
    if (say) prt_note(-1, "Loaded extra information");


    /* Read the inventory */
    rd_int16s(&inven_ctr);
    if (inven_ctr > INVEN_WIELD) {
	prt_note(-2, "Unable to read inventory");
	return (15);
    }
    for (i = 0; i < inven_ctr; i++) {
	rd_item_old(&inventory[i]);
    }
    for (i = INVEN_WIELD; i < INVEN_ARRAY_SIZE; i++) {
	rd_item_old(&inventory[i]);
    }
    rd_int16s(&inven_weight);
    rd_int16s(&equip_ctr);


    /* Read spell info */
    rd_int32u(&spell_learned);
    rd_int32u(&spell_worked);
    rd_int32u(&spell_forgotten);
    rd_int32u(&spell_learned2);
    rd_int32u(&spell_worked2);
    rd_int32u(&spell_forgotten2);

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
    rd_int32u(&randes_seed);
    rd_int32u(&town_seed);

    /* Old messages */
    rd_messages();

    /* Some leftover info */
    rd_int16s(&panic_save);
    rd_int16s(&total_winner);
    rd_int16s(&noscore);

    /* XXX Why was this here? */
    /* rd_int16s(&tmp16s); */

    /* Hack -- Read the player_hp array */
    if (older_than(2,6,2)) {
	for (i = 0; i < 50; i++) rd_int16u(&player_hp[i]);
    }
    else {
	for (i = 0; i < 99; i++) rd_int16u(&player_hp[i]);
    }

    if (say) prt_note(-1, "Read some more information.");


    /* Read the stores */
    for (i = 0; i < MAX_STORES; i++) {
	if (rd_store(&store[i])) {
	    prt_note(-2,"ERROR reading store");
	    return (32);
	}
    }


    /* Time at which file was saved */
    rd_int32u(&sf_when);

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
		if (rd_store(&store[i])) {
		    prt_note(-2,"ERROR in STORE_INVEN_MAX");
		    return (33);
		}
	    }
	}


	/* Time goes here, too */
	rd_int32u(&sf_when);
    }



    /* Assume success */
    return (0);
}




/*
 * Actually read the savefile
 */
static errr rd_savefile()
{
    int i;

    int8u tmp8u;
    int16u tmp16u;
    int32u tmp32u;

#ifdef VERIFY_CHECKSUMS
    int32u n_x_check, n_v_check;
    int32u o_x_check, o_v_check;
#endif


    prt_note(0,"Restoring Memory...");

    /* Get the version info */
    xor_byte = 0;
    rd_byte(&version_maj);
    xor_byte = 0;
    rd_byte(&version_min);
    xor_byte = 0;
    rd_byte(&patch_level);
    xor_byte = 0;
    rd_byte(&xor_byte);


    /* Clear the checksums */
    v_check = 0L;
    x_check = 0L;


    /* Handle stupidity from Angband 2.4 / 2.5 */
    if ((version_maj == 5) && (version_min == 2)) {
	version_maj = 2;
	version_min = 5;
    }


    /* Verify the "major version" */
    if (version_maj != CUR_VERSION_MAJ) {

	prt_note(-2,"This savefile is from a different version of Angband.");
	return (11);
    }


    /* XXX Hack -- We cannot read savefiles more recent than we are */
    if ((version_min > CUR_VERSION_MIN) ||
	(version_min == CUR_VERSION_MIN && patch_level > CUR_PATCH_LEVEL)) {

	prt_note(-2,"This savefile is from a more recent version of Angband.");
	return (12);
    }


    /* Begin Wizard messages */
    if (say) prt_note(-2,"Loading savefile...");


    /* Hack -- parse old savefiles */
    if (older_than(2,7,0)) return (rd_savefile_old());


    /* Operating system info */
    rd_int32u(&sf_xtra);

    /* Time of savefile creation */
    rd_int32u(&sf_when);

    /* Number of resurrections */
    rd_int16u(&sf_lives);

    /* Number of times played */
    rd_int16u(&sf_saves);


    /* A "sized" chunk of "unused" space */
    rd_int32u(&tmp32u);

    /* Read (and forget) those bytes */
    for (i = 0; i < tmp32u; i++) rd_byte(&tmp8u);


    /* A "sized" list of "strings" */
    rd_int32u(&tmp32u);

    /* Read (and forget) those strings */
    for (i = 0; i < tmp32u; i++) {

	/* Read and forget a string */
	while (1) {
	    rd_byte(&tmp8u);
	    if (!tmp8u) break;
	}
    }


    /* Then the options */
    rd_options();
    if (say) prt_note(-1,"Loaded Option Flags");


    /* Then the "messages" */
    rd_messages();
    if (say) prt_note(-1,"Loaded Messages");


    /* Monster Memory */
    rd_int16u(&tmp16u);
    for (i = 0; i < tmp16u; i++) rd_lore(&l_list[i]);
    if (say) prt_note(-1,"Loaded Monster Memory");


    /* Object Memory */
    rd_int16u(&tmp16u);
    for (i = 0; i < tmp16u; i++) rd_xtra(&x_list[i]);
    if (say) prt_note(-1,"Loaded Object Memory");


    /* Load the Quests */
    rd_int16u(&tmp16u);
    for (i = 0; i < tmp16u; i++) {
	rd_byte(&tmp8u);
	q_list[i].level = tmp8u;
	rd_byte(&tmp8u);
	rd_byte(&tmp8u);
	rd_byte(&tmp8u);
    }
    if (say) prt_note(-1,"Loaded Quests");


    /* Load the Artifacts */
    rd_int16u(&tmp16u);
    for (i = 0; i < tmp16u; i++) {
	rd_byte(&tmp8u);
	v_list[i].cur_num = tmp8u;
	rd_byte(&tmp8u);
	rd_byte(&tmp8u);
	rd_byte(&tmp8u);
    }
    if (say) prt_note(-1,"Loaded Artifacts");


    /* Read the extra stuff */
    rd_extra();
    if (say) prt_note(-1, "Loaded extra information");


    /* Read the player_hp array */
    rd_int16u(&tmp16u);
    for (i = 0; i < tmp16u; i++) {
	rd_int16u(&player_hp[i]);
    }


    /* Read spell info */
    rd_int32u(&spell_learned);
    rd_int32u(&spell_learned2);
    rd_int32u(&spell_worked);
    rd_int32u(&spell_worked2);
    rd_int32u(&spell_forgotten);
    rd_int32u(&spell_forgotten2);

    for (i = 0; i < 64; i++) {
	rd_byte(&spell_order[i]);
    }


    /* Read the inventory */
    if (rd_inventory()) {
	prt_note(-2, "Unable to read inventory");
	return (21);
    }


    /* Read the stores */
    rd_int16u(&tmp16u);
    for (i = 0; i < tmp16u; i++) {
	if (rd_store(&store[i])) return (22);
    }


    /* I'm not dead yet... */
    if (!death) {

	/* Dead players have no dungeon */
	prt_note(-1,"Restoring Dungeon...");
	if (rd_dungeon()) {
	    prt_note(-2, "Error reading dungeon data");
	    return (34);
	}

	/* Read the ghost info */
	rd_ghost();
    }


#ifdef VERIFY_CHECKSUMS

    /* Save the checksum */
    n_v_check = v_check;

    /* Read the old checksum */
    rd_int32u(&o_v_check);

    /* Verify */
    if (o_v_check != n_v_check) {
	prt_note(-2, "Invalid checksum");
	return (11);
    }


    /* Save the encoded checksum */
    n_x_check = x_check;

    /* Read the checksum */
    rd_int32u(&o_x_check);


    /* Verify */
    if (o_x_check != n_x_check) {
	prt_note(-2, "Invalid encoded checksum");
	return (11);
    }

#endif


    /* Success */
    return (0);
}




/*
 * Actually write a save-file
 */

static int wr_savefile()
{
    register int        i;

    int32u              now;

    int8u		tmp8u;
    int16u		tmp16u;


    /* Guess at the current time */
    now = time((time_t *)0);

#if 0
    /* If someone is messing with the clock, assume one day of play time */
    if (now < sf_when) now = sf_when + 86400L;
#endif


    /* Note the operating system */
    sf_xtra = 0L;

    /* Note when the file was saved */
    sf_when = now;

    /* Note the number of saves */
    sf_saves++;


    /*** Actually write the file ***/

    /* Dump the file header */
    xor_byte = 0;
    wr_byte(CUR_VERSION_MAJ);
    xor_byte = 0;
    wr_byte(CUR_VERSION_MIN);
    xor_byte = 0;
    wr_byte(CUR_PATCH_LEVEL);
    xor_byte = 0;
    tmp8u = rand_int(256);
    wr_byte(tmp8u);


    /* Reset the checksum */
    v_check = 0L;
    x_check = 0L;


    /* Operating system */
    wr_int32u(sf_xtra);


    /* Time file last saved */
    wr_int32u(sf_when);

    /* Number of past lives */
    wr_int16u(sf_lives);

    /* Number of times saved */
    wr_int16u(sf_saves);


    /* No extra bytes for this operating system */
    wr_int32u(0L);

    /* No extra strings for this operating system */
    wr_int32u(0L);


    /* Write the boolean "options" */
    wr_options();


    /* Dump the number of "messages" */
    tmp16u = message_num();
    if (compress_savefile && (tmp16u > 40)) tmp16u = 40;
    wr_int16u(tmp16u);

    /* Dump the messages (oldest first!) */
    for (i = tmp16u - 1; i >= 0; i--) {
	wr_string(message_str(i));
    }


    /* XXX This could probably be more "efficient" for "unseen" monsters */
    /* XXX But note that "max_num" is stored here as well (always "set") */
    
    /* Dump the monster lore */
    tmp16u = MAX_R_IDX;
    wr_int16u(tmp16u);
    for (i = 0; i < tmp16u; i++) wr_lore(&l_list[i]);


    /* Dump the object memory */
    tmp16u = MAX_K_IDX;
    wr_int16u(tmp16u);
    for (i = 0; i < tmp16u; i++) wr_xtra(&x_list[i]);


    /* Hack -- Dump the quests */    
    tmp16u = QUEST_MAX;
    wr_int16u(tmp16u);
    for (i = 0; i < tmp16u; i++) {
	wr_byte(q_list[i].level);
	wr_byte(0);
	wr_byte(0);
	wr_byte(0);
    }

    /* Hack -- Dump the artifacts */
    tmp16u = ART_MAX;
    wr_int16u(tmp16u);
    for (i = 0; i < tmp16u; i++) {
	wr_byte(v_list[i].cur_num);
	wr_byte(0);
	wr_byte(0);
	wr_byte(0);
    }



    /* Write the "extra" information */
    wr_extra();


    /* Dump the "player hp" entries */
    tmp16u = MAX_PLAYER_LEVEL;
    wr_int16u(tmp16u);
    for (i = 0; i < tmp16u; i++) {
	wr_int16u(player_hp[i]);
    }


    /* Write spell data */
    wr_int32u(spell_learned);
    wr_int32u(spell_learned2);
    wr_int32u(spell_worked);
    wr_int32u(spell_worked2);
    wr_int32u(spell_forgotten);
    wr_int32u(spell_forgotten2);

    /* Dump the ordered spells */
    for (i = 0; i < 64; i++) {
	wr_byte(spell_order[i]);
    }


    /* Write the inventory */
    for (i = 0; i < INVEN_ARRAY_SIZE; i++) {
	if (inventory[i].tval != TV_NOTHING) {
	    wr_int16u(i);
	    wr_item(&inventory[i]);
	}
    }    

    /* Add a sentinel */
    wr_int16u(0xFFFF);


    /* Note the stores */
    tmp16u = MAX_STORES;
    wr_int16u(tmp16u);

    /* Dump the stores */
    for (i = 0; i < tmp16u; i++) wr_store(&store[i]);


    /* Player is not dead, write the dungeon */
    if (!death) {

	/* Dump the dungeon */
	wr_dungeon();

	/* Dump the ghost */
	wr_ghost();
    }


    /* Write the "value check-sum" */
    wr_int32u(v_check);

    /* Write the "encoded checksum" */    
    wr_int32u(x_check);


    /* Error in save */
    if (ferror(fff) || (fflush(fff) == EOF)) return FALSE;

    /* Successful save */
    return TRUE;
}


/*
 * Medium level player saver
 */
int _save_player(char *fnam)
{
    int   ok, fd;
    vtype temp;

    /* Forbid suspend */
    signals_ignore_tstp();

    Term_fresh();
    disturb(1, 0);		   /* Turn off resting and searching. */

    /* Assume failure */
    ok = FALSE;

#ifdef ATARIST_MWC

    fff = my_tfopen(fnam, "wb");

#else /* ATARIST_MWC */

    fd = (-1);
    fff = NULL;		   /* Do not assume it has been init'ed */

#ifdef SET_UID
    fd = my_topen(fnam, O_RDWR | O_CREAT | O_EXCL | O_BINARY, 0600);
#else
    fd = my_topen(fnam, O_RDWR | O_CREAT | O_EXCL | O_BINARY, 0666);
#endif

#ifdef MACINTOSH

    if (fd < 0) {
	msg_print("Cannot write to savefile!");
    }

#else

    /* This might not work... */
    if ((fd < 0) && (access(fnam, 0) >= 0) &&
	(from_savefile ||
	 (wizard && get_check("Can't make new savefile. Overwrite old?")))) {

#ifdef SET_UID
	(void)chmod(fnam, 0600);
	fd = my_topen(fnam, O_RDWR | O_TRUNC | O_BINARY, 0600);
#else
	(void)chmod(fnam, 0666);
	fd = my_topen(fnam, O_RDWR | O_TRUNC | O_BINARY, 0666);
#endif

    }

#endif

    if (fd >= 0) {

	/* Close the "fd" */
	(void)close(fd);
	
	/* The save file is a binary file */
	fff = my_tfopen(fnam, "wb");
    }

#endif

    /* Successful open */
    if (fff) {

	int hack_pack = pack_heavy;
	
	/* Forget the "torch lite" */
	forget_lite();
	forget_view();
    
	/* Fix the speed */
	change_speed(-pack_heavy);

	/* Forget the weight */
	pack_heavy = 0;

	/* XXX XXX Hack -- clear the death flag when creating a HANGUP */
	/* save file so that player can see tombstone when restart. */
	if (eof_flag) death = FALSE;

	/* Write the savefile */
	ok = wr_savefile();

	/* Attempt to close it */
	if (fclose(fff) == EOF) ok = FALSE;

	/* Undo the changes above */
	pack_heavy = hack_pack;
	change_speed(pack_heavy);

	/* Fix the lite/view */
	update_view();
	update_lite();    
    }


    /* Error */
    if (!ok) {

	if (fd >= 0) (void)unlink(fnam);

	/* Allow suspend again */
	signals_handle_tstp();

	/* Oops */
	if (fd >= 0) (void)sprintf(temp, "Error writing to savefile");
	else (void)sprintf(temp, "Can't create new savefile");
	msg_print(temp);
	return FALSE;
    }

    /* Successful save */
    character_saved = 1;

    /* Allow suspend again */
    signals_handle_tstp();

    /* Successful save */
    return TRUE;
}



/*
 * Attempt to save the player in a savefile
 */
int save_player()
{
    int result = FALSE;
    bigvtype temp1, temp2;

#ifdef SECURE
    beGames();
#endif


    /* save file with extensions */
    (void)sprintf(temp1, "%s.new", savefile);
    (void)sprintf(temp2, "%s.old", savefile);

    /* Attempt to save the player */   
    if (_save_player(temp1)) {

	/* preserve old savefile */
	rename(savefile, temp2);

	/* activate new savefile */
	rename(temp1, savefile);

	/* remove preserved savefile */
	remove(temp2);

	/* Can re-write the savefile */
	from_savefile=1;

	/* Success */
	result = TRUE;
    }

#ifdef SECURE
    bePlayer();
#endif

    /* Return the result */
    return (result);
}








/*
 * Version 2.7.0 uses an entirely different "savefile" format.
 * It can still read the old files, though it may lose a little
 * data in transfer, in particular, some of the "saved messages".
 *
 * Note that versions "5.2.x" can never be made.
 * This boneheadedness is a direct result of the fact that Angband 2.4
 * had version constants of 5.2, not 2.4.  2.5 inherited this.  2.6 fixes
 * the problem.  Note that there must never be a 5.2.x version of Angband,
 * or else this code will get confused. -CWS
 *
 * Actually, this is not true, since by the time version 5.2 comes around,
 * anybody trying to use a version 2.5 savefile deserves what they get!
 */

int load_player(int *generate)
{
    int                    i, fd, ok, days;
    int32u                 age;

    vtype temp;


    /* Should be print "wizard" messages? */
    int wiz = to_be_wizard;


    /* Set "say" as well */
    if (wiz) say = TRUE;


    /* Forbid suspend */
    signals_ignore_tstp();

    /* Assume a cave must be generated */
    *generate = TRUE;

    /* Assume no file (used to catch errors below) */
    fd = (-1);


    /* Hack -- Cannot restore a game while still playing */
    if (turn > 0) {
	msg_print("IMPOSSIBLE! Attempt to restore while still alive!");
	return (FALSE);
    }



#ifndef MACINTOSH

    if (access(savefile, 0) < 0) {

	/* Allow suspend again */
	signals_handle_tstp();

	msg_print("Savefile does not exist.");
	return FALSE;
    }

#endif


    clear_screen();

    (void)sprintf(temp, "Restoring Character.");
    put_str(temp, 23, 0);
    sleep(1);

    /* Allow restoring a file belonging to someone else, */
    /* but only if we can delete it. */
    /* Hence first try to read without doing a chmod. */

    /* Open the BINARY savefile */
    fd = my_topen(savefile, O_RDONLY | O_BINARY, 0);

    if (fd < 0) {
	msg_print("Can't open file for reading.");
    }

    else {

#ifndef SET_UID
	struct stat         statbuf;
#endif

	/* Already done, but can't hurt */
	turn = 0;

	ok = TRUE;

#ifndef SET_UID
	(void)fstat(fd, &statbuf);
#endif

	(void)close(fd);

#if defined(__MINT__) || defined(atarist) || defined(ATARIST_MWC) || \
    defined(MSDOS) || defined(MACINTOSH)
	/* The savefile is definitely a binary file */
	fff = my_tfopen(savefile, "rb");
#else
	/* Is the savefile not a binary file? */
	fff = my_tfopen(savefile, "r");
#endif

	if (!fff) goto error;


	/* Actually read the savefile */
	if (rd_savefile()) goto error;

	/* Hack -- Alive, so no need to make a cave */
	if (!death) *generate = FALSE;


#ifndef SET_UID
#ifndef ALLOW_FIDDLING
	if (!wiz) {
	    if (sf_when > (statbuf.st_ctime + 100) ||
		sf_when < (statbuf.st_ctime - 100)) {
		prt_note(-2,"Fiddled save file");
		goto error;
	    }
	}
#endif
#endif



	/* Check for errors */
	if (ferror(fff)) {
	    prt_note(-2,"FILE ERROR");
	    goto error;
	}


	/* Process "dead" players */
	if (death) {

	    /* Wizards can revive dead characters */
	    if (wiz && get_check("Resurrect a dead character?")) {

		/* Revive the player */
		prt_note(0,"Attempting a resurrection!");

		/* Not quite dead */
		if (p_ptr->chp < 0) {
		    p_ptr->chp = 0;
		    p_ptr->chp_frac = 0;
		}

		/* don't let him starve to death immediately */
		if (p_ptr->food < 5000) p_ptr->food = 5000;

		cure_poison();
		cure_blindness();
		cure_confusion();
		remove_fear();

		if (p_ptr->image > 0) p_ptr->image = 0;
		if (p_ptr->cut > 0) p_ptr->cut = 0;

		if (p_ptr->stun > 0) {
		    if (p_ptr->stun > 50) {
			p_ptr->ptohit += 20;
			p_ptr->ptodam += 20;
		    }
		    else {
			p_ptr->ptohit += 5;
			p_ptr->ptodam += 5;
		    }
		    p_ptr->stun = 0;
		}

		if (p_ptr->word_recall > 0) p_ptr->word_recall = 0;

		/* Resurrect on the town level. */
		dun_level = 0;

		/* Set character_generated */
		character_generated = 1;

		/* set noscore to indicate a resurrection */
		noscore |= 0x1;

		/* XXX Don't enter wizard mode */
		to_be_wizard = FALSE;

		/* Player is no longer "dead" */
		death = FALSE;

		/* Hack -- force legal "turn" */
		if (turn < 1) turn = 1;
	    }

	    /* Normal "restoration" */
	    else {

		prt_note(0,"Restoring Memory of a departed spirit...");

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


	if (!turn) {

	    prt_note(-2,"Invalid turn");

error:

	    /* Assume bad data. */
	    ok = FALSE;
	}

	else {

	    /* don't overwrite the "killed by" string if character is dead */
	    if (p_ptr->chp >= 0) {
		(void)strcpy(died_from, "(alive and well)");
	    }

	    character_generated = 1;
	}

closefiles:

	if (fff) {
	    if (fclose(fff) < 0) ok = FALSE;
	}

	if (!ok) {
	    msg_print("Error during reading of file.");
	    msg_print(NULL);
	}

	else {

	    /* Hack -- Let the user overwrite the old savefile when save/quit */
	    from_savefile = 1;

	    /* Allow suspend again */
	    signals_handle_tstp();

	    /* Only if a full restoration. */
	    if (turn > 0) {


		/* Forget about weight problems */
		weapon_heavy = FALSE;
		pack_heavy = 0;
		
		/* Check the strength */
		check_strength();


		/* rotate store inventory, based on time passed */
		/* foreach day old (rounded up), call store_maint */

		/* Get the current time */
		age = time((time_t *)0);

		/* Subtract the save-file time */
		age = age - sf_when;

		/* Convert age to "real time" in days */
		days = age / 86400L;

		/* Assume no more than 10 days old */
		if (days > 10) days = 10;

		/* Rotate the store inventories (once per day) */
		for (i = 0; i < days; i++) store_maint();


		/* Combine items where possible */
		combine_pack();            


		/* Older savefiles do not save location */
		if (older_than(2,7,0)) {

		    int y, x;

		    /* Check the objects/monsters */
		    for (y = 0; y < cur_height; y++) {
			for (x = 0; x < cur_width; x++) {

			    /* Objects -- extract location */
			    if (cave[y][x].i_idx) {
				inven_type *i_ptr;
				i_ptr = &i_list[cave[y][x].i_idx];
				i_ptr->iy = y;
				i_ptr->ix = x;
			    }

			    /* Monsters -- count total population */
			    if (cave[y][x].m_idx) {
				monster_type *m_ptr;
				m_ptr = &m_list[cave[y][x].m_idx];
				l_list[m_ptr->r_idx].cur_num++;
			    }
			}
		    }
		}


		/* Update the monsters (to set "cdis") */
		update_monsters();
	    }

#if 0
	    if (noscore) {
		msg_print("This savefile cannot yield high scores.");
	    }
#endif

	    /* Give a warning */
	    if (version_min != CUR_VERSION_MIN ||
		patch_level != CUR_PATCH_LEVEL) {

		(void)sprintf(temp,
			"Save file from version %d.%d.%d %s game version %d.%d.%d.",
			      version_maj, version_min, patch_level,
			      ((version_min == CUR_VERSION_MIN) ?
			       "accepted for" : "converted for"),
			      CUR_VERSION_MAJ, CUR_VERSION_MIN, CUR_PATCH_LEVEL);
		msg_print(temp);
	    }

	    /* Return "a real character was restored" */
	    return (turn > 0);
	}
    }


    /* Oh well... */
    prt_note(-2,"Please try again without that savefile.");

    /* No game in progress */
    turn = 0;

    /* Allow suspend again */
    signals_handle_tstp();

    /* Abort */
    quit("unusable savefile");

    /* Compiler food */
    return FALSE;
}



