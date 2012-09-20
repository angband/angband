/*
** Lua binding: tolua
** Generated automatically by tolua 4.0b on Tue Nov 14 14:18:50 2000.
*/

#include "tolua.h"

/* Exported function */
int  tolua_tolua_open (lua_State* tolua_S);
void tolua_tolua_close (lua_State* tolua_S);

#define tolua_using(module)       (tolua_using)(tolua_S,module)
#define tolua_type(lo)            (tolua_type)(tolua_S,lo)
#define tolua_foreach(lo,f)       (tolua_foreach)(tolua_S,lo,f)
#define tolua_class(derived,base) (tolua_class)(tolua_S,derived,base)
#define tolua_instance(inst,cobj) (tolua_instance)(tolua_S,inst,cobj)
#define tolua_base(lo)            (tolua_base)(tolua_S,lo)
#define tolua_cast(lo,type)       (tolua_cast)(tolua_S,lo,type)
#define tolua_takeownership(lo)   (tolua_takeownership)(tolua_S,lo)

/* function to register type */
static void toluaI_reg_types (lua_State* tolua_S)
{
}

/* function: tolua_using */
static int toluaI_tolua_tolua_using00(lua_State* tolua_S)
{
 if (
     !tolua_isnoobj(tolua_S,2)
 )
  goto tolua_lerror;
 else
 {
  LUA_VALUE module = ((LUA_VALUE)  tolua_getvalue(tolua_S,1,0));
  {
   tolua_using(module);
  }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'using'.");
 return 0;
}

/* function: tolua_type */
static int toluaI_tolua_tolua_type00(lua_State* tolua_S)
{
 if (
     !tolua_isnoobj(tolua_S,2)
 )
  goto tolua_lerror;
 else
 {
  LUA_VALUE lo = ((LUA_VALUE)  tolua_getvalue(tolua_S,1,0));
  {
   char* toluaI_ret = (char*)  tolua_type(lo);
   tolua_pushstring(tolua_S,toluaI_ret);
  }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'type'.");
 return 0;
}

/* function: tolua_foreach */
static int toluaI_tolua_tolua_foreach00(lua_State* tolua_S)
{
 if (
     !tolua_isnoobj(tolua_S,3)
 )
  goto tolua_lerror;
 else
 {
  LUA_VALUE lo = ((LUA_VALUE)  tolua_getvalue(tolua_S,1,0));
  LUA_VALUE f = ((LUA_VALUE)  tolua_getvalue(tolua_S,2,0));
  {
   tolua_foreach(lo,f);
  }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'foreach'.");
 return 0;
}

/* function: tolua_class */
static int toluaI_tolua_tolua_class00(lua_State* tolua_S)
{
 if (
     !tolua_isnoobj(tolua_S,3)
 )
  goto tolua_lerror;
 else
 {
  LUA_VALUE derived = ((LUA_VALUE)  tolua_getvalue(tolua_S,1,0));
  LUA_VALUE base = ((LUA_VALUE)  tolua_getvalue(tolua_S,2,0));
  {
   tolua_class(derived,base);
  }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'class'.");
 return 0;
}

/* function: tolua_instance */
static int toluaI_tolua_tolua_instance00(lua_State* tolua_S)
{
 if (
     !tolua_isnoobj(tolua_S,3)
 )
  goto tolua_lerror;
 else
 {
  LUA_VALUE instance = ((LUA_VALUE)  tolua_getvalue(tolua_S,1,0));
  LUA_VALUE classobj = ((LUA_VALUE)  tolua_getvalue(tolua_S,2,0));
  {
   tolua_instance(instance,classobj);
  }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'instance'.");
 return 0;
}

/* function: tolua_base */
static int toluaI_tolua_tolua_base00(lua_State* tolua_S)
{
 if (
     !tolua_isnoobj(tolua_S,2)
 )
  goto tolua_lerror;
 else
 {
  LUA_VALUE lo = ((LUA_VALUE)  tolua_getvalue(tolua_S,1,0));
  {
   LUA_VALUE toluaI_ret = (LUA_VALUE)  tolua_base(lo);
   tolua_pushvalue(tolua_S,toluaI_ret);
  }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'base'.");
 return 0;
}

/* function: tolua_cast */
static int toluaI_tolua_tolua_cast00(lua_State* tolua_S)
{
 if (
     !tolua_istype(tolua_S,2,LUA_TSTRING,0) ||
     !tolua_isnoobj(tolua_S,3)
 )
  goto tolua_lerror;
 else
 {
  LUA_VALUE lo = ((LUA_VALUE)  tolua_getvalue(tolua_S,1,0));
  char* type = ((char*)  tolua_getstring(tolua_S,2,0));
  {
   LUA_VALUE toluaI_ret = (LUA_VALUE)  tolua_cast(lo,type);
   tolua_pushvalue(tolua_S,toluaI_ret);
  }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'cast'.");
 return 0;
}

/* function: tolua_takeownership */
static int toluaI_tolua_tolua_takeownership00(lua_State* tolua_S)
{
 if (
     !tolua_isnoobj(tolua_S,2)
 )
  goto tolua_lerror;
 else
 {
  LUA_VALUE lo = ((LUA_VALUE)  tolua_getvalue(tolua_S,1,0));
  {
   tolua_takeownership(lo);
  }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'takeownership'.");
 return 0;
}

/* Open function */
int tolua_tolua_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 toluaI_reg_types(tolua_S);
 tolua_module(tolua_S,"tolua");
 tolua_function(tolua_S,"tolua","using",toluaI_tolua_tolua_using00);
 tolua_function(tolua_S,"tolua","type",toluaI_tolua_tolua_type00);
 tolua_function(tolua_S,"tolua","foreach",toluaI_tolua_tolua_foreach00);
 tolua_function(tolua_S,"tolua","class",toluaI_tolua_tolua_class00);
 tolua_function(tolua_S,"tolua","instance",toluaI_tolua_tolua_instance00);
 tolua_function(tolua_S,"tolua","base",toluaI_tolua_tolua_base00);
 tolua_function(tolua_S,"tolua","cast",toluaI_tolua_tolua_cast00);
 tolua_function(tolua_S,"tolua","takeownership",toluaI_tolua_tolua_takeownership00);
 return 1;
}
/* Close function */
void tolua_tolua_close (lua_State* tolua_S)
{
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"tolua");
}
