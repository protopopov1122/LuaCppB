#include "catch.hpp"
#include "luacppb/Core/State.h"
#include "luacppb/Object/Object.h"
#include "luacppb/Reference/Reference.h"
#include "luacppb/Object/Class.h"
#include "luacppb/Object/Registry.h"

using namespace LuaCppB;

class Arith {
 public:
  Arith() : n(10) {}
  Arith(int n) : n(n) {}
  int add(int x) const {
    return n + x;
  }
  int sub(int x) {
    return n - x;
  }
  void set(int n) {
    this->n = n;
  }

  static Arith &getGlobal() {
    return Arith::global;
  }

  static Arith *getGlobalPointer() {
    return &Arith::global;
  }

  static std::unique_ptr<Arith> newArith(int n) {
    return std::make_unique<Arith>(n);
  }

  static std::shared_ptr<Arith> newSharedArith(int n) {
    return std::make_shared<Arith>(n);
  }
 private:
  static Arith global;
  int n;
};

Arith Arith::global(0);

TEST_CASE("Object binding") {
  const std::string &CODE = "result = { arith:add(50), arith:sub(100) }";
  LuaEnvironment env;
  Arith arith(10);
  LuaCppObject aObj(arith, env);
  aObj.bind("add", &Arith::add);
  aObj.bind("sub", &Arith::sub);
  env["arith"] = aObj;
  REQUIRE(env["arith"].exists());
  REQUIRE(env["arith"].getType() == LuaType::UserData);
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["result"][1].get<int>() == 60);
  REQUIRE(env["result"][2].get<int>() == -90);
}

TEST_CASE("Class manual binding") {
  const std::string &CODE = "a = Arith.build(55)"
                            "x = a:add(5)\n"
                            "a2 = Arith.new()\n"
                            "a2:set(50)\n"
                            "result = { x, a2:sub(20) }";
  LuaEnvironment env;
  LuaCppClass<Arith> arith(env);
  arith.bind("add", &Arith::add);
  arith.bind("sub", &Arith::sub);
  arith.bind("set", &Arith::set);
  arith.bind("build", &LuaCppConstructor<Arith, int>);
  env["Arith"] = arith;
  env.getClassRegistry().bind(arith);
  REQUIRE(env["Arith"].exists());
  REQUIRE(env["Arith"].getType() == LuaType::Table);
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["a"].exists());
  REQUIRE(env["a"].getType() == LuaType::UserData);
  REQUIRE(env["result"][1].get<int>() == 60);
  REQUIRE(env["result"][2].get<int>() == 30);
}

void test_object_from_invocable(LuaEnvironment &env, const std::string &CODE) {
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["arith"].exists());
  REQUIRE(env["arith"].getType() == LuaType::UserData);
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["result"][1].get<int>() == 50);
  REQUIRE(env["result"][2].get<int>() == -100);
}

void test_object_return_from_invocable(LuaEnvironment &env) {
  const std::string &CODE = "arith = getArith()\n"
                            "result = { arith:add(50), arith:sub(100) }";
  test_object_from_invocable(env, CODE);
}

void test_object_build_from_invocable(LuaEnvironment &env) {
  const std::string &CODE = "arith = getArith(0)\n"
                            "result = { arith:add(50), arith:sub(100) }";
  test_object_from_invocable(env, CODE);
}

void test_object_smart_pointer(LuaEnvironment &env) {
  const std::string &CODE = "result = { arith:add(50), arith:sub(100) }";
  REQUIRE(env["arith"].exists());
  REQUIRE(env["arith"].getType() == LuaType::UserData);
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["result"][1].get<int>() == 60);
  REQUIRE(env["result"][2].get<int>() == -90);
}

TEST_CASE("Object opaque binding") {
  LuaEnvironment env;
  LuaCppClass<Arith> arithCl("Arith", env);
  arithCl.bind("add", &Arith::add);
  arithCl.bind("sub", &Arith::sub);
  arithCl.bind("set", &Arith::set);
  env.getClassRegistry().bind(arithCl);
  SECTION("Assigning object") {
    SECTION("Assigning object by reference") {
      const std::string &CODE = "result = { arith:add(50), arithRef:sub(100) }";
      Arith arith(10);
      env["arith"] = arith;
      env["arithRef"] = std::ref(arith);
      REQUIRE(env["arith"].exists());
      REQUIRE(env["arithRef"].exists());
      REQUIRE(env["arith"].getType() == LuaType::UserData);
      REQUIRE(env["arithRef"].getType() == LuaType::UserData);
      REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
      REQUIRE(env["result"][1].get<int>() == 60);
      REQUIRE(env["result"][2].get<int>() == -90);
    }
    SECTION("Assigning object unique pointer") {
      env["arith"] = std::make_unique<Arith>(10);
      test_object_smart_pointer(env);
    }
    SECTION("Assigning object shared pointer") {
      env["arith"] = std::make_shared<Arith>(10);
      test_object_smart_pointer(env);
    }
  }
  SECTION("Returning object from invocable") {
    SECTION("Object pointer") {
      env["getArith"] = &Arith::getGlobal;
      test_object_return_from_invocable(env);
    }
    SECTION("Object reference") {
      env["getArith"] = &Arith::getGlobalPointer;
      test_object_return_from_invocable(env);
    }
    SECTION("Object unique pointer") {
      env["getArith"] = &Arith::newArith;
      test_object_build_from_invocable(env);
    }
    SECTION("Object shared pointer") {
      env["getArith"] = &Arith::newSharedArith;
      test_object_build_from_invocable(env);
    }
  }
}