/* Parsing functions for monster_base.txt */

#include "externs.h"
#include "monster/mon-power.h"
#include "monster/mon-spell.h"
#include "monster/monster.h"
#include "parser.h"
#include "z-util.h"
#include "z-virt.h"

static enum parser_error parse_rb_n(struct parser *p) {
	struct monster_base *h = parser_priv(p);
	struct monster_base *rb = mem_zalloc(sizeof *rb);
	rb->next = h;
	rb->name = string_make(parser_getstr(p, "name"));
	parser_setpriv(p, rb);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_rb_g(struct parser *p) {
	struct monster_base *rb = parser_priv(p);

	if (!rb)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	rb->d_char = parser_getchar(p, "glyph");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_rb_m(struct parser *p) {
	struct monster_base *rb = parser_priv(p);
	int pain_idx;

	if (!rb)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	pain_idx = parser_getuint(p, "pain");
	if (pain_idx >= z_info->mp_max)
		/* XXX need a real error code for this */
		return PARSE_ERROR_GENERIC;

	rb->pain = &pain_messages[pain_idx];

	return PARSE_ERROR_NONE;
}

const char *r_info_flags[] =
{
	#define RF(a, b) #a,
	#include "monster/list-mon-flags.h"
	#undef RF
	NULL
};

static enum parser_error parse_rb_f(struct parser *p) {
	struct monster_base *rb = parser_priv(p);
	char *flags;
	char *s;

	if (!rb)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(rb->flags, RF_SIZE, r_info_flags, s)) {
			mem_free(flags);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}

	mem_free(flags);
	return PARSE_ERROR_NONE;
}

const char *r_info_spell_flags[] =
{
	#define RSF(a, b, c, d, e, f, g, h, i, j, k, l, m) #a,
	#include "monster/list-mon-spells.h"
	#undef RSF
	NULL
};

static enum parser_error parse_rb_s(struct parser *p) {
	struct monster_base *rb = parser_priv(p);
	char *flags;
	char *s;

	if (!rb)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "spells"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "spells"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(rb->spell_flags, RSF_SIZE, r_info_spell_flags, s)) {
			mem_free(flags);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}

	mem_free(flags);
	return PARSE_ERROR_NONE;
}


static enum parser_error parse_rb_d(struct parser *p) {
	struct monster_base *rb = parser_priv(p);

	if (!rb)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	rb->text = string_append(rb->text, parser_getstr(p, "desc"));
	return PARSE_ERROR_NONE;
}


struct parser *init_parse_rb(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);

	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "N str name", parse_rb_n);
	parser_reg(p, "G char glyph", parse_rb_g);
	parser_reg(p, "M uint pain", parse_rb_m);
	parser_reg(p, "F ?str flags", parse_rb_f);
	parser_reg(p, "S ?str spells", parse_rb_s);
	parser_reg(p, "D str desc", parse_rb_d);
	return p;
}

static errr run_parse_rb(struct parser *p) {
	return parse_file(p, "monster_base");
}

static errr finish_parse_rb(struct parser *p) {
	rb_info = parser_priv(p);
	parser_destroy(p);
	return 0;
}

static void cleanup_rb(void)
{
	struct monster_base *rb, *next;

	rb = rb_info;
	while (rb) {
		next = rb->next;
		string_free(rb->text);
		string_free(rb->name);
		mem_free(rb);
		rb = next;
	}
}

struct file_parser rb_parser = {
	"monster_base",
	init_parse_rb,
	run_parse_rb,
	finish_parse_rb,
	cleanup_rb
};


