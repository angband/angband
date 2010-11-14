/* squelch.h - squelch interface */

#ifndef SQUELCH_H
#define SQUELCH_H

/* squelch.c */
void squelch_init(void);
void squelch_birth_init(void);
int get_autoinscription_index(s16b k_idx);
const char *get_autoinscription(s16b kind_idx);
int apply_autoinscription(object_type *o_ptr);
int remove_autoinscription(s16b kind);
int add_autoinscription(s16b kind, cptr inscription);
void autoinscribe_ground(void);
void autoinscribe_pack(void);
bool squelch_tval(int tval);
void kind_squelch_clear(object_kind *k_ptr);
bool kind_is_squelched_aware(const object_kind *k_ptr);
bool kind_is_squelched_unaware(const object_kind *k_ptr);
void kind_squelch_when_aware(object_kind *k_ptr);
void kind_squelch_when_unaware(object_kind *k_ptr);
bool squelch_item_ok(const object_type *o_ptr);
bool squelch_hide_item(object_type *o_ptr);
void squelch_items(void);
void squelch_drop(void);
void do_cmd_options_item(const char *title, int row);
bool squelch_interactive(const object_type *o_ptr);

extern byte squelch_level[];
const size_t squelch_size;

#endif /* !SQUELCH_H */
