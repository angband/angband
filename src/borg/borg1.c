/* File: borg1.c */
/* Purpose: Low level stuff for the Borg -BEN- */
#include "angband.h"
#include "object/tvalsval.h"
#include "cave.h"
#ifdef ALLOW_BORG
#include "borg1.h"

/*
 * This file contains various low level variables and routines.
 */

/* Date of the last change */
char borg_engine_date[] = __DATE__;

/*
 * Borg information, ScreenSaver or continual play mode;
 */
int borg_respawn_race;
int borg_respawn_class;
int borg_respawn_str;
int borg_respawn_int;
int borg_respawn_wis;
int borg_respawn_dex;
int borg_respawn_con;
int borg_respawn_chr;
int borg_dump_level;
int borg_save_death;
bool borg_respawn_winners;
int borg_delay_factor;


/* dynamic borg stuff */
bool borg_uses_swaps;
bool borg_uses_calcs = TRUE;
bool borg_worships_damage;
bool borg_worships_speed;
bool borg_worships_hp;
bool borg_worships_mana;
bool borg_worships_ac;
bool borg_worships_gold;
bool borg_plays_risky;
bool borg_slow_optimizehome;
bool borg_scums_uniques;
bool borg_kills_uniques;
int borg_chest_fail_tolerance;
s32b borg_money_scum_amount;
char borg_money_scum_item;
int borg_money_scum_who;
int borg_money_scum_ware;
bool borg_self_scum; 	/* borg scums on his own */
bool borg_lunal_mode;  /* see borg.txt */
bool borg_self_lunal;  /* borg allowed to do this himself */
bool borg_verbose;
bool borg_munchkin_start;
bool borg_munchkin_mode;
int borg_munchkin_level;
int borg_munchkin_depth;
int borg_enchant_limit;

/* HACK... this should really be a parm into borg_prepared */
/*         I am just being lazy */
bool borg_slow_return = FALSE;

req_item **borg_required_item;
int *n_req;
power_item **borg_power_item;
int *n_pwr;
int *borg_has;
int *borg_has_on;
int *borg_artifact;
int *borg_skill;
int size_class;
int size_depth;
int size_obj;
int *formula[1000];

char *prefix_pref[] =
{
/* personal attributes */
    "_STR",
    "_INT",
    "_WIS",
    "_DEX",
    "_CON",
    "_CHR",
    "_CSTR",
    "_CINT",
    "_CWIS",
    "_CDEX",
    "_CCON",
    "_CCHR",
    "_SSTR",
    "_SINT",
    "_SWIS",
    "_SDEX",
    "_SCON",
    "_SCHR",
    "_LIGHT",
    "_CURHP",
    "_MAXHP",
    "_CURSP",
    "_MAXSP",
    "_ADJSP",
    "_SFAIL1",
    "_SFAIL2",
    "_CLEVEL",
    "_MAXCLEVEL",
    "_ESP",
    "_CURLITE",
    "_RECALL",
    "_FOOD",  /* 29 */
    "_SPEED",
    "_SDIG",
    "_FEATH",
    "_REG",
    "_SINV",
    "_INFRA",
    "_DIS",
    "_DEV",
    "_SAV",
    "_STL",
    "_SRCH",
    "_SERCHFREQ",
    "_THN",
    "_THB",
    "_THT",
    "_DIG",
    "_IFIRE",
    "_IACID",
    "_ICOLD",
    "_IELEC",
	"_IPOIS",
	"_TRFIRE",
	"_TRCOLD",
	"_TRACID",
	"_TRPOIS",
	"_TRELEC",
    "_RFIRE",
    "_RCOLD",
    "_RELEC",
    "_RACID",
    "_RPOIS",
    "_RFEAR",
    "_RLITE",
    "_RDARK",
    "_RBLIND",
    "_RCONF",
    "_RSND",
    "_RSHRD",
    "_RNXUS",
    "_RNTHR",
    "_RKAOS",
    "_RDIS",
    "_HLIFE",
    "_FRACT",
    "_SRFIRE", /* same as without S but includes swap */
    "_SRCOLD",
    "_SRELEC",
    "_SRACID",
    "_SRPOIS",
    "_SRFEAR",
    "_SRLITE",
    "_SRDARK",
    "_SRBLIND",
    "_SRCONF",
    "_SRSND",
    "_SRSHRD",
    "_SRNXUS",
    "_SRNTHR",
    "_SRKAOS",
    "_SRDIS",
    "_SHLIFE",
    "_SFRACT",

/* random extra variable */
    "_DEPTH",  /* current depth being tested */
    "_CDEPTH", /* borgs current depth */
    "_MAXDEPTH", /* recall depth */
    "_KING",    /* borg has won */

/* player state things */
    "_ISWEAK",
    "_ISHUNGRY",
    "_ISFULL",
    "_ISGORGED",
    "_ISBLIND",
    "_ISAFRAID",
    "_ISCONFUSED",
    "_ISPOISONED",
    "_ISCUT",
    "_ISSTUN",
    "_ISHEAVYSTUN",
    "_ISPARALYZED",
    "_ISIMAGE",
	"_ISFORGET",
	"_ISENCUMB",
    "_ISSTUDY",
    "_ISSEARCHING",
    "_ISFIXLEV",
    "_ISFIXEXP",
    "_ISFIXSTR",
    "_ISFIXINT",
    "_ISFIXWIS",
    "_ISFIXDEX",
    "_ISFIXCON",
    "_ISFIXCHR",
    "_ISFIXALL",

/* some combat stuff */
    "_ARMOR",
    "_TOHIT",   /* base to hit, does not include weapon */
    "_TODAM",   /* base to damage, does not include weapon */
    "_WTOHIT",  /* weapon to hit */
    "_WTODAM",  /* weapon to damage */
    "_BTOHIT",  /* bow to hit */
    "_BTODAM",  /* bow to damage */
    "_BLOWS",
    "_SHOTS",
    "_WMAXDAM", /* max damage per round with weapon (normal blow) */
                /* Assumes you can enchant to +8 if you are level 25+ */
    "_WBASEDAM",/* max damage per round with weapon (normal blow) */
                /* Assumes you have no enchantment */
    "_BMAXDAM", /* max damage per round with bow (normal hit) */
                /* Assumes you can enchant to +8 if you are level 25+ */
    "_HEAVYWEPON",
    "_HEAVYBOW",

/* curses */
    "_CRSTELE",
    "_CRSAGRV",
	"_CRSHPIMP",	/* Impaired HP recovery */
	"_CRSMPIMP",	/* Impaired MP recovery */
	"_CRSFEAR",		/* Fear curse flag */
	"_CRSFVULN",	/* Vulnerable to fire */
	"_CRSEVULN",	/* Vulnerable to elec */
	"_CRSCVULN",	/* Vulnerable to Cold */
	"_CRSAVULN",	/* Vulnerable to Acid */

/* weapon attributes */
    "_WSANIMAL",  /* WS = weapon slays */
    "_WSEVIL",
    "_WSUNDEAD",
    "_WSDEMON",
    "_WSORC",
    "_WSTROLL",
    "_WSGIANT",
    "_WSDRAGON",
    "_WKUNDEAD", /* WK = weapon kills */
    "_WKDEMON",
    "_WKDRAGON",
    "_WIMPACT",
    "_WBACID",     /* WB = Weapon Branded With */
    "_WBELEC",
    "_WBFIRE",
    "_WBCOLD",
    "_WBPOIS",


/* amounts */
	"_APHASE",
    "_ATELEPORT",  /* all sources of teleport */
    "_AESCAPE",     /* Staff, artifact (can be used when blind/conf) */
    "_FUEL",
    "_HEAL",
    "_EZHEAL",
	"_LIFE",
    "_ID",
    "_ASPEED",
    "_ASTFMAGI",  /* Amount Staff Charges */
    "_ASTFDEST",
    "_ATPORTOTHER", /* How many Teleport Other charges you got? */
    "_AMISSILES",  /* only ones for your current bow count */
    "_ACUREPOIS",
    "_ADETTRAP",
    "_ADETDOOR",
    "_ADETEVIL",
    "_AMAGICMAP",
    "_ALITE",
    "_ARECHARGE",
    "_APFE",      /* Protection from Evil */
    "_AGLYPH",    /* Rune Protection */
    "_ACCW",     /* CCW potions (just because we use it so often) */
    "_ACSW",     /* CSW potions (+ CLW if cut) */
    "_ACLW",
	"_ARESHEAT", /* potions of res heat */
    "_ARESCOLD", /* pot of res cold */
	"_ARESPOIS", /* Potions of Res Poison */
    "_ATELEPORTLVL", /* scroll of teleport level */
    "_AHWORD",      /* Holy Word prayer Legal*/
	"_ADETONATE",      /* Potion of Detonation */
	"_AMASSBAN",	/* ?Mass Banishment */
	"_ASHROOM",		/* Number of cool mushrooms */
	"_AROD1",		/* Attack rods */
	"_AROD2",		/* Attack rods */
	"_DINV",        /* See Inv Spell is Legal */
    NULL
};

/*
 * Some variables
 */

bool borg_active;       /* Actually active */
bool borg_resurrect = FALSE;    /* continous play mode */

