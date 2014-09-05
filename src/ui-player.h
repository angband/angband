/**
   \file ui-player.h
   \brief character info
*/

#ifndef UI_PLAYER_H
#define UI_PLAYER_H

extern void display_player(int mode);
extern void display_player_stat_info(void);
extern void display_player_xtra_info(void);
extern errr file_character(const char *name, bool full);

#endif /* !UI_PLAYER_H */
