/**
 * \file borg-trait.c 
 * \brief The calculations to determine what items and abilities it has
 *        This code generally loads the arrays (borg_trait/has/activation)
 * 
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2007-9 Andi Sidwell, Chris Carr, Ed Graham, Erik Osheim
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband License":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#ifdef ALLOW_BORG

#include "borg-trait.h"

#include "../effects.h"
#include "../obj-util.h"
#include "../player-calcs.h"
#include "../player-timed.h"
#include "../player-util.h"

#include "borg.h"
#include "borg-flow.h"
#include "borg-item-analyze.h"
#include "borg-item-id.h"
#include "borg-item-use.h"
#include "borg-item-wear.h"
#include "borg-magic.h"
#include "borg-item-activation.h"
#include "borg-item-val.h"
#include "borg-trait-swap.h"

/* MAJOR HACK copied in because it is static in the main code */
/* I would just make them not static but trying not to change base code */
/* for now !FIX !TODO !AJG */
static const int borg_adj_mag_mana[STAT_RANGE] =
{
      0	/* 3 */,
     10	/* 4 */,
     20	/* 5 */,
     30	/* 6 */,
     40	/* 7 */,
     50	/* 8 */,
     60	/* 9 */,
     70	/* 10 */,
     80	/* 11 */,
     90	/* 12 */,
    100	/* 13 */,
    110	/* 14 */,
    120	/* 15 */,
    130	/* 16 */,
    140	/* 17 */,
    150	/* 18/00-18/09 */,
    160	/* 18/10-18/19 */,
    170	/* 18/20-18/29 */,
    180	/* 18/30-18/39 */,
    190	/* 18/40-18/49 */,
    200	/* 18/50-18/59 */,
    225	/* 18/60-18/69 */,
    250	/* 18/70-18/79 */,
    300	/* 18/80-18/89 */,
    350	/* 18/90-18/99 */,
    400	/* 18/100-18/109 */,
    450	/* 18/110-18/119 */,
    500	/* 18/120-18/129 */,
    550	/* 18/130-18/139 */,
    600	/* 18/140-18/149 */,
    650	/* 18/150-18/159 */,
    700	/* 18/160-18/169 */,
    750	/* 18/170-18/179 */,
    800	/* 18/180-18/189 */,
    800	/* 18/190-18/199 */,
    800	/* 18/200-18/209 */,
    800	/* 18/210-18/219 */,
    800	/* 18/220+ */
};

static const int borg_adj_dex_ta[STAT_RANGE] =
{
    -4	/* 3 */,
    -3	/* 4 */,
    -2	/* 5 */,
    -1	/* 6 */,
    0	/* 7 */,
    0	/* 8 */,
    0	/* 9 */,
    0	/* 10 */,
    0	/* 11 */,
    0	/* 12 */,
    0	/* 13 */,
    0	/* 14 */,
    1	/* 15 */,
    1	/* 16 */,
    1	/* 17 */,
    2	/* 18/00-18/09 */,
    2	/* 18/10-18/19 */,
    2	/* 18/20-18/29 */,
    2	/* 18/30-18/39 */,
    2	/* 18/40-18/49 */,
    3	/* 18/50-18/59 */,
    3	/* 18/60-18/69 */,
    3	/* 18/70-18/79 */,
    4	/* 18/80-18/89 */,
    5	/* 18/90-18/99 */,
    6	/* 18/100-18/109 */,
    7	/* 18/110-18/119 */,
    8	/* 18/120-18/129 */,
    9	/* 18/130-18/139 */,
    9	/* 18/140-18/149 */,
    10	/* 18/150-18/159 */,
    11	/* 18/160-18/169 */,
    12	/* 18/170-18/179 */,
    13	/* 18/180-18/189 */,
    14	/* 18/190-18/199 */,
    15	/* 18/200-18/209 */,
    15	/* 18/210-18/219 */,
    15	/* 18/220+ */
};

/**
 * Stat Table (STR) -- bonus to dam
 */
const int borg_adj_str_td[STAT_RANGE] =
{
    -2	/* 3 */,
    -2	/* 4 */,
    -1	/* 5 */,
    -1	/* 6 */,
    0	/* 7 */,
    0	/* 8 */,
    0	/* 9 */,
    0	/* 10 */,
    0	/* 11 */,
    0	/* 12 */,
    0	/* 13 */,
    0	/* 14 */,
    0	/* 15 */,
    1	/* 16 */,
    2	/* 17 */,
    2	/* 18/00-18/09 */,
    2	/* 18/10-18/19 */,
    3	/* 18/20-18/29 */,
    3	/* 18/30-18/39 */,
    3	/* 18/40-18/49 */,
    3	/* 18/50-18/59 */,
    3	/* 18/60-18/69 */,
    4	/* 18/70-18/79 */,
    5	/* 18/80-18/89 */,
    5	/* 18/90-18/99 */,
    6	/* 18/100-18/109 */,
    7	/* 18/110-18/119 */,
    8	/* 18/120-18/129 */,
    9	/* 18/130-18/139 */,
    10	/* 18/140-18/149 */,
    11	/* 18/150-18/159 */,
    12	/* 18/160-18/169 */,
    13	/* 18/170-18/179 */,
    14	/* 18/180-18/189 */,
    15	/* 18/190-18/199 */,
    16	/* 18/200-18/209 */,
    18	/* 18/210-18/219 */,
    20	/* 18/220+ */
};

/**
 * Stat Table (DEX) -- bonus to hit
 */
const int borg_adj_dex_th[STAT_RANGE] =
{
    -3	/* 3 */,
    -2	/* 4 */,
    -2	/* 5 */,
    -1	/* 6 */,
    -1	/* 7 */,
    0	/* 8 */,
    0	/* 9 */,
    0	/* 10 */,
    0	/* 11 */,
    0	/* 12 */,
    0	/* 13 */,
    0	/* 14 */,
    0	/* 15 */,
    1	/* 16 */,
    2	/* 17 */,
    3	/* 18/00-18/09 */,
    3	/* 18/10-18/19 */,
    3	/* 18/20-18/29 */,
    3	/* 18/30-18/39 */,
    3	/* 18/40-18/49 */,
    4	/* 18/50-18/59 */,
    4	/* 18/60-18/69 */,
    4	/* 18/70-18/79 */,
    4	/* 18/80-18/89 */,
    5	/* 18/90-18/99 */,
    6	/* 18/100-18/109 */,
    7	/* 18/110-18/119 */,
    8	/* 18/120-18/129 */,
    9	/* 18/130-18/139 */,
    9	/* 18/140-18/149 */,
    10	/* 18/150-18/159 */,
    11	/* 18/160-18/169 */,
    12	/* 18/170-18/179 */,
    13	/* 18/180-18/189 */,
    14	/* 18/190-18/199 */,
    15	/* 18/200-18/209 */,
    15	/* 18/210-18/219 */,
    15	/* 18/220+ */
};

/**
 * Stat Table (STR) -- bonus to hit
 */
static const int borg_adj_str_th[STAT_RANGE] =
{
    -3	/* 3 */,
    -2	/* 4 */,
    -1	/* 5 */,
    -1	/* 6 */,
    0	/* 7 */,
    0	/* 8 */,
    0	/* 9 */,
    0	/* 10 */,
    0	/* 11 */,
    0	/* 12 */,
    0	/* 13 */,
    0	/* 14 */,
    0	/* 15 */,
    0	/* 16 */,
    0	/* 17 */,
    1	/* 18/00-18/09 */,
    1	/* 18/10-18/19 */,
    1	/* 18/20-18/29 */,
    1	/* 18/30-18/39 */,
    1	/* 18/40-18/49 */,
    1	/* 18/50-18/59 */,
    1	/* 18/60-18/69 */,
    2	/* 18/70-18/79 */,
    3	/* 18/80-18/89 */,
    4	/* 18/90-18/99 */,
    5	/* 18/100-18/109 */,
    6	/* 18/110-18/119 */,
    7	/* 18/120-18/129 */,
    8	/* 18/130-18/139 */,
    9	/* 18/140-18/149 */,
    10	/* 18/150-18/159 */,
    11	/* 18/160-18/169 */,
    12	/* 18/170-18/179 */,
    13	/* 18/180-18/189 */,
    14	/* 18/190-18/199 */,
    15	/* 18/200-18/209 */,
    15	/* 18/210-18/219 */,
    15	/* 18/220+ */
};

static const int borg_adj_dex_blow[STAT_RANGE] =
{
    0	/* 3 */,
    0	/* 4 */,
    0	/* 5 */,
    0	/* 6 */,
    0	/* 7 */,
    0	/* 8 */,
    0	/* 9 */,
    1	/* 10 */,
    1	/* 11 */,
    1	/* 12 */,
    1	/* 13 */,
    1	/* 14 */,
    1	/* 15 */,
    1	/* 16 */,
    2	/* 17 */,
    2	/* 18/00-18/09 */,
    2	/* 18/10-18/19 */,
    3	/* 18/20-18/29 */,
    3	/* 18/30-18/39 */,
    4	/* 18/40-18/49 */,
    4	/* 18/50-18/59 */,
    5	/* 18/60-18/69 */,
    5	/* 18/70-18/79 */,
    6	/* 18/80-18/89 */,
    6	/* 18/90-18/99 */,
    7	/* 18/100-18/109 */,
    7	/* 18/110-18/119 */,
    8	/* 18/120-18/129 */,
    8	/* 18/130-18/139 */,
    8	/* 18/140-18/149 */,
    9	/* 18/150-18/159 */,
    9	/* 18/160-18/169 */,
    9	/* 18/170-18/179 */,
    10	/* 18/180-18/189 */,
    10	/* 18/190-18/199 */,
    11	/* 18/200-18/209 */,
    11	/* 18/210-18/219 */,
    11	/* 18/220+ */
};

static const int borg_blows_table[12][12] =
{
    /* P */
   /* D:   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11+ */
   /* DEX: 3,   10,  17,  /20, /40, /60, /80, /100,/120,/150,/180,/200 */

    /* 0  */
    {  100, 100, 95,  85,  75,  60,  50,  42,  35,  30,  25,  23 },

    /* 1  */
    {  100, 95,  85,  75,  60,  50,  42,  35,  30,  25,  23,  21 },

    /* 2  */
    {  95,  85,  75,  60,  50,  42,  35,  30,  26,  23,  21,  20 },

    /* 3  */
    {  85,  75,  60,  50,  42,  36,  32,  28,  25,  22,  20,  19 },

    /* 4  */
    {  75,  60,  50,  42,  36,  33,  28,  25,  23,  21,  19,  18 },

    /* 5  */
    {  60,  50,  42,  36,  33,  30,  27,  24,  22,  21,  19,  17 },

    /* 6  */
    {  50,  42,  36,  33,  30,  27,  25,  23,  21,  20,  18,  17 },

    /* 7  */
    {  42,  36,  33,  30,  28,  26,  24,  22,  20,  19,  18,  17 },

    /* 8  */
    {  36,  33,  30,  28,  26,  24,  22,  21,  20,  19,  17,  16 },

    /* 9  */
    {  35,  32,  29,  26,  24,  22,  21,  20,  19,  18,  17,  16 },

    /* 10 */
    {  34,  30,  27,  25,  23,  22,  21,  20,  19,  18,  17,  16 },

    /* 11+ */
    {  33,  29,  26,  24,  22,  21,  20,  19,  18,  17,  16,  15 },
    /* DEX: 3,   10,  17,  /20, /40, /60, /80, /100,/120,/150,/180,/200 */
};

static const int borg_adj_dex_dis[STAT_RANGE] =
{
    0	/* 3 */,
    0	/* 4 */,
    0	/* 5 */,
    0	/* 6 */,
    0	/* 7 */,
    1	/* 8 */,
    1	/* 9 */,
    1	/* 10 */,
    1	/* 11 */,
    1	/* 12 */,
    1	/* 13 */,
    1	/* 14 */,
    2	/* 15 */,
    2	/* 16 */,
    2	/* 17 */,
    3	/* 18/00-18/09 */,
    3	/* 18/10-18/19 */,
    3	/* 18/20-18/29 */,
    4	/* 18/30-18/39 */,
    4	/* 18/40-18/49 */,
    5	/* 18/50-18/59 */,
    6	/* 18/60-18/69 */,
    7	/* 18/70-18/79 */,
    8	/* 18/80-18/89 */,
    9	/* 18/90-18/99 */,
    10	/* 18/100-18/109 */,
    10	/* 18/110-18/119 */,
    11	/* 18/120-18/129 */,
    12	/* 18/130-18/139 */,
    13	/* 18/140-18/149 */,
    14	/* 18/150-18/159 */,
    15	/* 18/160-18/169 */,
    16	/* 18/170-18/179 */,
    17	/* 18/180-18/189 */,
    18	/* 18/190-18/199 */,
    19	/* 18/200-18/209 */,
    19	/* 18/210-18/219 */,
    19	/* 18/220+ */
};

static const int borg_adj_int_dis[STAT_RANGE] =
{
    0	/* 3 */,
    0	/* 4 */,
    0	/* 5 */,
    0	/* 6 */,
    0	/* 7 */,
    1	/* 8 */,
    1	/* 9 */,
    1	/* 10 */,
    1	/* 11 */,
    1	/* 12 */,
    1	/* 13 */,
    1	/* 14 */,
    2	/* 15 */,
    2	/* 16 */,
    2	/* 17 */,
    3	/* 18/00-18/09 */,
    3	/* 18/10-18/19 */,
    3	/* 18/20-18/29 */,
    4	/* 18/30-18/39 */,
    4	/* 18/40-18/49 */,
    5	/* 18/50-18/59 */,
    6	/* 18/60-18/69 */,
    7	/* 18/70-18/79 */,
    8	/* 18/80-18/89 */,
    9	/* 18/90-18/99 */,
    10	/* 18/100-18/109 */,
    10	/* 18/110-18/119 */,
    11	/* 18/120-18/129 */,
    12	/* 18/130-18/139 */,
    13	/* 18/140-18/149 */,
    14	/* 18/150-18/159 */,
    15	/* 18/160-18/169 */,
    16	/* 18/170-18/179 */,
    17	/* 18/180-18/189 */,
    18	/* 18/190-18/199 */,
    19	/* 18/200-18/209 */,
    19	/* 18/210-18/219 */,
    19	/* 18/220+ */
};

static const int borg_adj_int_dev[STAT_RANGE] =
{
    0	/* 3 */,
    0	/* 4 */,
    0	/* 5 */,
    0	/* 6 */,
    0	/* 7 */,
    1	/* 8 */,
    1	/* 9 */,
    1	/* 10 */,
    1	/* 11 */,
    1	/* 12 */,
    1	/* 13 */,
    1	/* 14 */,
    2	/* 15 */,
    2	/* 16 */,
    2	/* 17 */,
    3	/* 18/00-18/09 */,
    3	/* 18/10-18/19 */,
    3	/* 18/20-18/29 */,
    3	/* 18/30-18/39 */,
    3	/* 18/40-18/49 */,
    4	/* 18/50-18/59 */,
    4	/* 18/60-18/69 */,
    5	/* 18/70-18/79 */,
    5	/* 18/80-18/89 */,
    6	/* 18/90-18/99 */,
    6	/* 18/100-18/109 */,
    7	/* 18/110-18/119 */,
    7	/* 18/120-18/129 */,
    8	/* 18/130-18/139 */,
    8	/* 18/140-18/149 */,
    9	/* 18/150-18/159 */,
    9	/* 18/160-18/169 */,
    10	/* 18/170-18/179 */,
    10	/* 18/180-18/189 */,
    11	/* 18/190-18/199 */,
    11	/* 18/200-18/209 */,
    12	/* 18/210-18/219 */,
    13	/* 18/220+ */
};

