#ifndef GET_H
#define GET_H

struct getset {
	bool (*file)(const char *suggested_name, char *path, size_t len);
	bool (*string)(const char *prompt, char *buf, size_t len);
	s16b (*quantity)(const char *prompt, int max);
	bool (*check)(const char *prompt);
	bool (*item)(struct object *choice, const char *pmt, const char *fail, cmd_code cmd, item_filter filter, int mode);
	bool (*direction)(int *dir, bool allow_5);
	bool (*target)(int *dp);
	int (*spell)(const char *verb, bool (*spell_test)(int spell));
};

void set_getfunctions(struct getset *go);

/* Getters */
bool get_file(const char *suggested_name, char *path, size_t len);
bool get_string(const char *prompt, char *buf, size_t len);
s16b get_quantity(const char *prompt, int max);
bool get_check(const char *prompt);
bool get_item(struct object **choice, const char *pmt, const char *fail, cmd_code cmd, item_filter filter, int mode);
bool get_direction(int *dir, bool allow_5);
bool get_target(int *dp);
int get_spell(const char *verb, bool (*spell_test)(int spell));

#endif /* GET_H */
