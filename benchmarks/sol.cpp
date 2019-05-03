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
#include "sol.hpp"

static void sol_state_initialisation(benchmark::State &state) {
  for (auto _ : state) {
    sol::state env;
  }
}

static void sol_variable_assignment(benchmark::State &state) {
  sol::state env;
  const int value = 42;
  for (auto _ : state) {
    env.set("variable", value);
  }
}

static void sol_variable_access(benchmark::State &state) {
  sol::state env;
  env.set("variable", 42);
  volatile int value;
  for (auto _ : state) {
    value = env["variable"];
  }
}

static int c_function(int x, int y) {
  return x + y;
}

static void sol_function_call(benchmark::State &state) {
  sol::state env;
  env.set("mult", c_function);
  for (auto _ : state) {
    env.do_string("mult(4, 5)");
  }
}

static void sol_lua_function_call(benchmark::State &state) {
  sol::state env;
  env.do_string("function mult(x, y)\n    return x * y\nend");
  volatile int value;
  for (auto _ : state) {
    value = env["mult"](4, 5);
  }
}

static void sol_table_get(benchmark::State &state) {
  sol::state env;
  env["tbl"] = env.create_table_with(
    "field", 5
  );
  auto tbl = env["tbl"];
  volatile int value;
  for (auto _ : state) {
    value = tbl["field"];
  }
}

static void sol_object_binding(benchmark::State &state) {
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
  
  sol::state env;
  env.new_usertype<TestClass>("testClass", "sum", &TestClass::sum, "set", &TestClass::setValue);
  TestClass obj(10);
  env["obj"] = obj;
  const std::string &CODE = "obj:set(obj:sum(1))";
  for (auto _ : state) {
    env.do_string(CODE);
  }
}

struct temp {
  int add(int x) {
    this->value += x;
    return this->value;
  }

  int value;
};

static void sol_userdata_binding(benchmark::State &state) {
  sol::state env;
  env["void"] = [&]() {};
  auto type = env.new_usertype<temp>("integer", "add", &temp::add);
  env.do_string("i = integer.new()");
  for (auto _ : state) {
    env.do_string("void(i:add(1))");
  }
}

static void sol_stateful_functions(benchmark::State &state) {
  sol::state env;
  constexpr float C = 3.14f;
  std::function<float(float, float)> fn = [&](float x, float y) {
    return (x + y) * C;
  };
  env["fn"] = fn;
  env["a"] = 1;
  for (auto _ : state) {
    env.do_string("a = fn(a, 10)");
  }
}


BENCHMARK(sol_state_initialisation);
BENCHMARK(sol_variable_assignment);
BENCHMARK(sol_variable_access);
BENCHMARK(sol_function_call);
BENCHMARK(sol_lua_function_call);
BENCHMARK(sol_table_get);
BENCHMARK(sol_object_binding);
BENCHMARK(sol_userdata_binding);
BENCHMARK(sol_stateful_functions);
