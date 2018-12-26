#ifndef LUACPPB_INVOKE_CORO_H_
#define LUACPPB_INVOKE_CORO_H_

#include "luacppb/Base.h"
#include "luacppb/Core/Status.h"
#include "luacppb/Core/Runtime.h"
#include "luacppb/Value/Native.h"
#include "luacppb/Invoke/Lua.h"

namespace LuaCppB {

  class LuaCoroutine {
   public:
    LuaCoroutine(lua_State *, int, LuaCppRuntime &);
    LuaCoroutine(LuaThread &, LuaCppRuntime &);
    LuaCoroutine(LuaThread &&, LuaCppRuntime &);
    LuaCoroutine(LuaCppRuntime &);
    LuaCoroutine(const LuaCoroutine &);
    LuaCoroutine &operator=(const LuaCoroutine &);
    LuaStatusCode getStatus() const;

    template <typename ... A>
    Internal::LuaFunctionCallResult operator()(A... args) const {
			std::vector<LuaValue> result;
			LuaStatusCode status = this->call<A...>(result, args...);
      return Internal::LuaFunctionCallResult(result, status);
    }

    template <typename ... A>
    LuaStatusCode call(std::vector<LuaValue> &result, A &... args) const {
      Internal::LuaStack stack(this->thread.toState());
      Internal::LuaFunctionArgument<Internal::LuaNativeValue, A...>::push(thread.toState(), this->runtime, args...);
      LuaStatusCode status = static_cast<LuaStatusCode>(lua_resume(thread.toState(), nullptr, sizeof...(args)));
      int results = stack.getTop();
      while (results-- > 0) {
        result.push_back(LuaValue::peek(this->thread.toState()).value());
        stack.pop();
      }
      std::reverse(result.begin(), result.end());
      return status;
    }
   private:
    
    LuaThread thread;
    LuaCppRuntime &runtime;
  };
}

#endif