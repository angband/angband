/* File: borg-ben.c */

/* Purpose: one brain for the Borg (see "borg.c") -BEN- */

#include "angband.h"



#ifdef AUTO_PLAY

#include "borg.h"


/*
 * See "borg.h" for general information.
 *
 * This file provides support for the "Borg", an "automatic" Angband player.
 *
 * The initialization routine grabs the "Term_xtra()" hook of the main "Term",
 * allowing the Borg to notice when the game is asking for keypresses, and to
 * send its own keys to the keypress queue, while still allowing (occasional)
 * keys to be accepted from the user.  The keys from the user should probably
 * be single keycodes, to prevent interlace with the Borg keypresses.  Note
 * that the initialization function is invoked by the special "wizard command"
 * "^A$" which is usually available in wizard mode.
 *
 * Once the Borg has been initialized, the user can choose to start
 * the Borg or ask it questions via the "borg_mode() key" ("^Z"), which
 * invokes the "borg_mode()" function, which allows the user to do things
 * to the borg, including restart ("z"), resume ("r"), plus various queries.
 *
 * You may "stop" the Borg by entering wizard mode (using the "^W" command).
 * You should probably make a macro from some key to "\e\e\e\e\e^W", so that
 * you can kill the Borg if it gets stuck in a loop.
 *
 * The Borg is only supposed to "know" what is visible on the screen, which
 * it learns by using the "term.c" screen access function "Term_what()", and
 * the cursor location function "Term_locate()", and a hack to access the
 * cursor visibility using "Term_show/hide_cursor()".  It is only allowed to
 * send keypresses to the "Term" like a normal user, by using the standard
 * "Term_keypress()" routine.  It only thinks and sends keys when it catches
 * a request to "Term_xtra(TERM_XTRA_EVENT)", which the "term.c" file uses
 * to wait for a keypress (or other event) from the user.
 *
 * That is, if if were not for the "cheats" currently used (see below).
 *
 *
 * Must be careful of:
 *   Level changing (intentionally or accidentally)
 *   Gold (or objects) embedded in walls (need to tunnel)
 *   Objects with ',' or '$' symbols (look like monsters)
 *   Mimic monsters (look like objects) or invisible monsters
 *   Traps (disarm), Doors (open/bash), Rubble (tunnel)
 *   Stores (enter via movement, exit via escape)
 *   Stores (limited commands, such as no "throw" command)
 *   Must discard junk before trying to pick up more junk
 *   Must be very careful not to run out of food/light
 *   Should use "identify" cleverly to obtain knowledge/money
 *   Do not attempt to throw away cursed artifacts (oops)
 *   Do not sell junk or cursed/damaged items to any stores
 *   Do not attempt to buy something with insufficient funds
 *   Use the non-optimal stairs if stuck on a level
 *   Make sure that every important grid is in a room
 *   Use "flow" code for all tasks for consistency
 *   Weird objects ("Mace of Disruption"/"Scythe of Slicing")
 *   Do not try to read scrolls when blind or confused
 *   Attempt to heal when "confused", "blind", etc
 *   Occasionally remove both rings (to rearrange them)
 *   Make sure to "wear" items in order of "goodness"
 *   This will allow the "ring removal hack" to work
 *   Notice "failure" when using rod/staff of perceptions
 *   Do not fire at monsters that may have changed location
 *   Search for secret doors if stuck on a level (spastic)
 *   Earthquake (or destruction) of already mapped grids
 *   Collect monsters/objects left behind or seen from afar
 *
 * Should be careful of:
 *   Do not throw away cursed artifacts (non-warrior types)
 *   If wounded, must run away from monsters, then rest
 *   Invisible monsters causing damage while trying to rest
 *   Try to use a shovel/pick to help with tunnelling
 *   Becoming "afraid" (attacking takes a turn for no effect)
 *   Becoming "blind" (map may be no longer valid)
 *   Out of depth player ghosts (in the town)
 *   Be VERY careful not to access illegal locations!
 *   Extremely long object descriptions (no room for inscriptions)
 *   In particular, consider artifact dragon scale mail and such
 *   Attempt to actually walk to the end of all corridors
 *   This will allow the "search" routines to work effectively
 *   As a hack, we could only build corridors from touched grids
 *   Attempt to "track" monsters who flee down corridors
 *   We will not fire missiles at magic mushrooms when afraid (!)
 *   Consider using the "x" symbol for obnoxious monsters
 *   When totally surrounded by monsters, try to escape rooms
 *
 * Memory usage (out of date):
 *   Total: 300K total
 *   Largest chunk: 42K
 *   Rooms: (200*70/8)*24 = 42K
 *   Each row of Grids: 200*16 = 3K
 *   All rows of Grids: 70*3K = 210K
 *   Inventory: 36*132 = 5K
 *   Shops: 8*24*132 = 24K
 *
 * Note that there is minimal memory use by static variables compared
 * to the code itself, and compared to allocated memory.
 *
 * Currently, the auto-player "cheats" in a few situations.  Oops.
 *
 * The "big" cheat is that we "build" an inventory listing without
 * parsing the screen.  This is mainly for efficiency, and to prevent
 * constant screen flashing, since we do know how to parse the store.
 *
 * Consider parsing the "choice" window for inventory/equipment.
 *
 * Cheats that are significant, and possibly unavoidable:
 *   Knowledge of when we are being asked for a keypress.
 *   Note that this could be avoided by LONG timeouts/delays
 *
 * Cheats "required" by implementation, but not signifant:
 *   Direct access to the "keypress queue" (sending keys)
 *   Direct access to the cursor visibility (this is silly)
 *   Direct use of the "current screen image" (parsing screen)
 *   Note that this includes distinguishing white/black spaces
 *
 * Cheats that could be "overcome" trivially:
 *   Direct modification of the "current options"
 *
 * Cheats that could be avoided by duplicating code:
 *   Direct access to the "r_list" and "k_list" arrays
 *   Direct access to the "v_list" and "ego_name" arrays
 *
 * Cheats that save a lot of "time", but are "optional":
 *   Direct extraction of "w_x" and "w_y" (cheat_panel)
 *   Direct extraction of inventory/equipment (cheat_inven)
 *
 * Cheats that the Borg would love:
 *   Unique attr/char codes for every monster and object
 *   Ring of See invisible, Ring of Free Action, Helm of Seeing
 *   Use of "x" for mushroom monsters (magic mushrooms!)
 *
 * Simple ways to make the Borg more effective:
 *   Choose a male fighter (maximize strength)
 *   Roll for high strength (this is very important)
 *   Roll for 18 or 18/50 constitution (high hitpoints)
 *   Roll for high dexterity (may yield multiple attacks)
 *   Do not play a human or dunadin (no infravision)
 *   Choose a dwarf (resist blindness)
 *   Choose a high elf (see invisible)
 *   Choose a gnome (free action)
 */



static int shop_num = 7;	/* Most recent shop index */

static int last_visit = 0;	/* Last purchase visit */

static int goal = 0;		/* Current "goal" */

static int goal_rising = 0;	/* Currently fleeing to town */

static int stair_less;		/* Use the next "up" staircase */
static int stair_more;		/* Use the next "down" staircase */

static int count_floor;		/* Number of floor grids */
static int count_less;		/* Number of stairs (up) */
static int count_more;		/* Number of stairs (down) */
static int count_kill;		/* Number of monsters */
static int count_take;		/* Number of objects */

static s32b attention;		/* Attention this level deserves */

static s32b c_t;		/* Current time */


static bool cheat_inven = TRUE;	/* Cheat to get "inven/equip" */

static bool cheat_panel = TRUE;	/* Cheat to get "panel" */

static bool panic_death = TRUE;	/* Panic before Death */

static bool panic_stuff = TRUE;	/* Panic before Junking Stuff */

static s32b auto_began = 0L;	/* When this level began */

static s32b auto_shock = 0L;	/* When last "shocked" */

static s32b auto_recall = 0L;	/* When we read word of recall */






/*
 * Quick determination if a symbol is in one of those arrays
 */
static bool *auto_is_kill;
static bool *auto_is_take;



/*
 * Some kinds -- required items
 */
static int kind_food_ration = 21;
static int kind_potion_serious = 240;
static int kind_potion_critical = 241;
static int kind_scroll_recall = 220;
static int kind_scroll_teleport = 186;
static int kind_scroll_identify = 176;
static int kind_flask = 348;
static int kind_torch = 346;
static int kind_lantern = 347;

/*
 * Some kinds -- increase stat potions
 */
static int kind_potion_add_str = 225;
static int kind_potion_add_int = 228;
static int kind_potion_add_wis = 231;
static int kind_potion_add_dex = 251;
static int kind_potion_add_con = 243;
static int kind_potion_add_chr = 234;

/*
 * Some kinds -- restore stat potions
 */
static int kind_potion_fix_str = 227;
static int kind_potion_fix_int = 230;
static int kind_potion_fix_wis = 233;
static int kind_potion_fix_dex = 252;
static int kind_potion_fix_con = 253;
static int kind_potion_fix_chr = 236;

/*
 * Some kinds -- miscellaneous
 */
static int kind_potion_fix_exp = 260;
static int kind_staff_teleport = 303;
static int kind_staff_identify = 326;
static int kind_rod_identify = 372;

/*
 * Some kinds -- various enchantment scrolls
 */
static int kind_enchant_to_hit = 173;
static int kind_enchant_to_dam = 174;
static int kind_enchant_to_ac = 175;

/*
 * Some kinds -- various missiles
 */
static int kind_missile_shot = 83;
static int kind_missile_arrow = 78;
static int kind_missile_bolt = 80;

/*
 * Variable kinds -- preferred missile type
 */
static int kind_missile = 83;



/*
 * Hack -- Actual number of each "item kind".
 */
static byte kind_have[MAX_K_IDX];

/*
 * Hack -- Desired number of each "item kind".
 */
static byte kind_need[MAX_K_IDX];


/*
 * Hack -- scrolls that should always be read if possible
 */
static int kind_force_scrolls[] = {
    189,	/* magic mapping */
    194,	/* trap detection */
    197,	/* door/stair detection */
    198,	/* acquirement */
    199,	/* star-acquirement */
    209,	/* protection from evil */
    0
};

/*
 * Hack -- potions that should always be quaffed if possible
 */
static int kind_force_potions[] = {
    225,	/* strength */
    228,	/* intelligence */
    231,	/* wisdom */
    234,	/* charisma */
    243,	/* constitution */
    244,	/* experience */
    251,	/* dexterity */
    256,	/* enlightenment */
    418,	/* augmentation */
    0
};


/*
 * Hack -- scrolls that should be read rather than discarded
 */
static int kind_allow_scrolls[] = {
    180,	/* remove curse */
    181,	/* light */
    188,	/* monster confusion */
    189,	/* magic mapping */
    190,	/* rune of protection */
    191,	/* star-remove-curse */
    192,	/* treasure detection */
    193,	/* object detection */
    194,	/* trap detection */
    197,	/* door/stair detection */
    198,	/* acquirement */
    199,	/* star-acquirement */
    201,	/* detect invisible */
    204,	/* trap/door destruction */
    209,	/* protection from evil */
    210,	/* satisfy hunger */
    211,	/* dispel undead */
    217,	/* blessing */
    218,	/* holy chant */
    219,	/* holy prayer */
    0
};

/*
 * Hack -- potions that should be quaffed rather than discarded
 */
static int kind_allow_potions[] = {
    222,	/* slime mold juice */
    223,	/* apple juice */
    224,	/* water */
    225,	/* strength */
    227,	/* restore strength */
    228,	/* intelligence */
    230,	/* restore intelligence */
    231,	/* wisdom */
    233,	/* restore wisdom */
    234,	/* charisma */
    236,	/* restore charisma */
    237,	/* cure light wounds */
    240,	/* cure serious wounds */
    241,	/* cure critical wounds */
    242,	/* healing */
    243,	/* constitution */
    244,	/* experience */
    249,	/* speed */
    251,	/* dexterity */
    252,	/* restore dexterity */
    253,	/* restore constitution */
    256,	/* enlightenment */
    257,	/* heroism */
    258,	/* beserk strength */
    259,	/* boldness */
    260,	/* restore life levels */
    261,	/* resist fire */
    262,	/* resist cold */
    263,	/* detect invisible */
    264,	/* slow poison */
    265,	/* neutralize poison */
    266,	/* restore mana */
    267,	/* infravision */
    418,	/* augmentation */
    419,	/* star-healing */
    420,	/* life */
    0
};


/*
 * State variables extracted from the screen
 */

static bool do_weak;
static bool do_hungry;

static bool do_blind;
static bool do_afraid;
static bool do_confused;
static bool do_poisoned;

static bool do_add_str;
static bool do_add_int;
static bool do_add_wis;
static bool do_add_dex;
static bool do_add_con;
static bool do_add_chr;

static bool do_fix_str;
static bool do_fix_int;
static bool do_fix_wis;
static bool do_fix_dex;
static bool do_fix_con;
static bool do_fix_chr;

static bool do_fix_exp;

static int auto_level;		/* Current player level */

static int auto_curhp;		/* Current hitpoints */
static int auto_maxhp;		/* Maximum hitpoints */

static s32b auto_gold;		/* Current "gold" */

static int auto_stat_str;	/* Current strength */
static int auto_stat_int;	/* Current intelligence */
static int auto_stat_wis;	/* Current wisdom */
static int auto_stat_dex;	/* Current dexterity */
static int auto_stat_con;	/* Current constitution */
static int auto_stat_chr;	/* Current charisma */

static int auto_depth = -1;	/* Current dungeon "level" */





/*
 * Hack -- set the options the way we like them
 */
static void borg_play_options(void)
{
    /* The Borg uses the original keypress codes */
    rogue_like_commands = FALSE;

    /* Use color to identify monsters and such */
    use_color = TRUE;

    /* Pick up items when stepped on */
    always_pickup = TRUE;

    /* Require explicit target request */
    use_old_target = FALSE;

    /* Do NOT query "throw" commands */
    always_throw = TRUE;

    /* Do NOT query "pick up" commands */
    carry_query_flag = FALSE;

    /* Do NOT query "various" commands */
    other_query_flag = FALSE;

    /* Require explicit repeat commands */
    always_repeat = FALSE;

    /* Do not get confused by extra info */
    plain_descriptions = TRUE;

    /* Maximize space for information */
    show_inven_weight = FALSE;
    show_equip_weight = FALSE;
    show_store_weight = FALSE;

    /* Buy/Sell without haggling */
    no_haggle_flag = TRUE;

    /* Maximize screen info */
    fresh_before = TRUE;
    fresh_after = TRUE;

    /* Read the level directly (not in feet) */
    depth_in_feet = FALSE;

    /* Use the health bar (later) */
    show_health_bar = TRUE;

    /* Do not let equippy chars confuse the health bar */
    equippy_chars = FALSE;

    /* XXX Hack -- notice "command" mode */
    hilite_player = FALSE;

    /* XXX XXX Mega-Hack -- use "preserve" mode */
    p_ptr->preserve = TRUE;
}


#if 0

/*
 * Hack -- take a note later
 */
static void borg_tell(cptr what)
{
    cptr s;

    /* Hack -- self note */
    borg_keypress(':');
    for (s = what; *s; s++) borg_keypress(*s);
    borg_keypress('\n');
}

#endif



/*
 * Count the number of items of the given kind
 */
static bool borg_amount(int k)
{
    int i, n = 0;

    /* Scan the pack */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Require that kind */
        if (item->kind != k) continue;

        /* Count items */
        n += item->iqty;
    }

    /* Result */
    return (n);
}


/*
 * Hack -- perform an action on an item of the given "kind"
 * Hack -- by choosing the "last" pile, we prefer discounted items
 */
static bool borg_choose(int k)
{
    int i, n = -1;

    /* Nothing available */
    if (kind_have[k] <= 0) return (-1);
    
    /* Scan the pack */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Require that kind */
        if (item->kind != k) continue;

        /* Save the last "best" index (smallest pile) */
        if ((n < 0) || (item->iqty <= auto_items[n].iqty)) n = i;
    }

    /* Result */
    return (n);
}



/*
 * Hack -- perform an action on an item of the given "kind"
 */
static bool borg_action(char c, int k)
{
    int i;

    /* Choose a usable item */
    i = borg_choose(k);

    /* Nothing to use */
    if (i < 0) return (FALSE);

    /* Hack -- Cannot read when blind/confused */
    if ((c == 'r') && (do_blind || do_confused)) return (FALSE);

    /* Log the message */
    if (auto_fff) {
        borg_info(format("Action '%c' on item %s.", c, auto_items[i].desc));
    }

    /* Perform the action */
    borg_keypress(c);
    borg_keypress('a' + i);

    /* Success */
    return (TRUE);
}



/*
 * Hack -- perform an action on an item of the given "kind"
 * Hack -- only accept items with a non-zero "pval" code
 * Hack -- by choosing the first pile, we prefer "high charges"
 * This tends to result in "piles" of items with equal charges
 * This may also induce occasional "dropping" of items :-)
 */
static bool borg_choose_pval(int k)
{
    int i, n = -1;

    /* Nothing available */
    if (kind_have[k] <= 0) return (-1);
    
    /* Scan the pack */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Require that kind */
        if (item->kind != k) continue;

        /* Hack -- Require non-zero "pval" */
        if (item->pval == 0) continue;

        /* Save the first "best" index (smallest pile) */
        if ((n < 0) || (item->iqty < auto_items[n].iqty)) n = i;
    }

    /* Result */
    return (n);
}



/*
 * Hack -- perform an action on an item of the given "kind"
 * Hack -- only accept items with a positive "pval"
 */
static bool borg_action_pval(char c, int k)
{
    int i;

    /* Choose a usable item */
    i = borg_choose_pval(k);

    /* Nothing to use */
    if (i < 0) return (FALSE);

    /* Hack -- Cannot read when blind/confused */
    if ((c == 'r') && (do_blind || do_confused)) return (FALSE);

    /* Log the message */
    if (auto_fff) {
        borg_info(format("Action '%c' on item %s.", c, auto_items[i].desc));
    }

    /* Perform the action */
    borg_keypress(c);
    borg_keypress('a' + i);

    /* Success */
    return (TRUE);
}




/*
 * Decide whether to "inspect" a grid
 * Assume the given grid is NOT a wall
 */
