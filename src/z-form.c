/* File: z-form.c */

/* Purpose: Low level text formatting -BEN- */

#include "z-form.h"

#include "z-util.h"
#include "z-virt.h"


/*
 * Here is some information about the routines in this file.
 *
 * In general, the following routines take a "buffer", a "max length",
 * a "format string", and some "arguments", and use the format string
 * and the arguments to create a (terminated) string in the buffer
 * (using only the first "max length" bytes), and return the "length"
 * of the resulting string, not including the (mandatory) terminator.
 *
 * The format strings allow the basic "sprintf()" format sequences, though
 * some of them are processed slightly more carefully or portably, as well
 * as a few "special" sequences, including the "%r" and "%v" sequences, and
 * the "capilitization" sequences of "%C", "%S", and "%V".
 *
 * Note that some "limitations" are enforced by the current implementation,
 * for example, no "format sequence" can exceed 100 characters, including any
 * "length" restrictions, and the result of combining and "format sequence"
 * with the relevent "arguments" must not exceed 1000 characters.
 *
 * These limitations could be fixed by stealing some of the code from,
 * say, "vsprintf()" and placing it into my "vstrnfmt()" function.
 *
 * Note that a "^" inside a "format sequence" causes the first non-space
 * character in the string resulting from the combination of the format
 * sequence and the argument(s) to be "capitalized" if possible.  Note
 * that the "^" character is removed before the "standard" formatting
 * routines are called.  Likewise, a "*" inside a "format sequence" is
 * removed from the "format sequence", and replaced by the textual form
 * of the next argument in the argument list.  See examples below.
 *
 * Legal format characters: %,n,p,c,s,d,i,o,u,X,x,E,e,F,f,G,g,r,v.
 *
 * Format("%%")
 *   Append the literal "%".
 *   No legal modifiers.
 *
 * Format("%n", int *np)
 *   Save the current length into (*np).
 *   No legal modifiers.
 *
 * Format("%p", vptr v)
 *   Append the pointer "v" (implementation varies).
 *   No legal modifiers.
 *
 * Format("%E", double r)
 * Format("%F", double r)
 * Format("%G", double r)
 * Format("%e", double r)
 * Format("%f", double r)
 * Format("%g", double r)
 *   Append the double "r", in various formats.
 *
 * Format("%ld", long int i)
 *   Append the long integer "i".
 *
 * Format("%d", int i)
 *   Append the integer "i".
 *
 * Format("%lu", unsigned long int i)
 *   Append the unsigned long integer "i".
 *
 * Format("%u", unsigned int i)
 *   Append the unsigned integer "i".
 *
 * Format("%lo", unsigned long int i)
 *   Append the unsigned long integer "i", in octal.
 *
 * Format("%o", unsigned int i)
 *   Append the unsigned integer "i", in octal.
 *
 * Format("%lX", unsigned long int i)
 *   Note -- use all capital letters
 * Format("%lx", unsigned long int i)
 *   Append the unsigned long integer "i", in hexidecimal.
 *
 * Format("%X", unsigned int i)
 *   Note -- use all capital letters
 * Format("%x", unsigned int i)
 *   Append the unsigned integer "i", in hexidecimal.
 *
 * Format("%c", char c)
 *   Append the character "c".
 *   Do not use the "+" or "0" flags.
 *
 * Format("%s", cptr s)
 *   Append the string "s".
 *   Do not use the "+" or "0" flags.
 *   Note that a "NULL" value of "s" is converted to the empty string.
 *
 * Format("%V", vptr v)
 *   Note -- possibly significant mode flag
 * Format("%v", vptr v)
 *   Append the object "v", using the current "user defined print routine".
 *   User specified modifiers, often ignored.
 *
 * Format("%r", vstrnfmt_aux_func *fp)
 *   Set the "user defined print routine" (vstrnfmt_aux) to "fp".
 *   No legal modifiers.
 *
 *
 * For examples below, assume "int n = 0; int m = 100; char buf[100];",
 * plus "char *s = NULL;", and unknown values "char *txt; int i;".
 *
 * For example: "n = strnfmt(buf, -1, "(Max %d)", i);" will have a
 * similar effect as "sprintf(buf, "(Max %d)", i); n = strlen(buf);".
 *
 * For example: "(void)strnfmt(buf, 16, "%s", txt);" will have a similar
 * effect as "strncpy(buf, txt, 16); buf[15] = '\0';".
 *
 * For example: "if (strnfmt(buf, 16, "%s", txt) < 16) ..." will have
 * a similar effect as "strcpy(buf, txt)" but with bounds checking.
 *
 * For example: "s = buf; s += vstrnfmt(s, -1, ...); ..." will allow
 * multiple "appends" to "buf" (at the cost of losing the max-length info).
 *
 * For example: "s = buf; n = vstrnfmt(s+n, 100-n, ...); ..." will allow
 * multiple bounded "appends" to "buf", with constant access to "strlen(buf)".
 *
 * For example: "format("The %r%v was destroyed!", obj_desc, obj);"
 * (where "obj_desc(buf, max, fmt, obj)" will "append" a "description"
 * of the given object to the given buffer, and return the total length)
 * will return a "useful message" about the object "obj", for example,
 * "The Large Shield was destroyed!".
 *
 * For example: "format("%^-.*s", i, txt)" will produce a string containing
 * the first "i" characters of "txt", left justified, with the first non-space
 * character capitilized, if reasonable.
 */





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
 * dynamically by sending the proper "%r" sequence to "vstrnfmt()"
 */
