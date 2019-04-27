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

#ifndef LUACPPB_VALUE_IMPL_FAST_PATH_H_
#define LUACPPB_VALUE_IMPL_FAST_PATH_H_

#include "luacppb/Value/FastPath.h"

namespace LuaCppB::Internal {

  template <typename T>
  typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, T>::type LuaValueFastPath::peek(lua_State *state, int index) {
    return static_cast<T>(lua_tointeger(state, index));
  }

  template <typename T>
  typename std::enable_if<std::is_floating_point<T>::value, T>::type LuaValueFastPath::peek(lua_State *state, int index) {
    return static_cast<T>(lua_tonumber(state, index));
  }

  template <typename T>
  typename std::enable_if<std::is_same<T, bool>::value, T>::type LuaValueFastPath::peek(lua_State *state, int index) {
    return static_cast<T>(lua_toboolean(state, index));
  }

  template <typename T>
  typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value>::type LuaValueFastPath::push(lua_State *state, T value) {
    lua_pushinteger(state, value);
  }

  template <typename T>
  typename std::enable_if<std::is_floating_point<T>::value>::type LuaValueFastPath::push(lua_State *state, T value) {
    lua_pushnumber(state, value);
  }

  template <typename T>
  typename std::enable_if<std::is_same<T, bool>::value>::type LuaValueFastPath::push(lua_State *state, T value) {
    lua_pushboolean(state, static_cast<int>(value));
  }
}

#endif