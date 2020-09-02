/**
 * \file z-bitflag.c
 * \brief Low-level bit vector manipulation
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
 * true is returned when `flag` is on in `flags`, and false otherwise.
 * The flagset size is supplied in `size`.
 */
bool flag_has(const bitflag *flags, const size_t size, const int flag)
{
	const size_t flag_offset = FLAG_OFFSET(flag);
	const int flag_binary = FLAG_BINARY(flag);

	if (flag == FLAG_END) return false;

	assert(flag_offset < size);

	if (flags[flag_offset] & flag_binary) return true;

	return false;
}

bool flag_has_dbg(const bitflag *flags, const size_t size, const int flag,
				  const char *fi, const char *fl)
{
	const size_t flag_offset = FLAG_OFFSET(flag);
	const int flag_binary = FLAG_BINARY(flag);

	if (flag == FLAG_END) return false;

	if (flag_offset >= size) {
		quit_fmt("Error in flag_has(%s, %s): FlagID[%d] Size[%u] FlagOff[%u] FlagBV[%d]\n",
		         fi, fl, flag, (unsigned int) size, (unsigned int) flag_offset, flag_binary);
	}

	assert(flag_offset < size);

	if (flags[flag_offset] & flag_binary) return true;

	return false;
}


/**
 * Iterates over the flags which are "on" in a bitflag set.
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

	for (f = flag; f < max_flags; f++) {
		flag_offset = FLAG_OFFSET(f);
		flag_binary = FLAG_BINARY(f);

		if (flags[flag_offset] & flag_binary) return f;
	}

	return FLAG_END;
}


/**
 * Counts the flags which are "on" in a bitflag set.
 *
 * The bitfield size is supplied in `size`.
 */
int flag_count(const bitflag *flags, const size_t size)
{
	size_t i, j;
	int count = 0;

	for (i = 0; i < size; i++) {
		for (j = 1; j <= FLAG_WIDTH; j++) {
			if (flags[i] & FLAG_BINARY(j)) {
				count++;
			}
		}
	}

	return count;
}


/**
 * Tests a bitfield for emptiness.
 *
 * true is returned when no flags are set in `flags`, and false otherwise.
 * The bitfield size is supplied in `size`.
 */
bool flag_is_empty(const bitflag *flags, const size_t size)
{
	size_t i;

	for (i = 0; i < size; i++)
		if (flags[i] > 0) return false;

	return true;
}


/**
 * Tests a bitfield for fullness.
 *
 * true is returned when all flags are set in `flags`, and false otherwise.
 * The bitfield size is supplied in `size`.
 */
bool flag_is_full(const bitflag *flags, const size_t size)
{
	size_t i;

	for (i = 0; i < size; i++)
		if (flags[i] != (bitflag) -1) return false;

	return true;
}


/**
 * Tests two bitfields for intersection.
 *
 * true is returned when any flag is set in both `flags1` and `flags2`, and
 * false otherwise. The size of the bitfields is supplied in `size`.
 */
bool flag_is_inter(const bitflag *flags1, const bitflag *flags2,
				   const size_t size)
{
	size_t i;

	for (i = 0; i < size; i++)
		if (flags1[i] & flags2[i]) return true;

	return false;
}


/**
 * Test if one bitfield is a subset of another.
 *
 * true is returned when every set flag in `flags2` is also set in `flags1`,
 * and false otherwise. The size of the bitfields is supplied in `size`.
 */
bool flag_is_subset(const bitflag *flags1, const bitflag *flags2,
					const size_t size)
{
	size_t i;

	for (i = 0; i < size; i++)
		if (~flags1[i] & flags2[i]) return false;

	return true;
}


/**
 * Tests two bitfields for equality.
 *
 * true is returned when the flags set in `flags1` and `flags2` are identical,
 * and false otherwise. the size of the bitfields is supplied in `size`.
 */
bool flag_is_equal(const bitflag *flags1, const bitflag *flags2,
				   const size_t size)
{
	return (!memcmp(flags1, flags2, size * sizeof(bitflag)));
}


/**
 * Sets one bitflag in a bitfield.
 *
 * The bitflag identified by `flag` is set in `flags`. The bitfield size is
 * supplied in `size`.  true is returned when changes were made, false
 * otherwise.
 */
bool flag_on(bitflag *flags, const size_t size, const int flag)
{
	const size_t flag_offset = FLAG_OFFSET(flag);
	const int flag_binary = FLAG_BINARY(flag);

	assert(flag_offset < size);

	if (flags[flag_offset] & flag_binary) return false;

	flags[flag_offset] |= flag_binary;

	return true;
}

bool flag_on_dbg(bitflag *flags, const size_t size, const int flag,
				 const char *fi, const char *fl)
{
	const size_t flag_offset = FLAG_OFFSET(flag);
	const int flag_binary = FLAG_BINARY(flag);

	if (flag_offset >= size) {
		quit_fmt("Error in flag_on(%s, %s): FlagID[%d] Size[%u] FlagOff[%u] FlagBV[%d]\n",
		         fi, fl, flag, (unsigned int) size, (unsigned int) flag_offset, flag_binary);
	}

	assert(flag_offset < size);

	if (flags[flag_offset] & flag_binary) return false;

	flags[flag_offset] |= flag_binary;

	return true;
}


/**
 * Clears one flag in a bitfield.
 *
 * The bitflag identified by `flag` is cleared in `flags`. The bitfield size
 * is supplied in `size`.  true is returned when changes were made, false
 * otherwise.
 */
