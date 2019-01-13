#include "catch.hpp"
#include "luacppb/LuaCppB.h"

using namespace LuaCppB;

int test_sample_function(lua_State *state) {
  return 0;
}

int test_sample_function2(lua_State *state) {
  return 0;
}

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
    LuaNil().push(state);
    REQUIRE(lua_isnil(state, -1));
    REQUIRE(lua_isnil(state, -2));
  }
  SECTION("Integer") {
    LuaValue::create(100).push(state);
    REQUIRE(lua_isinteger(state, -1));
    REQUIRE(stack.toInteger() == 100);
    LuaValue(LuaInteger()).push(state);
    REQUIRE(lua_isinteger(state, -1));
    REQUIRE(stack.toInteger() == 0);
  }
  SECTION("Number") {
    LuaValue::create(3.14).push(state);
    REQUIRE(lua_isnumber(state, -1));
    REQUIRE(stack.toNumber() == 3.14);
    LuaValue(LuaNumber()).push(state);
    REQUIRE(lua_isnumber(state, -1));
    REQUIRE(stack.toNumber() == 0.0);
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
    LuaFunction func = LuaValue::peek(state).value();
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

TEST_CASE("Value get method") {
  LuaEnvironment env;
  SECTION("Integer") {
    REQUIRE(LuaValue::create(10).get<int>() == 10);
    REQUIRE(LuaValue().get<int>(100) == 100);
  }
  SECTION("Number") {
    REQUIRE(LuaValue::create(3.14).get<float>() == 3.14f);
    REQUIRE(LuaValue().get<float>(2.1828) == 2.1828f);
  }
  SECTION("Boolean") {
    REQUIRE_FALSE(LuaValue::create(false).get<bool>());
    REQUIRE(LuaValue().get<bool>(true));
  }
  SECTION("String") {
    REQUIRE(LuaValue::create("Hello").get<std::string>().compare("Hello") == 0);
    REQUIRE(LuaValue().get<std::string>("world").compare("world") == 0);
    REQUIRE(LuaValue::create("Hello").get<const std::string &>().compare("Hello") == 0);
    REQUIRE(LuaValue().get<const std::string &>("world").compare("world") == 0);
  }
  SECTION("CFunction") {
    REQUIRE(LuaValue::create(test_sample_function).get<LuaCFunction>() == test_sample_function);
    REQUIRE(LuaValue().get<LuaCFunction>(test_sample_function2) == test_sample_function2);
  }
  SECTION("Function") {
    REQUIRE(LuaValue(LuaFunction::create(env.getState(), test_sample_function, 0)).get<LuaFunction>().hasValue());
    REQUIRE_FALSE(LuaValue().get<LuaFunction>().hasValue());
  }
}

TEST_CASE("Value types get method") {
  LuaEnvironment env;
  Internal::LuaStack stack(env.getState());
  lua_State *state = env.getState();
  SECTION("Nil") {
    REQUIRE_NOTHROW(LuaNil::get(state));
  }
  SECTION("Integer") {
    stack.push(10);
    REQUIRE(LuaInteger::get(state) == 10);
  }
  SECTION("Integer") {
    stack.push(3.14);
    REQUIRE(LuaNumber::get(state) == 3.14);
  }
  SECTION("Boolean") {
    stack.push(true);
    REQUIRE(LuaBoolean::get(state));
  }
  SECTION("String") {
    stack.push(std::string("Hello"));
    REQUIRE(std::string("Hello").compare(LuaString::get(state)) == 0);
  }
  SECTION("Table") {
    stack.pushTable();
    REQUIRE(LuaTable::get(state).hasValue());
  }
  SECTION("Userdata") {
    stack.push<int>();
    REQUIRE(LuaUserData::get(state).hasValue());
  }
  SECTION("Thread") {
    LuaThread::create(state).push(state);
    REQUIRE(LuaThread::get(state).hasValue());
  }
  SECTION("Function") {
    LuaFunction::create(state, test_sample_function, 0).push(state);
    REQUIRE(LuaFunction::get(state).isCFunction());
  }
}

TEST_CASE("Value factory") {
  LuaEnvironment env;
  SECTION("Table factory") {
    const std::string &CODE = "res = tbl.hello + tbl.world";
    auto table = LuaValueFactory::newTable(env);
    table["hello"] = 123;
    table["world"] = 321;
    env["tbl"] = *table;
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["res"].get<int>() == 444);
  }
  SECTION("Function factory") {
    const std::string &CODE = "res = fun(5)";
    auto fn = LuaValueFactory::newFunction(env, [](int i) { return i * i; });
    env["fun"] = *fn;
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["res"].get<int>() == 25);
  }
#ifdef LUACPPB_COROUTINE_SUPPORT
  SECTION("Thread factory") {
    const std::string &CODE = "suc, res = coroutine.resume(coro, 10)";
    auto fn = LuaValueFactory::newFunction(env, [](int i) { return i * i; });
    auto coro = LuaValueFactory::newThread(env, fn);
    env["coro"] = *coro;
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["res"].get<int>() == 100);
  }
#endif
}