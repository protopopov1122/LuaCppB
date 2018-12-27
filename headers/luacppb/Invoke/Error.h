#ifndef LUACPPB_INVOKE_ERROR_H_
#define LUACPPB_INVOKE_ERROR_H_

#include "luacppb/Base.h"
#include "luacppb/Core/Status.h"
#include "luacppb/Value/Value.h"

namespace LuaCppB {

  class LuaError {
   public:
    LuaError(LuaStatusCode = LuaStatusCode::Ok);
    LuaError(LuaStatusCode, LuaValue);
    LuaError(const LuaError &);

    LuaStatusCode getStatus() const;
    LuaValue getError() const;
    bool hasError() const;

    bool operator==(LuaStatusCode) const;
    bool operator!=(LuaStatusCode) const;
   private:
    LuaStatusCode status;
    LuaValue error;
  };
}

#endif