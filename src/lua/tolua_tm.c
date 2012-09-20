/* tolua: tag methods
** Support code for Lua bindings.
** Written by Waldemar Celes
** TeCGraf/PUC-Rio
** Jul 1998
** $Id: tolua_tm.c,v 1.2 2002/11/23 21:31:25 rr9 Exp $
*/

/* This code is free software; you can redistribute it and/or modify it. 
** The software provided hereunder is on an "as is" basis, and 
** the author has no obligation to provide maintenance, support, updates,
** enhancements, or modifications. 
*/

#include "tolua.h"
#include "tolua_tm.h"
#include "tolua_tt.h"
#include "tolua_rg.h"

#include <string.h>



/* Global tables created in Lua registry:
    tolua_tbl_class: indexed by instance tags, stores the class tables.
    tolua_tbl_clone: indexed by memory address, stores the tag indicanting 
                     it is a clone.
    tolua_tbl_mate:  indexed by memory address, stores the associate instance
                     table.

   General tags stored in Lua registry:
    tolua_tag_global;
    tolua_tag_module;
    tolua_tag_class;
    tolua_tag_instance;
    tolua_tag_linstance;
    tolua_tag_indirect;
*/

/* internal function prototype */
static void setmethods (lua_State* L);

static void settag (lua_State* L, int lo, const char* tag_registry_field)
{
 toluaI_getregistry(L,tag_registry_field);
 lua_pushvalue(L,lo);
 lua_settag(L,(int)lua_tonumber(L,-2));
 lua_pop(L,2);
}

void toluaI_tm_global (lua_State* L, int lo)
{
 settag(L,lo,"tolua_tag_global");
}

void toluaI_tm_module (lua_State* L, int lo)
{
 settag(L,lo,"tolua_tag_module");
}

void toluaI_tm_setclass (lua_State* L, int lo)
{
 settag(L,lo,"tolua_tag_class");
}

void toluaI_tm_class (lua_State* L, int lo, const char* name)
{
 int tag_class;
 int tag = lua_newtag(L);
 char* type = toluaI_tt_concat("class ",name);
 toluaI_getregistry(L,"tolua_tag_class");
 tag_class = (int)lua_tonumber(L,-1);
 lua_copytagmethods(L,tag,tag_class);
 toluaI_tt_register(L,tag,type);
 toluaI_tt_sethierarchy(L,tag,tag_class);
 lua_pushvalue(L,lo);
 lua_settag(L,tag);
 lua_pop(L,2);  /* tag_class and lo */
}

void toluaI_tm_instance (lua_State* L, int tag, int lo)
{
 toluaI_getregistry(L,"tolua_tbl_class");
 lua_pushnumber(L,tag);
 lua_pushvalue(L,lo);
 lua_settable(L,-3);
 toluaI_getregistry(L,"tolua_tag_instance");
 lua_copytagmethods(L,tag,(int)lua_tonumber(L,-1));
 lua_pop(L,2); /* tbl_class and tag_instance */
}

void toluaI_tm_linstance (lua_State* L, int tag, int lo)
{
 toluaI_getregistry(L,"tolua_tbl_class");
 lua_pushnumber(L,tag);
 lua_pushvalue(L,lo);
 lua_settable(L,-3);
 toluaI_getregistry(L,"tolua_tag_linstance");
 lua_copytagmethods(L,tag,(int)lua_tonumber(L,-1));
 lua_pop(L,2); /* tbl_class and tag_linstance */
}

void* tolua_doclone (lua_State* L, void* value, int tag)
{
 toluaI_getregistry(L,"tolua_tbl_clone");
 lua_pushuserdata(L,value);
 lua_pushnumber(L,tag);
 lua_settable(L,-3);
 lua_pop(L,1);
 return value;
}

void* tolua_copy (lua_State* L, void* value, unsigned int size)
{
 void* clone = (void*)malloc(size);
 if (clone)
  memcpy(clone,value,size);
 else
  tolua_error(L,"insuficient memory");
 return clone;
}

