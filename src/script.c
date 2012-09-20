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


static int xxx_build_script_path(lua_State *L)
{
	char buf[1024];
	cptr filename;

	if (!tolua_istype(L, 1, LUA_TSTRING,0))
		tolua_error(L, "#vinvalid type in variable assignment.");

	filename = tolua_getstring(L, 1, 0);

	path_build(buf, sizeof(buf), ANGBAND_DIR_SCRIPT, filename);

	tolua_pushstring(L, buf);

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
	object_desc(buf, sizeof(buf), o_ptr, pref, mode);

	/* Return the name */
	tolua_pushstring(L, buf);

	/* One result */
	return 1;
}


static const struct luaL_reg anglib[] =
{
	{"build_script_path", xxx_build_script_path},
	{"object_desc", xxx_object_desc},
};


#define luaL_check_bit(L, n)  ((long)luaL_check_number(L, n))
#define luaL_check_ubit(L, n) ((unsigned long)luaL_check_bit(L, n))

static int int_not(lua_State* L)
{
	lua_pushnumber(L, ~luaL_check_bit(L, 1));
	return 1;
}

static int int_mod(lua_State* L)
{
	lua_pushnumber(L, luaL_check_bit(L, 1) % luaL_check_bit(L, 2));
	return 1;
}

static int int_or(lua_State *L)
{
	int n = lua_gettop(L), i;
	long w = luaL_check_bit(L, 1);
	for (i = 2; i <= n; i++)
		w |= luaL_check_bit(L, i);
	lua_pushnumber(L, w);
	return 1;
}

static int int_xor(lua_State *L)
{
	int n = lua_gettop(L), i;
	long w = luaL_check_bit(L, 1);
	for (i = 2; i <= n; i++)
		w ^= luaL_check_bit(L, i);
	lua_pushnumber(L, w);
	return 1;
}

static int int_and(lua_State *L)
{
	int n = lua_gettop(L), i;
	long w = luaL_check_bit(L, 1);
	for (i = 2; i <= n; i++)
		w &= luaL_check_bit(L, i);
	lua_pushnumber(L, w);
	return 1;
}

static int int_lshift(lua_State* L)
{
	lua_pushnumber(L, luaL_check_bit(L, 1) << luaL_check_ubit(L, 2));
	return 1;
}

static int int_rshift(lua_State* L)
{
	lua_pushnumber(L, luaL_check_ubit(L, 1) >> luaL_check_ubit(L, 2));
	return 1;
}

static int int_arshift(lua_State* L)
{
	lua_pushnumber(L, luaL_check_bit(L, 1) >> luaL_check_ubit(L, 2));
	return 1;
}

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
 * Call a Lua event handler
 *
 * Calls a Lua event handler with the name 'hook',
 * with arguments defined by the format string 'fmt',
 * and return values as defined by the 'ret' format string.
 * The next arguments to call_hook() are the arguments to
 * be passed to the Lua event handler, followed by pointers
 * to the expected return values.
 *
 * ToDo: explain the format strings ...
 *
 * The return value indicates if the hook was called
 * successfully (TRUE) or if there was an error (FALSE).
 *
 * Note that string and object_type* return values have to be
 * copied to a save place before the next call to Lua since
 * they will be lost when garbage collection takes place.
 * So store the values with string_make() or object_copy() into
 * a save location if you need them afterwards.
 */
static bool call_hook(cptr hook, cptr fmt, cptr ret, ...)
{
	va_list ap;
	int i = 0;
	int num_args = 0;
	int num_res;

	va_start(ap, ret);

	/* Select the global event handler */
	lua_getglobal(L, "notify_event_hook");

	/* Push the event name */
	lua_pushstring(L, hook);

	/* Push and count the arguments */
	while (fmt[i])
	{
		switch (fmt[i++])
		{
			case 'd':
				tolua_pushnumber(L, va_arg(ap, int));
				num_args++;
				break;
			case 'l':
				tolua_pushnumber(L, va_arg(ap, long));
				num_args++;
				break;
			case 'b':
				tolua_pushbool(L, va_arg(ap, int));
				num_args++;
				break;
			case 's':
				tolua_pushstring(L, va_arg(ap, cptr));
				num_args++;
				break;
			case 'O':
				tolua_pushusertype(L, va_arg(ap, object_type*), tolua_tag(L, "object_type"));
				num_args++;
				break;
			case '(':
			case ')':
			case ',':
				break;
		}
	}

	/* HACK - Count the return values */
	num_res = strlen(ret);

	/* Call the function */
	if (lua_call(L, num_args + 1, num_res))
	{
		/* An error occured */
		va_end(ap);
		return FALSE;
	}

	/* Get the return values */
	for (i = 0; i < num_res; i++)
	{
		switch (ret[i])
		{
			case 'd':
			{
				int *tmp = va_arg(ap, int*);

				if (lua_isnumber(L, -num_res + i))
					*tmp = tolua_getnumber(L, -num_res + i, 0);
				break;
			}
			case 'l':
			{
				long *tmp = va_arg(ap, long*);

				if (lua_isnumber(L, -num_res + i))
					*tmp = tolua_getnumber(L, -num_res + i, 0);
				break;
			}
			case 'b':
			{
				bool *tmp = va_arg(ap, bool*);
				*tmp = tolua_getbool(L, -num_res + i, FALSE);
				break;
			}
			case 's':
			{
				cptr *tmp = va_arg(ap, cptr*);
				if (lua_isstring(L, -num_res + i))
					*tmp = tolua_getstring(L, -num_res + i, "");
				break;
			}
			case 'O':
			{
				object_type **tmp = va_arg(ap, object_type**);
				if (tolua_istype(L, -num_res + i, tolua_tag(L, "object_type"), 0))
					*tmp = tolua_getuserdata(L, -num_res + i, NULL);
				break;
			}
		}
	}

	/* Remove the results */
	lua_pop(L, num_res);

	va_end(ap);

	/* Success */
	return TRUE;
}


