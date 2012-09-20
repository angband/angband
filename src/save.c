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

#ifdef __MINT__
# include <stat.h>		/* chmod() */
#endif

#if !defined(SET_UID) && !defined(ALLOW_FIDDLING)
# if defined(__EMX__) || defined(__GO32__) || defined(_Windows)
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
 * these are used for the save file, to avoid having to pass them to every
 * procedure 
 */

static FILE	*fff;		/* Current save "file" */

static byte	xor_byte;	/* Simple encryption */

static byte	version_maj;	/* Major version */
static byte	version_min;	/* Minor version */
static byte	patch_level;	/* Patch level */

static u32b	v_check = 0L;	/* A simple "checksum" on the actual values */
static u32b	x_check = 0L;	/* A simple "checksum" on the encoded bytes */

static int	from_savefile;	/* can overwrite old savefile when save */

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
    register byte c, v;

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

static void wr_u16b(u16b v)
{
    sf_put(v & 0xFF);
    sf_put((v >> 8) & 0xFF);
}

static void rd_u16b(u16b *ip)
{
    (*ip) = sf_get();
    (*ip) |= ((u16b)(sf_get()) << 8);
}


static void wr_s16b(s16b v)
{
    wr_u16b((u16b)v);
}

static void rd_s16b(s16b *ip)
{
    rd_u16b((u16b*)ip);
}



/*
 * Write/Read various "long" objects
 */

static void wr_u32b(u32b v)
{
    sf_put(v & 0xFF);
    sf_put((v >> 8) & 0xFF);
    sf_put((v >> 16) & 0xFF);
    sf_put((v >> 24) & 0xFF);
}

static void rd_u32b(u32b *ip)
{
    (*ip) = sf_get();
    (*ip) |= ((u32b)(sf_get()) << 8);
    (*ip) |= ((u32b)(sf_get()) << 16);
    (*ip) |= ((u32b)(sf_get()) << 24);
}


static void wr_s32b(s32b v)
{
    wr_u32b((u32b)v);
}

static void rd_s32b(s32b *ip)
{
    rd_u32b((u32b*)ip);
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
	byte tmp;
	rd_byte(&tmp);
	*str = tmp;
	if (!*str) break;
	str++;
    }
}




/*
 * Read an item (2.7.0 or later)
 */
static void rd_item(inven_type *i_ptr)
{
    byte tmp8u;
    u32b tmp32u;
    char note[128];

    /* Get the kind */
    rd_u16b(&i_ptr->k_idx);

    rd_byte(&i_ptr->iy);
    rd_byte(&i_ptr->ix);

    rd_byte(&i_ptr->tval);
    rd_byte(&i_ptr->sval);
    rd_s16b(&i_ptr->pval);

    rd_byte(&i_ptr->name1);
    rd_byte(&i_ptr->name2);
    rd_byte(&i_ptr->ident);
    rd_byte(&i_ptr->number);
    rd_s16b(&i_ptr->weight);
    rd_s16b(&i_ptr->timeout);

    rd_s16b(&i_ptr->tohit);
    rd_s16b(&i_ptr->todam);
    rd_s16b(&i_ptr->toac);
    rd_s16b(&i_ptr->ac);
    rd_byte(&i_ptr->dd);
    rd_byte(&i_ptr->ds);
    rd_byte(&tmp8u);
    rd_byte(&tmp8u);

    rd_s32b(&i_ptr->cost);
    rd_s32b(&i_ptr->scost);

    rd_u32b(&i_ptr->flags1);
    rd_u32b(&i_ptr->flags2);
    rd_u32b(&i_ptr->flags3);
    rd_u32b(&tmp32u);

    rd_string(note);
    inscribe(i_ptr, note);


    /* Hack -- Obtain tval/sval from k_list */
    i_ptr->tval = k_list[i_ptr->k_idx].tval;
    i_ptr->sval = k_list[i_ptr->k_idx].sval;
    

    /* Mega-Hack -- the "speed code" changed in 2.7.3 */
    if (older_than(2,7,3) && (i_ptr->flags1 & TR1_SPEED)) {

	/* Paranoia -- do not allow crazy speeds */
	if (i_ptr->pval <= 2) i_ptr->pval = i_ptr->pval * 10;
    }


    /* Hack -- repair problems with 2.7.0 and 2.7.1 */
    if (older_than(2,7,2)) {

	/* Repair books and chests and such */
	if (!wearable_p(i_ptr)) i_ptr->flags3 = 0L;

	/* Hack -- Grond may have been created by accident */
	if (i_ptr->name1 == ART_GROND) {
	    invcopy(i_ptr, OBJ_LEAD_FILLED_MACE);
	    v_list[i_ptr->name1].cur_num = 0;
	}

	/* Hack -- Morgoth may have been created by accident */
	if (i_ptr->name1 == ART_MORGOTH) {
	    invcopy(i_ptr, OBJ_IRON_CROWN);
	    v_list[i_ptr->name1].cur_num = 0;
	}

	/* Repair dragon scale mail */
	if (i_ptr->tval == TV_DRAG_ARMOR) {

	    /* Repair -- Acquire new flags/fields */
	    i_ptr->flags1 = k_list[i_ptr->k_idx].flags1;
	    i_ptr->flags2 = k_list[i_ptr->k_idx].flags2;
	    i_ptr->flags3 = k_list[i_ptr->k_idx].flags3;
	    i_ptr->pval = k_list[i_ptr->k_idx].pval;
	    i_ptr->ac = k_list[i_ptr->k_idx].ac;
	    i_ptr->dd = k_list[i_ptr->k_idx].dd;
	    i_ptr->ds = k_list[i_ptr->k_idx].ds;
	    i_ptr->weight = k_list[i_ptr->k_idx].weight;
	}

	/* Repair artifacts */
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
    }

    
    /* Hack -- fix problems with 2.7.2 and 2.7.3 */
    else if (older_than(2,7,4)) {

	/* Repair artifacts */
	if (i_ptr->name1) {

	    /* Hack -- make sure we have the correct values */
	    i_ptr->pval = v_list[i_ptr->name1].pval;
	    i_ptr->ac = v_list[i_ptr->name1].ac;
	    i_ptr->dd = v_list[i_ptr->name1].dd;
	    i_ptr->ds = v_list[i_ptr->name1].ds;
	    i_ptr->weight = v_list[i_ptr->name1].weight;
	    i_ptr->cost = v_list[i_ptr->name1].cost;
	}
    }
}


