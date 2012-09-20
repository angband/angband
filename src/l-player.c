/*
** Lua binding: player
** Generated automatically by tolua 4.0a - angband on Sun Nov 18 18:51:05 2001.
*/

#include "lua/tolua.h"

/* Exported function */
int tolua_player_open (lua_State* tolua_S);
void tolua_player_close (lua_State* tolua_S);

#include "angband.h"

/* function to register type */
static void toluaI_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"player_magic");
 tolua_usertype(tolua_S,"player_class");
 tolua_usertype(tolua_S,"start_item");
 tolua_usertype(tolua_S,"player_type");
 tolua_usertype(tolua_S,"player_sex");
 tolua_usertype(tolua_S,"player_race");
 tolua_usertype(tolua_S,"player_other");
 tolua_usertype(tolua_S,"hist_type");
}

/* error messages */
#define TOLUA_ERR_SELF tolua_error(tolua_S,"invalid 'self'")
#define TOLUA_ERR_ASSIGN tolua_error(tolua_S,"#vinvalid type in variable assignment.")

/* get function: py of class  player_type */
static int toluaI_get_player_player_type_py(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->py);
 return 1;
}

/* set function: py of class  player_type */
static int toluaI_set_player_player_type_py(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->py = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: px of class  player_type */
static int toluaI_get_player_player_type_px(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->px);
 return 1;
}

/* set function: px of class  player_type */
static int toluaI_set_player_player_type_px(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->px = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: psex of class  player_type */
static int toluaI_get_player_player_type_psex(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->psex);
 return 1;
}

/* set function: psex of class  player_type */
static int toluaI_set_player_player_type_psex(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->psex = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: prace of class  player_type */
static int toluaI_get_player_player_type_prace(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->prace);
 return 1;
}

/* set function: prace of class  player_type */
static int toluaI_set_player_player_type_prace(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->prace = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: pclass of class  player_type */
static int toluaI_get_player_player_type_pclass(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->pclass);
 return 1;
}

/* set function: pclass of class  player_type */
static int toluaI_set_player_player_type_pclass(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->pclass = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: hitdie of class  player_type */
static int toluaI_get_player_player_type_hitdie(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->hitdie);
 return 1;
}

/* set function: hitdie of class  player_type */
static int toluaI_set_player_player_type_hitdie(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->hitdie = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: expfact of class  player_type */
static int toluaI_get_player_player_type_expfact(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->expfact);
 return 1;
}

/* set function: expfact of class  player_type */
static int toluaI_set_player_player_type_expfact(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->expfact = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: age of class  player_type */
static int toluaI_get_player_player_type_age(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->age);
 return 1;
}

/* set function: age of class  player_type */
static int toluaI_set_player_player_type_age(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->age = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: ht of class  player_type */
static int toluaI_get_player_player_type_ht(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->ht);
 return 1;
}

/* set function: ht of class  player_type */
static int toluaI_set_player_player_type_ht(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->ht = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: wt of class  player_type */
static int toluaI_get_player_player_type_wt(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->wt);
 return 1;
}

/* set function: wt of class  player_type */
static int toluaI_set_player_player_type_wt(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->wt = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: sc of class  player_type */
static int toluaI_get_player_player_type_sc(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->sc);
 return 1;
}

/* set function: sc of class  player_type */
static int toluaI_set_player_player_type_sc(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->sc = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: au of class  player_type */
static int toluaI_get_player_player_type_au(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->au);
 return 1;
}

