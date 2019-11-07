/**
 * \file player-calcs.c
 * \brief Player status calculation, signalling ui events based on 
 *	status changes.
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2014 Nick McConnell
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
#include "game-event.h"
#include "game-input.h"
#include "game-world.h"
#include "init.h"
#include "mon-msg.h"
#include "mon-util.h"
#include "obj-curse.h"
#include "obj-gear.h"
#include "obj-ignore.h"
#include "obj-knowledge.h"
#include "obj-power.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-spell.h"
#include "player-timed.h"
#include "player-util.h"

/**
 * Stat Table (INT) -- Magic devices
 */
static const int adj_int_dev[STAT_RANGE] =
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

/**
 * Stat Table (WIS) -- Saving throw
 */
static const int adj_wis_sav[STAT_RANGE] =
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


/**
 * Stat Table (DEX) -- disarming
 */
static const int adj_dex_dis[STAT_RANGE] =
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


/**
 * Stat Table (INT) -- disarming
 */
static const int adj_int_dis[STAT_RANGE] =
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

/**
 * Stat Table (DEX) -- bonus to ac
 */
static const int adj_dex_ta[STAT_RANGE] =
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
const int adj_str_td[STAT_RANGE] =
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
const int adj_dex_th[STAT_RANGE] =
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
static const int adj_str_th[STAT_RANGE] =
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


/**
 * Stat Table (STR) -- weight limit in deca-pounds
 */
static const int adj_str_wgt[STAT_RANGE] =
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


/**
 * Stat Table (STR) -- weapon weight limit in pounds
 */
const int adj_str_hold[STAT_RANGE] =
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


/**
 * Stat Table (STR) -- digging value
 */
static const int adj_str_dig[STAT_RANGE] =
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


/**
 * Stat Table (STR) -- help index into the "blow" table
 */
const int adj_str_blow[STAT_RANGE] =
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


/**
 * Stat Table (DEX) -- index into the "blow" table
 */
static const int adj_dex_blow[STAT_RANGE] =
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


/**
 * Stat Table (DEX) -- chance of avoiding "theft" and "falling"
 */
const int adj_dex_safe[STAT_RANGE] =
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


/**
 * Stat Table (CON) -- base regeneration rate
 */
const int adj_con_fix[STAT_RANGE] =
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


/**
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

/**
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

/**
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
 * (all specified in class.txt now)
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
static const int blows_table[12][12] =
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

/**
 * Decide which object comes earlier in the standard inventory listing,
 * defaulting to the first if nothing separates them.
 *
 * \return whether to replace the original object with the new one
 */
bool earlier_object(struct object *orig, struct object *new, bool store)
{
	/* Check we have actual objects */
	if (!new) return false;
	if (!orig) return true;

	if (!store) {
		/* Readable books always come first */
		if (obj_can_browse(orig) && !obj_can_browse(new)) return false;
		if (!obj_can_browse(orig) && obj_can_browse(new)) return true;
	}

	/* Objects sort by decreasing type */
	if (orig->tval > new->tval) return false;
	if (orig->tval < new->tval) return true;

	if (!store) {
		/* Non-aware (flavored) items always come last (default to orig) */
		if (!object_flavor_is_aware(new)) return false;
		if (!object_flavor_is_aware(orig)) return true;
	}

	/* Objects sort by increasing sval */
	if (orig->sval < new->sval) return false;
	if (orig->sval > new->sval) return true;

	if (!store) {
		/* Unaware objects always come last (default to orig) */
		if (new->kind->flavor && !object_flavor_is_aware(new)) return false;
		if (orig->kind->flavor && !object_flavor_is_aware(orig)) return true;

		/* Lights sort by decreasing fuel */
		if (tval_is_light(orig)) {
			if (orig->pval > new->pval) return false;
			if (orig->pval < new->pval) return true;
		}
	}

	/* Objects sort by decreasing value, except ammo */
	if (tval_is_ammo(orig)) {
		if (object_value(orig, 1) <
			object_value(new, 1))
			return false;
		if (object_value(orig, 1) >
			object_value(new, 1))
			return true;
	} else {
		if (object_value(orig, 1) >
			object_value(new, 1))
			return false;
		if (object_value(orig, 1) <
			object_value(new, 1))
			return true;
	}

	/* No preference */
	return false;
}

int equipped_item_slot(struct player_body body, struct object *item)
{
	int i;

	if (item == NULL) return body.count;

	/* Look for an equipment slot with this item */
	for (i = 0; i < body.count; i++)
		if (item == body.slots[i].obj) break;

	/* Correct slot, or body.count if not equipped */
	return i;
}

/**
 * Put the player's inventory and quiver into easily accessible arrays.  The
 * pack may be overfull by one item
 */
void calc_inventory(struct player_upkeep *upkeep, struct object *gear,
					struct player_body body)
{
	int i;
	int old_inven_cnt = upkeep->inven_cnt;
	struct object **old_quiver = mem_zalloc(z_info->quiver_size *
												sizeof(struct object *));
	struct object **old_pack = mem_zalloc(z_info->pack_size *
											  sizeof(struct object *));

	/* Prepare to fill the quiver */
	upkeep->quiver_cnt = 0;

	/* Copy the current quiver */
	for (i = 0; i < z_info->quiver_size; i++)
		if (upkeep->quiver[i])
			old_quiver[i] = upkeep->quiver[i];
		else
			old_quiver[i] = NULL;

	/* First, allocate inscribed items */
	for (i = 0; i < z_info->quiver_size; i++) {
		struct object *current;

		/* Start with an empty slot */
		upkeep->quiver[i] = NULL;

		/* Find the first quiver object with the correct label */
		for (current = gear; current; current = current->next) {
			/* Ignore non-ammo */
			if (!tval_is_ammo(current)) continue;

			/* Allocate inscribed objects if it's the right slot */
			if (current->note) {
				const char *s = strchr(quark_str(current->note), '@');
				if (s && s[1] == 'f') {
					int choice = s[2] - '0';

					/* Correct slot, fill it straight away */
					if (choice == i) {
						upkeep->quiver[i] = current;
						upkeep->quiver_cnt += current->number;

						/* In the quiver counts as worn */
						object_learn_on_wield(player, current);

						/* Done with this slot */
						break;
					}
				}
			}
		}
	}

	/* Now fill the rest of the slots in order */
	for (i = 0; i < z_info->quiver_size; i++) {
		struct object *current, *first = NULL;
		int j;

		/* If the slot is full, move on */
		if (upkeep->quiver[i]) continue;

		/* Find the first quiver object not yet allocated */
		for (current = gear; current; current = current->next) {
			bool already = false;

			/* Ignore non-ammo */
			if (!tval_is_ammo(current)) continue;

			/* Ignore stuff already quivered */
			for (j = 0; j < z_info->quiver_size; j++)
				if (upkeep->quiver[j] == current)
					already = true;
			if (already) continue;

			/* Choose the first in order */
			if (earlier_object(first, current, false)) {
				first = current;
			}
		}

		/* Stop looking if there's nothing left */
		if (!first) break;

		/* If we have an item, slot it */
		upkeep->quiver[i] = first;
		upkeep->quiver_cnt += first->number;

		/* In the quiver counts as worn */
		object_learn_on_wield(player, first);
	}

