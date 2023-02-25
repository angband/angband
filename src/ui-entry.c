/**
 * \file ui-entry.c
 * \brief Definitions to link object/player properties to 2nd character screen
 *
 * Copyright (c) 2020 - 2021 Eric Branlund
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include "init.h"
#include "object.h"
#include "obj-curse.h"
#include "obj-gear.h"
#include "obj-knowledge.h"
#include "obj-util.h"
#include "player.h"
#include "player-timed.h"
#include "ui-entry.h"
#include "ui-entry-combiner.h"
#include "ui-entry-init.h"
#include "ui-entry-renderers.h"
#include "z-util.h"
#include "z-virt.h"

/*
 * This is the maximum number of characters stored for a label.  Used to
 * limit what's extracted from the configuration file.
 */
#define MAX_ENTRY_LABEL (80)

struct ui_entry_iterator {
	struct ui_entry **entries;
	int n, i;
};

struct cached_object_data {
	bitflag f[OF_SIZE];
};

struct cached_player_data {
	bitflag untimed[OF_SIZE];
	bitflag timed[OF_SIZE];
};

struct category_reference {
	const char *name;
	int priority;
	bool priority_set;
};

struct bound_object_property {
	int type;
	int index;
	int value;
	bool have_value;
	bool isaux;
};

struct bound_player_ability {
	struct player_ability *ability;
	int value;
	bool have_value;
	bool isaux;
};


enum {
	ENTRY_FLAG_TIMED_AUX = 1,
	/* Used internally; not set from within the configuration files. */
	ENTRY_FLAG_TEMPLATE_ONLY = (1UL << 20)
};
struct entry_flag {
	const char *name;
	int value;
};
static struct entry_flag entry_flags[] = {
	{ "TIMED_AS_AUX", ENTRY_FLAG_TIMED_AUX },
};

/*
 * Set the maximum number of shortened versions of the label that will be
 * accepted.  The shortened versions will have lengths of one to MAX_SHORTENED
 * characters, not including the terminating null.
 */
#define MAX_SHORTENED (10)
struct ui_entry {
	char *name;
	struct category_reference *categories;
	struct bound_object_property *obj_props;
	struct bound_player_ability *p_abilities;
	wchar_t *label;
	wchar_t *shortened_labels[MAX_SHORTENED];
	wchar_t shortened_buffer[MAX_SHORTENED + (MAX_SHORTENED * (MAX_SHORTENED + 1)) / 2];
	int nshortened[MAX_SHORTENED];
	int nlabel;
	int renderer_index;
	int combiner_index;
	int default_priority;
	int param_index;
	int flags;
	int n_category;
	int nalloc_category;
	int n_obj_prop;
	int nalloc_obj_prop;
	int n_p_ability;
	int nalloc_p_ability;
};

static int ui_entry_search(const char *name, int *ind);
static int ui_entry_search_categories(const struct ui_entry *entry,
	const char *name, int *ind);
static void modifier_to_skill(int modind, int *skillind, int *skill2mod_num,
	int *skill2mod_den
);
static int get_timed_element_effect(const struct player *p, int ind);
static int get_timed_modifier_effect(const struct player *p, int ind);

struct ui_entry_name_parameter {
	const char *name;
	int (*count_func)(void);
	const char *(*ith_name_func)(int i);
};

static int get_dummy_param_count(void);
static const char *get_dummy_param_name(int i);
static int get_element_count(void);
static const char *get_element_name(int i);
static int get_stat_count(void);
static const char *get_stat_name(int i);

static struct ui_entry_name_parameter name_parameters[] = {
	{ "", get_dummy_param_count, get_dummy_param_name },
	{ "element", get_element_count, get_element_name },
	{ "stat", get_stat_count, get_stat_name },
};

struct ui_entry_priority_scheme {
	const char *name;
	int (*priority)(int i);
};

static int get_dummy_priority(int i);
static int get_priority_from_index(int i);
static int get_priority_from_negative_index(int i);

static struct ui_entry_priority_scheme priority_schemes[] = {
	{ "", get_dummy_priority },
	{ "index", get_priority_from_index },
	{ "negative_index", get_priority_from_negative_index },
};

struct embryonic_category_reference {
	const char *name;
	int psource_index;
	int priority;
	bool priority_set;
};

struct embryonic_ui_entry {
	struct ui_entry *entry;
	struct embryonic_category_reference *categories;
	int param_index;
	int psource_index;
	int last_category_index;
	bool exists;
};

static int n_category = 0;
static int nalloc_category = 0;
static char **categories = NULL;

static int n_entry = 0;
static int nalloc_entry = 0;
static struct ui_entry **entries = NULL;


/**
 * Binds an object property, given by type and index, to a user interface
 * entry configured in ui_entry.txt.  If name isn't configured in that file
 * returns a nonzero value.  Otherwise returns zero after binding the property.
 * Currently, this is only used for some parts of the second character screen.
 */
int bind_object_property_to_ui_entry_by_name(const char *name, int type,
	int index, int value, bool have_value, bool isaux)
{
	int ind;

	if (! ui_entry_search(name, &ind)) {
		return 1;
	}
	if (entries[ind]->n_obj_prop == entries[ind]->nalloc_obj_prop) {
		struct bound_object_property *nblk;

		if (entries[ind]->nalloc_obj_prop > INT_MAX / 2) {
			return 2;
		}
		entries[ind]->nalloc_obj_prop =
			(entries[ind]->nalloc_obj_prop == 0) ?
			4 : 2 * entries[ind]->nalloc_obj_prop;
		nblk = mem_alloc(entries[ind]->nalloc_obj_prop *
			sizeof(*nblk));
		if (entries[ind]->n_obj_prop > 0) {
			(void) memcpy(nblk, entries[ind]->obj_props,
				entries[ind]->n_obj_prop * sizeof(*nblk));
		}
		mem_free(entries[ind]->obj_props);
		entries[ind]->obj_props = nblk;
	}
	entries[ind]->obj_props[entries[ind]->n_obj_prop].type = type;
	entries[ind]->obj_props[entries[ind]->n_obj_prop].index = index;
	entries[ind]->obj_props[entries[ind]->n_obj_prop].value = value;
	entries[ind]->obj_props[entries[ind]->n_obj_prop].have_value =
		have_value;
	entries[ind]->obj_props[entries[ind]->n_obj_prop].isaux = isaux;
	++entries[ind]->n_obj_prop;
	return 0;
}


/**
 * Binds a player ability to to a user interface entry configured in ui_entry.
 * If name isn't configured in that file returns a nonzero value.  Otherwise
 * returns zero after binding the ability.  Currently, this is only used for
 * some parts of the second character screen.
 */
int bind_player_ability_to_ui_entry_by_name(const char *name,
	struct player_ability *ability, int value, bool have_value, bool isaux)
{
	int ind;

	if (! ui_entry_search(name, &ind)) {
		return 1;
	}
	if (entries[ind]->n_p_ability ==
		entries[ind]->nalloc_p_ability) {
		struct bound_player_ability *abilities;

		if (entries[ind]->nalloc_p_ability > INT_MAX / 2) {
			return 2;
		}
		entries[ind]->nalloc_p_ability =
			(entries[ind]->nalloc_p_ability == 0) ?
			4 : 2 * entries[ind]->nalloc_p_ability;
		abilities = mem_alloc(entries[ind]->nalloc_p_ability *
			sizeof(*abilities));
		if (entries[ind]->n_p_ability > 0) {
			(void) memcpy(abilities, entries[ind]->p_abilities,
				entries[ind]->n_p_ability *
				sizeof(*abilities));
		}
		mem_free(entries[ind]->p_abilities);
		entries[ind]->p_abilities = abilities;
	}
	entries[ind]->p_abilities[entries[ind]->n_p_ability].ability = ability;
	entries[ind]->p_abilities[entries[ind]->n_p_ability].value = value;
	entries[ind]->p_abilities[entries[ind]->n_p_ability].have_value =
		have_value;
	entries[ind]->p_abilities[entries[ind]->n_p_ability].isaux = isaux;
	++entries[ind]->n_p_ability;
	return 0;
}


/**
 * Returns true if the given user interface entry was configured in
 * ui_entry.txt to be part of the category with the given name.  Otherwise,
 * returns false.
 */
bool ui_entry_has_category(const struct ui_entry *entry, const char *name)
{
	int ind;

	return ui_entry_search_categories(entry, name, &ind) != 0;
}


