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

#include "luacppb/Core/State.h"
#include "luacppb/Reference/Primary.h"
#include "luacppb/Reference/Field.h"
#include "luacppb/Object/Registry.h"
#include "luacppb/Core/Stack.h"
#include "luacppb/Core/StackGuard.h"
#include "luacppb/Invoke/Lua.h"
#include "luacppb/Invoke/Native.h"
#include <cassert>
#include <memory>

namespace LuaCppB {

	LuaState::LuaState(lua_State *state, std::shared_ptr<Internal::LuaRuntimeInfo> runtime)
		: state(state), runtime(runtime == nullptr ? std::make_shared<Internal::LuaRuntimeInfo>(std::make_shared<Internal::LuaCppClassRegistry>(state)) : runtime) {
		assert(this->state != nullptr);
		this->exception_handler = [](std::exception &ex) {
			throw ex;
		};
	}

	lua_State *LuaState::getState() const {
		return this->state;
	}

	Internal::LuaCppClassRegistry &LuaState::getClassRegistry() {
		return this->runtime->getClassRegistry();
	}

	Internal::LuaCppObjectBoxerRegistry &LuaState::getObjectBoxerRegistry() {
		return this->runtime->getClassRegistry();
	}

	std::shared_ptr<Internal::LuaRuntimeInfo> &LuaState::getRuntimeInfo() {
		return this->runtime;
	}

	void LuaState::setExceptionHandler(std::function<void(std::exception &)> eh) {
		this->exception_handler = eh;
	}
  
	std::function<void(std::exception &)> LuaState::getExceptionHandler() {
		return this->exception_handler;
	}

	LuaReferenceHandle LuaState::operator[](const std::string &name) {
		return LuaReferenceHandle(this->state, std::make_unique<Internal::LuaGlobalVariable>(*this, name));
	}


	LuaReferenceHandle LuaState::operator[](lua_Integer index) {
		return LuaReferenceHandle(this->state, std::make_unique<Internal::LuaStackReference>(*this, index));
	}

	Internal::LuaFunctionCallResult LuaState::operator()(const std::string &code, bool preprendReturn) {
		try {
			std::string exec(code);
			if (preprendReturn) {
				exec = "return (" + exec + ")";
			}
			LuaStatusCode status = static_cast<LuaStatusCode>(luaL_loadstring(this->state, exec.c_str()));
			if (status == LuaStatusCode::Ok) {
				Internal::LuaStack stack(this->state);
				std::optional<LuaValue> value = stack.get();
				stack.pop();
				LuaFunction func = value.value_or(LuaValue::Nil).get<LuaFunction>();
				return func.ref(*this)();
			} else {
				return Internal::LuaFunctionCallResult(LuaError(status));
			}
		} catch (std::exception &ex) {
			this->exception_handler(ex);
			return Internal::LuaFunctionCallResult(LuaError(LuaStatusCode::RuntimeError));
		}
	}

	LuaUniqueState::LuaUniqueState(lua_State *state)
		: LuaState(state != nullptr ? state : luaL_newstate()) {}

	LuaUniqueState::~LuaUniqueState() {
		lua_close(this->state);
	}

	LuaEnvironment::LuaEnvironment(bool openLibs)
		: LuaUniqueState() {
		if (openLibs) {
			luaL_openlibs(this->state);
		}
	}

	Internal::LuaFunctionCallResult LuaEnvironment::load(const std::string &path) {
		try {
			Internal::LuaStackCleaner cleaner(this->state);
			bool err = static_cast<bool>(luaL_dofile(this->state, path.c_str()));
			Internal::LuaCppBNativeException::check(this->state);
			return this->pollResult(err, cleaner.getDelta());
		} catch (std::exception &ex) {
			this->exception_handler(ex);
			return Internal::LuaFunctionCallResult(LuaError());
		}
	}

	Internal::LuaFunctionCallResult LuaEnvironment::execute(const std::string &code) {
		try {
			Internal::LuaStackCleaner cleaner(this->state);
			bool err = static_cast<bool>(luaL_dostring(this->state, code.c_str()));
			Internal::LuaCppBNativeException::check(this->state);
			return this->pollResult(err, cleaner.getDelta());
		} catch (std::exception &ex) {
			this->exception_handler(ex);
			return Internal::LuaFunctionCallResult(LuaError(LuaStatusCode::RuntimeError));
		}
	}

	Internal::LuaFunctionCallResult LuaEnvironment::pollResult(bool err, int delta) {
		std::vector<LuaValue> result;
		while (delta-- > 0) {
			result.push_back(LuaValue::peek(this->state, -1).value_or(LuaValue::Nil));
			lua_pop(this->state, 1);
		}
		if (err) {
			if (result.size() > 0) {
				LuaError err(LuaStatusCode::RuntimeError, result.at(0));
				return Internal::LuaFunctionCallResult(err);
			} else {
				return Internal::LuaFunctionCallResult(LuaError(LuaStatusCode::RuntimeError));	
			}
		} else {
			std::reverse(result.begin(), result.end());
			return Internal::LuaFunctionCallResult(result);
		}
	}
}