/* Parsing functions for monster.txt */
static enum parser_error parse_r_n(struct parser *p) {
	struct monster_race *h = parser_priv(p);
	struct monster_race *r = mem_zalloc(sizeof *r);
	r->next = h;
	r->ridx = parser_getuint(p, "index");
	r->name = string_make(parser_getstr(p, "name"));
	parser_setpriv(p, r);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_r_t(struct parser *p) {
	struct monster_race *r = parser_priv(p);

	r->base = lookup_monster_base(parser_getsym(p, "base"));
	if (r->base == NULL)
		/* Todo: make new error for this */
		return PARSE_ERROR_UNRECOGNISED_TVAL;

	/* The template sets the default display character */
	r->d_char = r->base->d_char;

	/* Give the monster its default flags */
	rf_union(r->flags, r->base->flags);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_r_g(struct parser *p) {
	struct monster_race *r = parser_priv(p);

	/* If the display character is specified, it overrides any template */
	r->d_char = parser_getchar(p, "glyph");

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_r_c(struct parser *p) {
	struct monster_race *r = parser_priv(p);
	const char *color;
	int attr;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
		color = parser_getsym(p, "color");
	if (strlen(color) > 1)
		attr = color_text_to_attr(color);
	else
		attr = color_char_to_attr(color[0]);
	if (attr < 0)
		return PARSE_ERROR_INVALID_COLOR;
	r->d_attr = attr;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_r_i(struct parser *p) {
	struct monster_race *r = parser_priv(p);

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->speed = parser_getint(p, "speed");
	r->avg_hp = parser_getint(p, "hp");
	r->aaf = parser_getint(p, "aaf");
	r->ac = parser_getint(p, "ac");
	r->sleep = parser_getint(p, "sleep");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_r_w(struct parser *p) {
	struct monster_race *r = parser_priv(p);

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->level = parser_getint(p, "level");
	r->rarity = parser_getint(p, "rarity");
	r->power = parser_getint(p, "power");
	r->mexp = parser_getint(p, "mexp");
	return PARSE_ERROR_NONE;
}

static const char *r_info_blow_method[] =
{
	#define RBM(a, b) #a,
	#include "monster/list-blow-methods.h"
	#undef RBM
	NULL
};

static int find_blow_method(const char *name) {
	int i;
	for (i = 0; r_info_blow_method[i]; i++)
		if (streq(name, r_info_blow_method[i]))
			break;
	return i;
}

static const char *r_info_blow_effect[] =
{
	#define RBE(a, b) #a,
	#include "monster/list-blow-effects.h"
	#undef RBE
	NULL
};

static int find_blow_effect(const char *name) {
	int i;
	for (i = 0; r_info_blow_effect[i]; i++)
		if (streq(name, r_info_blow_effect[i]))
			break;
	return i;
}

static enum parser_error parse_r_b(struct parser *p) {
	struct monster_race *r = parser_priv(p);
	int i;
	struct random dam;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	for (i = 0; i < MONSTER_BLOW_MAX; i++)
		if (!r->blow[i].method)
			break;
	if (i == MONSTER_BLOW_MAX)
		return PARSE_ERROR_TOO_MANY_ENTRIES;
	r->blow[i].method = find_blow_method(parser_getsym(p, "method"));
	if (!r_info_blow_method[r->blow[i].method])
		return PARSE_ERROR_UNRECOGNISED_BLOW;
	if (parser_hasval(p, "effect")) {
		r->blow[i].effect = find_blow_effect(parser_getsym(p, "effect"));
		if (!r_info_blow_effect[r->blow[i].effect])
			return PARSE_ERROR_INVALID_EFFECT;
	}
	if (parser_hasval(p, "damage")) {
		dam = parser_getrand(p, "damage");
		r->blow[i].d_dice = dam.dice;
		r->blow[i].d_side = dam.sides;
	}


	return PARSE_ERROR_NONE;
}

static enum parser_error parse_r_f(struct parser *p) {
	struct monster_race *r = parser_priv(p);
	char *flags;
	char *s;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(r->flags, RF_SIZE, r_info_flags, s)) {
			mem_free(flags);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}

	mem_free(flags);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_r_d(struct parser *p) {
	struct monster_race *r = parser_priv(p);

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->text = string_append(r->text, parser_getstr(p, "desc"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_r_s(struct parser *p) {
	struct monster_race *r = parser_priv(p);
	char *flags;
	char *s;
	int pct;
	int ret = PARSE_ERROR_NONE;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	flags = string_make(parser_getstr(p, "spells"));
	s = strtok(flags, " |");
	while (s) {
		if (1 == sscanf(s, "1_IN_%d", &pct)) {
			if (pct < 1 || pct > 100) {
				ret = PARSE_ERROR_INVALID_SPELL_FREQ;
				break;
			}
			r->freq_spell = 100 / pct;
			r->freq_innate = r->freq_spell;
		} else {
			if (grab_flag(r->spell_flags, RSF_SIZE, r_info_spell_flags, s)) {
				ret = PARSE_ERROR_INVALID_FLAG;
				break;
			}
		}
		s = strtok(NULL, " |");
	}

	/* Add the "base monster" flags to the monster */
	if (r->base)
		rsf_union(r->spell_flags, r->base->spell_flags);

	mem_free(flags);
	return ret;
}

static enum parser_error parse_r_drop(struct parser *p) {
	struct monster_race *r = parser_priv(p);
	struct monster_drop *d;
	int tval, sval;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	tval = tval_find_idx(parser_getsym(p, "tval"));
	if (tval < 0)
		return PARSE_ERROR_UNRECOGNISED_TVAL;
	sval = lookup_sval(tval, parser_getsym(p, "sval"));
	if (sval < 0)
		return PARSE_ERROR_UNRECOGNISED_SVAL;

	if (parser_getuint(p, "min") > 99 || parser_getuint(p, "max") > 99)
		return PARSE_ERROR_INVALID_ITEM_NUMBER;

	d = mem_zalloc(sizeof *d);
	d->kind = objkind_get(tval, sval);
	d->percent_chance = parser_getuint(p, "chance");
	d->min = parser_getuint(p, "min");
	d->max = parser_getuint(p, "max");
	d->next = r->drops;
	r->drops = d;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_r_drop_artifact(struct parser *p) {
	struct monster_race *r = parser_priv(p);
	struct monster_drop *d;
	int art;
	struct artifact *a;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	art = lookup_artifact_name(parser_getstr(p, "name"));
	if (art < 0)
		return PARSE_ERROR_GENERIC;
	a = &a_info[art];

	d = mem_zalloc(sizeof *d);
	d->artifact = a;
	d->min = 1;
	d->max = 1;
	d->percent_chance = 100;
	d->next = r->drops;
	r->drops = d;
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_r(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);

	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "N uint index str name", parse_r_n);
	parser_reg(p, "T sym base", parse_r_t);
	parser_reg(p, "G char glyph", parse_r_g);
	parser_reg(p, "C sym color", parse_r_c);
	parser_reg(p, "I int speed int hp int aaf int ac int sleep", parse_r_i);
	parser_reg(p, "W int level int rarity int power int mexp", parse_r_w);
	parser_reg(p, "B sym method ?sym effect ?rand damage", parse_r_b);
	parser_reg(p, "F ?str flags", parse_r_f);
	parser_reg(p, "D str desc", parse_r_d);
	parser_reg(p, "S str spells", parse_r_s);
	parser_reg(p, "drop sym tval sym sval uint chance uint min uint max", parse_r_drop);
	parser_reg(p, "drop-artifact str name", parse_r_drop_artifact);
	return p;
}

static errr run_parse_r(struct parser *p) {
	return parse_file(p, "monster");
}

static errr finish_parse_r(struct parser *p) {
	struct monster_race *r, *n;

	r_info = mem_zalloc(sizeof(*r) * z_info->r_max);
	for (r = parser_priv(p); r; r = r->next) {
		if (r->ridx >= z_info->r_max)
			continue;
		memcpy(&r_info[r->ridx], r, sizeof(*r));
	}
	eval_r_power(r_info);

	r = parser_priv(p);
	while (r) {
		n = r->next;
		mem_free(r);
		r = n;
	}

	parser_destroy(p);
	return 0;
}

static void cleanup_r(void)
{
	int ridx;

	for (ridx = 0; ridx < z_info->r_max; ridx++) {
		struct monster_race *r = &r_info[ridx];
		struct monster_drop *d, *dn;

		d = r->drops;
		while (d) {
			dn = d->next;
			mem_free(d);
			d = dn;
		}
		string_free(r->text);
		string_free(r->name);
	}

	mem_free(r_info);
}

struct file_parser r_parser = {
	"monster",
	init_parse_r,
	run_parse_r,
	finish_parse_r,
	cleanup_r
};

