/**
 * \file borg-formulas.c
 * \brief Prepare to perform an action while in a store
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2007-9 Andi Sidwell, Chris Carr, Ed Graham, Erik Osheim
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband License":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include "borg-formulas.h"

#ifdef ALLOW_BORG
#include "../obj-util.h"
#include "../player-spell.h"
#include "../z-type.h"

#include "borg-formulas-calc.h"
#include "borg-init.h"
#include "borg-io.h"
#include "borg-item-activation.h"
#include "borg-item-val.h"
#include "borg-magic.h"
#include "borg-trait.h"
#include "borg.h"

struct math_section {
    bool    is_calculation;
    int32_t calculation_index;
};

struct range {
    int32_t from;
    int32_t to;
};

enum value_type {
    VT_NONE = -1, /* for error */
    VT_RANGE_INDEX, /* the index into range processing */
    VT_TRAIT,
    VT_CONFIG,
    VT_ACTIVATION,
/* include the TV types */
#define TV(a, b) VT_##a,
#include "list-tvals.h"
#undef TV
    VT_MAX
};

/**
 * List of { valuetype, name } pairs.
 */
static const grouper value_type_names[] = {
    { VT_NONE, "error" },
    { VT_RANGE_INDEX, "range" },
    { VT_TRAIT, "trait" },
    { VT_CONFIG, "config" },
    { VT_ACTIVATION, "activation" },
#define TV(a, b) { VT_##a, b },
#include "list-tvals.h"
#undef TV
};

struct value {
    enum value_type type;
    int32_t         index;
};

struct borg_power {
    struct range        *range;
    struct math_section *reward;
    struct value        *value;
    struct math_section *condition;
};

struct borg_depth {
    int32_t              dlevel;
    char                *reason;
    struct math_section *condition;
};

struct borg_class_formula {
    /* array of power formulas */
    struct borg_array power;

    /* array of depth formulas */
    struct borg_array depth;
};

/* array of depth and power per class plus one for "any" */
struct borg_class_formula borg_class_formula[MAX_CLASSES + 1];

int borg_array_add(struct borg_array *a, void *item)
{
    if (!a->count) {
        a->max   = 10;
        a->items = mem_alloc(sizeof(void *) * a->max);
    }
    if (a->max < (a->count + 1)) {
        a->max += 10;
        a->items = mem_realloc(a->items, sizeof(void *) * a->max);
    }

    a->items[a->count] = item;

    return a->count++;
}

/*
 * get a class from a name
 */
static int borg_get_class(char *class_name)
{
    struct player_class *c;
    for (c = classes; c; c = c->next) {
        if (streq(c->name, class_name))
            return c->cidx;
    }
    return -1;
}

/*
 * get rid of whitespace
 *    NOTE: this modifies the initial string
 */
static char *borg_trim(char *line)
{
    // Trim leading space
    while (isspace((unsigned char)*line))
        line++;

    if (*line == 0) // All spaces?
        return line;

    // Trim trailing space
    char *end = line + strlen(line) - 1;
    while (end > line && isspace((unsigned char)*end))
        end--;

    // Write new null terminator character
    end[1] = '\0';

    return line;
}

/*
 * give an error for when a formula fails
 */
void borg_formula_error(
    char *section, char *full_line, char *section_label, char *error)
{
    borg_note("** borg formula failure ** ");
    borg_note(format("** error on line '%s'", full_line));
    borg_note(format("** error for section %s '%s'", section_label, section));
    if (error)
        borg_note(error);
    borg_note("** formulas disabled ** ");
}

/*
 * the line string should look like
 *    depth(nnn)
 * this should return the nnn as a number and -1 if it fails
 */
static int parse_depth(char *line, char *full_line)
{
    line = strchr(line, '(');
    if (!line)
        return -1;

    /* skip the "(" */
    line++;

    /* skip leading whitespace */
    while (isspace((unsigned char)*line))
        line++;

    char *end;
    long  l = strtol(line, &end, 10);
    if (end == line)
        return -1;

    /* skip whitespace in case the string is "depth( nn )" */
    while (isspace((unsigned char)*end))
        end++;

    /* make sure it ends ")" */
    if (end[0] != ')')
        return -1;

    return atoi(line++);
}

/*
 *  Just quick check to see if a string will parse as a number
 */
