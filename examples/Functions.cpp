#include "luacppb/LuaCppB.h"
#include <iostream>

using namespace LuaCppB;

unsigned int factorial(unsigned int n) {
  if (n < 2) {
    return 1;
  } else {
    return n * factorial(n - 1);
  }
}

void function_examples(int argc, const char **argv) {
  std::cout << "Functions: " << std::endl;
  LuaEnvironment env;
  env["factorial"] = factorial;
  env["fibonacci"] = [](unsigned int n) {
    std::vector<unsigned int> vec = { 1, 1 };
    for (std::size_t i = 2; i < n; i++) {
      vec.push_back(vec[i - 2] + vec[i - 1]);
    }
    return std::make_unique<std::vector<unsigned int>>(vec);
  };
  env.load("lua/functions.lua");
}