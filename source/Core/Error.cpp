#include "luacppb/Core/Error.h"

namespace LuaCppB {

  LuaCppBError::LuaCppBError(const std::string &msg, LuaCppBErrorCode code)
    : message(msg), error_code(code) {}

  LuaCppBError::LuaCppBError(const char *msg, LuaCppBErrorCode code)
    : message(msg), error_code(code) {}
  
  const char *LuaCppBError::what() const noexcept {
    return this->message.c_str();
  }

  LuaCppBErrorCode LuaCppBError::getErrorCode() const noexcept {
    return this->error_code;
  }
}