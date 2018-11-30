#include "luacppb/Reference/Registry.h"

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

  void LuaUniqueRegistryHandle::get(std::function<void (lua_State *)> callback) const {
    if (this->state) {
      lua_rawgeti(this->state, LUA_REGISTRYINDEX, this->ref);
      callback(this->state);
      lua_pop(this->state, 1);
    }
  }

  void LuaUniqueRegistryHandle::set(std::function<void (lua_State *)> gen) {
    if (this->state) {
      luaL_unref(this->state, LUA_REGISTRYINDEX, this->ref);
      gen(this->state);
      this->ref = luaL_ref(state, LUA_REGISTRYINDEX);
    }
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

  void LuaSharedRegistryHandle::get(std::function<void (lua_State *)> callback) const {
    if (this->handle) {
      this->handle->get(callback);
    }
  }

  void LuaSharedRegistryHandle::set(std::function<void (lua_State *)> gen) {
    if (this->handle) {
      this->handle->set(gen);
    }
  }
}