bool borg_cancel;       /* Being cancelled */

char genocide_target;   /* identity of the poor unsuspecting soul */
int zap_slot;                  /* slot of a wand/staff---to avoid a game bug*/
bool borg_casted_glyph;        /* because we dont have a launch anymore */
int borg_stop_dlevel = -1;
int borg_stop_clevel = -1;
int borg_no_deeper = 127;
bool borg_stop_king = TRUE;
bool borg_dont_react = FALSE;
int successful_target = 0;
int sold_item_tval[10];
int sold_item_sval[10];
int sold_item_pval[10];
int sold_item_store[10];
int sold_item_store[10];
int sold_item_num = -1;
int sold_item_nxt = 0;
int bought_item_tval[10];
int bought_item_sval[10];
int bought_item_pval[10];
int bought_item_store[10];
int bought_item_num = -1;
int bought_item_nxt = 0;
int borg_numb_live_unique;
int borg_living_unique_index;
int borg_unique_depth;

/*
 * Various silly flags
 */

bool borg_flag_save = FALSE;    /* Save savefile at each level */
bool borg_flag_dump = FALSE;    /* Save savefile at each death */
bool borg_save = FALSE;        /* do a save next level */
bool borg_graphics = FALSE;    /* rr9's graphics */
bool borg_confirm_target = FALSE; /* emergency spell use */
bool borg_scumming_pots = TRUE;	/* Borg will quickly store pots in home */

/*
 * Use a simple internal random number generator
 */

bool borg_rand_quick;       /* Save system setting */

u32b borg_rand_value;       /* Save system setting */

u32b borg_rand_local;       /* Save personal setting */


/*
 * Hack -- Time variables
 */

s16b borg_t = 0L;          /* Current "time" */
s16b borg_t_morgoth = 0L;  /* Last time I saw Morgoth */
s16b need_see_inviso = 0;    /* cast this when required */
s16b borg_see_inv = 0;
bool need_shift_panel = FALSE;    /* to spot offscreens */
s16b when_shift_panel = 0L;
s16b time_this_panel = 0L;   /* Current "time" on current panel*/
bool vault_on_level;         /* Borg will search for a vault */
int unique_on_level;
bool scaryguy_on_level;     /* flee from certain guys */
bool morgoth_on_level;
bool borg_morgoth_position;
int borg_t_antisummon;		/* Timestamp when in a AS spot */
bool borg_as_position;		/* Sitting in an anti-summon corridor */
bool borg_digging;			/* used in Anti-summon corridor */


bool breeder_level = FALSE;          /* Borg will shut door */
s16b old_depth = 128;
s16b borg_respawning = 0;
s16b borg_no_retreat= 0;

/*
 * Hack -- Other time variables
 */

s16b when_call_LIGHT;        /* When we last did call light */
s16b when_wizard_LIGHT;      /* When we last did wizard light */

s16b when_detect_traps;     /* When we last detected traps */
s16b when_detect_doors;     /* When we last detected doors */
s16b when_detect_walls;     /* When we last detected walls */
s16b when_detect_evil;      /* When we last detected monsters or evil */
s16b when_last_kill_mult = 0;   /* When a multiplier was last killed */

bool my_need_alter;        /* incase i hit a wall or door */
bool my_no_alter;          /*  */
bool my_need_redraw;        /* incase i hit a wall or door */
bool borg_attempting_refresh_resist = FALSE;  /* for the Resistance spell */

/*
 * Some information
 */

s16b goal;          /* Goal type */

bool goal_rising;       /* Currently returning to town */

bool goal_leaving;      /* Currently leaving the level */

bool goal_fleeing;      /* Currently fleeing the level */

bool goal_fleeing_lunal; /* Fleeing level while in lunal Mode */
bool goal_fleeing_munchkin; /* Fleeing level while in munchkin Mode */

bool borg_fleeing_town; /* Currently fleeing the level to return to town */

bool goal_ignoring;     /* Currently ignoring monsters */

int goal_recalling;     /* Currently waiting for recall, guessing the turns left */

bool goal_less;         /* return to, but dont use, the next up stairs */

s16b borg_times_twitch; /* how often twitchy on this level */
s16b borg_escapes;      /* how often teleported on this level */

bool stair_less;        /* Use the next "up" staircase */
bool stair_more;        /* Use the next "down" staircase */

s32b borg_began;        /* When this level began */
s32b borg_time_town;    /* how long it has been since I was in town */

s16b avoidance = 0;     /* Current danger thresh-hold */

bool borg_failure;      /* Notice failure */

bool borg_simulate;     /* Simulation flag */
bool borg_attacking;        /* Simulation flag */
bool borg_offsetting;    /* offset ball attacks */

bool borg_completed;        /* Completed the level */
bool borg_on_upstairs;      /* used when leaving a level */
bool borg_on_dnstairs;      /* used when leaving a level */

bool borg_needs_searching;  /* borg will search with each step */
s16b borg_oldchp;		/* hit points last game turn */
s16b borg_oldcsp;		/* mana points last game turn */

/* defence flags */
bool borg_prot_from_evil;
bool borg_speed;
bool borg_bless;
bool borg_hero;
bool borg_berserk;
s16b borg_game_ratio;  /* the ratio of borg time to game time */
s16b borg_resistance;  /* borg is Resistant to all elements */
s16b borg_no_rest_prep; /* borg wont rest for a few turns */

bool borg_shield;
bool borg_on_glyph;    /* borg is standing on a glyph of warding */
bool borg_create_door;    /* borg is going to create doors */
bool borg_sleep_spell;
bool borg_sleep_spell_ii;
bool borg_slow_spell;  /* borg is about to cast the spell */
bool borg_confuse_spell;
bool borg_fear_mon_spell;

/*
 * Current shopping information
 */

bool borg_in_shop = FALSE;  /* True if the borg is inside of a store */
s16b goal_shop = -1;        /* Next shop to visit */
s16b goal_ware = -1;        /* Next item to buy there */
s16b goal_item = -1;        /* Next item to sell there */
int borg_food_onsale = -1;      /* Are shops selling food? */
int borg_fuel_onsale = -1;      /* Are shops selling fuel? */
bool borg_needs_quick_shopping = FALSE; /* Needs to buy without browsing all shops */
s16b borg_best_fit_item = -1;	/* Item to be worn.  Index used to note which item not to sell */
int borg_best_item = -1;  /* Attempting to wear a best fit item */


/* VERSION_STRING == "3.2.0" */
char shop_orig[28] =  "acfhjmnoqruvyz13456790ABDFGH";
char shop_rogue[28] = "abcfmnoqrtuvyz13456790ABDFGH";

byte borg_nasties_num = 7;	/* Current size of the list */
byte borg_nasties_count[7];
char borg_nasties[7] = "ZAVULWD"; /* Order of Nastiness.  Hounds < Demons < Wyrms */
byte borg_nasties_limit[7] =
{20, 20, 10, 10, 10, 10, 10};

/*
 * Location variables
 */

int w_x;            /* Current panel offset (X) */
int w_y;            /* Current panel offset (Y) */
int morgy_panel_y;
int morgy_panel_x;

int borg_target_y;
int borg_target_x;  /* Current targetted location */

int c_x;            /* Current location (X) */
int c_y;            /* Current location (Y) */

int g_x;            /* Goal location (X) */
int g_y;            /* Goal location (Y) */

/* BIG HACK! Assume only 10 cursed artifacts */
int bad_obj_x[50];  /* Dropped cursed artifact at location (X) */
int bad_obj_y[50];  /* Dropped cursed artifact at location (Y) */
int bad_obj_cnt;	/* Count marker for quantity of bad objects */

/*
 * Some estimated state variables
 */

s16b my_stat_max[6];    /* Current "maximal" stat values */
s16b my_stat_cur[6];    /* Current "natural" stat values */
s16b my_stat_use[6];    /* Current "resulting" stat values */
s16b my_stat_ind[6];    /* Current "additions" to stat values */
bool my_need_stat_check[6];  /* do I need to check my stats? */

s16b my_stat_add[6];  /* additions to stats  This will allow upgrading of */
                      /* equiptment to allow a ring of int +4 to be traded */
                      /* for a ring of int +6 even if maximized to allow a */
                      /* later swap to be better. */

s16b home_stat_add[6];

int weapon_swap;    /* location of my swap weapon */
int armour_swap;    /* my swap of armour */

/* a 3 state boolean */
/*-1 = not cursed, no help needed for it */
/* 0 = light curse, needs light remove curse spell */
/* 1 = heavy curse, needs heavy remove curse spell */
int decurse_weapon_swap;  /* my swap is great, except its cursed */
int enchant_weapon_swap_to_h;  /* my swap is great, except its cursed */
int enchant_weapon_swap_to_d;  /* my swap is great, except its cursed */
int decurse_armour_swap;  /* my swap is great, except its cursed */
int enchant_armour_swap_to_a;  /* my swap is great, except its cursed */
bool borg_wearing_cursed;

s32b weapon_swap_value;
s32b armour_swap_value;