static void wr_item(inven_type *i_ptr)
{
    wr_u16b(i_ptr->k_idx);

    wr_byte(i_ptr->iy);
    wr_byte(i_ptr->ix);

    wr_byte(i_ptr->tval);
    wr_byte(i_ptr->sval);
    wr_s16b(i_ptr->pval);

    wr_byte(i_ptr->name1);
    wr_byte(i_ptr->name2);
    wr_byte(i_ptr->ident);
    wr_byte(i_ptr->number);
    wr_s16b(i_ptr->weight);
    wr_s16b(i_ptr->timeout);

    wr_s16b(i_ptr->tohit);
    wr_s16b(i_ptr->todam);
    wr_s16b(i_ptr->toac);
    wr_s16b(i_ptr->ac);
    wr_byte(i_ptr->dd);
    wr_byte(i_ptr->ds);
    wr_byte(0);
    wr_byte(0);

    wr_s32b(i_ptr->cost);
    wr_s32b(i_ptr->scost);

    wr_u32b(i_ptr->flags1);
    wr_u32b(i_ptr->flags2);
    wr_u32b(i_ptr->flags3);
    wr_u32b(0L);

    wr_string(i_ptr->inscrip);
}



/*
 * Read and Write monsters
 */


static void rd_monster(monster_type *m_ptr)
{
    rd_u16b(&m_ptr->r_idx);
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
    rd_byte(&m_ptr->unused);
    
    /* Hack -- fix speed in old versions */
    if (older_than(2,7,3)) {
	m_ptr->mspeed = r_list[m_ptr->r_idx].speed;
	m_ptr->energy = rand_int(100);
    }
}

static void wr_monster(monster_type *m_ptr)
{
    wr_u16b(m_ptr->r_idx);
    wr_byte(m_ptr->fy);
    wr_byte(m_ptr->fx);
    wr_u16b(m_ptr->hp);
    wr_u16b(m_ptr->maxhp);
    wr_u16b(m_ptr->csleep);
    wr_byte(m_ptr->mspeed);
    wr_byte(m_ptr->energy);
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
    byte tmp8u;
    u32b tmp32u;

    if (older_than(2,7,1)) {

	rd_u16b(&l_ptr->r_kills);
	rd_u16b(&l_ptr->r_deaths);

	rd_u32b(&l_ptr->r_spells1);
	rd_u32b(&l_ptr->r_spells2);
	rd_u32b(&l_ptr->r_spells3);
	rd_u32b(&l_ptr->r_cflags1);
	rd_u32b(&l_ptr->r_cflags2);
	rd_u32b(&tmp32u);

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
	rd_u32b(&l_ptr->r_spells1);
	rd_u32b(&l_ptr->r_spells2);
	rd_u32b(&l_ptr->r_spells3);
	rd_u32b(&l_ptr->r_cflags1);
	rd_u32b(&l_ptr->r_cflags2);
	rd_u32b(&tmp32u);

	/* XXX Hack -- fix my mistakes */
	l_ptr->r_cflags1 &= ~CM1_TREASURE;
	l_ptr->r_spells1 &= ~CS1_FREQ;

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
	rd_u16b(&l_ptr->r_kills);

	/* Count deaths of player */
	rd_u16b(&l_ptr->r_deaths);

	/* Read the "Racial" monster limit per level */
	rd_byte(&l_ptr->max_num);

	/* Later */
	rd_byte(&tmp8u);
    }
}

