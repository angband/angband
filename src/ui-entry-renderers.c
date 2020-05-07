/**
 * \file ui-entry-renderers.c
 * \brief Define backend handling for some parts of the second character screen
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

#include "ui-entry-init.h"
#include "ui-entry-renderers.h"
#include "ui-term.h"
#include "z-color.h"
#include "z-util.h"
#include "z-virt.h"

/*
 * This is the maximum number of symbols/colors used by any renderer.  Used to
 * limit what's extracted from the configuration file.
 */
#define MAX_PALETTE (64)

/*
 * This is the maximum number of characters to use for any units label.  Used
 * to limit what's extracted from the configuration file.
 */
#define MAX_UNITS_LABEL (24)

struct renderer_info;
typedef void (*renderer_func)(
	const wchar_t *label,
	int nlabel,
	const int *vals,
	const int *auxvals,
	int n,
	const struct ui_entry_details *details,
	const struct renderer_info *info);
typedef int (*valuewidth_func)(const struct renderer_info *info);

static void format_int(int i, bool add_one, wchar_t zero, wchar_t overflow,
	bool nonneg, bool use_sign, int nbuf, wchar_t *buf);
static void show_combined_generic(const struct renderer_info *info,
	const struct ui_entry_details *details, int vcombined, int acombined);

/* Implemented backends. */
static void renderer_COMPACT_RESIST_RENDERER_WITH_COMBINED_AUX(
	const wchar_t *label,
	int nlabel,
	const int *vals,
	const int *auxvals,
	int n,
	const struct ui_entry_details *details,
	const struct renderer_info *info);
static int valuewidth_COMPACT_RESIST_RENDERER_WITH_COMBINED_AUX(
	const struct renderer_info *info);
static void renderer_COMPACT_FLAG_RENDERER_WITH_COMBINED_AUX(
	const wchar_t *label,
	int nlabel,
	const int *vals,
	const int *auxvals,
	int n,
	const struct ui_entry_details *details,
	const struct renderer_info *info
);
static int valuewidth_COMPACT_FLAG_RENDERER_WITH_COMBINED_AUX(
	const struct renderer_info *info);
static void renderer_NUMERIC_AS_SIGN_RENDERER_WITH_COMBINED_AUX(
	const wchar_t *label,
	int nlabel,
	const int *vals,
	const int *auxvals,
	int n,
	const struct ui_entry_details *details,
	const struct renderer_info *info);
static int valuewidth_NUMERIC_AS_SIGN_RENDERER_WITH_COMBINED_AUX(
	const struct renderer_info *info);
static void renderer_NUMERIC_RENDERER_WITH_COMBINED_AUX(
	const wchar_t *label,
	int nlabel,
	const int *vals,
	const int *auxvals,
	int n,
	const struct ui_entry_details *details,
	const struct renderer_info *info);
static int valuewidth_NUMERIC_RENDERER_WITH_COMBINED_AUX(
	const struct renderer_info *info);
static void renderer_NUMERIC_RENDERER_WITH_BOOL_AUX(
	const wchar_t *label,
	int nlabel,
	const int *vals,
	const int *auxvals,
	int n,
	const struct ui_entry_details *details,
	const struct renderer_info *info);
static int valuewidth_NUMERIC_RENDERER_WITH_BOOL_AUX(
	const struct renderer_info *info);

struct backend_info {
	renderer_func func;
	valuewidth_func vw_func;
	const char *default_combiner_name;
	const char *default_colors;
	const char *default_labelcolors;
	const char *default_symbols;
	int default_ndigit;
	int default_sign;
};

enum {
	UI_ENTRY_NO_SIGN,
	UI_ENTRY_ALWAYS_SIGN,
	UI_ENTRY_NEGATIVE_SIGN,
	UI_ENTRY_SIGN_DEFAULT = INT_MIN
};

static const struct backend_info backends[] =
{
	#define F(x) renderer_##x
	#define FWIDTH(x) valuewidth_##x
	#define UI_ENTRY_RENDERER(x, co, c, lc, s, n, sg) { F(x), FWIDTH(x), co, c, lc, s, n, UI_ENTRY_##sg},
	#include "list-ui-entry-renderers.h"
	#undef UI_ENTRY_RENDERER
	#undef F
	#undef FWIDTH
};

static const char *backend_names[] = {
	#define UI_ENTRY_RENDERER(x, co, c, lc, s, n, sg) #x,
	#include "list-ui-entry-renderers.h"
	#undef UI_ENTRY_RENDERER
	NULL
};

static char *convert_attrs_to_chars(const int *attr, int n);
static void convert_chars_to_attrs(const char *colors, int n, int *attr);

struct renderer_info {
	char *name;
	char *comb_rend_nm;
	int *colors;
	int *label_colors;
	wchar_t *symbols;
	wchar_t *units_label;
	const struct backend_info *backend;
	int combined_renderer_index;
	int combiner_index;
	int ncolors;
	int nlabcolors;
	int nsym;
	int ndigit;
	int sign;
	int units_nlabel;
};

static int renderer_count = 0;
static int renderer_allocated = 0;
static struct renderer_info *renderers = 0;


/**
 * Return the first valid index accepted by ui_entry_renderer_apply()
 * and the other functions expecting a renderer index.  Any index whose
 * value is greater than or equal to ui_entry_get_min_index() and less than
 * ui_entry_get_index_limit() is valid.
 */
int ui_entry_renderer_get_min_index(void)
{
	return 1;
}


/**
 * Return the upper bound for indices accepted by ui_entry_renderer_apply()
 * and the other functions expecting a renderer index.  Any index whose
 * value is greater than or equal to ui_entry_get_min_index() and less than
 * ui_entry_get_index_limit() is valid.
 */
int ui_entry_renderer_get_index_limit(void)
{
	return renderer_count + 1;
}


/**
 * Returns the name for the renderer as configured in ui_entry_renderers.txt.
 * The returned value will be NULL if ind is out of bounds.
 */
const char *ui_entry_renderer_get_name(int ind)
{
	return (ind >= 1 && ind <= renderer_count) ?
		renderers[ind - 1].name : NULL;
}