/**
 * Fills label with length characters, including a terminating null, of a
 * label for a user interface entry configured in ui_entry.txt.  If the label
 * is naturally shorter than the specified length, the label will be padded
 * with spaces, either on the left if pad_left is true, or on the right if
 * pad_left is false.
 */
void get_ui_entry_label(const struct ui_entry *entry, int length,
	bool pad_left, wchar_t *label)
{
	static bool first_call = true;
	static wchar_t spc[2];
	const wchar_t *src;
	int n;

	if (first_call) {
		size_t nw = text_mbstowcs(spc, " ", 2);

		if (nw == (size_t)-1) assert(0);
		first_call = false;
	}

	if (length <= 0) {
		return;
	}
	if (length == 1) {
		label[0] = spc[1];
		return;
	}
	if (length <= MAX_SHORTENED + 1) {
		src = entry->shortened_labels[length - 2];
		n = entry->nshortened[length - 2];
	} else {
		src = entry->label;
		n = entry->nlabel;
	}
	if (n < length - 1) {
		int i;

		if (pad_left) {
			for (i = 0; i < length - 1 - n; ++i) {
				label[i] = spc[0];
			}
			(void) memcpy(label + length - 1 - n, src,
				n * sizeof(*label));
		} else {
			(void) memcpy(label, src, n * sizeof(*label));
			for (i = n; i < length - 1; ++i) {
				label[i] = spc[0];
			}
		}
	} else {
		(void) memcpy(label, src, (length - 1) * sizeof(*label));
	}
	label[length - 1] = spc[1];
}


static const char *category_for_cmp_desc_prio = NULL;
static int cmp_desc_prio(const void *left, const void *right)
{
	const struct ui_entry *eleft =
		*((const struct ui_entry* const *) left);
	const struct ui_entry *eright =
		*((const struct ui_entry* const *) right);
	int left_ind = -1, right_ind = -1;
	int result;

	if (category_for_cmp_desc_prio) {
		int ind;

		if (ui_entry_search_categories(eleft,
			category_for_cmp_desc_prio, &ind)) {
			left_ind = ind;
		}
		if (ui_entry_search_categories(eright,
			category_for_cmp_desc_prio, &ind)) {
			right_ind = ind;
		}
	}
	if (left_ind >= 0) {
		if (right_ind >= 0) {
			if (eleft->categories[left_ind].priority >
				eright->categories[right_ind].priority) {
				result = -1;
			} else if (eleft->categories[left_ind].priority <
				   eright->categories[right_ind].priority) {
				result = 1;
			} else {
				result = strcmp(eleft->name, eright->name);
			}
		} else {
			/*
			 * right is not in the sort category so it should be
			 * pushed toward the end.
			 */
			result = -1;
		}
	} else if (right_ind >= 0) {
		/*
		 * left is not in the sort category so it should be pushed
		 * towards the end.
		 */
		result = 1;
	} else {
		result = strcmp(eleft->name, eright->name);
	}
	return result;
}


/**
 * Constructs an iterator to enumerate all the user interface elements for
 * which the given predicate returns true.  The iterator will present those
 * elements in descending order of priority where the priority is that
 * configured for the element in the category named sortcategory.
 */
struct ui_entry_iterator *initialize_ui_entry_iterator(
	ui_entry_predicate predicate, void *closure, const char *sortcategory)
{
	struct ui_entry_iterator *result = mem_alloc(sizeof(*result));
	int i;

	result->entries = mem_alloc(n_entry * sizeof(*result->entries));
	result->n = 0;
	result->i = 0;
	for (i = 0; i < n_entry; ++i) {
		if (! (entries[i]->flags & ENTRY_FLAG_TEMPLATE_ONLY) &&
			(*predicate)(entries[i], closure)) {
			result->entries[result->n] = entries[i];
			++result->n;
		}
	}
	category_for_cmp_desc_prio = sortcategory;
	sort(result->entries, result->n, sizeof(*result->entries),
		cmp_desc_prio);
	return result;
}


/**
 * Releases the resources allocated by a prior call to
 * initialize_ui_entry_iterator.
 */
void release_ui_entry_iterator(struct ui_entry_iterator *i)
{
	mem_free(i->entries);
	mem_free(i);
}


/**
 * Resets the given iterator to the position it ahd when returned by
 * initialize_ui_entry_iterator.
 */
void reset_ui_entry_iterator(struct ui_entry_iterator *i)
{
	i->i = 0;
}


/**
 * Returns the number of elements remaining to be iterated for the given
 * iterator.
 */
int count_ui_entry_iterator(struct ui_entry_iterator *i)
{
	return i->n - i->i;
}


/**
 * Returns the user interface entry currently pointed to by the iterator and
 * advances the iterator.
 */
struct ui_entry *advance_ui_entry_iterator(struct ui_entry_iterator *i)
{
	struct ui_entry *result = i->entries[i->i];
	++i->i;
	return result;
}


/**
 * Returns the combiner index, suitable as the first argument to
 * ui_entry_combiner_get_funcs(), for the given user interface element.
 */
int get_ui_entry_combiner_index(const struct ui_entry *entry)
{
	return entry->combiner_index;
}


/**
 * Returns the renderer index, suitable as the first argument to
 * ui_entry_renderer_apply(), for the given user interface element.
 */
int get_ui_entry_renderer_index(const struct ui_entry *entry)
{
	return entry->renderer_index;
}


/**
 * Returns true if the properties/abilities bound to a user interface entry
 * correspond to a known rune.  Otherwise, returns false.
 */
bool is_ui_entry_for_known_rune(const struct ui_entry *entry,
	const struct player *p)
{
	bool result = true;
	int i;

	/*
	 * Mark it as known if all of the properties/abilities bound to the
	 * entry are known.
	 */
	for (i = 0; i < entry->n_obj_prop && result; ++i) {
		int ind = entry->obj_props[i].index;

		switch (entry->obj_props[i].type) {
		case OBJ_PROPERTY_STAT:
		case OBJ_PROPERTY_MOD:
			if (p->obj_k->modifiers[ind] == 0) {
				result = false;
			}
			break;

		case OBJ_PROPERTY_FLAG:
			if (! of_has(p->obj_k->flags, ind)) {
				result = false;
			}
			break;

		case OBJ_PROPERTY_IGNORE:
		case OBJ_PROPERTY_RESIST:
		case OBJ_PROPERTY_VULN:
		case OBJ_PROPERTY_IMM:
			if (p->obj_k->el_info[ind].res_level == 0) {
				result = false;
			}
			break;

		default:
			result = false;
			break;
		}
	}
	for (i = 0; i < entry->n_p_ability && result; ++i) {
		int ind = entry->p_abilities[i].ability->index;

		if (streq(entry->p_abilities[i].ability->type, "player")) {
			/*
			 * Not so easy to associate with a rune so don't let
			 * it change the result.
			 */
			continue;
		} else if (streq(entry->p_abilities[i].ability->type,
			"object")) {
			if (! of_has(p->obj_k->flags, ind)) {
				result = false;
			}
		} else if (streq(entry->p_abilities[i].ability->type,
			"element")) {
			if (p->obj_k->el_info[ind].res_level == 0) {
				result = false;
			}
		} else {
			result = false;
		}
	}
	return result;
}


