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

#ifndef LUACPPB_CONTAINER_TUPLE_H_
#define LUACPPB_CONTAINER_TUPLE_H_

#include "luacppb/Base.h"
#include "luacppb/Meta.h"
#include "luacppb/Core/Runtime.h"
#include "luacppb/Core/Stack.h"
#include <utility>
#include <tuple>

namespace LuaCppB::Internal {

  template <std::size_t I, class P, typename ... A>
  struct LuaCppTuplePush_Impl {
    using T = std::tuple<A...>;
    static void push(lua_State *, LuaCppRuntime &, T &);
  };

  template <class P>
  struct LuaCppTuplePush {
    template <typename ... A>
    static void push(lua_State *, LuaCppRuntime &, std::tuple<A...> &);
  };

  class LuaCppTuple {
   public:
    template <typename T, class P>
    static typename std::enable_if<is_instantiation<std::pair, T>::value>::type push(lua_State *, LuaCppRuntime &, T &);

    template <typename T, class P>
    static typename std::enable_if<is_instantiation<std::tuple, T>::value>::type push(lua_State *, LuaCppRuntime &, T &);

    template <typename T>
    static constexpr bool is_tuple() {
      return is_instantiation<std::pair, T>::value ||
             is_instantiation<std::tuple, T>::value;
    }
   private:
    template <typename A, typename B, class P>
    static void pushPair(lua_State *, LuaCppRuntime &, std::pair<A, B> &);
  };
}

#include "luacppb/Container/Impl/Tuple.h"

#endif