/**
 * Look up a renderer by name.  If the name is not one of those configured in
 * ui_entry_renderers.txt, returns zero.  Otherwise returns the index for
 * the renderer.
 */
int ui_entry_renderer_lookup(const char *name)
{
	int i = 0;

	while (i < renderer_count) {
		if (streq(name, renderers[i].name)) {
			return i + 1;
		}
		++i;
	}
	return 0;
}


/**
 * Query a renderer for the number of characters used to draw a value.  ind
 * is the index for the renderer; use ui_entry_renderer_lookup to get it.
 * Will return -1 if the renderer is not valid or does not have an implemented
 * backend.
 */
int ui_entry_renderer_query_value_width(int ind)
{
	if (ind <= 0 || ind > renderer_count || !renderers[ind - 1].backend) {
		return -1;
	}
	return (*renderers[ind - 1].backend->vw_func)(renderers + ind - 1);
}


/**
 * Query a renderer for the number of characters used to draw the combined
 * value.  ind is the index for the renderer; use ui_entry_renderer_lookup to
 * get it.  Will return -1 if the renderer is not valid or has not been
 * attached to another renderer to show the combined value.
 */
int ui_entry_renderer_query_combined_width(int ind)
{
	const struct renderer_info *combined_renderer;

	if (ind <= 0 || ind > renderer_count ||
		renderers[ind - 1].combined_renderer_index <= 0 ||
		renderers[ind - 1].combined_renderer_index > renderer_count) {
		return -1;
	}
	combined_renderer =
		&renderers[renderers[ind - 1].combined_renderer_index - 1];
	return (combined_renderer->backend) ?
		(*combined_renderer->backend->vw_func)(combined_renderer) : -1;
}


/**
 * Returns the combiner index, suitable as the first argument to
 * ui_entry_combiner_get_funcs(), for the given user interface renderer.
 * Returns zero if the renderer is not valid or has an invalid combiner.
 */
int ui_entry_renderer_query_combiner(int ind)
{
	return (ind > 0 && ind <= renderer_count) ?
		renderers[ind - 1].combiner_index : 0;
}


/**
 * Use a renderer to draw a set of values and, optionally, their label and
 * combined value.  ind is the index for the renderer; use
 * ui_entry_renderer_lookup to get it.  label and nlabel specify the label to
 * draw.  If nlabel is 0, no label will be drawn.  vals, auxvals, and n set
 * the values to draw.  vals and auxvals each refer to n values.  details
 * controls certain aspects of the rendering including positions and the
 * handling of the combined value.  The comments for it in
 * ui-entry-renderers.h describe it in more detail.
 */
void ui_entry_renderer_apply(int ind, const wchar_t *label, int nlabel,
	const int *vals, const int *auxvals, int n,
	const struct ui_entry_details *details)
{
	if (ind <= 0 || ind > renderer_count || !renderers[ind - 1].backend) {
		return;
	}
	(*renderers[ind - 1].backend->func)(
		label, nlabel, vals, auxvals, n, details, renderers + ind - 1);
}


/*
 * Change the colors, label colors, or symbols used by a renderer.  Any of
 * the three may be a null pointer which leaves the current setting for that
 * unchanged.  Returns zero if successful.  Returns a nonzero value if the
 * renderer index is invalid or it was not possible to convert one or more
 * of the values to change.
 */
int ui_entry_renderer_customize(int ind, const char *colors,
	const char *label_colors, const char *symbols)
{
	size_t length;

	if (ind <= 0 || ind > renderer_count) {
		return 1;
	}
	if (colors != 0) {
		length = strlen(colors);
		convert_chars_to_attrs(colors,
			(length < (size_t) renderers[ind - 1].ncolors) ?
			(int) length : renderers[ind - 1].ncolors,
			renderers[ind - 1].colors);
	}
	if (label_colors != 0) {
		length = strlen(label_colors);
		convert_chars_to_attrs(label_colors,
			(length < (size_t) renderers[ind - 1].ncolors) ?
			(int) length : renderers[ind - 1].ncolors,
			renderers[ind - 1].label_colors);
	}
	if (symbols != 0) {
		wchar_t *tmp = mem_alloc((renderers[ind - 1].nsym + 1) *
			sizeof(*tmp));

		length = text_mbstowcs(tmp, symbols,
			renderers[ind - 1].nsym + 1);
		if (length == (size_t)-1) {
			return 1;
		}
		(void) memcpy(renderers[ind - 1].symbols, tmp,
			(((int)length < renderers[ind - 1].nsym) ?
			(int)length : renderers[ind - 1].nsym) *
			sizeof(*renderers[ind - 1].symbols));
		mem_free(tmp);
	}
	return 0;
}


/*
 * If ind is valid, returns a dynamically allocated string with the current
 * setting for the palette of colors.  That string should be released with
 * string_free().  Otherwise, returns NULL.
 */
char *ui_entry_renderer_get_colors(int ind)
{
	if (ind < 1 || ind > renderer_count) {
		return NULL;
	}
	return convert_attrs_to_chars(renderers[ind - 1].colors,
		renderers[ind - 1].ncolors);
}


/*
 * If ind is valid, returns a dynamically allocated string with the current
 * setting for the palette of label colors.  That string should be released
 * with string_free().  Otherwise, returns NULL.
 */
char *ui_entry_renderer_get_label_colors(int ind)
{
	if (ind < 1 || ind > renderer_count) {
		return NULL;
	}
	return convert_attrs_to_chars(renderers[ind - 1].label_colors,
		renderers[ind - 1].nlabcolors);
}


/*
 * If ind is valid, returns a dynamically allocated string with the current
 * setting for the palette of symbols.  That string should be released
 * with string_free().  Otherwise, returns NULL.
 */
char *ui_entry_renderer_get_symbols(int ind)
{
	if (ind < 1 || ind > renderer_count) {
		return NULL;
	}
	return string_make(format("%ls", renderers[ind - 1].symbols));
}


