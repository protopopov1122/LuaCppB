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
#include <fstream>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <iostream>
#include "luacppb/LuaCppB.h"

using namespace LuaCppB;

#ifdef LUACPPB_ERROR_SUPPORT
void test_err(LuaState state) {
  LuaThrow(state, "Error");
}
#endif

TEST_CASE("State") {
  LuaEnvironment env;
  SECTION("Basic operations") {
    REQUIRE(env.execute("test()") != LuaStatusCode::Ok);
    REQUIRE(env("2+2*2").get<int>() == 6);
    REQUIRE_FALSE(env("2+2*2").hasError());
    REQUIRE_FALSE(env("return 2+2*2", false).hasError());
    REQUIRE(env("2+2*2", false).hasError());
    REQUIRE(env("return 2+2*2", true).hasError());
    REQUIRE(env.execute("test()").hasError());
    LuaError err;
    env.execute("x=2+2*2").error(err);
    REQUIRE(err == LuaStatusCode::Ok);
    REQUIRE_FALSE(err != LuaStatusCode::Ok);
  }
#ifdef LUACPPB_ERROR_SUPPORT
  SECTION("Error handling") {
    env["test"] = test_err;
    LuaError err;
    env("test()").error(err);
    REQUIRE(err.getError().get<std::string>().compare("Error") == 0);
  }
#endif
}

TEST_CASE("State moving") {
  LuaEnvironment env;
  env["a"] = 2;
  REQUIRE(env.isValid());
  LuaEnvironment env2(std::move(env));
  REQUIRE_FALSE(env.isValid());
  REQUIRE(env2.isValid());
  REQUIRE_FALSE(env["a"].exists());
  REQUIRE(env2["a"].get<int>() == 2);
  REQUIRE(env("a+a*a").hasError());
  REQUIRE(env2("a+a*a").get<int>() == 6);
  env = std::move(env2);
  REQUIRE(env.isValid());
  REQUIRE_FALSE(env2.isValid());
  REQUIRE(env.execute("b = a+a*a") == LuaStatusCode::Ok);
  REQUIRE(env["b"].get<int>() == 6);
}

TEST_CASE("Stack guard") {
  LuaEnvironment env;
  lua_State *state = env.getState();
  Internal::LuaStackGuard guard(state);
  SECTION("Basic operations") {
    lua_pushinteger(state, 1);
    REQUIRE(lua_gettop(state) == guard.size());
    REQUIRE(guard.checkIndex(1));
    REQUIRE_FALSE(guard.checkIndex(2));
    REQUIRE_NOTHROW(guard.assumeIndex(1));
    REQUIRE_THROWS(guard.assumeIndex(2));
    REQUIRE(guard.checkCapacity(1));
    REQUIRE_NOTHROW(guard.assumeCapacity(1));
    REQUIRE_FALSE(guard.checkCapacity(INT_MAX));
    REQUIRE_THROWS(guard.assumeCapacity(INT_MAX));
  }
  SECTION("Stack canary") {
    auto canary = guard.canary();
    REQUIRE(canary.check());
    REQUIRE_NOTHROW(canary.assume());
    lua_pushinteger(state, 1);
    REQUIRE(canary.check(1));
    REQUIRE_NOTHROW(canary.assume(1));
    REQUIRE_FALSE(canary.check());
    REQUIRE_THROWS(canary.assume());
    auto canary2 = guard.canary();
    lua_pop(state, 1);
    REQUIRE(canary2.check(-1));
    REQUIRE_NOTHROW(canary2.assume(-1));
    REQUIRE_FALSE(canary2.check());
    REQUIRE_THROWS(canary2.assume());
  }
}

int stack_test_fn(lua_State *stack) {
  return 0;
}

