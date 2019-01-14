#include "luacppb/Reference/Field.h"
#include "luacppb/Core/StackGuard.h"

namespace LuaCppB::Internal {

  LuaTableField::LuaTableField(LuaReferenceHandle ref, LuaCppRuntime &runtime, const std::string &name)
    : LuaReference(runtime), ref(ref), name(name) {}

  bool LuaTableField::putOnTop(std::function<void (lua_State *)> callback) {
    bool result = false;
    this->ref.getReference().putOnTop([&](lua_State *state) {
      Internal::LuaStackGuard guard(state);
      Internal::LuaStack stack(state);
      if (stack.is<LuaType::Table>()) {
        stack.getField(-1, this->name);
        if (!stack.is<LuaType::None>()) {
          auto canary = guard.canary();
          callback(state);
          canary.assume();
          result = true;
        }
        stack.pop(1);
      }
    });
    return result;
  }

  bool LuaTableField::setValue(std::function<void (lua_State *)> gen) {
    bool result = false;
    this->ref.getReference().putOnTop([&](lua_State *state) {
      Internal::LuaStackGuard guard(state);
      Internal::LuaStack stack(state);
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

  LuaArrayField::LuaArrayField(LuaReferenceHandle ref, LuaCppRuntime &runtime, lua_Integer index)
    : LuaReference(runtime), ref(ref), index(index) {}

  bool LuaArrayField::putOnTop(std::function<void (lua_State *)> callback) {
    bool result = false;
    this->ref.getReference().putOnTop([&](lua_State *state) {
      Internal::LuaStackGuard guard(state);
      Internal::LuaStack stack(state);
      if (stack.is<LuaType::Table>()) {
        stack.getIndex(-1, this->index);
        if (!stack.is<LuaType::None>()) {
          auto canary = guard.canary();
          callback(state);
          canary.assume();
          result = true;
        }
        stack.pop(1);
      }
    });
    return result;
  }

  bool LuaArrayField::setValue(std::function<void (lua_State *)> gen) {
    bool result = false;
    this->ref.getReference().putOnTop([&](lua_State *state) {
      Internal::LuaStackGuard guard(state);
      Internal::LuaStack stack(state);
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