/* macro.c - macro implementation */

#include "defines.h"
#include "macro.h"
#include "z-util.h"
#include "z-virt.h"

int max_macrotrigger = 0;
char *macro_template = NULL;
char *macro_modifier_chr;
char *macro_modifier_name[MAX_MACRO_MOD];
char *macro_trigger_name[MAX_MACRO_TRIGGER];
char *macro_trigger_keycode[2][MAX_MACRO_TRIGGER];

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
	size_t j;

	/* Free the macros */
	for (i = 0; i < macro__num; ++i)
	{
		string_free(macro__pat[i]);
		string_free(macro__act[i]);
	}

	FREE(macro__pat);
	FREE(macro__act);

	/* Free the keymaps */
	for (i = 0; i < KEYMAP_MODES; ++i)
	{
		for (j = 0; j < N_ELEMENTS(keymap_act[i]); ++j)
		{
			string_free(keymap_act[i][j]);
			keymap_act[i][j] = NULL;
		}
	}

	/* Success */
	return (0);
}


/*
 * Free the macro trigger package
 */
errr macro_trigger_free(void)
{
	int i;
	int num;

	if (macro_template != NULL)
	{
		/* Free the template */
		string_free(macro_template);
		macro_template = NULL;

		/* Free the trigger names and keycodes */
		for (i = 0; i < max_macrotrigger; i++)
		{
			string_free(macro_trigger_name[i]);

			string_free(macro_trigger_keycode[0][i]);
			string_free(macro_trigger_keycode[1][i]);
		}

		/* No more macro triggers */
		max_macrotrigger = 0;

		/* Count modifier-characters */
		num = strlen(macro_modifier_chr);

		/* Free modifier names */
		for (i = 0; i < num; i++)
			string_free(macro_modifier_name[i]);

		/* Free modifier chars */
		string_free(macro_modifier_chr);
	}

	/* Success */
	return (0);
}
