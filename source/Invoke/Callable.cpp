#include "luacppb/Invoke/Callable.h"

namespace LuaCppB {

  Internal::LuaLambdaHandle<LuaReferenceHandle> LuaLambda(LuaReferenceHandle &&ref) {
    return Internal::LuaLambdaHandle<LuaReferenceHandle>(std::forward<LuaReferenceHandle>(ref));
  }
}