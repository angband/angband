#include <stdio.h>


/*
 * The current "tab stop"
 */
int tab_stop = 8;

/*
 * Process "internal" tabs?
 *
 * Dangerous because of tabs/spaces inside strings
 */
int internal = 0;


/*
 * Convert tabs to spaces
 * Also, force final newline
 */
int detab()
{
  int i;

  while (1)
  {
    int new = 1;
    int col = 0;

    while (1)
    {
      i = getchar();

      if (i == EOF)
      {
	if (col) putchar ('\n');
	return (0);
      }


      if (i == '\n') break;

      /* Handle tabs */
      if (i == '\t')
      {
	/* Process "leading" tabs, or "all tabs" */
	if (new || internal) 
	{
	  do { putchar(' '); col++; } while (col % tab_stop);
	}
	else
	{
	  putchar(i); do { col++; } while (col % tab_stop);
	}
      }
      else
      {
	/* Convert "bad chars" */
	if (!isprint(i)) i = '?';

	/* Notice when inside a line */
	if (i != ' ') new = 0;

	/* Drop the char */
	putchar(i); col++;
      }
    }

    /* Newline */
    putchar('\n');
  }
}



/*
 * Convert spaces to tabs
 * Also, strip spaces on blank lines
 * Also, force final newline
 */
int entab()
{
  int i;

  while (1)
  {
    int new = 1;
    int col = 0;
    int src = 0;

    while (1)
    {
      i = getchar();

      if (i == EOF)
      {
	if (col) putchar ('\n');
	return (0);
      }

      if (i == '\n') break;

      /* Handle leading (or all) spaces */
      if ((i == ' ') && (new || internal))
      {
	/* Advance the source column */
	src++;
      }

      /* Handle leading (or all) tabs */
      else if ((i == '\t') && (new || internal))
      {
	/* Advance the source column one tab stop */
	do { src++; } while (src % tab_stop);
      }

      /* Flush spaces and continue */
      else
      {
	/* Flush indentation */
	while (col < src)
	{
	  /* Compute the next tab stop */
	  int k = col; do { k++; } while (k % tab_stop);

	  /* Use leading (or all) tabs (if replacing at least 2 spaces) */
	  if ((col + 1 < src) && (new || internal) && (k <= col))
	  {
	    putchar('\t'); col = i;
	  }

	  /* Fill it out with spaces */
	  else
	  {
	    putchar(' '); col++;
	  }
	}

	/* Handle internal tabs */
	if (i == '\t')
	{
	  putchar('\t');
	  do { col++; } while (col % tab_stop);
	}

	/* Normal chars */
	else
	{
	  /* Convert "bad chars" */
	  if (!isprint(i)) i = '?';

	  /* Notice when inside a line */
	  if (i != ' ') new = 0;

	  /* Drop the char */
	  putchar(i); col++;
	}
      }
    }

    /* Newline */
    putchar('\n');
  }
}



/* 
 * Main function
 *
 * Do NOT currently have an option for converting all spaces to tabs
 * EXCEPT those which are inside strings.  Thus, it is probably best
 * to NOT use the "internal" option for "entab()" yet.
 */
int main(int argc, char *argv[])
{
  int task = 0;

  if (argc != 2)
  {
    printf("Usage: %s -d/-e\n", argv[0]);
    return (-1);
  }

  if (argv[1][0] != '-')
  {
    printf("Usage: %s -d/-e\n", argv[0]);
    return (-2);
  }

  task = argv[1][1];


  if (task == 'd')
  {
    return (detab());
  }

  if (task == 'D')
  {
    internal = 1;
    return (detab());
  }

  if (task == 'e')
  {
    return (entab());
  }

  if (task == 'E')
  {
    internal = 1;
    return (entab());
  }


  printf("Usage: %s -d/-e\n", argv[0]);
  return (0);
}


