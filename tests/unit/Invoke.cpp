#include "catch.hpp"
#include "luacppb/Core/State.h"
#include "luacppb/Reference/Primary.h"
#include "luacppb/Reference/Field.h"
#include "luacppb/Reference/Handle.h"
#include "luacppb/Invoke/Callable.h"
#include "luacppb/Invoke/Continuation.h"
#include "luacppb/Object/Object.h"

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

float test_factor(Factor *factor, float n) {
  return factor->calc(n, n);
}

float test_factor_ref(const Factor &factor, float n) {
  return factor.calc(n, n);
}

std::pair<int, int> test_calc(int n) {
  return std::make_pair(n, n*n);
}

std::tuple<int, int, int> test_calc_tuple(int n) {
  return std::make_tuple(n, n*n, n*n*n);
}

TEST_CASE("Native function call") {
  LuaEnvironment env;
  SECTION("Function call") {
    const std::string &CODE = "results = { quad(16), invert(false) }";
    env["quad"] = test_quad_fn;
    env["invert"] = test_invert;
    REQUIRE(env["quad"].exists());
    REQUIRE(env["quad"].getType() == LuaType::Function);
    REQUIRE(env["invert"].exists());
    REQUIRE(env["invert"].getType() == LuaType::Function);
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["results"][1].get<float>() == 256.0f);
    REQUIRE(env["results"][2].get<bool>());
  }
  SECTION("Passing C++ object as parameters") {
    LuaCppClass<Factor> factorCl("Factor", env);
    env.getClassRegistry().bind(factorCl);
    Factor factor(10);
    env["factor"] = factor;
    env["callFactor"] = NativeCallable(test_factor, env);
    env["callFactorRef"] = test_factor_ref;
    const std::string &CODE = "results = { callFactor(factor, 10), callFactorRef(factor, 20) }";
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["results"][1].get<float>() == 200.0f);
    REQUIRE(env["results"][2].get<float>() == 400.0f);
  }
  SECTION("Returning std::pair") {
    const std::string &CODE = "x, y = calc(5)\n"
                              "sum = x + y";
    env["calc"] = test_calc;
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["sum"].get<int>() == 30);
  }
  SECTION("Returning std::tuple") {
    const std::string &CODE = "x, y, z = calc(5)\n"
                              "sum = x + y - z";
    env["calc"] = test_calc_tuple;
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["sum"].get<int>() == -95);
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

int test_callable_pass(int x) {
  return x * 2;
}

TEST_CASE("Passible callable as parameter") {
  const std::string &CODE = "function fn(x, y, z)\n"
                            "    return x(100) - y(100) + z()\n"
                            "end";
  Factor fact(50);
  LuaEnvironment env;
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  int res = env["fn"]([](int n) { return n * 10; }, test_callable_pass, NativeCallable(fact, &Factor::get, env));
  REQUIRE(res == 850);
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

#ifdef LUACPPB_COROUTINE_SUPPORT

template <typename T>
void test_coro(T &coro) {
  LuaStatusCode status = LuaStatusCode::Ok;
  int r1 = coro(100).status(status);
  REQUIRE(status == LuaStatusCode::Yield);
  REQUIRE(r1 == 100);
  int r2 = coro(200);
  REQUIRE(r2 == 300);
  int res = coro(0);
  REQUIRE(res == 300);
}

TEST_CASE("Coroutines") {
  const std::string &BASE = "fn = function(a)\n"
                            "    sum = a\n"
                            "    while a ~= 0 do\n"
                            "        a = coroutine.yield(sum)\n"
                            "        sum = sum + a\n"
                            "    end\n"
                            "    return sum\n"
                            "end";
  const std::string &CODE = "cfn = coroutine.create(fn)";
  LuaEnvironment env;
  SECTION("Creating coroutine") {
    REQUIRE(env.execute(BASE) == LuaStatusCode::Ok);
    LuaCoroutine coro = env["fn"];
    REQUIRE(coro.getStatus() == LuaStatusCode::Ok);
    test_coro(coro);
  }
  SECTION("Explicit coroutine invocation") {
    REQUIRE(env.execute(BASE) == LuaStatusCode::Ok);
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    LuaCoroutine coro = env["cfn"];
    REQUIRE(coro.getStatus() == LuaStatusCode::Ok);
    test_coro(coro);
  }
  SECTION("Implicit coroutine invocation") {
    REQUIRE(env.execute(BASE) == LuaStatusCode::Ok);
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    auto coro = env["cfn"];
    test_coro(coro);
  }
}

#define SHOULD_NOT_HAPPEN REQUIRE(false)

void test_cont(LuaState env, int val) {
  LuaContinuation(env["fn"], env).call([](int i) {
    REQUIRE(i == 120);
    return i * 2;
  }, [](auto) {
    SHOULD_NOT_HAPPEN;
  }, val);
}

void test_cont_error(LuaState env, int val) {
  LuaContinuation(env["fn"], env).call([](int i) {
    SHOULD_NOT_HAPPEN;
  }, [](auto err) {
    REQUIRE(err.hasError());
  }, val);
}

TEST_CASE("Continuations") {
  LuaEnvironment env;
  SECTION("Successful continuation") {
    const std::string &CODE = "function fn(x)\n"
                              "    a = coroutine.yield()\n"
                              "    return x + a\n"
                              "end\n"
                              "coro = coroutine.create(test)\n"
                              "coroutine.resume(coro, 100)\n"
                              "fin, res = coroutine.resume(coro, 20)";
    env["test"] = test_cont;
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["res"].get<int>() == 240);
  }
  SECTION("Error in continuation") {
    const std::string &CODE = "coro = coroutine.create(test)\n"
                              "coroutine.resume(coro, 100)\n"
                              "fin, res = coroutine.resume(coro, 20)";
    env["test"] = test_cont_error;
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  }
}

void test_yield(LuaState env, int x) {
  LuaContinuation::yield(env.getState(), env, [x](LuaState env, int y) {
    LuaContinuation::yield(env.getState(), env, [x, y](int z) {
      return x + y + z;
    }, [](auto) {
      SHOULD_NOT_HAPPEN;
    }, x + y);
  }, [](auto) {
    SHOULD_NOT_HAPPEN;
  }, x);
}

TEST_CASE("Yielding") {
  const std::string &CODE = "co = coroutine.wrap(fn)\n"
                            "r1 = co(10)\n"
                            "r2 = co(15)\n"
                            "res = co(5)";
  LuaEnvironment env;
  env["fn"] = test_yield;
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["r1"].get<int>() == 10);
  REQUIRE(env["r2"].get<int>() == 25);
  REQUIRE(env["res"].get<int>() == 30);
}

#endif

TEST_CASE("Bypassing function call result") {
  const std::string &CODE = "function fn(x)\n"
                            "    return x, x*2\n"
                            "end\n"
                            "r1, r2 = fn(2)";
  LuaEnvironment env;
  env["fn"] = [](int val, LuaState state) { return state[2](val); };
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["r1"].get<int>() == 2);
  REQUIRE(env["r2"].get<int>() == 4);
}