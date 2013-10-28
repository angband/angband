/* attack.h - attack interface */

#ifndef ATTACK_H
#define ATTACK_H

extern int breakage_chance(const object_type *o_ptr, bool hit_target);
extern bool test_hit(int chance, int ac, int vis);
extern void py_attack(int y, int x);

/**
 *
 */
struct attack_result {
    bool success;
    int dmg;
    u32b msg_type;
    const char *hit_verb;
};

/**
 * ranged_attack is a function pointer, used to execute a kind of attack.
 *
 * This allows us to abstract details of throwing, shooting, etc. out while
 * keeping the core projectile tracking, monster cleanup, and display code
 * in common.
 */
typedef struct attack_result (*ranged_attack) (object_type *o_ptr, int y, int x);

#endif /* !ATTACK_H */
