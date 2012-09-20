/* File: save.c */

/* Purpose: save the current game into a savefile */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"


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
 * Some "local" parameters, used to help write savefiles
 */

static FILE	*fff;		/* Current save "file" */

static byte	xor_byte;	/* Simple encryption */

static u32b	v_stamp = 0L;	/* A simple "checksum" on the actual values */
static u32b	x_stamp = 0L;	/* A simple "checksum" on the encoded bytes */



/*
 * These functions place information into a savefile a byte at a time
 */

static void sf_put(byte v)
{
    /* Encode the value, write a character */
    xor_byte ^= v;
    (void)putc((int)xor_byte, fff);

    /* Maintain the checksum info */
    v_stamp += v;
    x_stamp += xor_byte;
}

static void wr_byte(byte v)
{
    sf_put(v);
}

static void wr_char(char v)
{
    wr_byte((byte)v);
}

static void wr_u16b(u16b v)
{
    sf_put(v & 0xFF);
    sf_put((v >> 8) & 0xFF);
}

static void wr_s16b(s16b v)
{
    wr_u16b((u16b)v);
}

static void wr_u32b(u32b v)
{
    sf_put(v & 0xFF);
    sf_put((v >> 8) & 0xFF);
    sf_put((v >> 16) & 0xFF);
    sf_put((v >> 24) & 0xFF);
}

static void wr_s32b(s32b v)
{
    wr_u32b((u32b)v);
}

static void wr_string(cptr str)
{
    while (*str) {
        wr_byte(*str);
        str++;
    }
    wr_byte(*str);
}


/*
 * These functions write info in larger logical records
 */


/*
 * Write an "item" record
 */
static void wr_item(inven_type *i_ptr)
{
    wr_s16b(i_ptr->k_idx);

    wr_byte(i_ptr->iy);
    wr_byte(i_ptr->ix);

    wr_byte(i_ptr->tval);
    wr_byte(i_ptr->sval);
    wr_s16b(i_ptr->pval);

    wr_byte(i_ptr->discount);
    wr_byte(i_ptr->number);
    wr_s16b(i_ptr->weight);

    wr_byte(i_ptr->name1);
    wr_byte(i_ptr->name2);
    wr_s16b(i_ptr->timeout);

    wr_s16b(i_ptr->to_h);
    wr_s16b(i_ptr->to_d);
    wr_s16b(i_ptr->to_a);
    wr_s16b(i_ptr->ac);
    wr_byte(i_ptr->dd);
    wr_byte(i_ptr->ds);

    wr_byte(i_ptr->ident);

    wr_byte(i_ptr->marked);

    wr_u32b(0L);
    wr_u32b(0L);
    wr_u32b(0L);
    
    wr_u16b(0);
    
    wr_byte(i_ptr->xtra1);
    wr_byte(i_ptr->xtra2);

    /* Save the inscription (if any) */
    if (i_ptr->note) {
        wr_string(quark_str(i_ptr->note));
    }
    else {
        wr_string("");
    }
}


/*
 * Write a "monster" record
 */
static void wr_monster(monster_type *m_ptr)
{
    wr_s16b(m_ptr->r_idx);
    wr_byte(m_ptr->fy);
    wr_byte(m_ptr->fx);
    wr_s16b(m_ptr->hp);
    wr_s16b(m_ptr->maxhp);
    wr_s16b(m_ptr->csleep);
    wr_byte(m_ptr->mspeed);
    wr_byte(m_ptr->energy);
    wr_byte(m_ptr->stunned);
    wr_byte(m_ptr->confused);
    wr_byte(m_ptr->monfear);
    wr_byte(0);
}


/*
 * Write a "lore" record
 */
static void wr_lore(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];

    /* Count sights/deaths/kills */
    wr_s16b(r_ptr->r_sights);
    wr_s16b(r_ptr->r_deaths);
    wr_s16b(r_ptr->r_pkills);
    wr_s16b(r_ptr->r_tkills);

    /* Count wakes and ignores */
    wr_byte(r_ptr->r_wake);
    wr_byte(r_ptr->r_ignore);

    /* Extra stuff */
    wr_byte(r_ptr->r_xtra1);
    wr_byte(r_ptr->r_xtra2);

    /* Count drops */
    wr_byte(r_ptr->r_drop_gold);
    wr_byte(r_ptr->r_drop_item);

    /* Count spells */
    wr_byte(r_ptr->r_cast_inate);
    wr_byte(r_ptr->r_cast_spell);

    /* Count blows of each type */
    wr_byte(r_ptr->r_blows[0]);
    wr_byte(r_ptr->r_blows[1]);
    wr_byte(r_ptr->r_blows[2]);
    wr_byte(r_ptr->r_blows[3]);

    /* Memorize flags */
    wr_u32b(r_ptr->r_flags1);
    wr_u32b(r_ptr->r_flags2);
    wr_u32b(r_ptr->r_flags3);
    wr_u32b(r_ptr->r_flags4);
    wr_u32b(r_ptr->r_flags5);
    wr_u32b(r_ptr->r_flags6);


    /* Monster limit per level */
    wr_byte(r_ptr->max_num);

    /* Later (?) */
    wr_byte(0);
    wr_byte(0);
    wr_byte(0);
}


/*
 * Write an "xtra" record
 */
static void wr_xtra(int k_idx)
{
    byte tmp8u = 0;

    inven_kind *k_ptr = &k_info[k_idx];

    if (k_ptr->aware) tmp8u |= 0x01;
    if (k_ptr->tried) tmp8u |= 0x02;

    wr_byte(tmp8u);
}


/*
 * Write a "store" record
 */