	/* Note reordering */
	if (character_dungeon)
		for (i = 0; i < z_info->quiver_size; i++)
			if (old_quiver[i] && (upkeep->quiver[i] != old_quiver[i])) {
				msg("You re-arrange your quiver.");
				break;
			}

	/* Copy the current pack */
	for (i = 0; i < z_info->pack_size; i++)
		old_pack[i] = upkeep->inven[i];

	/* Prepare to fill the inventory */
	upkeep->inven_cnt = 0;

	for (i = 0; i <= z_info->pack_size; i++) {
		struct object *current, *first = NULL;
		for (current = gear; current; current = current->next) {
			bool possible = true;
			int j;

			/* Skip equipment */
			if (object_is_equipped(body, current))
				possible = false;

			/* Skip quivered objects */
			for (j = 0; j < z_info->quiver_size; j++)
				if (upkeep->quiver[j] == current)
					possible = false;

			/* Skip objects already allocated to the inventory */
			for (j = 0; j < upkeep->inven_cnt; j++)
				if (upkeep->inven[j] == current)
					possible = false;

			/* If still possible, choose the first in order */
			if (!possible)
				continue;
			else if (earlier_object(first, current, false)) {
				first = current;
			}
		}

		/* Allocate */
		upkeep->inven[i] = first;
		if (first)
			upkeep->inven_cnt++;
	}

	/* Note reordering */
	if (character_dungeon && (upkeep->inven_cnt == old_inven_cnt))
		for (i = 0; i < z_info->pack_size; i++)
			if (old_pack[i] && (upkeep->inven[i] != old_pack[i]) &&
				!object_is_equipped(body, old_pack[i])) {
				msg("You re-arrange your pack.");
				break;
			}

	mem_free(old_quiver);
	mem_free(old_pack);
}

static void update_inventory(struct player *p)
{
	calc_inventory(p->upkeep, p->gear, p->body);
}

/**
 * Average of the player's spell stats across all the realms they can cast
 * from, rounded up
 *
 * If the player can only cast from a single realm, this is simple the stat
 * for that realm
 */
static int average_spell_stat(struct player *p, struct player_state *state)
{
	int i, count, sum = 0;
	struct magic_realm *realm = class_magic_realms(p->class, &count), *r_next;

	for (i = count; i > 0; i--) {
		sum += state->stat_ind[realm->stat];
		r_next = realm->next;
		mem_free(realm);
		realm = r_next;
	}
	return (sum + count - 1) / count;
}

/**
 * Calculate number of spells player should have, and forget,
 * or remember, spells until that number is properly reflected.
 *
 * Note that this function induces various "status" messages,
 * which must be bypasses until the character is created.
 */
static void calc_spells(struct player *p)
{
	int i, j, k, levels;
	int num_allowed, num_known, num_total = p->class->magic.total_spells;
	int percent_spells;

	const struct class_spell *spell;

	s16b old_spells;

	/* Hack -- must be literate */
	if (!p->class->magic.total_spells) return;

	/* Hack -- wait for creation */
	if (!character_generated) return;

	/* Hack -- handle partial mode */
	if (p->upkeep->only_partial) return;

	/* Save the new_spells value */
	old_spells = p->upkeep->new_spells;

	/* Determine the number of spells allowed */
	levels = p->lev - p->class->magic.spell_first + 1;

	/* Hack -- no negative spells */
	if (levels < 0) levels = 0;

	/* Number of 1/100 spells per level (or something - needs clarifying) */
	percent_spells = adj_mag_study[average_spell_stat(p, &p->state)];

	/* Extract total allowed spells (rounded up) */
	num_allowed = (((percent_spells * levels) + 50) / 100);

	/* Assume none known */
	num_known = 0;

	/* Count the number of spells we know */
	for (j = 0; j < num_total; j++)
		if (p->spell_flags[j] & PY_SPELL_LEARNED)
			num_known++;

	/* See how many spells we must forget or may learn */
	p->upkeep->new_spells = num_allowed - num_known;

	/* Forget spells which are too hard */
	for (i = num_total - 1; i >= 0; i--) {
		/* Get the spell */
		j = p->spell_order[i];

		/* Skip non-spells */
		if (j >= 99) continue;

		/* Get the spell */
		spell = spell_by_index(j);

		/* Skip spells we are allowed to know */
		if (spell->slevel <= p->lev) continue;

		/* Is it known? */
		if (p->spell_flags[j] & PY_SPELL_LEARNED) {
			/* Mark as forgotten */
			p->spell_flags[j] |= PY_SPELL_FORGOTTEN;

			/* No longer known */
			p->spell_flags[j] &= ~PY_SPELL_LEARNED;

			/* Message */
			msg("You have forgotten the %s of %s.", spell->realm->spell_noun,
				spell->name);

			/* One more can be learned */
			p->upkeep->new_spells++;
		}
	}

	/* Forget spells if we know too many spells */
	for (i = num_total - 1; i >= 0; i--) {
		/* Stop when possible */
		if (p->upkeep->new_spells >= 0) break;

		/* Get the (i+1)th spell learned */
		j = p->spell_order[i];

		/* Skip unknown spells */
		if (j >= 99) continue;

		/* Get the spell */
		spell = spell_by_index(j);

		/* Forget it (if learned) */
		if (p->spell_flags[j] & PY_SPELL_LEARNED) {
			/* Mark as forgotten */
			p->spell_flags[j] |= PY_SPELL_FORGOTTEN;

			/* No longer known */
			p->spell_flags[j] &= ~PY_SPELL_LEARNED;

			/* Message */
			msg("You have forgotten the %s of %s.", spell->realm->spell_noun,
				spell->name);

			/* One more can be learned */
			p->upkeep->new_spells++;
		}
	}

	/* Check for spells to remember */
	for (i = 0; i < num_total; i++) {
		/* None left to remember */
		if (p->upkeep->new_spells <= 0) break;

		/* Get the next spell we learned */
		j = p->spell_order[i];

		/* Skip unknown spells */
		if (j >= 99) break;

		/* Get the spell */
		spell = spell_by_index(j);

		/* Skip spells we cannot remember */
		if (spell->slevel > p->lev) continue;

		/* First set of spells */
		if (p->spell_flags[j] & PY_SPELL_FORGOTTEN) {
			/* No longer forgotten */
			p->spell_flags[j] &= ~PY_SPELL_FORGOTTEN;

			/* Known once more */
			p->spell_flags[j] |= PY_SPELL_LEARNED;

			/* Message */
			msg("You have remembered the %s of %s.", spell->realm->spell_noun,
				spell->name);

			/* One less can be learned */
			p->upkeep->new_spells--;
		}
	}

	/* Assume no spells available */
	k = 0;

	/* Count spells that can be learned */
	for (j = 0; j < num_total; j++) {
		/* Get the spell */
		spell = spell_by_index(j);

		/* Skip spells we cannot remember or don't exist */
		if (!spell) continue;
		if (spell->slevel > p->lev || spell->slevel == 0) continue;

		/* Skip spells we already know */
		if (p->spell_flags[j] & PY_SPELL_LEARNED)
			continue;

		/* Count it */
		k++;
	}

	/* Cannot learn more spells than exist */
	if (p->upkeep->new_spells > k) p->upkeep->new_spells = k;

	/* Spell count changed */
	if (old_spells != p->upkeep->new_spells) {
		/* Message if needed */
		if (p->upkeep->new_spells) {
			int count;
			struct magic_realm *r = class_magic_realms(p->class, &count), *r1;
			char buf[120];

			my_strcpy(buf, r->spell_noun, sizeof(buf));
			if (p->upkeep->new_spells > 1) {
				my_strcat(buf, "s", sizeof(buf));
			}
			r1 = r->next;
			mem_free(r);
			r = r1;
			if (count > 1) {
				while (r) {
					count--;
					if (count) {
						my_strcat(buf, ", ", sizeof(buf));
					} else {
						my_strcat(buf, " or ", sizeof(buf));
					}
					my_strcat(buf, r->spell_noun, sizeof(buf));
					if (p->upkeep->new_spells > 1) {
						my_strcat(buf, "s", sizeof(buf));
					}
					r1 = r->next;
					mem_free(r);
					r = r1;
				}
			}
			/* Message */
			msg("You can learn %d more %s.", p->upkeep->new_spells, buf);
		}

		/* Redraw Study Status */
		p->upkeep->redraw |= (PR_STUDY | PR_OBJECT);
	}
}