static void format_int(int i, bool add_one, wchar_t zero, wchar_t overflow,
	bool nonneg, bool use_sign, int nbuf, wchar_t *buf)
{
	static bool first_call = true;
	static wchar_t wdigits[14];
	const char *digits = "0123456789+- ";
	int j = nbuf - 1;
	div_t parts;

	if (first_call) {
		if (text_mbstowcs(wdigits, digits, 14) == (size_t)-1) {
			quit("Invalid encoding for digits");
		}
		first_call = false;
	}

	/* Special handling for first digit. */
	assert(nbuf > 0);
	if (i == 0 && !add_one) {
		parts.quot = 0;
		parts.rem = 0;
		buf[j] = zero;
	} else {
		parts = div(i, 10);
		if (add_one) {
			++parts.rem;
			if (parts.rem == 10) {
				parts.rem = 0;
				++parts.quot;
			}
		}
		buf[j] = wdigits[parts.rem];
	}
	--j;

	while (parts.quot > 0 && j >= 0) {
		parts = div(parts.quot, 10);
		buf[j] = wdigits[parts.rem];
		--j;
	}

	if (parts.quot > 0 || (use_sign && j == -1)) {
		if (use_sign) {
			buf[0] = (nonneg) ? wdigits[10] : wdigits[11];
			j = 1;
		} else {
			j = 0;
		}
		while (j < nbuf) {
			buf[j] = overflow;
			++j;
		}
	} else {
		 if (use_sign && (i != 0 || add_one || zero == wdigits[0])) {
			buf[j] = (nonneg) ? wdigits[10] : wdigits[11];
			--j;
		}
		while (j >= 0) {
			buf[j] = wdigits[12];
			--j;
		}
	}
}


static void show_combined_generic(const struct renderer_info *info,
	const struct ui_entry_details *details, int vcombined, int acombined)
{
	struct ui_entry_details combined_details;

	combined_details.label_position = loc(0, 0);
	combined_details.value_position = details->combined_position;
	combined_details.position_step = loc(0, 0);
	combined_details.combined_position = loc(0, 0);
	combined_details.vertical_label = false;
	combined_details.alternate_color_first = false;
	combined_details.known_rune = details->known_rune;
	combined_details.show_combined = false;
	ui_entry_renderer_apply(info->combined_renderer_index, NULL,
		0, &vcombined, &acombined, 1, &combined_details);
}


/**
 * Result is  0 (no resistance)), 1 (resistance), 2 (vulnerable), 3 (immune),
 * 4 (unknown), 5 (not present).
 */
static int convert_vanilla_res_level(int i)
{
	int result;

	if (i == UI_ENTRY_UNKNOWN_VALUE) {
		result = 4;
	} else if (i == UI_ENTRY_VALUE_NOT_PRESENT) {
		result = 5;
	} else if (i >= 3) {
		result = 3;
	} else if (i >= 1) {
		result = 1;
	} else if (i <= -1) {
		result = 2;
	} else {
		result = 0;
	}
	return result;
}


static void renderer_COMPACT_RESIST_RENDERER_WITH_COMBINED_AUX(
	const wchar_t *label,
	int nlabel,
	const int *vals,
	const int *auxvals,
	int n,
	const struct ui_entry_details *details,
	const struct renderer_info *info)
{
	/*
	 * Fastest varying is no timed effect, timed resistance,
	 * timed vulnerability, timed immunity, unknown timed effect,
	 * no value for timed effect
	 */
	const int combined_effect_tbl[6][6] = {
		/* No permanent effect */
		{ 2, 6,  9, 11, 2, 2 },
		/* Permanent resistance */
		{ 3, 7, 10, 12, 3, 3 },
		/* Permanent vulnerability */
		{ 4, 8,  4, 13, 4, 4 },
		/* Permanent immunity */
		{ 5, 5,  5,  5, 5, 5 },
		/* Unknown permanent effect */
		{ 0, 0,  0,  0, 0, 0 },
		/* No value for permanent effect */
		{ 1, 1,  1,  1, 1, 1 }
	};
	struct loc p = details->value_position;
	int color_offset = (details->alternate_color_first) ? 14 : 0;
	struct ui_entry_combiner_funcs combiner;
	int vc, ac;
	int i;

	/* Check for defaults that are too short in list-ui-entry-renders.h. */
	assert(info->ncolors >= 14 && info->nlabcolors >= 13 &&
		info->nsym >= 14);

	for (i = 0; i < n; ++i) {
		int untimed_effect = convert_vanilla_res_level(vals[i]);
		int timed_effect = convert_vanilla_res_level(auxvals[i]);
		int palette_index;

		assert(untimed_effect >= 0 && untimed_effect < 6 &&
			timed_effect >= 0 && timed_effect < 6);
		palette_index =
			combined_effect_tbl[untimed_effect][timed_effect];
		Term_putch(p.x, p.y,
			info->colors[palette_index + color_offset],
			info->symbols[palette_index]);
		p = loc_sum(p, details->position_step);
		color_offset ^= 14;
	}

	if (nlabel <= 0 && !details->show_combined) {
		return;
	}

	if (ui_entry_combiner_get_funcs(info->combiner_index, &combiner)) {
		assert(0);
	}
	(*combiner.vec_func)(n, vals, auxvals, &vc, &ac);

	if (nlabel > 0) {
		const int combined_label_tbl[6][6] = {
			/* No permanent effect */
			{ 1, 5, 8, 10, 1, 1 },
			/* Permanent resistance */
			{ 2, 6, 9, 11, 2, 2 },
			/* Permanent vulnerability */
			{ 3, 7, 3, 12, 3, 3 },
			/* Permanent immunity */
			{ 4, 4, 4,  4, 4, 4 },
			/* Unknown permanent effect */
			{ 1, 5, 8, 10, 1, 1 },
			/* No value for permanent effect */
			{ 1, 5, 8, 10, 1, 1 }
		};
		int palette_index;

		if (details->known_rune) {
			int untimed_effect = convert_vanilla_res_level(vc);
			int timed_effect = convert_vanilla_res_level(ac);

			assert(untimed_effect >= 0 && untimed_effect < 6 &&
				timed_effect >= 0 && timed_effect < 6);
			palette_index =
				combined_label_tbl[untimed_effect][timed_effect];
		} else {
			palette_index = 0;
		}

		if (details->vertical_label) {
			p = details->label_position;
			for (i = 0; i < nlabel; ++i) {
				Term_putch(p.x, p.y,
					info->label_colors[palette_index],
					label[i]);
				p.y += 1;
			}
		} else {
			Term_queue_chars(details->label_position.x,
				details->label_position.y, nlabel,
				info->label_colors[palette_index], label);
		}
	}

	if (details->show_combined) {
		show_combined_generic(info, details, vc, ac);
	}
}


