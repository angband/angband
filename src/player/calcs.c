/*
 * File: player/calcs.c
 * Purpose: Player status calculation, signalling ui events based on status
 *          changes.
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"
#include "cave.h"
#include "files.h"
#include "game-event.h"
#include "monster/mon-msg.h"
#include "monster/mon-util.h"
#include "object/tvalsval.h"
#include "object/pval.h"
#include "spells.h"
#include "squelch.h"

/*
 * Stat Table (CHR) -- payment percentages
 */
const byte adj_chr_gold[STAT_RANGE] =
{
	143	/* 3 */,
	137	/* 4 */,
	134	/* 5 */,
	132	/* 6 */,
	129	/* 7 */,
	127	/* 8 */,
	123	/* 9 */,
	122	/* 10 */,
	121	/* 11 */,
	118	/* 12 */,
	116	/* 13 */,
	113	/* 14 */,
	113	/* 15 */,
	112	/* 16 */,
	111	/* 17 */,
	110	/* 18/00-18/09 */,
	108	/* 18/10-18/19 */,
	107	/* 18/20-18/29 */,
	106	/* 18/30-18/39 */,
	105	/* 18/40-18/49 */,
	104	/* 18/50-18/59 */,
	103	/* 18/60-18/69 */,
	102	/* 18/70-18/79 */,
	101	/* 18/80-18/89 */,
	100	/* 18/90-18/99 */,
	99	/* 18/100-18/109 */,
	97	/* 18/110-18/119 */,
	96	/* 18/120-18/129 */,
	95	/* 18/130-18/139 */,
	94	/* 18/140-18/149 */,
	93	/* 18/150-18/159 */,
	92	/* 18/160-18/169 */,
	91	/* 18/170-18/179 */,
	90	/* 18/180-18/189 */,
	90	/* 18/190-18/199 */,
	90	/* 18/200-18/209 */,
	90	/* 18/210-18/219 */,
	90	/* 18/220+ */
};

/*
 * Stat Table (INT) -- Magic devices
 */
static const byte adj_int_dev[STAT_RANGE] =
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

/*
 * Stat Table (WIS) -- Saving throw
 */
static const byte adj_wis_sav[STAT_RANGE] =
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


/*
 * Stat Table (DEX) -- disarming
 */
static const byte adj_dex_dis[STAT_RANGE] =
{
	0	/* 3 */,
	0	/* 4 */,
	0	/* 5 */,
	0	/* 6 */,
	0	/* 7 */,
	0	/* 8 */,
	0	/* 9 */,
	0	/* 10 */,
	0	/* 11 */,
	0	/* 12 */,
	1	/* 13 */,
	1	/* 14 */,
	1	/* 15 */,
	2	/* 16 */,
	2	/* 17 */,
	4	/* 18/00-18/09 */,
	4	/* 18/10-18/19 */,
	4	/* 18/20-18/29 */,
	4	/* 18/30-18/39 */,
	5	/* 18/40-18/49 */,
	5	/* 18/50-18/59 */,
	5	/* 18/60-18/69 */,
	6	/* 18/70-18/79 */,
	6	/* 18/80-18/89 */,
	7	/* 18/90-18/99 */,
	8	/* 18/100-18/109 */,
	8	/* 18/110-18/119 */,
	8	/* 18/120-18/129 */,
	8	/* 18/130-18/139 */,
	8	/* 18/140-18/149 */,
	9	/* 18/150-18/159 */,
	9	/* 18/160-18/169 */,
	9	/* 18/170-18/179 */,
	9	/* 18/180-18/189 */,
	9	/* 18/190-18/199 */,
	10	/* 18/200-18/209 */,
	10	/* 18/210-18/219 */,
	10	/* 18/220+ */
};


/*
 * Stat Table (INT) -- disarming
 */
static const byte adj_int_dis[STAT_RANGE] =
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

/*
 * Stat Table (DEX) -- bonus to ac (plus 128)
 */
static const byte adj_dex_ta[STAT_RANGE] =
{
	128 + -4	/* 3 */,
	128 + -3	/* 4 */,
	128 + -2	/* 5 */,
	128 + -1	/* 6 */,
	128 + 0	/* 7 */,
	128 + 0	/* 8 */,
	128 + 0	/* 9 */,
	128 + 0	/* 10 */,
	128 + 0	/* 11 */,
	128 + 0	/* 12 */,
	128 + 0	/* 13 */,
	128 + 0	/* 14 */,
	128 + 1	/* 15 */,
	128 + 1	/* 16 */,
	128 + 1	/* 17 */,
	128 + 2	/* 18/00-18/09 */,
	128 + 2	/* 18/10-18/19 */,
	128 + 2	/* 18/20-18/29 */,
	128 + 2	/* 18/30-18/39 */,
	128 + 2	/* 18/40-18/49 */,
	128 + 3	/* 18/50-18/59 */,
	128 + 3	/* 18/60-18/69 */,
	128 + 3	/* 18/70-18/79 */,
	128 + 4	/* 18/80-18/89 */,
	128 + 5	/* 18/90-18/99 */,
	128 + 6	/* 18/100-18/109 */,
	128 + 7	/* 18/110-18/119 */,
	128 + 8	/* 18/120-18/129 */,
	128 + 9	/* 18/130-18/139 */,
	128 + 9	/* 18/140-18/149 */,
	128 + 10	/* 18/150-18/159 */,
	128 + 11	/* 18/160-18/169 */,
	128 + 12	/* 18/170-18/179 */,
	128 + 13	/* 18/180-18/189 */,
	128 + 14	/* 18/190-18/199 */,
	128 + 15	/* 18/200-18/209 */,
	128 + 15	/* 18/210-18/219 */,
	128 + 15	/* 18/220+ */
};

/*
 * Stat Table (STR) -- bonus to dam (plus 128)
 */
static const byte adj_str_td[STAT_RANGE] =
{
	128 + -2	/* 3 */,
	128 + -2	/* 4 */,
	128 + -1	/* 5 */,
	128 + -1	/* 6 */,
	128 + 0	/* 7 */,
	128 + 0	/* 8 */,
	128 + 0	/* 9 */,
	128 + 0	/* 10 */,
	128 + 0	/* 11 */,
	128 + 0	/* 12 */,
	128 + 0	/* 13 */,
	128 + 0	/* 14 */,
	128 + 0	/* 15 */,
	128 + 1	/* 16 */,
	128 + 2	/* 17 */,
	128 + 2	/* 18/00-18/09 */,
	128 + 2	/* 18/10-18/19 */,
	128 + 3	/* 18/20-18/29 */,
	128 + 3	/* 18/30-18/39 */,
	128 + 3	/* 18/40-18/49 */,
	128 + 3	/* 18/50-18/59 */,
	128 + 3	/* 18/60-18/69 */,
	128 + 4	/* 18/70-18/79 */,
	128 + 5	/* 18/80-18/89 */,
	128 + 5	/* 18/90-18/99 */,
	128 + 6	/* 18/100-18/109 */,
	128 + 7	/* 18/110-18/119 */,
	128 + 8	/* 18/120-18/129 */,
	128 + 9	/* 18/130-18/139 */,
	128 + 10	/* 18/140-18/149 */,
	128 + 11	/* 18/150-18/159 */,
	128 + 12	/* 18/160-18/169 */,
	128 + 13	/* 18/170-18/179 */,
	128 + 14	/* 18/180-18/189 */,
	128 + 15	/* 18/190-18/199 */,
	128 + 16	/* 18/200-18/209 */,
	128 + 18	/* 18/210-18/219 */,
	128 + 20	/* 18/220+ */
};


/*
 * Stat Table (DEX) -- bonus to hit (plus 128)
 */
