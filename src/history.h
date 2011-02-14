/* history.h - player history tracking */

#ifndef HISTORY_H
#define HISTORY_H

void history_clear(void);
size_t history_get_num(void);
bool history_add_full(u16b type, struct artifact *artifact, s16b dlev, s16b clev, s32b turn, const char *text);
bool history_add(const char *event, u16b type, struct artifact *art);
bool history_add_artifact(struct artifact *art, bool known, bool found);
void history_unmask_unknown(void);
bool history_lose_artifact(struct artifact *art);
void history_display(void);
void dump_history(ang_file *file);
bool history_is_artifact_known(struct artifact *art);

extern history_info *history_list;

#endif /* !HISTORY_H */
