/* tolua: type & tag manipulation.
** Support code for Lua bindings.
** Written by Waldemar Celes
** TeCGraf/PUC-Rio
** Jul 1998
** $Id: tolua_tt.h,v 1.2 2002/11/23 21:31:25 rr9 Exp $
*/

/* This code is free software; you can redistribute it and/or modify it.
** The software provided hereunder is on an "as is" basis, and
** the author has no obligation to provide maintenance, support, updates,
** enhancements, or modifications.
*/


#ifndef tolua_tt_h
#define tolua_tt_h

void  toluaI_tt_init (lua_State* L);
void  toluaI_tt_register (lua_State* L, int tag, const char* type);
void  toluaI_tt_class (lua_State* L, int lo, const char* derived, const char* base);
void  toluaI_tt_sethierarchy (lua_State* L, int tag, int btag);
int   toluaI_tt_isusertype (lua_State* L, int lo);
int   toluaI_tt_gettag (lua_State* L, const char* type);
const char* toluaI_tt_getobjtype (lua_State* L, int lo);
char* toluaI_tt_concat (const char* s1, const char* s2);




#endif
