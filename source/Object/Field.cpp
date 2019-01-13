#include "luacppb/Object/Field.h"

namespace LuaCppB::Internal {


  LuaCppObjectFieldController::LuaCppObjectFieldController(const std::map<std::string, std::shared_ptr<LuaCppObjectFieldPusher>> &fields)
    : fields(fields) {}

  void LuaCppObjectFieldController::get(lua_State *state, void *object, const std::string &key) {
    if (this->fields.count(key)) {
      this->fields[key]->push(state, object);
    } else {
      LuaValue().push(state);
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
    void *object = stack.toPointer<void *>(1);
    std::string key = stack.toString(2);
    if (controller) {
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