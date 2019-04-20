/*
  SPDX short identifier: MIT

  Copyright (c) 2015 Kepler Project.
  Copyright 2018-2019 Jevgēnijs Protopopovs

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

/*
  Following compatibility functions were borrowed from lua-compat-5.3 project by Kepler Project.
  See <https://github.com/keplerproject/lua-compat-5.3>.
*/

#include "luacppb/Compat.h"

#ifdef LUACPPB_INTERNAL_COMPAT_501
int lua_absindex (lua_State *L, int i) {
  if (i < 0 && i > LUA_REGISTRYINDEX)
    i += lua_gettop(L) + 1;
  return i;
}

void luaL_setmetatable (lua_State *L, const char *tname) {
  luaL_checkstack(L, 1, "not enough stack slots");
  luaL_getmetatable(L, tname);
  lua_setmetatable(L, -2);
}

int lua_rawgetp (lua_State *L, int i, const void *p) {
  int abs_i = lua_absindex(L, i);
  lua_pushlightuserdata(L, (void*)p);
  lua_rawget(L, abs_i);
  return lua_type(L, -1);
}

void lua_rawsetp (lua_State *L, int i, const void *p) {
  int abs_i = lua_absindex(L, i);
  luaL_checkstack(L, 1, "not enough stack slots");
  lua_pushlightuserdata(L, (void*)p);
  lua_insert(L, -2);
  lua_rawset(L, abs_i);
}

static void compat53_call_lua (lua_State *L, char const code[], size_t len,
                               int nargs, int nret) {
  lua_rawgetp(L, LUA_REGISTRYINDEX, (void*)code);
  if (lua_type(L, -1) != LUA_TFUNCTION) {
    lua_pop(L, 1);
    if (luaL_loadbuffer(L, code, len, "=none"))
      lua_error(L);
    lua_pushvalue(L, -1);
    lua_rawsetp(L, LUA_REGISTRYINDEX, (void*)code);
  }
  lua_insert(L, -nargs-1);
  lua_call(L, nargs, nret);
}

static const char compat53_compare_code[] = "local a,b=...\n"
                                            "return a<=b\n";

int lua_compare (lua_State *L, int idx1, int idx2, int op) {
  int result = 0;
  switch (op) {
    case LUA_OPEQ:
      return lua_equal(L, idx1, idx2);
    case LUA_OPLT:
      return lua_lessthan(L, idx1, idx2);
    case LUA_OPLE:
      luaL_checkstack(L, 5, "not enough stack slots");
      idx1 = lua_absindex(L, idx1);
      idx2 = lua_absindex(L, idx2);
      lua_pushvalue(L, idx1);
      lua_pushvalue(L, idx2);
      compat53_call_lua(L, compat53_compare_code,
                        sizeof(compat53_compare_code)-1, 2, 1);
      result = lua_toboolean(L, -1);
      lua_pop(L, 1);
      return result;
    default:
      luaL_error(L, "invalid 'op' argument for lua_compare");
  }
  return 0;
}
#endif

#ifdef LUACPPB_INTERNAL_COMPAT_502
int lua_isinteger (lua_State *L, int index) {
  if (lua_type(L, index) == LUA_TNUMBER) {
    lua_Number n = lua_tonumber(L, index);
    lua_Integer i = lua_tointeger(L, index);
    if (i == n)
      return 1;
  }
  return 0;
}

int lua_geti (lua_State *L, int index, lua_Integer i) {
  index = lua_absindex(L, index);
  lua_pushinteger(L, i);
  lua_gettable(L, index);
  return lua_type(L, -1);
}

void lua_seti (lua_State *L, int index, lua_Integer i) {
  luaL_checkstack(L, 1, "not enough stack slots available");
  index = lua_absindex(L, index);
  lua_pushinteger(L, i);
  lua_insert(L, -2);
  lua_settable(L, index);
}
#endif