static int valuewidth_COMPACT_RESIST_RENDERER_WITH_COMBINED_AUX(
	const struct renderer_info *info)
{
	return 1;
}


static void renderer_COMPACT_FLAG_RENDERER_WITH_COMBINED_AUX(
	const wchar_t *label,
	int nlabel,
	const int *vals,
	const int *auxvals,
	int n,
	const struct ui_entry_details *details,
	const struct renderer_info *info)
{
	struct loc p = details->value_position;
	int color_offset = (details->alternate_color_first) ? 5 : 0;
	struct ui_entry_combiner_funcs combiner;
	int vc, ac;
	int i;

	/* Check for defaults that are too short in list-ui-entry-renders.h. */
	assert(info->ncolors >= 5 && info->nlabcolors >= 4 && info->nsym >= 5);

	for (i = 0; i < n; ++i) {
		int palette_index = 2;

		if (vals[i] == UI_ENTRY_UNKNOWN_VALUE) {
			palette_index = 0;
		} else if (vals[i] == UI_ENTRY_VALUE_NOT_PRESENT) {
			palette_index = 1;
		} else if (vals[i]) {
			palette_index = 3;
		}
		if (auxvals[i] && auxvals[i] != UI_ENTRY_UNKNOWN_VALUE &&
			auxvals[i] != UI_ENTRY_VALUE_NOT_PRESENT) {
			if (vals[i] == 0) {
				palette_index = 4;
			}
		}
		Term_putch(p.x, p.y,
			info->colors[palette_index + color_offset],
			info->symbols[palette_index]);
		p = loc_sum(p, details->position_step);
		color_offset ^= 5;
	}

	if (nlabel <= 0 && !details->show_combined) {
		return;
	}

	if (ui_entry_combiner_get_funcs(info->combiner_index, &combiner)) {
		assert(0);
	}
	(*combiner.vec_func)(n, vals, auxvals, &vc, &ac);

	if (nlabel > 0) {
		int palette_index = 1;

		if (! details->known_rune) {
			palette_index = 0;
		} else if (vc && vc != UI_ENTRY_UNKNOWN_VALUE &&
			vc != UI_ENTRY_VALUE_NOT_PRESENT) {
			palette_index = 2;
		} else if (ac && ac != UI_ENTRY_UNKNOWN_VALUE &&
			ac != UI_ENTRY_VALUE_NOT_PRESENT) {
			palette_index = 3;
		}
		if (details->vertical_label) {
			p = details->label_position;
			for (i = 0; i < nlabel; ++i) {
				Term_putch(p.x, p.y,
					info->label_colors[palette_index],
					label[i]);
				p.y += 1;
			}
		} else {
			Term_queue_chars(details->label_position.x,
				details->label_position.y, nlabel,
				info->label_colors[palette_index], label);
		}
	}

	if (details->show_combined) {
		show_combined_generic(info, details, vc, ac);
	}
}


static int valuewidth_COMPACT_FLAG_RENDERER_WITH_COMBINED_AUX(
	const struct renderer_info *info)
{
	return 1;
}


static void renderer_NUMERIC_AS_SIGN_RENDERER_WITH_COMBINED_AUX(
	const wchar_t *label,
	int nlabel,
	const int *vals,
	const int *auxvals,
	int n,
	const struct ui_entry_details *details,
	const struct renderer_info *info)
{
	struct loc p = details->value_position;
	int color_offset = (details->alternate_color_first) ? 7 : 0;
	struct ui_entry_combiner_funcs combiner;
	int vc, ac;
	int i;

	/* Check for defaults that are too short in list-ui-entry-renders.h. */
	assert(info->ncolors >= 7 && info->nlabcolors >= 6 && info->nsym >= 7);

	for (i = 0; i < n; ++i) {
		int palette_index = 2;

		if (vals[i] == UI_ENTRY_UNKNOWN_VALUE || (vals[i] == 0 &&
			auxvals[i] == UI_ENTRY_UNKNOWN_VALUE)) {
			palette_index = 0;
		} else if (vals[i] == UI_ENTRY_VALUE_NOT_PRESENT) {
			palette_index = 1;
		} else if (vals[i] > 0) {
			palette_index = 3;
		} else if (vals[i] < 0) {
			palette_index = 4;
		}
		if (auxvals[i] > 0 && auxvals[i] != UI_ENTRY_UNKNOWN_VALUE &&
			auxvals[i] != UI_ENTRY_VALUE_NOT_PRESENT) {
			if (vals[i] == 0) {
				palette_index = 5;
			}
		} else if (auxvals[i] < 0 &&
			auxvals[i] != UI_ENTRY_UNKNOWN_VALUE &&
			auxvals[i] != UI_ENTRY_VALUE_NOT_PRESENT) {
			if (vals[i] == 0) {
				palette_index = 6;
			}
		}
		Term_putch(p.x, p.y,
			info->colors[palette_index + color_offset],
			info->symbols[palette_index]);
		p = loc_sum(p, details->position_step);
		color_offset ^= 7;
	}

	if (nlabel <= 0 && !details->show_combined) {
		return;
	}

	if (ui_entry_combiner_get_funcs(info->combiner_index, &combiner)) {
		assert(0);
	}
	(*combiner.vec_func)(n, vals, auxvals, &vc, &ac);

	if (nlabel > 0) {
		int palette_index;

		if (! details->known_rune) {
			palette_index = 0;
		} else if (vc == UI_ENTRY_UNKNOWN_VALUE ||
			vc == UI_ENTRY_VALUE_NOT_PRESENT || vc == 0) {
			if (ac == UI_ENTRY_UNKNOWN_VALUE ||
				ac == UI_ENTRY_VALUE_NOT_PRESENT || ac == 0) {
				palette_index = 1;
			} else if (ac > 0) {
				palette_index = 4;
			} else {
				palette_index = 5;
			}
		} else if (vc > 0) {
			palette_index = 2;
		} else  {
			palette_index = 3;
		}
		if (details->vertical_label) {
			p = details->label_position;
			for (i = 0; i < nlabel; ++i) {
				Term_putch(p.x, p.y,
					info->label_colors[palette_index],
					label[i]);
				p.y += 1;
			}
		} else {
			Term_queue_chars(details->label_position.x,
				details->label_position.y, nlabel,
				info->label_colors[palette_index], label);
		}
	}

	if (details->show_combined) {
		show_combined_generic(info, details, vc, ac);
	}
}


