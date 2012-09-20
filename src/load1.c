/* File: load1.c */

/* Purpose: support for loading pre-2.7.0 savefiles -BEN- */


#include "angband.h"


/*
 * This file handles "old" Angband savefiles (pre-2.7.0)
 *
 * All "old" savefiles lose a lot of information when imported.
 *
 * A long time ago, before the official Angband version scheme
 * started, Angband savefiles used the "Moria" version numbers,
 * so a savefile might be marked as "5.2.2".  For consistancy,
 * any savefile marked "5.2.Z" is mentally converted to "2.5.Z",
 * and any other savefile marked "5.Y.Z" is mentally changed to
 * "1.Y.Z", though I have not found any of these yet...
 *
 * This file will correctly parse all known savefiles, though
 * perhaps some pre-2.5.0 savefiles may be unknown...
 *
 * Old "MacAngband 1.0" and "MacAngband 2.0.3" savefiles need
 * "arg_stupid" so they can parse the old color codes and the
 * old space saving values.
 *
 * Old "PCAngband 1.4" savefiles need "arg_stupid" so they can
 * parse the old color codes and the old space saving values,
 * and "arg_crappy" so they can instantiate "maximize" mode.
 *
 * We attempt to prevent corrupt savefiles from inducing memory errors.
 *
 * Note that this file should not use the random number generator, the
 * object flavors, the visual attr/char mappings, or anything else which
 * is initialized *after* or *during* the "load character" function.
 */


#ifdef ALLOW_OLD_SAVEFILES



/*
 * Handle for the savefile
 */
static FILE	*fff;

/*
 * Hack -- simple encryption byte
 */
static byte	xor_byte;

/*
 * Hack -- parse old MacAngband or PC-Angband savefile
 */
static bool	arg_stupid;

/*
 * Hack -- turn on maximize mode after loading savefile
 */
static bool	arg_crappy;



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
static bool wearable_p(object_type *i_ptr)
{
    /* Valid "tval" codes */
    switch (i_ptr->tval)
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

    /* Hack */
    if (feof(fff))
    {
        v = 0;
    }

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
    int i;
    byte tmp8u;

    /* Strip the bytes */
    for (i = 0; i < n; i++) rd_byte(&tmp8u);
}


/*
 * Owner Conversion -- pre-2.7.8 to 2.7.8
 * Shop is column, Owner is Row, see "tables.c"
 */
static byte convert_owner[24] =
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
 * Mega-Hack -- convert the old "name2" fields into the new name1/name2
 * fields.  Note that the entries below must map one-to-one onto the old
 * "SN_*" defines, shown in the comments after the new values.  Also note
 * that this code relies on the fact that there are only 128 ego-items and
 * only 128 artifacts in the new system.  If this changes, the array below
 * should be modified to contain "u16b" entries instead of "byte" entries.
 */
