#ifndef LUACPPB_INVOKE_INVOKE_H_
#define LUACPPB_INVOKE_INVOKE_H_

#include "luacppb/Reference/Base.h"
#include "luacppb/Invoke/Native.h"
#include "luacppb/Invoke/Lua.h"
#include "luacppb/Invoke/Coro.h"
#include <memory>

namespace LuaCppB::Internal {

  class LuaFunctionInvoke {
   public:
		template <typename T, typename ... A>
		static LuaFunctionCallResult invoke(T &, LuaCppRuntime &, A &&...);

		template <typename T, typename ... A>
		static void invokeK(T &, LuaCppRuntime &, std::unique_ptr<LuaFunctionContinuation>, A &&...);

		template <typename ... A>
		static void yieldK(lua_State *, LuaCppRuntime &, std::unique_ptr<LuaFunctionContinuation>, A &&...);
  };
}

#include "luacppb/Invoke/Impl/Invoke.h"

#endif