/**
 * For a given object and set of object properties bound to a ui_entry, loop
 * over those properties and report the combined value of them for the object.
 * \param entry Is the ui_entry to use.  It is the source of the object
 * properties that will be combined and the algorithm for combining the values.
 * \param obj Is the object to assess.  May be NULL.
 * \param p Is the player used to assess whether an object property is known.
 * May be NULL to assume that all the properties are known.
 * \param cache If *cache is not NULL, *cache is assumed to have been
 * initialized by a prior call to compute_ui_entry_values_for_object() for the
 * same object and for a player whose state of knowledge is the same as p's.
 * If *cache is NULL, it will be initialized by this call and is specific to
 * obj and the state of p's knowledge.  The cache is to optimize away repeated
 * calculations that would be performed when looping over multiple ui_entry
 * structures and calling this function for each of them with the same object
 * and player.  Once it is no longer needed, *cache should be passed to
 * release_cached_object_data() to release the resources associated with it.
 * \param val At exit, *val will be the combined value for the object across
 * the non-auxiliary object properties bound to entry.  It will be
 *   a) UI_ENTRY_VALUE_NOT_PRESENT if obj is NULL
 *   b) 0 if all the properties bound to entry are auxiliary properties
 *   c) UI_ENTRY_UNKNOWN_VALUE if there's at least one non-auxiliary property
 *   bound to entry but all such properties are unknown to p
 *   d) the combined value of the known non-auxiliary properties if there's at
 *   least one non-auxiliary property bound to entry and at least one such
 *   property is known to p
 * The value of *val at entry is not used.
 * One use of auxiliary properties is to have the modifier for a stat be bound
 * to an entry and have the sustain for the stat be bound to the same entry as
 * an auxiliary property.
 * \param auxval At exit, *auxval will be the combined value for the object
 * across the auxiliary object properties bound to entry.  It will be
 *   a) UI_ENTRY_VALUE_NOT_PRESENT if obj is NULL
 *   b) 0 if all the properties bound to entry are non-auxiliary properties
 *   c) UI_ENTRY_UNKNOWN_VALUE if there's at least one auxiliary property
 *   bound to entry but all such properties are unknown to p
 *   d) the combined value of the known auxiliary properties if there's at
 *   least one auxiliary property bound to entry and at least one such
 *   property is known to p
 * The value of *auxval at entry is not used.
 */
void compute_ui_entry_values_for_object(const struct ui_entry *entry,
	const struct object *obj, const struct player *p,
	struct cached_object_data **cache, int *val, int *auxval)
{
	struct ui_entry_combiner_state cst = { 0, 0, 0 };
	struct ui_entry_combiner_funcs combiner;
	const struct curse_data *curse;
	struct cached_object_data *cache2;
	bool first, all_unknown, all_aux_unknown, any_aux, all_aux;
	bool any_curse_unknown;
	int curse_ind;

	if (!obj || !entry->n_obj_prop) {
		*val = UI_ENTRY_VALUE_NOT_PRESENT;
		*auxval = UI_ENTRY_VALUE_NOT_PRESENT;
		return;
	}
	if (*cache == NULL) {
		*cache = mem_alloc(sizeof(**cache));
		of_wipe((*cache)->f);
		if (p) {
			object_flags_known(obj, (*cache)->f);
		} else {
			object_flags(obj, (*cache)->f);
		}
	}
	first = true;
	all_unknown = true;
	all_aux_unknown = true;
	any_aux = false;
	all_aux = true;
	if (ui_entry_combiner_get_funcs(entry->combiner_index, &combiner)) {
		assert(0);
	}
	cache2 = *cache;
	curse = obj->curses;
	any_curse_unknown = false;
	curse_ind = 0;
	while (obj) {
		int i;

		for (i = 0; i < entry->n_obj_prop; ++i) {
			int ind = entry->obj_props[i].index;

			if (entry->obj_props[i].isaux) {
				if (entry->flags & ENTRY_FLAG_TIMED_AUX) {
					continue;
				}
				any_aux = true;
			} else {
				all_aux = false;
			}

			switch (entry->obj_props[i].type) {
			case OBJ_PROPERTY_STAT:
			case OBJ_PROPERTY_MOD:
				if (!p || p->obj_k->modifiers[ind] != 0 ||
					obj->modifiers[ind] == 0) {
					int v = obj->modifiers[ind];
					int a = 0;

					if (v && entry->obj_props[i].have_value) {
						v = entry->obj_props[i].value;
					}
					if (entry->obj_props[i].isaux) {
						int t = a;

						a = v;
						v = t;
						all_aux_unknown = false;
					} else {
						all_unknown = false;
					}
					if (first) {
						(*combiner.init_func)(v, a, &cst);
						first = false;
					} else {
						(*combiner.accum_func)(v, a, &cst);
					}
				} else if (curse_ind > 0) {
					any_curse_unknown = true;
				}
				break;

			case OBJ_PROPERTY_FLAG:
				if (!p || object_flag_is_known(p, obj, ind)) {
					int v = of_has(cache2->f, ind) ? 1 : 0;
					int a = 0;

					if (v && entry->obj_props[i].have_value) {
						v = entry->obj_props[i].value;
					}
					if (entry->obj_props[i].isaux) {
						int t = a;

						a = v;
						v = t;
						all_aux_unknown = false;
					} else {
						all_unknown = false;
					}
					if (first) {
						(*combiner.init_func)(v, a, &cst);
						first = false;
					} else {
						(*combiner.accum_func)(v, a, &cst);
					}
				} else if (curse_ind > 0) {
					any_curse_unknown = true;
				}
				break;

			case OBJ_PROPERTY_IGNORE:
				if (!p || object_element_is_known(p, obj, ind)) {
					int v = (obj->el_info[ind].flags &
						EL_INFO_IGNORE) ? 1 : 0;
					int a = 0;

					if (v && entry->obj_props[i].have_value) {
						v = entry->obj_props[i].value;
					}
					if (entry->obj_props[i].isaux) {
						int t = a;

						a = v;
						v = t;
						all_aux_unknown = false;
					} else {
						all_unknown = false;
					}
					if (first) {
						(*combiner.init_func)(v, a, &cst);
						first = false;
					} else {
						(*combiner.accum_func)(v, a, &cst);
					}
				} else if (curse_ind > 0) {
					any_curse_unknown = true;
				}
				break;

			case OBJ_PROPERTY_RESIST:
			case OBJ_PROPERTY_VULN:
			case OBJ_PROPERTY_IMM:
				if (!p || object_element_is_known(p, obj, ind)) {
					int v = obj->el_info[ind].res_level;
					int a = 0;

					if (v && entry->obj_props[i].have_value) {
						v = entry->obj_props[i].value;
					}
					if (entry->obj_props[i].isaux) {
						int t = a;

						a = v;
						v = t;
						all_aux_unknown = false;
					} else {
						all_unknown = false;
					}
					if (first) {
						(*combiner.init_func)(v, a, &cst);
						first = false;
					} else {
						(*combiner.accum_func)(v, a, &cst);
					}
				} else if (curse_ind > 0) {
					any_curse_unknown = true;
				}
				break;

			default:
				break;
			}
		}

		if (curse) {
			/*
			 * Proceed to the next unprocessed curse object.
			 * Don't overwrite the cached data for the base.
			 */
			obj = NULL;
			if (curse_ind == 0) {
				cache2 = mem_alloc(sizeof(*cache2));
			}
			++curse_ind;
			while (1) {
				if (curse_ind >= z_info->curse_max) {
					mem_free(cache2);
					break;
				}
				if (curse[curse_ind].power) {
					obj = curses[curse_ind].obj;
					of_wipe(cache2->f);
					if (p) {
						object_flags_known(obj,
							cache2->f);
					} else {
						object_flags(obj, cache2->f);
					}
					break;
				}
				++curse_ind;
			}
		} else {
			obj = NULL;
		}
	}
	if (all_unknown && all_aux_unknown) {
		*val = (all_aux) ? 0 : UI_ENTRY_UNKNOWN_VALUE;
		*auxval = (any_aux) ? UI_ENTRY_UNKNOWN_VALUE : 0;
	} else {
		(*combiner.finish_func)(&cst);
		if (all_unknown || (cst.accum == 0 && any_curse_unknown)) {
			*val = (all_aux) ? 0 : UI_ENTRY_UNKNOWN_VALUE;
		} else {
			*val = cst.accum;
		}
		if (all_aux_unknown
				|| (cst.accum_aux == 0 && any_curse_unknown)) {
			*auxval = (any_aux) ? UI_ENTRY_UNKNOWN_VALUE : 0;
		} else {
			*auxval = cst.accum_aux;
		}
	}
}