static bool borg_inspect()
{
    int		i, wall = 0, supp = 0, diag = 0;

    char	cc[8];


    /* Hack -- Never inspect the town */
    if (auto_depth == 0) return (FALSE);

    /* Hack -- This grid is fully inspected */
    if (pg->xtra > 20) return (FALSE);

    /* Tweak -- only search occasionally */
    if (rand_int(50) != 0) return (FALSE);


    /* Examine adjacent grids */
    for (i = 0; i < 8; i++) {

        /* Extract the location */
        int xx = c_x + ddx[ddd[i]];
        int yy = c_y + ddy[ddd[i]];

        /* Obtain the grid */
        auto_grid *ag = grid(xx, yy);

        /* Require knowledge */
        if (ag->o_c == ' ') return (FALSE);

        /* Extract the symbol */
        cc[i] = ag->o_c;
    }


    /* Count possible door locations */
    for (i = 0; i < 4; i++) if (cc[i] == '#') wall++;

    /* Hack -- no possible doors */
    if (!wall) return (FALSE);


    /* Count supporting evidence for secret doors */
    for (i = 0; i < 4; i++) {
        if ((cc[i] == '#') || (cc[i] == '%')) supp++;
        else if ((cc[i] == '+') || (cc[i] == '\'')) supp++;
    }

    /* Count supporting evidence for secret doors */
    for (i = 4; i < 8; i++) {
        if ((cc[i] == '#') || (cc[i] == '%')) diag++;
    }

    /* Hack -- Examine "suspicious" walls */
    if (((diag >= 4) && (supp >= 3)) ||
        ((diag >= 4) && (rand_int(30) == 0)) ||
        ((diag >= 2) && (rand_int(900) == 0))) {

        /* Take note */
        borg_note("Searching...");

        /* Remember the search */
        if (pg->xtra < 100) pg->xtra += 9;

        /* Search a little */
        borg_keypress('0');
        borg_keypress('9');
        borg_keypress('s');

        /* Success */
        return (TRUE);
    }


    /* Assume no suspicions */
    return (FALSE);
}





#if 0

/*
 * Send a command to inscribe item number "i" with the inscription "str".
 */
static void borg_send_inscribe(int i, cptr str)
{
    cptr s;

    /* The "inscribe" command */
    char c1 = '{', c2 = '}';

    /* Label it */
    borg_keypress(c1);

    /* Hack -- allow "equipment" labelling */
    if (i >= INVEN_WIELD) {
        borg_keypress('/');
        i -= INVEN_WIELD;
    }

    /* Choose the item */
    borg_keypress('a' + i);

    /* Send the label */
    for (s = str; *s; s++) borg_keypress(*s);

    /* End the inscription */
    borg_keypress('\n');
}

#endif



/*
 * Hack -- determine if an item is "armor"
 */
static bool borg_item_is_armour(auto_item *item)
{
    /* Check for armor */
    if ((item->tval == TV_DRAG_ARMOR) ||
        (item->tval == TV_HARD_ARMOR) ||
        (item->tval == TV_SOFT_ARMOR) ||
        (item->tval == TV_SHIELD) ||
        (item->tval == TV_CROWN) ||
        (item->tval == TV_HELM) ||
        (item->tval == TV_CLOAK) ||
        (item->tval == TV_GLOVES) ||
        (item->tval == TV_BOOTS)) {

        /* Yep */
        return (TRUE);
    }

    /* Nope */
    return (FALSE);
}


/*
 * Hack -- determine if an item is a "weapon"
 */
static bool borg_item_is_weapon(auto_item *item)
{
    /* Check for weapon */
    if ((item->tval == TV_SWORD) ||
        (item->tval == TV_HAFTED) ||
        (item->tval == TV_POLEARM) ||
        (item->tval == TV_DIGGING) ||
        (item->tval == TV_BOW) ||
        (item->tval == TV_BOLT) ||
        (item->tval == TV_ARROW) ||
        (item->tval == TV_SHOT)) {

        /* Yep */
        return (TRUE);
    }

    /* Nope */
    return (FALSE);
}






/*
 * Estimate the "power" of an item.
 *
 * Make a heuristic guess at the "power" of an item in "gold"
 *
 * We assume that the given item is "aware" and "known" (or "average").
 * We also assume that the given item is "acceptable" (not worthless).
 *
 * Note -- This is used both for "wearing" items and for "buying" them.
 */
static s32b item_power(auto_item *item)
{
    s32b value;
    s32b awful;

    int to_a = item->to_a;
    int to_h = item->to_h;
    int to_d = item->to_d;
    
    
    /* Mega-Hack -- Never wear unknowns */
    if (!item->kind) return (0L);


    /* Hack -- Start with the "value" */
    awful = 0;


    /* Armour */
    if ((item->tval == TV_SHIELD) ||
        (item->tval == TV_SOFT_ARMOR) ||
        (item->tval == TV_HARD_ARMOR) ||
        (item->tval == TV_DRAG_ARMOR) ||
        (item->tval == TV_BOOTS) ||
        (item->tval == TV_GLOVES) ||
        (item->tval == TV_CLOAK) ||
        (item->tval == TV_CROWN) ||
        (item->tval == TV_HELM)) {

        /* Hack -- Base value */
        item->to_a = 8;
        value = borg_item_value(item);
        /* value -= (item->to_a - to_a) * 50L; */
        item->to_a = to_a;

        /* Reward the total armor */
        value += (item->ac + item->to_a) * 100L;
    }

    /* Weapons */
    else if ((item->tval == TV_HAFTED) ||
             (item->tval == TV_SWORD) ||
             (item->tval == TV_POLEARM) ||
             (item->tval == TV_DIGGING)) {

        /* Base value */
        item->to_h = 8;
        item->to_d = 8;
        value = borg_item_value(item);
        /* value -= (item->to_h - to_h) * 25L; */
        /* value -= (item->to_d - to_d) * 50L; */
        item->to_h = to_h;
        item->to_d = to_d;

        /* Favor high "max damage" */
        value += ((item->dd * item->ds) * 200L);

        /* Favor the bonuses */
        value += (item->to_h * 10L);
        value += (item->to_d * 100L);
    }

    /* Missile Launchers */
    else if (item->tval == TV_BOW) {

        /* Base value */
        item->to_h = 8;
        item->to_d = 8;
        value = borg_item_value(item);
        /* value -= (item->to_h - to_h) * 25L; */
        /* value -= (item->to_d - to_d) * 50L; */
        item->to_h = to_h;
        item->to_d = to_d;

        /* Mega-Hack -- Check the sval */
        value += (item->sval * 500L);

        /* Check the bonuses */
        value += (item->to_h * 10L);
        value += (item->to_d * 100L);
    }

    /* Rings/Amulets */
    else if ((item->tval == TV_AMULET) || 
             (item->tval == TV_RING)) {

        /* Avoid negative bonuses */
        if (item->to_h < 0) return (0L);
        if (item->to_d < 0) return (0L);
        if (item->to_a < 0) return (0L);
        if (item->pval < 0) return (0L);

        /* Base value */
        value = borg_item_value(item);
    }


    /* Lite */
    else if (item->tval == TV_LITE) {

        /* Base value */
        value = borg_item_value(item);

        /* Hack -- Prefer usable lites */
        if (item->pval) value += 5000L;
    }


    /* Flasks */
    else if (item->tval == TV_FLASK) {

        /* Base value */
        value = borg_item_value(item);

        /* Buy flasks for lanterns */
        value += 2000L;
    }

    /* Food */
    else if (item->tval == TV_FOOD) {

        /* Base value */
        value = borg_item_value(item);

        /* Buy normal food */
        if (item->kind == kind_food_ration) value += 50000L;
    }

    /* Scrolls */
    else if (item->tval == TV_SCROLL) {

        /* Base value */
        value = borg_item_value(item);

        /* Hack -- prefer certain items */
        if (item->kind == kind_scroll_teleport) value += 3000L;
        if (item->kind == kind_scroll_identify) value += 6000L;
        if (item->kind == kind_scroll_recall) value += 9000L;
    }

    /* Potions */
    else if (item->tval == TV_SCROLL) {

        /* Base value */
        value = borg_item_value(item);

        /* Hack -- prefer certain items */
        if (item->kind == kind_potion_critical) value += 5000L;
        if (item->kind == kind_potion_serious) value += 500L;
    }

    /* Others */
    else {
    
        /* Base value */
        value = borg_item_value(item);
    }


    /* Return the value */
    return (value);
}




/*
 * Determine how much it is "worth" to bring an object back to town
 * This function analyzes a single instance of the given item.
 *
 * Note that we make heavy use of the "borg_item_value()" routine, which
 * is pretty accurate, but is misleading for unaware or unidentified items,
 * and for items with "convenient" side effects, such as missiles or staffs
 * of perceptions, and for items which can easily "stack" with other items.
 *
 * This routine is used to determine how much gold the Borg will get for
 * holding onto an item until he can sell it in a shop.  This is NOT used
 * to determine which item is best to buy or wield/wear, see "item_power()".
 *
 * This function currently assumes that "borg_item_value()" is "correct",
 * which is a large assumption, since certain items have "hidden" cost
 * components, such as weapons of slay undead which also hold life.
 *
 * Hack -- we add "heuristic" bonuses to various items which have
 * "implicit" value to the Borg.  This is cute but a little dangerous.
 */
static s32b item_worth(auto_item *item)
{
    s32b value;


    /* Extract the base value */
    value = borg_item_value(item);

    /* Worthless item */
    if (value <= 0L) return (0L);


    /* Mega-Hack -- fake value for non-aware items */
    if (!item->kind) return (auto_depth * 100 + 50 + value);


    /* Hack -- staffs of identify can be used */
    if (item->kind == kind_staff_identify) value += item->pval * 200L;

    /* Hack -- staffs of teleport can be used */
    if (item->kind == kind_staff_teleport) value += item->pval * 200L;


    /* Return the value */
    return (value);
}



/*
 * Determine if an item can be sold in the current store
 */
static bool borg_good_sell(auto_item *item)
{
    int tval = item->tval;


    /* Hack -- artifacts */
    if (item->name1) {

        /* Save them in the home */
        if (shop_num == 7) return (TRUE);

        /* Never sell them */
        return (FALSE);
    }


    /* Switch on the store */
    switch (shop_num + 1) {

      /* General Store */
      case 1:

        /* Analyze the type */
        switch (tval) {
          case TV_DIGGING:
          case TV_CLOAK:
          case TV_FOOD:
          case TV_FLASK:
          case TV_LITE:
          case TV_SPIKE:
            return (TRUE);
        }
        break;

      /* Armoury */
      case 2:

        /* Analyze the type */
        switch (tval) {
          case TV_BOOTS:
          case TV_GLOVES:
          case TV_HELM:
          case TV_CROWN:
          case TV_SHIELD:
          case TV_SOFT_ARMOR:
          case TV_HARD_ARMOR:
          case TV_DRAG_ARMOR:
            return (TRUE);
        }
        break;

      /* Weapon Shop */
      case 3:

        /* Analyze the type */
        switch (tval) {
          case TV_SHOT:
          case TV_BOLT:
          case TV_ARROW:
          case TV_BOW:
          case TV_HAFTED:
          case TV_POLEARM:
          case TV_SWORD:
            return (TRUE);
        }
        break;

      /* Temple */
      case 4:

        /* Analyze the type */
        switch (tval) {
          case TV_HAFTED:
          case TV_SCROLL:
          case TV_POTION:
          case TV_PRAYER_BOOK:
            return (TRUE);
        }
        break;

      /* Alchemist */
      case 5:

        /* Analyze the type */
        switch (tval) {
          case TV_SCROLL:
          case TV_POTION:
            return (TRUE);
        }
        break;

      /* Magic Shop */
      case 6:

        /* Analyze the type */
        switch (tval) {
          case TV_MAGIC_BOOK:
          case TV_AMULET:
          case TV_RING:
          case TV_STAFF:
          case TV_WAND:
          case TV_ROD:
            return (TRUE);
        }
        break;
    }

    /* Assume not */
    return (FALSE);
}




/*
 * Determine if the Borg is running out of crucial supplies.
 * This routine is used to invoke word of recall (if possible)
 */
static bool borg_restock()
{
    auto_item *item = &auto_items[INVEN_LITE];

    /* Running out of food */
    if (kind_have[kind_food_ration] < 5) return (TRUE);

    /* Totally out of light */
    if (item->iqty == 0) return (TRUE);

    /* Running out of flasks for lanterns */
    if ((item->kind == kind_lantern) && (kind_have[kind_flask] < 5)) return (TRUE);

    /* Running out of torches */
    if ((item->kind == kind_torch) && (kind_have[kind_torch] < 5)) return (TRUE);

    /* Assume happy */
    return (FALSE);
}


/*
 * Determine if an item should be bought
 */
static bool borg_good_buy(auto_item *ware)
{
    int i, slot;

    auto_item *worn = NULL;


    /* Never buy "weird" stuff */
    if (!ware->kind) return (FALSE);


    /* Determine where the item would be worn */
    slot = borg_wield_slot(ware);

    /* Extract the item currently in that slot */
    if (slot >= 0) worn = &auto_items[slot];


    /* Use the "shopping list" */
    if (kind_need[ware->kind]) {

        /* Check the pile */
        if (borg_amount(ware->kind) < kind_need[ware->kind]) {
            return (TRUE);
        }
    }
    

    /* Process Torches */
    if (ware->kind == kind_torch) {

        /* Never use the black market */
        if (shop_num == 6) return (FALSE);

        /* Hack -- Artifact lites are the best */
        if (worn->name1) return (FALSE);

        /* Hack -- Torches are defeated by (fueled) lanterns */
        if (worn->kind == kind_lantern) {
            if (kind_have[kind_flask] >= 10) return (FALSE);
        }

        /* Always have at least 20 torches */
        if (kind_have[kind_torch] < 20) return (TRUE);
    }

    /* Process Lanterns */
    else if (ware->kind == kind_lantern) {

        /* Never use the black market */
        if (shop_num == 6) return (FALSE);

        /* Hack -- Artifact lites are the best */
        if (worn->name1) return (FALSE);

        /* Never buy a lantern when wielding one already */
        if (worn->kind == kind_lantern) return (FALSE);

        /* Always buy at least 1 lantern */
        if (kind_have[kind_lantern] < 1) return (TRUE);
    }


    /* Missiles */
    else if ((ware->tval == TV_SHOT) ||
             (ware->tval == TV_ARROW) ||
             (ware->tval == TV_BOLT)) {

        /* Never use the black market */
        if (shop_num == 6) return (FALSE);

        /* Never buy expensive missiles */
        if (ware->cost > 50) return (FALSE);

        /* Never buy incorrect missiles */
        if (ware->kind != kind_missile) return (FALSE);

        /* Never buy too many missiles */
        if (kind_have[kind_missile] >= 30) return (FALSE);
        
        /* Buy missiles */
        return (TRUE);
    }


    /* Process amulets and rings */
    else if ((ware->tval == TV_AMULET) || (ware->tval == TV_RING)) {

        /* XXX XXX XXX Never buy rings/amulets (for now) */
        return (FALSE);
    }


    /* Process Equipment */
    else if (slot >= 0) {

        /* Always use something */
        if (!worn->iqty) return (TRUE);

        /* Hack -- Always buy the best we can afford */
        if (item_power(ware) > item_power(worn)) return (TRUE);
    }


    /* Assume useless */
    return (FALSE);
}


/*
 * Hack -- determine what to do with an item
 */