s16b weapon_swap_digger;
byte weapon_swap_slay_animal;
byte weapon_swap_slay_evil;
byte weapon_swap_slay_undead;
byte weapon_swap_slay_demon;
byte weapon_swap_slay_orc;
byte weapon_swap_slay_troll;
byte weapon_swap_slay_giant;
byte weapon_swap_slay_dragon;
byte weapon_swap_kill_undead;
byte weapon_swap_kill_demon;
byte weapon_swap_kill_dragon;
byte weapon_swap_impact;
byte weapon_swap_brand_acid;
byte weapon_swap_brand_elec;
byte weapon_swap_brand_fire;
byte weapon_swap_brand_cold;
byte weapon_swap_brand_pois;
byte weapon_swap_see_infra;
byte weapon_swap_slow_digest;
byte weapon_swap_aggravate;
byte weapon_swap_teleport;
byte weapon_swap_regenerate;
byte weapon_swap_telepathy;
byte weapon_swap_LIGHT;
byte weapon_swap_see_invis;
byte weapon_swap_ffall;
byte weapon_swap_free_act;
byte weapon_swap_hold_life;
byte weapon_swap_immune_fire;
byte weapon_swap_immune_acid;
byte weapon_swap_immune_cold;
byte weapon_swap_immune_elec;
byte weapon_swap_resist_acid;
byte weapon_swap_resist_elec;
byte weapon_swap_resist_fire;
byte weapon_swap_resist_cold;
byte weapon_swap_resist_pois;
byte weapon_swap_resist_conf;
byte weapon_swap_resist_sound;
byte weapon_swap_resist_LIGHT;
byte weapon_swap_resist_dark;
byte weapon_swap_resist_chaos;
byte weapon_swap_resist_disen;
byte weapon_swap_resist_shard;
byte weapon_swap_resist_nexus;
byte weapon_swap_resist_blind;
byte weapon_swap_resist_neth;
byte weapon_swap_resist_fear;
byte armour_swap_slay_animal;
byte armour_swap_slay_evil;
byte armour_swap_slay_undead;
byte armour_swap_slay_demon;
byte armour_swap_slay_orc;
byte armour_swap_slay_troll;
byte armour_swap_slay_giant;
byte armour_swap_slay_dragon;
byte armour_swap_kill_undead;
byte armour_swap_kill_demon;
byte armour_swap_kill_dragon;
byte armour_swap_impact;
byte armour_swap_brand_acid;
byte armour_swap_brand_elec;
byte armour_swap_brand_fire;
byte armour_swap_brand_cold;
byte armour_swap_brand_pois;
byte armour_swap_see_infra;
byte armour_swap_slow_digest;
byte armour_swap_aggravate;
byte armour_swap_teleport;
byte armour_swap_regenerate;
byte armour_swap_telepathy;
byte armour_swap_LIGHT;
byte armour_swap_see_invis;
byte armour_swap_ffall;
byte armour_swap_free_act;
byte armour_swap_hold_life;
byte armour_swap_immune_fire;
byte armour_swap_immune_acid;
byte armour_swap_immune_cold;
byte armour_swap_immune_elec;
byte armour_swap_resist_acid;
byte armour_swap_resist_elec;
byte armour_swap_resist_fire;
byte armour_swap_resist_cold;
byte armour_swap_resist_pois;
byte armour_swap_resist_conf;
byte armour_swap_resist_sound;
byte armour_swap_resist_LIGHT;
byte armour_swap_resist_dark;
byte armour_swap_resist_chaos;
byte armour_swap_resist_disen;
byte armour_swap_resist_shard;
byte armour_swap_resist_nexus;
byte armour_swap_resist_blind;
byte armour_swap_resist_neth;
byte armour_swap_resist_fear;

byte my_ammo_tval;  /* Ammo -- "tval" */
byte my_ammo_sides; /* Ammo -- "sides" */
s16b my_ammo_power; /* Shooting multipler */

s16b my_need_enchant_to_a;  /* Need some enchantment */
s16b my_need_enchant_to_h;  /* Need some enchantment */
s16b my_need_enchant_to_d;  /* Need some enchantment */
s16b my_need_brand_weapon;  /*  actually brand bolts */
s16b my_need_id;			/* need to buy ID for an inventory item */

/*
 * Hack -- basic "power"
 */

s32b my_power;


/*
 * Various "amounts" (for the player)
 */

s16b amt_food_hical;
s16b amt_food_lowcal;

s16b amt_slow_poison;
s16b amt_cure_confusion;
s16b amt_cure_blind;

s16b amt_book[9];

s16b amt_add_stat[6];
s16b amt_inc_stat[6];  /* Stat potions */
s16b amt_fix_stat[7];  /* #7 is to fix all stats */
s16b amt_fix_exp;

s16b amt_cool_staff;   /* holiness - power staff */
s16b amt_cool_wand;	/* # of charges on Wands which can be useful for attacks */
s16b amt_enchant_to_a;
s16b amt_enchant_to_d;
s16b amt_enchant_to_h;
s16b amt_brand_weapon;  /*  brand bolts */
s16b amt_enchant_weapon;
s16b amt_enchant_armor;
s16b amt_digger;
s16b amt_ego;

/*
 * Various "amounts" (for the home)
 */

s16b num_food;
s16b num_fuel;
s16b num_mold;
s16b num_ident;
s16b num_recall;
s16b num_phase;
s16b num_escape;
s16b num_tele_staves;
s16b num_teleport;
s16b num_berserk;
s16b num_teleport_level;
s16b num_recharge;

s16b num_cure_critical;
s16b num_cure_serious;

s16b num_pot_rheat;
s16b num_pot_rcold;

s16b num_missile;

s16b num_book[9];

s16b num_fix_stat[7]; /* #7 is to fix all stats */

s16b num_fix_exp;
s16b num_mana;
s16b num_heal;
s16b num_heal_true;
s16b num_ezheal;
s16b num_ezheal_true;
s16b num_life;
s16b num_life_true;
s16b num_pfe;
s16b num_glyph;
s16b num_mass_genocide;
s16b num_speed;
s16b num_detonate;

/* Mushrooms */
s16b num_mush_second_sight;		/* esp */
s16b num_mush_fast_recovery;		/* cure stun, cut, pois, blind */
s16b num_mush_restoring;			/* Restore All */
s16b num_mush_cure_mind;			/* Cure confustion, Halluc, fear, tmp resist Conf */
s16b num_mush_emergency;			/* Hallucinate, Oppose Fire, Oppose Cold, Heal 200 */
s16b num_mush_terror;			/* Terror --give +5 speed boost */
s16b num_mush_stoneskin;			/* StoneSkin */
s16b num_mush_debility;			/* Mana Restore, temp loss of a stat (str/con) */
s16b num_mush_sprinting;			/* Sprinting (speed +10) */
s16b num_mush_purging;			/* Purging --Makes hungry, restore Str/Con, Cure Pois */

s16b num_enchant_to_a;
s16b num_enchant_to_d;
s16b num_enchant_to_h;
s16b num_brand_weapon;  /* brand bolts */
s16b num_genocide;

s16b num_artifact;
s16b num_ego;

s16b home_slot_free;
s16b home_damage;
s16b num_duplicate_items;
s16b num_slow_digest;
s16b num_regenerate;
s16b num_telepathy;
s16b num_LIGHT;
s16b num_see_inv;
s16b num_invisible;   /*  */

s16b num_ffall;
s16b num_free_act;
s16b num_hold_life;
s16b num_immune_acid;
s16b num_immune_elec;
s16b num_immune_fire;
s16b num_immune_cold;
s16b num_resist_acid;
s16b num_resist_elec;
s16b num_resist_fire;
s16b num_resist_cold;
s16b num_resist_pois;
s16b num_resist_conf;
s16b num_resist_sound;
s16b num_resist_LIGHT;
s16b num_resist_dark;
s16b num_resist_chaos;
s16b num_resist_disen;
s16b num_resist_shard;
s16b num_resist_nexus;
s16b num_resist_blind;
s16b num_resist_neth;
s16b num_sustain_str;
s16b num_sustain_int;
s16b num_sustain_wis;
s16b num_sustain_dex;
s16b num_sustain_con;
s16b num_sustain_all;

s16b num_speed;
s16b num_edged_weapon;
s16b num_bad_gloves;
s16b num_weapons;
s16b num_bow;
s16b num_rings;
s16b num_neck;
s16b num_armor;
s16b num_cloaks;
s16b num_shields;
s16b num_hats;
s16b num_gloves;
s16b num_boots;

/*
 * Hack -- extra state variables
 */

int borg_feeling = 0;   /* Current level "feeling" */

/*
 * Hack -- current shop index
 */

s16b shop_num = -1;     /* Current shop index */



/*
 * State variables extracted from the screen
 */

s32b borg_exp;      /* Current experience */

s32b borg_gold;     /* Current gold */

int borg_stat[6];   /* Current stat values */

int borg_book[9];   /* Current book slots */


/*
 * State variables extracted from the inventory/equipment
 */

int borg_cur_wgt;   /* Current weight */


/*
 * Constant state variables
 */

int borg_race;      /* Player race */
int borg_class;     /* Player class */


/*
 * Hack -- access the class/race records
 */
player_magic *mb_ptr;   /* Player magic info */



/*
 * Number of turns to step for (zero means forever)
 */
