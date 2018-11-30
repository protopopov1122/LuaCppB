#ifndef LUACPPB_ERROR_H_
#define LUACPPB_ERROR_H_

#include "luacppb/Base.h"
#include <exception>
#include <string>

namespace LuaCppB {

  enum class LuaCppBErrorCode {
    NotSpecified,
    InvalidState,
    StateMismatch,
    StackOverflow
  };

  class LuaCppBError : std::exception {
   public:
    LuaCppBError(const std::string &, LuaCppBErrorCode = LuaCppBErrorCode::NotSpecified);
    LuaCppBError(const char *, LuaCppBErrorCode = LuaCppBErrorCode::NotSpecified);

    const char *what() const noexcept override;
    LuaCppBErrorCode getErrorCode() const noexcept;
   private:
    std::string message;
    LuaCppBErrorCode error_code;
  };
}

#endif