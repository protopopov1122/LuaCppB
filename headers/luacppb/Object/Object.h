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
      lua_pushlightuserdata(state, this->object);
      lua_newtable(state);
      lua_newtable(state);
      for (auto it = this->methods.begin(); it != this->methods.end(); ++it) {
        it->second->push(state);
        lua_setfield(state, -2, it->first.c_str());
      }
      lua_setfield(state, -2, "__index");
      lua_setmetatable(state, -2);
    }
   private:
    T *object;
    std::map<std::string, std::shared_ptr<LuaData>> methods;
  };
}

#endif