#ifndef INCLUDED_SAVEFILE_H
#define INCLUDED_SAVEFILE_H

#define ITEM_VERSION	5

/* load-old.c */
int rd_savefile_old(void);


/* Utility */
void note(cptr msg);
bool older_than(int x, int y, int z);

/* Writing bits */
void wr_byte(byte v);
void wr_u16b(u16b v);
void wr_s16b(s16b v);
void wr_u32b(u32b v);
void wr_s32b(s32b v);
void wr_string(cptr str);

/* Reading bits */
void rd_byte(byte *ip);
void rd_u16b(u16b *ip);
void rd_s16b(s16b *ip);
void rd_u32b(u32b *ip);
void rd_s32b(s32b *ip);
void rd_string(char *str, int max);
void strip_bytes(int n);



/* load.c */
int rd_randomizer(u32b version);
int rd_options(u32b version);
int rd_messages(u32b version);
int rd_monster_memory(u32b version);
int rd_object_memory(u32b version);
int rd_quests(u32b version);
int rd_artifacts(u32b version);
int rd_player(u32b version);
int rd_squelch(u32b version);
int rd_misc(u32b version);
int rd_player_hp(u32b version);
int rd_player_spells(u32b version);
int rd_randarts(u32b version);
int rd_inventory(u32b version);
int rd_stores(u32b version);
int rd_dungeon(u32b version);
int rd_objects(u32b version);
int rd_monsters(u32b version);
int rd_ghost(u32b version);
int rd_history(u32b version);

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