static bool is_all_number(char *line)
{
    while (isdigit((unsigned char)*line))
        line++;
    if (*line)
        return false;
    return true;
}

/*
 *  parse a formula but check first if it is just a number.
 */
static struct math_section *parse_formula_section(char *line, char *full_line)
{
    int   formula = -1;
    char *start   = line;

    /* skip leading whitespace */
    while (isspace((unsigned char)*line))
        line++;

    if (is_all_number(line)) {
        struct math_section *c = mem_alloc(sizeof(struct math_section));
        c->calculation_index   = atol(line);
        c->is_calculation      = false;
        return c;
    }

    formula = parse_calculation_line(line, full_line);
    if (formula == -1)
        return NULL;

    struct math_section *c = mem_alloc(sizeof(struct math_section));
    c->calculation_index   = formula;
    c->is_calculation      = true;

    return c;
}

/*
 * the line string should look like
 *    condition({formula})
 * this should allocate and return the condition pointer and null if it fails
 */
static struct math_section *parse_condition(char *line, char *full_line)
{
    char *start = line;
    line        = strchr(line, '(');
    if (!line) {
        borg_formula_error(start, full_line, "condition", NULL);
        return NULL;
    }

    /* skip the "(" */
    line++;

    /* remove the end ) */
    if (line[strlen(line) - 1] != ')') {
        borg_formula_error(start, full_line, "condition", "** no ending ')'");
        return NULL;
    }
    line[strlen(line) - 1] = 0;

    return parse_formula_section(line, full_line);
}

/*
 * the line string should look like
 *    reward({formula})
 * this should allocate and return the reward pointer and null if it fails
 */
static struct math_section *parse_reward(char *line, char *full_line)
{
    char *start = line;
    line        = strchr(line, '(');
    if (!line) {
        borg_formula_error(start, full_line, "reward", 0);
        return NULL;
    }

    /* skip the "(" */
    line++;

    /* remove the end ) */
    if (line[strlen(line) - 1] != ')') {
        borg_formula_error(start, full_line, "reward", "** no ending ')'");
        return NULL;
    }
    line[strlen(line) - 1] = 0;

    return parse_formula_section(line, full_line);
}

/*
 * the line string should look like
 *    range(n1, n2)
 * this should allocate and return the range pointer and null if it fails
 * (for now n1 and n2 must be numbers, perhaps can make them formulas later)
 */
static struct range *parse_range(char *line, char *full_line)
{
    char *start = line;
    char *val1str;
    char *val2str;

    line = strchr(line, '(');
    if (!line) {
        borg_formula_error(start, full_line, "range", 0);
        return NULL;
    }

    /* skip the "(" */
    line++;
    while (isspace((unsigned char)*line))
        line++;
    val1str = line;

    line    = strchr(line, ',');
    if (!line) {
        borg_formula_error(start, full_line, "range",
            "** range needs two numbers separated by a ','");
        return NULL;
    }
    /* remove the "," */
    *line = 0;
    line++;
    while (isspace((unsigned char)*line))
        line++;
    val2str = line;

    /* remove the end ) */
    if (line[strlen(line) - 1] != ')') {
        borg_formula_error(
            start, full_line, "range", "** first value is not a number ");
        return NULL;
    }
    line[strlen(line) - 1] = 0;

    if (!is_all_number(val1str)) {
        return NULL;
    }
    if (!is_all_number(val2str)) {
        borg_formula_error(
            start, full_line, "range", "** second value is not a number ");
        return NULL;
    }
    struct range *r = mem_alloc(sizeof(struct range));
    r->from         = atol(val1str);
    r->to           = atol(val2str);

    return r;
}

static enum value_type get_value_type(char *line)
{
    for (int i = 0; i < N_ELEMENTS(value_type_names); i++) {
        if (!my_stricmp(line, value_type_names[i].name)) {
            return value_type_names[i].tval;
        }
    }
    return VT_NONE;
}

/*
 *  convert the name in value(type, name) to an index
 */
