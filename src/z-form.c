/* File: z-form.c */

#include "z-form.h"

#include "z-util.h"
#include "z-virt.h"


/*
 * The "type" of the "user defined print routine" pointer
 */
typedef uint (*vstrnfmt_aux_func)(char *buf, uint max, cptr fmt, vptr arg);

/*
 * The "default" user defined print routine.  Ignore the "fmt" string.
 */
static uint vstrnfmt_aux_dflt(char *buf, uint max, cptr fmt, vptr arg)
{
    int len;
    char tmp[32];
    sprintf(tmp, "<<%p>>", arg);
    len = strlen(tmp);
    if (len >= max) len = max - 1;
    tmp[len] = '\0';
    strcpy(buf, tmp);
    return (len);
}

/*
 * The "current" user defined print routine.  It can be changed
 * dynamically by sending the proper "%m" sequence to "vstrnfmt()"
 */
static vstrnfmt_aux_func vstrnfmt_aux = vstrnfmt_aux_dflt;

/*
 * Basic "vararg" format function.  Takes a buffer, a max byte count,
 * a format string, and a va_list of arguments to the format string.
 *
 * We use the format string and the arguments to build a string, in the
 * manner of "sprintf()".  Let "str" be this string, and let "len" be
 * the smaller of "max-1" and "strlen(str)".  We copy the first "len"
 * chars of "str" into "buf", place "null" into buf[len], and return "len".
 *
 * In English, we do a sprintf() into "buf", a buffer with size "max",
 * and we return the resulting value of "strlen(buf)".
 *
 * Thus, "s += vstrnfmt(s, -1, fmt, vp)" will allow multiple writes
 * (at the cost of losing the max-length info), and more properly,
 * "s=buf; len=sizeof(buf); n=vstrnfmt(s,len,...); s+=n; len-=n; ..."
 * will do the same without losing the length information.
 *
 * Typically, "max" is in fact the "size" of "buf", and thus represents
 * the "number" of chars in "buf" which are ALLOWED to be used.  An
 * alternative definition would have required "buf" to hold at least
 * "max+1" characters, and would have used that extra character only
 * in the case where "buf" was too short for the result.
 *
 * Note that if the buffer was "too short" to hold the result, we will
 * always return "max-1", but we also return "max-1" if the buffer was
 * "just long enough".  We could have returned "max" if the buffer was
 * too short, not written a null, and forced the programmer to deal with
 * this special case, but I felt that it is better to at least give a
 * "usable" result when the buffer was too long instead of either giving
 * a memory overwrite like "sprintf()" or a non-terminted string like
 * "strncpy()".  Note that "strncpy()" also "null-pads" the result.
 *
 * Perhaps we should also accept a caret format flag (as in "%^s"), or
 * use the plus format flag, to "uppercase" the following string.
 *
 * We should consider processing "%+d" by hand, since apparently it
 * does not work on all machines.
 *
 * We should also consider extracting and handling the "width" and
 * other "flags" by hand, it will be more "accurate".
 *
 * Consider "user-defined" conversion formats.
 *
 * Hack -- currently, all "sub-format-strings" must generate results
 * at most 128 chars in length (this appears to be a constraint also
 * enforced by "sprintf()" in some implementations).
 *
 * Error detection is not very graceful, currently, if an error is
 * detected in the format string, we simply return a "length" of zero.
 * We should also be sure to "terminate" the "broken" result string.
 *
 * Note that "%%", "%n", and "%p" cannot take any "modifiers".
 * Note that "%s" and "%c" should not take "+" or "0" flags.
 * Note that "%s" always converts NULL to the empty string.
 *
 * Note that "%v" is used for a "user defined" print formatter.
 * Note that "%r" is used to "choose" a "user defined" print routine.
 * That is, one can do: format("The %r%v was destroyed!", objprt, obj);
 */
