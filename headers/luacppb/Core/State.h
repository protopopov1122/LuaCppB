#ifndef LUACPPB_CORE_STATE_H_
#define LUACPPB_CORE_STATE_H_

#include "luacppb/Base.h"
#include "luacppb/Core/Status.h"
#include "luacppb/Core/Runtime.h"
#include "luacppb/Invoke/Error.h"
#include <string>
#include <memory>

namespace LuaCppB {

	class LuaReferenceHandle;
	namespace Internal {
		class LuaCppClassRegistry;
	}

	class LuaState : public LuaCppRuntime {
	 public:
		LuaState(lua_State *, std::shared_ptr<Internal::LuaRuntimeInfo> = nullptr);
		virtual ~LuaState() = default;
		lua_State *getState() const;
		Internal::LuaCppClassRegistry &getClassRegistry();
		Internal::LuaCppObjectBoxerRegistry &getObjectBoxerRegistry() override;
		std::shared_ptr<Internal::LuaRuntimeInfo> &getRuntimeInfo() override;
		
		LuaReferenceHandle operator[](const std::string &);
		LuaReferenceHandle operator[](lua_Integer);
		LuaReferenceHandle operator()(const std::string &);
	 protected:
		lua_State *state;
		std::shared_ptr<Internal::LuaRuntimeInfo> runtime;
	};

	class LuaUniqueState : public LuaState {
	 public:
		LuaUniqueState(lua_State * = nullptr);
		virtual ~LuaUniqueState();
	};

	class LuaEnvironment : public LuaUniqueState {
	 public:
		LuaEnvironment(bool = true);
		LuaError load(const std::string &);
		LuaError execute(const std::string &);
	};
}

#endif
