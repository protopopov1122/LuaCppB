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

#ifndef LUACPPB_CONTAINER_IMPL_CONTAINER_H_
#define LUACPPB_CONTAINER_IMPL_CONTAINER_H_

#include "luacppb/Container/Container.h"

namespace LuaCppB::Internal {

  template <typename T, class P>
  typename std::enable_if<is_instantiation<std::vector, T>::value>::type
    LuaCppContainer::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    LuaCppVector<P>::push(state, runtime, value);
  }

  template <typename T, class P>
  typename std::enable_if<std::is_const<T>::value && is_instantiation<std::vector, typename std::remove_const<T>::type>::value>::type
    LuaCppContainer::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    LuaCppVector<P>::push(state, runtime, value);
  }

  template <typename T, class P>
  typename std::enable_if<is_smart_pointer<T>::value && is_instantiation<std::vector, typename T::element_type>::value>::type
    LuaCppContainer::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    LuaCppVector<P>::push(state, runtime, value);
  }
  
  template <typename T, class P>
  typename std::enable_if<is_instantiation<std::map, T>::value>::type
    LuaCppContainer::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    LuaCppMap<P>::push(state, runtime, value);
  }

  template <typename T, class P>
  typename std::enable_if<std::is_const<T>::value && is_instantiation<std::map, typename std::remove_const<T>::type>::value>::type
    LuaCppContainer::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    LuaCppMap<P>::push(state, runtime, value);
  }

  template <typename T, class P>
  typename std::enable_if<is_smart_pointer<T>::value && is_instantiation<std::map, typename T::element_type>::value>::type
    LuaCppContainer::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    LuaCppMap<P>::push(state, runtime, value);
  }

  template <typename T, class P>
  typename std::enable_if<is_instantiation<std::set, T>::value>::type
    LuaCppContainer::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    LuaCppSet<P>::push(state, runtime, value);
  }

  template <typename T, class P>
  typename std::enable_if<std::is_const<T>::value && is_instantiation<std::set, typename std::remove_const<T>::type>::value>::type
    LuaCppContainer::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    LuaCppSet<P>::push(state, runtime, value);
  }

  template <typename T, class P>
  typename std::enable_if<is_smart_pointer<T>::value && is_instantiation<std::set, typename T::element_type>::value>::type
    LuaCppContainer::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    LuaCppSet<P>::push(state, runtime, value);
  }
  
  template <typename T, class P>
  typename std::enable_if<LuaCppTuple::is_tuple<T>()>::type
    LuaCppContainer::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    LuaCppTuple::push<T, P>(state, runtime, value);
  }

  template <typename T, class P>
  typename std::enable_if<is_instantiation<std::optional, T>::value>::type
    LuaCppContainer::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    if (value.has_value()) {
      P::push(state, runtime, value.value());
    } else {
      Internal::LuaStack stack(state);
      stack.push();
    }
  }
}

#endif