static vstrnfmt_aux_func vstrnfmt_aux = vstrnfmt_aux_dflt;



/*
 * Basic "vararg" format function.
 *
 * This function takes a buffer, a max byte count, a format string, and
 * a va_list of arguments to the format string, and uses the format string
 * and the arguments to create a string to the buffer.  The string is
 * derived from the format string and the arguments in the manner of the
 * "sprintf()" function, but with some extra "format" commands.  Note that
 * this function will never use more than the given number of bytes in the
 * buffer, preventing messy invalid memory references.  This function then
 * returns the total number of non-null bytes written into the buffer.
 *
 * Method: Let "str" be the (unlimited) created string, and let "len" be the
 * smaller of "max-1" and "strlen(str)".  We copy the first "len" chars of
 * "str" into "buf", place "\0" into buf[len], and return "len".
 *
 * In English, we do a sprintf() into "buf", a buffer with size "max",
 * and we return the resulting value of "strlen(buf)", but we allow some
 * special format commands, and we are more careful than "sprintf()".
 *
 * Typically, "max" is in fact the "size" of "buf", and thus represents
 * the "number" of chars in "buf" which are ALLOWED to be used.  An
 * alternative definition would have required "buf" to hold at least
 * "max+1" characters, and would have used that extra character only
 * in the case where "buf" was too short for the result.  This would
 * give an easy test for "overflow", but a less "obvious" semantics.
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
 * Note that in most cases "just long enough" is probably "too short".
 *
 * We should also consider extracting and processing the "width" and other
 * "flags" by hand, it might be more "accurate", and it would allow us to
 * remove the limit (1000 chars) on the result of format sequences.
 *
 * Also, some sequences, such as "%+d" by hand, do not work on all machines,
 * and could thus be correctly handled here.
 *
 * Error detection in this routine is not very graceful, in particular,
 * if an error is detected in the format string, we simply "pre-terminate"
 * the given buffer to a length of zero, and return a "length" of zero.
 * The contents of "buf", except for "buf[0]", may then be undefined.
 */
