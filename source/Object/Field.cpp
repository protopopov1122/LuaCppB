/*
  SPDX short identifier: MIT

  Copyright 2018-2019 Jevgēnijs Protopopovs

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

#include "luacppb/Object/Field.h"
#include "luacppb/Core/Impl/Stack.h"

namespace LuaCppB::Internal {


  LuaCppObjectFieldController::LuaCppObjectFieldController(const std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> &fields)
    : fields(fields) {}

  void LuaCppObjectFieldController::get(lua_State *state, void *object, const std::string &key) {
    if (this->fields.count(key)) {
      this->fields[key]->push(state, object);
    } else {
      LuaValue::Nil.push(state);
    }
  }

  void LuaCppObjectFieldController::push(lua_State *state, const std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> &fields) {
    Internal::LuaStack stack(state);
    LuaCppObjectFieldController *controller = stack.push<LuaCppObjectFieldController>();
    new(controller) LuaCppObjectFieldController(fields);
    stack.pushTable();
    stack.push(&LuaCppObjectFieldController::gcObject);
    stack.setField(-2, "__gc");
    stack.setMetatable(-2);
  }

  void LuaCppObjectFieldController::pushFunction(lua_State *state, const std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> &fields) {
    Internal::LuaStack stack(state);
    LuaCppObjectFieldController::push(state, fields);
    stack.push(&LuaCppObjectFieldController::indexFunction, 1);
  }

  int LuaCppObjectFieldController::indexFunction(lua_State *state) {
    Internal::LuaStack stack(state);
    LuaCppObjectFieldController *controller = stack.toUserData<LuaCppObjectFieldController *>(lua_upvalueindex(1));
    void *object = stack.toUserData<void *>(1);
    std::string key = stack.toString(2);
    if (controller && object) {
      controller->get(state, object, key);
    } else {
      stack.push();
    }
    return 1;
  }

  int LuaCppObjectFieldController::gcObject(lua_State *state) {
    Internal::LuaStack stack(state);
    LuaCppObjectFieldController *controller = stack.toUserData<LuaCppObjectFieldController *>(1);
    if (controller) {
      controller->~LuaCppObjectFieldController();
      ::operator delete(controller, controller);
    }
    return 0;
  }
}
