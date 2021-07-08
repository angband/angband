#ifndef EFFECT_SOURCE_H
#define EFFECT_SOURCE_H

/*
 * Structure that tells you where an effect came from
 */
struct source {
	enum {
		SRC_NONE,
		SRC_TRAP,
		SRC_PLAYER,
		SRC_MONSTER,
		SRC_OBJECT,
		SRC_CHEST_TRAP
	} what;

	union {
		struct trap *trap;
		int monster;
		struct object *object;
		struct chest_trap *chest_trap;
	} which;
};

/*
 * Generate different forms of the source for projection and effect
 * functions
 */
struct source source_none(void);
struct source source_trap(struct trap *);
struct source source_monster(int who);
struct source source_player(void);
struct source source_object(struct object *);
struct source source_chest_trap(struct chest_trap *chest_trap);

#endif /* EFFECT_SOURCE_H */
