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

#ifndef LUACPPB_INVOKE_IMPL_CONTINUATION_H_
#define LUACPPB_INVOKE_IMPL_CONTINUATION_H_

#include "luacppb/Invoke/Continuation.h"

#ifdef LUACPPB_COROUTINE_SUPPORT

namespace LuaCppB {

  namespace Internal {

    template <std::size_t I>
    std::tuple<> ContinuationCallbackArguments_Impl<I>::value(lua_State *state, LuaCppRuntime &runtime, std::vector<LuaValue> &result) {
      return std::make_tuple();
    }

    template <std::size_t I, typename T, typename ... Ts>
    std::tuple<T, Ts...> ContinuationCallbackArguments_Impl<I, T, Ts...>::value(lua_State *state, LuaCppRuntime &runtime, std::vector<LuaValue> &result) {
      if constexpr (!std::is_same<T, LuaState>::value) {
        return std::tuple_cat(std::forward_as_tuple(result.at(I).get<T>()), ContinuationCallbackArguments_Impl<I + 1, Ts...>::value(state, runtime, result));
      } else {
        LuaState newState(state, runtime.getRuntimeInfo());
        return std::tuple_cat(std::forward_as_tuple(newState), ContinuationCallbackArguments_Impl<I, Ts...>::value(state, runtime, result));
      }
    }
    
    template <typename ... A>
    std::tuple<A...> ContinuationCallbackArguments<A...>::value(lua_State *state, LuaCppRuntime &runtime, std::vector<LuaValue> &result) {
      return ContinuationCallbackArguments_Impl<0, A...>::value(state, runtime, result);
    }

    template <typename F, typename ... A>
    LuaContinuationHandler<F, A...>::LuaContinuationHandler(F callback, std::function<void(LuaError)> errorHandler)
      : callback(callback), errorHandler(errorHandler) {}
    
    template <typename F, typename ... A>
    int LuaContinuationHandler<F, A...>::call(lua_State *state, LuaCppRuntime &runtime, LuaError &error, std::vector<LuaValue> &result) {
      std::tuple<A...> args = ContinuationCallbackArguments<A...>::value(state, runtime, result);
      Internal::LuaStack stack(state);
      if (!error.hasError()) {  // lgtm [cpp/empty-block]
        if constexpr (std::is_void<R>::value) {
            std::apply(this->callback, args);
            return 0;
        } else {
          return Internal::NativeFunctionResult<Internal::LuaNativeValue, R>::set(state, runtime, std::apply(this->callback, args));
        }
      } else {
        this->errorHandler(error);
      }
      return 0;
    }

    template <typename C, typename R, typename ... A>
    template <typename F>
    std::unique_ptr<Internal::LuaContinuationHandler<F, A...>> LuaContinuationHandlerType<R(C::*)(A...) const>::newHandler(F callback, std::function<void(LuaError)> errorHandler) {
      return std::make_unique<Internal::LuaContinuationHandler<F, A...>>(callback, errorHandler);
    }

    template <typename R, typename ... A>
    template <typename F>
    std::unique_ptr<Internal::LuaContinuationHandler<F, A...>> LuaContinuationHandlerType<R(*)(A...)>::newHandler(F callback, std::function<void(LuaError)> errorHandler) {
      return std::make_unique<Internal::LuaContinuationHandler<F, A...>>(callback, errorHandler);
    }

    template <typename R, typename ... A>
    template <typename F>
    std::unique_ptr<Internal::LuaContinuationHandler<F, A...>> LuaContinuationHandlerType<R(A...)>::newHandler(F callback, std::function<void(LuaError)> errorHandler) {
      return std::make_unique<Internal::LuaContinuationHandler<F, A...>>(callback, errorHandler);
    }
  }
    
  template <typename F, typename ... A>
  void LuaContinuation::call(F callback, std::function<void(LuaError)> errorHandler, A &&... args) {
    Internal::LuaFunctionInvoke::invokeK<Internal::LuaReference, A...>(handle.getReference(),
      this->runtime, Internal::LuaContinuationHandlerType<F>::newHandler(callback, errorHandler), std::forward<A>(args)...);
  }

  template <typename S, typename F, typename ... A>
  void LuaContinuation::yield(S &env, F callback, std::function<void(LuaError)> errorHandler, A &&... args) {
    Internal::LuaFunctionInvoke::yieldK<A...>(env.getState(), env, Internal::LuaContinuationHandlerType<F>::newHandler(callback, errorHandler), std::forward<A>(args)...);
  }
}

#endif

#endif
