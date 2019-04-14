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

#ifndef LUACPPB_INVOKE_IMPL_LUA_H_
#define LUACPPB_INVOKE_IMPL_LUA_H_

#include "luacppb/Invoke/Lua.h"

namespace LuaCppB::Internal {

  template <typename P>
  void LuaFunctionArgument_Impl<P>::push(lua_State *state, LuaCppRuntime &runtime) {}

  template <typename P, typename B, typename ... A>
  void LuaFunctionArgument_Impl<P, B, A...>::push(lua_State *state, LuaCppRuntime &runtime, B &&arg, A &&... args) {
    P::push(state, runtime, arg);
    LuaFunctionArgument_Impl<P, A...>::push(state, runtime, std::forward<A>(args)...);
  }

  template <typename P, typename ... A>
  void LuaFunctionArgument<P, A...>::push(lua_State *state, LuaCppRuntime &runtime, A &&... args) {
    LuaFunctionArgument_Impl<P, A...>::push(state, runtime, std::forward<A>(args)...);
  }

  template <typename ... A>
  LuaError LuaFunctionCall::call(lua_State *state, int index, LuaCppRuntime &runtime, std::vector<LuaValue> &result, A &&... args) {
    Internal::LuaStack stack(state);
    int top = stack.getTop();
    stack.copy(index);
    LuaFunctionArgument<Internal::LuaNativeValue, A...>::push(state, runtime, std::forward<A>(args)...);
    int status = lua_pcall(state, sizeof...(args), LUA_MULTRET, 0);
    if (status == static_cast<int>(LuaStatusCode::Ok)) {
      int results = stack.getTop() - top;
      while (results-- > 0) {
        result.push_back(LuaValue::peek(state).value());
        stack.pop();
      }
      std::reverse(result.begin(), result.end());
      return LuaError();
    } else {
      LuaCppBNativeException::check(state);
      std::optional<LuaValue> value = stack.get();
      stack.pop();
      return LuaError(static_cast<LuaStatusCode>(status), value.value_or(LuaValue::Nil));
    }
  }

#ifdef LUACPPB_COROUTINE_SUPPORT

  template <typename ... A>
  void LuaFunctionCall::callK(lua_State *state, int index, LuaCppRuntime &runtime, std::unique_ptr<LuaFunctionContinuation> cont, A &&... args) {
    Internal::LuaStack stack(state);
    int top = stack.getTop();
    stack.copy(index);
    LuaFunctionArgument<Internal::LuaNativeValue, A...>::push(state, runtime, std::forward<A>(args)...);
    LuaFunctionContinuationHandle *handle = new LuaFunctionContinuationHandle(std::move(cont), runtime, top);
    lua_KContext ctx = reinterpret_cast<lua_KContext>(handle);
    LuaFunctionContinuationHandle::fnContinuation(state,
      lua_pcallk(state, sizeof...(args), LUA_MULTRET, 0,
                ctx, &LuaFunctionContinuationHandle::fnContinuation),
      ctx);
  }

  template <typename ... A>
  void LuaFunctionCall::yieldK(lua_State *state, LuaCppRuntime &runtime, std::unique_ptr<LuaFunctionContinuation> cont, A &&... args) {
    Internal::LuaStack stack(state);
    int top = stack.getTop();
    LuaFunctionArgument<Internal::LuaNativeValue, A...>::push(state, runtime, std::forward<A>(args)...);
    LuaFunctionContinuationHandle *handle = new LuaFunctionContinuationHandle(std::move(cont), runtime, top);
    lua_KContext ctx = reinterpret_cast<lua_KContext>(handle);
    LuaFunctionContinuationHandle::fnContinuation(state,
      lua_yieldk(state, sizeof...(args),
                ctx, &LuaFunctionContinuationHandle::fnContinuation),
      ctx);
  }

#endif

  template <std::size_t I>
  std::tuple<> LuaFunctionCallResultTuple_Impl<I>::get(std::vector<LuaValue> &results) {
    return std::make_tuple();
  }

  template <std::size_t I, typename T, typename ... B>
  std::tuple<T, B...> LuaFunctionCallResultTuple_Impl<I, T, B...>::get(std::vector<LuaValue> &results) {
    std::tuple<T> begin = std::make_tuple((I < results.size() ? results.at(I) : LuaValue::Nil).get<T>());
    return std::tuple_cat(begin, LuaFunctionCallResultTuple_Impl<I + 1, B...>::get(results));
  }

  template <typename ... T>
  std::tuple<T...> LuaFunctionCallResultTuple<T...>::get(std::vector<LuaValue> &results) {
    return LuaFunctionCallResultTuple_Impl<0, T...>::get(results);
  }

  template <typename T>
  T LuaFunctionCallResultGetter<T>::get(std::vector<LuaValue> &result) {
    LuaValue ret;
    if (!result.empty()) {
      ret = result.at(0);
    }
    if constexpr (!std::is_same<T, LuaValue>::value) {
      return ret.get<T>();
    } else {
      return ret;
    }
  }

  template <typename ... T>
  std::tuple<T...> LuaFunctionCallResultGetter<std::tuple<T...>>::get(std::vector<LuaValue> &result) {
    return LuaFunctionCallResultTuple<T...>::get(result);
  }

  template <typename A, typename B>
  std::pair<A, B> LuaFunctionCallResultGetter<std::pair<A, B>>::get(std::vector<LuaValue> &result) {
    if (result.size() >= 2) {
      return std::make_pair(result.at(0).get<A>(), result.at(1).get<B>());
    } else if (result.size() == 1) {
      return std::make_pair(result.at(0).get<A>(), LuaValue::Nil.get<B>());
    } else {
      return std::make_pair(LuaValue::Nil.get<A>(), LuaValue::Nil.get<B>());
    }
  }

  template <typename T>
  T LuaFunctionCallResult::get() {
    return LuaFunctionCallResultGetter<T>::get(this->result);
  }

  template <typename T>
  LuaFunctionCallResult::operator T() {
    return this->get<T>();
  }

  template<typename H, typename R, typename ... A>
  typename LuaLambdaBuilder<H, R(A...)>::FunctionType LuaLambdaBuilder<H, R(A...)>::create(H &&handle) {
    return [handle](A &&... args) {
      if constexpr (std::is_void<R>::value) {
        handle(std::forward<A>(args)...);
      } else {
        return static_cast<R>(handle(std::forward<A>(args)...));
      }
    };
  }

  template<typename H, typename R, typename ... A>
  typename LuaLambdaBuilder<H, R(*)(A...)>::FunctionType LuaLambdaBuilder<H, R(*)(A...)>::create(H &&handle) {
    return [handle](A &&... args) {
      if constexpr (std::is_void<R>::value) {
        handle(std::forward<A>(args)...);
      } else {
        return static_cast<R>(handle(std::forward<A>(args)...));
      }
    };
  }

  template<typename H, typename C, typename R, typename ... A>
  typename LuaLambdaBuilder<H, R (C::*)(A...) const>::FunctionType LuaLambdaBuilder<H, R (C::*)(A...) const>::create(H &&handle) {
    return [handle](A &&... args) {
      if constexpr (std::is_void<R>::value) {
        handle(std::forward<A>(args)...);
      } else {
        return handle(std::forward<A>(args)...).template get<R>();
      }
    };
  }

  template <typename H>
  LuaLambdaHandle<H>::LuaLambdaHandle(H &&handle) : handle(handle) {}

  template <typename H>
  template <typename T>
  LuaLambdaHandle<H>::operator T() {
    return LuaLambdaBuilder<H, T>::create(std::forward<H>(this->handle));
  }
}

#endif
