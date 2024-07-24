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
#include "../cmd-core.h"
#include "../obj-util.h"
#include "../player-spell.h"
#include "../z-type.h"

#include "borg-formulas-calc.h"
#include "borg-init.h"
#include "borg-io.h"
#include "borg-item-activation.h"
#include "borg-item-analyze.h"
#include "borg-item-val.h"
#include "borg-magic.h"
#include "borg-trait.h"
#include "borg-util.h"
#include "borg.h"

struct math_section {
    bool    is_calculation;
    int32_t calculation_index;
};

struct range_sec {
    int32_t from;
    int32_t to;
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
    { VT_CLASS, "class" },
#define TV(a, b) { VT_##a, b },
#include "list-tvals.h"
#undef TV
};

struct borg_power_line {
    struct range_sec    *range;
    struct math_section *reward;
    struct value_sec    *value;
    struct math_section *condition;
};

struct borg_depth_line {
    int32_t              dlevel;
    char                *reason;
    struct math_section *condition;
};

struct borg_formula_arrays {
    /* array of power formulas */
    struct borg_array power;

    /* array of depth formulas */
    struct borg_array depth;

    /* array of depth formulas for restocking */
    struct borg_array restock;
};

/* array of depth and power per class plus one for "any" */
struct borg_formula_arrays borg_formulas;

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
static int borg_get_class(const char *class_name)
{
    struct player_class *c;
    for (c = classes; c; c = c->next) {
        if (!my_stricmp(c->name, class_name))
            return c->cidx;
    }
    return -1;
}

/*
 * give an error for when a formula fails
 */
void borg_formula_error(const char *section, const char *full_line,
    const char *section_label, const char *error)
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
static int parse_depth(char *line, const char *full_line)
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

    return l;
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
static struct math_section *parse_formula_section(
    char *line, const char *full_line)
{
    int formula = -1;

    /* skip leading whitespace */
    while (isspace((unsigned char)*line))
        line++;

    if (is_all_number(line)) {
        struct math_section *c = mem_alloc(sizeof(struct math_section));
        c->calculation_index   = atol(line);
        c->is_calculation      = false;
        return c;
    }

    /* parse the formula so it can be calculated from */
    /* the formula is stored in the "formulas" array and */
    /* the index into the array is returned */
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
static struct math_section *parse_condition(char *line, const char *full_line)
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
static struct math_section *parse_reward(char *line, const char *full_line)
{
    char *start = line;
    line        = strchr(line, '(');
    if (!line) {
        borg_formula_error(start, full_line, "reward", "** no starting '('");
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
static struct range_sec *parse_range(char *line, const char *full_line)
{
    int   from, to;
    char *start = line;
    char *val1str;
    char *val2str;

    line = strchr(line, '(');
    if (!line) {
        borg_formula_error(start, full_line, "range", "** no starting '('");
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
            start, full_line, "range", "** first value is not a number");
        return NULL;
    }
    line[strlen(line) - 1] = 0;

    if (!is_all_number(val1str)) {
        return NULL;
    }
    if (!is_all_number(val2str)) {
        borg_formula_error(
            start, full_line, "range", "** second value is not a number");
        return NULL;
    }

    from = atol(val1str);
    to   = atol(val2str);
    if (from < 1) {
        borg_formula_error(
            start, full_line, "range", "** first value less than one");
        return NULL;
    }
    if (to < 0) {
        borg_formula_error(
            start, full_line, "range", "** second value less than one");
        return NULL;
    }
    if (to < from) {
        borg_formula_error(
            start, full_line, "range", "** second value less first value");
        return NULL;
    }

    struct range_sec *r = mem_alloc(sizeof(struct range_sec));
    r->from             = from;
    r->to               = to;

    return r;
}
/*
 *  Find the value type from the array matching names with enums
 */
static enum value_type get_value_type(char *value_type_name)
{
    for (int i = 0; i < (int)N_ELEMENTS(value_type_names); i++) {
        if (!my_stricmp(value_type_name, value_type_names[i].name)) {
            return value_type_names[i].tval;
        }
    }
    return VT_NONE;
}

/*
 * Convert the name in value(type, name) to an index.
 *  a later function (calculate_from_value) can take the index and type
 *  and turn it into a number.
 */
static int get_value_index(
    enum value_type t, const char *name, const char *full_line)
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

    case VT_CLASS:
        i = borg_get_class(name);
        if (i == -1)
            borg_formula_error(name, full_line, "value", "** invalid class");
        return i;

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
 * Turn a value pointer into a value (number) that can be used in a formula
 */
int32_t calculate_from_value(struct value_sec *value, int range_index)
{
    switch (value->type) {

    case VT_ACTIVATION:
        return borg.activation[value->index];

    case VT_TRAIT:
        return borg.trait[value->index];

    case VT_CONFIG:
        return borg_cfg[value->index];

    case VT_CLASS:
        return borg.trait[BI_CLASS] == value->index;

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
struct value_sec *parse_value(char *line, const char *full_line)
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

    enum value_type t      = get_value_type(val1str);
    if (t == VT_NONE) {
        borg_formula_error(start, full_line, "value",
            format("** value type (first parameter '%s') not found", val1str));
        return NULL;
    }

    int index = get_value_index(t, val2str, full_line);
    if (index == -1)
        return NULL;

    struct value_sec *v = mem_alloc(sizeof(struct value_sec));
    v->type             = t;
    v->index            = index;

    return v;
}

/*
 * get a section from a dynamic formula line.  Lines are
 *   name(xxx):name(xxx):name(xxx);
 * and this allocates and returns (in the value) one "name(xxx)"
 */
static bool get_section(
    char *line, const char *name, char **value, const char *full_line)
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

/*
 * Deallocate any memory associated with a depth line
 */
static void depth_free(struct borg_depth_line *depth)
{
    if (depth) {
        if (depth->condition) {
            mem_free(depth->condition);
            depth->condition = NULL;
        }

        if (depth->reason) {
            string_free(depth->reason);
            depth->reason = NULL;
        }

        mem_free(depth);
        depth = NULL;
    }
}

/*
 * depth lines appear as
 *   depth(nnn):condition(xxx)
 */
static bool parse_depth_line(bool restock, char *line, const char *full_line)
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

        if (*line && !isspace((unsigned char)*line) && line[0] != ':'
            && line[0] != ';') {
            char *err = string_make(
                format("** unparsed information '%s' in depth line", line));
            borg_formula_error(start, full_line, "depth", err);
            string_free(err);
            fail = true;
            break;
        }

        line++;
    }

    if (!fail && !s_depth) {
        borg_formula_error(
            start, full_line, "value", "** depth section missing");
        fail = true;
    }
    if (!fail && !s_condition) {
        borg_formula_error(
            start, full_line, "value", "** condition section missing");
        fail = true;
    }

    struct borg_depth_line *d = 0;
    if (!fail)
        d = mem_zalloc(sizeof(struct borg_depth_line));

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
            d->reason    = s_condition;
            if (d->condition == NULL)
                fail = true;
        }
    }

    /* add to the depth array */
    if (!fail) {
        if (restock)
            borg_array_add(&borg_formulas.restock, d);
        else
            borg_array_add(&borg_formulas.depth, d);
    }
    if (fail)
        depth_free(d);

    return fail;
}

