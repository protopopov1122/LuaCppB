#ifndef LUACPPB_INVOKE_IMPL_METHOD_H_
#define LUACPPB_INVOKE_IMPL_METHOD_H_

#include "luacppb/Invoke/Method.h"

namespace LuaCppB::Internal {

  template <typename C, typename R, typename ... A>
  NativeMethodWrapper<C, R, A...>::NativeMethodWrapper(M meth) : method(meth) {}

  template <typename C, typename R, typename ... A>
  NativeMethodWrapper<C, R, A...>::NativeMethodWrapper(Mc meth) : method(reinterpret_cast<M>(meth)) {}

  template <typename C, typename R, typename ... A>
  typename NativeMethodWrapper<C, R, A...>::M NativeMethodWrapper<C, R, A...>::get() const {
    return this->method;
  }

  template <typename C, typename R, typename ... A>
  NativeMethodWrapper<C, R, A...>::operator M() const {
    return this->method;
  }
}

#endif