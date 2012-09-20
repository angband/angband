/*
** Lua binding: spell
** Generated automatically by tolua 5.0a on Sat Jun 18 20:19:19 2005.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "lua/tolua.h"

/* Exported function */
TOLUA_API int tolua_spell_open (lua_State* tolua_S);

#include "angband.h"

/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"object_type");
}

/* function: poly_r_idx */
static int tolua_spell_poly_r_idx00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int r_idx = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  s16b tolua_ret = (s16b)  poly_r_idx(r_idx);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'poly_r_idx'.",&tolua_err);
 return 0;
#endif
}

/* function: teleport_away */
static int tolua_spell_teleport_away00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int m_idx = ((int)  tolua_tonumber(tolua_S,1,0));
  int dis = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  teleport_away(m_idx,dis);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'teleport_away'.",&tolua_err);
 return 0;
#endif
}

/* function: teleport_player */
static int tolua_spell_teleport_player00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dis = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  teleport_player(dis);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'teleport_player'.",&tolua_err);
 return 0;
#endif
}

/* function: teleport_player_to */
static int tolua_spell_teleport_player_to00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int ny = ((int)  tolua_tonumber(tolua_S,1,0));
  int nx = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  teleport_player_to(ny,nx);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'teleport_player_to'.",&tolua_err);
 return 0;
#endif
}

/* function: teleport_player_level */
static int tolua_spell_teleport_player_level00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  teleport_player_level();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'teleport_player_level'.",&tolua_err);
 return 0;
#endif
}

/* function: take_hit */
static int tolua_spell_take_hit00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dam = ((int)  tolua_tonumber(tolua_S,1,0));
  cptr kb_str = ((cptr)  tolua_tostring(tolua_S,2,0));
 {
  take_hit(dam,kb_str);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'take_hit'.",&tolua_err);
 return 0;
#endif
}

/* function: acid_dam */
static int tolua_spell_acid_dam00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dam = ((int)  tolua_tonumber(tolua_S,1,0));
  cptr kb_str = ((cptr)  tolua_tostring(tolua_S,2,0));
 {
  acid_dam(dam,kb_str);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'acid_dam'.",&tolua_err);
 return 0;
#endif
}

/* function: elec_dam */
static int tolua_spell_elec_dam00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dam = ((int)  tolua_tonumber(tolua_S,1,0));
  cptr kb_str = ((cptr)  tolua_tostring(tolua_S,2,0));
 {
  elec_dam(dam,kb_str);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'elec_dam'.",&tolua_err);
 return 0;
#endif
}

/* function: fire_dam */
static int tolua_spell_fire_dam00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dam = ((int)  tolua_tonumber(tolua_S,1,0));
  cptr kb_str = ((cptr)  tolua_tostring(tolua_S,2,0));
 {
  fire_dam(dam,kb_str);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'fire_dam'.",&tolua_err);
 return 0;
#endif
}

/* function: cold_dam */
static int tolua_spell_cold_dam00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dam = ((int)  tolua_tonumber(tolua_S,1,0));
  cptr kb_str = ((cptr)  tolua_tostring(tolua_S,2,0));
 {
  cold_dam(dam,kb_str);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'cold_dam'.",&tolua_err);
 return 0;
#endif
}

/* function: apply_disenchant */
static int tolua_spell_apply_disenchant00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int mode = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  apply_disenchant(mode);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'apply_disenchant'.",&tolua_err);
 return 0;
#endif
}

/* function: project */
static int tolua_spell_project00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,6,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,7,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,8,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int who = ((int)  tolua_tonumber(tolua_S,1,0));
  int rad = ((int)  tolua_tonumber(tolua_S,2,0));
  int y = ((int)  tolua_tonumber(tolua_S,3,0));
  int x = ((int)  tolua_tonumber(tolua_S,4,0));
  int dam = ((int)  tolua_tonumber(tolua_S,5,0));
  int typ = ((int)  tolua_tonumber(tolua_S,6,0));
  int flg = ((int)  tolua_tonumber(tolua_S,7,0));
 {
  bool tolua_ret = (bool)  project(who,rad,y,x,dam,typ,flg);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'project'.",&tolua_err);
 return 0;
#endif
}

/* function: warding_glyph */
static int tolua_spell_warding_glyph00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  warding_glyph();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'warding_glyph'.",&tolua_err);
 return 0;
#endif
}

/* function: identify_pack */
static int tolua_spell_identify_pack00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  identify_pack();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'identify_pack'.",&tolua_err);
 return 0;
#endif
}

/* function: remove_curse */
static int tolua_spell_remove_curse00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  remove_curse();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'remove_curse'.",&tolua_err);
 return 0;
