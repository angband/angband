
#include "get.h"
#include "init.h"

static struct getset = { 0 };

void set_getfunctions(struct getset *funcs) {
	memcpy(&getset, funcs, sizeof struct getset);
}


/* A version of get_file() built just using get_string() */
static bool get_file_backup(const char *suggested_name, char *path, size_t len) {
	char buf[160];

	/* Get filename */
	my_strcpy(buf, suggested_name, sizeof buf);
	if (!get_string("File name: ", buf, sizeof buf)) return FALSE;

	/* Make sure it's actually a filename */
	if (buf[0] == '\0' || buf[0] == ' ') return FALSE;

	/* Build the path */
	path_build(path, len, ANGBAND_DIR_USER, buf);

	/* Check if it already exists */
	if (file_exists(path) && !get_check("Replace existing file? "))
		return FALSE;

	/* Tell the user where it's saved to. */
	msg("Saving as %s.", path);

	return TRUE;
}

bool get_file(const char *suggested_name, char *path, size_t len) {
	if (getset.file)
		return getset.file(suggested_name, path, len);

	return get_file_backup(suggested_name, path, len);
}
bool get_string(const char *prompt, char *buf, size_t len) {
	assert(getset.string);
	return getset.file(prompt, buf, len);
}
s16b get_quantity(const char *prompt, int max) {
	assert(getset.quantity);
	return getset.quantity(prompt, max);
}
bool get_check(const char *prompt) {
	assert(getset.check);
	return getset.check(prompt);
}
bool get_item(struct object **choice, const char *pmt, const char *fail, cmd_code cmd, item_filter filter, int mode) {
	assert(getset.item);
	return getset.item(choice, pmt, fail, cmd, filter, mode);
}
bool get_direction(int *dir, bool allow_5) {
	assert(getset.direction);
	return getset.direction(dir, allow_5);
}
bool get_target(int *dp) {
	assert(getset.target);
	return getset.target(dp);
}
int get_spell(const char *verb, bool (*spell_test)(int spell)) {
	assert(getset.spell);
	return getset.spell(verb, spell_test);
}
