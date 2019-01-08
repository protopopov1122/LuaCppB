#ifndef LUACPPB_CONTAINER_TUPLE_H_
#define LUACPPB_CONTAINER_TUPLE_H_

#include "luacppb/Base.h"
#include "luacppb/Meta.h"
#include "luacppb/Core/Runtime.h"
#include "luacppb/Core/Stack.h"
#include <utility>
#include <tuple>

namespace LuaCppB::Internal {

  template <std::size_t I, class P, typename ... A>
  struct LuaCppTuplePush_Impl {
    using T = std::tuple<A...>;
    static void push(lua_State *, LuaCppRuntime &, T &);
  };

  template <class P>
  struct LuaCppTuplePush {
    template <typename ... A>
    static void push(lua_State *, LuaCppRuntime &, std::tuple<A...> &);
  };

  class LuaCppTuple {
   public:
    template <typename T, class P>
    static typename std::enable_if<is_instantiation<std::pair, T>::value>::type push(lua_State *, LuaCppRuntime &, T &);

    template <typename T, class P>
    static typename std::enable_if<is_instantiation<std::tuple, T>::value>::type push(lua_State *, LuaCppRuntime &, T &);

    template <typename T>
    static constexpr bool is_tuple() {
      return is_instantiation<std::pair, T>::value ||
             is_instantiation<std::tuple, T>::value;
    }
   private:
    template <typename A, typename B, class P>
    static void pushPair(lua_State *, LuaCppRuntime &, std::pair<A, B> &);
  };
}

#include "luacppb/Container/Impl/Tuple.h"

#endif