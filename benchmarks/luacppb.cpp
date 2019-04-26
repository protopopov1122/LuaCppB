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
    env["tbl"] = LuaFactory::newTable(env, "field", 5);
    auto tbl = env["tbl"];
    volatile int value;
    for (auto _ : state) {
        value = tbl["field"];
    }
}

BENCHMARK(luacppb_state_initialisation);
BENCHMARK(luacppb_variable_assignment);
BENCHMARK(luacppb_variable_access);
BENCHMARK(luacppb_function_call);
BENCHMARK(luacppb_lua_function_call);
BENCHMARK(luacppb_table_get);