/*
 * power lines appear as
 *   value(a, b):condition(xxx):range(n1, n2):reward(xxx)
 * value and range are optional
 */
static bool parse_power_line(char *line, const char *full_line)
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

        if (*line &&!isspace((unsigned char)*line) && line[0] != ':' && line[0] != ';') {
            char *err = string_make(
                format("** unparsed information '%s' in power line", line));
            borg_formula_error(start, full_line, "power", err);
            string_free(err);
            fail = true;
            break;
        }

        line++;
    }

    if (!fail && s_range && !s_value) {
        borg_formula_error(start, full_line, "power",
            "** if range() is specified value() must also be specified");
        fail = true;
    }

    if (!fail && !s_reward) {
        borg_formula_error(start, full_line, "power",
            "** reward() must be specified specified");
        fail = true;
    }

    struct borg_power_line *p = NULL;
    if (!fail)
        p = mem_zalloc(sizeof(struct borg_power_line));

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
        borg_array_add(&borg_formulas.power, p);

    if (fail && p) {
        mem_free(p);
        p = NULL;
    }

    return fail;
}

static bool check_condition(struct math_section *condition, int range)
{
    /* not sure why you would ever do this */
    if (!condition->is_calculation)
        return condition->calculation_index != 0;

    return borg_calculate_dynamic(condition->calculation_index, range);
}

/*
 * get a single value from a power line
 */