static const byte adj_dex_th[STAT_RANGE] =
{
	128 + -3	/* 3 */,
	128 + -2	/* 4 */,
	128 + -2	/* 5 */,
	128 + -1	/* 6 */,
	128 + -1	/* 7 */,
	128 + 0	/* 8 */,
	128 + 0	/* 9 */,
	128 + 0	/* 10 */,
	128 + 0	/* 11 */,
	128 + 0	/* 12 */,
	128 + 0	/* 13 */,
	128 + 0	/* 14 */,
	128 + 0	/* 15 */,
	128 + 1	/* 16 */,
	128 + 2	/* 17 */,
	128 + 3	/* 18/00-18/09 */,
	128 + 3	/* 18/10-18/19 */,
	128 + 3	/* 18/20-18/29 */,
	128 + 3	/* 18/30-18/39 */,
	128 + 3	/* 18/40-18/49 */,
	128 + 4	/* 18/50-18/59 */,
	128 + 4	/* 18/60-18/69 */,
	128 + 4	/* 18/70-18/79 */,
	128 + 4	/* 18/80-18/89 */,
	128 + 5	/* 18/90-18/99 */,
	128 + 6	/* 18/100-18/109 */,
	128 + 7	/* 18/110-18/119 */,
	128 + 8	/* 18/120-18/129 */,
	128 + 9	/* 18/130-18/139 */,
	128 + 9	/* 18/140-18/149 */,
	128 + 10	/* 18/150-18/159 */,
	128 + 11	/* 18/160-18/169 */,
	128 + 12	/* 18/170-18/179 */,
	128 + 13	/* 18/180-18/189 */,
	128 + 14	/* 18/190-18/199 */,
	128 + 15	/* 18/200-18/209 */,
	128 + 15	/* 18/210-18/219 */,
	128 + 15	/* 18/220+ */
};


/*
 * Stat Table (STR) -- bonus to hit (plus 128)
 */
static const byte adj_str_th[STAT_RANGE] =
{
	128 + -3	/* 3 */,
	128 + -2	/* 4 */,
	128 + -1	/* 5 */,
	128 + -1	/* 6 */,
	128 + 0	/* 7 */,
	128 + 0	/* 8 */,
	128 + 0	/* 9 */,
	128 + 0	/* 10 */,
	128 + 0	/* 11 */,
	128 + 0	/* 12 */,
	128 + 0	/* 13 */,
	128 + 0	/* 14 */,
	128 + 0	/* 15 */,
	128 + 0	/* 16 */,
	128 + 0	/* 17 */,
	128 + 1	/* 18/00-18/09 */,
	128 + 1	/* 18/10-18/19 */,
	128 + 1	/* 18/20-18/29 */,
	128 + 1	/* 18/30-18/39 */,
	128 + 1	/* 18/40-18/49 */,
	128 + 1	/* 18/50-18/59 */,
	128 + 1	/* 18/60-18/69 */,
	128 + 2	/* 18/70-18/79 */,
	128 + 3	/* 18/80-18/89 */,
	128 + 4	/* 18/90-18/99 */,
	128 + 5	/* 18/100-18/109 */,
	128 + 6	/* 18/110-18/119 */,
	128 + 7	/* 18/120-18/129 */,
	128 + 8	/* 18/130-18/139 */,
	128 + 9	/* 18/140-18/149 */,
	128 + 10	/* 18/150-18/159 */,
	128 + 11	/* 18/160-18/169 */,
	128 + 12	/* 18/170-18/179 */,
	128 + 13	/* 18/180-18/189 */,
	128 + 14	/* 18/190-18/199 */,
	128 + 15	/* 18/200-18/209 */,
	128 + 15	/* 18/210-18/219 */,
	128 + 15	/* 18/220+ */
};


/*
 * Stat Table (STR) -- weight limit in deca-pounds
 */
static const byte adj_str_wgt[STAT_RANGE] =
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


/*
 * Stat Table (STR) -- weapon weight limit in pounds
 */
const byte adj_str_hold[STAT_RANGE] =
{
	4	/* 3 */,
	5	/* 4 */,
	6	/* 5 */,
	7	/* 6 */,
	8	/* 7 */,
	10	/* 8 */,
	12	/* 9 */,
	14	/* 10 */,
	16	/* 11 */,
	18	/* 12 */,
	20	/* 13 */,
	22	/* 14 */,
	24	/* 15 */,
	26	/* 16 */,
	28	/* 17 */,
	30	/* 18/00-18/09 */,
	30	/* 18/10-18/19 */,
	35	/* 18/20-18/29 */,
	40	/* 18/30-18/39 */,
	45	/* 18/40-18/49 */,
	50	/* 18/50-18/59 */,
	55	/* 18/60-18/69 */,
	60	/* 18/70-18/79 */,
	65	/* 18/80-18/89 */,
	70	/* 18/90-18/99 */,
	80	/* 18/100-18/109 */,
	80	/* 18/110-18/119 */,
	80	/* 18/120-18/129 */,
	80	/* 18/130-18/139 */,
	80	/* 18/140-18/149 */,
	90	/* 18/150-18/159 */,
	90	/* 18/160-18/169 */,
	90	/* 18/170-18/179 */,
	90	/* 18/180-18/189 */,
	90	/* 18/190-18/199 */,
	100	/* 18/200-18/209 */,
	100	/* 18/210-18/219 */,
	100	/* 18/220+ */
};


/*
 * Stat Table (STR) -- digging value
 */
static const byte adj_str_dig[STAT_RANGE] =
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


/*
 * Stat Table (STR) -- help index into the "blow" table
 */
const byte adj_str_blow[STAT_RANGE] =
{
	3	/* 3 */,
	4	/* 4 */,
	5	/* 5 */,
	6	/* 6 */,
	7	/* 7 */,
	8	/* 8 */,
	9	/* 9 */,
	10	/* 10 */,
	11	/* 11 */,
	12	/* 12 */,
	13	/* 13 */,
	14	/* 14 */,
	15	/* 15 */,
	16	/* 16 */,
	17	/* 17 */,
	20 /* 18/00-18/09 */,
	30 /* 18/10-18/19 */,
	40 /* 18/20-18/29 */,
	50 /* 18/30-18/39 */,
	60 /* 18/40-18/49 */,
	70 /* 18/50-18/59 */,
	80 /* 18/60-18/69 */,
	90 /* 18/70-18/79 */,
	100 /* 18/80-18/89 */,
	110 /* 18/90-18/99 */,
	120 /* 18/100-18/109 */,
	130 /* 18/110-18/119 */,
	140 /* 18/120-18/129 */,
	150 /* 18/130-18/139 */,
	160 /* 18/140-18/149 */,
	170 /* 18/150-18/159 */,
	180 /* 18/160-18/169 */,
	190 /* 18/170-18/179 */,
	200 /* 18/180-18/189 */,
	210 /* 18/190-18/199 */,
	220 /* 18/200-18/209 */,
	230 /* 18/210-18/219 */,
	240 /* 18/220+ */
};


/*
 * Stat Table (DEX) -- index into the "blow" table
 */
static const byte adj_dex_blow[STAT_RANGE] =
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


/*
 * Stat Table (DEX) -- chance of avoiding "theft" and "falling"
 */
const byte adj_dex_safe[STAT_RANGE] =
{
	0	/* 3 */,
	1	/* 4 */,
	2	/* 5 */,
	3	/* 6 */,
	4	/* 7 */,
	5	/* 8 */,
	5	/* 9 */,
	6	/* 10 */,
	6	/* 11 */,
	7	/* 12 */,
	7	/* 13 */,
	8	/* 14 */,
	8	/* 15 */,
	9	/* 16 */,
	9	/* 17 */,
	10	/* 18/00-18/09 */,
	10	/* 18/10-18/19 */,
	15	/* 18/20-18/29 */,
	15	/* 18/30-18/39 */,
	20	/* 18/40-18/49 */,
	25	/* 18/50-18/59 */,
	30	/* 18/60-18/69 */,
	35	/* 18/70-18/79 */,
	40	/* 18/80-18/89 */,
	45	/* 18/90-18/99 */,
	50	/* 18/100-18/109 */,
	60	/* 18/110-18/119 */,
	70	/* 18/120-18/129 */,
	80	/* 18/130-18/139 */,
	90	/* 18/140-18/149 */,
	100	/* 18/150-18/159 */,
	100	/* 18/160-18/169 */,
	100	/* 18/170-18/179 */,
	100	/* 18/180-18/189 */,
	100	/* 18/190-18/199 */,
	100	/* 18/200-18/209 */,
	100	/* 18/210-18/219 */,
	100	/* 18/220+ */
};


