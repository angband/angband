/*
 * File: z-file.c
 * Purpose: General purpose file handling.
 *
 *
 *
 * The concept of the file routines is that all file handling should be done
 * using as few routines as possible, since every machine is slightly
 * different, but these routines always have the same semantics.
 *
 * Prhaps we should use "path_parse()" to convert from "canonical" filenames
 * (optional leading tildes, internal wildcards, slash as the path seperator,
 * etc) to "system" filenames (no special symbols, system-specific path
 * seperator, etc).  This would allow the program itself to assume that all
 * filenames are "Unix" filenames, and explicitly "extract" such filenames if
 * needed (by "path_parse()", or perhaps "path_canon()"). XXX
 *
 * path_temp() should probably return a "canonical" filename.  XXX
 *
 * Note that "my_fopen()" and "my_open()" and "my_make()" and "my_kill()"
 * and "my_move()" and "my_copy()" should all take "canonical" filenames.
 *
 * Canonical filenames use a leading slash to indicate an absolute path, and a
 * leading tilde to indicate a special directory.  They default to a relative
 * path.  DOS/Windows uses a leading "drivename plus colon" to indicate the
 * use of a "special drive", and then the rest of the path is parsed normally,
 * and MACINTOSH uses a leading colon to indicate a relative path, and an
 * embedded colon to indicate a "drive plus absolute path", and finally
 * defaults to a file in the current working directory, which may or may
 * not be defined.
 */
#include "angband.h"
#include "z-file.h"

#ifndef RISCOS
# ifdef MACINTOSH
#  include <stat.h>
# else
#  include <sys/types.h>
#  include <sys/stat.h>
# endif /* MACINTOSH */
#endif



#ifdef HAVE_MKSTEMP

FILE *my_fopen_temp(char *buf, size_t max)
{
	int fd;

	/* Prepare the buffer for mkstemp */
	my_strcpy(buf, "/tmp/anXXXXXX", max);

	/* Secure creation of a temporary file */
	fd = mkstemp(buf);

	/* Check the file-descriptor */
	if (fd < 0) return (NULL);

	/* Return a file stream */
	return (fdopen(fd, "w"));
}

#else /* HAVE_MKSTEMP */

/*
 * Consider rewriting this so it uses its own buffer.
 */
FILE *my_fopen_temp(char *buf, size_t max)
{
	const char *s;

	/* Temp file */
	s = tmpnam(NULL);

	/* Oops */
	if (!s) return (NULL);

	/* Copy to buffer */
	my_strcpy(buf, s, max);

	/* Open the file */
	return (my_fopen(buf, "w"));
}

#endif /* HAVE_MKSTEMP */


/*
 * Hack -- replacement for "fgets()"
 *
 * Read a string, without a newline, to a file
 *
 * Process tabs, strip internal non-printables
 */
#define TAB_COLUMNS   8

errr my_fgets(FILE *fff, char *buf, size_t n)
{
	u16b i = 0;
	char *s = buf;
	int len;


	/* Paranoia */
	if (n <= 0) return (1);

	/* Enforce historical upper bound */
	if (n > 1024) n = 1024;

	/* Leave a byte for terminating null */
	len = n - 1;

	/* While there's room left in the buffer */
	while (i < len)
	{
		int c;

		/*
		 * Read next character - stdio buffers I/O, so there's no
		 * need to buffer it again using fgets.
		 */
		c = fgetc(fff);

		/* End of file */
		if (c == EOF)
		{
			/* No characters read -- signal error */
			if (i == 0) break;

			/*
			 * Be nice to DOS/Windows, where a last line of a file isn't
			 * always \n terminated.
			 */
			*s = '\0';

			/* Success */
			return (0);
		}

#if defined(MACINTOSH) || defined(MACH_O_CARBON)

		/*
		 * Be nice to the Macintosh, where a file can have Mac or Unix
		 * end of line, especially since the introduction of OS X.
		 * MPW tools were also very tolerant to the Unix EOL.
		 */
		if (c == '\r') c = '\n';

#endif /* MACINTOSH || MACH_O_CARBON */

		/* End of line */
		if (c == '\n')
		{
			/* Null terminate */
			*s = '\0';

			/* Success */
			return (0);
		}

		/* Expand a tab into spaces */
		if (c == '\t')
		{
			int tabstop;

			/* Next tab stop */
			tabstop = ((i + TAB_COLUMNS) / TAB_COLUMNS) * TAB_COLUMNS;

			/* Bounds check */
			if (tabstop >= len) break;

			/* Convert it to spaces */
			while (i < tabstop)
			{
				/* Store space */
				*s++ = ' ';

				/* Count */
				i++;
			}
		}

		/* Ignore non-printables */
		else if (isprint(c))
		{
			/* Store character in the buffer */
			*s++ = c;

			/* Count number of characters in the buffer */
			i++;
		}
	}

	/* Buffer overflow or EOF - return an empty string */
	buf[0] = '\0';

	/* Error */
	return (1);
}