static byte convert_old_names[] =
{
    0				/* 0 = SN_NULL */,
    EGO_RESISTANCE		/* 1 = SN_R (XXX) */,
    EGO_RESIST_ACID		/* 2 = SN_RA (XXX) */,
    EGO_RESIST_FIRE		/* 3 = SN_RF (XXX) */,
    EGO_RESIST_COLD		/* 4 = SN_RC (XXX) */,
    EGO_RESIST_ELEC		/* 5 = SN_RL (XXX) */,
    EGO_HA			/* 6 = SN_HA */,
    EGO_DF			/* 7 = SN_DF */,
    EGO_SLAY_ANIMAL		/* 8 = SN_SA */,
    EGO_SLAY_DRAGON		/* 9 = SN_SD */,
    EGO_SLAY_EVIL		/* 10 = SN_SE */,
    EGO_SLAY_UNDEAD		/* 11 = SN_SU */,
    EGO_BRAND_FIRE		/* 12 = SN_FT */,
    EGO_BRAND_COLD		/* 13 = SN_FB */,
    EGO_FREE_ACTION		/* 14 = SN_FREE_ACTION (XXX) */,
    EGO_SLAYING			/* 15 = SN_SLAYING (XXX) */,
    EGO_CLUMSINESS		/* 16 = SN_CLUMSINESS */,
    EGO_WEAKNESS		/* 17 = SN_WEAKNESS */,
    EGO_SLOW_DESCENT		/* 18 = SN_SLOW_DESCENT */,
    EGO_SPEED			/* 19 = SN_SPEED */,
    EGO_STEALTH			/* 20 = SN_STEALTH (XXX) */,
    EGO_SLOWNESS		/* 21 = SN_SLOWNESS */,
    EGO_NOISE			/* 22 = SN_NOISE */,
    EGO_ANNOYANCE		/* 23 = SN_GREAT_MASS */,
    EGO_INTELLIGENCE		/* 24 = SN_INTELLIGENCE */,
    EGO_WISDOM			/* 25 = SN_WISDOM */,
    EGO_INFRAVISION		/* 26 = SN_INFRAVISION */,
    EGO_MIGHT			/* 27 = SN_MIGHT (XXX) */,
    EGO_LORDLINESS		/* 28 = SN_LORDLINESS */,
    EGO_MAGI			/* 29 = SN_MAGI (XXX) */,
    EGO_BEAUTY			/* 30 = SN_BEAUTY */,
    EGO_SEEING			/* 31 = SN_SEEING */,
    EGO_REGENERATION		/* 32 = SN_REGENERATION */,
    EGO_STUPIDITY		/* 33 = SN_STUPIDITY */,
    EGO_NAIVETY			/* 34 = SN_DULLNESS */,
    0				/* 35 = SN_BLINDNESS */,
    0				/* 36 = SN_TIMIDNESS */,
    0				/* 37 = SN_TELEPORTATION */,
    EGO_UGLINESS		/* 38 = SN_UGLINESS */,
    EGO_PROTECTION		/* 39 = SN_PROTECTION */,
    EGO_IRRITATION		/* 40 = SN_IRRITATION */,
    EGO_VULNERABILITY		/* 41 = SN_VULNERABILITY */,
    EGO_ENVELOPING		/* 42 = SN_ENVELOPING */,
    EGO_BRAND_FIRE		/* 43 = SN_FIRE (XXX) */,
    EGO_HURT_EVIL		/* 44 = SN_SLAY_EVIL (XXX) */,
    EGO_HURT_DRAGON		/* 45 = SN_DRAGON_SLAYING (XXX) */,
    0				/* 46 = SN_EMPTY */,
    0				/* 47 = SN_LOCKED */,
    0				/* 48 = SN_POISON_NEEDLE */,
    0				/* 49 = SN_GAS_TRAP */,
    0				/* 50 = SN_EXPLOSION_DEVICE */,
    0				/* 51 = SN_SUMMONING_RUNES */,
    0				/* 52 = SN_MULTIPLE_TRAPS */,
    0				/* 53 = SN_DISARMED */,
    0				/* 54 = SN_UNLOCKED */,
    EGO_HURT_ANIMAL		/* 55 = SN_SLAY_ANIMAL (XXX) */,
    ART_GROND + MAX_E_IDX		/* 56 = SN_GROND */,
    ART_RINGIL + MAX_E_IDX	/* 57 = SN_RINGIL */,
    ART_AEGLOS + MAX_E_IDX	/* 58 = SN_AEGLOS */,
    ART_ARUNRUTH + MAX_E_IDX	/* 59 = SN_ARUNRUTH */,
    ART_MORMEGIL + MAX_E_IDX	/* 60 = SN_MORMEGIL */,
    EGO_MORGUL			/* 61 = SN_MORGUL */,
    ART_ANGRIST + MAX_E_IDX	/* 62 = SN_ANGRIST */,
    ART_GURTHANG + MAX_E_IDX	/* 63 = SN_GURTHANG */,
    ART_CALRIS + MAX_E_IDX	/* 64 = SN_CALRIS */,
    EGO_ACCURACY		/* 65 = SN_ACCURACY */,
    ART_ANDURIL + MAX_E_IDX	/* 66 = SN_ANDURIL */,
    EGO_SLAY_ORC		/* 67 = SN_SO */,
    EGO_POWER			/* 68 = SN_POWER */,
    ART_DURIN + MAX_E_IDX	/* 69 = SN_DURIN */,
    ART_AULE + MAX_E_IDX	/* 70 = SN_AULE */,
    EGO_WEST			/* 71 = SN_WEST */,
    EGO_BLESS_BLADE		/* 72 = SN_BLESS_BLADE */,
    EGO_SLAY_DEMON		/* 73 = SN_SDEM */,
    EGO_SLAY_TROLL		/* 74 = SN_ST */,
    ART_BLOODSPIKE + MAX_E_IDX	/* 75 = SN_BLOODSPIKE */,
    ART_THUNDERFIST + MAX_E_IDX	/* 76 = SN_THUNDERFIST */,
    EGO_WOUNDING		/* 77 = SN_WOUNDING */,
    ART_ORCRIST + MAX_E_IDX	/* 78 = SN_ORCRIST */,
    ART_GLAMDRING + MAX_E_IDX	/* 79 = SN_GLAMDRING */,
    ART_STING + MAX_E_IDX	/* 80 = SN_STING */,
    EGO_LITE			/* 81 = SN_LITE */,
    EGO_AGILITY			/* 82 = SN_AGILITY */,
    EGO_BACKBITING		/* 83 = SN_BACKBITING */,
    ART_DOOMCALLER + MAX_E_IDX	/* 84 = SN_DOOMCALLER */,
    EGO_SLAY_GIANT		/* 85 = SN_SG */,
    EGO_TELEPATHY		/* 86 = SN_TELEPATHY */,
    0				/* 87 = SN_DRAGONKIND */,
    0				/* 88 = SN_NENYA */,
    0				/* 89 = SN_NARYA */,
    0				/* 90 = SN_VILYA */,
    EGO_AMAN			/* 91 = SN_AMAN */,
    ART_BELEGENNON + MAX_E_IDX	/* 92 = SN_BELEGENNON */,
    ART_FEANOR + MAX_E_IDX	/* 93 = SN_FEANOR */,
    ART_ANARION + MAX_E_IDX	/* 94 = SN_ANARION */,
    ART_ISILDUR + MAX_E_IDX	/* 95 = SN_ISILDUR */,
    ART_FINGOLFIN + MAX_E_IDX	/* 96 = SN_FINGOLFIN */,
    EGO_ELVENKIND		/* 97 = SN_ELVENKIND (XXX) */,
    ART_SOULKEEPER + MAX_E_IDX	/* 98 = SN_SOULKEEPER */,
    ART_DOR + MAX_E_IDX		/* 99 = SN_DOR_LOMIN */,
    ART_MORGOTH + MAX_E_IDX	/* 100 = SN_MORGOTH */,
    ART_BELTHRONDING + MAX_E_IDX	/* 101 = SN_BELTHRONDING */,
    ART_DAL + MAX_E_IDX		/* 102 = SN_DAL */,
    ART_PAURHACH + MAX_E_IDX	/* 103 = SN_PAURHACH */,
    ART_PAURNIMMEN + MAX_E_IDX	/* 104 = SN_PAURNIMMEN */,
    ART_PAURAEGEN + MAX_E_IDX	/* 105 = SN_PAURAEGEN */,
    ART_CAMMITHRIM + MAX_E_IDX	/* 106 = SN_CAMMITHRIM */,
    ART_CAMBELEG + MAX_E_IDX	/* 107 = SN_CAMBELEG */,
    ART_HOLHENNETH + MAX_E_IDX	/* 108 = SN_HOLHENNETH */,
    ART_PAURNEN + MAX_E_IDX	/* 109 = SN_PAURNEN */,
    ART_AEGLIN + MAX_E_IDX	/* 110 = SN_AEGLIN */,
    ART_CAMLOST + MAX_E_IDX	/* 111 = SN_CAMLOST */,
    ART_NIMLOTH + MAX_E_IDX	/* 112 = SN_NIMLOTH */,
    ART_NAR + MAX_E_IDX		/* 113 = SN_NAR */,
    ART_BERUTHIEL + MAX_E_IDX	/* 114 = SN_BERUTHIEL */,
    ART_GORLIM + MAX_E_IDX	/* 115 = SN_GORLIM */,
    ART_NARTHANC + MAX_E_IDX	/* 116 = SN_NARTHANC */,
    ART_NIMTHANC + MAX_E_IDX	/* 117 = SN_NIMTHANC */,
    ART_DETHANC + MAX_E_IDX	/* 118 = SN_DETHANC */,
    ART_GILETTAR + MAX_E_IDX	/* 119 = SN_GILETTAR */,
    ART_RILIA + MAX_E_IDX	/* 120 = SN_RILIA */,
    ART_BELANGIL + MAX_E_IDX	/* 121 = SN_BELANGIL */,
    ART_BALLI + MAX_E_IDX	/* 122 = SN_BALLI */,
    ART_LOTHARANG + MAX_E_IDX	/* 123 = SN_LOTHARANG */,
    ART_FIRESTAR + MAX_E_IDX	/* 124 = SN_FIRESTAR */,
    ART_ERIRIL + MAX_E_IDX	/* 125 = SN_ERIRIL */,
    ART_CUBRAGOL + MAX_E_IDX	/* 126 = SN_CUBRAGOL */,
    ART_BARD + MAX_E_IDX	/* 127 = SN_BARD */,
    ART_COLLUIN + MAX_E_IDX	/* 128 = SN_COLLUIN */,
    ART_HOLCOLLETH + MAX_E_IDX	/* 129 = SN_HOLCOLLETH */,
    ART_TOTILA + MAX_E_IDX	/* 130 = SN_TOTILA */,
    ART_PAIN + MAX_E_IDX	/* 131 = SN_PAIN */,
    ART_ELVAGIL + MAX_E_IDX	/* 132 = SN_ELVAGIL */,
    ART_AGLARANG + MAX_E_IDX	/* 133 = SN_AGLARANG */,
    ART_ROHIRRIM + MAX_E_IDX	/* 134 = SN_ROHIRRIM */,
    ART_EORLINGAS + MAX_E_IDX	/* 135 = SN_EORLINGAS */,
    ART_BARUKKHELED + MAX_E_IDX	/* 136 = SN_BARUKKHELED */,
    ART_WRATH + MAX_E_IDX	/* 137 = SN_WRATH */,
    ART_HARADEKKET + MAX_E_IDX	/* 138 = SN_HARADEKKET */,
    ART_MUNDWINE + MAX_E_IDX	/* 139 = SN_MUNDWINE */,
    ART_GONDRICAM + MAX_E_IDX	/* 140 = SN_GONDRICAM */,
    ART_ZARCUTHRA + MAX_E_IDX	/* 141 = SN_ZARCUTHRA */,
    ART_CARETH + MAX_E_IDX	/* 142 = SN_CARETH */,
    ART_FORASGIL + MAX_E_IDX	/* 143 = SN_FORASGIL */,
    ART_CRISDURIAN + MAX_E_IDX	/* 144 = SN_CRISDURIAN */,
    ART_COLANNON + MAX_E_IDX	/* 145 = SN_COLANNON */,
    ART_HITHLOMIR + MAX_E_IDX	/* 146 = SN_HITHLOMIR */,
    ART_THALKETTOTH + MAX_E_IDX	/* 147 = SN_THALKETTOTH */,
    ART_ARVEDUI + MAX_E_IDX	/* 148 = SN_ARVEDUI */,
    ART_THRANDUIL + MAX_E_IDX	/* 149 = SN_THRANDUIL */,
    ART_THENGEL + MAX_E_IDX	/* 150 = SN_THENGEL */,
    ART_HAMMERHAND + MAX_E_IDX	/* 151 = SN_HAMMERHAND */,
    ART_CELEGORM + MAX_E_IDX	/* 152 = SN_CELEGORM */,
    ART_THROR + MAX_E_IDX	/* 153 = SN_THROR */,
    ART_MAEDHROS + MAX_E_IDX	/* 154 = SN_MAEDHROS */,
    ART_OLORIN + MAX_E_IDX	/* 155 = SN_OLORIN */,
    ART_ANGUIREL + MAX_E_IDX	/* 156 = SN_ANGUIREL */,
    ART_THORIN + MAX_E_IDX	/* 157 = SN_THORIN */,
    ART_CELEBORN + MAX_E_IDX	/* 158 = SN_CELEBORN */,
    ART_OROME + MAX_E_IDX	/* 159 = SN_OROME */,
    ART_EONWE + MAX_E_IDX	/* 160 = SN_EONWE */,
    ART_GONDOR + MAX_E_IDX	/* 161 = SN_GONDOR */,
    ART_THEODEN + MAX_E_IDX	/* 162 = SN_THEODEN */,
    ART_THINGOL + MAX_E_IDX	/* 163 = SN_THINGOL */,
    ART_THORONGIL + MAX_E_IDX	/* 164 = SN_THORONGIL */,
    ART_LUTHIEN + MAX_E_IDX	/* 165 = SN_LUTHIEN */,
    ART_TUOR + MAX_E_IDX	/* 166 = SN_TUOR */,
    ART_ULMO + MAX_E_IDX	/* 167 = SN_ULMO */,
    ART_OSONDIR + MAX_E_IDX	/* 168 = SN_OSONDIR */,
    ART_TURMIL + MAX_E_IDX	/* 169 = SN_TURMIL */,
    ART_CASPANION + MAX_E_IDX	/* 170 = SN_CASPANION */,
    ART_TIL + MAX_E_IDX		/* 171 = SN_TIL */,
    ART_DEATHWREAKER + MAX_E_IDX	/* 172 = SN_DEATHWREAKER */,
    ART_AVAVIR + MAX_E_IDX	/* 173 = SN_AVAVIR */,
    ART_TARATOL + MAX_E_IDX	/* 174 = SN_TARATOL */,
    ART_RAZORBACK + MAX_E_IDX	/* 175 = SN_RAZORBACK */,
    ART_BLADETURNER + MAX_E_IDX	/* 176 = SN_BLADETURNER */,
    0				/* 177 = SN_SHATTERED */,
    0				/* 178 = SN_BLASTED */,
    EGO_ATTACKS			/* 179 = SN_ATTACKS */,
};




