#include "luacppb/Invoke/Coro.h"

#ifdef LUACPPB_COROUTINE_SUPPORT

namespace LuaCppB {

  LuaCoroutine::LuaCoroutine(lua_State *state, int index, LuaCppRuntime &runtime)
    : thread(LuaThread::create(state)), runtime(runtime) {
    lua_pushvalue(state, index);
    lua_xmove(state, thread.toState(), 1);
  }

  LuaCoroutine::LuaCoroutine(LuaThread &thread, LuaCppRuntime &runtime)
    : thread(thread), runtime(runtime) {}
  
  LuaCoroutine::LuaCoroutine(LuaThread &&thread, LuaCppRuntime &runtime)
    : thread(thread), runtime(runtime) {}

  LuaCoroutine::LuaCoroutine(LuaCppRuntime &runtime)
    : thread(), runtime(runtime) {}

  LuaCoroutine::LuaCoroutine(const LuaCoroutine &coro)
    : thread(coro.thread), runtime(coro.runtime) {}
  
  LuaCoroutine &LuaCoroutine::operator=(const LuaCoroutine &coro) {
    this->thread = coro.thread;
    this->runtime = coro.runtime;
    return *this;
  }

  LuaStatusCode LuaCoroutine::getStatus() const {
    return static_cast<LuaStatusCode>(lua_status(this->thread.toState()));
  }
}

#endif