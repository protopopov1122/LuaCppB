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