#include "catch.hpp"
#include "luacppb/LuaCppB.h"

using namespace LuaCppB;

TEST_CASE("Value constructors") {
  REQUIRE(LuaValue().getType() == LuaType::Nil);
  REQUIRE(LuaValue(LuaInteger(1)).getType() == LuaType::Number);
  REQUIRE(LuaValue(LuaNumber(1.0)).getType() == LuaType::Number);
  REQUIRE(LuaValue(LuaBoolean(true)).getType() == LuaType::Boolean);
  REQUIRE(LuaValue(LuaString("Hello, world!")).getType() == LuaType::String);
  REQUIRE(LuaValue(static_cast<LuaCFunction>(nullptr)).getType() == LuaType::Function);
  REQUIRE(LuaValue(LuaTable()).getType() == LuaType::Table);
}

TEST_CASE("Value create method") {
  SECTION("Integer") {
    LuaValue value = LuaValue::create(1);
    REQUIRE(value.getType() == LuaType::Number);
    REQUIRE(value.get<int>() == 1);
    REQUIRE(static_cast<int>(value) == 1);
    REQUIRE(value.get<float>() == 1.0);
    REQUIRE(static_cast<float>(value) == 1.0);
  }
  SECTION("Number") {
    LuaValue value = LuaValue::create(3.14);
    REQUIRE(value.getType() == LuaType::Number);
    REQUIRE(value.get<int>() == 3);
    REQUIRE(static_cast<int>(value) == 3);
    REQUIRE(value.get<float>() == 3.14f);
    REQUIRE(static_cast<float>(value) == 3.14f);
  }
  SECTION("Boolean") {
    LuaValue value = LuaValue::create(true);
    REQUIRE(value.getType() == LuaType::Boolean);
    REQUIRE(value.get<bool>());
    REQUIRE(static_cast<bool>(value));
  }
  SECTION("String") {
    const std::string &str = "Hello, world!";
    LuaValue value = LuaValue::create(str);
    REQUIRE(value.getType() == LuaType::String);
    REQUIRE(value.get<std::string>().compare(str) == 0);
  }
  SECTION("CFunction") {
    LuaCFunction fn = nullptr;
    LuaValue value = LuaValue::create(fn);
    REQUIRE(value.getType() == LuaType::Function);
    REQUIRE(value.get<LuaCFunction>() == fn);
  }
}

TEST_CASE("Value peek method") {
  LuaEnvironment env;
  lua_State *state = env.getState();
  REQUIRE_FALSE(LuaValue::peek(state).has_value());
  Internal::LuaStackGuard guard(state);
  auto canary = guard.canary();
  SECTION("Integer") {
    lua_pushinteger(state, 1);
    REQUIRE(LuaValue::peek(state).value().get<int>() == 1);
    REQUIRE(canary.check(1));
  }
  SECTION("Number") {
    lua_pushnumber(state, 3.14);
    REQUIRE(LuaValue::peek(state).value().get<float>() == 3.14f);
    REQUIRE(canary.check(1));
  }
  SECTION("Boolean") {
    lua_pushboolean(state, true);
    REQUIRE(LuaValue::peek(state).value().get<bool>());
    REQUIRE(canary.check(1));
  }
  SECTION("String") {
    const char *str = "Hello, world!";
    lua_pushstring(state, str);
    REQUIRE(LuaValue::peek(state).value().get<std::string>().compare(str) == 0);
    REQUIRE(canary.check(1));
  }
  SECTION("Function") {
    LuaCFunction fn = nullptr;
    lua_pushcfunction(state, fn);
    REQUIRE(((LuaCFunction) LuaValue::peek(state).value().get<LuaCFunction>()) == fn);
    REQUIRE(canary.check(1));
  }
  SECTION("Table") {
    lua_newtable(state);
    REQUIRE(LuaValue::peek(state).value().getType() == LuaType::Table);
    REQUIRE(canary.check(1));
  }
  SECTION("Peeking wrong index") {
    REQUIRE_THROWS(LuaValue::peek(state, 1000));
  }
  SECTION("Light userdata") {
    int value = 100;
    LuaValue(static_cast<void *>(&value)).push(state);
    auto data = LuaValue::peek(state).value();
    REQUIRE(data.getType() == LuaType::LightUserData);
    REQUIRE(canary.check(1));
  }
}

TEST_CASE("Value push method") {
  LuaEnvironment env;
  lua_State *state = env.getState();
  Internal::LuaStack stack(state);
  SECTION("Nil") {
    LuaValue().push(state);
    REQUIRE(lua_isnil(state, -1));
  }
  SECTION("Integer") {
    LuaValue::create(100).push(state);
    REQUIRE(lua_isinteger(state, -1));
    REQUIRE(stack.toInteger() == 100);
  }
  SECTION("Number") {
    LuaValue::create(3.14).push(state);
    REQUIRE(lua_isnumber(state, -1));
    REQUIRE(stack.toNumber() == 3.14);
  }
  SECTION("Boolean") {
    LuaValue::create(true).push(state);
    REQUIRE(lua_isboolean(state, -1));
    REQUIRE(stack.toBoolean());
  }
  SECTION("String") {
    const std::string &str = "Hello, world!";
    LuaValue::create(str).push(state);
    REQUIRE(lua_isstring(state, -1));
    REQUIRE(str.compare(stack.toString()) == 0);
  }
  SECTION("Function") {
    LuaCFunction fn = nullptr;
    LuaValue::create(fn).push(state);
    REQUIRE(lua_iscfunction(state, -1));
    REQUIRE(stack.toCFunction() == fn);
    LuaFunction func = LuaValue::peek(state).value().get<LuaFunction>();
    stack.pop();
    LuaValue(func).push(state);
  }
  SECTION("Table") {
    LuaTable table = LuaTable::create(state);
    table.ref(env)["test"] = 100;
    LuaValue(table).push(state);
    REQUIRE(lua_istable(state, -1));
  }
  SECTION("Light userdata") {
    int value = 100;
    LuaValue(static_cast<void *>(&value)).push(state);
    REQUIRE(lua_islightuserdata(state, -1));
  }
  SECTION("Userdata") {
    auto udata = LuaUserData::create(state, 100);
    LuaValue(udata).push(state);
    REQUIRE(lua_isuserdata(state, -1));
  }
  SECTION("Thread") {
    auto thread = LuaThread::create(state);
    LuaValue(thread).push(state);
    REQUIRE(lua_isthread(state, -1));
  }
}