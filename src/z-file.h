#ifndef INCLUDED_Z_FILE_H
#define INCLUDED_Z_FILE_H

#include "h-basic.h"

extern int player_uid;
extern int player_egid;

void safe_setuid_drop(void);
void safe_setuid_grab(void);

size_t path_build(char *buf, size_t len, const char *base, const char *leaf);

/* File code */
typedef struct ang_file ang_file;

typedef enum
{
	MODE_WRITE,
	MODE_READ,
	MODE_APPEND,
} file_mode;

typedef enum
{
	FTYPE_TEXT = 1, /* -> FILE_TYPE_TEXT */
	FTYPE_SAVE,
	FTYPE_RAW,		/* -> FILE_TYPE_DATA */
	FTYPE_HTML
} file_type;

bool file_exists(const char *fname);
bool file_delete(const char *fname);
bool file_move(const char *fname, const char *newname);
bool file_newer(const char *first, const char *second);

ang_file *file_open(const char *buf, file_mode mode, file_type ftype);
ang_file *file_temp();
bool file_close(ang_file *f);

void file_lock(ang_file *f);
void file_unlock(ang_file *f);

bool file_getl(ang_file *f, char *buf, size_t n);
bool file_put(ang_file *f, const char *buf);
bool file_putf(ang_file *f, const char *fmt, ...);

bool file_seek(ang_file *f, u32b pos);
size_t file_read(ang_file *f, char *buf, size_t n);
bool file_write(ang_file *f, const char *buf, size_t n);
bool file_readc(ang_file *f, byte *b);
bool file_writec(ang_file *f, byte b);



/* Directory code */
typedef struct ang_dir ang_dir;

ang_dir *my_dopen(const char *dirname);
bool my_dread(ang_dir *dir, char *fname, size_t len);
void my_dclose(ang_dir *dir);

#endif