uint vstrnfmt(char *buf, uint max, cptr fmt, va_list vp)
{
  cptr s;

  /* The argument is "long" */
  bool do_long;

  /* The argument needs "processing" */
  bool do_xtra;

  /* Bytes used in buffer */
  uint n;

  /* Bytes used in format sequence */
  uint q;

  /* Format sequence */
  char aux[128];  

  /* Resulting string */
  char tmp[1024];


  /* Mega-Hack -- treat "illegal" length as "infinite" */
  if (!max) max = 32767;

  /* Mega-Hack -- treat "no format" as "empty string" */
  if (!fmt) fmt = "";


  /* Begin the buffer */
  n = 0;
  
  /* Begin the format string */
  s = fmt;
  
  /* Scan the format string */
  while (TRUE)
  {
    /* All done */
    if (!*s) break;
    
    /* Normal character */
    if (*s != '%')
    {
      /* Check total length */
      if (n == max-1) break;

      /* Save the character */
      buf[n++] = *s++;

      /* Continue */
      continue;
    }

    /* Skip the "percent" */
    s++;

    /* Pre-process "%%" */
    if (*s == '%')
    {
      /* Check total length */
      if (n == max-1) break;

      /* Save the percent */
      buf[n++] = '%';
      
      /* Skip the "%" */
      s++;

      /* Continue */
      continue;
    }

    /* Pre-process "%n" */
    if (*s == 'n')
    {
      int *arg;
      
      /* Access the next argument */
      arg = va_arg(vp, int *);

      /* Save the current length */
      (*arg) = n;

      /* Skip the "n" */
      s++;

      /* Continue */
      continue;
    }

    /* Hack -- Pre-process "%r" */
    if (*s == 'r')
    {
      /* Extract the next argument, and save it (globally) */
      vstrnfmt_aux = va_arg(vp, vstrnfmt_aux_func);

      /* Skip the "r" */
      s++;

      /* Continue */
      continue;
    }


    /* Begin the "aux" string */
    q = 0;

    /* Save the "percent" */
    aux[q++] = '%';

    /* Assume no "long" argument */
    do_long = FALSE;

    /* Assume no "xtra" processing */
    do_xtra = FALSE;
    
    /* Build the "aux" string */
    while (TRUE)
    {
      /* Error -- format sequence is not terminated */
      if (!*s)
      {
        /* Terminate the buffer */
        buf[0] = '\0';

        /* Return "error" */
        return (0);
      }

      /* Error -- format sequence may be too long */
      if (q > 100)
      {
        /* Terminate the buffer */
        buf[0] = '\0';

        /* Return "error" */
        return (0);
      }

      /* Handle "alphabetic" chars */
      if (isalpha(*s))
      {
        /* Hack -- handle "long" request */
        if (*s == 'l')
        {
          /* Save the character */
          aux[q++] = *s++;

          /* Note the "long" flag */
          do_long = TRUE;
        }

        /* Mega-Hack -- handle "extra-long" request */
        else if (*s == 'L')
        {
          /* Error -- illegal format char */
          buf[0] = '\0';

          /* Return "error" */
          return (0);
        }

        /* Handle normal end of format sequence */
        else
        {
          /* Save the character */
          aux[q++] = *s++;

          /* Stop processing the format sequence */
          break;
        }
      }
      
      /* Handle "non-alphabetic" chars */
      else
      {
        /* Hack -- Handle 'star' (for "variable length" argument) */
        if (*s == '*')
        {
          int arg;
        
          /* Access the next argument */
          arg = va_arg(vp, int);

          /* Hack -- append the "length" */
          sprintf(aux + q, "%d", arg);

          /* Hack -- accept the "length" */
          while (aux[q]) q++;

          /* Skip the "*" */
          s++;
        }
      
        /* Mega-Hack -- Handle 'caret' (for "uppercase" request) */
        else if (*s == '^')
        {
          /* Note the "xtra" flag */
          do_xtra = TRUE;

          /* Skip the "^" */
          s++;
        }

        /* Collect "normal" characters (digits, "-", "+", ".", etc) */
        else
        {
          /* Save the character */
          aux[q++] = *s++;
        }
      }
    }


    /* Terminate "aux" */
    aux[q] = '\0';
    
    /* Clear "tmp" */
    tmp[0] = '\0';

    /* Process the "format" char */
    switch (aux[q-1])
    {
      /* Simple Character -- standard format */
      case 'c':
      {
        int arg;
        
        /* Access next argument */
        arg = va_arg(vp, int);

        /* Format the argument */
        sprintf(tmp, aux, arg);

        /* Done */
        break;
      }

      /* Signed Integers -- standard format */
      case 'd': case 'i':
      {
        if (do_long)
        {
          long arg;
          
          /* Access next argument */
          arg = va_arg(vp, long);

          /* Format the argument */
          sprintf(tmp, aux, arg);
        }
        else
        {
          int arg;
          
          /* Access next argument */
          arg = va_arg(vp, int);

          /* Format the argument */
          sprintf(tmp, aux, arg);
        }

        /* Done */
        break;
      }

      /* Unsigned Integers -- various formats */
      case 'u': case 'o': case 'x': case 'X':
      {
        if (do_long)
        {
          unsigned long arg;

          /* Access next argument */
          arg = va_arg(vp, unsigned long);

          /* Format the argument */
          sprintf(tmp, aux, arg);
        }
        else
        {
          unsigned int arg;

          /* Access next argument */
          arg = va_arg(vp, unsigned int);

          /* Format the argument */
          sprintf(tmp, aux, arg);
        }

        /* Done */
        break;
      }

      /* Floating Point -- various formats */
      case 'f':
      case 'e': case 'E':
      case 'g': case 'G':
      {
        double arg;

        /* Access next argument */
        arg = va_arg(vp, double);

        /* Format the argument */
        sprintf(tmp, aux, arg);

        /* Done */
        break;
      }

      /* Pointer -- implementation varies */
      case 'p':
      {
        vptr arg;

        /* Access next argument */
        arg = va_arg(vp, vptr);

        /* Format the argument */
        sprintf(tmp, aux, arg);

        /* Done */
        break;
      }

      /* String */
      case 's':
      {
        cptr arg;

        /* Access next argument */
        arg = va_arg(vp, cptr);

        /* Hack -- convert NULL to EMPTY */
        if (!arg) arg = "";

        /* Format the argument */
        sprintf(tmp, aux, arg);

        /* Done */
        break;
      }

      /* User defined data */
      case 'V':
      case 'v':
      {
        vptr arg;
      
        /* Access next argument */
        arg = va_arg(vp, vptr);

        /* Format the "user data" */
        (void)vstrnfmt_aux(tmp, 1000, aux, arg);

        /* Done */
        break;
      }


      /* Oops */
      default:
      {
        /* Error -- illegal format char */
        buf[0] = '\0';

        /* Return "error" */
        return (0);
      }
    }


    /* Mega-Hack -- handle "capitilization" */
    if (do_xtra)
    {
      /* Now append "tmp" to "buf" */
      for (q = 0; tmp[q]; q++)
      {
        /* Notice first non-space */
        if (!isspace(tmp[q]))
        {
          /* Capitalize if possible */
          if (islower(tmp[q])) tmp[q] = toupper(tmp[q]);
          
          /* Done */
          break;
        }
      }
    }

    /* Now append "tmp" to "buf" */
    for (q = 0; tmp[q]; q++)
    {
      /* Check total length */
      if (n == max-1) break;

      /* Save the character */
      buf[n++] = tmp[q];
    }
  }


  /* Terminate buffer */
  buf[n] = '\0';

  /* Return length */
  return (n);
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