/*
 * Hack -- replacement for "fputs()"
 *
 * Dump a string, plus a newline, to a file
 *
 * Perhaps this function should handle internal weirdness.
 */
errr my_fputs(FILE *fff, cptr buf, size_t n)
{
	/* Unused paramter */
	(void)n;

	/* Dump, ignore errors */
	(void)fprintf(fff, "%s\n", buf);

	/* Success */
	return (0);
}



/* The file routines for RISC OS are in main-ros.c. */
#ifndef RISCOS


#ifdef SET_UID

/*
 * Find a default user name from the system.
 */
void user_name(char *buf, size_t len, int id)
{
	struct passwd *pw;

	/* Look up the user name */
	if ((pw = getpwuid(id)))
	{
		/* Get the first 15 characters of the user name */
		my_strcpy(buf, pw->pw_name, len);

#ifdef CAPITALIZE_USER_NAME
		/* Hack -- capitalize the user name */
		if (islower((unsigned char)buf[0]))
			buf[0] = toupper((unsigned char)buf[0]);
#endif /* CAPITALIZE_USER_NAME */

		return;
	}

	/* Oops.  Hack -- default to "PLAYER" */
	my_strcpy(buf, "PLAYER", len);
}

#endif /* SET_UID */


#if defined(SET_UID) || defined(USE_PRIVATE_PATHS)

/*
 * Extract a "parsed" path from an initial filename
 * Normally, we simply copy the filename into the buffer
 * But leading tilde symbols must be handled in a special way
 * Replace "~user/" by the home directory of the user named "user"
 * Replace "~/" by the home directory of the current user
 */
errr path_parse(char *buf, size_t max, cptr file)
{
	cptr u, s;
	struct passwd	*pw;
	char user[128];


	/* Assume no result */
	buf[0] = '\0';

	/* No file? */
	if (!file) return (-1);

	/* File needs no parsing */
	if (file[0] != '~')
	{
		my_strcpy(buf, file, max);
		return (0);
	}

	/* Point at the user */
	u = file+1;

	/* Look for non-user portion of the file */
	s = strstr(u, PATH_SEP);

	/* Hack -- no long user names */
	if (s && (s >= u + sizeof(user))) return (1);

	/* Extract a user name */
	if (s)
	{
		int i;
		for (i = 0; u < s; ++i) user[i] = *u++;
		user[i] = '\0';
		u = user;
	}

	/* Look up the "current" user */
	if (u[0] == '\0') u = getlogin();

	/* Look up a user (or "current" user) */
	if (u) pw = getpwnam(u);
	else pw = getpwuid(getuid());

	/* Nothing found? */
	if (!pw) return (1);

	/* Make use of the info */
	my_strcpy(buf, pw->pw_dir, max);

	/* Append the rest of the filename, if any */
	if (s) my_strcat(buf, s, max);

	/* Success */
	return (0);
}


#else /* SET_UID */


/*
 * Extract a "parsed" path from an initial filename
 *
 * This requires no special processing on simple machines,
 * except for verifying the size of the filename.
 */
errr path_parse(char *buf, size_t max, cptr file)
{
	/* Accept the filename */
	my_strcpy(buf, file, max);

# ifdef MACH_O_CARBON

	/* Fix it according to the current operating system */
	convert_pathname(buf);

# endif

	/* Success */
	return (0);
}


