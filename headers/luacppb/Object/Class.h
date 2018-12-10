#ifndef LUACPPB_OBJECT_CLASS_H_
#define LUACPPB_OBJECT_CLASS_H_

#include "luacppb/Base.h"
#include "luacppb/Object/Method.h"
#include <map>
#include <type_traits>

namespace LuaCppB {

	template <typename C, typename ... A>
	class CppObjectInitializer : public LuaData {
		using F = void (*)(C *, A...);
	 public:
		CppObjectInitializer(const std::string &className, F fn) : className(className), function(fn) {}

		void push(lua_State *state) const override {
			lua_pushlightuserdata(state, reinterpret_cast<void *>(this->function));
      lua_pushstring(state, this->className.c_str());
			lua_pushcclosure(state, CppObjectInitializer<C, A...>::function_closure, 2);
		}
	 private:
		static int call(F function, const char *className, lua_State *state) {
			std::tuple<A...> args = NativeFunctionArgumentsTuple<1, A...>::value(state);
      CppObjectWrapper<C> *object = reinterpret_cast<CppObjectWrapper<C> *>(lua_newuserdata(state, sizeof(CppObjectWrapper<C>)));
      new(object) CppObjectWrapper<C>();
      std::tuple<C *, A...> allArgs = std::tuple_cat(std::make_tuple(object->get()), args);
      std::apply(function, allArgs);
      luaL_setmetatable(state, className);
      return 1;
		};

		static int function_closure(lua_State *state) {
			const void *fn = lua_topointer(state, lua_upvalueindex(1));
      const char *className = lua_tostring(state, lua_upvalueindex(2));
			return CppObjectInitializer<C, A...>::call(reinterpret_cast<F>(fn), className, state);
		};

    std::string className;
		F function;
	};

  template <typename C>
  class LuaCppClass : public LuaData {
   public:
    LuaCppClass(const std::string &className)
      : className(className) {}
    
    void push(lua_State *state) const override {
      if (luaL_newmetatable(state, this->className.c_str()) != 0) {
        lua_newtable(state);
        for (auto it = this->methods.begin(); it != this->methods.end(); ++it) {
          it->second->push(state);
          lua_setfield(state, -2, it->first.c_str());
        }
        lua_setfield(state, -2, "__index");
        if constexpr (std::is_default_constructible<C>::value) {
          lua_pushstring(state, this->className.c_str());
          lua_pushcclosure(state, &LuaCppClass<C>::newObject, 1);
          lua_setfield(state, -2, "new");
        }
        lua_pushcfunction(state, &LuaCppClass<C>::gcObject);
        lua_setfield(state, -2, "__gc");
        for (auto it = this->initializers.begin(); it != this->initializers.end(); ++it) {
          it->second->push(state);
          lua_setfield(state, -2, it->first.c_str());
        }
      } else {
        luaL_getmetatable(state, this->className.c_str());
      }
    }

    template <typename R, typename ... A>
    void bind(const std::string &key, R (C::*method)(A...)) {
      this->methods[key] = std::make_shared<CppObjectMethodCall<C, R, A...>>(NativeMethodWrapper(method).get());
    }

    template <typename R, typename ... A>
    void bind(const std::string &key, R (C::*method)(A...) const) {
      this->methods[key] = std::make_shared<CppObjectMethodCall<C, R, A...>>(NativeMethodWrapper(method).get());
    }

    template <typename ... A>
    void initializer(const std::string &key, void (*initializer)(C *, A...)) {
      this->initializers[key] = std::make_shared<CppObjectInitializer<C, A...>>(this->className, initializer);
    }
   private:
    static int newObject(lua_State *state) {
      if constexpr (std::is_default_constructible<C>::value) {
        const char *className = lua_tostring(state, lua_upvalueindex(1));
        CppObjectWrapper<C> *object = reinterpret_cast<CppObjectWrapper<C> *>(lua_newuserdata(state, sizeof(CppObjectWrapper<C>)));
        new(object) CppObjectWrapper<C>();
        new(object->get()) C();
        luaL_setmetatable(state, className);
      } else {
        lua_pushnil(state);
      }
      return 1;
    }

    static int gcObject(lua_State *state) {
      CppObjectWrapper<C> *object = reinterpret_cast<CppObjectWrapper<C> *>(lua_touserdata(state, 1));
      object->~CppObjectWrapper();
      ::operator delete(object, object);
      return 0;
    }

    std::string className;
    std::map<std::string, std::shared_ptr<LuaData>> methods;
    std::map<std::string, std::shared_ptr<LuaData>> initializers;
  };

  template <typename C>
  class CppClassBindingBase {
   public:
    CppClassBindingBase(const std::string &className) : luaClass(className) {}
    virtual ~CppClassBindingBase() = default;

    void push(lua_State *state) {
      this->luaClass.push(state);
    }

    void push(lua_State *state, C *object) {
      CppObjectWrapper<C> *wrapper = reinterpret_cast<CppObjectWrapper<C> *>(lua_newuserdata(state, sizeof(CppObjectWrapper<C>)));
      new(wrapper) CppObjectWrapper<C>(object);
      this->luaClass.push(state);
      lua_setmetatable(state, -2);
    }

    void push(lua_State *state, C &object) {
      this->push(state, &object);
    }
   protected:
    LuaCppClass<C> luaClass;
  };

  template <typename C>
  class CppClassBinding {};
}

#endif