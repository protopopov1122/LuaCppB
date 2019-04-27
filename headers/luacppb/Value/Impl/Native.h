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
  typename std::enable_if<LuaValueFastPath::isSupported<T>()>::type
    LuaNativeValue::push(lua_State *state, LuaCppRuntime &runtime, T value) {
    LuaValueFastPath::push(state, value);
  }
  
  template <typename T>
  typename std::enable_if<LuaValue::is_constructible<T>() && !LuaValueFastPath::isSupported<T>()>::type
    LuaNativeValue::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    LuaValue::create<T>(value).push(state);
  }

  template <typename T>
  typename std::enable_if<LuaValue::is_constructible<T>() && !LuaValueFastPath::isSupported<T>()>::type
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
  typename std::enable_if<LuaNativeValueObjectSmartPointer<T>::value>::type
    LuaNativeValue::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    Internal::LuaNativeObject::push<T>(state, runtime, value);
  }
  
  template <typename T>
  typename std::enable_if<is_instantiation<std::reference_wrapper, T>::value>::type
    LuaNativeValue::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    LuaNativeValue::push<typename T::type>(state, runtime, value.get());
  }
  
  template <typename T>
  typename std::enable_if<LuaNativeValueContainer<T>::value>::type
    LuaNativeValue::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    Internal::LuaCppContainer::push<T, Internal::LuaNativeValue>(state, runtime, value);
  }

  template <typename T>
  typename std::enable_if<LuaNativeValueContainerSmartPointer<T>::value>::type
    LuaNativeValue::push(lua_State *state, LuaCppRuntime &runtime, T &value) {
    Internal::LuaCppContainer::push<T, Internal::LuaNativeValue>(state, runtime, value);
  }
  
  template <typename T>
  typename std::enable_if<is_callable<T>::value && !is_instantiation<std::reference_wrapper, T>::value>::type
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