/**
 * For a player and a set of object properties bound to a ui_entry, loop
 * over those properties and report the combined value of them for the player.
 * \param entry Is the ui_entry to use.  It is the source of the object
 * properties that will be combined and the algorithm for combining the values.
 * \param p Is the player to assess.  May be NULL.
 * \param cache If *cache is not NULL, *cache is assumed to have been
 * initialized by a prior call to compute_ui_entry_values_for_player()
 * while the given player was in the same state.  If *cache is NULL, it will
 * be initialized by this call and is specific to the state of the given player.
 * The cache is to optimize away repeated calculations that would be performed
 * when looping over multiple ui_entry structures and calling this function for
 * each of them with the same player.  Once it is no longer needed, *cache
 * should be passed to release_cached_player_data() to release the resources
 * associated with it.
 * \param val At exit, *val will be the combined value for the player across
 * the non-auxiliary object properties bound to entry.  It will be
 *   a) UI_ENTRY_VALUE_NOT_PRESENT if p is NULL
 *   bound to entry but all such properties are unknown to p
 *   b) the combined value of the non-auxiliary properties bound to entry
 * The value of *val at entry is not used.
 * One use of auxiliary properties is to have the modifier for a stat be bound
 * to an entry and have the sustain for the stat be bound to the same entry as
 * an auxiliary property.
 * \param auxval At exit, *auxval will be the combined value for the object
 * across the auxiliary object properties bound to entry.  It will be
 *   a) UI_ENTRY_VALUE_NOT_PRESENT if p is NULL
 *   bound to entry but all such properties are unknown to p
 *   b) the combined value of the auxiliary properties bound to entry
 * The value of *auxval at entry is not used.
 */
void compute_ui_entry_values_for_player(const struct ui_entry *entry,
	struct player *p, struct cached_player_data **cache, int *val,
	int *auxval)
{
	struct ui_entry_combiner_state cst = { 0, 0, 0 };
	struct ui_entry_combiner_funcs combiner;
	bool first;
	int i;

	if (!p) {
		*val = UI_ENTRY_VALUE_NOT_PRESENT;
		*auxval = UI_ENTRY_VALUE_NOT_PRESENT;
		return;
	}
	if (*cache == NULL) {
		*cache = mem_alloc(sizeof(**cache));
		player_flags(p, (*cache)->untimed);
		of_wipe((*cache)->timed);
		player_flags_timed(p, (*cache)->timed);
		if (p->timed[TMD_TRAPSAFE]) {
			of_on((*cache)->timed, OF_TRAP_IMMUNE);
		}
	}
	first = true;
	if (ui_entry_combiner_get_funcs(entry->combiner_index, &combiner)) {
		assert(0);
	}
	for (i = 0; i < entry->n_p_ability; ++i) {
		int ind = entry->p_abilities[i].ability->index;

		if ((entry->flags & ENTRY_FLAG_TIMED_AUX) &&
			entry->p_abilities[i].isaux) {
			continue;
		}
		if (streq(entry->p_abilities[i].ability->type, "player")) {
			if (! player_has(p, ind)) {
				continue;
			}
			if (entry->p_abilities[i].have_value) {
				int v = entry->p_abilities[i].value;
				int a = UI_ENTRY_VALUE_NOT_PRESENT;

				if (entry->p_abilities[i].isaux) {
					int t = v;

					v = a;
					a = t;
				}
				if (first) {
					(*combiner.init_func)(v, a, &cst);
					first = false;
				} else {
					(*combiner.accum_func)(v, a, &cst);
				}
			} else {
				int v, a;
				struct object *launcher;

				/*
				 * Handle player abilities that did not bind a
				 * value to the user interface element as
				 * special cases.
				 */
				switch (ind) {
				case PF_FAST_SHOT:
					launcher = equipped_item_by_slot_name(
						p, "shooting");
					if (launcher && kf_has(launcher->kind->kind_flags,
						KF_SHOOTS_ARROWS)) {
						v = p->lev / 3;
						a = 0;
					} else {
						v = 0;
						a = 0;
					}
					if (entry->p_abilities[i].isaux) {
						int t = v;

						v = a;
						a = t;
					}
					if (first) {
						(*combiner.init_func)(v, a, &cst);
						first = false;
					} else {
						(*combiner.accum_func)(v, a, &cst);
					}
					break;

				case PF_BRAVERY_30:
					/*
					 * player_flags() accounts for
					 * PF_BRAVERY_30 so this is only
					 * necessary in cases where
					 * OF_PROT_FEAR isn't also bound to
					 * the element.
					 */
					v = (p->lev >= 30) ? 1 : 0;
					a = 0;
					if (entry->p_abilities[i].isaux) {
						int t = v;

						v = a;
						a = t;
					}
					if (first) {
						(*combiner.init_func)(v, a, &cst);
						first = false;
					} else {
						(*combiner.accum_func)(v, a, &cst);
					}
					break;
				}
			}
		} else if (streq(entry->p_abilities[i].ability->type,
			"object")) {
			int v = of_has((*cache)->untimed, ind) ? 1 : 0;
			int a;

			if (entry->flags & ENTRY_FLAG_TIMED_AUX) {
				a = of_has((*cache)->timed, ind) ? 1 : 0;
			} else {
				a = 0;
			}
			if (entry->p_abilities[i].isaux) {
				int t = v;

				v = a;
				a = t;
			}
			if (first) {
				(*combiner.init_func)(v, a, &cst);
				first = false;
			} else {
				(*combiner.accum_func)(v, a, &cst);
			}

			v = of_has(p->shape->flags, ind) ? 1 : 0;
			a = 0;
			if (v && of_has(p->obj_k->flags, ind)) {
				if (entry->p_abilities[i].isaux) {
					int t = v;

					v = a;
					a = t;
				}
				(*combiner.accum_func)(v, a, &cst);
			}
		} else if (streq(entry->p_abilities[i].ability->type,
			"element")) {
			int v = p->race->el_info[ind].res_level;
			int a;

			if (entry->flags & ENTRY_FLAG_TIMED_AUX) {
				a = get_timed_element_effect(p, ind);
			} else {
				a = 0;
			}
			if (entry->p_abilities[i].isaux) {
				int t = v;

				v = a;
				a = t;
			}
			if (first) {
				(*combiner.init_func)(v, a, &cst);
				first = false;
			} else {
				(*combiner.accum_func)(v, a, &cst);
			}
			v = p->shape->el_info[ind].res_level;
			a = 0;
			if (v != 0 && p->obj_k->el_info[ind].res_level) {
				if (entry->p_abilities[i].isaux) {
					int t = v;

					v = a;
					a = t;
				}
				(*combiner.accum_func)(v, a, &cst);
			}
		}
	}
	/*
	 * Since stats and modifiers aren't stored in the ability list, check
	 * if any object properties for those are bound to this element.
	 * Then lookup the player's intrinsic values for those.
	 */
	for (i = 0; i < entry->n_obj_prop; ++i) {
		int ind = entry->obj_props[i].index;
		int skill_ind, skill_cnv_num, skill_cnv_den;
		int v, a;

		if (entry->obj_props[i].isaux &&
			(entry->flags & ENTRY_FLAG_TIMED_AUX)) {
			continue;
		}
		switch (entry->obj_props[i].type) {
		case OBJ_PROPERTY_STAT:
		case OBJ_PROPERTY_MOD:
			v = p->shape->modifiers[ind];
			if (entry->flags & ENTRY_FLAG_TIMED_AUX) {
				a = get_timed_modifier_effect(p, ind);
			} else {
				a = 0;
			}
			if (entry->obj_props[i].isaux) {
				int t = v;

				v = a;
				a = t;
			}
			if (first) {
				(*combiner.init_func)(v, a, &cst);
				first = false;
			} else {
				(*combiner.accum_func)(v, a, &cst);
			}
			/*
			 * Racial information doesn't store modifiers but does
			 * store skills.  If applicable, extract the relevant
			 * value and convert.
			 */
			modifier_to_skill(ind, &skill_ind, &skill_cnv_num,
				&skill_cnv_den);
			if (skill_ind >= 0) {
				v = (p->race->r_skills[skill_ind] *
					skill_cnv_num) / skill_cnv_den;
				a = 0;
				if (entry->obj_props[i].isaux) {
					int t = v;

					v = a;
					a = t;
				}
				(*combiner.accum_func)(v, a, &cst);
			}
			/*
			 * Uggh, the player race handles infravision
			 * separately.
			 */
			if (ind == OBJ_MOD_INFRA) {
				v = p->race->infra;
				a = 0;
				if (entry->obj_props[i].isaux) {
					int t = v;

					v = a;
					a = t;
				}
				(*combiner.accum_func)(v, a, &cst);
			}
			break;
		}
	}
	if (first) {
		*val = UI_ENTRY_VALUE_NOT_PRESENT;
		*auxval = UI_ENTRY_VALUE_NOT_PRESENT;
	} else {
		(*combiner.finish_func)(&cst);
		*val = cst.accum;
		*auxval = cst.accum_aux;
	}
}


/**
 * Release a cache allocated by compute_ui_entry_values_for_object().
 */