/*
 * Read an old-version "item" structure
 */
static errr rd_item_old(object_type *i_ptr)
{
    byte old_ident;
    byte old_names;

    s16b old_pval;

    s32b old_cost;

    u32b f1, f2, f3;

    object_kind *k_ptr;

    char old_note[128];


    /* Hack -- wipe */
    WIPE(i_ptr, object_type);

    /* Index (translated below) */
    rd_s16b(&i_ptr->k_idx);

    /* Ego/Art index */
    rd_byte(&old_names);

    /* Inscription */
    rd_string(old_note, 128);

    /* Save the inscription */
    if (old_note[0]) i_ptr->note = quark_add(old_note);

    /* Ignore "f1", "tval", "tchar" */
    strip_bytes(6);

    /* Pval */
    rd_s16b(&old_pval);

    /* Cost */
    rd_s32b(&old_cost);

    /* Ignore "sval" */
    strip_bytes(1);

    /* Quantity */
    rd_byte(&i_ptr->number);

    /* Ignore "weight" */
    strip_bytes(2);

    /* Bonuses */
    rd_s16b(&i_ptr->to_h);
    rd_s16b(&i_ptr->to_d);

    /* Ignore "ac" */
    strip_bytes(2);

    /* Bonuses */
    rd_s16b(&i_ptr->to_a);

    /* Ignore "dd", "ds", level */
    strip_bytes(3);

    /* Special flags (see below) */
    rd_byte(&old_ident);

    /* Ignore "f2", "timeout" */
    strip_bytes(6);

    /* Old "color" data */
    if (arg_stupid) strip_bytes(1);


    /* XXX XXX XXX Note use of object indexes */
    /* Consider a simple array (size 513 or so) */

    /* Patch the object indexes */
    switch (i_ptr->k_idx)
    {
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

        /* Chests become ruined chests */
        case 338: i_ptr->k_idx = 344; break;
        case 339: i_ptr->k_idx = 344; break;
        case 340: i_ptr->k_idx = 344; break;
        case 341: i_ptr->k_idx = 344; break;
        case 342: i_ptr->k_idx = 344; break;
        case 343: i_ptr->k_idx = 344; break;

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

        /* Traps -- delete them all */
        case 459: i_ptr->k_idx = 0; break;
        case 460: i_ptr->k_idx = 0; break;
        case 461: i_ptr->k_idx = 0; break;
        case 462: i_ptr->k_idx = 0; break;
        case 463: i_ptr->k_idx = 0; break;
        case 464: i_ptr->k_idx = 0; break;
        case 465: i_ptr->k_idx = 0; break;
        case 466: i_ptr->k_idx = 0; break;
        case 467: i_ptr->k_idx = 0; break;
        case 468: i_ptr->k_idx = 0; break;
        case 469: i_ptr->k_idx = 0; break;
        case 470: i_ptr->k_idx = 0; break;
        case 471: i_ptr->k_idx = 0; break;
        case 472: i_ptr->k_idx = 0; break;
        case 473: i_ptr->k_idx = 0; break;
        case 474: i_ptr->k_idx = 0; break;
        case 475: i_ptr->k_idx = 0; break;
        case 476: i_ptr->k_idx = 0; break;

        /* Rubble */
        case 477: i_ptr->k_idx = 445; break;

        /* Mush -> Food Ration */
        case 478: i_ptr->k_idx = 21; break;

        /* Glyph of Warding */
        case 479: i_ptr->k_idx = 459; break;

        /* Nothing -- delete it */
        case 498: i_ptr->k_idx = 0; break;

        /* Ruined Chest */
        case 499: i_ptr->k_idx = 344; break;

        /* Broken object -- delete it */
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
    if (i_ptr->name1)
    {
        artifact_type *a_ptr = &a_info[i_ptr->name1];

        /* Lookup the real "kind" */
        i_ptr->k_idx = lookup_kind(a_ptr->tval, a_ptr->sval);
    }

    /* Parse the Old Special Names */
    else if (old_names)
    {
        /* Analyze the old "special name" */
        old_names = convert_old_names[old_names];

        /* It is an artifact */
        if (old_names >= MAX_E_IDX)
        {
            /* Extract the artifact index */
            i_ptr->name1 = (old_names - MAX_E_IDX);
        }

        /* It is an ego-item */
        else if (old_names > 0)
        {
            /* Extract the ego-item index */
            i_ptr->name2 = (old_names);
        }
    }


    /* Mega-Hack -- handle "dungeon objects" later */
    if ((i_ptr->k_idx >= 445) && (i_ptr->k_idx <= 479)) return (0);


    /*** Analyze the item ***/

    /* Access the item kind */
    k_ptr = &k_info[i_ptr->k_idx];

    /* Extract "tval" and "sval" */
    i_ptr->tval = k_ptr->tval;
    i_ptr->sval = k_ptr->sval;

    /* Hack -- notice "broken" items */
    if (k_ptr->cost <= 0) i_ptr->ident |= ID_BROKEN;

    /* Hack -- assume "cursed" items */
    if (k_ptr->flags3 & TR3_CURSED) i_ptr->ident |= ID_CURSED;


    /*** Hack -- repair ego-item names ***/

    /* Repair ego-item names */
    if (i_ptr->name2)
    {
        /* Hack -- fix some "ego-missiles" */
        if ((i_ptr->tval == TV_BOLT) ||
            (i_ptr->tval == TV_ARROW) ||
            (i_ptr->tval == TV_SHOT))
        {
            /* Special ego-item indexes */
            if (i_ptr->name2 == EGO_BRAND_FIRE)
            {
                i_ptr->name2 = EGO_FLAME;
            }
            else if (i_ptr->name2 == EGO_SLAYING)
            {
                i_ptr->name2 = EGO_FROST;
            }
            else if (i_ptr->name2 == EGO_SLAY_ANIMAL)
            {
                i_ptr->name2 = EGO_HURT_ANIMAL;
            }
            else if (i_ptr->name2 == EGO_SLAY_EVIL)
            {
                i_ptr->name2 = EGO_HURT_EVIL;
            }
            else if (i_ptr->name2 == EGO_SLAY_DRAGON)
            {
                i_ptr->name2 = EGO_HURT_DRAGON;
            }
        }

        /* Hack -- fix "Bows" */
        if (i_ptr->tval == TV_BOW)
        {
            /* Special ego-item indexes */
            if (i_ptr->name2 == EGO_MIGHT) i_ptr->name2 = EGO_VELOCITY;
        }

        /* Hack -- fix "Robes" */
        if (i_ptr->tval == TV_SOFT_ARMOR)
        {
            /* Special ego-item indexes */
            if (i_ptr->name2 == EGO_MAGI) i_ptr->name2 = EGO_PERMANENCE;
        }

        /* Hack -- fix "Boots" */
        if (i_ptr->tval == TV_BOOTS)
        {
            /* Special ego-item indexes */
            if (i_ptr->name2 == EGO_STEALTH)
            {
                i_ptr->name2 = EGO_QUIET;
            }
            else if (i_ptr->name2 == EGO_FREE_ACTION)
            {
                i_ptr->name2 = EGO_MOTION;
            }
        }

        /* Hack -- fix "Shields" */
        if (i_ptr->tval == TV_SHIELD)
        {
            /* Special ego-item indexes */
            if (i_ptr->name2 == EGO_RESIST_ACID)
            {
                i_ptr->name2 = EGO_ENDURE_ACID;
            }
            else if (i_ptr->name2 == EGO_RESIST_ELEC)
            {
                i_ptr->name2 = EGO_ENDURE_ELEC;
            }
            else if (i_ptr->name2 == EGO_RESIST_FIRE)
            {
                i_ptr->name2 = EGO_ENDURE_FIRE;
            }
            else if (i_ptr->name2 == EGO_RESIST_COLD)
            {
                i_ptr->name2 = EGO_ENDURE_COLD;
            }
            else if (i_ptr->name2 == EGO_RESISTANCE)
            {
                i_ptr->name2 = EGO_ENDURANCE;
            }
            else if (i_ptr->name2 == EGO_ELVENKIND)
            {
                i_ptr->name2 = EGO_ENDURANCE;
            }
        }
    }


    /*** Convert old flags ***/

    /* Convert "store-bought" to "aware" */
    if (old_ident & 0x10) k_ptr->aware = TRUE;

    /* Convert "identified" to "aware" */
    if (old_ident & 0x08) k_ptr->aware = TRUE;

    /* Convert "store-bought" to "known" */
    if (old_ident & 0x10) i_ptr->ident |= ID_KNOWN;

    /* Convert "identified" to "known" */
    if (old_ident & 0x08) i_ptr->ident |= ID_KNOWN;

    /* Convert "ID_DAMD" to "ID_SENSE" */
    if (old_ident & 0x02) i_ptr->ident |= ID_SENSE;

    /* Convert "ID_FELT" to "ID_SENSE" */
    if (old_ident & 0x01) i_ptr->ident |= ID_SENSE;


    /*** Acquire standard values ***/

    /* Acquire standard fields */
    i_ptr->ac = k_ptr->ac;
    i_ptr->dd = k_ptr->dd;
    i_ptr->ds = k_ptr->ds;

    /* Acquire standard weight */
    i_ptr->weight = k_ptr->weight;


    /*** Convert non-wearable items ***/

    /* Fix non-wearable items */
    if (!wearable_p(i_ptr))
    {
        /* Paranoia */
        i_ptr->name1 = i_ptr->name2 = 0;

        /* Assume normal bonuses */
        i_ptr->to_h = k_ptr->to_h;
        i_ptr->to_d = k_ptr->to_d;
        i_ptr->to_a = k_ptr->to_a;

        /* Acquire normal pval */
        i_ptr->pval = k_ptr->pval;

        /* Hack -- wands/staffs use "pval" for "charges" */
        if (i_ptr->tval == TV_WAND) i_ptr->pval = old_pval;
        if (i_ptr->tval == TV_STAFF) i_ptr->pval = old_pval;

        /* Hack -- Gold uses "pval" for "value" */
        if (i_ptr->tval == TV_GOLD) i_ptr->pval = old_cost;

        /* Success */
        return (0);
    }


    /*** Convert wearable items ***/

    /* Extract the flags */
    object_flags(i_ptr, &f1, &f2, &f3);

    /* Hack -- Rings/Amulets */
    if ((i_ptr->tval == TV_RING) || (i_ptr->tval == TV_AMULET))
    {
        /* Hack -- Adapt to the new "speed" code */
        if (f1 & TR1_SPEED)
        {
            /* Paranoia -- only expand small bonuses */
            if (old_pval < 3) old_pval = old_pval * 10;
        }

        /* Hack -- Adapt to the new "searching" code */
        if (f1 & TR1_SEARCH)
        {
            /* Paranoia -- only reduce large bonuses */
            old_pval = (old_pval + 4) / 5;
        }

        /* Hack -- Useful pval codes */
        if (f1 & TR1_PVAL_MASK)
        {
            /* Require a pval code */
            i_ptr->pval = (old_pval ? old_pval : 1);
        }
    }

    /* Hack -- Lites */
    else if (i_ptr->tval == TV_LITE)
    {
        /* Hack -- keep old pval */
        i_ptr->pval = old_pval;
    }

    /* Update artifacts */
    if (i_ptr->name1)
    {
        artifact_type *a_ptr = &a_info[i_ptr->name1];

        /* Acquire "broken" code */
        if (!a_ptr->cost) i_ptr->ident |= ID_BROKEN;

        /* Acquire artifact pval */
        i_ptr->pval = a_ptr->pval;

        /* Acquire artifact fields */
        i_ptr->ac = a_ptr->ac;
        i_ptr->dd = a_ptr->dd;
        i_ptr->ds = a_ptr->ds;

        /* Acquire artifact weight */
        i_ptr->weight = a_ptr->weight;

        /* Assume current "curse" */
        if (a_ptr->flags3 & TR3_CURSED) i_ptr->ident |= ID_CURSED;
    }

    /* Update ego-items */
    if (i_ptr->name2)
    {
        ego_item_type *e_ptr = &e_info[i_ptr->name2];

        /* Acquire "broken" code */
        if (!e_ptr->cost) i_ptr->ident |= ID_BROKEN;

        /* Hack -- Adapt to the new "speed" code */
        if (f1 & TR1_SPEED)
        {
            /* Paranoia -- only expand small bonuses */
            if (old_pval < 3) old_pval = old_pval * 10;
        }

        /* Hack -- Adapt to the new "searching" code */
        if (f1 & TR1_SEARCH)
        {
            /* Paranoia -- only reduce large bonuses */
            old_pval = (old_pval + 4) / 5;
        }

        /* Hack -- Useful pval codes */
        if (f1 & TR1_PVAL_MASK)
        {
            /* Require a pval code */
            i_ptr->pval = (old_pval ? old_pval : 1);
        }

        /* Assume current "curse" */
        if (e_ptr->flags3 & TR3_CURSED) i_ptr->ident |= ID_CURSED;
    }


    /* Success */
    return (0);
}


/*
 * Read the old lore
 *
 * Hack -- Assume all kills were by the player.
 */
static void rd_lore_old(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];

    /* Forget old flags */
    strip_bytes(16);

    /* Read kills and deaths */
    rd_s16b(&r_ptr->r_pkills);
    rd_s16b(&r_ptr->r_deaths);

    /* Forget old info */
    strip_bytes(10);

    /* Guess at "sights" */
    r_ptr->r_sights = MAX(r_ptr->r_pkills, r_ptr->r_deaths);

    /* XXX XXX Remember to "extract" max_num somewhere below */
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
        object_type forge;

        /* Strip the old "fixed cost" */
        strip_bytes(4);

        /* Load the item */
        rd_item_old(&forge);

        /* Forget the inscription */
        forge.note = 0;

        /* Save "valid" items */
        if (st_ptr->stock_num < STORE_INVEN_MAX)
        {
            /* Add that item to the store */
            st_ptr->stock[st_ptr->stock_num++] = forge;
        }
    }

    /* Success */
    return (0);
}