uint vstrnfmt(char *buf, uint max, cptr fmt, va_list vp)
{
  register cptr s;
  register char *a;
  register uint len = 0;

  int do_long;

  char aux[64];
  char tmp[128];

  /* Hack -- treat "illegal" length as "infinite" */
  if (!max) max = 32767;

  /* Hack -- treat "no format" as "empty string" */
  if (!fmt) fmt = "";

  /* Scan the format string */
  for (s = fmt; *s; )
  {
    /* Normal character, or "%%" format */
    if (*s != '%')
    {
      buf[len++] = *s++;
      if (len == max) break;
      continue;
    }

    /* Start the "aux" string */
    a = aux;

    /* Save (and skip) the "percent" */
    *a++ = *s++;

    /* Hack -- process "%%" */
    if (*s == '%')
    {
      buf[len++] = *s++;
      if (len == max) break;
      continue;
    }

    /* Hack -- process "%n" */
    if (*s == 'n')
    {
      int *arg = va_arg(vp, int *);
      (*arg) = len;
      s++;
      continue;
    }

    /* Hack -- process "%m" */
    if (*s == 'm')
    {
      /* Extract a "typed" function pointer and save it */
      vstrnfmt_aux = va_arg(vp, vstrnfmt_aux_func);

      /* Skip the "m" and continue */
      s++;
      continue;
    }

    /* Assume no "long" args */
    do_long = 0;

    /* Build the "aux" string */
    while (1)
    {
      /* Hack -- "illegal" format */
      if (!*s || (a > aux + 32))
      {
	buf[0] = '\0';
	return (0);
      }

      /* Save the character */
      *a++ = *s++;

      /* Hack -- Handle 'star' */
      if (a[-1] == '*')
      {
	int arg = va_arg(vp, int);
	a--;
	sprintf(a, "%d", arg);
	while (*a++);
      }

      /* Hack -- take note of "long" request */
      else if (a[-1] == 'l') do_long = 1;

      /* XXX Oops.  We do not handle this. */
      else if (a[-1] == 'L') quit("vstrnfmt('%...L') not allowed");

      /* Hack -- end the collection */
      else if (strchr("dioxXucsfeEgGpv", a[-1]))
      {
	*a = '\0';
	break;
      }
    }

    /* Clear "tmp" */
    tmp[0] = '\0';

    /* Hack -- user defined print routine (see above) */
    if (a[-1] == 'v')
    {
      /* Access the "user argument" */
      vptr arg = va_arg(vp, vptr);

      /* Append the result, collect the length */
      len += vstrnfmt_aux(buf+len, max-len, aux, arg);

      /* Continue */
      continue;
    }

    /* Process the "format" char */
    switch (a[-1])
    {
      /* Simple Character -- standard format */
      case 'c':
      {
	int arg = va_arg(vp, int);
	sprintf(tmp, aux, arg);
	break;
      }

      /* Signed Integers -- standard format */
      case 'd': case 'i':
      {
	if (do_long)
	{
	  long arg = va_arg(vp, long);
	  sprintf(tmp, aux, arg);
	}
	else
	{
	  int arg = va_arg(vp, int);
	  sprintf(tmp, aux, arg);
	}
	break;
      }

      /* Unsigned Integers -- various formats */
      case 'u': case 'o': case 'x': case 'X':
      {
	if (do_long)
	{
	  unsigned long arg = va_arg(vp, unsigned long);
	  sprintf(tmp, aux, arg);
	}
	else
	{
	  unsigned int arg = va_arg(vp, unsigned int);
	  sprintf(tmp, aux, arg);
	}
	break;
      }

      /* Floating Point -- various formats */
      case 'f':
      case 'e': case 'E':
      case 'g': case 'G':
      {
	double arg = va_arg(vp, double);
	sprintf(tmp, aux, arg);
	break;
      }

      /* Pointer -- implementation varies */
      case 'p':
      {
	vptr arg = va_arg(vp, vptr);
	sprintf(tmp, aux, arg);
	break;
      }

      case 's':
      {
	cptr arg = va_arg(vp, cptr);
	if (!arg) arg = "";
	sprintf(tmp, aux, arg);
	break;
      }

      default:
      {
	buf[0] = '\0';
	return (0);
      }
    }

    /* Now append "tmp" to "buf" */
    for (a = tmp; *a; a++)
    {
      buf[len++] = *a;
      if (len == max) break;
    }
  }

#if 0
  (void)(vsprintf(buf, fmt, vp));
  len = strlen(buf);
#endif

  /* Back up if needed (at most one space) */
  if (len >= max) len = max-1;

  /* Terminate */
  buf[len] = '\0';

  /* Return the number of chars written */
  return (len);
}


