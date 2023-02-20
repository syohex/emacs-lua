/*
  Copyright (C) 2022 by Shohei YOSHIDA

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <emacs-module.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

int plugin_is_GPL_compatible;

static void
lua_free(void *arg)
{
	if (arg) {
		lua_State *ls = (lua_State*)arg;
		lua_close(ls);
	}
}

static emacs_value
Flua_close(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void *data)
{
	lua_State *ls = env->get_user_ptr(env, args[0]);
	lua_close(ls);
	env->set_user_ptr(env, args[0], NULL);
	return env->intern(env, "nil");
}

static emacs_value
Flua_init(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void *data)
{
	lua_State *ls = luaL_newstate();
	luaL_openlibs(ls);
	return env->make_user_ptr(env, lua_free, ls);
}

static emacs_value
lua_to_elisp(emacs_env *env, lua_State *ls, int index)
{
	switch (lua_type(ls, index)) {
	case LUA_TNIL:
		return env->intern(env, "nil");
	case LUA_TNUMBER: {
		lua_Integer d = lua_tointeger(ls, index);
		lua_Number f = lua_tonumber(ls, index);

		if (d != f) {
			return env->make_float(env, f);
		} else {
			return env->make_integer(env, d);
		}
	}
	case LUA_TBOOLEAN:
		if (lua_toboolean(ls, index))
			return env->intern(env, "t");
		else
			return env->intern(env, "nil");
	case LUA_TSTRING: {
		const char *str = lua_tostring(ls, index);
		return env->make_string(env, str, strlen(str));
	}
	case LUA_TTABLE: {
		emacs_value Fmake_hash_table = env->intern(env, "make-hash-table");
		emacs_value Qtest = env->intern(env, ":test");
		emacs_value Fequal = env->intern(env, "equal");
		emacs_value make_hash_args[] = {Qtest, Fequal};
		emacs_value hash = env->funcall(env, Fmake_hash_table, 2, make_hash_args);

		emacs_value Fputhash = env->intern(env, "puthash");

		index = lua_gettop(ls);
		lua_pushnil(ls);
		while (lua_next(ls, index) != 0) {
			emacs_value key = lua_to_elisp(env, ls, -2);
			emacs_value val = lua_to_elisp(env, ls, -1);

			emacs_value puthash_args[] = {key, val, hash};
			env->funcall(env, Fputhash, 3, puthash_args);

			lua_pop(ls, 1);
		}

		return hash;
	}
	case LUA_TFUNCTION:
	case LUA_TUSERDATA:
	case LUA_TLIGHTUSERDATA:
	case LUA_TTHREAD:
	default: {
		emacs_value Ferror = env->intern(env, "error");
		char errmsg[] = "Sorry not supported type!!";
		emacs_value err_args[] = {
			env->make_string(env, errmsg, sizeof(errmsg) - 1),
		};
		return env->funcall(env, Ferror, 1, err_args);
	}
	}
}

static bool
eq_type(emacs_env *env, emacs_value type, const char *type_str)
{
	return env->eq(env, type, env->intern(env, type_str));
}

static int
emacs_message(lua_State *ls) {
	lua_getglobal(ls, "emacs_env");
	emacs_env *env = lua_touserdata(ls, -1);

	int n = lua_gettop(ls);
	emacs_value *args = malloc(sizeof(emacs_value) * (n - 1));
	for (int i = n - 1; i >= 1; --i) {
		args[i - 1] = lua_to_elisp(env, ls, i);
	}

	emacs_value Fmessage = env->intern(env, "message");
	env->funcall(env, Fmessage, n - 1, args);

	free(args);
	return 0;
}

static void
elisp_to_lua(emacs_env *env, lua_State *ls, emacs_value v)
{
	emacs_value type = env->type_of(env, v);

	if (!env->is_not_nil(env, v)) {
		lua_pushnil(ls);
	} else if (eq_type(env, type, "integer")) {
		lua_pushinteger(ls, env->extract_integer(env, v));
	} else if (eq_type(env, type, "float")) {
		lua_pushnumber(ls, env->extract_float(env, v));
	} else if (eq_type(env, type, "string")) {
		ptrdiff_t size = 0;
		env->copy_string_contents(env, v, NULL, &size);

		char *str = malloc(size);
		env->copy_string_contents(env, v, str, &size);
		lua_pushstring(ls, str);
		free(str);
	} else if (env->eq(env, v, env->intern(env, "t"))) {
		lua_pushboolean(ls, 1);
	} else {
		emacs_value Ferror = env->intern(env, "error");
		const char errmsg[] = "Sorry not supported type!!";
		emacs_value err_args[] = {
			env->make_string(env, errmsg, sizeof(errmsg) - 1),
		};
		env->funcall(env, Ferror, 1, err_args);
	}
}

static emacs_value
Flua_do_string(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void *data )
{
	emacs_value Qnil = env->intern(env, "nil");
	lua_State *ls = env->get_user_ptr(env, args[0]);
	if (ls == NULL)
		return Qnil;

	emacs_value code = args[1];
	ptrdiff_t size = 0;

	env->copy_string_contents(env, code, NULL, &size);
	char *code_buf = malloc(size);
	if (code_buf == NULL)
		return Qnil;
	env->copy_string_contents(env, code, code_buf, &size);

	lua_pushcfunction(ls, emacs_message);
	lua_setglobal(ls, "message");
	lua_pop(ls, -1);

	lua_pushlightuserdata(ls, env);
	lua_setglobal(ls, "emacs_env");
	lua_pop(ls, -1);

	int ret = luaL_dostring(ls, code_buf);
	if (ret != 0) {
		return Qnil;
	}

	return lua_to_elisp(env, ls, -1);
}

static emacs_value
Flua_get_global(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void *data )
{
	lua_State *ls = env->get_user_ptr(env, args[0]);

	emacs_value key = args[1];
	ptrdiff_t size = 0;

	env->copy_string_contents(env, key, NULL, &size);
	char *key_buf = malloc(size);
	if (key_buf == NULL)
		return env->intern(env, "nil");
	env->copy_string_contents(env, key, key_buf, &size);

	lua_getglobal(ls, key_buf);
	free(key_buf);

	return lua_to_elisp(env, ls, -1);
}

static emacs_value
Flua_set_global(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void *data )
{
	lua_State *ls = env->get_user_ptr(env, args[0]);

	emacs_value key = args[1];
	emacs_value val = args[2];

	ptrdiff_t size = 0;
	env->copy_string_contents(env, key, NULL, &size);
	char *key_buf = malloc(size);
	if (key_buf == NULL)
		return env->intern(env, "nil");
	env->copy_string_contents(env, key, key_buf, &size);

	elisp_to_lua(env, ls, val);
	lua_setglobal(ls, key_buf);

	return val;
}


static void
bind_function(emacs_env *env, const char *name, emacs_value Sfun)
{
	emacs_value Qfset = env->intern(env, "fset");
	emacs_value Qsym = env->intern(env, name);
	emacs_value args[] = { Qsym, Sfun };

	env->funcall(env, Qfset, 2, args);
}

static void
provide(emacs_env *env, const char *feature)
{
	emacs_value Qfeat = env->intern(env, feature);
	emacs_value Qprovide = env->intern (env, "provide");
	emacs_value args[] = { Qfeat };

	env->funcall(env, Qprovide, 1, args);
}

int
emacs_module_init(struct emacs_runtime *ert)
{
	emacs_env *env = ert->get_environment(ert);

#define DEFUN(lsym, csym, amin, amax, doc, data) \
	bind_function (env, lsym, env->make_function(env, amin, amax, csym, doc, data))

	DEFUN("lua-core-init", Flua_init, 0, 0, "Initialize lua state", NULL);
	DEFUN("lua-core-close", Flua_close, 1, 1, "Close lua state", NULL);
	DEFUN("lua-core-do-string", Flua_do_string, 2, 2, "Eval string as Lua code", NULL);
	DEFUN("lua-core-get-global", Flua_get_global, 2, 2, "Get lua value as Emacs Lisp value", NULL);
	DEFUN("lua-core-set-global", Flua_set_global, 3, 3, "Set Emacs Lisp value to lua environment", NULL);

#undef DEFUN

	provide(env, "lua-core");
	return 0;
}

/*
  Local Variables:
  c-basic-offset: 8
  indent-tabs-mode: t
  End:
*/