static void wr_store(store_type *st_ptr)
{
    int j;

    /* Save the "open" counter */
    wr_u32b(st_ptr->store_open);

    /* Save the "insults" */
    wr_s16b(st_ptr->insult_cur);

    /* Save the current owner */
    wr_byte(st_ptr->owner);

    /* Save the stock size */
    wr_byte(st_ptr->stock_num);

    /* Save the "haggle" info */
    wr_s16b(st_ptr->good_buy);
    wr_s16b(st_ptr->bad_buy);

    /* Save the stock */
    for (j = 0; j < st_ptr->stock_num; j++) {

        /* Save each item in stock */
        wr_item(&st_ptr->stock[j]);
    }
}


/*
 * Write the "options"
 */
static void wr_options(void)
{
    int i;

    u16b c;

    u32b opt[4];


    /*** Normal options ***/

    /* Clear the option flag sets */
    for (i = 0; i < 4; i++) opt[i] = 0L;

    /* Analyze the options */
    for (i = 0; options[i].o_desc; i++) {

        int os = options[i].o_set;
        int ob = options[i].o_bit;

        /* Extract a variable setting, if possible */
        if (options[i].o_var && os) {
            if (*options[i].o_var) opt[os-1] |= (1L << ob);
        }
    }

    /* Read the options */
    for (i = 0; i < 4; i++) wr_u32b(opt[i]);


    /*** Special Options ***/

    /* Write "delay_spd" */
    wr_byte(delay_spd);

    /* Write "hitpoint_warn" */
    wr_byte(hitpoint_warn);


    /*** Cheating options ***/

    c = 0;

    if (wizard) c |= 0x0002;

    if (cheat_peek) c |= 0x0100;
    if (cheat_hear) c |= 0x0200;
    if (cheat_room) c |= 0x0400;
    if (cheat_xtra) c |= 0x0800;
    if (cheat_know) c |= 0x1000;
    if (cheat_live) c |= 0x2000;

    wr_u16b(c);
}


/*
 * Hack -- Write the "ghost" info
 */
static void wr_ghost()
{
    int i;

    monster_race *r_ptr = &r_info[MAX_R_IDX-1];

    cptr name = (r_name + r_ptr->name);


    /* Name */
    wr_string(name);

    /* Visuals */
    wr_char(r_ptr->r_char);
    wr_byte(r_ptr->r_attr);

    /* Level/Rarity */
    wr_byte(r_ptr->level);
    wr_byte(r_ptr->rarity);

    /* Misc info */
    wr_byte(r_ptr->hdice);
    wr_byte(r_ptr->hside);
    wr_s16b(r_ptr->ac);
    wr_s16b(r_ptr->sleep);
    wr_byte(r_ptr->aaf);
    wr_byte(r_ptr->speed);

    /* Experience */
    wr_s32b(r_ptr->mexp);

    /* Extra */
    wr_s16b(r_ptr->extra);

    /* Frequency */
    wr_byte(r_ptr->freq_inate);
    wr_byte(r_ptr->freq_spell);

    /* Flags */
    wr_u32b(r_ptr->flags1);
    wr_u32b(r_ptr->flags2);
    wr_u32b(r_ptr->flags3);
    wr_u32b(r_ptr->flags4);
    wr_u32b(r_ptr->flags5);
    wr_u32b(r_ptr->flags6);

    /* Attacks */
    for (i = 0; i < 4; i++) {
        wr_byte(r_ptr->blow[i].method);
        wr_byte(r_ptr->blow[i].effect);
        wr_byte(r_ptr->blow[i].d_dice);
        wr_byte(r_ptr->blow[i].d_side);
    }
}


/*
 * Write some "extra" info
 */
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
    wr_byte(0);	/* oops */

    wr_byte(p_ptr->hitdie);
    wr_byte(p_ptr->expfact);

    wr_s16b(p_ptr->age);
    wr_s16b(p_ptr->ht);
    wr_s16b(p_ptr->wt);

    /* Dump the stats (maximum and current) */
    for (i = 0; i < 6; ++i) wr_s16b(p_ptr->stat_max[i]);
    for (i = 0; i < 6; ++i) wr_s16b(p_ptr->stat_cur[i]);

    /* Ignore the transient stats */
    for (i = 0; i < 12; ++i) wr_s16b(0);

    wr_u32b(p_ptr->au);

    wr_u32b(p_ptr->max_exp);
    wr_u32b(p_ptr->exp);
    wr_u16b(p_ptr->exp_frac);
    wr_s16b(p_ptr->lev);

    wr_s16b(p_ptr->mhp);
    wr_s16b(p_ptr->chp);
    wr_u16b(p_ptr->chp_frac);

    wr_s16b(p_ptr->msp);
    wr_s16b(p_ptr->csp);
    wr_u16b(p_ptr->csp_frac);

    /* Max Player and Dungeon Levels */
    wr_s16b(p_ptr->max_plv);
    wr_s16b(p_ptr->max_dlv);

    /* More info */
    wr_s16b(0);	/* oops */
    wr_s16b(0);	/* oops */
    wr_s16b(0);	/* oops */
    wr_s16b(0);	/* oops */
    wr_s16b(p_ptr->sc);
    wr_s16b(0);	/* oops */

    wr_s16b(0);		/* old "rest" */
    wr_s16b(p_ptr->blind);
    wr_s16b(p_ptr->paralyzed);
    wr_s16b(p_ptr->confused);
    wr_s16b(p_ptr->food);
    wr_s16b(0);	/* old "food_digested" */
    wr_s16b(0);	/* old "protection" */
    wr_s16b(p_ptr->energy);
    wr_s16b(p_ptr->fast);
    wr_s16b(p_ptr->slow);
    wr_s16b(p_ptr->afraid);
    wr_s16b(p_ptr->cut);
    wr_s16b(p_ptr->stun);
    wr_s16b(p_ptr->poisoned);
    wr_s16b(p_ptr->image);
    wr_s16b(p_ptr->protevil);
    wr_s16b(p_ptr->invuln);
    wr_s16b(p_ptr->hero);
    wr_s16b(p_ptr->shero);
    wr_s16b(p_ptr->shield);
    wr_s16b(p_ptr->blessed);
    wr_s16b(p_ptr->tim_invis);
    wr_s16b(p_ptr->word_recall);
    wr_s16b(p_ptr->see_infra);
    wr_s16b(p_ptr->tim_infra);
    wr_s16b(p_ptr->oppose_fire);
    wr_s16b(p_ptr->oppose_cold);
    wr_s16b(p_ptr->oppose_acid);
    wr_s16b(p_ptr->oppose_elec);
    wr_s16b(p_ptr->oppose_pois);

    wr_byte(p_ptr->confusing);
    wr_byte(0);	/* oops */
    wr_byte(0);	/* oops */
    wr_byte(0);	/* oops */
    wr_byte(p_ptr->searching);
    wr_byte(p_ptr->maximize);
    wr_byte(p_ptr->preserve);
    wr_byte(0);

    /* Future use */
    for (i = 0; i < 12; i++) wr_u32b(0L);

    /* Ignore some flags */
    wr_u32b(0L);	/* oops */
    wr_u32b(0L);	/* oops */
    wr_u32b(0L);	/* oops */


    /* Write the "object seeds" */
    wr_u32b(seed_flavor);
    wr_u32b(seed_town);


    /* Special stuff */
    wr_u16b(panic_save);
    wr_u16b(total_winner);
    wr_u16b(noscore);


    /* Write death */
    wr_byte(death);

    /* Write feeling */
    wr_byte(feeling);

    /* Turn of last "feeling" */
    wr_s32b(old_turn);

    /* Current turn */
    wr_s32b(turn);
}


