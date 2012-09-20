/* File: z-util.c */

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
 * Are string 'a' and 'b' equal?
 */
bool streq(cptr a, cptr b)
{
  return (!strcmp(a,b));
}


/*
 * Is string 'small' the suffix of string 'big'?
 */
bool suffix(cptr big, cptr small)
{
  int blen = strlen (big);
  int slen = strlen (small);

  /* Degenerate case: 'big' is smaller than 'small' */
  if (slen > blen) return (FALSE);

  /* Compare small to the end of big */
  return (!strcmp(big + blen - slen, small));
}


/*
 * Is string 'small' the prefix of 'big'?
 */
bool prefix(cptr big, cptr small)
{
  cptr b = big;
  cptr s = small;

  /* Scan each char of small */
  while (*s)
  {
    /* Note: case of ('big' < 'small') caught here */
    if (*s++ != *b++) return (FALSE);
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




