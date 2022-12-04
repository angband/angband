/**
 * \file z-file.c
 * \brief Low-level file (and directory) handling
 *
 * Copyright (c) 1997-2007 Ben Harrison, pelpel, Andi Sidwell, Matthew Jones
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
#include "h-basic.h"
#include "z-file.h"
#include "z-form.h"
#include "z-rand.h"
#include "z-util.h"
#include "z-virt.h"


#include <sys/types.h>

#ifdef WINDOWS
# include <windows.h>
# include <io.h>
# ifndef CYGWIN
#  include <direct.h>
# endif
#endif

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

#if defined (HAVE_DIRENT_H) || defined (CYGWIN)
# include <sys/types.h>
# include <dirent.h>
#endif

#ifdef HAVE_STAT
# include <sys/stat.h>
# include <sys/types.h>
#endif

#if defined (WINDOWS) && !defined (CYGWIN)
# define my_mkdir(path, perms) mkdir(path)
#elif defined(HAVE_MKDIR) || defined(MACH_O_CARBON) || defined (CYGWIN) || defined(NDS)
# define my_mkdir(path, perms) mkdir(path, perms)
#else
# define my_mkdir(path, perms) false
#endif

/* Suppress MSC C4996 error */
#if defined(_MSC_VER)
#define open _open
#define fdopen _fdopen
#define mkdir _mkdir
#endif

/**
 * Player info
 */
int player_uid;
int player_egid;




/**
 * Drop permissions
 */
void safe_setuid_drop(void)
{
#ifdef SETGID
# if defined(HAVE_SETRESGID)

	if (setresgid(-1, getgid(), -1) != 0)
		quit("setegid(): cannot drop permissions correctly!");

# else

	if (setegid(getgid()) != 0)
		quit("setegid(): cannot drop permissions correctly!");

# endif
#endif /* SETGID */
}


/**
 * Grab permissions
 */
void safe_setuid_grab(void)
{
#ifdef SETGID
# if defined(HAVE_SETRESGID)

	if (setresgid(-1, player_egid, -1) != 0)
		quit("setegid(): cannot grab permissions correctly!");

# elif defined(HAVE_SETEGID)

	if (setegid(player_egid) != 0)
		quit("setegid(): cannot grab permissions correctly!");

# endif
#endif /* SETGID */
}




/**
 * Apply special system-specific processing before dealing with a filename.
 */
static void path_parse(char *buf, size_t max, const char *file)
{
	/* Accept the filename */
	my_strcpy(buf, file, max);
}


static void path_process(char *buf, size_t len, size_t *cur_len,
						 const char *path)
{
#if defined(UNIX)

	/* Home directory on Unixes */
	if (path[0] == '~') {
		const char *s;
		const char *username = path + 1;

		struct passwd *pw;
		char user[128];

		/* Look for non-user portion of the file */
		s = strstr(username, PATH_SEP);
		if (s) {
			int i;

			/* Keep username a decent length */
			if (s >= username + sizeof(user)) return;

			for (i = 0; username < s; ++i) user[i] = *username++;
			user[i] = '\0';
			username = user;
		}

		/* Look up a user (or "current" user) */
		pw = username[0] ? getpwnam(username) : getpwuid(getuid());
		if (!pw) return;

		/* Copy across */
		strnfcat(buf, len, cur_len, "%s%s", pw->pw_dir, PATH_SEP);
		if (s) strnfcat(buf, len, cur_len, "%s", s);
	} else

#endif /* defined(UNIX) */

		strnfcat(buf, len, cur_len, "%s", path);
}



/**
 * Create a new path string by appending a 'leaf' to 'base'.
 *
 * On Unixes, we convert a tidle at the beginning of a basename to mean the
 * directory, complicating things a little, but better now than later.
 */
size_t path_build(char *buf, size_t len, const char *base, const char *leaf)
{
	size_t cur_len = 0;
	int starts_with_separator;

	buf[0] = '\0';

	if (!leaf || !leaf[0]) {
		if (base && base[0])
			path_process(buf, len, &cur_len, base);

		return cur_len;
	}


	/*
	 * If the leafname starts with the separator,
	 *   or with the tilde (on Unix),
	 *   or there's no base path,
	 * We use the leafname only.
	 */
	starts_with_separator = (!base || !base[0]) || prefix(leaf, PATH_SEP);
#if defined(UNIX)
	starts_with_separator = starts_with_separator || leaf[0] == '~';
#endif
	if (starts_with_separator) {
		path_process(buf, len, &cur_len, leaf);
		return cur_len;
	}


	/* There is both a relative leafname and a base path from which it is
	 * relative */
	path_process(buf, len, &cur_len, base);

	if (!suffix(base, PATH_SEP)) {
		/* Append separator if it isn't already in the string. */
		strnfcat(buf, len, &cur_len, "%s", PATH_SEP);
	}

	path_process(buf, len, &cur_len, leaf);

	return cur_len;
}

