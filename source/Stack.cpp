#include "luacppb/Stack.h"
#include "luacppb/Reference.h"

namespace LuaCppB {

  LuaStack::LuaStack(lua_State *state) : state(state) {}

  std::size_t LuaStack::getTop() const {
    return static_cast<std::size_t>(lua_gettop(this->state));
  }

  void LuaStack::setTop(std::size_t index) {
    lua_settop(this->state, static_cast<int>(index));
  }

  void LuaStack::insert(int index) {
    lua_insert(this->state, index);
  }

  void LuaStack::pop(std::size_t count) {
    lua_pop(this->state, static_cast<int>(count));
  }

  void LuaStack::remove(int index) {
    lua_remove(this->state, index);
  }

  void LuaStack::replace(int index) {
    lua_replace(this->state, index);
  }

  void LuaStack::copy(int index) {
    lua_pushvalue(this->state, index);
  }

  void LuaStack::move(int fromindex, int toindex) {
    lua_copy(this->state, fromindex, toindex);
  }

  LuaType LuaStack::getType(int index) const {
    return static_cast<LuaType>(lua_type(this->state, index));
  }

  void LuaStack::execute(std::function<void(lua_State *)> callback) {
    callback(this->state);
  }

  LuaReferenceHandle LuaStack::operator[](int index) {
    return LuaReferenceHandle(std::make_unique<LuaStackReference>(*this, index));
  }
}