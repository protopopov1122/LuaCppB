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

  LuaInteger::operator lua_Integer() const {
    return this->integer;
  }

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

  LuaNumber::operator lua_Number() const {
    return this->number;
  }
  
  void LuaNumber::push(lua_State *state) const {
    Internal::LuaStack stack(state);
    stack.push(this->number);
  }

  LuaNumber LuaNumber::get(lua_State *state, int index) {
    Internal::LuaStack stack(state);
    return LuaNumber(stack.toNumber(index));
  }

  LuaBoolean::LuaBoolean(bool b) : boolean(b) {}

  LuaBoolean::operator bool() const {
    return this->boolean;
  }

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
  
  LuaString::operator const std::string &() const {
    return this->string;
  }

  void LuaString::push(lua_State *state) const {
    Internal::LuaStack stack(state);
    stack.push(this->string);
  }

  LuaString LuaString::get(lua_State *state, int index) {
    Internal::LuaStack stack(state);
    return LuaString(stack.toString(index));
  }

  LuaReferencedValue::LuaReferencedValue()
    : handle()  {}

  LuaReferencedValue::LuaReferencedValue(lua_State *state, int index)
    : handle(state, index) {}

  LuaReferencedValue::LuaReferencedValue(const LuaReferencedValue &base)
    : handle(base.handle) {}

  void LuaReferencedValue::push(lua_State *state) const {
    this->handle.push(state);
  }

  bool LuaReferencedValue::hasValue() const {
    return this->handle.hasValue();
  }

  LuaReferenceHandle LuaReferencedValue::ref(LuaCppRuntime &runtime) {
    LuaReferenceHandle handle;
    this->handle.get([&](lua_State *state) {
      handle = LuaReferenceHandle(state, std::make_unique<Internal::LuaRegistryReference>(state, runtime));
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
  
  LuaStatusCode LuaThread::status() const {
    LuaStatusCode status;
    this->handle.get([&](lua_State *state) {
      status = static_cast<LuaStatusCode>(lua_status(lua_tothread(state, -1)));
    });
    return status;
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

  bool LuaFunction::isCFunction() const {
    bool cfun = false;
    this->handle.get([&](lua_State *state) {
      cfun = static_cast<bool>(lua_iscfunction(state, -1));
    });
    return cfun;
  }

  LuaCFunction LuaFunction::toCFunction() const {
    LuaCFunction fn = nullptr;
    this->handle.get([&](lua_State *state) {
      fn = lua_tocfunction(state, -1);
    });
    return fn;
  }

  LuaFunction LuaFunction::get(lua_State *state, int index) {
    return LuaFunction(state, index);
  }

  LuaFunction LuaFunction::create(lua_State *state, LuaCFunction fn, int upvalues) {
    Internal::LuaStack stack(state);
    stack.push(fn, upvalues);
    LuaFunction fun(state);
    stack.pop();
    return fun;
  }
}