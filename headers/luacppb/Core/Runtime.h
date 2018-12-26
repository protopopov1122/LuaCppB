#ifndef LUACPPB_CORE_RUNTIME_H_
#define LUACPPB_CORE_RUNTIME_H_

#include "luacppb/Base.h"
#include "luacppb/Core/RuntimeInfo.h"
#include "luacppb/Object/Boxer.h"
#include <memory>

namespace LuaCppB {

  class LuaCppRuntime {
   public:
    virtual ~LuaCppRuntime() = default;
    virtual Internal::LuaCppObjectBoxerRegistry &getObjectBoxerRegistry() = 0;
    virtual std::shared_ptr<Internal::LuaRuntimeInfo> &getRuntimeInfo() = 0;
  };
}

#endif