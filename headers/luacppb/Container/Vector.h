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

    template <typename V, typename D>
    static typename std::enable_if<is_instantiation<std::vector, V>::value>::type
      push(lua_State *, LuaCppRuntime &, std::unique_ptr<V, D> &);

    template <typename V>
    static typename std::enable_if<is_instantiation<std::vector, V>::value>::type
      push(lua_State *, LuaCppRuntime &, std::shared_ptr<V> &);
   private:
    template <typename V, typename D = std::default_delete<V>>
    static void set_vector_meta(lua_State *, LuaCppRuntime &);

    template <typename V>
    static int vector_index(lua_State *);

    template <typename V>
    static int vector_length(lua_State *);

    template <typename V, typename D>
    static int vector_gc(lua_State *);

    template <typename V>
    static int vector_pairs(lua_State *);

    template <typename V>
    static int vector_iter(lua_State *);
  };
}

#include "luacppb/Container/Impl/Vector.h"

#endif
