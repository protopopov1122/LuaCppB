#include "luacppb/Core/RuntimeInfo.h"
#include "luacppb/Object/Registry.h"

namespace LuaCppB::Internal {

  LuaRuntimeInfo::LuaRuntimeInfo(std::shared_ptr<LuaCppClassRegistry> classReg)
    : classRegistry(classReg) {}
  
  LuaCppClassRegistry &LuaRuntimeInfo::getClassRegistry() {
    return *this->classRegistry;
  }
}