TEST_CASE("Stack") {
  LuaEnvironment env;
  Internal::LuaStack stack(env.getState());

  REQUIRE(stack.getTop() == 0);
  SECTION("Incorrect constructor") {
    REQUIRE_THROWS(Internal::LuaStack(nullptr));
  }
  SECTION("Value pushing") {
    SECTION("Nil") {
      stack.push();
      REQUIRE(stack.getType() == LuaType::Nil);
    }
    SECTION("Integer") {
      stack.push(100);
      REQUIRE(stack.getTop() == 1);
      REQUIRE(stack.getType() == LuaType::Number);
      REQUIRE(stack.toInteger() == 100);
      auto value = stack.get();
      REQUIRE(value.has_value());
      REQUIRE(value.value().getType() == stack.getType());
      REQUIRE(value.value().get<int>() == 100);
    }
    SECTION("Float") {
      stack.push(3.14);
      REQUIRE(stack.getTop() == 1);
      REQUIRE(stack.getType() == LuaType::Number);
      REQUIRE(stack.toNumber() == 3.14);
      auto value = stack.get();
      REQUIRE(value.has_value());
      REQUIRE(value.value().getType() == stack.getType());
      REQUIRE(value.value().get<double>() == 3.14);
    }
    SECTION("Boolean") {
      stack.push(true);
      REQUIRE(stack.getTop() == 1);
      REQUIRE(stack.getType() == LuaType::Boolean);
      REQUIRE(stack.toBoolean());
      auto value = stack.get();
      REQUIRE(value.has_value());
      REQUIRE(value.value().getType() == stack.getType());
      REQUIRE(value.value().get<bool>());
    }
    SECTION("String") {
      const std::string &STR = "Hello, world!";
      stack.push(STR);
      REQUIRE(stack.getTop() == 1);
      REQUIRE(stack.getType() == LuaType::String);
      REQUIRE(STR.compare(stack.toString()) == 0);
      auto value = stack.get();
      REQUIRE(value.has_value());
      REQUIRE(value.value().getType() == stack.getType());
      REQUIRE(STR.compare(value.value().get<std::string>()) == 0);
      REQUIRE(STR.compare(value.value().get<const std::string &>()) == 0);
    }
    SECTION("Function") {
      stack.push(stack_test_fn);
      REQUIRE(stack.getTop() == 1);
      REQUIRE(stack.getType() == LuaType::Function);
      REQUIRE(stack.toCFunction() == stack_test_fn);
      auto value = stack.get();
      REQUIRE(value.has_value());
      REQUIRE(value.value().getType() == stack.getType());
      REQUIRE(value.value().get<LuaCFunction>() == stack_test_fn);
    }
    SECTION("Closure") {
      stack.push(100);
      stack.push(stack_test_fn, 1);
      REQUIRE(stack.getTop() == 1);
      REQUIRE(stack.getType() == LuaType::Function);
      REQUIRE(stack.toCFunction() == stack_test_fn);
      auto value = stack.get();
      REQUIRE(value.has_value());
      REQUIRE(value.value().getType() == stack.getType());
      REQUIRE(value.value().get<LuaCFunction>() == stack_test_fn);
    }
    SECTION("Table") {
      stack.pushTable();
      REQUIRE(stack.getTop() == 1);
      REQUIRE(stack.getType() == LuaType::Table);
      auto value = stack.get();
      REQUIRE(value.has_value());
      REQUIRE(value.value().getType() == stack.getType());
    }
    SECTION("Light user data") {
      int i = 100;
      stack.push(&i);
      REQUIRE(stack.getTop() == 1);
      REQUIRE(stack.getType() == LuaType::LightUserData);
      REQUIRE(stack.toPointer<int *>() == &i);
    }
    SECTION("User data") {
      int *i = stack.push<int>();
      REQUIRE(i != nullptr);
      REQUIRE(stack.getTop() == 1);
      REQUIRE(stack.getType() == LuaType::UserData);
      REQUIRE(stack.toUserData<int *>() == i);
    }
  }
  SECTION("Stack operations") {
    stack.push(100);
    stack.push(3.14);
    stack.push(true);
    REQUIRE(stack.getTop() == 3);
    stack.copy(-3);
    REQUIRE(stack.getTop() == 4);
    REQUIRE(stack.getType() == LuaType::Number);
    stack.pop();
    REQUIRE(stack.getTop() == 3);
    stack.pop(3);
    REQUIRE(stack.getTop() == 0);
  }
  SECTION("Field access") {
    stack.pushTable();
    REQUIRE(stack.getTop() == 1);
    
    stack.push(100);
    stack.setField(-2, "test1");
    stack.getField(-1, "test1");
    REQUIRE(stack.toInteger(-1) == 100);
    stack.pop();

    stack.push(200);
    stack.setIndex(-2, 1);
    stack.getIndex(-1, 1);
    REQUIRE(stack.toInteger(-1) == 200);
    stack.pop();

    stack.push(300);
    stack.setIndex<true>(-2, 2);
    stack.getIndex<true>(-1, 2);
    REQUIRE(stack.toInteger(-1) == 300);
    stack.pop();

    stack.push(std::string("test2"));
    stack.push(400);
    stack.setField(-3);
    stack.push(std::string("test2"));
    stack.getField(-2);
    REQUIRE(stack.toInteger(-1) == 400);
    stack.pop();

    stack.push(std::string("test2"));
    stack.push(500);
    stack.setField<true>(-3);
    stack.push(std::string("test2"));
    stack.getField<true>(-2);
    REQUIRE(stack.toInteger(-1) == 500);
    stack.pop();

    REQUIRE(stack.getTop() == 1);
  }
}

