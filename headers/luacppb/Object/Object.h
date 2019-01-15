#ifndef LUACPPB_OBJECT_OBJECT_H_
#define LUACPPB_OBJECT_OBJECT_H_

#include "luacppb/Base.h"
#include "luacppb/Object/Method.h"
#include "luacppb/Object/Field.h"
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
    LuaCppObject(T *, LuaCppRuntime &);
    LuaCppObject(T &, LuaCppRuntime &);

    template <typename R, typename ... A>
    void bind(const std::string &, R (T::*)(A...));

    template <typename R, typename ... A>
    void bind(const std::string &, R (T::*)(A...) const);

    template <typename V, typename B>
    typename std::enable_if<std::is_base_of<B, T>::value>::type
      bind(const std::string &, V B::*);

    template <typename V>
    typename std::enable_if<Internal::LuaValueWrapper<V>::Conversible>::type bind(const std::string &, V &);

    template <typename V>
    typename std::enable_if<Internal::LuaValueWrapper<V>::Conversible>::type bind(const std::string &, V &&);

    void push(lua_State *) const override;
   private:
    static int lookupObject(lua_State *);
    static int gcObject(lua_State *);

    T *object;
    std::map<std::string, std::shared_ptr<LuaData>> methods;
    std::map<std::string, std::shared_ptr<Internal::LuaCppObjectFieldPusher>> fields;
    std::map<std::string, LuaValue> dataFields;
    LuaCppRuntime &runtime;
    std::string className;

    static std::atomic<uint64_t> nextIdentifier;
  };
}

#include "luacppb/Object/Impl/Object.h"

#endif