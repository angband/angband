#ifndef INCLUDED_Z_FILE_H
#define INCLUDED_Z_FILE_H

#include "h-basic.h"

/*** Permissions code ***/

/**
 * Player's user ID and group ID, respectively.
 *
 * Only relevant to POSIX systems that use main.c, and set there.
 */
extern int player_uid;
extern int player_egid;

/**
 * Drop or grab privileges.
 *
 * This is used on multiuser systems, where the game wants to gain access to
 * system-wide files like the scores, raw files, or savefiles.  Reading from
 * these locations is permitted by anyone, but writing to them requires a call
 * to safe_setuid_grab() before opening the file for writing.
 *
 * safe_setuid_drop() should be called immediately after the file has been
 * opened, to prevent security risks, and restores the game's rights so that it
 * cannot write to the system-wide files.
 */
void safe_setuid_grab(void);
void safe_setuid_drop(void);


/*** Path building code ***/

/**
 * Concatenates "leaf" onto the end of "base", using system-specific path
 * separators, and places the result in buf[], truncated to "len" bytes.
 *
 * On Unixes, deals with the tilde as representing home directories.
 */
size_t path_build(char *buf, size_t len, const char *base, const char *leaf);



/*** File access code ***/

/** Data types **/

/**
 * An opaque file handle for Angband file handling.
 */
typedef struct ang_file ang_file;

/**
 * Specifies what kind of access is required to a file.  See file_open().
 */
typedef enum
{
	MODE_WRITE,
	MODE_READ,
	MODE_APPEND
} file_mode;

/**
 * Specifies what kind of thing a file is, when writing.  See file_open().
 */
typedef enum
{
	FTYPE_TEXT = 1,
	FTYPE_SAVE,
	FTYPE_RAW,
	FTYPE_HTML
} file_type;


/** Utility functions **/

/**
 * Returns TRUE if `fname` exists (and is a file), FALSE otherwise.
 */
bool file_exists(const char *fname);

/**
 * Tries to delete `fname`.
 *
 * Returns TRUE if successful, FALSE otherwise.
 */
bool file_delete(const char *fname);

/**
 * Moves the file `fname` to `newname`.
 *
 * Returns TRUE if successful, FALSE otherwise.
 */
bool file_move(const char *fname, const char *newname);

/**
 * Returns TRUE if the file `first` is newer than `second`.
 */
bool file_newer(const char *first, const char *second);


/** File handle creation **/

/**
 * Open file `buf`, returning a a file handling representing that file.
 *
 * The file mode specifies what kind of access is required to the file:
 *  - MODE_WRITE will overwrite the current contents of the file
 *  - MODE_READ will allow read-only access to the file
 *  - MODE_APPEND will allow write-only access, but will not overwrite the
 *    current contents of the file.
 *
 * The file type is specified to allow systems which don't use file extensions
 * to set the type of the file appropriately.  When reading, pass -1 as ftype;
 * when writing, use whichever filetype seems most appropriate.
 *
 * On any kind of error, this function returns NULL.
 */
ang_file *file_open(const char *buf, file_mode mode, file_type ftype);


/**
 * Platform hook for file_open.  Used to set filetypes.
 */
extern void (*file_open_hook)(const char *path, file_type ftype);


/**
 * Attempt to close the file handle `f`.
 *
 * Returns TRUE if successful, FALSE otherwise.
 */
bool file_close(ang_file *f);


/** File locking **/

/**
 * Lock or unlock the file represented by `f` for writing.
 * If the file is not open for writing, this call will fail.
 *
 * If `f` is closed, the file is automatically unlocked.
 */
void file_lock(ang_file *f);
void file_unlock(ang_file *f);


/** Line-based IO **/

/**
 * Get a line of text from the file represented by `f`, placing it into `buf`
 * to a maximum length of `n`.
 *
 * This expands tabs, replaces non-printables with '?', and deals with differing
 * line endings.
 *
 * Returns TRUE when data is returned; FALSE otherwise.
 */
bool file_getl(ang_file *f, char *buf, size_t n);

/**
 * Write the string pointed to by `buf` to the file represented by `f`.
 *
 * Returns TRUE if successful, FALSE otherwise.
 */
bool file_put(ang_file *f, const char *buf);

/**
 * Format (using strnfmt) the given args, and then call file_put().
 */
bool file_putf(ang_file *f, const char *fmt, ...);
bool file_vputf(ang_file *f, const char *fmt, va_list vp); 

/**
 * Format and translate a string, then print it out to file.
 */
bool x_file_putf(ang_file *f, int encoding, const char *fmt, ...);


/** Byte-based IO */

/**
 * Seek to position `pos` in the file represented by `f`.
 *
 * Returns TRUE if successful, FALSE otherwise. 
 */
bool file_seek(ang_file *f, u32b pos);

/**
 * Reads n bytes from file 'f' info buffer 'buf'.
 * \returns Number of bytes read; -1 on error
 */
int file_read(ang_file *f, char *buf, size_t n);

/**
 * Write the first `n` bytes following the pointer `buf` to the file represented
 * by `f`.  Do not mix with calls to file_writec().
 *
 * Returns TRUE if successful, FALSE otherwise.
 */
bool file_write(ang_file *f, const char *buf, size_t n);

/**
 * Read a byte from the file represented by `f` and place it at the location
 * specified by 'b'.
 *
 * Returns TRUE if successful, FALSE otherwise.
 */
bool file_readc(ang_file *f, byte *b);

/**
 * Write the byte `b` to the file represented by `f`.
 *
 * Returns TRUE if successful, FALSE otherwise.
 */
bool file_writec(ang_file *f, byte b);



/*** Directory code ***/

/**
 * Return whether or not a directory exists.
 */
bool dir_exists(const char *dirname);

/**
 * Create's the given directory, creating intermediate directories if
 * needed and possible. Returns whether or not the directory was created
 * successfully.
 */
bool dir_create(const char *dirname);

/**
 * An opaque file handle for Angband directory handling.
 */
typedef struct ang_dir ang_dir;


/**
 * Opens a directory handle.
 *
 * `dirname` must be a system-specific pathname to the directory
 * you want scanned.
 *
 * Returns a valid directory handle on success, NULL otherwise.
 */
ang_dir *my_dopen(const char *dirname);

/**
 * Reads a directory entry.
 *
 * `dir` must point to a directory handle previously returned by my_dopen().
 * `fname` must be a pointer to a writeable chunk of memory `len` long.
 *
 * Returns TRUE on successful reading, FALSE otherwise.
 * (FALSE generally indicates that there are no more files to be read.)
 */
bool my_dread(ang_dir *dir, char *fname, size_t len);

/**
 * Close a directory handle.
 */
void my_dclose(ang_dir *dir);

#endif
