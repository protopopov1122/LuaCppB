#ifndef LUACPPB_OBJECT_OBJECT_H_
#define LUACPPB_OBJECT_OBJECT_H_

#include "luacppb/Base.h"
#include "luacppb/Object/Method.h"
#include "luacppb/Core/Stack.h"
#include <map>
#include <type_traits>

namespace LuaCppB {

  template <typename T>
  class LuaCppObject : public LuaData {
   public:
    LuaCppObject(T *obj, LuaCppRuntime &runtime) : object(obj), runtime(runtime) {}
    LuaCppObject(T &obj, LuaCppRuntime &runtime) : object(&obj), runtime(runtime) {}

    template <typename R, typename ... A>
    void bind(const std::string &key, R (T::*method)(A...)) {
      using M = R (T::*)(A...);
      this->methods[key] = std::make_shared<LuaCppObjectMethodCall<T, M, R, A...>>(method, this->runtime);
    }

    template <typename R, typename ... A>
    void bind(const std::string &key, R (T::*method)(A...) const) {
      using M = R (T::*)(A...) const;
      this->methods[key] = std::make_shared<LuaCppObjectMethodCall<const T, M, R, A...>>(method, this->runtime);
    }

    void push(lua_State *state) const override {
      LuaStack stack(state);
      LuaCppObjectWrapper<T> *object = stack.push<LuaCppObjectWrapper<T>>();
      new(object) LuaCppObjectWrapper<T>(this->object);
      stack.pushTable();
      stack.pushTable();
      for (auto it = this->methods.begin(); it != this->methods.end(); ++it) {
        it->second->push(state);
        stack.setField(-2, it->first);
      }
      stack.setField(-2, "__index");
      stack.push(&LuaCppObject<T>::gcObject);
      stack.setField(-2, "__gc");
      stack.setMetatable(-2);
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
    LuaCppRuntime &runtime;
  };
}

#endif