static int valuewidth_NUMERIC_AS_SIGN_RENDERER_WITH_COMBINED_AUX(
	const struct renderer_info *info)
{
	return 1;
}


static void renderer_NUMERIC_RENDERER_WITH_COMBINED_AUX(
	const wchar_t *label,
	int nlabel,
	const int *vals,
	const int *auxvals,
	int n,
	const struct ui_entry_details *details,
	const struct renderer_info *info)
{
	struct loc p = details->value_position;
	int color_offset = (details->alternate_color_first) ? 7 : 0;
	int nbuf = info->ndigit + ((info->sign == UI_ENTRY_NO_SIGN) ? 0 : 1);
	wchar_t *buffer = mem_alloc(nbuf * sizeof(*buffer));
	struct ui_entry_combiner_funcs combiner;
	int vc, ac;
	int i;

	/* Check for defaults that are too short in list-ui-entry-renders.h. */
	assert(info->ncolors >= 7 && info->nlabcolors >= 6 && info->nsym >= 7);

	for (i = 0; i < n; ++i) {
		int palette_index;

		if (vals[i] == UI_ENTRY_UNKNOWN_VALUE || (vals[i] == 0 &&
			auxvals[i] == UI_ENTRY_UNKNOWN_VALUE)) {
			palette_index = 0;
			format_int(0, false, info->symbols[0],
				info->symbols[0], true,
				info->sign == UI_ENTRY_ALWAYS_SIGN, nbuf,
				buffer);
		} else if (vals[i] == UI_ENTRY_VALUE_NOT_PRESENT) {
			palette_index = 1;
			format_int(0, false, info->symbols[1],
				info->symbols[1], true,
				info->sign == UI_ENTRY_ALWAYS_SIGN, nbuf,
				buffer);
		} else if (vals[i] > 0) {
			palette_index = 3;
			format_int(vals[i], false, info->symbols[2],
				info->symbols[3], true,
				info->sign == UI_ENTRY_ALWAYS_SIGN,
				nbuf, buffer);
		} else if (vals[i] < 0) {
			int v;
			bool o;

			palette_index = 4;
			if (vals[i] == INT_MIN) {
				v = -(INT_MIN + 1);
				o = true;
			} else {
				v = -vals[i];
				o = false;
			}
			format_int(v, o, info->symbols[2], info->symbols[4],
				false, info->sign != UI_ENTRY_NO_SIGN, nbuf,
				buffer);
		} else if (auxvals[i] > 0 &&
			auxvals[i] != UI_ENTRY_UNKNOWN_VALUE &&
			auxvals[i] != UI_ENTRY_VALUE_NOT_PRESENT) {
			palette_index = 5;
			format_int(auxvals[i], false, info->symbols[2],
				info->symbols[5], true,
				info->sign == UI_ENTRY_ALWAYS_SIGN, nbuf,
				buffer);
		} else if (auxvals[i] < 0 &&
			auxvals[i] != UI_ENTRY_UNKNOWN_VALUE &&
			auxvals[i] != UI_ENTRY_VALUE_NOT_PRESENT) {
			int v;
			bool o;

			palette_index = 6;
			if (auxvals[i] == INT_MIN) {
				v = -(INT_MIN + 1);
				o = true;
			} else {
				v = -vals[i];
				o = false;
			}
			format_int(v, o, info->symbols[2], info->symbols[6],
				false, info->sign != UI_ENTRY_NO_SIGN, nbuf,
				buffer);
		} else {
			palette_index = 2;
			format_int(0, false, info->symbols[2],
				info->symbols[3], true,
				info->sign == UI_ENTRY_ALWAYS_SIGN,
				nbuf, buffer);
		}
		Term_queue_chars(p.x, p.y, nbuf,
			info->colors[palette_index + color_offset], buffer);
		if (info->units_nlabel != 0) {
			Term_queue_chars(p.x + nbuf, p.y, info->units_nlabel,
				info->colors[palette_index + color_offset],
				info->units_label);
		}
		p = loc_sum(p, details->position_step);
		color_offset ^= 7;
	}

	mem_free(buffer);

	if (nlabel <= 0 && !details->show_combined) {
		return;
	}

	if (ui_entry_combiner_get_funcs(info->combiner_index, &combiner)) {
		assert(0);
	}
	(*combiner.vec_func)(n, vals, auxvals, &vc, &ac);

	if (nlabel > 0) {
		int palette_index;

		if (! details->known_rune) {
			palette_index = 0;
		} else if (vc == 0 || vc == UI_ENTRY_UNKNOWN_VALUE ||
			vc == UI_ENTRY_VALUE_NOT_PRESENT) {
			if (ac == 0 || ac == UI_ENTRY_UNKNOWN_VALUE ||
				ac == UI_ENTRY_VALUE_NOT_PRESENT) {
				palette_index = 1;
			} else if (ac > 0) {
				palette_index = 4;
			} else {
				palette_index = 5;
			}
		} else if (vc > 0) {
			palette_index = 2;
		} else {
			palette_index = 3;
		}
		if (details->vertical_label) {
			p = details->label_position;
			for (i = 0; i < nlabel; ++i) {
				Term_putch(p.x, p.y,
					info->label_colors[palette_index],
					label[i]);
				p.y += 1;
			}
		} else {
			Term_queue_chars(details->label_position.x,
				details->label_position.y, nlabel,
				info->label_colors[palette_index], label);
		}
	}

	if (details->show_combined) {
		show_combined_generic(info, details, vc, ac);
	}
}