/*
 * Mega-Hack -- Initialize a fake object from a terrain feature
 *
 * This function picks random traps for invisible traps, losing
 * invisible trap doors, and converts embedded gold into copper
 * worth 50 gold pieces.  Locked and Jammed doors get a decent
 * guess at the lock/jam value.
 */
static bool wr_dungeon_aux(inven_type *i_ptr, int y, int x)
{
    int f;
    
    bool invis = FALSE;
    
    cave_type *c_ptr = &cave[y][x];


    /* Skip weirdness */    
    if ((c_ptr->feat & 0x3F) == 0x00) return (FALSE);

    /* Skip empty floors */
    if ((c_ptr->feat & 0x3F) == 0x01) return (FALSE);
    
    /* Skip empty magma/quartz */
    if ((c_ptr->feat & 0x3F) == 0x32) return (FALSE);
    if ((c_ptr->feat & 0x3F) == 0x33) return (FALSE);

    /* Skip walls */
    if ((c_ptr->feat & 0x3F) >= 0x38) return (FALSE);

    /* Wipe the object */
    WIPE(i_ptr, inven_type);
    
    /* A single object */
    i_ptr->number = 1;

    /* Hack -- worthless */
    i_ptr->ident |= ID_BROKEN;

    /* Save the actual feature */
    f = (c_ptr->feat & 0x3F);

    /* Mega-Hack -- invisible traps */
    if ((c_ptr->feat & 0x3F) == 0x02) {
 
        /* Make invisible */
        invis = TRUE;

        /* XXX XXX Hack -- assume any trap (except a trap door) */
        c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x10) + randint(15);
    }
    
    /* Analyze the terrain feature */
    switch (c_ptr->feat & 0x3F) {

        /* Invisible trap */
        case 0x02:
            /* See above */
            break;
            
        /* Glyph of warding */
        case 0x03:
            i_ptr->k_idx = 459;
            i_ptr->tval = 102;
            i_ptr->sval = 63;
            break;

        /* Open door */
        case 0x04:
            i_ptr->k_idx = 446;
            i_ptr->tval = 104;
            break;

        /* Broken door */
        case 0x05:
            i_ptr->k_idx = 446;
            i_ptr->tval = 104;
            i_ptr->pval = 1;
            break;

        /* Up stairs */
        case 0x06:
            i_ptr->k_idx = 449;
            i_ptr->tval = 107;
            break;

        /* Down stairs */
        case 0x07:
            i_ptr->k_idx = 450;
            i_ptr->tval = 108;
            break;

        /* Shops */
        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
            i_ptr->k_idx = 451 + (c_ptr->feat & 0x07);
            i_ptr->tval = 110;
            i_ptr->sval = 1 + (c_ptr->feat & 0x07);
            break;

        /* Visible trap -- trap door */
        case 0x10:
            i_ptr->k_idx = 462;
            i_ptr->tval = 102;
            i_ptr->sval = 2;
            i_ptr->pval = 20;
            break;

        /* Visible trap -- pit */
        case 0x11:
            i_ptr->k_idx = 460;
            i_ptr->tval = 102;
            i_ptr->sval = 0;
            i_ptr->pval = 1;
            break;

        /* Visible trap -- spiked pit */
        case 0x12:
            i_ptr->k_idx = 461;
            i_ptr->tval = 102;
            i_ptr->sval = 1;
            i_ptr->pval = 10;
            break;

        /* Visible trap -- poison pit -> spiked pit */
        case 0x13:
            i_ptr->k_idx = 461;
            i_ptr->tval = 102;
            i_ptr->sval = 1;
            i_ptr->pval = 10;
            break;

        /* Visible trap -- rune -- summon */
        case 0x14:
            i_ptr->k_idx = 469;
            i_ptr->tval = 102;
            i_ptr->sval = 15;
            i_ptr->pval = 5;
            break;

        /* Visible trap -- rune -- teleport */
        case 0x15:
            i_ptr->k_idx = 466;
            i_ptr->tval = 102;
            i_ptr->sval = 14;
            i_ptr->pval = 5;
            break;

        /* Visible trap -- spot -- fire */
        case 0x16:
            i_ptr->k_idx = 470;
            i_ptr->tval = 102;
            i_ptr->sval = 12;
            i_ptr->pval = 10;
            break;

        /* Visible trap -- spot -- acid */
        case 0x17:
            i_ptr->k_idx = 471;
            i_ptr->tval = 102;
            i_ptr->sval = 13;
            i_ptr->pval = 10;
            break;

        /* Visible trap -- dart -- slow */
        case 0x18:
            i_ptr->k_idx = 475;
            i_ptr->tval = 102;
            i_ptr->sval = 4;
            i_ptr->pval = 5;
            break;

        /* Visible trap -- dart -- str */
        case 0x19:
            i_ptr->k_idx = 465;
            i_ptr->tval = 102;
            i_ptr->sval = 6;
            i_ptr->pval = 5;
            break;

        /* Visible trap -- dart -- dex */
        case 0x1A:
            i_ptr->k_idx = 468;
            i_ptr->tval = 102;
            i_ptr->sval = 5;
            i_ptr->pval = 5;
            break;

        /* Visible trap -- dart -- con */
        case 0x1B:
            i_ptr->k_idx = 476;
            i_ptr->tval = 102;
            i_ptr->sval = 7;
            i_ptr->pval = 5;
            break;

        /* Visible trap -- gas -- blind */
        case 0x1C:
            i_ptr->k_idx = 473;
            i_ptr->tval = 102;
            i_ptr->sval = 9;
            i_ptr->pval = 10;
            break;

        /* Visible trap -- gas -- confuse */
        case 0x1D:
            i_ptr->k_idx = 474;
            i_ptr->tval = 102;
            i_ptr->sval = 10;
            i_ptr->pval = 10;
            break;

        /* Visible trap -- gas -- poison */
        case 0x1E:
            i_ptr->k_idx = 472;
            i_ptr->tval = 102;
            i_ptr->sval = 8;
            i_ptr->pval = 10;
            break;

        /* Visible trap -- gas -- sleep */
        case 0x1F:
            i_ptr->k_idx = 463;
            i_ptr->tval = 102;
            i_ptr->sval = 11;
            i_ptr->pval = 10;
            break;

        /* Doors -- locked */
        case 0x20:
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x24:
        case 0x25:
        case 0x26:
        case 0x27:
            i_ptr->k_idx = 447;
            i_ptr->tval = 118;
            i_ptr->pval = ((int)(c_ptr->feat & 0x07)) * 2;
            break;

        /* Doors -- jammed */
        case 0x28:
        case 0x29:
        case 0x2A:
        case 0x2B:
        case 0x2C:
        case 0x2D:
        case 0x2E:
        case 0x2F:
            i_ptr->k_idx = 447;
            i_ptr->tval = 118;
            i_ptr->pval = -1 - ((int)(c_ptr->feat & 0x07)) * 2;
            break;

        /* Secret doors */
        case 0x30:
            i_ptr->k_idx = 448;
            i_ptr->tval = 117;
            break;
        
        /* Rubble */
        case 0x31:
            i_ptr->k_idx = 445;
            i_ptr->tval = 119;
            break;
        
        /* Hack -- Vein + treasure */
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
            i_ptr->k_idx = 480;
            i_ptr->tval = 100;
            i_ptr->sval = 1;
            i_ptr->pval = 50;
            break;
    }

    /* Hack -- invisible traps */
    if (invis) i_ptr->tval = 101;

    /* Restore the feature */
    c_ptr->feat = ((c_ptr->feat & ~0x3F) | f);
    
    /* Paranoia -- Nothing made */
    if (!i_ptr->k_idx) return (FALSE);
        
    /* Set the position */
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Success */
    return (TRUE);
}