/*
 * Stat Table (CON) -- base regeneration rate
 */
const byte adj_con_fix[STAT_RANGE] =
{
	0	/* 3 */,
	0	/* 4 */,
	0	/* 5 */,
	0	/* 6 */,
	0	/* 7 */,
	0	/* 8 */,
	0	/* 9 */,
	0	/* 10 */,
	0	/* 11 */,
	0	/* 12 */,
	0	/* 13 */,
	1	/* 14 */,
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
	3	/* 18/80-18/89 */,
	3	/* 18/90-18/99 */,
	4	/* 18/100-18/109 */,
	4	/* 18/110-18/119 */,
	5	/* 18/120-18/129 */,
	6	/* 18/130-18/139 */,
	6	/* 18/140-18/149 */,
	7	/* 18/150-18/159 */,
	7	/* 18/160-18/169 */,
	8	/* 18/170-18/179 */,
	8	/* 18/180-18/189 */,
	8	/* 18/190-18/199 */,
	9	/* 18/200-18/209 */,
	9	/* 18/210-18/219 */,
	9	/* 18/220+ */
};


/*
 * Stat Table (CON) -- extra 1/100th hitpoints per level
 */
static const int adj_con_mhp[STAT_RANGE] =
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

static const int adj_mag_study[STAT_RANGE] =
{
	  0	/* 3 */,
	  0	/* 4 */,
	 10	/* 5 */,
	 20	/* 6 */,
	 30	/* 7 */,
	 40	/* 8 */,
	 50	/* 9 */,
	 60	/* 10 */,
	 70	/* 11 */,
	 80	/* 12 */,
	 85	/* 13 */,
	 90	/* 14 */,
	 95	/* 15 */,
	100	/* 16 */,
	105	/* 17 */,
	110	/* 18/00-18/09 */,
	115	/* 18/10-18/19 */,
	120	/* 18/20-18/29 */,
	130	/* 18/30-18/39 */,
	140	/* 18/40-18/49 */,
	150	/* 18/50-18/59 */,
	160	/* 18/60-18/69 */,
	170	/* 18/70-18/79 */,
	180	/* 18/80-18/89 */,
	190	/* 18/90-18/99 */,
	200	/* 18/100-18/109 */,
	210	/* 18/110-18/119 */,
	220	/* 18/120-18/129 */,
	230	/* 18/130-18/139 */,
	240	/* 18/140-18/149 */,
	250	/* 18/150-18/159 */,
	250	/* 18/160-18/169 */,
	250	/* 18/170-18/179 */,
	250	/* 18/180-18/189 */,
	250	/* 18/190-18/199 */,
	250	/* 18/200-18/209 */,
	250	/* 18/210-18/219 */,
	250	/* 18/220+ */
};

/*
 * Stat Table (INT/WIS) -- extra 1/100 mana-points per level
 */
static const int adj_mag_mana[STAT_RANGE] =
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

/*
 * This table is used to help calculate the number of blows the player can
 * make in a single round of attacks (one player turn) with a normal weapon.
 *
 * This number ranges from a single blow/round for weak players to up to six
 * blows/round for powerful warriors.
 *
 * Note that certain artifacts and ego-items give "bonus" blows/round.
 *
 * First, from the player class, we extract some values:
 *
 *    Warrior --> num = 6; mul = 5; div = MAX(30, weapon_weight);
 *    Mage    --> num = 4; mul = 2; div = MAX(40, weapon_weight);
 *    Priest  --> num = 4; mul = 3; div = MAX(35, weapon_weight);
 *    Rogue   --> num = 5; mul = 4; div = MAX(30, weapon_weight);
 *    Ranger  --> num = 5; mul = 4; div = MAX(35, weapon_weight);
 *    Paladin --> num = 5; mul = 5; div = MAX(30, weapon_weight);
 * (all specified in p_class.txt now)
 *
 * To get "P", we look up the relevant "adj_str_blow[]" (see above),
 * multiply it by "mul", and then divide it by "div", rounding down.
 *
 * To get "D", we look up the relevant "adj_dex_blow[]" (see above).
 *
 * Then we look up the energy cost of each blow using "blows_table[P][D]".
 * The player gets blows/round equal to 100/this number, up to a maximum of
 * "num" blows/round, plus any "bonus" blows/round.
 */
static const byte blows_table[12][12] =
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


/*
 * Calculate number of spells player should have, and forget,
 * or remember, spells until that number is properly reflected.
 *
 * Note that this function induces various "status" messages,
 * which must be bypasses until the character is created.
 */
static void calc_spells(void)
{
	int i, j, k, levels;
	int num_allowed, num_known;
	int percent_spells;

	const magic_type *s_ptr;

	s16b old_spells;

	const char *p = ((p_ptr->class->spell_book == TV_MAGIC_BOOK) ? "spell" : "prayer");

	/* Hack -- must be literate */
	if (!p_ptr->class->spell_book) return;

	/* Hack -- wait for creation */
	if (!character_generated) return;

	/* Hack -- handle "xtra" mode */
	if (character_xtra) return;

	/* Save the new_spells value */
	old_spells = p_ptr->new_spells;

	/* Determine the number of spells allowed */
	levels = p_ptr->lev - p_ptr->class->spell_first + 1;

	/* Hack -- no negative spells */
	if (levels < 0) levels = 0;

	/* Number of 1/100 spells per level */
	percent_spells = adj_mag_study[p_ptr->state.stat_ind[p_ptr->class->spell_stat]];

	/* Extract total allowed spells (rounded up) */
	num_allowed = (((percent_spells * levels) + 50) / 100);

	/* Assume none known */
	num_known = 0;

	/* Count the number of spells we know */
	for (j = 0; j < PY_MAX_SPELLS; j++)
	{
		/* Count known spells */
		if (p_ptr->spell_flags[j] & PY_SPELL_LEARNED)
		{
			num_known++;
		}
	}

	/* See how many spells we must forget or may learn */
	p_ptr->new_spells = num_allowed - num_known;

	/* Forget spells which are too hard */
	for (i = PY_MAX_SPELLS - 1; i >= 0; i--)
	{
		/* Get the spell */
		j = p_ptr->spell_order[i];

		/* Skip non-spells */
		if (j >= 99) continue;

		/* Get the spell */
		s_ptr = &p_ptr->class->spells.info[j];

		/* Skip spells we are allowed to know */
		if (s_ptr->slevel <= p_ptr->lev) continue;

		/* Is it known? */
		if (p_ptr->spell_flags[j] & PY_SPELL_LEARNED)
		{
			/* Mark as forgotten */
			p_ptr->spell_flags[j] |= PY_SPELL_FORGOTTEN;

			/* No longer known */
			p_ptr->spell_flags[j] &= ~PY_SPELL_LEARNED;

			/* Message */
			msg("You have forgotten the %s of %s.", p,
			           get_spell_name(p_ptr->class->spell_book, j));

			/* One more can be learned */
			p_ptr->new_spells++;
		}
	}

	/* Forget spells if we know too many spells */
	for (i = PY_MAX_SPELLS - 1; i >= 0; i--)
	{
		/* Stop when possible */
		if (p_ptr->new_spells >= 0) break;

		/* Get the (i+1)th spell learned */
		j = p_ptr->spell_order[i];

		/* Skip unknown spells */
		if (j >= 99) continue;

		/* Forget it (if learned) */
		if (p_ptr->spell_flags[j] & PY_SPELL_LEARNED)
		{
			/* Mark as forgotten */
			p_ptr->spell_flags[j] |= PY_SPELL_FORGOTTEN;

			/* No longer known */
			p_ptr->spell_flags[j] &= ~PY_SPELL_LEARNED;

			/* Message */
			msg("You have forgotten the %s of %s.", p,
			           get_spell_name(p_ptr->class->spell_book, j));

			/* One more can be learned */
			p_ptr->new_spells++;
		}
	}

	/* Check for spells to remember */
	for (i = 0; i < PY_MAX_SPELLS; i++)
	{
		/* None left to remember */
		if (p_ptr->new_spells <= 0) break;

		/* Get the next spell we learned */
		j = p_ptr->spell_order[i];

		/* Skip unknown spells */
		if (j >= 99) break;

		/* Get the spell */
		s_ptr = &p_ptr->class->spells.info[j];

		/* Skip spells we cannot remember */
		if (s_ptr->slevel > p_ptr->lev) continue;

		/* First set of spells */
		if (p_ptr->spell_flags[j] & PY_SPELL_FORGOTTEN)
		{
			/* No longer forgotten */
			p_ptr->spell_flags[j] &= ~PY_SPELL_FORGOTTEN;

			/* Known once more */
			p_ptr->spell_flags[j] |= PY_SPELL_LEARNED;

			/* Message */
			msg("You have remembered the %s of %s.",
			           p, get_spell_name(p_ptr->class->spell_book, j));

			/* One less can be learned */
			p_ptr->new_spells--;
		}
	}

	/* Assume no spells available */
	k = 0;

	/* Count spells that can be learned */
	for (j = 0; j < PY_MAX_SPELLS; j++)
	{
		/* Get the spell */
		s_ptr = &p_ptr->class->spells.info[j];

		/* Skip spells we cannot remember or don't exist */
		if (s_ptr->slevel > p_ptr->lev || s_ptr->slevel == 0) continue;

		/* Skip spells we already know */
		if (p_ptr->spell_flags[j] & PY_SPELL_LEARNED)
		{
			continue;
		}

		/* Count it */
		k++;
	}

	/* Cannot learn more spells than exist */
	if (p_ptr->new_spells > k) p_ptr->new_spells = k;

	/* Spell count changed */
	if (old_spells != p_ptr->new_spells)
	{
		/* Message if needed */
		if (p_ptr->new_spells)
		{
			/* Message */
			msg("You can learn %d more %s%s.",
			           p_ptr->new_spells, p,
			           (p_ptr->new_spells != 1) ? "s" : "");
		}

		/* Redraw Study Status */
		p_ptr->redraw |= (PR_STUDY | PR_OBJECT);
	}
}