static int valuewidth_NUMERIC_RENDERER_WITH_COMBINED_AUX(
	const struct renderer_info *info)
{
	return info->ndigit + ((info->sign == UI_ENTRY_NO_SIGN) ? 0 : 1) +
		info->units_nlabel;
}


static void renderer_NUMERIC_RENDERER_WITH_BOOL_AUX(
	const wchar_t *label,
	int nlabel,
	const int *vals,
	const int *auxvals,
	int n,
	const struct ui_entry_details *details,
	const struct renderer_info *info)
{
	struct loc p = details->value_position;
	int color_offset = (details->alternate_color_first) ? 8 : 0;
	int nbuf = info->ndigit + ((info->sign == UI_ENTRY_NO_SIGN) ? 0 : 1);
	wchar_t *buffer = mem_alloc(nbuf * sizeof(*buffer));
	struct ui_entry_combiner_funcs combiner;
	int vc, ac;
	int i;

	/* Check for defaults that are too short in list-ui-entry-renders.h. */
	assert(info->ncolors >= 8 && info->nlabcolors >= 7 && info->nsym >= 6);

	for (i = 0; i < n; ++i) {
		int palette_index;

		if (vals[i] == UI_ENTRY_UNKNOWN_VALUE || (vals[i] == 0 &&
			auxvals[i] == UI_ENTRY_UNKNOWN_VALUE)) {
			palette_index = 0;
			format_int(0, false, info->symbols[0],
				info->symbols[0], true,
				info->sign == UI_ENTRY_ALWAYS_SIGN, nbuf,
				buffer);
		} else if (vals[i] == UI_ENTRY_VALUE_NOT_PRESENT) {
			palette_index = 1;
			format_int(0, false, info->symbols[1],
				info->symbols[1], true,
				info->sign == UI_ENTRY_ALWAYS_SIGN, nbuf,
				buffer);
		} else if (vals[i] > 0) {
			if (auxvals[i] != 0 &&
				auxvals[i] != UI_ENTRY_UNKNOWN_VALUE &&
				auxvals[i] != UI_ENTRY_VALUE_NOT_PRESENT) {
				palette_index = 5;
			} else {
				palette_index = 4;
			}
			format_int(vals[i], false, info->symbols[2],
				info->symbols[4], true,
				info->sign == UI_ENTRY_ALWAYS_SIGN,
				nbuf, buffer);
		} else if (vals[i] < 0) {
			int v;
			bool o;

			if (auxvals[i] != 0 &&
				auxvals[i] != UI_ENTRY_UNKNOWN_VALUE &&
				auxvals[i] != UI_ENTRY_VALUE_NOT_PRESENT) {
				palette_index = 7;
			} else {
				palette_index = 6;
			}
			if (vals[i] == INT_MIN) {
				v = -(INT_MIN + 1);
				o = true;
			} else {
				v = -vals[i];
				o = false;
			}
			format_int(v, o, info->symbols[2], info->symbols[5],
				false, info->sign != UI_ENTRY_NO_SIGN,
				nbuf, buffer);
		} else {
			int zerosym;

			if (auxvals[i] != 0 &&
				auxvals[i] != UI_ENTRY_UNKNOWN_VALUE &&
				auxvals[i] != UI_ENTRY_VALUE_NOT_PRESENT) {
				palette_index = 3;
				zerosym = 3;
			} else {
				palette_index = 2;
				zerosym = 2;
			}
			format_int(0, false, info->symbols[zerosym],
				info->symbols[4], true,
				info->sign == UI_ENTRY_ALWAYS_SIGN,
				nbuf, buffer);
		}
		Term_queue_chars(p.x, p.y, nbuf,
			info->colors[palette_index + color_offset], buffer);
		if (info->units_nlabel != 0) {
			Term_queue_chars(p.x + nbuf, p.y, info->units_nlabel,
				info->colors[palette_index + color_offset],
				info->units_label);
		}
		p = loc_sum(p, details->position_step);
		color_offset ^= 8;
	}

	mem_free(buffer);

	if (nlabel <= 0 && !details->show_combined) {
		return;
	}

	if (ui_entry_combiner_get_funcs(info->combiner_index, &combiner)) {
		assert(0);
	}
	(*combiner.vec_func)(n, vals, auxvals, &vc, &ac);

	if (nlabel > 0) {
		bool acbool = ac && ac != UI_ENTRY_UNKNOWN_VALUE &&
			ac != UI_ENTRY_VALUE_NOT_PRESENT;
		int palette_index;

		if (! details->known_rune) {
			palette_index = 0;
		} else if (vc == 0 || vc == UI_ENTRY_UNKNOWN_VALUE ||
			vc == UI_ENTRY_VALUE_NOT_PRESENT) {
			palette_index = (acbool) ? 6 : 5;
		} else if (vc > 0) {
			palette_index = (acbool) ? 2 : 1;
		} else {
			palette_index = (acbool) ? 4 : 3;
		}
		if (details->vertical_label) {
			p = details->label_position;
			for (i = 0; i < nlabel; ++i) {
				Term_putch(p.x, p.y,
					info->label_colors[palette_index],
					label[i]);
				p.y += 1;
			}
		} else {
			Term_queue_chars(details->label_position.x,
				details->label_position.y, nlabel,
				info->label_colors[palette_index], label);
		}
	}

	if (details->show_combined) {
		show_combined_generic(info, details, vc, ac);
	}
}


static int valuewidth_NUMERIC_RENDERER_WITH_BOOL_AUX(
	const struct renderer_info *info)
{
	return info->ndigit + ((info->sign == UI_ENTRY_NO_SIGN) ? 0 : 1) +
		info->units_nlabel;
}


static int lookup_backend_by_name(const char *name)
{
	int n = N_ELEMENTS(backend_names);
	int i = 0;

	while (i < n) {
		if (streq(backend_names[i], name)) {
			return i + 1;
		}
		++i;
	}
	return 0;
}


