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

#ifndef LUACPPB_OBJECT_REGISTRY_H_
#define LUACPPB_OBJECT_REGISTRY_H_

#include "luacppb/Object/Class.h"
#include "luacppb/Object/Boxer.h"
#include "luacppb/Core/Error.h"
#include "luacppb/Core/Stack.h"
#include <typeindex>
#include <typeinfo>

namespace LuaCppB::Internal {

  template <typename T, typename P>
  class LuaCppClassObjectBoxer : public LuaCppObjectBoxer {
   public:
    LuaCppClassObjectBoxer(const std::string &, std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> &);

    void wrap(lua_State *, void *) override;
    void wrap(lua_State *, const void *) override;
    void wrapUnique(lua_State *, void *, std::unique_ptr<LuaCppObjectDeleter>) override;
    void wrapShared(lua_State *, std::shared_ptr<void>) override;
    const std::string &getClassName() override;
    void copyFields(std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> &) override;
   private:
    std::string className;
    std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> fields;
  };

  class LuaCppClassRegistry : public LuaCppObjectBoxerRegistry {
   public:
    using LuaCppObjectBoxerRegistry::LuaCppObjectBoxerRegistry;

    template <typename T, typename P>
    void bind(LuaCppClass<T, P> &);
  };
}

#endif
