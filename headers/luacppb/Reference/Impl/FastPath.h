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

#ifndef LUACPPB_REFERENCE_IMPL_FASTPATH_H_
#define LUACPPB_REFERENCE_IMPL_FASTPATH_H_

#include "luacppb/Base.h"

#ifdef LUACPPB_FAST_REFERENCE_SUPPORT

#include "luacppb/Reference/FastPath.h"
#include "luacppb/Reference/Impl/Handle.h"
#include "luacppb/Value/Impl/Native.h"

namespace LuaCppB::Internal {

  template <typename R>
  LuaFastRef<R>::operator LuaReferenceHandle() const {
    return static_cast<const R *>(this)->getHandle();
  }

  template <typename R>
  Internal::LuaReference &LuaFastRef<R>::getReference() const {
    return static_cast<const R *>(this)->getHandle().getReference();
  }

  template <typename R>
  LuaReferenceHandle LuaFastRef<R>::persistent() const {
    return static_cast<const R *>(this)->getHandle().persistent();
  }

  template <typename R>
  TableIterator LuaFastRef<R>::begin() const {
    return static_cast<const R *>(this)->getHandle().begin();
  }

  template <typename R>
  const TableIterator &LuaFastRef<R>::end() const {
    return TableIterator::End;
  }

  template <typename R>
  LuaReferenceHandle LuaFastRef<R>::getMetatable() const {
    return static_cast<const R *>(this)->getHandle().getMetatable();
  }

  template <typename R>
  void LuaFastRef<R>::setMetatable(LuaData &d) {
    static_cast<R *>(this)->getHandle().setMetatable(d);
  }

  template <typename R>
  void LuaFastRef<R>::setMetatable(LuaData &&d) {
    static_cast<R *>(this)->getHandle().setMetatable(std::forward<LuaData>(d));
  }

  template <typename R>
  template <typename T>
  typename std::enable_if<Internal::LuaValueWrapper<T>::Conversible, LuaReferenceHandle>::type LuaFastRef<R>::operator[](T idx) {
    return static_cast<R *>(this)->getHandle()[idx];
  }

  template <typename R>
  template <typename T, typename Type>
  LuaFastRef<R>::operator T () {
    return static_cast<R *>(this)->template get<T>();
  }

  template <typename R>
  template <typename T, typename Type>
  LuaFastRef<R>::operator T& () {
    return static_cast<R *>(this)->template get<T&>();
  }

  template <typename R>
  template <typename ... A>
  Internal::LuaFunctionCallResult LuaFastRef<R>::operator()(A &&... args) const {
    return static_cast<const R *>(this)->getHandle()(std::forward<A>(args)...);
  }

  template <typename T>
  typename std::enable_if<!std::is_same<T, LuaVariableRef>::value, LuaVariableRef>::type &LuaVariableRef::operator=(T &val) {
    lua_State *state = this->state.getState();
    LuaNativeValue::push(state, this->state, val);
    lua_setglobal(state, this->name.c_str());
    return *this;
  }

  template <typename T>
  typename std::enable_if<!std::is_same<T, LuaVariableRef>::value, LuaVariableRef>::type &LuaVariableRef::operator=(T &&val) {
    lua_State *state = this->state.getState();
    LuaNativeValue::push(state, this->state, val);
    lua_setglobal(state, this->name.c_str());
    return *this;
  }

  template <typename T>
  typename std::enable_if<!Internal::is_instantiation<std::function, T>::value && !std::is_same<LuaReferenceHandle, typename std::decay<T>::type>::value, T>::type LuaVariableRef::get() const {
    if constexpr (LuaValueFastPath::isSupported<T>()) {
      if (this->valid()) {
        lua_State *state = this->state.getState();
        lua_getglobal(state, this->name.c_str());
        T value = LuaValueFastPath::peek<T>(state);
        lua_pop(state, 1);
        return value;
      } else {
        return this->getHandle().get<T>();
      }
    } else {
      return this->getHandle().get<T>();
    }
  }
}

#endif

#endif