#endif /* SET_UID */




/*
 * Create a new path by appending a file (or directory) to a path
 *
 * This requires no special processing on simple machines, except
 * for verifying the size of the filename, but note the ability to
 * bypass the given "path" with certain special file-names.
 *
 * Note that the "file" may actually be a "sub-path", including
 * a path and a file.
 *
 * Note that this function yields a path which must be "parsed"
 * using the "parse" function above.
 */
errr path_build(char *buf, size_t max, cptr path, cptr file)
{
	/* Special file */
	if (file[0] == '~')
	{
		/* Use the file itself */
		my_strcpy(buf, file, max);
	}

	/* Absolute file, on "normal" systems */
	else if (prefix(file, PATH_SEP) && !streq(PATH_SEP, ""))
	{
		/* Use the file itself */
		my_strcpy(buf, file, max);
	}

	/* No path given */
	else if (!path[0])
	{
		/* Use the file itself */
		my_strcpy(buf, file, max);
	}

	/* Path and File */
	else
	{
		/* Build the new path */
		strnfmt(buf, max, "%s%s%s", path, PATH_SEP, file);
	}

	/* Success */
	return (0);
}


/*
 * Hack -- replacement for "fopen()"
 */
FILE *my_fopen(cptr file, cptr mode)
{
	char buf[1024];
	FILE *fff;

	/* Hack -- Try to parse the path */
	if (path_parse(buf, sizeof(buf), file)) return (NULL);

	/* Attempt to fopen the file anyway */
	fff = fopen(buf, mode);

#if defined(MAC_MPW) || defined(MACH_O_CARBON)

	/* Set file creator and type */
	if (fff && strchr(mode, 'w')) fsetfileinfo(buf, _fcreator, _ftype);

#endif

	/* Return open file or NULL */
	return (fff);
}


/*
 * Hack -- replacement for "fclose()"
 */
errr my_fclose(FILE *fff)
{
	/* Require a file */
	if (!fff) return (-1);

	/* Close, check for error */
	if (fclose(fff) == EOF) return (1);

	/* Success */
	return (0);
}


/*
 * Hack -- attempt to delete a file
 */
errr fd_kill(cptr file)
{
	char buf[1024];

	/* Hack -- Try to parse the path */
	if (path_parse(buf, sizeof(buf), file)) return (-1);

	/* Remove, return 0 on success, non-zero on failure */
	return (remove(buf));
}


/*
 * Hack -- attempt to move a file
 */
errr fd_move(cptr file, cptr what)
{
	char buf[1024];
	char aux[1024];

	/* Hack -- Try to parse the path */
	if (path_parse(buf, sizeof(buf), file)) return (-1);

	/* Hack -- Try to parse the path */
	if (path_parse(aux, sizeof(aux), what)) return (-1);

	/* Rename, return 0 on success, non-zero on failure */
	return (rename(buf, aux));
}


/*
 * Hack -- attempt to open a file descriptor (create file)
 *
 * This function should fail if the file already exists
 *
 * Note that we assume that the file should be "binary"
 */
int fd_make(cptr file, int mode)
{
	char buf[1024];
	int fd;

	/* Hack -- Try to parse the path */
	if (path_parse(buf, sizeof(buf), file)) return (-1);

#if defined(MACINTOSH)

	/* Create the file, fail if exists, write-only, binary */
	fd = open(buf, O_CREAT | O_EXCL | O_WRONLY | O_BINARY);

#else

	/* Create the file, fail if exists, write-only, binary */
	fd = open(buf, O_CREAT | O_EXCL | O_WRONLY | O_BINARY, mode);

#endif

#if defined(MAC_MPW) || defined(MACH_O_CARBON)

	/* Set file creator and type */
	if (fd >= 0) fsetfileinfo(buf, _fcreator, _ftype);

#endif

	/* Return descriptor */
	return (fd);
}


/*
 * Hack -- attempt to open a file descriptor (existing file)
 *
 * Note that we assume that the file should be "binary"
 */
