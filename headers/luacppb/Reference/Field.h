#ifndef LUACPPB_REFERENCE_FIELD_H_
#define LUACPPB_REFERENCE_FIELD_H_

#include "luacppb/Reference/Handle.h"

namespace LuaCppB {

	class LuaTableField : public LuaReference {
	 public:
		LuaTableField(LuaReferenceHandle ref, LuaCppRuntime &runtime, const std::string &name) : LuaReference(runtime), ref(ref), name(name) {}
		
		bool putOnTop(std::function<void (lua_State *)>) override;
		bool setValue(std::function<void (lua_State *)>) override;
	 private:
		LuaReferenceHandle ref;
		std::string name;
	};

	class LuaArrayField : public LuaReference {
	 public:
		LuaArrayField(LuaReferenceHandle ref, LuaCppRuntime &runtime, lua_Integer index) : LuaReference(runtime), ref(ref), index(index) {}

		bool putOnTop(std::function<void (lua_State *)>) override;
		bool setValue(std::function<void (lua_State *)>) override;
	 private:
		LuaReferenceHandle ref;
		lua_Integer index;
	};
}

#endif