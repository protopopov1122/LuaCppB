#include "luacppb/Reference/Registry.h"
#include "luacppb/Core/StackGuard.h"

namespace LuaCppB {

  LuaUniqueRegistryHandle::LuaUniqueRegistryHandle()
    : state(nullptr), ref(0) {}

  LuaUniqueRegistryHandle::LuaUniqueRegistryHandle(lua_State *state, int index)
    : state(state) {
    if (state) {
      lua_pushvalue(state, index);
      this->ref = luaL_ref(state, LUA_REGISTRYINDEX);
    }
  }

  LuaUniqueRegistryHandle::LuaUniqueRegistryHandle(const LuaUniqueRegistryHandle &handle)
    : state(handle.state), ref(0) {
    if (this->state) {
      lua_rawgeti(this->state, LUA_REGISTRYINDEX, handle.ref);
      this->ref = luaL_ref(state, LUA_REGISTRYINDEX);
    }
  }

  LuaUniqueRegistryHandle::~LuaUniqueRegistryHandle() {
    if (this->state) {
      luaL_unref(this->state, LUA_REGISTRYINDEX, this->ref);
    }
  }

  bool LuaUniqueRegistryHandle::isValid() const {
    return this->state != nullptr;
  }

  bool LuaUniqueRegistryHandle::get(std::function<void (lua_State *)> callback) const {
    if (this->state) {
      LuaStackGuard guard(this->state);
      lua_rawgeti(this->state, LUA_REGISTRYINDEX, this->ref);
      auto canary = guard.canary();
      callback(this->state);
      canary.assume();
      lua_pop(this->state, 1);
    }
    return this->state != nullptr;
  }

  bool LuaUniqueRegistryHandle::set(std::function<void (lua_State *)> gen) {
    if (this->state) {
      LuaStackGuard guard(this->state);
      luaL_unref(this->state, LUA_REGISTRYINDEX, this->ref);
      auto canary = guard.canary();
      gen(this->state);
      canary.assume(1);
      this->ref = luaL_ref(state, LUA_REGISTRYINDEX);
    }
    return this->state != nullptr;
  }

  LuaSharedRegistryHandle::LuaSharedRegistryHandle()
    : handle(nullptr) {}
  
  LuaSharedRegistryHandle::LuaSharedRegistryHandle(lua_State *state, int index) {
    this->handle = std::make_shared<LuaUniqueRegistryHandle>(state, index);
  }

  LuaSharedRegistryHandle::LuaSharedRegistryHandle(const LuaSharedRegistryHandle &handle)
    : handle(handle.handle) {}
  
  LuaSharedRegistryHandle::LuaSharedRegistryHandle(const LuaRegistryHandle &handle) {
    handle.get([&](lua_State *state) {
      this->handle = std::make_shared<LuaUniqueRegistryHandle>(state, -1);
    });
  }

  bool LuaSharedRegistryHandle::isValid() const {
    return this->handle != nullptr && this->handle->isValid();
  }

  bool LuaSharedRegistryHandle::get(std::function<void (lua_State *)> callback) const {
    if (this->handle) {
      return this->handle->get(callback);
    } else {
      return false;
    }
  }

  bool LuaSharedRegistryHandle::set(std::function<void (lua_State *)> gen) {
    if (this->handle) {
      return this->handle->set(gen);
    } else {
      return false;
    }
  }
}