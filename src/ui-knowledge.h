#ifndef UI_KNOWLEDGE_H
#define UI_KNOWLEDGE_H

void textui_browse_object_knowledge(const char *name, int row);
void textui_knowledge_init(void);
void textui_browse_knowledge(void);
void do_cmd_message_one(void);
void do_cmd_messages(void);
void do_cmd_inven(void);
void do_cmd_equip(void);
void do_cmd_look(void);
void do_cmd_locate(void);
int cmp_monsters(const void *a, const void *b);
void do_cmd_query_symbol(void);
void do_cmd_center_map(void);
void do_cmd_monlist(void);
void do_cmd_itemlist(void);

#endif /* UI_KNOWLEDGE_H */