static const int borg_adj_str_dig[STAT_RANGE] =
{
    0	/* 3 */,
    0	/* 4 */,
    1	/* 5 */,
    2	/* 6 */,
    3	/* 7 */,
    4	/* 8 */,
    4	/* 9 */,
    5	/* 10 */,
    5	/* 11 */,
    6	/* 12 */,
    6	/* 13 */,
    7	/* 14 */,
    7	/* 15 */,
    8	/* 16 */,
    8	/* 17 */,
    9	/* 18/00-18/09 */,
    10	/* 18/10-18/19 */,
    12	/* 18/20-18/29 */,
    15	/* 18/30-18/39 */,
    20	/* 18/40-18/49 */,
    25	/* 18/50-18/59 */,
    30	/* 18/60-18/69 */,
    35	/* 18/70-18/79 */,
    40	/* 18/80-18/89 */,
    45	/* 18/90-18/99 */,
    50	/* 18/100-18/109 */,
    55	/* 18/110-18/119 */,
    60	/* 18/120-18/129 */,
    65	/* 18/130-18/139 */,
    70	/* 18/140-18/149 */,
    75	/* 18/150-18/159 */,
    80	/* 18/160-18/169 */,
    85	/* 18/170-18/179 */,
    90	/* 18/180-18/189 */,
    95	/* 18/190-18/199 */,
    100	/* 18/200-18/209 */,
    100	/* 18/210-18/219 */,
    100	/* 18/220+ */
};

static const int borg_adj_wis_sav[STAT_RANGE] =
{
    0	/* 3 */,
    0	/* 4 */,
    0	/* 5 */,
    0	/* 6 */,
    0	/* 7 */,
    1	/* 8 */,
    1	/* 9 */,
    1	/* 10 */,
    1	/* 11 */,
    1	/* 12 */,
    1	/* 13 */,
    1	/* 14 */,
    2	/* 15 */,
    2	/* 16 */,
    2	/* 17 */,
    3	/* 18/00-18/09 */,
    3	/* 18/10-18/19 */,
    3	/* 18/20-18/29 */,
    3	/* 18/30-18/39 */,
    3	/* 18/40-18/49 */,
    4	/* 18/50-18/59 */,
    4	/* 18/60-18/69 */,
    5	/* 18/70-18/79 */,
    5	/* 18/80-18/89 */,
    6	/* 18/90-18/99 */,
    7	/* 18/100-18/109 */,
    8	/* 18/110-18/119 */,
    9	/* 18/120-18/129 */,
    10	/* 18/130-18/139 */,
    11	/* 18/140-18/149 */,
    12	/* 18/150-18/159 */,
    13	/* 18/160-18/169 */,
    14	/* 18/170-18/179 */,
    15	/* 18/180-18/189 */,
    16	/* 18/190-18/199 */,
    17	/* 18/200-18/209 */,
    18	/* 18/210-18/219 */,
    19	/* 18/220+ */
};

static const int borg_adj_str_wgt[STAT_RANGE] =
{
    5	/* 3 */,
    6	/* 4 */,
    7	/* 5 */,
    8	/* 6 */,
    9	/* 7 */,
    10	/* 8 */,
    11	/* 9 */,
    12	/* 10 */,
    13	/* 11 */,
    14	/* 12 */,
    15	/* 13 */,
    16	/* 14 */,
    17	/* 15 */,
    18	/* 16 */,
    19	/* 17 */,
    20	/* 18/00-18/09 */,
    22	/* 18/10-18/19 */,
    24	/* 18/20-18/29 */,
    26	/* 18/30-18/39 */,
    28	/* 18/40-18/49 */,
    30	/* 18/50-18/59 */,
    30	/* 18/60-18/69 */,
    30	/* 18/70-18/79 */,
    30	/* 18/80-18/89 */,
    30	/* 18/90-18/99 */,
    30	/* 18/100-18/109 */,
    30	/* 18/110-18/119 */,
    30	/* 18/120-18/129 */,
    30	/* 18/130-18/139 */,
    30	/* 18/140-18/149 */,
    30	/* 18/150-18/159 */,
    30	/* 18/160-18/169 */,
    30	/* 18/170-18/179 */,
    30	/* 18/180-18/189 */,
    30	/* 18/190-18/199 */,
    30	/* 18/200-18/209 */,
    30	/* 18/210-18/219 */,
    30	/* 18/220+ */
};

static const int borg_adj_con_mhp[STAT_RANGE] =
{
    -250	/* 3 */,
    -150	/* 4 */,
    -100	/* 5 */,
     -75	/* 6 */,
     -50	/* 7 */,
     -25	/* 8 */,
     -10	/* 9 */,
      -5	/* 10 */,
       0	/* 11 */,
       5	/* 12 */,
      10	/* 13 */,
      25	/* 14 */,
      50	/* 15 */,
      75	/* 16 */,
     100	/* 17 */,
     150	/* 18/00-18/09 */,
     175	/* 18/10-18/19 */,
     200	/* 18/20-18/29 */,
     225	/* 18/30-18/39 */,
     250	/* 18/40-18/49 */,
     275	/* 18/50-18/59 */,
     300	/* 18/60-18/69 */,
     350	/* 18/70-18/79 */,
     400	/* 18/80-18/89 */,
     450	/* 18/90-18/99 */,
     500	/* 18/100-18/109 */,
     550	/* 18/110-18/119 */,
     600	/* 18/120-18/129 */,
     650	/* 18/130-18/139 */,
     700	/* 18/140-18/149 */,
     750	/* 18/150-18/159 */,
     800	/* 18/160-18/169 */,
     900	/* 18/170-18/179 */,
    1000	/* 18/180-18/189 */,
    1100	/* 18/190-18/199 */,
    1250	/* 18/200-18/209 */,
    1250	/* 18/210-18/219 */,
    1250	/* 18/220+ */
};

/**
 * Stat Table (INT/WIS) -- Minimum failure rate (percentage)
 */
static const int borg_adj_mag_fail[STAT_RANGE] = {
    99 /* 3 */,
    99 /* 4 */,
    99 /* 5 */,
    99 /* 6 */,
    99 /* 7 */,
    50 /* 8 */,
    30 /* 9 */,
    20 /* 10 */,
    15 /* 11 */,
    12 /* 12 */,
    11 /* 13 */,
    10 /* 14 */,
    9 /* 15 */,
    8 /* 16 */,
    7 /* 17 */,
    6 /* 18/00-18/09 */,
    6 /* 18/10-18/19 */,
    5 /* 18/20-18/29 */,
    5 /* 18/30-18/39 */,
    5 /* 18/40-18/49 */,
    4 /* 18/50-18/59 */,
    4 /* 18/60-18/69 */,
    4 /* 18/70-18/79 */,
    4 /* 18/80-18/89 */,
    3 /* 18/90-18/99 */,
    3 /* 18/100-18/109 */,
    2 /* 18/110-18/119 */,
    2 /* 18/120-18/129 */,
    2 /* 18/130-18/139 */,
    2 /* 18/140-18/149 */,
    1 /* 18/150-18/159 */,
    1 /* 18/160-18/169 */,
    1 /* 18/170-18/179 */,
    1 /* 18/180-18/189 */,
    1 /* 18/190-18/199 */,
    0 /* 18/200-18/209 */,
    0 /* 18/210-18/219 */,
    0 /* 18/220+ */
};

/**
 * Stat Table (INT/WIS) -- failure rate adjustment
 */
static const int borg_adj_mag_stat[STAT_RANGE] = {
    -5 /* 3 */,
    -4 /* 4 */,
    -3 /* 5 */,
    -3 /* 6 */,
    -2 /* 7 */,
    -1 /* 8 */,
    0 /* 9 */,
    0 /* 10 */,
    0 /* 11 */,
    0 /* 12 */,
    0 /* 13 */,
    1 /* 14 */,
    2 /* 15 */,
    3 /* 16 */,
    4 /* 17 */,
    5 /* 18/00-18/09 */,
    6 /* 18/10-18/19 */,
    7 /* 18/20-18/29 */,
    8 /* 18/30-18/39 */,
    9 /* 18/40-18/49 */,
    10 /* 18/50-18/59 */,
    11 /* 18/60-18/69 */,
    12 /* 18/70-18/79 */,
    15 /* 18/80-18/89 */,
    18 /* 18/90-18/99 */,
    21 /* 18/100-18/109 */,
    24 /* 18/110-18/119 */,
    27 /* 18/120-18/129 */,
    30 /* 18/130-18/139 */,
    33 /* 18/140-18/149 */,
    36 /* 18/150-18/159 */,
    39 /* 18/160-18/169 */,
    42 /* 18/170-18/179 */,
    45 /* 18/180-18/189 */,
    48 /* 18/190-18/199 */,
    51 /* 18/200-18/209 */,
    54 /* 18/210-18/219 */,
    57 /* 18/220+ */
};

// !FIX !TODO !AJG probably wrap all this up in a "borg" structure which can contain all the things the borg thinks of itself
int* borg_has;
int* borg_trait;
int* borg_activation;

int borg_race; /* Player race */
int borg_class; /* Player class */

int32_t my_power;

/* 
 * temporary status 
 */
bool borg_lunal_mode;
bool borg_munchkin_mode;
bool borg_scumming_pots = true; /* Borg will quickly store pots in home */
int16_t borg_need_see_invis = 0;
int16_t borg_see_inv        = 0;
bool    need_shift_panel = false; /* to spot offscreens */
int16_t when_shift_panel = 0L;
int16_t time_this_panel = 0L; /* Current "time" on current panel*/
int16_t borg_no_retreat = 0;

int16_t when_call_light; /* When we last did call light */
int16_t when_wizard_light; /* When we last did wizard light */
int16_t when_detect_traps; /* When we last detected traps */
int16_t when_detect_doors; /* When we last detected doors */
int16_t when_detect_walls; /* When we last detected walls */
int16_t when_detect_evil; /* When we last detected monsters or evil */
int16_t when_detect_obj; /* When we last detected objects */
int16_t when_last_kill_mult = 0; /* When a multiplier was last killed */

int c_x; /* Current location (X) */
int c_y; /* Current location (Y) */

/*
 * Goal variables
 */
int16_t goal; /* Goal type */

int g_x; /* Goal location (X) */
int g_y; /* Goal location (Y) */

bool    goal_rising; /* Currently returning to town */
bool    goal_leaving; /* Currently leaving the level */
bool    goal_fleeing; /* Currently fleeing the level */
bool    goal_fleeing_lunal; /* Fleeing level while in lunal Mode */
bool    goal_fleeing_munchkin; /* Fleeing level while in munchkin Mode */
bool    goal_fleeing_to_town; /* Currently fleeing the level to return to town */
bool    goal_ignoring; /* Currently ignoring monsters */
int     goal_recalling; /* Currently waiting for recall, guessing the turns left */
bool    goal_less; /* return to, but dont use, the next up stairs */

int16_t     goal_shop = -1; /* Next shop to visit */
int16_t     goal_ware = -1; /* Next item to buy there */
int16_t     goal_item = -1; /* Next item to sell there */

bool    stair_less; /* Use the next "up" staircase */
bool    stair_more; /* Use the next "down" staircase */

int16_t borg_times_twitch; /* how often twitchy on this level */
int16_t borg_escapes; /* how often teleported on this level */

bool    borg_simulate; /* Simulation flag */
bool    borg_attacking; /* Simulation flag */

bool    borg_on_upstairs; /* used when leaving a level */
bool    borg_on_dnstairs; /* used when leaving a level */

/*
 * Notice failure
 */
bool    borg_failure;

int16_t borg_oldchp; /* hit points last game turn */

/* defence flags */
bool    borg_prot_from_evil;
bool    borg_speed;
bool    borg_bless;
bool    borg_hero;
bool    borg_berserk;
bool    borg_fastcast;
bool    borg_regen;
bool    borg_smite_evil;
bool    borg_venom;
bool    borg_shield;
bool    borg_on_glyph; /* borg is standing on a glyph of warding */
bool    borg_create_door; /* borg is going to create doors */
bool    borg_sleep_spell;
bool    borg_sleep_spell_ii;
bool    borg_crush_spell;
bool    borg_slow_spell; /* borg is about to cast the spell */
bool    borg_confuse_spell;
bool    borg_fear_mon_spell;

int16_t borg_game_ratio; /* the ratio of borg time to game time */
int16_t borg_resistance; /* borg is Resistant to all elements */
int16_t borg_no_rest_prep; /* borg wont rest for a few turns */

bool    borg_in_shop = false; /* True if the borg is inside of a store */

/* 
 * These values should likely be in borg_trait
 */
int16_t my_stat_max[STAT_MAX]; /* Current "maximal" stat values */
int16_t my_stat_cur[STAT_MAX]; /* Current "natural" stat values */
int16_t my_stat_ind[STAT_MAX]; /* Current "additions" to stat values */

int16_t my_stat_add[STAT_MAX]; /* additions to stats  This will allow upgrading of */
                               /* equiptment to allow a ring of int +4 to be traded */
                               /* for a ring of int +6 even if maximized to allow a */
                               /* later swap to be better. */

int16_t my_need_enchant_to_a; /* Need some enchantment */
int16_t my_need_enchant_to_h; /* Need some enchantment */
int16_t my_need_enchant_to_d; /* Need some enchantment */
int16_t my_need_brand_weapon; /*  actually brand bolts */
int16_t my_need_id; /* need to buy ID for an inventory item */

int16_t amt_food_hical;
int16_t amt_food_lowcal;

int16_t amt_slow_poison;
int16_t amt_cure_confusion;
int16_t amt_cure_blind;

int16_t amt_book[9];

int     borg_stat[6]; /* !FIX how does this differ from my_stat_cur */
int     borg_book[9]; /* !FIX how does this differ from amt_book? */

int16_t amt_add_stat[6];
int16_t amt_inc_stat[6]; /* Stat potions */
int16_t amt_fix_exp;

int16_t amt_cool_staff; /* holiness - power staff */
int16_t amt_cool_wand; /* # of charges on Wands which can be useful for attacks */
int16_t amt_digger;

/* a 3 state boolean */
/*-1 = not checked yet */
/* 0 = not ready */
/* 1 = ready */
int borg_ready_morgoth;