static void toluaI_tm_undoclone (lua_State* L, int tag, void* clone)
{
 toluaI_getregistry(L,"tolua_tbl_clone");
 lua_pushuserdata(L,clone);
 lua_gettable(L,-2);
 if (lua_isnumber(L,-1) && lua_tonumber(L,-1)==tag)
 {
  lua_pushuserdata(L,clone);
  lua_pushnil(L);
  lua_settable(L,-4);

  /* get base class */
  toluaI_getregistry(L,"tolua_tbl_class");
  lua_pushnumber(L,tag);
  lua_rawget(L,-2);

  /* look for destructor */
  lua_pushstring(L,"delete");
  lua_gettable(L,-2);
  if (lua_iscfunction(L,-1))
  {
   lua_pushusertag(L,clone,tag);
   lua_call(L,1,0);
  }
  else
  {
   free(clone);    /* no destructor: use raw free */
   lua_pop(L,1);   /* the nil function value */
  }
  lua_pop(L,2); /* tbl_class and class method table */
 }
 lua_pop(L,2);  /* table and value */
}

void toluaI_tm_pushmate (lua_State* L, int lo)
{
 toluaI_getregistry(L,"tolua_tbl_mate");
 lua_pushvalue(L,lo);
 lua_rawget(L,-2);
 lua_insert(L,-2);
 lua_pop(L,1);
}

void toluaI_tm_pushclass (lua_State* L, int lo)
{
 toluaI_getregistry(L,"tolua_tbl_class");
 lua_pushnumber(L,lua_tag(L,lo));
 lua_rawget(L,-2);
 lua_insert(L,-2);
 lua_pop(L,1);
}

static int toluaI_gettag (lua_State* L, const char* tagname)
{
 int tag;
 toluaI_getregistry(L,tagname);
 tag = (int)lua_tonumber(L,-1);
 lua_pop(L,1);
 return tag;
}

void toluaI_tm_init (lua_State* L)
{
 lua_newtable(L); toluaI_setregistry(L,"tolua_tbl_class");
 lua_newtable(L); toluaI_setregistry(L,"tolua_tbl_clone");
 lua_newtable(L); toluaI_setregistry(L,"tolua_tbl_mate");

 lua_pushnumber(L,lua_newtag(L)); toluaI_setregistry(L,"tolua_tag_global");
 lua_pushnumber(L,lua_newtag(L)); toluaI_setregistry(L,"tolua_tag_module");
 lua_pushnumber(L,lua_newtag(L)); toluaI_setregistry(L,"tolua_tag_class");
 lua_pushnumber(L,lua_newtag(L)); toluaI_setregistry(L,"tolua_tag_instance");
 lua_pushnumber(L,lua_newtag(L)); toluaI_setregistry(L,"tolua_tag_linstance");
 lua_pushnumber(L,lua_newtag(L)); toluaI_setregistry(L,"tolua_tag_indirect");

 toluaI_tt_register(L,toluaI_gettag(L,"tolua_tag_global"),"generic variable");
 toluaI_tt_register(L,toluaI_gettag(L,"tolua_tag_module"),"generic module");
 toluaI_tt_register(L,toluaI_gettag(L,"tolua_tag_class"),"generic class");
 toluaI_tt_register(L,toluaI_gettag(L,"tolua_tag_indirect"),"generic indirect");
 toluaI_tt_register(L,toluaI_gettag(L,"tolua_tag_instance"),"generic instance");
 toluaI_tt_register(L,toluaI_gettag(L,"tolua_tag_linstance"),"generic lua instance");

 /* allows modules and classes to be used as ordinary tables */
 toluaI_tt_sethierarchy(L,toluaI_gettag(L,"tolua_tag_module"),tolua_tag_table);
 toluaI_tt_sethierarchy(L,toluaI_gettag(L,"tolua_tag_class"),tolua_tag_table);

 setmethods(L);
}

