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

#ifndef LUACPPB_CORE_IMPL_LIBRARY_H_
#define LUACPPB_CORE_IMPL_LIBRARY_H_

#include "luacppb/Core/Library.h"

namespace LuaCppB {

  namespace Internal {

    template <typename A, typename ... B>
    typename LuaStdLibLoader<A, B...>::ReturnType LuaStdLibLoader<A, B...>::load(lua_State *state, LuaCppRuntime &runtime, bool global, A lib, B... libs) {
      lua_CFunction cfun = nullptr;
      const char *modname = nullptr;
      switch (lib) {
        case LuaStdLib::Base:
          cfun = luaopen_base;
          modname = "base";
          break;
        case LuaStdLib::Package:
          cfun = luaopen_package;
          modname = "package";
          break;
#ifndef LUACPPB_INTERNAL_COMPAT_501
        case LuaStdLib::Coroutine:
          cfun = luaopen_coroutine;
          modname = "coroutine";
          break;
#endif
        case LuaStdLib::String:
          cfun = luaopen_string;
          modname = "string";
          break;
#ifndef LUACPPB_INTERNAL_COMPAT_502
        case LuaStdLib::Utf8:
          cfun = luaopen_utf8;
          modname = "utf8";
          break;
#endif
        case LuaStdLib::Table:
          cfun = luaopen_table;
          modname = "table";
          break;
        case LuaStdLib::Math:
          cfun = luaopen_math;
          modname = "math";
          break;
        case LuaStdLib::IO:
          cfun = luaopen_io;
          modname = "io";
          break;
        case LuaStdLib::OS:
          cfun = luaopen_os;
          modname = "os";
          break;
        case LuaStdLib::Debug:
          cfun = luaopen_debug;
          modname = "debug";
          break;
      }
      std::tuple<LuaReferenceHandle> tuple1 = std::make_tuple(LuaReferenceHandle());
      if (cfun) {
#ifndef LUACPPB_INTERNAL_COMPAT_501
        luaL_requiref(state, modname, cfun, static_cast<int>(global));
        tuple1 = std::make_tuple(LuaReferenceHandle(state, std::make_unique<LuaRegistryReference>(state, runtime, -1)));
        lua_pop(state, 1);
#else
        lua_pushcfunction(state, cfun);
        lua_call(state, 0, 0);
#endif
      }
      auto tuple2 = LuaStdLibLoader<B...>::load(state, runtime, global, libs...);
      return std::tuple_cat(tuple1, tuple2);
    }
  }

  template <typename ... T>
  typename Internal::LuaStdLibLoader<T...>::ReturnType LuaLoadStdLibs(LuaState &state, bool global, T... libs) {
    return Internal::LuaStdLibLoader<T...>::load(state.getState(), state, global, libs...);
  }
}

#endif