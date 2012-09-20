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

    wr_byte(i_ptr->unused);
    wr_byte(i_ptr->discount);

    wr_s32b(i_ptr->cost);
    wr_s32b(i_ptr->scost);

    wr_u32b(i_ptr->flags1);
    wr_u32b(i_ptr->flags2);
    wr_u32b(i_ptr->flags3);
    wr_u32b(0L);

    wr_string(i_ptr->inscrip);
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
    wr_byte(m_ptr->dead);
}


/*
 * Write a "lore" record
 */
static void wr_lore(monster_lore *l_ptr)
{
    /* Count sights/deaths/kills */
    wr_s16b(l_ptr->sights);
    wr_s16b(l_ptr->deaths);
    wr_s16b(l_ptr->pkills);
    wr_s16b(l_ptr->tkills);

    /* Count wakes and ignores */
    wr_byte(l_ptr->wake);
    wr_byte(l_ptr->ignore);

    /* Extra stuff */
    wr_byte(l_ptr->xtra1);
    wr_byte(l_ptr->xtra2);

    /* Count drops */
    wr_byte(l_ptr->drop_gold);
    wr_byte(l_ptr->drop_item);

    /* Count spells */
    wr_byte(l_ptr->cast_inate);
    wr_byte(l_ptr->cast_spell);

    /* Count blows of each type */
    wr_byte(l_ptr->blows[0]);
    wr_byte(l_ptr->blows[1]);
    wr_byte(l_ptr->blows[2]);
    wr_byte(l_ptr->blows[3]);
    
    /* Memorize flags */
    wr_u32b(l_ptr->flags1);
    wr_u32b(l_ptr->flags2);
    wr_u32b(l_ptr->flags3);
    wr_u32b(l_ptr->flags4);
    wr_u32b(l_ptr->flags5);
    wr_u32b(l_ptr->flags6);
    

    /* Monster limit per level */
    wr_byte(l_ptr->max_num);

    /* Later (?) */
    wr_byte(0);
    wr_byte(0);
    wr_byte(0);
}


/*
 * Write an "xtra" record
 */
static void wr_xtra(inven_xtra *xtra)
{
    byte tmp8u = 0;

    if (xtra->aware) tmp8u |= 0x01;
    if (xtra->tried) tmp8u |= 0x02;

    wr_byte(tmp8u);
}


/*
 * Write a "store" record
 */
static void wr_store(store_type *st_ptr)
{
    int j;

    wr_u32b(st_ptr->store_open);
    wr_s16b(st_ptr->insult_cur);
    wr_byte(st_ptr->owner);
    wr_byte(st_ptr->store_ctr);
    wr_s16b(st_ptr->good_buy);
    wr_s16b(st_ptr->bad_buy);

    /* Write the items */
    for (j = 0; j < st_ptr->store_ctr; j++) {
        wr_item(&st_ptr->store_item[j]);
    }
}


/*
 * Write the "options"
 */
