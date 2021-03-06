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

class TestClass {};
class TestClass2 {};
class TestClass3 {
 public:
  TestClass3() = delete;
  TestClass3(int i) {}

  TestClass3 *get() {
    return this;
  }
};

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
  SECTION("Invalid references") {
    LuaReferenceHandle ref;
    REQUIRE_FALSE(ref.valid());
    REQUIRE_FALSE(ref.exists());
    auto subref = ref["test"];
    REQUIRE_FALSE(subref.valid());
    REQUIRE_FALSE(subref.exists());
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

TEST_CASE("Stack reference") {
  LuaEnvironment env;
  Internal::LuaStack stack(env.getState());
  stack.push(100);
  Internal::LuaStackReference ref(env, -1);
  REQUIRE(ref.putOnTop([](lua_State *state) {
    REQUIRE(lua_tointeger(state, -1) == 100);
  }));
  REQUIRE_FALSE(ref.setValue([](lua_State *state) {
    REQUIRE(false);
  }));
}

TEST_CASE("Registry reference") {
  LuaEnvironment env;
  Internal::LuaStack stack(env.getState());
  stack.push(100);
  REQUIRE_THROWS(Internal::LuaRegistryReference(nullptr, env, -1));
  Internal::LuaRegistryReference ref(env.getState(), env, -1);
  REQUIRE(ref.putOnTop([](lua_State *state) {
    REQUIRE(lua_tointeger(state, -1) == 100);
  }));
  REQUIRE(ref.setValue([](lua_State *state) {
    lua_pushinteger(state, 200);
  }));
  REQUIRE(ref.putOnTop([](lua_State *state) {
    REQUIRE(lua_tointeger(state, -1) == 200);
  }));
}

TEST_CASE("Object getting") {
  LuaEnvironment env;
  ClassBinder<TestClass>::bind(env);
  TestClass obj;
  env["obj"] = obj;
  REQUIRE(static_cast<TestClass *>(env["obj"]) == &obj);
  REQUIRE_THROWS(static_cast<TestClass2 *>(env["obj"]) == nullptr);
  REQUIRE_THROWS(static_cast<TestClass3 &>(env["obj"]).get() == nullptr);
  REQUIRE(env["obj"].get<TestClass *>() == &obj);
  REQUIRE_THROWS(env["obj"].get<TestClass2 *>() == nullptr);
  REQUIRE_THROWS(env["obj"].get<TestClass3 &>().get() == nullptr);
  env.setExceptionHandler([](auto&) {});
  REQUIRE(static_cast<TestClass2 *>(env["obj"]) == nullptr);
  REQUIRE_THROWS(static_cast<TestClass3 &>(env["obj"]).get() != nullptr);
  REQUIRE(env["obj"].get<TestClass2 *>() == nullptr);
  REQUIRE_THROWS(env["obj"].get<TestClass3 &>().get() == nullptr);
}

TEST_CASE("Object assignment") {
  LuaEnvironment env;
  ClassBinder<TestClass>::bind(env);
  REQUIRE_NOTHROW(env["obj"] = TestClass());
  REQUIRE_NOTHROW(env["obj2"] = TestClass2());
  REQUIRE(env["obj2"].get<LuaValue>().getType() == LuaType::Nil);
}

TEST_CASE("Metatable operations") {
  const std::string &CODE = "res = tbl.a + tbl.b + getmetatable(tbl).__index.b";
  LuaEnvironment env;
  auto table = LuaFactory::newTable(env);
  table.setMetatable(*LuaFactory::newTable(env));
  auto metatable = table.getMetatable();
  auto index = LuaFactory::newTable(env);
  metatable["__index"] = *index;
  table["a"] = 25;
  index["b"] = 50;
  env["tbl"] = *table;
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["res"].get<int>() == 125);
}

TEST_CASE("Table iterator") {
  const std::string &CODE = "tbl = {\n"
                            "    a = 1,\n"
                            "    b = 2,\n"
                            "    c = 3,\n"
                            "    d = 4\n"
                            "}";
  LuaEnvironment env;
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  std::string line;
  int sum = 0;
  for (auto it = env["tbl"].begin(); it != env["tbl"].end(); ++it) {
    auto entry = *it;
    line.append(entry.first.get<const std::string &>());
    sum += entry.second.get<int>();
    int value = env["tbl"][entry.first];
    sum += value;
  }
  for (TableIterator it(env["tbl"].begin()); !(it == env["tbl"].end()); it++) {
    auto entry = *it;
    line.append(entry.first.get<const std::string &>());
    sum += entry.second.get<int>();
    int value = env["tbl"][entry.first];
    sum += value;
  }
  std::sort(line.begin(), line.end());
  REQUIRE(line.compare("aabbccdd") == 0);
  REQUIRE(sum == 40);
}

TEST_CASE("Reference comparison") {
  const std::string &CODE = "a = 2\n"
                            "b = 4\n"
                            "c = b - a";
  LuaEnvironment env;
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["a"] == env["c"]);
  REQUIRE(env["a"] != env["b"]);
  REQUIRE(env["a"] != LuaReferenceHandle());
  REQUIRE(LuaReferenceHandle() == LuaReferenceHandle());
}

TEST_CASE("Persistent references") {
  LuaEnvironment env;
  env["a"] = 10;
  auto ref1 = env["a"];
  auto ref2 = ref1.persistent();
  REQUIRE(ref1 == ref2);
}

TEST_CASE("Reference copying") {
  LuaEnvironment env;
  auto ref = env["a"];
  ref = 5;
  auto ref2 = ref;
  auto ref3 = ref.persistent();
  REQUIRE(env["a"].get<int>() == 5);
  REQUIRE(ref2.get<int>() == 5);
  REQUIRE(ref3.get<int>() == 5);
  ref2 = 10;
  REQUIRE(env["a"].get<int>() == 10);
  REQUIRE(ref.get<int>() == 10);
  REQUIRE(ref2.get<int>() == 10);
  REQUIRE(ref3.get<int>() == 5);
}