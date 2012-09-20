/*
** Lua binding: object
** Generated automatically by tolua 5.0a on Sun May 23 19:11:26 2004.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "lua/tolua.h"

/* Exported function */
TOLUA_API int tolua_object_open (lua_State* tolua_S);

#include "angband.h"

/* function to release collected object via destructor */
#ifdef __cplusplus

static int tolua_collect_artifact_type (lua_State* tolua_S)
{
 artifact_type* self = (artifact_type*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_object_kind (lua_State* tolua_S)
{
 object_kind* self = (object_kind*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_ego_item_type (lua_State* tolua_S)
{
 ego_item_type* self = (ego_item_type*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_flavor_type (lua_State* tolua_S)
{
 flavor_type* self = (flavor_type*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_object_type (lua_State* tolua_S)
{
 object_type* self = (object_type*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}
#endif


/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"artifact_type");
 tolua_usertype(tolua_S,"object_kind");
 tolua_usertype(tolua_S,"ego_item_type");
 tolua_usertype(tolua_S,"flavor_type");
 tolua_usertype(tolua_S,"object_type");
}

/* get function: k_idx of class  object_type */
static int tolua_get_object_type_k_idx(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'k_idx'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->k_idx);
 return 1;
}

/* set function: k_idx of class  object_type */
static int tolua_set_object_type_k_idx(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'k_idx'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->k_idx = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: iy of class  object_type */
static int tolua_get_object_type_iy(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'iy'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->iy);
 return 1;
}

/* set function: iy of class  object_type */
static int tolua_set_object_type_iy(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'iy'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->iy = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: ix of class  object_type */
static int tolua_get_object_type_ix(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ix'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->ix);
 return 1;
}

/* set function: ix of class  object_type */
static int tolua_set_object_type_ix(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ix'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ix = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: tval of class  object_type */
static int tolua_get_object_type_tval(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'tval'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->tval);
 return 1;
}

/* set function: tval of class  object_type */
static int tolua_set_object_type_tval(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'tval'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->tval = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: sval of class  object_type */
static int tolua_get_object_type_sval(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sval'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->sval);
 return 1;
}

/* set function: sval of class  object_type */
static int tolua_set_object_type_sval(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sval'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->sval = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: pval of class  object_type */
static int tolua_get_object_type_pval(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'pval'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->pval);
 return 1;
}

/* set function: pval of class  object_type */
static int tolua_set_object_type_pval(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'pval'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->pval = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: discount of class  object_type */
static int tolua_get_object_type_discount(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'discount'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->discount);
 return 1;
}

/* set function: discount of class  object_type */
static int tolua_set_object_type_discount(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'discount'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->discount = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: number of class  object_type */
static int tolua_get_object_type_number(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'number'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->number);
 return 1;
}

/* set function: number of class  object_type */
static int tolua_set_object_type_number(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'number'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->number = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: weight of class  object_type */
static int tolua_get_object_type_weight(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'weight'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->weight);
 return 1;
}

/* set function: weight of class  object_type */
static int tolua_set_object_type_weight(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'weight'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->weight = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: name1 of class  object_type */
static int tolua_get_object_type_name1(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'name1'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->name1);
 return 1;
}

/* set function: name1 of class  object_type */
static int tolua_set_object_type_name1(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'name1'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->name1 = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: name2 of class  object_type */
static int tolua_get_object_type_name2(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'name2'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->name2);
 return 1;
}

/* set function: name2 of class  object_type */
static int tolua_set_object_type_name2(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'name2'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->name2 = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: xtra1 of class  object_type */
static int tolua_get_object_type_xtra1(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'xtra1'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->xtra1);
 return 1;
}

/* set function: xtra1 of class  object_type */
static int tolua_set_object_type_xtra1(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'xtra1'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->xtra1 = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: xtra2 of class  object_type */
static int tolua_get_object_type_xtra2(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'xtra2'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->xtra2);
 return 1;
}

/* set function: xtra2 of class  object_type */
static int tolua_set_object_type_xtra2(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'xtra2'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->xtra2 = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: to_h of class  object_type */
static int tolua_get_object_type_to_h(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'to_h'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->to_h);
 return 1;
}

/* set function: to_h of class  object_type */
static int tolua_set_object_type_to_h(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'to_h'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->to_h = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: to_d of class  object_type */
static int tolua_get_object_type_to_d(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'to_d'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->to_d);
 return 1;
}

/* set function: to_d of class  object_type */
static int tolua_set_object_type_to_d(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'to_d'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->to_d = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: to_a of class  object_type */
static int tolua_get_object_type_to_a(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'to_a'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->to_a);
 return 1;
}

/* set function: to_a of class  object_type */
static int tolua_set_object_type_to_a(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'to_a'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->to_a = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: ac of class  object_type */
static int tolua_get_object_type_ac(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ac'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->ac);
 return 1;
}

/* set function: ac of class  object_type */
static int tolua_set_object_type_ac(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ac'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ac = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: dd of class  object_type */
static int tolua_get_object_type_dd(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'dd'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->dd);
 return 1;
}

/* set function: dd of class  object_type */
static int tolua_set_object_type_dd(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'dd'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->dd = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: ds of class  object_type */
static int tolua_get_object_type_ds(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ds'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->ds);
 return 1;
}

/* set function: ds of class  object_type */
static int tolua_set_object_type_ds(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ds'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ds = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: timeout of class  object_type */
static int tolua_get_object_type_timeout(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'timeout'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->timeout);
 return 1;
}

/* set function: timeout of class  object_type */
static int tolua_set_object_type_timeout(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'timeout'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->timeout = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: ident of class  object_type */
static int tolua_get_object_type_ident(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ident'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->ident);
 return 1;
}

/* set function: ident of class  object_type */
static int tolua_set_object_type_ident(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ident'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ident = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: marked of class  object_type */
static int tolua_get_object_type_marked(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'marked'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->marked);
 return 1;
}

/* set function: marked of class  object_type */
static int tolua_set_object_type_marked(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'marked'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->marked = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: note of class  object_type */
static int tolua_get_object_type_note(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'note'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->note);
 return 1;
}

/* set function: note of class  object_type */
static int tolua_set_object_type_note(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'note'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->note = ((u16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: next_o_idx of class  object_type */
static int tolua_get_object_type_next_o_idx(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'next_o_idx'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->next_o_idx);
 return 1;
}

/* set function: next_o_idx of class  object_type */
static int tolua_set_object_type_next_o_idx(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'next_o_idx'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->next_o_idx = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: held_m_idx of class  object_type */
static int tolua_get_object_type_held_m_idx(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'held_m_idx'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->held_m_idx);
 return 1;
}

/* set function: held_m_idx of class  object_type */
static int tolua_set_object_type_held_m_idx(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'held_m_idx'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->held_m_idx = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: name of class  object_kind */
static int tolua_get_object_kind_name(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'name'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->name);
 return 1;
}

/* set function: name of class  object_kind */
static int tolua_set_object_kind_name(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'name'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->name = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: text of class  object_kind */
static int tolua_get_object_kind_text(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'text'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->text);
 return 1;
}

/* set function: text of class  object_kind */
static int tolua_set_object_kind_text(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'text'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->text = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: tval of class  object_kind */
static int tolua_get_object_kind_tval(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'tval'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->tval);
 return 1;
}

/* set function: tval of class  object_kind */
static int tolua_set_object_kind_tval(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'tval'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->tval = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: sval of class  object_kind */
static int tolua_get_object_kind_sval(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sval'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->sval);
 return 1;
}

/* set function: sval of class  object_kind */
static int tolua_set_object_kind_sval(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sval'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->sval = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: pval of class  object_kind */
static int tolua_get_object_kind_pval(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'pval'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->pval);
 return 1;
}

/* set function: pval of class  object_kind */
static int tolua_set_object_kind_pval(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'pval'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->pval = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: to_h of class  object_kind */
static int tolua_get_object_kind_to_h(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'to_h'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->to_h);
 return 1;
}

/* set function: to_h of class  object_kind */
static int tolua_set_object_kind_to_h(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'to_h'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->to_h = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: to_d of class  object_kind */
static int tolua_get_object_kind_to_d(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'to_d'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->to_d);
 return 1;
}

/* set function: to_d of class  object_kind */
static int tolua_set_object_kind_to_d(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'to_d'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->to_d = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: to_a of class  object_kind */
static int tolua_get_object_kind_to_a(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'to_a'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->to_a);
 return 1;
}

/* set function: to_a of class  object_kind */
static int tolua_set_object_kind_to_a(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'to_a'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->to_a = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: ac of class  object_kind */
static int tolua_get_object_kind_ac(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ac'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->ac);
 return 1;
}

/* set function: ac of class  object_kind */
static int tolua_set_object_kind_ac(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ac'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ac = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: dd of class  object_kind */
static int tolua_get_object_kind_dd(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'dd'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->dd);
 return 1;
}

/* set function: dd of class  object_kind */
static int tolua_set_object_kind_dd(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'dd'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->dd = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: ds of class  object_kind */
static int tolua_get_object_kind_ds(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ds'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->ds);
 return 1;
}

/* set function: ds of class  object_kind */
static int tolua_set_object_kind_ds(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ds'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ds = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: weight of class  object_kind */
static int tolua_get_object_kind_weight(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'weight'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->weight);
 return 1;
}

/* set function: weight of class  object_kind */
static int tolua_set_object_kind_weight(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'weight'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->weight = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: cost of class  object_kind */
static int tolua_get_object_kind_cost(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cost'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->cost);
 return 1;
}

/* set function: cost of class  object_kind */
static int tolua_set_object_kind_cost(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cost'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->cost = ((s32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flags1 of class  object_kind */
static int tolua_get_object_kind_flags1(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags1'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags1);
 return 1;
}

/* set function: flags1 of class  object_kind */
static int tolua_set_object_kind_flags1(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags1'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags1 = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flags2 of class  object_kind */
static int tolua_get_object_kind_flags2(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags2'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags2);
 return 1;
}

/* set function: flags2 of class  object_kind */
static int tolua_set_object_kind_flags2(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags2'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags2 = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flags3 of class  object_kind */
static int tolua_get_object_kind_flags3(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags3'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags3);
 return 1;
}

/* set function: flags3 of class  object_kind */
static int tolua_set_object_kind_flags3(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags3'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags3 = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: locale of class  object_kind */
static int tolua_get_object_object_kind_locale(lua_State* tolua_S)
{
 int tolua_index;
  object_kind* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (object_kind*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=4)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->locale[tolua_index]);
 return 1;
}

/* set function: locale of class  object_kind */
static int tolua_set_object_object_kind_locale(lua_State* tolua_S)
{
 int tolua_index;
  object_kind* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (object_kind*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=4)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->locale[tolua_index] = ((byte)  tolua_tonumber(tolua_S,3,0));
 return 0;
}

/* get function: chance of class  object_kind */
static int tolua_get_object_object_kind_chance(lua_State* tolua_S)
{
 int tolua_index;
  object_kind* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (object_kind*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=4)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->chance[tolua_index]);
 return 1;
}

/* set function: chance of class  object_kind */
static int tolua_set_object_object_kind_chance(lua_State* tolua_S)
{
 int tolua_index;
  object_kind* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (object_kind*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=4)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->chance[tolua_index] = ((byte)  tolua_tonumber(tolua_S,3,0));
 return 0;
}

/* get function: level of class  object_kind */
static int tolua_get_object_kind_level(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'level'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->level);
 return 1;
}

/* set function: level of class  object_kind */
static int tolua_set_object_kind_level(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'level'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->level = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: extra of class  object_kind */
static int tolua_get_object_kind_extra(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'extra'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->extra);
 return 1;
}

/* set function: extra of class  object_kind */
static int tolua_set_object_kind_extra(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'extra'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->extra = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: d_attr of class  object_kind */
static int tolua_get_object_kind_d_attr(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'd_attr'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->d_attr);
 return 1;
}

/* set function: d_attr of class  object_kind */
static int tolua_set_object_kind_d_attr(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'd_attr'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->d_attr = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: d_char of class  object_kind */
static int tolua_get_object_kind_d_char(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'd_char'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->d_char);
 return 1;
}

/* set function: d_char of class  object_kind */
static int tolua_set_object_kind_d_char(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'd_char'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->d_char = ((char)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: x_attr of class  object_kind */
static int tolua_get_object_kind_x_attr(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_attr'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->x_attr);
 return 1;
}

/* set function: x_attr of class  object_kind */
static int tolua_set_object_kind_x_attr(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_attr'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->x_attr = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: x_char of class  object_kind */
static int tolua_get_object_kind_x_char(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_char'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->x_char);
 return 1;
}

/* set function: x_char of class  object_kind */
static int tolua_set_object_kind_x_char(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_char'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->x_char = ((char)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flavor of class  object_kind */
static int tolua_get_object_kind_flavor(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flavor'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flavor);
 return 1;
}

/* set function: flavor of class  object_kind */
static int tolua_set_object_kind_flavor(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flavor'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flavor = ((u16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: aware of class  object_kind */
static int tolua_get_object_kind_aware(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'aware'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->aware);
 return 1;
}

/* set function: aware of class  object_kind */
static int tolua_set_object_kind_aware(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'aware'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->aware = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: tried of class  object_kind */
static int tolua_get_object_kind_tried(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'tried'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->tried);
 return 1;
}

/* set function: tried of class  object_kind */
static int tolua_set_object_kind_tried(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'tried'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->tried = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: name of class  artifact_type */
static int tolua_get_artifact_type_name(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'name'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->name);
 return 1;
}

/* set function: name of class  artifact_type */
static int tolua_set_artifact_type_name(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'name'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->name = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: text of class  artifact_type */
static int tolua_get_artifact_type_text(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'text'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->text);
 return 1;
}

/* set function: text of class  artifact_type */
static int tolua_set_artifact_type_text(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'text'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->text = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: tval of class  artifact_type */
static int tolua_get_artifact_type_tval(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'tval'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->tval);
 return 1;
}

/* set function: tval of class  artifact_type */
static int tolua_set_artifact_type_tval(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'tval'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->tval = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: sval of class  artifact_type */
static int tolua_get_artifact_type_sval(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sval'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->sval);
 return 1;
}

/* set function: sval of class  artifact_type */
static int tolua_set_artifact_type_sval(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sval'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->sval = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: pval of class  artifact_type */
static int tolua_get_artifact_type_pval(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'pval'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->pval);
 return 1;
}

/* set function: pval of class  artifact_type */
static int tolua_set_artifact_type_pval(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'pval'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->pval = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: to_h of class  artifact_type */
static int tolua_get_artifact_type_to_h(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'to_h'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->to_h);
 return 1;
}

/* set function: to_h of class  artifact_type */
static int tolua_set_artifact_type_to_h(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'to_h'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->to_h = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: to_d of class  artifact_type */
static int tolua_get_artifact_type_to_d(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'to_d'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->to_d);
 return 1;
}

/* set function: to_d of class  artifact_type */
static int tolua_set_artifact_type_to_d(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'to_d'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->to_d = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: to_a of class  artifact_type */
static int tolua_get_artifact_type_to_a(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'to_a'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->to_a);
 return 1;
}

/* set function: to_a of class  artifact_type */
static int tolua_set_artifact_type_to_a(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'to_a'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->to_a = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: ac of class  artifact_type */
static int tolua_get_artifact_type_ac(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ac'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->ac);
 return 1;
}

/* set function: ac of class  artifact_type */
static int tolua_set_artifact_type_ac(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ac'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ac = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: dd of class  artifact_type */
static int tolua_get_artifact_type_dd(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'dd'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->dd);
 return 1;
}

/* set function: dd of class  artifact_type */
static int tolua_set_artifact_type_dd(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'dd'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->dd = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: ds of class  artifact_type */
static int tolua_get_artifact_type_ds(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ds'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->ds);
 return 1;
}

/* set function: ds of class  artifact_type */
static int tolua_set_artifact_type_ds(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ds'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ds = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: weight of class  artifact_type */
static int tolua_get_artifact_type_weight(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'weight'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->weight);
 return 1;
}

/* set function: weight of class  artifact_type */
static int tolua_set_artifact_type_weight(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'weight'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->weight = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: cost of class  artifact_type */
static int tolua_get_artifact_type_cost(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cost'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->cost);
 return 1;
}

/* set function: cost of class  artifact_type */
static int tolua_set_artifact_type_cost(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cost'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->cost = ((s32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flags1 of class  artifact_type */
static int tolua_get_artifact_type_flags1(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags1'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags1);
 return 1;
}

/* set function: flags1 of class  artifact_type */
static int tolua_set_artifact_type_flags1(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags1'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags1 = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flags2 of class  artifact_type */
static int tolua_get_artifact_type_flags2(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags2'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags2);
 return 1;
}

/* set function: flags2 of class  artifact_type */
static int tolua_set_artifact_type_flags2(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags2'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags2 = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flags3 of class  artifact_type */
static int tolua_get_artifact_type_flags3(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags3'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags3);
 return 1;
}

/* set function: flags3 of class  artifact_type */
static int tolua_set_artifact_type_flags3(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags3'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags3 = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: level of class  artifact_type */
static int tolua_get_artifact_type_level(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'level'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->level);
 return 1;
}

/* set function: level of class  artifact_type */
static int tolua_set_artifact_type_level(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'level'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->level = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: rarity of class  artifact_type */
static int tolua_get_artifact_type_rarity(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'rarity'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->rarity);
 return 1;
}

/* set function: rarity of class  artifact_type */
static int tolua_set_artifact_type_rarity(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'rarity'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->rarity = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: cur_num of class  artifact_type */
static int tolua_get_artifact_type_cur_num(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cur_num'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->cur_num);
 return 1;
}

/* set function: cur_num of class  artifact_type */
static int tolua_set_artifact_type_cur_num(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cur_num'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->cur_num = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: max_num of class  artifact_type */
static int tolua_get_artifact_type_max_num(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'max_num'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->max_num);
 return 1;
}

/* set function: max_num of class  artifact_type */
static int tolua_set_artifact_type_max_num(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'max_num'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->max_num = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: activation of class  artifact_type */
static int tolua_get_artifact_type_activation(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'activation'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->activation);
 return 1;
}

/* set function: activation of class  artifact_type */
static int tolua_set_artifact_type_activation(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'activation'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->activation = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: time of class  artifact_type */
static int tolua_get_artifact_type_time(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'time'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->time);
 return 1;
}

/* set function: time of class  artifact_type */
static int tolua_set_artifact_type_time(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'time'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->time = ((u16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: randtime of class  artifact_type */
static int tolua_get_artifact_type_randtime(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'randtime'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->randtime);
 return 1;
}

/* set function: randtime of class  artifact_type */
static int tolua_set_artifact_type_randtime(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'randtime'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->randtime = ((u16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: name of class  ego_item_type */
static int tolua_get_ego_item_type_name(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'name'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->name);
 return 1;
}

/* set function: name of class  ego_item_type */
static int tolua_set_ego_item_type_name(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'name'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->name = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: text of class  ego_item_type */
static int tolua_get_ego_item_type_text(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'text'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->text);
 return 1;
}

/* set function: text of class  ego_item_type */
static int tolua_set_ego_item_type_text(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'text'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->text = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: cost of class  ego_item_type */
static int tolua_get_ego_item_type_cost(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cost'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->cost);
 return 1;
}

/* set function: cost of class  ego_item_type */
static int tolua_set_ego_item_type_cost(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cost'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->cost = ((s32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flags1 of class  ego_item_type */
static int tolua_get_ego_item_type_flags1(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags1'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags1);
 return 1;
}

/* set function: flags1 of class  ego_item_type */
static int tolua_set_ego_item_type_flags1(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags1'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags1 = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flags2 of class  ego_item_type */
static int tolua_get_ego_item_type_flags2(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags2'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags2);
 return 1;
}

/* set function: flags2 of class  ego_item_type */
static int tolua_set_ego_item_type_flags2(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags2'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags2 = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flags3 of class  ego_item_type */
static int tolua_get_ego_item_type_flags3(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags3'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags3);
 return 1;
}

/* set function: flags3 of class  ego_item_type */
static int tolua_set_ego_item_type_flags3(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags3'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags3 = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: level of class  ego_item_type */
static int tolua_get_ego_item_type_level(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'level'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->level);
 return 1;
}

/* set function: level of class  ego_item_type */
static int tolua_set_ego_item_type_level(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'level'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->level = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: rarity of class  ego_item_type */
static int tolua_get_ego_item_type_rarity(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'rarity'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->rarity);
 return 1;
}

/* set function: rarity of class  ego_item_type */
static int tolua_set_ego_item_type_rarity(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'rarity'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->rarity = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: rating of class  ego_item_type */
static int tolua_get_ego_item_type_rating(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'rating'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->rating);
 return 1;
}

/* set function: rating of class  ego_item_type */
static int tolua_set_ego_item_type_rating(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'rating'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->rating = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: tval of class  ego_item_type */
static int tolua_get_object_ego_item_type_tval(lua_State* tolua_S)
{
 int tolua_index;
  ego_item_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (ego_item_type*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=EGO_TVALS_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->tval[tolua_index]);
 return 1;
}

/* set function: tval of class  ego_item_type */
static int tolua_set_object_ego_item_type_tval(lua_State* tolua_S)
{
 int tolua_index;
  ego_item_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (ego_item_type*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=EGO_TVALS_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->tval[tolua_index] = ((byte)  tolua_tonumber(tolua_S,3,0));
 return 0;
}

/* get function: min_sval of class  ego_item_type */
static int tolua_get_object_ego_item_type_min_sval(lua_State* tolua_S)
{
 int tolua_index;
  ego_item_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (ego_item_type*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=EGO_TVALS_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->min_sval[tolua_index]);
 return 1;
}

/* set function: min_sval of class  ego_item_type */
static int tolua_set_object_ego_item_type_min_sval(lua_State* tolua_S)
{
 int tolua_index;
  ego_item_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (ego_item_type*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=EGO_TVALS_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->min_sval[tolua_index] = ((byte)  tolua_tonumber(tolua_S,3,0));
 return 0;
}

/* get function: max_sval of class  ego_item_type */
static int tolua_get_object_ego_item_type_max_sval(lua_State* tolua_S)
{
 int tolua_index;
  ego_item_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (ego_item_type*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=EGO_TVALS_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->max_sval[tolua_index]);
 return 1;
}

/* set function: max_sval of class  ego_item_type */
static int tolua_set_object_ego_item_type_max_sval(lua_State* tolua_S)
{
 int tolua_index;
  ego_item_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (ego_item_type*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=EGO_TVALS_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->max_sval[tolua_index] = ((byte)  tolua_tonumber(tolua_S,3,0));
 return 0;
}

/* get function: max_to_h of class  ego_item_type */
static int tolua_get_ego_item_type_max_to_h(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'max_to_h'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->max_to_h);
 return 1;
}

/* set function: max_to_h of class  ego_item_type */
static int tolua_set_ego_item_type_max_to_h(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'max_to_h'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->max_to_h = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: max_to_d of class  ego_item_type */
static int tolua_get_ego_item_type_max_to_d(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'max_to_d'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->max_to_d);
 return 1;
}

/* set function: max_to_d of class  ego_item_type */
static int tolua_set_ego_item_type_max_to_d(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'max_to_d'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->max_to_d = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: max_to_a of class  ego_item_type */
static int tolua_get_ego_item_type_max_to_a(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'max_to_a'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->max_to_a);
 return 1;
}

/* set function: max_to_a of class  ego_item_type */
static int tolua_set_ego_item_type_max_to_a(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'max_to_a'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->max_to_a = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: max_pval of class  ego_item_type */
static int tolua_get_ego_item_type_max_pval(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'max_pval'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->max_pval);
 return 1;
}

/* set function: max_pval of class  ego_item_type */
static int tolua_set_ego_item_type_max_pval(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'max_pval'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->max_pval = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: xtra of class  ego_item_type */
static int tolua_get_ego_item_type_xtra(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'xtra'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->xtra);
 return 1;
}

/* set function: xtra of class  ego_item_type */
static int tolua_set_ego_item_type_xtra(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'xtra'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->xtra = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: text of class  flavor_type */
static int tolua_get_flavor_type_text(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'text'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->text);
 return 1;
}

/* set function: text of class  flavor_type */
static int tolua_set_flavor_type_text(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'text'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->text = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: tval of class  flavor_type */
static int tolua_get_flavor_type_tval(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'tval'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->tval);
 return 1;
}

/* set function: tval of class  flavor_type */
static int tolua_set_flavor_type_tval(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'tval'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->tval = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: sval of class  flavor_type */
static int tolua_get_flavor_type_sval(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sval'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->sval);
 return 1;
}

/* set function: sval of class  flavor_type */
static int tolua_set_flavor_type_sval(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sval'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->sval = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: d_attr of class  flavor_type */
static int tolua_get_flavor_type_d_attr(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'd_attr'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->d_attr);
 return 1;
}

/* set function: d_attr of class  flavor_type */
static int tolua_set_flavor_type_d_attr(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'd_attr'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->d_attr = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: d_char of class  flavor_type */
static int tolua_get_flavor_type_d_char(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'd_char'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->d_char);
 return 1;
}

/* set function: d_char of class  flavor_type */
static int tolua_set_flavor_type_d_char(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'd_char'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->d_char = ((char)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: x_attr of class  flavor_type */
static int tolua_get_flavor_type_x_attr(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_attr'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->x_attr);
 return 1;
}

/* set function: x_attr of class  flavor_type */
static int tolua_set_flavor_type_x_attr(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_attr'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->x_attr = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: x_char of class  flavor_type */
static int tolua_get_flavor_type_x_char(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_char'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->x_char);
 return 1;
}

/* set function: x_char of class  flavor_type */
static int tolua_set_flavor_type_x_char(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_char'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->x_char = ((char)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: o_max */
static int tolua_get_o_max(lua_State* tolua_S)
{
 tolua_pushnumber(tolua_S,(long)o_max);
 return 1;
}

/* set function: o_max */
static int tolua_set_o_max(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  o_max = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: o_cnt */
static int tolua_get_o_cnt(lua_State* tolua_S)
{
 tolua_pushnumber(tolua_S,(long)o_cnt);
 return 1;
}

/* set function: o_cnt */
static int tolua_set_o_cnt(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  o_cnt = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: o_list */
static int tolua_get_object_o_list(lua_State* tolua_S)
{
 int tolua_index;
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=o_max)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushusertype(tolua_S,(void*)&o_list[tolua_index],"object_type");
 return 1;
}

/* set function: o_list */
static int tolua_set_object_o_list(lua_State* tolua_S)
{
 int tolua_index;
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=o_max)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  o_list[tolua_index] = *((object_type*)  tolua_tousertype(tolua_S,3,0));
 return 0;
}

/* get function: k_info */
static int tolua_get_object_k_info(lua_State* tolua_S)
{
 int tolua_index;
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=z_info->k_max)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushusertype(tolua_S,(void*)&k_info[tolua_index],"object_kind");
 return 1;
}

/* set function: k_info */
static int tolua_set_object_k_info(lua_State* tolua_S)
{
 int tolua_index;
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=z_info->k_max)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  k_info[tolua_index] = *((object_kind*)  tolua_tousertype(tolua_S,3,0));
 return 0;
}

/* get function: k_name */
static int tolua_get_k_name(lua_State* tolua_S)
{
 tolua_pushstring(tolua_S,(const char*)k_name);
 return 1;
}

/* set function: k_name */
static int tolua_set_k_name(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!tolua_isstring(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  k_name = ((char*)  tolua_tostring(tolua_S,2,0));
 return 0;
}

/* get function: k_text */
static int tolua_get_k_text(lua_State* tolua_S)
{
 tolua_pushstring(tolua_S,(const char*)k_text);
 return 1;
}

/* set function: k_text */
static int tolua_set_k_text(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!tolua_isstring(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  k_text = ((char*)  tolua_tostring(tolua_S,2,0));
 return 0;
}

/* get function: a_info */
static int tolua_get_object_a_info(lua_State* tolua_S)
{
 int tolua_index;
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=z_info->a_max)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushusertype(tolua_S,(void*)&a_info[tolua_index],"artifact_type");
 return 1;
}

/* set function: a_info */
static int tolua_set_object_a_info(lua_State* tolua_S)
{
 int tolua_index;
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=z_info->a_max)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  a_info[tolua_index] = *((artifact_type*)  tolua_tousertype(tolua_S,3,0));
 return 0;
}

/* get function: a_name */
static int tolua_get_a_name(lua_State* tolua_S)
{
 tolua_pushstring(tolua_S,(const char*)a_name);
 return 1;
}

/* set function: a_name */
static int tolua_set_a_name(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!tolua_isstring(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  a_name = ((char*)  tolua_tostring(tolua_S,2,0));
 return 0;
}

/* get function: a_text */
static int tolua_get_a_text(lua_State* tolua_S)
{
 tolua_pushstring(tolua_S,(const char*)a_text);
 return 1;
}

/* set function: a_text */
static int tolua_set_a_text(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!tolua_isstring(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  a_text = ((char*)  tolua_tostring(tolua_S,2,0));
 return 0;
}

/* get function: e_info */
static int tolua_get_object_e_info(lua_State* tolua_S)
{
 int tolua_index;
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=z_info->e_max)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushusertype(tolua_S,(void*)&e_info[tolua_index],"ego_item_type");
 return 1;
}

/* set function: e_info */
static int tolua_set_object_e_info(lua_State* tolua_S)
{
 int tolua_index;
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=z_info->e_max)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  e_info[tolua_index] = *((ego_item_type*)  tolua_tousertype(tolua_S,3,0));
 return 0;
}

/* get function: e_name */
static int tolua_get_e_name(lua_State* tolua_S)
{
 tolua_pushstring(tolua_S,(const char*)e_name);
 return 1;
}

/* set function: e_name */
static int tolua_set_e_name(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!tolua_isstring(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  e_name = ((char*)  tolua_tostring(tolua_S,2,0));
 return 0;
}

/* get function: e_text */
static int tolua_get_e_text(lua_State* tolua_S)
{
 tolua_pushstring(tolua_S,(const char*)e_text);
 return 1;
}

/* set function: e_text */
static int tolua_set_e_text(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!tolua_isstring(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  e_text = ((char*)  tolua_tostring(tolua_S,2,0));
 return 0;
}

/* get function: flavor_info */
static int tolua_get_object_flavor_info(lua_State* tolua_S)
{
 int tolua_index;
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=z_info->flavor_max)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushusertype(tolua_S,(void*)&flavor_info[tolua_index],"flavor_type");
 return 1;
}

/* set function: flavor_info */
static int tolua_set_object_flavor_info(lua_State* tolua_S)
{
 int tolua_index;
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=z_info->flavor_max)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  flavor_info[tolua_index] = *((flavor_type*)  tolua_tousertype(tolua_S,3,0));
 return 0;
}

/* get function: flavor_name */
static int tolua_get_flavor_name(lua_State* tolua_S)
{
 tolua_pushstring(tolua_S,(const char*)flavor_name);
 return 1;
}

/* set function: flavor_name */
static int tolua_set_flavor_name(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!tolua_isstring(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  flavor_name = ((char*)  tolua_tostring(tolua_S,2,0));
 return 0;
}

/* get function: flavor_text */
static int tolua_get_flavor_text(lua_State* tolua_S)
{
 tolua_pushstring(tolua_S,(const char*)flavor_text);
 return 1;
}

/* set function: flavor_text */
static int tolua_set_flavor_text(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!tolua_isstring(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  flavor_text = ((char*)  tolua_tostring(tolua_S,2,0));
 return 0;
}

/* function: flavor_init */
static int tolua_object_flavor_init00(lua_State* tolua_S)
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
  flavor_init();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'flavor_init'.",&tolua_err);
 return 0;
#endif
}

/* function: reset_visuals */
static int tolua_object_reset_visuals00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isboolean(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  bool prefs = ((bool)  tolua_toboolean(tolua_S,1,0));
 {
  reset_visuals(prefs);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'reset_visuals'.",&tolua_err);
 return 0;
#endif
}

/* function: identify_random_gen */
static int tolua_object_identify_random_gen00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const object_type* o_ptr = ((const object_type*)  tolua_tousertype(tolua_S,1,0));
 {
  identify_random_gen(o_ptr);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'identify_random_gen'.",&tolua_err);
 return 0;
#endif
}

/* function: index_to_label */
static int tolua_object_index_to_label00(lua_State* tolua_S)
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
  int i = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  char tolua_ret = (char)  index_to_label(i);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'index_to_label'.",&tolua_err);
 return 0;
#endif
}

/* function: label_to_inven */
static int tolua_object_label_to_inven00(lua_State* tolua_S)
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
  int c = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  s16b tolua_ret = (s16b)  label_to_inven(c);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'label_to_inven'.",&tolua_err);
 return 0;
#endif
}

/* function: label_to_equip */
static int tolua_object_label_to_equip00(lua_State* tolua_S)
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
  int c = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  s16b tolua_ret = (s16b)  label_to_equip(c);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'label_to_equip'.",&tolua_err);
 return 0;
#endif
}

/* function: wield_slot */
static int tolua_object_wield_slot00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const object_type* o_ptr = ((const object_type*)  tolua_tousertype(tolua_S,1,0));
 {
  s16b tolua_ret = (s16b)  wield_slot(o_ptr);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'wield_slot'.",&tolua_err);
 return 0;
#endif
}

/* function: mention_use */
static int tolua_object_mention_use00(lua_State* tolua_S)
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
  int i = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  cptr tolua_ret = (cptr)  mention_use(i);
 tolua_pushstring(tolua_S,(const char*)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'mention_use'.",&tolua_err);
 return 0;
#endif
}

/* function: describe_use */
static int tolua_object_describe_use00(lua_State* tolua_S)
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
  int i = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  cptr tolua_ret = (cptr)  describe_use(i);
 tolua_pushstring(tolua_S,(const char*)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'describe_use'.",&tolua_err);
 return 0;
#endif
}

/* function: item_tester_okay */
static int tolua_object_item_tester_okay00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const object_type* o_ptr = ((const object_type*)  tolua_tousertype(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  item_tester_okay(o_ptr);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'item_tester_okay'.",&tolua_err);
 return 0;
#endif
}

/* function: scan_floor */
static int tolua_object_scan_floor00(lua_State* tolua_S)
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
  int items = ((int)  tolua_tonumber(tolua_S,1,0));
  int size = ((int)  tolua_tonumber(tolua_S,2,0));
  int y = ((int)  tolua_tonumber(tolua_S,3,0));
  int x = ((int)  tolua_tonumber(tolua_S,4,0));
  int mode = ((int)  tolua_tonumber(tolua_S,5,0));
 {
  int tolua_ret = (int)  scan_floor(&items,size,y,x,mode);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 tolua_pushnumber(tolua_S,(long)items);
 }
 }
 return 2;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'scan_floor'.",&tolua_err);
 return 0;
#endif
}

/* function: display_inven */
static int tolua_object_display_inven00(lua_State* tolua_S)
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
  display_inven();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'display_inven'.",&tolua_err);
 return 0;
#endif
}

/* function: display_equip */
static int tolua_object_display_equip00(lua_State* tolua_S)
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
  display_equip();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'display_equip'.",&tolua_err);
 return 0;
#endif
}

/* function: show_inven */
static int tolua_object_show_inven00(lua_State* tolua_S)
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
  show_inven();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'show_inven'.",&tolua_err);
 return 0;
#endif
}

/* function: show_equip */
static int tolua_object_show_equip00(lua_State* tolua_S)
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
  show_equip();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'show_equip'.",&tolua_err);
 return 0;
#endif
}

/* function: toggle_inven_equip */
static int tolua_object_toggle_inven_equip00(lua_State* tolua_S)
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
  toggle_inven_equip();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'toggle_inven_equip'.",&tolua_err);
 return 0;
#endif
}

/* function: get_item */
static int tolua_object_get_item00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isstring(tolua_S,3,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int cp = ((int)  tolua_tonumber(tolua_S,1,0));
  cptr pmt = ((cptr)  tolua_tostring(tolua_S,2,0));
  cptr str = ((cptr)  tolua_tostring(tolua_S,3,0));
  int mode = ((int)  tolua_tonumber(tolua_S,4,0));
 {
  bool tolua_ret = (bool)  get_item(&cp,pmt,str,mode);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 tolua_pushnumber(tolua_S,(long)cp);
 }
 }
 return 2;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'get_item'.",&tolua_err);
 return 0;
#endif
}

/* function: excise_object_idx */
static int tolua_object_excise_object_idx00(lua_State* tolua_S)
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
  int o_idx = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  excise_object_idx(o_idx);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'excise_object_idx'.",&tolua_err);
 return 0;
#endif
}

/* function: delete_object_idx */
static int tolua_object_delete_object_idx00(lua_State* tolua_S)
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
  int o_idx = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  delete_object_idx(o_idx);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete_object_idx'.",&tolua_err);
 return 0;
#endif
}

/* function: delete_object */
static int tolua_object_delete_object00(lua_State* tolua_S)
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
  int y = ((int)  tolua_tonumber(tolua_S,1,0));
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  delete_object(y,x);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete_object'.",&tolua_err);
 return 0;
#endif
}

/* function: compact_objects */
static int tolua_object_compact_objects00(lua_State* tolua_S)
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
  int size = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  compact_objects(size);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'compact_objects'.",&tolua_err);
 return 0;
#endif
}

/* function: wipe_o_list */
static int tolua_object_wipe_o_list00(lua_State* tolua_S)
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
  wipe_o_list();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'wipe_o_list'.",&tolua_err);
 return 0;
#endif
}

/* function: o_pop */
static int tolua_object_o_pop00(lua_State* tolua_S)
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
  s16b tolua_ret = (s16b)  o_pop();
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'o_pop'.",&tolua_err);
 return 0;
#endif
}

/* function: get_obj_num_prep */
static int tolua_object_get_obj_num_prep00(lua_State* tolua_S)
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
  errr tolua_ret = (errr)  get_obj_num_prep();
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'get_obj_num_prep'.",&tolua_err);
 return 0;
#endif
}

/* function: get_obj_num */
static int tolua_object_get_obj_num00(lua_State* tolua_S)
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
  int level = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  s16b tolua_ret = (s16b)  get_obj_num(level);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'get_obj_num'.",&tolua_err);
 return 0;
#endif
}

/* function: object_known */
static int tolua_object_object_known00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  object_type* o_ptr = ((object_type*)  tolua_tousertype(tolua_S,1,0));
 {
  object_known(o_ptr);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_known'.",&tolua_err);
 return 0;
#endif
}

/* function: object_aware */
static int tolua_object_object_aware00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  object_type* o_ptr = ((object_type*)  tolua_tousertype(tolua_S,1,0));
 {
  object_aware(o_ptr);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_aware'.",&tolua_err);
 return 0;
#endif
}

/* function: object_tried */
static int tolua_object_object_tried00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  object_type* o_ptr = ((object_type*)  tolua_tousertype(tolua_S,1,0));
 {
  object_tried(o_ptr);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_tried'.",&tolua_err);
 return 0;
#endif
}

/* function: is_blessed */
static int tolua_object_is_blessed00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const object_type* o_ptr = ((const object_type*)  tolua_tousertype(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  is_blessed(o_ptr);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'is_blessed'.",&tolua_err);
 return 0;
#endif
}

/* function: object_value */
static int tolua_object_object_value00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const object_type* o_ptr = ((const object_type*)  tolua_tousertype(tolua_S,1,0));
 {
  s32b tolua_ret = (s32b)  object_value(o_ptr);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_value'.",&tolua_err);
 return 0;
#endif
}

/* function: object_similar */
static int tolua_object_object_similar00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const object_type",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"const object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const object_type* o_ptr = ((const object_type*)  tolua_tousertype(tolua_S,1,0));
  const object_type* j_ptr = ((const object_type*)  tolua_tousertype(tolua_S,2,0));
 {
  bool tolua_ret = (bool)  object_similar(o_ptr,j_ptr);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_similar'.",&tolua_err);
 return 0;
#endif
}

/* function: object_absorb */
static int tolua_object_object_absorb00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"object_type",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"const object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  object_type* o_ptr = ((object_type*)  tolua_tousertype(tolua_S,1,0));
  const object_type* j_ptr = ((const object_type*)  tolua_tousertype(tolua_S,2,0));
 {
  object_absorb(o_ptr,j_ptr);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_absorb'.",&tolua_err);
 return 0;
#endif
}

/* function: lookup_kind */
static int tolua_object_lookup_kind00(lua_State* tolua_S)
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
  int tval = ((int)  tolua_tonumber(tolua_S,1,0));
  int sval = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  s16b tolua_ret = (s16b)  lookup_kind(tval,sval);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'lookup_kind'.",&tolua_err);
 return 0;
#endif
}

/* function: object_wipe */
static int tolua_object_object_wipe00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  object_type* o_ptr = ((object_type*)  tolua_tousertype(tolua_S,1,0));
 {
  object_wipe(o_ptr);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_wipe'.",&tolua_err);
 return 0;
#endif
}

/* function: object_copy */
static int tolua_object_object_copy00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"object_type",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"const object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  object_type* o_ptr = ((object_type*)  tolua_tousertype(tolua_S,1,0));
  const object_type* j_ptr = ((const object_type*)  tolua_tousertype(tolua_S,2,0));
 {
  object_copy(o_ptr,j_ptr);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_copy'.",&tolua_err);
 return 0;
#endif
}

/* function: object_prep */
static int tolua_object_object_prep00(lua_State* tolua_S)
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
  int k_idx = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  object_prep(o_ptr,k_idx);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_prep'.",&tolua_err);
 return 0;
#endif
}

/* function: apply_magic */
static int tolua_object_apply_magic00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"object_type",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isboolean(tolua_S,3,0,&tolua_err) ||
 !tolua_isboolean(tolua_S,4,0,&tolua_err) ||
 !tolua_isboolean(tolua_S,5,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,6,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  object_type* o_ptr = ((object_type*)  tolua_tousertype(tolua_S,1,0));
  int lev = ((int)  tolua_tonumber(tolua_S,2,0));
  bool okay = ((bool)  tolua_toboolean(tolua_S,3,0));
  bool good = ((bool)  tolua_toboolean(tolua_S,4,0));
  bool great = ((bool)  tolua_toboolean(tolua_S,5,0));
 {
  apply_magic(o_ptr,lev,okay,good,great);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'apply_magic'.",&tolua_err);
 return 0;
#endif
}

/* function: make_object */
static int tolua_object_make_object00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"object_type",0,&tolua_err) ||
 !tolua_isboolean(tolua_S,2,0,&tolua_err) ||
 !tolua_isboolean(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  object_type* j_ptr = ((object_type*)  tolua_tousertype(tolua_S,1,0));
  bool good = ((bool)  tolua_toboolean(tolua_S,2,0));
  bool great = ((bool)  tolua_toboolean(tolua_S,3,0));
 {
  bool tolua_ret = (bool)  make_object(j_ptr,good,great);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'make_object'.",&tolua_err);
 return 0;
#endif
}

/* function: make_gold */
static int tolua_object_make_gold00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  object_type* j_ptr = ((object_type*)  tolua_tousertype(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  make_gold(j_ptr);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'make_gold'.",&tolua_err);
 return 0;
#endif
}

/* function: floor_carry */
static int tolua_object_floor_carry00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isusertype(tolua_S,3,"object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int y = ((int)  tolua_tonumber(tolua_S,1,0));
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
  object_type* j_ptr = ((object_type*)  tolua_tousertype(tolua_S,3,0));
 {
  s16b tolua_ret = (s16b)  floor_carry(y,x,j_ptr);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'floor_carry'.",&tolua_err);
 return 0;
#endif
}

/* function: drop_near */
static int tolua_object_drop_near00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"object_type",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  object_type* j_ptr = ((object_type*)  tolua_tousertype(tolua_S,1,0));
  int chance = ((int)  tolua_tonumber(tolua_S,2,0));
  int y = ((int)  tolua_tonumber(tolua_S,3,0));
  int x = ((int)  tolua_tonumber(tolua_S,4,0));
 {
  drop_near(j_ptr,chance,y,x);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'drop_near'.",&tolua_err);
 return 0;
#endif
}

/* function: acquirement */
static int tolua_object_acquirement00(lua_State* tolua_S)
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
  int num = ((int)  tolua_tonumber(tolua_S,3,0));
  bool great = ((bool)  tolua_toboolean(tolua_S,4,0));
 {
  acquirement(y1,x1,num,great);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'acquirement'.",&tolua_err);
 return 0;
#endif
}

/* function: place_object */
static int tolua_object_place_object00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isboolean(tolua_S,3,0,&tolua_err) ||
 !tolua_isboolean(tolua_S,4,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int y = ((int)  tolua_tonumber(tolua_S,1,0));
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
  bool good = ((bool)  tolua_toboolean(tolua_S,3,0));
  bool great = ((bool)  tolua_toboolean(tolua_S,4,0));
 {
  place_object(y,x,good,great);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'place_object'.",&tolua_err);
 return 0;
#endif
}

/* function: place_gold */
static int tolua_object_place_gold00(lua_State* tolua_S)
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
  int y = ((int)  tolua_tonumber(tolua_S,1,0));
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  place_gold(y,x);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'place_gold'.",&tolua_err);
 return 0;
#endif
}

/* function: pick_trap */
static int tolua_object_pick_trap00(lua_State* tolua_S)
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
  int y = ((int)  tolua_tonumber(tolua_S,1,0));
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  pick_trap(y,x);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'pick_trap'.",&tolua_err);
 return 0;
#endif
}

/* function: place_trap */
static int tolua_object_place_trap00(lua_State* tolua_S)
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
  int y = ((int)  tolua_tonumber(tolua_S,1,0));
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  place_trap(y,x);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'place_trap'.",&tolua_err);
 return 0;
#endif
}

/* function: place_secret_door */
static int tolua_object_place_secret_door00(lua_State* tolua_S)
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
  int y = ((int)  tolua_tonumber(tolua_S,1,0));
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  place_secret_door(y,x);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'place_secret_door'.",&tolua_err);
 return 0;
#endif
}

/* function: place_closed_door */
static int tolua_object_place_closed_door00(lua_State* tolua_S)
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
  int y = ((int)  tolua_tonumber(tolua_S,1,0));
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  place_closed_door(y,x);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'place_closed_door'.",&tolua_err);
 return 0;
#endif
}

/* function: place_random_door */
static int tolua_object_place_random_door00(lua_State* tolua_S)
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
  int y = ((int)  tolua_tonumber(tolua_S,1,0));
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  place_random_door(y,x);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'place_random_door'.",&tolua_err);
 return 0;
#endif
}

/* function: inven_item_charges */
static int tolua_object_inven_item_charges00(lua_State* tolua_S)
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
  int item = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  inven_item_charges(item);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'inven_item_charges'.",&tolua_err);
 return 0;
#endif
}

/* function: inven_item_describe */
static int tolua_object_inven_item_describe00(lua_State* tolua_S)
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
  int item = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  inven_item_describe(item);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'inven_item_describe'.",&tolua_err);
 return 0;
#endif
}

/* function: inven_item_increase */
static int tolua_object_inven_item_increase00(lua_State* tolua_S)
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
  int item = ((int)  tolua_tonumber(tolua_S,1,0));
  int num = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  inven_item_increase(item,num);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'inven_item_increase'.",&tolua_err);
 return 0;
#endif
}

/* function: inven_item_optimize */
static int tolua_object_inven_item_optimize00(lua_State* tolua_S)
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
  int item = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  inven_item_optimize(item);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'inven_item_optimize'.",&tolua_err);
 return 0;
#endif
}

/* function: floor_item_charges */
static int tolua_object_floor_item_charges00(lua_State* tolua_S)
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
  int item = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  floor_item_charges(item);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'floor_item_charges'.",&tolua_err);
 return 0;
#endif
}

/* function: floor_item_describe */
static int tolua_object_floor_item_describe00(lua_State* tolua_S)
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
  int item = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  floor_item_describe(item);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'floor_item_describe'.",&tolua_err);
 return 0;
#endif
}

/* function: floor_item_increase */
static int tolua_object_floor_item_increase00(lua_State* tolua_S)
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
  int item = ((int)  tolua_tonumber(tolua_S,1,0));
  int num = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  floor_item_increase(item,num);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'floor_item_increase'.",&tolua_err);
 return 0;
#endif
}

/* function: floor_item_optimize */
static int tolua_object_floor_item_optimize00(lua_State* tolua_S)
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
  int item = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  floor_item_optimize(item);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'floor_item_optimize'.",&tolua_err);
 return 0;
#endif
}

/* function: inven_carry_okay */
static int tolua_object_inven_carry_okay00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const object_type* o_ptr = ((const object_type*)  tolua_tousertype(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  inven_carry_okay(o_ptr);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'inven_carry_okay'.",&tolua_err);
 return 0;
#endif
}

/* function: inven_carry */
static int tolua_object_inven_carry00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  object_type* o_ptr = ((object_type*)  tolua_tousertype(tolua_S,1,0));
 {
  s16b tolua_ret = (s16b)  inven_carry(o_ptr);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'inven_carry'.",&tolua_err);
 return 0;
#endif
}

/* function: inven_takeoff */
static int tolua_object_inven_takeoff00(lua_State* tolua_S)
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
  int item = ((int)  tolua_tonumber(tolua_S,1,0));
  int amt = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  s16b tolua_ret = (s16b)  inven_takeoff(item,amt);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'inven_takeoff'.",&tolua_err);
 return 0;
#endif
}

/* function: inven_drop */
static int tolua_object_inven_drop00(lua_State* tolua_S)
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
  int item = ((int)  tolua_tonumber(tolua_S,1,0));
  int amt = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  inven_drop(item,amt);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'inven_drop'.",&tolua_err);
 return 0;
#endif
}

/* function: combine_pack */
static int tolua_object_combine_pack00(lua_State* tolua_S)
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
  combine_pack();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'combine_pack'.",&tolua_err);
 return 0;
#endif
}

/* function: reorder_pack */
static int tolua_object_reorder_pack00(lua_State* tolua_S)
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
  reorder_pack();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'reorder_pack'.",&tolua_err);
 return 0;
#endif
}

/* function: object_aware_p */
static int tolua_object_object_aware_p00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const object_type* o_ptr = ((const object_type*)  tolua_tousertype(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  object_aware_p(o_ptr);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_aware_p'.",&tolua_err);
 return 0;
#endif
}

/* function: object_tried_p */
static int tolua_object_object_tried_p00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const object_type* o_ptr = ((const object_type*)  tolua_tousertype(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  object_tried_p(o_ptr);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_tried_p'.",&tolua_err);
 return 0;
#endif
}

/* function: object_known_p */
static int tolua_object_object_known_p00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const object_type* o_ptr = ((const object_type*)  tolua_tousertype(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  object_known_p(o_ptr);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_known_p'.",&tolua_err);
 return 0;
#endif
}

/* function: object_attr */
static int tolua_object_object_attr00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const object_type* o_ptr = ((const object_type*)  tolua_tousertype(tolua_S,1,0));
 {
  byte tolua_ret = (byte)  object_attr(o_ptr);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_attr'.",&tolua_err);
 return 0;
#endif
}

/* function: object_char */
static int tolua_object_object_char00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const object_type* o_ptr = ((const object_type*)  tolua_tousertype(tolua_S,1,0));
 {
  char tolua_ret = (char)  object_char(o_ptr);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_char'.",&tolua_err);
 return 0;
#endif
}

/* function: artifact_p */
static int tolua_object_artifact_p00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const object_type* o_ptr = ((const object_type*)  tolua_tousertype(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  artifact_p(o_ptr);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'artifact_p'.",&tolua_err);
 return 0;
#endif
}

/* function: ego_item_p */
static int tolua_object_ego_item_p00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const object_type* o_ptr = ((const object_type*)  tolua_tousertype(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  ego_item_p(o_ptr);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ego_item_p'.",&tolua_err);
 return 0;
#endif
}

/* function: broken_p */
static int tolua_object_broken_p00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const object_type* o_ptr = ((const object_type*)  tolua_tousertype(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  broken_p(o_ptr);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'broken_p'.",&tolua_err);
 return 0;
#endif
}

/* function: cursed_p */
static int tolua_object_cursed_p00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const object_type* o_ptr = ((const object_type*)  tolua_tousertype(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  cursed_p(o_ptr);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'cursed_p'.",&tolua_err);
 return 0;
#endif
}

/* function: object_info_out */
static int tolua_object_object_info_out00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const object_type* o_ptr = ((const object_type*)  tolua_tousertype(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  object_info_out(o_ptr);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_info_out'.",&tolua_err);
 return 0;
#endif
}

/* function: object_info_screen */
static int tolua_object_object_info_screen00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const object_type* o_ptr = ((const object_type*)  tolua_tousertype(tolua_S,1,0));
 {
  object_info_screen(o_ptr);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_info_screen'.",&tolua_err);
 return 0;
#endif
}

/* Open function */
TOLUA_API int tolua_object_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,1);
 tolua_beginmodule(tolua_S,NULL);
 tolua_constant(tolua_S,"ART_POWER",ART_POWER);
 tolua_constant(tolua_S,"ART_MORGOTH",ART_MORGOTH);
 tolua_constant(tolua_S,"ART_GROND",ART_GROND);
 tolua_constant(tolua_S,"ART_MIN_NORMAL",ART_MIN_NORMAL);
 tolua_constant(tolua_S,"EGO_TVALS_MAX",EGO_TVALS_MAX);
 tolua_constant(tolua_S,"EGO_RESIST_ACID",EGO_RESIST_ACID);
 tolua_constant(tolua_S,"EGO_RESIST_ELEC",EGO_RESIST_ELEC);
 tolua_constant(tolua_S,"EGO_RESIST_FIRE",EGO_RESIST_FIRE);
 tolua_constant(tolua_S,"EGO_RESIST_COLD",EGO_RESIST_COLD);
 tolua_constant(tolua_S,"EGO_RESISTANCE",EGO_RESISTANCE);
 tolua_constant(tolua_S,"EGO_ELVENKIND",EGO_ELVENKIND);
 tolua_constant(tolua_S,"EGO_ARMR_VULN",EGO_ARMR_VULN);
 tolua_constant(tolua_S,"EGO_PERMANENCE",EGO_PERMANENCE);
 tolua_constant(tolua_S,"EGO_ARMR_DWARVEN",EGO_ARMR_DWARVEN);
 tolua_constant(tolua_S,"EGO_ENDURE_ACID",EGO_ENDURE_ACID);
 tolua_constant(tolua_S,"EGO_ENDURE_ELEC",EGO_ENDURE_ELEC);
 tolua_constant(tolua_S,"EGO_ENDURE_FIRE",EGO_ENDURE_FIRE);
 tolua_constant(tolua_S,"EGO_ENDURE_COLD",EGO_ENDURE_COLD);
 tolua_constant(tolua_S,"EGO_ENDURANCE",EGO_ENDURANCE);
 tolua_constant(tolua_S,"EGO_SHIELD_ELVENKIND",EGO_SHIELD_ELVENKIND);
 tolua_constant(tolua_S,"EGO_SHIELD_PRESERVATION",EGO_SHIELD_PRESERVATION);
 tolua_constant(tolua_S,"EGO_SHIELD_VULN",EGO_SHIELD_VULN);
 tolua_constant(tolua_S,"EGO_INTELLIGENCE",EGO_INTELLIGENCE);
 tolua_constant(tolua_S,"EGO_WISDOM",EGO_WISDOM);
 tolua_constant(tolua_S,"EGO_BEAUTY",EGO_BEAUTY);
 tolua_constant(tolua_S,"EGO_MAGI",EGO_MAGI);
 tolua_constant(tolua_S,"EGO_MIGHT",EGO_MIGHT);
 tolua_constant(tolua_S,"EGO_LORDLINESS",EGO_LORDLINESS);
 tolua_constant(tolua_S,"EGO_SEEING",EGO_SEEING);
 tolua_constant(tolua_S,"EGO_INFRAVISION",EGO_INFRAVISION);
 tolua_constant(tolua_S,"EGO_LITE",EGO_LITE);
 tolua_constant(tolua_S,"EGO_TELEPATHY",EGO_TELEPATHY);
 tolua_constant(tolua_S,"EGO_REGENERATION",EGO_REGENERATION);
 tolua_constant(tolua_S,"EGO_TELEPORTATION",EGO_TELEPORTATION);
 tolua_constant(tolua_S,"EGO_STUPIDITY",EGO_STUPIDITY);
 tolua_constant(tolua_S,"EGO_NAIVETY",EGO_NAIVETY);
 tolua_constant(tolua_S,"EGO_UGLINESS",EGO_UGLINESS);
 tolua_constant(tolua_S,"EGO_SICKLINESS",EGO_SICKLINESS);
 tolua_constant(tolua_S,"EGO_PROTECTION",EGO_PROTECTION);
 tolua_constant(tolua_S,"EGO_STEALTH",EGO_STEALTH);
 tolua_constant(tolua_S,"EGO_AMAN",EGO_AMAN);
 tolua_constant(tolua_S,"EGO_CLOAK_MAGI",EGO_CLOAK_MAGI);
 tolua_constant(tolua_S,"EGO_ENVELOPING",EGO_ENVELOPING);
 tolua_constant(tolua_S,"EGO_VULNERABILITY",EGO_VULNERABILITY);
 tolua_constant(tolua_S,"EGO_IRRITATION",EGO_IRRITATION);
 tolua_constant(tolua_S,"EGO_FREE_ACTION",EGO_FREE_ACTION);
 tolua_constant(tolua_S,"EGO_SLAYING",EGO_SLAYING);
 tolua_constant(tolua_S,"EGO_AGILITY",EGO_AGILITY);
 tolua_constant(tolua_S,"EGO_POWER",EGO_POWER);
 tolua_constant(tolua_S,"EGO_GLOVES_THIEVERY",EGO_GLOVES_THIEVERY);
 tolua_constant(tolua_S,"EGO_GAUNTLETS_COMBAT",EGO_GAUNTLETS_COMBAT);
 tolua_constant(tolua_S,"EGO_WEAKNESS",EGO_WEAKNESS);
 tolua_constant(tolua_S,"EGO_CLUMSINESS",EGO_CLUMSINESS);
 tolua_constant(tolua_S,"EGO_SLOW_DESCENT",EGO_SLOW_DESCENT);
 tolua_constant(tolua_S,"EGO_QUIET",EGO_QUIET);
 tolua_constant(tolua_S,"EGO_MOTION",EGO_MOTION);
 tolua_constant(tolua_S,"EGO_SPEED",EGO_SPEED);
 tolua_constant(tolua_S,"EGO_STABILITY",EGO_STABILITY);
 tolua_constant(tolua_S,"EGO_NOISE",EGO_NOISE);
 tolua_constant(tolua_S,"EGO_SLOWNESS",EGO_SLOWNESS);
 tolua_constant(tolua_S,"EGO_ANNOYANCE",EGO_ANNOYANCE);
 tolua_constant(tolua_S,"EGO_HA",EGO_HA);
 tolua_constant(tolua_S,"EGO_DF",EGO_DF);
 tolua_constant(tolua_S,"EGO_BLESS_BLADE",EGO_BLESS_BLADE);
 tolua_constant(tolua_S,"EGO_GONDOLIN",EGO_GONDOLIN);
 tolua_constant(tolua_S,"EGO_WEST",EGO_WEST);
 tolua_constant(tolua_S,"EGO_ATTACKS",EGO_ATTACKS);
 tolua_constant(tolua_S,"EGO_FURY",EGO_FURY);
 tolua_constant(tolua_S,"EGO_BRAND_ACID",EGO_BRAND_ACID);
 tolua_constant(tolua_S,"EGO_BRAND_ELEC",EGO_BRAND_ELEC);
 tolua_constant(tolua_S,"EGO_BRAND_FIRE",EGO_BRAND_FIRE);
 tolua_constant(tolua_S,"EGO_BRAND_COLD",EGO_BRAND_COLD);
 tolua_constant(tolua_S,"EGO_BRAND_POIS",EGO_BRAND_POIS);
 tolua_constant(tolua_S,"EGO_SLAY_ANIMAL",EGO_SLAY_ANIMAL);
 tolua_constant(tolua_S,"EGO_SLAY_EVIL",EGO_SLAY_EVIL);
 tolua_constant(tolua_S,"EGO_SLAY_UNDEAD",EGO_SLAY_UNDEAD);
 tolua_constant(tolua_S,"EGO_SLAY_DEMON",EGO_SLAY_DEMON);
 tolua_constant(tolua_S,"EGO_SLAY_ORC",EGO_SLAY_ORC);
 tolua_constant(tolua_S,"EGO_SLAY_TROLL",EGO_SLAY_TROLL);
 tolua_constant(tolua_S,"EGO_SLAY_GIANT",EGO_SLAY_GIANT);
 tolua_constant(tolua_S,"EGO_SLAY_DRAGON",EGO_SLAY_DRAGON);
 tolua_constant(tolua_S,"EGO_KILL_ANIMAL",EGO_KILL_ANIMAL);
 tolua_constant(tolua_S,"EGO_KILL_EVIL",EGO_KILL_EVIL);
 tolua_constant(tolua_S,"EGO_KILL_UNDEAD",EGO_KILL_UNDEAD);
 tolua_constant(tolua_S,"EGO_KILL_DEMON",EGO_KILL_DEMON);
 tolua_constant(tolua_S,"EGO_KILL_ORC",EGO_KILL_ORC);
 tolua_constant(tolua_S,"EGO_KILL_TROLL",EGO_KILL_TROLL);
 tolua_constant(tolua_S,"EGO_KILL_GIANT",EGO_KILL_GIANT);
 tolua_constant(tolua_S,"EGO_KILL_DRAGON",EGO_KILL_DRAGON);
 tolua_constant(tolua_S,"EGO_DIGGING",EGO_DIGGING);
 tolua_constant(tolua_S,"EGO_DIGGER_EARTHQUAKE",EGO_DIGGER_EARTHQUAKE);
 tolua_constant(tolua_S,"EGO_MORGUL",EGO_MORGUL);
 tolua_constant(tolua_S,"EGO_ACCURACY",EGO_ACCURACY);
 tolua_constant(tolua_S,"EGO_VELOCITY",EGO_VELOCITY);
 tolua_constant(tolua_S,"EGO_BOW_LORIEN",EGO_BOW_LORIEN);
 tolua_constant(tolua_S,"EGO_CROSSBOW_HARAD",EGO_CROSSBOW_HARAD);
 tolua_constant(tolua_S,"EGO_EXTRA_MIGHT",EGO_EXTRA_MIGHT);
 tolua_constant(tolua_S,"EGO_EXTRA_SHOTS",EGO_EXTRA_SHOTS);
 tolua_constant(tolua_S,"EGO_SLING_BUCKLAND",EGO_SLING_BUCKLAND);
 tolua_constant(tolua_S,"EGO_NAZGUL",EGO_NAZGUL);
 tolua_constant(tolua_S,"EGO_HURT_ANIMAL",EGO_HURT_ANIMAL);
 tolua_constant(tolua_S,"EGO_HURT_EVIL",EGO_HURT_EVIL);
 tolua_constant(tolua_S,"EGO_HURT_UNDEAD",EGO_HURT_UNDEAD);
 tolua_constant(tolua_S,"EGO_HURT_DEMON",EGO_HURT_DEMON);
 tolua_constant(tolua_S,"EGO_HURT_ORC",EGO_HURT_ORC);
 tolua_constant(tolua_S,"EGO_HURT_TROLL",EGO_HURT_TROLL);
 tolua_constant(tolua_S,"EGO_HURT_GIANT",EGO_HURT_GIANT);
 tolua_constant(tolua_S,"EGO_HURT_DRAGON",EGO_HURT_DRAGON);
 tolua_constant(tolua_S,"EGO_AMMO_HOLY",EGO_AMMO_HOLY);
 tolua_constant(tolua_S,"EGO_AMMO_VENOM",EGO_AMMO_VENOM);
 tolua_constant(tolua_S,"EGO_FLAME",EGO_FLAME);
 tolua_constant(tolua_S,"EGO_FROST",EGO_FROST);
 tolua_constant(tolua_S,"EGO_WOUNDING",EGO_WOUNDING);
 tolua_constant(tolua_S,"EGO_BACKBITING",EGO_BACKBITING);
 tolua_constant(tolua_S,"EGO_SHATTERED",EGO_SHATTERED);
 tolua_constant(tolua_S,"EGO_BLASTED",EGO_BLASTED);
 tolua_constant(tolua_S,"TV_SKELETON",TV_SKELETON);
 tolua_constant(tolua_S,"TV_BOTTLE",TV_BOTTLE);
 tolua_constant(tolua_S,"TV_JUNK",TV_JUNK);
 tolua_constant(tolua_S,"TV_SPIKE",TV_SPIKE);
 tolua_constant(tolua_S,"TV_CHEST",TV_CHEST);
 tolua_constant(tolua_S,"TV_SHOT",TV_SHOT);
 tolua_constant(tolua_S,"TV_ARROW",TV_ARROW);
 tolua_constant(tolua_S,"TV_BOLT",TV_BOLT);
 tolua_constant(tolua_S,"TV_BOW",TV_BOW);
 tolua_constant(tolua_S,"TV_DIGGING",TV_DIGGING);
 tolua_constant(tolua_S,"TV_HAFTED",TV_HAFTED);
 tolua_constant(tolua_S,"TV_POLEARM",TV_POLEARM);
 tolua_constant(tolua_S,"TV_SWORD",TV_SWORD);
 tolua_constant(tolua_S,"TV_BOOTS",TV_BOOTS);
 tolua_constant(tolua_S,"TV_GLOVES",TV_GLOVES);
 tolua_constant(tolua_S,"TV_HELM",TV_HELM);
 tolua_constant(tolua_S,"TV_CROWN",TV_CROWN);
 tolua_constant(tolua_S,"TV_SHIELD",TV_SHIELD);
 tolua_constant(tolua_S,"TV_CLOAK",TV_CLOAK);
 tolua_constant(tolua_S,"TV_SOFT_ARMOR",TV_SOFT_ARMOR);
 tolua_constant(tolua_S,"TV_HARD_ARMOR",TV_HARD_ARMOR);
 tolua_constant(tolua_S,"TV_DRAG_ARMOR",TV_DRAG_ARMOR);
 tolua_constant(tolua_S,"TV_LITE",TV_LITE);
 tolua_constant(tolua_S,"TV_AMULET",TV_AMULET);
 tolua_constant(tolua_S,"TV_RING",TV_RING);
 tolua_constant(tolua_S,"TV_STAFF",TV_STAFF);
 tolua_constant(tolua_S,"TV_WAND",TV_WAND);
 tolua_constant(tolua_S,"TV_ROD",TV_ROD);
 tolua_constant(tolua_S,"TV_SCROLL",TV_SCROLL);
 tolua_constant(tolua_S,"TV_POTION",TV_POTION);
 tolua_constant(tolua_S,"TV_FLASK",TV_FLASK);
 tolua_constant(tolua_S,"TV_FOOD",TV_FOOD);
 tolua_constant(tolua_S,"TV_MAGIC_BOOK",TV_MAGIC_BOOK);
 tolua_constant(tolua_S,"TV_PRAYER_BOOK",TV_PRAYER_BOOK);
 tolua_constant(tolua_S,"TV_GOLD",TV_GOLD);
 tolua_constant(tolua_S,"SV_AMMO_LIGHT",SV_AMMO_LIGHT);
 tolua_constant(tolua_S,"SV_AMMO_NORMAL",SV_AMMO_NORMAL);
 tolua_constant(tolua_S,"SV_AMMO_HEAVY",SV_AMMO_HEAVY);
 tolua_constant(tolua_S,"SV_AMMO_SILVER",SV_AMMO_SILVER);
 tolua_constant(tolua_S,"SV_SLING",SV_SLING);
 tolua_constant(tolua_S,"SV_SHORT_BOW",SV_SHORT_BOW);
 tolua_constant(tolua_S,"SV_LONG_BOW",SV_LONG_BOW);
 tolua_constant(tolua_S,"SV_LIGHT_XBOW",SV_LIGHT_XBOW);
 tolua_constant(tolua_S,"SV_HEAVY_XBOW",SV_HEAVY_XBOW);
 tolua_constant(tolua_S,"SV_SHOVEL",SV_SHOVEL);
 tolua_constant(tolua_S,"SV_GNOMISH_SHOVEL",SV_GNOMISH_SHOVEL);
 tolua_constant(tolua_S,"SV_DWARVEN_SHOVEL",SV_DWARVEN_SHOVEL);
 tolua_constant(tolua_S,"SV_PICK",SV_PICK);
 tolua_constant(tolua_S,"SV_ORCISH_PICK",SV_ORCISH_PICK);
 tolua_constant(tolua_S,"SV_DWARVEN_PICK",SV_DWARVEN_PICK);
 tolua_constant(tolua_S,"SV_MATTOCK",SV_MATTOCK);
 tolua_constant(tolua_S,"SV_WHIP",SV_WHIP);
 tolua_constant(tolua_S,"SV_QUARTERSTAFF",SV_QUARTERSTAFF);
 tolua_constant(tolua_S,"SV_MACE",SV_MACE);
 tolua_constant(tolua_S,"SV_BALL_AND_CHAIN",SV_BALL_AND_CHAIN);
 tolua_constant(tolua_S,"SV_WAR_HAMMER",SV_WAR_HAMMER);
 tolua_constant(tolua_S,"SV_LUCERN_HAMMER",SV_LUCERN_HAMMER);
 tolua_constant(tolua_S,"SV_MORNING_STAR",SV_MORNING_STAR);
 tolua_constant(tolua_S,"SV_FLAIL",SV_FLAIL);
 tolua_constant(tolua_S,"SV_LEAD_FILLED_MACE",SV_LEAD_FILLED_MACE);
 tolua_constant(tolua_S,"SV_TWO_HANDED_FLAIL",SV_TWO_HANDED_FLAIL);
 tolua_constant(tolua_S,"SV_MACE_OF_DISRUPTION",SV_MACE_OF_DISRUPTION);
 tolua_constant(tolua_S,"SV_GROND",SV_GROND);
 tolua_constant(tolua_S,"SV_SPEAR",SV_SPEAR);
 tolua_constant(tolua_S,"SV_AWL_PIKE",SV_AWL_PIKE);
 tolua_constant(tolua_S,"SV_TRIDENT",SV_TRIDENT);
 tolua_constant(tolua_S,"SV_PIKE",SV_PIKE);
 tolua_constant(tolua_S,"SV_BEAKED_AXE",SV_BEAKED_AXE);
 tolua_constant(tolua_S,"SV_BROAD_AXE",SV_BROAD_AXE);
 tolua_constant(tolua_S,"SV_GLAIVE",SV_GLAIVE);
 tolua_constant(tolua_S,"SV_HALBERD",SV_HALBERD);
 tolua_constant(tolua_S,"SV_SCYTHE",SV_SCYTHE);
 tolua_constant(tolua_S,"SV_LANCE",SV_LANCE);
 tolua_constant(tolua_S,"SV_BATTLE_AXE",SV_BATTLE_AXE);
 tolua_constant(tolua_S,"SV_GREAT_AXE",SV_GREAT_AXE);
 tolua_constant(tolua_S,"SV_LOCHABER_AXE",SV_LOCHABER_AXE);
 tolua_constant(tolua_S,"SV_SCYTHE_OF_SLICING",SV_SCYTHE_OF_SLICING);
 tolua_constant(tolua_S,"SV_BROKEN_DAGGER",SV_BROKEN_DAGGER);
 tolua_constant(tolua_S,"SV_BROKEN_SWORD",SV_BROKEN_SWORD);
 tolua_constant(tolua_S,"SV_DAGGER",SV_DAGGER);
 tolua_constant(tolua_S,"SV_MAIN_GAUCHE",SV_MAIN_GAUCHE);
 tolua_constant(tolua_S,"SV_RAPIER",SV_RAPIER);
 tolua_constant(tolua_S,"SV_SMALL_SWORD",SV_SMALL_SWORD);
 tolua_constant(tolua_S,"SV_SHORT_SWORD",SV_SHORT_SWORD);
 tolua_constant(tolua_S,"SV_SABRE",SV_SABRE);
 tolua_constant(tolua_S,"SV_CUTLASS",SV_CUTLASS);
 tolua_constant(tolua_S,"SV_TULWAR",SV_TULWAR);
 tolua_constant(tolua_S,"SV_BROAD_SWORD",SV_BROAD_SWORD);
 tolua_constant(tolua_S,"SV_LONG_SWORD",SV_LONG_SWORD);
 tolua_constant(tolua_S,"SV_SCIMITAR",SV_SCIMITAR);
 tolua_constant(tolua_S,"SV_KATANA",SV_KATANA);
 tolua_constant(tolua_S,"SV_BASTARD_SWORD",SV_BASTARD_SWORD);
 tolua_constant(tolua_S,"SV_TWO_HANDED_SWORD",SV_TWO_HANDED_SWORD);
 tolua_constant(tolua_S,"SV_EXECUTIONERS_SWORD",SV_EXECUTIONERS_SWORD);
 tolua_constant(tolua_S,"SV_BLADE_OF_CHAOS",SV_BLADE_OF_CHAOS);
 tolua_constant(tolua_S,"SV_SMALL_LEATHER_SHIELD",SV_SMALL_LEATHER_SHIELD);
 tolua_constant(tolua_S,"SV_SMALL_METAL_SHIELD",SV_SMALL_METAL_SHIELD);
 tolua_constant(tolua_S,"SV_LARGE_LEATHER_SHIELD",SV_LARGE_LEATHER_SHIELD);
 tolua_constant(tolua_S,"SV_LARGE_METAL_SHIELD",SV_LARGE_METAL_SHIELD);
 tolua_constant(tolua_S,"SV_SHIELD_OF_DEFLECTION",SV_SHIELD_OF_DEFLECTION);
 tolua_constant(tolua_S,"SV_HARD_LEATHER_CAP",SV_HARD_LEATHER_CAP);
 tolua_constant(tolua_S,"SV_METAL_CAP",SV_METAL_CAP);
 tolua_constant(tolua_S,"SV_IRON_HELM",SV_IRON_HELM);
 tolua_constant(tolua_S,"SV_STEEL_HELM",SV_STEEL_HELM);
 tolua_constant(tolua_S,"SV_IRON_CROWN",SV_IRON_CROWN);
 tolua_constant(tolua_S,"SV_GOLDEN_CROWN",SV_GOLDEN_CROWN);
 tolua_constant(tolua_S,"SV_JEWELED_CROWN",SV_JEWELED_CROWN);
 tolua_constant(tolua_S,"SV_MORGOTH",SV_MORGOTH);
 tolua_constant(tolua_S,"SV_PAIR_OF_SOFT_LEATHER_BOOTS",SV_PAIR_OF_SOFT_LEATHER_BOOTS);
 tolua_constant(tolua_S,"SV_PAIR_OF_HARD_LEATHER_BOOTS",SV_PAIR_OF_HARD_LEATHER_BOOTS);
 tolua_constant(tolua_S,"SV_PAIR_OF_METAL_SHOD_BOOTS",SV_PAIR_OF_METAL_SHOD_BOOTS);
 tolua_constant(tolua_S,"SV_CLOAK",SV_CLOAK);
 tolua_constant(tolua_S,"SV_SHADOW_CLOAK",SV_SHADOW_CLOAK);
 tolua_constant(tolua_S,"SV_SET_OF_LEATHER_GLOVES",SV_SET_OF_LEATHER_GLOVES);
 tolua_constant(tolua_S,"SV_SET_OF_GAUNTLETS",SV_SET_OF_GAUNTLETS);
 tolua_constant(tolua_S,"SV_SET_OF_CESTI",SV_SET_OF_CESTI);
 tolua_constant(tolua_S,"SV_FILTHY_RAG",SV_FILTHY_RAG);
 tolua_constant(tolua_S,"SV_ROBE",SV_ROBE);
 tolua_constant(tolua_S,"SV_SOFT_LEATHER_ARMOR",SV_SOFT_LEATHER_ARMOR);
 tolua_constant(tolua_S,"SV_SOFT_STUDDED_LEATHER",SV_SOFT_STUDDED_LEATHER);
 tolua_constant(tolua_S,"SV_HARD_LEATHER_ARMOR",SV_HARD_LEATHER_ARMOR);
 tolua_constant(tolua_S,"SV_HARD_STUDDED_LEATHER",SV_HARD_STUDDED_LEATHER);
 tolua_constant(tolua_S,"SV_LEATHER_SCALE_MAIL",SV_LEATHER_SCALE_MAIL);
 tolua_constant(tolua_S,"SV_RUSTY_CHAIN_MAIL",SV_RUSTY_CHAIN_MAIL);
 tolua_constant(tolua_S,"SV_METAL_SCALE_MAIL",SV_METAL_SCALE_MAIL);
 tolua_constant(tolua_S,"SV_CHAIN_MAIL",SV_CHAIN_MAIL);
 tolua_constant(tolua_S,"SV_AUGMENTED_CHAIN_MAIL",SV_AUGMENTED_CHAIN_MAIL);
 tolua_constant(tolua_S,"SV_DOUBLE_CHAIN_MAIL",SV_DOUBLE_CHAIN_MAIL);
 tolua_constant(tolua_S,"SV_BAR_CHAIN_MAIL",SV_BAR_CHAIN_MAIL);
 tolua_constant(tolua_S,"SV_METAL_BRIGANDINE_ARMOUR",SV_METAL_BRIGANDINE_ARMOUR);
 tolua_constant(tolua_S,"SV_PARTIAL_PLATE_ARMOUR",SV_PARTIAL_PLATE_ARMOUR);
 tolua_constant(tolua_S,"SV_METAL_LAMELLAR_ARMOUR",SV_METAL_LAMELLAR_ARMOUR);
 tolua_constant(tolua_S,"SV_FULL_PLATE_ARMOUR",SV_FULL_PLATE_ARMOUR);
 tolua_constant(tolua_S,"SV_RIBBED_PLATE_ARMOUR",SV_RIBBED_PLATE_ARMOUR);
 tolua_constant(tolua_S,"SV_MITHRIL_CHAIN_MAIL",SV_MITHRIL_CHAIN_MAIL);
 tolua_constant(tolua_S,"SV_MITHRIL_PLATE_MAIL",SV_MITHRIL_PLATE_MAIL);
 tolua_constant(tolua_S,"SV_ADAMANTITE_PLATE_MAIL",SV_ADAMANTITE_PLATE_MAIL);
 tolua_constant(tolua_S,"SV_DRAGON_BLACK",SV_DRAGON_BLACK);
 tolua_constant(tolua_S,"SV_DRAGON_BLUE",SV_DRAGON_BLUE);
 tolua_constant(tolua_S,"SV_DRAGON_WHITE",SV_DRAGON_WHITE);
 tolua_constant(tolua_S,"SV_DRAGON_RED",SV_DRAGON_RED);
 tolua_constant(tolua_S,"SV_DRAGON_GREEN",SV_DRAGON_GREEN);
 tolua_constant(tolua_S,"SV_DRAGON_MULTIHUED",SV_DRAGON_MULTIHUED);
 tolua_constant(tolua_S,"SV_DRAGON_SHINING",SV_DRAGON_SHINING);
 tolua_constant(tolua_S,"SV_DRAGON_LAW",SV_DRAGON_LAW);
 tolua_constant(tolua_S,"SV_DRAGON_BRONZE",SV_DRAGON_BRONZE);
 tolua_constant(tolua_S,"SV_DRAGON_GOLD",SV_DRAGON_GOLD);
 tolua_constant(tolua_S,"SV_DRAGON_CHAOS",SV_DRAGON_CHAOS);
 tolua_constant(tolua_S,"SV_DRAGON_BALANCE",SV_DRAGON_BALANCE);
 tolua_constant(tolua_S,"SV_DRAGON_POWER",SV_DRAGON_POWER);
 tolua_constant(tolua_S,"SV_LITE_TORCH",SV_LITE_TORCH);
 tolua_constant(tolua_S,"SV_LITE_LANTERN",SV_LITE_LANTERN);
 tolua_constant(tolua_S,"SV_LITE_GALADRIEL",SV_LITE_GALADRIEL);
 tolua_constant(tolua_S,"SV_LITE_ELENDIL",SV_LITE_ELENDIL);
 tolua_constant(tolua_S,"SV_LITE_THRAIN",SV_LITE_THRAIN);
 tolua_constant(tolua_S,"SV_LITE_PALANTIR",SV_LITE_PALANTIR);
 tolua_constant(tolua_S,"SV_AMULET_DOOM",SV_AMULET_DOOM);
 tolua_constant(tolua_S,"SV_AMULET_TELEPORT",SV_AMULET_TELEPORT);
 tolua_constant(tolua_S,"SV_AMULET_ADORNMENT",SV_AMULET_ADORNMENT);
 tolua_constant(tolua_S,"SV_AMULET_SLOW_DIGEST",SV_AMULET_SLOW_DIGEST);
 tolua_constant(tolua_S,"SV_AMULET_RESIST_ACID",SV_AMULET_RESIST_ACID);
 tolua_constant(tolua_S,"SV_AMULET_SEARCHING",SV_AMULET_SEARCHING);
 tolua_constant(tolua_S,"SV_AMULET_WISDOM",SV_AMULET_WISDOM);
 tolua_constant(tolua_S,"SV_AMULET_CHARISMA",SV_AMULET_CHARISMA);
 tolua_constant(tolua_S,"SV_AMULET_THE_MAGI",SV_AMULET_THE_MAGI);
 tolua_constant(tolua_S,"SV_AMULET_SUSTENANCE",SV_AMULET_SUSTENANCE);
 tolua_constant(tolua_S,"SV_AMULET_CARLAMMAS",SV_AMULET_CARLAMMAS);
 tolua_constant(tolua_S,"SV_AMULET_INGWE",SV_AMULET_INGWE);
 tolua_constant(tolua_S,"SV_AMULET_DWARVES",SV_AMULET_DWARVES);
 tolua_constant(tolua_S,"SV_AMULET_ESP",SV_AMULET_ESP);
 tolua_constant(tolua_S,"SV_AMULET_RESIST",SV_AMULET_RESIST);
 tolua_constant(tolua_S,"SV_AMULET_REGEN",SV_AMULET_REGEN);
 tolua_constant(tolua_S,"SV_AMULET_ELESSAR",SV_AMULET_ELESSAR);
 tolua_constant(tolua_S,"SV_AMULET_EVENSTAR",SV_AMULET_EVENSTAR);
 tolua_constant(tolua_S,"SV_AMULET_DEVOTION",SV_AMULET_DEVOTION);
 tolua_constant(tolua_S,"SV_AMULET_WEAPONMASTERY",SV_AMULET_WEAPONMASTERY);
 tolua_constant(tolua_S,"SV_AMULET_TRICKERY",SV_AMULET_TRICKERY);
 tolua_constant(tolua_S,"SV_AMULET_INFRAVISION",SV_AMULET_INFRAVISION);
 tolua_constant(tolua_S,"SV_AMULET_RESIST_LIGHTNING",SV_AMULET_RESIST_LIGHTNING);
 tolua_constant(tolua_S,"SV_RING_WOE",SV_RING_WOE);
 tolua_constant(tolua_S,"SV_RING_AGGRAVATION",SV_RING_AGGRAVATION);
 tolua_constant(tolua_S,"SV_RING_WEAKNESS",SV_RING_WEAKNESS);
 tolua_constant(tolua_S,"SV_RING_STUPIDITY",SV_RING_STUPIDITY);
 tolua_constant(tolua_S,"SV_RING_TELEPORTATION",SV_RING_TELEPORTATION);
 tolua_constant(tolua_S,"SV_RING_SLOW_DIGESTION",SV_RING_SLOW_DIGESTION);
 tolua_constant(tolua_S,"SV_RING_FEATHER_FALL",SV_RING_FEATHER_FALL);
 tolua_constant(tolua_S,"SV_RING_RESIST_FIRE",SV_RING_RESIST_FIRE);
 tolua_constant(tolua_S,"SV_RING_RESIST_COLD",SV_RING_RESIST_COLD);
 tolua_constant(tolua_S,"SV_RING_SUSTAIN_STR",SV_RING_SUSTAIN_STR);
 tolua_constant(tolua_S,"SV_RING_SUSTAIN_INT",SV_RING_SUSTAIN_INT);
 tolua_constant(tolua_S,"SV_RING_SUSTAIN_WIS",SV_RING_SUSTAIN_WIS);
 tolua_constant(tolua_S,"SV_RING_SUSTAIN_DEX",SV_RING_SUSTAIN_DEX);
 tolua_constant(tolua_S,"SV_RING_SUSTAIN_CON",SV_RING_SUSTAIN_CON);
 tolua_constant(tolua_S,"SV_RING_SUSTAIN_CHR",SV_RING_SUSTAIN_CHR);
 tolua_constant(tolua_S,"SV_RING_PROTECTION",SV_RING_PROTECTION);
 tolua_constant(tolua_S,"SV_RING_ACID",SV_RING_ACID);
 tolua_constant(tolua_S,"SV_RING_FLAMES",SV_RING_FLAMES);
 tolua_constant(tolua_S,"SV_RING_ICE",SV_RING_ICE);
 tolua_constant(tolua_S,"SV_RING_RESIST_POIS",SV_RING_RESIST_POIS);
 tolua_constant(tolua_S,"SV_RING_FREE_ACTION",SV_RING_FREE_ACTION);
 tolua_constant(tolua_S,"SV_RING_SEE_INVIS",SV_RING_SEE_INVIS);
 tolua_constant(tolua_S,"SV_RING_SEARCHING",SV_RING_SEARCHING);
 tolua_constant(tolua_S,"SV_RING_STR",SV_RING_STR);
 tolua_constant(tolua_S,"SV_RING_INT",SV_RING_INT);
 tolua_constant(tolua_S,"SV_RING_DEX",SV_RING_DEX);
 tolua_constant(tolua_S,"SV_RING_CON",SV_RING_CON);
 tolua_constant(tolua_S,"SV_RING_ACCURACY",SV_RING_ACCURACY);
 tolua_constant(tolua_S,"SV_RING_DAMAGE",SV_RING_DAMAGE);
 tolua_constant(tolua_S,"SV_RING_SLAYING",SV_RING_SLAYING);
 tolua_constant(tolua_S,"SV_RING_SPEED",SV_RING_SPEED);
 tolua_constant(tolua_S,"SV_RING_BARAHIR",SV_RING_BARAHIR);
 tolua_constant(tolua_S,"SV_RING_TULKAS",SV_RING_TULKAS);
 tolua_constant(tolua_S,"SV_RING_NARYA",SV_RING_NARYA);
 tolua_constant(tolua_S,"SV_RING_NENYA",SV_RING_NENYA);
 tolua_constant(tolua_S,"SV_RING_VILYA",SV_RING_VILYA);
 tolua_constant(tolua_S,"SV_RING_POWER",SV_RING_POWER);
 tolua_constant(tolua_S,"SV_RING_LIGHTNING",SV_RING_LIGHTNING);
 tolua_constant(tolua_S,"SV_STAFF_DARKNESS",SV_STAFF_DARKNESS);
 tolua_constant(tolua_S,"SV_STAFF_SLOWNESS",SV_STAFF_SLOWNESS);
 tolua_constant(tolua_S,"SV_STAFF_HASTE_MONSTERS",SV_STAFF_HASTE_MONSTERS);
 tolua_constant(tolua_S,"SV_STAFF_SUMMONING",SV_STAFF_SUMMONING);
 tolua_constant(tolua_S,"SV_STAFF_TELEPORTATION",SV_STAFF_TELEPORTATION);
 tolua_constant(tolua_S,"SV_STAFF_IDENTIFY",SV_STAFF_IDENTIFY);
 tolua_constant(tolua_S,"SV_STAFF_REMOVE_CURSE",SV_STAFF_REMOVE_CURSE);
 tolua_constant(tolua_S,"SV_STAFF_STARLITE",SV_STAFF_STARLITE);
 tolua_constant(tolua_S,"SV_STAFF_LITE",SV_STAFF_LITE);
 tolua_constant(tolua_S,"SV_STAFF_MAPPING",SV_STAFF_MAPPING);
 tolua_constant(tolua_S,"SV_STAFF_DETECT_GOLD",SV_STAFF_DETECT_GOLD);
 tolua_constant(tolua_S,"SV_STAFF_DETECT_ITEM",SV_STAFF_DETECT_ITEM);
 tolua_constant(tolua_S,"SV_STAFF_DETECT_TRAP",SV_STAFF_DETECT_TRAP);
 tolua_constant(tolua_S,"SV_STAFF_DETECT_DOOR",SV_STAFF_DETECT_DOOR);
 tolua_constant(tolua_S,"SV_STAFF_DETECT_INVIS",SV_STAFF_DETECT_INVIS);
 tolua_constant(tolua_S,"SV_STAFF_DETECT_EVIL",SV_STAFF_DETECT_EVIL);
 tolua_constant(tolua_S,"SV_STAFF_CURE_LIGHT",SV_STAFF_CURE_LIGHT);
 tolua_constant(tolua_S,"SV_STAFF_CURING",SV_STAFF_CURING);
 tolua_constant(tolua_S,"SV_STAFF_HEALING",SV_STAFF_HEALING);
 tolua_constant(tolua_S,"SV_STAFF_THE_MAGI",SV_STAFF_THE_MAGI);
 tolua_constant(tolua_S,"SV_STAFF_SLEEP_MONSTERS",SV_STAFF_SLEEP_MONSTERS);
 tolua_constant(tolua_S,"SV_STAFF_SLOW_MONSTERS",SV_STAFF_SLOW_MONSTERS);
 tolua_constant(tolua_S,"SV_STAFF_SPEED",SV_STAFF_SPEED);
 tolua_constant(tolua_S,"SV_STAFF_PROBING",SV_STAFF_PROBING);
 tolua_constant(tolua_S,"SV_STAFF_DISPEL_EVIL",SV_STAFF_DISPEL_EVIL);
 tolua_constant(tolua_S,"SV_STAFF_POWER",SV_STAFF_POWER);
 tolua_constant(tolua_S,"SV_STAFF_HOLINESS",SV_STAFF_HOLINESS);
 tolua_constant(tolua_S,"SV_STAFF_BANISHMENT",SV_STAFF_BANISHMENT);
 tolua_constant(tolua_S,"SV_STAFF_EARTHQUAKES",SV_STAFF_EARTHQUAKES);
 tolua_constant(tolua_S,"SV_STAFF_DESTRUCTION",SV_STAFF_DESTRUCTION);
 tolua_constant(tolua_S,"SV_WAND_HEAL_MONSTER",SV_WAND_HEAL_MONSTER);
 tolua_constant(tolua_S,"SV_WAND_HASTE_MONSTER",SV_WAND_HASTE_MONSTER);
 tolua_constant(tolua_S,"SV_WAND_CLONE_MONSTER",SV_WAND_CLONE_MONSTER);
 tolua_constant(tolua_S,"SV_WAND_TELEPORT_AWAY",SV_WAND_TELEPORT_AWAY);
 tolua_constant(tolua_S,"SV_WAND_DISARMING",SV_WAND_DISARMING);
 tolua_constant(tolua_S,"SV_WAND_TRAP_DOOR_DEST",SV_WAND_TRAP_DOOR_DEST);
 tolua_constant(tolua_S,"SV_WAND_STONE_TO_MUD",SV_WAND_STONE_TO_MUD);
 tolua_constant(tolua_S,"SV_WAND_LITE",SV_WAND_LITE);
 tolua_constant(tolua_S,"SV_WAND_SLEEP_MONSTER",SV_WAND_SLEEP_MONSTER);
 tolua_constant(tolua_S,"SV_WAND_SLOW_MONSTER",SV_WAND_SLOW_MONSTER);
 tolua_constant(tolua_S,"SV_WAND_CONFUSE_MONSTER",SV_WAND_CONFUSE_MONSTER);
 tolua_constant(tolua_S,"SV_WAND_FEAR_MONSTER",SV_WAND_FEAR_MONSTER);
 tolua_constant(tolua_S,"SV_WAND_DRAIN_LIFE",SV_WAND_DRAIN_LIFE);
 tolua_constant(tolua_S,"SV_WAND_POLYMORPH",SV_WAND_POLYMORPH);
 tolua_constant(tolua_S,"SV_WAND_STINKING_CLOUD",SV_WAND_STINKING_CLOUD);
 tolua_constant(tolua_S,"SV_WAND_MAGIC_MISSILE",SV_WAND_MAGIC_MISSILE);
 tolua_constant(tolua_S,"SV_WAND_ACID_BOLT",SV_WAND_ACID_BOLT);
 tolua_constant(tolua_S,"SV_WAND_ELEC_BOLT",SV_WAND_ELEC_BOLT);
 tolua_constant(tolua_S,"SV_WAND_FIRE_BOLT",SV_WAND_FIRE_BOLT);
 tolua_constant(tolua_S,"SV_WAND_COLD_BOLT",SV_WAND_COLD_BOLT);
 tolua_constant(tolua_S,"SV_WAND_ACID_BALL",SV_WAND_ACID_BALL);
 tolua_constant(tolua_S,"SV_WAND_ELEC_BALL",SV_WAND_ELEC_BALL);
 tolua_constant(tolua_S,"SV_WAND_FIRE_BALL",SV_WAND_FIRE_BALL);
 tolua_constant(tolua_S,"SV_WAND_COLD_BALL",SV_WAND_COLD_BALL);
 tolua_constant(tolua_S,"SV_WAND_WONDER",SV_WAND_WONDER);
 tolua_constant(tolua_S,"SV_WAND_ANNIHILATION",SV_WAND_ANNIHILATION);
 tolua_constant(tolua_S,"SV_WAND_DRAGON_FIRE",SV_WAND_DRAGON_FIRE);
 tolua_constant(tolua_S,"SV_WAND_DRAGON_COLD",SV_WAND_DRAGON_COLD);
 tolua_constant(tolua_S,"SV_WAND_DRAGON_BREATH",SV_WAND_DRAGON_BREATH);
 tolua_constant(tolua_S,"SV_ROD_DETECT_TRAP",SV_ROD_DETECT_TRAP);
 tolua_constant(tolua_S,"SV_ROD_DETECT_DOOR",SV_ROD_DETECT_DOOR);
 tolua_constant(tolua_S,"SV_ROD_IDENTIFY",SV_ROD_IDENTIFY);
 tolua_constant(tolua_S,"SV_ROD_RECALL",SV_ROD_RECALL);
 tolua_constant(tolua_S,"SV_ROD_ILLUMINATION",SV_ROD_ILLUMINATION);
 tolua_constant(tolua_S,"SV_ROD_MAPPING",SV_ROD_MAPPING);
 tolua_constant(tolua_S,"SV_ROD_DETECTION",SV_ROD_DETECTION);
 tolua_constant(tolua_S,"SV_ROD_PROBING",SV_ROD_PROBING);
 tolua_constant(tolua_S,"SV_ROD_CURING",SV_ROD_CURING);
 tolua_constant(tolua_S,"SV_ROD_HEALING",SV_ROD_HEALING);
 tolua_constant(tolua_S,"SV_ROD_RESTORATION",SV_ROD_RESTORATION);
 tolua_constant(tolua_S,"SV_ROD_SPEED",SV_ROD_SPEED);
 tolua_constant(tolua_S,"SV_ROD_TELEPORT_AWAY",SV_ROD_TELEPORT_AWAY);
 tolua_constant(tolua_S,"SV_ROD_DISARMING",SV_ROD_DISARMING);
 tolua_constant(tolua_S,"SV_ROD_LITE",SV_ROD_LITE);
 tolua_constant(tolua_S,"SV_ROD_SLEEP_MONSTER",SV_ROD_SLEEP_MONSTER);
 tolua_constant(tolua_S,"SV_ROD_SLOW_MONSTER",SV_ROD_SLOW_MONSTER);
 tolua_constant(tolua_S,"SV_ROD_DRAIN_LIFE",SV_ROD_DRAIN_LIFE);
 tolua_constant(tolua_S,"SV_ROD_POLYMORPH",SV_ROD_POLYMORPH);
 tolua_constant(tolua_S,"SV_ROD_ACID_BOLT",SV_ROD_ACID_BOLT);
 tolua_constant(tolua_S,"SV_ROD_ELEC_BOLT",SV_ROD_ELEC_BOLT);
 tolua_constant(tolua_S,"SV_ROD_FIRE_BOLT",SV_ROD_FIRE_BOLT);
 tolua_constant(tolua_S,"SV_ROD_COLD_BOLT",SV_ROD_COLD_BOLT);
 tolua_constant(tolua_S,"SV_ROD_ACID_BALL",SV_ROD_ACID_BALL);
 tolua_constant(tolua_S,"SV_ROD_ELEC_BALL",SV_ROD_ELEC_BALL);
 tolua_constant(tolua_S,"SV_ROD_FIRE_BALL",SV_ROD_FIRE_BALL);
 tolua_constant(tolua_S,"SV_ROD_COLD_BALL",SV_ROD_COLD_BALL);
 tolua_constant(tolua_S,"SV_SCROLL_DARKNESS",SV_SCROLL_DARKNESS);
 tolua_constant(tolua_S,"SV_SCROLL_AGGRAVATE_MONSTER",SV_SCROLL_AGGRAVATE_MONSTER);
 tolua_constant(tolua_S,"SV_SCROLL_CURSE_ARMOR",SV_SCROLL_CURSE_ARMOR);
 tolua_constant(tolua_S,"SV_SCROLL_CURSE_WEAPON",SV_SCROLL_CURSE_WEAPON);
 tolua_constant(tolua_S,"SV_SCROLL_SUMMON_MONSTER",SV_SCROLL_SUMMON_MONSTER);
 tolua_constant(tolua_S,"SV_SCROLL_SUMMON_UNDEAD",SV_SCROLL_SUMMON_UNDEAD);
 tolua_constant(tolua_S,"SV_SCROLL_TRAP_CREATION",SV_SCROLL_TRAP_CREATION);
 tolua_constant(tolua_S,"SV_SCROLL_PHASE_DOOR",SV_SCROLL_PHASE_DOOR);
 tolua_constant(tolua_S,"SV_SCROLL_TELEPORT",SV_SCROLL_TELEPORT);
 tolua_constant(tolua_S,"SV_SCROLL_TELEPORT_LEVEL",SV_SCROLL_TELEPORT_LEVEL);
 tolua_constant(tolua_S,"SV_SCROLL_WORD_OF_RECALL",SV_SCROLL_WORD_OF_RECALL);
 tolua_constant(tolua_S,"SV_SCROLL_IDENTIFY",SV_SCROLL_IDENTIFY);
 tolua_constant(tolua_S,"SV_SCROLL_STAR_IDENTIFY",SV_SCROLL_STAR_IDENTIFY);
 tolua_constant(tolua_S,"SV_SCROLL_REMOVE_CURSE",SV_SCROLL_REMOVE_CURSE);
 tolua_constant(tolua_S,"SV_SCROLL_STAR_REMOVE_CURSE",SV_SCROLL_STAR_REMOVE_CURSE);
 tolua_constant(tolua_S,"SV_SCROLL_ENCHANT_ARMOR",SV_SCROLL_ENCHANT_ARMOR);
 tolua_constant(tolua_S,"SV_SCROLL_ENCHANT_WEAPON_TO_HIT",SV_SCROLL_ENCHANT_WEAPON_TO_HIT);
 tolua_constant(tolua_S,"SV_SCROLL_ENCHANT_WEAPON_TO_DAM",SV_SCROLL_ENCHANT_WEAPON_TO_DAM);
 tolua_constant(tolua_S,"SV_SCROLL_STAR_ENCHANT_ARMOR",SV_SCROLL_STAR_ENCHANT_ARMOR);
 tolua_constant(tolua_S,"SV_SCROLL_STAR_ENCHANT_WEAPON",SV_SCROLL_STAR_ENCHANT_WEAPON);
 tolua_constant(tolua_S,"SV_SCROLL_RECHARGING",SV_SCROLL_RECHARGING);
 tolua_constant(tolua_S,"SV_SCROLL_LIGHT",SV_SCROLL_LIGHT);
 tolua_constant(tolua_S,"SV_SCROLL_MAPPING",SV_SCROLL_MAPPING);
 tolua_constant(tolua_S,"SV_SCROLL_DETECT_GOLD",SV_SCROLL_DETECT_GOLD);
 tolua_constant(tolua_S,"SV_SCROLL_DETECT_ITEM",SV_SCROLL_DETECT_ITEM);
 tolua_constant(tolua_S,"SV_SCROLL_DETECT_TRAP",SV_SCROLL_DETECT_TRAP);
 tolua_constant(tolua_S,"SV_SCROLL_DETECT_DOOR",SV_SCROLL_DETECT_DOOR);
 tolua_constant(tolua_S,"SV_SCROLL_DETECT_INVIS",SV_SCROLL_DETECT_INVIS);
 tolua_constant(tolua_S,"SV_SCROLL_SATISFY_HUNGER",SV_SCROLL_SATISFY_HUNGER);
 tolua_constant(tolua_S,"SV_SCROLL_BLESSING",SV_SCROLL_BLESSING);
 tolua_constant(tolua_S,"SV_SCROLL_HOLY_CHANT",SV_SCROLL_HOLY_CHANT);
 tolua_constant(tolua_S,"SV_SCROLL_HOLY_PRAYER",SV_SCROLL_HOLY_PRAYER);
 tolua_constant(tolua_S,"SV_SCROLL_MONSTER_CONFUSION",SV_SCROLL_MONSTER_CONFUSION);
 tolua_constant(tolua_S,"SV_SCROLL_PROTECTION_FROM_EVIL",SV_SCROLL_PROTECTION_FROM_EVIL);
 tolua_constant(tolua_S,"SV_SCROLL_RUNE_OF_PROTECTION",SV_SCROLL_RUNE_OF_PROTECTION);
 tolua_constant(tolua_S,"SV_SCROLL_TRAP_DOOR_DESTRUCTION",SV_SCROLL_TRAP_DOOR_DESTRUCTION);
 tolua_constant(tolua_S,"SV_SCROLL_STAR_DESTRUCTION",SV_SCROLL_STAR_DESTRUCTION);
 tolua_constant(tolua_S,"SV_SCROLL_DISPEL_UNDEAD",SV_SCROLL_DISPEL_UNDEAD);
 tolua_constant(tolua_S,"SV_SCROLL_BANISHMENT",SV_SCROLL_BANISHMENT);
 tolua_constant(tolua_S,"SV_SCROLL_MASS_BANISHMENT",SV_SCROLL_MASS_BANISHMENT);
 tolua_constant(tolua_S,"SV_SCROLL_ACQUIREMENT",SV_SCROLL_ACQUIREMENT);
 tolua_constant(tolua_S,"SV_SCROLL_STAR_ACQUIREMENT",SV_SCROLL_STAR_ACQUIREMENT);
 tolua_constant(tolua_S,"SV_POTION_WATER",SV_POTION_WATER);
 tolua_constant(tolua_S,"SV_POTION_APPLE_JUICE",SV_POTION_APPLE_JUICE);
 tolua_constant(tolua_S,"SV_POTION_SLIME_MOLD",SV_POTION_SLIME_MOLD);
 tolua_constant(tolua_S,"SV_POTION_SLOWNESS",SV_POTION_SLOWNESS);
 tolua_constant(tolua_S,"SV_POTION_SALT_WATER",SV_POTION_SALT_WATER);
 tolua_constant(tolua_S,"SV_POTION_POISON",SV_POTION_POISON);
 tolua_constant(tolua_S,"SV_POTION_BLINDNESS",SV_POTION_BLINDNESS);
 tolua_constant(tolua_S,"SV_POTION_CONFUSION",SV_POTION_CONFUSION);
 tolua_constant(tolua_S,"SV_POTION_SLEEP",SV_POTION_SLEEP);
 tolua_constant(tolua_S,"SV_POTION_LOSE_MEMORIES",SV_POTION_LOSE_MEMORIES);
 tolua_constant(tolua_S,"SV_POTION_RUINATION",SV_POTION_RUINATION);
 tolua_constant(tolua_S,"SV_POTION_DEC_STR",SV_POTION_DEC_STR);
 tolua_constant(tolua_S,"SV_POTION_DEC_INT",SV_POTION_DEC_INT);
 tolua_constant(tolua_S,"SV_POTION_DEC_WIS",SV_POTION_DEC_WIS);
 tolua_constant(tolua_S,"SV_POTION_DEC_DEX",SV_POTION_DEC_DEX);
 tolua_constant(tolua_S,"SV_POTION_DEC_CON",SV_POTION_DEC_CON);
 tolua_constant(tolua_S,"SV_POTION_DEC_CHR",SV_POTION_DEC_CHR);
 tolua_constant(tolua_S,"SV_POTION_DETONATIONS",SV_POTION_DETONATIONS);
 tolua_constant(tolua_S,"SV_POTION_DEATH",SV_POTION_DEATH);
 tolua_constant(tolua_S,"SV_POTION_INFRAVISION",SV_POTION_INFRAVISION);
 tolua_constant(tolua_S,"SV_POTION_DETECT_INVIS",SV_POTION_DETECT_INVIS);
 tolua_constant(tolua_S,"SV_POTION_SLOW_POISON",SV_POTION_SLOW_POISON);
 tolua_constant(tolua_S,"SV_POTION_CURE_POISON",SV_POTION_CURE_POISON);
 tolua_constant(tolua_S,"SV_POTION_BOLDNESS",SV_POTION_BOLDNESS);
 tolua_constant(tolua_S,"SV_POTION_SPEED",SV_POTION_SPEED);
 tolua_constant(tolua_S,"SV_POTION_RESIST_HEAT",SV_POTION_RESIST_HEAT);
 tolua_constant(tolua_S,"SV_POTION_RESIST_COLD",SV_POTION_RESIST_COLD);
 tolua_constant(tolua_S,"SV_POTION_HEROISM",SV_POTION_HEROISM);
 tolua_constant(tolua_S,"SV_POTION_BERSERK_STRENGTH",SV_POTION_BERSERK_STRENGTH);
 tolua_constant(tolua_S,"SV_POTION_CURE_LIGHT",SV_POTION_CURE_LIGHT);
 tolua_constant(tolua_S,"SV_POTION_CURE_SERIOUS",SV_POTION_CURE_SERIOUS);
 tolua_constant(tolua_S,"SV_POTION_CURE_CRITICAL",SV_POTION_CURE_CRITICAL);
 tolua_constant(tolua_S,"SV_POTION_HEALING",SV_POTION_HEALING);
 tolua_constant(tolua_S,"SV_POTION_STAR_HEALING",SV_POTION_STAR_HEALING);
 tolua_constant(tolua_S,"SV_POTION_LIFE",SV_POTION_LIFE);
 tolua_constant(tolua_S,"SV_POTION_RESTORE_MANA",SV_POTION_RESTORE_MANA);
 tolua_constant(tolua_S,"SV_POTION_RESTORE_EXP",SV_POTION_RESTORE_EXP);
 tolua_constant(tolua_S,"SV_POTION_RES_STR",SV_POTION_RES_STR);
 tolua_constant(tolua_S,"SV_POTION_RES_INT",SV_POTION_RES_INT);
 tolua_constant(tolua_S,"SV_POTION_RES_WIS",SV_POTION_RES_WIS);
 tolua_constant(tolua_S,"SV_POTION_RES_DEX",SV_POTION_RES_DEX);
 tolua_constant(tolua_S,"SV_POTION_RES_CON",SV_POTION_RES_CON);
 tolua_constant(tolua_S,"SV_POTION_RES_CHR",SV_POTION_RES_CHR);
 tolua_constant(tolua_S,"SV_POTION_INC_STR",SV_POTION_INC_STR);
 tolua_constant(tolua_S,"SV_POTION_INC_INT",SV_POTION_INC_INT);
 tolua_constant(tolua_S,"SV_POTION_INC_WIS",SV_POTION_INC_WIS);
 tolua_constant(tolua_S,"SV_POTION_INC_DEX",SV_POTION_INC_DEX);
 tolua_constant(tolua_S,"SV_POTION_INC_CON",SV_POTION_INC_CON);
 tolua_constant(tolua_S,"SV_POTION_INC_CHR",SV_POTION_INC_CHR);
 tolua_constant(tolua_S,"SV_POTION_AUGMENTATION",SV_POTION_AUGMENTATION);
 tolua_constant(tolua_S,"SV_POTION_ENLIGHTENMENT",SV_POTION_ENLIGHTENMENT);
 tolua_constant(tolua_S,"SV_POTION_STAR_ENLIGHTENMENT",SV_POTION_STAR_ENLIGHTENMENT);
 tolua_constant(tolua_S,"SV_POTION_SELF_KNOWLEDGE",SV_POTION_SELF_KNOWLEDGE);
 tolua_constant(tolua_S,"SV_POTION_EXPERIENCE",SV_POTION_EXPERIENCE);
 tolua_constant(tolua_S,"SV_FOOD_POISON",SV_FOOD_POISON);
 tolua_constant(tolua_S,"SV_FOOD_BLINDNESS",SV_FOOD_BLINDNESS);
 tolua_constant(tolua_S,"SV_FOOD_PARANOIA",SV_FOOD_PARANOIA);
 tolua_constant(tolua_S,"SV_FOOD_CONFUSION",SV_FOOD_CONFUSION);
 tolua_constant(tolua_S,"SV_FOOD_HALLUCINATION",SV_FOOD_HALLUCINATION);
 tolua_constant(tolua_S,"SV_FOOD_PARALYSIS",SV_FOOD_PARALYSIS);
 tolua_constant(tolua_S,"SV_FOOD_WEAKNESS",SV_FOOD_WEAKNESS);
 tolua_constant(tolua_S,"SV_FOOD_SICKNESS",SV_FOOD_SICKNESS);
 tolua_constant(tolua_S,"SV_FOOD_STUPIDITY",SV_FOOD_STUPIDITY);
 tolua_constant(tolua_S,"SV_FOOD_NAIVETY",SV_FOOD_NAIVETY);
 tolua_constant(tolua_S,"SV_FOOD_UNHEALTH",SV_FOOD_UNHEALTH);
 tolua_constant(tolua_S,"SV_FOOD_DISEASE",SV_FOOD_DISEASE);
 tolua_constant(tolua_S,"SV_FOOD_CURE_POISON",SV_FOOD_CURE_POISON);
 tolua_constant(tolua_S,"SV_FOOD_CURE_BLINDNESS",SV_FOOD_CURE_BLINDNESS);
 tolua_constant(tolua_S,"SV_FOOD_CURE_PARANOIA",SV_FOOD_CURE_PARANOIA);
 tolua_constant(tolua_S,"SV_FOOD_CURE_CONFUSION",SV_FOOD_CURE_CONFUSION);
 tolua_constant(tolua_S,"SV_FOOD_CURE_SERIOUS",SV_FOOD_CURE_SERIOUS);
 tolua_constant(tolua_S,"SV_FOOD_RESTORE_STR",SV_FOOD_RESTORE_STR);
 tolua_constant(tolua_S,"SV_FOOD_RESTORE_CON",SV_FOOD_RESTORE_CON);
 tolua_constant(tolua_S,"SV_FOOD_RESTORING",SV_FOOD_RESTORING);
 tolua_constant(tolua_S,"SV_FOOD_BISCUIT",SV_FOOD_BISCUIT);
 tolua_constant(tolua_S,"SV_FOOD_JERKY",SV_FOOD_JERKY);
 tolua_constant(tolua_S,"SV_FOOD_RATION",SV_FOOD_RATION);
 tolua_constant(tolua_S,"SV_FOOD_SLIME_MOLD",SV_FOOD_SLIME_MOLD);
 tolua_constant(tolua_S,"SV_FOOD_WAYBREAD",SV_FOOD_WAYBREAD);
 tolua_constant(tolua_S,"SV_FOOD_PINT_OF_ALE",SV_FOOD_PINT_OF_ALE);
 tolua_constant(tolua_S,"SV_FOOD_PINT_OF_WINE",SV_FOOD_PINT_OF_WINE);
 tolua_constant(tolua_S,"SV_FOOD_MIN_FOOD",SV_FOOD_MIN_FOOD);
 tolua_constant(tolua_S,"SV_ROD_MIN_DIRECTION",SV_ROD_MIN_DIRECTION);
 tolua_constant(tolua_S,"SV_CHEST_MIN_LARGE",SV_CHEST_MIN_LARGE);
 tolua_constant(tolua_S,"SV_BOOK_MIN_GOOD",SV_BOOK_MIN_GOOD);
 tolua_constant(tolua_S,"CHEST_LOSE_STR",CHEST_LOSE_STR);
 tolua_constant(tolua_S,"CHEST_LOSE_CON",CHEST_LOSE_CON);
 tolua_constant(tolua_S,"CHEST_POISON",CHEST_POISON);
 tolua_constant(tolua_S,"CHEST_PARALYZE",CHEST_PARALYZE);
 tolua_constant(tolua_S,"CHEST_EXPLODE",CHEST_EXPLODE);
 tolua_constant(tolua_S,"CHEST_SUMMON",CHEST_SUMMON);
 tolua_constant(tolua_S,"IDENT_SENSE",IDENT_SENSE);
 tolua_constant(tolua_S,"IDENT_EMPTY",IDENT_EMPTY);
 tolua_constant(tolua_S,"IDENT_KNOWN",IDENT_KNOWN);
 tolua_constant(tolua_S,"IDENT_STORE",IDENT_STORE);
 tolua_constant(tolua_S,"IDENT_MENTAL",IDENT_MENTAL);
 tolua_constant(tolua_S,"IDENT_CURSED",IDENT_CURSED);
 tolua_constant(tolua_S,"IDENT_BROKEN",IDENT_BROKEN);
 tolua_constant(tolua_S,"INSCRIP_NULL",INSCRIP_NULL);
 tolua_constant(tolua_S,"INSCRIP_TERRIBLE",INSCRIP_TERRIBLE);
 tolua_constant(tolua_S,"INSCRIP_WORTHLESS",INSCRIP_WORTHLESS);
 tolua_constant(tolua_S,"INSCRIP_CURSED",INSCRIP_CURSED);
 tolua_constant(tolua_S,"INSCRIP_BROKEN",INSCRIP_BROKEN);
 tolua_constant(tolua_S,"INSCRIP_AVERAGE",INSCRIP_AVERAGE);
 tolua_constant(tolua_S,"INSCRIP_GOOD",INSCRIP_GOOD);
 tolua_constant(tolua_S,"INSCRIP_EXCELLENT",INSCRIP_EXCELLENT);
 tolua_constant(tolua_S,"INSCRIP_SPECIAL",INSCRIP_SPECIAL);
 tolua_constant(tolua_S,"INSCRIP_UNCURSED",INSCRIP_UNCURSED);
 tolua_constant(tolua_S,"INSCRIP_INDESTRUCTIBLE",INSCRIP_INDESTRUCTIBLE);
 tolua_constant(tolua_S,"MAX_INSCRIP",MAX_INSCRIP);
 tolua_constant(tolua_S,"TR1_STR",TR1_STR);
 tolua_constant(tolua_S,"TR1_INT",TR1_INT);
 tolua_constant(tolua_S,"TR1_WIS",TR1_WIS);
 tolua_constant(tolua_S,"TR1_DEX",TR1_DEX);
 tolua_constant(tolua_S,"TR1_CON",TR1_CON);
 tolua_constant(tolua_S,"TR1_CHR",TR1_CHR);
 tolua_constant(tolua_S,"TR1_XXX1",TR1_XXX1);
 tolua_constant(tolua_S,"TR1_XXX2",TR1_XXX2);
 tolua_constant(tolua_S,"TR1_STEALTH",TR1_STEALTH);
 tolua_constant(tolua_S,"TR1_SEARCH",TR1_SEARCH);
 tolua_constant(tolua_S,"TR1_INFRA",TR1_INFRA);
 tolua_constant(tolua_S,"TR1_TUNNEL",TR1_TUNNEL);
 tolua_constant(tolua_S,"TR1_SPEED",TR1_SPEED);
 tolua_constant(tolua_S,"TR1_BLOWS",TR1_BLOWS);
 tolua_constant(tolua_S,"TR1_SHOTS",TR1_SHOTS);
 tolua_constant(tolua_S,"TR1_MIGHT",TR1_MIGHT);
 tolua_constant(tolua_S,"TR1_SLAY_ANIMAL",TR1_SLAY_ANIMAL);
 tolua_constant(tolua_S,"TR1_SLAY_EVIL",TR1_SLAY_EVIL);
 tolua_constant(tolua_S,"TR1_SLAY_UNDEAD",TR1_SLAY_UNDEAD);
 tolua_constant(tolua_S,"TR1_SLAY_DEMON",TR1_SLAY_DEMON);
 tolua_constant(tolua_S,"TR1_SLAY_ORC",TR1_SLAY_ORC);
 tolua_constant(tolua_S,"TR1_SLAY_TROLL",TR1_SLAY_TROLL);
 tolua_constant(tolua_S,"TR1_SLAY_GIANT",TR1_SLAY_GIANT);
 tolua_constant(tolua_S,"TR1_SLAY_DRAGON",TR1_SLAY_DRAGON);
 tolua_constant(tolua_S,"TR1_KILL_DRAGON",TR1_KILL_DRAGON);
 tolua_constant(tolua_S,"TR1_KILL_DEMON",TR1_KILL_DEMON);
 tolua_constant(tolua_S,"TR1_KILL_UNDEAD",TR1_KILL_UNDEAD);
 tolua_constant(tolua_S,"TR1_BRAND_POIS",TR1_BRAND_POIS);
 tolua_constant(tolua_S,"TR1_BRAND_ACID",TR1_BRAND_ACID);
 tolua_constant(tolua_S,"TR1_BRAND_ELEC",TR1_BRAND_ELEC);
 tolua_constant(tolua_S,"TR1_BRAND_FIRE",TR1_BRAND_FIRE);
 tolua_constant(tolua_S,"TR1_BRAND_COLD",TR1_BRAND_COLD);
 tolua_constant(tolua_S,"TR2_SUST_STR",TR2_SUST_STR);
 tolua_constant(tolua_S,"TR2_SUST_INT",TR2_SUST_INT);
 tolua_constant(tolua_S,"TR2_SUST_WIS",TR2_SUST_WIS);
 tolua_constant(tolua_S,"TR2_SUST_DEX",TR2_SUST_DEX);
 tolua_constant(tolua_S,"TR2_SUST_CON",TR2_SUST_CON);
 tolua_constant(tolua_S,"TR2_SUST_CHR",TR2_SUST_CHR);
 tolua_constant(tolua_S,"TR2_XXX1",TR2_XXX1);
 tolua_constant(tolua_S,"TR2_XXX2",TR2_XXX2);
 tolua_constant(tolua_S,"TR2_XXX3",TR2_XXX3);
 tolua_constant(tolua_S,"TR2_XXX4",TR2_XXX4);
 tolua_constant(tolua_S,"TR2_XXX5",TR2_XXX5);
 tolua_constant(tolua_S,"TR2_XXX6",TR2_XXX6);
 tolua_constant(tolua_S,"TR2_IM_ACID",TR2_IM_ACID);
 tolua_constant(tolua_S,"TR2_IM_ELEC",TR2_IM_ELEC);
 tolua_constant(tolua_S,"TR2_IM_FIRE",TR2_IM_FIRE);
 tolua_constant(tolua_S,"TR2_IM_COLD",TR2_IM_COLD);
 tolua_constant(tolua_S,"TR2_RES_ACID",TR2_RES_ACID);
 tolua_constant(tolua_S,"TR2_RES_ELEC",TR2_RES_ELEC);
 tolua_constant(tolua_S,"TR2_RES_FIRE",TR2_RES_FIRE);
 tolua_constant(tolua_S,"TR2_RES_COLD",TR2_RES_COLD);
 tolua_constant(tolua_S,"TR2_RES_POIS",TR2_RES_POIS);
 tolua_constant(tolua_S,"TR2_RES_FEAR",TR2_RES_FEAR);
 tolua_constant(tolua_S,"TR2_RES_LITE",TR2_RES_LITE);
 tolua_constant(tolua_S,"TR2_RES_DARK",TR2_RES_DARK);
 tolua_constant(tolua_S,"TR2_RES_BLIND",TR2_RES_BLIND);
 tolua_constant(tolua_S,"TR2_RES_CONFU",TR2_RES_CONFU);
 tolua_constant(tolua_S,"TR2_RES_SOUND",TR2_RES_SOUND);
 tolua_constant(tolua_S,"TR2_RES_SHARD",TR2_RES_SHARD);
 tolua_constant(tolua_S,"TR2_RES_NEXUS",TR2_RES_NEXUS);
 tolua_constant(tolua_S,"TR2_RES_NETHR",TR2_RES_NETHR);
 tolua_constant(tolua_S,"TR2_RES_CHAOS",TR2_RES_CHAOS);
 tolua_constant(tolua_S,"TR2_RES_DISEN",TR2_RES_DISEN);
 tolua_constant(tolua_S,"TR3_SLOW_DIGEST",TR3_SLOW_DIGEST);
 tolua_constant(tolua_S,"TR3_FEATHER",TR3_FEATHER);
 tolua_constant(tolua_S,"TR3_LITE",TR3_LITE);
 tolua_constant(tolua_S,"TR3_REGEN",TR3_REGEN);
 tolua_constant(tolua_S,"TR3_TELEPATHY",TR3_TELEPATHY);
 tolua_constant(tolua_S,"TR3_SEE_INVIS",TR3_SEE_INVIS);
 tolua_constant(tolua_S,"TR3_FREE_ACT",TR3_FREE_ACT);
 tolua_constant(tolua_S,"TR3_HOLD_LIFE",TR3_HOLD_LIFE);
 tolua_constant(tolua_S,"TR3_XXX1",TR3_XXX1);
 tolua_constant(tolua_S,"TR3_XXX2",TR3_XXX2);
 tolua_constant(tolua_S,"TR3_XXX3",TR3_XXX3);
 tolua_constant(tolua_S,"TR3_XXX4",TR3_XXX4);
 tolua_constant(tolua_S,"TR3_IMPACT",TR3_IMPACT);
 tolua_constant(tolua_S,"TR3_TELEPORT",TR3_TELEPORT);
 tolua_constant(tolua_S,"TR3_AGGRAVATE",TR3_AGGRAVATE);
 tolua_constant(tolua_S,"TR3_DRAIN_EXP",TR3_DRAIN_EXP);
 tolua_constant(tolua_S,"TR3_IGNORE_ACID",TR3_IGNORE_ACID);
 tolua_constant(tolua_S,"TR3_IGNORE_ELEC",TR3_IGNORE_ELEC);
 tolua_constant(tolua_S,"TR3_IGNORE_FIRE",TR3_IGNORE_FIRE);
 tolua_constant(tolua_S,"TR3_IGNORE_COLD",TR3_IGNORE_COLD);
 tolua_constant(tolua_S,"TR3_XXX5",TR3_XXX5);
 tolua_constant(tolua_S,"TR3_XXX6",TR3_XXX6);
 tolua_constant(tolua_S,"TR3_BLESSED",TR3_BLESSED);
 tolua_constant(tolua_S,"TR3_ACTIVATE",TR3_ACTIVATE);
 tolua_constant(tolua_S,"TR3_INSTA_ART",TR3_INSTA_ART);
 tolua_constant(tolua_S,"TR3_EASY_KNOW",TR3_EASY_KNOW);
 tolua_constant(tolua_S,"TR3_HIDE_TYPE",TR3_HIDE_TYPE);
 tolua_constant(tolua_S,"TR3_SHOW_MODS",TR3_SHOW_MODS);
 tolua_constant(tolua_S,"TR3_XXX7",TR3_XXX7);
 tolua_constant(tolua_S,"TR3_LIGHT_CURSE",TR3_LIGHT_CURSE);
 tolua_constant(tolua_S,"TR3_HEAVY_CURSE",TR3_HEAVY_CURSE);
 tolua_constant(tolua_S,"TR3_PERMA_CURSE",TR3_PERMA_CURSE);
 tolua_constant(tolua_S,"OBJECT_XTRA_TYPE_SUSTAIN",OBJECT_XTRA_TYPE_SUSTAIN);
 tolua_constant(tolua_S,"OBJECT_XTRA_TYPE_RESIST",OBJECT_XTRA_TYPE_RESIST);
 tolua_constant(tolua_S,"OBJECT_XTRA_TYPE_POWER",OBJECT_XTRA_TYPE_POWER);
 tolua_constant(tolua_S,"OBJECT_XTRA_WHAT_SUSTAIN",OBJECT_XTRA_WHAT_SUSTAIN);
 tolua_constant(tolua_S,"OBJECT_XTRA_WHAT_RESIST",OBJECT_XTRA_WHAT_RESIST);
 tolua_constant(tolua_S,"OBJECT_XTRA_WHAT_POWER",OBJECT_XTRA_WHAT_POWER);
 tolua_constant(tolua_S,"OBJECT_XTRA_BASE_SUSTAIN",OBJECT_XTRA_BASE_SUSTAIN);
 tolua_constant(tolua_S,"OBJECT_XTRA_BASE_RESIST",OBJECT_XTRA_BASE_RESIST);
 tolua_constant(tolua_S,"OBJECT_XTRA_BASE_POWER",OBJECT_XTRA_BASE_POWER);
 tolua_constant(tolua_S,"OBJECT_XTRA_SIZE_SUSTAIN",OBJECT_XTRA_SIZE_SUSTAIN);
 tolua_constant(tolua_S,"OBJECT_XTRA_SIZE_RESIST",OBJECT_XTRA_SIZE_RESIST);
 tolua_constant(tolua_S,"OBJECT_XTRA_SIZE_POWER",OBJECT_XTRA_SIZE_POWER);
 tolua_constant(tolua_S,"ACT_ILLUMINATION",ACT_ILLUMINATION);
 tolua_constant(tolua_S,"ACT_MAGIC_MAP",ACT_MAGIC_MAP);
 tolua_constant(tolua_S,"ACT_CLAIRVOYANCE",ACT_CLAIRVOYANCE);
 tolua_constant(tolua_S,"ACT_PROT_EVIL",ACT_PROT_EVIL);
 tolua_constant(tolua_S,"ACT_DISP_EVIL",ACT_DISP_EVIL);
 tolua_constant(tolua_S,"ACT_HEAL1",ACT_HEAL1);
 tolua_constant(tolua_S,"ACT_HEAL2",ACT_HEAL2);
 tolua_constant(tolua_S,"ACT_CURE_WOUNDS",ACT_CURE_WOUNDS);
 tolua_constant(tolua_S,"ACT_HASTE1",ACT_HASTE1);
 tolua_constant(tolua_S,"ACT_HASTE2",ACT_HASTE2);
 tolua_constant(tolua_S,"ACT_FIRE1",ACT_FIRE1);
 tolua_constant(tolua_S,"ACT_FIRE2",ACT_FIRE2);
 tolua_constant(tolua_S,"ACT_FIRE3",ACT_FIRE3);
 tolua_constant(tolua_S,"ACT_FROST1",ACT_FROST1);
 tolua_constant(tolua_S,"ACT_FROST2",ACT_FROST2);
 tolua_constant(tolua_S,"ACT_FROST3",ACT_FROST3);
 tolua_constant(tolua_S,"ACT_FROST4",ACT_FROST4);
 tolua_constant(tolua_S,"ACT_FROST5",ACT_FROST5);
 tolua_constant(tolua_S,"ACT_ACID1",ACT_ACID1);
 tolua_constant(tolua_S,"ACT_RECHARGE1",ACT_RECHARGE1);
 tolua_constant(tolua_S,"ACT_SLEEP",ACT_SLEEP);
 tolua_constant(tolua_S,"ACT_LIGHTNING_BOLT",ACT_LIGHTNING_BOLT);
 tolua_constant(tolua_S,"ACT_ELEC2",ACT_ELEC2);
 tolua_constant(tolua_S,"ACT_BANISHMENT",ACT_BANISHMENT);
 tolua_constant(tolua_S,"ACT_MASS_BANISHMENT",ACT_MASS_BANISHMENT);
 tolua_constant(tolua_S,"ACT_IDENTIFY",ACT_IDENTIFY);
 tolua_constant(tolua_S,"ACT_DRAIN_LIFE1",ACT_DRAIN_LIFE1);
 tolua_constant(tolua_S,"ACT_DRAIN_LIFE2",ACT_DRAIN_LIFE2);
 tolua_constant(tolua_S,"ACT_BIZZARE",ACT_BIZZARE);
 tolua_constant(tolua_S,"ACT_STAR_BALL",ACT_STAR_BALL);
 tolua_constant(tolua_S,"ACT_RAGE_BLESS_RESIST",ACT_RAGE_BLESS_RESIST);
 tolua_constant(tolua_S,"ACT_PHASE",ACT_PHASE);
 tolua_constant(tolua_S,"ACT_TRAP_DOOR_DEST",ACT_TRAP_DOOR_DEST);
 tolua_constant(tolua_S,"ACT_DETECT",ACT_DETECT);
 tolua_constant(tolua_S,"ACT_RESIST",ACT_RESIST);
 tolua_constant(tolua_S,"ACT_TELEPORT",ACT_TELEPORT);
 tolua_constant(tolua_S,"ACT_RESTORE_LIFE",ACT_RESTORE_LIFE);
 tolua_constant(tolua_S,"ACT_MISSILE",ACT_MISSILE);
 tolua_constant(tolua_S,"ACT_ARROW",ACT_ARROW);
 tolua_constant(tolua_S,"ACT_REM_FEAR_POIS",ACT_REM_FEAR_POIS);
 tolua_constant(tolua_S,"ACT_STINKING_CLOUD",ACT_STINKING_CLOUD);
 tolua_constant(tolua_S,"ACT_STONE_TO_MUD",ACT_STONE_TO_MUD);
 tolua_constant(tolua_S,"ACT_TELE_AWAY",ACT_TELE_AWAY);
 tolua_constant(tolua_S,"ACT_WOR",ACT_WOR);
 tolua_constant(tolua_S,"ACT_CONFUSE",ACT_CONFUSE);
 tolua_constant(tolua_S,"ACT_PROBE",ACT_PROBE);
 tolua_constant(tolua_S,"ACT_FIREBRAND",ACT_FIREBRAND);
 tolua_constant(tolua_S,"ACT_STARLIGHT",ACT_STARLIGHT);
 tolua_constant(tolua_S,"ACT_MANA_BOLT",ACT_MANA_BOLT);
 tolua_constant(tolua_S,"ACT_BERSERKER",ACT_BERSERKER);
 tolua_constant(tolua_S,"ACT_MAX",ACT_MAX);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"object_type","object_type","",tolua_collect_object_type);
#else
 tolua_cclass(tolua_S,"object_type","object_type","",NULL);
#endif
 tolua_beginmodule(tolua_S,"object_type");
 tolua_variable(tolua_S,"k_idx",tolua_get_object_type_k_idx,tolua_set_object_type_k_idx);
 tolua_variable(tolua_S,"iy",tolua_get_object_type_iy,tolua_set_object_type_iy);
 tolua_variable(tolua_S,"ix",tolua_get_object_type_ix,tolua_set_object_type_ix);
 tolua_variable(tolua_S,"tval",tolua_get_object_type_tval,tolua_set_object_type_tval);
 tolua_variable(tolua_S,"sval",tolua_get_object_type_sval,tolua_set_object_type_sval);
 tolua_variable(tolua_S,"pval",tolua_get_object_type_pval,tolua_set_object_type_pval);
 tolua_variable(tolua_S,"discount",tolua_get_object_type_discount,tolua_set_object_type_discount);
 tolua_variable(tolua_S,"number",tolua_get_object_type_number,tolua_set_object_type_number);
 tolua_variable(tolua_S,"weight",tolua_get_object_type_weight,tolua_set_object_type_weight);
 tolua_variable(tolua_S,"name1",tolua_get_object_type_name1,tolua_set_object_type_name1);
 tolua_variable(tolua_S,"name2",tolua_get_object_type_name2,tolua_set_object_type_name2);
 tolua_variable(tolua_S,"xtra1",tolua_get_object_type_xtra1,tolua_set_object_type_xtra1);
 tolua_variable(tolua_S,"xtra2",tolua_get_object_type_xtra2,tolua_set_object_type_xtra2);
 tolua_variable(tolua_S,"to_h",tolua_get_object_type_to_h,tolua_set_object_type_to_h);
 tolua_variable(tolua_S,"to_d",tolua_get_object_type_to_d,tolua_set_object_type_to_d);
 tolua_variable(tolua_S,"to_a",tolua_get_object_type_to_a,tolua_set_object_type_to_a);
 tolua_variable(tolua_S,"ac",tolua_get_object_type_ac,tolua_set_object_type_ac);
 tolua_variable(tolua_S,"dd",tolua_get_object_type_dd,tolua_set_object_type_dd);
 tolua_variable(tolua_S,"ds",tolua_get_object_type_ds,tolua_set_object_type_ds);
 tolua_variable(tolua_S,"timeout",tolua_get_object_type_timeout,tolua_set_object_type_timeout);
 tolua_variable(tolua_S,"ident",tolua_get_object_type_ident,tolua_set_object_type_ident);
 tolua_variable(tolua_S,"marked",tolua_get_object_type_marked,tolua_set_object_type_marked);
 tolua_variable(tolua_S,"note",tolua_get_object_type_note,tolua_set_object_type_note);
 tolua_variable(tolua_S,"next_o_idx",tolua_get_object_type_next_o_idx,tolua_set_object_type_next_o_idx);
 tolua_variable(tolua_S,"held_m_idx",tolua_get_object_type_held_m_idx,tolua_set_object_type_held_m_idx);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"object_kind","object_kind","",tolua_collect_object_kind);
#else
 tolua_cclass(tolua_S,"object_kind","object_kind","",NULL);
#endif
 tolua_beginmodule(tolua_S,"object_kind");
 tolua_variable(tolua_S,"name",tolua_get_object_kind_name,tolua_set_object_kind_name);
 tolua_variable(tolua_S,"text",tolua_get_object_kind_text,tolua_set_object_kind_text);
 tolua_variable(tolua_S,"tval",tolua_get_object_kind_tval,tolua_set_object_kind_tval);
 tolua_variable(tolua_S,"sval",tolua_get_object_kind_sval,tolua_set_object_kind_sval);
 tolua_variable(tolua_S,"pval",tolua_get_object_kind_pval,tolua_set_object_kind_pval);
 tolua_variable(tolua_S,"to_h",tolua_get_object_kind_to_h,tolua_set_object_kind_to_h);
 tolua_variable(tolua_S,"to_d",tolua_get_object_kind_to_d,tolua_set_object_kind_to_d);
 tolua_variable(tolua_S,"to_a",tolua_get_object_kind_to_a,tolua_set_object_kind_to_a);
 tolua_variable(tolua_S,"ac",tolua_get_object_kind_ac,tolua_set_object_kind_ac);
 tolua_variable(tolua_S,"dd",tolua_get_object_kind_dd,tolua_set_object_kind_dd);
 tolua_variable(tolua_S,"ds",tolua_get_object_kind_ds,tolua_set_object_kind_ds);
 tolua_variable(tolua_S,"weight",tolua_get_object_kind_weight,tolua_set_object_kind_weight);
 tolua_variable(tolua_S,"cost",tolua_get_object_kind_cost,tolua_set_object_kind_cost);
 tolua_variable(tolua_S,"flags1",tolua_get_object_kind_flags1,tolua_set_object_kind_flags1);
 tolua_variable(tolua_S,"flags2",tolua_get_object_kind_flags2,tolua_set_object_kind_flags2);
 tolua_variable(tolua_S,"flags3",tolua_get_object_kind_flags3,tolua_set_object_kind_flags3);
 tolua_array(tolua_S,"locale",tolua_get_object_object_kind_locale,tolua_set_object_object_kind_locale);
 tolua_array(tolua_S,"chance",tolua_get_object_object_kind_chance,tolua_set_object_object_kind_chance);
 tolua_variable(tolua_S,"level",tolua_get_object_kind_level,tolua_set_object_kind_level);
 tolua_variable(tolua_S,"extra",tolua_get_object_kind_extra,tolua_set_object_kind_extra);
 tolua_variable(tolua_S,"d_attr",tolua_get_object_kind_d_attr,tolua_set_object_kind_d_attr);
 tolua_variable(tolua_S,"d_char",tolua_get_object_kind_d_char,tolua_set_object_kind_d_char);
 tolua_variable(tolua_S,"x_attr",tolua_get_object_kind_x_attr,tolua_set_object_kind_x_attr);
 tolua_variable(tolua_S,"x_char",tolua_get_object_kind_x_char,tolua_set_object_kind_x_char);
 tolua_variable(tolua_S,"flavor",tolua_get_object_kind_flavor,tolua_set_object_kind_flavor);
 tolua_variable(tolua_S,"aware",tolua_get_object_kind_aware,tolua_set_object_kind_aware);
 tolua_variable(tolua_S,"tried",tolua_get_object_kind_tried,tolua_set_object_kind_tried);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"artifact_type","artifact_type","",tolua_collect_artifact_type);
#else
 tolua_cclass(tolua_S,"artifact_type","artifact_type","",NULL);
#endif
 tolua_beginmodule(tolua_S,"artifact_type");
 tolua_variable(tolua_S,"name",tolua_get_artifact_type_name,tolua_set_artifact_type_name);
 tolua_variable(tolua_S,"text",tolua_get_artifact_type_text,tolua_set_artifact_type_text);
 tolua_variable(tolua_S,"tval",tolua_get_artifact_type_tval,tolua_set_artifact_type_tval);
 tolua_variable(tolua_S,"sval",tolua_get_artifact_type_sval,tolua_set_artifact_type_sval);
 tolua_variable(tolua_S,"pval",tolua_get_artifact_type_pval,tolua_set_artifact_type_pval);
 tolua_variable(tolua_S,"to_h",tolua_get_artifact_type_to_h,tolua_set_artifact_type_to_h);
 tolua_variable(tolua_S,"to_d",tolua_get_artifact_type_to_d,tolua_set_artifact_type_to_d);
 tolua_variable(tolua_S,"to_a",tolua_get_artifact_type_to_a,tolua_set_artifact_type_to_a);
 tolua_variable(tolua_S,"ac",tolua_get_artifact_type_ac,tolua_set_artifact_type_ac);
 tolua_variable(tolua_S,"dd",tolua_get_artifact_type_dd,tolua_set_artifact_type_dd);
 tolua_variable(tolua_S,"ds",tolua_get_artifact_type_ds,tolua_set_artifact_type_ds);
 tolua_variable(tolua_S,"weight",tolua_get_artifact_type_weight,tolua_set_artifact_type_weight);
 tolua_variable(tolua_S,"cost",tolua_get_artifact_type_cost,tolua_set_artifact_type_cost);
 tolua_variable(tolua_S,"flags1",tolua_get_artifact_type_flags1,tolua_set_artifact_type_flags1);
 tolua_variable(tolua_S,"flags2",tolua_get_artifact_type_flags2,tolua_set_artifact_type_flags2);
 tolua_variable(tolua_S,"flags3",tolua_get_artifact_type_flags3,tolua_set_artifact_type_flags3);
 tolua_variable(tolua_S,"level",tolua_get_artifact_type_level,tolua_set_artifact_type_level);
 tolua_variable(tolua_S,"rarity",tolua_get_artifact_type_rarity,tolua_set_artifact_type_rarity);
 tolua_variable(tolua_S,"cur_num",tolua_get_artifact_type_cur_num,tolua_set_artifact_type_cur_num);
 tolua_variable(tolua_S,"max_num",tolua_get_artifact_type_max_num,tolua_set_artifact_type_max_num);
 tolua_variable(tolua_S,"activation",tolua_get_artifact_type_activation,tolua_set_artifact_type_activation);
 tolua_variable(tolua_S,"time",tolua_get_artifact_type_time,tolua_set_artifact_type_time);
 tolua_variable(tolua_S,"randtime",tolua_get_artifact_type_randtime,tolua_set_artifact_type_randtime);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"ego_item_type","ego_item_type","",tolua_collect_ego_item_type);
#else
 tolua_cclass(tolua_S,"ego_item_type","ego_item_type","",NULL);
#endif
 tolua_beginmodule(tolua_S,"ego_item_type");
 tolua_variable(tolua_S,"name",tolua_get_ego_item_type_name,tolua_set_ego_item_type_name);
 tolua_variable(tolua_S,"text",tolua_get_ego_item_type_text,tolua_set_ego_item_type_text);
 tolua_variable(tolua_S,"cost",tolua_get_ego_item_type_cost,tolua_set_ego_item_type_cost);
 tolua_variable(tolua_S,"flags1",tolua_get_ego_item_type_flags1,tolua_set_ego_item_type_flags1);
 tolua_variable(tolua_S,"flags2",tolua_get_ego_item_type_flags2,tolua_set_ego_item_type_flags2);
 tolua_variable(tolua_S,"flags3",tolua_get_ego_item_type_flags3,tolua_set_ego_item_type_flags3);
 tolua_variable(tolua_S,"level",tolua_get_ego_item_type_level,tolua_set_ego_item_type_level);
 tolua_variable(tolua_S,"rarity",tolua_get_ego_item_type_rarity,tolua_set_ego_item_type_rarity);
 tolua_variable(tolua_S,"rating",tolua_get_ego_item_type_rating,tolua_set_ego_item_type_rating);
 tolua_array(tolua_S,"tval",tolua_get_object_ego_item_type_tval,tolua_set_object_ego_item_type_tval);
 tolua_array(tolua_S,"min_sval",tolua_get_object_ego_item_type_min_sval,tolua_set_object_ego_item_type_min_sval);
 tolua_array(tolua_S,"max_sval",tolua_get_object_ego_item_type_max_sval,tolua_set_object_ego_item_type_max_sval);
 tolua_variable(tolua_S,"max_to_h",tolua_get_ego_item_type_max_to_h,tolua_set_ego_item_type_max_to_h);
 tolua_variable(tolua_S,"max_to_d",tolua_get_ego_item_type_max_to_d,tolua_set_ego_item_type_max_to_d);
 tolua_variable(tolua_S,"max_to_a",tolua_get_ego_item_type_max_to_a,tolua_set_ego_item_type_max_to_a);
 tolua_variable(tolua_S,"max_pval",tolua_get_ego_item_type_max_pval,tolua_set_ego_item_type_max_pval);
 tolua_variable(tolua_S,"xtra",tolua_get_ego_item_type_xtra,tolua_set_ego_item_type_xtra);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"flavor_type","flavor_type","",tolua_collect_flavor_type);
#else
 tolua_cclass(tolua_S,"flavor_type","flavor_type","",NULL);
#endif
 tolua_beginmodule(tolua_S,"flavor_type");
 tolua_variable(tolua_S,"text",tolua_get_flavor_type_text,tolua_set_flavor_type_text);
 tolua_variable(tolua_S,"tval",tolua_get_flavor_type_tval,tolua_set_flavor_type_tval);
 tolua_variable(tolua_S,"sval",tolua_get_flavor_type_sval,tolua_set_flavor_type_sval);
 tolua_variable(tolua_S,"d_attr",tolua_get_flavor_type_d_attr,tolua_set_flavor_type_d_attr);
 tolua_variable(tolua_S,"d_char",tolua_get_flavor_type_d_char,tolua_set_flavor_type_d_char);
 tolua_variable(tolua_S,"x_attr",tolua_get_flavor_type_x_attr,tolua_set_flavor_type_x_attr);
 tolua_variable(tolua_S,"x_char",tolua_get_flavor_type_x_char,tolua_set_flavor_type_x_char);
 tolua_endmodule(tolua_S);
 tolua_variable(tolua_S,"o_max",tolua_get_o_max,tolua_set_o_max);
 tolua_variable(tolua_S,"o_cnt",tolua_get_o_cnt,tolua_set_o_cnt);
 tolua_array(tolua_S,"o_list",tolua_get_object_o_list,tolua_set_object_o_list);
 tolua_array(tolua_S,"k_info",tolua_get_object_k_info,tolua_set_object_k_info);
 tolua_variable(tolua_S,"k_name",tolua_get_k_name,tolua_set_k_name);
 tolua_variable(tolua_S,"k_text",tolua_get_k_text,tolua_set_k_text);
 tolua_array(tolua_S,"a_info",tolua_get_object_a_info,tolua_set_object_a_info);
 tolua_variable(tolua_S,"a_name",tolua_get_a_name,tolua_set_a_name);
 tolua_variable(tolua_S,"a_text",tolua_get_a_text,tolua_set_a_text);
 tolua_array(tolua_S,"e_info",tolua_get_object_e_info,tolua_set_object_e_info);
 tolua_variable(tolua_S,"e_name",tolua_get_e_name,tolua_set_e_name);
 tolua_variable(tolua_S,"e_text",tolua_get_e_text,tolua_set_e_text);
 tolua_array(tolua_S,"flavor_info",tolua_get_object_flavor_info,tolua_set_object_flavor_info);
 tolua_variable(tolua_S,"flavor_name",tolua_get_flavor_name,tolua_set_flavor_name);
 tolua_variable(tolua_S,"flavor_text",tolua_get_flavor_text,tolua_set_flavor_text);
 tolua_function(tolua_S,"flavor_init",tolua_object_flavor_init00);
 tolua_function(tolua_S,"reset_visuals",tolua_object_reset_visuals00);
 tolua_function(tolua_S,"identify_random_gen",tolua_object_identify_random_gen00);
 tolua_function(tolua_S,"index_to_label",tolua_object_index_to_label00);
 tolua_function(tolua_S,"label_to_inven",tolua_object_label_to_inven00);
 tolua_function(tolua_S,"label_to_equip",tolua_object_label_to_equip00);
 tolua_function(tolua_S,"wield_slot",tolua_object_wield_slot00);
 tolua_function(tolua_S,"mention_use",tolua_object_mention_use00);
 tolua_function(tolua_S,"describe_use",tolua_object_describe_use00);
 tolua_function(tolua_S,"item_tester_okay",tolua_object_item_tester_okay00);
 tolua_function(tolua_S,"scan_floor",tolua_object_scan_floor00);
 tolua_function(tolua_S,"display_inven",tolua_object_display_inven00);
 tolua_function(tolua_S,"display_equip",tolua_object_display_equip00);
 tolua_function(tolua_S,"show_inven",tolua_object_show_inven00);
 tolua_function(tolua_S,"show_equip",tolua_object_show_equip00);
 tolua_function(tolua_S,"toggle_inven_equip",tolua_object_toggle_inven_equip00);
 tolua_function(tolua_S,"get_item",tolua_object_get_item00);
 tolua_function(tolua_S,"excise_object_idx",tolua_object_excise_object_idx00);
 tolua_function(tolua_S,"delete_object_idx",tolua_object_delete_object_idx00);
 tolua_function(tolua_S,"delete_object",tolua_object_delete_object00);
 tolua_function(tolua_S,"compact_objects",tolua_object_compact_objects00);
 tolua_function(tolua_S,"wipe_o_list",tolua_object_wipe_o_list00);
 tolua_function(tolua_S,"o_pop",tolua_object_o_pop00);
 tolua_function(tolua_S,"get_obj_num_prep",tolua_object_get_obj_num_prep00);
 tolua_function(tolua_S,"get_obj_num",tolua_object_get_obj_num00);
 tolua_function(tolua_S,"object_known",tolua_object_object_known00);
 tolua_function(tolua_S,"object_aware",tolua_object_object_aware00);
 tolua_function(tolua_S,"object_tried",tolua_object_object_tried00);
 tolua_function(tolua_S,"is_blessed",tolua_object_is_blessed00);
 tolua_function(tolua_S,"object_value",tolua_object_object_value00);
 tolua_function(tolua_S,"object_similar",tolua_object_object_similar00);
 tolua_function(tolua_S,"object_absorb",tolua_object_object_absorb00);
 tolua_function(tolua_S,"lookup_kind",tolua_object_lookup_kind00);
 tolua_function(tolua_S,"object_wipe",tolua_object_object_wipe00);
 tolua_function(tolua_S,"object_copy",tolua_object_object_copy00);
 tolua_function(tolua_S,"object_prep",tolua_object_object_prep00);
 tolua_function(tolua_S,"apply_magic",tolua_object_apply_magic00);
 tolua_function(tolua_S,"make_object",tolua_object_make_object00);
 tolua_function(tolua_S,"make_gold",tolua_object_make_gold00);
 tolua_function(tolua_S,"floor_carry",tolua_object_floor_carry00);
 tolua_function(tolua_S,"drop_near",tolua_object_drop_near00);
 tolua_function(tolua_S,"acquirement",tolua_object_acquirement00);
 tolua_function(tolua_S,"place_object",tolua_object_place_object00);
 tolua_function(tolua_S,"place_gold",tolua_object_place_gold00);
 tolua_function(tolua_S,"pick_trap",tolua_object_pick_trap00);
 tolua_function(tolua_S,"place_trap",tolua_object_place_trap00);
 tolua_function(tolua_S,"place_secret_door",tolua_object_place_secret_door00);
 tolua_function(tolua_S,"place_closed_door",tolua_object_place_closed_door00);
 tolua_function(tolua_S,"place_random_door",tolua_object_place_random_door00);
 tolua_function(tolua_S,"inven_item_charges",tolua_object_inven_item_charges00);
 tolua_function(tolua_S,"inven_item_describe",tolua_object_inven_item_describe00);
 tolua_function(tolua_S,"inven_item_increase",tolua_object_inven_item_increase00);
 tolua_function(tolua_S,"inven_item_optimize",tolua_object_inven_item_optimize00);
 tolua_function(tolua_S,"floor_item_charges",tolua_object_floor_item_charges00);
 tolua_function(tolua_S,"floor_item_describe",tolua_object_floor_item_describe00);
 tolua_function(tolua_S,"floor_item_increase",tolua_object_floor_item_increase00);
 tolua_function(tolua_S,"floor_item_optimize",tolua_object_floor_item_optimize00);
 tolua_function(tolua_S,"inven_carry_okay",tolua_object_inven_carry_okay00);
 tolua_function(tolua_S,"inven_carry",tolua_object_inven_carry00);
 tolua_function(tolua_S,"inven_takeoff",tolua_object_inven_takeoff00);
 tolua_function(tolua_S,"inven_drop",tolua_object_inven_drop00);
 tolua_function(tolua_S,"combine_pack",tolua_object_combine_pack00);
 tolua_function(tolua_S,"reorder_pack",tolua_object_reorder_pack00);
 tolua_function(tolua_S,"object_aware_p",tolua_object_object_aware_p00);
 tolua_function(tolua_S,"object_tried_p",tolua_object_object_tried_p00);
 tolua_function(tolua_S,"object_known_p",tolua_object_object_known_p00);
 tolua_function(tolua_S,"object_attr",tolua_object_object_attr00);
 tolua_function(tolua_S,"object_char",tolua_object_object_char00);
 tolua_function(tolua_S,"artifact_p",tolua_object_artifact_p00);
 tolua_function(tolua_S,"ego_item_p",tolua_object_ego_item_p00);
 tolua_function(tolua_S,"broken_p",tolua_object_broken_p00);
 tolua_function(tolua_S,"cursed_p",tolua_object_cursed_p00);
 tolua_function(tolua_S,"object_info_out",tolua_object_object_info_out00);
 tolua_function(tolua_S,"object_info_screen",tolua_object_object_info_screen00);
 tolua_endmodule(tolua_S);
 return 1;
}