/*
 * Calculate maximum mana.  You do not need to know any spells.
 * Note that mana is lowered by heavy (or inappropriate) armor.
 *
 * This function induces status messages.
 */
static void calc_mana(void)
{
	int msp, levels, cur_wgt, max_wgt;

	object_type *o_ptr;

	bool old_cumber_glove = p_ptr->cumber_glove;
	bool old_cumber_armor = p_ptr->cumber_armor;

	/* Hack -- Must be literate */
	if (!p_ptr->class->spell_book)
	{
		p_ptr->msp = 0;
		p_ptr->csp = 0;
		p_ptr->csp_frac = 0;
		return;
	}

	/* Extract "effective" player level */
	levels = (p_ptr->lev - p_ptr->class->spell_first) + 1;
	if (levels > 0)
	{
		msp = 1;
		msp += adj_mag_mana[p_ptr->state.stat_ind[p_ptr->class->spell_stat]] * levels / 100;
	}
	else
	{
		levels = 0;
		msp = 0;
	}

	/* Process gloves for those disturbed by them */
	if (player_has(PF_CUMBER_GLOVE))
	{
		bitflag f[OF_SIZE];

		/* Assume player is not encumbered by gloves */
		p_ptr->cumber_glove = FALSE;

		/* Get the gloves */
		o_ptr = &p_ptr->inventory[INVEN_HANDS];

		/* Examine the gloves */
		object_flags(o_ptr, f);

		/* Normal gloves hurt mage-type spells */
		if (o_ptr->kind &&
				!of_has(f, OF_FREE_ACT) && !of_has(f, OF_SPELLS_OK) &&
				!(of_has(f, OF_DEX) && (o_ptr->pval[which_pval(o_ptr, OF_DEX)] > 0)))
		{
			/* Encumbered */
			p_ptr->cumber_glove = TRUE;

			/* Reduce mana */
			msp = (3 * msp) / 4;
		}
	}

	/* Assume player not encumbered by armor */
	p_ptr->cumber_armor = FALSE;

	/* Weigh the armor */
	cur_wgt = 0;
	cur_wgt += p_ptr->inventory[INVEN_BODY].weight;
	cur_wgt += p_ptr->inventory[INVEN_HEAD].weight;
	cur_wgt += p_ptr->inventory[INVEN_ARM].weight;
	cur_wgt += p_ptr->inventory[INVEN_OUTER].weight;
	cur_wgt += p_ptr->inventory[INVEN_HANDS].weight;
	cur_wgt += p_ptr->inventory[INVEN_FEET].weight;

	/* Determine the weight allowance */
	max_wgt = p_ptr->class->spell_weight;

	/* Heavy armor penalizes mana */
	if (((cur_wgt - max_wgt) / 10) > 0)
	{
		/* Encumbered */
		p_ptr->cumber_armor = TRUE;

		/* Reduce mana */
		msp -= ((cur_wgt - max_wgt) / 10);
	}

	/* Mana can never be negative */
	if (msp < 0) msp = 0;

	/* Maximum mana has changed */
	if (p_ptr->msp != msp)
	{
		/* Save new limit */
		p_ptr->msp = msp;

		/* Enforce new limit */
		if (p_ptr->csp >= msp)
		{
			p_ptr->csp = msp;
			p_ptr->csp_frac = 0;
		}

		/* Display mana later */
		p_ptr->redraw |= (PR_MANA);
	}

	/* Hack -- handle "xtra" mode */
	if (character_xtra) return;

	/* Take note when "glove state" changes */
	if (old_cumber_glove != p_ptr->cumber_glove)
	{
		/* Message */
		if (p_ptr->cumber_glove)
		{
			msg("Your covered hands feel unsuitable for spellcasting.");
		}
		else
		{
			msg("Your hands feel more suitable for spellcasting.");
		}
	}

	/* Take note when "armor state" changes */
	if (old_cumber_armor != p_ptr->cumber_armor)
	{
		/* Message */
		if (p_ptr->cumber_armor)
		{
			msg("The weight of your armor encumbers your movement.");
		}
		else
		{
			msg("You feel able to move more freely.");
		}
	}
}


/*
 * Calculate the players (maximal) hit points
 *
 * Adjust current hitpoints if necessary
 */
static void calc_hitpoints(void)
{
	long bonus;
	int mhp;

	/* Get "1/100th hitpoint bonus per level" value */
	bonus = adj_con_mhp[p_ptr->state.stat_ind[A_CON]];

	/* Calculate hitpoints */
	mhp = p_ptr->player_hp[p_ptr->lev-1] + (bonus * p_ptr->lev / 100);

	/* Always have at least one hitpoint per level */
	if (mhp < p_ptr->lev + 1) mhp = p_ptr->lev + 1;

	/* New maximum hitpoints */
	if (p_ptr->mhp != mhp)
	{
		/* Save new limit */
		p_ptr->mhp = mhp;

		/* Enforce new limit */
		if (p_ptr->chp >= mhp)
		{
			p_ptr->chp = mhp;
			p_ptr->chp_frac = 0;
		}

		/* Display hitpoints (later) */
		p_ptr->redraw |= (PR_HP);
	}
}


/*
 * Calculate and set the current light radius.
 *
 * The brightest wielded object counts as the light source; radii do not add
 * up anymore.
 *
 * Note that a cursed light source no longer emits light.
 */
