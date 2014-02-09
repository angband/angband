/* attack.h - attack interface */

#ifndef ATTACK_H
#define ATTACK_H

/* attack.c */
extern void do_cmd_fire(cmd_code code, cmd_arg args[]);
extern void textui_cmd_fire_at_nearest(void);
extern void do_cmd_throw(cmd_code code, cmd_arg args[]);
extern void textui_cmd_throw(void);


extern int breakage_chance(const object_type *o_ptr, bool hit_target);
extern bool test_hit(int chance, int ac, int vis);
extern void py_attack(int y, int x);
int py_attack_hit_chance(const object_type *weapon);

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