TEST_CASE("Loading from file") {
  const std::string &CODE = "function fn(x)\n"
                            "   return x * x\n"
                            "end";
  const std::string &filename = "__test__.lua";
  std::ofstream os(filename);
  os << CODE << std::endl;
  os.flush();
  REQUIRE(os.good());
  os.close();

  LuaEnvironment env;
  LuaError err;
  REQUIRE(env.load(filename) == LuaStatusCode::Ok);
  REQUIRE(env["fn"](5).get<int>() == 25);
  REQUIRE(remove(filename.c_str()) == 0);
}

TEST_CASE("Registry handles") {
  LuaEnvironment env;
  Internal::LuaStack stack(env.getState());
  SECTION("Unique handle") {
    Internal::LuaUniqueRegistryHandle empty;
    REQUIRE_FALSE(empty.hasValue());
    REQUIRE_FALSE(Internal::LuaUniqueRegistryHandle(empty).hasValue());
    REQUIRE_FALSE(Internal::LuaUniqueRegistryHandle(std::move(empty)).hasValue());
    stack.push(100);
    Internal::LuaUniqueRegistryHandle handle(env.getState(), -1);
    REQUIRE(handle.hasValue());
    REQUIRE(Internal::LuaUniqueRegistryHandle(handle).hasValue());
    handle.get([](lua_State *state) {
      REQUIRE(lua_tointeger(state, -1) == 100);
    });
    handle.set([](lua_State *state) {
      lua_pushinteger(state, 200);
    });
    handle.get([](lua_State *state) {
      REQUIRE(lua_tointeger(state, -1) == 200);
    });
  }
  SECTION("Shared registry handle") {
    Internal::LuaSharedRegistryHandle empty;
    REQUIRE_FALSE(empty.hasValue());
    REQUIRE_FALSE(Internal::LuaSharedRegistryHandle(empty).hasValue());
    REQUIRE_FALSE(empty.get([](lua_State *state) {
      REQUIRE(false);
    }));
    REQUIRE_FALSE(empty.set([](lua_State *state) {
      REQUIRE(false);
    }));
    stack.push(100);
    Internal::LuaSharedRegistryHandle handle(env.getState(), -1);
    REQUIRE(handle.hasValue());
    Internal::LuaUniqueRegistryHandle uniqHandle(env.getState(), -1);
    REQUIRE(Internal::LuaSharedRegistryHandle(uniqHandle).hasValue());
    REQUIRE(handle.get([](lua_State *state) {
      REQUIRE(lua_tointeger(state, -1) == 100);
    }));
    REQUIRE(handle.set([](lua_State *state) {
      lua_pushinteger(state, 200);
    }));
    REQUIRE(handle.get([](lua_State *state) {
      REQUIRE(lua_tointeger(state, -1) == 200);
    }));
  }
}

TEST_CASE("Exception") {
  try {
    throw LuaCppBError("Test", LuaCppBErrorCode::StackOverflow);
    REQUIRE(false);
  } catch (LuaCppBError &ex) {
    REQUIRE(std::string("Test").compare(ex.what()) == 0);
    REQUIRE(ex.getErrorCode() == LuaCppBErrorCode::StackOverflow);
  }
}

