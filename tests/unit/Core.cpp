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
    #ifdef LUACPPB_GC_ISRUNNING
    REQUIRE_FALSE(gc.isRunning());
    #endif
    REQUIRE(gc.count() == 0);
    REQUIRE(gc.countBytes() == 0);
    REQUIRE_NOTHROW(gc.stop());
    #ifdef LUACPPB_GC_ISRUNNING
    REQUIRE_FALSE(gc.isRunning());
    #endif
    REQUIRE_NOTHROW(gc.restart());
    #ifdef LUACPPB_GC_ISRUNNING
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
    #ifdef LUACPPB_GC_ISRUNNING
    REQUIRE(gc.isRunning());
    #endif
    REQUIRE_NOTHROW(gc.stop());
    #ifdef LUACPPB_GC_ISRUNNING
    REQUIRE_FALSE(gc.isRunning());
    #endif
    REQUIRE_NOTHROW(gc.restart());
    #ifdef LUACPPB_GC_ISRUNNING
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

#ifndef LUACPPB_NO_CUSTOM_ALLOCATOR

class TestLuaAllocator : public LuaAllocator {
 public:
  TestLuaAllocator() : current(0), max(0) {}

  void *allocate(LuaType type, std::size_t size) override {
    void *ptr = ::operator new(size);
    this->current += size;
    this->updateMax();
    return ptr;
  }

  void *reallocate(void *ptr, std::size_t osize, std::size_t nsize) override {
    void *nptr = ::operator new(nsize);
    std::memmove(nptr, ptr, std::min(osize, nsize));
    this->current += nsize - osize;
    ::operator delete(ptr);
    this->updateMax();
    return nptr;
  }
  
  void deallocate(void *ptr, std::size_t size) override {
    ::operator delete(ptr);
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
  const std::string &CODE = "tbl = {}\n"
                            "for i = 1,100 do\n"
                            "    tbl[i] = 'Hello, ' .. i\n"
                            "end";
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(alloc->getMaxAllocation() > 0);
}

#endif