u16b borg_step = 0;     /* Step count (if any) */


/*
 * Status message search string
 */
char borg_match[128] = "plain gold ring";  /* Search string */


/*
 * Hack -- single character constants
 */

const char p1 = '(', p2 = ')';
const char c1 = '{', c2 = '}';
const char b1 = '[', b2 = ']';


/*
 * Hack -- the detection arrays
 */

bool borg_detect_wall[6][18];

bool borg_detect_trap[6][18];

bool borg_detect_door[6][18];

bool borg_detect_evil[6][18];

/*
 * Locate the store doors
 */

int *track_shop_x;
int *track_shop_y;


/*
 * Track "stairs up"
 */

s16b track_less_num;
s16b track_less_size;
int *track_less_x;
int *track_less_y;


/*
 * Track "stairs down"
 */

s16b track_more_num;
s16b track_more_size;
int *track_more_x;
int *track_more_y;

/*
 * Track glyphs
 */
s16b track_glyph_num;
s16b track_glyph_size;
int *track_glyph_x;
int *track_glyph_y;

bool borg_needs_new_sea; /* Environment changed.  Need to make a new Sea of Runes for Morgy */

/*
 * Track the items worn to avoid loops
 */
s16b track_worn_num;
s16b track_worn_size;
byte *track_worn_name1;
s16b track_worn_time;

/*
 * ghijk  The borg will use the following ddx and ddy to search
 * d827a  for a suitable grid in an open room.
 * e4@3b
 * f615c
 * lmnop  24 grids
 *
 */
const s16b borg_ddx_ddd[24] =
{ 0, 0, 1, -1, 1, -1, 1, -1, 2, 2, 2, -2, -2, -2, -2, -1, 0, 1, 2, -2, -1, 0, 1, 2};
const s16b borg_ddy_ddd[24] =
{ 1, -1, 0, 0, 1, 1, -1, -1, -1, 0, 1, -1, 0, 1, -2, -2, -2, -2, -2, 2, 2, 2, 2, 2};

/*
 * Track Steps
 */
s16b track_step_num;
s16b track_step_size;
int *track_step_x;
int *track_step_y;

/*
 * Track closed doors which I have closed
 */
s16b track_door_num;
s16b track_door_size;
int *track_door_x;
int *track_door_y;

/*
 * Track closed doors which started closed
 */
s16b track_closed_num;
s16b track_closed_size;
int *track_closed_x;
int *track_closed_y;

/*
 * Track the mineral veins with treasure
 *
 */
s16b track_vein_num;
s16b track_vein_size;
int *track_vein_x;
int *track_vein_y;

/*
 * The object list.  This list is used to "track" objects.
 */

s16b borg_takes_cnt;
s16b borg_takes_nxt;
borg_take *borg_takes;


/*
 * The monster list.  This list is used to "track" monsters.
 */

s16b borg_kills_cnt;
s16b borg_kills_summoner;    /* index of a summoner */
s16b borg_kills_nxt;

borg_kill *borg_kills;


/* a 3 state boolean */
/*-1 = not checked yet */
/* 0 = not ready */
/* 1 = ready */
int borg_ready_morgoth;


/*
 * Hack -- extra fear per "region"
 */
u16b borg_fear_region[6][18];

/*
 * Hack -- extra fear per "region" induced from extra monsters.
 */
u16b borg_fear_monsters[AUTO_MAX_Y][AUTO_MAX_X];

/*
 * Hack -- count racial appearances per level
 */
s16b *borg_race_count;


/*
 * Hack -- count racial kills (for uniques)
 */

s16b *borg_race_death;

/*
 * The current map
 */

borg_grid *borg_grids[AUTO_MAX_Y];  /* The grids */


/*
 * Maintain a set of grids marked as "BORG_LIGHT"
 */

s16b borg_LIGHT_n = 0;

byte borg_LIGHT_x[AUTO_LIGHT_MAX];
byte borg_LIGHT_y[AUTO_LIGHT_MAX];

/*
 * Maintain a set of grids marked as "BORG_GLOW"
 */

s16b borg_glow_n = 0;

byte borg_glow_x[AUTO_LIGHT_MAX];
byte borg_glow_y[AUTO_LIGHT_MAX];


/*
 * Maintain a set of grids marked as "BORG_VIEW"
 */

s16b borg_view_n = 0;

byte borg_view_x[AUTO_VIEW_MAX];
byte borg_view_y[AUTO_VIEW_MAX];


/*
 * Maintain a temporary set of grids
 * Used to store monster info.
 */

s16b borg_temp_n = 0;

byte borg_temp_x[AUTO_TEMP_MAX];
byte borg_temp_y[AUTO_TEMP_MAX];

/*
 * Maintain a temporary set of grids
 * Used to store lit grid info
 */
s16b borg_temp_lit_n = 0;
byte borg_temp_lit_x[AUTO_TEMP_MAX];
byte borg_temp_lit_y[AUTO_TEMP_MAX];

/*
 * Maintain a set of special grids used for Teleport Other
 */
s16b borg_tp_other_n = 0;
byte borg_tp_other_x[255];
byte borg_tp_other_y[255];
int borg_tp_other_index[255];



byte offset_x;
byte offset_y;


/*
 * Maintain a circular queue of grids
 */

s16b borg_flow_n = 0;

byte borg_flow_x[AUTO_FLOW_MAX];
byte borg_flow_y[AUTO_FLOW_MAX];


/*
 * Hack -- use "flow" array as a queue
 */

int flow_head = 0;
int flow_tail = 0;



/*
 * Some variables
 */

borg_data *borg_data_flow;  /* Current "flow" data */

borg_data *borg_data_cost;  /* Current "cost" data */

borg_data *borg_data_hard;  /* Constant "hard" data */

borg_data *borg_data_know;  /* Current "know" flags */

borg_data *borg_data_icky;  /* Current "icky" flags */



/*
 * Strategy flags -- recalculate things
 */

bool borg_danger_wipe = FALSE;  /* Recalculate danger */

bool borg_do_update_view = FALSE;  /* Recalculate view */

bool borg_do_update_LIGHT = FALSE;  /* Recalculate lite */

/*
  * Strategy flags -- examine the world
  */

 bool borg_do_inven = TRUE;  /* Acquire "inven" info */
 bool borg_do_equip = TRUE;  /* Acquire "equip" info */
 bool borg_do_panel = TRUE;  /* Acquire "panel" info */
 bool borg_do_frame = TRUE;  /* Acquire "frame" info */
 bool borg_do_spell = TRUE;  /* Acquire "spell" info */
 byte borg_do_spell_aux = 0; /* Hack -- book for "borg_do_spell" */
 bool borg_do_browse = 0;    /* Acquire "store" info */
 byte borg_do_browse_what = 0;   /* Hack -- store for "borg_do_browse" */
 byte borg_do_browse_more = 0;   /* Hack -- pages for "borg_do_browse" */


 /*
  * Strategy flags -- run certain functions
  */
 bool borg_do_crush_junk = FALSE;
 bool borg_do_crush_hole = FALSE;
 bool borg_do_crush_slow = FALSE;

/* am I fighting a unique? */
int borg_fighting_unique;
bool borg_fighting_evil_unique;		/* Need to know if evil for Priest Banishment */

/* am I fighting a summoner? */
bool borg_fighting_summoner;

/*
 * Calculate "incremental motion". Used by project() and shoot().
 * Assumes that (*y,*x) lies on the path from (y1,x1) to (y2,x2).
 */
