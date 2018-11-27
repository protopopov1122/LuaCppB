#include "luacppb/Reference.h"

namespace LuaCppB {

  bool LuaReference::set(LuaData &pushable) {
    return this->setValue([&pushable](lua_State *state) {
      pushable.push(state);
    });
    return false;
  }

  void LuaGlobalVariable::putOnTop(std::function<void (lua_State *)> callback) {
    this->scope.get(this->name, callback);
  }

  bool LuaGlobalVariable::setValue(std::function<void (lua_State *)> gen) {
    this->scope.set(this->name, gen);
    return true;
  }

  void LuaStackReference::putOnTop(std::function<void (lua_State *)> callback) {
    this->stack.move(this->index, -1);
    this->stack.execute(callback);
    this->stack.move(-1, this->index);
  }

  bool LuaStackReference::setValue(std::function<void (lua_State *)> gen) {
    return false;
  }

  LuaReference &LuaReferenceHandle::getReference() {
    return *this->ref;
  }

  LuaReferenceHandle LuaReferenceHandle::operator[](const std::string &name) {
    return LuaReferenceHandle(std::make_shared<LuaTableField>(*this, name));
  }

  LuaReferenceHandle LuaReferenceHandle::operator[](lua_Integer index) {
    return LuaReferenceHandle(std::make_shared<LuaArrayField>(*this, index));
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

  LuaReferenceHandle & LuaReferenceHandle::operator=(LuaData &pushable) {
    this->ref->set(pushable);
    return *this;
  }

  void LuaTableField::putOnTop(std::function<void (lua_State *)> callback) {
    this->ref.getReference().putOnTop([&](lua_State *state) {
      if (lua_istable(state, -1)) {
        lua_getfield(state, -1, this->name.c_str());
        if (!lua_isnone(state, -1)) {
          callback(state);
        }
        lua_pop(state, -1);
      }
    });
  }

  bool LuaTableField::setValue(std::function<void (lua_State *)> gen) {
    bool result = false;
    this->ref.getReference().putOnTop([&](lua_State *state) {
      if (lua_istable(state, -1)) {
        gen(state);
        lua_setfield(state, -2, this->name.c_str());
        result = true;
      }
    });
    return result;
  }

  void LuaArrayField::putOnTop(std::function<void (lua_State *)> callback) {
    this->ref.getReference().putOnTop([&](lua_State *state) {
      if (lua_istable(state, -1)) {
        lua_geti(state, -1, this->index);
        if (!lua_isnone(state, -1)) {
          callback(state);
        }
        lua_pop(state, -1);
      }
    });
  }

  bool LuaArrayField::setValue(std::function<void (lua_State *)> gen) {
    bool result = false;
    this->ref.getReference().putOnTop([&](lua_State *state) {
      if (lua_istable(state, -1)) {
        gen(state);
        lua_seti(state, -2, this->index);
        result = true;
      }
    });
    return result;
  }
}