#include "luacppb/State.h"
#include "luacppb/Reference/Reference.h"
#include <cassert>

namespace LuaCppB {

	LuaState::LuaState(lua_State *state)
		: state(state) {
		assert(this->state != nullptr);
	}

	lua_State *LuaState::getState() const {
		return this->state;
	}

	LuaReferenceHandle LuaState::operator[](const std::string &name) {
		return LuaReferenceHandle(this->state, std::make_unique<LuaGlobalVariable>(this->state, name));
	}


	LuaReferenceHandle LuaState::operator[](lua_Integer index) {
		return LuaReferenceHandle(this->state, std::make_unique<LuaStackReference>(this->state, index));
	}

	LuaStateUnique::LuaStateUnique(lua_State *state)
		: LuaState(state != nullptr ? state : luaL_newstate()) {}

	LuaStateUnique::~LuaStateUnique() {
		lua_close(this->state);
	}

	LuaEnvironment::LuaEnvironment(bool openLibs)
		: LuaStateUnique() {
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
