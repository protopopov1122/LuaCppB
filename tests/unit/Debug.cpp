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

using namespace LuaCppB;

TEST_CASE("Debug") {
  const std::string &CODE = "function sum(a, b)\n"
                            "  callback(function() return a+b end)\n"
                            "  return a + b\n"
                            "end";
  LuaEnvironment env;
  env["callback"] = [&](LuaReferenceHandle handle) {
    auto debug = env.getDebugFrame(1);
    std::size_t i = 1;
    std::optional<LuaDebugFrame::Variable> var = debug.getUpvalue(handle, i);
    for (; var.has_value(); var = debug.getUpvalue(handle, ++i)) {
      std::cout << var.value().key << '\t' << var.value().value.get<int>() << '\t' << debug.getUpvalueId(handle, i) << std::endl;
    }
    debug.getFunctionInfo(handle, LuaDebugFrame::Function::Name, LuaDebugFrame::Function::Source, LuaDebugFrame::Function::Line);
    std::cout << debug.getSource() << "\t" << handle().get<int>() << std::endl;
  };
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  try {
    env["sum"](2, 3);
  } catch (std::exception &ex) {
    std::cout << ex.what() << std::endl;
  }
}