/**
 * Calculate maximum mana.  You do not need to know any spells.
 * Note that mana is lowered by heavy (or inappropriate) armor.
 *
 * This function induces status messages.
 */
static void calc_mana(struct player *p, struct player_state *state, bool update)
{
	int i, msp, levels, cur_wgt, max_wgt; 

	/* Must be literate */
	if (!p->class->magic.total_spells) {
		p->msp = 0;
		p->csp = 0;
		p->csp_frac = 0;
		return;
	}

	/* Extract "effective" player level */
	levels = (p->lev - p->class->magic.spell_first) + 1;
	if (levels > 0) {
		msp = 1;
		msp += adj_mag_mana[average_spell_stat(p, state)] * levels / 100;
	} else {
		levels = 0;
		msp = 0;
	}

	/* Assume player not encumbered by armor */
	state->cumber_armor = false;

	/* Weigh the armor */
	cur_wgt = 0;
	for (i = 0; i < p->body.count; i++) {
		struct object *obj_local = slot_object(p, i);

		/* Ignore non-armor */
		if (slot_type_is(i, EQUIP_WEAPON)) continue;
		if (slot_type_is(i, EQUIP_BOW)) continue;
		if (slot_type_is(i, EQUIP_RING)) continue;
		if (slot_type_is(i, EQUIP_AMULET)) continue;
		if (slot_type_is(i, EQUIP_LIGHT)) continue;

		/* Add weight */
		if (obj_local)
			cur_wgt += obj_local->weight;
	}

	/* Determine the weight allowance */
	max_wgt = p->class->magic.spell_weight;

	/* Heavy armor penalizes mana */
	if (((cur_wgt - max_wgt) / 10) > 0) {
		/* Encumbered */
		state->cumber_armor = true;

		/* Reduce mana */
		msp -= ((cur_wgt - max_wgt) / 10);
	}

	/* Mana can never be negative */
	if (msp < 0) msp = 0;

	/* Return if no updates */
	if (!update) return;

	/* Maximum mana has changed */
	if (p->msp != msp) {
		/* Save new limit */
		p->msp = msp;

		/* Enforce new limit */
		if (p->csp >= msp) {
			p->csp = msp;
			p->csp_frac = 0;
		}

		/* Display mana later */
		p->upkeep->redraw |= (PR_MANA);
	}
}


/**
 * Calculate the players (maximal) hit points
 *
 * Adjust current hitpoints if necessary
 */
static void calc_hitpoints(struct player *p)
{
	long bonus;
	int mhp;

	/* Get "1/100th hitpoint bonus per level" value */
	bonus = adj_con_mhp[p->state.stat_ind[STAT_CON]];

	/* Calculate hitpoints */
	mhp = p->player_hp[p->lev-1] + (bonus * p->lev / 100);

	/* Always have at least one hitpoint per level */
	if (mhp < p->lev + 1) mhp = p->lev + 1;

	/* New maximum hitpoints */
	if (p->mhp != mhp) {
		/* Save new limit */
		p->mhp = mhp;

		/* Enforce new limit */
		if (p->chp >= mhp) {
			p->chp = mhp;
			p->chp_frac = 0;
		}

		/* Display hitpoints (later) */
		p->upkeep->redraw |= (PR_HP);
	}
}


/**
 * Calculate and set the current light radius.
 *
 * The light radius will be the total of all lights carried.
 */
static void calc_light(struct player *p, struct player_state *state,
					   bool update)
{
	int i;

	/* Assume no light */
	state->cur_light = 0;

	/* Ascertain lightness if in the town */
	if (!p->depth && is_daytime() && update) {
		/* Update the visuals if necessary*/
		if (p->state.cur_light != state->cur_light)
			p->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

		return;
	}

	/* Examine all wielded objects, use the brightest */
	for (i = 0; i < p->body.count; i++) {
		int amt = 0;
		struct object *obj = slot_object(p, i);

		/* Skip empty slots */
		if (!obj) continue;

		/* Light radius - innate plus modifier */
		if (of_has(obj->flags, OF_LIGHT_2))
			amt = 2;
		else if (of_has(obj->flags, OF_LIGHT_3))
			amt = 3;
		amt += obj->modifiers[OBJ_MOD_LIGHT];

		/* Examine actual lights */
		if (tval_is_light(obj) && !of_has(obj->flags, OF_NO_FUEL) &&
				obj->timeout == 0)
			/* Lights without fuel provide no light */
			amt = 0;

		/* Alter p->state.cur_light if reasonable */
	    state->cur_light += amt;
	}
}

/**
 * Populates `chances` with the player's chance of digging through
 * the diggable terrain types in one turn out of 1600.
 */
