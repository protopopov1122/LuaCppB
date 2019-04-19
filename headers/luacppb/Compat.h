/*
  SPDX short identifier: MIT

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

#ifndef LUACPPB_COMPAT_H_
#define LUACPPB_COMPAT_H_

#include "luacppb/Base.h"

#if (LUA_VERSION_NUM < 501)
#error "Minimal supported Lua version is 5.1"
#elif (LUA_VERSION_NUM == 501)
#define LUACPPB_COMPAT_501
#define LUACPPB_COMPAT_502
#elif (LUA_VERSION_NUM == 502)
#define LUACPPB_COMPAT_502
#define LUACPPB_GC_ISRUNNING
#else
#define LUACPPB_GC_ISRUNNING
#define LUACPPB_CONTAINER_PAIRS
#endif

#ifdef LUACPPB_COMPAT_501
#define LUA_OK 0
#define LUA_ERRGCMM LUA_ERRRUN
#define LUA_OPEQ 0
#define LUA_OPLT 1
#define LUA_OPLE 2

int lua_absindex (lua_State *, int);
void luaL_setmetatable (lua_State *, const char *);
int lua_compare (lua_State *, int, int, int);
int lua_rawgetp (lua_State *, int, const void *);
void lua_rawsetp (lua_State *, int, const void *);

#define LUACPPB_EMULATED_MAINTHREAD
constexpr const char LUACPPB_RIDX_MAINTHREAD[] = "LuaCppB_Internal_MainThread";
#define LUACPPB_NO_DEBUG_EXTRAS

#endif

#ifdef LUACPPB_COMPAT_502
int lua_isinteger (lua_State *, int);
int lua_geti(lua_State *, int, lua_Integer);
void lua_seti(lua_State *, int, lua_Integer);
#ifdef LUACPPB_COROUTINE_SUPPORT
#undef LUACPPB_COROUTINE_SUPPORT
#endif
#endif

#endif