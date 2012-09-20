/*
** Lua binding: ui
** Generated automatically by tolua 4.0a - angband on Sun Jan  6 15:39:39 2002.
*/

#include "lua/tolua.h"

/* Exported function */
int tolua_ui_open (lua_State* tolua_S);
void tolua_ui_close (lua_State* tolua_S);

#include "angband.h"

/* function to register type */
static void toluaI_reg_types (lua_State* tolua_S)
{
}

/* error messages */
#define TOLUA_ERR_SELF tolua_error(tolua_S,"invalid 'self'")
#define TOLUA_ERR_ASSIGN tolua_error(tolua_S,"#vinvalid type in variable assignment.")

/* function: notice_stuff */
static int toluaI_ui_notice_stuff00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  notice_stuff();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'notice_stuff'.");
 return 0;
}

/* function: update_stuff */
static int toluaI_ui_update_stuff00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  update_stuff();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'update_stuff'.");
 return 0;
}

/* function: redraw_stuff */
static int toluaI_ui_redraw_stuff00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  redraw_stuff();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'redraw_stuff'.");
 return 0;
}

/* function: window_stuff */
static int toluaI_ui_window_stuff00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  window_stuff();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'window_stuff'.");
 return 0;
}

/* function: handle_stuff */
static int toluaI_ui_handle_stuff00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  handle_stuff();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'handle_stuff'.");
 return 0;
}

/* function: inkey */
static int toluaI_ui_inkey00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  char toluaI_ret = (char)  inkey();
 tolua_pushnumber(tolua_S,(long)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'inkey'.");
 return 0;
}

/* function: bell */
static int toluaI_ui_bell00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TSTRING,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  cptr reason = ((cptr)  tolua_getstring(tolua_S,1,0));
 {
  bell(reason);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'bell'.");
 return 0;
}

/* function: sound */
static int toluaI_ui_sound00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int val = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  sound(val);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'sound'.");
 return 0;
}

/* function: msg_print */
static int toluaI_ui_msg_print00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TSTRING,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  cptr msg = ((cptr)  tolua_getstring(tolua_S,1,0));
 {
  msg_print(msg);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'msg_print'.");
 return 0;
}

/* function: message */
static int toluaI_ui_message00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,LUA_TSTRING,0) ||
 !tolua_isnoobj(tolua_S,4)
 )
 goto tolua_lerror;
 else
 {
  u16b message_type = ((u16b)  tolua_getnumber(tolua_S,1,0));
  s16b extra = ((s16b)  tolua_getnumber(tolua_S,2,0));
  cptr message_text = ((cptr)  tolua_getstring(tolua_S,3,0));
 {
  message(message_type,extra,message_text);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'message'.");
 return 0;
}

/* function: message_flush */
static int toluaI_ui_message_flush00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  message_flush();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'message_flush'.");
 return 0;
}

/* function: screen_save */
static int toluaI_ui_screen_save00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  screen_save();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'screen_save'.");
 return 0;
}

/* function: screen_load */
static int toluaI_ui_screen_load00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  screen_load();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'screen_load'.");
 return 0;
}

