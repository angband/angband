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

#include "borg-formulas-calc.h"

#ifdef ALLOW_BORG
#include "borg-formulas.h"
#include "borg-io.h"

struct borg_calculation {
    struct borg_array *token_array;
    int32_t            max_depth;
};

/* array of mathematic formulas */
struct borg_array calculations;

/* types of tokens that can be in a calculation */
enum token_type {
    TOK_NONE,
    /* values */
    TOK_VALUE,
    TOK_NUMBER,
    /* operators */
    TOK_MINUS,
    TOK_PLUS,
    TOK_MULT,
    TOK_DIV,
    TOK_AND,
    TOK_OR,
    TOK_EQUALS,
    TOK_LT,
    TOK_GT,
    TOK_LE,
    TOK_GE,
};

#define OP_LEVELS 4

/* a single token in the calculation */
struct token {
    enum token_type type;
    bool not ;
    void   *token;
    int32_t pdepth;
};

/*
 * Allocate and initialize a token structure
 */
static struct token *new_token(
    enum token_type ttype, void *token, int pdepth, bool not )
{
    struct token *tok = mem_zalloc(sizeof(struct token));
    tok->type         = ttype;
    tok->token        = token;
    tok->pdepth       = pdepth;
    tok->not          = not ;
    return tok;
}

/* 
 * grab an operator (+, -, and, or etc) from the line 
 */
static bool get_operator(char **in, enum token_type *ttype)
{
    char *line = *in;
    if (*line == '+') {
        *ttype = TOK_PLUS;
        (*in)++;
        return true;
    }

    if (*line == '-') {
        if (isdigit((int)(*(line + 1))))
            return false;
        (*in)++;
        *ttype = TOK_MINUS;
        return true;
    }

    if (*line == '*') {
        *ttype = TOK_MULT;
        (*in)++;
        return true;
    }

    if (*line == '/') {
        *ttype = TOK_DIV;
        (*in)++;
        return true;
    }

    if (prefix_i(line, "and")) {
        *ttype = TOK_AND;
        (*in) += 3;
        return true;
    }

    if (prefix_i(line, "&&")) {
        *ttype = TOK_AND;
        (*in) += 2;
        return true;
    }

    if (prefix_i(line, "or")) {
        *ttype = TOK_OR;
        (*in) += 2;
        return true;
    }

    if (prefix_i(line, "||")) {
        *ttype = TOK_OR;
        (*in) += 2;
        return true;
    }

    if (*line == '=') {
        *ttype = TOK_EQUALS;
        (*in)++;
        return true;
    }

    if (*line == '>') {
        *ttype = TOK_GT;
        (*in)++;
        if (*(line + 1) == '=') {
            *ttype = TOK_GE;
            (*in)++;
        }
        return true;
    }
    if (*line == '<') {
        *ttype = TOK_LT;
        (*in)++;
        if (*(line + 1) == '=') {
            *ttype = TOK_LE;
            (*in)++;
        }
        return true;
    }
    return false;
}

/*
 * get a "value()" from a formula string
 * this allocates and returns (in the value) one "name(xxx)"
 * returns false if this string doesn't start "value"
 */
static bool get_value_string(char *line, char **value)
{
    if (prefix_i(line, "value")) {
        char buf[1024];

        int   len;
        char *pos = line;
        line      = strchr(line, ')');
        if (line)
            len = line - pos + 1;
        else
            len = strlen(pos);
        strncpy(buf, pos, len);
        buf[len] = 0;
        *value   = string_make(buf);
        return true;
    }

    return false;
}

/*
 * turn formula into an array of tokens
 */
static bool tokenize_math(
    struct borg_calculation *f, char *line, const char *full_line)
{
    bool fail              = false;
    bool not               = false;
    int             pdepth = 0;
    char           *start  = line;
    enum token_type type;
    char           *value_token = NULL;

    f->token_array              = mem_zalloc(sizeof(struct borg_array));

