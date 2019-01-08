#ifndef LUACPPB_VALUE_IMPL_NATIVE_H_
#define LUACPPB_VALUE_IMPL_NATIVE_H_

#include "luacppb/Value/Native.h"

namespace LuaCppB::Internal {

  template <typename T>
  typename std::enable_if<std::is_base_of<LuaData, T>::value>::type
    LuaNativeValue::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    value.push(state);
  }
  
  template <typename T>
  typename std::enable_if<LuaValue::is_constructible<T>()>::type
    LuaNativeValue::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    LuaValue::create<T>(value).push(state);
  }

  template <typename T>
  typename std::enable_if<LuaValue::is_constructible<T>()>::type
    LuaNativeValue::push(lua_State *state, LuaCppRuntime &runtime, T &&value) {
    LuaValue::create<T>(value).push(state);
  }

  template <typename T>
  typename std::enable_if<std::is_enum<T>::value>::type
    LuaNativeValue::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    using EnumType = typename std::underlying_type<T>::type;
    LuaNativeValue::push(state, runtime, static_cast<EnumType>(value));
  }

  template <typename T>
  typename std::enable_if<!LuaNativeValueSpecialCase<T>::value>::type
    LuaNativeValue::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    Internal::LuaNativeObject::push<T>(state, runtime, value);
  }
  
  template <typename T>
  typename std::enable_if<is_smart_pointer<T>::value && !Internal::LuaCppContainer::is_container<typename T::element_type>()>::type
    LuaNativeValue::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    Internal::LuaNativeObject::push<T>(state, runtime, value);
  }
  
  template <typename T>
  typename std::enable_if<is_instantiation<std::reference_wrapper, T>::value>::type
    LuaNativeValue::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    LuaNativeValue::push<typename T::type>(state, runtime, value.get());
  }
  
  template <typename T>
  typename std::enable_if<Internal::LuaCppContainer::is_container<T>()>::type
    LuaNativeValue::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    Internal::LuaCppContainer::push<T, Internal::LuaNativeValue>(state, runtime, value);
  }

  template <typename T>
  typename std::enable_if<is_smart_pointer<T>::value && Internal::LuaCppContainer::is_container<typename T::element_type>()>::type
    LuaNativeValue::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    Internal::LuaCppContainer::push<T, Internal::LuaNativeValue>(state, runtime, value);
  }
  
  template <typename T>
  typename std::enable_if<is_callable<T>::value>::type
    LuaNativeValue::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    NativeInvocable<Internal::LuaNativeValue>::create(value, runtime).push(state);
  }
  template <typename T>
  typename std::enable_if<std::is_same<T, LuaFunctionCallResult>::value>::type
    LuaNativeValue::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    value.push(state);
  }
}

#endif