static void calc_torch(void)
{
	int i;

	s16b old_light = p_ptr->cur_light;
	bool burn_light = TRUE;

	s16b new_light = 0;
	int extra_light = 0;

	/* Ascertain lightness if in the town */
	if (!p_ptr->depth && ((turn % (10L * TOWN_DAWN)) < ((10L * TOWN_DAWN) / 2)))
		burn_light = FALSE;

	/* Examine all wielded objects, use the brightest */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		bitflag f[OF_SIZE];

		int amt = 0;
		object_type *o_ptr = &p_ptr->inventory[i];

		/* Skip empty slots */
		if (!o_ptr->kind) continue;

		/* Extract the flags */
		object_flags(o_ptr, f);

		/* Cursed objects emit no light */
		if (of_has(f, OF_LIGHT_CURSE))
			amt = 0;

		/* Examine actual lights */
		else if (o_ptr->tval == TV_LIGHT)
		{
			int flag_inc = of_has(f, OF_LIGHT) ? 1 : 0;

			/* Artifact lights provide permanent bright light */
			if (o_ptr->artifact)
				amt = 3 + flag_inc;

			/* Non-artifact lights and those without fuel provide no light */
			else if (!burn_light || o_ptr->timeout == 0)
				amt = 0;

			/* All lit lights provide at least radius 2 light */
			else
			{
				amt = 2 + flag_inc;

				/* Torches below half fuel provide less light */
				if (o_ptr->sval == SV_LIGHT_TORCH && o_ptr->timeout < (FUEL_TORCH / 4))
				    amt--;
			}
		}

		else
		{
			/* LIGHT flag on an non-cursed non-lights always increases radius */
			if (of_has(f, OF_LIGHT)) extra_light++;
		}

		/* Alter p_ptr->cur_light if reasonable */
		if (new_light < amt)
		    new_light = amt;
	}

	/* Add bonus from LIGHT flags */
	new_light += extra_light;

	/* Limit light */
	new_light = MIN(new_light, 5);
	new_light = MAX(new_light, 0);

	/* Notice changes in the "light radius" */
	if (old_light != new_light)
	{
		/* Update the visuals */
		p_ptr->cur_light = new_light;
		p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
	}
}

/*
 * Calculate the blows a player would get.
 *
 * \param o_ptr is the object for which we are calculating blows
 * \param state is the player state for which we are calculating blows
 * \param extra_blows is the number of +blows available from this object and
 * this state
 *
 * N.B. state->num_blows is now 100x the number of blows.
 */
int calc_blows(const object_type *o_ptr, player_state *state, int extra_blows)
{
	int blows;
	int str_index, dex_index;
	int div;
	int blow_energy;

	/* Enforce a minimum "weight" (tenth pounds) */
	div = ((o_ptr->weight < p_ptr->class->min_weight) ? p_ptr->class->min_weight :
		o_ptr->weight);

	/* Get the strength vs weight */
	str_index = adj_str_blow[state->stat_ind[A_STR]] *
			p_ptr->class->att_multiply / div;

	/* Maximal value */
	if (str_index > 11) str_index = 11;

	/* Index by dexterity */
	dex_index = MIN(adj_dex_blow[state->stat_ind[A_DEX]], 11);

	/* Use the blows table to get energy per blow */
	blow_energy = blows_table[str_index][dex_index];

	blows = MIN((10000 / blow_energy), (100 * p_ptr->class->max_attacks));

	/* Require at least one blow */
	return MAX(blows + (100 * extra_blows), 100);
}


/*
 * Computes current weight limit.
 */
static int weight_limit(player_state *state)
{
	int i;

	/* Weight limit based only on strength */
	i = adj_str_wgt[state->stat_ind[A_STR]] * 100;

	/* Return the result */
	return (i);
}


/*
 * Computes weight remaining before burdened.
 */
int weight_remaining(void)
{
	int i;

	/* Weight limit based only on strength */
	i = 60 * adj_str_wgt[p_ptr->state.stat_ind[A_STR]] - p_ptr->total_weight - 1;

	/* Return the result */
	return (i);
}


/*
 * Calculate the players current "state", taking into account
 * not only race/class intrinsics, but also objects being worn
 * and temporary spell effects.
 *
 * See also calc_mana() and calc_hitpoints().
 *
 * Take note of the new "speed code", in particular, a very strong
 * player will start slowing down as soon as he reaches 150 pounds,
 * but not until he reaches 450 pounds will he be half as fast as
 * a normal kobold.  This both hurts and helps the player, hurts
 * because in the old days a player could just avoid 300 pounds,
 * and helps because now carrying 300 pounds is not very painful.
 *
 * The "weapon" and "bow" do *not* add to the bonuses to hit or to
 * damage, since that would affect non-combat things.  These values
 * are actually added in later, at the appropriate place.
 *
 * If id_only is true, calc_bonuses() will only use the known
 * information of objects; thus it returns what the player _knows_
 * the character state to be.
 */