static bool borg_notice_aux(auto_item *item)
{
    int i, slot;

    auto_item *worn = NULL;

    bool res = FALSE;


    /* Clear the flags */
    item->wear = item->cash = item->junk = item->test = FALSE;


    /* Throw away "junk" and "spikes" */
    if ((item_worth(item) <= 0) ||
        (item->tval == TV_SPIKE)) {

        /* This is junk */
        item->junk = TRUE;

        /* Do not sell this */
        item->cash = FALSE;

        /* All done */
        return (res);
    }


    /* Hack -- Assume we will sell it */
    item->cash = TRUE;


    /* Check the "shopping list" */
    if (kind_need[item->kind]) {
    
        /* Hack -- Do not sell items we "need" */
        if (kind_have[item->kind] <= kind_need[item->kind]) {
            item->cash = FALSE;
        }
    }
    

    /* Process scrolls */
    if (item->tval == TV_SCROLL) {

        /* Hack -- Identify "interesting" items */
        if (!item->kind && (auto_depth > 5)) {
            item->test = TRUE;
            item->cash = FALSE;
        }
    }


    /* Process potions */
    else if (item->tval == TV_POTION) {

        /* Hack -- Identify "interesting" items */
        if (!item->kind && (auto_depth > 5)) {
            item->test = TRUE;
            item->cash = FALSE;
        }
    }


    /* Process rods */
    else if (item->tval == TV_ROD) {

        /* Always identify rods */
        if (!item->kind) {
            item->test = TRUE;
            item->cash = FALSE;
        }
    }


    /* Process wands/staffs */
    else if ((item->tval == TV_WAND) || (item->tval == TV_STAFF)) {

        /* Identify type and charges */
        if (!item->kind || !item->able) {
            item->test = TRUE;
            item->cash = FALSE;
        }
    }


    /* Mega-Hack -- Junk chests */
    else if (item->tval == TV_CHEST) {

        /* Hack -- Throw it away */
        item->junk = TRUE;
        item->cash = FALSE;
    }


    /* Keep some food */
    else if (item->tval == TV_FOOD) {

        /* Hack -- Identify "interesting" items */
        if (!item->kind) {
            item->test = TRUE;
            item->cash = FALSE;
        }
    }


    /* Keep flasks (unless unnecessary) */
    else if (item->tval == TV_FLASK) {

        /* Hack -- assume we will keep it */
        item->cash = FALSE;

        /* Hack -- Artifact lites need no fuel */
        if (auto_items[INVEN_LITE].name1) item->cash = TRUE;
    }


    /* Hack -- skip "non-wearables" */
    if (item->tval < TV_MIN_WEAR) return (res);
    if (item->tval > TV_MAX_WEAR) return (res);


    /* See what slot that item could go in */
    slot = borg_wield_slot(item);

    /* Extract the item currently in that slot */
    if (slot >= 0) worn = &auto_items[slot];


    /* Note -- Assume we will not sell it */
    item->cash = FALSE;


    /* Process rings */
    if (item->tval == TV_RING) {

        /* Hack -- identify items */
        if (!item->kind || !item->able) {
            item->test = TRUE;
        }

        /* Wear most powerful items */
        else if (item_power(item) > item_power(worn)) {
            item->wear = TRUE;
        }

        /* Sell everything else */
        else {
            item->cash = TRUE;
        }

        /* All done */
        return (res);
    }


    /* Process amulets */
    if (item->tval == TV_AMULET) {

        /* Hack -- identify items */
        if (!item->kind || !item->able) {
            item->test = TRUE;
        }

        /* Wear most powerful items */
        else if (item_power(item) > item_power(worn)) {
            item->wear = TRUE;
        }

        /* Sell everything else */
        else {
            item->cash = TRUE;
        }

        /* All done */
        return (res);
    }


    /* Process Lite's */
    if (item->tval == TV_LITE) {

        /* Hack -- identify lites */
        if (!item->kind || !item->able) {
            item->test = TRUE;
        }

        /* Hack -- Replace missing lites */
        else if (!worn->iqty) {
            item->wear = TRUE;
        }

        /* Hack -- Sell "empty" Lites */
        else if (!item->pval) {
            item->cash = TRUE;
        }

        /* Hack -- replace empty lites */
        else if (!worn->pval) {
            item->wear = TRUE;
        }

        /* Hack -- Heuristic test */
        else if (item_power(item) > item_power(worn)) {
            item->wear = TRUE;
        }

        /* Hack -- Keep a single artifact lite */
        else if (worn->name1) {
            item->cash = TRUE;
        }

        /* Notice "junky" torches */
        else if (item->kind == kind_torch) {

            /* Lantern plus fuel pre-empts torches */
            if (worn->kind == kind_lantern) {
                if (kind_have[kind_flask] > 10) {
                    item->cash = TRUE;
                }
            }

            /* Sell "excess" torches */
            else if (kind_have[kind_torch] > 30) {
                item->cash = TRUE;
            }
        }

        /* Notice "extra" lanterns */
        else if (item->kind == kind_lantern) {

            /* Already wielding a lantern */
            if (worn->kind == kind_lantern) {
                item->cash = TRUE;
            }

            /* Already carrying a lantern */
            else if (kind_have[kind_lantern] > 1) {
                item->cash = TRUE;
            }
        }

        /* All done */
        return (res);
    }


    /* Identify "good" items */
    if (!item->able &&
        ((streq(item->note, "{good}")) ||
         (streq(item->note, "{blessed}")) ||
         (streq(item->note, "{excellent}")) ||
         (streq(item->note, "{special}")))) {

        /* Test this */
        item->test = TRUE;

        /* All done */
        return (res);
    }


    /* Hack -- Wait for feelings */
    if (!item->able) {

        /* All done */
        return (res);
    }


    /* Analyze missiles */
    if ((item->tval == TV_SHOT) ||
        (item->tval == TV_ARROW) ||
        (item->tval == TV_BOLT)) {

        /* Sell invalid missiles */
        if (item->kind != kind_missile) {
            item->cash = TRUE;
        }

        /* Hack -- sell big piles of missiles */
        else if (item->iqty > 50) {
            item->cash = TRUE;
        }

        /* All done */
        return (res);
    }


    /* Wear something */
    if (!worn->iqty) {

        /* Wear it */
        item->wear = TRUE;
    }

    /* Wear the "best" equipment */
    else if (item_power(item) > item_power(worn)) {

        /* Wear it */
        item->wear = TRUE;
    }

    /* Sell the rest */
    else {

        /* Sell it */
        item->cash = TRUE;
    }


    /* Success */
    return (res);
}



/*
 * Examine the inventory
 */
static void borg_notice(void)
{
    int i, k, n;


    /* Assume we have nothing */
    C_WIPE(&kind_have, MAX_K_IDX, byte);
    
    /* Assume we need nothing */
    C_WIPE(&kind_need, MAX_K_IDX, byte);


    /* Count "amount" of each item kind in pack */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Obtain quantity */
        n = kind_have[item->kind] + item->iqty;
        
        /* Maintain count (max out at 255) */
        kind_have[item->kind] = ((n < 255) ? n : 255);
    }
        

    /* Choose a missile type */
    if (TRUE) {
    
        /* Access the bow (if any) */
        auto_item *worn = &auto_items[INVEN_BOW];

        /* Default ammo */
        kind_missile = kind_missile_shot;
        
        /* Access the "proper" ammo kind (ignore "seeker" ammo) */
        if (worn->sval == SV_SHORT_BOW) kind_missile = kind_missile_arrow;
        if (worn->sval == SV_LONG_BOW)  kind_missile = kind_missile_arrow;
        if (worn->sval == SV_LIGHT_XBOW) kind_missile = kind_missile_bolt;
        if (worn->sval == SV_HEAVY_XBOW) kind_missile = kind_missile_bolt;
    }


    /* Collect food */
    kind_need[kind_food_ration] = 30;
    

    /* Collect Flasks for lanterns */
    if (auto_items[INVEN_LITE].kind == kind_lantern) {
        kind_need[kind_flask] = 30;
    }


    /* Collect scrolls of identify */
    kind_need[kind_scroll_identify] = 40;

    /* Collect scrolls of teleport */
    kind_need[kind_scroll_teleport] = 10;

    /* Collect scrolls of recall */
    kind_need[kind_scroll_recall] = 5;


    /* Collect potions of cure critical wounds */
    kind_need[kind_potion_critical] = 10;

    /* Collect potions of cure serious wounds */
    kind_need[kind_potion_serious] = 10;


    /* Desire Stat Potions (if useful) */
    kind_need[kind_potion_add_str] = (do_add_str) ? 1 : 0;
    kind_need[kind_potion_add_int] = (do_add_int) ? 1 : 0;
    kind_need[kind_potion_add_wis] = (do_add_wis) ? 1 : 0;
    kind_need[kind_potion_add_dex] = (do_add_dex) ? 1 : 0;
    kind_need[kind_potion_add_con] = (do_add_con) ? 1 : 0;
    kind_need[kind_potion_add_chr] = (do_add_chr) ? 1 : 0;

    /* Desire Stat Restoration Potions (if needed) */
    kind_need[kind_potion_fix_str] = (do_fix_str) ? 1 : 0;
    kind_need[kind_potion_fix_int] = (do_fix_int) ? 1 : 0;
    kind_need[kind_potion_fix_wis] = (do_fix_wis) ? 1 : 0;
    kind_need[kind_potion_fix_dex] = (do_fix_dex) ? 1 : 0;
    kind_need[kind_potion_fix_con] = (do_fix_con) ? 1 : 0;
    kind_need[kind_potion_fix_chr] = (do_fix_chr) ? 1 : 0;


    /* Desire Experience Restoration Potions (if needed) */
    kind_need[kind_potion_fix_exp] = (do_fix_exp) ? 1 : 0;


    /* Look for equipment that needs enchanting */
    for (i = INVEN_WIELD; i <= INVEN_FEET; i++) {

        auto_item *item = &auto_items[i];

        /* Skip non-items */
        if (!item->iqty) continue;

        /* Enchant all armour */
        if (borg_item_is_armour(item) && (item->to_a < 10)) {
            kind_need[kind_enchant_to_ac] += (10 - item->to_a);
        }

        /* Enchant all weapons (to damage) */
        if (borg_item_is_weapon(item) && (item->to_d < 10)) {
            kind_need[kind_enchant_to_dam] += (10 - item->to_d);
        }

        /* Enchant all weapons (to hit) */
        if (borg_item_is_weapon(item) && (item->to_h < 10)) {
            kind_need[kind_enchant_to_hit] += (10 - item->to_h);
        }
    }



    /* Mega-Hack -- Assume "average" equals "known" */
    for (i = 0; i < INVEN_TOTAL; i++) {

        auto_item *item = &auto_items[i];

        /* Skip "empty" items */
        if (!item->iqty) continue;

        /* Skip "known" items */
        if (item->able) continue;
        
        /* Assume "average" items are "known" */
        if (streq(item->note, "{average}")) item->able = TRUE;
    }



    /* Analyze the inventory */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Decide what to do with it */
        (void)borg_notice_aux(item);
    }
}



/*
 * Look at the screen and update the borg
 *
 * Uses the "panel" info (w_x, w_y) obtained earlier
 *
 * Note that the "c_t" variable corresponds *roughly* to player turns,
 * except that resting and "repeated" commands count as a single turn,
 * and "free" moves (including "illegal" moves, such as attempted moves
 * into walls, or tunneling into monsters) are counted as turns.
 */
static void borg_update()
{
    int n, x, y, dx, dy, lev = auto_depth;

    auto_grid *ag;
    auto_room *ar;

    byte t_a;

    cptr s;

    char buf[128];

    byte old_attr[SCREEN_HGT][SCREEN_WID];
    char old_char[SCREEN_HGT][SCREEN_WID];


    /* Increase the "time" */
    c_t++;


    /* Clear all the "state flags" */
    do_weak = do_hungry = FALSE;
    do_blind = do_confused = do_afraid = do_poisoned = FALSE;
    do_add_str = do_add_int = do_add_wis = FALSE;
    do_add_dex = do_add_con = do_add_chr = FALSE;
    do_fix_str = do_fix_int = do_fix_wis = FALSE;
    do_fix_dex = do_fix_con = do_fix_chr = FALSE;
    do_fix_exp = FALSE;


    /* Extract current level */
    if (0 == borg_what_text(6, ROW_LEVEL, -6, &t_a, buf)) {
        auto_level = atoi(buf);
    }

    /* Extract current hitpoints */
    if (0 == borg_what_text(6, ROW_CURHP, -6, &t_a, buf)) {
        auto_curhp = atoi(buf);
    }

    /* Extract maximum hitpoints */
    if (0 == borg_what_text(6, ROW_MAXHP, -6, &t_a, buf)) {
        auto_maxhp = atoi(buf);
    }

    /* Extract current gold */
    if (0 == borg_what_text(COL_GOLD, ROW_GOLD, -9, &t_a, buf)) {
        auto_gold = atol(buf);
    }



    /* Check for hunger */
    if (0 == borg_what_text(COL_HUNGRY, ROW_HUNGRY, 6, &t_a, buf)) {
        if (streq(buf, "Hungry")) do_hungry = TRUE;
        if (streq(buf, "Weak  ")) do_weak = TRUE;
    }

    /* Check for blind */
    if (0 == borg_what_text(COL_BLIND, ROW_HUNGRY, 5, &t_a, buf)) {
        if (streq(buf, "Blind")) do_blind = TRUE;
    }

    /* Check for confused */
    if (0 == borg_what_text(COL_CONFUSED, ROW_HUNGRY, 8, &t_a, buf)) {
        if (streq(buf, "Confused")) do_confused = TRUE;
    }

    /* Check for afraid */
    if (0 == borg_what_text(COL_AFRAID, ROW_HUNGRY, 6, &t_a, buf)) {
        if (streq(buf, "Afraid")) do_afraid = TRUE;
    }

    /* Check for poisoned */
    if (0 == borg_what_text(COL_POISONED, ROW_HUNGRY, 8, &t_a, buf)) {
        if (streq(buf, "Poisoned")) do_poisoned = TRUE;
    }


    /* Obtain actual stats */
    if (0 == borg_what_text(COL_STAT, ROW_STAT+0, 6, &t_a, buf)) {
        if (buf[5] == '*') auto_stat_str = 18 + 220;
        else if (buf[2] == '/') auto_stat_str = 18 + atoi(buf+3);
        else if (buf[3] == '/') auto_stat_str = 18 + atoi(buf+4);
        else auto_stat_str = atoi(buf);
    }
    if (0 == borg_what_text(COL_STAT, ROW_STAT+1, 6, &t_a, buf)) {
        if (buf[5] == '*') auto_stat_int = 18 + 220;
        else if (buf[2] == '/') auto_stat_int = 18 + atoi(buf+3);
        else if (buf[3] == '/') auto_stat_int = 18 + atoi(buf+4);
        else auto_stat_int = atoi(buf);
    }
    if (0 == borg_what_text(COL_STAT, ROW_STAT+2, 6, &t_a, buf)) {
        if (buf[5] == '*') auto_stat_wis = 18 + 220;
        else if (buf[2] == '/') auto_stat_wis = 18 + atoi(buf+3);
        else if (buf[3] == '/') auto_stat_wis = 18 + atoi(buf+4);
        else auto_stat_wis = atoi(buf);
    }
    if (0 == borg_what_text(COL_STAT, ROW_STAT+3, 6, &t_a, buf)) {
        if (buf[5] == '*') auto_stat_dex = 18 + 220;
        else if (buf[2] == '/') auto_stat_dex = 18 + atoi(buf+3);
        else if (buf[3] == '/') auto_stat_dex = 18 + atoi(buf+4);
        else auto_stat_dex = atoi(buf);
    }
    if (0 == borg_what_text(COL_STAT, ROW_STAT+4, 6, &t_a, buf)) {
        if (buf[5] == '*') auto_stat_con = 18 + 220;
        else if (buf[2] == '/') auto_stat_con = 18 + atoi(buf+3);
        else if (buf[3] == '/') auto_stat_con = 18 + atoi(buf+4);
        else auto_stat_con = atoi(buf);
    }
    if (0 == borg_what_text(COL_STAT, ROW_STAT+5, 6, &t_a, buf)) {
        if (buf[5] == '*') auto_stat_chr = 18 + 220;
        else if (buf[2] == '/') auto_stat_chr = 18 + atoi(buf+3);
        else if (buf[3] == '/') auto_stat_chr = 18 + atoi(buf+4);
        else auto_stat_chr = atoi(buf);
    }


    /* Check for non-maximal stats */
    if (auto_stat_str < 18+100) do_add_str = TRUE;
    if (auto_stat_int < 18+100) do_add_int = TRUE;
    if (auto_stat_wis < 18+100) do_add_wis = TRUE;
    if (auto_stat_dex < 18+100) do_add_dex = TRUE;
    if (auto_stat_con < 18+100) do_add_con = TRUE;
    if (auto_stat_chr < 18+100) do_add_chr = TRUE;


    /* Check for drained stats */
    if (0 == borg_what_text(0, ROW_STAT+0, 3, &t_a, buf)) {
        if (islower(buf[2])) do_fix_str = TRUE;
    }
    if (0 == borg_what_text(0, ROW_STAT+1, 3, &t_a, buf)) {
        if (islower(buf[2])) do_fix_int = TRUE;
    }
    if (0 == borg_what_text(0, ROW_STAT+2, 3, &t_a, buf)) {
        if (islower(buf[2])) do_fix_wis = TRUE;
    }
    if (0 == borg_what_text(0, ROW_STAT+3, 3, &t_a, buf)) {
        if (islower(buf[2])) do_fix_dex = TRUE;
    }
    if (0 == borg_what_text(0, ROW_STAT+4, 3, &t_a, buf)) {
        if (islower(buf[2])) do_fix_con = TRUE;
    }
    if (0 == borg_what_text(0, ROW_STAT+5, 3, &t_a, buf)) {
        if (islower(buf[2])) do_fix_chr = TRUE;
    }

    /* Check for drained experience */
    if (0 == borg_what_text(0, ROW_EXP, 3, &t_a, buf)) {
        if (islower(buf[2])) do_fix_exp = TRUE;
    }


    /* Extract the "current dungeon level" */
    if (0 == borg_what_text(70, 23, -7, &t_a, buf)) {
        for (s = buf; *s && !isdigit(*s); s++);
        lev = atoi(s);
    }


    /* Notice changes in the level */
    if (auto_depth != lev) {

        /* Restart the clock */
        c_t = 10000L;

        /* Start a new level */
        auto_began = c_t;

        /* Shocking */
        auto_shock = c_t;

        /* No goal yet */
        goal = 0;

        /* Do not use any stairs */
        stair_less = stair_more = FALSE;

        /* Hack -- cannot rise past town */
        if (!lev) goal_rising = FALSE;

        /* Hack -- town is "boring" */
        if (!lev) attention = 0L;

        /* No known grids yet */
        count_floor = count_less = count_more = 0;

        /* Nothing to chase yet */
        count_kill = count_take = 0;

        /* No word of recall yet */
        auto_recall = 0L;

        /* Nothing bought yet */
        last_visit = 0;

        /* Wipe the old "map" arrays */
        borg_forget_map();

        /* Save the new level */
        auto_depth = lev;

        /* Hack -- Clear the key buffer */
        borg_flush();

        /* Hack -- Verify options */
        borg_play_options();
    }


    /* Memorize the "current" (66x22 grid) map sector */
    for (dy = 0; dy < SCREEN_HGT; dy++) {
        for (dx = 0; dx < SCREEN_WID; dx++) {

            /* Obtain the map location */
            x = w_x + dx;
            y = w_y + dy;

            /* Get the auto_grid */
            ag = grid(x, y);

            /* Save the current knowledge */
            old_attr[dy][dx] = ag->o_a;
            old_char[dy][dx] = ag->o_c;
        }
    }


    /* Analyze the map */
    borg_update_map();

    /* Paranoia -- catch failure */
    if (!pg || !auto_active) return;

    /* Update the view */
    borg_update_view();



    /* Make "fake" rooms for all viewable unknown grids */
    for (n = 0; n < view_n; n++) {

        /* Access the location */
        y = view_y[n];
        x = view_x[n];

        /* Access the grid */
        ag = grid(x,y);

        /* Skip walls */
        if (ag->info & BORG_WALL) continue;

        /* Skip "known" grids */
        if (ag->o_c != ' ') continue;

        /* Make "fake" rooms as needed */
        if (!ag->room) {

            auto_room *ar;

            /* Get a new room */
            ar = borg_free_room();

            /* Initialize the room */
            ar->x = ar->x1 = ar->x2 = x;
            ar->y = ar->y1 = ar->y2 = y;

            /* Save the room */
            ag->room = ar->self;
        }
    }


    /* Analyze the current (66x22 grid) map sector */
    for (dy = 0; dy < SCREEN_HGT; dy++) {
        for (dx = 0; dx < SCREEN_WID; dx++) {

            /* Obtain the map location */
            x = w_x + dx;
            y = w_y + dy;

            /* Get the auto_grid */
            ag = grid(x, y);

            /* Skip unknown grids */
            if (ag->o_c == ' ') continue;

            /* Notice first "knowledge" */
            if (old_char[dy][dx] == ' ') {

                /* We are shocked */
                auto_shock = c_t;
            }

            /* Count certain "changes" */
            if (ag->o_c != old_char[dy][dx]) {

                /* Lost old "floor" grids */
                if (old_char[dy][dx] == '.') count_floor--;

                /* Lost old "permanent" landmarks */
                if (old_char[dy][dx] == '<') count_less--;
                if (old_char[dy][dx] == '>') count_more--;

                /* Lost old "temporary" info */
                if (auto_is_kill[(byte)(old_char[dy][dx])]) count_kill--;
                if (auto_is_take[(byte)(old_char[dy][dx])]) count_take--;

                /* Found new "floor" grids */		
                if (ag->o_c == '.') count_floor++;

                /* Found new "permanent" landmarks */
                if (ag->o_c == '<') count_less++;
                if (ag->o_c == '>') count_more++;

                /* Lost old "temporary" info */
                if (auto_is_kill[(byte)(ag->o_c)]) count_kill++;
                if (auto_is_take[(byte)(ag->o_c)]) count_take++;
            }


            /* Clear all "broken" rooms */
            if ((ag->o_c == '#') || (ag->o_c == '%')) {

                /* Clear all rooms containing walls */
                if (ag->room) {

                    /* Clear all rooms containing walls */
                    if (borg_clear_room(x, y)) goal = 0;
                }
            }

            /* Create "fake" rooms as needed */
            else {

                /* Mega-Hack -- super-fake rooms */
                if (!ag->room) {

                    /* Acquire a new room */
                    ar = borg_free_room();

                    /* Initialize the room */
                    ar->x = ar->x1 = ar->x2 = x;
                    ar->y = ar->y1 = ar->y2 = y;

                    /* Save the room */
                    ag->room = ar->self;
                }	
            }


            /* Skip non-viewable grids */
            if (!(ag->info & BORG_VIEW)) continue;

            /* Hack -- Notice all (viewable) changes */
            if ((ag->o_c != old_char[dy][dx]) ||
                (ag->o_a != old_attr[dy][dx])) {

                /* Important -- Cancel goals */
                goal = 0;
            }
        }
    }	


    /* Paranoia -- require a self room */
    if (!pg->room) {

        /* Acquire a new room */
        ar = borg_free_room();

        /* Initialize the room */
        ar->x = ar->x1 = ar->x2 = c_x;
        ar->y = ar->y1 = ar->y2 = c_y;

        /* Save the room */
        pg->room = ar->self;
    }

    /* Build a "bigger" room around the player */
    if (borg_build_room(c_x, c_y)) goal = 0;

    /* Mark all the "containing rooms" as visited. */
    for (ar = room(1,c_x,c_y); ar; ar = room(0,0,0)) {

        /* Note the visit */
        ar->when = c_t;
    }

    /* Hack -- Count visits to this grid */
    /* if (pg->visits < 100) pg->visits++; */
}


