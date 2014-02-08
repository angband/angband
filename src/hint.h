#ifndef HINT_H
#define HINT_H

/*
 * A hint.
 */
struct hint {
	char *hint;
	struct hint *next;
};

extern struct hint *hints; /* store.c */

#endif /* HINT_H */
