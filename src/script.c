/* File: script.c */

#include "angband.h"

#include "script.h"


#ifdef USE_SCRIPT

#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
#include "lua/tolua.h"


/*
 * Lua state
 */
static lua_State* L = NULL;


static int xxx_msg_print(lua_State *L)
{
	cptr text = lua_tostring(L, 1);
	if (text) msg_print(text);
	lua_pop(L, 1);

	return 0;
}


static int xxx_msg_flush(lua_State *L)
{
	message_flush();

	return 0;
}


static int xxx_build_script_path(lua_State *L)
{
	char buf[1024];
	cptr filename;

	if (!tolua_istype(L, 1, LUA_TSTRING,0))
		tolua_error(L, "#vinvalid type in variable assignment.");

	filename = tolua_getstring(L, 1, 0);

	path_build(buf, 1024, ANGBAND_DIR_SCRIPT, filename);

	tolua_pushstring(L, buf);

	return 1;
}


static int xxx_get_aim_dir(lua_State *L)
{
	int dir;
	bool success;

	success = get_aim_dir(&dir);
	lua_pushnumber(L, success);
	lua_pushnumber(L, dir);

	return 2;
}


static int xxx_fire_beam(lua_State *L)
{
	int typ, dir, dam;
	bool result;

	typ = (int)luaL_check_number(L, 1);
	dir = (int)luaL_check_number(L, 2);
	dam = (int)luaL_check_number(L, 3);
	result = fire_beam(typ, dir, dam);
	lua_pushnumber(L, result);

	return 1;
}


static const struct luaL_reg anglib[] =
{
	{"msg_print", xxx_msg_print},
	{"msg_flush", xxx_msg_flush},
	{"get_aim_dir", xxx_get_aim_dir},
	{"fire_beam", xxx_fire_beam},
	{"build_script_path", xxx_build_script_path},
};


#define DYADIC(name, op) \
    static int name(lua_State* L) { \
        lua_pushnumber(L, luaL_check_int(L, 1) op luaL_check_int(L, 2)); \
		return 1; \
    }

#define MONADIC(name, op) \
    static int name(lua_State* L) { \
        lua_pushnumber(L, op luaL_check_int(L, 1)); \
		return 1; \
    }


DYADIC(intMod,      % )
DYADIC(intAnd,      & )
DYADIC(intOr,       | )
DYADIC(intXor,      ^ )
DYADIC(intShiftl,   <<)
DYADIC(intShiftr,   >>)
MONADIC(intBitNot,  ~ )


static const struct luaL_reg intMathLib[] =
{
    {"mod",    intMod    },
    {"bAnd",   intAnd    },
    {"bOr",    intOr     },
    {"bXor",   intXor    },
    {"bNot",   intBitNot },
    {"shiftl", intShiftl },
    {"shiftr", intShiftr },
};


/*
 * Callback for using an object
 */
bool use_object(object_type *o_ptr, bool *ident)
{
	bool used_up;

	lua_getglobal(L, "use_object_hook");
	tolua_pushusertype(L, (void*)o_ptr, tolua_tag(L, "object_type"));

	/* Call the function with 1 argument and 2 results */
	lua_call(L, 1, 2);

	*ident = tolua_getbool(L, 1, FALSE);
	used_up = tolua_getbool(L, 2, FALSE);

	/* Remove the results */
	lua_pop(L, 2);

	return (used_up);
}


extern int tolua_player_open(lua_State* tolua_S);
extern void tolua_player_close(lua_State* tolua_S);
extern int tolua_object_open(lua_State* tolua_S);
extern void tolua_object_close(lua_State* tolua_S);
extern int tolua_monster_open(lua_State* tolua_S);
extern void tolua_monster_close(lua_State* tolua_S);
extern int tolua_random_open(lua_State* tolua_S);
extern void tolua_random_close(lua_State* tolua_S);
extern int tolua_ui_open(lua_State* tolua_S);
extern void tolua_ui_close(lua_State* tolua_S);
extern int tolua_misc_open(lua_State* tolua_S);
extern void tolua_misc_close(lua_State* tolua_S);
extern int tolua_spell_open(lua_State* tolua_S);
extern void tolua_spell_close(lua_State* tolua_S);


/*
 * Initialize scripting support
 */
errr script_init(void)
{
	char buf[1024];

	/* Start the interpreter with default stack size */
	L = lua_open(0);

	/* Register the Lua base libraries */
	lua_baselibopen(L);
	lua_strlibopen(L);
	lua_dblibopen(L);

	/* Register library with binary functions */
	luaL_openl(L, intMathLib);

	/* Register the Angband base library */
	luaL_openl(L, anglib);

	/* Register various Angband libraries */
	tolua_player_open(L);
	tolua_object_open(L);
	tolua_monster_open(L);
	tolua_random_open(L);
	tolua_ui_open(L);
	tolua_misc_open(L);
	tolua_spell_open(L);

	/* Initialization code */
	path_build(buf, 1024, ANGBAND_DIR_SCRIPT, "init.lua");
	script_do_file(buf);

	return 0;
}


errr script_free(void)
{
	if (L) lua_close(L);

	return 0;
}


bool script_do_string(cptr script)
{
	lua_dostring(L, script);

	return TRUE;
}


bool script_do_file(cptr filename)
{
	lua_dofile(L, filename);

	return TRUE;
}


#else /* USE_SCRIPT */

errr script_init(void)
{
	return 0;
}


errr script_free(void)
{
	return 0;
}


bool script_do_string(cptr script)
{
	return FALSE;
}


bool script_do_file(cptr filename)
{
	return FALSE;
}


#endif /* USE_SCRIPT */