/*
 * Attempt to eat something edible
 * Slime Molds and Elvish Waybread
 */
static bool borg_eat_food_any(void)
{
    int i;

    /* Scan the inventory */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Skip unknown food */
        if (!item->kind) continue;

        /* Skip normal food rations */
        if (item->kind == kind_food_ration) continue;

        /* Skip non-food */
        if (item->tval != TV_FOOD) continue;

        /* Eat "acceptable" food */
        if (item->sval >= SV_FOOD_MIN_FOOD) {

            /* Eat something of that type */
            if (borg_action('E', item->kind)) return (TRUE);
        }
    }

    /* Nothing */
    return (FALSE);
}




/*
 * Read a scroll of recall if not in town and allowed
 */
static bool borg_recall(void)
{
    /* Not in town */
    if (auto_depth == 0) return (FALSE);

    /* Only read recall once per level */
    if (auto_recall) return (FALSE);

    /* Cannot read any scrolls */
    if (do_blind || do_confused) return (FALSE);

    /* Try to read a scroll of recall */
    if (borg_action('r', kind_scroll_recall)) {

        /* Remember when */
        auto_recall = c_t;

        /* Success */
        return (TRUE);
    }

    /* Nothing */
    return (FALSE);
}




/*
 * Use things in a useful manner
 * Attempt healing, refueling, eating, etc.
 */
static bool borg_use_things(void)
{
    auto_item *item = &auto_items[INVEN_LITE];


    /* Panic -- avoid possible death */
    if (panic_death &&
       (auto_curhp < 100) &&
       (auto_curhp < auto_maxhp / 4)) {

        borg_oops("Almost dead.");
        return (TRUE);
    }


    /* Hack -- attempt to escape */
    if ((auto_curhp < auto_maxhp / 2) && (rand_int(4) == 0)) {
        if ((kind_have[kind_potion_critical] < 5) && borg_recall()) return (TRUE);
    }


    /* Hack -- heal when wounded */
    if ((auto_curhp < auto_maxhp / 2) && (rand_int(4) == 0)) {
        if (borg_action('q', kind_potion_critical)) return (TRUE);
        if (borg_action('q', kind_potion_serious)) return (TRUE);
    }

    /* Hack -- heal when blind/confused */
    if ((do_blind || do_confused) && (rand_int(4) == 0)) {
        if (borg_action('q', kind_potion_serious)) return (TRUE);
        if (borg_action('q', kind_potion_critical)) return (TRUE);
    }


    /* Eat or abort */
    if (do_hungry || do_weak) {

        /* Attempt to eat something edible */
        if (borg_eat_food_any()) return (TRUE);

        /* Attempt to eat food rations */
        if (borg_action('E', kind_food_ration)) return (TRUE);

        /* Mega-Hack -- Flee to town for food */
        if (borg_recall()) return (TRUE);
    }

    /* Hack -- Do NOT starve to death */
    if (panic_death && do_weak) {

        borg_oops("Starving.");
        return (TRUE);
    }


    /* Refuel current torch */
    if ((item->kind == kind_torch) && (item->pval < 1000)) {

        /* Try to wield a lantern */
        if (borg_action('w', kind_lantern)) return (TRUE);

        /* Try to refuel the torch */
        if (borg_action('F', kind_torch)) return (TRUE);
    }


    /* Refuel current lantern */
    if ((item->kind == kind_lantern) && (item->pval < 5000)) {

        /* Try to refill the lantern */
        if (borg_action('F', kind_flask)) return (TRUE);
    }


    /* Nothing to do */
    return (FALSE);
}


/*
 * Use things in a useful, but non-essential, manner
 */
static bool borg_use_others(void)
{
    int i, j, b, a;

    int num_armour = 0, num_weapon = 0;


    /* Quaff experience restoration potion */
    if (do_fix_exp && borg_action('q', kind_potion_fix_exp)) return (TRUE);


    /* Quaff stat restoration potions */
    if (do_fix_str && borg_action('q', kind_potion_fix_str)) return (TRUE);
    if (do_fix_int && borg_action('q', kind_potion_fix_int)) return (TRUE);
    if (do_fix_wis && borg_action('q', kind_potion_fix_wis)) return (TRUE);
    if (do_fix_dex && borg_action('q', kind_potion_fix_dex)) return (TRUE);
    if (do_fix_con && borg_action('q', kind_potion_fix_con)) return (TRUE);
    if (do_fix_chr && borg_action('q', kind_potion_fix_chr)) return (TRUE);


    /* Quaff "useful" potions/scrolls */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Check list of potions */
        for (j = 0; kind_force_potions[j]; j++) {

            /* Skip non-matches */
            if (item->kind != kind_force_potions[j]) continue;

            /* Try quaffing each potion */
            if (borg_action('q', item->kind)) return (TRUE);
        }

        /* Hack -- check Blind/Confused */
        if (do_blind || do_confused) continue;

        /* Check list of potions */
        for (j = 0; kind_force_scrolls[j]; j++) {

            /* Skip non-matches */
            if (item->kind != kind_force_scrolls[j]) continue;

            /* Try reading each scroll */
            if (borg_action('r', item->kind)) return (TRUE);
        }
    }


    /* Hack -- blind/confused -- no scrolls */
    if (do_blind || do_confused) return (FALSE);


    /* No enchantment scrolls */
    i = 0;

    /* Count enchantment scrolls */
    i += kind_have[kind_enchant_to_ac];
    i += kind_have[kind_enchant_to_hit];
    i += kind_have[kind_enchant_to_dam];

    /* No enchantment scrolls */
    if (!i) return (FALSE);


    /* Mega-Hack -- count armour/weapons in pack (see below) */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Count armour */
        if (borg_item_is_armour(item)) num_armour++;

        /* Count weapons */
        if (borg_item_is_weapon(item)) num_weapon++;
    }


    /* Look for armor that needs enchanting */
    for (b = 0, a = 99, i = INVEN_BODY; i <= INVEN_FEET; i++) {

        auto_item *item = &auto_items[i];

        /* Skip non-items */
        if (!item->iqty) continue;

        /* Find the least enchanted item */
        if (item->to_a > a) continue;

        /* Save the info */
        b = i; a = item->to_a;
    }

    /* Enchant that item if "possible" */
    if (b && (a < 10) && (borg_action('r', kind_enchant_to_ac))) {

        /* Choose from equipment */
        if (num_armour) borg_keypress('/');

        /* Choose that item */
        borg_keypress('a' + b - INVEN_WIELD);

        /* Success */
        return (TRUE);
    }


    /* Look for a weapon that needs enchanting */
    for (b = 0, a = 99, i = INVEN_WIELD; i <= INVEN_BOW; i++) {

        auto_item *item = &auto_items[i];

        /* Skip non-items */
        if (!item->iqty) continue;

        /* Find the least enchanted item */
        if (item->to_h > a) continue;

        /* Save the info */
        b = i; a = item->to_h;
    }

    /* Enchant that item if "possible" */
    if (b && (a < 10) && (borg_action('r', kind_enchant_to_hit))) {

        /* Choose from equipment */
        if (num_weapon) borg_keypress('/');

        /* Choose that item */
        borg_keypress('a' + b - INVEN_WIELD);

        /* Success */
        return (TRUE);
    }


    /* Look for a weapon that needs enchanting */
    for (b = 0, a = 99, i = INVEN_WIELD; i <= INVEN_BOW; i++) {

        auto_item *item = &auto_items[i];

        /* Skip non-items */
        if (!item->iqty) continue;

        /* Find the least enchanted item */
        if (item->to_d > a) continue;

        /* Save the info */
        b = i; a = item->to_d;
    }

    /* Enchant that item if "possible" */
    if (b && (a < 10) && (borg_action('r', kind_enchant_to_dam))) {

        /* Choose from equipment */
        if (num_weapon) borg_keypress('/');

        /* Choose that item */
        borg_keypress('a' + b - INVEN_WIELD);

        /* Success */
        return (TRUE);
    }


    /* Nothing to do */
    return (FALSE);
}


/*
 * Destroy everything we know we don't want
 */
static bool borg_throw_junk(void)
{
    int i, j;


    /* Quaff harmless "junk" potions/scrolls */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Only "discard" junk */
        if (!item->junk) continue;

        /* Check list of "allowable" potions */
        for (j = 0; kind_allow_potions[j]; j++) {

            /* Skip non-matches */
            if (item->kind != kind_allow_potions[j]) continue;

            /* Quaff junk */
            borg_info(format("Quaffing junk (%s).", item->desc));

            /* Try quaffing each potion */
            if (borg_action('q', item->kind)) return (TRUE);
        }

        /* Hack -- check Blind/Confused */
        if (do_blind || do_confused) continue;

        /* Check list of "allowable" scrolls */
        for (j = 0; kind_allow_scrolls[j]; j++) {

            /* Skip non-matches */
            if (item->kind != kind_allow_scrolls[j]) continue;

            /* Read junk */
            borg_info(format("Reading junk (%s).", item->desc));

            /* Try reading each scroll */
            if (borg_action('r', item->kind)) return (TRUE);
        }
    }


    /* Throw away "junk" */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Throw junk */
        if (item->junk) {

            /* Throw away junk */
            borg_info(format("Trashing junk (%s).", item->desc));

            /* Throw it at myself */
            borg_keypress('f');
            borg_keypress('a' + i);
            borg_keypress('*');
            borg_keypress('p');
            borg_keypress('t');

            /* Did something */
            return (TRUE);
        }
    }


    /* Nothing to destroy */
    return (FALSE);
}



/*
 * Make sure we have at least one free inventory slot.
 * This will fail if every slot holds "essentials" (food, lite,
 * word of recall, teleport, identify, healing potions, etc) or
 * "holders" (unidentified items).
 */
static bool borg_free_space(void)
{
    int i, k;

    s32b cost, limit = 999999L;

    auto_item *item, *junk;


    /* We have plenty of space */
    if (!auto_items[INVEN_PACK-1].iqty) return (FALSE);


    /* Nothing to junk yet */
    junk = NULL;

    /* Find something to trash */
    for (i = 0; i < INVEN_PACK; i++) {

        item = &auto_items[i];

        /* Stop at end of inventory */
        if (!item->iqty) break;

        /* Skip non-sell items */
        if (!item->cash) continue;

        /* Evaluate the slot */
        cost = item_worth(item) * item->iqty;

        /* Skip expensive items */
        if (junk && (cost > limit)) continue;

        /* Track cheapest item */
        junk = item;
        limit = cost;
    }

    /* Throw something away */
    if (junk) {

        /* Debug */
        borg_note(format("Junking %ld gold", limit));

        /* Hack -- Mark it as junk */
        junk->junk = TRUE;

        /* Try to throw away the junk */
        if (borg_throw_junk()) return (TRUE);
    }


    /* Mega-Hack -- try to give "feelings" a chance */
    if (rand_int(100) != 0) return (FALSE);


    /* Mega-Hack -- get very desperate */
    for (i = 0; i < INVEN_PACK; i++) {

        item = &auto_items[i];

        /* Stop at end of inventory */
        if (!item->iqty) break;

        /* Find the smallest pile of these items */
        k = borg_choose(item->kind);

        /* Do not throw away the best (or only) pile */	
        if (i == k) continue;

        /* Debug */
        borg_note(format("Discarding %s!", item->desc));

        /* Hack -- Mark it as junk */
        item->junk = TRUE;

        /* Try to throw away the junk */
        if (borg_throw_junk()) return (TRUE);	
    }


    /* Oops */
    if (panic_stuff) {

        borg_oops("Too much stuff.");
        return (FALSE);
    }


    /* Nothing to junk yet */
    junk = NULL;

    /* Find something to trash */
    for (i = 0; i < INVEN_PACK; i++) {

        item = &auto_items[i];

        /* Stop at end of inventory */
        if (!item->iqty) break;

        /* Hack -- Skip identified items */
        if (item->able) continue;

        /* Evaluate the slot */
        cost = item_worth(item) * item->iqty;

        /* Skip expensive items */
        if (junk && (cost > limit)) continue;

        /* Track cheapest item */
        junk = item;
        limit = cost;
    }

    /* Throw something away */
    if (junk) {

        /* Debug */
        borg_note(format("Desperately junking %ld+ gold!", limit));

        /* Hack -- Mark it as junk */
        junk->junk = TRUE;

        /* Try to throw away the junk */
        if (borg_throw_junk()) return (TRUE);
    }


    /* Failure */
    return (FALSE);
}


/*
 * Count the number of items worth "selling"
 * This determines the choice of stairs.
 */
static int borg_count_sell(void)
{
    int i, k = 0;
    s32b greed, worth;

    /* Calculate "greed" factor */
    greed = auto_gold / 100L;

    /* Throw away "junk" */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Skip non-items */
        if (!item->iqty) continue;	

        /* Skip "junk" items */
        if (item->junk) continue;

        /* Skip known "cheap" items */
        if (item->kind && item->cash && item->able) {

            /* Guess at the item value */
            worth = item_worth(item) * item->iqty;

            /* Do not bother with "cheap" items */
            if (worth < greed) continue;
        }

        /* Skip "average" "cheap" items */
        if (item->kind && item->cash && streq(item->note, "{average}")) {

            /* Guess at the item value */
            worth = item_worth(item) * item->iqty;

            /* Do not bother with "cheap" items */
            if (worth < greed) continue;
        }

        /* Count remaining items */
        k++;
    }


    /* Count them */
    return (k);
}




/*
 * Identify items if possible
 *
 * Note that "borg_parse()" will "cancel" the identification if it
 * detects a "You failed..." message.  This is VERY important!!!
 * Otherwise the "identify" might induce bizarre actions by sending
 * the "index" of an item as a command.
 */
static bool borg_test_stuff(void)
{
    int i;


    /* Look for an item to identify */
    for (i = 0; i < INVEN_PACK; i++) {

        /* Notice end of inventory */
        if (!auto_items[i].iqty) break;

        /* Skip tested items */
        if (!auto_items[i].test) continue;

        /* Use a Rod of Perceptions or a Staff of Perceptions */
        if (borg_action_pval('z', kind_rod_identify) ||
            borg_action_pval('u', kind_staff_identify)) {

            /* Log -- may be cancelled */
            borg_info(format("Identifying %s.", auto_items[i].desc));

            /* Identify the item */
            borg_keypress('a' + i);

            /* Success */
            return (TRUE);
        }

        /* Blind or confused */
        if (do_blind || do_confused) return (FALSE);

        /* Attempt to read a scroll of identify */
        if (borg_action('r', kind_scroll_identify)) {

            /* Log */
            borg_info(format("Identifying %s.", auto_items[i].desc));

            /* Identify the item */
            borg_keypress('a' + i);

            /* Success */
            return (TRUE);
        }
    }


    /* Nothing to do */
    return (FALSE);
}




/*
 * Maintain a "useful" inventory
 */
static bool borg_wear_stuff(void)
{
    int i, b_i = -1;

    s32b p, b_p = 0L;

    /* Wear stuff (top down) */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Skip "unknown" items */
        if (!item->wear) continue;

        /* Acquire the "power" */
        p = item_power(item);

        /* Skip bad items */
        if ((b_i >= 0) && (p < b_p)) continue;

        /* Maintain the best */
        b_i = i; b_p = p;
    }

    /* Wear the "best" item */
    if (b_i >= 0) {

        auto_item *item = &auto_items[b_i];

        /* Log */
        borg_info(format("Wearing %s.", item->desc));

        /* Wear it */
        borg_keypress('w');
        borg_keypress('a' + b_i);

        /* Did something */
        return (TRUE);
    }


    /* Nothing to do */
    return (FALSE);
}







/*
 * Process a "goto" goal, return "TRUE" if goal is still okay.
 */
