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

using namespace LuaCppB;

struct IntegerArray {
  IntegerArray(std::size_t length) : length(length) {
    this->array = new int[length];
  }

  ~IntegerArray() {
    delete[] this->array;
  }
  std::size_t length;
  int *array;
};

TEST_CASE("Custom userdata") {
  const std::string &CODE = "sum = 0\n"
                            "for i = 1, #arr do\n"
                            "    sum = sum + arr[i]\n"
                            "end\n"
                            "arr2[50] = arr2[50] + (arr2 + 50)\n"
                            "metaNil = getmetatable(arr).__len == nil";
  LuaEnvironment env;

  CustomUserDataClass<IntegerArray> arrayUD(env);
  arrayUD.setDefaultConstructor([](IntegerArray &arr) {
    new(&arr) IntegerArray(100);
    for (std::size_t i = 0; i < arr.length; i++) {
      arr.array[i] = 0;
    }
  });
  arrayUD.bind(LuaMetamethod::GC, [](IntegerArray &arr) {
    arr.~IntegerArray();
    ::operator delete(&arr, &arr);
  });
  arrayUD.bind("__index", [](IntegerArray &arr, std::size_t idx) {
    idx--;
    if (idx < arr.length) {
      return arr.array[idx];
    } else {
      return -1;
    }
  });
  arrayUD.bind("__newindex", [](IntegerArray &arr, std::size_t idx, int value) mutable {
    idx--;
    if (idx < arr.length) {
      arr.array[idx] = value;
    }
  });
  arrayUD.bind("__len", [](IntegerArray &arr) {
    return arr.length;
  });
  arrayUD.bind(LuaMetamethod::Add, [](IntegerArray &arr, int num) {
    return arr.length + num;
  });
  arrayUD.bind("__metatable", *LuaFactory::newTable(env));

  auto filledArray = arrayUD.create(env, [](IntegerArray &arr) {
    new(&arr) IntegerArray(100);
    for (std::size_t i = 0; i < arr.length; i++) {
      arr.array[i] = i;
    }
  });
  auto emptyArray = arrayUD.create(env);

  IntegerArray *array = filledArray;
  IntegerArray &array2 = emptyArray;
  REQUIRE(array != nullptr);
  env["arr"] = filledArray;
  env["arr2"] = emptyArray;
  REQUIRE(env["arr"].get<IntegerArray *>() == array);
  REQUIRE(&env["arr2"].get<IntegerArray &>() == &array2);
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["sum"].get<int>() == 4950);
  REQUIRE(array2.array[49] == 150);
  REQUIRE(env["metaNil"].get<bool>());
}