#endif
}

/* function: remove_all_curse */
static int tolua_spell_remove_all_curse00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  remove_all_curse();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'remove_all_curse'.",&tolua_err);
 return 0;
#endif
}

/* function: self_knowledge */
static int tolua_spell_self_knowledge00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  self_knowledge();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'self_knowledge'.",&tolua_err);
 return 0;
#endif
}

/* function: lose_all_info */
static int tolua_spell_lose_all_info00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  lose_all_info();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'lose_all_info'.",&tolua_err);
 return 0;
#endif
}

/* function: set_recall */
static int tolua_spell_set_recall00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  set_recall();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_recall'.",&tolua_err);
 return 0;
#endif
}

/* function: detect_traps */
static int tolua_spell_detect_traps00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  detect_traps();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'detect_traps'.",&tolua_err);
 return 0;
#endif
}

/* function: detect_doors */
static int tolua_spell_detect_doors00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  detect_doors();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'detect_doors'.",&tolua_err);
 return 0;
#endif
}

/* function: detect_stairs */
static int tolua_spell_detect_stairs00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  detect_stairs();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'detect_stairs'.",&tolua_err);
 return 0;
#endif
}

/* function: detect_treasure */
static int tolua_spell_detect_treasure00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  detect_treasure();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'detect_treasure'.",&tolua_err);
 return 0;
#endif
}

/* function: detect_objects_gold */
static int tolua_spell_detect_objects_gold00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  detect_objects_gold();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'detect_objects_gold'.",&tolua_err);
 return 0;
#endif
}

/* function: detect_objects_normal */
static int tolua_spell_detect_objects_normal00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  detect_objects_normal();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'detect_objects_normal'.",&tolua_err);
 return 0;
#endif
}

/* function: detect_objects_magic */
static int tolua_spell_detect_objects_magic00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  detect_objects_magic();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'detect_objects_magic'.",&tolua_err);
 return 0;
#endif
}

/* function: detect_monsters_normal */
static int tolua_spell_detect_monsters_normal00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  detect_monsters_normal();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'detect_monsters_normal'.",&tolua_err);
 return 0;
#endif
}

/* function: detect_monsters_invis */
static int tolua_spell_detect_monsters_invis00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  detect_monsters_invis();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'detect_monsters_invis'.",&tolua_err);
 return 0;
#endif
}

/* function: detect_monsters_evil */
static int tolua_spell_detect_monsters_evil00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  detect_monsters_evil();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'detect_monsters_evil'.",&tolua_err);
 return 0;
#endif
}

/* function: detect_all */
static int tolua_spell_detect_all00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  detect_all();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'detect_all'.",&tolua_err);
 return 0;
#endif
}

/* function: stair_creation */
static int tolua_spell_stair_creation00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  stair_creation();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'stair_creation'.",&tolua_err);
 return 0;
#endif
}

/* function: enchant */
static int tolua_spell_enchant00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"object_type",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  object_type* o_ptr = ((object_type*)  tolua_tousertype(tolua_S,1,0));
  int n = ((int)  tolua_tonumber(tolua_S,2,0));
  int eflag = ((int)  tolua_tonumber(tolua_S,3,0));
 {
  bool tolua_ret = (bool)  enchant(o_ptr,n,eflag);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'enchant'.",&tolua_err);
 return 0;
#endif
}

/* function: enchant_spell */
static int tolua_spell_enchant_spell00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int num_hit = ((int)  tolua_tonumber(tolua_S,1,0));
  int num_dam = ((int)  tolua_tonumber(tolua_S,2,0));
  int num_ac = ((int)  tolua_tonumber(tolua_S,3,0));
 {
  bool tolua_ret = (bool)  enchant_spell(num_hit,num_dam,num_ac);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'enchant_spell'.",&tolua_err);
 return 0;
#endif
}

/* function: ident_spell */
static int tolua_spell_ident_spell00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  ident_spell();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ident_spell'.",&tolua_err);
 return 0;
#endif
}

/* function: identify_fully */
static int tolua_spell_identify_fully00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  identify_fully();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'identify_fully'.",&tolua_err);
 return 0;
#endif
}

/* function: recharge */
static int tolua_spell_recharge00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int num = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  recharge(num);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'recharge'.",&tolua_err);
 return 0;
#endif
}

/* function: speed_monsters */
static int tolua_spell_speed_monsters00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  speed_monsters();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'speed_monsters'.",&tolua_err);
 return 0;
#endif
}

/* function: slow_monsters */
static int tolua_spell_slow_monsters00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  slow_monsters();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'slow_monsters'.",&tolua_err);
 return 0;
