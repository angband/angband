#ifndef __angband_z_file__
#define __angband_z_file__

/*** Various system-specific fixes ***/

/*
 * Several systems have no "O_BINARY" flag
 */
#ifndef O_BINARY
# define O_BINARY 0
#endif /* O_BINARY */

/*
 * Hack -- assist "main-ros.c" XXX
 */
#ifdef RISCOS
# define O_RDONLY   0
# define O_WRONLY   1
# define O_RDWR     2
#endif

/*
 * Hack -- force definitions -- see fd_seek()
 */
#ifndef SEEK_SET
# define SEEK_SET   0
#endif
#ifndef SEEK_CUR
# define SEEK_CUR   1
#endif
#ifndef SEEK_END
# define SEEK_END   2
#endif

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


#endif
