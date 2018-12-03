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

  template <std::size_t I, typename ... T>
  struct LuaFunctionCallResultTuple_Impl {};

  template <std::size_t I>
  struct LuaFunctionCallResultTuple_Impl<I> {
    static std::tuple<> get(std::vector<LuaValue> &results) {
      return std::make_tuple();
    }
  };

  template <std::size_t I, typename T, typename ... B>
  struct LuaFunctionCallResultTuple_Impl<I, T, B...> {
    static std::tuple<T, B...> get(std::vector<LuaValue> &results) {
      std::tuple<T> begin = std::make_tuple((I < results.size() ? results.at(I) : LuaValue()).get<T>());
      return std::tuple_cat(begin, LuaFunctionCallResultTuple_Impl<I + 1, B...>::get(results));
    }
  };

  template <typename ... T>
  struct LuaFunctionCallResultTuple {
    static std::tuple<T...> get(std::vector<LuaValue> &results) {
      return LuaFunctionCallResultTuple_Impl<0, T...>::get(results);
    }
  };

  template <typename T>
  struct LuaFunctionCallResultGetter {
    static T get(std::vector<LuaValue> &result) {
      LuaValue ret;
      if (!result.empty()) {
        ret = result.at(0);
      }
      return ret.get<T>();
    }
  };

  template <typename ... T>
  struct LuaFunctionCallResultGetter<std::tuple<T...>> {
    static std::tuple<T...> get(std::vector<LuaValue> &result) {
      return LuaFunctionCallResultTuple<T...>::get(result);
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
      return LuaFunctionCallResultGetter<T>::get(this->result);
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