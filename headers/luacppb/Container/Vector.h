#ifndef LUACPPB_CONTAINER_VECTOR_H_
#define LUACPPB_CONTAINER_VECTOR_H_

#include "luacppb/Base.h"
#include "luacppb/Object/Wrapper.h"
#include "luacppb/Core/Stack.h"
#include <vector>
#include <typeinfo>
#include <type_traits>

namespace LuaCppB::Internal {

  template <typename T, typename A, typename E = void>
  struct cpp_vector_newindex {
    static int newindex(lua_State *);
  };

  template <typename T, typename A>
  struct cpp_vector_newindex<T, A, typename std::enable_if<std::is_same<decltype(std::declval<LuaValue>().get<T>()), T>::value>::type> {
    static int newindex(lua_State *);
  };

  template <class P>
  class LuaCppVector {
   public:
    template <typename V>
    static typename std::enable_if<is_instantiation<std::vector, V>::value>::type
      push(lua_State *, LuaCppRuntime &, V &);

    template <typename V>
    static typename std::enable_if<is_instantiation<std::vector, V>::value>::type
      push(lua_State *, LuaCppRuntime &, const V &);

    template <typename V>
    static typename std::enable_if<is_instantiation<std::vector, V>::value>::type
      push(lua_State *, LuaCppRuntime &, std::unique_ptr<V> &);

    template <typename V>
    static typename std::enable_if<is_instantiation<std::vector, V>::value>::type
      push(lua_State *, LuaCppRuntime &, std::shared_ptr<V> &);
   private:
    template <typename V>
    static void set_vector_meta(lua_State *, LuaCppRuntime &);

    template <typename V>
    static int vector_index(lua_State *);

    template <typename V>
    static int vector_length(lua_State *);

    template <typename V>
    static int vector_gc(lua_State *);

    template <typename V>
    static int vector_pairs(lua_State *);

    template <typename V>
    static int vector_iter(lua_State *);
  };
}

#include "luacppb/Container/Impl/Vector.h"

#endif