/**
 * Converts path_in to an absolute path without redundant path separators.
 *
 * \param buf is the buffer to hold the result.  If len is greater than zero,
 * it will always be null terminated.  buf may be NULL if len is zero.
 * \param len is the size of the buffer.
 * \param path_in is the null-terminated string with the path to normalize.
 * \param trailing_sep will, if true, cause the normalized path to have a
 * trailing path separator if it does not already have one.
 * \param req_len if not NULL, will be dereferenced and set to the required
 * buffer size needed to hold the normalized result or to zero if it was not
 * possible to compute the normalized result.
 * \param root_len if not NULL, will be dereferenced and set to the length
 * of the root portion (the part that will always remain after repeatedly
 * going up the directory tree) of the path or to zero if it was not possible
 * to compute the normalized result.
 * \return zero if the normalization operation succeeded and the untruncated
 * result was stored in buf.  Returns 1 if the normalized result was
 * truncated.  Returns 2 if the normalized result could not be determined.
 *
 * On Unixes, deals with the tilde as representing home directories.
 */
int path_normalize(char *buf, size_t len, const char *path_in,
		bool trailing_sep, size_t *req_len, size_t *root_len)
{
	size_t oidx_high = 0, oidx, iidx, root_size;
	int result;

#ifdef WINDOWS
	/*
	 * See https://learn.microsoft.com/en-us/dotnet/standard/io/file-path-formats
	 * for a primer on path formats used by Windows systems.
	 *
	 * The use of GetCurrentDirectory() below assumes the code is
	 * compiled without the _UNICODE and _MBCS preprocessor directives
	 * set.
	 */
	if (path_in[0] == PATH_SEPC) {
		if (path_in[1] == PATH_SEPC) {
			/*
			 * It's an UNC path ("\\<server_name>\<sharename>\...".
			 */
			const char *s = strchr(path_in + 2, PATH_SEPC);

			if (!s || !(s = strchr(s + 1, PATH_SEPC))) {
				/*
				 * It's an incomplete UNC path:  missing a
				 * share name or the path separator to terminate
				 * the share name.  Give up.
				 */
				goto ABNORMAL_RETURN;
			}
			iidx = (s - path_in) + 1;
			if (len > 0) {
				(void) memcpy(buf, path_in, (iidx < len) ?
					iidx : len);
			}
			root_size = iidx;
			oidx = iidx;
		} else {
			/*
			 * It's a path from the root of the current drive
			 * ("\my_path_here").  Get the current working
			 * directory, but only the root portion is needed.
			 */
			DWORD lwd = GetCurrentDirectory(0, NULL);
			LPTSTR work = mem_alloc((lwd + 1) * sizeof(*work));

			/*
			 * The working directory changed to a longer name
			 * (presumably from another thread) or is not long
			 * enough to be an UNC path or absolute DOS path.
			 * Give up.
			 */
			if (GetCurrentDirectory(lwd, work) > lwd || lwd < 3) {
				free(work);
				goto ABNORMAL_RETURN;
			}
			if (work[0] == PATH_SEPC) {
				/* Root part should look like an UNC PATH. */
				char *s;

				if (work[1] != PATH_SEPC
						|| !(s = strchr(work + 2, PATH_SEPC))
						|| !(s = strchr(s + 1, PATH_SEPC))) {
					/*
					 * It doesn't look like an UNC path.
					 * Give up.
					 */
					free(work);
					goto ABNORMAL_RETURN;
				}
				lwd = (s + 1) - work;
			} else if (work[1] == ':' && work[2] == PATH_SEPC) {
				/* Root part looks like an absolute DOS path. */
				lwd = 3;
			} else {
				/*
				 * It's neither an UNC path nor an absolute
				 * DOS path.  Give up.
				 */
				free(work);
				goto ABNORMAL_RETURN;
			}
			if (len > 0) {
				(void) memcpy(buf, work, (lwd < len) ?
					lwd : len);
			}
			free(work);
			oidx = lwd;
			iidx = 1;
			root_size = lwd;
		}
	} else if (path_in[0] && path_in[1] == ':') {
		/* It's a DOS path with a drive letter. */
		if (path_in[2] == PATH_SEPC) {
			/* It's absolute. */
			if (len > 0) {
				(void) memcpy(buf, path_in, (3 < len) ?
					3 : len);
			}
			oidx = 3;
			iidx = 3;
		} else {
			/*
			 * It's relative to the current directory on that drive.
			 */
			DWORD lwd = GetCurrentDirectory(0, NULL);
			LPTSTR work = mem_alloc((lwd + 1) * sizeof(*work));

			/*
			 * Either the working directory changed to a longer
			 * name (presumably from another thread) or the working
			 * directory does not start with the given drive
			 * letter + ':' + PATH_SEPC which is what's expected
			 * for an absolute DOS path on that drive.  Give up.
			 */
			if (GetCurrentDirectory(lwd, work) > lwd
					|| lwd < 3 || work[0] != path_in[0]
					|| work[1] != ':'
					|| work[2] != PATH_SEPC) {
				free(work);
				goto ABNORMAL_RETURN;
			}
			/*
			 * Process it through path_normalize() so any
			 * redundant path separators or interstitial relative
			 * parts in the working directory are handled.
			 */
			result = path_normalize(buf, len, work,
				(path_in[3]) ? true : false, &oidx_high,
				&root_size);
			free(work);
			assert(root_size == 3);
			if ((result != 0 && result != 1)
					|| oidx_high <= root_size) {
				goto ABNORMAL_RETURN;
			}
			/* Backup over the trailing null. */
			oidx = oidx_high - 1;
			iidx = 2;
		}
		root_size = 3;
	} else {
		/* It's a path relative to the current working directory. */
		DWORD lwd = GetCurrentDirectory(0, NULL);
		LPTSTR work = mem_alloc((lwd + 1) * sizeof(*work));

		/*
		 * Either the working directory changed to a longer
		 * name (presumably from another thread) or is not long
		 * enough to be an UNC path or an absoloute DOS path.  Give up.
		 */
		if (GetCurrentDirectory(lwd, work) > lwd || lwd < 3) {
			free(work);
			goto ABNORMAL_RETURN;
		}
		if (work[0] == PATH_SEPC) {
			/* Root part should look like an UNC PATH. */
			char *s;

			if (work[1] != PATH_SEPC
					|| !(s = strchr(work + 2, PATH_SEPC))
					|| !(s = strchr(s + 1, PATH_SEPC))) {
				/*
				 * It doesn't look like an UNC path.  Give up.
				 */
				free(work);
				goto ABNORMAL_RETURN;
			}
		} else if (work[1] != ':' || work[2] != PATH_SEPC) {
			/*
			 * It's neither an UNC path nor an absolute DOS path.
			 * Give up.
			 */
			free(work);
			goto ABNORMAL_RETURN;
		}
		/*
		 * Process it through path_normalize() so any redundant path
		 * separators or interstitial relative parts in the working
		 * directory are handled.
		 */
		result = path_normalize(buf, len, work,
			(path_in[0]) ? true : false, &oidx_high, &root_size);
		free(work);
		assert(root_size >= 3);
		if ((result != 0 && result != 1) || oidx_high <= root_size) {
			goto ABNORMAL_RETURN;
		}
		/* Backup over the trailing null. */
		oidx = oidx_high - 1;
		iidx = 0;
	}
#elif defined(UNIX)
	if (path_in[0] != PATH_SEPC) {
		/*
		 * Input path is relative to the current working directory
		 * or in a user's home directory.
		 */
		if (path_in[0] == '~') {
			/* It's in a user's home directory. */
			struct passwd *pw;

			if (!path_in[1] || path_in[1] == PATH_SEPC) {
				/*
				 * It's not qualified by a user name, use
				 * the current user.
				 */
				pw = getpwuid(getuid());
				iidx = 1;
			} else {
				const char *s = strchr(path_in + 1, PATH_SEPC);

				if (s) {
					size_t nlen = s - path_in;
					char *work = mem_alloc(nlen);

					(void) memcpy(work, path_in + 1,
						nlen - 1);
					work[nlen - 1] = '\0';
					pw = getpwnam(work);
					mem_free(work);
					iidx = nlen;
				} else {
					pw = getpwnam(path_in + 1);
					iidx = strlen(path_in);
				}
			}
			/*
			 * If that user's directory isn't available or it
			 * begins with '~', give up.
			 */
			if (!pw || !pw->pw_dir || pw->pw_dir[0] == '~') {
				goto ABNORMAL_RETURN;
			}
			/*
			 * Process the user's directory through path_normalize()
			 * to guarantee that it is normalized.
			 */
			result = path_normalize(buf, len, pw->pw_dir,
				(path_in[iidx]) ? true : false, &oidx_high,
				NULL);
			if ((result != 0 && result != 1) || oidx_high <= 1) {
				goto ABNORMAL_RETURN;
			}
			/* Backup over the trailing null. */
			oidx = oidx_high - 1;
			/*
			 * Skip the '/' in the input since it's already been
			 * passed to the output.
			 */
			if (path_in[iidx]) {
				assert(path_in[iidx] == PATH_SEPC);
				++iidx;
			}
		} else {
			/*
			 * It's relative to the current working directory.
			 * Put the current working directory at the start
			 * of the output.
			 */
			size_t work_sz = 1024;
			char *work = mem_alloc(work_sz);

			/*
			 * Determine what size of buffer is necessary to
			 * hold the working directory.
			 */
			while (1) {
				if (getcwd(work, work_sz)) {
					/*
					 * getcwd() is supposed to return an
					 * absolute path name.  Process it
					 * through path_normalize() in case
					 * doesn't exclude having redundant
					 * path separators or things like "/./"
					 * or "/../" in it.
					 */
					if (work[0] != PATH_SEPC) {
						/*
						 * Got a relative path.  Give
						 * up.
						 */
						mem_free(work);
						goto ABNORMAL_RETURN;
					}
					result = path_normalize(buf, len, work,
						(path_in[0]) ? true : false,
						&oidx_high, NULL);
					mem_free(work);
					if ((result != 0 && result != 1)
							|| oidx_high <= 1) {
						goto ABNORMAL_RETURN;
					}
					/* Backup over the trailing null. */
					oidx = oidx_high - 1;
					break;
				} else if (errno != ERANGE) {
					/*
					 * Increasing the buffer size won't
					 * help resolve the error so give up.
					 */
					mem_free(work);
					goto ABNORMAL_RETURN;
				}
				if (work_sz < ((size_t) -1) / 2) {
					work_sz *= 2;
				} else if (work_sz < (size_t) -1) {
					work_sz = (size_t) -1;
				} else {
					/* Give up. */
					mem_free(work);
					goto ABNORMAL_RETURN;
				}
				work = mem_realloc(work, work_sz);
			}
			iidx = 0;
		}
	} else {
		/* Copy the leading path separator. */
		if (len) {
			buf[0] = PATH_SEPC;
		}
		oidx = 1;
		iidx = 1;
	}
	root_size = 1;
#else
	/*
	 * It's neither Windows nor Unix and don't know how to get the working
	 * directory so reject any paths that aren't absolute.
	 */
	if (path_in[0] != PATH_SEPC) {
		goto ABNORMAL_RETURN;
	}
	/* Copy the leading path separator. */
	if (len) {
		buf[0] = PATH_SEPC;
	}
	oidx = 1;
	iidx = 1;
	root_size = 1;
#endif /* ifdef WINDOWS else block */

	while (1) {
		if (!path_in[iidx]) {
			break;
		}
		/*
		 * The output path generated so for should end with a path
		 * separator, unless it was necessary to truncate the result.
		 */
		assert((oidx <= len && oidx > 0 && buf[oidx - 1] == PATH_SEPC)
			|| (oidx >= len));
		if (path_in[iidx] == PATH_SEPC) {
			/* Strip redundant path separator. */
			++iidx;
			continue;
		}
		if (path_in[iidx] == '.') {
			if (!path_in[iidx + 1]) {
				/*
				 * Trailing, PATH_SEPC + "." strips off the
				 * PATH_SEPC, if possible.
				 */
				if (oidx > root_size) {
					/*
					 * Remember what the size was before
					 * removing the path separator since
					 * that can affect the length of the
					 * buffer needed to for the
					 * normalization.
					 */
					oidx_high = MAX(oidx, oidx_high);
					--oidx;
					++iidx;
				}
				break;
			}
			if (path_in[iidx + 1] == PATH_SEPC) {
				/*
				 * Interstitial PATH_SEPC + "." + PATH_SEPC
				 * turns into PATH_SEPC which has already been
				 * transferred to the output.
				 */
				iidx += 2;
				continue;
			}
			if (path_in[iidx + 1] == '.') {
				if (!path_in[iidx + 2] || path_in[iidx + 2]
						== PATH_SEPC) {
					/*
					 * Trailing or interstitial PATH_SEPC +
					 * ".." + PATH_SEPC removes the last
					 * path component (if already at the
					 * root of the path, remain there).  In
					 * this implementation, don't try to
					 * remove that component if have already
					 * exceeded the size of the output
					 * buffer.
					 */
					if (oidx > root_size && oidx <= len) {
						/*
						 * Remember what the size was
						 * before removing the last
						 * component since that can
						 * influence what buffer
						 * size is needed to do the
						 * normalization with this
						 * implementation.
						 */
						oidx_high = MAX(oidx,
							oidx_high);
						/*
						 * Back over the trailing path
						 * separator already in the
						 * output buffer.
						 */
						--oidx;
						/*
						 * Remove the last path
						 * component.  Leaving a
						 * trailing separator.
						 */
						while (1) {
							assert(oidx > 0);
							if (buf[oidx - 1] == PATH_SEPC) {
								break;
							}
							--oidx;
						}
					}
					if (!path_in[iidx + 2]) {
						/*
						 * Since ".." is at the end of
						 * the path, remove the trailing
						 * separator, unless that
						 * would cut into the root
						 * portion.
						 */
						if (oidx > root_size) {
							--oidx;
							/*
							 * Leave at the end
							 * of the string for
							 * handling of
							 * trailing_sep.
							 */
							iidx += 2;
						}
						break;
					}
					/* Skip over the ".." + PATH_SEPC. */
					iidx += 3;
					continue;
				}
			}
		}

		/*
		 * Copy up to and including the next path separator or the
		 * end, whichever comes first.
		 */
		while (1) {
			if (!path_in[iidx]) {
				break;
			}
			if (path_in[iidx] == PATH_SEPC) {
				if (oidx < len) {
					buf[oidx] = path_in[iidx];
				}
				++oidx;
				++iidx;
				break;
			}
			if (oidx < len) {
				buf[oidx] = path_in[iidx];
			}
			++oidx;
			++iidx;
		}
	}

	/*
	 * Add trailing path separator if requested, needed, and there's room.
	 */
	if (trailing_sep) {
		/*
		 * The input doesn't end in a path separator so the output
		 * so far won't have one at the end as well.
		 */
		if (iidx == 0 || path_in[iidx - 1] != PATH_SEPC) {
			if (oidx < len) {
				buf[oidx] = PATH_SEPC;
			}
			++oidx;
		}
	}

	/* Guarantee null termination if len is greater than zero. */
	if (oidx < len) {
		buf[oidx] = '\0';
		result = 0;
	} else {
		if (len > 0) {
			buf[len - 1] = '\0';
		}
		result = 1;
	}
	++oidx;

	if (req_len) {
		*req_len = MAX(oidx, oidx_high);
	}
	if (root_len) {
		*root_len = root_size;
	}

	return result;

ABNORMAL_RETURN:
	if (len > 0) {
		buf[0] = '\0';
	}
	if (req_len) {
		*req_len = 0;
	}
	if (root_len) {
		*root_len = 0;
	}
	return 2;
}

