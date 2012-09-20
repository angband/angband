/* tolua: error handling
** Support code for Lua bindings.
** Written by Waldemar Celes
** TeCGraf/PUC-Rio
** Jul 1998
** $Id: tolua_eh.c,v 1.1 2001/10/27 19:35:29 angband Exp $
*/

/* This code is free software; you can redistribute it and/or modify it. 
** The software provided hereunder is on an "as is" basis, and 
** the author has no obligation to provide maintenance, support, updates,
** enhancements, or modifications. 
*/

#include "tolua.h"
#include "tolua_eh.h"
#include "tolua_rg.h"

#include <stdio.h>

/* registry fiels used to hold current error info 
   - tolua_err_narg: number of wrong argument
   - tolua_err_provided: provided type
   - tolua_err_expected: expected type
*/

void toluaI_eh_set 
(lua_State* L, int narg, const char* provided, const char* expected)
{
 lua_pushnumber(L,narg);
 toluaI_setregistry(L,"tolua_err_narg");
 lua_pushstring(L,provided);
 toluaI_setregistry(L,"tolua_err_provided");
 lua_pushstring(L,expected);
 toluaI_setregistry(L,"tolua_err_expected");
}

void tolua_error (lua_State* L, char* msg)
{
 if (msg[0]=='#')
 {
  static char buffer[BUFSIZ];
  const char* err_provided;
  const char* err_expected;
  toluaI_getregistry(L,"tolua_err_provided");
  err_provided = lua_tostring(L,-1);
  toluaI_getregistry(L,"tolua_err_expected");
  err_expected = lua_tostring(L,-1);
  lua_pop(L,2);
  if (msg[1]=='f')
  {
   int err_narg;
   toluaI_getregistry(L,"tolua_err_narg");
   err_narg = (int)lua_tonumber(L,-1);
   lua_pop(L,1);
   sprintf(buffer,"%s\n     argument #%d is '%s'; '%s' expected.\n",
           msg+2,err_narg,err_provided,err_expected);
  }
  else if (msg[1]=='v')
   sprintf(buffer,"%s\n     value is '%s'; '%s' expected.\n",
           msg+2,err_provided,err_expected);
  lua_error(L,buffer);
 }
 else
  lua_error(L,msg);
}
