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

#ifndef LUACPPB_OBJECT_IMPL_BIND_H_
#define LUACPPB_OBJECT_IMPL_BIND_H_

#include "luacppb/Object/Bind.h"


namespace LuaCppB {

  namespace Internal {

    template <typename C>
    void ObjectBinder_Impl<C>::bind(C &obj) {}

    template <typename C, typename N, typename M, typename ... T>
    void ObjectBinder_Impl<C, N, M, T...>::bind(C &obj, N &&name, M &&method, T &&... args) {
      obj.bind(name, method);
      ObjectBinder_Impl<C, T...>::bind(obj, std::forward<T>(args)...);
    }

    template <typename C>
    void ClassBinder_Impl<C>::bind(C &cl) {}

    template <typename C, typename N, typename M, typename ... T>
    void ClassBinder_Impl<C, N, M, T...>::bind(C &cl, N &&name, M &&method, T &&... args) {
      cl.bind(name, method);
      ClassBinder_Impl<C, T...>::bind(cl, std::forward<T>(args)...);
    }
  }

  template <typename C, typename ... T>
  LuaCppObject<C> ObjectBinder::bind(C &obj, LuaCppRuntime &runtime, T &&... methods) {
    LuaCppObject<C> object(obj, runtime);
    Internal::ObjectBinder_Impl<LuaCppObject<C>, T...>::bind(object, std::forward<T>(methods)...);
    return object;
  }

  template <typename C, typename ... T>
  LuaCppObject<C> ObjectBinder::bind(C *obj, LuaCppRuntime &runtime, T &&... methods) {
    LuaCppObject<C> object(obj, runtime);
    Internal::ObjectBinder_Impl<LuaCppObject<C>, T...>::bind(object, std::forward<T>(methods)...);
    return object;
  }

  template <typename C, typename P>
  template <typename ... T>
  LuaCppClass<C, P> ClassBinder<C, P>::bind(LuaState &state, T &&... methods) {
    LuaCppClass<C, P> cl(state);
    Internal::ClassBinder_Impl<LuaCppClass<C, P>, T...>::bind(cl, std::forward<T>(methods)...);
    state.getClassRegistry().bind(cl);
    return cl;
  }

  template <typename C, typename P>
  template <typename ... T>
  LuaCppClass<C, P> ClassBinder<C, P>::bind(const std::string &name, LuaState &state, T &&... methods) {
    LuaCppClass<C, P> cl(name, state);
    Internal::ClassBinder_Impl<LuaCppClass<C, P>, T...>::bind(cl, std::forward<T>(methods)...);
    state.getClassRegistry().bind(cl);
    return cl;
  }
}

#endif
