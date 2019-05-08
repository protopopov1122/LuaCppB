/*
  SPDX short identifier: MIT

  Copyright 2018-2019 Jevgēnijs Protopopovs

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

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

    void copyFields(std::map<std::string, std::shared_ptr<Internal::LuaCppObjectFieldPusher>> &);
    
    void push(lua_State *) const override;
    void bind(lua_State *);

    template <typename R, typename ... A>
    void bind(const std::string &, R (C::*)(A...));

    template <typename R, typename ... A>
    void bind(const std::string &, R (C::*)(A...) const);

    template <typename R, typename ... A>
    void bind(const std::string &, R (*)(A...));

    template <typename T, typename B>
    typename std::enable_if<std::is_base_of<B, C>::value>::type
      bind(const std::string &, T B::*);

    template <typename V>
    typename std::enable_if<Internal::LuaValueWrapper<V>::Conversible>::type bind(const std::string &, V &);

    template <typename V>
    typename std::enable_if<Internal::LuaValueWrapper<V>::Conversible>::type bind(const std::string &, V &&);
    
   private:
    std::string fullName() const;
    static int lookupObject(lua_State *);
    static int newObject(lua_State *);
    static int gcObject(lua_State *);

    void setupParentClassStaticMembers(lua_State *) const;
    void setupParentClassInstanceMembers(lua_State *) const;
    void setupInstanceMembers(lua_State *) const;
    void setupStaticMembers(lua_State *) const;

    std::string className;
    std::map<std::string, std::shared_ptr<LuaData>> methods;
    std::map<std::string, std::shared_ptr<LuaData>> staticMethods;
    std::map<std::string, std::shared_ptr<Internal::LuaCppObjectFieldPusher>> fields;
    std::map<std::string, LuaValue> dataFields;
    LuaCppRuntime &runtime;
  };
}

#endif
