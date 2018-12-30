#ifndef LUACPPB_OBJECT_OBJECT_H_
#define LUACPPB_OBJECT_OBJECT_H_

#include "luacppb/Base.h"
#include "luacppb/Object/Method.h"
#include "luacppb/Core/Stack.h"
#include <map>
#include <type_traits>
#include <sstream>
#include <typeinfo>
#include <atomic>

namespace LuaCppB {

  template <typename T>
  class LuaCppObject : public LuaData {
   public:
    LuaCppObject(T *obj, LuaCppRuntime &runtime) : object(obj), runtime(runtime) {
      std::stringstream ss;
      ss << "$object" << (LuaCppObject::nextIdentifier++) << '$' << typeid(T).name() << '@' << this->object;
      this->className = ss.str();
    }
    LuaCppObject(T &obj, LuaCppRuntime &runtime) : LuaCppObject(&obj, runtime) {}

    template <typename R, typename ... A>
    void bind(const std::string &key, R (T::*method)(A...)) {
      using M = R (T::*)(A...);
      this->methods[key] = std::make_shared<Internal::LuaCppObjectMethodCall<T, M, R, A...>>(method, this->className, this->runtime);
    }

    template <typename R, typename ... A>
    void bind(const std::string &key, R (T::*method)(A...) const) {
      using M = R (T::*)(A...) const;
      this->methods[key] = std::make_shared<Internal::LuaCppObjectMethodCall<const T, M, R, A...>>(method, this->className, this->runtime);
    }

    void push(lua_State *state) const override {
      Internal::LuaStack stack(state);
      LuaCppObjectWrapper<T> *object = stack.push<LuaCppObjectWrapper<T>>();
      new(object) LuaCppObjectWrapper<T>(this->object);
      if (stack.metatable(this->className)) {
        stack.pushTable();
        for (auto it = this->methods.begin(); it != this->methods.end(); ++it) {
          it->second->push(state);
          stack.setField(-2, it->first);
        }
        stack.setField(-2, "__index");
        stack.push(&LuaCppObject<T>::gcObject);
        stack.setField(-2, "__gc");
      }
      stack.setMetatable(-2);
    }
   private:
    static int gcObject(lua_State *state) {
      Internal::LuaStack stack(state);
      LuaCppObjectWrapper<T> *object = stack.toUserData<LuaCppObjectWrapper<T> *>(1);
      object->~LuaCppObjectWrapper();
      ::operator delete(object, object);
      return 0;
    }

    T *object;
    std::map<std::string, std::shared_ptr<LuaData>> methods;
    LuaCppRuntime &runtime;
    std::string className;

    static std::atomic<uint64_t> nextIdentifier;
  };

  template <typename T>
  std::atomic<uint64_t> LuaCppObject<T>::nextIdentifier(0);
}

#endif