static int get_value_index(enum value_type t, char *name, char *full_line)
{
    int i;
    switch (t) {
    case VT_ACTIVATION:
        i = borg_findact(name);
        if (i == -1)
            borg_formula_error(
                name, full_line, "value", "** unable to find activation");
        return i;

    case VT_TRAIT:
        for (i = 0; i < BI_MAX; i++) {
            if (!my_stricmp(prefix_pref[i], name))
                return i;
        }
        borg_formula_error(name, full_line, "value", "** unable to find trait");
        return -1;

    case VT_CONFIG:
        i = 0;
        while (borg_settings[i].setting_string) {
            /* +5 to get rid of borg_ */
            if (!my_stricmp(borg_settings[i].setting_string + 5, name))
                return i;
            i++;
        }
        borg_formula_error(
            name, full_line, "value", "** unable to find config");
        return -1;

    case VT_RANGE_INDEX:
        return 1;

    case VT_NONE:
    case VT_NULL:
    case VT_MAX:
        borg_formula_error(name, full_line, "value", "** invalid value type");
        return -1;

    /* lookup in the borg_has array */
    default: {
        int tval = t - VT_NULL;
        int sval = lookup_sval(tval, name);
        if (sval == -1) {
            borg_formula_error(name, full_line, "value",
                format("** value error for object type '%s' subtype '%s'",
                    value_type_names[t].name, name));
            return -1;
        }
        i = borg_lookup_kind(tval, sval);
        if (i == -1) {
            borg_formula_error(name, full_line, "value",
                format("** value error for object type '%s' subtype '%s'",
                    value_type_names[t].name, name));
        }
        return i;
    }
    }
}

/*
 * Turn a value pointer into a value that can be used in a formula
 */
int32_t calculate_from_value(struct value *value, int range_index)
{
    switch (value->type) {

    case VT_ACTIVATION:
        return borg.activation[value->index];

    case VT_TRAIT:
        return borg.trait[value->index];

    case VT_CONFIG:
        return borg_cfg[value->index];

    case VT_RANGE_INDEX:
        return range_index;

    default:
        return borg.has[value->index];
    }
}

/*
 * the line string should look like
 *    value(v1, v2)
 * this should allocate and return the value pointer and null if it fails
 * The possible values for v1 are listed in the value_type enum and
 *   associated array
 * The possible values for v2 depend on v1.
 */
struct value *parse_value(char *line, char *full_line)
{
    char *start = line;
    char *val1str;
    char *val2str;

    line = strchr(line, '(');
    if (!line) {
        borg_formula_error(start, full_line, "value", 0);
        return NULL;
    }

    /* skip the "(" and leading space */
    line++;
    while (isspace((unsigned char)*line))
        line++;
    val1str = line;

    line    = strchr(line, ',');
    if (!line) {
        borg_formula_error(start, full_line, "value",
            "** value needs two values separated by a ',' ");
        return NULL;
    }
    /* remove the "," and leading space*/
    *line = 0;
    line++;
    while (isspace((unsigned char)*line))
        line++;
    val2str = line;

    /* remove the end ) */
    if (line[strlen(line) - 1] != ')') {
        borg_formula_error(start, full_line, "value", "** no ending ')' ");
        return NULL;
    }
    line[strlen(line) - 1] = 0;

    enum type t            = get_value_type(val1str);
    if (t == VT_NONE) {
        borg_formula_error(start, full_line, "value",
            format("** value type (first parameter '%s') not found", val1str));
        return NULL;
    }

    int index = get_value_index(t, val2str, full_line);
    if (index == -1)
        return NULL;

    struct value *v = mem_alloc(sizeof(struct value));
    v->type         = t;
    v->index        = index;

    return v;
}

/*
 * get a section from a dynamic formula line.  Lines are
 *   name(xxx):name(xxx):name(xxx)
 * and this allocates and returns (in the value) one "name(xxx)"
 */
static bool get_section(char *line, char *name, char **value, char *full_line)
{
    char *start = line;
    if (prefix_i(line, name)) {
        char buf[1024];

        /* this value already exists */
        if (*value) {
            borg_formula_error(
                start, full_line, "value", "** section repeated");
            return false;
        }

        int   len;
        char *pos = line;
        line      = strchr(line, ':');
        if (line)
            len = line - pos;
        else
            len = strlen(pos);
        if (pos[len - 1] == ';')
            len--;
        strncpy(buf, pos, len);
        buf[len] = 0;
        *value   = string_make(buf);
        return true;
    }

    return false;
}

static void depth_free(struct borg_depth *depth)
{
    if (depth) {
        if (depth->condition)
            mem_free(depth->condition);
        if (depth->reason)
            mem_free(depth->reason);

        mem_free(depth);
    }
}

