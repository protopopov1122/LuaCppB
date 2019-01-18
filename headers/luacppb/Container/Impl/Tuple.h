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

#ifndef LUACPPB_CONTAINER_IMPL_TUPLE_H_
#define LUACPPB_CONTAINER_IMPL_TUPLE_H_

#include "luacppb/Container/Tuple.h"

namespace LuaCppB::Internal {

  template <std::size_t I, class P, typename ... A>
  void LuaCppTuplePush_Impl<I, P, A...>::push(lua_State *state, LuaCppRuntime &runtime, T &tuple) {
    Internal::LuaStack stack(state);
    if constexpr (I < std::tuple_size<T>::value) {
      P::push(state, runtime, std::get<I>(tuple));
      stack.setIndex<true>(-2, I + 1);
      LuaCppTuplePush_Impl<I + 1, P, A...>::push(state, runtime, tuple);
    }
  }

  template <class P>
  template <typename ... A>
  void LuaCppTuplePush<P>::push(lua_State *state, LuaCppRuntime &runtime, std::tuple<A...> &tuple) {
    Internal::LuaStack stack(state);
    stack.pushTable();
    LuaCppTuplePush_Impl<0, P, A...>::push(state, runtime, tuple);
  }

  template <typename T, class P>
  typename std::enable_if<is_instantiation<std::pair, T>::value>::type
    LuaCppTuple::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    LuaCppTuple::pushPair<typename T::first_type, typename T::second_type, P>(state, runtime, value);
  }

  template <typename T, class P>
  typename std::enable_if<is_instantiation<std::tuple, T>::value>::type
    LuaCppTuple::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    LuaCppTuplePush<P>::push(state, runtime, value);
  }
  
  template <typename A, typename B, class P>
  void LuaCppTuple::pushPair(lua_State *state, LuaCppRuntime &runtime, std::pair<A, B> &pair) {
    Internal::LuaStack stack(state);
    stack.pushTable();
    P::push(state, runtime, pair.first);
    stack.setIndex<true>(-2, 1);
    P::push(state, runtime, pair.second);
    stack.setIndex<true>(-2, 2);
  }
}

#endif
