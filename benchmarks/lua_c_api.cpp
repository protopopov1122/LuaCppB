#include <benchmark/benchmark.h>
#include "luacppb/LuaCppB.h"

static void lua_state_initialisation(benchmark::State &state) {
    for (auto _ : state) {
        lua_State *L = luaL_newstate();
        lua_close(L);
    }
}

static void lua_variable_assignment(benchmark::State &state) {
    lua_State *L = luaL_newstate();
    const int value = 42;
    for (auto _ : state) {
        lua_pushinteger(L, value);
        lua_setglobal(L, "variable");
    }
    lua_close(L);
}

static void lua_variable_access(benchmark::State &state) {
    lua_State *L = luaL_newstate();
    lua_pushinteger(L, 42);
    lua_setglobal(L, "variable");
    volatile int value;
    for (auto _ : state) {
        lua_getglobal(L, "variable");
        value = lua_tointeger(L, -1);
        lua_pop(L, 1);
    }
    lua_close(L);
}

static int c_function(lua_State *L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    lua_pushinteger(L, x * y);
    return 1;
}

static void lua_c_function_call(benchmark::State &state) {
    lua_State *L = luaL_newstate();
    lua_pushcfunction(L, c_function);
    lua_setglobal(L, "mult");
    for (auto _ : state) {
        luaL_dostring(L, "mult(4, 5)");
    }
    lua_close(L);
}

static void lua_lua_function_call(benchmark::State &state) {
    lua_State *L = luaL_newstate();
    luaL_dostring(L, "function mult(x, y)\n    return x * y\nend");
    volatile int value;
    for (auto _ : state) {
        lua_getglobal(L, "mult");
        lua_pushinteger(L, 4);
        lua_pushinteger(L, 5);
        lua_pcall(L, 2, 1, 0);
        value = lua_tointeger(L, -1);
        lua_pop(L, 1);
    }
    lua_close(L);
}

BENCHMARK(lua_state_initialisation);
BENCHMARK(lua_variable_assignment);
BENCHMARK(lua_variable_access);
BENCHMARK(lua_c_function_call);
BENCHMARK(lua_lua_function_call);