/* macro.c - macro implementation */

#include "angband.h"
#include "defines.h"
#include "ui-event.h"
#include "keymap.h"
#include "macro.h"
#include "z-util.h"
#include "z-virt.h"

int max_macrotrigger = 0;
char *macro_modifier_chr;
char *macro_modifier_name[MAX_MACRO_MOD];

s16b macro__num;
char **macro__pat;
char **macro__act;
static bool macro__use[256];

/*
 * Find the macro (if any) which exactly matches the given pattern
 */
int macro_find_exact(const char *pat)
{
	int i;

	/* Nothing possible */
	if (!macro__use[(byte)(pat[0])])
		return -1;

	/* Scan the macros */
	for (i = 0; i < macro__num; ++i)
	{
		if (streq(macro__pat[i], pat))
			return i;
	}

	/* No matches */
	return -1;
}


/*
 * Find the first macro (if any) which contains the given pattern
 */
int macro_find_check(const char *pat)
{
	int i;

	/* Nothing possible */
	if (!macro__use[(byte)(pat[0])])
		return -1;

	/* Scan the macros */
	for (i = 0; i < macro__num; ++i)
	{
		if (prefix(macro__pat[i], pat))
			return i;
	}

	/* Nothing */
	return -1;
}


/*
 * Find the first macro (if any) which contains the given pattern and more
 */
int macro_find_maybe(const char *pat)
{
	int i;

	/* Nothing possible */
	if (!macro__use[(byte)(pat[0])])
		return -1;

	/* Scan the macros */
	for (i = 0; i < macro__num; ++i)
	{
		if (prefix(macro__pat[i], pat) && !streq(macro__pat[i], pat))
			return i;
	}

	/* Nothing */
	return -1;
}



/*
 * Find the longest macro (if any) which starts with the given pattern
 */
int macro_find_ready(const char *pat)
{
	int i, t, n = -1, s = -1;

	/* Nothing possible */
	if (!macro__use[(byte)(pat[0])])
		return -1;

	/* Scan the macros */
	for (i = 0; i < macro__num; ++i)
	{
		/* Skip macros which are not contained by the pattern */
		if (!prefix(pat, macro__pat[i])) continue;

		/* Obtain the length of this macro */
		t = strlen(macro__pat[i]);

		/* Only track the "longest" pattern */
		if ((n >= 0) && (s > t)) continue;

		/* Track the entry */
		n = i;
		s = t;
	}

	/* Result */
	return n;
}

/*
 * Add a macro definition (or redefinition).
 *
 * We should use "act == NULL" to "remove" a macro, but this might make it
 * impossible to save the "removal" of a macro definition.  XXX XXX XXX
 *
 * We should consider refusing to allow macros which contain existing macros,
 * or which are contained in existing macros, because this would simplify the
 * macro analysis code.  XXX XXX XXX
 *
 * We should consider removing the "command macro" crap, and replacing it
 * with some kind of "powerful keymap" ability, but this might make it hard
 * to change the "roguelike" option from inside the game.  XXX XXX XXX
 */
errr macro_add(const char *pat, const char *act)
{
	int n;

	if (!pat || !act) return (-1);


	/* Look for any existing macro */
	n = macro_find_exact(pat);

	/* Replace existing macro */
	if (n >= 0)
	{
		string_free(macro__act[n]);
	}

	/* Create a new macro */
	else
	{
		/* Get a new index */
		n = macro__num++;
		if (macro__num >= MACRO_MAX) quit("Too many macros!");

		/* Save the pattern */
		macro__pat[n] = string_make(pat);
	}

	/* Save the action */
	macro__act[n] = string_make(act);

	/* Efficiency */
	macro__use[(byte)(pat[0])] = TRUE;

	/* Success */
	return (0);
}



/*
 * Initialize the "macro" package
 */
errr macro_init(void)
{
	/* Macro patterns */
	macro__pat = C_ZNEW(MACRO_MAX, char *);

	/* Macro actions */
	macro__act = C_ZNEW(MACRO_MAX, char *);

	/* Success */
	return (0);
}


/*
 * Free the macro package
 */
errr macro_free(void)
{
	int i;

	/* Free the macros */
	for (i = 0; i < macro__num; ++i)
	{
		string_free(macro__pat[i]);
		string_free(macro__act[i]);
	}

	FREE(macro__pat);
	FREE(macro__act);

	/* Success */
	return (0);
}