static void wr_lore(monster_lore *l_ptr)
{
    /* Write the info */
    wr_u32b(l_ptr->r_spells1);
    wr_u32b(l_ptr->r_spells2);
    wr_u32b(l_ptr->r_spells3);
    wr_u32b(l_ptr->r_cflags1);
    wr_u32b(l_ptr->r_cflags2);
    wr_u32b(0L);

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
    wr_u16b(l_ptr->r_kills);
    wr_u16b(l_ptr->r_deaths);

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
    byte tmp8u;

    rd_byte(&tmp8u);

    xtra->aware = (tmp8u & 0x01) ? TRUE: FALSE;
    xtra->tried = (tmp8u & 0x02) ? TRUE: FALSE;
}

static void wr_xtra(inven_xtra *xtra)
{
    byte tmp8u = 0;

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

    wr_u32b(st_ptr->store_open);
    wr_u16b(st_ptr->insult_cur);
    wr_byte(st_ptr->owner);
    wr_byte(st_ptr->store_ctr);
    wr_u16b(st_ptr->good_buy);
    wr_u16b(st_ptr->bad_buy);

    /* Write the items */
    for (j = 0; j < st_ptr->store_ctr; j++) {
	wr_item(&st_ptr->store_item[j]);
    }
}


static errr rd_store(store_type *st_ptr)
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
	rd_item(&st_ptr->store_item[j]);
    }

    /* Success */
    return (0);
}





/*
 * Read options -- 2.7.4 format
 *
 * Some options from 2.7.0 to 2.7.3 will get scrambled.
 */
static void rd_options(void)
{
    byte tmp8u;
    u32b l;


    /* Option set 1 -- ??? */
    
    rd_u32b(&l);

    rogue_like_commands =  (l & 0x00000001L) ? TRUE : FALSE;
    prompt_carry_flag =    (l & 0x00000002L) ? TRUE : FALSE;
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
    new_screen_layout =    (l & 0x00002000L) ? TRUE : FALSE;
    depth_in_feet =	   (l & 0x00004000L) ? TRUE : FALSE;
    hilite_player =	   (l & 0x00008000L) ? TRUE : FALSE;

    use_recall_win =       (l & 0x00100000L) ? TRUE : FALSE;
    use_choice_win =       (l & 0x00200000L) ? TRUE : FALSE;


    /* Option set 2 -- ??? */
    
    rd_u32b(&l);

    compress_savefile =    (l & 0x00000010L) ? TRUE : FALSE;
    
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

    view_yellow_lite =     (l & 0x01000000L) ? TRUE : FALSE;
    view_bright_lite =     (l & 0x02000000L) ? TRUE : FALSE;
    view_yellow_fast =     (l & 0x04000000L) ? TRUE : FALSE;
    view_bright_fast =     (l & 0x08000000L) ? TRUE : FALSE;


    /* Option set 3 -- Gameplay */

    rd_u32b(&l);

    view_pre_compute =		(l & 0x00000001L) ? TRUE : FALSE;
    view_xxx_compute =		(l & 0x00000002L) ? TRUE : FALSE;
    view_reduce_view =		(l & 0x00000004L) ? TRUE : FALSE;
    view_reduce_lite =		(l & 0x00000008L) ? TRUE : FALSE;

    view_perma_grids =		(l & 0x00000010L) ? TRUE : FALSE;
    view_torch_grids =		(l & 0x00000020L) ? TRUE : FALSE;
    view_wall_memory =		(l & 0x00000040L) ? TRUE : FALSE;
    view_xtra_memory =		(l & 0x00000080L) ? TRUE : FALSE;

    flow_by_sound =		(l & 0x00000100L) ? TRUE : FALSE; 
    flow_by_smell =		(l & 0x00000200L) ? TRUE : FALSE;

    no_haggle_flag =		(l & 0x00010000L) ? TRUE : FALSE; 
    shuffle_owners =		(l & 0x00020000L) ? TRUE : FALSE;

    show_inven_weight =		(l & 0x00100000L) ? TRUE : FALSE;
    show_equip_weight =		(l & 0x00200000L) ? TRUE : FALSE;
    show_store_weight =		(l & 0x00400000L) ? TRUE : FALSE;
    plain_descriptions =	(l & 0x00800000L) ? TRUE : FALSE;

    stack_allow_items =		(l & 0x01000000L) ? TRUE : FALSE;
    stack_allow_wands =		(l & 0x02000000L) ? TRUE : FALSE;
    stack_force_notes =		(l & 0x04000000L) ? TRUE : FALSE;
    stack_force_costs =		(l & 0x08000000L) ? TRUE : FALSE;


    /* Future options */
    rd_u32b(&l);


    /* Read "delay_spd" */
    rd_byte(&tmp8u);
    delay_spd = tmp8u;

    /* Read "hitpoint_warn" */
    rd_byte(&tmp8u);
    hitpoint_warn = tmp8u;

    /* Future options */
    rd_byte(&tmp8u);
    rd_byte(&tmp8u);
}


