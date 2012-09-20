/* File: save-old.c */

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

static byte	version_maj;	/* Major version */
static byte	version_min;	/* Minor version */
static byte	patch_level;	/* Patch level */

static u32b	v_check = 0L;	/* A simple "checksum" on the actual values */
static u32b	x_check = 0L;	/* A simple "checksum" on the encoded bytes */

static bool	say = FALSE;	/* Show "extra" messages */



/*
 * This function determines if the version of the savefile
 * currently being read is older than version "x.y.z".
 */
static bool older_than(byte x, byte y, byte z)
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

static void rd_string(char *str)
{
    while (1) {
        byte tmp8u;
        rd_byte(&tmp8u);
        *str = tmp8u;
        if (!*str) break;
        str++;
    }
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
 * Mega-Hack -- convert the old "name2" fields into the new
 * name1/name2 fields.  Note that the entries below appear
 * in the same order as the old "SN_xxx" defines.
 */
static int convert_name2[] = {

    0				/* 0 = SN_NULL */,
    2000+EGO_RESIST		/* 1 = SN_R */,
    2000+EGO_RESIST_A		/* 2 = SN_RA */,
    2000+EGO_RESIST_F		/* 3 = SN_RF */,
    2000+EGO_RESIST_C		/* 4 = SN_RC */,
    2000+EGO_RESIST_E		/* 5 = SN_RL */,
    2000+EGO_HA			/* 6 = SN_HA */,
    2000+EGO_DF			/* 7 = SN_DF */,
    2000+EGO_SLAY_ANIMAL	/* 8 = SN_SA */,
    2000+EGO_SLAY_DRAGON	/* 9 = SN_SD */,
    2000+EGO_SLAY_EVIL		/* 10 = SN_SE */,
    2000+EGO_SLAY_UNDEAD	/* 11 = SN_SU */,
    2000+EGO_FT			/* 12 = SN_FT */,
    2000+EGO_FB			/* 13 = SN_FB */,
    2000+EGO_FREE_ACTION	/* 14 = SN_FREE_ACTION */,
    2000+EGO_SLAYING		/* 15 = SN_SLAYING (XXX or EGO_AMMO_SLAYING) */,
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
    2000+EGO_MIGHT		/* 27 = SN_MIGHT (XXX or EGO_VELOCITY) */,
    2000+EGO_LORDLINESS		/* 28 = SN_LORDLINESS */,
    2000+EGO_MAGI		/* 29 = SN_MAGI (XXX or EGO_ROBE_MAGI) */,
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
    2000+EGO_FIRE		/* 43 = SN_FIRE (XXX or EGO_AMMO_FIRE) */,
    2000+EGO_AMMO_EVIL		/* 44 = SN_SLAY_EVIL */,
    2000+EGO_AMMO_DRAGON	/* 45 = SN_DRAGON_SLAYING */,
    0				/* 46 = SN_EMPTY */,
    0				/* 47 = SN_LOCKED */,
    0				/* 48 = SN_POISON_NEEDLE */,
    0				/* 49 = SN_GAS_TRAP */,
    0				/* 50 = SN_EXPLOSION_DEVICE */,
    0				/* 51 = SN_SUMMONING_RUNES */,
    0				/* 52 = SN_MULTIPLE_TRAPS */,
    0				/* 53 = SN_DISARMED */,
    0				/* 54 = SN_UNLOCKED */,
    2000+EGO_AMMO_ANIMAL	/* 55 = SN_SLAY_ANIMAL */,
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
    2000+EGO_SLAY_ORC		/* 67 = SN_SO */,
    2000+EGO_POWER		/* 68 = SN_POWER */,
    1000+ART_DURIN		/* 69 = SN_DURIN */,
    1000+ART_AULE		/* 70 = SN_AULE */,
    2000+EGO_WEST		/* 71 = SN_WEST */,
    2000+EGO_BLESS_BLADE	/* 72 = SN_BLESS_BLADE */,
    2000+EGO_SLAY_DEMON		/* 73 = SN_SDEM */,
    2000+EGO_SLAY_TROLL		/* 74 = SN_ST */,
    1000+ART_BLOODSPIKE		/* 75 = SN_BLOODSPIKE */,
    1000+ART_THUNDERFIST	/* 76 = SN_THUNDERFIST */,
    2000+EGO_AMMO_WOUNDING	/* 77 = SN_WOUNDING */,
    1000+ART_ORCRIST		/* 78 = SN_ORCRIST */,
    1000+ART_GLAMDRING		/* 79 = SN_GLAMDRING */,
    1000+ART_STING		/* 80 = SN_STING */,
    2000+EGO_LITE		/* 81 = SN_LITE */,
    2000+EGO_AGILITY		/* 82 = SN_AGILITY */,
    2000+EGO_BACKBITING		/* 83 = SN_BACKBITING */,
    1000+ART_DOOMCALLER		/* 84 = SN_DOOMCALLER */,
    2000+EGO_SLAY_GIANT		/* 85 = SN_SG */,
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
 * Extract item flags from old savefile "flags" (see below).
 */
static void repair_item_flags_old(inven_type *i_ptr, u32b f1, u32b f2)
{
    int p = 0;


    /* Extract old "TR_SUST_STAT" flag, save it for later */
    if (f1 & 0x00400000L) p = i_ptr->pval;


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
    /* (f1 & 0x00400000L) sustain stats */
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
    /* (f2 & 0x02000000L) -- artifact flag */
    if (f2 & 0x04000000L) i_ptr->flags3 |= TR3_BLESSED;
    if (f2 & 0x08000000L) i_ptr->flags1 |= TR1_BLOWS;
    /* (f2 & 0x10000000L) -- resist fear */


    /* Sustain stats */
    if (p) {

        /* Sustain some stats */
        switch (p) {

            /* Sustain one stat */
            case 1: i_ptr->flags2 |= (TR2_SUST_STR); break;
            case 2: i_ptr->flags2 |= (TR2_SUST_INT); break;
            case 3: i_ptr->flags2 |= (TR2_SUST_WIS); break;
            case 4: i_ptr->flags2 |= (TR2_SUST_DEX); break;
            case 5: i_ptr->flags2 |= (TR2_SUST_CON); break;
            case 6: i_ptr->flags2 |= (TR2_SUST_CHR); break;

            /* Sustain all stats */
            case 10: i_ptr->flags2 |= (TR2_SUST_STR | TR2_SUST_DEX | 
                                       TR2_SUST_CON | TR2_SUST_INT | 
                                       TR2_SUST_WIS | TR2_SUST_CHR);
        }
    }
}


/*
 * Read an old-version "item" structure
 */
static errr rd_item_old(inven_type *i_ptr)
{
    u16b tmp16u;
 
    byte old_ident;
    byte old_name2;
    
    s32b old_cost;
    
    u32b f1, f2;
       
    char note[128];

    inven_kind *k_ptr;


    /* Clear it first */
    invwipe(i_ptr);

    /* Index (translated below) */
    rd_s16b(&i_ptr->k_idx);

    /* Ego/Art index */
    rd_byte(&old_name2);

    /* Inscription */
    rd_string(note);

    /* Save the inscription */
    if (note[0]) i_ptr->note = quark_add(note);

    /* Flags1 (translated below) */
    rd_u32b(&f1);

    /* Old Tval (ignored) */
    strip_bytes(1);

    /* Tchar */
    strip_bytes(1);

    /* Pval */
    rd_s16b(&i_ptr->pval);

    /* Cost */
    rd_s32b(&old_cost);

    /* Old Sval (ignored) */
    strip_bytes(1);

    /* Quantity */
    rd_byte(&i_ptr->number);

    /* Weight */
    rd_u16b(&tmp16u);
    i_ptr->weight = tmp16u;

    /* Bonuses */
    rd_s16b(&i_ptr->tohit);
    rd_s16b(&i_ptr->todam);
    rd_s16b(&i_ptr->ac);
    rd_s16b(&i_ptr->toac);

    /* Damage */
    rd_byte(&i_ptr->dd);
    rd_byte(&i_ptr->ds);

    /* Level */
    strip_bytes(1);

    /* Special flags (translated below) */
    rd_byte(&old_ident);

    /* Flags2 (translated below) */
    rd_u32b(&f2);

    /* Timeout */
    strip_bytes(2);
    

    /* Patch the object indexes */
    switch (i_ptr->k_idx) {

        /* Mushroom of poison */
        case 0: i_ptr->k_idx = 15; break;

        /* Mushrooms of restoring (extra) */
        case 13: i_ptr->k_idx = 12; break;
        case 14: i_ptr->k_idx = 12; break;

        /* Hairy Mold of poison */
        case 16: i_ptr->k_idx = 15; break;
        
        /* Ration of Food (extra) */
        case 22: i_ptr->k_idx = 21; break;
        case 23: i_ptr->k_idx = 21; break;

        /* Piece of Elvish Waybread (extra) */
        case 26: i_ptr->k_idx = 25; break;
        case 27: i_ptr->k_idx = 25; break;

        /* Main Gauche */
        case 28: i_ptr->k_idx = 38; break;

        /* Dagger */
        case 29: i_ptr->k_idx = 43; break;

        /* Broad-sword */
        case 35: i_ptr->k_idx = 34; break;

        /* Two-Handed Sword */
        case 38: i_ptr->k_idx = 37; break;

        /* Long sword */
        case 43: i_ptr->k_idx = 42; break;

        /* Awl-Pike */
        case 58: i_ptr->k_idx = 62; break;

        /* Lucern Hammer */
        case 62: i_ptr->k_idx = 58; break;

        /* Spike */
        case 84: i_ptr->k_idx = 345; break;

        /* Lantern */
        case 85: i_ptr->k_idx = 347; break;

        /* Torch */
        case 86: i_ptr->k_idx = 346; break;

        /* Orcish Pick */
        case 87: i_ptr->k_idx = 88; break;

        /* Dwarven Pick */
        case 88: i_ptr->k_idx = 89; break;

        /* Gnomish Shovel */
        case 89: i_ptr->k_idx = 85; break;

        /* Dwarven shovel */
        case 90: i_ptr->k_idx = 86; break;

        /* Robe */
        case 102: i_ptr->k_idx = 101; break;

        /* Arrow */
        case 144: i_ptr->k_idx = 78; break;

        /* Bolt */
        case 170: i_ptr->k_idx = 80; break;

        /* Scroll of Identify (extras) */
        case 177: i_ptr->k_idx = 176; break;
        case 178: i_ptr->k_idx = 176; break;
        case 179: i_ptr->k_idx = 176; break;

        /* Scroll of Light (extras) */
        case 182: i_ptr->k_idx = 181; break;
        case 183: i_ptr->k_idx = 181; break;
        
        /* Scroll of Rune of Protection */
        case 191: i_ptr->k_idx = 190; break;

        /* Scroll of Trap Detection (extra) */
        case 195: i_ptr->k_idx = 194; break;

        /* Rod of Trap Location (Hack!) */
        case 196: i_ptr->k_idx = 352; break;

        /* Scroll of Door/Stair Location (extra) */
        case 198: i_ptr->k_idx = 197; break;
        case 199: i_ptr->k_idx = 197; break;

        /* Scroll of *Enchant Armor* */
        case 205: i_ptr->k_idx = 214; break;

        /* Potion of Cure Light Wounds (extra) */
        case 238: i_ptr->k_idx = 237; break;
        case 239: i_ptr->k_idx = 237; break;

        /* Potion of Speed (extra) */
        case 256: i_ptr->k_idx = 249; break;

        /* Flask of oil */
        case 268: i_ptr->k_idx = 348; break;

        /* Wand of Clone Monster */
        case 284: i_ptr->k_idx = 283; break;

        /* Staff of Light */
        case 293: i_ptr->k_idx = 306; break;

        /* Staff of Door/Stair Location */
        case 299: i_ptr->k_idx = 316; break;

        /* Staff of Summoning (extra?) */
        case 306: i_ptr->k_idx = 305; break;

        /* Staff of *Destruction* */
        case 316: i_ptr->k_idx = 307; break;

        /* Staff of Darkness (extra?) */
        case 321: i_ptr->k_idx = 322; break;

        /* Junk and Skeletons */
        case 344: i_ptr->k_idx = 394; break;
        case 345: i_ptr->k_idx = 393; break;

        /* Filthy Rag */
        case 346: i_ptr->k_idx = 102; break;

        /* Junk and Skeletons */
        case 347: i_ptr->k_idx = 349; break;
        case 348: i_ptr->k_idx = 389; break;
        case 349: i_ptr->k_idx = 395; break;
        case 350: i_ptr->k_idx = 396; break;
        case 351: i_ptr->k_idx = 397; break;
        case 352: i_ptr->k_idx = 398; break;
        case 353: i_ptr->k_idx = 391; break;
        case 354: i_ptr->k_idx = 392; break;
        case 355: i_ptr->k_idx = 390; break;

        /* Scroll of *Remove Curse* */
        case 378: i_ptr->k_idx = 191; break;

        /* Blue Dragon Scale Mail */
        case 389: i_ptr->k_idx = 401; break;

        /* White Dragon Scale Mail */
        case 390: i_ptr->k_idx = 402; break;

        /* Black Dragon Scale Mail */
        case 391: i_ptr->k_idx = 400; break;

        /* Green Dragon Scale Mail */
        case 392: i_ptr->k_idx = 404; break;

        /* Red Dragon Scale Mail */
        case 393: i_ptr->k_idx = 403; break;

        /* Multi-Hued Dragon Scale Mail */
        case 394: i_ptr->k_idx = 405; break;

        /* Daggers (ancient artifacts?) */
        case 395: i_ptr->k_idx = 43; break;
        case 396: i_ptr->k_idx = 43; break;
        case 397: i_ptr->k_idx = 43; break;

        /* Short Sword */
        case 398: i_ptr->k_idx = 35; break;

        /* Potion of *Enlightenment* */
        case 399: i_ptr->k_idx = 422; break;

        /* Potion of Detonations */
        case 400: i_ptr->k_idx = 417; break;

        /* Potion of Death */
        case 401: i_ptr->k_idx = 415; break;

        /* Potion of Life */
        case 402: i_ptr->k_idx = 420; break;

        /* Potion of Augmentation */
        case 403: i_ptr->k_idx = 418; break;

        /* Potion of Ruination */
        case 404: i_ptr->k_idx = 416; break;

        /* Rod of Illumination */
        case 405: i_ptr->k_idx = 355; break;

        /* Rod of Probing */
        case 406: i_ptr->k_idx = 353; break;

        /* Staff of Probing */
        case 407: i_ptr->k_idx = 321; break;

        /* Bronze Dragon Scale Mail */
        case 408: i_ptr->k_idx = 408; break;

        /* Gold Dragon Scale Mail */
        case 409: i_ptr->k_idx = 409; break;

        /* Rod of Recall */
        case 410: i_ptr->k_idx = 354; break;

        /* Cloak (extra) */
        case 411: i_ptr->k_idx = 123; break;

        /* Scroll of Acquirement */
        case 412: i_ptr->k_idx = 198; break;

        /* Scroll of *Acquirement* */
        case 413: i_ptr->k_idx = 199; break;

        /* Ring of Free Action */
        case 414: i_ptr->k_idx = 144; break;

        /* Chaos Dragon Scale Mail */
        case 415: i_ptr->k_idx = 410; break;

        /* Law Dragon Scale Mail */
        case 416: i_ptr->k_idx = 407; break;

        /* Balance Dragon Scale Mail */
        case 417: i_ptr->k_idx = 411; break;

        /* Shining Dragon Scale Mail */
        case 418: i_ptr->k_idx = 406; break;

        /* Power Dragon Scale Mail */
        case 419: i_ptr->k_idx = 412; break;

        /* Potion of Enlightenment */
        case 420: i_ptr->k_idx = 256; break;

        /* Potion of Self Knowledge */
        case 421: i_ptr->k_idx = 421; break;

        /* Postion of *Healing* */
        case 422: i_ptr->k_idx = 419; break;

        /* Food ration (extra) */
        case 423: i_ptr->k_idx = 21; break;

        /* Hard Biscuit */
        case 424: i_ptr->k_idx = 22; break;

        /* Strip of Beef Jerky */
        case 425: i_ptr->k_idx = 23; break;

        /* Pint of Fine Ale */
        case 426: i_ptr->k_idx = 26; break;

        /* Pint of Fine Wine */
        case 427: i_ptr->k_idx = 27; break;

        /* Pick */
        case 428: i_ptr->k_idx = 87; break;

        /* Shovel */
        case 429: i_ptr->k_idx = 84; break;

        /* Scrolls of Identify (store) */
        /* Scrolls of Light (store) */
        /* Scrolls of Phase Door (store) */
        /* Scrolls of Mapping (store) */
        /* Scrolls of Treasure Detection (store) */
        /* Scrolls of Object Detection (store) */
        /* Scrolls of Detect Invisible (store) */
        /* Scrolls of Blessing (store) */
        /* Scrolls of Word of Recall (store) */
        case 430: i_ptr->k_idx = 176; break;
        case 431: i_ptr->k_idx = 181; break;
        case 432: i_ptr->k_idx = 185; break;
        case 433: i_ptr->k_idx = 189; break;
        case 434: i_ptr->k_idx = 192; break;
        case 435: i_ptr->k_idx = 193; break;
        case 436: i_ptr->k_idx = 201; break;
        case 437: i_ptr->k_idx = 217; break;
        case 438: i_ptr->k_idx = 220; break;

        /* Potions of Cure Light Wounds (store) */
        /* Potions of Heroism (store) */
        /* Potions of Boldness (store) */
        /* Potions of Slow Poison (store) */
        case 439: i_ptr->k_idx = 237; break;
        case 440: i_ptr->k_idx = 257; break;
        case 441: i_ptr->k_idx = 259; break;
        case 442: i_ptr->k_idx = 264; break;

        /* Lantern (store) */
        case 443: i_ptr->k_idx = 347; break;

        /* Torches (store) */
        case 444: i_ptr->k_idx = 346; break;

        /* Flasks of Oil (store) */
        case 445: i_ptr->k_idx = 348; break;

        /* Open pit */
        case 459: i_ptr->k_idx = 460; break;

        /* Arrow Trap */
        case 460: i_ptr->k_idx = 477; break;

        /* Rubble */
        case 477: i_ptr->k_idx = 445; break;

        /* Mush -> Food Ration */
        case 478: i_ptr->k_idx = 21; break;

        /* Glyph of Warding */
        case 479: i_ptr->k_idx = 459; break;

        /* The old "nothing" object */
        case 498: i_ptr->k_idx = 0; break;

        /* Ruined Chest */
        case 499: i_ptr->k_idx = 344; break;

        /* An old "broken" object */
        case 500: i_ptr->k_idx = 0; break;
        
        /* Hack -- Special objects (see below) */
        case 501: i_ptr->name1 = ART_NARYA; break;
        case 502: i_ptr->name1 = ART_NENYA; break;
        case 503: i_ptr->name1 = ART_VILYA; break;
        case 504: i_ptr->name1 = ART_POWER; break;
        case 505: i_ptr->name1 = ART_GALADRIEL; break;
        case 506: i_ptr->name1 = ART_INGWE; break;
        case 507: i_ptr->name1 = ART_CARLAMMAS; break;
        case 508: i_ptr->name1 = ART_ELENDIL; break;
        case 509: i_ptr->name1 = ART_THRAIN; break;
        case 510: i_ptr->name1 = ART_TULKAS; break;
        case 511: i_ptr->name1 = ART_DWARVES; break;
        case 512: i_ptr->name1 = ART_BARAHIR; break;
    }


    /* Artifact Names Dominate Ego-Item Names */
    if (i_ptr->name1) {

        inven_very *v_ptr = &v_list[i_ptr->name1];
        
        /* Forget the "ego-name" */
        old_name2 = 0;

        /* Lookup the real "kind" */
        i_ptr->k_idx = lookup_kind(v_ptr->tval, v_ptr->sval);
    }

    /* Parse the Old Special Names */
    if (old_name2) {

        int hack;

        /* Analyze the old "special name" */
        hack = convert_name2[old_name2];

        /* It is an ego-item */
        if (hack > 2000) {

            /* Move it elsewhere in the table */
            i_ptr->name2 = (hack - 2000);
        }

        /* It is an artifact */
        else if (hack > 1000) {

            /* Move it into the artifact table */
            i_ptr->name1 = (hack - 1000);
        }
    }


    /* Access the (possibly new) item template */
    k_ptr = &k_list[i_ptr->k_idx];

    /* Hack -- repair "tval" and "sval" */
    i_ptr->tval = k_ptr->tval;
    i_ptr->sval = k_ptr->sval;

    /* Hack -- notice "broken" items */
    if (k_ptr->cost <= 0) i_ptr->ident |= ID_BROKEN;

    
    /* Chests become empty and worthless */
    if (i_ptr->tval == TV_CHEST) {

        /* Empty */
        i_ptr->pval = 0;
        
        /* Worthless */
        i_ptr->ident |= ID_BROKEN;
    }

    /* Spikes/Missiles have no "missile_ctr" */
    if ((i_ptr->tval == TV_SPIKE) || (i_ptr->tval == TV_SHOT) ||
        (i_ptr->tval == TV_ARROW) || (i_ptr->tval == TV_BOLT)) {

        /* Forget "missile_ctr" */
        i_ptr->pval = 0;
    }


    /* Bows have no "pval" */
    if (i_ptr->tval == TV_BOW) {

        /* Hack -- repair "pval" (ancient "bug") */
        i_ptr->pval = 0;

        /* XXX XXX Normal bows of extra might */
        /* Can only be detected by weird "sval" */
        /* i_ptr->flags3 |= (TR3_XTRA_MIGHT); */
    }



    /* Repair "wearable" objects */
    if (wearable_p(i_ptr)) {

        /* Completely repair old flags */
        repair_item_flags_old(i_ptr, f1, f2);

        /* Convert "ID_SHOW_HITDAM" to "TR3_SHOW_MODS" */
        if (old_ident & 0x20) i_ptr->flags3 |= TR3_SHOW_MODS;

        /* Convert "ID_NOSHOW_TYPE" to "TR3_HIDE_TYPE" */
        if (old_ident & 0x80) i_ptr->flags3 |= TR3_HIDE_TYPE;

        /* Hack -- Inherit "EASY_KNOW" from parent */
        if (k_ptr->flags3 & TR3_EASY_KNOW) i_ptr->flags3 |= TR3_EASY_KNOW;
    }



    /*** Analyze the old "ident" flags (and friends) ***/

    /* Convert "store-bought" to "aware" */
    if (old_ident & 0x10) x_list[i_ptr->k_idx].aware = TRUE;

    /* Convert "store-bought" to "known" */
    if (old_ident & 0x10) i_ptr->ident |= ID_KNOWN;
    
    /* Convert old "ID_DAMD" flag into new "ID_SENSE" method */
    if (old_ident & 0x02) i_ptr->ident |= ID_SENSE;

    /* Convert "ID_FELT" to "ID_SENSE" */
    if (old_ident & 0x01) i_ptr->ident |= ID_SENSE;


    /*** Non wearable items use standard values ***/

    /* Fix non-wearable items */    
    if (!wearable_p(i_ptr)) {

        /* Parse "total value" of "gold" */
        if (i_ptr->tval == TV_GOLD) i_ptr->pval = old_cost;

        /* Acquire normal flags (all zeros) */
        i_ptr->flags1 = k_ptr->flags1;
        i_ptr->flags2 = k_ptr->flags2;
        i_ptr->flags3 = k_ptr->flags3;
        
        /* Acquire normal fields (all zero) */
        i_ptr->ac = k_ptr->ac;
        i_ptr->tohit = k_ptr->tohit;
        i_ptr->todam = k_ptr->todam;
        i_ptr->toac = k_ptr->toac;

        /* Acquire normal fields */
        i_ptr->dd = k_ptr->dd;
        i_ptr->ds = k_ptr->ds;
        i_ptr->weight = k_ptr->weight;
        
        /* Success */    
        return (0);
    }
    
    
    /*** Analyze wearable items some more ***/

    /* Observe current "curse" */
    if (i_ptr->flags3 & TR3_CURSED) i_ptr->ident |= ID_CURSED;

    /* Adapt to the new "speed" code */
    if (i_ptr->flags1 & TR1_SPEED) {

        /* Paranoia -- only expand small bonuses */
        if (i_ptr->pval < 3) i_ptr->pval = i_ptr->pval * 10;
    }

    /* Adapt to the new "searching" code */
    if (i_ptr->flags1 & TR1_SEARCH) {

        /* Paranoia -- only reduce large bonuses */
        if (i_ptr->pval >= 5) i_ptr->pval = i_ptr->pval / 5;
    }


    /* Update artifacts */
    if (i_ptr->name1) {

        inven_very *v_ptr = &v_list[i_ptr->name1];
        
        /* Acquire artifact flags */
        i_ptr->flags1 = v_ptr->flags1;
        i_ptr->flags2 = v_ptr->flags2;
        i_ptr->flags3 = v_ptr->flags3;

        /* Acquire artifact pval */
        i_ptr->pval = v_ptr->pval;

        /* Acquire artifact fields */
        i_ptr->ac = v_ptr->ac;
        i_ptr->dd = v_ptr->dd;
        i_ptr->ds = v_ptr->ds;
        i_ptr->weight = v_ptr->weight;
    }

    /* Update ego-items */
    else if (i_ptr->name2) {

        /* Hack -- Ego-Items of Morgul */
        if (i_ptr->name2 == EGO_MORGUL) {

            /* Heavily cursed */
            i_ptr->flags3 |= TR3_HEAVY_CURSE;
        }

        /* Hack -- fix some "ego-missiles" */
        if ((i_ptr->tval == TV_BOLT) ||
            (i_ptr->tval == TV_ARROW) ||
            (i_ptr->tval == TV_SHOT)) {

            /* Special ego-items */
            if (i_ptr->name2 == EGO_FIRE) i_ptr->name2 = EGO_AMMO_FIRE;
            if (i_ptr->name2 == EGO_SLAYING) i_ptr->name2 = EGO_AMMO_SLAYING;
            if (i_ptr->name2 == EGO_SLAY_EVIL) i_ptr->name2 = EGO_AMMO_EVIL;
        }

        /* Hack -- fix some "ego-bows" */
        if (i_ptr->tval == TV_BOW) {

            /* Special ego-items */
            if (i_ptr->name2 == EGO_MIGHT) i_ptr->name2 = EGO_VELOCITY;
        }
        
        /* Hack -- fix "robe of the magi" */
        if ((i_ptr->tval == TV_SOFT_ARMOR) &&
            (i_ptr->sval == SV_ROBE) &&
            (i_ptr->name2 == EGO_MAGI)) {

            /* Special ego-item */
            i_ptr->name2 = EGO_ROBE_MAGI;
        }

        /* Forget useless "pval" codes */
        if (!(i_ptr->flags1 & TR1_PVAL_MASK)) i_ptr->pval = 0;
    }
    
    /* Update normal wearable items */
    else {
    
        /* Acquire standard flags */
        i_ptr->flags1 = k_ptr->flags1;
        i_ptr->flags2 = k_ptr->flags2;
        i_ptr->flags3 = k_ptr->flags3;

        /* Examine Lites */
        if (i_ptr->tval == TV_LITE) {

            /* Hack -- keep old "pval" */
        }
        
        /* Examine Rings/Amulets */
        else if ((i_ptr->tval == TV_RING) || (i_ptr->tval == TV_AMULET)) {

            /* Forget "useless" pval settings (keep others) */
            if (!(i_ptr->flags1 & TR1_PVAL_MASK)) i_ptr->pval = 0;
        }

        /* Examine non lite/amulet/ring items */
        else {

            /* Acquire standard pval (oops) */
            i_ptr->pval = k_ptr->pval;
        }

        /* Acquire standard fields */
        i_ptr->ac = k_ptr->ac;
        i_ptr->dd = k_ptr->dd;
        i_ptr->ds = k_ptr->ds;
        i_ptr->weight = k_ptr->weight;
    }


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


    /* Success */
    return (0);
}


/*
 * Read old monsters
 */
static void rd_monster_old(monster_type *m_ptr)
{
    monster_race *r_ptr;


    /* Read the current hitpoints */
    rd_s16b(&m_ptr->hp);

    /* Read or extimate max hitpoints */
    if (older_than(2,6,0)) {
        /* Hack -- see below as well */
        m_ptr->maxhp = m_ptr->hp;
    }
    else {
        /* Read the maximal hitpoints */
        rd_s16b(&m_ptr->maxhp);
    }

    /* Current sleep counter */
    rd_s16b(&m_ptr->csleep);

    /* Forget speed */
    strip_bytes(2);

    /* Read race */
    rd_s16b(&m_ptr->r_idx);

    /* Access the race */
    r_ptr = &r_list[m_ptr->r_idx];

    /* Location */
    rd_byte(&m_ptr->fy);
    rd_byte(&m_ptr->fx);

    /* Hack -- Repair broken hit-points */
    if (m_ptr->maxhp <= 0) m_ptr->maxhp = maxroll(r_ptr->hdice, r_ptr->hside);

    /* Confusion, etc */
    strip_bytes(4);

    /* Fear */
    if (!older_than(2,6,0)) {
        strip_bytes(1);
    }

    /* Hack -- recalculate speed */
    m_ptr->mspeed = r_ptr->speed;
    m_ptr->energy = rand_int(100);

    /* Hack -- kill "broken" monsters and player ghosts */
    if ((m_ptr->r_idx <= 0) || (m_ptr->r_idx >= MAX_R_IDX-1)) {

	/* Wipe the record */
        WIPE(m_ptr, monster_type);
    }
}



/*
 * Read the old lore (well, a tiny piece of it)
 */
static void rd_lore_old(monster_lore *l_ptr)
{
    /* Forget old info */
    strip_bytes(16);

    /* Read kills and deaths */
    rd_s16b(&l_ptr->tkills);
    rd_s16b(&l_ptr->deaths);

    /* Forget old info */
    strip_bytes(10);

    /* Guess at "sights" */
    l_ptr->sights = l_ptr->tkills + l_ptr->deaths;
    
    /* XXX XXX Remember to "extract" max_num somewhere below */
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

    /* Extract the owner */
    st_ptr->owner = convert_owner[own];

    /* Read the items */
    for (j = 0; j < num; j++) {

        inven_type forge;
        
        /* Strip the old "fixed cost" */
        strip_bytes(4);

        /* Read the item */
        rd_item_old(&forge);

        /* Carry the item */
        store_acquire(n, &forge);
    }

    /* Success */
    return (0);
}



/*
 * Hack -- array of old artifact index order
 */
static byte old_art_order[] = {
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
static void rd_artifacts_old()
{
    int i;

    /* Read the old artifact existance flags */
    for (i = 0; old_art_order[i]; i++) {
        u32b tmp32u;
        rd_u32b(&tmp32u);
        if (tmp32u) v_list[old_art_order[i]].cur_num = 1;
    }
}





/*
 * Read the "ghost" information (and discard it)
 */
static void rd_ghost_old()
{
    /* Strip stuff */
    strip_bytes(131);

    /* Strip more stuff */
    if (!older_than(2,6,0)) {
        strip_bytes(2);
    }
}



/*
 * Read the OLD extra information
 */
static void rd_extra_old()
{
    int i;
    s32b tmp32s;

    byte hack_array[12];

        
    rd_string(player_name);

    rd_byte(&p_ptr->male);
    rd_s32b(&p_ptr->au);
    rd_s32b(&p_ptr->max_exp);
    rd_s32b(&p_ptr->exp);
    rd_u16b(&p_ptr->exp_frac);
    rd_s16b(&p_ptr->age);
    rd_s16b(&p_ptr->ht);
    rd_s16b(&p_ptr->wt);
    rd_s16b(&p_ptr->lev);
    rd_s16b(&p_ptr->max_dlv);
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

    /* Read the history */
    for (i = 0; i < 4; i++) {
        rd_string(history[i]);
    }

    /* Read the "maximum" stats */
    for (i = 0; i < 6; i++) rd_s16b(&p_ptr->max_stat[i]);

    /* Read the "current" stats (assumes no endian shift!!!) */
    if (older_than(2,5,7)) {
        u16b *hack_ptr = (u16b*)(hack_array);
        for (i = 0; i < 6; i++) rd_u16b(&hack_ptr[i]);
    }

    /* Read the "current" stats (okay) */
    else {
        for (i = 0; i < 6; i++) rd_byte(&hack_array[i]);
    }

    /* Extract the "current" stats */
    for (i = 0; i < 6; i++) p_ptr->cur_stat[i] = hack_array[i];

    /* Strip "use_stat" and "mod_stat" */
    strip_bytes(24);

    /* Read some stuff (ignore some) */
    strip_bytes(4);
    rd_s16b(&p_ptr->rest);
    rd_s16b(&p_ptr->blind);
    rd_s16b(&p_ptr->paralysis);
    rd_s16b(&p_ptr->confused);
    rd_s16b(&p_ptr->food);
    strip_bytes(6);
    rd_s16b(&p_ptr->fast);
    rd_s16b(&p_ptr->slow);
    rd_s16b(&p_ptr->fear);
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
    strip_bytes(40);

    /* Oops */
    if (!older_than(2,6,0)) strip_bytes(1);

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
    int i, m = 22;
    char buf[1024];

    u16b last_msg;

    /* Hack -- old method used circular queue */
    rd_u16b(&last_msg);

    /* Minor Hack -- may lose some old messages */
    for (i = 0; i < m; i++) {

        /* Read the message */
        rd_string(buf);

        /* Save (most of) the messages */
        if (buf[0] && (i <= last_msg)) message_new(buf, -1);
    }
}






/*
 * More obsolete definitions...
 *
 * Fval definitions: various types of dungeon floors and walls
 *
 * In 2.7.3, the "darkness" quality was moved into the "info" flags.
 *
 * In 2.7.6 (?) the grid flags changed completely
 */

#define NULL_WALL	0	/* Temp value for "generate.c" */

#define ROOM_FLOOR	1	/* Floor, in a room */
#define VAULT_FLOOR	3	/* Floor, in a vault */
#define CORR_FLOOR	5	/* Floor, in a corridor */

#define MIN_WALL	8	/* Hack -- minimum "wall" fval */

#define TMP1_WALL	8
#define TMP2_WALL	9

#define GRANITE_WALL	12	/* Granite */
#define QUARTZ_WALL	13	/* Quartz */
#define MAGMA_WALL	14	/* Magma */
#define BOUNDARY_WALL	15	/* Permanent */


/*
 * Old cave "info" flags
 */
#define CAVE_LR		0x01	/* Grid is part of a room */
#define CAVE_FM		0x02	/* Grid is "field marked" */
#define CAVE_PL		0x04	/* Grid is perma-lit */
#define CAVE_TL		0x08	/* Grid is torch-lit */
#define CAVE_INIT	0x10	/* Grid has been "initialized" */
#define CAVE_SEEN	0x20	/* Grid is "being processed" */
#define CAVE_VIEW	0x40	/* Grid is currently "viewable" */
#define CAVE_XTRA	0x80	/* Grid is "easily" viewable */



/*
 * Old method
 */
static errr rd_dungeon_old()
{
    int i, y, x;
    byte count;
    byte ychar, xchar;
    int total_count;
    byte tmp8u;
    u16b tmp16u;
    cave_type *c_ptr;


    /* Header info */
    rd_s16b(&dun_level);
    rd_s16b(&py);
    rd_s16b(&px);
    strip_bytes(2);	/* mon_tot_mult */
    rd_s16b(&cur_hgt);
    rd_s16b(&cur_wid);
    rd_s16b(&max_panel_rows);
    rd_s16b(&max_panel_cols);


    /* Set the dungeon to indicate the player location */
    cave[py][px].m_idx = 1;


    /* read in the creature ptr info */
    while (1) {

        rd_byte(&tmp8u);
        if (tmp8u == 0xFF) break;

        ychar = tmp8u;
        rd_byte(&xchar);

        /* Invalid cave location */
        if (xchar >= MAX_WID || ychar >= MAX_HGT) return (71);

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
        if (xchar >= MAX_WID || ychar >= MAX_HGT) return (72);
        cave[ychar][xchar].i_idx = tmp16u;
    }


    /* Read in the actual "cave" data */
    total_count = 0;
    xchar = ychar = 0;

    /* Read until done */
    while (total_count < MAX_HGT * MAX_WID) {

        /* Extract some RLE info */
        rd_byte(&count);
        rd_byte(&tmp8u);

        /* Apply the RLE info */
        for (i = count; i > 0; i--) {

            /* Prevent over-run */
            if (ychar >= MAX_HGT) {
                note("Dungeon too big!");
                return (81);
            }

            /* Access the cave */
            c_ptr = &cave[ychar][xchar];

            /* Hack -- Clear all the flags */
            c_ptr->info = 0;

            /* Extract the old "info" flags */
            if ((tmp8u >> 4) & 0x1) c_ptr->info |= GRID_ROOM;
            if ((tmp8u >> 5) & 0x1) c_ptr->info |= GRID_MARK;
            if ((tmp8u >> 6) & 0x1) c_ptr->info |= GRID_GLOW;

            /* Hack -- process old style "light" */
            if (c_ptr->info & GRID_GLOW) {
                c_ptr->info |= GRID_MARK;
            }

            /* Mega-Hack -- light all walls */
            else if ((tmp8u & 0xF) >= 12) {
                c_ptr->info |= GRID_GLOW;
            }

            /* Process the "floor type" */
            switch (tmp8u & 0xF) {

                case 2: /* Lite Room Floor */
                    c_ptr->info |= GRID_GLOW;

                case 1:	/* Dark Room Floor */
                    c_ptr->info |= GRID_ROOM;
                    break;

                case 4: /* Lite Vault Floor */
                    c_ptr->info |= GRID_GLOW;

                case 3: /* Dark Vault Floor */
                    c_ptr->info |= GRID_ROOM;
                    c_ptr->info |= GRID_ICKY;
                    break;

                case 5:	/* Corridor Floor */
                    break;

                case 15:	/* Perma-wall */
                    c_ptr->info |= GRID_PERM;

                case 12:	/* Granite */
                    c_ptr->info |= GRID_WALL_GRANITE;
                    break;

                case 13:	/* Quartz */
                    c_ptr->info |= GRID_WALL_QUARTZ;
                    break;

                case 14:	/* Magma */
                    c_ptr->info |= GRID_WALL_MAGMA;
                    break;
            }

            /* Advance the cave pointers */
            xchar++;

            /* Wrap to the next line */
            if (xchar >= MAX_WID) {
                xchar = 0;
                ychar++;
            }
        }

        total_count += count;
    }


    /* Read the item count */
    rd_s16b(&i_max);
    if (i_max > MAX_I_IDX) {
        note("Too many objects");
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
        note("Too many monsters");
        return (93);
    }

    /* Read the monsters */
    for (i = MIN_M_IDX; i < m_max; i++) {
        monster_type *m_ptr = &m_list[i];
        rd_monster_old(m_ptr);
        if (m_ptr->r_idx) {
            cave[m_ptr->fy][m_ptr->fx].m_idx = i;
        }
    }


    /* Read the ghost info */
    rd_ghost_old();


    /* Check the objects/monsters */
    for (y = 0; y < cur_hgt; y++) {
        for (x = 0; x < cur_wid; x++) {

            cave_type *c_ptr = &cave[y][x];
            inven_type *i_ptr = &i_list[c_ptr->i_idx];
            monster_type *m_ptr = &m_list[c_ptr->m_idx];
            monster_lore *l_ptr = &l_list[m_ptr->r_idx];

            /* Objects -- extract location */
            if (c_ptr->i_idx) {
                i_ptr->iy = y;
                i_ptr->ix = x;
            }

            /* Monsters -- count total population */
            if (c_ptr->m_idx) {
                l_ptr->cur_num++;
            }
        }
    }


    /* Success */
    return (0);
}







/*
 * Read an old savefile
 */

static void rd_options_old()
{
    u32b tmp32u;

    /* Unused */
    strip_bytes(2);

    /* Standard options */
    rd_u32b(&tmp32u);

    /* Hack -- Extract death */
    death = (tmp32u & 0x80000000) ? TRUE : FALSE;

    /* Hack -- Extract keyset */
    rogue_like_commands = (tmp32u & 0x0020) ? TRUE : FALSE;

    /* Hack -- Unused options */
    if (!older_than(2,6,0)) strip_bytes(12);
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

        /* Hack -- "hold" old aux items */
        case OLD_INVEN_AUX: return (INVEN_WIELD - 1);
    }

    /* Default */
    return (old);
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
        note("Unable to read inventory");
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


/* 
 * Read a pre-2.7.0 savefile
 */
static errr rd_savefile_old()
{
    int i;

    u16b tmp16u;
    u32b tmp32u;


    /* XXX XXX Fake the system info (?) */


    /* Read the artifacts */
    rd_artifacts_old();
    if (say) note("Loaded Artifacts");


    /* Load the Quests */
    rd_u32b(&tmp32u);
    q_list[0].level = tmp32u ? 99 : 0;
    rd_u32b(&tmp32u);
    q_list[1].level = 100;
    rd_u32b(&tmp32u);
    q_list[2].level = 0;
    rd_u32b(&tmp32u);
    q_list[3].level = 0;
    if (say) note("Loaded Quests");


    /* Load the old "Uniques" flags */
    for (i = 0; i < MAX_R_IDX; i++) {

        monster_race *r_ptr = &r_list[i];
        monster_lore *l_ptr = &l_list[i];

        strip_bytes(4);

        /* Already true, but do it again anyway */
        l_ptr->cur_num = 0;

        /* Hack -- initialize max population */
        l_ptr->max_num = 100;
        if (r_ptr->rflags1 & RF1_UNIQUE) l_ptr->max_num = 1;

        /* Hack -- Notice dead uniques */
        rd_u32b(&tmp32u);
        if (tmp32u) l_ptr->max_num = 0;
    }
    if (say) note("Loaded Unique Beasts");


    /* Hack -- assume previous lives */
    sf_lives = 1;

    /* Load the recall memory */
    while (1) {

        /* Read some info, check for sentinal */
        rd_u16b(&tmp16u);
        if (tmp16u == 0xFFFF) break;

        /* Incompatible save files */
        if (tmp16u >= MAX_R_IDX) {
            note("Too many monsters!");
            return (21);
        }

        /* Extract the monster lore */
        rd_lore_old(&l_list[tmp16u]);

        /* Hack -- Assume no kills */
        l_list[tmp16u].pkills = 0;

        /* Hack -- Assume obvious kills */
        if (r_list[tmp16u].rflags1 & RF1_UNIQUE) {
            if (l_list[tmp16u].max_num == 0) {
                l_list[tmp16u].pkills = 1;
            }
        }
    }
    if (say) note("Loaded Monster Memory");


    /* Read the old options */
    rd_options_old();
    if (say) note("Loaded options");

    /* Read the extra stuff */
    rd_extra_old();
    if (say) note("Loaded extra information");


    /* XXX XXX Initialize the race/class */
    rp_ptr = &race_info[p_ptr->prace];
    cp_ptr = &class_info[p_ptr->pclass];

    /* XXX XXX Important -- Initialize the magic */
    mp_ptr = &magic_info[p_ptr->pclass];


    /* Fake some "item awareness" */
    for (i = 0; i < MAX_K_IDX; i++) {

        inven_kind *k_ptr = &k_list[i];
        inven_xtra *x_ptr = &x_list[i];
        
        /* XXX XXX Hack -- learn about "obvious" items */
        if (k_ptr->level < p_ptr->max_dlv) x_ptr->aware = TRUE;
    }


    /* Read the inventory */
    rd_inventory_old();


    /* Read spell info */
    rd_u32b(&spell_learned1);
    rd_u32b(&spell_worked1);
    rd_u32b(&spell_forgotten1);
    rd_u32b(&spell_learned2);
    rd_u32b(&spell_worked2);
    rd_u32b(&spell_forgotten2);

    for (i = 0; i < 64; i++) {
        rd_byte(&spell_order[i]);
    }

    if (say) note("Read spell information");


    /* Ignore old aware/tried flags */
    strip_bytes(1024);

    /* Old seeds */
    rd_u32b(&randes_seed);
    rd_u32b(&town_seed);

    /* Old messages */
    rd_messages_old();

    /* Some leftover info */
    rd_u16b(&panic_save);
    rd_u16b(&total_winner);
    rd_u16b(&noscore);

    /* Read the player_hp array */
    for (i = 0; i < 50; i++) rd_s16b(&player_hp[i]);

    /* Hack -- Version 2.6.2 did silly things */
    if (!older_than(2,6,2)) strip_bytes(100);

    if (say) note("Read some more information.");


    /* Read the stores */
    for (i = 0; i < MAX_STORES; i++) {
        if (rd_store_old(i)) {
            note("ERROR reading store");
            return (32);
        }
    }


    /* Time at which file was saved */
    rd_u32b(&sf_when);

    /* Read the cause of death, if any */
    rd_string(died_from);

    if (say) note("All player info restored");


    /* I'm not dead yet... */
    if (!death) {

        /* Dead players have no dungeon */
        note("Restoring Dungeon...");
        if (rd_dungeon_old()) {
            note("Error reading dungeon data");
            return (25);
        }

        /* Really old version -- read stores again */
        if (older_than(2,1,3)) {

            /* Read the stores (again) */
            for (i = 0; i < MAX_STORES; i++) {
                if (rd_store_old(i)) {
                    note("ERROR in STORE_INVEN_MAX");
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
 * Read an item (2.7.0 or later)
 *
 * Note that savefiles from 2.7.0 and 2.7.1 may be "slightly" broken.
 *
 * Note that "speed" changed in 2.7.3 and "searching" in 2.7.6 and
 * "discounts" in 2.7.6, all of which are handled in this function.
 *
 * Note that various "repairs" are performed by this function,
 * including some major repairs leading to 2.7.8, in which all
 * non-wearable items lost their "flags".  This repair also grabs
 * the "latest" damage dice and costs for various objects.
 */
static void rd_item(inven_type *i_ptr)
{
    byte old_ident;
    byte old_unused;
    
    s32b old_cost;
    s32b old_scost;
    
    u32b tmp32u;

    inven_kind *k_ptr;

    char note[128];

    bool invis_trap = FALSE;


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
        rd_byte(&old_ident);
        rd_byte(&i_ptr->number);
        rd_s16b(&i_ptr->weight);
        rd_s16b(&i_ptr->timeout);

        rd_s16b(&i_ptr->tohit);
        rd_s16b(&i_ptr->todam);
        rd_s16b(&i_ptr->toac);
        rd_s16b(&i_ptr->ac);
        rd_byte(&i_ptr->dd);
        rd_byte(&i_ptr->ds);

        rd_byte(&old_unused);
        rd_byte(&i_ptr->discount);

        rd_s32b(&old_cost);
        rd_s32b(&old_scost);

        i_ptr->ident = old_ident;
    }

    /* New method */
    else {

        rd_byte(&i_ptr->discount);
        rd_byte(&i_ptr->number);
        rd_s16b(&i_ptr->weight);

        rd_byte(&i_ptr->name1);
        rd_byte(&i_ptr->name2);
        rd_s16b(&i_ptr->timeout);

        rd_s16b(&i_ptr->tohit);
        rd_s16b(&i_ptr->todam);
        rd_s16b(&i_ptr->toac);
        rd_s16b(&i_ptr->ac);
        rd_byte(&i_ptr->dd);
        rd_byte(&i_ptr->ds);

        rd_u16b(&i_ptr->ident);
    }
        
    rd_u32b(&i_ptr->flags1);
    rd_u32b(&i_ptr->flags2);
    rd_u32b(&i_ptr->flags3);
    rd_u32b(&tmp32u);

    rd_string(note);

    /* Save the inscription */
    if (note[0]) i_ptr->note = quark_add(note);


    /* Obtain the "kind" template */
    k_ptr = &k_list[i_ptr->k_idx];

    /* Hack -- preserve invisible traps */
    invis_trap = (i_ptr->tval == TV_INVIS_TRAP);

    /* Obtain tval/sval from k_list */
    i_ptr->tval = k_ptr->tval;
    i_ptr->sval = k_ptr->sval;

    /* Hack -- preserve invisible traps */
    if (invis_trap) i_ptr->tval = TV_INVIS_TRAP;


    /* Only repair pre-2.7.8 savefiles */
    if (!older_than(2,7,8)) return;
    

    /* Hack -- notice "broken" items */
    if (k_ptr->cost <= 0) i_ptr->ident |= ID_BROKEN;


    /* Parse "gold" */
    if (i_ptr->tval == TV_GOLD) {

        /* Clear the discount field */
        i_ptr->discount = 0;
        
        /* Extract the value */
        i_ptr->pval = old_cost;
        
        /* Done */
        return;        
    }
    
    
    /* Hack -- Parse old "discount" information */
    if (older_than(2,7,6)) {

        /* Clear the discount field */
        i_ptr->discount = 0;
        
        /* Hack -- Extract discounts on "normal" items */
        if (old_cost && (old_cost < k_ptr->cost)) {

            /* Catch 90% discount */
            if (old_cost <= (k_ptr->cost * 10 / 100)) {
                i_ptr->discount = 90;
            }

            /* Catch 75% discount */
            else if (old_cost <= (k_ptr->cost * 25 / 100)) {
                i_ptr->discount = 75;
            }

            /* Catch 50% discount */
            else if (old_cost <= (k_ptr->cost * 50 / 100)) {
                i_ptr->discount = 50;
            }

            /* Assume 25% discount */
            else if (old_cost <= (k_ptr->cost * 75 / 100)) {
                i_ptr->discount = 25;
            }
        }
    }
    

    /* Repair non-wearable items */
    if (!wearable_p(i_ptr)) {

        /* Acquire correct flags (always zero) */
        i_ptr->flags1 = k_ptr->flags1;
        i_ptr->flags2 = k_ptr->flags2;
        i_ptr->flags3 = k_ptr->flags3;

        /* Acquire correct fields (always zero) */
        i_ptr->tohit = k_ptr->tohit;
        i_ptr->todam = k_ptr->todam;
        i_ptr->toac = k_ptr->toac;

        /* Acquire correct fields */
        i_ptr->ac = k_ptr->ac;
        i_ptr->dd = k_ptr->dd;
        i_ptr->ds = k_ptr->ds;
        i_ptr->weight = k_ptr->weight;
        
        /* All done */
        return;
    }
    

    /* Take note of current "curse" */
    if (i_ptr->flags3 & TR3_CURSED) i_ptr->ident |= ID_CURSED;


    /* Hack -- the "speed" bonuses changed in 2.7.3 */
    if (older_than(2,7,3) && (i_ptr->flags1 & TR1_SPEED)) {

        /* Paranoia -- do not allow crazy speeds */
        if (i_ptr->pval <= 2) i_ptr->pval = i_ptr->pval * 10;
    }

    /* Hack -- the "searching" bonuses changed in 2.7.6 */
    if (older_than(2,7,6)) {

        /* Reduce the "pval" bonus on "search" */
        if (i_ptr->flags1 & TR1_SEARCH) {

            /* Paranoia -- only reduce large bonuses */
            if (i_ptr->pval >= 5) i_ptr->pval = i_ptr->pval / 5;
        }
    }


    /* Artifacts */
    if (i_ptr->name1) {

        inven_very *v_ptr = &v_list[i_ptr->name1];
        
        /* Acquire new artifact flags */
        i_ptr->flags1 = v_ptr->flags1;
        i_ptr->flags2 = v_ptr->flags2;
        i_ptr->flags3 = v_ptr->flags3;

        /* Acquire new artifact "pval" */
        i_ptr->pval = v_ptr->pval;

        /* Acquire new artifact fields */
        i_ptr->ac = v_ptr->ac;
        i_ptr->dd = v_ptr->dd;
        i_ptr->ds = v_ptr->ds;
        i_ptr->weight = v_ptr->weight;
    }

    /* Ego items */
    else if (i_ptr->name2) {

        /* Hack -- fix missile labels */
        if ((i_ptr->tval == TV_BOLT) ||
            (i_ptr->tval == TV_ARROW) ||
            (i_ptr->tval == TV_SHOT)) {

            /* Fix certain ego-items */
            if (i_ptr->name2 == EGO_FIRE) i_ptr->name2 = EGO_AMMO_FIRE;
            if (i_ptr->name2 == EGO_SLAYING) i_ptr->name2 = EGO_AMMO_SLAYING;
            if (i_ptr->name2 == EGO_SLAY_EVIL) i_ptr->name2 = EGO_AMMO_EVIL;
        }

        /* Hack -- fix some "ego-bows" */
        if (i_ptr->tval == TV_BOW) {

            /* Special ego-items */
            if (i_ptr->name2 == EGO_MIGHT) i_ptr->name2 = EGO_VELOCITY;
        }
        
        /* Hack -- fix "robe of the magi" */
        if ((i_ptr->tval == TV_SOFT_ARMOR) &&
            (i_ptr->sval == SV_ROBE) &&
            (i_ptr->name2 == EGO_MAGI)) {

            /* Special ego-item */
            i_ptr->name2 = EGO_ROBE_MAGI;
        }
    }

    /* Normal items */
    else {
    
        /* Acquire standard flags */
        i_ptr->flags1 = k_ptr->flags1;
        i_ptr->flags2 = k_ptr->flags2;
        i_ptr->flags3 = k_ptr->flags3;

        /* Acquire standard fields */
        i_ptr->ac = k_ptr->ac;
        i_ptr->dd = k_ptr->dd;
        i_ptr->ds = k_ptr->ds;
        i_ptr->weight = k_ptr->weight;
    }
}




/*
 * Read a monster
 */
static void rd_monster(monster_type *m_ptr)
{
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
    rd_byte(&m_ptr->xtra);
}





/*
 * Write/Read the monster lore
 */

static void rd_lore(monster_lore *l_ptr)
{
    byte tmp8u;


    /* Broken versions */
    if (older_than(2,7,1)) {

        rd_s16b(&l_ptr->tkills);
        rd_s16b(&l_ptr->deaths);

        strip_bytes(31);

        rd_byte(&l_ptr->max_num);

        l_ptr->sights = l_ptr->tkills + l_ptr->deaths;
    }

    /* Pre-2.7.7 */
    else if (older_than(2,7,7)) {

        /* Strip old flags */
        strip_bytes(20);

        /* Kills during this life */
        rd_s16b(&l_ptr->pkills);

        /* Strip something */
        strip_bytes(2);

        /* Count observations of attacks */
        rd_byte(&l_ptr->blows[0]);
        rd_byte(&l_ptr->blows[1]);
        rd_byte(&l_ptr->blows[2]);
        rd_byte(&l_ptr->blows[3]);

        /* Count some other stuff */
        rd_byte(&l_ptr->wake);
        rd_byte(&l_ptr->ignore);

        /* Strip something */
        strip_bytes(2);

        /* Count kills by player */
        rd_s16b(&l_ptr->tkills);

        /* Count deaths of player */
        rd_s16b(&l_ptr->deaths);

        /* Read the "Racial" monster limit per level */
        rd_byte(&l_ptr->max_num);

        /* Strip something */
        strip_bytes(1);

        /* Hack -- guess at "sights" */
        l_ptr->sights = l_ptr->tkills + l_ptr->deaths;
    }

    /* Current */
    else {

        /* Count sights/deaths/kills */
        rd_s16b(&l_ptr->sights);
        rd_s16b(&l_ptr->deaths);
        rd_s16b(&l_ptr->pkills);
        rd_s16b(&l_ptr->tkills);

        /* Count wakes and ignores */
        rd_byte(&l_ptr->wake);
        rd_byte(&l_ptr->ignore);

        /* Extra stuff */
        rd_byte(&l_ptr->xtra1);
        rd_byte(&l_ptr->xtra2);

        /* Count drops */
        rd_byte(&l_ptr->drop_gold);
        rd_byte(&l_ptr->drop_item);

        /* Count spells */
        rd_byte(&l_ptr->cast_inate);
        rd_byte(&l_ptr->cast_spell);

        /* Count blows of each type */
        rd_byte(&l_ptr->blows[0]);
        rd_byte(&l_ptr->blows[1]);
        rd_byte(&l_ptr->blows[2]);
        rd_byte(&l_ptr->blows[3]);
        
        /* Memorize flags */
        rd_u32b(&l_ptr->flags1);
        rd_u32b(&l_ptr->flags2);
        rd_u32b(&l_ptr->flags3);
        rd_u32b(&l_ptr->flags4);
        rd_u32b(&l_ptr->flags5);
        rd_u32b(&l_ptr->flags6);


        /* Read the "Racial" monster limit per level */
        rd_byte(&l_ptr->max_num);

        /* Later (?) */
        rd_byte(&tmp8u);
        rd_byte(&tmp8u);
        rd_byte(&tmp8u);
    }
}





/*
 * Read the "xtra" info for objects
 */
static void rd_xtra(inven_xtra *xtra)
{
    byte tmp8u;

    rd_byte(&tmp8u);

    xtra->aware = (tmp8u & 0x01) ? TRUE: FALSE;
    xtra->tried = (tmp8u & 0x02) ? TRUE: FALSE;
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

        /* Acquire the item */
        store_acquire(n, &forge);
    }

    /* Success */
    return (0);
}





/*
 * Read options -- 2.7.6 format
 * Pre-2.7.6 savefiles get default options.
 */
static void rd_options(void)
{
    byte b;
    
    u16b c;

    u32b l;


    /* Hack -- ignore options from pre-2.7.6 */
    if (older_than(2,7,6)) {
        strip_bytes(20);
        return;
    }


    /* General options */

    rd_u32b(&l);

    rogue_like_commands =  (l & 0x00000001L) ? TRUE : FALSE;
    other_query_flag =     (l & 0x00000002L) ? TRUE : FALSE;
    carry_query_flag =     (l & 0x00000004L) ? TRUE : FALSE;
    always_throw =         (l & 0x00000008L) ? TRUE : FALSE;
    always_repeat =        (l & 0x00000010L) ? TRUE : FALSE;
    quick_messages =       (l & 0x00000020L) ? TRUE : FALSE;
    use_old_target =       (l & 0x00000040L) ? TRUE : FALSE;
    always_pickup =        (l & 0x00000080L) ? TRUE : FALSE;

    use_color =            (l & 0x00000100L) ? TRUE : FALSE;
    notice_seams =         (l & 0x00000400L) ? TRUE : FALSE;
    ring_bell =            (l & 0x00000800L) ? TRUE : FALSE;
    equippy_chars =        (l & 0x00001000L) ? TRUE : FALSE;
    unused_option =        (l & 0x00002000L) ? TRUE : FALSE;
    depth_in_feet =	   (l & 0x00004000L) ? TRUE : FALSE;
    hilite_player =	   (l & 0x00008000L) ? TRUE : FALSE;

    compress_savefile =    (l & 0x00100000L) ? TRUE : FALSE;

    view_yellow_lite =     (l & 0x01000000L) ? TRUE : FALSE;
    view_bright_lite =     (l & 0x02000000L) ? TRUE : FALSE;


    /* Disturbance options */

    rd_u32b(&l);

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

    flush_command =        (l & 0x01000000L) ? TRUE : FALSE;
    flush_disturb =        (l & 0x04000000L) ? TRUE : FALSE;
    flush_failure =        (l & 0x08000000L) ? TRUE : FALSE;

    fresh_before =         (l & 0x10000000L) ? TRUE : FALSE;
    fresh_after =          (l & 0x20000000L) ? TRUE : FALSE;
    fresh_find =           (l & 0x40000000L) ? TRUE : FALSE;


    /* Gameplay options */

    rd_u32b(&l);

    dungeon_align =		(l & 0x00000001L) ? TRUE : FALSE;
    dungeon_stair =		(l & 0x00000002L) ? TRUE : FALSE;

    view_reduce_view =		(l & 0x00000004L) ? TRUE : FALSE;
    view_reduce_lite =		(l & 0x00000008L) ? TRUE : FALSE;

    view_perma_grids =		(l & 0x00000010L) ? TRUE : FALSE;
    view_torch_grids =		(l & 0x00000020L) ? TRUE : FALSE;
    view_wall_memory =		(l & 0x00000040L) ? TRUE : FALSE;
    view_xtra_memory =		(l & 0x00000080L) ? TRUE : FALSE;

    flow_by_sound =		(l & 0x00000100L) ? TRUE : FALSE;
    flow_by_smell =		(l & 0x00000200L) ? TRUE : FALSE;

    track_follow =		(l & 0x00000400L) ? TRUE : FALSE;
    track_target =		(l & 0x00000800L) ? TRUE : FALSE;

    smart_learn =		(l & 0x00004000L) ? TRUE : FALSE;
    smart_cheat =		(l & 0x00008000L) ? TRUE : FALSE;

    no_haggle_flag =		(l & 0x00010000L) ? TRUE : FALSE;
    shuffle_owners =		(l & 0x00020000L) ? TRUE : FALSE;

    show_spell_info =		(l & 0x00040000L) ? TRUE : FALSE;
    show_health_bar =		(l & 0x00080000L) ? TRUE : FALSE;

    show_inven_weight =		(l & 0x00100000L) ? TRUE : FALSE;
    show_equip_weight =		(l & 0x00200000L) ? TRUE : FALSE;
    show_store_weight =		(l & 0x00400000L) ? TRUE : FALSE;
    plain_descriptions =	(l & 0x00800000L) ? TRUE : FALSE;

    stack_allow_items =		(l & 0x01000000L) ? TRUE : FALSE;
    stack_allow_wands =		(l & 0x02000000L) ? TRUE : FALSE;
    stack_force_notes =		(l & 0x04000000L) ? TRUE : FALSE;
    stack_force_costs =		(l & 0x08000000L) ? TRUE : FALSE;

    begin_maximize =		(l & 0x10000000L) ? TRUE : FALSE;
    begin_preserve =		(l & 0x20000000L) ? TRUE : FALSE;


    /* Special options */

    rd_u32b(&l);

    use_screen_win =		(l & 0x00000001L) ? TRUE : FALSE;
    use_recall_win =		(l & 0x00000004L) ? TRUE : FALSE;
    use_choice_win =		(l & 0x00000008L) ? TRUE : FALSE;

    recall_show_desc =		(l & 0x00010000L) ? TRUE : FALSE;
    recall_show_kill =		(l & 0x00020000L) ? TRUE : FALSE;

    choice_show_info =		(l & 0x01000000L) ? TRUE : FALSE;
    choice_show_weight =	(l & 0x02000000L) ? TRUE : FALSE;
    choice_show_spells =	(l & 0x04000000L) ? TRUE : FALSE;
    choice_show_label =		(l & 0x08000000L) ? TRUE : FALSE;


    /* Read "delay_spd" */
    rd_byte(&b);
    delay_spd = b;

    /* Read "hitpoint_warn" */
    rd_byte(&b);
    hitpoint_warn = b;


    /* Cheating options */

    rd_u16b(&c);

    if (c & 0x0002) wizard = TRUE;

    cheat_peek = (c & 0x0100) ? TRUE : FALSE;
    cheat_hear = (c & 0x0200) ? TRUE : FALSE;
    cheat_room = (c & 0x0400) ? TRUE : FALSE;
    cheat_xtra = (c & 0x0800) ? TRUE : FALSE;
    cheat_know = (c & 0x1000) ? TRUE : FALSE;
    cheat_live = (c & 0x2000) ? TRUE : FALSE;
}






/*
 * Hack -- read the ghost info
 */
static void rd_ghost()
{
    int i;
    
    monster_race *r_ptr = &r_list[MAX_R_IDX-1];
    monster_lore *l_ptr = &l_list[MAX_R_IDX-1];


    /* Pre-2.7.7 ghosts */
    if (older_than(2,7,7)) {

        /* Read the old name */
        rd_string(ghost_name);

        /* Strip old data */
        strip_bytes(52);
    }
    
    /* Newer method */
    else {

        /* Name */
        rd_string(ghost_name);

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
        rd_u32b(&r_ptr->rflags1);
        rd_u32b(&r_ptr->rflags2);
        rd_u32b(&r_ptr->rflags3);
        rd_u32b(&r_ptr->rflags4);
        rd_u32b(&r_ptr->rflags5);
        rd_u32b(&r_ptr->rflags6);

        /* Attacks */
        for (i = 0; i < 4; i++) {
            rd_byte(&r_ptr->blow[i].method);
            rd_byte(&r_ptr->blow[i].effect);
            rd_byte(&r_ptr->blow[i].d_dice);
            rd_byte(&r_ptr->blow[i].d_side);
        }
    }
    
    
    /* Hack -- set the "graphic" info */
    l_ptr->l_attr = r_ptr->r_attr;
    l_ptr->l_char = r_ptr->r_char;
}




/*
 * Read/Write the "extra" information
 */

static void rd_extra()
{
    int i;

    byte tmp8u;
        
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
    rd_s16b(&p_ptr->age);
    rd_s16b(&p_ptr->ht);
    rd_s16b(&p_ptr->wt);

    /* Read the stat info */
    for (i = 0; i < 6; i++) rd_s16b(&p_ptr->max_stat[i]);
    for (i = 0; i < 6; i++) rd_s16b(&p_ptr->cur_stat[i]);

    /* Read the "ignored" stat info */
    for (i = 0; i < 6; i++) rd_s16b(&p_ptr->mod_stat[i]);
    for (i = 0; i < 6; i++) rd_s16b(&p_ptr->use_stat[i]);

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
    rd_s16b(&p_ptr->rest);
    rd_s16b(&p_ptr->blind);
    rd_s16b(&p_ptr->paralysis);
    rd_s16b(&p_ptr->confused);
    rd_s16b(&p_ptr->food);
    rd_s16b(&p_ptr->food_digested);
    strip_bytes(2);	/* Old "protection" */
    rd_s16b(&p_ptr->energy);
    rd_s16b(&p_ptr->fast);
    rd_s16b(&p_ptr->slow);
    rd_s16b(&p_ptr->fear);
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

    /* Read the new flags */
    rd_u32b(&p_ptr->update);	/* oops */
    rd_u32b(&p_ptr->notice);
    rd_u32b(&p_ptr->redraw);	/* oops */

    /* Hack -- cancel update/redraw flags */
    p_ptr->update = p_ptr->redraw = 0L;


    /* Hack -- the two "special seeds" */
    rd_u32b(&randes_seed);
    rd_u32b(&town_seed);


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
    rd_u32b(&old_turn);

    /* Current turn */
    rd_u32b(&turn);
}




/*
 * Read the player inventory
 *
 * Note that the inventory changed from 2.7.3 to 2.7.4.  Two extra
 * pack slots were added and the equipment was rearranged.  Note
 * that these two features combine when parsing old save-files, in
 * which items from the old "aux" slot are "carried", perhaps into
 * one of the two new "inventory" slots.
 *
 * XXX XXX XXX XXX (?)
 * This function is *supposed* to automatically sort the inventory
 * into the appropriate locations.  But it does not appear to work.
 */
static errr rd_inventory()
{
    inven_type forge;

    inven_ctr = 0;
    equip_ctr = 0;
    inven_weight = 0;

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

        /* Carry items in the pack */
        if (n < INVEN_WIELD) {

            /* Just carry it */
            (void)inven_carry(&forge);
        }

        /* Wield equipment */
        else {

            /* One more item */
            equip_ctr++;

            /* Hack -- structure copy */
            inventory[n] = forge;

            /* Hack -- Add the weight */
            inven_weight += (forge.number * forge.weight);
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
    char buf[1024];

    u16b num;

    /* Hack -- old method used circular queue */
    rd_u16b(&num);

    /* Read the messages */
    for (i = 0; i < num; i++) {

        /* Read the message */
        rd_string(buf);

        /* Save the message */
        message_new(buf, -1);
    }
}



/*
 * Write/Read the actual Dungeon
 */









/*
 * New Method
 */

static errr rd_dungeon()
{
    int i;
    byte count;
    byte ychar, xchar;
    byte tmp8u;
    s16b mon_tot_mult;
    int ymax, xmax;
    int total_count;

    cave_type *c_ptr;

    inven_type *i_ptr;


    /* Header info */
    rd_s16b(&dun_level);
    rd_s16b(&mon_tot_mult);
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
            if (ychar >= ymax) {
                note("Dungeon too big!");
                return (81);
            }

            /* Access the cave */
            c_ptr = &cave[ychar][xchar];

            /* Hack -- Clear all the flags */
            c_ptr->info = 0;


            /* New method */
            if (!older_than(2,7,5)) {

                /* Restore the first eight bit-flags */
                c_ptr->info = tmp8u;

                /* Advance the cave pointers */
                xchar++;

                /* Wrap to the next line */
                if (xchar >= xmax) {
                    xchar = 0;
                    ychar++;
                }

                /* Skip the old method */
                continue;
            }


            /* Extract the old "info" flags */
            if ((tmp8u >> 4) & 0x1) c_ptr->info |= GRID_ROOM;
            if ((tmp8u >> 5) & 0x1) c_ptr->info |= GRID_MARK;
            if ((tmp8u >> 6) & 0x1) c_ptr->info |= GRID_GLOW;

            /* Hack -- process old style "light" */
            if (c_ptr->info & GRID_GLOW) {
                c_ptr->info |= GRID_MARK;
            }

            /* Mega-Hack -- light all walls */
            else if ((tmp8u & 0xF) >= 12) {
                c_ptr->info |= GRID_GLOW;
            }

            /* Process the "floor type" */
            switch (tmp8u & 0xF) {

                case 2: /* Lite Room Floor */
                    c_ptr->info |= GRID_GLOW;

                case 1:	/* Dark Room Floor */
                    c_ptr->info |= GRID_ROOM;
                    break;

                case 4: /* Lite Vault Floor */
                    c_ptr->info |= GRID_GLOW;

                case 3: /* Dark Vault Floor */
                    c_ptr->info |= GRID_ROOM;
                    c_ptr->info |= GRID_ICKY;
                    break;

                case 5:	/* Corridor Floor */
                    break;

                case 15:	/* Perma-wall */
                    c_ptr->info |= GRID_PERM;

                case 12:	/* Granite */
                    c_ptr->info |= GRID_WALL_GRANITE;
                    break;

                case 13:	/* Quartz */
                    c_ptr->info |= GRID_WALL_QUARTZ;
                    break;

                case 14:	/* Magma */
                    c_ptr->info |= GRID_WALL_MAGMA;
                    break;
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
    rd_s16b(&i_max);
    if (i_max > MAX_I_IDX) {
        note("Too many objects");
        return (92);
    }

    /* Read the dungeon items */
    for (i = 1; i < i_max; i++) {

        /* Access the item */
        i_ptr = &i_list[i];
 
        /* Read the item */
        rd_item(i_ptr);

        /* Access the item location */
        c_ptr = &cave[i_ptr->iy][i_ptr->ix];

        /* Note the item location */
        if (i_ptr->k_idx) c_ptr->i_idx = i;
    }


    /* Read the monster count */
    rd_s16b(&m_max);
    if (m_max > MAX_M_IDX) {
        note("Too many monsters");
        return (93);
    }

    /* Slightly older method */
    if (older_than(2,7,7)) {
    
        /* Read the monsters (starting at record 2) */
        for (i = 2; i < m_max; i++) {

            monster_type *m_ptr;
            monster_lore *l_ptr;

            /* Access the monster */
            m_ptr = &m_list[i];

            /* Read the monster */
            rd_monster(m_ptr);


            /* Mega-Hack -- eliminate old urchins and ghosts */
            if ((m_ptr->r_idx <= 0) || (m_ptr->r_idx == MAX_R_IDX-1)) {

                /* Just kill the monster */
                WIPE(m_ptr, monster_type);
            }


            /* Hack -- fix speed in old versions */
            if (older_than(2,7,3) && (m_ptr->r_idx)) {

                monster_race *r_ptr = &r_list[m_ptr->r_idx];

                /* Racial speed */
                m_ptr->mspeed = r_ptr->speed;

                /* Random energy */
                m_ptr->energy = rand_int(100);
            }


            /* Process real monsters */
            if (m_ptr->r_idx) {
            
                /* Access the lore */
                l_ptr = &l_list[m_ptr->r_idx];

                /* Access the location */
                c_ptr = &cave[m_ptr->fy][m_ptr->fx];

                /* Note the location */
                c_ptr->m_idx = i;

                /* Count the monsters */
                l_ptr->cur_num++;
            }
        }
    }

    /* Slightly newer method */
    else {
    
        /* Read the monsters */
        for (i = 1; i < m_max; i++) {

            monster_type *m_ptr;
            monster_lore *l_ptr;

            /* Access the monster */
            m_ptr = &m_list[i];

            /* Read the monster */
            rd_monster(m_ptr);


            /* Access the location */
            c_ptr = &cave[m_ptr->fy][m_ptr->fx];

            /* Note the location */
            if (m_ptr->r_idx) c_ptr->m_idx = i;


            /* Access the lore */
            l_ptr = &l_list[m_ptr->r_idx];

            /* Count the monsters */
            if (m_ptr->r_idx) l_ptr->cur_num++;
        }
    }


    /* Note the player location in the cave */
    cave[py][px].m_idx = 1;


    /* Success */
    return (0);
}


/*
 * Hack -- see "save-old.c"
 */


/*
 * Actually read the savefile
 *
 * XXX XXX XXX Several crashes / security problems lurk in this code,
 * wherever we read an array bound and then read array entries up to
 * that bound.  We need to do some form of "bounds" checking...
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
    rd_byte(&version_maj);
    xor_byte = 0;
    rd_byte(&version_min);
    xor_byte = 0;
    rd_byte(&patch_level);
    xor_byte = 0;
    rd_byte(&xor_byte);


    /* Handle stupidity from Angband 2.4 / 2.5 */
    if ((version_maj == 5) && (version_min == 2)) {
        version_maj = 2;
        version_min = 5;
    }


    /* Build a message */
    sprintf(buf, "Loading a %d.%d.%d savefile...",
            version_maj, version_min, patch_level);

    /* Display the message */
    note(buf);

    
    /* Clear the checksums */
    v_check = 0L;
    x_check = 0L;


    /* We cannot load savefiles from Angband 1.0 */
    if (version_maj != CUR_VERSION_MAJ) {

        note("This savefile is from a different version of Angband.");
        return (11);
    }


    /* We cannot load savefiles from newer versions of Angband */
    if ((version_min > CUR_VERSION_MIN) ||
        (version_min == CUR_VERSION_MIN && patch_level > CUR_PATCH_LEVEL)) {

        note("This savefile is from a more recent version of Angband.");
        return (12);
    }


    /* Hack -- parse really old savefiles */
    if (older_than(2,7,0)) return (rd_savefile_old());


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
    if (say) note("Loaded Option Flags");


    /* Then the "messages" */
    rd_messages();
    if (say) note("Loaded Messages");


    /* Monster Memory */
    rd_u16b(&tmp16u);

    /* Read the available records */
    for (i = 0; i < tmp16u; i++) {

        /* Read the lore */
        rd_lore(&l_list[i]);

        /* XXX XXX Hack -- repair old savefiles */
        if (older_than(2,7,6)) {

            /* Assume no kills */
            l_list[i].pkills = 0;

            /* Hack -- no previous lives */
            if (sf_lives == 0) {

                /* All kills by this life */
                l_list[i].pkills = l_list[i].tkills;
            }

            /* Hack -- handle uniques */
            if (r_list[i].rflags1 & RF1_UNIQUE) {

                /* Assume no kills */
                l_list[i].pkills = 0;

                /* Handle dead uniques */
                if (l_list[i].max_num == 0) {
                    l_list[i].pkills = 1;
                }
            }
        }
    }
    if (say) note("Loaded Monster Memory");


    /* Object Memory */
    rd_u16b(&tmp16u);
    for (i = 0; i < tmp16u; i++) rd_xtra(&x_list[i]);
    if (say) note("Loaded Object Memory");


    /* Load the Quests */
    rd_u16b(&tmp16u);
    for (i = 0; i < tmp16u; i++) {
        rd_byte(&tmp8u);
        q_list[i].level = tmp8u;
        rd_byte(&tmp8u);
        rd_byte(&tmp8u);
        rd_byte(&tmp8u);
    }
    if (say) note("Loaded Quests");


    /* Load the Artifacts */
    rd_u16b(&tmp16u);
    for (i = 0; i < tmp16u; i++) {
        rd_byte(&tmp8u);
        v_list[i].cur_num = tmp8u;
        rd_byte(&tmp8u);
        rd_byte(&tmp8u);
        rd_byte(&tmp8u);
    }
    if (say) note("Loaded Artifacts");


    /* Mega-Hack -- make sure Grond/Morgoth are available */
    if (older_than(2,7,2)) {
        v_list[ART_GROND].cur_num = 0;
        v_list[ART_MORGOTH].cur_num = 0;
    }


    /* Read the extra stuff */
    rd_extra();
    if (say) note("Loaded extra information");


    /* Read the player_hp array */
    rd_u16b(&tmp16u);
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
 * Version 2.7.0 introduced an entirely different "savefile" format
 * from older versions such as 2.6.0.
 *
 * Some information from pre-2.7.0 savefiles is "lost", such
 * as some of the "saved messages", and all "ghosts".
 *
 * Most monster memory, and player ghosts, from pre-2.7.7 savefiles
 * is invalid, and is automatically erased.
 *
 * Allow restoring a file belonging to someone else,
 * but only if we can delete it.
 * Hence first try to read without doing a chmod.
 */
bool load_player(void)
{
    int		fd = -1;
    bool	ok = FALSE;


    /* Hack -- allow "debugging" */
    int wiz = to_be_wizard;


    /* Set "say" as well */
    if (wiz) say = TRUE;


#if !defined(MACINTOSH)

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
        fkk = fopen(temp, "r");
        
        /* Oops, lock exists */
        if (fkk) {
        
            /* Close the file */
            fclose(fkk);
            
            /* Message */
            msg_print("Savefile is currently in use.");
            
            /* Nothing loaded */
            return (FALSE);
        }

        /* Create a lock file */
        fkk = fopen(temp, "w");
        
        /* Dump a line of info */
        fprintf(fkk, "Lock file for savefile '%s'\n", savefile);
        
        /* Close the lock file */
        fclose(fkk);
    }

#endif


    /* Forbid suspend */
    signals_ignore_tstp();


    /* Notify the player */
    clear_screen();

    /* First note */
    note("Restoring Character.");


    /* Open the BINARY savefile */
    fd = my_topen(savefile, O_RDONLY | O_BINARY, 0);

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
        (void)close(fd);


        /* The savefile is a binary file */
        fff = my_tfopen(savefile, "rb");

        /* Paranoia */
        if (!fff) goto error;


        /* Actually read the savefile */
        if (rd_savefile()) goto error;


#ifdef VERIFY_TIMESTAMP
        /* Allow wizards to cheat */
        if (!wiz) {

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
            if (wiz && get_check("Resurrect a dead character?")) {

                /* Revive the player */
                note("Attempting a resurrection!");

                /* Not quite dead */
                if (p_ptr->chp < 0) {
                    p_ptr->chp = 0;
                    p_ptr->chp_frac = 0;
                }

                /* Clean up food */
                p_ptr->food = PY_FOOD_FULL - 1;

                /* Cure stuff */
                cure_poison();
                cure_blindness();
                cure_confusion();
                cure_fear();

                /* Cure hallucination */
                p_ptr->image = 0;

                /* Cure cuts/stun */
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
            if (fclose(fff) < 0) ok = FALSE;
        }


        /* Normal load */
        if (ok) {

            /* A character was loaded */
            character_loaded = TRUE;

            /* Allow suspend again */
            signals_handle_tstp();

            /* Give a warning */
            if (version_min != CUR_VERSION_MIN ||
                patch_level != CUR_PATCH_LEVEL) {

                msg_format("Save file from version %d.%d.%d %s game version %d.%d.%d.",
                           version_maj, version_min, patch_level,
                           ((version_min == CUR_VERSION_MIN) ?
                            "accepted for" : "converted for"),
                           CUR_VERSION_MAJ, CUR_VERSION_MIN, CUR_PATCH_LEVEL);
            }

            /* Hack -- let the system react to new options */
            Term_xtra(TERM_XTRA_REACT, 0);

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

    /* Compiler food */
    return (FALSE);
}



