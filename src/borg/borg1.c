/* File: borg1.c */
/* Purpose: Low level stuff for the Borg -BEN- */
#include "angband.h"

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
int borg_dump_level;
int borg_save_death;
bool borg_respawn_winners;
int borg_delay_factor;


/* dynamic borg stuff */
bool borg_init_failure;
bool borg_uses_swaps;
bool borg_uses_calcs = true;
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

req_item** borg_required_item;
int* n_req;
power_item** borg_power_item;
int* n_pwr;
int* borg_has;
int* borg_skill;
int size_depth;
int size_obj;
int* formula[1000];

const char* prefix_pref[] =
{
    /* personal attributes */
    "_STR",
    "_INT",
    "_WIS",
    "_DEX",
    "_CON",
    "_CSTR",
    "_CINT",
    "_CWIS",
    "_CDEX",
    "_CCON",
    "_SSTR",
    "_SINT",
    "_SWIS",
    "_SDEX",
    "_SCON",
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
    "_FOOD",
    "_SPEED",
    "_MOD_MOVES",
    "_DAM_RED",
    "_SDIG",
    "_FEATH",
    "_REG",
    "_SINV",
    "_INFRA",
    "_DISP",
    "_DISM",
    "_DEV",
    "_SAV",
    "_STL",
    "_SRCH",
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
    "_ISFIXLEV",
    "_ISFIXEXP",
    "_ISFIXSTR",
    "_ISFIXINT",
    "_ISFIXWIS",
    "_ISFIXDEX",
    "_ISFIXCON",
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
    "_AMMO_COUNT", /* count of all ammo */
    "_AMMO_TVAL",
    "_AMMO_SIDES",
    "_AMMO_POWER",
    "_AMISSILES",  /* only ones for your current bow count */
    "_QUIVER_SLOTS", /* number of inven slots the quivered items take */
    "_FIRST_CURSED", /* first cursed item */

    /* curses */
    "_CRSTELE",
    "_CRSPOIS",
    "_CRSSIREN",
    "_CRSHALU",
    "_CRSPARA",
    "_CRSSDEM",
    "_CRSSDRA",
    "_CRSSUND",
    "_CRSSTONE",
    "_CRSNOTEL",
    "_CRSTWEP",
    "_CRSUNKNO",
    "_CRSAGRV",
    "_CRSHPIMP",	/* Impaired HP recovery */
    "_CRSMPIMP",	/* Impaired MP recovery */
    "_CRSFEAR",		/* Fear curse flag */
    "_CRSDRAIN_XP", /* drain XP flag */
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
    "_AMASSBAN",	/* ?Mass Banishment */
    "_ASHROOM",		/* Number of cool mushrooms */
    "_AROD1",		/* Attack rods */
    "_AROD2",		/* Attack rods */
    "_DINV",        /* See Inv Spell is Legal */
    "_WEIGHT",
    NULL
};

/*
 * Some variables
 */

bool borg_active;       /* Actually active */
bool borg_resurrect = false;    /* continous play mode */

bool borg_cancel;       /* Being cancelled */

char genocide_target;   /* identity of the poor unsuspecting soul */
int zap_slot;                  /* slot of a wand/staff---to avoid a game bug*/
bool borg_casted_glyph;        /* because we dont have a launch anymore */
int borg_stop_dlevel = -1;
int borg_stop_clevel = -1;
int borg_no_deeper = 127;
bool borg_stop_king = true;
bool borg_dont_react = false;
int successful_target = 0;
int sold_item_tval[10];
int sold_item_sval[10];
int sold_item_pval[10];
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

bool borg_flag_save = false;    /* Save savefile at each level */
bool borg_flag_dump = false;    /* Save savefile at each death */
bool borg_save = false;        /* do a save next level */
bool borg_graphics = false;    /* rr9's graphics */
bool borg_confirm_target = false; /* emergency spell use */
bool borg_scumming_pots = true;	/* Borg will quickly store pots in home */

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
bool need_shift_panel = false;    /* to spot offscreens */
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


bool breeder_level = false;          /* Borg will shut door */
s16b old_depth = 128;
s16b borg_respawning = 0;
s16b borg_no_retreat = 0;

/*
 * Hack -- Other time variables
 */

s16b when_call_LIGHT;        /* When we last did call light */
s16b when_wizard_LIGHT;      /* When we last did wizard light */

s16b when_detect_traps;     /* When we last detected traps */
s16b when_detect_doors;     /* When we last detected doors */
s16b when_detect_walls;     /* When we last detected walls */
s16b when_detect_evil;      /* When we last detected monsters or evil */
s16b when_detect_obj;      /* When we last detected objects */
s16b when_last_kill_mult = 0;   /* When a multiplier was last killed */

bool my_need_alter;        /* incase i hit a wall or door */
bool my_no_alter;          /*  */
bool my_need_redraw;        /* incase i hit a wall or door */
bool borg_attempting_refresh_resist = false;  /* for the Resistance spell */

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
bool borg_fastcast;
bool borg_regen;
bool borg_smite_evil;
bool borg_venom;
s16b borg_game_ratio;  /* the ratio of borg time to game time */
s16b borg_resistance;  /* borg is Resistant to all elements */
s16b borg_no_rest_prep; /* borg wont rest for a few turns */

bool borg_shield;
bool borg_on_glyph;    /* borg is standing on a glyph of warding */
bool borg_create_door;    /* borg is going to create doors */
bool borg_sleep_spell;
bool borg_sleep_spell_ii;
bool borg_crush_spell;
bool borg_slow_spell;  /* borg is about to cast the spell */
bool borg_confuse_spell;
bool borg_fear_mon_spell;

/*
 * Current shopping information
 */

bool borg_in_shop = false;  /* True if the borg is inside of a store */
s16b goal_shop = -1;        /* Next shop to visit */
s16b goal_ware = -1;        /* Next item to buy there */
s16b goal_item = -1;        /* Next item to sell there */
int borg_food_onsale = -1;      /* Are shops selling food? */
int borg_fuel_onsale = -1;      /* Are shops selling fuel? */
bool borg_needs_quick_shopping = false; /* Needs to buy without browsing all shops */
s16b borg_best_fit_item = -1;	/* Item to be worn.  Index used to note which item not to sell */
int borg_best_item = -1;  /* Attempting to wear a best fit item */

char* shop_menu_items = "acfhjmnoqruvyzABDFGHJKLMNOPQRSTUVWXYZ";

byte borg_nasties_num = 7;	/* Current size of the list */
byte borg_nasties_count[7];
char borg_nasties[7] = "ZAVULWD"; /* Order of Nastiness.  Hounds < Demons < Wyrms */
byte borg_nasties_limit[7] =
{ 20, 20, 10, 10, 10, 10, 10 };

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

/*
 * Some estimated state variables
 */

s16b my_stat_max[STAT_MAX];    /* Current "maximal" stat values */
s16b my_stat_cur[STAT_MAX];    /* Current "natural" stat values */
s16b my_stat_ind[STAT_MAX];    /* Current "additions" to stat values */
bool my_need_stat_check[STAT_MAX];  /* do I need to check my stats? */

s16b my_stat_add[STAT_MAX];  /* additions to stats  This will allow upgrading of */
/* equiptment to allow a ring of int +4 to be traded */
/* for a ring of int +6 even if maximized to allow a */
/* later swap to be better. */

s16b home_stat_add[STAT_MAX];

int weapon_swap;    /* location of my swap weapon */
int armour_swap;    /* my swap of armour */

bool decurse_weapon_swap;  /* my swap is great, except its cursed */
int enchant_weapon_swap_to_h;  /* my swap is great, except its cursed */
int enchant_weapon_swap_to_d;  /* my swap is great, except its cursed */
bool decurse_armour_swap;  /* my swap is great, except its cursed */
int enchant_armour_swap_to_a;  /* my swap is great, except its cursed */

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
byte weapon_swap_impact;
byte weapon_swap_brand_acid;
byte weapon_swap_brand_elec;
byte weapon_swap_brand_fire;
byte weapon_swap_brand_cold;
byte weapon_swap_brand_pois;
byte weapon_swap_see_infra;
byte weapon_swap_slow_digest;
byte weapon_swap_aggravate;
byte weapon_swap_bad_curse;
byte weapon_swap_regenerate;
byte weapon_swap_telepathy;
byte weapon_swap_light;
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
byte weapon_swap_resist_light;
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
byte armour_swap_impact;
byte armour_swap_brand_acid;
byte armour_swap_brand_elec;
byte armour_swap_brand_fire;
byte armour_swap_brand_cold;
byte armour_swap_brand_pois;
byte armour_swap_see_infra;
byte armour_swap_slow_digest;
byte armour_swap_aggravate;
byte armour_swap_bad_curse;
byte armour_swap_regenerate;
byte armour_swap_telepathy;
byte armour_swap_light;
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

