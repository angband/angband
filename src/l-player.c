/*
** Lua binding: player
** Generated automatically by tolua 5.0a on Sun May 23 19:11:30 2004.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "lua/tolua.h"

/* Exported function */
TOLUA_API int tolua_player_open (lua_State* tolua_S);

#include "angband.h"

/* function to release collected object via destructor */
#ifdef __cplusplus

static int tolua_collect_start_item (lua_State* tolua_S)
{
 start_item* self = (start_item*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}
#endif


/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"player_sex");
 tolua_usertype(tolua_S,"player_race");
 tolua_usertype(tolua_S,"player_class");
 tolua_usertype(tolua_S,"hist_type");
 tolua_usertype(tolua_S,"player_other");
 tolua_usertype(tolua_S,"start_item");
 tolua_usertype(tolua_S,"player_magic");
 tolua_usertype(tolua_S,"player_type");
}

/* get function: py of class  player_type */
static int tolua_get_player_type_py(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'py'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->py);
 return 1;
}

/* set function: py of class  player_type */
static int tolua_set_player_type_py(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'py'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->py = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: px of class  player_type */
static int tolua_get_player_type_px(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'px'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->px);
 return 1;
}

/* set function: px of class  player_type */
static int tolua_set_player_type_px(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'px'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->px = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: psex of class  player_type */
static int tolua_get_player_type_psex(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'psex'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->psex);
 return 1;
}

/* set function: psex of class  player_type */
static int tolua_set_player_type_psex(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'psex'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->psex = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: prace of class  player_type */
static int tolua_get_player_type_prace(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'prace'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->prace);
 return 1;
}

/* set function: prace of class  player_type */
static int tolua_set_player_type_prace(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'prace'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->prace = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: pclass of class  player_type */
static int tolua_get_player_type_pclass(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'pclass'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->pclass);
 return 1;
}

/* set function: pclass of class  player_type */
static int tolua_set_player_type_pclass(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'pclass'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->pclass = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: hitdie of class  player_type */
static int tolua_get_player_type_hitdie(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'hitdie'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->hitdie);
 return 1;
}

/* set function: hitdie of class  player_type */
static int tolua_set_player_type_hitdie(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'hitdie'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->hitdie = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: expfact of class  player_type */
static int tolua_get_player_type_expfact(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'expfact'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->expfact);
 return 1;
}

/* set function: expfact of class  player_type */
static int tolua_set_player_type_expfact(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'expfact'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->expfact = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: age of class  player_type */
static int tolua_get_player_type_age(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'age'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->age);
 return 1;
}

