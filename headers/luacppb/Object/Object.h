#ifndef LUACPPB_OBJECT_OBJECT_H_
#define LUACPPB_OBJECT_OBJECT_H_

#include "luacppb/Base.h"
#include "luacppb/Object/Method.h"
#include "luacppb/Core/Stack.h"
#include <map>

namespace LuaCppB {

  template <typename T>
  class LuaCppObject : public LuaData {
   public:
    LuaCppObject(T *obj, LuaCppObjectBoxerRegistry &boxer) : object(obj), boxer(boxer) {}
    LuaCppObject(T &obj, LuaCppObjectBoxerRegistry &boxer) : object(&obj), boxer(boxer) {}
		LuaCppObject(const T *obj, LuaCppObjectBoxerRegistry &boxer) : object(const_cast<T *>(obj)), boxer(boxer) {}
		LuaCppObject(const T &obj, LuaCppObjectBoxerRegistry &boxer) : object(const_cast<T *>(&obj)), boxer(boxer) {}

    template <typename R, typename ... A>
    void bind(const std::string &key, R (T::*method)(A...)) {
      this->methods[key] = std::make_shared<LuaCppObjectMethodCall<T, R, A...>>(NativeMethodWrapper(method).get(), this->boxer);
    }

    template <typename R, typename ... A>
    void bind(const std::string &key, R (T::*method)(A...) const) {
      this->methods[key] = std::make_shared<LuaCppObjectMethodCall<T, R, A...>>(NativeMethodWrapper(method).get(), this->boxer);
    }

    void push(lua_State *state) const override {
      LuaCppObjectWrapper<T> *object = reinterpret_cast<LuaCppObjectWrapper<T> *>(lua_newuserdata(state, sizeof(LuaCppObjectWrapper<T>)));
      new(object) LuaCppObjectWrapper<T>(this->object);
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
      LuaStack stack(state);
      LuaCppObjectWrapper<T> *object = stack.toUserData<LuaCppObjectWrapper<T> *>(1);
      object->~LuaCppObjectWrapper();
      ::operator delete(object, object);
      return 0;
    }

    T *object;
    std::map<std::string, std::shared_ptr<LuaData>> methods;
    LuaCppObjectBoxerRegistry &boxer;
  };
}

#endif