/* function: c_put_str */
static int toluaI_ui_c_put_str00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TSTRING,0) ||
 !tolua_istype(tolua_S,3,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,4,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,5)
 )
 goto tolua_lerror;
 else
 {
  byte attr = ((byte)  tolua_getnumber(tolua_S,1,0));
  cptr str = ((cptr)  tolua_getstring(tolua_S,2,0));
  int row = ((int)  tolua_getnumber(tolua_S,3,0));
  int col = ((int)  tolua_getnumber(tolua_S,4,0));
 {
  c_put_str(attr,str,row,col);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'c_put_str'.");
 return 0;
}

/* function: put_str */
static int toluaI_ui_put_str00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TSTRING,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,4)
 )
 goto tolua_lerror;
 else
 {
  cptr str = ((cptr)  tolua_getstring(tolua_S,1,0));
  int row = ((int)  tolua_getnumber(tolua_S,2,0));
  int col = ((int)  tolua_getnumber(tolua_S,3,0));
 {
  put_str(str,row,col);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'put_str'.");
 return 0;
}

/* function: c_prt */
static int toluaI_ui_c_prt00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TSTRING,0) ||
 !tolua_istype(tolua_S,3,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,4,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,5)
 )
 goto tolua_lerror;
 else
 {
  byte attr = ((byte)  tolua_getnumber(tolua_S,1,0));
  cptr str = ((cptr)  tolua_getstring(tolua_S,2,0));
  int row = ((int)  tolua_getnumber(tolua_S,3,0));
  int col = ((int)  tolua_getnumber(tolua_S,4,0));
 {
  c_prt(attr,str,row,col);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'c_prt'.");
 return 0;
}

/* function: prt */
static int toluaI_ui_prt00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TSTRING,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,4)
 )
 goto tolua_lerror;
 else
 {
  cptr str = ((cptr)  tolua_getstring(tolua_S,1,0));
  int row = ((int)  tolua_getnumber(tolua_S,2,0));
  int col = ((int)  tolua_getnumber(tolua_S,3,0));
 {
  prt(str,row,col);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'prt'.");
 return 0;
}

/* function: clear_from */
static int toluaI_ui_clear_from00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int row = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  clear_from(row);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'clear_from'.");
 return 0;
}

/* function: pause_line */
static int toluaI_ui_pause_line00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int row = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  pause_line(row);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'pause_line'.");
 return 0;
}

/* function: request_command */
static int toluaI_ui_request_command00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"bool"),0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  bool shopping = ((bool)  tolua_getbool(tolua_S,1,0));
 {
  request_command(shopping);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'request_command'.");
 return 0;
}

/* function: text_out_to_screen */
static int toluaI_ui_text_out00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TSTRING,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  byte a = ((byte)  tolua_getnumber(tolua_S,1,0));
  cptr str = ((cptr)  tolua_getstring(tolua_S,2,0));
 {
  text_out_to_screen(a,str);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'text_out'.");
 return 0;
}

/* function: Term_clear */
static int toluaI_ui_Term_clear00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  errr toluaI_ret = (errr)  Term_clear();
 tolua_pushnumber(tolua_S,(long)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Term_clear'.");
 return 0;
}

