#ifndef INCLUDED_SAVEFILE_H
#define INCLUDED_SAVEFILE_H

#define ITEM_VERSION	5

/*** Savefile API ***/

/**
 * Load the savefile given.  Returns TRUE on succcess, FALSE otherwise.
 */
bool savefile_load(const char *path);

/**
 * Save to the given location.  Returns TRUE on success, FALSE otherwise.
 */
bool savefile_save(const char *path);



/*** Ignore these ***/

/* Utility */
void note(const char *msg);

/* Writing bits */
void wr_byte(byte v);
void wr_u16b(u16b v);
void wr_s16b(s16b v);
void wr_u32b(u32b v);
void wr_s32b(s32b v);
void wr_string(const char *str);
void pad_bytes(int n);

/* Reading bits */
void rd_byte(byte *ip);
void rd_u16b(u16b *ip);
void rd_s16b(s16b *ip);
void rd_u32b(u32b *ip);
void rd_s32b(s32b *ip);
void rd_string(char *str, int max);
void strip_bytes(int n);



/* load.c */
int rd_randomizer(void);
int rd_options_1(void);
int rd_options_2(void);
int rd_messages(void);
int rd_monster_memory_1(void);
int rd_monster_memory_2(void);
int rd_object_memory(void);
int rd_quests(void);
int rd_artifacts(void);
int rd_player(void);
int rd_squelch(void);
int rd_misc(void);
int rd_player_hp(void);
int rd_player_spells(void);
int rd_randarts_1(void);
int rd_randarts_2(void);
int rd_inventory_1(void);
int rd_inventory_2(void);
int rd_inventory_3(void);
int rd_stores_1(void);
int rd_stores_2(void);
int rd_stores_3(void);
int rd_dungeon(void);
int rd_objects_1(void);
int rd_objects_2(void);
int rd_objects_3(void);
int rd_monsters_1(void);
int rd_monsters_2(void);
int rd_monsters_3(void);
int rd_monsters_4(void);
int rd_monsters_5(void);
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