static int map (lua_State* L)
{
 int m = lua_gettop(L);
 /* do not pass string fields starting with a dot */
 if (!lua_isstring(L,1) || *lua_tostring(L,1)!='.')
 {
  lua_getglobals(L);
  lua_pushvalue(L,1);
  lua_pushvalue(L,m);
  lua_rawset(L,-3);
  lua_pop(L,1);
 }
 return 0;
}

void toluaI_tm_using (lua_State* L, int module)
{
 lua_newtable(L);
 lua_settag(L,toluaI_gettag(L,"tolua_tag_indirect"));
 lua_pushstring(L,".module");
 lua_pushvalue(L,module);
 lua_settable(L,-3);

 lua_getglobal(L,"foreach");
 lua_pushvalue(L,module);
 lua_pushvalue(L,-3);
 lua_pushcclosure(L,map,1);
 lua_call(L,2,0);

 lua_getglobal(L,"foreach");
 lua_pushvalue(L,module);
 lua_pushstring(L,".get");
 lua_gettable(L,-2);
 lua_insert(L,-2);
 lua_pop(L,1);   /* module table */
 lua_pushvalue(L,-3);
 lua_pushcclosure(L,map,1);
 lua_call(L,2,0);
 lua_pop(L,1);   /* indirect table */
}

/********************************************************** tag methods */

/* tag methods coded in C */

/* generic gettable */
static void oo_gettable (lua_State* L, int table, int base, int index)
{
 while (lua_istable(L,base))
 {
  lua_pushvalue(L,index); 
  lua_rawget(L,base);
  if (!lua_isnil(L,-1))
   return;            /* returned value already on the top */
  else if (lua_isnumber(L,index))
  {
   lua_pushstring(L,"operator_get");
   lua_rawget(L,base);
   if (!lua_isnil(L,-1))
   {
    lua_pushvalue(L,table);
    lua_pushvalue(L,index);
    lua_call(L,2,1);
    return;
   }
  }
  else
  {
   lua_pushstring(L,".get");
   lua_rawget(L,base);
   if (!lua_isnil(L,-1))
   {
    lua_pushvalue(L,index);
    lua_rawget(L,-2);
    if (!lua_isnil(L,-1))
    {
     lua_pushvalue(L,table);
     lua_pushvalue(L,index); /* need to access array field (?) */
     lua_call(L,2,1);
     return;
    }
   }
  }
  lua_pushstring(L,".base"); lua_rawget(L,base);
  base = lua_gettop(L);
 }
 lua_pushnil(L);
}

/* generic settable */
static int oo_settable (lua_State* L, int table, int base, int index, int value) 
{
 while (lua_istable(L,base))
 {
  lua_pushstring(L,".set");
  lua_rawget(L,base);
  if (!lua_isnil(L,-1))
  {
   lua_pushvalue(L,index);
   lua_rawget(L,-2);
   if (!lua_isnil(L,-1))
   {
    lua_pushvalue(L,table);
    lua_pushvalue(L,value);
    lua_call(L,2,0);
    return 1;
   }
  }
  lua_pushstring(L,".base"); lua_rawget(L,base);
  base = lua_gettop(L);
 }
 return 0;
}

/* class tag methods */
static int class_index (lua_State* L)
{
 int table = 1;
 int index = 2;
 oo_gettable(L,table,table,index);
 return 1;
}
static int class_settable (lua_State* L)
{
 int table = 1;
 int index = 2;
 int value = 3;
 if (oo_settable(L,table,table,index,value) == 0)
 {
  lua_pushvalue(L,table);
  lua_pushvalue(L,index);
  lua_pushvalue(L,value);
  lua_rawset(L,-3);
 }
 return 0;
}

