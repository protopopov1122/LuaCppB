#include <iostream>
#include <cstdlib>
#include "luacppb/State.h"
#include "luacppb/Reference.h"
#include "luacppb/Function.h"
#include "luacppb/Table.h"

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
	auto table = LuaTable();
	table["inner"] = LuaTable().put("add", &sum, &Sum::add);
	env["sum"] = table;
	env["test"] = CMethodCall(&sum, &Sum::add);
	env["populate"] = CInvocableCall<std::function<void(LuaTableBase)>, LuaTableBase>([](LuaTableBase tb) {
		LuaReferenceHandle table = tb;
		table["x"] = 15;
		table["msg"] = "Hello, world!";
	});
	env["_print"] = CFunctionCall(print);
	env["test"] = LuaTable().put("x", 100).put("print", print);
	env.load("test.lua");
	std::cout << env["y"].get<std::string>() << std::endl;
	return EXIT_SUCCESS;
}
