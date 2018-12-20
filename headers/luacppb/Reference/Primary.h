#ifndef LUACPPB_REFERENCE_PRIMARY_H_
#define LUACPPB_REFERENCE_PRIMARY_H_

#include "luacppb/Reference/Base.h"
#include "luacppb/Object/Registry.h"
#include "luacppb/Core/Error.h"

namespace LuaCppB {

	class LuaGlobalVariable : public LuaReference {
	 public:
		LuaGlobalVariable(LuaState &state, const std::string &name)
			: LuaReference(state.getClassRegistry()), state(state.getState()), name(name) {
			if (this->state == nullptr) {
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
		LuaStackReference(LuaState &state, int index) : LuaReference(state.getClassRegistry()), state(state.getState()), index(index) {
			if (this->state == nullptr) {
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
		LuaRegistryReference(lua_State *, LuaCppObjectBoxerRegistry &, int = -1);

		bool putOnTop(std::function<void (lua_State *)>) override;
		bool setValue(std::function<void (lua_State *)>) override;
	 private:
	 	LuaSharedRegistryHandle handle;
	};
}

#endif