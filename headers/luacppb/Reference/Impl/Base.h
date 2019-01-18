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

#ifndef LUACPPB_REFERENCE_IMPL_BASE_H_
#define LUACPPB_REFERENCE_IMPL_BASE_H_

#include "luacppb/Reference/Base.h"

namespace LuaCppB::Internal {


  template <typename T>
  typename std::enable_if<std::is_same<T, LuaValue>::value, T>::type LuaReference::get() {
    std::optional<LuaValue> value;
    this->putOnTop([&](lua_State *state) {
      value = LuaValue::peek(state);
    });
    return value.value_or(LuaValue());
  }

#ifdef LUACPPB_COROUTINE_SUPPORT
  template <typename T>
  typename std::enable_if<std::is_convertible<LuaValue, T>::value && !std::is_same<T, LuaValue>::value && !std::is_same<T, LuaCoroutine>::value, T>::type
    LuaReference::get() {
    return this->get<LuaValue>().get<T>();
  }

  template <typename T>
  typename std::enable_if<std::is_same<T, LuaCoroutine>::value, T>::type
    LuaReference::get() {
    LuaCoroutine coro(this->getRuntime());
    this->putOnTop([&](lua_State *state) {
      Internal::LuaStack stack(state);
      if (stack.is<LuaType::Thread>(-1)) {
        coro = LuaCoroutine(LuaThread(state, -1), this->getRuntime());
      } else {
        coro = LuaCoroutine(state, -1, this->getRuntime());
      }
    });
    return coro;
  }
#else
  template <typename T>
  typename std::enable_if<std::is_convertible<LuaValue, T>::value && !std::is_same<T, LuaValue>::value, T>::type
    LuaReference::get() {
    return this->get<LuaValue>().get<T>();
  }
#endif

  template <typename T>
  typename std::enable_if<std::is_base_of<LuaData, T>::value>::type LuaReference::set(T &value) {
    this->setValue([&value](lua_State *state) {
      value.push(state);
    });
  }

  template <typename T>
  typename std::enable_if<!std::is_base_of<LuaData, T>::value>::type LuaReference::set(T &value) {
    this->setValue([&](lua_State *state) {
      Internal::LuaNativeValue::push<T>(state, this->runtime, value);
    });
  }
}

#endif
