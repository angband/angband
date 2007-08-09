#ifndef INCLUDED_BLOCKFILE_H
#define INCLUDED_BLOCKFILE_H

#include "h-basic.h"
#include "z-file.h"

#define BF_WRITE	0x00000001
#define BF_READ		0x00000002

typedef struct _record_t
{
	u32b len;
	void *data;
	struct _record_t *next;
} record_t;

typedef struct _block_t
{
	u32b namelen;
	char *name;
	u32b nr_records;
	record_t *record_curr;
	record_t *record_head;
	record_t *record_tail;
	struct _block_t *next;
} block_t;

typedef struct _blockfile_t
{
	ang_file *fh;
	u32b nr_blocks;
	u32b flags;
	block_t *block_curr;
	block_t *block_head;
	block_t *block_tail;
} blockfile_t;

blockfile_t *bf_open(char *name, u32b flags);
u32b bf_nrblocks(blockfile_t *bf);

block_t *bf_createblock(blockfile_t *bf, char *name);
block_t *bf_findblock(blockfile_t *bf, char *name);
block_t *bf_nextblock(blockfile_t *bf);
void bf_rewindblock(blockfile_t *bf);
void bf_eachblock(blockfile_t *bf, void (*fn)(block_t *block));
char *bf_name(block_t *block);
u32b bf_nrrecords(block_t *block);
u32b bf_blocksize(block_t *block);

void bf_createrecord(block_t *block, void *data, u32b len);
void *bf_nextrecord(block_t *block, u32b *len);
void bf_rewindrecord(block_t *block);
void bf_eachrecord(block_t *block, void (*fn)(record_t *rec));

void bf_save(blockfile_t *bf);
void bf_close(blockfile_t *bf);

#endif /* !INCLUDED_BLOCKFILE_H */
