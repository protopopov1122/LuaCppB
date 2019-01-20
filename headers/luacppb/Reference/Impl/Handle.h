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

#ifndef LUACPPB_REFERENCE_IMPL_HANDLE_H_
#define LUACPPB_REFERENCE_IMPL_HANDLE_H_

#include "luacppb/Reference/Handle.h"

namespace LuaCppB {

  template <typename T>
  typename std::enable_if<!std::is_same<T, LuaReferenceHandle>::value, LuaReferenceHandle>::type &LuaReferenceHandle::operator=(T &value) {
    try {
      if (this->ref) {
        this->ref->set<T>(value);
      }
    } catch (std::exception &ex) {
      this->getRuntime().getExceptionHandler()(ex);
    }
    return *this;
  }

  template <typename T>
  typename std::enable_if<!std::is_same<T, LuaReferenceHandle>::value, LuaReferenceHandle>::type &LuaReferenceHandle::operator=(T &&value) {
    try {
      if (this->ref) {
        this->ref->set<T>(value);
      }
    } catch (std::exception &ex) {
      this->getRuntime().getExceptionHandler()(ex);
    }
    return *this;
  }

  template <typename T, typename Type>
    LuaReferenceHandle::operator T () {
    try {
      return this->ref->get<T>();
    } catch (std::exception &ex) {
      if constexpr (std::is_default_constructible<T>::value) {
        this->getRuntime().getExceptionHandler()(ex);
        return T{};
      } else {
        throw;
      }
    }
  }

  template <typename T, typename Type>
    LuaReferenceHandle::operator T& () {
    return this->ref->get<T &>();
  }

  template <typename T>
  typename std::enable_if<!Internal::is_instantiation<std::function, T>::value, T>::type LuaReferenceHandle::get() const {
    try {
      return this->ref->get<T>();
    } catch (std::exception &ex) {
      if constexpr (std::is_default_constructible<T>::value) {
        this->getRuntime().getExceptionHandler()(ex);
        return T{};
      } else {
        throw;
      }
    }
  }

  template <typename ... A>
  Internal::LuaFunctionCallResult LuaReferenceHandle::operator()(A &&... args) const {
    try {
      if (this->ref) {
        return Internal::LuaFunctionInvoke::invoke<Internal::LuaReference, A...>(*this->ref, this->getRuntime(), std::forward<A>(args)...);
      }
    } catch (std::exception &ex) {
      this->getRuntime().getExceptionHandler()(ex);
    }
    return Internal::LuaFunctionCallResult(LuaError(LuaStatusCode::RuntimeError));
  }


  template <std::size_t I, typename T>
  T Internal::NativeFunctionArgument<I, T, typename std::enable_if<std::is_same<T, LuaReferenceHandle>::value>::type>::get(lua_State *state, LuaCppRuntime &runtime, LuaValue &value) {
    return LuaState(state, runtime.getRuntimeInfo())[I];
  }
}

#endif
