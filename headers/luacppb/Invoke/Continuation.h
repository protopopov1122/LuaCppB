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
      
      int call(lua_State *, LuaCppRuntime &, LuaError, std::vector<LuaValue> &) override;
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
    LuaContinuation(const LuaReferenceHandle &, LuaCppRuntime &);
    
    template <typename F, typename ... A>
    void call(F, std::function<void(LuaError)>, A &&...);

    template <typename S, typename F, typename ... A>
    static void yield(S &, F, std::function<void(LuaError)>, A &&...);
   private:

    LuaReferenceHandle handle;
    LuaCppRuntime &runtime;
  };
}

#endif

#include "luacppb/Invoke/Impl/Continuation.h"

#endif