/**
 * Return the index of the filename in a path, using PATH_SEPC. If no path
 * separator is found, return 0.
 */
size_t path_filename_index(const char *path)
{
	int i;

	if (strlen(path) == 0)
		return 0;

	for (i = strlen(path) - 1; i >= 0; i--) {
		if (path[i] == PATH_SEPC)
			return i + 1;
	}

	return 0;
}

/**
 * ------------------------------------------------------------------------
 * File-handling API
 * ------------------------------------------------------------------------ */


/* Some defines for compatibility between various build platforms */
#ifndef S_IRUSR
#define S_IRUSR S_IREAD
#endif

#ifndef S_IWUSR
#define S_IWUSR S_IWRITE
#endif

/* if the flag O_BINARY is not defined, it is not needed , but we still
 * need it defined so it will compile */
#ifndef O_BINARY
#define O_BINARY 0
#endif

/* Avoid a compiler warning when cross compiling for windows */
#ifdef __STRICT_ANSI__
FILE *fdopen(int handle, const char *mode);
#endif

/* Private structure to hold file pointers and useful info. */
struct ang_file
{
	FILE *fh;
	char *fname;
	file_mode mode;
};



/** Utility functions **/

/**
 * Delete file 'fname'.
 */
bool file_delete(const char *fname)
{
	char buf[1024];

	/* Get the system-specific paths */
	path_parse(buf, sizeof(buf), fname);

	return (remove(buf) == 0);
}

