#include "luacppb/Invoke/Native.h"

namespace LuaCppB::Internal {

	LuaCppBNativeException::LuaCppBNativeException(std::exception_ptr eptr) : eptr(eptr) {}

	std::exception_ptr LuaCppBNativeException::get() {
		return this->eptr;
	}

	void LuaCppBNativeException::process(lua_State *state, std::exception_ptr eptr) {
		LuaCppBNativeException *ex = reinterpret_cast<LuaCppBNativeException *>(lua_newuserdata(state, sizeof(LuaCppBNativeException)));
		new(ex) LuaCppBNativeException(eptr);
		if (luaL_newmetatable(state, LuaCppBNativeException::LUACPPB_EXCEPTION_POINTER)) {
			lua_pushcfunction(state, &LuaCppBNativeException::gc);
			lua_setfield(state, -2, "__gc");
		}
		lua_setmetatable(state, -2);
		lua_error(state);
	}

	void LuaCppBNativeException::check(lua_State *state) {
		if (lua_isuserdata(state, -1)) {
			lua_getmetatable(state, -1);
			if (lua_istable(state, -1)) {
				lua_pushstring(state, "__name");
				lua_rawget(state, -2);
				std::string tableName(lua_tostring(state, -1));
				lua_pop(state, 2);
				if (tableName.compare(LuaCppBNativeException::LUACPPB_EXCEPTION_POINTER) == 0) {
					LuaCppBNativeException *ex = reinterpret_cast<LuaCppBNativeException *>(luaL_checkudata(state, -1, LuaCppBNativeException::LUACPPB_EXCEPTION_POINTER));
					if (ex) {
						std::exception_ptr eptr = ex->get();
						std::rethrow_exception(eptr);
					}
				}
			} else {
				lua_pop(state, 1);
			}
		}
	}

	int LuaCppBNativeException::gc(lua_State *state) {
		LuaCppBNativeException *ex = reinterpret_cast<LuaCppBNativeException *>(luaL_checkudata(state, 1, LuaCppBNativeException::LUACPPB_EXCEPTION_POINTER));
		ex->~LuaCppBNativeException();
		::operator delete(ex, ex);
		return 0;
	}

#ifdef LUACPPB_EXCEPTION_PROPAGATION
	int luacpp_handle_exception(lua_State *state, std::exception_ptr eptr) {
		LuaCppBNativeException::process(state, eptr);
    return 0;
	}
#else
	int luacpp_handle_exception(lua_State *state, std::exception_ptr eptr) {
		std::rethrow_exception(eptr);
    return 0;
	}
#endif
}