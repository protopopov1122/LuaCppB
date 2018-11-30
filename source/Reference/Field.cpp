#include "luacppb/Reference/Reference.h"

namespace LuaCppB {

  void LuaTableField::putOnTop(std::function<void (lua_State *)> callback) {
    this->ref.getReference().putOnTop([&](lua_State *state) {
      if (lua_istable(state, -1)) {
        lua_getfield(state, -1, this->name.c_str());
        if (!lua_isnone(state, -1)) {
          callback(state);
        }
        lua_pop(state, 1);
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
}