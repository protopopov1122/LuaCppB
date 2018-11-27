#include "luacppb/Reference.h"

namespace LuaCppB {

  void LuaGlobalVariable::putOnTop(std::function<void (lua_State *)> callback) {
    if (this->state) {
      lua_getglobal(this->state, this->name.c_str());
      callback(this->state);
      lua_pop(this->state, 1);
    }
  }

  void LuaGlobalVariable::setValue(std::function<void (lua_State *)> gen) {
    if (this->state) {
      gen(this->state);
      lua_setglobal(this->state, this->name.c_str());
    }
  }

  void LuaStackReference::putOnTop(std::function<void (lua_State *)> callback) {
    lua_pushvalue(this->state, this->index);
    callback(this->state);
    lua_pop(this->state, 1);
  }

  void LuaStackReference::setValue(std::function<void (lua_State *)> gen) {
  }

  LuaReferenceHandle::LuaReferenceHandle(const LuaReferenceHandle &handle) : state(handle.state) {
    handle.getReference().putOnTop([&](lua_State *state) {
      this->ref = std::make_unique<LuaRegistryReference>(state, -1);
    });
  }

  LuaReference &LuaReferenceHandle::getReference() const {
    return *this->ref;
  }

  LuaReferenceHandle LuaReferenceHandle::operator[](const std::string &name) {
    return LuaReferenceHandle(this->state, std::make_unique<LuaTableField>(*this, name));
  }

  LuaReferenceHandle LuaReferenceHandle::operator[](lua_Integer index) {
    return LuaReferenceHandle(this->state, std::make_unique<LuaArrayField>(*this, index));
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
    handle.getReference().putOnTop([&](lua_State *state) {
      this->ref = std::make_unique<LuaRegistryReference>(state, -1);
    });
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

  void LuaTableField::setValue(std::function<void (lua_State *)> gen) {
    this->ref.getReference().putOnTop([&](lua_State *state) {
      if (lua_istable(state, -1)) {
        gen(state);
        lua_setfield(state, -2, this->name.c_str());
      }
    });
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

  void LuaArrayField::setValue(std::function<void (lua_State *)> gen) {
    this->ref.getReference().putOnTop([&](lua_State *state) {
      if (lua_istable(state, -1)) {
        gen(state);
        lua_seti(state, -2, this->index);
      }
    });
  }

  LuaRegistryReference::LuaRegistryReference(lua_State *state, int index) : state(state) {
    lua_pushvalue(state, index);
    this->ref = luaL_ref(state, LUA_REGISTRYINDEX);
  }

  LuaRegistryReference::~LuaRegistryReference() {
    luaL_unref(this->state, LUA_REGISTRYINDEX, this->ref);
  }

  void LuaRegistryReference::putOnTop(std::function<void (lua_State *)> callback) {
    lua_rawgeti(this->state, LUA_REGISTRYINDEX, this->ref);
    callback(this->state);
    lua_pop(this->state, 1);
  }

  void LuaRegistryReference::setValue(std::function<void (lua_State *)> gen) {
    luaL_unref(this->state, LUA_REGISTRYINDEX, this->ref);
    gen(this->state);
    this->ref = luaL_ref(state, LUA_REGISTRYINDEX);
  }
}