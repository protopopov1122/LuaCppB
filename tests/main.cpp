#include <iostream>
#include <cstdlib>
#include "luacppb/State.h"
#include "luacppb/Reference/Reference.h"
#include "luacppb/Invoke/Native.h"

using namespace LuaCppB;

class Sum {
 public:
	Sum(float x) : x(x) {}
	float add(float y) {
		return this->x + y;
	}
 private:
	float x;
};

void print(std::string line, bool newline) {
	std::cout << line;
	if (newline) {
		std::cout << std::endl;
	}
}

int main(int argc, char **argv) {
	LuaEnvironment env;
	Sum sum(10);
	auto table_tmp = LuaTable::create(env.getState());
	LuaReferenceHandle table = table_tmp;
	table["inner"] = LuaTable::create(env.getState());
	LuaReferenceHandle innerTable = table["inner"];
	innerTable["add"] = NativeMethodCall(&sum, &Sum::add);
	env["sum"] = *table;
	env["test"] = NativeMethodCall(&sum, &Sum::add);
	env["populate"] = NativeInvocableCall<std::function<void(LuaTable)>, LuaTable>([](LuaTable tb) {
		LuaReferenceHandle table = tb;
		table["x"] = 15;
		table["msg"] = "Hello, world!";
	});
	env["_print"] = NativeFunctionCall(print);
	LuaReferenceHandle test = LuaTable::create(env.getState());
	test["print"] = print;
	test["x"] = 100;
	env["test"] = *test;
	if (env.load("test.lua") != LuaStatusCode::Ok) {
		std::cout << lua_tostring(env.getState(), -1) << std::endl;
	}
	env["root"]();
	int quad = env["quad"](16);
	std::tuple<int, int> quadTuple = env["quad"](16);
	std::cout << quad << '\t' << std::get<1>(quadTuple) << '\t' << env["y"].get<std::string>() << std::endl;
	return EXIT_SUCCESS;
}
