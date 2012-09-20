/*
** Lua binding: object
** Generated automatically by tolua 4.0a - angband on Sun Jan  6 15:39:11 2002.
*/

#include "lua/tolua.h"

/* Exported function */
int tolua_object_open (lua_State* tolua_S);
void tolua_object_close (lua_State* tolua_S);

#include "angband.h"

/* function to register type */
static void toluaI_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"flavor_type");
 tolua_usertype(tolua_S,"artifact_type");
 tolua_usertype(tolua_S,"ego_item_type");
 tolua_usertype(tolua_S,"object_type");
 tolua_usertype(tolua_S,"object_kind");
}

/* error messages */
#define TOLUA_ERR_SELF tolua_error(tolua_S,"invalid 'self'")
#define TOLUA_ERR_ASSIGN tolua_error(tolua_S,"#vinvalid type in variable assignment.")

/* get function: k_idx of class  object_type */
static int toluaI_get_object_object_type_k_idx(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->k_idx);
 return 1;
}

/* set function: k_idx of class  object_type */
static int toluaI_set_object_object_type_k_idx(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->k_idx = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: iy of class  object_type */
static int toluaI_get_object_object_type_iy(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->iy);
 return 1;
}

/* set function: iy of class  object_type */
static int toluaI_set_object_object_type_iy(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->iy = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: ix of class  object_type */
static int toluaI_get_object_object_type_ix(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->ix);
 return 1;
}

/* set function: ix of class  object_type */
static int toluaI_set_object_object_type_ix(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->ix = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: tval of class  object_type */
static int toluaI_get_object_object_type_tval(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->tval);
 return 1;
}

/* set function: tval of class  object_type */
static int toluaI_set_object_object_type_tval(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->tval = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: sval of class  object_type */
static int toluaI_get_object_object_type_sval(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->sval);
 return 1;
}

/* set function: sval of class  object_type */
static int toluaI_set_object_object_type_sval(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->sval = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: pval of class  object_type */
static int toluaI_get_object_object_type_pval(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->pval);
 return 1;
}

/* set function: pval of class  object_type */
static int toluaI_set_object_object_type_pval(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->pval = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: discount of class  object_type */
static int toluaI_get_object_object_type_discount(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->discount);
 return 1;
}

/* set function: discount of class  object_type */
static int toluaI_set_object_object_type_discount(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->discount = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: number of class  object_type */
static int toluaI_get_object_object_type_number(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->number);
 return 1;
}

/* set function: number of class  object_type */
static int toluaI_set_object_object_type_number(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->number = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: weight of class  object_type */
static int toluaI_get_object_object_type_weight(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->weight);
 return 1;
}

/* set function: weight of class  object_type */
static int toluaI_set_object_object_type_weight(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->weight = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: name1 of class  object_type */
static int toluaI_get_object_object_type_name1(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->name1);
 return 1;
}

/* set function: name1 of class  object_type */
static int toluaI_set_object_object_type_name1(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->name1 = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: name2 of class  object_type */
static int toluaI_get_object_object_type_name2(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->name2);
 return 1;
}

/* set function: name2 of class  object_type */
static int toluaI_set_object_object_type_name2(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->name2 = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: xtra1 of class  object_type */
static int toluaI_get_object_object_type_xtra1(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->xtra1);
 return 1;
}

/* set function: xtra1 of class  object_type */
static int toluaI_set_object_object_type_xtra1(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->xtra1 = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: xtra2 of class  object_type */
static int toluaI_get_object_object_type_xtra2(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->xtra2);
 return 1;
}

/* set function: xtra2 of class  object_type */
static int toluaI_set_object_object_type_xtra2(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->xtra2 = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: to_h of class  object_type */
static int toluaI_get_object_object_type_to_h(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->to_h);
 return 1;
}

/* set function: to_h of class  object_type */
static int toluaI_set_object_object_type_to_h(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->to_h = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: to_d of class  object_type */
static int toluaI_get_object_object_type_to_d(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->to_d);
 return 1;
}

/* set function: to_d of class  object_type */
static int toluaI_set_object_object_type_to_d(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->to_d = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: to_a of class  object_type */
static int toluaI_get_object_object_type_to_a(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->to_a);
 return 1;
}

/* set function: to_a of class  object_type */
static int toluaI_set_object_object_type_to_a(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->to_a = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: ac of class  object_type */
static int toluaI_get_object_object_type_ac(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->ac);
 return 1;
}

/* set function: ac of class  object_type */
static int toluaI_set_object_object_type_ac(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->ac = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: dd of class  object_type */
static int toluaI_get_object_object_type_dd(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->dd);
 return 1;
}

/* set function: dd of class  object_type */
static int toluaI_set_object_object_type_dd(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->dd = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: ds of class  object_type */
static int toluaI_get_object_object_type_ds(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->ds);
 return 1;
}

/* set function: ds of class  object_type */
static int toluaI_set_object_object_type_ds(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->ds = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: timeout of class  object_type */
static int toluaI_get_object_object_type_timeout(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->timeout);
 return 1;
}

/* set function: timeout of class  object_type */
static int toluaI_set_object_object_type_timeout(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->timeout = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: ident of class  object_type */
static int toluaI_get_object_object_type_ident(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->ident);
 return 1;
}

/* set function: ident of class  object_type */
static int toluaI_set_object_object_type_ident(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->ident = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: marked of class  object_type */
static int toluaI_get_object_object_type_marked(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->marked);
 return 1;
}

/* set function: marked of class  object_type */
static int toluaI_set_object_object_type_marked(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->marked = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: note of class  object_type */
static int toluaI_get_object_object_type_note(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->note);
 return 1;
}

/* set function: note of class  object_type */
static int toluaI_set_object_object_type_note(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->note = ((u16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: next_o_idx of class  object_type */
static int toluaI_get_object_object_type_next_o_idx(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->next_o_idx);
 return 1;
}

/* set function: next_o_idx of class  object_type */
static int toluaI_set_object_object_type_next_o_idx(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->next_o_idx = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: held_m_idx of class  object_type */
static int toluaI_get_object_object_type_held_m_idx(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->held_m_idx);
 return 1;
}

/* set function: held_m_idx of class  object_type */
static int toluaI_set_object_object_type_held_m_idx(lua_State* tolua_S)
{
  object_type* self = (object_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->held_m_idx = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: name of class  object_kind */
static int toluaI_get_object_object_kind_name(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->name);
 return 1;
}

/* set function: name of class  object_kind */
static int toluaI_set_object_object_kind_name(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->name = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: text of class  object_kind */
static int toluaI_get_object_object_kind_text(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->text);
 return 1;
}

/* set function: text of class  object_kind */
static int toluaI_set_object_object_kind_text(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->text = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: tval of class  object_kind */
static int toluaI_get_object_object_kind_tval(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->tval);
 return 1;
}

/* set function: tval of class  object_kind */
static int toluaI_set_object_object_kind_tval(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->tval = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: sval of class  object_kind */
static int toluaI_get_object_object_kind_sval(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->sval);
 return 1;
}

/* set function: sval of class  object_kind */
static int toluaI_set_object_object_kind_sval(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->sval = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: pval of class  object_kind */
static int toluaI_get_object_object_kind_pval(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->pval);
 return 1;
}

/* set function: pval of class  object_kind */
static int toluaI_set_object_object_kind_pval(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->pval = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: to_h of class  object_kind */
static int toluaI_get_object_object_kind_to_h(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->to_h);
 return 1;
}

/* set function: to_h of class  object_kind */
static int toluaI_set_object_object_kind_to_h(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->to_h = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: to_d of class  object_kind */
static int toluaI_get_object_object_kind_to_d(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->to_d);
 return 1;
}

/* set function: to_d of class  object_kind */
static int toluaI_set_object_object_kind_to_d(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->to_d = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: to_a of class  object_kind */
static int toluaI_get_object_object_kind_to_a(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->to_a);
 return 1;
}

/* set function: to_a of class  object_kind */
static int toluaI_set_object_object_kind_to_a(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->to_a = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: ac of class  object_kind */
static int toluaI_get_object_object_kind_ac(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->ac);
 return 1;
}

/* set function: ac of class  object_kind */
static int toluaI_set_object_object_kind_ac(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->ac = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: dd of class  object_kind */
static int toluaI_get_object_object_kind_dd(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->dd);
 return 1;
}

/* set function: dd of class  object_kind */
static int toluaI_set_object_object_kind_dd(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->dd = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: ds of class  object_kind */
static int toluaI_get_object_object_kind_ds(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->ds);
 return 1;
}

/* set function: ds of class  object_kind */
static int toluaI_set_object_object_kind_ds(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->ds = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: weight of class  object_kind */
static int toluaI_get_object_object_kind_weight(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->weight);
 return 1;
}

/* set function: weight of class  object_kind */
static int toluaI_set_object_object_kind_weight(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->weight = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: cost of class  object_kind */
static int toluaI_get_object_object_kind_cost(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->cost);
 return 1;
}

/* set function: cost of class  object_kind */
static int toluaI_set_object_object_kind_cost(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->cost = ((s32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flags1 of class  object_kind */
static int toluaI_get_object_object_kind_flags1(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags1);
 return 1;
}

/* set function: flags1 of class  object_kind */
static int toluaI_set_object_object_kind_flags1(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags1 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flags2 of class  object_kind */
static int toluaI_get_object_object_kind_flags2(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags2);
 return 1;
}

/* set function: flags2 of class  object_kind */
static int toluaI_set_object_object_kind_flags2(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags2 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flags3 of class  object_kind */
static int toluaI_get_object_object_kind_flags3(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags3);
 return 1;
}

/* set function: flags3 of class  object_kind */
static int toluaI_set_object_object_kind_flags3(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags3 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: locale of class  object_kind */
static int toluaI_get_object_object_kind_locale(lua_State* tolua_S)
{
 int toluaI_index;
  object_kind* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (object_kind*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=4)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushnumber(tolua_S,(long)self->locale[toluaI_index]);
 return 1;
}

/* set function: locale of class  object_kind */
static int toluaI_set_object_object_kind_locale(lua_State* tolua_S)
{
 int toluaI_index;
  object_kind* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (object_kind*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=4)
 tolua_error(tolua_S,"array indexing out of range.");
  self->locale[toluaI_index] = ((byte)  tolua_getnumber(tolua_S,3,0));
 return 0;
}

/* get function: chance of class  object_kind */
static int toluaI_get_object_object_kind_chance(lua_State* tolua_S)
{
 int toluaI_index;
  object_kind* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (object_kind*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=4)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushnumber(tolua_S,(long)self->chance[toluaI_index]);
 return 1;
}

/* set function: chance of class  object_kind */
static int toluaI_set_object_object_kind_chance(lua_State* tolua_S)
{
 int toluaI_index;
  object_kind* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (object_kind*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=4)
 tolua_error(tolua_S,"array indexing out of range.");
  self->chance[toluaI_index] = ((byte)  tolua_getnumber(tolua_S,3,0));
 return 0;
}

/* get function: level of class  object_kind */
static int toluaI_get_object_object_kind_level(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->level);
 return 1;
}

/* set function: level of class  object_kind */
static int toluaI_set_object_object_kind_level(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->level = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: extra of class  object_kind */
static int toluaI_get_object_object_kind_extra(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->extra);
 return 1;
}

/* set function: extra of class  object_kind */
static int toluaI_set_object_object_kind_extra(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->extra = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: d_attr of class  object_kind */
static int toluaI_get_object_object_kind_d_attr(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->d_attr);
 return 1;
}

/* set function: d_attr of class  object_kind */
static int toluaI_set_object_object_kind_d_attr(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->d_attr = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: d_char of class  object_kind */
static int toluaI_get_object_object_kind_d_char(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->d_char);
 return 1;
}

/* set function: d_char of class  object_kind */
static int toluaI_set_object_object_kind_d_char(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->d_char = ((char)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: x_attr of class  object_kind */
static int toluaI_get_object_object_kind_x_attr(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->x_attr);
 return 1;
}

/* set function: x_attr of class  object_kind */
static int toluaI_set_object_object_kind_x_attr(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->x_attr = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: x_char of class  object_kind */
static int toluaI_get_object_object_kind_x_char(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->x_char);
 return 1;
}

/* set function: x_char of class  object_kind */
static int toluaI_set_object_object_kind_x_char(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->x_char = ((char)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flavor of class  object_kind */
static int toluaI_get_object_object_kind_flavor(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flavor);
 return 1;
}

/* set function: flavor of class  object_kind */
static int toluaI_set_object_object_kind_flavor(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flavor = ((u16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: aware of class  object_kind */
static int toluaI_get_object_object_kind_aware(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->aware);
 return 1;
}

/* set function: aware of class  object_kind */
static int toluaI_set_object_object_kind_aware(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->aware = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: tried of class  object_kind */
static int toluaI_get_object_object_kind_tried(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->tried);
 return 1;
}

/* set function: tried of class  object_kind */
static int toluaI_set_object_object_kind_tried(lua_State* tolua_S)
{
  object_kind* self = (object_kind*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->tried = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: name of class  artifact_type */
static int toluaI_get_object_artifact_type_name(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->name);
 return 1;
}

/* set function: name of class  artifact_type */
static int toluaI_set_object_artifact_type_name(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->name = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: text of class  artifact_type */
static int toluaI_get_object_artifact_type_text(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->text);
 return 1;
}

/* set function: text of class  artifact_type */
static int toluaI_set_object_artifact_type_text(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->text = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: tval of class  artifact_type */
static int toluaI_get_object_artifact_type_tval(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->tval);
 return 1;
}

/* set function: tval of class  artifact_type */
static int toluaI_set_object_artifact_type_tval(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->tval = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: sval of class  artifact_type */
static int toluaI_get_object_artifact_type_sval(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->sval);
 return 1;
}

/* set function: sval of class  artifact_type */
static int toluaI_set_object_artifact_type_sval(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->sval = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: pval of class  artifact_type */
static int toluaI_get_object_artifact_type_pval(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->pval);
 return 1;
}

/* set function: pval of class  artifact_type */
static int toluaI_set_object_artifact_type_pval(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->pval = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: to_h of class  artifact_type */
static int toluaI_get_object_artifact_type_to_h(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->to_h);
 return 1;
}

/* set function: to_h of class  artifact_type */
static int toluaI_set_object_artifact_type_to_h(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->to_h = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: to_d of class  artifact_type */
static int toluaI_get_object_artifact_type_to_d(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->to_d);
 return 1;
}

/* set function: to_d of class  artifact_type */
static int toluaI_set_object_artifact_type_to_d(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->to_d = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: to_a of class  artifact_type */
static int toluaI_get_object_artifact_type_to_a(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->to_a);
 return 1;
}

/* set function: to_a of class  artifact_type */
static int toluaI_set_object_artifact_type_to_a(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->to_a = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: ac of class  artifact_type */
static int toluaI_get_object_artifact_type_ac(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->ac);
 return 1;
}

/* set function: ac of class  artifact_type */
static int toluaI_set_object_artifact_type_ac(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->ac = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: dd of class  artifact_type */
static int toluaI_get_object_artifact_type_dd(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->dd);
 return 1;
}

/* set function: dd of class  artifact_type */
static int toluaI_set_object_artifact_type_dd(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->dd = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: ds of class  artifact_type */
static int toluaI_get_object_artifact_type_ds(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->ds);
 return 1;
}

/* set function: ds of class  artifact_type */
static int toluaI_set_object_artifact_type_ds(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->ds = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: weight of class  artifact_type */
static int toluaI_get_object_artifact_type_weight(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->weight);
 return 1;
}

/* set function: weight of class  artifact_type */
static int toluaI_set_object_artifact_type_weight(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->weight = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: cost of class  artifact_type */
static int toluaI_get_object_artifact_type_cost(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->cost);
 return 1;
}

/* set function: cost of class  artifact_type */
static int toluaI_set_object_artifact_type_cost(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->cost = ((s32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flags1 of class  artifact_type */
static int toluaI_get_object_artifact_type_flags1(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags1);
 return 1;
}

/* set function: flags1 of class  artifact_type */
static int toluaI_set_object_artifact_type_flags1(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags1 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flags2 of class  artifact_type */
static int toluaI_get_object_artifact_type_flags2(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags2);
 return 1;
}

/* set function: flags2 of class  artifact_type */
static int toluaI_set_object_artifact_type_flags2(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags2 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flags3 of class  artifact_type */
static int toluaI_get_object_artifact_type_flags3(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags3);
 return 1;
}

/* set function: flags3 of class  artifact_type */
static int toluaI_set_object_artifact_type_flags3(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags3 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: level of class  artifact_type */
static int toluaI_get_object_artifact_type_level(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->level);
 return 1;
}

/* set function: level of class  artifact_type */
static int toluaI_set_object_artifact_type_level(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->level = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: rarity of class  artifact_type */
static int toluaI_get_object_artifact_type_rarity(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->rarity);
 return 1;
}

/* set function: rarity of class  artifact_type */
static int toluaI_set_object_artifact_type_rarity(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->rarity = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: cur_num of class  artifact_type */
static int toluaI_get_object_artifact_type_cur_num(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->cur_num);
 return 1;
}

/* set function: cur_num of class  artifact_type */
static int toluaI_set_object_artifact_type_cur_num(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->cur_num = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: max_num of class  artifact_type */
static int toluaI_get_object_artifact_type_max_num(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->max_num);
 return 1;
}

/* set function: max_num of class  artifact_type */
static int toluaI_set_object_artifact_type_max_num(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->max_num = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: activation of class  artifact_type */
static int toluaI_get_object_artifact_type_activation(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->activation);
 return 1;
}

/* set function: activation of class  artifact_type */
static int toluaI_set_object_artifact_type_activation(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->activation = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: time of class  artifact_type */
static int toluaI_get_object_artifact_type_time(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->time);
 return 1;
}

/* set function: time of class  artifact_type */
static int toluaI_set_object_artifact_type_time(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->time = ((u16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: randtime of class  artifact_type */
static int toluaI_get_object_artifact_type_randtime(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->randtime);
 return 1;
}

/* set function: randtime of class  artifact_type */
static int toluaI_set_object_artifact_type_randtime(lua_State* tolua_S)
{
  artifact_type* self = (artifact_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->randtime = ((u16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: name of class  ego_item_type */
static int toluaI_get_object_ego_item_type_name(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->name);
 return 1;
}

/* set function: name of class  ego_item_type */
static int toluaI_set_object_ego_item_type_name(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->name = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: text of class  ego_item_type */
static int toluaI_get_object_ego_item_type_text(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->text);
 return 1;
}

/* set function: text of class  ego_item_type */
static int toluaI_set_object_ego_item_type_text(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->text = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: slot of class  ego_item_type */
static int toluaI_get_object_ego_item_type_slot(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->slot);
 return 1;
}

/* set function: slot of class  ego_item_type */
static int toluaI_set_object_ego_item_type_slot(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->slot = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: rating of class  ego_item_type */
static int toluaI_get_object_ego_item_type_rating(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->rating);
 return 1;
}

/* set function: rating of class  ego_item_type */
static int toluaI_set_object_ego_item_type_rating(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->rating = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: level of class  ego_item_type */
static int toluaI_get_object_ego_item_type_level(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->level);
 return 1;
}

/* set function: level of class  ego_item_type */
static int toluaI_set_object_ego_item_type_level(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->level = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: rarity of class  ego_item_type */
static int toluaI_get_object_ego_item_type_rarity(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->rarity);
 return 1;
}

/* set function: rarity of class  ego_item_type */
static int toluaI_set_object_ego_item_type_rarity(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->rarity = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: tval of class  ego_item_type */
static int toluaI_get_object_ego_item_type_tval(lua_State* tolua_S)
{
 int toluaI_index;
  ego_item_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (ego_item_type*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=3)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushnumber(tolua_S,(long)self->tval[toluaI_index]);
 return 1;
}

/* set function: tval of class  ego_item_type */
static int toluaI_set_object_ego_item_type_tval(lua_State* tolua_S)
{
 int toluaI_index;
  ego_item_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (ego_item_type*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=3)
 tolua_error(tolua_S,"array indexing out of range.");
  self->tval[toluaI_index] = ((byte)  tolua_getnumber(tolua_S,3,0));
 return 0;
}

/* get function: min_sval of class  ego_item_type */
static int toluaI_get_object_ego_item_type_min_sval(lua_State* tolua_S)
{
 int toluaI_index;
  ego_item_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (ego_item_type*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=3)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushnumber(tolua_S,(long)self->min_sval[toluaI_index]);
 return 1;
}

/* set function: min_sval of class  ego_item_type */
static int toluaI_set_object_ego_item_type_min_sval(lua_State* tolua_S)
{
 int toluaI_index;
  ego_item_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (ego_item_type*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=3)
 tolua_error(tolua_S,"array indexing out of range.");
  self->min_sval[toluaI_index] = ((byte)  tolua_getnumber(tolua_S,3,0));
 return 0;
}

/* get function: max_sval of class  ego_item_type */
static int toluaI_get_object_ego_item_type_max_sval(lua_State* tolua_S)
{
 int toluaI_index;
  ego_item_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (ego_item_type*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=3)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushnumber(tolua_S,(long)self->max_sval[toluaI_index]);
 return 1;
}

/* set function: max_sval of class  ego_item_type */
static int toluaI_set_object_ego_item_type_max_sval(lua_State* tolua_S)
{
 int toluaI_index;
  ego_item_type* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (ego_item_type*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=3)
 tolua_error(tolua_S,"array indexing out of range.");
  self->max_sval[toluaI_index] = ((byte)  tolua_getnumber(tolua_S,3,0));
 return 0;
}

/* get function: xtra of class  ego_item_type */
static int toluaI_get_object_ego_item_type_xtra(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->xtra);
 return 1;
}

/* set function: xtra of class  ego_item_type */
static int toluaI_set_object_ego_item_type_xtra(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->xtra = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: max_to_h of class  ego_item_type */
static int toluaI_get_object_ego_item_type_max_to_h(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->max_to_h);
 return 1;
}

/* set function: max_to_h of class  ego_item_type */
static int toluaI_set_object_ego_item_type_max_to_h(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->max_to_h = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: max_to_d of class  ego_item_type */
static int toluaI_get_object_ego_item_type_max_to_d(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->max_to_d);
 return 1;
}

/* set function: max_to_d of class  ego_item_type */
static int toluaI_set_object_ego_item_type_max_to_d(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->max_to_d = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: max_to_a of class  ego_item_type */
static int toluaI_get_object_ego_item_type_max_to_a(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->max_to_a);
 return 1;
}

/* set function: max_to_a of class  ego_item_type */
static int toluaI_set_object_ego_item_type_max_to_a(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->max_to_a = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: max_pval of class  ego_item_type */
static int toluaI_get_object_ego_item_type_max_pval(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->max_pval);
 return 1;
}

/* set function: max_pval of class  ego_item_type */
static int toluaI_set_object_ego_item_type_max_pval(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->max_pval = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: cost of class  ego_item_type */
static int toluaI_get_object_ego_item_type_cost(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->cost);
 return 1;
}

/* set function: cost of class  ego_item_type */
static int toluaI_set_object_ego_item_type_cost(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->cost = ((s32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flags1 of class  ego_item_type */
static int toluaI_get_object_ego_item_type_flags1(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags1);
 return 1;
}

/* set function: flags1 of class  ego_item_type */
static int toluaI_set_object_ego_item_type_flags1(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags1 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flags2 of class  ego_item_type */
static int toluaI_get_object_ego_item_type_flags2(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags2);
 return 1;
}

/* set function: flags2 of class  ego_item_type */
static int toluaI_set_object_ego_item_type_flags2(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags2 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flags3 of class  ego_item_type */
static int toluaI_get_object_ego_item_type_flags3(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags3);
 return 1;
}

/* set function: flags3 of class  ego_item_type */
static int toluaI_set_object_ego_item_type_flags3(lua_State* tolua_S)
{
  ego_item_type* self = (ego_item_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags3 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: text of class  flavor_type */
static int toluaI_get_object_flavor_type_text(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->text);
 return 1;
}

/* set function: text of class  flavor_type */
static int toluaI_set_object_flavor_type_text(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->text = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: tval of class  flavor_type */
static int toluaI_get_object_flavor_type_tval(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->tval);
 return 1;
}

/* set function: tval of class  flavor_type */
static int toluaI_set_object_flavor_type_tval(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->tval = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: d_attr of class  flavor_type */
static int toluaI_get_object_flavor_type_d_attr(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->d_attr);
 return 1;
}

/* set function: d_attr of class  flavor_type */
static int toluaI_set_object_flavor_type_d_attr(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->d_attr = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: d_char of class  flavor_type */
static int toluaI_get_object_flavor_type_d_char(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->d_char);
 return 1;
}

/* set function: d_char of class  flavor_type */
static int toluaI_set_object_flavor_type_d_char(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->d_char = ((char)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: x_attr of class  flavor_type */
static int toluaI_get_object_flavor_type_x_attr(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->x_attr);
 return 1;
}

/* set function: x_attr of class  flavor_type */
static int toluaI_set_object_flavor_type_x_attr(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->x_attr = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: x_char of class  flavor_type */
static int toluaI_get_object_flavor_type_x_char(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->x_char);
 return 1;
}

/* set function: x_char of class  flavor_type */
static int toluaI_set_object_flavor_type_x_char(lua_State* tolua_S)
{
  flavor_type* self = (flavor_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->x_char = ((char)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: o_max */
static int toluaI_get_object_o_max(lua_State* tolua_S)
{
 tolua_pushnumber(tolua_S,(long)o_max);
 return 1;
}

/* set function: o_max */
static int toluaI_set_object_o_max(lua_State* tolua_S)
{
 if (!tolua_istype(tolua_S,1,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  o_max = ((s16b)  tolua_getnumber(tolua_S,1,0));
 return 0;
}

/* get function: o_cnt */
static int toluaI_get_object_o_cnt(lua_State* tolua_S)
{
 tolua_pushnumber(tolua_S,(long)o_cnt);
 return 1;
}

/* set function: o_cnt */
static int toluaI_set_object_o_cnt(lua_State* tolua_S)
{
 if (!tolua_istype(tolua_S,1,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  o_cnt = ((s16b)  tolua_getnumber(tolua_S,1,0));
 return 0;
}

/* get function: o_list */
static int toluaI_get_object_o_list(lua_State* tolua_S)
{
 int toluaI_index;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=o_max)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushusertype(tolua_S,(void*)&o_list[toluaI_index],tolua_tag(tolua_S,"object_type"));
 return 1;
}

/* set function: o_list */
static int toluaI_set_object_o_list(lua_State* tolua_S)
{
 int toluaI_index;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=o_max)
 tolua_error(tolua_S,"array indexing out of range.");
  o_list[toluaI_index] = *((object_type*)  tolua_getusertype(tolua_S,3,0));
 return 0;
}

/* get function: k_info */
static int toluaI_get_object_k_info(lua_State* tolua_S)
{
 int toluaI_index;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=z_info->k_max)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushusertype(tolua_S,(void*)&k_info[toluaI_index],tolua_tag(tolua_S,"object_kind"));
 return 1;
}

/* set function: k_info */
static int toluaI_set_object_k_info(lua_State* tolua_S)
{
 int toluaI_index;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=z_info->k_max)
 tolua_error(tolua_S,"array indexing out of range.");
  k_info[toluaI_index] = *((object_kind*)  tolua_getusertype(tolua_S,3,0));
 return 0;
}

/* get function: k_name */
static int toluaI_get_object_k_name(lua_State* tolua_S)
{
 tolua_pushstring(tolua_S,(const char*)k_name);
 return 1;
}

/* set function: k_name */
static int toluaI_set_object_k_name(lua_State* tolua_S)
{
 if (!tolua_istype(tolua_S,1,LUA_TSTRING,0))
 TOLUA_ERR_ASSIGN;
  k_name = ((char*)  tolua_getstring(tolua_S,1,0));
 return 0;
}

/* get function: k_text */
static int toluaI_get_object_k_text(lua_State* tolua_S)
{
 tolua_pushstring(tolua_S,(const char*)k_text);
 return 1;
}

/* set function: k_text */
static int toluaI_set_object_k_text(lua_State* tolua_S)
{
 if (!tolua_istype(tolua_S,1,LUA_TSTRING,0))
 TOLUA_ERR_ASSIGN;
  k_text = ((char*)  tolua_getstring(tolua_S,1,0));
 return 0;
}

/* get function: a_info */
static int toluaI_get_object_a_info(lua_State* tolua_S)
{
 int toluaI_index;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=z_info->a_max)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushusertype(tolua_S,(void*)&a_info[toluaI_index],tolua_tag(tolua_S,"artifact_type"));
 return 1;
}

/* set function: a_info */
static int toluaI_set_object_a_info(lua_State* tolua_S)
{
 int toluaI_index;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=z_info->a_max)
 tolua_error(tolua_S,"array indexing out of range.");
  a_info[toluaI_index] = *((artifact_type*)  tolua_getusertype(tolua_S,3,0));
 return 0;
}

/* get function: a_name */
static int toluaI_get_object_a_name(lua_State* tolua_S)
{
 tolua_pushstring(tolua_S,(const char*)a_name);
 return 1;
}

/* set function: a_name */
static int toluaI_set_object_a_name(lua_State* tolua_S)
{
 if (!tolua_istype(tolua_S,1,LUA_TSTRING,0))
 TOLUA_ERR_ASSIGN;
  a_name = ((char*)  tolua_getstring(tolua_S,1,0));
 return 0;
}

/* get function: a_text */
static int toluaI_get_object_a_text(lua_State* tolua_S)
{
 tolua_pushstring(tolua_S,(const char*)a_text);
 return 1;
}

/* set function: a_text */
static int toluaI_set_object_a_text(lua_State* tolua_S)
{
 if (!tolua_istype(tolua_S,1,LUA_TSTRING,0))
 TOLUA_ERR_ASSIGN;
  a_text = ((char*)  tolua_getstring(tolua_S,1,0));
 return 0;
}

/* get function: e_info */
static int toluaI_get_object_e_info(lua_State* tolua_S)
{
 int toluaI_index;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=z_info->e_max)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushusertype(tolua_S,(void*)&e_info[toluaI_index],tolua_tag(tolua_S,"ego_item_type"));
 return 1;
}

/* set function: e_info */
static int toluaI_set_object_e_info(lua_State* tolua_S)
{
 int toluaI_index;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=z_info->e_max)
 tolua_error(tolua_S,"array indexing out of range.");
  e_info[toluaI_index] = *((ego_item_type*)  tolua_getusertype(tolua_S,3,0));
 return 0;
}

/* get function: e_name */
static int toluaI_get_object_e_name(lua_State* tolua_S)
{
 tolua_pushstring(tolua_S,(const char*)e_name);
 return 1;
}

/* set function: e_name */
static int toluaI_set_object_e_name(lua_State* tolua_S)
{
 if (!tolua_istype(tolua_S,1,LUA_TSTRING,0))
 TOLUA_ERR_ASSIGN;
  e_name = ((char*)  tolua_getstring(tolua_S,1,0));
 return 0;
}

/* get function: e_text */
static int toluaI_get_object_e_text(lua_State* tolua_S)
{
 tolua_pushstring(tolua_S,(const char*)e_text);
 return 1;
}

/* set function: e_text */
static int toluaI_set_object_e_text(lua_State* tolua_S)
{
 if (!tolua_istype(tolua_S,1,LUA_TSTRING,0))
 TOLUA_ERR_ASSIGN;
  e_text = ((char*)  tolua_getstring(tolua_S,1,0));
 return 0;
}

/* get function: flavor_info */
static int toluaI_get_object_flavor_info(lua_State* tolua_S)
{
 int toluaI_index;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=z_info->flavor_max)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushusertype(tolua_S,(void*)&flavor_info[toluaI_index],tolua_tag(tolua_S,"flavor_type"));
 return 1;
}

/* set function: flavor_info */
static int toluaI_set_object_flavor_info(lua_State* tolua_S)
{
 int toluaI_index;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=z_info->flavor_max)
 tolua_error(tolua_S,"array indexing out of range.");
  flavor_info[toluaI_index] = *((flavor_type*)  tolua_getusertype(tolua_S,3,0));
 return 0;
}

/* get function: flavor_name */
static int toluaI_get_object_flavor_name(lua_State* tolua_S)
{
 tolua_pushstring(tolua_S,(const char*)flavor_name);
 return 1;
}

/* set function: flavor_name */
static int toluaI_set_object_flavor_name(lua_State* tolua_S)
{
 if (!tolua_istype(tolua_S,1,LUA_TSTRING,0))
 TOLUA_ERR_ASSIGN;
  flavor_name = ((char*)  tolua_getstring(tolua_S,1,0));
 return 0;
}

/* get function: flavor_text */
static int toluaI_get_object_flavor_text(lua_State* tolua_S)
{
 tolua_pushstring(tolua_S,(const char*)flavor_text);
 return 1;
}

/* set function: flavor_text */
static int toluaI_set_object_flavor_text(lua_State* tolua_S)
{
 if (!tolua_istype(tolua_S,1,LUA_TSTRING,0))
 TOLUA_ERR_ASSIGN;
  flavor_text = ((char*)  tolua_getstring(tolua_S,1,0));
 return 0;
}

/* function: flavor_init */
static int toluaI_object_flavor_init00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  flavor_init();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'flavor_init'.");
 return 0;
}

/* function: reset_visuals */
static int toluaI_object_reset_visuals00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"bool"),0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  bool prefs = ((bool)  tolua_getbool(tolua_S,1,0));
 {
  reset_visuals(prefs);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'reset_visuals'.");
 return 0;
}

/* function: object_flags */
static int toluaI_object_object_flags00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"const object_type"),0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,4,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,5)
 )
 goto tolua_lerror;
 else
 {
  const object_type* o_ptr = ((const object_type*)  tolua_getusertype(tolua_S,1,0));
  u32b f1 = ((u32b)  tolua_getnumber(tolua_S,2,0));
  u32b f2 = ((u32b)  tolua_getnumber(tolua_S,3,0));
  u32b f3 = ((u32b)  tolua_getnumber(tolua_S,4,0));
 {
  object_flags(o_ptr,&f1,&f2,&f3);
 tolua_pushnumber(tolua_S,(long)f1);
 tolua_pushnumber(tolua_S,(long)f2);
 tolua_pushnumber(tolua_S,(long)f3);
 }
 }
 return 3;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_flags'.");
 return 0;
}

/* function: object_flags_known */
static int toluaI_object_object_flags_known00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"const object_type"),0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,4,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,5)
 )
 goto tolua_lerror;
 else
 {
  const object_type* o_ptr = ((const object_type*)  tolua_getusertype(tolua_S,1,0));
  u32b f1 = ((u32b)  tolua_getnumber(tolua_S,2,0));
  u32b f2 = ((u32b)  tolua_getnumber(tolua_S,3,0));
  u32b f3 = ((u32b)  tolua_getnumber(tolua_S,4,0));
 {
  object_flags_known(o_ptr,&f1,&f2,&f3);
 tolua_pushnumber(tolua_S,(long)f1);
 tolua_pushnumber(tolua_S,(long)f2);
 tolua_pushnumber(tolua_S,(long)f3);
 }
 }
 return 3;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_flags_known'.");
 return 0;
}

/* function: identify_random_gen */
static int toluaI_object_identify_random_gen00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"const object_type"),0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  const object_type* o_ptr = ((const object_type*)  tolua_getusertype(tolua_S,1,0));
 {
  identify_random_gen(o_ptr);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'identify_random_gen'.");
 return 0;
}

/* function: identify_fully_aux */
static int toluaI_object_identify_fully_aux00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"const object_type"),0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  const object_type* o_ptr = ((const object_type*)  tolua_getusertype(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  identify_fully_aux(o_ptr);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'identify_fully_aux'.");
 return 0;
}

/* function: index_to_label */
static int toluaI_object_index_to_label00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int i = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  char toluaI_ret = (char)  index_to_label(i);
 tolua_pushnumber(tolua_S,(long)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'index_to_label'.");
 return 0;
}

/* function: label_to_inven */
static int toluaI_object_label_to_inven00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int c = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  s16b toluaI_ret = (s16b)  label_to_inven(c);
 tolua_pushnumber(tolua_S,(long)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'label_to_inven'.");
 return 0;
}

/* function: label_to_equip */
static int toluaI_object_label_to_equip00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int c = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  s16b toluaI_ret = (s16b)  label_to_equip(c);
 tolua_pushnumber(tolua_S,(long)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'label_to_equip'.");
 return 0;
}

/* function: wield_slot */
static int toluaI_object_wield_slot00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"const object_type"),0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  const object_type* o_ptr = ((const object_type*)  tolua_getusertype(tolua_S,1,0));
 {
  s16b toluaI_ret = (s16b)  wield_slot(o_ptr);
 tolua_pushnumber(tolua_S,(long)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'wield_slot'.");
 return 0;
}

/* function: mention_use */
static int toluaI_object_mention_use00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int i = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  cptr toluaI_ret = (cptr)  mention_use(i);
 tolua_pushstring(tolua_S,(const char*)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'mention_use'.");
 return 0;
}

/* function: describe_use */
static int toluaI_object_describe_use00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int i = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  cptr toluaI_ret = (cptr)  describe_use(i);
 tolua_pushstring(tolua_S,(const char*)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'describe_use'.");
 return 0;
}

/* function: item_tester_okay */
static int toluaI_object_item_tester_okay00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"const object_type"),0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  const object_type* o_ptr = ((const object_type*)  tolua_getusertype(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  item_tester_okay(o_ptr);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'item_tester_okay'.");
 return 0;
}

/* function: scan_floor */
static int toluaI_object_scan_floor00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,4,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,5,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,6)
 )
 goto tolua_lerror;
 else
 {
  int items = ((int)  tolua_getnumber(tolua_S,1,0));
  int size = ((int)  tolua_getnumber(tolua_S,2,0));
  int y = ((int)  tolua_getnumber(tolua_S,3,0));
  int x = ((int)  tolua_getnumber(tolua_S,4,0));
  int mode = ((int)  tolua_getnumber(tolua_S,5,0));
 {
  sint toluaI_ret = (sint)  scan_floor(&items,size,y,x,mode);
 tolua_pushnumber(tolua_S,(long)toluaI_ret);
 tolua_pushnumber(tolua_S,(long)items);
 }
 }
 return 2;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'scan_floor'.");
 return 0;
}

/* function: display_inven */
static int toluaI_object_display_inven00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  display_inven();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'display_inven'.");
 return 0;
}

/* function: display_equip */
static int toluaI_object_display_equip00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  display_equip();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'display_equip'.");
 return 0;
}

/* function: show_inven */
static int toluaI_object_show_inven00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  show_inven();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'show_inven'.");
 return 0;
}

/* function: show_equip */
static int toluaI_object_show_equip00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  show_equip();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'show_equip'.");
 return 0;
}