void calc_bonuses(object_type inventory[], player_state *state, bool id_only)
{
	int i, j, hold;

	int extra_blows = 0;
	int extra_shots = 0;
	int extra_might = 0;

	object_type *o_ptr;

	bitflag f[OF_SIZE];
	bitflag collect_f[OF_SIZE];

	/*** Reset ***/

	memset(state, 0, sizeof *state);

	/* Set various defaults */
	state->speed = 110;
	state->num_blows = 100;


	/*** Extract race/class info ***/

	/* Base infravision (purely racial) */
	state->see_infra = p_ptr->race->infra;

	/* Base skills */
	for (i = 0; i < SKILL_MAX; i++)
		state->skills[i] = p_ptr->race->r_skills[i] + p_ptr->class->c_skills[i];


	/*** Analyze player ***/

	/* Extract the player flags */
	player_flags(collect_f);


	/*** Analyze equipment ***/

	/* Scan the equipment */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->kind) continue;

		/* Extract the item flags */
		if (id_only)
			object_flags_known(o_ptr, f);
		else
			object_flags(o_ptr, f);

		of_union(collect_f, f);

		/* Affect stats */
		if (of_has(f, OF_STR)) state->stat_add[A_STR] +=
			o_ptr->pval[which_pval(o_ptr, OF_STR)];
		if (of_has(f, OF_INT)) state->stat_add[A_INT] +=
			o_ptr->pval[which_pval(o_ptr, OF_INT)];
		if (of_has(f, OF_WIS)) state->stat_add[A_WIS] +=
			o_ptr->pval[which_pval(o_ptr, OF_WIS)];
		if (of_has(f, OF_DEX)) state->stat_add[A_DEX] +=
			o_ptr->pval[which_pval(o_ptr, OF_DEX)];
		if (of_has(f, OF_CON)) state->stat_add[A_CON] +=
			o_ptr->pval[which_pval(o_ptr, OF_CON)];
		if (of_has(f, OF_CHR)) state->stat_add[A_CHR] +=
			o_ptr->pval[which_pval(o_ptr, OF_CHR)];

		/* Affect stealth */
		if (of_has(f, OF_STEALTH)) state->skills[SKILL_STEALTH] +=
			o_ptr->pval[which_pval(o_ptr, OF_STEALTH)];

		/* Affect searching ability (factor of five) */
		if (of_has(f, OF_SEARCH)) state->skills[SKILL_SEARCH] +=
			(o_ptr->pval[which_pval(o_ptr, OF_SEARCH)] * 5);

		/* Affect searching frequency (factor of five) */
		if (of_has(f, OF_SEARCH)) state->skills[SKILL_SEARCH_FREQUENCY]
			+= (o_ptr->pval[which_pval(o_ptr, OF_SEARCH)] * 5);

		/* Affect infravision */
		if (of_has(f, OF_INFRA)) state->see_infra +=
			o_ptr->pval[which_pval(o_ptr, OF_INFRA)];

		/* Affect digging (factor of 20) */
		if (of_has(f, OF_TUNNEL)) state->skills[SKILL_DIGGING] +=
			(o_ptr->pval[which_pval(o_ptr, OF_TUNNEL)] * 20);

		/* Affect speed */
		if (of_has(f, OF_SPEED)) state->speed +=
			o_ptr->pval[which_pval(o_ptr, OF_SPEED)];

		/* Affect blows */
		if (of_has(f, OF_BLOWS)) extra_blows +=
			o_ptr->pval[which_pval(o_ptr, OF_BLOWS)];

		/* Affect shots */
		if (of_has(f, OF_SHOTS)) extra_shots +=
			o_ptr->pval[which_pval(o_ptr, OF_SHOTS)];

		/* Affect Might */
		if (of_has(f, OF_MIGHT)) extra_might +=
			o_ptr->pval[which_pval(o_ptr, OF_MIGHT)];

		/* Modify the base armor class */
		state->ac += o_ptr->ac;

		/* The base armor class is always known */
		state->dis_ac += o_ptr->ac;

		/* Apply the bonuses to armor class */
		if (!id_only || object_is_known(o_ptr))
			state->to_a += o_ptr->to_a;

		/* Apply the mental bonuses to armor class, if known */
		if (object_defence_plusses_are_visible(o_ptr))
			state->dis_to_a += o_ptr->to_a;

		/* Hack -- do not apply "weapon" bonuses */
		if (i == INVEN_WIELD) continue;

		/* Hack -- do not apply "bow" bonuses */
		if (i == INVEN_BOW) continue;

		/* Apply the bonuses to hit/damage */
		if (!id_only || object_is_known(o_ptr))
		{
			state->to_h += o_ptr->to_h;
			state->to_d += o_ptr->to_d;
		}

		/* Apply the mental bonuses tp hit/damage, if known */
		if (object_attack_plusses_are_visible(o_ptr))
		{
			state->dis_to_h += o_ptr->to_h;
			state->dis_to_d += o_ptr->to_d;
		}
	}


	/*** Update all flags ***/

	for (i = 0; i < OF_MAX; i++)
		if (of_has(collect_f, i))
			of_on(state->flags, i);


	/*** Handle stats ***/

	/* Calculate stats */
	for (i = 0; i < A_MAX; i++)
	{
		int add, top, use, ind;

		/* Extract modifier */
		add = state->stat_add[i];

		/* Maximize mode */
		if (OPT(birth_maximize))
		{
			/* Modify the stats for race/class */
			add += (p_ptr->race->r_adj[i] + p_ptr->class->c_adj[i]);
		}

		/* Extract the new "stat_top" value for the stat */
		top = modify_stat_value(p_ptr->stat_max[i], add);

		/* Save the new value */
		state->stat_top[i] = top;

		/* Extract the new "stat_use" value for the stat */
		use = modify_stat_value(p_ptr->stat_cur[i], add);

		/* Save the new value */
		state->stat_use[i] = use;

		/* Values: n/a */
		if (use <= 3) ind = 0;

		/* Values: 3, 4, ..., 18 */
		else if (use <= 18) ind = (use - 3);

		/* Ranges: 18/00-18/09, ..., 18/210-18/219 */
		else if (use <= 18+219) ind = (15 + (use - 18) / 10);

		/* Range: 18/220+ */
		else ind = (37);

		assert((0 <= ind) && (ind < STAT_RANGE));

		/* Save the new index */
		state->stat_ind[i] = ind;
	}


	/*** Temporary flags ***/

	/* Apply temporary "stun" */
	if (p_ptr->timed[TMD_STUN] > 50)
	{
		state->to_h -= 20;
		state->dis_to_h -= 20;
		state->to_d -= 20;
		state->dis_to_d -= 20;
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE]
			* 8 / 10;
	}
	else if (p_ptr->timed[TMD_STUN])
	{
		state->to_h -= 5;
		state->dis_to_h -= 5;
		state->to_d -= 5;
		state->dis_to_d -= 5;
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE]
			* 9 / 10;
	}

	/* Invulnerability */
	if (p_ptr->timed[TMD_INVULN])
	{
		state->to_a += 100;
		state->dis_to_a += 100;
	}

	/* Temporary blessing */
	if (p_ptr->timed[TMD_BLESSED])
	{
		state->to_a += 5;
		state->dis_to_a += 5;
		state->to_h += 10;
		state->dis_to_h += 10;
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE]
			* 105 / 100;
	}

	/* Temporary shield */
	if (p_ptr->timed[TMD_SHIELD])
	{
		state->to_a += 50;
		state->dis_to_a += 50;
	}

	/* Temporary stoneskin */
	if (p_ptr->timed[TMD_STONESKIN])
	{
		state->to_a += 40;
		state->dis_to_a += 40;
		state->speed -= 5;
	}

	/* Temporary "Hero" */
	if (p_ptr->timed[TMD_HERO])
	{
		state->to_h += 12;
		state->dis_to_h += 12;
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE]
			* 105 / 100;
	}

	/* Temporary "Berserk" */
	if (p_ptr->timed[TMD_SHERO])
	{
		state->to_h += 24;
		state->dis_to_h += 24;
		state->to_a -= 10;
		state->dis_to_a -= 10;
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE]
			* 9 / 10;
	}

	/* Temporary "fast" */
	if (p_ptr->timed[TMD_FAST] || p_ptr->timed[TMD_SPRINT])
		state->speed += 10;

	/* Temporary "slow" */
	if (p_ptr->timed[TMD_SLOW])
		state->speed -= 10;

	/* Temporary infravision boost */
	if (p_ptr->timed[TMD_SINFRA])
		state->see_infra += 5;

	/* Terror - this is necessary because TMD_AFRAID already occupies the 
	 * of_ptr->timed slot for OF_AFRAID */
	if (p_ptr->timed[TMD_TERROR] > p_ptr->timed[TMD_AFRAID])
		p_ptr->timed[TMD_AFRAID] = p_ptr->timed[TMD_TERROR];

	if (p_ptr->timed[TMD_TERROR])
		state->speed += 5;

	/* Fear can come from item flags too */
	if (check_state(p_ptr, OF_AFRAID, p_ptr->state.flags))
	{
		state->to_h -= 20;
		state->dis_to_h -= 20;
		state->to_a += 8;
		state->dis_to_a += 8;
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE]
			* 95 / 100;
	}

	/* Confusion */
	if (p_ptr->timed[TMD_CONFUSED])
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE]
			* 75 / 100;

	/* Amnesia */
	if (p_ptr->timed[TMD_AMNESIA])
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE]
			* 8 / 10;

	/* Poison */
	if (p_ptr->timed[TMD_POISONED])
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE]
			* 95 / 100;

	/* Hallucination */
	if (p_ptr->timed[TMD_IMAGE])
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE]
			* 8 / 10;

	/*** Analyze weight ***/

	/* Extract the current weight (in tenth pounds) */
	j = p_ptr->total_weight;

	/* Extract the "weight limit" (in tenth pounds) */
	i = weight_limit(state);

	/* Apply "encumbrance" from weight */
	if (j > i / 2) state->speed -= ((j - (i / 2)) / (i / 10));

	/* Bloating slows the player down (a little) */
	if (p_ptr->food >= PY_FOOD_MAX) state->speed -= 10;

	/* Searching slows the player down */
	if (p_ptr->searching) state->speed -= 10;

	/* Sanity check on extreme speeds */
	if (state->speed < 0) state->speed = 0;
	if (state->speed > 199) state->speed = 199;

	/*** Apply modifier bonuses ***/

	/* Actual Modifier Bonuses (Un-inflate stat bonuses) */
	state->to_a += ((int)(adj_dex_ta[state->stat_ind[A_DEX]]) - 128);
	state->to_d += ((int)(adj_str_td[state->stat_ind[A_STR]]) - 128);
	state->to_h += ((int)(adj_dex_th[state->stat_ind[A_DEX]]) - 128);
	state->to_h += ((int)(adj_str_th[state->stat_ind[A_STR]]) - 128);

	/* Displayed Modifier Bonuses (Un-inflate stat bonuses) */
	state->dis_to_a += ((int)(adj_dex_ta[state->stat_ind[A_DEX]]) - 128);
	state->dis_to_d += ((int)(adj_str_td[state->stat_ind[A_STR]]) - 128);
	state->dis_to_h += ((int)(adj_dex_th[state->stat_ind[A_DEX]]) - 128);
	state->dis_to_h += ((int)(adj_str_th[state->stat_ind[A_STR]]) - 128);


	/*** Modify skills ***/

	/* Affect Skill -- disarming (DEX and INT) */
	state->skills[SKILL_DISARM] += adj_dex_dis[state->stat_ind[A_DEX]];
	state->skills[SKILL_DISARM] += adj_int_dis[state->stat_ind[A_INT]];

	/* Affect Skill -- magic devices (INT) */
	state->skills[SKILL_DEVICE] += adj_int_dev[state->stat_ind[A_INT]];

	/* Affect Skill -- saving throw (WIS) */
	state->skills[SKILL_SAVE] += adj_wis_sav[state->stat_ind[A_WIS]];

	/* Affect Skill -- digging (STR) */
	state->skills[SKILL_DIGGING] += adj_str_dig[state->stat_ind[A_STR]];

	/* Affect Skills (Level, by Class) */
	for (i = 0; i < SKILL_MAX; i++)
		state->skills[i] += (p_ptr->class->x_skills[i] * p_ptr->lev / 10);

	/* Limit Skill -- digging from 1 up */
	if (state->skills[SKILL_DIGGING] < 1) state->skills[SKILL_DIGGING] = 1;

	/* Limit Skill -- stealth from 0 to 30 */
	if (state->skills[SKILL_STEALTH] > 30) state->skills[SKILL_STEALTH] = 30;
	if (state->skills[SKILL_STEALTH] < 0) state->skills[SKILL_STEALTH] = 0;

	/* Apply Skill -- Extract noise from stealth */
	state->noise = (1L << (30 - state->skills[SKILL_STEALTH]));

	/* Obtain the "hold" value */
	hold = adj_str_hold[state->stat_ind[A_STR]];


	/*** Analyze current bow ***/

	/* Examine the "current bow" */
	o_ptr = &inventory[INVEN_BOW];

	/* Assume not heavy */
	state->heavy_shoot = FALSE;

	/* It is hard to hold a heavy bow */
	if (hold < o_ptr->weight / 10)
	{
		/* Hard to wield a heavy bow */
		state->to_h += 2 * (hold - o_ptr->weight / 10);
		state->dis_to_h += 2 * (hold - o_ptr->weight / 10);

		/* Heavy Bow */
		state->heavy_shoot = TRUE;
	}

	/* Analyze launcher */
	if (o_ptr->kind)
	{
		/* Get to shoot */
		state->num_shots = 1;

		/* Analyze the launcher */
		switch (o_ptr->sval)
		{
			/* Sling and ammo */
			case SV_SLING:
			{
				state->ammo_tval = TV_SHOT;
				state->ammo_mult = 2;
				break;
			}

			/* Short Bow and Arrow */
			case SV_SHORT_BOW:
			{
				state->ammo_tval = TV_ARROW;
				state->ammo_mult = 2;
				break;
			}

			/* Long Bow and Arrow */
			case SV_LONG_BOW:
			{
				state->ammo_tval = TV_ARROW;
				state->ammo_mult = 3;
				break;
			}

			/* Light Crossbow and Bolt */
			case SV_LIGHT_XBOW:
			{
				state->ammo_tval = TV_BOLT;
				state->ammo_mult = 3;
				break;
			}

			/* Heavy Crossbow and Bolt */
			case SV_HEAVY_XBOW:
			{
				state->ammo_tval = TV_BOLT;
				state->ammo_mult = 4;
				break;
			}
		}

		/* Apply special flags */
		if (o_ptr->kind && !state->heavy_shoot)
		{
			/* Extra shots */
			state->num_shots += extra_shots;

			/* Extra might */
			state->ammo_mult += extra_might;

			/* Hack -- Rangers love Bows */
			if (player_has(PF_EXTRA_SHOT) &&
			    (state->ammo_tval == TV_ARROW))
			{
				/* Extra shot at level 20 */
				if (p_ptr->lev >= 20) state->num_shots++;

				/* Extra shot at level 40 */
				if (p_ptr->lev >= 40) state->num_shots++;
			}
		}

		/* Require at least one shot */
		if (state->num_shots < 1) state->num_shots = 1;
	}


	/*** Analyze weapon ***/

	/* Examine the "current weapon" */
	o_ptr = &inventory[INVEN_WIELD];

	/* Assume not heavy */
	state->heavy_wield = FALSE;

	/* It is hard to hold a heavy weapon */
	if (hold < o_ptr->weight / 10)
	{
		/* Hard to wield a heavy weapon */
		state->to_h += 2 * (hold - o_ptr->weight / 10);
		state->dis_to_h += 2 * (hold - o_ptr->weight / 10);

		/* Heavy weapon */
		state->heavy_wield = TRUE;
	}

	/* Non-object means barehanded attacks */
	if (!o_ptr->kind)
		assert(o_ptr->weight == 0);

	/* Normal weapons */
	if (!state->heavy_wield)
	{
		/* Calculate number of blows */
		state->num_blows = calc_blows(o_ptr, state, extra_blows);

		/* Boost digging skill by weapon weight */
		state->skills[SKILL_DIGGING] += (o_ptr->weight / 10);
	}

	/* Assume okay */
	state->icky_wield = FALSE;

	/* Priest weapon penalty for non-blessed edged weapons */
	if (player_has(PF_BLESS_WEAPON) && !check_state(p_ptr, OF_BLESSED, p_ptr->state.flags) &&
		((o_ptr->tval == TV_SWORD) || (o_ptr->tval == TV_POLEARM)))
	{
		/* Reduce the real bonuses */
		state->to_h -= 2;
		state->to_d -= 2;

		/* Reduce the mental bonuses */
		state->dis_to_h -= 2;
		state->dis_to_d -= 2;

		/* Icky weapon */
		state->icky_wield = TRUE;
	}

	return;
}

