
#ifndef DS_BTN_H
#define DS_BTN_H

s16 nds_buttons_to_btnid(u16 kd, u16 kh);
void nds_btn_vblank();
void nds_assign_button();
void nds_check_buttons(u16 kd, u16 kh);
void nds_init_buttons();

#endif
