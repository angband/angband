/* target.h - target interface */

#ifndef TARGET_H
#define TARGET_H

/*
 * Convert a "key event" into a "location" (Y)
 */
#define KEY_GRID_Y(K) \
  ((int) (((K.mouse.y - ROW_MAP) / tile_height) + Term->offset_y))

/*
 * Convert a "key event" into a "location" (X)
 */
#define KEY_GRID_X(K) \
	((int) (((K.mouse.x - COL_MAP) / tile_width) + Term->offset_x))


/*
 * Bit flags for target_set()
 *
 *	KILL: Target monsters
 *	LOOK: Describe grid fully
 *	XTRA: Currently unused flag (NOT USED)
 *	GRID: Select from all grids (NOT USED)
 * QUIET: Prevent targeting messages.
 */
#define TARGET_KILL   0x01
#define TARGET_LOOK   0x02
#define TARGET_XTRA   0x04
#define TARGET_GRID   0x08
#define TARGET_QUIET  0x10

int motion_dir(int y1, int x1, int y2, int x2);
int target_dir(struct keypress ch);
int target_dir_allow(struct keypress ch, bool allow_5);
bool target_able(struct monster *m);
bool target_okay(void);
bool target_set_closest(int mode);
bool target_set_monster(struct monster *m);
void target_set_location(int y, int x);
bool target_set_interactive(int mode, int x, int y);
bool get_rep_dir(int *dp, bool allow_5);
bool get_aim_dir(int *dp);
void target_get(s16b *col, s16b *row);
struct monster *target_get_monster(void);
bool target_sighted(void);

#endif /* !TARGET_H */
