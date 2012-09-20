/*
** Lua binding: random
** Generated automatically by tolua 5.0a on Sat Jun 18 20:19:17 2005.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "lua/tolua.h"

/* Exported function */
TOLUA_API int tolua_random_open (lua_State* tolua_S);

#include "angband.h"

/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
}

/* get function: Rand_quick */
static int tolua_get_Rand_quick(lua_State* tolua_S)
{
 tolua_pushboolean(tolua_S,(bool)Rand_quick);
 return 1;
}

/* set function: Rand_quick */
static int tolua_set_Rand_quick(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  Rand_quick = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* function: rand_int */
static int tolua_random_rand_int00(lua_State* tolua_S)
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
  u32b m = ((u32b)  tolua_tonumber(tolua_S,1,0));
 {
  s32b tolua_ret = (s32b)  rand_int(m);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'rand_int'.",&tolua_err);
 return 0;
#endif
}

/* function: randint */
static int tolua_random_randint00(lua_State* tolua_S)
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
  u32b m = ((u32b)  tolua_tonumber(tolua_S,1,0));
 {
  s32b tolua_ret = (s32b)  randint(m);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'randint'.",&tolua_err);
 return 0;
#endif
}

/* function: rand_die */
static int tolua_random_rand_die00(lua_State* tolua_S)
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
  u32b m = ((u32b)  tolua_tonumber(tolua_S,1,0));
 {
  s32b tolua_ret = (s32b)  rand_die(m);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'rand_die'.",&tolua_err);
 return 0;
#endif
}

/* function: rand_range */
static int tolua_random_rand_range00(lua_State* tolua_S)
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
  u32b A = ((u32b)  tolua_tonumber(tolua_S,1,0));
  u32b B = ((u32b)  tolua_tonumber(tolua_S,2,0));
 {
  s32b tolua_ret = (s32b)  rand_range(A,B);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'rand_range'.",&tolua_err);
 return 0;
#endif
}

/* function: rand_spread */
static int tolua_random_rand_spread00(lua_State* tolua_S)
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
  u32b A = ((u32b)  tolua_tonumber(tolua_S,1,0));
  u32b D = ((u32b)  tolua_tonumber(tolua_S,2,0));
 {
  s32b tolua_ret = (s32b)  rand_spread(A,D);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'rand_spread'.",&tolua_err);
 return 0;
#endif
}

/* function: damroll */
static int tolua_random_damroll00(lua_State* tolua_S)
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
  int num = ((int)  tolua_tonumber(tolua_S,1,0));
  int sides = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  int tolua_ret = (int)  damroll(num,sides);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'damroll'.",&tolua_err);
 return 0;
#endif
}

/* function: maxroll */
static int tolua_random_maxroll00(lua_State* tolua_S)
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
  int num = ((int)  tolua_tonumber(tolua_S,1,0));
  int sides = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  int tolua_ret = (int)  maxroll(num,sides);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'maxroll'.",&tolua_err);
 return 0;
#endif
}

/* function: Rand_state_init */
static int tolua_random_Rand_state_init00(lua_State* tolua_S)
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
  u32b seed = ((u32b)  tolua_tonumber(tolua_S,1,0));
 {
  Rand_state_init(seed);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Rand_state_init'.",&tolua_err);
 return 0;
#endif
}

/* function: Rand_div */
static int tolua_random_Rand_div00(lua_State* tolua_S)
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
  u32b m = ((u32b)  tolua_tonumber(tolua_S,1,0));
 {
  u32b tolua_ret = (u32b)  Rand_div(m);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Rand_div'.",&tolua_err);
 return 0;
#endif
}

/* function: Rand_normal */
static int tolua_random_Rand_normal00(lua_State* tolua_S)
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
  int mean = ((int)  tolua_tonumber(tolua_S,1,0));
  int stand = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  s16b tolua_ret = (s16b)  Rand_normal(mean,stand);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Rand_normal'.",&tolua_err);
 return 0;
#endif
}

/* function: Rand_simple */
static int tolua_random_Rand_simple00(lua_State* tolua_S)
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
  u32b m = ((u32b)  tolua_tonumber(tolua_S,1,0));
 {
  u32b tolua_ret = (u32b)  Rand_simple(m);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Rand_simple'.",&tolua_err);
 return 0;
#endif
}

/* Open function */
TOLUA_API int tolua_random_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,1);
 tolua_beginmodule(tolua_S,NULL);
 tolua_variable(tolua_S,"Rand_quick",tolua_get_Rand_quick,tolua_set_Rand_quick);
 tolua_function(tolua_S,"rand_int",tolua_random_rand_int00);
 tolua_function(tolua_S,"randint",tolua_random_randint00);
 tolua_function(tolua_S,"rand_die",tolua_random_rand_die00);
 tolua_function(tolua_S,"rand_range",tolua_random_rand_range00);
 tolua_function(tolua_S,"rand_spread",tolua_random_rand_spread00);
 tolua_function(tolua_S,"damroll",tolua_random_damroll00);
 tolua_function(tolua_S,"maxroll",tolua_random_maxroll00);
 tolua_function(tolua_S,"Rand_state_init",tolua_random_Rand_state_init00);
 tolua_function(tolua_S,"Rand_div",tolua_random_Rand_div00);
 tolua_function(tolua_S,"Rand_normal",tolua_random_Rand_normal00);
 tolua_function(tolua_S,"Rand_simple",tolua_random_Rand_simple00);
 tolua_endmodule(tolua_S);
 return 1;
}
