#ifndef LUACPPB_INVOKE_PROMISE_H_
#define LUACPPB_INVOKE_PROMISE_H_

#include "luacppb/Base.h"
#include "luacppb/Invoke/Invoke.h"
#include "luacppb/Reference/Handle.h"
#include <functional>

namespace LuaCppB {


	template <std::size_t I, typename ... Ts>
	struct PromiseCallbackArguments_Impl {};

	template <std::size_t I>
	struct PromiseCallbackArguments_Impl<I> {
		static std::tuple<> value(std::vector<LuaValue> &result) {
			return std::make_tuple();
		}
	};

	template <std::size_t I, typename T, typename ... Ts>
	struct PromiseCallbackArguments_Impl<I, T, Ts...> {
		static std::tuple<T, Ts...> value(std::vector<LuaValue> &result) {
			return std::tuple_cat(std::forward_as_tuple(result.at(I).get<T>()), PromiseCallbackArguments_Impl<I + 1, Ts...>::value(result));
		};
	};
	
	template <typename ... A>
	struct PromiseCallbackArguments {
		static std::tuple<A...> value(std::vector<LuaValue> &result) {
			return PromiseCallbackArguments_Impl<0, A...>::value(result);
		}
	};

  template <typename ... R>
  class LuaPromiseContinuation : public LuaFunctionContinuation {
   public:
    LuaPromiseContinuation(std::function<void(R...)> callback)
      : callback(callback) {}
    
    int call(LuaStatusCode status, std::vector<LuaValue> &result) override {
      if (status == LuaStatusCode::Ok || status == LuaStatusCode::Yield) {
        std::tuple<R...> args = PromiseCallbackArguments<R...>::value(result);
        std::apply(this->callback, args);
      }
      return 0;
    }
   private:
    std::function<void(R...)> callback;
  };

  template <typename ... R>
  class LuaPromise {
   public:
    LuaPromise(const LuaReferenceHandle &handle, LuaCppRuntime &runtime)
      : handle(handle), runtime(runtime) {}
    
    template <typename ... A>
    void call(std::function<void(R...)> callback, A &... args) {
      LuaFunctionInvoke<LuaReference>::invokeK<A...>(handle.getReference(), this->runtime, std::make_unique<LuaPromiseContinuation<R...>>(callback), args...);
    }

    template <typename ... A>
    void call(std::function<void(R...)> callback, A &&... args) {
      LuaFunctionInvoke<LuaReference>::invokeK<A...>(handle.getReference(), this->runtime, std::make_unique<LuaPromiseContinuation<R...>>(callback), args...);
    }
   private:
    LuaReferenceHandle handle;
    LuaCppRuntime &runtime;
  };
}

#endif