int borg_feeling_danger = 0;  /* Current level "feeling" */
int borg_feeling_stuff = 0;   /* Current level "feeling" */

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
 * Number of turns to step for (zero means forever)
 */
u16b borg_step = 0;     /* Step count (if any) */


/*
 * Status message search string
 */
char borg_match[128] = "plain gold ring";  /* Search string */



/*
 * Hack -- the detection arrays
 */

bool borg_detect_wall[6][18];

bool borg_detect_trap[6][18];

bool borg_detect_door[6][18];

bool borg_detect_evil[6][18];

bool borg_detect_obj[6][18];

/*
 * Locate the store doors
 */

int* track_shop_x;
int* track_shop_y;


/*
 * Track "stairs up"
 */

s16b track_less_num;
s16b track_less_size;
int* track_less_x;
int* track_less_y;


/*
 * Track "stairs down"
 */

s16b track_more_num;
s16b track_more_size;
int* track_more_x;
int* track_more_y;

/*
 * Track glyphs
 */
s16b track_glyph_num;
s16b track_glyph_size;
int* track_glyph_x;
int* track_glyph_y;

bool borg_needs_new_sea; /* Environment changed.  Need to make a new Sea of Runes for Morgy */

/*
 * Track the items worn to avoid loops
 */
s16b track_worn_num;
s16b track_worn_size;
byte* track_worn_name1;
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
{ 0, 0, 1, -1, 1, -1, 1, -1, 2, 2, 2, -2, -2, -2, -2, -1, 0, 1, 2, -2, -1, 0, 1, 2 };
const s16b borg_ddy_ddd[24] =
{ 1, -1, 0, 0, 1, 1, -1, -1, -1, 0, 1, -1, 0, 1, -2, -2, -2, -2, -2, 2, 2, 2, 2, 2 };

/*
 * Track Steps
 */
s16b track_step_num;
s16b track_step_size;
int* track_step_x;
int* track_step_y;

/*
 * Track closed doors which I have closed
 */
s16b track_door_num;
s16b track_door_size;
int* track_door_x;
int* track_door_y;

/*
 * Track closed doors which started closed
 */
s16b track_closed_num;
s16b track_closed_size;
int* track_closed_x;
int* track_closed_y;

/*
 * Track the mineral veins with treasure
 *
 */
s16b track_vein_num;
s16b track_vein_size;
int* track_vein_x;
int* track_vein_y;

/*
 * The object list.  This list is used to "track" objects.
 */

s16b borg_takes_cnt;
s16b borg_takes_nxt;
borg_take* borg_takes;


/*
 * The monster list.  This list is used to "track" monsters.
 */

s16b borg_kills_cnt;
s16b borg_kills_summoner;    /* index of a summoner */
s16b borg_kills_nxt;

borg_kill* borg_kills;


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
u16b borg_fear_monsters[AUTO_MAX_Y + 1][AUTO_MAX_X + 1];

/*
 * Hack -- count racial appearances per level
 */
s16b* borg_race_count;


/*
 * Hack -- count racial kills (for uniques)
 */

s16b* borg_race_death;

/*
 * The current map
 */

borg_grid* borg_grids[AUTO_MAX_Y];  /* The grids */


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

borg_data* borg_data_flow;  /* Current "flow" data */

borg_data* borg_data_cost;  /* Current "cost" data */

borg_data* borg_data_hard;  /* Constant "hard" data */

borg_data* borg_data_know;  /* Current "know" flags */

borg_data* borg_data_icky;  /* Current "icky" flags */



/*
 * Strategy flags -- recalculate things
 */

bool borg_danger_wipe = false;  /* Recalculate danger */

bool borg_do_update_view = false;  /* Recalculate view */

bool borg_do_update_LIGHT = false;  /* Recalculate lite */

/*
  * Strategy flags -- examine the world
  */

bool borg_do_inven = true;  /* Acquire "inven" info */
bool borg_do_equip = true;  /* Acquire "equip" info */
bool borg_do_panel = true;  /* Acquire "panel" info */
bool borg_do_frame = true;  /* Acquire "frame" info */
bool borg_do_spell = true;  /* Acquire "spell" info */
byte borg_do_spell_aux = 0; /* Hack -- book for "borg_do_spell" */
bool borg_do_browse = 0;    /* Acquire "store" info */
byte borg_do_browse_what = 0;   /* Hack -- store for "borg_do_browse" */
byte borg_do_browse_more = 0;   /* Hack -- pages for "borg_do_browse" */


/*
 * Strategy flags -- run certain functions
 */
bool borg_do_crush_junk = false;
bool borg_do_crush_hole = false;
bool borg_do_crush_slow = false;

/* am I fighting a unique? */
int borg_fighting_unique;
bool borg_fighting_evil_unique;		/* Need to know if evil for Priest Banishment */

/* am I fighting a summoner? */
bool borg_fighting_summoner;

/**
 * Stat Table (INT/WIS) -- Minimum failure rate (percentage)
 */
const int borg_adj_mag_fail[STAT_RANGE] =
{
    99	/* 3 */,
    99	/* 4 */,
    99	/* 5 */,
    99	/* 6 */,
    99	/* 7 */,
    50	/* 8 */,
    30	/* 9 */,
    20	/* 10 */,
    15	/* 11 */,
    12	/* 12 */,
    11	/* 13 */,
    10	/* 14 */,
    9	/* 15 */,
    8	/* 16 */,
    7	/* 17 */,
    6	/* 18/00-18/09 */,
    6	/* 18/10-18/19 */,
    5	/* 18/20-18/29 */,
    5	/* 18/30-18/39 */,
    5	/* 18/40-18/49 */,
    4	/* 18/50-18/59 */,
    4	/* 18/60-18/69 */,
    4	/* 18/70-18/79 */,
    4	/* 18/80-18/89 */,
    3	/* 18/90-18/99 */,
    3	/* 18/100-18/109 */,
    2	/* 18/110-18/119 */,
    2	/* 18/120-18/129 */,
    2	/* 18/130-18/139 */,
    2	/* 18/140-18/149 */,
    1	/* 18/150-18/159 */,
    1	/* 18/160-18/169 */,
    1	/* 18/170-18/179 */,
    1	/* 18/180-18/189 */,
    1	/* 18/190-18/199 */,
    0	/* 18/200-18/209 */,
    0	/* 18/210-18/219 */,
    0	/* 18/220+ */
};

/**
 * Stat Table (INT/WIS) -- failure rate adjustment
 */
const int borg_adj_mag_stat[STAT_RANGE] =
{
    -5	/* 3 */,
    -4	/* 4 */,
    -3	/* 5 */,
    -3	/* 6 */,
    -2	/* 7 */,
    -1	/* 8 */,
     0	/* 9 */,
     0	/* 10 */,
     0	/* 11 */,
     0	/* 12 */,
     0	/* 13 */,
     1	/* 14 */,
     2	/* 15 */,
     3	/* 16 */,
     4	/* 17 */,
     5	/* 18/00-18/09 */,
     6	/* 18/10-18/19 */,
     7	/* 18/20-18/29 */,
     8	/* 18/30-18/39 */,
     9	/* 18/40-18/49 */,
    10	/* 18/50-18/59 */,
    11	/* 18/60-18/69 */,
    12	/* 18/70-18/79 */,
    15	/* 18/80-18/89 */,
    18	/* 18/90-18/99 */,
    21	/* 18/100-18/109 */,
    24	/* 18/110-18/119 */,
    27	/* 18/120-18/129 */,
    30	/* 18/130-18/139 */,
    33	/* 18/140-18/149 */,
    36	/* 18/150-18/159 */,
    39	/* 18/160-18/169 */,
    42	/* 18/170-18/179 */,
    45	/* 18/180-18/189 */,
    48	/* 18/190-18/199 */,
    51	/* 18/200-18/209 */,
    54	/* 18/210-18/219 */,
    57	/* 18/220+ */
};

