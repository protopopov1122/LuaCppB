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
        LuaError error(status, value.value_or(LuaValue()));
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