TEST_CASE("Garbage collector") {
  SECTION("Empty GC") {
    Internal::LuaGC gc;
    REQUIRE_FALSE(gc.valid());
    #ifdef LUACPPB_GC_ISRUNNING_SUPPORT
    REQUIRE_FALSE(gc.isRunning());
    #endif
    REQUIRE(gc.count() == 0);
    REQUIRE(gc.countBytes() == 0);
    REQUIRE_NOTHROW(gc.stop());
    #ifdef LUACPPB_GC_ISRUNNING_SUPPORT
    REQUIRE_FALSE(gc.isRunning());
    #endif
    REQUIRE_NOTHROW(gc.restart());
    #ifdef LUACPPB_GC_ISRUNNING_SUPPORT
    REQUIRE_FALSE(gc.isRunning());
    #endif
    REQUIRE_NOTHROW(gc.collect());
    REQUIRE_FALSE(gc.step(0));
    REQUIRE_FALSE(gc.step(100));
    REQUIRE(gc.setPause(100) == 0);
    REQUIRE(gc.setPause(0) == 0);
    REQUIRE(gc.setStepMultiplier(100) == 0);
    REQUIRE(gc.setStepMultiplier(0) == 0);
  }
  SECTION("Valid GC") {
    LuaEnvironment env;
    Internal::LuaGC gc(env.getState());
    REQUIRE(gc.valid());
    #ifdef LUACPPB_GC_ISRUNNING_SUPPORT
    REQUIRE(gc.isRunning());
    #endif
    REQUIRE_NOTHROW(gc.stop());
    #ifdef LUACPPB_GC_ISRUNNING_SUPPORT
    REQUIRE_FALSE(gc.isRunning());
    #endif
    REQUIRE_NOTHROW(gc.restart());
    #ifdef LUACPPB_GC_ISRUNNING_SUPPORT
    REQUIRE(gc.isRunning());
    #endif
    REQUIRE_NOTHROW(gc.step(0));

    int pause = gc.setPause(100);
    REQUIRE(gc.setPause(pause) == 100);
    REQUIRE(gc.setPause(pause) == pause);
    int step = gc.setStepMultiplier(100);
    REQUIRE(gc.setStepMultiplier(step) == 100);
    REQUIRE(gc.setStepMultiplier(step) == step);
  }
}

#ifdef LUACPPB_CUSTOM_ALLOCATOR_SUPPORT

class TestLuaAllocator : public LuaAllocator {
 public:
  TestLuaAllocator() : current(0), max(0) {}

  void *allocate(LuaType type, std::size_t size) override {
    void *ptr = malloc(size);
    this->current += size;
    this->updateMax();
    return ptr;
  }

  void *reallocate(void *ptr, std::size_t osize, std::size_t nsize) override {
    void *nptr = realloc(ptr, nsize);
    this->current += nsize - osize;
    this->updateMax();
    return nptr;
  }
  
  void deallocate(void *ptr, std::size_t size) override {
    free(ptr);
    this->current -= size;
  }

  std::size_t getMaxAllocation() const {
    return this->max;
  }
 private:
  void updateMax() {
    if (this->current > this->max) {
      this->max = this->current;
    }
  }
  std::size_t current;
  std::size_t max;
};

TEST_CASE("Allocator") {
  LuaEnvironment env;
  auto alloc = std::make_shared<TestLuaAllocator>();
  env.setCustomAllocator(alloc);
  REQUIRE(alloc->getMaxAllocation() == 0);
  const std::string &CODE = "tbl = {}\n"
                            "for i = 1,100 do\n"
                            "    tbl[i] = 'Hello, ' .. i\n"
                            "end";
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(alloc->getMaxAllocation() > 0);
}

TEST_CASE("Constructing with allocator") {
  auto alloc = std::make_shared<TestLuaAllocator>();
  LuaEnvironment env(alloc);
  REQUIRE(alloc->getMaxAllocation() > 0);
  const std::string &CODE = "tbl = {}\n"
                            "for i = 1,100 do\n"
                            "    tbl[i] = 'Hello, ' .. i\n"
                            "end";
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(alloc->getMaxAllocation() > 0);
}

class LuaNoAllocator : public LuaAllocator {
 public:
  void *allocate(LuaType type, std::size_t size) override {
    return nullptr;
  }

  void *reallocate(void *ptr, std::size_t osize, std::size_t nsize) override {
    return nullptr;
  }
  
  void deallocate(void *ptr, std::size_t size) override {
  }
};

#endif

#if __has_include("lstate.h")
#include "lstate.h"