/* function: toggle_inven_equip */
static int toluaI_object_toggle_inven_equip00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  toggle_inven_equip();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'toggle_inven_equip'.");
 return 0;
}

/* function: get_item */
static int toluaI_object_get_item00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TSTRING,0) ||
 !tolua_istype(tolua_S,3,LUA_TSTRING,0) ||
 !tolua_istype(tolua_S,4,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,5)
 )
 goto tolua_lerror;
 else
 {
  int cp = ((int)  tolua_getnumber(tolua_S,1,0));
  cptr pmt = ((cptr)  tolua_getstring(tolua_S,2,0));
  cptr str = ((cptr)  tolua_getstring(tolua_S,3,0));
  int mode = ((int)  tolua_getnumber(tolua_S,4,0));
 {
  bool toluaI_ret = (bool)  get_item(&cp,pmt,str,mode);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 tolua_pushnumber(tolua_S,(long)cp);
 }
 }
 return 2;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'get_item'.");
 return 0;
}

/* function: excise_object_idx */
static int toluaI_object_excise_object_idx00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int o_idx = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  excise_object_idx(o_idx);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'excise_object_idx'.");
 return 0;
}

/* function: delete_object_idx */
static int toluaI_object_delete_object_idx00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int o_idx = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  delete_object_idx(o_idx);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete_object_idx'.");
 return 0;
}

/* function: delete_object */
static int toluaI_object_delete_object00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int y = ((int)  tolua_getnumber(tolua_S,1,0));
  int x = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  delete_object(y,x);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete_object'.");
 return 0;
}

/* function: compact_objects */
static int toluaI_object_compact_objects00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int size = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  compact_objects(size);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'compact_objects'.");
 return 0;
}

/* function: wipe_o_list */
static int toluaI_object_wipe_o_list00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  wipe_o_list();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'wipe_o_list'.");
 return 0;
}

/* function: o_pop */
static int toluaI_object_o_pop00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  s16b toluaI_ret = (s16b)  o_pop();
 tolua_pushnumber(tolua_S,(long)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'o_pop'.");
 return 0;
}

/* function: get_obj_num_prep */
static int toluaI_object_get_obj_num_prep00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  errr toluaI_ret = (errr)  get_obj_num_prep();
 tolua_pushnumber(tolua_S,(long)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'get_obj_num_prep'.");
 return 0;
}

/* function: get_obj_num */
static int toluaI_object_get_obj_num00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int level = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  s16b toluaI_ret = (s16b)  get_obj_num(level);
 tolua_pushnumber(tolua_S,(long)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'get_obj_num'.");
 return 0;
}

/* function: object_known */
static int toluaI_object_object_known00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"object_type"),0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  object_type* o_ptr = ((object_type*)  tolua_getusertype(tolua_S,1,0));
 {
  object_known(o_ptr);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_known'.");
 return 0;
}

/* function: object_aware */
static int toluaI_object_object_aware00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"object_type"),0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  object_type* o_ptr = ((object_type*)  tolua_getusertype(tolua_S,1,0));
 {
  object_aware(o_ptr);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_aware'.");
 return 0;
}

/* function: object_tried */
static int toluaI_object_object_tried00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"object_type"),0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  object_type* o_ptr = ((object_type*)  tolua_getusertype(tolua_S,1,0));
 {
  object_tried(o_ptr);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_tried'.");
 return 0;
}

/* function: object_value */
static int toluaI_object_object_value00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"const object_type"),0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  const object_type* o_ptr = ((const object_type*)  tolua_getusertype(tolua_S,1,0));
 {
  s32b toluaI_ret = (s32b)  object_value(o_ptr);
 tolua_pushnumber(tolua_S,(long)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_value'.");
 return 0;
}

/* function: object_similar */
static int toluaI_object_object_similar00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"const object_type"),0) ||
 !tolua_istype(tolua_S,2,tolua_tag(tolua_S,"const object_type"),0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  const object_type* o_ptr = ((const object_type*)  tolua_getusertype(tolua_S,1,0));
  const object_type* j_ptr = ((const object_type*)  tolua_getusertype(tolua_S,2,0));
 {
  bool toluaI_ret = (bool)  object_similar(o_ptr,j_ptr);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_similar'.");
 return 0;
}

/* function: object_absorb */
static int toluaI_object_object_absorb00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"object_type"),0) ||
 !tolua_istype(tolua_S,2,tolua_tag(tolua_S,"const object_type"),0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  object_type* o_ptr = ((object_type*)  tolua_getusertype(tolua_S,1,0));
  const object_type* j_ptr = ((const object_type*)  tolua_getusertype(tolua_S,2,0));
 {
  object_absorb(o_ptr,j_ptr);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_absorb'.");
 return 0;
}

/* function: lookup_kind */
static int toluaI_object_lookup_kind00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int tval = ((int)  tolua_getnumber(tolua_S,1,0));
  int sval = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  s16b toluaI_ret = (s16b)  lookup_kind(tval,sval);
 tolua_pushnumber(tolua_S,(long)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'lookup_kind'.");
 return 0;
}

/* function: object_wipe */
static int toluaI_object_object_wipe00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"object_type"),0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  object_type* o_ptr = ((object_type*)  tolua_getusertype(tolua_S,1,0));
 {
  object_wipe(o_ptr);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_wipe'.");
 return 0;
}

/* function: object_copy */
static int toluaI_object_object_copy00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"object_type"),0) ||
 !tolua_istype(tolua_S,2,tolua_tag(tolua_S,"const object_type"),0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  object_type* o_ptr = ((object_type*)  tolua_getusertype(tolua_S,1,0));
  const object_type* j_ptr = ((const object_type*)  tolua_getusertype(tolua_S,2,0));
 {
  object_copy(o_ptr,j_ptr);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_copy'.");
 return 0;
}

/* function: object_prep */
static int toluaI_object_object_prep00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"object_type"),0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  object_type* o_ptr = ((object_type*)  tolua_getusertype(tolua_S,1,0));
  int k_idx = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  object_prep(o_ptr,k_idx);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_prep'.");
 return 0;
}

