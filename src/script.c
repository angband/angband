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


static int xxx_object_desc(lua_State *L)
{
	char buf[1024];
	int pref, mode;
	object_type *o_ptr;

	if (!tolua_istype(L, 1, tolua_tag(L, "object_type"), 0))
	{
		tolua_error(L, "#ferror in function 'object_desc'.");
		return 0;
	}

	/* Get the arguments */
	o_ptr = tolua_getusertype(L, 1, 0);
	pref = (int)luaL_check_number(L, 2);
	mode = (int)luaL_check_number(L, 3);

	/* Get the description */
	object_desc(buf, o_ptr, pref, mode);

	/* Return the name */
	tolua_pushstring(L, buf);

	/* One result */
	return 1;
}


static const struct luaL_reg anglib[] =
{
	{"msg_print", xxx_msg_print},
	{"msg_flush", xxx_msg_flush},
	{"get_aim_dir", xxx_get_aim_dir},
	{"fire_beam", xxx_fire_beam},
	{"build_script_path", xxx_build_script_path},
	{"object_desc", xxx_object_desc},
};


#define luaL_check_bit(L, n)  ((long)luaL_check_number(L, n))
#define luaL_check_ubit(L, n) ((unsigned long)luaL_check_bit(L, n))

#define TDYADIC(name, op, t1, t2) \
	static int int_ ## name(lua_State* L) \
	{ \
		lua_pushnumber(L, \
		luaL_check_ ## t1 ## bit(L, 1) op luaL_check_ ## t2 ## bit(L, 2)); \
		return 1; \
	}

#define DYADIC(name, op) \
	TDYADIC(name, op, , )

#define MONADIC(name, op) \
	static int int_ ## name(lua_State* L) \
	{ \
		lua_pushnumber(L, op luaL_check_bit(L, 1)); \
		return 1; \
	}

#define VARIADIC(name, op) \
	static int int_ ## name(lua_State *L) \
	{ \
		int n = lua_gettop(L), i; \
		long w = luaL_check_bit(L, 1); \
		for (i = 2; i <= n; i++) \
			w op ## = luaL_check_bit(L, i); \
		lua_pushnumber(L, w); \
		return 1; \
	}

MONADIC(not,     ~)
DYADIC(mod,      %)
VARIADIC(and,    &)
VARIADIC(or,     |)
VARIADIC(xor,    ^)
TDYADIC(lshift,  <<, , u)
TDYADIC(rshift,  >>, u, u)
TDYADIC(arshift, >>, , u)

static const struct luaL_reg bitlib[] =
{
	{"bNot",    int_not},
	{"iMod",    int_mod},  /* "mod" already in Lua math library */
	{"bAnd",    int_and},
	{"bOr",     int_or},
	{"bXor",    int_xor},
	{"lshift",  int_lshift},
	{"rshift",  int_rshift},
	{"arshift", int_arshift},
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
		*ident = tolua_getbool(L, -2, FALSE);
		used_up = tolua_getbool(L, -1, FALSE);

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


int get_spell_index(const object_type *o_ptr, int index)
{
	int status;
	int spell;

	lua_getglobal(L, "get_spell_index_hook");
	tolua_pushusertype(L, (void*)o_ptr, tolua_tag(L, "object_type"));
	lua_pushnumber(L, index);

	/* Call the function with 2 arguments and 1 result */
	status = lua_call(L, 2, 1);

	if (status == 0)
	{
		spell = tolua_getnumber(L, -1, -1);

		/* Remove the result */
		lua_pop(L, 1);
	}
	else
	{
		/* Error */
		spell = -1;
	}

	return (spell);
}


cptr get_spell_name(int tval, int index)
{
	static char buffer[80];
	cptr name;

	lua_getglobal(L, "get_spell_name_hook");
	lua_pushnumber(L, tval);
	lua_pushnumber(L, index);

	/* Erase the buffer */
	buffer[0] = '\0';

	/* Call the function with 2 arguments and 1 result */
	if (!lua_call(L, 2, 1))
	{
		name = tolua_getstring(L, -1, "");

		/* Get a copy of the name */
		strncpy(buffer, name, 80);

		/* Make sure it's terminated */
		buffer[79] = '\0';

		/* Remove the result */
		lua_pop(L, 1);
	}

	return (buffer);
}


cptr get_spell_info(int tval, int index)
{
	static char buffer[80];
	cptr info;

	lua_getglobal(L, "get_spell_info_hook");
	lua_pushnumber(L, tval);
	lua_pushnumber(L, index);

	/* Erase the buffer */
	buffer[0] = '\0';

	/* Call the function with 2 arguments and 1 result */
	if (!lua_call(L, 2, 1))
	{
		info = tolua_getstring(L, -1, "");

		/* Get a copy of the name */
		strncpy(buffer, info, 80);

		/* Make sure it's terminated */
		buffer[79] = '\0';

		/* Remove the result */
		lua_pop(L, 1);
	}

	return (buffer);
}


bool cast_spell(int tval, int index)
{
	static char buffer[80];
	bool done = FALSE;

	lua_getglobal(L, "cast_spell_hook");
	lua_pushnumber(L, tval);
	lua_pushnumber(L, index);

	/* Erase the buffer */
	buffer[0] = '\0';

	/* Call the function with 2 arguments and 1 result */
	if (!lua_call(L, 2, 1))
	{
		done = tolua_getbool(L, -1, FALSE);

		/* Remove the result */
		lua_pop(L, 1);
	}

	return (done);
}


void describe_item_activation(const object_type *o_ptr)
{
	cptr desc;

	lua_getglobal(L, "describe_item_activation_hook");

	tolua_pushusertype(L, (void*)o_ptr, tolua_tag(L, "object_type"));

	/* Call the function with 1 arguments and 1 result */
	if (!lua_call(L, 1, 1))
	{
		desc = tolua_getstring(L, -1, "");

		text_out(desc);

		/* Remove the result */
		lua_pop(L, 1);
	}
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
		if (op_ptr->window_flag[j] & PW_SCRIPT_SOURCE)
		{
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
		else if (op_ptr->window_flag[j] & PW_SCRIPT_VARS)
		{
			char buf[1024];

			/* Activate */
			Term_activate(angband_term[j]);

			path_build(buf, 1024, ANGBAND_DIR_SCRIPT, "trace.lua");

			/* Execute the file */
			script_do_file(buf);

			/* Fresh */
			Term_fresh();

			/* Restore */
			Term_activate(old);
		}
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
	luaL_openl(L, bitlib);

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
