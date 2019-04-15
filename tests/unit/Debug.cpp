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
#include <iostream>
#include <map>
#include <set>

using namespace LuaCppB;

TEST_CASE("Local variables") {
  const std::string &CODE = "function sum(a, b)\n"
                            "  callback()\n"
                            "  return a + b\n"
                            "end";
  LuaEnvironment env;
  std::map<std::string, int> values;
  env["callback"] = [&]() {
    auto debug = env.getDebugFrame(1);
    std::size_t i = 1;
    std::optional<LuaDebugFrame::Variable> var = debug.getLocal(i);
    for (; var.has_value(); var = debug.getLocal(++i)) {
      values[var.value().key] = var.value().value.get<int>();
    }
  };
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["sum"](2, 3) == LuaStatusCode::Ok);
  REQUIRE(values.count("a") != 0);
  REQUIRE(values.count("b") != 0);
  REQUIRE(values.count("c") == 0);
  REQUIRE(values["a"] == 2);
  REQUIRE(values["b"] == 3);
}

TEST_CASE("Local variable symbols") {
  const std::string &CODE = "function sum(a, b)\n"
                            "  callback()\n"
                            "  return a + b\n"
                            "end";
  LuaEnvironment env;
  std::set<std::string> symbols;
  env["callback"] = [&]() {
    auto debug = env.getDebugFrame(1);
    std::size_t i = 1;
    std::optional<std::string> var = debug.getLocal(env["sum"], i);
    for (; var.has_value(); var = debug.getLocal(env["sum"], ++i)) {
      symbols.emplace(var.value());
    }
  };
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["sum"](2, 3) == LuaStatusCode::Ok);
  REQUIRE(symbols.count("a") != 0);
  REQUIRE(symbols.count("b") != 0);
  REQUIRE(symbols.count("c") == 0);
}

TEST_CASE("Upvalues") {
  const std::string &CODE = "function sum(a, b)\n"
                            "  callback(function() return a + b end)\n"
                            "  return a + b\n"
                            "end";
  LuaEnvironment env;
  std::map<std::string, int> values;
  std::map<std::string, LuaDebugFrame::UpvalueId> idents;
  env["callback"] = [&](LuaReferenceHandle fn) {
    auto debug = env.getDebugFrame(1);
    std::size_t i = 1;
    std::optional<LuaDebugFrame::Variable> var = debug.getUpvalue(fn, i);
    for (; var.has_value(); var = debug.getUpvalue(fn, ++i)) {
      values[var.value().key] = var.value().value.get<int>();
      idents[var.value().key] = debug.getUpvalueId(fn, i);
    }
  };
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["sum"](2, 3) == LuaStatusCode::Ok);
  REQUIRE(values.count("a") != 0);
  REQUIRE(values.count("b") != 0);
  REQUIRE(values.count("c") == 0);
  REQUIRE(idents["a"] != nullptr);
  REQUIRE(idents["b"] != nullptr);
  REQUIRE(idents["a"] != idents["b"]);
  REQUIRE(values["a"] == 2);
  REQUIRE(values["b"] == 3);
}

TEST_CASE("Current function info") {
  const std::string &CODE = "function sum(a, b)\n"
                            "  callback(function() return a + b end)\n"
                            "  return a + b\n"
                            "end";
  LuaEnvironment env;
  std::map<std::string, int> values;
  std::map<std::string, LuaDebugFrame::UpvalueId> idents;
  bool called = false;
  env["callback"] = [&](LuaReferenceHandle fn) {
    called = true;
    auto debug = env.getDebugFrame();
    debug.getCurrentFunctionInfo(LuaDebugFrame::Function::Name, LuaDebugFrame::Function::Line, LuaDebugFrame::Function::Source, LuaDebugFrame::Function::Params);
    REQUIRE_NOTHROW(debug.getEvent());
    REQUIRE(debug.getName().compare("callback") == 0);
    REQUIRE(debug.getNameWhat().compare("global") == 0);
    REQUIRE(debug.getWhat().compare("C") == 0);
    REQUIRE(debug.getSource().compare("=[C]") == 0);
    REQUIRE(debug.getShortSource().compare("[C]") == 0);
    REQUIRE(debug.getCurrentLine() == -1);
    REQUIRE(debug.getLineDefined() == -1);
    REQUIRE(debug.getLastLineDefined() == -1);
    REQUIRE(debug.getUpvalues() == 2);
    REQUIRE(debug.getParameters() == 0);
    REQUIRE(debug.isVarArg());
    REQUIRE_FALSE(debug.isTailCall());
  };
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["sum"](2, 3) == LuaStatusCode::Ok);
  REQUIRE(called);
}

TEST_CASE("Function info") {
  const std::string &CODE = "function sum(a, b)\n"
                            "  callback(sum)\n"
                            "  return a + b\n"
                            "end";
  LuaEnvironment env;
  std::map<std::string, int> values;
  std::map<std::string, LuaDebugFrame::UpvalueId> idents;
  bool called = false;
  env["callback"] = [&](LuaReferenceHandle fn) {
    called = true;
    auto debug = env.getDebugFrame();
    debug.getFunctionInfo(fn, LuaDebugFrame::Function::Name, LuaDebugFrame::Function::Line, LuaDebugFrame::Function::Source, LuaDebugFrame::Function::Params);
    REQUIRE_NOTHROW(debug.getEvent());
    REQUIRE_NOTHROW(debug.getName());
    REQUIRE_NOTHROW(debug.getNameWhat());
    REQUIRE(debug.getWhat().compare("Lua") == 0);
    REQUIRE(debug.getSource().compare(CODE) == 0);
    REQUIRE(debug.getShortSource().compare("[string \"function sum(a, b)...\"]") == 0);
    REQUIRE(debug.getCurrentLine() == -1);
    REQUIRE(debug.getLineDefined() == 1);
    REQUIRE(debug.getLastLineDefined() == 4);
    REQUIRE(debug.getUpvalues() == 1);
    REQUIRE(debug.getParameters() == 2);
    REQUIRE_FALSE(debug.isVarArg());
    REQUIRE_FALSE(debug.isTailCall());
  };
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["sum"](2, 3) == LuaStatusCode::Ok);
  REQUIRE(called);
}