/* NOTE: This must exactly match the enum in borg-trait.h */
const char *prefix_pref[] = {
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
    "_ADJHP",
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
    "_GOLD",
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
    "_DEPTH", /* current depth being tested */
    "_CDEPTH", /* borgs current depth */
    "_MAXDEPTH", /* recall depth */
    "_KING", /* borg has won */

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
    "_TOHIT", /* base to hit, does not include weapon */
    "_TODAM", /* base to damage, does not include weapon */
    "_WTOHIT", /* weapon to hit */
    "_WTODAM", /* weapon to damage */
    "_BTOHIT", /* bow to hit */
    "_BTODAM", /* bow to damage */
    "_BLOWS",
    "_SHOTS",
    "_WMAXDAM", /* max damage per round with weapon (normal blow) */
    /* Assumes you can enchant to +8 if you are level 25+ */
    "_WBASEDAM", /* max damage per round with weapon (normal blow) */
    /* Assumes you have no enchantment */
    "_BMAXDAM", /* max damage per round with bow (normal hit) */
    /* Assumes you can enchant to +8 if you are level 25+ */
    "_HEAVYWEPON",
    "_HEAVYBOW",
    "_AMMO_COUNT", /* count of all ammo */
    "_AMMO_TVAL",
    "_AMMO_SIDES",
    "_AMMO_POWER",
    "_AMISSILES", /* only ones for your current bow count */
    "_AMISSILES_SPECIAL", /* and are ego */
    "_AMISSILES_CURSED", /* and are cursed */
    "_QUIVER_SLOTS", /* number of inven slots the quivered items take */
    "_FIRST_CURSED", /* first cursed item */
    "_WHERE_CURSED", /* where curses are 1 inv, 2 equ, 4 quiv */

    /* curses */
    "_CRSENVELOPING",
    "_CRSIRRITATION",
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
    "_CRSAGRV",
    "_CRSHPIMP", /* Impaired HP recovery */
    "_CRSMPIMP", /* Impaired MP recovery */
    "_CRSSTEELSKIN",
    "_CRSAIRSWING",
    "_CRSFEAR", /* Fear curse flag */
    "_CRSDRAIN_XP", /* drain XP flag */
    "_CRSFVULN", /* Vulnerable to fire */
    "_CRSEVULN", /* Vulnerable to elec */
    "_CRSCVULN", /* Vulnerable to Cold */
    "_CRSAVULN", /* Vulnerable to Acid */
    "_CRSUNKNO",

    /* weapon attributes */
    "_WSANIMAL", /* WS = weapon slays */
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
    "_WBACID", /* WB = Weapon Branded With */
    "_WBELEC",
    "_WBFIRE",
    "_WBCOLD",
    "_WBPOIS",

    /* amounts */
    "_APHASE",
    "_ATELEPORT", /* all sources of teleport */
    "_AESCAPE", /* Staff, artifact (can be used when blind/conf) */
    "_FUEL",
    "_HEAL",
    "_EZHEAL",
    "_LIFE",
    "_ID",
    "_ASPEED",
    "_ASTFMAGI", /* Amount Staff Charges */
    "_ASTFDEST",
    "_ATPORTOTHER", /* How many Teleport Other charges you got? */
    "_ACUREPOIS",
    "_ADETTRAP",
    "_ADETDOOR",
    "_ADETEVIL",
    "_AMAGICMAP",
    "_ALITE",
    "_ARECHARGE",
    "_APFE", /* Protection from Evil */
    "_AGLYPH", /* Rune Protection */
    "_ACCW", /* CCW potions (just because we use it so often) */
    "_ACSW", /* CSW potions (+ CLW if cut) */
    "_ACLW",
    "_AENCH_TOH", /* enchant weapons and armor (+spells) */
    "_AENCH_TOD",
    "_AENCH_SWEP",
    "_AENCH_ARM",
    "_AENCH_SARM",
    "_ABRAND",
    "_ARESHEAT", /* potions of res heat */
    "_ARESCOLD", /* pot of res cold */
    "_ARESPOIS", /* Potions of Res Poison */
    "_ATELEPORTLVL", /* scroll of teleport level */
    "_AHWORD", /* Holy Word prayer Legal*/
    "_AMASSBAN", /* ?Mass Banishment */
    "_ASHROOM", /* Number of cool mushrooms */
    "_AROD1", /* Attack rods */
    "_AROD2", /* Attack rods */
    "_ANEED_ID", /* a wielded item that needs ID */
    "_MULTI_BONUSES", /* Items with multiple useful bonuses */
    "_DINV", /* See Inv Spell is Legal */
    "_WEIGHT", /* weight of all inventory and equipment */
    "_CARRY", /* carry capacity based on str */
    "_EMPTY", /* number of empty slots */
    NULL
};

/**
 * Calculate the blows a player would get.
 *
 * copied and ajusted from player-calcs.c
 */
int borg_calc_blows(borg_item * item)
{
    int blows;
    int str_index, dex_index;
    int div;
    int blow_energy;

    int weight = item->weight;
    int min_weight = player->class->min_weight;

    /* Enforce a minimum "weight" (tenth pounds) */
    div = (weight < min_weight) ? min_weight : weight;

    /* Get the strength vs weight */
    str_index = adj_str_blow[my_stat_ind[STAT_STR]]
        * player->class->att_multiply / div;

    /* Maximal value */
    if (str_index > 11)
        str_index = 11;

    /* Index by dexterity */
    dex_index = MIN(borg_adj_dex_blow[my_stat_ind[STAT_DEX]], 11);

    /* Use the blows table to get energy per blow */
    blow_energy = borg_blows_table[str_index][dex_index];

    blows = MIN((10000 / blow_energy), (100 * player->class->max_attacks));

    /* Require at least one blow, two for O-combat */
    return (MAX(blows + (100 * item->modifiers[OBJ_MOD_BLOWS]),
        OPT(player, birth_percent_damage) ? 200 : 100))
        / 100;
}

/*
 * Note that we assume that any item with quantity zero does not exist,
 * thus, when simulating possible worlds, we do not actually have to
 * "optimize" empty slots.
 *
 */

 /*
  * The "notice" functions examine various aspects of the player inventory,
  * the player equipment, or the home contents, and extract various numerical
  * quantities based on those aspects, adjusting them for various "abilities",
  * such as the ability to cast certain spells, etc.
  *
  * The "power" functions use the numerical quantities described above, and
  * use them to do two different things:  (1) rank the "value" of having
  * various abilities relative to the possible "money" reward of carrying
  * sellable items instead, and (2) rank the value of various abilities
  * relative to each other, which is used to determine what to wear/buy,
  * and in what order to wear/buy those items.
  *
  * These functions use some very heuristic values, by the way...
  *
  * We should probably take account of things like possible enchanting
  * (especially when in town), and items which may be found soon.
  *
  * We consider several things:
  *   (1) the actual "power" of the current weapon and bow
  *   (2) the various "flags" imparted by the equipment
  *   (3) the various abilities imparted by the equipment
  *   (4) the penalties induced by heavy armor or gloves or edged weapons
  *   (5) the abilities required to enter the "max_depth" dungeon level
  *   (6) the various abilities of some useful inventory items
  *
  * Note the use of special "item counters" for evaluating the value of
  * a collection of items of the given type.  Basically, the first item
  * of the given type is always the most valuable, with subsequent items
  * being worth less, until the "limit" is reached, after which point any
  * extra items are only worth as much as they can be sold for.
  */

  /*
   * Helper function -- notice one slot of ammo
   */
static void borg_notice_ammo(int slot)
{
    const borg_item *item = &borg_items[slot];

    /* Skip empty items */
    if (!item->iqty)
        return;

    /* total up the weight of the items */
    borg_trait[BI_WEIGHT] += item->weight * item->iqty;

    /* Count all ammo */
    borg_trait[BI_AMMO_COUNT] += item->iqty;

    if (item->tval != borg_trait[BI_AMMO_TVAL])
        return;

    /* Count missiles that fit your bow */
    borg_trait[BI_AMISSILES] += item->iqty;

    /* track first cursed item */
    if (item->uncursable) {
        borg_trait[BI_WHERE_CURSED] |= BORG_QUILL;
        if (!borg_trait[BI_FIRST_CURSED])
            borg_trait[BI_FIRST_CURSED] = slot + 1;

        borg_trait[BI_AMISSILES_CURSED] += item->iqty;
        return;
    }

    if (item->ego_idx)
        borg_trait[BI_AMISSILES_SPECIAL] += item->iqty;

    /* check for ammo to enchant */

    /* Hack -- ignore worthless missiles */
    if (item->value <= 0)
        return;

    /* Only enchant ammo if we have a good shooter,
     * otherwise, store the enchants in the home.
     */
    if (borg_trait[BI_AMMO_POWER] >= 3) {

        if ((borg_equips_item(act_firebrand, false)
            || borg_spell_legal_fail(BRAND_AMMUNITION, 65))
            && item->iqty >= 5 &&
            /* Skip artifacts and ego-items */
            !item->ego_idx && !item->art_idx && item->ident
            && item->tval == borg_trait[BI_AMMO_TVAL]) {
            my_need_brand_weapon += 10L;
        }

        /* if we have loads of cash (as we will at level 35),  */
        /* enchant missiles */
        if (borg_trait[BI_CLEVEL] > 35) {
            if (borg_spell_legal_fail(ENCHANT_WEAPON, 65) && item->iqty >= 5) {
                if (item->to_h < 10) {
                    my_need_enchant_to_h += (10 - item->to_h);
                }

                if (item->to_d < 10) {
                    my_need_enchant_to_d += (10 - item->to_d);
                }
            } else {
                if (item->to_h < 8) {
                    my_need_enchant_to_h += (8 - item->to_h);
                }

                if (item->to_d < 8) {
                    my_need_enchant_to_d += (8 - item->to_d);
                }
            }
        }
    } /* Ammo Power > 3 */

    /* Only enchant ammo if we have a good shooter,
     * otherwise, store the enchants in the home.
     */
    if (borg_trait[BI_AMMO_POWER] < 3)
        return;

    if ((borg_equips_item(act_firebrand, false)
        || borg_spell_legal_fail(BRAND_AMMUNITION, 65))
        && item->iqty >= 5 &&
        /* Skip artifacts and ego-items */
        !item->art_idx && !item->ego_idx && item->ident
        && item->tval == borg_trait[BI_AMMO_TVAL]) {
        my_need_brand_weapon += 10L;
    }
}

/*
 * Helper function -- notice the player equipment
 */
