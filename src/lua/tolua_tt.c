/* tolua: type & tag manipulation.
** Support code for Lua bindings.
** Written by Waldemar Celes
** TeCGraf/PUC-Rio
** Jul 1998
** $Id: tolua_tt.c,v 1.2 2002/11/23 21:31:25 rr9 Exp $
*/

/* This code is free software; you can redistribute it and/or modify it. 
** The software provided hereunder is on an "as is" basis, and 
** the author has no obligation to provide maintenance, support, updates,
** enhancements, or modifications. 
*/

#include "tolua.h"
#include "tolua_tt.h"
#include "tolua_tm.h"
#include "tolua_eh.h"
#include "tolua_rg.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

/* Global tables created in Lua registry:
   tolua_tbl_itype: indexed by instance tags, stores the instance types.
   tolua_tbl_itag: indexed by instance types, stores the instance tags.
   tolua_tbl_const: indexed by constant tags, stores the tags.
   tolua_tbl_hierarchy: indexed by instance tags, stores the base tags.
*/

/* exported basic type tags */
int tolua_tag_nil;
int tolua_tag_number;
int tolua_tag_string;
int tolua_tag_userdata;
int tolua_tag_table;
int tolua_tag_function;


static const char* gettype (lua_State* L, int tag)
{
 const char* type;
 toluaI_getregistry(L,"tolua_tbl_itype");
 lua_pushnumber(L,tag);
 lua_gettable(L,-2);
 type = lua_tostring(L,-1);
 if (type==NULL) type = "[undefined]";
 lua_pop(L,2);
 return type;
}

const char* toluaI_tt_getobjtype (lua_State* L, int lo)
{
 if (lua_gettop(L)<abs(lo))
  return "[no object]";
 else
  return gettype(L,lua_tag(L,lo));
}

int toluaI_tt_gettag (lua_State* L, const char* type)
{
 int tag;
 toluaI_getregistry(L,"tolua_tbl_itag");
 lua_pushstring(L,type);
 lua_gettable(L,-2);
 tag = (int)lua_tonumber(L,-1);
 lua_pop(L,2);
 return tag;
}

static int basetag (lua_State* L, int hierarchy, int tag)
{
 int btag;
 lua_pushnumber(L,tag);
 lua_gettable(L,hierarchy);
 btag = (int)lua_tonumber(L,-1);
 lua_pop(L,1);
 return btag;
}

static int istype (lua_State* L, int lo, int tag)
{
 int otag = lua_tag(L,lo);
 if (tag==otag)		/* check simplest case */
  return 1;
 else if (lua_isnil(L,lo) && 
          tag!=LUA_TNUMBER &&
          tag!=LUA_TTABLE &&
          tag!=LUA_TFUNCTION
         )
  return 1;
 else if ((tag==LUA_TSTRING && lua_isstring(L,lo)) || /* check convertions */
          (tag==LUA_TNUMBER && lua_isnumber(L,lo))
         )
  return 1;
 else if (tag==LUA_TUSERDATA && lua_isuserdata(L,lo)) /* pointer to void* */
  return 1;
 else if (tag==toluaI_tt_gettag(L,"bool") && otag==LUA_TNUMBER)
  return 1;
 else
 {
  /* if requested type is const, the non-const is an alternative type */
  int tag2;
  int tbl_hierarchy;
  toluaI_getregistry(L,"tolua_tbl_const");
  lua_pushnumber(L,tag);
  lua_gettable(L,-2);
  tag2 = (int)lua_tonumber(L,-1);
  lua_pop(L,2);
  if (tag2 && tag2==otag)
   return 1;
  /* check for base classes */
  toluaI_getregistry(L,"tolua_tbl_hierarchy");
  tbl_hierarchy = lua_gettop(L);
  otag = basetag(L,tbl_hierarchy,otag);  
  while (otag)
  {
   if (tag==otag || (tag2 && tag2==otag))
    break;
   otag = basetag(L,tbl_hierarchy,otag);
  }
  lua_pop(L,1);
  return otag!=0;
 }
}

void toluaI_tt_init (lua_State* L)
{
 lua_newtable(L); toluaI_setregistry(L,"tolua_tbl_itype");
 lua_newtable(L); toluaI_setregistry(L,"tolua_tbl_itag");
 lua_newtable(L); toluaI_setregistry(L,"tolua_tbl_const");
 lua_newtable(L); toluaI_setregistry(L,"tolua_tbl_hierarchy");

 /* set and register basic Lua type tag */
#if 0
 lua_pushnumber(L,LUA_TNIL); toluaI_setregistry(L,"tolua_tag_nil");
 lua_pushnumber(L,LUA_TNUMBER); toluaI_setregistry(L,"tolua_tag_number");
 lua_pushnumber(L,LUA_TSTRING); toluaI_setregistry(L,"tolua_tag_string");
 lua_pushnumber(L,LUA_TUSERDATA); toluaI_setregistry(L,"tolua_tag_userdata");
 lua_pushnumber(L,LUA_TTABLE); toluaI_setregistry(L,"tolua_tag_table");
 lua_pushnumber(L,LUA_TFUNCTION); toluaI_setregistry(L,"tolua_tag_function");
 
 toluaI_tt_register(L,toluaI_tt_gettag(L,"tolua_tag_nil"),"nil");
 toluaI_tt_register(L,toluaI_tt_gettag(L,"tolua_tag_number"),"number");
 toluaI_tt_register(L,toluaI_tt_gettag(L,"tolua_tag_string"),"string");
 toluaI_tt_register(L,toluaI_tt_gettag(L,"tolua_tag_userdata"),"userdata");
 toluaI_tt_register(L,toluaI_tt_gettag(L,"tolua_tag_table"),"table");
 toluaI_tt_register(L,toluaI_tt_gettag(L,"tolua_tag_function"),"function");
#else
 toluaI_tt_register(L,LUA_TNIL,"nil");
 toluaI_tt_register(L,LUA_TNUMBER,"number");
 toluaI_tt_register(L,LUA_TSTRING,"string");
 toluaI_tt_register(L,LUA_TUSERDATA,"userdata");
 toluaI_tt_register(L,LUA_TTABLE,"table");
 toluaI_tt_register(L,LUA_TFUNCTION,"function");
 toluaI_tt_register(L,lua_newtag(L),"bool");
#endif
}