void release_cached_object_data(struct cached_object_data *cache)
{
	mem_free(cache);
}


/**
 * Release a cache allocated by compute_ui_entry_values_for_player().
 */
void release_cached_player_data(struct cached_player_data *cache)
{
	mem_free(cache);
}


/**
 * Lookup the entry by name.  Return a nonzero value if present and set *ind to
 * its index.  Otherwise return zero and set *ind to where it should be
 * inserted.
 */
static int ui_entry_search(const char *name, int *ind)
{
	/* They're sorted by name so use a binary search. */
	int ilow = 0, ihigh = n_entry;

	while (1) {
		int imid, cmp;

		if (ilow == ihigh) {
			*ind = ilow;
			return 0;
		}
		imid = (ilow + ihigh) / 2;
		cmp = strcmp(entries[imid]->name, name);
		if (cmp == 0) {
			*ind = imid;
			return 1;
		}
		if (cmp < 0) {
			ilow = imid + 1;
		} else {
			ihigh = imid;
		}
	}
}


/**
 * Lookup the entry by name.  If present return its index + 1.  Otherwise,
 * return 0.
 */
static int ui_entry_lookup(const char *name)
{
	int ind;

	return ui_entry_search(name, &ind) ? ind + 1 : 0;
}


/**
 * Insert the entry.
 */
static void ui_entry_insert(struct ui_entry *entry)
{
	int ind;

	if (ui_entry_search(entry->name, &ind)) {
		quit("Attempted to insert ui_entry with same name");
	}

	if (n_entry == nalloc_entry) {
		struct ui_entry **extended;

		if (nalloc_entry > INT_MAX / 2) {
			quit("Too many ui_entry");
		}
		nalloc_entry = (nalloc_entry == 0) ? 8 : 2 * nalloc_entry;
		extended = mem_alloc(nalloc_entry * sizeof(*extended));
		if (ind > 0) {
			(void) memcpy(extended, entries,
				ind * sizeof(*entries));
		}
		extended[ind] = entry;
		if (ind < n_entry) {
			(void) memcpy(extended + ind + 1, entries + ind,
				(n_entry - ind) * sizeof(*entries));
		}
		mem_free(entries);
		entries = extended;
	} else {
		int i;

		for (i = n_entry; i > ind; --i) {
			entries[i] = entries[i - 1];
		}
		entries[ind] = entry;
	}
	++n_entry;
}


/**
 * Search for name in the categories associated with ui_entry.  Return a
 * nonzero value if present and set *ind to its index.  Otherwise, return zero
 * and set *ind to where it should be inserted.
 */
static int ui_entry_search_categories(const struct ui_entry *entry,
	const char *name, int *ind)
{
	/* They're sorted by name so use a binary search. */
	int ilow = 0, ihigh = entry->n_category;

	while (1) {
		int imid, cmp;

		if (ilow == ihigh) {
			*ind = ilow;
			return 0;
		}
		imid = (ilow + ihigh) / 2;
		cmp = strcmp(entry->categories[imid].name, name);
		if (cmp == 0) {
			*ind = imid;
			return 1;
		}
		if (cmp < 0) {
			ilow = imid + 1;
		} else {
			ihigh = imid;
		}
	}
}


static void modifier_to_skill(int modind, int *skillind, int *skill2mod_num,
	int *skill2mod_den
)
{
	switch (modind) {
	case OBJ_MOD_TUNNEL:
		*skillind = SKILL_DIGGING;
		*skill2mod_num = 1;
		*skill2mod_den = 20;
		break;

	/*
	 * Stealth and searching also have skills, but the calculations
	 * formerly in ui-player.c didn't include the racial contribution from
	 * those in what was shown in the second player screen.  If that's
	 * desirable, these are the conversion factors used in player-calcs.c
	 */
#if 0
	case OBJ_MOD_STEALTH:
		*skillind = SKILL_STEALTH;
		*skill2mod_num = 1;
		*skill2mod_den = 1;
		break;

	case OBJ_MOD_SEARCH:
		*skillind = SKILL_SEARCH;
		*skill2mod_num = 1;
		*skill2mod_den = 5;
		break;
#endif

	default:
		*skillind = -1;
		*skill2mod_num = 1;
		*skill2mod_den = 1;
		break;
	}
}


static int get_timed_element_effect(const struct player *p, int ind)
{
	int result;

	switch (ind) {
	case ELEM_ACID:
		result = p->timed[TMD_OPP_ACID] ? 1 : 0;
		break;

	case ELEM_ELEC:
		result = p->timed[TMD_OPP_ELEC] ? 1 : 0;
		break;

	case ELEM_FIRE:
		result = p->timed[TMD_OPP_FIRE] ? 1 : 0;
		break;

	case ELEM_COLD:
		result = p->timed[TMD_OPP_COLD] ? 1 : 0;
		break;

	case ELEM_POIS:
		result = p->timed[TMD_OPP_POIS] ? 1 : 0;
		break;

	default:
		result = 0;
		break;
	}
	return result;
}


static int get_timed_modifier_effect(const struct player *p, int ind)
{
	int result;

	/* Mimics calculations made in player-calcs.c. */
	switch (ind) {
	case OBJ_MOD_BLOWS:
		result = (p->timed[TMD_BLOODLUST]) ?
			p->timed[TMD_BLOODLUST] / 20 : 0;
		break;

	case OBJ_MOD_INFRA:
		result = (p->timed[TMD_SINFRA]) ? 5 : 0;
		break;

	case OBJ_MOD_SPEED:
		result = (p->timed[TMD_FAST] || p->timed[TMD_SPRINT]) ? 10 : 0;
		if (p->timed[TMD_STONESKIN]) {
			result -= 5;
		}
		if (p->timed[TMD_SLOW]) {
			result -= 10;
		}
		if (p->timed[TMD_TERROR]) {
			result += 10;
		}
		break;

	case OBJ_MOD_STEALTH:
		result = (p->timed[TMD_STEALTH]) ? 10 : 0;
		break;

	default:
		result = 0;
		break;
	}
	return result;
}


/**
 * Search for name in the categories associated with ui_entry being parsed.
 * Return a nonzero value if present and set *ind to its index.  Otherwise,
 * return zero and set *ind to where it should be inserted.
 */
static int search_embryo_categories(const struct embryonic_ui_entry *embryo,
	const char *name, int *ind)
{
	int ilow, ihigh;

	if (embryo->exists) {
		return ui_entry_search_categories(embryo->entry, name, ind);
	}

	/* They're sorted by name so use a binary search. */
	ilow = 0;
	ihigh = embryo->entry->n_category;
	while (1) {
		int imid, cmp;

		if (ilow == ihigh) {
			*ind = ilow;
			return 0;
		}
		imid = (ilow + ihigh) / 2;
		cmp = strcmp(embryo->categories[imid].name, name);
		if (cmp == 0) {
			*ind = imid;
			return 1;
		}
		if (cmp < 0) {
			ilow = imid + 1;
		} else {
			ihigh = imid;
		}
	}
}


/**
 * Search the list of all categories seen so far for a name.  Return a
 * nonzero value if found and set *ind to its index.  Otherwise, return zero
 * and set *ind to where it should be inserted.
 */
static int search_categories(const char *name, int *ind)
{
	int ilow = 0, ihigh = n_category;

	while (1) {
		int imid, cmp;

		if (ilow == ihigh) {
			*ind = ilow;
			return 0;
		}
		imid = (ilow + ihigh) / 2;
		cmp = strcmp(categories[imid], name);
		if (cmp == 0) {
			*ind = imid;
			return 1;
		}
		if (cmp < 0) {
			ilow = imid + 1;
		} else {
			ihigh = imid;
		}
	}
}


/**
 * Insert a category with (name, priority) into the ui_entry being parsed.
 * Use ind as the insertion point.
 */
