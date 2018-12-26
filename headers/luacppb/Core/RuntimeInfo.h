#ifndef LUACPPB_CORE_RUNTIMEINFO_H_
#define LUACPPB_CORE_RUNTIMEINFO_H_

#include <memory>

namespace LuaCppB::Internal {

  class LuaCppClassRegistry;

  class LuaRuntimeInfo {
   public:
    LuaRuntimeInfo(std::shared_ptr<LuaCppClassRegistry>);

    LuaCppClassRegistry &getClassRegistry();
   private:
    std::shared_ptr<LuaCppClassRegistry> classRegistry;
  };
}

#endif