/*
 * depth lines appear as
 *   depth(nnn):condition(xxx)
 */
static bool parse_depth_line(char *line, int cclass, char *full_line)
{
    char *start       = line;
    bool  fail        = false;
    char *s_depth     = NULL;
    char *s_condition = NULL;

    while (*line) {
        if (get_section(line, "depth", &s_depth, full_line))
            line += strlen(s_depth);
        if (get_section(line, "condition", &s_condition, full_line))
            line += strlen(s_condition);

        line++;
    }

    if (!s_depth) {
        borg_formula_error(
            start, full_line, "value", "** depth section missing");
        fail = true;
    }
    if (!s_condition) {
        borg_formula_error(
            start, full_line, "value", "** condition section missing");
        fail = true;
    }

    struct borg_depth *d = 0;
    if (!fail)
        d = mem_zalloc(sizeof(struct borg_depth));

    if (s_depth) {
        if (!fail) {
            d->dlevel = parse_depth(s_depth, full_line);
            if (d->dlevel < 0) {
                borg_formula_error(start, full_line, "value",
                    "** depth section can't be parsed");
                fail = true;
            }
        }
        string_free(s_depth);
    }

    if (s_condition) {
        if (!fail) {
            d->condition = parse_condition(s_condition, full_line);
            if (d->condition == NULL)
                fail = true;
            else
                d->reason = s_condition;
        }
    }

    /* add to the depth array */
    if (!fail)
        borg_array_add(&borg_class_formula[cclass].depth, d);

    if (fail)
        depth_free(d);

    return fail;
}

/*
 * power lines appear as
 *   value(a, b):condition(xxx):range(n1, n2):reward(xxx)
 * value and range are optional
 */
static bool parse_power_line(char *line, int cclass, char *full_line)
{
    char *start       = line;
    bool  fail        = false;
    char *s_range     = NULL;
    char *s_reward    = NULL;
    char *s_value     = NULL;
    char *s_condition = NULL;

    while (*line) {
        if (get_section(line, "range", &s_range, full_line))
            line += strlen(s_range);
        if (get_section(line, "reward", &s_reward, full_line))
            line += strlen(s_reward);
        if (get_section(line, "value", &s_value, full_line))
            line += strlen(s_value);
        if (get_section(line, "condition", &s_condition, full_line))
            line += strlen(s_condition);

        line++;
    }

    if (s_range && !s_value) {
        borg_formula_error(start, full_line, "power",
            "** if range() is specified value() must also be specified");
        fail = true;
    }

    struct borg_power *p = NULL;
    if (!fail)
        p = mem_zalloc(sizeof(struct borg_power));

    if (s_range) {
        if (!fail) {
            p->range = parse_range(s_range, full_line);
            if (p->range == NULL)
                fail = true;
        }

        string_free(s_range);
    }

    if (s_reward) {
        if (!fail) {
            p->reward = parse_reward(s_reward, full_line);
            if (p->reward == NULL)
                fail = true;
        }

        string_free(s_reward);
    }

    if (s_value) {
        if (!fail) {
            p->value = parse_value(s_value, full_line);
            if (p->value == NULL)
                fail = true;
        }
        string_free(s_value);
    }

    if (s_condition) {
        if (!fail) {
            p->condition = parse_condition(s_condition, full_line);
            if (p->condition == NULL)
                fail = true;
        }

        string_free(s_condition);
    }

    if (!fail)
        borg_array_add(&borg_class_formula[cclass].power, p);

    if (fail && p)
        mem_free(p);

    return fail;
}

