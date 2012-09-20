/*
** Lua binding: monster
** Generated automatically by tolua 5.0a on Sun May 23 19:11:24 2004.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "lua/tolua.h"

/* Exported function */
TOLUA_API int tolua_monster_open (lua_State* tolua_S);

#include "angband.h"
static int tolua_monster_desc(lua_State *L)
{
	char buf[1024];
	int mode;
	monster_type *m_ptr;

	/* Get the arguments */
	m_ptr = tolua_tousertype(L, 1, 0);

	if (!m_ptr)
	{
		tolua_error(L, "#ferror in function 'monster_desc'.", NULL);
		return 0;
	}

	mode = (int)luaL_check_number(L, 2);

	/* Get the description */
	monster_desc(buf, sizeof(buf), m_ptr, mode);

	/* Return the name */
	tolua_pushstring(L, buf);

	/* One result */
	return 1;
}

/* function to release collected object via destructor */
#ifdef __cplusplus

static int tolua_collect_monster_blow (lua_State* tolua_S)
{
 monster_blow* self = (monster_blow*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_monster_type (lua_State* tolua_S)
{
 monster_type* self = (monster_type*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}
#endif


/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"monster_lore");
 tolua_usertype(tolua_S,"monster_blow");
 tolua_usertype(tolua_S,"monster_type");
 tolua_usertype(tolua_S,"monster_race");
 tolua_usertype(tolua_S,"object_type");
}

/* get function: method of class  monster_blow */
static int tolua_get_monster_blow_method(lua_State* tolua_S)
{
  monster_blow* self = (monster_blow*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'method'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->method);
 return 1;
}

/* set function: method of class  monster_blow */
static int tolua_set_monster_blow_method(lua_State* tolua_S)
{
  monster_blow* self = (monster_blow*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'method'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->method = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: effect of class  monster_blow */
static int tolua_get_monster_blow_effect(lua_State* tolua_S)
{
  monster_blow* self = (monster_blow*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'effect'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->effect);
 return 1;
}

/* set function: effect of class  monster_blow */
static int tolua_set_monster_blow_effect(lua_State* tolua_S)
{
  monster_blow* self = (monster_blow*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'effect'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->effect = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: d_dice of class  monster_blow */
static int tolua_get_monster_blow_d_dice(lua_State* tolua_S)
{
  monster_blow* self = (monster_blow*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'd_dice'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->d_dice);
 return 1;
}

/* set function: d_dice of class  monster_blow */
static int tolua_set_monster_blow_d_dice(lua_State* tolua_S)
{
  monster_blow* self = (monster_blow*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'd_dice'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->d_dice = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: d_side of class  monster_blow */
static int tolua_get_monster_blow_d_side(lua_State* tolua_S)
{
  monster_blow* self = (monster_blow*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'd_side'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->d_side);
 return 1;
}

/* set function: d_side of class  monster_blow */
static int tolua_set_monster_blow_d_side(lua_State* tolua_S)
{
  monster_blow* self = (monster_blow*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'd_side'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->d_side = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: name of class  monster_race */
static int tolua_get_monster_race_name(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'name'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->name);
 return 1;
}

/* set function: name of class  monster_race */
static int tolua_set_monster_race_name(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'name'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->name = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: text of class  monster_race */
static int tolua_get_monster_race_text(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'text'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->text);
 return 1;
}

/* set function: text of class  monster_race */
static int tolua_set_monster_race_text(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'text'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->text = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: hdice of class  monster_race */
static int tolua_get_monster_race_hdice(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'hdice'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->hdice);
 return 1;
}

/* set function: hdice of class  monster_race */
static int tolua_set_monster_race_hdice(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'hdice'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->hdice = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: hside of class  monster_race */
static int tolua_get_monster_race_hside(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'hside'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->hside);
 return 1;
}

/* set function: hside of class  monster_race */
static int tolua_set_monster_race_hside(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'hside'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->hside = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: ac of class  monster_race */
static int tolua_get_monster_race_ac(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ac'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->ac);
 return 1;
}

/* set function: ac of class  monster_race */
static int tolua_set_monster_race_ac(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ac'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ac = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: sleep of class  monster_race */
static int tolua_get_monster_race_sleep(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sleep'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->sleep);
 return 1;
}

/* set function: sleep of class  monster_race */
static int tolua_set_monster_race_sleep(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sleep'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->sleep = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: aaf of class  monster_race */
static int tolua_get_monster_race_aaf(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'aaf'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->aaf);
 return 1;
}

/* set function: aaf of class  monster_race */
static int tolua_set_monster_race_aaf(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'aaf'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->aaf = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: speed of class  monster_race */
static int tolua_get_monster_race_speed(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'speed'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->speed);
 return 1;
}

/* set function: speed of class  monster_race */
static int tolua_set_monster_race_speed(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'speed'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->speed = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: mexp of class  monster_race */
static int tolua_get_monster_race_mexp(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'mexp'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->mexp);
 return 1;
}

/* set function: mexp of class  monster_race */
static int tolua_set_monster_race_mexp(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'mexp'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->mexp = ((s32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: freq_innate of class  monster_race */
static int tolua_get_monster_race_freq_innate(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'freq_innate'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->freq_innate);
 return 1;
}

/* set function: freq_innate of class  monster_race */
static int tolua_set_monster_race_freq_innate(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'freq_innate'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->freq_innate = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: freq_spell of class  monster_race */
static int tolua_get_monster_race_freq_spell(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'freq_spell'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->freq_spell);
 return 1;
}

/* set function: freq_spell of class  monster_race */
static int tolua_set_monster_race_freq_spell(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'freq_spell'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->freq_spell = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flags1 of class  monster_race */
static int tolua_get_monster_race_flags1(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags1'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags1);
 return 1;
}

/* set function: flags1 of class  monster_race */
static int tolua_set_monster_race_flags1(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags1'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags1 = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flags2 of class  monster_race */
static int tolua_get_monster_race_flags2(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags2'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags2);
 return 1;
}

/* set function: flags2 of class  monster_race */
static int tolua_set_monster_race_flags2(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags2'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags2 = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flags3 of class  monster_race */
static int tolua_get_monster_race_flags3(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags3'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags3);
 return 1;
}

/* set function: flags3 of class  monster_race */
static int tolua_set_monster_race_flags3(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags3'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags3 = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flags4 of class  monster_race */
static int tolua_get_monster_race_flags4(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags4'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags4);
 return 1;
}

/* set function: flags4 of class  monster_race */
static int tolua_set_monster_race_flags4(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags4'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags4 = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flags5 of class  monster_race */
static int tolua_get_monster_race_flags5(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags5'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags5);
 return 1;
}

/* set function: flags5 of class  monster_race */
static int tolua_set_monster_race_flags5(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags5'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags5 = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flags6 of class  monster_race */
static int tolua_get_monster_race_flags6(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags6'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags6);
 return 1;
}

/* set function: flags6 of class  monster_race */
static int tolua_set_monster_race_flags6(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags6'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags6 = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: blow of class  monster_race */
static int tolua_get_monster_monster_race_blow(lua_State* tolua_S)
{
 int tolua_index;
  monster_race* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (monster_race*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=MONSTER_BLOW_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushusertype(tolua_S,(void*)&self->blow[tolua_index],"monster_blow");
 return 1;
}

/* set function: blow of class  monster_race */
static int tolua_set_monster_monster_race_blow(lua_State* tolua_S)
{
 int tolua_index;
  monster_race* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (monster_race*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=MONSTER_BLOW_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->blow[tolua_index] = *((monster_blow*)  tolua_tousertype(tolua_S,3,0));
 return 0;
}

/* get function: level of class  monster_race */
static int tolua_get_monster_race_level(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'level'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->level);
 return 1;
}

/* set function: level of class  monster_race */
static int tolua_set_monster_race_level(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'level'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->level = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: rarity of class  monster_race */
static int tolua_get_monster_race_rarity(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'rarity'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->rarity);
 return 1;
}

/* set function: rarity of class  monster_race */
static int tolua_set_monster_race_rarity(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'rarity'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->rarity = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: d_attr of class  monster_race */
static int tolua_get_monster_race_d_attr(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'd_attr'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->d_attr);
 return 1;
}

/* set function: d_attr of class  monster_race */
static int tolua_set_monster_race_d_attr(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'd_attr'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->d_attr = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: d_char of class  monster_race */
static int tolua_get_monster_race_d_char(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'd_char'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->d_char);
 return 1;
}

/* set function: d_char of class  monster_race */
static int tolua_set_monster_race_d_char(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'd_char'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->d_char = ((char)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: x_attr of class  monster_race */
static int tolua_get_monster_race_x_attr(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_attr'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->x_attr);
 return 1;
}

/* set function: x_attr of class  monster_race */
static int tolua_set_monster_race_x_attr(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_attr'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->x_attr = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: x_char of class  monster_race */
static int tolua_get_monster_race_x_char(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_char'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->x_char);
 return 1;
}

/* set function: x_char of class  monster_race */
static int tolua_set_monster_race_x_char(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_char'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->x_char = ((char)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: max_num of class  monster_race */
static int tolua_get_monster_race_max_num(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'max_num'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->max_num);
 return 1;
}

/* set function: max_num of class  monster_race */
static int tolua_set_monster_race_max_num(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'max_num'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->max_num = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: cur_num of class  monster_race */
static int tolua_get_monster_race_cur_num(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cur_num'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->cur_num);
 return 1;
}

/* set function: cur_num of class  monster_race */
static int tolua_set_monster_race_cur_num(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cur_num'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->cur_num = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: sights of class  monster_lore */
static int tolua_get_monster_lore_sights(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sights'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->sights);
 return 1;
}

/* set function: sights of class  monster_lore */
static int tolua_set_monster_lore_sights(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'sights'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->sights = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: deaths of class  monster_lore */
static int tolua_get_monster_lore_deaths(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'deaths'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->deaths);
 return 1;
}

/* set function: deaths of class  monster_lore */
static int tolua_set_monster_lore_deaths(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'deaths'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->deaths = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: pkills of class  monster_lore */
static int tolua_get_monster_lore_pkills(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'pkills'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->pkills);
 return 1;
}

/* set function: pkills of class  monster_lore */
static int tolua_set_monster_lore_pkills(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'pkills'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->pkills = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: tkills of class  monster_lore */
static int tolua_get_monster_lore_tkills(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'tkills'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->tkills);
 return 1;
}

/* set function: tkills of class  monster_lore */
static int tolua_set_monster_lore_tkills(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'tkills'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->tkills = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: wake of class  monster_lore */
static int tolua_get_monster_lore_wake(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'wake'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->wake);
 return 1;
}

/* set function: wake of class  monster_lore */
static int tolua_set_monster_lore_wake(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'wake'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->wake = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: ignore of class  monster_lore */
static int tolua_get_monster_lore_ignore(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ignore'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->ignore);
 return 1;
}

/* set function: ignore of class  monster_lore */
static int tolua_set_monster_lore_ignore(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ignore'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ignore = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: drop_gold of class  monster_lore */
static int tolua_get_monster_lore_drop_gold(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'drop_gold'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->drop_gold);
 return 1;
}

/* set function: drop_gold of class  monster_lore */
static int tolua_set_monster_lore_drop_gold(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'drop_gold'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->drop_gold = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: drop_item of class  monster_lore */
static int tolua_get_monster_lore_drop_item(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'drop_item'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->drop_item);
 return 1;
}

/* set function: drop_item of class  monster_lore */
static int tolua_set_monster_lore_drop_item(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'drop_item'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->drop_item = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: cast_innate of class  monster_lore */
static int tolua_get_monster_lore_cast_innate(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cast_innate'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->cast_innate);
 return 1;
}

/* set function: cast_innate of class  monster_lore */
static int tolua_set_monster_lore_cast_innate(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cast_innate'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->cast_innate = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: cast_spell of class  monster_lore */
static int tolua_get_monster_lore_cast_spell(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cast_spell'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->cast_spell);
 return 1;
}

/* set function: cast_spell of class  monster_lore */
static int tolua_set_monster_lore_cast_spell(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cast_spell'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->cast_spell = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: blows of class  monster_lore */
static int tolua_get_monster_monster_lore_blows(lua_State* tolua_S)
{
 int tolua_index;
  monster_lore* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (monster_lore*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=MONSTER_BLOW_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->blows[tolua_index]);
 return 1;
}

/* set function: blows of class  monster_lore */
static int tolua_set_monster_monster_lore_blows(lua_State* tolua_S)
{
 int tolua_index;
  monster_lore* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (monster_lore*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0)-1;
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=MONSTER_BLOW_MAX)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->blows[tolua_index] = ((byte)  tolua_tonumber(tolua_S,3,0));
 return 0;
}

/* get function: flags1 of class  monster_lore */
static int tolua_get_monster_lore_flags1(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags1'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags1);
 return 1;
}

/* set function: flags1 of class  monster_lore */
static int tolua_set_monster_lore_flags1(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags1'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags1 = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flags2 of class  monster_lore */
static int tolua_get_monster_lore_flags2(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags2'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags2);
 return 1;
}

/* set function: flags2 of class  monster_lore */
static int tolua_set_monster_lore_flags2(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags2'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags2 = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flags3 of class  monster_lore */
static int tolua_get_monster_lore_flags3(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags3'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags3);
 return 1;
}

/* set function: flags3 of class  monster_lore */
static int tolua_set_monster_lore_flags3(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags3'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags3 = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flags4 of class  monster_lore */
static int tolua_get_monster_lore_flags4(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags4'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags4);
 return 1;
}

/* set function: flags4 of class  monster_lore */
static int tolua_set_monster_lore_flags4(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags4'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags4 = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flags5 of class  monster_lore */
static int tolua_get_monster_lore_flags5(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags5'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags5);
 return 1;
}

/* set function: flags5 of class  monster_lore */
static int tolua_set_monster_lore_flags5(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags5'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags5 = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: flags6 of class  monster_lore */
static int tolua_get_monster_lore_flags6(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags6'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->flags6);
 return 1;
}

/* set function: flags6 of class  monster_lore */
static int tolua_set_monster_lore_flags6(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'flags6'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->flags6 = ((u32b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: r_idx of class  monster_type */
static int tolua_get_monster_type_r_idx(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'r_idx'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->r_idx);
 return 1;
}

/* set function: r_idx of class  monster_type */
static int tolua_set_monster_type_r_idx(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'r_idx'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->r_idx = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: fy of class  monster_type */
static int tolua_get_monster_type_fy(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'fy'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->fy);
 return 1;
}

/* set function: fy of class  monster_type */
static int tolua_set_monster_type_fy(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'fy'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->fy = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: fx of class  monster_type */
static int tolua_get_monster_type_fx(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'fx'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->fx);
 return 1;
}

/* set function: fx of class  monster_type */
static int tolua_set_monster_type_fx(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'fx'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->fx = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: hp of class  monster_type */
static int tolua_get_monster_type_hp(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'hp'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->hp);
 return 1;
}

/* set function: hp of class  monster_type */
static int tolua_set_monster_type_hp(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'hp'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->hp = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: maxhp of class  monster_type */
static int tolua_get_monster_type_maxhp(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'maxhp'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->maxhp);
 return 1;
}

/* set function: maxhp of class  monster_type */
static int tolua_set_monster_type_maxhp(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'maxhp'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->maxhp = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: csleep of class  monster_type */
static int tolua_get_monster_type_csleep(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'csleep'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->csleep);
 return 1;
}

/* set function: csleep of class  monster_type */
static int tolua_set_monster_type_csleep(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'csleep'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->csleep = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: mspeed of class  monster_type */
static int tolua_get_monster_type_mspeed(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'mspeed'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->mspeed);
 return 1;
}

/* set function: mspeed of class  monster_type */
static int tolua_set_monster_type_mspeed(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'mspeed'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->mspeed = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: energy of class  monster_type */
static int tolua_get_monster_type_energy(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'energy'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->energy);
 return 1;
}

/* set function: energy of class  monster_type */
static int tolua_set_monster_type_energy(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'energy'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->energy = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: stunned of class  monster_type */
static int tolua_get_monster_type_stunned(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'stunned'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->stunned);
 return 1;
}

/* set function: stunned of class  monster_type */
static int tolua_set_monster_type_stunned(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'stunned'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->stunned = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: confused of class  monster_type */
static int tolua_get_monster_type_confused(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'confused'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->confused);
 return 1;
}

/* set function: confused of class  monster_type */
static int tolua_set_monster_type_confused(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'confused'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->confused = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: monfear of class  monster_type */
static int tolua_get_monster_type_monfear(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'monfear'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->monfear);
 return 1;
}

/* set function: monfear of class  monster_type */
static int tolua_set_monster_type_monfear(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'monfear'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->monfear = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: cdis of class  monster_type */
static int tolua_get_monster_type_cdis(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cdis'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->cdis);
 return 1;
}

/* set function: cdis of class  monster_type */
static int tolua_set_monster_type_cdis(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cdis'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->cdis = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: mflag of class  monster_type */
static int tolua_get_monster_type_mflag(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'mflag'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->mflag);
 return 1;
}

/* set function: mflag of class  monster_type */
static int tolua_set_monster_type_mflag(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'mflag'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->mflag = ((byte)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: ml of class  monster_type */
static int tolua_get_monster_type_ml(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ml'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->ml);
 return 1;
}

/* set function: ml of class  monster_type */
static int tolua_set_monster_type_ml(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ml'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ml = ((bool)  tolua_toboolean(tolua_S,2,0));
 return 0;
}

/* get function: hold_o_idx of class  monster_type */
static int tolua_get_monster_type_hold_o_idx(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'hold_o_idx'",NULL);
#endif
 tolua_pushnumber(tolua_S,(long)self->hold_o_idx);
 return 1;
}

/* set function: hold_o_idx of class  monster_type */
static int tolua_set_monster_type_hold_o_idx(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'hold_o_idx'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->hold_o_idx = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: mon_max */
static int tolua_get_mon_max(lua_State* tolua_S)
{
 tolua_pushnumber(tolua_S,(long)mon_max);
 return 1;
}

/* set function: mon_max */
static int tolua_set_mon_max(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  mon_max = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: mon_cnt */
static int tolua_get_mon_cnt(lua_State* tolua_S)
{
 tolua_pushnumber(tolua_S,(long)mon_cnt);
 return 1;
}

/* set function: mon_cnt */
static int tolua_set_mon_cnt(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  mon_cnt = ((s16b)  tolua_tonumber(tolua_S,2,0));
 return 0;
}

/* get function: mon_list */
static int tolua_get_monster_mon_list(lua_State* tolua_S)
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
 if (tolua_index<0 || tolua_index>=z_info->m_max)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushusertype(tolua_S,(void*)&mon_list[tolua_index],"monster_type");
 return 1;
}

/* set function: mon_list */
static int tolua_set_monster_mon_list(lua_State* tolua_S)
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
 if (tolua_index<0 || tolua_index>=z_info->m_max)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  mon_list[tolua_index] = *((monster_type*)  tolua_tousertype(tolua_S,3,0));
 return 0;
}

/* function: make_attack_normal */
static int tolua_monster_make_attack_normal00(lua_State* tolua_S)
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
  int m_idx = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  make_attack_normal(m_idx);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'make_attack_normal'.",&tolua_err);
 return 0;
#endif
}

/* function: make_attack_spell */
static int tolua_monster_make_attack_spell00(lua_State* tolua_S)
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
  int m_idx = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  make_attack_spell(m_idx);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'make_attack_spell'.",&tolua_err);
 return 0;
#endif
}

/* function: process_monsters */
static int tolua_monster_process_monsters00(lua_State* tolua_S)
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
  byte minimum_energy = ((byte)  tolua_tonumber(tolua_S,1,0));
 {
  process_monsters(minimum_energy);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'process_monsters'.",&tolua_err);
 return 0;
#endif
}

/* function: roff_top */
static int tolua_monster_roff_top00(lua_State* tolua_S)
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
  int r_idx = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  roff_top(r_idx);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'roff_top'.",&tolua_err);
 return 0;
#endif
}

/* function: screen_roff */
static int tolua_monster_screen_roff00(lua_State* tolua_S)
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
  int r_idx = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  screen_roff(r_idx);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'screen_roff'.",&tolua_err);
 return 0;
#endif
}

/* function: display_roff */
static int tolua_monster_display_roff00(lua_State* tolua_S)
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
  int r_idx = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  display_roff(r_idx);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'display_roff'.",&tolua_err);
 return 0;
#endif
}

/* function: delete_monster_idx */
static int tolua_monster_delete_monster_idx00(lua_State* tolua_S)
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
  int i = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  delete_monster_idx(i);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete_monster_idx'.",&tolua_err);
 return 0;
#endif
}

/* function: delete_monster */
static int tolua_monster_delete_monster00(lua_State* tolua_S)
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
  int y = ((int)  tolua_tonumber(tolua_S,1,0));
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  delete_monster(y,x);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete_monster'.",&tolua_err);
 return 0;
#endif
}

/* function: compact_monsters */
static int tolua_monster_compact_monsters00(lua_State* tolua_S)
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
  int size = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  compact_monsters(size);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'compact_monsters'.",&tolua_err);
 return 0;
#endif
}

/* function: wipe_mon_list */
static int tolua_monster_wipe_mon_list00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  wipe_mon_list();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'wipe_mon_list'.",&tolua_err);
 return 0;
#endif
}

/* function: mon_pop */
static int tolua_monster_mon_pop00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  s16b tolua_ret = (s16b)  mon_pop();
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'mon_pop'.",&tolua_err);
 return 0;
#endif
}

/* function: get_mon_num_prep */
static int tolua_monster_get_mon_num_prep00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  errr tolua_ret = (errr)  get_mon_num_prep();
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'get_mon_num_prep'.",&tolua_err);
 return 0;
#endif
}

/* function: get_mon_num */
static int tolua_monster_get_mon_num00(lua_State* tolua_S)
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
  int level = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  s16b tolua_ret = (s16b)  get_mon_num(level);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'get_mon_num'.",&tolua_err);
 return 0;
#endif
}

/* function: lore_do_probe */
static int tolua_monster_lore_do_probe00(lua_State* tolua_S)
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
  int m_idx = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  lore_do_probe(m_idx);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'lore_do_probe'.",&tolua_err);
 return 0;
#endif
}

/* function: lore_treasure */
static int tolua_monster_lore_treasure00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int m_idx = ((int)  tolua_tonumber(tolua_S,1,0));
  int num_item = ((int)  tolua_tonumber(tolua_S,2,0));
  int num_gold = ((int)  tolua_tonumber(tolua_S,3,0));
 {
  lore_treasure(m_idx,num_item,num_gold);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'lore_treasure'.",&tolua_err);
 return 0;
#endif
}

/* function: update_mon */
static int tolua_monster_update_mon00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isboolean(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int m_idx = ((int)  tolua_tonumber(tolua_S,1,0));
  bool full = ((bool)  tolua_toboolean(tolua_S,2,0));
 {
  update_mon(m_idx,full);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'update_mon'.",&tolua_err);
 return 0;
#endif
}

/* function: update_monsters */
static int tolua_monster_update_monsters00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isboolean(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  bool full = ((bool)  tolua_toboolean(tolua_S,1,0));
 {
  update_monsters(full);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'update_monsters'.",&tolua_err);
 return 0;
#endif
}

/* function: monster_carry */
static int tolua_monster_monster_carry00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"object_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int m_idx = ((int)  tolua_tonumber(tolua_S,1,0));
  object_type* j_ptr = ((object_type*)  tolua_tousertype(tolua_S,2,0));
 {
  s16b tolua_ret = (s16b)  monster_carry(m_idx,j_ptr);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'monster_carry'.",&tolua_err);
 return 0;
#endif
}

/* function: monster_swap */
static int tolua_monster_monster_swap00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int y1 = ((int)  tolua_tonumber(tolua_S,1,0));
  int x1 = ((int)  tolua_tonumber(tolua_S,2,0));
  int y2 = ((int)  tolua_tonumber(tolua_S,3,0));
  int x2 = ((int)  tolua_tonumber(tolua_S,4,0));
 {
  monster_swap(y1,x1,y2,x2);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'monster_swap'.",&tolua_err);
 return 0;
#endif
}

/* function: player_place */
static int tolua_monster_player_place00(lua_State* tolua_S)
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
  int y = ((int)  tolua_tonumber(tolua_S,1,0));
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  s16b tolua_ret = (s16b)  player_place(y,x);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'player_place'.",&tolua_err);
 return 0;
#endif
}

/* function: monster_place */
static int tolua_monster_monster_place00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isusertype(tolua_S,3,"monster_type",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int y = ((int)  tolua_tonumber(tolua_S,1,0));
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
  monster_type* n_ptr = ((monster_type*)  tolua_tousertype(tolua_S,3,0));
 {
  s16b tolua_ret = (s16b)  monster_place(y,x,n_ptr);
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'monster_place'.",&tolua_err);
 return 0;
#endif
}

/* function: place_monster_aux */
static int tolua_monster_place_monster_aux00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isboolean(tolua_S,4,0,&tolua_err) ||
 !tolua_isboolean(tolua_S,5,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,6,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int y = ((int)  tolua_tonumber(tolua_S,1,0));
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
  int r_idx = ((int)  tolua_tonumber(tolua_S,3,0));
  bool slp = ((bool)  tolua_toboolean(tolua_S,4,0));
  bool grp = ((bool)  tolua_toboolean(tolua_S,5,0));
 {
  bool tolua_ret = (bool)  place_monster_aux(y,x,r_idx,slp,grp);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'place_monster_aux'.",&tolua_err);
 return 0;
#endif
}

/* function: place_monster */
static int tolua_monster_place_monster00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isboolean(tolua_S,3,0,&tolua_err) ||
 !tolua_isboolean(tolua_S,4,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int y = ((int)  tolua_tonumber(tolua_S,1,0));
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
  bool slp = ((bool)  tolua_toboolean(tolua_S,3,0));
  bool grp = ((bool)  tolua_toboolean(tolua_S,4,0));
 {
  bool tolua_ret = (bool)  place_monster(y,x,slp,grp);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'place_monster'.",&tolua_err);
 return 0;
#endif
}

/* function: alloc_monster */
static int tolua_monster_alloc_monster00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isboolean(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dis = ((int)  tolua_tonumber(tolua_S,1,0));
  bool slp = ((bool)  tolua_toboolean(tolua_S,2,0));
 {
  bool tolua_ret = (bool)  alloc_monster(dis,slp);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'alloc_monster'.",&tolua_err);
 return 0;
#endif
}

/* function: summon_specific */
static int tolua_monster_summon_specific00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int y1 = ((int)  tolua_tonumber(tolua_S,1,0));
  int x1 = ((int)  tolua_tonumber(tolua_S,2,0));
  int lev = ((int)  tolua_tonumber(tolua_S,3,0));
  int type = ((int)  tolua_tonumber(tolua_S,4,0));
 {
  bool tolua_ret = (bool)  summon_specific(y1,x1,lev,type);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'summon_specific'.",&tolua_err);
 return 0;
#endif
}

/* function: multiply_monster */
static int tolua_monster_multiply_monster00(lua_State* tolua_S)
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
  int m_idx = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  multiply_monster(m_idx);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'multiply_monster'.",&tolua_err);
 return 0;
#endif
}

/* function: message_pain */
static int tolua_monster_message_pain00(lua_State* tolua_S)
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
  int m_idx = ((int)  tolua_tonumber(tolua_S,1,0));
  int dam = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  message_pain(m_idx,dam);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'message_pain'.",&tolua_err);
 return 0;
#endif
}

/* function: update_smart_learn */
static int tolua_monster_update_smart_learn00(lua_State* tolua_S)
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
  int m_idx = ((int)  tolua_tonumber(tolua_S,1,0));
  int what = ((int)  tolua_tonumber(tolua_S,2,0));
 {
  update_smart_learn(m_idx,what);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'update_smart_learn'.",&tolua_err);
 return 0;
#endif
}

/* function: monster_death */
static int tolua_monster_monster_death00(lua_State* tolua_S)
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
  int m_idx = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  monster_death(m_idx);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'monster_death'.",&tolua_err);
 return 0;
#endif
}

/* function: mon_take_hit */
static int tolua_monster_mon_take_hit00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isboolean(tolua_S,3,0,&tolua_err) ||
 !tolua_isstring(tolua_S,4,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int m_idx = ((int)  tolua_tonumber(tolua_S,1,0));
  int dam = ((int)  tolua_tonumber(tolua_S,2,0));
  bool fear = ((bool)  tolua_toboolean(tolua_S,3,0));
  cptr note = ((cptr)  tolua_tostring(tolua_S,4,0));
 {
  bool tolua_ret = (bool)  mon_take_hit(m_idx,dam,&fear,note);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 tolua_pushboolean(tolua_S,(bool)fear);
 }
 }
 return 2;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'mon_take_hit'.",&tolua_err);
 return 0;
#endif
}

/* Open function */
TOLUA_API int tolua_monster_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,1);
 tolua_beginmodule(tolua_S,NULL);
 tolua_constant(tolua_S,"RBM_HIT",RBM_HIT);
 tolua_constant(tolua_S,"RBM_TOUCH",RBM_TOUCH);
 tolua_constant(tolua_S,"RBM_PUNCH",RBM_PUNCH);
 tolua_constant(tolua_S,"RBM_KICK",RBM_KICK);
 tolua_constant(tolua_S,"RBM_CLAW",RBM_CLAW);
 tolua_constant(tolua_S,"RBM_BITE",RBM_BITE);
 tolua_constant(tolua_S,"RBM_STING",RBM_STING);
 tolua_constant(tolua_S,"RBM_XXX1",RBM_XXX1);
 tolua_constant(tolua_S,"RBM_BUTT",RBM_BUTT);
 tolua_constant(tolua_S,"RBM_CRUSH",RBM_CRUSH);
 tolua_constant(tolua_S,"RBM_ENGULF",RBM_ENGULF);
 tolua_constant(tolua_S,"RBM_XXX2",RBM_XXX2);
 tolua_constant(tolua_S,"RBM_CRAWL",RBM_CRAWL);
 tolua_constant(tolua_S,"RBM_DROOL",RBM_DROOL);
 tolua_constant(tolua_S,"RBM_SPIT",RBM_SPIT);
 tolua_constant(tolua_S,"RBM_XXX3",RBM_XXX3);
 tolua_constant(tolua_S,"RBM_GAZE",RBM_GAZE);
 tolua_constant(tolua_S,"RBM_WAIL",RBM_WAIL);
 tolua_constant(tolua_S,"RBM_SPORE",RBM_SPORE);
 tolua_constant(tolua_S,"RBM_XXX4",RBM_XXX4);
 tolua_constant(tolua_S,"RBM_BEG",RBM_BEG);
 tolua_constant(tolua_S,"RBM_INSULT",RBM_INSULT);
 tolua_constant(tolua_S,"RBM_MOAN",RBM_MOAN);
 tolua_constant(tolua_S,"RBM_XXX5",RBM_XXX5);
 tolua_constant(tolua_S,"RBE_HURT",RBE_HURT);
 tolua_constant(tolua_S,"RBE_POISON",RBE_POISON);
 tolua_constant(tolua_S,"RBE_UN_BONUS",RBE_UN_BONUS);
 tolua_constant(tolua_S,"RBE_UN_POWER",RBE_UN_POWER);
 tolua_constant(tolua_S,"RBE_EAT_GOLD",RBE_EAT_GOLD);
 tolua_constant(tolua_S,"RBE_EAT_ITEM",RBE_EAT_ITEM);
 tolua_constant(tolua_S,"RBE_EAT_FOOD",RBE_EAT_FOOD);
 tolua_constant(tolua_S,"RBE_EAT_LITE",RBE_EAT_LITE);
 tolua_constant(tolua_S,"RBE_ACID",RBE_ACID);
 tolua_constant(tolua_S,"RBE_ELEC",RBE_ELEC);
 tolua_constant(tolua_S,"RBE_FIRE",RBE_FIRE);
 tolua_constant(tolua_S,"RBE_COLD",RBE_COLD);
 tolua_constant(tolua_S,"RBE_BLIND",RBE_BLIND);
 tolua_constant(tolua_S,"RBE_CONFUSE",RBE_CONFUSE);
 tolua_constant(tolua_S,"RBE_TERRIFY",RBE_TERRIFY);
 tolua_constant(tolua_S,"RBE_PARALYZE",RBE_PARALYZE);
 tolua_constant(tolua_S,"RBE_LOSE_STR",RBE_LOSE_STR);
 tolua_constant(tolua_S,"RBE_LOSE_INT",RBE_LOSE_INT);
 tolua_constant(tolua_S,"RBE_LOSE_WIS",RBE_LOSE_WIS);
 tolua_constant(tolua_S,"RBE_LOSE_DEX",RBE_LOSE_DEX);
 tolua_constant(tolua_S,"RBE_LOSE_CON",RBE_LOSE_CON);
 tolua_constant(tolua_S,"RBE_LOSE_CHR",RBE_LOSE_CHR);
 tolua_constant(tolua_S,"RBE_LOSE_ALL",RBE_LOSE_ALL);
 tolua_constant(tolua_S,"RBE_SHATTER",RBE_SHATTER);
 tolua_constant(tolua_S,"RBE_EXP_10",RBE_EXP_10);
 tolua_constant(tolua_S,"RBE_EXP_20",RBE_EXP_20);
 tolua_constant(tolua_S,"RBE_EXP_40",RBE_EXP_40);
 tolua_constant(tolua_S,"RBE_EXP_80",RBE_EXP_80);
 tolua_constant(tolua_S,"RBE_HALLU",RBE_HALLU);
 tolua_constant(tolua_S,"SM_OPP_ACID",SM_OPP_ACID);
 tolua_constant(tolua_S,"SM_OPP_ELEC",SM_OPP_ELEC);
 tolua_constant(tolua_S,"SM_OPP_FIRE",SM_OPP_FIRE);
 tolua_constant(tolua_S,"SM_OPP_COLD",SM_OPP_COLD);
 tolua_constant(tolua_S,"SM_OPP_POIS",SM_OPP_POIS);
 tolua_constant(tolua_S,"SM_OPP_XXX1",SM_OPP_XXX1);
 tolua_constant(tolua_S,"SM_OPP_XXX2",SM_OPP_XXX2);
 tolua_constant(tolua_S,"SM_OPP_XXX3",SM_OPP_XXX3);
 tolua_constant(tolua_S,"SM_IMM_XXX5",SM_IMM_XXX5);
 tolua_constant(tolua_S,"SM_IMM_XXX6",SM_IMM_XXX6);
 tolua_constant(tolua_S,"SM_IMM_FREE",SM_IMM_FREE);
 tolua_constant(tolua_S,"SM_IMM_MANA",SM_IMM_MANA);
 tolua_constant(tolua_S,"SM_IMM_ACID",SM_IMM_ACID);
 tolua_constant(tolua_S,"SM_IMM_ELEC",SM_IMM_ELEC);
 tolua_constant(tolua_S,"SM_IMM_FIRE",SM_IMM_FIRE);
 tolua_constant(tolua_S,"SM_IMM_COLD",SM_IMM_COLD);
 tolua_constant(tolua_S,"SM_RES_ACID",SM_RES_ACID);
 tolua_constant(tolua_S,"SM_RES_ELEC",SM_RES_ELEC);
 tolua_constant(tolua_S,"SM_RES_FIRE",SM_RES_FIRE);
 tolua_constant(tolua_S,"SM_RES_COLD",SM_RES_COLD);
 tolua_constant(tolua_S,"SM_RES_POIS",SM_RES_POIS);
 tolua_constant(tolua_S,"SM_RES_FEAR",SM_RES_FEAR);
 tolua_constant(tolua_S,"SM_RES_LITE",SM_RES_LITE);
 tolua_constant(tolua_S,"SM_RES_DARK",SM_RES_DARK);
 tolua_constant(tolua_S,"SM_RES_BLIND",SM_RES_BLIND);
 tolua_constant(tolua_S,"SM_RES_CONFU",SM_RES_CONFU);
 tolua_constant(tolua_S,"SM_RES_SOUND",SM_RES_SOUND);
 tolua_constant(tolua_S,"SM_RES_SHARD",SM_RES_SHARD);
 tolua_constant(tolua_S,"SM_RES_NEXUS",SM_RES_NEXUS);
 tolua_constant(tolua_S,"SM_RES_NETHR",SM_RES_NETHR);
 tolua_constant(tolua_S,"SM_RES_CHAOS",SM_RES_CHAOS);
 tolua_constant(tolua_S,"SM_RES_DISEN",SM_RES_DISEN);
 tolua_constant(tolua_S,"MFLAG_VIEW",MFLAG_VIEW);
 tolua_constant(tolua_S,"MFLAG_NICE",MFLAG_NICE);
 tolua_constant(tolua_S,"MFLAG_SHOW",MFLAG_SHOW);
 tolua_constant(tolua_S,"MFLAG_MARK",MFLAG_MARK);
 tolua_constant(tolua_S,"RF1_UNIQUE",RF1_UNIQUE);
 tolua_constant(tolua_S,"RF1_QUESTOR",RF1_QUESTOR);
 tolua_constant(tolua_S,"RF1_MALE",RF1_MALE);
 tolua_constant(tolua_S,"RF1_FEMALE",RF1_FEMALE);
 tolua_constant(tolua_S,"RF1_CHAR_CLEAR",RF1_CHAR_CLEAR);
 tolua_constant(tolua_S,"RF1_CHAR_MULTI",RF1_CHAR_MULTI);
 tolua_constant(tolua_S,"RF1_ATTR_CLEAR",RF1_ATTR_CLEAR);
 tolua_constant(tolua_S,"RF1_ATTR_MULTI",RF1_ATTR_MULTI);
 tolua_constant(tolua_S,"RF1_FORCE_DEPTH",RF1_FORCE_DEPTH);
 tolua_constant(tolua_S,"RF1_FORCE_MAXHP",RF1_FORCE_MAXHP);
 tolua_constant(tolua_S,"RF1_FORCE_SLEEP",RF1_FORCE_SLEEP);
 tolua_constant(tolua_S,"RF1_FORCE_EXTRA",RF1_FORCE_EXTRA);
 tolua_constant(tolua_S,"RF1_FRIEND",RF1_FRIEND);
 tolua_constant(tolua_S,"RF1_FRIENDS",RF1_FRIENDS);
 tolua_constant(tolua_S,"RF1_ESCORT",RF1_ESCORT);
 tolua_constant(tolua_S,"RF1_ESCORTS",RF1_ESCORTS);
 tolua_constant(tolua_S,"RF1_NEVER_BLOW",RF1_NEVER_BLOW);
 tolua_constant(tolua_S,"RF1_NEVER_MOVE",RF1_NEVER_MOVE);
 tolua_constant(tolua_S,"RF1_RAND_25",RF1_RAND_25);
 tolua_constant(tolua_S,"RF1_RAND_50",RF1_RAND_50);
 tolua_constant(tolua_S,"RF1_ONLY_GOLD",RF1_ONLY_GOLD);
 tolua_constant(tolua_S,"RF1_ONLY_ITEM",RF1_ONLY_ITEM);
 tolua_constant(tolua_S,"RF1_DROP_60",RF1_DROP_60);
 tolua_constant(tolua_S,"RF1_DROP_90",RF1_DROP_90);
 tolua_constant(tolua_S,"RF1_DROP_1D2",RF1_DROP_1D2);
 tolua_constant(tolua_S,"RF1_DROP_2D2",RF1_DROP_2D2);
 tolua_constant(tolua_S,"RF1_DROP_3D2",RF1_DROP_3D2);
 tolua_constant(tolua_S,"RF1_DROP_4D2",RF1_DROP_4D2);
 tolua_constant(tolua_S,"RF1_DROP_GOOD",RF1_DROP_GOOD);
 tolua_constant(tolua_S,"RF1_DROP_GREAT",RF1_DROP_GREAT);
 tolua_constant(tolua_S,"RF1_DROP_USEFUL",RF1_DROP_USEFUL);
 tolua_constant(tolua_S,"RF1_DROP_CHOSEN",RF1_DROP_CHOSEN);
 tolua_constant(tolua_S,"RF2_STUPID",RF2_STUPID);
 tolua_constant(tolua_S,"RF2_SMART",RF2_SMART);
 tolua_constant(tolua_S,"RF2_XXX1",RF2_XXX1);
 tolua_constant(tolua_S,"RF2_XXX2",RF2_XXX2);
 tolua_constant(tolua_S,"RF2_INVISIBLE",RF2_INVISIBLE);
 tolua_constant(tolua_S,"RF2_COLD_BLOOD",RF2_COLD_BLOOD);
 tolua_constant(tolua_S,"RF2_EMPTY_MIND",RF2_EMPTY_MIND);
 tolua_constant(tolua_S,"RF2_WEIRD_MIND",RF2_WEIRD_MIND);
 tolua_constant(tolua_S,"RF2_MULTIPLY",RF2_MULTIPLY);
 tolua_constant(tolua_S,"RF2_REGENERATE",RF2_REGENERATE);
 tolua_constant(tolua_S,"RF2_XXX3",RF2_XXX3);
 tolua_constant(tolua_S,"RF2_XXX4",RF2_XXX4);
 tolua_constant(tolua_S,"RF2_POWERFUL",RF2_POWERFUL);
 tolua_constant(tolua_S,"RF2_XXX5",RF2_XXX5);
 tolua_constant(tolua_S,"RF2_XXX7",RF2_XXX7);
 tolua_constant(tolua_S,"RF2_XXX6",RF2_XXX6);
 tolua_constant(tolua_S,"RF2_OPEN_DOOR",RF2_OPEN_DOOR);
 tolua_constant(tolua_S,"RF2_BASH_DOOR",RF2_BASH_DOOR);
 tolua_constant(tolua_S,"RF2_PASS_WALL",RF2_PASS_WALL);
 tolua_constant(tolua_S,"RF2_KILL_WALL",RF2_KILL_WALL);
 tolua_constant(tolua_S,"RF2_MOVE_BODY",RF2_MOVE_BODY);
 tolua_constant(tolua_S,"RF2_KILL_BODY",RF2_KILL_BODY);
 tolua_constant(tolua_S,"RF2_TAKE_ITEM",RF2_TAKE_ITEM);
 tolua_constant(tolua_S,"RF2_KILL_ITEM",RF2_KILL_ITEM);
 tolua_constant(tolua_S,"RF2_BRAIN_1",RF2_BRAIN_1);
 tolua_constant(tolua_S,"RF2_BRAIN_2",RF2_BRAIN_2);
 tolua_constant(tolua_S,"RF2_BRAIN_3",RF2_BRAIN_3);
 tolua_constant(tolua_S,"RF2_BRAIN_4",RF2_BRAIN_4);
 tolua_constant(tolua_S,"RF2_BRAIN_5",RF2_BRAIN_5);
 tolua_constant(tolua_S,"RF2_BRAIN_6",RF2_BRAIN_6);
 tolua_constant(tolua_S,"RF2_BRAIN_7",RF2_BRAIN_7);
 tolua_constant(tolua_S,"RF2_BRAIN_8",RF2_BRAIN_8);
 tolua_constant(tolua_S,"RF3_ORC",RF3_ORC);
 tolua_constant(tolua_S,"RF3_TROLL",RF3_TROLL);
 tolua_constant(tolua_S,"RF3_GIANT",RF3_GIANT);
 tolua_constant(tolua_S,"RF3_DRAGON",RF3_DRAGON);
 tolua_constant(tolua_S,"RF3_DEMON",RF3_DEMON);
 tolua_constant(tolua_S,"RF3_UNDEAD",RF3_UNDEAD);
 tolua_constant(tolua_S,"RF3_EVIL",RF3_EVIL);
 tolua_constant(tolua_S,"RF3_ANIMAL",RF3_ANIMAL);
 tolua_constant(tolua_S,"RF3_XXX1",RF3_XXX1);
 tolua_constant(tolua_S,"RF3_XXX2",RF3_XXX2);
 tolua_constant(tolua_S,"RF3_XXX3",RF3_XXX3);
 tolua_constant(tolua_S,"RF3_XXX4",RF3_XXX4);
 tolua_constant(tolua_S,"RF3_HURT_LITE",RF3_HURT_LITE);
 tolua_constant(tolua_S,"RF3_HURT_ROCK",RF3_HURT_ROCK);
 tolua_constant(tolua_S,"RF3_HURT_FIRE",RF3_HURT_FIRE);
 tolua_constant(tolua_S,"RF3_HURT_COLD",RF3_HURT_COLD);
 tolua_constant(tolua_S,"RF3_IM_ACID",RF3_IM_ACID);
 tolua_constant(tolua_S,"RF3_IM_ELEC",RF3_IM_ELEC);
 tolua_constant(tolua_S,"RF3_IM_FIRE",RF3_IM_FIRE);
 tolua_constant(tolua_S,"RF3_IM_COLD",RF3_IM_COLD);
 tolua_constant(tolua_S,"RF3_IM_POIS",RF3_IM_POIS);
 tolua_constant(tolua_S,"RF3_XXX5",RF3_XXX5);
 tolua_constant(tolua_S,"RF3_RES_NETH",RF3_RES_NETH);
 tolua_constant(tolua_S,"RF3_IM_WATER",RF3_IM_WATER);
 tolua_constant(tolua_S,"RF3_RES_PLAS",RF3_RES_PLAS);
 tolua_constant(tolua_S,"RF3_RES_NEXUS",RF3_RES_NEXUS);
 tolua_constant(tolua_S,"RF3_RES_DISE",RF3_RES_DISE);
 tolua_constant(tolua_S,"RF3_XXX6",RF3_XXX6);
 tolua_constant(tolua_S,"RF3_NO_FEAR",RF3_NO_FEAR);
 tolua_constant(tolua_S,"RF3_NO_STUN",RF3_NO_STUN);
 tolua_constant(tolua_S,"RF3_NO_CONF",RF3_NO_CONF);
 tolua_constant(tolua_S,"RF3_NO_SLEEP",RF3_NO_SLEEP);
 tolua_constant(tolua_S,"RF4_SHRIEK",RF4_SHRIEK);
 tolua_constant(tolua_S,"RF4_XXX2",RF4_XXX2);
 tolua_constant(tolua_S,"RF4_XXX3",RF4_XXX3);
 tolua_constant(tolua_S,"RF4_XXX4",RF4_XXX4);
 tolua_constant(tolua_S,"RF4_ARROW_1",RF4_ARROW_1);
 tolua_constant(tolua_S,"RF4_ARROW_2",RF4_ARROW_2);
 tolua_constant(tolua_S,"RF4_ARROW_3",RF4_ARROW_3);
 tolua_constant(tolua_S,"RF4_ARROW_4",RF4_ARROW_4);
 tolua_constant(tolua_S,"RF4_BR_ACID",RF4_BR_ACID);
 tolua_constant(tolua_S,"RF4_BR_ELEC",RF4_BR_ELEC);
 tolua_constant(tolua_S,"RF4_BR_FIRE",RF4_BR_FIRE);
 tolua_constant(tolua_S,"RF4_BR_COLD",RF4_BR_COLD);
 tolua_constant(tolua_S,"RF4_BR_POIS",RF4_BR_POIS);
 tolua_constant(tolua_S,"RF4_BR_NETH",RF4_BR_NETH);
 tolua_constant(tolua_S,"RF4_BR_LITE",RF4_BR_LITE);
 tolua_constant(tolua_S,"RF4_BR_DARK",RF4_BR_DARK);
 tolua_constant(tolua_S,"RF4_BR_CONF",RF4_BR_CONF);
 tolua_constant(tolua_S,"RF4_BR_SOUN",RF4_BR_SOUN);
 tolua_constant(tolua_S,"RF4_BR_CHAO",RF4_BR_CHAO);
 tolua_constant(tolua_S,"RF4_BR_DISE",RF4_BR_DISE);
 tolua_constant(tolua_S,"RF4_BR_NEXU",RF4_BR_NEXU);
 tolua_constant(tolua_S,"RF4_BR_TIME",RF4_BR_TIME);
 tolua_constant(tolua_S,"RF4_BR_INER",RF4_BR_INER);
 tolua_constant(tolua_S,"RF4_BR_GRAV",RF4_BR_GRAV);
 tolua_constant(tolua_S,"RF4_BR_SHAR",RF4_BR_SHAR);
 tolua_constant(tolua_S,"RF4_BR_PLAS",RF4_BR_PLAS);
 tolua_constant(tolua_S,"RF4_BR_WALL",RF4_BR_WALL);
 tolua_constant(tolua_S,"RF4_BR_MANA",RF4_BR_MANA);
 tolua_constant(tolua_S,"RF4_XXX5",RF4_XXX5);
 tolua_constant(tolua_S,"RF4_XXX6",RF4_XXX6);
 tolua_constant(tolua_S,"RF4_XXX7",RF4_XXX7);
 tolua_constant(tolua_S,"RF4_BOULDER",RF4_BOULDER);
 tolua_constant(tolua_S,"RF5_BA_ACID",RF5_BA_ACID);
 tolua_constant(tolua_S,"RF5_BA_ELEC",RF5_BA_ELEC);
 tolua_constant(tolua_S,"RF5_BA_FIRE",RF5_BA_FIRE);
 tolua_constant(tolua_S,"RF5_BA_COLD",RF5_BA_COLD);
 tolua_constant(tolua_S,"RF5_BA_POIS",RF5_BA_POIS);
 tolua_constant(tolua_S,"RF5_BA_NETH",RF5_BA_NETH);
 tolua_constant(tolua_S,"RF5_BA_WATE",RF5_BA_WATE);
 tolua_constant(tolua_S,"RF5_BA_MANA",RF5_BA_MANA);
 tolua_constant(tolua_S,"RF5_BA_DARK",RF5_BA_DARK);
 tolua_constant(tolua_S,"RF5_DRAIN_MANA",RF5_DRAIN_MANA);
 tolua_constant(tolua_S,"RF5_MIND_BLAST",RF5_MIND_BLAST);
 tolua_constant(tolua_S,"RF5_BRAIN_SMASH",RF5_BRAIN_SMASH);
 tolua_constant(tolua_S,"RF5_CAUSE_1",RF5_CAUSE_1);
 tolua_constant(tolua_S,"RF5_CAUSE_2",RF5_CAUSE_2);
 tolua_constant(tolua_S,"RF5_CAUSE_3",RF5_CAUSE_3);
 tolua_constant(tolua_S,"RF5_CAUSE_4",RF5_CAUSE_4);
 tolua_constant(tolua_S,"RF5_BO_ACID",RF5_BO_ACID);
 tolua_constant(tolua_S,"RF5_BO_ELEC",RF5_BO_ELEC);
 tolua_constant(tolua_S,"RF5_BO_FIRE",RF5_BO_FIRE);
 tolua_constant(tolua_S,"RF5_BO_COLD",RF5_BO_COLD);
 tolua_constant(tolua_S,"RF5_BO_POIS",RF5_BO_POIS);
 tolua_constant(tolua_S,"RF5_BO_NETH",RF5_BO_NETH);
 tolua_constant(tolua_S,"RF5_BO_WATE",RF5_BO_WATE);
 tolua_constant(tolua_S,"RF5_BO_MANA",RF5_BO_MANA);
 tolua_constant(tolua_S,"RF5_BO_PLAS",RF5_BO_PLAS);
 tolua_constant(tolua_S,"RF5_BO_ICEE",RF5_BO_ICEE);
 tolua_constant(tolua_S,"RF5_MISSILE",RF5_MISSILE);
 tolua_constant(tolua_S,"RF5_SCARE",RF5_SCARE);
 tolua_constant(tolua_S,"RF5_BLIND",RF5_BLIND);
 tolua_constant(tolua_S,"RF5_CONF",RF5_CONF);
 tolua_constant(tolua_S,"RF5_SLOW",RF5_SLOW);
 tolua_constant(tolua_S,"RF5_HOLD",RF5_HOLD);
 tolua_constant(tolua_S,"RF6_HASTE",RF6_HASTE);
 tolua_constant(tolua_S,"RF6_XXX1",RF6_XXX1);
 tolua_constant(tolua_S,"RF6_HEAL",RF6_HEAL);
 tolua_constant(tolua_S,"RF6_XXX2",RF6_XXX2);
 tolua_constant(tolua_S,"RF6_BLINK",RF6_BLINK);
 tolua_constant(tolua_S,"RF6_TPORT",RF6_TPORT);
 tolua_constant(tolua_S,"RF6_XXX3",RF6_XXX3);
 tolua_constant(tolua_S,"RF6_XXX4",RF6_XXX4);
 tolua_constant(tolua_S,"RF6_TELE_TO",RF6_TELE_TO);
 tolua_constant(tolua_S,"RF6_TELE_AWAY",RF6_TELE_AWAY);
 tolua_constant(tolua_S,"RF6_TELE_LEVEL",RF6_TELE_LEVEL);
 tolua_constant(tolua_S,"RF6_XXX5",RF6_XXX5);
 tolua_constant(tolua_S,"RF6_DARKNESS",RF6_DARKNESS);
 tolua_constant(tolua_S,"RF6_TRAPS",RF6_TRAPS);
 tolua_constant(tolua_S,"RF6_FORGET",RF6_FORGET);
 tolua_constant(tolua_S,"RF6_XXX6",RF6_XXX6);
 tolua_constant(tolua_S,"RF6_S_KIN",RF6_S_KIN);
 tolua_constant(tolua_S,"RF6_S_HI_DEMON",RF6_S_HI_DEMON);
 tolua_constant(tolua_S,"RF6_S_MONSTER",RF6_S_MONSTER);
 tolua_constant(tolua_S,"RF6_S_MONSTERS",RF6_S_MONSTERS);
 tolua_constant(tolua_S,"RF6_S_ANIMAL",RF6_S_ANIMAL);
 tolua_constant(tolua_S,"RF6_S_SPIDER",RF6_S_SPIDER);
 tolua_constant(tolua_S,"RF6_S_HOUND",RF6_S_HOUND);
 tolua_constant(tolua_S,"RF6_S_HYDRA",RF6_S_HYDRA);
 tolua_constant(tolua_S,"RF6_S_ANGEL",RF6_S_ANGEL);
 tolua_constant(tolua_S,"RF6_S_DEMON",RF6_S_DEMON);
 tolua_constant(tolua_S,"RF6_S_UNDEAD",RF6_S_UNDEAD);
 tolua_constant(tolua_S,"RF6_S_DRAGON",RF6_S_DRAGON);
 tolua_constant(tolua_S,"RF6_S_HI_UNDEAD",RF6_S_HI_UNDEAD);
 tolua_constant(tolua_S,"RF6_S_HI_DRAGON",RF6_S_HI_DRAGON);
 tolua_constant(tolua_S,"RF6_S_WRAITH",RF6_S_WRAITH);
 tolua_constant(tolua_S,"RF6_S_UNIQUE",RF6_S_UNIQUE);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"monster_blow","monster_blow","",tolua_collect_monster_blow);
#else
 tolua_cclass(tolua_S,"monster_blow","monster_blow","",NULL);
#endif
 tolua_beginmodule(tolua_S,"monster_blow");
 tolua_variable(tolua_S,"method",tolua_get_monster_blow_method,tolua_set_monster_blow_method);
 tolua_variable(tolua_S,"effect",tolua_get_monster_blow_effect,tolua_set_monster_blow_effect);
 tolua_variable(tolua_S,"d_dice",tolua_get_monster_blow_d_dice,tolua_set_monster_blow_d_dice);
 tolua_variable(tolua_S,"d_side",tolua_get_monster_blow_d_side,tolua_set_monster_blow_d_side);
 tolua_endmodule(tolua_S);
 tolua_cclass(tolua_S,"monster_race","monster_race","",NULL);
 tolua_beginmodule(tolua_S,"monster_race");
 tolua_variable(tolua_S,"name",tolua_get_monster_race_name,tolua_set_monster_race_name);
 tolua_variable(tolua_S,"text",tolua_get_monster_race_text,tolua_set_monster_race_text);
 tolua_variable(tolua_S,"hdice",tolua_get_monster_race_hdice,tolua_set_monster_race_hdice);
 tolua_variable(tolua_S,"hside",tolua_get_monster_race_hside,tolua_set_monster_race_hside);
 tolua_variable(tolua_S,"ac",tolua_get_monster_race_ac,tolua_set_monster_race_ac);
 tolua_variable(tolua_S,"sleep",tolua_get_monster_race_sleep,tolua_set_monster_race_sleep);
 tolua_variable(tolua_S,"aaf",tolua_get_monster_race_aaf,tolua_set_monster_race_aaf);
 tolua_variable(tolua_S,"speed",tolua_get_monster_race_speed,tolua_set_monster_race_speed);
 tolua_variable(tolua_S,"mexp",tolua_get_monster_race_mexp,tolua_set_monster_race_mexp);
 tolua_variable(tolua_S,"freq_innate",tolua_get_monster_race_freq_innate,tolua_set_monster_race_freq_innate);
 tolua_variable(tolua_S,"freq_spell",tolua_get_monster_race_freq_spell,tolua_set_monster_race_freq_spell);
 tolua_variable(tolua_S,"flags1",tolua_get_monster_race_flags1,tolua_set_monster_race_flags1);
 tolua_variable(tolua_S,"flags2",tolua_get_monster_race_flags2,tolua_set_monster_race_flags2);
 tolua_variable(tolua_S,"flags3",tolua_get_monster_race_flags3,tolua_set_monster_race_flags3);
 tolua_variable(tolua_S,"flags4",tolua_get_monster_race_flags4,tolua_set_monster_race_flags4);
 tolua_variable(tolua_S,"flags5",tolua_get_monster_race_flags5,tolua_set_monster_race_flags5);
 tolua_variable(tolua_S,"flags6",tolua_get_monster_race_flags6,tolua_set_monster_race_flags6);
 tolua_array(tolua_S,"blow",tolua_get_monster_monster_race_blow,tolua_set_monster_monster_race_blow);
 tolua_variable(tolua_S,"level",tolua_get_monster_race_level,tolua_set_monster_race_level);
 tolua_variable(tolua_S,"rarity",tolua_get_monster_race_rarity,tolua_set_monster_race_rarity);
 tolua_variable(tolua_S,"d_attr",tolua_get_monster_race_d_attr,tolua_set_monster_race_d_attr);
 tolua_variable(tolua_S,"d_char",tolua_get_monster_race_d_char,tolua_set_monster_race_d_char);
 tolua_variable(tolua_S,"x_attr",tolua_get_monster_race_x_attr,tolua_set_monster_race_x_attr);
 tolua_variable(tolua_S,"x_char",tolua_get_monster_race_x_char,tolua_set_monster_race_x_char);
 tolua_variable(tolua_S,"max_num",tolua_get_monster_race_max_num,tolua_set_monster_race_max_num);
 tolua_variable(tolua_S,"cur_num",tolua_get_monster_race_cur_num,tolua_set_monster_race_cur_num);
 tolua_endmodule(tolua_S);
 tolua_cclass(tolua_S,"monster_lore","monster_lore","",NULL);
 tolua_beginmodule(tolua_S,"monster_lore");
 tolua_variable(tolua_S,"sights",tolua_get_monster_lore_sights,tolua_set_monster_lore_sights);
 tolua_variable(tolua_S,"deaths",tolua_get_monster_lore_deaths,tolua_set_monster_lore_deaths);
 tolua_variable(tolua_S,"pkills",tolua_get_monster_lore_pkills,tolua_set_monster_lore_pkills);
 tolua_variable(tolua_S,"tkills",tolua_get_monster_lore_tkills,tolua_set_monster_lore_tkills);
 tolua_variable(tolua_S,"wake",tolua_get_monster_lore_wake,tolua_set_monster_lore_wake);
 tolua_variable(tolua_S,"ignore",tolua_get_monster_lore_ignore,tolua_set_monster_lore_ignore);
 tolua_variable(tolua_S,"drop_gold",tolua_get_monster_lore_drop_gold,tolua_set_monster_lore_drop_gold);
 tolua_variable(tolua_S,"drop_item",tolua_get_monster_lore_drop_item,tolua_set_monster_lore_drop_item);
 tolua_variable(tolua_S,"cast_innate",tolua_get_monster_lore_cast_innate,tolua_set_monster_lore_cast_innate);
 tolua_variable(tolua_S,"cast_spell",tolua_get_monster_lore_cast_spell,tolua_set_monster_lore_cast_spell);
 tolua_array(tolua_S,"blows",tolua_get_monster_monster_lore_blows,tolua_set_monster_monster_lore_blows);
 tolua_variable(tolua_S,"flags1",tolua_get_monster_lore_flags1,tolua_set_monster_lore_flags1);
 tolua_variable(tolua_S,"flags2",tolua_get_monster_lore_flags2,tolua_set_monster_lore_flags2);
 tolua_variable(tolua_S,"flags3",tolua_get_monster_lore_flags3,tolua_set_monster_lore_flags3);
 tolua_variable(tolua_S,"flags4",tolua_get_monster_lore_flags4,tolua_set_monster_lore_flags4);
 tolua_variable(tolua_S,"flags5",tolua_get_monster_lore_flags5,tolua_set_monster_lore_flags5);
 tolua_variable(tolua_S,"flags6",tolua_get_monster_lore_flags6,tolua_set_monster_lore_flags6);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"monster_type","monster_type","",tolua_collect_monster_type);
#else
 tolua_cclass(tolua_S,"monster_type","monster_type","",NULL);
#endif
 tolua_beginmodule(tolua_S,"monster_type");
 tolua_variable(tolua_S,"r_idx",tolua_get_monster_type_r_idx,tolua_set_monster_type_r_idx);
 tolua_variable(tolua_S,"fy",tolua_get_monster_type_fy,tolua_set_monster_type_fy);
 tolua_variable(tolua_S,"fx",tolua_get_monster_type_fx,tolua_set_monster_type_fx);
 tolua_variable(tolua_S,"hp",tolua_get_monster_type_hp,tolua_set_monster_type_hp);
 tolua_variable(tolua_S,"maxhp",tolua_get_monster_type_maxhp,tolua_set_monster_type_maxhp);
 tolua_variable(tolua_S,"csleep",tolua_get_monster_type_csleep,tolua_set_monster_type_csleep);
 tolua_variable(tolua_S,"mspeed",tolua_get_monster_type_mspeed,tolua_set_monster_type_mspeed);
 tolua_variable(tolua_S,"energy",tolua_get_monster_type_energy,tolua_set_monster_type_energy);
 tolua_variable(tolua_S,"stunned",tolua_get_monster_type_stunned,tolua_set_monster_type_stunned);
 tolua_variable(tolua_S,"confused",tolua_get_monster_type_confused,tolua_set_monster_type_confused);
 tolua_variable(tolua_S,"monfear",tolua_get_monster_type_monfear,tolua_set_monster_type_monfear);
 tolua_variable(tolua_S,"cdis",tolua_get_monster_type_cdis,tolua_set_monster_type_cdis);
 tolua_variable(tolua_S,"mflag",tolua_get_monster_type_mflag,tolua_set_monster_type_mflag);
 tolua_variable(tolua_S,"ml",tolua_get_monster_type_ml,tolua_set_monster_type_ml);
 tolua_variable(tolua_S,"hold_o_idx",tolua_get_monster_type_hold_o_idx,tolua_set_monster_type_hold_o_idx);
 tolua_endmodule(tolua_S);
 tolua_variable(tolua_S,"mon_max",tolua_get_mon_max,tolua_set_mon_max);
 tolua_variable(tolua_S,"mon_cnt",tolua_get_mon_cnt,tolua_set_mon_cnt);
 tolua_array(tolua_S,"mon_list",tolua_get_monster_mon_list,tolua_set_monster_mon_list);
 tolua_function(tolua_S,"make_attack_normal",tolua_monster_make_attack_normal00);
 tolua_function(tolua_S,"make_attack_spell",tolua_monster_make_attack_spell00);
 tolua_function(tolua_S,"process_monsters",tolua_monster_process_monsters00);
 tolua_function(tolua_S,"roff_top",tolua_monster_roff_top00);
 tolua_function(tolua_S,"screen_roff",tolua_monster_screen_roff00);
 tolua_function(tolua_S,"display_roff",tolua_monster_display_roff00);
 tolua_function(tolua_S,"delete_monster_idx",tolua_monster_delete_monster_idx00);
 tolua_function(tolua_S,"delete_monster",tolua_monster_delete_monster00);
 tolua_function(tolua_S,"compact_monsters",tolua_monster_compact_monsters00);
 tolua_function(tolua_S,"wipe_mon_list",tolua_monster_wipe_mon_list00);
 tolua_function(tolua_S,"mon_pop",tolua_monster_mon_pop00);
 tolua_function(tolua_S,"get_mon_num_prep",tolua_monster_get_mon_num_prep00);
 tolua_function(tolua_S,"get_mon_num",tolua_monster_get_mon_num00);
 tolua_function(tolua_S,"lore_do_probe",tolua_monster_lore_do_probe00);
 tolua_function(tolua_S,"lore_treasure",tolua_monster_lore_treasure00);
 tolua_function(tolua_S,"update_mon",tolua_monster_update_mon00);
 tolua_function(tolua_S,"update_monsters",tolua_monster_update_monsters00);
 tolua_function(tolua_S,"monster_carry",tolua_monster_monster_carry00);
 tolua_function(tolua_S,"monster_swap",tolua_monster_monster_swap00);
 tolua_function(tolua_S,"player_place",tolua_monster_player_place00);
 tolua_function(tolua_S,"monster_place",tolua_monster_monster_place00);
 tolua_function(tolua_S,"place_monster_aux",tolua_monster_place_monster_aux00);
 tolua_function(tolua_S,"place_monster",tolua_monster_place_monster00);
 tolua_function(tolua_S,"alloc_monster",tolua_monster_alloc_monster00);
 tolua_function(tolua_S,"summon_specific",tolua_monster_summon_specific00);
 tolua_function(tolua_S,"multiply_monster",tolua_monster_multiply_monster00);
 tolua_function(tolua_S,"message_pain",tolua_monster_message_pain00);
 tolua_function(tolua_S,"update_smart_learn",tolua_monster_update_smart_learn00);
 tolua_function(tolua_S,"monster_death",tolua_monster_monster_death00);
 tolua_function(tolua_S,"mon_take_hit",tolua_monster_mon_take_hit00);
{
	tolua_function(tolua_S, "monster_desc", tolua_monster_desc);
}
 tolua_endmodule(tolua_S);
 return 1;
}
