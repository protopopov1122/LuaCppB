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

#ifndef LUACPPB_CONTAINER_SET_H_
#define LUACPPB_CONTAINER_SET_H_

#include "luacppb/Base.h"
#include "luacppb/Meta.h"
#include "luacppb/Core/Stack.h"
#include <set>

namespace LuaCppB::Internal {

  template <class P>
  class LuaCppSet {
   public:
    template <typename S>
    static typename std::enable_if<is_instantiation<std::set, S>::value>::type
      push(lua_State *, LuaCppRuntime &, S &);
    
    template <typename S>
    static typename std::enable_if<is_instantiation<std::set, S>::value>::type
      push(lua_State *, LuaCppRuntime &, const S &);

    template <typename S>
    static typename std::enable_if<is_instantiation<std::set, S>::value>::type
      push(lua_State *, LuaCppRuntime &, std::unique_ptr<S> &);

    template <typename S>
    static typename std::enable_if<is_instantiation<std::set, S>::value>::type
      push(lua_State *, LuaCppRuntime &, std::shared_ptr<S> &);
   private:
    template <typename S>
    static void set_set_meta(lua_State *, LuaCppRuntime &);

    template <typename S>
    static int set_get(lua_State *);
	
    template <typename S>
    static int set_put(lua_State *);

    template <typename S>
    static int set_size(lua_State *);

    template <typename S>
    static int set_pairs(lua_State *);

    template <typename S>
    static int set_iter(lua_State *);

    template <typename S>
    static int set_gc(lua_State *);
	
	template <class T1, typename T2>
	friend int LuaCppSet_put(lua_State *);
  };
}

#include "luacppb/Container/Impl/Set.h"

#endif
