#ifndef LUACPPB_REFERENCE_PRIMARY_H_
#define LUACPPB_REFERENCE_PRIMARY_H_

#include "luacppb/Reference/Base.h"
#include "luacppb/Reference/Registry.h"
#include "luacppb/Error.h"

namespace LuaCppB {

	class LuaGlobalVariable : public LuaReference {
	 public:
		LuaGlobalVariable(lua_State *state, const std::string &name) : state(state), name(name) {
			if (state == nullptr) {
				throw LuaCppBError("Lua state can't be null", LuaCppBErrorCode::InvalidState);
			}
		}
	
		void putOnTop(std::function<void (lua_State *)>) override;
		void setValue(std::function<void (lua_State *)>) override;
	 private:
		lua_State *state;
		std::string name;
	};

	class LuaStackReference : public LuaReference {
	 public:
		LuaStackReference(lua_State *state, int index) : state(state), index(index) {
			if (state == nullptr) {
				throw LuaCppBError("Lua state can't be null", LuaCppBErrorCode::InvalidState);
			}
		}

		void putOnTop(std::function<void (lua_State *)>) override;
		void setValue(std::function<void (lua_State *)> gen) override;
	 private:
		lua_State *state;
		int index;
	};

	class LuaRegistryReference : public LuaReference {
	 public:
		LuaRegistryReference(lua_State *, int = -1);

		void putOnTop(std::function<void (lua_State *)>) override;
		void setValue(std::function<void (lua_State *)>) override;
	 private:
	 	LuaSharedRegistryHandle handle;
	};
}

#endif