void calc_digging_chances(struct player_state *state, int chances[DIGGING_MAX])
{
	int i;

	chances[DIGGING_RUBBLE] = state->skills[SKILL_DIGGING] * 8;
	chances[DIGGING_MAGMA] = (state->skills[SKILL_DIGGING] - 10) * 4;
	chances[DIGGING_QUARTZ] = (state->skills[SKILL_DIGGING] - 20) * 2;
	chances[DIGGING_GRANITE] = (state->skills[SKILL_DIGGING] - 40) * 1;
	/* Approximate a 1/1200 chance per skill point over 30 */
	chances[DIGGING_DOORS] = (state->skills[SKILL_DIGGING] * 4 - 119) / 3;

	/* Don't let any negative chances through */
	for (i = 0; i < DIGGING_MAX; i++)
		chances[i] = MAX(0, chances[i]);
}

/**
 * Calculate the blows a player would get.
 *
 * \param obj is the object for which we are calculating blows
 * \param state is the player state for which we are calculating blows
 * \param extra_blows is the number of +blows available from this object and
 * this state
 *
 * N.B. state->num_blows is now 100x the number of blows.
 */
int calc_blows(struct player *p, const struct object *obj,
			   struct player_state *state, int extra_blows)
{
	int blows;
	int str_index, dex_index;
	int div;
	int blow_energy;

	int weight = (obj == NULL) ? 0 : obj->weight;
	int min_weight = p->class->min_weight;

	/* Enforce a minimum "weight" (tenth pounds) */
	div = (weight < min_weight) ? min_weight : weight;

	/* Get the strength vs weight */
	str_index = adj_str_blow[state->stat_ind[STAT_STR]] *
			p->class->att_multiply / div;

	/* Maximal value */
	if (str_index > 11) str_index = 11;

	/* Index by dexterity */
	dex_index = MIN(adj_dex_blow[state->stat_ind[STAT_DEX]], 11);

	/* Use the blows table to get energy per blow */
	blow_energy = blows_table[str_index][dex_index];

	blows = MIN((10000 / blow_energy), (100 * p->class->max_attacks));

	/* Require at least one blow, two for O-combat */
	return MAX(blows + (100 * extra_blows),
			   OPT(p, birth_percent_damage) ? 200 : 100);
}


/**
 * Computes current weight limit.
 */
static int weight_limit(struct player_state *state)
{
	int i;

	/* Weight limit based only on strength */
	i = adj_str_wgt[state->stat_ind[STAT_STR]] * 100;

	/* Return the result */
	return (i);
}


/**
 * Computes weight remaining before burdened.
 */
int weight_remaining(struct player *p)
{
	int i;

	/* Weight limit based only on strength */
	i = 60 * adj_str_wgt[p->state.stat_ind[STAT_STR]]
		- p->upkeep->total_weight - 1;

	/* Return the result */
	return (i);
}


/**
 * Calculate the effect of a shapechange on player state
 */
static void calc_shapechange(struct player_state *state,
							 struct player_shape *shape,
							 int *blows, int *shots, int *might, int *moves)
{
	int i;

	/* Combat stats */
	state->to_a += shape->to_a;
	state->to_h += shape->to_h;
	state->to_d += shape->to_d;

	/* Skills */
	for (i = 0; i < SKILL_MAX; i++) {
		state->skills[i] += shape->skills[i];
	}

	/* Object flags */
	of_union(state->flags, shape->flags);

	/* Player flags */
	pf_union(state->pflags, shape->pflags);

	/* Stats */
	for (i = 0; i < STAT_MAX; i++) {
		state->stat_add[i] += shape->modifiers[i];
	}

	/* Other modifiers */
	state->skills[SKILL_STEALTH] += shape->modifiers[OBJ_MOD_STEALTH];
	state->skills[SKILL_SEARCH] += (shape->modifiers[OBJ_MOD_SEARCH] * 5);
	state->see_infra += shape->modifiers[OBJ_MOD_INFRA];
	state->skills[SKILL_DIGGING] += (shape->modifiers[OBJ_MOD_TUNNEL] * 20);
	state->speed += shape->modifiers[OBJ_MOD_SPEED];
	state->dam_red += shape->modifiers[OBJ_MOD_DAM_RED];
	*blows += shape->modifiers[OBJ_MOD_BLOWS];
	*shots += shape->modifiers[OBJ_MOD_SHOTS];
	*might += shape->modifiers[OBJ_MOD_MIGHT];
	*moves += shape->modifiers[OBJ_MOD_MOVES];

	/* Resists and vulnerabilities */
	for (i = 0; i < ELEM_MAX; i++) {
		if (state->el_info[i].res_level == 0) {
			/* Simple, just apply shape res/vuln */
			state->el_info[i].res_level = shape->el_info[i].res_level;
		} else if (state->el_info[i].res_level == -1) {
			/* Shape resists cancel, immunities trump, vulnerabilities */
			if (shape->el_info[i].res_level == 1) {
				state->el_info[i].res_level = 0;
			} else if (shape->el_info[i].res_level == 3) {
				state->el_info[i].res_level = 3;
			}
		} else if (state->el_info[i].res_level == 1) {
			/* Shape vulnerabilities cancel, immunities enhance, resists */
			if (shape->el_info[i].res_level == -1) {
				state->el_info[i].res_level = 0;
			} else if (shape->el_info[i].res_level == 3) {
				state->el_info[i].res_level = 3;
			}
		} else if (state->el_info[i].res_level == 3) {
			/* Immmunity, shape has no effect */
		}
	}

}

/**
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
 * If known_only is true, calc_bonuses() will only use the known
 * information of objects; thus it returns what the player _knows_
 * the character state to be.
 */
