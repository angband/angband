/*
** Lua binding: spell
** Generated automatically by tolua 4.0a - angband on Sun Jan 27 16:18:20 2002.
*/

#include "lua/tolua.h"

/* Exported function */
int tolua_spell_open (lua_State* tolua_S);
void tolua_spell_close (lua_State* tolua_S);

#include "angband.h"

/* function to register type */
static void toluaI_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"object_type");
}

/* error messages */
#define TOLUA_ERR_SELF tolua_error(tolua_S,"invalid 'self'")
#define TOLUA_ERR_ASSIGN tolua_error(tolua_S,"#vinvalid type in variable assignment.")

/* function: poly_r_idx */
static int toluaI_spell_poly_r_idx00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int r_idx = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  s16b toluaI_ret = (s16b)  poly_r_idx(r_idx);
 tolua_pushnumber(tolua_S,(long)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'poly_r_idx'.");
 return 0;
}

/* function: teleport_away */
static int toluaI_spell_teleport_away00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int m_idx = ((int)  tolua_getnumber(tolua_S,1,0));
  int dis = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  teleport_away(m_idx,dis);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'teleport_away'.");
 return 0;
}

/* function: teleport_player */
static int toluaI_spell_teleport_player00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int dis = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  teleport_player(dis);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'teleport_player'.");
 return 0;
}

/* function: teleport_player_to */
static int toluaI_spell_teleport_player_to00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int ny = ((int)  tolua_getnumber(tolua_S,1,0));
  int nx = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  teleport_player_to(ny,nx);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'teleport_player_to'.");
 return 0;
}

/* function: teleport_player_level */
static int toluaI_spell_teleport_player_level00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  teleport_player_level();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'teleport_player_level'.");
 return 0;
}

/* function: take_hit */
static int toluaI_spell_take_hit00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TSTRING,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int dam = ((int)  tolua_getnumber(tolua_S,1,0));
  cptr kb_str = ((cptr)  tolua_getstring(tolua_S,2,0));
 {
  take_hit(dam,kb_str);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'take_hit'.");
 return 0;
}

/* function: acid_dam */
static int toluaI_spell_acid_dam00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TSTRING,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int dam = ((int)  tolua_getnumber(tolua_S,1,0));
  cptr kb_str = ((cptr)  tolua_getstring(tolua_S,2,0));
 {
  acid_dam(dam,kb_str);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'acid_dam'.");
 return 0;
}

/* function: elec_dam */
static int toluaI_spell_elec_dam00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TSTRING,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int dam = ((int)  tolua_getnumber(tolua_S,1,0));
  cptr kb_str = ((cptr)  tolua_getstring(tolua_S,2,0));
 {
  elec_dam(dam,kb_str);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'elec_dam'.");
 return 0;
}

/* function: fire_dam */
static int toluaI_spell_fire_dam00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TSTRING,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int dam = ((int)  tolua_getnumber(tolua_S,1,0));
  cptr kb_str = ((cptr)  tolua_getstring(tolua_S,2,0));
 {
  fire_dam(dam,kb_str);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'fire_dam'.");
 return 0;
}

/* function: cold_dam */
static int toluaI_spell_cold_dam00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TSTRING,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int dam = ((int)  tolua_getnumber(tolua_S,1,0));
  cptr kb_str = ((cptr)  tolua_getstring(tolua_S,2,0));
 {
  cold_dam(dam,kb_str);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'cold_dam'.");
 return 0;
}

/* function: inc_stat */
static int toluaI_spell_inc_stat00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int stat = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  inc_stat(stat);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'inc_stat'.");
 return 0;
}

/* function: dec_stat */
static int toluaI_spell_dec_stat00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,4)
 )
 goto tolua_lerror;
 else
 {
  int stat = ((int)  tolua_getnumber(tolua_S,1,0));
  int amount = ((int)  tolua_getnumber(tolua_S,2,0));
  int permanent = ((int)  tolua_getnumber(tolua_S,3,0));
 {
  bool toluaI_ret = (bool)  dec_stat(stat,amount,permanent);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'dec_stat'.");
 return 0;
}

/* function: res_stat */
static int toluaI_spell_res_stat00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int stat = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  res_stat(stat);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'res_stat'.");
 return 0;
}

/* function: apply_disenchant */
static int toluaI_spell_apply_disenchant00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int mode = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  apply_disenchant(mode);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'apply_disenchant'.");
 return 0;
}

/* function: project */
static int toluaI_spell_project00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,4,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,5,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,6,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,7,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,8)
 )
 goto tolua_lerror;
 else
 {
  int who = ((int)  tolua_getnumber(tolua_S,1,0));
  int rad = ((int)  tolua_getnumber(tolua_S,2,0));
  int y = ((int)  tolua_getnumber(tolua_S,3,0));
  int x = ((int)  tolua_getnumber(tolua_S,4,0));
  int dam = ((int)  tolua_getnumber(tolua_S,5,0));
  int typ = ((int)  tolua_getnumber(tolua_S,6,0));
  int flg = ((int)  tolua_getnumber(tolua_S,7,0));
 {
  bool toluaI_ret = (bool)  project(who,rad,y,x,dam,typ,flg);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'project'.");
 return 0;
}

/* function: warding_glyph */
static int toluaI_spell_warding_glyph00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  warding_glyph();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'warding_glyph'.");
 return 0;
}

