#include "catch.hpp"
#include "luacppb/Core/State.h"
#include "luacppb/Reference/Reference.h"
#include "luacppb/Invoke/Native.h"

using namespace LuaCppB;

float test_quad_fn(float x) {
  return x * x;
}

bool test_invert(bool b) {
  return !b;
}

class Factor {
 public:
  Factor(int factor) : factor(factor) {}

  float calc(float x, float y) {
    return (x + y) * factor;
  }

  float get() const {
    return this->factor;
  }

  void set(float f) {
    this->factor = f;
  }
 private:
  int factor;
};

TEST_CASE("Native function call") {
  const std::string &CODE = "results = { quad(16), invert(false) }";
  LuaEnvironment env;
  env["quad"] = test_quad_fn;
  env["invert"] = NativeFunctionCall(test_invert, env.getClassRegistry());
  REQUIRE(env["quad"].exists());
  REQUIRE(env["quad"].getType() == LuaType::Function);
  REQUIRE(env["invert"].exists());
  REQUIRE(env["invert"].getType() == LuaType::Function);
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["results"][1].get<float>() == 256.0f);
  REQUIRE(env["results"][2].get<bool>());
}

TEST_CASE("Native method call") {
  const std::string &CODE = "res1 = get()\n"
                            "res2 = calc(2, 3)\n"
                            "set(20)\n"
                            "res3 = calc(2, 3)";
  LuaEnvironment env;
  Factor factor(10);
  env["calc"] = NativeMethodCall<Factor, float, float, float>(&factor, &Factor::calc, env.getClassRegistry());
  env["get"] = NativeMethodCall<Factor, float>(&factor, &Factor::get, env.getClassRegistry());
  env["set"] = NativeMethodCall<Factor, void, float>(factor, &Factor::set, env.getClassRegistry()); 
  REQUIRE((env["calc"].exists() && env["get"].exists() && env["set"].exists()));
  REQUIRE(env["calc"].getType() == LuaType::Function);
  REQUIRE(env["get"].getType() == LuaType::Function);
  REQUIRE(env["set"].getType() == LuaType::Function);
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["res1"].get<float>() == 10.0f);
  REQUIRE(env["res2"].get<float>() == 50.0f);
  REQUIRE(env["res3"].get<float>() == 100.0f);
}

TEST_CASE("Invocable object call") {
  const std::string &CODE = "result = { factorial(5), factorial(7) }";
  LuaEnvironment env;
  env["factorial"] = NativeInvocableCall<std::function<int(int)>, int>([](int n) {
    int res = 1;
    while (n > 0) {
      res *= n--;
    }
    return res;
  }, env.getClassRegistry());
  REQUIRE(env["factorial"].exists());
  REQUIRE(env["factorial"].getType() == LuaType::Function);
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["result"][1].get<int>() == 120);
  REQUIRE(env["result"][2].get<int>() == 5040);
}

TEST_CASE("Lua function call") {
  const std::string &CODE = "function add1000(x)\n"
                            "    return x + 1000\n"
                            "end\n";
  LuaEnvironment env;
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  float res = env["add1000"](3);
  REQUIRE(res == 1003);
}