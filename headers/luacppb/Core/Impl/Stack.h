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

#ifndef LUACPPB_CORE_IMPL_STACK_H_
#define LUACPPB_CORE_IMPL_STACK_H_

#include "luacppb/Core/Stack.h"

namespace LuaCppB::Internal {

  template <typename T>
  T LuaStack::toPointer(int index) {
    return reinterpret_cast<T>(const_cast<void *>(lua_topointer(this->state, index)));
  }

  template <typename T>
  T LuaStack::toUserData(int index) {
    return reinterpret_cast<T>(lua_touserdata(this->state, index));
  }

  template <typename T>
  typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value>::type
    LuaStack::push(T value) {
    lua_pushinteger(this->state, static_cast<lua_Integer>(value));
  }

  template <typename T>
  typename std::enable_if<std::is_floating_point<T>::value>::type
    LuaStack::push(T value) {
    lua_pushnumber(this->state, static_cast<lua_Number>(value));
  }

  template <typename T>
  void LuaStack::push(T *pointer) {
    lua_pushlightuserdata(this->state, reinterpret_cast<void *>(pointer));
  }

  template <typename T>
  T *LuaStack::push() {
    return reinterpret_cast<T *>(lua_newuserdata(state, sizeof(T)));
  }

  template <typename T>
  T *LuaStack::checkUserData(int index, const std::string &name) {
    return reinterpret_cast<T *>(const_cast<void *>(luaL_checkudata(this->state, index, name.c_str())));
  }

  template <LuaType T>
  bool LuaStack::is(int index) {
    return false;
  }

  template <>
  inline void LuaStack::setField<true>(int index) {
    lua_rawset(this->state, index);
  }

  template <>
  inline void LuaStack::getField<true>(int index) {
    lua_rawget(this->state, index);
  }

  template <>
  inline void LuaStack::setIndex<true>(int index, int idx) {
    lua_rawseti(this->state, index, idx);
  }

  template <>
  inline void LuaStack::getIndex<true>(int index, int idx) {
    lua_rawgeti(this->state, index, idx);
  }

  template <>
  inline void LuaStack::setIndex<false>(int index, int idx) {
    lua_seti(this->state, index, idx);
  }

  template <>
  inline void LuaStack::getIndex<false>(int index, int idx) {
    lua_geti(this->state, index, idx);
  }

  template <>
  inline void LuaStack::setField<false>(int index) {
    lua_settable(this->state, index);
  }

  template <>
  inline void LuaStack::getField<false>(int index) {
    lua_gettable(this->state, index);
  }  

  template <>
  inline bool LuaStack::is<LuaType::None>(int index) {
    return static_cast<bool>(lua_isnone(this->state, index));
  }

  template <>
  inline bool LuaStack::is<LuaType::Nil>(int index) {
    return static_cast<bool>(lua_isnil(this->state, index));
  }

  template <>
  inline bool LuaStack::is<LuaType::Number>(int index) {
    return static_cast<bool>(lua_isnumber(this->state, index));
  }

  template <>
  inline bool LuaStack::is<LuaType::Boolean>(int index) {
    return static_cast<bool>(lua_isboolean(this->state, index));
  }

  template <>
  inline bool LuaStack::is<LuaType::String>(int index) {
    return static_cast<bool>(lua_isstring(this->state, index));
  }

  template <>
  inline bool LuaStack::is<LuaType::Function>(int index) {
    return static_cast<bool>(lua_isfunction(this->state, index));
  }

  template <>
  inline bool LuaStack::is<LuaType::Table>(int index) {
    return static_cast<bool>(lua_istable(this->state, index));
  }

  template <>
  inline bool LuaStack::is<LuaType::LightUserData>(int index) {
    return static_cast<bool>(lua_islightuserdata(this->state, index));
  }

  template <>
  inline bool LuaStack::is<LuaType::UserData>(int index) {
    return static_cast<bool>(lua_isuserdata(this->state, index));
  }

  template <>
  inline bool LuaStack::is<LuaType::Thread>(int index) {
    return static_cast<bool>(lua_isthread(this->state, index));
  }
}

#endif