void calc_bonuses(struct player *p, struct player_state *state, bool known_only,
				  bool update)
{
	int i, j, hold;
	int extra_blows = 0;
	int extra_shots = 0;
	int extra_might = 0;
	int extra_moves = 0;
	struct object *launcher = equipped_item_by_slot_name(p, "shooting");
	struct object *weapon = equipped_item_by_slot_name(p, "weapon");
	bitflag f[OF_SIZE];
	bitflag collect_f[OF_SIZE];
	bool vuln[ELEM_MAX];

	/* Hack to allow calculating hypothetical blows for extra STR, DEX - NRM */
	int str_ind = state->stat_ind[STAT_STR];
	int dex_ind = state->stat_ind[STAT_DEX];

	/* Reset */
	memset(state, 0, sizeof *state);

	/* Set various defaults */
	state->speed = 110;
	state->num_blows = 100;

	/* Extract race/class info */
	state->see_infra = p->race->infra;
	for (i = 0; i < SKILL_MAX; i++) {
		state->skills[i] = p->race->r_skills[i]	+ p->class->c_skills[i];
	}
	for (i = 0; i < ELEM_MAX; i++) {
		vuln[i] = false;
		if (p->race->el_info[i].res_level == -1) {
			vuln[i] = true;
		} else {
			state->el_info[i].res_level = p->race->el_info[i].res_level;
		}
	}

	/* Base pflags */
	pf_wipe(state->pflags);
	pf_copy(state->pflags, p->race->pflags);
	pf_union(state->pflags, p->class->pflags);

	/* Extract the player flags */
	player_flags(p, collect_f);

	/* Analyze equipment */
	for (i = 0; i < p->body.count; i++) {
		int index = 0;
		struct object *obj = slot_object(p, i);
		struct curse_data *curse = obj ? obj->curses : NULL;

		while (obj) {
			int dig = 0;

			/* Extract the item flags */
			if (known_only) {
				object_flags_known(obj, f);
			} else {
				object_flags(obj, f);
			}
			of_union(collect_f, f);

			/* Apply modifiers */
			state->stat_add[STAT_STR] += obj->modifiers[OBJ_MOD_STR]
				* p->obj_k->modifiers[OBJ_MOD_STR];
			state->stat_add[STAT_INT] += obj->modifiers[OBJ_MOD_INT]
				* p->obj_k->modifiers[OBJ_MOD_INT];
			state->stat_add[STAT_WIS] += obj->modifiers[OBJ_MOD_WIS]
				* p->obj_k->modifiers[OBJ_MOD_WIS];
			state->stat_add[STAT_DEX] += obj->modifiers[OBJ_MOD_DEX]
				* p->obj_k->modifiers[OBJ_MOD_DEX];
			state->stat_add[STAT_CON] += obj->modifiers[OBJ_MOD_CON]
				* p->obj_k->modifiers[OBJ_MOD_CON];
			state->skills[SKILL_STEALTH] += obj->modifiers[OBJ_MOD_STEALTH]
				* p->obj_k->modifiers[OBJ_MOD_STEALTH];
			state->skills[SKILL_SEARCH] += (obj->modifiers[OBJ_MOD_SEARCH] * 5)
				* p->obj_k->modifiers[OBJ_MOD_SEARCH];

			state->see_infra += obj->modifiers[OBJ_MOD_INFRA]
				* p->obj_k->modifiers[OBJ_MOD_INFRA];
			if (tval_is_digger(obj)) {
				if (of_has(obj->flags, OF_DIG_1))
					dig = 1;
				else if (of_has(obj->flags, OF_DIG_2))
					dig = 2;
				else if (of_has(obj->flags, OF_DIG_3))
					dig = 3;
			}
			dig += obj->modifiers[OBJ_MOD_TUNNEL]
				* p->obj_k->modifiers[OBJ_MOD_TUNNEL];
			state->skills[SKILL_DIGGING] += (dig * 20);
			state->speed += obj->modifiers[OBJ_MOD_SPEED]
				* p->obj_k->modifiers[OBJ_MOD_SPEED];
			state->dam_red += obj->modifiers[OBJ_MOD_DAM_RED]
				* p->obj_k->modifiers[OBJ_MOD_DAM_RED];
			extra_blows += obj->modifiers[OBJ_MOD_BLOWS]
				* p->obj_k->modifiers[OBJ_MOD_BLOWS];
			extra_shots += obj->modifiers[OBJ_MOD_SHOTS]
				* p->obj_k->modifiers[OBJ_MOD_SHOTS];
			extra_might += obj->modifiers[OBJ_MOD_MIGHT]
				* p->obj_k->modifiers[OBJ_MOD_MIGHT];
			extra_moves += obj->modifiers[OBJ_MOD_MOVES]
				* p->obj_k->modifiers[OBJ_MOD_MOVES];

			/* Apply element info, noting vulnerabilites for later processing */
			for (j = 0; j < ELEM_MAX; j++) {
				if (!known_only || obj->known->el_info[j].res_level) {
					if (obj->el_info[j].res_level == -1)
						vuln[i] = true;

					/* OK because res_level hasn't included vulnerability yet */
					if (obj->el_info[j].res_level > state->el_info[j].res_level)
						state->el_info[j].res_level = obj->el_info[j].res_level;
				}
			}

			/* Apply combat bonuses */
			state->ac += obj->ac;
			if (!known_only || obj->known->to_a)
				state->to_a += obj->to_a;
			if (!slot_type_is(i, EQUIP_WEAPON) && !slot_type_is(i, EQUIP_BOW)) {
				if (!known_only || obj->known->to_h) {
					state->to_h += obj->to_h;
				}
				if (!known_only || obj->known->to_d) {
					state->to_d += obj->to_d;
				}
			}

			/* Move to any unprocessed curse object */
			if (curse) {
				index++;
				obj = NULL;
				while (index < z_info->curse_max) {
					if (curse[index].power) {
						obj = curses[index].obj;
						break;
					} else {
						index++;
					}
				}
			} else {
				obj = NULL;
			}
		}
	}

	/* Apply the collected flags */
	of_union(state->flags, collect_f);

	/* Now deal with vulnerabilities */
	for (i = 0; i < ELEM_MAX; i++) {
		if (vuln[i] && (state->el_info[i].res_level < 3))
			state->el_info[i].res_level--;
	}

	/* Add shapechange info */
	calc_shapechange(state, p->shape, &extra_blows, &extra_shots, &extra_might,
		&extra_moves);

	/* Calculate light */
	calc_light(p, state, update);

	/* Unlight - needs change if anything but resist is introduced for dark */
	if (player_has(p, PF_UNLIGHT) && character_dungeon) {
		state->el_info[ELEM_DARK].res_level = 1;
	}

	/* Calculate the various stat values */
	for (i = 0; i < STAT_MAX; i++) {
		int add, use, ind;

		add = state->stat_add[i];
		add += (p->race->r_adj[i] + p->class->c_adj[i]);
		state->stat_top[i] =  modify_stat_value(p->stat_max[i], add);
		use = modify_stat_value(p->stat_cur[i], add);

		state->stat_use[i] = use;

		if (use <= 3) {/* Values: n/a */
			ind = 0;
		} else if (use <= 18) {/* Values: 3, 4, ..., 18 */
			ind = (use - 3);
		} else if (use <= 18+219) {/* Ranges: 18/00-18/09, ..., 18/210-18/219 */
			ind = (15 + (use - 18) / 10);
		} else {/* Range: 18/220+ */
			ind = (37);
		}

		assert((0 <= ind) && (ind < STAT_RANGE));

		/* Hack for hypothetical blows - NRM */
		if (!update) {
			if (i == STAT_STR) {
				ind += str_ind;
				ind = MIN(ind, 37);
				ind = MAX(ind, 3);
			} else if (i == STAT_DEX) {
				ind += dex_ind;
				ind = MIN(ind, 37);
				ind = MAX(ind, 3);
			}
		}

		/* Save the new index */
		state->stat_ind[i] = ind;
	}

	/* Effects of food outside the "Fed" range */
	if (!player_timed_grade_eq(p, TMD_FOOD, "Fed")) {
		int excess = p->timed[TMD_FOOD] - PY_FOOD_FULL;
		int lack = PY_FOOD_HUNGRY - p->timed[TMD_FOOD];
		if ((excess > 0) && !p->timed[TMD_ATT_VAMP]) {
			/* Scale to units 1/10 of the range and subtract from speed */
			excess = (excess * 10) / (PY_FOOD_MAX - PY_FOOD_FULL);
			state->speed -= excess;
		} else if (lack > 0) {
			/* Scale to units 1/20 of the range */
			lack = (lack * 20) / PY_FOOD_HUNGRY;

			/* Apply effects progressively */
			state->to_h -= lack;
			state->to_d -= lack;
			if ((lack > 10) && (lack <= 15)) {
				state->skills[SKILL_DEVICE] *= 9;
				state->skills[SKILL_DEVICE] /= 10;
			} else if ((lack > 15) && (lack <= 18)) {
				state->skills[SKILL_DEVICE] *= 8;
				state->skills[SKILL_DEVICE] /= 10;
				state->skills[SKILL_DISARM_PHYS] *= 9;
				state->skills[SKILL_DISARM_PHYS] /= 10;
				state->skills[SKILL_DISARM_MAGIC] *= 9;
				state->skills[SKILL_DISARM_MAGIC] /= 10;
			} else if (lack > 18) {
				state->skills[SKILL_DEVICE] *= 7;
				state->skills[SKILL_DEVICE] /= 10;
				state->skills[SKILL_DISARM_PHYS] *= 8;
				state->skills[SKILL_DISARM_PHYS] /= 10;
				state->skills[SKILL_DISARM_MAGIC] *= 8;
				state->skills[SKILL_DISARM_MAGIC] /= 10;
				state->skills[SKILL_SAVE] *= 9;
				state->skills[SKILL_SAVE] /= 10;
				state->skills[SKILL_SEARCH] *=9;
				state->skills[SKILL_SEARCH] /= 10;
			}
		}
	}

	/* Other timed effects */
	if (player_timed_grade_eq(p, TMD_STUN, "Heavy Stun")) {
		state->to_h -= 20;
		state->to_d -= 20;
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE] * 8 / 10;
	} else if (player_timed_grade_eq(p, TMD_STUN, "Stun")) {
		state->to_h -= 5;
		state->to_d -= 5;
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE] * 9 / 10;
	}
	if (p->timed[TMD_INVULN]) {
		state->to_a += 100;
	}
	if (p->timed[TMD_BLESSED]) {
		state->to_a += 5;
		state->to_h += 10;
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE] * 105 / 100;
	}
	if (p->timed[TMD_SHIELD]) {
		state->to_a += 50;
	}
	if (p->timed[TMD_STONESKIN]) {
		state->to_a += 40;
		state->speed -= 5;
	}
	if (p->timed[TMD_BOLD]) {
		of_on(state->flags, OF_PROT_FEAR);
	}
	if (p->timed[TMD_HERO]) {
		of_on(state->flags, OF_PROT_FEAR);
		state->to_h += 12;
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE] * 105 / 100;
	}
	if (p->timed[TMD_SHERO]) {
		of_on(state->flags, OF_PROT_FEAR);
		state->to_h += 24;
		state->to_a -= 10;
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE] * 9 / 10;
	}
	if (p->timed[TMD_FAST] || p->timed[TMD_SPRINT]) {
		state->speed += 10;
	}
	if (p->timed[TMD_SLOW]) {
		state->speed -= 10;
	}
	if (p->timed[TMD_SINFRA]) {
		state->see_infra += 5;
	}
	if (p->timed[TMD_TELEPATHY]) {
		of_on(state->flags, OF_TELEPATHY);
	}
	if (p->timed[TMD_SINVIS]) {
		of_on(state->flags, OF_SEE_INVIS);
	}
	if (p->timed[TMD_AFRAID] || p->timed[TMD_TERROR]) {
		of_on(state->flags, OF_AFRAID);
	}
	if (p->timed[TMD_TERROR]) {
		state->speed += 10;
	}
	if (p->timed[TMD_OPP_ACID] && (state->el_info[ELEM_ACID].res_level < 2)) {
			state->el_info[ELEM_ACID].res_level++;
	}
	if (p->timed[TMD_OPP_ELEC] && (state->el_info[ELEM_ELEC].res_level < 2)) {
			state->el_info[ELEM_ELEC].res_level++;
	}
	if (p->timed[TMD_OPP_FIRE] && (state->el_info[ELEM_FIRE].res_level < 2)) {
			state->el_info[ELEM_FIRE].res_level++;
	}
	if (p->timed[TMD_OPP_COLD] && (state->el_info[ELEM_COLD].res_level < 2)) {
			state->el_info[ELEM_COLD].res_level++;
	}
	if (p->timed[TMD_OPP_POIS] && (state->el_info[ELEM_POIS].res_level < 2)) {
			state->el_info[ELEM_POIS].res_level++;
	}
	if (p->timed[TMD_OPP_CONF]) {
		of_on(state->flags, OF_PROT_CONF);
	}
	if (p->timed[TMD_CONFUSED]) {
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE] * 75 / 100;
	}
	if (p->timed[TMD_AMNESIA]) {
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE] * 8 / 10;
	}
	if (p->timed[TMD_POISONED]) {
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE] * 95 / 100;
	}
	if (p->timed[TMD_IMAGE]) {
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE] * 8 / 10;
	}
	if (p->timed[TMD_BLOODLUST]) {
		state->to_d += p->timed[TMD_BLOODLUST] / 2;
		extra_blows += p->timed[TMD_BLOODLUST] / 20;
	}
	if (p->timed[TMD_STEALTH]) {
		state->skills[SKILL_STEALTH] += 10;
	}

	/* Analyze flags - check for fear */
	if (of_has(state->flags, OF_AFRAID)) {
		state->to_h -= 20;
		state->to_a += 8;
		state->skills[SKILL_DEVICE] = state->skills[SKILL_DEVICE] * 95 / 100;
	}

	/* Analyze weight */
	j = p->upkeep->total_weight;
	i = weight_limit(state);
	if (j > i / 2)
		state->speed -= ((j - (i / 2)) / (i / 10));
	if (state->speed < 0)
		state->speed = 0;
	if (state->speed > 199)
		state->speed = 199;

	/* Apply modifier bonuses (Un-inflate stat bonuses) */
	state->to_a += adj_dex_ta[state->stat_ind[STAT_DEX]];
	state->to_d += adj_str_td[state->stat_ind[STAT_STR]];
	state->to_h += adj_dex_th[state->stat_ind[STAT_DEX]];
	state->to_h += adj_str_th[state->stat_ind[STAT_STR]];


	/* Modify skills */
	state->skills[SKILL_DISARM_PHYS] += adj_dex_dis[state->stat_ind[STAT_DEX]];
	state->skills[SKILL_DISARM_MAGIC] += adj_int_dis[state->stat_ind[STAT_INT]];
	state->skills[SKILL_DEVICE] += adj_int_dev[state->stat_ind[STAT_INT]];
	state->skills[SKILL_SAVE] += adj_wis_sav[state->stat_ind[STAT_WIS]];
	state->skills[SKILL_DIGGING] += adj_str_dig[state->stat_ind[STAT_STR]];
	for (i = 0; i < SKILL_MAX; i++)
		state->skills[i] += (p->class->x_skills[i] * p->lev / 10);

	if (state->skills[SKILL_DIGGING] < 1) state->skills[SKILL_DIGGING] = 1;
	if (state->skills[SKILL_STEALTH] > 30) state->skills[SKILL_STEALTH] = 30;
	if (state->skills[SKILL_STEALTH] < 0) state->skills[SKILL_STEALTH] = 0;
	hold = adj_str_hold[state->stat_ind[STAT_STR]];


	/* Analyze launcher */
	state->heavy_shoot = false;
	if (launcher) {
		if (hold < launcher->weight / 10) {
			state->to_h += 2 * (hold - launcher->weight / 10);
			state->heavy_shoot = true;
		}

		state->num_shots = 10;

		/* Type of ammo */
		if (kf_has(launcher->kind->kind_flags, KF_SHOOTS_SHOTS))
			state->ammo_tval = TV_SHOT;
		else if (kf_has(launcher->kind->kind_flags, KF_SHOOTS_ARROWS))
			state->ammo_tval = TV_ARROW;
		else if (kf_has(launcher->kind->kind_flags, KF_SHOOTS_BOLTS))
			state->ammo_tval = TV_BOLT;

		/* Multiplier */
		state->ammo_mult = launcher->pval;

		/* Apply special flags */
		if (!state->heavy_shoot) {
			state->num_shots += extra_shots;
			state->ammo_mult += extra_might;
			if (player_has(p, PF_FAST_SHOT) && (state->ammo_tval == TV_ARROW)) {
				state->num_shots += p->lev / 3;
			}
		}

		/* Require at least one shot */
		if (state->num_shots < 10) state->num_shots = 10;
	}


	/* Analyze weapon */
	state->heavy_wield = false;
	state->bless_wield = false;
	if (weapon) {
		/* It is hard to hold a heavy weapon */
		if (hold < weapon->weight / 10) {
			state->to_h += 2 * (hold - weapon->weight / 10);
			state->heavy_wield = true;
		}

		/* Normal weapons */
		if (!state->heavy_wield) {
			state->num_blows = calc_blows(p, weapon, state, extra_blows);
			state->skills[SKILL_DIGGING] += (weapon->weight / 10);
		}

		/* Divine weapon bonus for blessed weapons */
		if (player_has(p, PF_BLESS_WEAPON) && of_has(state->flags, OF_BLESSED)){
			state->to_h += 2;
			state->to_d += 2;
			state->bless_wield = true;
		}
	} else {
		state->num_blows = calc_blows(p, NULL, state, extra_blows);
	}

	/* Mana */
	calc_mana(p, state, update);
	if (!p->msp) {
		pf_on(state->pflags, PF_NO_MANA);
	}

	/* Movement speed */
	state->num_moves = 1 + extra_moves;;

	return;
}