static void borg_notice_equipment(void)
{
    int                        i, hold;
    const struct player_race *rb_ptr = player->race;
    const struct player_class *cb_ptr = player->class;

    int                        extra_blows = 0;

    int                        extra_shots = 0;
    int                        extra_might = 0;
    int                        my_num_fire;

    bitflag                    f[OF_SIZE];

    borg_item *item;

    /* Recalc some Variables */
    borg_trait[BI_ARMOR] = 0;
    borg_trait[BI_SPEED] = 110;
    borg_trait[BI_WEIGHT] = 0;

    /* Start with a single blow per turn */
    borg_trait[BI_BLOWS] = 1;

    /* Start with a single shot per turn */
    my_num_fire = 1;

    /* Reset the "ammo" attributes */
    borg_trait[BI_AMMO_COUNT] = 0;
    borg_trait[BI_AMMO_TVAL] = -1;
    borg_trait[BI_AMMO_SIDES] = 4;
    borg_trait[BI_AMMO_POWER] = 0;

    /* Reset the count of ID needed immediately */
    my_need_id = 0;

    /* Base infravision (purely racial) */
    borg_trait[BI_INFRA] = rb_ptr->infra;

    /* Base skill -- disarming */
    borg_trait[BI_DISP] = rb_ptr->r_skills[SKILL_DISARM_PHYS]
        + cb_ptr->c_skills[SKILL_DISARM_PHYS];
    borg_trait[BI_DISM] = rb_ptr->r_skills[SKILL_DISARM_MAGIC]
        + cb_ptr->c_skills[SKILL_DISARM_MAGIC];

    /* Base skill -- magic devices */
    borg_trait[BI_DEV]
        = rb_ptr->r_skills[SKILL_DEVICE] + cb_ptr->c_skills[SKILL_DEVICE];

    /* Base skill -- saving throw */
    borg_trait[BI_SAV]
        = rb_ptr->r_skills[SKILL_SAVE] + cb_ptr->c_skills[SKILL_SAVE];

    /* Base skill -- stealth */
    borg_trait[BI_STL]
        = rb_ptr->r_skills[SKILL_STEALTH] + cb_ptr->c_skills[SKILL_STEALTH];

    /* Base skill -- searching ability */
    borg_trait[BI_SRCH]
        = rb_ptr->r_skills[SKILL_SEARCH] + cb_ptr->c_skills[SKILL_SEARCH];

    /* Base skill -- combat (normal) */
    borg_trait[BI_THN] = rb_ptr->r_skills[SKILL_TO_HIT_MELEE]
        + cb_ptr->c_skills[SKILL_TO_HIT_MELEE];

    /* Base skill -- combat (shooting) */
    borg_trait[BI_THB] = rb_ptr->r_skills[SKILL_TO_HIT_BOW]
        + cb_ptr->c_skills[SKILL_TO_HIT_BOW];

    /* Base skill -- combat (throwing) */
    borg_trait[BI_THT] = rb_ptr->r_skills[SKILL_TO_HIT_THROW]
        + cb_ptr->c_skills[SKILL_TO_HIT_THROW];

    /* Affect Skill -- digging (STR) */
    borg_trait[BI_DIG]
        = rb_ptr->r_skills[SKILL_DIGGING] + cb_ptr->c_skills[SKILL_DIGGING];

    /** Racial Skills **/

    /* Extract the player flags */
    player_flags(player, f);

    /* Good flags */
    if (of_has(f, OF_SLOW_DIGEST))
        borg_trait[BI_SDIG] = true;
    if (of_has(f, OF_FEATHER))
        borg_trait[BI_FEATH] = true;
    if (of_has(f, OF_REGEN))
        borg_trait[BI_REG] = true;
    if (of_has(f, OF_TELEPATHY))
        borg_trait[BI_ESP] = true;
    if (of_has(f, OF_SEE_INVIS))
        borg_trait[BI_SINV] = true;
    if (of_has(f, OF_FREE_ACT))
        borg_trait[BI_FRACT] = true;
    if (of_has(f, OF_HOLD_LIFE))
        borg_trait[BI_HLIFE] = true;

    /* Weird flags */

    /* Bad flags */
    if (of_has(f, OF_IMPACT))
        borg_trait[BI_W_IMPACT] = true;
    if (of_has(f, OF_AGGRAVATE))
        borg_trait[BI_CRSAGRV] = true;
    if (of_has(f, OF_AFRAID))
        borg_trait[BI_CRSFEAR] = true;
    if (of_has(f, OF_DRAIN_EXP))
        borg_trait[BI_CRSDRAIN_XP] = true;

    if (rb_ptr->el_info[ELEM_FIRE].res_level == -1)
        borg_trait[BI_CRSFVULN] = true;
    if (rb_ptr->el_info[ELEM_ACID].res_level == -1)
        borg_trait[BI_CRSAVULN] = true;
    if (rb_ptr->el_info[ELEM_COLD].res_level == -1)
        borg_trait[BI_CRSCVULN] = true;
    if (rb_ptr->el_info[ELEM_ELEC].res_level == -1)
        borg_trait[BI_CRSEVULN] = true;

    /* Immunity flags */
    if (rb_ptr->el_info[ELEM_FIRE].res_level == 3)
        borg_trait[BI_IFIRE] = true;
    if (rb_ptr->el_info[ELEM_ACID].res_level == 3)
        borg_trait[BI_IACID] = true;
    if (rb_ptr->el_info[ELEM_COLD].res_level == 3)
        borg_trait[BI_ICOLD] = true;
    if (rb_ptr->el_info[ELEM_ELEC].res_level == 3)
        borg_trait[BI_IELEC] = true;

    /* Resistance flags */
    if (rb_ptr->el_info[ELEM_FIRE].res_level > 0)
        borg_trait[BI_RACID] = true;
    if (rb_ptr->el_info[ELEM_ELEC].res_level > 0)
        borg_trait[BI_RELEC] = true;
    if (rb_ptr->el_info[ELEM_FIRE].res_level > 0)
        borg_trait[BI_RFIRE] = true;
    if (rb_ptr->el_info[ELEM_COLD].res_level > 0)
        borg_trait[BI_RCOLD] = true;
    if (rb_ptr->el_info[ELEM_POIS].res_level > 0)
        borg_trait[BI_RPOIS] = true;
    if (rb_ptr->el_info[ELEM_LIGHT].res_level > 0)
        borg_trait[BI_RLITE] = true;
    if (rb_ptr->el_info[ELEM_DARK].res_level > 0)
        borg_trait[BI_RDARK] = true;
    if (rb_ptr->el_info[ELEM_SOUND].res_level > 0)
        borg_trait[BI_RSND] = true;
    if (rb_ptr->el_info[ELEM_SHARD].res_level > 0)
        borg_trait[BI_RSHRD] = true;
    if (rb_ptr->el_info[ELEM_NEXUS].res_level > 0)
        borg_trait[BI_RNXUS] = true;
    if (rb_ptr->el_info[ELEM_NETHER].res_level > 0)
        borg_trait[BI_RNTHR] = true;
    if (rb_ptr->el_info[ELEM_CHAOS].res_level > 0)
        borg_trait[BI_RKAOS] = true;
    if (rb_ptr->el_info[ELEM_DISEN].res_level > 0)
        borg_trait[BI_RDIS] = true;
    if (rf_has(f, OF_PROT_FEAR))
        borg_trait[BI_RFEAR] = true;
    if (rf_has(f, OF_PROT_BLIND))
        borg_trait[BI_RBLIND] = true;
    if (rf_has(f, OF_PROT_CONF))
        borg_trait[BI_RCONF] = true;

    /* Sustain flags */
    if (rf_has(f, OF_SUST_STR))
        borg_trait[BI_SSTR] = true;
    if (rf_has(f, OF_SUST_INT))
        borg_trait[BI_SINT] = true;
    if (rf_has(f, OF_SUST_WIS))
        borg_trait[BI_SWIS] = true;
    if (rf_has(f, OF_SUST_DEX))
        borg_trait[BI_SDEX] = true;
    if (rf_has(f, OF_SUST_CON))
        borg_trait[BI_SCON] = true;

    /* I am pretty sure the CF_flags will be caught by the
     * code above when the player flags are checked
     */

     /* Clear the stat modifiers */
    for (i = 0; i < STAT_MAX; i++)
        my_stat_add[i] = 0;

    /* track activations */
    /* note this is done first so that it we can use this */
    /* array in borg_equips_item */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
        if (borg_items[i].activ_idx) {
            borg_activation[borg_items[i].activ_idx] += 1;
        }
    }

    /* Scan the usable inventory */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
        item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* total up the weight of the items */
        borg_trait[BI_WEIGHT] += item->weight * item->iqty;

        if (borg_item_note_needs_id(item)) {
            my_need_id++;
            borg_trait[BI_ANEED_ID] += 1;
        }

        /* track first cursed item */
        if (!borg_trait[BI_FIRST_CURSED] && item->uncursable) {
            borg_trait[BI_WHERE_CURSED] |= BORG_EQUIP;
            borg_trait[BI_FIRST_CURSED] = i + 1;
        }

        /* Affect stats */
        my_stat_add[STAT_STR] += item->modifiers[OBJ_MOD_STR]
            * player->obj_k->modifiers[OBJ_MOD_STR];
        my_stat_add[STAT_INT] += item->modifiers[OBJ_MOD_INT]
            * player->obj_k->modifiers[OBJ_MOD_INT];
        my_stat_add[STAT_WIS] += item->modifiers[OBJ_MOD_WIS]
            * player->obj_k->modifiers[OBJ_MOD_WIS];
        my_stat_add[STAT_DEX] += item->modifiers[OBJ_MOD_DEX]
            * player->obj_k->modifiers[OBJ_MOD_DEX];
        my_stat_add[STAT_CON] += item->modifiers[OBJ_MOD_CON]
            * player->obj_k->modifiers[OBJ_MOD_CON];

        /* various slays */
        borg_trait[BI_WS_ANIMAL] = item->slays[RF_ANIMAL];
        borg_trait[BI_WS_EVIL] = item->slays[RF_EVIL];
        borg_trait[BI_WS_UNDEAD] = item->slays[RF_UNDEAD];
        borg_trait[BI_WS_DEMON] = item->slays[RF_DEMON];
        borg_trait[BI_WS_ORC] = item->slays[RF_ORC];
        borg_trait[BI_WS_TROLL] = item->slays[RF_TROLL];
        borg_trait[BI_WS_GIANT] = item->slays[RF_GIANT];
        borg_trait[BI_WS_DRAGON] = item->slays[RF_DRAGON];

        /* various brands */
        if (item->brands[ELEM_ACID])
            borg_trait[BI_WB_ACID] = true;
        if (item->brands[ELEM_ELEC])
            borg_trait[BI_WB_ELEC] = true;
        if (item->brands[ELEM_FIRE])
            borg_trait[BI_WB_FIRE] = true;
        if (item->brands[ELEM_COLD])
            borg_trait[BI_WB_COLD] = true;
        if (item->brands[ELEM_POIS])
            borg_trait[BI_WB_POIS] = true;
        if (of_has(item->flags, OF_IMPACT))
            borg_trait[BI_W_IMPACT] = true;

        /* Affect infravision */
        borg_trait[BI_INFRA] += item->modifiers[OBJ_MOD_INFRA];

        /* Affect stealth */
        borg_trait[BI_STL] += item->modifiers[OBJ_MOD_STEALTH];

        /* Affect searching ability (factor of five) */
        borg_trait[BI_SRCH] += (item->modifiers[OBJ_MOD_SEARCH] * 5);

        /* weapons of digging type get a special bonus */
        int dig = 0;
        if (item->tval == TV_DIGGING) {
            if (of_has(item->flags, OF_DIG_1))
                dig = 1;
            else if (of_has(item->flags, OF_DIG_2))
                dig = 2;
            else if (of_has(item->flags, OF_DIG_3))
                dig = 3;
        }
        dig += item->modifiers[OBJ_MOD_TUNNEL];

        /* Affect digging (factor of 20) */
        borg_trait[BI_DIG] += (dig * 20);

        /* Affect speed */
        borg_trait[BI_SPEED] += item->modifiers[OBJ_MOD_SPEED];

        /* Affect blows */
        extra_blows += item->modifiers[OBJ_MOD_BLOWS];

        /* Boost shots */
        extra_shots += item->modifiers[OBJ_MOD_SHOTS];

        /* Boost might */
        extra_might += item->modifiers[OBJ_MOD_MIGHT];

        /* Item makes player glow or has a light radius  */
        if (item->modifiers[OBJ_MOD_LIGHT]) {
            /* Special case for Torches/Lantern of Brightness, they are not
             * perm. */
            if (item->tval == TV_LIGHT
                && (item->sval == sv_light_torch
                    || item->sval == sv_light_lantern)
                && !item->timeout)
                borg_trait[BI_LIGHT]++;
        }

        /* Boost mod moves */
        borg_trait[BI_MOD_MOVES] += item->modifiers[OBJ_MOD_MOVES];

        /* Boost damage reduction */
        borg_trait[BI_DAM_RED] += item->modifiers[OBJ_MOD_DAM_RED];

        /* Various flags */
        if (of_has(item->flags, OF_SLOW_DIGEST))
            borg_trait[BI_SDIG] = true;
        if (of_has(item->flags, OF_AGGRAVATE))
            borg_trait[BI_CRSAGRV] = true;
        if (of_has(item->flags, OF_IMPAIR_HP))
            borg_trait[BI_CRSHPIMP] = true;
        if (of_has(item->flags, OF_IMPAIR_MANA))
            borg_trait[BI_CRSMPIMP] = true;
        if (of_has(item->flags, OF_AFRAID))
            borg_trait[BI_CRSFEAR] = true;
        if (of_has(item->flags, OF_DRAIN_EXP))
            borg_trait[BI_CRSDRAIN_XP] = true;

        /* curses that don't have flags or stat changes that are tracked
         * elsewhere */
        if (item->curses[BORG_CURSE_VULNERABILITY]) {
            borg_trait[BI_CRSAGRV] = true;
            borg_trait[BI_ARMOR] -= 50;
        }
        if (item->curses[BORG_CURSE_TELEPORTATION])
            borg_trait[BI_CRSTELE] = true;
        if (item->curses[BORG_CURSE_DULLNESS]) {
            borg_trait[BI_CINT] -= 5;
            borg_trait[BI_CWIS] -= 5;
        }
        if (item->curses[BORG_CURSE_SICKLINESS]) {
            borg_trait[BI_CSTR] -= 5;
            borg_trait[BI_CDEX] -= 5;
            borg_trait[BI_CCON] -= 5;
        }
        if (item->curses[BORG_CURSE_ENVELOPING])
            borg_trait[BI_CRSENVELOPING] = true;
        if (item->curses[BORG_CURSE_IRRITATION]) {
            borg_trait[BI_CRSAGRV] = true;
            borg_trait[BI_CRSIRRITATION] = true;
        }
        if (item->curses[BORG_CURSE_WEAKNESS]) {
            borg_trait[BI_CSTR] -= 10;
        }
        if (item->curses[BORG_CURSE_CLUMSINESS]) {
            borg_trait[BI_CSTR] -= 10;
        }
        if (item->curses[BORG_CURSE_SLOWNESS]) {
            borg_trait[BI_SPEED] = -5;
        }
        if (item->curses[BORG_CURSE_ANNOYANCE]) {
            borg_trait[BI_SPEED] = -10;
            borg_trait[BI_STL] = -10;
            borg_trait[BI_CRSAGRV] = true;
        }
        if (item->curses[BORG_CURSE_POISON])
            borg_trait[BI_CRSPOIS] = true;
        if (item->curses[BORG_CURSE_SIREN])
            borg_trait[BI_CRSSIREN] = true;
        if (item->curses[BORG_CURSE_HALLUCINATION])
            borg_trait[BI_CRSHALU] = true;
        if (item->curses[BORG_CURSE_PARALYSIS])
            borg_trait[BI_CRSPARA] = true;
        if (item->curses[BORG_CURSE_DEMON_SUMMON])
            borg_trait[BI_CRSSDEM] = true;
        if (item->curses[BORG_CURSE_DRAGON_SUMMON])
            borg_trait[BI_CRSSDRA] = true;
        if (item->curses[BORG_CURSE_UNDEAD_SUMMON])
            borg_trait[BI_CRSSUND] = true;
        if (item->curses[BORG_CURSE_IMPAIR_MANA_RECOVERY])
            borg_trait[BI_CRSMPIMP] = true;
        if (item->curses[BORG_CURSE_IMPAIR_HITPOINT_RECOVERY])
            borg_trait[BI_CRSHPIMP] = true;
        if (item->curses[BORG_CURSE_COWARDICE])
            borg_trait[BI_CRSFEAR] = true;
        if (item->curses[BORG_CURSE_STONE])
            borg_trait[BI_CRSSTONE] = true;
        if (item->curses[BORG_CURSE_ANTI_TELEPORTATION])
            borg_trait[BI_CRSNOTEL] = true;
        if (item->curses[BORG_CURSE_TREACHEROUS_WEAPON])
            borg_trait[BI_CRSTWEP] = true;
        if (item->curses[BORG_CURSE_BURNING_UP]) {
            borg_trait[BI_CRSFVULN] = true;
            borg_trait[BI_RCOLD] = true;
        }
        if (item->curses[BORG_CURSE_CHILLED_TO_THE_BONE]) {
            borg_trait[BI_CRSCVULN] = true;
            borg_trait[BI_RFIRE] = true;
        }
        if (item->curses[BORG_CURSE_STEELSKIN])
            borg_trait[BI_CRSSTEELSKIN] = true;
        if (item->curses[BORG_CURSE_AIR_SWING])
            borg_trait[BI_CRSAIRSWING] = true;
        if (item->curses[BORG_CURSE_UNKNOWN])
            borg_trait[BI_CRSUNKNO] = true;

        if (item->el_info[ELEM_FIRE].res_level == -1)
            borg_trait[BI_CRSFVULN] = true;
        if (item->el_info[ELEM_ACID].res_level == -1)
            borg_trait[BI_CRSAVULN] = true;
        if (item->el_info[ELEM_COLD].res_level == -1)
            borg_trait[BI_CRSCVULN] = true;
        if (item->el_info[ELEM_ELEC].res_level == -1)
            borg_trait[BI_CRSEVULN] = true;

        if (of_has(item->flags, OF_REGEN))
            borg_trait[BI_REG] = true;
        if (of_has(item->flags, OF_TELEPATHY))
            borg_trait[BI_ESP] = true;
        if (of_has(item->flags, OF_SEE_INVIS))
            borg_trait[BI_SINV] = true;
        if (of_has(item->flags, OF_FEATHER))
            borg_trait[BI_FEATH] = true;
        if (of_has(item->flags, OF_FREE_ACT))
            borg_trait[BI_FRACT] = true;
        if (of_has(item->flags, OF_HOLD_LIFE))
            borg_trait[BI_HLIFE] = true;
        if (of_has(item->flags, OF_PROT_CONF))
            borg_trait[BI_RCONF] = true;
        if (of_has(item->flags, OF_PROT_BLIND))
            borg_trait[BI_RBLIND] = true;

        /* assume all light artifacts give off light */
        if (item->tval == TV_LIGHT && item->art_idx)
            borg_trait[BI_LIGHT]++;

        /* Immunity flags */
        /* if you are immune you automaticly resist */
        if (item->el_info[ELEM_FIRE].res_level == 3) {
            borg_trait[BI_IFIRE] = true;
            borg_trait[BI_RFIRE] = true;
            borg_trait[BI_TRFIRE] = true;
        }
        if (item->el_info[ELEM_ACID].res_level == 3) {
            borg_trait[BI_IACID] = true;
            borg_trait[BI_RACID] = true;
            borg_trait[BI_TRACID] = true;
        }
        if (item->el_info[ELEM_COLD].res_level == 3) {
            borg_trait[BI_ICOLD] = true;
            borg_trait[BI_RCOLD] = true;
            borg_trait[BI_TRCOLD] = true;
        }
        if (item->el_info[ELEM_ELEC].res_level == 3) {
            borg_trait[BI_IELEC] = true;
            borg_trait[BI_RELEC] = true;
            borg_trait[BI_TRELEC] = true;
        }

        /* Resistance flags */
        if (item->el_info[ELEM_ACID].res_level > 0)
            borg_trait[BI_RACID] = true;
        if (item->el_info[ELEM_ELEC].res_level > 0)
            borg_trait[BI_RELEC] = true;
        if (item->el_info[ELEM_FIRE].res_level > 0)
            borg_trait[BI_RFIRE] = true;
        if (item->el_info[ELEM_COLD].res_level > 0)
            borg_trait[BI_RCOLD] = true;
        if (item->el_info[ELEM_POIS].res_level > 0)
            borg_trait[BI_RPOIS] = true;
        if (item->el_info[ELEM_SOUND].res_level > 0)
            borg_trait[BI_RSND] = true;
        if (item->el_info[ELEM_LIGHT].res_level > 0)
            borg_trait[BI_RLITE] = true;
        if (item->el_info[ELEM_DARK].res_level > 0)
            borg_trait[BI_RDARK] = true;
        if (item->el_info[ELEM_CHAOS].res_level > 0)
            borg_trait[BI_RKAOS] = true;
        if (item->el_info[ELEM_DISEN].res_level > 0)
            borg_trait[BI_RDIS] = true;
        if (item->el_info[ELEM_SHARD].res_level > 0)
            borg_trait[BI_RSHRD] = true;
        if (item->el_info[ELEM_NEXUS].res_level > 0)
            borg_trait[BI_RNXUS] = true;
        if (item->el_info[ELEM_NETHER].res_level > 0)
            borg_trait[BI_RNTHR] = true;

        /* Sustain flags */
        if (of_has(item->flags, OF_SUST_STR))
            borg_trait[BI_SSTR] = true;
        if (of_has(item->flags, OF_SUST_INT))
            borg_trait[BI_SINT] = true;
        if (of_has(item->flags, OF_SUST_WIS))
            borg_trait[BI_SWIS] = true;
        if (of_has(item->flags, OF_SUST_DEX))
            borg_trait[BI_SDEX] = true;
        if (of_has(item->flags, OF_SUST_CON))
            borg_trait[BI_SCON] = true;

        /* Good to have one item with multiple high resists */
        int bonuses = ((item->el_info[ELEM_POIS].res_level > 0)
            + (item->el_info[ELEM_SOUND].res_level > 0)
            + (item->el_info[ELEM_SHARD].res_level > 0)
            + (item->el_info[ELEM_NEXUS].res_level > 0)
            + (item->el_info[ELEM_NETHER].res_level > 0)
            + (item->el_info[ELEM_CHAOS].res_level > 0)
            + (item->el_info[ELEM_DISEN].res_level > 0) +
            /* resist base 4 */
            ((item->el_info[ELEM_FIRE].res_level > 0)
                && (item->el_info[ELEM_COLD].res_level > 0)
                && (item->el_info[ELEM_ELEC].res_level > 0)
                && (item->el_info[ELEM_ACID].res_level > 0))
            +
            /* sustains all stats  */
            (of_has(item->flags, OF_SUST_STR)
                && of_has(item->flags, OF_SUST_INT)
                && of_has(item->flags, OF_SUST_WIS)
                && of_has(item->flags, OF_SUST_DEX)
                && of_has(item->flags, OF_SUST_CON)));

        if (bonuses > 2)
            borg_trait[BI_MULTIPLE_BONUSES] += bonuses;

        /* Hack -- Net-zero The borg will miss read acid damaged items such as
         * Leather Gloves [2,-2] and falsely assume they help his power.
         * this hack rewrites the bonus to an extremely negative value
         * thus encouraging him to remove the non-helpful-non-harmful but
         * heavy-none-the-less item.
         */
        if ((!item->art_idx && !item->ego_idx) && item->ac >= 1
            && item->to_a + item->ac <= 0) {
            item->to_a = -20;
        }

        /* Modify the base armor class */
        borg_trait[BI_ARMOR] += item->ac;

        /* Apply the bonuses to armor class */
        borg_trait[BI_ARMOR] += item->to_a;

        /* Hack -- do not apply "weapon" bonuses */
        if (i == INVEN_WIELD)
            continue;

        /* Hack -- do not apply "bow" bonuses */
        if (i == INVEN_BOW)
            continue;

        /* Apply the bonuses to hit/damage */
        borg_trait[BI_TOHIT] += item->to_h;
        borg_trait[BI_TODAM] += item->to_d;
    }

    /* The borg needs to update his base stat points */
    for (i = 0; i < STAT_MAX; i++) {
        /* Cheat the exact number from the game.  This number is available to
         * the player on the extra term window.
         */
        my_stat_cur[i] = player->stat_cur[i];

        /* Max stat is the max that the cur stat ever is. */
        if (my_stat_cur[i] > my_stat_max[i])
            my_stat_max[i] = my_stat_cur[i];
    }

    /* Update "stats" */
    for (i = 0; i < STAT_MAX; i++) {
        int add, use, ind;

        add = my_stat_add[i];

        /* Modify the stats for race/class */
        add += (player->race->r_adj[i] + player->class->c_adj[i]);

        /* Extract the new "use_stat" value for the stat */
        use = modify_stat_value(my_stat_cur[i], add);

        /* Values: 3, ..., 17 */
        if (use <= 18)
            ind = (use - 3);

        /* Ranges: 18/00-18/09, ..., 18/210-18/219 */
        else if (use <= 18 + 219)
            ind = (15 + (use - 18) / 10);

        /* Range: 18/220+ */
        else
            ind = (37);

        /* Save the index */
        if (ind > 37)
            my_stat_ind[i] = 37;
        else
            my_stat_ind[i] = ind;
        borg_trait[BI_STR + i] = my_stat_ind[i];
        borg_trait[BI_CSTR + i] = borg_stat[i];
    }

    borg_trait[BI_HP_ADJ] = player->player_hp[player->lev - 1]
        + borg_adj_con_mhp[my_stat_ind[STAT_CON]] * borg_trait[BI_CLEVEL]
        / 100;


    /* 'Mana' is actually the 'mana adjustment' */
    int spell_stat = borg_spell_stat();
    if (spell_stat >= 0) {
        borg_trait[BI_SP_ADJ]
            = ((borg_adj_mag_mana[my_stat_ind[spell_stat]]
                * (borg_trait[BI_CLEVEL] - player->class->magic.spell_first
                    + 1))
                / 2);
        borg_trait[BI_FAIL1] = borg_adj_mag_stat[my_stat_ind[spell_stat]];
        borg_trait[BI_FAIL2] = borg_adj_mag_fail[my_stat_ind[spell_stat]];
    }

    /* Bloating slows the player down (a little) */
    if (borg_trait[BI_ISGORGED])
        borg_trait[BI_SPEED] -= 10;

    /* Actual Modifier Bonuses */
    borg_trait[BI_ARMOR] += borg_adj_dex_ta[my_stat_ind[STAT_DEX]];
    borg_trait[BI_TODAM] += borg_adj_str_td[my_stat_ind[STAT_STR]];
    borg_trait[BI_TOHIT] += borg_adj_dex_th[my_stat_ind[STAT_DEX]];
    borg_trait[BI_TOHIT] += borg_adj_str_th[my_stat_ind[STAT_STR]];

    /* Obtain the "hold" value */
    hold = adj_str_hold[my_stat_ind[STAT_STR]];

    /* digging */
    borg_trait[BI_DIG] += borg_adj_str_dig[my_stat_ind[STAT_STR]];

    /** Examine the "current bow" **/
    item = &borg_items[INVEN_BOW];

    /* attacking with bare hands */
    if (item->iqty == 0) {
        item->ds = 0;
        item->dd = 0;
        item->to_d = 0;
        item->to_h = 0;
        item->weight = 0;
    }

    /* Real bonuses */
    borg_trait[BI_BTOHIT] = item->to_h;
    borg_trait[BI_BTODAM] = item->to_d;

    /* It is hard to carholdry a heavy bow */
    if (hold < item->weight / 10) {
        borg_trait[BI_HEAVYBOW] = true;
        /* Hard to wield a heavy bow */
        borg_trait[BI_TOHIT] += 2 * (hold - item->weight / 10);
    }

    /* Compute "extra shots" if needed */
    if (item->iqty && (hold >= item->weight / 10)) {
        /* Take note of required "tval" for missiles */
        if (item->sval == sv_sling) {
            borg_trait[BI_AMMO_TVAL] = TV_SHOT;
            borg_trait[BI_AMMO_SIDES] = 3;
            borg_trait[BI_AMMO_POWER] = 2;
        } else if (item->sval == sv_short_bow) {
            borg_trait[BI_AMMO_TVAL] = TV_ARROW;
            borg_trait[BI_AMMO_SIDES] = 4;
            borg_trait[BI_AMMO_POWER] = 2;
        } else if (item->sval == sv_long_bow) {
            borg_trait[BI_AMMO_TVAL] = TV_ARROW;
            borg_trait[BI_AMMO_SIDES] = 4;
            borg_trait[BI_AMMO_POWER] = 3;
        } else if (item->sval == sv_light_xbow) {
            borg_trait[BI_AMMO_TVAL] = TV_BOLT;
            borg_trait[BI_AMMO_SIDES] = 5;
            borg_trait[BI_AMMO_POWER] = 3;
        } else if (item->sval == sv_heavy_xbow) {
            borg_trait[BI_AMMO_TVAL] = TV_BOLT;
            borg_trait[BI_AMMO_SIDES] = 5;
            borg_trait[BI_AMMO_POWER] = 4;
        }

        /* Add in extra power */
        borg_trait[BI_AMMO_POWER] += extra_might;

        /* Hack -- Reward High Level Rangers using Bows */
        if (player_has(player, PF_FAST_SHOT)
            && (borg_trait[BI_AMMO_TVAL] == TV_ARROW)) {
            /* Extra shot at level 20 */
            if (borg_trait[BI_CLEVEL] >= 20)
                my_num_fire++;

            /* Extra shot at level 40 */
            if (borg_trait[BI_CLEVEL] >= 40)
                my_num_fire++;
        }

        /* Add in the "bonus shots" */
        my_num_fire += extra_shots;

        /* Require at least one shot */
        if (my_num_fire < 1)
            my_num_fire = 1;
    }
    borg_trait[BI_SHOTS] = my_num_fire;

    /* Calculate "average" damage per "normal" shot (times 2) */
    borg_trait[BI_BMAXDAM] = (borg_trait[BI_AMMO_SIDES] + borg_trait[BI_BTODAM])
        * borg_trait[BI_AMMO_POWER];
    borg_trait[BI_BMAXDAM] *= borg_trait[BI_SHOTS];

    /* Examine the "main weapon" */
    item = &borg_items[INVEN_WIELD];

    /* attacking with bare hands */
    if (item->iqty == 0) {
        item->ds = 0;
        item->dd = 0;
        item->to_d = 0;
        item->to_h = 0;
        item->weight = 0;
    }

    /* Real values */
    borg_trait[BI_WTOHIT] = item->to_h;
    borg_trait[BI_WTODAM] = item->to_d;

    /* It is hard to hold a heavy weapon */
    if (hold < item->weight / 10) {
        borg_trait[BI_HEAVYWEPON] = true;

        /* Hard to wield a heavy weapon */
        borg_trait[BI_TOHIT] += 2 * (hold - item->weight / 10);
    }

    /* Normal weapons */
    if (item->iqty && (hold >= item->weight / 10)) {
        /* calculate the number of blows */
        borg_trait[BI_BLOWS] = borg_calc_blows(item);

        /* Boost digging skill by weapon weight */
        borg_trait[BI_DIG] += (item->weight / 10);
    }

    /* Calculate "max" damage per "normal" blow  */
    /* and assume we can enchant up to +8 if borg_trait[BI_CLEVEL] > 25 */
    borg_trait[BI_WMAXDAM]
        = (item->dd * item->ds + borg_trait[BI_TODAM] + borg_trait[BI_WTODAM]);
    /* Calculate base damage, used to calculating slays */
    borg_trait[BI_WBASEDAM] = (item->dd * item->ds);

    /* Hack -- Reward High Level Warriors with Res Fear */
    if (player_has(player, PF_BRAVERY_30)) {
        /* Resist fear at level 30 */
        if (borg_trait[BI_CLEVEL] >= 30)
            borg_trait[BI_RFEAR] = true;
    }

    /* Affect Skill -- stealth (bonus one) */
    borg_trait[BI_STL] += 1;

    /* Affect Skill -- disarming (DEX and INT) */
    borg_trait[BI_DISP] += borg_adj_dex_dis[my_stat_ind[STAT_DEX]];
    borg_trait[BI_DISM] += borg_adj_int_dis[my_stat_ind[STAT_INT]];

    /* Affect Skill -- magic devices (INT) */
    borg_trait[BI_DEV] += borg_adj_int_dev[my_stat_ind[STAT_INT]];

    /* Affect Skill -- saving throw (WIS) */
    borg_trait[BI_SAV] += borg_adj_wis_sav[my_stat_ind[STAT_WIS]];

    /* Affect Skill -- disarming (Level, by Class) */
    borg_trait[BI_DISP] += (cb_ptr->x_skills[SKILL_DISARM_PHYS]
        * borg_trait[BI_MAXCLEVEL] / 10);
    borg_trait[BI_DISM] += (cb_ptr->x_skills[SKILL_DISARM_MAGIC]
        * borg_trait[BI_MAXCLEVEL] / 10);

    /* Affect Skill -- magic devices (Level, by Class) */
    borg_trait[BI_DEV]
        += (cb_ptr->x_skills[SKILL_DEVICE] * borg_trait[BI_MAXCLEVEL] / 10);

    /* Affect Skill -- saving throw (Level, by Class) */
    borg_trait[BI_SAV]
        += (cb_ptr->x_skills[SKILL_SAVE] * borg_trait[BI_MAXCLEVEL] / 10);

    /* Affect Skill -- stealth (Level, by Class) */
    borg_trait[BI_STL]
        += (cb_ptr->x_skills[SKILL_STEALTH] * borg_trait[BI_MAXCLEVEL] / 10);

    /* Affect Skill -- search ability (Level, by Class) */
    borg_trait[BI_SRCH]
        += (cb_ptr->x_skills[SKILL_SEARCH] * borg_trait[BI_MAXCLEVEL] / 10);

    /* Affect Skill -- combat (normal) (Level, by Class) */
    borg_trait[BI_THN] += (cb_ptr->x_skills[SKILL_TO_HIT_MELEE]
        * borg_trait[BI_MAXCLEVEL] / 10);

    /* Affect Skill -- combat (shooting) (Level, by Class) */
    borg_trait[BI_THB]
        += (cb_ptr->x_skills[SKILL_TO_HIT_BOW] * borg_trait[BI_MAXCLEVEL] / 10);

    /* Affect Skill -- combat (throwing) (Level, by Class) */
    borg_trait[BI_THT] += (cb_ptr->x_skills[SKILL_TO_HIT_THROW]
        * borg_trait[BI_MAXCLEVEL] / 10);

    /* Limit Skill -- stealth from 0 to 30 */
    if (borg_trait[BI_STL] > 30)
        borg_trait[BI_STL] = 30;
    if (borg_trait[BI_STL] < 0)
        borg_trait[BI_STL] = 0;

    /* Limit Skill -- digging from 1 up */
    if (borg_trait[BI_DIG] < 1)
        borg_trait[BI_DIG] = 1;

    /*** Some penalties to consider ***/

    /* Fear from spell or effect or flag */
    if (borg_trait[BI_ISAFRAID] || borg_trait[BI_CRSFEAR]) {
        borg_trait[BI_TOHIT] -= 20;
        borg_trait[BI_ARMOR] += 8;
        borg_trait[BI_DEV] = borg_trait[BI_DEV] * 95 / 100;
    }

    /* priest weapon penalty for non-blessed edged weapons */
    if (player_has(player, PF_BLESS_WEAPON)
        && ((item->tval == TV_SWORD || item->tval == TV_POLEARM)
            && !of_has(item->flags, OF_BLESSED))) {
        /* Reduce the real bonuses */
        borg_trait[BI_TOHIT] -= 2;
        borg_trait[BI_TODAM] -= 2;
    }

    /*** Count needed enchantment ***/

    /* Assume no enchantment needed */
    my_need_enchant_to_a = 0;
    my_need_enchant_to_h = 0;
    my_need_enchant_to_d = 0;
    my_need_brand_weapon = 0;

    /* Hack -- enchant all the equipment (weapons) */
    for (i = INVEN_WIELD; i <= INVEN_BOW; i++) {
        item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Skip "unknown" items */
        if (!item->ident)
            continue;

        /* Most classes store the enchants until they get
         * a 3x shooter (like a long bow).
         * --Important: Also look in borg7.c for the enchanting.
         * --We do not want the bow enchanted by mistake.
         */
        if (i == INVEN_BOW && /* bow */
            borg_trait[BI_AMMO_POWER] < 3 && /* 3x shooter */
            (!item->art_idx && !item->ego_idx)) /* Not Ego or Artifact */
            continue;

        /* Enchant all weapons (to hit) */
        if ((borg_spell_legal_fail(ENCHANT_WEAPON, 65)
            || borg_trait[BI_AENCH_SWEP] >= 1)) {
            if (item->to_h < borg_cfg[BORG_ENCHANT_LIMIT]) {
                my_need_enchant_to_h
                    += (borg_cfg[BORG_ENCHANT_LIMIT] - item->to_h);
            }

            /* Enchant all weapons (to damage) */
            if (item->to_d < borg_cfg[BORG_ENCHANT_LIMIT]) {
                my_need_enchant_to_d
                    += (borg_cfg[BORG_ENCHANT_LIMIT] - item->to_d);
            }
        } else /* I don't have the spell or *enchant* */
        {
            if (item->to_h < 8) {
                my_need_enchant_to_h += (8 - item->to_h);
            }

            /* Enchant all weapons (to damage) */
            if (item->to_d < 8) {
                my_need_enchant_to_d += (8 - item->to_d);
            }
        }
    }

    /* Hack -- enchant all the equipment (armor) */
    for (i = INVEN_BODY; i <= INVEN_FEET; i++) {
        item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Skip "unknown" items */
        if (!item->ident)
            continue;

        /* Note need for enchantment */
        if (borg_spell_legal_fail(ENCHANT_ARMOUR, 65)
            || borg_trait[BI_AENCH_SARM] >= 1) {
            if (item->to_a < borg_cfg[BORG_ENCHANT_LIMIT]) {
                my_need_enchant_to_a
                    += (borg_cfg[BORG_ENCHANT_LIMIT] - item->to_a);
            }
        } else {
            if (item->to_a < 8) {
                my_need_enchant_to_a += (8 - item->to_a);
            }
        }
    }

    /* Examine the lite */
    item = &borg_items[INVEN_LIGHT];

    /* Assume normal lite radius */
    borg_trait[BI_CURLITE] = 0;

    /* Glowing player has light */
    if (borg_trait[BI_LIGHT])
        borg_trait[BI_CURLITE] = borg_trait[BI_LIGHT];

    /* Lite */
    if (item->tval == TV_LIGHT) {
        if (item->timeout || of_has(item->flags, OF_NO_FUEL)) {
            if (of_has(item->flags, OF_LIGHT_2)) {
                borg_trait[BI_CURLITE] = borg_trait[BI_CURLITE] + 2;
            } else if (of_has(item->flags, OF_LIGHT_3)) {
                borg_trait[BI_CURLITE] = borg_trait[BI_CURLITE] + 3;
            }
        }
    }

    borg_trait[BI_CURLITE] += item->modifiers[OBJ_MOD_LIGHT];

    /* Special way to handle See Inv */
    if (borg_see_inv >= 1)
        borg_trait[BI_SINV] = true;
    if (borg_trait[BI_CDEPTH] == 0
        && /* only in town.  Allow him to recall down */
        borg_spell_legal(SENSE_INVISIBLE))
        borg_trait[BI_SINV] = true;

    /* Very special handling of Free Action.
     * If the person has perfect Savings throw, he can be
     * considered ok on Free Action.  This can free up an
     * equipment slot.
     */
    if (borg_trait[BI_SAV] >= 100)
        borg_trait[BI_FRACT] = true;

    /* Special case for RBlindness.  Perfect saves and the
     * resistances for light and dark are good enough for RBlind
     */
    if (borg_trait[BI_SAV] >= 100 && borg_trait[BI_RDARK]
        && borg_trait[BI_RLITE])
        borg_trait[BI_RBLIND] = true;

    /*** Quiver needs to be evaluated ***/

    /* Hack -- ignore invalid missiles */
    for (i = QUIVER_START; i < QUIVER_END; i++)
        borg_notice_ammo(i);
    }