static bool borg_read_formula_class(ang_file *fp, int cclass)
{
    enum e_section { SEC_NONE, SEC_POWER, SEC_DEPTH };
    char           buf[1024];
    bool           formula_fail = false;
    enum e_section section      = SEC_NONE;
    bool           skip_open    = true;

    /* Parse the class section of formula section the file */
    while (file_getl(fp, buf, sizeof(buf) - 1)) {

        char *line = borg_trim(buf);

        /* Skip comments and blank lines */
        if (!line[0] || (line[0] == '#'))
            continue;

        /* sections are bounded in [] */
        if (line[0] == '[') {
            if (!skip_open)
                formula_fail = true;

            skip_open = false;
            continue;
        }
        if (line[0] == ']') {
            if (section == SEC_NONE)
                return formula_fail;
            section = SEC_NONE;
            continue;
        }

        /* if we failed anywhere */
        if (formula_fail)
            continue;

        if (prefix_i(line, "Power")) {
            if (section != SEC_NONE)
                formula_fail = true;

            section   = SEC_POWER;
            skip_open = true;
            continue;
        }
        if (prefix_i(line, "Depth Requirement")) {
            if (section != SEC_NONE)
                formula_fail = true;

            section   = SEC_DEPTH;
            skip_open = true;
            continue;
        }

        /* lines must end in ; */
        if (line[strlen(line) - 1] != ';') {
            if (section == SEC_DEPTH)
                borg_formula_error(
                    buf, buf, "depth", "** line must end in a ;");
            else if (section == SEC_POWER)
                borg_formula_error(
                    buf, buf, "power", "** line must end in a ;");
            else
                borg_formula_error(buf, buf, "base", "** line must end in a ;");
            formula_fail = true;
        } else {
            if (section == SEC_DEPTH)
                formula_fail = parse_depth_line(line, cclass, buf);
            if (section == SEC_POWER)
                formula_fail = parse_power_line(line, cclass, buf);
            if (section == SEC_NONE) {
                borg_formula_error(buf, buf, "base",
                    "** line must be part of a Power or Depth section");
                formula_fail = true;
            }
        }
        continue;
    }

    return formula_fail;
}

static bool check_condition(struct math_section *condition)
{
    /* not sure why you would ever do this */
    if (!condition->is_calculation)
        return condition->calculation_index != 0;

    return borg_calculate_dynamic(condition->calculation_index, 0);
}

static int32_t calc_power(struct borg_power *p)
{
    int32_t total = 0;
    if (p->condition)
        if (!check_condition(p->condition))
            return 0;

    if (p->range) {
        int i;

        /* loop from the from to the to or count, whichever is lower */
        int count = calculate_from_value(p->value, 0);
        int end   = MIN(count, p->range->to);
        for (i = p->range->from; i <= end; i++) {
            if (p->reward->is_calculation)
                total
                    += borg_calculate_dynamic(p->reward->calculation_index, i);
            else
                total += p->reward->calculation_index;
        }
        return total;
    }

    if (p->reward->is_calculation)
        total += borg_calculate_dynamic(p->reward->calculation_index, 0);
    else
        total += p->reward->calculation_index;
    return total;
}

/*
 * Calculate the basic "power"
 */
int32_t borg_power_dynamic(void)
{
    int     i;
    int32_t total = 0;

    /* MAX is used for "any" */
    struct borg_class_formula *bcc = &borg_class_formula[MAX_CLASSES];
    for (i = 0; i < bcc->power.count; i++) {
        total += calc_power(bcc->power.items[i]);
    }
    bcc = &borg_class_formula[borg.trait[BI_CLASS]];
    for (i = 0; i < bcc->power.count; i++) {
        total += calc_power(bcc->power.items[i]);
    }

    /* HUGE HACK need some extra stuff that is hard to make dynamic*/
    if (borg_spell_stat() > 0)
        total += (100 - spell_chance(0)) * 100;

    /* should try to get min fail to 0 */
    if (player_has(player, PF_ZERO_FAIL)) {
        /* other fail rates */
        if (spell_chance(0) < 1)
            total += 30000L;
    }

    /*** Penalize armor weight ***/
    if (borg.stat_ind[STAT_STR] < 15) {
        if (borg_items[INVEN_BODY].weight > 200)
            total -= (borg_items[INVEN_BODY].weight - 200) * 15;
        if (borg_items[INVEN_HEAD].weight > 30)
            total -= 250;
        if (borg_items[INVEN_ARM].weight > 10)
            total -= 250;
        if (borg_items[INVEN_FEET].weight > 50)
            total -= 250;
    }

    /* Compute the total armor weight */
    int cur_wgt = borg_items[INVEN_BODY].weight;
    cur_wgt += borg_items[INVEN_HEAD].weight;
    cur_wgt += borg_items[INVEN_ARM].weight;
    cur_wgt += borg_items[INVEN_OUTER].weight;
    cur_wgt += borg_items[INVEN_HANDS].weight;
    cur_wgt += borg_items[INVEN_FEET].weight;

    /* Determine the weight allowance */
    int max_wgt = player->class->magic.spell_weight;

    /* Hack -- heavy armor hurts magic */
    if (player->class->magic.total_spells && ((cur_wgt - max_wgt) / 10) > 0) {
        /* max sp must be calculated in case it changed with the armor */
        int max_sp = borg.trait[BI_SP_ADJ] / 100 + 1;
        max_sp -= ((cur_wgt - max_wgt) / 10);
        /* Mega-Hack -- Penalize heavy armor which hurts mana */
        if (max_sp >= 300 && max_sp <= 350)
            total -= (((cur_wgt - max_wgt) / 10) * 400L);
        if (max_sp >= 200 && max_sp <= 299)
            total -= (((cur_wgt - max_wgt) / 10) * 800L);
        if (max_sp >= 100 && max_sp <= 199)
            total -= (((cur_wgt - max_wgt) / 10) * 1600L);
        if (max_sp >= 1 && max_sp <= 99)
            total -= (((cur_wgt - max_wgt) / 10) * 3200L);
    }
    /* END MAJOR HACK */

    return total;
}