/* changing this to be more like project_path */
/* note that this is much slower but much more accurate */
void mmove2(int *py, int *px, int y1, int x1, int y2, int x2)
{
    int dy, dx;
    int sy, sx;
    int y, x;

    /* Scale factors */
    int full, half;

    /* Fractions */
    int frac;

    /* Slope */
    int m;

    /* Distance */
    int k = 0;

    /* Extract the distance travelled */
    /* Analyze "dy" */
    if (y2 < y1)
    {
        dy = (y1 - y2);
        sy = -1;
    }
    else
    {
        dy = (y2 - y1);
        sy = 1;
    }

    /* Analyze "dx" */
    if (x2 < x1)
    {
        dx = (x1 - x2);
        sx = -1;
    }
    else
    {
        dx = (x2 - x1);
        sx = 1;
    }

    /* Paranoia -- Hack -- no motion */
    if (!dy && !dx) return;

    /* Number of "units" in one "half" grid */
    half = (dy * dx);

    /* Number of "units" in one "full" grid */
    full = half << 1;

    /* First step is fixed */
    if (*px == x1 && *py == y1)
    {
        if (dy > dx)
        {
            *py+=sy;
            return;
        }
        else if (dx > dy)
        {
            *px+=sx;
            return;
        }
        else
        {
            *px+=sx;
            *py+=sy;
            return;
        }
    }

    /* Move mostly vertically */
    if (dy > dx)
    {
        k = dy;

        /* Start at tile edge */
        frac = dx * dx;

        /* Let m = ((dx/dy) * full) = (dx * dx * 2) = (frac * 2) */
        m = frac << 1;

        /* Start */
        y = y1 + sy;
        x = x1;

        /* Create the projection path */
        while (1)
        {
            if (x == *px && y == *py)
                k = 1;

            /* Slant */
            if (m)
            {
                /* Advance (X) part 1 */
                frac += m;

                /* Horizontal change */
                if (frac >= half)
                {
                    /* Advance (X) part 2 */
                    x += sx;

                    /* Advance (X) part 3 */
                    frac -= full;
                }
            }

            /* Advance (Y) */
            y += sy;

            /* Track distance */
            k--;

            if (!k)
            {
                *px = x;
                *py = y;
                return;
            }
        }
    }
    /* Move mostly horizontally */
    else if (dx > dy)
    {
        /* Start at tile edge */
        frac = dy * dy;

        /* Let m = ((dy/dx) * full) = (dy * dy * 2) = (frac * 2) */
        m = frac << 1;

        /* Start */
        y = y1;
        x = x1 + sx;
        k = dx;

        /* Create the projection path */
        while (1)
        {
            if (x == *px && y == *py)
                k = 1;

            /* Slant */
            if (m)
            {
                /* Advance (Y) part 1 */
                frac += m;

                /* Vertical change */
                if (frac >= half)
                {
                    /* Advance (Y) part 2 */
                    y += sy;

                    /* Advance (Y) part 3 */
                    frac -= full;
                }
            }

            /* Advance (X) */
            x += sx;

            /* Track distance */
            k--;

            if (!k)
            {
                *px = x;
                *py = y;
                return;
            }
        }
    }
    /* Diagonal */
    else
    {
        /* Start */
        k = dy;
        y = y1 + sy;
        x = x1 + sx;

        /* Create the projection path */
        while (1)
        {
            if (x == *px && y == *py)
                k=1;

            /* Advance (Y) */
            y += sy;

            /* Advance (X) */
            x += sx;

            /* Track distance */
            k--;

            if (!k)
            {
                *px = x;
                *py = y;
                return;
            }
        }
    }
}

/*
 * Query the "attr/char" at a given location on the screen
 * We return "zero" if the given location was legal
 *
 * XXX XXX XXX We assume the given location is legal
 */
errr borg_what_char(int x, int y, byte *a, wchar_t *c)
{
    /* Direct access XXX XXX XXX */
    (*a) = (Term->scr->a[y][x]);
    (*c) = (Term->scr->c[y][x]);

    /* Success */
    return (0);
}


/*
 * Query the "attr/chars" at a given location on the screen
 *
 * Note that "a" points to a single "attr", and "s" to an array
 * of "chars", into which the attribute and text at the given
 * location are stored.
 *
 * We will not grab more than "ABS(n)" characters for the string.
 * If "n" is "positive", we will grab exactly "n" chars, or fail.
 * If "n" is "negative", we will grab until the attribute changes.
 *
 * We automatically convert all "blanks" and "invisible text" into
 * spaces, and we ignore the attribute of such characters.
 *
 * We do not strip final spaces, so this function will very often
 * read characters all the way to the end of the line.
 *
 * We succeed only if a string of some form existed, and all of
 * the non-space characters in the string have the same attribute,
 * and the string was long enough.
 *
 * XXX XXX XXX We assume the given location is legal
 */
errr borg_what_text(int x, int y, int n, byte *a, char *s)
{
    int i;
    wchar_t screen_str[1024];

    byte t_a;
    wchar_t t_c;

    byte *aa;
    wchar_t *cc;

	int w, h;

    /* Current attribute */
    byte d_a = 0;

    /* Max length to scan for */
    int m = ABS(n);

    /* Activate */
    /* Do I need to get the right window? Term_activate(angband_term[0]); */

	/* Obtain the size */
	(void)Term_get_size(&w, &h);

    /* Hack -- Do not run off the screen */
    if (x + m > w) m = w - x;

    /* Direct access XXX XXX XXX */
    aa = &(Term->scr->a[y][x]);
    cc = &(Term->scr->c[y][x]);

    /* Grab the string */
    for (i = 0; i < m; i++)
    {
        /* Access */
        t_a = *aa++;
        t_c = *cc++;

        /* Handle spaces */
        if ((t_c == L' ') || !t_a)
        {
            /* Save space */
            screen_str[i] = L' ';
        }

        /* Handle real text */
        else
        {
            /* Attribute ready */
            if (d_a)
            {
                /* Verify the "attribute" (or stop) */
                if (t_a != d_a) break;
            }

            /* Acquire attribute */
            else
            {
                /* Save it */
                d_a = t_a;
            }

            /* Save char */
            screen_str[i] = t_c;
        }
    }

    /* Terminate the string */
    screen_str[i] = L'\0';

    /* Save the attribute */
    (*a) = d_a;

    /* Convert back to a char string */
    wcstombs(s, screen_str, n+1);
    /* Too short */
    if ((n > 0) && (i != n)) return (1);

    /* Success */
    return (0);
}



/*
 * Log a message to a file
 */
void borg_info(char *what)
{

}



/*
 * Memorize a message, Log it, Search it, and Display it in pieces
 */
void borg_note(char *what)
{
    int j, n, i, k;

    int w, h, x, y;


    term *old = Term;

    /* Memorize it */
    message_add(what, MSG_GENERIC);


    /* Log the message */
    borg_info(what);


    /* Mega-Hack -- Check against the search string */
    if (borg_match[0] && strstr(what, borg_match))
    {
        /* Clean cancel */
        borg_cancel = TRUE;
    }

    /* Mega-Hack -- Check against the swap loops */
    if (strstr(what, "Best Combo") ||
	    strstr(what, "Taking off "))
    {
        /* Tick the anti loop clock */
        time_this_panel += 10;
        borg_note(format("# Anti-loop variable tick (%d).", time_this_panel));
    }

    /* Scan windows */
    for (j = 0; j < 8; j++)
    {
        if (!angband_term[j]) continue;

        /* Check flag */
        if (!(op_ptr->window_flag[j] & PW_BORG_1)) continue;

        /* Activate */
        Term_activate(angband_term[j]);

        /* Access size */
        Term_get_size(&w, &h);

        /* Access cursor */
        Term_locate(&x, &y);

        /* Erase current line */
        Term_erase(0, y, 255);


        /* Total length */
        n = strlen(what);

        /* Too long */
        if (n > w - 2)
        {
            char buf[1024];

            /* Split */
            while (n > w - 2)
            {
                /* Default */
                k = w - 2;

                /* Find a split point */
                for (i = w / 2; i < w - 2; i++)
                {
                    /* Pre-emptive split point */
                    if (isspace(what[i])) k = i;
                }

                /* Copy over the split message */
                for (i = 0; i < k; i++)
                {
                    /* Copy */
                    buf[i] = what[i];
                }

                /* Indicate split */
                buf[i++] = '\\';

                /* Terminate */
                buf[i] = '\0';

                /* Show message */
                Term_addstr(-1, TERM_WHITE, buf);

                /* Advance (wrap) */
                if (++y >= h) y = 0;

                /* Erase next line */
                Term_erase(0, y, 255);

                /* Advance */
                what += k;

                /* Reduce */
                n -= k;
            }

            /* Show message tail */
            Term_addstr(-1, TERM_WHITE, what);

            /* Advance (wrap) */
            if (++y >= h) y = 0;

            /* Erase next line */
            Term_erase(0, y, 255);
        }

        /* Normal */
        else
        {
            /* Show message */
            Term_addstr(-1, TERM_WHITE, what);

            /* Advance (wrap) */
            if (++y >= h) y = 0;

            /* Erase next line */
            Term_erase(0, y, 255);
        }


        /* Flush output */
        Term_fresh();

        /* Use correct window */
        Term_activate(old);
    }
 }




/*
 * Abort the Borg, noting the reason
 */
void borg_oops(char *what)
{
    /* Stop processing */
    borg_active = FALSE;

    /* Give a warning */
    borg_note(format("# Aborting (%s).", what));

    /* Forget borg keys */
    borg_flush();
}

/*
 * A Queue of keypresses to be sent
 */
static keycode_t *borg_key_queue;
static s16b borg_key_head;
static s16b borg_key_tail;


/*
 * Add a keypress to the "queue" (fake event)
 */
errr borg_keypress(keycode_t k)
{
    /* Hack -- Refuse to enqueue "nul" */
    if (!k) return (-1);

	/* Hack -- note the keypress */
   	borg_note(format("& Key <%c>", k));

    /* Store the char, advance the queue */
    borg_key_queue[borg_key_head++] = k;

    /* Circular queue, handle wrap */
    if (borg_key_head == KEY_SIZE) borg_key_head = 0;

    /* Hack -- Catch overflow (forget oldest) */
    if (borg_key_head == borg_key_tail) borg_oops("overflow");

    /* Hack -- Overflow may induce circular queue */
    if (borg_key_tail == KEY_SIZE) borg_key_tail = 0;

    /* Success */
    return (0);
}


/*
 * Add a keypress to the "queue" (fake event)
 */
errr borg_keypresses(char *str)
{
    char *s;

    /* Enqueue them */
    for (s = str; *s; s++) borg_keypress(*s);

    /* Success */
    return (0);
}


/*
 * Get the next Borg keypress
 */
