#ifndef LUACPPB_INVOKE_CONTINUATION_H_
#define LUACPPB_INVOKE_CONTINUATION_H_

#include "luacppb/Base.h"
#include "luacppb/Invoke/Invoke.h"
#include "luacppb/Reference/Handle.h"
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

  template <typename R, typename ... A>
  struct FnWrapper {
    using FunctionType = R(A...);
    using LambdaType = std::function<FunctionType>;
    FnWrapper(LambdaType lambda) : lambda(lambda) {}

    LambdaType lambda;
  };

  template <typename B, typename ... R>
  class LuaContinuationHandler : public LuaFunctionContinuation {
   public:
    LuaContinuationHandler(FnWrapper<B, R...> callback)
      : callback(callback) {}
    
    int call(lua_State *state, LuaCppRuntime &runtime, LuaStatusCode status, std::vector<LuaValue> &result) override {
      std::tuple<R...> args = ContinuationCallbackArguments<R...>::value(result);
      if (status == LuaStatusCode::Ok || status == LuaStatusCode::Yield) {
        if constexpr (std::is_void<B>::value) {
            std::apply(this->callback.lambda, args);
            return 0;
        } else {
          B value = std::apply(this->callback.lambda, args);
          return NativeFunctionResult<B>::set(state, runtime, value);
        }
      }
      return 0;
    }
   private:
    FnWrapper<B, R...> callback;
  };  
  
  template <typename ... R>
  class LuaContinuation {
   public:
    LuaContinuation(const LuaReferenceHandle &handle, LuaCppRuntime &runtime)
      : handle(handle), runtime(runtime) {}
    
    template <typename B, typename ... A>
    void call(FnWrapper<B, R...> callback, A &... args) {
      LuaFunctionInvoke::invokeK<LuaReference, A...>(handle.getReference(), this->runtime, std::make_unique<LuaContinuationHandler<B, R...>>(callback), args...);
    }

    template <typename B, typename ... A>
    typename std::enable_if<(sizeof...(A) > 0)>::type
      call(FnWrapper<B, R...> callback, A &&... args) {
      LuaContinuation<R...>::call<B, A...>(callback, args...);
    }

    template <typename B, typename ... A>
    static void yield(lua_State *state, LuaCppRuntime &runtime, FnWrapper<B, R...> callback, A &... args) {
      LuaFunctionInvoke::yieldK<A...>(state, runtime, std::make_unique<LuaContinuationHandler<B, R...>>(callback), args...);
    }

    template <typename B, typename ... A>
    static typename std::enable_if<(sizeof...(A) > 0)>::type
      yield(lua_State *state, LuaCppRuntime &runtime, FnWrapper<B, R...> callback, A &&... args) {
      LuaContinuation<R...>::yield<B, A...>(state, runtime, callback, args...);
    }
   private:

    LuaReferenceHandle handle;
    LuaCppRuntime &runtime;
  };
}

#endif