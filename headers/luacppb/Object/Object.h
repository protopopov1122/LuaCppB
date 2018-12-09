#ifndef LUACPPB_OBJECT_OBJECT_H_
#define LUACPPB_OBJECT_OBJECT_H_

#include "luacppb/Base.h"
#include "luacppb/Object/Method.h"
#include <map>

namespace LuaCppB {

  template <typename T>
  class LuaCppObject : public LuaData {
   public:
    LuaCppObject(T *obj) : object(obj) {}
    LuaCppObject(T &obj) : object(&obj) {}
		LuaCppObject(const T *obj) : object(const_cast<T *>(obj)) {}
		LuaCppObject(const T &obj) : object(const_cast<T *>(&obj)) {}

    template <typename R, typename ... A>
    void bind(const std::string &key, R (T::*method)(A...)) {
      this->methods[key] = std::make_shared<CppObjectMethodCall<T, R, A...>>(NativeMethodWrapper(method).get());
    }

    template <typename R, typename ... A>
    void bind(const std::string &key, R (T::*method)(A...) const) {
      this->methods[key] = std::make_shared<CppObjectMethodCall<T, R, A...>>(NativeMethodWrapper(method).get());
    }

    void push(lua_State *state) const override {
      CppObjectWrapper<T> *object = reinterpret_cast<CppObjectWrapper<T> *>(lua_newuserdata(state, sizeof(CppObjectWrapper<T>)));
      new(object) CppObjectWrapper<T>(this->object);
      lua_newtable(state);
      lua_newtable(state);
      for (auto it = this->methods.begin(); it != this->methods.end(); ++it) {
        it->second->push(state);
        lua_setfield(state, -2, it->first.c_str());
      }
      lua_setfield(state, -2, "__index");
      lua_pushcfunction(state, &LuaCppObject<T>::gcObject);
      lua_setfield(state, -2, "__gc");
      lua_setmetatable(state, -2);
    }
   private:
    static int gcObject(lua_State *state) {
      CppObjectWrapper<T> *object = reinterpret_cast<CppObjectWrapper<T> *>(lua_touserdata(state, 1));
      object->~CppObjectWrapper();
      ::operator delete(object, object);
      return 0;
    }

    T *object;
    std::map<std::string, std::shared_ptr<LuaData>> methods;
  };
}

#endif