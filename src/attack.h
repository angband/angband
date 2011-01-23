/* attack.h - attack interface */

#ifndef ATTACK_H
#define ATTACK_H

extern int breakage_chance(const object_type *o_ptr);
extern bool test_hit(int chance, int ac, int vis);
extern void py_attack(int y, int x);

struct attack_result {
    bool hit;
    int dmg;
    u32b msg_type;
    const char *hit_verb;
};

/*
* cave_builder is a function pointer which builds a level.
*/
typedef bool (*ranged_attack) (object_type *o_ptr, int y, int x, int *dmg, u32b *msg_type, const char *hit_verb);
typedef struct attack_result (*ranged_attack2) (object_type *o_ptr, int y, int x);

#endif /* !ATTACK_H */