/*
 * Determine what level the borg is prepared to dive to.
 */
const char *borg_prepared_dynamic(int depth)
{
    int i;

    /* MAX is used for "any" */
    struct borg_class_formula *bcc = &borg_class_formula[MAX_CLASSES];
    for (i = 0; i < bcc->depth.count; i++) {
        struct borg_depth *d = bcc->depth.items[i];
        if (d->dlevel > depth)
            break;
        if (!check_condition(d->condition))
            return d->reason;
    }

    bcc = &borg_class_formula[borg.trait[BI_CLASS]];
    for (i = 0; i < bcc->depth.count; i++) {
        struct borg_depth *d = bcc->depth.items[i];
        if (d->dlevel > depth)
            break;
        if (!check_condition(d->condition))
            return d->reason;
    }
    return NULL;
}

/*
 * read the formula section of the borg.txt file
 */
bool borg_load_formulas(ang_file *fp)
{
    char buf[1024];
    bool inClass = false;
    int  cclass;
    bool formulas_off = false;

    for (cclass = 0; cclass <= MAX_CLASSES; cclass++) {
        borg_class_formula[cclass].depth.count = 0;
        borg_class_formula[cclass].power.count = 0;
    }

    /* read but don't process formulas if we aren't using them */
    if (!borg_cfg[BORG_USES_DYNAMIC_CALCS])
        formulas_off = true;

    /* Parse the formula section of the file */
    while (file_getl(fp, buf, sizeof(buf) - 1)) {

        /* Skip comments and blank lines */
        if (!buf[0] || (buf[0] == '#'))
            continue;

        if (prefix_i(buf, "[END FORMULA SECTION]")) {
            if (formulas_off && borg_cfg[BORG_USES_DYNAMIC_CALCS])
                borg_cfg[BORG_USES_DYNAMIC_CALCS] = false;

            return formulas_off;
        }

        /* if we failed anywhere */
        if (formulas_off)
            continue;

        /* formula section is divided into classes */
        cclass = borg_get_class(buf);

        /* if we can't find that class it might be "any" */
        /* if it isn't, error */
        if (cclass == -1) {
            if (!prefix_i(buf, "Any")) {
                formulas_off = true;
                borg_formula_error(buf, buf, "base", "class not found");
                continue;
            }
            cclass = MAX_CLASSES;
        }

        formulas_off = borg_read_formula_class(fp, cclass);
    }
    if (formulas_off && borg_cfg[BORG_USES_DYNAMIC_CALCS])
        borg_cfg[BORG_USES_DYNAMIC_CALCS] = false;

    return formulas_off;
}

static void power_free(struct borg_power *power)
{
    if (power->condition)
        mem_free(power->condition);
    if (power->range)
        mem_free(power->range);
    if (power->reward)
        mem_free(power->reward);
    if (power->value)
        mem_free(power->value);
    mem_free(power);
}

void borg_free_formulas(void)
{
    int i, j;
    for (i = 0; i <= MAX_CLASSES; i++) {
        for (j = 0; j < borg_class_formula[i].depth.count; j++)
            depth_free(borg_class_formula[i].depth.items[j]);

        for (j = 0; j < borg_class_formula[i].power.count; j++)
            power_free(borg_class_formula[i].power.items[j]);
    }

    calculations_free();
}

#endif
