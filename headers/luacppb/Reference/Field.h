#ifndef LUACPPB_REFERENCE_FIELD_H_
#define LUACPPB_REFERENCE_FIELD_H_

#include "luacppb/Reference/Handle.h"

namespace LuaCppB::Internal {

	class LuaTableField : public LuaReference {
	 public:
		LuaTableField(LuaReferenceHandle, LuaCppRuntime &, const std::string &);
		
		bool putOnTop(std::function<void (lua_State *)>) override;
		bool setValue(std::function<void (lua_State *)>) override;
	 private:
		LuaReferenceHandle ref;
		std::string name;
	};

	class LuaArrayField : public LuaReference {
	 public:
		LuaArrayField(LuaReferenceHandle, LuaCppRuntime &, lua_Integer);

		bool putOnTop(std::function<void (lua_State *)>) override;
		bool setValue(std::function<void (lua_State *)>) override;
	 private:
		LuaReferenceHandle ref;
		lua_Integer index;
	};
}

#endif