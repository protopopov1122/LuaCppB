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

#ifndef LUACPPB_INVOKE_IMPL_NATIVE_H_
#define LUACPPB_INVOKE_IMPL_NATIVE_H_

#include "luacppb/Invoke/Native.h"

namespace LuaCppB::Internal {
	template <typename P, typename R, typename ... A>
  NativeFunctionCall<P, R, A...>::NativeFunctionCall(F fn, LuaCppRuntime &runtime) : function(fn), runtime(runtime) {}


	template <typename P, typename R, typename ... A>
  void NativeFunctionCall<P, R, A...>::push(lua_State *state) const {
    Internal::LuaStack stack(state);
    stack.push(reinterpret_cast<void*>(this->function));
    stack.push(&this->runtime);
    stack.push(&NativeFunctionCall<P, R, A...>::function_closure, 2);
  }
  
	template <typename P, typename R, typename ... A>
  int NativeFunctionCall<P, R, A...>::call(F function, LuaCppRuntime &runtime, lua_State *state) {
    std::array<LuaValue, sizeof...(A)> wrappedArgs;
    WrappedFunctionArguments<1, A...>::get(state, wrappedArgs);
    std::tuple<A...> args = Internal::NativeFunctionArgumentsTuple<1, A...>::value(state, runtime, wrappedArgs);
    if constexpr (std::is_void<R>::value) {
      std::apply(function, args);
      return 0;
    } else {
      return Internal::NativeFunctionResult<P, R>::set(state, runtime, std::apply(function, args));
    }
  }

