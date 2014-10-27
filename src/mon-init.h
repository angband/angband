/**
 * mon-init.h
 *
 * noz: 1 Jun 2011
 *
 * Parsing functions for monsters and monster base types
 */

#ifndef MONSTER_INIT_H_
#define MONSTER_INIT_H_

extern const char *r_info_flags[];
extern const char *r_info_spell_flags[];
extern struct file_parser lore_parser;
extern struct file_parser mon_spell_parser;
extern struct file_parser r_parser;
extern struct file_parser mon_base_parser;

void write_flags(ang_file *fff, const char *intro_text, bitflag *flags,
				 int flag_size, const char *names[]);

#endif /* MONSTER_INIT_H_ */