/*
 * Calculate bonuses, and print various things on changes.
 */
static void update_bonuses(void)
{
	int i;

	player_state *state = &p_ptr->state;
	player_state old = p_ptr->state;


	/*** Calculate bonuses ***/

	calc_bonuses(p_ptr->inventory, &p_ptr->state, FALSE);


	/*** Notice changes ***/

	/* Analyze stats */
	for (i = 0; i < A_MAX; i++)
	{
		/* Notice changes */
		if (state->stat_top[i] != old.stat_top[i])
		{
			/* Redisplay the stats later */
			p_ptr->redraw |= (PR_STATS);
		}

		/* Notice changes */
		if (state->stat_use[i] != old.stat_use[i])
		{
			/* Redisplay the stats later */
			p_ptr->redraw |= (PR_STATS);
		}

		/* Notice changes */
		if (state->stat_ind[i] != old.stat_ind[i])
		{
			/* Change in CON affects Hitpoints */
			if (i == A_CON)
			{
				p_ptr->update |= (PU_HP);
			}

			/* Change in INT may affect Mana/Spells */
			else if (i == A_INT)
			{
				if (p_ptr->class->spell_stat == A_INT)
				{
					p_ptr->update |= (PU_MANA | PU_SPELLS);
				}
			}

			/* Change in WIS may affect Mana/Spells */
			else if (i == A_WIS)
			{
				if (p_ptr->class->spell_stat == A_WIS)
				{
					p_ptr->update |= (PU_MANA | PU_SPELLS);
				}
			}
		}
	}


	/* Hack -- Telepathy Change */
	if (of_has(state->flags, OF_TELEPATHY) != of_has(old.flags, OF_TELEPATHY))
	{
		/* Update monster visibility */
		p_ptr->update |= (PU_MONSTERS);
	}

	/* Hack -- See Invis Change */
	if (of_has(state->flags, OF_SEE_INVIS) != of_has(old.flags, OF_SEE_INVIS))
	{
		/* Update monster visibility */
		p_ptr->update |= (PU_MONSTERS);
	}

	/* Redraw speed (if needed) */
	if (state->speed != old.speed)
	{
		/* Redraw speed */
		p_ptr->redraw |= (PR_SPEED);
	}

	/* Redraw armor (if needed) */
	if ((state->dis_ac != old.dis_ac) || (state->dis_to_a != old.dis_to_a))
	{
		/* Redraw */
		p_ptr->redraw |= (PR_ARMOR);
	}

	/* Hack -- handle "xtra" mode */
	if (character_xtra) return;

	/* Take note when "heavy bow" changes */
	if (old.heavy_shoot != state->heavy_shoot)
	{
		/* Message */
		if (state->heavy_shoot)
			msg("You have trouble wielding such a heavy bow.");
		else if (p_ptr->inventory[INVEN_BOW].kind)
			msg("You have no trouble wielding your bow.");
		else
			msg("You feel relieved to put down your heavy bow.");
	}

	/* Take note when "heavy weapon" changes */
	if (old.heavy_wield != state->heavy_wield)
	{
		/* Message */
		if (state->heavy_wield)
			msg("You have trouble wielding such a heavy weapon.");
		else if (p_ptr->inventory[INVEN_WIELD].kind)
			msg("You have no trouble wielding your weapon.");
		else
			msg("You feel relieved to put down your heavy weapon.");	
	}

	/* Take note when "illegal weapon" changes */
	if (old.icky_wield != state->icky_wield)
	{
		/* Message */
		if (state->icky_wield)
			msg("You do not feel comfortable with your weapon.");
		else if (p_ptr->inventory[INVEN_WIELD].kind)
			msg("You feel comfortable with your weapon.");
		else
			msg("You feel more comfortable after removing your weapon.");
	}
}