	template <typename P, typename R, typename ... A>
  int NativeFunctionCall<P, R, A...>::function_closure(lua_State *state) {
    try {
      Internal::LuaStack stack(state);
      F fn = stack.toPointer<F>(lua_upvalueindex(1));
      LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(2));
      return NativeFunctionCall<P, R, A...>::call(fn, runtime, state);
    } catch (std::exception &ex) {
      return luacpp_handle_exception(state, std::current_exception());
    }
  }

	template <typename P, typename C, typename R, typename ... A>
  NativeMethodCall<P, C, R, A...>::NativeMethodCall(C *obj, M met, LuaCppRuntime &runtime) : object(obj), method(met), runtime(runtime) {}
	
  template <typename P, typename C, typename R, typename ... A>
  NativeMethodCall<P, C, R, A...>::NativeMethodCall(C &obj, M met, LuaCppRuntime &runtime) : object(&obj), method(met), runtime(runtime) {}
	
  template <typename P, typename C, typename R, typename ... A>
  NativeMethodCall<P, C, R, A...>::NativeMethodCall(const C *obj, Mc met, LuaCppRuntime &runtime)
    : object(const_cast<C *>(obj)), method(Internal::NativeMethodWrapper<C, R, A...>(met).get()), runtime(runtime) {}
  
	template <typename P, typename C, typename R, typename ... A>
  NativeMethodCall<P, C, R, A...>::NativeMethodCall(const C &obj, Mc met, LuaCppRuntime &runtime)
    : object(const_cast<C *>(&obj)), method(Internal::NativeMethodWrapper<C, R, A...>(met).get()), runtime(runtime) {}
  
	template <typename P, typename C, typename R, typename ... A>
  void NativeMethodCall<P, C, R, A...>::push(lua_State *state) const {
    Internal::LuaStack stack(state);
    NativeMethodDescriptor<C, M> *descriptor = NativeMethodDescriptor<C, M>::pushDescriptor(state);
    descriptor->object = this->object;
    descriptor->method = this->method;
    stack.push(&this->runtime);
    stack.push(&NativeMethodCall<P, C, R, A...>::method_closure, 2);
  }

	template <typename P, typename C, typename R, typename ... A>
  int NativeMethodCall<P, C, R, A...>::call(C *object, M &&method, LuaCppRuntime &runtime, lua_State *state) {
    std::array<LuaValue, sizeof...(A)> wrappedArgs;
    WrappedFunctionArguments<1, A...>::get(state, wrappedArgs);
    std::tuple<A...> args = Internal::NativeFunctionArgumentsTuple<1, A...>::value(state, runtime, wrappedArgs);
    if constexpr (std::is_void<R>::value) {
      std::apply([object, method](A... args) {	
        return (object->*method)(args...);
      }, args);
      return 0;
    } else {
      return Internal::NativeFunctionResult<P, R>::set(state, runtime, std::apply([object, method](A... args)->R {	
        return (object->*method)(args...);
      }, args));
    }
  }

	template <typename P, typename C, typename R, typename ... A>
  int NativeMethodCall<P, C, R, A...>::method_closure(lua_State *state) {
    try {
      Internal::LuaStack stack(state);
      NativeMethodDescriptor<C, M> *descriptor = stack.toUserData<NativeMethodDescriptor<C, M> *>(lua_upvalueindex(1));
      LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(2));
      return NativeMethodCall<P, C, R, A...>::call(descriptor->object, std::forward<M>(descriptor->method), runtime, state);
    } catch (std::exception &ex) {
      return luacpp_handle_exception(state, std::current_exception());
    }
  }

  template <typename P>
  template <typename C, typename R, typename ... A>
  NativeMethodCall<P, C, R, A...> NativeMethod<P>::create(C &obj, R (C::*met)(A...), LuaCppRuntime &runtime) {
    return NativeMethodCall<P, C, R, A...>(obj, met, runtime);
  }
  
  template <typename P>
  template <typename C, typename R, typename ... A>
  NativeMethodCall<P, C, R, A...> NativeMethod<P>::create(C *obj, R (C::*met)(A...), LuaCppRuntime &runtime) {
    return NativeMethodCall<P, C, R, A...>(obj, met, runtime);
  }

  template <typename P>
  template <typename C, typename R, typename ... A>
  NativeMethodCall<P, C, R, A...> NativeMethod<P>::create(C &obj, R (C::*met)(A...) const, LuaCppRuntime &runtime) {
    return NativeMethodCall<P, C, R, A...>(obj, met, runtime);
  }
  
  template <typename P>
  template <typename C, typename R, typename ... A>
  NativeMethodCall<P, C, R, A...> NativeMethod<P>::create(C *obj, R (C::*met)(A...) const, LuaCppRuntime &runtime) {
    return NativeMethodCall<P, C, R, A...>(obj, met, runtime);
  }

  template <typename P>
  template <typename C, typename R, typename ... A>
  NativeMethodCall<P, C, R, A...> NativeMethod<P>::create(const C &obj, R (C::*met)(A...) const, LuaCppRuntime &runtime) {
    return NativeMethodCall<P, C, R, A...>(obj, met, runtime);
  }
  
  template <typename P>
  template <typename C, typename R, typename ... A>
  NativeMethodCall<P, C, R, A...> NativeMethod<P>::create(const C *obj, R (C::*met)(A...) const, LuaCppRuntime &runtime) {
    return NativeMethodCall<P, C, R, A...>(obj, met, runtime);
  }

	template <typename P, typename T, typename ... A>
  NativeInvocableCall<P, T, A...>::NativeInvocableCall(T &&inv, LuaCppRuntime &runtime) : invocable(inv), runtime(runtime) {}

	template <typename P, typename T, typename ... A>
  void NativeInvocableCall<P, T, A...>::push(lua_State *state) const {
    Internal::LuaStack stack(state);
    NativeInvocableDescriptor<T> *descriptor = NativeInvocableDescriptor<T>::pushDescriptor(state);
    new(descriptor) NativeInvocableDescriptor(this->invocable);
    stack.push(&this->runtime);
    stack.push(&NativeInvocableCall<P, T, A...>::invocable_closure, 2);
  }

	template <typename P, typename T, typename ... A>
  int NativeInvocableCall<P, T, A...>::call(T &invocable, LuaCppRuntime &runtime, lua_State *state) {
    std::array<LuaValue, sizeof...(A)> wrappedArgs;
    WrappedFunctionArguments<1, A...>::get(state, wrappedArgs);
    std::tuple<A...> args = Internal::NativeFunctionArgumentsTuple<1, A...>::value(state, runtime, wrappedArgs);
    if constexpr (std::is_void<R>::value) {
      std::apply(invocable, args);
      return 0;
    } else {
      return Internal::NativeFunctionResult<P, R>::set(state, runtime, std::apply(invocable, args));
    }
  }

	template <typename P, typename T, typename ... A>
  int NativeInvocableCall<P, T, A...>::invocable_closure(lua_State *state) {
    try {
      Internal::LuaStack stack(state);
      NativeInvocableDescriptor<T> *descriptor = stack.toUserData<NativeInvocableDescriptor<T> *>(lua_upvalueindex(1));
      LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(2));
      return NativeInvocableCall<P, T, A...>::call(descriptor->invocable, runtime, state);
    } catch (std::exception &ex) {
      return luacpp_handle_exception(state, std::current_exception());
    }
  }

	template<typename P, typename T>
  typename NativeInvocableCallBuilder<P, T>::Type
    NativeInvocableCallBuilder<P, T>::create(FunctionType && f, LuaCppRuntime &runtime) {
    return Type(std::forward<FunctionType>(f), runtime);
  }

	template <typename P>
  template <typename R, typename ... A>
  NativeFunctionCall<P, R, A...> NativeInvocable<P>::create(FType<R, A...> func, LuaCppRuntime &runtime) {
    return NativeFunctionCall<P, R, A...>(std::forward<FType<R, A...>>(func), runtime);
  }

  template <typename P>
  template <typename F>
  typename NativeInvocableCallBuilder<P, F>::Type NativeInvocable<P>::create(F func, LuaCppRuntime &runtime) {
    return NativeInvocableCallBuilder<P, F>::create(std::forward<F>(func), runtime);
  }
}

#endif
