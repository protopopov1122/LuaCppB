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

#ifndef LUACPPB_INVOKE_CONTINUATION_H_
#define LUACPPB_INVOKE_CONTINUATION_H_

#include "luacppb/Base.h"
#include "luacppb/Invoke/Invoke.h"
#include "luacppb/Reference/Handle.h"
#include "luacppb/Core/Stack.h"
#include "luacppb/Core/State.h"
#include "luacppb/Value/Native.h"
#include <functional>

#ifdef LUACPPB_COROUTINE_SUPPORT

namespace LuaCppB {

  namespace Internal {

    template <std::size_t I, typename ... Ts>
    struct ContinuationCallbackArguments_Impl {};

    template <std::size_t I>
    struct ContinuationCallbackArguments_Impl<I> {
      static std::tuple<> value(lua_State *, LuaCppRuntime &, std::vector<LuaValue> &);
    };



    template <std::size_t I, typename T, typename ... Ts>
    struct ContinuationCallbackArguments_Impl<I, T, Ts...> {
      static std::tuple<T, Ts...> value(lua_State *, LuaCppRuntime &, std::vector<LuaValue> &);
    };
    
    template <typename ... A>
    struct ContinuationCallbackArguments {
      static std::tuple<A...> value(lua_State *, LuaCppRuntime &, std::vector<LuaValue> &);
    };

    template <typename F, typename ... A>
    class LuaContinuationHandler : public LuaFunctionContinuation {
      using R = decltype(std::declval<F>()(std::declval<A>()...));
    public:
      LuaContinuationHandler(F, std::function<void(LuaError)>);
      
      int call(lua_State *, LuaCppRuntime &, LuaError &, std::vector<LuaValue> &) override;
    private:
      F callback;
      std::function<void(LuaError)> errorHandler;
    };

    template <typename F>
    struct LuaContinuationHandlerType : public LuaContinuationHandlerType<decltype(&F::operator())> {};

    template <typename C, typename R, typename ... A>
    struct LuaContinuationHandlerType<R(C::*)(A...) const> {
      template <typename F>
      static std::unique_ptr<Internal::LuaContinuationHandler<F, A...>> newHandler(F, std::function<void(LuaError)>);
    };

    template <typename C, typename R, typename ... A>
    struct LuaContinuationHandlerType<R(C::*)(A...)> {
      template <typename F>
      static std::unique_ptr<Internal::LuaContinuationHandler<F, A...>> newHandler(F, std::function<void(LuaError)>);
    };

    template <typename R, typename ... A>
    struct LuaContinuationHandlerType<R(*)(A...)> {
      template <typename F>
      static std::unique_ptr<Internal::LuaContinuationHandler<F, A...>> newHandler(F, std::function<void(LuaError)>);
    };

    template <typename R, typename ... A>
    struct LuaContinuationHandlerType<R(A...)> {
      template <typename F>
      static std::unique_ptr<Internal::LuaContinuationHandler<F, A...>> newHandler(F, std::function<void(LuaError)>);
    };
  }
  
  class LuaContinuation {
   public:
    LuaContinuation(const LuaReferenceHandle &, LuaState &);
    
    template <typename F, typename ... A>
    void call(F, std::function<void(LuaError)>, A &&...);

    template <typename S, typename F, typename ... A>
    static void yield(S &, F, std::function<void(LuaError)>, A &&...);
   private:

    LuaReferenceHandle handle;
    LuaState &state;
  };
}

#include "luacppb/Invoke/Impl/Continuation.h"

#endif

#endif