static bool borg_play_step(int x, int y)
{
    auto_grid *ag;

    int dir;


    /* We have arrived */
    if ((c_x == x) && (c_y == y)) return (FALSE);

    /* Get a direction (may be a wall there) */
    dir = borg_goto_dir(c_x, c_y, x, y);

    /* We are confused */
    if ((dir == 0) || (dir == 5)) return (FALSE);


    /* Access the grid we are stepping on */
    ag = grid(c_x + ddx[dir], c_y + ddy[dir]);


    /* Must "disarm" traps */
    if (ag->o_c == '^') {
        borg_info("Disarming a trap.");
        borg_keypress('D');
    }

    /* Occasionally "bash" doors */
    else if ((ag->o_c == '+') && (rand_int(10) == 0)) {
        borg_info("Bashing a door.");
        borg_keypress('B');
    }

    /* Must "open" (or "bash") doors */
    else if (ag->o_c == '+') {
        borg_info("Opening a door.");
        borg_keypress('0');
        borg_keypress('9');
        borg_keypress('o');
    }

    /* Tunnel through rubble */
    else if (ag->o_c == ':') {
        borg_info("Digging rubble.");
        borg_keypress('0');
        borg_keypress('9');
        borg_keypress('9');
        borg_keypress('T');
    }

    /* Mega-Hack -- eventually give up digging walls */
    else if (((ag->o_c == '#') || (ag->o_c == '%')) && (rand_int(100) == 0)) {
        return (FALSE);
    }

    /* Tunnel through walls (or give up) */
    else if ((ag->o_c == '#') || (ag->o_c == '%')) {
        borg_info("Digging a wall.");
        borg_keypress('0');
        borg_keypress('9');
        borg_keypress('9');
        borg_keypress('T');
    }

    /* XXX Hack -- Occasionally, tunnel for gold */
    else if (((ag->o_c == '$') || (ag->o_c == '*')) && (rand_int(10) == 0)) {
        borg_info("Digging for treasure.");
        borg_keypress('0');
        borg_keypress('9');
        borg_keypress('9');
        borg_keypress('T');
    }

    /* XXX XXX XXX Hack -- Occasionally, tunnel anyway */
    else if (rand_int(1000) == 0) {
        borg_info("Digging for fun.");
        borg_keypress('0');
        borg_keypress('9');
        borg_keypress('9');
        borg_keypress('T');
    }


    /* Walk (or tunnel or open or bash) in that direction */
    borg_keypress('0' + dir);


    /* Mega-Hack -- prepare to take stairs if desired */
    if (stair_less && (ag->o_c == '<')) borg_keypress('<');
    if (stair_more && (ag->o_c == '>')) borg_keypress('>');


    /* Did something */
    return (TRUE);
}



/*
 * Choose a missile to fire
 */
static int borg_choose_missile(void)
{
    int i, n = -1;

    /* Scan the pack */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Skip bad missiles */
        if (item->kind != kind_missile) continue;

        /* Skip un-identified missiles */
        if (!item->able) continue;

        /* Find the smallest pile */
        if ((n < 0) || (item->iqty < auto_items[n].iqty)) n = i;
    }

    /* Use that missile */
    if (n >= 0) return (n);


    /* Use any missile */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Acceptable missiles */
        if ((item->tval == TV_BOLT) ||
            (item->tval == TV_ARROW) ||
            (item->tval == TV_SHOT)) {

            /* Skip un-identified missiles */
            if (!item->able) continue;

            /* Find the smallest pile */
            if ((n < 0) || (item->iqty < auto_items[n].iqty)) n = i;
        }
    }

    /* Use that missile */
    if (n >= 0) return (n);
    

    /* Nothing to fire */
    return (-1);
}




/*
 * Take a step towards the current goal location
 * Return TRUE if this goal is still "okay".
 * Otherwise, cancel the goal and return FALSE.
 */
static bool borg_play_old_goal(void)
{
    auto_room *ar;


    /* Flow towards the goal */
    if (goal) {

        int x = c_x, y = c_y, best = 999;

        /* See "borg_flow_cost(c_x, c_y)" */
        for (ar = room(1,c_x,c_y); ar; ar = room(0,0,0)) {

            int dx = ar->x - c_x;
            int dy = ar->y - c_y;

            int ax = ABS(dx);
            int ay = ABS(dy);

            /* Calculate the cost */
            int cost = ar->cost + MAX(ax, ay);

            /* Skip icky costs */	
            if (cost >= best) continue;

            /* Note the best so far */
            x = ar->x; y = ar->y; best = cost;
        }

        /* Attempt to take one step in the path */
        if (borg_play_step(x, y)) return (TRUE);
    }

    /* Cancel goals */
    goal = 0;

    /* Nothing to do */
    return (FALSE);
}



/*
 * Determine if a grid touches unknown grids
 */
static bool borg_interesting(int x1, int y1)
{
    int i, x, y;

    auto_grid *ag;

    /* Scan all eight neighbors */
    for (i = 0; i < 8; i++) {

        /* Get the location */
        x = x1 + ddx[ddd[i]];
        y = y1 + ddy[ddd[i]];

        /* Get the grid */
        ag = grid(x, y);

        /* Unknown grids are interesting */	
        if (ag->o_c == ' ') return (TRUE);

        /* Known walls are boring */
        if (ag->info & BORG_WALL) continue;

        /* Unroomed grids are interesting */
        if (!ag->room) return (TRUE);
    }

    /* Assume not */
    return (FALSE);
}







/*
 * Prepare to "flow" towards monsters to "kill"
 */
static bool borg_flow_kill()
{
    int i, x, y;

    auto_grid *ag;


    /* Nothing found */
    seen_n = 0;

    /* Look for something to kill */
    for (i = 0; i < view_n; i++) {

        /* Access the "view grid" */
        y = view_y[i];
        x = view_x[i];

        /* Get the auto_grid */
        ag = grid(x, y);

        /* Skip the player grid */
        if (ag == pg) continue;

        /* Skip unknown grids */
        if (ag->o_c == ' ') continue;

        /* Notice monsters (efficiently) */
        if (auto_is_kill[(byte)(ag->o_c)]) {

            /* Remember it */
            if (seen_n < SEEN_MAX) {
                seen_x[seen_n] = x;
                seen_y[seen_n] = y;
                seen_n++;
            }
        }
    }

    /* Nothing to kill */
    if (!seen_n) return (FALSE);


    /* Clear the flow codes */
    borg_flow_clear();

    /* Look for something to kill */
    for (i = 0; i < seen_n; i++) {

        /* Enqueue the grid */
        borg_flow_enqueue_grid(seen_x[i], seen_y[i]);
    }

    /* Clear the seen array */
    seen_n = 0;

    /* Spread the flow a little */
    borg_flow_spread(TRUE);

    /* Extract the "best" cost */
    i = borg_flow_cost(c_x, c_y);

    /* No good path */
    if (i >= 999) return (FALSE);

    /* Note */
    borg_info(format("Flowing toward monsters, at cost %d", i));

    /* Set the "goal" */
    goal = GOAL_KILL;

    /* Success */
    return (TRUE);
}


/*
 * Prepare to "flow" towards objects to "take"
 */
static bool borg_flow_take()
{
    int i, x, y;

    auto_grid *ag;


    /* Nothing yet */
    seen_n = 0;

    /* Look for something to take */
    for (i = 0; i < view_n; i++) {

        /* Access the "view grid" */
        y = view_y[i];
        x = view_x[i];

        /* Get the auto_grid */
        ag = grid(x, y);

        /* Skip the player grid */
        if (ag == pg) continue;

        /* Skip unknown grids */
        if (ag->o_c == ' ') continue;

        /* Notice objects (efficiently) */
        if (auto_is_take[(byte)(ag->o_c)]) {

            /* Remember it */
            if (seen_n < SEEN_MAX) {
                seen_x[seen_n] = x;
                seen_y[seen_n] = y;
                seen_n++;
            }
        }
    }

    /* Nothing to take */
    if (!seen_n) return (FALSE);


    /* Clear the flow codes */
    borg_flow_clear();

    /* Look for something to take */
    for (i = 0; i < seen_n; i++) {

        /* Enqueue the grid */
        borg_flow_enqueue_grid(seen_x[i], seen_y[i]);
    }

    /* Clear the seen array */
    seen_n = 0;

    /* Spread the flow (a little) */
    borg_flow_spread(TRUE);

    /* Extract the "best" cost */
    i = borg_flow_cost(c_x, c_y);

    /* No good path */
    if (i >= 999) return (FALSE);

    /* Note */
    borg_info(format("Flowing toward objects, at cost %d", i));

    /* Set the "goal" */
    goal = GOAL_TAKE;

    /* Success */
    return (TRUE);
}


/*
 * Prepare to "flow" towards "dark" or "unknown" grids
 */
static bool borg_flow_dark()
{
    int i, x, y;

    auto_grid *ag;


    /* Nothing yet */
    seen_n = 0;

    /* Look for something unknown */
    for (i = 0; i < view_n; i++) {

        /* Access the "view grid" */
        y = view_y[i];
        x = view_x[i];

        /* Get the auto_grid */
        ag = grid(x, y);

        /* Skip the player grid */
        if (ag == pg) continue;

        /* Skip unknown grids */
        if (ag->o_c == ' ') continue;

        /* Cannot explore walls */
        if (ag->info & BORG_WALL) continue;

        /* Notice interesting grids */
        if (borg_interesting(x, y)) {

            /* Remember it */
            if (seen_n < SEEN_MAX) {
                seen_x[seen_n] = x;
                seen_y[seen_n] = y;
                seen_n++;
            }
        }
    }

    /* Nothing dark */
    if (!seen_n) return (FALSE);


    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < seen_n; i++) {

        /* Enqueue the grid */
        borg_flow_enqueue_grid(seen_x[i], seen_y[i]);
    }

    /* Clear the seen array */
    seen_n = 0;

    /* Spread the flow (a little) */
    borg_flow_spread(TRUE);

    /* Extract the "best" cost */
    i = borg_flow_cost(c_x, c_y);

    /* No good path */
    if (i >= 999) return (FALSE);

    /* Note */
    borg_info(format("Flowing toward unknown grids, at cost %d", i));

    /* Set the "goal" */
    goal = GOAL_DARK;

    /* Success */
    return (TRUE);
}


/*
 * Prepare to "flow" towards "interesting" things
 */
static bool borg_flow_explore(void)
{
    int x, y, i;

    auto_grid *ag;


    /* Nothing yet */
    seen_n = 0;

    /* Examine every legal grid */
    for (y = 1; y < AUTO_MAX_Y-1; y++) {
        for (x = 1; x < AUTO_MAX_X-1; x++) {

            /* Get the grid */
            ag = grid(x, y);

            /* Skip current location */
            if (pg == ag) continue;

            /* Skip stuff */
            if (ag->o_c == ' ') continue;
            if (!ag->room) continue;

            /* Cannot explore walls */
            if (ag->info & BORG_WALL) continue;

            /* Only examine "interesting" grids */
            if (!borg_interesting(x, y)) continue;

            /* Remember it */
            if (seen_n < SEEN_MAX) {
                seen_x[seen_n] = x;
                seen_y[seen_n] = y;
                seen_n++;
            }
        }
    }

    /* Nothing useful */
    if (!seen_n) return (FALSE);


    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < seen_n; i++) {

        /* Enqueue the grid */
        borg_flow_enqueue_grid(seen_x[i], seen_y[i]);
    }

    /* Clear the seen array */
    seen_n = 0;

    /* Spread the flow (a little) */
    borg_flow_spread(TRUE);

    /* Extract the "best" cost */
    i = borg_flow_cost(c_x, c_y);

    /* No good path */
    if (i >= 999) return (FALSE);

    /* Note */
    borg_note(format("Chasing unknowns, at cost %d", i));

    /* Set the "goal" */
    goal = GOAL_DARK;

    /* Success */
    return (TRUE);
}


/*
 * Prepare to "flow" towards "old" monsters
 */
static bool borg_flow_kill_any(void)
{
    int i, x, y;

    auto_grid *ag;


    /* Efficiency -- Nothing to kill */
    if (!count_kill) return (FALSE);


    /* Nothing yet */
    seen_n = 0;

    /* Examine every legal grid */
    for (y = 1; y < AUTO_MAX_Y-1; y++) {
        for (x = 1; x < AUTO_MAX_X-1; x++) {

            /* Get the grid */
            ag = grid(x, y);

            /* Skip current location */
            if (pg == ag) continue;

            /* Skip stuff */
            if (ag->o_c == ' ') continue;
            if (!ag->room) continue;

            /* Skip non-monsters (efficiently) */
            if (!auto_is_kill[(byte)(ag->o_c)]) continue;

            /* Remember it */
            if (seen_n < SEEN_MAX) {
                seen_x[seen_n] = x;
                seen_y[seen_n] = y;
                seen_n++;
            }
        }
    }

    /* Nothing useful */
    if (!seen_n) return (FALSE);


    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < seen_n; i++) {

        /* Enqueue the grid */
        borg_flow_enqueue_grid(seen_x[i], seen_y[i]);
    }

    /* Clear the "seen" array */
    seen_n = 0;

    /* Spread the flow (a little) */
    borg_flow_spread(TRUE);

    /* Extract the "best" cost */
    i = borg_flow_cost(c_x, c_y);

    /* No good path */
    if (i >= 999) return (FALSE);

    /* Note */
    borg_note(format("Chasing monsters, at cost %d", i));

    /* Set the "goal" */
    goal = GOAL_KILL;

    /* Success */
    return (TRUE);
}


/*
 * Prepare to "flow" towards "old" objects
 */
static bool borg_flow_take_any(void)
{
    int i, x, y;

    auto_grid *ag;


    /* Efficiency -- Nothing to take */
    if (!count_take) return (FALSE);


    /* Nothing yet */
    seen_n = 0;

    /* Examine every legal grid */
    for (y = 1; y < AUTO_MAX_Y-1; y++) {
        for (x = 1; x < AUTO_MAX_X-1; x++) {

            /* Get the grid */
            ag = grid(x, y);

            /* Skip current location */
            if (pg == ag) continue;

            /* Skip stuff */
            if (ag->o_c == ' ') continue;
            if (!ag->room) continue;

            /* Skip non-objects (efficiently) */
            if (!auto_is_take[(byte)(ag->o_c)]) continue;

            /* Remember it */
            if (seen_n < SEEN_MAX) {
                seen_x[seen_n] = x;
                seen_y[seen_n] = y;
                seen_n++;
            }
        }
    }

    /* Nothing useful */
    if (!seen_n) return (FALSE);


    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < seen_n; i++) {

        /* Enqueue the grid */
        borg_flow_enqueue_grid(seen_x[i], seen_y[i]);
    }

    /* Clear the "seen" array */
    seen_n = 0;

    /* Spread the flow (a little) */
    borg_flow_spread(TRUE);

    /* Extract the "best" cost */
    i = borg_flow_cost(c_x, c_y);

    /* No good path */
    if (i >= 999) return (FALSE);

    /* Note */
    borg_note(format("Chasing objects, at cost %d", i));

    /* Set the "goal" */
    goal = GOAL_TAKE;

    /* Success */
    return (TRUE);
}





/*
 * Prepare to "flow" towards "interesting" things
 */
static bool borg_flow_symbol(char what)
{
    int i, x, y;

    auto_grid *ag;


    /* Nothing yet */
    seen_n = 0;

    /* Examine every legal grid */
    for (y = 1; y < AUTO_MAX_Y-1; y++) {
        for (x = 1; x < AUTO_MAX_X-1; x++) {

            /* Get the grid */
            ag = grid(x, y);

            /* Skip current location */
            if (pg == ag) continue;

            /* Skip stuff */
            if (ag->o_c == ' ') continue;
            if (!ag->room) continue;

            /* Skip incorrect symbols */
            if (ag->o_c != what) continue;

            /* Remember it */
            if (seen_n < SEEN_MAX) {
                seen_x[seen_n] = x;
                seen_y[seen_n] = y;
                seen_n++;
            }
        }
    }

    /* Nothing useful */
    if (!seen_n) return (FALSE);


    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < seen_n; i++) {

        /* Enqueue the grid */
        borg_flow_enqueue_grid(seen_x[i], seen_y[i]);
    }

    /* Clear the "seen" array */
    seen_n = 0;

    /* Spread the flow (a little) */
    borg_flow_spread(TRUE);

    /* Extract the "best" cost */
    i = borg_flow_cost(c_x, c_y);

    /* No good path */
    if (i >= 999) return (FALSE);

    /* Note */
    borg_note(format("Chasing symbols (%c), at cost %d", what, i));

    /* Set the "goal" */
    goal = GOAL_XTRA;

    /* Success */
    return (TRUE);
}





/*
 * Go shopping in the town (do stores in order)
 */
static bool borg_flow_shop(void)
{
    int i, n = -1, v = 99;

    /* Must be in town */
    if (auto_depth) return (FALSE);

    /* Visit the oldest shop */
    for (i = 0; i < 8; i++) {

        /* Skip recent shops */
        if ((n >= 0) && (auto_shops[i].visit >= v)) continue;

        /* Save this shop */
        n = i; v = auto_shops[i].visit;
    }

    /* Plenty of visits */
    if (v >= last_visit + 2) return (FALSE);

    /* Try and visit it */
    if (borg_flow_symbol('1' + n)) return (TRUE);

    /* Failure */
    return (FALSE);
}




/*
 * Walk around the dungeon looking for monsters
 */
static bool borg_flow_revisit(void)
{
    int x, y, i;

    auto_room *ar;

    int r_n = -1;
    s32b r_age = 0L;

    s32b age;


    /* First find the reachable spaces */
    borg_flow_reverse();

    /* Re-visit "old" rooms */
    for (i = 1; i < auto_room_max; i++) {

        /* Access the "room" */
        ar = &auto_rooms[i];

        /* Skip "dead" rooms */
        if (ar->free) continue;

        /* Skip "unreachable" rooms */
        if (ar->cost >= 999) continue;

        /* Hack -- skip "boring" rooms */
        /* if ((ar->x1 == ar->x2) || (ar->y1 == ar->y2)) continue; */

        /* Reward "age" and "distance" and "luck" */
        age = (c_t - ar->when) + (ar->cost / 2);

        /* Skip "recent" rooms */
        if ((r_n >= 0) && (age < r_age)) continue;

        /* Save the index, and the age */
        r_n = i; r_age = age;
    }

    /* Clear the flow codes */
    borg_flow_clear();

    /* Hack -- No rooms to visit (!) */
    if (r_n < 0) return (FALSE);

    /* Get the room */
    ar = &auto_rooms[r_n];

    /* Visit a random grid of that room */
    x = rand_range(ar->x1, ar->x2);
    y = rand_range(ar->y1, ar->y2);

    /* Enqueue the room */
    borg_flow_enqueue_grid(x, y);

    /* Spread the flow */
    borg_flow_spread(TRUE);

    /* Extract the "best" cost */
    i = borg_flow_cost(c_x, c_y);

    /* No good path */
    if (i >= 999) return (FALSE);

    /* Note */
    borg_note(format("Revisiting (%d, %d) with age %ld, at cost %d",
                     x, y, r_age, i));

    /* Set the "goal" */
    goal = GOAL_XTRA;

    /* Success */
    return (TRUE);
}


