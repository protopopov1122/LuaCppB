#include "catch.hpp"
#include "luacppb/State.h"
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

  static void build(Arith *arith, int n) {
    new(arith) Arith(n);
  }
 private:
  int n;
};

TEST_CASE("Object binding") {
  const std::string &CODE = "result = { arith:add(50), arith:sub(100) }";
  LuaEnvironment env;
  Arith arith(10);
  LuaCppObject aObj(arith);
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
  const std::string &CODE = "a = Arith.build(55)\n"
                            "x = a:add(5)\n"
                            "a2 = Arith.new()\n"
                            "a2:set(50)\n"
                            "result = { x, a2:sub(20) }";
  LuaCppClass<Arith> arith("Arith");
  arith.bind("add", &Arith::add);
  arith.bind("sub", &Arith::sub);
  arith.bind("set", &Arith::set);
  arith.initializer("build", &Arith::build);
  LuaEnvironment env;
  env["Arith"] = arith;
  REQUIRE(env["Arith"].exists());
  REQUIRE(env["Arith"].getType() == LuaType::Table);
  env.execute(CODE);
  REQUIRE(env["a"].exists());
  REQUIRE(env["a"].getType() == LuaType::UserData);
  REQUIRE(env["result"][1].get<int>() == 60);
  REQUIRE(env["result"][2].get<int>() == 30);
}

TEST_CASE("Object opaque binding") {
  const std::string &CODE = "result = { arith:add(50), arith:sub(100) }";
  LuaEnvironment env;
  LuaCppClass<Arith> arithCl("Arith");
  arithCl.bind("add", &Arith::add);
  arithCl.bind("sub", &Arith::sub);
  arithCl.bind("set", &Arith::set);
  arithCl.initializer("build", &Arith::build);
  env.getClassRegistry().bind(arithCl);
  Arith arith(10);
  env["arith"] = arith;
  REQUIRE(env["arith"].exists());
  REQUIRE(env["arith"].getType() == LuaType::UserData);
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["result"][1].get<int>() == 60);
  REQUIRE(env["result"][2].get<int>() == -90);
}