/* function: apply_magic */
static int toluaI_object_apply_magic00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"object_type"),0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,tolua_tag(tolua_S,"bool"),0) ||
 !tolua_istype(tolua_S,4,tolua_tag(tolua_S,"bool"),0) ||
 !tolua_istype(tolua_S,5,tolua_tag(tolua_S,"bool"),0) ||
 !tolua_isnoobj(tolua_S,6)
 )
 goto tolua_lerror;
 else
 {
  object_type* o_ptr = ((object_type*)  tolua_getusertype(tolua_S,1,0));
  int lev = ((int)  tolua_getnumber(tolua_S,2,0));
  bool okay = ((bool)  tolua_getbool(tolua_S,3,0));
  bool good = ((bool)  tolua_getbool(tolua_S,4,0));
  bool great = ((bool)  tolua_getbool(tolua_S,5,0));
 {
  apply_magic(o_ptr,lev,okay,good,great);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'apply_magic'.");
 return 0;
}

/* function: make_object */
static int toluaI_object_make_object00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"object_type"),0) ||
 !tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0) ||
 !tolua_istype(tolua_S,3,tolua_tag(tolua_S,"bool"),0) ||
 !tolua_isnoobj(tolua_S,4)
 )
 goto tolua_lerror;
 else
 {
  object_type* j_ptr = ((object_type*)  tolua_getusertype(tolua_S,1,0));
  bool good = ((bool)  tolua_getbool(tolua_S,2,0));
  bool great = ((bool)  tolua_getbool(tolua_S,3,0));
 {
  bool toluaI_ret = (bool)  make_object(j_ptr,good,great);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'make_object'.");
 return 0;
}

/* function: make_gold */
static int toluaI_object_make_gold00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"object_type"),0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  object_type* j_ptr = ((object_type*)  tolua_getusertype(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  make_gold(j_ptr);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'make_gold'.");
 return 0;
}

/* function: floor_carry */
static int toluaI_object_floor_carry00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,tolua_tag(tolua_S,"object_type"),0) ||
 !tolua_isnoobj(tolua_S,4)
 )
 goto tolua_lerror;
 else
 {
  int y = ((int)  tolua_getnumber(tolua_S,1,0));
  int x = ((int)  tolua_getnumber(tolua_S,2,0));
  object_type* j_ptr = ((object_type*)  tolua_getusertype(tolua_S,3,0));
 {
  s16b toluaI_ret = (s16b)  floor_carry(y,x,j_ptr);
 tolua_pushnumber(tolua_S,(long)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'floor_carry'.");
 return 0;
}

/* function: drop_near */
static int toluaI_object_drop_near00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"object_type"),0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,4,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,5)
 )
 goto tolua_lerror;
 else
 {
  object_type* j_ptr = ((object_type*)  tolua_getusertype(tolua_S,1,0));
  int chance = ((int)  tolua_getnumber(tolua_S,2,0));
  int y = ((int)  tolua_getnumber(tolua_S,3,0));
  int x = ((int)  tolua_getnumber(tolua_S,4,0));
 {
  drop_near(j_ptr,chance,y,x);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'drop_near'.");
 return 0;
}

/* function: acquirement */
static int toluaI_object_acquirement00(lua_State* tolua_S)
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
  int num = ((int)  tolua_getnumber(tolua_S,3,0));
  bool great = ((bool)  tolua_getbool(tolua_S,4,0));
 {
  acquirement(y1,x1,num,great);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'acquirement'.");
 return 0;
}

/* function: place_object */
static int toluaI_object_place_object00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,tolua_tag(tolua_S,"bool"),0) ||
 !tolua_istype(tolua_S,4,tolua_tag(tolua_S,"bool"),0) ||
 !tolua_isnoobj(tolua_S,5)
 )
 goto tolua_lerror;
 else
 {
  int y = ((int)  tolua_getnumber(tolua_S,1,0));
  int x = ((int)  tolua_getnumber(tolua_S,2,0));
  bool good = ((bool)  tolua_getbool(tolua_S,3,0));
  bool great = ((bool)  tolua_getbool(tolua_S,4,0));
 {
  place_object(y,x,good,great);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'place_object'.");
 return 0;
}

/* function: place_gold */
static int toluaI_object_place_gold00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int y = ((int)  tolua_getnumber(tolua_S,1,0));
  int x = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  place_gold(y,x);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'place_gold'.");
 return 0;
}

/* function: pick_trap */
static int toluaI_object_pick_trap00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int y = ((int)  tolua_getnumber(tolua_S,1,0));
  int x = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  pick_trap(y,x);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'pick_trap'.");
 return 0;
}

/* function: place_trap */
static int toluaI_object_place_trap00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int y = ((int)  tolua_getnumber(tolua_S,1,0));
  int x = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  place_trap(y,x);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'place_trap'.");
 return 0;
}

/* function: place_secret_door */
static int toluaI_object_place_secret_door00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int y = ((int)  tolua_getnumber(tolua_S,1,0));
  int x = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  place_secret_door(y,x);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'place_secret_door'.");
 return 0;
}

/* function: place_closed_door */
static int toluaI_object_place_closed_door00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int y = ((int)  tolua_getnumber(tolua_S,1,0));
  int x = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  place_closed_door(y,x);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'place_closed_door'.");
 return 0;
}

/* function: place_random_door */
static int toluaI_object_place_random_door00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int y = ((int)  tolua_getnumber(tolua_S,1,0));
  int x = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  place_random_door(y,x);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'place_random_door'.");
 return 0;
}

/* function: inven_item_charges */
static int toluaI_object_inven_item_charges00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int item = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  inven_item_charges(item);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'inven_item_charges'.");
 return 0;
}

/* function: inven_item_describe */
static int toluaI_object_inven_item_describe00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int item = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  inven_item_describe(item);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'inven_item_describe'.");
 return 0;
}

/* function: inven_item_increase */
static int toluaI_object_inven_item_increase00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int item = ((int)  tolua_getnumber(tolua_S,1,0));
  int num = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  inven_item_increase(item,num);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'inven_item_increase'.");
 return 0;
}

/* function: inven_item_optimize */
static int toluaI_object_inven_item_optimize00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int item = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  inven_item_optimize(item);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'inven_item_optimize'.");
 return 0;
}

/* function: floor_item_charges */
static int toluaI_object_floor_item_charges00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int item = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  floor_item_charges(item);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'floor_item_charges'.");
 return 0;
}

/* function: floor_item_describe */
static int toluaI_object_floor_item_describe00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int item = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  floor_item_describe(item);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'floor_item_describe'.");
 return 0;
}

/* function: floor_item_increase */
static int toluaI_object_floor_item_increase00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int item = ((int)  tolua_getnumber(tolua_S,1,0));
  int num = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  floor_item_increase(item,num);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'floor_item_increase'.");
 return 0;
}

/* function: floor_item_optimize */
static int toluaI_object_floor_item_optimize00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int item = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  floor_item_optimize(item);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'floor_item_optimize'.");
 return 0;
}

/* function: inven_carry_okay */
static int toluaI_object_inven_carry_okay00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"const object_type"),0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  const object_type* o_ptr = ((const object_type*)  tolua_getusertype(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  inven_carry_okay(o_ptr);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'inven_carry_okay'.");
 return 0;
}

/* function: inven_carry */
static int toluaI_object_inven_carry00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"object_type"),0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  object_type* o_ptr = ((object_type*)  tolua_getusertype(tolua_S,1,0));
 {
  s16b toluaI_ret = (s16b)  inven_carry(o_ptr);
 tolua_pushnumber(tolua_S,(long)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'inven_carry'.");
 return 0;
}

/* function: inven_takeoff */
static int toluaI_object_inven_takeoff00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int item = ((int)  tolua_getnumber(tolua_S,1,0));
  int amt = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  s16b toluaI_ret = (s16b)  inven_takeoff(item,amt);
 tolua_pushnumber(tolua_S,(long)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'inven_takeoff'.");
 return 0;
}

/* function: inven_drop */
static int toluaI_object_inven_drop00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int item = ((int)  tolua_getnumber(tolua_S,1,0));
  int amt = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  inven_drop(item,amt);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'inven_drop'.");
 return 0;
}

/* function: combine_pack */
static int toluaI_object_combine_pack00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  combine_pack();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'combine_pack'.");
 return 0;
}

/* function: reorder_pack */
static int toluaI_object_reorder_pack00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  reorder_pack();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'reorder_pack'.");
 return 0;
}