/**
 * Move file 'fname' to 'newname'.
 */
bool file_move(const char *fname, const char *newname)
{
	char buf[1024];
	char aux[1024];

	/* Get the system-specific paths */
	path_parse(buf, sizeof(buf), fname);
	path_parse(aux, sizeof(aux), newname);

	return (rename(buf, aux) == 0);
}


/**
 * Decide whether a file exists or not.
 */

#if defined(HAVE_STAT)

bool file_exists(const char *fname)
{
	struct stat st;
	return (stat(fname, &st) == 0);
}

#elif defined(WINDOWS)

bool file_exists(const char *fname)
{
	char path[MAX_PATH];
	DWORD attrib;

	/* API says we mustn't pass anything larger than MAX_PATH */
	my_strcpy(path, fname, sizeof(path));

	attrib = GetFileAttributes(path);
	if (attrib == INVALID_FILE_NAME) return false;
	if (attrib & FILE_ATTRIBUTE_DIRECTORY) return false;

	return true;
}

#else

bool file_exists(const char *fname)
{
	ang_file *f = file_open(fname, MODE_READ, 0);

	if (f) file_close(f);
	return (f ? true : false);
}

#endif

/**
 * Set filename to a new filename based on an existing filename, using
 * the specified file extension.  Make it shorter than the specified
 * maximum length.  Resulting filename doesn't usually exist yet.
 */
