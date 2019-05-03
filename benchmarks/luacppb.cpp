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

#include <benchmark/benchmark.h>
#include "luacppb/LuaCppB.h"

using namespace LuaCppB;

static void luacppb_state_initialisation(benchmark::State &state) {
  for (auto _ : state) {
    LuaEnvironment env(false);
    env.close();
  }
}

static void luacppb_variable_assignment(benchmark::State &state) {
  LuaEnvironment env(false);
  const int value = 42;
  for (auto _ : state) {
    env["variable"] = value; 
  }
}

static void luacppb_variable_access(benchmark::State &state) {
  LuaEnvironment env(false);
  env["variable"] = 42;
  volatile int value;
  for (auto _ : state) {
    value = env["variable"];
  }
}

static int c_function(int x, int y) {
    return x + y;
}

static void luacppb_function_call(benchmark::State &state) {
  LuaEnvironment env(false);
  env["mult"] = c_function;
  for (auto _ : state) {
      env.execute("mult(4, 5)");
  }
}

static void luacppb_lua_function_call(benchmark::State &state) {
  LuaEnvironment env(false);
  env.execute("function mult(x, y)\n    return x * y\nend");
  volatile int value;
  for (auto _ : state) {
      value = env["mult"](4, 5);
  }
}

static void luacppb_table_get(benchmark::State &state) {
  LuaEnvironment env(false);
  env["tbl"] = *LuaFactory::newTable(env, "field", 5);
  auto tbl = env["tbl"];
  volatile int value;
  for (auto _ : state) {
      value = tbl["field"].get<int>();
  }
}

static void luacppb_object_binding(benchmark::State &state) {
  class TestClass {
    public:
    TestClass(int x) : value(x) {}

    int sum(int x) const {
      return x + this->value;
    }

    void setValue(int x) {
      this->value = x;
    }
    private:
    int value;
  };
  LuaEnvironment env(false);
  TestClass obj(10);
  ClassBinder<TestClass>::bind(env, "sum", &TestClass::sum, "set", &TestClass::setValue);
  env["obj"] = obj;
  const std::string &CODE = "obj:set(obj:sum(1))";
  for (auto _ : state) {
    env.execute(CODE);
  }
}

struct temp {
  temp(int v) : value(v) {}

  int value;
};

static void luacppb_userdata_binding(benchmark::State &state) {
  LuaEnvironment env(false);
  env["void"] = [&]() {};
  CustomUserDataClass<temp> ud(env);
  ud.setDefaultConstructor([](temp &arr) {
    new(&arr) temp(100);
  });
  ud.bind(LuaMetamethod::GC, [](temp &arr) {
    arr.~temp();
    ::operator delete(&arr, &arr);
  });
  ud.bind(LuaMetamethod::Add, [](temp &arr, int idx) {
    arr.value += idx;
    return arr.value;
  });

  env["i"] = ud.create(env);
  for (auto _ : state) {
    env.execute("void(i + 1)");
  }
}

static void luacppb_stateful_functions(benchmark::State &state) {
  LuaEnvironment env(false);
  constexpr float C = 3.14f;
  std::function<float(float, float)> fn = [&](float x, float y) {
    return (x + y) * C;
  };
  env["fn"] = fn;
  env["a"] = 1;
  for (auto _ : state) {
    env.execute("a = fn(a, 10)");
  }
}

BENCHMARK(luacppb_state_initialisation);
BENCHMARK(luacppb_variable_assignment);
BENCHMARK(luacppb_variable_access);
BENCHMARK(luacppb_function_call);
BENCHMARK(luacppb_lua_function_call);
BENCHMARK(luacppb_table_get);
BENCHMARK(luacppb_object_binding);
BENCHMARK(luacppb_userdata_binding);
BENCHMARK(luacppb_stateful_functions);