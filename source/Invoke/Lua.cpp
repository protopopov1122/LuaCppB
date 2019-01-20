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

#include "luacppb/Invoke/Lua.h"

namespace LuaCppB {
  
  LuaError::LuaError(LuaStatusCode status)
    : status(status), error(LuaValue::Nil) {}

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

  namespace Internal {

    LuaFunctionContinuationHandle::LuaFunctionContinuationHandle(std::unique_ptr<LuaFunctionContinuation> cont, LuaCppRuntime &runtime, int top)
      : cont(std::move(cont)), runtime(runtime.getRuntimeInfo()), top(top) {}
    
    void LuaFunctionContinuationHandle::getResult(lua_State *state, std::vector<LuaValue> &result) {
      Internal::LuaStack stack(state);
      int results = stack.getTop() - top;
      while (results-- > 0) {
        result.push_back(LuaValue::peek(state).value());
        stack.pop();
      }
      std::reverse(result.begin(), result.end());
    }

    std::shared_ptr<LuaRuntimeInfo> &LuaFunctionContinuationHandle::getRuntime() {
      return this->runtime;
    }

    LuaFunctionContinuation &LuaFunctionContinuationHandle::getContinuation() {
      return *this->cont;
    }

    int LuaFunctionContinuationHandle::fnContinuation(lua_State *state, int statusCode, lua_KContext ctx) {
      std::unique_ptr<LuaFunctionContinuationHandle> handle(reinterpret_cast<LuaFunctionContinuationHandle *>(ctx));
      LuaState luaState(state, handle->getRuntime());
      LuaStatusCode status = static_cast<LuaStatusCode>(statusCode);
      std::vector<LuaValue> result;
      if (status == LuaStatusCode::Ok || status == LuaStatusCode::Yield) {
        handle->getResult(state, result);
        return handle->getContinuation().call(state, luaState, LuaError(status), result);
      } else {
        LuaCppBNativeException::check(state);
        LuaStack stack(state);
        std::optional<LuaValue> value = stack.get();
        stack.pop();
        LuaError error(status, value.value_or(LuaValue::Nil));
        return handle->getContinuation().call(state, luaState, error, result);
      }
    }

    LuaFunctionCallResult::LuaFunctionCallResult(std::vector<LuaValue> &result, LuaError err)
      : result(result), errorVal(err) {}
    
    LuaFunctionCallResult::LuaFunctionCallResult(LuaError err)
      : result(), errorVal(err) {}
    
    LuaFunctionCallResult &LuaFunctionCallResult::status(LuaStatusCode &status) {
      status = this->errorVal.getStatus();
      return *this;
    }

    LuaFunctionCallResult &LuaFunctionCallResult::error(LuaError &error) {
      error = this->errorVal;
      return *this;
    }

    bool LuaFunctionCallResult::hasError() const {
      return this->errorVal.hasError();
    }

    bool LuaFunctionCallResult::operator==(LuaStatusCode code) const {
      return this->errorVal.getStatus() == code;
    }

    bool LuaFunctionCallResult::operator!=(LuaStatusCode code) const {
      return this->errorVal.getStatus() != code;
    }

    void LuaFunctionCallResult::push(lua_State *state) {
      for (auto &res : this->result) {
        res.push(state);
      }
    }
  }
}