/*
 * Calculate "incremental motion". Used by project() and shoot().
 * Assumes that (*y,*x) lies on the path from (y1,x1) to (y2,x2).
 */
 /* changing this to be more like project_path */
 /* note that this is much slower but much more accurate */
void mmove2(int* py, int* px, int y1, int x1, int y2, int x2)
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
            *py += sy;
            return;
        }
        else if (dx > dy)
        {
            *px += sx;
            return;
        }
        else
        {
            *px += sx;
            *py += sy;
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
                k = 1;

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
errr borg_what_char(int x, int y, byte* a, wchar_t* c)
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
errr borg_what_text(int x, int y, int n, byte* a, char* s)
{
    int i;
    wchar_t screen_str[1024];

    int t_a;
    wchar_t t_c;

    int* aa;
    wchar_t* cc;

    int w, h;

    /* Current attribute */
    int d_a = 0;

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
    wcstombs(s, screen_str, ABS(n) + 1);
    /* Too short */
    if ((n > 0) && (i != n)) return (1);

    /* Success */
    return (0);
}



/*
 * Log a message to a file
 */
void borg_info(const char* what)
{

}



/*
 * Memorize a message, Log it, Search it, and Display it in pieces
 */
void borg_note(const char* what)
{
    int j, n, i, k;

    int w, h, x, y;


    term* old = Term;

    /* Memorize it */
    message_add(what, MSG_GENERIC);


    /* Log the message */
    borg_info(what);


    /* Mega-Hack -- Check against the search string */
    if (borg_match[0] && strstr(what, borg_match))
    {
        /* Clean cancel */
        borg_cancel = true;
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
        if (!(window_flag[j] & PW_BORG_1)) continue;

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
                Term_addstr(-1, COLOUR_WHITE, buf);

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
            Term_addstr(-1, COLOUR_WHITE, what);

            /* Advance (wrap) */
            if (++y >= h) y = 0;

            /* Erase next line */
            Term_erase(0, y, 255);
        }

        /* Normal */
        else
        {
            /* Show message */
            Term_addstr(-1, COLOUR_WHITE, what);

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
void borg_oops(const char* what)
{
    /* Stop processing */
    borg_active = false;

    /* Give a warning */
    borg_note(format("# Aborting (%s).", what));

    /* Forget borg keys */
    borg_flush();
}

/*
 * A Queue of keypresses to be sent
 */
static keycode_t* borg_key_queue;
static s16b borg_key_head;
static s16b borg_key_tail;

/*
 * since the code now only asks for a direction if
 * the instruction is ambiguous and it is hard for
 * the borg to know if it is asking for something
 * ambiguous, it queues up a direction in case it is
 * requested.
 */
static keycode_t borg_queued_direction = 0;

/*
 * Add a keypress to the "queue" (fake event)
 */
errr borg_keypress(keycode_t k)
{
    /* Hack -- Refuse to enqueue "nul" */
    if (!k)
    {
        borg_note(" & Key * *BAD KEY * *");
        return (-1);
    }

    /* Hack -- note the keypress */
    if (k >= 32 && k <= 126) {
        borg_note(format("& Key <%c> (0x%02X)", k, k));
    }
    else {
        borg_note(format("& Key <0x%02X>", k));
    }

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
errr borg_keypresses(const char* str)
{
    const char* s;

    /* Enqueue them */
    for (s = str; *s; s++) borg_keypress(*s);

    /* Success */
    return (0);
}


/*
 * Get the next Borg keypress
 */
keycode_t borg_inkey(bool take)
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

    borg_queued_direction = 0;
}

/*
*  Save and retrieve direction when the command may be ambiguous.
*  Watch for "Direction or <click> (Escape to cancel)?"
*/
void borg_queue_direction(keycode_t k)
{
    borg_queued_direction = k;
    borg_confirm_target = true;
}

keycode_t borg_get_queued_direction(void)
{
    keycode_t k = borg_queued_direction;
    borg_queued_direction = 0;
    return k;
}


/*
 * Hack -- take a note later
 */
bool borg_tell(char* what)
{
    /* Hack -- self note */
    borg_keypress(':');
    borg_keypresses(what);
    borg_keypress(KC_ENTER);

    /* Success */
    return (true);
}



/*
 * Attempt to change the borg's name
 */
bool borg_change_name(char* str)
{
    /* Cancel everything */
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);

    /* Character description */
    borg_keypress('C');

    /* Change the name */
    borg_keypress('c');

    /* Enter the new name */
    borg_keypresses(str);

    /* End the name */
    borg_keypress(KC_ENTER);

    /* Cancel everything */
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);

    /* Success */
    return (true);
}


/*
 * Attempt to dump a character description file
 */
bool borg_dump_character(char* str)
{
    /* Cancel everything */
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);

    /* Character description */
    borg_keypress('C');

    /* Dump character file */
    borg_keypress('f');

    /* Enter the new name */
    borg_keypresses(str);

    /* End the file name */
    borg_keypress(KC_ENTER);

    /* Cancel everything */
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);

    /* Success */
    return (true);
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
    return (true);
}




/*
 * Update the Borg based on the current "frame"
 *
 * Assumes the Borg is actually in the dungeon.
 */
