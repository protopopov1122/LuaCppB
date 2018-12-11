#ifndef LUACPPB_INVOKE_METHOD_H_
#define LUACPPB_INVOKE_METHOD_H_

#include "luacppb/Base.h"

namespace LuaCppB {

  template <typename C, typename R, typename ... A>
  class NativeMethodWrapper {
    using M = R (C::*)(A...);
    using Mc = R (C::*)(A...) const;
   public:
    NativeMethodWrapper(M meth) : method(meth) {}
    NativeMethodWrapper(Mc meth) : method(reinterpret_cast<M>(meth)) {}

    M get() const {
      return this->method;
    }

    operator M() const {
      return this->method;
    }
   private:
    M method;
  };

  template <typename M>
  struct LuaCppObjectMethodCallDescriptor {
    M method;
  };
}

#endif