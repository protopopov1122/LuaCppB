/*
  SPDX short identifier: MIT

  Copyright 2018-2019 Jevgēnijs Protopopovs

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

#include "catch.hpp"
#include "luacppb/LuaCppB.h"

using namespace LuaCppB;

void test_basic_operations(LuaEnvironment &env) {
  const std::string &CODE = "vec[1] = nil\n"
                            "sum = 0\n"
                            "for i = 1, #vec do\n"
                            "    sum = sum + vec[i]\n"
                            "end\n"
#ifdef LUACPPB_CONTAINER_PAIRS
                            "psum = 0\n"
                            "for k, v in pairs(vec) do\n"
                            "    psum = psum + k * v\n"
                            "end\n"
                            "vec[#vec + 1] = 5\n"
                            "isum = 0\n"
                            "for k, v in ipairs(vec) do\n"
                            "    isum = isum + k * v\n"
                            "end"
#endif
                            ;
  REQUIRE(env["vec"].exists());
  REQUIRE(env["vec"].getType() == LuaType::UserData);
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["sum"].get<int>() == 10);
#ifdef LUACPPB_CONTAINER_PAIRS
  REQUIRE(env["psum"].get<int>() == 30);
  REQUIRE(env["isum"].get<int>() == 55);
#endif
}

TEST_CASE("Vector") {
  LuaEnvironment env;
  SECTION("Basic vectors") {
    std::vector<int> vec = {1000, 1, 2, 3, 4};
    env["vec"] = vec;
    test_basic_operations(env);
  }
  SECTION("Unique pointer vectors") {
    std::vector<int> vec = {1000, 1, 2, 3, 4};
    env["vec"] = std::make_unique<std::vector<int>>(vec);
    test_basic_operations(env);
  }
  SECTION("Unique pointer vectors") {
    std::vector<int> vec = {1000, 1, 2, 3, 4};
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
                            "map[10] = nil\n"
                            "sz = #map\n"
#ifdef LUACPPB_CONTAINER_PAIRS
                            "psum = 0\n"
                            "for k, v in pairs(map) do\n"
                            "    psum = psum + k*v\n"
                            "end\n"
                            "isum = 0\n"
                            "for k, v in ipairs(map) do\n"
                            "    isum = isum + k*v\n"
                            "end"
#endif
                            ;
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["sz"].get<int>() == 3);
#ifdef LUACPPB_CONTAINER_PAIRS
  int psum = env["psum"].get<int>();
  int isum = env["isum"].get<int>();
  REQUIRE(psum == 140);
  REQUIRE(isum == 140);
#endif
}

TEST_CASE("Map") {
  LuaEnvironment env;
  std::map<int, int> map = {
    { 1, 10 },
    { 2, 20 },
    { 10, 100 }
  };
  SECTION("By reference") {
    env["map"] = map;
    test_map(env);
    REQUIRE(map[3] == 30);
  }
  SECTION("By const reference") {
   const std::string &CODE = "sz = #map\n"
#ifdef LUACPPB_CONTAINER_PAIRS
                             "psum = 0\n"
                             "for k, v in pairs(map) do\n"
                             "    psum = psum + k*v\n"
                             "end\n"
                             "isum = 0\n"
                             "for k, v in ipairs(map) do\n"
                             "    isum = isum + k*v\n"
                             "end"
#endif
                             ;
    const auto &cMap = map;
    env["map"] = cMap;
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["sz"].get<int>() == 3);
#ifdef LUACPPB_CONTAINER_PAIRS
    int psum = env["psum"];
    int isum = env["isum"];
    REQUIRE(psum == 1050);
    REQUIRE(isum == 50);
#endif
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
#if !defined(__clang_major__)
  env["x"] = std::optional("Hello, world!");
#else
  env["x"] = std::optional<std::string>("Hello, world!");
#endif
  env["y"] = std::optional<std::string>();
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["res"][1].get<bool>());
  REQUIRE(env["res"][2].get<bool>());
}


void test_set(LuaEnvironment &env) {
  const std::string &CODE = "numbers[10] = true\n"
                            "numbers[1] = nil\n"
                            "sz = #numbers\n"
#ifdef LUACPPB_CONTAINER_PAIRS
                            "psum = 0\n"
                            "for k, v in pairs(numbers) do\n"
                            "    psum = psum + k\n"
                            "end\n"
#endif
                            "res = numbers[2] and (numbers[100] or numbers[3])";
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["sz"].get<int>() == 5);
#ifdef LUACPPB_CONTAINER_PAIRS
  int psum = env["psum"].get<int>();
  REQUIRE(psum == 24);
#endif
  bool res = env["res"];
  REQUIRE(res);
}

TEST_CASE("Set") {
  LuaEnvironment env;
  std::set<int> numbers = { 1, 2, 3, 4, 5 };
  SECTION("By reference") {
    env["numbers"] = numbers;
    test_set(env);
    REQUIRE(numbers.count(10) != 0);
    REQUIRE(numbers.count(1) == 0);
  }
  SECTION("By constant reference") {
  const std::string &CODE = "sz = #numbers\n"
#ifdef LUACPPB_CONTAINER_PAIRS
                            "psum = 0\n"
                            "for k, v in pairs(numbers) do\n"
                            "    psum = psum + k\n"
                            "end\n"
#endif
                            "res = numbers[2] and (numbers[100] or numbers[3])";
    const auto &cNumbers = numbers;
    env["numbers"] = cNumbers;
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["sz"].get<int>() == 5);
#ifdef LUACPPB_CONTAINER_PAIRS
    int psum = env["psum"].get<int>();
    REQUIRE(psum == 15);
#endif
    bool res = env["res"];
    REQUIRE(res);
  }
  SECTION("Unique pointer") {
    env["numbers"] = std::make_unique<std::set<int>>(numbers);
    test_set(env);
  }
  SECTION("Shared pointer") {
    env["numbers"] = std::make_shared<std::set<int>>(numbers);
    test_set(env);
  }
}