/*
 * Write the options -- 2.7.4 format
 */
static void wr_options(void)
{
    u32b l;

    /* Option set 1 and 2 */

    l = 0L;

    if (rogue_like_commands)	l |= 0x00000001L;
    if (prompt_carry_flag)	l |= 0x00000002L;
    if (carry_query_flag)	l |= 0x00000004L;
    if (always_throw)		l |= 0x00000008L;
    if (always_repeat)		l |= 0x00000010L;
    if (quick_messages)		l |= 0x00000020L;
    if (use_old_target)		l |= 0x00000040L;
    if (always_pickup)		l |= 0x00000080L;
    
    if (use_color)		l |= 0x00000100L;
    if (notice_seams)		l |= 0x00000400L;
    if (ring_bell)		l |= 0x00000800L;
    if (equippy_chars)		l |= 0x00001000L;
    if (new_screen_layout)	l |= 0x00002000L;
    if (depth_in_feet)		l |= 0x00004000L;
    if (hilite_player)		l |= 0x00008000L;

    if (use_recall_win)		l |= 0x00100000L;
    if (use_choice_win)		l |= 0x00200000L;

    wr_u32b(l);


    /* Option set 1 and 2 */

    l = 0L;

    if (compress_savefile)	l |= 0x00000010L;

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

    if (view_yellow_lite)	l |= 0x01000000L;
    if (view_bright_lite)	l |= 0x02000000L;
    if (view_yellow_fast)	l |= 0x04000000L;
    if (view_bright_fast)	l |= 0x08000000L;

    wr_u32b(l);


    /* Option set 3 -- Gameplay */
    
    l = 0L;

    if (view_pre_compute)		l |= 0x00000001L;
    if (view_xxx_compute)		l |= 0x00000002L;
    if (view_reduce_view)		l |= 0x00000004L;
    if (view_reduce_lite)		l |= 0x00000008L;

    if (view_perma_grids)		l |= 0x00000010L;
    if (view_torch_grids)		l |= 0x00000020L;
    if (view_wall_memory)		l |= 0x00000040L;
    if (view_xtra_memory)		l |= 0x00000080L;

    if (flow_by_sound)			l |= 0x00000100L;
    if (flow_by_smell)			l |= 0x00000200L;

    if (no_haggle_flag)			l |= 0x00010000L;
    if (shuffle_owners)			l |= 0x00020000L;

    if (show_inven_weight)		l |= 0x00100000L;
    if (show_equip_weight)		l |= 0x00200000L;
    if (show_store_weight)		l |= 0x00400000L;
    if (plain_descriptions)		l |= 0x00800000L;

    if (stack_allow_items)		l |= 0x01000000L;
    if (stack_allow_wands)		l |= 0x02000000L;
    if (stack_force_notes)		l |= 0x04000000L;
    if (stack_force_costs)		l |= 0x08000000L;

    wr_u32b(l);


    /* Future options */    
    wr_u32b(0L);



    /* Write "delay_spd" */
    wr_byte(delay_spd);

    /* Write "hitpoint_warn" */
    wr_byte(hitpoint_warn);

    /* Future options */
    wr_byte(0);
    wr_byte(0);
}





static void rd_ghost()
{
    monster_race *r_ptr = &r_list[MAX_R_IDX-1];

    byte tmp8u;
    u32b tmp32u; 


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
    rd_u16b(&r_ptr->ac);
    rd_u16b(&r_ptr->sleep);
    rd_byte(&r_ptr->aaf);
    rd_byte(&r_ptr->speed);

    rd_u32b(&r_ptr->mexp);

    rd_u16b(&r_ptr->damage[0]);
    rd_u16b(&r_ptr->damage[1]);
    rd_u16b(&r_ptr->damage[2]);
    rd_u16b(&r_ptr->damage[3]);

    rd_u32b(&r_ptr->cflags1);
    rd_u32b(&r_ptr->cflags2);
    rd_u32b(&tmp32u);

    rd_u32b(&r_ptr->spells1);
    rd_u32b(&r_ptr->spells2);
    rd_u32b(&r_ptr->spells3);

    /* Hack -- fix speed in old versions */
    if (older_than(2,7,0)) {
	r_ptr->speed = (r_ptr->speed - 10) * 10 + 100;
    }

    /* Hack -- set the "graphic" info */
    r_attr[MAX_R_IDX-1] = r_ptr->r_attr;
    r_char[MAX_R_IDX-1] = r_ptr->r_char;
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
    wr_u16b(r_ptr->ac);
    wr_u16b(r_ptr->sleep);
    wr_byte(r_ptr->aaf);
    wr_byte(r_ptr->speed);

    wr_u32b(r_ptr->mexp);

    wr_u16b(r_ptr->damage[0]);
    wr_u16b(r_ptr->damage[1]);
    wr_u16b(r_ptr->damage[2]);
    wr_u16b(r_ptr->damage[3]);

    wr_u32b(r_ptr->cflags1);
    wr_u32b(r_ptr->cflags2);
    wr_u32b(0L);

    wr_u32b(r_ptr->spells1);
    wr_u32b(r_ptr->spells2);
    wr_u32b(r_ptr->spells3);
}




