#ifndef INCLUDED_SIGNALS_H
#define INCLUDED_SIGNALS_H

extern s16b signal_count;

void signals_ignore_tstp(void);
void signals_handle_tstp(void);
void signals_init(void);

#endif /* INCLUDED_SIGNALS_H */