static void insert_embryo_category(struct embryonic_ui_entry *embryo,
	const char *name, int psource_index, int priority, bool priority_set,
	int ind)
{
	int cind;

	if (! search_categories(name, &cind)) {
		if (n_category == nalloc_category) {
			char **extended;

			if (n_category > INT_MAX / 2) {
				quit("Too many categories");
			}
			nalloc_category = (nalloc_category == 0) ?
				8 : nalloc_category * 2;
			extended = mem_alloc(nalloc_category *
				sizeof(*extended));
			if (cind > 0) {
				memcpy(extended, categories,
					cind * sizeof(*extended));
			}
			extended[cind] = string_make(name);
			if (cind < n_category) {
				memcpy(extended + cind + 1, categories + cind,
					(n_category - cind) *
					sizeof(*extended));
			}
			mem_free(categories);
			categories = extended;
		} else {
			int i;

			for (i = n_category; i > cind; --i) {
				categories[i] = categories[i - 1];
			}
			categories[cind] = string_make(name);
		}
		++n_category;
	}
	if (embryo->entry->n_category == embryo->entry->nalloc_category) {
		if (embryo->entry->nalloc_category > INT_MAX / 2) {
			quit("Too many categories for an ui_entry");
		}
		embryo->entry->nalloc_category =
			(embryo->entry->nalloc_category == 0) ?
			4 : 2 * embryo->entry->nalloc_category;
		if (embryo->exists) {
			struct category_reference *extended =
				mem_alloc(embryo->entry->nalloc_category *
				sizeof(*extended));

			if (ind > 0) {
				(void) memcpy(extended,
					embryo->entry->categories,
					ind * sizeof(*extended));
			}
			extended[ind].name = categories[cind];
			extended[ind].priority = priority;
			extended[ind].priority_set = priority_set;
			if (ind < embryo->entry->n_category) {
				(void) memcpy(extended + ind + 1,
					embryo->entry->categories + ind,
					(embryo->entry->n_category - ind) *
					sizeof(*extended));
			}
			mem_free(embryo->entry->categories);
			embryo->entry->categories = extended;
		} else {
			struct embryonic_category_reference *extended =
				mem_alloc(embryo->entry->nalloc_category *
				sizeof(*extended));

			if (ind > 0) {
				(void) memcpy(extended,
					embryo->categories,
					ind * sizeof(*extended));
			}
			extended[ind].name = categories[cind];
			extended[ind].psource_index = psource_index;
			extended[ind].priority = priority;
			extended[ind].priority_set = priority_set;
			if (ind < embryo->entry->n_category) {
				(void) memcpy(extended + ind + 1,
					embryo->categories + ind,
					(embryo->entry->n_category - ind) *
					sizeof(*extended));
			}
			mem_free(embryo->categories);
			embryo->categories = extended;
		}
	} else {
		int i;

		if (embryo->exists) {
			for (i = embryo->entry->n_category; i > ind; --i) {
				embryo->entry->categories[i] =
					embryo->entry->categories[i - 1];
			}
			embryo->entry->categories[ind].name = categories[cind];
			embryo->entry->categories[ind].priority = priority;
			embryo->entry->categories[ind].priority_set =
				priority_set;
		} else {
			for (i = embryo->entry->n_category; i > ind; --i) {
				embryo->categories[i] =
					embryo->categories[i - 1];
			}
			embryo->categories[ind].name = categories[cind];
			embryo->categories[ind].psource_index = psource_index;
			embryo->categories[ind].priority = priority;
			embryo->categories[ind].priority_set = priority_set;
		}
	}
	++embryo->entry->n_category;
}


/* These are for handling unparameterized entry names. */
static int get_dummy_param_count(void)
{
	return 1;
}


static const char *get_dummy_param_name(int i)
{
	assert(i == 0);
	return "";
}


/* These are for handling of entries parameterized by the element name. */
static const char *element_names[] = {
	#define ELEM(x) #x,
	#include "list-elements.h"
	#undef ELEM
};


static int get_element_count(void)
{
	return N_ELEMENTS(element_names);
}


static const char *get_element_name(int i)
{
	assert(i >= 0 && i < get_element_count());
	return element_names[i];
}


/* These are for handling of entries parameterized by the stat name. */
static const char *stat_names[] = {
	#define STAT(x) #x,
	#include "list-stats.h"
	#undef STAT
};


static int get_stat_count(void)
{
	return N_ELEMENTS(stat_names);
}


static const char *get_stat_name(int i)
{
	assert(i >= 0 && i < get_stat_count());
	return stat_names[i];
}


/*
 * For handling of automatically setting the priority based on the parameter
 * indexing.
 */
static int get_dummy_priority(int i)
{
	return 0;
}


static int get_priority_from_index(int i)
{
	return i;
}


static int get_priority_from_negative_index(int i)
{
	return -i;
}


/* Convert list of categories in the embryo to its final form. */
static void parameterize_category_list(
	const struct embryonic_category_reference *ctgs, int n, int ind,
	struct ui_entry *entry)
{
	int i;

	entry->categories = mem_alloc(n * sizeof(*entry->categories));
	entry->n_category = n;
	entry->nalloc_category = n;

	for (i = 0; i < n; ++i) {
		entry->categories[i].name = ctgs[i].name;
		if (ctgs[i].priority_set) {
			entry->categories[i].priority =
				(ctgs[i].psource_index > 0) ?
				(*priority_schemes[ctgs[i].psource_index].priority)(ind) :
				ctgs[i].priority;
			entry->categories[i].priority_set = true;
		} else {
			entry->categories[i].priority = 0;
			entry->categories[i].priority_set = false;
		}
	}
}


static void initialize_shortened(struct ui_entry *entry)
{
	int i;

	assert(MAX_SHORTENED > 0);
	entry->shortened_labels[0] = entry->shortened_buffer;
	for (i = 1; i < MAX_SHORTENED; ++i) {
		entry->shortened_labels[i] =
			entry->shortened_labels[i - 1] + i + 1;
	}
	for (i = 0; i < MAX_SHORTENED; ++i) {
		entry->nshortened[i] = 0;
	}
}


static void copy_shortened_labels(struct ui_entry *dest,
	const struct ui_entry *src)
{
	int i;

	for (i = 0; i < MAX_SHORTENED; ++i) {
		dest->shortened_labels[i] = (i == 0) ? dest->shortened_buffer :
			dest->shortened_labels[i - 1] + i + 1;
		dest->nshortened[i] = src->nshortened[i];
		if (src->nshortened[i] > 0) {
			(void) memcpy(dest->shortened_labels[i],
				src->shortened_labels[i],
				(src->nshortened[i] + 1) *
				sizeof(*dest->shortened_labels[i]));
		}
	}
}


static void fill_out_shortened(struct ui_entry *entry)
{
	int i;

	for (i = 0; i < MAX_SHORTENED; ++i) {
		int j;
		int n;
		const wchar_t *src;

		if (entry->nshortened[i] != 0) {
			continue;
		}
		/*
		 * Is there a longer abbreviated label already set?  Use it
		 * as the basis for this one.  Otherwise, use the full label.
		 */
		j = i + 1;
		while (1) {
			if (j >= MAX_SHORTENED) {
				n = entry->nlabel;
				src = entry->label;
				break;
			}
			if (entry->nshortened[j] != 0) {
				n = entry->nshortened[j];
				src = entry->shortened_labels[j];
				break;
			}
			++j;
		}
		entry->nshortened[i] = (n < i + 1) ? n : i + 1;
		(void) memcpy(entry->shortened_labels[i], src,
			(entry->nshortened[i] + 1) *
			sizeof(*entry->shortened_labels[i]));
	}
}