/*
 * Helper function -- notice the player inventory
 */
static void borg_notice_inventory(void)
{
    int        i;

    borg_item *item;

    /*** Reset counters ***/

    /* Reset basic */
    amt_food_lowcal = 0;
    amt_food_hical = 0;

    /* Reset healing */
    amt_slow_poison = 0;
    amt_cure_confusion = 0;
    amt_cure_blind = 0;

    /* Reset stat potions */
    for (i = 0; i < 6; i++)
        amt_inc_stat[i] = 0;

    /* Reset books */
    for (i = 0; i < 9; i++)
        amt_book[i] = 0;

    /* Reset various */
    amt_add_stat[STAT_STR] = 0;
    amt_add_stat[STAT_INT] = 0;
    amt_add_stat[STAT_WIS] = 0;
    amt_add_stat[STAT_DEX] = 0;
    amt_add_stat[STAT_CON] = 0;

    amt_fix_exp = 0;
    amt_cool_staff = 0;
    amt_cool_wand = 0;
    amt_digger = 0;

    /*** Process the inventory ***/

    /* Scan the inventory */
    for (i = 0; i < PACK_SLOTS; i++) {
        item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) {
            borg_trait[BI_EMPTY]++;
            continue;
        }

        /* special case for ammo outside the quiver. */
        /* this happens when we are deciding what to buy so items */
        /* are put in empty slots */
        if (borg_is_ammo(item->tval)) {
            borg_notice_ammo(i);
            continue;
        }

        /* total up the weight of the items */
        borg_trait[BI_WEIGHT] += item->weight * item->iqty;

        /* Does the borg need to get an ID for it? */
        if (borg_item_note_needs_id(item))
            my_need_id++;

        /* Hack -- skip un-aware items */
        if (!item->kind)
            continue;

        /* count up the items on the borg (do not count artifacts  */
        /* that are not being wielded) */
        borg_has[item->kind] += item->iqty;

        /* track first cursed item */
        if (item->uncursable) {
            borg_trait[BI_WHERE_CURSED] |= BORG_INVEN;
            if (!borg_trait[BI_FIRST_CURSED])
                borg_trait[BI_FIRST_CURSED] = i + 1;
        }

        /* Analyze the item */
        switch (item->tval) {
            /* Books */
        case TV_MAGIC_BOOK:
        case TV_PRAYER_BOOK:
        case TV_NATURE_BOOK:
        case TV_SHADOW_BOOK:
        case TV_OTHER_BOOK:
        /* Skip incorrect books (if we can browse this book, it is good) */
        if (!obj_kind_can_browse(&k_info[item->kind]))
            break;
        /* Count the books */
        amt_book[borg_get_book_num(item->sval)] += item->iqty;
        break;

        /* Food */
        case TV_MUSHROOM:
        if (item->sval == sv_mush_purging || item->sval == sv_mush_restoring
            || item->sval == sv_mush_cure_mind) {
            if (borg_cfg[BORG_MUNCHKIN_START]
                && borg_trait[BI_MAXCLEVEL]
                < borg_cfg[BORG_MUNCHKIN_LEVEL]) {
                break;
            }
        }
        if (item->sval == sv_mush_second_sight
            || item->sval == sv_mush_emergency
            || item->sval == sv_mush_terror
            || item->sval == sv_mush_stoneskin
            || item->sval == sv_mush_debility
            || item->sval == sv_mush_sprinting)
            if (borg_cfg[BORG_MUNCHKIN_START]
                && borg_trait[BI_MAXCLEVEL]
                >= borg_cfg[BORG_MUNCHKIN_LEVEL]) {
                borg_trait[BI_ASHROOM] += item->iqty;
            }
        /* fall through */
        case TV_FOOD:
        /* Analyze */
        {
            /* unknown types */
            if (!item->kind)
                break;

            /* check for food that hurts us */
            if (borg_obj_has_effect(item->kind, EF_CRUNCH, -1)
                || borg_obj_has_effect(
                    item->kind, EF_TIMED_INC, TMD_CONFUSED))
                break;

            /* check for food that gives nutrition */
            if (item->tval == TV_MUSHROOM) {
                /* mushrooms that increase nutrition are low effect */
                if (borg_obj_has_effect(item->kind, EF_NOURISH, 0))
                    amt_food_lowcal += item->iqty;
            } else /* TV_FOOD */
            {
                if (item->sval == sv_food_apple
                    || item->sval == sv_food_handful
                    || item->sval == sv_food_slime_mold
                    || item->sval == sv_food_pint
                    || item->sval == sv_food_sip) {
                    amt_food_lowcal += item->iqty;
                } else if (item->sval == sv_food_ration
                    || item->sval == sv_food_slice
                    || item->sval == sv_food_honey_cake
                    || item->sval == sv_food_waybread
                    || item->sval == sv_food_draught)
                    amt_food_hical += item->iqty;
            }

            /* check for food that does stuff */
            if (borg_obj_has_effect(item->kind, EF_CURE, TMD_POISONED))
                borg_trait[BI_ACUREPOIS] += item->iqty;
            if (borg_obj_has_effect(item->kind, EF_CURE, TMD_CONFUSED))
                amt_cure_confusion += item->iqty;
            if (borg_obj_has_effect(item->kind, EF_CURE, TMD_BLIND))
                amt_cure_blind += item->iqty;
        }
        break;

        /* Potions */
        case TV_POTION:
        /* Analyze */
        if (item->sval == sv_potion_healing)
            borg_trait[BI_AHEAL] += item->iqty;
        else if (item->sval == sv_potion_star_healing)
            borg_trait[BI_AEZHEAL] += item->iqty;
        else if (item->sval == sv_potion_life)
            borg_trait[BI_ALIFE] += item->iqty;
        else if (item->sval == sv_potion_cure_critical)
            borg_trait[BI_ACCW] += item->iqty;
        else if (item->sval == sv_potion_cure_serious)
            borg_trait[BI_ACSW] += item->iqty;
        else if (item->sval == sv_potion_cure_light)
            borg_trait[BI_ACLW] += item->iqty;
        else if (item->sval == sv_potion_cure_poison)
            borg_trait[BI_ACUREPOIS] += item->iqty;
        else if (item->sval == sv_potion_resist_heat)
            borg_trait[BI_ARESHEAT] += item->iqty;
        else if (item->sval == sv_potion_resist_cold)
            borg_trait[BI_ARESCOLD] += item->iqty;
        else if (item->sval == sv_potion_resist_pois)
            borg_trait[BI_ARESPOIS] += item->iqty;
        else if (item->sval == sv_potion_inc_str)
            amt_inc_stat[STAT_STR] += item->iqty;
        else if (item->sval == sv_potion_inc_int)
            amt_inc_stat[STAT_INT] += item->iqty;
        else if (item->sval == sv_potion_inc_wis)
            amt_inc_stat[STAT_WIS] += item->iqty;
        else if (item->sval == sv_potion_inc_dex)
            amt_inc_stat[STAT_DEX] += item->iqty;
        else if (item->sval == sv_potion_inc_con)
            amt_inc_stat[STAT_CON] += item->iqty;
        else if (item->sval == sv_potion_inc_all) {
            amt_inc_stat[STAT_STR] += item->iqty;
            amt_inc_stat[STAT_INT] += item->iqty;
            amt_inc_stat[STAT_WIS] += item->iqty;
            amt_inc_stat[STAT_DEX] += item->iqty;
            amt_inc_stat[STAT_CON] += item->iqty;
        } else if (item->sval == sv_potion_restore_life)
            amt_fix_exp += item->iqty;
        else if (item->sval == sv_potion_speed)
            borg_trait[BI_ASPEED] += item->iqty;
        break;

        /* Scrolls */
        case TV_SCROLL:

        if (item->sval == sv_scroll_identify)
            borg_trait[BI_AID] += item->iqty;
        else if (item->sval == sv_scroll_recharging)
            borg_trait[BI_ARECHARGE] += item->iqty;
        else if (item->sval == sv_scroll_phase_door)
            borg_trait[BI_APHASE] += item->iqty;
        else if (item->sval == sv_scroll_teleport)
            borg_trait[BI_ATELEPORT] += item->iqty;
        else if (item->sval == sv_scroll_word_of_recall)
            borg_trait[BI_RECALL] += item->iqty;
        else if (item->sval == sv_scroll_enchant_armor)
            borg_trait[BI_AENCH_ARM] += item->iqty;
        else if (item->sval == sv_scroll_star_enchant_armor)
            borg_trait[BI_AENCH_SARM] += item->iqty;
        else if (item->sval == sv_scroll_enchant_weapon_to_hit)
            borg_trait[BI_AENCH_TOH] += item->iqty;
        else if (item->sval == sv_scroll_enchant_weapon_to_dam)
            borg_trait[BI_AENCH_TOD] += item->iqty;
        else if (item->sval == sv_scroll_star_enchant_weapon)
            borg_trait[BI_AENCH_SWEP] += item->iqty;
        else if (item->sval == sv_scroll_protection_from_evil)
            borg_trait[BI_APFE] += item->iqty;
        else if (item->sval == sv_scroll_rune_of_protection)
            borg_trait[BI_AGLYPH] += item->iqty;
        else if (item->sval == sv_scroll_teleport_level) {
            borg_trait[BI_ATELEPORTLVL] += item->iqty;
            borg_trait[BI_ATELEPORT] += 1;
        } else if (item->sval == sv_scroll_mass_banishment)
            borg_trait[BI_AMASSBAN] += item->iqty;
        break;

        /* Rods */
        case TV_ROD:

        /* Analyze */
        if (item->sval == sv_rod_recall) {
            /* Don't count on it if I suck at activations */
            if (borg_activate_failure(item->tval, item->sval) < 500) {
                borg_trait[BI_RECALL] += item->iqty * 100;
            } else {
                borg_trait[BI_RECALL] += item->iqty;
            }
        } else if (item->sval == sv_rod_detection) {
            borg_trait[BI_ADETTRAP] += item->iqty * 100;
            borg_trait[BI_ADETDOOR] += item->iqty * 100;
            borg_trait[BI_ADETEVIL] += item->iqty * 100;
        } else if (item->sval == sv_rod_illumination)
            borg_trait[BI_ALITE] += item->iqty * 100;
        else if (item->sval == sv_rod_speed) {
            /* Don't count on it if I suck at activations */
            if (borg_activate_failure(item->tval, item->sval) < 500) {
                borg_trait[BI_ASPEED] += item->iqty * 100;
            } else {
                borg_trait[BI_ASPEED] += item->iqty;
            }
        } else if (item->sval == sv_rod_mapping)
            borg_trait[BI_AMAGICMAP] += item->iqty * 100;
        else if (item->sval == sv_rod_healing) {
            /* only +2 per rod because of long charge time. */
            /* Don't count on it if I suck at activations */
            if (borg_activate_failure(item->tval, item->sval) < 500) {
                borg_trait[BI_AHEAL] += item->iqty * 3;
            } else {
                borg_trait[BI_AHEAL] += item->iqty + 1;
            }
        } else if (item->sval == sv_rod_light
            || item->sval == sv_rod_fire_bolt
            || item->sval == sv_rod_elec_bolt
            || item->sval == sv_rod_cold_bolt
            || item->sval == sv_rod_acid_bolt) {
            borg_trait[BI_AROD1] += item->iqty;
        } else if (item->sval == sv_rod_drain_life
            || item->sval == sv_rod_fire_ball
            || item->sval == sv_rod_elec_ball
            || item->sval == sv_rod_cold_ball
            || item->sval == sv_rod_acid_ball) {
            borg_trait[BI_AROD2] += item->iqty;
        }
        break;

        /* Wands */
        case TV_WAND:

        /* Analyze each */
        if (item->sval == sv_wand_teleport_away) {
            borg_trait[BI_ATPORTOTHER] += item->pval;
        }

        if (item->sval == sv_wand_stinking_cloud
            && borg_trait[BI_MAXDEPTH] < 30) {
            amt_cool_wand += item->pval;
        }

        if (item->sval == sv_wand_magic_missile
            && borg_trait[BI_MAXDEPTH] < 30) {
            amt_cool_wand += item->pval;
        }

        if (item->sval == sv_wand_annihilation) {
            amt_cool_wand += item->pval;
        }

        break;

        /* Staffs */
        case TV_STAFF:
        /* Analyze */
        if (item->sval == sv_staff_teleportation) {
            if (borg_trait[BI_MAXDEPTH] <= 95) {
                borg_trait[BI_AESCAPE] += (item->iqty);
                if (borg_activate_failure(item->tval, item->sval) < 500) {
                    borg_trait[BI_AESCAPE] += item->pval;
                }
            }
        } else if (item->sval == sv_staff_speed) {
            if (borg_trait[BI_MAXDEPTH] <= 95)
                borg_trait[BI_ASPEED] += item->pval;
        } else if (item->sval == sv_staff_healing)
            borg_trait[BI_AHEAL] += item->pval;
        else if (item->sval == sv_staff_the_magi)
            borg_trait[BI_ASTFMAGI] += item->pval;
        else if (item->sval == sv_staff_destruction)
            borg_trait[BI_ASTFDEST] += item->pval;
        else if (item->sval == sv_staff_power)
            amt_cool_staff += item->iqty;
        else if (item->sval == sv_staff_holiness) {
            amt_cool_staff += item->iqty;
            borg_trait[BI_AHEAL] += item->pval;
        }

        break;

        /* Flasks */
        case TV_FLASK:

        /* Use as fuel if we equip a lantern */
        if (borg_items[INVEN_LIGHT].sval == sv_light_lantern)
            borg_trait[BI_AFUEL] += item->iqty;

        break;

        /* Torches */
        case TV_LIGHT:

        /* Use as fuel if it is a torch and we carry a torch */
        if ((item->sval == sv_light_torch && item->timeout >= 1)
            && (borg_items[INVEN_LIGHT].sval == sv_light_torch)
            && borg_items[INVEN_LIGHT].iqty) {
            borg_trait[BI_AFUEL] += item->iqty;
        }

        break;

        /* Weapons */
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_SWORD:
        /* These items are checked a bit later in a sub routine
         * to notice the flags.  It is done outside this switch.
         */
        break;

        /* Shovels and such */
        case TV_DIGGING:

        /* Hack -- ignore worthless ones (including cursed) */
        if (item->value <= 0)
            break;
        if (item->cursed)
            break;

        /* Do not carry if weak, won't be able to dig anyway */
        if (borg_trait[BI_DIG] < BORG_DIG)
            break;

        amt_digger += item->iqty;
        break;
        }
    }

    /* flasks of oil are ammo at low levels */
    if (borg_has[kv_flask_oil] && borg_trait[BI_CLEVEL] < 15) {
        /* only count the first 15 */
        if (borg_has[kv_flask_oil] < 15)
            borg_trait[BI_AMISSILES] += borg_has[kv_flask_oil];
        else
            borg_trait[BI_AMISSILES] += 15;
    }

    /*** Process the Spells and Prayers ***/
    /*    artifact activations are accounted here
     *  But some artifacts are not counted for two reasons .
     *  1.  Some spells-powers are needed instantly and are considered in
     *  the borg preparation code.  An artifact maybe non-charged at the
     *  moment he needes it.  Then he would need the spell and not be able
     *  to cast it. (ie. teleport, phase)
     *  2.  An artifact may grant a power then he assumes he has infinite
     *  amounts.  He then sells off his scrolls with the duplicate power.
     *  When it comes time to upgrade and swap out the artifact, he wont
     *  because his power drops since he does not have the scrolls anymore.
     *  and he does not buy items first.
     *
     *  A possible solution would be to have him keep a few scrolls as a
     *  back-up, or to remove the bonus for level preparation from borg_power.
     *  Right now I think it is better that he not consider the artifacts
     *  Whose powers are considered in borg_prep.
     */

     /* Handle "satisfy hunger" -> infinite food */
    if (borg_spell_legal_fail(REMOVE_HUNGER, 80)
        || borg_spell_legal_fail(HERBAL_CURING, 80)) /* VAMPIRE_STRIKE? */
    {
        borg_trait[BI_FOOD] += 1000;
    }

    /* Handle "identify" -> infinite identifies */
    if (borg_spell_legal(IDENTIFY_RUNE)) {
        borg_trait[BI_AID] += 1000;
    }

    /* Handle "detect traps" */
    if (borg_spell_legal(FIND_TRAPS_DOORS_STAIRS)
        || borg_spell_legal(DETECTION)) {
        borg_trait[BI_ADETTRAP] = 1000;
    }

    /* Handle "detect evil & monsters" */
    if (borg_spell_legal(REVEAL_MONSTERS) || borg_spell_legal(DETECT_LIFE)
        || borg_spell_legal(DETECT_EVIL) || borg_spell_legal(READ_MINDS)
        || borg_spell_legal(DETECT_MONSTERS) || borg_spell_legal(SEEK_BATTLE)) {
        borg_trait[BI_ADETEVIL] = 1000;
    }

    /* Handle DETECTION */
    if (borg_spell_legal(DETECTION)
        || borg_equips_item(act_enlightenment, false)
        || borg_equips_item(act_clairvoyance, false)) {
        borg_trait[BI_ADETDOOR] = 1000;
        borg_trait[BI_ADETTRAP] = 1000;
        borg_trait[BI_ADETEVIL] = 1000;
    }

    /* Handle "See Invisible" in a special way. */
    if (borg_spell_legal(SENSE_INVISIBLE)) {
        borg_trait[BI_DINV] = true;
    }

    /* Handle "magic mapping" */
    if (borg_spell_legal(SENSE_SURROUNDINGS)
        || borg_equips_item(act_detect_all, false)
        || borg_equips_item(act_mapping, false)) {
        borg_trait[BI_ADETDOOR] = 1000;
        borg_trait[BI_ADETTRAP] = 1000;
        borg_trait[BI_AMAGICMAP] = 1000;
    }

    /* Handle "call lite" */
    if (borg_spell_legal(LIGHT_ROOM) || borg_equips_item(act_light, false)
        || borg_equips_item(act_illumination, false)
        || borg_spell_legal(CALL_LIGHT)) {
        borg_trait[BI_ALITE] += 1000;
    }

    /* Handle PROTECTION_FROM_EVIL */
    if (borg_spell_legal(PROTECTION_FROM_EVIL)
        || borg_equips_item(act_protevil, false) || borg_has[kv_staff_holiness]
        || borg_equips_item(act_staff_holy, false)) {
        borg_trait[BI_APFE] += 1000;
    }

    /* Handle "rune of protection" glyph" */
    if (borg_spell_legal(GLYPH_OF_WARDING)
        || borg_equips_item(act_glyph, false)) {
        borg_trait[BI_AGLYPH] += 1000;
    }

    /* Handle "detect traps/doors" */
    if (borg_spell_legal(FIND_TRAPS_DOORS_STAIRS)) {
        borg_trait[BI_ADETDOOR] = 1000;
        borg_trait[BI_ADETTRAP] = 1000;
    }

    /* Handle ENCHANT_WEAPON */
    if (borg_spell_legal_fail(ENCHANT_WEAPON, 65)
        || borg_equips_item(act_enchant_weapon, false)) {
        borg_trait[BI_AENCH_TOH] += 1000;
        borg_trait[BI_AENCH_TOD] += 1000;
        borg_trait[BI_AENCH_SWEP] += 1000;
    }
    if (borg_equips_item(act_enchant_tohit, false)) {
        borg_trait[BI_AENCH_TOH] += 1000;
    }
    if (borg_equips_item(act_enchant_todam, false)) {
        borg_trait[BI_AENCH_TOD] += 1000;
    }

    /* Handle "Brand Weapon (bolts)" */
    if (borg_equips_item(act_firebrand, false)
        || borg_spell_legal_fail(BRAND_AMMUNITION, 65)) {
        borg_trait[BI_ABRAND] += 1000;
    }

    /* Handle "enchant armor" */
    if (borg_spell_legal_fail(ENCHANT_ARMOUR, 65)
        || borg_equips_item(act_enchant_armor, false)
        || borg_equips_item(act_enchant_armor2, false)) {
        borg_trait[BI_AENCH_ARM] += 1000;
        borg_trait[BI_AENCH_SARM] += 1000;
    }

    /* Handle Diggers (stone to mud) */
    if (borg_spell_legal_fail(TURN_STONE_TO_MUD, 40)
        || borg_spell_legal_fail(SHATTER_STONE, 40)
        || borg_equips_item(act_stone_to_mud, false)
        || borg_equips_ring(sv_ring_digging)) {
        amt_digger += 1;
    }

    /* Handle recall */
    if (borg_spell_legal_fail(WORD_OF_RECALL, 40)
        || (borg_trait[BI_CDEPTH] == 100 && borg_spell_legal(WORD_OF_RECALL))) {
        borg_trait[BI_RECALL] += 1000;
    }
    if (borg_equips_item(act_recall, false)) {
        borg_trait[BI_RECALL] += 1;
    }

    /* Handle teleport_level */
    if (borg_spell_legal_fail(TELEPORT_LEVEL, 20)) {
        borg_trait[BI_ATELEPORTLVL] += 1000;
    }

    /* Handle PhaseDoor spell carefully */
    if (borg_spell_legal_fail(PHASE_DOOR, 3)) {
        borg_trait[BI_APHASE] += 1000;
    }
    if (borg_equips_item(act_tele_phase, false)) {
        borg_trait[BI_APHASE] += 1;
    }

    /* Handle teleport spell carefully */
    if (borg_spell_legal_fail(TELEPORT_SELF, 1) ||
        borg_spell_legal_fail(PORTAL, 1) ||
        borg_spell_legal_fail(SHADOW_SHIFT, 1) ||
        borg_spell_legal_fail(DIMENSION_DOOR, 1)) {
        borg_trait[BI_ATELEPORT] += 1000;
    }
    if (borg_equips_item(act_tele_long, false)) {
        borg_trait[BI_AESCAPE] += 1;
        borg_trait[BI_ATELEPORT] += 1;
    }

    /* Handle teleport away */
    if (borg_spell_legal_fail(TELEPORT_OTHER, 40)) {
        borg_trait[BI_ATPORTOTHER] += 1000;
    }

    /* Handle Holy Word prayer just to see if legal */
    if (borg_spell_legal(HOLY_WORD)) {
        borg_trait[BI_AHWORD] += 1000;
    }

    /* speed spells HASTE*/
    if (borg_spell_legal(HASTE_SELF) || borg_equips_item(act_haste, false)
        || borg_equips_item(act_haste1, false)
        || borg_equips_item(act_haste2, false)) {
        borg_trait[BI_ASPEED] += 1000;
    }

    /* Handle "cure light wounds" */
    if (borg_equips_item(act_cure_light, false)) {
        borg_trait[BI_ACLW] += 1000;
    }

    /* Handle "cure serious wounds" */
    if (borg_equips_item(act_cure_serious, false)) {
        borg_trait[BI_ACSW] += 1000;
    }

    /* Handle "cure critical wounds" */
    if (borg_equips_item(act_cure_critical, false)) {
        borg_trait[BI_ACCW] += 1000;
    }

    /* Handle "heal" */
    if (borg_equips_item(act_cure_full, false)
        || borg_equips_item(act_cure_full2, false)
        || borg_equips_item(act_cure_nonorlybig, false)
        || borg_equips_item(act_heal1, false)
        || borg_equips_item(act_heal2, false)
        || borg_equips_item(act_heal3, false) || borg_spell_legal(HEALING)) {
        borg_trait[BI_AHEAL] += 1000;
    }

    /* Handle "fix exp" */
    if (borg_equips_item(act_cure_nonorlybig, false)
        || borg_equips_item(act_restore_exp, false)
        || borg_equips_item(act_restore_st_lev, false)
        || borg_equips_item(act_restore_life, false)) {
        amt_fix_exp += 1000;
    }

    /* Handle REMEMBRANCE -- is just as good as Hold Life */
    if (borg_spell_legal(REMEMBRANCE)
        || borg_equips_item(act_cure_nonorlybig, false)
        || borg_equips_item(act_restore_exp, false)
        || borg_equips_item(act_restore_st_lev, false)
        || borg_equips_item(act_restore_life, false)) {
        borg_trait[BI_HLIFE] = true;
    }

    /* Handle "recharge" */
    if (borg_equips_item(act_recharge, false) || borg_spell_legal(RECHARGING)) {
        borg_trait[BI_ARECHARGE] += 1000;
    }

    /*** Process the Needs ***/

    /* No need for fuel if we know it doesn't need it */
    if (of_has(borg_items[INVEN_LIGHT].flags, OF_NO_FUEL))
        borg_trait[BI_AFUEL] += 1000;

    /* No need to *buy* stat increase potions */
    if (my_stat_cur[STAT_STR] >= (18 + 100) +
        (10 * (player->race->r_adj[STAT_STR]) +
            player->class->c_adj[STAT_STR]))
        amt_add_stat[STAT_STR] += 1000;

    if (my_stat_cur[STAT_INT] >= (18 + 100) +
        (10 * (player->race->r_adj[STAT_INT]) +
            player->class->c_adj[STAT_INT]))
        amt_add_stat[STAT_INT] += 1000;

    if (my_stat_cur[STAT_WIS] >= (18 + 100) +
        (10 * (player->race->r_adj[STAT_WIS]) +
            player->class->c_adj[STAT_WIS]))
        amt_add_stat[STAT_WIS] += 1000;

    if (my_stat_cur[STAT_DEX] >= (18 + 100) +
        (10 * (player->race->r_adj[STAT_DEX]) +
            player->class->c_adj[STAT_DEX]))
        amt_add_stat[STAT_DEX] += 1000;

    if (my_stat_cur[STAT_CON] >= (18 + 100) +
        (10 * (player->race->r_adj[STAT_CON]) +
            player->class->c_adj[STAT_CON]))
        amt_add_stat[STAT_CON] += 1000;

    /* No need for experience repair */
    if (!borg_trait[BI_ISFIXEXP])
        amt_fix_exp += 1000;

    /* Correct the high and low calorie foods */
    borg_trait[BI_FOOD] += amt_food_hical;
    if (amt_food_hical <= 3)
        borg_trait[BI_FOOD] += amt_food_lowcal;

    /* If weak, do not count food spells */
    if (borg_trait[BI_ISWEAK] && (borg_trait[BI_FOOD] >= 1000))
        borg_trait[BI_FOOD] -= 1000;
}