/* set function: age of class  player_type */
static int tolua_set_player_type_age(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'age'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->age = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: ht of class  player_type */
static int tolua_get_player_type_ht(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ht'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->ht);
 return 1;
}

/* set function: ht of class  player_type */
static int tolua_set_player_type_ht(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ht'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ht = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: wt of class  player_type */
static int tolua_get_player_type_wt(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'wt'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->wt);
 return 1;
}

/* set function: wt of class  player_type */
static int tolua_set_player_type_wt(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'wt'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->wt = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: sc of class  player_type */
static int tolua_get_player_type_sc(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sc'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->sc);
 return 1;
}

/* set function: sc of class  player_type */
static int tolua_set_player_type_sc(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sc'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->sc = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: au of class  player_type */
static int tolua_get_player_type_au(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'au'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->au);
 return 1;
}

/* set function: au of class  player_type */
static int tolua_set_player_type_au(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'au'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->au = ((s32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: max_depth of class  player_type */
static int tolua_get_player_type_max_depth(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'max_depth'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->max_depth);
 return 1;
}

/* set function: max_depth of class  player_type */
static int tolua_set_player_type_max_depth(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'max_depth'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->max_depth = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: depth of class  player_type */
static int tolua_get_player_type_depth(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'depth'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->depth);
 return 1;
}

/* set function: depth of class  player_type */
static int tolua_set_player_type_depth(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'depth'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->depth = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: max_lev of class  player_type */
static int tolua_get_player_type_max_lev(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'max_lev'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->max_lev);
 return 1;
}

/* set function: max_lev of class  player_type */
static int tolua_set_player_type_max_lev(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'max_lev'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->max_lev = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: lev of class  player_type */
static int tolua_get_player_type_lev(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'lev'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->lev);
 return 1;
}

/* set function: lev of class  player_type */
static int tolua_set_player_type_lev(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'lev'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->lev = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: max_exp of class  player_type */
static int tolua_get_player_type_max_exp(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'max_exp'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->max_exp);
 return 1;
}

/* set function: max_exp of class  player_type */
static int tolua_set_player_type_max_exp(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'max_exp'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->max_exp = ((s32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: exp of class  player_type */
static int tolua_get_player_type_exp(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'exp'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->exp);
 return 1;
}

/* set function: exp of class  player_type */
static int tolua_set_player_type_exp(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'exp'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->exp = ((s32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: exp_frac of class  player_type */
static int tolua_get_player_type_exp_frac(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'exp_frac'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->exp_frac);
 return 1;
}

/* set function: exp_frac of class  player_type */
static int tolua_set_player_type_exp_frac(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'exp_frac'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->exp_frac = ((u16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: mhp of class  player_type */
static int tolua_get_player_type_mhp(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'mhp'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->mhp);
 return 1;
}

/* set function: mhp of class  player_type */
static int tolua_set_player_type_mhp(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'mhp'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->mhp = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: chp of class  player_type */
static int tolua_get_player_type_chp(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'chp'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->chp);
 return 1;
}

/* set function: chp of class  player_type */
static int tolua_set_player_type_chp(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'chp'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->chp = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: chp_frac of class  player_type */
static int tolua_get_player_type_chp_frac(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'chp_frac'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->chp_frac);
 return 1;
}

/* set function: chp_frac of class  player_type */
static int tolua_set_player_type_chp_frac(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'chp_frac'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->chp_frac = ((u16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: msp of class  player_type */
static int tolua_get_player_type_msp(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'msp'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->msp);
 return 1;
}

/* set function: msp of class  player_type */
static int tolua_set_player_type_msp(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'msp'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->msp = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: csp of class  player_type */
static int tolua_get_player_type_csp(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'csp'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->csp);
 return 1;
}

/* set function: csp of class  player_type */
static int tolua_set_player_type_csp(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'csp'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->csp = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: csp_frac of class  player_type */
static int tolua_get_player_type_csp_frac(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'csp_frac'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->csp_frac);
 return 1;
}

/* set function: csp_frac of class  player_type */
static int tolua_set_player_type_csp_frac(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'csp_frac'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->csp_frac = ((u16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: stat_max of class  player_type */
static int tolua_get_player_player_type_stat_max(lua_State* tolua_S)
{
 int tolua_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->stat_max[tolua_index]);
 return 1;
}

/* set function: stat_max of class  player_type */
static int tolua_set_player_player_type_stat_max(lua_State* tolua_S)
{
 int tolua_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->stat_max[tolua_index] = ((s16b)  tolua_tonumber(tolua_S,3,0));
 return 0;
}

/* get function: stat_cur of class  player_type */
static int tolua_get_player_player_type_stat_cur(lua_State* tolua_S)
{
 int tolua_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->stat_cur[tolua_index]);
 return 1;
}

/* set function: stat_cur of class  player_type */
static int tolua_set_player_player_type_stat_cur(lua_State* tolua_S)
{
 int tolua_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->stat_cur[tolua_index] = ((s16b)  tolua_tonumber(tolua_S,3,0));
 return 0;
}

/* get function: fast of class  player_type */
static int tolua_get_player_type_fast(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'fast'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->fast);
 return 1;
}

/* set function: fast of class  player_type */
static int tolua_set_player_type_fast(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'fast'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->fast = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: slow of class  player_type */
static int tolua_get_player_type_slow(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'slow'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->slow);
 return 1;
}

/* set function: slow of class  player_type */
static int tolua_set_player_type_slow(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'slow'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->slow = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: blind of class  player_type */
static int tolua_get_player_type_blind(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'blind'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->blind);
 return 1;
}

/* set function: blind of class  player_type */
static int tolua_set_player_type_blind(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'blind'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->blind = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: paralyzed of class  player_type */
static int tolua_get_player_type_paralyzed(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'paralyzed'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->paralyzed);
 return 1;
}

/* set function: paralyzed of class  player_type */
static int tolua_set_player_type_paralyzed(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'paralyzed'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->paralyzed = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: confused of class  player_type */
static int tolua_get_player_type_confused(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'confused'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->confused);
 return 1;
}

/* set function: confused of class  player_type */
static int tolua_set_player_type_confused(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'confused'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->confused = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: afraid of class  player_type */
static int tolua_get_player_type_afraid(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'afraid'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->afraid);
 return 1;
}

/* set function: afraid of class  player_type */
static int tolua_set_player_type_afraid(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'afraid'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->afraid = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: image of class  player_type */
static int tolua_get_player_type_image(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'image'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->image);
 return 1;
}

/* set function: image of class  player_type */
static int tolua_set_player_type_image(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'image'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->image = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: poisoned of class  player_type */
static int tolua_get_player_type_poisoned(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'poisoned'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->poisoned);
 return 1;
}

/* set function: poisoned of class  player_type */
static int tolua_set_player_type_poisoned(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'poisoned'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->poisoned = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: cut of class  player_type */
static int tolua_get_player_type_cut(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cut'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->cut);
 return 1;
}

/* set function: cut of class  player_type */
static int tolua_set_player_type_cut(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cut'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->cut = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: stun of class  player_type */
static int tolua_get_player_type_stun(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'stun'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->stun);
 return 1;
}

/* set function: stun of class  player_type */
static int tolua_set_player_type_stun(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'stun'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->stun = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: protevil of class  player_type */
static int tolua_get_player_type_protevil(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'protevil'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->protevil);
 return 1;
}

/* set function: protevil of class  player_type */
static int tolua_set_player_type_protevil(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'protevil'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->protevil = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: invuln of class  player_type */
static int tolua_get_player_type_invuln(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'invuln'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->invuln);
 return 1;
}

/* set function: invuln of class  player_type */
static int tolua_set_player_type_invuln(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'invuln'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->invuln = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: hero of class  player_type */
static int tolua_get_player_type_hero(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'hero'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->hero);
 return 1;
}

/* set function: hero of class  player_type */
static int tolua_set_player_type_hero(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'hero'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->hero = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: shero of class  player_type */
static int tolua_get_player_type_shero(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'shero'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->shero);
 return 1;
}

/* set function: shero of class  player_type */
static int tolua_set_player_type_shero(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'shero'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->shero = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: shield of class  player_type */
static int tolua_get_player_type_shield(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'shield'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->shield);
 return 1;
}

/* set function: shield of class  player_type */
static int tolua_set_player_type_shield(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'shield'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->shield = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: blessed of class  player_type */
static int tolua_get_player_type_blessed(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'blessed'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->blessed);
 return 1;
}

/* set function: blessed of class  player_type */
static int tolua_set_player_type_blessed(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'blessed'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->blessed = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: tim_invis of class  player_type */
static int tolua_get_player_type_tim_invis(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'tim_invis'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->tim_invis);
 return 1;
}

/* set function: tim_invis of class  player_type */
static int tolua_set_player_type_tim_invis(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'tim_invis'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->tim_invis = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: tim_infra of class  player_type */
static int tolua_get_player_type_tim_infra(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'tim_infra'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->tim_infra);
 return 1;
}

/* set function: tim_infra of class  player_type */
static int tolua_set_player_type_tim_infra(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'tim_infra'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->tim_infra = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: oppose_acid of class  player_type */
static int tolua_get_player_type_oppose_acid(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'oppose_acid'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->oppose_acid);
 return 1;
}

/* set function: oppose_acid of class  player_type */
static int tolua_set_player_type_oppose_acid(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'oppose_acid'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->oppose_acid = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: oppose_elec of class  player_type */
static int tolua_get_player_type_oppose_elec(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'oppose_elec'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->oppose_elec);
 return 1;
}

/* set function: oppose_elec of class  player_type */
static int tolua_set_player_type_oppose_elec(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'oppose_elec'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->oppose_elec = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: oppose_fire of class  player_type */
static int tolua_get_player_type_oppose_fire(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'oppose_fire'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->oppose_fire);
 return 1;
}

/* set function: oppose_fire of class  player_type */
static int tolua_set_player_type_oppose_fire(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'oppose_fire'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->oppose_fire = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: oppose_cold of class  player_type */
static int tolua_get_player_type_oppose_cold(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'oppose_cold'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->oppose_cold);
 return 1;
}

/* set function: oppose_cold of class  player_type */
static int tolua_set_player_type_oppose_cold(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'oppose_cold'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->oppose_cold = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: oppose_pois of class  player_type */
static int tolua_get_player_type_oppose_pois(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'oppose_pois'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->oppose_pois);
 return 1;
}

/* set function: oppose_pois of class  player_type */
static int tolua_set_player_type_oppose_pois(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'oppose_pois'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->oppose_pois = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: word_recall of class  player_type */
static int tolua_get_player_type_word_recall(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'word_recall'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->word_recall);
 return 1;
}

/* set function: word_recall of class  player_type */
static int tolua_set_player_type_word_recall(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'word_recall'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->word_recall = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: energy of class  player_type */
static int tolua_get_player_type_energy(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'energy'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->energy);
 return 1;
}

/* set function: energy of class  player_type */
static int tolua_set_player_type_energy(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'energy'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->energy = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: food of class  player_type */
static int tolua_get_player_type_food(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'food'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->food);
 return 1;
}

/* set function: food of class  player_type */
static int tolua_set_player_type_food(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'food'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->food = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: confusing of class  player_type */
static int tolua_get_player_type_confusing(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'confusing'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->confusing);
 return 1;
}

/* set function: confusing of class  player_type */
static int tolua_set_player_type_confusing(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'confusing'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->confusing = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: searching of class  player_type */
static int tolua_get_player_type_searching(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'searching'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->searching);
 return 1;
}

/* set function: searching of class  player_type */
static int tolua_set_player_type_searching(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'searching'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->searching = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: spell_flags of class  player_type */
static int tolua_get_player_player_type_spell_flags(lua_State* tolua_S)
{
 int tolua_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=PY_MAX_SPELLS)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->spell_flags[tolua_index]);
 return 1;
}

/* set function: spell_flags of class  player_type */
static int tolua_set_player_player_type_spell_flags(lua_State* tolua_S)
{
 int tolua_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=PY_MAX_SPELLS)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->spell_flags[tolua_index] = ((byte)  tolua_tonumber(tolua_S,3,0));
 return 0;
}

/* get function: spell_order of class  player_type */
static int tolua_get_player_player_type_spell_order(lua_State* tolua_S)
{
 int tolua_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=PY_MAX_SPELLS)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->spell_order[tolua_index]);
 return 1;
}

/* set function: spell_order of class  player_type */
static int tolua_set_player_player_type_spell_order(lua_State* tolua_S)
{
 int tolua_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=PY_MAX_SPELLS)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->spell_order[tolua_index] = ((byte)  tolua_tonumber(tolua_S,3,0));
 return 0;
}

/* get function: player_hp of class  player_type */
static int tolua_get_player_player_type_player_hp(lua_State* tolua_S)
{
 int tolua_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=PY_MAX_LEVEL)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->player_hp[tolua_index]);
 return 1;
}

/* set function: player_hp of class  player_type */
static int tolua_set_player_player_type_player_hp(lua_State* tolua_S)
{
 int tolua_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=PY_MAX_LEVEL)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->player_hp[tolua_index] = ((s16b)  tolua_tonumber(tolua_S,3,0));
 return 0;
}

/* get function: died_from of class  player_type */
static int tolua_get_player_type_died_from(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'died_from'",NULL);
#endif
 tolua_pushstring(tolua_S,(const char*)self->died_from);
 return 1;
}

/* set function: died_from of class  player_type */
static int tolua_set_player_type_died_from(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'died_from'",NULL);
 if (!tolua_isstring(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
 strncpy(self->died_from,tolua_tostring(tolua_S,2,0),80-1);
 return 0;
}

/* get function: total_winner of class  player_type */
static int tolua_get_player_type_total_winner(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'total_winner'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->total_winner);
 return 1;
}

/* set function: total_winner of class  player_type */
static int tolua_set_player_type_total_winner(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'total_winner'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->total_winner = ((u16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: panic_save of class  player_type */
static int tolua_get_player_type_panic_save(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'panic_save'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->panic_save);
 return 1;
}

/* set function: panic_save of class  player_type */
static int tolua_set_player_type_panic_save(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'panic_save'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->panic_save = ((u16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: noscore of class  player_type */
static int tolua_get_player_type_noscore(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'noscore'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->noscore);
 return 1;
}

/* set function: noscore of class  player_type */
static int tolua_set_player_type_noscore(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'noscore'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->noscore = ((u16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: is_dead of class  player_type */
static int tolua_get_player_type_is_dead(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'is_dead'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->is_dead);
 return 1;
}

/* set function: is_dead of class  player_type */
static int tolua_set_player_type_is_dead(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'is_dead'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->is_dead = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: wizard of class  player_type */
static int tolua_get_player_type_wizard(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'wizard'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->wizard);
 return 1;
}

/* set function: wizard of class  player_type */
static int tolua_set_player_type_wizard(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'wizard'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->wizard = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: playing of class  player_type */
static int tolua_get_player_type_playing(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'playing'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->playing);
 return 1;
}

/* set function: playing of class  player_type */
static int tolua_set_player_type_playing(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'playing'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->playing = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: leaving of class  player_type */
static int tolua_get_player_type_leaving(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'leaving'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->leaving);
 return 1;
}

/* set function: leaving of class  player_type */
static int tolua_set_player_type_leaving(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'leaving'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->leaving = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: create_up_stair of class  player_type */
static int tolua_get_player_type_create_up_stair(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'create_up_stair'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->create_up_stair);
 return 1;
}

/* set function: create_up_stair of class  player_type */
static int tolua_set_player_type_create_up_stair(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'create_up_stair'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->create_up_stair = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: create_down_stair of class  player_type */
static int tolua_get_player_type_create_down_stair(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'create_down_stair'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->create_down_stair);
 return 1;
}

/* set function: create_down_stair of class  player_type */
static int tolua_set_player_type_create_down_stair(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'create_down_stair'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->create_down_stair = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: wy of class  player_type */
static int tolua_get_player_type_wy(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'wy'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->wy);
 return 1;
}

/* set function: wy of class  player_type */
static int tolua_set_player_type_wy(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'wy'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->wy = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: wx of class  player_type */
static int tolua_get_player_type_wx(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'wx'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->wx);
 return 1;
}

/* set function: wx of class  player_type */
static int tolua_set_player_type_wx(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'wx'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->wx = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: total_weight of class  player_type */
static int tolua_get_player_type_total_weight(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'total_weight'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->total_weight);
 return 1;
}

/* set function: total_weight of class  player_type */
static int tolua_set_player_type_total_weight(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'total_weight'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->total_weight = ((s32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: inven_cnt of class  player_type */
static int tolua_get_player_type_inven_cnt(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'inven_cnt'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->inven_cnt);
 return 1;
}

/* set function: inven_cnt of class  player_type */
static int tolua_set_player_type_inven_cnt(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'inven_cnt'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->inven_cnt = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: equip_cnt of class  player_type */
static int tolua_get_player_type_equip_cnt(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'equip_cnt'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->equip_cnt);
 return 1;
}

/* set function: equip_cnt of class  player_type */
static int tolua_set_player_type_equip_cnt(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'equip_cnt'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->equip_cnt = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: target_set of class  player_type */
static int tolua_get_player_type_target_set(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'target_set'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->target_set);
 return 1;
}

/* set function: target_set of class  player_type */
static int tolua_set_player_type_target_set(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'target_set'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->target_set = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: target_who of class  player_type */
static int tolua_get_player_type_target_who(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'target_who'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->target_who);
 return 1;
}

/* set function: target_who of class  player_type */
static int tolua_set_player_type_target_who(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'target_who'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->target_who = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: target_row of class  player_type */
static int tolua_get_player_type_target_row(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'target_row'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->target_row);
 return 1;
}

/* set function: target_row of class  player_type */
static int tolua_set_player_type_target_row(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'target_row'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->target_row = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: target_col of class  player_type */
static int tolua_get_player_type_target_col(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'target_col'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->target_col);
 return 1;
}

/* set function: target_col of class  player_type */
static int tolua_set_player_type_target_col(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'target_col'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->target_col = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: health_who of class  player_type */
static int tolua_get_player_type_health_who(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'health_who'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->health_who);
 return 1;
}

/* set function: health_who of class  player_type */
static int tolua_set_player_type_health_who(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'health_who'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->health_who = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: monster_race_idx of class  player_type */
static int tolua_get_player_type_monster_race_idx(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'monster_race_idx'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->monster_race_idx);
 return 1;
}

/* set function: monster_race_idx of class  player_type */
static int tolua_set_player_type_monster_race_idx(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'monster_race_idx'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->monster_race_idx = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: object_kind_idx of class  player_type */
static int tolua_get_player_type_object_kind_idx(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'object_kind_idx'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->object_kind_idx);
 return 1;
}

/* set function: object_kind_idx of class  player_type */
static int tolua_set_player_type_object_kind_idx(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'object_kind_idx'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->object_kind_idx = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: energy_use of class  player_type */
static int tolua_get_player_type_energy_use(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'energy_use'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->energy_use);
 return 1;
}

/* set function: energy_use of class  player_type */
static int tolua_set_player_type_energy_use(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'energy_use'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->energy_use = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: resting of class  player_type */
static int tolua_get_player_type_resting(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resting'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->resting);
 return 1;
}

/* set function: resting of class  player_type */
static int tolua_set_player_type_resting(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resting'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->resting = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: running of class  player_type */
static int tolua_get_player_type_running(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'running'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->running);
 return 1;
}

/* set function: running of class  player_type */
static int tolua_set_player_type_running(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'running'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->running = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: run_cur_dir of class  player_type */
static int tolua_get_player_type_run_cur_dir(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'run_cur_dir'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->run_cur_dir);
 return 1;
}

/* set function: run_cur_dir of class  player_type */
static int tolua_set_player_type_run_cur_dir(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'run_cur_dir'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->run_cur_dir = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: run_old_dir of class  player_type */
static int tolua_get_player_type_run_old_dir(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'run_old_dir'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->run_old_dir);
 return 1;
}

/* set function: run_old_dir of class  player_type */
static int tolua_set_player_type_run_old_dir(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'run_old_dir'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->run_old_dir = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: run_unused of class  player_type */
static int tolua_get_player_type_run_unused(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'run_unused'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->run_unused);
 return 1;
}

/* set function: run_unused of class  player_type */
static int tolua_set_player_type_run_unused(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'run_unused'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->run_unused = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: run_open_area of class  player_type */
static int tolua_get_player_type_run_open_area(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'run_open_area'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->run_open_area);
 return 1;
}

/* set function: run_open_area of class  player_type */
static int tolua_set_player_type_run_open_area(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'run_open_area'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->run_open_area = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: run_break_right of class  player_type */
static int tolua_get_player_type_run_break_right(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'run_break_right'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->run_break_right);
 return 1;
}

/* set function: run_break_right of class  player_type */
static int tolua_set_player_type_run_break_right(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'run_break_right'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->run_break_right = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: run_break_left of class  player_type */
static int tolua_get_player_type_run_break_left(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'run_break_left'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->run_break_left);
 return 1;
}

/* set function: run_break_left of class  player_type */
static int tolua_set_player_type_run_break_left(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'run_break_left'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->run_break_left = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: command_cmd of class  player_type */
static int tolua_get_player_type_command_cmd(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'command_cmd'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->command_cmd);
 return 1;
}

/* set function: command_cmd of class  player_type */
static int tolua_set_player_type_command_cmd(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'command_cmd'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->command_cmd = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: command_arg of class  player_type */
static int tolua_get_player_type_command_arg(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'command_arg'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->command_arg);
 return 1;
}

/* set function: command_arg of class  player_type */
static int tolua_set_player_type_command_arg(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'command_arg'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->command_arg = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: command_rep of class  player_type */
static int tolua_get_player_type_command_rep(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'command_rep'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->command_rep);
 return 1;
}

/* set function: command_rep of class  player_type */
static int tolua_set_player_type_command_rep(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'command_rep'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->command_rep = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: command_dir of class  player_type */
static int tolua_get_player_type_command_dir(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'command_dir'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->command_dir);
 return 1;
}

/* set function: command_dir of class  player_type */
static int tolua_set_player_type_command_dir(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'command_dir'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->command_dir = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: command_see of class  player_type */
static int tolua_get_player_type_command_see(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'command_see'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->command_see);
 return 1;
}

/* set function: command_see of class  player_type */
static int tolua_set_player_type_command_see(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'command_see'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->command_see = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: command_wrk of class  player_type */
static int tolua_get_player_type_command_wrk(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'command_wrk'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->command_wrk);
 return 1;
}

/* set function: command_wrk of class  player_type */
static int tolua_set_player_type_command_wrk(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'command_wrk'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->command_wrk = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: command_new of class  player_type */
static int tolua_get_player_type_command_new(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'command_new'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->command_new);
 return 1;
}

/* set function: command_new of class  player_type */
static int tolua_set_player_type_command_new(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'command_new'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->command_new = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: new_spells of class  player_type */
static int tolua_get_player_type_new_spells(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'new_spells'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->new_spells);
 return 1;
}

/* set function: new_spells of class  player_type */
static int tolua_set_player_type_new_spells(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'new_spells'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->new_spells = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: cumber_armor of class  player_type */
static int tolua_get_player_type_cumber_armor(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cumber_armor'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->cumber_armor);
 return 1;
}

/* set function: cumber_armor of class  player_type */
static int tolua_set_player_type_cumber_armor(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cumber_armor'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->cumber_armor = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: cumber_glove of class  player_type */
static int tolua_get_player_type_cumber_glove(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cumber_glove'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->cumber_glove);
 return 1;
}

/* set function: cumber_glove of class  player_type */
static int tolua_set_player_type_cumber_glove(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cumber_glove'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->cumber_glove = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: heavy_wield of class  player_type */
static int tolua_get_player_type_heavy_wield(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'heavy_wield'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->heavy_wield);
 return 1;
}

/* set function: heavy_wield of class  player_type */
static int tolua_set_player_type_heavy_wield(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'heavy_wield'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->heavy_wield = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: heavy_shoot of class  player_type */
static int tolua_get_player_type_heavy_shoot(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'heavy_shoot'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->heavy_shoot);
 return 1;
}

/* set function: heavy_shoot of class  player_type */
static int tolua_set_player_type_heavy_shoot(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'heavy_shoot'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->heavy_shoot = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: icky_wield of class  player_type */
static int tolua_get_player_type_icky_wield(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'icky_wield'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->icky_wield);
 return 1;
}

/* set function: icky_wield of class  player_type */
static int tolua_set_player_type_icky_wield(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'icky_wield'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->icky_wield = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: cur_lite of class  player_type */
static int tolua_get_player_type_cur_lite(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cur_lite'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->cur_lite);
 return 1;
}

/* set function: cur_lite of class  player_type */
static int tolua_set_player_type_cur_lite(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cur_lite'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->cur_lite = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: notice of class  player_type */
static int tolua_get_player_type_notice(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'notice'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->notice);
 return 1;
}

/* set function: notice of class  player_type */
static int tolua_set_player_type_notice(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'notice'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->notice = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: update of class  player_type */
static int tolua_get_player_type_update(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'update'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->update);
 return 1;
}

/* set function: update of class  player_type */
static int tolua_set_player_type_update(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'update'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->update = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: redraw of class  player_type */
static int tolua_get_player_type_redraw(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'redraw'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->redraw);
 return 1;
}

/* set function: redraw of class  player_type */
static int tolua_set_player_type_redraw(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'redraw'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->redraw = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: window of class  player_type */
static int tolua_get_player_type_window(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'window'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->window);
 return 1;
}

/* set function: window of class  player_type */
static int tolua_set_player_type_window(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'window'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->window = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: stat_use of class  player_type */
static int tolua_get_player_player_type_stat_use(lua_State* tolua_S)
{
 int tolua_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->stat_use[tolua_index]);
 return 1;
}

/* set function: stat_use of class  player_type */
static int tolua_set_player_player_type_stat_use(lua_State* tolua_S)
{
 int tolua_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->stat_use[tolua_index] = ((s16b)  tolua_tonumber(tolua_S,3,0));
 return 0;
}

/* get function: stat_top of class  player_type */
static int tolua_get_player_player_type_stat_top(lua_State* tolua_S)
{
 int tolua_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->stat_top[tolua_index]);
 return 1;
}

/* set function: stat_top of class  player_type */
static int tolua_set_player_player_type_stat_top(lua_State* tolua_S)
{
 int tolua_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->stat_top[tolua_index] = ((s16b)  tolua_tonumber(tolua_S,3,0));
 return 0;
}

/* get function: stat_add of class  player_type */
static int tolua_get_player_player_type_stat_add(lua_State* tolua_S)
{
 int tolua_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->stat_add[tolua_index]);
 return 1;
}

/* set function: stat_add of class  player_type */
static int tolua_set_player_player_type_stat_add(lua_State* tolua_S)
{
 int tolua_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->stat_add[tolua_index] = ((s16b)  tolua_tonumber(tolua_S,3,0));
 return 0;
}

/* get function: stat_ind of class  player_type */
static int tolua_get_player_player_type_stat_ind(lua_State* tolua_S)
{
 int tolua_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->stat_ind[tolua_index]);
 return 1;
}

/* set function: stat_ind of class  player_type */
static int tolua_set_player_player_type_stat_ind(lua_State* tolua_S)
{
 int tolua_index;
  player_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_type*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->stat_ind[tolua_index] = ((s16b)  tolua_tonumber(tolua_S,3,0));
 return 0;
}

/* get function: immune_acid of class  player_type */
static int tolua_get_player_type_immune_acid(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'immune_acid'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->immune_acid);
 return 1;
}

/* set function: immune_acid of class  player_type */
static int tolua_set_player_type_immune_acid(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'immune_acid'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->immune_acid = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: immune_elec of class  player_type */
static int tolua_get_player_type_immune_elec(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'immune_elec'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->immune_elec);
 return 1;
}

/* set function: immune_elec of class  player_type */
static int tolua_set_player_type_immune_elec(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'immune_elec'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->immune_elec = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: immune_fire of class  player_type */
static int tolua_get_player_type_immune_fire(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'immune_fire'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->immune_fire);
 return 1;
}

/* set function: immune_fire of class  player_type */
static int tolua_set_player_type_immune_fire(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'immune_fire'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->immune_fire = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: immune_cold of class  player_type */
static int tolua_get_player_type_immune_cold(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'immune_cold'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->immune_cold);
 return 1;
}

/* set function: immune_cold of class  player_type */
static int tolua_set_player_type_immune_cold(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'immune_cold'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->immune_cold = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: resist_acid of class  player_type */
static int tolua_get_player_type_resist_acid(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_acid'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->resist_acid);
 return 1;
}

/* set function: resist_acid of class  player_type */
static int tolua_set_player_type_resist_acid(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_acid'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->resist_acid = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: resist_elec of class  player_type */
static int tolua_get_player_type_resist_elec(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_elec'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->resist_elec);
 return 1;
}

/* set function: resist_elec of class  player_type */
static int tolua_set_player_type_resist_elec(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_elec'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->resist_elec = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: resist_fire of class  player_type */
static int tolua_get_player_type_resist_fire(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_fire'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->resist_fire);
 return 1;
}

/* set function: resist_fire of class  player_type */
static int tolua_set_player_type_resist_fire(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_fire'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->resist_fire = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: resist_cold of class  player_type */
static int tolua_get_player_type_resist_cold(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_cold'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->resist_cold);
 return 1;
}

/* set function: resist_cold of class  player_type */
static int tolua_set_player_type_resist_cold(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_cold'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->resist_cold = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: resist_pois of class  player_type */
static int tolua_get_player_type_resist_pois(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_pois'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->resist_pois);
 return 1;
}

/* set function: resist_pois of class  player_type */
static int tolua_set_player_type_resist_pois(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_pois'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->resist_pois = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: resist_fear of class  player_type */
static int tolua_get_player_type_resist_fear(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_fear'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->resist_fear);
 return 1;
}

/* set function: resist_fear of class  player_type */
static int tolua_set_player_type_resist_fear(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_fear'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->resist_fear = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: resist_lite of class  player_type */
static int tolua_get_player_type_resist_lite(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_lite'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->resist_lite);
 return 1;
}

/* set function: resist_lite of class  player_type */
static int tolua_set_player_type_resist_lite(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_lite'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->resist_lite = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: resist_dark of class  player_type */
static int tolua_get_player_type_resist_dark(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_dark'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->resist_dark);
 return 1;
}

/* set function: resist_dark of class  player_type */
static int tolua_set_player_type_resist_dark(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_dark'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->resist_dark = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: resist_blind of class  player_type */
static int tolua_get_player_type_resist_blind(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_blind'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->resist_blind);
 return 1;
}

/* set function: resist_blind of class  player_type */
static int tolua_set_player_type_resist_blind(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_blind'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->resist_blind = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: resist_confu of class  player_type */
static int tolua_get_player_type_resist_confu(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_confu'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->resist_confu);
 return 1;
}

/* set function: resist_confu of class  player_type */
static int tolua_set_player_type_resist_confu(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_confu'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->resist_confu = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: resist_sound of class  player_type */
static int tolua_get_player_type_resist_sound(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_sound'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->resist_sound);
 return 1;
}

/* set function: resist_sound of class  player_type */
static int tolua_set_player_type_resist_sound(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_sound'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->resist_sound = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: resist_shard of class  player_type */
static int tolua_get_player_type_resist_shard(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_shard'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->resist_shard);
 return 1;
}

/* set function: resist_shard of class  player_type */
static int tolua_set_player_type_resist_shard(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_shard'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->resist_shard = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: resist_nexus of class  player_type */
static int tolua_get_player_type_resist_nexus(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_nexus'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->resist_nexus);
 return 1;
}

/* set function: resist_nexus of class  player_type */
static int tolua_set_player_type_resist_nexus(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_nexus'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->resist_nexus = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: resist_nethr of class  player_type */
static int tolua_get_player_type_resist_nethr(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_nethr'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->resist_nethr);
 return 1;
}

/* set function: resist_nethr of class  player_type */
static int tolua_set_player_type_resist_nethr(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_nethr'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->resist_nethr = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: resist_chaos of class  player_type */
static int tolua_get_player_type_resist_chaos(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_chaos'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->resist_chaos);
 return 1;
}

/* set function: resist_chaos of class  player_type */
static int tolua_set_player_type_resist_chaos(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_chaos'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->resist_chaos = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: resist_disen of class  player_type */
static int tolua_get_player_type_resist_disen(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_disen'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->resist_disen);
 return 1;
}

/* set function: resist_disen of class  player_type */
static int tolua_set_player_type_resist_disen(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'resist_disen'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->resist_disen = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: sustain_str of class  player_type */
static int tolua_get_player_type_sustain_str(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sustain_str'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->sustain_str);
 return 1;
}

/* set function: sustain_str of class  player_type */
static int tolua_set_player_type_sustain_str(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sustain_str'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->sustain_str = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: sustain_int of class  player_type */
static int tolua_get_player_type_sustain_int(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sustain_int'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->sustain_int);
 return 1;
}

/* set function: sustain_int of class  player_type */
static int tolua_set_player_type_sustain_int(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sustain_int'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->sustain_int = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: sustain_wis of class  player_type */
static int tolua_get_player_type_sustain_wis(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sustain_wis'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->sustain_wis);
 return 1;
}

/* set function: sustain_wis of class  player_type */
static int tolua_set_player_type_sustain_wis(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sustain_wis'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->sustain_wis = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: sustain_dex of class  player_type */
static int tolua_get_player_type_sustain_dex(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sustain_dex'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->sustain_dex);
 return 1;
}

/* set function: sustain_dex of class  player_type */
static int tolua_set_player_type_sustain_dex(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sustain_dex'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->sustain_dex = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: sustain_con of class  player_type */
static int tolua_get_player_type_sustain_con(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sustain_con'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->sustain_con);
 return 1;
}

/* set function: sustain_con of class  player_type */
static int tolua_set_player_type_sustain_con(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sustain_con'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->sustain_con = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: sustain_chr of class  player_type */
static int tolua_get_player_type_sustain_chr(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sustain_chr'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->sustain_chr);
 return 1;
}

/* set function: sustain_chr of class  player_type */
static int tolua_set_player_type_sustain_chr(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sustain_chr'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->sustain_chr = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: slow_digest of class  player_type */
static int tolua_get_player_type_slow_digest(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'slow_digest'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->slow_digest);
 return 1;
}

/* set function: slow_digest of class  player_type */
static int tolua_set_player_type_slow_digest(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'slow_digest'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->slow_digest = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: ffall of class  player_type */
static int tolua_get_player_type_ffall(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ffall'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->ffall);
 return 1;
}

/* set function: ffall of class  player_type */
static int tolua_set_player_type_ffall(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ffall'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ffall = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: lite of class  player_type */
static int tolua_get_player_type_lite(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'lite'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->lite);
 return 1;
}

/* set function: lite of class  player_type */
static int tolua_set_player_type_lite(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'lite'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->lite = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: regenerate of class  player_type */
static int tolua_get_player_type_regenerate(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'regenerate'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->regenerate);
 return 1;
}

/* set function: regenerate of class  player_type */
static int tolua_set_player_type_regenerate(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'regenerate'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->regenerate = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: telepathy of class  player_type */
static int tolua_get_player_type_telepathy(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'telepathy'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->telepathy);
 return 1;
}

/* set function: telepathy of class  player_type */
static int tolua_set_player_type_telepathy(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'telepathy'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->telepathy = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: see_inv of class  player_type */
static int tolua_get_player_type_see_inv(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'see_inv'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->see_inv);
 return 1;
}

/* set function: see_inv of class  player_type */
static int tolua_set_player_type_see_inv(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'see_inv'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->see_inv = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: free_act of class  player_type */
static int tolua_get_player_type_free_act(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'free_act'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->free_act);
 return 1;
}

/* set function: free_act of class  player_type */
static int tolua_set_player_type_free_act(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'free_act'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->free_act = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: hold_life of class  player_type */
static int tolua_get_player_type_hold_life(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'hold_life'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->hold_life);
 return 1;
}

/* set function: hold_life of class  player_type */
static int tolua_set_player_type_hold_life(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'hold_life'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->hold_life = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: impact of class  player_type */
static int tolua_get_player_type_impact(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'impact'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->impact);
 return 1;
}

/* set function: impact of class  player_type */
static int tolua_set_player_type_impact(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'impact'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->impact = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: aggravate of class  player_type */
static int tolua_get_player_type_aggravate(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'aggravate'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->aggravate);
 return 1;
}

/* set function: aggravate of class  player_type */
static int tolua_set_player_type_aggravate(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'aggravate'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->aggravate = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: teleport of class  player_type */
static int tolua_get_player_type_teleport(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'teleport'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->teleport);
 return 1;
}

/* set function: teleport of class  player_type */
static int tolua_set_player_type_teleport(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'teleport'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->teleport = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: exp_drain of class  player_type */
static int tolua_get_player_type_exp_drain(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'exp_drain'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->exp_drain);
 return 1;
}

/* set function: exp_drain of class  player_type */
static int tolua_set_player_type_exp_drain(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'exp_drain'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->exp_drain = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: bless_blade of class  player_type */
static int tolua_get_player_type_bless_blade(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'bless_blade'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->bless_blade);
 return 1;
}

/* set function: bless_blade of class  player_type */
static int tolua_set_player_type_bless_blade(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'bless_blade'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->bless_blade = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: dis_to_h of class  player_type */
static int tolua_get_player_type_dis_to_h(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'dis_to_h'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->dis_to_h);
 return 1;
}

/* set function: dis_to_h of class  player_type */
static int tolua_set_player_type_dis_to_h(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'dis_to_h'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->dis_to_h = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: dis_to_d of class  player_type */
static int tolua_get_player_type_dis_to_d(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'dis_to_d'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->dis_to_d);
 return 1;
}

/* set function: dis_to_d of class  player_type */
static int tolua_set_player_type_dis_to_d(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'dis_to_d'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->dis_to_d = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: dis_to_a of class  player_type */
static int tolua_get_player_type_dis_to_a(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'dis_to_a'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->dis_to_a);
 return 1;
}

/* set function: dis_to_a of class  player_type */
static int tolua_set_player_type_dis_to_a(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'dis_to_a'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->dis_to_a = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: dis_ac of class  player_type */
static int tolua_get_player_type_dis_ac(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'dis_ac'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->dis_ac);
 return 1;
}

/* set function: dis_ac of class  player_type */
static int tolua_set_player_type_dis_ac(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'dis_ac'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->dis_ac = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: to_h of class  player_type */
static int tolua_get_player_type_to_h(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'to_h'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->to_h);
 return 1;
}

/* set function: to_h of class  player_type */
static int tolua_set_player_type_to_h(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'to_h'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->to_h = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: to_d of class  player_type */
static int tolua_get_player_type_to_d(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'to_d'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->to_d);
 return 1;
}

/* set function: to_d of class  player_type */
static int tolua_set_player_type_to_d(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'to_d'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->to_d = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: to_a of class  player_type */
static int tolua_get_player_type_to_a(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'to_a'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->to_a);
 return 1;
}

/* set function: to_a of class  player_type */
static int tolua_set_player_type_to_a(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'to_a'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->to_a = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: ac of class  player_type */
static int tolua_get_player_type_ac(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ac'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->ac);
 return 1;
}

/* set function: ac of class  player_type */
static int tolua_set_player_type_ac(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ac'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ac = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: see_infra of class  player_type */
static int tolua_get_player_type_see_infra(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'see_infra'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->see_infra);
 return 1;
}

/* set function: see_infra of class  player_type */
static int tolua_set_player_type_see_infra(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'see_infra'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->see_infra = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: skill_dis of class  player_type */
static int tolua_get_player_type_skill_dis(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'skill_dis'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->skill_dis);
 return 1;
}

/* set function: skill_dis of class  player_type */
static int tolua_set_player_type_skill_dis(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'skill_dis'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->skill_dis = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: skill_dev of class  player_type */
static int tolua_get_player_type_skill_dev(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'skill_dev'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->skill_dev);
 return 1;
}

/* set function: skill_dev of class  player_type */
static int tolua_set_player_type_skill_dev(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'skill_dev'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->skill_dev = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: skill_sav of class  player_type */
static int tolua_get_player_type_skill_sav(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'skill_sav'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->skill_sav);
 return 1;
}

/* set function: skill_sav of class  player_type */
static int tolua_set_player_type_skill_sav(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'skill_sav'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->skill_sav = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: skill_stl of class  player_type */
static int tolua_get_player_type_skill_stl(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'skill_stl'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->skill_stl);
 return 1;
}

/* set function: skill_stl of class  player_type */
static int tolua_set_player_type_skill_stl(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'skill_stl'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->skill_stl = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: skill_srh of class  player_type */
static int tolua_get_player_type_skill_srh(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'skill_srh'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->skill_srh);
 return 1;
}

/* set function: skill_srh of class  player_type */
static int tolua_set_player_type_skill_srh(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'skill_srh'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->skill_srh = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: skill_fos of class  player_type */
static int tolua_get_player_type_skill_fos(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'skill_fos'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->skill_fos);
 return 1;
}

/* set function: skill_fos of class  player_type */
static int tolua_set_player_type_skill_fos(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'skill_fos'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->skill_fos = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: skill_thn of class  player_type */
static int tolua_get_player_type_skill_thn(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'skill_thn'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->skill_thn);
 return 1;
}

/* set function: skill_thn of class  player_type */
static int tolua_set_player_type_skill_thn(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'skill_thn'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->skill_thn = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: skill_thb of class  player_type */
static int tolua_get_player_type_skill_thb(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'skill_thb'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->skill_thb);
 return 1;
}

/* set function: skill_thb of class  player_type */
static int tolua_set_player_type_skill_thb(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'skill_thb'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->skill_thb = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: skill_tht of class  player_type */
static int tolua_get_player_type_skill_tht(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'skill_tht'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->skill_tht);
 return 1;
}

/* set function: skill_tht of class  player_type */
static int tolua_set_player_type_skill_tht(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'skill_tht'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->skill_tht = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: skill_dig of class  player_type */
static int tolua_get_player_type_skill_dig(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'skill_dig'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->skill_dig);
 return 1;
}

/* set function: skill_dig of class  player_type */
static int tolua_set_player_type_skill_dig(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'skill_dig'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->skill_dig = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: noise of class  player_type */
static int tolua_get_player_type_noise(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'noise'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->noise);
 return 1;
}

/* set function: noise of class  player_type */
static int tolua_set_player_type_noise(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'noise'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->noise = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: num_blow of class  player_type */
static int tolua_get_player_type_num_blow(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'num_blow'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->num_blow);
 return 1;
}

/* set function: num_blow of class  player_type */
static int tolua_set_player_type_num_blow(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'num_blow'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->num_blow = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: num_fire of class  player_type */
static int tolua_get_player_type_num_fire(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'num_fire'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->num_fire);
 return 1;
}

/* set function: num_fire of class  player_type */
static int tolua_set_player_type_num_fire(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'num_fire'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->num_fire = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: ammo_mult of class  player_type */
static int tolua_get_player_type_ammo_mult(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ammo_mult'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->ammo_mult);
 return 1;
}

/* set function: ammo_mult of class  player_type */
static int tolua_set_player_type_ammo_mult(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ammo_mult'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ammo_mult = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: ammo_tval of class  player_type */
static int tolua_get_player_type_ammo_tval(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ammo_tval'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->ammo_tval);
 return 1;
}

/* set function: ammo_tval of class  player_type */
static int tolua_set_player_type_ammo_tval(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ammo_tval'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ammo_tval = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: pspeed of class  player_type */
static int tolua_get_player_type_pspeed(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'pspeed'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->pspeed);
 return 1;
}

/* set function: pspeed of class  player_type */
static int tolua_set_player_type_pspeed(lua_State* tolua_S)
{
  player_type* self = (player_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'pspeed'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->pspeed = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: title of class  player_sex */
static int tolua_get_player_sex_title(lua_State* tolua_S)
{
  player_sex* self = (player_sex*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'title'",NULL);
#endif
 tolua_pushstring(tolua_S,(const char*)self->title);
 return 1;
}

/* set function: title of class  player_sex */
static int tolua_set_player_sex_title(lua_State* tolua_S)
{
  player_sex* self = (player_sex*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'title'",NULL);
 if (!tolua_isstring(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->title = ((cptr)  tolua_tostring(tolua_S,2,0));
 return 0;
}

/* get function: winner of class  player_sex */
static int tolua_get_player_sex_winner(lua_State* tolua_S)
{
  player_sex* self = (player_sex*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'winner'",NULL);
#endif
 tolua_pushstring(tolua_S,(const char*)self->winner);
 return 1;
}

/* set function: winner of class  player_sex */
static int tolua_set_player_sex_winner(lua_State* tolua_S)
{
  player_sex* self = (player_sex*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'winner'",NULL);
 if (!tolua_isstring(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->winner = ((cptr)  tolua_tostring(tolua_S,2,0));
 return 0;
}

/* get function: name of class  player_race */
static int tolua_get_player_race_name(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'name'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->name);
 return 1;
}

/* set function: name of class  player_race */
static int tolua_set_player_race_name(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'name'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->name = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: text of class  player_race */
static int tolua_get_player_race_text(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'text'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->text);
 return 1;
}

/* set function: text of class  player_race */
static int tolua_set_player_race_text(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'text'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->text = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: r_adj of class  player_race */
static int tolua_get_player_player_race_r_adj(lua_State* tolua_S)
{
 int tolua_index;
  player_race* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_race*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->r_adj[tolua_index]);
 return 1;
}

/* set function: r_adj of class  player_race */
static int tolua_set_player_player_race_r_adj(lua_State* tolua_S)
{
 int tolua_index;
  player_race* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_race*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->r_adj[tolua_index] = ((s16b)  tolua_tonumber(tolua_S,3,0));
 return 0;
}

/* get function: r_dis of class  player_race */
static int tolua_get_player_race_r_dis(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'r_dis'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->r_dis);
 return 1;
}

/* set function: r_dis of class  player_race */
static int tolua_set_player_race_r_dis(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'r_dis'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->r_dis = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: r_dev of class  player_race */
static int tolua_get_player_race_r_dev(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'r_dev'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->r_dev);
 return 1;
}

/* set function: r_dev of class  player_race */
static int tolua_set_player_race_r_dev(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'r_dev'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->r_dev = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: r_sav of class  player_race */
static int tolua_get_player_race_r_sav(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'r_sav'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->r_sav);
 return 1;
}

/* set function: r_sav of class  player_race */
static int tolua_set_player_race_r_sav(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'r_sav'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->r_sav = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: r_stl of class  player_race */
static int tolua_get_player_race_r_stl(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'r_stl'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->r_stl);
 return 1;
}

/* set function: r_stl of class  player_race */
static int tolua_set_player_race_r_stl(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'r_stl'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->r_stl = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: r_srh of class  player_race */
static int tolua_get_player_race_r_srh(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'r_srh'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->r_srh);
 return 1;
}

/* set function: r_srh of class  player_race */
static int tolua_set_player_race_r_srh(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'r_srh'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->r_srh = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: r_fos of class  player_race */
static int tolua_get_player_race_r_fos(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'r_fos'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->r_fos);
 return 1;
}

/* set function: r_fos of class  player_race */
static int tolua_set_player_race_r_fos(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'r_fos'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->r_fos = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: r_thn of class  player_race */
static int tolua_get_player_race_r_thn(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'r_thn'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->r_thn);
 return 1;
}

/* set function: r_thn of class  player_race */
static int tolua_set_player_race_r_thn(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'r_thn'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->r_thn = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: r_thb of class  player_race */
static int tolua_get_player_race_r_thb(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'r_thb'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->r_thb);
 return 1;
}

/* set function: r_thb of class  player_race */
static int tolua_set_player_race_r_thb(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'r_thb'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->r_thb = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: r_mhp of class  player_race */
static int tolua_get_player_race_r_mhp(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'r_mhp'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->r_mhp);
 return 1;
}

/* set function: r_mhp of class  player_race */
static int tolua_set_player_race_r_mhp(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'r_mhp'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->r_mhp = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: r_exp of class  player_race */
static int tolua_get_player_race_r_exp(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'r_exp'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->r_exp);
 return 1;
}

/* set function: r_exp of class  player_race */
static int tolua_set_player_race_r_exp(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'r_exp'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->r_exp = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: b_age of class  player_race */
static int tolua_get_player_race_b_age(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'b_age'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->b_age);
 return 1;
}

/* set function: b_age of class  player_race */
static int tolua_set_player_race_b_age(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'b_age'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->b_age = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: m_age of class  player_race */
static int tolua_get_player_race_m_age(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'm_age'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->m_age);
 return 1;
}

/* set function: m_age of class  player_race */
static int tolua_set_player_race_m_age(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'm_age'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->m_age = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: m_b_ht of class  player_race */
static int tolua_get_player_race_m_b_ht(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'm_b_ht'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->m_b_ht);
 return 1;
}

/* set function: m_b_ht of class  player_race */
static int tolua_set_player_race_m_b_ht(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'm_b_ht'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->m_b_ht = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: m_m_ht of class  player_race */
static int tolua_get_player_race_m_m_ht(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'm_m_ht'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->m_m_ht);
 return 1;
}

/* set function: m_m_ht of class  player_race */
static int tolua_set_player_race_m_m_ht(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'm_m_ht'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->m_m_ht = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: m_b_wt of class  player_race */
static int tolua_get_player_race_m_b_wt(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'm_b_wt'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->m_b_wt);
 return 1;
}

/* set function: m_b_wt of class  player_race */
static int tolua_set_player_race_m_b_wt(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'm_b_wt'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->m_b_wt = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: m_m_wt of class  player_race */
static int tolua_get_player_race_m_m_wt(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'm_m_wt'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->m_m_wt);
 return 1;
}

/* set function: m_m_wt of class  player_race */
static int tolua_set_player_race_m_m_wt(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'm_m_wt'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->m_m_wt = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: f_b_ht of class  player_race */
static int tolua_get_player_race_f_b_ht(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'f_b_ht'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->f_b_ht);
 return 1;
}

/* set function: f_b_ht of class  player_race */
static int tolua_set_player_race_f_b_ht(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'f_b_ht'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->f_b_ht = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: f_m_ht of class  player_race */
static int tolua_get_player_race_f_m_ht(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'f_m_ht'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->f_m_ht);
 return 1;
}

/* set function: f_m_ht of class  player_race */
static int tolua_set_player_race_f_m_ht(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'f_m_ht'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->f_m_ht = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: f_b_wt of class  player_race */
static int tolua_get_player_race_f_b_wt(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'f_b_wt'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->f_b_wt);
 return 1;
}

/* set function: f_b_wt of class  player_race */
static int tolua_set_player_race_f_b_wt(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'f_b_wt'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->f_b_wt = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: f_m_wt of class  player_race */
static int tolua_get_player_race_f_m_wt(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'f_m_wt'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->f_m_wt);
 return 1;
}

/* set function: f_m_wt of class  player_race */
static int tolua_set_player_race_f_m_wt(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'f_m_wt'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->f_m_wt = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: infra of class  player_race */
static int tolua_get_player_race_infra(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'infra'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->infra);
 return 1;
}

/* set function: infra of class  player_race */
static int tolua_set_player_race_infra(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'infra'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->infra = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: choice of class  player_race */
static int tolua_get_player_race_choice(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'choice'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->choice);
 return 1;
}

/* set function: choice of class  player_race */
static int tolua_set_player_race_choice(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'choice'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->choice = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: hist of class  player_race */
static int tolua_get_player_race_hist(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'hist'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->hist);
 return 1;
}

/* set function: hist of class  player_race */
static int tolua_set_player_race_hist(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'hist'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->hist = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flags1 of class  player_race */
static int tolua_get_player_race_flags1(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags1'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags1);
 return 1;
}

/* set function: flags1 of class  player_race */
static int tolua_set_player_race_flags1(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags1'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags1 = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flags2 of class  player_race */
static int tolua_get_player_race_flags2(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags2'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags2);
 return 1;
}

/* set function: flags2 of class  player_race */
static int tolua_set_player_race_flags2(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags2'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags2 = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flags3 of class  player_race */
static int tolua_get_player_race_flags3(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags3'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags3);
 return 1;
}

/* set function: flags3 of class  player_race */
static int tolua_set_player_race_flags3(lua_State* tolua_S)
{
  player_race* self = (player_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags3'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags3 = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: tval of class  start_item */
static int tolua_get_start_item_tval(lua_State* tolua_S)
{
  start_item* self = (start_item*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'tval'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->tval);
 return 1;
}

/* set function: tval of class  start_item */
static int tolua_set_start_item_tval(lua_State* tolua_S)
{
  start_item* self = (start_item*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'tval'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->tval = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: sval of class  start_item */
static int tolua_get_start_item_sval(lua_State* tolua_S)
{
  start_item* self = (start_item*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sval'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->sval);
 return 1;
}

/* set function: sval of class  start_item */
static int tolua_set_start_item_sval(lua_State* tolua_S)
{
  start_item* self = (start_item*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sval'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->sval = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: min of class  start_item */
static int tolua_get_start_item_min(lua_State* tolua_S)
{
  start_item* self = (start_item*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'min'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->min);
 return 1;
}

/* set function: min of class  start_item */
static int tolua_set_start_item_min(lua_State* tolua_S)
{
  start_item* self = (start_item*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'min'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->min = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: max of class  start_item */
static int tolua_get_start_item_max(lua_State* tolua_S)
{
  start_item* self = (start_item*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'max'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->max);
 return 1;
}

/* set function: max of class  start_item */
static int tolua_set_start_item_max(lua_State* tolua_S)
{
  start_item* self = (start_item*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'max'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->max = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: name of class  player_class */
static int tolua_get_player_class_name(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'name'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->name);
 return 1;
}

/* set function: name of class  player_class */
static int tolua_set_player_class_name(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'name'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->name = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: title of class  player_class */
static int tolua_get_player_player_class_title(lua_State* tolua_S)
{
 int tolua_index;
  player_class* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_class*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=10)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->title[tolua_index]);
 return 1;
}

/* set function: title of class  player_class */
static int tolua_set_player_player_class_title(lua_State* tolua_S)
{
 int tolua_index;
  player_class* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_class*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=10)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->title[tolua_index] = ((u32b)  tolua_tonumber(tolua_S,3,0));
 return 0;
}

/* get function: c_adj of class  player_class */
static int tolua_get_player_player_class_c_adj(lua_State* tolua_S)
{
 int tolua_index;
  player_class* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_class*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->c_adj[tolua_index]);
 return 1;
}

/* set function: c_adj of class  player_class */
static int tolua_set_player_player_class_c_adj(lua_State* tolua_S)
{
 int tolua_index;
  player_class* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_class*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=A_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->c_adj[tolua_index] = ((s16b)  tolua_tonumber(tolua_S,3,0));
 return 0;
}

/* get function: c_dis of class  player_class */
static int tolua_get_player_class_c_dis(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'c_dis'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->c_dis);
 return 1;
}

/* set function: c_dis of class  player_class */
static int tolua_set_player_class_c_dis(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'c_dis'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->c_dis = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: c_dev of class  player_class */
static int tolua_get_player_class_c_dev(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'c_dev'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->c_dev);
 return 1;
}

/* set function: c_dev of class  player_class */
static int tolua_set_player_class_c_dev(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'c_dev'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->c_dev = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: c_sav of class  player_class */
static int tolua_get_player_class_c_sav(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'c_sav'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->c_sav);
 return 1;
}

/* set function: c_sav of class  player_class */
static int tolua_set_player_class_c_sav(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'c_sav'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->c_sav = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: c_stl of class  player_class */
static int tolua_get_player_class_c_stl(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'c_stl'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->c_stl);
 return 1;
}

/* set function: c_stl of class  player_class */
static int tolua_set_player_class_c_stl(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'c_stl'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->c_stl = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: c_srh of class  player_class */
static int tolua_get_player_class_c_srh(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'c_srh'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->c_srh);
 return 1;
}

/* set function: c_srh of class  player_class */
static int tolua_set_player_class_c_srh(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'c_srh'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->c_srh = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: c_fos of class  player_class */
static int tolua_get_player_class_c_fos(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'c_fos'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->c_fos);
 return 1;
}

/* set function: c_fos of class  player_class */
static int tolua_set_player_class_c_fos(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'c_fos'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->c_fos = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: c_thn of class  player_class */
static int tolua_get_player_class_c_thn(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'c_thn'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->c_thn);
 return 1;
}

/* set function: c_thn of class  player_class */
static int tolua_set_player_class_c_thn(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'c_thn'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->c_thn = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: c_thb of class  player_class */
static int tolua_get_player_class_c_thb(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'c_thb'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->c_thb);
 return 1;
}

/* set function: c_thb of class  player_class */
static int tolua_set_player_class_c_thb(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'c_thb'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->c_thb = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: x_dis of class  player_class */
static int tolua_get_player_class_x_dis(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_dis'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->x_dis);
 return 1;
}

/* set function: x_dis of class  player_class */
static int tolua_set_player_class_x_dis(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_dis'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->x_dis = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: x_dev of class  player_class */
static int tolua_get_player_class_x_dev(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_dev'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->x_dev);
 return 1;
}

/* set function: x_dev of class  player_class */
static int tolua_set_player_class_x_dev(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_dev'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->x_dev = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: x_sav of class  player_class */
static int tolua_get_player_class_x_sav(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_sav'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->x_sav);
 return 1;
}

/* set function: x_sav of class  player_class */
static int tolua_set_player_class_x_sav(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_sav'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->x_sav = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: x_stl of class  player_class */
static int tolua_get_player_class_x_stl(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_stl'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->x_stl);
 return 1;
}

/* set function: x_stl of class  player_class */
static int tolua_set_player_class_x_stl(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_stl'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->x_stl = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: x_srh of class  player_class */
static int tolua_get_player_class_x_srh(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_srh'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->x_srh);
 return 1;
}

/* set function: x_srh of class  player_class */
static int tolua_set_player_class_x_srh(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_srh'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->x_srh = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: x_fos of class  player_class */
static int tolua_get_player_class_x_fos(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_fos'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->x_fos);
 return 1;
}

/* set function: x_fos of class  player_class */
static int tolua_set_player_class_x_fos(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_fos'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->x_fos = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: x_thn of class  player_class */
static int tolua_get_player_class_x_thn(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_thn'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->x_thn);
 return 1;
}

/* set function: x_thn of class  player_class */
static int tolua_set_player_class_x_thn(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_thn'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->x_thn = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: x_thb of class  player_class */
static int tolua_get_player_class_x_thb(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_thb'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->x_thb);
 return 1;
}

/* set function: x_thb of class  player_class */
static int tolua_set_player_class_x_thb(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_thb'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->x_thb = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: c_mhp of class  player_class */
static int tolua_get_player_class_c_mhp(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'c_mhp'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->c_mhp);
 return 1;
}

/* set function: c_mhp of class  player_class */
static int tolua_set_player_class_c_mhp(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'c_mhp'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->c_mhp = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: c_exp of class  player_class */
static int tolua_get_player_class_c_exp(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'c_exp'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->c_exp);
 return 1;
}

/* set function: c_exp of class  player_class */
static int tolua_set_player_class_c_exp(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'c_exp'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->c_exp = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flags of class  player_class */
static int tolua_get_player_class_flags(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags);
 return 1;
}

/* set function: flags of class  player_class */
static int tolua_set_player_class_flags(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: max_attacks of class  player_class */
static int tolua_get_player_class_max_attacks(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'max_attacks'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->max_attacks);
 return 1;
}

/* set function: max_attacks of class  player_class */
static int tolua_set_player_class_max_attacks(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'max_attacks'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->max_attacks = ((u16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: min_weight of class  player_class */
static int tolua_get_player_class_min_weight(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'min_weight'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->min_weight);
 return 1;
}

/* set function: min_weight of class  player_class */
static int tolua_set_player_class_min_weight(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'min_weight'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->min_weight = ((u16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: att_multiply of class  player_class */
static int tolua_get_player_class_att_multiply(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'att_multiply'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->att_multiply);
 return 1;
}

/* set function: att_multiply of class  player_class */
static int tolua_set_player_class_att_multiply(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'att_multiply'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->att_multiply = ((u16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: spell_book of class  player_class */
static int tolua_get_player_class_spell_book(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'spell_book'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->spell_book);
 return 1;
}

/* set function: spell_book of class  player_class */
static int tolua_set_player_class_spell_book(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'spell_book'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->spell_book = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: spell_stat of class  player_class */
static int tolua_get_player_class_spell_stat(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'spell_stat'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->spell_stat);
 return 1;
}

/* set function: spell_stat of class  player_class */
static int tolua_set_player_class_spell_stat(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'spell_stat'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->spell_stat = ((u16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: spell_first of class  player_class */
static int tolua_get_player_class_spell_first(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'spell_first'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->spell_first);
 return 1;
}

/* set function: spell_first of class  player_class */
static int tolua_set_player_class_spell_first(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'spell_first'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->spell_first = ((u16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: spell_weight of class  player_class */
static int tolua_get_player_class_spell_weight(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'spell_weight'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->spell_weight);
 return 1;
}

/* set function: spell_weight of class  player_class */
static int tolua_set_player_class_spell_weight(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'spell_weight'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->spell_weight = ((u16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: sense_base of class  player_class */
static int tolua_get_player_class_sense_base(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sense_base'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->sense_base);
 return 1;
}

/* set function: sense_base of class  player_class */
static int tolua_set_player_class_sense_base(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sense_base'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->sense_base = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: sense_div of class  player_class */
static int tolua_get_player_class_sense_div(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sense_div'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->sense_div);
 return 1;
}

/* set function: sense_div of class  player_class */
static int tolua_set_player_class_sense_div(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sense_div'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->sense_div = ((u16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: start_items of class  player_class */
static int tolua_get_player_player_class_start_items(lua_State* tolua_S)
{
 int tolua_index;
  player_class* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_class*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=MAX_START_ITEMS)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushusertype(tolua_S,(void*)&self->start_items[tolua_index],"start_item");
 return 1;
}

/* set function: start_items of class  player_class */
static int tolua_set_player_player_class_start_items(lua_State* tolua_S)
{
 int tolua_index;
  player_class* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_class*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=MAX_START_ITEMS)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->start_items[tolua_index] = *((start_item*)  tolua_tousertype(tolua_S,3,0));
 return 0;
}

/* get function: spells of class  player_class */
static int tolua_get_player_class_spells(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'spells'",NULL);
#endif
 tolua_pushusertype(tolua_S,(void*)&self->spells,"player_magic");
 return 1;
}

/* set function: spells of class  player_class */
static int tolua_set_player_class_spells(lua_State* tolua_S)
{
  player_class* self = (player_class*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'spells'",NULL);
 if (!tolua_isusertype(tolua_S,2,"player_magic",0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->spells = *((player_magic*)  tolua_tousertype(tolua_S,2,0));
 return 0;
}

/* get function: text of class  hist_type */
static int tolua_get_hist_type_text(lua_State* tolua_S)
{
  hist_type* self = (hist_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'text'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->text);
 return 1;
}

/* set function: text of class  hist_type */
static int tolua_set_hist_type_text(lua_State* tolua_S)
{
  hist_type* self = (hist_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'text'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->text = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: roll of class  hist_type */
static int tolua_get_hist_type_roll(lua_State* tolua_S)
{
  hist_type* self = (hist_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'roll'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->roll);
 return 1;
}

/* set function: roll of class  hist_type */
static int tolua_set_hist_type_roll(lua_State* tolua_S)
{
  hist_type* self = (hist_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'roll'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->roll = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: chart of class  hist_type */
static int tolua_get_hist_type_chart(lua_State* tolua_S)
{
  hist_type* self = (hist_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'chart'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->chart);
 return 1;
}

/* set function: chart of class  hist_type */
static int tolua_set_hist_type_chart(lua_State* tolua_S)
{
  hist_type* self = (hist_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'chart'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->chart = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: next of class  hist_type */
static int tolua_get_hist_type_next(lua_State* tolua_S)
{
  hist_type* self = (hist_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'next'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->next);
 return 1;
}

/* set function: next of class  hist_type */
static int tolua_set_hist_type_next(lua_State* tolua_S)
{
  hist_type* self = (hist_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'next'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->next = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: bonus of class  hist_type */
static int tolua_get_hist_type_bonus(lua_State* tolua_S)
{
  hist_type* self = (hist_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'bonus'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->bonus);
 return 1;
}

/* set function: bonus of class  hist_type */
static int tolua_set_hist_type_bonus(lua_State* tolua_S)
{
  hist_type* self = (hist_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'bonus'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->bonus = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: full_name of class  player_other */
static int tolua_get_player_other_full_name(lua_State* tolua_S)
{
  player_other* self = (player_other*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'full_name'",NULL);
#endif
 tolua_pushstring(tolua_S,(const char*)self->full_name);
 return 1;
}

/* set function: full_name of class  player_other */
static int tolua_set_player_other_full_name(lua_State* tolua_S)
{
  player_other* self = (player_other*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'full_name'",NULL);
 if (!tolua_isstring(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
 strncpy(self->full_name,tolua_tostring(tolua_S,2,0),32-1);
 return 0;
}

/* get function: base_name of class  player_other */
static int tolua_get_player_other_base_name(lua_State* tolua_S)
{
  player_other* self = (player_other*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'base_name'",NULL);
#endif
 tolua_pushstring(tolua_S,(const char*)self->base_name);
 return 1;
}

/* set function: base_name of class  player_other */
static int tolua_set_player_other_base_name(lua_State* tolua_S)
{
  player_other* self = (player_other*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'base_name'",NULL);
 if (!tolua_isstring(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
 strncpy(self->base_name,tolua_tostring(tolua_S,2,0),32-1);
 return 0;
}

/* get function: opt of class  player_other */
static int tolua_get_player_player_other_opt(lua_State* tolua_S)
{
 int tolua_index;
  player_other* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_other*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=OPT_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->opt[tolua_index]);
 return 1;
}

/* set function: opt of class  player_other */
static int tolua_set_player_player_other_opt(lua_State* tolua_S)
{
 int tolua_index;
  player_other* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_other*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=OPT_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->opt[tolua_index] = ((bool)  tolua_toboolean(tolua_S,3,0));
 return 0;
}

/* get function: window_flag of class  player_other */
static int tolua_get_player_player_other_window_flag(lua_State* tolua_S)
{
 int tolua_index;
  player_other* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_other*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=ANGBAND_TERM_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->window_flag[tolua_index]);
 return 1;
}

/* set function: window_flag of class  player_other */
static int tolua_set_player_player_other_window_flag(lua_State* tolua_S)
{
 int tolua_index;
  player_other* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (player_other*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=ANGBAND_TERM_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->window_flag[tolua_index] = ((u32b)  tolua_tonumber(tolua_S,3,0));
 return 0;
}

/* get function: hitpoint_warn of class  player_other */
static int tolua_get_player_other_hitpoint_warn(lua_State* tolua_S)
{
  player_other* self = (player_other*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'hitpoint_warn'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->hitpoint_warn);
 return 1;
}

/* set function: hitpoint_warn of class  player_other */
static int tolua_set_player_other_hitpoint_warn(lua_State* tolua_S)
{
  player_other* self = (player_other*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'hitpoint_warn'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->hitpoint_warn = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: delay_factor of class  player_other */
static int tolua_get_player_other_delay_factor(lua_State* tolua_S)
{
  player_other* self = (player_other*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'delay_factor'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->delay_factor);
 return 1;
}

/* set function: delay_factor of class  player_other */
static int tolua_set_player_other_delay_factor(lua_State* tolua_S)
{
  player_other* self = (player_other*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'delay_factor'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->delay_factor = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: sp_ptr */
static int tolua_get_sp_ptr_ptr(lua_State* tolua_S)
{
 tolua_pushusertype(tolua_S,(void*)sp_ptr,"const player_sex");
 return 1;
}

/* get function: rp_ptr */
static int tolua_get_rp_ptr_ptr(lua_State* tolua_S)
{
 tolua_pushusertype(tolua_S,(void*)rp_ptr,"const player_race");
 return 1;
}

/* get function: cp_ptr */
static int tolua_get_cp_ptr_ptr(lua_State* tolua_S)
{
 tolua_pushusertype(tolua_S,(void*)cp_ptr,"const player_class");
 return 1;
}

/* get function: mp_ptr */
static int tolua_get_mp_ptr_ptr(lua_State* tolua_S)
{
 tolua_pushusertype(tolua_S,(void*)mp_ptr,"const player_magic");
 return 1;
}

/* get function: op_ptr */
static int tolua_get_op_ptr_ptr(lua_State* tolua_S)
{
 tolua_pushusertype(tolua_S,(void*)op_ptr,"player_other");
 return 1;
}

/* set function: op_ptr */
static int tolua_set_op_ptr_ptr(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!tolua_isusertype(tolua_S,2,"player_other",0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  op_ptr = ((player_other*)  tolua_tousertype(tolua_S,2,0));
 return 0;
}

/* get function: p_ptr */
static int tolua_get_p_ptr_ptr(lua_State* tolua_S)
{
 tolua_pushusertype(tolua_S,(void*)p_ptr,"player_type");
 return 1;
}

/* set function: p_ptr */
static int tolua_set_p_ptr_ptr(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!tolua_isusertype(tolua_S,2,"player_type",0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  p_ptr = ((player_type*)  tolua_tousertype(tolua_S,2,0));
 return 0;
}

/* function: set_blind */
static int tolua_player_set_blind00(lua_State* tolua_S)
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
  int v = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  set_blind(v);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_blind'.",&tolua_err);
 return 0;
#endif
}

/* function: set_confused */
static int tolua_player_set_confused00(lua_State* tolua_S)
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
  int v = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  set_confused(v);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_confused'.",&tolua_err);
 return 0;
#endif
}

/* function: set_poisoned */
static int tolua_player_set_poisoned00(lua_State* tolua_S)
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
  int v = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  set_poisoned(v);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_poisoned'.",&tolua_err);
 return 0;
#endif
}

/* function: set_afraid */
static int tolua_player_set_afraid00(lua_State* tolua_S)
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
  int v = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  set_afraid(v);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_afraid'.",&tolua_err);
 return 0;
#endif
}

/* function: set_paralyzed */
static int tolua_player_set_paralyzed00(lua_State* tolua_S)
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
  int v = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  set_paralyzed(v);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_paralyzed'.",&tolua_err);
 return 0;
#endif
}

/* function: set_image */
static int tolua_player_set_image00(lua_State* tolua_S)
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
  int v = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  set_image(v);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_image'.",&tolua_err);
 return 0;
#endif
}

/* function: set_fast */
static int tolua_player_set_fast00(lua_State* tolua_S)
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
  int v = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  set_fast(v);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_fast'.",&tolua_err);
 return 0;
#endif
}

/* function: set_slow */
static int tolua_player_set_slow00(lua_State* tolua_S)
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
  int v = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  set_slow(v);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_slow'.",&tolua_err);
 return 0;
#endif
}

/* function: set_shield */
static int tolua_player_set_shield00(lua_State* tolua_S)
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
  int v = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  set_shield(v);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_shield'.",&tolua_err);
 return 0;
#endif
}

/* function: set_blessed */
static int tolua_player_set_blessed00(lua_State* tolua_S)
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
  int v = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  set_blessed(v);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_blessed'.",&tolua_err);
 return 0;
#endif
}

/* function: set_hero */
static int tolua_player_set_hero00(lua_State* tolua_S)
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
  int v = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  set_hero(v);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_hero'.",&tolua_err);
 return 0;
#endif
}

/* function: set_shero */
static int tolua_player_set_shero00(lua_State* tolua_S)
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
  int v = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  set_shero(v);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_shero'.",&tolua_err);
 return 0;
#endif
}

/* function: set_protevil */
static int tolua_player_set_protevil00(lua_State* tolua_S)
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
  int v = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  set_protevil(v);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_protevil'.",&tolua_err);
 return 0;
#endif
}

/* function: set_invuln */
static int tolua_player_set_invuln00(lua_State* tolua_S)
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
  int v = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  set_invuln(v);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_invuln'.",&tolua_err);
 return 0;
#endif
}

/* function: set_tim_invis */
static int tolua_player_set_tim_invis00(lua_State* tolua_S)
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
  int v = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  set_tim_invis(v);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_tim_invis'.",&tolua_err);
 return 0;
#endif
}

/* function: set_tim_infra */
static int tolua_player_set_tim_infra00(lua_State* tolua_S)
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
  int v = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  set_tim_infra(v);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_tim_infra'.",&tolua_err);
 return 0;
#endif
}

/* function: set_oppose_acid */
static int tolua_player_set_oppose_acid00(lua_State* tolua_S)
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
  int v = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  set_oppose_acid(v);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_oppose_acid'.",&tolua_err);
 return 0;
#endif
}

/* function: set_oppose_elec */
static int tolua_player_set_oppose_elec00(lua_State* tolua_S)
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
  int v = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  set_oppose_elec(v);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_oppose_elec'.",&tolua_err);
 return 0;
#endif
}

/* function: set_oppose_fire */
static int tolua_player_set_oppose_fire00(lua_State* tolua_S)
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
  int v = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  set_oppose_fire(v);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_oppose_fire'.",&tolua_err);
 return 0;
#endif
}

/* function: set_oppose_cold */
static int tolua_player_set_oppose_cold00(lua_State* tolua_S)
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
  int v = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  set_oppose_cold(v);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_oppose_cold'.",&tolua_err);
 return 0;
#endif
}

/* function: set_oppose_pois */
static int tolua_player_set_oppose_pois00(lua_State* tolua_S)
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
  int v = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  set_oppose_pois(v);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_oppose_pois'.",&tolua_err);
 return 0;
#endif
}

/* function: set_stun */
static int tolua_player_set_stun00(lua_State* tolua_S)
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
  int v = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  set_stun(v);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_stun'.",&tolua_err);
 return 0;
#endif
}

/* function: set_cut */
static int tolua_player_set_cut00(lua_State* tolua_S)
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
  int v = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  set_cut(v);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_cut'.",&tolua_err);
 return 0;
#endif
}

/* function: set_food */
static int tolua_player_set_food00(lua_State* tolua_S)
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
  int v = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  set_food(v);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_food'.",&tolua_err);
 return 0;
#endif
}

/* function: check_experience */
static int tolua_player_check_experience00(lua_State* tolua_S)
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
  check_experience();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'check_experience'.",&tolua_err);
 return 0;
#endif
}

/* function: gain_exp */
static int tolua_player_gain_exp00(lua_State* tolua_S)
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
  s32b amount = ((s32b)  tolua_tonumber(tolua_S,1,0));
 {
  gain_exp(amount);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'gain_exp'.",&tolua_err);
 return 0;
#endif
}

/* function: lose_exp */
static int tolua_player_lose_exp00(lua_State* tolua_S)
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
  s32b amount = ((s32b)  tolua_tonumber(tolua_S,1,0));
 {
  lose_exp(amount);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'lose_exp'.",&tolua_err);
 return 0;
#endif
}

/* function: hp_player */
static int tolua_player_hp_player00(lua_State* tolua_S)
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
  bool tolua_ret = (bool)  hp_player(num);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'hp_player'.",&tolua_err);
 return 0;
#endif
}

/* function: do_dec_stat */
static int tolua_player_do_dec_stat00(lua_State* tolua_S)
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
  int stat = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  do_dec_stat(stat);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'do_dec_stat'.",&tolua_err);
 return 0;
#endif
}

/* function: do_res_stat */
static int tolua_player_do_res_stat00(lua_State* tolua_S)
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
  int stat = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  do_res_stat(stat);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'do_res_stat'.",&tolua_err);
 return 0;
#endif
}

/* function: do_inc_stat */
static int tolua_player_do_inc_stat00(lua_State* tolua_S)
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
  int stat = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  do_inc_stat(stat);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'do_inc_stat'.",&tolua_err);
 return 0;
#endif
}

/* function: take_hit */
static int tolua_player_take_hit00(lua_State* tolua_S)
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

/* function: inc_stat */
static int tolua_player_inc_stat00(lua_State* tolua_S)
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
  int stat = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  inc_stat(stat);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'inc_stat'.",&tolua_err);
 return 0;
#endif
}

/* function: dec_stat */
static int tolua_player_dec_stat00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isboolean(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int stat = ((int)  tolua_tonumber(tolua_S,1,0));
  int amount = ((int)  tolua_tonumber(tolua_S,2,0));
  bool permanent = ((bool)  tolua_toboolean(tolua_S,3,0));
 {
  bool tolua_ret = (bool)  dec_stat(stat,amount,permanent);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'dec_stat'.",&tolua_err);
 return 0;
#endif
}

/* function: res_stat */
static int tolua_player_res_stat00(lua_State* tolua_S)
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
  int stat = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  res_stat(stat);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'res_stat'.",&tolua_err);
 return 0;
#endif
}

/* function: restore_level */
static int tolua_player_restore_level00(lua_State* tolua_S)
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
  bool tolua_ret = (bool)  restore_level();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'restore_level'.",&tolua_err);
 return 0;
#endif
}

/* Open function */
TOLUA_API int tolua_player_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,1);
 tolua_beginmodule(tolua_S,NULL);
 tolua_constant(tolua_S,"A_STR",A_STR);
 tolua_constant(tolua_S,"A_INT",A_INT);
 tolua_constant(tolua_S,"A_WIS",A_WIS);
 tolua_constant(tolua_S,"A_DEX",A_DEX);
 tolua_constant(tolua_S,"A_CON",A_CON);
 tolua_constant(tolua_S,"A_CHR",A_CHR);
 tolua_constant(tolua_S,"A_MAX",A_MAX);
 tolua_constant(tolua_S,"SEX_FEMALE",SEX_FEMALE);
 tolua_constant(tolua_S,"SEX_MALE",SEX_MALE);
 tolua_constant(tolua_S,"PY_MAX_EXP",PY_MAX_EXP);
 tolua_constant(tolua_S,"PY_MAX_GOLD",PY_MAX_GOLD);
 tolua_constant(tolua_S,"PY_MAX_LEVEL",PY_MAX_LEVEL);
 tolua_constant(tolua_S,"PY_FOOD_MAX",PY_FOOD_MAX);
 tolua_constant(tolua_S,"PY_FOOD_FULL",PY_FOOD_FULL);
 tolua_constant(tolua_S,"PY_FOOD_ALERT",PY_FOOD_ALERT);
 tolua_constant(tolua_S,"PY_FOOD_WEAK",PY_FOOD_WEAK);
 tolua_constant(tolua_S,"PY_FOOD_FAINT",PY_FOOD_FAINT);
 tolua_constant(tolua_S,"PY_FOOD_STARVE",PY_FOOD_STARVE);
 tolua_constant(tolua_S,"PY_MAX_SPELLS",PY_MAX_SPELLS);
 tolua_constant(tolua_S,"PY_SPELL_LEARNED",PY_SPELL_LEARNED);
 tolua_constant(tolua_S,"PY_SPELL_WORKED",PY_SPELL_WORKED);
 tolua_constant(tolua_S,"PY_SPELL_FORGOTTEN",PY_SPELL_FORGOTTEN);
 tolua_constant(tolua_S,"PY_REGEN_NORMAL",PY_REGEN_NORMAL);
 tolua_constant(tolua_S,"PY_REGEN_WEAK",PY_REGEN_WEAK);
 tolua_constant(tolua_S,"PY_REGEN_FAINT",PY_REGEN_FAINT);
 tolua_constant(tolua_S,"PY_REGEN_HPBASE",PY_REGEN_HPBASE);
 tolua_constant(tolua_S,"PY_REGEN_MNBASE",PY_REGEN_MNBASE);
 tolua_constant(tolua_S,"CF_EXTRA_SHOT",CF_EXTRA_SHOT);
 tolua_constant(tolua_S,"CF_BRAVERY_30",CF_BRAVERY_30);
 tolua_constant(tolua_S,"CF_BLESS_WEAPON",CF_BLESS_WEAPON);
 tolua_constant(tolua_S,"CF_CUMBER_GLOVE",CF_CUMBER_GLOVE);
 tolua_constant(tolua_S,"CF_ZERO_FAIL",CF_ZERO_FAIL);
 tolua_constant(tolua_S,"CF_BEAM",CF_BEAM);
 tolua_constant(tolua_S,"CF_CHOOSE_SPELLS",CF_CHOOSE_SPELLS);
 tolua_constant(tolua_S,"CF_PSEUDO_ID_HEAVY",CF_PSEUDO_ID_HEAVY);
 tolua_constant(tolua_S,"CF_PSEUDO_ID_IMPROV",CF_PSEUDO_ID_IMPROV);
 tolua_cclass(tolua_S,"player_type","player_type","",NULL);
 tolua_beginmodule(tolua_S,"player_type");
 tolua_variable(tolua_S,"py",tolua_get_player_type_py,tolua_set_player_type_py);
 tolua_variable(tolua_S,"px",tolua_get_player_type_px,tolua_set_player_type_px);
 tolua_variable(tolua_S,"psex",tolua_get_player_type_psex,tolua_set_player_type_psex);
 tolua_variable(tolua_S,"prace",tolua_get_player_type_prace,tolua_set_player_type_prace);
 tolua_variable(tolua_S,"pclass",tolua_get_player_type_pclass,tolua_set_player_type_pclass);
 tolua_variable(tolua_S,"hitdie",tolua_get_player_type_hitdie,tolua_set_player_type_hitdie);
 tolua_variable(tolua_S,"expfact",tolua_get_player_type_expfact,tolua_set_player_type_expfact);
 tolua_variable(tolua_S,"age",tolua_get_player_type_age,tolua_set_player_type_age);
 tolua_variable(tolua_S,"ht",tolua_get_player_type_ht,tolua_set_player_type_ht);
 tolua_variable(tolua_S,"wt",tolua_get_player_type_wt,tolua_set_player_type_wt);
 tolua_variable(tolua_S,"sc",tolua_get_player_type_sc,tolua_set_player_type_sc);
 tolua_variable(tolua_S,"au",tolua_get_player_type_au,tolua_set_player_type_au);
 tolua_variable(tolua_S,"max_depth",tolua_get_player_type_max_depth,tolua_set_player_type_max_depth);
 tolua_variable(tolua_S,"depth",tolua_get_player_type_depth,tolua_set_player_type_depth);
 tolua_variable(tolua_S,"max_lev",tolua_get_player_type_max_lev,tolua_set_player_type_max_lev);
 tolua_variable(tolua_S,"lev",tolua_get_player_type_lev,tolua_set_player_type_lev);
 tolua_variable(tolua_S,"max_exp",tolua_get_player_type_max_exp,tolua_set_player_type_max_exp);
 tolua_variable(tolua_S,"exp",tolua_get_player_type_exp,tolua_set_player_type_exp);
 tolua_variable(tolua_S,"exp_frac",tolua_get_player_type_exp_frac,tolua_set_player_type_exp_frac);
 tolua_variable(tolua_S,"mhp",tolua_get_player_type_mhp,tolua_set_player_type_mhp);
 tolua_variable(tolua_S,"chp",tolua_get_player_type_chp,tolua_set_player_type_chp);
 tolua_variable(tolua_S,"chp_frac",tolua_get_player_type_chp_frac,tolua_set_player_type_chp_frac);
 tolua_variable(tolua_S,"msp",tolua_get_player_type_msp,tolua_set_player_type_msp);
 tolua_variable(tolua_S,"csp",tolua_get_player_type_csp,tolua_set_player_type_csp);
 tolua_variable(tolua_S,"csp_frac",tolua_get_player_type_csp_frac,tolua_set_player_type_csp_frac);
 tolua_array(tolua_S,"stat_max",tolua_get_player_player_type_stat_max,tolua_set_player_player_type_stat_max);
 tolua_array(tolua_S,"stat_cur",tolua_get_player_player_type_stat_cur,tolua_set_player_player_type_stat_cur);
 tolua_variable(tolua_S,"fast",tolua_get_player_type_fast,tolua_set_player_type_fast);
 tolua_variable(tolua_S,"slow",tolua_get_player_type_slow,tolua_set_player_type_slow);
 tolua_variable(tolua_S,"blind",tolua_get_player_type_blind,tolua_set_player_type_blind);
 tolua_variable(tolua_S,"paralyzed",tolua_get_player_type_paralyzed,tolua_set_player_type_paralyzed);
 tolua_variable(tolua_S,"confused",tolua_get_player_type_confused,tolua_set_player_type_confused);
 tolua_variable(tolua_S,"afraid",tolua_get_player_type_afraid,tolua_set_player_type_afraid);
 tolua_variable(tolua_S,"image",tolua_get_player_type_image,tolua_set_player_type_image);
 tolua_variable(tolua_S,"poisoned",tolua_get_player_type_poisoned,tolua_set_player_type_poisoned);
 tolua_variable(tolua_S,"cut",tolua_get_player_type_cut,tolua_set_player_type_cut);
 tolua_variable(tolua_S,"stun",tolua_get_player_type_stun,tolua_set_player_type_stun);
 tolua_variable(tolua_S,"protevil",tolua_get_player_type_protevil,tolua_set_player_type_protevil);
 tolua_variable(tolua_S,"invuln",tolua_get_player_type_invuln,tolua_set_player_type_invuln);
 tolua_variable(tolua_S,"hero",tolua_get_player_type_hero,tolua_set_player_type_hero);
 tolua_variable(tolua_S,"shero",tolua_get_player_type_shero,tolua_set_player_type_shero);
 tolua_variable(tolua_S,"shield",tolua_get_player_type_shield,tolua_set_player_type_shield);
 tolua_variable(tolua_S,"blessed",tolua_get_player_type_blessed,tolua_set_player_type_blessed);
 tolua_variable(tolua_S,"tim_invis",tolua_get_player_type_tim_invis,tolua_set_player_type_tim_invis);
 tolua_variable(tolua_S,"tim_infra",tolua_get_player_type_tim_infra,tolua_set_player_type_tim_infra);
 tolua_variable(tolua_S,"oppose_acid",tolua_get_player_type_oppose_acid,tolua_set_player_type_oppose_acid);
 tolua_variable(tolua_S,"oppose_elec",tolua_get_player_type_oppose_elec,tolua_set_player_type_oppose_elec);
 tolua_variable(tolua_S,"oppose_fire",tolua_get_player_type_oppose_fire,tolua_set_player_type_oppose_fire);
 tolua_variable(tolua_S,"oppose_cold",tolua_get_player_type_oppose_cold,tolua_set_player_type_oppose_cold);
 tolua_variable(tolua_S,"oppose_pois",tolua_get_player_type_oppose_pois,tolua_set_player_type_oppose_pois);
 tolua_variable(tolua_S,"word_recall",tolua_get_player_type_word_recall,tolua_set_player_type_word_recall);
 tolua_variable(tolua_S,"energy",tolua_get_player_type_energy,tolua_set_player_type_energy);
 tolua_variable(tolua_S,"food",tolua_get_player_type_food,tolua_set_player_type_food);
 tolua_variable(tolua_S,"confusing",tolua_get_player_type_confusing,tolua_set_player_type_confusing);
 tolua_variable(tolua_S,"searching",tolua_get_player_type_searching,tolua_set_player_type_searching);
 tolua_array(tolua_S,"spell_flags",tolua_get_player_player_type_spell_flags,tolua_set_player_player_type_spell_flags);
 tolua_array(tolua_S,"spell_order",tolua_get_player_player_type_spell_order,tolua_set_player_player_type_spell_order);
 tolua_array(tolua_S,"player_hp",tolua_get_player_player_type_player_hp,tolua_set_player_player_type_player_hp);
 tolua_variable(tolua_S,"died_from",tolua_get_player_type_died_from,tolua_set_player_type_died_from);
 tolua_variable(tolua_S,"total_winner",tolua_get_player_type_total_winner,tolua_set_player_type_total_winner);
 tolua_variable(tolua_S,"panic_save",tolua_get_player_type_panic_save,tolua_set_player_type_panic_save);
 tolua_variable(tolua_S,"noscore",tolua_get_player_type_noscore,tolua_set_player_type_noscore);
 tolua_variable(tolua_S,"is_dead",tolua_get_player_type_is_dead,tolua_set_player_type_is_dead);
 tolua_variable(tolua_S,"wizard",tolua_get_player_type_wizard,tolua_set_player_type_wizard);
 tolua_variable(tolua_S,"playing",tolua_get_player_type_playing,tolua_set_player_type_playing);
 tolua_variable(tolua_S,"leaving",tolua_get_player_type_leaving,tolua_set_player_type_leaving);
 tolua_variable(tolua_S,"create_up_stair",tolua_get_player_type_create_up_stair,tolua_set_player_type_create_up_stair);
 tolua_variable(tolua_S,"create_down_stair",tolua_get_player_type_create_down_stair,tolua_set_player_type_create_down_stair);
 tolua_variable(tolua_S,"wy",tolua_get_player_type_wy,tolua_set_player_type_wy);
 tolua_variable(tolua_S,"wx",tolua_get_player_type_wx,tolua_set_player_type_wx);
 tolua_variable(tolua_S,"total_weight",tolua_get_player_type_total_weight,tolua_set_player_type_total_weight);
 tolua_variable(tolua_S,"inven_cnt",tolua_get_player_type_inven_cnt,tolua_set_player_type_inven_cnt);
 tolua_variable(tolua_S,"equip_cnt",tolua_get_player_type_equip_cnt,tolua_set_player_type_equip_cnt);
 tolua_variable(tolua_S,"target_set",tolua_get_player_type_target_set,tolua_set_player_type_target_set);
 tolua_variable(tolua_S,"target_who",tolua_get_player_type_target_who,tolua_set_player_type_target_who);
 tolua_variable(tolua_S,"target_row",tolua_get_player_type_target_row,tolua_set_player_type_target_row);
 tolua_variable(tolua_S,"target_col",tolua_get_player_type_target_col,tolua_set_player_type_target_col);
 tolua_variable(tolua_S,"health_who",tolua_get_player_type_health_who,tolua_set_player_type_health_who);
 tolua_variable(tolua_S,"monster_race_idx",tolua_get_player_type_monster_race_idx,tolua_set_player_type_monster_race_idx);
 tolua_variable(tolua_S,"object_kind_idx",tolua_get_player_type_object_kind_idx,tolua_set_player_type_object_kind_idx);
 tolua_variable(tolua_S,"energy_use",tolua_get_player_type_energy_use,tolua_set_player_type_energy_use);
 tolua_variable(tolua_S,"resting",tolua_get_player_type_resting,tolua_set_player_type_resting);
 tolua_variable(tolua_S,"running",tolua_get_player_type_running,tolua_set_player_type_running);
 tolua_variable(tolua_S,"run_cur_dir",tolua_get_player_type_run_cur_dir,tolua_set_player_type_run_cur_dir);
 tolua_variable(tolua_S,"run_old_dir",tolua_get_player_type_run_old_dir,tolua_set_player_type_run_old_dir);
 tolua_variable(tolua_S,"run_unused",tolua_get_player_type_run_unused,tolua_set_player_type_run_unused);
 tolua_variable(tolua_S,"run_open_area",tolua_get_player_type_run_open_area,tolua_set_player_type_run_open_area);
 tolua_variable(tolua_S,"run_break_right",tolua_get_player_type_run_break_right,tolua_set_player_type_run_break_right);
 tolua_variable(tolua_S,"run_break_left",tolua_get_player_type_run_break_left,tolua_set_player_type_run_break_left);
 tolua_variable(tolua_S,"command_cmd",tolua_get_player_type_command_cmd,tolua_set_player_type_command_cmd);
 tolua_variable(tolua_S,"command_arg",tolua_get_player_type_command_arg,tolua_set_player_type_command_arg);
 tolua_variable(tolua_S,"command_rep",tolua_get_player_type_command_rep,tolua_set_player_type_command_rep);
 tolua_variable(tolua_S,"command_dir",tolua_get_player_type_command_dir,tolua_set_player_type_command_dir);
 tolua_variable(tolua_S,"command_see",tolua_get_player_type_command_see,tolua_set_player_type_command_see);
 tolua_variable(tolua_S,"command_wrk",tolua_get_player_type_command_wrk,tolua_set_player_type_command_wrk);
 tolua_variable(tolua_S,"command_new",tolua_get_player_type_command_new,tolua_set_player_type_command_new);
 tolua_variable(tolua_S,"new_spells",tolua_get_player_type_new_spells,tolua_set_player_type_new_spells);
 tolua_variable(tolua_S,"cumber_armor",tolua_get_player_type_cumber_armor,tolua_set_player_type_cumber_armor);
 tolua_variable(tolua_S,"cumber_glove",tolua_get_player_type_cumber_glove,tolua_set_player_type_cumber_glove);
 tolua_variable(tolua_S,"heavy_wield",tolua_get_player_type_heavy_wield,tolua_set_player_type_heavy_wield);
 tolua_variable(tolua_S,"heavy_shoot",tolua_get_player_type_heavy_shoot,tolua_set_player_type_heavy_shoot);
 tolua_variable(tolua_S,"icky_wield",tolua_get_player_type_icky_wield,tolua_set_player_type_icky_wield);
 tolua_variable(tolua_S,"cur_lite",tolua_get_player_type_cur_lite,tolua_set_player_type_cur_lite);
 tolua_variable(tolua_S,"notice",tolua_get_player_type_notice,tolua_set_player_type_notice);
 tolua_variable(tolua_S,"update",tolua_get_player_type_update,tolua_set_player_type_update);
 tolua_variable(tolua_S,"redraw",tolua_get_player_type_redraw,tolua_set_player_type_redraw);
 tolua_variable(tolua_S,"window",tolua_get_player_type_window,tolua_set_player_type_window);
 tolua_array(tolua_S,"stat_use",tolua_get_player_player_type_stat_use,tolua_set_player_player_type_stat_use);
 tolua_array(tolua_S,"stat_top",tolua_get_player_player_type_stat_top,tolua_set_player_player_type_stat_top);
 tolua_array(tolua_S,"stat_add",tolua_get_player_player_type_stat_add,tolua_set_player_player_type_stat_add);
 tolua_array(tolua_S,"stat_ind",tolua_get_player_player_type_stat_ind,tolua_set_player_player_type_stat_ind);
 tolua_variable(tolua_S,"immune_acid",tolua_get_player_type_immune_acid,tolua_set_player_type_immune_acid);
 tolua_variable(tolua_S,"immune_elec",tolua_get_player_type_immune_elec,tolua_set_player_type_immune_elec);
 tolua_variable(tolua_S,"immune_fire",tolua_get_player_type_immune_fire,tolua_set_player_type_immune_fire);
 tolua_variable(tolua_S,"immune_cold",tolua_get_player_type_immune_cold,tolua_set_player_type_immune_cold);
 tolua_variable(tolua_S,"resist_acid",tolua_get_player_type_resist_acid,tolua_set_player_type_resist_acid);
 tolua_variable(tolua_S,"resist_elec",tolua_get_player_type_resist_elec,tolua_set_player_type_resist_elec);
 tolua_variable(tolua_S,"resist_fire",tolua_get_player_type_resist_fire,tolua_set_player_type_resist_fire);
 tolua_variable(tolua_S,"resist_cold",tolua_get_player_type_resist_cold,tolua_set_player_type_resist_cold);
 tolua_variable(tolua_S,"resist_pois",tolua_get_player_type_resist_pois,tolua_set_player_type_resist_pois);
 tolua_variable(tolua_S,"resist_fear",tolua_get_player_type_resist_fear,tolua_set_player_type_resist_fear);
 tolua_variable(tolua_S,"resist_lite",tolua_get_player_type_resist_lite,tolua_set_player_type_resist_lite);
 tolua_variable(tolua_S,"resist_dark",tolua_get_player_type_resist_dark,tolua_set_player_type_resist_dark);
 tolua_variable(tolua_S,"resist_blind",tolua_get_player_type_resist_blind,tolua_set_player_type_resist_blind);
 tolua_variable(tolua_S,"resist_confu",tolua_get_player_type_resist_confu,tolua_set_player_type_resist_confu);
 tolua_variable(tolua_S,"resist_sound",tolua_get_player_type_resist_sound,tolua_set_player_type_resist_sound);
 tolua_variable(tolua_S,"resist_shard",tolua_get_player_type_resist_shard,tolua_set_player_type_resist_shard);
 tolua_variable(tolua_S,"resist_nexus",tolua_get_player_type_resist_nexus,tolua_set_player_type_resist_nexus);
 tolua_variable(tolua_S,"resist_nethr",tolua_get_player_type_resist_nethr,tolua_set_player_type_resist_nethr);
 tolua_variable(tolua_S,"resist_chaos",tolua_get_player_type_resist_chaos,tolua_set_player_type_resist_chaos);
 tolua_variable(tolua_S,"resist_disen",tolua_get_player_type_resist_disen,tolua_set_player_type_resist_disen);
 tolua_variable(tolua_S,"sustain_str",tolua_get_player_type_sustain_str,tolua_set_player_type_sustain_str);
 tolua_variable(tolua_S,"sustain_int",tolua_get_player_type_sustain_int,tolua_set_player_type_sustain_int);
 tolua_variable(tolua_S,"sustain_wis",tolua_get_player_type_sustain_wis,tolua_set_player_type_sustain_wis);
 tolua_variable(tolua_S,"sustain_dex",tolua_get_player_type_sustain_dex,tolua_set_player_type_sustain_dex);
 tolua_variable(tolua_S,"sustain_con",tolua_get_player_type_sustain_con,tolua_set_player_type_sustain_con);
 tolua_variable(tolua_S,"sustain_chr",tolua_get_player_type_sustain_chr,tolua_set_player_type_sustain_chr);
 tolua_variable(tolua_S,"slow_digest",tolua_get_player_type_slow_digest,tolua_set_player_type_slow_digest);
 tolua_variable(tolua_S,"ffall",tolua_get_player_type_ffall,tolua_set_player_type_ffall);
 tolua_variable(tolua_S,"lite",tolua_get_player_type_lite,tolua_set_player_type_lite);
 tolua_variable(tolua_S,"regenerate",tolua_get_player_type_regenerate,tolua_set_player_type_regenerate);
 tolua_variable(tolua_S,"telepathy",tolua_get_player_type_telepathy,tolua_set_player_type_telepathy);
 tolua_variable(tolua_S,"see_inv",tolua_get_player_type_see_inv,tolua_set_player_type_see_inv);
 tolua_variable(tolua_S,"free_act",tolua_get_player_type_free_act,tolua_set_player_type_free_act);
 tolua_variable(tolua_S,"hold_life",tolua_get_player_type_hold_life,tolua_set_player_type_hold_life);
 tolua_variable(tolua_S,"impact",tolua_get_player_type_impact,tolua_set_player_type_impact);
 tolua_variable(tolua_S,"aggravate",tolua_get_player_type_aggravate,tolua_set_player_type_aggravate);
 tolua_variable(tolua_S,"teleport",tolua_get_player_type_teleport,tolua_set_player_type_teleport);
 tolua_variable(tolua_S,"exp_drain",tolua_get_player_type_exp_drain,tolua_set_player_type_exp_drain);
 tolua_variable(tolua_S,"bless_blade",tolua_get_player_type_bless_blade,tolua_set_player_type_bless_blade);
 tolua_variable(tolua_S,"dis_to_h",tolua_get_player_type_dis_to_h,tolua_set_player_type_dis_to_h);
 tolua_variable(tolua_S,"dis_to_d",tolua_get_player_type_dis_to_d,tolua_set_player_type_dis_to_d);
 tolua_variable(tolua_S,"dis_to_a",tolua_get_player_type_dis_to_a,tolua_set_player_type_dis_to_a);
 tolua_variable(tolua_S,"dis_ac",tolua_get_player_type_dis_ac,tolua_set_player_type_dis_ac);
 tolua_variable(tolua_S,"to_h",tolua_get_player_type_to_h,tolua_set_player_type_to_h);
 tolua_variable(tolua_S,"to_d",tolua_get_player_type_to_d,tolua_set_player_type_to_d);
 tolua_variable(tolua_S,"to_a",tolua_get_player_type_to_a,tolua_set_player_type_to_a);
 tolua_variable(tolua_S,"ac",tolua_get_player_type_ac,tolua_set_player_type_ac);
 tolua_variable(tolua_S,"see_infra",tolua_get_player_type_see_infra,tolua_set_player_type_see_infra);
 tolua_variable(tolua_S,"skill_dis",tolua_get_player_type_skill_dis,tolua_set_player_type_skill_dis);
 tolua_variable(tolua_S,"skill_dev",tolua_get_player_type_skill_dev,tolua_set_player_type_skill_dev);
 tolua_variable(tolua_S,"skill_sav",tolua_get_player_type_skill_sav,tolua_set_player_type_skill_sav);
 tolua_variable(tolua_S,"skill_stl",tolua_get_player_type_skill_stl,tolua_set_player_type_skill_stl);
 tolua_variable(tolua_S,"skill_srh",tolua_get_player_type_skill_srh,tolua_set_player_type_skill_srh);
 tolua_variable(tolua_S,"skill_fos",tolua_get_player_type_skill_fos,tolua_set_player_type_skill_fos);
 tolua_variable(tolua_S,"skill_thn",tolua_get_player_type_skill_thn,tolua_set_player_type_skill_thn);
 tolua_variable(tolua_S,"skill_thb",tolua_get_player_type_skill_thb,tolua_set_player_type_skill_thb);
 tolua_variable(tolua_S,"skill_tht",tolua_get_player_type_skill_tht,tolua_set_player_type_skill_tht);
 tolua_variable(tolua_S,"skill_dig",tolua_get_player_type_skill_dig,tolua_set_player_type_skill_dig);
 tolua_variable(tolua_S,"noise",tolua_get_player_type_noise,tolua_set_player_type_noise);
 tolua_variable(tolua_S,"num_blow",tolua_get_player_type_num_blow,tolua_set_player_type_num_blow);
 tolua_variable(tolua_S,"num_fire",tolua_get_player_type_num_fire,tolua_set_player_type_num_fire);
 tolua_variable(tolua_S,"ammo_mult",tolua_get_player_type_ammo_mult,tolua_set_player_type_ammo_mult);
 tolua_variable(tolua_S,"ammo_tval",tolua_get_player_type_ammo_tval,tolua_set_player_type_ammo_tval);
 tolua_variable(tolua_S,"pspeed",tolua_get_player_type_pspeed,tolua_set_player_type_pspeed);
 tolua_endmodule(tolua_S);
 tolua_cclass(tolua_S,"player_sex","player_sex","",NULL);
 tolua_beginmodule(tolua_S,"player_sex");
 tolua_variable(tolua_S,"title",tolua_get_player_sex_title,tolua_set_player_sex_title);
 tolua_variable(tolua_S,"winner",tolua_get_player_sex_winner,tolua_set_player_sex_winner);
 tolua_endmodule(tolua_S);
 tolua_cclass(tolua_S,"player_race","player_race","",NULL);
 tolua_beginmodule(tolua_S,"player_race");
 tolua_variable(tolua_S,"name",tolua_get_player_race_name,tolua_set_player_race_name);
 tolua_variable(tolua_S,"text",tolua_get_player_race_text,tolua_set_player_race_text);
 tolua_array(tolua_S,"r_adj",tolua_get_player_player_race_r_adj,tolua_set_player_player_race_r_adj);
 tolua_variable(tolua_S,"r_dis",tolua_get_player_race_r_dis,tolua_set_player_race_r_dis);
 tolua_variable(tolua_S,"r_dev",tolua_get_player_race_r_dev,tolua_set_player_race_r_dev);
 tolua_variable(tolua_S,"r_sav",tolua_get_player_race_r_sav,tolua_set_player_race_r_sav);
 tolua_variable(tolua_S,"r_stl",tolua_get_player_race_r_stl,tolua_set_player_race_r_stl);
 tolua_variable(tolua_S,"r_srh",tolua_get_player_race_r_srh,tolua_set_player_race_r_srh);
 tolua_variable(tolua_S,"r_fos",tolua_get_player_race_r_fos,tolua_set_player_race_r_fos);
 tolua_variable(tolua_S,"r_thn",tolua_get_player_race_r_thn,tolua_set_player_race_r_thn);
 tolua_variable(tolua_S,"r_thb",tolua_get_player_race_r_thb,tolua_set_player_race_r_thb);
 tolua_variable(tolua_S,"r_mhp",tolua_get_player_race_r_mhp,tolua_set_player_race_r_mhp);
 tolua_variable(tolua_S,"r_exp",tolua_get_player_race_r_exp,tolua_set_player_race_r_exp);
 tolua_variable(tolua_S,"b_age",tolua_get_player_race_b_age,tolua_set_player_race_b_age);
 tolua_variable(tolua_S,"m_age",tolua_get_player_race_m_age,tolua_set_player_race_m_age);
 tolua_variable(tolua_S,"m_b_ht",tolua_get_player_race_m_b_ht,tolua_set_player_race_m_b_ht);
 tolua_variable(tolua_S,"m_m_ht",tolua_get_player_race_m_m_ht,tolua_set_player_race_m_m_ht);
 tolua_variable(tolua_S,"m_b_wt",tolua_get_player_race_m_b_wt,tolua_set_player_race_m_b_wt);
 tolua_variable(tolua_S,"m_m_wt",tolua_get_player_race_m_m_wt,tolua_set_player_race_m_m_wt);
 tolua_variable(tolua_S,"f_b_ht",tolua_get_player_race_f_b_ht,tolua_set_player_race_f_b_ht);
 tolua_variable(tolua_S,"f_m_ht",tolua_get_player_race_f_m_ht,tolua_set_player_race_f_m_ht);
 tolua_variable(tolua_S,"f_b_wt",tolua_get_player_race_f_b_wt,tolua_set_player_race_f_b_wt);
 tolua_variable(tolua_S,"f_m_wt",tolua_get_player_race_f_m_wt,tolua_set_player_race_f_m_wt);
 tolua_variable(tolua_S,"infra",tolua_get_player_race_infra,tolua_set_player_race_infra);
 tolua_variable(tolua_S,"choice",tolua_get_player_race_choice,tolua_set_player_race_choice);
 tolua_variable(tolua_S,"hist",tolua_get_player_race_hist,tolua_set_player_race_hist);
 tolua_variable(tolua_S,"flags1",tolua_get_player_race_flags1,tolua_set_player_race_flags1);
 tolua_variable(tolua_S,"flags2",tolua_get_player_race_flags2,tolua_set_player_race_flags2);
 tolua_variable(tolua_S,"flags3",tolua_get_player_race_flags3,tolua_set_player_race_flags3);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"start_item","start_item","",tolua_collect_start_item);
#else
 tolua_cclass(tolua_S,"start_item","start_item","",NULL);
#endif
 tolua_beginmodule(tolua_S,"start_item");
 tolua_variable(tolua_S,"tval",tolua_get_start_item_tval,tolua_set_start_item_tval);
 tolua_variable(tolua_S,"sval",tolua_get_start_item_sval,tolua_set_start_item_sval);
 tolua_variable(tolua_S,"min",tolua_get_start_item_min,tolua_set_start_item_min);
 tolua_variable(tolua_S,"max",tolua_get_start_item_max,tolua_set_start_item_max);
 tolua_endmodule(tolua_S);
 tolua_cclass(tolua_S,"player_class","player_class","",NULL);
 tolua_beginmodule(tolua_S,"player_class");
 tolua_variable(tolua_S,"name",tolua_get_player_class_name,tolua_set_player_class_name);
 tolua_array(tolua_S,"title",tolua_get_player_player_class_title,tolua_set_player_player_class_title);
 tolua_array(tolua_S,"c_adj",tolua_get_player_player_class_c_adj,tolua_set_player_player_class_c_adj);
 tolua_variable(tolua_S,"c_dis",tolua_get_player_class_c_dis,tolua_set_player_class_c_dis);
 tolua_variable(tolua_S,"c_dev",tolua_get_player_class_c_dev,tolua_set_player_class_c_dev);
 tolua_variable(tolua_S,"c_sav",tolua_get_player_class_c_sav,tolua_set_player_class_c_sav);
 tolua_variable(tolua_S,"c_stl",tolua_get_player_class_c_stl,tolua_set_player_class_c_stl);
 tolua_variable(tolua_S,"c_srh",tolua_get_player_class_c_srh,tolua_set_player_class_c_srh);
 tolua_variable(tolua_S,"c_fos",tolua_get_player_class_c_fos,tolua_set_player_class_c_fos);
 tolua_variable(tolua_S,"c_thn",tolua_get_player_class_c_thn,tolua_set_player_class_c_thn);
 tolua_variable(tolua_S,"c_thb",tolua_get_player_class_c_thb,tolua_set_player_class_c_thb);
 tolua_variable(tolua_S,"x_dis",tolua_get_player_class_x_dis,tolua_set_player_class_x_dis);
 tolua_variable(tolua_S,"x_dev",tolua_get_player_class_x_dev,tolua_set_player_class_x_dev);
 tolua_variable(tolua_S,"x_sav",tolua_get_player_class_x_sav,tolua_set_player_class_x_sav);
 tolua_variable(tolua_S,"x_stl",tolua_get_player_class_x_stl,tolua_set_player_class_x_stl);
 tolua_variable(tolua_S,"x_srh",tolua_get_player_class_x_srh,tolua_set_player_class_x_srh);
 tolua_variable(tolua_S,"x_fos",tolua_get_player_class_x_fos,tolua_set_player_class_x_fos);
 tolua_variable(tolua_S,"x_thn",tolua_get_player_class_x_thn,tolua_set_player_class_x_thn);
 tolua_variable(tolua_S,"x_thb",tolua_get_player_class_x_thb,tolua_set_player_class_x_thb);
 tolua_variable(tolua_S,"c_mhp",tolua_get_player_class_c_mhp,tolua_set_player_class_c_mhp);
 tolua_variable(tolua_S,"c_exp",tolua_get_player_class_c_exp,tolua_set_player_class_c_exp);
 tolua_variable(tolua_S,"flags",tolua_get_player_class_flags,tolua_set_player_class_flags);
 tolua_variable(tolua_S,"max_attacks",tolua_get_player_class_max_attacks,tolua_set_player_class_max_attacks);
 tolua_variable(tolua_S,"min_weight",tolua_get_player_class_min_weight,tolua_set_player_class_min_weight);
 tolua_variable(tolua_S,"att_multiply",tolua_get_player_class_att_multiply,tolua_set_player_class_att_multiply);
 tolua_variable(tolua_S,"spell_book",tolua_get_player_class_spell_book,tolua_set_player_class_spell_book);
 tolua_variable(tolua_S,"spell_stat",tolua_get_player_class_spell_stat,tolua_set_player_class_spell_stat);
 tolua_variable(tolua_S,"spell_first",tolua_get_player_class_spell_first,tolua_set_player_class_spell_first);
 tolua_variable(tolua_S,"spell_weight",tolua_get_player_class_spell_weight,tolua_set_player_class_spell_weight);
 tolua_variable(tolua_S,"sense_base",tolua_get_player_class_sense_base,tolua_set_player_class_sense_base);
 tolua_variable(tolua_S,"sense_div",tolua_get_player_class_sense_div,tolua_set_player_class_sense_div);
 tolua_array(tolua_S,"start_items",tolua_get_player_player_class_start_items,tolua_set_player_player_class_start_items);
 tolua_variable(tolua_S,"spells",tolua_get_player_class_spells,tolua_set_player_class_spells);
 tolua_endmodule(tolua_S);
 tolua_cclass(tolua_S,"hist_type","hist_type","",NULL);
 tolua_beginmodule(tolua_S,"hist_type");
 tolua_variable(tolua_S,"text",tolua_get_hist_type_text,tolua_set_hist_type_text);
 tolua_variable(tolua_S,"roll",tolua_get_hist_type_roll,tolua_set_hist_type_roll);
 tolua_variable(tolua_S,"chart",tolua_get_hist_type_chart,tolua_set_hist_type_chart);
 tolua_variable(tolua_S,"next",tolua_get_hist_type_next,tolua_set_hist_type_next);
 tolua_variable(tolua_S,"bonus",tolua_get_hist_type_bonus,tolua_set_hist_type_bonus);
 tolua_endmodule(tolua_S);
 tolua_cclass(tolua_S,"player_other","player_other","",NULL);
 tolua_beginmodule(tolua_S,"player_other");
 tolua_variable(tolua_S,"full_name",tolua_get_player_other_full_name,tolua_set_player_other_full_name);
 tolua_variable(tolua_S,"base_name",tolua_get_player_other_base_name,tolua_set_player_other_base_name);
 tolua_array(tolua_S,"opt",tolua_get_player_player_other_opt,tolua_set_player_player_other_opt);
 tolua_array(tolua_S,"window_flag",tolua_get_player_player_other_window_flag,tolua_set_player_player_other_window_flag);
 tolua_variable(tolua_S,"hitpoint_warn",tolua_get_player_other_hitpoint_warn,tolua_set_player_other_hitpoint_warn);
 tolua_variable(tolua_S,"delay_factor",tolua_get_player_other_delay_factor,tolua_set_player_other_delay_factor);
 tolua_endmodule(tolua_S);
 tolua_variable(tolua_S,"sp_ptr",tolua_get_sp_ptr_ptr,NULL);
 tolua_variable(tolua_S,"rp_ptr",tolua_get_rp_ptr_ptr,NULL);
 tolua_variable(tolua_S,"cp_ptr",tolua_get_cp_ptr_ptr,NULL);
 tolua_variable(tolua_S,"mp_ptr",tolua_get_mp_ptr_ptr,NULL);
 tolua_variable(tolua_S,"op_ptr",tolua_get_op_ptr_ptr,tolua_set_op_ptr_ptr);
 tolua_variable(tolua_S,"player",tolua_get_p_ptr_ptr,tolua_set_p_ptr_ptr);
 tolua_function(tolua_S,"set_blind",tolua_player_set_blind00);
 tolua_function(tolua_S,"set_confused",tolua_player_set_confused00);
 tolua_function(tolua_S,"set_poisoned",tolua_player_set_poisoned00);
 tolua_function(tolua_S,"set_afraid",tolua_player_set_afraid00);
 tolua_function(tolua_S,"set_paralyzed",tolua_player_set_paralyzed00);
 tolua_function(tolua_S,"set_image",tolua_player_set_image00);
 tolua_function(tolua_S,"set_fast",tolua_player_set_fast00);
 tolua_function(tolua_S,"set_slow",tolua_player_set_slow00);
 tolua_function(tolua_S,"set_shield",tolua_player_set_shield00);
 tolua_function(tolua_S,"set_blessed",tolua_player_set_blessed00);
 tolua_function(tolua_S,"set_hero",tolua_player_set_hero00);
 tolua_function(tolua_S,"set_shero",tolua_player_set_shero00);
 tolua_function(tolua_S,"set_protevil",tolua_player_set_protevil00);
 tolua_function(tolua_S,"set_invuln",tolua_player_set_invuln00);
 tolua_function(tolua_S,"set_tim_invis",tolua_player_set_tim_invis00);
 tolua_function(tolua_S,"set_tim_infra",tolua_player_set_tim_infra00);
 tolua_function(tolua_S,"set_oppose_acid",tolua_player_set_oppose_acid00);
 tolua_function(tolua_S,"set_oppose_elec",tolua_player_set_oppose_elec00);
 tolua_function(tolua_S,"set_oppose_fire",tolua_player_set_oppose_fire00);
 tolua_function(tolua_S,"set_oppose_cold",tolua_player_set_oppose_cold00);
 tolua_function(tolua_S,"set_oppose_pois",tolua_player_set_oppose_pois00);
 tolua_function(tolua_S,"set_stun",tolua_player_set_stun00);
 tolua_function(tolua_S,"set_cut",tolua_player_set_cut00);
 tolua_function(tolua_S,"set_food",tolua_player_set_food00);
 tolua_function(tolua_S,"check_experience",tolua_player_check_experience00);
 tolua_function(tolua_S,"gain_exp",tolua_player_gain_exp00);
 tolua_function(tolua_S,"lose_exp",tolua_player_lose_exp00);
 tolua_function(tolua_S,"hp_player",tolua_player_hp_player00);
 tolua_function(tolua_S,"do_dec_stat",tolua_player_do_dec_stat00);
 tolua_function(tolua_S,"do_res_stat",tolua_player_do_res_stat00);
 tolua_function(tolua_S,"do_inc_stat",tolua_player_do_inc_stat00);
 tolua_function(tolua_S,"take_hit",tolua_player_take_hit00);
 tolua_function(tolua_S,"inc_stat",tolua_player_inc_stat00);
 tolua_function(tolua_S,"dec_stat",tolua_player_dec_stat00);
 tolua_function(tolua_S,"res_stat",tolua_player_res_stat00);
 tolua_function(tolua_S,"restore_level",tolua_player_restore_level00);
 tolua_endmodule(tolua_S);
 return 1;
}
