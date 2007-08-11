/*
 * File: save-block.c
 * Purpose: Blockfile saving and loading
 *
 * Copyright (c) 2007 Elly
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 */
#include "z-virt.h"
#include "z-blockfile.h"

static void bf_load(blockfile_t *bf);
void bf_loadblock(blockfile_t *bf);
void bf_loadrecord(blockfile_t *bf, block_t *bl);
void bf_saveblock(blockfile_t *bf, block_t *bl);
void bf_saverecord(blockfile_t *bf, record_t *rec);
void bf_freeblock(block_t *bl);

/* Public interface */
blockfile_t *bf_open(const char *name, u32b flags)
{
	blockfile_t *bf = ZNEW(blockfile_t);

	if (flags & BF_WRITE)
	{
		int ftype = FTYPE_RAW;
		if (flags & BF_SAVE) ftype = FTYPE_SAVE;

		bf->fh = file_open(name, MODE_WRITE, ftype);
	}
	else
	{
		bf->fh = file_open(name, MODE_READ, -1);
		bf_load(bf);
	}

	return bf;
}

u32b bf_nrblocks(blockfile_t *bf)
{
	return bf->nr_blocks;
}

block_t *bf_createblock(blockfile_t *bf, const char *name)
{
	block_t *block = ZNEW(block_t);

	block->namelen = strlen(name) + 1;
	block->name = string_make(name);

	if (!bf->block_head)
	{
		bf->block_curr = bf->block_head = bf->block_tail = block;
	}
	else
	{
		bf->block_tail->next = block;
		bf->block_tail = block;
	}

	bf->nr_blocks++;

	return block;
}

block_t *bf_findblock(blockfile_t *bf, const char *name)
{
	block_t *tmp = bf->block_head;

	while (tmp)
	{
		if (!strcmp(tmp->name, name))
			break;

		tmp = tmp->next;
	}

	return tmp;
}

block_t *bf_nextblock(blockfile_t *bf)
{
	block_t *bl = bf->block_curr;

	if (bl)
		bf->block_curr = bl->next;

	return bl;
}

void bf_rewind(blockfile_t *bf)
{
	bf->block_curr = bf->block_head;
}

void bf_eachblock(blockfile_t *bf, void (*fn)(block_t *block))
{
	block_t *bl = bf->block_head;
	while (bl)
	{
		fn(bl);
		bl = bl->next;
	}
}

const char *bf_name(block_t *block)
{
	return block->name;
}

u32b bf_nrrecords(block_t *block)
{
	return block->nr_records;
}

u32b bf_blocksize(block_t *block)
{
	u32b size = 0;
	record_t *rec = block->record_head;

	while (rec != NULL)
	{
		size += rec->len;
		rec = rec->next;
	}

	return size;
}

void bf_createrecord(block_t *block, void *data, u32b len)
{
	record_t *rec = ZNEW(record_t);

	rec->len = len;
	rec->data = mem_alloc(len);
	memcpy(rec->data, data, len);
	
	if (!block->record_head)
	{
		block->record_head = block->record_tail = block->record_curr = rec;
	}
	else
	{
		block->record_tail->next = rec;
		block->record_tail = rec;
	}

	block->nr_records++;
}

const void *bf_nextrecord(block_t *block, u32b *len)
{
	record_t *rec = block->record_curr;
	if (!rec) return NULL;

	*len = rec->len;
	block->record_curr = rec->next;

	return rec->data;
}

void bf_rewindrecord(block_t *block)
{
	block->record_curr = block->record_head;
}

void bf_eachrecord(block_t *block, void (*fn)(const void *, u32b))
{
	record_t *rec = block->record_head;
	while (rec)
	{
		fn(rec->data, rec->len);
		rec = rec->next;
	}
}
		

void bf_save(blockfile_t *bf)
{
	block_t *tmp;
	u32b nr_blocks = flip_u32b(bf->nr_blocks);

	file_seek(bf->fh, 0);
	file_write(bf->fh, (char *) &(nr_blocks), sizeof(nr_blocks));

	tmp = bf->block_head;
	while (tmp)
	{
		bf_saveblock(bf, tmp);
		tmp = tmp->next;
	}
}

void bf_close(blockfile_t *bf)
{
	block_t *tmp;
	block_t *next;

	file_close(bf->fh);
	tmp = bf->block_head;
	while (tmp)
	{
		next = tmp->next;
		bf_freeblock(tmp);
		tmp = next;
	}

	FREE(bf);
}

static void bf_load(blockfile_t *bf)
{
	u32b blockno = 0;

	file_read(bf->fh, (char *) &(bf->nr_blocks), sizeof(bf->nr_blocks));
	bf->nr_blocks = flip_u32b(bf->nr_blocks);

	while (blockno < bf->nr_blocks)
	{
		bf_loadblock(bf);
		blockno++;
	}
}

void bf_loadblock(blockfile_t *bf)
{
	block_t *bl = ZNEW(block_t);
	u32b recno = 0;

	file_read(bf->fh, (char *) &(bl->namelen), sizeof(bl->namelen));
	bl->namelen = flip_u32b(bl->namelen);

	bl->name = mem_alloc(bl->namelen);
	file_read(bf->fh, bl->name, bl->namelen);

	file_read(bf->fh, (char *) &(bl->nr_records), sizeof(bl->nr_records));
	bl->nr_records = flip_u32b(bl->nr_records);

	while (recno < bl->nr_records)
	{
		bf_loadrecord(bf, bl);
		recno++;
	}

	if (!bf->block_head)
	{
		bf->block_head = bf->block_tail = bf->block_curr = bl;
	}
	else
	{
		bf->block_tail->next = bl;
		bf->block_tail = bl;
	}
}

void bf_loadrecord(blockfile_t *bf, block_t *bl)
{
	record_t *rec = ZNEW(record_t);

	file_read(bf->fh, (char *) &(rec->len), sizeof(rec->len));
	rec->len = flip_u32b(rec->len);

	rec->data = mem_alloc(rec->len);
	file_read(bf->fh, rec->data, rec->len);

	if (!bl->record_head)
	{
		bl->record_head = bl->record_tail = bl->record_curr = rec;
	}
	else
	{
		bl->record_tail->next = rec;
		bl->record_tail = rec;
	}
}



void bf_saveblock(blockfile_t *bf, block_t *bl)
{
	record_t *tmp;
	u32b namelen = flip_u32b(bl->namelen);
	u32b nr_records = flip_u32b(bl->nr_records);

	file_write(bf->fh, (char *) &(namelen), sizeof(namelen));
	file_write(bf->fh, bl->name, bl->namelen);
	file_write(bf->fh, (char *) &(nr_records), sizeof(nr_records));

	tmp = bl->record_head;
	while (tmp)
	{
		bf_saverecord(bf, tmp);
		tmp = tmp->next;
	}
}

void bf_saverecord(blockfile_t *bf, record_t *rec)
{
	u32b len = flip_u32b(rec->len);

	file_write(bf->fh, (char *) &(len), sizeof(rec->len));
	file_write(bf->fh, rec->data, rec->len);
}


void bf_freeblock(block_t *block)
{
	record_t *tmp;
	record_t *next; 

	FREE(block->name);

	tmp = block->record_head;
	while (tmp)
	{
		next = tmp->next;
		FREE(tmp->data);
		FREE(tmp);
		tmp = next;
	}

	FREE(block);
}
