#ifndef LUACPPB_REFERENCE_REGISTRY_H_
#define LUACPPB_REFERENCE_REGISTRY_H_

#include "luacppb/Base.h"
#include <functional>
#include <memory>

namespace LuaCppB::Internal {

  class LuaRegistryHandle {
   public:
    virtual ~LuaRegistryHandle() = default;
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

    bool hasValue() const override;
    bool get(std::function<void(lua_State *)>) const override;
    bool set(std::function<void(lua_State *)>) override;
   private:
    std::shared_ptr<LuaUniqueRegistryHandle> handle;
  };
}

#endif