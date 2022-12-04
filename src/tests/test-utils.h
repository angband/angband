/* test-utils.h 
 *
 * Function prototypes for test-utils
 *
 * Created by: myshkin
 *             26 Apr 2011
 */

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "z-type.h"

void set_file_paths(void);
void read_edit_files(void);

/* Build an arena - an empty level with a permanent wall around the perimeter.
 * You can pass 0 for height or width, in which case the defaults from z_info
 * will be used. */
struct chunk *t_build_arena(int height, int width);

/* Generate a monster of the named race, place it at the given location, and
 * return it. This function cannot return NULL. */
struct monster *t_add_monster(struct chunk *c, struct loc g, const char *race);

#endif /* TEST_UTIL_H */
