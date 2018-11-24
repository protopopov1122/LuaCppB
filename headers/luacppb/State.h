#ifndef LUACPPB_STATE_H_
#define LUACPPB_STATE_H_

#include "luacppb/Base.h"
#include <string>

namespace LuaCppB {

	class LuaReferenceHandle;

	class LuaState {
	 public:
		LuaState(lua_State *);
		virtual ~LuaState() = default;
		lua_State *getState() const;
		
		LuaReferenceHandle operator[](const std::string &);
		LuaReferenceHandle operator[](lua_Integer);
	 protected:
		lua_State *state;
	};

	class LuaEnvironment : public LuaState {
	 public:
		LuaEnvironment(bool = true);
		virtual ~LuaEnvironment();
		void load(const std::string &);
		void execute(const std::string &);
	};
}

#endif