static void wr_options(void)
{
    u32b l;
    u16b c;

    /* General options */

    l = 0L;

    if (rogue_like_commands)	l |= 0x00000001L;
    if (other_query_flag)	l |= 0x00000002L;
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

    if (compress_savefile)	l |= 0x00100000L;

    if (view_yellow_lite)	l |= 0x01000000L;
    if (view_bright_lite)	l |= 0x02000000L;

    wr_u32b(l);


    /* Disturbance options */

    l = 0L;

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

    if (flush_command)		l |= 0x01000000L;
    if (flush_disturb)		l |= 0x04000000L;
    if (flush_failure)		l |= 0x08000000L;

    if (fresh_before)		l |= 0x10000000L;
    if (fresh_after)		l |= 0x20000000L;
    if (fresh_find)		l |= 0x40000000L;

    wr_u32b(l);


    /* Gameplay options */

    l = 0L;

    if (dungeon_align)			l |= 0x00000001L;
    if (dungeon_other)			l |= 0x00000002L;

    if (view_reduce_view)		l |= 0x00000004L;
    if (view_reduce_lite)		l |= 0x00000008L;

    if (view_perma_grids)		l |= 0x00000010L;
    if (view_torch_grids)		l |= 0x00000020L;
    if (view_wall_memory)		l |= 0x00000040L;
    if (view_xtra_memory)		l |= 0x00000080L;

    if (flow_by_sound)			l |= 0x00000100L;
    if (flow_by_smell)			l |= 0x00000200L;

    if (track_follow)			l |= 0x00000400L;
    if (track_target)			l |= 0x00000800L;

    if (smart_learn)			l |= 0x00004000L;
    if (smart_cheat)			l |= 0x00008000L;

    if (no_haggle_flag)			l |= 0x00010000L;
    if (shuffle_owners)			l |= 0x00020000L;

    if (show_spell_info)		l |= 0x00040000L;
    if (show_health_bar)		l |= 0x00080000L;

    if (show_inven_weight)		l |= 0x00100000L;
    if (show_equip_weight)		l |= 0x00200000L;
    if (show_store_weight)		l |= 0x00400000L;
    if (plain_descriptions)		l |= 0x00800000L;

    if (stack_allow_items)		l |= 0x01000000L;
    if (stack_allow_wands)		l |= 0x02000000L;
    if (stack_force_notes)		l |= 0x04000000L;
    if (stack_force_costs)		l |= 0x08000000L;

    if (begin_maximize)			l |= 0x10000000L;
    if (begin_preserve)			l |= 0x20000000L;

    wr_u32b(l);


    /* Special Options -- Recall/Choice */

    l = 0L;

    if (use_screen_win)		l |= 0x00000001L;
    if (use_recall_win)		l |= 0x00000004L;
    if (use_choice_win)		l |= 0x00000008L;

    if (recall_show_desc)	l |= 0x00010000L;
    if (recall_show_kill)	l |= 0x00020000L;

    if (choice_inven_wgt)	l |= 0x01000000L;
    if (choice_equip_wgt)	l |= 0x02000000L;

    if (choice_inven_xtra)	l |= 0x04000000L;
    if (choice_equip_xtra)	l |= 0x08000000L;

    wr_u32b(l);


    /* Write "delay_spd" */
    wr_byte(delay_spd);

    /* Write "hitpoint_warn" */
    wr_byte(hitpoint_warn);


    /* Save the "cheating" options */

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
    
    monster_race *r_ptr = &r_list[MAX_R_IDX-1];


    /* Name */
    wr_string(ghost_name);

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
    wr_byte(r_ptr->xtra1);
    wr_byte(r_ptr->xtra1);

    /* Frequency */
    wr_byte(r_ptr->freq_inate);
    wr_byte(r_ptr->freq_spell);

    /* Flags */
    wr_u32b(r_ptr->rflags1);
    wr_u32b(r_ptr->rflags2);
    wr_u32b(r_ptr->rflags3);
    wr_u32b(r_ptr->rflags4);
    wr_u32b(r_ptr->rflags5);
    wr_u32b(r_ptr->rflags6);

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
    wr_byte(p_ptr->new_spells);

    wr_byte(p_ptr->hitdie);
    wr_byte(p_ptr->expfact);

    wr_s16b(p_ptr->age);
    wr_s16b(p_ptr->ht);
    wr_s16b(p_ptr->wt);

    /* Dump the stats */
    for (i = 0; i < 6; ++i) wr_s16b(p_ptr->max_stat[i]);
    for (i = 0; i < 6; ++i) wr_s16b(p_ptr->cur_stat[i]);
    for (i = 0; i < 6; ++i) wr_s16b(p_ptr->mod_stat[i]);
    for (i = 0; i < 6; ++i) wr_s16b(p_ptr->use_stat[i]);

    wr_u32b(p_ptr->au);

    wr_u32b(p_ptr->max_exp);
    wr_u32b(p_ptr->exp);
    wr_u16b(p_ptr->exp_frac);
    wr_s16b(p_ptr->lev);

    wr_s16b(p_ptr->mhp);
    wr_s16b(p_ptr->chp);
    wr_u16b(p_ptr->chp_frac);

    wr_s16b(p_ptr->mana);
    wr_s16b(p_ptr->cmana);
    wr_u16b(p_ptr->cmana_frac);

    /* Max Player and Dungeon Levels */
    wr_s16b(p_ptr->max_plv);
    wr_s16b(p_ptr->max_dlv);

    /* More info */
    wr_s16b(p_ptr->srh);
    wr_s16b(p_ptr->fos);
    wr_s16b(p_ptr->disarm);
    wr_s16b(p_ptr->save);
    wr_s16b(p_ptr->sc);
    wr_s16b(p_ptr->stl);
    
    wr_s16b(p_ptr->rest);
    wr_s16b(p_ptr->blind);
    wr_s16b(p_ptr->paralysis);
    wr_s16b(p_ptr->confused);
    wr_s16b(p_ptr->food);
    wr_s16b(p_ptr->food_digested);
    wr_s16b(p_ptr->protection);
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
    wr_s16b(p_ptr->detect_inv);
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

    /* Write the new flags */
    wr_u32b(p_ptr->update);
    wr_u32b(p_ptr->notice);
    wr_u32b(p_ptr->redraw);


    /* Write the "object seeds" */
    wr_u32b(randes_seed);
    wr_u32b(town_seed);


    /* Special stuff */
    wr_u16b(panic_save);
    wr_u16b(total_winner);
    wr_u16b(noscore);


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
 * Write the current dungeon
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
    for (i = 0; i < cur_hgt; i++) {
        for (j = 0; j < cur_wid; j++) {

            /* Get the cave */
            c_ptr = &cave[i][j];

            /* Paranoia */
            if (c_ptr->i_idx) {
                inven_type *i_ptr = &i_list[c_ptr->i_idx];
                i_ptr->iy = i;
                i_ptr->ix = j;
            }

            /* Paranoia */
            if (c_ptr->m_idx > 1) {
                monster_type *m_ptr = &m_list[c_ptr->m_idx];
                m_ptr->fy = i;
                m_ptr->fx = j;
            }

            /* Semi-Hack -- Save the first eight bit-flags */
            tmp8u = (c_ptr->info & 0xFF);

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


    /* Dump the "real" items */
    wr_u16b(i_max);
    for (i = 1; i < i_max; i++) {
        inven_type *i_ptr = &i_list[i];
        wr_item(i_ptr);
    }


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
    for (i = 0; i < tmp16u; i++) wr_lore(&l_list[i]);


    /* Dump the object memory */
    tmp16u = MAX_K_IDX;
    wr_u16b(tmp16u);
    for (i = 0; i < tmp16u; i++) wr_xtra(&x_list[i]);


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
 */
int _save_player(char *fnam)
{
    int		ok, fd;

    char	temp[160];

    int		mode = 0644;
    
    
    /* Forbid suspend */
    signals_ignore_tstp();

    /* Handle stuff */
    handle_stuff(TRUE);

    /* Refresh */
    Term_fresh();
    
    /* Turn off resting and searching. */
    disturb(1, 0);

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

    /* Create a savefile */
    fd = my_topen(fnam, O_RDWR | O_CREAT | O_EXCL | O_BINARY, mode);

#ifdef MACINTOSH

    /* Missing directory? */
    if (fd < 0) {
        msg_print("Cannot write to savefile!");
    }

#else

    /* This might not work... */
    if ((fd < 0) && (access(fnam, 0) >= 0) &&
        (character_loaded ||
         (wizard && get_check("Can't make new savefile. Overwrite old?")))) {

        /* Open the file again? */
        (void)chmod(fnam, mode);
        fd = my_topen(fnam, O_RDWR | O_TRUNC | O_BINARY, mode);
    }

#endif

    /* File is okay */
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

        /* Write the savefile */
        ok = wr_savefile();

        /* Attempt to close it */
        if (fclose(fff) == EOF) ok = FALSE;
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
    int		result = FALSE;

    char	temp1[1024];
    char	temp2[1024];


#if defined(SET_UID) && defined(SECURE)
    /* Get "games" permissions */
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

        /* Hack -- Pretend the character was loaded */
        character_loaded = TRUE;

        /* Success */
        result = TRUE;
    }


#if defined(SET_UID) && defined(SECURE)
    /* Drop "games" permissions */
    bePlayer();
#endif


    /* Return the result */
    return (result);
}



