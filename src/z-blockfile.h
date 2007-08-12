#ifndef INCLUDED_BLOCKFILE_H
#define INCLUDED_BLOCKFILE_H

#include "h-basic.h"
#include "z-file.h"


/*** Constants ***/

/*
 * For use with bf_open().  Indicate mode of file opening, and when "BF_SAVE"
 * is set, the file will be marked as a savefile on participating OSes.
 */
#define BF_WRITE	0x00000001
#define BF_READ		0x00000002
#define BF_SAVE		0x00000004


/*** Data types ***/

/* (Forward declarations) */
typedef struct _record_t record_t;
typedef struct _block_t block_t;
typedef struct _blockfile_t blockfile_t;


/*
 * The basic "blockfile" type.
 *
 * A blockfile contains a given number of blocks and is associated with a file
 * handle.  Each block is named, and is of type block_t.
 */
struct _blockfile_t
{
	ang_file *fh;
	u32b nr_blocks;
	u32b flags;
	block_t *block_curr;
	block_t *block_head;
	block_t *block_tail;
};

/*
 * A block, stored in a blockfile
 *
 * Many blocks make up a blockfile, and each block contains one or more
 * records, which are of type record_t.
 */
struct _block_t
{
	u32b namelen;
	char *name;
	u32b nr_records;
	record_t *record_curr;
	record_t *record_head;
	record_t *record_tail;
	struct _block_t *next;
};

/*
 * A record in a block.
 */
struct _record_t
{
	u32b len;
	void *data;
	struct _record_t *next;
};


/*** Functions ***/

/** Opening/saving/closing **/

/*
 * Open a new blockfile, with the filename "name" and flags as follows:
 *
 * - BF_READ indicates opening an already-existing file, and loading data from
 *   it into memory
 * - BF_WRITE indicates overwriting any existing file with a clean structure,
 *   ready for writing new data
 * - BF_SAVE indicates that this file is a savefile.
 *
 * Returns a blockfile handle if successful, or NULL if unsucessful.
 */
blockfile_t *bf_open(const char *name, u32b flags);

/*
 * Save all currently held data to the blockfile.
 *
 * Only do this if you have opened a blockfile for writing.  If you do *not* do
 * this when writing, then data will not be stored to the blockfile.
 */
void bf_save(blockfile_t *bf);

/*
 * Close a blockfile handle.
 *
 * If the blockfile is opened for writing, you should call bf_save() first to
 * save any data you have added, or it will not be saved.
 */
void bf_close(blockfile_t *bf);


/** Block info **/

/*
 * Returns the number of blocks in the blockfile represented by `bf`.
 */
u32b bf_nrblocks(blockfile_t *bf);


/** Block creation **/

/*
 * Create a new block in the blockfile `bf` with name `name`.
 *
 * Returns the block when successful, NULL otherwise.
 *
 * When you have created a block, you should use bf_createrecord() to fill it
 * with data.
 */
block_t *bf_createblock(blockfile_t *bf, const char *name);


/** Finding blocks **/

/*
 * Finds a block in the blockfile `bf` with the name `name`.
 *
 * If the block cannot be found, NULL is returned; otherwise, a pointer to the
 * block is returned.
 */
block_t *bf_findblock(blockfile_t *bf, const char *name);

/*
 * Return the next block in the blockfile `bf`.
 *
 * On the first call, this will return the first block written to the file; the
 * second call will return the second block, etc.  If there is no "next block"
 * in the file, NULL will be returned.
 *
 * If you want to iterate over all the blocks again, use bf_rewind().
 */
block_t *bf_nextblock(blockfile_t *bf);

/*
 * Reset the current block in the blockfile `bf`.  This is only useful if,
 * having once iterated through some blocks in the file, you want to return to
 * the beginning to go again.
 */
void bf_rewind(blockfile_t *bf);


/*
 * Calls the provided function for each block in the file `bf`, in the order in
 * which they were written.
 */
void bf_eachblock(blockfile_t *bf, void (*fn)(block_t *block));


/** Block information **/

/*
 * Returns the name of block `block`.
 */
const char *bf_name(block_t *block);

/*
 * Returns the size of block `block`.
 */
u32b bf_blocksize(block_t *block);

/*
 * Returns the number of records stored within the block `block`.
 */
u32b bf_nrrecords(block_t *block);



/** Record creation and reading **/

/*
 * Create a new record in the block `block`, with data `data` of length `len`.
 *
 * Much like a blockfile contains multiple blocks, each block contains one or
 * more records.  These are read back using bf_nextrecord() or bf_eachrecord().
 */
void bf_createrecord(block_t *block, void *data, u32b len);

/*
 * Return the data of the next record in the block `block`, and place the
 * length of this data in *`len`.
 *
 * On the first call, this will return the first record written to the file;
 * the second call will return the second, etc.  If there is no "next record"
 * in the file, NULL will be returned.
 *
 * If you want to iterate over all the records again, use bf_rewindrecord().
 */
const void *bf_nextrecord(block_t *block, u32b *len);

/*
 * Reset the current record in the block `block`.  This is only useful if,
 * having once iterated through some records in the block, you want to return
 * to the beginning to go again.
 */
void bf_rewindrecord(block_t *block);

/*
 * Calls the supplied `fn` to iterate over each record in the block `block`.
 */
void bf_eachrecord(block_t *block, void (*fn)(const void *rec, u32b len));


#endif /* !INCLUDED_BLOCKFILE_H */
