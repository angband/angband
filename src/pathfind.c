/*
 * File: pathfind.c
 * Purpose: Pathfinding algorithm
 * Author: Chris Cavalaria
 */

#include "angband.h"

/*** Constants ***/

/* Maximum size around the player to consider in the pathfinde */
#define MAX_PF_RADIUS 50

/* Maximum distance to consider in the pathfinder */
#define MAX_PF_LENGTH 250


/*** Globals ***/

static int terrain[MAX_PF_RADIUS][MAX_PF_RADIUS];
char pf_result[MAX_PF_LENGTH];
int pf_result_index;

static int ox, oy, ex, ey;


/*** Pathfinding code ***/

static bool is_valid_pf(int y, int x)
{
	/* Unvisited means forbiden */
	if (!(cave_info[y][x] & (CAVE_MARK))) return (FALSE);

	/* Require open space */
	return (cave_floor_bold(y, x));
}

static void fill_terrain_info(void)
{
	int i, j;

	ox = MAX(p_ptr->px - MAX_PF_RADIUS / 2, 0);
	oy = MAX(p_ptr->py - MAX_PF_RADIUS / 2, 0);

	ex = MIN(p_ptr->px + MAX_PF_RADIUS / 2 - 1, DUNGEON_WID);
	ey = MIN(p_ptr->py + MAX_PF_RADIUS / 2 - 1, DUNGEON_HGT);

	for (i = 0; i < MAX_PF_RADIUS * MAX_PF_RADIUS; i++)
		terrain[0][i] = -1;

	for (j = oy; j < ey; j++)
		for (i = ox; i < ex; i++)
			if (is_valid_pf(j, i))
				terrain[j - oy][i - ox] = MAX_PF_LENGTH;

	terrain[p_ptr->py - oy][p_ptr->px - ox] = 1;
}

#define MARK_DISTANCE(c,d) if ((c <= MAX_PF_LENGTH) && (c > d)) { c = d; try_again = (TRUE); }

bool findpath(int y, int x)
{
	int i, j, dir, wanted_dir;
	bool try_again;
	int cur_distance;

	fill_terrain_info();

	terrain[p_ptr->py - oy][p_ptr->px - ox] = 1;

	if ((x >= ox) && (x < ex) && (y >= oy) && (y < ey))
	{
		if ((cave_m_idx[y][x] > 0) && (mon_list[cave_m_idx[y][x]].ml))
		{
			terrain[y - oy][x - ox] = MAX_PF_LENGTH;
		}

#if 0
		else if (terrain[y-oy][x-ox] != MAX_PF_LENGTH)
		{
		   bell("Target blocked");
		   return (FALSE);
		}
#endif

		terrain[y - oy][x - ox] = MAX_PF_LENGTH;
	}
	else
	{
		bell("Target out of range.");
		return (FALSE);
	}

	if (terrain[y - oy][x - ox] == -1)
	{
		bell("Target space forbidden");
		return (FALSE);
	}


	/* 
	 * And now starts the very naive and very 
	 * inefficient pathfinding algorithm
	 */
	do
	{
		try_again = FALSE;

		for (j = oy + 1; j < ey - 1; j++)
		{
			for (i = ox + 1; i < ex - 1; i++)
			{
				cur_distance = terrain[j - oy][i - ox] + 1;

				if ((cur_distance > 0) && (cur_distance < MAX_PF_LENGTH))
				{
					for (dir = 1; dir < 10; dir++)
					{
						if (dir == 5)
							dir++;

						MARK_DISTANCE(terrain[j - oy + ddy[dir]][i - ox + ddx[dir]], cur_distance);
					}
				}
			}
		}

		if (terrain[y - oy][x - ox] < MAX_PF_LENGTH)
			try_again = (FALSE);

	}
	while (try_again);

	/* Failure */
	if (terrain[y - oy][x - ox] == MAX_PF_LENGTH)
	{
		bell("Target space unreachable.");
		return (FALSE);
	}

	/* Success */
	i = x;
	j = y;

	pf_result_index = 0;

	while ((i != p_ptr->px) || (j != p_ptr->py))
	{
		cur_distance = terrain[j - oy][i - ox] - 1;
		for (dir = 1; dir < 10; dir++)
		{
			if (terrain[j - oy + ddy[dir]][i - ox + ddx[dir]] == cur_distance)
				break;
		}

		/* Should never happend */
		if (dir == 10)
		{
			bell("Wtf ?");
			return (FALSE);
		}

		else if (dir == 5)
		{
			bell("Heyyy !");
			return (FALSE);
		}

		pf_result[pf_result_index++] = '0' + (char)(10 - dir);
		i += ddx[dir];
		j += ddy[dir];
	}

	pf_result_index--;

	return (TRUE);
}