/* function: identify_pack */
static int toluaI_spell_identify_pack00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  identify_pack();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'identify_pack'.");
 return 0;
}

/* function: remove_curse */
static int toluaI_spell_remove_curse00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  remove_curse();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'remove_curse'.");
 return 0;
}

/* function: remove_all_curse */
static int toluaI_spell_remove_all_curse00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  remove_all_curse();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'remove_all_curse'.");
 return 0;
}

/* function: self_knowledge */
static int toluaI_spell_self_knowledge00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  self_knowledge();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'self_knowledge'.");
 return 0;
}

/* function: lose_all_info */
static int toluaI_spell_lose_all_info00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  lose_all_info();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'lose_all_info'.");
 return 0;
}

/* function: set_recall */
static int toluaI_spell_set_recall00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  set_recall();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_recall'.");
 return 0;
}

/* function: detect_traps */
static int toluaI_spell_detect_traps00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  detect_traps();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'detect_traps'.");
 return 0;
}

/* function: detect_doors */
static int toluaI_spell_detect_doors00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  detect_doors();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'detect_doors'.");
 return 0;
}

/* function: detect_stairs */
static int toluaI_spell_detect_stairs00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  detect_stairs();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'detect_stairs'.");
 return 0;
}

/* function: detect_treasure */
static int toluaI_spell_detect_treasure00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  detect_treasure();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'detect_treasure'.");
 return 0;
}

/* function: detect_objects_gold */
static int toluaI_spell_detect_objects_gold00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  detect_objects_gold();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'detect_objects_gold'.");
 return 0;
}

/* function: detect_objects_normal */
static int toluaI_spell_detect_objects_normal00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  detect_objects_normal();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'detect_objects_normal'.");
 return 0;
}

/* function: detect_objects_magic */
static int toluaI_spell_detect_objects_magic00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  detect_objects_magic();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'detect_objects_magic'.");
 return 0;
}

/* function: detect_monsters_normal */
static int toluaI_spell_detect_monsters_normal00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  detect_monsters_normal();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'detect_monsters_normal'.");
 return 0;
}

/* function: detect_monsters_invis */
static int toluaI_spell_detect_monsters_invis00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  detect_monsters_invis();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'detect_monsters_invis'.");
 return 0;
}

/* function: detect_monsters_evil */
static int toluaI_spell_detect_monsters_evil00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  detect_monsters_evil();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'detect_monsters_evil'.");
 return 0;
}

/* function: detect_all */
static int toluaI_spell_detect_all00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  detect_all();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'detect_all'.");
 return 0;
}

/* function: stair_creation */
static int toluaI_spell_stair_creation00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  stair_creation();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'stair_creation'.");
 return 0;
}

/* function: enchant */
static int toluaI_spell_enchant00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"object_type"),0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,4)
 )
 goto tolua_lerror;
 else
 {
  object_type* o_ptr = ((object_type*)  tolua_getusertype(tolua_S,1,0));
  int n = ((int)  tolua_getnumber(tolua_S,2,0));
  int eflag = ((int)  tolua_getnumber(tolua_S,3,0));
 {
  bool toluaI_ret = (bool)  enchant(o_ptr,n,eflag);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'enchant'.");
 return 0;
}

/* function: enchant_spell */
static int toluaI_spell_enchant_spell00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,4)
 )
 goto tolua_lerror;
 else
 {
  int num_hit = ((int)  tolua_getnumber(tolua_S,1,0));
  int num_dam = ((int)  tolua_getnumber(tolua_S,2,0));
  int num_ac = ((int)  tolua_getnumber(tolua_S,3,0));
 {
  bool toluaI_ret = (bool)  enchant_spell(num_hit,num_dam,num_ac);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'enchant_spell'.");
 return 0;
}

/* function: ident_spell */
static int toluaI_spell_ident_spell00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  ident_spell();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ident_spell'.");
 return 0;
}

/* function: identify_fully */
static int toluaI_spell_identify_fully00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  identify_fully();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'identify_fully'.");
 return 0;
}

/* function: recharge */
static int toluaI_spell_recharge00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int num = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  recharge(num);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'recharge'.");
 return 0;
}

/* function: speed_monsters */
static int toluaI_spell_speed_monsters00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  speed_monsters();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'speed_monsters'.");
 return 0;
}

/* function: slow_monsters */
static int toluaI_spell_slow_monsters00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  slow_monsters();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'slow_monsters'.");
 return 0;
}

/* function: sleep_monsters */
static int toluaI_spell_sleep_monsters00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  sleep_monsters();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'sleep_monsters'.");
 return 0;
}

