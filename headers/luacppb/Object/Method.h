#ifndef LUACPPB_OBJECT_METHOD_H_
#define LUACPPB_OBJECT_METHOD_H_

#include "luacppb/Base.h"
#include "luacppb/Value/Native.h"
#include "luacppb/Object/Wrapper.h"
#include "luacppb/Invoke/ArgRet.h"
#include "luacppb/Invoke/Method.h"
#include "luacppb/Core/Stack.h"
#include "luacppb/Core/Runtime.h"
#include <optional>

namespace LuaCppB::Internal {

	template <typename C, typename M, typename R, typename ... A>
	class LuaCppObjectMethodCall : public LuaData {
	 public:
		LuaCppObjectMethodCall(M, const std::string &, LuaCppRuntime &);

		void push(lua_State *) const override;
	 private:
	 	static int call(C *, M, LuaCppRuntime &, lua_State *);
		static int class_method_closure(lua_State *);
		static LuaCppObjectWrapper<C> *checkUserData(const std::string &, lua_State *, int = -1);

		M method;
		std::string className;
		LuaCppRuntime &runtime;
	};
}

#include "luacppb/Object/Impl/Method.h"

#endif