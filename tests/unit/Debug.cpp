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
#include <map>
#include <set>

#ifdef LUACPPB_DEBUG_SUPPORT

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

#ifdef LUACPPB_DEBUG_EXTRAS_SUPPORT
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
#endif

TEST_CASE("Upvalues") {
  const std::string &CODE = "function sum(a, b)\n"
                            "  callback(function() return a + b end)\n"
                            "  return a + b\n"
                            "end";
  LuaEnvironment env;
  std::map<std::string, int> values;
#ifdef LUACPPB_DEBUG_EXTRAS_SUPPORT
  std::map<std::string, LuaDebugFrame::UpvalueId> idents;
#endif
  env["callback"] = [&](LuaReferenceHandle fn) {
    auto debug = env.getDebugFrame(1);
    std::size_t i = 1;
    std::optional<LuaDebugFrame::Variable> var = debug.getUpvalue(fn, i);
    for (; var.has_value(); var = debug.getUpvalue(fn, ++i)) {
      values[var.value().key] = var.value().value.get<int>();
#ifdef LUACPPB_DEBUG_EXTRAS_SUPPORT
      idents[var.value().key] = debug.getUpvalueId(fn, i);
#endif
    }
  };
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["sum"](2, 3) == LuaStatusCode::Ok);
  REQUIRE(values.count("a") != 0);
  REQUIRE(values.count("b") != 0);
  REQUIRE(values.count("c") == 0);
#ifdef LUACPPB_DEBUG_EXTRAS_SUPPORT
  REQUIRE(idents["a"] != nullptr);
  REQUIRE(idents["b"] != nullptr);
  REQUIRE(idents["a"] != idents["b"]);
#endif
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
#ifdef LUACPPB_DEBUG_EXTRAS_SUPPORT
    REQUIRE(debug.getParameters() == 0);
    REQUIRE(debug.isVarArg());
    // REQUIRE_FALSE(debug.isTailCall());
#endif
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
    REQUIRE_NOTHROW(debug.getUpvalues());
#ifdef LUACPPB_DEBUG_EXTRAS_SUPPORT
    REQUIRE(debug.getParameters() == 2);
    REQUIRE_FALSE(debug.isVarArg());
    // REQUIRE_FALSE(debug.isTailCall());
#endif
  };
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["sum"](2, 3) == LuaStatusCode::Ok);
  REQUIRE(called);
}

TEST_CASE("Setting locals") {
  const std::string &CODE = "function sum(a, b)\n"
                            "  callback()\n"
                            "  return a + b\n"
                            "end";
  LuaEnvironment env;
  env["callback"] = [&]() {
    auto debug = env.getDebugFrame(1);
    std::size_t i = 1;
    std::optional<LuaDebugFrame::Variable> var = debug.getLocal(i++);
    auto val = LuaFactory::wrap(env, 50);
    for (; var.has_value(); var = debug.getLocal(i++)) {
      REQUIRE(debug.setLocal(i - 1, LuaFactory::mkref(env, val)));
    }
    REQUIRE_FALSE(debug.setLocal(i, LuaFactory::mkref(env, val)));
  };
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  auto res = env["sum"](2, 3);
  REQUIRE(res == LuaStatusCode::Ok);
  REQUIRE(res.get<int>() == 100);
}

TEST_CASE("Setting upvalues") {
  const std::string &CODE = "function sum(a, b)\n"
                            "  fn = function() return a+b end\n"
                            "  callback(fn)\n"
                            "  return fn()\n"
                            "end";
  LuaEnvironment env;
  env["callback"] = [&](LuaReferenceHandle fn) {
    auto debug = env.getDebugFrame();
    auto val = LuaFactory::wrap(env, 50);
    std::size_t i = 1;
    std::optional<LuaDebugFrame::Variable> var = debug.getUpvalue(fn, i++);
    for (; var.has_value(); var = debug.getUpvalue(fn, i++)) {
      REQUIRE(debug.setUpvalue(fn, i - 1, LuaFactory::mkref(env, val)));
    }
    REQUIRE_FALSE(debug.setUpvalue(fn, i, LuaFactory::mkref(env, val)));
  };
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  auto res = env["sum"](2, 3);
  REQUIRE(res == LuaStatusCode::Ok);
  REQUIRE(res.get<int>() == 100);
}