char borg_inkey(bool take)
{
    int i;

    /* Nothing ready */
    if (borg_key_head == borg_key_tail)
        return (0);

    /* Extract the keypress */
    i = borg_key_queue[borg_key_tail];

    /* Do not advance */
    if (!take) return (i);

    /* Advance the queue */
    borg_key_tail++;

    /* Circular queue requires wrap-around */
    if (borg_key_tail == KEY_SIZE) borg_key_tail = 0;

    /* Return the key */
    return (i);
}



/*
 * Get the next Borg keypress
 */
void borg_flush(void)
{
    /* Simply forget old keys */
    borg_key_tail = borg_key_head;
}






/*
 * Hack -- take a note later
 */
bool borg_tell(char *what)
{
    char *s;

    /* Hack -- self note */
    borg_keypress(':');
    for (s = what; *s; s++) borg_keypress(*s);
    borg_keypress('\n');

    /* Success */
    return (TRUE);
}



/*
 * Attempt to change the borg's name
 */
bool borg_change_name(char *str)
{
    char *s;

    /* Cancel everything */
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);

    /* Character description */
    borg_keypress('C');

    /* Change the name */
    borg_keypress('c');

    /* Enter the new name */
    for (s = str; *s; s++) borg_keypress(*s);

    /* End the name */
    borg_keypress(KC_ENTER);

    /* Cancel everything */
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);

    /* Success */
    return (TRUE);
}


/*
 * Attempt to dump a character description file
 */
bool borg_dump_character(char *str)
{
    char *s;

    /* Cancel everything */
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);

    /* Character description */
    borg_keypress('C');

    /* Dump character file */
    borg_keypress('f');

    /* Enter the new name */
    for (s = str; *s; s++) borg_keypress(*s);

    /* End the file name */
    borg_keypress(KC_ENTER);

    /* Cancel everything */
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);

    /* Success */
    return (TRUE);
}




/*
 * Attempt to save the game
 */
bool borg_save_game(void)
{
    /* Cancel everything */
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);

    /* Save the game */
    borg_keypress('^');
    borg_keypress('S');

    /* Cancel everything */
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);

    /* Success */
    return (TRUE);
}




/*
 * Update the Borg based on the current "frame"
 *
 * Assumes the Borg is actually in the dungeon.
 */
void borg_update_frame(void)
{
    int i;

	player_state *state = &p_ptr->state;

    /* Assume level is fine */
    borg_skill[BI_ISFIXLEV] = FALSE;

    /* Note "Lev" vs "LEV" */
    if (p_ptr->lev < p_ptr->max_lev) borg_skill[BI_ISFIXLEV] = TRUE;

    /* Extract "LEVEL xxxxxx" */
    borg_skill[BI_CLEVEL] = p_ptr->lev;

    /* cheat the max clevel */
    borg_skill[BI_MAXCLEVEL] = p_ptr->max_lev;

    /* Note "Winner" */
    borg_skill[BI_KING] = p_ptr->total_winner;

    /* Assume experience is fine */
    borg_skill[BI_ISFIXEXP] = FALSE;

    /* Note "Exp" vs "EXP" and am I lower than level 50*/
    if (p_ptr->exp < p_ptr->max_exp)
	{
		/* fix it if in town */
		if (borg_skill[BI_CLEVEL] == 50 && borg_skill[BI_CDEPTH] == 0) borg_skill[BI_ISFIXEXP] = TRUE;

		/* dont worry about fixing it in the dungeon */
		if (borg_skill[BI_CLEVEL] == 50 && borg_skill[BI_CDEPTH] >=1) borg_skill[BI_ISFIXEXP] = FALSE;

		/* Not at Max Level */
		if (borg_skill[BI_CLEVEL] != 50) borg_skill[BI_ISFIXEXP] = TRUE;
	}

    /* Extract "EXP xxxxxxxx" */
    borg_exp = p_ptr->exp;


    /* Extract "AU xxxxxxxxx" */
    borg_gold = p_ptr->au;


    /* Extract "Fast (+x)" or "Slow (-x)" */
    borg_skill[BI_SPEED] = state->speed;

    /* Check my float for decrementing variables */
    if (borg_skill[BI_SPEED] >110)
    {
        borg_game_ratio = 100000/(((borg_skill[BI_SPEED]-110)*10)+100);
    }
    else
    {
        borg_game_ratio = 1000;
    }


	/* A quick cheat to see if I missed a message about my status on some timed spells */
    if (!goal_recalling && p_ptr->word_recall ) goal_recalling = TRUE;
    if (!borg_prot_from_evil && p_ptr->timed[TMD_PROTEVIL]) borg_prot_from_evil = (p_ptr->timed[TMD_PROTEVIL] ? TRUE : FALSE);
	if (!borg_speed && (p_ptr->timed[TMD_FAST] || p_ptr->timed[TMD_SPRINT] || p_ptr->timed[TMD_TERROR]))
		(borg_speed = (p_ptr->timed[TMD_FAST] || p_ptr->timed[TMD_SPRINT] || p_ptr->timed[TMD_TERROR]) ? TRUE : FALSE);
    borg_skill[BI_TRACID] = (p_ptr->timed[TMD_OPP_ACID] ? TRUE : FALSE);
    borg_skill[BI_TRELEC] = (p_ptr->timed[TMD_OPP_ELEC] ? TRUE : FALSE);
    borg_skill[BI_TRFIRE] = (p_ptr->timed[TMD_OPP_FIRE] ? TRUE : FALSE);
    borg_skill[BI_TRCOLD] = (p_ptr->timed[TMD_OPP_COLD] ? TRUE : FALSE);
    borg_skill[BI_TRPOIS] = (p_ptr->timed[TMD_OPP_POIS] ? TRUE : FALSE);
    borg_bless = (p_ptr->timed[TMD_BLESSED] ? TRUE : FALSE);
    borg_shield = (p_ptr->timed[TMD_SHIELD] ? TRUE : FALSE);
    borg_shield = (p_ptr->timed[TMD_STONESKIN] ? TRUE : FALSE);
    borg_hero = (p_ptr->timed[TMD_HERO] ? TRUE : FALSE);
    borg_berserk = (p_ptr->timed[TMD_SHERO] ? TRUE : FALSE);

	/* if hasting, it doesn't count as 'borg_speed'.  The speed */
    /* gained from hasting is counted seperately. */
    if (borg_speed)
	{
        if (p_ptr->timed[TMD_FAST] || p_ptr->timed[TMD_SPRINT]) borg_skill[BI_SPEED] -= 10;
		else if (p_ptr->timed[TMD_TERROR]) borg_skill[BI_SPEED] -=5;
	}

    /* Extract "Cur AC xxxxx" */
    borg_skill[BI_ARMOR] = state->dis_ac + state->dis_to_a;

    /* Extract "Cur HP xxxxx" */
    borg_skill[BI_CURHP] = p_ptr->chp;

    /* Extract "Max HP xxxxx" */
    borg_skill[BI_MAXHP] = p_ptr->mhp;

    /* Extract "Cur SP xxxxx" (or zero) */
    borg_skill[BI_CURSP] = p_ptr->csp;

    /* Extract "Max SP xxxxx" (or zero) */
    borg_skill[BI_MAXSP] = p_ptr->msp;

    /* Clear all the "state flags" */
    borg_skill[BI_ISWEAK] = borg_skill[BI_ISHUNGRY] = borg_skill[BI_ISFULL] = borg_skill[BI_ISGORGED] = FALSE;
    borg_skill[BI_ISBLIND] = borg_skill[BI_ISCONFUSED] = borg_skill[BI_ISAFRAID] = borg_skill[BI_ISPOISONED] = FALSE;
    borg_skill[BI_ISCUT] = borg_skill[BI_ISSTUN] = borg_skill[BI_ISHEAVYSTUN] = borg_skill[BI_ISIMAGE] = borg_skill[BI_ISSTUDY] = FALSE;
    borg_skill[BI_ISSEARCHING] = FALSE;
	borg_skill[BI_ISPARALYZED] = FALSE;
	borg_skill[BI_ISFORGET] = FALSE;

    /* Check for "Weak" */
    if (p_ptr->food < PY_FOOD_WEAK) borg_skill[BI_ISWEAK] = borg_skill[BI_ISHUNGRY] = TRUE;

    /* Check for "Hungry" */
    else if (p_ptr->food < PY_FOOD_ALERT) borg_skill[BI_ISHUNGRY] = TRUE;

    /* Check for "Normal" */
    else if (p_ptr->food < PY_FOOD_FULL) /* Nothing */;

    /* Check for "Full" */
    else if (p_ptr->food < PY_FOOD_MAX) borg_skill[BI_ISFULL] = TRUE;

    /* Check for "Gorged" */
    else borg_skill[BI_ISGORGED] = borg_skill[BI_ISFULL] = TRUE;

    /* Check for "Blind" */
    if (p_ptr->timed[TMD_BLIND]) borg_skill[BI_ISBLIND] = TRUE;

    /* Check for "Confused" */
    if (p_ptr->timed[TMD_CONFUSED]) borg_skill[BI_ISCONFUSED] = TRUE;

    /* Check for "Afraid" */
	if (p_ptr->timed[TMD_AFRAID]) borg_skill[BI_ISAFRAID] = TRUE;

    /* Check for "Poisoned" */
    if (p_ptr->timed[TMD_POISONED]) borg_skill[BI_ISPOISONED] = TRUE;

    /* Check for any text */
    if (p_ptr->timed[TMD_CUT]) borg_skill[BI_ISCUT] = TRUE;

    /* Check for Stun */
    if (p_ptr->timed[TMD_STUN] && (p_ptr->timed[TMD_STUN] <= 50)) borg_skill[BI_ISSTUN] = TRUE;

    /* Check for Heavy Stun */
    if (p_ptr->timed[TMD_STUN] > 50) borg_skill[BI_ISHEAVYSTUN] = TRUE;

    /* Check for Paralyze */
    if (p_ptr->timed[TMD_PARALYZED] > 50) borg_skill[BI_ISPARALYZED] = TRUE;

    /* Check for "Hallucinating" */
    if (p_ptr->timed[TMD_IMAGE]) borg_skill[BI_ISIMAGE] = TRUE;

    /* Check for "Amnesia" */
    if (p_ptr->timed[TMD_AMNESIA]) borg_skill[BI_ISFORGET] = TRUE;

	/* Check to "Bless" */
	borg_bless = (p_ptr->timed[TMD_BLESSED] ? TRUE : FALSE);

    /* Check for Searching mode */
    if (p_ptr->searching) borg_skill[BI_ISSEARCHING] = TRUE;

    /* Check for "Study" */
    if (p_ptr->new_spells) borg_skill[BI_ISSTUDY] = TRUE;


    /* Parse stats */
    for (i = 0; i < 6; i++)
    {
        borg_skill[BI_ISFIXSTR+i] = p_ptr->stat_cur[A_STR+i] < p_ptr->stat_max[A_STR+i];
        borg_skill[BI_CSTR+i] = p_ptr->stat_cur[A_STR+i];
        borg_stat[i] = p_ptr->stat_cur[i];

    }

    /* Hack -- Access max depth */
    borg_skill[BI_CDEPTH] = p_ptr->depth;

    /* Hack -- Access max depth */
    borg_skill[BI_MAXDEPTH] = p_ptr->max_depth;

}