TEST_CASE("Panic handler") {
  const std::string &CODE = "callback()";
  LuaEnvironment env;
  bool panicked = false;
  env.setPanicHandler([&](LuaState &state)->int {
    panicked = true;
    throw std::exception();
  });
  env["callback"] = [&](LuaState state) {
    G(state.getState())->panic(state.getState());  // Poking Lua internals to cause panic
  };
  try {
    env.execute(CODE);
    REQUIRE(false);
  } catch (std::exception &ex) {
    REQUIRE(panicked);
  }
}
#endif

TEST_CASE("Global table") {
  LuaEnvironment env;
  env["a"] = 2;
  env["b"] = true;
  auto ref = env.getGlobals();
  REQUIRE(ref["a"].exists());
  REQUIRE(ref["a"].get<int>() == 2);
  REQUIRE(ref["b"].exists());
  REQUIRE(ref["b"].get<bool>());
}

TEST_CASE("Main thread getter") {
  LuaEnvironment env;
  auto thread = env.getMainThread();
  REQUIRE(thread.isValid());
  REQUIRE(thread.getState() == env.getState());
}

TEST_CASE("Lua function dump") {
  LuaEnvironment env;
  LuaLoader loader(env);
  SECTION("Empty reference") {
    LuaReferenceHandle ref;
    auto res = loader.dump(ref);
    REQUIRE_FALSE(res.has_value());
  }
  SECTION("Non-function reference") {
    env["a"] = 10;
    auto res = loader.dump(env["a"]);
    REQUIRE_FALSE(res.has_value());
  }
  SECTION("C-function reference") {
    env["fn"] = []() {};
    auto res = loader.dump(env["fn"]);
    REQUIRE_FALSE(res.has_value());
  }
  SECTION("Lua function reference") {
    const std::string &CODE = "function sum(a, b)\n"
                              "    return a + b\n"
                              "end";
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    auto res = loader.dump(env["sum"]);
    REQUIRE(res.has_value());
    LuaLoader::Image &img = res.value();
    REQUIRE_FALSE(img.empty());
  }
}

TEST_CASE("Lua function load") {
  LuaEnvironment env;
  LuaLoader loader(env);
  const std::string &CODE = "function sum(a, b)\n"
                            "    return a + b\n"
                            "end";
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  auto res = loader.dump(env["sum"]);
  REQUIRE(res.has_value());
  LuaLoader::Image &img = res.value();
  REQUIRE_FALSE(img.empty());
  auto fn = loader.load(img, "sumFunc");
  REQUIRE(fn.has_value());
  REQUIRE(fn.value().valid());
  REQUIRE(fn.value()(2, 3).get<int>() == 5);
}

TEST_CASE("Lua function source load") {
  LuaEnvironment env;
  LuaLoader loader(env);
  const std::string &CODE = "return 2+2*2";
  std::stringstream code(CODE);
  auto fun = loader.load(code, "fun");
  REQUIRE(fun.has_value());
  auto res = fun.value()();
  REQUIRE(res == LuaStatusCode::Ok);
  REQUIRE(res.get<int>() == 6);
}

TEST_CASE("Lua version getter") {
  LuaEnvironment env;
  REQUIRE(env.version() == LUA_VERSION_NUM);
}

