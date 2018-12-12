#include "luacppb/Core/StackGuard.h"
#include "luacppb/Core/Error.h"

namespace LuaCppB {

  LuaStackCanary::LuaStackCanary(lua_State *state)
    : state(state) {
      this->size = lua_gettop(state);
  }

  bool LuaStackCanary::check(int offset) const noexcept {
    return lua_gettop(this->state) == this->size + offset;
  }

  void LuaStackCanary::assume(int offset) const {
    const int assumed = this->size + offset;
    if (lua_gettop(this->state) != assumed) {
      throw LuaCppBError("Assumed stack size " + std::to_string(assumed), LuaCppBErrorCode::StackSizeMismatch);
    }
  }

  LuaStackGuard::LuaStackGuard(lua_State *state)
    : state(state) {}

  std::size_t LuaStackGuard::size() const noexcept {
    return static_cast<std::size_t>(lua_gettop(this->state));
  }

  bool LuaStackGuard::checkCapacity(std::size_t slots) const noexcept {
    return static_cast<bool>(lua_checkstack(this->state, slots));
  }

  void LuaStackGuard::assumeCapacity(std::size_t slots) const {
    if (!lua_checkstack(this->state, slots)) {
      throw LuaCppBError("Expected at least " + std::to_string(slots) + " stack slots");
    }
  }

  bool LuaStackGuard::checkIndex(int index) const noexcept {
    return index <= lua_gettop(this->state) && index >= -lua_gettop(this->state);
  }

  void LuaStackGuard::assumeIndex(int index) const {
    if (!(index <= lua_gettop(this->state) && index >= -lua_gettop(this->state))) {
      throw LuaCppBError("Assumed stack index " + std::to_string(index) + " to be valid");
    }
  }

  LuaStackCanary LuaStackGuard::canary() const noexcept {
    return LuaStackCanary(this->state);
  }
}