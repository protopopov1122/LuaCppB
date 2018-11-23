#include <iostream>
#include <cstdlib>
#include "luacppb/LuaState.h"
#include "luacppb/LuaReference.h"
#include "luacppb/Function.h"

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

void populate(LuaReferenceHandle table) {
	table["x"] = 15;
}

int main(int argc, char **argv) {
	LuaEnvironment env;
	Sum sum(10);
	env["add"] = CMethodCall(&sum, &Sum::add);
	env["populate"] = populate;
	env["_print"] = print;
	env.load("test.lua");
	return EXIT_SUCCESS;
}
