/*
 * monster/init.h
 *
 * noz: 1 Jun 2011
 *
 * Parsing functions for monsters and monster base types
 */

#ifndef MONSTER_INIT_H_
#define MONSTER_INIT_H_

extern const char *r_info_flags[];
extern const char *r_info_spell_flags[];
extern struct file_parser r_parser;
extern struct file_parser rb_parser;
void monsters_free(void);

#endif /* MONSTER_INIT_H_ */
