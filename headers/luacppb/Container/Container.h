#ifndef LUACPPB_CONTAINER_CONTAINER_H_
#define LUACPPB_CONTAINER_CONTAINER_H_

#include "luacppb/Container/Vector.h"
#include "luacppb/Container/Map.h"
#include "luacppb/Container/Tuple.h"
#include "luacppb/Container/Set.h"
#include "luacppb/Core/Stack.h"
#include "luacppb/Meta.h"
#include <type_traits>
#include <optional>

namespace LuaCppB::Internal {

  class LuaCppContainer {
   public:
    template <typename T, class P>
    static typename std::enable_if<is_instantiation<std::vector, T>::value>::type
      push(lua_State *, LuaCppRuntime &, T &);

    template <typename T, class P>
    static typename std::enable_if<std::is_const<T>::value && is_instantiation<std::vector, typename std::remove_const<T>::type>::value>::type
      push(lua_State *, LuaCppRuntime &, T &);

    template <typename T, class P>
    static typename std::enable_if<is_smart_pointer<T>::value && is_instantiation<std::vector, typename T::element_type>::value>::type
      push(lua_State *, LuaCppRuntime &, T &);
    
    template <typename T, class P>
    static typename std::enable_if<is_instantiation<std::map, T>::value>::type
      push(lua_State *, LuaCppRuntime &, T &);

    template <typename T, class P>
    static typename std::enable_if<std::is_const<T>::value && is_instantiation<std::map, typename std::remove_const<T>::type>::value>::type
      push(lua_State *, LuaCppRuntime &, T &);

    template <typename T, class P>
    static typename std::enable_if<is_smart_pointer<T>::value && is_instantiation<std::map, typename T::element_type>::value>::type
      push(lua_State *, LuaCppRuntime &, T &);

    template <typename T, class P>
    static typename std::enable_if<is_instantiation<std::set, T>::value>::type
      push(lua_State *, LuaCppRuntime &, T &);

    template <typename T, class P>
    static typename std::enable_if<std::is_const<T>::value && is_instantiation<std::set, typename std::remove_const<T>::type>::value>::type
      push(lua_State *, LuaCppRuntime &, T &);

    template <typename T, class P>
    static typename std::enable_if<is_smart_pointer<T>::value && is_instantiation<std::set, typename T::element_type>::value>::type
      push(lua_State *, LuaCppRuntime &, T &);
    
    template <typename T, class P>
    static typename std::enable_if<LuaCppTuple::is_tuple<T>()>::type
      push(lua_State *, LuaCppRuntime &, T &);
  
    template <typename T, class P>
    static typename std::enable_if<is_instantiation<std::optional, T>::value>::type
      push(lua_State *, LuaCppRuntime &, T &);

    template <typename T>
    static constexpr bool is_container() {
      return is_instantiation<std::vector, T>::value ||
             is_instantiation<std::map, T>::value ||
             is_instantiation<std::set, T>::value ||
             is_instantiation<std::optional, T>::value ||
             LuaCppTuple::is_tuple<T>() ||
             (std::is_const<T>::value && LuaCppContainer::is_container<typename std::remove_const<T>::type>());
    }

  };
}

#include "luacppb/Container/Impl/Container.h"

#endif