#endif
}

/* function: sleep_monsters */
static int tolua_spell_sleep_monsters00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  sleep_monsters();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'sleep_monsters'.",&tolua_err);
 return 0;
#endif
}

/* function: banish_evil */
static int tolua_spell_banish_evil00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dist = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  banish_evil(dist);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'banish_evil'.",&tolua_err);
 return 0;
#endif
}

/* function: turn_undead */
static int tolua_spell_turn_undead00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  turn_undead();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'turn_undead'.",&tolua_err);
 return 0;
#endif
}

/* function: dispel_undead */
static int tolua_spell_dispel_undead00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dam = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  dispel_undead(dam);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'dispel_undead'.",&tolua_err);
 return 0;
#endif
}

/* function: dispel_evil */
static int tolua_spell_dispel_evil00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dam = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  dispel_evil(dam);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'dispel_evil'.",&tolua_err);
 return 0;
#endif
}

/* function: dispel_monsters */
static int tolua_spell_dispel_monsters00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dam = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  dispel_monsters(dam);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'dispel_monsters'.",&tolua_err);
 return 0;
#endif
}

/* function: aggravate_monsters */
static int tolua_spell_aggravate_monsters00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int who = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  aggravate_monsters(who);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'aggravate_monsters'.",&tolua_err);
 return 0;
#endif
}

/* function: banishment */
static int tolua_spell_banishment00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  banishment();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'banishment'.",&tolua_err);
 return 0;
#endif
}

/* function: mass_banishment */
static int tolua_spell_mass_banishment00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  mass_banishment();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'mass_banishment'.",&tolua_err);
 return 0;
#endif
}

/* function: probing */
static int tolua_spell_probing00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  probing();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'probing'.",&tolua_err);
 return 0;
#endif
}

/* function: destroy_area */
static int tolua_spell_destroy_area00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isboolean(tolua_S,4,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int y1 = ((int)  tolua_tonumber(tolua_S,1,0));
  int x1 = ((int)  tolua_tonumber(tolua_S,2,0));
  int r = ((int)  tolua_tonumber(tolua_S,3,0));
  bool full = ((bool)  tolua_toboolean(tolua_S,4,0));
 {
  destroy_area(y1,x1,r,full);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'destroy_area'.",&tolua_err);
 return 0;
#endif
}

/* function: earthquake */
static int tolua_spell_earthquake00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int cy = ((int)  tolua_tonumber(tolua_S,1,0));
  int cx = ((int)  tolua_tonumber(tolua_S,2,0));
  int r = ((int)  tolua_tonumber(tolua_S,3,0));
 {
  earthquake(cy,cx,r);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'earthquake'.",&tolua_err);
 return 0;
#endif
}

/* function: lite_room */
static int tolua_spell_lite_room00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int y1 = ((int)  tolua_tonumber(tolua_S,1,0));
  int x1 = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  lite_room(y1,x1);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'lite_room'.",&tolua_err);
 return 0;
#endif
}

/* function: unlite_room */
static int tolua_spell_unlite_room00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int y1 = ((int)  tolua_tonumber(tolua_S,1,0));
  int x1 = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  unlite_room(y1,x1);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'unlite_room'.",&tolua_err);
 return 0;
#endif
}

/* function: lite_area */
static int tolua_spell_lite_area00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dam = ((int)  tolua_tonumber(tolua_S,1,0));
  int rad = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  bool tolua_ret = (bool)  lite_area(dam,rad);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'lite_area'.",&tolua_err);
 return 0;
#endif
}

/* function: unlite_area */
static int tolua_spell_unlite_area00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dam = ((int)  tolua_tonumber(tolua_S,1,0));
  int rad = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  bool tolua_ret = (bool)  unlite_area(dam,rad);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'unlite_area'.",&tolua_err);
 return 0;
#endif
}

/* function: fire_ball */
static int tolua_spell_fire_ball00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int typ = ((int)  tolua_tonumber(tolua_S,1,0));
  int dir = ((int)  tolua_tonumber(tolua_S,2,0));
  int dam = ((int)  tolua_tonumber(tolua_S,3,0));
  int rad = ((int)  tolua_tonumber(tolua_S,4,0));
 {
  bool tolua_ret = (bool)  fire_ball(typ,dir,dam,rad);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'fire_ball'.",&tolua_err);
 return 0;
#endif
}

