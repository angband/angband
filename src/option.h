#ifndef INCLUDED_OPTIONS_H
#define INCLUDED_OPTIONS_H

#include "z-file.h"

/*
 * Option types 
 */
enum
{
	OP_INTERFACE = 0,
	OP_BIRTH,
	OP_CHEAT,
	OP_SCORE,
	OP_SPECIAL,

	OP_MAX
};

/*** Functions ***/

/** Given an option index, return its name */
const char *option_name(int opt);

/** Given an option index, return its description */
const char *option_desc(int opt);

/** Determine the type of option (score, birth etc) */
int option_type(int opt);

/** Set an an option, return TRUE if successful */
bool option_set(const char *opt, int val);

/** Initialise options to defaults */
void init_options(void);

/** Write options to file */
void option_dump(ang_file *f);

/*** Option display definitions ***/

/*
 * Information for "do_cmd_options()".
 */
#define OPT_PAGE_MAX				OP_SCORE
#define OPT_PAGE_PER				20

#define OPT_PAGE_BIRTH				1

/* The option data structures */
extern int option_page[OPT_PAGE_MAX][OPT_PAGE_PER];


/*
 * Option indexes 
 */
enum
{
    #define OP(a, b, c, d) OPT_##a,
    #include "list-options.h"
    #undef OP
	OPT_MAX
};


#define OPT(opt_name)	op_ptr->opt[OPT_##opt_name]

#endif /* !INCLUDED_OPTIONS_H */