int
borg_check_formula(int *formula)
{
    int     oper1;          /* operand #1 */
    int     oper2;          /* operand #2 */
    int     stack[256];     /* stack */
    int     *stackptr;      /* stack pointer */

    /* loop until we hit BFO_DONE */
    for (stackptr = stack; *formula; formula++)
    {
        if (stackptr < stack)
            return 0;
        switch (*formula)
        {
            /* Number */
            case BFO_NUMBER:
                *stackptr++ = *++formula;
                break;

            /* Variable */
            case BFO_VARIABLE:
                *stackptr++ = borg_has[*++formula];
                if ((*formula) > (z_info->k_max + z_info->k_max + z_info->a_max + BI_MAX))
                    return 0;
                break;

            /* Equal */
            case BFO_EQ:
                oper2 = *--stackptr;
                oper1 = *--stackptr;
                *stackptr++ = (oper1 == oper2);
                break;

            /* Not Equal */
            case BFO_NEQ:
                oper2 = *--stackptr;
                oper1 = *--stackptr;
                *stackptr++ = (oper1 != oper2);
                break;

            /* Less Than */
            case BFO_LT:
                oper2 = *--stackptr;
                oper1 = *--stackptr;
                *stackptr++ = (oper1 < oper2);
                break;

            /* Less Than Or Equal */
            case BFO_LTE:
                oper2 = *--stackptr;
                oper1 = *--stackptr;
                *stackptr++ = (oper1 <= oper2);
                break;

            /* Greater Than */
            case BFO_GT:
                oper2 = *--stackptr;
                oper1 = *--stackptr;
                *stackptr++ = (oper1 > oper2);
                break;

            /* Greater Than Or Equal */
            case BFO_GTE:
                oper2 = *--stackptr;
                oper1 = *--stackptr;
                *stackptr++ = (oper1 >= oper2);
                break;

            /* Logical And */
            case BFO_AND:
                oper2 = *--stackptr;
                oper1 = *--stackptr;
                *stackptr++ = (oper1 && oper2);
                break;

            /* Logical Or */
            case BFO_OR:
                oper2 = *--stackptr;
                oper1 = *--stackptr;
                *stackptr++ = (oper1 || oper2);
                break;

            /* Plus */
            case BFO_PLUS:
                oper2 = *--stackptr;
                oper1 = *--stackptr;
                *stackptr++ = (oper1 + oper2);
                break;

            /* Minus */
            case BFO_MINUS:
                oper2 = *--stackptr;
                oper1 = *--stackptr;
                *stackptr++ = (oper1 - oper2);
                break;

            /* Divide */
            case BFO_DIVIDE:
                oper2 = *--stackptr;
                oper1 = *--stackptr;
                *stackptr++ = (oper1 / (oper2 ? oper2 : 1));
                break;

            /* Multiply */
            case BFO_MULT:
                oper2 = *--stackptr;
                oper1 = *--stackptr;
                *stackptr++ = (oper1 * oper2);
                break;

            /* Logical Not */
            case BFO_NOT:
                oper1 = *--stackptr;
                *stackptr++ = (!oper1);
                break;

            default:
                return 0;
        }
    }

    if (stackptr != (stack+1))
        return 0;
    return 1;
}

char *borg_prt_formula(int *formula)
{
    static char FormulaStr[2000];
    char tmpStr[50];

    memset(FormulaStr, 0, sizeof(FormulaStr));
    /* loop until we hit BFO_DONE */
    for (; *formula; formula++)
    {
        switch (*formula)
        {
            /* Number */
            case BFO_NUMBER:

                sprintf(tmpStr, "%d ", *++formula);
                strcat(FormulaStr, tmpStr);
                break;

            /* Variable */
            case BFO_VARIABLE:
                strcat(FormulaStr, "'");
                strcat(FormulaStr, borg_prt_item(*++formula));
                strcat(FormulaStr, "'");
                strcat(FormulaStr, " ");
                break;

            /* Equal */
            case BFO_EQ:
                strcat(FormulaStr, "== ");
                break;

            /* Not Equal */
            case BFO_NEQ:
                strcat(FormulaStr, "!= ");
                break;

            /* Less Than */
            case BFO_LT:
                strcat(FormulaStr, "< ");
                break;

            /* Less Than Or Equal */
            case BFO_LTE:
                strcat(FormulaStr, "<= ");
                break;

            /* Greater Than */
            case BFO_GT:
                strcat(FormulaStr, "> ");
                break;

            /* Greater Than Or Equal */
            case BFO_GTE:
                strcat(FormulaStr, ">= ");
                break;

            /* Logical And */
            case BFO_AND:
                strcat(FormulaStr, "&& ");
                break;

            /* Logical Or */
            case BFO_OR:
                strcat(FormulaStr, "|| ");
                break;

            /* Plus */
            case BFO_PLUS:
                strcat(FormulaStr, "+ ");
                break;

            /* Minus */
            case BFO_MINUS:
                strcat(FormulaStr, "- ");
                break;

            /* Divide */
            case BFO_DIVIDE:
                strcat(FormulaStr, "/ ");
                break;

            /* Multiply */
            case BFO_MULT:
                strcat(FormulaStr, "* ");
                break;

            /* Logical Not */
            case BFO_NOT:
                strcat(FormulaStr, "! ");
                break;
        }
    }

    /* BFO_DONE */
    return FormulaStr;
}

int
borg_calc_formula(int *formula)
{
    int     oper1;          /* operand #1 */
    int     oper2;          /* operand #2 */
    int     stack[256];     /* stack */
    int     *stackptr;      /* stack pointer */


    if (!formula)
        return 0;

    *stack = 0;
    /* loop until we hit BFO_DONE */
    for (stackptr = stack; *formula; formula++)
    {
        switch (*formula)
        {
            /* Number */
            case BFO_NUMBER:
                *stackptr++ = *++formula;
                break;

            /* Variable */
            case BFO_VARIABLE:
                *stackptr++ = borg_has[*++formula];
                break;

            /* Equal */
            case BFO_EQ:
                oper2 = *--stackptr;
                oper1 = *--stackptr;
                *stackptr++ = (oper1 == oper2);
                break;

            /* Not Equal */
            case BFO_NEQ:
                oper2 = *--stackptr;
                oper1 = *--stackptr;
                *stackptr++ = (oper1 != oper2);
                break;

            /* Less Than */
            case BFO_LT:
                oper2 = *--stackptr;
                oper1 = *--stackptr;
                *stackptr++ = (oper1 < oper2);
                break;

            /* Less Than Or Equal */
            case BFO_LTE:
                oper2 = *--stackptr;
                oper1 = *--stackptr;
                *stackptr++ = (oper1 <= oper2);
                break;

            /* Greater Than */
            case BFO_GT:
                oper2 = *--stackptr;
                oper1 = *--stackptr;
                *stackptr++ = (oper1 > oper2);
                break;

            /* Greater Than Or Equal */
            case BFO_GTE:
                oper2 = *--stackptr;
                oper1 = *--stackptr;
                *stackptr++ = (oper1 >= oper2);
                break;

            /* Logical And */
            case BFO_AND:
                oper2 = *--stackptr;
                oper1 = *--stackptr;
                *stackptr++ = (oper1 && oper2);
                break;

            /* Logical Or */
            case BFO_OR:
                oper2 = *--stackptr;
                oper1 = *--stackptr;
                *stackptr++ = (oper1 || oper2);
                break;

            /* Plus */
            case BFO_PLUS:
                oper2 = *--stackptr;
                oper1 = *--stackptr;
                *stackptr++ = (oper1 + oper2);
                break;

            /* Minus */
            case BFO_MINUS:
                oper2 = *--stackptr;
                oper1 = *--stackptr;
                *stackptr++ = (oper1 - oper2);
                break;

            /* Divide */
            case BFO_DIVIDE:
                oper2 = *--stackptr;
                oper1 = *--stackptr;
                *stackptr++ = (oper1 / (oper2 ? oper2 : 1));
                break;

            /* Multiply */
            case BFO_MULT:
                oper2 = *--stackptr;
                oper1 = *--stackptr;
                *stackptr++ = (oper1 * oper2);
                break;

            /* Logical Not */
            case BFO_NOT:
                oper1 = *--stackptr;
                *stackptr++ = (!oper1);
                break;
        }
    }

    /* BFO_DONE */
    return *--stackptr;
}