/*
 * Hack -- array of old artifact index order
 */
static byte old_art_order[] =
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
static void rd_artifacts_old()
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
static void rd_extra_old()
{
    int i;


    rd_string(player_name, 32);

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
    for (i = 0; i < 4; i++)
    {
        rd_string(history[i], 60);
    }

    /* Read the "maximum" stats */
    for (i = 0; i < 6; i++)
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
    if (older_than(2,5,7))
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
    if (!older_than(2,6,0)) strip_bytes(1);

    /* Old "missile counter */
    strip_bytes(2);

    /* Current turn */
    rd_s32b(&turn);
    if (turn < 0) turn = 0;

    /* Last turn */
    if (older_than(2,6,0))
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
static void rd_messages_old()
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
        if (buf[0] && (i <= last_msg)) message_add(buf);
    }
}






/*
 * More obsolete definitions...
 *
 * Fval definitions: various types of dungeon floors and walls
 *
 * In 2.7.3 (?), the "darkness" quality was moved into the "info" flags.
 *
 * In 2.7.6 (?), the grid flags changed completely
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
#define OLD_CAVE_LR	0x01	/* Grid is part of a room */
#define OLD_CAVE_FM	0x02	/* Grid is "field marked" */
#define OLD_CAVE_PL	0x04	/* Grid is perma-lit */
#define OLD_CAVE_TL	0x08	/* Grid is torch-lit */
#define OLD_CAVE_INIT	0x10	/* Grid has been "initialized" */
#define OLD_CAVE_SEEN	0x20	/* Grid is "being processed" */
#define OLD_CAVE_VIEW	0x40	/* Grid is currently "viewable" */
#define OLD_CAVE_XTRA	0x80	/* Grid is "easily" viewable */



