#ifndef LUACPPB_CORE_STATE_H_
#define LUACPPB_CORE_STATE_H_

#include "luacppb/Base.h"
#include "luacppb/Core/Status.h"
#include "luacppb/Core/Runtime.h"
#include <string>
#include <memory>

namespace LuaCppB {

	class LuaReferenceHandle;
	class LuaCppClassRegistry;

	class LuaState : public LuaCppRuntime {
	 public:
		LuaState(lua_State *, std::shared_ptr<LuaCppClassRegistry> = nullptr);
		virtual ~LuaState() = default;
		lua_State *getState() const;
		LuaCppClassRegistry &getClassRegistry();
		LuaCppObjectBoxerRegistry &getObjectBoxerRegistry() override;
		std::shared_ptr<LuaCppObjectBoxerRegistry> getOwnedObjectBoxerRegistry() override;
		
		LuaReferenceHandle operator[](const std::string &);
		LuaReferenceHandle operator[](lua_Integer);
	 protected:
		lua_State *state;
		std::shared_ptr<LuaCppClassRegistry> registry;
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
