#ifndef LUACPPB_INVOKE_CONTINUATION_H_
#define LUACPPB_INVOKE_CONTINUATION_H_

#include "luacppb/Base.h"
#include "luacppb/Invoke/Invoke.h"
#include "luacppb/Reference/Handle.h"
#include "luacppb/Core/Stack.h"
#include <functional>

namespace LuaCppB {


	template <std::size_t I, typename ... Ts>
	struct ContinuationCallbackArguments_Impl {};

	template <std::size_t I>
	struct ContinuationCallbackArguments_Impl<I> {
		static std::tuple<> value(std::vector<LuaValue> &result) {
			return std::make_tuple();
		}
	};

	template <std::size_t I, typename T, typename ... Ts>
	struct ContinuationCallbackArguments_Impl<I, T, Ts...> {
		static std::tuple<T, Ts...> value(std::vector<LuaValue> &result) {
			return std::tuple_cat(std::forward_as_tuple(result.at(I).get<T>()), ContinuationCallbackArguments_Impl<I + 1, Ts...>::value(result));
		};
	};
	
	template <typename ... A>
	struct ContinuationCallbackArguments {
		static std::tuple<A...> value(std::vector<LuaValue> &result) {
			return ContinuationCallbackArguments_Impl<0, A...>::value(result);
		}
	};

  template <typename F, typename ... A>
  class LuaContinuationHandler : public LuaFunctionContinuation {
    using R = decltype(std::declval<F>()(std::declval<A>()...));
   public:
    LuaContinuationHandler(F callback)
      : callback(callback) {}
    
    int call(lua_State *state, LuaCppRuntime &runtime, LuaStatusCode status, std::vector<LuaValue> &result) override {
      std::tuple<A...> args = ContinuationCallbackArguments<A...>::value(result);
      LuaStack stack(state);
      if (status == LuaStatusCode::Ok || status == LuaStatusCode::Yield) {
        if constexpr (std::is_void<R>::value) {
            std::apply(this->callback, args);
            return 0;
        } else {
          R value = std::apply(this->callback, args);
          return NativeFunctionResult<R>::set(state, runtime, value);
        }
      }
      return 0;
    }
   private:
    F callback;
  };  
  
  template <typename ... R>
  class LuaContinuation {
   public:
    LuaContinuation(const LuaReferenceHandle &handle, LuaCppRuntime &runtime)
      : handle(handle), runtime(runtime) {}
    
    template <typename F, typename ... A>
    void call(F callback, A &... args) {
      LuaFunctionInvoke::invokeK<LuaReference, A...>(handle.getReference(), this->runtime, std::make_unique<LuaContinuationHandler<F, R...>>(callback), args...);
    }

    template <typename F, typename ... A>
    typename std::enable_if<(sizeof...(A) > 0)>::type
      call(F callback, A &&... args) {
      LuaContinuation<R...>::call<F, A...>(callback, args...);
    }

    template <typename F, typename ... A>
    static void yield(lua_State *state, LuaCppRuntime &runtime, F callback, A &... args) {
      LuaFunctionInvoke::yieldK<A...>(state, runtime, std::make_unique<LuaContinuationHandler<F, R...>>(callback), args...);
    }

    template <typename F, typename ... A>
    static typename std::enable_if<(sizeof...(A) > 0)>::type
      yield(lua_State *state, LuaCppRuntime &runtime, F callback, A &&... args) {
      LuaContinuation<R...>::yield<F, A...>(state, runtime, callback, args...);
    }
   private:

    LuaReferenceHandle handle;
    LuaCppRuntime &runtime;
  };
}

#endif