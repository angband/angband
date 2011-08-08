/*
 * File: z-bitflag.c
 * Purpose: Low-level bit vector manipulation
 *
 * Copyright (c) 2010 William L Moore
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

#include "z-bitflag.h"


/**
 * Tests if a flag is "on" in a bitflag set.
 *
 * TRUE is returned when `flag` is on in `flags`, and FALSE otherwise.
 * The flagset size is supplied in `size`.
 */
bool flag_has(const bitflag *flags, const size_t size, const int flag)
{
	const size_t flag_offset = FLAG_OFFSET(flag);
	const int flag_binary = FLAG_BINARY(flag);

	if (flag == FLAG_END) return FALSE;

	assert(flag_offset < size);

	if (flags[flag_offset] & flag_binary) return TRUE;

	return FALSE;
}

bool flag_has_dbg(const bitflag *flags, const size_t size, const int flag, const char *fi, const char *fl)
{
	const size_t flag_offset = FLAG_OFFSET(flag);
	const int flag_binary = FLAG_BINARY(flag);

	if (flag == FLAG_END) return FALSE;

	if (flag_offset >= size)
	{
		quit_fmt("Error in flag_has(%s, %s): FlagID[%d] Size[%u] FlagOff[%u] FlagBV[%d]\n",
		         fi, fl, flag, (unsigned int) size, (unsigned int) flag_offset, flag_binary);
	}

	assert(flag_offset < size);

	if (flags[flag_offset] & flag_binary) return TRUE;

	return FALSE;
}


/**
 * Interates over the flags which are "on" in a bitflag set.
 *
 * Returns the next on flag in `flags`, starting from (and including)
 * `flag`. FLAG_END will be returned when the end of the flag set is reached.
 * Iteration will start at the beginning of the flag set when `flag` is
 * FLAG_END. The bitfield size is supplied in `size`.
 */
int flag_next(const bitflag *flags, const size_t size, const int flag)
{
	const int max_flags = FLAG_MAX(size);
	int f, flag_offset, flag_binary;

	for (f = flag; f < max_flags; f++)
	{
		flag_offset = FLAG_OFFSET(f);
		flag_binary = FLAG_BINARY(f);

		if (flags[flag_offset] & flag_binary) return f;
	}

	return FLAG_END;
}


/**
 * Tests a bitfield for emptiness.
 *
 * TRUE is returned when no flags are set in `flags`, and FALSE otherwise.
 * The bitfield size is supplied in `size`.
 */
bool flag_is_empty(const bitflag *flags, const size_t size)
{
	size_t i;

	for (i = 0; i < size; i++)
		if (flags[i] > 0) return FALSE;

	return TRUE;
}


/**
 * Tests a bitfield for fullness.
 *
 * TRUE is returned when all flags are set in `flags`, and FALSE otherwise.
 * The bitfield size is supplied in `size`.
 */
bool flag_is_full(const bitflag *flags, const size_t size)
{
	size_t i;

	for (i = 0; i < size; i++)
		if (flags[i] != (bitflag) -1) return FALSE;

	return TRUE;
}


/**
 * Tests two bitfields for intersection.
 *
 * TRUE is returned when any flag is set in both `flags1` and `flags2`, and
 * FALSE otherwise. The size of the bitfields is supplied in `size`.
 */
bool flag_is_inter(const bitflag *flags1, const bitflag *flags2, const size_t size)
{
	size_t i;

	for (i = 0; i < size; i++)
		if (flags1[i] & flags2[i]) return TRUE;

	return FALSE;
}


/**
 * Test if one bitfield is a subset of another.
 *
 * TRUE is returned when every set flag in `flags2` is also set in `flags1`,
 * and FALSE otherwise. The size of the bitfields is supplied in `size`.
 */
bool flag_is_subset(const bitflag *flags1, const bitflag *flags2, const size_t size)
{
	size_t i;

	for (i = 0; i < size; i++)
		if (~flags1[i] & flags2[i]) return FALSE;

	return TRUE;
}


/**
 * Tests two bitfields for equality.
 *
 * TRUE is returned when the flags set in `flags1` and `flags2` are identical,
 * and FALSE otherwise. the size of the bitfields is supplied in `size`.
 */
bool flag_is_equal(const bitflag *flags1, const bitflag *flags2, const size_t size)
{
	return (!memcmp(flags1, flags2, size * sizeof(bitflag)));
}


/**
 * Sets one bitflag in a bitfield.
 *
 * The bitflag identified by `flag` is set in `flags`. The bitfield size is
 * supplied in `size`.  TRUE is returned when changes were made, FALSE
 * otherwise.
 */
bool flag_on(bitflag *flags, const size_t size, const int flag)
{
	const size_t flag_offset = FLAG_OFFSET(flag);
	const int flag_binary = FLAG_BINARY(flag);

	assert(flag_offset < size);

	if (flags[flag_offset] & flag_binary) return FALSE;

	flags[flag_offset] |= flag_binary;

	return TRUE;
}