static int32_t calc_power(struct borg_power_line *p)
{
    int32_t total = 0;
    if (p->condition)
        if (!check_condition(p->condition, 0))
            return 0;

    if (p->range) {
        int i;

        /* loop from the "from" to the count or the "to", whichever is lower */
        int end = MIN(calculate_from_value(p->value, 0), p->range->to);
        for (i = p->range->from; i <= end; i++) {
            if (p->reward->is_calculation)
                total += borg_calculate_dynamic(
                    p->reward->calculation_index, i - 1);
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
    for (i = 0; i < borg_formulas.power.count; i++) {
        total += calc_power(borg_formulas.power.items[i]);
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

    /*
     * Hack-- Reward the borg for carrying a NON-ID items that have random
     * powers
     */
    if (borg_items[INVEN_OUTER].iqty) {
        borg_item *item = &borg_items[INVEN_OUTER];
        if (((borg_ego_has_random_power(&e_info[item->ego_idx])
                && !item->ident)))
            total += 999999L;
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
    if (borg_can_cast() && ((cur_wgt - max_wgt) / 10) > 0) {
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

    /* SECOND MAJOR HACK for inventory */

    /* Reward carrying a shovel if low level */
    if (borg.trait[BI_MAXDEPTH] <= 40 && borg.trait[BI_MAXDEPTH] >= 25
        && borg.trait[BI_GOLD] < 100000
        && borg_items[INVEN_WIELD].tval != TV_DIGGING
        && borg.trait[BI_ADIGGER] == 1)
        total += 5000L;

    /*** Hack -- books ***/
    /*   Reward books    */
    for (int book = 0; book < 9; book++) {
        /* No copies */
        if (!borg.amt_book[book])
            continue;

        /* The "hard" books */
        if (player->class->magic.books[book].dungeon) {
            int what;

            /* Scan the spells */
            for (what = 0; what < 9; what++) {
                borg_magic *as = borg_get_spell_entry(book, what);
                if (!as)
                    break;

                /* Track minimum level */
                if (as->level > borg.trait[BI_MAXCLEVEL])
                    continue;

                /* Track Mana req. */
                if (as->power > borg.trait[BI_MAXSP])
                    continue;

                /* Reward the book based on the spells I can cast */
                total += 15000L;
            }
        }

        /* The "easy" books */
        else {
            int what, when = 99;

            /* Scan the spells */
            for (what = 0; what < 9; what++) {
                borg_magic *as = borg_get_spell_entry(book, what);
                if (!as)
                    break;

                /* Track minimum level */
                if (as->level < when)
                    when = as->level;

                /* Track Mana req. */
                /* if (as->power < mana) mana = as->power; */
            }

            /* Hack -- Ignore "difficult" normal books */
            if ((when > 5) && (when >= borg.trait[BI_MAXCLEVEL] + 2))
                continue;
            /* if (mana > borg.trait[BI_MAXSP]) continue; */

            /* Reward the book */
            int k = 0;
            for (; k < 1 && k < borg.amt_book[book]; k++)
                total += 500000L;
            if (borg.trait[BI_STR] > 5)
                for (; k < 2 && k < borg.amt_book[book]; k++)
                    total += 10000L;
        }
    }

    /*  Hack -- Apply "encumbrance" from weight */

    /* XXX XXX XXX Apply "encumbrance" from weight */
    if (borg.trait[BI_WEIGHT] > borg.trait[BI_CARRY] / 2) {
        /* *HACK*  when testing items, the borg puts them in the last empty
         */
        /* slot so this is POSSIBLY just a test item */
        borg_item *item = NULL;
        for (i = PACK_SLOTS; i >= 0; i--) {
            if (borg_items[i].iqty) {
                item = &borg_items[i];
                break;
            }
        }

        /* Some items will be used immediately and should not contribute to
         * encumbrance */
        if (item && item->iqty
            && ((item->tval == TV_SCROLL
                    && ((item->sval == sv_scroll_enchant_armor
                            && borg.trait[BI_AENCH_ARM] < 1000
                            && borg.trait[BI_NEED_ENCHANT_TO_A])
                        || (item->sval == sv_scroll_enchant_weapon_to_hit
                            && borg.trait[BI_AENCH_TOH] < 1000
                            && borg.trait[BI_NEED_ENCHANT_TO_H])
                        || (item->sval == sv_scroll_enchant_weapon_to_dam
                            && borg.trait[BI_AENCH_TOD] < 1000
                            && borg.trait[BI_NEED_ENCHANT_TO_D])
                        || item->sval == sv_scroll_star_enchant_weapon
                        || item->sval == sv_scroll_star_enchant_armor))
                || (item->tval == TV_POTION
                    && (item->sval == sv_potion_inc_str
                        || item->sval == sv_potion_inc_int
                        || item->sval == sv_potion_inc_wis
                        || item->sval == sv_potion_inc_dex
                        || item->sval == sv_potion_inc_con
                        || item->sval == sv_potion_inc_all)))) {
            /* No encumbrance penalty for purchasing these items */
        } else {
            total -= ((borg.trait[BI_WEIGHT] - (borg.trait[BI_CARRY] / 2))
                      / (borg.trait[BI_CARRY] / 10) * 1000L);
        }
    }
    /* END SECOND MAJOR HACK */

    return total;
}

/*
 * Determine what level the borg is prepared to dive to.
 */
const char *borg_prepared_dynamic(int depth)
{
    int i;

    for (i = 0; i < borg_formulas.depth.count; i++) {
        struct borg_depth_line *d = borg_formulas.depth.items[i];
        if (d->dlevel > depth)
            break;
        if (check_condition(d->condition, depth))
            return d->reason;
    }

    return NULL;
}

/*
 * Determine if the borg needs to restock.
 */
const char *borg_restock_dynamic(int depth)
{
    int i;

    for (i = 0; i < borg_formulas.restock.count; i++) {
        struct borg_depth_line *d = borg_formulas.restock.items[i];
        if (d->dlevel > depth)
            break;
        if (check_condition(d->condition, depth))
            return d->reason;
    }

    return NULL;
}

/*
 * read the formula section of the borg.txt file
 */
bool borg_load_formulas(ang_file *fp)
{
    enum e_section { SEC_NONE, SEC_POWER, SEC_DEPTH, SEC_RESTOCK };

    char           buf[1024];
    bool           formulas_off = false;
    enum e_section section      = SEC_NONE;
    bool           skip_open    = true;

    borg_formulas.restock.count = 0;
    borg_formulas.depth.count   = 0;
    borg_formulas.power.count   = 0;

    /* read but don't process formulas if we aren't using them */
    if (!borg_cfg[BORG_USES_DYNAMIC_CALCS])
        formulas_off = true;

    /* Parse the formula section of the file */
    while (file_getl(fp, buf, sizeof(buf) - 1)) {

        char *line = borg_trim(buf);

        /* Skip comments and blank lines */
        if (!line[0] || (line[0] == '#'))
            continue;

        if (prefix_i(line, "[END FORMULA SECTION]"))
            break;

        /* if we failed anywhere */
        if (formulas_off)
            continue;

        /* sections are bounded in [] */
        if (line[0] == '[') {
            if (!skip_open)
                formulas_off = true;

            skip_open = false;
            continue;
        }
        if (line[0] == ']') {
            if (section == SEC_NONE)
                return formulas_off;
            section = SEC_NONE;
            continue;
        }

        /* if we failed anywhere */
        if (formulas_off)
            continue;

        if (prefix_i(line, "Power")) {
            if (section != SEC_NONE)
                formulas_off = true;

            section   = SEC_POWER;
            skip_open = true;
            continue;
        }
        if (prefix_i(line, "Depth Requirement")) {
            if (section != SEC_NONE)
                formulas_off = true;

            section   = SEC_DEPTH;
            skip_open = true;
            continue;
        }
        if (prefix_i(line, "Restock Requirement")) {
            if (section != SEC_NONE)
                formulas_off = true;

            section   = SEC_RESTOCK;
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
            else if (section == SEC_RESTOCK)
                borg_formula_error(
                    buf, buf, "restock", "** line must end in a ;");
            else
                borg_formula_error(buf, buf, "base", "** line must end in a ;");
            formulas_off = true;
        } else {
            if (section == SEC_DEPTH)
                formulas_off = parse_depth_line(false, line, buf);
            if (section == SEC_RESTOCK)
                formulas_off = parse_depth_line(true, line, buf);
            if (section == SEC_POWER)
                formulas_off = parse_power_line(line, buf);
            if (section == SEC_NONE) {
                borg_formula_error(buf, buf, "base",
                    "** line must be part of a Power or Depth section");
                formulas_off = true;
            }
        }
    }
    if (formulas_off && borg_cfg[BORG_USES_DYNAMIC_CALCS])
        borg_cfg[BORG_USES_DYNAMIC_CALCS] = false;

    return formulas_off;
}

/*
 * Free all memory used in one power line
 */
static void power_free(struct borg_power_line *power)
{
    if (power) {
        if (power->condition) {
            mem_free(power->condition);
            power->condition = NULL;
        }
        if (power->range) {
            mem_free(power->range);
            power->range = NULL;
        }
        if (power->reward) {
            mem_free(power->reward);
            power->reward = NULL;
        }
        if (power->value) {
            mem_free(power->value);
            power->value = NULL;
        }
        mem_free(power);
        power = NULL;
    }
}

/*
 * Free all memory used in dynamic calculations 
 */
void borg_free_formulas(void)
{
    int i;
    for (i = 0; i < borg_formulas.restock.count; i++) {
        depth_free(borg_formulas.restock.items[i]);
        borg_formulas.restock.items[i] = NULL;
    }

    for (i = 0; i < borg_formulas.depth.count; i++) {
        depth_free(borg_formulas.depth.items[i]);
        borg_formulas.depth.items[i] = NULL;
    }

    for (i = 0; i < borg_formulas.power.count; i++) {
        power_free(borg_formulas.power.items[i]);
        borg_formulas.power.items[i] = NULL;
    }

    borg_formulas.restock.count = 0;
    borg_formulas.depth.count = 0;
    borg_formulas.power.count = 0;

    calculations_free();
}

#endif
