#ifndef INCLUDED_WIZARD_H
#define INCLUDED_WIZARD_H

/* wizard.c */
extern void do_cmd_debug(void);

/* wiz-stats.c */
void stats_collect(void);
void disconnect_stats(void);
void pit_stats(void);

/* wiz-spoil.c */
void do_cmd_spoilers(void);

#endif /* !INCLUDED_WIZARD_H */
