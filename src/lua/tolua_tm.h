/* tolua: tag methods
** Support code for Lua bindings.
** Written by Waldemar Celes
** TeCGraf/PUC-Rio
** Jul 1998
** $Id: tolua_tm.h,v 1.2 2002/11/23 21:31:25 rr9 Exp $
*/

/* This code is free software; you can redistribute it and/or modify it.
** The software provided hereunder is on an "as is" basis, and
** the author has no obligation to provide maintenance, support, updates,
** enhancements, or modifications.
*/



#ifndef tolua_tm_h
#define tolua_tm_h

void toluaI_tm_init (lua_State* L);
void toluaI_tm_global (lua_State* L, int lo);
void toluaI_tm_module (lua_State* L, int lo);
void toluaI_tm_class (lua_State* L, int lo, const char* name);
void toluaI_tm_instance (lua_State* L, int tag, int lo);
void toluaI_tm_linstance (lua_State* L, int tag, int lo);
void toluaI_tm_using (lua_State* L, int module);
void toluaI_tm_setclass (lua_State* L, int lo);
void toluaI_tm_pushmate (lua_State* L, int lo);
void toluaI_tm_pushclass (lua_State* L, int lo);


#endif