static char *convert_attrs_to_chars(const int *attr, int n)
{
	char buf[9];
	int nbuf = N_ELEMENTS(buf) - 1;
	char *result = NULL;
	int i = 0, j = 0;

	while (1) {
		if (i == nbuf || j >= n) {
			buf[i] = '\0';
			result = string_append(result, buf);
			if (j >= n) {
				return result;
			}
			i = 0;
		}
		if (attr[j] >= 0 && attr[j] < BASIC_COLORS) {
			buf[i] = color_table[attr[j]].index_char;
		} else {
			buf[i] = 'w';
		}
		++i;
		++j;
	}
}


static void convert_chars_to_attrs(const char *colors, int n, int *attr)
{
	int i;

	for (i = 0; i < n; ++i) {
		attr[i] = color_char_to_attr(colors[i]);
	}
}


static void augment_colors(const char *colors, int **attr, int *nattr)
{
	int n = (int) strlen(colors);

	if (*nattr < n) {
		int *newattr = mem_alloc(sizeof(*newattr) * n);

		if (*nattr > 0) {
			(void) memcpy(newattr, *attr, *nattr *
				sizeof(*newattr));
		}
		mem_free(*attr);
		convert_chars_to_attrs(colors + *nattr, n - *nattr,
			newattr + *nattr);
		*attr = newattr;
		*nattr = n;
	}
}


static void augment_symbols(const char *symbols, wchar_t **s, int *n)
{
	wchar_t defsym[MAX_PALETTE];
	size_t nd = text_mbstowcs(defsym, symbols, MAX_PALETTE);

	if (nd == (size_t)-1) {
		quit("Invalid encoding for default symbols");
	}
	if (*n < (int) nd) {
		wchar_t *newsym = mem_alloc((nd + 1) * sizeof(*newsym));

		if (*n > 0) {
			(void) memcpy(newsym, *s, *n * sizeof(*newsym));
		}
		mem_free(*s);
		(void) memcpy(newsym + *n, defsym + *n,
			((int) nd - *n) * sizeof(*newsym));
		/* Ensure null termination. */
		if (text_mbstowcs(newsym + nd, "", 1) == (size_t)-1) {
			quit("Couldn't terminate null character string");
		}
		*n = (int) nd;
		*s = newsym;
	}
}


static enum parser_error parse_renderer_name(struct parser *p)
{
	const char *name = parser_getstr(p, "name");
	int ind = ui_entry_renderer_lookup(name);
	struct renderer_info *renderer;

	if (ind == 0) {
		if (renderer_count >= renderer_allocated) {
			if (renderer_allocated > INT_MAX / 2) {
				return PARSE_ERROR_TOO_MANY_ENTRIES;
			}
			renderer_allocated = (renderer_allocated == 0) ?
				4 : renderer_allocated * 2;
			renderers = mem_realloc(renderers,
				renderer_allocated * sizeof(*renderers));
		}
		renderer = renderers + renderer_count;
		++renderer_count;
		renderer->name = string_make(name);
		renderer->comb_rend_nm = NULL;
		renderer->colors = NULL;
		renderer->label_colors = NULL;
		renderer->symbols = NULL;
		renderer->units_label = NULL;
		renderer->backend = NULL;
		renderer->combined_renderer_index = 0;
		renderer->combiner_index = 0;
		renderer->ncolors = 0;
		renderer->nlabcolors = 0;
		renderer->nsym = 0;
		renderer->ndigit = INT_MIN;
		renderer->sign = UI_ENTRY_SIGN_DEFAULT;
		renderer->units_nlabel = 0;
	} else {
		renderer = renderers + ind - 1;
	}
	parser_setpriv(p, renderer);
	return PARSE_ERROR_NONE;
}


static enum parser_error parse_renderer_code(struct parser *p)
{
	struct renderer_info *renderer = parser_priv(p);
	const char *code;
	int ind;

	if (!renderer) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	code = parser_getstr(p, "code");
	ind = lookup_backend_by_name(code);
	if (ind == 0) {
		return PARSE_ERROR_INVALID_VALUE;
	}
	renderer->backend = backends + ind - 1;
	return PARSE_ERROR_NONE;
}


static enum parser_error parse_renderer_combine(struct parser *p)
{
	struct renderer_info *renderer = parser_priv(p);
	const char *name;

	if (!renderer) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	name = parser_getstr(p, "combine");
	renderer->combiner_index = ui_entry_combiner_lookup(name);
	return (renderer->combiner_index) ?
		PARSE_ERROR_NONE : PARSE_ERROR_INVALID_VALUE;
}


static enum parser_error parse_renderer_colors(struct parser *p)
{
	struct renderer_info *renderer = parser_priv(p);
	const char *colors;
	size_t n;

	if (!renderer) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	colors = parser_getstr(p, "colors");
	mem_free(renderer->colors);
	n = strlen(colors);
	renderer->ncolors = (n < MAX_PALETTE) ? (int) n : MAX_PALETTE;
	renderer->colors = mem_alloc(renderer->ncolors *
		sizeof(*renderer->colors));
	convert_chars_to_attrs(colors, renderer->ncolors, renderer->colors);
	return PARSE_ERROR_NONE;
}


static enum parser_error parse_renderer_labelcolors(struct parser *p)
{
	struct renderer_info *renderer = parser_priv(p);
	const char *colors;
	size_t n;

	if (!renderer) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	colors = parser_getstr(p, "colors");
	mem_free(renderer->label_colors);
	n = strlen(colors);
	renderer->nlabcolors = (n < MAX_PALETTE) ? (int) n : MAX_PALETTE;
	renderer->label_colors = mem_alloc(renderer->nlabcolors *
		sizeof(*renderer->label_colors));
	convert_chars_to_attrs(colors, renderer->nlabcolors,
		renderer->label_colors);
	return PARSE_ERROR_NONE;
}


static enum parser_error parse_renderer_symbols(struct parser *p)
{
	struct renderer_info *renderer = parser_priv(p);
	const char *symbols;
	size_t n;

