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
      static std::tuple<> value(lua_State *state, LuaCppRuntime &runtime, std::vector<LuaValue> &result) {
        return std::make_tuple();
      }
    };



    template <std::size_t I, typename T, typename ... Ts>
    struct ContinuationCallbackArguments_Impl<I, T, Ts...> {
      static std::tuple<T, Ts...> value(lua_State *state, LuaCppRuntime &runtime, std::vector<LuaValue> &result) {
        if constexpr (!std::is_same<T, LuaState>::value) {
          return std::tuple_cat(std::forward_as_tuple(result.at(I).get<T>()), ContinuationCallbackArguments_Impl<I + 1, Ts...>::value(state, runtime, result));
        } else {
          LuaState newState(state, runtime.getRuntimeInfo());
          return std::tuple_cat(std::forward_as_tuple(newState), ContinuationCallbackArguments_Impl<I, Ts...>::value(state, runtime, result));
        }
      };
    };
    
    template <typename ... A>
    struct ContinuationCallbackArguments {
      static std::tuple<A...> value(lua_State *state, LuaCppRuntime &runtime, std::vector<LuaValue> &result) {
        return ContinuationCallbackArguments_Impl<0, A...>::value(state, runtime, result);
      }
    };

    template <typename F, typename ... A>
    class LuaContinuationHandler : public LuaFunctionContinuation {
      using R = decltype(std::declval<F>()(std::declval<A>()...));
    public:
      LuaContinuationHandler(F callback, std::function<void(LuaError)> errorHandler)
        : callback(callback), errorHandler(errorHandler) {}
      
      int call(lua_State *state, LuaCppRuntime &runtime, LuaError error, std::vector<LuaValue> &result) override {
        std::tuple<A...> args = ContinuationCallbackArguments<A...>::value(state, runtime, result);
        Internal::LuaStack stack(state);
        if (!error.hasError()) {
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
    private:
      F callback;
      std::function<void(LuaError)> errorHandler;
    };

    template <typename F>
    struct LuaContinuationHandlerType : public LuaContinuationHandlerType<decltype(&F::operator())> {};

    template <typename C, typename R, typename ... A>
    struct LuaContinuationHandlerType<R(C::*)(A...) const> {
      template <typename F>
      static auto newHandler(F callback, std::function<void(LuaError)> errorHandler) {
        return std::make_unique<Internal::LuaContinuationHandler<F, A...>>(callback, errorHandler);
      }
    };

    template <typename R, typename ... A>
    struct LuaContinuationHandlerType<R(*)(A...)> {
      template <typename F>
      static auto newHandler(F callback, std::function<void(LuaError)> errorHandler) {
        return std::make_unique<Internal::LuaContinuationHandler<F, A...>>(callback, errorHandler);
      }
    };

    template <typename R, typename ... A>
    struct LuaContinuationHandlerType<R(A...)> {
      template <typename F>
      static auto newHandler(F callback, std::function<void(LuaError)> errorHandler) {
        return std::make_unique<Internal::LuaContinuationHandler<F, A...>>(callback, errorHandler);
      }
    };
  }
  
  class LuaContinuation {
   public:
    LuaContinuation(const LuaReferenceHandle &handle, LuaCppRuntime &runtime)
      : handle(handle), runtime(runtime) {}
    
    template <typename F, typename ... A>
    void call(F callback, std::function<void(LuaError)> errorHandler, A &&... args) {
      Internal::LuaFunctionInvoke::invokeK<Internal::LuaReference, A...>(handle.getReference(),
        this->runtime, Internal::LuaContinuationHandlerType<F>::newHandler(callback, errorHandler), std::forward<A>(args)...);
    }

    template <typename S, typename F, typename ... A>
    static void yield(S &env, F callback, std::function<void(LuaError)> errorHandler, A &&... args) {
      Internal::LuaFunctionInvoke::yieldK<A...>(env.getState(), env, Internal::LuaContinuationHandlerType<F>::newHandler(callback, errorHandler), std::forward<A>(args)...);
    }
   private:

    LuaReferenceHandle handle;
    LuaCppRuntime &runtime;
  };
}

#endif

#endif