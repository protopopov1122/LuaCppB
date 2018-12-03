#ifndef LUACPPB_INVOKE_LUA_H_
#define LUACPPB_INVOKE_LUA_H_

#include "luacppb/Base.h"
#include "luacppb/Value/Value.h"
#include "luacppb/Reference/Registry.h"
#include <type_traits>
#include <tuple>
#include <vector>
#include <algorithm>

namespace LuaCppB {

  template <typename ... A>
  struct LuaFunctionArgument_Impl {};

  template <>
  struct LuaFunctionArgument_Impl<> {
    static void push(lua_State *state) {}
  };

  template <typename B, typename ... A>
  struct LuaFunctionArgument_Impl<B, A...> {
    static void push(lua_State *state, B arg, A... args) {
      LuaValue::create<B>(arg).push(state);
      LuaFunctionArgument_Impl<A...>::push(state, args...);
    }
  };

  template <typename ... A>
  struct LuaFunctionArgument {
    static void push(lua_State *state, A... args) {
      LuaFunctionArgument_Impl<A...>::push(state, args...);
    }
  };

  class LuaFunctionCall {
   public:
    template <typename R, typename ... A>
    static R call(lua_State *state, int index, A... args) {
      int top = lua_gettop(state);
      lua_pushvalue(state, index);
      LuaFunctionArgument<A...>::push(state, args...);
      lua_pcall(state, sizeof...(args), LUA_MULTRET, 0);
      int rets = lua_gettop(state) - top;
      if constexpr (std::is_same<R, void>::value) {
        lua_pop(state, rets);
      } else if (rets == 1) {
        R result = LuaValue::peek(state).value().get<R>();
        lua_pop(state, rets);
        return result;
      } else {
        // TODO Handle multiple return results
        lua_pop(state, rets);
      }
    }

    template <typename ... A>
    static void call(lua_State *state, int index, std::vector<LuaValue> &result, A... args) {
      int top = lua_gettop(state);
      lua_pushvalue(state, index);
      LuaFunctionArgument<A...>::push(state, args...);
      lua_pcall(state, sizeof...(args), LUA_MULTRET, 0);
      int results = lua_gettop(state) - top;
      while (results-- > 0) {
        result.push_back(LuaValue::peek(state).value());
        lua_pop(state, 1);
      }
      std::reverse(result.begin(), result.end());
    }
  };

  class LuaFunctionCallResult {
   public:
    LuaFunctionCallResult(std::vector<LuaValue> &result)
      : result(result) {}
    LuaFunctionCallResult(const LuaFunctionCallResult &) = delete;
    LuaFunctionCallResult &operator=(const LuaFunctionCallResult &) = delete;

    template <typename T>
    T get() {
      // TODO Handle multiple return results
      LuaValue ret;
      if (!result.empty()) {
        ret = result.at(0);
      }
      return ret.get<T>();
    }

    template <typename T>
    operator T() {
      return this->get<T>();
    }
   private:
    std::vector<LuaValue> result;
  };
}

#endif