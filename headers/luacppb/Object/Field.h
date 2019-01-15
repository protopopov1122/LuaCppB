#ifndef LUACPPB_OBJECT_FIELD_H_
#define LUACPPB_OBJECT_FIELD_H_

#include "luacppb/Base.h"
#include "luacppb/Core/Runtime.h"
#include "luacppb/Value/Native.h"
#include <memory>

namespace LuaCppB::Internal {

	class LuaCppObjectFieldPusher {
	 public:
		virtual ~LuaCppObjectFieldPusher() = default;
		virtual void push(lua_State *, void *) const = 0;
	};

	template <typename C, typename B, typename T>
	class LuaCppObjectFieldHandle : public LuaCppObjectFieldPusher {
	 public:
		LuaCppObjectFieldHandle(T C::*, LuaCppRuntime &);

		void push(lua_State *, void *) const override;
	 private:
		T C::*field;
		LuaCppRuntime &runtime;
	};

  class LuaCppObjectFieldController {
   public:
    LuaCppObjectFieldController(const std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> &);

    void get(lua_State *, void *, const std::string &);
    static void push(lua_State *, const std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> &);
    static void pushFunction(lua_State *, const std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> &);
   private:
    static int indexFunction(lua_State *);
    static int gcObject(lua_State *);
    std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> fields;
  };
}

#include "luacppb/Object/Impl/Field.h"

#endif