/*
 * Analyze the equipment and inventory
 */
void borg_notice(bool notice_swap)
{
    /* Clear out trait arrays */
    memset(borg_has, 0, z_info->k_max * sizeof(int));
    memset(borg_trait, 0, BI_MAX * sizeof(int));
    memset(borg_activation, 0, z_info->act_max * sizeof(int));

    /* Many of our variables are tied to borg_trait[], which is erased at the
     * the start of borg_notice().  So we must update the frame the cheat in
     * all the non inventory skills.
     */
    borg_update_frame();

    /* Notice the equipment */
    borg_notice_equipment();

    /* Notice the inventory */
    borg_notice_inventory();

    /* number of inventory slots the quiver used  */
    borg_trait[BI_QUIVER_SLOTS]
        = (borg_trait[BI_AMMO_COUNT] - 1) / z_info->quiver_slot_size + 1;

    /* Notice and locate my swap weapon */
    if (notice_swap) {
        borg_notice_weapon_swap();
        borg_notice_armour_swap();
    }
    borg_trait[BI_SRACID] = borg_trait[BI_RACID] || armour_swap_resist_acid
        || weapon_swap_resist_acid
        || borg_spell_legal_fail(RESISTANCE, 15); /* Res FECAP */
    borg_trait[BI_SRELEC] = borg_trait[BI_RELEC] || armour_swap_resist_elec
        || weapon_swap_resist_elec
        || borg_spell_legal_fail(RESISTANCE, 15); /* Res FECAP */
    borg_trait[BI_SRFIRE] = borg_trait[BI_RFIRE] || armour_swap_resist_fire
        || weapon_swap_resist_fire
        || borg_spell_legal_fail(RESISTANCE, 15); /* Res FECAP */
    borg_trait[BI_SRCOLD] = borg_trait[BI_RCOLD] || armour_swap_resist_cold
        || weapon_swap_resist_cold
        || borg_spell_legal_fail(RESISTANCE, 15); /* Res FECAP */
    borg_trait[BI_SRPOIS] = borg_trait[BI_RPOIS] || armour_swap_resist_pois
        || weapon_swap_resist_pois
        || borg_spell_legal_fail(RESIST_POISON, 15); /* Res P */
    borg_trait[BI_SRFEAR] = borg_trait[BI_RFEAR] || armour_swap_resist_fear
        || weapon_swap_resist_fear;
    borg_trait[BI_SRLITE] = borg_trait[BI_RLITE] || armour_swap_resist_light
        || weapon_swap_resist_light;
    borg_trait[BI_SRDARK] = borg_trait[BI_RDARK] || armour_swap_resist_dark
        || weapon_swap_resist_dark;
    borg_trait[BI_SRBLIND] = borg_trait[BI_RBLIND] || armour_swap_resist_blind
        || weapon_swap_resist_blind;
    borg_trait[BI_SRCONF] = borg_trait[BI_RCONF] || armour_swap_resist_conf
        || weapon_swap_resist_conf;
    borg_trait[BI_SRSND] = borg_trait[BI_RSND] || armour_swap_resist_sound
        || weapon_swap_resist_sound;
    borg_trait[BI_SRSHRD] = borg_trait[BI_RSHRD] || armour_swap_resist_shard
        || weapon_swap_resist_shard;
    borg_trait[BI_SRNXUS] = borg_trait[BI_RNXUS] || armour_swap_resist_nexus
        || weapon_swap_resist_nexus;
    borg_trait[BI_SRNTHR] = borg_trait[BI_RNTHR] || armour_swap_resist_neth
        || weapon_swap_resist_neth;
    borg_trait[BI_SRKAOS] = borg_trait[BI_RKAOS] || armour_swap_resist_chaos
        || weapon_swap_resist_chaos;
    borg_trait[BI_SRDIS] = borg_trait[BI_RDIS] || armour_swap_resist_disen
        || weapon_swap_resist_disen;
    borg_trait[BI_SHLIFE] = borg_trait[BI_HLIFE] || armour_swap_hold_life
        || weapon_swap_hold_life;
    borg_trait[BI_SFRACT]
        = borg_trait[BI_FRACT] || armour_swap_free_act || weapon_swap_free_act;

    /* Apply "encumbrance" from weight */
    /* Extract the "weight limit" (in tenth pounds) */
    borg_trait[BI_CARRY] = borg_adj_str_wgt[my_stat_ind[STAT_STR]] * 100;

    /* Apply "encumbrance" from weight */
    if (borg_trait[BI_WEIGHT] > borg_trait[BI_CARRY] / 2)
        borg_trait[BI_SPEED]
        -= ((borg_trait[BI_WEIGHT] - (borg_trait[BI_CARRY] / 2))
            / (borg_trait[BI_CARRY] / 10));
}


