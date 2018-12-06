#include "luacppb/Reference/Reference.h"
#include "luacppb/Core/StackGuard.h"

namespace LuaCppB {

  bool LuaTableField::putOnTop(std::function<void (lua_State *)> callback) {
    bool result = false;
    this->ref.getReference().putOnTop([&](lua_State *state) {
      LuaStackGuard guard(state);
      if (lua_istable(state, -1)) {
        lua_getfield(state, -1, this->name.c_str());
        if (!lua_isnone(state, -1)) {
          auto canary = guard.canary();
          callback(state);
          canary.assume();
          result = true;
        }
        lua_pop(state, 1);
      }
    });
    return result;
  }

  bool LuaTableField::setValue(std::function<void (lua_State *)> gen) {
    bool result = false;
    this->ref.getReference().putOnTop([&](lua_State *state) {
      LuaStackGuard guard(state);
      if (lua_istable(state, -1)) {
        auto canary = guard.canary();
        gen(state);
        canary.assume(1);
        lua_setfield(state, -2, this->name.c_str());
        result = true;
      }
    });
    return result;
  }

  bool LuaArrayField::putOnTop(std::function<void (lua_State *)> callback) {
    bool result = false;
    this->ref.getReference().putOnTop([&](lua_State *state) {
      LuaStackGuard guard(state);
      if (lua_istable(state, -1)) {
        lua_geti(state, -1, this->index);
        if (!lua_isnone(state, -1)) {
          auto canary = guard.canary();
          callback(state);
          canary.assume();
          result = true;
        }
        lua_pop(state, 1);
      }
    });
    return result;
  }

  bool LuaArrayField::setValue(std::function<void (lua_State *)> gen) {
    bool result = false;
    this->ref.getReference().putOnTop([&](lua_State *state) {
      LuaStackGuard guard(state);
      if (lua_istable(state, -1)) {
        auto canary = guard.canary();
        gen(state);
        canary.assume(1);
        lua_seti(state, -2, this->index);
        result = true;
      }
    });
    return result;
  }
}