static int hatch_embryo(struct embryonic_ui_entry *embryo)
{
	/* If was editing an existing entry, do not have to recommit it. */
	if (! embryo->exists) {
		int n, i;

		/* Check that required fields are valid. */
		if (embryo->entry->combiner_index == 0) {
			string_free(embryo->entry->name);
			mem_free(embryo->entry->label);
			mem_free(embryo->entry);
			mem_free(embryo->categories);
			return 1;
		}

		/* Parameterize the name generating multiple entries. */
		n = (*name_parameters[embryo->param_index].count_func)();
		for (i = 0; i < n - 1; ++i) {
			struct ui_entry *entry = mem_alloc(sizeof(*entry));
			const char *name = (*name_parameters[embryo->param_index].ith_name_func)(i);
			entry->name = string_make(
				format("%s<%s>", embryo->entry->name, name));
			if (embryo->psource_index != 0) {
				entry->default_priority =
					(*priority_schemes[embryo->psource_index].priority)(i);
			} else {
				entry->default_priority =
					embryo->entry->default_priority;
			}
			parameterize_category_list(
				embryo->categories, embryo->entry->n_category,
				i, entry);
			entry->obj_props = NULL;
			entry->p_abilities = NULL;
			if (embryo->entry->nlabel > 0) {
				size_t sz = (embryo->entry->nlabel + 1) *
					sizeof(*entry->label);

				entry->label = mem_alloc(sz);
				(void) memcpy(entry->label,
					embryo->entry->label, sz);
				entry->nlabel = embryo->entry->nlabel;
			} else {
				size_t nw;

				entry->label = mem_alloc(
					(MAX_ENTRY_LABEL + 1) *
					sizeof(*entry->label));
				nw = text_mbstowcs(entry->label, name,
					MAX_ENTRY_LABEL);
				/* Ensure null termination. */
				if (nw != (size_t)-1 &&
					text_mbstowcs(entry->label + nw, "", 1) != (size_t)-1) {
					entry->nlabel = nw;
				}
			}
			copy_shortened_labels(entry, embryo->entry);
			entry->renderer_index = embryo->entry->renderer_index;
			entry->combiner_index = embryo->entry->combiner_index;
			entry->param_index = i;
			entry->flags = embryo->entry->flags;
			entry->n_obj_prop = 0;
			entry->nalloc_obj_prop = 0;
			entry->n_p_ability = 0;
			entry->nalloc_p_ability = 0;
			ui_entry_insert(entry);
		}
		/*
		 * For the last one, reuse the structure stored in the embryo
		 * to save a bit of effort.
		 */
		if (embryo->param_index == 0) {
			/* Not parameterized */
			embryo->entry->param_index = -1;
		} else {
			const char *name = (*name_parameters[embryo->param_index].ith_name_func)(n - 1);
			char *entnm = string_make(
				format("%s<%s>", embryo->entry->name, name));

			string_free(embryo->entry->name);
			embryo->entry->name = entnm;
			if (embryo->entry->nlabel == 0) {
				size_t nw;

				embryo->entry->label = mem_alloc(
					(MAX_ENTRY_LABEL + 1) *
					sizeof(*embryo->entry->label));
				nw = text_mbstowcs(embryo->entry->label, name,
					MAX_ENTRY_LABEL);
				/* Ensure null termination. */
				if (nw != (size_t)-1 &&
					text_mbstowcs(embryo->entry->label + nw, "", 1) != (size_t)-1) {
					embryo->entry->nlabel = nw;
				}
			}
			embryo->entry->param_index = n - 1;
			if (embryo->psource_index != 0) {
				embryo->entry->default_priority =
					(*priority_schemes[embryo->psource_index].priority)(n - 1);
			}
		}
		parameterize_category_list(embryo->categories,
			embryo->entry->n_category, n - 1, embryo->entry);
		ui_entry_insert(embryo->entry);
	}
	mem_free(embryo->categories);
	return 0;
}


static int hatch_last_embryo(struct parser *p)
{
	struct embryonic_ui_entry *embryo = parser_priv(p);
	int result = 0;

	if (embryo) {
		if (hatch_embryo(embryo)) {
			result = 1;
		}
		mem_free(embryo);
		parser_setpriv(p, NULL);
	}
	return result;
}


static enum parser_error parse_entry_name(struct parser *p)
{
	const char *name = parser_getstr(p, "name");
	struct embryonic_ui_entry *embryo = parser_priv(p);
	int ind;

	if (embryo) {
		if (streq(name, embryo->entry->name)) {
			/*
			 * Strange case since the last record is the same as
			 * the this one.  Simply proceeed to modify the
			 * previous record without a warning.
			 */
			return PARSE_ERROR_NONE;
		}
		if (hatch_embryo(embryo)) {
			mem_free(embryo);
			parser_setpriv(p, NULL);
			return PARSE_ERROR_INVALID_VALUE;
		}
	} else {
		embryo = mem_alloc(sizeof(*embryo));
	}

	parser_setpriv(p, embryo);
	ind = ui_entry_lookup(name);
	if (ind > 0) {
		/*
		 * Modify an already existing record.  Recall it, convert it
		 * back to embryonic form, and mark it as already existing.
		 */
		embryo->entry = entries[ind - 1];
		embryo->exists = true;
	} else {
		/* Make a completely new entry. */
		embryo->entry = mem_alloc(sizeof(*embryo->entry));
		embryo->entry->name = string_make(name);
		embryo->entry->categories = NULL;
		embryo->entry->obj_props = NULL;
		embryo->entry->p_abilities = NULL;
		embryo->entry->label = NULL;
		initialize_shortened(embryo->entry);
		embryo->entry->nlabel = 0;
		embryo->entry->renderer_index = 0;
		embryo->entry->combiner_index = 0;
		embryo->entry->default_priority = 0;
		embryo->entry->flags = 0;
		embryo->entry->n_category = 0;
		embryo->entry->nalloc_category = 0;
		embryo->entry->n_obj_prop = 0;
		embryo->entry->nalloc_obj_prop = 0;
		embryo->entry->n_p_ability = 0;
		embryo->entry->nalloc_p_ability = 0;
		embryo->exists = false;
	}
	embryo->categories = NULL;
	embryo->param_index = 0;
	embryo->psource_index = 0;
	embryo->last_category_index = -1;

	return PARSE_ERROR_NONE;
}


static enum parser_error parse_entry_template(struct parser *p)
{
	struct embryonic_ui_entry *embryo = parser_priv(p);
	const char *name;
	const struct ui_entry *tentry;
	int template_ind, i;

	if (!embryo) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	name = parser_getstr(p, "template");
	template_ind = ui_entry_lookup(name);
	if (template_ind == 0) {
		return PARSE_ERROR_INVALID_VALUE;
	}
	tentry = entries[template_ind - 1];
	/* Replace simple fields with what's in the template. */
	embryo->entry->renderer_index = tentry->renderer_index;
	embryo->entry->combiner_index = tentry->combiner_index;
	embryo->entry->default_priority = tentry->default_priority;
	embryo->entry->flags = tentry->flags & ~ENTRY_FLAG_TEMPLATE_ONLY;
	/* Add in categories not already present. */
	for (i = 0; i < tentry->n_category; ++i) {
		int ind;

		if (! search_embryo_categories(embryo,
			tentry->categories[i].name, &ind)) {
			insert_embryo_category(embryo,
				tentry->categories[i].name, 0,
				tentry->categories[i].priority,
				tentry->categories[i].priority_set, ind);
		}
	}
	return PARSE_ERROR_NONE;
}


static enum parser_error parse_entry_parameter(struct parser *p)
{
	struct embryonic_ui_entry *embryo = parser_priv(p);
	const char *name;
	int n = N_ELEMENTS(name_parameters);
	int i;

	if (!embryo) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	/*
	 * Don't allow parameterization when editing an already existing entry.
	 */
	if (embryo->exists) {
		return PARSE_ERROR_INVALID_OPTION;
	}
	name = parser_getstr(p, "parameter");
	i = 0;
	while (1) {
		if (i >= n) {
			return PARSE_ERROR_INVALID_VALUE;
		}
		if (streq(name, name_parameters[i].name)) {
			embryo->param_index = i;
			break;
		}
		++i;
	}
	return PARSE_ERROR_NONE;
}


static enum parser_error parse_entry_renderer(struct parser *p)
{
	struct embryonic_ui_entry *embryo = parser_priv(p);
	const char *name;

	if (!embryo) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	name = parser_getstr(p, "renderer");
	embryo->entry->renderer_index = ui_entry_renderer_lookup(name);
	return (embryo->entry->renderer_index) ?
		PARSE_ERROR_NONE : PARSE_ERROR_INVALID_VALUE;
}


static enum parser_error parse_entry_combine(struct parser *p)
{
	struct embryonic_ui_entry *embryo = parser_priv(p);
	const char *name;

	if (!embryo) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	name = parser_getstr(p, "combine");
	embryo->entry->combiner_index = ui_entry_combiner_lookup(name);
	return (embryo->entry->combiner_index) ?
		PARSE_ERROR_NONE : PARSE_ERROR_INVALID_VALUE;
}


static enum parser_error parse_entry_label(struct parser *p)
{
	struct embryonic_ui_entry *embryo = parser_priv(p);
	const char *name;
	size_t nw;

