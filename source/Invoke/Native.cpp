#include "luacppb/Invoke/Native.h"

namespace LuaCppB::Internal {

	LuaCppBNativeException::LuaCppBNativeException(std::exception_ptr eptr) : eptr(eptr) {}

	std::exception_ptr LuaCppBNativeException::get() {
		return this->eptr;
	}

	void LuaCppBNativeException::process(lua_State *state, std::exception_ptr eptr) {
		Internal::LuaStack stack(state);
		LuaCppBNativeException *ex = stack.push<LuaCppBNativeException>();
		new(ex) LuaCppBNativeException(eptr);
		if (stack.metatable(LuaCppBNativeException::LUACPPB_EXCEPTION_POINTER)) {
			stack.push(&LuaCppBNativeException::gc);
			stack.setField(-2, "__gc");
		}
		stack.setMetatable(-2);
		lua_error(state);
	}

	void LuaCppBNativeException::check(lua_State *state) {
		Internal::LuaStack stack(state);
		if (stack.is<LuaType::UserData>(-1)) {
			stack.getMetatable(-1);
			if (stack.is<LuaType::Table>(-1)) {
				stack.push(std::string("__name"));
				lua_rawget(state, -2);
				std::string tableName(stack.toString(-1));
				stack.pop(2);
				if (tableName.compare(LuaCppBNativeException::LUACPPB_EXCEPTION_POINTER) == 0) {
					LuaCppBNativeException *ex = stack.checkUserData<LuaCppBNativeException>(-1, LuaCppBNativeException::LUACPPB_EXCEPTION_POINTER);
					if (ex) {
						std::exception_ptr eptr = ex->get();
						std::rethrow_exception(eptr);
					}
				}
			} else {
				stack.pop();
			}
		}
	}



	int LuaCppBNativeException::gc(lua_State *state) {
		Internal::LuaStack stack(state);
		LuaCppBNativeException *ex = stack.checkUserData<LuaCppBNativeException>(1, LuaCppBNativeException::LUACPPB_EXCEPTION_POINTER);
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