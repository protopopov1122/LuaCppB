#include "catch.hpp"
#include "luacppb/LuaCppB.h"
#include <iostream>

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
                            "arr2[50] = arr2[50] + 100";
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
  arrayUD.bind("__newindex", [](IntegerArray &arr, std::size_t idx, int value) {
    idx--;
    if (idx < arr.length) {
      arr.array[idx] = value;
    }
  });
  arrayUD.bind("__len", [](IntegerArray &arr) {
    return arr.length;
  });

  auto filledArray = arrayUD.create(env.getState(), [](IntegerArray &arr) {
    new(&arr) IntegerArray(100);
    for (std::size_t i = 0; i < arr.length; i++) {
      arr.array[i] = i;
    }
  });
  auto emptyArray = arrayUD.create(env.getState());

  IntegerArray *array = filledArray;
  IntegerArray &array2 = emptyArray;
  REQUIRE(array != nullptr);
  env["arr"] = filledArray;
  env["arr2"] = emptyArray;
  REQUIRE(env["arr"].get<IntegerArray *>() == array);
  REQUIRE(&env["arr2"].get<IntegerArray &>() == &array2);
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["sum"].get<int>() == 4950);
  REQUIRE(array2.array[49] == 100);
}