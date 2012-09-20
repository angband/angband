/*
** Lua binding: misc
** Generated automatically by tolua 5.0a on Sat Jun 18 20:19:18 2005.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "lua/tolua.h"

/* Exported function */
TOLUA_API int tolua_misc_open (lua_State* tolua_S);

#include "angband.h"
#include "script.h"

/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
}

/* get function: ddd */
static int tolua_get_misc_ddd(lua_State* tolua_S)
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
 if (tolua_index<0 || tolua_index>=9)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)ddd[tolua_index]);
 return 1;
}

/* get function: ddx */
static int tolua_get_misc_ddx(lua_State* tolua_S)
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
 if (tolua_index<0 || tolua_index>=10)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)ddx[tolua_index]);
 return 1;
}

/* get function: ddy */
static int tolua_get_misc_ddy(lua_State* tolua_S)
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
 if (tolua_index<0 || tolua_index>=10)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)ddy[tolua_index]);
 return 1;
}

/* get function: ddx_ddd */
static int tolua_get_misc_ddx_ddd(lua_State* tolua_S)
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
 if (tolua_index<0 || tolua_index>=9)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)ddx_ddd[tolua_index]);
 return 1;
}

/* get function: ddy_ddd */
static int tolua_get_misc_ddy_ddd(lua_State* tolua_S)
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
 if (tolua_index<0 || tolua_index>=9)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)ddy_ddd[tolua_index]);
 return 1;
}

/* function: script_do_file */
static int tolua_misc_script_do_file00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isstring(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  cptr filename = ((cptr)  tolua_tostring(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  script_do_file(filename);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'script_do_file'.",&tolua_err);
 return 0;
#endif
}

/* Open function */
TOLUA_API int tolua_misc_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,1);
 tolua_beginmodule(tolua_S,NULL);
 tolua_array(tolua_S,"ddd",tolua_get_misc_ddd,NULL);
 tolua_array(tolua_S,"ddx",tolua_get_misc_ddx,NULL);
 tolua_array(tolua_S,"ddy",tolua_get_misc_ddy,NULL);
 tolua_array(tolua_S,"ddx_ddd",tolua_get_misc_ddx_ddd,NULL);
 tolua_array(tolua_S,"ddy_ddd",tolua_get_misc_ddy_ddd,NULL);
 tolua_function(tolua_S,"script_do_file",tolua_misc_script_do_file00);
 tolua_endmodule(tolua_S);
 return 1;
}
