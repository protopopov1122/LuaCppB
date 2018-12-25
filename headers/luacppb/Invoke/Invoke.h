#ifndef LUACPPB_INVOKE_INVOKE_H_
#define LUACPPB_INVOKE_INVOKE_H_

#include "luacppb/Reference/Base.h"
#include "luacppb/Invoke/Native.h"
#include "luacppb/Invoke/Lua.h"
#include "luacppb/Invoke/Coro.h"
#include <memory>

namespace LuaCppB {

  class LuaFunctionInvoke {
   public:
		template <typename T, typename ... A>
		static LuaFunctionCallResult invoke(T &ref, LuaCppRuntime &runtime, A &... args) {
			std::vector<LuaValue> results;
			LuaStatusCode status = LuaStatusCode::Ok;
			ref.putOnTop([&](lua_State *state) {
				if (lua_isfunction(state, -1)) {
					status = LuaFunctionCall::call<A...>(state, -1, runtime, results, args...);
				} else if (lua_isthread(state, -1)) {
					LuaCoroutine coro(LuaThread(state, -1), runtime);
					status = coro.call(results, args...);
				}
			});
			return LuaFunctionCallResult(results, status);
		}

		template <typename T, typename ... A>
		static void invokeK(T &ref, LuaCppRuntime &runtime, std::unique_ptr<LuaFunctionContinuation> cont, A &... args) {
			ref.putOnTop([&](lua_State *state) {
				if (lua_isfunction(state, -1)) {
					LuaFunctionCall::callK<A...>(state, -1, runtime, std::move(cont), args...);
				} else if (lua_isthread(state, -1)) {
          std::vector<LuaValue> results;
					LuaCoroutine coro(LuaThread(state, -1), runtime);
					LuaStatusCode status = coro.call(results, args...);
          cont->call(state, runtime, status, results);
				}
			});
		}

		template <typename ... A>
		static void yieldK(lua_State *state, LuaCppRuntime &runtime, std::unique_ptr<LuaFunctionContinuation> cont, A &... args) {
			LuaFunctionCall::yieldK<A...>(state, runtime, std::move(cont), args...);
		}
  };
}

#endif