/*
 * Read/Write the "extra" information
 */

static void rd_extra()
{
    int i;
    byte tmp8u;
    s16b tmp16s;
    u32b tmp32u;

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
    rd_u16b(&p_ptr->age);
    rd_u16b(&p_ptr->ht);
    rd_u16b(&p_ptr->wt);

    /* Read the stats, Keep it simple */    
    for (i = 0; i < 6; i++) rd_s16b(&p_ptr->max_stat[i]);
    for (i = 0; i < 6; i++) rd_s16b(&p_ptr->cur_stat[i]);
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

    rd_s16b(&p_ptr->mana);
    rd_s16b(&p_ptr->cmana);
    rd_u16b(&p_ptr->cmana_frac);

    rd_s16b(&p_ptr->max_plv);
    rd_s16b(&p_ptr->max_dlv);

    /* More info */
    rd_s16b(&p_ptr->srh);
    rd_s16b(&p_ptr->fos);
    rd_s16b(&p_ptr->disarm);
    rd_s16b(&p_ptr->save);
    rd_s16b(&p_ptr->sc);
    rd_s16b(&p_ptr->stl);
    rd_s16b(&p_ptr->bth);
    rd_s16b(&p_ptr->bthb);
    
    /* Skip old bonuses */
    rd_s16b(&tmp16s);
    rd_s16b(&tmp16s);
    rd_s16b(&tmp16s);
    rd_s16b(&tmp16s);

    /* Skip old displayed bonuses */
    rd_s16b(&tmp16s);
    rd_s16b(&tmp16s);
    rd_s16b(&tmp16s);
    rd_s16b(&tmp16s);


    /* Read the flags */
    rd_u32b(&p_ptr->status);
    rd_s16b(&p_ptr->rest);
    rd_s16b(&p_ptr->blind);
    rd_s16b(&p_ptr->paralysis);
    rd_s16b(&p_ptr->confused);
    rd_s16b(&p_ptr->food);
    rd_s16b(&p_ptr->food_digested);
    rd_s16b(&p_ptr->protection);
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
    rd_s16b(&p_ptr->detect_inv);
    rd_s16b(&p_ptr->word_recall);
    rd_s16b(&p_ptr->see_infra);
    rd_s16b(&p_ptr->tim_infra);
    rd_s16b(&p_ptr->oppose_fire);
    rd_s16b(&p_ptr->oppose_cold);
    rd_s16b(&p_ptr->oppose_acid);
    rd_s16b(&p_ptr->oppose_elec);
    rd_s16b(&p_ptr->oppose_pois);
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
    for (i = 0; i < 15; i++) rd_u32b(&tmp32u);


    /* This is not that important any more */
    p_ptr->searching = 0;
    p_ptr->status &= ~PY_SEARCH;


    /* Hack -- the two "special seeds" */            
    rd_u32b(&randes_seed);
    rd_u32b(&town_seed);


    /* Special stuff */
    rd_s16b(&panic_save);
    rd_s16b(&total_winner);
    rd_s16b(&noscore);


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

    wr_u16b(p_ptr->age);
    wr_u16b(p_ptr->ht);
    wr_u16b(p_ptr->wt);

    /* Dump the stats */
    for (i = 0; i < 6; ++i) wr_s16b(p_ptr->max_stat[i]);
    for (i = 0; i < 6; ++i) wr_s16b(p_ptr->cur_stat[i]);
    for (i = 0; i < 6; ++i) wr_s16b(p_ptr->mod_stat[i]);
    for (i = 0; i < 6; ++i) wr_s16b(p_ptr->use_stat[i]);

    wr_u32b(p_ptr->au);

    wr_u32b(p_ptr->max_exp);
    wr_u32b(p_ptr->exp);
    wr_u16b(p_ptr->exp_frac);
    wr_u16b(p_ptr->lev);

    wr_u16b(p_ptr->mhp);
    wr_u16b(p_ptr->chp);
    wr_u16b(p_ptr->chp_frac);

    wr_u16b(p_ptr->mana);
    wr_u16b(p_ptr->cmana);
    wr_u16b(p_ptr->cmana_frac);

    /* Max Player and Dungeon Levels */
    wr_u16b(p_ptr->max_plv);
    wr_u16b(p_ptr->max_dlv);

    /* More info */
    wr_u16b(p_ptr->srh);
    wr_u16b(p_ptr->fos);
    wr_u16b(p_ptr->disarm);
    wr_u16b(p_ptr->save);
    wr_u16b(p_ptr->sc);
    wr_u16b(p_ptr->stl);
    wr_u16b(p_ptr->bth);
    wr_u16b(p_ptr->bthb);
    
    /* Ignore old bonuses */
    wr_u16b(0);
    wr_u16b(0);
    wr_u16b(0);
    wr_u16b(0);

    /* Ignore old displayed bonuses */
    wr_u16b(0);
    wr_u16b(0);
    wr_u16b(0);
    wr_u16b(0);

    /* XXX Warning -- some of these should be signed */
    wr_u32b(p_ptr->status);
    wr_u16b(p_ptr->rest);
    wr_u16b(p_ptr->blind);
    wr_u16b(p_ptr->paralysis);
    wr_u16b(p_ptr->confused);
    wr_u16b(p_ptr->food);
    wr_u16b(p_ptr->food_digested);
    wr_u16b(p_ptr->protection);
    wr_u16b(p_ptr->energy);
    wr_u16b(p_ptr->fast);
    wr_u16b(p_ptr->slow);
    wr_u16b(p_ptr->afraid);
    wr_u16b(p_ptr->cut);
    wr_u16b(p_ptr->stun);
    wr_u16b(p_ptr->poisoned);
    wr_u16b(p_ptr->image);
    wr_u16b(p_ptr->protevil);
    wr_u16b(p_ptr->invuln);
    wr_u16b(p_ptr->hero);
    wr_u16b(p_ptr->shero);
    wr_u16b(p_ptr->shield);
    wr_u16b(p_ptr->blessed);
    wr_u16b(p_ptr->detect_inv);
    wr_u16b(p_ptr->word_recall);
    wr_u16b(p_ptr->see_infra);
    wr_u16b(p_ptr->tim_infra);
    wr_u16b(p_ptr->oppose_fire);
    wr_u16b(p_ptr->oppose_cold);
    wr_u16b(p_ptr->oppose_acid);
    wr_u16b(p_ptr->oppose_elec);
    wr_u16b(p_ptr->oppose_pois);
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
    for (i = 0; i < 15; i++) wr_u32b(0L);


    /* Write the "object seeds" */
    wr_u32b(randes_seed);
    wr_u32b(town_seed);


    /* Special stuff */
    wr_s16b(panic_save);
    wr_s16b(total_winner);
    wr_s16b(noscore);


    /* Write death */
    wr_byte(death);

    /* Write feeling */
    wr_byte(feeling);

    /* Turn of last "feeling" */
    wr_u32b(old_turn);

    /* Current turn */
    wr_u32b(turn);
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


/*
 * Read the player inventory
 *
 * Note that the inventory changed from 2.7.3 to 2.7.4.  Two extra
 * pack slots were added and the equipment was rearranged.  Note
 * that these two features combine when parsing old save-files, in
 * which items from the old "aux" slot are "carried", perhaps into
 * one of the two new "inventory" slots.
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
	if (!forge.tval) return (53);

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

static void wr_dungeon()
{
    int i, j;
    byte count, prev_char;
    byte tmp8u;
    cave_type *c_ptr;

    /* Dungeon specific info follows */
    wr_u16b(dun_level);
    wr_u16b(0);		/* Was mon_tot_mult */
    wr_u16b(char_row);
    wr_u16b(char_col);
    wr_u16b(cur_height);
    wr_u16b(cur_width);
    wr_u16b(max_panel_rows);
    wr_u16b(max_panel_cols);


    /*** Simple "Run-Length-Encoding" of cave ***/

    /* Note that this will induce two wasted bytes */
    count = 0;
    prev_char = 0;

    /* Dump the cave */
    for (i = 0; i < cur_height; i++) {
	for (j = 0; j < cur_width; j++) {

	    byte t_lr, t_fm, t_pl, t_xx;

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
	    t_xx = (c_ptr->info & CAVE_INIT) ? 1 : 0;

	    /* Create an encoded byte of info */            
	    tmp8u = (c_ptr->fval);
	    tmp8u |= ((t_lr << 4) | (t_fm << 5) | (t_pl << 6) | (t_xx << 7));

	    /* If the run is broken, or too full, flush it */
	    if (tmp8u != prev_char || count == MAX_UCHAR) {
		wr_byte((byte) count);
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
	wr_byte((byte) count);
	wr_byte(prev_char);
    }


    /* Dump the items (note: starting at #1) */
    wr_u16b(i_max);
    for (i = MIN_I_IDX; i < i_max; i++) {
	wr_item(&i_list[i]);
    }


    /* Dump the monsters (note: starting at #2) */    
    wr_u16b(m_max);
    for (i = MIN_M_IDX; i < m_max; i++) {
	wr_monster(&m_list[i]);
    }
}









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


    /* Header info */            
    rd_s16b(&dun_level);
    rd_s16b(&mon_tot_mult);
    rd_s16b(&char_row);
    rd_s16b(&char_col);
    rd_s16b(&cur_height);
    rd_s16b(&cur_width);
    rd_s16b(&max_panel_rows);
    rd_s16b(&max_panel_cols);

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
	    if ((tmp8u >> 7) & 0x1) c_ptr->info |= CAVE_INIT;

	    /* Hack -- Repair old savefiles */
	    if (older_than(2,7,3)) {

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
    rd_s16b(&m_max);
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
 * Hack -- see "save-old.c"
 */


/*
 * Actually read the savefile
 */
static errr rd_savefile()
{
    int i;

    byte tmp8u;
    u16b tmp16u;
    u32b tmp32u;

#ifdef VERIFY_CHECKSUMS
    u32b n_x_check, n_v_check;
    u32b o_x_check, o_v_check;
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
    if (older_than(2,7,0)) {
	extern errr rd_old_sf(FILE *fff1, int vmaj, int vmin, int vpat, int say);
	return (rd_old_sf(fff, version_maj, version_min, patch_level, say));
    }

    /* Operating system info */
    rd_u32b(&sf_xtra);

    /* Time of savefile creation */
    rd_u32b(&sf_when);

    /* Number of resurrections */
    rd_u16b(&sf_lives);

    /* Number of times played */
    rd_u16b(&sf_saves);


    /* A "sized" chunk of "unused" space */
    rd_u32b(&tmp32u);

    /* Read (and forget) those bytes */
    for (i = 0; i < tmp32u; i++) rd_byte(&tmp8u);


    /* A "sized" list of "strings" */
    rd_u32b(&tmp32u);

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
    rd_u16b(&tmp16u);
    for (i = 0; i < tmp16u; i++) rd_lore(&l_list[i]);
    if (say) prt_note(-1,"Loaded Monster Memory");


    /* Object Memory */
    rd_u16b(&tmp16u);
    for (i = 0; i < tmp16u; i++) rd_xtra(&x_list[i]);
    if (say) prt_note(-1,"Loaded Object Memory");


    /* Load the Quests */
    rd_u16b(&tmp16u);
    for (i = 0; i < tmp16u; i++) {
	rd_byte(&tmp8u);
	q_list[i].level = tmp8u;
	rd_byte(&tmp8u);
	rd_byte(&tmp8u);
	rd_byte(&tmp8u);
    }
    if (say) prt_note(-1,"Loaded Quests");


    /* Load the Artifacts */
    rd_u16b(&tmp16u);
    for (i = 0; i < tmp16u; i++) {
	rd_byte(&tmp8u);
	v_list[i].cur_num = tmp8u;
	rd_byte(&tmp8u);
	rd_byte(&tmp8u);
	rd_byte(&tmp8u);
    }
    if (say) prt_note(-1,"Loaded Artifacts");

    /* Hack -- make sure Grond/Morgoth are available */
    if (older_than(2,7,2)) {
	v_list[ART_GROND].cur_num = 0;
	v_list[ART_MORGOTH].cur_num = 0;
    }


    /* Read the extra stuff */
    rd_extra();
    if (say) prt_note(-1, "Loaded extra information");


    /* Read the player_hp array */
    rd_u16b(&tmp16u);
    for (i = 0; i < tmp16u; i++) {
	rd_u16b(&player_hp[i]);
    }


    /* Read spell info */
    rd_u32b(&spell_learned);
    rd_u32b(&spell_learned2);
    rd_u32b(&spell_worked);
    rd_u32b(&spell_worked2);
    rd_u32b(&spell_forgotten);
    rd_u32b(&spell_forgotten2);

    for (i = 0; i < 64; i++) {
	rd_byte(&spell_order[i]);
    }


    /* Read the inventory */
    if (rd_inventory()) {
	prt_note(-2, "Unable to read inventory");
	return (21);
    }


    /* Read the stores */
    rd_u16b(&tmp16u);
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
    rd_u32b(&o_v_check);

    /* Verify */
    if (o_v_check != n_v_check) {
	prt_note(-2, "Invalid checksum");
	return (11);
    }


    /* Save the encoded checksum */
    n_x_check = x_check;

    /* Read the checksum */
    rd_u32b(&o_x_check);


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

    u32b              now;

    byte		tmp8u;
    u16b		tmp16u;


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


    /* XXX This could probably be more "efficient" for "unseen" monsters */
    /* XXX But note that "max_num" is stored here as well (always "set") */

    /* Dump the monster lore */
    tmp16u = MAX_R_IDX;
    wr_u16b(tmp16u);
    for (i = 0; i < tmp16u; i++) wr_lore(&l_list[i]);


    /* Dump the object memory */
    tmp16u = MAX_K_IDX;
    wr_u16b(tmp16u);
    for (i = 0; i < tmp16u; i++) wr_xtra(&x_list[i]);


    /* Hack -- Dump the quests */    
    tmp16u = QUEST_MAX;
    wr_u16b(tmp16u);
    for (i = 0; i < tmp16u; i++) {
	wr_byte(q_list[i].level);
	wr_byte(0);
	wr_byte(0);
	wr_byte(0);
    }

    /* Hack -- Dump the artifacts */
    tmp16u = ART_MAX;
    wr_u16b(tmp16u);
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
    wr_u16b(tmp16u);
    for (i = 0; i < tmp16u; i++) {
	wr_u16b(player_hp[i]);
    }


    /* Write spell data */
    wr_u32b(spell_learned);
    wr_u32b(spell_learned2);
    wr_u32b(spell_worked);
    wr_u32b(spell_worked2);
    wr_u32b(spell_forgotten);
    wr_u32b(spell_forgotten2);

    /* Dump the ordered spells */
    for (i = 0; i < 64; i++) {
	wr_byte(spell_order[i]);
    }


    /* Write the inventory */
    for (i = 0; i < INVEN_TOTAL; i++) {
	if (inventory[i].tval) {
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
    wr_u32b(v_check);

    /* Write the "encoded checksum" */    
    wr_u32b(x_check);


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

#ifdef MACINTOSH
    _ftype = 'SAVE';
#endif
    
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

#ifdef MACINTOSH
    _ftype = 'SAVE';
#endif
    
	/* The save file is a binary file */
	fff = my_tfopen(fnam, "wb");
    }

#endif

    /* Successful open */
    if (fff) {

	/* Forget the "torch lite" */
	forget_lite();
	forget_view();

	/* Write the savefile */
	ok = wr_savefile();

	/* Attempt to close it */
	if (fclose(fff) == EOF) ok = FALSE;

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
    u32b                 age;

    vtype temp;


    /* Hack -- allow "debugging" */
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


    /* Notify the player */
    clear_screen();
    (void)sprintf(temp, "Restoring Character.");
    put_str(temp, 23, 0);

    /* Hack -- let the message get read */
    delay(1000);

    /* Allow restoring a file belonging to someone else, */
    /* but only if we can delete it. */
    /* Hence first try to read without doing a chmod. */

    /* Open the BINARY savefile */
    fd = my_topen(savefile, O_RDONLY | O_BINARY, 0);

    if (fd < 0) {
	msg_print("Can't open file for reading.");
    }

    else {

#if !defined(SET_UID) && !defined(ALLOW_FIDDLING)
	struct stat         statbuf;
#endif

	/* Already done, but can't hurt */
	turn = 0;

	ok = TRUE;

#if !defined(SET_UID) && !defined(ALLOW_FIDDLING)
	(void)fstat(fd, &statbuf);
#endif

	(void)close(fd);


	/* The savefile is a binary file */
	fff = my_tfopen(savefile, "rb");
	if (!fff) goto error;


	/* Actually read the savefile */
	if (rd_savefile()) goto error;

	/* Hack -- Alive, so no need to make a cave */
	if (!death) *generate = FALSE;


#if !defined(SET_UID) && !defined(ALLOW_FIDDLING)
	if (!wiz) {
	    if (sf_when > (statbuf.st_ctime + 100) ||
		sf_when < (statbuf.st_ctime - 100)) {
		prt_note(-2,"Fiddled save file");
		goto error;
	    }
	}
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

		p_ptr->image = 0;
		p_ptr->cut = 0;
		p_ptr->stun = 0;
		p_ptr->word_recall = 0;

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

		/* Re-calculate bonuses */
		calc_bonuses();
		

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


		/* Mega-Hack -- fix problems with 2.7.0/2.7.1 */
		if (older_than(2,7,2)) {

		    int i, y, x;

		    /* Scan the monster list */
		    for (i = MIN_M_IDX; i < m_max; i++) {
		    
			monster_type *m_ptr = &m_list[i];

			/* Get the location */
			x = m_ptr->fx, y = m_ptr->fy;
			
			/* Notice "missing" monsters */
			if (cave[y][x].m_idx == i) continue;

			/* Hack -- find a "naked" floor grid */
			while (!naked_grid_bold(y, x)) {
			    y = rand_int(cur_height);
			    x = rand_int(cur_width);
			}

			/* Save the new location */
			m_ptr->fy = y;
			m_ptr->fx = x;

			/* Use the new location instead */
			cave[y][x].m_idx = i;
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

	    /* Hack -- let the system react to new options */
	    Term_xtra(TERM_XTRA_REACT, 0);

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


