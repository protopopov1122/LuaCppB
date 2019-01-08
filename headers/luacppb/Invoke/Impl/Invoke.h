#ifndef LUACPPB_INVOKE_IMPL_INVOKE_H_
#define LUACPPB_INVOKE_IMPL_INVOKE_H_

#include "luacppb/Invoke/Invoke.h"

namespace LuaCppB::Internal {

  template <typename T, typename ... A>
  LuaFunctionCallResult LuaFunctionInvoke::invoke(T &ref, LuaCppRuntime &runtime, A &&... args) {
    std::vector<LuaValue> results;
    LuaError error;
    ref.putOnTop([&](lua_State *state) {
      if (lua_isfunction(state, -1)) {
        error = LuaFunctionCall::call<A...>(state, -1, runtime, results, std::forward<A>(args)...);
#ifdef LUACPPB_COROUTINE_SUPPORT
      } else if (lua_isthread(state, -1)) {
        LuaCoroutine coro(LuaThread(state, -1), runtime);
        error = coro.call(results, args...);
#endif
      }
    });
    return LuaFunctionCallResult(results, error);
  }

  template <typename T, typename ... A>
  void LuaFunctionInvoke::invokeK(T &ref, LuaCppRuntime &runtime, std::unique_ptr<LuaFunctionContinuation> cont, A &&... args) {
    ref.putOnTop([&](lua_State *state) {
      if (lua_isfunction(state, -1)) {
        LuaFunctionCall::callK<A...>(state, -1, runtime, std::move(cont), std::forward<A>(args)...);
#ifdef LUACPPB_COROUTINE_SUPPORT
      } else if (lua_isthread(state, -1)) {
        std::vector<LuaValue> results;
        LuaCoroutine coro(LuaThread(state, -1), runtime);
        LuaError error = coro.call(results, args...);
        cont->call(state, runtime, error.getStatus(), results);
#endif
      }
    });
  }

  template <typename ... A>
  void LuaFunctionInvoke::yieldK(lua_State *state, LuaCppRuntime &runtime, std::unique_ptr<LuaFunctionContinuation> cont, A &&... args) {
    LuaFunctionCall::yieldK<A...>(state, runtime, std::move(cont), std::forward<A>(args)...);
  }
}

#endif