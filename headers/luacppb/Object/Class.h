#ifndef LUACPPB_OBJECT_CLASS_H_
#define LUACPPB_OBJECT_CLASS_H_

#include "luacppb/Base.h"
#include "luacppb/Value/Native.h"
#include "luacppb/Invoke/ArgRet.h"
#include "luacppb/Object/Method.h"
#include "luacppb/Core/Stack.h"
#include <map>
#include <type_traits>
#include <typeinfo>

namespace LuaCppB {

  template <typename C, typename ... A>
  std::unique_ptr<C> LuaCppConstructor(A... args) {
    return std::make_unique<C>(args...);
  }

  template <typename C, typename P = void>
  class LuaCppClass : public LuaData {
   public:
    LuaCppClass(const std::string &className, LuaCppRuntime &runtime)
      : className(className), runtime(runtime) {}
    
    LuaCppClass(LuaCppRuntime &runtime)
      : className(typeid(C).name()), runtime(runtime) {}

    const std::string &getClassName() const {
      return this->className;
    }
    
    void push(lua_State *state) const override {
      Internal::LuaStack stack(state);
      std::string fullName = this->fullName();
      if (stack.metatable(this->className)) {
        stack.push(fullName);
        stack.setField(-2, "__name");
        stack.pushTable();
        if constexpr (!std::is_void<P>::value) {
          std::string parentName = this->runtime.getObjectBoxerRegistry().getClassName<P>();
          if (!parentName.empty()) {
            luaL_setmetatable(state, parentName.c_str());
          }
        }
        for (auto it = this->methods.begin(); it != this->methods.end(); ++it) {
          it->second->push(state);
          stack.setField(-2, it->first);
        }
        stack.setField(-2, "__index");
        if constexpr (std::is_default_constructible<C>::value) {
          stack.push(this->className);
          stack.push(&LuaCppClass<C, P>::newObject, 1);
          stack.setField(-2, "new");
        }
        for (auto it = this->staticMethods.begin(); it != this->staticMethods.end(); ++it) {
          it->second->push(state);
          stack.setField(-2, it->first);
        }
        stack.push(this->className);
        stack.push(&LuaCppClass<C, P>::gcObject, 1);
        stack.setField(-2, "__gc");
      }
    }

    void bind(lua_State *state) {
      this->push(state);
      lua_pop(state, 1);
    }

    template <typename R, typename ... A>
    void bind(const std::string &key, R (C::*method)(A...)) {
      using M = R (C::*)(A...);
      this->methods[key] = std::make_shared<Internal::LuaCppObjectMethodCall<C, M, R, A...>>(method, this->fullName(), this->runtime);
    }

    template <typename R, typename ... A>
    void bind(const std::string &key, R (C::*method)(A...) const) {
      using M = R (C::*)(A...) const;
      this->methods[key] = std::make_shared<Internal::LuaCppObjectMethodCall<const C, M, R, A...>>(method, this->fullName(), this->runtime);
    }

    template <typename R, typename ... A>
    void bind(const std::string &key, R (*function)(A...)) {
      this->staticMethods[key] = std::make_shared<Internal::NativeFunctionCall<Internal::LuaNativeValue, R, A...>>(function, this->runtime);
    }
   private:
    std::string fullName() const {
      if constexpr (!std::is_void<P>::value) {
        return this->runtime.getObjectBoxerRegistry().getClassName<P>() + "::" + this->className;
      } else {
        return this->className;
      }
    }
  
    static int newObject(lua_State *state) {
      Internal::LuaStack stack(state);
      if constexpr (std::is_default_constructible<C>::value) {
        std::string className = stack.toString(lua_upvalueindex(1));
        LuaCppObjectWrapper<C> *object = stack.push<LuaCppObjectWrapper<C>>();
        new(object) LuaCppObjectWrapper<C>();
        new(object->get()) C();
        if constexpr (!std::is_void<P>()) {
          object->addParentType(std::type_index(typeid(P)));
        }
        stack.setMetatable(className);
      } else {
        stack.push();
      }
      return 1;
    }

    static int gcObject(lua_State *state) {
      Internal::LuaStack stack(state);
      std::string className = stack.toString(lua_upvalueindex(1));
      LuaCppObjectWrapper<C> *object = stack.checkUserData<LuaCppObjectWrapper<C>>(1, className);
      if (object) {
        object->~LuaCppObjectWrapper();
        ::operator delete(object, object);
      }
      return 0;
    }

    std::string className;
    std::map<std::string, std::shared_ptr<LuaData>> methods;
    std::map<std::string, std::shared_ptr<LuaData>> staticMethods;
    LuaCppRuntime &runtime;
  };
}

#endif