/*
 * Update the Borg based on the current "frame"
 *
 * Assumes the Borg is actually in the dungeon.
 * 
 * !FIX !TODO !AJG I think this is wrong.  This should be part of borg_notice since 
 * borg_notice clears the borg_trait array.
 */
void borg_update_frame(void)
{
    int                  i;

    struct player_state *state = &player->known_state;

    /* Assume level is fine */
    borg_trait[BI_ISFIXLEV] = false;

    /* Note "Lev" vs "LEV" */
    if (player->lev < player->max_lev)
        borg_trait[BI_ISFIXLEV] = true;

    /* Extract "LEVEL xxxxxx" */
    borg_trait[BI_CLEVEL] = player->lev;

    /* cheat the max clevel */
    borg_trait[BI_MAXCLEVEL] = player->max_lev;

    /* Note "Winner" */
    borg_trait[BI_KING] = player->total_winner;

    /* Assume experience is fine */
    borg_trait[BI_ISFIXEXP] = false;

    /* Note "Exp" vs "EXP" and am I lower than level 50*/
    if (player->exp < player->max_exp) {
        /* fix it if in town */
        if (borg_trait[BI_CLEVEL] == 50 && borg_trait[BI_CDEPTH] == 0)
            borg_trait[BI_ISFIXEXP] = true;

        /* dont worry about fixing it in the dungeon */
        if (borg_trait[BI_CLEVEL] == 50 && borg_trait[BI_CDEPTH] >= 1)
            borg_trait[BI_ISFIXEXP] = false;

        /* Not at Max Level */
        if (borg_trait[BI_CLEVEL] != 50)
            borg_trait[BI_ISFIXEXP] = true;
    }

    /* Extract "AU xxxxxxxxx" */
    borg_trait[BI_GOLD] = player->au;

    borg_trait[BI_WEIGHT] = player->upkeep->total_weight;

    /* Extract "Fast (+x)" or "Slow (-x)" */
    borg_trait[BI_SPEED] = state->speed;

    /* Check my float for decrementing variables */
    if (borg_trait[BI_SPEED] > 110) {
        borg_game_ratio = 100000 / (((borg_trait[BI_SPEED] - 110) * 10) + 100);
    } else {
        borg_game_ratio = 1000;
    }

    /* A quick cheat to see if I missed a message about my status on some timed spells */
    if (!goal_recalling && player->word_recall)
        goal_recalling = true;
    if (!borg_prot_from_evil && player->timed[TMD_PROTEVIL])
        borg_prot_from_evil = (player->timed[TMD_PROTEVIL] ? true : false);
    if (!borg_speed && (player->timed[TMD_FAST] || player->timed[TMD_SPRINT] || player->timed[TMD_TERROR]))
        (borg_speed = (player->timed[TMD_FAST] || player->timed[TMD_SPRINT] || player->timed[TMD_TERROR]) ? true : false);
    borg_trait[BI_TRACID] = (player->timed[TMD_OPP_ACID] ? true : false);
    borg_trait[BI_TRELEC] = (player->timed[TMD_OPP_ELEC] ? true : false);
    borg_trait[BI_TRFIRE] = (player->timed[TMD_OPP_FIRE] ? true : false);
    borg_trait[BI_TRCOLD] = (player->timed[TMD_OPP_COLD] ? true : false);
    borg_trait[BI_TRPOIS] = (player->timed[TMD_OPP_POIS] ? true : false);
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
    if (borg_speed) {
        if (player->timed[TMD_FAST] || player->timed[TMD_SPRINT])
            borg_trait[BI_SPEED] -= 10;
        else if (player->timed[TMD_TERROR])
            borg_trait[BI_SPEED] -= 5;
    }

    /* Extract "Cur AC xxxxx" */
    borg_trait[BI_ARMOR] = state->ac + state->to_a;

    /* Extract "Cur HP xxxxx" */
    borg_trait[BI_CURHP] = player->chp;

    /* Extract "Max HP xxxxx" */
    borg_trait[BI_MAXHP] = player->mhp;

    /* Extract "Cur SP xxxxx" (or zero) */
    borg_trait[BI_CURSP] = player->csp;

    /* Extract "Max SP xxxxx" (or zero) */
    borg_trait[BI_MAXSP] = player->msp;

    /* Clear all the "state flags" */
    borg_trait[BI_ISWEAK] = borg_trait[BI_ISHUNGRY] = borg_trait[BI_ISFULL] = borg_trait[BI_ISGORGED] = false;
    borg_trait[BI_ISBLIND] = borg_trait[BI_ISCONFUSED] = borg_trait[BI_ISAFRAID] = borg_trait[BI_ISPOISONED] = false;
    borg_trait[BI_ISCUT] = borg_trait[BI_ISSTUN] = borg_trait[BI_ISHEAVYSTUN] = borg_trait[BI_ISIMAGE] = borg_trait[BI_ISSTUDY] = false;
    borg_trait[BI_ISPARALYZED] = false;
    borg_trait[BI_ISFORGET] = false;

    /* Check for "Weak" */
    if (player->timed[TMD_FOOD] < PY_FOOD_WEAK)
        borg_trait[BI_ISWEAK] = borg_trait[BI_ISHUNGRY] = true;

    /* Check for "Hungry" */
    else if (player->timed[TMD_FOOD] < PY_FOOD_HUNGRY)
        borg_trait[BI_ISHUNGRY] = true;

    /* Check for "Normal" */
    else if (player->timed[TMD_FOOD] < PY_FOOD_FULL) /* Nothing */
        ;

    /* Check for "Full" */
    else if (player->timed[TMD_FOOD] < PY_FOOD_MAX)
        borg_trait[BI_ISFULL] = true;

    /* Check for "Gorged" */
    else
        borg_trait[BI_ISGORGED] = borg_trait[BI_ISFULL] = true;

    /* Check for "Blind" */
    if (player->timed[TMD_BLIND])
        borg_trait[BI_ISBLIND] = true;

    /* Check for "Confused" */
    if (player->timed[TMD_CONFUSED])
        borg_trait[BI_ISCONFUSED] = true;

    /* Check for "Afraid" */
    if (player->timed[TMD_AFRAID])
        borg_trait[BI_ISAFRAID] = true;

    /* Check for "Poisoned" */
    if (player->timed[TMD_POISONED])
        borg_trait[BI_ISPOISONED] = true;

    /* Check for any text */
    if (player->timed[TMD_CUT])
        borg_trait[BI_ISCUT] = true;

    /* Check for Stun */
    if (player->timed[TMD_STUN] && (player->timed[TMD_STUN] <= 50))
        borg_trait[BI_ISSTUN] = true;

    /* Check for Heavy Stun */
    if (player->timed[TMD_STUN] > 50)
        borg_trait[BI_ISHEAVYSTUN] = true;

    /* Check for Paralyze */
    if (player->timed[TMD_PARALYZED] > 50)
        borg_trait[BI_ISPARALYZED] = true;

    /* Check for "Hallucinating" */
    if (player->timed[TMD_IMAGE])
        borg_trait[BI_ISIMAGE] = true;

    /* Check for "Amnesia" */
    if (player->timed[TMD_AMNESIA])
        borg_trait[BI_ISFORGET] = true;

    /* Check to BLESS */
    borg_bless = (player->timed[TMD_BLESSED] ? true : false);

    /* Check for "Study" */
    if (player->upkeep->new_spells)
        borg_trait[BI_ISSTUDY] = true;

    /* Parse stats */
    for (i = 0; i < 5; i++) {
        borg_trait[BI_ISFIXSTR + i] = player->stat_cur[STAT_STR + i] < player->stat_max[STAT_STR + i];
        borg_trait[BI_CSTR + i] = player->stat_cur[STAT_STR + i];
        borg_stat[i] = player->stat_cur[i];
    }

    /* Hack -- Access max depth */
    borg_trait[BI_CDEPTH] = player->depth;

    /* Hack -- Access max depth */
    borg_trait[BI_MAXDEPTH] = player->max_depth;
}

void borg_trait_init(void)
{
    borg_has = mem_zalloc(z_info->k_max * sizeof(int));
    borg_trait = mem_zalloc(BI_MAX * sizeof(int));
    borg_activation = mem_zalloc(z_info->act_max * sizeof(int));
}


void borg_trait_free(void)
{
    mem_free(borg_has);
    borg_has = NULL;
    mem_free(borg_trait);
    borg_trait = NULL;
    mem_free(borg_activation);
    borg_activation = NULL;
}


#endif