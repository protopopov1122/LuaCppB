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
    LuaThread thread = env["cfn"];
    REQUIRE(thread.status() == LuaStatusCode::Ok);
    test_coro(coro);
  }
  SECTION("Copying coroutine") {
    REQUIRE(env.execute(BASE) == LuaStatusCode::Ok);
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    LuaCoroutine coroOrig = env["cfn"];
    LuaCoroutine coro(coroOrig);
    REQUIRE(coro.getStatus() == LuaStatusCode::Ok);
  }
  SECTION("Building coroutine from LuaThread") {
    REQUIRE(env.execute(BASE) == LuaStatusCode::Ok);
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    LuaThread thread = env["cfn"];
    LuaCoroutine coro(thread, env);
    REQUIRE(coro.getStatus() == LuaStatusCode::Ok);
  }
  SECTION("Implicit coroutine invocation") {
    REQUIRE(env.execute(BASE) == LuaStatusCode::Ok);
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    auto coro = env["cfn"];
    test_coro(coro);
  }
  SECTION("Dead coroutine invocation") {
    REQUIRE(env.execute(BASE) == LuaStatusCode::Ok);
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    auto coro = env["cfn"];
    coro(100);
    coro(50);
    coro(300);
    int val = coro(0);
    REQUIRE(val == 450);
    REQUIRE(coro(100).hasError());
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

void test_cont_coro(LuaState env, int val) {
  LuaContinuation(env["coro"], env).call([val](LuaState env) {
    LuaContinuation(env["coro"], env).call([val](int res) mutable {
      REQUIRE(res == val * 2);
    }, [](auto) {}, val);
  }, [](auto) {}, val);
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
  SECTION("Resuming coroutine from continuation") {
    const std::string &CODE = "coro = coroutine.create(function(x)\n"
                              "    y = coroutine.yield()\n"
                              "    return x + y\n"
                              "end)\n"
                              "c2 = coroutine.create(test)\n"
                              "coroutine.resume(c2, 100)";
    env["test"] = test_cont_coro;
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  }
}

void test_yield(LuaState env, int x) {
  LuaContinuation::yield(env, [x](LuaState env, int y) {
    LuaContinuation::yield(env, [x, y](int z) {
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