void borg_update_frame(void)
{
    int i;

    struct player_state* state = &player->known_state;

    /* Assume level is fine */
    borg_skill[BI_ISFIXLEV] = false;

    /* Note "Lev" vs "LEV" */
    if (player->lev < player->max_lev) borg_skill[BI_ISFIXLEV] = true;

    /* Extract "LEVEL xxxxxx" */
    borg_skill[BI_CLEVEL] = player->lev;

    /* cheat the max clevel */
    borg_skill[BI_MAXCLEVEL] = player->max_lev;

    /* Note "Winner" */
    borg_skill[BI_KING] = player->total_winner;

    /* Assume experience is fine */
    borg_skill[BI_ISFIXEXP] = false;

    /* Note "Exp" vs "EXP" and am I lower than level 50*/
    if (player->exp < player->max_exp)
    {
        /* fix it if in town */
        if (borg_skill[BI_CLEVEL] == 50 && borg_skill[BI_CDEPTH] == 0) borg_skill[BI_ISFIXEXP] = true;

        /* dont worry about fixing it in the dungeon */
        if (borg_skill[BI_CLEVEL] == 50 && borg_skill[BI_CDEPTH] >= 1) borg_skill[BI_ISFIXEXP] = false;

        /* Not at Max Level */
        if (borg_skill[BI_CLEVEL] != 50) borg_skill[BI_ISFIXEXP] = true;
    }

    /* Extract "EXP xxxxxxxx" */
    borg_exp = player->exp;


    /* Extract "AU xxxxxxxxx" */
    borg_gold = player->au;

    borg_skill[BI_WEIGHT] = player->upkeep->total_weight;

    /* Extract "Fast (+x)" or "Slow (-x)" */
    borg_skill[BI_SPEED] = state->speed;

    /* Check my float for decrementing variables */
    if (borg_skill[BI_SPEED] > 110)
    {
        borg_game_ratio = 100000 / (((borg_skill[BI_SPEED] - 110) * 10) + 100);
    }
    else
    {
        borg_game_ratio = 1000;
    }


    /* A quick cheat to see if I missed a message about my status on some timed spells */
    if (!goal_recalling && player->word_recall) goal_recalling = true;
    if (!borg_prot_from_evil && player->timed[TMD_PROTEVIL]) borg_prot_from_evil = (player->timed[TMD_PROTEVIL] ? true : false);
    if (!borg_speed && (player->timed[TMD_FAST] || player->timed[TMD_SPRINT] || player->timed[TMD_TERROR]))
        (borg_speed = (player->timed[TMD_FAST] || player->timed[TMD_SPRINT] || player->timed[TMD_TERROR]) ? true : false);
    borg_skill[BI_TRACID] = (player->timed[TMD_OPP_ACID] ? true : false);
    borg_skill[BI_TRELEC] = (player->timed[TMD_OPP_ELEC] ? true : false);
    borg_skill[BI_TRFIRE] = (player->timed[TMD_OPP_FIRE] ? true : false);
    borg_skill[BI_TRCOLD] = (player->timed[TMD_OPP_COLD] ? true : false);
    borg_skill[BI_TRPOIS] = (player->timed[TMD_OPP_POIS] ? true : false);
    borg_bless = (player->timed[TMD_BLESSED] ? true : false);
    borg_shield = (player->timed[TMD_SHIELD] ? true : false);
    borg_shield = (player->timed[TMD_STONESKIN] ? true : false);
    borg_fastcast = (player->timed[TMD_FASTCAST] ? true : false);
    borg_hero = (player->timed[TMD_HERO] ? true : false);
    borg_berserk = (player->timed[TMD_SHERO] ? true : false);
    borg_regen = (player->timed[TMD_HEAL] ? true : false);
    borg_venom = (player->timed[TMD_ATT_POIS] ? true : false);
    borg_smite_evil = (player->timed[TMD_ATT_EVIL] ? true : false);


    /* if hasting, it doesn't count as 'borg_speed'.  The speed */
    /* gained from hasting is counted seperately. */
    if (borg_speed)
    {
        if (player->timed[TMD_FAST] || player->timed[TMD_SPRINT]) borg_skill[BI_SPEED] -= 10;
        else if (player->timed[TMD_TERROR]) borg_skill[BI_SPEED] -= 5;
    }

    /* Extract "Cur AC xxxxx" */
    borg_skill[BI_ARMOR] = state->ac + state->to_a;

    /* Extract "Cur HP xxxxx" */
    borg_skill[BI_CURHP] = player->chp;

    /* Extract "Max HP xxxxx" */
    borg_skill[BI_MAXHP] = player->mhp;

    /* Extract "Cur SP xxxxx" (or zero) */
    borg_skill[BI_CURSP] = player->csp;

    /* Extract "Max SP xxxxx" (or zero) */
    borg_skill[BI_MAXSP] = player->msp;

    /* Clear all the "state flags" */
    borg_skill[BI_ISWEAK] = borg_skill[BI_ISHUNGRY] = borg_skill[BI_ISFULL] = borg_skill[BI_ISGORGED] = false;
    borg_skill[BI_ISBLIND] = borg_skill[BI_ISCONFUSED] = borg_skill[BI_ISAFRAID] = borg_skill[BI_ISPOISONED] = false;
    borg_skill[BI_ISCUT] = borg_skill[BI_ISSTUN] = borg_skill[BI_ISHEAVYSTUN] = borg_skill[BI_ISIMAGE] = borg_skill[BI_ISSTUDY] = false;
    borg_skill[BI_ISPARALYZED] = false;
    borg_skill[BI_ISFORGET] = false;

    /* Check for "Weak" */
    if (player->timed[TMD_FOOD] < PY_FOOD_WEAK) borg_skill[BI_ISWEAK] = borg_skill[BI_ISHUNGRY] = true;

    /* Check for "Hungry" */
    else if (player->timed[TMD_FOOD] < PY_FOOD_HUNGRY) borg_skill[BI_ISHUNGRY] = true;

    /* Check for "Normal" */
    else if (player->timed[TMD_FOOD] < PY_FOOD_FULL) /* Nothing */;

    /* Check for "Full" */
    else if (player->timed[TMD_FOOD] < PY_FOOD_MAX) borg_skill[BI_ISFULL] = true;

    /* Check for "Gorged" */
    else borg_skill[BI_ISGORGED] = borg_skill[BI_ISFULL] = true;

    /* Check for "Blind" */
    if (player->timed[TMD_BLIND]) borg_skill[BI_ISBLIND] = true;

    /* Check for "Confused" */
    if (player->timed[TMD_CONFUSED]) borg_skill[BI_ISCONFUSED] = true;

    /* Check for "Afraid" */
    if (player->timed[TMD_AFRAID]) borg_skill[BI_ISAFRAID] = true;

    /* Check for "Poisoned" */
    if (player->timed[TMD_POISONED]) borg_skill[BI_ISPOISONED] = true;

    /* Check for any text */
    if (player->timed[TMD_CUT]) borg_skill[BI_ISCUT] = true;

    /* Check for Stun */
    if (player->timed[TMD_STUN] && (player->timed[TMD_STUN] <= 50)) borg_skill[BI_ISSTUN] = true;

    /* Check for Heavy Stun */
    if (player->timed[TMD_STUN] > 50) borg_skill[BI_ISHEAVYSTUN] = true;

    /* Check for Paralyze */
    if (player->timed[TMD_PARALYZED] > 50) borg_skill[BI_ISPARALYZED] = true;

    /* Check for "Hallucinating" */
    if (player->timed[TMD_IMAGE]) borg_skill[BI_ISIMAGE] = true;

    /* Check for "Amnesia" */
    if (player->timed[TMD_AMNESIA]) borg_skill[BI_ISFORGET] = true;

    /* Check to BLESS */
    borg_bless = (player->timed[TMD_BLESSED] ? true : false);

    /* Check for "Study" */
    if (player->upkeep->new_spells) borg_skill[BI_ISSTUDY] = true;


    /* Parse stats */
    for (i = 0; i < 5; i++)
    {
        borg_skill[BI_ISFIXSTR + i] = player->stat_cur[STAT_STR + i] < player->stat_max[STAT_STR + i];
        borg_skill[BI_CSTR + i] = player->stat_cur[STAT_STR + i];
        borg_stat[i] = player->stat_cur[i];

    }

    /* Hack -- Access max depth */
    borg_skill[BI_CDEPTH] = player->depth;

    /* Hack -- Access max depth */
    borg_skill[BI_MAXDEPTH] = player->max_depth;

}


