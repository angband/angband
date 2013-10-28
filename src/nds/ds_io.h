
#ifndef DS_FILEIO_H
#define DS_FILEIO_H

// guarantee that this file will be included before the #defs
#include <fcntl.h>

//#include "nds.h"
/*#define BG_GFX_SUB        ((unsigned short*)0x6200000)
#define RGB15(r,g,b)  ((r)|((g)<<5)|((b)<<10))
extern int vram_pos;
#define D(s) 	BG_GFX_SUB[vram_pos++] = RGB15(0,31,31) | (1<<15);
#define F(x,y)	BG_GFX_SUB[y*256*2+x*2] = RGB15(31,0,0) | (1<<15);
#define G(x,y,r,g,b)	BG_GFX_SUB[y*256*2+x*2] = RGB15(r,g,b) | (1<<15);*/
# define NDECL(f)	f()
# define FDECL(f,p)	f()
# define VDECL(f,p)	f()

/*
#define fopen		ds_fopen
#define fclose		ds_fclose
#define fseek		ds_fseek
#define rewind		ds_rewind
#define fgetc		ds_fgetc
#define fgets		ds_fgets
#define fread		ds_fread
*/
#define open		ds_open
#define close		ds_close
#define creat		ds_creat
#define lseek		ds_lseek
#define read		ds_read
#define sopen		ds_sopen
#define write		ds_write
// I wish I knew a better way to do this...
/*
#define ftell		ds_ftell
#define fputc		ds_fputc
#define fputs		ds_fputs
#define fwrite		ds_fwrite
#define fscanf		ds_fscanf
#define fprintf		ds_fprintf
#define vfprintf	ds_vfprintf
#define access		ds_access
#define unlink		ds_unlink
#define getuid		ds_getuid
#define getgid		ds_getgid
#define getpid		ds_getpid
#define time		ds_time
#define getenv		ds_getenv

#define freopen(a,b,c)		ds_open(a,b)
*/
//#define EOF -1
	
#define E extern
E void VDECL(error, (const char *,...)); //PRINTF_F(1,2);
E void FDECL(regularize, (char *));
E char* FDECL(get_username, (int *));
E FILE* FDECL(ds_fopen, (const char *, const char *));
E int FDECL(ds_fclose, (FILE *));
E long FDECL(ds_ftell, (FILE *));
E int FDECL(ds_fseek, (FILE *, long, int));
E void FDECL(ds_rewind, (FILE *));
E int FDECL(ds_fgetc, (FILE *));
E int FDECL(ds_fputc, (int, FILE *));
E char* FDECL(ds_fgets, (char *, int, FILE *));
E int FDECL(ds_fputs, (const char *, FILE *));
E int FDECL(ds_fread, (void *, size_t, size_t, FILE *));
E int FDECL(ds_fwrite, (const void *, size_t, size_t, FILE *));
E int VDECL(ds_fscanf, (FILE *, const char *, ...));
E int VDECL(ds_fprintf, (FILE *, const char *, ...)); //PRINTF_F(2,3);
E int FDECL(ds_vfprintf, (FILE *, const char *, va_list));
E int FDECL(ds_access, (const char *, int));
E int FDECL(ds_close, (int));
E int FDECL(ds_creat, (const char *, int));
E off_t FDECL(ds_lseek, (int, off_t, int));
E int FDECL(ds_open, (const char *, int, int));
E int FDECL(ds_read, (int, void *, unsigned int));
E int FDECL(ds_sopen, (const char *, int, int, int));
E int FDECL(ds_unlink, (const char *));
E int FDECL(ds_write, (int, const void *, unsigned int));
E int NDECL(ds_getuid);
E int NDECL(ds_getgid);
E int NDECL(ds_getpid);
E time_t FDECL(ds_time, (time_t *));
E struct tm* FDECL(ds_localtime, (time_t *));
E char* FDECL(ds_getenv, (const char*));



#endif
