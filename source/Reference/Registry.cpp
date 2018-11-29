#include "luacppb/Reference/Registry.h"
#include <iostream>

namespace LuaCppB {

  LuaRegistryHandle::LuaRegistryHandle()
    : state(nullptr), ref(0) {}

  LuaRegistryHandle::LuaRegistryHandle(lua_State *state, int index)
    : state(state) {
    if (state) {
      lua_pushvalue(state, index);
      this->ref = luaL_ref(state, LUA_REGISTRYINDEX);
    }
  }

  LuaRegistryHandle::LuaRegistryHandle(const LuaRegistryHandle &handle)
    : state(handle.state), ref(0) {
    if (this->state) {
      lua_rawgeti(this->state, LUA_REGISTRYINDEX, handle.ref);
      this->ref = luaL_ref(state, LUA_REGISTRYINDEX);
    }
  }

  LuaRegistryHandle::~LuaRegistryHandle() {
    if (this->state) {
      luaL_unref(this->state, LUA_REGISTRYINDEX, this->ref);
    }
  }

  void LuaRegistryHandle::get(std::function<void (lua_State *)> callback) const {
    if (this->state) {
      lua_rawgeti(this->state, LUA_REGISTRYINDEX, this->ref);
      callback(this->state);
      lua_pop(this->state, 1);
    }
  }

  void LuaRegistryHandle::set(std::function<void (lua_State *)> gen) {
    if (this->state) {
      luaL_unref(this->state, LUA_REGISTRYINDEX, this->ref);
      gen(this->state);
      this->ref = luaL_ref(state, LUA_REGISTRYINDEX);
    }
  }
}