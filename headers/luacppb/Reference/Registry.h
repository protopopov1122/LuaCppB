#ifndef LUACPPB_REFERENCE_REGISTRY_H_
#define LUACPPB_REFERENCE_REGISTRY_H_

#include "luacppb/Base.h"
#include <functional>
#include <memory>

namespace LuaCppB {

  class LuaRegistryHandle {
   public:
    virtual ~LuaRegistryHandle() = default;
    virtual bool isValid() const = 0;
    virtual void get(std::function<void (lua_State *)>) const = 0;
    virtual void set(std::function<void (lua_State *)>) = 0;
  };

  class LuaUniqueRegistryHandle : public LuaRegistryHandle {
   public:
    LuaUniqueRegistryHandle();
    LuaUniqueRegistryHandle(lua_State *, int = -1);
    LuaUniqueRegistryHandle(const LuaUniqueRegistryHandle &);
    virtual ~LuaUniqueRegistryHandle();

    bool isValid() const override;
    void get(std::function<void(lua_State *)>) const override;
    void set(std::function<void(lua_State *)>) override;
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

    bool isValid() const override;
    void get(std::function<void(lua_State *)>) const override;
    void set(std::function<void(lua_State *)>) override;
   private:
    std::shared_ptr<LuaUniqueRegistryHandle> handle;
  };
}

#endif