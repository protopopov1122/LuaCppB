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

#ifndef LUACPPB_OBJECT_IMPL_METHOD_H_
#define LUACPPB_OBJECT_IMPL_METHOD_H_

#include "luacppb/Object/Method.h"
#include "luacppb/Core/Impl/Stack.h"

namespace LuaCppB::Internal {

	template <typename C, typename M, typename R, typename ... A>
  LuaCppObjectMethodCall<C, M, R, A...>::LuaCppObjectMethodCall(M met, const std::string &cName, LuaCppRuntime &runtime)
    : method(met), className(cName), runtime(runtime) {}

	template <typename C, typename M, typename R, typename ... A>
  void LuaCppObjectMethodCall<C, M, R, A...>::push(lua_State *state) const {
    Internal::LuaStack stack(state);
    LuaCppObjectMethodCallDescriptor<M> *descriptor = stack.push<LuaCppObjectMethodCallDescriptor<M>>();
    descriptor->method = this->method;
    stack.push(&this->runtime);
    stack.push(this->className);
    stack.push(&LuaCppObjectMethodCall<C, M, R, A...>::class_method_closure, 3);
  }
  
	template <typename C, typename M, typename R, typename ... A>
  int LuaCppObjectMethodCall<C, M, R, A...>::call(C *object, M method, LuaCppRuntime &runtime, lua_State *state) {
    std::array<LuaValue, sizeof...(A)> wrappedArgs;
    WrappedFunctionArguments<2, A...>::get(state, wrappedArgs);
    auto args = NativeFunctionArgumentsTuple<2, A...>::value(state, runtime, wrappedArgs);
    if constexpr (std::is_void<R>::value) {
      std::apply([object, method](A... args) {	
        return (object->*method)(args...);
      }, args);
      return 0;
    } else {
      return NativeFunctionResult<Internal::LuaNativeValue, R>::set(state, runtime, std::apply([object, method](A... args)->R {	
        return (object->*method)(args...);
      }, args));
    }
  }

	template <typename C, typename M, typename R, typename ... A>
  int LuaCppObjectMethodCall<C, M, R, A...>::class_method_closure(lua_State *state) {
    try {
      Internal::LuaStack stack(state);
      LuaCppObjectMethodCallDescriptor<M> *descriptor = stack.toUserData<LuaCppObjectMethodCallDescriptor<M> *>(lua_upvalueindex(1));
      LuaCppRuntime &runtime = *stack.toPointer<LuaCppRuntime *>(lua_upvalueindex(2));
      std::string className = stack.toString(lua_upvalueindex(3));
      LuaCppObjectWrapper<C> *object = checkUserData(className, state, 1);
      if (object) {
        return LuaCppObjectMethodCall<C, M, R, A...>::call(object->get(), descriptor->method, runtime, state);
      } else {
        return 0;
      }
    } catch (std::exception &ex) {
      return luacpp_handle_exception(state, std::current_exception());
    }
  }

	template <typename C, typename M, typename R, typename ... A>
  LuaCppObjectWrapper<C> *LuaCppObjectMethodCall<C, M, R, A...>::checkUserData(const std::string &className, lua_State *state, int index) {
    Internal::LuaStack stack(state);
    if (!stack.is<LuaType::UserData>(index)) {
      return nullptr;
    }
    stack.getMetatable(index);
    stack.push(std::string("__name"));
    stack.getField<true>(-2);
    std::string name = stack.toString(-1);
    stack.pop(2);
    if (name.find(className) == 0) {
      return stack.toPointer<LuaCppObjectWrapper<C> *>(index);
    } else {
      return nullptr;
    }
  }
}

#endif