/**
 * Calculate bonuses, and print various things on changes.
 */
static void update_bonuses(struct player *p)
{
	int i;

	struct player_state state = p->state;
	struct player_state known_state = p->known_state;


	/* ------------------------------------
	 * Calculate bonuses
	 * ------------------------------------ */

	calc_bonuses(p, &state, false, true);
	calc_bonuses(p, &known_state, true, true);


	/* ------------------------------------
	 * Notice changes
	 * ------------------------------------ */

	/* Analyze stats */
	for (i = 0; i < STAT_MAX; i++) {
		/* Notice changes */
		if (state.stat_top[i] != p->state.stat_top[i])
			/* Redisplay the stats later */
			p->upkeep->redraw |= (PR_STATS);

		/* Notice changes */
		if (state.stat_use[i] != p->state.stat_use[i])
			/* Redisplay the stats later */
			p->upkeep->redraw |= (PR_STATS);

		/* Notice changes */
		if (state.stat_ind[i] != p->state.stat_ind[i]) {
			/* Change in CON affects Hitpoints */
			if (i == STAT_CON)
				p->upkeep->update |= (PU_HP);

			/* Change in stats may affect Mana/Spells */
			p->upkeep->update |= (PU_MANA | PU_SPELLS);
		}
	}


	/* Hack -- Telepathy Change */
	if (of_has(state.flags, OF_TELEPATHY) !=
		of_has(p->state.flags, OF_TELEPATHY))
		/* Update monster visibility */
		p->upkeep->update |= (PU_MONSTERS);
	/* Hack -- See Invis Change */
	if (of_has(state.flags, OF_SEE_INVIS) !=
		of_has(p->state.flags, OF_SEE_INVIS))
		/* Update monster visibility */
		p->upkeep->update |= (PU_MONSTERS);

	/* Redraw speed (if needed) */
	if (state.speed != p->state.speed)
		p->upkeep->redraw |= (PR_SPEED);

	/* Redraw armor (if needed) */
	if ((known_state.ac != p->known_state.ac) || 
		(known_state.to_a != p->known_state.to_a))
		p->upkeep->redraw |= (PR_ARMOR);

	/* Notice changes in the "light radius" */
	if (p->state.cur_light != state.cur_light) {
		/* Update the visuals */
		p->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
	}

	/* Hack -- handle partial mode */
	if (!p->upkeep->only_partial) {
		/* Take note when "heavy bow" changes */
		if (p->state.heavy_shoot != state.heavy_shoot) {
			/* Message */
			if (state.heavy_shoot)
				msg("You have trouble wielding such a heavy bow.");
			else if (equipped_item_by_slot_name(p, "shooting"))
				msg("You have no trouble wielding your bow.");
			else
				msg("You feel relieved to put down your heavy bow.");
		}

		/* Take note when "heavy weapon" changes */
		if (p->state.heavy_wield != state.heavy_wield) {
			/* Message */
			if (state.heavy_wield)
				msg("You have trouble wielding such a heavy weapon.");
			else if (equipped_item_by_slot_name(p, "weapon"))
				msg("You have no trouble wielding your weapon.");
			else
				msg("You feel relieved to put down your heavy weapon.");	
		}

		/* Take note when "illegal weapon" changes */
		if (p->state.bless_wield != state.bless_wield) {
			/* Message */
			if (state.bless_wield) {
				msg("You feel attuned to your weapon.");
			} else if (equipped_item_by_slot_name(p, "weapon")) {
				msg("You feel less attuned to your weapon.");
			}
		}

		/* Take note when "armor state" changes */
		if (p->state.cumber_armor != state.cumber_armor) {
			/* Message */
			if (state.cumber_armor)
				msg("The weight of your armor encumbers your movement.");
			else
				msg("You feel able to move more freely.");
		}
	}

	memcpy(&p->state, &state, sizeof(state));
	memcpy(&p->known_state, &known_state, sizeof(known_state));
}




