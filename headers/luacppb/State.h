#ifndef LUACPPB_STATE_H_
#define LUACPPB_STATE_H_

#include "luacppb/Base.h"
#include "luacppb/Status.h"
#include <string>
#include <memory>

namespace LuaCppB {

	class LuaReferenceHandle;
	class LuaCppClassRegistry;

	class LuaState {
	 public:
		LuaState(lua_State *);
		virtual ~LuaState() = default;
		lua_State *getState() const;
		LuaCppClassRegistry &getClassRegistry();
		
		LuaReferenceHandle operator[](const std::string &);
		LuaReferenceHandle operator[](lua_Integer);
	 protected:
		lua_State *state;
		std::unique_ptr<LuaCppClassRegistry> registry;
	};

	class LuaUniqueState : public LuaState {
	 public:
		LuaUniqueState(lua_State * = nullptr);
		virtual ~LuaUniqueState();
	};

	class LuaEnvironment : public LuaUniqueState {
	 public:
		LuaEnvironment(bool = true);
		LuaStatusCode load(const std::string &);
		LuaStatusCode execute(const std::string &);
	};
}

#endif
