/*
  SPDX short identifier: MIT

  Copyright 2018-2019 Jevgēnijs Protopopovs

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

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

#endif