/**
 * ------------------------------------------------------------------------
 * Monster and object tracking functions
 * ------------------------------------------------------------------------ */

/**
 * Track the given monster
 */
void health_track(struct player_upkeep *upkeep, struct monster *mon)
{
	upkeep->health_who = mon;
	upkeep->redraw |= PR_HEALTH;
}

/**
 * Track the given monster race
 */
void monster_race_track(struct player_upkeep *upkeep, struct monster_race *race)
{
	/* Save this monster ID */
	upkeep->monster_race = race;

	/* Window stuff */
	upkeep->redraw |= (PR_MONSTER);
}

/**
 * Track the given object
 */
void track_object(struct player_upkeep *upkeep, struct object *obj)
{
	upkeep->object = obj;
	upkeep->object_kind = NULL;
	upkeep->redraw |= (PR_OBJECT);
}

/**
 * Track the given object kind
 */
void track_object_kind(struct player_upkeep *upkeep, struct object_kind *kind)
{
	upkeep->object = NULL;
	upkeep->object_kind = kind;
	upkeep->redraw |= (PR_OBJECT);
}

/**
 * Cancel all object tracking
 */
void track_object_cancel(struct player_upkeep *upkeep)
{
	upkeep->object = NULL;
	upkeep->object_kind = NULL;
	upkeep->redraw |= (PR_OBJECT);
}