void file_get_savefile(char *filename, size_t max, const char *base,
	const char *ext)
{
	int count = 0;

	count = 0;

#ifdef DJGPP
	/* DOS needs a shorter file name */
	strnfmt(filename, max, "%s/temp.%s", dirname(base), ext);
	while (file_exists(filename) && (count++ < 100))
		strnfmt(filename, max, "%s/temp%u.%s", dirname(base), count,
			ext);
#else
	strnfmt(filename, max, "%s%lu.%s",
		base, (unsigned long)Rand_simple(1000000), ext);
	while (file_exists(filename) && (count++ < 100))
		strnfmt(filename, max, "%s%lu%u.%s",
			base, (unsigned long)Rand_simple(1000000), count, ext);
#endif /* ! DJGPP */
	return;
}

/**
 * Set filename to a new filename based on an existing filename, using
 * the specified file extension.  Make it shorter than the specified
 * maximum length.
 */
void file_get_tempfile(char *filename, size_t max, const char *base,
	const char *ext)
{
#ifdef DJGPP
	/* DOS needs a shorter file name */
        strnfmt(filename, max, "%s/temp.%s", dirname(base), ext);
#else
        strnfmt(filename, max, "%s.%s", base, ext);
#endif
	return;
}

/**
 * Return true if first is newer than second, false otherwise.
 */
