#include "luacppb/Core/State.h"
#include "luacppb/Reference/Primary.h"
#include "luacppb/Reference/Field.h"
#include "luacppb/Object/Registry.h"
#include <cassert>
#include <memory>

namespace LuaCppB {

	LuaState::LuaState(lua_State *state, std::shared_ptr<Internal::LuaCppClassRegistry> registry)
		: state(state), registry(registry == nullptr ? std::make_shared<Internal::LuaCppClassRegistry>(state) : registry) {
		assert(this->state != nullptr);
	}

	lua_State *LuaState::getState() const {
		return this->state;
	}

	Internal::LuaCppClassRegistry &LuaState::getClassRegistry() {
		return *this->registry;
	}

	Internal::LuaCppObjectBoxerRegistry &LuaState::getObjectBoxerRegistry() {
		return *this->registry;
	}

	std::shared_ptr<Internal::LuaCppObjectBoxerRegistry> LuaState::getOwnedObjectBoxerRegistry() {
		return this->registry;
	}

	LuaReferenceHandle LuaState::operator[](const std::string &name) {
		return LuaReferenceHandle(this->state, std::make_unique<Internal::LuaGlobalVariable>(*this, name));
	}


	LuaReferenceHandle LuaState::operator[](lua_Integer index) {
		return LuaReferenceHandle(this->state, std::make_unique<Internal::LuaStackReference>(*this, index));
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

	LuaStatusCode LuaEnvironment::load(const std::string &path) {
		return static_cast<LuaStatusCode>(luaL_dofile(this->state, path.c_str()));
	}

	LuaStatusCode LuaEnvironment::execute(const std::string &code) {
		return static_cast<LuaStatusCode>(luaL_dostring(this->state, code.c_str()));
	}
}
