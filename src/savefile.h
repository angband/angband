#ifndef INCLUDED_SAVEFILE_H
#define INCLUDED_SAVEFILE_H

/* savefile.c */
typedef struct
{
	char name[16];
	int (*loader)(void);
	void (*saver)(void);
	u32b cur_ver;
	u32b oldest_ver;
} savefile_block_t;

#define N_SAVEFILE_BLOCKS	20
extern savefile_block_t savefile_blocks[N_SAVEFILE_BLOCKS];


/* Writing bits */
void wr_byte(byte v);
void wr_u16b(u16b v);
void wr_s16b(s16b v);
void wr_u32b(u32b v);
void wr_s32b(s32b v);
void wr_string(cptr str);



/* load.c */
int rd_randomizer(void);
int rd_options(void);
int rd_messages(void);
int rd_monster_memory(void);
int rd_object_memory(void);
int rd_quests(void);
int rd_artifacts(void);
int rd_player(void);
int rd_squelch(void);
int rd_misc(void);
int rd_player_hp(void);
int rd_player_spells(void);
int rd_randarts(void);
int rd_inventory(void);
int rd_stores(void);
int rd_dungeon(void);
int rd_objects(void);
int rd_monsters(void);
int rd_ghost(void);
int rd_history(void);

/* save.c */
void wr_randomizer(void);
void wr_options(void);
void wr_messages(void);
void wr_monster_memory(void);
void wr_object_memory(void);
void wr_quests(void);
void wr_artifacts(void);
void wr_player(void);
void wr_squelch(void);
void wr_misc(void);
void wr_player_hp(void);
void wr_player_spells(void);
void wr_randarts(void);
void wr_inventory(void);
void wr_stores(void);
void wr_dungeon(void);
void wr_objects(void);
void wr_monsters(void);
void wr_ghost(void);
void wr_history(void);


#endif /* INCLUDED_SAVEFILE_H */