    while (*line) {
        if (get_value_string(line, &value_token)) {
            line += strlen(value_token);
            struct value_sec *v = parse_value(value_token, full_line);
            if (v == NULL)
                fail = true;
            mem_free(value_token);
            value_token = NULL;
            if (fail) {
                mem_free(v);
                v = NULL;
                break;
            }
            if (pdepth > f->max_depth)
                f->max_depth = pdepth;
            borg_array_add(
                f->token_array, new_token(TOK_VALUE, v, pdepth, not ));
            not = false;
            continue;
        }

        if (*line == '!') {
            not = true;
            line++;
            continue;
        }

        if (*line == '(') {
            pdepth++;
            line++;
            continue;
        }

        if (*line == ')') {
            pdepth--;
            if (pdepth < 0) {
                borg_formula_error(start, full_line, "calculation",
                    "** unmatched parenthesis");
                fail = true;
                break;
            }
            line++;
            continue;
        }

        if (isdigit((int)(*line))
            || (*line == '-' && isdigit((int)(*(line + 1))))) {

            int32_t *val = mem_alloc(sizeof(int32_t));
            *val         = atol(line);

            if (pdepth > f->max_depth)
                f->max_depth = pdepth;
            borg_array_add(
                f->token_array, new_token(TOK_NUMBER, val, pdepth, not ));
            not = false;
            while (isdigit((unsigned char)*line) || *line == '-')
                line++;
            continue;
        }

        /* get operator moves the line pointer since operators are */
        /* varying length */
        if (get_operator(&line, &type)) {
            if (pdepth > f->max_depth)
                f->max_depth = pdepth;
            borg_array_add(
                f->token_array, new_token(type, NULL, pdepth, false));
            if (not ) {
                borg_formula_error(start, full_line, "calculation",
                    "** not (!) can only be applied to values");
                fail = true;
            }
            continue;
        }
        line++;
    }
    if (pdepth && !fail) {
        borg_formula_error(
            start, full_line, "calculation", "** unmatched parenthesis");
        fail = true;
    }

    return fail;
}

/* 
 * Quick helper to return if this token is an operator or a value
 */
static bool is_operator(enum token_type type) { return type >= TOK_MINUS; }

/*
 * Precedence order for operators
 * 0) mult/div
 * 1) add/sub
 * 2) compare (eq, gt, lt, le, ge)
 * 3) logic (and or)
 */
static int operation_level(enum token_type type)
{
    if (type == TOK_DIV || type == TOK_MULT)
        return 0;
    if (type == TOK_PLUS || type == TOK_MINUS)
        return 1;
    if (type == TOK_AND || type == TOK_OR)
        return 3;
    return 2;
}

/*
 * This takes a calculation and the position of an operation and adds 
 * "depth" (virtual parenthesis) to that operation.
 */
static void add_depth(struct borg_calculation *f, int pos)
{
    struct token *tok    = f->token_array->items[pos];
    int           pdepth = tok->pdepth;
    int           i;

    tok->pdepth++;

    /* left */
    for (i = pos - 1; i >= 0; i--) {
        tok = f->token_array->items[i];
        /* check for after this operand*/
        if (tok->pdepth < pdepth
            || (is_operator(tok->type) && tok->pdepth == pdepth))
            break;
        tok->pdepth++;
        if (tok->pdepth > f->max_depth)
            f->max_depth = tok->pdepth;
    }
    /* right */
    int end = f->token_array->count;
    for (i = pos + 1; i < end; i++) {
        tok = f->token_array->items[i];
        if (tok->pdepth < pdepth
            || (is_operator(tok->type) && tok->pdepth == pdepth))
            break;
        tok->pdepth++;
        if (tok->pdepth > f->max_depth)
            f->max_depth = tok->pdepth;
    }
}

/*
 * adjust the formula so the depth reflects order of operation.
 * 1) mult/div
 * 2) add/sub
 * 3) compare (eq, gt, lt, le, ge)
 * 4) logic (and or)
 *
 * if there are more than one operator (so a + b * c = d)
 * calculate as if (a + (b * c)) = d
 */
static bool adjust_order_ops_depth_block(
    struct borg_calculation *f, int pdepth, int start, int end)
{
    struct operator_struct {
        enum token_type type;
        int             level;
        int             position;
    };
    struct borg_array      *operators = mem_zalloc(sizeof(struct borg_array));
    struct operator_struct *o;
    int                     tok;
    int                     i, j;
    int                     lvl;
    int                     l[4] = { 0, 0, 0, 0 };

    for (tok = start; tok < end; tok++) {
        struct token *token = f->token_array->items[tok];
        if (token->pdepth != pdepth)
            continue;
        if (is_operator(token->type)) {
            o           = mem_alloc(sizeof(struct operator_struct));
            o->type     = token->type;
            o->level    = operation_level(token->type);
            o->position = tok;
            l[o->level]++;
            borg_array_add(operators, o);
        }
    }

