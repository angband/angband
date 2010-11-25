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
	if (!cp_ptr->spell_book)
	{
		msg_print("You cannot pray or produce magics.");
		return FALSE;
	}

	if (p_ptr->timed[TMD_BLIND] || no_light())
	{
		msg_print("You cannot see!");
		return FALSE;
	}

	if (p_ptr->timed[TMD_CONFUSED])
	{
		msg_print("You are too confused!");
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
		cptr p = ((cp_ptr->spell_book == TV_MAGIC_BOOK) ? "spell" : "prayer");
		msg_format("You cannot learn any new %ss!", p);
		return FALSE;
	}

	return TRUE;
}

/* Determine if the player can read scrolls. */
bool player_can_read(void)
{
	if (p_ptr->timed[TMD_BLIND])
	{
		msg_print("You can't see anything.");
		return FALSE;
	}

	if (no_light())
	{
		msg_print("You have no light to read by.");
		return FALSE;
	}

	if (p_ptr->timed[TMD_CONFUSED])
	{
		msg_print("You are too confused to read!");
		return FALSE;
	}

	if (p_ptr->timed[TMD_AMNESIA])
	{
		msg_print("You can't remember how to read!");
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
		msg_print("You have nothing to fire with.");
		return FALSE;
	}

	return TRUE;
}