/*
 * Heuristic -- Help locate secret doors (see below)
 *
 * Determine the "likelihood" of a secret door touching "(x,y)",
 * which is "d" grids away from the player.
 *
 * Assume grid is legal, non-wall, and reachable.
 * Assume grid neighbors are legal and known.
 */
static int borg_secrecy(int x, int y, int d)
{
    int		i, v;
    int		wall = 0, supp = 0, diag = 0;

    char	cc[8];

    auto_grid	*ag;


    /* No secret doors in town */
    if (auto_depth == 0) return (0);


    /* Get the central grid */
    ag = grid(x, y);

    /* Tweak -- Limit total searches */
    if (ag->xtra > 50) return (0);


    /* Extract adjacent locations */
    for (i = 0; i < 8; i++) {

        /* Extract the location */
        int xx = x + ddx[ddd[i]];
        int yy = y + ddy[ddd[i]];

        /* Get the grid */
        cc[i] = grid(xx,yy)->o_c;
    }


    /* Count possible door locations */
    for (i = 0; i < 4; i++) if (cc[i] == '#') wall++;

    /* No possible secret doors */
    if (wall <= 0) return (0);


    /* Count supporting evidence for secret doors */
    for (i = 0; i < 4; i++) {
        if ((cc[i] == '#') || (cc[i] == '%')) supp++;
        else if ((cc[i] == '+') || (cc[i] == '\'')) supp++;
    }

    /* Count supporting evidence for secret doors */
    for (i = 4; i < 8; i++) {
        if ((cc[i] == '#') || (cc[i] == '%')) diag++;
    }


    /* Tweak -- Reward walls, punish visitation and distance */
    v = (supp * 300) + (diag * 100) - (ag->xtra * 20) - (d * 1);

    /* Result */
    return (v);
}



/*
 * Search carefully for secret doors and such
 */
static bool borg_flow_spastic(void)
{
    int i, x, y, v;
    int g_x = c_x, g_y = c_y, g_v = -1;

    int boredom;

    auto_room *ar;


    /* Hack -- Tweak -- Determine boredom */
    boredom = count_floor - 500;
    if (boredom < 200) boredom = 200;
    if (boredom > 800) boredom = 800;

    /* Tweak -- Search based on dungeon knowledge */
    if (c_t - auto_shock < boredom) return (FALSE);


    /* Hack -- first find the reachable spaces */
    borg_flow_reverse();

    /* Scan the entire map */
    for (y = 1; y < AUTO_MAX_Y-1; y++) {
        for (x = 1; x < AUTO_MAX_X-1; x++) {

            auto_grid *ag = grid(x, y);

            /* Skip stuff */
            if (ag->o_c == ' ') continue;
            if (!ag->room) continue;

            /* Cannot search inside walls */
            if (ag->info & BORG_WALL) continue;

            /* Scan the rooms */
            for (ar = room(1,x,y); ar; ar = room(0,0,0)) {

                /* Skip "unreachable" rooms */
                if (ar->cost >= 999) continue;

                /* Get the "searchability" */
                v = borg_secrecy(x, y, ar->cost);

                /* The grid is not searchable */
                if (v <= 0) continue;

                /* Skip non-perfect grids */
                if ((g_v >= 0) && (v < g_v)) continue;

                /* Save the data */
                g_v = v; g_x = x; g_y = y;
            }
        }
    }

    /* Clear the flow codes */
    borg_flow_clear();

    /* Hack -- Nothing found */
    if (g_v < 0) return (FALSE);


    /* We have arrived */
    if ((c_x == g_x) && (c_y == g_y)) {

        /* Take note */
        borg_note("Spastic Searching...");

        /* Tweak -- Remember the search */
        if (pg->xtra < 100) pg->xtra += 9;

        /* Tweak -- Search a little */
        borg_keypress('0');
        borg_keypress('9');
        borg_keypress('s');

        /* Success */
        return (TRUE);
    }


    /* Enqueue the grid */
    borg_flow_enqueue_grid(g_x, g_y);

    /* Spread the flow a little */
    borg_flow_spread(TRUE);

    /* Extract the "best" cost */
    i = borg_flow_cost(c_x, c_y);

    /* No good path */
    if (i >= 999) return (FALSE);

    /* Note */
    borg_note(format("Spastic twitch towards (%d,%d)", g_x, g_y));

    /* Set the "goal" */
    goal = GOAL_XTRA;

    /* Success */
    return (TRUE);
}





/*
 * Given a location, attempt to determine what type of monster is
 * at that location.  Use type "0" for "no known monster".
 *
 * This guess can be improved by the judicious use of a specialized
 * "attr/char" mapping, especially for unique monsters.  Note that
 * we cheat by directly accessing the attr/char tables.
 *
 * We will never correctly identify player ghosts
 * We will usually fail to identify multi-hued monsters
 * We will usually fail for very out of depth monsters
 * We will attempt to err on the side of caution
 *
 * We treat the "town" level specially, and unless a player ghost
 * appears in town, our guess will always be correct.
 */
static int guess_monster(int x, int y)
{
    int i;

    auto_grid *ag = grid(x, y);


    /* Efficiency */
    if (ag->o_c == ' ') return (0);
    if (ag->o_c == '.') return (0);
    if (ag->o_c == '#') return (0);


    /* Special treatment of "town" monsters */
    if (auto_depth <= 0) {

        /* Hack -- Find the first "acceptable" monster */
        for (i = 1; i < MAX_R_IDX-1; i++) {

            monster_race *r_ptr = &r_list[i];
            monster_lore *l_ptr = &l_list[i];

            if (r_ptr->level) break;

            if (ag->o_c != l_ptr->l_char) continue;
            if (ag->o_a != l_ptr->l_attr) continue;

            return (i);
        }
    }

    /* Guess what monster it might be */
    else {

        /* Hack -- Find the highest "acceptable" monster */
        for (i = (MAX_R_IDX-1) - 1; i > 0; i--) {

            monster_race *r_ptr = &r_list[i];
            monster_lore *l_ptr = &l_list[i];

            if (r_ptr->level > auto_depth + 5) continue;

            if (ag->o_c != l_ptr->l_char) continue;
            if (ag->o_a != l_ptr->l_attr) continue;

            return (i);
        }
    }


    /* Oops */
    return (0);
}


/*
 * Count the monsters touching a location
 * Assumes the location is not an outer wall
 */
static int borg_count_monsters(int x0, int y0)
{
    int i, d, x, y, n = 0;

    auto_grid *ag;

    /* Look around */
    for (i = 0; i < 8; i++) {

        /* Access the grid */
        d = ddd[i];
        x = x0 + ddx[d];
        y = y0 + ddy[d];
        ag = grid(x, y);

        /* Look for monsters (efficiently) */
        if (auto_is_kill[(byte)(ag->o_c)]) n++;
    }

    /* Return the count */
    return (n);
}


/*
 * Target a location
 */
static bool borg_target(int x, int y)
{
    int x1, y1, x2, y2;

    /* Hack -- Cannot target off-screen location */
    if ((x < w_x) || (x >= w_x + SCREEN_WID)) return (FALSE);
    if ((y < w_y) || (y >= w_y + SCREEN_HGT)) return (FALSE);

    /* Log */
    borg_info("Targetting a location.");

    /* Target the location */
    borg_keypress('*');
    borg_keypress('p');

    /* Determine "path" */
    x1 = c_x; y1 = c_y; x2 = x; y2 = y;

    /* Move to the location */
    for ( ; y1 < y2; y1++) borg_keypress('2');
    for ( ; y1 > y2; y1--) borg_keypress('8');
    for ( ; x1 < x2; x1++) borg_keypress('6');
    for ( ; x1 > x2; x1--) borg_keypress('4');

    /* Select the target */
    borg_keypress('t');

    /* Success */
    return (TRUE);
}



/*
 * Attempt to fire at monsters
 */
static bool borg_play_fire(void)
{
    int i, k, r;

    int f_x = c_x, f_y = c_y, f_d = 999;


    /* Look for something to kill */
    for (i = 0; i < view_n; i++) {

        /* Access the "view grid" */
        int y = view_y[i];
        int x = view_x[i];

        /* Get the auto_grid */
        auto_grid *ag = grid(x, y);

        /* Assume no monster */
        bool kill = FALSE;

        /* Skip the player grid */
        if (ag == pg) continue;

        /* Skip unknown grids */
        if (ag->o_c == ' ') continue;

        /* Cannot shoot into walls */
        if (ag->info & BORG_WALL) continue;

        /* Should not shoot at "dark" grids */
        if (!(ag->info & BORG_OKAY)) continue;

        /* Notice monsters (efficiently) */
        if (auto_is_kill[(byte)(ag->o_c)]) kill = TRUE;

        /* XXX XXX XXX Mega-Hack -- hit mushrooms sometimes */
        if ((ag->o_c == ',') && (rand_int(10) == 0)) kill = TRUE;

        /* Skip non-monsters */
        if (!kill) continue;

        /* Must have "missile line of sight" */
        if (!borg_projectable(c_x, c_y, x, y)) continue;

        /* Check distance */
        k = distance(c_y, c_x, y, x);

        /* Must be within range */
        if (k > 10) continue;

        /* Must not be adjacent (unless afraid) */
        if (!do_afraid && (k <= 1)) continue;

        /* Skip far away monsters */
        if (k > f_d) continue;

        /* Save the distance */
        f_d = k; f_x = x; f_y = y;
    }

    /* Hack -- try firing missiles */
    if (f_d >= 999) return (FALSE);


    /* Only a one in five chance (unless afraid) */
    if (!do_afraid && (rand_int(5) != 0)) return (FALSE);


    /* Find a "missile" */
    i = borg_choose_missile();

    /* Nothing to fire */
    if (i < 0) return (FALSE);


    /* Target the monster (if possible) */
    if (!borg_target(f_x, f_y)) return (FALSE);


    /* Acquire monster type */
    r = guess_monster(f_x, f_y);

    /* Note */
    borg_note(format("Launching (%s).", r_list[r].name));

    /* Fire at the target */
    borg_keypress('f');
    borg_keypress('a' + i);
    borg_keypress('t');


    /* Success */
    return (TRUE);
}



/*
 * Hack -- be cautious
 */
static int borg_caution(void)
{
    int x, y, i, d, k, o_n, g_n, g_d = -1;

    auto_grid *ag;


    /* Look around */
    o_n = borg_count_monsters(c_x, c_y);


    /* Require an adjacent monster */
    if (o_n < 1) return (FALSE);

    /* XXX XXX XXX Hack -- attempt to teleport to safety */
    if (auto_curhp < auto_maxhp / 3) {

        /* Read scrolls of teleport (low failure rate) */
        if (borg_action('r', kind_scroll_teleport)) return (TRUE);

        /* Mega-Hack -- Use (charged) staffs of teleport */
        if (borg_action_pval('u', kind_staff_teleport)) return (TRUE);
    }


    /* Ignore small numbers of monsters */
    if (o_n < 2) return (FALSE);

    /* XXX XXX XXX Hack -- attempt to teleport to safety */
    if (auto_curhp < auto_maxhp / 2) {

        /* Mega-Hack -- Use (charged) staffs of teleport */
        if (borg_action_pval('u', kind_staff_teleport)) return (TRUE);

        /* Read scrolls of teleport */
        if (borg_action('r', kind_scroll_teleport)) return (TRUE);
    }


    /* Goal minimum */
    g_n = o_n;

    /* Attempt to find a better grid */
    for (i = 0; i < 8; i++) {

        /* Access the grid */
        d = ddd[i];
        x = c_x + ddx[d];
        y = c_y + ddy[d];
        ag = grid(x, y);

        /* Skip walls (!) */
        if (ag->info & BORG_WALL) continue;
        
        /* Skip unknown grids */
        if (ag->o_c == ' ') continue;

        /* Skip monsters (efficiently) */
        if (auto_is_kill[(byte)(ag->o_c)]) continue;

        /* Hack -- Skip traps */
        if (ag->o_c == '^') continue;

        /* Mega-Hack -- Skip stores */
        if (isdigit(ag->o_c)) continue;

        /* Count the monsters */
        k = borg_count_monsters(x, y);

        /* Require fewer monsters */
        if (k >= g_n) continue;

        /* Save the info */
        g_n = k; g_d = d;
    }

    /* Nowhere to run */
    if (g_d < 0) return (FALSE);


    /* Only be cautious occasionally */
    if (rand_int(3) != 0) return (FALSE);

    /* Note */
    borg_note(format("Caution (%d > %d).", o_n, g_n));

    /* Hack -- Flee! */
    borg_keypress('0' + g_d);

    /* Success */
    return (TRUE);
}


/*
 * Attack monsters (unless afraid)
 */
static int borg_attack(void)
{
    int i, r, n = 0, g_d = -1, g_r = -1;

    /* Check adjacent grids */
    for (i = 0; i < 8; i++) {

        /* Direction */
        int d = ddd[i];

        /* Location */
        int x = c_x + ddx[d];
        int y = c_y + ddy[d];

        /* Grid */
        auto_grid *ag = grid(x, y);

        /* Skip non-monsters (efficiently) */
        if (!auto_is_kill[(byte)(ag->o_c)]) continue;

        /* Count monsters */
        n++;

        /* Guess the monster */
        r = guess_monster(x, y);

        /* Mega-Hack -- Use the "hardest" monster */
        if ((g_d >= 0) && (r < g_r)) continue;

        /* Save the info */
        g_d = d; g_r = r;
    }

    /* Nothing to attack */
    if (!n) return (FALSE);

    /* Do not attack when afraid */
    if (do_afraid) {
        borg_note(format("Fearing (%s).", r_list[g_r].name));
        return (FALSE);
    }

    /* Mention monster */
    if (n > 1) borg_note(format("Selecting (%s).", r_list[g_r].name));
    else borg_note(format("Attacking (%s).", r_list[g_r].name));

    /* Hack -- Attack! */
    borg_keypress('0' + g_d);

    /* Success */
    return (TRUE);
}



/*
 * Help the Borg decide if it should flee back to town
 * This function analyzes the inventory for "essential" items
 * It could also verify adequate "resistance" and other stuff
 * If "bored" is true then we have fully explored the current level
 * Proper use of stores requires "waiting" for them to "restock"
 * Note that this "waiting" must be done in the dungeon (level 1)
 */
static bool borg_leave_level_aux(bool bored)
{
    auto_item *item = &auto_items[INVEN_LITE];

    /* Consider the appropriate level */
    int depth = bored ? (auto_depth + 1) : auto_depth;


    /*** Stair scumming ***/

    /* Mega-Hack -- give stores a chance to restock */
    if ((auto_depth <= 1) && !bored) return (FALSE);


    /*** Essential Items ***/

    /* Require Food Rations */
    if (kind_have[kind_food_ration] < 10) return (TRUE);

    /* Require Lite and/or fuel */
    if (item->iqty == 0) return (TRUE);
    if ((item->kind == kind_torch) && (kind_have[kind_torch] < 10)) return (TRUE);
    if ((item->kind == kind_lantern) && (kind_have[kind_flask] < 10)) return (TRUE);


    /*** Useful Items ***/

    /* Scrolls of Identify */
    if (kind_have[kind_scroll_identify] < 5) return (TRUE);
    if (bored && (kind_have[kind_scroll_identify] < 10)) return (TRUE);

    /* Scrolls of Word of Recall */
    if ((depth >= 2) && (kind_have[kind_scroll_recall] < 1)) return (TRUE);
    if ((depth >= 5) && (kind_have[kind_scroll_recall] < 2)) return (TRUE);
    if ((depth >= 5) && bored && (kind_have[kind_scroll_recall] < 4)) return (TRUE);

    /* Potions of Cure Serious Wounds */
    if ((depth >= 2) && (kind_have[kind_potion_serious] < 2)) return (TRUE);

    /* Potions of Cure Critical Wounds */
    if ((depth >= 5) && (kind_have[kind_potion_critical] < 5)) return (TRUE);
    if ((depth >= 5) && bored && (kind_have[kind_potion_critical] < 10)) return (TRUE);


    /* Stay here */
    return (FALSE);
}



/*
 * Leave the level if necessary (or bored)
 */
static bool borg_leave_level(bool bored)
{
    int k, g = 0;


    /* Cancel stair requests */
    stair_less = stair_more = FALSE;


    /* Hack -- already leaving */
    if (auto_recall) return (FALSE);


    /* Count sellable items */
    k = borg_count_sell();


    /* Power-dive */
    if (auto_depth && (auto_level > auto_depth * 2)) g = 1;

    /* Dive when bored */
    if (auto_depth && bored && (c_t - auto_shock > 1000L)) g = 1;

    /* Mega-Hack -- Do not leave a level until "bored" */
    if (c_t - auto_began < attention) g = 0;

    /* Do not dive when "full" of items */
    if (g && (k >= 18)) g = 0;

    /* Hack -- do not dive when drained */
    if (do_fix_exp) g = 0;


    /* Return to town (immediately) if we are worried */
    if (borg_leave_level_aux(bored)) goal_rising = TRUE;

    /* Return to town to sell stuff */
    if (bored && (k >= 18)) goal_rising = TRUE;

    /* Hack -- return to town to restore experience */
    if (bored && do_fix_exp) goal_rising = TRUE;

    /* Never rise from town */
    if (auto_depth == 0) goal_rising = FALSE;

    /* Return to town */
    if (goal_rising) g = -1;


    /* Bored and in town, go down */
    if (bored && (auto_depth == 0)) g = 1;


    /* Use random stairs when really bored */
    if (auto_depth && bored && (c_t - auto_shock > 3000L)) {

        /* Use random stairs */
        g = (rand_int(100) < 50) ? -1 : 1;
    }


    /* Go Up */
    if (g < 0) {

        /* Hack -- use scroll of recall if able */
        if (goal_rising && (auto_depth > 4) && (kind_have[kind_scroll_recall] > 3)) {
            if (borg_recall()) return (TRUE);
        }

        /* Attempt to use stairs */
        if (count_less && borg_flow_symbol('<')) {
            borg_note("Returning to town.");
            stair_less = TRUE;
            if (borg_play_old_goal()) return (TRUE);
        }

        /* Cannot find any stairs, try word of recall */
        if (goal_rising && bored) {
            if (borg_recall()) return (TRUE);
        }
    }


    /* Go Down */
    if (g > 0) {

        /* Attempt to use those stairs */
        if (count_more && borg_flow_symbol('>')) {
            borg_note("Diving into the dungeon.");
            stair_more = TRUE;
            if (borg_play_old_goal()) return (TRUE);
        }
    }


    /* Failure */
    return (FALSE);
}






