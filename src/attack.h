/* attack.h - attack interface */

#ifndef ATTACK_H
#define ATTACK_H

extern int breakage_chance(const object_type *o_ptr);
extern bool test_hit(int chance, int ac, int vis);
extern void py_attack(int y, int x);

#endif /* !ATTACK_H */