/* function: banish_evil */
static int toluaI_spell_banish_evil00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int dist = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  banish_evil(dist);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'banish_evil'.");
 return 0;
}

/* function: turn_undead */
static int toluaI_spell_turn_undead00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  turn_undead();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'turn_undead'.");
 return 0;
}

/* function: dispel_undead */
static int toluaI_spell_dispel_undead00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int dam = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  dispel_undead(dam);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'dispel_undead'.");
 return 0;
}

/* function: dispel_evil */
static int toluaI_spell_dispel_evil00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int dam = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  dispel_evil(dam);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'dispel_evil'.");
 return 0;
}

/* function: dispel_monsters */
static int toluaI_spell_dispel_monsters00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int dam = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  dispel_monsters(dam);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'dispel_monsters'.");
 return 0;
}

/* function: aggravate_monsters */
static int toluaI_spell_aggravate_monsters00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int who = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  aggravate_monsters(who);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'aggravate_monsters'.");
 return 0;
}

/* function: genocide */
static int toluaI_spell_genocide00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  genocide();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'genocide'.");
 return 0;
}

/* function: mass_genocide */
static int toluaI_spell_mass_genocide00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  mass_genocide();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'mass_genocide'.");
 return 0;
}

/* function: probing */
static int toluaI_spell_probing00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  probing();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'probing'.");
 return 0;
}

/* function: destroy_area */
static int toluaI_spell_destroy_area00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,4,tolua_tag(tolua_S,"bool"),0) ||
 !tolua_isnoobj(tolua_S,5)
 )
 goto tolua_lerror;
 else
 {
  int y1 = ((int)  tolua_getnumber(tolua_S,1,0));
  int x1 = ((int)  tolua_getnumber(tolua_S,2,0));
  int r = ((int)  tolua_getnumber(tolua_S,3,0));
  bool full = ((bool)  tolua_getbool(tolua_S,4,0));
 {
  destroy_area(y1,x1,r,full);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'destroy_area'.");
 return 0;
}

/* function: earthquake */
static int toluaI_spell_earthquake00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,4)
 )
 goto tolua_lerror;
 else
 {
  int cy = ((int)  tolua_getnumber(tolua_S,1,0));
  int cx = ((int)  tolua_getnumber(tolua_S,2,0));
  int r = ((int)  tolua_getnumber(tolua_S,3,0));
 {
  earthquake(cy,cx,r);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'earthquake'.");
 return 0;
}

/* function: lite_room */
static int toluaI_spell_lite_room00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int y1 = ((int)  tolua_getnumber(tolua_S,1,0));
  int x1 = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  lite_room(y1,x1);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'lite_room'.");
 return 0;
}

/* function: unlite_room */
static int toluaI_spell_unlite_room00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int y1 = ((int)  tolua_getnumber(tolua_S,1,0));
  int x1 = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  unlite_room(y1,x1);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'unlite_room'.");
 return 0;
}

/* function: lite_area */
static int toluaI_spell_lite_area00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int dam = ((int)  tolua_getnumber(tolua_S,1,0));
  int rad = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  bool toluaI_ret = (bool)  lite_area(dam,rad);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'lite_area'.");
 return 0;
}

/* function: unlite_area */
static int toluaI_spell_unlite_area00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int dam = ((int)  tolua_getnumber(tolua_S,1,0));
  int rad = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  bool toluaI_ret = (bool)  unlite_area(dam,rad);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'unlite_area'.");
 return 0;
}

/* function: fire_ball */
static int toluaI_spell_fire_ball00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,4,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,5)
 )
 goto tolua_lerror;
 else
 {
  int typ = ((int)  tolua_getnumber(tolua_S,1,0));
  int dir = ((int)  tolua_getnumber(tolua_S,2,0));
  int dam = ((int)  tolua_getnumber(tolua_S,3,0));
  int rad = ((int)  tolua_getnumber(tolua_S,4,0));
 {
  bool toluaI_ret = (bool)  fire_ball(typ,dir,dam,rad);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'fire_ball'.");
 return 0;
}

/* function: fire_bolt */
static int toluaI_spell_fire_bolt00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,4)
 )
 goto tolua_lerror;
 else
 {
  int typ = ((int)  tolua_getnumber(tolua_S,1,0));
  int dir = ((int)  tolua_getnumber(tolua_S,2,0));
  int dam = ((int)  tolua_getnumber(tolua_S,3,0));
 {
  bool toluaI_ret = (bool)  fire_bolt(typ,dir,dam);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'fire_bolt'.");
 return 0;
}

/* function: fire_beam */
static int toluaI_spell_fire_beam00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,4)
 )
 goto tolua_lerror;
 else
 {
  int typ = ((int)  tolua_getnumber(tolua_S,1,0));
  int dir = ((int)  tolua_getnumber(tolua_S,2,0));
  int dam = ((int)  tolua_getnumber(tolua_S,3,0));
 {
  bool toluaI_ret = (bool)  fire_beam(typ,dir,dam);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'fire_beam'.");
 return 0;
}

/* function: fire_bolt_or_beam */
static int toluaI_spell_fire_bolt_or_beam00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,4,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,5)
 )
 goto tolua_lerror;
 else
 {
  int prob = ((int)  tolua_getnumber(tolua_S,1,0));
  int typ = ((int)  tolua_getnumber(tolua_S,2,0));
  int dir = ((int)  tolua_getnumber(tolua_S,3,0));
  int dam = ((int)  tolua_getnumber(tolua_S,4,0));
 {
  bool toluaI_ret = (bool)  fire_bolt_or_beam(prob,typ,dir,dam);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'fire_bolt_or_beam'.");
 return 0;
}

/* function: lite_line */
static int toluaI_spell_lite_line00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int dir = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  lite_line(dir);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'lite_line'.");
 return 0;
}