/*
 * Buy items from the current store
 */
static bool borg_think_store_buy(void)
{
    int k, n;
    s32b p;

    int b_k = -1, b_n = -1;
    s32b b_p = 0L;


    /* XXX Hack -- notice "full" inventory */
    if (auto_items[INVEN_PACK-1].iqty) return (FALSE);


    /* Visit everyone before buying anything */
    for (k = 0; k < 8; k++) {

        /* Make sure we visited that shop */
        if (!auto_shops[k].visit) return (FALSE);
    }

    /* Check each store */
    for (k = 0; k < 8; k++) {
    
        auto_shop *shop = &auto_shops[k];

        /* Buy stuff */
        for (n = 0; n < 24; n++) {

            auto_item *ware = &shop->ware[n];

            /* Notice end of shop inventory */
            if (!ware->iqty) break;

            /* We must have "sufficient" cash */
            if (auto_gold < ware->cost * 2) continue;

            /* Only buy useful stuff */
            if (!borg_good_buy(ware)) continue;

            /* Extract the worth of this item */
            p = item_power(ware);

            /* Hack -- notice the "best" thing */
            if ((b_n >= 0) && (p < b_p)) continue;

            /* Save the item and worth */
            b_k = k; b_n = n; b_p = p;
        }
    }

    /* Buy the "best" item, if any, if in this store */
    if ((b_n >= 0) && (shop_num == b_k)) {

        auto_shop *shop = &auto_shops[shop_num];
        auto_item *ware = &shop->ware[b_n];

        /* Minor Hack -- Go to the correct page */
        if ((b_n / 12) != auto_shops[shop_num].page) borg_keypress(' ');

        /* Log */
        borg_info(format("Buying %s.", auto_shops[shop_num].ware[b_n].desc));

        /* Buy an item */
        borg_keypress('p');

        /* Buy the desired item */
        borg_keypress('a' + (b_n % 12));

        /* Hack -- Buy a single item */
        if (ware->iqty > 1) borg_keypress('\n');

        /* Hack -- Accept the price */
        borg_keypress('\n');
        borg_keypress('\n');
        borg_keypress('\n');
        borg_keypress('\n');

        /* Hack -- Note visit index */
        if (last_visit < shop->visit) last_visit = shop->visit;

        /* Success */
        return (TRUE);
    }


    /* Nothing to buy */
    return (FALSE);
}


/*
 * Sell items to the current store
 */
static bool borg_think_store_sell(void)
{
    int i, p;

    int b_n = -1, b_p = 0L;


    /* XXX Hack -- notice "full" store */
    if (auto_shops[shop_num].ware[23].iqty) return (FALSE);


    /* Sell stuff */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Consider "cash" items */
        if (!item->cash) continue;

        /* Do not try to make "bad" sales */
        if (!borg_good_sell(item)) continue;

        /* Extract the worth of this item */
        p = item_worth(item);

        /* Hack -- notice the "best" thing */
        if ((b_n >= 0) && (p < b_p)) continue;

        /* Save the item and worth */
        b_n = i; b_p = p;
    }

    /* Sell the most expensive item (if any) */
    if (b_n >= 0) {

        auto_item *item = &auto_items[b_n];

        /* Log */
        borg_info(format("Selling %s.", auto_items[b_n].desc));

        /* Sell that item */
        borg_keypress('s');
        borg_keypress('a' + b_n);

        /* Hack -- Sell a single item */
        if (item->iqty > 1) borg_keypress('\n');

        /* Hack -- Accept the price */
        borg_keypress('\n');
        borg_keypress('\n');
        borg_keypress('\n');
        borg_keypress('\n');

        /* Success */
        return (TRUE);
    }


    /* Assume not */
    return (FALSE);
}


/*
 * Deal with being in a store
 */
static bool borg_think_store(void)
{
    int i, n;

    byte t_a;

    char /* p1 = '(', */ p2 = ')';

    char desc[80];
    char cost[10];

    char buf[256];


    /* Hack -- make sure both pages are seen */
    static int browse = TRUE;

    /* What store did I *think* I was in */
    static int old_store = 0;

    /* How many pages did I *think* there were */
    static int old_more = 0;


    /* Hack -- reset page/more */
    auto_shops[shop_num].page = 0;
    auto_shops[shop_num].more = 0;


    /* React to new stores */
    if (old_store != shop_num) {

        /* Clear all the items */
        for (i = 0; i < 24; i++) {

            /* XXX Wipe the ware */
            WIPE(&auto_shops[shop_num].ware[i], auto_item);
        }

        /* Save the store */
        old_store = shop_num;
    }


    /* Extract the "page", if any */
    if ((0 == borg_what_text(20, 5, 8, &t_a, buf)) &&
        (prefix(buf, "(Page "))) /* --)-- */ {

        /* Take note of the page */
        auto_shops[shop_num].more = 1;
        auto_shops[shop_num].page = (buf[6] - '0') - 1;
    }

    /* React to disappearing pages */
    if (old_more != auto_shops[shop_num].more) {

        /* Clear the second page */
        for (i = 12; i < 24; i++) {

            /* XXX Wipe the ware */
            WIPE(&auto_shops[shop_num].ware[i], auto_item);
        }

        /* Save the new one */
        old_more = auto_shops[shop_num].more;
    }


    /* Extract the current gold (unless in home) */
    if (0 == borg_what_text(68, 19, -9, &t_a, buf)) {

        /* Ignore this "field" in the home */
        if (shop_num != 7) auto_gold = atol(buf);
    }


    /* Parse the store (or home) inventory */
    for (i = 0; i < 12; i++) {

        /* Default to "empty" */
        desc[0] = '\0';
        cost[0] = '\0';

        /* Verify "intro" to the item */
        if ((0 == borg_what_text(0, i + 6, 3, &t_a, buf)) &&
            (buf[0] == 'a' + i) && (buf[1] == p2) && (buf[2] == ' ')) {

            /* Extract the item description */
            if (0 != borg_what_text(3, i + 6, -65, &t_a, desc)) desc[0] = '\0';

            /* XXX Make sure trailing spaces get stripped */

            /* Extract the item cost */
            if (0 != borg_what_text(68, i + 6, -9, &t_a, cost)) cost[0] = '\0';

            /* Hack -- forget the cost in the home */
            if (shop_num == 7) cost[0] = '\0';
        }

        /* Extract actual index */
        n = auto_shops[shop_num].page * 12 + i;

        /* Ignore "unchanged" descriptions */
        if (streq(desc, auto_shops[shop_num].ware[n].desc)) continue;

        /* Analyze it (including the cost) */
        borg_item_analyze(&auto_shops[shop_num].ware[n], desc, cost);
    }


    /* Hack -- browse as needed */
    if (auto_shops[shop_num].more && browse) {
        borg_keypress(' ');
        browse = FALSE;
        return (TRUE);
    }

    /* Hack -- must browse later */
    browse = TRUE;


    /* Complete and Total Hack */
    if (rand_int(1000) == 0) {

        bool done = FALSE;

        /* Take off the left ring */
        if (auto_items[INVEN_LEFT].iqty) {
            borg_keypress('t');
            borg_keypress('c');
            done = TRUE;
        }

        /* Take off the right ring */
        if (auto_items[INVEN_RIGHT].iqty) {
            borg_keypress('t');
            borg_keypress('d');
            done = TRUE;
        }

        /* Success */
        if (done) return (TRUE);
    }


    /* Examine the inventory */
    borg_notice();

    /* Wear things */
    if (borg_wear_stuff()) return (TRUE);

    /* Try to sell stuff */
    if (borg_think_store_sell()) return (TRUE);

    /* Try to buy stuff */
    if (borg_think_store_buy()) return (TRUE);


    /* Count the successful visits */
    auto_shops[shop_num].visit++;


    /* Leave the store */
    borg_keypress(ESCAPE);

    /* Done */
    return (TRUE);
}



/*
 * Perform an action in the dungeon
 *
 * Return TRUE if a "meaningful" action was performed
 * Otherwise, return FALSE so we will be called again
 *
 * Strategy:
 *   Make sure we are happy with our "status" (see above)
 *   Attack and kill visible monsters, if near enough
 *   Open doors, disarm traps, tunnel through rubble
 *   Pick up (or tunnel to) gold and useful objects
 *   Explore "interesting" grids, to expand the map
 *   Explore the dungeon and revisit old grids
 */
static bool borg_think_dungeon(void)
{
    /* Examine the screen */
    borg_update();

    /* Analyze the inventory */
    borg_notice();

    /* Paranoia -- catch death */
    if (!pg || !auto_active) return (TRUE);

    /* Wear things that need to be worn */
    if (borg_wear_stuff()) return (TRUE);

    /* Use things */
    if (borg_use_things()) return (TRUE);

    /* Flee to the town when low on crucial supplies */
    if (borg_restock() && borg_recall()) return (TRUE);

    /* Flee the level after a long time */
    if ((c_t - auto_began > 50000L) && borg_recall()) return (TRUE);


    /*** Deal with monsters ***/

    /* Apply a tiny bit of chaos */
    if (rand_int(1000) == 0) {
        borg_keypress('0' + randint(9));
        return (TRUE);
    }

    /* Try not to get surrounded by monsters */
    if (borg_caution()) return (TRUE);

    /* Fire at nearby monsters */
    if (borg_play_fire()) return (TRUE);

    /* Attack neighboring monsters */
    if (borg_attack()) return (TRUE);

    /* Continue flowing towards monsters */
    if (goal == GOAL_KILL) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Apply a tiny amount of chaos */
    if (rand_int(200) == 0) {
        borg_keypress('0' + randint(9));
        return (TRUE);
    }

    /* Rest occasionally if damaged */
    if ((auto_curhp < auto_maxhp) && (rand_int(10) == 0)) {

        /* Take note */
        borg_note("Resting...");

        /* Rest until done */
        borg_keypress('R');
        borg_keypress('&');
        borg_keypress('\n');

        /* Done */
        return (TRUE);
    }

    /* Start a new one */
    if (borg_flow_kill()) {
        if (borg_play_old_goal()) return (TRUE);
    }


    /*** Deal with inventory objects ***/

    /* Use other things */
    if (borg_use_others()) return (TRUE);

    /* Identify unknown things */
    if (borg_test_stuff()) return (TRUE);

    /* Throw away junk */
    if (borg_throw_junk()) return (TRUE);

    /* Acquire free space */
    if (borg_free_space()) return (TRUE);


    /*** Flow towards objects ***/

    /* Hack -- beware of blindness and confusion */
    if ((do_blind || do_confused) && (rand_int(2) == 0)) {

        /* Take note */
        borg_note("Resting...");

        /* Rest until done */
        borg_keypress('R');
        borg_keypress('&');
        borg_keypress('\n');

        /* Done */
        return (TRUE);
    }
    
    /* Continue flowing to objects */
    if (goal == GOAL_TAKE) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Rest occasionally */
    if ((auto_curhp < auto_maxhp) && (rand_int(2) == 0)) {

        /* Take note */
        borg_note("Resting...");

        /* Rest until done */
        borg_keypress('R');
        borg_keypress('&');
        borg_keypress('\n');

        /* Done */
        return (TRUE);
    }

    /* Start a new one */
    if (borg_flow_take()) {
        if (borg_play_old_goal()) return (TRUE);
    }


    /*** Exploration ***/

    /* Hack -- Search intersections for doors */
    if (borg_inspect()) return (TRUE);

    /* Continue flowing (explore) */
    if (goal == GOAL_DARK) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Continue flowing (see below) */
    if (goal == GOAL_XTRA) {
        if (borg_play_old_goal()) return (TRUE);
    }


    /*** Try grids that are farther away ***/

    /* Chase old monsters */
    if (borg_flow_kill_any()) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Chase old objects */
    if (borg_flow_take_any()) {
        if (borg_play_old_goal()) return (TRUE);
    }


    /*** Wander around ***/

    /* Hack -- occasionally update the "free room" list */
    /* if (rand_int(100) == 0) borg_free_room_update(); */

    /* Leave the level (if needed) */
    if (borg_leave_level(FALSE)) return (TRUE);

    /* Explore nearby interesting grids */
    if (borg_flow_dark()) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Explore interesting grids */
    if (borg_flow_explore()) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Hack -- Visit the shops */
    if (borg_flow_shop()) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Leave the level (if possible) */
    if (borg_leave_level(TRUE)) return (TRUE);

    /* Search for secret doors */
    if (borg_flow_spastic()) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Re-visit old rooms */
    if (borg_flow_revisit()) {
        if (borg_play_old_goal()) return (TRUE);
    }


    /*** Oops ***/

    /* This is a bad thing */
    borg_note("Twitchy!");

    /* Try searching */
    if (rand_int(10) == 0) {

        /* Take note */
        borg_note("Searching...");

        /* Remember the search */
        if (pg->xtra < 100) pg->xtra += 9;

        /* Search */
        borg_keypress('0');
        borg_keypress('9');
        borg_keypress('s');

        /* Success */
        return (TRUE);
    }

    /* Mega-Hack -- Occasional tunnel */
    if (rand_int(100) == 0) {

        /* Tunnel a lot */
        borg_keypress('0');
        borg_keypress('9');
        borg_keypress('9');
        borg_keypress('T');

        /* In a legal direction */
        borg_keypress('0' + ddd[rand_int(8)]);
    }

    /* Move (or tunnel) */
    borg_keypress('0' + randint(9));

    /* We did something */
    return (TRUE);
}


/*
 * Think about the world and perform an action
 * Check inventory/equipment once per turn
 * Process "store" modes when necessary
 *
 * Note that the non-cheating "inventory" and "equipment" parsers
 * will get confused by a "weird" situation involving an ant ("a")
 * on line one of the screen, near the left, next to a shield, of
 * the same color, and using --(-- the ")" symbol, directly to the
 * right of the ant.  This is very rare, but perhaps not completely
 * impossible.  I ignore this situation.  :-)
 */
static bool borg_think(void)
{
    int i;

    byte t_a;

    char /* p1 = '(', */ p2 = ')';

    char buf[128];

    static bool do_inven = TRUE;
    static bool do_equip = TRUE;

    static bool do_panel = TRUE;


    /*** Process inventory/equipment ***/

    /* Cheat */
    if (cheat_inven) {

        /* Extract the inventory */
        for (i = 0; i < INVEN_TOTAL; i++) {

            char buf[256];

            /* Default to "nothing" */
            buf[0] = '\0';

            /* XXX XXX Extract a real item */
            if (inventory[i].tval) {
                objdes(buf, &inventory[i], TRUE);
                buf[75] = '\0';
            }

            /* Ignore "unchanged" items */
            if (streq(buf, auto_items[i].desc)) continue;

            /* Analyze the item (no price) */
            borg_item_analyze(&auto_items[i], buf, "");
        }

        /* No need to get the data */
        do_inven = do_equip = FALSE;
    }


    /* Hack -- Check for "inventory" mode */
    if ((0 == borg_what_text(0, 0, 10, &t_a, buf)) &&
       (streq(buf, "Equipment "))) {

        int row, col;

        bool done = FALSE;

        /* Find the column */
        for (col = 0; col < 55; col++) {

            /* Look for first prefix */
            if ((0 == borg_what_text_hack(col, 1, 3, &t_a, buf)) &&
                (buf[0] == 'a') && (buf[1] == p2) && (buf[2] == ' ')) {

                break;
            }
        }

        /* Extract the inventory */
        for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {

            /* Access the row */
            row = i - INVEN_WIELD;

            /* Attempt to get some text */
            if (!done &&
                (0 == borg_what_text_hack(col, row+1, 3, &t_a, buf)) &&
                (buf[0] == 'a' + row) && (buf[1] == p2) && (buf[2] == ' ') &&
                (0 == borg_what_text(col+19, row+1, -80, &t_a, buf)) &&
                (buf[0] && (buf[0] != ' '))) {
                /* XXX Strip final spaces */
            }

            /* Default to "nothing" */
            else {
                buf[0] = '\0';
                done = TRUE;
            }

            /* Use the nice "show_empty_slots" flag */
            if (streq(buf, "(nothing)")) strcpy(buf, "");

            /* Ignore "unchanged" items */
            if (streq(buf, auto_items[i].desc)) continue;

            /* Analyze the item (no price) */
            borg_item_analyze(&auto_items[i], buf, "");
        }

        /* Hack -- leave this mode */
        borg_keypress(ESCAPE);

        /* Done */
        return (TRUE);
    }


    /* Hack -- Check for "inventory" mode */
    if ((0 == borg_what_text(0, 0, 10, &t_a, buf)) &&
       (streq(buf, "Inventory "))) {

        int row, col;

        bool done = FALSE;

        /* Find the column */
        for (col = 0; col < 55; col++) {

            /* Look for first prefix */
            if ((0 == borg_what_text_hack(col, 1, 3, &t_a, buf)) &&
                (buf[0] == 'a') && (buf[1] == p2) && (buf[2] == ' ')) {

                break;
            }
        }

        /* Extract the inventory */
        for (i = 0; i < INVEN_PACK; i++) {

            /* Access the row */
            row = i;

            /* Attempt to get some text */
            if (!done &&
                (0 == borg_what_text_hack(col, row+1, 3, &t_a, buf)) &&
                (buf[0] == 'a' + row) && (buf[1] == p2) && (buf[2] == ' ') &&
                (0 == borg_what_text(col+3, row+1, -80, &t_a, buf)) &&
                (buf[0] && (buf[0] != ' '))) {
                /* XXX Strip final spaces */
            }

            /* Default to "nothing" */
            else {
                buf[0] = '\0';
                done = TRUE;
            }

            /* Use the nice "show_empty_slots" flag */
            /* if (streq(buf, "(nothing)")) strcpy(buf, ""); */

            /* Ignore "unchanged" items */
            if (streq(buf, auto_items[i].desc)) continue;

            /* Analyze the item (no price) */
            borg_item_analyze(&auto_items[i], buf, "");
        }

        /* Hack -- leave this mode */
        borg_keypress(ESCAPE);

        /* Done */
        return (TRUE);
    }


    /* Check equipment */
    if (do_equip) {
        do_equip = FALSE;
        borg_keypress('e');
        return (TRUE);
    }

    /* Check inventory */
    if (do_inven) {
        do_inven = FALSE;
        borg_keypress('i');
        return (TRUE);
    }



    /*** Handle stores ***/

    /* Hack -- Check for being in a store */
    if ((0 == borg_what_text(3, 5, 16, &t_a, buf)) &&
       (streq(buf, "Item Description"))) {

        /* Assume the Home */
        shop_num = 7;

        /* Extract the "store" name */
        if (0 == borg_what_text(50, 3, -20, &t_a, buf)) {

            int i;

            /* Check the store names */
            for (i = 0; i < 7; i++) {
                cptr name = k_list[OBJ_STORE_LIST+i].name;
                if (streq(buf, name)) shop_num = i;
            }
        }

#if 0
        /* Note the store */
        borg_note(format("Inside store '%d' (%s)", shop_num + 1,
                         k_list[OBJ_STORE_LIST+shop_num].name));
#endif

        /* Hack -- Check inventory/equipment again later */
        do_inven = do_equip = TRUE;

        /* Process the store */
        return (borg_think_store());
    }


    /*** Determine panel ***/

    /* Hack -- cheat */
    if (cheat_panel) {

        /* Cheat */
        w_y = panel_row * (SCREEN_HGT / 2);
        w_x = panel_col * (SCREEN_WID / 2);

        /* Done */
        do_panel = FALSE;
    }

    /* Hack -- Check for "sector" mode */
    if ((0 == borg_what_text(0, 0, 16, &t_a, buf)) &&
       (prefix(buf, "Map sector "))) {

        /* Hack -- get the panel info */
        w_y = (buf[12] - '0') * (SCREEN_HGT / 2);
        w_x = (buf[14] - '0') * (SCREEN_WID / 2);

        /* Leave panel mode */
        borg_keypress(ESCAPE);

        /* Done */
        return (TRUE);
    }

    /* Check equipment */
    if (do_panel) {
        do_panel = FALSE;
        borg_keypress('L');
        return (TRUE);
    }


    /*** Re-activate Tests ***/

    /* Check inventory/equipment again later */
    do_inven = do_equip = TRUE;

    /* Check panel again later */
    do_panel = TRUE;


    /*** Actually Do Something ***/

    /* Repeat until done */
    while (auto_active) {

        /* Do domething */
        if (borg_think_dungeon()) break;
    }

    /* Assume success */
    return (TRUE);
}



