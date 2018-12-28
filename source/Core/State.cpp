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

	LuaReferenceHandle LuaState::operator[](const std::string &name) {
		return LuaReferenceHandle(this->state, std::make_unique<Internal::LuaGlobalVariable>(*this, name));
	}


	LuaReferenceHandle LuaState::operator[](lua_Integer index) {
		return LuaReferenceHandle(this->state, std::make_unique<Internal::LuaStackReference>(*this, index));
	}

	Internal::LuaFunctionCallResult LuaState::operator()(const std::string &code, bool preprendReturn) {
		std::string exec(code);
		if (preprendReturn) {
			exec = "return (" + exec + ")";
		}
		LuaStatusCode status = static_cast<LuaStatusCode>(luaL_loadstring(this->state, exec.c_str()));
		if (status == LuaStatusCode::Ok) {
			Internal::LuaStack stack(this->state);
			std::optional<LuaValue> value = stack.get();
			stack.pop();
			LuaFunction func = value.value_or(LuaValue()).get<LuaFunction>();
			return func.ref(*this)();
		} else {
			return Internal::LuaFunctionCallResult(LuaError(status));
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
		Internal::LuaStackCleaner cleaner(this->state);
		bool err = static_cast<bool>(luaL_dofile(this->state, path.c_str()));
		Internal::LuaCppBNativeException::check(this->state);
		return this->pollResult(err, cleaner.getDelta());
	}

	Internal::LuaFunctionCallResult LuaEnvironment::execute(const std::string &code) {
		Internal::LuaStackCleaner cleaner(this->state);
		bool err = static_cast<bool>(luaL_dostring(this->state, code.c_str()));
		Internal::LuaCppBNativeException::check(this->state);
		return this->pollResult(err, cleaner.getDelta());
	}

	Internal::LuaFunctionCallResult LuaEnvironment::pollResult(bool err, int delta) {
		std::vector<LuaValue> result;
		while (delta-- > 0) {
			result.push_back(LuaValue::peek(this->state, -1).value_or(LuaValue()));
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
