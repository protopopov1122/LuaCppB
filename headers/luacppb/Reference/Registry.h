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

#ifndef LUACPPB_REFERENCE_REGISTRY_H_
#define LUACPPB_REFERENCE_REGISTRY_H_

#include "luacppb/Base.h"
#include <functional>
#include <memory>

namespace LuaCppB::Internal {

  class LuaRegistryHandle {
   public:
    virtual ~LuaRegistryHandle() = default;
    virtual lua_State *getState() = 0;
    virtual bool hasValue() const = 0;
    virtual bool get(std::function<void (lua_State *)>) const = 0;
    virtual bool set(std::function<void (lua_State *)>) = 0;
    void push(lua_State *) const;
  };

  class LuaUniqueRegistryHandle : public LuaRegistryHandle {
   public:
    LuaUniqueRegistryHandle();
    LuaUniqueRegistryHandle(lua_State *, int = -1);
    LuaUniqueRegistryHandle(const LuaUniqueRegistryHandle &);
    LuaUniqueRegistryHandle(LuaUniqueRegistryHandle &&);
    virtual ~LuaUniqueRegistryHandle();

    lua_State *getState() override;
    bool hasValue() const override;
    bool get(std::function<void(lua_State *)>) const override;
    bool set(std::function<void(lua_State *)>) override;
   private:
    lua_State *state;
    int ref;
  };

  class LuaSharedRegistryHandle : public LuaRegistryHandle {
   public:
    LuaSharedRegistryHandle();
    LuaSharedRegistryHandle(lua_State *, int = -1);
    LuaSharedRegistryHandle(const LuaSharedRegistryHandle &);
    LuaSharedRegistryHandle(const LuaRegistryHandle &);

    lua_State *getState() override;
    bool hasValue() const override;
    bool get(std::function<void(lua_State *)>) const override;
    bool set(std::function<void(lua_State *)>) override;
   private:
    std::shared_ptr<LuaUniqueRegistryHandle> handle;
  };
}

#endif
