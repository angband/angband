/* birth.h */

#ifndef BIRTH_H
#define BIRTH_H

#include "player/types.h"

extern void player_init(struct player *p);
extern void player_generate(struct player *p, const player_sex *s,
                            struct player_race *r, player_class *c);
extern char *get_history(struct history_chart *h, s16b *sc);
extern void wield_all(struct player *p);

#endif /* !BIRTH_H */
