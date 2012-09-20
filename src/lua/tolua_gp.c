/* tolua: get & push functions.
** Support code for Lua bindings.
** Written by Waldemar Celes
** TeCGraf/PUC-Rio
** Jul 1998
** $Id: tolua_gp.c,v 1.2 2002/11/23 21:31:25 rr9 Exp $
*/

/* This code is free software; you can redistribute it and/or modify it. 
** The software provided hereunder is on an "as is" basis, and 
** the author has no obligation to provide maintenance, support, updates,
** enhancements, or modifications. 
*/

#include "tolua.h"
#include "tolua_tm.h"

#include <string.h>
#include <stdlib.h>

long tolua_getnumber (lua_State* L, int narg, long def)
{
 return lua_gettop(L)<abs(narg) ? def : lua_tonumber(L,narg);
}

const char* tolua_getstring (lua_State* L, int narg, const char* def)
{
 return lua_gettop(L)<abs(narg) ? def : lua_tostring(L,narg);
}

void* tolua_getuserdata (lua_State* L, int narg, void* def)
{
 return lua_gettop(L)<abs(narg) ? def : lua_touserdata(L,narg);
}

void* tolua_getusertype (lua_State* L, int narg, void* def)
{
 return lua_gettop(L)<abs(narg) ? def : lua_touserdata(L,narg);
}

int tolua_getvalue (lua_State* L, int narg, int def)
{
 return lua_gettop(L)<abs(narg) ? def : narg;
}

int tolua_getbool (lua_State* L, int narg, int def)
{
 return lua_gettop(L)<abs(narg) ? 
         def : 
         lua_isnil(L,narg) ? 0 : lua_tonumber(L,narg)!=0;
}

long tolua_getfieldnumber (lua_State* L, int lo, int index, long def)
{
 long v;
 lua_pushnumber(L,index);
 lua_gettable(L,lo);
 v = lua_isnil(L,-1) ? def : lua_tonumber(L,-1);
 lua_pop(L,1);
 return v;
}

const char* tolua_getfieldstring 
(lua_State* L, int lo, int index, const char* def)
{
 const char* v;
 lua_pushnumber(L,index);
 lua_gettable(L,lo);
 v = lua_isnil(L,-1) ? def : lua_tostring(L,-1);
 lua_pop(L,1);
 return v;
}

void* tolua_getfielduserdata (lua_State* L, int lo, int index, void* def)
{
 void* v;
 lua_pushnumber(L,index);
 lua_gettable(L,lo);
 v = lua_isnil(L,-1) ? def : lua_touserdata(L,-1);
 lua_pop(L,1);
 return v;
}

void* tolua_getfieldusertype (lua_State* L, int lo, int index, void* def)
{
 void* v;
 lua_pushnumber(L,index);
 lua_gettable(L,lo);
 v = lua_isnil(L,-1) ? def : lua_touserdata(L,-1);
 lua_pop(L,1);
 return v;
}

int tolua_getfieldvalue (lua_State* L, int lo, int index, int def)
{
 int v;
 lua_pushnumber(L,index);
 lua_gettable(L,lo);
 v = lua_isnil(L,-1) ? def : lo;
 lua_pop(L,1);
 return v;
}

int tolua_getfieldbool (lua_State* L, int lo, int index, int def)
{
 int v;
 lua_pushnumber(L,index);
 lua_gettable(L,lo);
 v = lua_isnil(L,-1) ? def : lua_tonumber(L,-1)!=0;
 lua_pop(L,1);
 return v;
}

void tolua_pushnumber (lua_State* L, long value)
{
 lua_pushnumber(L,value);
}

void tolua_pushstring (lua_State* L, const char* value)
{
 if (value == NULL)
  lua_pushnil(L);
 else
  lua_pushstring(L,value);
}

void tolua_pushuserdata (lua_State* L, void* value)
{
 if (value == NULL)
  lua_pushnil(L);
 else
  lua_pushuserdata(L,value);
}

void tolua_pushusertype (lua_State* L, void* value, int tag)
{
 if (value == NULL)
  lua_pushnil(L);
 else
  lua_pushusertag(L,value,tag);
}

void tolua_pushvalue (lua_State* L, int lo)
{
 lua_pushvalue(L,lo);
}

void tolua_pushbool (lua_State* L, int value)
{
 if (value)
  lua_pushnumber(L,(long)value);
 else
  lua_pushnil(L);
}

void tolua_pushfieldnumber (lua_State* L, int lo, int index, long v)
{
 lua_pushnumber(L,index);
 tolua_pushnumber(L,v);
 lua_settable(L,lo);
}

void tolua_pushfieldstring (lua_State* L, int lo, int index, char* v)
{
 lua_pushnumber(L,index);
 tolua_pushstring(L,v);
 lua_settable(L,lo);
}

void tolua_pushfielduserdata (lua_State* L, int lo, int index, void* v)
{
 lua_pushnumber(L,index);
 tolua_pushuserdata(L,v);
 lua_settable(L,lo);
}

void tolua_pushfieldusertype (lua_State* L, int lo, int index, void* v, int tag)
{
 lua_pushnumber(L,index);
 tolua_pushusertype(L,v,tag);
 lua_settable(L,lo);
}

void tolua_pushfieldvalue (lua_State* L, int lo, int index, int v)
{
 lua_pushnumber(L,index);
 lua_pushvalue(L,v);
 lua_settable(L,lo);
}

void tolua_pushfieldbool (lua_State* L, int lo, int index, int v)
{
 lua_pushnumber(L,index);
 tolua_pushbool(L,v);
 lua_settable(L,lo);
}

