/* File: script.c */

#include "angband.h"

#include "script.h"


#ifdef USE_SCRIPT

#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
#include "lua/tolua.h"
#include "lua/luadebug.h"


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
	tolua_pushbool(L, success);
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
	tolua_pushbool(L, result);

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
	int status;

	lua_getglobal(L, "use_object_hook");
	tolua_pushusertype(L, (void*)o_ptr, tolua_tag(L, "object_type"));

	/* Call the function with 1 argument and 2 results */
	status = lua_call(L, 1, 2);

	if (status == 0)
	{
		*ident = tolua_getbool(L, 1, FALSE);
		used_up = tolua_getbool(L, 2, FALSE);

		/* Remove the results */
		lua_pop(L, 2);
	}
	else
	{
		/* Error */
		*ident = FALSE;
		used_up = FALSE;
	}

	return (used_up);
}


static void line_hook(lua_State *L, lua_Debug *ar)
{
	int j;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		term *old = Term;

		/* No window */
		if (!angband_term[j]) continue;

		/* No relevant flags */
		if (!(op_ptr->window_flag[j] & PW_SCRIPT_SOURCE)) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		lua_getstack(L, 0, ar);
		lua_getinfo(L, "S", ar);
		show_file(ar->source + 1, ar->short_src, ar->currentline - 1, 1);

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


static void script_trace_start(void)
{
	if (!L) return;

	lua_setlinehook(L, line_hook);
}


static void script_trace_stop(void)
{
	if (!L) return;

	lua_setlinehook(L, NULL);
}


void do_cmd_script(void)
{
	int ch;
	char tmp[80];


	/* Save screen */
	screen_save();

	/* Clear screen */
	Term_clear();

	/* Ask for a choice */
	prt("Debug scripts", 2, 0);

	/* Give some choices */
	prt("(1) Execute a script file", 4, 5);
	prt("(2) Execute a script command", 5, 5);
	prt("(3) Start tracing scripts", 6, 5);
	prt("(4) Stop tracing scripts", 7, 5);
	prt("(5) Re-initialize scripts", 8, 5);

	/* Prompt */
	prt("Command: ", 15, 0);

	/* Prompt */
	ch = inkey();

	/* Load screen */
	screen_load();

	switch (ch)
	{
		case '1':
		{
			char buf[1024];

			/* Prompt */
			prt("Lua script: ", 0, 0);

			/* Default filename */
			sprintf(tmp, "test.lua");

			/* Ask for a file */
			if (!askfor_aux(tmp, 80)) break;

			/* Clear the prompt */
			prt("", 0, 0);

			path_build(buf, 1024, ANGBAND_DIR_SCRIPT, tmp);

			/* Execute the file */
			script_do_file(buf);

			break;
		}
		case '2':
		{
			/* Prompt */
			prt("Lua command: ", 0, 0);

			/* Empty default */
			strcpy(tmp, "");

			/* Ask for a command */
			if (!askfor_aux(tmp, 80)) break;

			/* Clear the prompt */
			prt("", 0, 0);

			/* Execute the command */
			script_do_string(tmp);

			break;
		}
		case '3':
		{
			script_trace_start();

			break;
		}
		case '4':
		{
			script_trace_stop();

			break;
		}
		case '5':
		{
			char buf[1024];

			/* Initialization code */
			path_build(buf, 1024, ANGBAND_DIR_SCRIPT, "init.lua");
			script_do_file(buf);

			break;
		}
	}
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
	if (L)
	{
		lua_close(L);
		return 0;
	}
	else
	{
		/* Error */
		return -1;
	}
}


bool script_do_string(cptr script)
{
	if (!L) return FALSE;

	if (!lua_dostring(L, script)) return TRUE;

	return FALSE;
}


bool script_do_file(cptr filename)
{
	if (!L) return FALSE;

	if (!lua_dofile(L, filename)) return TRUE;

	return FALSE;
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

void do_cmd_script(void)
{
	/* Do nothing */
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