/* set function: au of class  player_type */
static int toluaI_set_player_player_type_au(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->au = ((s32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: max_depth of class  player_type */
static int toluaI_get_player_player_type_max_depth(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->max_depth);
 return 1;
}

/* set function: max_depth of class  player_type */
static int toluaI_set_player_player_type_max_depth(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->max_depth = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: depth of class  player_type */
static int toluaI_get_player_player_type_depth(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->depth);
 return 1;
}

/* set function: depth of class  player_type */
static int toluaI_set_player_player_type_depth(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->depth = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: max_lev of class  player_type */
static int toluaI_get_player_player_type_max_lev(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->max_lev);
 return 1;
}

/* set function: max_lev of class  player_type */
static int toluaI_set_player_player_type_max_lev(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->max_lev = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: lev of class  player_type */
static int toluaI_get_player_player_type_lev(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->lev);
 return 1;
}

/* set function: lev of class  player_type */
static int toluaI_set_player_player_type_lev(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->lev = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: max_exp of class  player_type */
static int toluaI_get_player_player_type_max_exp(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->max_exp);
 return 1;
}

/* set function: max_exp of class  player_type */
static int toluaI_set_player_player_type_max_exp(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->max_exp = ((s32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: exp of class  player_type */
static int toluaI_get_player_player_type_exp(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->exp);
 return 1;
}

/* set function: exp of class  player_type */
static int toluaI_set_player_player_type_exp(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->exp = ((s32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: exp_frac of class  player_type */
static int toluaI_get_player_player_type_exp_frac(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->exp_frac);
 return 1;
}

/* set function: exp_frac of class  player_type */
static int toluaI_set_player_player_type_exp_frac(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->exp_frac = ((u16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: mhp of class  player_type */
static int toluaI_get_player_player_type_mhp(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->mhp);
 return 1;
}

/* set function: mhp of class  player_type */
static int toluaI_set_player_player_type_mhp(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->mhp = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: chp of class  player_type */
static int toluaI_get_player_player_type_chp(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->chp);
 return 1;
}

/* set function: chp of class  player_type */
static int toluaI_set_player_player_type_chp(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->chp = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: chp_frac of class  player_type */
static int toluaI_get_player_player_type_chp_frac(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->chp_frac);
 return 1;
}

/* set function: chp_frac of class  player_type */
static int toluaI_set_player_player_type_chp_frac(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->chp_frac = ((u16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: msp of class  player_type */
static int toluaI_get_player_player_type_msp(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->msp);
 return 1;
}

/* set function: msp of class  player_type */
static int toluaI_set_player_player_type_msp(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->msp = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: csp of class  player_type */
static int toluaI_get_player_player_type_csp(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->csp);
 return 1;
}

/* set function: csp of class  player_type */
static int toluaI_set_player_player_type_csp(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->csp = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: csp_frac of class  player_type */
static int toluaI_get_player_player_type_csp_frac(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->csp_frac);
 return 1;
}

/* set function: csp_frac of class  player_type */
static int toluaI_set_player_player_type_csp_frac(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->csp_frac = ((u16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: stat_max of class  player_type */
static int toluaI_get_player_player_type_stat_max(lua_State* tolua_S)
{
 int toluaI_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushnumber(tolua_S,(long)self->stat_max[toluaI_index]);
 return 1;
}

/* set function: stat_max of class  player_type */
static int toluaI_set_player_player_type_stat_max(lua_State* tolua_S)
{
 int toluaI_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.");
  self->stat_max[toluaI_index] = ((s16b)  tolua_getnumber(tolua_S,3,0));
 return 0;
}

/* get function: stat_cur of class  player_type */
static int toluaI_get_player_player_type_stat_cur(lua_State* tolua_S)
{
 int toluaI_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushnumber(tolua_S,(long)self->stat_cur[toluaI_index]);
 return 1;
}

/* set function: stat_cur of class  player_type */
static int toluaI_set_player_player_type_stat_cur(lua_State* tolua_S)
{
 int toluaI_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.");
  self->stat_cur[toluaI_index] = ((s16b)  tolua_getnumber(tolua_S,3,0));
 return 0;
}

/* get function: fast of class  player_type */
static int toluaI_get_player_player_type_fast(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->fast);
 return 1;
}

/* set function: fast of class  player_type */
static int toluaI_set_player_player_type_fast(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->fast = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: slow of class  player_type */
static int toluaI_get_player_player_type_slow(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->slow);
 return 1;
}

/* set function: slow of class  player_type */
static int toluaI_set_player_player_type_slow(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->slow = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: blind of class  player_type */
static int toluaI_get_player_player_type_blind(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->blind);
 return 1;
}

/* set function: blind of class  player_type */
static int toluaI_set_player_player_type_blind(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->blind = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: paralyzed of class  player_type */
static int toluaI_get_player_player_type_paralyzed(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->paralyzed);
 return 1;
}

/* set function: paralyzed of class  player_type */
static int toluaI_set_player_player_type_paralyzed(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->paralyzed = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: confused of class  player_type */
static int toluaI_get_player_player_type_confused(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->confused);
 return 1;
}

/* set function: confused of class  player_type */
static int toluaI_set_player_player_type_confused(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->confused = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: afraid of class  player_type */
static int toluaI_get_player_player_type_afraid(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->afraid);
 return 1;
}

/* set function: afraid of class  player_type */
static int toluaI_set_player_player_type_afraid(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->afraid = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: image of class  player_type */
static int toluaI_get_player_player_type_image(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->image);
 return 1;
}

/* set function: image of class  player_type */
static int toluaI_set_player_player_type_image(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->image = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: poisoned of class  player_type */
static int toluaI_get_player_player_type_poisoned(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->poisoned);
 return 1;
}

/* set function: poisoned of class  player_type */
static int toluaI_set_player_player_type_poisoned(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->poisoned = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: cut of class  player_type */
static int toluaI_get_player_player_type_cut(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->cut);
 return 1;
}

/* set function: cut of class  player_type */
static int toluaI_set_player_player_type_cut(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->cut = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: stun of class  player_type */
static int toluaI_get_player_player_type_stun(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->stun);
 return 1;
}

/* set function: stun of class  player_type */
static int toluaI_set_player_player_type_stun(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->stun = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: protevil of class  player_type */
static int toluaI_get_player_player_type_protevil(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->protevil);
 return 1;
}

/* set function: protevil of class  player_type */
static int toluaI_set_player_player_type_protevil(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->protevil = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: invuln of class  player_type */
static int toluaI_get_player_player_type_invuln(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->invuln);
 return 1;
}

/* set function: invuln of class  player_type */
static int toluaI_set_player_player_type_invuln(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->invuln = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: hero of class  player_type */
static int toluaI_get_player_player_type_hero(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->hero);
 return 1;
}

/* set function: hero of class  player_type */
static int toluaI_set_player_player_type_hero(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->hero = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: shero of class  player_type */
static int toluaI_get_player_player_type_shero(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->shero);
 return 1;
}

/* set function: shero of class  player_type */
static int toluaI_set_player_player_type_shero(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->shero = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: shield of class  player_type */
static int toluaI_get_player_player_type_shield(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->shield);
 return 1;
}

/* set function: shield of class  player_type */
static int toluaI_set_player_player_type_shield(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->shield = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: blessed of class  player_type */
static int toluaI_get_player_player_type_blessed(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->blessed);
 return 1;
}

/* set function: blessed of class  player_type */
static int toluaI_set_player_player_type_blessed(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->blessed = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: tim_invis of class  player_type */
static int toluaI_get_player_player_type_tim_invis(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->tim_invis);
 return 1;
}

/* set function: tim_invis of class  player_type */
static int toluaI_set_player_player_type_tim_invis(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->tim_invis = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: tim_infra of class  player_type */
static int toluaI_get_player_player_type_tim_infra(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->tim_infra);
 return 1;
}

/* set function: tim_infra of class  player_type */
static int toluaI_set_player_player_type_tim_infra(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->tim_infra = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: oppose_acid of class  player_type */
static int toluaI_get_player_player_type_oppose_acid(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->oppose_acid);
 return 1;
}

/* set function: oppose_acid of class  player_type */
static int toluaI_set_player_player_type_oppose_acid(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->oppose_acid = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: oppose_elec of class  player_type */
static int toluaI_get_player_player_type_oppose_elec(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->oppose_elec);
 return 1;
}

/* set function: oppose_elec of class  player_type */
static int toluaI_set_player_player_type_oppose_elec(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->oppose_elec = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: oppose_fire of class  player_type */
static int toluaI_get_player_player_type_oppose_fire(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->oppose_fire);
 return 1;
}

/* set function: oppose_fire of class  player_type */
static int toluaI_set_player_player_type_oppose_fire(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->oppose_fire = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: oppose_cold of class  player_type */
static int toluaI_get_player_player_type_oppose_cold(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->oppose_cold);
 return 1;
}

/* set function: oppose_cold of class  player_type */
static int toluaI_set_player_player_type_oppose_cold(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->oppose_cold = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: oppose_pois of class  player_type */
static int toluaI_get_player_player_type_oppose_pois(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->oppose_pois);
 return 1;
}

/* set function: oppose_pois of class  player_type */
static int toluaI_set_player_player_type_oppose_pois(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->oppose_pois = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: word_recall of class  player_type */
static int toluaI_get_player_player_type_word_recall(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->word_recall);
 return 1;
}

/* set function: word_recall of class  player_type */
static int toluaI_set_player_player_type_word_recall(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->word_recall = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: energy of class  player_type */
static int toluaI_get_player_player_type_energy(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->energy);
 return 1;
}

/* set function: energy of class  player_type */
static int toluaI_set_player_player_type_energy(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->energy = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: food of class  player_type */
static int toluaI_get_player_player_type_food(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->food);
 return 1;
}

/* set function: food of class  player_type */
static int toluaI_set_player_player_type_food(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->food = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: confusing of class  player_type */
static int toluaI_get_player_player_type_confusing(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->confusing);
 return 1;
}

/* set function: confusing of class  player_type */
static int toluaI_set_player_player_type_confusing(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->confusing = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: searching of class  player_type */
static int toluaI_get_player_player_type_searching(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->searching);
 return 1;
}

/* set function: searching of class  player_type */
static int toluaI_set_player_player_type_searching(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->searching = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: spell_learned1 of class  player_type */
static int toluaI_get_player_player_type_spell_learned1(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->spell_learned1);
 return 1;
}

/* set function: spell_learned1 of class  player_type */
static int toluaI_set_player_player_type_spell_learned1(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->spell_learned1 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: spell_learned2 of class  player_type */
static int toluaI_get_player_player_type_spell_learned2(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->spell_learned2);
 return 1;
}

/* set function: spell_learned2 of class  player_type */
static int toluaI_set_player_player_type_spell_learned2(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->spell_learned2 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: spell_worked1 of class  player_type */
static int toluaI_get_player_player_type_spell_worked1(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->spell_worked1);
 return 1;
}

/* set function: spell_worked1 of class  player_type */
static int toluaI_set_player_player_type_spell_worked1(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->spell_worked1 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: spell_worked2 of class  player_type */
static int toluaI_get_player_player_type_spell_worked2(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->spell_worked2);
 return 1;
}

/* set function: spell_worked2 of class  player_type */
static int toluaI_set_player_player_type_spell_worked2(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->spell_worked2 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: spell_forgotten1 of class  player_type */
static int toluaI_get_player_player_type_spell_forgotten1(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->spell_forgotten1);
 return 1;
}

/* set function: spell_forgotten1 of class  player_type */
static int toluaI_set_player_player_type_spell_forgotten1(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->spell_forgotten1 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: spell_forgotten2 of class  player_type */
static int toluaI_get_player_player_type_spell_forgotten2(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->spell_forgotten2);
 return 1;
}

/* set function: spell_forgotten2 of class  player_type */
static int toluaI_set_player_player_type_spell_forgotten2(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->spell_forgotten2 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: spell_order of class  player_type */
static int toluaI_get_player_player_type_spell_order(lua_State* tolua_S)
{
 int toluaI_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=PY_MAX_SPELLS)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushnumber(tolua_S,(long)self->spell_order[toluaI_index]);
 return 1;
}

/* set function: spell_order of class  player_type */
static int toluaI_set_player_player_type_spell_order(lua_State* tolua_S)
{
 int toluaI_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=PY_MAX_SPELLS)
 tolua_error(tolua_S,"array indexing out of range.");
  self->spell_order[toluaI_index] = ((byte)  tolua_getnumber(tolua_S,3,0));
 return 0;
}

/* get function: player_hp of class  player_type */
static int toluaI_get_player_player_type_player_hp(lua_State* tolua_S)
{
 int toluaI_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=PY_MAX_LEVEL)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushnumber(tolua_S,(long)self->player_hp[toluaI_index]);
 return 1;
}

/* set function: player_hp of class  player_type */
static int toluaI_set_player_player_type_player_hp(lua_State* tolua_S)
{
 int toluaI_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=PY_MAX_LEVEL)
 tolua_error(tolua_S,"array indexing out of range.");
  self->player_hp[toluaI_index] = ((s16b)  tolua_getnumber(tolua_S,3,0));
 return 0;
}

/* get function: died_from of class  player_type */
static int toluaI_get_player_player_type_died_from(lua_State* tolua_S)
{
 int toluaI_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=80)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushnumber(tolua_S,(long)self->died_from[toluaI_index]);
 return 1;
}

/* set function: died_from of class  player_type */
static int toluaI_set_player_player_type_died_from(lua_State* tolua_S)
{
 int toluaI_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=80)
 tolua_error(tolua_S,"array indexing out of range.");
  self->died_from[toluaI_index] = ((char)  tolua_getnumber(tolua_S,3,0));
 return 0;
}

/* get function: total_winner of class  player_type */
static int toluaI_get_player_player_type_total_winner(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->total_winner);
 return 1;
}

/* set function: total_winner of class  player_type */
static int toluaI_set_player_player_type_total_winner(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->total_winner = ((u16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: panic_save of class  player_type */
static int toluaI_get_player_player_type_panic_save(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->panic_save);
 return 1;
}

/* set function: panic_save of class  player_type */
static int toluaI_set_player_player_type_panic_save(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->panic_save = ((u16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: noscore of class  player_type */
static int toluaI_get_player_player_type_noscore(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->noscore);
 return 1;
}

/* set function: noscore of class  player_type */
static int toluaI_set_player_player_type_noscore(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->noscore = ((u16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: is_dead of class  player_type */
static int toluaI_get_player_player_type_is_dead(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->is_dead);
 return 1;
}

/* set function: is_dead of class  player_type */
static int toluaI_set_player_player_type_is_dead(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->is_dead = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: wizard of class  player_type */
static int toluaI_get_player_player_type_wizard(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->wizard);
 return 1;
}

/* set function: wizard of class  player_type */
static int toluaI_set_player_player_type_wizard(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->wizard = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: playing of class  player_type */
static int toluaI_get_player_player_type_playing(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->playing);
 return 1;
}

/* set function: playing of class  player_type */
static int toluaI_set_player_player_type_playing(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->playing = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: leaving of class  player_type */
static int toluaI_get_player_player_type_leaving(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->leaving);
 return 1;
}

/* set function: leaving of class  player_type */
static int toluaI_set_player_player_type_leaving(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->leaving = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: create_up_stair of class  player_type */
static int toluaI_get_player_player_type_create_up_stair(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->create_up_stair);
 return 1;
}

/* set function: create_up_stair of class  player_type */
static int toluaI_set_player_player_type_create_up_stair(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->create_up_stair = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: create_down_stair of class  player_type */
static int toluaI_get_player_player_type_create_down_stair(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->create_down_stair);
 return 1;
}

/* set function: create_down_stair of class  player_type */
static int toluaI_set_player_player_type_create_down_stair(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->create_down_stair = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: wy of class  player_type */
static int toluaI_get_player_player_type_wy(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->wy);
 return 1;
}

/* set function: wy of class  player_type */
static int toluaI_set_player_player_type_wy(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->wy = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: wx of class  player_type */
static int toluaI_get_player_player_type_wx(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->wx);
 return 1;
}

/* set function: wx of class  player_type */
static int toluaI_set_player_player_type_wx(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->wx = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: total_weight of class  player_type */
static int toluaI_get_player_player_type_total_weight(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->total_weight);
 return 1;
}

/* set function: total_weight of class  player_type */
static int toluaI_set_player_player_type_total_weight(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->total_weight = ((s32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: inven_cnt of class  player_type */
static int toluaI_get_player_player_type_inven_cnt(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->inven_cnt);
 return 1;
}

/* set function: inven_cnt of class  player_type */
static int toluaI_set_player_player_type_inven_cnt(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->inven_cnt = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: equip_cnt of class  player_type */
static int toluaI_get_player_player_type_equip_cnt(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->equip_cnt);
 return 1;
}

/* set function: equip_cnt of class  player_type */
static int toluaI_set_player_player_type_equip_cnt(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->equip_cnt = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: target_set of class  player_type */
static int toluaI_get_player_player_type_target_set(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->target_set);
 return 1;
}

/* set function: target_set of class  player_type */
static int toluaI_set_player_player_type_target_set(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->target_set = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: target_who of class  player_type */
static int toluaI_get_player_player_type_target_who(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->target_who);
 return 1;
}

/* set function: target_who of class  player_type */
static int toluaI_set_player_player_type_target_who(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->target_who = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: target_row of class  player_type */
static int toluaI_get_player_player_type_target_row(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->target_row);
 return 1;
}

/* set function: target_row of class  player_type */
static int toluaI_set_player_player_type_target_row(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->target_row = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: target_col of class  player_type */
static int toluaI_get_player_player_type_target_col(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->target_col);
 return 1;
}

/* set function: target_col of class  player_type */
static int toluaI_set_player_player_type_target_col(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->target_col = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: health_who of class  player_type */
static int toluaI_get_player_player_type_health_who(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->health_who);
 return 1;
}

/* set function: health_who of class  player_type */
static int toluaI_set_player_player_type_health_who(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->health_who = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: monster_race_idx of class  player_type */
static int toluaI_get_player_player_type_monster_race_idx(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->monster_race_idx);
 return 1;
}

/* set function: monster_race_idx of class  player_type */
static int toluaI_set_player_player_type_monster_race_idx(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->monster_race_idx = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: object_kind_idx of class  player_type */
static int toluaI_get_player_player_type_object_kind_idx(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->object_kind_idx);
 return 1;
}

/* set function: object_kind_idx of class  player_type */
static int toluaI_set_player_player_type_object_kind_idx(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->object_kind_idx = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: energy_use of class  player_type */
static int toluaI_get_player_player_type_energy_use(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->energy_use);
 return 1;
}

/* set function: energy_use of class  player_type */
static int toluaI_set_player_player_type_energy_use(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->energy_use = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: resting of class  player_type */
static int toluaI_get_player_player_type_resting(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->resting);
 return 1;
}

/* set function: resting of class  player_type */
static int toluaI_set_player_player_type_resting(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->resting = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: running of class  player_type */
static int toluaI_get_player_player_type_running(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->running);
 return 1;
}

/* set function: running of class  player_type */
static int toluaI_set_player_player_type_running(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->running = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: run_cur_dir of class  player_type */
static int toluaI_get_player_player_type_run_cur_dir(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->run_cur_dir);
 return 1;
}

/* set function: run_cur_dir of class  player_type */
static int toluaI_set_player_player_type_run_cur_dir(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->run_cur_dir = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: run_old_dir of class  player_type */
static int toluaI_get_player_player_type_run_old_dir(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->run_old_dir);
 return 1;
}

/* set function: run_old_dir of class  player_type */
static int toluaI_set_player_player_type_run_old_dir(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->run_old_dir = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: run_unused of class  player_type */
static int toluaI_get_player_player_type_run_unused(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->run_unused);
 return 1;
}

/* set function: run_unused of class  player_type */
static int toluaI_set_player_player_type_run_unused(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->run_unused = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: run_open_area of class  player_type */
static int toluaI_get_player_player_type_run_open_area(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->run_open_area);
 return 1;
}

/* set function: run_open_area of class  player_type */
static int toluaI_set_player_player_type_run_open_area(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->run_open_area = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: run_break_right of class  player_type */
static int toluaI_get_player_player_type_run_break_right(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->run_break_right);
 return 1;
}

/* set function: run_break_right of class  player_type */
static int toluaI_set_player_player_type_run_break_right(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->run_break_right = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: run_break_left of class  player_type */
static int toluaI_get_player_player_type_run_break_left(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->run_break_left);
 return 1;
}

/* set function: run_break_left of class  player_type */
static int toluaI_set_player_player_type_run_break_left(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->run_break_left = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: command_cmd of class  player_type */
static int toluaI_get_player_player_type_command_cmd(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->command_cmd);
 return 1;
}

/* set function: command_cmd of class  player_type */
static int toluaI_set_player_player_type_command_cmd(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->command_cmd = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: command_arg of class  player_type */
static int toluaI_get_player_player_type_command_arg(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->command_arg);
 return 1;
}

/* set function: command_arg of class  player_type */
static int toluaI_set_player_player_type_command_arg(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->command_arg = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: command_rep of class  player_type */
static int toluaI_get_player_player_type_command_rep(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->command_rep);
 return 1;
}

/* set function: command_rep of class  player_type */
static int toluaI_set_player_player_type_command_rep(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->command_rep = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: command_dir of class  player_type */
static int toluaI_get_player_player_type_command_dir(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->command_dir);
 return 1;
}

/* set function: command_dir of class  player_type */
static int toluaI_set_player_player_type_command_dir(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->command_dir = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: command_see of class  player_type */
static int toluaI_get_player_player_type_command_see(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->command_see);
 return 1;
}

/* set function: command_see of class  player_type */
static int toluaI_set_player_player_type_command_see(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->command_see = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: command_wrk of class  player_type */
static int toluaI_get_player_player_type_command_wrk(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->command_wrk);
 return 1;
}

/* set function: command_wrk of class  player_type */
static int toluaI_set_player_player_type_command_wrk(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->command_wrk = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: command_new of class  player_type */
static int toluaI_get_player_player_type_command_new(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->command_new);
 return 1;
}

/* set function: command_new of class  player_type */
static int toluaI_set_player_player_type_command_new(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->command_new = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: new_spells of class  player_type */
static int toluaI_get_player_player_type_new_spells(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->new_spells);
 return 1;
}

/* set function: new_spells of class  player_type */
static int toluaI_set_player_player_type_new_spells(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->new_spells = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: cumber_armor of class  player_type */
static int toluaI_get_player_player_type_cumber_armor(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->cumber_armor);
 return 1;
}

/* set function: cumber_armor of class  player_type */
static int toluaI_set_player_player_type_cumber_armor(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->cumber_armor = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: cumber_glove of class  player_type */
static int toluaI_get_player_player_type_cumber_glove(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->cumber_glove);
 return 1;
}

/* set function: cumber_glove of class  player_type */
static int toluaI_set_player_player_type_cumber_glove(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->cumber_glove = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: heavy_wield of class  player_type */
static int toluaI_get_player_player_type_heavy_wield(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->heavy_wield);
 return 1;
}

/* set function: heavy_wield of class  player_type */
static int toluaI_set_player_player_type_heavy_wield(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->heavy_wield = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: heavy_shoot of class  player_type */
static int toluaI_get_player_player_type_heavy_shoot(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->heavy_shoot);
 return 1;
}

/* set function: heavy_shoot of class  player_type */
static int toluaI_set_player_player_type_heavy_shoot(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->heavy_shoot = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: icky_wield of class  player_type */
static int toluaI_get_player_player_type_icky_wield(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->icky_wield);
 return 1;
}

/* set function: icky_wield of class  player_type */
static int toluaI_set_player_player_type_icky_wield(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->icky_wield = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: cur_lite of class  player_type */
static int toluaI_get_player_player_type_cur_lite(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->cur_lite);
 return 1;
}

/* set function: cur_lite of class  player_type */
static int toluaI_set_player_player_type_cur_lite(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->cur_lite = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: notice of class  player_type */
static int toluaI_get_player_player_type_notice(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->notice);
 return 1;
}

/* set function: notice of class  player_type */
static int toluaI_set_player_player_type_notice(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->notice = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: update of class  player_type */
static int toluaI_get_player_player_type_update(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->update);
 return 1;
}

/* set function: update of class  player_type */
static int toluaI_set_player_player_type_update(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->update = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: redraw of class  player_type */
static int toluaI_get_player_player_type_redraw(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->redraw);
 return 1;
}

/* set function: redraw of class  player_type */
static int toluaI_set_player_player_type_redraw(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->redraw = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: window of class  player_type */
static int toluaI_get_player_player_type_window(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->window);
 return 1;
}

/* set function: window of class  player_type */
static int toluaI_set_player_player_type_window(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->window = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: stat_use of class  player_type */
static int toluaI_get_player_player_type_stat_use(lua_State* tolua_S)
{
 int toluaI_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushnumber(tolua_S,(long)self->stat_use[toluaI_index]);
 return 1;
}

/* set function: stat_use of class  player_type */
static int toluaI_set_player_player_type_stat_use(lua_State* tolua_S)
{
 int toluaI_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.");
  self->stat_use[toluaI_index] = ((s16b)  tolua_getnumber(tolua_S,3,0));
 return 0;
}

/* get function: stat_top of class  player_type */
static int toluaI_get_player_player_type_stat_top(lua_State* tolua_S)
{
 int toluaI_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushnumber(tolua_S,(long)self->stat_top[toluaI_index]);
 return 1;
}

/* set function: stat_top of class  player_type */
static int toluaI_set_player_player_type_stat_top(lua_State* tolua_S)
{
 int toluaI_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.");
  self->stat_top[toluaI_index] = ((s16b)  tolua_getnumber(tolua_S,3,0));
 return 0;
}

/* get function: stat_add of class  player_type */
static int toluaI_get_player_player_type_stat_add(lua_State* tolua_S)
{
 int toluaI_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushnumber(tolua_S,(long)self->stat_add[toluaI_index]);
 return 1;
}

/* set function: stat_add of class  player_type */
static int toluaI_set_player_player_type_stat_add(lua_State* tolua_S)
{
 int toluaI_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.");
  self->stat_add[toluaI_index] = ((s16b)  tolua_getnumber(tolua_S,3,0));
 return 0;
}

/* get function: stat_ind of class  player_type */
static int toluaI_get_player_player_type_stat_ind(lua_State* tolua_S)
{
 int toluaI_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushnumber(tolua_S,(long)self->stat_ind[toluaI_index]);
 return 1;
}

/* set function: stat_ind of class  player_type */
static int toluaI_set_player_player_type_stat_ind(lua_State* tolua_S)
{
 int toluaI_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.");
  self->stat_ind[toluaI_index] = ((s16b)  tolua_getnumber(tolua_S,3,0));
 return 0;
}

/* get function: immune_acid of class  player_type */
static int toluaI_get_player_player_type_immune_acid(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->immune_acid);
 return 1;
}

/* set function: immune_acid of class  player_type */
static int toluaI_set_player_player_type_immune_acid(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->immune_acid = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: immune_elec of class  player_type */
static int toluaI_get_player_player_type_immune_elec(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->immune_elec);
 return 1;
}

/* set function: immune_elec of class  player_type */
static int toluaI_set_player_player_type_immune_elec(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->immune_elec = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: immune_fire of class  player_type */
static int toluaI_get_player_player_type_immune_fire(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->immune_fire);
 return 1;
}

/* set function: immune_fire of class  player_type */
static int toluaI_set_player_player_type_immune_fire(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->immune_fire = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: immune_cold of class  player_type */
static int toluaI_get_player_player_type_immune_cold(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->immune_cold);
 return 1;
}

/* set function: immune_cold of class  player_type */
static int toluaI_set_player_player_type_immune_cold(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->immune_cold = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: resist_acid of class  player_type */
static int toluaI_get_player_player_type_resist_acid(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->resist_acid);
 return 1;
}

/* set function: resist_acid of class  player_type */
static int toluaI_set_player_player_type_resist_acid(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->resist_acid = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: resist_elec of class  player_type */
static int toluaI_get_player_player_type_resist_elec(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->resist_elec);
 return 1;
}

/* set function: resist_elec of class  player_type */
static int toluaI_set_player_player_type_resist_elec(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->resist_elec = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: resist_fire of class  player_type */
static int toluaI_get_player_player_type_resist_fire(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->resist_fire);
 return 1;
}

/* set function: resist_fire of class  player_type */
static int toluaI_set_player_player_type_resist_fire(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->resist_fire = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: resist_cold of class  player_type */
static int toluaI_get_player_player_type_resist_cold(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->resist_cold);
 return 1;
}

/* set function: resist_cold of class  player_type */
static int toluaI_set_player_player_type_resist_cold(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->resist_cold = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: resist_pois of class  player_type */
static int toluaI_get_player_player_type_resist_pois(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->resist_pois);
 return 1;
}

/* set function: resist_pois of class  player_type */
static int toluaI_set_player_player_type_resist_pois(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->resist_pois = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: resist_fear of class  player_type */
static int toluaI_get_player_player_type_resist_fear(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->resist_fear);
 return 1;
}

/* set function: resist_fear of class  player_type */
static int toluaI_set_player_player_type_resist_fear(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->resist_fear = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: resist_lite of class  player_type */
static int toluaI_get_player_player_type_resist_lite(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->resist_lite);
 return 1;
}

/* set function: resist_lite of class  player_type */
static int toluaI_set_player_player_type_resist_lite(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->resist_lite = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: resist_dark of class  player_type */
static int toluaI_get_player_player_type_resist_dark(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->resist_dark);
 return 1;
}

/* set function: resist_dark of class  player_type */
static int toluaI_set_player_player_type_resist_dark(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->resist_dark = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: resist_blind of class  player_type */
static int toluaI_get_player_player_type_resist_blind(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->resist_blind);
 return 1;
}

/* set function: resist_blind of class  player_type */
static int toluaI_set_player_player_type_resist_blind(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->resist_blind = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: resist_confu of class  player_type */
static int toluaI_get_player_player_type_resist_confu(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->resist_confu);
 return 1;
}

/* set function: resist_confu of class  player_type */
static int toluaI_set_player_player_type_resist_confu(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->resist_confu = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: resist_sound of class  player_type */
static int toluaI_get_player_player_type_resist_sound(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->resist_sound);
 return 1;
}

/* set function: resist_sound of class  player_type */
static int toluaI_set_player_player_type_resist_sound(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->resist_sound = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: resist_shard of class  player_type */
static int toluaI_get_player_player_type_resist_shard(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->resist_shard);
 return 1;
}

/* set function: resist_shard of class  player_type */
static int toluaI_set_player_player_type_resist_shard(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->resist_shard = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: resist_nexus of class  player_type */
static int toluaI_get_player_player_type_resist_nexus(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->resist_nexus);
 return 1;
}

/* set function: resist_nexus of class  player_type */
static int toluaI_set_player_player_type_resist_nexus(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->resist_nexus = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: resist_nethr of class  player_type */
static int toluaI_get_player_player_type_resist_nethr(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->resist_nethr);
 return 1;
}

/* set function: resist_nethr of class  player_type */
static int toluaI_set_player_player_type_resist_nethr(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->resist_nethr = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: resist_chaos of class  player_type */
static int toluaI_get_player_player_type_resist_chaos(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->resist_chaos);
 return 1;
}

/* set function: resist_chaos of class  player_type */
static int toluaI_set_player_player_type_resist_chaos(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->resist_chaos = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: resist_disen of class  player_type */
static int toluaI_get_player_player_type_resist_disen(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->resist_disen);
 return 1;
}

/* set function: resist_disen of class  player_type */
static int toluaI_set_player_player_type_resist_disen(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->resist_disen = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: sustain_str of class  player_type */
static int toluaI_get_player_player_type_sustain_str(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->sustain_str);
 return 1;
}

/* set function: sustain_str of class  player_type */
static int toluaI_set_player_player_type_sustain_str(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->sustain_str = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: sustain_int of class  player_type */
static int toluaI_get_player_player_type_sustain_int(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->sustain_int);
 return 1;
}

/* set function: sustain_int of class  player_type */
static int toluaI_set_player_player_type_sustain_int(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->sustain_int = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: sustain_wis of class  player_type */
static int toluaI_get_player_player_type_sustain_wis(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->sustain_wis);
 return 1;
}

/* set function: sustain_wis of class  player_type */
static int toluaI_set_player_player_type_sustain_wis(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->sustain_wis = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: sustain_dex of class  player_type */
static int toluaI_get_player_player_type_sustain_dex(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->sustain_dex);
 return 1;
}

/* set function: sustain_dex of class  player_type */
static int toluaI_set_player_player_type_sustain_dex(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->sustain_dex = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: sustain_con of class  player_type */
static int toluaI_get_player_player_type_sustain_con(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->sustain_con);
 return 1;
}

/* set function: sustain_con of class  player_type */
static int toluaI_set_player_player_type_sustain_con(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->sustain_con = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: sustain_chr of class  player_type */
static int toluaI_get_player_player_type_sustain_chr(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->sustain_chr);
 return 1;
}

/* set function: sustain_chr of class  player_type */
static int toluaI_set_player_player_type_sustain_chr(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->sustain_chr = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: slow_digest of class  player_type */
static int toluaI_get_player_player_type_slow_digest(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->slow_digest);
 return 1;
}

/* set function: slow_digest of class  player_type */
static int toluaI_set_player_player_type_slow_digest(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->slow_digest = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: ffall of class  player_type */
static int toluaI_get_player_player_type_ffall(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->ffall);
 return 1;
}

/* set function: ffall of class  player_type */
static int toluaI_set_player_player_type_ffall(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->ffall = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: lite of class  player_type */
static int toluaI_get_player_player_type_lite(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->lite);
 return 1;
}

/* set function: lite of class  player_type */
static int toluaI_set_player_player_type_lite(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->lite = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: regenerate of class  player_type */
static int toluaI_get_player_player_type_regenerate(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->regenerate);
 return 1;
}

/* set function: regenerate of class  player_type */
static int toluaI_set_player_player_type_regenerate(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->regenerate = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: telepathy of class  player_type */
static int toluaI_get_player_player_type_telepathy(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->telepathy);
 return 1;
}

/* set function: telepathy of class  player_type */
static int toluaI_set_player_player_type_telepathy(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->telepathy = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: see_inv of class  player_type */
static int toluaI_get_player_player_type_see_inv(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->see_inv);
 return 1;
}

/* set function: see_inv of class  player_type */
static int toluaI_set_player_player_type_see_inv(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->see_inv = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: free_act of class  player_type */
static int toluaI_get_player_player_type_free_act(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->free_act);
 return 1;
}

/* set function: free_act of class  player_type */
static int toluaI_set_player_player_type_free_act(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->free_act = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: hold_life of class  player_type */
static int toluaI_get_player_player_type_hold_life(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->hold_life);
 return 1;
}

/* set function: hold_life of class  player_type */
static int toluaI_set_player_player_type_hold_life(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->hold_life = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: impact of class  player_type */
static int toluaI_get_player_player_type_impact(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->impact);
 return 1;
}

/* set function: impact of class  player_type */
static int toluaI_set_player_player_type_impact(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->impact = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: aggravate of class  player_type */
static int toluaI_get_player_player_type_aggravate(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->aggravate);
 return 1;
}

/* set function: aggravate of class  player_type */
static int toluaI_set_player_player_type_aggravate(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->aggravate = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: teleport of class  player_type */
static int toluaI_get_player_player_type_teleport(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->teleport);
 return 1;
}

/* set function: teleport of class  player_type */
static int toluaI_set_player_player_type_teleport(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->teleport = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: exp_drain of class  player_type */
static int toluaI_get_player_player_type_exp_drain(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->exp_drain);
 return 1;
}

/* set function: exp_drain of class  player_type */
static int toluaI_set_player_player_type_exp_drain(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->exp_drain = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: bless_blade of class  player_type */
static int toluaI_get_player_player_type_bless_blade(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->bless_blade);
 return 1;
}

/* set function: bless_blade of class  player_type */
static int toluaI_set_player_player_type_bless_blade(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->bless_blade = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: dis_to_h of class  player_type */
static int toluaI_get_player_player_type_dis_to_h(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->dis_to_h);
 return 1;
}

/* set function: dis_to_h of class  player_type */
static int toluaI_set_player_player_type_dis_to_h(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->dis_to_h = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: dis_to_d of class  player_type */
static int toluaI_get_player_player_type_dis_to_d(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->dis_to_d);
 return 1;
}

/* set function: dis_to_d of class  player_type */
static int toluaI_set_player_player_type_dis_to_d(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->dis_to_d = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: dis_to_a of class  player_type */
static int toluaI_get_player_player_type_dis_to_a(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->dis_to_a);
 return 1;
}

/* set function: dis_to_a of class  player_type */
static int toluaI_set_player_player_type_dis_to_a(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->dis_to_a = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: dis_ac of class  player_type */
static int toluaI_get_player_player_type_dis_ac(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->dis_ac);
 return 1;
}

/* set function: dis_ac of class  player_type */
static int toluaI_set_player_player_type_dis_ac(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->dis_ac = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: to_h of class  player_type */
static int toluaI_get_player_player_type_to_h(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->to_h);
 return 1;
}

/* set function: to_h of class  player_type */
static int toluaI_set_player_player_type_to_h(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->to_h = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: to_d of class  player_type */
static int toluaI_get_player_player_type_to_d(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->to_d);
 return 1;
}

/* set function: to_d of class  player_type */
static int toluaI_set_player_player_type_to_d(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->to_d = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: to_a of class  player_type */
static int toluaI_get_player_player_type_to_a(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->to_a);
 return 1;
}

/* set function: to_a of class  player_type */
static int toluaI_set_player_player_type_to_a(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->to_a = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: ac of class  player_type */
static int toluaI_get_player_player_type_ac(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->ac);
 return 1;
}

/* set function: ac of class  player_type */
static int toluaI_set_player_player_type_ac(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->ac = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: see_infra of class  player_type */
static int toluaI_get_player_player_type_see_infra(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->see_infra);
 return 1;
}

/* set function: see_infra of class  player_type */
static int toluaI_set_player_player_type_see_infra(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->see_infra = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: skill_dis of class  player_type */
static int toluaI_get_player_player_type_skill_dis(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->skill_dis);
 return 1;
}

/* set function: skill_dis of class  player_type */
static int toluaI_set_player_player_type_skill_dis(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->skill_dis = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: skill_dev of class  player_type */
static int toluaI_get_player_player_type_skill_dev(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->skill_dev);
 return 1;
}

/* set function: skill_dev of class  player_type */
static int toluaI_set_player_player_type_skill_dev(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->skill_dev = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: skill_sav of class  player_type */
static int toluaI_get_player_player_type_skill_sav(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->skill_sav);
 return 1;
}

/* set function: skill_sav of class  player_type */
static int toluaI_set_player_player_type_skill_sav(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->skill_sav = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: skill_stl of class  player_type */
static int toluaI_get_player_player_type_skill_stl(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->skill_stl);
 return 1;
}

/* set function: skill_stl of class  player_type */
static int toluaI_set_player_player_type_skill_stl(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->skill_stl = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: skill_srh of class  player_type */
static int toluaI_get_player_player_type_skill_srh(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->skill_srh);
 return 1;
}

/* set function: skill_srh of class  player_type */
static int toluaI_set_player_player_type_skill_srh(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->skill_srh = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: skill_fos of class  player_type */
static int toluaI_get_player_player_type_skill_fos(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->skill_fos);
 return 1;
}

/* set function: skill_fos of class  player_type */
static int toluaI_set_player_player_type_skill_fos(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->skill_fos = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: skill_thn of class  player_type */
static int toluaI_get_player_player_type_skill_thn(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->skill_thn);
 return 1;
}

/* set function: skill_thn of class  player_type */
static int toluaI_set_player_player_type_skill_thn(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->skill_thn = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: skill_thb of class  player_type */
static int toluaI_get_player_player_type_skill_thb(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->skill_thb);
 return 1;
}

/* set function: skill_thb of class  player_type */
static int toluaI_set_player_player_type_skill_thb(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->skill_thb = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: skill_tht of class  player_type */
static int toluaI_get_player_player_type_skill_tht(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->skill_tht);
 return 1;
}

/* set function: skill_tht of class  player_type */
static int toluaI_set_player_player_type_skill_tht(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->skill_tht = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: skill_dig of class  player_type */
static int toluaI_get_player_player_type_skill_dig(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->skill_dig);
 return 1;
}

/* set function: skill_dig of class  player_type */
static int toluaI_set_player_player_type_skill_dig(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->skill_dig = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: noise of class  player_type */
static int toluaI_get_player_player_type_noise(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->noise);
 return 1;
}

/* set function: noise of class  player_type */
static int toluaI_set_player_player_type_noise(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->noise = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: num_blow of class  player_type */
static int toluaI_get_player_player_type_num_blow(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->num_blow);
 return 1;
}

/* set function: num_blow of class  player_type */
static int toluaI_set_player_player_type_num_blow(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->num_blow = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: num_fire of class  player_type */
static int toluaI_get_player_player_type_num_fire(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->num_fire);
 return 1;
}

/* set function: num_fire of class  player_type */
static int toluaI_set_player_player_type_num_fire(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->num_fire = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: ammo_mult of class  player_type */
static int toluaI_get_player_player_type_ammo_mult(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->ammo_mult);
 return 1;
}

/* set function: ammo_mult of class  player_type */
static int toluaI_set_player_player_type_ammo_mult(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->ammo_mult = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: ammo_tval of class  player_type */
static int toluaI_get_player_player_type_ammo_tval(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->ammo_tval);
 return 1;
}

/* set function: ammo_tval of class  player_type */
static int toluaI_set_player_player_type_ammo_tval(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->ammo_tval = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: pspeed of class  player_type */
static int toluaI_get_player_player_type_pspeed(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->pspeed);
 return 1;
}

/* set function: pspeed of class  player_type */
static int toluaI_set_player_player_type_pspeed(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->pspeed = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: title of class  player_sex */
static int toluaI_get_player_player_sex_title(lua_State* tolua_S)
{
  player_sex* self = (player_sex*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushstring(tolua_S,(const char*)self->title);
 return 1;
}

/* set function: title of class  player_sex */
static int toluaI_set_player_player_sex_title(lua_State* tolua_S)
{
  player_sex* self = (player_sex*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TSTRING,0))
 TOLUA_ERR_ASSIGN;
  self->title = ((cptr)  tolua_getstring(tolua_S,2,0));
 return 0;
}

/* get function: winner of class  player_sex */
static int toluaI_get_player_player_sex_winner(lua_State* tolua_S)
{
  player_sex* self = (player_sex*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushstring(tolua_S,(const char*)self->winner);
 return 1;
}

/* set function: winner of class  player_sex */
static int toluaI_set_player_player_sex_winner(lua_State* tolua_S)
{
  player_sex* self = (player_sex*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TSTRING,0))
 TOLUA_ERR_ASSIGN;
  self->winner = ((cptr)  tolua_getstring(tolua_S,2,0));
 return 0;
}

/* get function: name of class  player_race */
static int toluaI_get_player_player_race_name(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->name);
 return 1;
}

/* set function: name of class  player_race */
static int toluaI_set_player_player_race_name(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->name = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: text of class  player_race */
static int toluaI_get_player_player_race_text(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->text);
 return 1;
}

/* set function: text of class  player_race */
static int toluaI_set_player_player_race_text(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->text = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: r_adj of class  player_race */
static int toluaI_get_player_player_race_r_adj(lua_State* tolua_S)
{
 int toluaI_index;
  player_race* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_race*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushnumber(tolua_S,(long)self->r_adj[toluaI_index]);
 return 1;
}

/* set function: r_adj of class  player_race */
static int toluaI_set_player_player_race_r_adj(lua_State* tolua_S)
{
 int toluaI_index;
  player_race* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_race*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.");
  self->r_adj[toluaI_index] = ((s16b)  tolua_getnumber(tolua_S,3,0));
 return 0;
}

/* get function: r_dis of class  player_race */
static int toluaI_get_player_player_race_r_dis(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->r_dis);
 return 1;
}

/* set function: r_dis of class  player_race */
static int toluaI_set_player_player_race_r_dis(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->r_dis = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: r_dev of class  player_race */
static int toluaI_get_player_player_race_r_dev(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->r_dev);
 return 1;
}

/* set function: r_dev of class  player_race */
static int toluaI_set_player_player_race_r_dev(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->r_dev = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: r_sav of class  player_race */
static int toluaI_get_player_player_race_r_sav(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->r_sav);
 return 1;
}

/* set function: r_sav of class  player_race */
static int toluaI_set_player_player_race_r_sav(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->r_sav = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: r_stl of class  player_race */
static int toluaI_get_player_player_race_r_stl(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->r_stl);
 return 1;
}

/* set function: r_stl of class  player_race */
static int toluaI_set_player_player_race_r_stl(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->r_stl = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: r_srh of class  player_race */
static int toluaI_get_player_player_race_r_srh(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->r_srh);
 return 1;
}

/* set function: r_srh of class  player_race */
static int toluaI_set_player_player_race_r_srh(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->r_srh = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: r_fos of class  player_race */
static int toluaI_get_player_player_race_r_fos(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->r_fos);
 return 1;
}

/* set function: r_fos of class  player_race */
static int toluaI_set_player_player_race_r_fos(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->r_fos = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: r_thn of class  player_race */
static int toluaI_get_player_player_race_r_thn(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->r_thn);
 return 1;
}

/* set function: r_thn of class  player_race */
static int toluaI_set_player_player_race_r_thn(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->r_thn = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: r_thb of class  player_race */
static int toluaI_get_player_player_race_r_thb(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->r_thb);
 return 1;
}

/* set function: r_thb of class  player_race */
static int toluaI_set_player_player_race_r_thb(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->r_thb = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: r_mhp of class  player_race */
static int toluaI_get_player_player_race_r_mhp(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->r_mhp);
 return 1;
}

/* set function: r_mhp of class  player_race */
static int toluaI_set_player_player_race_r_mhp(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->r_mhp = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: r_exp of class  player_race */
static int toluaI_get_player_player_race_r_exp(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->r_exp);
 return 1;
}

/* set function: r_exp of class  player_race */
static int toluaI_set_player_player_race_r_exp(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->r_exp = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: b_age of class  player_race */
static int toluaI_get_player_player_race_b_age(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->b_age);
 return 1;
}

/* set function: b_age of class  player_race */
static int toluaI_set_player_player_race_b_age(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->b_age = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: m_age of class  player_race */
static int toluaI_get_player_player_race_m_age(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->m_age);
 return 1;
}

/* set function: m_age of class  player_race */
static int toluaI_set_player_player_race_m_age(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->m_age = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: m_b_ht of class  player_race */
static int toluaI_get_player_player_race_m_b_ht(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->m_b_ht);
 return 1;
}

/* set function: m_b_ht of class  player_race */
static int toluaI_set_player_player_race_m_b_ht(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->m_b_ht = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: m_m_ht of class  player_race */
static int toluaI_get_player_player_race_m_m_ht(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->m_m_ht);
 return 1;
}

/* set function: m_m_ht of class  player_race */
static int toluaI_set_player_player_race_m_m_ht(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->m_m_ht = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: m_b_wt of class  player_race */
static int toluaI_get_player_player_race_m_b_wt(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->m_b_wt);
 return 1;
}

/* set function: m_b_wt of class  player_race */
static int toluaI_set_player_player_race_m_b_wt(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->m_b_wt = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: m_m_wt of class  player_race */
static int toluaI_get_player_player_race_m_m_wt(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->m_m_wt);
 return 1;
}

/* set function: m_m_wt of class  player_race */
static int toluaI_set_player_player_race_m_m_wt(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->m_m_wt = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: f_b_ht of class  player_race */
static int toluaI_get_player_player_race_f_b_ht(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->f_b_ht);
 return 1;
}

/* set function: f_b_ht of class  player_race */
static int toluaI_set_player_player_race_f_b_ht(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->f_b_ht = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: f_m_ht of class  player_race */
static int toluaI_get_player_player_race_f_m_ht(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->f_m_ht);
 return 1;
}

/* set function: f_m_ht of class  player_race */
static int toluaI_set_player_player_race_f_m_ht(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->f_m_ht = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: f_b_wt of class  player_race */
static int toluaI_get_player_player_race_f_b_wt(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->f_b_wt);
 return 1;
}

/* set function: f_b_wt of class  player_race */
static int toluaI_set_player_player_race_f_b_wt(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->f_b_wt = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: f_m_wt of class  player_race */
static int toluaI_get_player_player_race_f_m_wt(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->f_m_wt);
 return 1;
}

/* set function: f_m_wt of class  player_race */
static int toluaI_set_player_player_race_f_m_wt(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->f_m_wt = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: infra of class  player_race */
static int toluaI_get_player_player_race_infra(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->infra);
 return 1;
}

/* set function: infra of class  player_race */
static int toluaI_set_player_player_race_infra(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->infra = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: choice of class  player_race */
static int toluaI_get_player_player_race_choice(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->choice);
 return 1;
}

/* set function: choice of class  player_race */
static int toluaI_set_player_player_race_choice(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->choice = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: hist of class  player_race */
static int toluaI_get_player_player_race_hist(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->hist);
 return 1;
}

/* set function: hist of class  player_race */
static int toluaI_set_player_player_race_hist(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->hist = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flags1 of class  player_race */
static int toluaI_get_player_player_race_flags1(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags1);
 return 1;
}

/* set function: flags1 of class  player_race */
static int toluaI_set_player_player_race_flags1(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags1 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flags2 of class  player_race */
static int toluaI_get_player_player_race_flags2(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags2);
 return 1;
}

/* set function: flags2 of class  player_race */
static int toluaI_set_player_player_race_flags2(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags2 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flags3 of class  player_race */
static int toluaI_get_player_player_race_flags3(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags3);
 return 1;
}

/* set function: flags3 of class  player_race */
static int toluaI_set_player_player_race_flags3(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags3 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: tval of class  start_item */
static int toluaI_get_player_start_item_tval(lua_State* tolua_S)
{
  start_item* self = (start_item*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->tval);
 return 1;
}

/* set function: tval of class  start_item */
static int toluaI_set_player_start_item_tval(lua_State* tolua_S)
{
  start_item* self = (start_item*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->tval = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: sval of class  start_item */
static int toluaI_get_player_start_item_sval(lua_State* tolua_S)
{
  start_item* self = (start_item*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->sval);
 return 1;
}

/* set function: sval of class  start_item */
static int toluaI_set_player_start_item_sval(lua_State* tolua_S)
{
  start_item* self = (start_item*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->sval = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: min of class  start_item */
static int toluaI_get_player_start_item_min(lua_State* tolua_S)
{
  start_item* self = (start_item*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->min);
 return 1;
}

/* set function: min of class  start_item */
static int toluaI_set_player_start_item_min(lua_State* tolua_S)
{
  start_item* self = (start_item*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->min = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: max of class  start_item */
static int toluaI_get_player_start_item_max(lua_State* tolua_S)
{
  start_item* self = (start_item*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->max);
 return 1;
}

/* set function: max of class  start_item */
static int toluaI_set_player_start_item_max(lua_State* tolua_S)
{
  start_item* self = (start_item*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->max = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: name of class  player_class */
static int toluaI_get_player_player_class_name(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->name);
 return 1;
}

/* set function: name of class  player_class */
static int toluaI_set_player_player_class_name(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->name = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: title of class  player_class */
static int toluaI_get_player_player_class_title(lua_State* tolua_S)
{
 int toluaI_index;
  player_class* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_class*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=10)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushnumber(tolua_S,(long)self->title[toluaI_index]);
 return 1;
}

/* set function: title of class  player_class */
static int toluaI_set_player_player_class_title(lua_State* tolua_S)
{
 int toluaI_index;
  player_class* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_class*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=10)
 tolua_error(tolua_S,"array indexing out of range.");
  self->title[toluaI_index] = ((u32b)  tolua_getnumber(tolua_S,3,0));
 return 0;
}

/* get function: c_adj of class  player_class */
static int toluaI_get_player_player_class_c_adj(lua_State* tolua_S)
{
 int toluaI_index;
  player_class* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_class*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushnumber(tolua_S,(long)self->c_adj[toluaI_index]);
 return 1;
}

/* set function: c_adj of class  player_class */
static int toluaI_set_player_player_class_c_adj(lua_State* tolua_S)
{
 int toluaI_index;
  player_class* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_class*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.");
  self->c_adj[toluaI_index] = ((s16b)  tolua_getnumber(tolua_S,3,0));
 return 0;
}

/* get function: c_dis of class  player_class */
static int toluaI_get_player_player_class_c_dis(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->c_dis);
 return 1;
}

/* set function: c_dis of class  player_class */
static int toluaI_set_player_player_class_c_dis(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->c_dis = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: c_dev of class  player_class */
static int toluaI_get_player_player_class_c_dev(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->c_dev);
 return 1;
}

/* set function: c_dev of class  player_class */
static int toluaI_set_player_player_class_c_dev(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->c_dev = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: c_sav of class  player_class */
static int toluaI_get_player_player_class_c_sav(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->c_sav);
 return 1;
}

/* set function: c_sav of class  player_class */
static int toluaI_set_player_player_class_c_sav(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->c_sav = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: c_stl of class  player_class */
static int toluaI_get_player_player_class_c_stl(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->c_stl);
 return 1;
}

/* set function: c_stl of class  player_class */
static int toluaI_set_player_player_class_c_stl(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->c_stl = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: c_srh of class  player_class */
static int toluaI_get_player_player_class_c_srh(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->c_srh);
 return 1;
}

/* set function: c_srh of class  player_class */
static int toluaI_set_player_player_class_c_srh(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->c_srh = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: c_fos of class  player_class */
static int toluaI_get_player_player_class_c_fos(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->c_fos);
 return 1;
}

/* set function: c_fos of class  player_class */
static int toluaI_set_player_player_class_c_fos(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->c_fos = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: c_thn of class  player_class */
static int toluaI_get_player_player_class_c_thn(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->c_thn);
 return 1;
}

/* set function: c_thn of class  player_class */
static int toluaI_set_player_player_class_c_thn(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->c_thn = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: c_thb of class  player_class */
static int toluaI_get_player_player_class_c_thb(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->c_thb);
 return 1;
}

/* set function: c_thb of class  player_class */
static int toluaI_set_player_player_class_c_thb(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->c_thb = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: x_dis of class  player_class */
static int toluaI_get_player_player_class_x_dis(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->x_dis);
 return 1;
}

/* set function: x_dis of class  player_class */
static int toluaI_set_player_player_class_x_dis(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->x_dis = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: x_dev of class  player_class */
static int toluaI_get_player_player_class_x_dev(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->x_dev);
 return 1;
}

/* set function: x_dev of class  player_class */
static int toluaI_set_player_player_class_x_dev(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->x_dev = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: x_sav of class  player_class */
static int toluaI_get_player_player_class_x_sav(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->x_sav);
 return 1;
}

/* set function: x_sav of class  player_class */
static int toluaI_set_player_player_class_x_sav(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->x_sav = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: x_stl of class  player_class */
static int toluaI_get_player_player_class_x_stl(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->x_stl);
 return 1;
}

/* set function: x_stl of class  player_class */
static int toluaI_set_player_player_class_x_stl(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->x_stl = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: x_srh of class  player_class */
static int toluaI_get_player_player_class_x_srh(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->x_srh);
 return 1;
}

/* set function: x_srh of class  player_class */
static int toluaI_set_player_player_class_x_srh(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->x_srh = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: x_fos of class  player_class */
static int toluaI_get_player_player_class_x_fos(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->x_fos);
 return 1;
}

/* set function: x_fos of class  player_class */
static int toluaI_set_player_player_class_x_fos(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->x_fos = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: x_thn of class  player_class */
static int toluaI_get_player_player_class_x_thn(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->x_thn);
 return 1;
}

/* set function: x_thn of class  player_class */
static int toluaI_set_player_player_class_x_thn(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->x_thn = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: x_thb of class  player_class */
static int toluaI_get_player_player_class_x_thb(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->x_thb);
 return 1;
}

/* set function: x_thb of class  player_class */
static int toluaI_set_player_player_class_x_thb(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->x_thb = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: c_mhp of class  player_class */
static int toluaI_get_player_player_class_c_mhp(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->c_mhp);
 return 1;
}

/* set function: c_mhp of class  player_class */
static int toluaI_set_player_player_class_c_mhp(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->c_mhp = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: c_exp of class  player_class */
static int toluaI_get_player_player_class_c_exp(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->c_exp);
 return 1;
}

/* set function: c_exp of class  player_class */
static int toluaI_set_player_player_class_c_exp(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->c_exp = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flags of class  player_class */
static int toluaI_get_player_player_class_flags(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags);
 return 1;
}

/* set function: flags of class  player_class */
static int toluaI_set_player_player_class_flags(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: max_attacks of class  player_class */
static int toluaI_get_player_player_class_max_attacks(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->max_attacks);
 return 1;
}

/* set function: max_attacks of class  player_class */
static int toluaI_set_player_player_class_max_attacks(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->max_attacks = ((u16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: min_weight of class  player_class */
static int toluaI_get_player_player_class_min_weight(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->min_weight);
 return 1;
}

/* set function: min_weight of class  player_class */
static int toluaI_set_player_player_class_min_weight(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->min_weight = ((u16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: att_multiply of class  player_class */
static int toluaI_get_player_player_class_att_multiply(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->att_multiply);
 return 1;
}

/* set function: att_multiply of class  player_class */
static int toluaI_set_player_player_class_att_multiply(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->att_multiply = ((u16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: spell_book of class  player_class */
static int toluaI_get_player_player_class_spell_book(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->spell_book);
 return 1;
}

/* set function: spell_book of class  player_class */
static int toluaI_set_player_player_class_spell_book(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->spell_book = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: spell_stat of class  player_class */
static int toluaI_get_player_player_class_spell_stat(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->spell_stat);
 return 1;
}

/* set function: spell_stat of class  player_class */
static int toluaI_set_player_player_class_spell_stat(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->spell_stat = ((u16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: spell_type of class  player_class */
static int toluaI_get_player_player_class_spell_type(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->spell_type);
 return 1;
}

/* set function: spell_type of class  player_class */
static int toluaI_set_player_player_class_spell_type(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->spell_type = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: spell_first of class  player_class */
static int toluaI_get_player_player_class_spell_first(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->spell_first);
 return 1;
}

/* set function: spell_first of class  player_class */
static int toluaI_set_player_player_class_spell_first(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->spell_first = ((u16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: spell_weight of class  player_class */
static int toluaI_get_player_player_class_spell_weight(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->spell_weight);
 return 1;
}

/* set function: spell_weight of class  player_class */
static int toluaI_set_player_player_class_spell_weight(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->spell_weight = ((u16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: sense_base of class  player_class */
static int toluaI_get_player_player_class_sense_base(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->sense_base);
 return 1;
}

/* set function: sense_base of class  player_class */
static int toluaI_set_player_player_class_sense_base(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->sense_base = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: sense_div of class  player_class */
static int toluaI_get_player_player_class_sense_div(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->sense_div);
 return 1;
}

/* set function: sense_div of class  player_class */
static int toluaI_set_player_player_class_sense_div(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->sense_div = ((u16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: start_items of class  player_class */
static int toluaI_get_player_player_class_start_items(lua_State* tolua_S)
{
 int toluaI_index;
  player_class* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_class*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=MAX_START_ITEMS)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushusertype(tolua_S,(void*)&self->start_items[toluaI_index],tolua_tag(tolua_S,"start_item"));
 return 1;
}

/* set function: start_items of class  player_class */
static int toluaI_set_player_player_class_start_items(lua_State* tolua_S)
{
 int toluaI_index;
  player_class* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_class*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=MAX_START_ITEMS)
 tolua_error(tolua_S,"array indexing out of range.");
  self->start_items[toluaI_index] = *((start_item*)  tolua_getusertype(tolua_S,3,0));
 return 0;
}

/* get function: spells of class  player_class */
static int toluaI_get_player_player_class_spells(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushusertype(tolua_S,(void*)&self->spells,tolua_tag(tolua_S,"player_magic"));
 return 1;
}

/* set function: spells of class  player_class */
static int toluaI_set_player_player_class_spells(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"player_magic"),0))
 TOLUA_ERR_ASSIGN;
  self->spells = *((player_magic*)  tolua_getusertype(tolua_S,2,0));
 return 0;
}

/* get function: text of class  hist_type */
static int toluaI_get_player_hist_type_text(lua_State* tolua_S)
{
  hist_type* self = (hist_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->text);
 return 1;
}

/* set function: text of class  hist_type */
static int toluaI_set_player_hist_type_text(lua_State* tolua_S)
{
  hist_type* self = (hist_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->text = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: roll of class  hist_type */
static int toluaI_get_player_hist_type_roll(lua_State* tolua_S)
{
  hist_type* self = (hist_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->roll);
 return 1;
}

/* set function: roll of class  hist_type */
static int toluaI_set_player_hist_type_roll(lua_State* tolua_S)
{
  hist_type* self = (hist_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->roll = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: chart of class  hist_type */
static int toluaI_get_player_hist_type_chart(lua_State* tolua_S)
{
  hist_type* self = (hist_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->chart);
 return 1;
}

/* set function: chart of class  hist_type */
static int toluaI_set_player_hist_type_chart(lua_State* tolua_S)
{
  hist_type* self = (hist_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->chart = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: next of class  hist_type */
static int toluaI_get_player_hist_type_next(lua_State* tolua_S)
{
  hist_type* self = (hist_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->next);
 return 1;
}

/* set function: next of class  hist_type */
static int toluaI_set_player_hist_type_next(lua_State* tolua_S)
{
  hist_type* self = (hist_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->next = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: bonus of class  hist_type */
static int toluaI_get_player_hist_type_bonus(lua_State* tolua_S)
{
  hist_type* self = (hist_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->bonus);
 return 1;
}

/* set function: bonus of class  hist_type */
static int toluaI_set_player_hist_type_bonus(lua_State* tolua_S)
{
  hist_type* self = (hist_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->bonus = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: full_name of class  player_other */
static int toluaI_get_player_player_other_full_name(lua_State* tolua_S)
{
 int toluaI_index;
  player_other* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_other*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=32)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushnumber(tolua_S,(long)self->full_name[toluaI_index]);
 return 1;
}

/* set function: full_name of class  player_other */
static int toluaI_set_player_player_other_full_name(lua_State* tolua_S)
{
 int toluaI_index;
  player_other* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_other*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=32)
 tolua_error(tolua_S,"array indexing out of range.");
  self->full_name[toluaI_index] = ((char)  tolua_getnumber(tolua_S,3,0));
 return 0;
}

/* get function: base_name of class  player_other */
static int toluaI_get_player_player_other_base_name(lua_State* tolua_S)
{
 int toluaI_index;
  player_other* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_other*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=32)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushnumber(tolua_S,(long)self->base_name[toluaI_index]);
 return 1;
}

/* set function: base_name of class  player_other */
static int toluaI_set_player_player_other_base_name(lua_State* tolua_S)
{
 int toluaI_index;
  player_other* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_other*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=32)
 tolua_error(tolua_S,"array indexing out of range.");
  self->base_name[toluaI_index] = ((char)  tolua_getnumber(tolua_S,3,0));
 return 0;
}

/* get function: opt of class  player_other */
static int toluaI_get_player_player_other_opt(lua_State* tolua_S)
{
 int toluaI_index;
  player_other* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_other*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=OPT_MAX)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushbool(tolua_S,(int)self->opt[toluaI_index]);
 return 1;
}

/* set function: opt of class  player_other */
static int toluaI_set_player_player_other_opt(lua_State* tolua_S)
{
 int toluaI_index;
  player_other* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_other*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=OPT_MAX)
 tolua_error(tolua_S,"array indexing out of range.");
  self->opt[toluaI_index] = ((bool)  tolua_getbool(tolua_S,3,0));
 return 0;
}

/* get function: window_flag of class  player_other */
static int toluaI_get_player_player_other_window_flag(lua_State* tolua_S)
{
 int toluaI_index;
  player_other* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_other*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=ANGBAND_TERM_MAX)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushnumber(tolua_S,(long)self->window_flag[toluaI_index]);
 return 1;
}

/* set function: window_flag of class  player_other */
static int toluaI_set_player_player_other_window_flag(lua_State* tolua_S)
{
 int toluaI_index;
  player_other* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_other*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=ANGBAND_TERM_MAX)
 tolua_error(tolua_S,"array indexing out of range.");
  self->window_flag[toluaI_index] = ((u32b)  tolua_getnumber(tolua_S,3,0));
 return 0;
}

/* get function: hitpoint_warn of class  player_other */
static int toluaI_get_player_player_other_hitpoint_warn(lua_State* tolua_S)
{
  player_other* self = (player_other*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->hitpoint_warn);
 return 1;
}

/* set function: hitpoint_warn of class  player_other */
static int toluaI_set_player_player_other_hitpoint_warn(lua_State* tolua_S)
{
  player_other* self = (player_other*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->hitpoint_warn = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: delay_factor of class  player_other */
static int toluaI_get_player_player_other_delay_factor(lua_State* tolua_S)
{
  player_other* self = (player_other*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->delay_factor);
 return 1;
}

/* set function: delay_factor of class  player_other */
static int toluaI_set_player_player_other_delay_factor(lua_State* tolua_S)
{
  player_other* self = (player_other*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->delay_factor = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: sp_ptr */
static int toluaI_get_player_sp_ptr(lua_State* tolua_S)
{
 tolua_pushusertype(tolua_S,(void*)sp_ptr,tolua_tag(tolua_S,"const player_sex"));
 return 1;
}

/* get function: rp_ptr */
static int toluaI_get_player_rp_ptr(lua_State* tolua_S)
{
 tolua_pushusertype(tolua_S,(void*)rp_ptr,tolua_tag(tolua_S,"const player_race"));
 return 1;
}

/* get function: cp_ptr */
static int toluaI_get_player_cp_ptr(lua_State* tolua_S)
{
 tolua_pushusertype(tolua_S,(void*)cp_ptr,tolua_tag(tolua_S,"const player_class"));
 return 1;
}

/* get function: mp_ptr */
static int toluaI_get_player_mp_ptr(lua_State* tolua_S)
{
 tolua_pushusertype(tolua_S,(void*)mp_ptr,tolua_tag(tolua_S,"const player_magic"));
 return 1;
}

/* get function: op_ptr */
static int toluaI_get_player_op_ptr(lua_State* tolua_S)
{
 tolua_pushusertype(tolua_S,(void*)op_ptr,tolua_tag(tolua_S,"player_other"));
 return 1;
}

/* set function: op_ptr */
static int toluaI_set_player_op_ptr(lua_State* tolua_S)
{
 if (!tolua_istype(tolua_S,1,tolua_tag(tolua_S,"player_other"),0))
 TOLUA_ERR_ASSIGN;
  op_ptr = ((player_other*)  tolua_getusertype(tolua_S,1,0));
 return 0;
}

/* get function: p_ptr */
static int toluaI_get_player_player(lua_State* tolua_S)
{
 tolua_pushusertype(tolua_S,(void*)p_ptr,tolua_tag(tolua_S,"player_type"));
 return 1;
}

/* set function: p_ptr */
static int toluaI_set_player_player(lua_State* tolua_S)
{
 if (!tolua_istype(tolua_S,1,tolua_tag(tolua_S,"player_type"),0))
 TOLUA_ERR_ASSIGN;
  p_ptr = ((player_type*)  tolua_getusertype(tolua_S,1,0));
 return 0;
}

/* function: set_blind */
static int toluaI_player_set_blind00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int v = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  set_blind(v);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_blind'.");
 return 0;
}

/* function: set_confused */
static int toluaI_player_set_confused00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int v = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  set_confused(v);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_confused'.");
 return 0;
}

/* function: set_poisoned */
static int toluaI_player_set_poisoned00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int v = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  set_poisoned(v);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_poisoned'.");
 return 0;
}

/* function: set_afraid */
static int toluaI_player_set_afraid00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int v = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  set_afraid(v);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_afraid'.");
 return 0;
}

/* function: set_paralyzed */
static int toluaI_player_set_paralyzed00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int v = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  set_paralyzed(v);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_paralyzed'.");
 return 0;
}

/* function: set_image */
static int toluaI_player_set_image00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int v = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  set_image(v);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_image'.");
 return 0;
}

/* function: set_fast */
static int toluaI_player_set_fast00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int v = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  set_fast(v);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_fast'.");
 return 0;
}

/* function: set_slow */
static int toluaI_player_set_slow00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int v = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  set_slow(v);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_slow'.");
 return 0;
}

/* function: set_shield */
static int toluaI_player_set_shield00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int v = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  set_shield(v);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_shield'.");
 return 0;
}

/* function: set_blessed */
static int toluaI_player_set_blessed00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int v = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  set_blessed(v);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_blessed'.");
 return 0;
}

/* function: set_hero */
static int toluaI_player_set_hero00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int v = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  set_hero(v);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_hero'.");
 return 0;
}

/* function: set_shero */
static int toluaI_player_set_shero00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int v = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  set_shero(v);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_shero'.");
 return 0;
}

/* function: set_protevil */
static int toluaI_player_set_protevil00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int v = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  set_protevil(v);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_protevil'.");
 return 0;
}

/* function: set_invuln */
static int toluaI_player_set_invuln00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int v = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  set_invuln(v);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_invuln'.");
 return 0;
}

/* function: set_tim_invis */
static int toluaI_player_set_tim_invis00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int v = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  set_tim_invis(v);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_tim_invis'.");
 return 0;
}

/* function: set_tim_infra */
static int toluaI_player_set_tim_infra00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int v = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  set_tim_infra(v);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_tim_infra'.");
 return 0;
}

/* function: set_oppose_acid */
static int toluaI_player_set_oppose_acid00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int v = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  set_oppose_acid(v);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_oppose_acid'.");
 return 0;
}

/* function: set_oppose_elec */
static int toluaI_player_set_oppose_elec00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int v = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  set_oppose_elec(v);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_oppose_elec'.");
 return 0;
}

/* function: set_oppose_fire */
static int toluaI_player_set_oppose_fire00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int v = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  set_oppose_fire(v);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_oppose_fire'.");
 return 0;
}

/* function: set_oppose_cold */
static int toluaI_player_set_oppose_cold00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int v = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  set_oppose_cold(v);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_oppose_cold'.");
 return 0;
}

/* function: set_oppose_pois */
static int toluaI_player_set_oppose_pois00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int v = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  set_oppose_pois(v);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_oppose_pois'.");
 return 0;
}

/* function: set_stun */
static int toluaI_player_set_stun00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int v = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  set_stun(v);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_stun'.");
 return 0;
}

/* function: set_cut */
static int toluaI_player_set_cut00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int v = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  set_cut(v);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_cut'.");
 return 0;
}

/* function: set_food */
static int toluaI_player_set_food00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int v = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  set_food(v);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_food'.");
 return 0;
}

/* function: check_experience */
static int toluaI_player_check_experience00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  check_experience();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'check_experience'.");
 return 0;
}

/* function: gain_exp */
static int toluaI_player_gain_exp00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  s32b amount = ((s32b)  tolua_getnumber(tolua_S,1,0));
 {
  gain_exp(amount);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'gain_exp'.");
 return 0;
}

/* function: lose_exp */
static int toluaI_player_lose_exp00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  s32b amount = ((s32b)  tolua_getnumber(tolua_S,1,0));
 {
  lose_exp(amount);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'lose_exp'.");
 return 0;
}

/* function: hp_player */
static int toluaI_player_hp_player00(lua_State* tolua_S)
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
  bool toluaI_ret = (bool)  hp_player(num);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'hp_player'.");
 return 0;
}

/* function: do_dec_stat */
static int toluaI_player_do_dec_stat00(lua_State* tolua_S)
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
  bool toluaI_ret = (bool)  do_dec_stat(stat);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'do_dec_stat'.");
 return 0;
}

/* function: do_res_stat */
static int toluaI_player_do_res_stat00(lua_State* tolua_S)
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
  bool toluaI_ret = (bool)  do_res_stat(stat);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'do_res_stat'.");
 return 0;
}

/* function: do_inc_stat */
static int toluaI_player_do_inc_stat00(lua_State* tolua_S)
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
  bool toluaI_ret = (bool)  do_inc_stat(stat);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'do_inc_stat'.");
 return 0;
}

/* function: take_hit */
static int toluaI_player_take_hit00(lua_State* tolua_S)
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

/* function: inc_stat */
static int toluaI_player_inc_stat00(lua_State* tolua_S)
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
static int toluaI_player_dec_stat00(lua_State* tolua_S)
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
static int toluaI_player_res_stat00(lua_State* tolua_S)
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

/* function: restore_level */
static int toluaI_player_restore_level00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  bool toluaI_ret = (bool)  restore_level();
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'restore_level'.");
 return 0;
}

/* Open function */
int tolua_player_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 toluaI_reg_types(tolua_S);
 tolua_constant(tolua_S,NULL,"A_STR",A_STR);
 tolua_constant(tolua_S,NULL,"A_INT",A_INT);
 tolua_constant(tolua_S,NULL,"A_WIS",A_WIS);
 tolua_constant(tolua_S,NULL,"A_DEX",A_DEX);
 tolua_constant(tolua_S,NULL,"A_CON",A_CON);
 tolua_constant(tolua_S,NULL,"A_CHR",A_CHR);
 tolua_constant(tolua_S,NULL,"A_MAX",A_MAX);
 tolua_constant(tolua_S,NULL,"SEX_FEMALE",SEX_FEMALE);
 tolua_constant(tolua_S,NULL,"SEX_MALE",SEX_MALE);
 tolua_constant(tolua_S,NULL,"PY_MAX_EXP",PY_MAX_EXP);
 tolua_constant(tolua_S,NULL,"PY_MAX_GOLD",PY_MAX_GOLD);
 tolua_constant(tolua_S,NULL,"PY_MAX_LEVEL",PY_MAX_LEVEL);
 tolua_constant(tolua_S,NULL,"PY_FOOD_MAX",PY_FOOD_MAX);
 tolua_constant(tolua_S,NULL,"PY_FOOD_FULL",PY_FOOD_FULL);
 tolua_constant(tolua_S,NULL,"PY_FOOD_ALERT",PY_FOOD_ALERT);
 tolua_constant(tolua_S,NULL,"PY_FOOD_WEAK",PY_FOOD_WEAK);
 tolua_constant(tolua_S,NULL,"PY_FOOD_FAINT",PY_FOOD_FAINT);
 tolua_constant(tolua_S,NULL,"PY_FOOD_STARVE",PY_FOOD_STARVE);
 tolua_constant(tolua_S,NULL,"PY_REGEN_NORMAL",PY_REGEN_NORMAL);
 tolua_constant(tolua_S,NULL,"PY_REGEN_WEAK",PY_REGEN_WEAK);
 tolua_constant(tolua_S,NULL,"PY_REGEN_FAINT",PY_REGEN_FAINT);
 tolua_constant(tolua_S,NULL,"PY_REGEN_HPBASE",PY_REGEN_HPBASE);
 tolua_constant(tolua_S,NULL,"PY_REGEN_MNBASE",PY_REGEN_MNBASE);
 tolua_cclass(tolua_S,"player_type","");
 tolua_tablevar(tolua_S,"player_type","py",toluaI_get_player_player_type_py,toluaI_set_player_player_type_py);
 tolua_tablevar(tolua_S,"player_type","px",toluaI_get_player_player_type_px,toluaI_set_player_player_type_px);
 tolua_tablevar(tolua_S,"player_type","psex",toluaI_get_player_player_type_psex,toluaI_set_player_player_type_psex);
 tolua_tablevar(tolua_S,"player_type","prace",toluaI_get_player_player_type_prace,toluaI_set_player_player_type_prace);
 tolua_tablevar(tolua_S,"player_type","pclass",toluaI_get_player_player_type_pclass,toluaI_set_player_player_type_pclass);
 tolua_tablevar(tolua_S,"player_type","hitdie",toluaI_get_player_player_type_hitdie,toluaI_set_player_player_type_hitdie);
 tolua_tablevar(tolua_S,"player_type","expfact",toluaI_get_player_player_type_expfact,toluaI_set_player_player_type_expfact);
 tolua_tablevar(tolua_S,"player_type","age",toluaI_get_player_player_type_age,toluaI_set_player_player_type_age);
 tolua_tablevar(tolua_S,"player_type","ht",toluaI_get_player_player_type_ht,toluaI_set_player_player_type_ht);
 tolua_tablevar(tolua_S,"player_type","wt",toluaI_get_player_player_type_wt,toluaI_set_player_player_type_wt);
 tolua_tablevar(tolua_S,"player_type","sc",toluaI_get_player_player_type_sc,toluaI_set_player_player_type_sc);
 tolua_tablevar(tolua_S,"player_type","au",toluaI_get_player_player_type_au,toluaI_set_player_player_type_au);
 tolua_tablevar(tolua_S,"player_type","max_depth",toluaI_get_player_player_type_max_depth,toluaI_set_player_player_type_max_depth);
 tolua_tablevar(tolua_S,"player_type","depth",toluaI_get_player_player_type_depth,toluaI_set_player_player_type_depth);
 tolua_tablevar(tolua_S,"player_type","max_lev",toluaI_get_player_player_type_max_lev,toluaI_set_player_player_type_max_lev);
 tolua_tablevar(tolua_S,"player_type","lev",toluaI_get_player_player_type_lev,toluaI_set_player_player_type_lev);
 tolua_tablevar(tolua_S,"player_type","max_exp",toluaI_get_player_player_type_max_exp,toluaI_set_player_player_type_max_exp);
 tolua_tablevar(tolua_S,"player_type","exp",toluaI_get_player_player_type_exp,toluaI_set_player_player_type_exp);
 tolua_tablevar(tolua_S,"player_type","exp_frac",toluaI_get_player_player_type_exp_frac,toluaI_set_player_player_type_exp_frac);
 tolua_tablevar(tolua_S,"player_type","mhp",toluaI_get_player_player_type_mhp,toluaI_set_player_player_type_mhp);
 tolua_tablevar(tolua_S,"player_type","chp",toluaI_get_player_player_type_chp,toluaI_set_player_player_type_chp);
 tolua_tablevar(tolua_S,"player_type","chp_frac",toluaI_get_player_player_type_chp_frac,toluaI_set_player_player_type_chp_frac);
 tolua_tablevar(tolua_S,"player_type","msp",toluaI_get_player_player_type_msp,toluaI_set_player_player_type_msp);
 tolua_tablevar(tolua_S,"player_type","csp",toluaI_get_player_player_type_csp,toluaI_set_player_player_type_csp);
 tolua_tablevar(tolua_S,"player_type","csp_frac",toluaI_get_player_player_type_csp_frac,toluaI_set_player_player_type_csp_frac);
 tolua_tablearray(tolua_S,"player_type","stat_max",toluaI_get_player_player_type_stat_max,toluaI_set_player_player_type_stat_max);
 tolua_tablearray(tolua_S,"player_type","stat_cur",toluaI_get_player_player_type_stat_cur,toluaI_set_player_player_type_stat_cur);
 tolua_tablevar(tolua_S,"player_type","fast",toluaI_get_player_player_type_fast,toluaI_set_player_player_type_fast);
 tolua_tablevar(tolua_S,"player_type","slow",toluaI_get_player_player_type_slow,toluaI_set_player_player_type_slow);
 tolua_tablevar(tolua_S,"player_type","blind",toluaI_get_player_player_type_blind,toluaI_set_player_player_type_blind);
 tolua_tablevar(tolua_S,"player_type","paralyzed",toluaI_get_player_player_type_paralyzed,toluaI_set_player_player_type_paralyzed);
 tolua_tablevar(tolua_S,"player_type","confused",toluaI_get_player_player_type_confused,toluaI_set_player_player_type_confused);
 tolua_tablevar(tolua_S,"player_type","afraid",toluaI_get_player_player_type_afraid,toluaI_set_player_player_type_afraid);
 tolua_tablevar(tolua_S,"player_type","image",toluaI_get_player_player_type_image,toluaI_set_player_player_type_image);
 tolua_tablevar(tolua_S,"player_type","poisoned",toluaI_get_player_player_type_poisoned,toluaI_set_player_player_type_poisoned);
 tolua_tablevar(tolua_S,"player_type","cut",toluaI_get_player_player_type_cut,toluaI_set_player_player_type_cut);
 tolua_tablevar(tolua_S,"player_type","stun",toluaI_get_player_player_type_stun,toluaI_set_player_player_type_stun);
 tolua_tablevar(tolua_S,"player_type","protevil",toluaI_get_player_player_type_protevil,toluaI_set_player_player_type_protevil);
 tolua_tablevar(tolua_S,"player_type","invuln",toluaI_get_player_player_type_invuln,toluaI_set_player_player_type_invuln);
 tolua_tablevar(tolua_S,"player_type","hero",toluaI_get_player_player_type_hero,toluaI_set_player_player_type_hero);
 tolua_tablevar(tolua_S,"player_type","shero",toluaI_get_player_player_type_shero,toluaI_set_player_player_type_shero);
 tolua_tablevar(tolua_S,"player_type","shield",toluaI_get_player_player_type_shield,toluaI_set_player_player_type_shield);
 tolua_tablevar(tolua_S,"player_type","blessed",toluaI_get_player_player_type_blessed,toluaI_set_player_player_type_blessed);
 tolua_tablevar(tolua_S,"player_type","tim_invis",toluaI_get_player_player_type_tim_invis,toluaI_set_player_player_type_tim_invis);
 tolua_tablevar(tolua_S,"player_type","tim_infra",toluaI_get_player_player_type_tim_infra,toluaI_set_player_player_type_tim_infra);
 tolua_tablevar(tolua_S,"player_type","oppose_acid",toluaI_get_player_player_type_oppose_acid,toluaI_set_player_player_type_oppose_acid);
 tolua_tablevar(tolua_S,"player_type","oppose_elec",toluaI_get_player_player_type_oppose_elec,toluaI_set_player_player_type_oppose_elec);
 tolua_tablevar(tolua_S,"player_type","oppose_fire",toluaI_get_player_player_type_oppose_fire,toluaI_set_player_player_type_oppose_fire);
 tolua_tablevar(tolua_S,"player_type","oppose_cold",toluaI_get_player_player_type_oppose_cold,toluaI_set_player_player_type_oppose_cold);
 tolua_tablevar(tolua_S,"player_type","oppose_pois",toluaI_get_player_player_type_oppose_pois,toluaI_set_player_player_type_oppose_pois);
 tolua_tablevar(tolua_S,"player_type","word_recall",toluaI_get_player_player_type_word_recall,toluaI_set_player_player_type_word_recall);
 tolua_tablevar(tolua_S,"player_type","energy",toluaI_get_player_player_type_energy,toluaI_set_player_player_type_energy);
 tolua_tablevar(tolua_S,"player_type","food",toluaI_get_player_player_type_food,toluaI_set_player_player_type_food);
 tolua_tablevar(tolua_S,"player_type","confusing",toluaI_get_player_player_type_confusing,toluaI_set_player_player_type_confusing);
 tolua_tablevar(tolua_S,"player_type","searching",toluaI_get_player_player_type_searching,toluaI_set_player_player_type_searching);
 tolua_tablevar(tolua_S,"player_type","spell_learned1",toluaI_get_player_player_type_spell_learned1,toluaI_set_player_player_type_spell_learned1);
 tolua_tablevar(tolua_S,"player_type","spell_learned2",toluaI_get_player_player_type_spell_learned2,toluaI_set_player_player_type_spell_learned2);
 tolua_tablevar(tolua_S,"player_type","spell_worked1",toluaI_get_player_player_type_spell_worked1,toluaI_set_player_player_type_spell_worked1);
 tolua_tablevar(tolua_S,"player_type","spell_worked2",toluaI_get_player_player_type_spell_worked2,toluaI_set_player_player_type_spell_worked2);
 tolua_tablevar(tolua_S,"player_type","spell_forgotten1",toluaI_get_player_player_type_spell_forgotten1,toluaI_set_player_player_type_spell_forgotten1);
 tolua_tablevar(tolua_S,"player_type","spell_forgotten2",toluaI_get_player_player_type_spell_forgotten2,toluaI_set_player_player_type_spell_forgotten2);
 tolua_tablearray(tolua_S,"player_type","spell_order",toluaI_get_player_player_type_spell_order,toluaI_set_player_player_type_spell_order);
 tolua_tablearray(tolua_S,"player_type","player_hp",toluaI_get_player_player_type_player_hp,toluaI_set_player_player_type_player_hp);
 tolua_tablearray(tolua_S,"player_type","died_from",toluaI_get_player_player_type_died_from,toluaI_set_player_player_type_died_from);
 tolua_tablevar(tolua_S,"player_type","total_winner",toluaI_get_player_player_type_total_winner,toluaI_set_player_player_type_total_winner);
 tolua_tablevar(tolua_S,"player_type","panic_save",toluaI_get_player_player_type_panic_save,toluaI_set_player_player_type_panic_save);
 tolua_tablevar(tolua_S,"player_type","noscore",toluaI_get_player_player_type_noscore,toluaI_set_player_player_type_noscore);
 tolua_tablevar(tolua_S,"player_type","is_dead",toluaI_get_player_player_type_is_dead,toluaI_set_player_player_type_is_dead);
 tolua_tablevar(tolua_S,"player_type","wizard",toluaI_get_player_player_type_wizard,toluaI_set_player_player_type_wizard);
 tolua_tablevar(tolua_S,"player_type","playing",toluaI_get_player_player_type_playing,toluaI_set_player_player_type_playing);
 tolua_tablevar(tolua_S,"player_type","leaving",toluaI_get_player_player_type_leaving,toluaI_set_player_player_type_leaving);
 tolua_tablevar(tolua_S,"player_type","create_up_stair",toluaI_get_player_player_type_create_up_stair,toluaI_set_player_player_type_create_up_stair);
 tolua_tablevar(tolua_S,"player_type","create_down_stair",toluaI_get_player_player_type_create_down_stair,toluaI_set_player_player_type_create_down_stair);
 tolua_tablevar(tolua_S,"player_type","wy",toluaI_get_player_player_type_wy,toluaI_set_player_player_type_wy);
 tolua_tablevar(tolua_S,"player_type","wx",toluaI_get_player_player_type_wx,toluaI_set_player_player_type_wx);
 tolua_tablevar(tolua_S,"player_type","total_weight",toluaI_get_player_player_type_total_weight,toluaI_set_player_player_type_total_weight);
 tolua_tablevar(tolua_S,"player_type","inven_cnt",toluaI_get_player_player_type_inven_cnt,toluaI_set_player_player_type_inven_cnt);
 tolua_tablevar(tolua_S,"player_type","equip_cnt",toluaI_get_player_player_type_equip_cnt,toluaI_set_player_player_type_equip_cnt);
 tolua_tablevar(tolua_S,"player_type","target_set",toluaI_get_player_player_type_target_set,toluaI_set_player_player_type_target_set);
 tolua_tablevar(tolua_S,"player_type","target_who",toluaI_get_player_player_type_target_who,toluaI_set_player_player_type_target_who);
 tolua_tablevar(tolua_S,"player_type","target_row",toluaI_get_player_player_type_target_row,toluaI_set_player_player_type_target_row);
 tolua_tablevar(tolua_S,"player_type","target_col",toluaI_get_player_player_type_target_col,toluaI_set_player_player_type_target_col);
 tolua_tablevar(tolua_S,"player_type","health_who",toluaI_get_player_player_type_health_who,toluaI_set_player_player_type_health_who);
 tolua_tablevar(tolua_S,"player_type","monster_race_idx",toluaI_get_player_player_type_monster_race_idx,toluaI_set_player_player_type_monster_race_idx);
 tolua_tablevar(tolua_S,"player_type","object_kind_idx",toluaI_get_player_player_type_object_kind_idx,toluaI_set_player_player_type_object_kind_idx);
 tolua_tablevar(tolua_S,"player_type","energy_use",toluaI_get_player_player_type_energy_use,toluaI_set_player_player_type_energy_use);
 tolua_tablevar(tolua_S,"player_type","resting",toluaI_get_player_player_type_resting,toluaI_set_player_player_type_resting);
 tolua_tablevar(tolua_S,"player_type","running",toluaI_get_player_player_type_running,toluaI_set_player_player_type_running);
 tolua_tablevar(tolua_S,"player_type","run_cur_dir",toluaI_get_player_player_type_run_cur_dir,toluaI_set_player_player_type_run_cur_dir);
 tolua_tablevar(tolua_S,"player_type","run_old_dir",toluaI_get_player_player_type_run_old_dir,toluaI_set_player_player_type_run_old_dir);
 tolua_tablevar(tolua_S,"player_type","run_unused",toluaI_get_player_player_type_run_unused,toluaI_set_player_player_type_run_unused);
 tolua_tablevar(tolua_S,"player_type","run_open_area",toluaI_get_player_player_type_run_open_area,toluaI_set_player_player_type_run_open_area);
 tolua_tablevar(tolua_S,"player_type","run_break_right",toluaI_get_player_player_type_run_break_right,toluaI_set_player_player_type_run_break_right);
 tolua_tablevar(tolua_S,"player_type","run_break_left",toluaI_get_player_player_type_run_break_left,toluaI_set_player_player_type_run_break_left);
 tolua_tablevar(tolua_S,"player_type","command_cmd",toluaI_get_player_player_type_command_cmd,toluaI_set_player_player_type_command_cmd);
 tolua_tablevar(tolua_S,"player_type","command_arg",toluaI_get_player_player_type_command_arg,toluaI_set_player_player_type_command_arg);
 tolua_tablevar(tolua_S,"player_type","command_rep",toluaI_get_player_player_type_command_rep,toluaI_set_player_player_type_command_rep);
 tolua_tablevar(tolua_S,"player_type","command_dir",toluaI_get_player_player_type_command_dir,toluaI_set_player_player_type_command_dir);
 tolua_tablevar(tolua_S,"player_type","command_see",toluaI_get_player_player_type_command_see,toluaI_set_player_player_type_command_see);
 tolua_tablevar(tolua_S,"player_type","command_wrk",toluaI_get_player_player_type_command_wrk,toluaI_set_player_player_type_command_wrk);
 tolua_tablevar(tolua_S,"player_type","command_new",toluaI_get_player_player_type_command_new,toluaI_set_player_player_type_command_new);
 tolua_tablevar(tolua_S,"player_type","new_spells",toluaI_get_player_player_type_new_spells,toluaI_set_player_player_type_new_spells);
 tolua_tablevar(tolua_S,"player_type","cumber_armor",toluaI_get_player_player_type_cumber_armor,toluaI_set_player_player_type_cumber_armor);
 tolua_tablevar(tolua_S,"player_type","cumber_glove",toluaI_get_player_player_type_cumber_glove,toluaI_set_player_player_type_cumber_glove);
 tolua_tablevar(tolua_S,"player_type","heavy_wield",toluaI_get_player_player_type_heavy_wield,toluaI_set_player_player_type_heavy_wield);
 tolua_tablevar(tolua_S,"player_type","heavy_shoot",toluaI_get_player_player_type_heavy_shoot,toluaI_set_player_player_type_heavy_shoot);
 tolua_tablevar(tolua_S,"player_type","icky_wield",toluaI_get_player_player_type_icky_wield,toluaI_set_player_player_type_icky_wield);
 tolua_tablevar(tolua_S,"player_type","cur_lite",toluaI_get_player_player_type_cur_lite,toluaI_set_player_player_type_cur_lite);
 tolua_tablevar(tolua_S,"player_type","notice",toluaI_get_player_player_type_notice,toluaI_set_player_player_type_notice);
 tolua_tablevar(tolua_S,"player_type","update",toluaI_get_player_player_type_update,toluaI_set_player_player_type_update);
 tolua_tablevar(tolua_S,"player_type","redraw",toluaI_get_player_player_type_redraw,toluaI_set_player_player_type_redraw);
 tolua_tablevar(tolua_S,"player_type","window",toluaI_get_player_player_type_window,toluaI_set_player_player_type_window);
 tolua_tablearray(tolua_S,"player_type","stat_use",toluaI_get_player_player_type_stat_use,toluaI_set_player_player_type_stat_use);
 tolua_tablearray(tolua_S,"player_type","stat_top",toluaI_get_player_player_type_stat_top,toluaI_set_player_player_type_stat_top);
 tolua_tablearray(tolua_S,"player_type","stat_add",toluaI_get_player_player_type_stat_add,toluaI_set_player_player_type_stat_add);
 tolua_tablearray(tolua_S,"player_type","stat_ind",toluaI_get_player_player_type_stat_ind,toluaI_set_player_player_type_stat_ind);
 tolua_tablevar(tolua_S,"player_type","immune_acid",toluaI_get_player_player_type_immune_acid,toluaI_set_player_player_type_immune_acid);
 tolua_tablevar(tolua_S,"player_type","immune_elec",toluaI_get_player_player_type_immune_elec,toluaI_set_player_player_type_immune_elec);
 tolua_tablevar(tolua_S,"player_type","immune_fire",toluaI_get_player_player_type_immune_fire,toluaI_set_player_player_type_immune_fire);
 tolua_tablevar(tolua_S,"player_type","immune_cold",toluaI_get_player_player_type_immune_cold,toluaI_set_player_player_type_immune_cold);
 tolua_tablevar(tolua_S,"player_type","resist_acid",toluaI_get_player_player_type_resist_acid,toluaI_set_player_player_type_resist_acid);
 tolua_tablevar(tolua_S,"player_type","resist_elec",toluaI_get_player_player_type_resist_elec,toluaI_set_player_player_type_resist_elec);
 tolua_tablevar(tolua_S,"player_type","resist_fire",toluaI_get_player_player_type_resist_fire,toluaI_set_player_player_type_resist_fire);
 tolua_tablevar(tolua_S,"player_type","resist_cold",toluaI_get_player_player_type_resist_cold,toluaI_set_player_player_type_resist_cold);
 tolua_tablevar(tolua_S,"player_type","resist_pois",toluaI_get_player_player_type_resist_pois,toluaI_set_player_player_type_resist_pois);
 tolua_tablevar(tolua_S,"player_type","resist_fear",toluaI_get_player_player_type_resist_fear,toluaI_set_player_player_type_resist_fear);
 tolua_tablevar(tolua_S,"player_type","resist_lite",toluaI_get_player_player_type_resist_lite,toluaI_set_player_player_type_resist_lite);
 tolua_tablevar(tolua_S,"player_type","resist_dark",toluaI_get_player_player_type_resist_dark,toluaI_set_player_player_type_resist_dark);
 tolua_tablevar(tolua_S,"player_type","resist_blind",toluaI_get_player_player_type_resist_blind,toluaI_set_player_player_type_resist_blind);
 tolua_tablevar(tolua_S,"player_type","resist_confu",toluaI_get_player_player_type_resist_confu,toluaI_set_player_player_type_resist_confu);
 tolua_tablevar(tolua_S,"player_type","resist_sound",toluaI_get_player_player_type_resist_sound,toluaI_set_player_player_type_resist_sound);
 tolua_tablevar(tolua_S,"player_type","resist_shard",toluaI_get_player_player_type_resist_shard,toluaI_set_player_player_type_resist_shard);
 tolua_tablevar(tolua_S,"player_type","resist_nexus",toluaI_get_player_player_type_resist_nexus,toluaI_set_player_player_type_resist_nexus);
 tolua_tablevar(tolua_S,"player_type","resist_nethr",toluaI_get_player_player_type_resist_nethr,toluaI_set_player_player_type_resist_nethr);
 tolua_tablevar(tolua_S,"player_type","resist_chaos",toluaI_get_player_player_type_resist_chaos,toluaI_set_player_player_type_resist_chaos);
 tolua_tablevar(tolua_S,"player_type","resist_disen",toluaI_get_player_player_type_resist_disen,toluaI_set_player_player_type_resist_disen);
 tolua_tablevar(tolua_S,"player_type","sustain_str",toluaI_get_player_player_type_sustain_str,toluaI_set_player_player_type_sustain_str);
 tolua_tablevar(tolua_S,"player_type","sustain_int",toluaI_get_player_player_type_sustain_int,toluaI_set_player_player_type_sustain_int);
 tolua_tablevar(tolua_S,"player_type","sustain_wis",toluaI_get_player_player_type_sustain_wis,toluaI_set_player_player_type_sustain_wis);
 tolua_tablevar(tolua_S,"player_type","sustain_dex",toluaI_get_player_player_type_sustain_dex,toluaI_set_player_player_type_sustain_dex);
 tolua_tablevar(tolua_S,"player_type","sustain_con",toluaI_get_player_player_type_sustain_con,toluaI_set_player_player_type_sustain_con);
 tolua_tablevar(tolua_S,"player_type","sustain_chr",toluaI_get_player_player_type_sustain_chr,toluaI_set_player_player_type_sustain_chr);
 tolua_tablevar(tolua_S,"player_type","slow_digest",toluaI_get_player_player_type_slow_digest,toluaI_set_player_player_type_slow_digest);
 tolua_tablevar(tolua_S,"player_type","ffall",toluaI_get_player_player_type_ffall,toluaI_set_player_player_type_ffall);
 tolua_tablevar(tolua_S,"player_type","lite",toluaI_get_player_player_type_lite,toluaI_set_player_player_type_lite);
 tolua_tablevar(tolua_S,"player_type","regenerate",toluaI_get_player_player_type_regenerate,toluaI_set_player_player_type_regenerate);
 tolua_tablevar(tolua_S,"player_type","telepathy",toluaI_get_player_player_type_telepathy,toluaI_set_player_player_type_telepathy);
 tolua_tablevar(tolua_S,"player_type","see_inv",toluaI_get_player_player_type_see_inv,toluaI_set_player_player_type_see_inv);
 tolua_tablevar(tolua_S,"player_type","free_act",toluaI_get_player_player_type_free_act,toluaI_set_player_player_type_free_act);
 tolua_tablevar(tolua_S,"player_type","hold_life",toluaI_get_player_player_type_hold_life,toluaI_set_player_player_type_hold_life);
 tolua_tablevar(tolua_S,"player_type","impact",toluaI_get_player_player_type_impact,toluaI_set_player_player_type_impact);
 tolua_tablevar(tolua_S,"player_type","aggravate",toluaI_get_player_player_type_aggravate,toluaI_set_player_player_type_aggravate);
 tolua_tablevar(tolua_S,"player_type","teleport",toluaI_get_player_player_type_teleport,toluaI_set_player_player_type_teleport);
 tolua_tablevar(tolua_S,"player_type","exp_drain",toluaI_get_player_player_type_exp_drain,toluaI_set_player_player_type_exp_drain);
 tolua_tablevar(tolua_S,"player_type","bless_blade",toluaI_get_player_player_type_bless_blade,toluaI_set_player_player_type_bless_blade);
 tolua_tablevar(tolua_S,"player_type","dis_to_h",toluaI_get_player_player_type_dis_to_h,toluaI_set_player_player_type_dis_to_h);
 tolua_tablevar(tolua_S,"player_type","dis_to_d",toluaI_get_player_player_type_dis_to_d,toluaI_set_player_player_type_dis_to_d);
 tolua_tablevar(tolua_S,"player_type","dis_to_a",toluaI_get_player_player_type_dis_to_a,toluaI_set_player_player_type_dis_to_a);
 tolua_tablevar(tolua_S,"player_type","dis_ac",toluaI_get_player_player_type_dis_ac,toluaI_set_player_player_type_dis_ac);
 tolua_tablevar(tolua_S,"player_type","to_h",toluaI_get_player_player_type_to_h,toluaI_set_player_player_type_to_h);
 tolua_tablevar(tolua_S,"player_type","to_d",toluaI_get_player_player_type_to_d,toluaI_set_player_player_type_to_d);
 tolua_tablevar(tolua_S,"player_type","to_a",toluaI_get_player_player_type_to_a,toluaI_set_player_player_type_to_a);
 tolua_tablevar(tolua_S,"player_type","ac",toluaI_get_player_player_type_ac,toluaI_set_player_player_type_ac);
 tolua_tablevar(tolua_S,"player_type","see_infra",toluaI_get_player_player_type_see_infra,toluaI_set_player_player_type_see_infra);
 tolua_tablevar(tolua_S,"player_type","skill_dis",toluaI_get_player_player_type_skill_dis,toluaI_set_player_player_type_skill_dis);
 tolua_tablevar(tolua_S,"player_type","skill_dev",toluaI_get_player_player_type_skill_dev,toluaI_set_player_player_type_skill_dev);
 tolua_tablevar(tolua_S,"player_type","skill_sav",toluaI_get_player_player_type_skill_sav,toluaI_set_player_player_type_skill_sav);
 tolua_tablevar(tolua_S,"player_type","skill_stl",toluaI_get_player_player_type_skill_stl,toluaI_set_player_player_type_skill_stl);
 tolua_tablevar(tolua_S,"player_type","skill_srh",toluaI_get_player_player_type_skill_srh,toluaI_set_player_player_type_skill_srh);
 tolua_tablevar(tolua_S,"player_type","skill_fos",toluaI_get_player_player_type_skill_fos,toluaI_set_player_player_type_skill_fos);
 tolua_tablevar(tolua_S,"player_type","skill_thn",toluaI_get_player_player_type_skill_thn,toluaI_set_player_player_type_skill_thn);
 tolua_tablevar(tolua_S,"player_type","skill_thb",toluaI_get_player_player_type_skill_thb,toluaI_set_player_player_type_skill_thb);
 tolua_tablevar(tolua_S,"player_type","skill_tht",toluaI_get_player_player_type_skill_tht,toluaI_set_player_player_type_skill_tht);
 tolua_tablevar(tolua_S,"player_type","skill_dig",toluaI_get_player_player_type_skill_dig,toluaI_set_player_player_type_skill_dig);
 tolua_tablevar(tolua_S,"player_type","noise",toluaI_get_player_player_type_noise,toluaI_set_player_player_type_noise);
 tolua_tablevar(tolua_S,"player_type","num_blow",toluaI_get_player_player_type_num_blow,toluaI_set_player_player_type_num_blow);
 tolua_tablevar(tolua_S,"player_type","num_fire",toluaI_get_player_player_type_num_fire,toluaI_set_player_player_type_num_fire);
 tolua_tablevar(tolua_S,"player_type","ammo_mult",toluaI_get_player_player_type_ammo_mult,toluaI_set_player_player_type_ammo_mult);
 tolua_tablevar(tolua_S,"player_type","ammo_tval",toluaI_get_player_player_type_ammo_tval,toluaI_set_player_player_type_ammo_tval);
 tolua_tablevar(tolua_S,"player_type","pspeed",toluaI_get_player_player_type_pspeed,toluaI_set_player_player_type_pspeed);
 tolua_cclass(tolua_S,"player_sex","");
 tolua_tablevar(tolua_S,"player_sex","title",toluaI_get_player_player_sex_title,toluaI_set_player_player_sex_title);
 tolua_tablevar(tolua_S,"player_sex","winner",toluaI_get_player_player_sex_winner,toluaI_set_player_player_sex_winner);
 tolua_cclass(tolua_S,"player_race","");
 tolua_tablevar(tolua_S,"player_race","name",toluaI_get_player_player_race_name,toluaI_set_player_player_race_name);
 tolua_tablevar(tolua_S,"player_race","text",toluaI_get_player_player_race_text,toluaI_set_player_player_race_text);
 tolua_tablearray(tolua_S,"player_race","r_adj",toluaI_get_player_player_race_r_adj,toluaI_set_player_player_race_r_adj);
 tolua_tablevar(tolua_S,"player_race","r_dis",toluaI_get_player_player_race_r_dis,toluaI_set_player_player_race_r_dis);
 tolua_tablevar(tolua_S,"player_race","r_dev",toluaI_get_player_player_race_r_dev,toluaI_set_player_player_race_r_dev);
 tolua_tablevar(tolua_S,"player_race","r_sav",toluaI_get_player_player_race_r_sav,toluaI_set_player_player_race_r_sav);
 tolua_tablevar(tolua_S,"player_race","r_stl",toluaI_get_player_player_race_r_stl,toluaI_set_player_player_race_r_stl);
 tolua_tablevar(tolua_S,"player_race","r_srh",toluaI_get_player_player_race_r_srh,toluaI_set_player_player_race_r_srh);
 tolua_tablevar(tolua_S,"player_race","r_fos",toluaI_get_player_player_race_r_fos,toluaI_set_player_player_race_r_fos);
 tolua_tablevar(tolua_S,"player_race","r_thn",toluaI_get_player_player_race_r_thn,toluaI_set_player_player_race_r_thn);
 tolua_tablevar(tolua_S,"player_race","r_thb",toluaI_get_player_player_race_r_thb,toluaI_set_player_player_race_r_thb);
 tolua_tablevar(tolua_S,"player_race","r_mhp",toluaI_get_player_player_race_r_mhp,toluaI_set_player_player_race_r_mhp);
 tolua_tablevar(tolua_S,"player_race","r_exp",toluaI_get_player_player_race_r_exp,toluaI_set_player_player_race_r_exp);
 tolua_tablevar(tolua_S,"player_race","b_age",toluaI_get_player_player_race_b_age,toluaI_set_player_player_race_b_age);
 tolua_tablevar(tolua_S,"player_race","m_age",toluaI_get_player_player_race_m_age,toluaI_set_player_player_race_m_age);
 tolua_tablevar(tolua_S,"player_race","m_b_ht",toluaI_get_player_player_race_m_b_ht,toluaI_set_player_player_race_m_b_ht);
 tolua_tablevar(tolua_S,"player_race","m_m_ht",toluaI_get_player_player_race_m_m_ht,toluaI_set_player_player_race_m_m_ht);
 tolua_tablevar(tolua_S,"player_race","m_b_wt",toluaI_get_player_player_race_m_b_wt,toluaI_set_player_player_race_m_b_wt);
 tolua_tablevar(tolua_S,"player_race","m_m_wt",toluaI_get_player_player_race_m_m_wt,toluaI_set_player_player_race_m_m_wt);
 tolua_tablevar(tolua_S,"player_race","f_b_ht",toluaI_get_player_player_race_f_b_ht,toluaI_set_player_player_race_f_b_ht);
 tolua_tablevar(tolua_S,"player_race","f_m_ht",toluaI_get_player_player_race_f_m_ht,toluaI_set_player_player_race_f_m_ht);
 tolua_tablevar(tolua_S,"player_race","f_b_wt",toluaI_get_player_player_race_f_b_wt,toluaI_set_player_player_race_f_b_wt);
 tolua_tablevar(tolua_S,"player_race","f_m_wt",toluaI_get_player_player_race_f_m_wt,toluaI_set_player_player_race_f_m_wt);
 tolua_tablevar(tolua_S,"player_race","infra",toluaI_get_player_player_race_infra,toluaI_set_player_player_race_infra);
 tolua_tablevar(tolua_S,"player_race","choice",toluaI_get_player_player_race_choice,toluaI_set_player_player_race_choice);
 tolua_tablevar(tolua_S,"player_race","hist",toluaI_get_player_player_race_hist,toluaI_set_player_player_race_hist);
 tolua_tablevar(tolua_S,"player_race","flags1",toluaI_get_player_player_race_flags1,toluaI_set_player_player_race_flags1);
 tolua_tablevar(tolua_S,"player_race","flags2",toluaI_get_player_player_race_flags2,toluaI_set_player_player_race_flags2);
 tolua_tablevar(tolua_S,"player_race","flags3",toluaI_get_player_player_race_flags3,toluaI_set_player_player_race_flags3);
 tolua_cclass(tolua_S,"start_item","");
 tolua_tablevar(tolua_S,"start_item","tval",toluaI_get_player_start_item_tval,toluaI_set_player_start_item_tval);
 tolua_tablevar(tolua_S,"start_item","sval",toluaI_get_player_start_item_sval,toluaI_set_player_start_item_sval);
 tolua_tablevar(tolua_S,"start_item","min",toluaI_get_player_start_item_min,toluaI_set_player_start_item_min);
 tolua_tablevar(tolua_S,"start_item","max",toluaI_get_player_start_item_max,toluaI_set_player_start_item_max);
 tolua_cclass(tolua_S,"player_class","");
 tolua_tablevar(tolua_S,"player_class","name",toluaI_get_player_player_class_name,toluaI_set_player_player_class_name);
 tolua_tablearray(tolua_S,"player_class","title",toluaI_get_player_player_class_title,toluaI_set_player_player_class_title);
 tolua_tablearray(tolua_S,"player_class","c_adj",toluaI_get_player_player_class_c_adj,toluaI_set_player_player_class_c_adj);
 tolua_tablevar(tolua_S,"player_class","c_dis",toluaI_get_player_player_class_c_dis,toluaI_set_player_player_class_c_dis);
 tolua_tablevar(tolua_S,"player_class","c_dev",toluaI_get_player_player_class_c_dev,toluaI_set_player_player_class_c_dev);
 tolua_tablevar(tolua_S,"player_class","c_sav",toluaI_get_player_player_class_c_sav,toluaI_set_player_player_class_c_sav);
 tolua_tablevar(tolua_S,"player_class","c_stl",toluaI_get_player_player_class_c_stl,toluaI_set_player_player_class_c_stl);
 tolua_tablevar(tolua_S,"player_class","c_srh",toluaI_get_player_player_class_c_srh,toluaI_set_player_player_class_c_srh);
 tolua_tablevar(tolua_S,"player_class","c_fos",toluaI_get_player_player_class_c_fos,toluaI_set_player_player_class_c_fos);
 tolua_tablevar(tolua_S,"player_class","c_thn",toluaI_get_player_player_class_c_thn,toluaI_set_player_player_class_c_thn);
 tolua_tablevar(tolua_S,"player_class","c_thb",toluaI_get_player_player_class_c_thb,toluaI_set_player_player_class_c_thb);
 tolua_tablevar(tolua_S,"player_class","x_dis",toluaI_get_player_player_class_x_dis,toluaI_set_player_player_class_x_dis);
 tolua_tablevar(tolua_S,"player_class","x_dev",toluaI_get_player_player_class_x_dev,toluaI_set_player_player_class_x_dev);
 tolua_tablevar(tolua_S,"player_class","x_sav",toluaI_get_player_player_class_x_sav,toluaI_set_player_player_class_x_sav);
 tolua_tablevar(tolua_S,"player_class","x_stl",toluaI_get_player_player_class_x_stl,toluaI_set_player_player_class_x_stl);
 tolua_tablevar(tolua_S,"player_class","x_srh",toluaI_get_player_player_class_x_srh,toluaI_set_player_player_class_x_srh);
 tolua_tablevar(tolua_S,"player_class","x_fos",toluaI_get_player_player_class_x_fos,toluaI_set_player_player_class_x_fos);
 tolua_tablevar(tolua_S,"player_class","x_thn",toluaI_get_player_player_class_x_thn,toluaI_set_player_player_class_x_thn);
 tolua_tablevar(tolua_S,"player_class","x_thb",toluaI_get_player_player_class_x_thb,toluaI_set_player_player_class_x_thb);
 tolua_tablevar(tolua_S,"player_class","c_mhp",toluaI_get_player_player_class_c_mhp,toluaI_set_player_player_class_c_mhp);
 tolua_tablevar(tolua_S,"player_class","c_exp",toluaI_get_player_player_class_c_exp,toluaI_set_player_player_class_c_exp);
 tolua_tablevar(tolua_S,"player_class","flags",toluaI_get_player_player_class_flags,toluaI_set_player_player_class_flags);
 tolua_tablevar(tolua_S,"player_class","max_attacks",toluaI_get_player_player_class_max_attacks,toluaI_set_player_player_class_max_attacks);
 tolua_tablevar(tolua_S,"player_class","min_weight",toluaI_get_player_player_class_min_weight,toluaI_set_player_player_class_min_weight);
 tolua_tablevar(tolua_S,"player_class","att_multiply",toluaI_get_player_player_class_att_multiply,toluaI_set_player_player_class_att_multiply);
 tolua_tablevar(tolua_S,"player_class","spell_book",toluaI_get_player_player_class_spell_book,toluaI_set_player_player_class_spell_book);
 tolua_tablevar(tolua_S,"player_class","spell_stat",toluaI_get_player_player_class_spell_stat,toluaI_set_player_player_class_spell_stat);
 tolua_tablevar(tolua_S,"player_class","spell_type",toluaI_get_player_player_class_spell_type,toluaI_set_player_player_class_spell_type);
 tolua_tablevar(tolua_S,"player_class","spell_first",toluaI_get_player_player_class_spell_first,toluaI_set_player_player_class_spell_first);
 tolua_tablevar(tolua_S,"player_class","spell_weight",toluaI_get_player_player_class_spell_weight,toluaI_set_player_player_class_spell_weight);
 tolua_tablevar(tolua_S,"player_class","sense_base",toluaI_get_player_player_class_sense_base,toluaI_set_player_player_class_sense_base);
 tolua_tablevar(tolua_S,"player_class","sense_div",toluaI_get_player_player_class_sense_div,toluaI_set_player_player_class_sense_div);
 tolua_tablearray(tolua_S,"player_class","start_items",toluaI_get_player_player_class_start_items,toluaI_set_player_player_class_start_items);
 tolua_tablevar(tolua_S,"player_class","spells",toluaI_get_player_player_class_spells,toluaI_set_player_player_class_spells);
 tolua_cclass(tolua_S,"hist_type","");
 tolua_tablevar(tolua_S,"hist_type","text",toluaI_get_player_hist_type_text,toluaI_set_player_hist_type_text);
 tolua_tablevar(tolua_S,"hist_type","roll",toluaI_get_player_hist_type_roll,toluaI_set_player_hist_type_roll);
 tolua_tablevar(tolua_S,"hist_type","chart",toluaI_get_player_hist_type_chart,toluaI_set_player_hist_type_chart);
 tolua_tablevar(tolua_S,"hist_type","next",toluaI_get_player_hist_type_next,toluaI_set_player_hist_type_next);
 tolua_tablevar(tolua_S,"hist_type","bonus",toluaI_get_player_hist_type_bonus,toluaI_set_player_hist_type_bonus);
 tolua_cclass(tolua_S,"player_other","");
 tolua_tablearray(tolua_S,"player_other","full_name",toluaI_get_player_player_other_full_name,toluaI_set_player_player_other_full_name);
 tolua_tablearray(tolua_S,"player_other","base_name",toluaI_get_player_player_other_base_name,toluaI_set_player_player_other_base_name);
 tolua_tablearray(tolua_S,"player_other","opt",toluaI_get_player_player_other_opt,toluaI_set_player_player_other_opt);
 tolua_tablearray(tolua_S,"player_other","window_flag",toluaI_get_player_player_other_window_flag,toluaI_set_player_player_other_window_flag);
 tolua_tablevar(tolua_S,"player_other","hitpoint_warn",toluaI_get_player_player_other_hitpoint_warn,toluaI_set_player_player_other_hitpoint_warn);
 tolua_tablevar(tolua_S,"player_other","delay_factor",toluaI_get_player_player_other_delay_factor,toluaI_set_player_player_other_delay_factor);
 tolua_globalvar(tolua_S,"sp_ptr",toluaI_get_player_sp_ptr,NULL);
 tolua_globalvar(tolua_S,"rp_ptr",toluaI_get_player_rp_ptr,NULL);
 tolua_globalvar(tolua_S,"cp_ptr",toluaI_get_player_cp_ptr,NULL);
 tolua_globalvar(tolua_S,"mp_ptr",toluaI_get_player_mp_ptr,NULL);
 tolua_globalvar(tolua_S,"op_ptr",toluaI_get_player_op_ptr,toluaI_set_player_op_ptr);
 tolua_globalvar(tolua_S,"player",toluaI_get_player_player,toluaI_set_player_player);
 tolua_function(tolua_S,NULL,"set_blind",toluaI_player_set_blind00);
 tolua_function(tolua_S,NULL,"set_confused",toluaI_player_set_confused00);
 tolua_function(tolua_S,NULL,"set_poisoned",toluaI_player_set_poisoned00);
 tolua_function(tolua_S,NULL,"set_afraid",toluaI_player_set_afraid00);
 tolua_function(tolua_S,NULL,"set_paralyzed",toluaI_player_set_paralyzed00);
 tolua_function(tolua_S,NULL,"set_image",toluaI_player_set_image00);
 tolua_function(tolua_S,NULL,"set_fast",toluaI_player_set_fast00);
 tolua_function(tolua_S,NULL,"set_slow",toluaI_player_set_slow00);
 tolua_function(tolua_S,NULL,"set_shield",toluaI_player_set_shield00);
 tolua_function(tolua_S,NULL,"set_blessed",toluaI_player_set_blessed00);
 tolua_function(tolua_S,NULL,"set_hero",toluaI_player_set_hero00);
 tolua_function(tolua_S,NULL,"set_shero",toluaI_player_set_shero00);
 tolua_function(tolua_S,NULL,"set_protevil",toluaI_player_set_protevil00);
 tolua_function(tolua_S,NULL,"set_invuln",toluaI_player_set_invuln00);
 tolua_function(tolua_S,NULL,"set_tim_invis",toluaI_player_set_tim_invis00);
 tolua_function(tolua_S,NULL,"set_tim_infra",toluaI_player_set_tim_infra00);
 tolua_function(tolua_S,NULL,"set_oppose_acid",toluaI_player_set_oppose_acid00);
 tolua_function(tolua_S,NULL,"set_oppose_elec",toluaI_player_set_oppose_elec00);
 tolua_function(tolua_S,NULL,"set_oppose_fire",toluaI_player_set_oppose_fire00);
 tolua_function(tolua_S,NULL,"set_oppose_cold",toluaI_player_set_oppose_cold00);
 tolua_function(tolua_S,NULL,"set_oppose_pois",toluaI_player_set_oppose_pois00);
 tolua_function(tolua_S,NULL,"set_stun",toluaI_player_set_stun00);
 tolua_function(tolua_S,NULL,"set_cut",toluaI_player_set_cut00);
 tolua_function(tolua_S,NULL,"set_food",toluaI_player_set_food00);
 tolua_function(tolua_S,NULL,"check_experience",toluaI_player_check_experience00);
 tolua_function(tolua_S,NULL,"gain_exp",toluaI_player_gain_exp00);
 tolua_function(tolua_S,NULL,"lose_exp",toluaI_player_lose_exp00);
 tolua_function(tolua_S,NULL,"hp_player",toluaI_player_hp_player00);
 tolua_function(tolua_S,NULL,"do_dec_stat",toluaI_player_do_dec_stat00);
 tolua_function(tolua_S,NULL,"do_res_stat",toluaI_player_do_res_stat00);
 tolua_function(tolua_S,NULL,"do_inc_stat",toluaI_player_do_inc_stat00);
 tolua_function(tolua_S,NULL,"take_hit",toluaI_player_take_hit00);
 tolua_function(tolua_S,NULL,"inc_stat",toluaI_player_inc_stat00);
 tolua_function(tolua_S,NULL,"dec_stat",toluaI_player_dec_stat00);
 tolua_function(tolua_S,NULL,"res_stat",toluaI_player_res_stat00);
 tolua_function(tolua_S,NULL,"restore_level",toluaI_player_restore_level00);
 return 1;
}
/* Close function */
void tolua_player_close (lua_State* tolua_S)
{
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"A_STR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"A_INT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"A_WIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"A_DEX");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"A_CON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"A_CHR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"A_MAX");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SEX_FEMALE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SEX_MALE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PY_MAX_EXP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PY_MAX_GOLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PY_MAX_LEVEL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PY_FOOD_MAX");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PY_FOOD_FULL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PY_FOOD_ALERT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PY_FOOD_WEAK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PY_FOOD_FAINT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PY_FOOD_STARVE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PY_REGEN_NORMAL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PY_REGEN_WEAK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PY_REGEN_FAINT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PY_REGEN_HPBASE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"PY_REGEN_MNBASE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"player_type");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"player_sex");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"player_race");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"start_item");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"player_class");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"hist_type");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"player_other");
 lua_getglobals(tolua_S);
 lua_pushstring(tolua_S,"sp_ptr"); lua_pushnil(tolua_S); lua_rawset(tolua_S,-3);
 lua_pop(tolua_S,1);
 lua_getglobals(tolua_S);
 lua_pushstring(tolua_S,"rp_ptr"); lua_pushnil(tolua_S); lua_rawset(tolua_S,-3);
 lua_pop(tolua_S,1);
 lua_getglobals(tolua_S);
 lua_pushstring(tolua_S,"cp_ptr"); lua_pushnil(tolua_S); lua_rawset(tolua_S,-3);
 lua_pop(tolua_S,1);
 lua_getglobals(tolua_S);
 lua_pushstring(tolua_S,"mp_ptr"); lua_pushnil(tolua_S); lua_rawset(tolua_S,-3);
 lua_pop(tolua_S,1);
 lua_getglobals(tolua_S);
 lua_pushstring(tolua_S,"op_ptr"); lua_pushnil(tolua_S); lua_rawset(tolua_S,-3);
 lua_pop(tolua_S,1);
 lua_getglobals(tolua_S);
 lua_pushstring(tolua_S,"player"); lua_pushnil(tolua_S); lua_rawset(tolua_S,-3);
 lua_pop(tolua_S,1);
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_blind");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_confused");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_poisoned");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_afraid");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_paralyzed");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_image");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_fast");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_slow");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_shield");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_blessed");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_hero");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_shero");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_protevil");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_invuln");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_tim_invis");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_tim_infra");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_oppose_acid");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_oppose_elec");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_oppose_fire");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_oppose_cold");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_oppose_pois");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_stun");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_cut");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"set_food");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"check_experience");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"gain_exp");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"lose_exp");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"hp_player");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"do_dec_stat");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"do_res_stat");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"do_inc_stat");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"take_hit");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"inc_stat");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"dec_stat");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"res_stat");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"restore_level");
}