/* function: object_aware_p */
static int toluaI_object_object_aware_p00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"const object_type"),0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  const object_type* o_ptr = ((const object_type*)  tolua_getusertype(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  object_aware_p(o_ptr);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_aware_p'.");
 return 0;
}

/* function: object_tried_p */
static int toluaI_object_object_tried_p00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"const object_type"),0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  const object_type* o_ptr = ((const object_type*)  tolua_getusertype(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  object_tried_p(o_ptr);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_tried_p'.");
 return 0;
}

/* function: object_known_p */
static int toluaI_object_object_known_p00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"const object_type"),0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  const object_type* o_ptr = ((const object_type*)  tolua_getusertype(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  object_known_p(o_ptr);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_known_p'.");
 return 0;
}

/* function: object_attr */
static int toluaI_object_object_attr00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"const object_type"),0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  const object_type* o_ptr = ((const object_type*)  tolua_getusertype(tolua_S,1,0));
 {
  byte toluaI_ret = (byte)  object_attr(o_ptr);
 tolua_pushnumber(tolua_S,(long)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_attr'.");
 return 0;
}

/* function: object_char */
static int toluaI_object_object_char00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"const object_type"),0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  const object_type* o_ptr = ((const object_type*)  tolua_getusertype(tolua_S,1,0));
 {
  char toluaI_ret = (char)  object_char(o_ptr);
 tolua_pushnumber(tolua_S,(long)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'object_char'.");
 return 0;
}

/* function: artifact_p */
static int toluaI_object_artifact_p00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"const object_type"),0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  const object_type* o_ptr = ((const object_type*)  tolua_getusertype(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  artifact_p(o_ptr);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'artifact_p'.");
 return 0;
}

/* function: ego_item_p */
static int toluaI_object_ego_item_p00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"const object_type"),0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  const object_type* o_ptr = ((const object_type*)  tolua_getusertype(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  ego_item_p(o_ptr);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ego_item_p'.");
 return 0;
}

/* function: broken_p */
static int toluaI_object_broken_p00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"const object_type"),0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  const object_type* o_ptr = ((const object_type*)  tolua_getusertype(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  broken_p(o_ptr);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'broken_p'.");
 return 0;
}

/* function: cursed_p */
static int toluaI_object_cursed_p00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"const object_type"),0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  const object_type* o_ptr = ((const object_type*)  tolua_getusertype(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  cursed_p(o_ptr);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'cursed_p'.");
 return 0;
}

/* Open function */
int tolua_object_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 toluaI_reg_types(tolua_S);
 tolua_constant(tolua_S,NULL,"ART_POWER",ART_POWER);
 tolua_constant(tolua_S,NULL,"ART_MORGOTH",ART_MORGOTH);
 tolua_constant(tolua_S,NULL,"ART_GROND",ART_GROND);
 tolua_constant(tolua_S,NULL,"ART_MIN_NORMAL",ART_MIN_NORMAL);
 tolua_constant(tolua_S,NULL,"EGO_RESIST_ACID",EGO_RESIST_ACID);
 tolua_constant(tolua_S,NULL,"EGO_RESIST_ELEC",EGO_RESIST_ELEC);
 tolua_constant(tolua_S,NULL,"EGO_RESIST_FIRE",EGO_RESIST_FIRE);
 tolua_constant(tolua_S,NULL,"EGO_RESIST_COLD",EGO_RESIST_COLD);
 tolua_constant(tolua_S,NULL,"EGO_RESISTANCE",EGO_RESISTANCE);
 tolua_constant(tolua_S,NULL,"EGO_ELVENKIND",EGO_ELVENKIND);
 tolua_constant(tolua_S,NULL,"EGO_ARMR_VULN",EGO_ARMR_VULN);
 tolua_constant(tolua_S,NULL,"EGO_PERMANENCE",EGO_PERMANENCE);
 tolua_constant(tolua_S,NULL,"EGO_ARMR_DWARVEN",EGO_ARMR_DWARVEN);
 tolua_constant(tolua_S,NULL,"EGO_ENDURE_ACID",EGO_ENDURE_ACID);
 tolua_constant(tolua_S,NULL,"EGO_ENDURE_ELEC",EGO_ENDURE_ELEC);
 tolua_constant(tolua_S,NULL,"EGO_ENDURE_FIRE",EGO_ENDURE_FIRE);
 tolua_constant(tolua_S,NULL,"EGO_ENDURE_COLD",EGO_ENDURE_COLD);
 tolua_constant(tolua_S,NULL,"EGO_ENDURANCE",EGO_ENDURANCE);
 tolua_constant(tolua_S,NULL,"EGO_SHIELD_ELVENKIND",EGO_SHIELD_ELVENKIND);
 tolua_constant(tolua_S,NULL,"EGO_SHIELD_PRESERVATION",EGO_SHIELD_PRESERVATION);
 tolua_constant(tolua_S,NULL,"EGO_SHIELD_VULN",EGO_SHIELD_VULN);
 tolua_constant(tolua_S,NULL,"EGO_INTELLIGENCE",EGO_INTELLIGENCE);
 tolua_constant(tolua_S,NULL,"EGO_WISDOM",EGO_WISDOM);
 tolua_constant(tolua_S,NULL,"EGO_BEAUTY",EGO_BEAUTY);
 tolua_constant(tolua_S,NULL,"EGO_MAGI",EGO_MAGI);
 tolua_constant(tolua_S,NULL,"EGO_MIGHT",EGO_MIGHT);
 tolua_constant(tolua_S,NULL,"EGO_LORDLINESS",EGO_LORDLINESS);
 tolua_constant(tolua_S,NULL,"EGO_SEEING",EGO_SEEING);
 tolua_constant(tolua_S,NULL,"EGO_INFRAVISION",EGO_INFRAVISION);
 tolua_constant(tolua_S,NULL,"EGO_LITE",EGO_LITE);
 tolua_constant(tolua_S,NULL,"EGO_TELEPATHY",EGO_TELEPATHY);
 tolua_constant(tolua_S,NULL,"EGO_REGENERATION",EGO_REGENERATION);
 tolua_constant(tolua_S,NULL,"EGO_TELEPORTATION",EGO_TELEPORTATION);
 tolua_constant(tolua_S,NULL,"EGO_STUPIDITY",EGO_STUPIDITY);
 tolua_constant(tolua_S,NULL,"EGO_NAIVETY",EGO_NAIVETY);
 tolua_constant(tolua_S,NULL,"EGO_UGLINESS",EGO_UGLINESS);
 tolua_constant(tolua_S,NULL,"EGO_SICKLINESS",EGO_SICKLINESS);
 tolua_constant(tolua_S,NULL,"EGO_PROTECTION",EGO_PROTECTION);
 tolua_constant(tolua_S,NULL,"EGO_STEALTH",EGO_STEALTH);
 tolua_constant(tolua_S,NULL,"EGO_AMAN",EGO_AMAN);
 tolua_constant(tolua_S,NULL,"EGO_CLOAK_MAGI",EGO_CLOAK_MAGI);
 tolua_constant(tolua_S,NULL,"EGO_ENVELOPING",EGO_ENVELOPING);
 tolua_constant(tolua_S,NULL,"EGO_VULNERABILITY",EGO_VULNERABILITY);
 tolua_constant(tolua_S,NULL,"EGO_IRRITATION",EGO_IRRITATION);
 tolua_constant(tolua_S,NULL,"EGO_FREE_ACTION",EGO_FREE_ACTION);
 tolua_constant(tolua_S,NULL,"EGO_SLAYING",EGO_SLAYING);
 tolua_constant(tolua_S,NULL,"EGO_AGILITY",EGO_AGILITY);
 tolua_constant(tolua_S,NULL,"EGO_POWER",EGO_POWER);
 tolua_constant(tolua_S,NULL,"EGO_GLOVES_THIEVERY",EGO_GLOVES_THIEVERY);
 tolua_constant(tolua_S,NULL,"EGO_GAUNTLETS_COMBAT",EGO_GAUNTLETS_COMBAT);
 tolua_constant(tolua_S,NULL,"EGO_WEAKNESS",EGO_WEAKNESS);
 tolua_constant(tolua_S,NULL,"EGO_CLUMSINESS",EGO_CLUMSINESS);
 tolua_constant(tolua_S,NULL,"EGO_SLOW_DESCENT",EGO_SLOW_DESCENT);
 tolua_constant(tolua_S,NULL,"EGO_QUIET",EGO_QUIET);
 tolua_constant(tolua_S,NULL,"EGO_MOTION",EGO_MOTION);
 tolua_constant(tolua_S,NULL,"EGO_SPEED",EGO_SPEED);
 tolua_constant(tolua_S,NULL,"EGO_STABILITY",EGO_STABILITY);
 tolua_constant(tolua_S,NULL,"EGO_NOISE",EGO_NOISE);
 tolua_constant(tolua_S,NULL,"EGO_SLOWNESS",EGO_SLOWNESS);
 tolua_constant(tolua_S,NULL,"EGO_ANNOYANCE",EGO_ANNOYANCE);
 tolua_constant(tolua_S,NULL,"EGO_HA",EGO_HA);
 tolua_constant(tolua_S,NULL,"EGO_DF",EGO_DF);
 tolua_constant(tolua_S,NULL,"EGO_BLESS_BLADE",EGO_BLESS_BLADE);
 tolua_constant(tolua_S,NULL,"EGO_GONDOLIN",EGO_GONDOLIN);
 tolua_constant(tolua_S,NULL,"EGO_WEST",EGO_WEST);
 tolua_constant(tolua_S,NULL,"EGO_ATTACKS",EGO_ATTACKS);
 tolua_constant(tolua_S,NULL,"EGO_FURY",EGO_FURY);
 tolua_constant(tolua_S,NULL,"EGO_BRAND_ACID",EGO_BRAND_ACID);
 tolua_constant(tolua_S,NULL,"EGO_BRAND_ELEC",EGO_BRAND_ELEC);
 tolua_constant(tolua_S,NULL,"EGO_BRAND_FIRE",EGO_BRAND_FIRE);
 tolua_constant(tolua_S,NULL,"EGO_BRAND_COLD",EGO_BRAND_COLD);
 tolua_constant(tolua_S,NULL,"EGO_BRAND_POIS",EGO_BRAND_POIS);
 tolua_constant(tolua_S,NULL,"EGO_SLAY_ANIMAL",EGO_SLAY_ANIMAL);
 tolua_constant(tolua_S,NULL,"EGO_SLAY_EVIL",EGO_SLAY_EVIL);
 tolua_constant(tolua_S,NULL,"EGO_SLAY_UNDEAD",EGO_SLAY_UNDEAD);
 tolua_constant(tolua_S,NULL,"EGO_SLAY_DEMON",EGO_SLAY_DEMON);
 tolua_constant(tolua_S,NULL,"EGO_SLAY_ORC",EGO_SLAY_ORC);
 tolua_constant(tolua_S,NULL,"EGO_SLAY_TROLL",EGO_SLAY_TROLL);
 tolua_constant(tolua_S,NULL,"EGO_SLAY_GIANT",EGO_SLAY_GIANT);
 tolua_constant(tolua_S,NULL,"EGO_SLAY_DRAGON",EGO_SLAY_DRAGON);
 tolua_constant(tolua_S,NULL,"EGO_KILL_ANIMAL",EGO_KILL_ANIMAL);
 tolua_constant(tolua_S,NULL,"EGO_KILL_EVIL",EGO_KILL_EVIL);
 tolua_constant(tolua_S,NULL,"EGO_KILL_UNDEAD",EGO_KILL_UNDEAD);
 tolua_constant(tolua_S,NULL,"EGO_KILL_DEMON",EGO_KILL_DEMON);
 tolua_constant(tolua_S,NULL,"EGO_KILL_ORC",EGO_KILL_ORC);
 tolua_constant(tolua_S,NULL,"EGO_KILL_TROLL",EGO_KILL_TROLL);
 tolua_constant(tolua_S,NULL,"EGO_KILL_GIANT",EGO_KILL_GIANT);
 tolua_constant(tolua_S,NULL,"EGO_KILL_DRAGON",EGO_KILL_DRAGON);
 tolua_constant(tolua_S,NULL,"EGO_DIGGING",EGO_DIGGING);
 tolua_constant(tolua_S,NULL,"EGO_DIGGER_EARTHQUAKE",EGO_DIGGER_EARTHQUAKE);
 tolua_constant(tolua_S,NULL,"EGO_MORGUL",EGO_MORGUL);
 tolua_constant(tolua_S,NULL,"EGO_ACCURACY",EGO_ACCURACY);
 tolua_constant(tolua_S,NULL,"EGO_VELOCITY",EGO_VELOCITY);
 tolua_constant(tolua_S,NULL,"EGO_BOW_LORIEN",EGO_BOW_LORIEN);
 tolua_constant(tolua_S,NULL,"EGO_CROSSBOW_HARAD",EGO_CROSSBOW_HARAD);
 tolua_constant(tolua_S,NULL,"EGO_EXTRA_MIGHT",EGO_EXTRA_MIGHT);
 tolua_constant(tolua_S,NULL,"EGO_EXTRA_SHOTS",EGO_EXTRA_SHOTS);
 tolua_constant(tolua_S,NULL,"EGO_SLING_BUCKLAND",EGO_SLING_BUCKLAND);
 tolua_constant(tolua_S,NULL,"EGO_NAZGUL",EGO_NAZGUL);
 tolua_constant(tolua_S,NULL,"EGO_HURT_ANIMAL",EGO_HURT_ANIMAL);
 tolua_constant(tolua_S,NULL,"EGO_HURT_EVIL",EGO_HURT_EVIL);
 tolua_constant(tolua_S,NULL,"EGO_HURT_UNDEAD",EGO_HURT_UNDEAD);
 tolua_constant(tolua_S,NULL,"EGO_HURT_DEMON",EGO_HURT_DEMON);
 tolua_constant(tolua_S,NULL,"EGO_HURT_ORC",EGO_HURT_ORC);
 tolua_constant(tolua_S,NULL,"EGO_HURT_TROLL",EGO_HURT_TROLL);
 tolua_constant(tolua_S,NULL,"EGO_HURT_GIANT",EGO_HURT_GIANT);
 tolua_constant(tolua_S,NULL,"EGO_HURT_DRAGON",EGO_HURT_DRAGON);
 tolua_constant(tolua_S,NULL,"EGO_AMMO_HOLY",EGO_AMMO_HOLY);
 tolua_constant(tolua_S,NULL,"EGO_AMMO_VENOM",EGO_AMMO_VENOM);
 tolua_constant(tolua_S,NULL,"EGO_FLAME",EGO_FLAME);
 tolua_constant(tolua_S,NULL,"EGO_FROST",EGO_FROST);
 tolua_constant(tolua_S,NULL,"EGO_WOUNDING",EGO_WOUNDING);
 tolua_constant(tolua_S,NULL,"EGO_BACKBITING",EGO_BACKBITING);
 tolua_constant(tolua_S,NULL,"EGO_SHATTERED",EGO_SHATTERED);
 tolua_constant(tolua_S,NULL,"EGO_BLASTED",EGO_BLASTED);
 tolua_constant(tolua_S,NULL,"TV_SKELETON",TV_SKELETON);
 tolua_constant(tolua_S,NULL,"TV_BOTTLE",TV_BOTTLE);
 tolua_constant(tolua_S,NULL,"TV_JUNK",TV_JUNK);
 tolua_constant(tolua_S,NULL,"TV_SPIKE",TV_SPIKE);
 tolua_constant(tolua_S,NULL,"TV_CHEST",TV_CHEST);
 tolua_constant(tolua_S,NULL,"TV_SHOT",TV_SHOT);
 tolua_constant(tolua_S,NULL,"TV_ARROW",TV_ARROW);
 tolua_constant(tolua_S,NULL,"TV_BOLT",TV_BOLT);
 tolua_constant(tolua_S,NULL,"TV_BOW",TV_BOW);
 tolua_constant(tolua_S,NULL,"TV_DIGGING",TV_DIGGING);
 tolua_constant(tolua_S,NULL,"TV_HAFTED",TV_HAFTED);
 tolua_constant(tolua_S,NULL,"TV_POLEARM",TV_POLEARM);
 tolua_constant(tolua_S,NULL,"TV_SWORD",TV_SWORD);
 tolua_constant(tolua_S,NULL,"TV_BOOTS",TV_BOOTS);
 tolua_constant(tolua_S,NULL,"TV_GLOVES",TV_GLOVES);
 tolua_constant(tolua_S,NULL,"TV_HELM",TV_HELM);
 tolua_constant(tolua_S,NULL,"TV_CROWN",TV_CROWN);
 tolua_constant(tolua_S,NULL,"TV_SHIELD",TV_SHIELD);
 tolua_constant(tolua_S,NULL,"TV_CLOAK",TV_CLOAK);
 tolua_constant(tolua_S,NULL,"TV_SOFT_ARMOR",TV_SOFT_ARMOR);
 tolua_constant(tolua_S,NULL,"TV_HARD_ARMOR",TV_HARD_ARMOR);
 tolua_constant(tolua_S,NULL,"TV_DRAG_ARMOR",TV_DRAG_ARMOR);
 tolua_constant(tolua_S,NULL,"TV_LITE",TV_LITE);
 tolua_constant(tolua_S,NULL,"TV_AMULET",TV_AMULET);
 tolua_constant(tolua_S,NULL,"TV_RING",TV_RING);
 tolua_constant(tolua_S,NULL,"TV_STAFF",TV_STAFF);
 tolua_constant(tolua_S,NULL,"TV_WAND",TV_WAND);
 tolua_constant(tolua_S,NULL,"TV_ROD",TV_ROD);
 tolua_constant(tolua_S,NULL,"TV_SCROLL",TV_SCROLL);
 tolua_constant(tolua_S,NULL,"TV_POTION",TV_POTION);
 tolua_constant(tolua_S,NULL,"TV_FLASK",TV_FLASK);
 tolua_constant(tolua_S,NULL,"TV_FOOD",TV_FOOD);
 tolua_constant(tolua_S,NULL,"TV_MAGIC_BOOK",TV_MAGIC_BOOK);
 tolua_constant(tolua_S,NULL,"TV_PRAYER_BOOK",TV_PRAYER_BOOK);
 tolua_constant(tolua_S,NULL,"TV_GOLD",TV_GOLD);
 tolua_constant(tolua_S,NULL,"SV_AMMO_LIGHT",SV_AMMO_LIGHT);
 tolua_constant(tolua_S,NULL,"SV_AMMO_NORMAL",SV_AMMO_NORMAL);
 tolua_constant(tolua_S,NULL,"SV_AMMO_HEAVY",SV_AMMO_HEAVY);
 tolua_constant(tolua_S,NULL,"SV_SLING",SV_SLING);
 tolua_constant(tolua_S,NULL,"SV_SHORT_BOW",SV_SHORT_BOW);
 tolua_constant(tolua_S,NULL,"SV_LONG_BOW",SV_LONG_BOW);
 tolua_constant(tolua_S,NULL,"SV_LIGHT_XBOW",SV_LIGHT_XBOW);
 tolua_constant(tolua_S,NULL,"SV_HEAVY_XBOW",SV_HEAVY_XBOW);
 tolua_constant(tolua_S,NULL,"SV_SHOVEL",SV_SHOVEL);
 tolua_constant(tolua_S,NULL,"SV_GNOMISH_SHOVEL",SV_GNOMISH_SHOVEL);
 tolua_constant(tolua_S,NULL,"SV_DWARVEN_SHOVEL",SV_DWARVEN_SHOVEL);
 tolua_constant(tolua_S,NULL,"SV_PICK",SV_PICK);
 tolua_constant(tolua_S,NULL,"SV_ORCISH_PICK",SV_ORCISH_PICK);
 tolua_constant(tolua_S,NULL,"SV_DWARVEN_PICK",SV_DWARVEN_PICK);
 tolua_constant(tolua_S,NULL,"SV_WHIP",SV_WHIP);
 tolua_constant(tolua_S,NULL,"SV_QUARTERSTAFF",SV_QUARTERSTAFF);
 tolua_constant(tolua_S,NULL,"SV_MACE",SV_MACE);
 tolua_constant(tolua_S,NULL,"SV_BALL_AND_CHAIN",SV_BALL_AND_CHAIN);
 tolua_constant(tolua_S,NULL,"SV_WAR_HAMMER",SV_WAR_HAMMER);
 tolua_constant(tolua_S,NULL,"SV_LUCERN_HAMMER",SV_LUCERN_HAMMER);
 tolua_constant(tolua_S,NULL,"SV_MORNING_STAR",SV_MORNING_STAR);
 tolua_constant(tolua_S,NULL,"SV_FLAIL",SV_FLAIL);
 tolua_constant(tolua_S,NULL,"SV_LEAD_FILLED_MACE",SV_LEAD_FILLED_MACE);
 tolua_constant(tolua_S,NULL,"SV_TWO_HANDED_FLAIL",SV_TWO_HANDED_FLAIL);
 tolua_constant(tolua_S,NULL,"SV_MACE_OF_DISRUPTION",SV_MACE_OF_DISRUPTION);
 tolua_constant(tolua_S,NULL,"SV_GROND",SV_GROND);
 tolua_constant(tolua_S,NULL,"SV_SPEAR",SV_SPEAR);
 tolua_constant(tolua_S,NULL,"SV_AWL_PIKE",SV_AWL_PIKE);
 tolua_constant(tolua_S,NULL,"SV_TRIDENT",SV_TRIDENT);
 tolua_constant(tolua_S,NULL,"SV_PIKE",SV_PIKE);
 tolua_constant(tolua_S,NULL,"SV_BEAKED_AXE",SV_BEAKED_AXE);
 tolua_constant(tolua_S,NULL,"SV_BROAD_AXE",SV_BROAD_AXE);
 tolua_constant(tolua_S,NULL,"SV_GLAIVE",SV_GLAIVE);
 tolua_constant(tolua_S,NULL,"SV_HALBERD",SV_HALBERD);
 tolua_constant(tolua_S,NULL,"SV_SCYTHE",SV_SCYTHE);
 tolua_constant(tolua_S,NULL,"SV_LANCE",SV_LANCE);
 tolua_constant(tolua_S,NULL,"SV_BATTLE_AXE",SV_BATTLE_AXE);
 tolua_constant(tolua_S,NULL,"SV_GREAT_AXE",SV_GREAT_AXE);
 tolua_constant(tolua_S,NULL,"SV_LOCHABER_AXE",SV_LOCHABER_AXE);
 tolua_constant(tolua_S,NULL,"SV_SCYTHE_OF_SLICING",SV_SCYTHE_OF_SLICING);
 tolua_constant(tolua_S,NULL,"SV_BROKEN_DAGGER",SV_BROKEN_DAGGER);
 tolua_constant(tolua_S,NULL,"SV_BROKEN_SWORD",SV_BROKEN_SWORD);
 tolua_constant(tolua_S,NULL,"SV_DAGGER",SV_DAGGER);
 tolua_constant(tolua_S,NULL,"SV_MAIN_GAUCHE",SV_MAIN_GAUCHE);
 tolua_constant(tolua_S,NULL,"SV_RAPIER",SV_RAPIER);
 tolua_constant(tolua_S,NULL,"SV_SMALL_SWORD",SV_SMALL_SWORD);
 tolua_constant(tolua_S,NULL,"SV_SHORT_SWORD",SV_SHORT_SWORD);
 tolua_constant(tolua_S,NULL,"SV_SABRE",SV_SABRE);
 tolua_constant(tolua_S,NULL,"SV_CUTLASS",SV_CUTLASS);
 tolua_constant(tolua_S,NULL,"SV_TULWAR",SV_TULWAR);
 tolua_constant(tolua_S,NULL,"SV_BROAD_SWORD",SV_BROAD_SWORD);
 tolua_constant(tolua_S,NULL,"SV_LONG_SWORD",SV_LONG_SWORD);
 tolua_constant(tolua_S,NULL,"SV_SCIMITAR",SV_SCIMITAR);
 tolua_constant(tolua_S,NULL,"SV_KATANA",SV_KATANA);
 tolua_constant(tolua_S,NULL,"SV_BASTARD_SWORD",SV_BASTARD_SWORD);
 tolua_constant(tolua_S,NULL,"SV_TWO_HANDED_SWORD",SV_TWO_HANDED_SWORD);
 tolua_constant(tolua_S,NULL,"SV_EXECUTIONERS_SWORD",SV_EXECUTIONERS_SWORD);
 tolua_constant(tolua_S,NULL,"SV_BLADE_OF_CHAOS",SV_BLADE_OF_CHAOS);
 tolua_constant(tolua_S,NULL,"SV_SMALL_LEATHER_SHIELD",SV_SMALL_LEATHER_SHIELD);
 tolua_constant(tolua_S,NULL,"SV_SMALL_METAL_SHIELD",SV_SMALL_METAL_SHIELD);
 tolua_constant(tolua_S,NULL,"SV_LARGE_LEATHER_SHIELD",SV_LARGE_LEATHER_SHIELD);
 tolua_constant(tolua_S,NULL,"SV_LARGE_METAL_SHIELD",SV_LARGE_METAL_SHIELD);
 tolua_constant(tolua_S,NULL,"SV_SHIELD_OF_DEFLECTION",SV_SHIELD_OF_DEFLECTION);
 tolua_constant(tolua_S,NULL,"SV_HARD_LEATHER_CAP",SV_HARD_LEATHER_CAP);
 tolua_constant(tolua_S,NULL,"SV_METAL_CAP",SV_METAL_CAP);
 tolua_constant(tolua_S,NULL,"SV_IRON_HELM",SV_IRON_HELM);
 tolua_constant(tolua_S,NULL,"SV_STEEL_HELM",SV_STEEL_HELM);
 tolua_constant(tolua_S,NULL,"SV_IRON_CROWN",SV_IRON_CROWN);
 tolua_constant(tolua_S,NULL,"SV_GOLDEN_CROWN",SV_GOLDEN_CROWN);
 tolua_constant(tolua_S,NULL,"SV_JEWELED_CROWN",SV_JEWELED_CROWN);
 tolua_constant(tolua_S,NULL,"SV_MORGOTH",SV_MORGOTH);
 tolua_constant(tolua_S,NULL,"SV_PAIR_OF_SOFT_LEATHER_BOOTS",SV_PAIR_OF_SOFT_LEATHER_BOOTS);
 tolua_constant(tolua_S,NULL,"SV_PAIR_OF_HARD_LEATHER_BOOTS",SV_PAIR_OF_HARD_LEATHER_BOOTS);
 tolua_constant(tolua_S,NULL,"SV_PAIR_OF_METAL_SHOD_BOOTS",SV_PAIR_OF_METAL_SHOD_BOOTS);
 tolua_constant(tolua_S,NULL,"SV_CLOAK",SV_CLOAK);
 tolua_constant(tolua_S,NULL,"SV_SHADOW_CLOAK",SV_SHADOW_CLOAK);
 tolua_constant(tolua_S,NULL,"SV_SET_OF_LEATHER_GLOVES",SV_SET_OF_LEATHER_GLOVES);
 tolua_constant(tolua_S,NULL,"SV_SET_OF_GAUNTLETS",SV_SET_OF_GAUNTLETS);
 tolua_constant(tolua_S,NULL,"SV_SET_OF_CESTI",SV_SET_OF_CESTI);
 tolua_constant(tolua_S,NULL,"SV_FILTHY_RAG",SV_FILTHY_RAG);
 tolua_constant(tolua_S,NULL,"SV_ROBE",SV_ROBE);
 tolua_constant(tolua_S,NULL,"SV_SOFT_LEATHER_ARMOR",SV_SOFT_LEATHER_ARMOR);
 tolua_constant(tolua_S,NULL,"SV_SOFT_STUDDED_LEATHER",SV_SOFT_STUDDED_LEATHER);
 tolua_constant(tolua_S,NULL,"SV_HARD_LEATHER_ARMOR",SV_HARD_LEATHER_ARMOR);
 tolua_constant(tolua_S,NULL,"SV_HARD_STUDDED_LEATHER",SV_HARD_STUDDED_LEATHER);
 tolua_constant(tolua_S,NULL,"SV_LEATHER_SCALE_MAIL",SV_LEATHER_SCALE_MAIL);
 tolua_constant(tolua_S,NULL,"SV_RUSTY_CHAIN_MAIL",SV_RUSTY_CHAIN_MAIL);
 tolua_constant(tolua_S,NULL,"SV_METAL_SCALE_MAIL",SV_METAL_SCALE_MAIL);
 tolua_constant(tolua_S,NULL,"SV_CHAIN_MAIL",SV_CHAIN_MAIL);
 tolua_constant(tolua_S,NULL,"SV_AUGMENTED_CHAIN_MAIL",SV_AUGMENTED_CHAIN_MAIL);
 tolua_constant(tolua_S,NULL,"SV_DOUBLE_CHAIN_MAIL",SV_DOUBLE_CHAIN_MAIL);
 tolua_constant(tolua_S,NULL,"SV_BAR_CHAIN_MAIL",SV_BAR_CHAIN_MAIL);
 tolua_constant(tolua_S,NULL,"SV_METAL_BRIGANDINE_ARMOUR",SV_METAL_BRIGANDINE_ARMOUR);
 tolua_constant(tolua_S,NULL,"SV_PARTIAL_PLATE_ARMOUR",SV_PARTIAL_PLATE_ARMOUR);
 tolua_constant(tolua_S,NULL,"SV_METAL_LAMELLAR_ARMOUR",SV_METAL_LAMELLAR_ARMOUR);
 tolua_constant(tolua_S,NULL,"SV_FULL_PLATE_ARMOUR",SV_FULL_PLATE_ARMOUR);
 tolua_constant(tolua_S,NULL,"SV_RIBBED_PLATE_ARMOUR",SV_RIBBED_PLATE_ARMOUR);
 tolua_constant(tolua_S,NULL,"SV_MITHRIL_CHAIN_MAIL",SV_MITHRIL_CHAIN_MAIL);
 tolua_constant(tolua_S,NULL,"SV_MITHRIL_PLATE_MAIL",SV_MITHRIL_PLATE_MAIL);
 tolua_constant(tolua_S,NULL,"SV_ADAMANTITE_PLATE_MAIL",SV_ADAMANTITE_PLATE_MAIL);
 tolua_constant(tolua_S,NULL,"SV_DRAGON_BLACK",SV_DRAGON_BLACK);
 tolua_constant(tolua_S,NULL,"SV_DRAGON_BLUE",SV_DRAGON_BLUE);
 tolua_constant(tolua_S,NULL,"SV_DRAGON_WHITE",SV_DRAGON_WHITE);
 tolua_constant(tolua_S,NULL,"SV_DRAGON_RED",SV_DRAGON_RED);
 tolua_constant(tolua_S,NULL,"SV_DRAGON_GREEN",SV_DRAGON_GREEN);
 tolua_constant(tolua_S,NULL,"SV_DRAGON_MULTIHUED",SV_DRAGON_MULTIHUED);
 tolua_constant(tolua_S,NULL,"SV_DRAGON_SHINING",SV_DRAGON_SHINING);
 tolua_constant(tolua_S,NULL,"SV_DRAGON_LAW",SV_DRAGON_LAW);
 tolua_constant(tolua_S,NULL,"SV_DRAGON_BRONZE",SV_DRAGON_BRONZE);
 tolua_constant(tolua_S,NULL,"SV_DRAGON_GOLD",SV_DRAGON_GOLD);
 tolua_constant(tolua_S,NULL,"SV_DRAGON_CHAOS",SV_DRAGON_CHAOS);
 tolua_constant(tolua_S,NULL,"SV_DRAGON_BALANCE",SV_DRAGON_BALANCE);
 tolua_constant(tolua_S,NULL,"SV_DRAGON_POWER",SV_DRAGON_POWER);
 tolua_constant(tolua_S,NULL,"SV_LITE_TORCH",SV_LITE_TORCH);
 tolua_constant(tolua_S,NULL,"SV_LITE_LANTERN",SV_LITE_LANTERN);
 tolua_constant(tolua_S,NULL,"SV_LITE_GALADRIEL",SV_LITE_GALADRIEL);
 tolua_constant(tolua_S,NULL,"SV_LITE_ELENDIL",SV_LITE_ELENDIL);
 tolua_constant(tolua_S,NULL,"SV_LITE_THRAIN",SV_LITE_THRAIN);
 tolua_constant(tolua_S,NULL,"SV_AMULET_DOOM",SV_AMULET_DOOM);
 tolua_constant(tolua_S,NULL,"SV_AMULET_TELEPORT",SV_AMULET_TELEPORT);
 tolua_constant(tolua_S,NULL,"SV_AMULET_ADORNMENT",SV_AMULET_ADORNMENT);
 tolua_constant(tolua_S,NULL,"SV_AMULET_SLOW_DIGEST",SV_AMULET_SLOW_DIGEST);
 tolua_constant(tolua_S,NULL,"SV_AMULET_RESIST_ACID",SV_AMULET_RESIST_ACID);
 tolua_constant(tolua_S,NULL,"SV_AMULET_SEARCHING",SV_AMULET_SEARCHING);
 tolua_constant(tolua_S,NULL,"SV_AMULET_WISDOM",SV_AMULET_WISDOM);
 tolua_constant(tolua_S,NULL,"SV_AMULET_CHARISMA",SV_AMULET_CHARISMA);
 tolua_constant(tolua_S,NULL,"SV_AMULET_THE_MAGI",SV_AMULET_THE_MAGI);
 tolua_constant(tolua_S,NULL,"SV_AMULET_SUSTENANCE",SV_AMULET_SUSTENANCE);
 tolua_constant(tolua_S,NULL,"SV_AMULET_CARLAMMAS",SV_AMULET_CARLAMMAS);
 tolua_constant(tolua_S,NULL,"SV_AMULET_INGWE",SV_AMULET_INGWE);
 tolua_constant(tolua_S,NULL,"SV_AMULET_DWARVES",SV_AMULET_DWARVES);
 tolua_constant(tolua_S,NULL,"SV_AMULET_ESP",SV_AMULET_ESP);
 tolua_constant(tolua_S,NULL,"SV_AMULET_RESIST",SV_AMULET_RESIST);
 tolua_constant(tolua_S,NULL,"SV_AMULET_REGEN",SV_AMULET_REGEN);
 tolua_constant(tolua_S,NULL,"SV_AMULET_ELESSAR",SV_AMULET_ELESSAR);
 tolua_constant(tolua_S,NULL,"SV_AMULET_EVENSTAR",SV_AMULET_EVENSTAR);
 tolua_constant(tolua_S,NULL,"SV_AMULET_DEVOTION",SV_AMULET_DEVOTION);
 tolua_constant(tolua_S,NULL,"SV_AMULET_WEAPONMASTERY",SV_AMULET_WEAPONMASTERY);
 tolua_constant(tolua_S,NULL,"SV_AMULET_TRICKERY",SV_AMULET_TRICKERY);
 tolua_constant(tolua_S,NULL,"SV_AMULET_INFRAVISION",SV_AMULET_INFRAVISION);
 tolua_constant(tolua_S,NULL,"SV_AMULET_RESIST_LIGHTNING",SV_AMULET_RESIST_LIGHTNING);
 tolua_constant(tolua_S,NULL,"SV_RING_WOE",SV_RING_WOE);
 tolua_constant(tolua_S,NULL,"SV_RING_AGGRAVATION",SV_RING_AGGRAVATION);
 tolua_constant(tolua_S,NULL,"SV_RING_WEAKNESS",SV_RING_WEAKNESS);
 tolua_constant(tolua_S,NULL,"SV_RING_STUPIDITY",SV_RING_STUPIDITY);
 tolua_constant(tolua_S,NULL,"SV_RING_TELEPORTATION",SV_RING_TELEPORTATION);
 tolua_constant(tolua_S,NULL,"SV_RING_SLOW_DIGESTION",SV_RING_SLOW_DIGESTION);
 tolua_constant(tolua_S,NULL,"SV_RING_FEATHER_FALL",SV_RING_FEATHER_FALL);
 tolua_constant(tolua_S,NULL,"SV_RING_RESIST_FIRE",SV_RING_RESIST_FIRE);
 tolua_constant(tolua_S,NULL,"SV_RING_RESIST_COLD",SV_RING_RESIST_COLD);
 tolua_constant(tolua_S,NULL,"SV_RING_SUSTAIN_STR",SV_RING_SUSTAIN_STR);
 tolua_constant(tolua_S,NULL,"SV_RING_SUSTAIN_INT",SV_RING_SUSTAIN_INT);
 tolua_constant(tolua_S,NULL,"SV_RING_SUSTAIN_WIS",SV_RING_SUSTAIN_WIS);
 tolua_constant(tolua_S,NULL,"SV_RING_SUSTAIN_DEX",SV_RING_SUSTAIN_DEX);
 tolua_constant(tolua_S,NULL,"SV_RING_SUSTAIN_CON",SV_RING_SUSTAIN_CON);
 tolua_constant(tolua_S,NULL,"SV_RING_SUSTAIN_CHR",SV_RING_SUSTAIN_CHR);
 tolua_constant(tolua_S,NULL,"SV_RING_PROTECTION",SV_RING_PROTECTION);
 tolua_constant(tolua_S,NULL,"SV_RING_ACID",SV_RING_ACID);
 tolua_constant(tolua_S,NULL,"SV_RING_FLAMES",SV_RING_FLAMES);
 tolua_constant(tolua_S,NULL,"SV_RING_ICE",SV_RING_ICE);
 tolua_constant(tolua_S,NULL,"SV_RING_RESIST_POIS",SV_RING_RESIST_POIS);
 tolua_constant(tolua_S,NULL,"SV_RING_FREE_ACTION",SV_RING_FREE_ACTION);
 tolua_constant(tolua_S,NULL,"SV_RING_SEE_INVIS",SV_RING_SEE_INVIS);
 tolua_constant(tolua_S,NULL,"SV_RING_SEARCHING",SV_RING_SEARCHING);
 tolua_constant(tolua_S,NULL,"SV_RING_STR",SV_RING_STR);
 tolua_constant(tolua_S,NULL,"SV_RING_INT",SV_RING_INT);
 tolua_constant(tolua_S,NULL,"SV_RING_DEX",SV_RING_DEX);
 tolua_constant(tolua_S,NULL,"SV_RING_CON",SV_RING_CON);
 tolua_constant(tolua_S,NULL,"SV_RING_ACCURACY",SV_RING_ACCURACY);
 tolua_constant(tolua_S,NULL,"SV_RING_DAMAGE",SV_RING_DAMAGE);
 tolua_constant(tolua_S,NULL,"SV_RING_SLAYING",SV_RING_SLAYING);
 tolua_constant(tolua_S,NULL,"SV_RING_SPEED",SV_RING_SPEED);
 tolua_constant(tolua_S,NULL,"SV_RING_BARAHIR",SV_RING_BARAHIR);
 tolua_constant(tolua_S,NULL,"SV_RING_TULKAS",SV_RING_TULKAS);
 tolua_constant(tolua_S,NULL,"SV_RING_NARYA",SV_RING_NARYA);
 tolua_constant(tolua_S,NULL,"SV_RING_NENYA",SV_RING_NENYA);
 tolua_constant(tolua_S,NULL,"SV_RING_VILYA",SV_RING_VILYA);
 tolua_constant(tolua_S,NULL,"SV_RING_POWER",SV_RING_POWER);
 tolua_constant(tolua_S,NULL,"SV_RING_LIGHTNING",SV_RING_LIGHTNING);
 tolua_constant(tolua_S,NULL,"SV_STAFF_DARKNESS",SV_STAFF_DARKNESS);
 tolua_constant(tolua_S,NULL,"SV_STAFF_SLOWNESS",SV_STAFF_SLOWNESS);
 tolua_constant(tolua_S,NULL,"SV_STAFF_HASTE_MONSTERS",SV_STAFF_HASTE_MONSTERS);
 tolua_constant(tolua_S,NULL,"SV_STAFF_SUMMONING",SV_STAFF_SUMMONING);
 tolua_constant(tolua_S,NULL,"SV_STAFF_TELEPORTATION",SV_STAFF_TELEPORTATION);
 tolua_constant(tolua_S,NULL,"SV_STAFF_IDENTIFY",SV_STAFF_IDENTIFY);
 tolua_constant(tolua_S,NULL,"SV_STAFF_REMOVE_CURSE",SV_STAFF_REMOVE_CURSE);
 tolua_constant(tolua_S,NULL,"SV_STAFF_STARLITE",SV_STAFF_STARLITE);
 tolua_constant(tolua_S,NULL,"SV_STAFF_LITE",SV_STAFF_LITE);
 tolua_constant(tolua_S,NULL,"SV_STAFF_MAPPING",SV_STAFF_MAPPING);
 tolua_constant(tolua_S,NULL,"SV_STAFF_DETECT_GOLD",SV_STAFF_DETECT_GOLD);
 tolua_constant(tolua_S,NULL,"SV_STAFF_DETECT_ITEM",SV_STAFF_DETECT_ITEM);
 tolua_constant(tolua_S,NULL,"SV_STAFF_DETECT_TRAP",SV_STAFF_DETECT_TRAP);
 tolua_constant(tolua_S,NULL,"SV_STAFF_DETECT_DOOR",SV_STAFF_DETECT_DOOR);
 tolua_constant(tolua_S,NULL,"SV_STAFF_DETECT_INVIS",SV_STAFF_DETECT_INVIS);
 tolua_constant(tolua_S,NULL,"SV_STAFF_DETECT_EVIL",SV_STAFF_DETECT_EVIL);
 tolua_constant(tolua_S,NULL,"SV_STAFF_CURE_LIGHT",SV_STAFF_CURE_LIGHT);
 tolua_constant(tolua_S,NULL,"SV_STAFF_CURING",SV_STAFF_CURING);
 tolua_constant(tolua_S,NULL,"SV_STAFF_HEALING",SV_STAFF_HEALING);
 tolua_constant(tolua_S,NULL,"SV_STAFF_THE_MAGI",SV_STAFF_THE_MAGI);
 tolua_constant(tolua_S,NULL,"SV_STAFF_SLEEP_MONSTERS",SV_STAFF_SLEEP_MONSTERS);
 tolua_constant(tolua_S,NULL,"SV_STAFF_SLOW_MONSTERS",SV_STAFF_SLOW_MONSTERS);
 tolua_constant(tolua_S,NULL,"SV_STAFF_SPEED",SV_STAFF_SPEED);
 tolua_constant(tolua_S,NULL,"SV_STAFF_PROBING",SV_STAFF_PROBING);
 tolua_constant(tolua_S,NULL,"SV_STAFF_DISPEL_EVIL",SV_STAFF_DISPEL_EVIL);
 tolua_constant(tolua_S,NULL,"SV_STAFF_POWER",SV_STAFF_POWER);
 tolua_constant(tolua_S,NULL,"SV_STAFF_HOLINESS",SV_STAFF_HOLINESS);
 tolua_constant(tolua_S,NULL,"SV_STAFF_GENOCIDE",SV_STAFF_GENOCIDE);
 tolua_constant(tolua_S,NULL,"SV_STAFF_EARTHQUAKES",SV_STAFF_EARTHQUAKES);
 tolua_constant(tolua_S,NULL,"SV_STAFF_DESTRUCTION",SV_STAFF_DESTRUCTION);
 tolua_constant(tolua_S,NULL,"SV_WAND_HEAL_MONSTER",SV_WAND_HEAL_MONSTER);
 tolua_constant(tolua_S,NULL,"SV_WAND_HASTE_MONSTER",SV_WAND_HASTE_MONSTER);
 tolua_constant(tolua_S,NULL,"SV_WAND_CLONE_MONSTER",SV_WAND_CLONE_MONSTER);
 tolua_constant(tolua_S,NULL,"SV_WAND_TELEPORT_AWAY",SV_WAND_TELEPORT_AWAY);
 tolua_constant(tolua_S,NULL,"SV_WAND_DISARMING",SV_WAND_DISARMING);
 tolua_constant(tolua_S,NULL,"SV_WAND_TRAP_DOOR_DEST",SV_WAND_TRAP_DOOR_DEST);
 tolua_constant(tolua_S,NULL,"SV_WAND_STONE_TO_MUD",SV_WAND_STONE_TO_MUD);
 tolua_constant(tolua_S,NULL,"SV_WAND_LITE",SV_WAND_LITE);
 tolua_constant(tolua_S,NULL,"SV_WAND_SLEEP_MONSTER",SV_WAND_SLEEP_MONSTER);
 tolua_constant(tolua_S,NULL,"SV_WAND_SLOW_MONSTER",SV_WAND_SLOW_MONSTER);
 tolua_constant(tolua_S,NULL,"SV_WAND_CONFUSE_MONSTER",SV_WAND_CONFUSE_MONSTER);
 tolua_constant(tolua_S,NULL,"SV_WAND_FEAR_MONSTER",SV_WAND_FEAR_MONSTER);
 tolua_constant(tolua_S,NULL,"SV_WAND_DRAIN_LIFE",SV_WAND_DRAIN_LIFE);
 tolua_constant(tolua_S,NULL,"SV_WAND_POLYMORPH",SV_WAND_POLYMORPH);
 tolua_constant(tolua_S,NULL,"SV_WAND_STINKING_CLOUD",SV_WAND_STINKING_CLOUD);
 tolua_constant(tolua_S,NULL,"SV_WAND_MAGIC_MISSILE",SV_WAND_MAGIC_MISSILE);
 tolua_constant(tolua_S,NULL,"SV_WAND_ACID_BOLT",SV_WAND_ACID_BOLT);
 tolua_constant(tolua_S,NULL,"SV_WAND_ELEC_BOLT",SV_WAND_ELEC_BOLT);
 tolua_constant(tolua_S,NULL,"SV_WAND_FIRE_BOLT",SV_WAND_FIRE_BOLT);
 tolua_constant(tolua_S,NULL,"SV_WAND_COLD_BOLT",SV_WAND_COLD_BOLT);
 tolua_constant(tolua_S,NULL,"SV_WAND_ACID_BALL",SV_WAND_ACID_BALL);
 tolua_constant(tolua_S,NULL,"SV_WAND_ELEC_BALL",SV_WAND_ELEC_BALL);
 tolua_constant(tolua_S,NULL,"SV_WAND_FIRE_BALL",SV_WAND_FIRE_BALL);
 tolua_constant(tolua_S,NULL,"SV_WAND_COLD_BALL",SV_WAND_COLD_BALL);
 tolua_constant(tolua_S,NULL,"SV_WAND_WONDER",SV_WAND_WONDER);
 tolua_constant(tolua_S,NULL,"SV_WAND_ANNIHILATION",SV_WAND_ANNIHILATION);
 tolua_constant(tolua_S,NULL,"SV_WAND_DRAGON_FIRE",SV_WAND_DRAGON_FIRE);
 tolua_constant(tolua_S,NULL,"SV_WAND_DRAGON_COLD",SV_WAND_DRAGON_COLD);
 tolua_constant(tolua_S,NULL,"SV_WAND_DRAGON_BREATH",SV_WAND_DRAGON_BREATH);
 tolua_constant(tolua_S,NULL,"SV_ROD_DETECT_TRAP",SV_ROD_DETECT_TRAP);
 tolua_constant(tolua_S,NULL,"SV_ROD_DETECT_DOOR",SV_ROD_DETECT_DOOR);
 tolua_constant(tolua_S,NULL,"SV_ROD_IDENTIFY",SV_ROD_IDENTIFY);
 tolua_constant(tolua_S,NULL,"SV_ROD_RECALL",SV_ROD_RECALL);
 tolua_constant(tolua_S,NULL,"SV_ROD_ILLUMINATION",SV_ROD_ILLUMINATION);
 tolua_constant(tolua_S,NULL,"SV_ROD_MAPPING",SV_ROD_MAPPING);
 tolua_constant(tolua_S,NULL,"SV_ROD_DETECTION",SV_ROD_DETECTION);
 tolua_constant(tolua_S,NULL,"SV_ROD_PROBING",SV_ROD_PROBING);
 tolua_constant(tolua_S,NULL,"SV_ROD_CURING",SV_ROD_CURING);
 tolua_constant(tolua_S,NULL,"SV_ROD_HEALING",SV_ROD_HEALING);
 tolua_constant(tolua_S,NULL,"SV_ROD_RESTORATION",SV_ROD_RESTORATION);
 tolua_constant(tolua_S,NULL,"SV_ROD_SPEED",SV_ROD_SPEED);
 tolua_constant(tolua_S,NULL,"SV_ROD_TELEPORT_AWAY",SV_ROD_TELEPORT_AWAY);
 tolua_constant(tolua_S,NULL,"SV_ROD_DISARMING",SV_ROD_DISARMING);
 tolua_constant(tolua_S,NULL,"SV_ROD_LITE",SV_ROD_LITE);
 tolua_constant(tolua_S,NULL,"SV_ROD_SLEEP_MONSTER",SV_ROD_SLEEP_MONSTER);
 tolua_constant(tolua_S,NULL,"SV_ROD_SLOW_MONSTER",SV_ROD_SLOW_MONSTER);
 tolua_constant(tolua_S,NULL,"SV_ROD_DRAIN_LIFE",SV_ROD_DRAIN_LIFE);
 tolua_constant(tolua_S,NULL,"SV_ROD_POLYMORPH",SV_ROD_POLYMORPH);
 tolua_constant(tolua_S,NULL,"SV_ROD_ACID_BOLT",SV_ROD_ACID_BOLT);
 tolua_constant(tolua_S,NULL,"SV_ROD_ELEC_BOLT",SV_ROD_ELEC_BOLT);
 tolua_constant(tolua_S,NULL,"SV_ROD_FIRE_BOLT",SV_ROD_FIRE_BOLT);
 tolua_constant(tolua_S,NULL,"SV_ROD_COLD_BOLT",SV_ROD_COLD_BOLT);
 tolua_constant(tolua_S,NULL,"SV_ROD_ACID_BALL",SV_ROD_ACID_BALL);
 tolua_constant(tolua_S,NULL,"SV_ROD_ELEC_BALL",SV_ROD_ELEC_BALL);
 tolua_constant(tolua_S,NULL,"SV_ROD_FIRE_BALL",SV_ROD_FIRE_BALL);
 tolua_constant(tolua_S,NULL,"SV_ROD_COLD_BALL",SV_ROD_COLD_BALL);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_DARKNESS",SV_SCROLL_DARKNESS);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_AGGRAVATE_MONSTER",SV_SCROLL_AGGRAVATE_MONSTER);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_CURSE_ARMOR",SV_SCROLL_CURSE_ARMOR);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_CURSE_WEAPON",SV_SCROLL_CURSE_WEAPON);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_SUMMON_MONSTER",SV_SCROLL_SUMMON_MONSTER);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_SUMMON_UNDEAD",SV_SCROLL_SUMMON_UNDEAD);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_TRAP_CREATION",SV_SCROLL_TRAP_CREATION);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_PHASE_DOOR",SV_SCROLL_PHASE_DOOR);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_TELEPORT",SV_SCROLL_TELEPORT);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_TELEPORT_LEVEL",SV_SCROLL_TELEPORT_LEVEL);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_WORD_OF_RECALL",SV_SCROLL_WORD_OF_RECALL);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_IDENTIFY",SV_SCROLL_IDENTIFY);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_STAR_IDENTIFY",SV_SCROLL_STAR_IDENTIFY);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_REMOVE_CURSE",SV_SCROLL_REMOVE_CURSE);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_STAR_REMOVE_CURSE",SV_SCROLL_STAR_REMOVE_CURSE);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_ENCHANT_ARMOR",SV_SCROLL_ENCHANT_ARMOR);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_ENCHANT_WEAPON_TO_HIT",SV_SCROLL_ENCHANT_WEAPON_TO_HIT);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_ENCHANT_WEAPON_TO_DAM",SV_SCROLL_ENCHANT_WEAPON_TO_DAM);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_STAR_ENCHANT_ARMOR",SV_SCROLL_STAR_ENCHANT_ARMOR);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_STAR_ENCHANT_WEAPON",SV_SCROLL_STAR_ENCHANT_WEAPON);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_RECHARGING",SV_SCROLL_RECHARGING);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_LIGHT",SV_SCROLL_LIGHT);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_MAPPING",SV_SCROLL_MAPPING);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_DETECT_GOLD",SV_SCROLL_DETECT_GOLD);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_DETECT_ITEM",SV_SCROLL_DETECT_ITEM);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_DETECT_TRAP",SV_SCROLL_DETECT_TRAP);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_DETECT_DOOR",SV_SCROLL_DETECT_DOOR);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_DETECT_INVIS",SV_SCROLL_DETECT_INVIS);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_SATISFY_HUNGER",SV_SCROLL_SATISFY_HUNGER);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_BLESSING",SV_SCROLL_BLESSING);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_HOLY_CHANT",SV_SCROLL_HOLY_CHANT);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_HOLY_PRAYER",SV_SCROLL_HOLY_PRAYER);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_MONSTER_CONFUSION",SV_SCROLL_MONSTER_CONFUSION);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_PROTECTION_FROM_EVIL",SV_SCROLL_PROTECTION_FROM_EVIL);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_RUNE_OF_PROTECTION",SV_SCROLL_RUNE_OF_PROTECTION);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_TRAP_DOOR_DESTRUCTION",SV_SCROLL_TRAP_DOOR_DESTRUCTION);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_STAR_DESTRUCTION",SV_SCROLL_STAR_DESTRUCTION);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_DISPEL_UNDEAD",SV_SCROLL_DISPEL_UNDEAD);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_GENOCIDE",SV_SCROLL_GENOCIDE);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_MASS_GENOCIDE",SV_SCROLL_MASS_GENOCIDE);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_ACQUIREMENT",SV_SCROLL_ACQUIREMENT);
 tolua_constant(tolua_S,NULL,"SV_SCROLL_STAR_ACQUIREMENT",SV_SCROLL_STAR_ACQUIREMENT);
 tolua_constant(tolua_S,NULL,"SV_POTION_WATER",SV_POTION_WATER);
 tolua_constant(tolua_S,NULL,"SV_POTION_APPLE_JUICE",SV_POTION_APPLE_JUICE);
 tolua_constant(tolua_S,NULL,"SV_POTION_SLIME_MOLD",SV_POTION_SLIME_MOLD);
 tolua_constant(tolua_S,NULL,"SV_POTION_SLOWNESS",SV_POTION_SLOWNESS);
 tolua_constant(tolua_S,NULL,"SV_POTION_SALT_WATER",SV_POTION_SALT_WATER);
 tolua_constant(tolua_S,NULL,"SV_POTION_POISON",SV_POTION_POISON);
 tolua_constant(tolua_S,NULL,"SV_POTION_BLINDNESS",SV_POTION_BLINDNESS);
 tolua_constant(tolua_S,NULL,"SV_POTION_CONFUSION",SV_POTION_CONFUSION);
 tolua_constant(tolua_S,NULL,"SV_POTION_SLEEP",SV_POTION_SLEEP);
 tolua_constant(tolua_S,NULL,"SV_POTION_LOSE_MEMORIES",SV_POTION_LOSE_MEMORIES);
 tolua_constant(tolua_S,NULL,"SV_POTION_RUINATION",SV_POTION_RUINATION);
 tolua_constant(tolua_S,NULL,"SV_POTION_DEC_STR",SV_POTION_DEC_STR);
 tolua_constant(tolua_S,NULL,"SV_POTION_DEC_INT",SV_POTION_DEC_INT);
 tolua_constant(tolua_S,NULL,"SV_POTION_DEC_WIS",SV_POTION_DEC_WIS);
 tolua_constant(tolua_S,NULL,"SV_POTION_DEC_DEX",SV_POTION_DEC_DEX);
 tolua_constant(tolua_S,NULL,"SV_POTION_DEC_CON",SV_POTION_DEC_CON);
 tolua_constant(tolua_S,NULL,"SV_POTION_DEC_CHR",SV_POTION_DEC_CHR);
 tolua_constant(tolua_S,NULL,"SV_POTION_DETONATIONS",SV_POTION_DETONATIONS);
 tolua_constant(tolua_S,NULL,"SV_POTION_DEATH",SV_POTION_DEATH);
 tolua_constant(tolua_S,NULL,"SV_POTION_INFRAVISION",SV_POTION_INFRAVISION);
 tolua_constant(tolua_S,NULL,"SV_POTION_DETECT_INVIS",SV_POTION_DETECT_INVIS);
 tolua_constant(tolua_S,NULL,"SV_POTION_SLOW_POISON",SV_POTION_SLOW_POISON);
 tolua_constant(tolua_S,NULL,"SV_POTION_CURE_POISON",SV_POTION_CURE_POISON);
 tolua_constant(tolua_S,NULL,"SV_POTION_BOLDNESS",SV_POTION_BOLDNESS);
 tolua_constant(tolua_S,NULL,"SV_POTION_SPEED",SV_POTION_SPEED);
 tolua_constant(tolua_S,NULL,"SV_POTION_RESIST_HEAT",SV_POTION_RESIST_HEAT);
 tolua_constant(tolua_S,NULL,"SV_POTION_RESIST_COLD",SV_POTION_RESIST_COLD);
 tolua_constant(tolua_S,NULL,"SV_POTION_HEROISM",SV_POTION_HEROISM);
 tolua_constant(tolua_S,NULL,"SV_POTION_BERSERK_STRENGTH",SV_POTION_BERSERK_STRENGTH);
 tolua_constant(tolua_S,NULL,"SV_POTION_CURE_LIGHT",SV_POTION_CURE_LIGHT);
 tolua_constant(tolua_S,NULL,"SV_POTION_CURE_SERIOUS",SV_POTION_CURE_SERIOUS);
 tolua_constant(tolua_S,NULL,"SV_POTION_CURE_CRITICAL",SV_POTION_CURE_CRITICAL);
 tolua_constant(tolua_S,NULL,"SV_POTION_HEALING",SV_POTION_HEALING);
 tolua_constant(tolua_S,NULL,"SV_POTION_STAR_HEALING",SV_POTION_STAR_HEALING);
 tolua_constant(tolua_S,NULL,"SV_POTION_LIFE",SV_POTION_LIFE);
 tolua_constant(tolua_S,NULL,"SV_POTION_RESTORE_MANA",SV_POTION_RESTORE_MANA);
 tolua_constant(tolua_S,NULL,"SV_POTION_RESTORE_EXP",SV_POTION_RESTORE_EXP);
 tolua_constant(tolua_S,NULL,"SV_POTION_RES_STR",SV_POTION_RES_STR);
 tolua_constant(tolua_S,NULL,"SV_POTION_RES_INT",SV_POTION_RES_INT);
 tolua_constant(tolua_S,NULL,"SV_POTION_RES_WIS",SV_POTION_RES_WIS);
 tolua_constant(tolua_S,NULL,"SV_POTION_RES_DEX",SV_POTION_RES_DEX);
 tolua_constant(tolua_S,NULL,"SV_POTION_RES_CON",SV_POTION_RES_CON);
 tolua_constant(tolua_S,NULL,"SV_POTION_RES_CHR",SV_POTION_RES_CHR);
 tolua_constant(tolua_S,NULL,"SV_POTION_INC_STR",SV_POTION_INC_STR);
 tolua_constant(tolua_S,NULL,"SV_POTION_INC_INT",SV_POTION_INC_INT);
 tolua_constant(tolua_S,NULL,"SV_POTION_INC_WIS",SV_POTION_INC_WIS);
 tolua_constant(tolua_S,NULL,"SV_POTION_INC_DEX",SV_POTION_INC_DEX);
 tolua_constant(tolua_S,NULL,"SV_POTION_INC_CON",SV_POTION_INC_CON);
 tolua_constant(tolua_S,NULL,"SV_POTION_INC_CHR",SV_POTION_INC_CHR);
 tolua_constant(tolua_S,NULL,"SV_POTION_AUGMENTATION",SV_POTION_AUGMENTATION);
 tolua_constant(tolua_S,NULL,"SV_POTION_ENLIGHTENMENT",SV_POTION_ENLIGHTENMENT);
 tolua_constant(tolua_S,NULL,"SV_POTION_STAR_ENLIGHTENMENT",SV_POTION_STAR_ENLIGHTENMENT);
 tolua_constant(tolua_S,NULL,"SV_POTION_SELF_KNOWLEDGE",SV_POTION_SELF_KNOWLEDGE);
 tolua_constant(tolua_S,NULL,"SV_POTION_EXPERIENCE",SV_POTION_EXPERIENCE);
 tolua_constant(tolua_S,NULL,"SV_FOOD_POISON",SV_FOOD_POISON);
 tolua_constant(tolua_S,NULL,"SV_FOOD_BLINDNESS",SV_FOOD_BLINDNESS);
 tolua_constant(tolua_S,NULL,"SV_FOOD_PARANOIA",SV_FOOD_PARANOIA);
 tolua_constant(tolua_S,NULL,"SV_FOOD_CONFUSION",SV_FOOD_CONFUSION);
 tolua_constant(tolua_S,NULL,"SV_FOOD_HALLUCINATION",SV_FOOD_HALLUCINATION);
 tolua_constant(tolua_S,NULL,"SV_FOOD_PARALYSIS",SV_FOOD_PARALYSIS);
 tolua_constant(tolua_S,NULL,"SV_FOOD_WEAKNESS",SV_FOOD_WEAKNESS);
 tolua_constant(tolua_S,NULL,"SV_FOOD_SICKNESS",SV_FOOD_SICKNESS);
 tolua_constant(tolua_S,NULL,"SV_FOOD_STUPIDITY",SV_FOOD_STUPIDITY);
 tolua_constant(tolua_S,NULL,"SV_FOOD_NAIVETY",SV_FOOD_NAIVETY);
 tolua_constant(tolua_S,NULL,"SV_FOOD_UNHEALTH",SV_FOOD_UNHEALTH);
 tolua_constant(tolua_S,NULL,"SV_FOOD_DISEASE",SV_FOOD_DISEASE);
 tolua_constant(tolua_S,NULL,"SV_FOOD_CURE_POISON",SV_FOOD_CURE_POISON);
 tolua_constant(tolua_S,NULL,"SV_FOOD_CURE_BLINDNESS",SV_FOOD_CURE_BLINDNESS);
 tolua_constant(tolua_S,NULL,"SV_FOOD_CURE_PARANOIA",SV_FOOD_CURE_PARANOIA);
 tolua_constant(tolua_S,NULL,"SV_FOOD_CURE_CONFUSION",SV_FOOD_CURE_CONFUSION);
 tolua_constant(tolua_S,NULL,"SV_FOOD_CURE_SERIOUS",SV_FOOD_CURE_SERIOUS);
 tolua_constant(tolua_S,NULL,"SV_FOOD_RESTORE_STR",SV_FOOD_RESTORE_STR);
 tolua_constant(tolua_S,NULL,"SV_FOOD_RESTORE_CON",SV_FOOD_RESTORE_CON);
 tolua_constant(tolua_S,NULL,"SV_FOOD_RESTORING",SV_FOOD_RESTORING);
 tolua_constant(tolua_S,NULL,"SV_FOOD_BISCUIT",SV_FOOD_BISCUIT);
 tolua_constant(tolua_S,NULL,"SV_FOOD_JERKY",SV_FOOD_JERKY);
 tolua_constant(tolua_S,NULL,"SV_FOOD_RATION",SV_FOOD_RATION);
 tolua_constant(tolua_S,NULL,"SV_FOOD_SLIME_MOLD",SV_FOOD_SLIME_MOLD);
 tolua_constant(tolua_S,NULL,"SV_FOOD_WAYBREAD",SV_FOOD_WAYBREAD);
 tolua_constant(tolua_S,NULL,"SV_FOOD_PINT_OF_ALE",SV_FOOD_PINT_OF_ALE);
 tolua_constant(tolua_S,NULL,"SV_FOOD_PINT_OF_WINE",SV_FOOD_PINT_OF_WINE);
 tolua_constant(tolua_S,NULL,"SV_FOOD_MIN_FOOD",SV_FOOD_MIN_FOOD);
 tolua_constant(tolua_S,NULL,"SV_ROD_MIN_DIRECTION",SV_ROD_MIN_DIRECTION);
 tolua_constant(tolua_S,NULL,"SV_CHEST_MIN_LARGE",SV_CHEST_MIN_LARGE);
 tolua_constant(tolua_S,NULL,"SV_BOOK_MIN_GOOD",SV_BOOK_MIN_GOOD);
 tolua_constant(tolua_S,NULL,"CHEST_LOSE_STR",CHEST_LOSE_STR);
 tolua_constant(tolua_S,NULL,"CHEST_LOSE_CON",CHEST_LOSE_CON);
 tolua_constant(tolua_S,NULL,"CHEST_POISON",CHEST_POISON);
 tolua_constant(tolua_S,NULL,"CHEST_PARALYZE",CHEST_PARALYZE);
 tolua_constant(tolua_S,NULL,"CHEST_EXPLODE",CHEST_EXPLODE);
 tolua_constant(tolua_S,NULL,"CHEST_SUMMON",CHEST_SUMMON);
 tolua_constant(tolua_S,NULL,"IDENT_SENSE",IDENT_SENSE);
 tolua_constant(tolua_S,NULL,"IDENT_FIXED",IDENT_FIXED);
 tolua_constant(tolua_S,NULL,"IDENT_EMPTY",IDENT_EMPTY);
 tolua_constant(tolua_S,NULL,"IDENT_KNOWN",IDENT_KNOWN);
 tolua_constant(tolua_S,NULL,"IDENT_RUMOUR",IDENT_RUMOUR);
 tolua_constant(tolua_S,NULL,"IDENT_MENTAL",IDENT_MENTAL);
 tolua_constant(tolua_S,NULL,"IDENT_CURSED",IDENT_CURSED);
 tolua_constant(tolua_S,NULL,"IDENT_BROKEN",IDENT_BROKEN);
 tolua_constant(tolua_S,NULL,"INSCRIP_NULL",INSCRIP_NULL);
 tolua_constant(tolua_S,NULL,"INSCRIP_TERRIBLE",INSCRIP_TERRIBLE);
 tolua_constant(tolua_S,NULL,"INSCRIP_WORTHLESS",INSCRIP_WORTHLESS);
 tolua_constant(tolua_S,NULL,"INSCRIP_CURSED",INSCRIP_CURSED);
 tolua_constant(tolua_S,NULL,"INSCRIP_BROKEN",INSCRIP_BROKEN);
 tolua_constant(tolua_S,NULL,"INSCRIP_AVERAGE",INSCRIP_AVERAGE);
 tolua_constant(tolua_S,NULL,"INSCRIP_GOOD",INSCRIP_GOOD);
 tolua_constant(tolua_S,NULL,"INSCRIP_EXCELLENT",INSCRIP_EXCELLENT);
 tolua_constant(tolua_S,NULL,"INSCRIP_SPECIAL",INSCRIP_SPECIAL);
 tolua_constant(tolua_S,NULL,"INSCRIP_UNCURSED",INSCRIP_UNCURSED);
 tolua_constant(tolua_S,NULL,"INSCRIP_INDESTRUCTIBLE",INSCRIP_INDESTRUCTIBLE);
 tolua_constant(tolua_S,NULL,"MAX_INSCRIP",MAX_INSCRIP);
 tolua_constant(tolua_S,NULL,"TR1_STR",TR1_STR);
 tolua_constant(tolua_S,NULL,"TR1_INT",TR1_INT);
 tolua_constant(tolua_S,NULL,"TR1_WIS",TR1_WIS);
 tolua_constant(tolua_S,NULL,"TR1_DEX",TR1_DEX);
 tolua_constant(tolua_S,NULL,"TR1_CON",TR1_CON);
 tolua_constant(tolua_S,NULL,"TR1_CHR",TR1_CHR);
 tolua_constant(tolua_S,NULL,"TR1_XXX1",TR1_XXX1);
 tolua_constant(tolua_S,NULL,"TR1_XXX2",TR1_XXX2);
 tolua_constant(tolua_S,NULL,"TR1_STEALTH",TR1_STEALTH);
 tolua_constant(tolua_S,NULL,"TR1_SEARCH",TR1_SEARCH);
 tolua_constant(tolua_S,NULL,"TR1_INFRA",TR1_INFRA);
 tolua_constant(tolua_S,NULL,"TR1_TUNNEL",TR1_TUNNEL);
 tolua_constant(tolua_S,NULL,"TR1_SPEED",TR1_SPEED);
 tolua_constant(tolua_S,NULL,"TR1_BLOWS",TR1_BLOWS);
 tolua_constant(tolua_S,NULL,"TR1_SHOTS",TR1_SHOTS);
 tolua_constant(tolua_S,NULL,"TR1_MIGHT",TR1_MIGHT);
 tolua_constant(tolua_S,NULL,"TR1_SLAY_ANIMAL",TR1_SLAY_ANIMAL);
 tolua_constant(tolua_S,NULL,"TR1_SLAY_EVIL",TR1_SLAY_EVIL);
 tolua_constant(tolua_S,NULL,"TR1_SLAY_UNDEAD",TR1_SLAY_UNDEAD);
 tolua_constant(tolua_S,NULL,"TR1_SLAY_DEMON",TR1_SLAY_DEMON);
 tolua_constant(tolua_S,NULL,"TR1_SLAY_ORC",TR1_SLAY_ORC);
 tolua_constant(tolua_S,NULL,"TR1_SLAY_TROLL",TR1_SLAY_TROLL);
 tolua_constant(tolua_S,NULL,"TR1_SLAY_GIANT",TR1_SLAY_GIANT);
 tolua_constant(tolua_S,NULL,"TR1_SLAY_DRAGON",TR1_SLAY_DRAGON);
 tolua_constant(tolua_S,NULL,"TR1_KILL_DRAGON",TR1_KILL_DRAGON);
 tolua_constant(tolua_S,NULL,"TR1_KILL_DEMON",TR1_KILL_DEMON);
 tolua_constant(tolua_S,NULL,"TR1_KILL_UNDEAD",TR1_KILL_UNDEAD);
 tolua_constant(tolua_S,NULL,"TR1_BRAND_POIS",TR1_BRAND_POIS);
 tolua_constant(tolua_S,NULL,"TR1_BRAND_ACID",TR1_BRAND_ACID);
 tolua_constant(tolua_S,NULL,"TR1_BRAND_ELEC",TR1_BRAND_ELEC);
 tolua_constant(tolua_S,NULL,"TR1_BRAND_FIRE",TR1_BRAND_FIRE);
 tolua_constant(tolua_S,NULL,"TR1_BRAND_COLD",TR1_BRAND_COLD);
 tolua_constant(tolua_S,NULL,"TR2_SUST_STR",TR2_SUST_STR);
 tolua_constant(tolua_S,NULL,"TR2_SUST_INT",TR2_SUST_INT);
 tolua_constant(tolua_S,NULL,"TR2_SUST_WIS",TR2_SUST_WIS);
 tolua_constant(tolua_S,NULL,"TR2_SUST_DEX",TR2_SUST_DEX);
 tolua_constant(tolua_S,NULL,"TR2_SUST_CON",TR2_SUST_CON);
 tolua_constant(tolua_S,NULL,"TR2_SUST_CHR",TR2_SUST_CHR);
 tolua_constant(tolua_S,NULL,"TR2_XXX1",TR2_XXX1);
 tolua_constant(tolua_S,NULL,"TR2_XXX2",TR2_XXX2);
 tolua_constant(tolua_S,NULL,"TR2_XXX3",TR2_XXX3);
 tolua_constant(tolua_S,NULL,"TR2_XXX4",TR2_XXX4);
 tolua_constant(tolua_S,NULL,"TR2_XXX5",TR2_XXX5);
 tolua_constant(tolua_S,NULL,"TR2_XXX6",TR2_XXX6);
 tolua_constant(tolua_S,NULL,"TR2_IM_ACID",TR2_IM_ACID);
 tolua_constant(tolua_S,NULL,"TR2_IM_ELEC",TR2_IM_ELEC);
 tolua_constant(tolua_S,NULL,"TR2_IM_FIRE",TR2_IM_FIRE);
 tolua_constant(tolua_S,NULL,"TR2_IM_COLD",TR2_IM_COLD);
 tolua_constant(tolua_S,NULL,"TR2_RES_ACID",TR2_RES_ACID);
 tolua_constant(tolua_S,NULL,"TR2_RES_ELEC",TR2_RES_ELEC);
 tolua_constant(tolua_S,NULL,"TR2_RES_FIRE",TR2_RES_FIRE);
 tolua_constant(tolua_S,NULL,"TR2_RES_COLD",TR2_RES_COLD);
 tolua_constant(tolua_S,NULL,"TR2_RES_POIS",TR2_RES_POIS);
 tolua_constant(tolua_S,NULL,"TR2_RES_FEAR",TR2_RES_FEAR);
 tolua_constant(tolua_S,NULL,"TR2_RES_LITE",TR2_RES_LITE);
 tolua_constant(tolua_S,NULL,"TR2_RES_DARK",TR2_RES_DARK);
 tolua_constant(tolua_S,NULL,"TR2_RES_BLIND",TR2_RES_BLIND);
 tolua_constant(tolua_S,NULL,"TR2_RES_CONFU",TR2_RES_CONFU);
 tolua_constant(tolua_S,NULL,"TR2_RES_SOUND",TR2_RES_SOUND);
 tolua_constant(tolua_S,NULL,"TR2_RES_SHARD",TR2_RES_SHARD);
 tolua_constant(tolua_S,NULL,"TR2_RES_NEXUS",TR2_RES_NEXUS);
 tolua_constant(tolua_S,NULL,"TR2_RES_NETHR",TR2_RES_NETHR);
 tolua_constant(tolua_S,NULL,"TR2_RES_CHAOS",TR2_RES_CHAOS);
 tolua_constant(tolua_S,NULL,"TR2_RES_DISEN",TR2_RES_DISEN);
 tolua_constant(tolua_S,NULL,"TR3_SLOW_DIGEST",TR3_SLOW_DIGEST);
 tolua_constant(tolua_S,NULL,"TR3_FEATHER",TR3_FEATHER);
 tolua_constant(tolua_S,NULL,"TR3_LITE",TR3_LITE);
 tolua_constant(tolua_S,NULL,"TR3_REGEN",TR3_REGEN);
 tolua_constant(tolua_S,NULL,"TR3_TELEPATHY",TR3_TELEPATHY);
 tolua_constant(tolua_S,NULL,"TR3_SEE_INVIS",TR3_SEE_INVIS);
 tolua_constant(tolua_S,NULL,"TR3_FREE_ACT",TR3_FREE_ACT);
 tolua_constant(tolua_S,NULL,"TR3_HOLD_LIFE",TR3_HOLD_LIFE);
 tolua_constant(tolua_S,NULL,"TR3_XXX1",TR3_XXX1);
 tolua_constant(tolua_S,NULL,"TR3_XXX2",TR3_XXX2);
 tolua_constant(tolua_S,NULL,"TR3_XXX3",TR3_XXX3);
 tolua_constant(tolua_S,NULL,"TR3_XXX4",TR3_XXX4);
 tolua_constant(tolua_S,NULL,"TR3_IMPACT",TR3_IMPACT);
 tolua_constant(tolua_S,NULL,"TR3_TELEPORT",TR3_TELEPORT);
 tolua_constant(tolua_S,NULL,"TR3_AGGRAVATE",TR3_AGGRAVATE);
 tolua_constant(tolua_S,NULL,"TR3_DRAIN_EXP",TR3_DRAIN_EXP);
 tolua_constant(tolua_S,NULL,"TR3_IGNORE_ACID",TR3_IGNORE_ACID);
 tolua_constant(tolua_S,NULL,"TR3_IGNORE_ELEC",TR3_IGNORE_ELEC);
 tolua_constant(tolua_S,NULL,"TR3_IGNORE_FIRE",TR3_IGNORE_FIRE);
 tolua_constant(tolua_S,NULL,"TR3_IGNORE_COLD",TR3_IGNORE_COLD);
 tolua_constant(tolua_S,NULL,"TR3_XXX5",TR3_XXX5);
 tolua_constant(tolua_S,NULL,"TR3_XXX6",TR3_XXX6);
 tolua_constant(tolua_S,NULL,"TR3_BLESSED",TR3_BLESSED);
 tolua_constant(tolua_S,NULL,"TR3_ACTIVATE",TR3_ACTIVATE);
 tolua_constant(tolua_S,NULL,"TR3_INSTA_ART",TR3_INSTA_ART);
 tolua_constant(tolua_S,NULL,"TR3_EASY_KNOW",TR3_EASY_KNOW);
 tolua_constant(tolua_S,NULL,"TR3_HIDE_TYPE",TR3_HIDE_TYPE);
 tolua_constant(tolua_S,NULL,"TR3_SHOW_MODS",TR3_SHOW_MODS);
 tolua_constant(tolua_S,NULL,"TR3_XXX7",TR3_XXX7);
 tolua_constant(tolua_S,NULL,"TR3_LIGHT_CURSE",TR3_LIGHT_CURSE);
 tolua_constant(tolua_S,NULL,"TR3_HEAVY_CURSE",TR3_HEAVY_CURSE);
 tolua_constant(tolua_S,NULL,"TR3_PERMA_CURSE",TR3_PERMA_CURSE);
 tolua_constant(tolua_S,NULL,"OBJECT_XTRA_TYPE_SUSTAIN",OBJECT_XTRA_TYPE_SUSTAIN);
 tolua_constant(tolua_S,NULL,"OBJECT_XTRA_TYPE_RESIST",OBJECT_XTRA_TYPE_RESIST);
 tolua_constant(tolua_S,NULL,"OBJECT_XTRA_TYPE_POWER",OBJECT_XTRA_TYPE_POWER);
 tolua_constant(tolua_S,NULL,"OBJECT_XTRA_WHAT_SUSTAIN",OBJECT_XTRA_WHAT_SUSTAIN);
 tolua_constant(tolua_S,NULL,"OBJECT_XTRA_WHAT_RESIST",OBJECT_XTRA_WHAT_RESIST);
 tolua_constant(tolua_S,NULL,"OBJECT_XTRA_WHAT_POWER",OBJECT_XTRA_WHAT_POWER);
 tolua_constant(tolua_S,NULL,"OBJECT_XTRA_BASE_SUSTAIN",OBJECT_XTRA_BASE_SUSTAIN);
 tolua_constant(tolua_S,NULL,"OBJECT_XTRA_BASE_RESIST",OBJECT_XTRA_BASE_RESIST);
 tolua_constant(tolua_S,NULL,"OBJECT_XTRA_BASE_POWER",OBJECT_XTRA_BASE_POWER);
 tolua_constant(tolua_S,NULL,"OBJECT_XTRA_SIZE_SUSTAIN",OBJECT_XTRA_SIZE_SUSTAIN);
 tolua_constant(tolua_S,NULL,"OBJECT_XTRA_SIZE_RESIST",OBJECT_XTRA_SIZE_RESIST);
 tolua_constant(tolua_S,NULL,"OBJECT_XTRA_SIZE_POWER",OBJECT_XTRA_SIZE_POWER);
 tolua_constant(tolua_S,NULL,"ACT_ILLUMINATION",ACT_ILLUMINATION);
 tolua_constant(tolua_S,NULL,"ACT_MAGIC_MAP",ACT_MAGIC_MAP);
 tolua_constant(tolua_S,NULL,"ACT_CLAIRVOYANCE",ACT_CLAIRVOYANCE);
 tolua_constant(tolua_S,NULL,"ACT_PROT_EVIL",ACT_PROT_EVIL);
 tolua_constant(tolua_S,NULL,"ACT_DISP_EVIL",ACT_DISP_EVIL);
 tolua_constant(tolua_S,NULL,"ACT_HEAL1",ACT_HEAL1);
 tolua_constant(tolua_S,NULL,"ACT_HEAL2",ACT_HEAL2);
 tolua_constant(tolua_S,NULL,"ACT_CURE_WOUNDS",ACT_CURE_WOUNDS);
 tolua_constant(tolua_S,NULL,"ACT_HASTE1",ACT_HASTE1);
 tolua_constant(tolua_S,NULL,"ACT_HASTE2",ACT_HASTE2);
 tolua_constant(tolua_S,NULL,"ACT_FIRE1",ACT_FIRE1);
 tolua_constant(tolua_S,NULL,"ACT_FIRE2",ACT_FIRE2);
 tolua_constant(tolua_S,NULL,"ACT_FIRE3",ACT_FIRE3);
 tolua_constant(tolua_S,NULL,"ACT_FROST1",ACT_FROST1);
 tolua_constant(tolua_S,NULL,"ACT_FROST2",ACT_FROST2);
 tolua_constant(tolua_S,NULL,"ACT_FROST3",ACT_FROST3);
 tolua_constant(tolua_S,NULL,"ACT_FROST4",ACT_FROST4);
 tolua_constant(tolua_S,NULL,"ACT_FROST5",ACT_FROST5);
 tolua_constant(tolua_S,NULL,"ACT_ACID1",ACT_ACID1);
 tolua_constant(tolua_S,NULL,"ACT_RECHARGE1",ACT_RECHARGE1);
 tolua_constant(tolua_S,NULL,"ACT_SLEEP",ACT_SLEEP);
 tolua_constant(tolua_S,NULL,"ACT_LIGHTNING_BOLT",ACT_LIGHTNING_BOLT);
 tolua_constant(tolua_S,NULL,"ACT_ELEC2",ACT_ELEC2);
 tolua_constant(tolua_S,NULL,"ACT_GENOCIDE",ACT_GENOCIDE);
 tolua_constant(tolua_S,NULL,"ACT_MASS_GENOCIDE",ACT_MASS_GENOCIDE);
 tolua_constant(tolua_S,NULL,"ACT_IDENTIFY",ACT_IDENTIFY);
 tolua_constant(tolua_S,NULL,"ACT_DRAIN_LIFE1",ACT_DRAIN_LIFE1);
 tolua_constant(tolua_S,NULL,"ACT_DRAIN_LIFE2",ACT_DRAIN_LIFE2);
 tolua_constant(tolua_S,NULL,"ACT_BIZZARE",ACT_BIZZARE);
 tolua_constant(tolua_S,NULL,"ACT_STAR_BALL",ACT_STAR_BALL);
 tolua_constant(tolua_S,NULL,"ACT_RAGE_BLESS_RESIST",ACT_RAGE_BLESS_RESIST);
 tolua_constant(tolua_S,NULL,"ACT_PHASE",ACT_PHASE);
 tolua_constant(tolua_S,NULL,"ACT_TRAP_DOOR_DEST",ACT_TRAP_DOOR_DEST);
 tolua_constant(tolua_S,NULL,"ACT_DETECT",ACT_DETECT);
 tolua_constant(tolua_S,NULL,"ACT_RESIST",ACT_RESIST);
 tolua_constant(tolua_S,NULL,"ACT_TELEPORT",ACT_TELEPORT);
 tolua_constant(tolua_S,NULL,"ACT_RESTORE_LIFE",ACT_RESTORE_LIFE);
 tolua_constant(tolua_S,NULL,"ACT_MISSILE",ACT_MISSILE);
 tolua_constant(tolua_S,NULL,"ACT_ARROW",ACT_ARROW);
 tolua_constant(tolua_S,NULL,"ACT_REM_FEAR_POIS",ACT_REM_FEAR_POIS);
 tolua_constant(tolua_S,NULL,"ACT_STINKING_CLOUD",ACT_STINKING_CLOUD);
 tolua_constant(tolua_S,NULL,"ACT_STONE_TO_MUD",ACT_STONE_TO_MUD);
 tolua_constant(tolua_S,NULL,"ACT_TELE_AWAY",ACT_TELE_AWAY);
 tolua_constant(tolua_S,NULL,"ACT_WOR",ACT_WOR);
 tolua_constant(tolua_S,NULL,"ACT_CONFUSE",ACT_CONFUSE);
 tolua_constant(tolua_S,NULL,"ACT_PROBE",ACT_PROBE);
 tolua_constant(tolua_S,NULL,"ACT_FIREBRAND",ACT_FIREBRAND);
 tolua_constant(tolua_S,NULL,"ACT_MAX",ACT_MAX);
 tolua_cclass(tolua_S,"object_type","");
 tolua_tablevar(tolua_S,"object_type","k_idx",toluaI_get_object_object_type_k_idx,toluaI_set_object_object_type_k_idx);
 tolua_tablevar(tolua_S,"object_type","iy",toluaI_get_object_object_type_iy,toluaI_set_object_object_type_iy);
 tolua_tablevar(tolua_S,"object_type","ix",toluaI_get_object_object_type_ix,toluaI_set_object_object_type_ix);
 tolua_tablevar(tolua_S,"object_type","tval",toluaI_get_object_object_type_tval,toluaI_set_object_object_type_tval);
 tolua_tablevar(tolua_S,"object_type","sval",toluaI_get_object_object_type_sval,toluaI_set_object_object_type_sval);
 tolua_tablevar(tolua_S,"object_type","pval",toluaI_get_object_object_type_pval,toluaI_set_object_object_type_pval);
 tolua_tablevar(tolua_S,"object_type","discount",toluaI_get_object_object_type_discount,toluaI_set_object_object_type_discount);
 tolua_tablevar(tolua_S,"object_type","number",toluaI_get_object_object_type_number,toluaI_set_object_object_type_number);
 tolua_tablevar(tolua_S,"object_type","weight",toluaI_get_object_object_type_weight,toluaI_set_object_object_type_weight);
 tolua_tablevar(tolua_S,"object_type","name1",toluaI_get_object_object_type_name1,toluaI_set_object_object_type_name1);
 tolua_tablevar(tolua_S,"object_type","name2",toluaI_get_object_object_type_name2,toluaI_set_object_object_type_name2);
 tolua_tablevar(tolua_S,"object_type","xtra1",toluaI_get_object_object_type_xtra1,toluaI_set_object_object_type_xtra1);
 tolua_tablevar(tolua_S,"object_type","xtra2",toluaI_get_object_object_type_xtra2,toluaI_set_object_object_type_xtra2);
 tolua_tablevar(tolua_S,"object_type","to_h",toluaI_get_object_object_type_to_h,toluaI_set_object_object_type_to_h);
 tolua_tablevar(tolua_S,"object_type","to_d",toluaI_get_object_object_type_to_d,toluaI_set_object_object_type_to_d);
 tolua_tablevar(tolua_S,"object_type","to_a",toluaI_get_object_object_type_to_a,toluaI_set_object_object_type_to_a);
 tolua_tablevar(tolua_S,"object_type","ac",toluaI_get_object_object_type_ac,toluaI_set_object_object_type_ac);
 tolua_tablevar(tolua_S,"object_type","dd",toluaI_get_object_object_type_dd,toluaI_set_object_object_type_dd);
 tolua_tablevar(tolua_S,"object_type","ds",toluaI_get_object_object_type_ds,toluaI_set_object_object_type_ds);
 tolua_tablevar(tolua_S,"object_type","timeout",toluaI_get_object_object_type_timeout,toluaI_set_object_object_type_timeout);
 tolua_tablevar(tolua_S,"object_type","ident",toluaI_get_object_object_type_ident,toluaI_set_object_object_type_ident);
 tolua_tablevar(tolua_S,"object_type","marked",toluaI_get_object_object_type_marked,toluaI_set_object_object_type_marked);
 tolua_tablevar(tolua_S,"object_type","note",toluaI_get_object_object_type_note,toluaI_set_object_object_type_note);
 tolua_tablevar(tolua_S,"object_type","next_o_idx",toluaI_get_object_object_type_next_o_idx,toluaI_set_object_object_type_next_o_idx);
 tolua_tablevar(tolua_S,"object_type","held_m_idx",toluaI_get_object_object_type_held_m_idx,toluaI_set_object_object_type_held_m_idx);
 tolua_cclass(tolua_S,"object_kind","");
 tolua_tablevar(tolua_S,"object_kind","name",toluaI_get_object_object_kind_name,toluaI_set_object_object_kind_name);
 tolua_tablevar(tolua_S,"object_kind","text",toluaI_get_object_object_kind_text,toluaI_set_object_object_kind_text);
 tolua_tablevar(tolua_S,"object_kind","tval",toluaI_get_object_object_kind_tval,toluaI_set_object_object_kind_tval);
 tolua_tablevar(tolua_S,"object_kind","sval",toluaI_get_object_object_kind_sval,toluaI_set_object_object_kind_sval);
 tolua_tablevar(tolua_S,"object_kind","pval",toluaI_get_object_object_kind_pval,toluaI_set_object_object_kind_pval);
 tolua_tablevar(tolua_S,"object_kind","to_h",toluaI_get_object_object_kind_to_h,toluaI_set_object_object_kind_to_h);
 tolua_tablevar(tolua_S,"object_kind","to_d",toluaI_get_object_object_kind_to_d,toluaI_set_object_object_kind_to_d);
 tolua_tablevar(tolua_S,"object_kind","to_a",toluaI_get_object_object_kind_to_a,toluaI_set_object_object_kind_to_a);
 tolua_tablevar(tolua_S,"object_kind","ac",toluaI_get_object_object_kind_ac,toluaI_set_object_object_kind_ac);
 tolua_tablevar(tolua_S,"object_kind","dd",toluaI_get_object_object_kind_dd,toluaI_set_object_object_kind_dd);
 tolua_tablevar(tolua_S,"object_kind","ds",toluaI_get_object_object_kind_ds,toluaI_set_object_object_kind_ds);
 tolua_tablevar(tolua_S,"object_kind","weight",toluaI_get_object_object_kind_weight,toluaI_set_object_object_kind_weight);
 tolua_tablevar(tolua_S,"object_kind","cost",toluaI_get_object_object_kind_cost,toluaI_set_object_object_kind_cost);
 tolua_tablevar(tolua_S,"object_kind","flags1",toluaI_get_object_object_kind_flags1,toluaI_set_object_object_kind_flags1);
 tolua_tablevar(tolua_S,"object_kind","flags2",toluaI_get_object_object_kind_flags2,toluaI_set_object_object_kind_flags2);
 tolua_tablevar(tolua_S,"object_kind","flags3",toluaI_get_object_object_kind_flags3,toluaI_set_object_object_kind_flags3);
 tolua_tablearray(tolua_S,"object_kind","locale",toluaI_get_object_object_kind_locale,toluaI_set_object_object_kind_locale);
 tolua_tablearray(tolua_S,"object_kind","chance",toluaI_get_object_object_kind_chance,toluaI_set_object_object_kind_chance);
 tolua_tablevar(tolua_S,"object_kind","level",toluaI_get_object_object_kind_level,toluaI_set_object_object_kind_level);
 tolua_tablevar(tolua_S,"object_kind","extra",toluaI_get_object_object_kind_extra,toluaI_set_object_object_kind_extra);
 tolua_tablevar(tolua_S,"object_kind","d_attr",toluaI_get_object_object_kind_d_attr,toluaI_set_object_object_kind_d_attr);
 tolua_tablevar(tolua_S,"object_kind","d_char",toluaI_get_object_object_kind_d_char,toluaI_set_object_object_kind_d_char);
 tolua_tablevar(tolua_S,"object_kind","x_attr",toluaI_get_object_object_kind_x_attr,toluaI_set_object_object_kind_x_attr);
 tolua_tablevar(tolua_S,"object_kind","x_char",toluaI_get_object_object_kind_x_char,toluaI_set_object_object_kind_x_char);
 tolua_tablevar(tolua_S,"object_kind","flavor",toluaI_get_object_object_kind_flavor,toluaI_set_object_object_kind_flavor);
 tolua_tablevar(tolua_S,"object_kind","aware",toluaI_get_object_object_kind_aware,toluaI_set_object_object_kind_aware);
 tolua_tablevar(tolua_S,"object_kind","tried",toluaI_get_object_object_kind_tried,toluaI_set_object_object_kind_tried);
 tolua_cclass(tolua_S,"artifact_type","");
 tolua_tablevar(tolua_S,"artifact_type","name",toluaI_get_object_artifact_type_name,toluaI_set_object_artifact_type_name);
 tolua_tablevar(tolua_S,"artifact_type","text",toluaI_get_object_artifact_type_text,toluaI_set_object_artifact_type_text);
 tolua_tablevar(tolua_S,"artifact_type","tval",toluaI_get_object_artifact_type_tval,toluaI_set_object_artifact_type_tval);
 tolua_tablevar(tolua_S,"artifact_type","sval",toluaI_get_object_artifact_type_sval,toluaI_set_object_artifact_type_sval);
 tolua_tablevar(tolua_S,"artifact_type","pval",toluaI_get_object_artifact_type_pval,toluaI_set_object_artifact_type_pval);
 tolua_tablevar(tolua_S,"artifact_type","to_h",toluaI_get_object_artifact_type_to_h,toluaI_set_object_artifact_type_to_h);
 tolua_tablevar(tolua_S,"artifact_type","to_d",toluaI_get_object_artifact_type_to_d,toluaI_set_object_artifact_type_to_d);
 tolua_tablevar(tolua_S,"artifact_type","to_a",toluaI_get_object_artifact_type_to_a,toluaI_set_object_artifact_type_to_a);
 tolua_tablevar(tolua_S,"artifact_type","ac",toluaI_get_object_artifact_type_ac,toluaI_set_object_artifact_type_ac);
 tolua_tablevar(tolua_S,"artifact_type","dd",toluaI_get_object_artifact_type_dd,toluaI_set_object_artifact_type_dd);
 tolua_tablevar(tolua_S,"artifact_type","ds",toluaI_get_object_artifact_type_ds,toluaI_set_object_artifact_type_ds);
 tolua_tablevar(tolua_S,"artifact_type","weight",toluaI_get_object_artifact_type_weight,toluaI_set_object_artifact_type_weight);
 tolua_tablevar(tolua_S,"artifact_type","cost",toluaI_get_object_artifact_type_cost,toluaI_set_object_artifact_type_cost);
 tolua_tablevar(tolua_S,"artifact_type","flags1",toluaI_get_object_artifact_type_flags1,toluaI_set_object_artifact_type_flags1);
 tolua_tablevar(tolua_S,"artifact_type","flags2",toluaI_get_object_artifact_type_flags2,toluaI_set_object_artifact_type_flags2);
 tolua_tablevar(tolua_S,"artifact_type","flags3",toluaI_get_object_artifact_type_flags3,toluaI_set_object_artifact_type_flags3);
 tolua_tablevar(tolua_S,"artifact_type","level",toluaI_get_object_artifact_type_level,toluaI_set_object_artifact_type_level);
 tolua_tablevar(tolua_S,"artifact_type","rarity",toluaI_get_object_artifact_type_rarity,toluaI_set_object_artifact_type_rarity);
 tolua_tablevar(tolua_S,"artifact_type","cur_num",toluaI_get_object_artifact_type_cur_num,toluaI_set_object_artifact_type_cur_num);
 tolua_tablevar(tolua_S,"artifact_type","max_num",toluaI_get_object_artifact_type_max_num,toluaI_set_object_artifact_type_max_num);
 tolua_tablevar(tolua_S,"artifact_type","activation",toluaI_get_object_artifact_type_activation,toluaI_set_object_artifact_type_activation);
 tolua_tablevar(tolua_S,"artifact_type","time",toluaI_get_object_artifact_type_time,toluaI_set_object_artifact_type_time);
 tolua_tablevar(tolua_S,"artifact_type","randtime",toluaI_get_object_artifact_type_randtime,toluaI_set_object_artifact_type_randtime);
 tolua_cclass(tolua_S,"ego_item_type","");
 tolua_tablevar(tolua_S,"ego_item_type","name",toluaI_get_object_ego_item_type_name,toluaI_set_object_ego_item_type_name);
 tolua_tablevar(tolua_S,"ego_item_type","text",toluaI_get_object_ego_item_type_text,toluaI_set_object_ego_item_type_text);
 tolua_tablevar(tolua_S,"ego_item_type","slot",toluaI_get_object_ego_item_type_slot,toluaI_set_object_ego_item_type_slot);
 tolua_tablevar(tolua_S,"ego_item_type","rating",toluaI_get_object_ego_item_type_rating,toluaI_set_object_ego_item_type_rating);
 tolua_tablevar(tolua_S,"ego_item_type","level",toluaI_get_object_ego_item_type_level,toluaI_set_object_ego_item_type_level);
 tolua_tablevar(tolua_S,"ego_item_type","rarity",toluaI_get_object_ego_item_type_rarity,toluaI_set_object_ego_item_type_rarity);
 tolua_tablearray(tolua_S,"ego_item_type","tval",toluaI_get_object_ego_item_type_tval,toluaI_set_object_ego_item_type_tval);
 tolua_tablearray(tolua_S,"ego_item_type","min_sval",toluaI_get_object_ego_item_type_min_sval,toluaI_set_object_ego_item_type_min_sval);
 tolua_tablearray(tolua_S,"ego_item_type","max_sval",toluaI_get_object_ego_item_type_max_sval,toluaI_set_object_ego_item_type_max_sval);
 tolua_tablevar(tolua_S,"ego_item_type","xtra",toluaI_get_object_ego_item_type_xtra,toluaI_set_object_ego_item_type_xtra);
 tolua_tablevar(tolua_S,"ego_item_type","max_to_h",toluaI_get_object_ego_item_type_max_to_h,toluaI_set_object_ego_item_type_max_to_h);
 tolua_tablevar(tolua_S,"ego_item_type","max_to_d",toluaI_get_object_ego_item_type_max_to_d,toluaI_set_object_ego_item_type_max_to_d);
 tolua_tablevar(tolua_S,"ego_item_type","max_to_a",toluaI_get_object_ego_item_type_max_to_a,toluaI_set_object_ego_item_type_max_to_a);
 tolua_tablevar(tolua_S,"ego_item_type","max_pval",toluaI_get_object_ego_item_type_max_pval,toluaI_set_object_ego_item_type_max_pval);
 tolua_tablevar(tolua_S,"ego_item_type","cost",toluaI_get_object_ego_item_type_cost,toluaI_set_object_ego_item_type_cost);
 tolua_tablevar(tolua_S,"ego_item_type","flags1",toluaI_get_object_ego_item_type_flags1,toluaI_set_object_ego_item_type_flags1);
 tolua_tablevar(tolua_S,"ego_item_type","flags2",toluaI_get_object_ego_item_type_flags2,toluaI_set_object_ego_item_type_flags2);
 tolua_tablevar(tolua_S,"ego_item_type","flags3",toluaI_get_object_ego_item_type_flags3,toluaI_set_object_ego_item_type_flags3);
 tolua_cclass(tolua_S,"flavor_type","");
 tolua_tablevar(tolua_S,"flavor_type","text",toluaI_get_object_flavor_type_text,toluaI_set_object_flavor_type_text);
 tolua_tablevar(tolua_S,"flavor_type","tval",toluaI_get_object_flavor_type_tval,toluaI_set_object_flavor_type_tval);
 tolua_tablevar(tolua_S,"flavor_type","d_attr",toluaI_get_object_flavor_type_d_attr,toluaI_set_object_flavor_type_d_attr);
 tolua_tablevar(tolua_S,"flavor_type","d_char",toluaI_get_object_flavor_type_d_char,toluaI_set_object_flavor_type_d_char);
 tolua_tablevar(tolua_S,"flavor_type","x_attr",toluaI_get_object_flavor_type_x_attr,toluaI_set_object_flavor_type_x_attr);
 tolua_tablevar(tolua_S,"flavor_type","x_char",toluaI_get_object_flavor_type_x_char,toluaI_set_object_flavor_type_x_char);
 tolua_globalvar(tolua_S,"o_max",toluaI_get_object_o_max,toluaI_set_object_o_max);
 tolua_globalvar(tolua_S,"o_cnt",toluaI_get_object_o_cnt,toluaI_set_object_o_cnt);
 tolua_globalarray(tolua_S,"o_list",toluaI_get_object_o_list,toluaI_set_object_o_list);
 tolua_globalarray(tolua_S,"k_info",toluaI_get_object_k_info,toluaI_set_object_k_info);
 tolua_globalvar(tolua_S,"k_name",toluaI_get_object_k_name,toluaI_set_object_k_name);
 tolua_globalvar(tolua_S,"k_text",toluaI_get_object_k_text,toluaI_set_object_k_text);
 tolua_globalarray(tolua_S,"a_info",toluaI_get_object_a_info,toluaI_set_object_a_info);
 tolua_globalvar(tolua_S,"a_name",toluaI_get_object_a_name,toluaI_set_object_a_name);
 tolua_globalvar(tolua_S,"a_text",toluaI_get_object_a_text,toluaI_set_object_a_text);
 tolua_globalarray(tolua_S,"e_info",toluaI_get_object_e_info,toluaI_set_object_e_info);
 tolua_globalvar(tolua_S,"e_name",toluaI_get_object_e_name,toluaI_set_object_e_name);
 tolua_globalvar(tolua_S,"e_text",toluaI_get_object_e_text,toluaI_set_object_e_text);
 tolua_globalarray(tolua_S,"flavor_info",toluaI_get_object_flavor_info,toluaI_set_object_flavor_info);
 tolua_globalvar(tolua_S,"flavor_name",toluaI_get_object_flavor_name,toluaI_set_object_flavor_name);
 tolua_globalvar(tolua_S,"flavor_text",toluaI_get_object_flavor_text,toluaI_set_object_flavor_text);
 tolua_function(tolua_S,NULL,"flavor_init",toluaI_object_flavor_init00);
 tolua_function(tolua_S,NULL,"reset_visuals",toluaI_object_reset_visuals00);
 tolua_function(tolua_S,NULL,"object_flags",toluaI_object_object_flags00);
 tolua_function(tolua_S,NULL,"object_flags_known",toluaI_object_object_flags_known00);
 tolua_function(tolua_S,NULL,"identify_random_gen",toluaI_object_identify_random_gen00);
 tolua_function(tolua_S,NULL,"identify_fully_aux",toluaI_object_identify_fully_aux00);
 tolua_function(tolua_S,NULL,"index_to_label",toluaI_object_index_to_label00);
 tolua_function(tolua_S,NULL,"label_to_inven",toluaI_object_label_to_inven00);
 tolua_function(tolua_S,NULL,"label_to_equip",toluaI_object_label_to_equip00);
 tolua_function(tolua_S,NULL,"wield_slot",toluaI_object_wield_slot00);
 tolua_function(tolua_S,NULL,"mention_use",toluaI_object_mention_use00);
 tolua_function(tolua_S,NULL,"describe_use",toluaI_object_describe_use00);
 tolua_function(tolua_S,NULL,"item_tester_okay",toluaI_object_item_tester_okay00);
 tolua_function(tolua_S,NULL,"scan_floor",toluaI_object_scan_floor00);
 tolua_function(tolua_S,NULL,"display_inven",toluaI_object_display_inven00);
 tolua_function(tolua_S,NULL,"display_equip",toluaI_object_display_equip00);
 tolua_function(tolua_S,NULL,"show_inven",toluaI_object_show_inven00);
 tolua_function(tolua_S,NULL,"show_equip",toluaI_object_show_equip00);
 tolua_function(tolua_S,NULL,"toggle_inven_equip",toluaI_object_toggle_inven_equip00);
 tolua_function(tolua_S,NULL,"get_item",toluaI_object_get_item00);
 tolua_function(tolua_S,NULL,"excise_object_idx",toluaI_object_excise_object_idx00);
 tolua_function(tolua_S,NULL,"delete_object_idx",toluaI_object_delete_object_idx00);
 tolua_function(tolua_S,NULL,"delete_object",toluaI_object_delete_object00);
 tolua_function(tolua_S,NULL,"compact_objects",toluaI_object_compact_objects00);
 tolua_function(tolua_S,NULL,"wipe_o_list",toluaI_object_wipe_o_list00);
 tolua_function(tolua_S,NULL,"o_pop",toluaI_object_o_pop00);
 tolua_function(tolua_S,NULL,"get_obj_num_prep",toluaI_object_get_obj_num_prep00);
 tolua_function(tolua_S,NULL,"get_obj_num",toluaI_object_get_obj_num00);
 tolua_function(tolua_S,NULL,"object_known",toluaI_object_object_known00);
 tolua_function(tolua_S,NULL,"object_aware",toluaI_object_object_aware00);
 tolua_function(tolua_S,NULL,"object_tried",toluaI_object_object_tried00);
 tolua_function(tolua_S,NULL,"object_value",toluaI_object_object_value00);
 tolua_function(tolua_S,NULL,"object_similar",toluaI_object_object_similar00);
 tolua_function(tolua_S,NULL,"object_absorb",toluaI_object_object_absorb00);
 tolua_function(tolua_S,NULL,"lookup_kind",toluaI_object_lookup_kind00);
 tolua_function(tolua_S,NULL,"object_wipe",toluaI_object_object_wipe00);
 tolua_function(tolua_S,NULL,"object_copy",toluaI_object_object_copy00);
 tolua_function(tolua_S,NULL,"object_prep",toluaI_object_object_prep00);
 tolua_function(tolua_S,NULL,"apply_magic",toluaI_object_apply_magic00);
 tolua_function(tolua_S,NULL,"make_object",toluaI_object_make_object00);
 tolua_function(tolua_S,NULL,"make_gold",toluaI_object_make_gold00);
 tolua_function(tolua_S,NULL,"floor_carry",toluaI_object_floor_carry00);
 tolua_function(tolua_S,NULL,"drop_near",toluaI_object_drop_near00);
 tolua_function(tolua_S,NULL,"acquirement",toluaI_object_acquirement00);
 tolua_function(tolua_S,NULL,"place_object",toluaI_object_place_object00);
 tolua_function(tolua_S,NULL,"place_gold",toluaI_object_place_gold00);
 tolua_function(tolua_S,NULL,"pick_trap",toluaI_object_pick_trap00);
 tolua_function(tolua_S,NULL,"place_trap",toluaI_object_place_trap00);
 tolua_function(tolua_S,NULL,"place_secret_door",toluaI_object_place_secret_door00);
 tolua_function(tolua_S,NULL,"place_closed_door",toluaI_object_place_closed_door00);
 tolua_function(tolua_S,NULL,"place_random_door",toluaI_object_place_random_door00);
 tolua_function(tolua_S,NULL,"inven_item_charges",toluaI_object_inven_item_charges00);
 tolua_function(tolua_S,NULL,"inven_item_describe",toluaI_object_inven_item_describe00);
 tolua_function(tolua_S,NULL,"inven_item_increase",toluaI_object_inven_item_increase00);
 tolua_function(tolua_S,NULL,"inven_item_optimize",toluaI_object_inven_item_optimize00);
 tolua_function(tolua_S,NULL,"floor_item_charges",toluaI_object_floor_item_charges00);
 tolua_function(tolua_S,NULL,"floor_item_describe",toluaI_object_floor_item_describe00);
 tolua_function(tolua_S,NULL,"floor_item_increase",toluaI_object_floor_item_increase00);
 tolua_function(tolua_S,NULL,"floor_item_optimize",toluaI_object_floor_item_optimize00);
 tolua_function(tolua_S,NULL,"inven_carry_okay",toluaI_object_inven_carry_okay00);
 tolua_function(tolua_S,NULL,"inven_carry",toluaI_object_inven_carry00);
 tolua_function(tolua_S,NULL,"inven_takeoff",toluaI_object_inven_takeoff00);
 tolua_function(tolua_S,NULL,"inven_drop",toluaI_object_inven_drop00);
 tolua_function(tolua_S,NULL,"combine_pack",toluaI_object_combine_pack00);
 tolua_function(tolua_S,NULL,"reorder_pack",toluaI_object_reorder_pack00);
 tolua_function(tolua_S,NULL,"object_aware_p",toluaI_object_object_aware_p00);
 tolua_function(tolua_S,NULL,"object_tried_p",toluaI_object_object_tried_p00);
 tolua_function(tolua_S,NULL,"object_known_p",toluaI_object_object_known_p00);
 tolua_function(tolua_S,NULL,"object_attr",toluaI_object_object_attr00);
 tolua_function(tolua_S,NULL,"object_char",toluaI_object_object_char00);
 tolua_function(tolua_S,NULL,"artifact_p",toluaI_object_artifact_p00);
 tolua_function(tolua_S,NULL,"ego_item_p",toluaI_object_ego_item_p00);
 tolua_function(tolua_S,NULL,"broken_p",toluaI_object_broken_p00);
 tolua_function(tolua_S,NULL,"cursed_p",toluaI_object_cursed_p00);
 return 1;
}
/* Close function */
void tolua_object_close (lua_State* tolua_S)
{
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ART_POWER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ART_MORGOTH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ART_GROND");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ART_MIN_NORMAL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_RESIST_ACID");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_RESIST_ELEC");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_RESIST_FIRE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_RESIST_COLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_RESISTANCE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_ELVENKIND");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_ARMR_VULN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_PERMANENCE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_ARMR_DWARVEN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_ENDURE_ACID");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_ENDURE_ELEC");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_ENDURE_FIRE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_ENDURE_COLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_ENDURANCE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_SHIELD_ELVENKIND");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_SHIELD_PRESERVATION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_SHIELD_VULN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_INTELLIGENCE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_WISDOM");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_BEAUTY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_MAGI");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_MIGHT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_LORDLINESS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_SEEING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_INFRAVISION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_LITE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_TELEPATHY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_REGENERATION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_TELEPORTATION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_STUPIDITY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_NAIVETY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_UGLINESS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_SICKLINESS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_PROTECTION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_STEALTH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_AMAN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_CLOAK_MAGI");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_ENVELOPING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_VULNERABILITY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_IRRITATION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_FREE_ACTION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_SLAYING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_AGILITY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_POWER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_GLOVES_THIEVERY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_GAUNTLETS_COMBAT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_WEAKNESS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_CLUMSINESS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_SLOW_DESCENT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_QUIET");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_MOTION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_SPEED");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_STABILITY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_NOISE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_SLOWNESS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_ANNOYANCE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_HA");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_DF");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_BLESS_BLADE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_GONDOLIN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_WEST");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_ATTACKS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_FURY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_BRAND_ACID");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_BRAND_ELEC");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_BRAND_FIRE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_BRAND_COLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_BRAND_POIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_SLAY_ANIMAL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_SLAY_EVIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_SLAY_UNDEAD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_SLAY_DEMON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_SLAY_ORC");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_SLAY_TROLL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_SLAY_GIANT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_SLAY_DRAGON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_KILL_ANIMAL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_KILL_EVIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_KILL_UNDEAD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_KILL_DEMON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_KILL_ORC");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_KILL_TROLL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_KILL_GIANT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_KILL_DRAGON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_DIGGING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_DIGGER_EARTHQUAKE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_MORGUL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_ACCURACY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_VELOCITY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_BOW_LORIEN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_CROSSBOW_HARAD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_EXTRA_MIGHT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_EXTRA_SHOTS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_SLING_BUCKLAND");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_NAZGUL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_HURT_ANIMAL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_HURT_EVIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_HURT_UNDEAD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_HURT_DEMON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_HURT_ORC");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_HURT_TROLL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_HURT_GIANT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_HURT_DRAGON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_AMMO_HOLY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_AMMO_VENOM");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_FLAME");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_FROST");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_WOUNDING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_BACKBITING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_SHATTERED");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"EGO_BLASTED");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_SKELETON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_BOTTLE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_JUNK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_SPIKE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_CHEST");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_SHOT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_ARROW");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_BOLT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_BOW");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_DIGGING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_HAFTED");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_POLEARM");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_SWORD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_BOOTS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_GLOVES");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_HELM");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_CROWN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_SHIELD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_CLOAK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_SOFT_ARMOR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_HARD_ARMOR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_DRAG_ARMOR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_LITE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_AMULET");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_RING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_STAFF");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_WAND");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_ROD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_SCROLL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_POTION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_FLASK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_FOOD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_MAGIC_BOOK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_PRAYER_BOOK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TV_GOLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMMO_LIGHT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMMO_NORMAL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMMO_HEAVY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SLING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SHORT_BOW");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_LONG_BOW");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_LIGHT_XBOW");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_HEAVY_XBOW");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SHOVEL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_GNOMISH_SHOVEL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_DWARVEN_SHOVEL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_PICK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ORCISH_PICK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_DWARVEN_PICK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WHIP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_QUARTERSTAFF");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_MACE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_BALL_AND_CHAIN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAR_HAMMER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_LUCERN_HAMMER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_MORNING_STAR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FLAIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_LEAD_FILLED_MACE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_TWO_HANDED_FLAIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_MACE_OF_DISRUPTION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_GROND");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SPEAR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AWL_PIKE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_TRIDENT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_PIKE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_BEAKED_AXE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_BROAD_AXE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_GLAIVE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_HALBERD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCYTHE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_LANCE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_BATTLE_AXE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_GREAT_AXE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_LOCHABER_AXE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCYTHE_OF_SLICING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_BROKEN_DAGGER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_BROKEN_SWORD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_DAGGER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_MAIN_GAUCHE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RAPIER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SMALL_SWORD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SHORT_SWORD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SABRE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_CUTLASS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_TULWAR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_BROAD_SWORD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_LONG_SWORD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCIMITAR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_KATANA");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_BASTARD_SWORD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_TWO_HANDED_SWORD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_EXECUTIONERS_SWORD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_BLADE_OF_CHAOS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SMALL_LEATHER_SHIELD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SMALL_METAL_SHIELD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_LARGE_LEATHER_SHIELD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_LARGE_METAL_SHIELD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SHIELD_OF_DEFLECTION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_HARD_LEATHER_CAP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_METAL_CAP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_IRON_HELM");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STEEL_HELM");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_IRON_CROWN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_GOLDEN_CROWN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_JEWELED_CROWN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_MORGOTH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_PAIR_OF_SOFT_LEATHER_BOOTS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_PAIR_OF_HARD_LEATHER_BOOTS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_PAIR_OF_METAL_SHOD_BOOTS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_CLOAK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SHADOW_CLOAK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SET_OF_LEATHER_GLOVES");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SET_OF_GAUNTLETS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SET_OF_CESTI");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FILTHY_RAG");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROBE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SOFT_LEATHER_ARMOR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SOFT_STUDDED_LEATHER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_HARD_LEATHER_ARMOR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_HARD_STUDDED_LEATHER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_LEATHER_SCALE_MAIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RUSTY_CHAIN_MAIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_METAL_SCALE_MAIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_CHAIN_MAIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AUGMENTED_CHAIN_MAIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_DOUBLE_CHAIN_MAIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_BAR_CHAIN_MAIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_METAL_BRIGANDINE_ARMOUR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_PARTIAL_PLATE_ARMOUR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_METAL_LAMELLAR_ARMOUR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FULL_PLATE_ARMOUR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RIBBED_PLATE_ARMOUR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_MITHRIL_CHAIN_MAIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_MITHRIL_PLATE_MAIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ADAMANTITE_PLATE_MAIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_DRAGON_BLACK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_DRAGON_BLUE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_DRAGON_WHITE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_DRAGON_RED");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_DRAGON_GREEN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_DRAGON_MULTIHUED");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_DRAGON_SHINING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_DRAGON_LAW");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_DRAGON_BRONZE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_DRAGON_GOLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_DRAGON_CHAOS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_DRAGON_BALANCE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_DRAGON_POWER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_LITE_TORCH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_LITE_LANTERN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_LITE_GALADRIEL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_LITE_ELENDIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_LITE_THRAIN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMULET_DOOM");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMULET_TELEPORT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMULET_ADORNMENT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMULET_SLOW_DIGEST");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMULET_RESIST_ACID");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMULET_SEARCHING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMULET_WISDOM");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMULET_CHARISMA");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMULET_THE_MAGI");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMULET_SUSTENANCE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMULET_CARLAMMAS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMULET_INGWE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMULET_DWARVES");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMULET_ESP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMULET_RESIST");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMULET_REGEN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMULET_ELESSAR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMULET_EVENSTAR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMULET_DEVOTION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMULET_WEAPONMASTERY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMULET_TRICKERY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMULET_INFRAVISION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_AMULET_RESIST_LIGHTNING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_WOE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_AGGRAVATION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_WEAKNESS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_STUPIDITY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_TELEPORTATION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_SLOW_DIGESTION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_FEATHER_FALL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_RESIST_FIRE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_RESIST_COLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_SUSTAIN_STR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_SUSTAIN_INT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_SUSTAIN_WIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_SUSTAIN_DEX");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_SUSTAIN_CON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_SUSTAIN_CHR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_PROTECTION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_ACID");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_FLAMES");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_ICE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_RESIST_POIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_FREE_ACTION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_SEE_INVIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_SEARCHING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_STR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_INT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_DEX");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_CON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_ACCURACY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_DAMAGE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_SLAYING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_SPEED");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_BARAHIR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_TULKAS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_NARYA");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_NENYA");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_VILYA");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_POWER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_RING_LIGHTNING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_DARKNESS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_SLOWNESS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_HASTE_MONSTERS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_SUMMONING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_TELEPORTATION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_IDENTIFY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_REMOVE_CURSE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_STARLITE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_LITE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_MAPPING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_DETECT_GOLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_DETECT_ITEM");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_DETECT_TRAP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_DETECT_DOOR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_DETECT_INVIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_DETECT_EVIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_CURE_LIGHT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_CURING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_HEALING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_THE_MAGI");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_SLEEP_MONSTERS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_SLOW_MONSTERS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_SPEED");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_PROBING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_DISPEL_EVIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_POWER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_HOLINESS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_GENOCIDE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_EARTHQUAKES");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_STAFF_DESTRUCTION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_HEAL_MONSTER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_HASTE_MONSTER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_CLONE_MONSTER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_TELEPORT_AWAY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_DISARMING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_TRAP_DOOR_DEST");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_STONE_TO_MUD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_LITE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_SLEEP_MONSTER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_SLOW_MONSTER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_CONFUSE_MONSTER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_FEAR_MONSTER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_DRAIN_LIFE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_POLYMORPH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_STINKING_CLOUD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_MAGIC_MISSILE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_ACID_BOLT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_ELEC_BOLT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_FIRE_BOLT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_COLD_BOLT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_ACID_BALL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_ELEC_BALL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_FIRE_BALL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_COLD_BALL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_WONDER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_ANNIHILATION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_DRAGON_FIRE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_DRAGON_COLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_WAND_DRAGON_BREATH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_DETECT_TRAP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_DETECT_DOOR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_IDENTIFY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_RECALL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_ILLUMINATION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_MAPPING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_DETECTION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_PROBING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_CURING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_HEALING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_RESTORATION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_SPEED");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_TELEPORT_AWAY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_DISARMING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_LITE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_SLEEP_MONSTER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_SLOW_MONSTER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_DRAIN_LIFE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_POLYMORPH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_ACID_BOLT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_ELEC_BOLT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_FIRE_BOLT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_COLD_BOLT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_ACID_BALL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_ELEC_BALL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_FIRE_BALL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_COLD_BALL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_DARKNESS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_AGGRAVATE_MONSTER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_CURSE_ARMOR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_CURSE_WEAPON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_SUMMON_MONSTER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_SUMMON_UNDEAD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_TRAP_CREATION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_PHASE_DOOR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_TELEPORT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_TELEPORT_LEVEL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_WORD_OF_RECALL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_IDENTIFY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_STAR_IDENTIFY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_REMOVE_CURSE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_STAR_REMOVE_CURSE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_ENCHANT_ARMOR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_ENCHANT_WEAPON_TO_HIT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_ENCHANT_WEAPON_TO_DAM");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_STAR_ENCHANT_ARMOR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_STAR_ENCHANT_WEAPON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_RECHARGING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_LIGHT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_MAPPING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_DETECT_GOLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_DETECT_ITEM");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_DETECT_TRAP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_DETECT_DOOR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_DETECT_INVIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_SATISFY_HUNGER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_BLESSING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_HOLY_CHANT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_HOLY_PRAYER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_MONSTER_CONFUSION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_PROTECTION_FROM_EVIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_RUNE_OF_PROTECTION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_TRAP_DOOR_DESTRUCTION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_STAR_DESTRUCTION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_DISPEL_UNDEAD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_GENOCIDE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_MASS_GENOCIDE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_ACQUIREMENT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_SCROLL_STAR_ACQUIREMENT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_WATER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_APPLE_JUICE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_SLIME_MOLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_SLOWNESS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_SALT_WATER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_POISON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_BLINDNESS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_CONFUSION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_SLEEP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_LOSE_MEMORIES");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_RUINATION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_DEC_STR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_DEC_INT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_DEC_WIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_DEC_DEX");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_DEC_CON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_DEC_CHR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_DETONATIONS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_DEATH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_INFRAVISION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_DETECT_INVIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_SLOW_POISON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_CURE_POISON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_BOLDNESS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_SPEED");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_RESIST_HEAT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_RESIST_COLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_HEROISM");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_BERSERK_STRENGTH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_CURE_LIGHT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_CURE_SERIOUS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_CURE_CRITICAL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_HEALING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_STAR_HEALING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_LIFE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_RESTORE_MANA");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_RESTORE_EXP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_RES_STR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_RES_INT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_RES_WIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_RES_DEX");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_RES_CON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_RES_CHR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_INC_STR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_INC_INT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_INC_WIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_INC_DEX");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_INC_CON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_INC_CHR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_AUGMENTATION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_ENLIGHTENMENT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_STAR_ENLIGHTENMENT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_SELF_KNOWLEDGE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_POTION_EXPERIENCE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_POISON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_BLINDNESS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_PARANOIA");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_CONFUSION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_HALLUCINATION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_PARALYSIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_WEAKNESS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_SICKNESS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_STUPIDITY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_NAIVETY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_UNHEALTH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_DISEASE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_CURE_POISON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_CURE_BLINDNESS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_CURE_PARANOIA");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_CURE_CONFUSION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_CURE_SERIOUS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_RESTORE_STR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_RESTORE_CON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_RESTORING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_BISCUIT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_JERKY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_RATION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_SLIME_MOLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_WAYBREAD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_PINT_OF_ALE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_PINT_OF_WINE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_FOOD_MIN_FOOD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_ROD_MIN_DIRECTION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_CHEST_MIN_LARGE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SV_BOOK_MIN_GOOD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"CHEST_LOSE_STR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"CHEST_LOSE_CON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"CHEST_POISON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"CHEST_PARALYZE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"CHEST_EXPLODE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"CHEST_SUMMON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"IDENT_SENSE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"IDENT_FIXED");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"IDENT_EMPTY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"IDENT_KNOWN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"IDENT_RUMOUR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"IDENT_MENTAL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"IDENT_CURSED");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"IDENT_BROKEN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"INSCRIP_NULL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"INSCRIP_TERRIBLE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"INSCRIP_WORTHLESS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"INSCRIP_CURSED");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"INSCRIP_BROKEN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"INSCRIP_AVERAGE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"INSCRIP_GOOD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"INSCRIP_EXCELLENT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"INSCRIP_SPECIAL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"INSCRIP_UNCURSED");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"INSCRIP_INDESTRUCTIBLE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MAX_INSCRIP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_STR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_INT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_WIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_DEX");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_CON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_CHR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_XXX1");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_XXX2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_STEALTH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_SEARCH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_INFRA");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_TUNNEL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_SPEED");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_BLOWS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_SHOTS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_MIGHT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_SLAY_ANIMAL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_SLAY_EVIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_SLAY_UNDEAD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_SLAY_DEMON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_SLAY_ORC");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_SLAY_TROLL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_SLAY_GIANT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_SLAY_DRAGON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_KILL_DRAGON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_KILL_DEMON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_KILL_UNDEAD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_BRAND_POIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_BRAND_ACID");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_BRAND_ELEC");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_BRAND_FIRE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR1_BRAND_COLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_SUST_STR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_SUST_INT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_SUST_WIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_SUST_DEX");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_SUST_CON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_SUST_CHR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_XXX1");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_XXX2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_XXX3");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_XXX4");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_XXX5");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_XXX6");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_IM_ACID");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_IM_ELEC");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_IM_FIRE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_IM_COLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_RES_ACID");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_RES_ELEC");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_RES_FIRE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_RES_COLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_RES_POIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_RES_FEAR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_RES_LITE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_RES_DARK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_RES_BLIND");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_RES_CONFU");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_RES_SOUND");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_RES_SHARD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_RES_NEXUS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_RES_NETHR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_RES_CHAOS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR2_RES_DISEN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_SLOW_DIGEST");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_FEATHER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_LITE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_REGEN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_TELEPATHY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_SEE_INVIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_FREE_ACT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_HOLD_LIFE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_XXX1");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_XXX2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_XXX3");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_XXX4");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_IMPACT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_TELEPORT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_AGGRAVATE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_DRAIN_EXP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_IGNORE_ACID");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_IGNORE_ELEC");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_IGNORE_FIRE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_IGNORE_COLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_XXX5");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_XXX6");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_BLESSED");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_ACTIVATE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_INSTA_ART");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_EASY_KNOW");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_HIDE_TYPE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_SHOW_MODS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_XXX7");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_LIGHT_CURSE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_HEAVY_CURSE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"TR3_PERMA_CURSE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"OBJECT_XTRA_TYPE_SUSTAIN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"OBJECT_XTRA_TYPE_RESIST");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"OBJECT_XTRA_TYPE_POWER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"OBJECT_XTRA_WHAT_SUSTAIN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"OBJECT_XTRA_WHAT_RESIST");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"OBJECT_XTRA_WHAT_POWER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"OBJECT_XTRA_BASE_SUSTAIN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"OBJECT_XTRA_BASE_RESIST");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"OBJECT_XTRA_BASE_POWER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"OBJECT_XTRA_SIZE_SUSTAIN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"OBJECT_XTRA_SIZE_RESIST");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"OBJECT_XTRA_SIZE_POWER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_ILLUMINATION");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_MAGIC_MAP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_CLAIRVOYANCE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_PROT_EVIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_DISP_EVIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_HEAL1");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_HEAL2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_CURE_WOUNDS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_HASTE1");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_HASTE2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_FIRE1");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_FIRE2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_FIRE3");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_FROST1");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_FROST2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_FROST3");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_FROST4");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_FROST5");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_ACID1");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_RECHARGE1");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_SLEEP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_LIGHTNING_BOLT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_ELEC2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_GENOCIDE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_MASS_GENOCIDE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_IDENTIFY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_DRAIN_LIFE1");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_DRAIN_LIFE2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_BIZZARE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_STAR_BALL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_RAGE_BLESS_RESIST");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_PHASE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_TRAP_DOOR_DEST");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_DETECT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_RESIST");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_TELEPORT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_RESTORE_LIFE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_MISSILE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_ARROW");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_REM_FEAR_POIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_STINKING_CLOUD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_STONE_TO_MUD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_TELE_AWAY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_WOR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_CONFUSE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_PROBE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_FIREBRAND");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ACT_MAX");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"object_type");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"object_kind");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"artifact_type");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ego_item_type");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"flavor_type");
 lua_getglobals(tolua_S);
 lua_pushstring(tolua_S,"o_max"); lua_pushnil(tolua_S); lua_rawset(tolua_S,-3);
 lua_pop(tolua_S,1);
 lua_getglobals(tolua_S);
 lua_pushstring(tolua_S,"o_cnt"); lua_pushnil(tolua_S); lua_rawset(tolua_S,-3);
 lua_pop(tolua_S,1);
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"o_list");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"k_info");
 lua_getglobals(tolua_S);
 lua_pushstring(tolua_S,"k_name"); lua_pushnil(tolua_S); lua_rawset(tolua_S,-3);
 lua_pop(tolua_S,1);
 lua_getglobals(tolua_S);
 lua_pushstring(tolua_S,"k_text"); lua_pushnil(tolua_S); lua_rawset(tolua_S,-3);
 lua_pop(tolua_S,1);
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"a_info");
 lua_getglobals(tolua_S);
 lua_pushstring(tolua_S,"a_name"); lua_pushnil(tolua_S); lua_rawset(tolua_S,-3);
 lua_pop(tolua_S,1);
 lua_getglobals(tolua_S);
 lua_pushstring(tolua_S,"a_text"); lua_pushnil(tolua_S); lua_rawset(tolua_S,-3);
 lua_pop(tolua_S,1);
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"e_info");
 lua_getglobals(tolua_S);
 lua_pushstring(tolua_S,"e_name"); lua_pushnil(tolua_S); lua_rawset(tolua_S,-3);
 lua_pop(tolua_S,1);
 lua_getglobals(tolua_S);
 lua_pushstring(tolua_S,"e_text"); lua_pushnil(tolua_S); lua_rawset(tolua_S,-3);
 lua_pop(tolua_S,1);
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"flavor_info");
 lua_getglobals(tolua_S);
 lua_pushstring(tolua_S,"flavor_name"); lua_pushnil(tolua_S); lua_rawset(tolua_S,-3);
 lua_pop(tolua_S,1);
 lua_getglobals(tolua_S);
 lua_pushstring(tolua_S,"flavor_text"); lua_pushnil(tolua_S); lua_rawset(tolua_S,-3);
 lua_pop(tolua_S,1);
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"flavor_init");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"reset_visuals");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"object_flags");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"object_flags_known");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"identify_random_gen");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"identify_fully_aux");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"index_to_label");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"label_to_inven");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"label_to_equip");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"wield_slot");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"mention_use");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"describe_use");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"item_tester_okay");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"scan_floor");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"display_inven");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"display_equip");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"show_inven");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"show_equip");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"toggle_inven_equip");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"get_item");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"excise_object_idx");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"delete_object_idx");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"delete_object");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"compact_objects");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"wipe_o_list");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"o_pop");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"get_obj_num_prep");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"get_obj_num");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"object_known");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"object_aware");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"object_tried");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"object_value");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"object_similar");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"object_absorb");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"lookup_kind");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"object_wipe");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"object_copy");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"object_prep");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"apply_magic");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"make_object");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"make_gold");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"floor_carry");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"drop_near");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"acquirement");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"place_object");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"place_gold");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"pick_trap");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"place_trap");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"place_secret_door");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"place_closed_door");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"place_random_door");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"inven_item_charges");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"inven_item_describe");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"inven_item_increase");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"inven_item_optimize");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"floor_item_charges");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"floor_item_describe");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"floor_item_increase");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"floor_item_optimize");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"inven_carry_okay");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"inven_carry");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"inven_takeoff");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"inven_drop");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"combine_pack");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"reorder_pack");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"object_aware_p");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"object_tried_p");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"object_known_p");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"object_attr");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"object_char");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"artifact_p");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"ego_item_p");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"broken_p");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"cursed_p");
}