bool file_newer(const char *first, const char *second)
{
#ifdef HAVE_STAT
	struct stat stat1, stat2;

	/* If the first doesn't exist, the first is not newer. */
	if (stat(first, &stat1) != 0) return false;

	/* If the second doesn't exist, the first is always newer. */
	if (stat(second, &stat2) != 0) return true;

	/* Compare modification times. */
	return stat1.st_mtime > stat2.st_mtime ? true : false;
#else /* HAVE_STAT */
	return false;
#endif /* !HAVE_STAT */
}




/** File-handle functions **/

void (*file_open_hook)(const char *path, file_type ftype);

/**
 * Open file 'fname', in mode 'mode', with filetype 'ftype'.
 * Returns file handle or NULL.
 */
ang_file *file_open(const char *fname, file_mode mode, file_type ftype)
{
	ang_file *f = mem_zalloc(sizeof(ang_file));
	char buf[1024];

	(void)ftype;

	/* Get the system-specific path */
	path_parse(buf, sizeof(buf), fname);

	switch (mode) {
		case MODE_WRITE: { 
			if (ftype == FTYPE_SAVE) {
				/* open only if the file does not exist */
				int fd;
				fd = open(buf, O_CREAT | O_EXCL | O_WRONLY | O_BINARY, S_IRUSR | S_IWUSR);
				if (fd < 0) {
					/* there was some error */
					f->fh = NULL;
				} else {
					f->fh = fdopen(fd, "wb");
				}
			} else {
				f->fh = fopen(buf, "wb");
			}
			break;
		}
		case MODE_READ:
			f->fh = fopen(buf, "rb");
			break;
		case MODE_APPEND:
			f->fh = fopen(buf, "a+");
			break;
		default:
			assert(0);
	}

	if (f->fh == NULL) {
		mem_free(f);
		return NULL;
	}

	f->fname = string_make(buf);
	f->mode = mode;

	if (mode != MODE_READ && file_open_hook)
		file_open_hook(buf, ftype);

	return f;
}


/**
 * Close file handle 'f'.
 */
bool file_close(ang_file *f)
{
	if (fclose(f->fh) != 0)
		return false;

	mem_free(f->fname);
	mem_free(f);

	return true;
}



/** Locking functions **/

/**
 * Lock a file using POSIX locks, on platforms where this is supported.
 */
void file_lock(ang_file *f)
{
#if defined(HAVE_FCNTL_H) && defined(UNIX)
	struct flock lock;
	lock.l_type = (f->mode == MODE_READ ? F_RDLCK : F_WRLCK);
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_pid = 0;
	fcntl(fileno(f->fh), F_SETLKW, &lock);
#endif /* HAVE_FCNTL_H && UNIX */
}

/**
 * Unlock a file locked using file_lock().
 */
void file_unlock(ang_file *f)
{
#if defined(HAVE_FCNTL_H) && defined(UNIX)
	struct flock lock;
	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_pid = 0;
	fcntl(fileno(f->fh), F_SETLK, &lock);
#endif /* HAVE_FCNTL_H && UNIX */
}


/** Byte-based IO and functions **/

/**
 * Seek to location 'pos' in file 'f'.
 */
bool file_skip(ang_file *f, int bytes)
{
	return (fseek(f->fh, bytes, SEEK_CUR) == 0);
}

/**
 * Read a single, 8-bit character from file 'f'.
 */
bool file_readc(ang_file *f, uint8_t *b)
{
	int i = fgetc(f->fh);

	if (i == EOF)
		return false;

	*b = (uint8_t)i;
	return true;
}

