#ifndef LUACPPB_CONTAINER_TUPLE_H_
#define LUACPPB_CONTAINER_TUPLE_H_

#include "luacppb/Base.h"
#include "luacppb/Meta.h"
#include "luacppb/Core/Runtime.h"
#include "luacppb/Core/Stack.h"
#include <utility>
#include <tuple>

namespace LuaCppB {

  template <std::size_t I, class P, typename ... A>
  struct LuaCppTuplePush_Impl {
    using T = std::tuple<A...>;
    static void push(lua_State *state, LuaCppRuntime &runtime, T &tuple) {
      LuaStack stack(state);
      if constexpr (I < std::tuple_size<T>::value) {
        P::push(state, runtime, std::get<I>(tuple));
        lua_rawseti(state, -2, I + 1);
        LuaCppTuplePush_Impl<I + 1, P, A...>::push(state, runtime, tuple);
      }
    }
  };

  template <class P>
  struct LuaCppTuplePush {
    template <typename ... A>
    static void push(lua_State *state, LuaCppRuntime &runtime, std::tuple<A...> &tuple) {
      lua_newtable(state);
      LuaCppTuplePush_Impl<0, P, A...>::push(state, runtime, tuple);
    }
  };

  class LuaCppTuple {
   public:
    template <typename T, class P>
    static typename std::enable_if<is_instantiation<std::pair, T>::value>::type push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      LuaCppTuple::pushPair<typename T::first_type, typename T::second_type, P>(state, runtime, value);
    }

    template <typename T, class P>
    static typename std::enable_if<is_instantiation<std::tuple, T>::value>::type push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      LuaCppTuplePush<P>::push(state, runtime, value);
    }

    template <typename T>
    static constexpr bool is_tuple() {
      return is_instantiation<std::pair, T>::value ||
             is_instantiation<std::tuple, T>::value;
    }
   private:
    template <typename A, typename B, class P>
    static void pushPair(lua_State *state, LuaCppRuntime &runtime, std::pair<A, B> &pair) {
      lua_newtable(state);
      P::push(state, runtime, pair.first);
      lua_rawseti(state, -2, 1);
      P::push(state, runtime, pair.second);
      lua_rawseti(state, -2, 2);
    }
  };
}

#endif