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


static void lua_table_get(benchmark::State &state) {
    lua_State *L = luaL_newstate();
    lua_newtable(L);
    lua_pushinteger(L, 42);
    lua_setfield(L, -2, "field");
    lua_setglobal(L, "tbl");
    volatile int value;
    for (auto _ : state) {
        lua_getglobal(L, "tbl");
        lua_getfield(L, -1, "field");
        value = lua_tointeger(L, -1);
        lua_pop(L, 2);
    }
    lua_close(L);
}

class LuaTestClass {
    public:
    LuaTestClass(int x) : value(x) {}

    int sum(int x) const {
    return x + this->value;
    }

    void setValue(int x) {
        this->value = x;
    }
    private:
    int value;
};

static int lua_testobj_sum(lua_State *L) {
    LuaTestClass *obj = reinterpret_cast<LuaTestClass *>(lua_touserdata(L, 1));
    int x = lua_tointeger(L, 2);
    lua_pushinteger(L, obj->sum(x));
    return 1;
}

static int lua_testobj_set(lua_State *L) {
    LuaTestClass *obj = reinterpret_cast<LuaTestClass *>(lua_touserdata(L, 1));
    int x = lua_tointeger(L, 2);
    obj->setValue(x);
    return 0;
}

static void lua_object_binding(benchmark::State &state) {
    lua_State *L = luaL_newstate();
    LuaTestClass obj(10);
    lua_pushlightuserdata(L, &obj);
    lua_newtable(L);
    lua_newtable(L);
    lua_pushcfunction(L, lua_testobj_sum);
    lua_setfield(L, -2, "sum");
    lua_pushcfunction(L, lua_testobj_set);
    lua_setfield(L, -2, "set");
    lua_setfield(L, -2, "__index");
    lua_setmetatable(L, -2);
    lua_setglobal(L, "obj");
    const char *CODE = "obj:set(obj:sum(1))";
    for (auto _ : state) {
      luaL_dostring(L, CODE);
    }
    lua_close(L);
}

struct temp {
    int value;
};

static int lua_udata_add(lua_State *L) {
    temp *i = reinterpret_cast<temp *>(lua_touserdata(L, 1));
    int x = lua_tointeger(L, 2);
    i->value += x;
    lua_pushinteger(L, i->value);
    return 1;
}

static int lua_void(lua_State *state) {
    return 0;
}

static void lua_userdata_binding(benchmark::State &state) {
    lua_State *L = luaL_newstate();
    temp *i = reinterpret_cast<temp *>(lua_newuserdata(L, sizeof(temp)));
    i->value = 10;
    lua_newtable(L);
    lua_pushcfunction(L, lua_udata_add);
    lua_setfield(L, -2, "__add");
    lua_setmetatable(L, -2);
    lua_setglobal(L, "i");
    lua_pushcfunction(L, lua_void);
    lua_setglobal(L, "void");
    const char *CODE = "void(i + 1)";
    for (auto _ : state) {
      luaL_dostring(L, CODE);
    }
    lua_close(L);
}

BENCHMARK(lua_state_initialisation);
BENCHMARK(lua_variable_assignment);
BENCHMARK(lua_variable_access);
BENCHMARK(lua_c_function_call);
BENCHMARK(lua_lua_function_call);
BENCHMARK(lua_table_get);
BENCHMARK(lua_object_binding);
BENCHMARK(lua_userdata_binding);