#ifndef LUACPPB_REFERENCE_PRIMARY_H_
#define LUACPPB_REFERENCE_PRIMARY_H_

#include "luacppb/Reference/Base.h"
#include "luacppb/Reference/Registry.h"
#include "luacppb/Error.h"

namespace LuaCppB {

	class LuaGlobalVariable : public LuaReference {
	 public:
		LuaGlobalVariable(lua_State *state, LuaCppClassRegistry &registry, const std::string &name) : LuaReference(registry), state(state), name(name) {
			if (state == nullptr) {
				throw LuaCppBError("Lua state can't be null", LuaCppBErrorCode::InvalidState);
			}
		}
	
		bool putOnTop(std::function<void (lua_State *)>) override;
		bool setValue(std::function<void (lua_State *)>) override;
	 private:
		lua_State *state;
		std::string name;
	};

	class LuaStackReference : public LuaReference {
	 public:
		LuaStackReference(lua_State *state, LuaCppClassRegistry &registry, int index) : LuaReference(registry), state(state), index(index) {
			if (state == nullptr) {
				throw LuaCppBError("Lua state can't be null", LuaCppBErrorCode::InvalidState);
			}
		}

		bool putOnTop(std::function<void (lua_State *)>) override;
		bool setValue(std::function<void (lua_State *)> gen) override;
	 private:
		lua_State *state;
		int index;
	};

	class LuaRegistryReference : public LuaReference {
	 public:
		LuaRegistryReference(lua_State *, LuaCppClassRegistry &, int = -1);

		bool putOnTop(std::function<void (lua_State *)>) override;
		bool setValue(std::function<void (lua_State *)>) override;
	 private:
	 	LuaSharedRegistryHandle handle;
	};
}

#endif