/* function: drain_life */
static int toluaI_spell_drain_life00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int dir = ((int)  tolua_getnumber(tolua_S,1,0));
  int dam = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  bool toluaI_ret = (bool)  drain_life(dir,dam);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'drain_life'.");
 return 0;
}

/* function: wall_to_mud */
static int toluaI_spell_wall_to_mud00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int dir = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  wall_to_mud(dir);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'wall_to_mud'.");
 return 0;
}

/* function: destroy_door */
static int toluaI_spell_destroy_door00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int dir = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  destroy_door(dir);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'destroy_door'.");
 return 0;
}

/* function: disarm_trap */
static int toluaI_spell_disarm_trap00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int dir = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  disarm_trap(dir);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'disarm_trap'.");
 return 0;
}

/* function: heal_monster */
static int toluaI_spell_heal_monster00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int dir = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  heal_monster(dir);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'heal_monster'.");
 return 0;
}

/* function: speed_monster */
static int toluaI_spell_speed_monster00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int dir = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  speed_monster(dir);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'speed_monster'.");
 return 0;
}

/* function: slow_monster */
static int toluaI_spell_slow_monster00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int dir = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  slow_monster(dir);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'slow_monster'.");
 return 0;
}

/* function: sleep_monster */
static int toluaI_spell_sleep_monster00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int dir = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  sleep_monster(dir);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'sleep_monster'.");
 return 0;
}

/* function: confuse_monster */
static int toluaI_spell_confuse_monster00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int dir = ((int)  tolua_getnumber(tolua_S,1,0));
  int plev = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  bool toluaI_ret = (bool)  confuse_monster(dir,plev);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'confuse_monster'.");
 return 0;
}

/* function: poly_monster */
static int toluaI_spell_poly_monster00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int dir = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  poly_monster(dir);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'poly_monster'.");
 return 0;
}

/* function: clone_monster */
static int toluaI_spell_clone_monster00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int dir = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  clone_monster(dir);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'clone_monster'.");
 return 0;
}

/* function: fear_monster */
static int toluaI_spell_fear_monster00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int dir = ((int)  tolua_getnumber(tolua_S,1,0));
  int plev = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  bool toluaI_ret = (bool)  fear_monster(dir,plev);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'fear_monster'.");
 return 0;
}

/* function: teleport_monster */
static int toluaI_spell_teleport_monster00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int dir = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  teleport_monster(dir);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'teleport_monster'.");
 return 0;
}

/* function: door_creation */
static int toluaI_spell_door_creation00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  door_creation();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'door_creation'.");
 return 0;
}

/* function: trap_creation */
static int toluaI_spell_trap_creation00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  trap_creation();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'trap_creation'.");
 return 0;
}

/* function: destroy_doors_touch */
static int toluaI_spell_destroy_doors_touch00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  destroy_doors_touch();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'destroy_doors_touch'.");
 return 0;
}

/* function: sleep_monsters_touch */
static int toluaI_spell_sleep_monsters_touch00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  sleep_monsters_touch();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'sleep_monsters_touch'.");
 return 0;
}

/* function: curse_armor */
static int toluaI_spell_curse_armor00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  curse_armor();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'curse_armor'.");
 return 0;
}

/* function: curse_weapon */
static int toluaI_spell_curse_weapon00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  curse_weapon();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'curse_weapon'.");
 return 0;
}

/* function: brand_weapon */
static int toluaI_spell_brand_weapon00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  brand_weapon();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'brand_weapon'.");
 return 0;
}

/* function: brand_bolts */
static int toluaI_spell_brand_bolts00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  brand_bolts();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'brand_bolts'.");
 return 0;
}

/* function: map_area */
static int toluaI_spell_map_area00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  map_area();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'map_area'.");
 return 0;
}

/* function: wiz_lite */
static int toluaI_spell_wiz_lite00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  wiz_lite();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'wiz_lite'.");
 return 0;
}

/* function: wiz_dark */
static int toluaI_spell_wiz_dark00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  wiz_dark();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'wiz_dark'.");
 return 0;
}

