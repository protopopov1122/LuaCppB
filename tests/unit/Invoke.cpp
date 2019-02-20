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

float test_function_call1(float x) {
  return x * x;
}

bool test_function_call2(bool b) {
  return !b;
}

int test_function_call3(int x) {
  return x * 2;
}

class Factor {
 public:
  Factor(int factor) : factor(factor) {}

  float calc(float x, float y) const {
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

float test_object_pointer_passing(Factor *factor, float n) {
  return factor->calc(n, n);
}

float test_object_reference_passing(const Factor &factor, float n) {
  return factor.calc(n, n);
}

std::pair<int, int> test_pair_returning(int n) {
  return std::make_pair(n, n*n);
}

std::tuple<int, int, int> test_tuple_returning(int n) {
  return std::make_tuple(n, n*n, n*n*n);
}

void test_ref_prm(int factor, LuaState state, LuaReferenceHandle ref) {
  ref["c"] = factor * (ref["a"].get<int>() + ref["b"].get<int>());
}

void test_unsupported_object_wrap(LuaEnvironment *env) {}

int test_ignore_argument(int x, LuaEmpty e, int y) {
  return x * y;
}

TEST_CASE("Native function call") {
  LuaEnvironment env;
  SECTION("Function call") {
    const std::string &CODE = "results = { quad(16), invert(false) }";
    env["quad"] = test_function_call1;
    env["invert"] = test_function_call2;
    REQUIRE(env["quad"].exists());
    REQUIRE(env["quad"].getType() == LuaType::Function);
    REQUIRE(env["invert"].exists());
    REQUIRE(env["invert"].getType() == LuaType::Function);
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["results"][1].get<float>() == 256.0f);
    REQUIRE(env["results"][2].get<bool>());
  }
}

TEST_CASE("Native method call") {
  const std::string &CODE = "res1 = get()\n"
                            "res2 = calc(2, 3)\n"
                            "set(20)\n"
                            "res3 = calc(2, 3)";
  LuaEnvironment env;
  Factor factor(10);
  env["calc"] = NativeCallable(&factor, &Factor::calc, env);
  env["get"] = NativeCallable(&factor, &Factor::get, env);
  env["set"] = NativeCallable(&factor, &Factor::set, env); 
  REQUIRE((env["calc"].exists() && env["get"].exists() && env["set"].exists()));
  REQUIRE(env["calc"].getType() == LuaType::Function);
  REQUIRE(env["get"].getType() == LuaType::Function);
  REQUIRE(env["set"].getType() == LuaType::Function);
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["res1"].get<float>() == 10.0f);
  REQUIRE(env["res2"].get<float>() == 50.0f);
  REQUIRE(env["res3"].get<float>() == 100.0f);
}

TEST_CASE("Native lambda call") {
  const std::string &CODE = "set(5)\n"
                            "res = get() * 10";
  LuaEnvironment env;
  int value = 0;
  env["set"] = [&](int v) mutable {
    value = v;
  };
  env["get"] = [&] {
    return value;
  };
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["res"].get<int>() == 50);
}

TEST_CASE("Reference as function parameter") {
  const std::string &CODE = "tbl = { a = 3, b = 10 }\n"
                            "test(2, tbl)\n"
                            "res = tbl.c";
  LuaEnvironment env;
  env["test"] = test_ref_prm;
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["res"].get<int>() == 26);
}

TEST_CASE("Invocable object call") {
  const std::string &CODE = "result = { factorial(5), factorial(7) }";
  LuaEnvironment env;
  env["factorial"] = [](int n) {
    int res = 1;
    while (n > 0) {
      res *= n--;
    }
    return res;
  };
  REQUIRE(env["factorial"].exists());
  REQUIRE(env["factorial"].getType() == LuaType::Function);
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["result"][1].get<int>() == 120);
  REQUIRE(env["result"][2].get<int>() == 5040);
}

TEST_CASE("Passing C++ object as parameters") {
  LuaEnvironment env;
  LuaCppClass<Factor> factorCl("Factor", env);
  env.getClassRegistry().bind(factorCl);
  Factor factor(10);
  env["factor"] = factor;
  env["callFactor"] = NativeCallable(test_object_pointer_passing, env);
  env["callFactorRef"] = test_object_reference_passing;
  const std::string &CODE = "results = { callFactor(factor, 10), callFactorRef(factor, 20) }";
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["results"][1].get<float>() == 200.0f);
  REQUIRE(env["results"][2].get<float>() == 400.0f);
}

TEST_CASE("Passing callable as parameter") {
  const std::string &CODE = "function fn(x, y, z)\n"
                            "    return x(100) - y(100) + z()\n"
                            "end";
  Factor fact(50);
  LuaEnvironment env;
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  int res = env["fn"]([](int n) { return n * 10; }, test_function_call3, NativeCallable(fact, &Factor::get, env));
  REQUIRE(res == 850);
}

