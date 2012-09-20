/* File: random.h */

#ifndef INCLUDED_RANDOM_H
#define INCLUDED_RANDOM_H

#include "h-include.h"

/*
 * Header file for the Random Number Generator
 *
 * This library is a copy of the standard BSD "random" library,
 * with a few minor changes related to error messages and such.
 *
 * Also, to prevent annoying situations with having or not having
 * the built in library, I have simply *required* this library by
 * redefining the function names.  Ugly, but it should work fine.
 */


/**** Hack -- undefine the function names ****/

#undef random
#undef srandom
#undef initstate
#undef setstate


/**** Hack -- redefine the function names ****/

#define random random_hack
#define srandom srandom_hack
#define initstate initstate_hack
#define setstate setstate_hack


/**** Available functions ****/

extern s32b random(void);
extern void srandom(u32b);
extern char *initstate(u32b, char *, int);
extern char *setstate(char *);


#endif