/**
 * Write a single, 8-bit character 'b' to file 'f'.
 */
bool file_writec(ang_file *f, uint8_t b)
{
	return file_write(f, (const char *)&b, 1);
}

/**
 * Read 'n' bytes from file 'f' into array 'buf'.
 */
int file_read(ang_file *f, char *buf, size_t n)
{
	size_t read = fread(buf, 1, n, f->fh);

	if (read == 0 && ferror(f->fh))
		return -1;
	else
		return read;
}

/**
 * Append 'n' bytes of array 'buf' to file 'f'.
 */
bool file_write(ang_file *f, const char *buf, size_t n)
{
	return fwrite(buf, 1, n, f->fh) == n;
}

/** Line-based IO **/

/**
 * Read a line of text from file 'f' into buffer 'buf' of size 'n' bytes.
 *
 * Support both \r\n and \n as line endings, but not the outdated \r that used
 * to be used on Macs.  Replace non-printables with '?', and \ts with ' '.
 */
#define TAB_COLUMNS 4

bool file_getl(ang_file *f, char *buf, size_t len)
{
	bool seen_cr = false;
	uint8_t b;
	size_t i = 0;

	/* Leave a byte for the terminating 0 */
	size_t max_len = len - 1;

	while (i < max_len) {
		char c;

		if (!file_readc(f, &b)) {
			buf[i] = '\0';
			return (i == 0) ? false : true;
		}

		c = (char) b;

		if (c == '\r') {
			seen_cr = true;
			continue;
		}

		if (seen_cr && c != '\n') {
			fseek(f->fh, -1, SEEK_CUR);
			buf[i] = '\0';
			return true;
		}

		if (c == '\n') {
			buf[i] = '\0';
			return true;
		}

		/* Expand tabs */
		if (c == '\t') {
			/* Next tab stop */
			size_t tabstop = ((i + TAB_COLUMNS) / TAB_COLUMNS) * TAB_COLUMNS;
			if (tabstop >= len) break;

			/* Convert to spaces */
			while (i < tabstop)
				buf[i++] = ' ';

			continue;
		}

		buf[i++] = c;
	}

	buf[i] = '\0';
	return true;
}

/**
 * Append a line of text 'buf' to the end of file 'f', using system-dependent
 * line ending.
 */
bool file_put(ang_file *f, const char *buf)
{
	return file_write(f, buf, strlen(buf));
}

/*
 * The comp.lang.c FAQ recommends this pairing for varargs functions.
 * See <http://c-faq.com/varargs/handoff.html>
 */

/**
 * Append a formatted line of text to the end of file 'f'.
 *
 * file_putf() is the ellipsis version. Most file output will call this
 * version. It calls file_vputf() to do the real work. It returns true
 * if the write was successful and false otherwise.
 */
bool file_putf(ang_file *f, const char *fmt, ...)
{
	va_list vp;
	bool status;

	if (!f) return false;

	va_start(vp, fmt);
	status = file_vputf(f, fmt, vp);
	va_end(vp);

	return status;
}

/**
 * Append a formatted line of text to the end of file 'f'.
 *
 * file_vputf() is the va_list version. It returns true if the write was
 * successful and false otherwise.
 */
bool file_vputf(ang_file *f, const char *fmt, va_list vp)
{
	char buf[1024];

	if (!f) return false;

	(void)vstrnfmt(buf, sizeof(buf), fmt, vp);
	return file_put(f, buf);
}


bool dir_exists(const char *path)
{
	#ifdef HAVE_STAT
	struct stat buf;
	if (stat(path, &buf) != 0)
		return false;
	else if (buf.st_mode & S_IFDIR)
		return true;
	else
		return false;
	#else
	return true;
	#endif
}

#ifdef HAVE_STAT
bool dir_create(const char *path)
{
	const char *ptr;
	char buf[512];

	/* If the directory already exists then we're done */
	if (dir_exists(path)) return true;

	#ifdef WINDOWS
	/* If we're on windows, we need to skip past the "C:" part. */
	if (isalpha(path[0]) && path[1] == ':') path += 2;
	#endif

	/* Iterate through the path looking for path segments. At each step,
	 * create the path segment if it doesn't already exist. */
	for (ptr = path; *ptr; ptr++) {
		if (*ptr == PATH_SEPC) {
			/* Find the length of the parent path string */
			size_t len = (size_t)(ptr - path);

			/* Skip the initial slash */
			if (len == 0) continue;

			/* If this is a duplicate path separator, continue */
			if (*(ptr - 1) == PATH_SEPC) continue;

			/* We can't handle really big filenames */
			if (len - 1 > 512) return false;

			/* Create the parent path string, plus null-padding */
			my_strcpy(buf, path, len + 1);

			/* Skip if the parent exists */
			if (dir_exists(buf)) continue;

			/* The parent doesn't exist, so create it or fail */
			if (my_mkdir(buf, 0755) != 0) return false;
		}
	}
	return my_mkdir(path, 0755) == 0 ? true : false;
}