/**
 * Is the given item tracked?
 */
bool tracked_object_is(struct player_upkeep *upkeep, struct object *obj)
{
	return (upkeep->object == obj);
}



/**
 * ------------------------------------------------------------------------
 * Generic "deal with" functions
 * ------------------------------------------------------------------------ */

/**
 * Handle "player->upkeep->notice"
 */
void notice_stuff(struct player *p)
{
	/* Notice stuff */
	if (!p->upkeep->notice) return;

	/* Deal with ignore stuff */
	if (p->upkeep->notice & PN_IGNORE) {
		p->upkeep->notice &= ~(PN_IGNORE);
		ignore_drop();
	}

	/* Combine the pack */
	if (p->upkeep->notice & PN_COMBINE) {
		p->upkeep->notice &= ~(PN_COMBINE);
		combine_pack();
	}

	/* Dump the monster messages */
	if (p->upkeep->notice & PN_MON_MESSAGE) {
		p->upkeep->notice &= ~(PN_MON_MESSAGE);

		/* Make sure this comes after all of the monster messages */
		show_monster_messages();
	}
}

/**
 * Handle "player->upkeep->update"
 */
void update_stuff(struct player *p)
{
	/* Update stuff */
	if (!p->upkeep->update) return;


	if (p->upkeep->update & (PU_INVEN)) {
		p->upkeep->update &= ~(PU_INVEN);
		update_inventory(p);
	}

	if (p->upkeep->update & (PU_BONUS)) {
		p->upkeep->update &= ~(PU_BONUS);
		update_bonuses(p);
	}

	if (p->upkeep->update & (PU_TORCH)) {
		p->upkeep->update &= ~(PU_TORCH);
		calc_light(p, &p->state, true);
	}

	if (p->upkeep->update & (PU_HP)) {
		p->upkeep->update &= ~(PU_HP);
		calc_hitpoints(p);
	}

	if (p->upkeep->update & (PU_MANA)) {
		p->upkeep->update &= ~(PU_MANA);
		calc_mana(p, &p->state, true);
	}

	if (p->upkeep->update & (PU_SPELLS)) {
		p->upkeep->update &= ~(PU_SPELLS);
		if (p->class->magic.total_spells > 0) {
			calc_spells(p);
		}
	}

	/* Character is not ready yet, no map updates */
	if (!character_generated) return;

	/* Map is not shown, no map updates */
	if (!map_is_visible()) return;

	if (p->upkeep->update & (PU_UPDATE_VIEW)) {
		p->upkeep->update &= ~(PU_UPDATE_VIEW);
		update_view(cave, p);
	}

	if (p->upkeep->update & (PU_DISTANCE)) {
		p->upkeep->update &= ~(PU_DISTANCE);
		p->upkeep->update &= ~(PU_MONSTERS);
		update_monsters(true);
	}

	if (p->upkeep->update & (PU_MONSTERS)) {
		p->upkeep->update &= ~(PU_MONSTERS);
		update_monsters(false);
	}


	if (p->upkeep->update & (PU_PANEL)) {
		p->upkeep->update &= ~(PU_PANEL);
		event_signal(EVENT_PLAYERMOVED);
	}
}



struct flag_event_trigger
{
	u32b flag;
	game_event_type event;
};



/**
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
	{ PR_FEELING, EVENT_FEELING },
	{ PR_LIGHT,   EVENT_LIGHT },

	{ PR_INVEN,   EVENT_INVENTORY },
	{ PR_EQUIP,   EVENT_EQUIPMENT },
	{ PR_MONLIST, EVENT_MONSTERLIST },
	{ PR_ITEMLIST, EVENT_ITEMLIST },
	{ PR_MONSTER, EVENT_MONSTERTARGET },
	{ PR_OBJECT, EVENT_OBJECTTARGET },
	{ PR_MESSAGE, EVENT_MESSAGE },
};

/**
 * Handle "player->upkeep->redraw"
 */
void redraw_stuff(struct player *p)
{
	size_t i;
	u32b redraw = p->upkeep->redraw;

	/* Redraw stuff */
	if (!redraw) return;

	/* Character is not ready yet, no screen updates */
	if (!character_generated) return;

	/* Map is not shown, subwindow updates only */
	if (!map_is_visible()) 
		redraw &= PR_SUBWINDOW;

	/* Hack - rarely update while resting or running, makes it over quicker */
	if (((player_resting_count(p) % 100) || (p->upkeep->running % 100))
		&& !(redraw & PR_MESSAGE))
		return;

	/* For each listed flag, send the appropriate signal to the UI */
	for (i = 0; i < N_ELEMENTS(redraw_events); i++) {
		const struct flag_event_trigger *hnd = &redraw_events[i];

		if (redraw & hnd->flag)
			event_signal(hnd->event);
	}

	/* Then the ones that require parameters to be supplied. */
	if (redraw & PR_MAP) {
		/* Mark the whole map to be redrawn */
		event_signal_point(EVENT_MAP, -1, -1);
	}

	p->upkeep->redraw &= ~redraw;

	/* Map is not shown, subwindow updates only */
	if (!map_is_visible()) return;

	/*
	 * Do any plotting, etc. delayed from earlier - this set of updates
	 * is over.
	 */
	event_signal(EVENT_END);
}


/**
 * Handle "player->upkeep->update" and "player->upkeep->redraw"
 */
void handle_stuff(struct player *p)
{
	if (p->upkeep->update) update_stuff(p);
	if (p->upkeep->redraw) redraw_stuff(p);
}