bool flag_on_dbg(bitflag *flags, const size_t size, const int flag, const char *fi, const char *fl)
{
	const size_t flag_offset = FLAG_OFFSET(flag);
	const int flag_binary = FLAG_BINARY(flag);

	if (flag_offset >= size)
	{
		quit_fmt("Error in flag_on(%s, %s): FlagID[%d] Size[%u] FlagOff[%u] FlagBV[%d]\n",
		         fi, fl, flag, (unsigned int) size, (unsigned int) flag_offset, flag_binary);
	}

	assert(flag_offset < size);

	if (flags[flag_offset] & flag_binary) return FALSE;

	flags[flag_offset] |= flag_binary;

	return TRUE;
}


/**
 * Clears one flag in a bitfield.
 *
 * The bitflag identified by `flag` is cleared in `flags`. The bitfield size
 * is supplied in `size`.  TRUE is returned when changes were made, FALSE
 * otherwise.
 */
bool flag_off(bitflag *flags, const size_t size, const int flag)
{
	const size_t flag_offset = FLAG_OFFSET(flag);
	const int flag_binary = FLAG_BINARY(flag);

	assert(flag_offset < size);

	if (!(flags[flag_offset] & flag_binary)) return FALSE;

	flags[flag_offset] &= ~flag_binary;

	return TRUE;
}


/**
 * Clears all flags in a bitfield.
 *
 * All flags in `flags` are cleared. The bitfield size is supplied in `size`.
 */
void flag_wipe(bitflag *flags, const size_t size)
{
	memset(flags, 0, size * sizeof(bitflag));
}


/**
 * Sets all flags in a bitfield.
 *
 * All flags in `flags` are set. The bitfield size is supplied in `size`.
 */
void flag_setall(bitflag *flags, const size_t size)
{
	memset(flags, 255, size * sizeof(bitflag));
}


/**
 * Negates all flags in a bitfield.
 *
 * All flags in `flags` are toggled. The bitfield size is supplied in `size`.
 */
void flag_negate(bitflag *flags, const size_t size)
{
	size_t i;
	
	for (i = 0; i < size; i++)
		flags[i] = ~flags[i];
}


/**
 * Copies one bitfield into another.
 *
 * All flags in `flags2` are copied into `flags1`. The size of the bitfields is
 * supplied in `size`.
 */
void flag_copy(bitflag *flags1, const bitflag *flags2, const size_t size)
{
	memcpy(flags1, flags2, size * sizeof(bitflag));
}


/**
 * Computes the union of two bitfields.
 *
 * For every set flag in `flags2`, the corresponding flag is set in `flags1`.
 * The size of the bitfields is supplied in `size`. TRUE is returned when
 * changes were made, and FALSE otherwise.
 */
bool flag_union(bitflag *flags1, const bitflag *flags2, const size_t size)
{
	size_t i;
	bool delta = FALSE;

	for (i = 0; i < size; i++)
	{
		/* !flag_is_subset() */
		if (~flags1[i] & flags2[i]) delta = TRUE;

		flags1[i] |= flags2[i];
	}

	return delta;
}


/**
 * Computes the union of one bitfield and the complement of another.
 *
 * For every unset flag in `flags2`, the corresponding flag is set in `flags1`.
 * The size of the bitfields is supplied in `size`. TRUE is returned when
 * changes were made, and FALSE otherwise.
 */
bool flag_comp_union(bitflag *flags1, const bitflag *flags2, const size_t size)
{
	size_t i;
	bool delta = FALSE;

	for (i = 0; i < size; i++)
	{
		/* no equivalent fn */
		if (!(~flags1[i] & ~flags2[i])) delta = TRUE;

		flags1[i] |= ~flags2[i];
	}

	return delta;
}


/**
 * Computes the intersection of two bitfields.
 *
 * For every unset flag in `flags2`, the corresponding flag is cleared in
 * `flags1`. The size of the bitfields is supplied in `size`. TRUE is returned
 * when changes were made, and FALSE otherwise.
 */
bool flag_inter(bitflag *flags1, const bitflag *flags2, const size_t size)
{
	size_t i;
	bool delta = FALSE;

	for (i = 0; i < size; i++)
	{
		/* !flag_is_equal() */
		if (!(flags1[i] == flags2[i])) delta = TRUE;

		flags1[i] &= flags2[i];
	}

	return delta;

}


/**
 * Computes the difference of two bitfields.
 *
 * For every set flag in `flags2`, the corresponding flag is cleared in
 * `flags1`. The size of the bitfields is supplied in `size`. TRUE is returned
 * when changes were made, and FALSE otherwise.
 */
bool flag_diff(bitflag *flags1, const bitflag *flags2, const size_t size)
{
	size_t i;
	bool delta = FALSE;

	for (i = 0; i < size; i++)
	{
		/* flag_is_inter() */
		if (flags1[i] & flags2[i]) delta = TRUE;

		flags1[i] &= ~flags2[i];
	}

	return delta;
}





/**
 * Tests if any of multiple bitflags are set in a bitfield.
 *
 * TRUE is returned if any of the flags specified in `...` are set in `flags`,
 * FALSE otherwise. The bitfield size is supplied in `size`.
 *
 * WARNING: FLAG_END must be the final argument in the `...` list.
 */
