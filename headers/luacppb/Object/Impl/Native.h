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

#ifndef LUACPPB_OBJECT_IMPL_NATIVE_H_
#define LUACPPB_OBJECT_IMPL_NATIVE_H_

#include "luacppb/Object/Native.h"


namespace LuaCppB::Internal {

  template <typename T>
  typename std::enable_if<std::is_pointer<T>::value>::type LuaNativeObject::push(lua_State *state, LuaCppRuntime &runtime, T value) {
    using P = typename std::remove_pointer<T>::type;
    Internal::LuaStack stack(state);
    LuaCppObjectBoxerRegistry &boxer = runtime.getObjectBoxerRegistry();
    if (boxer.canWrap<P>()) {
      boxer.wrap(state, value);
    } else {
      stack.push();
    }
  }

  template <typename T>
  typename std::enable_if<!std::is_pointer<T>::value && !is_smart_pointer<T>::value>::type
    LuaNativeObject::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    Internal::LuaStack stack(state);
    LuaCppObjectBoxerRegistry &boxer = runtime.getObjectBoxerRegistry();
    if (boxer.canWrap<T>()) {
      boxer.wrap(state, &value);
    } else {
      stack.push();
    }
  }

  template <typename T>
  typename std::enable_if<is_instantiation<std::unique_ptr, T>::value>::type
    LuaNativeObject::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    using V = typename T::element_type;
    Internal::LuaStack stack(state);
    LuaCppObjectBoxerRegistry &boxer = runtime.getObjectBoxerRegistry();
    if (boxer.canWrap<V>()) {
      boxer.wrap(state, std::move(value));
    } else {
      stack.push();
    }
  }

  template <typename T>
  typename std::enable_if<is_instantiation<std::shared_ptr, T>::value>::type
    LuaNativeObject::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    using V = typename T::element_type;
    Internal::LuaStack stack(state);
    LuaCppObjectBoxerRegistry &boxer = runtime.getObjectBoxerRegistry();
    if (boxer.canWrap<V>()) {
      boxer.wrap(state, value);
    } else {
      stack.push();
    }
  }
}

#endif