/*
 * Write the current dungeon
 *
 * XXX XXX XXX Mega-Hack -- convert new "terrain feature" info back
 * into standard Angband 2.7.9 savefile format using "fake" objects,
 * so that I can use the new "terrain features" even though the new
 * savefile format is not ready yet.
 */
static void wr_dungeon()
{
    int i, j, y, x;
    byte count, prev_char;
    byte tmp8u;

    inven_type forge;

    cave_type *c_ptr;

    inven_type *i_ptr;


    /* Dungeon specific info follows */
    wr_u16b(dun_level);
    wr_u16b(num_repro);
    wr_u16b(py);
    wr_u16b(px);
    wr_u16b(cur_hgt);
    wr_u16b(cur_wid);
    wr_u16b(max_panel_rows);
    wr_u16b(max_panel_cols);


    /*** Simple "Run-Length-Encoding" of cave ***/

    /* Note that this will induce two wasted bytes */
    count = 0;
    prev_char = 0;

    /* Dump the cave */
    for (y = 0; y < cur_hgt; y++) {
        for (x = 0; x < cur_wid; x++) {

            /* Get the cave */
            c_ptr = &cave[y][x];

            /* Paranoia */
            if (c_ptr->i_idx) {
                inven_type *i_ptr = &i_list[c_ptr->i_idx];
                i_ptr->iy = y;
                i_ptr->ix = x;
            }

            /* Paranoia */
            if (c_ptr->m_idx) {
                monster_type *m_ptr = &m_list[c_ptr->m_idx];
                m_ptr->fy = y;
                m_ptr->fx = x;
            }

            /* Start with nothing */
            tmp8u = 0;
            
            /* The old "vault" flag */
            if (c_ptr->feat & CAVE_ICKY) tmp8u |= OLD_GRID_ICKY;

            /* The old "room" flag */
            if (c_ptr->feat & CAVE_ROOM) tmp8u |= OLD_GRID_ROOM;

            /* The old "glow" flag */
            if (c_ptr->feat & CAVE_GLOW) tmp8u |= OLD_GRID_GLOW;

            /* The old "mark" flag */
            if (c_ptr->feat & CAVE_MARK) tmp8u |= OLD_GRID_MARK;

            /* Convert the terrain type */
            switch (c_ptr->feat & 0x3F) {

                case 0x06:
                case 0x07:
                    tmp8u |= OLD_GRID_PERM;
                    break;

                case 0x08:
                case 0x09:
                case 0x0A:
                case 0x0B:
                case 0x0C:
                case 0x0D:
                case 0x0E:
                case 0x0F:
                    tmp8u |= OLD_GRID_PERM;
                    break;

                case 0x32:
                case 0x34:
                case 0x36:
                    tmp8u |= OLD_GRID_WALL_MAGMA;
                    break;

                case 0x33:
                case 0x35:
                case 0x37:
                    tmp8u |= OLD_GRID_WALL_QUARTZ;
                    break;

                case 0x38:
                case 0x39:
                case 0x3A:
                case 0x3B:
                    tmp8u |= OLD_GRID_WALL_GRANITE;
                    break;

                case 0x3C:
                case 0x3D:
                case 0x3E:
                case 0x3F:
                    tmp8u |= OLD_GRID_WALL_GRANITE;
                    tmp8u |= OLD_GRID_PERM;
                    break;
            }

            /* If the run is broken, or too full, flush it */
            if ((tmp8u != prev_char) || (count == MAX_UCHAR)) {
                wr_byte((byte)count);
                wr_byte((byte)prev_char);
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
        wr_byte((byte)count);
        wr_byte((byte)prev_char);
    }


    /* Mega-Hack -- reorder the objects */
    reorder_objects();


    /* Nothing yet */
    j = 0;

    /* Point at the object */
    i_ptr = &forge;
    
    /* Fake objects */
    for (y = 0; y < cur_hgt; y++) {
        for (x = 0; x < cur_wid; x++) {

            /* Count "fake objects" */
            if (wr_dungeon_aux(i_ptr, y, x)) j++;
        }
    }

    /* Total objects */
    wr_u16b(i_max + j);

    /* Dump the "real" items */
    for (i = 1; i < i_max; i++) {
        i_ptr = &i_list[i];
        wr_item(i_ptr);
    }

    /* Point at the object */
    i_ptr = &forge;
    
    /* Fake objects */
    for (y = 0; y < cur_hgt; y++) {
        for (x = 0; x < cur_wid; x++) {

            /* Dump the "fake objects" */
            if (wr_dungeon_aux(i_ptr, y, x)) wr_item(i_ptr);
        }
    }


    /* Mega-Hack -- reorder the monsters */
    reorder_monsters();
    
    /* Dump the "real" monsters */
    wr_u16b(m_max);
    for (i = 1; i < m_max; i++) {
        monster_type *m_ptr = &m_list[i];
        wr_monster(m_ptr);
    }
}



/*
 * Actually write a save-file
 */
static int wr_savefile()
{
    int        i;

    u32b              now;

    byte		tmp8u;
    u16b		tmp16u;


    /* Guess at the current time */
    now = time((time_t *)0);


    /* Note the operating system */
    sf_xtra = 0L;

    /* Note when the file was saved */
    sf_when = now;

    /* Note the number of saves */
    sf_saves++;


    /*** Actually write the file ***/

    /* Dump the file header */
    xor_byte = 0;
    wr_byte(VERSION_MAJOR);
    xor_byte = 0;
    wr_byte(VERSION_MINOR);
    xor_byte = 0;
    wr_byte(VERSION_PATCH);
    xor_byte = 0;
    tmp8u = rand_int(256);
    wr_byte(tmp8u);


    /* Reset the checksum */
    v_stamp = 0L;
    x_stamp = 0L;


    /* Operating system */
    wr_u32b(sf_xtra);


    /* Time file last saved */
    wr_u32b(sf_when);

    /* Number of past lives */
    wr_u16b(sf_lives);

    /* Number of times saved */
    wr_u16b(sf_saves);


    /* No extra bytes for this operating system */
    wr_u32b(0L);

    /* No extra strings for this operating system */
    wr_u32b(0L);


    /* Write the boolean "options" */
    wr_options();


    /* Dump the number of "messages" */
    tmp16u = message_num();
    if (compress_savefile && (tmp16u > 40)) tmp16u = 40;
    wr_u16b(tmp16u);

    /* Dump the messages (oldest first!) */
    for (i = tmp16u - 1; i >= 0; i--) {
        wr_string(message_str(i));
    }


    /* Dump the monster lore */
    tmp16u = MAX_R_IDX;
    wr_u16b(tmp16u);
    for (i = 0; i < tmp16u; i++) wr_lore(i);


    /* Dump the object memory */
    tmp16u = MAX_K_IDX;
    wr_u16b(tmp16u);
    for (i = 0; i < tmp16u; i++) wr_xtra(i);


    /* Hack -- Dump the quests */
    tmp16u = MAX_Q_IDX;
    wr_u16b(tmp16u);
    for (i = 0; i < tmp16u; i++) {
        wr_byte(q_list[i].level);
        wr_byte(0);
        wr_byte(0);
        wr_byte(0);
    }

    /* Hack -- Dump the artifacts */
    tmp16u = MAX_A_IDX;
    wr_u16b(tmp16u);
    for (i = 0; i < tmp16u; i++) {
        artifact_type *a_ptr = &a_info[i];
        wr_byte(a_ptr->cur_num);
        wr_byte(0);
        wr_byte(0);
        wr_byte(0);
    }



    /* Write the "extra" information */
    wr_extra();


    /* Dump the "player hp" entries */
    tmp16u = PY_MAX_LEVEL;
    wr_u16b(tmp16u);
    for (i = 0; i < tmp16u; i++) {
        wr_s16b(player_hp[i]);
    }


    /* Write spell data */
    wr_u32b(spell_learned1);
    wr_u32b(spell_learned2);
    wr_u32b(spell_worked1);
    wr_u32b(spell_worked2);
    wr_u32b(spell_forgotten1);
    wr_u32b(spell_forgotten2);

    /* Dump the ordered spells */
    for (i = 0; i < 64; i++) {
        wr_byte(spell_order[i]);
    }


    /* Write the inventory */
    for (i = 0; i < INVEN_TOTAL; i++) {
        if (inventory[i].k_idx) {
            wr_u16b(i);
            wr_item(&inventory[i]);
        }
    }

    /* Add a sentinel */
    wr_u16b(0xFFFF);


    /* Note the stores */
    tmp16u = MAX_STORES;
    wr_u16b(tmp16u);

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
    wr_u32b(v_stamp);

    /* Write the "encoded checksum" */
    wr_u32b(x_stamp);


    /* Error in save */
    if (ferror(fff) || (fflush(fff) == EOF)) return FALSE;

    /* Successful save */
    return TRUE;
}


/*
 * Medium level player saver
 *
 * XXX XXX XXX Angband 2.8.0 will use "fd" instead of "fff" if possible
 */
static bool save_player_aux(char *name)
{
    bool	ok = FALSE;

    int		fd = -1;

    int		mode = 0644;


    /* No file yet */
    fff = NULL;


#if defined(MACINTOSH) && !defined(applec)
    /* Global -- "save file" */
    _ftype = 'SAVE';
#endif


    /* Open (or create) the binary savefile (with truncation) */
    fd = fd_open(name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, mode);

    /* File is okay */
    if (fd >= 0) {

        /* Close the "fd" */
        (void)fd_close(fd);

        /* Open the savefile */
        fff = my_fopen(name, "wb");

        /* Successful open */
        if (fff) {

            /* Write the savefile */
            if (wr_savefile()) ok = TRUE;

            /* Attempt to close it */
            if (my_fclose(fff)) ok = FALSE;
        }

        /* Remove "broken" files */
        if (!ok) (void)remove(name);
    }


    /* Failure */
    if (!ok) return (FALSE);

    /* Successful save */
    character_saved = TRUE;

    /* Success */
    return (TRUE);
}



/*
 * Attempt to save the player in a savefile
 */
bool save_player()
{
    int		result = FALSE;

    char	safe[1024];


#ifdef SET_UID

# ifdef SECURE

    /* Get "games" permissions */
    beGames();

# endif

#endif


    /* New savefile */
    strcpy(safe, savefile);
    strcat(safe, ".new");

#ifdef VM
    /* Hack -- support "flat directory" usage on VM/ESA */
    strcpy(safe, savefile);
    strcat(safe, "n");
#endif /* VM */

    /* Remove it */
    remove(safe);

    /* Attempt to save the player */
    if (save_player_aux(safe)) {

        char temp[1024];

        /* Old savefile */
        strcpy(temp, savefile);
        strcat(temp, ".old");

#ifdef VM
        /* Hack -- support "flat directory" usage on VM/ESA */
        strcpy(temp, savefile);
        strcat(temp, "o");
#endif /* VM */

        /* Remove it */
        remove(temp);

        /* Preserve old savefile */
        rename(savefile, temp);

        /* Activate new savefile */
        rename(safe, savefile);

        /* Remove preserved savefile */
        remove(temp);

        /* Hack -- Pretend the character was loaded */
        character_loaded = TRUE;

#ifdef VERIFY_SAVEFILE

        /* Lock on savefile */
        strcpy(temp, savefile);
        strcat(temp, ".lok");

        /* Remove lock file */
        remove(temp);

#endif

        /* Success */
        result = TRUE;
    }


#ifdef SET_UID

# ifdef SECURE

    /* Drop "games" permissions */
    bePlayer();

# endif

#endif


    /* Return the result */
    return (result);
}



#if 0


/* XXX XXX XXX Ignore this for now... */

/*
 * The basic format of Angband 2.8.0 (and later) savefiles is simple.
 *
 * The savefile itself is a "header" (4 bytes) plus a series of "blocks".
 *
 * The "header" contains information about the "version" of the savefile.
 * Conveniently, pre-2.8.0 savefiles also use a 4 byte header, though the
 * interpretation of the "sf_extra" byte is very different.  Unfortunately,
 * savefiles from Angband 2.5.X reverse the sf_major and sf_minor fields,
 * and must be handled specially, until we decide to start ignoring them.
 *
 * Each "block" is a "type" (2 bytes), plus a "size", plus a "data", plus a "zero", plus a "zero".
 *
 */


/*
 * Hack -- current block type
 */
static u16b data_type;

/*
 * Hack -- current block size
 */
static u16b data_size;

/*
 * Hack -- pointer to the data buffer
 */
static byte *data_head;

/*
 * Hack -- pointer into the data buffer
 */
static byte *data_next;



/*
 * Hack -- actually place data into the savefile
 */
static void wr_data(u16b n, byte *v)
{
    int i;

    /* XXX XXX XXX Hack -- Write */    
    for (i = 0; i < n; i++) {

        /* Write */
        (void)putc(v[i], fff);
    }

    /* Success */
    return (0);
}


/*
 * Hack -- write a "header" to the savefile
 */
static errr wr_head(void)
{
    byte fake[4];

    /* Dump the version */
    fake[0] = (byte)(VERSION_MAJOR);
    fake[1] = (byte)(VERSION_MINOR);
    fake[2] = (byte)(VERSION_PATCH);
    fake[3] = (byte)(VERSION_EXTRA);

    /* Dump the data */
    wr_data(4, &fake);

    /* Hack -- reset */
    data_next = data_head;

    /* Success */
    return (0);
}


/*
 * Hack -- write the current "block" to the savefile
 */
static errr wr_block(void)
{
    byte fake[4];

    /* Save the type and size */
    fake[0] = (byte)(data_type);
    fake[1] = (byte)(data_type >> 8);
    fake[2] = (byte)(data_size);
    fake[3] = (byte)(data_size >> 8);

    /* Dump the data */
    wr_data(4, &fake);

    /* Dump the actual data */
    wr_data(data_size, data_head);
    
    /* XXX XXX XXX */
    fake[0] = 0;
    fake[1] = 0;
    fake[2] = 0;
    fake[3] = 0;

    /* Dump the data */
    wr_data(4, &fake);

    /* Hack -- reset */
    data_next = data_head;

    /* Success */
    return (0);
}



/*
 * Hack -- add data to the current block
 */
static void put_byte(byte v)
{
    *data_next++ = v;
}

/*
 * Hack -- add data to the current block
 */
static void put_char(char v)
{
    put_byte((byte)(v));
}

/*
 * Hack -- add data to the current block
 */
static void put_u16b(u16b v)
{
    *data_next++ = (byte)(v);
    *data_next++ = (byte)(v >> 8);
}

/*
 * Hack -- add data to the current block
 */
static void put_s16b(s16b v)
{
    put_u16b((u16b)(v));
}

/*
 * Hack -- add data to the current block
 */
static void put_u32b(u32b v)
{
    *data_next++ = (byte)(v);
    *data_next++ = (byte)(v >> 8);
    *data_next++ = (byte)(v >> 16);
    *data_next++ = (byte)(v >> 24);
}

/*
 * Hack -- add data to the current block
 */
static void put_s32b(s32b v)
{
    put_u32b((u32b)(v));
}

/*
 * Hack -- add data to the current block
 */
static void put_string(char *str)
{
    while ((*data_next++ = *str++) != '\0') ;
}



/*
 * Hack -- put the "options" (flags and masks)
 *
 * Alternative method -- dump actual option "names" and settings
 */
static void put_options()
{
    int i;

    u16b c;

    u32b flag[8];
    u32b mask[8];


    /*** Normal options ***/

    /* Clear the option flags */
    for (i = 0; i < 8; i++) flag[i] = 0L;

    /* Clear the option masks */
    for (i = 0; i < 8; i++) mask[i] = 0L;

    /* Analyze the options */
    for (i = 0; options[i].o_desc; i++) {

        int os = options[i].o_set;
        int ob = options[i].o_bit;

        /* Extract a variable setting, if possible */
        if (options[i].o_var && os) {

            /* Mark the mask */
            mask[os-1] |= (1L << ob);
            
            /* Mark the flag if needed */
            if (*options[i].o_var) flag[os-1] |= (1L << ob);
        }
    }

    /* Put the flags */
    for (i = 0; i < 8; i++) put_u32b(flag[i]);

    /* Put the masks */
    for (i = 0; i < 8; i++) put_u32b(mask[i]);


    /*** Special Options ***/

    /* Write "delay_spd" */
    wr_byte(delay_spd);

    /* Write "hitpoint_warn" */
    wr_byte(hitpoint_warn);


    /*** Cheating options ***/

    c = 0;

    if (wizard) c |= 0x0002;

    if (cheat_peek) c |= 0x0100;
    if (cheat_hear) c |= 0x0200;
    if (cheat_room) c |= 0x0400;
    if (cheat_xtra) c |= 0x0800;
    if (cheat_know) c |= 0x1000;
    if (cheat_live) c |= 0x2000;

    wr_u16b(c);
}



/*
 * Hack -- actually read data from the savefile
 */
static void rd_data(u16b n, byte *v)
{
    int i;

    /* XXX XXX XXX Hack -- Read */    
    for (i = 0; i < n; i++) {

        /* Read */
        v[i] = getc(fff);
    }

    /* Success */
    return (0);
}


/*
 * Hack -- read a "header" from the savefile
 */
static errr rd_head(void)
{
    byte fake[4];

    /* Dump the data */
    rd_data(4, &fake);

    /* Dump the version */
    sf_major = fake[0];
    sf_minor = fake[1];
    sf_patch = fake[2];
    sf_extra = fake[3];

    /* XXX XXX XXX Check 2.4.X savefiles (?) */
    /* Hack -- fix mistake in Angband 2.5.X */
    if ((sf_major == 5) && (sf_minor == 2)) {
        sf_major = 2; sf_minor = 5;
    }

    /* Hack -- reset */
    data_next = data_head;

    /* Success */
    return (0);
}


/*
 * Hack -- read the next "block" from the savefile
 */
static errr rd_block(void)
{
    byte fake[4];

    /* Read the data */
    rd_data(4, &fake);

    /* Extract the type and size */
    data_type = (fake[0] | ((u16b)fake[1] << 8));
    data_size = (fake[2] | ((u16b)fake[3] << 8));

    /* Read the actual data */
    rd_data(data_size, data_head);
    
    /* Read the data */
    rd_data(4, &fake);

    /* XXX XXX XXX */
    fake[0] = 0;
    fake[1] = 0;
    fake[2] = 0;
    fake[3] = 0;

    /* Hack -- reset */
    data_next = data_head;

    /* Success */
    return (0);
}





/*
 * Hack -- get data from the current block
 */
static void get_byte(byte *ip)
{
    byte d1 = (*data_next++);
    (*ip) = (d1);
}

/*
 * Hack -- get data from the current block
 */
static void get_char(char *ip)
{
    get_byte((byte*)ip);
}

/*
 * Hack -- get data from the current block
 */
static void get_u16b(u16b *ip)
{
    byte d0 = (*data_next++);
    byte d1 = (*data_next++);
    (*ip) = ((u16b)d0 | ((u16b)d1 << 8));
}

/*
 * Hack -- get data from the current block
 */
static void get_s16b(s16b *ip)
{
    get_u16b((u16b*)ip);
}

/*
 * Hack -- get data from the current block
 */
static void get_u32b(u32b *ip)
{
    byte d0 = (*data_next++);
    byte d1 = (*data_next++);
    byte d2 = (*data_next++);
    byte d3 = (*data_next++);
    (*ip) = ((u32b)d0 | ((u32b)d1 << 8) | ((u32b)d2 << 16) | ((u32b)d3 << 24));
}

/*
 * Hack -- get data from the current block
 */
static void get_s32b(s32b *ip)
{
    get_u32b((u32b*)ip);
}

/*
 * Hack -- get data from the current block
 */
static void get_string(char *str)
{
    while ((*str++ = *data_next++) != '\0') ;
}



/*
 * Write a savefile for Angband 2.8.0
 */
static errr wr_savefile()
{
    int		i;

    u32b	now;

    byte	tmp8u;
    u16b	tmp16u;


    /*** Hack -- extract some data ***/

    /* Hack -- Acquire the current time */
    now = time((time_t*)(NULL));

    /* Note the operating system */
    sf_xtra = 0L;

    /* Note when the file was saved */
    sf_when = now;

    /* Note the number of saves */
    sf_saves++;


    /*** Actually write the file ***/

    /* Dump the file header */
    wr_head();


#if 0
    /* Operating system */
    wr_u32b(sf_xtra);

    /* Time file last saved */
    wr_u32b(sf_when);

    /* Number of past lives */
    wr_u16b(sf_lives);

    /* Number of times saved */
    wr_u16b(sf_saves);
#endif


    /* Dump the "options" */
    put_options();
    
    /* Set the type */
    data_type = TYPE_OPTIONS;

    /* Set the "options" size */
    data_size = (data_next - data_head);

    /* Write the block */
    wr_block();

    [...]

    /* Read the block */
    rd_block();

    /* Analyze the type */
    switch (data_type) {

        /* Grab the options */
        case TYPE_OPTIONS: get_options(); break;
    }

    /* Paranoia -- verify "data_next" */
    if (data_next - data_head > data_size) return (1);


    /* Dump the "messages" */
    
    /* Dump the number of "messages" */
    tmp16u = message_num();
    if (compress_savefile && (tmp16u > 40)) tmp16u = 40;
    wr_u16b(tmp16u);

    /* Dump the messages (oldest first!) */
    for (i = tmp16u - 1; i >= 0; i--) {
        wr_string(message_str(i));
    }


    /* Dump the monster lore */
    tmp16u = MAX_R_IDX;
    wr_u16b(tmp16u);
    for (i = 0; i < tmp16u; i++) wr_lore(i);


    /* Dump the object memory */
    tmp16u = MAX_K_IDX;
    wr_u16b(tmp16u);
    for (i = 0; i < tmp16u; i++) wr_xtra(i);


    /* Hack -- Dump the quests */
    tmp16u = MAX_Q_IDX;
    wr_u16b(tmp16u);
    for (i = 0; i < tmp16u; i++) {
        wr_byte(q_list[i].level);
        wr_byte(0);
        wr_byte(0);
        wr_byte(0);
    }

    /* Hack -- Dump the artifacts */
    tmp16u = MAX_A_IDX;
    wr_u16b(tmp16u);
    for (i = 0; i < tmp16u; i++) {
        artifact_type *a_ptr = &a_info[i];
        wr_byte(a_ptr->cur_num);
        wr_byte(0);
        wr_byte(0);
        wr_byte(0);
    }



    /* Write the "extra" information */
    wr_extra();


    /* Dump the "player hp" entries */
    tmp16u = PY_MAX_LEVEL;
    wr_u16b(tmp16u);
    for (i = 0; i < tmp16u; i++) {
        wr_s16b(player_hp[i]);
    }


    /* Write spell data */
    wr_u32b(spell_learned1);
    wr_u32b(spell_learned2);
    wr_u32b(spell_worked1);
    wr_u32b(spell_worked2);
    wr_u32b(spell_forgotten1);
    wr_u32b(spell_forgotten2);

    /* Dump the ordered spells */
    for (i = 0; i < 64; i++) {
        wr_byte(spell_order[i]);
    }


    /* Write the inventory */
    for (i = 0; i < INVEN_TOTAL; i++) {
        if (inventory[i].k_idx) {
            wr_u16b(i);
            wr_item(&inventory[i]);
        }
    }

    /* Add a sentinel */
    wr_u16b(0xFFFF);


    /* Note the stores */
    tmp16u = MAX_STORES;
    wr_u16b(tmp16u);

    /* Dump the stores */
    for (i = 0; i < tmp16u; i++) wr_store(&store[i]);


    /* Player is not dead, write the dungeon */
    if (!death) {

        /* Dump the dungeon */
        wr_dungeon();

        /* Dump the ghost */
        wr_ghost();
    }


    /* Dump the "final" marker */
    /* Type zero, Size zero, Contents zero, etc */


    /* Error in save */
    if (ferror(fff) || (fflush(fff) == EOF)) return (1);


    /* Successful save */
    return (0);
}

#endif
