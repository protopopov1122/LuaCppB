#include "luacppb/Invoke/Lua.h"

namespace LuaCppB {
  
  LuaError::LuaError(LuaStatusCode status)
    : status(status), error(LuaValue()) {}

  LuaError::LuaError(LuaStatusCode status, LuaValue error)
    : status(status), error(error) {}
  
  LuaError::LuaError(const LuaError &err)
    : status(err.status), error(err.error) {}

  LuaStatusCode LuaError::getStatus() const {
    return this->status;
  }
  
  LuaValue LuaError::getError() const {
    return this->error;
  }

  bool LuaError::hasError() const {
    return !(this->status == LuaStatusCode::Ok || this->status == LuaStatusCode::Yield);
  }

  bool LuaError::operator==(LuaStatusCode status) const {
    return this->status == status;
  }

  bool LuaError::operator!=(LuaStatusCode status) const {
    return this->status != status;
  }
}