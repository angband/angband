/* tolua
** Support code for Lua bindings.
** Written by Waldemar Celes
** TeCGraf/PUC-Rio
** Jul 1998
** $Id: tolua_lb.c,v 1.2 2002/11/23 21:31:25 rr9 Exp $
*/

/* This code is free software; you can redistribute it and/or modify it. 
** The software provided hereunder is on an "as is" basis, and 
** the author has no obligation to provide maintenance, support, updates,
** enhancements, or modifications. 
*/

#include "tolua.h"
#include "tolua_rg.h"
#include "tolua_tm.h"
#include "tolua_tt.h"


int tolua_open (lua_State* L)
{
 int tolua_tolua_open (lua_State* L);
 /* check if alread opened */
 toluaI_getregistry(L,"TOLUA");
 if (lua_isnil(L,-1))
 {
  lua_pushnumber(L,1);
  toluaI_setregistry(L,"TOLUA");
  toluaI_tt_init(L);
  toluaI_tm_init(L);
  lua_getglobal(L,"foreach");
  toluaI_setregistry(L,"tolua_orig_foreach");
  tolua_tolua_open(L);	/* opens tolua own binding */
 }
 lua_pop(L,1);
 return 1;
}

void tolua_using (lua_State* L, int module)
{
 toluaI_tm_using(L,module);
}

void tolua_class (lua_State* L, int derived, int base)
{
 int tag = lua_newtag(L);    /* new tag of instances of that class */
 toluaI_tm_setclass(L,derived);
 toluaI_tm_linstance(L,tag,derived);
 lua_pushvalue(L,derived);
 lua_pushstring(L,".base");
 lua_pushvalue(L,base);
 lua_rawset(L,-3); 
 lua_pushstring(L,".itag");
 lua_pushnumber(L,tag);
 lua_rawset(L,-3);
 lua_pop(L,1);
}

void tolua_instance (lua_State* L, int instance, int classobj)
{
 int tag;
 lua_pushvalue(L,classobj);
 lua_pushstring(L,".itag");
 lua_gettable(L,-2);
 tag = (int) lua_tonumber(L,-1);
 lua_pop(L,2);  /* number and table */
 if (tag==0)
  tolua_error(L,"unregistered 'classobj' in function 'tolua_instance'.");
 lua_pushvalue(L,instance);
 lua_settag(L,tag);
 lua_pop(L,1);
}

static int filter (lua_State* L)
{
 int n = 1; /* name */
 int v = 2; /* value */
 int f = lua_gettop(L); /* function */
 /* do not pass string fields starting with a dot */
 if (!lua_isstring(L,n) || *lua_tostring(L,n)!='.')
 {
  lua_pushvalue(L,f);
  lua_pushvalue(L,n);
  lua_pushvalue(L,v);
  lua_call(L,2,1);
 }
 else
  lua_pushnil(L);
 return 1;
}

void tolua_foreach (lua_State* L, int lo, int f)
{
 if (toluaI_tt_isusertype(L,lo))
 {
  toluaI_tm_pushmate(L,lo);
  if (lua_isnil(L,-1))
   return;    /* no field in mate table */
  else
   lo = lua_gettop(L);
 }
 toluaI_getregistry(L,"tolua_orig_foreach");
 lua_pushvalue(L,lo);
 lua_pushvalue(L,f);
 lua_pushcclosure(L,filter,1);
 lua_call(L,2,1);
}

const char* tolua_type (lua_State* L, int lo)
{
 return toluaI_tt_getobjtype(L,lo);
}

int tolua_tag (lua_State* L, const char* type)
{
 return toluaI_tt_gettag(L,type);
}

int tolua_base (lua_State* L, int lo)
{
 if (toluaI_tt_isusertype(L,lo))
 {
  toluaI_tm_pushclass(L,lo);
  return lua_gettop(L);
 }
 else if (lua_istable(L,lo))
 {
  lua_pushvalue(L,lo); 
  lua_pushstring(L,".base");
  lua_rawget(L,-2);
  return -1;
 }
 else
  return 0;
}

int tolua_cast (lua_State* L, int lo, char* type)
{
 if (lua_isuserdata(L,lo))
 {
  tolua_pushusertype(L,lua_touserdata(L,lo),toluaI_tt_gettag(L,type));
  return -1;
 }
 else
  return 0;
}

void tolua_takeownership (lua_State* L, int lo)
{
 if (toluaI_tt_isusertype(L,lo))
 {
  /* force garbage collection to avoid C to reuse a to-be-collected address */
  lua_setgcthreshold(L,0);
  tolua_doclone(L,lua_touserdata(L,lo),lua_tag(L,lo));  
 }
 else
  tolua_error(L,"cannot take ownership of specified obejct.");
}

