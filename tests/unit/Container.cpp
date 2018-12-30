#include "catch.hpp"
#include "luacppb/Core/State.h"
#include "luacppb/Container/Container.h"
#include "luacppb/Object/Object.h"
#include "luacppb/Reference/Handle.h"
#include "luacppb/Object/Class.h"
#include "luacppb/Object/Registry.h"

using namespace LuaCppB;

void test_basic_operations(LuaEnvironment &env) {
  const std::string &CODE = "sum = 0\n"
                            "for i = 1, #vec do\n"
                            "    sum = sum + vec[i]\n"
                            "end\n"
                            "psum = 0\n"
                            "for k, v in pairs(vec) do\n"
                            "    psum = psum + k * v\n"
                            "end\n"
                            "vec[#vec + 1] = 5\n"
                            "isum = 0\n"
                            "for k, v in ipairs(vec) do\n"
                            "    isum = isum + k * v\n"
                            "end";
  REQUIRE(env["vec"].exists());
  REQUIRE(env["vec"].getType() == LuaType::UserData);
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["sum"].get<int>() == 10);
  REQUIRE(env["psum"].get<int>() == 30);
  REQUIRE(env["isum"].get<int>() == 55);
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
  SECTION("Assigning values") {
    SECTION("Primitive") {
      const std::string &CODE = "vec[2] = cvec[2] * 50";
      const std::string &CODE2 = "cvec[2] = 100";
      LuaEnvironment env;
      std::vector<int> vec = {1, 2, 3, 4};
      const std::vector<int> vecC(vec);
      env["vec"] = vec;
      env["cvec"] = vecC;
      REQUIRE(env["vec"].exists());
      REQUIRE(env["vec"].getType() == LuaType::UserData);
      REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
      REQUIRE(env.execute(CODE2) != LuaStatusCode::Ok);
      REQUIRE(vec[1] == 100);
      REQUIRE(vecC[1] == 2);
    }
    SECTION("Unsupported") {
      const std::string &CODE = "vec[2] = 100\n";
      LuaEnvironment env;
      std::vector<LuaType> vec;
      env["vec"] = vec;
      REQUIRE(env["vec"].exists());
      REQUIRE(env["vec"].getType() == LuaType::UserData);
      REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    }
  }
}

TEST_CASE("Pair") {
  SECTION("Assignment") {
    const std::string &CODE = "res = pair[1] .. ', ' .. pair[2]";
    LuaEnvironment env;
    env["pair"] = std::make_pair(500, "Hello");
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["res"].get<std::string>().compare("500, Hello") == 0);
  }
  SECTION("Function argument") {
    const std::string &CODE = "function test(pair)\n"
                              "    return pair[1] + pair[2]\n"
                              "end";
    LuaEnvironment env;
    auto pair = std::make_pair(100, 200);
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["test"](pair).get<int>() == 300);
  }
}

TEST_CASE("Tuple") {
  SECTION("Assignment") {
    const std::string &CODE = "res = (tuple[1] + tuple[2]) .. tuple[4]";
    LuaEnvironment env;
    env["tuple"] = std::make_tuple(3.14, 1, true, "Hi");
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["res"].get<std::string>().compare("4.14Hi") == 0);
  }
  SECTION("Function argument") {
    const std::string &CODE = "function sum(tuple)\n"
                              "    local sum = 0\n"
                              "    for i = 1, #tuple do\n"
                              "        sum = sum + tuple[i]\n"
                              "    end\n"
                              "    return sum\n"
                              "end";
    LuaEnvironment env;
    auto tuple = std::make_tuple(1, 2, 3, 4);
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["sum"](tuple).get<int>() == 10);
  }
}

void test_map(LuaEnvironment &env) {
  const std::string &CODE = "map[3] = map[1] + map[2]\n"
                            "sz = #map\n"
                            "psum = 0\n"
                            "for k, v in pairs(map) do\n"
                            "    psum = psum + k*v\n"
                            "end\n"
                            "isum = 0\n"
                            "for k, v in ipairs(map) do\n"
                            "    isum = isum + k*v\n"
                            "end";
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["sz"].get<int>() == 3);
  int psum = env["psum"].get<int>();
  int isum = env["isum"].get<int>();
  REQUIRE(psum == 140);
  REQUIRE(isum == 140);
}

TEST_CASE("Map") {
  LuaEnvironment env;
  std::map<int, int> map = {
    { 1, 10 },
    { 2, 20 }
  };
  SECTION("By reference") {
    env["map"] = map;
    test_map(env);
    REQUIRE(map[3] == 30);
  }
  SECTION("By const reference") {
   const std::string &CODE = "sz = #map\n"
                             "psum = 0\n"
                             "for k, v in pairs(map) do\n"
                             "    psum = psum + k*v\n"
                             "end\n"
                             "isum = 0\n"
                             "for k, v in ipairs(map) do\n"
                             "    isum = isum + k*v\n"
                             "end";
    const auto &cMap = map;
    env["map"] = cMap;
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["sz"].get<int>() == 2);
    int psum = env["psum"];
    int isum = env["isum"];
    REQUIRE(psum == 50);
    REQUIRE(isum == 50);
  }
  SECTION("Unique pointer") {
    env["map"] = std::make_unique<std::map<int, int>>(map);
    test_map(env);
  }
  SECTION("Shared pointer") {
    auto ptr = std::make_shared<std::map<int, int>>(map);
    env["map"] = ptr;
    test_map(env);
    REQUIRE(ptr->at(3) == 30);
  }
}

TEST_CASE("Optional") {
  const std::string &CODE = "res = { type(x) == 'string', type(y) == 'nil' }";
  LuaEnvironment env;
#if !defined(__clang_major__) || (__clang_major__ > 6)
  env["x"] = std::optional("Hello, world!");
#else
  env["x"] = std::optional<std::string>("Hello, world!");
#endif
  env["y"] = std::optional<std::string>();
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["res"][1].get<bool>());
  REQUIRE(env["res"][2].get<bool>());
}
