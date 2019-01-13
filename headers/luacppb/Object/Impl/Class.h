#ifndef LUACPPB_OBJECT_IMPL_CLASS_H_
#define LUACPPB_OBJECT_IMPL_CLASS_H_

#include "luacppb/Object/Class.h"

namespace LuaCppB {

  template <typename C, typename ... A>
  std::unique_ptr<C> LuaCppConstructor(A... args) {
    return std::make_unique<C>(args...);
  }

  template <typename C, typename P>
  LuaCppClass<C, P>::LuaCppClass(const std::string &className, LuaCppRuntime &runtime)
    : className(className), runtime(runtime) {
    if constexpr (!std::is_void<P>::value) {
      this->runtime.getObjectBoxerRegistry().template fillFields<P>(this->fields);
    }
  }
  
  template <typename C, typename P>
  LuaCppClass<C, P>::LuaCppClass(LuaCppRuntime &runtime)
    : className(typeid(C).name()), runtime(runtime) {
    if constexpr (!std::is_void<P>::value) {
      this->runtime.getObjectBoxerRegistry().template fillFields<P>(this->fields);
    }
  }

  template <typename C, typename P>
  const std::string &LuaCppClass<C, P>::getClassName() const {
    return this->className;
  }

  template <typename C, typename P>
  void LuaCppClass<C, P>::fillFields(std::map<std::string, std::shared_ptr<Internal::LuaCppObjectFieldPusher>> &fields) {
    fields.insert(this->fields.begin(), this->fields.end());
  }
  
  template <typename C, typename P>
  void LuaCppClass<C, P>::push(lua_State *state) const {
    Internal::LuaStack stack(state);
    std::string fullName = this->fullName();
    if (stack.metatable(this->className)) {
      stack.push(fullName);
      stack.setField(-2, "__name");
      stack.pushTable();
      if constexpr (!std::is_void<P>::value) {
        std::string parentName = this->runtime.getObjectBoxerRegistry().template getClassName<P>();
        if (!parentName.empty()) {
          luaL_setmetatable(state, parentName.c_str());
        }
      }
      for (auto it = this->methods.begin(); it != this->methods.end(); ++it) {
        it->second->push(state);
        stack.setField(-2, it->first);
      }

      Internal::LuaCppObjectFieldController::pushFunction(state, this->fields);
      stack.push(&LuaCppClass<C, P>::lookupObject, 2);
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

  template <typename C, typename P>
  void LuaCppClass<C, P>::bind(lua_State *state) {
    this->push(state);
    lua_pop(state, 1);
  }

  template <typename C, typename P>
  template <typename R, typename ... A>
  void LuaCppClass<C, P>::bind(const std::string &key, R (C::*method)(A...)) {
    using M = R (C::*)(A...);
    this->methods[key] = std::make_shared<Internal::LuaCppObjectMethodCall<C, M, R, A...>>(method, this->fullName(), this->runtime);
  }

  template <typename C, typename P>
  template <typename R, typename ... A>
  void LuaCppClass<C, P>::bind(const std::string &key, R (C::*method)(A...) const) {
    using M = R (C::*)(A...) const;
    this->methods[key] = std::make_shared<Internal::LuaCppObjectMethodCall<const C, M, R, A...>>(method, this->fullName(), this->runtime);
  }

  template <typename C, typename P>
  template <typename R, typename ... A>
  void LuaCppClass<C, P>::bind(const std::string &key, R (*function)(A...)) {
    this->staticMethods[key] = std::make_shared<Internal::NativeFunctionCall<Internal::LuaNativeValue, R, A...>>(function, this->runtime);
  }

  template <typename C, typename P>
  template <typename T>
  void LuaCppClass<C, P>::bind(const std::string &key, T C::*field) {
    this->fields[key] = std::make_shared<Internal::LuaCppObjectFieldHandle<C, T>>(field, this->runtime);
  }

  template <typename C, typename P>
  std::string LuaCppClass<C, P>::fullName() const {
    if constexpr (!std::is_void<P>::value) {
      return this->runtime.getObjectBoxerRegistry().template getClassName<P>() + "::" + this->className;
    } else {
      return this->className;
    }
  }

  template <typename C, typename P>
  int LuaCppClass<C, P>::lookupObject(lua_State *state) {
    Internal::LuaStack stack(state);
    
    stack.copy(lua_upvalueindex(2));
    stack.copy(1);
    stack.copy(2);
    lua_pcall(state, 2, 1, 0);
    
    if (lua_isnoneornil(state, -1)) {
      stack.pop(1);
      stack.copy(lua_upvalueindex(1));
      stack.copy(2);
      lua_gettable(state, -2);
      lua_remove(state, -2);
    }
    return 1;
  }

  template <typename C, typename P>
  int LuaCppClass<C, P>::newObject(lua_State *state) {
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

  template <typename C, typename P>
  int LuaCppClass<C, P>::gcObject(lua_State *state) {
    Internal::LuaStack stack(state);
    std::string className = stack.toString(lua_upvalueindex(1));
    LuaCppObjectWrapper<C> *object = stack.checkUserData<LuaCppObjectWrapper<C>>(1, className);
    if (object) {
      object->~LuaCppObjectWrapper();
      ::operator delete(object, object);
    }
    return 0;
  }
}

#endif