TEST_CASE("Parameter ignore") {
  const std::string &CODE = "res = test(2, 3, 4)";
  LuaEnvironment env;
  env["test"] = test_ignore_argument;
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["res"].get<int>() == 8);
}

TEST_CASE("Returning std::pair") {
  LuaEnvironment env;
  const std::string &CODE = "x, y = calc(5)\n"
                            "sum = x + y";
  env["calc"] = test_pair_returning;
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["sum"].get<int>() == 30);
}

TEST_CASE("Returning std::tuple") {
  LuaEnvironment env;
  const std::string &CODE = "x, y, z = calc(5)\n"
                            "sum = x + y - z";
  env["calc"] = test_tuple_returning;
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["sum"].get<int>() == -95);
}

#if !defined(__clang_major__)
TEST_CASE("Bypassing Lua function call result") {
  const std::string &CODE = "function fn(x)\n"
                            "    return x, x*2\n"
                            "end\n"
                            "r1, r2 = fun(2, fn)";
  LuaEnvironment env;
  env["fun"] = [](int val, LuaState state) { return state[2](val); };
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["r1"].get<int>() == 2);
  REQUIRE(env["r2"].get<int>() == 4);
}
#endif

TEST_CASE("Lambda wrapping") {
  const std::string &CODE = "function sum(x, y)\n"
                            "    return x + y\n"
                            "end\n"
                            "function mul(x, y)\n"
                            "    return x * y\n"
                            "end\n"
                            "function tostr(x)\n"
                            "    return '=' .. x\n"
                            "end";
  LuaEnvironment env;
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  std::function<int(int, int)> sum = LuaLambda(env["sum"]);
  std::function<int(int, int)> mul = LuaLambda(env["mul"]);
  std::function<std::string(int)> tostr = LuaLambda(env["tostr"]);
  REQUIRE(tostr(sum(2, mul(2, 2))).compare("=6") == 0);
}

TEST_CASE("Invocation with exception ignoring") {
  LuaEnvironment env;
  Factor factor(10);
  LuaCppClass<Factor> factorCl("Factor", env);
  env.getClassRegistry().bind(factorCl);
  env["factor"] = factor;
  env["fn"] = test_unsupported_object_wrap;
  env.setExceptionHandler([](auto &ex) {});
  REQUIRE(env["fn"](factor).hasError());
  REQUIRE(env["fn"](factor) == LuaStatusCode::RuntimeError);
}

TEST_CASE("Lua function call") {
  LuaEnvironment env;
  SECTION("Plain call") {
    const std::string &CODE = "function add1000(x)\n"
                              "    return x + 1000\n"
                              "end\n";
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    float res = env["add1000"](3);
    REQUIRE(res == 1003);
  }
  SECTION("Passing and receiving an object") {
    const std::string &CODE = "function test(factor)\n"
                              "    return factor:get()\n"
                              "end\n"
                              "function test2(factor, n)\n"
                              "    return factor\n"
                              "end\n";
    Factor factor(10);
    LuaCppClass<Factor> factorCl("Factor", env);
    factorCl.bind("get", &Factor::get);
    env.getClassRegistry().bind(factorCl);
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["test"](factor).get<float>() == 10.0f);
    REQUIRE(env["test"](&factor).get<float>() == 10.0f);
    REQUIRE(env["test"](std::make_unique<Factor>(10)).get<float>() == 10.0f);
    REQUIRE(env["test"](std::make_shared<Factor>(10)).get<float>() == 10.0f);
    Factor &resFactor = env["test2"](factor, factor).get<Factor &>();
    Factor *resFactor2 = env["test2"](factor, 10);
    REQUIRE(&resFactor == &factor);
    REQUIRE(resFactor2 == &factor);
    REQUIRE_THROWS(env["test2"](factor, 10).get<LuaEnvironment *>() != nullptr);
  }
  SECTION("Receiving std::pair") {
    const std::string &CODE = "function test(n)\n"
                              "    return 1, n\n"
                              "end";
    LuaEnvironment env;
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    std::pair<int, int> res = env["test"](10);
    REQUIRE((res.first == 1 && res.second == 10));
  }
  SECTION("Receiving std::tuple") {
    const std::string &CODE = "function test(n)\n"
                              "    return 1, n, n*n\n"
                              "end";
    LuaEnvironment env;
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    std::tuple<int, int, int> res = env["test"](5);
    REQUIRE(std::get<0>(res) == 1);
    REQUIRE(std::get<1>(res) == 5);
    REQUIRE(std::get<2>(res) == 25);
  }
}