#else /* HAVE_STAT */
bool dir_create(const char *path) { return false; }
#endif /* !HAVE_STAT */

/**
 * ------------------------------------------------------------------------
 * Directory scanning API
 * ------------------------------------------------------------------------ */


/*
 * For information on what these are meant to do, please read the header file.
 */

#if defined(WINDOWS) && !defined(HAVE_DIRENT_H)


/* System-specific struct */
struct ang_dir
{
	HANDLE h;
	char *first_file;
	bool first_is_dir, only_files;
};

ang_dir *my_dopen(const char *dirname)
{
	WIN32_FIND_DATA fd;
	HANDLE h;
   	ang_dir *dir;
	
	/* Try to open it */
	h = FindFirstFile(format("%s\\*", dirname), &fd);

	/* Abort */
	if (h == INVALID_HANDLE_VALUE)
		return NULL;

	/* Set up the handle */
	dir = mem_zalloc(sizeof(ang_dir));
	dir->h = h;
	dir->first_file = string_make(fd.cFileName);
	dir->first_is_dir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		|| streq(dir->first_file, ".") || streq(dir->first_file, "..");
	dir->only_files = true;

	/* Success */
	return dir;
}

bool alter_ang_dir_only_files(ang_dir *dir, bool newval)
{
	bool oldval = dir->only_files;

	dir->only_files = newval;
	return oldval;
}

bool my_dread(ang_dir *dir, char *fname, size_t len)
{
	WIN32_FIND_DATA fd;
	bool ok;

	/* Try the first file */
	if (dir->first_file) {
		if (!dir->first_is_dir || !dir->only_files) {
			/* Copy the string across, then free it */
			my_strcpy(fname, dir->first_file, len);
			mem_free(dir->first_file);
			dir->first_file = NULL;

			/* Wild success */
			return true;
		} else {
			mem_free(dir->first_file);
			dir->first_file = NULL;
		}
	}

	/* Try the next file */
	while (1) {
		ok = FindNextFile(dir->h, &fd);
		if (!ok) return false;

		/* Skip directories if requested. */
		if (dir->only_files
				&& (fd.dwFileAttributes
				& FILE_ATTRIBUTE_DIRECTORY
				|| streq(fd.cFileName, ".")
				|| streq(fd.cFileName, ".."))) {
			continue;
		}
		/* Take this one */
		break;
	}

	/* Copy name */
	my_strcpy(fname, fd.cFileName, len);

	return true;
}

void my_dclose(ang_dir *dir)
{
	/* Close directory */
	if (dir->h)
		FindClose(dir->h);

	/* Free memory */
	mem_free(dir->first_file);
	mem_free(dir);
}

#else /* defined(WINDOWS) && !defined(HAVE_DIRENT_H) */

#ifdef HAVE_DIRENT_H

/* Define our ang_dir type */
struct ang_dir
{
	DIR *d;
	char *dirname;
	bool only_files;
};

ang_dir *my_dopen(const char *dirname)
{
	ang_dir *dir;
	DIR *d;

	/* Try to open the directory */
	d = opendir(dirname);
	if (!d) return NULL;

	/* Allocate memory for the handle */
	dir = mem_zalloc(sizeof(ang_dir));
	if (!dir) {
		closedir(d);
		return NULL;
	}

	/* Set up the handle */
	dir->d = d;
	dir->dirname = string_make(dirname);
	dir->only_files = true;

	/* Success */
	return dir;
}

bool alter_ang_dir_only_files(ang_dir *dir, bool newval)
{
	bool oldval = dir->only_files;

	dir->only_files = newval;
	return oldval;
}

bool my_dread(ang_dir *dir, char *fname, size_t len)
{
	struct dirent *entry;
	struct stat filedata;
	char path[1024];

	assert(dir != NULL);

	/* Try reading another entry */
	while (1) {
		entry = readdir(dir->d);
		if (!entry) return false;

		path_build(path, sizeof path, dir->dirname, entry->d_name);
            
		/* Check to see if it exists */
		if (stat(path, &filedata) != 0)
			continue;

		/* Check to see if it's a directory */
		if (dir->only_files && S_ISDIR(filedata.st_mode))
			continue;

		/* We've found something worth returning */
		break;
	}

	/* Copy the filename */
	my_strcpy(fname, entry->d_name, len);

	return true;
}

void my_dclose(ang_dir *dir)
{
	/* Close directory */
	if (dir->d)
		closedir(dir->d);

	/* Free memory */
	mem_free(dir->dirname);
	mem_free(dir);
}

#endif /* HAVE_DIRENT_H */
#endif /* WINDOWS */

