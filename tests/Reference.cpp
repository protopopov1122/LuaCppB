#include "catch.hpp"
#include "luacppb/State.h"
#include "luacppb/Reference/Reference.h"
#include <iostream>

using namespace LuaCppB;

TEST_CASE("Variable references") {
  LuaEnvironment env;
  REQUIRE(env.load("references.lua") == LuaStatusCode::Ok);
  SECTION("Global variables") {
    REQUIRE(env["integer"].get<int>() == 42);
    env["integer"] = 43;
    REQUIRE(env["integer"].get<int>() == 43);
  }
  SECTION("Table fields") {
    REQUIRE(env["table"]["integer"].get<int>() == 42);
    env["table"]["integer"] = 43;
    REQUIRE(env["table"]["integer"].get<int>() == 43);
  }
  SECTION("Array fields") {
    for (int i = 1; i <= 4; i++) {
      REQUIRE(env["array"][i].get<int>() == i);
      env["array"][i] = i*2;
      REQUIRE(env["array"][i].get<int>() == i*2);
    }
  }
}