/*
 * Do a vstrnfmt (see above) into a (growable) static buffer.
 * This buffer is usable for very short term formatting of results.
 */
char *vformat(cptr fmt, va_list vp)
{
  static char *format_buf = NULL;
  static huge format_len = 0;

  /* Initial allocation */
  if (!format_buf)
  {
    format_len = 1024;
    C_MAKE(format_buf, format_len, char);
  }

  /* Null format yields last result */
  if (!fmt) return (format_buf);

  /* Keep going until successful */
  while (1)
  {
    uint len;

    /* Build the string */
    len = vstrnfmt(format_buf, format_len, fmt, vp);

    /* Success */
    if (len < format_len-1) break;

    /* Grow the buffer */
    C_KILL(format_buf, format_len, char);
    format_len = format_len * 2;
    C_MAKE(format_buf, format_len, char);
  }

  /* Return the new buffer */
  return (format_buf);
}



/*
 * Do a vstrnfmt (see above) into a buffer of a given size.
 */
uint strnfmt(char *buf, uint max, cptr fmt, ...)
{
  uint len;

  va_list vp;

  /* Begin the Varargs Stuff */
  va_start(vp, fmt);

  /* Do a virtual fprintf to stderr */
  len = vstrnfmt(buf, max, fmt, vp);

  /* End the Varargs Stuff */
  va_end(vp);

  /* Return the number of bytes written */
  return (len);
}


/*
 * Do a vstrnfmt (see above) into a buffer of unknown size.
 * Since the buffer size is unknown, the user better verify the args.
 */
uint strfmt(char *buf, cptr fmt, ...)
{
  uint len;

  va_list vp;

  /* Begin the Varargs Stuff */
  va_start(vp, fmt);

  /* Build the string, assume 32K buffer */
  len = vstrnfmt(buf, 32767, fmt, vp);

  /* End the Varargs Stuff */
  va_end(vp);

  /* Return the number of bytes written */
  return (len);
}




/*
 * Do a vstrnfmt() into (see above) into a (growable) static buffer.
 * This buffer is usable for very short term formatting of results.
 * Note that the buffer is (technically) writable, but only up to
 * the length of the string contained inside it.
 */
char *format(cptr fmt, ...)
{
  char *res;
  va_list vp;

  /* Begin the Varargs Stuff */
  va_start(vp, fmt);

  /* Format the args */
  res = vformat(fmt, vp);

  /* End the Varargs Stuff */
  va_end(vp);

  /* Return the result */
  return (res);
}




/*
 * Vararg interface to plog()
 */
void plog_fmt(cptr fmt, ...)
{
  char *res;
  va_list vp;

  /* Begin the Varargs Stuff */
  va_start(vp, fmt);

  /* Format the args */
  res = vformat(fmt, vp);

  /* End the Varargs Stuff */
  va_end(vp);

  /* Call plog */
  plog(res);
}



/*
 * Vararg interface to quit()
 */
void quit_fmt(cptr fmt, ...)
{
  char *res;
  va_list vp;

  /* Begin the Varargs Stuff */
  va_start(vp, fmt);

  /* Format */
  res = vformat(fmt, vp);

  /* End the Varargs Stuff */
  va_end(vp);

  /* Call quit() */
  quit(res);
}



/*
 * Vararg interface to core()
 */
void core_fmt(cptr fmt, ...)
{
  char *res;
  va_list vp;

  /* Begin the Varargs Stuff */
  va_start(vp, fmt);

  /* If requested, Do a virtual fprintf to stderr */
  res = vformat(fmt, vp);

  /* End the Varargs Stuff */
  va_end(vp);

  /* Call core() */
  core(res);
}