/*
 * Old method
 *
 * All sorts of information is lost from pre-2.7.0 savefiles,
 * because they were not saved in a very intelligent manner.
 *
 * Where important, we attempt to recover lost information, or
 * at least to simulate the presence of such information.
 *
 * XXX XXX XXX Prevent old "terrain" objects in inventory.
 */
static errr rd_dungeon_old()
{
    int i, y, x;
    byte count;
    byte ychar, xchar;
    int total_count;
    byte tmp8u;
    u16b tmp16u;
    u16b limit;

    cave_type *c_ptr;

    byte ix[512];
    byte iy[512];


    /* Header info */
    rd_s16b(&dun_level);
    rd_s16b(&py);
    rd_s16b(&px);
    rd_s16b(&num_repro);
    rd_s16b(&cur_hgt);
    rd_s16b(&cur_wid);
    rd_s16b(&max_panel_rows);
    rd_s16b(&max_panel_cols);


    /* Paranoia */
    if ((dun_level < 0) || (dun_level > MAX_DEPTH))
    {
        note("Illegal dungeon level!");
        return (1);
    }

    /* Paranoia */
    if ((px < 0) || (px >= cur_wid) || (py < 0) || (py >= cur_hgt))
    {
        note("Illegal player location!");
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

        /* Invalid cave location */
        if ((xchar >= cur_wid) || (ychar >= cur_hgt))
        {
            note("Illegal monster location!");
            return (71);
        }

        /* Strip the index */
        if (older_than(2,6,0))
        {
            strip_bytes(1);
        }
        else
        {
            strip_bytes(2);
        }
    }


    /* Clear the object indexes */
    for (i = 0; i < 512; i++) ix[i] = iy[i] = 0;

    /* Read the object indexes */
    while (1)
    {
        /* Read location */
        rd_byte(&tmp8u);
        if (tmp8u == 0xFF) break;
        ychar = tmp8u;
        rd_byte(&xchar);

        /* Invalid cave location */
        if ((xchar >= cur_wid) || (ychar >= cur_hgt))
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
        ix[tmp16u] = xchar;
        iy[tmp16u] = ychar;
    }


    /* Read in the actual "cave" data */
    total_count = 0;
    xchar = ychar = 0;

    /* Read until done */
    while (total_count < MAX_HGT * MAX_WID)
    {
        /* Extract some RLE info */
        rd_byte(&count);
        rd_byte(&tmp8u);

        /* Apply the RLE info */
        for (i = count; i > 0; i--)
        {
            /* Invalid cave location */
            if ((xchar >= MAX_WID) || (ychar >= MAX_HGT))
            {
                note("Dungeon too large!");
                return (81);
            }

            /* Access the cave */
            c_ptr = &cave[ychar][xchar];

            /* Hack -- Clear all the flags */
            c_ptr->fdat = 0x00;

            /* Hack -- Clear all the flags */
            c_ptr->ftyp = 0x00;

            /* Extract the old "info" flags */
            if ((tmp8u >> 4) & 0x1) c_ptr->fdat |= CAVE_ROOM;
            if ((tmp8u >> 5) & 0x1) c_ptr->fdat |= CAVE_MARK;
            if ((tmp8u >> 6) & 0x1) c_ptr->fdat |= CAVE_GLOW;

            /* Hack -- process old style "light" */
            if (c_ptr->fdat & CAVE_GLOW)
            {
                c_ptr->fdat |= CAVE_MARK;
            }

            /* Mega-Hack -- light all walls */
            else if ((tmp8u & 0xF) >= 12)
            {
                c_ptr->fdat |= CAVE_GLOW;
            }

            /* Process the "floor type" */
            switch (tmp8u & 0xF)
            {
                /* Lite Room Floor */
                case 2:
                    c_ptr->fdat |= CAVE_GLOW;

                /* Dark Room Floor */
                case 1:
                    c_ptr->fdat |= CAVE_ROOM;
                    break;

                /* Lite Vault Floor */
                case 4:
                    c_ptr->fdat |= CAVE_GLOW;

                /* Dark Vault Floor */
                case 3:
                    c_ptr->fdat |= CAVE_ROOM;
                    c_ptr->fdat |= CAVE_ICKY;
                    break;

               	/* Corridor Floor */
                case 5:
                    break;

                /* Perma-wall (assume "solid") */
                case 15:
                    c_ptr->ftyp = 0x3F;
                    break;

                /* Granite wall (assume "basic") */
                case 12:
                    c_ptr->ftyp = 0x38;
                    break;

                /* Quartz vein */
                case 13:
                    c_ptr->ftyp = 0x33;
                    break;

                /* Magma vein */
                case 14:
                    c_ptr->ftyp = 0x32;
                    break;
            }

            /* Advance the cave pointers */
            xchar++;

            /* Wrap to the next line */
            if (xchar >= MAX_WID)
            {
                xchar = 0;
                ychar++;
            }
        }

        /* Increase the count */
        total_count += count;
    }


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
        int i_idx;

        object_type *i_ptr;

        object_type forge;


        /* Point at it */
        i_ptr = &forge;

        /* Read the item */
        rd_item_old(i_ptr);

        /* Save the location */
        i_ptr->iy = iy[i];
        i_ptr->ix = ix[i];


        /* Invalid cave location */
        if ((i_ptr->ix >= cur_wid) || (i_ptr->iy >= cur_hgt))
        {
            note("Illegal object location!!!");
            return (72);
        }

        /* Access grid */
        c_ptr = &cave[i_ptr->iy][i_ptr->ix];

        /* Something already there */
        if (c_ptr->i_idx) continue;

        /* Skip dead objects */
        if (!i_ptr->k_idx) continue;


        /* Hack -- convert old "dungeon" objects */
        if ((i_ptr->k_idx >= 445) && (i_ptr->k_idx <= 479))
        {
            int k = 0;

            /* Analyze the "dungeon objects" */
            switch (i_ptr->k_idx)
            {
                /* Rubble */
                case 445:
                    k = 0x31;
                    break;

                /* Open Door */
                case 446:
                    k = 0x04;
                    break;

                /* Closed Door */
                case 447:
                    k = 0x20;
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
            }

            /* Hack -- use the feature */
            c_ptr->ftyp = k;

            /* Done */
            continue;
        }


        /* Hack -- treasure in walls */
        if (i_ptr->tval == TV_GOLD)
        {
            /* Quartz + treasure */
            if (c_ptr->ftyp == 0x33)
            {
                /* Set the feature bits */
                c_ptr->ftyp = 0x37;

                /* Done */
                continue;
            }

            /* Magma + treasure */
            if (c_ptr->ftyp == 0x32)
            {
                /* Set the feature bits */
                c_ptr->ftyp = 0x36;

                /* Done */
                continue;
            }
        }


        /* Get a new record */
        i_idx = i_pop();

        /* Oops */
        if (!i_idx)
        {
            note(format("Too many (%d) objects!", i_max));
            return (152);
        }

        /* Acquire place */
        i_ptr = &i_list[i_idx];

        /* Copy the item */
        (*i_ptr) = forge;

        /* Mark the location */
        c_ptr->i_idx = i_idx;
    }


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
        int m_idx;

        monster_type *m_ptr;
        monster_race *r_ptr;

        monster_type forge;


        /* Forge */
        m_ptr = &forge;

        /* Hack -- wipe */
        WIPE(m_ptr, monster_type);

        /* Read the current hitpoints */
        rd_s16b(&m_ptr->hp);

        /* Strip max hitpoints */
        if (!older_than(2,6,0)) strip_bytes(2);

        /* Current sleep counter */
        rd_s16b(&m_ptr->csleep);

        /* Strip speed */
        strip_bytes(2);

        /* Read race */
        rd_s16b(&m_ptr->r_idx);

        /* Read location */
        rd_byte(&m_ptr->fy);
        rd_byte(&m_ptr->fx);

        /* Strip confusion, etc */
        strip_bytes(4);

        /* Fear */
        if (!older_than(2,6,0)) strip_bytes(1);

        /* Old "color" data */
        if (arg_stupid) strip_bytes(1);


        /* Invalid cave location */
        if ((m_ptr->fx >= cur_wid) || (m_ptr->fy >= cur_hgt))
        {
            note("Illegal monster location!!!");
            return (71);
        }


        /* Access grid */
        c_ptr = &cave[m_ptr->fy][m_ptr->fx];

        /* Hack -- Ignore "double" monsters */
        if (c_ptr->m_idx) continue;

        /* Hack -- ignore "broken" monsters */
        if (m_ptr->r_idx <= 0) continue;

        /* Hack -- ignore "player ghosts" */
        if (m_ptr->r_idx >= MAX_R_IDX-1) continue;


        /* Access the race */
        r_ptr = &r_info[m_ptr->r_idx];

        /* Hack -- recalculate speed */
        m_ptr->mspeed = r_ptr->speed;

        /* Hack -- fake energy */
        m_ptr->energy = i % 100;

        /* Hack -- maximal hitpoints */
        m_ptr->maxhp = r_ptr->hdice * r_ptr->hside;


        /* Get a new record */
        m_idx = m_pop();

        /* Oops */
        if (!m_idx)
        {
            note(format("Too many (%d) monsters!", m_max));
            return (162);
        }

        /* Acquire place */
        m_ptr = &m_list[m_idx];

        /* Copy the item */
        (*m_ptr) = forge;

        /* Mark the location */
        c_ptr->m_idx = m_idx;

        /* Count XXX XXX XXX */
        r_ptr->cur_num++;
    }


    /* Hack -- clean up terrain */
    for (y = 0; y < cur_hgt; y++)
    {
        for (x = 0; x < cur_wid; x++)
        {

            c_ptr = &cave[y][x];

            /* Hack -- convert nothing-ness into floors */
            if (c_ptr->ftyp == 0x00) c_ptr->ftyp = 0x01;
        }
    }


    /* Read the dungeon */
    character_dungeon = TRUE;


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

    /* Mega-Hack -- Extract death */
    death = (tmp32u & 0x80000000) ? TRUE : FALSE;

    /* Hack -- Unused options */
    if (!older_than(2,6,0)) strip_bytes(12);
}