/* function: fire_swarm */
static int tolua_spell_fire_swarm00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,6,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int num = ((int)  tolua_tonumber(tolua_S,1,0));
  int typ = ((int)  tolua_tonumber(tolua_S,2,0));
  int dir = ((int)  tolua_tonumber(tolua_S,3,0));
  int dam = ((int)  tolua_tonumber(tolua_S,4,0));
  int rad = ((int)  tolua_tonumber(tolua_S,5,0));
 {
  bool tolua_ret = (bool)  fire_swarm(num,typ,dir,dam,rad);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'fire_swarm'.",&tolua_err);
 return 0;
#endif
}

/* function: fire_bolt */
static int tolua_spell_fire_bolt00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int typ = ((int)  tolua_tonumber(tolua_S,1,0));
  int dir = ((int)  tolua_tonumber(tolua_S,2,0));
  int dam = ((int)  tolua_tonumber(tolua_S,3,0));
 {
  bool tolua_ret = (bool)  fire_bolt(typ,dir,dam);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'fire_bolt'.",&tolua_err);
 return 0;
#endif
}

/* function: fire_beam */
static int tolua_spell_fire_beam00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int typ = ((int)  tolua_tonumber(tolua_S,1,0));
  int dir = ((int)  tolua_tonumber(tolua_S,2,0));
  int dam = ((int)  tolua_tonumber(tolua_S,3,0));
 {
  bool tolua_ret = (bool)  fire_beam(typ,dir,dam);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'fire_beam'.",&tolua_err);
 return 0;
#endif
}

/* function: fire_bolt_or_beam */
static int tolua_spell_fire_bolt_or_beam00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int prob = ((int)  tolua_tonumber(tolua_S,1,0));
  int typ = ((int)  tolua_tonumber(tolua_S,2,0));
  int dir = ((int)  tolua_tonumber(tolua_S,3,0));
  int dam = ((int)  tolua_tonumber(tolua_S,4,0));
 {
  bool tolua_ret = (bool)  fire_bolt_or_beam(prob,typ,dir,dam);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'fire_bolt_or_beam'.",&tolua_err);
 return 0;
#endif
}

/* function: project_los */
static int tolua_spell_project_los00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int typ = ((int)  tolua_tonumber(tolua_S,1,0));
  int dam = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  bool tolua_ret = (bool)  project_los(typ,dam);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'project_los'.",&tolua_err);
 return 0;
#endif
}

/* function: lite_line */
static int tolua_spell_lite_line00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dir = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  lite_line(dir);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'lite_line'.",&tolua_err);
 return 0;
#endif
}

/* function: strong_lite_line */
static int tolua_spell_strong_lite_line00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dir = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  strong_lite_line(dir);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'strong_lite_line'.",&tolua_err);
 return 0;
#endif
}

/* function: drain_life */
static int tolua_spell_drain_life00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dir = ((int)  tolua_tonumber(tolua_S,1,0));
  int dam = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  bool tolua_ret = (bool)  drain_life(dir,dam);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'drain_life'.",&tolua_err);
 return 0;
#endif
}

/* function: wall_to_mud */
static int tolua_spell_wall_to_mud00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dir = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  wall_to_mud(dir);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'wall_to_mud'.",&tolua_err);
 return 0;
#endif
}

/* function: destroy_door */
static int tolua_spell_destroy_door00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dir = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  destroy_door(dir);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'destroy_door'.",&tolua_err);
 return 0;
#endif
}

/* function: disarm_trap */
static int tolua_spell_disarm_trap00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dir = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  disarm_trap(dir);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'disarm_trap'.",&tolua_err);
 return 0;
#endif
}

/* function: heal_monster */
static int tolua_spell_heal_monster00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dir = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  heal_monster(dir);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'heal_monster'.",&tolua_err);
 return 0;
#endif
}

/* function: speed_monster */
static int tolua_spell_speed_monster00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dir = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  speed_monster(dir);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'speed_monster'.",&tolua_err);
 return 0;
#endif
}

/* function: slow_monster */
static int tolua_spell_slow_monster00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dir = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  slow_monster(dir);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'slow_monster'.",&tolua_err);
 return 0;
#endif
}

/* function: sleep_monster */
static int tolua_spell_sleep_monster00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dir = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  sleep_monster(dir);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'sleep_monster'.",&tolua_err);
 return 0;
#endif
}

/* function: confuse_monster */
static int tolua_spell_confuse_monster00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dir = ((int)  tolua_tonumber(tolua_S,1,0));
  int plev = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  bool tolua_ret = (bool)  confuse_monster(dir,plev);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'confuse_monster'.",&tolua_err);
 return 0;
#endif
}

/* function: poly_monster */
static int tolua_spell_poly_monster00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dir = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  poly_monster(dir);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'poly_monster'.",&tolua_err);
 return 0;
