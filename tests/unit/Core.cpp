#include "catch.hpp"
#include "luacppb/State.h"
#include "luacppb/Core/StackGuard.h"

using namespace LuaCppB;

TEST_CASE("Stack guard") {
  LuaEnvironment env;
  lua_State *state = env.getState();
  LuaStackGuard guard(state);
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