#ifndef LUACPPB_CORE_RUNTIME_H_
#define LUACPPB_CORE_RUNTIME_H_

#include "luacppb/Base.h"
#include "luacppb/Core/RuntimeInfo.h"
#include "luacppb/Object/Boxer.h"
#include <memory>
#include <functional>
#include <exception>

namespace LuaCppB {

  class LuaCppRuntime {
   public:
    virtual ~LuaCppRuntime() = default;
    virtual Internal::LuaCppObjectBoxerRegistry &getObjectBoxerRegistry() = 0;
    virtual std::shared_ptr<Internal::LuaRuntimeInfo> &getRuntimeInfo() = 0;
    virtual void setExceptionHandler(std::function<void(std::exception &)>) = 0;
    virtual std::function<void(std::exception &)> getExceptionHandler() = 0;
  };
}

#endif