    /* find groups at the same level so the formula */
    /* 1 + 2 * (3 + 4 * 5) + 6 + 7 */
    /* already has things at two levels */
    /* "1 + 2 *" is at one level "3 + 4 * 5" is deeper (done first) and */
    /* "+ 6 + 7" is back at the first level.  */
    /* it grabs the groups and the adds depth (as if they were parenthesized) */
    /* to higher operations levels.  so it grabs 1 + 2 * (deeper) and */
    /* calculates "*" is first so it makes it as if it was 1 + (2 * (deeper)) */
    /* by pushing the 2 and the "(deeper)" down.  Repeat until all levels are */
    /* handled */
    bool higher;
    if (operators->count > 1) {
        int count_at_level = 0;
        for (lvl = 0; lvl < OP_LEVELS; lvl++) {
            if (l[lvl]) {
                for (i = 0; i < operators->count; i++) {
                    o = operators->items[i];
                    if (o->level == lvl) {
                        /* if there is an at higher level or this isn't the last
                         */
                        /* the last check is for a*b*c needs to be (a*b)*c */
                        higher = false;
                        for (j = lvl + 1; j < OP_LEVELS; j++)
                            if (l[j]) {
                                higher = true;
                                break;
                            }
                        if (higher || count_at_level < (operators->count - 1)) {
                            add_depth(f, o->position);
                            count_at_level++;
                        }
                    }
                }
            }
        }
    }

    /* free up the temporary memory */
    for (i = 0; i < operators->count; i++) {
        mem_free(operators->items[i]);
        operators->items[i] = NULL;
    }
    mem_free(operators);
    operators = NULL;

    return false;
}

/*
 * apply order of operations to the formula.  Do this by pretending there
 * are parens around operations of higher precedence when there is more than
 * one operator at a level.
 */
static bool adjust_order_operations(struct borg_calculation *f)
{
    bool fail = false;
    int  pdepth, itoken;
    int  start;
    for (pdepth = 0; pdepth <= f->max_depth; pdepth++) {
        start = -1;
        for (itoken = 0; itoken < f->token_array->count; itoken++) {
            struct token *tok = f->token_array->items[itoken];
            if (start == -1 && tok->pdepth == pdepth) {
                start = itoken;
                continue;
            }
            /* do a block of tokens */
            if (start != -1 && tok->pdepth < pdepth) {
                fail
                    = adjust_order_ops_depth_block(f, pdepth, start, itoken - 1);
                start = -1;
            }
        }
        if (start != -1) {
            fail  = adjust_order_ops_depth_block(f, pdepth, start, itoken - 1);
            start = -1;
        }
    }
    return fail;
}

/* 
 * check for format of a formula 
 */
static bool validate_calculation(
    struct borg_calculation *f, char *line, const char *full_line)
{
    if (!f->token_array->count) {
        borg_formula_error(
            line, full_line, "calculation", "** formula must contain values");
        return true;
    }

    int           ntoken;
    struct token *tok;
    for (ntoken = 0; ntoken < f->token_array->count; ntoken++) {
        tok = f->token_array->items[ntoken];
        if (ntoken % 2) {
            if (!is_operator(tok->type)) {
                borg_formula_error(line, full_line, "calculation",
                    "** formula must be values separated by operators");
                return true;
            }
        }
        if (!(ntoken % 2)) {
            if (is_operator(tok->type)) {
                borg_formula_error(line, full_line, "calculation",
                    "** formula must be values separated by operators");
                return true;
            }
        }
    }
    tok = f->token_array->items[f->token_array->count - 1];
    if (is_operator(tok->type)) {
        borg_formula_error(line, full_line, "calculation",
            "** formula can't end in an operation");
        return true;
    }
    return false;
}

/*
 * parse the formula into a structure that can be calculated from.
 */
static bool parse_calculation(
    struct borg_calculation *f, char *line, const char *full_line)
{
    if (tokenize_math(f, line, full_line))
        return true;

    /* modify the depth's to conform with order of operations */
    if (adjust_order_operations(f))
        return true;

    return validate_calculation(f, line, full_line);
}

/*
 * free memory used by a token
 */
static void token_free(struct token *t)
{
    if (t->token) {
        mem_free(t->token);
        t->token = NULL;
    }
    mem_free(t);
    t = NULL;
}

/*
 * free memory used by a borg_calculation
 */