bool flags_test(const bitflag *flags, const size_t size, ...)
{
	size_t flag_offset;
	int flag_binary;
	int f;
	va_list args;
	bool delta = FALSE;

	va_start(args, size);

	/* Process each flag in the va-args */
	for (f = va_arg(args, int); f != FLAG_END; f = va_arg(args, int))
	{
		flag_offset = FLAG_OFFSET(f);
		flag_binary = FLAG_BINARY(f);

		assert(flag_offset < size);

		/* flag_has() */
		if (flags[flag_offset] & flag_binary)
		{
			delta = TRUE;
			break;
		}
	}

	va_end(args);
	
	return delta;
}


/**
 * Tests if all of the multiple bitflags are set in a bitfield.
 *
 * TRUE is returned if all of the flags specified in `...` are set in `flags`,
 * FALSE otherwise. The bitfield size is supplied in `size`. 
 *
 * WARNING: FLAG_END must be the final argument in the `...` list.
 */
bool flags_test_all(const bitflag *flags, const size_t size, ...)
{
	size_t flag_offset;
	int flag_binary;
	int f;
	va_list args;
	bool delta = TRUE;

	va_start(args, size);

	/* Process each flag in the va-args */
	for (f = va_arg(args, int); f != FLAG_END; f = va_arg(args, int))
	{
		flag_offset = FLAG_OFFSET(f);
		flag_binary = FLAG_BINARY(f);

		assert(flag_offset < size);

		/* !flag_has() */
		if (!(flags[flag_offset] & flag_binary))
		{
			delta = FALSE;
			break;
		}
	}

	va_end(args);
	
	return delta;
}


/**
 * Clears multiple bitflags in a bitfield.
 *
 * The flags specified in `...` are cleared in `flags`. The bitfield size is
 * supplied in `size`. TRUE is returned when changes were made, FALSE
 * otherwise.
 *
 * WARNING: FLAG_END must be the final argument in the `...` list.
 */
bool flags_clear(bitflag *flags, const size_t size, ...)
{
	size_t flag_offset;
	int flag_binary;
	int f;
	va_list args;
	bool delta = FALSE;

	va_start(args, size);

	/* Process each flag in the va-args */
	for (f = va_arg(args, int); f != FLAG_END; f = va_arg(args, int))
	{
		flag_offset = FLAG_OFFSET(f);
		flag_binary = FLAG_BINARY(f);

		assert(flag_offset < size);

		/* flag_has() */
		if (flags[flag_offset] & flag_binary) delta = TRUE;

		/* flag_off() */
		flags[flag_offset] &= ~flag_binary;
	}

	va_end(args);

	return delta;
}


/**
 * Sets multiple bitflags in a bitfield.
 *
 * The flags specified in `...` are set in `flags`. The bitfield size is
 * supplied in `size`. TRUE is returned when changes were made, FALSE
 * otherwise.
 *
 * WARNING: FLAG_END must be the final argument in the `...` list.
 */
bool flags_set(bitflag *flags, const size_t size, ...)
{
	size_t flag_offset;
	int flag_binary;
	int f;
	va_list args;
	bool delta = FALSE;

	va_start(args, size);

	/* Process each flag in the va-args */
	for (f = va_arg(args, int); f != FLAG_END; f = va_arg(args, int))
	{
		flag_offset = FLAG_OFFSET(f);
		flag_binary = FLAG_BINARY(f);

		assert(flag_offset < size);

		/* !flag_has() */
		if (!(flags[flag_offset] & flag_binary)) delta = TRUE;

		/* flag_on() */
		flags[flag_offset] |= flag_binary;
	}

	va_end(args);

	return delta;
}


/**
 * Wipes a bitfield, and then sets multiple bitflags.
 *
 * The flags specified in `...` are set in `flags`, while all other flags are
 * cleared. The bitfield size is supplied in `size`.
 *
 * WARNING: FLAG_END must be the final argument in the `...` list.
 */
void flags_init(bitflag *flags, const size_t size, ...)
{
	int f;
	va_list args;

	flag_wipe(flags, size);

	va_start(args, size);

	/* Process each flag in the va-args */
	for (f = va_arg(args, int); f != FLAG_END; f = va_arg(args, int))
		flag_on(flags, size, f);

	va_end(args);
}


/**
 * Computes the intersection of a bitfield and multiple bitflags.
 *
 * The flags not specified in `...` are cleared in `flags`. The bitfeild size
 * is supplied in `size`. TRUE is returned when changes were made, FALSE
 * otherwise.
 *
 * WARNING: FLAG_END must be the final argument in the `...` list.
 */
bool flags_mask(bitflag *flags, const size_t size, ...)
{
	int f;
	va_list args;
	bool delta = FALSE;

	bitflag *mask;

	/* Build the mask */
	mask = C_ZNEW(size, bitflag);

	va_start(args, size);

	/* Process each flag in the va-args */
	for (f = va_arg(args, int); f != FLAG_END; f = va_arg(args, int))
		flag_on(mask, size, f);

	va_end(args);

	delta = flag_inter(flags, mask, size);

	/* Free the mask */

	FREE(mask);

	return delta;
}
