#include "luacppb/LuaState.h"
#include "luacppb/LuaReference.h"

namespace LuaCppB {

	LuaState::LuaState(lua_State *state)
		: state(state) {}

	lua_State *LuaState::getState() const {
		return this->state;
	}

	LuaReferenceHandle LuaState::operator[](const std::string &name) {
		return LuaReferenceHandle(std::make_unique<LuaGlobalVariable>(*this, name));
	}


	LuaReferenceHandle LuaState::operator[](lua_Integer index) {
		return LuaReferenceHandle(std::make_unique<LuaIndexReference>(*this, index));
	}

	LuaEnvironment::LuaEnvironment(bool openLibs)
		: LuaState(luaL_newstate()) {
	
		if (openLibs) {
			luaL_openlibs(this->state);
		}
	}

	LuaEnvironment::~LuaEnvironment() {
		lua_close(this->state);
	}

	void LuaEnvironment::load(const std::string &path) {
		luaL_dofile(this->state, path.c_str());
	}

	void LuaEnvironment::execute(const std::string &code) {
		luaL_dostring(this->state, code.c_str());
	}
}