/* instance tags */
static int instance_gettable (lua_State* L)
{
 int table = 1;
 int index = 2;
 toluaI_tm_pushmate(L,table);   /* pushes mate */
 if (!lua_isnil(L,-1))          /* if there's a mate table */
 {
  lua_pushvalue(L,index);
  lua_rawget(L,-2);
  if (!lua_isnil(L,-1))         /* if field in mate table exists */
   return 1;
 }
 toluaI_tm_pushclass(L,table);  /* pushes base */
 oo_gettable(L,table,lua_gettop(L),index);
 return 1;
}
static int instance_settable (lua_State* L)
{
 int table = 1;
 int index = 2;
 int value = 3;
 toluaI_tm_pushclass(L,table);  /* pushes base */
 if (lua_isnumber(L,index))
 {
  lua_pushstring(L,"operator_set");
  lua_rawget(L,-2);
  if (!lua_isnil(L,-1))
  {/* the stack here is: table,index,value,base,operator */
   /* call operator passing table, index, and value */
   lua_insert(L,1);
   lua_pop(L,1);       /* base */
   lua_call(L,3,0);
   return 0;
  }
 }
 if (oo_settable(L,table,4,index,value) == 0)
 {
  toluaI_tm_pushmate(L,table);    /* pushes mate */
  if (lua_isnil(L,-1))
  {
   /* creates mate table */
   lua_newtable(L);
   toluaI_getregistry(L,"tolua_tbl_mate");
   lua_pushvalue(L,table);        /* that is the userdata */
   lua_pushvalue(L,-3);
   lua_rawset(L,-3);
   lua_pop(L,1);     /* tbl_mate */
  }
  /* the mate table is on the top */
  lua_pushvalue(L,index);
  lua_pushvalue(L,value);
  lua_rawset(L,-3);
 }
 return 0;
}
static int instance_gc (lua_State* L)
{
 toluaI_tm_undoclone(L,lua_tag(L,1),lua_touserdata(L,1)); 
 return 0;
}
static int gen_operator (lua_State* L)
{
 int op1 = 1;
 int op2 = 2;
 int event = 3;
 char* name = toluaI_tt_concat("operator_",lua_tostring(L,event));
 lua_pushstring(L,name); 
 lua_gettable(L,op1);
 lua_pushvalue(L,op1);
 lua_pushvalue(L,op2);
 lua_call(L,2,1);
 return 1;
}
static int instance_operator (lua_State* L)
{
 return gen_operator(L);
}
static int instance_relational (lua_State* L)
{
 gen_operator(L);
 if ((int)lua_tonumber(L,-1)==0) lua_pushnil(L);
 return 1;
}

/* lua instance tags */
static int linstance_index (lua_State* L)
{
 toluaI_tm_pushclass(L,1);
 oo_gettable(L,1,3,2);  /* table,base,index */
 return 1;
}


/* module tag methods */
static int module_index (lua_State* L)
{
 int table = 1;
 int index = 2;
 lua_pushstring(L,".get");
 lua_rawget(L,table);
 if (!lua_isnil(L,-1))
 {
  lua_pushvalue(L,index);
  lua_rawget(L,-2);
  if (!lua_isnil(L,-1))
  {
   lua_call(L,0,1);
   return 1;
  }
 }
 lua_pushnil(L);
 return 1;
}
static int module_settable (lua_State* L)
{
 int table = 1;
 int index = 2;
 int value = 3;
 lua_pushstring(L,".set");
 lua_rawget(L,table);
 if (!lua_isnil(L,-1))
 {
  lua_pushvalue(L,index);
  lua_rawget(L,-2);
  if (!lua_isnil(L,-1))
  {
   lua_pushvalue(L,value);
   lua_call(L,1,0);
   return 0;
  }
 }
 lua_pushvalue(L,index);
 lua_pushvalue(L,value);
 lua_rawset(L,table);
 return 0;
}

