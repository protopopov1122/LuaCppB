#ifndef LUACPPB_INVOKE_METHOD_H_
#define LUACPPB_INVOKE_METHOD_H_

#include "luacppb/Base.h"

namespace LuaCppB::Internal {

  template <typename C, typename R, typename ... A>
  class NativeMethodWrapper {
    using M = R (C::*)(A...);
    using Mc = R (C::*)(A...) const;
   public:
    NativeMethodWrapper(M);
    NativeMethodWrapper(Mc);

    M get() const;
    operator M() const;
   private:
    M method;
  };

  template <typename M>
  struct LuaCppObjectMethodCallDescriptor {
    M method;
  };
}

#include "luacppb/Invoke/Impl/Method.h"

#endif