	if (!embryo) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	name = parser_getstr(p, "label");
	mem_free(embryo->entry->label);
	embryo->entry->label = mem_alloc((MAX_ENTRY_LABEL + 1) *
		sizeof(*embryo->entry->label));
	nw = text_mbstowcs(embryo->entry->label, name, MAX_ENTRY_LABEL);
	if (nw != (size_t)-1) {
		/* Ensure null termination. */
		size_t nw2 = text_mbstowcs(embryo->entry->label + nw, "", 1);

		if (nw2 != (size_t)-1) {
			embryo->entry->nlabel = nw;
		} else {
			return PARSE_ERROR_INVALID_VALUE;
		}
	} else {
		return PARSE_ERROR_INVALID_VALUE;
	}
	return PARSE_ERROR_NONE;
}


static enum parser_error parse_entry_shortened_label(struct parser *p)
{
	struct embryonic_ui_entry *embryo = parser_priv(p);
	int n = 1;
	const char *name;
	size_t nw;

	if (!embryo) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	while (1) {
		if (n > MAX_SHORTENED) {
			return PARSE_ERROR_INTERNAL;
		}
		if (parser_hasval(p, format("label%d", n))) {
			break;
		}
		++n;
	}
	name = parser_getstr(p, format("label%d", n));
	nw = text_mbstowcs(embryo->entry->shortened_labels[n - 1], name, n);
	if (nw != (size_t) -1) {
		/* Ensure null termination. */
		size_t nw2 = text_mbstowcs(
			embryo->entry->shortened_labels[n - 1] +
			((int)nw < n ? (int)nw : n), "", 1);

		if (nw2 != (size_t)-1) {
			embryo->entry->nshortened[n - 1] = nw;
		} else {
			return PARSE_ERROR_INVALID_VALUE;
		}
	} else {
		return PARSE_ERROR_INVALID_VALUE;
	}
	return PARSE_ERROR_NONE;
}


static enum parser_error parse_entry_category(struct parser *p)
{
	struct embryonic_ui_entry *embryo = parser_priv(p);
	const char *name;
	int ind;

	if (!embryo) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	name = parser_getstr(p, "category");
	if (! search_embryo_categories(embryo, name, &ind)) {
		insert_embryo_category(embryo, name, embryo->psource_index,
			embryo->entry->default_priority, false, ind);
	}
	embryo->last_category_index = ind;
	return PARSE_ERROR_NONE;
}


static enum parser_error parse_entry_priority(struct parser *p)
{
	struct embryonic_ui_entry *embryo = parser_priv(p);
	const char *name;
	int psource_index = 0;
	int priority = 0;
	int n = N_ELEMENTS(priority_schemes);
	int i;

	if (!embryo) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	name = parser_getstr(p, "priority");
	i = 1;
	while (1) {
		if (i >= n) {
			char *end;
			long v;

			v = strtol(name, &end, 10);
			if (! *name || *end) {
				return PARSE_ERROR_NOT_NUMBER;
			}
			priority = (v <= INT_MAX) ?
				((v >= INT_MIN) ? v : INT_MIN) : INT_MAX;
			break;
		}
		if (streq(name, priority_schemes[i].name)) {
			if (embryo->exists) {
				priority = (*priority_schemes[i].priority)(
					embryo->entry->param_index);
			} else {
				psource_index = i;
			}
			break;
		}
		++i;
	}
	if (embryo->last_category_index == -1) {
		embryo->psource_index = psource_index;
		embryo->entry->default_priority = priority;
	} else {
		if (embryo->exists) {
			embryo->entry->categories[embryo->last_category_index].priority = priority;
			embryo->entry->categories[embryo->last_category_index].priority_set = true;
		} else {
			embryo->categories[embryo->last_category_index].psource_index = psource_index;
			embryo->categories[embryo->last_category_index].priority = priority;
			embryo->categories[embryo->last_category_index].priority_set = true;
		}
	}
	return PARSE_ERROR_NONE;
}


static enum parser_error parse_entry_flags(struct parser *p)
{
	struct embryonic_ui_entry *embryo = parser_priv(p);
	char *flags;
	char *s;
	int n;

	if (!embryo) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	if (!parser_hasval(p, "flags")) {
		return PARSE_ERROR_NONE;
	}
	flags = string_make(parser_getstr(p, "flags"));
	n = N_ELEMENTS(entry_flags);
	s = strtok(flags, " |");
	while (s) {
		int i = 0;

		while (1) {
			if (i >= n) {
				string_free(flags);
				return PARSE_ERROR_INVALID_FLAG;
			}
			if (streq(s, entry_flags[i].name)) {
				embryo->entry->flags |= entry_flags[i].value;
				break;
			}
			++i;
		}
		s = strtok(NULL, " |");
	}
	string_free(flags);
	return PARSE_ERROR_NONE;
}


static enum parser_error parse_entry_desc(struct parser *p)
{
	struct embryonic_ui_entry *embryo = parser_priv(p);

	if (!embryo) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	/* Don't bother to store the description. */
	return PARSE_ERROR_NONE;
}


static struct parser *init_parse_ui_entry(void)
{
	struct parser *p = parser_new();
	int i;

	parser_setpriv(p, NULL);
	parser_reg(p, "name str name", parse_entry_name);
	parser_reg(p, "template str template", parse_entry_template);
	parser_reg(p, "parameter str parameter", parse_entry_parameter);
	parser_reg(p, "renderer str renderer", parse_entry_renderer);
	parser_reg(p, "combine str combine", parse_entry_combine);
	parser_reg(p, "label str label", parse_entry_label);
	for (i = 1; i <= MAX_SHORTENED; ++i) {
		parser_reg(p, format("label%d str label%d", i, i),
			parse_entry_shortened_label);
	}
	parser_reg(p, "category str category", parse_entry_category);
	parser_reg(p, "priority str priority", parse_entry_priority);
	parser_reg(p, "flags ?str flags", parse_entry_flags);
	parser_reg(p, "desc str desc", parse_entry_desc);
	return p;
}


static errr run_parse_ui_entry(struct parser *p)
{
	errr result = parse_file(p, "ui_entry_base");
	int i;

	if (result != 0) {
		return result;
	}
	if (hatch_last_embryo(p)) {
		return 1;
	}
	/*
	 * Mark those as templates only so they'll never be directly displayed.
	 */
	for (i = 0; i < n_entry; ++i) {
		entries[i]->flags |= ENTRY_FLAG_TEMPLATE_ONLY;
	}
	return parse_file(p, "ui_entry");
}


static errr finish_parse_ui_entry(struct parser *p)
{
	errr result = 0;
	int i;

	if (hatch_last_embryo(p)) {
		result = -1;
	}
	for (i = 0; i < n_entry; ++i) {
		int j;

		/* Set labels not already configured. */
		if (entries[i]->nlabel == 0) {
			size_t n;

			entries[i]->label = mem_alloc((MAX_ENTRY_LABEL + 1) *
				sizeof(*entries[i]->label));
			n = text_mbstowcs(entries[i]->label, entries[i]->name,
				MAX_ENTRY_LABEL);
			if (n != (size_t)-1) {
			 	/* Ensure null termination. */
				size_t n2 = text_mbstowcs(
					entries[i]->label + n, "", 1);

				if (n2 != (size_t)-1) {
					entries[i]->nlabel = n;
				} else {
					result = -1;
				}
			} else {
				result = -1;
			}
		}
		fill_out_shortened(entries[i]);

		/* Set priorities not already configured. */
		for (j = 0; j < entries[i]->n_category; ++j) {
			if (! entries[i]->categories[j].priority_set) {
				entries[i]->categories[j].priority =
					entries[i]->default_priority;
				entries[i]->categories[j].priority_set = true;
			}
		}
	}

	parser_destroy(p);
	return result;
}


static void cleanup_parse_ui_entry(void)
{
	int i;

	for (i = 0; i < n_entry; ++i) {
		string_free(entries[i]->name);
		mem_free(entries[i]->categories);
		mem_free(entries[i]->obj_props);
		mem_free(entries[i]->p_abilities);
		mem_free(entries[i]->label);
		mem_free(entries[i]);
	}
	mem_free(entries);
	n_entry = 0;
	nalloc_entry = 0;
	entries = NULL;

	for (i = 0; i < n_category; ++i) {
		string_free(categories[i]);
	}
	mem_free(categories);
	n_category = 0;
	nalloc_category = 0;
	categories = NULL;
}


struct file_parser ui_entry_parser = {
	"ui_entry",
	init_parse_ui_entry,
	run_parse_ui_entry,
	finish_parse_ui_entry,
	cleanup_parse_ui_entry
};
