/*
 * File: randname.c
 * Purpose: Random name generation
 *
 * Copyright (c) 2007 Antony Sidwell, Sheldon Simms
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
#include "randname.h"

/* Markers for the start and end of words. */
#define S_WORD 26
#define E_WORD S_WORD
#define TOTAL  27

typedef unsigned short name_probs[S_WORD+1][S_WORD+1][TOTAL+1];

/*
 * This function builds probability tables from a list of purely alphabetical
 * lower-case words, and puts them into the supplied name_probs object.
 * The array of names should have a NULL entry at the end of the list.
 * It relies on the ASCII character set (through use of A2I).
 */
static void build_prob(name_probs probs, const char **learn)
{
	int c_prev, c_cur, c_next;
	const char *ch;
	int i;

	/* Build raw frequencies */
	for (i = 0; learn[i] != NULL; i++)
	{
		c_prev = c_cur = S_WORD;
		ch = learn[i];

		/* Iterate over the next word */
		while (*ch != '\0')
		{
			c_next = A2I(tolower((unsigned char)*ch));

			probs[c_prev][c_cur][c_next]++;
			probs[c_prev][c_cur][TOTAL]++;
                        
			/* Step on */
			c_prev = c_cur;
			c_cur = c_next;
			ch++;
		}

		probs[c_prev][c_cur][E_WORD]++;
		probs[c_prev][c_cur][TOTAL]++;
	}
}

/*
 * Use W. Sheldon Simms' random name generator algorithm (Markov Chain stylee).
 * 
 * Generate a random word using the probability tables we built earlier.  
 * Relies on the A2I and I2A macros (and so the ASCII character set) and 
 * is_a_vowel (so the basic 5 English vowels).
 */
size_t randname_make(randname_type name_type, size_t min, size_t max, char *word_buf, size_t buflen, const char ***sections)
{
	size_t lnum = 0;
	bool found_word = FALSE;

	static name_probs lprobs;
	static randname_type cached_type = RANDNAME_NUM_TYPES;

	assert(name_type > 0 && name_type < RANDNAME_NUM_TYPES);

	/* To allow for a terminating character */
	assert(buflen > max);

	/* We cache one set of probabilities, only regenerate when
	   the type changes.  It's as good a way as any for now.
	   Frankly, we could probably regenerate every time. */
	if (cached_type != name_type)
	{
		const char **wordlist = NULL;

		wordlist = sections[name_type];

		build_prob(lprobs, wordlist);

		cached_type = name_type;
	}
        
	/* Generate the actual word wanted. */
	while (!found_word)
	{
		char *cp = word_buf;
		int c_prev = S_WORD;
		int c_cur = S_WORD;
		int tries = 0;
		bool contains_vowel = FALSE;
		lnum = 0;

		/* We start the word again if we run out of space or have
		   had to have 10 goes to find a word that satisfies the
		   minimal conditions. */
		while (tries < 10 && lnum <= max && !found_word)
		{
			/* Pick the next letter based on a simple weighting
			  of which letters can follow the previous two */
			int r;
			int c_next = 0;

			assert(c_prev >= 0 && c_prev <= S_WORD);
			assert(c_cur >= 0 && c_cur <= S_WORD);

			r = randint0(lprobs[c_prev][c_cur][TOTAL]);

			while (r >= lprobs[c_prev][c_cur][c_next])
			{
				r -= lprobs[c_prev][c_cur][c_next];
				c_next++;
			}

			assert(c_next <= E_WORD);
			assert(c_next >= 0);
            
			if (c_next == E_WORD)
			{
				/* If we've reached the end, we check if we've
				   met the simple conditions, otherwise have
				   another go at choosing a letter for this
				   position. */
				if (lnum >= min && contains_vowel)
				{
					*cp = '\0';
					found_word = TRUE;
				}
				else
				{
					tries++;
				}
			}
			else
			{
				/* Add the letter to the word and move on. */
				*cp = I2A(c_next);

				if (is_a_vowel(*cp))
					contains_vowel = TRUE;

				cp++;
				lnum++;
				assert(c_next <= S_WORD);
				assert(c_next >= 0);
				c_prev = c_cur;
				c_cur = c_next;
			}
		}
	}

	return lnum;
}


/* 
 * To run standalone tests, #define RANDNAME_TESTING and link with
 *  with just z-rand.c from Angband. 
 */
#ifdef RANDNAME_TESTING

#include <stdio.h>
#include <time.h>


bool is_a_vowel(int ch)
{
	switch (ch)
	{
		case 'a':
		case 'e':
		case 'i':
		case 'o':
		case 'u':
			 return (TRUE);
	}

	return (FALSE);
}

int main(int argc, char *argv[])
{
	int i;
	char name[256];

	Rand_value = time(NULL);

	for (i = 0; i < 20; i++)
	{
		randname_make(RANDNAME_TOLKIEN, 5, 9, name, 256, name_sections);
		name[0] = toupper((unsigned char) name[0]);
		printf("%s\n", name);
	}

	return 0;
}

#endif
