#ifndef INCLUDED_Z_FILE_H
#define INCLUDED_Z_FILE_H

/*** Various system-specific fixes ***/

/*
 * Use POSIX file control where we can, otherwise help out other platforms
 */
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#else
# define O_RDONLY   0
# define O_WRONLY   1
# define O_RDWR     2
#endif


/*
 * Several systems have no "O_BINARY" flag
 */
#ifndef O_BINARY
# define O_BINARY 0
#endif /* O_BINARY */


/*
 * Hack -- force definitions -- see fd_lock()  XXX
 */
#ifndef F_UNLCK
# define F_UNLCK    0
#endif
#ifndef F_RDLCK
# define F_RDLCK    1
#endif
#ifndef F_WRLCK
# define F_WRLCK    2
#endif


/*** Functions provided in the package ***/

extern errr path_parse(char *buf, size_t max, cptr file);
extern errr path_build(char *buf, size_t max, cptr path, cptr file);
extern FILE *my_fopen(cptr file, cptr mode);
extern FILE *my_fopen_temp(char *buf, size_t max);
extern errr my_fclose(FILE *fff);
extern errr my_fgets(FILE *fff, char *buf, size_t n);
extern errr my_fputs(FILE *fff, cptr buf, size_t n);
extern bool my_fexists(const char *fname);
extern errr fd_kill(cptr file);
extern errr fd_move(cptr file, cptr what);
extern int fd_make(cptr file, int mode);
extern int fd_open(cptr file, int flags);
extern errr fd_lock(int fd, int what);
extern errr fd_seek(int fd, long n);
extern errr fd_read(int fd, char *buf, size_t n);
extern errr fd_write(int fd, cptr buf, size_t n);
extern errr fd_close(int fd);
extern errr check_modification_date(int fd, cptr template_file);


typedef struct ang_dir ang_dir;

ang_dir *my_dopen(const char *dirname);
bool my_dread(ang_dir *dir, char *fname, size_t len);
void my_dclose(ang_dir *dir);


#endif
