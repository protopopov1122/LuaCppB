#include "luacppb/Reference/Reference.h"
#include "luacppb/Core/StackGuard.h"

namespace LuaCppB {

  bool LuaTableField::putOnTop(std::function<void (lua_State *)> callback) {
    bool result = false;
    this->ref.getReference().putOnTop([&](lua_State *state) {
      LuaStackGuard guard(state);
      LuaStack stack(state);
      if (stack.is<LuaType::Table>()) {
        stack.getField(-1, this->name);
        if (!stack.is<LuaType::None>()) {
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
      LuaStack stack(state);
      if (stack.is<LuaType::Table>()) {
        auto canary = guard.canary();
        gen(state);
        canary.assume(1);
        stack.setField(-2, this->name);
        result = true;
      }
    });
    return result;
  }

  bool LuaArrayField::putOnTop(std::function<void (lua_State *)> callback) {
    bool result = false;
    this->ref.getReference().putOnTop([&](lua_State *state) {
      LuaStackGuard guard(state);
      LuaStack stack(state);
      if (stack.is<LuaType::Table>()) {
        stack.getIndex(-1, this->index);
        if (!stack.is<LuaType::None>()) {
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
      LuaStack stack(state);
      if (stack.is<LuaType::Table>()) {
        auto canary = guard.canary();
        gen(state);
        canary.assume(1);
        stack.setIndex(-2, this->index);
        result = true;
      }
    });
    return result;
  }
}