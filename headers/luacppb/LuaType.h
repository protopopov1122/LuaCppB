#ifndef LUACPPB_LUATYPE_H_
#define LUACPPB_LUATYPE_H_

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

	typedef int (*LuaCFunction)(lua_State *);
}

#endif
