#ifndef INCLUDED_UI_SPELL_H
#define INCLUDED_UI_SPELL_H

int get_spell_from_book(const char *verb, struct object *book,
		const char *error, bool (*spell_filter)(int spell));
int get_spell(const char *verb, item_tester book_filter,
		cmd_code cmd, const char *error, bool (*spell_filter)(int spell));
void textui_book_browse(const object_type *o_ptr);
void textui_spell_browse(void);

#endif /* INCLUDED_UI_SPELL_H */