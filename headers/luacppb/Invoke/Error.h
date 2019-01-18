/*
  SPDX short identifier: MIT

  Copyright 2018-2019 Jevgēnijs Protopopovs

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

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
