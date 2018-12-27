#include "luacppb/Core/State.h"
#include "luacppb/Reference/Primary.h"
#include "luacppb/Reference/Field.h"
#include "luacppb/Object/Registry.h"
#include "luacppb/Core/Stack.h"
#include <cassert>
#include <memory>
#include <iostream>

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

	LuaReferenceHandle LuaState::operator()(const std::string &code) {
		int status = luaL_loadstring(this->state, code.c_str());
		if (status == LUA_OK) {
			Internal::LuaStack stack(this->state);
			std::optional<LuaValue> value = stack.get();
			stack.pop();
			LuaFunction func = value.value_or(LuaValue()).get<LuaFunction>();
			return func.ref(*this);
		} else {
			return LuaReferenceHandle();
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

	LuaError LuaEnvironment::load(const std::string &path) {
		bool err = static_cast<bool>(luaL_dofile(this->state, path.c_str()));
		if (err) {
			return LuaError(LuaStatusCode::RuntimeError);
		} else {
			return LuaError();
		}
	}

	LuaError LuaEnvironment::execute(const std::string &code) {
		bool err = static_cast<bool>(luaL_dostring(this->state, code.c_str()));
		if (err) {
			return LuaError(LuaStatusCode::RuntimeError);
		} else {
			return LuaError();
		}
	}
}
