#ifndef LUACPPB_INVOKE_INVOKE_H_
#define LUACPPB_INVOKE_INVOKE_H_

#include "luacppb/Reference/Base.h"
#include "luacppb/Invoke/Native.h"
#include "luacppb/Invoke/Lua.h"
#include "luacppb/Invoke/Coro.h"
#include <memory>

namespace LuaCppB {

  template <typename T>
  class LuaFunctionInvoke {
   public:
		template <typename ... A>
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

		template <typename ... A>
		static void invokeK(T &ref, LuaCppRuntime &runtime, std::unique_ptr<LuaFunctionContinuation> cont, A &... args) {
			ref.putOnTop([&](lua_State *state) {
        int top = lua_gettop(state);
				if (lua_isfunction(state, -1)) {
					LuaFunctionCall::callK<A...>(state, -1, top, runtime, std::move(cont), args...);
				} else if (lua_isthread(state, -1)) {
          std::vector<LuaValue> results;
					LuaCoroutine coro(LuaThread(state, -1), runtime);
					LuaStatusCode status = coro.call(results, args...);
          cont->call(status, results);
				}
			});
		}
  };
}

#endif