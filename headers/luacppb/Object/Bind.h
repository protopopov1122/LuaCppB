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

#ifndef LUACPPB_OBJECT_BIND_H_
#define LUACPPB_OBJECT_BIND_H_

#include "luacppb/Object/Class.h"
#include "luacppb/Core/State.h"

namespace LuaCppB {

  namespace Internal {
    template <typename C, typename ... T>
    struct ObjectBinder_Impl {};

    template <typename C>
    struct ObjectBinder_Impl<C> {
      static void bind(C &obj);
    };

    template <typename C, typename N, typename M, typename ... T>
    struct ObjectBinder_Impl<C, N, M, T...> {
      static void bind(C &, N &&, M &&, T &&...);
    };

    template <typename C, typename ... T>
    struct ClassBinder_Impl {};

    template <typename C>
    struct ClassBinder_Impl<C> {
      static void bind(C &cl);
    };

    template <typename C, typename N, typename M, typename ... T>
    struct ClassBinder_Impl<C, N, M, T...> {
      static void bind(C &, N &&, M &&, T &&...);
    };
  }

  class ObjectBinder {
   public:
    template <typename C, typename ... T>
    static LuaCppObject<C> bind(C &, LuaCppRuntime &, T &&...);

    template <typename C, typename ... T>
    static LuaCppObject<C> bind(C *, LuaCppRuntime &, T &&...);
  };

  template <typename C, typename P = void>
  class ClassBinder {
   public:
    template <typename ... T>
    static LuaCppClass<C, P> bind(LuaState &, T &&...);

    template <typename ... T>
    static LuaCppClass<C, P> bind(const std::string &, LuaState &, T &&...);
  };
}

#endif
