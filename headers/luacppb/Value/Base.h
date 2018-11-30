#ifndef LUACPPB_VALUE_BASE_H_
#define LUACPPB_VALUE_BASE_H_

#include "luacppb/Base.h"

namespace LuaCppB {

	enum class LuaType {
		None = LUA_TNONE,
		Nil = LUA_TNIL,
		Number = LUA_TNUMBER,
		Boolean = LUA_TBOOLEAN,
		String = LUA_TSTRING,
		Table = LUA_TTABLE,
		Function = LUA_TFUNCTION,
		UserData = LUA_TUSERDATA,
		Thread = LUA_TTHREAD,
		LightUserData = LUA_TLIGHTUSERDATA
	};

  class LuaData {
	 public:
		virtual ~LuaData() = default;
		virtual void push(lua_State *state) const = 0;
	};

	class LuaValueBase : public LuaData {};
}

#endif