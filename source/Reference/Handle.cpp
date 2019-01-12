#include "luacppb/Reference/Handle.h"
#include "luacppb/Reference/Primary.h"
#include "luacppb/Reference/Field.h"
#include "luacppb/Core/Stack.h"

namespace LuaCppB {

  LuaReferenceHandle::LuaReferenceHandle()
    : state(nullptr), ref(nullptr) {}
  
  LuaReferenceHandle::LuaReferenceHandle(lua_State *state, std::unique_ptr<Internal::LuaReference> ref)
    : state(state), ref(std::move(ref)) {}

  LuaReferenceHandle::LuaReferenceHandle(const LuaReferenceHandle &handle) : state(handle.state) {
    handle.getReference().putOnTop([&](lua_State *state) {
      this->ref = std::make_unique<Internal::LuaRegistryReference>(state, handle.getRuntime(), -1);
    });
  }

  LuaReferenceHandle::LuaReferenceHandle(LuaReferenceHandle &&handle)
    : state(handle.state), ref(std::move(handle.ref)) {
    handle.state = nullptr;
    handle.ref = nullptr;
  }

  Internal::LuaReference &LuaReferenceHandle::getReference() const {
    return *this->ref;
  }

  LuaCppRuntime &LuaReferenceHandle::getRuntime() const {
    return this->ref->getRuntime();
  }

  LuaReferenceHandle LuaReferenceHandle::operator[](const std::string &name) {
    if (this->ref) {
      return LuaReferenceHandle(this->state, std::make_unique<Internal::LuaTableField>(*this, this->ref->getRuntime(), name));
    } else {
      return LuaReferenceHandle();
    }
  }

  LuaReferenceHandle LuaReferenceHandle::operator[](const char *name)  {
    return this->operator[](std::string(name));
  }

  LuaReferenceHandle LuaReferenceHandle::operator[](lua_Integer index) {
    if (this->ref) {
      return LuaReferenceHandle(this->state, std::make_unique<Internal::LuaArrayField>(*this, this->ref->getRuntime(), index));
    } else {
      return LuaReferenceHandle();
    }
  }

  LuaValue LuaReferenceHandle::operator*() {
    return this->get<LuaValue>();
  }

  bool LuaReferenceHandle::exists() {
    bool exists = false;
    if (this->ref) {
      this->ref->putOnTop([&](lua_State *state) {
        Internal::LuaStack stack(state);
        exists = !(stack.is<LuaType::None>() || stack.is<LuaType::Nil>());
      });
    }
    return exists;
  }

  LuaType LuaReferenceHandle::getType() {
    LuaType type = LuaType::None;
    if (this->ref) {
      this->ref->putOnTop([&](lua_State *state) {
        type = static_cast<LuaType>(lua_type(state, -1));
      });
    }
    return type;
  }

  LuaReferenceHandle &LuaReferenceHandle::operator=(const LuaReferenceHandle &handle) {
    this->state = handle.state;
    handle.getReference().putOnTop([&](lua_State *state) {
      this->ref = std::make_unique<Internal::LuaRegistryReference>(state, handle.getRuntime(), -1);
    });
    return *this;
  }

  LuaReferenceHandle LuaReferenceHandle::getMetatable() {
    LuaReferenceHandle handle;
    this->getReference().putOnTop([&](lua_State *state) {
      lua_getmetatable(state, -1);
      handle = LuaReferenceHandle(state, std::make_unique<Internal::LuaRegistryReference>(state, this->getRuntime(), -1));
      lua_pop(state, 1);
    });
    return handle;
  }

  void LuaReferenceHandle::setMetatable(LuaData &data) {
    this->getReference().putOnTop([&](lua_State *state) {
      data.push(state);
      lua_setmetatable(state, -2);
    });
  }

  void LuaReferenceHandle::setMetatable(LuaData &&data) {
    this->setMetatable(data);
  }
}