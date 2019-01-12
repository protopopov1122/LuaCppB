#ifndef LUACPPB_INVOKE_CORO_H_
#define LUACPPB_INVOKE_CORO_H_

#include "luacppb/Base.h"
#include "luacppb/Core/Status.h"
#include "luacppb/Core/Runtime.h"
#include "luacppb/Value/Native.h"
#include "luacppb/Invoke/Lua.h"

#ifdef LUACPPB_COROUTINE_SUPPORT

namespace LuaCppB {

  class LuaCoroutine : public LuaData {
   public:
    LuaCoroutine(lua_State *, int, LuaCppRuntime &);
    LuaCoroutine(LuaThread &, LuaCppRuntime &);
    LuaCoroutine(LuaThread &&, LuaCppRuntime &);
    LuaCoroutine(LuaCppRuntime &);
    LuaCoroutine(const LuaCoroutine &);
    LuaCoroutine &operator=(const LuaCoroutine &);
    LuaStatusCode getStatus() const;

    void push(lua_State *) const override;

    template <typename ... A>
    Internal::LuaFunctionCallResult operator()(A &&...) const;

    template <typename ... A>
    LuaError call(std::vector<LuaValue> &, A &&...) const;
   private:
    
    LuaThread thread;
    LuaCppRuntime &runtime;
  };
}

#endif

#include "luacppb/Invoke/Impl/Coro.h"

#endif