static void calc_free(struct borg_calculation *calc)
{
    if (calc->token_array) {
        for (int i = 0; i < calc->token_array->count; i++) {
            token_free(calc->token_array->items[i]);
            calc->token_array->items[i] = NULL;
        }
        mem_free(calc->token_array);
        calc->token_array = NULL;
    }
    mem_free(calc);
    calc = NULL;
}

/*
 * parse the formula and add it to the formula list
 */
int parse_calculation_line(char *line, const char *full_line)
{
    struct borg_calculation *f = mem_zalloc(sizeof(struct borg_calculation));
    if (parse_calculation(f, line, full_line)) {
        calc_free(f);
        f = NULL;
        return -1;
    }

    int i = borg_array_add(&calculations, f);
    if (i == -1)
        borg_formula_error(
            line, full_line, "calculation", "** can't add to formula array");
    return i;
}

/*
 * Turn a formula into a value for a given parenthesis level
 *   this will use recursion to get deeper levels of the formula */
static int32_t calculate_value_from_formula_depth(
    struct borg_array *f, int *i, int pdepth, int range_index)
{
    struct token *left_token = (struct token *)f->items[*i];
    int32_t       left_value = 0;

    /* get the value on the left */
    if (left_token->pdepth > pdepth)
        left_value
            = calculate_value_from_formula_depth(f, i, pdepth + 1, range_index);
    else if (left_token->type == TOK_NUMBER) {
        left_value = *((int32_t *)left_token->token);
        if (left_token->not )
            left_value = !left_value;
    } else if (left_token->type == TOK_VALUE) {
        left_value = calculate_from_value(
            (struct value_sec *)left_token->token, range_index);
        if (left_token->not )
            left_value = !left_value;
    }

    if ((*i) + 1 >= f->count)
        return left_value;

    struct token *operation = (struct token *)f->items[(*i) + 1];
    if (operation->pdepth < pdepth)
        return left_value;
    ++(*i);

    /* get the value on the right */
    int32_t       right_value = 0;
    struct token *right_token = (struct token *)f->items[++(*i)];
    if (right_token->pdepth > pdepth)
        right_value
            = calculate_value_from_formula_depth(f, i, pdepth + 1, range_index);
    else if (right_token->type == TOK_NUMBER) {
        right_value = *((int32_t *)right_token->token);
        if (right_token->not )
            right_value = !right_value;
    } else if (right_token->type == TOK_VALUE) {
        right_value = calculate_from_value(
            (struct value_sec *)right_token->token, range_index);
        if (right_token->not )
            right_value = !right_value;
    }

    int32_t total = 0;
    switch (operation->type) {
    case TOK_MINUS:
        total = left_value - right_value;
        break;
    case TOK_PLUS:
        total = left_value + right_value;
        break;
    case TOK_MULT:
        total = left_value * right_value;
        break;
    case TOK_DIV:
        total = left_value / right_value;
        break;
    case TOK_AND:
        total = left_value && right_value;
        break;
    case TOK_OR:
        total = left_value || right_value;
        break;
    case TOK_EQUALS:
        total = left_value == right_value;
        break;
    case TOK_LT:
        total = left_value < right_value;
        break;
    case TOK_GT:
        total = left_value > right_value;
        break;
    case TOK_LE:
        total = left_value <= right_value;
        break;
    case TOK_GE:
        total = left_value >= right_value;
        break;

    default: /* should never get here. */
        borg_note("** borg formula failure ** ");
        borg_note("** error calculating a formula value");
    }
    if ((*i) + 1 >= f->count)
        return total;
    struct token *next_token = (struct token *)f->items[(*i) + 1];
    if (next_token->pdepth != pdepth)
        return total;

    /* we should never get here.  The parsing should handle all validation */
    borg_note("** borg formula failure ** ");
    borg_note("** error calculating a formula value");
    return 0;
}

/*
 * Turn a formula pointer into a value
 */
int32_t borg_calculate_dynamic(int formula, int range_index)
{
    int                      i = 0;
    struct borg_calculation *f = calculations.items[formula];

    return calculate_value_from_formula_depth(
        f->token_array, &i, 0, range_index);
}

/*
 * Free up all memory used in calculations.
 */
void calculations_free(void)
{
    for (int i = 0; i < calculations.count; i++) {
        calc_free(calculations.items[i]);
        calculations.items[i] = NULL;
    }
    calculations.count = 0;
}

#endif
