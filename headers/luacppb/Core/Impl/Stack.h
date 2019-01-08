#ifndef LUACPPB_CORE_IMPL_STACK_H_
#define LUACPPB_CORE_IMPL_STACK_H_

#include "luacppb/Core/Stack.h"

namespace LuaCppB::Internal {

  template <bool R>
  void LuaStack::setIndex(int index, int idx) {
    lua_seti(this->state, index, idx);
  }

  template <bool R>
  void LuaStack::getIndex(int index, int idx) {
    lua_geti(this->state, index, idx);
  }

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
  void LuaStack::setIndex<true>(int, int);
  template <>
  void LuaStack::getIndex<true>(int, int);
  
  template <>
  bool LuaStack::is<LuaType::None>(int);
  template <>
  bool LuaStack::is<LuaType::Nil>(int);
  template <>
  bool LuaStack::is<LuaType::Number>(int);
  template <>
  bool LuaStack::is<LuaType::Boolean>(int);
  template <>
  bool LuaStack::is<LuaType::String>(int);
  template <>
  bool LuaStack::is<LuaType::Function>(int);
  template <>
  bool LuaStack::is<LuaType::Table>(int);
  template <>
  bool LuaStack::is<LuaType::LightUserData>(int);
  template <>
  bool LuaStack::is<LuaType::UserData>(int);
  template <>
  bool LuaStack::is<LuaType::Thread>(int);
}

#endif