int
borg_check_formula(int* arg_formula)
{
    int     oper1;          /* operand #1 */
    int     oper2;          /* operand #2 */
    int     stack[256];     /* stack */
    int* stackptr;      /* stack pointer */

    /* loop until we hit BFO_DONE */
    for (stackptr = stack; *arg_formula; arg_formula++)
    {
        if (stackptr < stack)
            return 0;
        switch (*arg_formula)
        {
            /* Number */
        case BFO_NUMBER:
        *stackptr++ = *++arg_formula;
        break;

        /* Variable */
        case BFO_VARIABLE:
        *stackptr++ = borg_has[*++arg_formula];
        if ((*arg_formula) > (z_info->k_max + BI_MAX))
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

    if (stackptr != (stack + 1))
        return 0;
    return 1;
}

char* borg_prt_formula(int* arg_formula)
{
    static char FormulaStr[2000];
    char tmpStr[50];

    memset(FormulaStr, 0, sizeof(FormulaStr));
    /* loop until we hit BFO_DONE */
    for (; *arg_formula; arg_formula++)
    {
        switch (*arg_formula)
        {
            /* Number */
        case BFO_NUMBER:

        sprintf(tmpStr, "%d ", *++arg_formula);
        strcat(FormulaStr, tmpStr);
        break;

        /* Variable */
        case BFO_VARIABLE:
        strcat(FormulaStr, "'");
        strcat(FormulaStr, borg_prt_item(*++arg_formula));
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
borg_calc_formula(int* arg_formula)
{
    int     oper1;          /* operand #1 */
    int     oper2;          /* operand #2 */
    int     stack[256];     /* stack */
    int* stackptr;      /* stack pointer */


    if (!arg_formula)
        return 0;

    *stack = 0;
    /* loop until we hit BFO_DONE */
    for (stackptr = stack; *arg_formula; arg_formula++)
    {
        switch (*arg_formula)
        {
            /* Number */
        case BFO_NUMBER:
        *stackptr++ = *++arg_formula;
        break;

        /* Variable */
        case BFO_VARIABLE:
        *stackptr++ = borg_has[*++arg_formula];
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
static void borg_sort_aux(void* u, void* v, int p, int q)
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
    while (true)
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
    borg_sort_aux(u, v, b + 1, q);
}


/*
 * Borg's sorting algorithm -- quick sort in place
 *
 * Note that the details of the data we are sorting is hidden,
 * and we rely on the "ang_sort_comp()" and "ang_sort_swap()"
 * function hooks to interact with the data, which is given as
 * two pointers, and which may have any user-defined form.
 */
void borg_sort(void* u, void* v, int n)
{
    /* Sort the array */
    borg_sort_aux(u, v, 0, n - 1);
}

/*
 * Sorting hook -- comp function -- see below
 *
 * We use "u" to point to an array of strings, and "v" to point to
 * an array of indexes, and we sort them together by the strings.
 */
bool borg_sort_comp_hook(void* u, void* v, int a, int b)
{
    char** text = (char**)(u);
    s16b* what = (s16b*)(v);

    int cmp;

    /* Compare the two strings */
    cmp = (strcmp(text[a], text[b]));

    /* Strictly less */
    if (cmp < 0) return (true);

    /* Strictly more */
    if (cmp > 0) return (false);

    /* Enforce "stable" sort */
    return (what[a] <= what[b]);
}

/*
 * Sorting hook -- swap function -- see below
 *
 * We use "u" to point to an array of strings, and "v" to point to
 * an array of indexes, and we sort them together by the strings.
 */
void borg_sort_swap_hook(void* u, void* v, int a, int b)
{
    char** text = (char**)(u);
    s16b* what = (s16b*)(v);

    char* texttmp;
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

int sv_food_ration;
int sv_food_slime_mold;
int sv_food_draught;
int sv_food_sip;
int sv_food_waybread;
int sv_food_honey_cake;
int sv_food_slice;
int sv_food_handful;

int sv_mush_second_sight;
int sv_mush_fast_recovery;
int sv_mush_restoring;
int sv_mush_mana;
int sv_mush_emergency;
int sv_mush_terror;
int sv_mush_stoneskin;
int kv_mush_stoneskin;
int sv_mush_debility;
int sv_mush_sprinting;
int sv_mush_cure_mind;
int sv_mush_purging;

int sv_light_lantern;
int sv_light_torch;

int sv_flask_oil;
int kv_flask_oil;

int sv_potion_cure_critical;
int sv_potion_cure_serious;
int sv_potion_cure_light;
int sv_potion_healing;
int kv_potion_healing;
int sv_potion_star_healing;
int sv_potion_life;
int sv_potion_restore_mana;
int kv_potion_restore_mana;
int sv_potion_cure_poison;
int sv_potion_resist_heat;
int sv_potion_resist_cold;
int sv_potion_resist_pois;
int sv_potion_inc_str;
int sv_potion_inc_int;
int sv_potion_inc_wis;
int sv_potion_inc_dex;
int sv_potion_inc_con;
int sv_potion_inc_str2;
int sv_potion_inc_int2;
int sv_potion_inc_wis2;
int sv_potion_inc_dex2;
int sv_potion_inc_con2;
int sv_potion_inc_all;
int sv_potion_restore_life;
int sv_potion_speed;
int sv_potion_berserk;
int sv_potion_sleep;
int sv_potion_slowness;
int sv_potion_poison;
int sv_potion_blindness;
int sv_potion_confusion;
int sv_potion_heroism;
int sv_potion_boldness;
int sv_potion_detect_invis;
int sv_potion_enlightenment;
int sv_potion_slime_mold;
int sv_potion_berserk;
int sv_potion_infravision;
int sv_potion_inc_exp;

int sv_scroll_identify;
int sv_scroll_phase_door;
int sv_scroll_teleport;
int sv_scroll_word_of_recall;
int sv_scroll_enchant_armor;
int sv_scroll_enchant_weapon_to_hit;
int sv_scroll_enchant_weapon_to_dam;
int sv_scroll_star_enchant_weapon;
int sv_scroll_star_enchant_armor;
int sv_scroll_protection_from_evil;
int sv_scroll_rune_of_protection;
int sv_scroll_teleport_level;
int sv_scroll_recharging;
int sv_scroll_banishment;
int sv_scroll_mass_banishment;
int kv_scroll_mass_banishment;
int sv_scroll_blessing;
int sv_scroll_holy_chant;
int sv_scroll_holy_prayer;
int sv_scroll_detect_invis;
int sv_scroll_satisfy_hunger;
int sv_scroll_light;
int sv_scroll_mapping;
int sv_scroll_acquirement;
int sv_scroll_star_acquirement;
int sv_scroll_remove_curse;
int kv_scroll_remove_curse;
int sv_scroll_star_remove_curse;
int kv_scroll_star_remove_curse;
int sv_scroll_monster_confusion;
int sv_scroll_trap_door_destruction;
int sv_scroll_dispel_undead;

int sv_ring_flames;
int sv_ring_ice;
int sv_ring_acid;
int sv_ring_lightning;
int sv_ring_digging;
int sv_ring_speed;
int sv_ring_damage;

int sv_amulet_teleportation;

int sv_rod_recall;
int kv_rod_recall;
int sv_rod_detection;
int sv_rod_illumination;
int sv_rod_speed;
int sv_rod_mapping;
int sv_rod_healing;
int kv_rod_healing;
int sv_rod_light;
int sv_rod_fire_bolt;
int sv_rod_elec_bolt;
int sv_rod_cold_bolt;
int sv_rod_acid_bolt;
int sv_rod_drain_life;
int sv_rod_fire_ball;
int sv_rod_elec_ball;
int sv_rod_cold_ball;
int sv_rod_acid_ball;
int sv_rod_teleport_other;
int sv_rod_slow_monster;
int sv_rod_sleep_monster;
int sv_rod_curing;

int sv_staff_teleportation;
int sv_staff_destruction;
int sv_staff_speed;
int sv_staff_healing;
int sv_staff_the_magi;
int sv_staff_power;
int sv_staff_curing;
int sv_staff_holiness;
int kv_staff_holiness;
int sv_staff_sleep_monsters;
int sv_staff_slow_monsters;
int sv_staff_detect_invis;
int sv_staff_detect_evil;
int sv_staff_dispel_evil;
int sv_staff_banishment;
int sv_staff_light;
int sv_staff_mapping;
int sv_staff_remove_curse;

int sv_wand_light;
int sv_wand_teleport_away;
int sv_wand_stinking_cloud;
int kv_wand_stinking_cloud;
int sv_wand_magic_missile;
int kv_wand_magic_missile;
int sv_wand_annihilation;
int kv_wand_annihilation;
int sv_wand_stone_to_mud;
int sv_wand_wonder;
int sv_wand_slow_monster;
int sv_wand_hold_monster;
int sv_wand_fear_monster;
int sv_wand_confuse_monster;
int sv_wand_fire_bolt;
int sv_wand_cold_bolt;
int sv_wand_acid_bolt;
int sv_wand_elec_bolt;
int sv_wand_fire_ball;
int sv_wand_cold_ball;
int sv_wand_acid_ball;
int sv_wand_elec_ball;
int sv_wand_dragon_cold;
int sv_wand_dragon_fire;
int sv_wand_drain_life;

int sv_dagger;

int sv_sling;
int sv_short_bow;
int sv_long_bow;
int sv_light_xbow;
int sv_heavy_xbow;

int sv_arrow_seeker;
int sv_arrow_mithril;

int sv_bolt_seeker;
int sv_bolt_mithril;

int sv_set_of_leather_gloves;

int sv_cloak;

int sv_robe;

int sv_iron_crown;

int sv_dragon_blue;
int sv_dragon_black;
int sv_dragon_white;
int sv_dragon_red;
int sv_dragon_green;
int sv_dragon_multihued;
int sv_dragon_shining;
int sv_dragon_law;
int sv_dragon_gold;
int sv_dragon_chaos;
int sv_dragon_balance;
int sv_dragon_power;

/* a helper to make sure our definitions are correct */
static int borg_lookup_sval_fail(int tval, const char* name)
{
    int sval = lookup_sval(tval, name);
    if (sval == -1)
    {
        borg_note(format("**STARTUP FAILURE** sval lookup failure ", name));
        borg_init_failure = true;
    }
    return sval;
}

void borg_init_svs_and_kvs(void)
{
    int tval = tval_find_idx("food");
    sv_food_ration = borg_lookup_sval_fail(tval, "Ration of Food");
    sv_food_slime_mold = borg_lookup_sval_fail(tval, "Slime Mold");
    sv_food_draught = borg_lookup_sval_fail(tval, "Draught of the Ents");
    sv_food_sip = borg_lookup_sval_fail(tval, "Sip of Miruvor");
    sv_food_waybread = borg_lookup_sval_fail(tval, "Piece of Elvish Waybread");
    sv_food_honey_cake = borg_lookup_sval_fail(tval, "Honey-cake");
    sv_food_slice = borg_lookup_sval_fail(tval, "Slice of Meat");
    sv_food_handful = borg_lookup_sval_fail(tval, "Handful of Dried Fruits");

    tval = tval_find_idx("mushroom");
    sv_mush_second_sight = borg_lookup_sval_fail(tval, "Second Sight");
    sv_mush_fast_recovery = borg_lookup_sval_fail(tval, "Fast Recovery");
    sv_mush_restoring = borg_lookup_sval_fail(tval, "Vigor");
    sv_mush_mana = borg_lookup_sval_fail(tval, "Clear Mind");
    sv_mush_emergency = borg_lookup_sval_fail(tval, "Emergency");
    sv_mush_terror = borg_lookup_sval_fail(tval, "Terror");
    sv_mush_stoneskin = borg_lookup_sval_fail(tval, "Stoneskin");
    kv_mush_stoneskin = borg_lookup_kind(tval, sv_mush_stoneskin);
    sv_mush_debility = borg_lookup_sval_fail(tval, "Debility");
    sv_mush_sprinting = borg_lookup_sval_fail(tval, "Sprinting");
    sv_mush_cure_mind = borg_lookup_sval_fail(tval, "Clear Mind");
    sv_mush_purging = borg_lookup_sval_fail(tval, "Purging");

    tval = tval_find_idx("light");
    sv_light_lantern = borg_lookup_sval_fail(tval, "Lantern");
    sv_light_torch = borg_lookup_sval_fail(tval, "Wooden Torch");

    tval = tval_find_idx("flask");
    sv_flask_oil = borg_lookup_sval_fail(tval, "Flask of Oil");
    kv_flask_oil = borg_lookup_kind(tval, sv_flask_oil);

    tval = tval_find_idx("potion");
    sv_potion_cure_critical = borg_lookup_sval_fail(tval, "Cure Critical Wounds");
    sv_potion_cure_serious = borg_lookup_sval_fail(tval, "Cure Serious Wounds");
    sv_potion_cure_light = borg_lookup_sval_fail(tval, "Cure Light Wounds");
    sv_potion_healing = borg_lookup_sval_fail(tval, "Healing");
    kv_potion_healing = borg_lookup_kind(tval, sv_potion_healing);
    sv_potion_star_healing = borg_lookup_sval_fail(tval, "*Healing*");
    sv_potion_life = borg_lookup_sval_fail(tval, "Life");
    sv_potion_restore_mana = borg_lookup_sval_fail(tval, "Restore Mana");
    kv_potion_restore_mana = borg_lookup_kind(tval, sv_potion_restore_mana);
    sv_potion_cure_poison = borg_lookup_sval_fail(tval, "Neutralize Poison");
    sv_potion_resist_heat = borg_lookup_sval_fail(tval, "Resist Heat");
    sv_potion_resist_cold = borg_lookup_sval_fail(tval, "Resist Cold");
    sv_potion_resist_pois = borg_lookup_sval_fail(tval, "Resist Poison");
    sv_potion_inc_str = borg_lookup_sval_fail(tval, "Strength");
    sv_potion_inc_int = borg_lookup_sval_fail(tval, "Intelligence");
    sv_potion_inc_wis = borg_lookup_sval_fail(tval, "Wisdom");
    sv_potion_inc_dex = borg_lookup_sval_fail(tval, "Dexterity");
    sv_potion_inc_con = borg_lookup_sval_fail(tval, "Constitution");
    sv_potion_inc_all = borg_lookup_sval_fail(tval, "Augmentation");
    sv_potion_inc_str2 = borg_lookup_sval_fail(tval, "Brawn");
    sv_potion_inc_int2 = borg_lookup_sval_fail(tval, "Intellect");
    sv_potion_inc_wis2 = borg_lookup_sval_fail(tval, "Contemplation");
    sv_potion_inc_dex2 = borg_lookup_sval_fail(tval, "Nimbleness");
    sv_potion_inc_con2 = borg_lookup_sval_fail(tval, "Toughness");
    sv_potion_restore_life = borg_lookup_sval_fail(tval, "Restore Life Levels");
    sv_potion_speed = borg_lookup_sval_fail(tval, "Speed");
    sv_potion_berserk = borg_lookup_sval_fail(tval, "Berserk Strength");
    sv_potion_sleep = borg_lookup_sval_fail(tval, "Sleep");
    sv_potion_slowness = borg_lookup_sval_fail(tval, "Slowness");
    sv_potion_poison = borg_lookup_sval_fail(tval, "Poison");
    sv_potion_blindness = borg_lookup_sval_fail(tval, "Blindness");
    sv_potion_confusion = borg_lookup_sval_fail(tval, "Confusion");
    sv_potion_heroism = borg_lookup_sval_fail(tval, "Heroism");
    sv_potion_boldness = borg_lookup_sval_fail(tval, "Boldness");
    sv_potion_detect_invis = borg_lookup_sval_fail(tval, "True Seeing");
    sv_potion_enlightenment = borg_lookup_sval_fail(tval, "Enlightenment");
    sv_potion_slime_mold = borg_lookup_sval_fail(tval, "Slime Mold Juice");
    sv_potion_berserk = borg_lookup_sval_fail(tval, "Berserk Strength");
    sv_potion_infravision = borg_lookup_sval_fail(tval, "Infravision");
    sv_potion_inc_exp = borg_lookup_sval_fail(tval, "Experience");

    tval = tval_find_idx("scroll");
    sv_scroll_identify = borg_lookup_sval_fail(tval, "Identify Rune");
    sv_scroll_phase_door = borg_lookup_sval_fail(tval, "Phase Door");
    sv_scroll_teleport = borg_lookup_sval_fail(tval, "Teleportation");
    sv_scroll_word_of_recall = borg_lookup_sval_fail(tval, "Word of Recall");
    sv_scroll_enchant_armor = borg_lookup_sval_fail(tval, "Enchant Armour");
    sv_scroll_enchant_weapon_to_hit = borg_lookup_sval_fail(tval, "Enchant Weapon To-Hit");
    sv_scroll_enchant_weapon_to_dam = borg_lookup_sval_fail(tval, "Enchant Weapon To-Dam");
    sv_scroll_star_enchant_armor = borg_lookup_sval_fail(tval, "*Enchant Armour*");
    sv_scroll_star_enchant_weapon = borg_lookup_sval_fail(tval, "*Enchant Weapon*");
    sv_scroll_protection_from_evil = borg_lookup_sval_fail(tval, "Protection from Evil");
    sv_scroll_rune_of_protection = borg_lookup_sval_fail(tval, "Rune of Protection");
    sv_scroll_teleport_level = borg_lookup_sval_fail(tval, "Teleport Level");
    sv_scroll_recharging = borg_lookup_sval_fail(tval, "Recharging");
    sv_scroll_banishment = borg_lookup_sval_fail(tval, "Banishment");
    sv_scroll_mass_banishment = borg_lookup_sval_fail(tval, "Mass Banishment");
    kv_scroll_mass_banishment = borg_lookup_kind(tval, sv_scroll_mass_banishment);
    sv_scroll_blessing = borg_lookup_sval_fail(tval, "Blessing");
    sv_scroll_holy_chant = borg_lookup_sval_fail(tval, "Holy Chant");
    sv_scroll_holy_prayer = borg_lookup_sval_fail(tval, "Holy Prayer");
    sv_scroll_detect_invis = borg_lookup_sval_fail(tval, "Detect Invisible");
    sv_scroll_satisfy_hunger = borg_lookup_sval_fail(tval, "Remove Hunger");
    sv_scroll_light = borg_lookup_sval_fail(tval, "Light");
    sv_scroll_mapping = borg_lookup_sval_fail(tval, "Magic Mapping");
    sv_scroll_acquirement = borg_lookup_sval_fail(tval, "Acquirement");
    sv_scroll_star_acquirement = borg_lookup_sval_fail(tval, "*Acquirement*");
    sv_scroll_remove_curse = borg_lookup_sval_fail(tval, "Remove Curse");
    kv_scroll_remove_curse = borg_lookup_kind(tval, sv_scroll_remove_curse);
    sv_scroll_star_remove_curse = borg_lookup_sval_fail(tval, "*Remove Curse*");
    kv_scroll_star_remove_curse = borg_lookup_kind(tval, sv_scroll_star_remove_curse);
    sv_scroll_monster_confusion = borg_lookup_sval_fail(tval, "Monster Confusion");
    sv_scroll_trap_door_destruction = borg_lookup_sval_fail(tval, "Door Destruction");
    sv_scroll_dispel_undead = borg_lookup_sval_fail(tval, "Dispel Undead");

    tval = tval_find_idx("ring");
    sv_ring_flames = borg_lookup_sval_fail(tval, "Flames");
    sv_ring_ice = borg_lookup_sval_fail(tval, "Ice");
    sv_ring_acid = borg_lookup_sval_fail(tval, "Acid");
    sv_ring_lightning = borg_lookup_sval_fail(tval, "Lightning");
    sv_ring_digging = borg_lookup_sval_fail(tval, "Digging");
    sv_ring_speed = borg_lookup_sval_fail(tval, "Speed");
    sv_ring_damage = borg_lookup_sval_fail(tval, "Damage");

    tval = tval_find_idx("amulet");
    sv_amulet_teleportation = borg_lookup_sval_fail(tval, "Teleportation");

    tval = tval_find_idx("rod");
    sv_rod_recall = borg_lookup_sval_fail(tval, "Recall");
    kv_rod_recall = borg_lookup_kind(tval, sv_rod_recall);
    sv_rod_detection = borg_lookup_sval_fail(tval, "Detection");
    sv_rod_illumination = borg_lookup_sval_fail(tval, "Illumination");
    sv_rod_speed = borg_lookup_sval_fail(tval, "Speed");
    sv_rod_mapping = borg_lookup_sval_fail(tval, "Magic Mapping");
    sv_rod_healing = borg_lookup_sval_fail(tval, "Healing");
    kv_rod_healing = borg_lookup_kind(tval, sv_rod_healing);
    sv_rod_light = borg_lookup_sval_fail(tval, "Light");
    sv_rod_fire_bolt = borg_lookup_sval_fail(tval, "Fire Bolts");
    sv_rod_elec_bolt = borg_lookup_sval_fail(tval, "Lightning Bolts");
    sv_rod_cold_bolt = borg_lookup_sval_fail(tval, "Frost Bolts");
    sv_rod_acid_bolt = borg_lookup_sval_fail(tval, "Acid Bolts");
    sv_rod_drain_life = borg_lookup_sval_fail(tval, "Drain Life");
    sv_rod_fire_ball = borg_lookup_sval_fail(tval, "Fire Balls");
    sv_rod_elec_ball = borg_lookup_sval_fail(tval, "Lightning Balls");
    sv_rod_cold_ball = borg_lookup_sval_fail(tval, "Cold Balls");
    sv_rod_acid_ball = borg_lookup_sval_fail(tval, "Acid Balls");
    sv_rod_teleport_other = borg_lookup_sval_fail(tval, "Teleport Other");
    sv_rod_slow_monster = borg_lookup_sval_fail(tval, "Slow Monster");
    sv_rod_sleep_monster = borg_lookup_sval_fail(tval, "Hold Monster");
    sv_rod_curing = borg_lookup_sval_fail(tval, "Curing");

    tval = tval_find_idx("staff");
    sv_staff_teleportation = borg_lookup_sval_fail(tval, "Teleportation");
    sv_staff_destruction = borg_lookup_sval_fail(tval, "*Destruction*");
    sv_staff_speed = borg_lookup_sval_fail(tval, "Speed");
    sv_staff_healing = borg_lookup_sval_fail(tval, "Healing");
    sv_staff_the_magi = borg_lookup_sval_fail(tval, "the Magi");
    sv_staff_power = borg_lookup_sval_fail(tval, "Power");
    sv_staff_holiness = borg_lookup_sval_fail(tval, "Holiness");
    kv_staff_holiness = borg_lookup_kind(tval, sv_staff_holiness);
    sv_staff_curing = borg_lookup_sval_fail(tval, "Curing");
    sv_staff_sleep_monsters = borg_lookup_sval_fail(tval, "Sleep Monsters");
    sv_staff_slow_monsters = borg_lookup_sval_fail(tval, "Slow Monsters");
    sv_staff_detect_invis = borg_lookup_sval_fail(tval, "Detect Invisible");
    sv_staff_detect_evil = borg_lookup_sval_fail(tval, "Detect Evil");
    sv_staff_dispel_evil = borg_lookup_sval_fail(tval, "Dispel Evil");
    sv_staff_banishment = borg_lookup_sval_fail(tval, "Banishment");
    sv_staff_light = borg_lookup_sval_fail(tval, "Light");
    sv_staff_mapping = borg_lookup_sval_fail(tval, "Mapping");
    sv_staff_remove_curse = borg_lookup_sval_fail(tval, "Remove Curse");

    tval = tval_find_idx("wand");
    sv_wand_light = borg_lookup_sval_fail(tval, "Light");
    sv_wand_teleport_away = borg_lookup_sval_fail(tval, "Teleport Other");
    sv_wand_stinking_cloud = borg_lookup_sval_fail(tval, "Stinking Cloud");
    kv_wand_stinking_cloud = borg_lookup_kind(tval, sv_wand_stinking_cloud);
    sv_wand_magic_missile = borg_lookup_sval_fail(tval, "Magic Missile");
    kv_wand_magic_missile = borg_lookup_kind(tval, sv_wand_magic_missile);
    sv_wand_annihilation = borg_lookup_sval_fail(tval, "Annihilation");
    kv_wand_annihilation = borg_lookup_kind(tval, sv_wand_annihilation);
    sv_wand_stone_to_mud = borg_lookup_sval_fail(tval, "Stone to Mud");
    sv_wand_wonder = borg_lookup_sval_fail(tval, "Wonder");
    sv_wand_hold_monster = borg_lookup_sval_fail(tval, "Hold Monster");
    sv_wand_slow_monster = borg_lookup_sval_fail(tval, "Slow Monster");
    sv_wand_fear_monster = borg_lookup_sval_fail(tval, "Scare Monster");
    sv_wand_confuse_monster = borg_lookup_sval_fail(tval, "Scare Monster");
    sv_wand_fire_bolt = borg_lookup_sval_fail(tval, "Fire Bolts");
    sv_wand_cold_bolt = borg_lookup_sval_fail(tval, "Frost Bolts");
    sv_wand_acid_bolt = borg_lookup_sval_fail(tval, "Acid Bolts");
    sv_wand_elec_bolt = borg_lookup_sval_fail(tval, "Lightning Bolts");
    sv_wand_fire_ball = borg_lookup_sval_fail(tval, "Fire Balls");
    sv_wand_cold_ball = borg_lookup_sval_fail(tval, "Cold Balls");
    sv_wand_acid_ball = borg_lookup_sval_fail(tval, "Acid Balls");
    sv_wand_elec_ball = borg_lookup_sval_fail(tval, "Lightning Bolts");
    sv_wand_dragon_cold = borg_lookup_sval_fail(tval, "Dragon's Frost");
    sv_wand_dragon_fire = borg_lookup_sval_fail(tval, "Dragon's Flame");
    sv_wand_drain_life = borg_lookup_sval_fail(tval, "Drain Life");

    tval = tval_find_idx("sword");
    sv_dagger = borg_lookup_sval_fail(tval, "Dagger");

    tval = tval_find_idx("bow");
    sv_sling = borg_lookup_sval_fail(tval, "Sling");
    sv_short_bow = borg_lookup_sval_fail(tval, "Short Bow");
    sv_long_bow = borg_lookup_sval_fail(tval, "Long Bow");
    sv_light_xbow = borg_lookup_sval_fail(tval, "Light Crossbow");
    sv_heavy_xbow = borg_lookup_sval_fail(tval, "Heavy Crossbow");

    tval = tval_find_idx("arrow");
    sv_arrow_seeker = borg_lookup_sval_fail(tval, "Seeker Arrow");
    sv_arrow_mithril = borg_lookup_sval_fail(tval, "Mithril Arrow");

    tval = tval_find_idx("bolt");
    sv_bolt_seeker = borg_lookup_sval_fail(tval, "Seeker Bolt");
    sv_bolt_mithril = borg_lookup_sval_fail(tval, "Mithril Bolt");

    tval = tval_find_idx("gloves");
    sv_set_of_leather_gloves = borg_lookup_sval_fail(tval, "Set of Leather Gloves");

    tval = tval_find_idx("cloak");
    sv_cloak = borg_lookup_sval_fail(tval, "Cloak");

    tval = tval_find_idx("soft armor");
    sv_robe = borg_lookup_sval_fail(tval, "Robe");

    tval = tval_find_idx("crown");
    sv_iron_crown = borg_lookup_sval_fail(tval, "Iron Crown");

    tval = tval_find_idx("dragon armor");
    sv_dragon_blue = borg_lookup_sval_fail(tval, "Blue Dragon Scale Mail");
    sv_dragon_black = borg_lookup_sval_fail(tval, "Black Dragon Scale Mail");
    sv_dragon_white = borg_lookup_sval_fail(tval, "White Dragon Scale Mail");
    sv_dragon_red = borg_lookup_sval_fail(tval, "Red Dragon Scale Mail");
    sv_dragon_green = borg_lookup_sval_fail(tval, "Green Dragon Scale Mail");
    sv_dragon_multihued = borg_lookup_sval_fail(tval, "Multi-Hued Dragon Scale Mail");
    sv_dragon_shining = borg_lookup_sval_fail(tval, "Shining Dragon Scale Mail");
    sv_dragon_law = borg_lookup_sval_fail(tval, "Law Dragon Scale Mail");
    sv_dragon_gold = borg_lookup_sval_fail(tval, "Gold Dragon Scale Mail");
    sv_dragon_chaos = borg_lookup_sval_fail(tval, "Chaos Dragon Scale Mail");
    sv_dragon_balance = borg_lookup_sval_fail(tval, "Balance Dragon Scale Mail");
    sv_dragon_power = borg_lookup_sval_fail(tval, "Power Dragon Scale Mail");
}


/*
 * Initialize this file
 */
void borg_init_1(void)
{
    int i, x, y;


    /* Allocate the "keypress queue" */
    borg_key_queue = mem_zalloc(KEY_SIZE * sizeof(keycode_t));


    /* Prapare a local random number seed */
    if (!borg_rand_local)
        borg_rand_local = randint1(0x10000000);

    /* initialize the sv_ and kv_ values used by the borg */
    borg_init_svs_and_kvs();

    /*** Grids ***/

    /* Make each row of grids */
    for (y = 0; y < AUTO_MAX_Y; y++)
    {
        /* Make each row */
        borg_grids[y] = mem_zalloc(AUTO_MAX_X * sizeof(borg_grid));
    }


    /*** Grid data ***/

    /* Allocate */
    borg_data_flow = mem_zalloc(sizeof(borg_data));

    /* Allocate */
    borg_data_cost = mem_zalloc(sizeof(borg_data));

    /* Allocate */
    borg_data_hard = mem_zalloc(sizeof(borg_data));

    /* Allocate */
    borg_data_know = mem_zalloc(sizeof(borg_data));

    /* Allocate */
    borg_data_icky = mem_zalloc(sizeof(borg_data));

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
    track_shop_x = mem_zalloc(9 * sizeof(int));
    track_shop_y = mem_zalloc(9 * sizeof(int));


    /*** Special "tracking" arrays ***/

    /* Track "up" stairs */
    track_less_num = 0;
    track_less_size = 16;
    track_less_x = mem_zalloc(track_less_size * sizeof(int));
    track_less_y = mem_zalloc(track_less_size * sizeof(int));

    /* Track "down" stairs */
    track_more_num = 0;
    track_more_size = 16;
    track_more_x = mem_zalloc(track_more_size * sizeof(int));
    track_more_y = mem_zalloc(track_more_size * sizeof(int));

    /* Track glyphs */
    track_glyph_num = 0;
    track_glyph_size = 200;
    track_glyph_x = mem_zalloc(track_glyph_size * sizeof(int));
    track_glyph_y = mem_zalloc(track_glyph_size * sizeof(int));

    /* Track the worn items to avoid loops */
    track_worn_num = 0;
    track_worn_size = 10;
    track_worn_time = 0;
    track_worn_name1 = mem_zalloc(track_worn_size * sizeof(byte));

    /* Track Steps */
    track_step_num = 0;
    track_step_size = 100;
    track_step_x = mem_zalloc(track_step_size * sizeof(int));
    track_step_y = mem_zalloc(track_step_size * sizeof(int));

    /* Track doors closed by borg */
    track_door_num = 0;
    track_door_size = 100;
    track_door_x = mem_zalloc(track_door_size * sizeof(int));
    track_door_y = mem_zalloc(track_door_size * sizeof(int));

    /* Track closed doors on map */
    track_closed_num = 0;
    track_closed_size = 100;
    track_closed_x = mem_zalloc(track_closed_size * sizeof(int));
    track_closed_y = mem_zalloc(track_closed_size * sizeof(int));

    /* Track mineral veins with treasure. */
    track_vein_num = 0;
    track_vein_size = 100;
    track_vein_x = mem_zalloc(track_vein_size * sizeof(int));
    track_vein_y = mem_zalloc(track_vein_size * sizeof(int));

    /*** Object tracking ***/

    /* No objects yet */
    borg_takes_cnt = 0;
    borg_takes_nxt = 1;

    /* Array of objects */
    borg_takes = mem_zalloc(256 * sizeof(borg_take));

    /*** Monster tracking ***/

    /* No monsters yet */
    borg_kills_cnt = 0;
    borg_kills_nxt = 1;

    /* Array of monsters */
    borg_kills = mem_zalloc(256 * sizeof(borg_kill));

    /*** Special counters ***/

    /* Count racial appearances */
    borg_race_count = mem_zalloc(z_info->r_max * sizeof(s16b));

    /* Count racial deaths */
    borg_race_death = mem_zalloc(z_info->r_max * sizeof(s16b));


    /*** XXX XXX XXX Hack -- Cheat ***/

    /* Hack -- Extract dead uniques */
    for (i = 1; i < z_info->r_max - 1; i++)
    {
        struct monster_race* r_ptr = &r_info[i];

        /* Skip non-monsters */
        if (!r_ptr->name) continue;

        /* Skip non-uniques */
        if (rf_has(r_ptr->flags, RF_UNIQUE)) continue;

        /* Mega-Hack -- Access "dead unique" list */
        if (r_ptr->max_num == 0) borg_race_death[i] = 1;
    }

    /* sanity check  */
    if (DUNGEON_WID != z_info->dungeon_wid ||
        DUNGEON_HGT != z_info->dungeon_hgt)
    {
        borg_note("**STARTUP FAILURE** dungeon size miss match");
        borg_init_failure = true;
    }
    /* note: I would check if player_id2class returns null but it */
    /* never does, even on a bad class */
    if (!streq(player_id2class(CLASS_WARRIOR)->name, "Warrior") ||
        !streq(player_id2class(CLASS_MAGE)->name, "Mage") ||
        !streq(player_id2class(CLASS_DRUID)->name, "Druid") ||
        !streq(player_id2class(CLASS_PRIEST)->name, "Priest") ||
        !streq(player_id2class(CLASS_NECROMANCER)->name, "Necromancer") ||
        !streq(player_id2class(CLASS_PALADIN)->name, "Paladin") ||
        !streq(player_id2class(CLASS_ROGUE)->name, "Rogue") ||
        !streq(player_id2class(CLASS_RANGER)->name, "Ranger") ||
        !streq(player_id2class(CLASS_BLACKGUARD)->name, "Blackguard"))
    {
        borg_note("**STARTUP FAILURE** classes do not match");
        borg_init_failure = true;
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
        struct object_kind* k_ptr = &k_info[k];

        /* Found a match */
        if ((k_ptr->tval == tval) && (k_ptr->sval == sval)) return (k);
    }

    /* Failure */
    msg("No object (%s,%d,%d)", tval_find_name(tval), tval, sval);
    return 0;
}

int borg_distance(int y, int x, int y2, int x2)
{
    return distance(loc(x, y), loc(x2, y2));
}

/*
*  check an item for being ammo.
*/
bool borg_is_ammo(int tval)
{
    switch (tval)
    {
    case TV_SHOT:
    case TV_ARROW:
    case TV_BOLT:
    return true;
    default:
    return false;
    }
}


/* AJG *HACK* this handles the é and á in some monster names but, gods it is ugly */
/* convert to wide and back to match the processing of special characters */
/* memory can be passed in, if it isn't this routine will allocate any memory it needs */
/* and it is up to the caller to detect that memory was allocated and free it. */
char* borg_massage_special_chars(char* name, char* memory)
{
    bool allocated = false;
    wchar_t wide_name[1024];

    if (memory == NULL)
    {
        allocated = true;
        memory = mem_zalloc((strlen(name) + 1) * sizeof(char));
    }
    text_mbstowcs(wide_name, name, strlen(name) + 1);
    wcstombs(memory, wide_name, strlen(name) + 1);

    /* free up the memory if the name didn't contain special chars */
    /* otherwise it is assumed the caller will free it. */
    /* note: the borg doesn't bother since this is init processing */
    if (allocated && streq(memory, name))
    {
        mem_free(memory);
        return name;
    }

    return memory;
}


#ifdef MACINTOSH
static int HACK = 0;
#endif

#endif /* ALLOW_BORG */
