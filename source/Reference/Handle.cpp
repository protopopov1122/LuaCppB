#include "luacppb/Reference/Handle.h"
#include "luacppb/Reference/Primary.h"
#include "luacppb/Reference/Field.h"
#include "luacppb/Core/Stack.h"

namespace LuaCppB {

  LuaReferenceHandle::LuaReferenceHandle(const LuaReferenceHandle &handle) : state(handle.state) {
    handle.getReference().putOnTop([&](lua_State *state) {
      this->ref = std::make_unique<Internal::LuaRegistryReference>(state, handle.getRuntime(), -1);
    });
  }

  Internal::LuaReference &LuaReferenceHandle::getReference() const {
    return *this->ref;
  }

  LuaCppRuntime &LuaReferenceHandle::getRuntime() const {
    return this->ref->getRuntime();
  }

  LuaReferenceHandle LuaReferenceHandle::operator[](const std::string &name) {
    return LuaReferenceHandle(this->state, std::make_unique<Internal::LuaTableField>(*this, this->ref->getRuntime(), name));
  }

  LuaReferenceHandle LuaReferenceHandle::operator[](lua_Integer index) {
    return LuaReferenceHandle(this->state, std::make_unique<Internal::LuaArrayField>(*this, this->ref->getRuntime(), index));
  }

  LuaValue LuaReferenceHandle::operator*() {
    return this->get<LuaValue>();
  }

  bool LuaReferenceHandle::exists() {
    bool exists = false;
    this->ref->putOnTop([&](lua_State *state) {
      Internal::LuaStack stack(state);
      exists = !(stack.is<LuaType::None>() || stack.is<LuaType::Nil>());
    });
    return exists;
  }

  LuaType LuaReferenceHandle::getType() {
    LuaType type = LuaType::Nil;
    this->ref->putOnTop([&](lua_State *state) {
      type = static_cast<LuaType>(lua_type(state, -1));
    });
    return type;
  }

  LuaReferenceHandle &LuaReferenceHandle::operator=(const LuaReferenceHandle &handle) {
    this->state = handle.state;
    handle.getReference().putOnTop([&](lua_State *state) {
      this->ref = std::make_unique<Internal::LuaRegistryReference>(state, handle.getRuntime(), -1);
    });
    return *this;
  }
}