/*
 * Callback for using an object
 */
bool use_object(object_type *o_ptr, bool *ident)
{
	bool used_up = FALSE;

	if (!call_hook("use_object", "(O)", "bb",
	               o_ptr,
	               ident, &used_up))
	{
		/* Error */
		*ident = FALSE;
		used_up = FALSE;
	}

	return (used_up);
}


int get_spell_index(const object_type *o_ptr, int index)
{
	int spell = -1;

	if (!call_hook("get_spell_index", "(O,d)", "d",
	               o_ptr, index,
	               &spell))
	{
		/* Error */
		return -1;
	}

	return (spell);
}


cptr get_spell_name(int tval, int index)
{
	static char buffer[80];
	cptr name;

	/* Erase the buffer */
	buffer[0] = '\0';

	if (!call_hook("get_spell_name", "(d,d)", "s",
	               tval, index,
	               &name))
	{
		/* Error */
		return "";
	}

	/* Get a copy of the name */
	my_strcpy(buffer, name, sizeof(buffer));

	/* Return the result */
	return (buffer);
}


cptr get_spell_info(int tval, int index)
{
	static char buffer[80];
	cptr info;

	/* Erase the buffer */
	buffer[0] = '\0';

	if (!call_hook("get_spell_info", "(d,d)", "s",
	               tval, index,
	               &info))
	{
		/* Error */
		return "";
	}

	/* Get a copy of the info */
	my_strcpy(buffer, info, sizeof(buffer));

	/* Return the result */
	return (buffer);
}


bool cast_spell(int tval, int index)
{
	bool result = FALSE;

	if (!call_hook("cast_spell", "(d,d)", "b",
	               tval, index,
	               &result))
	{
		/* Error */
		return FALSE;
	}

	return (result);
}


void describe_item_activation(const object_type *o_ptr)
{
	cptr desc = NULL;

	if (call_hook("describe_item_activation", "(O)", "s",
	              o_ptr,
	              &desc))
	{
		if (desc) text_out(desc);
	}
}


/*
 * Get an object index for an item to put in stock
 * at a selected store.
 */
int get_store_choice(int store_num)
{
	int choice = -1;

	if (!call_hook("get_store_choice", "(d)", "d",
	               store_num,
	               &choice))
	{
		/* Error */
		return -1;
	}

	return (choice);
}


/*
 * Determine if a store will purchase the given object
 */
bool store_will_buy(int store_num, const object_type *o_ptr)
{
	bool result = FALSE;

	if (!call_hook("store_will_buy", "(d,O)", "b",
	               store_num, o_ptr,
	               &result))
	{
		/* Error */
		return FALSE;
	}

	return (result);
}


void player_birth_done_hook(void)
{
	call_hook("player_birth_done", "", "");
}


void player_calc_bonus_hook(void)
{
	call_hook("player_calc_bonus", "", "");
}


void start_game_hook(void)
{
	call_hook("start_game", "", "");
}


void enter_level_hook(void)
{
	call_hook("enter_level", "", "");
}


void leave_level_hook(void)
{
	call_hook("leave_level", "", "");
}


bool process_command_hook(int command)
{
	int result = 0;

	if (!call_hook("process_command", "(d)", "d",
	               command,
	               &result))
	{
		/* Error */
		return FALSE;
	}

	/* Return the result */
	return (result != 0) ? TRUE : FALSE;
}


bool generate_level_hook(int level)
{
	int result = 0;

	if (!call_hook("generate_level", "(d)", "d",
	               level,
	               &result))
	{
		/* Error */
		return FALSE;
	}

	/* Return the result */
	return (result != 0) ? TRUE : FALSE;
}


#ifdef ALLOW_USER_SCRIPTS

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

			path_build(buf, sizeof(buf), ANGBAND_DIR_SCRIPT, "trace.lua");

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
			if (!askfor_aux(tmp, sizeof(tmp))) break;

			/* Clear the prompt */
			prt("", 0, 0);

			path_build(buf, sizeof(buf), ANGBAND_DIR_SCRIPT, tmp);

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
			if (!askfor_aux(tmp, sizeof(tmp))) break;

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
			path_build(buf, sizeof(buf), ANGBAND_DIR_SCRIPT, "init.lua");
			script_do_file(buf);

			break;
		}
	}
}

#else /* ALLOW_USER_SCRIPTS */

void do_cmd_script(void)
{
	/* Do nothing */
}

#endif /* ALLOW_USER_SCRIPTS */


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
	path_build(buf, sizeof(buf), ANGBAND_DIR_SCRIPT, "init.lua");
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

#ifdef RISCOS
	{
		char *realname = riscosify_name(filename);
		if (!lua_dofile(L, realname)) return TRUE;
	}
#else /* RISCOS */
	if (!lua_dofile(L, filename)) return TRUE;
#endif /* RISCOS */

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