bool flag_off(bitflag *flags, const size_t size, const int flag)
{
	const size_t flag_offset = FLAG_OFFSET(flag);
	const int flag_binary = FLAG_BINARY(flag);

	assert(flag_offset < size);

	if (!(flags[flag_offset] & flag_binary)) return false;

	flags[flag_offset] &= ~flag_binary;

	return true;
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
 * The size of the bitfields is supplied in `size`. true is returned when
 * changes were made, and false otherwise.
 */
bool flag_union(bitflag *flags1, const bitflag *flags2, const size_t size)
{
	size_t i;
	bool delta = false;

	for (i = 0; i < size; i++) {
		/* !flag_is_subset() */
		if (~flags1[i] & flags2[i]) delta = true;

		flags1[i] |= flags2[i];
	}

	return delta;
}


/**
 * Computes the intersection of two bitfields.
 *
 * For every unset flag in `flags2`, the corresponding flag is cleared in
 * `flags1`. The size of the bitfields is supplied in `size`. true is returned
 * when changes were made, and false otherwise.
 */
bool flag_inter(bitflag *flags1, const bitflag *flags2, const size_t size)
{
	size_t i;
	bool delta = false;

	for (i = 0; i < size; i++) {
		/* !flag_is_equal() */
		if (!(flags1[i] == flags2[i])) delta = true;

		flags1[i] &= flags2[i];
	}

	return delta;

}


/**
 * Computes the difference of two bitfields.
 *
 * For every set flag in `flags2`, the corresponding flag is cleared in
 * `flags1`. The size of the bitfields is supplied in `size`. true is returned
 * when changes were made, and false otherwise.
 */
bool flag_diff(bitflag *flags1, const bitflag *flags2, const size_t size)
{
	size_t i;
	bool delta = false;

	for (i = 0; i < size; i++) {
		/* flag_is_inter() */
		if (flags1[i] & flags2[i]) delta = true;

		flags1[i] &= ~flags2[i];
	}

	return delta;
}





/**
 * Tests if any of multiple bitflags are set in a bitfield.
 *
 * true is returned if any of the flags specified in `...` are set in `flags`,
 * false otherwise. The bitfield size is supplied in `size`.
 *
 * WARNING: FLAG_END must be the final argument in the `...` list.
 */
bool flags_test(const bitflag *flags, const size_t size, ...)
{
	size_t flag_offset;
	int flag_binary;
	int f;
	va_list args;
	bool delta = false;

	va_start(args, size);

	/* Process each flag in the va-args */
	for (f = va_arg(args, int); f != FLAG_END; f = va_arg(args, int)) {
		flag_offset = FLAG_OFFSET(f);
		flag_binary = FLAG_BINARY(f);

		assert(flag_offset < size);

		/* flag_has() */
		if (flags[flag_offset] & flag_binary) {
			delta = true;
			break;
		}
	}

	va_end(args);
	
	return delta;
}


/**
 * Tests if all of the multiple bitflags are set in a bitfield.
 *
 * true is returned if all of the flags specified in `...` are set in `flags`,
 * false otherwise. The bitfield size is supplied in `size`. 
 *
 * WARNING: FLAG_END must be the final argument in the `...` list.
 */
bool flags_test_all(const bitflag *flags, const size_t size, ...)
{
	size_t flag_offset;
	int flag_binary;
	int f;
	va_list args;
	bool delta = true;

	va_start(args, size);

	/* Process each flag in the va-args */
	for (f = va_arg(args, int); f != FLAG_END; f = va_arg(args, int)) {
		flag_offset = FLAG_OFFSET(f);
		flag_binary = FLAG_BINARY(f);

		assert(flag_offset < size);

		/* !flag_has() */
		if (!(flags[flag_offset] & flag_binary)) {
			delta = false;
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
 * supplied in `size`. true is returned when changes were made, false
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
	bool delta = false;

	va_start(args, size);

	/* Process each flag in the va-args */
	for (f = va_arg(args, int); f != FLAG_END; f = va_arg(args, int)) {
		flag_offset = FLAG_OFFSET(f);
		flag_binary = FLAG_BINARY(f);

		assert(flag_offset < size);

		/* flag_has() */
		if (flags[flag_offset] & flag_binary) delta = true;

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
 * supplied in `size`. true is returned when changes were made, false
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
	bool delta = false;

	va_start(args, size);

	/* Process each flag in the va-args */
	for (f = va_arg(args, int); f != FLAG_END; f = va_arg(args, int)) {
		flag_offset = FLAG_OFFSET(f);
		flag_binary = FLAG_BINARY(f);

		assert(flag_offset < size);

		/* !flag_has() */
		if (!(flags[flag_offset] & flag_binary)) delta = true;

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
 * is supplied in `size`. true is returned when changes were made, false
 * otherwise.
 *
 * WARNING: FLAG_END must be the final argument in the `...` list.
 */
bool flags_mask(bitflag *flags, const size_t size, ...)
{
	int f;
	va_list args;
	bool delta = false;

	bitflag *mask;

	/* Build the mask */
	mask = mem_zalloc(size * sizeof(bitflag));

	va_start(args, size);

	/* Process each flag in the va-args */
	for (f = va_arg(args, int); f != FLAG_END; f = va_arg(args, int))
		flag_on(mask, size, f);

	va_end(args);

	delta = flag_inter(flags, mask, size);

	/* Free the mask */

	mem_free(mask);

	return delta;
}
