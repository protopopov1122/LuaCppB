#include "catch.hpp"
#include "luacppb/State.h"
#include "luacppb/Reference/Reference.h"

using namespace LuaCppB;

const std::string &CODE = "globalVariable = 42\n"
                          "table = {\n"
                          "    x = 100,\n"
                          "    nestedTable = {\n"
                          "        y = 200\n"
                          "    }\n"
                          "}\n"
                          "array = { 4, 3, 2, 1}\n"
                          "nestedArray = {{ 1, 2, 3, 4 }, {4, 3, 2, 1}}";

TEST_CASE("Global variables") {
  LuaEnvironment env;
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  SECTION("Global variable getters") {
    REQUIRE(env["globalVariable"].exists());
    REQUIRE_FALSE(env["globalVariable2"].exists());
    REQUIRE(env["globalVariable"].getType() == LuaType::Number);
    REQUIRE(env["globalVariable"].get<int>() == 42);
    REQUIRE(static_cast<int>(env["globalVariable"]) == 42);
    REQUIRE((*env["globalVariable"]).get<int>() == 42);
  }
  SECTION("Global variable setters") {
    REQUIRE_FALSE(env["globalVariable2"].exists());
    env["globalVariable2"] = true;
    REQUIRE(env["globalVariable2"].getType() == LuaType::Boolean);
    REQUIRE(env["globalVariable2"].get<bool>());
  }
}

TEST_CASE("Tables") {
  LuaEnvironment env;
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  auto table = env["table"];
  REQUIRE(table.exists());
  REQUIRE(table.getType() == LuaType::Table);
  REQUIRE(table["x"].exists());
  REQUIRE(table["x"].getType() == LuaType::Number);
  REQUIRE(table["x"].get<int>() == 100);
  REQUIRE_FALSE(table["y"].exists());
  table["y"] = 10;
  REQUIRE(table["y"].exists());
  REQUIRE(table["y"].getType() == LuaType::Number);
  REQUIRE(table["y"].get<int>() == 10);
  SECTION("Nested tables") {
    auto nested = table["nestedTable"];
    REQUIRE(nested.exists());
    REQUIRE(nested.getType() == LuaType::Table);
    REQUIRE(nested["y"].exists());
    REQUIRE(nested["y"].get<int>() == 200);
    REQUIRE_FALSE(nested["x"].exists());
  }
}

TEST_CASE("Arrays") {
  LuaEnvironment env;
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  SECTION("Arrays") {
    auto array = env["array"];
    for (int i = 1; i <= 4; i++) {
      REQUIRE(array[i].exists());
      REQUIRE(array[i].getType() == LuaType::Number);
      REQUIRE(array[i].get<int>() == 5 - i);
    }
    REQUIRE_FALSE(array[5].exists());
    array[5] = 3.14;
    REQUIRE(array[5].exists());
    REQUIRE(array[5].getType() == LuaType::Number);
    REQUIRE(array[5].get<float>() == 3.14f);
  }
  SECTION("Nested arrays") {
    auto array = env["nestedArray"];
    for (int i = 1; i <= 2; i++) {
      auto nested = array[i];
      REQUIRE(nested.exists());
      REQUIRE(nested.getType() == LuaType::Table);
      for (int j = 1; j <= 4; j++) {
        int value = i % 2 == 1 ? j : 5 - j;
        REQUIRE(nested[j].exists());
        REQUIRE(nested[j].getType() == LuaType::Number);
        REQUIRE(nested[j].get<int>() == value);
      }
    }
  }
}
