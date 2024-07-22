/**
 * \file borg_util.c
 * \brief Utility functions like sorting
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

#include "borg-util.h"

#ifdef ALLOW_BORG

bool (*borg_sort_comp)(void *u, void *v, int a, int b);
void (*borg_sort_swap)(void *u, void *v, int a, int b);

/*
 * Borg's sorting algorithm -- quick sort in place
 *
 * Note that the details of the data we are sorting is hidden,
 * and we rely on the "ang_sort_comp()" and "ang_sort_swap()"
 * function hooks to interact with the data, which is given as
 * two pointers, and which may have any user-defined form.
 */
static void borg_sort_aux(void *u, void *v, int p, int q)
{
    int z, a, b;

    /* Done sort */
    if (p >= q)
        return;

    /* Pivot */
    z = p;

    /* Begin */
    a = p;
    b = q;

    /* Partition */
    while (true) {
        /* Slide i2 */
        while (!(*borg_sort_comp)(u, v, b, z))
            b--;

        /* Slide i1 */
        while (!(*borg_sort_comp)(u, v, z, a))
            a++;

        /* Done partition */
        if (a >= b)
            break;

        /* Swap */
        (*borg_sort_swap)(u, v, a, b);

        /* Advance */
        a++, b--;
    }

    /* Recurse left side */
    borg_sort_aux(u, v, p, b);

    /* Recurse right side */
    borg_sort_aux(u, v, b + 1, q);
}

/*
 * Borg's sorting algorithm -- quick sort in place
 *
 * Note that the details of the data we are sorting is hidden,
 * and we rely on the "ang_sort_comp()" and "ang_sort_swap()"
 * function hooks to interact with the data, which is given as
 * two pointers, and which may have any user-defined form.
 */
void borg_sort(void *u, void *v, int n)
{
    /* Sort the array */
    borg_sort_aux(u, v, 0, n - 1);
}

/*
 * Sorting hook -- comp function -- see below
 *
 * We use "u" to point to an array of strings, and "v" to point to
 * an array of indexes, and we sort them together by the strings.
 */
bool borg_sort_comp_hook(void *u, void *v, int a, int b)
{
    char   **text = (char **)(u);
    int16_t *what = (int16_t *)(v);

    int cmp;

    /* Compare the two strings */
    cmp = (strcmp(text[a], text[b]));

    /* Strictly less */
    if (cmp < 0)
        return true;

    /* Strictly more */
    if (cmp > 0)
        return false;

    /* Enforce "stable" sort */
    return (what[a] <= what[b]);
}

/*
 * Sorting hook -- swap function -- see below
 *
 * We use "u" to point to an array of strings, and "v" to point to
 * an array of indexes, and we sort them together by the strings.
 */
void borg_sort_swap_hook(void *u, void *v, int a, int b)
{
    char   **text = (char **)(u);
    int16_t *what = (int16_t *)(v);

    char   *texttmp;
    int16_t whattmp;

    /* Swap "text" */
    texttmp = text[a];
    text[a] = text[b];
    text[b] = texttmp;

    /* Swap "what" */
    whattmp = what[a];
    what[a] = what[b];
    what[b] = whattmp;
}

/*
 * get rid of whitespace
 *    NOTE: this modifies the initial string
 */
char *borg_trim(char *line)
{
    // Trim leading space
    while (isspace((unsigned char)*line))
        line++;

    if (*line == 0) // All spaces?
        return line;

    // Trim trailing space
    char *end = line + strlen(line) - 1;
    while (end > line && isspace((unsigned char)*end))
        end--;

    // Write new null terminator character
    end[1] = '\0';

    return line;
}


#endif