bool (*borg_sort_comp)(void* u, void* v, int a, int b);
void (*borg_sort_swap)(void* u, void* v, int a, int b);

/*
 * Borg's sorting algorithm -- quick sort in place
 *
 * Note that the details of the data we are sorting is hidden,
 * and we rely on the "ang_sort_comp()" and "ang_sort_swap()"
 * function hooks to interact with the data, which is given as
 * two pointers, and which may have any user-defined form.
 */
void borg_sort_aux(void *u, void *v, int p, int q)
{
    int z, a, b;

    /* Done sort */
    if (p >= q) return;

    /* Pivot */
    z = p;

    /* Begin */
    a = p;
    b = q;

    /* Partition */
    while (TRUE)
    {
        /* Slide i2 */
        while (!(*borg_sort_comp)(u, v, b, z)) b--;

        /* Slide i1 */
        while (!(*borg_sort_comp)(u, v, z, a)) a++;

        /* Done partition */
        if (a >= b) break;

        /* Swap */
        (*borg_sort_swap)(u, v, a, b);

        /* Advance */
        a++, b--;
    }

    /* Recurse left side */
    borg_sort_aux(u, v, p, b);

    /* Recurse right side */
    borg_sort_aux(u, v, b+1, q);
}


/*
 * Borg's sorting algorithm -- quick sort in place
 *
 * Note that the details of the data we are sorting is hidden,
 * and we rely on the "ang_sort_comp()" and "ang_sort_swap()"
 * function hooks to interact with the data, which is given as
 * two pointers, and which may have any user-defined form.
 */
void borg_sort(void *u, void *v, int n)
{
    /* Sort the array */
    borg_sort_aux(u, v, 0, n-1);
}

/*
 * Sorting hook -- comp function -- see below
 *
 * We use "u" to point to an array of strings, and "v" to point to
 * an array of indexes, and we sort them together by the strings.
 */
bool borg_sort_comp_hook(void *u, void *v, int a, int b)
{
    char **text = (char **)(u);
    s16b *what = (s16b*)(v);

    int cmp;

    /* Compare the two strings */
    cmp = (strcmp(text[a], text[b]));

    /* Strictly less */
    if (cmp < 0) return (TRUE);

    /* Strictly more */
    if (cmp > 0) return (FALSE);

    /* Enforce "stable" sort */
    return (what[a] <= what[b]);
}

/*
 * Sorting hook -- swap function -- see below
 *
 * We use "u" to point to an array of strings, and "v" to point to
 * an array of indexes, and we sort them together by the strings.
 */
void borg_sort_swap_hook(void *u, void *v, int a, int b)
{
    char **text = (char**)(u);
    s16b *what = (s16b*)(v);

    char *texttmp;
    s16b whattmp;

    /* Swap "text" */
    texttmp = text[a];
    text[a] = text[b];
    text[b] = texttmp;

    /* Swap "what" */
    whattmp = what[a];
    what[a] = what[b];
    what[b] = whattmp;
}



/*
 * Initialize this file
 */
void borg_init_1(void)
{
    int i, x, y;


    /* Allocate the "keypress queue" */
    C_MAKE(borg_key_queue, KEY_SIZE, keycode_t);


    /* Prapare a local random number seed */
if (!borg_rand_local)
    borg_rand_local = randint1(0x10000000);


    /*** Grids ***/

    /* Make each row of grids */
    for (y = 0; y < AUTO_MAX_Y; y++)
    {
        /* Make each row */
        C_MAKE(borg_grids[y], AUTO_MAX_X, borg_grid);
    }


    /*** Grid data ***/

    /* Allocate */
    MAKE(borg_data_flow, borg_data);

    /* Allocate */
    MAKE(borg_data_cost, borg_data);

    /* Allocate */
    MAKE(borg_data_hard, borg_data);

    /* Allocate */
    MAKE(borg_data_know, borg_data);

    /* Allocate */
    MAKE(borg_data_icky, borg_data);

    /* Prepare "borg_data_hard" */
    for (y = 0; y < AUTO_MAX_Y; y++)
    {
        for (x = 0; x < AUTO_MAX_X; x++)
        {
            /* Prepare "borg_data_hard" */
            borg_data_hard->data[y][x] = 255;
        }
    }


    /*** Very special "tracking" array ***/

    /* Track the shop locations */
    C_MAKE(track_shop_x, 9, int);
    C_MAKE(track_shop_y, 9, int);


    /*** Special "tracking" arrays ***/

    /* Track "up" stairs */
    track_less_num = 0;
    track_less_size = 16;
    C_MAKE(track_less_x, track_less_size, int);
    C_MAKE(track_less_y, track_less_size, int);

    /* Track "down" stairs */
    track_more_num = 0;
    track_more_size = 16;
    C_MAKE(track_more_x, track_more_size, int);
    C_MAKE(track_more_y, track_more_size, int);

    /* Track glyphs */
    track_glyph_num = 0;
    track_glyph_size = 200;
    C_MAKE(track_glyph_x, track_glyph_size, int);
    C_MAKE(track_glyph_y, track_glyph_size, int);

    /* Track the worn items to avoid loops */
    track_worn_num = 0;
    track_worn_size = 10;
	track_worn_time = 0;
    C_MAKE(track_worn_name1, track_worn_size, byte);

    /* Track Steps */
    track_step_num = 0;
    track_step_size = 100;
    C_MAKE(track_step_x, track_step_size, int);
    C_MAKE(track_step_y, track_step_size, int);

    /* Track doors closed by borg */
    track_door_num = 0;
    track_door_size = 100;
    C_MAKE(track_door_x, track_door_size, int);
    C_MAKE(track_door_y, track_door_size, int);

    /* Track closed doors on map */
    track_closed_num = 0;
    track_closed_size = 100;
    C_MAKE(track_closed_x, track_closed_size, int);
    C_MAKE(track_closed_y, track_closed_size, int);

    /* Track mineral veins with treasure. */
    track_vein_num = 0;
    track_vein_size = 100;
    C_MAKE(track_vein_x, track_vein_size, int);
    C_MAKE(track_vein_y, track_vein_size, int);

    /*** Object tracking ***/

    /* No objects yet */
    borg_takes_cnt = 0;
    borg_takes_nxt = 1;

    /* Array of objects */
    C_MAKE(borg_takes, 256, borg_take);

    /*** Monster tracking ***/

    /* No monsters yet */
    borg_kills_cnt = 0;
    borg_kills_nxt = 1;

    /* Array of monsters */
    C_MAKE(borg_kills, 256, borg_kill);

    /*** Special counters ***/

    /* Count racial appearances */
    C_MAKE(borg_race_count, z_info->r_max, s16b);

    /* Count racial deaths */
    C_MAKE(borg_race_death, z_info->r_max, s16b);


    /*** XXX XXX XXX Hack -- Cheat ***/

    /* Hack -- Extract dead uniques */
    for (i = 1; i < z_info->r_max-1; i++)
    {
        monster_race *r_ptr = &r_info[i];

        /* Skip non-monsters */
        if (!r_ptr->name) continue;

        /* Skip non-uniques */
        if (rf_has(r_ptr->flags, RF_UNIQUE)) continue;

        /* Mega-Hack -- Access "dead unique" list */
        if (r_ptr->max_num == 0) borg_race_death[i] = 1;
    }

}

/*** Object kind lookup functions ***/

/**
 * Return the k_idx of the object kind with the given `tval` and `sval`, or 0.
 */
int borg_lookup_kind(int tval, int sval)
{
	int k;

	/* Look for it */
	for (k = 1; k < z_info->k_max; k++)
	{
		object_kind *k_ptr = &k_info[k];

		/* Found a match */
		if ((k_ptr->tval == tval) && (k_ptr->sval == sval)) return (k);
	}

	/* Failure */
	msg("No object (%s,%d,%d)", tval_find_name(tval), tval, sval);
	return 0;
}



#ifdef MACINTOSH
static int HACK = 0;
#endif

#endif /* ALLOW_BORG */