/* global variable tag methods */
static int global_getglobal (lua_State* L)
{
 int value = 2;
 lua_pushstring(L,".get");
 lua_rawget(L,value);
 lua_call(L,0,1);
 return 1;
}
static int global_setglobal (lua_State* L)
{
 int value = 2;
 int newvalue = 3;
 lua_pushstring(L,".set");
 lua_rawget(L,value);
 if (lua_isnil(L,-1))
  lua_error(L,"value of const variable cannot be changed");
 else
 {
  lua_pushvalue(L,newvalue);
  lua_call(L,1,0);
 }
 return 0;
}

/* indirect tag methods */
static int indirect_getglobal (lua_State* L)
{
 int varname = 1;
 int value = 2;
 lua_pushstring(L,".module");
 lua_gettable(L,value);
 lua_pushvalue(L,varname);
 lua_gettable(L,-2);
 return 1;
}
static int indirect_setglobal (lua_State* L)
{
 int varname = 1;
 int value = 2;
 int newvalue = 3;
 lua_pushstring(L,".module");
 lua_gettable(L,value);
 lua_pushvalue(L,varname);
 lua_pushvalue(L,newvalue);
 lua_settable(L,-3);
 return 0;
}

static void setmethods (lua_State* L)
{
 /* global variable */
 lua_pushcfunction(L,global_getglobal);
 lua_settagmethod(L,toluaI_gettag(L,"tolua_tag_global"),"getglobal");
 lua_pushcfunction(L,global_setglobal);
 lua_settagmethod(L,toluaI_gettag(L,"tolua_tag_global"),"setglobal");

 /* module */
 lua_pushcfunction(L,module_index);
 lua_settagmethod(L,toluaI_gettag(L,"tolua_tag_module"),"index");
 lua_pushcfunction(L,module_settable);
 lua_settagmethod(L,toluaI_gettag(L,"tolua_tag_module"),"settable");

 /* class */
 lua_pushcfunction(L,class_index);
 lua_settagmethod(L,toluaI_gettag(L,"tolua_tag_class"),"index");
 lua_pushcfunction(L,class_settable);
 lua_settagmethod(L,toluaI_gettag(L,"tolua_tag_class"),"settable");

 /* instance */
 lua_pushcfunction(L,instance_gettable);
 lua_settagmethod(L,toluaI_gettag(L,"tolua_tag_instance"),"gettable");
 lua_pushcfunction(L,instance_settable);
 lua_settagmethod(L,toluaI_gettag(L,"tolua_tag_instance"),"settable");
 lua_pushcfunction(L,instance_operator);
 lua_settagmethod(L,toluaI_gettag(L,"tolua_tag_instance"),"add");
 lua_pushcfunction(L,instance_operator);
 lua_settagmethod(L,toluaI_gettag(L,"tolua_tag_instance"),"sub");
 lua_pushcfunction(L,instance_operator);
 lua_settagmethod(L,toluaI_gettag(L,"tolua_tag_instance"),"mul");
 lua_pushcfunction(L,instance_operator);
 lua_settagmethod(L,toluaI_gettag(L,"tolua_tag_instance"),"div");
 lua_pushcfunction(L,instance_relational);
 lua_settagmethod(L,toluaI_gettag(L,"tolua_tag_instance"),"lt");
 lua_pushcfunction(L,instance_gc);
 lua_settagmethod(L,toluaI_gettag(L,"tolua_tag_instance"),"gc");

 /* lua instance */
 lua_pushcfunction(L,linstance_index);
 lua_settagmethod(L,toluaI_gettag(L,"tolua_tag_linstance"),"index");

 /* indirect */
 lua_pushcfunction(L,indirect_getglobal);
 lua_settagmethod(L,toluaI_gettag(L,"tolua_tag_indirect"),"getglobal");
 lua_pushcfunction(L,indirect_setglobal);
 lua_settagmethod(L,toluaI_gettag(L,"tolua_tag_indirect"),"setglobal");
}



