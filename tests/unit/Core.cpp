#include "catch.hpp"
#include "luacppb/Core/State.h"
#include "luacppb/Core/StackGuard.h"
#include "luacppb/Core/Stack.h"
#include "luacppb/Core/Throw.h"
#include "luacppb/Reference/Handle.h"

using namespace LuaCppB;

#ifdef LUACPPB_ERROR_SUPPORT
void test_err(LuaState state) {
  LuaThrow(state.getState(), state, "Error");
}
#endif

TEST_CASE("State") {
  LuaEnvironment env;
  SECTION("Basic operations") {
    REQUIRE(env.execute("test()") != LuaStatusCode::Ok);
    REQUIRE(env("2+2*2").get<int>() == 6);
    REQUIRE_FALSE(env("2+2*2").hasError());
    REQUIRE_FALSE(env("return 2+2*2", false).hasError());
    REQUIRE(env("2+2*2", false).hasError());
    REQUIRE(env("return 2+2*2", true).hasError());
    REQUIRE(env.execute("test()").hasError());
  }
#ifdef LUACPPB_ERROR_SUPPORT
  SECTION("Error handling") {
    env["test"] = test_err;
    LuaError err;
    env("test()").error(err);
    REQUIRE(err.getError().get<std::string>().compare("Error") == 0);
  }
#endif
}

TEST_CASE("Stack guard") {
  LuaEnvironment env;
  lua_State *state = env.getState();
  Internal::LuaStackGuard guard(state);
  SECTION("Basic operations") {
    lua_pushinteger(state, 1);
    REQUIRE(lua_gettop(state) == guard.size());
    REQUIRE(guard.checkIndex(1));
    REQUIRE_FALSE(guard.checkIndex(2));
    REQUIRE_NOTHROW(guard.assumeIndex(1));
    REQUIRE_THROWS(guard.assumeIndex(2));
    REQUIRE(guard.checkCapacity(1));
    REQUIRE_NOTHROW(guard.assumeCapacity(1));
    REQUIRE_FALSE(guard.checkCapacity(INT_MAX));
    REQUIRE_THROWS(guard.assumeCapacity(INT_MAX));
  }
  SECTION("Stack canary") {
    auto canary = guard.canary();
    REQUIRE(canary.check());
    REQUIRE_NOTHROW(canary.assume());
    lua_pushinteger(state, 1);
    REQUIRE(canary.check(1));
    REQUIRE_NOTHROW(canary.assume(1));
    REQUIRE_FALSE(canary.check());
    REQUIRE_THROWS(canary.assume());
    auto canary2 = guard.canary();
    lua_pop(state, 1);
    REQUIRE(canary2.check(-1));
    REQUIRE_NOTHROW(canary2.assume(-1));
    REQUIRE_FALSE(canary2.check());
    REQUIRE_THROWS(canary2.assume());
  }
}

int stack_test_fn(lua_State *stack) {
  return 0;
}

TEST_CASE("Stack") {
  LuaEnvironment env;
  Internal::LuaStack stack(env.getState());

  REQUIRE(stack.getTop() == 0);
  SECTION("Value pushing") {
    SECTION("Nil") {
      stack.push();
      REQUIRE(stack.getType() == LuaType::Nil);
    }
    SECTION("Integer") {
      stack.push(100);
      REQUIRE(stack.getTop() == 1);
      REQUIRE(stack.getType() == LuaType::Number);
      REQUIRE(stack.toInteger() == 100);
      auto value = stack.get();
      REQUIRE(value.has_value());
      REQUIRE(value.value().getType() == stack.getType());
      REQUIRE(value.value().get<int>() == 100);
    }
    SECTION("Float") {
      stack.push(3.14);
      REQUIRE(stack.getTop() == 1);
      REQUIRE(stack.getType() == LuaType::Number);
      REQUIRE(stack.toNumber() == 3.14);
      auto value = stack.get();
      REQUIRE(value.has_value());
      REQUIRE(value.value().getType() == stack.getType());
      REQUIRE(value.value().get<double>() == 3.14);
    }
    SECTION("Boolean") {
      stack.push(true);
      REQUIRE(stack.getTop() == 1);
      REQUIRE(stack.getType() == LuaType::Boolean);
      REQUIRE(stack.toBoolean());
      auto value = stack.get();
      REQUIRE(value.has_value());
      REQUIRE(value.value().getType() == stack.getType());
      REQUIRE(value.value().get<bool>());
    }
    SECTION("String") {
      const std::string &STR = "Hello, world!";
      stack.push(STR);
      REQUIRE(stack.getTop() == 1);
      REQUIRE(stack.getType() == LuaType::String);
      REQUIRE(STR.compare(stack.toString()) == 0);
      auto value = stack.get();
      REQUIRE(value.has_value());
      REQUIRE(value.value().getType() == stack.getType());
      REQUIRE(STR.compare(value.value().get<std::string>()) == 0);
    }
    SECTION("Function") {
      stack.push(stack_test_fn);
      REQUIRE(stack.getTop() == 1);
      REQUIRE(stack.getType() == LuaType::Function);
      REQUIRE(stack.toCFunction() == stack_test_fn);
      auto value = stack.get();
      REQUIRE(value.has_value());
      REQUIRE(value.value().getType() == stack.getType());
      REQUIRE(value.value().get<LuaCFunction>() == stack_test_fn);
    }
    SECTION("Closure") {
      stack.push(100);
      stack.push(stack_test_fn, 1);
      REQUIRE(stack.getTop() == 1);
      REQUIRE(stack.getType() == LuaType::Function);
      REQUIRE(stack.toCFunction() == stack_test_fn);
      auto value = stack.get();
      REQUIRE(value.has_value());
      REQUIRE(value.value().getType() == stack.getType());
      REQUIRE(value.value().get<LuaCFunction>() == stack_test_fn);
    }
    SECTION("Table") {
      stack.pushTable();
      REQUIRE(stack.getTop() == 1);
      REQUIRE(stack.getType() == LuaType::Table);
      auto value = stack.get();
      REQUIRE(value.has_value());
      REQUIRE(value.value().getType() == stack.getType());
    }
    SECTION("Light user data") {
      int i = 100;
      stack.push(&i);
      REQUIRE(stack.getTop() == 1);
      REQUIRE(stack.getType() == LuaType::LightUserData);
      REQUIRE(stack.toPointer<int *>() == &i);
    }
    SECTION("User data") {
      int *i = stack.push<int>();
      REQUIRE(i != nullptr);
      REQUIRE(stack.getTop() == 1);
      REQUIRE(stack.getType() == LuaType::UserData);
      REQUIRE(stack.toUserData<int *>() == i);
    }
  }
  SECTION("Stack operations") {
    stack.push(100);
    stack.push(3.14);
    stack.push(true);
    REQUIRE(stack.getTop() == 3);
    stack.copy(-3);
    REQUIRE(stack.getTop() == 4);
    REQUIRE(stack.getType() == LuaType::Number);
    stack.pop();
    REQUIRE(stack.getTop() == 3);
    stack.pop(3);
    REQUIRE(stack.getTop() == 0);
  }
}