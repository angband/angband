/* tolua: register functions
** Support code for Lua bindings.
** Written by Waldemar Celes
** TeCGraf/PUC-Rio
** Nov 200
** $Id: tolua_rg.h,v 1.1 2001/10/27 19:35:29 angband Exp $
*/

/* This code is free software; you can redistribute it and/or modify it.
** The software provided hereunder is on an "as is" basis, and
** the author has no obligation to provide maintenance, support, updates,
** enhancements, or modifications.
*/


#ifndef tolua_rg_h
#define tolua_rg_h

void toluaI_setregistry (lua_State* L, char* field);
void toluaI_getregistry (lua_State* L, char* field);

#endif
