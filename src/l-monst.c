/*
** Lua binding: monster
** Generated automatically by tolua 4.0a - angband.
*/

#include "lua/tolua.h"

/* Exported function */
int tolua_monster_open (lua_State* tolua_S);
void tolua_monster_close (lua_State* tolua_S);

#include "angband.h"

/* function to register type */
static void toluaI_reg_types (lua_State* tolua_S)
{
(void)tolua_S; /* Hack - prevent compiler warnings */
 tolua_usertype(tolua_S,"size_t");
 tolua_usertype(tolua_S,"monster_type");
 tolua_usertype(tolua_S,"monster_blow");
 tolua_usertype(tolua_S,"object_type");
 tolua_usertype(tolua_S,"monster_race");
 tolua_usertype(tolua_S,"monster_lore");
}

/* error messages */
#define TOLUA_ERR_SELF tolua_error(tolua_S,"invalid 'self'")
#define TOLUA_ERR_ASSIGN tolua_error(tolua_S,"#vinvalid type in variable assignment.")

/* get function: method of class  monster_blow */
static int toluaI_get_monster_monster_blow_method(lua_State* tolua_S)
{
  monster_blow* self = (monster_blow*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->method);
 return 1;
}

/* set function: method of class  monster_blow */
static int toluaI_set_monster_monster_blow_method(lua_State* tolua_S)
{
  monster_blow* self = (monster_blow*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->method = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: effect of class  monster_blow */
static int toluaI_get_monster_monster_blow_effect(lua_State* tolua_S)
{
  monster_blow* self = (monster_blow*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->effect);
 return 1;
}

/* set function: effect of class  monster_blow */
static int toluaI_set_monster_monster_blow_effect(lua_State* tolua_S)
{
  monster_blow* self = (monster_blow*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->effect = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: d_dice of class  monster_blow */
static int toluaI_get_monster_monster_blow_d_dice(lua_State* tolua_S)
{
  monster_blow* self = (monster_blow*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->d_dice);
 return 1;
}

/* set function: d_dice of class  monster_blow */
static int toluaI_set_monster_monster_blow_d_dice(lua_State* tolua_S)
{
  monster_blow* self = (monster_blow*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->d_dice = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: d_side of class  monster_blow */
static int toluaI_get_monster_monster_blow_d_side(lua_State* tolua_S)
{
  monster_blow* self = (monster_blow*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->d_side);
 return 1;
}

/* set function: d_side of class  monster_blow */
static int toluaI_set_monster_monster_blow_d_side(lua_State* tolua_S)
{
  monster_blow* self = (monster_blow*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->d_side = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: name of class  monster_race */
static int toluaI_get_monster_monster_race_name(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->name);
 return 1;
}

/* set function: name of class  monster_race */
static int toluaI_set_monster_monster_race_name(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->name = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: text of class  monster_race */
static int toluaI_get_monster_monster_race_text(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->text);
 return 1;
}

/* set function: text of class  monster_race */
static int toluaI_set_monster_monster_race_text(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->text = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: hdice of class  monster_race */
static int toluaI_get_monster_monster_race_hdice(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->hdice);
 return 1;
}

/* set function: hdice of class  monster_race */
static int toluaI_set_monster_monster_race_hdice(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->hdice = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: hside of class  monster_race */
static int toluaI_get_monster_monster_race_hside(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->hside);
 return 1;
}

/* set function: hside of class  monster_race */
static int toluaI_set_monster_monster_race_hside(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->hside = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: ac of class  monster_race */
static int toluaI_get_monster_monster_race_ac(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->ac);
 return 1;
}

/* set function: ac of class  monster_race */
static int toluaI_set_monster_monster_race_ac(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->ac = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: sleep of class  monster_race */
static int toluaI_get_monster_monster_race_sleep(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->sleep);
 return 1;
}

/* set function: sleep of class  monster_race */
static int toluaI_set_monster_monster_race_sleep(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->sleep = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: aaf of class  monster_race */
static int toluaI_get_monster_monster_race_aaf(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->aaf);
 return 1;
}

/* set function: aaf of class  monster_race */
static int toluaI_set_monster_monster_race_aaf(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->aaf = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: speed of class  monster_race */
static int toluaI_get_monster_monster_race_speed(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->speed);
 return 1;
}

/* set function: speed of class  monster_race */
static int toluaI_set_monster_monster_race_speed(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->speed = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: mexp of class  monster_race */
static int toluaI_get_monster_monster_race_mexp(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->mexp);
 return 1;
}

/* set function: mexp of class  monster_race */
static int toluaI_set_monster_monster_race_mexp(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->mexp = ((s32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: freq_innate of class  monster_race */
static int toluaI_get_monster_monster_race_freq_innate(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->freq_innate);
 return 1;
}

/* set function: freq_innate of class  monster_race */
static int toluaI_set_monster_monster_race_freq_innate(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->freq_innate = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: freq_spell of class  monster_race */
static int toluaI_get_monster_monster_race_freq_spell(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->freq_spell);
 return 1;
}

/* set function: freq_spell of class  monster_race */
static int toluaI_set_monster_monster_race_freq_spell(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->freq_spell = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flags1 of class  monster_race */
static int toluaI_get_monster_monster_race_flags1(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags1);
 return 1;
}

/* set function: flags1 of class  monster_race */
static int toluaI_set_monster_monster_race_flags1(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags1 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flags2 of class  monster_race */
static int toluaI_get_monster_monster_race_flags2(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags2);
 return 1;
}

/* set function: flags2 of class  monster_race */
static int toluaI_set_monster_monster_race_flags2(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags2 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flags3 of class  monster_race */
static int toluaI_get_monster_monster_race_flags3(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags3);
 return 1;
}

/* set function: flags3 of class  monster_race */
static int toluaI_set_monster_monster_race_flags3(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags3 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flags4 of class  monster_race */
static int toluaI_get_monster_monster_race_flags4(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags4);
 return 1;
}

/* set function: flags4 of class  monster_race */
static int toluaI_set_monster_monster_race_flags4(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags4 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flags5 of class  monster_race */
static int toluaI_get_monster_monster_race_flags5(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags5);
 return 1;
}

/* set function: flags5 of class  monster_race */
static int toluaI_set_monster_monster_race_flags5(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags5 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flags6 of class  monster_race */
static int toluaI_get_monster_monster_race_flags6(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags6);
 return 1;
}

/* set function: flags6 of class  monster_race */
static int toluaI_set_monster_monster_race_flags6(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags6 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: blow of class  monster_race */
static int toluaI_get_monster_monster_race_blow(lua_State* tolua_S)
{
 int toluaI_index;
  monster_race* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (monster_race*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=MONSTER_BLOW_MAX)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushusertype(tolua_S,(void*)&self->blow[toluaI_index],tolua_tag(tolua_S,"monster_blow"));
 return 1;
}

/* set function: blow of class  monster_race */
static int toluaI_set_monster_monster_race_blow(lua_State* tolua_S)
{
 int toluaI_index;
  monster_race* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (monster_race*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=MONSTER_BLOW_MAX)
 tolua_error(tolua_S,"array indexing out of range.");
  self->blow[toluaI_index] = *((monster_blow*)  tolua_getusertype(tolua_S,3,0));
 return 0;
}

/* get function: level of class  monster_race */
static int toluaI_get_monster_monster_race_level(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->level);
 return 1;
}

/* set function: level of class  monster_race */
static int toluaI_set_monster_monster_race_level(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->level = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: rarity of class  monster_race */
static int toluaI_get_monster_monster_race_rarity(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->rarity);
 return 1;
}

/* set function: rarity of class  monster_race */
static int toluaI_set_monster_monster_race_rarity(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->rarity = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: d_attr of class  monster_race */
static int toluaI_get_monster_monster_race_d_attr(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->d_attr);
 return 1;
}

/* set function: d_attr of class  monster_race */
static int toluaI_set_monster_monster_race_d_attr(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->d_attr = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: d_char of class  monster_race */
static int toluaI_get_monster_monster_race_d_char(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->d_char);
 return 1;
}

/* set function: d_char of class  monster_race */
static int toluaI_set_monster_monster_race_d_char(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->d_char = ((char)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: x_attr of class  monster_race */
static int toluaI_get_monster_monster_race_x_attr(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->x_attr);
 return 1;
}

/* set function: x_attr of class  monster_race */
static int toluaI_set_monster_monster_race_x_attr(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->x_attr = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: x_char of class  monster_race */
static int toluaI_get_monster_monster_race_x_char(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->x_char);
 return 1;
}

/* set function: x_char of class  monster_race */
static int toluaI_set_monster_monster_race_x_char(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->x_char = ((char)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: max_num of class  monster_race */
static int toluaI_get_monster_monster_race_max_num(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->max_num);
 return 1;
}

/* set function: max_num of class  monster_race */
static int toluaI_set_monster_monster_race_max_num(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->max_num = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: cur_num of class  monster_race */
static int toluaI_get_monster_monster_race_cur_num(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->cur_num);
 return 1;
}

/* set function: cur_num of class  monster_race */
static int toluaI_set_monster_monster_race_cur_num(lua_State* tolua_S)
{
  monster_race* self = (monster_race*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->cur_num = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: sights of class  monster_lore */
static int toluaI_get_monster_monster_lore_sights(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->sights);
 return 1;
}

/* set function: sights of class  monster_lore */
static int toluaI_set_monster_monster_lore_sights(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->sights = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: deaths of class  monster_lore */
static int toluaI_get_monster_monster_lore_deaths(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->deaths);
 return 1;
}

/* set function: deaths of class  monster_lore */
static int toluaI_set_monster_monster_lore_deaths(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->deaths = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: pkills of class  monster_lore */
static int toluaI_get_monster_monster_lore_pkills(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->pkills);
 return 1;
}

/* set function: pkills of class  monster_lore */
static int toluaI_set_monster_monster_lore_pkills(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->pkills = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: tkills of class  monster_lore */
static int toluaI_get_monster_monster_lore_tkills(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->tkills);
 return 1;
}

/* set function: tkills of class  monster_lore */
static int toluaI_set_monster_monster_lore_tkills(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->tkills = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: wake of class  monster_lore */
static int toluaI_get_monster_monster_lore_wake(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->wake);
 return 1;
}

/* set function: wake of class  monster_lore */
static int toluaI_set_monster_monster_lore_wake(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->wake = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: ignore of class  monster_lore */
static int toluaI_get_monster_monster_lore_ignore(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->ignore);
 return 1;
}

/* set function: ignore of class  monster_lore */
static int toluaI_set_monster_monster_lore_ignore(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->ignore = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: drop_gold of class  monster_lore */
static int toluaI_get_monster_monster_lore_drop_gold(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->drop_gold);
 return 1;
}

/* set function: drop_gold of class  monster_lore */
static int toluaI_set_monster_monster_lore_drop_gold(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->drop_gold = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: drop_item of class  monster_lore */
static int toluaI_get_monster_monster_lore_drop_item(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->drop_item);
 return 1;
}

/* set function: drop_item of class  monster_lore */
static int toluaI_set_monster_monster_lore_drop_item(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->drop_item = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: cast_innate of class  monster_lore */
static int toluaI_get_monster_monster_lore_cast_innate(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->cast_innate);
 return 1;
}

/* set function: cast_innate of class  monster_lore */
static int toluaI_set_monster_monster_lore_cast_innate(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->cast_innate = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: cast_spell of class  monster_lore */
static int toluaI_get_monster_monster_lore_cast_spell(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->cast_spell);
 return 1;
}

/* set function: cast_spell of class  monster_lore */
static int toluaI_set_monster_monster_lore_cast_spell(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->cast_spell = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: blows of class  monster_lore */
static int toluaI_get_monster_monster_lore_blows(lua_State* tolua_S)
{
 int toluaI_index;
  monster_lore* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (monster_lore*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=MONSTER_BLOW_MAX)
 tolua_error(tolua_S,"array indexing out of range.");
 tolua_pushnumber(tolua_S,(long)self->blows[toluaI_index]);
 return 1;
}

/* set function: blows of class  monster_lore */
static int toluaI_set_monster_monster_lore_blows(lua_State* tolua_S)
{
 int toluaI_index;
  monster_lore* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (monster_lore*)  lua_touserdata(tolua_S,-1);
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 tolua_error(tolua_S,"invalid type in array indexing.");
 toluaI_index = (int)tolua_getnumber(tolua_S,2,0)-1;
 if (toluaI_index<0 || toluaI_index>=MONSTER_BLOW_MAX)
 tolua_error(tolua_S,"array indexing out of range.");
  self->blows[toluaI_index] = ((byte)  tolua_getnumber(tolua_S,3,0));
 return 0;
}

/* get function: flags1 of class  monster_lore */
static int toluaI_get_monster_monster_lore_flags1(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags1);
 return 1;
}

/* set function: flags1 of class  monster_lore */
static int toluaI_set_monster_monster_lore_flags1(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags1 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flags2 of class  monster_lore */
static int toluaI_get_monster_monster_lore_flags2(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags2);
 return 1;
}

/* set function: flags2 of class  monster_lore */
static int toluaI_set_monster_monster_lore_flags2(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags2 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flags3 of class  monster_lore */
static int toluaI_get_monster_monster_lore_flags3(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags3);
 return 1;
}

/* set function: flags3 of class  monster_lore */
static int toluaI_set_monster_monster_lore_flags3(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags3 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flags4 of class  monster_lore */
static int toluaI_get_monster_monster_lore_flags4(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags4);
 return 1;
}

/* set function: flags4 of class  monster_lore */
static int toluaI_set_monster_monster_lore_flags4(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags4 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flags5 of class  monster_lore */
static int toluaI_get_monster_monster_lore_flags5(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags5);
 return 1;
}

/* set function: flags5 of class  monster_lore */
static int toluaI_set_monster_monster_lore_flags5(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags5 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: flags6 of class  monster_lore */
static int toluaI_get_monster_monster_lore_flags6(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->flags6);
 return 1;
}

/* set function: flags6 of class  monster_lore */
static int toluaI_set_monster_monster_lore_flags6(lua_State* tolua_S)
{
  monster_lore* self = (monster_lore*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->flags6 = ((u32b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: r_idx of class  monster_type */
static int toluaI_get_monster_monster_type_r_idx(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->r_idx);
 return 1;
}

/* set function: r_idx of class  monster_type */
static int toluaI_set_monster_monster_type_r_idx(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->r_idx = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: fy of class  monster_type */
static int toluaI_get_monster_monster_type_fy(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->fy);
 return 1;
}

/* set function: fy of class  monster_type */
static int toluaI_set_monster_monster_type_fy(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->fy = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: fx of class  monster_type */
static int toluaI_get_monster_monster_type_fx(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->fx);
 return 1;
}

/* set function: fx of class  monster_type */
static int toluaI_set_monster_monster_type_fx(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->fx = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: hp of class  monster_type */
static int toluaI_get_monster_monster_type_hp(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->hp);
 return 1;
}

/* set function: hp of class  monster_type */
static int toluaI_set_monster_monster_type_hp(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->hp = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: maxhp of class  monster_type */
static int toluaI_get_monster_monster_type_maxhp(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->maxhp);
 return 1;
}

/* set function: maxhp of class  monster_type */
static int toluaI_set_monster_monster_type_maxhp(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->maxhp = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: csleep of class  monster_type */
static int toluaI_get_monster_monster_type_csleep(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->csleep);
 return 1;
}

/* set function: csleep of class  monster_type */
static int toluaI_set_monster_monster_type_csleep(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->csleep = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: mspeed of class  monster_type */
static int toluaI_get_monster_monster_type_mspeed(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->mspeed);
 return 1;
}

/* set function: mspeed of class  monster_type */
static int toluaI_set_monster_monster_type_mspeed(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->mspeed = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: energy of class  monster_type */
static int toluaI_get_monster_monster_type_energy(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->energy);
 return 1;
}

/* set function: energy of class  monster_type */
static int toluaI_set_monster_monster_type_energy(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->energy = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: stunned of class  monster_type */
static int toluaI_get_monster_monster_type_stunned(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->stunned);
 return 1;
}

/* set function: stunned of class  monster_type */
static int toluaI_set_monster_monster_type_stunned(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->stunned = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: confused of class  monster_type */
static int toluaI_get_monster_monster_type_confused(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->confused);
 return 1;
}

/* set function: confused of class  monster_type */
static int toluaI_set_monster_monster_type_confused(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->confused = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: monfear of class  monster_type */
static int toluaI_get_monster_monster_type_monfear(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->monfear);
 return 1;
}

/* set function: monfear of class  monster_type */
static int toluaI_set_monster_monster_type_monfear(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->monfear = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: cdis of class  monster_type */
static int toluaI_get_monster_monster_type_cdis(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->cdis);
 return 1;
}

/* set function: cdis of class  monster_type */
static int toluaI_set_monster_monster_type_cdis(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->cdis = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: mflag of class  monster_type */
static int toluaI_get_monster_monster_type_mflag(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->mflag);
 return 1;
}

/* set function: mflag of class  monster_type */
static int toluaI_set_monster_monster_type_mflag(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->mflag = ((byte)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: ml of class  monster_type */
static int toluaI_get_monster_monster_type_ml(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushbool(tolua_S,(int)self->ml);
 return 1;
}

/* set function: ml of class  monster_type */
static int toluaI_set_monster_monster_type_ml(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0))
 TOLUA_ERR_ASSIGN;
  self->ml = ((bool)  tolua_getbool(tolua_S,2,0));
 return 0;
}

/* get function: hold_o_idx of class  monster_type */
static int toluaI_get_monster_monster_type_hold_o_idx(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 tolua_pushnumber(tolua_S,(long)self->hold_o_idx);
 return 1;
}

/* set function: hold_o_idx of class  monster_type */
static int toluaI_set_monster_monster_type_hold_o_idx(lua_State* tolua_S)
{
  monster_type* self = (monster_type*)  tolua_getusertype(tolua_S,1,0);
 if (!self) TOLUA_ERR_SELF;
 if (!tolua_istype(tolua_S,2,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  self->hold_o_idx = ((s16b)  tolua_getnumber(tolua_S,2,0));
 return 0;
}

/* get function: mon_max */
static int toluaI_get_monster_mon_max(lua_State* tolua_S)
{
 tolua_pushnumber(tolua_S,(long)mon_max);
 return 1;
}

/* set function: mon_max */
static int toluaI_set_monster_mon_max(lua_State* tolua_S)
{
 if (!tolua_istype(tolua_S,1,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  mon_max = ((s16b)  tolua_getnumber(tolua_S,1,0));
 return 0;
}

/* get function: mon_cnt */
static int toluaI_get_monster_mon_cnt(lua_State* tolua_S)
{
 tolua_pushnumber(tolua_S,(long)mon_cnt);
 return 1;
}

/* set function: mon_cnt */
static int toluaI_set_monster_mon_cnt(lua_State* tolua_S)
{
 if (!tolua_istype(tolua_S,1,LUA_TNUMBER,0))
 TOLUA_ERR_ASSIGN;
  mon_cnt = ((s16b)  tolua_getnumber(tolua_S,1,0));
 return 0;
}

/* function: make_attack_normal */
static int toluaI_monster_make_attack_normal00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int m_idx = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  make_attack_normal(m_idx);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'make_attack_normal'.");
 return 0;
}

/* function: make_attack_spell */
static int toluaI_monster_make_attack_spell00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int m_idx = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  make_attack_spell(m_idx);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'make_attack_spell'.");
 return 0;
}

/* function: process_monsters */
static int toluaI_monster_process_monsters00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  byte minimum_energy = ((byte)  tolua_getnumber(tolua_S,1,0));
 {
  process_monsters(minimum_energy);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'process_monsters'.");
 return 0;
}

/* function: screen_roff */
static int toluaI_monster_screen_roff00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int r_idx = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  screen_roff(r_idx);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'screen_roff'.");
 return 0;
}

/* function: display_roff */
static int toluaI_monster_display_roff00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int r_idx = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  display_roff(r_idx);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'display_roff'.");
 return 0;
}

/* function: delete_monster_idx */
static int toluaI_monster_delete_monster_idx00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int i = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  delete_monster_idx(i);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete_monster_idx'.");
 return 0;
}

/* function: delete_monster */
static int toluaI_monster_delete_monster00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int y = ((int)  tolua_getnumber(tolua_S,1,0));
  int x = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  delete_monster(y,x);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete_monster'.");
 return 0;
}

/* function: compact_monsters */
static int toluaI_monster_compact_monsters00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int size = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  compact_monsters(size);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'compact_monsters'.");
 return 0;
}

/* function: wipe_mon_list */
static int toluaI_monster_wipe_mon_list00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  wipe_mon_list();
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'wipe_mon_list'.");
 return 0;
}

/* function: mon_pop */
static int toluaI_monster_mon_pop00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  s16b toluaI_ret = (s16b)  mon_pop();
 tolua_pushnumber(tolua_S,(long)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'mon_pop'.");
 return 0;
}

/* function: get_mon_num_prep */
static int toluaI_monster_get_mon_num_prep00(lua_State* tolua_S)
{
 if (
 !tolua_isnoobj(tolua_S,1)
 )
 goto tolua_lerror;
 else
 {
 {
  errr toluaI_ret = (errr)  get_mon_num_prep();
 tolua_pushnumber(tolua_S,(long)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'get_mon_num_prep'.");
 return 0;
}

/* function: get_mon_num */
static int toluaI_monster_get_mon_num00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int level = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  s16b toluaI_ret = (s16b)  get_mon_num(level);
 tolua_pushnumber(tolua_S,(long)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'get_mon_num'.");
 return 0;
}

/* function: monster_desc */
static int toluaI_monster_monster_desc00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TSTRING,0) ||
 !tolua_istype(tolua_S,2,tolua_tag(tolua_S,"size_t"),0) ||
 !tolua_istype(tolua_S,3,tolua_tag(tolua_S,"const monster_type"),0) ||
 !tolua_istype(tolua_S,4,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,5)
 )
 goto tolua_lerror;
 else
 {
  char* desc = ((char*)  tolua_getstring(tolua_S,1,0));
  size_t max = *((size_t*)  tolua_getusertype(tolua_S,2,0));
  const monster_type* m_ptr = ((const monster_type*)  tolua_getusertype(tolua_S,3,0));
  int mode = ((int)  tolua_getnumber(tolua_S,4,0));
 {
  monster_desc(desc,max,m_ptr,mode);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'monster_desc'.");
 return 0;
}

/* function: lore_do_probe */
static int toluaI_monster_lore_do_probe00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int m_idx = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  lore_do_probe(m_idx);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'lore_do_probe'.");
 return 0;
}

/* function: lore_treasure */
static int toluaI_monster_lore_treasure00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,4)
 )
 goto tolua_lerror;
 else
 {
  int m_idx = ((int)  tolua_getnumber(tolua_S,1,0));
  int num_item = ((int)  tolua_getnumber(tolua_S,2,0));
  int num_gold = ((int)  tolua_getnumber(tolua_S,3,0));
 {
  lore_treasure(m_idx,num_item,num_gold);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'lore_treasure'.");
 return 0;
}

/* function: update_mon */
static int toluaI_monster_update_mon00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int m_idx = ((int)  tolua_getnumber(tolua_S,1,0));
  bool full = ((bool)  tolua_getbool(tolua_S,2,0));
 {
  update_mon(m_idx,full);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'update_mon'.");
 return 0;
}

/* function: update_monsters */
static int toluaI_monster_update_monsters00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,tolua_tag(tolua_S,"bool"),0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  bool full = ((bool)  tolua_getbool(tolua_S,1,0));
 {
  update_monsters(full);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'update_monsters'.");
 return 0;
}

/* function: monster_carry */
static int toluaI_monster_monster_carry00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,tolua_tag(tolua_S,"object_type"),0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int m_idx = ((int)  tolua_getnumber(tolua_S,1,0));
  object_type* j_ptr = ((object_type*)  tolua_getusertype(tolua_S,2,0));
 {
  s16b toluaI_ret = (s16b)  monster_carry(m_idx,j_ptr);
 tolua_pushnumber(tolua_S,(long)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'monster_carry'.");
 return 0;
}

/* function: monster_swap */
static int toluaI_monster_monster_swap00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,4,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,5)
 )
 goto tolua_lerror;
 else
 {
  int y1 = ((int)  tolua_getnumber(tolua_S,1,0));
  int x1 = ((int)  tolua_getnumber(tolua_S,2,0));
  int y2 = ((int)  tolua_getnumber(tolua_S,3,0));
  int x2 = ((int)  tolua_getnumber(tolua_S,4,0));
 {
  monster_swap(y1,x1,y2,x2);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'monster_swap'.");
 return 0;
}

/* function: player_place */
static int toluaI_monster_player_place00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int y = ((int)  tolua_getnumber(tolua_S,1,0));
  int x = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  s16b toluaI_ret = (s16b)  player_place(y,x);
 tolua_pushnumber(tolua_S,(long)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'player_place'.");
 return 0;
}

/* function: monster_place */
static int toluaI_monster_monster_place00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,tolua_tag(tolua_S,"monster_type"),0) ||
 !tolua_isnoobj(tolua_S,4)
 )
 goto tolua_lerror;
 else
 {
  int y = ((int)  tolua_getnumber(tolua_S,1,0));
  int x = ((int)  tolua_getnumber(tolua_S,2,0));
  monster_type* n_ptr = ((monster_type*)  tolua_getusertype(tolua_S,3,0));
 {
  s16b toluaI_ret = (s16b)  monster_place(y,x,n_ptr);
 tolua_pushnumber(tolua_S,(long)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'monster_place'.");
 return 0;
}

/* function: place_monster_aux */
static int toluaI_monster_place_monster_aux00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,4,tolua_tag(tolua_S,"bool"),0) ||
 !tolua_istype(tolua_S,5,tolua_tag(tolua_S,"bool"),0) ||
 !tolua_isnoobj(tolua_S,6)
 )
 goto tolua_lerror;
 else
 {
  int y = ((int)  tolua_getnumber(tolua_S,1,0));
  int x = ((int)  tolua_getnumber(tolua_S,2,0));
  int r_idx = ((int)  tolua_getnumber(tolua_S,3,0));
  bool slp = ((bool)  tolua_getbool(tolua_S,4,0));
  bool grp = ((bool)  tolua_getbool(tolua_S,5,0));
 {
  bool toluaI_ret = (bool)  place_monster_aux(y,x,r_idx,slp,grp);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'place_monster_aux'.");
 return 0;
}

/* function: place_monster */
static int toluaI_monster_place_monster00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,tolua_tag(tolua_S,"bool"),0) ||
 !tolua_istype(tolua_S,4,tolua_tag(tolua_S,"bool"),0) ||
 !tolua_isnoobj(tolua_S,5)
 )
 goto tolua_lerror;
 else
 {
  int y = ((int)  tolua_getnumber(tolua_S,1,0));
  int x = ((int)  tolua_getnumber(tolua_S,2,0));
  bool slp = ((bool)  tolua_getbool(tolua_S,3,0));
  bool grp = ((bool)  tolua_getbool(tolua_S,4,0));
 {
  bool toluaI_ret = (bool)  place_monster(y,x,slp,grp);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'place_monster'.");
 return 0;
}

/* function: alloc_monster */
static int toluaI_monster_alloc_monster00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,tolua_tag(tolua_S,"bool"),0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int dis = ((int)  tolua_getnumber(tolua_S,1,0));
  bool slp = ((bool)  tolua_getbool(tolua_S,2,0));
 {
  bool toluaI_ret = (bool)  alloc_monster(dis,slp);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'alloc_monster'.");
 return 0;
}

/* function: summon_specific */
static int toluaI_monster_summon_specific00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,4,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,5)
 )
 goto tolua_lerror;
 else
 {
  int y1 = ((int)  tolua_getnumber(tolua_S,1,0));
  int x1 = ((int)  tolua_getnumber(tolua_S,2,0));
  int lev = ((int)  tolua_getnumber(tolua_S,3,0));
  int type = ((int)  tolua_getnumber(tolua_S,4,0));
 {
  bool toluaI_ret = (bool)  summon_specific(y1,x1,lev,type);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'summon_specific'.");
 return 0;
}

/* function: multiply_monster */
static int toluaI_monster_multiply_monster00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int m_idx = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  bool toluaI_ret = (bool)  multiply_monster(m_idx);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 }
 }
 return 1;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'multiply_monster'.");
 return 0;
}

/* function: message_pain */
static int toluaI_monster_message_pain00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int m_idx = ((int)  tolua_getnumber(tolua_S,1,0));
  int dam = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  message_pain(m_idx,dam);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'message_pain'.");
 return 0;
}

/* function: update_smart_learn */
static int toluaI_monster_update_smart_learn00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,3)
 )
 goto tolua_lerror;
 else
 {
  int m_idx = ((int)  tolua_getnumber(tolua_S,1,0));
  int what = ((int)  tolua_getnumber(tolua_S,2,0));
 {
  update_smart_learn(m_idx,what);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'update_smart_learn'.");
 return 0;
}

/* function: monster_death */
static int toluaI_monster_monster_death00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_isnoobj(tolua_S,2)
 )
 goto tolua_lerror;
 else
 {
  int m_idx = ((int)  tolua_getnumber(tolua_S,1,0));
 {
  monster_death(m_idx);
 }
 }
 return 0;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'monster_death'.");
 return 0;
}

/* function: mon_take_hit */
static int toluaI_monster_mon_take_hit00(lua_State* tolua_S)
{
 if (
 !tolua_istype(tolua_S,1,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,2,LUA_TNUMBER,0) ||
 !tolua_istype(tolua_S,3,tolua_tag(tolua_S,"bool"),0) ||
 !tolua_istype(tolua_S,4,LUA_TSTRING,0) ||
 !tolua_isnoobj(tolua_S,5)
 )
 goto tolua_lerror;
 else
 {
  int m_idx = ((int)  tolua_getnumber(tolua_S,1,0));
  int dam = ((int)  tolua_getnumber(tolua_S,2,0));
  bool fear = ((bool)  tolua_getbool(tolua_S,3,0));
  cptr note = ((cptr)  tolua_getstring(tolua_S,4,0));
 {
  bool toluaI_ret = (bool)  mon_take_hit(m_idx,dam,&fear,note);
 tolua_pushbool(tolua_S,(int)toluaI_ret);
 tolua_pushbool(tolua_S,(int)fear);
 }
 }
 return 2;
tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'mon_take_hit'.");
 return 0;
}

/* Open function */
int tolua_monster_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 toluaI_reg_types(tolua_S);
 tolua_constant(tolua_S,NULL,"RBM_HIT",RBM_HIT);
 tolua_constant(tolua_S,NULL,"RBM_TOUCH",RBM_TOUCH);
 tolua_constant(tolua_S,NULL,"RBM_PUNCH",RBM_PUNCH);
 tolua_constant(tolua_S,NULL,"RBM_KICK",RBM_KICK);
 tolua_constant(tolua_S,NULL,"RBM_CLAW",RBM_CLAW);
 tolua_constant(tolua_S,NULL,"RBM_BITE",RBM_BITE);
 tolua_constant(tolua_S,NULL,"RBM_STING",RBM_STING);
 tolua_constant(tolua_S,NULL,"RBM_XXX1",RBM_XXX1);
 tolua_constant(tolua_S,NULL,"RBM_BUTT",RBM_BUTT);
 tolua_constant(tolua_S,NULL,"RBM_CRUSH",RBM_CRUSH);
 tolua_constant(tolua_S,NULL,"RBM_ENGULF",RBM_ENGULF);
 tolua_constant(tolua_S,NULL,"RBM_XXX2",RBM_XXX2);
 tolua_constant(tolua_S,NULL,"RBM_CRAWL",RBM_CRAWL);
 tolua_constant(tolua_S,NULL,"RBM_DROOL",RBM_DROOL);
 tolua_constant(tolua_S,NULL,"RBM_SPIT",RBM_SPIT);
 tolua_constant(tolua_S,NULL,"RBM_XXX3",RBM_XXX3);
 tolua_constant(tolua_S,NULL,"RBM_GAZE",RBM_GAZE);
 tolua_constant(tolua_S,NULL,"RBM_WAIL",RBM_WAIL);
 tolua_constant(tolua_S,NULL,"RBM_SPORE",RBM_SPORE);
 tolua_constant(tolua_S,NULL,"RBM_XXX4",RBM_XXX4);
 tolua_constant(tolua_S,NULL,"RBM_BEG",RBM_BEG);
 tolua_constant(tolua_S,NULL,"RBM_INSULT",RBM_INSULT);
 tolua_constant(tolua_S,NULL,"RBM_MOAN",RBM_MOAN);
 tolua_constant(tolua_S,NULL,"RBM_XXX5",RBM_XXX5);
 tolua_constant(tolua_S,NULL,"RBE_HURT",RBE_HURT);
 tolua_constant(tolua_S,NULL,"RBE_POISON",RBE_POISON);
 tolua_constant(tolua_S,NULL,"RBE_UN_BONUS",RBE_UN_BONUS);
 tolua_constant(tolua_S,NULL,"RBE_UN_POWER",RBE_UN_POWER);
 tolua_constant(tolua_S,NULL,"RBE_EAT_GOLD",RBE_EAT_GOLD);
 tolua_constant(tolua_S,NULL,"RBE_EAT_ITEM",RBE_EAT_ITEM);
 tolua_constant(tolua_S,NULL,"RBE_EAT_FOOD",RBE_EAT_FOOD);
 tolua_constant(tolua_S,NULL,"RBE_EAT_LITE",RBE_EAT_LITE);
 tolua_constant(tolua_S,NULL,"RBE_ACID",RBE_ACID);
 tolua_constant(tolua_S,NULL,"RBE_ELEC",RBE_ELEC);
 tolua_constant(tolua_S,NULL,"RBE_FIRE",RBE_FIRE);
 tolua_constant(tolua_S,NULL,"RBE_COLD",RBE_COLD);
 tolua_constant(tolua_S,NULL,"RBE_BLIND",RBE_BLIND);
 tolua_constant(tolua_S,NULL,"RBE_CONFUSE",RBE_CONFUSE);
 tolua_constant(tolua_S,NULL,"RBE_TERRIFY",RBE_TERRIFY);
 tolua_constant(tolua_S,NULL,"RBE_PARALYZE",RBE_PARALYZE);
 tolua_constant(tolua_S,NULL,"RBE_LOSE_STR",RBE_LOSE_STR);
 tolua_constant(tolua_S,NULL,"RBE_LOSE_INT",RBE_LOSE_INT);
 tolua_constant(tolua_S,NULL,"RBE_LOSE_WIS",RBE_LOSE_WIS);
 tolua_constant(tolua_S,NULL,"RBE_LOSE_DEX",RBE_LOSE_DEX);
 tolua_constant(tolua_S,NULL,"RBE_LOSE_CON",RBE_LOSE_CON);
 tolua_constant(tolua_S,NULL,"RBE_LOSE_CHR",RBE_LOSE_CHR);
 tolua_constant(tolua_S,NULL,"RBE_LOSE_ALL",RBE_LOSE_ALL);
 tolua_constant(tolua_S,NULL,"RBE_SHATTER",RBE_SHATTER);
 tolua_constant(tolua_S,NULL,"RBE_EXP_10",RBE_EXP_10);
 tolua_constant(tolua_S,NULL,"RBE_EXP_20",RBE_EXP_20);
 tolua_constant(tolua_S,NULL,"RBE_EXP_40",RBE_EXP_40);
 tolua_constant(tolua_S,NULL,"RBE_EXP_80",RBE_EXP_80);
 tolua_constant(tolua_S,NULL,"RBE_HALLU",RBE_HALLU);
 tolua_constant(tolua_S,NULL,"SM_OPP_ACID",SM_OPP_ACID);
 tolua_constant(tolua_S,NULL,"SM_OPP_ELEC",SM_OPP_ELEC);
 tolua_constant(tolua_S,NULL,"SM_OPP_FIRE",SM_OPP_FIRE);
 tolua_constant(tolua_S,NULL,"SM_OPP_COLD",SM_OPP_COLD);
 tolua_constant(tolua_S,NULL,"SM_OPP_POIS",SM_OPP_POIS);
 tolua_constant(tolua_S,NULL,"SM_OPP_XXX1",SM_OPP_XXX1);
 tolua_constant(tolua_S,NULL,"SM_OPP_XXX2",SM_OPP_XXX2);
 tolua_constant(tolua_S,NULL,"SM_OPP_XXX3",SM_OPP_XXX3);
 tolua_constant(tolua_S,NULL,"SM_IMM_XXX5",SM_IMM_XXX5);
 tolua_constant(tolua_S,NULL,"SM_IMM_XXX6",SM_IMM_XXX6);
 tolua_constant(tolua_S,NULL,"SM_IMM_FREE",SM_IMM_FREE);
 tolua_constant(tolua_S,NULL,"SM_IMM_MANA",SM_IMM_MANA);
 tolua_constant(tolua_S,NULL,"SM_IMM_ACID",SM_IMM_ACID);
 tolua_constant(tolua_S,NULL,"SM_IMM_ELEC",SM_IMM_ELEC);
 tolua_constant(tolua_S,NULL,"SM_IMM_FIRE",SM_IMM_FIRE);
 tolua_constant(tolua_S,NULL,"SM_IMM_COLD",SM_IMM_COLD);
 tolua_constant(tolua_S,NULL,"SM_RES_ACID",SM_RES_ACID);
 tolua_constant(tolua_S,NULL,"SM_RES_ELEC",SM_RES_ELEC);
 tolua_constant(tolua_S,NULL,"SM_RES_FIRE",SM_RES_FIRE);
 tolua_constant(tolua_S,NULL,"SM_RES_COLD",SM_RES_COLD);
 tolua_constant(tolua_S,NULL,"SM_RES_POIS",SM_RES_POIS);
 tolua_constant(tolua_S,NULL,"SM_RES_FEAR",SM_RES_FEAR);
 tolua_constant(tolua_S,NULL,"SM_RES_LITE",SM_RES_LITE);
 tolua_constant(tolua_S,NULL,"SM_RES_DARK",SM_RES_DARK);
 tolua_constant(tolua_S,NULL,"SM_RES_BLIND",SM_RES_BLIND);
 tolua_constant(tolua_S,NULL,"SM_RES_CONFU",SM_RES_CONFU);
 tolua_constant(tolua_S,NULL,"SM_RES_SOUND",SM_RES_SOUND);
 tolua_constant(tolua_S,NULL,"SM_RES_SHARD",SM_RES_SHARD);
 tolua_constant(tolua_S,NULL,"SM_RES_NEXUS",SM_RES_NEXUS);
 tolua_constant(tolua_S,NULL,"SM_RES_NETHR",SM_RES_NETHR);
 tolua_constant(tolua_S,NULL,"SM_RES_CHAOS",SM_RES_CHAOS);
 tolua_constant(tolua_S,NULL,"SM_RES_DISEN",SM_RES_DISEN);
 tolua_constant(tolua_S,NULL,"MFLAG_VIEW",MFLAG_VIEW);
 tolua_constant(tolua_S,NULL,"MFLAG_NICE",MFLAG_NICE);
 tolua_constant(tolua_S,NULL,"MFLAG_SHOW",MFLAG_SHOW);
 tolua_constant(tolua_S,NULL,"MFLAG_MARK",MFLAG_MARK);
 tolua_constant(tolua_S,NULL,"RF1_UNIQUE",RF1_UNIQUE);
 tolua_constant(tolua_S,NULL,"RF1_QUESTOR",RF1_QUESTOR);
 tolua_constant(tolua_S,NULL,"RF1_MALE",RF1_MALE);
 tolua_constant(tolua_S,NULL,"RF1_FEMALE",RF1_FEMALE);
 tolua_constant(tolua_S,NULL,"RF1_CHAR_CLEAR",RF1_CHAR_CLEAR);
 tolua_constant(tolua_S,NULL,"RF1_CHAR_MULTI",RF1_CHAR_MULTI);
 tolua_constant(tolua_S,NULL,"RF1_ATTR_CLEAR",RF1_ATTR_CLEAR);
 tolua_constant(tolua_S,NULL,"RF1_ATTR_MULTI",RF1_ATTR_MULTI);
 tolua_constant(tolua_S,NULL,"RF1_FORCE_DEPTH",RF1_FORCE_DEPTH);
 tolua_constant(tolua_S,NULL,"RF1_FORCE_MAXHP",RF1_FORCE_MAXHP);
 tolua_constant(tolua_S,NULL,"RF1_FORCE_SLEEP",RF1_FORCE_SLEEP);
 tolua_constant(tolua_S,NULL,"RF1_FORCE_EXTRA",RF1_FORCE_EXTRA);
 tolua_constant(tolua_S,NULL,"RF1_FRIEND",RF1_FRIEND);
 tolua_constant(tolua_S,NULL,"RF1_FRIENDS",RF1_FRIENDS);
 tolua_constant(tolua_S,NULL,"RF1_ESCORT",RF1_ESCORT);
 tolua_constant(tolua_S,NULL,"RF1_ESCORTS",RF1_ESCORTS);
 tolua_constant(tolua_S,NULL,"RF1_NEVER_BLOW",RF1_NEVER_BLOW);
 tolua_constant(tolua_S,NULL,"RF1_NEVER_MOVE",RF1_NEVER_MOVE);
 tolua_constant(tolua_S,NULL,"RF1_RAND_25",RF1_RAND_25);
 tolua_constant(tolua_S,NULL,"RF1_RAND_50",RF1_RAND_50);
 tolua_constant(tolua_S,NULL,"RF1_ONLY_GOLD",RF1_ONLY_GOLD);
 tolua_constant(tolua_S,NULL,"RF1_ONLY_ITEM",RF1_ONLY_ITEM);
 tolua_constant(tolua_S,NULL,"RF1_DROP_60",RF1_DROP_60);
 tolua_constant(tolua_S,NULL,"RF1_DROP_90",RF1_DROP_90);
 tolua_constant(tolua_S,NULL,"RF1_DROP_1D2",RF1_DROP_1D2);
 tolua_constant(tolua_S,NULL,"RF1_DROP_2D2",RF1_DROP_2D2);
 tolua_constant(tolua_S,NULL,"RF1_DROP_3D2",RF1_DROP_3D2);
 tolua_constant(tolua_S,NULL,"RF1_DROP_4D2",RF1_DROP_4D2);
 tolua_constant(tolua_S,NULL,"RF1_DROP_GOOD",RF1_DROP_GOOD);
 tolua_constant(tolua_S,NULL,"RF1_DROP_GREAT",RF1_DROP_GREAT);
 tolua_constant(tolua_S,NULL,"RF1_DROP_USEFUL",RF1_DROP_USEFUL);
 tolua_constant(tolua_S,NULL,"RF1_DROP_CHOSEN",RF1_DROP_CHOSEN);
 tolua_constant(tolua_S,NULL,"RF2_STUPID",RF2_STUPID);
 tolua_constant(tolua_S,NULL,"RF2_SMART",RF2_SMART);
 tolua_constant(tolua_S,NULL,"RF2_XXX1",RF2_XXX1);
 tolua_constant(tolua_S,NULL,"RF2_XXX2",RF2_XXX2);
 tolua_constant(tolua_S,NULL,"RF2_INVISIBLE",RF2_INVISIBLE);
 tolua_constant(tolua_S,NULL,"RF2_COLD_BLOOD",RF2_COLD_BLOOD);
 tolua_constant(tolua_S,NULL,"RF2_EMPTY_MIND",RF2_EMPTY_MIND);
 tolua_constant(tolua_S,NULL,"RF2_WEIRD_MIND",RF2_WEIRD_MIND);
 tolua_constant(tolua_S,NULL,"RF2_MULTIPLY",RF2_MULTIPLY);
 tolua_constant(tolua_S,NULL,"RF2_REGENERATE",RF2_REGENERATE);
 tolua_constant(tolua_S,NULL,"RF2_XXX3",RF2_XXX3);
 tolua_constant(tolua_S,NULL,"RF2_XXX4",RF2_XXX4);
 tolua_constant(tolua_S,NULL,"RF2_POWERFUL",RF2_POWERFUL);
 tolua_constant(tolua_S,NULL,"RF2_XXX5",RF2_XXX5);
 tolua_constant(tolua_S,NULL,"RF2_XXX7",RF2_XXX7);
 tolua_constant(tolua_S,NULL,"RF2_XXX6",RF2_XXX6);
 tolua_constant(tolua_S,NULL,"RF2_OPEN_DOOR",RF2_OPEN_DOOR);
 tolua_constant(tolua_S,NULL,"RF2_BASH_DOOR",RF2_BASH_DOOR);
 tolua_constant(tolua_S,NULL,"RF2_PASS_WALL",RF2_PASS_WALL);
 tolua_constant(tolua_S,NULL,"RF2_KILL_WALL",RF2_KILL_WALL);
 tolua_constant(tolua_S,NULL,"RF2_MOVE_BODY",RF2_MOVE_BODY);
 tolua_constant(tolua_S,NULL,"RF2_KILL_BODY",RF2_KILL_BODY);
 tolua_constant(tolua_S,NULL,"RF2_TAKE_ITEM",RF2_TAKE_ITEM);
 tolua_constant(tolua_S,NULL,"RF2_KILL_ITEM",RF2_KILL_ITEM);
 tolua_constant(tolua_S,NULL,"RF2_BRAIN_1",RF2_BRAIN_1);
 tolua_constant(tolua_S,NULL,"RF2_BRAIN_2",RF2_BRAIN_2);
 tolua_constant(tolua_S,NULL,"RF2_BRAIN_3",RF2_BRAIN_3);
 tolua_constant(tolua_S,NULL,"RF2_BRAIN_4",RF2_BRAIN_4);
 tolua_constant(tolua_S,NULL,"RF2_BRAIN_5",RF2_BRAIN_5);
 tolua_constant(tolua_S,NULL,"RF2_BRAIN_6",RF2_BRAIN_6);
 tolua_constant(tolua_S,NULL,"RF2_BRAIN_7",RF2_BRAIN_7);
 tolua_constant(tolua_S,NULL,"RF2_BRAIN_8",RF2_BRAIN_8);
 tolua_constant(tolua_S,NULL,"RF3_ORC",RF3_ORC);
 tolua_constant(tolua_S,NULL,"RF3_TROLL",RF3_TROLL);
 tolua_constant(tolua_S,NULL,"RF3_GIANT",RF3_GIANT);
 tolua_constant(tolua_S,NULL,"RF3_DRAGON",RF3_DRAGON);
 tolua_constant(tolua_S,NULL,"RF3_DEMON",RF3_DEMON);
 tolua_constant(tolua_S,NULL,"RF3_UNDEAD",RF3_UNDEAD);
 tolua_constant(tolua_S,NULL,"RF3_EVIL",RF3_EVIL);
 tolua_constant(tolua_S,NULL,"RF3_ANIMAL",RF3_ANIMAL);
 tolua_constant(tolua_S,NULL,"RF3_XXX1",RF3_XXX1);
 tolua_constant(tolua_S,NULL,"RF3_XXX2",RF3_XXX2);
 tolua_constant(tolua_S,NULL,"RF3_XXX3",RF3_XXX3);
 tolua_constant(tolua_S,NULL,"RF3_XXX4",RF3_XXX4);
 tolua_constant(tolua_S,NULL,"RF3_HURT_LITE",RF3_HURT_LITE);
 tolua_constant(tolua_S,NULL,"RF3_HURT_ROCK",RF3_HURT_ROCK);
 tolua_constant(tolua_S,NULL,"RF3_HURT_FIRE",RF3_HURT_FIRE);
 tolua_constant(tolua_S,NULL,"RF3_HURT_COLD",RF3_HURT_COLD);
 tolua_constant(tolua_S,NULL,"RF3_IM_ACID",RF3_IM_ACID);
 tolua_constant(tolua_S,NULL,"RF3_IM_ELEC",RF3_IM_ELEC);
 tolua_constant(tolua_S,NULL,"RF3_IM_FIRE",RF3_IM_FIRE);
 tolua_constant(tolua_S,NULL,"RF3_IM_COLD",RF3_IM_COLD);
 tolua_constant(tolua_S,NULL,"RF3_IM_POIS",RF3_IM_POIS);
 tolua_constant(tolua_S,NULL,"RF3_XXX5",RF3_XXX5);
 tolua_constant(tolua_S,NULL,"RF3_RES_NETH",RF3_RES_NETH);
 tolua_constant(tolua_S,NULL,"RF3_RES_WATE",RF3_RES_WATE);
 tolua_constant(tolua_S,NULL,"RF3_RES_PLAS",RF3_RES_PLAS);
 tolua_constant(tolua_S,NULL,"RF3_RES_NEXU",RF3_RES_NEXU);
 tolua_constant(tolua_S,NULL,"RF3_RES_DISE",RF3_RES_DISE);
 tolua_constant(tolua_S,NULL,"RF3_XXX6",RF3_XXX6);
 tolua_constant(tolua_S,NULL,"RF3_NO_FEAR",RF3_NO_FEAR);
 tolua_constant(tolua_S,NULL,"RF3_NO_STUN",RF3_NO_STUN);
 tolua_constant(tolua_S,NULL,"RF3_NO_CONF",RF3_NO_CONF);
 tolua_constant(tolua_S,NULL,"RF3_NO_SLEEP",RF3_NO_SLEEP);
 tolua_constant(tolua_S,NULL,"RF4_SHRIEK",RF4_SHRIEK);
 tolua_constant(tolua_S,NULL,"RF4_XXX2",RF4_XXX2);
 tolua_constant(tolua_S,NULL,"RF4_XXX3",RF4_XXX3);
 tolua_constant(tolua_S,NULL,"RF4_XXX4",RF4_XXX4);
 tolua_constant(tolua_S,NULL,"RF4_ARROW_1",RF4_ARROW_1);
 tolua_constant(tolua_S,NULL,"RF4_ARROW_2",RF4_ARROW_2);
 tolua_constant(tolua_S,NULL,"RF4_ARROW_3",RF4_ARROW_3);
 tolua_constant(tolua_S,NULL,"RF4_ARROW_4",RF4_ARROW_4);
 tolua_constant(tolua_S,NULL,"RF4_BR_ACID",RF4_BR_ACID);
 tolua_constant(tolua_S,NULL,"RF4_BR_ELEC",RF4_BR_ELEC);
 tolua_constant(tolua_S,NULL,"RF4_BR_FIRE",RF4_BR_FIRE);
 tolua_constant(tolua_S,NULL,"RF4_BR_COLD",RF4_BR_COLD);
 tolua_constant(tolua_S,NULL,"RF4_BR_POIS",RF4_BR_POIS);
 tolua_constant(tolua_S,NULL,"RF4_BR_NETH",RF4_BR_NETH);
 tolua_constant(tolua_S,NULL,"RF4_BR_LITE",RF4_BR_LITE);
 tolua_constant(tolua_S,NULL,"RF4_BR_DARK",RF4_BR_DARK);
 tolua_constant(tolua_S,NULL,"RF4_BR_CONF",RF4_BR_CONF);
 tolua_constant(tolua_S,NULL,"RF4_BR_SOUN",RF4_BR_SOUN);
 tolua_constant(tolua_S,NULL,"RF4_BR_CHAO",RF4_BR_CHAO);
 tolua_constant(tolua_S,NULL,"RF4_BR_DISE",RF4_BR_DISE);
 tolua_constant(tolua_S,NULL,"RF4_BR_NEXU",RF4_BR_NEXU);
 tolua_constant(tolua_S,NULL,"RF4_BR_TIME",RF4_BR_TIME);
 tolua_constant(tolua_S,NULL,"RF4_BR_INER",RF4_BR_INER);
 tolua_constant(tolua_S,NULL,"RF4_BR_GRAV",RF4_BR_GRAV);
 tolua_constant(tolua_S,NULL,"RF4_BR_SHAR",RF4_BR_SHAR);
 tolua_constant(tolua_S,NULL,"RF4_BR_PLAS",RF4_BR_PLAS);
 tolua_constant(tolua_S,NULL,"RF4_BR_WALL",RF4_BR_WALL);
 tolua_constant(tolua_S,NULL,"RF4_BR_MANA",RF4_BR_MANA);
 tolua_constant(tolua_S,NULL,"RF4_XXX5",RF4_XXX5);
 tolua_constant(tolua_S,NULL,"RF4_XXX6",RF4_XXX6);
 tolua_constant(tolua_S,NULL,"RF4_XXX7",RF4_XXX7);
 tolua_constant(tolua_S,NULL,"RF4_BOULDER",RF4_BOULDER);
 tolua_constant(tolua_S,NULL,"RF5_BA_ACID",RF5_BA_ACID);
 tolua_constant(tolua_S,NULL,"RF5_BA_ELEC",RF5_BA_ELEC);
 tolua_constant(tolua_S,NULL,"RF5_BA_FIRE",RF5_BA_FIRE);
 tolua_constant(tolua_S,NULL,"RF5_BA_COLD",RF5_BA_COLD);
 tolua_constant(tolua_S,NULL,"RF5_BA_POIS",RF5_BA_POIS);
 tolua_constant(tolua_S,NULL,"RF5_BA_NETH",RF5_BA_NETH);
 tolua_constant(tolua_S,NULL,"RF5_BA_WATE",RF5_BA_WATE);
 tolua_constant(tolua_S,NULL,"RF5_BA_MANA",RF5_BA_MANA);
 tolua_constant(tolua_S,NULL,"RF5_BA_DARK",RF5_BA_DARK);
 tolua_constant(tolua_S,NULL,"RF5_DRAIN_MANA",RF5_DRAIN_MANA);
 tolua_constant(tolua_S,NULL,"RF5_MIND_BLAST",RF5_MIND_BLAST);
 tolua_constant(tolua_S,NULL,"RF5_BRAIN_SMASH",RF5_BRAIN_SMASH);
 tolua_constant(tolua_S,NULL,"RF5_CAUSE_1",RF5_CAUSE_1);
 tolua_constant(tolua_S,NULL,"RF5_CAUSE_2",RF5_CAUSE_2);
 tolua_constant(tolua_S,NULL,"RF5_CAUSE_3",RF5_CAUSE_3);
 tolua_constant(tolua_S,NULL,"RF5_CAUSE_4",RF5_CAUSE_4);
 tolua_constant(tolua_S,NULL,"RF5_BO_ACID",RF5_BO_ACID);
 tolua_constant(tolua_S,NULL,"RF5_BO_ELEC",RF5_BO_ELEC);
 tolua_constant(tolua_S,NULL,"RF5_BO_FIRE",RF5_BO_FIRE);
 tolua_constant(tolua_S,NULL,"RF5_BO_COLD",RF5_BO_COLD);
 tolua_constant(tolua_S,NULL,"RF5_BO_POIS",RF5_BO_POIS);
 tolua_constant(tolua_S,NULL,"RF5_BO_NETH",RF5_BO_NETH);
 tolua_constant(tolua_S,NULL,"RF5_BO_WATE",RF5_BO_WATE);
 tolua_constant(tolua_S,NULL,"RF5_BO_MANA",RF5_BO_MANA);
 tolua_constant(tolua_S,NULL,"RF5_BO_PLAS",RF5_BO_PLAS);
 tolua_constant(tolua_S,NULL,"RF5_BO_ICEE",RF5_BO_ICEE);
 tolua_constant(tolua_S,NULL,"RF5_MISSILE",RF5_MISSILE);
 tolua_constant(tolua_S,NULL,"RF5_SCARE",RF5_SCARE);
 tolua_constant(tolua_S,NULL,"RF5_BLIND",RF5_BLIND);
 tolua_constant(tolua_S,NULL,"RF5_CONF",RF5_CONF);
 tolua_constant(tolua_S,NULL,"RF5_SLOW",RF5_SLOW);
 tolua_constant(tolua_S,NULL,"RF5_HOLD",RF5_HOLD);
 tolua_constant(tolua_S,NULL,"RF6_HASTE",RF6_HASTE);
 tolua_constant(tolua_S,NULL,"RF6_XXX1",RF6_XXX1);
 tolua_constant(tolua_S,NULL,"RF6_HEAL",RF6_HEAL);
 tolua_constant(tolua_S,NULL,"RF6_XXX2",RF6_XXX2);
 tolua_constant(tolua_S,NULL,"RF6_BLINK",RF6_BLINK);
 tolua_constant(tolua_S,NULL,"RF6_TPORT",RF6_TPORT);
 tolua_constant(tolua_S,NULL,"RF6_XXX3",RF6_XXX3);
 tolua_constant(tolua_S,NULL,"RF6_XXX4",RF6_XXX4);
 tolua_constant(tolua_S,NULL,"RF6_TELE_TO",RF6_TELE_TO);
 tolua_constant(tolua_S,NULL,"RF6_TELE_AWAY",RF6_TELE_AWAY);
 tolua_constant(tolua_S,NULL,"RF6_TELE_LEVEL",RF6_TELE_LEVEL);
 tolua_constant(tolua_S,NULL,"RF6_XXX5",RF6_XXX5);
 tolua_constant(tolua_S,NULL,"RF6_DARKNESS",RF6_DARKNESS);
 tolua_constant(tolua_S,NULL,"RF6_TRAPS",RF6_TRAPS);
 tolua_constant(tolua_S,NULL,"RF6_FORGET",RF6_FORGET);
 tolua_constant(tolua_S,NULL,"RF6_XXX6",RF6_XXX6);
 tolua_constant(tolua_S,NULL,"RF6_S_KIN",RF6_S_KIN);
 tolua_constant(tolua_S,NULL,"RF6_S_HI_DEMON",RF6_S_HI_DEMON);
 tolua_constant(tolua_S,NULL,"RF6_S_MONSTER",RF6_S_MONSTER);
 tolua_constant(tolua_S,NULL,"RF6_S_MONSTERS",RF6_S_MONSTERS);
 tolua_constant(tolua_S,NULL,"RF6_S_ANIMAL",RF6_S_ANIMAL);
 tolua_constant(tolua_S,NULL,"RF6_S_SPIDER",RF6_S_SPIDER);
 tolua_constant(tolua_S,NULL,"RF6_S_HOUND",RF6_S_HOUND);
 tolua_constant(tolua_S,NULL,"RF6_S_HYDRA",RF6_S_HYDRA);
 tolua_constant(tolua_S,NULL,"RF6_S_ANGEL",RF6_S_ANGEL);
 tolua_constant(tolua_S,NULL,"RF6_S_DEMON",RF6_S_DEMON);
 tolua_constant(tolua_S,NULL,"RF6_S_UNDEAD",RF6_S_UNDEAD);
 tolua_constant(tolua_S,NULL,"RF6_S_DRAGON",RF6_S_DRAGON);
 tolua_constant(tolua_S,NULL,"RF6_S_HI_UNDEAD",RF6_S_HI_UNDEAD);
 tolua_constant(tolua_S,NULL,"RF6_S_HI_DRAGON",RF6_S_HI_DRAGON);
 tolua_constant(tolua_S,NULL,"RF6_S_WRAITH",RF6_S_WRAITH);
 tolua_constant(tolua_S,NULL,"RF6_S_UNIQUE",RF6_S_UNIQUE);
 tolua_cclass(tolua_S,"monster_blow","");
 tolua_tablevar(tolua_S,"monster_blow","method",toluaI_get_monster_monster_blow_method,toluaI_set_monster_monster_blow_method);
 tolua_tablevar(tolua_S,"monster_blow","effect",toluaI_get_monster_monster_blow_effect,toluaI_set_monster_monster_blow_effect);
 tolua_tablevar(tolua_S,"monster_blow","d_dice",toluaI_get_monster_monster_blow_d_dice,toluaI_set_monster_monster_blow_d_dice);
 tolua_tablevar(tolua_S,"monster_blow","d_side",toluaI_get_monster_monster_blow_d_side,toluaI_set_monster_monster_blow_d_side);
 tolua_cclass(tolua_S,"monster_race","");
 tolua_tablevar(tolua_S,"monster_race","name",toluaI_get_monster_monster_race_name,toluaI_set_monster_monster_race_name);
 tolua_tablevar(tolua_S,"monster_race","text",toluaI_get_monster_monster_race_text,toluaI_set_monster_monster_race_text);
 tolua_tablevar(tolua_S,"monster_race","hdice",toluaI_get_monster_monster_race_hdice,toluaI_set_monster_monster_race_hdice);
 tolua_tablevar(tolua_S,"monster_race","hside",toluaI_get_monster_monster_race_hside,toluaI_set_monster_monster_race_hside);
 tolua_tablevar(tolua_S,"monster_race","ac",toluaI_get_monster_monster_race_ac,toluaI_set_monster_monster_race_ac);
 tolua_tablevar(tolua_S,"monster_race","sleep",toluaI_get_monster_monster_race_sleep,toluaI_set_monster_monster_race_sleep);
 tolua_tablevar(tolua_S,"monster_race","aaf",toluaI_get_monster_monster_race_aaf,toluaI_set_monster_monster_race_aaf);
 tolua_tablevar(tolua_S,"monster_race","speed",toluaI_get_monster_monster_race_speed,toluaI_set_monster_monster_race_speed);
 tolua_tablevar(tolua_S,"monster_race","mexp",toluaI_get_monster_monster_race_mexp,toluaI_set_monster_monster_race_mexp);
 tolua_tablevar(tolua_S,"monster_race","freq_innate",toluaI_get_monster_monster_race_freq_innate,toluaI_set_monster_monster_race_freq_innate);
 tolua_tablevar(tolua_S,"monster_race","freq_spell",toluaI_get_monster_monster_race_freq_spell,toluaI_set_monster_monster_race_freq_spell);
 tolua_tablevar(tolua_S,"monster_race","flags1",toluaI_get_monster_monster_race_flags1,toluaI_set_monster_monster_race_flags1);
 tolua_tablevar(tolua_S,"monster_race","flags2",toluaI_get_monster_monster_race_flags2,toluaI_set_monster_monster_race_flags2);
 tolua_tablevar(tolua_S,"monster_race","flags3",toluaI_get_monster_monster_race_flags3,toluaI_set_monster_monster_race_flags3);
 tolua_tablevar(tolua_S,"monster_race","flags4",toluaI_get_monster_monster_race_flags4,toluaI_set_monster_monster_race_flags4);
 tolua_tablevar(tolua_S,"monster_race","flags5",toluaI_get_monster_monster_race_flags5,toluaI_set_monster_monster_race_flags5);
 tolua_tablevar(tolua_S,"monster_race","flags6",toluaI_get_monster_monster_race_flags6,toluaI_set_monster_monster_race_flags6);
 tolua_tablearray(tolua_S,"monster_race","blow",toluaI_get_monster_monster_race_blow,toluaI_set_monster_monster_race_blow);
 tolua_tablevar(tolua_S,"monster_race","level",toluaI_get_monster_monster_race_level,toluaI_set_monster_monster_race_level);
 tolua_tablevar(tolua_S,"monster_race","rarity",toluaI_get_monster_monster_race_rarity,toluaI_set_monster_monster_race_rarity);
 tolua_tablevar(tolua_S,"monster_race","d_attr",toluaI_get_monster_monster_race_d_attr,toluaI_set_monster_monster_race_d_attr);
 tolua_tablevar(tolua_S,"monster_race","d_char",toluaI_get_monster_monster_race_d_char,toluaI_set_monster_monster_race_d_char);
 tolua_tablevar(tolua_S,"monster_race","x_attr",toluaI_get_monster_monster_race_x_attr,toluaI_set_monster_monster_race_x_attr);
 tolua_tablevar(tolua_S,"monster_race","x_char",toluaI_get_monster_monster_race_x_char,toluaI_set_monster_monster_race_x_char);
 tolua_tablevar(tolua_S,"monster_race","max_num",toluaI_get_monster_monster_race_max_num,toluaI_set_monster_monster_race_max_num);
 tolua_tablevar(tolua_S,"monster_race","cur_num",toluaI_get_monster_monster_race_cur_num,toluaI_set_monster_monster_race_cur_num);
 tolua_cclass(tolua_S,"monster_lore","");
 tolua_tablevar(tolua_S,"monster_lore","sights",toluaI_get_monster_monster_lore_sights,toluaI_set_monster_monster_lore_sights);
 tolua_tablevar(tolua_S,"monster_lore","deaths",toluaI_get_monster_monster_lore_deaths,toluaI_set_monster_monster_lore_deaths);
 tolua_tablevar(tolua_S,"monster_lore","pkills",toluaI_get_monster_monster_lore_pkills,toluaI_set_monster_monster_lore_pkills);
 tolua_tablevar(tolua_S,"monster_lore","tkills",toluaI_get_monster_monster_lore_tkills,toluaI_set_monster_monster_lore_tkills);
 tolua_tablevar(tolua_S,"monster_lore","wake",toluaI_get_monster_monster_lore_wake,toluaI_set_monster_monster_lore_wake);
 tolua_tablevar(tolua_S,"monster_lore","ignore",toluaI_get_monster_monster_lore_ignore,toluaI_set_monster_monster_lore_ignore);
 tolua_tablevar(tolua_S,"monster_lore","drop_gold",toluaI_get_monster_monster_lore_drop_gold,toluaI_set_monster_monster_lore_drop_gold);
 tolua_tablevar(tolua_S,"monster_lore","drop_item",toluaI_get_monster_monster_lore_drop_item,toluaI_set_monster_monster_lore_drop_item);
 tolua_tablevar(tolua_S,"monster_lore","cast_innate",toluaI_get_monster_monster_lore_cast_innate,toluaI_set_monster_monster_lore_cast_innate);
 tolua_tablevar(tolua_S,"monster_lore","cast_spell",toluaI_get_monster_monster_lore_cast_spell,toluaI_set_monster_monster_lore_cast_spell);
 tolua_tablearray(tolua_S,"monster_lore","blows",toluaI_get_monster_monster_lore_blows,toluaI_set_monster_monster_lore_blows);
 tolua_tablevar(tolua_S,"monster_lore","flags1",toluaI_get_monster_monster_lore_flags1,toluaI_set_monster_monster_lore_flags1);
 tolua_tablevar(tolua_S,"monster_lore","flags2",toluaI_get_monster_monster_lore_flags2,toluaI_set_monster_monster_lore_flags2);
 tolua_tablevar(tolua_S,"monster_lore","flags3",toluaI_get_monster_monster_lore_flags3,toluaI_set_monster_monster_lore_flags3);
 tolua_tablevar(tolua_S,"monster_lore","flags4",toluaI_get_monster_monster_lore_flags4,toluaI_set_monster_monster_lore_flags4);
 tolua_tablevar(tolua_S,"monster_lore","flags5",toluaI_get_monster_monster_lore_flags5,toluaI_set_monster_monster_lore_flags5);
 tolua_tablevar(tolua_S,"monster_lore","flags6",toluaI_get_monster_monster_lore_flags6,toluaI_set_monster_monster_lore_flags6);
 tolua_cclass(tolua_S,"monster_type","");
 tolua_tablevar(tolua_S,"monster_type","r_idx",toluaI_get_monster_monster_type_r_idx,toluaI_set_monster_monster_type_r_idx);
 tolua_tablevar(tolua_S,"monster_type","fy",toluaI_get_monster_monster_type_fy,toluaI_set_monster_monster_type_fy);
 tolua_tablevar(tolua_S,"monster_type","fx",toluaI_get_monster_monster_type_fx,toluaI_set_monster_monster_type_fx);
 tolua_tablevar(tolua_S,"monster_type","hp",toluaI_get_monster_monster_type_hp,toluaI_set_monster_monster_type_hp);
 tolua_tablevar(tolua_S,"monster_type","maxhp",toluaI_get_monster_monster_type_maxhp,toluaI_set_monster_monster_type_maxhp);
 tolua_tablevar(tolua_S,"monster_type","csleep",toluaI_get_monster_monster_type_csleep,toluaI_set_monster_monster_type_csleep);
 tolua_tablevar(tolua_S,"monster_type","mspeed",toluaI_get_monster_monster_type_mspeed,toluaI_set_monster_monster_type_mspeed);
 tolua_tablevar(tolua_S,"monster_type","energy",toluaI_get_monster_monster_type_energy,toluaI_set_monster_monster_type_energy);
 tolua_tablevar(tolua_S,"monster_type","stunned",toluaI_get_monster_monster_type_stunned,toluaI_set_monster_monster_type_stunned);
 tolua_tablevar(tolua_S,"monster_type","confused",toluaI_get_monster_monster_type_confused,toluaI_set_monster_monster_type_confused);
 tolua_tablevar(tolua_S,"monster_type","monfear",toluaI_get_monster_monster_type_monfear,toluaI_set_monster_monster_type_monfear);
 tolua_tablevar(tolua_S,"monster_type","cdis",toluaI_get_monster_monster_type_cdis,toluaI_set_monster_monster_type_cdis);
 tolua_tablevar(tolua_S,"monster_type","mflag",toluaI_get_monster_monster_type_mflag,toluaI_set_monster_monster_type_mflag);
 tolua_tablevar(tolua_S,"monster_type","ml",toluaI_get_monster_monster_type_ml,toluaI_set_monster_monster_type_ml);
 tolua_tablevar(tolua_S,"monster_type","hold_o_idx",toluaI_get_monster_monster_type_hold_o_idx,toluaI_set_monster_monster_type_hold_o_idx);
 tolua_globalvar(tolua_S,"mon_max",toluaI_get_monster_mon_max,toluaI_set_monster_mon_max);
 tolua_globalvar(tolua_S,"mon_cnt",toluaI_get_monster_mon_cnt,toluaI_set_monster_mon_cnt);
 tolua_function(tolua_S,NULL,"make_attack_normal",toluaI_monster_make_attack_normal00);
 tolua_function(tolua_S,NULL,"make_attack_spell",toluaI_monster_make_attack_spell00);
 tolua_function(tolua_S,NULL,"process_monsters",toluaI_monster_process_monsters00);
 tolua_function(tolua_S,NULL,"screen_roff",toluaI_monster_screen_roff00);
 tolua_function(tolua_S,NULL,"display_roff",toluaI_monster_display_roff00);
 tolua_function(tolua_S,NULL,"delete_monster_idx",toluaI_monster_delete_monster_idx00);
 tolua_function(tolua_S,NULL,"delete_monster",toluaI_monster_delete_monster00);
 tolua_function(tolua_S,NULL,"compact_monsters",toluaI_monster_compact_monsters00);
 tolua_function(tolua_S,NULL,"wipe_mon_list",toluaI_monster_wipe_mon_list00);
 tolua_function(tolua_S,NULL,"mon_pop",toluaI_monster_mon_pop00);
 tolua_function(tolua_S,NULL,"get_mon_num_prep",toluaI_monster_get_mon_num_prep00);
 tolua_function(tolua_S,NULL,"get_mon_num",toluaI_monster_get_mon_num00);
 tolua_function(tolua_S,NULL,"monster_desc",toluaI_monster_monster_desc00);
 tolua_function(tolua_S,NULL,"lore_do_probe",toluaI_monster_lore_do_probe00);
 tolua_function(tolua_S,NULL,"lore_treasure",toluaI_monster_lore_treasure00);
 tolua_function(tolua_S,NULL,"update_mon",toluaI_monster_update_mon00);
 tolua_function(tolua_S,NULL,"update_monsters",toluaI_monster_update_monsters00);
 tolua_function(tolua_S,NULL,"monster_carry",toluaI_monster_monster_carry00);
 tolua_function(tolua_S,NULL,"monster_swap",toluaI_monster_monster_swap00);
 tolua_function(tolua_S,NULL,"player_place",toluaI_monster_player_place00);
 tolua_function(tolua_S,NULL,"monster_place",toluaI_monster_monster_place00);
 tolua_function(tolua_S,NULL,"place_monster_aux",toluaI_monster_place_monster_aux00);
 tolua_function(tolua_S,NULL,"place_monster",toluaI_monster_place_monster00);
 tolua_function(tolua_S,NULL,"alloc_monster",toluaI_monster_alloc_monster00);
 tolua_function(tolua_S,NULL,"summon_specific",toluaI_monster_summon_specific00);
 tolua_function(tolua_S,NULL,"multiply_monster",toluaI_monster_multiply_monster00);
 tolua_function(tolua_S,NULL,"message_pain",toluaI_monster_message_pain00);
 tolua_function(tolua_S,NULL,"update_smart_learn",toluaI_monster_update_smart_learn00);
 tolua_function(tolua_S,NULL,"monster_death",toluaI_monster_monster_death00);
 tolua_function(tolua_S,NULL,"mon_take_hit",toluaI_monster_mon_take_hit00);
 return 1;
}
/* Close function */
void tolua_monster_close (lua_State* tolua_S)
{
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBM_HIT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBM_TOUCH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBM_PUNCH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBM_KICK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBM_CLAW");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBM_BITE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBM_STING");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBM_XXX1");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBM_BUTT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBM_CRUSH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBM_ENGULF");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBM_XXX2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBM_CRAWL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBM_DROOL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBM_SPIT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBM_XXX3");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBM_GAZE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBM_WAIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBM_SPORE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBM_XXX4");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBM_BEG");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBM_INSULT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBM_MOAN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBM_XXX5");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_HURT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_POISON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_UN_BONUS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_UN_POWER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_EAT_GOLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_EAT_ITEM");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_EAT_FOOD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_EAT_LITE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_ACID");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_ELEC");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_FIRE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_COLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_BLIND");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_CONFUSE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_TERRIFY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_PARALYZE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_LOSE_STR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_LOSE_INT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_LOSE_WIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_LOSE_DEX");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_LOSE_CON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_LOSE_CHR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_LOSE_ALL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_SHATTER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_EXP_10");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_EXP_20");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_EXP_40");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_EXP_80");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RBE_HALLU");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_OPP_ACID");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_OPP_ELEC");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_OPP_FIRE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_OPP_COLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_OPP_POIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_OPP_XXX1");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_OPP_XXX2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_OPP_XXX3");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_IMM_XXX5");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_IMM_XXX6");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_IMM_FREE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_IMM_MANA");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_IMM_ACID");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_IMM_ELEC");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_IMM_FIRE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_IMM_COLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_RES_ACID");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_RES_ELEC");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_RES_FIRE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_RES_COLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_RES_POIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_RES_FEAR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_RES_LITE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_RES_DARK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_RES_BLIND");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_RES_CONFU");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_RES_SOUND");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_RES_SHARD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_RES_NEXUS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_RES_NETHR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_RES_CHAOS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"SM_RES_DISEN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MFLAG_VIEW");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MFLAG_NICE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MFLAG_SHOW");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"MFLAG_MARK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_UNIQUE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_QUESTOR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_MALE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_FEMALE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_CHAR_CLEAR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_CHAR_MULTI");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_ATTR_CLEAR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_ATTR_MULTI");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_FORCE_DEPTH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_FORCE_MAXHP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_FORCE_SLEEP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_FORCE_EXTRA");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_FRIEND");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_FRIENDS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_ESCORT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_ESCORTS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_NEVER_BLOW");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_NEVER_MOVE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_RAND_25");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_RAND_50");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_ONLY_GOLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_ONLY_ITEM");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_DROP_60");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_DROP_90");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_DROP_1D2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_DROP_2D2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_DROP_3D2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_DROP_4D2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_DROP_GOOD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_DROP_GREAT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_DROP_USEFUL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF1_DROP_CHOSEN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_STUPID");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_SMART");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_XXX1");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_XXX2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_INVISIBLE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_COLD_BLOOD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_EMPTY_MIND");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_WEIRD_MIND");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_MULTIPLY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_REGENERATE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_XXX3");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_XXX4");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_POWERFUL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_XXX5");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_XXX7");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_XXX6");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_OPEN_DOOR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_BASH_DOOR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_PASS_WALL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_KILL_WALL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_MOVE_BODY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_KILL_BODY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_TAKE_ITEM");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_KILL_ITEM");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_BRAIN_1");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_BRAIN_2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_BRAIN_3");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_BRAIN_4");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_BRAIN_5");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_BRAIN_6");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_BRAIN_7");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF2_BRAIN_8");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_ORC");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_TROLL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_GIANT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_DRAGON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_DEMON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_UNDEAD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_EVIL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_ANIMAL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_XXX1");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_XXX2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_XXX3");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_XXX4");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_HURT_LITE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_HURT_ROCK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_HURT_FIRE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_HURT_COLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_IM_ACID");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_IM_ELEC");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_IM_FIRE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_IM_COLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_IM_POIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_XXX5");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_RES_NETH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_RES_WATE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_RES_PLAS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_RES_NEXU");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_RES_DISE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_XXX6");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_NO_FEAR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_NO_STUN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_NO_CONF");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF3_NO_SLEEP");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_SHRIEK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_XXX2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_XXX3");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_XXX4");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_ARROW_1");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_ARROW_2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_ARROW_3");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_ARROW_4");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_BR_ACID");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_BR_ELEC");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_BR_FIRE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_BR_COLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_BR_POIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_BR_NETH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_BR_LITE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_BR_DARK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_BR_CONF");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_BR_SOUN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_BR_CHAO");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_BR_DISE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_BR_NEXU");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_BR_TIME");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_BR_INER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_BR_GRAV");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_BR_SHAR");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_BR_PLAS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_BR_WALL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_BR_MANA");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_XXX5");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_XXX6");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_XXX7");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF4_BOULDER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_BA_ACID");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_BA_ELEC");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_BA_FIRE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_BA_COLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_BA_POIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_BA_NETH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_BA_WATE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_BA_MANA");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_BA_DARK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_DRAIN_MANA");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_MIND_BLAST");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_BRAIN_SMASH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_CAUSE_1");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_CAUSE_2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_CAUSE_3");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_CAUSE_4");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_BO_ACID");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_BO_ELEC");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_BO_FIRE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_BO_COLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_BO_POIS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_BO_NETH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_BO_WATE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_BO_MANA");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_BO_PLAS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_BO_ICEE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_MISSILE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_SCARE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_BLIND");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_CONF");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_SLOW");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF5_HOLD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_HASTE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_XXX1");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_HEAL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_XXX2");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_BLINK");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_TPORT");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_XXX3");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_XXX4");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_TELE_TO");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_TELE_AWAY");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_TELE_LEVEL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_XXX5");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_DARKNESS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_TRAPS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_FORGET");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_XXX6");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_S_KIN");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_S_HI_DEMON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_S_MONSTER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_S_MONSTERS");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_S_ANIMAL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_S_SPIDER");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_S_HOUND");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_S_HYDRA");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_S_ANGEL");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_S_DEMON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_S_UNDEAD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_S_DRAGON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_S_HI_UNDEAD");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_S_HI_DRAGON");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_S_WRAITH");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"RF6_S_UNIQUE");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"monster_blow");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"monster_race");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"monster_lore");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"monster_type");
 lua_getglobals(tolua_S);
 lua_pushstring(tolua_S,"mon_max"); lua_pushnil(tolua_S); lua_rawset(tolua_S,-3);
 lua_pop(tolua_S,1);
 lua_getglobals(tolua_S);
 lua_pushstring(tolua_S,"mon_cnt"); lua_pushnil(tolua_S); lua_rawset(tolua_S,-3);
 lua_pop(tolua_S,1);
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"make_attack_normal");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"make_attack_spell");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"process_monsters");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"screen_roff");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"display_roff");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"delete_monster_idx");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"delete_monster");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"compact_monsters");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"wipe_mon_list");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"mon_pop");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"get_mon_num_prep");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"get_mon_num");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"monster_desc");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"lore_do_probe");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"lore_treasure");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"update_mon");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"update_monsters");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"monster_carry");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"monster_swap");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"player_place");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"monster_place");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"place_monster_aux");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"place_monster");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"alloc_monster");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"summon_specific");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"multiply_monster");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"message_pain");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"update_smart_learn");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"monster_death");
 lua_pushnil(tolua_S); lua_setglobal(tolua_S,"mon_take_hit");
}