#endif
}

/* function: clone_monster */
static int tolua_spell_clone_monster00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dir = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  clone_monster(dir);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'clone_monster'.",&tolua_err);
 return 0;
#endif
}

/* function: fear_monster */
static int tolua_spell_fear_monster00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dir = ((int)  tolua_tonumber(tolua_S,1,0));
  int plev = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  bool tolua_ret = (bool)  fear_monster(dir,plev);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'fear_monster'.",&tolua_err);
 return 0;
#endif
}

/* function: teleport_monster */
static int tolua_spell_teleport_monster00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dir = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  teleport_monster(dir);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'teleport_monster'.",&tolua_err);
 return 0;
#endif
}

/* function: door_creation */
static int tolua_spell_door_creation00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  door_creation();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'door_creation'.",&tolua_err);
 return 0;
#endif
}

/* function: trap_creation */
static int tolua_spell_trap_creation00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  trap_creation();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'trap_creation'.",&tolua_err);
 return 0;
#endif
}

/* function: destroy_doors_touch */
static int tolua_spell_destroy_doors_touch00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  destroy_doors_touch();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'destroy_doors_touch'.",&tolua_err);
 return 0;
#endif
}

/* function: sleep_monsters_touch */
static int tolua_spell_sleep_monsters_touch00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  sleep_monsters_touch();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'sleep_monsters_touch'.",&tolua_err);
 return 0;
#endif
}

/* function: curse_armor */
static int tolua_spell_curse_armor00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  curse_armor();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'curse_armor'.",&tolua_err);
 return 0;
#endif
}

/* function: curse_weapon */
static int tolua_spell_curse_weapon00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  curse_weapon();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'curse_weapon'.",&tolua_err);
 return 0;
#endif
}

/* function: brand_object */
static int tolua_spell_brand_object00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"object_type",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  object_type* o_ptr = ((object_type*)  tolua_tousertype(tolua_S,1,0));
  byte brand_type = ((byte)  tolua_tonumber(tolua_S,2,0));
 {
  brand_object(o_ptr,brand_type);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'brand_object'.",&tolua_err);
 return 0;
#endif
}

/* function: brand_weapon */
static int tolua_spell_brand_weapon00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  brand_weapon();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'brand_weapon'.",&tolua_err);
 return 0;
#endif
}

/* function: brand_ammo */
static int tolua_spell_brand_ammo00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  brand_ammo();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'brand_ammo'.",&tolua_err);
 return 0;
#endif
}

/* function: brand_bolts */
static int tolua_spell_brand_bolts00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  brand_bolts();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'brand_bolts'.",&tolua_err);
 return 0;
#endif
}

/* function: ring_of_power */
static int tolua_spell_ring_of_power00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dir = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  ring_of_power(dir);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ring_of_power'.",&tolua_err);
 return 0;
#endif
}

/* function: map_area */
static int tolua_spell_map_area00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  map_area();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'map_area'.",&tolua_err);
 return 0;
#endif
}

/* function: wiz_lite */
static int tolua_spell_wiz_lite00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  wiz_lite();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'wiz_lite'.",&tolua_err);
 return 0;
#endif
}

/* function: wiz_dark */
static int tolua_spell_wiz_dark00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  wiz_dark();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'wiz_dark'.",&tolua_err);
 return 0;
#endif
}

