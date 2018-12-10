#include "luacppb/Reference/Reference.h"

namespace LuaCppB {

  LuaReferenceHandle::LuaReferenceHandle(const LuaReferenceHandle &handle) : state(handle.state) {
    handle.getReference().putOnTop([&](lua_State *state) {
      this->ref = std::make_unique<LuaRegistryReference>(state, handle.getClassRegistry(), -1);
    });
  }

  LuaReference &LuaReferenceHandle::getReference() const {
    return *this->ref;
  }

  CppClassRegistry &LuaReferenceHandle::getClassRegistry() const {
    return this->ref->getClassRegistry();
  }

  LuaReferenceHandle LuaReferenceHandle::operator[](const std::string &name) {
    return LuaReferenceHandle(this->state, std::make_unique<LuaTableField>(*this, this->ref->getClassRegistry(), name));
  }

  LuaReferenceHandle LuaReferenceHandle::operator[](lua_Integer index) {
    return LuaReferenceHandle(this->state, std::make_unique<LuaArrayField>(*this, this->ref->getClassRegistry(), index));
  }

  LuaValue LuaReferenceHandle::operator*() {
    return this->get<LuaValue>();
  }

  bool LuaReferenceHandle::exists() {
    bool exists = false;
    this->ref->putOnTop([&](lua_State *state) {
      exists = !(lua_isnone(state, -1) || lua_isnil(state, -1));
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

  LuaReferenceHandle &LuaReferenceHandle::operator=(LuaData &pushable) {
    this->ref->set(pushable);
    return *this;
  }

  LuaReferenceHandle &LuaReferenceHandle::operator=(const LuaReferenceHandle &handle) {
    this->state = handle.state;
    handle.getReference().putOnTop([&](lua_State *state) {
      this->ref = std::make_unique<LuaRegistryReference>(state, handle.getClassRegistry(), -1);
    });
    return *this;
  }
}