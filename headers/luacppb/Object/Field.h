#ifndef LUACPPB_OBJECT_FIELD_H_
#define LUACPPB_OBJECT_FIELD_H_

#include "luacppb/Base.h"
#include "luacppb/Core/Runtime.h"
#include "luacppb/Value/Native.h"
#include <memory>

namespace LuaCppB::Internal {

	template <typename C, typename T>
	class LuaCppObjectFieldHandle : public LuaData {
	 public:
		LuaCppObjectFieldHandle(C &, T C::*, LuaCppRuntime &);

		void push(lua_State *) const override;
	 private:
		C &object;
		T C::*field;
		LuaCppRuntime &runtime;
	};

	template <typename C>
	class LuaCppObjectFieldHandleBuilder {
	 public:
		virtual ~LuaCppObjectFieldHandleBuilder() = default;
		virtual std::shared_ptr<LuaData> build(C &, LuaCppRuntime &) = 0;
	};

	template <typename C, typename T>
	class LuaCppObjectFieldHandleBuilder_Impl : public LuaCppObjectFieldHandleBuilder<C> {
	 public:
		LuaCppObjectFieldHandleBuilder_Impl(T C::*);

		std::shared_ptr<LuaData> build(C &, LuaCppRuntime &) override;
	 private:
		T C::*field;
	};

  class LuaCppObjectFieldController {
   public:
    LuaCppObjectFieldController(const std::map<std::string, std::shared_ptr<LuaData>> &);

    void get(lua_State *, const std::string &);
    static void push(lua_State *, const std::map<std::string, std::shared_ptr<LuaData>> &);
    static void pushFunction(lua_State *, const std::map<std::string, std::shared_ptr<LuaData>> &);
   private:
    static int indexFunction(lua_State *);
    static int gcObject(lua_State *);
    std::map<std::string, std::shared_ptr<LuaData>> fields;
  };
}

#include "luacppb/Object/Impl/Field.h"

#endif