/*** Generic "deal with" functions ***/

/*
 * Handle "p_ptr->notice"
 */
void notice_stuff(struct player *p)
{
	/* Notice stuff */
	if (!p->notice) return;


	/* Deal with autoinscribe stuff */
	if (p->notice & PN_AUTOINSCRIBE)
	{
		p->notice &= ~(PN_AUTOINSCRIBE);
		autoinscribe_pack();
		autoinscribe_ground();
	}

	/* Deal with squelch stuff */
	if (p->notice & PN_SQUELCH)
	{
		p->notice &= ~(PN_SQUELCH);
		squelch_drop();
	}

	/* Combine the pack */
	if (p->notice & PN_COMBINE)
	{
		p->notice &= ~(PN_COMBINE);
		combine_pack();
	}

	/* Reorder the pack */
	if (p->notice & PN_REORDER)
	{
		p->notice &= ~(PN_REORDER);
		reorder_pack();
	}

	/* Sort the quiver */
	if (p->notice & PN_SORT_QUIVER)
	{
		p->notice &= ~(PN_SORT_QUIVER);
		sort_quiver();
	}

	/* Dump the monster messages */
	if (p->notice & PN_MON_MESSAGE)
	{
		p->notice &= ~(PN_MON_MESSAGE);

		/* Make sure this comes after all of the monster messages */
		flush_all_monster_messages();
	}
}

/*
 * Handle "p_ptr->update"
 */
void update_stuff(struct player *p)
{
	/* Update stuff */
	if (!p->update) return;


	if (p->update & (PU_BONUS))
	{
		p->update &= ~(PU_BONUS);
		update_bonuses();
	}

	if (p->update & (PU_TORCH))
	{
		p->update &= ~(PU_TORCH);
		calc_torch();
	}

	if (p->update & (PU_HP))
	{
		p->update &= ~(PU_HP);
		calc_hitpoints();
	}

	if (p->update & (PU_MANA))
	{
		p->update &= ~(PU_MANA);
		calc_mana();
	}

	if (p->update & (PU_SPELLS))
	{
		p->update &= ~(PU_SPELLS);
		calc_spells();
	}


	/* Character is not ready yet, no screen updates */
	if (!character_generated) return;


	/* Character is in "icky" mode, no screen updates */
	if (character_icky) return;


	if (p->update & (PU_FORGET_VIEW))
	{
		p->update &= ~(PU_FORGET_VIEW);
		forget_view();
	}

	if (p->update & (PU_UPDATE_VIEW))
	{
		p->update &= ~(PU_UPDATE_VIEW);
		update_view();
	}


	if (p->update & (PU_FORGET_FLOW))
	{
		p->update &= ~(PU_FORGET_FLOW);
		cave_forget_flow(cave);
	}

	if (p->update & (PU_UPDATE_FLOW))
	{
		p->update &= ~(PU_UPDATE_FLOW);
		cave_update_flow(cave);
	}


	if (p->update & (PU_DISTANCE))
	{
		p->update &= ~(PU_DISTANCE);
		p->update &= ~(PU_MONSTERS);
		update_monsters(TRUE);
	}

	if (p->update & (PU_MONSTERS))
	{
		p->update &= ~(PU_MONSTERS);
		update_monsters(FALSE);
	}


	if (p->update & (PU_PANEL))
	{
		p->update &= ~(PU_PANEL);
		event_signal(EVENT_PLAYERMOVED);
	}
}



struct flag_event_trigger
{
	u32b flag;
	game_event_type event;
};



/*
 * Events triggered by the various flags.
 */
static const struct flag_event_trigger redraw_events[] =
{
	{ PR_MISC,    EVENT_RACE_CLASS },
	{ PR_TITLE,   EVENT_PLAYERTITLE },
	{ PR_LEV,     EVENT_PLAYERLEVEL },
	{ PR_EXP,     EVENT_EXPERIENCE },
	{ PR_STATS,   EVENT_STATS },
	{ PR_ARMOR,   EVENT_AC },
	{ PR_HP,      EVENT_HP },
	{ PR_MANA,    EVENT_MANA },
	{ PR_GOLD,    EVENT_GOLD },
	{ PR_HEALTH,  EVENT_MONSTERHEALTH },
	{ PR_DEPTH,   EVENT_DUNGEONLEVEL },
	{ PR_SPEED,   EVENT_PLAYERSPEED },
	{ PR_STATE,   EVENT_STATE },
	{ PR_STATUS,  EVENT_STATUS },
	{ PR_STUDY,   EVENT_STUDYSTATUS },
	{ PR_DTRAP,   EVENT_DETECTIONSTATUS },
	{ PR_BUTTONS, EVENT_MOUSEBUTTONS },

	{ PR_INVEN,   EVENT_INVENTORY },
	{ PR_EQUIP,   EVENT_EQUIPMENT },
	{ PR_MONLIST, EVENT_MONSTERLIST },
	{ PR_ITEMLIST, EVENT_ITEMLIST },
	{ PR_MONSTER, EVENT_MONSTERTARGET },
	{ PR_OBJECT, EVENT_OBJECTTARGET },
	{ PR_MESSAGE, EVENT_MESSAGE },
};

/*
 * Handle "p_ptr->redraw"
 */
void redraw_stuff(struct player *p)
{
	size_t i;

	/* Redraw stuff */
	if (!p->redraw) return;

	/* Character is not ready yet, no screen updates */
	if (!character_generated) return;

	/* Character is in "icky" mode, no screen updates */
	if (character_icky) return;

	/* For each listed flag, send the appropriate signal to the UI */
	for (i = 0; i < N_ELEMENTS(redraw_events); i++)
	{
		const struct flag_event_trigger *hnd = &redraw_events[i];

		if (p->redraw & hnd->flag)
			event_signal(hnd->event);
	}

	/* Then the ones that require parameters to be supplied. */
	if (p->redraw & PR_MAP)
	{
		/* Mark the whole map to be redrawn */
		event_signal_point(EVENT_MAP, -1, -1);
	}

	p->redraw = 0;

	/*
	 * Do any plotting, etc. delayed from earlier - this set of updates
	 * is over.
	 */
	event_signal(EVENT_END);
}


/*
 * Handle "p_ptr->update" and "p_ptr->redraw"
 */
void handle_stuff(struct player *p)
{
	if (p->update) update_stuff(p);
	if (p->redraw) redraw_stuff(p);
}