/* Open function */
TOLUA_API int tolua_spell_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,0);
 tolua_beginmodule(tolua_S,NULL);
 tolua_constant(tolua_S,"SUMMON_ANIMAL",SUMMON_ANIMAL);
 tolua_constant(tolua_S,"SUMMON_SPIDER",SUMMON_SPIDER);
 tolua_constant(tolua_S,"SUMMON_HOUND",SUMMON_HOUND);
 tolua_constant(tolua_S,"SUMMON_HYDRA",SUMMON_HYDRA);
 tolua_constant(tolua_S,"SUMMON_ANGEL",SUMMON_ANGEL);
 tolua_constant(tolua_S,"SUMMON_DEMON",SUMMON_DEMON);
 tolua_constant(tolua_S,"SUMMON_UNDEAD",SUMMON_UNDEAD);
 tolua_constant(tolua_S,"SUMMON_DRAGON",SUMMON_DRAGON);
 tolua_constant(tolua_S,"SUMMON_HI_DEMON",SUMMON_HI_DEMON);
 tolua_constant(tolua_S,"SUMMON_HI_UNDEAD",SUMMON_HI_UNDEAD);
 tolua_constant(tolua_S,"SUMMON_HI_DRAGON",SUMMON_HI_DRAGON);
 tolua_constant(tolua_S,"SUMMON_WRAITH",SUMMON_WRAITH);
 tolua_constant(tolua_S,"SUMMON_UNIQUE",SUMMON_UNIQUE);
 tolua_constant(tolua_S,"SUMMON_KIN",SUMMON_KIN);
 tolua_constant(tolua_S,"GF_XXX1",GF_XXX1);
 tolua_constant(tolua_S,"GF_ARROW",GF_ARROW);
 tolua_constant(tolua_S,"GF_MISSILE",GF_MISSILE);
 tolua_constant(tolua_S,"GF_MANA",GF_MANA);
 tolua_constant(tolua_S,"GF_HOLY_ORB",GF_HOLY_ORB);
 tolua_constant(tolua_S,"GF_LITE_WEAK",GF_LITE_WEAK);
 tolua_constant(tolua_S,"GF_DARK_WEAK",GF_DARK_WEAK);
 tolua_constant(tolua_S,"GF_WATER",GF_WATER);
 tolua_constant(tolua_S,"GF_PLASMA",GF_PLASMA);
 tolua_constant(tolua_S,"GF_METEOR",GF_METEOR);
 tolua_constant(tolua_S,"GF_ICE",GF_ICE);
 tolua_constant(tolua_S,"GF_GRAVITY",GF_GRAVITY);
 tolua_constant(tolua_S,"GF_INERTIA",GF_INERTIA);
 tolua_constant(tolua_S,"GF_FORCE",GF_FORCE);
 tolua_constant(tolua_S,"GF_TIME",GF_TIME);
 tolua_constant(tolua_S,"GF_ACID",GF_ACID);
 tolua_constant(tolua_S,"GF_ELEC",GF_ELEC);
 tolua_constant(tolua_S,"GF_FIRE",GF_FIRE);
 tolua_constant(tolua_S,"GF_COLD",GF_COLD);
 tolua_constant(tolua_S,"GF_POIS",GF_POIS);
 tolua_constant(tolua_S,"GF_XXX2",GF_XXX2);
 tolua_constant(tolua_S,"GF_LITE",GF_LITE);
 tolua_constant(tolua_S,"GF_DARK",GF_DARK);
 tolua_constant(tolua_S,"GF_XXX3",GF_XXX3);
 tolua_constant(tolua_S,"GF_CONFUSION",GF_CONFUSION);
 tolua_constant(tolua_S,"GF_SOUND",GF_SOUND);
 tolua_constant(tolua_S,"GF_SHARD",GF_SHARD);
 tolua_constant(tolua_S,"GF_NEXUS",GF_NEXUS);
 tolua_constant(tolua_S,"GF_NETHER",GF_NETHER);
 tolua_constant(tolua_S,"GF_CHAOS",GF_CHAOS);
 tolua_constant(tolua_S,"GF_DISENCHANT",GF_DISENCHANT);
 tolua_constant(tolua_S,"GF_XXX4",GF_XXX4);
 tolua_constant(tolua_S,"GF_KILL_WALL",GF_KILL_WALL);
 tolua_constant(tolua_S,"GF_KILL_DOOR",GF_KILL_DOOR);
 tolua_constant(tolua_S,"GF_KILL_TRAP",GF_KILL_TRAP);
 tolua_constant(tolua_S,"GF_MAKE_WALL",GF_MAKE_WALL);
 tolua_constant(tolua_S,"GF_MAKE_DOOR",GF_MAKE_DOOR);
 tolua_constant(tolua_S,"GF_MAKE_TRAP",GF_MAKE_TRAP);
 tolua_constant(tolua_S,"GF_XXX5",GF_XXX5);
 tolua_constant(tolua_S,"GF_XXX6",GF_XXX6);
 tolua_constant(tolua_S,"GF_AWAY_UNDEAD",GF_AWAY_UNDEAD);
 tolua_constant(tolua_S,"GF_AWAY_EVIL",GF_AWAY_EVIL);
 tolua_constant(tolua_S,"GF_AWAY_ALL",GF_AWAY_ALL);
 tolua_constant(tolua_S,"GF_TURN_UNDEAD",GF_TURN_UNDEAD);
 tolua_constant(tolua_S,"GF_TURN_EVIL",GF_TURN_EVIL);
 tolua_constant(tolua_S,"GF_TURN_ALL",GF_TURN_ALL);
 tolua_constant(tolua_S,"GF_DISP_UNDEAD",GF_DISP_UNDEAD);
 tolua_constant(tolua_S,"GF_DISP_EVIL",GF_DISP_EVIL);
 tolua_constant(tolua_S,"GF_DISP_ALL",GF_DISP_ALL);
 tolua_constant(tolua_S,"GF_XXX7",GF_XXX7);
 tolua_constant(tolua_S,"GF_OLD_CLONE",GF_OLD_CLONE);
 tolua_constant(tolua_S,"GF_OLD_POLY",GF_OLD_POLY);
 tolua_constant(tolua_S,"GF_OLD_HEAL",GF_OLD_HEAL);
 tolua_constant(tolua_S,"GF_OLD_SPEED",GF_OLD_SPEED);
 tolua_constant(tolua_S,"GF_OLD_SLOW",GF_OLD_SLOW);
 tolua_constant(tolua_S,"GF_OLD_CONF",GF_OLD_CONF);
 tolua_constant(tolua_S,"GF_OLD_SLEEP",GF_OLD_SLEEP);
 tolua_constant(tolua_S,"GF_OLD_DRAIN",GF_OLD_DRAIN);
 tolua_constant(tolua_S,"GF_XXX8",GF_XXX8);
 tolua_function(tolua_S,"poly_r_idx",tolua_spell_poly_r_idx00);
 tolua_function(tolua_S,"teleport_away",tolua_spell_teleport_away00);
 tolua_function(tolua_S,"teleport_player",tolua_spell_teleport_player00);
 tolua_function(tolua_S,"teleport_player_to",tolua_spell_teleport_player_to00);
 tolua_function(tolua_S,"teleport_player_level",tolua_spell_teleport_player_level00);
 tolua_function(tolua_S,"take_hit",tolua_spell_take_hit00);
 tolua_function(tolua_S,"acid_dam",tolua_spell_acid_dam00);
 tolua_function(tolua_S,"elec_dam",tolua_spell_elec_dam00);
 tolua_function(tolua_S,"fire_dam",tolua_spell_fire_dam00);
 tolua_function(tolua_S,"cold_dam",tolua_spell_cold_dam00);
 tolua_function(tolua_S,"apply_disenchant",tolua_spell_apply_disenchant00);
 tolua_function(tolua_S,"project",tolua_spell_project00);
 tolua_function(tolua_S,"warding_glyph",tolua_spell_warding_glyph00);
 tolua_function(tolua_S,"identify_pack",tolua_spell_identify_pack00);
 tolua_function(tolua_S,"remove_curse",tolua_spell_remove_curse00);
 tolua_function(tolua_S,"remove_all_curse",tolua_spell_remove_all_curse00);
 tolua_function(tolua_S,"self_knowledge",tolua_spell_self_knowledge00);
 tolua_function(tolua_S,"lose_all_info",tolua_spell_lose_all_info00);
 tolua_function(tolua_S,"set_recall",tolua_spell_set_recall00);
 tolua_function(tolua_S,"detect_traps",tolua_spell_detect_traps00);
 tolua_function(tolua_S,"detect_doors",tolua_spell_detect_doors00);
 tolua_function(tolua_S,"detect_stairs",tolua_spell_detect_stairs00);
 tolua_function(tolua_S,"detect_treasure",tolua_spell_detect_treasure00);
 tolua_function(tolua_S,"detect_objects_gold",tolua_spell_detect_objects_gold00);
 tolua_function(tolua_S,"detect_objects_normal",tolua_spell_detect_objects_normal00);
 tolua_function(tolua_S,"detect_objects_magic",tolua_spell_detect_objects_magic00);
 tolua_function(tolua_S,"detect_monsters_normal",tolua_spell_detect_monsters_normal00);
 tolua_function(tolua_S,"detect_monsters_invis",tolua_spell_detect_monsters_invis00);
 tolua_function(tolua_S,"detect_monsters_evil",tolua_spell_detect_monsters_evil00);
 tolua_function(tolua_S,"detect_all",tolua_spell_detect_all00);
 tolua_function(tolua_S,"stair_creation",tolua_spell_stair_creation00);
 tolua_function(tolua_S,"enchant",tolua_spell_enchant00);
 tolua_function(tolua_S,"enchant_spell",tolua_spell_enchant_spell00);
 tolua_function(tolua_S,"ident_spell",tolua_spell_ident_spell00);
 tolua_function(tolua_S,"identify_fully",tolua_spell_identify_fully00);
 tolua_function(tolua_S,"recharge",tolua_spell_recharge00);
 tolua_function(tolua_S,"speed_monsters",tolua_spell_speed_monsters00);
 tolua_function(tolua_S,"slow_monsters",tolua_spell_slow_monsters00);
 tolua_function(tolua_S,"sleep_monsters",tolua_spell_sleep_monsters00);
 tolua_function(tolua_S,"banish_evil",tolua_spell_banish_evil00);
 tolua_function(tolua_S,"turn_undead",tolua_spell_turn_undead00);
 tolua_function(tolua_S,"dispel_undead",tolua_spell_dispel_undead00);
 tolua_function(tolua_S,"dispel_evil",tolua_spell_dispel_evil00);
 tolua_function(tolua_S,"dispel_monsters",tolua_spell_dispel_monsters00);
 tolua_function(tolua_S,"aggravate_monsters",tolua_spell_aggravate_monsters00);
 tolua_function(tolua_S,"banishment",tolua_spell_banishment00);
 tolua_function(tolua_S,"mass_banishment",tolua_spell_mass_banishment00);
 tolua_function(tolua_S,"probing",tolua_spell_probing00);
 tolua_function(tolua_S,"destroy_area",tolua_spell_destroy_area00);
 tolua_function(tolua_S,"earthquake",tolua_spell_earthquake00);
 tolua_function(tolua_S,"lite_room",tolua_spell_lite_room00);
 tolua_function(tolua_S,"unlite_room",tolua_spell_unlite_room00);
 tolua_function(tolua_S,"lite_area",tolua_spell_lite_area00);
 tolua_function(tolua_S,"unlite_area",tolua_spell_unlite_area00);
 tolua_function(tolua_S,"fire_ball",tolua_spell_fire_ball00);
 tolua_function(tolua_S,"fire_swarm",tolua_spell_fire_swarm00);
 tolua_function(tolua_S,"fire_bolt",tolua_spell_fire_bolt00);
 tolua_function(tolua_S,"fire_beam",tolua_spell_fire_beam00);
 tolua_function(tolua_S,"fire_bolt_or_beam",tolua_spell_fire_bolt_or_beam00);
 tolua_function(tolua_S,"project_los",tolua_spell_project_los00);
 tolua_function(tolua_S,"lite_line",tolua_spell_lite_line00);
 tolua_function(tolua_S,"strong_lite_line",tolua_spell_strong_lite_line00);
 tolua_function(tolua_S,"drain_life",tolua_spell_drain_life00);
 tolua_function(tolua_S,"wall_to_mud",tolua_spell_wall_to_mud00);
 tolua_function(tolua_S,"destroy_door",tolua_spell_destroy_door00);
 tolua_function(tolua_S,"disarm_trap",tolua_spell_disarm_trap00);
 tolua_function(tolua_S,"heal_monster",tolua_spell_heal_monster00);
 tolua_function(tolua_S,"speed_monster",tolua_spell_speed_monster00);
 tolua_function(tolua_S,"slow_monster",tolua_spell_slow_monster00);
 tolua_function(tolua_S,"sleep_monster",tolua_spell_sleep_monster00);
 tolua_function(tolua_S,"confuse_monster",tolua_spell_confuse_monster00);
 tolua_function(tolua_S,"poly_monster",tolua_spell_poly_monster00);
 tolua_function(tolua_S,"clone_monster",tolua_spell_clone_monster00);
 tolua_function(tolua_S,"fear_monster",tolua_spell_fear_monster00);
 tolua_function(tolua_S,"teleport_monster",tolua_spell_teleport_monster00);
 tolua_function(tolua_S,"door_creation",tolua_spell_door_creation00);
 tolua_function(tolua_S,"trap_creation",tolua_spell_trap_creation00);
 tolua_function(tolua_S,"destroy_doors_touch",tolua_spell_destroy_doors_touch00);
 tolua_function(tolua_S,"sleep_monsters_touch",tolua_spell_sleep_monsters_touch00);
 tolua_function(tolua_S,"curse_armor",tolua_spell_curse_armor00);
 tolua_function(tolua_S,"curse_weapon",tolua_spell_curse_weapon00);
 tolua_function(tolua_S,"brand_object",tolua_spell_brand_object00);
 tolua_function(tolua_S,"brand_weapon",tolua_spell_brand_weapon00);
 tolua_function(tolua_S,"brand_ammo",tolua_spell_brand_ammo00);
 tolua_function(tolua_S,"brand_bolts",tolua_spell_brand_bolts00);
 tolua_function(tolua_S,"ring_of_power",tolua_spell_ring_of_power00);
 tolua_function(tolua_S,"map_area",tolua_spell_map_area00);
 tolua_function(tolua_S,"wiz_lite",tolua_spell_wiz_lite00);
 tolua_function(tolua_S,"wiz_dark",tolua_spell_wiz_dark00);
 tolua_endmodule(tolua_S);
 return 1;
}
