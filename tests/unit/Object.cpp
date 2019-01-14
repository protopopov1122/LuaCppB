#include "catch.hpp"
#include "luacppb/LuaCppB.h"

using namespace LuaCppB;

class Arith {
 public:
  Arith() : Value(10), n(10) {}
  Arith(int n) : Value(n), n(n) {}
  int add(int x) const {
    return n + x;
  }
  int sub(int x) {
    return n - x;
  }
  void set(int n) {
    this->n = n;
  }

  static Arith &getGlobal() {
    return Arith::global;
  }

  static Arith *getGlobalPointer() {
    return &Arith::global;
  }

  static std::unique_ptr<Arith> newArith(int n) {
    return std::make_unique<Arith>(n);
  }

  static std::shared_ptr<Arith> newSharedArith(int n) {
    return std::make_shared<Arith>(n);
  }

  const int Value;
 protected:
  static Arith global;
  int n;
};

class NArith : public Arith {
 public:
  using Arith::Arith;
  int mul(int n) {
    return this->n * n;
  }
};

Arith Arith::global(0);

void test_object_as_argument(Arith obj, const Arith &obj2, Arith *obj3) {
  REQUIRE(obj.add(10) == 20);
  REQUIRE(obj2.add(20) == 30);
  REQUIRE(obj3->add(30) == 40);
  REQUIRE(&obj2 == obj3);
}

void test_object_from_invocable(LuaEnvironment &env, const std::string &CODE) {
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["arith"].exists());
  REQUIRE(env["arith"].getType() == LuaType::UserData);
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["result"][1].get<int>() == 50);
  REQUIRE(env["result"][2].get<int>() == -100);
}

void test_object_return_from_invocable(LuaEnvironment &env) {
  const std::string &CODE = "arith = getArith()\n"
                            "result = { arith:add(50), arith:sub(100) }";
  test_object_from_invocable(env, CODE);
}

void test_object_build_from_invocable(LuaEnvironment &env) {
  const std::string &CODE = "arith = getArith(0)\n"
                            "result = { arith:add(50), arith:sub(100) }";
  test_object_from_invocable(env, CODE);
}

void test_object_smart_pointer(LuaEnvironment &env) {
  const std::string &CODE = "result = { arith:add(50), arith:sub(100) }";
  REQUIRE(env["arith"].exists());
  REQUIRE(env["arith"].getType() == LuaType::UserData);
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["result"][1].get<int>() == 60);
  REQUIRE(env["result"][2].get<int>() == -90);
}

TEST_CASE("Object binding") {
  LuaEnvironment env;
  Arith arith(10);
  auto constant = LuaValueFactory::newTable(env);
  constant["pi"] = 3.14f;
  LuaCppObject aObj(arith, env);
  aObj.bind("add", &Arith::add);
  aObj.bind("sub", &Arith::sub);
  aObj.bind("value", &Arith::Value);
  aObj.bind("constant", *constant);
  env["arith"] = aObj;
  SECTION("Binding") {
    const std::string &CODE = "result = { arith:add(50), arith:sub(100) - arith.value, arith.constant.pi }";
    REQUIRE(env["arith"].exists());
    REQUIRE(env["arith"].getType() == LuaType::UserData);
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["result"][1].get<int>() == 60);
    REQUIRE(env["result"][2].get<int>() == -100);
    REQUIRE(env["result"][3].get<float>() == 3.14f);
  }
  SECTION("Object extracting") {
    const std::string &CODE = "test(arith, arith, arith)";
    const Arith obj = env["arith"];
    Arith &obj2 = env["arith"];
    REQUIRE(obj.add(10) == 20);
    REQUIRE(obj2.add(20) == 30);
    REQUIRE(&obj2 == &arith);
    env["test"] = test_object_as_argument;
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  }
}

