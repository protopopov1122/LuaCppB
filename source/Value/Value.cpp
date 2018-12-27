#include "luacppb/Value/Value.h"
#include "luacppb/Core/Error.h"
#include "luacppb/Core/Stack.h"

namespace LuaCppB {

  LuaType LuaValue::getType() const noexcept {
    return this->type;
  }

  void LuaValue::push(lua_State *state) const {
    Internal::LuaStack stack(state);
    switch (this->type) {
      case LuaType::Number:
        assert(this->value.index() == 0 || this->value.index() == 1);
        if (this->value.index() == 0) {
          std::get<LuaInteger>(this->value).push(state);
        } else {
          std::get<LuaNumber>(this->value).push(state);
        }
        break;
      case LuaType::Boolean:
        assert(this->value.index() == 2);
        std::get<LuaBoolean>(this->value).push(state);
        break;
      case LuaType::String:
        assert(this->value.index() == 3);
        std::get<LuaString>(this->value).push(state);
        break;
      case LuaType::Function:
        if (this->value.index() == 4) {
          stack.push(std::get<LuaCFunction>(this->value));
        } else {
          assert(this->value.index() == 9);
          std::get<LuaFunction>(this->value).push(state);
        }
        break;
      case LuaType::Table:
        assert(this->value.index() == 5);
        std::get<LuaTable>(this->value).push(state);
        break;
      case LuaType::LightUserData:
        assert(this->value.index() == 6);
        stack.push(std::get<void *>(this->value));
        break;
      case LuaType::UserData:
        assert(this->value.index() == 7);
        std::get<LuaUserData>(this->value).push(state);
        break;
      case LuaType::Thread:
        assert(this->value.index() == 8);
        std::get<LuaThread>(this->value).push(state);
      default:
        break;
    } 
  }

  std::optional<LuaValue> LuaValue::peek(lua_State *state, lua_Integer index) {
    Internal::LuaStack stack(state);
    if (index > stack.getTop()) {
      throw LuaCppBError("Lua stack overflow", LuaCppBErrorCode::StackOverflow);
    }
    std::optional<LuaValue> value;
    if (stack.isInteger(index)) {
      value = LuaValue::create<lua_Integer>(stack.toInteger(index));
    } else if (stack.is<LuaType::Number>(index)) {
      value = LuaValue::create<lua_Number>(stack.toNumber(index));
    } else if (stack.is<LuaType::Boolean>(index)) {
      value = LuaValue::create<bool>(stack.toBoolean(index));
    } else if (stack.is<LuaType::String>(index)) {
      value = LuaValue::create<std::string>(stack.toString(index));
    } else if (stack.is<LuaType::Function>(index)) {
      value = LuaValue(LuaFunction(state, index));
    } else if (stack.is<LuaType::Table>(index)) {
      value = LuaValue(LuaTable(state, index));
    } else if (stack.is<LuaType::LightUserData>(index)) {
      value = LuaValue(stack.toPointer<void *>(index));
    } else if (stack.is<LuaType::UserData>(index)) {
      value = LuaValue(LuaUserData(state, index));
    } else if (stack.is<LuaType::Thread>(index)) {
      value = LuaValue(LuaThread(state, index));
    }
    return value;
  }
}