/*
 * Hack -- Parse a message from the world
 *
 * Note that detecting failures is EXTREMELY important, to prevent
 * bizarre situations after failing to use a staff of perceptions,
 * which would otherwise go ahead and send the "item index" which
 * might be a legal command (such as "a" for "aim").
 *
 * Currently, nothing else must be parsed, though some interesting
 * possibilities suggest themselves... :-)
 */
static void borg_parse(char *msg)
{
    /* Log (if needed) */
    if (auto_fff) borg_info(format("Msg <%s>", msg));


    /* Detect various "failures" */
    if (prefix(msg, "You failed ")) {

        /* Mega-Hack -- flush the key-buffer */
        borg_flush();
    }

 #if 0

    /* Hit somebody */
    else if (prefix(msg, "You hit ")) {

        /* XXX Determine identity */
    }

    /* Miss somebody */
    else if (prefix(msg, "You miss ")) {

        /* XXX Determine identity */
    }

    /* Hack -- detect word of recall */
    else if (prefix(msg, "The air about you becomes charged.")) {

        /* Note the time */
        auto_recall = c_t;
    }

    /* Hack -- detect word of recall */
    else if (prefix(msg, "A tension leaves the air around you.")) {

        /* Hack -- Oops */
        auto_recall = 0;
    }

    /* Word of recall fizzles out */
    else if (prefix(msg, "You feel tension leave the air.")) {

        /* Note the time */
        auto_recall = 0;
    }

    /* Word of recall kicks in */
    else if (prefix(msg, "You feel yourself yanked ")) {

        /* Note the time */
        auto_recall = 0;
    }

#endif

    /* Feelings */
    else if (prefix(msg, "You feel there is something special")) {

        /* Hang around */
        attention = 2000L;
    }

    /* Feelings */
    else if (prefix(msg, "You have a superb feeling")) {

        /* Hang around */
        attention = 2000L;
    }

    /* Feelings */
    else if (prefix(msg, "You have an excellent feeling")) {

        /* Hang around */
        attention = 1500L;
    }

    /* Feelings */
    else if (prefix(msg, "You have a very good feeling")) {

        /* Hang around */
        attention = 1000L;
    }

    /* Feelings */
    else if (prefix(msg, "You have a good feeling")) {

        /* Hang around */
        attention = 500L;
    }

    /* Feelings */
    else if (prefix(msg, "You feel strangely lucky")) {

        /* Stair scum */
        attention = 200L;
    }

    /* Feelings */
    else if (prefix(msg, "You feel your luck is turning")) {

        /* Stair scum */
        attention = 200L;
    }

    /* Feelings */
    else if (prefix(msg, "You like the look of this place")) {

        /* Stair scum */
        attention = 200L;
    }

    /* Feelings */
    else if (prefix(msg, "This level can't be all bad")) {

        /* Stair scum */
        attention = 200L;
    }

    /* Feelings */
    else if (prefix(msg, "What a boring place")) {

        /* Stair scum */
        attention = 200L;
    }

    /* Feelings */
    else if (prefix(msg, "Looks like any other level")) {

        /* Stair scum */
        attention = 200L;
    }
}







/*
 * Maintain the "old" hook
 */
static errr (*Term_xtra_hook_old)(int n, int v) = NULL;


/*
 * Our own hook.  Allow thinking when no keys ready.
 */
static errr Term_xtra_borg(int n, int v)
{
    /* Hack -- Notice "NOISE" events (errors) */
    if (n == TERM_XTRA_NOISE) {

        /* Hack -- Give a message and stop thinking */
        if (auto_active) borg_oops("Caught request for noise.");
    }

    /* Hack -- The Borg pre-empts keypresses */
    while (auto_active && (n == TERM_XTRA_EVENT)) {

        int i, x, y;
        byte t_a;
        char buf[128];

        errr res = 0;

        bool visible;

        static inside = 0;


        /* Paranoia -- require main window */
        if (Term != term_screen) {
            borg_oops("Bizarre request!");
            break;
        }


        /* Paranoia -- prevent recursion */
        if (inside) {
            borg_oops("Bizarre recursion!");
            break;
        }


        /* XXX XXX XXX Mega-Hack -- Check the user */
        res = (*Term_xtra_hook_old)(TERM_XTRA_CHECK, v);

        /* XXX XXX XXX Abort on user keypress (?) */
        /* if (Term_kbhit()) auto_active = FALSE; */


        /* Note that the cursor visibility determines whether the */
        /* game is asking us for a "command" or for some other key. */
        /* XXX XXX This requires that "hilite_player" be FALSE. */


        /* Hack -- Extract the cursor visibility */
        visible = (!Term_hide_cursor());
        if (visible) Term_show_cursor();


        /* XXX XXX XXX Mega-Hack -- Catch "-more-" messages */
        /* If the cursor is visible... */
        /* And the cursor is on the top line... */
        /* And there is text before the cursor... */
        /* And that text is "-more-" */
        if (visible &&
            (0 == Term_locate(&x, &y)) && (y == 0) && (x >= 6) &&
            (0 == borg_what_text(x-6, y, 6, &t_a, buf)) &&
            (streq(buf, "-more-"))) {

            int col = 0;

            /* Get each message */
            while ((0 == borg_what_text(col, 0, -80, &t_a, buf)) &&
                   (t_a == TERM_WHITE) && (buf[0] && (buf[0] != ' '))) {

                /* Advance the column */
                col += strlen(buf) + 1;

                /* Parse */
                borg_parse(buf);
            }

            /* Hack -- Clear the message */
            Term_keypress(' ');

            /* Done */
            return (0);
        }

        /* XXX XXX XXX Mega-Hack -- catch normal messages */
        /* If the cursor is NOT visible... */
        /* And there is text on the first line... */
        if (!visible &&
            (borg_what_text(0, 0, 1, &t_a, buf) == 0) &&
            (t_a == TERM_WHITE) && (buf[0] && (buf[0] != ' '))) {

            int col = 0;

            /* Get each message */
            while ((0 == borg_what_text(col, 0, -80, &t_a, buf)) &&
                   (t_a == TERM_WHITE) && (buf[0] && (buf[0] != ' '))) {

                /* Advance the column */
                col += strlen(buf) + 1;

                /* Parse */
                borg_parse(buf);
            }

            /* Hack -- Clear the message */
            Term_keypress(' ');

            /* Done */
            return (0);
        }


        /* Check for a Borg keypress */
        i = borg_inkey();

        /* Take the next keypress */
        if (i) {

            /* Enqueue the keypress */
            Term_keypress(i);

            /* Success */
            return (0);
        }


        /* Wizard mode kills the Borg */
        if (wizard) auto_active = FALSE;


        /* Inside */
        inside++;

        /* Think until done */
        while (auto_active) {

            /* Actually think */
            if (borg_think()) break;
        }

        /* Outside */
        inside--;
    }


    /* Hack -- Usually just pass the call through */
    return ((*Term_xtra_hook_old)(n, v));
}




/*
 * Initialize the Borg
 */
void borg_init(void)
{
    int i;

    /* Only initialize once */
    if (auto_ready) return;

    /* Message */
    msg_print("Initializing the Borg...");

    /* Hack -- flush it */
    Term_fresh();

    /* Remember the "normal" event scanner */
    Term_xtra_hook_old = Term->xtra_hook;

    /* Cheat -- drop a hook into the "event scanner" */
    Term->xtra_hook = Term_xtra_borg;

    /* Make some arrays */
    C_MAKE(auto_is_take, 128, bool);
    C_MAKE(auto_is_kill, 128, bool);

    /* Optimize "strchr()" calls */
    for (i = 0; i < 128; i++) {

        /* Optimize "strchr(auto_str_take, ag->o_c)" */
        if (strchr("^+:;$*?!_-\\|/\"=~{([])},", i)) auto_is_take[i] = TRUE;

        /* Optimize "strchr(auto_str_kill, ag->o_c)" */
        else if (isalpha(i) || (i == '&')) auto_is_kill[i] = TRUE;
    }

    /* Init the other arrays */
    borg_init_arrays();

    /* Done initialization */
    msg_print("done.");

    /* Now it is ready */
    auto_ready = TRUE;
}


/*
 * Hack -- interact with the Borg.  Includes "initialization" call.
 */
void borg_mode(void)
{
    char cmd;


    /* Not initialized? */
    if (!auto_ready) return;

    /* Currently active? */
    if (auto_active) return;


    /* Get a special command */
    if (!get_com("Borg command: ", &cmd)) return;


    /* Command: Resume */
    if (cmd == 'r') {

        /* Activate (see below) */
        auto_active = TRUE;
    }

    /* Command: Restart -- use an "illegal" level */
    else if (cmd == 'z') {

        /* Activate (see below) */
        auto_active = TRUE;

        /* Hack -- restart the level */
        auto_depth = -1;
    }


    /* Start a new log file */
    else if (cmd == 'f') {

        char buf[1024];

        /* Close the log file */
        if (auto_fff) fclose(auto_fff);

        /* Make a new log file name */
        prt("Borg Log: ", 0, 0);

        /* Hack -- drop permissions */
        safe_setuid_drop();

        /* Get the name and open the log file */
        if (askfor(buf, 70)) auto_fff = fopen(buf, "w");

        /* Hack -- grab permissions */
        safe_setuid_grab();

        /* Failure */
        if (!auto_fff) msg_print("Cannot open that file.");
    }


    /* Command: toggle "cheat" for "inven"/"equip" */
    else if (cmd == 'i') {
        cheat_inven = !cheat_inven;
        msg_print(format("Borg -- cheat_inven is now %d.", cheat_inven));
    }

    /* Command: toggle "cheat" for "panel" */
    else if (cmd == 'p') {
        cheat_panel = !cheat_panel;
        msg_print(format("Borg -- cheat_panel is now %d.", cheat_panel));
    }


    /* Command: toggle "panic_death" */
    else if (cmd == 'd') {
        panic_death = !panic_death;
        msg_print(format("Borg -- panic_death is now %d.", panic_death));
    }

    /* Command: toggle "panic_stuff" */
    else if (cmd == 'j') {
        panic_stuff = !panic_stuff;
        msg_print(format("Borg -- panic_stuff is now %d.", panic_stuff));
    }


    /* Command: Show all Rooms (Containers) */
    else if (cmd == 's') {

        int k, x, y, xx, yy;

        int w_y = panel_row * (SCREEN_HGT / 2);
        int w_x = panel_col * (SCREEN_WID / 2);

        auto_room *ar;

        /* Examine all the rooms */
        for (y = w_y; y < w_y + SCREEN_HGT; y++) {
            for (x = w_x; x < w_x + SCREEN_WID; x++) {

                /* Scan the rooms */
                for (ar = room(1,x,y); ar; ar = room(0,0,0)) {

                    /* Skip done rooms */
                    if ((ar->y1 < y) && (y > w_y)) continue;
                    if ((ar->x1 < x) && (x > w_x)) continue;

                    /* Hack -- hilite the room -- count draws */
                    for (yy = ar->y1; yy <= ar->y2; yy++) {	
                        for (xx = ar->x1; xx <= ar->x2; xx++) {	
                            byte a = TERM_RED;
                            char c = '*';
                            if (grid(xx,yy)->info & BORG_VIEW) a = TERM_YELLOW;
                            if (grid(xx,yy)->o_c == ' ') c = '?';
                            mh_print_rel(c, a, 0, yy, xx);
                        }
                    }

                    /* Describe and wait */
                    Term_putstr(0, 0, -1, TERM_WHITE,
                                format("Room %d (%dx%d). ", ar->self,
                                (1 + ar->x2 - ar->x1), (1 + ar->y2 - ar->y1)));

                    /* Get a key */
                    k = inkey();

                    /* Hack -- hilite the room -- count draws */
                    for (yy = ar->y1; yy <= ar->y2; yy++) {	
                        for (xx = ar->x1; xx <= ar->x2; xx++) {	
                            lite_spot(yy, xx);
                        }
                    }

                    /* Erase the prompt */
                    Term_erase(0, 0, 80-1, 0);

                    /* Flush the erase */
                    Term_fresh();

                    /* Leave the outer loop */
                    if (k == ESCAPE) x = y = 999;

                    /* Leave this loop */
                    if (k == ESCAPE) break;
                }
            }
        }
    }

    /* Command: Rooms (Containers) */
    else if (cmd == 'c') {

        int n, w, h;

        auto_room *ar;

        int used = 0, n_1x1 = 0, n_1xN = 0, n_Nx1 = 0;
        int n_2x2 = 0, n_2xN = 0, n_Nx2 = 0, n_NxN = 0;

        /* Examine all the rooms */
        for (n = 0; n < AUTO_ROOMS; n++) {

            /* Access the n'th room */
            ar = &auto_rooms[n];

            /* Skip "dead" rooms */
            if (ar->free) continue;

            /* Count the "used" rooms */
            used++;

            /* Extract the "size" */
            w = 1 + ar->x2 - ar->x1;
            h = 1 + ar->y2 - ar->y1;

            /* Count the "singles" */
            if ((w == 1) && (h == 1)) n_1x1++;
            else if (w == 1) n_1xN++;
            else if (h == 1) n_Nx1++;
            else if ((w == 2) && (h == 2)) n_2x2++;
            else if (w == 2) n_2xN++;
            else if (h == 2) n_Nx2++;
            else n_NxN++;
        }


        /* Display some info */	
        msg_print(format("Rooms: %d/%d used.", used, auto_room_max));
        msg_print(format("Corridors: 1xN (%d), Nx1 (%d)", n_1xN, n_Nx1));
        msg_print(format("Thickies: 2x2 (%d), 2xN (%d), Nx2 (%d)",
                         n_2x2, n_2xN, n_Nx2));
        msg_print(format("Singles: %d.  Normals: %d.", n_1x1, n_NxN));
        msg_print(NULL);
    }

    /* Command: Grid Info */
    else if (cmd == 'g') {

        int x, y, n;

        auto_grid *ag;
        auto_room *ar;

        int tg = 0;
        int tr = 0;

        int cc[8];

        /* Count the crossing factors */
        cc[0] = cc[1] = cc[2] = cc[3] = cc[4] = cc[5] = cc[6] = cc[7] = 0;

        /* Total explored grids */
        for (y = 0; y < AUTO_MAX_Y; y++) {
            for (x = 0; x < AUTO_MAX_X; x++) {

                /* Get the grid */
                ag = grid(x, y);

                /* Skip unknown grids */
                if (ag->o_c == ' ') continue;

                /* Count them */
                tg++;

                /* Count the rooms this grid is in */
                for (n = 0, ar = room(1,x,y); ar && (n<7); ar = room(0,0,0)) n++;

                /* Hack -- Mention some locations */
                if ((n > 1) && !cc[n]) {
                    msg_print(format("The grid at %d,%d is in %d rooms.", x, y, n));
                }

                /* Count them */
                if (n) tr++;

                /* Count the number of grids in that many rooms */
                cc[n]++;
            }
        }

        /* Display some info */	
        msg_print(format("Grids: %d known, %d in rooms.", tg, tr));
        msg_print(format("Roomies: %d, %d, %d, %d, %d, %d, %d, %d.",
                         cc[0], cc[1], cc[2], cc[3],
                         cc[4], cc[5], cc[6], cc[7]));
        msg_print(NULL);
    }


    /* Mega-Hack -- Activate Borg */
    if (auto_active) {

        /* Mega-Hack -- Turn off wizard mode */
        wizard = FALSE;

        /* Mega-Hack -- Redraw Everything */
        do_cmd_redraw();

        /* Mega-Hack -- Flush the borg key-queue */
        borg_flush();

        /* Mega-Hack -- make sure we are okay */
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);

        /* Start a new level */
        if (auto_depth < 0) {

            /* Mega-Hack -- ask for a feeling */
            borg_keypress('^');
            borg_keypress('F');
        }
    }
}


#endif