/*
 * Read the pre-2.7.0 inventory
 */
static errr rd_inventory_old()
{
    int i, n;
    int slot = 0;
    s16b ictr;
    object_type forge;

    /* No weight */
    total_weight = 0;

    /* No items */
    inven_cnt = 0;
    equip_cnt = 0;

    /* Count the items */
    rd_s16b(&ictr);

    /* Verify */
    if (ictr > INVEN_PACK)
    {
        note("Too many items in the inventory!");
        return (15);
    }

    /* Normal pack items */
    for (i = 0; i < ictr; i++)
    {
        /* Read the item */
        rd_item_old(&forge);

        /* Assume aware */
        object_aware(&forge);

        /* Get the next slot */
        n = slot++;

        /* Structure copy */
        inventory[n] = forge;

        /* Add the weight */
        total_weight += (forge.number * forge.weight);

        /* One more item */
        inven_cnt++;
    }

    /* Old "normal" equipment */
    for (i = OLD_INVEN_WIELD; i < OLD_INVEN_AUX; i++)
    {
        /* Read the item */
        rd_item_old(&forge);

        /* Assume aware */
        object_aware(&forge);

        /* Skip "empty" slots */
        if (!forge.tval) continue;

        /* Hack -- convert old slot numbers */
        n = convert_slot(i);

        /* Structure copy */
        inventory[n] = forge;

        /* Add the weight */
        total_weight += (forge.number * forge.weight);

        /* One more item */
        equip_cnt++;
    }

    /* Old "aux" item */
    for (i = OLD_INVEN_AUX; i <= OLD_INVEN_AUX; i++)
    {
        /* Read the item */
        rd_item_old(&forge);

        /* Assume aware */
        object_aware(&forge);

        /* Skip "empty" slots */
        if (!forge.tval) continue;

        /* Get the next slot */
        n = slot++;

        /* Structure copy */
        inventory[n] = forge;

        /* Add the weight */
        total_weight += (forge.number * forge.weight);

        /* One more item */
        inven_cnt++;
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


    /* XXX XXX XXX */
    if (older_than(2,5,2))
    {
        /* Allow use of old PC Angband 1.4 savefiles */
        if (get_check("Are you using an old PC Angband savefile? "))
        {
            /* Set a flag */
            arg_stupid = arg_crappy = TRUE;
        }

        /* Allow use of old MacAngband 1.0 and 2.0.3 savefiles */
        else if (get_check("Are you using an old MacAngband savefile? "))
        {
            /* Set a flag */
            arg_stupid = TRUE;
        }
    }


    /* Strip the version bytes */
    strip_bytes(4);

    /* Hack -- decrypt */
    xor_byte = sf_extra;


    /* XXX XXX Fake the system info (?) */


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
    for (i = 0; i < MAX_R_IDX; i++)
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
        if (r_ptr->flags1 & RF1_UNIQUE) r_ptr->max_num = 1;

        /* Hack -- No ghosts */
        if (i == MAX_R_IDX-1) r_ptr->max_num = 0;

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

        /* Read some info, check for sentinal */
        rd_u16b(&tmp16u);
        if (tmp16u == 0xFFFF) break;

        /* Incompatible save files */
        if (tmp16u >= MAX_R_IDX)
        {
            note(format("Too many (%u) monster races!", tmp16u));
            return (21);
        }

        /* Access the monster */
        r_ptr = &r_info[tmp16u];

        /* Extract the monster lore */
        rd_lore_old(tmp16u);

        /* Hack -- Prevent fake kills */
        if (r_ptr->flags1 & RF1_UNIQUE)
        {
            /* Hack -- Note living uniques */
            if (r_ptr->max_num != 0) r_ptr->r_pkills = 0;
        }
    }
    note("Loaded Monster Memory");


    /* Read the old options */
    rd_options_old();
    note("Loaded options");

    /* Read the extra stuff */
    rd_extra_old();
    note("Loaded extra information");


    /* XXX XXX Initialize the race/class */
    rp_ptr = &race_info[p_ptr->prace];
    cp_ptr = &class_info[p_ptr->pclass];

    /* XXX XXX Important -- Initialize the magic */
    mp_ptr = &magic_info[p_ptr->pclass];


    /* Fake some "item awareness" */
    for (i = 1; i < MAX_K_IDX; i++)
    {
        object_kind *k_ptr = &k_info[i];

        /* XXX XXX Hack -- learn about "obvious" items */
        if (k_ptr->level < p_ptr->max_dlv) k_ptr->aware = TRUE;
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

    for (i = 0; i < 64; i++)
    {
        rd_byte(&spell_order[i]);
    }

    note("Loaded spell information");


    /* Ignore old aware/tried flags */
    strip_bytes(1024);

    /* Old seeds */
    rd_u32b(&seed_flavor);
    rd_u32b(&seed_town);

    /* Paranoia */
    if (!seed_flavor && !seed_town)
    {
        note("Illegal random seeds!");
        return (1);
    }

    /* Old messages */
    rd_messages_old();

    /* Some leftover info */
    rd_u16b(&panic_save);
    rd_u16b(&total_winner);
    rd_u16b(&noscore);

    /* Read the player_hp array */
    for (i = 0; i < 50; i++) rd_s16b(&player_hp[i]);

    /* Hack -- Version 2.6.2 did silly things */
    if (!older_than(2,6,2)) strip_bytes(98);

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
    rd_string(died_from, 80);

    note("Loaded all player info");


    /* Read dungeon */
    if (!death)
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
    r_info[MAX_R_IDX-1].max_num = 0;


    /* Hack -- reset morgoth XXX XXX XXX */
    r_info[MAX_R_IDX-2].max_num = 1;

    /* Hack -- reset sauron XXX XXX XXX */
    r_info[MAX_R_IDX-3].max_num = 1;


    /* Hack -- reset morgoth XXX XXX XXX */
    r_info[MAX_R_IDX-2].r_pkills = 0;

    /* Hack -- reset sauron XXX XXX XXX */
    r_info[MAX_R_IDX-3].r_pkills = 0;


    /* Add a special quest XXX XXX XXX */
    q_list[0].level = 99;

    /* Add a second quest XXX XXX XXX */
    q_list[1].level = 100;

    /* Reset third quest XXX XXX XXX */
    q_list[2].level = 0;

    /* Reset fourth quest XXX XXX XXX */
    q_list[3].level = 0;


    /* Hack -- maximize mode */
    if (arg_crappy) p_ptr->maximize = TRUE;


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

    /* The savefile is a binary file */
    fff = my_fopen(savefile, "rb");

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




