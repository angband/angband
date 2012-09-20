/* File: z-util.c */

/*
 * Copyright (c) 1997 Ben Harrison
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

/* Purpose: Low level utilities -BEN- */

#include "z-util.h"



/*
 * Global variables for temporary use
 */
char char_tmp = 0;
byte byte_tmp = 0;
sint sint_tmp = 0;
uint uint_tmp = 0;
long long_tmp = 0;
huge huge_tmp = 0;
errr errr_tmp = 0;


/*
 * Global pointers for temporary use
 */
cptr cptr_tmp = NULL;
vptr vptr_tmp = NULL;




/*
 * Constant bool meaning true
 */
bool bool_true = 1;

/*
 * Constant bool meaning false
 */
bool bool_false = 0;


/*
 * Global NULL cptr
 */
cptr cptr_null = NULL;


/*
 * Global NULL vptr
 */
vptr vptr_null = NULL;



/*
 * Global SELF vptr
 */
vptr vptr_self = (vptr)(&vptr_self);



/*
 * Convenient storage of the program name
 */
cptr argv0 = NULL;



/*
 * A routine that does nothing
 */
void func_nothing(void)
{
	/* Do nothing */
}


/*
 * A routine that always returns "success"
 */
errr func_success(void)
{
	return (0);
}


/*
 * A routine that always returns a simple "problem code"
 */
errr func_problem(void)
{
	return (1);
}


/*
 * A routine that always returns a simple "failure code"
 */
errr func_failure(void)
{
	return (-1);
}



/*
 * A routine that always returns "true"
 */
bool func_true(void)
{
	return (1);
}


/*
 * A routine that always returns "false"
 */
bool func_false(void)
{
	return (0);
}




/*
 * Determine if string "t" is equal to string "t"
 */
bool streq(cptr a, cptr b)
{
	return (!strcmp(a, b));
}


/*
 * Determine if string "t" is a suffix of string "s"
 */
bool suffix(cptr s, cptr t)
{
	int tlen = strlen(t);
	int slen = strlen(s);

	/* Check for incompatible lengths */
	if (tlen > slen) return (FALSE);

	/* Compare "t" to the end of "s" */
	return (!strcmp(s + slen - tlen, t));
}


/*
 * Determine if string "t" is a prefix of string "s"
 */
bool prefix(cptr s, cptr t)
{
	/* Scan "t" */
	while (*t)
	{
		/* Compare content and length */
		if (*t++ != *s++) return (FALSE);
	}

	/* Matched, we have a prefix */
	return (TRUE);
}



/*
 * Redefinable "plog" action
 */
void (*plog_aux)(cptr) = NULL;

/*
 * Print (or log) a "warning" message (ala "perror()")
 * Note the use of the (optional) "plog_aux" hook.
 */
void plog(cptr str)
{
	/* Use the "alternative" function if possible */
	if (plog_aux) (*plog_aux)(str);

	/* Just do a labeled fprintf to stderr */
	else (void)(fprintf(stderr, "%s: %s\n", argv0 ? argv0 : "???", str));
}



/*
 * Redefinable "quit" action
 */
void (*quit_aux)(cptr) = NULL;

/*
 * Exit (ala "exit()").  If 'str' is NULL, do "exit(0)".
 * If 'str' begins with "+" or "-", do "exit(atoi(str))".
 * Otherwise, plog() 'str' and exit with an error code of -1.
 * But always use 'quit_aux', if set, before anything else.
 */
void quit(cptr str)
{
	/* Attempt to use the aux function */
	if (quit_aux) (*quit_aux)(str);

	/* Success */
	if (!str) (void)(exit(0));

	/* Extract a "special error code" */
	if ((str[0] == '-') || (str[0] == '+')) (void)(exit(atoi(str)));

	/* Send the string to plog() */
	plog(str);

	/* Failure */
	(void)(exit(-1));
}



/*
 * Redefinable "core" action
 */
void (*core_aux)(cptr) = NULL;

/*
 * Dump a core file, after printing a warning message
 * As with "quit()", try to use the "core_aux()" hook first.
 */
void core(cptr str)
{
	char *crash = NULL;

	/* Use the aux function */
	if (core_aux) (*core_aux)(str);

	/* Dump the warning string */
	if (str) plog(str);

	/* Attempt to Crash */
	(*crash) = (*crash);

	/* Be sure we exited */
	quit("core() failed");
}




