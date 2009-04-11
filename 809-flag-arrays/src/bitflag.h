/*
 * File: bitflag.h
 * Purpose: Manipulating bitflags.
 *
 * Copyright (c) 2007 Kenneth Boyd.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 */

/*
 * alternate flag approach for 32-bit bitvectors
 */
#define FLAG_FROM_INDEX(F)   (1UL << ((F)%32))
#define OFFSET_FROM_INDEX(F) ((F)/32)
#define TEST_FLAG(A,F)       (A[OFFSET_FROM_INDEX(F)] & FLAG_FROM_INDEX(F))
#define SET_FLAG(A,F)        (A[OFFSET_FROM_INDEX(F)] |= FLAG_FROM_INDEX(F))
#define RESET_FLAG(A,F)      (A[OFFSET_FROM_INDEX(F)] &= ~(FLAG_FROM_INDEX(F))