TEST_CASE("Class manual binding") {
  LuaEnvironment env;
  SECTION("Class binding") {
    const std::string &CODE = "a = Arith.build(55)"
                              "x = a:add(5)\n"
                              "a2 = Arith.new()\n"
                              "a2:set(50)\n"
                              "result = { x, a2:sub(20), a.value + a2.value, a.constant .. Arith.constant }";
    LuaCppClass<Arith> arith(env);
    arith.bind("add", &Arith::add);
    arith.bind("sub", &Arith::sub);
    arith.bind("set", &Arith::set);
    arith.bind("value", &Arith::Value);
    arith.bind("constant", "Hello");
    arith.bind("build", &LuaCppConstructor<Arith, int>);
    env["Arith"] = arith;
    env.getClassRegistry().bind(arith);
    REQUIRE(env["Arith"].exists());
    REQUIRE(env["Arith"].getType() == LuaType::Table);
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["a"].exists());
    REQUIRE(env["a"].getType() == LuaType::UserData);
    REQUIRE(env["result"][1].get<int>() == 60);
    REQUIRE(env["result"][2].get<int>() == 30);
    REQUIRE(env["result"][3].get<int>() == 65);
    REQUIRE(env["result"][4].get<std::string>().compare("HelloHello") == 0);
  }
  SECTION("Unbound class assignment") {
    const std::string &CODE = "res = arith == nil and parith == nil and uarith == nil and sarith == nil";
    Arith arith(100);
    env["arith"] = arith;
    env["parith"] = &arith;
    env["uarith"] = std::make_unique<Arith>(100);
    env["sarith"] = std::make_shared<Arith>(100);
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["res"].get<bool>());
  }
}

TEST_CASE("Inheritance") {
  const std::string &CODE = "narith = NArith.build(20 * arith.constant)\n"
                            "narith2 = NArith.new()\n"
                            "result = { narith:mul(narith.constant) + narith.value, arith:add(35), narith2:mul(6) }\n";
  LuaEnvironment env;
  LuaCppClass<Arith> arithCl("Arith", env);
  arithCl.bind("add", &Arith::add);
  arithCl.bind("sub", &Arith::sub);
  arithCl.bind("set", &Arith::set);
  arithCl.bind("value", &Arith::Value);
  arithCl.bind("constant", 5);
  arithCl.bind("build", &LuaCppConstructor<Arith, int>);
  env.getClassRegistry().bind(arithCl);

  LuaCppClass<NArith, Arith> narithCl("NArith", env);
  narithCl.bind("mul", &NArith::mul);
  narithCl.bind("build", &LuaCppConstructor<NArith, int>);
  env.getClassRegistry().bind(narithCl);
  env["NArith"] = narithCl;

  NArith arith(10);
  env["arith"] = arith;
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["result"][1].get<int>() == 600);
  REQUIRE(env["result"][2].get<int>() == 45);
  REQUIRE(env["result"][3].get<int>() == 60);
}

TEST_CASE("Object opaque binding") {
  LuaEnvironment env;
  LuaCppClass<Arith> arithCl("Arith", env);
  arithCl.bind("add", &Arith::add);
  arithCl.bind("sub", &Arith::sub);
  arithCl.bind("set", &Arith::set);
  arithCl.bind("value", &Arith::Value);
  arithCl.bind("constant", 10);
  env.getClassRegistry().bind(arithCl);
  SECTION("Assigning object") {
    SECTION("Assigning object by reference") {
      const std::string &CODE = "result = { arith:add(arith.constant * 5) * arith.value, arithRef:sub(100) }";
      Arith arith(10);
      env["arith"] = arith;
      env["arithRef"] = std::ref(arith);
      REQUIRE(env["arith"].exists());
      REQUIRE(env["arithRef"].exists());
      REQUIRE(env["arith"].getType() == LuaType::UserData);
      REQUIRE(env["arithRef"].getType() == LuaType::UserData);
      REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
      REQUIRE(env["result"][1].get<int>() == 600);
      REQUIRE(env["result"][2].get<int>() == -90);
    }
    SECTION("Assigning object unique pointer") {
      env["arith"] = std::make_unique<Arith>(10);
      test_object_smart_pointer(env);
    }
    SECTION("Assigning object shared pointer") {
      env["arith"] = std::make_shared<Arith>(10);
      test_object_smart_pointer(env);
    }
  }
  SECTION("Returning object from invocable") {
    SECTION("Object pointer") {
      env["getArith"] = &Arith::getGlobal;
      test_object_return_from_invocable(env);
    }
    SECTION("Object reference") {
      env["getArith"] = &Arith::getGlobalPointer;
      test_object_return_from_invocable(env);
    }
    SECTION("Object unique pointer") {
      env["getArith"] = &Arith::newArith;
      test_object_build_from_invocable(env);
    }
    SECTION("Object shared pointer") {
      env["getArith"] = &Arith::newSharedArith;
      test_object_build_from_invocable(env);
    }
  }
}