#ifdef LUACPPB_DEBUG_EXTRAS_SUPPORT

TEST_CASE("Joining upvalues") {

  const std::string &CODE = "function sum(a, b)\n"
                            "  fn = function() return a+b end\n"
                            "  callback(fn)\n"
                            "  return fn()\n"
                            "end";
  LuaEnvironment env;
  env["callback"] = [&](LuaReferenceHandle fn) {
    auto debug = env.getDebugFrame();
    REQUIRE_NOTHROW(debug.joinUpvalues(fn, 2, fn, 1));
  };
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  auto res = env["sum"](2, 3);
  REQUIRE(res == LuaStatusCode::Ok);
  REQUIRE(res.get<int>() == 4);
}

TEST_CASE("Getting current function") {
  const std::string &CODE = "function sum(a, b)\n"
                            "  callback()\n"
                            "  return a+b\n"
                            "end\n"
                            "sum(10, 20)";
  LuaEnvironment env;
  LuaReferenceHandle fn;
  env["callback"] = [&]() {
    auto debug = env.getDebugFrame(1);
    debug.getCurrentFunctionInfo();
    if (!fn.valid()) {
      fn = debug.getCurrentFunction();
    }
  };
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  auto res = fn(2, 3);
  REQUIRE(res == LuaStatusCode::Ok);
  REQUIRE(res.get<int>() == 5);
}

#endif

TEST_CASE("Getting current function lines") {
  const std::string &CODE = "function sum(a, b)\n"
                            "  callback()\n"
                            "  return a+b\n"
                            "end\n"
                            "sum(10, 20)";
  LuaEnvironment env;
  LuaReferenceHandle lines;
  env["callback"] = [&]() {
    auto debug = env.getDebugFrame(1);
    lines = debug.getLines();
  };
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(lines.getType() == LuaType::Table);
  REQUIRE(lines[2].getType() == LuaType::Boolean);
  REQUIRE(lines[3].getType() == LuaType::Boolean);
}

TEST_CASE("Getting function lines") {
  const std::string &CODE = "function sum(a, b)\n"
                            "  callback(sum)\n"
                            "  return a+b\n"
                            "end\n"
                            "sum(10, 20)";
  LuaEnvironment env;
  LuaReferenceHandle lines;
  env["callback"] = [&](LuaReferenceHandle fn) {
    auto debug = env.getDebugFrame();
    lines = debug.getLines(fn);
  };
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(lines.getType() == LuaType::Table);
  REQUIRE(lines[2].getType() == LuaType::Boolean);
  REQUIRE(lines[3].getType() == LuaType::Boolean);
}

TEST_CASE("Per-line debug hooks") {
  const std::string &CODE = "a = 1\n"
                            "b = 2\n"
                            "c = 3\n"
                            "res = a + b + c";
  LuaEnvironment env;
  auto &debug = env.getDebugHooks();
  std::size_t sum = 0, sum2 = 0;
  debug.onLine([&](LuaDebugFrame &frame) {
    sum += frame.getCurrentLine();
  });
  LuaDebugHooks::Detach detach = debug.onLine([&](LuaDebugFrame &frame) {
    sum2 += frame.getCurrentLine();
    if (frame.getCurrentLine() > 1) {
      detach();
    }
  });
  REQUIRE(detach.attached());
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE_FALSE(detach.attached());
  REQUIRE(sum == 10);
  REQUIRE(sum2 == 3);
}