void toluaI_tt_register (lua_State* L, int tag, const char* type)
{
 toluaI_getregistry(L,"tolua_tbl_itype");
 lua_pushnumber(L,tag);
 lua_pushstring(L,type);
 lua_settable(L,-3);

 toluaI_getregistry(L,"tolua_tbl_itag");
 lua_pushstring(L,type);
 lua_pushnumber(L,tag);
 lua_settable(L,-3);

 lua_pop(L,2);
}


void toluaI_tt_class (lua_State* L, int lo, const char* derived, const char* base)
{
 char* cderived = toluaI_tt_concat("const ",derived);
 int tag = toluaI_tt_gettag(L,derived);
 int ctag = toluaI_tt_gettag(L,cderived);
 toluaI_tm_instance(L,tag,lo);
 toluaI_tm_instance(L,ctag,lo);
 if (*base != 0)
 {
  char* cbase = toluaI_tt_concat("const ",base);
  int btag = toluaI_tt_gettag(L,base);
  int cbtag = toluaI_tt_gettag(L,cbase);
  toluaI_tt_sethierarchy(L,tag,btag);
  toluaI_tt_sethierarchy(L,ctag,cbtag);
 }
}

void toluaI_tt_sethierarchy (lua_State* L, int tag, int btag)
{
 toluaI_getregistry(L,"tolua_tbl_hierarchy");
 lua_pushnumber(L,tag);
 lua_pushnumber(L,btag);
 lua_settable(L,-3);
 lua_pop(L,1);
}

char* toluaI_tt_concat (const char* s1, const char* s2)
{
 static char s[BUFSIZ];
 assert(strlen(s1)+strlen(s2)<BUFSIZ);
 return strcat(strcpy(s,s1),s2);
}

void tolua_usertype (lua_State* L, const char* type)
{
 /* check if type is already registered */
 toluaI_getregistry(L,"tolua_tbl_itag");
 lua_pushstring(L,type);
 lua_gettable(L,-2);
 if (lua_isnil(L,-1))
 {
  char *ctype = toluaI_tt_concat("const ",type);
  int tag = lua_newtag(L);
  int ctag = lua_newtag(L);
  toluaI_tt_register(L,tag,type);
  toluaI_tt_register(L,ctag,ctype);
  /* set const table */
  toluaI_getregistry(L,"tolua_tbl_const");
  lua_pushnumber(L,ctag); 
  lua_pushnumber(L,tag); 
  lua_settable(L,-3);
  lua_pop(L,1);
 }
 lua_pop(L,2);
}

int toluaI_tt_isusertype (lua_State* L, int lo)
{
 if (lua_isuserdata(L,lo) && 
     toluaI_tt_gettag(L,"tolua_tag_userdata")!=lua_tag(L,lo)
    )
 {
  int status;
  toluaI_getregistry(L,"tolua_tbl_itype");
  lua_pushnumber(L,lua_tag(L,lo));
  lua_gettable(L,-2);
  status = !lua_isnil(L,-1);
  lua_pop(L,2);
  return status;
 }
 return 0;
}

#if 0
void tolua_settag (lua_State* L, char* type, int* tag)
{
 toluaI_getregistry(L,"tolua_tbl_itag");
 lua_pushstring(L,type);
 lua_gettable(L,-2);
 *tag = (int) lua_tonumber(L,-1);
 lua_pop(L,2);
}
#endif

int tolua_istype (lua_State* L, int narg, int tag, int def)
{
 if (lua_gettop(L)<abs(narg))
 {
  if (def==0)
  {
   toluaI_eh_set(L,narg,toluaI_tt_getobjtype(L,narg),gettype(L,tag));
   return 0;
  }
 }
 else
 {
  if (!istype(L,narg,tag))
  {
   toluaI_eh_set(L,narg,toluaI_tt_getobjtype(L,narg),gettype(L,tag));
   return 0;
  }
 }
 return 1;
}

int tolua_arrayistype (lua_State* L, int narg, int tag, int dim, int def)
{
 int i;
 for (i=0; i<dim; ++i)
 {
  int tf;
  lua_pushnumber(L,i+1);
  lua_gettable(L,narg);
  tf = lua_gettop(L);
  if (!istype(L,tf,tag) && (!def || !lua_isnil(L,tf)))
  {
   static char t1[BUFSIZ], t2[BUFSIZ];
   sprintf(t1,"array of %s",toluaI_tt_getobjtype(L,tf));
   sprintf(t2,"array of %s (dimension=%d)",gettype(L,tag),dim);
   toluaI_eh_set(L,narg,t1,t2);
   return 0;
  }
  lua_pop(L,1);
 }
 return 1;
}

int tolua_isnoobj (lua_State* L, int narg)
{
 if (lua_gettop(L)>=abs(narg))
 {
  toluaI_eh_set(L,narg,toluaI_tt_getobjtype(L,narg),
                toluaI_tt_getobjtype(L,lua_gettop(L)+1)); 
  return 0;
 }
 return 1;
}


