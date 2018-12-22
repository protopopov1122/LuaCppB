#include "catch.hpp"
#include "luacppb/Core/State.h"
#include "luacppb/Container/Container.h"
#include "luacppb/Object/Object.h"
#include "luacppb/Reference/Reference.h"
#include "luacppb/Object/Class.h"
#include "luacppb/Object/Registry.h"

using namespace LuaCppB;

void test_basic_operations(LuaEnvironment &env) {
  const std::string &CODE = "sum = 0\n"
                            "for i = 1, #vec do\n"
                            "    sum = sum + vec[i]\n"
                            "end";
  REQUIRE(env["vec"].exists());
  REQUIRE(env["vec"].getType() == LuaType::UserData);
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["sum"].get<int>() == 10);
}

TEST_CASE("Vector") {
  LuaEnvironment env;
  SECTION("Basic vectors") {
    std::vector<int> vec = {1, 2, 3, 4};
    env["vec"] = vec;
    test_basic_operations(env);
  }
  SECTION("Unique pointer vectors") {
    std::vector<int> vec = {1, 2, 3, 4};
    env["vec"] = std::make_unique<std::vector<int>>(vec);
    test_basic_operations(env);
  }
  SECTION("Unique pointer vectors") {
    std::vector<int> vec = {1, 2, 3, 4};
    env["vec"] = std::make_shared<std::vector<int>>(vec);
    test_basic_operations(env);
  }
}