	if (!renderer) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	symbols = parser_getstr(p, "symbols");
	mem_free(renderer->symbols);
	renderer->symbols = mem_alloc((MAX_PALETTE + 1) *
		sizeof(*renderer->symbols));
	n = text_mbstowcs(renderer->symbols, symbols, MAX_PALETTE);
	if (n == (size_t)-1) {
		return PARSE_ERROR_INVALID_VALUE;
	}
	/* Make sure there's a null terminator. */
	if (text_mbstowcs(renderer->symbols + n, "", 1) == (size_t)-1) {
		return PARSE_ERROR_INVALID_VALUE;
	}
	renderer->nsym = n;
	return PARSE_ERROR_NONE;
}


static enum parser_error parse_renderer_combined_renderer(struct parser *p)
{
	struct renderer_info *renderer = parser_priv(p);
	const char *name;

	if (!renderer) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	name = parser_getstr(p, "name");
	string_free(renderer->comb_rend_nm);
	renderer->comb_rend_nm = string_make(name);
	return PARSE_ERROR_NONE;
}


static enum parser_error parse_renderer_ndigit(struct parser *p)
{
	struct renderer_info *renderer = parser_priv(p);
	int ndigit;

	if (!renderer) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	ndigit = parser_getint(p, "ndigit");
	if (ndigit < 1) {
		return PARSE_ERROR_INVALID_VALUE;
	}
	renderer->ndigit = ndigit;
	return PARSE_ERROR_NONE;
}


static enum parser_error parse_renderer_sign(struct parser *p)
{
	struct renderer_info *renderer = parser_priv(p);
	const char *sign;

	if (!renderer) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	sign = parser_getstr(p, "sign");
	if (streq(sign, "NO_SIGN")) {
		renderer->sign = UI_ENTRY_NO_SIGN;
	} else if (streq(sign, "ALWAYS_SIGN")) {
		renderer->sign = UI_ENTRY_ALWAYS_SIGN;
	} else if (streq(sign, "NEGATIVE_SIGN")) {
		renderer->sign = UI_ENTRY_NEGATIVE_SIGN;
	} else {
		return PARSE_ERROR_INVALID_VALUE;
	}
	return PARSE_ERROR_NONE;
}


static enum parser_error parse_renderer_units(struct parser *p)
{
	struct renderer_info *renderer = parser_priv(p);
	const char *units;
	size_t n, n2;

	if (!renderer) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	mem_free(renderer->units_label);
	renderer->units_label = mem_alloc((MAX_UNITS_LABEL + 1) *
		sizeof(*renderer->units_label));
	units = parser_getstr(p, "units");
	n = text_mbstowcs(renderer->units_label, units, MAX_UNITS_LABEL);
	if (n == (size_t)-1) {
		return PARSE_ERROR_INVALID_VALUE;
	}
	/* Ensure null termination. */
	n2 = text_mbstowcs(renderer->units_label + n, "", 1);
	if (n2 == (size_t)-1) {
		return PARSE_ERROR_INVALID_VALUE;
	}
	renderer->units_nlabel = n;
	return PARSE_ERROR_NONE;
}


static struct parser *init_parse_ui_entry_renderer(void)
{
	struct parser *p = parser_new();

	parser_setpriv(p, NULL);
	parser_reg(p, "name str name", parse_renderer_name);
	parser_reg(p, "code str code", parse_renderer_code);
	parser_reg(p, "combine str combine", parse_renderer_combine);
	parser_reg(p, "colors str colors", parse_renderer_colors);
	parser_reg(p, "labelcolors str colors", parse_renderer_labelcolors);
	parser_reg(p, "symbols str symbols", parse_renderer_symbols);
	parser_reg(p, "ndigit int ndigit", parse_renderer_ndigit);
	parser_reg(p, "sign str sign", parse_renderer_sign);
	parser_reg(p, "units str units", parse_renderer_units);
	parser_reg(p, "combined-renderer str name",
		parse_renderer_combined_renderer);
	return p;
}


static errr run_parse_ui_entry_renderer(struct parser *p)
{
	return parse_file(p, "ui_entry_renderer");
}


static errr finish_parse_ui_entry_renderer(struct parser *p)
{
	int i;

	for (i = 0; i < renderer_count; ++i) {
		/* Get the combined value renderer. */
		if (renderers[i].comb_rend_nm) {
			renderers[i].combined_renderer_index =
				ui_entry_renderer_lookup(renderers[i].comb_rend_nm);
			string_free(renderers[i].comb_rend_nm);
			renderers[i].comb_rend_nm = NULL;
		}

		if (!renderers[i].backend) {
			continue;
		}

		/* Use the default combiner if nothing was set. */
		if (!renderers[i].combiner_index) {
			renderers[i].combiner_index = ui_entry_combiner_lookup(
				renderers[i].backend->default_combiner_name);
			assert(renderers[i].combiner_index != 0);
		}

		/*
		 * If have fewer colors or symbols than the defaults for the
		 * backend, augment what was set with the default values.
		 */
		augment_colors(renderers[i].backend->default_colors,
			&renderers[i].colors, &renderers[i].ncolors);
		augment_colors(renderers[i].backend->default_labelcolors,
			&renderers[i].label_colors, &renderers[i].nlabcolors);
		augment_symbols(renderers[i].backend->default_symbols,
			&renderers[i].symbols, &renderers[i].nsym);

		if (renderers[i].ndigit == INT_MIN) {
			renderers[i].ndigit =
				renderers[i].backend->default_ndigit;
		}
		if (renderers[i].sign == UI_ENTRY_SIGN_DEFAULT) {
			renderers[i].sign =
				renderers[i].backend->default_sign;
		}
	}
	parser_destroy(p);
	return 0;
}


static void cleanup_parse_ui_entry_renderer(void)
{
	int i;

	for (i = 0; i < renderer_count; i++) {
		mem_free(renderers[i].symbols);
		mem_free(renderers[i].label_colors);
		mem_free(renderers[i].colors);
		string_free(renderers[i].comb_rend_nm);
		string_free(renderers[i].name);
	}
	mem_free(renderers);
	renderers = NULL;
	renderer_count = 0;
	renderer_allocated = 0;
}


struct file_parser ui_entry_renderer_parser = {
	"ui_entry_renderer",
	init_parse_ui_entry_renderer,
	run_parse_ui_entry_renderer,
	finish_parse_ui_entry_renderer,
	cleanup_parse_ui_entry_renderer
};
