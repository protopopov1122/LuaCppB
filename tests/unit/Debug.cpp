#include "catch.hpp"
#include "luacppb/LuaCppB.h"
#include <iostream>

using namespace LuaCppB;

TEST_CASE("Debug") {
  const std::string &CODE = "function sum(a, b)\n"
                            "  return a + b\n"
                            "end\n"
                            "callback(sum)";
  LuaEnvironment env;
  env["callback"] = [&](LuaReferenceHandle sum) {
    auto debug = env.getDebugFrame();
    debug.getFunctionInfo(sum, LuaDebugFrame::Function::Name, LuaDebugFrame::Function::Source, LuaDebugFrame::Function::Line);
    std::cout << debug.getSource() << "\t" << sum(2, 3).get<int>() << std::endl;
  };
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
}