TEST_CASE("Costant object references") {
  LuaEnvironment env;
  Arith arith(10);
  const Arith &cArith = arith;
  SECTION("Object binding") {
    const std::string &CODE = "res = arith:add(5)";
    LuaCppObject<const Arith> obj(cArith, env);
    obj.bind("add", &Arith::add);
    env["arith"] = obj;
    REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
    REQUIRE(env["res"].get<int>() == 15);
  }
#ifdef LUACPPB_EXCEPTION_PROPAGATION
  SECTION("Class binding") {
    const std::string &CODE = "arith:set(5)\n"
                              "res = arith:add(5)";
    LuaCppClass<Arith> arithCl("Arith", env);
    arithCl.bind("add", &Arith::add);
    arithCl.bind("set", &Arith::set);
    env.getClassRegistry().bind(arithCl);
    env["arith"] = cArith;
    REQUIRE_THROWS(env.execute(CODE) != LuaStatusCode::Ok);
  }
#endif
}

TEST_CASE("Exception handling") {
  const std::string &CODE = "arith:set(5)\n"
                            "res = arith:add(5)";
  LuaEnvironment env;
  Arith arith(10);
  const Arith &cArith = arith;
  LuaCppClass<Arith> arithCl("Arith", env);
  arithCl.bind("add", &Arith::add);
  arithCl.bind("set", &Arith::set);
  env.getClassRegistry().bind(arithCl);
  env["arith"] = cArith;
  REQUIRE_THROWS(env.execute(CODE) != LuaStatusCode::Ok);
  env.setExceptionHandler([](std::exception &ex) {});
  REQUIRE_NOTHROW(env.execute(CODE) != LuaStatusCode::Ok);
}

enum class EnumTest {
  A = 1,
  B = 2,
  C = 3
};

TEST_CASE("Enum") {
  const std::string &CODE = "res = A + B + C\n"
                            "el = A + B";
  LuaEnvironment env;
  env["A"] = EnumTest::A;
  env["B"] = EnumTest::B;
  env["C"] = EnumTest::C;
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["res"].get<int>() == 6);
  REQUIRE(env["el"].get<EnumTest>() == EnumTest::C);
}

TEST_CASE("Object binder") {
  const std::string &CODE = "r1 = arith:sub(ptr.constant) * arith.value\n"
                            "arith:set(20)\n"
                            "r2 = ptr:add(10) / ptr.value\n";
  LuaEnvironment env;
  Arith arith(10);
  const Arith *ptr = &arith;
  env["ptr"] = ObjectBinder::bind(ptr, env, "add", &Arith::add, "value", &Arith::Value, "constant", 5);
  env["arith"] = ObjectBinder::bind(arith, env, "add", &Arith::add, "sub", &Arith::sub, "set", &Arith::set, "value", &Arith::Value);
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["r1"].get<int>() == 50);
  REQUIRE(env["r2"].get<int>() == 3);
}

TEST_CASE("Class binder") {
  const std::string &CODE = "temp = NArith.new(NArith.constant)\n"
                            "arith = Arith.new(temp.constant / 2)\n"
                            "r1 = arith:add(4)\n"
                            "arith:set(100)\n"
                            "r2 = arith:sub(5)\n"
                            "r3 = narith:mul(5) + temp:sub(5) - narith.value";
  LuaEnvironment env;
  env["Arith"] = ClassBinder<Arith>::bind(env, "add", &Arith::add, "sub", &Arith::sub, "set", &Arith::set, "new", &Arith::newArith, "value", &Arith::Value, "constant", 100);
  env["NArith"] = ClassBinder<NArith, Arith>::bind("NArith", env, "mul", &NArith::mul, "new", &LuaCppConstructor<NArith, int>);
  NArith narith(60);
  env["narith"] = narith;
  REQUIRE(env.execute(CODE) == LuaStatusCode::Ok);
  REQUIRE(env["r1"].get<int>() == 54);
  REQUIRE(env["r2"].get<int>() == 95);
  REQUIRE(env["r3"].get<int>() == 335);
}