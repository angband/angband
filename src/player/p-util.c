/* player/p-util.c
 * Copyright (c) 2011 The Angband Developers. See COPYING.
 */

#include "angband.h"
#include "cave.h"
#include "object/tvalsval.h"

/*
 * Modify a stat value by a "modifier", return new value
 *
 * Stats go up: 3,4,...,17,18,18/10,18/20,...,18/220
 * Or even: 18/13, 18/23, 18/33, ..., 18/220
 *
 * Stats go down: 18/220, 18/210,..., 18/10, 18, 17, ..., 3
 * Or even: 18/13, 18/03, 18, 17, ..., 3
 */
s16b modify_stat_value(int value, int amount)
{
	int i;

	/* Reward */
	if (amount > 0)
	{
		/* Apply each point */
		for (i = 0; i < amount; i++)
		{
			/* One point at a time */
			if (value < 18) value++;

			/* Ten "points" at a time */
			else value += 10;
		}
	}

	/* Penalty */
	else if (amount < 0)
	{
		/* Apply each point */
		for (i = 0; i < (0 - amount); i++)
		{
			/* Ten points at a time */
			if (value >= 18+10) value -= 10;

			/* Hack -- prevent weirdness */
			else if (value > 18) value = 18;

			/* One point at a time */
			else if (value > 3) value--;
		}
	}

	/* Return new value */
	return (value);
}

/* Is the player capable of casting a spell? */
bool player_can_cast(void)
{
	if (!p_ptr->class->spell_book)
	{
		msg("You cannot pray or produce magics.");
		return FALSE;
	}

	if (p_ptr->timed[TMD_BLIND] || no_light())
	{
		msg("You cannot see!");
		return FALSE;
	}

	if (p_ptr->timed[TMD_CONFUSED])
	{
		msg("You are too confused!");
		return FALSE;
	}

	return TRUE;
}

/* Is the player capable of studying? */
bool player_can_study(void)
{
	if (!player_can_cast())
		return FALSE;

	if (!p_ptr->new_spells)
	{
		const char *p = ((p_ptr->class->spell_book == TV_MAGIC_BOOK) ? "spell" : "prayer");
		msg("You cannot learn any new %ss!", p);
		return FALSE;
	}

	return TRUE;
}

/* Does the player carry a book with a spell they can study? */
bool player_can_study_book(void)
{
	int item_list[INVEN_TOTAL];
	int item_num;

	object_type *o_ptr;
	struct spell *sp;

	/* Check if the player can cast spells */
	if (!player_can_cast())
		return FALSE;

	/* Check if the player can learn new spells */
	if (!p_ptr->new_spells)
		return FALSE;

	/* Get the number of books in inventory */
	item_tester_hook = obj_can_browse;
	item_num = scan_items(item_list, N_ELEMENTS(item_list), (USE_INVEN));

	/* Check through all available books */
	for (int i = 0; i < item_num; i++)
	{
		o_ptr = object_from_item_idx(i);

		/* Extract spells */
		for (sp = o_ptr->kind->spells; sp; sp = sp->next)
		{
			/* Check if the player can study it */
			if (spell_okay_to_study(sp->spell_index))
			{
				/* There is a spell the player can study */
				return TRUE;
			}
		}
	}

	return FALSE;
}

/* Determine if the player can read scrolls. */
bool player_can_read(void)
{
	if (p_ptr->timed[TMD_BLIND])
	{
		msg("You can't see anything.");
		return FALSE;
	}

	if (no_light())
	{
		msg("You have no light to read by.");
		return FALSE;
	}

	if (p_ptr->timed[TMD_CONFUSED])
	{
		msg("You are too confused to read!");
		return FALSE;
	}

	if (p_ptr->timed[TMD_AMNESIA])
	{
		msg("You can't remember how to read!");
		return FALSE;
	}

	return TRUE;
}

/* Determine if the player can fire with the bow */
bool player_can_fire(void)
{
	object_type *o_ptr = &p_ptr->inventory[INVEN_BOW];

	/* Require a usable launcher */
	if (!o_ptr->tval || !p_ptr->state.ammo_tval)
	{
		msg("You have nothing to fire with.");
		return FALSE;
	}

	return TRUE;
}

bool player_can_refuel(void)
{
	object_type *obj = &p_ptr->inventory[INVEN_LIGHT];

	if (obj->kind && obj->sval == SV_LIGHT_LANTERN)
		return TRUE;

	msg("Your light cannot be refuelled.");
	return FALSE;
}

/*
 * Apply confusion, if needed, to a direction
 *
 * Display a message and return TRUE if direction changes.
 */
bool player_confuse_dir(struct player *p, int *dp, bool too)
{
	int dir = *dp;

	if (p->timed[TMD_CONFUSED])
		if ((dir == 5) || (randint0(100) < 75))
			/* Random direction */
			dir = ddd[randint0(8)];

	if (*dp != dir) {
		if (too)
			msg("You are too confused.");
		else
			msg("You are confused.");

		*dp = dir;
		return TRUE;
	}

	return FALSE;
}
