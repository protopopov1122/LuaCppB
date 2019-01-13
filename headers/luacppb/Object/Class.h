#ifndef LUACPPB_OBJECT_CLASS_H_
#define LUACPPB_OBJECT_CLASS_H_

#include "luacppb/Base.h"
#include "luacppb/Value/Native.h"
#include "luacppb/Invoke/ArgRet.h"
#include "luacppb/Object/Method.h"
#include "luacppb/Object/Field.h"
#include "luacppb/Core/Stack.h"
#include <map>
#include <type_traits>
#include <typeinfo>

namespace LuaCppB {

  template <typename C, typename ... A>
  std::unique_ptr<C> LuaCppConstructor(A...);

  template <typename C, typename P = void>
  class LuaCppClass : public LuaData {
   public:
    LuaCppClass(const std::string &, LuaCppRuntime &);
    LuaCppClass(LuaCppRuntime &);

    const std::string &getClassName() const;

    void fillFields(std::map<std::string, std::shared_ptr<Internal::LuaCppObjectFieldPusher>> &);
    
    void push(lua_State *) const override;
    void bind(lua_State *);

    template <typename R, typename ... A>
    void bind(const std::string &, R (C::*)(A...));

    template <typename R, typename ... A>
    void bind(const std::string &, R (C::*)(A...) const);

    template <typename R, typename ... A>
    void bind(const std::string &, R (*)(A...));

    template <typename T>
    void bind(const std::string &, T C::*);
   private:
    std::string fullName() const;
    static int lookupObject(lua_State *);
    static int newObject(lua_State *);
    static int gcObject(lua_State *);

    std::string className;
    std::map<std::string, std::shared_ptr<LuaData>> methods;
    std::map<std::string, std::shared_ptr<LuaData>> staticMethods;
    std::map<std::string, std::shared_ptr<Internal::LuaCppObjectFieldPusher>> fields;
    LuaCppRuntime &runtime;
  };
}

#include "luacppb/Object/Impl/Class.h"

#endif