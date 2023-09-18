/* File: borg4.c */
/*  Purpose: Notice and Power code for the Borg -BEN- */

#include "../angband.h"
#include "../cave.h"
#include "../game-world.h"
#include "../mon-spell.h"
#include "../player-calcs.h"
#include "../cmd-core.h"
#include "../player-spell.h"
#include "../player-timed.h"
#include "../player-util.h"
#include "../project.h"
#include "../trap.h"


#ifdef ALLOW_BORG
#include "borg1.h"
#include "borg2.h"
#include "borg3.h"
#include "borg4.h"

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


int borg_calc_blows(int extra_blows);


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
static void borg_notice_aux_ammo(int slot)
{
    const borg_item * item = &borg_items[slot];

    /* Skip empty items */
    if (!item->iqty) return;

    /* total up the weight of the items */
    borg_skill[BI_WEIGHT] += item->weight * item->iqty;

    /* Count all ammo */
    borg_skill[BI_AMMO_COUNT] += item->iqty;

    if (item->tval != borg_skill[BI_AMMO_TVAL]) return;

    /* Count missiles that fit your bow */
    borg_skill[BI_AMISSILES] += item->iqty;



    /* track first cursed item */
    if (item->uncursable)
    {
        borg_skill[BI_WHERE_CURSED] |= BORG_QUILL;
        if (!borg_skill[BI_FIRST_CURSED])
            borg_skill[BI_FIRST_CURSED] = slot + 1;

        borg_skill[BI_AMISSILES_CURSED] += item->iqty;
        return;
    }

    if (item->ego_idx)
        borg_skill[BI_AMISSILES_SPECIAL] += item->iqty;

    /* check for ammo to enchant */

    /* Hack -- ignore worthless missiles */
    if (item->value <= 0) return;

    /* Only enchant ammo if we have a good shooter,
     * otherwise, store the enchants in the home.
     */
    if (borg_skill[BI_AMMO_POWER] >= 3)
    {

        if ((borg_equips_item(act_firebrand, false) ||
            borg_spell_legal_fail(BRAND_AMMUNITION, 65)) &&
            item->iqty >= 5 &&
            /* Skip artifacts and ego-items */
            !item->ego_idx &&
            !item->art_idx &&
            item->ident &&
            item->tval == borg_skill[BI_AMMO_TVAL])
        {
            my_need_brand_weapon += 10L;
        }

        /* if we have loads of cash (as we will at level 35),  */
        /* enchant missiles */
        if (borg_skill[BI_CLEVEL] > 35)
        {
            if (borg_spell_legal_fail(ENCHANT_WEAPON, 65)
                && item->iqty >= 5)
            {
                if (item->to_h < 10)
                {
                    my_need_enchant_to_h += (10 - item->to_h);
                }

                if (item->to_d < 10)
                {
                    my_need_enchant_to_d += (10 - item->to_d);
                }
            }
            else
            {
                if (item->to_h < 8)
                {
                    my_need_enchant_to_h += (8 - item->to_h);
                }

                if (item->to_d < 8)
                {
                    my_need_enchant_to_d += (8 - item->to_d);
                }
            }
        }
    } /* Ammo Power > 3 */

    /* Only enchant ammo if we have a good shooter,
     * otherwise, store the enchants in the home.
     */
    if (borg_skill[BI_AMMO_POWER] < 3) return;

    if ((borg_equips_item(act_firebrand, false) ||
        borg_spell_legal_fail(BRAND_AMMUNITION, 65)) &&
        item->iqty >= 5 &&
        /* Skip artifacts and ego-items */
        !item->art_idx &&
        !item->ego_idx &&
        item->ident &&
        item->tval == borg_skill[BI_AMMO_TVAL])
    {
        my_need_brand_weapon += 10L;
    }
}


  /*
   * Helper function -- notice the player equipment
   */
static void borg_notice_aux1(void)
{
    int         i, hold;
    const struct player_race* rb_ptr = player->race;
    const struct player_class* cb_ptr = player->class;

    int         extra_blows = 0;

    int         extra_shots = 0;
    int         extra_might = 0;
    int         my_num_fire;

    bitflag f[OF_SIZE];

    borg_item* item;

    /* Recalc some Variables */
    borg_skill[BI_ARMOR] = 0;
    borg_skill[BI_SPEED] = 110;
    borg_skill[BI_WEIGHT] = 0;

    /* Start with a single blow per turn */
    borg_skill[BI_BLOWS] = 1;

    /* Start with a single shot per turn */
    my_num_fire = 1;

    /* Reset the "ammo" attributes */
    borg_skill[BI_AMMO_COUNT] = 0;
    borg_skill[BI_AMMO_TVAL] = -1;
    borg_skill[BI_AMMO_SIDES] = 4;
    borg_skill[BI_AMMO_POWER] = 0;

    /* Reset the count of ID needed immediately */
    my_need_id = 0;

    /* Base infravision (purely racial) */
    borg_skill[BI_INFRA] = rb_ptr->infra;

    /* Base skill -- disarming */
    borg_skill[BI_DISP] = rb_ptr->r_skills[SKILL_DISARM_PHYS] + cb_ptr->c_skills[SKILL_DISARM_PHYS];
    borg_skill[BI_DISM] = rb_ptr->r_skills[SKILL_DISARM_MAGIC] + cb_ptr->c_skills[SKILL_DISARM_MAGIC];

    /* Base skill -- magic devices */
    borg_skill[BI_DEV] = rb_ptr->r_skills[SKILL_DEVICE] + cb_ptr->c_skills[SKILL_DEVICE];

    /* Base skill -- saving throw */
    borg_skill[BI_SAV] = rb_ptr->r_skills[SKILL_SAVE] + cb_ptr->c_skills[SKILL_SAVE];

    /* Base skill -- stealth */
    borg_skill[BI_STL] = rb_ptr->r_skills[SKILL_STEALTH] + cb_ptr->c_skills[SKILL_STEALTH];

    /* Base skill -- searching ability */
    borg_skill[BI_SRCH] = rb_ptr->r_skills[SKILL_SEARCH] + cb_ptr->c_skills[SKILL_SEARCH];

    /* Base skill -- combat (normal) */
    borg_skill[BI_THN] = rb_ptr->r_skills[SKILL_TO_HIT_MELEE] + cb_ptr->c_skills[SKILL_TO_HIT_MELEE];

    /* Base skill -- combat (shooting) */
    borg_skill[BI_THB] = rb_ptr->r_skills[SKILL_TO_HIT_BOW] + cb_ptr->c_skills[SKILL_TO_HIT_BOW];

    /* Base skill -- combat (throwing) */
    borg_skill[BI_THT] = rb_ptr->r_skills[SKILL_TO_HIT_THROW] + cb_ptr->c_skills[SKILL_TO_HIT_THROW];

    /* Affect Skill -- digging (STR) */
    borg_skill[BI_DIG] = rb_ptr->r_skills[SKILL_DIGGING] + cb_ptr->c_skills[SKILL_DIGGING];

    /** Racial Skills **/

    /* Extract the player flags */
    player_flags(player, f);

    /* Good flags */
    if (of_has(f, OF_SLOW_DIGEST)) borg_skill[BI_SDIG] = true;
    if (of_has(f, OF_FEATHER)) borg_skill[BI_FEATH] = true;
    if (of_has(f, OF_REGEN)) borg_skill[BI_REG] = true;
    if (of_has(f, OF_TELEPATHY)) borg_skill[BI_ESP] = true;
    if (of_has(f, OF_SEE_INVIS)) borg_skill[BI_SINV] = true;
    if (of_has(f, OF_FREE_ACT)) borg_skill[BI_FRACT] = true;
    if (of_has(f, OF_HOLD_LIFE)) borg_skill[BI_HLIFE] = true;

    /* Weird flags */

    /* Bad flags */
    if (of_has(f, OF_IMPACT)) borg_skill[BI_W_IMPACT] = true;
    if (of_has(f, OF_AGGRAVATE)) borg_skill[BI_CRSAGRV] = true;
    if (of_has(f, OF_AFRAID)) borg_skill[BI_CRSFEAR] = true;
    if (of_has(f, OF_DRAIN_EXP)) borg_skill[BI_CRSDRAIN_XP] = true;
    

    if (rb_ptr->el_info[ELEM_FIRE].res_level == -1) borg_skill[BI_CRSFVULN] = true;
    if (rb_ptr->el_info[ELEM_ACID].res_level == -1) borg_skill[BI_CRSAVULN] = true;
    if (rb_ptr->el_info[ELEM_COLD].res_level == -1) borg_skill[BI_CRSCVULN] = true;
    if (rb_ptr->el_info[ELEM_ELEC].res_level == -1) borg_skill[BI_CRSEVULN] = true;

    /* Immunity flags */
    if (rb_ptr->el_info[ELEM_FIRE].res_level == 3) borg_skill[BI_IFIRE] = true;
    if (rb_ptr->el_info[ELEM_ACID].res_level == 3) borg_skill[BI_IACID] = true;
    if (rb_ptr->el_info[ELEM_COLD].res_level == 3) borg_skill[BI_ICOLD] = true;
    if (rb_ptr->el_info[ELEM_ELEC].res_level == 3) borg_skill[BI_IELEC] = true;

    /* Resistance flags */
    if (rb_ptr->el_info[ELEM_FIRE].res_level > 0) borg_skill[BI_RACID] = true;
    if (rb_ptr->el_info[ELEM_ELEC].res_level > 0) borg_skill[BI_RELEC] = true;
    if (rb_ptr->el_info[ELEM_FIRE].res_level > 0) borg_skill[BI_RFIRE] = true;
    if (rb_ptr->el_info[ELEM_COLD].res_level > 0) borg_skill[BI_RCOLD] = true;
    if (rb_ptr->el_info[ELEM_POIS].res_level > 0) borg_skill[BI_RPOIS] = true;
    if (rb_ptr->el_info[ELEM_LIGHT].res_level > 0) borg_skill[BI_RLITE] = true;
    if (rb_ptr->el_info[ELEM_DARK].res_level > 0) borg_skill[BI_RDARK] = true;
    if (rb_ptr->el_info[ELEM_SOUND].res_level > 0) borg_skill[BI_RSND] = true;
    if (rb_ptr->el_info[ELEM_SHARD].res_level > 0) borg_skill[BI_RSHRD] = true;
    if (rb_ptr->el_info[ELEM_NEXUS].res_level > 0) borg_skill[BI_RNXUS] = true;
    if (rb_ptr->el_info[ELEM_NETHER].res_level > 0) borg_skill[BI_RNTHR] = true;
    if (rb_ptr->el_info[ELEM_CHAOS].res_level > 0) borg_skill[BI_RKAOS] = true;
    if (rb_ptr->el_info[ELEM_DISEN].res_level > 0) borg_skill[BI_RDIS] = true;
    if (rf_has(f, OF_PROT_FEAR)) borg_skill[BI_RFEAR] = true;
    if (rf_has(f, OF_PROT_BLIND)) borg_skill[BI_RBLIND] = true;
    if (rf_has(f, OF_PROT_CONF)) borg_skill[BI_RCONF] = true;

    /* Sustain flags */
    if (rf_has(f, OF_SUST_STR)) borg_skill[BI_SSTR] = true;
    if (rf_has(f, OF_SUST_INT)) borg_skill[BI_SINT] = true;
    if (rf_has(f, OF_SUST_WIS)) borg_skill[BI_SWIS] = true;
    if (rf_has(f, OF_SUST_DEX)) borg_skill[BI_SDEX] = true;
    if (rf_has(f, OF_SUST_CON)) borg_skill[BI_SCON] = true;

    /* I am pretty sure the CF_flags will be caught by the
     * code above when the player flags are checked
     */

     /* Clear the stat modifiers */
    for (i = 0; i < STAT_MAX; i++) my_stat_add[i] = 0;

    /* Scan the usable inventory */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
    {
        item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* total up the weight of the items */
        borg_skill[BI_WEIGHT] += item->weight * item->iqty;

        if (borg_item_note_needs_id(item)) my_need_id++;

        /* track first cursed item */
        if (!borg_skill[BI_FIRST_CURSED] && item->uncursable)
        {
            borg_skill[BI_WHERE_CURSED] |= BORG_EQUIP;
            borg_skill[BI_FIRST_CURSED] = i + 1;
        }

        /* Affect stats */
        my_stat_add[STAT_STR] += item->modifiers[OBJ_MOD_STR] * player->obj_k->modifiers[OBJ_MOD_STR];
        my_stat_add[STAT_INT] += item->modifiers[OBJ_MOD_INT] * player->obj_k->modifiers[OBJ_MOD_INT];
        my_stat_add[STAT_WIS] += item->modifiers[OBJ_MOD_WIS] * player->obj_k->modifiers[OBJ_MOD_WIS];
        my_stat_add[STAT_DEX] += item->modifiers[OBJ_MOD_DEX] * player->obj_k->modifiers[OBJ_MOD_DEX];
        my_stat_add[STAT_CON] += item->modifiers[OBJ_MOD_CON] * player->obj_k->modifiers[OBJ_MOD_CON];

        /* various slays */
        borg_skill[BI_WS_ANIMAL] = item->slays[RF_ANIMAL];
        borg_skill[BI_WS_EVIL] = item->slays[RF_EVIL];
        borg_skill[BI_WS_UNDEAD] = item->slays[RF_UNDEAD];
        borg_skill[BI_WS_DEMON] = item->slays[RF_DEMON];
        borg_skill[BI_WS_ORC] = item->slays[RF_ORC];
        borg_skill[BI_WS_TROLL] = item->slays[RF_TROLL];
        borg_skill[BI_WS_GIANT] = item->slays[RF_GIANT];
        borg_skill[BI_WS_DRAGON] = item->slays[RF_DRAGON];

        /* various brands */
        if (item->brands[ELEM_ACID])  borg_skill[BI_WB_ACID] = true;
        if (item->brands[ELEM_ELEC])  borg_skill[BI_WB_ELEC] = true;
        if (item->brands[ELEM_FIRE])  borg_skill[BI_WB_FIRE] = true;
        if (item->brands[ELEM_COLD])  borg_skill[BI_WB_COLD] = true;
        if (item->brands[ELEM_POIS])  borg_skill[BI_WB_POIS] = true;
        if (of_has(item->flags, OF_IMPACT))      borg_skill[BI_W_IMPACT] = true;

        /* Affect infravision */
        borg_skill[BI_INFRA] += item->modifiers[OBJ_MOD_INFRA];

        /* Affect stealth */
        borg_skill[BI_STL] += item->modifiers[OBJ_MOD_STEALTH];

        /* Affect searching ability (factor of five) */
        borg_skill[BI_SRCH] += (item->modifiers[OBJ_MOD_SEARCH] * 5);

        /* weapons of digging type get a special bonus */
        int dig = 0;
        if (item->tval == TV_DIGGING)
        {
            if (of_has(item->flags, OF_DIG_1))
                dig = 1;
            else if (of_has(item->flags, OF_DIG_2))
                dig = 2;
            else if (of_has(item->flags, OF_DIG_3))
                dig = 3;
        }
        dig += item->modifiers[OBJ_MOD_TUNNEL];

        /* Affect digging (factor of 20) */
        borg_skill[BI_DIG] += (dig * 20);

        /* Affect speed */
        borg_skill[BI_SPEED] += item->modifiers[OBJ_MOD_SPEED];

        /* Affect blows */
        extra_blows += item->modifiers[OBJ_MOD_BLOWS];

        /* Boost shots */
        extra_shots += item->modifiers[OBJ_MOD_SHOTS];

        /* Boost might */
        extra_might += item->modifiers[OBJ_MOD_MIGHT];

        /* Item makes player glow or has a light radius  */
        if (item->modifiers[OBJ_MOD_LIGHT])
        {
            /* Special case for Torches/Lantern of Brightness, they are not perm. */
            if (item->tval == TV_LIGHT &&
                (item->sval == sv_light_torch ||
                    item->sval == sv_light_lantern) &&
                !item->timeout) borg_skill[BI_LIGHT]++;
        }

        /* Boost mod moves */
        borg_skill[BI_MOD_MOVES] += item->modifiers[OBJ_MOD_MOVES];

        /* Boost damage reduction */
        borg_skill[BI_DAM_RED] += item->modifiers[OBJ_MOD_DAM_RED];

        /* Various flags */
        if (of_has(item->flags, OF_SLOW_DIGEST)) borg_skill[BI_SDIG] = true;
        if (of_has(item->flags, OF_AGGRAVATE)) borg_skill[BI_CRSAGRV] = true;
        if (of_has(item->flags, OF_IMPAIR_HP)) borg_skill[BI_CRSHPIMP] = true;
        if (of_has(item->flags, OF_IMPAIR_MANA)) borg_skill[BI_CRSMPIMP] = true;
        if (of_has(item->flags, OF_AFRAID)) borg_skill[BI_CRSFEAR] = true;
        if (of_has(item->flags, OF_DRAIN_EXP)) borg_skill[BI_CRSDRAIN_XP] = true;

        /* curses that don't have flags or stat changes that are tracked elsewhere */
        if (item->curses[BORG_CURSE_VULNERABILITY]) {
            borg_skill[BI_CRSAGRV] = true;
            borg_skill[BI_ARMOR] -= 50;
        }
        if (item->curses[BORG_CURSE_TELEPORTATION]) borg_skill[BI_CRSTELE] = true;
        if (item->curses[BORG_CURSE_DULLNESS]) {
            borg_skill[BI_CINT] -= 5;
            borg_skill[BI_CWIS] -= 5;
        }
        if (item->curses[BORG_CURSE_SICKLINESS]) {
            borg_skill[BI_CSTR] -= 5;
            borg_skill[BI_CDEX] -= 5;
            borg_skill[BI_CCON] -= 5;
        }
        if (item->curses[BORG_CURSE_ENVELOPING]) borg_skill[BI_CRSENVELOPING] = true;
        if (item->curses[BORG_CURSE_IRRITATION]) {
            borg_skill[BI_CRSAGRV] = true;
            borg_skill[BI_CRSIRRITATION] = true;
        }
        if (item->curses[BORG_CURSE_WEAKNESS]) {
            borg_skill[BI_CSTR] -= 10;
        }
        if (item->curses[BORG_CURSE_CLUMSINESS]) {
            borg_skill[BI_CSTR] -= 10;
        }
        if (item->curses[BORG_CURSE_SLOWNESS]) {
            borg_skill[BI_SPEED] = -5;
        }
        if (item->curses[BORG_CURSE_ANNOYANCE]) {
            borg_skill[BI_SPEED] = -10;
            borg_skill[BI_STL] = -10;
            borg_skill[BI_CRSAGRV] = true;
        }
        if (item->curses[BORG_CURSE_POISON]) borg_skill[BI_CRSPOIS] = true;
        if (item->curses[BORG_CURSE_SIREN]) borg_skill[BI_CRSSIREN] = true;
        if (item->curses[BORG_CURSE_HALLUCINATION]) borg_skill[BI_CRSHALU] = true;
        if (item->curses[BORG_CURSE_PARALYSIS]) borg_skill[BI_CRSPARA] = true;
        if (item->curses[BORG_CURSE_DEMON_SUMMON]) borg_skill[BI_CRSSDEM] = true;
        if (item->curses[BORG_CURSE_DRAGON_SUMMON]) borg_skill[BI_CRSSDRA] = true;
        if (item->curses[BORG_CURSE_UNDEAD_SUMMON]) borg_skill[BI_CRSSUND] = true;
        if (item->curses[BORG_CURSE_IMPAIR_MANA_RECOVERY]) borg_skill[BI_CRSMPIMP] = true;
        if (item->curses[BORG_CURSE_IMPAIR_HITPOINT_RECOVERY]) borg_skill[BI_CRSHPIMP] = true;
        if (item->curses[BORG_CURSE_COWARDICE]) borg_skill[BI_CRSFEAR] = true;
        if (item->curses[BORG_CURSE_STONE]) borg_skill[BI_CRSSTONE] = true;
        if (item->curses[BORG_CURSE_ANTI_TELEPORTATION]) borg_skill[BI_CRSNOTEL] = true;
        if (item->curses[BORG_CURSE_TREACHEROUS_WEAPON]) borg_skill[BI_CRSTWEP] = true;
        if (item->curses[BORG_CURSE_BURNING_UP]) {
            borg_skill[BI_CRSFVULN] = true;
            borg_skill[BI_RCOLD] = true;
        }
        if (item->curses[BORG_CURSE_CHILLED_TO_THE_BONE]) {
            borg_skill[BI_CRSCVULN] = true;
            borg_skill[BI_RFIRE] = true;
        }
        if (item->curses[BORG_CURSE_STEELSKIN]) borg_skill[BI_CRSSTEELSKIN] = true;
        if (item->curses[BORG_CURSE_AIR_SWING]) borg_skill[BI_CRSAIRSWING] = true;
        if (item->curses[BORG_CURSE_UNKNOWN]) borg_skill[BI_CRSUNKNO] = true;

        if (item->el_info[ELEM_FIRE].res_level == -1) borg_skill[BI_CRSFVULN] = true;
        if (item->el_info[ELEM_ACID].res_level == -1) borg_skill[BI_CRSAVULN] = true;
        if (item->el_info[ELEM_COLD].res_level == -1) borg_skill[BI_CRSCVULN] = true;
        if (item->el_info[ELEM_ELEC].res_level == -1) borg_skill[BI_CRSEVULN] = true;


        if (of_has(item->flags, OF_REGEN)) borg_skill[BI_REG] = true;
        if (of_has(item->flags, OF_TELEPATHY)) borg_skill[BI_ESP] = true;
        if (of_has(item->flags, OF_SEE_INVIS)) borg_skill[BI_SINV] = true;
        if (of_has(item->flags, OF_FEATHER)) borg_skill[BI_FEATH] = true;
        if (of_has(item->flags, OF_FREE_ACT)) borg_skill[BI_FRACT] = true;
        if (of_has(item->flags, OF_HOLD_LIFE)) borg_skill[BI_HLIFE] = true;
        if (of_has(item->flags, OF_PROT_CONF)) borg_skill[BI_RCONF] = true;
        if (of_has(item->flags, OF_PROT_BLIND)) borg_skill[BI_RBLIND] = true;

        /* assume all light artifacts give off light */
        if (item->tval == TV_LIGHT && item->art_idx)
            borg_skill[BI_LIGHT]++;

        /* Immunity flags */
        /* if you are immune you automaticly resist */
        if (item->el_info[ELEM_FIRE].res_level == 3)
        {
            borg_skill[BI_IFIRE] = true;
            borg_skill[BI_RFIRE] = true;
            borg_skill[BI_TRFIRE] = true;
        }
        if (item->el_info[ELEM_ACID].res_level == 3)
        {
            borg_skill[BI_IACID] = true;
            borg_skill[BI_RACID] = true;
            borg_skill[BI_TRACID] = true;
        }
        if (item->el_info[ELEM_COLD].res_level == 3)
        {
            borg_skill[BI_ICOLD] = true;
            borg_skill[BI_RCOLD] = true;
            borg_skill[BI_TRCOLD] = true;
        }
        if (item->el_info[ELEM_ELEC].res_level == 3)
        {
            borg_skill[BI_IELEC] = true;
            borg_skill[BI_RELEC] = true;
            borg_skill[BI_TRELEC] = true;
        }

        /* Resistance flags */
        if (item->el_info[ELEM_ACID].res_level > 0) borg_skill[BI_RACID] = true;
        if (item->el_info[ELEM_ELEC].res_level > 0) borg_skill[BI_RELEC] = true;
        if (item->el_info[ELEM_FIRE].res_level > 0) borg_skill[BI_RFIRE] = true;
        if (item->el_info[ELEM_COLD].res_level > 0) borg_skill[BI_RCOLD] = true;
        if (item->el_info[ELEM_POIS].res_level > 0) borg_skill[BI_RPOIS] = true;
        if (item->el_info[ELEM_SOUND].res_level > 0) borg_skill[BI_RSND] = true;
        if (item->el_info[ELEM_LIGHT].res_level > 0) borg_skill[BI_RLITE] = true;
        if (item->el_info[ELEM_DARK].res_level > 0) borg_skill[BI_RDARK] = true;
        if (item->el_info[ELEM_CHAOS].res_level > 0) borg_skill[BI_RKAOS] = true;
        if (item->el_info[ELEM_DISEN].res_level > 0) borg_skill[BI_RDIS] = true;
        if (item->el_info[ELEM_SHARD].res_level > 0) borg_skill[BI_RSHRD] = true;
        if (item->el_info[ELEM_NEXUS].res_level > 0) borg_skill[BI_RNXUS] = true;
        if (item->el_info[ELEM_NETHER].res_level > 0) borg_skill[BI_RNTHR] = true;

        /* Sustain flags */
        if (of_has(item->flags, OF_SUST_STR)) borg_skill[BI_SSTR] = true;
        if (of_has(item->flags, OF_SUST_INT)) borg_skill[BI_SINT] = true;
        if (of_has(item->flags, OF_SUST_WIS)) borg_skill[BI_SWIS] = true;
        if (of_has(item->flags, OF_SUST_DEX)) borg_skill[BI_SDEX] = true;
        if (of_has(item->flags, OF_SUST_CON)) borg_skill[BI_SCON] = true;


        /* Hack -- Net-zero The borg will miss read acid damaged items such as
         * Leather Gloves [2,-2] and falsely assume they help his power.
         * this hack rewrites the bonus to an extremely negative value
         * thus encouraging him to remove the non-helpful-non-harmful but
         * heavy-none-the-less item.
         */
        if ((!item->art_idx && !item->ego_idx) &&
            item->ac >= 1 && item->to_a + item->ac <= 0)
        {
            item->to_a = -20;
        }

        /* Modify the base armor class */
        borg_skill[BI_ARMOR] += item->ac;

        /* Apply the bonuses to armor class */
        borg_skill[BI_ARMOR] += item->to_a;

        /* Hack -- do not apply "weapon" bonuses */
        if (i == INVEN_WIELD) continue;

        /* Hack -- do not apply "bow" bonuses */
        if (i == INVEN_BOW) continue;

        /* Apply the bonuses to hit/damage */
        borg_skill[BI_TOHIT] += item->to_h;
        borg_skill[BI_TODAM] += item->to_d;
    }


    /* The borg needs to update his base stat points */
    for (i = 0; i < STAT_MAX; i++)
    {
        /* Cheat the exact number from the game.  This number is available to the player
         * on the extra term window.
         */
        my_stat_cur[i] = player->stat_cur[i];

        /* Max stat is the max that the cur stat ever is. */
        if (my_stat_cur[i] > my_stat_max[i])
            my_stat_max[i] = my_stat_cur[i];
    }


    /* Update "stats" */
    for (i = 0; i < STAT_MAX; i++)
    {
        int add, use, ind;

        add = my_stat_add[i];

        /* Modify the stats for race/class */
        add += (player->race->r_adj[i] + player->class->c_adj[i]);

        /* Extract the new "use_stat" value for the stat */
        use = modify_stat_value(my_stat_cur[i], add);

        /* Values: 3, ..., 17 */
        if (use <= 18) ind = (use - 3);

        /* Ranges: 18/00-18/09, ..., 18/210-18/219 */
        else if (use <= 18 + 219) ind = (15 + (use - 18) / 10);

        /* Range: 18/220+ */
        else ind = (37);

        /* Save the index */
        if (ind > 37)
            my_stat_ind[i] = 37;
        else
            my_stat_ind[i] = ind;
        borg_skill[BI_STR + i] = my_stat_ind[i];
        borg_skill[BI_CSTR + i] = borg_stat[i];
    }

    /* 'Mana' is actually the 'mana adjustment' */
    int spell_stat = borg_spell_stat();
    if (spell_stat >= 0)
    {
        borg_skill[BI_SP_ADJ] =
            ((borg_adj_mag_mana[my_stat_ind[spell_stat]] * borg_skill[BI_CLEVEL]) / 2);
        borg_skill[BI_FAIL1] = borg_adj_mag_stat[my_stat_ind[spell_stat]];
        borg_skill[BI_FAIL2] = borg_adj_mag_fail[my_stat_ind[spell_stat]];
    }


    /* Bloating slows the player down (a little) */
    if (borg_skill[BI_ISGORGED]) borg_skill[BI_SPEED] -= 10;


    /* Actual Modifier Bonuses */
    borg_skill[BI_ARMOR] += borg_adj_dex_ta[my_stat_ind[STAT_DEX]];
    borg_skill[BI_TODAM] += borg_adj_str_td[my_stat_ind[STAT_STR]];
    borg_skill[BI_TOHIT] += borg_adj_dex_th[my_stat_ind[STAT_DEX]];
    borg_skill[BI_TOHIT] += borg_adj_str_th[my_stat_ind[STAT_STR]];

    /* Obtain the "hold" value */
    hold = adj_str_hold[my_stat_ind[STAT_STR]];

    /* digging */
    borg_skill[BI_DIG] += borg_adj_str_dig[my_stat_ind[STAT_STR]];


    /** Examine the "current bow" **/
    item = &borg_items[INVEN_BOW];

    /* attacking with bare hands */
    if (item->iqty == 0)
    {
        item->ds = 0;
        item->dd = 0;
        item->to_d = 0;
        item->to_h = 0;
        item->weight = 0;
    }

    /* Real bonuses */
    borg_skill[BI_BTOHIT] = item->to_h;
    borg_skill[BI_BTODAM] = item->to_d;

    /* It is hard to carholdry a heavy bow */
    if (hold < item->weight / 10)
    {
        borg_skill[BI_HEAVYBOW] = true;
        /* Hard to wield a heavy bow */
        borg_skill[BI_TOHIT] += 2 * (hold - item->weight / 10);
    }

    /* Compute "extra shots" if needed */
    if (item->iqty && (hold >= item->weight / 10))
    {
        /* Take note of required "tval" for missiles */
        if (item->sval == sv_sling)
        {
            borg_skill[BI_AMMO_TVAL]  = TV_SHOT;
            borg_skill[BI_AMMO_SIDES] = 3;
            borg_skill[BI_AMMO_POWER] = 2;
        }
        else if (item->sval == sv_short_bow)
        {
            borg_skill[BI_AMMO_TVAL]  = TV_ARROW;
            borg_skill[BI_AMMO_SIDES] = 4;
            borg_skill[BI_AMMO_POWER] = 2;
        }
        else if (item->sval == sv_long_bow)
        {
            borg_skill[BI_AMMO_TVAL]  = TV_ARROW;
            borg_skill[BI_AMMO_SIDES] = 4;
            borg_skill[BI_AMMO_POWER] = 3;
        }
        else if (item->sval == sv_light_xbow)
        {
            borg_skill[BI_AMMO_TVAL]  = TV_BOLT;
            borg_skill[BI_AMMO_SIDES] = 5;
            borg_skill[BI_AMMO_POWER] = 3;
        }
        else if (item->sval == sv_heavy_xbow)
        {
            borg_skill[BI_AMMO_TVAL]  = TV_BOLT;
            borg_skill[BI_AMMO_SIDES] = 5;
            borg_skill[BI_AMMO_POWER] = 4;
        }

        /* Add in extra power */
        borg_skill[BI_AMMO_POWER] += extra_might;

        /* Hack -- Reward High Level Rangers using Bows */
        if (player_has(player, PF_FAST_SHOT) && (borg_skill[BI_AMMO_TVAL]  == TV_ARROW))
        {
            /* Extra shot at level 20 */
            if (borg_skill[BI_CLEVEL] >= 20) my_num_fire++;

            /* Extra shot at level 40 */
            if (borg_skill[BI_CLEVEL] >= 40) my_num_fire++;
        }

        /* Add in the "bonus shots" */
        my_num_fire += extra_shots;

        /* Require at least one shot */
        if (my_num_fire < 1) my_num_fire = 1;
    }
    borg_skill[BI_SHOTS] = my_num_fire;

    /* Calculate "average" damage per "normal" shot (times 2) */
    borg_skill[BI_BMAXDAM] = (borg_skill[BI_AMMO_SIDES] + borg_skill[BI_BTODAM]) * borg_skill[BI_AMMO_POWER];
    borg_skill[BI_BMAXDAM] *= borg_skill[BI_SHOTS];

    /* Examine the "main weapon" */
    item = &borg_items[INVEN_WIELD];

    /* attacking with bare hands */
    if (item->iqty == 0)
    {
        item->ds = 0;
        item->dd = 0;
        item->to_d = 0;
        item->to_h = 0;
        item->weight = 0;
    }

    /* Real values */
    borg_skill[BI_WTOHIT] = item->to_h;
    borg_skill[BI_WTODAM] = item->to_d;

    /* It is hard to hold a heavy weapon */
    if (hold < item->weight / 10)
    {
        borg_skill[BI_HEAVYWEPON] = true;

        /* Hard to wield a heavy weapon */
        borg_skill[BI_TOHIT] += 2 * (hold - item->weight / 10);
    }

    /* Normal weapons */
    if (item->iqty && (hold >= item->weight / 10))
    {
        /* calculate the number of blows */
        borg_skill[BI_BLOWS] = borg_calc_blows(extra_blows);

        /* Boost digging skill by weapon weight */
        borg_skill[BI_DIG] += (item->weight / 10);

    }

    /* Calculate "max" damage per "normal" blow  */
    /* and assume we can enchant up to +8 if borg_skill[BI_CLEVEL] > 25 */
    borg_skill[BI_WMAXDAM] =
        (item->dd * item->ds + borg_skill[BI_TODAM] + borg_skill[BI_WTODAM]);
    /* Calculate base damage, used to calculating slays */
    borg_skill[BI_WBASEDAM] =
        (item->dd * item->ds);

    /* Hack -- Reward High Level Warriors with Res Fear */
    if (player_has(player, PF_BRAVERY_30))
    {
        /* Resist fear at level 30 */
        if (borg_skill[BI_CLEVEL] >= 30) borg_skill[BI_RFEAR] = true;
    }

    /* Affect Skill -- stealth (bonus one) */
    borg_skill[BI_STL] += 1;

    /* Affect Skill -- disarming (DEX and INT) */
    borg_skill[BI_DISP] += borg_adj_dex_dis[my_stat_ind[STAT_DEX]];
    borg_skill[BI_DISM] += borg_adj_int_dis[my_stat_ind[STAT_INT]];

    /* Affect Skill -- magic devices (INT) */
    borg_skill[BI_DEV] += borg_adj_int_dev[my_stat_ind[STAT_INT]];

    /* Affect Skill -- saving throw (WIS) */
    borg_skill[BI_SAV] += borg_adj_wis_sav[my_stat_ind[STAT_WIS]];


    /* Affect Skill -- disarming (Level, by Class) */
    borg_skill[BI_DISP] += (cb_ptr->x_skills[SKILL_DISARM_PHYS] * borg_skill[BI_MAXCLEVEL] / 10);
    borg_skill[BI_DISM] += (cb_ptr->x_skills[SKILL_DISARM_MAGIC] * borg_skill[BI_MAXCLEVEL] / 10);

    /* Affect Skill -- magic devices (Level, by Class) */
    borg_skill[BI_DEV] += (cb_ptr->x_skills[SKILL_DEVICE] * borg_skill[BI_MAXCLEVEL] / 10);

    /* Affect Skill -- saving throw (Level, by Class) */
    borg_skill[BI_SAV] += (cb_ptr->x_skills[SKILL_SAVE] * borg_skill[BI_MAXCLEVEL] / 10);

    /* Affect Skill -- stealth (Level, by Class) */
    borg_skill[BI_STL] += (cb_ptr->x_skills[SKILL_STEALTH] * borg_skill[BI_MAXCLEVEL] / 10);

    /* Affect Skill -- search ability (Level, by Class) */
    borg_skill[BI_SRCH] += (cb_ptr->x_skills[SKILL_SEARCH] * borg_skill[BI_MAXCLEVEL] / 10);

    /* Affect Skill -- combat (normal) (Level, by Class) */
    borg_skill[BI_THN] += (cb_ptr->x_skills[SKILL_TO_HIT_MELEE] * borg_skill[BI_MAXCLEVEL] / 10);

    /* Affect Skill -- combat (shooting) (Level, by Class) */
    borg_skill[BI_THB] += (cb_ptr->x_skills[SKILL_TO_HIT_BOW] * borg_skill[BI_MAXCLEVEL] / 10);

    /* Affect Skill -- combat (throwing) (Level, by Class) */
    borg_skill[BI_THT] += (cb_ptr->x_skills[SKILL_TO_HIT_THROW] * borg_skill[BI_MAXCLEVEL] / 10);

    /* Limit Skill -- stealth from 0 to 30 */
    if (borg_skill[BI_STL] > 30) borg_skill[BI_STL] = 30;
    if (borg_skill[BI_STL] < 0) borg_skill[BI_STL] = 0;

    /* Limit Skill -- digging from 1 up */
    if (borg_skill[BI_DIG] < 1) borg_skill[BI_DIG] = 1;


    /*** Some penalties to consider ***/

    /* Fear from spell or effect or flag */
    if (borg_skill[BI_ISAFRAID] || borg_skill[BI_CRSFEAR])
    {
        borg_skill[BI_TOHIT] -= 20;
        borg_skill[BI_ARMOR] += 8;
        borg_skill[BI_DEV] = borg_skill[BI_DEV] * 95 / 100;
    }

    /* priest weapon penalty for non-blessed edged weapons */
    if (player_has(player, PF_BLESS_WEAPON) &&
        ((item->tval == TV_SWORD || item->tval == TV_POLEARM) &&
            !of_has(item->flags, OF_BLESSED)))
    {
        /* Reduce the real bonuses */
        borg_skill[BI_TOHIT] -= 2;
        borg_skill[BI_TODAM] -= 2;
    }

    /*** Count needed enchantment ***/

    /* Assume no enchantment needed */
    my_need_enchant_to_a = 0;
    my_need_enchant_to_h = 0;
    my_need_enchant_to_d = 0;
    my_need_brand_weapon = 0;

    /* Hack -- enchant all the equipment (weapons) */
    for (i = INVEN_WIELD; i <= INVEN_BOW; i++)
    {
        item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip "unknown" items */
        if (!item->ident) continue;

        /* Most classes store the enchants until they get
         * a 3x shooter (like a long bow).
         * --Important: Also look in borg7.c for the enchanting.
         * --We do not want the bow enchanted by mistake.
         */
        if (i == INVEN_BOW &&  /* bow */
            borg_skill[BI_AMMO_POWER] < 3 && /* 3x shooter */
            (!item->art_idx && !item->ego_idx)) /* Not Ego or Artifact */
            continue;

        /* Enchant all weapons (to hit) */
        if ((borg_spell_legal_fail(ENCHANT_WEAPON, 65) ||
            amt_enchant_weapon >= 1))
        {
            if (item->to_h < borg_cfg[BORG_ENCHANT_LIMIT])
            {
                my_need_enchant_to_h += (borg_cfg[BORG_ENCHANT_LIMIT] - item->to_h);
            }

            /* Enchant all weapons (to damage) */
            if (item->to_d < borg_cfg[BORG_ENCHANT_LIMIT])
            {
                my_need_enchant_to_d += (borg_cfg[BORG_ENCHANT_LIMIT] - item->to_d);
            }
        }
        else /* I dont have the spell or *enchant* */
        {
            if (item->to_h < 8)
            {
                my_need_enchant_to_h += (8 - item->to_h);
            }

            /* Enchant all weapons (to damage) */
            if (item->to_d < 8)
            {
                my_need_enchant_to_d += (8 - item->to_d);
            }
        }
    }

    /* Hack -- enchant all the equipment (armor) */
    for (i = INVEN_BODY; i <= INVEN_FEET; i++)
    {
        item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip "unknown" items */
        if (!item->ident) continue;

        /* Note need for enchantment */
        if (borg_spell_legal_fail(ENCHANT_ARMOUR, 65) ||
            amt_enchant_armor >= 1)
        {
            if (item->to_a < borg_cfg[BORG_ENCHANT_LIMIT])
            {
                my_need_enchant_to_a += (borg_cfg[BORG_ENCHANT_LIMIT] - item->to_a);
            }
        }
        else
        {
            if (item->to_a < 8)
            {
                my_need_enchant_to_a += (8 - item->to_a);
            }
        }
    }

    /* Examine the lite */
    item = &borg_items[INVEN_LIGHT];

    /* Assume normal lite radius */
    borg_skill[BI_CURLITE] = 0;

    /* Glowing player has light */
    if (borg_skill[BI_LIGHT]) borg_skill[BI_CURLITE] = borg_skill[BI_LIGHT];

    /* Lite */
    if (item->tval == TV_LIGHT)
    {
        if (item->timeout || of_has(item->flags, OF_NO_FUEL))
        {
            if (of_has(item->flags, OF_LIGHT_2))
            {
                borg_skill[BI_CURLITE] = borg_skill[BI_CURLITE] + 2;
            }
            else if (of_has(item->flags, OF_LIGHT_3))
            {
                borg_skill[BI_CURLITE] = borg_skill[BI_CURLITE] + 3;
            }
        }
    }

    borg_skill[BI_CURLITE] += item->modifiers[OBJ_MOD_LIGHT];

    /* Special way to handle See Inv */
    if (borg_see_inv >= 1) borg_skill[BI_SINV] = true;
    if (borg_skill[BI_CDEPTH] == 0 && /* only in town.  Allow him to recall down */
        borg_spell_legal(SENSE_INVISIBLE)) borg_skill[BI_SINV] = true;

    /* Very special handling of Free Action.
     * If the person has perfect Savings throw, he can be
     * considered ok on Free Action.  This can free up an
     * equipment slot.
     */
    if (borg_skill[BI_SAV] >= 100) borg_skill[BI_FRACT] = true;

    /* Special case for RBlindness.  Perfect saves and the
     * resistances for light and dark are good enough for RBlind
     */
    if (borg_skill[BI_SAV] >= 100 && borg_skill[BI_RDARK] &&
        borg_skill[BI_RLITE]) borg_skill[BI_RBLIND] = true;

    /*** Quiver needs to be evaluated ***/

    /* Hack -- ignore invalid missiles */
    for (i = QUIVER_START; i < QUIVER_END; i++)
        borg_notice_aux_ammo(i);
}


/*
 * Helper function -- notice the player inventory
 */
static void borg_notice_aux2(void)
{
    int i;

    borg_item* item;


    /*** Reset counters ***/


    /* Reset basic */
    amt_food_lowcal = 0;
    amt_food_hical = 0;

    /* Reset healing */
    amt_slow_poison = 0;
    amt_cure_confusion = 0;
    amt_cure_blind = 0;

    /* Reset stat potions */
    for (i = 0; i < 6; i++) amt_inc_stat[i] = 0;

    /* Reset books */
    for (i = 0; i < 9; i++) amt_book[i] = 0;

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

    /* Reset enchantment */
    amt_enchant_to_a = 0;
    amt_enchant_to_d = 0;
    amt_enchant_to_h = 0;

    amt_brand_weapon = 0;
    amt_enchant_weapon = 0;
    amt_enchant_armor = 0;

    /* Reset number of Ego items needing *ID* */
    amt_ego = 0;

    /*** Process the inventory ***/

    /* Scan the inventory */
    for (i = 0; i < PACK_SLOTS; i++)
    {
        item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty)
        {
            borg_skill[BI_EMPTY]++;
            continue;
        }

        /* special case for ammo outside the quiver. */
        /* this happens when we are deciding what to buy so items */
        /* are put in empty slots */
        if (borg_is_ammo(item->tval))
        {
            borg_notice_aux_ammo(i);
            continue;
        }

        /* total up the weight of the items */
        borg_skill[BI_WEIGHT] += item->weight * item->iqty;

        /* Does the borg need to get an ID for it? */
        if (borg_item_note_needs_id(item)) my_need_id++;

        /* Hack -- skip un-aware items */
        if (!item->kind) continue;

        /* count up the items on the borg (do not count artifacts  */
        /* that are not being wielded) */
        borg_has[item->kind] += item->iqty;

        /* track first cursed item */
        if (item->uncursable)
        {
            borg_skill[BI_WHERE_CURSED] |= BORG_INVEN;
            if (!borg_skill[BI_FIRST_CURSED])
                borg_skill[BI_FIRST_CURSED] = i + 1;
        }

        /* Analyze the item */
        switch (item->tval)
        {
            /* Books */
        case TV_MAGIC_BOOK:
        case TV_PRAYER_BOOK:
        case TV_NATURE_BOOK:
        case TV_SHADOW_BOOK:
        case TV_OTHER_BOOK:
        /* Skip incorrect books (if we can browse this book, it is good) */
        if (!obj_kind_can_browse(&k_info[item->kind])) break;
        /* Count the books */
        amt_book[borg_get_book_num(item->sval)] += item->iqty;
        break;


        /* Food */
        case TV_MUSHROOM:
        if (item->sval == sv_mush_purging ||
            item->sval == sv_mush_restoring ||
            item->sval == sv_mush_cure_mind)
        {
            if (borg_cfg[BORG_MUNCHKIN_START] && 
                borg_skill[BI_MAXCLEVEL] < borg_cfg[BORG_MUNCHKIN_LEVEL])
            {
                break;
            }
        }
        if (item->sval == sv_mush_second_sight ||
            item->sval == sv_mush_emergency ||
            item->sval == sv_mush_terror ||
            item->sval == sv_mush_stoneskin ||
            item->sval == sv_mush_debility ||
            item->sval == sv_mush_sprinting)
            if (borg_cfg[BORG_MUNCHKIN_START] && 
                borg_skill[BI_MAXCLEVEL] >= borg_cfg[BORG_MUNCHKIN_LEVEL])
            {
                borg_skill[BI_ASHROOM] += item->iqty;
            }
        /* fall through */
        case TV_FOOD:
        /* Analyze */
        {
            /* unknown types */
            if (!item->kind)
                break;

            /* check for food that hurts us */
            if (borg_obj_has_effect(item->kind, EF_CRUNCH, -1) ||
                borg_obj_has_effect(item->kind, EF_TIMED_INC, TMD_CONFUSED))
                break;

            /* check for food that gives nutrition */
            if (item->tval == TV_MUSHROOM)
            {
                /* mushrooms that increase nutrition are low effect */
                if (borg_obj_has_effect(item->kind, EF_NOURISH, 0))
                    amt_food_lowcal += item->iqty;
            }
            else /* TV_FOOD */
            {
                if (item->sval == sv_food_apple ||
                    item->sval == sv_food_handful ||
                    item->sval == sv_food_slime_mold ||
                    item->sval == sv_food_pint ||
                    item->sval == sv_food_sip)
                {
                    amt_food_lowcal += item->iqty;
                } 
                else if (item->sval == sv_food_ration ||
                    item->sval == sv_food_slice ||
                    item->sval == sv_food_honey_cake ||
                    item->sval == sv_food_waybread ||
                    item->sval == sv_food_draught )
                    amt_food_hical += item->iqty;
            }

            /* check for food that does stuff */
            if (borg_obj_has_effect(item->kind, EF_CURE, TMD_POISONED))
                borg_skill[BI_ACUREPOIS] += item->iqty;
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
            borg_skill[BI_AHEAL] += item->iqty;
        else if (item->sval == sv_potion_star_healing)
            borg_skill[BI_AEZHEAL] += item->iqty;
        else if (item->sval == sv_potion_life)
            borg_skill[BI_ALIFE] += item->iqty;
        else if (item->sval == sv_potion_cure_critical)
            borg_skill[BI_ACCW] += item->iqty;
        else if (item->sval == sv_potion_cure_serious)
            borg_skill[BI_ACSW] += item->iqty;
        else if (item->sval == sv_potion_cure_light)
            borg_skill[BI_ACLW] += item->iqty;
        else if (item->sval == sv_potion_cure_poison)
            borg_skill[BI_ACUREPOIS] += item->iqty;
        else if (item->sval == sv_potion_resist_heat)
            borg_skill[BI_ARESHEAT] += item->iqty;
        else if (item->sval == sv_potion_resist_cold)
            borg_skill[BI_ARESCOLD] += item->iqty;
        else if (item->sval == sv_potion_resist_pois)
            borg_skill[BI_ARESPOIS] += item->iqty;
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
        else if (item->sval == sv_potion_inc_all)
        {
            amt_inc_stat[STAT_STR] += item->iqty;
            amt_inc_stat[STAT_INT] += item->iqty;
            amt_inc_stat[STAT_WIS] += item->iqty;
            amt_inc_stat[STAT_DEX] += item->iqty;
            amt_inc_stat[STAT_CON] += item->iqty;
        }
        else if (item->sval == sv_potion_restore_life)
            amt_fix_exp += item->iqty;
        else if (item->sval == sv_potion_speed)
            borg_skill[BI_ASPEED] += item->iqty;
        break;



        /* Scrolls */
        case TV_SCROLL:

        if (item->sval == sv_scroll_identify)
            borg_skill[BI_AID] += item->iqty;
        else if (item->sval == sv_scroll_recharging)
            borg_skill[BI_ARECHARGE] += item->iqty;
        else if (item->sval == sv_scroll_phase_door)
            borg_skill[BI_APHASE] += item->iqty;
        else if (item->sval == sv_scroll_teleport)
            borg_skill[BI_ATELEPORT] += item->iqty;
        else if (item->sval == sv_scroll_word_of_recall)
            borg_skill[BI_RECALL] += item->iqty;
        else if (item->sval == sv_scroll_enchant_armor)
            amt_enchant_to_a += item->iqty;
        else if (item->sval == sv_scroll_enchant_weapon_to_hit)
            amt_enchant_to_h += item->iqty;
        else if (item->sval == sv_scroll_enchant_weapon_to_dam)
            amt_enchant_to_d += item->iqty;
        else if (item->sval == sv_scroll_star_enchant_weapon)
            amt_enchant_weapon += item->iqty;
        else if (item->sval == sv_scroll_protection_from_evil)
            borg_skill[BI_APFE] += item->iqty;
        else if (item->sval == sv_scroll_star_enchant_armor)
            amt_enchant_armor += item->iqty;
        else if (item->sval == sv_scroll_rune_of_protection)
            borg_skill[BI_AGLYPH] += item->iqty;
        else if (item->sval == sv_scroll_teleport_level)
        {
            borg_skill[BI_ATELEPORTLVL] += item->iqty;
            borg_skill[BI_ATELEPORT] += 1;
        }
        else if (item->sval == sv_scroll_mass_banishment)
            borg_skill[BI_AMASSBAN] += item->iqty;
        break;


        /* Rods */
        case TV_ROD:

        /* Analyze */
        if (item->sval == sv_rod_recall)
        {
            /* Don't count on it if I suck at activations */
            if (borg_activate_failure(item->tval, item->sval) < 500)
            {
                borg_skill[BI_RECALL] += item->iqty * 100;
            }
            else
            {
                borg_skill[BI_RECALL] += item->iqty;
            }
        }
        else if (item->sval == sv_rod_detection)
        {
            borg_skill[BI_ADETTRAP] += item->iqty * 100;
            borg_skill[BI_ADETDOOR] += item->iqty * 100;
            borg_skill[BI_ADETEVIL] += item->iqty * 100;
        }
        else if (item->sval == sv_rod_illumination)
            borg_skill[BI_ALITE] += item->iqty * 100;
        else if (item->sval == sv_rod_speed)
        {
            /* Don't count on it if I suck at activations */
            if (borg_activate_failure(item->tval, item->sval) < 500)
            {
                borg_skill[BI_ASPEED] += item->iqty * 100;
            }
            else
            {
                borg_skill[BI_ASPEED] += item->iqty;
            }
        }
        else if (item->sval == sv_rod_mapping)
            borg_skill[BI_AMAGICMAP] += item->iqty * 100;
        else if (item->sval == sv_rod_healing)
        {
            /* only +2 per rod because of long charge time. */
            /* Don't count on it if I suck at activations */
            if (borg_activate_failure(item->tval, item->sval) < 500)
            {
                borg_skill[BI_AHEAL] += item->iqty * 3;
            }
            else
            {
                borg_skill[BI_AHEAL] += item->iqty + 1;
            }
        }
        else if (item->sval == sv_rod_light ||
            item->sval == sv_rod_fire_bolt ||
            item->sval == sv_rod_elec_bolt ||
            item->sval == sv_rod_cold_bolt ||
            item->sval == sv_rod_acid_bolt)
        {
            borg_skill[BI_AROD1] += item->iqty;
        }
        else if (item->sval == sv_rod_drain_life ||
            item->sval == sv_rod_fire_ball ||
            item->sval == sv_rod_elec_ball ||
            item->sval == sv_rod_cold_ball ||
            item->sval == sv_rod_acid_ball)
        {
            borg_skill[BI_AROD2] += item->iqty;
        }
        break;

        /* Wands */
        case TV_WAND:

        /* Analyze each */
        if (item->sval == sv_wand_teleport_away)
        {
            borg_skill[BI_ATPORTOTHER] += item->pval;
        }

        if (item->sval == sv_wand_stinking_cloud &&
            borg_skill[BI_MAXDEPTH] < 30)
        {
            amt_cool_wand += item->pval;
        }

        if (item->sval == sv_wand_magic_missile &&
            borg_skill[BI_MAXDEPTH] < 30)
        {
            amt_cool_wand += item->pval;
        }

        if (item->sval == sv_wand_annihilation)
        {
            amt_cool_wand += item->pval;
        }

        break;


        /* Staffs */
        case TV_STAFF:
        /* Analyze */
        if (item->sval == sv_staff_teleportation)
        {
            if (borg_skill[BI_MAXDEPTH] <= 95)
            {
                borg_skill[BI_AESCAPE] += (item->iqty);
                if (borg_activate_failure(item->tval, item->sval) < 500)
                {
                    borg_skill[BI_AESCAPE] += item->pval;
                }
            }
        }
        else if (item->sval == sv_staff_speed)
        {
            if (borg_skill[BI_MAXDEPTH] <= 95) borg_skill[BI_ASPEED] += item->pval;
        }
        else if (item->sval == sv_staff_healing)
            borg_skill[BI_AHEAL] += item->pval;
        else if (item->sval == sv_staff_the_magi)
            borg_skill[BI_ASTFMAGI] += item->pval;
        else if (item->sval == sv_staff_destruction)
            borg_skill[BI_ASTFDEST] += item->pval;
        else if (item->sval == sv_staff_power)
            amt_cool_staff += item->iqty;
        else if (item->sval == sv_staff_holiness)
        {
            amt_cool_staff += item->iqty;
            borg_skill[BI_AHEAL] += item->pval;
        }

        break;


        /* Flasks */
        case TV_FLASK:

        /* Use as fuel if we equip a lantern */
        if (borg_items[INVEN_LIGHT].sval == sv_light_lantern) borg_skill[BI_AFUEL] += item->iqty;

        break;


        /* Torches */
        case TV_LIGHT:

        /* Use as fuel if it is a torch and we carry a torch */
        if ((item->sval == sv_light_torch && item->timeout >= 1) &&
            (borg_items[INVEN_LIGHT].sval == sv_light_torch) && borg_items[INVEN_LIGHT].iqty)
        {
            borg_skill[BI_AFUEL] += item->iqty;
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
        if (item->value <= 0) break;
        if (item->cursed) break;

        /* Do not carry if weak, won't be able to dig anyway */
        if (borg_skill[BI_DIG] < BORG_DIG) break;

        amt_digger += item->iqty;
        break;

        }
    }

    /* flasks of oil are ammo at low levels */
    if (borg_has[kv_flask_oil] && borg_skill[BI_CLEVEL] < 15)
    {
        /* only count the first 15 */
        if (borg_has[kv_flask_oil] < 15)
            borg_skill[BI_AMISSILES] += borg_has[kv_flask_oil];
        else
            borg_skill[BI_AMISSILES] += 15;
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
    if (borg_spell_legal_fail(REMOVE_HUNGER, 80) || borg_spell_legal_fail(HERBAL_CURING, 80)) /* VAMPIRE_STRIKE? */
    {
        borg_skill[BI_FOOD] += 1000;
    }

    /* Handle "identify" -> infinite identifies */
    if (borg_spell_legal(IDENTIFY_RUNE))
    {
        borg_skill[BI_AID] += 1000;
    }

    /* Handle "detect traps" */
    if (borg_spell_legal(FIND_TRAPS_DOORS_STAIRS) || borg_spell_legal(DETECTION))
    {
        borg_skill[BI_ADETTRAP] = 1000;
    }

    /* Handle "detect evil & monsters" */
    if (borg_spell_legal(REVEAL_MONSTERS) ||
        borg_spell_legal(DETECT_LIFE) ||
        borg_spell_legal(DETECT_EVIL) ||
        borg_spell_legal(READ_MINDS) ||
        borg_spell_legal(DETECT_MONSTERS) ||
        borg_spell_legal(SEEK_BATTLE))
    {
        borg_skill[BI_ADETEVIL] = 1000;
    }

    /* Handle DETECTION */
    if (borg_spell_legal(DETECTION) ||
        borg_equips_item(act_enlightenment, false) ||
        borg_equips_item(act_clairvoyance, false))
    {
        borg_skill[BI_ADETDOOR] = 1000;
        borg_skill[BI_ADETTRAP] = 1000;
        borg_skill[BI_ADETEVIL] = 1000;

    }

    /* Handle "See Invisible" in a special way. */
    if (borg_spell_legal(SENSE_INVISIBLE))
    {
        borg_skill[BI_DINV] = true;
    }

    /* Handle "magic mapping" */
    if (borg_spell_legal(SENSE_SURROUNDINGS) ||
        borg_equips_item(act_detect_all, false) ||
        borg_equips_item(act_mapping, false))
    {
        borg_skill[BI_ADETDOOR] = 1000;
        borg_skill[BI_ADETTRAP] = 1000;
        borg_skill[BI_AMAGICMAP] = 1000;
    }

    /* Handle "call lite" */
    if (borg_spell_legal(LIGHT_ROOM) ||
        borg_equips_item(act_light, false) ||
        borg_equips_item(act_illumination, false) ||
        borg_spell_legal(CALL_LIGHT))
    {
        borg_skill[BI_ALITE] += 1000;
    }

    /* Handle PROTECTION_FROM_EVIL */
    if (borg_spell_legal(PROTECTION_FROM_EVIL) ||
        borg_equips_item(act_protevil, false) ||
        borg_has[kv_staff_holiness] ||
        borg_equips_item(act_staff_holy, false))
    {
        borg_skill[BI_APFE] += 1000;
    }

    /* Handle "rune of protection" glyph" */
    if (borg_spell_legal(GLYPH_OF_WARDING) ||
        borg_equips_item(act_glyph, false))
    {
        borg_skill[BI_AGLYPH] += 1000;
    }

    /* Handle "detect traps/doors" */
    if (borg_spell_legal(FIND_TRAPS_DOORS_STAIRS))
    {
        borg_skill[BI_ADETDOOR] = 1000;
        borg_skill[BI_ADETTRAP] = 1000;
    }

    /* Handle ENCHANT_WEAPON */
    if (borg_spell_legal_fail(ENCHANT_WEAPON, 65))
    {
        amt_enchant_to_h += 1000;
        amt_enchant_to_d += 1000;
        amt_enchant_weapon += 1000;
    }

    /* Handle "Brand Weapon (bolts)" */
    if (borg_equips_item(act_firebrand, false) ||
        borg_spell_legal_fail(BRAND_AMMUNITION, 65))
    {
        amt_brand_weapon += 1000;
    }

    /* Handle "enchant armor" */
    if (borg_spell_legal_fail(ENCHANT_ARMOUR, 65))
    {
        amt_enchant_to_a += 1000;
        amt_enchant_armor += 1000;
    }

    /* Handle Diggers (stone to mud) */
    if (borg_spell_legal_fail(TURN_STONE_TO_MUD, 40) ||
        borg_spell_legal_fail(SHATTER_STONE, 40) ||
        borg_equips_item(act_stone_to_mud, false) ||
        borg_equips_ring(sv_ring_digging))
    {
        amt_digger += 1;
    }

    /* Handle recall */
    if (borg_spell_legal_fail(WORD_OF_RECALL, 40) ||
        (borg_skill[BI_CDEPTH] == 100 && borg_spell_legal(WORD_OF_RECALL)))
    {
        borg_skill[BI_RECALL] += 1000;
    }
    if (borg_equips_item(act_recall, false))
    {
        borg_skill[BI_RECALL] += 1;
    }

    /* Handle teleport_level */
    if (borg_spell_legal_fail(TELEPORT_LEVEL, 20))
    {
        borg_skill[BI_ATELEPORTLVL] += 1000;
    }

    /* Handle PhaseDoor spell carefully */
    if (borg_spell_legal_fail(PHASE_DOOR, 3))
    {
        borg_skill[BI_APHASE] += 1000;
    }
    if (borg_equips_item(act_tele_phase, false))
    {
        borg_skill[BI_APHASE] += 1;
    }

    /* Handle teleport spell carefully */
    if (borg_spell_legal_fail(TELEPORT_SELF, 1) ||
        borg_spell_legal_fail(PORTAL, 1) ||
        borg_spell_legal_fail(SHADOW_SHIFT, 1) ||
        borg_spell_legal_fail(DIMENSION_DOOR, 1))
    {
        borg_skill[BI_ATELEPORT] += 1000;
    }
    if (borg_equips_item(act_tele_long, false))
    {
        borg_skill[BI_AESCAPE] += 1;
        borg_skill[BI_ATELEPORT] += 1;
    }

    /* Handle teleport away */
    if (borg_spell_legal_fail(TELEPORT_OTHER, 40))
    {
        borg_skill[BI_ATPORTOTHER] += 1000;
    }

    /* Handle Holy Word prayer just to see if legal */
    if (borg_spell_legal(HOLY_WORD))
    {
        borg_skill[BI_AHWORD] += 1000;
    }

    /* speed spells HASTE*/
    if (borg_spell_legal(HASTE_SELF) ||
        borg_equips_item(act_haste, false) ||
        borg_equips_item(act_haste1, false) ||
        borg_equips_item(act_haste2, false))
    {
        borg_skill[BI_ASPEED] += 1000;
    }

    /* Handle "cure light wounds" */
    if (borg_equips_item(act_cure_light, false))
    {
        borg_skill[BI_ACLW] += 1000;
    }


    /* Handle "cure serious wounds" */
    if (borg_equips_item(act_cure_serious, false))
    {
        borg_skill[BI_ACSW] += 1000;
    }

    /* Handle "cure critical wounds" */
    if (borg_equips_item(act_cure_critical, false))
    {
        borg_skill[BI_ACCW] += 1000;
    }

    /* Handle "heal" */
    if (borg_equips_item(act_cure_full, false) ||
        borg_equips_item(act_cure_full2, false) ||
        borg_equips_item(act_cure_nonorlybig, false) ||
        borg_equips_item(act_heal1, false) ||
        borg_equips_item(act_heal2, false) ||
        borg_equips_item(act_heal3, false) ||
        borg_spell_legal(HEALING))
    {
        borg_skill[BI_AHEAL] += 1000;
    }

    /* Handle "fix exp" */
    if (borg_equips_item(act_cure_nonorlybig, false) ||
        borg_equips_item(act_restore_exp, false) ||
        borg_equips_item(act_restore_st_lev, false) ||
        borg_equips_item(act_restore_life, false))
    {
        amt_fix_exp += 1000;
    }

    /* Handle REMEMBRANCE -- is just as good as Hold Life */
    if (borg_spell_legal(REMEMBRANCE) ||
        borg_equips_item(act_cure_nonorlybig, false) ||
        borg_equips_item(act_restore_exp, false) ||
        borg_equips_item(act_restore_st_lev, false) ||
        borg_equips_item(act_restore_life, false))
    {
        borg_skill[BI_HLIFE] = true;
    }

    /* Handle "recharge" */
    if (borg_equips_item(act_recharge, false) ||
        borg_spell_legal(RECHARGING))
    {
        borg_skill[BI_ARECHARGE] += 1000;
    }

    /*** Process the Needs ***/

    /* No need for fuel if we know it doesn't need it */
    if (of_has(borg_items[INVEN_LIGHT].flags, OF_NO_FUEL))
        borg_skill[BI_AFUEL] += 1000;

    /* No need to *buy* stat increase potions */
    if (my_stat_cur[STAT_STR] >= (18 + 100) + 10 * (player->race->r_adj[STAT_STR] + player->class->c_adj[STAT_STR]))
        amt_add_stat[STAT_STR] += 1000;

    if (my_stat_cur[STAT_INT] >= (18 + 100) + 10 * (player->race->r_adj[STAT_INT] + player->class->c_adj[STAT_INT]))
        amt_add_stat[STAT_INT] += 1000;

    if (my_stat_cur[STAT_WIS] >= (18 + 100) + 10 * (player->race->r_adj[STAT_WIS] + player->class->c_adj[STAT_WIS]))
        amt_add_stat[STAT_WIS] += 1000;

    if (my_stat_cur[STAT_DEX] >= (18 + 100) + 10 * (player->race->r_adj[STAT_DEX] + player->class->c_adj[STAT_DEX]))
        amt_add_stat[STAT_DEX] += 1000;

    if (my_stat_cur[STAT_CON] >= (18 + 100) + 10 * (player->race->r_adj[STAT_CON] + player->class->c_adj[STAT_CON]))
        amt_add_stat[STAT_CON] += 1000;

    /* No need for experience repair */
    if (!borg_skill[BI_ISFIXEXP]) amt_fix_exp += 1000;

    /* Correct the high and low calorie foods */
    borg_skill[BI_FOOD] += amt_food_hical;
    if (amt_food_hical <= 3) borg_skill[BI_FOOD] += amt_food_lowcal;

    /* If weak, do not count food spells */
    if (borg_skill[BI_ISWEAK] && (borg_skill[BI_FOOD] >= 1000))
        borg_skill[BI_FOOD] -= 1000;
}

/**
 * Calculate the blows a player would get.
 *
 * copied and ajusted from player-calcs.c
 */
int borg_calc_blows(int extra_blows)
{
    int blows;
    int str_index, dex_index;
    int div;
    int blow_energy;

    int weight = borg_items[INVEN_WIELD].weight;
    int min_weight = player->class->min_weight;

    /* Enforce a minimum "weight" (tenth pounds) */
    div = (weight < min_weight) ? min_weight : weight;

    /* Get the strength vs weight */
    str_index = adj_str_blow[my_stat_ind[STAT_STR]] *
        player->class->att_multiply / div;

    /* Maximal value */
    if (str_index > 11) str_index = 11;

    /* Index by dexterity */
    dex_index = MIN(borg_adj_dex_blow[my_stat_ind[STAT_DEX]], 11);

    /* Use the blows table to get energy per blow */
    blow_energy = borg_blows_table[str_index][dex_index];

    blows = MIN((10000 / blow_energy), (100 * player->class->max_attacks));

    /* Require at least one blow, two for O-combat */
    return (MAX(blows + (100 * extra_blows),
        OPT(player, birth_percent_damage) ? 200 : 100)) / 100;
}

/*
 * for swap items for now lump all curses together as "bad"
 */
static bool borg_has_bad_curse(borg_item* item)
{
    if (item->curses[BORG_CURSE_TELEPORTATION] ||
        item->curses[BORG_CURSE_POISON] ||
        item->curses[BORG_CURSE_SIREN] ||
        item->curses[BORG_CURSE_HALLUCINATION] ||
        item->curses[BORG_CURSE_PARALYSIS] ||
        item->curses[BORG_CURSE_DEMON_SUMMON] ||
        item->curses[BORG_CURSE_DRAGON_SUMMON] ||
        item->curses[BORG_CURSE_UNDEAD_SUMMON] ||
        item->curses[BORG_CURSE_STONE] ||
        item->curses[BORG_CURSE_ANTI_TELEPORTATION] ||
        item->curses[BORG_CURSE_TREACHEROUS_WEAPON] ||
        item->curses[BORG_CURSE_UNKNOWN])
        return true;
    return false;
}

/*
 * Helper function -- notice the player swap weapon
 */
static void borg_notice_weapon_swap(void)
{
    int i;
    int b_i = -1;

    int32_t v = -1;
    int32_t b_v = -1;

    int dam, damage;
    borg_item* item;

    weapon_swap = 0;

    /*** Process the inventory ***/
    for (i = 0; i < z_info->pack_size; i++)
    {
        item = &borg_items[i];

        /* reset counter */
        v = -1L;
        dam = 0;
        damage = 0;

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Hack -- skip un-aware items */
        if (!item->kind) continue;

        /* Skip non-wearable items */
        if (borg_slot(item->tval, item->sval) == -1) continue;

        /* Dont carry swaps until dlevel 50.  They are heavy.
           Unless the item is a digger, then carry it */
        if (borg_skill[BI_MAXDEPTH] < 50 && item->tval != TV_DIGGING) continue;

        /* priest weapon penalty for non-blessed edged weapons */
        if (player_has(player, PF_BLESS_WEAPON) &&
            (item->tval == TV_SWORD || item->tval == TV_POLEARM) &&
            !of_has(item->flags, OF_BLESSED)) continue;

        /* Clear all the swap weapon flags as I look at each one. */
        weapon_swap_digger = 0;
        weapon_swap_slay_animal = 0;
        weapon_swap_slay_evil = 0;
        weapon_swap_slay_undead = 0;
        weapon_swap_slay_demon = 0;
        weapon_swap_slay_orc = 0;
        weapon_swap_slay_troll = 0;
        weapon_swap_slay_giant = 0;
        weapon_swap_slay_dragon = 0;
        weapon_swap_impact = false;
        weapon_swap_brand_acid = false;
        weapon_swap_brand_elec = false;
        weapon_swap_brand_fire = false;
        weapon_swap_brand_cold = false;
        weapon_swap_brand_pois = false;
        weapon_swap_see_infra = false;
        weapon_swap_slow_digest = false;
        weapon_swap_aggravate = false;
        weapon_swap_bad_curse = false;
        weapon_swap_regenerate = false;
        weapon_swap_telepathy = false;
        weapon_swap_light = false;
        weapon_swap_see_invis = false;
        weapon_swap_ffall = false;
        weapon_swap_free_act = false;
        weapon_swap_hold_life = false;
        weapon_swap_immune_fire = false;
        weapon_swap_immune_acid = false;
        weapon_swap_immune_cold = false;
        weapon_swap_immune_elec = false;
        weapon_swap_resist_acid = false;
        weapon_swap_resist_elec = false;
        weapon_swap_resist_fire = false;
        weapon_swap_resist_cold = false;
        weapon_swap_resist_pois = false;
        weapon_swap_resist_conf = false;
        weapon_swap_resist_sound = false;
        weapon_swap_resist_light = false;
        weapon_swap_resist_dark = false;
        weapon_swap_resist_chaos = false;
        weapon_swap_resist_disen = false;
        weapon_swap_resist_shard = false;
        weapon_swap_resist_nexus = false;
        weapon_swap_resist_blind = false;
        weapon_swap_resist_neth = false;
        decurse_weapon_swap = false;

        /* Analyze the item */
        switch (item->tval)
        {

            /* weapons */
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_SWORD:
        case TV_DIGGING:
        {

            /* Digging */
            if (of_has(item->flags, OF_DIG_1) || of_has(item->flags, OF_DIG_2) || of_has(item->flags, OF_DIG_3))
            {
                /* Don't notice digger if we can turn stone to mud,
                 * or I am using one.
                 */
                 /* Hack -- ignore worthless ones (including cursed) */
                if (item->value <= 0) break;
                if (item->cursed) break;
                if (!borg_spell_legal_fail(TURN_STONE_TO_MUD, 40) &&
                    !borg_spell_legal_fail(SHATTER_STONE, 40) &&
                    !of_has(borg_items[INVEN_WIELD].flags, OF_DIG_1) &&
                    !of_has(borg_items[INVEN_WIELD].flags, OF_DIG_2) &&
                    !of_has(borg_items[INVEN_WIELD].flags, OF_DIG_3))
                    weapon_swap_digger = item->pval;
            }

            /* various slays */
            if (of_has(item->flags, OF_IMPACT))      weapon_swap_impact = true;

            weapon_swap_slay_animal = item->slays[RF_ANIMAL];
            weapon_swap_slay_evil = item->slays[RF_EVIL];
            weapon_swap_slay_undead = item->slays[RF_UNDEAD];
            weapon_swap_slay_demon = item->slays[RF_DEMON];
            weapon_swap_slay_orc = item->slays[RF_ORC];
            weapon_swap_slay_troll = item->slays[RF_TROLL];
            weapon_swap_slay_giant = item->slays[RF_GIANT];
            weapon_swap_slay_dragon = item->slays[RF_DRAGON];

            if (item->brands[ELEM_ACID]) weapon_swap_brand_acid = true;
            if (item->brands[ELEM_ELEC]) weapon_swap_brand_elec = true;
            if (item->brands[ELEM_FIRE]) weapon_swap_brand_fire = true;
            if (item->brands[ELEM_COLD]) weapon_swap_brand_cold = true;
            if (item->brands[ELEM_POIS]) weapon_swap_brand_pois = true;

            /* Affect infravision */
            weapon_swap_see_infra += item->modifiers[OBJ_MOD_INFRA];

            /* Affect speed */

            /* Various flags */
            if (of_has(item->flags, OF_SLOW_DIGEST)) weapon_swap_slow_digest = true;
            if (of_has(item->flags, OF_AGGRAVATE)) weapon_swap_aggravate = true;
            if (of_has(item->flags, OF_REGEN)) weapon_swap_regenerate = true;
            if (of_has(item->flags, OF_TELEPATHY)) weapon_swap_telepathy = true;
            if (of_has(item->flags, OF_LIGHT_2) || of_has(item->flags, OF_LIGHT_3)) weapon_swap_light = true;
            if (of_has(item->flags, OF_SEE_INVIS)) weapon_swap_see_invis = true;
            if (of_has(item->flags, OF_FEATHER)) weapon_swap_ffall = true;
            if (of_has(item->flags, OF_FREE_ACT)) weapon_swap_free_act = true;
            if (of_has(item->flags, OF_HOLD_LIFE)) weapon_swap_hold_life = true;
            if (of_has(item->flags, OF_PROT_CONF)) weapon_swap_resist_conf = true;
            if (of_has(item->flags, OF_PROT_BLIND)) weapon_swap_resist_blind = true;

            /* curses */
            if (borg_has_bad_curse(item)) weapon_swap_bad_curse = true;

            /* Immunity flags */
            if (item->el_info[ELEM_FIRE].res_level == 3) weapon_swap_immune_fire = true;
            if (item->el_info[ELEM_ACID].res_level == 3) weapon_swap_immune_acid = true;
            if (item->el_info[ELEM_COLD].res_level == 3) weapon_swap_immune_cold = true;
            if (item->el_info[ELEM_ELEC].res_level == 3) weapon_swap_immune_elec = true;

            /* Resistance flags */
            if (item->el_info[ELEM_ACID].res_level > 0) weapon_swap_resist_acid = true;
            if (item->el_info[ELEM_ELEC].res_level > 0) weapon_swap_resist_elec = true;
            if (item->el_info[ELEM_FIRE].res_level > 0) weapon_swap_resist_fire = true;
            if (item->el_info[ELEM_COLD].res_level > 0) weapon_swap_resist_cold = true;
            if (item->el_info[ELEM_POIS].res_level > 0) weapon_swap_resist_pois = true;
            if (item->el_info[ELEM_SOUND].res_level > 0) weapon_swap_resist_sound = true;
            if (item->el_info[ELEM_LIGHT].res_level > 0) weapon_swap_resist_light = true;
            if (item->el_info[ELEM_DARK].res_level > 0) weapon_swap_resist_dark = true;
            if (item->el_info[ELEM_CHAOS].res_level > 0) weapon_swap_resist_chaos = true;
            if (item->el_info[ELEM_DISEN].res_level > 0) weapon_swap_resist_disen = true;
            if (item->el_info[ELEM_SHARD].res_level > 0) weapon_swap_resist_shard = true;
            if (item->el_info[ELEM_NEXUS].res_level > 0) weapon_swap_resist_nexus = true;
            if (item->el_info[ELEM_NETHER].res_level > 0) weapon_swap_resist_neth = true;
            if (item->uncursable) decurse_weapon_swap = true;

            /* Sustain flags */

            /* calculating the value of the swap weapon. */
            damage = (item->dd * (item->ds) * 25L);

            /* Reward "damage" and increased blows per round*/
            v += damage * (borg_skill[BI_BLOWS] + 1);

            /* Reward "bonus to hit" */
            v += ((borg_skill[BI_TOHIT] + item->to_h) * 100L);

            /* Reward "bonus to dam" */
            v += ((borg_skill[BI_TODAM] + item->to_d) * 75L);

            dam = damage * borg_skill[BI_BLOWS];

            /* assume 2x base damage for x% of creatures */
            dam = damage * 2 * borg_skill[BI_BLOWS];
            /* rewared SAnimal if no electric brand */
            if (!borg_skill[BI_WS_ANIMAL] && !borg_skill[BI_WB_ELEC] && weapon_swap_slay_animal) v += (dam * weapon_swap_slay_animal) / 2;
            if (!borg_skill[BI_WS_EVIL] && weapon_swap_slay_evil) v += (dam * weapon_swap_slay_evil) / 2;

            /* assume 3x base damage for x% of creatures */
            dam = damage * 3 * borg_skill[BI_BLOWS];

            /* half of the reward now for SOrc and STroll*/
            if (!borg_skill[BI_WS_ORC] && weapon_swap_slay_orc) v += (dam * weapon_swap_slay_orc) / 2;
            if (!borg_skill[BI_WS_TROLL] && weapon_swap_slay_troll) v += (dam * 2) / 2;

            if (!borg_skill[BI_WS_UNDEAD] && weapon_swap_slay_undead) v += (dam * weapon_swap_slay_undead) / 2;
            if (!borg_skill[BI_WS_DEMON] && weapon_swap_slay_demon) v += (dam * weapon_swap_slay_demon) / 2;
            if (!borg_skill[BI_WS_GIANT] && weapon_swap_slay_giant) v += (dam * weapon_swap_slay_giant) / 2;
            if (!borg_skill[BI_WS_DRAGON] && !borg_skill[BI_WK_DRAGON] && weapon_swap_slay_dragon) v += (dam * weapon_swap_slay_dragon) / 2;
            if (!borg_skill[BI_WB_ACID] && weapon_swap_brand_acid) v += (dam * 4) / 2;
            if (!borg_skill[BI_WB_ELEC] && weapon_swap_brand_elec) v += (dam * 5) / 2;
            if (!borg_skill[BI_WB_FIRE] && weapon_swap_brand_fire) v += (dam * 3) / 2;
            if (!borg_skill[BI_WB_COLD] && weapon_swap_brand_cold) v += (dam * 3) / 2;
            if (!borg_skill[BI_WB_POIS] && weapon_swap_brand_pois) v += (dam * 3) / 2;
            /* Orcs and Trolls get the second half of the reward if SEvil is not possesed. */
            if (!borg_skill[BI_WS_ORC] && !borg_skill[BI_WS_EVIL] && weapon_swap_slay_orc) v += (dam * weapon_swap_slay_orc) / 2;
            if (!borg_skill[BI_WS_TROLL] && !borg_skill[BI_WS_EVIL] && weapon_swap_slay_troll) v += (dam * weapon_swap_slay_troll) / 2;

            /* reward the Tunnel factor when low level */
            if (borg_skill[BI_MAXDEPTH] <= 40 && borg_skill[BI_MAXDEPTH] >= 25 && borg_gold < 100000 && weapon_swap_digger) v += (weapon_swap_digger * 3500L) + 1000L;

            /* Other Skills */
            if (!borg_skill[BI_SDIG] && weapon_swap_slow_digest) v += 10L;
            if (weapon_swap_aggravate) v -= 8000L;
            if (weapon_swap_bad_curse) v -= 100000L;
            if (decurse_weapon_swap) v -= 5000L;
            if (!borg_skill[BI_REG] && weapon_swap_regenerate) v += 2000L;
            if (!borg_skill[BI_ESP] && weapon_swap_telepathy) v += 5000L;
            if (!borg_skill[BI_LIGHT] && weapon_swap_light) v += 2000L;
            if (!borg_skill[BI_SINV] && weapon_swap_see_invis) v += 50000L;
            if (!borg_skill[BI_FEATH] && weapon_swap_ffall) v += 10L;
            if (!borg_skill[BI_FRACT] && weapon_swap_free_act) v += 10000L;
            if (!borg_skill[BI_HLIFE] && (borg_skill[BI_MAXCLEVEL] < 50) && weapon_swap_hold_life) v += 2000L;
            if (!borg_skill[BI_IFIRE] && weapon_swap_immune_fire) v += 70000L;
            if (!borg_skill[BI_IACID] && weapon_swap_immune_acid) v += 30000L;
            if (!borg_skill[BI_ICOLD] && weapon_swap_immune_cold) v += 50000L;
            if (!borg_skill[BI_IELEC] && weapon_swap_immune_elec) v += 25000L;
            if (!borg_skill[BI_RFIRE] && weapon_swap_resist_fire) v += 8000L;
            if (!borg_skill[BI_RACID] && weapon_swap_resist_acid) v += 6000L;
            if (!borg_skill[BI_RCOLD] && weapon_swap_resist_cold) v += 4000L;
            if (!borg_skill[BI_RELEC] && weapon_swap_resist_elec) v += 3000L;
            /* extra bonus for getting all basic resist */
            if (weapon_swap_resist_fire &&
                weapon_swap_resist_acid &&
                weapon_swap_resist_elec &&
                weapon_swap_resist_cold) v += 10000L;
            if (!borg_skill[BI_RPOIS] && weapon_swap_resist_pois) v += 20000L;
            if (!borg_skill[BI_RCONF] && weapon_swap_resist_conf) v += 5000L;
            if (!borg_skill[BI_RSND] && weapon_swap_resist_sound) v += 2000L;
            if (!borg_skill[BI_RLITE] && weapon_swap_resist_light) v += 800L;
            if (!borg_skill[BI_RDARK] && weapon_swap_resist_dark) v += 800L;
            if (!borg_skill[BI_RKAOS] && weapon_swap_resist_chaos) v += 8000L;
            if (!borg_skill[BI_RDIS] && weapon_swap_resist_disen) v += 5000L;
            if (!borg_skill[BI_RSHRD] && weapon_swap_resist_shard) v += 100L;
            if (!borg_skill[BI_RNXUS] && weapon_swap_resist_nexus) v += 100L;
            if (!borg_skill[BI_RBLIND] && weapon_swap_resist_blind) v += 5000L;
            if (!borg_skill[BI_RNTHR] && weapon_swap_resist_neth) v += 5500L;
            if (!borg_skill[BI_RFEAR] && weapon_swap_resist_fear) v += 5500L;

            /* Special concern if Tarrasque is alive */
            if (borg_skill[BI_MAXDEPTH] >= 75 &&
                ((!borg_skill[BI_ICOLD] && weapon_swap_immune_cold) ||
                    (!borg_skill[BI_IFIRE] && weapon_swap_immune_fire)))
            {
                /* If Tarraseque is alive */
                if (borg_race_death[539] == 0)
                {
                    if (!borg_skill[BI_ICOLD] && weapon_swap_immune_cold) v += 90000L;
                    if (!borg_skill[BI_IFIRE] && weapon_swap_immune_fire) v += 90000L;
                }

            }


            /*  Mega-Hack -- resists (level 60) */
            /* its possible that he will get a sword and a cloak
             * both with the same high resist and keep each based
             * on that resist.  We want him to check to see
             * that the other swap does not already have the high resist.
             */
            if (!borg_skill[BI_RNTHR] && (borg_skill[BI_MAXDEPTH] + 1 >= 55) &&
                weapon_swap_resist_neth) v += 100000L;
            if (!borg_skill[BI_RKAOS] && (borg_skill[BI_MAXDEPTH] + 1 >= 60) &&
                weapon_swap_resist_chaos) v += 100000L;
            if (!borg_skill[BI_RDIS] && (borg_skill[BI_MAXDEPTH] + 1 >= 60) &&
                weapon_swap_resist_disen) v += 100000L;

            /* some artifacts would make good back ups for their activation */


            /* skip usless ones */
            if (v <= 1000) continue;

            /* collect the best one */
            if (v < b_v) continue;

            /* track it */
            b_i = i;
            b_v = v;
        }


        }
    }

    /* Clear all the swap weapon flags. */
    weapon_swap_slay_animal = 0;
    weapon_swap_slay_evil = 0;
    weapon_swap_slay_undead = 0;
    weapon_swap_slay_demon = 0;
    weapon_swap_slay_orc = 0;
    weapon_swap_slay_troll = 0;
    weapon_swap_slay_giant = 0;
    weapon_swap_slay_dragon = 0;
    weapon_swap_impact = false;
    weapon_swap_brand_acid = false;
    weapon_swap_brand_elec = false;
    weapon_swap_brand_fire = false;
    weapon_swap_brand_cold = false;
    weapon_swap_brand_pois = false;
    weapon_swap_see_infra = false;
    weapon_swap_slow_digest = false;
    weapon_swap_aggravate = false;
    weapon_swap_bad_curse = false;
    weapon_swap_regenerate = false;
    weapon_swap_telepathy = false;
    weapon_swap_light = false;
    weapon_swap_see_invis = false;
    weapon_swap_ffall = false;
    weapon_swap_free_act = false;
    weapon_swap_hold_life = false;
    weapon_swap_immune_fire = false;
    weapon_swap_immune_acid = false;
    weapon_swap_immune_cold = false;
    weapon_swap_immune_elec = false;
    weapon_swap_resist_acid = false;
    weapon_swap_resist_elec = false;
    weapon_swap_resist_fire = false;
    weapon_swap_resist_cold = false;
    weapon_swap_resist_pois = false;
    weapon_swap_resist_conf = false;
    weapon_swap_resist_sound = false;
    weapon_swap_resist_light = false;
    weapon_swap_resist_dark = false;
    weapon_swap_resist_chaos = false;
    weapon_swap_resist_disen = false;
    weapon_swap_resist_shard = false;
    weapon_swap_resist_nexus = false;
    weapon_swap_resist_blind = false;
    weapon_swap_resist_neth = false;
    decurse_weapon_swap = false;

    /* Assume no enchantment needed */
    enchant_weapon_swap_to_h = 0;
    enchant_weapon_swap_to_d = 0;

    if (b_i == -1)
        return;

    /* mark the swap item and its value */
    weapon_swap_value = b_v;
    weapon_swap = b_i + 1;

    /* Now that we know who the best swap is lets set our swap
     * flags and get a move on
     */
     /*** Process the best inven item ***/

    item = &borg_items[b_i];

    /* Enchant swap weapons (to hit) */
    if ((borg_spell_legal_fail(ENCHANT_WEAPON, 65) ||
        amt_enchant_weapon >= 1) && item->tval != TV_DIGGING)
    {
        if (item->to_h < 10)
        {
            enchant_weapon_swap_to_h += (10 - item->to_h);
        }

        /* Enchant my swap (to damage) */
        if (item->to_d < 10)
        {
            enchant_weapon_swap_to_d += (10 - item->to_d);
        }
    }
    else if (item->tval != TV_DIGGING)
    {
        if (item->to_h < 8)
        {
            enchant_weapon_swap_to_h += (8 - item->to_h);
        }

        /* Enchant my swap (to damage) */
        if (item->to_d < 8)
        {
            enchant_weapon_swap_to_d += (8 - item->to_d);
        }
    }

    /* various slays */
    weapon_swap_slay_animal = item->slays[RF_ANIMAL];
    weapon_swap_slay_evil = item->slays[RF_EVIL];
    weapon_swap_slay_undead = item->slays[RF_UNDEAD];
    weapon_swap_slay_demon = item->slays[RF_DEMON];
    weapon_swap_slay_orc = item->slays[RF_ORC];
    weapon_swap_slay_troll = item->slays[RF_TROLL];
    weapon_swap_slay_giant = item->slays[RF_GIANT];
    weapon_swap_slay_dragon = item->slays[RF_DRAGON];
    weapon_swap_slay_undead = item->slays[RF_UNDEAD];
    weapon_swap_slay_demon = item->slays[RF_DEMON];

    if (item->brands[ELEM_ACID]) weapon_swap_brand_acid = true;
    if (item->brands[ELEM_ELEC]) weapon_swap_brand_elec = true;
    if (item->brands[ELEM_FIRE]) weapon_swap_brand_fire = true;
    if (item->brands[ELEM_COLD]) weapon_swap_brand_cold = true;
    if (item->brands[ELEM_POIS]) weapon_swap_brand_pois = true;

    /* Affect infravision */
    weapon_swap_see_infra += item->modifiers[OBJ_MOD_INFRA];
    /* Affect various skills */
    /* Affect speed */

    /* Various flags */
    if (of_has(item->flags, OF_IMPACT))      weapon_swap_impact = true;
    if (of_has(item->flags, OF_SLOW_DIGEST)) weapon_swap_slow_digest = true;
    if (of_has(item->flags, OF_AGGRAVATE)) weapon_swap_aggravate = true;
    if (of_has(item->flags, OF_REGEN)) weapon_swap_regenerate = true;
    if (of_has(item->flags, OF_TELEPATHY)) weapon_swap_telepathy = true;
    if (of_has(item->flags, OF_LIGHT_2) || of_has(item->flags, OF_LIGHT_3)) weapon_swap_light = true;
    if (of_has(item->flags, OF_SEE_INVIS)) weapon_swap_see_invis = true;
    if (of_has(item->flags, OF_FEATHER)) weapon_swap_ffall = true;
    if (of_has(item->flags, OF_FREE_ACT)) weapon_swap_free_act = true;
    if (of_has(item->flags, OF_HOLD_LIFE)) weapon_swap_hold_life = true;
    if (of_has(item->flags, OF_PROT_CONF)) weapon_swap_resist_conf = true;
    if (of_has(item->flags, OF_PROT_BLIND)) weapon_swap_resist_blind = true;

    /* curses */
    if (borg_has_bad_curse(item)) weapon_swap_bad_curse = true;

    /* Immunity flags */
    if (item->el_info[ELEM_FIRE].res_level == 3) weapon_swap_immune_fire = true;
    if (item->el_info[ELEM_ACID].res_level == 3) weapon_swap_immune_acid = true;
    if (item->el_info[ELEM_COLD].res_level == 3) weapon_swap_immune_cold = true;
    if (item->el_info[ELEM_ELEC].res_level == 3) weapon_swap_immune_elec = true;

    /* Resistance flags */
    if (item->el_info[ELEM_ELEC].res_level > 0) weapon_swap_resist_acid = true;
    if (item->el_info[ELEM_ELEC].res_level > 0) weapon_swap_resist_elec = true;
    if (item->el_info[ELEM_FIRE].res_level > 0) weapon_swap_resist_fire = true;
    if (item->el_info[ELEM_COLD].res_level > 0) weapon_swap_resist_cold = true;
    if (item->el_info[ELEM_POIS].res_level > 0) weapon_swap_resist_pois = true;
    if (item->el_info[ELEM_SOUND].res_level > 0) weapon_swap_resist_sound = true;
    if (item->el_info[ELEM_LIGHT].res_level > 0) weapon_swap_resist_light = true;
    if (item->el_info[ELEM_DARK].res_level > 0) weapon_swap_resist_dark = true;
    if (item->el_info[ELEM_CHAOS].res_level > 0) weapon_swap_resist_chaos = true;
    if (item->el_info[ELEM_DISEN].res_level > 0) weapon_swap_resist_disen = true;
    if (item->el_info[ELEM_SHARD].res_level > 0) weapon_swap_resist_shard = true;
    if (item->el_info[ELEM_NEXUS].res_level > 0) weapon_swap_resist_nexus = true;
    if (item->el_info[ELEM_NETHER].res_level > 0) weapon_swap_resist_neth = true;
    if (item->uncursable) decurse_weapon_swap = true;
}

/*
 * Helper function -- notice the player swap armour
 */
static void borg_notice_armour_swap(void)
{
    int i;
    int b_i = -1;
    int32_t v = -1L;
    int32_t b_v = 0L;
    int dam, damage;

    borg_item* item;

    armour_swap = 0;

    /* borg option to not use them */
    if (!borg_cfg[BORG_USES_SWAPS]) return;

    /*** Process the inventory ***/
    for (i = 0; i < z_info->pack_size; i++)
    {
        item = &borg_items[i];

        /* reset counter */
        v = -1L;
        dam = 0;
        damage = 0;

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Hack -- skip un-aware items */
        if (!item->kind) continue;

        /* Skip non-wearable items */
        if (borg_slot(item->tval, item->sval) == -1) continue;

        /* Dont carry swaps until dlevel 50.  They are heavy */
        if (borg_skill[BI_MAXDEPTH] < 50) continue;

        /* Skip it if it is not decursable */
        if (item->cursed && !item->uncursable) continue;

        /* One Ring is not a swap */
        if (item->one_ring) continue;

        /* Clear all the swap weapon flags as I look at each one. */
        armour_swap_slay_animal = 0;
        armour_swap_slay_evil = 0;
        armour_swap_slay_undead = 0;
        armour_swap_slay_demon = 0;
        armour_swap_slay_orc = 0;
        armour_swap_slay_troll = 0;
        armour_swap_slay_giant = 0;
        armour_swap_slay_dragon = 0;
        armour_swap_impact = false;
        armour_swap_brand_acid = false;
        armour_swap_brand_elec = false;
        armour_swap_brand_fire = false;
        armour_swap_brand_cold = false;
        armour_swap_brand_pois = false;
        armour_swap_see_infra = false;
        armour_swap_slow_digest = false;
        armour_swap_aggravate = false;
        armour_swap_bad_curse = false;
        armour_swap_regenerate = false;
        armour_swap_telepathy = false;
        armour_swap_light = false;
        armour_swap_see_invis = false;
        armour_swap_ffall = false;
        armour_swap_free_act = false;
        armour_swap_hold_life = false;
        armour_swap_immune_fire = false;
        armour_swap_immune_acid = false;
        armour_swap_immune_cold = false;
        armour_swap_immune_elec = false;
        armour_swap_resist_acid = false;
        armour_swap_resist_elec = false;
        armour_swap_resist_fire = false;
        armour_swap_resist_cold = false;
        armour_swap_resist_pois = false;
        armour_swap_resist_conf = false;
        armour_swap_resist_sound = false;
        armour_swap_resist_LIGHT = false;
        armour_swap_resist_dark = false;
        armour_swap_resist_chaos = false;
        armour_swap_resist_disen = false;
        armour_swap_resist_shard = false;
        armour_swap_resist_nexus = false;
        armour_swap_resist_blind = false;
        armour_swap_resist_neth = false;
        decurse_armour_swap = false;

        /* Analyze the item */
        switch (item->tval)
        {
            /* ARMOUR TYPE STUFF */
        case TV_RING:
        case TV_AMULET:
        case TV_BOOTS:
        case TV_HELM:
        case TV_CROWN:
        case TV_SHIELD:
        case TV_CLOAK:
        case TV_SOFT_ARMOR:
        case TV_HARD_ARMOR:
        case TV_DRAG_ARMOR:
        {
            /* various slays */
            /* as of 280, armours dont have slays but random artifacts might.
             */
            armour_swap_slay_animal = item->slays[RF_ANIMAL];
            armour_swap_slay_evil = item->slays[RF_EVIL];
            armour_swap_slay_undead = item->slays[RF_UNDEAD];
            armour_swap_slay_demon = item->slays[RF_DEMON];
            armour_swap_slay_orc = item->slays[RF_ORC];
            armour_swap_slay_troll = item->slays[RF_TROLL];
            armour_swap_slay_giant = item->slays[RF_GIANT];
            armour_swap_slay_dragon = item->slays[RF_DRAGON];
            if (of_has(item->flags, OF_IMPACT))      armour_swap_impact = true;
            if (item->brands)
            {
                if (item->brands[ELEM_ACID]) armour_swap_brand_acid = true;
                if (item->brands[ELEM_ELEC]) armour_swap_brand_elec = true;
                if (item->brands[ELEM_FIRE]) armour_swap_brand_fire = true;
                if (item->brands[ELEM_COLD]) armour_swap_brand_cold = true;
                if (item->brands[ELEM_POIS]) armour_swap_brand_pois = true;
            }

            /* Affect infravision */
            armour_swap_see_infra += item->modifiers[OBJ_MOD_INFRA];
            /* Affect various skills */
            /* Affect speed */

            /* Various flags */
            if (of_has(item->flags, OF_SLOW_DIGEST)) armour_swap_slow_digest = true;
            if (of_has(item->flags, OF_AGGRAVATE)) armour_swap_aggravate = true;
            if (of_has(item->flags, OF_REGEN)) armour_swap_regenerate = true;
            if (of_has(item->flags, OF_TELEPATHY)) armour_swap_telepathy = true;
            if (of_has(item->flags, OF_LIGHT_2) || of_has(item->flags, OF_LIGHT_3)) armour_swap_light = true;
            if (of_has(item->flags, OF_SEE_INVIS)) armour_swap_see_invis = true;
            if (of_has(item->flags, OF_FEATHER)) armour_swap_ffall = true;
            if (of_has(item->flags, OF_FREE_ACT)) armour_swap_free_act = true;
            if (of_has(item->flags, OF_HOLD_LIFE)) armour_swap_hold_life = true;
            if (of_has(item->flags, OF_PROT_CONF)) armour_swap_resist_conf = true;
            if (of_has(item->flags, OF_PROT_BLIND)) armour_swap_resist_blind = true;

            if (borg_has_bad_curse(item)) armour_swap_bad_curse = true;

            /* Immunity flags */
            if (item->el_info[ELEM_FIRE].res_level == 3) armour_swap_immune_fire = true;
            if (item->el_info[ELEM_ACID].res_level == 3) armour_swap_immune_acid = true;
            if (item->el_info[ELEM_COLD].res_level == 3) armour_swap_immune_cold = true;
            if (item->el_info[ELEM_ELEC].res_level == 3) armour_swap_immune_elec = true;

            /* Resistance flags */
            if (item->el_info[ELEM_ACID].res_level > 0) armour_swap_resist_acid = true;
            if (item->el_info[ELEM_ELEC].res_level > 0) armour_swap_resist_elec = true;
            if (item->el_info[ELEM_FIRE].res_level > 0) armour_swap_resist_fire = true;
            if (item->el_info[ELEM_COLD].res_level > 0) armour_swap_resist_cold = true;
            if (item->el_info[ELEM_POIS].res_level > 0) armour_swap_resist_pois = true;
            if (item->el_info[ELEM_SOUND].res_level > 0) armour_swap_resist_sound = true;
            if (item->el_info[ELEM_LIGHT].res_level > 0) armour_swap_resist_LIGHT = true;
            if (item->el_info[ELEM_DARK].res_level > 0) armour_swap_resist_dark = true;
            if (item->el_info[ELEM_CHAOS].res_level > 0) armour_swap_resist_chaos = true;
            if (item->el_info[ELEM_DISEN].res_level > 0) armour_swap_resist_disen = true;
            if (item->el_info[ELEM_SHARD].res_level > 0) armour_swap_resist_shard = true;
            if (item->el_info[ELEM_NEXUS].res_level > 0) armour_swap_resist_nexus = true;
            if (item->el_info[ELEM_NETHER].res_level > 0) armour_swap_resist_neth = true;
            if (item->uncursable) decurse_armour_swap = true;

            /* Sustain flags */

            /* calculating the value of the swap weapon. */
            damage = (item->dd * item->ds * 35L);

            /* Reward "damage" and increased blows per round*/
            v += damage * (borg_skill[BI_BLOWS] + 1);

            /* Reward "bonus to hit" */
            v += ((borg_skill[BI_TOHIT] + item->to_h) * 100L);

            /* Reward "bonus to dam" */
            v += ((borg_skill[BI_TODAM] + item->to_d) * 35L);

            dam = damage * borg_skill[BI_BLOWS];

            /* assume 2x base damage for x% of creatures */
            dam = damage * 2 * borg_skill[BI_BLOWS];

            if (!borg_skill[BI_WS_ANIMAL] && !borg_skill[BI_WB_ELEC] && armour_swap_slay_animal) v += (dam * armour_swap_slay_animal) / 2;
            if (!borg_skill[BI_WS_EVIL] && armour_swap_slay_evil) v += (dam * armour_swap_slay_evil) / 2;
            /* assume 3x base damage for x% of creatures */
            dam = damage * 3 * borg_skill[BI_BLOWS];

            if (!borg_skill[BI_WS_UNDEAD] && armour_swap_slay_undead) v += (dam * armour_swap_slay_undead) / 2;
            if (!borg_skill[BI_WS_DEMON] && armour_swap_slay_demon) v += (dam * armour_swap_slay_demon) / 2;
            if (!borg_skill[BI_WS_GIANT] && armour_swap_slay_giant) v += (dam * armour_swap_slay_giant) / 2;
            if (!borg_skill[BI_WS_DRAGON] && !borg_skill[BI_WK_DRAGON] && armour_swap_slay_dragon) v += (dam * armour_swap_slay_dragon) / 2;
            if (!borg_skill[BI_WB_ACID] && armour_swap_brand_acid) v += (dam * 4) / 2;
            if (!borg_skill[BI_WB_ELEC] && armour_swap_brand_elec) v += (dam * 5) / 2;
            if (!borg_skill[BI_WB_FIRE] && armour_swap_brand_fire) v += (dam * 3) / 2;
            if (!borg_skill[BI_WB_COLD] && armour_swap_brand_cold) v += (dam * 3) / 2;
            if (!borg_skill[BI_WB_POIS] && armour_swap_brand_pois) v += (dam * 3) / 2;
            /* SOrc and STroll get 1/2 reward now */
            if (!borg_skill[BI_WS_ORC] && armour_swap_slay_orc) v += (dam * armour_swap_slay_orc) / 2;
            if (!borg_skill[BI_WS_TROLL] && armour_swap_slay_troll) v += (dam * armour_swap_slay_troll) / 2;
            /* SOrc and STroll get 2/2 reward if slay evil not possesed */
            if (!borg_skill[BI_WS_ORC] && !borg_skill[BI_WS_EVIL] && armour_swap_slay_orc) v += (dam * armour_swap_slay_orc) / 2;
            if (!borg_skill[BI_WS_TROLL] && !borg_skill[BI_WS_EVIL] && armour_swap_slay_troll) v += (dam * armour_swap_slay_troll) / 2;


            if (!borg_skill[BI_SDIG] && armour_swap_slow_digest) v += 10L;
            if (armour_swap_aggravate) v -= 8000L;
            /* for now, all "bad" curses are lumped together */
            if (armour_swap_bad_curse) v -= 100000L;
            if (decurse_armour_swap) v -= 5000L;
            if (!borg_skill[BI_REG] && armour_swap_regenerate) v += 2000L;
            if (!borg_skill[BI_ESP] && armour_swap_telepathy) v += 5000L;
            if (!borg_skill[BI_LIGHT] && armour_swap_light) v += 2000L;
            if (!borg_skill[BI_SINV] && armour_swap_see_invis) v += 50000L;
            if (!borg_skill[BI_FEATH] && armour_swap_ffall) v += 10L;
            if (!borg_skill[BI_FRACT] && armour_swap_free_act) v += 10000L;
            if (!borg_skill[BI_HLIFE] && (borg_skill[BI_MAXCLEVEL] < 50) && armour_swap_hold_life) v += 2000L;
            if (!borg_skill[BI_IFIRE] && armour_swap_immune_fire) v += 70000L;
            if (!borg_skill[BI_IACID] && armour_swap_immune_acid) v += 30000L;
            if (!borg_skill[BI_ICOLD] && armour_swap_immune_cold) v += 50000L;
            if (!borg_skill[BI_IELEC] && armour_swap_immune_elec) v += 25000L;
            if (!borg_skill[BI_RFIRE] && armour_swap_resist_fire) v += 8000L;
            if (!borg_skill[BI_RACID] && armour_swap_resist_acid) v += 6000L;
            if (!borg_skill[BI_RCOLD] && armour_swap_resist_cold) v += 4000L;
            if (!borg_skill[BI_RELEC] && armour_swap_resist_elec) v += 3000L;
            /* extra bonus for getting all basic resist */
            if (armour_swap_resist_fire &&
                armour_swap_resist_acid &&
                armour_swap_resist_elec &&
                armour_swap_resist_cold) v += 10000L;
            if (!borg_skill[BI_RPOIS] && armour_swap_resist_pois) v += 20000L;
            if (!borg_skill[BI_RCONF] && armour_swap_resist_conf) v += 5000L;
            if (!borg_skill[BI_RSND] && armour_swap_resist_sound) v += 2000L;
            if (!borg_skill[BI_RLITE] && armour_swap_resist_LIGHT) v += 800L;
            if (!borg_skill[BI_RDARK] && armour_swap_resist_dark) v += 800L;
            if (!borg_skill[BI_RKAOS] && armour_swap_resist_chaos) v += 8000L;
            if (!borg_skill[BI_RDIS] && armour_swap_resist_disen) v += 5000L;
            if (!borg_skill[BI_RSHRD] && armour_swap_resist_shard) v += 100L;
            if (!borg_skill[BI_RNXUS] && armour_swap_resist_nexus) v += 100L;
            if (!borg_skill[BI_RBLIND] && armour_swap_resist_blind) v += 5000L;
            if (!borg_skill[BI_RNTHR] && armour_swap_resist_neth) v += 5500L;
            /* Special concern if Tarraseque is alive */
            if (borg_skill[BI_MAXDEPTH] >= 75 &&
                ((!borg_skill[BI_ICOLD] && armour_swap_immune_cold) ||
                    (!borg_skill[BI_IFIRE] && armour_swap_immune_fire)))
            {
                /* If Tarraseque is alive */
                if (borg_race_death[539] == 0)
                {
                    if (!borg_skill[BI_ICOLD] && armour_swap_immune_cold) v += 90000L;
                    if (!borg_skill[BI_IFIRE] && armour_swap_immune_fire) v += 90000L;
                }

            }



            /*  Mega-Hack -- resists (level 60) */
            /* Its possible that he will get a sword and a cloak
             * both with the same high resist and keep each based
             * on that resist.  We want him to check to see
             * that the other swap does not already have the high resist.
             */
            if (!borg_skill[BI_RNTHR] && borg_skill[BI_MAXDEPTH] + 1 >= 55 &&
                !weapon_swap_resist_neth &&
                armour_swap_resist_neth) v += 105000L;
            if (!borg_skill[BI_RKAOS] && borg_skill[BI_MAXDEPTH] + 1 >= 60 &&
                !weapon_swap_resist_chaos &&
                armour_swap_resist_chaos) v += 104000L;
            if (!borg_skill[BI_RDIS] && borg_skill[BI_MAXDEPTH] + 1 >= 60 &&
                !weapon_swap_resist_disen &&
                armour_swap_resist_disen) v += 100000L;

            /* some artifacts would make good back ups for their activation */

        }

        /* skip usless ones */
        if (v <= 1000) continue;

        /* collect the best one */
        if ((b_i >= 0) && (v < b_v)) continue;

        /* track it */
        b_i = i;
        b_v = v;
        armour_swap_value = v;
        armour_swap = i - 1;
        }
    }

    /* Now that we know who the best swap is lets set our swap
     * flags and get a move on
     */

    /* Clear all the swap weapon flags as I look at each one. */
    armour_swap_slay_animal = 0;
    armour_swap_slay_evil = 0;
    armour_swap_slay_undead = 0;
    armour_swap_slay_demon = 0;
    armour_swap_slay_orc = 0;
    armour_swap_slay_troll = 0;
    armour_swap_slay_giant = 0;
    armour_swap_slay_dragon = 0;
    armour_swap_impact = false;
    armour_swap_brand_acid = false;
    armour_swap_brand_elec = false;
    armour_swap_brand_fire = false;
    armour_swap_brand_cold = false;
    armour_swap_brand_pois = false;
    armour_swap_see_infra = false;
    armour_swap_slow_digest = false;
    armour_swap_aggravate = false;
    armour_swap_bad_curse = false;
    armour_swap_regenerate = false;
    armour_swap_telepathy = false;
    armour_swap_light = false;
    armour_swap_see_invis = false;
    armour_swap_ffall = false;
    armour_swap_free_act = false;
    armour_swap_hold_life = false;
    armour_swap_immune_fire = false;
    armour_swap_immune_acid = false;
    armour_swap_immune_cold = false;
    armour_swap_immune_elec = false;
    armour_swap_resist_acid = false;
    armour_swap_resist_elec = false;
    armour_swap_resist_fire = false;
    armour_swap_resist_cold = false;
    armour_swap_resist_pois = false;
    armour_swap_resist_conf = false;
    armour_swap_resist_sound = false;
    armour_swap_resist_LIGHT = false;
    armour_swap_resist_dark = false;
    armour_swap_resist_chaos = false;
    armour_swap_resist_disen = false;
    armour_swap_resist_shard = false;
    armour_swap_resist_nexus = false;
    armour_swap_resist_blind = false;
    armour_swap_resist_neth = false;
    decurse_armour_swap = false;

    if (b_i == -1)
        return;

    /*** Process the best inven item ***/
    item = &borg_items[b_i];

    /* various slays */
    armour_swap_slay_animal = item->slays[RF_ANIMAL];
    armour_swap_slay_evil = item->slays[RF_EVIL];
    armour_swap_slay_undead = item->slays[RF_UNDEAD];
    armour_swap_slay_demon = item->slays[RF_DEMON];
    armour_swap_slay_orc = item->slays[RF_ORC];
    armour_swap_slay_troll = item->slays[RF_TROLL];
    armour_swap_slay_giant = item->slays[RF_GIANT];
    armour_swap_slay_dragon = item->slays[RF_DRAGON];

    if (item->brands[ELEM_ACID]) armour_swap_brand_acid = true;
    if (item->brands[ELEM_ELEC]) armour_swap_brand_elec = true;
    if (item->brands[ELEM_FIRE]) armour_swap_brand_fire = true;
    if (item->brands[ELEM_COLD]) armour_swap_brand_cold = true;
    if (item->brands[ELEM_POIS]) armour_swap_brand_pois = true;

    /* Affect infravision */
    armour_swap_see_infra += item->modifiers[OBJ_MOD_INFRA];
    /* Affect various skills */
    /* Affect speed */

    /* Various flags */
    if (of_has(item->flags, OF_IMPACT))      armour_swap_impact = true;
    if (of_has(item->flags, OF_SLOW_DIGEST)) armour_swap_slow_digest = true;
    if (of_has(item->flags, OF_AGGRAVATE)) armour_swap_aggravate = true;
    if (of_has(item->flags, OF_REGEN)) armour_swap_regenerate = true;
    if (of_has(item->flags, OF_TELEPATHY)) armour_swap_telepathy = true;
    if (of_has(item->flags, OF_LIGHT_2) || of_has(item->flags, OF_LIGHT_3)) armour_swap_light = true;
    if (of_has(item->flags, OF_SEE_INVIS)) armour_swap_see_invis = true;
    if (of_has(item->flags, OF_FEATHER)) armour_swap_ffall = true;
    if (of_has(item->flags, OF_FREE_ACT)) armour_swap_free_act = true;
    if (of_has(item->flags, OF_HOLD_LIFE)) armour_swap_hold_life = true;
    if (of_has(item->flags, OF_PROT_CONF)) armour_swap_resist_conf = true;
    if (of_has(item->flags, OF_PROT_BLIND)) armour_swap_resist_blind = true;

    /* curses */
    if (borg_has_bad_curse(item)) armour_swap_bad_curse = true;

    /* Immunity flags */
    if (item->el_info[ELEM_FIRE].res_level == 3) armour_swap_immune_fire = true;
    if (item->el_info[ELEM_ACID].res_level == 3) armour_swap_immune_acid = true;
    if (item->el_info[ELEM_COLD].res_level == 3) armour_swap_immune_cold = true;
    if (item->el_info[ELEM_ELEC].res_level == 3) armour_swap_immune_elec = true;

    /* Resistance flags */
    if (item->el_info[ELEM_ACID].res_level > 0) armour_swap_resist_acid = true;
    if (item->el_info[ELEM_ELEC].res_level > 0) armour_swap_resist_elec = true;
    if (item->el_info[ELEM_FIRE].res_level > 0) armour_swap_resist_fire = true;
    if (item->el_info[ELEM_COLD].res_level > 0) armour_swap_resist_cold = true;
    if (item->el_info[ELEM_POIS].res_level > 0) armour_swap_resist_pois = true;
    if (item->el_info[ELEM_SOUND].res_level > 0) armour_swap_resist_sound = true;
    if (item->el_info[ELEM_LIGHT].res_level > 0) armour_swap_resist_LIGHT = true;
    if (item->el_info[ELEM_DARK].res_level > 0) armour_swap_resist_dark = true;
    if (item->el_info[ELEM_CHAOS].res_level > 0) armour_swap_resist_chaos = true;
    if (item->el_info[ELEM_DISEN].res_level > 0) armour_swap_resist_disen = true;
    if (item->el_info[ELEM_SHARD].res_level > 0) armour_swap_resist_shard = true;
    if (item->el_info[ELEM_NEXUS].res_level > 0) armour_swap_resist_nexus = true;
    if (item->el_info[ELEM_NETHER].res_level > 0) armour_swap_resist_neth = true;
    if (item->uncursable) decurse_armour_swap = true;

    enchant_armour_swap_to_a = 0;

    /* dont look for enchantment on non armours */
    if (item->tval >= TV_LIGHT) return;

    /* Hack -- enchant the swap equipment (armor) */
    /* Note need for enchantment */
    if (borg_spell_legal_fail(ENCHANT_ARMOUR, 65) ||
        amt_enchant_armor >= 1)
    {
        if (item->to_a < 10)
        {
            enchant_armour_swap_to_a += (10 - item->to_a);
        }
    }
    else
    {
        if (item->to_a < 8)
        {
            enchant_armour_swap_to_a += (8 - item->to_a);
        }
    }

}

/*
 * Analyze the equipment and inventory
 */
void borg_notice(bool notice_swap)
{
    int inven_weight;
    int carry_capacity;

    /* Clear out 'has' array */
    memset(borg_has, 0, size_obj * sizeof(int));

    /* Many of our variables are tied to borg_skill[], which is erased at the
     * the start of borg_notice().  So we must update the frame the cheat in
     * all the non inventory skills.
     */
    borg_update_frame();

    /* Notice the equipment */
    borg_notice_aux1();

    /* Notice the inventory */
    borg_notice_aux2();

    /* number of inventory slots the quiver used  */
    borg_skill[BI_QUIVER_SLOTS] = (borg_skill[BI_AMMO_COUNT] - 1) / z_info->quiver_slot_size + 1;

    /* Notice and locate my swap weapon */
    if (notice_swap)
    {
        borg_notice_weapon_swap();
        borg_notice_armour_swap();
    }
    borg_skill[BI_SRACID] = borg_skill[BI_RACID]
        || armour_swap_resist_acid
        || weapon_swap_resist_acid
        || borg_spell_legal_fail(RESISTANCE, 15); /* Res FECAP */
    borg_skill[BI_SRELEC] = borg_skill[BI_RELEC]
        || armour_swap_resist_elec
        || weapon_swap_resist_elec
        || borg_spell_legal_fail(RESISTANCE, 15); /* Res FECAP */
    borg_skill[BI_SRFIRE] = borg_skill[BI_RFIRE]
        || armour_swap_resist_fire
        || weapon_swap_resist_fire
        || borg_spell_legal_fail(RESISTANCE, 15); /* Res FECAP */
    borg_skill[BI_SRCOLD] = borg_skill[BI_RCOLD]
        || armour_swap_resist_cold
        || weapon_swap_resist_cold
        || borg_spell_legal_fail(RESISTANCE, 15); /* Res FECAP */
    borg_skill[BI_SRPOIS] = borg_skill[BI_RPOIS]
        || armour_swap_resist_pois
        || weapon_swap_resist_pois
        || borg_spell_legal_fail(RESISTANCE, 15) /* Res FECAP */
        || borg_spell_legal_fail(RESIST_POISON, 15); /* Res P */
    borg_skill[BI_SRFEAR] = borg_skill[BI_RFEAR]
        || armour_swap_resist_fear
        || weapon_swap_resist_fear;
    borg_skill[BI_SRLITE] = borg_skill[BI_RLITE]
        || armour_swap_resist_LIGHT
        || weapon_swap_resist_light;
    borg_skill[BI_SRDARK] = borg_skill[BI_RDARK]
        || armour_swap_resist_dark
        || weapon_swap_resist_dark;
    borg_skill[BI_SRBLIND] = borg_skill[BI_RBLIND]
        || armour_swap_resist_blind
        || weapon_swap_resist_blind;
    borg_skill[BI_SRCONF] = borg_skill[BI_RCONF]
        || armour_swap_resist_conf
        || weapon_swap_resist_conf;
    borg_skill[BI_SRSND] = borg_skill[BI_RSND]
        || armour_swap_resist_sound
        || weapon_swap_resist_sound;
    borg_skill[BI_SRSHRD] = borg_skill[BI_RSHRD]
        || armour_swap_resist_shard
        || weapon_swap_resist_shard;
    borg_skill[BI_SRNXUS] = borg_skill[BI_RNXUS]
        || armour_swap_resist_nexus
        || weapon_swap_resist_nexus;
    borg_skill[BI_SRNTHR] = borg_skill[BI_RNTHR]
        || armour_swap_resist_neth
        || weapon_swap_resist_neth;
    borg_skill[BI_SRKAOS] = borg_skill[BI_RKAOS]
        || armour_swap_resist_chaos
        || weapon_swap_resist_chaos;
    borg_skill[BI_SRDIS] = borg_skill[BI_RDIS]
        || armour_swap_resist_disen
        || weapon_swap_resist_disen;
    borg_skill[BI_SHLIFE] = borg_skill[BI_HLIFE]
        || armour_swap_hold_life
        || weapon_swap_hold_life;
    borg_skill[BI_SFRACT] = borg_skill[BI_FRACT]
        || armour_swap_free_act
        || weapon_swap_free_act;


    /* Apply "encumbrance" from weight */
    inven_weight = borg_skill[BI_WEIGHT];

    /* Extract the "weight limit" (in tenth pounds) */
    carry_capacity = borg_adj_str_wgt[my_stat_ind[STAT_STR]] * 100;

    /* Apply "encumbrance" from weight */
    if (inven_weight > carry_capacity / 2) borg_skill[BI_SPEED] -= ((inven_weight - (carry_capacity / 2)) / (carry_capacity / 10));

}

/*
 * Helper function -- clear counters for home equipment
 */
static void borg_notice_home_clear(borg_item* in_item, bool no_items)
{

    /*** Reset counters ***/

    /* Reset basic */
    num_food = 0;
    num_fuel = 0;
    num_mold = 0;
    num_ident = 0;
    num_recall = 0;
    num_phase = 0;
    num_escape = 0;
    num_tele_staves = 0;
    num_teleport = 0;
    num_teleport_level = 0;
    num_recharge = 0;

    num_artifact = 0;
    num_ego = 0;

    num_invisible = 0;
    num_pfe = 0;
    num_glyph = 0;
    num_genocide = 0;
    num_mass_genocide = 0;
    num_berserk = 0;
    num_pot_rheat = 0;
    num_pot_rcold = 0;
    num_speed = 0;

    num_slow_digest = 0;
    num_regenerate = 0;
    num_telepathy = 0;
    num_see_inv = 0;
    num_ffall = 0;
    num_free_act = 0;
    num_hold_life = 0;
    num_immune_acid = 0;
    num_immune_elec = 0;
    num_immune_fire = 0;
    num_immune_cold = 0;
    num_resist_acid = 0;
    num_resist_elec = 0;
    num_resist_fire = 0;
    num_resist_cold = 0;
    num_resist_pois = 0;
    num_resist_conf = 0;
    num_resist_sound = 0;
    num_resist_LIGHT = 0;
    num_resist_dark = 0;
    num_resist_chaos = 0;
    num_resist_disen = 0;
    num_resist_shard = 0;
    num_resist_nexus = 0;
    num_resist_blind = 0;
    num_resist_neth = 0;
    num_sustain_str = 0;
    num_sustain_int = 0;
    num_sustain_wis = 0;
    num_sustain_dex = 0;
    num_sustain_con = 0;
    num_sustain_all = 0;

    home_stat_add[STAT_STR] = 0;
    home_stat_add[STAT_INT] = 0;
    home_stat_add[STAT_WIS] = 0;
    home_stat_add[STAT_DEX] = 0;
    home_stat_add[STAT_CON] = 0;

    num_weapons = 0;

    num_bow = 0;
    num_rings = 0;
    num_neck = 0;
    num_armor = 0;
    num_cloaks = 0;
    num_shields = 0;
    num_hats = 0;
    num_gloves = 0;
    num_boots = 0;
    num_LIGHT = 0;
    num_speed = 0;
    num_edged_weapon = 0;
    num_bad_gloves = 0;

    /* Reset healing */
    num_cure_critical = 0;
    num_cure_serious = 0;
    num_fix_exp = 0;
    num_mana = 0;
    num_heal = 0;
    num_ezheal = 0;
    num_life = 0;
    if (!in_item && !no_items) num_ezheal_true = 0;
    if (!in_item && !no_items) num_heal_true = 0;
    if (!in_item && !no_items) num_life_true = 0;

    /* Reset missiles */
    num_missile = 0;

    /* Reset books */
    num_book[0] = 0;
    num_book[1] = 0;
    num_book[2] = 0;
    num_book[3] = 0;
    num_book[4] = 0;
    num_book[5] = 0;
    num_book[6] = 0;
    num_book[7] = 0;
    num_book[8] = 0;

    /* Reset various */
    num_fix_stat[STAT_STR] = 0;
    num_fix_stat[STAT_INT] = 0;
    num_fix_stat[STAT_WIS] = 0;
    num_fix_stat[STAT_DEX] = 0;
    num_fix_stat[STAT_CON] = 0;
    num_fix_stat[6] = 0;

    /* Reset enchantment */
    num_enchant_to_a = 0;
    num_enchant_to_d = 0;
    num_enchant_to_h = 0;

    home_slot_free = 0;
    home_damage = 0;
    home_un_id = 0;

    num_duplicate_items = 0;
}

bool borg_ego_has_random_power(struct ego_item* e_ptr)
{
    if (kf_has(e_ptr->kind_flags, KF_RAND_POWER) ||
        kf_has(e_ptr->kind_flags, KF_RAND_SUSTAIN) ||
        kf_has(e_ptr->kind_flags, KF_RAND_BASE_RES) ||
        kf_has(e_ptr->kind_flags, KF_RAND_HI_RES) ||
        kf_has(e_ptr->kind_flags, KF_RAND_RES_POWER))
        return true;
    return false;
}

/*
 * This checks for duplicate items in the home
 */
static void borg_notice_home_dupe(borg_item* item, bool check_sval, int i)
{
    /* eventually check for power overlap... armor of resistence is same as weak elvenkind.*/
    /*  two armors of elvenkind that resist poison is a dupe.  AJG*/

    int dupe_count, x;
    borg_item* item2;
    struct ego_item* e_ptr = &e_info[item->ego_idx];

    /* check for a duplicate.  */
    /* be carefull about extra powers (elvenkind/magi) */
    if (borg_ego_has_random_power(e_ptr)) return;

    /* if it isn't identified, it isn't duplicate */
    if (item->needs_ident) return;

    /* if this is a stack of items then all after the first are a */
    /* duplicate */
    dupe_count = item->iqty - 1;

    /* Look for other items before this one that are the same */
    for (x = 0; x < i; x++)
    {
        if (x < z_info->store_inven_max)
            item2 = &borg_shops[7].ware[x];
        else
            /* Check what the borg has on as well.*/
            item2 = &borg_items[((x - z_info->store_inven_max) + INVEN_WIELD)];

        /* if everything matches it is a duplicate item */
        /* Note that we only check sval on certain items.  This */
        /* is because, for example, two pairs of dragon armor */
        /* are not the same unless thier subtype (color) matches */
        /* but a defender is a defender even if one is a dagger and */
        /* one is a mace */
        if ((item->tval == item2->tval) &&
            (check_sval ? (item->sval == item2->sval) : true) &&
            (item->art_idx == item2->art_idx) &&
            (item->ego_idx == item2->ego_idx))
        {
            dupe_count++;
        }
    }

    /* there can be one dupe of rings because there are two ring slots. */
    if (item->tval == TV_RING && dupe_count)
        dupe_count--;

    /* Add this items count to the total duplicate count */
    num_duplicate_items += dupe_count;
}

/*
 * Helper function -- notice the home inventory
 */
static void borg_notice_home_aux(borg_item* in_item, bool no_items)
{
    int i;

    borg_item* item = NULL;

    borg_shop* shop = &borg_shops[7];
    bitflag f[OF_SIZE];

    /*** Process the inventory ***/

    /* Scan the home */
    for (i = 0; i < (z_info->store_inven_max + (INVEN_TOTAL - INVEN_WIELD)); i++)
    {
        if (no_items) break;

        if (!in_item)
            if (i < z_info->store_inven_max)
                item = &shop->ware[i];
            else
                item = &borg_items[((i - z_info->store_inven_max) + INVEN_WIELD)];
        else
            item = in_item;

        /* Skip empty items */
        if (!item->iqty && (i < z_info->store_inven_max))
        {
            home_slot_free++;
            continue;
        }

        /* Hack -- skip un-aware items */
        if (!item->kind && (i < z_info->store_inven_max))
        {
            home_slot_free++;
            continue;
        }

        if (of_has(item->flags, OF_SLOW_DIGEST)) num_slow_digest += item->iqty;
        if (of_has(item->flags, OF_REGEN)) num_regenerate += item->iqty;
        if (of_has(item->flags, OF_TELEPATHY)) num_telepathy += item->iqty;
        if (of_has(item->flags, OF_SEE_INVIS)) num_see_inv += item->iqty;
        if (of_has(item->flags, OF_FEATHER)) num_ffall += item->iqty;
        if (of_has(item->flags, OF_FREE_ACT)) num_free_act += item->iqty;
        if (of_has(item->flags, OF_HOLD_LIFE)) num_hold_life += item->iqty;
        if (of_has(item->flags, OF_PROT_CONF)) num_resist_conf += item->iqty;
        if (of_has(item->flags, OF_PROT_BLIND)) num_resist_blind += item->iqty;
        if (item->el_info[ELEM_FIRE].res_level == 3)
        {
            num_immune_fire += item->iqty;
            num_resist_fire += item->iqty;
        }
        if (item->el_info[ELEM_ACID].res_level == 3)
        {
            num_immune_acid += item->iqty;
            num_resist_acid += item->iqty;
        }
        if (item->el_info[ELEM_COLD].res_level == 3)
        {
            num_immune_cold += item->iqty;
            num_resist_cold += item->iqty;
        }
        if (item->el_info[ELEM_ELEC].res_level == 3)
        {
            num_immune_elec += item->iqty;
            num_resist_elec += item->iqty;
        }
        if (item->el_info[ELEM_ACID].res_level == 1) num_resist_acid += item->iqty;
        if (item->el_info[ELEM_ELEC].res_level == 1) num_resist_elec += item->iqty;
        if (item->el_info[ELEM_FIRE].res_level == 1) num_resist_fire += item->iqty;
        if (item->el_info[ELEM_COLD].res_level == 1) num_resist_cold += item->iqty;
        if (item->el_info[ELEM_POIS].res_level == 1) num_resist_pois += item->iqty;
        if (item->el_info[ELEM_SOUND].res_level == 1) num_resist_sound += item->iqty;
        if (item->el_info[ELEM_LIGHT].res_level == 1) num_resist_LIGHT += item->iqty;
        if (item->el_info[ELEM_DARK].res_level == 1) num_resist_dark += item->iqty;
        if (item->el_info[ELEM_CHAOS].res_level == 1) num_resist_chaos += item->iqty;
        if (item->el_info[ELEM_DISEN].res_level == 1) num_resist_disen += item->iqty;
        if (item->el_info[ELEM_SHARD].res_level == 1) num_resist_shard += item->iqty;
        if (item->el_info[ELEM_NEXUS].res_level == 1) num_resist_nexus += item->iqty;
        if (item->el_info[ELEM_NETHER].res_level == 1) num_resist_neth += item->iqty;

        /* Count Sustains */
        if (of_has(item->flags, OF_SUST_STR)) num_sustain_str += item->iqty;
        if (of_has(item->flags, OF_SUST_INT)) num_sustain_str += item->iqty;
        if (of_has(item->flags, OF_SUST_WIS)) num_sustain_str += item->iqty;
        if (of_has(item->flags, OF_SUST_DEX)) num_sustain_str += item->iqty;
        if (of_has(item->flags, OF_SUST_CON)) num_sustain_str += item->iqty;
        if (of_has(item->flags, OF_SUST_STR) &&
            of_has(item->flags, OF_SUST_INT) &&
            of_has(item->flags, OF_SUST_WIS) &&
            of_has(item->flags, OF_SUST_DEX) &&
            of_has(item->flags, OF_SUST_CON)) num_sustain_all += item->iqty;

        /* count up bonus to stats */
        /* HACK only collect stat rings above +3 */

        if (item->modifiers[OBJ_MOD_STR])
        {
            if (item->tval != TV_RING || item->modifiers[OBJ_MOD_STR] > 3)
                home_stat_add[STAT_STR] += item->modifiers[OBJ_MOD_STR] * item->iqty;
        }
        if (item->modifiers[OBJ_MOD_INT])
        {
            if (item->tval != TV_RING || item->modifiers[OBJ_MOD_INT] > 3)
                home_stat_add[STAT_INT] += item->modifiers[OBJ_MOD_INT] * item->iqty;
        }
        if (item->modifiers[OBJ_MOD_WIS])
        {
            if (item->tval != TV_RING || item->modifiers[OBJ_MOD_WIS] > 3)
                home_stat_add[STAT_WIS] += item->modifiers[OBJ_MOD_WIS] * item->iqty;
        }
        if (item->modifiers[OBJ_MOD_DEX])
        {
            if (item->tval != TV_RING || item->modifiers[OBJ_MOD_DEX] > 3)
                home_stat_add[STAT_DEX] += item->modifiers[OBJ_MOD_DEX] * item->iqty;
        }
        if (item->modifiers[OBJ_MOD_CON])
        {
            if (item->tval != TV_RING || item->modifiers[OBJ_MOD_CON] > 3)
                home_stat_add[STAT_CON] += item->modifiers[OBJ_MOD_CON] * item->iqty;
        }

        /* count up bonus to speed */
        num_speed += item->modifiers[OBJ_MOD_SPEED] * item->iqty;

        /* count artifacts */
        if (item->art_idx)
        {
            num_artifact += item->iqty;
        }
        /* count egos that need *ID* */
        if (borg_ego_has_random_power(&e_info[item->ego_idx]) &&
            item->needs_ident)
        {
            num_ego += item->iqty;
        }

        /* count up unidetified stuff */
        if (item->needs_ident && (i < z_info->store_inven_max))
        {
            home_un_id++;
        }

        /* Analyze the item */
        switch (item->tval)
        {
        case TV_SOFT_ARMOR:
        case TV_HARD_ARMOR:
        num_armor += item->iqty;

        /* see if this item is duplicated */
        borg_notice_home_dupe(item, false, i);
        break;

        case TV_DRAG_ARMOR:
        num_armor += item->iqty;

        /* see if this item is duplicated */
        borg_notice_home_dupe(item, true, i);
        break;

        case TV_CLOAK:
        num_cloaks += item->iqty;

        /* see if this item is duplicated */
        borg_notice_home_dupe(item, false, i);

        break;

        case TV_SHIELD:
        num_shields += item->iqty;

        /* see if this item is duplicated */
        borg_notice_home_dupe(item, false, i);
        break;

        case TV_HELM:
        case TV_CROWN:
        num_hats += item->iqty;

        /* see if this item is duplicated */
        borg_notice_home_dupe(item, false, i);

        break;

        case TV_GLOVES:
        num_gloves += item->iqty;

        /* gloves of slaying give a damage bonus */
        home_damage += item->to_d * 3;

        /* see if this item is duplicated */
        borg_notice_home_dupe(item, false, i);

        break;

        case TV_FLASK:
        /* Use as fuel if we equip a lantern */
        if (borg_items[INVEN_LIGHT].sval == sv_light_lantern)
        {
            num_fuel += item->iqty;
            /* borg_note(format("1.num_fuel=%d",num_fuel)); */
        }
        break;

        case TV_LIGHT:
        /* Fuel */
        if (borg_items[INVEN_LIGHT].sval == sv_light_torch)
        {
            num_fuel += item->iqty;
        }

        /* Artifacts */
        if (item->art_idx)
        {
            num_LIGHT += item->iqty;
        }
        break;

        case TV_BOOTS:
        num_boots += item->iqty;

        /* see if this item is duplicated */
        borg_notice_home_dupe(item, false, i);
        break;

        case TV_SWORD:
        case TV_POLEARM:
        case TV_HAFTED:
        /* case TV_DIGGING: */
        {
            int16_t num_blow;

            num_weapons += item->iqty;
            /*  most edged weapons hurt magic for priests */
            if (player_has(player, PF_BLESS_WEAPON))
            {
                /* Penalize non-blessed edged weapons */
                if ((item->tval == TV_SWORD || item->tval == TV_POLEARM) &&
                    !of_has(item->flags, OF_BLESSED))
                {
                    num_edged_weapon += item->iqty;
                }
            }


            /* NOTE:  This damage does not take slays into account. */
            /* it is just a rough estimate to make sure the glave of pain*/
            /* is kept if it is found */
            /* It is hard to hold a heavy weapon */
            num_blow = 1;
            if (adj_str_hold[my_stat_ind[STAT_STR]] >= item->weight / 10)
            {
                int str_index, dex_index;

                /* Enforce a minimum "weight" */
                int div = ((item->weight < player->class->min_weight) ? player->class->min_weight : item->weight);

                /* Access the strength vs weight */
                str_index = (adj_str_blow[my_stat_ind[STAT_STR]] * player->class->att_multiply / div);

                /* Maximal value */
                if (str_index > 11) str_index = 11;

                /* Index by dexterity */
                dex_index = (borg_adj_dex_blow[my_stat_ind[STAT_DEX]]);

                /* Maximal value */
                if (dex_index > 11) dex_index = 11;

                /* Use the blows table */
                num_blow = borg_blows_table[str_index][dex_index];

                /* Maximal value */
                if (num_blow > player->class->max_attacks) num_blow = player->class->max_attacks;

            }

            /* Require at least one blow */
            if (num_blow < 1) num_blow = 1;

            num_blow += item->modifiers[OBJ_MOD_BLOWS];
            num_blow *= item->iqty;
            if (item->to_d > 8 || borg_skill[BI_CLEVEL] < 15)
            {
                home_damage += num_blow * (item->dd * (item->ds) +
                    (borg_skill[BI_TODAM] + item->to_d));
            }
            else
            {
                home_damage += num_blow * (item->dd * (item->ds) +
                    (borg_skill[BI_TODAM] + 8));
            }

            /* see if this item is a duplicate */
            borg_notice_home_dupe(item, false, i);
            break;
        }

        case TV_BOW:
        num_bow += item->iqty;

        /* see if this item is a duplicate */
        borg_notice_home_dupe(item, false, i);
        break;

        case TV_RING:
        num_rings += item->iqty;

        /* see if this item is a duplicate */
        borg_notice_home_dupe(item, true, i);

        break;

        case TV_AMULET:
        num_neck += item->iqty;

        /* see if this item is a duplicate */
        borg_notice_home_dupe(item, true, i);
        break;


        /* Books */
        case TV_MAGIC_BOOK:
        case TV_PRAYER_BOOK:
        case TV_NATURE_BOOK:
        case TV_SHADOW_BOOK:
        case TV_OTHER_BOOK:

        /* Skip incorrect books (if we can browse this book, it is good) */
        if (!obj_kind_can_browse(&k_info[item->kind])) break;

        /* Count the books */
        num_book[item->sval] += item->iqty;

        break;


        /* Food */
        case TV_FOOD:

        if (item->sval == sv_food_ration)
            num_food += item->iqty;
        else if (item->sval == sv_food_slime_mold)
            num_mold += item->iqty;
        else if (item->sval == sv_mush_purging)
        {
            num_fix_stat[STAT_CON] += item->iqty;
            num_fix_stat[STAT_STR] += item->iqty;
        }
        else if (item->sval == sv_mush_restoring)
        {
            num_fix_stat[STAT_STR] += item->iqty;
            num_fix_stat[STAT_INT] += item->iqty;
            num_fix_stat[STAT_WIS] += item->iqty;
            num_fix_stat[STAT_DEX] += item->iqty;
            num_fix_stat[STAT_CON] += item->iqty;
        }
        break;


        /* Potions */
        case TV_POTION:

        /* Analyze */
        if (item->sval == sv_potion_cure_critical)
            num_cure_critical += item->iqty;
        else if (item->sval == sv_potion_cure_serious)
            num_cure_serious += item->iqty;
        else if (item->sval == sv_potion_resist_heat)
            num_pot_rheat += item->iqty;
        else if (item->sval == sv_potion_resist_cold)
            num_pot_rcold += item->iqty;
        else if (item->sval == sv_potion_restore_life)
            num_fix_exp += item->iqty;
        else if (item->sval == sv_potion_restore_mana)
            num_mana += item->iqty;
        else if (item->sval == sv_potion_healing)
        {
            num_heal += item->iqty;
            if (!in_item && !no_items) num_heal_true += item->iqty;
        }
        else if (item->sval == sv_potion_star_healing)
        {
            num_ezheal += item->iqty;
            if (!in_item && !no_items) num_ezheal_true += item->iqty;
        }
        else if (item->sval == sv_potion_life)
        {
            num_life += item->iqty;
            if (!in_item && !no_items) num_life_true += item->iqty;
        }
        else if (item->sval == sv_potion_berserk)
            num_berserk += item->iqty;
        else if (item->sval == sv_potion_speed)
            num_speed += item->iqty;

        break;


        /* Scrolls */
        case TV_SCROLL:

        /* Analyze the scroll */
        if (item->sval == sv_scroll_identify)
            num_ident += item->iqty;
        else if (item->sval == sv_scroll_phase_door)
            num_phase += item->iqty;
        else if (item->sval == sv_scroll_teleport)
            num_teleport += item->iqty;
        else if (item->sval == sv_scroll_word_of_recall)
            num_recall += item->iqty;
        else if (item->sval == sv_scroll_enchant_armor)
            num_enchant_to_a += item->iqty;
        else if (item->sval == sv_scroll_enchant_weapon_to_hit)
            num_enchant_to_h += item->iqty;
        else if (item->sval == sv_scroll_enchant_weapon_to_dam)
            num_enchant_to_d += item->iqty;
        else if (item->sval == sv_scroll_protection_from_evil)
            num_pfe += item->iqty;
        else if (item->sval == sv_scroll_rune_of_protection)
            num_glyph += item->iqty;
        else if (item->sval == sv_scroll_teleport_level)
            num_teleport_level += item->iqty;
        else if (item->sval == sv_scroll_recharging)
            num_recharge += item->iqty;
        else if (item->sval == sv_scroll_mass_banishment)
            num_mass_genocide += item->iqty;

        break;


        /* Rods */
        case TV_ROD:

        /* Analyze */
        if (item->sval == sv_rod_recall)
            num_recall += item->iqty * 100;

        break;


        /* Staffs */
        case TV_STAFF:

        /* only collect staves with more than 3 charges at high level */
        if (item->pval <= 3 && borg_skill[BI_CLEVEL] > 30)
            break;

        /* Analyze */
        if (item->sval == sv_staff_teleportation)
        {
            num_escape += item->pval * item->iqty;
            num_tele_staves++;
        }

        break;


        /* Missiles */
        case TV_SHOT:
        case TV_ARROW:
        case TV_BOLT:

        /* Hack -- ignore invalid missiles */
        if (item->tval != borg_skill[BI_AMMO_TVAL] ) break;

        /* Hack -- ignore worthless missiles */
        if (item->value <= 0) break;

        /* Count them */
        num_missile += item->iqty;

        break;
        }

        /* if only doing one item, break. */
        if (in_item) break;
    }


    /*** Process the Spells and Prayers ***/

    /* Handle "satisfy hunger" -> infinite food */
    if (borg_spell_legal(REMOVE_HUNGER) || borg_spell_legal(HERBAL_CURING))
    {
        num_food += 1000;
    }

    /* Handle "identify" -> infinite identifies */
    if (borg_spell_legal(IDENTIFY_RUNE))
    {
        num_ident += 1000;
    }

    /* Handle ENCHANT_WEAPON */
    if (borg_spell_legal_fail(ENCHANT_WEAPON, 65))
    {
        num_enchant_to_h += 1000;
        num_enchant_to_d += 1000;
    }

    /*  Handle PROTECTION_FROM_EVIL */
    if (borg_spell_legal(PROTECTION_FROM_EVIL))
    {
        num_pfe += 1000;
    }

    /*  Handle "rune of protection" glyph */
    if (borg_spell_legal(GLYPH_OF_WARDING) ||
        borg_equips_item(act_glyph, false))
    {
        num_glyph += 1000;
    }

    /* handle restore */

    /* Handle recall */
    if (borg_spell_legal(WORD_OF_RECALL))
    {
        num_recall += 1000;
    }

    /* Handle teleport_level */
    if (borg_spell_legal(TELEPORT_LEVEL))
    {
        num_teleport_level += 1000;
    }

    /* Handle recharge */
    if (borg_spell_legal(RECHARGING))
    {
        num_recharge += 1000;
    }

    /*** Process the Needs ***/

    /* Hack -- No need for stat repair */
    if (borg_skill[BI_SSTR]) num_fix_stat[STAT_STR] += 1000;
    if (borg_skill[BI_SINT]) num_fix_stat[STAT_INT] += 1000;
    if (borg_skill[BI_SWIS]) num_fix_stat[STAT_WIS] += 1000;
    if (borg_skill[BI_SDEX]) num_fix_stat[STAT_DEX] += 1000;
    if (borg_skill[BI_SCON]) num_fix_stat[STAT_CON] += 1000;

    /* Extract the player flags */
    player_flags(player, f);

    /* Good flags */
    if (of_has(f, OF_SLOW_DIGEST)) num_slow_digest++;
    if (of_has(f, OF_FEATHER)) num_ffall++;
    if (of_has(f, OF_LIGHT_2) || rf_has(f, OF_LIGHT_3)) num_LIGHT++;
    if (of_has(f, OF_REGEN)) num_regenerate++;
    if (of_has(f, OF_TELEPATHY)) num_telepathy++;
    if (of_has(f, OF_SEE_INVIS)) num_see_inv++;
    if (of_has(f, OF_FREE_ACT)) num_free_act++;
    if (of_has(f, OF_HOLD_LIFE)) num_hold_life++;
    if (of_has(f, OF_PROT_CONF)) num_resist_conf++;
    if (of_has(f, OF_PROT_BLIND)) num_resist_blind++;

    /* Weird flags */

    /* Bad flags */

    /* Immunity flags */
    if (player->race->el_info[ELEM_FIRE].res_level == 3) num_immune_fire++;
    if (player->race->el_info[ELEM_ACID].res_level == 3) num_immune_acid++;
    if (player->race->el_info[ELEM_COLD].res_level == 3) num_immune_cold++;
    if (player->race->el_info[ELEM_ELEC].res_level == 3) num_immune_elec++;

    /* Resistance flags */
    if (player->race->el_info[ELEM_ACID].res_level > 0) num_resist_acid++;
    if (player->race->el_info[ELEM_ELEC].res_level > 0) num_resist_elec++;
    if (player->race->el_info[ELEM_FIRE].res_level > 0) num_resist_fire++;
    if (player->race->el_info[ELEM_COLD].res_level > 0) num_resist_cold++;
    if (player->race->el_info[ELEM_POIS].res_level > 0) num_resist_pois++;
    if (player->race->el_info[ELEM_LIGHT].res_level > 0) num_resist_LIGHT++;
    if (player->race->el_info[ELEM_DARK].res_level > 0) num_resist_dark++;
    if (player->race->el_info[ELEM_SOUND].res_level > 0) num_resist_sound++;
    if (player->race->el_info[ELEM_SHARD].res_level > 0) num_resist_shard++;
    if (player->race->el_info[ELEM_NEXUS].res_level > 0) num_resist_nexus++;
    if (player->race->el_info[ELEM_NETHER].res_level > 0) num_resist_neth++;
    if (player->race->el_info[ELEM_CHAOS].res_level > 0) num_resist_chaos++;
    if (player->race->el_info[ELEM_DISEN].res_level > 0) num_resist_disen++;

    /* Sustain flags */
    if (rf_has(f, OF_SUST_STR)) num_sustain_str++;
    if (rf_has(f, OF_SUST_INT)) num_sustain_int++;
    if (rf_has(f, OF_SUST_WIS)) num_sustain_wis++;
    if (rf_has(f, OF_SUST_DEX)) num_sustain_dex++;
    if (rf_has(f, OF_SUST_CON)) num_sustain_con++;

}

/*
 * Extract the bonuses for items in the home.
 *
 * in_item is passed in if you want to pretend that in_item is
 *          the only item in the home.
 * no_items is passed in as true if you want to pretend that the
 *          home is empty.
 */
void borg_notice_home(borg_item* in_item, bool no_items)
{
    /* Notice the home equipment */
    borg_notice_home_clear(in_item, no_items);

    /* Notice the home inventory */
    borg_notice_home_aux(in_item, no_items);
}

static bool borg_feature_protected(borg_grid* ag)
{
    if (ag->glyph || ag->kill ||
        ((ag->feat >= FEAT_CLOSED) && (ag->feat <= FEAT_PERM)))
        return true;
    return false;
}

/*
 * Helper function -- calculate "power" of equipment
 */
static int32_t borg_power_aux1(void)
{
    int         hold;
    int         damage, dam;

    int         i;

    int         cur_wgt = 0;
    int         max_wgt = 0;

    int32_t        value = 0L;

    borg_item* item;


    /* Obtain the "hold" value (weight limit for weapons) */
    hold = adj_str_hold[my_stat_ind[STAT_STR]];

    /*** Analyze weapon ***/

    /* Examine current weapon */
    item = &borg_items[INVEN_WIELD];

    /* We give a bonus to wearing an unID'D sword in order to use it and
     * garner a pseudoID from it.  We do not do this late in the game though
     * because our weapon often has traits that we need in order to be deep (FA, SeeInvis)
     */
    if (borg_skill[BI_CDEPTH] < 10 && borg_skill[BI_MAXCLEVEL] < 15 &&
        item->iqty && item->ident != true) value += 1000000;

    /* Calculate "average" damage per "normal" blow  */
    /* and assume we can enchant up to +8 if borg_skill[BI_CLEVEL] > 25 */
    damage = (item->dd * item->ds * 20L);


    /* Reward "damage" and increased blows per round*/
    value += damage * (borg_skill[BI_BLOWS] + 1);

    /* Reward "bonus to hit" */
    value += ((borg_skill[BI_TOHIT] + item->to_h) * 100L);

    /* Reward "bonus to dam" */
    value += ((borg_skill[BI_TODAM] + item->to_d) * 30L);

    /* extra damage for some */
    if (borg_cfg[BORG_WORSHIPS_DAMAGE])
    {
        value += ((borg_skill[BI_TOHIT] + item->to_h) * 15L);
    }

    /* extra boost for deep dungeon */
    if (borg_skill[BI_MAXDEPTH] >= 75)
    {
        value += ((borg_skill[BI_TOHIT] + item->to_h) * 15L);

        value += item->dd *
            item->ds * 20L *
            2 * borg_skill[BI_BLOWS];
    }

    /* assume 2x base damage for x% of creatures */
    dam = damage * 2 * borg_skill[BI_BLOWS];
    if (borg_skill[BI_WS_ANIMAL]) value += (dam * 2) / 2;
    if (borg_skill[BI_WS_EVIL])   value += (dam * 7) / 2;

    /* extra damage for some */
    if (borg_cfg[BORG_WORSHIPS_DAMAGE])
    {
        value += (dam);
    }

    /* assume 3x base damage for x% of creatures */
    dam = damage * 3 * borg_skill[BI_BLOWS];
    if (borg_skill[BI_WS_UNDEAD] && (!borg_skill[BI_WK_UNDEAD])) value += (dam * 5) / 2;
    if (borg_skill[BI_WS_DEMON] && (!borg_skill[BI_WK_DEMON]))  value += (dam * 3) / 2;
    if (borg_skill[BI_WS_DRAGON] && (!borg_skill[BI_WK_DRAGON])) value += (dam * 6) / 2;
    if (borg_skill[BI_WS_GIANT])  value += (dam * 4) / 2;
    if (borg_skill[BI_WB_ACID])  value += (dam * 4) / 2;
    if (borg_skill[BI_WB_ELEC])  value += (dam * 5) / 2;
    if (borg_skill[BI_WB_FIRE])  value += (dam * 3) / 2;
    if (borg_skill[BI_WB_COLD])  value += (dam * 3) / 2;
    /* SOrc and STroll get 1/2 of reward now */
    if (borg_skill[BI_WS_ORC])    value += (dam * 1) / 2;
    if (borg_skill[BI_WS_TROLL])  value += (dam * 2) / 2;
    /* and the other 2/2 if SEvil not possesed */
    if (borg_skill[BI_WS_ORC] && !borg_skill[BI_WS_EVIL])    value += (dam * 1) / 2;
    if (borg_skill[BI_WS_TROLL] && !borg_skill[BI_WS_EVIL])  value += (dam * 1) / 2;

    /* extra damage for some */
    if (borg_cfg[BORG_WORSHIPS_DAMAGE])
    {
        value += (dam);
    }

    /* assume 5x base damage for x% of creatures */
    dam = damage * 5 * borg_skill[BI_BLOWS];
    if (borg_skill[BI_WK_UNDEAD]) value += (dam * 5) / 2;
    if (borg_skill[BI_WK_DEMON]) value += (dam * 5) / 2;
    if (borg_skill[BI_WK_DRAGON]) value += (dam * 5) / 2;
    /* extra damage for some */
    if (borg_cfg[BORG_WORSHIPS_DAMAGE])
    {
        value += (dam);
    }

    /* It used to be only on Grond */
    if (borg_skill[BI_W_IMPACT]) value += 50L;

    /* Hack -- It is hard to hold a heavy weapon */
    if (borg_skill[BI_HEAVYWEPON]) value -= 500000L;

    /* HACK -- Borg worships num_blow, even on broken swords. */
    /* kind 47 is a broken sword usually 1d2 in damage */
    /* if (item->kind == 47 || item->kind == 30 ||item->kind == 390 ) value -=90000L; */


    /* We want low level borgs to have high blows (dagger, whips) */
    if (borg_skill[BI_CLEVEL] <= 10) value += borg_skill[BI_BLOWS] * 45000L;

    /*** Analyze bow ***/

    /* Examine current bow */
    item = &borg_items[INVEN_BOW];

    /* We give a bonus to wearing an unID'D bow in order to use it and
     * garner a pseudoID from it.  We do not do this late in the game though
     * because our weapon often has traits that we need in order to be deep (FA, SeeInvis)
     */
    if (borg_skill[BI_CDEPTH] < 10 && borg_skill[BI_MAXCLEVEL] < 15 &&
        item->iqty && item->ident != true) value += 6000000;

    /* Calculate "average" damage per "normal" shot (times 2) */
    if (item->to_d > 8 || borg_skill[BI_CLEVEL] < 25)
        damage = ((borg_skill[BI_AMMO_SIDES])+(item->to_d)) * borg_skill[BI_AMMO_POWER];
    else
        damage = (borg_skill[BI_AMMO_SIDES] + 8) * borg_skill[BI_AMMO_POWER];

    /* Reward "damage" */
    if (borg_cfg[BORG_WORSHIPS_DAMAGE])
    {
        value += (borg_skill[BI_SHOTS] * damage * 11L);
    }
    else
    {
        value += (borg_skill[BI_SHOTS] * damage * 9L);
    }

    /* Extra bonus for low levels, they need a ranged weap */
    if (borg_skill[BI_CLEVEL] < 15) value += (borg_skill[BI_SHOTS] * damage * 200L);


    /* slings force you to carry heavy ammo.  Penalty for that unles you have lots of str  */
    if (item->sval == sv_sling &&
        !item->art_idx &&
        my_stat_ind[STAT_STR] < 9)
    {
        value -= 5000L;
    }

    /* Bonus if level 1 to buy a sling, they are cheap ranged weapons */
    if (item->sval == sv_sling && borg_skill[BI_CLEVEL] == 1 && my_stat_ind[STAT_STR] >= 9) value += 8000;


    /* Reward "bonus to hit" */
    value += ((borg_skill[BI_TOHIT] + item->to_h) * 100L);;

    /* extra damage for some */
    if (borg_cfg[BORG_WORSHIPS_DAMAGE])
    {
        value += ((borg_skill[BI_TOHIT] + item->to_h) * 25L);
    }


    /* Prefer bows */
    if (player_has(player, PF_FAST_SHOT) && borg_skill[BI_AMMO_TVAL]  == TV_ARROW) value += 30000L;

    /* Hack -- It is hard to hold a heavy weapon */
    if (hold < item->weight / 10) value -= 500000L;



    /***  Analyze dragon armour  ***/

    /* Examine current armor */
    item = &borg_items[INVEN_BODY];

    if (item->tval == TV_DRAG_ARMOR && !item->art_idx)
    {
        if (item->sval == sv_dragon_black ||
            item->sval == sv_dragon_blue ||
            item->sval == sv_dragon_white ||
            item->sval == sv_dragon_red)
            value += 1100;
        else if (item->sval == sv_dragon_green)
            value += 2750;
        else if (item->sval == sv_dragon_multihued)
            value += 3250;
        else if (item->sval == sv_dragon_shining ||
            item->sval == sv_dragon_law ||
            item->sval == sv_dragon_gold ||
            item->sval == sv_dragon_chaos ||
            item->sval == sv_dragon_balance ||
            item->sval == sv_dragon_power)
            value += 5150;
    }

    /*** Examine the Rings for special types ***/
    for (i = INVEN_RIGHT; i <= INVEN_LEFT; i++)
    {
        /* Obtain the item */
        item = &borg_items[i];

        /* Reward the [Elemental] protection rings for their activation */
        if (item->sval == sv_ring_flames)   value += 25000;
        if (item->sval == sv_ring_acid)	    value += 10000;
        if (item->sval == sv_ring_ice)	    value += 15000;
        if (item->sval == sv_ring_lightning) value += 10000;
    }

    /*** Reward various things ***/

    /* Hack -- Reward light radius */
    if (borg_skill[BI_CURLITE] <= 3) value += (borg_skill[BI_CURLITE] * 10000L);
    if (borg_skill[BI_CURLITE] > 3) value += (30000L) + (borg_skill[BI_CURLITE] * 1000);

    value += borg_skill[BI_MOD_MOVES] * (3000L);
    value += borg_skill[BI_DAM_RED] * (10000L);

    /* Hack -- Reward speed
     * see if speed can be a bonus if good speed; not +3.
     * reward higher for +10 than +50 speed (decreased return).
     */
    if (borg_cfg[BORG_WORSHIPS_SPEED])
    {
        if (borg_skill[BI_SPEED] >= 150)
            value += (((borg_skill[BI_SPEED] - 120) * 1500L) + 185000L);

        if (borg_skill[BI_SPEED] >= 145 && borg_skill[BI_SPEED] <= 149)
            value += (((borg_skill[BI_SPEED] - 120) * 1500L) + 180000L);

        if (borg_skill[BI_SPEED] >= 140 && borg_skill[BI_SPEED] <= 144)
            value += (((borg_skill[BI_SPEED] - 120) * 1500L) + 175000L);

        if (borg_skill[BI_SPEED] >= 135 && borg_skill[BI_SPEED] <= 139)
            value += (((borg_skill[BI_SPEED] - 120) * 1500L) + 175000L);

        if (borg_skill[BI_SPEED] >= 130 && borg_skill[BI_SPEED] <= 134)
            value += (((borg_skill[BI_SPEED] - 120) * 1500L) + 160000L);

        if (borg_skill[BI_SPEED] >= 125 && borg_skill[BI_SPEED] <= 129)
            value += (((borg_skill[BI_SPEED] - 110) * 1500L) + 135000L);

        if (borg_skill[BI_SPEED] >= 120 && borg_skill[BI_SPEED] <= 124)
            value += (((borg_skill[BI_SPEED] - 110) * 1500L) + 110000L);

        if (borg_skill[BI_SPEED] >= 115 && borg_skill[BI_SPEED] <= 119)
            value += (((borg_skill[BI_SPEED] - 110) * 1500L) + 85000L);

        if (borg_skill[BI_SPEED] >= 110 && borg_skill[BI_SPEED] <= 114)
            value += (((borg_skill[BI_SPEED] - 110) * 1500L) + 65000L);
        else
            value += (((borg_skill[BI_SPEED] - 110) * 2500L));
    }
    else
    {
        if (borg_skill[BI_SPEED] >= 140)
            value += (((borg_skill[BI_SPEED] - 120) * 1000L) + 175000L);

        if (borg_skill[BI_SPEED] >= 135 && borg_skill[BI_SPEED] <= 139)
            value += (((borg_skill[BI_SPEED] - 120) * 1000L) + 165000L);

        if (borg_skill[BI_SPEED] >= 130 && borg_skill[BI_SPEED] <= 134)
            value += (((borg_skill[BI_SPEED] - 120) * 1000L) + 150000L);

        if (borg_skill[BI_SPEED] >= 125 && borg_skill[BI_SPEED] <= 129)
            value += (((borg_skill[BI_SPEED] - 110) * 1000L) + 125000L);

        if (borg_skill[BI_SPEED] >= 120 && borg_skill[BI_SPEED] <= 124)
            value += (((borg_skill[BI_SPEED] - 110) * 1000L) + 100000L);

        if (borg_skill[BI_SPEED] >= 115 && borg_skill[BI_SPEED] <= 119)
            value += (((borg_skill[BI_SPEED] - 110) * 1000L) + 75000L);

        if (borg_skill[BI_SPEED] >= 110 && borg_skill[BI_SPEED] <= 114)
            value += (((borg_skill[BI_SPEED] - 110) * 1000L) + 55000L);
        else
            value += (((borg_skill[BI_SPEED] - 110) * 2500L));
    }


    /* Hack -- Reward strength bonus */
    value += (my_stat_ind[STAT_STR] * 100L);

    /* Hack -- Reward spell stat bonus */
    int spell_stat = borg_spell_stat();
    if (spell_stat >= 0)
    {
        if (my_stat_ind[spell_stat] <= 37)
        {
            value += (my_stat_ind[spell_stat] * 500L);

            /* Bonus for sp. */
            if (borg_cfg[BORG_WORSHIPS_MANA])
            {
                value += ((borg_adj_mag_mana[my_stat_ind[spell_stat]] * borg_skill[BI_CLEVEL]) / 2) * 255L;
            }
            else
            {
                value += ((borg_adj_mag_mana[my_stat_ind[spell_stat]] * borg_skill[BI_CLEVEL]) / 2) * 155L;
            }

            /* bonus for fail rate */
            value += spell_chance(0) * 100;

            /* should try to get min fail to 0 */
            if (player_has(player, PF_ZERO_FAIL))
            {
                /* other fail rates */
                if (spell_chance(0) < 1)
                    value += 30000L;
            }
        }
    }

    /* Dexterity Bonus --good for attacking and ac*/
    if (my_stat_ind[STAT_DEX] <= 37)
    {
        /* Hack -- Reward bonus */
        value += (my_stat_ind[STAT_DEX] * 120L);
    }

    /* Constitution Bonus */
    if (my_stat_ind[STAT_CON] <= 37)
    {
        int bonus_hp = player->player_hp[player->lev - 1] + borg_adj_con_mhp[my_stat_ind[STAT_CON]] * borg_skill[BI_CLEVEL] / 100;

        if (borg_cfg[BORG_WORSHIPS_HP])
        {
            value += (my_stat_ind[STAT_CON] * 250L);
            /* Hack -- Reward hp bonus */
            /*         This is a bit wierd because we are not really giving a bonus for */
            /*         what hp you have, but for the 'bonus' hp you get */
            /*         getting over 800hp is very important. */
            if (bonus_hp < 800)
                value += bonus_hp * 450L;
            else
                value += (bonus_hp - 800) * 100L + (350L * 500);
        }
        else /*does not worship hp */
        {
            value += (my_stat_ind[STAT_CON] * 150L);
            /* Hack -- Reward hp bonus */
            /*         This is a bit wierd because we are not really giving a bonus for */
            /*         what hp you have, but for the 'bonus' hp you get */
            /*         getting over 500hp is very important. */
            if (bonus_hp < 500)
                value += bonus_hp * 350L;
            else
                value += (bonus_hp - 500) * 100L + (350L * 500);
        }

    }

    /* HACK - a small bonus for adding to stats even above max. */
    /*        This will allow us to swap a ring of int +6 for */
    /*        our ring of int +2 even though we are at max int because */
    /*        we are wielding a weapon that has +4 int */
    /*        later it might be nice to swap to a weapon that does not */
    /*        have an int bonus */
    for (i = 0; i < STAT_MAX; i++) value += my_stat_add[i];

    /* Hack -- tiny rewards */
    value += (borg_skill[BI_DISP] * 2L);
    value += (borg_skill[BI_DISM] * 2L);
    value += (borg_skill[BI_DEV] * 25L);
    value += (borg_skill[BI_SAV] * 25L);
    /* perfect saves are very nice */
    if (borg_skill[BI_SAV] > 99)
        value += 10000;
    value += (borg_skill[BI_STL] * 2L);
    value += (borg_skill[BI_SRCH] * 1L);
    value += (borg_skill[BI_THN] * 5L);
    value += (borg_skill[BI_THB] * 35L);
    value += (borg_skill[BI_THT] * 2L);
    value += (borg_skill[BI_DIG] * 2L);


    /*** Reward current flags ***/

    /* Various flags */
    if (borg_skill[BI_SDIG]) value += 750L;
    if (borg_skill[BI_SDIG] && borg_skill[BI_ISHUNGRY]) value += 7500L;
    if (borg_skill[BI_SDIG] && borg_skill[BI_ISWEAK]) value += 7500L;

    /* Feather Fall if low level is nice */
    if (borg_skill[BI_MAXDEPTH] < 20)
    {
        if (borg_skill[BI_FEATH]) value += 500L;
    }
    else
    {
        if (borg_skill[BI_FEATH]) value += 50;
    }

    if (borg_skill[BI_LIGHT]) value += 2000L;

    if (borg_skill[BI_ESP])
    {
        if (borg_skill[BI_SINV]) value += 500L;
    }

    if (!borg_skill[BI_DINV])
    {
        if (borg_skill[BI_SINV]) value += 5000L;
    }

    if (borg_skill[BI_FRACT]) value += 10000L;

    /* after you max out you are pretty safe from drainers.*/
    if (borg_skill[BI_MAXCLEVEL] < 50)
    {
        if (borg_skill[BI_HLIFE]) value += 2000L;
    }
    else
    {
        if (borg_skill[BI_HLIFE]) value += 200L;
    }
    if (borg_skill[BI_REG]) value += 2000L;
    if (borg_skill[BI_ESP]) value += 80000L;

    /* Immunity flags */
    if (borg_skill[BI_ICOLD]) value += 65000L;
    if (borg_skill[BI_IELEC]) value += 40000L;
    if (borg_skill[BI_IFIRE]) value += 80000L;
    if (borg_skill[BI_IACID]) value += 50000L;

    /* Resistance flags */
    if (borg_skill[BI_RCOLD]) value += 3000L;
    if (borg_skill[BI_RELEC]) value += 4000L;
    if (borg_skill[BI_RACID]) value += 6000L;
    if (borg_skill[BI_RFIRE]) value += 8000L;
    /* extra bonus for getting all basic resist */
    if (borg_skill[BI_RFIRE] &&
        borg_skill[BI_RACID] &&
        borg_skill[BI_RELEC] &&
        borg_skill[BI_RCOLD]) value += 10000L;
    if (borg_skill[BI_RPOIS]) value += 20000L;
    if (borg_skill[BI_RSND]) value += 3500L;
    if (borg_skill[BI_RLITE]) value += 800L;
    if (borg_skill[BI_RDARK]) value += 800L;
    if (borg_skill[BI_RKAOS]) value += 5000L;

    /* this is way boosted to avoid carrying stuff you don't need */
    if (borg_skill[BI_RCONF]) value += 80000L;

    /* mages need a slight boost for this */
    if (borg_class == CLASS_MAGE && borg_skill[BI_RCONF]) value += 2000L;
    if (borg_skill[BI_RDIS]) value += 5000L;
    if (borg_skill[BI_RSHRD]) value += 100L;
    if (borg_skill[BI_RNXUS]) value += 100L;
    if (borg_skill[BI_RBLIND]) value += 5000L;
    if (borg_skill[BI_RNTHR]) value += 5500L;
    if (borg_skill[BI_RFEAR]) value += 2000L;

    /* Sustain flags */
    if (borg_skill[BI_SSTR]) value += 50L;
    if (borg_skill[BI_SINT]) value += 50L;
    if (borg_skill[BI_SWIS]) value += 50L;
    if (borg_skill[BI_SCON]) value += 50L;
    if (borg_skill[BI_SDEX]) value += 50L;
    /* boost for getting them all */
    if (borg_skill[BI_SSTR] &&
        borg_skill[BI_SINT] &&
        borg_skill[BI_SWIS] &&
        borg_skill[BI_SDEX] &&
        borg_skill[BI_SCON])  value += 1000L;


    /*** XXX XXX XXX Reward "necessary" flags ***/

    /* Mega-Hack -- See invisible (level 10) */
    if ((borg_skill[BI_SINV] || borg_skill[BI_ESP]) && (borg_skill[BI_MAXDEPTH] + 1 >= 10)) value += 100000L;


    /* Mega-Hack -- Free action (level 20) */
    if (borg_skill[BI_FRACT] && (borg_skill[BI_MAXDEPTH] + 1 >= 20)) value += 100000L;


    /*  Mega-Hack -- resists (level 25) */
    if (borg_skill[BI_RFIRE] && (borg_skill[BI_MAXDEPTH] + 1 >= 25)) value += 100000L;


    /*  Mega-Hack -- resists (level 40) */
    if (borg_skill[BI_RPOIS] && (borg_skill[BI_MAXDEPTH] + 1 >= 40)) value += 100000L;
    if (borg_skill[BI_RELEC] && (borg_skill[BI_MAXDEPTH] + 1 >= 40)) value += 100000L;
    if (borg_skill[BI_RACID] && (borg_skill[BI_MAXDEPTH] + 1 >= 40)) value += 100000L;
    if (borg_skill[BI_RCOLD] && (borg_skill[BI_MAXDEPTH] + 1 >= 40)) value += 100000L;


    /*  Mega-Hack -- Speed / Hold Life (level 46) and maxed out */
    if ((borg_skill[BI_HLIFE] && (borg_skill[BI_MAXDEPTH] + 1 >= 46) && (borg_skill[BI_MAXCLEVEL] < 50))) value += 100000L;
    if ((borg_skill[BI_SPEED] >= 115) && (borg_skill[BI_MAXDEPTH] + 1 >= 46)) value += 100000L;
    if (borg_skill[BI_RCONF] && (borg_skill[BI_MAXDEPTH] + 1 >= 46)) value += 100000L;

    /*  Mega-Hack -- resist Nether is -very- nice to have at level 50 */
    if (borg_skill[BI_RNTHR] && (borg_skill[BI_MAXDEPTH] + 1 >= 50)) value += 55000L;

    /*  Mega-Hack -- resist Sound to avoid being KO'd */
    if (borg_skill[BI_RSND] && (borg_skill[BI_MAXDEPTH] + 1 >= 50)) value += 100000L;

    /*  Mega-Hack -- resists & Telepathy (level 55) */
    if (borg_skill[BI_RBLIND] && (borg_skill[BI_MAXDEPTH] + 1 >= 55)) value += 100000L;
    if (borg_skill[BI_ESP] && (borg_skill[BI_MAXDEPTH] + 1 >= 55)) value += 100000L;
    if (borg_skill[BI_RNTHR] && (borg_skill[BI_MAXDEPTH] + 1 >= 60)) value += 55000L;


    /*  Mega-Hack -- resists & +10 speed (level 60) */
    if (borg_skill[BI_RKAOS] && (borg_skill[BI_MAXDEPTH] + 1 >= 60)) value += 104000L;
    if (borg_skill[BI_RDIS] && (borg_skill[BI_MAXDEPTH] + 1 >= 60)) value += 90000L;
    if ((borg_skill[BI_SPEED] >= 120) && (borg_skill[BI_MAXDEPTH] + 1 >= 60)) value += 100000L;

    /*  Must have +20 speed (level 80) */
    if ((borg_skill[BI_SPEED] >= 130) && (borg_skill[BI_MAXDEPTH] + 1 >= 80)) value += 100000L;

    /* Not Req, but a good idea:
     * Extra boost to Nether deeper down
     * RDark for deeper uniques
     * Good to have +30 speed
     */
    if (borg_skill[BI_RNTHR] && (borg_skill[BI_MAXDEPTH] + 1 >= 80)) value += 15000L;
    if (borg_skill[BI_RDARK] && (borg_skill[BI_MAXDEPTH] + 1 >= 80)) value += 25000L;
    if ((borg_skill[BI_SPEED] >= 140) && (borg_skill[BI_MAXDEPTH] + 1 >= 80) &&
        borg_class == CLASS_WARRIOR)                value += 100000L;


    /*** Reward powerful armor ***/

    /* Reward armor */
    if (borg_cfg[BORG_WORSHIPS_AC])
    {
        if (borg_skill[BI_ARMOR] < 15) value += ((borg_skill[BI_ARMOR]) * 2500L);
        if (borg_skill[BI_ARMOR] >= 15 && borg_skill[BI_ARMOR] < 75) value += ((borg_skill[BI_ARMOR]) * 2000L) + 28250L;
        if (borg_skill[BI_ARMOR] >= 75) value += ((borg_skill[BI_ARMOR]) * 1500L) + 73750L;
    }
    else
    {
        if (borg_skill[BI_ARMOR] < 15)   value += ((borg_skill[BI_ARMOR]) * 2000L);
        if (borg_skill[BI_ARMOR] >= 15 &&
            borg_skill[BI_ARMOR] < 75)   value += ((borg_skill[BI_ARMOR]) * 500L) + 28350L;
        if (borg_skill[BI_ARMOR] >= 75) value += ((borg_skill[BI_ARMOR]) * 100L) + 73750L;
    }


    /* Hack-- Reward the borg for carrying a NON-ID items that have random powers
     */
    if (amt_ego ||
        ((borg_ego_has_random_power(&e_info[item->ego_idx]) &&
            !borg_items[INVEN_OUTER].ident))) value += 999999L;

    /*** Penalize various things ***/

    /* Penalize various flags */
    if (borg_skill[BI_CRSAGRV]) value -= 800000L;
    if (borg_skill[BI_CRSHPIMP]) value -= 35000;
    if (borg_class != CLASS_WARRIOR && borg_skill[BI_CRSMPIMP]) value -= 15000;
    if ((borg_class == CLASS_MAGE ||
        borg_class == CLASS_PRIEST ||
        borg_class == CLASS_DRUID ||
        borg_class == CLASS_NECROMANCER) &&
        borg_skill[BI_CRSMPIMP]) value -= 15000;
    if (borg_skill[BI_CRSFEAR]) value -= 400000L;
    if (borg_skill[BI_CRSFEAR] && borg_class != CLASS_MAGE) value -= 200000L;
    if (borg_skill[BI_CRSDRAIN_XP]) value -= 400000L;
    if (borg_skill[BI_CRSFVULN]) value -= 30000;
    if (borg_skill[BI_CRSEVULN]) value -= 30000;
    if (borg_skill[BI_CRSCVULN]) value -= 30000;
    if (borg_skill[BI_CRSAVULN]) value -= 30000;

    if (borg_skill[BI_CRSTELE]) value -= 100000L;
    if (borg_skill[BI_CRSENVELOPING]) value -= 50000L;
    if (borg_skill[BI_CRSIRRITATION]) value -= 20000L;
    if (borg_skill[BI_CRSPOIS]) value -= 10000L;
    if (borg_skill[BI_CRSSIREN]) value -= 800000L;
    if (borg_skill[BI_CRSHALU]) value -= 100000L;
    if (borg_skill[BI_CRSPARA]) value -= 800000L;
    if (borg_skill[BI_CRSSDEM]) value -= 100000L;
    if (borg_skill[BI_CRSSDRA]) value -= 100000L;
    if (borg_skill[BI_CRSSUND]) value -= 100000L;
    if (borg_skill[BI_CRSSTONE] && borg_skill[BI_SPEED] < 20) value -= 10000L;
    if (borg_skill[BI_CRSSTEELSKIN] && borg_skill[BI_SPEED] < 20) value -= 10000L;
    if (borg_skill[BI_CRSNOTEL]) value -= 700000L;
    if (borg_skill[BI_CRSTWEP]) value -= 100000L;
    if (borg_skill[BI_CRSAIRSWING]) value -= 10000L;
    if (borg_skill[BI_CRSUNKNO]) value -= 9999999L;

    /*** Penalize armor weight ***/
    if (my_stat_ind[STAT_STR] < 15)
    {
        if (borg_items[INVEN_BODY].weight > 200)
            value -= (borg_items[INVEN_BODY].weight - 200) * 15;
        if (borg_items[INVEN_HEAD].weight > 30)
            value -= 250;
        if (borg_items[INVEN_ARM].weight > 10)
            value -= 250;
        if (borg_items[INVEN_FEET].weight > 50)
            value -= 250;
    }

    /* Compute the total armor weight */
    cur_wgt += borg_items[INVEN_BODY].weight;
    cur_wgt += borg_items[INVEN_HEAD].weight;
    cur_wgt += borg_items[INVEN_ARM].weight;
    cur_wgt += borg_items[INVEN_OUTER].weight;
    cur_wgt += borg_items[INVEN_HANDS].weight;
    cur_wgt += borg_items[INVEN_FEET].weight;

    /* Determine the weight allowance */
    max_wgt = player->class->magic.spell_weight;

    /* Hack -- heavy armor hurts magic */
    if (player->class->magic.total_spells && ((cur_wgt - max_wgt) / 10) > 0)
    {
        /* max sp must be calculated in case it changed with the armor */
        int lvl = borg_skill[BI_CLEVEL] - player->class->magic.spell_first + 1;
        int max_sp = borg_adj_mag_mana[my_stat_ind[borg_spell_stat()]] * lvl / 100 + 1;
        max_sp -= ((cur_wgt - max_wgt) / 10);
        /* Mega-Hack -- Penalize heavy armor which hurts mana */
        if (max_sp >= 300 && max_sp <= 350)
            value -= (((cur_wgt - max_wgt) / 10) * 400L);
        if (max_sp >= 200 && max_sp <= 299)
            value -= (((cur_wgt - max_wgt) / 10) * 800L);
        if (max_sp >= 100 && max_sp <= 199)
            value -= (((cur_wgt - max_wgt) / 10) * 1600L);
        if (max_sp >= 1 && max_sp <= 99)
            value -= (((cur_wgt - max_wgt) / 10) * 3200L);
    }


    /*** Penalize bad magic ***/

    /*  Hack -- most edged weapons hurt magic for priests */
    if (player_has(player, PF_BLESS_WEAPON))
    {
        item = &borg_items[INVEN_WIELD];

        /* Penalize non-blessed edged weapons */
        if ((item->tval == TV_SWORD || item->tval == TV_POLEARM) &&
            !of_has(item->flags, OF_BLESSED))
        {
            /* Hack -- Major penalty */
            value -= 75000L;
        }
    }


#if 0 /* I wonder if this causes the borg to change his gear so radically at depth 99 */
    /* HUGE MEGA MONDO HACK! prepare for the big fight */
    /* go after Morgoth new priorities. */
    if ((borg_skill[BI_MAXDEPTH] + 1 == 100 || borg_skill[BI_CDEPTH] == 100) && (!borg_skill[BI_KING]))
    {
        /* protect from stat drain */
        if (borg_skill[BI_SSTR]) value += 35000L;
        /* extra bonus for spell casters */
        if (player->class->spell_book == TV_MAGIC_BOOK && borg_skill[BI_SINT]) value += 45000L;
        /* extra bonus for spell casters */
        if (player->class->spell_book == TV_PRAYER_BOOK && borg_skill[BI_SWIS]) value += 35000L;
        if (borg_skill[BI_SCON]) value += 55000L;
        if (borg_skill[BI_SDEX]) value += 15000L;
        if (borg_skill[BI_WS_EVIL])  value += 15000L;

        /* Another bonus for resist nether, poison and base four */
        if (borg_skill[BI_RNTHR]) value += 15000L;
        if (borg_skill[BI_RDIS]) value += 15000L;

        /* to protect against summoned baddies */
        if (borg_skill[BI_RPOIS]) value += 100000L;
        if (borg_skill[BI_RFIRE] &&
            borg_skill[BI_RACID] &&
            borg_skill[BI_RELEC] &&
            borg_skill[BI_RCOLD]) value += 100000L;
    }
#endif

    /* Reward for activatable items in inventory */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
    {
        int multibonus = 0;

        item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* a small bonus for wearing things that are not IDd*/
        /* unless you know the ID spell*/
        if (!item->ident && !borg_spell_legal(IDENTIFY_RUNE))
            value += 10000L;

        /* Good to have one item with multiple high resists */
        multibonus = ((item->el_info[ELEM_POIS].res_level > 0) +
            (item->el_info[ELEM_SOUND].res_level > 0) +
            (item->el_info[ELEM_SHARD].res_level > 0) +
            (item->el_info[ELEM_NEXUS].res_level > 0) +
            (item->el_info[ELEM_NETHER].res_level > 0) +
            (item->el_info[ELEM_CHAOS].res_level > 0) +
            (item->el_info[ELEM_DISEN].res_level > 0) +
            ((item->el_info[ELEM_FIRE].res_level > 0) &&
                (item->el_info[ELEM_COLD].res_level > 0) &&
                (item->el_info[ELEM_ELEC].res_level > 0) &&
                (item->el_info[ELEM_ACID].res_level > 0)) +
            (of_has(item->flags, OF_SUST_STR) &&
                of_has(item->flags, OF_SUST_INT) &&
                of_has(item->flags, OF_SUST_WIS) &&
                of_has(item->flags, OF_SUST_DEX) &&
                of_has(item->flags, OF_SUST_CON)));

        if (multibonus >= 2) value += 3000 * multibonus;

        int activation = item->activ_idx;

            /* an extra bonus for activations */
        if (activation)
        {
            if (act_illumination == activation)
                value += 500;
            else if (act_mapping == activation)
                value += 550;
            else if (act_clairvoyance == activation)
                value += 600;
            else if (act_fire_bolt == activation)
                value += (500 + (9 * (8 + 1) / 2));
            else if (act_cold_bolt == activation)
                value += (500 + (6 * (8 + 1) / 2));
            else if (act_elec_bolt == activation)
                value += (500 + (4 * (8 + 1) / 2));
            else if (act_acid_bolt == activation)
                value += (500 + (5 * (8 + 1) / 2));
            else if (act_mana_bolt == activation)
                value += (500 + (12 * (8 + 1) / 2));
            else if (act_stinking_cloud == activation)
                value += (500 + (24));
            else if (act_cold_ball50 == activation)
                value += (500 + (96));
            else if (act_cold_ball100 == activation)
                value += (500 + (200));
            else if (act_fire_bolt72 == activation)
                value += (500 + (72));
            else if (act_cold_bolt2 == activation)
                value += (500 + (12 * (8 + 1) / 2));
            else if (act_fire_ball == activation)
                value += (500 + (144));
            else if (act_dispel_evil == activation)
                value += (500 + (10 + (borg_skill[BI_CLEVEL] * 5) / 2));
            else if (act_confuse2 == activation)
                value += 0; /* no code to handle this activation */
            else if (act_haste == activation)
                value += 0;  /* handled by adding to speed available */
            else if (act_haste1 == activation)
                value += 0; /* handled by adding to speed available */
            else if (act_haste2 == activation)
                value += 0; /* handled by adding to speed available */
            else if (act_detect_objects ==activation)
                value += 10;
            else if (act_probing == activation)
                value += 0; /* no code to handle this activation */
            else if (act_stone_to_mud == activation)
                value += 0; /* handled by adding to digger available */
            else if (act_tele_other == activation)
            {
                if (borg_class == CLASS_MAGE)
                    value += 500;
                else
                    value += (500 + (500));
            }
            else if (act_drain_life1 == activation)
                value += (500 + 90);
            else if (act_drain_life2 == activation)
                value += (500 + 120);
            else if (act_berserker == activation)
                value += (500);
            else if (act_cure_light == activation)
                value += 0; /* handled by adding to healing available */
            else if (act_cure_serious == activation)
                value += 0; /* handled by adding to healing available */
            else if (act_cure_critical == activation)
                value += 0; /* handled by adding to healing available */
            else if (act_cure_full2 == activation)
                value += 0; /* handled by adding to healing available */
            else if (act_loskill == activation)
                value += (500 + 200);
            else if (act_recall == activation)
                value += 0; /* handled by adding to recall available */
            else if (act_arrow == activation)
                value += (500 + (150));
            else if (act_rem_fear_pois == activation)
            {
                if (borg_class == CLASS_MAGE || borg_class == CLASS_PRIEST || borg_class == CLASS_DRUID)
                    value += 500;
                else
                    value += (500 + (200));
            }
            else if (act_tele_phase == activation)
                value += 500;
            else if (act_detect_all == activation)
                value += 0; /* handled by adding to detects available */
            else if (act_cure_full == activation)
                value += 0; /* handled by adding to healing available */
            else if (act_heal1 == activation)
                value += 0; /* handled by adding to healing available */
            else if (act_heal2 == activation)
                value += 0; /* handled by adding to healing available */
            else if (act_heal3 == activation)
                value += 0;  /* handled by adding to healing available */
            else if (act_cure_nonorlybig == activation)
                value += 0;  /* handled by adding to healing available */
            else if (act_protevil == activation)
                value += 0; /* handled by adding to PFE available */
            else if (act_destroy_doors == activation)
                value += 0; /* no code to handle this activation */
            else if (act_banishment == activation)
                value += 1000;
            else if (act_resist_all == activation)
            {
                value += (500 + (150));
                /* extra bonus if you can't cast RESISTANCE */
                if (borg_class != CLASS_MAGE) value += 25000;
            }
            else if (act_sleepii == activation)
            {
                value += 500;
                /* extra bonus if you can't cast a sleep type spell */
                if ((borg_class != CLASS_DRUID) && (borg_class != CLASS_NECROMANCER))
                    value += 200;
            }
            else if (act_recharge == activation)
            {
                value += 500;
                /* extra bonus if you can't cast a charge type spell */
                if ((borg_class != CLASS_MAGE) && (borg_class != CLASS_ROGUE))
                    value += 100;
            }
            else if (act_tele_long == activation)
                value += 300;
            else if (act_missile == activation)
                value += (500 + (2 * (6 + 1) / 2));
            else if (act_cure_temp == activation)
                value += 500;
            else if (act_starlight2 == activation)
                value += 100 + (10 * (8 + 1)) / 2;
            else if (act_bizarre == activation)
                value += (999999); /* HACK this is the one ring */
            else if (act_star_ball == activation)
                value += (500 + (300));
            else if (act_rage_bless_resist == activation)
            {
                value += (500 + (150));
                /* extra bonus if you can't cast RESISTANCE */
                if (borg_class != CLASS_MAGE) value += 25000;
            }
            else if (act_polymorph == activation)
            {
                value += 0;  /* no value, borg doesn't use polymorph */
            }
            else if (act_starlight == activation)
                value += 100 + (6 * (8 + 1)) / 2;
            else if (act_light == activation)
                value += 0;  /* handled by adding to ALITE */
            else if (act_firebrand == activation)
                value += 500;
            else if (act_restore_life == activation)
                value += 0;  /* handled by adding to the rll available */
            else if (act_restore_exp == activation)
                value += 0;  /* handled by adding to the rll available */
            else if (act_restore_st_lev == activation)
                value += 0;  /* handled by adding to the rll available */
            else if (act_enlightenment == activation)
                value += 500;
            else if (act_hero == activation)
                value += 500;  
            else if (act_shero == activation)
                value += 500;
            else if (act_cure_paranoia == activation)
                value += 100;
            else if (act_cure_mind == activation)
                value += 1000;
            else if (act_cure_body == activation)
                value += 1000;
            else if (act_mon_slow == activation)
                value += 500; //!FIX recalc  
            else if (act_mon_confuse == activation)
                value += 500; //!FIX recalc  
            else if (act_sleep_all == activation)
                value += 500; //!FIX recalc  
            else if (act_mon_scare == activation)
                value += 500; //!FIX recalc 
            else if (act_light_line == activation)
                value += 50;  //!FIX recalc 
            else if (act_disable_traps == activation)
                value += 0;  // !FIX no code to handle
            else if (act_drain_life3 == activation)
                value += (500 + 150);
            else if (act_drain_life4 == activation)
                value += (500 + 250);
            else if (act_elec_ball == activation)
                value += 500 + 64;  
            else if (act_elec_ball2 == activation)
                value += 500 + 250;  
            else if (act_acid_bolt2 == activation)
                value += (500 + (10 * (8 + 1) / 2));
            else if (act_acid_bolt3 == activation)
                value += (500 + (12 * (8 + 1) / 2));
            else if (act_acid_ball == activation)
                value += 500 + 120; 
            else if (act_cold_ball160 == activation)
                value += 500 + 160; 
            else if (act_cold_ball2 == activation)
                value += 500 + 200;
            else if (act_fire_ball2 == activation)
                value += 500 + 120;
            else if (act_fire_ball200 == activation)
                value += 500 + 200;
            else if (act_fire_bolt2 == activation)
                value += (500 + (12 * (8 + 1) / 2));
            else if (act_fire_bolt3 == activation)
                value += (500 + (16 * (8 + 1) / 2));
            else if (act_dispel_evil60 == activation)
                value += 500 + 60;
            else if (act_dispel_undead == activation)
                value += 500 + 60;
            else if (act_dispel_all == activation)
                value += 500 + 60 * 2;
            else if (act_deep_descent == activation)
                value += 0;  // !FIX no code to handle
            else if (act_earthquakes == activation)
                value += 0;  // !FIX no code to handle
            else if (act_destruction2 == activation)
                value += 500;  
            else if (act_losslow == activation)
                value += 50;
            else if (act_lossleep == activation)
                value += 100;
            else if (act_losconf == activation)
                value += 100;
            else if (act_satisfy == activation)
                value += 0;  // !FIX no code to handle
            else if (act_blessing == activation)
                value += 50;
            else if (act_blessing2 == activation)
                value += 50;
            else if (act_blessing3 == activation)
                value += 50;
            else if (act_glyph == activation)
                value += 0;  /* handled by adding to skill */
            else if (act_tele_level == activation)
                value += 5000L;
            else if (act_confusing == activation)
                value += 0;  // !FIX no code to handle
            else if (act_enchant_tohit == activation)
                value += 0;  // !FIX no code to handle
            else if (act_enchant_todam == activation)
                value += 0;  // !FIX no code to handle
            else if (act_enchant_weapon == activation)
                value += 0;  // !FIX no code to handle
            else if (act_enchant_armor == activation)
                value += 0;  // !FIX no code to handle
            else if (act_enchant_armor2 == activation)
                value += 0;  // !FIX no code to handle
            else if (act_remove_curse == activation)
                value += 9000;  
            else if (act_remove_curse2 == activation)
                value += 10000;
            else if (act_detect_treasure == activation)
                value += 0;  // !FIX no code to handle
            else if (act_detect_invis == activation)
                value += 0;  // !FIX no code to handle
            else if (act_detect_evil == activation)
                value += 0;  // !FIX no code to handle
            else if (act_restore_mana == activation)
                value += 5000;
            else if (act_brawn == activation)
                value += 0;  // !FIX no code to handle
            else if (act_intellect == activation)
                value += 0;  // !FIX no code to handle
            else if (act_contemplation == activation)
                value += 0;  // !FIX no code to handle
            else if (act_toughness == activation)
                value += 0;  // !FIX no code to handle
            else if (act_nimbleness == activation)
                value += 0;  // !FIX no code to handle
            else if (act_restore_str == activation)
                value += 50;  
            else if (act_restore_int == activation)
                value += 50;  
            else if (act_restore_wis == activation)
                value += 50;  
            else if (act_restore_dex == activation)
                value += 50;  
            else if (act_restore_con == activation)
                value += 50;  
            else if (act_restore_all == activation)
                value += 150;
            else if (act_tmd_free_act == activation)
                value += 0;  // !FIX no code to handle
            else if (act_tmd_infra == activation)
                value += 0;  // !FIX no code to handle
            else if (act_tmd_sinvis == activation)
                value += 0;  // !FIX no code to handle
            else if (act_tmd_esp == activation)
                value += 0;  // !FIX no code to handle
            else if (act_resist_acid == activation)
                value += 100;
            else if (act_resist_elec == activation)
                value += 100;
            else if (act_resist_fire == activation)
                value += 100;  
            else if (act_resist_cold == activation)
                value += 100;
            else if (act_resist_pois == activation)
                value += 150;
            else if (act_cure_confusion == activation)
                value += 0;  // !FIX no code to handle
            else if (act_wonder == activation)
                value += 300;
            else if (act_wand_breath == activation)
                value += 0;  // !FIX no code to handle (currently no code for wands of drag breath)
            else if (act_staff_magi == activation)
                borg_skill[BI_ASTFMAGI] += 10;
            else if (act_staff_holy == activation)
                value += 1000;
            else if (act_drink_breath == activation)
                value += 0;  // !FIX no code to handle (nor for the potion)
            else if (act_food_waybread == activation)
                value += 50;  
            else if (act_shroom_emergency == activation)
                value += 0;  // !FIX no code to handle
            else if (act_shroom_terror == activation)
                value += 0;  // !FIX no code to handle
            else if (act_shroom_stone == activation)
                value += 0;  // !FIX no code to handle
            else if (act_shroom_debility == activation)
                value += 0;  // !FIX no code to handle
            else if (act_shroom_sprinting == activation)
                value += 0;  // !FIX no code to handle
            else if (act_shroom_purging == activation)
                value += 50;
            else if (act_ring_acid == activation)
                value += 10000;
            else if (act_ring_flames == activation)
                value += 25000;
            else if (act_ring_ice == activation)
                value += 15000;
            else if (act_ring_lightning == activation)
                value += 10000;
            else if (act_dragon_blue == activation)
                value += 1100;
            else if (act_dragon_green == activation)
                value += 2750;
            else if (act_dragon_red == activation)
                value += 1100;
            else if (act_dragon_multihued == activation)
                value += 3250;
            else if (act_dragon_gold == activation)
                value += 5150;
            else if (act_dragon_chaos == activation)
                value += 5150;
            else if (act_dragon_law == activation)
                value += 5150;
            else if (act_dragon_balance == activation)
                value += 5150;
            else if (act_dragon_shining == activation)
                value += 5150;
            else if (act_dragon_power == activation)
                value += 5150;

        }
    }


    /* Result */
    return (value);
}



/*
 * Helper function -- calculate power of inventory
 */
static int32_t borg_power_aux2(void)
{
    int         k, carry_capacity, book;

    int32_t        value = 0L;


    /*** Basic abilities ***/

     /* Reward fuel */
    k = 0;
    for (; k < 6 && k < borg_skill[BI_AFUEL]; k++) value += 60000L;
    if (borg_skill[BI_STR] >= 15)
    {
        for (; k < 10 && k < borg_skill[BI_AFUEL]; k++) value += 6000L - (k * 100);
    }

    /* Reward Food */
    /* if hungry, food is THE top priority */
    if ((borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK]) && borg_skill[BI_FOOD]) value += 100000;
    k = 0;
    for (; k < 7 && k < borg_skill[BI_FOOD]; k++) value += 50000L;
    if (borg_skill[BI_STR] >= 15)
    {
        for (; k < 10 && k < borg_skill[BI_FOOD]; k++) value += 200L;
    }
    if (borg_skill[BI_REG] && borg_skill[BI_CLEVEL] <= 15)
    {
        k = 0;
        for (; k < 15 && k < borg_skill[BI_FOOD]; k++) value += 700L;
    }
    /* Prefere to buy HiCalorie foods over LowCalorie */
    if (amt_food_hical <= 5) value += amt_food_hical * 50;


    /* Reward Cure Poison and Cuts*/
    if ((borg_skill[BI_ISCUT] || borg_skill[BI_ISPOISONED]) && borg_skill[BI_ACCW]) value += 100000;
    if ((borg_skill[BI_ISCUT] || borg_skill[BI_ISPOISONED]) && borg_skill[BI_AHEAL]) value += 50000;
    if ((borg_skill[BI_ISCUT] || borg_skill[BI_ISPOISONED]) && borg_skill[BI_ACSW])
    {   /* usually takes more than one */
        k = 0;
        for (; k < 5 && k < borg_skill[BI_ACSW]; k++) value += 25000L;
    }
    if (borg_skill[BI_ISPOISONED] && borg_skill[BI_ACUREPOIS]) value += 15000;
    if (borg_skill[BI_ISPOISONED] && amt_slow_poison) value += 5000;

    /* collect Resistance pots if not immune -- All Classes */
    if (!borg_skill[BI_IPOIS] && borg_skill[BI_ACUREPOIS] <= 20)
    {
        /* Not if munchkin starting */
        if (borg_cfg[BORG_MUNCHKIN_START] && 
            borg_skill[BI_MAXCLEVEL] < borg_cfg[BORG_MUNCHKIN_LEVEL])
        {
            /* Do not carry these until later */
        }
        else
        {
            for (; k < 4 && k < borg_skill[BI_ARESPOIS]; k++) value += 300L;
        }
    }

    /* Reward Resistance Potions for Warriors */
    if (borg_class == CLASS_WARRIOR && borg_skill[BI_MAXDEPTH] > 20)
    {
        k = 0;
        /* Not if munchkin starting */
        if (borg_cfg[BORG_MUNCHKIN_START] && 
            borg_skill[BI_MAXCLEVEL] < borg_cfg[BORG_MUNCHKIN_LEVEL])
        {
            /* Do not carry these until later */
        }
        else
        {
            /* collect pots if not immune */
            if (!borg_skill[BI_IFIRE])
            {
                for (; k < 4 && k < borg_skill[BI_ARESHEAT]; k++) value += 500L;
            }
            k = 0;
            /* collect pots if not immune */
            if (!borg_skill[BI_ICOLD])
            {
                for (; k < 4 && k < borg_skill[BI_ARESCOLD]; k++) value += 500L;
            }
            /* collect pots if not immune */
            if (!borg_skill[BI_IPOIS])
            {
                for (; k < 4 && k < borg_skill[BI_ARESPOIS]; k++) value += 500L;
            }
        }
    }

    /* Reward ident */
    k = 0;
    if (borg_skill[BI_CLEVEL] >= 10)
    {
        for (; k < 5 && k < borg_skill[BI_AID]; k++) value += 6000L;
        if (borg_skill[BI_STR] >= 15)
        {
            for (; k < 15 && k < borg_skill[BI_AID]; k++) value += 600L;
        }
    }
    /* Reward ID if I am carrying a {magical} or {excellent} item */
    if (my_need_id)
    {
        k = 0;
        for (; k < my_need_id && k < borg_skill[BI_AID]; k++) value += 6000L;
    }

    /*  Reward PFE  carry lots of these*/
    k = 0;
    /* Not if munchkin starting */
    if (borg_cfg[BORG_MUNCHKIN_START] && 
        borg_skill[BI_MAXCLEVEL] < borg_cfg[BORG_MUNCHKIN_LEVEL])
    {
        /* Do not carry these until later */
    }
    else
    {
        for (; k < 10 && k < borg_skill[BI_APFE]; k++) value += 10000L;
        for (; k < 25 && k < borg_skill[BI_APFE]; k++) value += 2000L;
    }
    /*  Reward Glyph- Rune of Protection-  carry lots of these*/
    k = 0;
    for (; k < 10 && k < borg_skill[BI_AGLYPH]; k++) value += 10000L;
    for (; k < 25 && k < borg_skill[BI_AGLYPH]; k++) value += 2000L;
    if (borg_skill[BI_MAXDEPTH] >= 100)
    {
        k = 0;
        for (; k < 10 && k < borg_skill[BI_AGLYPH]; k++) value += 2500L;
        for (; k < 75 && k < borg_skill[BI_AGLYPH]; k++) value += 2500L;
    }

    /* Reward Scroll of Mass Genocide, only when fighting Morgoth */
    if (borg_skill[BI_MAXDEPTH] >= 100)
    {
        k = 0;
        for (; k < 99 && k < borg_skill[BI_AMASSBAN]; k++) value += 2500L;
    }

    /* Reward recall */
    k = 0;
    if (borg_skill[BI_CLEVEL] > 7)
    {
        /* Not if munchkin starting */
        if (borg_cfg[BORG_MUNCHKIN_START] && 
            borg_skill[BI_MAXCLEVEL] < borg_cfg[BORG_MUNCHKIN_LEVEL])
        {
            /* Do not carry these until later */
        }
        else
        {
            for (; k < 3 && k < borg_skill[BI_RECALL]; k++) value += 50000L;
            if (borg_skill[BI_STR] >= 15)
            {
                for (; k < 7 && k < borg_skill[BI_RECALL]; k++) value += 5000L;
            }
            /* Deep borgs need the rod to avoid low mana traps */
            if (borg_skill[BI_MAXDEPTH] >= 50 && borg_has[kv_rod_recall]) value += 12000;
        }
    }

    /* Reward phase */
    k = 1;
    /* first phase door is very important */
    if (borg_skill[BI_APHASE]) value += 50000;
    /* Not if munchkin starting */
    if (borg_cfg[BORG_MUNCHKIN_START] && 
        borg_skill[BI_MAXCLEVEL] < borg_cfg[BORG_MUNCHKIN_LEVEL])
    {
        /* Do not carry these until later */
    }
    else
    {
        for (; k < 8 && k < borg_skill[BI_APHASE]; k++) value += 500L;
        if (borg_skill[BI_STR] >= 15)
        {
            for (; k < 15 && k < borg_skill[BI_APHASE]; k++) value += 500L;
        }
    }

    /* Reward escape (staff of teleport or artifact */
    k = 0;
    if (borg_cfg[BORG_MUNCHKIN_START] && 
        borg_skill[BI_MAXCLEVEL] < borg_cfg[BORG_MUNCHKIN_LEVEL])
    {
        /* Do not carry these until later */
    }
    else
    {
        for (; k < 2 && k < borg_skill[BI_AESCAPE]; k++) value += 10000L;
        if (borg_skill[BI_CDEPTH] > 70)
        {
            k = 0;
            for (; k < 3 && k < borg_skill[BI_AESCAPE]; k++) value += 10000L;
        }
    }

    /* Reward teleport scroll*/
    k = 0;
    if (borg_skill[BI_CLEVEL] >= 3)
    {
        if (borg_skill[BI_ATELEPORT]) value += 10000L;
    }
    if (borg_skill[BI_CLEVEL] >= 7)
    {
        for (; k < 3 && k < borg_skill[BI_ATELEPORT]; k++) value += 10000L;
    }
    if (borg_skill[BI_CLEVEL] >= 30)
    {
        for (; k < 10 && k < borg_skill[BI_ATELEPORT]; k++) value += 10000L;
    }

    /* Reward Teleport Level scrolls */
    k = 0;
    if (borg_skill[BI_CLEVEL] >= 15)
    {
        for (; k < 5 && k < borg_skill[BI_ATELEPORTLVL]; k++) value += 5000L;
    }


    /*** Healing ***/
    /* !TODO !FIX !AJG make sure these numbers make sense for the new classes */
    if (borg_class == CLASS_WARRIOR ||
        borg_class == CLASS_ROGUE ||
        borg_class == CLASS_BLACKGUARD)
    {
        k = 0;
        for (; k < 15 && k < borg_skill[BI_AHEAL]; k++) value += 8000L;

        k = 0; /* carry a couple for emergency. Store the rest. */
        if (borg_skill[BI_MAXDEPTH] >= 46)
        {
            if (borg_scumming_pots)
            {
                for (; k < 1 && k < borg_skill[BI_AEZHEAL]; k++) value += 10000L;
            }
            else
            {
                for (; k < 2 && k < borg_skill[BI_AEZHEAL]; k++) value += 10000L;
            }
        }

        /* These guys need to carry the rods more, town runs low on supply. */
        k = 0;
        for (; k < 6 && k < borg_has[kv_rod_healing]; k++) value += 20000L;
    }
    else if (borg_class == CLASS_RANGER ||
        borg_class == CLASS_PALADIN ||
        borg_class == CLASS_NECROMANCER ||
        borg_class == CLASS_MAGE)
    {
        k = 0;
        for (; k < 10 && k < borg_skill[BI_AHEAL]; k++) value += 4000L;

        k = 0; /* carry a couple for emergency. Store the rest. */
        if (borg_skill[BI_MAXDEPTH] >= 46)
        {
            if (borg_scumming_pots)
            {
                for (; k < 1 && k < borg_skill[BI_AEZHEAL]; k++) value += 10000L;
            }
            else
            {
                for (; k < 2 && k < borg_skill[BI_AEZHEAL]; k++) value += 10000L;
            }
        }

        if (borg_class == CLASS_PALADIN)
        {
            /* Reward heal potions */
            k = 0;
            for (; k < 3 && k < borg_has[kv_potion_healing]; k++) value += 5000L;
        }

        /* These guys need to carry the rods more, town runs low on supply. */
        k = 0;
        for (; k < 4 && k < borg_has[kv_rod_healing]; k++) value += 20000L;

    }
    else if (borg_class == CLASS_PRIEST || borg_class == CLASS_DRUID)
    {
        /* Level 1 priests are given a Potion of Healing.  It is better
         * for them to sell that potion and buy equipment or several
         * Cure Crits with it.
         */
        if (borg_skill[BI_CLEVEL] == 1)
        {
            k = 0;
            for (; k < 10 && k < borg_has[kv_potion_healing]; k++) value -= 2000L;
        }
        /* Reward heal potions */
        k = 0;
        for (; k < 5 && k < borg_has[kv_potion_healing]; k++) value += 2000L;

        k = 0; /* carry a couple for emergency. Store the rest. */
        if (borg_skill[BI_MAXDEPTH] >= 46)
        {
            if (borg_scumming_pots)
            {
                for (; k < 1 && k < borg_skill[BI_AEZHEAL]; k++) value += 10000L;
            }
            else
            {
                for (; k < 2 && k < borg_skill[BI_AEZHEAL]; k++) value += 10000L;
            }
        }
    }


    /* Collecting Potions, prepping for Morgoth/Sauron fight */
    if (borg_skill[BI_MAXDEPTH] >= 99)
    {
        /* Sauron is alive -- carry them all*/
        if (borg_race_death[546] == 0)
        {
            k = 0;
            for (; k < 99 && k < borg_has[kv_potion_healing]; k++) value += 8000L;
            k = 0;
            for (; k < 99 && k < borg_skill[BI_AEZHEAL]; k++) value += 10000L;
            k = 0;
            for (; k < 99 && k < borg_skill[BI_ASPEED]; k++) value += 8000L;
            k = 0;
            for (; k < 99 && k < borg_skill[BI_ALIFE]; k++) value += 10000L;
            k = 0;
            if (borg_class != CLASS_WARRIOR)
            {
                for (; k < 99 && k < borg_has[kv_potion_restore_mana]; k++) value += 5000L;
            }
            k = 0;
            for (; k < 40 && k < borg_has[kv_mush_stoneskin]; k++) value += 5000L;

            /* No need to store extras in home */
            borg_scumming_pots = false;

        }
        /* Sauron is dead -- store them unless I have enough */
        if (borg_race_death[546] != 0)
        {
            /* Must scum for more pots */
            if ((num_heal_true + borg_has[kv_potion_healing] + num_ezheal_true + borg_skill[BI_AEZHEAL] < 30) ||
                (num_ezheal_true + borg_skill[BI_AEZHEAL] < 20) ||
                (num_speed + borg_skill[BI_ASPEED] < 15))
            {
                /* leave pots at home so they dont shatter */
                borg_scumming_pots = true;
            }
            /* I have enough, carry all pots, and other good stuff. */
            else
            {
                k = 0;
                for (; k < 99 && k < borg_has[kv_potion_healing]; k++) value += 8000L;
                k = 0;
                for (; k < 99 && k < borg_skill[BI_AEZHEAL]; k++) value += 10000L;
                k = 0;
                for (; k < 99 && k < borg_skill[BI_ALIFE]; k++) value += 10000L;
                k = 0;
                for (; k < 99 && k < borg_skill[BI_ASPEED]; k++) value += 8000L;
                k = 0;
                for (; k < 40 && k < borg_has[kv_mush_stoneskin]; k++) value += 5000L;
                k = 0;
                if (borg_class != CLASS_WARRIOR)
                {
                    for (; k < 99 && k < borg_has[kv_potion_restore_mana]; k++) value += 5000L;
                }
                /* Reward Scroll of Mass Genocide, only when fighting Morgoth */
                k = 0;
                for (; k < 99 && k < borg_skill[BI_AMASSBAN]; k++) value += 2500L;

                /* No need to store extras in home */
                borg_scumming_pots = false;
            }
        }
    }

    /* Restore Mana */
    if (borg_skill[BI_MAXSP] > 100)
    {
        k = 0;
        for (; k < 10 && k < borg_has[kv_potion_restore_mana]; k++) value += 4000L;
        k = 0;
        for (; k < 100 && k < borg_skill[BI_ASTFMAGI]; k++) value += 4000L;
    }

    /* Reward Cure Critical.  Heavy reward on first 5 */
    if (borg_skill[BI_CLEVEL] < 35 && borg_skill[BI_CLEVEL] > 10)
    {
        k = 0;
        for (; k < 10 && k < borg_skill[BI_ACCW]; k++) value += 5000L;
    }
    else if (borg_skill[BI_CLEVEL] > 35)
    {
        /* Reward cure critical.  Later on in game. */
        k = 0;
        for (; k < 10 && k < borg_skill[BI_ACCW]; k++) value += 5000L;
        if (borg_skill[BI_STR] > 15)
        {
            for (; k < 15 && k < borg_skill[BI_ACCW]; k++) value += 500L;
        }
    }

    /* Reward cure serious -- only reward serious if low on crits */
    if (borg_skill[BI_ACCW] < 5 && borg_skill[BI_MAXCLEVEL] > 10 && (borg_skill[BI_CLEVEL] < 35 || !borg_skill[BI_RCONF]))
    {
        k = 0;
        for (; k < 7 && k < borg_skill[BI_ACSW]; k++) value += 50L;
        if (borg_skill[BI_STR] > 15)
        {
            for (; k < 10 && k < borg_skill[BI_ACSW]; k++) value += 5L;
        }
    }

    /* Reward cure light -- Low Level Characters */
    if ((borg_skill[BI_ACCW] + borg_skill[BI_ACSW] < 5) && borg_skill[BI_CLEVEL] < 8)
    {
        k = 0;
        for (; k < 5 && k < borg_skill[BI_ACLW]; k++) value += 550L;
    }

    /* Reward Cures */
    if (!borg_skill[BI_RCONF])
    {
        if (borg_cfg[BORG_MUNCHKIN_START] && borg_skill[BI_MAXCLEVEL] < 10)
        {
            /* Do not carry these until later */
        }
        else
        {
            k = 0;
            for (; k < 10 && k < amt_cure_confusion; k++) value += 400L;
        }
    }
    if (!borg_skill[BI_RBLIND])
    {
        k = 0;
        if (borg_cfg[BORG_MUNCHKIN_START] && 
            borg_skill[BI_MAXCLEVEL] < borg_cfg[BORG_MUNCHKIN_LEVEL])
        {
            /* Do not carry these until later */
        }
        else
        {
            for (; k < 5 && k < amt_cure_blind; k++) value += 300L;
        }
    }
    if (!borg_skill[BI_RPOIS])
    {
        k = 0;
        if (borg_cfg[BORG_MUNCHKIN_START] && 
            borg_skill[BI_MAXCLEVEL] < borg_cfg[BORG_MUNCHKIN_LEVEL])
        {
            /* Do not carry these until later */
        }
        else
        {
            for (; k < 5 && k < borg_skill[BI_ACUREPOIS]; k++) value += 250L;
        }
    }
    /*** Detection ***/

    /* Reward detect trap */
    k = 0;
    for (; k < 1 && k < borg_skill[BI_ADETTRAP]; k++) value += 4000L;

    /* Reward detect door */
    k = 0;
    for (; k < 1 && k < borg_skill[BI_ADETDOOR]; k++) value += 2000L;

    /* Reward detect evil for non spell caster guys */
    if (!borg_skill[BI_ESP] && !borg_spell_legal(DETECT_EVIL))
    {
        k = 0;
        for (; k < 1 && k < borg_skill[BI_ADETEVIL]; k++) value += 1000L;
    }

    /* Reward magic mapping */
    k = 0;
    for (; k < 1 && k < borg_skill[BI_AMAGICMAP]; k++) value += 4000L;

    /* Reward call lite */
    k = 0;
    for (; k < 1 && k < borg_skill[BI_ALITE]; k++) value += 1000L;

    /* Genocide scrolls. Just scrolls, mainly used for Morgoth */
    if (borg_skill[BI_MAXDEPTH] >= 100)
    {
        k = 0;
        for (; k < 10 && k < borg_has[kv_scroll_mass_banishment]; k++) value += 10000L;
        for (; k < 25 && k < borg_has[kv_scroll_mass_banishment]; k++) value += 2000L;
    }

    /* Reward speed potions/rods/staves (but no staves deeper than depth 95) */
    k = 0;
    if (borg_cfg[BORG_MUNCHKIN_START] && 
        borg_skill[BI_MAXCLEVEL] < borg_cfg[BORG_MUNCHKIN_LEVEL])
    {
        /* Do not carry these until later */
    }
    else
    {
        for (; k < 20 && k < borg_skill[BI_ASPEED]; k++) value += 5000L;
    }

    /* Reward Recharge ability */
    if (borg_skill[BI_ARECHARGE] && borg_skill[BI_MAXDEPTH] < 99) value += 5000L;

    /*** Missiles ***/

    /* Reward missiles */
    if (borg_class == CLASS_RANGER || borg_class == CLASS_WARRIOR)
    {
        k = 0;
        for (; k < 40 && k < borg_skill[BI_AMISSILES]; k++) value += 1000L;
        if (borg_skill[BI_STR] > 15 && borg_skill[BI_STR] < 18)
        {
            for (; k < 80 && k < borg_skill[BI_AMISSILES]; k++) value += 100L;
        }
        if (borg_skill[BI_STR] > 18)
        {
            for (; k < 180 && k < borg_skill[BI_AMISSILES]; k++) value += 80L;
        }

        /* peanalize use of too many quiver slots */
        for (k = QUIVER_START + 4; k < QUIVER_END; k++)
        {
            if (borg_items[k].iqty) value -= 10000L;
        }

    }
    else
    {
        k = 0;
        for (; k < 20 && k < borg_skill[BI_AMISSILES] && k < 99; k++) value += 1000L;
        if (borg_skill[BI_STR] > 15)
        {
            for (; k < 50 && k < borg_skill[BI_AMISSILES] && k < 99; k++) value += 100L;
        }
        /* Don't carry too many */
        if (borg_skill[BI_STR] <= 15 && borg_skill[BI_AMISSILES] > 20) value -= 1000L;
        /* peanalize use of too many quiver slots */
        for (k = QUIVER_START + 2; k < QUIVER_END; k++)
        {
            if (borg_items[k].iqty) value -= 10000L;
        }
    }

    /* cursed arrows are "bad" */
    value -= 1000L * borg_skill[BI_AMISSILES_CURSED];

    /* ego arrows are worth a bonus */
    value += 100L * borg_skill[BI_AMISSILES_SPECIAL];

    /*** Various ***/

    /*  -- Reward carrying a staff of destruction. */
    if (borg_skill[BI_ASTFDEST]) value += 5000L;
    k = 0;
    for (; k < 9 && k < borg_skill[BI_ASTFDEST]; k++) value += 200L;

    /*  -- Reward carrying a wand of Teleport Other. */
    if (borg_skill[BI_ATPORTOTHER]) value += 5000L;
    /* Much greater reward for Warrior */
    if (borg_class == CLASS_WARRIOR && borg_skill[BI_ATPORTOTHER]) value += 50000L;
    /* reward per charge */
    k = 0;
    for (; k < 15 && k < borg_skill[BI_ATPORTOTHER]; k++) value += 5000L;

    /*  -- Reward carrying an attack wand.
     */
    if ((borg_has[kv_wand_magic_missile] || borg_has[kv_wand_stinking_cloud]) && borg_skill[BI_MAXDEPTH] < 30) value += 5000L;
    if (borg_has[kv_wand_annihilation] && borg_skill[BI_CDEPTH] < 30) value += 5000L;
    /* Much greater reward for Warrior or lower level  */
    if ((borg_class == CLASS_WARRIOR || borg_skill[BI_CLEVEL] <= 20) &&
        (borg_has[kv_wand_magic_missile] || borg_has[kv_wand_annihilation] || borg_has[kv_wand_stinking_cloud])) value += 10000L;
    /* reward per charge */
    value += amt_cool_wand * 50L;

    /* These staves are great but do not clutter inven with them */
    /*  -- Reward carrying a staff of holiness/power */
    if (amt_cool_staff) value += 2500L;
    k = 0;
    for (; k < 3 && k < amt_cool_staff; k++) value += 500L;

    /* Rods of attacking are good too */
    k = 0;
    for (; k < 6 && k < borg_skill[BI_AROD1]; k++) value += 8000;
    k = 0;
    for (; k < 6 && k < borg_skill[BI_AROD2]; k++) value += 12000;

    /* Hack -- Reward add stat */
    if (amt_add_stat[STAT_STR]) value += 50000;
    if (amt_add_stat[STAT_INT]) value += 20000;

    int spell_stat = borg_spell_stat();
    if (spell_stat >= 0)
        if (amt_add_stat[spell_stat]) value += 50000;

    if (amt_add_stat[STAT_WIS]) value += 20000;
    if (amt_add_stat[STAT_DEX]) value += 50000;
    if (amt_add_stat[STAT_CON]) value += 50000;

    /* Hack -- Reward stat potions */
    if (amt_inc_stat[STAT_STR] && my_stat_cur[STAT_STR] < (18 + 100)) value += 550000;
    if (amt_inc_stat[STAT_INT] && my_stat_cur[STAT_INT] < (18 + 100)) value += 520000;
    if (spell_stat >= 0)
        if (amt_inc_stat[spell_stat] && my_stat_cur[spell_stat] < (18 + 100)) value += 575000;
    if (amt_inc_stat[STAT_WIS] && my_stat_cur[STAT_WIS] < (18 + 100)) value += 520000;
    if (amt_inc_stat[STAT_DEX] && my_stat_cur[STAT_DEX] < (18 + 100)) value += 550000;
    if (amt_inc_stat[STAT_CON] && my_stat_cur[STAT_CON] < (18 + 100)) value += 550000;

    /* Reward Remove Curse */
    if (borg_skill[BI_FIRST_CURSED])
    {
        if (borg_has[kv_scroll_star_remove_curse]) value += 90000;
        if (borg_has[kv_scroll_remove_curse]) value += 90000;
    }

    /* Hack -- Restore experience */
    if (amt_fix_exp) value += 50000;

    /*** Enchantment ***/

    /* Reward enchant armor */
    if (amt_enchant_to_a < 1000 && my_need_enchant_to_a) value += 540L;

    /* Reward enchant weapon to hit */
    if (amt_enchant_to_h < 1000 && my_need_enchant_to_h) value += 540L;

    /* Reward enchant weapon to damage */
    if (amt_enchant_to_d < 1000 && my_need_enchant_to_d) value += 500L;

    /* Reward *enchant weapon* to damage */
    if (amt_enchant_weapon) value += 5000L;

    /* Reward *enchant armour*  */
    if (amt_enchant_armor) value += 5000L;

    /* Reward carrying a shovel if low level */
    if (borg_skill[BI_MAXDEPTH] <= 40 && borg_skill[BI_MAXDEPTH] >= 25 && borg_gold < 100000 && borg_items[INVEN_WIELD].tval != TV_DIGGING &&
        amt_digger == 1) value += 5000L;


    /*** Hack -- books ***/
    /*   Reward books    */
    for (book = 0; book < 9; book++)
    {
        /* No copies */
        if (!amt_book[book]) continue;

        /* The "hard" books */
        if (player->class->magic.books[book].dungeon)
        {
            int what;

            /* Scan the spells */
            for (what = 0; what < 9; what++)
            {
                borg_magic* as = borg_get_spell_entry(book, what);
                if (!as) break;

                /* Track minimum level */
                if (as->level > borg_skill[BI_MAXCLEVEL]) continue;

                /* Track Mana req. */
                if (as->power > borg_skill[BI_MAXSP]) continue;

                /* Reward the book based on the spells I can cast */
                value += 15000L;
            }
        }

        /* The "easy" books */
        else
        {
            int what, when = 99;

            /* Scan the spells */
            for (what = 0; what < 9; what++)
            {
                borg_magic* as = borg_get_spell_entry(book, what);
                if (!as) break;

                /* Track minimum level */
                if (as->level < when) when = as->level;

                /* Track Mana req. */
                /* if (as->power < mana) mana = as->power; */
            }

            /* Hack -- Ignore "difficult" normal books */
            if ((when > 5) && (when >= borg_skill[BI_MAXCLEVEL] + 2)) continue;
            /* if (mana > borg_skill[BI_MAXSP]) continue; */

            /* Reward the book */
            k = 0;
            for (; k < 1 && k < amt_book[book]; k++) value += 500000L;
            if (borg_skill[BI_STR] > 5)
                for (; k < 2 && k < amt_book[book]; k++) value += 10000L;
        }
    }


    /*  Hack -- Apply "encumbrance" from weight */

    /* Extract the "weight limit" (in tenth pounds) */
    carry_capacity = borg_adj_str_wgt[my_stat_ind[STAT_STR]] * 100;

    /* XXX XXX XXX Apply "encumbrance" from weight */
    if (borg_skill[BI_WEIGHT] > carry_capacity / 2)
    {
        /* *HACK*  when testing items, the borg puts them in the last empty slot so this is */
        /* POSSIBLY just a test item */
        borg_item* item = NULL;
        for (int i = PACK_SLOTS; i >= 0; i--)
        {
            if (borg_items[i].iqty)
            {
                item = &borg_items[i];
                break;
            }
        }

        /* Some items will be used immediately and should not contribute to encumbrance */
        if (item && item->iqty &&
            ((item->tval == TV_SCROLL &&
                ((item->sval == sv_scroll_enchant_armor && amt_enchant_to_a < 1000 && my_need_enchant_to_a) ||
                    (item->sval == sv_scroll_enchant_weapon_to_hit && amt_enchant_to_h < 1000 && my_need_enchant_to_h) ||
                    (item->sval == sv_scroll_enchant_weapon_to_dam && amt_enchant_to_d < 1000 && my_need_enchant_to_d) ||
                    item->sval == sv_scroll_star_enchant_weapon ||
                    item->sval == sv_scroll_star_enchant_armor)) ||
                (item->tval == TV_POTION &&
                    (item->sval == sv_potion_inc_str ||
                        item->sval == sv_potion_inc_int ||
                        item->sval == sv_potion_inc_wis ||
                        item->sval == sv_potion_inc_dex ||
                        item->sval == sv_potion_inc_con ||
                        item->sval == sv_potion_inc_all))))
        {
            /* No encumbrance penalty for purchasing these items */
        }
        else
        {
            value -= ((borg_skill[BI_WEIGHT] - (carry_capacity / 2)) / (carry_capacity / 10) * 1000L);
        }
    }
    /* Reward empty slots (up to 5) */
    if (borg_skill[BI_EMPTY] < 6)
        value += 40L * borg_skill[BI_EMPTY];
    else
        value += 40L * 5;

    /* Return the value */
    return (value);
}


/*
 * Calculate the "power" of the Borg
 */
int32_t borg_power(void)
{
    int i = 1;
    int32_t value = 0L;

    /* Process the equipment */
    value += borg_power_aux1();

    /* Process the inventory */
    value += borg_power_aux2();

    /* Add a bonus for deep level prep */
    /* Dump prep codes */
   /* Scan from surface to deep , stop when not preped */
    for (i = 1; i <= borg_skill[BI_MAXDEPTH] + 50; i++)
    {
        /* Dump fear code*/
        if ((char*)NULL != borg_prepared(i)) break;
    }
    value += ((i - 1) * 40000L);

    /* Add the value for the swap items */
    value += weapon_swap_value;
    value += armour_swap_value;

    /* Return the value */
    return (value);
}

/*
 * Helper function -- calculate power of equipment in the home
 */
static int32_t borg_power_home_aux1(void)
{
    int32_t        value = 0L;

    /* This would be better seperated by item type (so 1 bonus for resist cold armor */
    /*   1 bonus for resist cold shield... but that would take a bunch more code. */

    /* try to collect at least 2 of each resist/power (for swapping) */
    /* This can be used to get rid of extra artifacts... */

    /* spare lite sources.  Artifacts only */
    if (num_LIGHT == 1)
        value += 150L;
    else
        if (num_LIGHT == 2)
            value += 170L;
        else
            if (num_LIGHT > 2)
                value += 170L + (num_LIGHT - 2) * 5L;

    if (num_slow_digest == 1)
        value += 50L;
    else
        if (num_slow_digest == 2)
            value += 70L;
        else
            if (num_slow_digest > 2)
                value += 70L + (num_slow_digest - 2) * 5L;

    if (num_regenerate == 1)
        value += 75L;
    else
        if (num_regenerate == 2)
            value += 100L;
        else
            if (num_regenerate > 2)
                value += 100L + (num_regenerate - 2) * 10L;

    if (num_telepathy == 1)
        value += 1000L;
    else
        if (num_telepathy == 2)
            value += 1500L;
        else
            if (num_telepathy > 2)
                value += 1500L + (num_telepathy - 2) * 10L;

    if (num_see_inv == 1)
        value += 800L;
    else
        if (num_see_inv == 2)
            value += 1200L;
        else
            if (num_see_inv > 2)
                value += 1200L + (num_see_inv - 2) * 10L;

    if (num_ffall == 1)
        value += 10L;
    else
        if (num_ffall == 2)
            value += 15L;
        else
            if (num_ffall > 2)
                value += 15L + (num_ffall - 2) * 1L;


    if (num_free_act == 1)
        value += 1000L;
    else
        if (num_free_act == 2)
            value += 1500L;
        else
            if (num_free_act > 2)
                value += 1500L + (num_free_act - 2) * 10L;

    if (num_hold_life == 1)
        value += 1000L;
    else
        if (num_hold_life == 2)
            value += 1500L;
        else
            if (num_hold_life > 2)
                value += 1500L + (num_hold_life - 2) * 10L;

    if (num_resist_acid == 1)
        value += 1000L;
    else
        if (num_resist_acid == 2)
            value += 1500L;
        else
            if (num_resist_acid > 2)
                value += 1500L + (num_resist_acid - 2) * 1L;
    if (num_immune_acid == 1)
        value += 3000L;
    else
        if (num_immune_acid == 2)
            value += 5000L;
        else
            if (num_immune_acid > 2)
                value += 5000L + (num_immune_acid - 2) * 30L;

    if (num_resist_elec == 1)
        value += 1000L;
    else
        if (num_resist_elec == 2)
            value += 1500L;
        else
            if (num_resist_elec > 2)
                value += 1500L + (num_resist_elec - 2) * 1L;
    if (num_immune_elec == 1)
        value += 3000L;
    else
        if (num_immune_elec == 2)
            value += 5000L;
        else
            if (num_immune_elec > 2)
                value += 5000L + (num_immune_elec - 2) * 30L;

    if (num_resist_fire == 1)
        value += 1000L;
    else
        if (num_resist_fire == 2)
            value += 1500L;
        else
            if (num_resist_fire > 2)
                value += 1500L + (num_resist_fire - 2) * 1L;
    if (num_immune_fire == 1)
        value += 3000L;
    else
        if (num_immune_fire == 2)
            value += 5000L;
        else
            if (num_immune_fire > 2)
                value += 5000L + (num_immune_fire - 2) * 30L;

    if (num_resist_cold == 1)
        value += 1000L;
    else
        if (num_resist_cold == 2)
            value += 1500L;
        else
            if (num_resist_cold > 2)
                value += 1500L + (num_resist_cold - 2) * 1L;
    if (num_immune_cold == 1)
        value += 3000L;
    else
        if (num_immune_cold == 2)
            value += 5000L;
        else
            if (num_immune_cold > 2)
                value += 5000L + (num_immune_cold - 2) * 30L;

    if (num_resist_pois == 1)
        value += 5000L;
    else
        if (num_resist_pois == 2)
            value += 9000L;
        else
            if (num_resist_pois > 2)
                value += 9000L + (num_resist_pois - 2) * 40L;

    if (num_resist_conf == 1)
        value += 2000L;
    else
        if (num_resist_conf == 2)
            value += 8000L;
        else
            if (num_resist_conf > 2)
                value += 8000L + (num_resist_conf - 2) * 45L;

    if (num_resist_sound == 1)
        value += 500L;
    else
        if (num_resist_sound == 2)
            value += 700L;
        else
            if (num_resist_sound > 2)
                value += 700L + (num_resist_sound - 2) * 30L;

    if (num_resist_LIGHT == 1)
        value += 100L;
    else
        if (num_resist_LIGHT == 2)
            value += 150L;
        else
            if (num_resist_LIGHT > 2)
                value += 150L + (num_resist_LIGHT - 2) * 1L;

    if (num_resist_dark == 1)
        value += 100L;
    else
        if (num_resist_dark == 2)
            value += 150L;
        else
            if (num_resist_dark > 2)
                value += 150L + (num_resist_dark - 2) * 1L;

    if (num_resist_chaos == 1)
        value += 1000L;
    else
        if (num_resist_chaos == 2)
            value += 1500L;
        else
            if (num_resist_chaos > 2)
                value += 1500L + (num_resist_chaos - 2) * 10L;

    if (num_resist_disen == 1)
        value += 5000L;
    else
        if (num_resist_disen == 2)
            value += 7000L;
        else
            if (num_resist_disen > 2)
                value += 7000L + (num_resist_disen - 2) * 35L;

    if (num_resist_shard == 1)
        value += 100L;
    else
        if (num_resist_shard == 2)
            value += 150L;
        else
            if (num_resist_shard > 2)
                value += 150L + (num_resist_shard - 2) * 1L;

    if (num_resist_nexus == 1)
        value += 200L;
    else
        if (num_resist_nexus == 2)
            value += 300L;
        else
            if (num_resist_nexus > 2)
                value += 300L + (num_resist_nexus - 2) * 2L;

    if (num_resist_blind == 1)
        value += 500L;
    else
        if (num_resist_blind == 2)
            value += 1000L;
        else
            if (num_resist_blind > 2)
                value += 1000L + (num_resist_blind - 2) * 5L;

    if (num_resist_neth == 1)
        value += 3000L;
    else
        if (num_resist_neth == 2)
            value += 4000L;
        else
            if (num_resist_neth > 2)
                value += 4000L + (num_resist_neth - 2) * 45L;

    /* stat gain items as well...(good to carry ring of dex +6 in */
    /*                            house even if I don't need it right now) */
    if (home_stat_add[STAT_STR] < 9)
        value += home_stat_add[STAT_STR] * 300L;
    else
        if (home_stat_add[STAT_STR] < 15)
            value += 9 * 300L + (home_stat_add[STAT_STR] - 9) * 200L;
        else
            value += 9 * 300L + 6 * 200L +
            (home_stat_add[STAT_STR] - 15) * 1L;

    if (home_stat_add[STAT_DEX] < 9)
        value += home_stat_add[STAT_DEX] * 300L;
    else
        if (home_stat_add[STAT_DEX] < 15)
            value += 9 * 300L + (home_stat_add[STAT_DEX] - 9) * 200L;
        else
            value += 9 * 300L + 6 * 200L +
            (home_stat_add[STAT_DEX] - 15) * 1L;

    /* HACK extra con for thorin and other such things */
    if (home_stat_add[STAT_CON] < 15)
        value += home_stat_add[STAT_CON] * 300L;
    else
        if (home_stat_add[STAT_CON] < 21)
            value += 15 * 300L + (home_stat_add[STAT_CON] - 15) * 200L;
        else
            value += 15 * 300L + 6 * 200L + (home_stat_add[STAT_CON] - 21) * 1L;

    /* spell stat is only bonused for spell casters. */
    int spell_stat = borg_spell_stat();
    if (spell_stat >= 0)
    {
        if (home_stat_add[spell_stat] < 20)
            value += home_stat_add[spell_stat] * 400L;
        else
            if (home_stat_add[spell_stat] < 26)
                value += 20 * 400L + (home_stat_add[spell_stat] - 20) * 300L;
            else
                value += 20 * 100L + 6 * 300L +
                (home_stat_add[spell_stat] - 26) * 5L;
    }

    /* Sustains */
    if (num_sustain_str == 1)
        value += 200L;
    else
        if (num_sustain_str == 2)
            value += 250L;
        else
            if (num_sustain_str > 2)
                value += 250L + (num_sustain_str - 2) * 1L;

    if (num_sustain_int == 1)
        value += 200L;
    else
        if (num_sustain_int == 2)
            value += 250L;
        else
            if (num_sustain_int > 2)
                value += 250L + (num_sustain_int - 2) * 1L;

    if (num_sustain_wis == 1)
        value += 200L;
    else
        if (num_sustain_wis == 2)
            value += 250L;
        else
            if (num_sustain_wis > 2)
                value += 250L + (num_sustain_wis - 2) * 1L;

    if (num_sustain_con == 1)
        value += 200L;
    else
        if (num_sustain_con == 2)
            value += 250L;
        else
            if (num_sustain_con > 2)
                value += 250L + (num_sustain_con - 2) * 1L;

    if (num_sustain_dex == 1)
        value += 200L;
    else
        if (num_sustain_dex == 2)
            value += 250L;
        else
            if (num_sustain_dex > 2)
                value += 250L + (num_sustain_dex - 2) * 1L;

    if (num_sustain_all == 1)
        value += 1000L;
    else
        if (num_sustain_all == 2)
            value += 1500L;
        else
            if (num_sustain_all > 2)
                value += 1500L + (num_sustain_all - 2) * 1L;

    /* do a minus for too many duplicates.  This way we do not store */
    /* useless items and spread out types of items. */
    if (num_weapons > 5)
        value -= (num_weapons - 5) * 2000L;
    else
        if (num_weapons > 1)
            value -= (num_weapons - 1) * 100L;
    if (num_bow > 2)
        value -= (num_bow - 2) * 1000L;
    if (num_rings > 6)
        value -= (num_rings - 6) * 4000L;
    else
        if (num_rings > 4)
            value -= (num_rings - 4) * 2000L;
    if (num_neck > 3)
        value -= (num_neck - 3) * 1500L;
    else
        if (num_neck > 3)
            value -= (num_neck - 3) * 700L;
    if (num_armor > 6)
        value -= (num_armor - 6) * 1000L;
    if (num_cloaks > 3)
        value -= (num_cloaks - 3) * 1000L;
    if (num_shields > 3)
        value -= (num_shields - 3) * 1000L;
    if (num_hats > 4)
        value -= (num_hats - 4) * 1000L;
    if (num_gloves > 3)
        value -= (num_gloves - 3) * 1000L;
    if (num_boots > 3)
        value -= (num_boots - 3) * 1000L;


    value += home_damage;

    /* if edged and priest, dump it   */
    value -= num_edged_weapon * 3000L;

    /* if gloves and mage or ranger and not FA/Dex, dump it. */
    value -= num_bad_gloves * 3000L;

    /* do not allow duplication of items. */
    value -= num_duplicate_items * 50000L;


    /* Return the value */
    return (value);
}


/*
 * Helper function -- calculate power of items in the home
 *
 * The weird calculations help spread out the purchase order
 */
static int32_t borg_power_home_aux2(void)
{
    int         k, book;

    int32_t        value = 0L;


    /*** Basic abilities ***/

    /* Collect food */
    if (borg_skill[BI_MAXCLEVEL] < 10)
    {
        for (k = 0; k < kb_info[TV_FOOD].max_stack && k < num_food; k++) value += 8000L - k * 10L;
    }

    /* Collect ident */
    for (k = 0; k < kb_info[TV_SCROLL].max_stack && k < num_ident; k++) value += 2000L - k * 10L;

    /* Collect enchantments armour */
    if (borg_skill[BI_CLEVEL] < 45)
    {
        for (k = 0; k < kb_info[TV_SCROLL].max_stack && k < num_enchant_to_a; k++) value += 500L - k * 10L;
    }
    /* Collect enchantments to hit */
    if (borg_skill[BI_CLEVEL] < 45)
    {
        for (k = 0; k < kb_info[TV_SCROLL].max_stack && k < num_enchant_to_h; k++) value += 500L - k * 10L;
    }
    /* Collect enchantments to dam */
    if (borg_skill[BI_CLEVEL] < 45)
    {
        for (k = 0; k < kb_info[TV_SCROLL].max_stack && k < num_enchant_to_d; k++) value += 500L - k * 10L;
    }

    /* Collect pfe */
    for (k = 0; k < kb_info[TV_SCROLL].max_stack && k < num_pfe; k++) value += 500L - k * 10L;

    /* Collect glyphs */
    for (k = 0; k < kb_info[TV_SCROLL].max_stack && k < num_glyph; k++) value += 500L - k * 10L;

    /* Reward Genocide scrolls. Just scrolls, mainly used for Morgoth */
    for (k = 0; k < (kb_info[TV_SCROLL].max_stack * 2) && k < num_genocide; k++) value += 500L - k * 10L;

    /* Reward Mass Genocide scrolls. Just scrolls, mainly used for Morgoth */
    for (k = 0; k < (kb_info[TV_SCROLL].max_stack * 2) && k < num_mass_genocide; k++) value += 500L;

    /* Collect Recharge ability */
    for (k = 0; k < kb_info[TV_SCROLL].max_stack && k < num_recharge; k++) value += 500L - k * 10L;

    /* Reward Resistance Potions for Warriors */
    if (borg_class == CLASS_WARRIOR && borg_skill[BI_MAXDEPTH] > 20 && borg_skill[BI_MAXDEPTH] < 80)
    {
        k = 0;
        for (; k < kb_info[TV_POTION].max_stack && k < num_pot_rheat; k++) value += 100L - k * 10L;
        for (; k < kb_info[TV_POTION].max_stack && k < num_pot_rcold; k++) value += 100L - k * 10L;
    }

    /* Collect recall - stick to 5 spare, for if you run out of money */
    for (k = 0; k < 5 && k < num_recall; k++) value += 100L;

    /* Collect escape  (staff of teleport) */
    if (borg_skill[BI_MAXCLEVEL] < 40)
    {
        for (k = 0; k < 85 && k < num_escape; k++) value += 2000L - k * 10L;
    }

    /* Collect a maximal number of staves in the home */
    for (k = 0; k < kb_info[TV_STAFF].max_stack && k < num_tele_staves; k++) value -= 50000L;

    /* Collect teleport */
    for (k = 0; k < 85 && k < num_teleport; k++) value += 5000L;

    /* Collect phase */
    if (borg_skill[BI_MAXCLEVEL] < 10)
    {
        for (k = 0; k < kb_info[TV_SCROLL].max_stack && k < num_phase; k++) value += 5000L;
    }

    /* Collect Speed */
    /* for (k = 0; k < 85 && k < num_speed; k++) value += 5000L - k*10L; */

    /* collect mana/ */
    if (borg_skill[BI_MAXSP] > 1)
    {
        for (k = 0; k < kb_info[TV_POTION].max_stack && k < num_mana; k++) value += 6000L - k * 8L;
    }

    /* Level 1 priests are given a Potion of Healing.  It is better
     * for them to sell that potion and buy equipment or several
     * Cure Crits with it.
     */
    if (borg_skill[BI_CLEVEL] == 1)
    {
        k = 0;
        for (; k < 10 && k < num_heal; k++) value -= 5000L;
    }

    /*** Healing ***/

    /* Collect cure critical */
    for (k = 0; k < kb_info[TV_POTION].max_stack && k < num_cure_critical; k++) value += 1500L - k * 10L;

    /* Collect heal, *Heal*, Life */
    for (k = 0; k < 90 && k < num_heal; k++) value += 3000L;
    for (k = 0; k < 198 && k < num_ezheal; k++) value += 8000L;
    for (k = 0; k < 198 && k < num_life; k++) value += 9000L;

    /* junk cure serious if we have some in the home */
    if (borg_skill[BI_CLEVEL] > 35)    /* dont bother keeping them if high level */
        for (k = 0; k < 90 && k < num_cure_serious; k++) value -= 1500L - k * 10L;

    /*** Various ***/

    /* Fixing Stats */
    if (borg_skill[BI_CLEVEL] == 50 && num_fix_exp) value -= 7500L;
    if (borg_skill[BI_CLEVEL] > 35 && borg_skill[BI_CLEVEL] <= 49)
        for (k = 0; k < 70 && k < num_fix_exp; k++) value += 1000L - k * 10L;
    else if (borg_skill[BI_CLEVEL] <= 35)
        for (k = 0; k < 5 && k < num_fix_exp; k++) value += 1000L - k * 10L;

    /*** Hack -- books ***/

    /* Reward books */
    for (book = 0; book < 4; book++)
    {
        /* only collect books up to level 14.  */ 
        /* After that, just buy them, they are always in stock*/
        if (borg_skill[BI_CLEVEL] < 15)
        {
            /* Collect up to 5 copies of each normal book */
            for (k = 0; k < 5 && k < num_book[book]; k++)
            {
                /* Hack -- only stockpile useful books */
                if (num_book[book]) value += 5000L - k * 10L;
            }
        }
    }

    /* Reward artifacts in the home */
    value += num_artifact * 500L;

    /* Reward certain types of egos in the home */
    value += num_ego * 5000L;

    /* Only allow unid'd stuff if we can't id them */
    if (home_un_id)
        value += (home_un_id - borg_skill[BI_AID]) * 1005L;

    /* Return the value */
    return (value);
}


/*
 * Calculate the "power" of the home
 */
int32_t borg_power_home(void)
{
    int32_t value = 0L;

    /* Process the home equipment */
    value += borg_power_home_aux1();

    /* Process the home inventory */
    value += borg_power_home_aux2();

    /* Return the value */
    return (value);
}


/* *HACK* must match code in mon-blows.c */
int borg_mon_blow_effect(const char* name)
{
    static const struct {
        const char* name;
        enum BORG_MONBLOW	val;
    } monblow[] = {
    { "NONE", MONBLOW_NONE },
    { "HURT", MONBLOW_HURT },
    { "POISON", MONBLOW_POISON },
    { "DISENCHANT", MONBLOW_DISENCHANT},
    { "DRAIN_CHARGES", MONBLOW_DRAIN_CHARGES },
    { "EAT_GOLD", MONBLOW_EAT_GOLD },
    { "EAT_ITEM", MONBLOW_EAT_ITEM },
    { "EAT_FOOD", MONBLOW_EAT_FOOD },
    { "EAT_LIGHT", MONBLOW_EAT_LIGHT },
    { "ACID", MONBLOW_ACID },
    { "ELEC", MONBLOW_ELEC },
    { "FIRE", MONBLOW_FIRE },
    { "COLD", MONBLOW_COLD },
    { "BLIND", MONBLOW_BLIND },
    { "CONFUSE", MONBLOW_CONFUSE },
    { "TERRIFY", MONBLOW_TERRIFY },
    { "PARALYZE", MONBLOW_PARALYZE },
    { "LOSE_STR", MONBLOW_LOSE_STR },
    { "LOSE_INT", MONBLOW_LOSE_INT },
    { "LOSE_WIS", MONBLOW_LOSE_WIS },
    { "LOSE_DEX", MONBLOW_LOSE_DEX },
    { "LOSE_CON", MONBLOW_LOSE_CON },
    { "LOSE_ALL", MONBLOW_LOSE_ALL },
    { "SHATTER", MONBLOW_SHATTER },
    { "EXP_10", MONBLOW_EXP_10 },
    { "EXP_20", MONBLOW_EXP_20 },
    { "EXP_40", MONBLOW_EXP_40 },
    { "EXP_80", MONBLOW_EXP_80 },
    { "HALLU", MONBLOW_HALLU },
    { "BLACK_BREATH", MONBLOW_BLACK_BREATH },
    { NULL, MONBLOW_NONE } };
    unsigned long i;
    for (i = 0; i < sizeof(monblow) / sizeof(monblow[0]); ++i)
        if (!strcmp(name, monblow[i].name))
            return monblow[i].val;
    return MONBLOW_NONE;
}


/*
 * Calculate base danger from a monster's physical attacks
 *
 * We attempt to take account of various resistances, both in
 * terms of actual damage, and special effects, as appropriate.
 *
 * We reduce the danger from distant "sleeping" monsters.
 * PFE reduces my fear of an area.
 *
 */
static int borg_danger_aux1(int i, bool full_damage)
{
    int k, n = 0;
    int pfe = 0;
    int power, chance;

    int16_t ac = borg_skill[BI_ARMOR];

    borg_kill* kill = &borg_kills[i];

    struct monster_race* r_ptr = &r_info[kill->r_idx];

    /* shields gives +50 to ac and deflects some missiles and balls*/
    if (borg_shield)
        ac += 50;

    /*  PFE gives a protection.  */
    /* Hack -- Apply PROTECTION_FROM_EVIL */
    if ((borg_prot_from_evil) &&
        (rf_has(r_ptr->flags, RF_EVIL)) &&
        ((borg_skill[BI_CLEVEL]) >= r_ptr->level))
    {
        pfe = 1;
    }

    /* Mega-Hack -- unknown monsters */
    if (kill->r_idx == 0) return (1000);
    if (kill->r_idx >= z_info->r_max) return (1000);

    /* Analyze each physical attack */
    for (k = 0; k < z_info->mon_blows_max; k++)
    {
        int z = 0;

        /* Extract the attack infomation */
        struct blow_effect* effect = r_ptr->blow[k].effect;
        struct blow_method* method = r_ptr->blow[k].method;
        int d_dice = r_ptr->blow[k].dice.dice;
        int d_side = r_ptr->blow[k].dice.sides;

        power = 0;

        /* Done */
        if (!method) break;

        /* Analyze the attack */
        switch (borg_mon_blow_effect(effect->name))
        {
        case MONBLOW_HURT:
        z = (d_dice * d_side);
        /* stun */
        if ((d_side < 3) && (z > d_dice * d_side))
        {
            n += 200;
        }
        /* fudge- only mystics kick and they tend to KO.  Avoid close */
        /* combat like the plauge */
        if (method->stun)
        {
            n += 400;
        }
        power = 60;
        if ((pfe) && !borg_attacking)
        {
            z /= 2;
        }
        break;

        case MONBLOW_POISON:
        z = (d_dice * d_side);
        power = 5;
        if (borg_skill[BI_RPOIS]) break;
        if (borg_skill[BI_TRPOIS]) break;
        /* Add fear for the effect */
        z += 10;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_DISENCHANT:
        z = (d_dice * d_side);
        power = 20;
        if (borg_skill[BI_RDIS]) break;
        /* Add fear for the effect */
        z += 500;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_DRAIN_CHARGES:
        z = (d_dice * d_side);
        /* Add fear for the effect */
        z += 20;
        power = 15;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_EAT_GOLD:
        z = (d_dice * d_side);
        /* if in town and low level avoid them stupid urchins */
        if (borg_skill[BI_CLEVEL] < 5) z += 50;
        power = 5;
        if (100 <= adj_dex_safe[my_stat_ind[STAT_DEX]] + borg_skill[BI_CLEVEL]) break;
        if (borg_gold < 100) break;
        if (borg_gold > 100000) break;
        /* Add fear for the effect */
        z += 5;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_EAT_ITEM:
        z = (d_dice * d_side);
        power = 5;
        if (100 <= adj_dex_safe[my_stat_ind[STAT_DEX]] + borg_skill[BI_CLEVEL]) break;
        /* Add fear for the effect */
        z += 5;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_EAT_FOOD:
        z = (d_dice * d_side);
        power = 5;
        if (borg_skill[BI_FOOD] > 5) break;
        /* Add fear for the effect */
        z += 5;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_EAT_LIGHT:
        z = (d_dice * d_side);
        power = 5;
        if (borg_skill[BI_CURLITE] == 0) break;
        if (borg_skill[BI_AFUEL] > 5) break;
        /* Add fear for the effect */
        z += 5;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_ACID:
        if (borg_skill[BI_IACID]) break;
        z = (d_dice * d_side);
        if (borg_skill[BI_RACID]) z = (z + 2) / 3;
        if (borg_skill[BI_TRACID]) z = (z + 2) / 3;
        /* Add fear for the effect */
        z += 200; /* We dont want our armour corroded. */
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_ELEC:
        if (borg_skill[BI_IELEC]) break;
        z = (d_dice * d_side);
        power = 10;
        if (borg_skill[BI_RELEC]) z = (z + 2) / 3;
        if (borg_skill[BI_TRELEC]) z = (z + 2) / 3;
        /* Add fear for the effect */
        z = z * 2;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_FIRE:
        if (borg_skill[BI_IFIRE]) break;
        z = (d_dice * d_side);
        power = 10;
        if (borg_skill[BI_RFIRE]) z = (z + 2) / 3;
        if (borg_skill[BI_TRFIRE]) z = (z + 2) / 3;
        /* Add fear for the effect */
        z = z * 2;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_COLD:
        if (borg_skill[BI_ICOLD]) break;
        z = (d_dice * d_side);
        power = 10;
        if (borg_skill[BI_RCOLD]) z = (z + 2) / 3;
        if (borg_skill[BI_TRCOLD]) z = (z + 2) / 3;
        /* Add fear for the effect */
        z = z * 2;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_BLIND:
        z = (d_dice * d_side);
        power = 2;
        if (borg_skill[BI_RBLIND]) break;
        /* Add fear for the effect */
        z += 10;
        if (borg_class == CLASS_MAGE) z += 75;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_CONFUSE:
        z = (d_dice * d_side);
        power = 10;
        if (borg_skill[BI_RCONF]) break;
        /* Add fear for the effect */
        z += 200;
        if (borg_class == CLASS_MAGE) z += 200;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_TERRIFY:
        z = (d_dice * d_side);
        power = 10;
        if (borg_skill[BI_RFEAR]) break;
        /* Add fear for the effect */
        z = z * 2;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_PARALYZE:
        z = (d_dice * d_side);
        power = 2;
        if (borg_skill[BI_FRACT]) break;
        z += 200;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_LOSE_STR:
        z = (d_dice * d_side);
        if (borg_skill[BI_SSTR]) break;
        if (borg_stat[STAT_STR] <= 3) break;
        if (borg_spell_legal(RESTORATION)) break;
        if (borg_spell_legal(REVITALIZE)) break;
        if (borg_spell_legal(UNHOLY_REPRIEVE)) break;
        z += 150;
        /* extra scary to have str drain below 10 */
        if (borg_stat[STAT_STR] < 10)
            z += 100;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_LOSE_DEX:
        z = (d_dice * d_side);
        if (borg_skill[BI_SDEX]) break;
        if (borg_stat[STAT_DEX] <= 3) break;
        if (borg_spell_legal(RESTORATION)) break;
        if (borg_spell_legal(REVITALIZE)) break;
        z += 150;
        /* extra scary to have drain below 10 */
        if (borg_stat[STAT_DEX] < 10)
            z += 100;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_LOSE_CON:
        z = (d_dice * d_side);
        if (borg_skill[BI_SCON]) break;
        if (borg_stat[STAT_CON] <= 3) break;
        if (borg_spell_legal(RESTORATION)) break;
        if (borg_spell_legal(REVITALIZE)) break;
        if (borg_spell_legal(UNHOLY_REPRIEVE)) break;
        /* Add fear for the effect */
        z += 150;
        /* extra scary to have con drain below 8 */
        if (borg_stat[STAT_STR] < 8)
            z += 100;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_LOSE_INT:
        z = (d_dice * d_side);
        if (borg_skill[BI_SINT]) break;
        if (borg_stat[STAT_INT] <= 3) break;
        if (borg_spell_legal(RESTORATION)) break;
        if (borg_spell_legal(REVITALIZE)) break;
        if (borg_spell_legal(UNHOLY_REPRIEVE)) break;
        z += 150;
        /* extra scary for spell caster */
        if (borg_spell_stat() == STAT_INT)
            z += 50;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_LOSE_WIS:
        z = (d_dice * d_side);
        if (borg_skill[BI_SWIS]) break;
        if (borg_stat[STAT_WIS] <= 3) break;
        if (borg_spell_legal(RESTORATION)) break;
        if (borg_spell_legal(REVITALIZE)) break;
        z += 150;
        /* extra scary for pray'er */
        if (borg_spell_stat() == STAT_WIS)
            z += 50;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_LOSE_ALL:
        z = (d_dice * d_side);
        power = 2;
        /* only morgoth. HACK to make it easier to fight him */
        break;

        case MONBLOW_SHATTER:
        z = (d_dice * d_side);
        z -= (z * ((ac < 150) ? ac : 150) / 250);
        power = 60;
        /* Add fear for the effect */
        z += 150;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_EXP_10:
        z = (d_dice * d_side);
        if (borg_skill[BI_HLIFE]) break;
        /* do not worry about drain exp after level 50 */
        if (borg_skill[BI_CLEVEL] == 50) break;
        if (borg_spell_legal(REMEMBRANCE) || borg_spell_legal(UNHOLY_REPRIEVE) || borg_spell_legal(REVITALIZE))  break;
        /* Add fear for the effect */
        z += 100;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_EXP_20:
        z = (d_dice * d_side);
        if (borg_skill[BI_HLIFE]) break;
        /* do not worry about drain exp after level 50 */
        if (borg_skill[BI_CLEVEL] >= 50) break;
        if (borg_spell_legal(REMEMBRANCE) || borg_spell_legal(UNHOLY_REPRIEVE) || borg_spell_legal(REVITALIZE))  break;
        /* Add fear for the effect */
        z += 150;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_EXP_40:
        z = (d_dice * d_side);
        if (borg_skill[BI_HLIFE]) break;
        /* do not worry about drain exp after level 50 */
        if (borg_skill[BI_CLEVEL] >= 50) break;
        if (borg_spell_legal(REMEMBRANCE) || borg_spell_legal(UNHOLY_REPRIEVE) || borg_spell_legal(REVITALIZE))  break;
        /* Add fear for the effect */
        z += 200;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_EXP_80:
        z = (d_dice * d_side);
        if (borg_skill[BI_HLIFE]) break;
        /* do not worry about drain exp after level 50 */
        if (borg_skill[BI_CLEVEL] >= 50) break;
        if (borg_spell_legal(REMEMBRANCE) || borg_spell_legal(UNHOLY_REPRIEVE) || borg_spell_legal(REVITALIZE))  break;
        /* Add fear for the effect */
        z += 250;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;

        case MONBLOW_HALLU:
        z = (d_dice * d_side);
        /* Add fear for the effect */
        z += 250;
        if ((pfe) && !borg_attacking)
            z /= 2;
        break;
        }

        /* reduce by damage reduction */
        z -= borg_skill[BI_DAM_RED];
        if (z < 0) z = 0;

        /* if we are doing partial damage reduce for % chance that it will */
        /* hit you. */
        if (!full_damage)
        {
            /* figure out chance that monster will hit you. */
            /* add a 50% bonus in to account for bad luck. */
            if (borg_fighting_unique || (r_ptr->level + power) > 0)
                chance = 150 - (((ac * 300) / 4) / 1 + ((r_ptr->level + power) * 3));
            else
                chance = -1;

            /* always have a 5% chance of hitting. */
            if (chance < 5) chance = 5;

            /* Modify the damage by the chance of getting hit */
            z = (z * chance) / 100;
        }

        /* Add in damage */
        n += z;
    }

    /* Danger */
    return (n);
}


/*
 * Calculate base danger from a monster's spell attacks
 *
 * We attempt to take account of various resistances, both in
 * terms of actual damage, and special effects, as appropriate.
 *
 * We reduce the danger from distant "sleeping" monsters.
 *
 * We reduce the danger if the monster is immobile or not LOS
 */
static int borg_danger_aux2(int i, int y, int x, int d, bool average, bool full_damage)
{
    int q, n = 0, pfe = 0, glyph = 0, glyph_check = 0;

    int spot_x, spot_y, spot_safe = 1;

    int hp, total_dam = 0, av;

    bool bolt = false;

    borg_kill* kill = &borg_kills[i];

    borg_grid* ag;

    struct monster_race* r_ptr = &r_info[kill->r_idx];

    /*  PFE gives a protection.  */
        /* Hack -- Apply PROTECTION_FROM_EVIL */
    if ((borg_prot_from_evil) &&
        (rf_has(r_ptr->flags, RF_EVIL)) &&
        ((borg_skill[BI_CLEVEL]) >= r_ptr->level))
    {
        pfe = 1;
    }

    /* glyph of warding rune of protection provides some small
     * protection with some ranged atacks; mainly summon attacks.
     * We should reduce the danger commensurate to the probability of the
     * monster breaking the glyph as defined by melee2.c
     */
    if (borg_on_glyph)
    {
        glyph = 1;
    }
    else if (track_glyph.num)
    {
        /* Check all existing glyphs */
        for (glyph_check = 0; glyph_check < track_glyph.num; glyph_check++)
        {
            if ((track_glyph.y[glyph_check] == y) && (track_glyph.x[glyph_check] == x))
            {
                /* Reduce the danger */
                glyph = 1;
            }
        }
    }

    /* Mega-Hack -- unknown monsters */
    if (kill->r_idx == 0) return (1000);
    if (kill->r_idx >= z_info->r_max) return (1000);


    /* Paranoia -- Nothing to cast */
    if (!kill->ranged_attack) return (0);

    /* Extract hit-points */
    hp = kill->power;

    /* Analyze the spells */
    for (q = 0; q < kill->ranged_attack; q++)
    {
        int p = 0;

        int z = 0;

        /* Cast the spell. */
        switch (kill->spell[q])
        {
        case RSF_SHRIEK:
        /* if looking at full damage, things that are just annoying */
        /* do not count.*/
        /* Add fear for the effect */
        p += 5;
        break;

        case RSF_WHIP:
        if (d < 3)
        {
            z = 100;
        }
        break;

        case RSF_SPIT:
        if (d < 4)
        {
            z = 100;
        }
        break;

        case RSF_SHOT:
        z = ((r_ptr->spell_power / 8) + 1) * 5;
        break;

        case RSF_ARROW:
        z = ((r_ptr->spell_power / 8) + 1) * 6;
        break;

        case RSF_BOLT:
        z = ((r_ptr->spell_power / 8) + 1) * 7;
        break;

        case RSF_BR_ACID:
        if (borg_skill[BI_IACID]) break;
        z = (hp / 3);
        /* max damage */
        if (z > 1600)
            z = 1600;
        if (borg_skill[BI_RACID]) z = (z + 2) / 3;
        if (borg_skill[BI_TRACID]) z = (z + 2) / 3;
        /* if looking at full damage, things that are just annoying */
        /* do not count.*/
        /* Add fear for the effect */
        p += 40;
        break;

        case RSF_BR_ELEC:
        if (borg_skill[BI_IELEC]) break;
        z = (hp / 3);
        /* max damage */
        if (z > 1600)
            z = 1600;
        if (borg_skill[BI_RELEC]) z = (z + 2) / 3;
        if (borg_skill[BI_TRELEC]) z = (z + 2) / 3;
        /* if looking at full damage, things that are just annoying */
        /* do not count.*/
        /* Add fear for the effect */
        p += 20;
        break;

        case RSF_BR_FIRE:
        if (borg_skill[BI_IFIRE]) break;
        z = (hp / 3);
        /* max damage */
        if (z > 1600)
            z = 1600;
        if (borg_skill[BI_RFIRE]) z = (z + 2) / 3;
        if (borg_skill[BI_TRFIRE]) z = (z + 2) / 3;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 40;
        break;

        case RSF_BR_COLD:
        if (borg_skill[BI_ICOLD]) break;
        z = (hp / 3);
        /* max damage */
        if (z > 1600)
            z = 1600;
        if (borg_skill[BI_RCOLD]) z = (z + 2) / 3;
        if (borg_skill[BI_TRCOLD]) z = (z + 2) / 3;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 20;
        break;

        case RSF_BR_POIS:
        z = (hp / 3);
        /* max damage */
        if (z > 800)
            z = 800;
        if (borg_skill[BI_RPOIS]) z = (z + 2) / 3;
        if (borg_skill[BI_TRPOIS]) z = (z + 2) / 3;
        if (borg_skill[BI_TRPOIS]) break;
        if (borg_skill[BI_RPOIS]) break;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 20;
        break;

        case RSF_BR_NETH:
        z = (hp / 6);
        /* max damage */
        if (z > 600)
            z = 600;
        if (borg_skill[BI_RNTHR])
        {
            z = (z * 6) / 9;
            break;
        }
        /* Add fear for the effect */
        p += 125;
        break;

        case RSF_BR_LIGHT:
        z = (hp / 6);
        /* max damage */
        if (z > 500)
            z = 500;
        if (borg_skill[BI_RLITE])
        {
            z = (z * 2) / 3;
            break;
        }
        if (borg_skill[BI_RBLIND]) break;
        p += 20;
        if (borg_class == CLASS_MAGE) p += 20;
        break;

        case RSF_BR_DARK:
        z = (hp / 6);
        /* max damage */
        if (z > 500)
            z = 500;
        if (borg_skill[BI_RDARK]) z = (z * 2) / 3;
        if (borg_skill[BI_RDARK]) break;
        if (borg_skill[BI_RBLIND]) break;
        p += 20;
        if (borg_class == CLASS_MAGE) p += 20;
        break;

        case RSF_BR_SOUN:
        z = (hp / 6);
        /* max damage */
        if (z > 500)
            z = 500;
        if (borg_skill[BI_RSND]) z = (z * 5) / 9;
        if (borg_skill[BI_RSND]) break;
        /* if already stunned be REALLY nervous dangerousabout this */
        if (borg_skill[BI_ISSTUN])
            z += 500;
        if (borg_skill[BI_ISHEAVYSTUN])
            z += 1000;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 50;
        break;

        case RSF_BR_CHAO:
        z = (hp / 6);
        /* max damage */
        if (z > 600)
            z = 600;
        if (borg_skill[BI_RKAOS]) z = (z * 6) / 9;
        /* Add fear for the effect */
        p += 100;
        if (borg_skill[BI_RKAOS]) break;
        p += 200;
        break;

        case RSF_BR_DISE:
        z = (hp / 6);
        /* max damage */
        if (z > 500)
            z = 500;
        if (borg_skill[BI_RDIS]) z = (z * 6) / 10;
        if (borg_skill[BI_RDIS]) break;
        p += 500;
        break;

        case RSF_BR_NEXU:
        z = (hp / 6);
        /* max damage */
        if (z > 400)
            z = 400;
        if (borg_skill[BI_RNXUS]) z = (z * 6) / 10;
        if (borg_skill[BI_RNXUS]) break;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 100;
        break;

        case RSF_BR_TIME:
        z = (hp / 3);
        /* max damage */
        if (z > 150)
            z = 150;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 250;
        break;

        case RSF_BR_INER:
        z = (hp / 6);
        /* max damage */
        if (z > 200)
            z = 200;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 100;
        break;

        case RSF_BR_GRAV:
        z = (hp / 3);
        /* max damage */
        if (z > 200)
            z = 200;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 100;
        if (borg_skill[BI_RSND]) break;
        /* if already stunned be REALLY nervous about this */
        if (borg_skill[BI_ISSTUN])
            z += 500;
        if (borg_skill[BI_ISHEAVYSTUN])
            z += 1000;
        break;

        case RSF_BR_SHAR:
        z = (hp / 6);
        /* max damage */
        if (z > 500)
            z = 500;
        if (borg_skill[BI_RSHRD]) z = (z * 6) / 9;
        if (borg_skill[BI_RSHRD]) break;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 50;
        break;

        case RSF_BR_PLAS:
        z = (hp / 6);
        /* max damage */
        if (z > 150)
            z = 150;
        if (borg_skill[BI_RSND]) break;
        /* Pump this up if you have goi so that the borg is sure */
        /* to be made nervous */
        p += 100;
        /* if already stunned be REALLY nervous about this */
        if (borg_skill[BI_ISSTUN])
            z += 500;
        if (borg_skill[BI_ISHEAVYSTUN])
            z += 1000;
        break;

        case RSF_BR_WALL:
        z = (hp / 6);
        /* max damage */
        if (z > 200)
            z = 200;
        if (borg_skill[BI_RSND]) break;
        /* if already stunned be REALLY nervous about this */
        if (borg_skill[BI_ISSTUN])
            z += 100;
        if (borg_skill[BI_ISHEAVYSTUN])
            z += 500;
        /* Add fear for the effect */
        p += 50;
        break;

        case RSF_BR_MANA:
        /* Nothing currently breaths mana but best to have a check */
        z = (hp / 3);
        /* max damage */
        if (z > 1600)
            z = 1600;
        break;

        case RSF_BOULDER:
        z = ((1 + r_ptr->spell_power / 7) * 12);
        bolt = true;
        break;

        case RSF_WEAVE:
        /* annoying creation of traps */
        break;

        case RSF_BA_ACID:
        if (borg_skill[BI_IACID]) break;
        z = (r_ptr->spell_power * 3) + 15;
        if (borg_skill[BI_RACID]) z = (z + 2) / 3;
        if (borg_skill[BI_TRACID]) z = (z + 2) / 3;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 40;
        break;

        case RSF_BA_ELEC:
        if (borg_skill[BI_IELEC]) break;
        z = (r_ptr->spell_power * 3) / 2 + 8;
        if (borg_skill[BI_RELEC]) z = (z + 2) / 3;
        if (borg_skill[BI_TRELEC]) z = (z + 2) / 3;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 20;
        break;

        case RSF_BA_FIRE:
        if (borg_skill[BI_IFIRE]) break;
        z = (r_ptr->spell_power * 7) / 2 + 10;
        if (borg_skill[BI_RFIRE]) z = (z + 2) / 3;
        if (borg_skill[BI_TRFIRE]) z = (z + 2) / 3;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 40;
        break;

        case RSF_BA_COLD:
        if (borg_skill[BI_ICOLD]) break;
        z = (r_ptr->spell_power * 3 / 2) + 10;
        if (borg_skill[BI_RCOLD]) z = (z + 2) / 3;
        if (borg_skill[BI_TRCOLD]) z = (z + 2) / 3;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 20;
        break;

        case RSF_BA_POIS:
        z = (r_ptr->spell_power / 2 + 3) * 4;
        if (borg_skill[BI_RPOIS]) z = (z + 2) / 3;
        if (borg_skill[BI_TRPOIS]) z = (z + 2) / 3;
        if (borg_skill[BI_TRPOIS]) break;
        if (borg_skill[BI_RPOIS]) break;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 20;
        break;

        case RSF_BA_SHAR:
        z = ((r_ptr->spell_power * 3) / 2) + 10;
        if (borg_skill[BI_RSHRD]) z = (z * 6) / 9;
        if (borg_skill[BI_RSHRD]) break;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 20;
        break;

        case RSF_BA_NETH:
        z = (r_ptr->spell_power * 4) + (10 * 10);
        if (borg_skill[BI_RNTHR]) z = (z * 6) / 8;
        if (borg_skill[BI_RNTHR]) break;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 250;
        break;

        case RSF_BA_WATE:
        z = (r_ptr->spell_power * 5) / 2 + 50;
        if (borg_skill[BI_RSND]) break;
        /* if already stunned be REALLY nervous about this */
        if (borg_skill[BI_ISSTUN])
            p += 500;
        if (borg_skill[BI_ISHEAVYSTUN])
            p += 1000;
        if (borg_skill[BI_RCONF]) break;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 50;
        if (borg_class == CLASS_MAGE) p += 20;
        break;

        case RSF_BA_MANA:
        z = ((r_ptr->spell_power * 5) + (10 * 10));
        /* Add fear for the effect */
        p += 50;
        break;

        case RSF_BA_HOLY:
        z = 10 + ((r_ptr->spell_power * 3) / 2 + 1) / 2;
        /* Add fear for the effect */
        p += 50;
        break;

        case RSF_BA_DARK:
        z = ((r_ptr->spell_power * 4) + (10 * 10));
        if (borg_skill[BI_RDARK]) z = (z * 6) / 9;
        if (borg_skill[BI_RDARK]) break;
        if (borg_skill[BI_RBLIND]) break;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 20;
        if (borg_class == CLASS_MAGE) p += 20;
        break;

        case RSF_BA_LIGHT:
        z = 10 + (r_ptr->spell_power * 3 / 2);
        if (borg_skill[BI_RLITE]) z = (z * 6) / 9;
        if (borg_skill[BI_RLITE]) break;
        if (borg_skill[BI_RBLIND]) break;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 20;
        if (borg_class == CLASS_MAGE) p += 20;
        break;

        case RSF_STORM:
        z = 70 + (r_ptr->spell_power * 5);
        if (borg_skill[BI_RSND]) break;
        /* if already stunned be REALLY nervous about this */
        if (borg_skill[BI_ISSTUN])
            p += 500;
        if (borg_skill[BI_ISHEAVYSTUN])
            p += 1000;
        if (borg_skill[BI_RCONF]) break;
        break;

        case RSF_DRAIN_MANA:
        if (borg_skill[BI_MAXSP]) p += 100;
        break;

        case RSF_MIND_BLAST:
        if (borg_skill[BI_SAV] < 100)
            z = (r_ptr->spell_power / 2 + 1);
        break;

        case RSF_BRAIN_SMASH:
        z = (12 * (15 + 1)) / 2;
        p += 200 - 2 * borg_skill[BI_SAV];
        if (p < 0) p = 0;
        break;

        case RSF_WOUND:
        if (borg_skill[BI_SAV] >= 100) break;
        z = ((r_ptr->spell_power / 3 * 2) * 5);
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        /* reduce by % chance of save  (add 20% for fudge) */
        z = z * (120 - borg_skill[BI_SAV]) / 100;
        break;

        case RSF_BO_ACID:
        bolt = true;
        if (borg_skill[BI_IACID]) break;
        z = ((7 * 8) + (r_ptr->spell_power / 3));
        if (borg_skill[BI_RACID]) z = (z + 2) / 3;
        if (borg_skill[BI_TRACID]) z = (z + 2) / 3;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 40;
        break;

        case RSF_BO_ELEC:
        if (borg_skill[BI_IELEC]) break;
        bolt = true;
        z = ((4 * 8) + (r_ptr->spell_power / 3));
        if (borg_skill[BI_RELEC]) z = (z + 2) / 3;
        if (borg_skill[BI_TRELEC]) z = (z + 2) / 3;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 20;
        break;

        case RSF_BO_FIRE:
        if (borg_skill[BI_IFIRE]) break;
        bolt = true;
        z = ((9 * 8) + (r_ptr->spell_power / 3));
        if (borg_skill[BI_RFIRE]) z = (z + 2) / 3;
        if (borg_skill[BI_TRFIRE]) z = (z + 2) / 3;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 40;
        break;

        case RSF_BO_COLD:
        if (borg_skill[BI_ICOLD]) break;
        bolt = true;
        z = ((6 * 8) + (r_ptr->spell_power / 3));
        if (borg_skill[BI_RCOLD]) z = (z + 2) / 3;
        if (borg_skill[BI_TRCOLD]) z = (z + 2) / 3;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 20;
        break;

        case RSF_BO_POIS:
        if (borg_skill[BI_IPOIS]) break;
        z = ((9 * 8) + (r_ptr->spell_power / 3));
        if (borg_skill[BI_RPOIS]) z = (z + 2) / 3;
        if (borg_skill[BI_TRPOIS]) z = (z + 2) / 3;
        bolt = true;
        break;

        case RSF_BO_NETH:
        bolt = true;
        z = (5 * 5) + (r_ptr->spell_power * 3 / 2) + 50;
        if (borg_skill[BI_RNTHR]) z = (z * 6) / 8;
        if (borg_skill[BI_RNTHR]) break;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 200;
        break;

        case RSF_BO_WATE:
        z = (10 * 10) + (r_ptr->spell_power);
        bolt = true;
        if (borg_skill[BI_RSND]) break;
        /* if already stunned be REALLY nervous about this */
        if (borg_skill[BI_ISSTUN])
            p += 500;
        if (borg_skill[BI_ISHEAVYSTUN])
            p += 1000;
        if (borg_skill[BI_RCONF]) break;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 20;
        if (borg_class == CLASS_MAGE) p += 20;
        break;

        case RSF_BO_MANA:
        z = (r_ptr->spell_power * 5) / 2 + 50;
        bolt = true;
        /* Add fear for the effect */
        p += 50;
        break;

        case RSF_BO_PLAS:
        z = (10 + (8 * 7) + (r_ptr->spell_power));
        bolt = true;
        if (borg_skill[BI_RSND]) break;
        /* if already stunned be REALLY nervous about this */
        if (borg_skill[BI_ISSTUN])
            z += 500;
        if (borg_skill[BI_ISHEAVYSTUN])
            z += 1000;
        break;

        case RSF_BO_ICE:
        z = (6 * 6) + (r_ptr->spell_power);
        bolt = true;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 20;
        if (borg_skill[BI_RSND]) break;
        /* if already stunned be REALLY nervous about this */
        if (borg_skill[BI_ISSTUN])
            z += 50;
        if (borg_skill[BI_ISHEAVYSTUN])
            z += 1000;
        break;

        case RSF_MISSILE:
        z = ((2 * 6) + (r_ptr->spell_power / 3));
        bolt = true;
        break;

        case RSF_BE_ELEC:
        if (borg_skill[BI_IELEC]) break;
        z = ((5 * 5) + (r_ptr->spell_power * 2) + 30);
        if (borg_skill[BI_RELEC]) z = (z + 2) / 3;
        if (borg_skill[BI_TRELEC]) z = (z + 2) / 3;
        bolt = true;
        break;

        case RSF_BE_NETH:
        bolt = true;
        z = ((5 * 5) + (r_ptr->spell_power * 2) + 30);
        if (borg_skill[BI_RNTHR]) z = (z * 6) / 8;
        if (borg_skill[BI_RNTHR]) break;
        bolt = true;
        break;

        case RSF_SCARE:
        if (borg_skill[BI_SAV] >= 100) break;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 10;
        break;

        case RSF_BLIND:
        if (borg_skill[BI_RBLIND]) break;
        if (borg_skill[BI_SAV] >= 100) break;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 10;
        break;

        case RSF_CONF:
        if (borg_skill[BI_RCONF]) break;
        if (borg_skill[BI_SAV] >= 100) break;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 10;
        break;

        case RSF_SLOW:
        if (borg_skill[BI_FRACT]) break;
        if (borg_skill[BI_SAV] >= 100) break;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 5;
        break;

        case RSF_HOLD:
        if (borg_skill[BI_FRACT]) break;
        if (borg_skill[BI_SAV] >= 100) break;
        p += 150;
        break;

        case RSF_HASTE:
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 10;
        break;

        case RSF_HEAL:
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 10;
        break;

        case RSF_HEAL_KIN:
        break;

        case RSF_BLINK:
        break;

        case RSF_TPORT:
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 10;
        break;

        case RSF_TELE_TO:
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 20;
        break;

        case RSF_TELE_SELF_TO:
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 20;
        break;

        case RSF_TELE_AWAY:
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 10;
        break;

        case RSF_TELE_LEVEL:
        if (borg_skill[BI_SAV] >= 100) break;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 50;
        break;

        case RSF_DARKNESS:
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 5;
        break;

        case RSF_TRAPS:
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 50;
        break;

        case RSF_FORGET:
        if (borg_skill[BI_SAV] >= 100) break;
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        {
            /* if you are a spell caster, this is a big issue */
            if (borg_skill[BI_CURSP] < 15)
            {
                p += 500;
            }
            else
            {
                p += 30;
            }
        }
        break;

        case RSF_SHAPECHANGE:
        /* if looking at full damage, things that are just annoying */
        /* do not count. */
        /* Add fear for the effect */
        p += 200;
        break;

        /* Summoning is only as dangerous as the monster that is
         * actually summoned but the monsters that summon are a priority
         * to kill.  PFE reduces danger from some evil summoned monsters
         * One Problem with GOI and Create Door is that the GOI reduces
         * the fear so much that the borg won't cast the Create Door,
         * eventhough it would be a good idea.
         */

        case RSF_S_KIN:
        /* This is used to calculate the free squares next to us.
         * This is important when dealing with summoners.
         */
        for (spot_x = -1; spot_x <= 1; spot_x++)
        {
            for (spot_y = -1; spot_y <= 1; spot_y++)
            {
                /* Acquire location */
                x = spot_x + kill->x;
                y = spot_y + kill->y;

                ag = &borg_grids[y][x];

                /* skip our own spot */
                if (x == kill->x && y == kill->y) continue;

                /* track spaces already protected */
                if (borg_feature_protected(ag))
                {   /* track the safe areas for calculating danger */
                    spot_safe++;

                    /* Just in case */
                    if (spot_safe == 0) spot_safe = 1;
                    if (spot_safe == 8) spot_safe = 100;
                    if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
                }

            }
        }
        if (pfe)
        {
            p += (r_ptr->spell_power);
            p = p / spot_safe;
        }
        else if (glyph || borg_create_door || borg_fighting_unique)
        {
            p += (r_ptr->spell_power) * 3;
            p = p / spot_safe;
        }
        else
        {
            p += (r_ptr->spell_power) * 7;
            p = p / spot_safe;
        }
        /* reduce the fear if it is a unique */
        if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75 / 100;

        break;

        case RSF_S_HI_DEMON:
        /* This is used to calculate the free squares next to us.
         * This is important when dealing with summoners.
         */
        for (spot_x = -1; spot_x <= 1; spot_x++)
        {
            for (spot_y = -1; spot_y <= 1; spot_y++)
            {
                /* Acquire location */
                x = spot_x + kill->x;
                y = spot_y + kill->y;

                ag = &borg_grids[y][x];

                /* skip our own spot */
                if (x == kill->x && y == kill->y) continue;

                /* track spaces already protected */
                if (borg_feature_protected(ag))
                {
                    /* track the safe areas for calculating danger */
                    spot_safe++;

                    /* Just in case */
                    if (spot_safe == 0) spot_safe = 1;
                    if (spot_safe == 8) spot_safe = 100;
                    if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
                }

            }
        }
        if (pfe)
        {
            p += (r_ptr->spell_power);
            p = p / spot_safe;
        }
        else if (glyph || borg_create_door || borg_fighting_unique)
        {
            p += (r_ptr->spell_power) * 6;
            p = p / spot_safe;
        }
        else
        {
            p += (r_ptr->spell_power) * 12;
            p = p / spot_safe;
        }
        /* reduce the fear if it is a unique */
        if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75 / 100;
        break;


        case RSF_S_MONSTER:
        /* This is used to calculate the free squares next to us.
         * This is important when dealing with summoners.
         */
        for (spot_x = -1; spot_x <= 1; spot_x++)
        {
            for (spot_y = -1; spot_y <= 1; spot_y++)
            {
                /* Acquire location */
                x = spot_x + kill->x;
                y = spot_y + kill->y;

                ag = &borg_grids[y][x];

                /* skip our own spot */
                if (x == kill->x && y == kill->y) continue;

                /* track spaces already protected */
                if (borg_feature_protected(ag))
                {
                    /* track the safe areas for calculating danger */
                    spot_safe++;

                    /* Just in case */
                    if (spot_safe == 0) spot_safe = 1;
                    if (spot_safe == 8) spot_safe = 100;
                    if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
                }

            }
        }
        if (pfe || glyph || borg_create_door || borg_fighting_unique)
            p += 0;
        else
        {
            p += (r_ptr->spell_power) * 5;
            p = p / spot_safe;
        }
        break;

        case RSF_S_MONSTERS:
        /* This is used to calculate the free squares next to us.
         * This is important when dealing with summoners.
         */
        for (spot_x = -1; spot_x <= 1; spot_x++)
        {
            for (spot_y = -1; spot_y <= 1; spot_y++)
            {
                /* Acquire location */
                x = spot_x + kill->x;
                y = spot_y + kill->y;

                ag = &borg_grids[y][x];

                /* skip our own spot */
                if (x == kill->x && y == kill->y) continue;

                /* track spaces already protected */
                if (borg_feature_protected(ag))
                {
                    /* track the safe areas for calculating danger */
                    spot_safe++;

                    /* Just in case */
                    if (spot_safe == 0) spot_safe = 1;
                    if (spot_safe == 8) spot_safe = 100;
                    if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
                }

            }
        }
        if (pfe || glyph || borg_create_door || borg_fighting_unique)
            p += 0;
        else
        {
            p += (r_ptr->spell_power) * 7;
            p = p / spot_safe;
        }
        /* reduce the fear if it is a unique */
        if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75 / 100;
        break;

        case RSF_S_ANIMAL:
        /* This is used to calculate the free squares next to us.
         * This is important when dealing with summoners.
         */
        for (spot_x = -1; spot_x <= 1; spot_x++)
        {
            for (spot_y = -1; spot_y <= 1; spot_y++)
            {
                /* Acquire location */
                x = spot_x + kill->x;
                y = spot_y + kill->y;

                ag = &borg_grids[y][x];

                /* skip our own spot */
                if (x == kill->x && y == kill->y) continue;

                /* track spaces already protected */
                if (borg_feature_protected(ag))
                {
                    /* track the safe areas for calculating danger */
                    spot_safe++;

                    /* Just in case */
                    if (spot_safe == 0) spot_safe = 1;
                    if (spot_safe == 8) spot_safe = 100;
                    if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
                }

            }
        }
        if (pfe || glyph || borg_create_door || borg_fighting_unique)
            p += 0;
        else
        {
            p += (r_ptr->spell_power) * 5;
            p = p / spot_safe;
        }
        /* reduce the fear if it is a unique */
        if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75 / 100;
        break;

        case RSF_S_SPIDER:
        /* This is used to calculate the free squares next to us.
         * This is important when dealing with summoners.
         */
        for (spot_x = -1; spot_x <= 1; spot_x++)
        {
            for (spot_y = -1; spot_y <= 1; spot_y++)
            {
                /* Acquire location */
                x = spot_x + kill->x;
                y = spot_y + kill->y;

                ag = &borg_grids[y][x];

                /* skip our own spot */
                if (x == kill->x && y == kill->y) continue;

                /* track spaces already protected */
                if (borg_feature_protected(ag))
                {
                    /* track the safe areas for calculating danger */
                    spot_safe++;

                    /* Just in case */
                    if (spot_safe == 0) spot_safe = 1;
                    if (spot_safe == 8) spot_safe = 100;
                    if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
                }

            }
        }
        if (pfe || glyph || borg_create_door || borg_fighting_unique)
            p += 0;
        else
        {
            p += (r_ptr->spell_power) * 5;
            p = p / spot_safe;
        }
        /* reduce the fear if it is a unique */
        if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75 / 100;
        break;

        case RSF_S_HOUND:
        /* This is used to calculate the free squares next to us.
         * This is important when dealing with summoners.
         */
        for (spot_x = -1; spot_x <= 1; spot_x++)
        {
            for (spot_y = -1; spot_y <= 1; spot_y++)
            {
                /* Acquire location */
                x = spot_x + kill->x;
                y = spot_y + kill->y;

                ag = &borg_grids[y][x];

                /* skip our own spot */
                if (x == kill->x && y == kill->y) continue;

                /* track spaces already protected */
                if (borg_feature_protected(ag))
                {
                    /* track the safe areas for calculating danger */
                    spot_safe++;

                    /* Just in case */
                    if (spot_safe == 0) spot_safe = 1;
                    if (spot_safe == 8) spot_safe = 100;
                    if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
                }

            }
        }
        if (pfe || glyph || borg_create_door || borg_fighting_unique)
            p += 0;
        else
        {
            p += (r_ptr->spell_power) * 5;
            p = p / spot_safe;
        }
        /* reduce the fear if it is a unique */
        if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75 / 100;
        break;

        case RSF_S_HYDRA:
        /* This is used to calculate the free squares next to us.
         * This is important when dealing with summoners.
         */
        for (spot_x = -1; spot_x <= 1; spot_x++)
        {
            for (spot_y = -1; spot_y <= 1; spot_y++)
            {
                /* Acquire location */
                x = spot_x + kill->x;
                y = spot_y + kill->y;

                ag = &borg_grids[y][x];

                /* skip our own spot */
                if (x == kill->x && y == kill->y) continue;

                /* track spaces already protected */
                if (borg_feature_protected(ag))
                {
                    /* track the safe areas for calculating danger */
                    spot_safe++;

                    /* Just in case */
                    if (spot_safe == 0) spot_safe = 1;
                    if (spot_safe == 8) spot_safe = 100;
                    if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
                }

            }
        }
        if (pfe)
        {
            p += (r_ptr->spell_power);
            p = p / spot_safe;
        }
        else if (glyph || borg_create_door || borg_fighting_unique)
        {
            p += (r_ptr->spell_power) * 2;
            p = p / spot_safe;
        }
        else
        {
            p += (r_ptr->spell_power) * 5;
            p = p / spot_safe;
        }
        /* reduce the fear if it is a unique */
        if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75 / 100;
        break;

        case RSF_S_AINU:
        /* This is used to calculate the free squares next to us.
         * This is important when dealing with summoners.
         */
        for (spot_x = -1; spot_x <= 1; spot_x++)
        {
            for (spot_y = -1; spot_y <= 1; spot_y++)
            {
                /* Acquire location */
                x = spot_x + kill->x;
                y = spot_y + kill->y;

                ag = &borg_grids[y][x];

                /* skip our own spot */
                if (x == kill->x && y == kill->y) continue;

                /* track spaces already protected */
                if (borg_feature_protected(ag))
                {
                    /* track the safe areas for calculating danger */
                    spot_safe++;

                    /* Just in case */
                    if (spot_safe == 0) spot_safe = 1;
                    if (spot_safe == 8) spot_safe = 100;
                    if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
                }

            }
        }
        if (pfe || borg_fighting_unique)
        {
            p += (r_ptr->spell_power);
            p = p / spot_safe;
        }
        else if (glyph || borg_create_door || borg_fighting_unique)
        {
            p += (r_ptr->spell_power) * 3;
            p = p / spot_safe;
        }
        else
        {
            p += (r_ptr->spell_power) * 7;
            p = p / spot_safe;
        }
        /* reduce the fear if it is a unique */
        if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75 / 100;
        break;

        case RSF_S_DEMON:
        /* This is used to calculate the free squares next to us.
         * This is important when dealing with summoners.
         */
        for (spot_x = -1; spot_x <= 1; spot_x++)
        {
            for (spot_y = -1; spot_y <= 1; spot_y++)
            {
                /* Acquire location */
                x = spot_x + kill->x;
                y = spot_y + kill->y;

                ag = &borg_grids[y][x];

                /* skip our own spot */
                if (x == kill->x && y == kill->y) continue;

                /* track spaces already protected */
                if (borg_feature_protected(ag))
                {
                    /* track the safe areas for calculating danger */
                    spot_safe++;

                    /* Just in case */
                    if (spot_safe == 0) spot_safe = 1;
                    if (spot_safe == 8) spot_safe = 100;
                    if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
                }

            }
        }
        if (pfe)
        {
            p += (r_ptr->spell_power);
            p = p / spot_safe;
        }
        else if (glyph || borg_create_door || borg_fighting_unique)
        {
            p += (r_ptr->spell_power) * 3;
            p = p / spot_safe;
        }
        else
        {
            p += (r_ptr->spell_power) * 7;
            p = p / spot_safe;
        }
        /* reduce the fear if it is a unique */
        if (rf_has(r_info->flags, RF_UNIQUE)) p = (p * 75) / 100;
        break;

        case RSF_S_UNDEAD:
        /* This is used to calculate the free squares next to us.
         * This is important when dealing with summoners.
         */
        for (spot_x = -1; spot_x <= 1; spot_x++)
        {
            for (spot_y = -1; spot_y <= 1; spot_y++)
            {
                /* Acquire location */
                x = spot_x + kill->x;
                y = spot_y + kill->y;

                ag = &borg_grids[y][x];

                /* skip our own spot */
                if (x == kill->x && y == kill->y) continue;

                /* track spaces already protected */
                if (borg_feature_protected(ag))
                {
                    /* track the safe areas for calculating danger */
                    spot_safe++;

                    /* Just in case */
                    if (spot_safe == 0) spot_safe = 1;
                    if (spot_safe == 8) spot_safe = 100;
                    if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
                }

            }
        }
        if (pfe)
        {
            p += (r_ptr->spell_power);
            p = p / spot_safe;
        }
        else if (glyph || borg_create_door || borg_fighting_unique)
        {
            p += (r_ptr->spell_power) * 3;
            p = p / spot_safe;
        }
        else
        {
            p += (r_ptr->spell_power) * 7;
            p = p / spot_safe;
        }
        /* reduce the fear if it is a unique */
        if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75 / 100;
        break;

        case RSF_S_DRAGON:
        /* This is used to calculate the free squares next to us.
         * This is important when dealing with summoners.
         */
        for (spot_x = -1; spot_x <= 1; spot_x++)
        {
            for (spot_y = -1; spot_y <= 1; spot_y++)
            {
                /* Acquire location */
                x = spot_x + kill->x;
                y = spot_y + kill->y;

                ag = &borg_grids[y][x];

                /* skip our own spot */
                if (x == kill->x && y == kill->y) continue;

                /* track spaces already protected */
                if (borg_feature_protected(ag))
                {
                    /* track the safe areas for calculating danger */
                    spot_safe++;

                    /* Just in case */
                    if (spot_safe == 0) spot_safe = 1;
                    if (spot_safe == 8) spot_safe = 100;
                    if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
                }

            }
        }
        if (pfe)
        {
            p += (r_ptr->spell_power);
            p = p / spot_safe;
        }
        else if (glyph || borg_create_door || borg_fighting_unique)
        {
            p += (r_ptr->spell_power) * 3;
            p = p / spot_safe;
        }
        else
        {
            p += (r_ptr->spell_power) * 7;
            p = p / spot_safe;
        }
        /* reduce the fear if it is a unique */
        if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75 / 100;
        break;

        case RSF_S_HI_UNDEAD:
        /* This is used to calculate the free squares next to us.
         * This is important when dealing with summoners.
         */
        for (spot_x = -1; spot_x <= 1; spot_x++)
        {
            for (spot_y = -1; spot_y <= 1; spot_y++)
            {
                /* Acquire location */
                x = spot_x + kill->x;
                y = spot_y + kill->y;

                ag = &borg_grids[y][x];

                /* skip our own spot */
                if (x == kill->x && y == kill->y) continue;

                /* track spaces already protected */
                if (borg_feature_protected(ag))
                {
                    /* track the safe areas for calculating danger */
                    spot_safe++;

                    /* Just in case */
                    if (spot_safe == 0) spot_safe = 1;
                    if (spot_safe == 8) spot_safe = 100;
                    if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
                }

            }
        }
        if (pfe)
        {
            p += (r_ptr->spell_power);
            p = p / spot_safe;
        }
        else if (glyph || borg_create_door || borg_fighting_unique)
        {
            p += (r_ptr->spell_power) * 6;
            p = p / spot_safe;
        }
        else
        {
            p += (r_ptr->spell_power) * 12;
            p = p / spot_safe;
        }
        /* reduce the fear if it is a unique */
        if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75 / 100;
        break;

        case RSF_S_HI_DRAGON:
        /* This is used to calculate the free squares next to us.
         * This is important when dealing with summoners.
         */
        for (spot_x = -1; spot_x <= 1; spot_x++)
        {
            for (spot_y = -1; spot_y <= 1; spot_y++)
            {
                /* Acquire location */
                x = spot_x + kill->x;
                y = spot_y + kill->y;

                ag = &borg_grids[y][x];

                /* skip our own spot */
                if (x == kill->x && y == kill->y) continue;

                /* track spaces already protected */
                if (borg_feature_protected(ag))
                {
                    /* track the safe areas for calculating danger */
                    spot_safe++;

                    /* Just in case */
                    if (spot_safe == 0) spot_safe = 1;
                    if (spot_safe == 8) spot_safe = 100;
                    if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
                }

            }
        }
        if (pfe)
        {
            p = p / spot_safe;
        }
        else if (glyph || borg_create_door || borg_fighting_unique)
        {
            p += (r_ptr->spell_power) * 6;
            p = p / spot_safe;
        }
        else
        {
            p += (r_ptr->spell_power) * 12;
            p = p / spot_safe;
        }
        /* reduce the fear if it is a unique */
        if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75 / 100;
        break;

        case RSF_S_WRAITH:
        /* This is used to calculate the free squares next to us.
         * This is important when dealing with summoners.
         */
        for (spot_x = -1; spot_x <= 1; spot_x++)
        {
            for (spot_y = -1; spot_y <= 1; spot_y++)
            {
                /* Acquire location */
                x = spot_x + kill->x;
                y = spot_y + kill->y;

                ag = &borg_grids[y][x];

                /* skip our own spot */
                if (x == kill->x && y == kill->y) continue;

                /* track spaces already protected */
                if (borg_feature_protected(ag))
                {
                    /* track the safe areas for calculating danger */
                    spot_safe++;

                    /* Just in case */
                    if (spot_safe == 0) spot_safe = 1;
                    if (spot_safe == 8) spot_safe = 100;
                    if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
                }

            }
        }
        if (pfe)
        {
            p += (r_ptr->spell_power);
            p = p / spot_safe;
        }
        else if (glyph || borg_create_door || borg_fighting_unique)
        {
            p += (r_ptr->spell_power) * 6;
            p = p / spot_safe;
        }
        else
        {
            p += (r_ptr->spell_power) * 12;
            p = p / spot_safe;
        }
        /* reduce the fear if it is a unique */
        if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75 / 100;
        break;

        case RSF_S_UNIQUE:
        /* This is used to calculate the free squares next to us.
         * This is important when dealing with summoners.
         */
        for (spot_x = -1; spot_x <= 1; spot_x++)
        {
            for (spot_y = -1; spot_y <= 1; spot_y++)
            {
                /* Acquire location */
                x = spot_x + kill->x;
                y = spot_y + kill->y;

                ag = &borg_grids[y][x];

                /* skip our own spot */
                if (x == kill->x && y == kill->y) continue;

                /* track spaces already protected */
                if (borg_feature_protected(ag))
                {
                    /* track the safe areas for calculating danger */
                    spot_safe++;

                    /* Just in case */
                    if (spot_safe == 0) spot_safe = 1;
                    if (spot_safe == 8) spot_safe = 100;
                    if (borg_morgoth_position || borg_as_position) spot_safe = 1000;
                }

            }
        }
        if (pfe)
        {
            p += (r_ptr->spell_power);
            p = p / spot_safe;
        }
        else if (glyph || borg_create_door)
        {
            p += (r_ptr->spell_power) * 3;    /* slightly reduced danger for unique */
            p = p / spot_safe;
        }
        else
        {
            p += (r_ptr->spell_power) * 6;
            p = p / spot_safe;
        }
        /* reduce the fear if it is a unique */
        if (rf_has(r_info->flags, RF_UNIQUE)) p = p * 75 / 100;
        break;
        }

        /* A bolt spell cannot jump monsters to hit the borg. */
        if (bolt == true && !borg_projectable_pure(kill->y, kill->x, c_y, c_x)) z = 0;

        /* Some borgs are concerned with the 'effects' of an attack.  ie, cold attacks shatter potions,
         * fire attacks burn scrolls, electric attacks zap rings.
         */
        if (borg_skill[BI_MAXDEPTH] >= 75) p = 0;

        /* Notice damage */
        p += z;

        /* Track the most dangerous spell */
        if (p > n) n = p;

        /* Track the damage of all the spells, used in averaging */
        total_dam += p;
    }

    /* reduce by damage reduction */
    total_dam -= borg_skill[BI_DAM_RED];
    if (total_dam < 0) total_dam = 0;

    /* Slightly decrease the danger if the borg is sitting in
     * a sea of runes.
     */
    if (borg_morgoth_position || borg_as_position) total_dam = total_dam * 7 / 10;

    /* Average damage of all the spells & compare to most dangerous spell */
    av = total_dam / kill->ranged_attack;

    /* If the most dangerous spell is alot bigger than the average,
     * then return the dangerous one.
     *
     * There is a problem when dealing with defence manuevers.
     * If the borg is considering casting a spell like
     * Resistance and the monster also has a non
     * resistable attack (like Disenchant) then the damage
     * returned will be for that spell, since the danger of the
     * others (like fire, cold) will be greatly reduced by the
     * proposed defence spell.  The result will be the borg will
     * not cast the resistance spell eventhough it may be a very
     * good idea.
     *
     * Example: a monster has three breath attacks (Fire, Ice,
     * Disenchant) and each hits for 800 pts of damage.  The
     * borg currently resists all three, so the danger would be
     * 500. If the borg were to use a Res Heat Potion that would
     * decrease the danger to:
     * Fire:  333
     * Ice:   500
     * Disen: 500
     * Now the Average is 444.  Not really worth it, nominal change.
     * But if the resistance spell was both Fire and Ice, then
     * it would be:
     * Fire:  333
     * Ice:   333
     * Disen: 500
     * With an average of 388. Probably worth it, but the borg
     * would see that the Disen attack is a quite dangerous and
     * would return the result of 500.
     *
     * To fix this, the flag 'average' is added to the
     * borg_danger() to skip this check and return the average
     * damage.  If the flag is false then the formula below is
     * SKIPPED and the value returned with be the average.
     * If the flag is true, then the formula below will be used
     * to determine the returned value.  Currently the elemental
     * resistance spells and PFE have the flag set as false.
     *
     */
    if (!average) return (av);
    if (n >= av * 15 / 10 || n > borg_skill[BI_CURHP] * 8 / 10) return (n);
    else
        /* Average Danger */
        return (av);
}


/*
 * Calculate the danger to a grid from a monster  XXX XXX XXX
 *
 * Note that we are paranoid, especially about "monster speed",
 * since even if a monster is slower than us, it will occasionally
 * get one full turn to attack us.
 *
 * Note that we assume that monsters can walk through walls and
 * other monsters to get to the player.  XXX XXX XXX
 *
 * This function attempts to consider possibilities such as movement plus
 * spell attacks, physical attacks and spell attacks together,
 * and other similar situations.
 *
 * Currently we assume that "sleeping" monsters are less dangerous
 * unless you get near them, which may wake them up.
 *
 * We attempt to take into account things like monsters which sometimes
 * "stumble", and monsters which only "sometimes" use powerful spells.
 */
int borg_danger_aux(int y, int x, int c, int i, bool average, bool full_damage)
{
    borg_kill* kill = &borg_kills[i];

    struct monster_race* r_ptr = &r_info[kill->r_idx];

    int x9 = kill->x;
    int y9 = kill->y;
    int y_temp, x_temp;

    int ax, ay, d;

    int q = 0, r, p, v1 = 0, v2 = 0, b_v2 = 0, b_v1 = 0;

    int fake_speed = borg_skill[BI_SPEED];
    int monster_speed = kill->speed;
    int t, e;

    int ii;
    int chance;

    /* Paranoia */
    if (!kill->r_idx) return (0);

    /* Skip certain monster indexes.
     * These have been listed mainly in Teleport Other
     * checks in borg6.c in the defence maneuvers.
     */
    if (borg_tp_other_n)
    {
        for (ii = 1; ii <= borg_tp_other_n; ii++)
        {
            /* Is the current danger check same as a saved monster index? */
            if (i == borg_tp_other_index[ii])
            {
                return (0);
            }
        }
    }


    /* Distance components */
    ax = (x9 > x) ? (x9 - x) : (x - x9);
    ay = (y9 > y) ? (y9 - y) : (y - y9);

    /* Distance */
    d = MAX(ax, ay);

    /* Minimal distance */
    if (d < 1) d = 1;

    /* Minimal distance */
    if (d > 20) return (0);

    /* A very speedy borg will miscalculate danger of some monsters */
    if (borg_skill[BI_SPEED] >= 135) fake_speed = (borg_fighting_unique ? 120 : 125);

    /* Consider the character haste and slow monster spells */
    if (borg_speed)
        fake_speed += 10;
    if (borg_slow_spell)
        monster_speed -= 10;

    /* Assume monsters are a little fast when you are low level */
    if (borg_skill[BI_MAXHP] < 20 && borg_skill[BI_CDEPTH])
        monster_speed += 3;


    /* Player energy per game turn  */
    e = extract_energy[(fake_speed)];

    /* Game turns per player move  */
    t = (100 + (e - 1)) / e;

    /*  Monster energy per game turn  */
    e = extract_energy[monster_speed];

    /* Monster moves */
    q = c * ((t * e) / 10);

    /* allow partial hits when not caculating full possible damage */
    if (full_damage)
        q = (int)((q + 9) / 10) * 10;

    /* Minimal energy.  Monsters get at least some energy.
     * If the borg is very fast relative to a monster, then the
     * monster danger is artifically low due to the way the borg
     * will calculate the danger and energy.  So the monsters must
     * be given some base energy to equate the borg's.
     * ie:  the borg with speed +40 (speed = 150) is attacking
     * a monster with normal speed (speed = 110).  One would
     * think that the borg gets 4 attacks per turn over the monster.
     * and this does happen.  What if the monster can deal out
     * 1000 damage pts per monster attack turn?  The borg will
     * reduce the danger to 250 because the borg is 4x faster
     * than the monster.  But eventually the borg will get hit
     * by that 1000 pt damage attack.  And when it does, its
     * going to hurt.
     * So we make sure the monster is at least as fast as us.
     * But the monster is allowed to be faster than us.
     */
    if (q <= 10) q = 10;

    /** Danger from physical attacks **/

    /* Physical attacks */
    v1 = borg_danger_aux1(i, full_damage);

    /* Hack -- Under Stressful Situation.
     */
    if (time_this_panel > 1200 || borg_t > 25000)
    {
        /* he might be stuck and could overflow */
        v1 = v1 / 5;
    }

    /* No attacks for some monsters */
    if (rf_has(r_ptr->flags, RF_NEVER_BLOW))
    {
        v1 = 0;
    }

    /* No movement for some monsters */
    if ((rf_has(r_ptr->flags, RF_NEVER_MOVE)) && (d > 1))
    {
        v1 = 0;
    }

    /* multipliers yeild some trouble when I am weak */
    if ((rf_has(r_ptr->flags, RF_MULTIPLY)) && (borg_skill[BI_CLEVEL] < 20))
    {   /* extra 50% */
        v1 = v1 + (v1 * 15 / 10);
    }

    /* Friends yeild some trouble when I am weak */
    if ((r_ptr->friends || r_ptr->friends_base) &&
        (borg_skill[BI_CLEVEL] < 20))
    {
        if (borg_skill[BI_CLEVEL] < 15)
        {
            /* extra 80% */
            v1 = v1 + (v1 * 18 / 10);
        }
        else
        {
            /* extra 30% */
            v1 = v1 + (v1 * 13 / 10);
        }

    }

    /* Reduce danger from sleeping monsters */
    if (!kill->awake)
    {
        int inc = r_ptr->sleep + 5;
        /* Reduce the fear if Borg is higher level */
        if (borg_skill[BI_CLEVEL] >= 25)
        {
            v1 = v1 / 2;
        }

        /* Tweak danger based on the "alertness" of the monster */
        /* increase the danger for light sleepers */
        v1 = v1 + (v1 * inc / 100);
    }
    /* Reduce danger from sleeping monsters with the sleep 2 spell*/
    if (borg_sleep_spell_ii)
    {
        if ((d == 1) &&
            (kill->awake) &&
            (!(rf_has(r_ptr->flags, RF_NO_SLEEP))) &&
            (!(rf_has(r_ptr->flags, RF_UNIQUE))) &&
            (kill->level <= (borg_skill[BI_CLEVEL] - 15)))
        {
            /* Under special circumstances force the damage to 0 */
            if (borg_skill[BI_CLEVEL] < 20 &&
                borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2)
            {
                v1 = 0;
            }
            else
            {
                v1 = v1 / 3;
            }
        }
    }
    /* Reduce danger from sleeping monsters with the sleep 1,3 spell*/
    if (borg_sleep_spell)
    {
        if (kill->awake &&
            (!(rf_has(r_ptr->flags, RF_NO_SLEEP))) &&
            (!(rf_has(r_ptr->flags, RF_UNIQUE))) &&
            (kill->level <= (borg_skill[BI_CLEVEL] - 15)))
        {
            /* Under special circumstances force the damage to 0 */
            if (borg_skill[BI_CLEVEL] < 20 &&
                borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2)
            {
                v1 = 0;
            }
            else
            {
                v1 = v1 / (d + 2);
            }
        }
    }
    if (borg_crush_spell)
    {
        /* HACK for now, either it dies or it doesn't.  */
        /* If we discover it isn't using this spell much, we can modify */
        if ((kill->power * kill->injury) / 100 < borg_skill[BI_CLEVEL] * 4)
            v1 = 0;
    }

    /* Reduce danger from confused monsters */
    if (kill->confused)
    {
        v1 = v1 / 2;
    }
    if (kill->stunned)
    {
        v1 = v1 * 10 / 13;
    }
    if (borg_confuse_spell)
    {
        if (kill->awake &&
            !kill->confused &&
            (!(rf_has(r_ptr->flags, RF_NO_SLEEP))) &&
            (!(rf_has(r_ptr->flags, RF_UNIQUE))) &&
            (kill->level <= (borg_skill[BI_CLEVEL] - 15)))
        {
            /* Under special circumstances force the damage to 0 */
            if (borg_skill[BI_CLEVEL] < 20 &&
                borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2)
            {
                v1 = 0;
            }
            else
            {
                v1 = v1 / (d + 2);
            }
        }
    }
    /* Perceive a reduce danger from scared monsters */
    if (borg_fear_mon_spell)
    {
        v1 = 0;
    }

    /* Hack -- Physical attacks require proximity
     *
     * Note that we do try to consider a fast monster moving and attacking
     * in the same round.  We should consider monsters that have a speed 2 or 3 classes
     * higher than ours, but most times, the borg will only encounter monsters with a single
     * catagory higher speed.
     */
    if (q > 10 && d != 1 && !(rf_has(r_ptr->flags, RF_NEVER_MOVE)))
    {
        b_v1 = 0;

        /* Check for a single grid movement, simulating the monster's move action. */
        for (ii = 0; ii < 8; ii++)
        {
            /* Obtain a grid to which the monster might move */
            y_temp = y9 + ddy_ddd[ii];
            x_temp = x9 + ddx_ddd[ii];

            /* Check for legality */
            if (!square_in_bounds_fully(cave, loc(x_temp, y_temp))) continue;

            /* Cannot occupy another monster's grid */
            if (borg_grids[y_temp][x_temp].kill) continue;

            /* Cannot occupy a closed door */
            if (borg_grids[y_temp][x_temp].feat == FEAT_CLOSED) continue;

            /* Cannot occupy a perma-wall */
            if (borg_grids[y_temp][x_temp].feat == FEAT_PERM) continue;

            /* Cannot occupy a wall/seam grid (unless pass_wall or kill_wall) */
            if (borg_grids[y_temp][x_temp].feat == FEAT_GRANITE ||
                (borg_grids[y_temp][x_temp].feat == FEAT_MAGMA ||
                    borg_grids[y_temp][x_temp].feat == FEAT_QUARTZ ||
                    borg_grids[y_temp][x_temp].feat == FEAT_MAGMA_K ||
                    borg_grids[y_temp][x_temp].feat == FEAT_QUARTZ_K ||
                    borg_grids[y_temp][x_temp].feat == FEAT_RUBBLE))
            {
                /* legally on a wall of some sort, check for closeness*/
                if (rf_has(r_ptr->flags, RF_PASS_WALL))
                {
                    if (borg_distance(y_temp, x_temp, y, x) == 1) b_v1 = v1;
                }
                if (rf_has(r_ptr->flags, RF_KILL_WALL))
                {
                    if (borg_distance(y_temp, x_temp, y, x) == 1) b_v1 = v1;
                }
            }

            /* Is this grid being considered adjacent to the grid for which the borg_danger() was called? */
            if (borg_distance(y_temp, x_temp, y, x) > 1) continue;

            /* A legal floor grid */
            if (borg_cave_floor_bold(y_temp, x_temp))
            {
                /* Really fast monster can hit me more than once after it's move */
                b_v1 = v1 * (q / (d * 10));
            }
        }

        /* Monster is not able to move and threaten me in the same round */
        v1 = b_v1;
    }

    /* Consider a monster that is fast and can strike more than once per round */
    if (q > 10 && d == 1)
    {
        v1 = v1 * q / 10;
    }

    /* Need to be close if you are normal speed */
    if (q == 10 && d > 1)
    {
        v1 = 0;
    }

    /** Ranged Attacks **/
    v2 = borg_danger_aux2(i, y, x, d, average, full_damage);

    /* Never cast spells */
    if (!r_ptr->freq_innate && !r_ptr->freq_spell)
    {
        v2 = 0;
    }

    /* Hack -- verify distance */
    if (borg_distance(y9, x9, y, x) > z_info->max_range)
    {
        v2 = 0;
    }

    /* Hack -- verify line of sight (both ways) for monsters who can only move 1 grid. */
    if (q <= 10 && !borg_projectable(y9, x9, y, x) && !borg_projectable(y, x, y9, x9))
    {
        v2 = 0;
    }

    /* Hack -- verify line of sight (both ways) for monsters who can only move > 1 grid.
     * Some fast monsters can take a move action and range attack in the same round.
     * Basically, we see how many grids the monster can move and check LOS from each of
     * those grids to determine the relative danger.  We need to make sure that the monster
     * is not passing through walls unless he has that ability.
         * Consider a fast monster who can move and cast a spell in the same round.
         * This is important for a borg looking for a safe grid from a ranged attacker.
         * If the attacker is fast then he might be able to move to a grid which does have LOS
         * to the grid the borg is considering.
         *
         * ##############
         * #.....#.#.1#D#   Grids marked 2 are threatened by the D currently.
         * #####.#..##@##	Grids marked 1 are safe currently, but the fast D will be able
         * #####.#..1221#	to move to the borg's grid after he moves and the D will be able
         * ##############	to use a ranged attack to grids 1, all in the same round.
         *					The borg should not consider grid 1 as safe.
    */
    if (q >= 20)
    {
        int b_q = q;
        b_v2 = 0;

        /* Maximal speed check */
        if (q > 20) q = 20;

        /* Check for a single grid movement, simulating the monster's move action. */
        for (ii = 0; ii < 8; ii++)
        {
            /* Obtain a grid to which the monster might move */
            y_temp = y9 + ddy_ddd[ii];
            x_temp = x9 + ddx_ddd[ii];

            /* Check for legality */
            if (!square_in_bounds_fully(cave, loc(x_temp, y_temp))) continue;

            /* Cannot occupy another monster's grid */
            if (borg_grids[y_temp][x_temp].kill) continue;

            /* Cannot occupy a closed door */
            if (borg_grids[y_temp][x_temp].feat == FEAT_CLOSED) continue;

            /* Cannot occupy a perma-wall */
            if (borg_grids[y_temp][x_temp].feat == FEAT_PERM) continue;

            /* Cannot occupy a wall/seam grid (unless pass_wall or kill_wall) */
            if (borg_grids[y_temp][x_temp].feat >= FEAT_GRANITE ||
                (borg_grids[y_temp][x_temp].feat == FEAT_MAGMA ||
                    borg_grids[y_temp][x_temp].feat == FEAT_QUARTZ ||
                    borg_grids[y_temp][x_temp].feat == FEAT_MAGMA_K ||
                    borg_grids[y_temp][x_temp].feat == FEAT_QUARTZ_K ||
                    borg_grids[y_temp][x_temp].feat == FEAT_RUBBLE))
            {
                /* legally on a wall of some sort, check for LOS*/
                if (rf_has(r_ptr->flags, RF_PASS_WALL))
                {
                    if (borg_projectable(y_temp, x_temp, y, x)) b_v2 = v2 * b_q / 10;
                }
                if (rf_has(r_ptr->flags, RF_KILL_WALL))
                {
                    if (borg_projectable(y_temp, x_temp, y, x)) b_v2 = v2 * b_q / 10;
                }
            }

            /* Monster on a legal floor grid.  Can he see me? */
            else if (borg_projectable(y_temp, x_temp, y, x)) b_v2 = v2 * b_q / 10;
        }

        /* Monster is not able to move and threaten me in the same round */
        v2 = b_v2;
    }

    /* Hack -- Under Stressful Situation.
     */
    if (time_this_panel > 1200 || borg_t > 25000)
    {
        /* he might be stuck and could overflow */
        v2 = v2 / 5;
    }

    /* multipliers yeild some trouble when I am weak */
    if ((rf_has(r_ptr->flags, RF_MULTIPLY)) && (borg_skill[BI_CLEVEL] < 20))
    {
        v2 = v2 + (v2 * 12 / 10);
    }

    /* Friends yeild some trouble when I am weak */
    if ((r_ptr->friends || r_ptr->friends_base) &&
        (borg_skill[BI_CLEVEL] < 20))
    {
        v2 = v2 + (v2 * 12 / 10);
    }

    /* Reduce danger from sleeping monsters */
    if (!kill->awake)
    {
        int inc = r_ptr->sleep + 5;
        /* weaklings and should still fear */
        if (borg_skill[BI_CLEVEL] >= 25)
        {
            v2 = v2 / 2;
        }

        /* Tweak danger based on the "alertness" of the monster */
        /* increase the danger for light sleepers */
        v2 = v2 + (v2 * inc / 100);
    }

    /* Reduce danger from sleeping monsters with the sleep 2 spell*/
    if (borg_sleep_spell_ii)
    {

        if ((d == 1) &&
            (kill->awake) &&
            (!(rf_has(r_ptr->flags, RF_NO_SLEEP))) &&
            (!(rf_has(r_ptr->flags, RF_UNIQUE))) &&
            (kill->level <= ((borg_skill[BI_CLEVEL] < 15) ? borg_skill[BI_CLEVEL] : (((borg_skill[BI_CLEVEL] - 10) / 4) * 3) + 10)))
        {
            v2 = v2 / 3;
        }
    }

    if (borg_crush_spell)
    {
        /* HACK for now, either it dies or it doesn't.  */
        /* If we discover it isn't using this spell much, we can modify */
        if ((kill->power * kill->injury) / 100 < borg_skill[BI_CLEVEL] * 4)
            v2 = 0;
    }

    /* Reduce danger from sleeping monsters with the sleep 1,3 spell*/
    if (borg_sleep_spell)
    {
        v2 = v2 / (d + 2);
    }
    /* Reduce danger from confused monsters */
    if (kill->confused)
    {
        v2 = v2 / 2;
    }
    /* Reduce danger from stunnned monsters  */
    if (kill->stunned)
    {
        v2 = v2 * 10 / 13;
    }
    if (borg_confuse_spell)
    {
        v2 = v2 / 6;
    }

#if 0 /* They still cast spells, they are still dangerous */
    /* Reduce danger from scared monsters */
    if (borg_fear_mon_spell)
    {
        v2 = v2 * 8 / 10;
    }
    if (kill->afraid)
    {
        v2 = v2 * 8 / 10;
    }
#endif
    if (!full_damage)
    {
        /* reduce for frequency. */
        chance = (r_ptr->freq_innate + r_ptr->freq_spell) / 2;
        if (chance < 11)
            v2 = ((v2 * 4) / 10);
        else
            if (chance < 26)
                v2 = ((v2 * 6) / 10);
            else
                if (chance < 51)
                    v2 = ((v2 * 8) / 10);
    }

    /* Danger */
    if (v2)
    {
        /* Full power */
        r = q;

        /* Total danger */
        v2 = v2 * r / 10;
    }

    /* Maximal danger */
    p = MAX(v1, v2);
    if (p > 2000) p = 2000;

    /* Result */
    return (p);
}


/*
 * Hack -- Calculate the "danger" of the given grid.
 *
 * Currently based on the physical power of nearby monsters, as well
 * as the spell power of monsters which can target the given grid.
 *
 * This function is extremely expensive, mostly due to the number of
 * times it is called, and also to the fact that it calls its helper
 * functions about thirty times each per call.
 *
 * We need to do more intelligent processing with the "c" parameter,
 * since currently the Borg does not realize that backing into a
 * hallway is a good idea, since as far as he can tell, many of
 * the nearby monsters can "squeeze" into a single grid.
 *
 * Note that we also take account of the danger of the "region" in
 * which the grid is located, which allows us to apply some "fear"
 * of invisible monsters and things of that nature.
 *
 * Generally bool Average is true.
 */
int borg_danger(int y, int x, int c, bool average, bool full_damage)
{
    int i, p = 0;

    struct loc l = loc(x, y);
    if (!square_in_bounds(cave, l))
        return 2000;

    /* Base danger (from regional fear) but not within a vault.  Cheating the floor grid */
    if (!square_isvault(cave, l) && borg_skill[BI_CDEPTH] <= 80)
    {
        p += borg_fear_region[y / 11][x / 11] * c;
    }

    /* Reduce regional fear on Depth 100 */
    if (borg_skill[BI_CDEPTH] == 100 && p >= 300) p = 300;

    /* Added danger (from a lot of monsters).
     * But do not add it if we have been sitting on
     * this panel for too long, or monster's in a vault.  The fear_monsters[][]
     * can induce some bouncy behavior.
     */
    if (time_this_panel <= 200 && !square_isvault(cave, loc(x, y)))
        p += borg_fear_monsters[y][x] * c;

    full_damage = true;

    /* Examine all the monsters */
    for (i = 1; i < borg_kills_nxt; i++)
    {
        borg_kill* kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Collect danger from monster */
        p += borg_danger_aux(y, x, c, i, average, full_damage);
    }

    /* Return the danger */
    return (p > 2000 ? 2000 : p);
}




/*
 * Determine if the Borg is out of "crucial" supplies.
 *
 * Note that we ignore "restock" issues for the first several turns
 * on each level, to prevent repeated "level bouncing".
 */
const char* borg_restock(int depth)
{

    /* We are now looking at our preparedness */
    if (-1 == borg_ready_morgoth)
        borg_ready_morgoth = 0;

    /* Always ready for the town */
    if (!depth) return ((char*)NULL);

    /* Always Ready to leave town */
    if (borg_skill[BI_CDEPTH] == 0) return ((char*)NULL);

    /* Always spend time on a level unless 100*/
    if (borg_t - borg_began < 100 && borg_skill[BI_CDEPTH] != 100) return ((char*)NULL);


    /*** Level 1 ***/

    /* Must have some lite */
    if (borg_skill[BI_CURLITE] < 1) return ("rs my_CURLITE");

    /* Must have "fuel" */
    if (borg_skill[BI_AFUEL] < 1 && !borg_skill[BI_LIGHT]) return ("rs amt_fuel");

    /* Must have "food" */
    if (borg_skill[BI_FOOD] < 1) return ("rs amt_food");

    /* Assume happy at level 1 */
    if (depth <= 1) return ((char*)NULL);

    /*** Level 2 and 3 ***/

    /* Must have "fuel" */
    if (borg_skill[BI_AFUEL] < 2 && !borg_skill[BI_LIGHT]) return ("rs fuel+2");

    /* Must have "food" */
    if (borg_skill[BI_FOOD] < 3) return ("rs food+2");

    /* Must have "recall" */
    /* if (borg_skill[BI_RECALL] < 2) return ("rs recall"); */

    /* Assume happy at level 3 */
    if (depth <= 3) return ((char*)NULL);

    /*** Level 3 to 5 ***/

    if (depth <= 5) return ((char*)NULL);

    /*** Level 6 to 9 ***/

    /* Must have "phase" */
    if (borg_skill[BI_APHASE] < 1) return ("rs phase");

    /* Potions of Cure Wounds */
    if ((borg_skill[BI_MAXCLEVEL] < 30) && borg_skill[BI_ACLW] + borg_skill[BI_ACSW] + borg_skill[BI_ACCW] < 1) return ("rs clw/csw");

    /* Assume happy at level 9 */
    if (depth <= 9) return ((char*)NULL);


    /*** Level 10 - 19  ***/

    /* Must have good light */
    if (borg_skill[BI_CURLITE] < 2) return "2 Light";

    /* Must have "cure" */
    if ((borg_skill[BI_MAXCLEVEL] < 30) && borg_skill[BI_ACLW] + borg_skill[BI_ACSW] + borg_skill[BI_ACCW] < 2) return ("rs cure");

    /* Must have "teleport" */
    if (borg_skill[BI_ATELEPORT] + borg_skill[BI_AESCAPE] < 2) return ("rs tele&esc(1)");

    /* Assume happy at level 19 */
    if (depth <= 19) return ((char*)NULL);


    /*** Level 20 - 35  ***/

    /* Must have "cure" */
    if ((borg_skill[BI_MAXCLEVEL] < 30) && borg_skill[BI_ACSW] + borg_skill[BI_ACCW] < 4) return ("rs cure");

    /* Must have "teleport" or Staff */
    if (borg_skill[BI_ATELEPORT] + borg_skill[BI_AESCAPE] < 4) return ("rs tele&esc(4)");

    /* Assume happy at level 44 */
    if (depth <= 35) return ((char*)NULL);


    /*** Level 36 - 45  ***/

    /* Must have Scroll of Teleport (or good 2nd choice) */
    if (borg_skill[BI_ATELEPORT] + borg_skill[BI_ATELEPORTLVL] < 2) return ("rs teleport(1)");

    /* Assume happy at level 44 */
    if (depth <= 45) return ((char*)NULL);


    /*** Level 46 - 64  ***/

    /* Assume happy at level 65 */
    if (depth <= 64) return ((char*)NULL);

    /*** Level 65 - 99  ***/

    /* Must have "Heal" */
    if (borg_skill[BI_AHEAL] + borg_has[kv_rod_healing] + borg_skill[BI_AEZHEAL] < 1) return ("rs heal");

    /* Assume happy at level 99 */
    if (depth <= 99) return ((char*)NULL);

    /*** Level 100  ***/

    /* Must have "Heal" */
    /* If I just got to dlevel 100 and low on heals, get out now. */
    if (borg_t - borg_began < 10 && borg_skill[BI_AEZHEAL] < 15) return ("rs *heal*");

    /* Assume happy */
    return ((char*)NULL);
}


/*
 * Determine if the Borg meets the "minimum" requirements for a level
 */
static const char* borg_prepared_aux(int depth)
{
    if (-1 == borg_ready_morgoth)
        borg_ready_morgoth = 0;
    if (borg_skill[BI_KING])
    {
        borg_ready_morgoth = 1;
        return (NULL);
    }

    /* Always ready for the town */
    if (!depth) return (NULL);


    /*** Essential Items for Level 1 ***/

    /* Require lite (any) */
    if (borg_skill[BI_CURLITE] < 1) return ("1 Lite");

    /* Require food */
    if (borg_skill[BI_FOOD] < 5) return ("5 Food");

    /* Usually ready for level 1 */
    if (depth <= 1) return ((char*)NULL);


    /*** Essential Items for Level 2 ***/

    /* Require fuel */
    if (borg_skill[BI_AFUEL] < 5 && !borg_skill[BI_LIGHT]) return ("5 Fuel");

    /* Require recall */
    /* if (borg_skill[BI_RECALL] < 1) return ("1 recall"); */

    if (!borg_cfg[BORG_PLAYS_RISKY])
    {
        /* Require 30 hp */
        if (borg_skill[BI_MAXHP] < 30) return ("30 hp");
    }

    /* Usually ready for level 2 */
    if (depth <= 2) return ((char*)NULL);

    /*** Essential Items for Level 3 and 4 ***/

    if (!borg_cfg[BORG_PLAYS_RISKY])
    {
        /* class specific requirement */
        switch (borg_class)
        {
        case CLASS_WARRIOR:
        case CLASS_BLACKGUARD:
        if (borg_skill[BI_MAXHP] < 50) return ("50 hp");
        if (borg_skill[BI_MAXCLEVEL] < 4) return ("4 clevel");
        break;
        case CLASS_ROGUE:
        if (borg_skill[BI_MAXHP] < 50) return ("50 hp");
        if (borg_skill[BI_MAXCLEVEL] < 8) return ("8 clevel");
        break;
        case CLASS_PRIEST:
        case CLASS_DRUID:
        if (borg_skill[BI_MAXHP] < 40) return ("40 hp");
        if (borg_skill[BI_MAXCLEVEL] < 9) return ("9 level");
        break;
        case CLASS_PALADIN:
        if (borg_skill[BI_MAXHP] < 50) return ("50 hp");
        if (borg_skill[BI_MAXCLEVEL] < 4) return ("4 clevel");
        break;
        case CLASS_RANGER:
        if (borg_skill[BI_MAXHP] < 50) return ("50 hp");
        if (borg_skill[BI_MAXCLEVEL] < 4) return ("4 clevel");
        break;
        case CLASS_MAGE:
        case CLASS_NECROMANCER:
        if (borg_skill[BI_MAXHP] < 60) return ("60 hp");
        if (borg_skill[BI_MAXCLEVEL] < 11) return ("11 clevel");
        break;
        }
    }

    /* Potions of Cure Serious Wounds */
    if ((borg_skill[BI_MAXCLEVEL] < 30) && borg_skill[BI_ACLW] + borg_skill[BI_ACSW] + borg_skill[BI_ACCW] < 2) return ("2 cure");

    /* Usually ready for level 3 and 4 */
    if (depth <= 4) return ((char*)NULL);


    /*** Essential Items for Level 5 to 9 ***/

    if (!borg_cfg[BORG_PLAYS_RISKY])
    {
        /* class specific requirement */
        if (borg_skill[BI_CDEPTH])
        {
            switch (borg_class)
            {
            case CLASS_WARRIOR:
            case CLASS_BLACKGUARD:
            if (borg_skill[BI_MAXHP] < 60) return ("60 hp");
            if (borg_skill[BI_MAXCLEVEL] < 6) return ("6 clevel");
            break;
            case CLASS_ROGUE:
            if (borg_skill[BI_MAXHP] < 60) return ("60 hp");
            if (borg_skill[BI_MAXCLEVEL] < 10) return ("10 clevel");
            break;
            case CLASS_PRIEST:
            case CLASS_DRUID:
            if (borg_skill[BI_MAXHP] < 60) return ("60 hp");
            if (borg_skill[BI_MAXCLEVEL] < 15) return ("15 clevel");
            break;
            case CLASS_PALADIN:
            if (borg_skill[BI_MAXHP] < 60) return ("60 hp");
            if (borg_skill[BI_MAXCLEVEL] < 6) return ("6 clevel");
            break;
            case CLASS_RANGER:
            if (borg_skill[BI_MAXHP] < 60) return ("60 hp");
            if (borg_skill[BI_MAXCLEVEL] < 6) return ("6 clevel");
            break;
            case CLASS_MAGE:
            case CLASS_NECROMANCER:
            if (borg_skill[BI_MAXHP] < 80) return ("80 hp");
            if (borg_skill[BI_MAXCLEVEL] < 15) return ("15 level");
            break;
            }
        }
    }

    /* Potions of Cure Serious/Critical Wounds */
    if ((borg_skill[BI_MAXCLEVEL] < 30) && borg_skill[BI_ACLW] + borg_skill[BI_ACSW] + borg_skill[BI_ACCW] < 2) return ("2 cures");

    /* Scrolls of Word of Recall */
    if (borg_skill[BI_RECALL] < 1) return ("1 recall");

    /* Usually ready for level 5 to 9 */
    if (depth <= 9) return ((char*)NULL);


    /*** Essential Items for Level 10 to 19 ***/

    /* Require light (radius 2) */
    if (borg_skill[BI_CURLITE] < 2) return "2 Light";

    /* Escape or Teleport */
    if (borg_skill[BI_ATELEPORT] + borg_skill[BI_AESCAPE] < 2) return ("2 tele&esc");

    if (!borg_cfg[BORG_PLAYS_RISKY])
    {
        /* class specific requirement */
        switch (borg_class)
        {
        case CLASS_WARRIOR:
        case CLASS_BLACKGUARD:
        if (borg_skill[BI_MAXCLEVEL] < (depth - 4) && depth <= 19)
            return ("dlevel - 4 >= clevel");
        break;
        case CLASS_ROGUE:
        if (borg_skill[BI_MAXCLEVEL] < depth && depth <= 19) return ("dlevel >= clevel");
        break;
        case CLASS_PRIEST:
        case CLASS_DRUID:
        if (borg_skill[BI_MAXCLEVEL] < depth && depth <= 19) return ("dlevel >= clevel");
        break;
        case CLASS_PALADIN:
        if (borg_skill[BI_MAXCLEVEL] < depth && depth <= 19) return ("dlevel >= clevel");
        break;
        case CLASS_RANGER:
        if (borg_skill[BI_MAXCLEVEL] < depth && depth <= 19) return ("dlevel >= clevel");
        break;
        case CLASS_MAGE:
        case CLASS_NECROMANCER:
        if (borg_skill[BI_MAXCLEVEL] < (depth + 5) && borg_skill[BI_MAXCLEVEL] <= 28)
            return ("dlevel + 5 > = clevel");
        break;
        }
    }

    /* Potions of Cure Critical Wounds */
    if ((borg_skill[BI_MAXCLEVEL] < 30) && borg_skill[BI_ACCW] < 3) return ("cure crit3");

    /* See invisible */
    /* or telepathy */
    if ((!borg_skill[BI_SINV] && !borg_skill[BI_DINV] &&
        !borg_skill[BI_ESP])) return ("See Invis : ESP");

    /* Usually ready for level 10 to 19 */
    if (depth <= 19) return ((char*)NULL);


    /*** Essential Items for Level 20 ***/


    /* Free action */
    if (!borg_skill[BI_FRACT]) return ("FA");

    /* ready for level 20 */
    if (depth <= 20) return ((char*)NULL);


    /*** Essential Items for Level 25 ***/

    /* must have fire + 2 other basic resists */
    if (!borg_skill[BI_SRFIRE]) return ("RF");
    {
        int basics = borg_skill[BI_RACID] + borg_skill[BI_RCOLD] + borg_skill[BI_RELEC];

        if (basics < 2) return ("basic resist2");
    }
    /* have some minimal stats */
    if (borg_stat[STAT_STR] < 7) return ("low STR");

    int spell_stat = borg_spell_stat();
    if (spell_stat != -1)
    {
        if (borg_stat[spell_stat] < 7) return ("low spell stat");
    }
    if (borg_stat[STAT_DEX] < 7) return ("low DEX");
    if (borg_stat[STAT_CON] < 7) return ("low CON");

    if (!borg_cfg[BORG_PLAYS_RISKY])
    {
        /* class specific requirement */
        switch (borg_class)
        {
        case CLASS_WARRIOR:
        case CLASS_BLACKGUARD:
        if (borg_skill[BI_MAXCLEVEL] < (depth + 5) && borg_skill[BI_MAXCLEVEL] <= 38)
            return ("dlevel + 5 >= clevel");
        break;
        case CLASS_ROGUE:
        if (borg_skill[BI_MAXCLEVEL] < (depth + 10) && borg_skill[BI_MAXCLEVEL] <= 43)
            return ("dlevel + 10 >= clevel");
        break;
        case CLASS_PRIEST:
        case CLASS_DRUID:
        if (borg_skill[BI_MAXCLEVEL] < (depth + 13) && borg_skill[BI_MAXCLEVEL] <= 46)
            return ("dlevel + 13 >= clevel");
        break;
        case CLASS_PALADIN:
        if (borg_skill[BI_MAXCLEVEL] < (depth + 7) && borg_skill[BI_MAXCLEVEL] <= 40)
            return ("dlevel + 7 >= clevel");
        break;
        case CLASS_RANGER:
        if (borg_skill[BI_MAXCLEVEL] < (depth + 8) && borg_skill[BI_MAXCLEVEL] <= 41 && borg_skill[BI_MAXCLEVEL] > 28)
            return ("dlevel + 8 >= clevel");
        break;
        case CLASS_MAGE:
        case CLASS_NECROMANCER:
        if (borg_skill[BI_MAXCLEVEL] < (depth + 8) && borg_skill[BI_MAXCLEVEL] <= 38)
            return ("dlevel + 8 >= clevel");
        if (((borg_skill[BI_MAXCLEVEL] - 38) * 2 + 30) < depth &&
            borg_skill[BI_MAXCLEVEL] <= 44 &&
            borg_skill[BI_MAXCLEVEL] > 38)
            return ("(clevel-38)*2+30 < dlevel");
        break;
        }
    }

    /* Ready for level 25 */
    if (depth <= 25) return ((char*)NULL);


    /*** Essential Items for Level 25 to 39 ***/

        /* All Basic resistance*/
    if (!borg_skill[BI_SRCOLD]) return ("RC");
    if (!borg_skill[BI_SRELEC]) return ("RE");
    if (!borg_skill[BI_SRACID]) return ("RA");

    /* Escape and Teleport */
    if (borg_skill[BI_ATELEPORT] + borg_skill[BI_AESCAPE] < 6) return ("tell&esc6");

    /* Cure Critical Wounds */
    if ((borg_skill[BI_MAXCLEVEL] < 30) && (borg_skill[BI_ACCW] + borg_skill[BI_ACSW]) < 10) return ("cure10");

    /* Ready for level 33 */
    if (depth <= 33) return ((char*)NULL);

    /* Minimal level */
    if (borg_skill[BI_MAXCLEVEL] < 40 && !borg_cfg[BORG_PLAYS_RISKY]) return ("level 40");

    /* Usually ready for level 20 to 39 */
    if (depth <= 39) return ((char*)NULL);



    /*** Essential Items for Level 40 to 45 ***/

        /* Resist */
    if (!borg_skill[BI_SRPOIS]) return ("RPois");
    if (!borg_skill[BI_SRCONF])  return ("RConf");

    if (borg_stat[STAT_STR] < 16) return ("low STR");

    if (spell_stat != -1)
    {
        if (borg_stat[spell_stat] < 16) return ("low spell stat");
    }
    if (borg_stat[STAT_DEX] < 16) return ("low DEX");
    if (borg_stat[STAT_CON] < 16) return ("low CON");


    /* Ok to continue */
    if (depth <= 45) return ((char*)NULL);


    /*** Essential Items for Level 46 to 55 ***/

        /*  Must have +5 speed after level 46 */
    if (borg_skill[BI_SPEED] < 115) return ("+5 speed");

    /* Potions of heal */
    if (borg_skill[BI_AHEAL] < 1 && (borg_skill[BI_AEZHEAL] < 1)) return ("1heal");

    if (!borg_cfg[BORG_PLAYS_RISKY])
    {
        /* Minimal hitpoints */
        if (borg_skill[BI_MAXHP] < 500) return ("HP 500");
    }

    /* High stats XXX XXX XXX */
    if (borg_stat[STAT_STR] < 18 + 40) return ("low STR");

    if (spell_stat != -1)
    {
        if (borg_stat[spell_stat] < 18 + 100) return ("low spell stat");
    }
    if (borg_stat[STAT_DEX] < 18 + 60) return ("low DEX");
    if (borg_stat[STAT_CON] < 18 + 60) return ("low CON");

    /* Hold Life */
    if ((!borg_skill[BI_HLIFE] && !weapon_swap_hold_life &&
        !armour_swap_hold_life) && (borg_skill[BI_MAXCLEVEL] < 50)) return ("hold life");

    /* Usually ready for level 46 to 55 */
    if (depth <= 55) return ((char*)NULL);

    /*** Essential Items for Level 55 to 59 ***/

        /* Potions of heal */
    if (borg_skill[BI_AHEAL] < 2 && borg_skill[BI_AEZHEAL] < 1) return ("2heal");

    /* Resists */
    if (!borg_skill[BI_SRBLIND]) return ("RBlind");

    /* Must have resist nether */
/*    if (!borg_settings[BORG_PLAYS_RISKY] && !borg_skill[BI_SRNTHR]) return ("RNeth"); */


    /* Telepathy, better have it by now */
    if (!borg_skill[BI_ESP]) return ("ESP");

    /* Usually ready for level 55 to 59 */
    if (depth <= 59) return ((char*)NULL);



    /*** Essential Items for Level 61 to 80 ***/

        /* Must have +10 speed */
    if (borg_skill[BI_SPEED] < 120) return ("+10 speed");


    /* Resists */
    if (!borg_skill[BI_SRKAOS]) return ("RChaos");
    if (!borg_skill[BI_SRDIS]) return ("RDisen");

    /* Usually ready for level 61 to 80 */
    if (depth <= 80) return ((char*)NULL);

    /*** Essential Items for Level 81-85 ***/
        /* Minimal Speed */
    if (borg_skill[BI_SPEED] < 130) return ("+20 Speed");

    /* Usually ready for level 81 to 85 */
    if (depth <= 85) return ((char*)NULL);


    /*** Essential Items for Level 86-99 ***/


        /* Usually ready for level 86 to 99 */
    if (depth <= 99) return ((char*)NULL);

    /*** Essential Items for Level 100 ***/

        /* must have lots of restore mana to go after MORGOTH */
    if (!borg_skill[BI_KING])
    {
        if ((borg_skill[BI_MAXSP] > 100) && (borg_has[kv_potion_restore_mana] < 15)) return ("10ResMana");

        /* must have lots of heal */
        if (borg_has[kv_potion_healing] < 5) return ("5Heal");

        /* must have lots of ez-heal */
        if (borg_skill[BI_AEZHEAL] < 15) return ("15EZHeal");

        /* must have lots of speed */
        if (borg_skill[BI_ASPEED] < 10) return ("10Speed");

    }

    /* Its good to be the king */
    if (depth <= 127) return ((char*)NULL);

    /* all bases covered */
    return ((char*)NULL);
}

/* buffer for borg_prepared mesage
 */
#define MAX_REASON 1024
static char borg_prepared_buffer[MAX_REASON];

/*
 * Determine if the Borg is "prepared" for the given level
 *
 * This routine does not help him decide how to get ready for the
 * given level, so it must work closely with "borg_power()".
 *
 * Note that we ignore any "town fear", and we allow fear of one
 * level up to and including the relevant depth.
 *
 * This now returns a string with the reason you are not prepared.
 *
 */
const char* borg_prepared(int depth)
{
    const char* reason;

    /* Town and First level */
    if (depth == 1) return ((char*)NULL);

    /* Not prepared if I need to restock */
    if ((reason = borg_restock(depth)))	return (reason);

    /*** Require his Clevel to be greater than or equal to Depth */
    if (borg_skill[BI_MAXCLEVEL] < depth && borg_skill[BI_MAXCLEVEL] < 50) return ("Clevel < depth");

    /* Must meet minimal requirements */
    if (depth <= 99)
    {
        if ((reason = borg_prepared_aux(depth))) return (reason);
    }

    /* Not if No_Deeper is set */
    if (depth >= borg_cfg[BORG_NO_DEEPER])
    {
        strnfmt(borg_prepared_buffer, MAX_REASON, "No deeper %d.", borg_cfg[BORG_NO_DEEPER]);
        return (borg_prepared_buffer);
    }


    /* Once Morgoth is dead */
    if (borg_skill[BI_KING])
    {
        return ((char*)NULL);
    }

    /* Always okay from town */
    if (!borg_skill[BI_CDEPTH])	return (reason);

    /* Scum on depth 80-81 for some *heal* potions */
    if (depth >= 82 && (num_ezheal < 10 && borg_skill[BI_AEZHEAL] < 10))
    {
        /* Must know exact number of Potions  in home */
        borg_notice_home(NULL, false);

        strnfmt(borg_prepared_buffer, MAX_REASON, "Scumming *Heal* potions (%d to go).", 10 - num_ezheal);
        return (borg_prepared_buffer);
    }

    /* Scum on depth 80-81 for lots of *Heal* potions preparatory for Endgame */
    if (depth >= 82 && borg_skill[BI_MAXDEPTH] >= 97)
    {
        /* Must know exact number of Potions  in home */
        borg_notice_home(NULL, false);

        /* Scum for 30*/
        if (num_ezheal_true + borg_skill[BI_AEZHEAL] < 30)
        {
            strnfmt(borg_prepared_buffer, MAX_REASON, "Scumming *Heal* potions (%d to go).", 30 -
                (num_ezheal_true + borg_skill[BI_AEZHEAL]));
            return (borg_prepared_buffer);
        }

        /* Return to town to get your stock from the home*/
        if (num_ezheal_true + borg_skill[BI_AEZHEAL] >= 30 && /* Enough combined EZ_HEALS */
            num_ezheal_true >= 1 && borg_skill[BI_MAXDEPTH] >= 99) /* Still some sitting in the house */
        {
            strnfmt(borg_prepared_buffer, MAX_REASON, "Collect from house (%d potions).", num_ezheal_true);
            return (borg_prepared_buffer);
        }
    }

    /* Check to make sure the borg does not go below where 3 living */
    /* uniques are. */
    if (borg_skill[BI_MAXDEPTH] <= 98)
    {
        struct monster_race* r_ptr = &r_info[borg_living_unique_index];

        /* are too many uniques alive */
        if (borg_numb_live_unique < 3 || 
            borg_cfg[BORG_PLAYS_RISKY] ||
            borg_skill[BI_CLEVEL] == 50 || 
            borg_cfg[BORG_KILLS_UNIQUES] == false)
            return ((char*)NULL);

        /* Check for the dlevel of the unique */
        if (depth < borg_unique_depth) return ((char*)NULL);

        /* To avoid double calls to format() */
        /* Reset our description for not diving */
        strnfmt(borg_prepared_buffer, MAX_REASON, "Must kill %s.", r_ptr->name);
        return (borg_prepared_buffer);

    }
    else if (borg_skill[BI_MAXDEPTH] >= 98 || depth >= 98)
        /* check to make sure the borg does not go to level 100 */
        /* unless all the uniques are dead. */
    {
        struct monster_race* r_ptr;

        /* Access the living unique obtained from borg_update() */
        r_ptr = &r_info[borg_living_unique_index];

        /* -1 is unknown. */
        borg_ready_morgoth = -1;

        if (borg_numb_live_unique < 1 ||
            borg_living_unique_index == borg_morgoth_id) /* Morgoth */
        {
            if (depth >= 99) borg_ready_morgoth = 1;
            return ((char*)NULL);
        }

        /* Under special cases allow the borg to dive to 99 then quickly
         * get his butt to dlevel 98
         */
        if (borg_skill[BI_MAXDEPTH] == 99 && depth <= 98 &&
            (borg_spell_legal_fail(TELEPORT_LEVEL, 20) || /* Teleport Level */
                borg_skill[BI_ATELEPORTLVL] >= 1)) /* Teleport Level scroll */
        {
            return ((char*)NULL);
        }

        /* To avoid double calls to format() */
        strnfmt(borg_prepared_buffer, MAX_REASON, "%s still alive!", r_ptr->name);
        return (borg_prepared_buffer);

    }
    return (char*)NULL;
}

/*
 * Initialize this file
 */
void borg_init_4(void)
{
    /* Do nothing? */
}

/*
 * Release resources allocated by borg_init_4().
 */
void borg_clean_4(void)
{
    /* Nothing */
}

#ifdef MACINTOSH
static int HACK = 0;
#endif
#endif /* ALLOW_BORG */
