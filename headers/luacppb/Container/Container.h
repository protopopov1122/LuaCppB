#ifndef LUACPPB_CONTAINER_CONTAINER_H_
#define LUACPPB_CONTAINER_CONTAINER_H_

#include "luacppb/Container/Vector.h"
#include "luacppb/Container/Tuple.h"
#include "luacppb/Meta.h"
#include <type_traits>

namespace LuaCppB {

  class LuaCppContainer {
   public:
    template <typename T, class P>
    static typename std::enable_if<is_instantiation<std::vector, T>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      using E = typename T::value_type;
      using Pusher = void (*)(lua_State *, LuaCppRuntime &, E &);
      Pusher pusher = &P::push;
      LuaCppVector::push(state, runtime, value, pusher);
    }

    template <typename T, class P>
    static typename std::enable_if<std::is_const<T>::value && is_instantiation<std::vector, typename std::remove_const<T>::type>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      using E = typename T::value_type;
      using Pusher = void (*)(lua_State *, LuaCppRuntime &, E &);
      Pusher pusher = &P::push;
      LuaCppVector::push(state, runtime, value, pusher);
    }

    template <typename T, class P>
    static typename std::enable_if<is_smart_pointer<T>::value && is_instantiation<std::vector, typename T::element_type>::value>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      using E = typename T::element_type::value_type;
      using Pusher = void (*)(lua_State *, LuaCppRuntime &, E &);
      Pusher pusher = &P::push;
      LuaCppVector::push(state, runtime, value, pusher);
    }

    template <typename T, class P>
    static typename std::enable_if<LuaCppTuple::is_tuple<T>()>::type
      push(lua_State *state, LuaCppRuntime &runtime, T &value) {
      LuaCppTuple::push<T, P>(state, runtime, value);
    }

    template <typename T>
    static constexpr bool is_container() {
      return is_instantiation<std::vector, T>::value ||
             LuaCppTuple::is_tuple<T>() ||
             (std::is_const<T>::value && LuaCppContainer::is_container<typename std::remove_const<T>::type>());
    }

  };
}

#endif