/* target.h - target interface */

#ifndef TARGET_H
#define TARGET_H

bool target_able(int m_idx);
bool target_okay(void);
bool target_set_closest(int mode);
void target_set_monster(int m_idx);
void target_set_location(int y, int x);
bool target_set_interactive(int mode, int x, int y);
bool get_aim_dir(int *dp);
void target_get(int16_t *col, int16_t *row);
int16_t target_get_monster(void);

#endif /* !TARGET_H */