TEST_CASE("On-return debug hooks") {
  const std::string &CODE = "function sum(a, b)\n"
                            "    return a + b\n"
                            "end\n"
                            "sum(2, 3)";
  LuaEnvironment env;
  auto &debug = env.getDebugHooks();
  int sumReturn = 0;
  int line = 0;
  LuaDebugHooks::Detach detach = debug.onReturn([&](LuaDebugFrame &frame) {
    frame.getCurrentFunctionInfo(LuaDebugFrame::Function::Line, LuaDebugFrame::Function::Name, LuaDebugFrame::Function::Source, LuaDebugFrame::Function::Params);
    sumReturn = frame.getCurrentLine();
    if (frame.getName().compare("sum") == 0) {
      detach();
    }
  });
  debug.onReturn([&](LuaDebugFrame &frame) {
    frame.getCurrentFunctionInfo(LuaDebugFrame::Function::Line, LuaDebugFrame::Function::Name, LuaDebugFrame::Function::Source, LuaDebugFrame::Function::Params);
    REQUIRE(frame.getCurrentLine() > line);
    line = frame.getCurrentLine();
  });
  REQUIRE(detach.attached());
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE_FALSE(detach.attached());
  REQUIRE(sumReturn == 2);
  REQUIRE(line > 0);
}

TEST_CASE("On-call debug hook") {
  const std::string &CODE = "function sum(a, b)\n"
                            "    return a + b\n"
                            "end\n"
                            "sum(2, 3)";
  LuaEnvironment env;
  auto &debug = env.getDebugHooks();
  int sumEntry = 0;
  LuaDebugHooks::Detach detach = debug.onCall([&](LuaDebugFrame &frame) {
    frame.getCurrentFunctionInfo(LuaDebugFrame::Function::Line, LuaDebugFrame::Function::Name, LuaDebugFrame::Function::Source, LuaDebugFrame::Function::Params);
    sumEntry = 0;
    if (frame.getName().compare("sum") == 0) {
      sumEntry = frame.getCurrentLine();
      detach();
    }
  });
  REQUIRE(detach.attached());
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE_FALSE(detach.attached());
  REQUIRE((sumEntry > 0 && sumEntry < 3));
}

TEST_CASE("On-counter debug hook") {
  const std::string &CODE = "a = 1\n"
                            "b = 2\n"
                            "c = 3\n"
                            "res = a + b + c";
  LuaEnvironment env;
  auto &debug = env.getDebugHooks();
  int total = 0, total2 = 0, total3 = 0;
  LuaDebugHooks::Detach detach = debug.onCount([&](LuaDebugFrame &frame) {
    total++;
    if (total > 7) {
      detach();
    } 
  });
  debug.onCount([&](LuaDebugFrame &frame) {
    total2++;
  }, 2);
  debug.onCount([&](LuaDebugFrame &frame) {
    total3++;
  });
  REQUIRE(detach.attached());
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE_FALSE(detach.attached());
  REQUIRE(total == 8);
  REQUIRE((total3 - total2) == total3 / 2);
}

TEST_CASE("Debug hook movement") {
  const std::string &CODE = "a = 1\n"
                            "b = 2\n"
                            "c = 3\n"
                            "res = a + b + c";
  LuaEnvironment env;
  auto &debug = env.getDebugHooks();
  LuaDebugHooks::Detach finalDetach;
  bool res = false;
  auto detach = debug.onLine([&](LuaDebugFrame &) {
    if (res) {
      REQUIRE(false);
    } else {
      res = true;
      finalDetach();
    }
  });
  REQUIRE(detach.attached());
  LuaDebugHooks::Detach detach2(std::move(detach));
  REQUIRE_FALSE(detach.attached());
  REQUIRE_NOTHROW(detach());
  REQUIRE(detach2.attached());
  finalDetach = std::move(detach2);
  REQUIRE_FALSE(detach2.attached());
  REQUIRE_NOTHROW(detach2());
  REQUIRE(finalDetach.attached());
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(res);
}

#endif