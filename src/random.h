/* File: random.h */

#ifndef INCLUDED_RANDOM_H
#define INCLUDED_RANDOM_H

/*
 * Header file for the Random Number Generator
 *
 * If you get complaints about this file, it probably means
 * that you already HAVE the "random" package, and you do not
 * need to link it in again.
 *
 * Macintosh users will need this package.
 */


/**** Available functions ****/

extern long random(void);
extern void srandom(unsigned int);
extern char *initstate(unsigned int, char *, int);
extern char *setstate(char *);


#endif