TEST_CASE("Library loading") {
  LuaEnvironment env(false);
  SECTION("Base library") {
    const std::string &CODE = "res = tonumber(arg)";
    auto res = LuaLoadStdLibs(env, true, LuaStdLib::Base);
    if (std::get<0>(res).valid()) {
      REQUIRE(std::get<0>(res)["type"](50).get<std::string>() == "number");
    } else {
      REQUIRE(env["type"](50).get<std::string>() == "number");
    }
    env["arg"] = "100";
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["res"].get<int>() == 100);
  }
  SECTION("Package library") {
    const std::string &CODE = "loaded = package.loaded";
    auto res = LuaLoadStdLibs(env, true, LuaStdLib::Package);
    if (std::get<0>(res).valid()) {
      REQUIRE(std::get<0>(res)["loaded"].getType() == LuaType::Table);
    } else {
      REQUIRE(env["package"]["loaded"].getType() == LuaType::Table);
    }
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["loaded"].getType() == LuaType::Table);
  }
  SECTION("String library") {
    const std::string &CODE = "res = string.reverse(arg)";
    auto res = LuaLoadStdLibs(env, true, LuaStdLib::String);
    if (std::get<0>(res).valid()) {
      REQUIRE(std::get<0>(res)["reverse"]("abc").get<std::string>() == "cba");
    } else {
      REQUIRE(env["string"]["reverse"]("abc").get<std::string>() == "cba");
    }
    env["arg"] = "Hello";
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["res"].get<std::string>() == "olleH");
  }
  SECTION("Table library") {
    const std::string &CODE = "res = table.concat(arg)";
    auto tbl = LuaFactory::newTable(env);
    tbl[1] = 1;
    tbl[2] = "2";
    tbl[3] = 3;
    auto res = LuaLoadStdLibs(env, true, LuaStdLib::Table);
    if (std::get<0>(res).valid()) {
      REQUIRE(std::get<0>(res)["concat"](*tbl).get<std::string>() == "123");
    } else {
      REQUIRE(env["table"]["concat"](*tbl).get<std::string>() == "123");
    }
    env["arg"] = *tbl;
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["res"].get<std::string>() == "123");
  }
  SECTION("Math library") {
    const std::string &CODE = "res = math.abs(arg)";
    auto res = LuaLoadStdLibs(env, true, LuaStdLib::Math);
    if (std::get<0>(res).valid()) {
      REQUIRE(std::get<0>(res)["abs"](-10).get<int>() == 10);
    } else {
      REQUIRE(env["math"]["abs"](-10).get<int>() == 10);
    }
    env["arg"] = -100;
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["res"].get<int>() == 100);
  }
  SECTION("IO library") {
    const std::string &CODE = "res = io.type(io.stdin)";
    auto res = LuaLoadStdLibs(env, true, LuaStdLib::IO);
    if (std::get<0>(res).valid()) {
      REQUIRE(std::get<0>(res)["type"](*std::get<0>(res)["stdin"]).get<std::string>() == "file");
    } else {
      REQUIRE(env["io"]["type"](*env["io"]["stdin"]).get<std::string>() == "file");
    }
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["res"].get<std::string>() == "file");
  }
  SECTION("OS library") {
    const std::string &CODE = "res = os.time()";
    auto res = LuaLoadStdLibs(env, true, LuaStdLib::OS);
    if (std::get<0>(res).valid()) {
      REQUIRE(std::get<0>(res)["time"]().get<uint64_t>() > 0);
    } else {
      REQUIRE(env["os"]["time"]().get<uint64_t>() > 0);
    }
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["res"].get<uint64_t>() > 0);
  }
  SECTION("Debug library") {
    const std::string &CODE = "res = debug.getregistry()";
    auto res = LuaLoadStdLibs(env, true, LuaStdLib::Debug);
    if (std::get<0>(res).valid()) {
      auto val = std::get<0>(res)["getregistry"]().get<LuaValue>();
      REQUIRE(val.getType() == LuaType::Table);
    } else {
      auto val = env["debug"]["getregistry"]().get<LuaValue>();
      REQUIRE(val.getType() == LuaType::Table);
    }
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["res"].getType()== LuaType::Table);
  }
#ifndef LUACPPB_INTERNAL_COMPAT_501
  SECTION("Coroutine library") {
    const std::string &CODE = "res = coroutine.running()";
    auto res = LuaLoadStdLibs(env, true, LuaStdLib::Coroutine);
    if (std::get<0>(res).valid()) {
      LuaValue val = std::get<0>(res)["running"]();
      REQUIRE(val.getType() == LuaType::Thread);
    } else {
      LuaValue val = env["coroutine"]["running"]();
      REQUIRE(val.getType() == LuaType::Thread);
    }
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["res"].getType()== LuaType::Thread);
  }
#endif
#ifndef LUACPPB_INTERNAL_COMPAT_502
    SECTION("Utf8 library") {
      const std::string &CODE = "res = utf8.len(arg)";
      auto res = LuaLoadStdLibs(env, true, LuaStdLib::Utf8);
      if (std::get<0>(res).valid()) {
        REQUIRE(std::get<0>(res)["len"]("Hello").get<int>() == 5);
      } else {
        REQUIRE(env["utf8"]["len"]("Hello").get<int>() == 5);
      }
      env["arg"] = "world!";
      REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
      REQUIRE(env["res"].get<int>() == 6);
    }
#endif
}
