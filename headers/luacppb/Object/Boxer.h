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

#ifndef LUACPPB_OBJECT_BOXER_H_
#define LUACPPB_OBJECT_BOXER_H_

#include "luacppb/Core/Error.h"
#include <typeindex>
#include <typeinfo>
#include <map>
#include <memory>

namespace LuaCppB::Internal {

  class LuaCppObjectFieldPusher;

  class LuaCppObjectBoxer {
  public:
    virtual ~LuaCppObjectBoxer() = default;
    virtual void wrap(lua_State *, void *) = 0;
    virtual void wrap(lua_State *, const void *) = 0;
    virtual void wrapUnique(lua_State *, void *) = 0;
    virtual void wrapShared(lua_State *, std::shared_ptr<void>) = 0;
    virtual const std::string &getClassName() = 0;
    virtual void copyFields(std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> &) = 0;
  };

  class LuaCppObjectBoxerRegistry {
   public:
    LuaCppObjectBoxerRegistry(lua_State *state)
      : state(state) {}
    
    template <typename T>
    const std::string &getClassName();

    template <typename T>
    void copyFields(std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> &);

    template <typename T>
    bool canWrap();

    template <typename T>
    void wrap(lua_State *, T *);

    template <typename T>
    void wrap(lua_State *, std::unique_ptr<T>);

    template <typename T>
    void wrap(lua_State *, std::shared_ptr<T>);
   protected:
    template <typename T>
    void addBoxer(std::shared_ptr<Internal::LuaCppObjectBoxer>);

    lua_State *state;
   private:
    std::map<std::type_index, std::shared_ptr<Internal::LuaCppObjectBoxer>> wrappers;
  };
}

#include "luacppb/Object/Impl/Boxer.h"

#endif
