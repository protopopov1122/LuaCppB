#ifndef LUACPPB_CORE_RUNTIME_H_
#define LUACPPB_CORE_RUNTIME_H_

#include "luacppb/Base.h"
#include "luacppb/Object/Boxer.h"

namespace LuaCppB {

  class LuaCppRuntime {
   public:
    virtual ~LuaCppRuntime() = default;
    virtual LuaCppObjectBoxerRegistry &getObjectBoxerRegistry() = 0;
  };
}

#endif