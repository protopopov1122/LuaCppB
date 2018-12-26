#include "luacppb/Value/Value.h"
#include "luacppb/Reference/Primary.h"
#include "luacppb/Reference/Field.h"
#include "luacppb/Reference/Handle.h"
#include "luacppb/Core/Error.h"
#include "luacppb/Core/Stack.h"

namespace LuaCppB {

  void LuaNil::push(lua_State *state) const {
    Internal::LuaStack stack(state);
    stack.push();
  }

  LuaNil LuaNil::get(lua_State *state, int index) {
    return LuaNil();
  }

  LuaInteger::LuaInteger() : integer(0) {}

  LuaInteger::LuaInteger(lua_Integer i) : integer(i) {}

  void LuaInteger::push(lua_State *state) const {
    Internal::LuaStack stack(state);
    stack.push(this->integer);
  }

  LuaInteger LuaInteger::get(lua_State *state, int index) {
    Internal::LuaStack stack(state);
    return LuaInteger(stack.toInteger(index));
  }

  LuaNumber::LuaNumber() : number(0) {}

  LuaNumber::LuaNumber(lua_Number n) : number(n) {}
  
  void LuaNumber::push(lua_State *state) const {
    Internal::LuaStack stack(state);
    stack.push(this->number);
  }

  LuaNumber LuaNumber::get(lua_State *state, int index) {
    Internal::LuaStack stack(state);
    return LuaNumber(stack.toNumber(index));
  }

  LuaBoolean::LuaBoolean(bool b) : boolean(b) {}

  void LuaBoolean::push(lua_State *state) const {
    Internal::LuaStack stack(state);
    stack.push(this->boolean);
  }

  LuaBoolean LuaBoolean::get(lua_State *state, int index) {
    Internal::LuaStack stack(state);
    return LuaBoolean(stack.toBoolean(index));
  }

  LuaString::LuaString(const std::string &str) : string(str) {}

  LuaString::LuaString(const char *str) : string(str) {}

  void LuaString::push(lua_State *state) const {
    Internal::LuaStack stack(state);
    stack.push(this->string);
  }

  LuaString LuaString::get(lua_State *state, int index) {
    Internal::LuaStack stack(state);
    return LuaString(stack.toString(index));
  }

  LuaCFunction::LuaCFunction(LuaCFunction_ptr fn) : function(fn) {}
  
  void LuaCFunction::push(lua_State *state) const {
    Internal::LuaStack stack(state);
    stack.push(this->function);
  }

  LuaCFunction LuaCFunction::get(lua_State *state, int index) {
    Internal::LuaStack stack(state);
    return LuaCFunction(stack.toCFunction(index));
  }

  LuaReferencedValue::LuaReferencedValue()
    : handle()  {}

  LuaReferencedValue::LuaReferencedValue(lua_State *state, int index)
    : handle(state, index) {}

  LuaReferencedValue::LuaReferencedValue(const LuaReferencedValue &base)
    : handle(base.handle) {}

  void LuaReferencedValue::push(lua_State *state) const {
    Internal::LuaStack stack(state);
    int ref = -1;
    handle.get([&](lua_State *handleState) {
      if (state == handleState) {
        stack.copy(-1);
        ref = stack.ref();
      } else {
        throw LuaCppBError("Reference handler state must match requested state", LuaCppBErrorCode::StateMismatch);
      }
    });
    stack.getIndex<true>(LUA_REGISTRYINDEX, ref);
    stack.unref(ref);
  }

  template <>
  LuaReferenceHandle LuaReferencedValue::convert<LuaReferenceHandle>() {
    LuaReferenceHandle handle;
    this->handle.get([&](lua_State *state) {
      handle = LuaReferenceHandle(state, std::make_unique<Internal::LuaRegistryReference>(state, handle.getRuntime()));
    });
    return handle;
  }

  LuaTable LuaTable::get(lua_State *state, int index) {
    return LuaTable(state, index);
  }

  LuaTable LuaTable::create(lua_State *state) {
    Internal::LuaStack stack(state);
    stack.pushTable();
    LuaTable table(state);
    stack.pop();
    return table;
  }

  LuaUserData LuaUserData::get(lua_State *state, int index) {
    return LuaUserData(state, index);
  }

  LuaUserData LuaUserData::create(lua_State *state, std::size_t sz) {
    Internal::LuaStack stack(state);
    stack.pushUserData(sz);
    LuaUserData data(state);
    stack.pop();
    return data;
  }

  lua_State *LuaThread::toState() const {
    return this->LuaReferencedValue::toPointer<lua_State *>();
  }

  LuaThread LuaThread::get(lua_State *state, int index) {
    return LuaThread(state, index);
  }

  LuaThread LuaThread::create(lua_State *state) {
    Internal::LuaStack stack(state);
    stack.pushThread();
    LuaThread thread(state);
    stack.pop();
    return thread;
  }
}