/* Open function */
int tolua_ui_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 toluaI_reg_types(tolua_S);
 tolua_constant(tolua_S,NULL,"TERM_DARK",TERM_DARK);
 tolua_constant(tolua_S,NULL,"TERM_WHITE",TERM_WHITE);
 tolua_constant(tolua_S,NULL,"TERM_SLATE",TERM_SLATE);
 tolua_constant(tolua_S,NULL,"TERM_ORANGE",TERM_ORANGE);
 tolua_constant(tolua_S,NULL,"TERM_RED",TERM_RED);
 tolua_constant(tolua_S,NULL,"TERM_GREEN",TERM_GREEN);
 tolua_constant(tolua_S,NULL,"TERM_BLUE",TERM_BLUE);
 tolua_constant(tolua_S,NULL,"TERM_UMBER",TERM_UMBER);
 tolua_constant(tolua_S,NULL,"TERM_L_DARK",TERM_L_DARK);
 tolua_constant(tolua_S,NULL,"TERM_L_WHITE",TERM_L_WHITE);
 tolua_constant(tolua_S,NULL,"TERM_VIOLET",TERM_VIOLET);
 tolua_constant(tolua_S,NULL,"TERM_YELLOW",TERM_YELLOW);
 tolua_constant(tolua_S,NULL,"TERM_L_RED",TERM_L_RED);
 tolua_constant(tolua_S,NULL,"TERM_L_GREEN",TERM_L_GREEN);
 tolua_constant(tolua_S,NULL,"TERM_L_BLUE",TERM_L_BLUE);
 tolua_constant(tolua_S,NULL,"TERM_L_UMBER",TERM_L_UMBER);
 tolua_constant(tolua_S,NULL,"MSG_GENERIC",MSG_GENERIC);
 tolua_constant(tolua_S,NULL,"MSG_HIT",MSG_HIT);
 tolua_constant(tolua_S,NULL,"MSG_MISS",MSG_MISS);
 tolua_constant(tolua_S,NULL,"MSG_FLEE",MSG_FLEE);
 tolua_constant(tolua_S,NULL,"MSG_DROP",MSG_DROP);
 tolua_constant(tolua_S,NULL,"MSG_KILL",MSG_KILL);
 tolua_constant(tolua_S,NULL,"MSG_LEVEL",MSG_LEVEL);
 tolua_constant(tolua_S,NULL,"MSG_DEATH",MSG_DEATH);
 tolua_constant(tolua_S,NULL,"MSG_STUDY",MSG_STUDY);
 tolua_constant(tolua_S,NULL,"MSG_TELEPORT",MSG_TELEPORT);
 tolua_constant(tolua_S,NULL,"MSG_SHOOT",MSG_SHOOT);
 tolua_constant(tolua_S,NULL,"MSG_QUAFF",MSG_QUAFF);
 tolua_constant(tolua_S,NULL,"MSG_ZAP",MSG_ZAP);
 tolua_constant(tolua_S,NULL,"MSG_WALK",MSG_WALK);
 tolua_constant(tolua_S,NULL,"MSG_TPOTHER",MSG_TPOTHER);
 tolua_constant(tolua_S,NULL,"MSG_HITWALL",MSG_HITWALL);
 tolua_constant(tolua_S,NULL,"MSG_EAT",MSG_EAT);
 tolua_constant(tolua_S,NULL,"MSG_STORE1",MSG_STORE1);
 tolua_constant(tolua_S,NULL,"MSG_STORE2",MSG_STORE2);
 tolua_constant(tolua_S,NULL,"MSG_STORE3",MSG_STORE3);
 tolua_constant(tolua_S,NULL,"MSG_STORE4",MSG_STORE4);
 tolua_constant(tolua_S,NULL,"MSG_DIG",MSG_DIG);
 tolua_constant(tolua_S,NULL,"MSG_OPENDOOR",MSG_OPENDOOR);
 tolua_constant(tolua_S,NULL,"MSG_SHUTDOOR",MSG_SHUTDOOR);
 tolua_constant(tolua_S,NULL,"MSG_TPLEVEL",MSG_TPLEVEL);
 tolua_constant(tolua_S,NULL,"MSG_BELL",MSG_BELL);
 tolua_constant(tolua_S,NULL,"MSG_NOTHING_TO_OPEN",MSG_NOTHING_TO_OPEN);
 tolua_constant(tolua_S,NULL,"MSG_LOCKPICK_FAIL",MSG_LOCKPICK_FAIL);
 tolua_constant(tolua_S,NULL,"MSG_STAIRS",MSG_STAIRS);
 tolua_constant(tolua_S,NULL,"MSG_HITPOINT_WARN",MSG_HITPOINT_WARN);
 tolua_constant(tolua_S,NULL,"MSG_MAX",MSG_MAX);
 tolua_constant(tolua_S,NULL,"PN_COMBINE",PN_COMBINE);
 tolua_constant(tolua_S,NULL,"PN_REORDER",PN_REORDER);
 tolua_constant(tolua_S,NULL,"PU_BONUS",PU_BONUS);
 tolua_constant(tolua_S,NULL,"PU_TORCH",PU_TORCH);
 tolua_constant(tolua_S,NULL,"PU_HP",PU_HP);
 tolua_constant(tolua_S,NULL,"PU_MANA",PU_MANA);
 tolua_constant(tolua_S,NULL,"PU_SPELLS",PU_SPELLS);
 tolua_constant(tolua_S,NULL,"PU_FORGET_VIEW",PU_FORGET_VIEW);
 tolua_constant(tolua_S,NULL,"PU_UPDATE_VIEW",PU_UPDATE_VIEW);
 tolua_constant(tolua_S,NULL,"PU_FORGET_FLOW",PU_FORGET_FLOW);
 tolua_constant(tolua_S,NULL,"PU_UPDATE_FLOW",PU_UPDATE_FLOW);
 tolua_constant(tolua_S,NULL,"PU_MONSTERS",PU_MONSTERS);
 tolua_constant(tolua_S,NULL,"PU_DISTANCE",PU_DISTANCE);
 tolua_constant(tolua_S,NULL,"PU_PANEL",PU_PANEL);
 tolua_constant(tolua_S,NULL,"PR_MISC",PR_MISC);
 tolua_constant(tolua_S,NULL,"PR_TITLE",PR_TITLE);
 tolua_constant(tolua_S,NULL,"PR_LEV",PR_LEV);
 tolua_constant(tolua_S,NULL,"PR_EXP",PR_EXP);
 tolua_constant(tolua_S,NULL,"PR_STATS",PR_STATS);
 tolua_constant(tolua_S,NULL,"PR_ARMOR",PR_ARMOR);
 tolua_constant(tolua_S,NULL,"PR_HP",PR_HP);
 tolua_constant(tolua_S,NULL,"PR_MANA",PR_MANA);
 tolua_constant(tolua_S,NULL,"PR_GOLD",PR_GOLD);
 tolua_constant(tolua_S,NULL,"PR_DEPTH",PR_DEPTH);
 tolua_constant(tolua_S,NULL,"PR_HEALTH",PR_HEALTH);
 tolua_constant(tolua_S,NULL,"PR_CUT",PR_CUT);
 tolua_constant(tolua_S,NULL,"PR_STUN",PR_STUN);
 tolua_constant(tolua_S,NULL,"PR_HUNGER",PR_HUNGER);
 tolua_constant(tolua_S,NULL,"PR_BLIND",PR_BLIND);
 tolua_constant(tolua_S,NULL,"PR_CONFUSED",PR_CONFUSED);
 tolua_constant(tolua_S,NULL,"PR_AFRAID",PR_AFRAID);
 tolua_constant(tolua_S,NULL,"PR_POISONED",PR_POISONED);
 tolua_constant(tolua_S,NULL,"PR_STATE",PR_STATE);
 tolua_constant(tolua_S,NULL,"PR_SPEED",PR_SPEED);
 tolua_constant(tolua_S,NULL,"PR_STUDY",PR_STUDY);
 tolua_constant(tolua_S,NULL,"PR_EXTRA",PR_EXTRA);
 tolua_constant(tolua_S,NULL,"PR_BASIC",PR_BASIC);
 tolua_constant(tolua_S,NULL,"PR_MAP",PR_MAP);
 tolua_constant(tolua_S,NULL,"PW_INVEN",PW_INVEN);
 tolua_constant(tolua_S,NULL,"PW_EQUIP",PW_EQUIP);
 tolua_constant(tolua_S,NULL,"PW_PLAYER_0",PW_PLAYER_0);
 tolua_constant(tolua_S,NULL,"PW_PLAYER_1",PW_PLAYER_1);
 tolua_constant(tolua_S,NULL,"PW_MESSAGE",PW_MESSAGE);
 tolua_constant(tolua_S,NULL,"PW_OVERHEAD",PW_OVERHEAD);
 tolua_constant(tolua_S,NULL,"PW_MONSTER",PW_MONSTER);
 tolua_constant(tolua_S,NULL,"PW_OBJECT",PW_OBJECT);
 tolua_constant(tolua_S,NULL,"PW_SNAPSHOT",PW_SNAPSHOT);
 tolua_constant(tolua_S,NULL,"PW_BORG_1",PW_BORG_1);
 tolua_constant(tolua_S,NULL,"PW_BORG_2",PW_BORG_2);
 tolua_function(tolua_S,NULL,"notice_stuff",toluaI_ui_notice_stuff00);
 tolua_function(tolua_S,NULL,"update_stuff",toluaI_ui_update_stuff00);
 tolua_function(tolua_S,NULL,"redraw_stuff",toluaI_ui_redraw_stuff00);
 tolua_function(tolua_S,NULL,"window_stuff",toluaI_ui_window_stuff00);
 tolua_function(tolua_S,NULL,"handle_stuff",toluaI_ui_handle_stuff00);
 tolua_function(tolua_S,NULL,"inkey",toluaI_ui_inkey00);
 tolua_function(tolua_S,NULL,"bell",toluaI_ui_bell00);
 tolua_function(tolua_S,NULL,"sound",toluaI_ui_sound00);
 tolua_function(tolua_S,NULL,"msg_print",toluaI_ui_msg_print00);
 tolua_function(tolua_S,NULL,"message",toluaI_ui_message00);
 tolua_function(tolua_S,NULL,"message_flush",toluaI_ui_message_flush00);
 tolua_function(tolua_S,NULL,"screen_save",toluaI_ui_screen_save00);
 tolua_function(tolua_S,NULL,"screen_load",toluaI_ui_screen_load00);
 tolua_function(tolua_S,NULL,"c_put_str",toluaI_ui_c_put_str00);
 tolua_function(tolua_S,NULL,"put_str",toluaI_ui_put_str00);
 tolua_function(tolua_S,NULL,"c_prt",toluaI_ui_c_prt00);
 tolua_function(tolua_S,NULL,"prt",toluaI_ui_prt00);
 tolua_function(tolua_S,NULL,"clear_from",toluaI_ui_clear_from00);
 tolua_function(tolua_S,NULL,"pause_line",toluaI_ui_pause_line00);
 tolua_function(tolua_S,NULL,"request_command",toluaI_ui_request_command00);
 tolua_function(tolua_S,NULL,"text_out",toluaI_ui_text_out00);
 tolua_function(tolua_S,NULL,"Term_clear",toluaI_ui_Term_clear00);
 return 1;
}
/* Close function */
void tolua_ui_close (lua_State* tolua_S)
{
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TERM_DARK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TERM_WHITE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TERM_SLATE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TERM_ORANGE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TERM_RED");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TERM_GREEN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TERM_BLUE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TERM_UMBER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TERM_L_DARK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TERM_L_WHITE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TERM_VIOLET");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TERM_YELLOW");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TERM_L_RED");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TERM_L_GREEN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TERM_L_BLUE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TERM_L_UMBER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_GENERIC");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_HIT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_MISS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_FLEE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_DROP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_KILL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_LEVEL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_DEATH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_STUDY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_TELEPORT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_SHOOT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_QUAFF");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_ZAP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_WALK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_TPOTHER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_HITWALL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_EAT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_STORE1");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_STORE2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_STORE3");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_STORE4");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_DIG");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_OPENDOOR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_SHUTDOOR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_TPLEVEL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_BELL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_NOTHING_TO_OPEN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_LOCKPICK_FAIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_STAIRS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_HITPOINT_WARN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MSG_MAX");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PN_COMBINE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PN_REORDER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PU_BONUS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PU_TORCH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PU_HP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PU_MANA");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PU_SPELLS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PU_FORGET_VIEW");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PU_UPDATE_VIEW");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PU_FORGET_FLOW");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PU_UPDATE_FLOW");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PU_MONSTERS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PU_DISTANCE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PU_PANEL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PR_MISC");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PR_TITLE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PR_LEV");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PR_EXP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PR_STATS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PR_ARMOR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PR_HP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PR_MANA");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PR_GOLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PR_DEPTH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PR_HEALTH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PR_CUT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PR_STUN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PR_HUNGER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PR_BLIND");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PR_CONFUSED");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PR_AFRAID");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PR_POISONED");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PR_STATE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PR_SPEED");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PR_STUDY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PR_EXTRA");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PR_BASIC");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PR_MAP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PW_INVEN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PW_EQUIP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PW_PLAYER_0");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PW_PLAYER_1");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PW_MESSAGE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PW_OVERHEAD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PW_MONSTER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PW_OBJECT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PW_SNAPSHOT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PW_BORG_1");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PW_BORG_2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"notice_stuff");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"update_stuff");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"redraw_stuff");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"window_stuff");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"handle_stuff");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"inkey");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"bell");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"sound");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"msg_print");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"message");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"message_flush");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"screen_save");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"screen_load");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"c_put_str");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"put_str");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"c_prt");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"prt");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"clear_from");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"pause_line");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"request_command");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"text_out");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"Term_clear");
}
