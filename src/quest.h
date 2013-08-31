#ifndef QUEST_H
#define QUEST_H

/*
 * Structure for the "quests"
 *
 * Hack -- currently, only the "level" parameter is set, with the
 * semantics that "one (QUEST) monster of that level" must be killed,
 * and then the "level" is reset to zero, meaning "all done".  Later,
 * we should allow quests like "kill 100 fire hounds", and note that
 * the "quest level" is then the level past which progress is forbidden
 * until the quest is complete.  Note that the "QUESTOR" flag then could
 * become a more general "never out of depth" flag for monsters.
 */
typedef struct quest quest;
struct quest
{
	byte level;		/* Dungeon level */
	int r_idx;		/* Monster race */

	int cur_num;	/* Number killed (unused) */
	int max_num;	/* Number required (unused) */
};

/* Maximum number of quests */
#define MAX_Q_IDX	4

/* Quest list */
extern quest q_list[MAX_Q_IDX];

/* Functions */
bool is_quest(int level);
void quest_reset(void);
bool quest_check(const struct monster *m);
void quest_init(void);
void quest_free(void);


#endif /* QUEST_H */