/* Open function */
int tolua_spell_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 toluaI_reg_types(tolua_S);
 tolua_constant(tolua_S,NULL,"SUMMON_ANIMAL",SUMMON_ANIMAL);
 tolua_constant(tolua_S,NULL,"SUMMON_SPIDER",SUMMON_SPIDER);
 tolua_constant(tolua_S,NULL,"SUMMON_HOUND",SUMMON_HOUND);
 tolua_constant(tolua_S,NULL,"SUMMON_HYDRA",SUMMON_HYDRA);
 tolua_constant(tolua_S,NULL,"SUMMON_ANGEL",SUMMON_ANGEL);
 tolua_constant(tolua_S,NULL,"SUMMON_DEMON",SUMMON_DEMON);
 tolua_constant(tolua_S,NULL,"SUMMON_UNDEAD",SUMMON_UNDEAD);
 tolua_constant(tolua_S,NULL,"SUMMON_DRAGON",SUMMON_DRAGON);
 tolua_constant(tolua_S,NULL,"SUMMON_HI_DEMON",SUMMON_HI_DEMON);
 tolua_constant(tolua_S,NULL,"SUMMON_HI_UNDEAD",SUMMON_HI_UNDEAD);
 tolua_constant(tolua_S,NULL,"SUMMON_HI_DRAGON",SUMMON_HI_DRAGON);
 tolua_constant(tolua_S,NULL,"SUMMON_WRAITH",SUMMON_WRAITH);
 tolua_constant(tolua_S,NULL,"SUMMON_UNIQUE",SUMMON_UNIQUE);
 tolua_constant(tolua_S,NULL,"SUMMON_KIN",SUMMON_KIN);
 tolua_constant(tolua_S,NULL,"GF_XXX1",GF_XXX1);
 tolua_constant(tolua_S,NULL,"GF_ARROW",GF_ARROW);
 tolua_constant(tolua_S,NULL,"GF_MISSILE",GF_MISSILE);
 tolua_constant(tolua_S,NULL,"GF_MANA",GF_MANA);
 tolua_constant(tolua_S,NULL,"GF_HOLY_ORB",GF_HOLY_ORB);
 tolua_constant(tolua_S,NULL,"GF_LITE_WEAK",GF_LITE_WEAK);
 tolua_constant(tolua_S,NULL,"GF_DARK_WEAK",GF_DARK_WEAK);
 tolua_constant(tolua_S,NULL,"GF_WATER",GF_WATER);
 tolua_constant(tolua_S,NULL,"GF_PLASMA",GF_PLASMA);
 tolua_constant(tolua_S,NULL,"GF_METEOR",GF_METEOR);
 tolua_constant(tolua_S,NULL,"GF_ICE",GF_ICE);
 tolua_constant(tolua_S,NULL,"GF_GRAVITY",GF_GRAVITY);
 tolua_constant(tolua_S,NULL,"GF_INERTIA",GF_INERTIA);
 tolua_constant(tolua_S,NULL,"GF_FORCE",GF_FORCE);
 tolua_constant(tolua_S,NULL,"GF_TIME",GF_TIME);
 tolua_constant(tolua_S,NULL,"GF_ACID",GF_ACID);
 tolua_constant(tolua_S,NULL,"GF_ELEC",GF_ELEC);
 tolua_constant(tolua_S,NULL,"GF_FIRE",GF_FIRE);
 tolua_constant(tolua_S,NULL,"GF_COLD",GF_COLD);
 tolua_constant(tolua_S,NULL,"GF_POIS",GF_POIS);
 tolua_constant(tolua_S,NULL,"GF_XXX2",GF_XXX2);
 tolua_constant(tolua_S,NULL,"GF_LITE",GF_LITE);
 tolua_constant(tolua_S,NULL,"GF_DARK",GF_DARK);
 tolua_constant(tolua_S,NULL,"GF_XXX3",GF_XXX3);
 tolua_constant(tolua_S,NULL,"GF_CONFUSION",GF_CONFUSION);
 tolua_constant(tolua_S,NULL,"GF_SOUND",GF_SOUND);
 tolua_constant(tolua_S,NULL,"GF_SHARD",GF_SHARD);
 tolua_constant(tolua_S,NULL,"GF_NEXUS",GF_NEXUS);
 tolua_constant(tolua_S,NULL,"GF_NETHER",GF_NETHER);
 tolua_constant(tolua_S,NULL,"GF_CHAOS",GF_CHAOS);
 tolua_constant(tolua_S,NULL,"GF_DISENCHANT",GF_DISENCHANT);
 tolua_constant(tolua_S,NULL,"GF_XXX4",GF_XXX4);
 tolua_constant(tolua_S,NULL,"GF_KILL_WALL",GF_KILL_WALL);
 tolua_constant(tolua_S,NULL,"GF_KILL_DOOR",GF_KILL_DOOR);
 tolua_constant(tolua_S,NULL,"GF_KILL_TRAP",GF_KILL_TRAP);
 tolua_constant(tolua_S,NULL,"GF_MAKE_WALL",GF_MAKE_WALL);
 tolua_constant(tolua_S,NULL,"GF_MAKE_DOOR",GF_MAKE_DOOR);
 tolua_constant(tolua_S,NULL,"GF_MAKE_TRAP",GF_MAKE_TRAP);
 tolua_constant(tolua_S,NULL,"GF_XXX5",GF_XXX5);
 tolua_constant(tolua_S,NULL,"GF_XXX6",GF_XXX6);
 tolua_constant(tolua_S,NULL,"GF_AWAY_UNDEAD",GF_AWAY_UNDEAD);
 tolua_constant(tolua_S,NULL,"GF_AWAY_EVIL",GF_AWAY_EVIL);
 tolua_constant(tolua_S,NULL,"GF_AWAY_ALL",GF_AWAY_ALL);
 tolua_constant(tolua_S,NULL,"GF_TURN_UNDEAD",GF_TURN_UNDEAD);
 tolua_constant(tolua_S,NULL,"GF_TURN_EVIL",GF_TURN_EVIL);
 tolua_constant(tolua_S,NULL,"GF_TURN_ALL",GF_TURN_ALL);
 tolua_constant(tolua_S,NULL,"GF_DISP_UNDEAD",GF_DISP_UNDEAD);
 tolua_constant(tolua_S,NULL,"GF_DISP_EVIL",GF_DISP_EVIL);
 tolua_constant(tolua_S,NULL,"GF_DISP_ALL",GF_DISP_ALL);
 tolua_constant(tolua_S,NULL,"GF_XXX7",GF_XXX7);
 tolua_constant(tolua_S,NULL,"GF_OLD_CLONE",GF_OLD_CLONE);
 tolua_constant(tolua_S,NULL,"GF_OLD_POLY",GF_OLD_POLY);
 tolua_constant(tolua_S,NULL,"GF_OLD_HEAL",GF_OLD_HEAL);
 tolua_constant(tolua_S,NULL,"GF_OLD_SPEED",GF_OLD_SPEED);
 tolua_constant(tolua_S,NULL,"GF_OLD_SLOW",GF_OLD_SLOW);
 tolua_constant(tolua_S,NULL,"GF_OLD_CONF",GF_OLD_CONF);
 tolua_constant(tolua_S,NULL,"GF_OLD_SLEEP",GF_OLD_SLEEP);
 tolua_constant(tolua_S,NULL,"GF_OLD_DRAIN",GF_OLD_DRAIN);
 tolua_constant(tolua_S,NULL,"GF_XXX8",GF_XXX8);
 tolua_function(tolua_S,NULL,"poly_r_idx",toluaI_spell_poly_r_idx00);
 tolua_function(tolua_S,NULL,"teleport_away",toluaI_spell_teleport_away00);
 tolua_function(tolua_S,NULL,"teleport_player",toluaI_spell_teleport_player00);
 tolua_function(tolua_S,NULL,"teleport_player_to",toluaI_spell_teleport_player_to00);
 tolua_function(tolua_S,NULL,"teleport_player_level",toluaI_spell_teleport_player_level00);
 tolua_function(tolua_S,NULL,"take_hit",toluaI_spell_take_hit00);
 tolua_function(tolua_S,NULL,"acid_dam",toluaI_spell_acid_dam00);
 tolua_function(tolua_S,NULL,"elec_dam",toluaI_spell_elec_dam00);
 tolua_function(tolua_S,NULL,"fire_dam",toluaI_spell_fire_dam00);
 tolua_function(tolua_S,NULL,"cold_dam",toluaI_spell_cold_dam00);
 tolua_function(tolua_S,NULL,"inc_stat",toluaI_spell_inc_stat00);
 tolua_function(tolua_S,NULL,"dec_stat",toluaI_spell_dec_stat00);
 tolua_function(tolua_S,NULL,"res_stat",toluaI_spell_res_stat00);
 tolua_function(tolua_S,NULL,"apply_disenchant",toluaI_spell_apply_disenchant00);
 tolua_function(tolua_S,NULL,"project",toluaI_spell_project00);
 tolua_function(tolua_S,NULL,"warding_glyph",toluaI_spell_warding_glyph00);
 tolua_function(tolua_S,NULL,"identify_pack",toluaI_spell_identify_pack00);
 tolua_function(tolua_S,NULL,"remove_curse",toluaI_spell_remove_curse00);
 tolua_function(tolua_S,NULL,"remove_all_curse",toluaI_spell_remove_all_curse00);
 tolua_function(tolua_S,NULL,"self_knowledge",toluaI_spell_self_knowledge00);
 tolua_function(tolua_S,NULL,"lose_all_info",toluaI_spell_lose_all_info00);
 tolua_function(tolua_S,NULL,"set_recall",toluaI_spell_set_recall00);
 tolua_function(tolua_S,NULL,"detect_traps",toluaI_spell_detect_traps00);
 tolua_function(tolua_S,NULL,"detect_doors",toluaI_spell_detect_doors00);
 tolua_function(tolua_S,NULL,"detect_stairs",toluaI_spell_detect_stairs00);
 tolua_function(tolua_S,NULL,"detect_treasure",toluaI_spell_detect_treasure00);
 tolua_function(tolua_S,NULL,"detect_objects_gold",toluaI_spell_detect_objects_gold00);
 tolua_function(tolua_S,NULL,"detect_objects_normal",toluaI_spell_detect_objects_normal00);
 tolua_function(tolua_S,NULL,"detect_objects_magic",toluaI_spell_detect_objects_magic00);
 tolua_function(tolua_S,NULL,"detect_monsters_normal",toluaI_spell_detect_monsters_normal00);
 tolua_function(tolua_S,NULL,"detect_monsters_invis",toluaI_spell_detect_monsters_invis00);
 tolua_function(tolua_S,NULL,"detect_monsters_evil",toluaI_spell_detect_monsters_evil00);
 tolua_function(tolua_S,NULL,"detect_all",toluaI_spell_detect_all00);
 tolua_function(tolua_S,NULL,"stair_creation",toluaI_spell_stair_creation00);
 tolua_function(tolua_S,NULL,"enchant",toluaI_spell_enchant00);
 tolua_function(tolua_S,NULL,"enchant_spell",toluaI_spell_enchant_spell00);
 tolua_function(tolua_S,NULL,"ident_spell",toluaI_spell_ident_spell00);
 tolua_function(tolua_S,NULL,"identify_fully",toluaI_spell_identify_fully00);
 tolua_function(tolua_S,NULL,"recharge",toluaI_spell_recharge00);
 tolua_function(tolua_S,NULL,"speed_monsters",toluaI_spell_speed_monsters00);
 tolua_function(tolua_S,NULL,"slow_monsters",toluaI_spell_slow_monsters00);
 tolua_function(tolua_S,NULL,"sleep_monsters",toluaI_spell_sleep_monsters00);
 tolua_function(tolua_S,NULL,"banish_evil",toluaI_spell_banish_evil00);
 tolua_function(tolua_S,NULL,"turn_undead",toluaI_spell_turn_undead00);
 tolua_function(tolua_S,NULL,"dispel_undead",toluaI_spell_dispel_undead00);
 tolua_function(tolua_S,NULL,"dispel_evil",toluaI_spell_dispel_evil00);
 tolua_function(tolua_S,NULL,"dispel_monsters",toluaI_spell_dispel_monsters00);
 tolua_function(tolua_S,NULL,"aggravate_monsters",toluaI_spell_aggravate_monsters00);
 tolua_function(tolua_S,NULL,"genocide",toluaI_spell_genocide00);
 tolua_function(tolua_S,NULL,"mass_genocide",toluaI_spell_mass_genocide00);
 tolua_function(tolua_S,NULL,"probing",toluaI_spell_probing00);
 tolua_function(tolua_S,NULL,"destroy_area",toluaI_spell_destroy_area00);
 tolua_function(tolua_S,NULL,"earthquake",toluaI_spell_earthquake00);
 tolua_function(tolua_S,NULL,"lite_room",toluaI_spell_lite_room00);
 tolua_function(tolua_S,NULL,"unlite_room",toluaI_spell_unlite_room00);
 tolua_function(tolua_S,NULL,"lite_area",toluaI_spell_lite_area00);
 tolua_function(tolua_S,NULL,"unlite_area",toluaI_spell_unlite_area00);
 tolua_function(tolua_S,NULL,"fire_ball",toluaI_spell_fire_ball00);
 tolua_function(tolua_S,NULL,"fire_bolt",toluaI_spell_fire_bolt00);
 tolua_function(tolua_S,NULL,"fire_beam",toluaI_spell_fire_beam00);
 tolua_function(tolua_S,NULL,"fire_bolt_or_beam",toluaI_spell_fire_bolt_or_beam00);
 tolua_function(tolua_S,NULL,"lite_line",toluaI_spell_lite_line00);
 tolua_function(tolua_S,NULL,"drain_life",toluaI_spell_drain_life00);
 tolua_function(tolua_S,NULL,"wall_to_mud",toluaI_spell_wall_to_mud00);
 tolua_function(tolua_S,NULL,"destroy_door",toluaI_spell_destroy_door00);
 tolua_function(tolua_S,NULL,"disarm_trap",toluaI_spell_disarm_trap00);
 tolua_function(tolua_S,NULL,"heal_monster",toluaI_spell_heal_monster00);
 tolua_function(tolua_S,NULL,"speed_monster",toluaI_spell_speed_monster00);
 tolua_function(tolua_S,NULL,"slow_monster",toluaI_spell_slow_monster00);
 tolua_function(tolua_S,NULL,"sleep_monster",toluaI_spell_sleep_monster00);
 tolua_function(tolua_S,NULL,"confuse_monster",toluaI_spell_confuse_monster00);
 tolua_function(tolua_S,NULL,"poly_monster",toluaI_spell_poly_monster00);
 tolua_function(tolua_S,NULL,"clone_monster",toluaI_spell_clone_monster00);
 tolua_function(tolua_S,NULL,"fear_monster",toluaI_spell_fear_monster00);
 tolua_function(tolua_S,NULL,"teleport_monster",toluaI_spell_teleport_monster00);
 tolua_function(tolua_S,NULL,"door_creation",toluaI_spell_door_creation00);
 tolua_function(tolua_S,NULL,"trap_creation",toluaI_spell_trap_creation00);
 tolua_function(tolua_S,NULL,"destroy_doors_touch",toluaI_spell_destroy_doors_touch00);
 tolua_function(tolua_S,NULL,"sleep_monsters_touch",toluaI_spell_sleep_monsters_touch00);
 tolua_function(tolua_S,NULL,"curse_armor",toluaI_spell_curse_armor00);
 tolua_function(tolua_S,NULL,"curse_weapon",toluaI_spell_curse_weapon00);
 tolua_function(tolua_S,NULL,"brand_weapon",toluaI_spell_brand_weapon00);
 tolua_function(tolua_S,NULL,"brand_bolts",toluaI_spell_brand_bolts00);
 tolua_function(tolua_S,NULL,"map_area",toluaI_spell_map_area00);
 tolua_function(tolua_S,NULL,"wiz_lite",toluaI_spell_wiz_lite00);
 tolua_function(tolua_S,NULL,"wiz_dark",toluaI_spell_wiz_dark00);
 return 1;
}
/* Close function */
void tolua_spell_close (lua_State* tolua_S)
{
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SUMMON_ANIMAL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SUMMON_SPIDER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SUMMON_HOUND");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SUMMON_HYDRA");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SUMMON_ANGEL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SUMMON_DEMON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SUMMON_UNDEAD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SUMMON_DRAGON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SUMMON_HI_DEMON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SUMMON_HI_UNDEAD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SUMMON_HI_DRAGON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SUMMON_WRAITH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SUMMON_UNIQUE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SUMMON_KIN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_XXX1");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_ARROW");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_MISSILE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_MANA");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_HOLY_ORB");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_LITE_WEAK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_DARK_WEAK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_WATER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_PLASMA");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_METEOR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_ICE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_GRAVITY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_INERTIA");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_FORCE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_TIME");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_ACID");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_ELEC");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_FIRE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_COLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_POIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_XXX2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_LITE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_DARK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_XXX3");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_CONFUSION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_SOUND");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_SHARD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_NEXUS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_NETHER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_CHAOS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_DISENCHANT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_XXX4");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_KILL_WALL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_KILL_DOOR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_KILL_TRAP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_MAKE_WALL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_MAKE_DOOR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_MAKE_TRAP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_XXX5");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_XXX6");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_AWAY_UNDEAD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_AWAY_EVIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_AWAY_ALL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_TURN_UNDEAD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_TURN_EVIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_TURN_ALL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_DISP_UNDEAD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_DISP_EVIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_DISP_ALL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_XXX7");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_OLD_CLONE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_OLD_POLY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_OLD_HEAL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_OLD_SPEED");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_OLD_SLOW");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_OLD_CONF");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_OLD_SLEEP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_OLD_DRAIN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"GF_XXX8");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"poly_r_idx");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"teleport_away");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"teleport_player");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"teleport_player_to");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"teleport_player_level");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"take_hit");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"acid_dam");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"elec_dam");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"fire_dam");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"cold_dam");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"inc_stat");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"dec_stat");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"res_stat");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"apply_disenchant");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"project");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"warding_glyph");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"identify_pack");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"remove_curse");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"remove_all_curse");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"self_knowledge");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"lose_all_info");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_recall");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"detect_traps");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"detect_doors");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"detect_stairs");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"detect_treasure");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"detect_objects_gold");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"detect_objects_normal");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"detect_objects_magic");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"detect_monsters_normal");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"detect_monsters_invis");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"detect_monsters_evil");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"detect_all");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"stair_creation");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"enchant");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"enchant_spell");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ident_spell");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"identify_fully");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"recharge");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"speed_monsters");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"slow_monsters");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"sleep_monsters");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"banish_evil");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"turn_undead");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"dispel_undead");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"dispel_evil");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"dispel_monsters");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"aggravate_monsters");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"genocide");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"mass_genocide");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"probing");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"destroy_area");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"earthquake");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"lite_room");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"unlite_room");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"lite_area");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"unlite_area");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"fire_ball");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"fire_bolt");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"fire_beam");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"fire_bolt_or_beam");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"lite_line");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"drain_life");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"wall_to_mud");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"destroy_door");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"disarm_trap");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"heal_monster");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"speed_monster");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"slow_monster");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"sleep_monster");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"confuse_monster");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"poly_monster");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"clone_monster");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"fear_monster");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"teleport_monster");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"door_creation");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"trap_creation");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"destroy_doors_touch");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"sleep_monsters_touch");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"curse_armor");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"curse_weapon");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"brand_weapon");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"brand_bolts");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"map_area");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"wiz_lite");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"wiz_dark");
}
