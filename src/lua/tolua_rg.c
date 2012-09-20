/* tolua: register functions
** Support code for Lua bindings.
** Written by Waldemar Celes
** TeCGraf/PUC-Rio
** Jul 1998
** $Id: tolua_rg.c,v 1.2 2002/11/23 21:31:25 rr9 Exp $
*/

/* This code is free software; you can redistribute it and/or modify it. 
** The software provided hereunder is on an "as is" basis, and 
** the author has no obligation to provide maintenance, support, updates,
** enhancements, or modifications. 
*/

#include <stdio.h>

#include "tolua.h"
#include "tolua_rg.h"
#include "tolua_tm.h"
#include "tolua_tt.h"

void tolua_globalvar (lua_State* L, const char* name, lua_CFunction get, lua_CFunction set)
{
 lua_newtable(L);
 lua_pushstring(L,".get");
 lua_pushcfunction(L,get);
 lua_settable(L,-3);
 if (set)
 {
  lua_pushstring(L,".set");
  lua_pushcfunction(L,set);
  lua_settable(L,-3);
 }
 lua_pushvalue(L,-1);  /* duplicate top */
 lua_setglobal(L,name);
 toluaI_tm_global(L,lua_gettop(L));
 lua_pop(L,1);
}

static int toluaI_const_global_array (lua_State* L)
{
 lua_error(L,"value of const array cannot be changed");
 return 0;
}


void tolua_globalarray (lua_State* L, const char* name, lua_CFunction get, lua_CFunction set)
{
 int tag = lua_newtag(L);
 lua_newtable(L);
 lua_settag(L,tag);
 lua_setglobal(L,name);
 
 lua_pushcfunction(L,get);
 lua_settagmethod(L,tag,"gettable"); 
 if (set)
  lua_pushcfunction(L,set);
 else
  lua_pushcfunction(L,toluaI_const_global_array);
 lua_settagmethod(L,tag,"settable");
}

void tolua_tablevar 
(lua_State* L, const char* table, const char* name, lua_CFunction get, lua_CFunction set)
{
 lua_getglobal(L,table);

 lua_pushstring(L,".get");
 lua_gettable(L,-2);
 lua_pushstring(L,name);
 lua_pushcfunction(L,get);
 lua_settable(L,-3);
 lua_pop(L,1);
 if (set)
 {
  lua_pushstring(L,".set");
  lua_gettable(L,-2);
  lua_pushstring(L,name);
  lua_pushcfunction(L,set);
  lua_settable(L,-3);
  lua_pop(L,1);
 }

 lua_pop(L,1);
}

static int toluaI_get_array (lua_State* L)
{
 void* self = tolua_getuserdata(L,1,0);
 const char* field = tolua_getstring(L,2,0);

 if (!field)
  tolua_error(L,"invalid 'field' in accessing array");
 if (!self)
 {
  static char msg[BUFSIZ];
  sprintf(msg,"invalid 'self' in accessing array '%s'",field);
  tolua_error(L,msg);
 }
 toluaI_getregistry(L,"tolua_tbl_itype");
 lua_pushnumber(L,lua_tag(L,1));
 lua_gettable(L,-2);
 lua_getglobal(L,lua_tostring(L,-1));
 lua_pushstring(L,".array");
 lua_gettable(L,-2);             
 lua_pushvalue(L,2);    /* field */
 lua_gettable(L,-2);
 lua_pushstring(L,".self");
 lua_pushvalue(L,1);    /* self */
 lua_rawset(L,-3);
 return 1;
}

static int toluaI_const_array (lua_State* L)
{
 lua_error(L,"value of const field cannot be changed");
 return 0;
}

void tolua_tablearray
(lua_State* L, const char* table, const char* name, lua_CFunction get, lua_CFunction set)
{
 int tag = lua_newtag(L);
 lua_getglobal(L,table);
 lua_pushstring(L,".array");
 lua_rawget(L,-2);
 lua_pushstring(L,name);
 lua_newtable(L);
 lua_settag(L,tag);
 lua_settable(L,-3);
 lua_pop(L,2);

 lua_pushcfunction(L,get);
 lua_settagmethod(L,tag,"gettable");
 if (set)
  lua_pushcfunction(L,set);
 else
  lua_pushcfunction(L,toluaI_const_array);
 lua_settagmethod(L,tag,"settable");

 tolua_tablevar(L,table,name,toluaI_get_array,NULL);
}

void tolua_module (lua_State* L, const char* name)
{
 lua_getglobal(L,name);
 if (!lua_istable(L,-1))
 {
  lua_newtable(L);
  lua_pushstring(L,".get");
  lua_newtable(L);
  lua_settable(L,-3);
  lua_pushstring(L,".set");
  lua_newtable(L);
  lua_settable(L,-3);
  lua_pushvalue(L,-1);  /* duplicate top */
  lua_setglobal(L,name);
  toluaI_tm_module(L,lua_gettop(L));
  lua_pop(L,1);
 }
 lua_pop(L,1);
}

void tolua_cclass (lua_State* L, const char* name, const char* base)
{
 int t;
 lua_newtable(L);
 lua_pushstring(L,".get");
 lua_newtable(L);
 lua_settable(L,-3);
 lua_pushstring(L,".set");
 lua_newtable(L);
 lua_settable(L,-3);
 lua_pushstring(L,".array");
 lua_newtable(L);
 lua_settable(L,-3);
 if (*base != 0)
 {
  lua_pushstring(L,".base");
  lua_getglobal(L,base);
  lua_rawset(L,-3);
 }
 lua_pushvalue(L,-1);  /* duplicate top */
 lua_setglobal(L,name);
 t = lua_gettop(L); 
 toluaI_tm_class(L,t,name);
 toluaI_tt_class(L,t,name,base);
 lua_pop(L,1);
}


void tolua_function (lua_State* L, const char* parent, const char* name, lua_CFunction func)
{
 if (parent==NULL)
 {
  lua_pushcfunction(L,func);
  lua_setglobal(L,name);
 }
 else
 {
  lua_getglobal(L,parent);
  lua_pushstring(L,name);
  lua_pushcfunction(L,func);
  lua_settable(L,-3);
  lua_pop(L,1);
 }
}

void tolua_constant (lua_State* L, char* parent, const char* name, long value)
{
 if (parent==NULL)
 {
  lua_pushnumber(L,value);
  lua_setglobal(L,name);
 }
 else
 {
  lua_getglobal(L,parent);
  lua_pushstring(L,name);
  lua_pushnumber(L,value);
  lua_settable(L,-3);
  lua_pop(L,1);
 }
}

void toluaI_setregistry (lua_State* L, const char* field)
{
 lua_getregistry(L);
 lua_insert(L,-2);
 lua_pushstring(L,field);
 lua_insert(L,-2);
 lua_settable(L,-3);
 lua_pop(L,1);
}

void toluaI_getregistry (lua_State* L, const char* field)
{
 lua_getregistry(L);
 lua_pushstring(L,field);
 lua_gettable(L,-2);
 lua_insert(L,-2);
 lua_pop(L,1);
}
