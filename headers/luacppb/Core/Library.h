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

#ifndef LUACPPB_CORE_LIBRARY_H_
#define LUACPPB_CORE_LIBRARY_H_

#include "luacppb/Base.h"
#include "luacppb/Reference/Handle.h"
#include "luacppb/Core/State.h"
#include <tuple>
#include <type_traits>

namespace LuaCppB {

  enum class LuaStdLib {
    Base,
    Package,
#ifndef LUACPPB_INTERNAL_COMPAT_501
    Coroutine,
#endif
    String,
#ifndef LUACPPB_INTERNAL_COMPAT_502
    Utf8,
#endif
    Table,
    Math,
    IO,
    OS,
    Debug
  };

  namespace Internal {

    template <typename ... T>
    struct LuaStdLibLoader {};

    template <>
    struct LuaStdLibLoader<> {
      using ReturnType = std::tuple<>;
      static ReturnType load(lua_State *, LuaCppRuntime &, bool) {
        return std::make_tuple();
      }
    };

    template <typename A, typename ... B>
    struct LuaStdLibLoader<A, B...> {
      using ReturnType = decltype(std::tuple_cat(std::make_tuple(std::declval<LuaReferenceHandle>()), std::declval<typename LuaStdLibLoader<B...>::ReturnType>()));
      static ReturnType load(lua_State *, LuaCppRuntime &, bool, A, B...);
    };
  }

  template <typename ... T>
  typename Internal::LuaStdLibLoader<T...>::ReturnType LuaLoadStdLibs(LuaState &, bool, T...);
}

#endif