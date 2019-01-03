#include <cstdlib>
#include "luacppb/LuaCppB.h"
#include <iostream>

using namespace LuaCppB;

class Person {
 public:
  Person(const std::string &name, unsigned int age) : name(name), age(age) {}

  const std::string &getName() const {
    return this->name;
  }

  unsigned int getAge() const {
    return this->age;
  }

  void setName(const std::string &name) {
    this->name = name;
  }
 private:
  std::string name;
  unsigned int age;
};

class Employee : public Person {
 public:
  Employee(const std::string &name, unsigned int age, unsigned int salary) : Person(name, age), salary(salary) {}

  unsigned int getSalary() const {
    return this->salary;
  }
 private:
  unsigned int salary;
};

class PersonHandle {
 public:
  PersonHandle(std::unique_ptr<Person> person) : person(std::move(person)) {}
  Person &getPerson() {
    return *this->person;
  }
 private:
  std::unique_ptr<Person> person;
};

void object_examples(int argc, const char **argv) {
  std::cout << "Objects: " << std::endl;
  LuaEnvironment env;
  env["Person"] = ClassBinder<Person>::bind(env,
    "new", &LuaCppConstructor<Person, const std::string &, unsigned int>,
    "getName", &Person::getName,
    "getAge", &Person::getAge,
    "setName", &Person::setName);
  env["Employee"] = ClassBinder<Employee, Person>::bind(env,
    "new", &LuaCppConstructor<Employee, const std::string &, unsigned int, unsigned int>,
    "getSalary", &Employee::getSalary);
  PersonHandle handle(std::make_unique<Employee>("Test", 50, 150));
  env["handle"] = ObjectBinder::bind(handle, env,
    "getPerson", &PersonHandle::getPerson);
  if (!env.load("lua/objects.lua").hasError()) {
    env.execute("classBinding('Hello')");
  }
}