int fd_open(cptr file, int flags)
{
	char buf[1024];

	/* Hack -- Try to parse the path */
	if (path_parse(buf, sizeof(buf), file)) return (-1);

#if defined(MACINTOSH) || defined(WINDOWS)

	/* Attempt to open the file */
	return (open(buf, flags | O_BINARY));

#else

	/* Attempt to open the file */
	return (open(buf, flags | O_BINARY, 0));

#endif

}


/*
 * Hack -- attempt to lock a file descriptor
 *
 * Legal lock types -- F_UNLCK, F_RDLCK, F_WRLCK
 */
errr fd_lock(int fd, int what)
{
#ifdef SET_UID

	struct flock lock;
#endif

	/* Verify the fd */
	if (fd < 0) return (-1);

#ifdef SET_UID

	lock.l_type = what;
	lock.l_start = 0; /* Lock the entire file */
	lock.l_whence = SEEK_SET; /* Lock the entire file */
	lock.l_len = 0; /* Lock the entire file */

	/* Wait for access and set lock status */
	/*
	 * Change F_SETLKW to G_SETLK if it's preferable to return
	 * without locking and reporting an error instead of waiting.
	 */
	return (fcntl(fd, F_SETLKW, &lock));


#else /* SET_UID */

	/* Unused parameter */
	(void)what;

#endif /* SET_UID */

	/* Success */
	return (0);
}


/*
 * Hack -- attempt to seek on a file descriptor
 */
errr fd_seek(int fd, long n)
{
	long p;

	/* Verify fd */
	if (fd < 0) return (-1);

	/* Seek to the given position */
	p = lseek(fd, n, SEEK_SET);

	/* Failure */
	if (p < 0) return (1);

	/* Failure */
	if (p != n) return (1);

	/* Success */
	return (0);
}


#ifndef SET_UID
#define FILE_BUF_SIZE 16384
#endif


/*
 * Hack -- attempt to read data from a file descriptor
 */
errr fd_read(int fd, char *buf, size_t n)
{
	/* Verify the fd */
	if (fd < 0) return (-1);

#ifndef SET_UID

	/* Read pieces */
	while (n >= FILE_BUF_SIZE)
	{
		/* Read a piece */
		if (read(fd, buf, FILE_BUF_SIZE) != FILE_BUF_SIZE) return (1);

		/* Shorten the task */
		buf += FILE_BUF_SIZE;

		/* Shorten the task */
		n -= FILE_BUF_SIZE;
	}

#endif

	/* Read the final piece */
	if (read(fd, buf, n) != (int)n) return (1);

	/* Success */
	return (0);
}


/*
 * Hack -- Attempt to write data to a file descriptor
 */
errr fd_write(int fd, cptr buf, size_t n)
{
	/* Verify the fd */
	if (fd < 0) return (-1);

#ifndef SET_UID

	/* Write pieces */
	while (n >= FILE_BUF_SIZE)
	{
		/* Write a piece */
		if (write(fd, buf, FILE_BUF_SIZE) != FILE_BUF_SIZE) return (1);

		/* Shorten the task */
		buf += FILE_BUF_SIZE;

		/* Shorten the task */
		n -= FILE_BUF_SIZE;
	}

#endif

	/* Write the final piece */
	if (write(fd, buf, n) != (int)n) return (1);

	/* Success */
	return (0);
}


/*
 * Hack -- attempt to close a file descriptor
 */
errr fd_close(int fd)
{
	/* Verify the fd */
	if (fd < 0) return (-1);

	/* Close, return 0 on success, -1 on failure */
	return (close(fd));
}


#if defined(CHECK_MODIFICATION_TIME) && !defined(MAC_MPW)

errr check_modification_date(int fd, cptr template_file)
{
	char buf[1024];

	struct stat txt_stat, raw_stat;

	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_EDIT, template_file);

	/* Access stats on text file */
	if (stat(buf, &txt_stat))
	{
		/* No text file - continue */
	}

	/* Access stats on raw file */
	else if (fstat(fd, &raw_stat))
	{
		/* Error */
		return (-1);
	}

	/* Ensure text file is not newer than raw file */
	else if (txt_stat.st_mtime > raw_stat.st_mtime)
	{
		/* Reprocess text file */
		return (-1);
	}

	return (0);
}

#endif /* CHECK_MODIFICATION_TIME */

#endif /* RISCOS */

