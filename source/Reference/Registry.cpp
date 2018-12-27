#include "luacppb/Reference/Registry.h"
#include "luacppb/Core/StackGuard.h"
#include "luacppb/Core/Stack.h"

namespace LuaCppB::Internal {

  LuaUniqueRegistryHandle::LuaUniqueRegistryHandle()
    : state(nullptr), ref(0) {}

  LuaUniqueRegistryHandle::LuaUniqueRegistryHandle(lua_State *state, int index)
    : state(state) {
    if (state) {
      Internal::LuaStack stack(state);
      stack.copy(index);
      this->ref = stack.ref();
    }
  }

  LuaUniqueRegistryHandle::LuaUniqueRegistryHandle(const LuaUniqueRegistryHandle &handle)
    : state(handle.state), ref(0) {
    if (this->state) {
      Internal::LuaStack stack(state);
      stack.getIndex<true>(LUA_REGISTRYINDEX, handle.ref);
      this->ref = stack.ref();
    }
  }

  LuaUniqueRegistryHandle::~LuaUniqueRegistryHandle() {
    if (this->state) {
      Internal::LuaStack stack(this->state);
      stack.unref(this->ref);
    }
  }

  bool LuaUniqueRegistryHandle::hasValue() const {
    return this->state != nullptr;
  }

  bool LuaUniqueRegistryHandle::get(std::function<void (lua_State *)> callback) const {
    if (this->state) {
      Internal::LuaStackGuard guard(this->state);
      Internal::LuaStack stack(this->state);
      stack.getIndex<true>(LUA_REGISTRYINDEX, this->ref);
      auto canary = guard.canary();
      callback(this->state);
      canary.assume();
      lua_pop(this->state, 1);
    }
    return this->state != nullptr;
  }

  bool LuaUniqueRegistryHandle::set(std::function<void (lua_State *)> gen) {
    if (this->state) {
      Internal::LuaStackGuard guard(this->state);
      Internal::LuaStack stack(this->state);
      stack.unref(this->ref);
      auto canary = guard.canary();
      gen(this->state);
      canary.assume(1);
      this->ref = stack.ref();
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

  bool LuaSharedRegistryHandle::hasValue() const {
    return this->handle != nullptr && this->handle->hasValue();
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