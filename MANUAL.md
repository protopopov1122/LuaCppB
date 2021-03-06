## LuaCppB manual
This manual describes `LuaCppB` library usage.
### Installation
`LuaCppB` library requires integration into build system. Module for CMake is prepared in the `cmake` directory. Use unit test build script `CMakeLists.txt` as an example. \
You should also enable custom build of Lua - download and unpack Lua sources, specify source directory in `cmake` module arguments. This enables Lua continuation support and error throwing mechanism.
### Preparations
Before using LuaCppB you should include the header `luacppb/LuaCppB.h`:
```C++
#include "luacppb/LuaCppB.h"
```
Then you should construct new `LuaEnvironment` object:
```C++
namespace lcb = LuaCppB; // Convenience alias

int main(int argc, const char **argv) {
  lcb::LuaEnvironment env;
}
```
This environment object holds raw Lua state and should not be destructed until you finished work with Lua. Raw Lua state can be retrieved using `getState` method.

### Usage
Namespace `LuaCppB` is omitted to simplify code examples. You should use `using namespace` statement or define an alias (e.g. `namespace lcb = LuaCppB;`).
#### Running Lua code
There are several ways to run Lua code:
* loading from file - use `env.load("path to file")`.
* directly executing Lua statement - use `env.execute("Lua code")`
* directly executing Lua expression - use `env("Lua code")`
Examples
```C++
if (!env.load("file.lua").hasError()) {
  // Do something
}
env.execute("print('Hello, world')");
int val = env("2+2*2");
```
#### Accessing global variables, table fields and stack
Variables from global scope can be accessed, using operator `[]` of `LuaState` class (`LuaEnvironment` is a subclass of `LuaState`). This gives you a reference to Lua data (which in general may be global variable, table field, value from stack). Reference can be implicitly converted to some of C++ types (integral and floating point numbers, booleans, strings, references and pointers to arbitrary types and containers, some `LuaCppB` objects), converted to `LuaValue` (wrapper class for any Lua values), copied (referenced values will be accessible as long as reference exists). New values can be assigned to references. Examples:
```C++
int sum = env["sum"]; // Implicitly converting the value of Lua global to integer
std::cout << env["name"].get<const std::string &>() << std::endl; // Explicit conversion is possible then type deduction may not work
env["flag"] = true; // Setting boolean as the value of Lua global name
std::vector<int> &vec = env["table"]["arrays"][1]; // Getting the value from nested tables
auto fn = env[1]; // Referencing the value directly from Lua stack; read-only reference. Sometimes it's useful to get raw access to function arguments
```
If the reference is not converted to some C++ types, it will be stored as an instance of `LuaReferenceHandle` class.

#### Lua values
Most C++ values are implicitly converted to Lua equivalents. `LuaCppB` includes the factory for some complex values:
* Tables - creating new table objects are performed with following calls:
  ```C++
  auto table = LuaFactory::newTable(env);  // Creating and referencing a new table
  table["abc"] = 123;                           // Adding keys and values
  // Then you can assign it to any reference
  // * operator is vital, otherwise only reference itself will be copied
  env["table"] = *table;
  ```
* Functions - creating anonymous function references
  ```C++
  auto fn = LuaFactory::newFunction(env, [](int i) { return i * i; }); // You can use function pointers as well
  env["some_func"] = *fn;
  ```
* Threads - building Lua coroutines
  ```C++
  auto thread = LuaFactory::newThread(env, env["some_func"]);    // You can pass any function reference, including the one from previous example
  env["thread"] = *thread;
  ```

You can also wrap any of supported types in `LuaValue` object using `LuaFactory::wrap` method. It may be useful when assigning constants to wrapped classes. Example:
```C++
LuaValue value = LuaFactory::wrap(env, "Hello, world!");
```
`LuaCppB` supports iteration over Lua tables:
```C++
auto table = env["tbl"];
for (auto it = table.begin(); it != table.end(); ++it) {
  std::pair<LuaValue, LuaValue> key_value = *it;
}
```

#### Metatables
It's possible to assign/retrieve metatable to/from any reference. Consider following example:
```C++
  auto sometable = env["table"];
  auto metatable = LuaFactory::newTable(env);
  sometable.setMetatable(*metatable);                 // Assigning metatable
  sometable.getMetatable()["a"] = 1;                  // Retrieving metatable
```

#### Calling C++ from Lua and vice versa
C++ functions (as well as lambdas and object methods) are treated as the first-class objects. It means, that they can be assigned to any `LuaCppB` reference (except read-only stack references), or passed as an argument while calling Lua function. You can use special `NativeCallable` function (which is strictly necessary while dealing with object methods) or directly pass them to Lua. Examples:
```C++
int sum(int, int);
int test(LuaState, int); // LuaState is a virtual argument which is generated by LuaCppB in runtime
                         // Actually Lua function will have 1 argument
...
env["sum"] = sum; // Direct assignment
env["lua_function"]([](float x) { return -x; }); // Passing lambda as an argument to Lua function
SomeObject obj;
env["method"] = NativeCallable(obj, &SomeObject::some_method, env); // Binding object method
```
Lua function calling is pretty straightforward:
```C++
std::string result = env["fn"](1, true, "5");
std::tuple<int, int, int> res = env["getInts"](5); // Multiple results can be retrieved using std::pair and std::tuple
```
Lua functions can also be converted to `std::function` instances:
```C++
int res = env("fn")(5); // Just call it
std::function fn = LuaLambda(env["fn"]); // Or convert it into std::function
res = fn(5);
```

#### Binding C++ objects to Lua
C++ classes and separate objects can be bound to `LuaCppB` environment. That creates necessary metatables with method wrappers and enables transparent conversion (e.g. you can pass C++ object to Lua function). Class binding is possible using `LuaCppClass` - it is bound to Lua environment to enable transparent conversion. If you need constructing new objects inside Lua code, you should also save it as the global variable.
```C++
env["Person"] = ClassBinder<Person>::bind(env,  // Class is bound to environment and exposed to Lua code at the same time
    "getName", &Person::getName,    // const methods are properly handled
    "setName", &Person::setName,
    "build", &Person::build,        // You can bind static methods
    "id", &Person::ID,              // Class fields are bound in read-only mode
    "seed", 1234,                   // Bound constants will be an instance fields
    "new", &LuaCppConstructor<Person, const std::string &> // And even constructors
);
```
You can override the default name of Lua metatable by passing it as the first parameter to class binder. If the class has the default constructor it will be implicitly bound as `new` method (you can override it). There is limited inheritance support (you should bind superclass **before** subclass):
```C++
env["Employee"] = ClassBinder<Employee, Person>(env,
    "getSalary" &Employee::getSalary  // Bind only methods from subclass
);
```
Similarly you can bind separate objects (static methods and constructors are not supported as it makes a little sense for a separate object):
```C++
SomeObject obj;
env["object"] = ObjectBinder(obj, env,
  "add", &SomeObject::add,
  "get", &SomeObject::get,
  "id", &SomeObject::ID,
  "constant", 1234
)
```
You can use object references (const or non-const), pointer (const or non-const), as well as standard smart pointers (`std::unique_ptr` and `std::shared_ptr`) while passing C++ object to Lua.
```C++
env["me"] = std::make_unique<Person>("Eugene");
env["someone"] = &someone;
Person &eugene = env["me"]; // Retrieving an object from Lua
```

#### Standard library classes
`LuaCppB` supports following STL containers - `std::vector`, `std::map`, `std::set`, `std::pair` and `std::tuple`. Vectors, maps and sets may be passed by reference (const and non-const) or using smart pointers. Pairs and tuples are passed only by reference and are simultaneously converted to Lua tables, while vectors, maps and sets are wrapped by `LuaCppB`. On Lua side you can access those container content using operator `[]` or functions `pairs` and `ipairs`, as well as modify it (if passed object is not constant). `#` operator is also available. Examples:
```C++
std::vector<int> ints = { 1, 2, 3, 4 };
env["array"] = ints;                            // Passing vector by reference
std::vector<int> &the_same_ints = env["array"]; // And getting it back
env["unique_map"] = std::make_unique<std::map<std::string, std::string>>(); // Binding unique std::map
```
`std::optional` and `std::reference_wrapper` classes are also supported. Optionals will be unwrapped (using Nil as the fallback value), references will be unwrapped.

#### Coroutines
`LuaCppB` supports coroutines from both sides - you can invoke Lua coroutine from C++ and build native coroutine in C++. Lua coroutine invocation is simple:
```C++
int some_yielded_result = env["coro"](5); // You can call existing coroutine the same way as any other Lua function
LuaCoroutine coro = env["coro"];          // Or use a special wrapper class
int also_yielded_result = coro(5);
LuaStatusCode status = coro.getStatus();  // Retrieving coroutine status
```
`LuaCppB` support continuations in the native code. There are two functions which operate with continuations: `LuaContinuation::call` and `LuaContinuation::yield`.

> **Warning!** Invocation of continuation-related functions should be the last statement in the surrouning function. Execution of statements after it is not guaranteed and you should not rely on that. **Moreover**, most likely the call stack will be unwound and those functions will never return. **Also**, note that these function can not be called in the context of the main thread (read about Lua threading).

You can call Lua function (which may of may not yield) and setup success and failure callbacks:
```C++
LuaContinuation(env["fn"], env)       // Specify the function you are calling and Lua environment
  .call([](int result) {
    // On success
  }, [](LuaError &error) {
    // On failure
  }, 1, 2, 3);                        // Function arguments
```
You can also yield from C++ function:
```C++
LuaContinuation::yield(env, [](int argument) {   // Arguments are optional
  // On resume
  // You can return values there
}, 1, 2, 3);                                     // Values you are yielding
```

#### Lua error handling
Lua errors can be handled, using `LuaError` class. It provides status code (represented by enum `LuaStatusCode`) and additional error information. You can retrieve error object shortly after running Lua code:
```C++
LuaError error;
env.load("file.lua").error(error);            // When loading file
env.execute("fn(5)").error(error);            // When directly he running code
int value = env["lua_function"].error(error); // When calling Lua function or coroutine
```
Then you can process the error:
```C++
LuaError error;
...
if (error.hasError()) {
  LuaStatusCode status = error.getStatus(); // Get status code
  std::string message = error.getError();   // Extract error object
}
```
You can also read status code instead of full `LuaError`:
```C++
LuaStatusCode status;
env["fn"](5).status(status);
// And even
if (env["fn"](5) != LuaStatusCode::Ok) {
  // Do something
}
```
Status code list (integer representations of status codes are equal to corresponding `LUA_*` contants):
* `LuaStatusCode::Ok` - execution finished succesfully.
* `LuaStatusCode::Yield` - coroutine yielded (but not finished).
* `LuaStatusCode::RuntimeError` - Lua runtime error.
* `LuaStatusCode::SyntaxError` - Lua syntax error.
* `LuaStatusCode::MemAllocError` - memory allocation error.
* `LuaStatusCode::MsgHandlerError` - error occured while processing previous error in the message handler (should not occur unless you are writing custom message handler).
* `LuaStatusCode::GCMethodError` - error occured during `__gc` method execution (should not occur unless you are implementing custom userdata).


You can throw Lua errors using `LuaThrow` function:
```C++
int function(LuaState env, int x) {
  if (x <= 0) {
    LuaThrow(env, "X must be positive");
    // The rest of code won't be executed
  }
  return -x;
}
```
#### C++ exception handling
C++ exceptions thrown by native functions can bypass `LuaCppB` and Lua engine, which means that you should catch them (the only requirement - these exceptions should by derived from std::exception class). `LuaCppB` can also throw exceptions of type `LuaCppBError` which signalize internal error (e.g. incorrect pointer casts, corrupt Lua states). By default all exceptions (both `LuaCppB`-related and generated by native code) are simply rethrown. You can catch and process them by using `setExceptionHandler(std::function<void(std::exception &)>)` method of `LuaEnvironment`. It will catch all native code exceptions and most of `LuaCppB` errors (however, some critical errors can't be catched by handler). Example:
```C++
env.setExceptionHandler([](std::exception &ex) {
  throw ex;  // Just rethrow it. The default behavior
});
```

#### Custom memory allocators
Custom memory allocators are supported on standard Lua engine (not LuaJIT). To implement custom allocation mechanism, you should create `LuaAllocator` subclass as following:
```C++
class CustomLuaAllocator : public LuaAllocator {
 public:
  void *allocate(LuaType type, std::size_t size) override {
    // Allocation logic
    // Type may be Table, Function, String, Thread, Userdata or None (if it can be determined)
  }

  void *reallocate(void *ptr, std::size_t old_size, std::size_t new_size) override {
    // Reallocation logic
  }
  
  void deallocate(void *ptr, std::size_t size) override {
    // Deallocation login
  }
};
```
And bind it to `LuaState`:
```C++
auto alloc = std::make_shared<CustomLuaAllocator>();
state.setCustomAllocator(alloc);
```

#### Custom userdata types
`LuaCppB` supports custom userdata type definitions. You should use `CustomUserDataClass` to build userdata metatable and then use method `CustomUserDataClass::create` to instantiate `CustomUserData` objects. These objects can be implicitly converted into appropriate pointers and references, as well as passed to Lua. Example:
```C++
CustomUserDataClass<SomeType> udClass(env);
udClass.setDefaultConstructor([](SomeType &obj) {
  // You can assign a default constructor to your type.
  // The default constructor is optional.
  // For example, it could initialize objects.
  // The default "default constructor" just do nothing
  new(&obj) SomeType();
});
// Then you may bind metamethods to your userdata type
// Metamethods can be bound by name
udClass.bind("__add", [](SomeType &obj, int x) {
  return obj + x;
});
// Or using enumetation
udClass.bind(LuaMetamethod::GC, [](SomeType &obj) {
  // Finalizer could properly destroy object
  obj.~SomeType();
  ::operator delete(&obj, &obj);
});
// Then you instantiate your userdata's
auto ud1 = udClass.create(env.getState());                    // Uses default constructor
auto ud2 = udClass.create(env.getState(), [](auto& obj) {})   // Uses custom constructor
// Convert them to pointers or references
SomeType *st1 = ud1;
SomeType &st2 = ud2;  // Or vice versa. You can also use get method
// And pass them to Lua in the usual way
env["data"] = ud1;
env["fn"](ud2);
```
> **WARNING** You are responsible for proper resource initialization and finalizaton. Lua only allocates and deallocates memory. For example, you should use placement new & delete operators. It's recommended to attach appropriate default constructors to your types and implement `__gc` methods.
> 
Metamethod constants (part of `LuaMetamethod` enumeration):
* `GC`
* `Index`
* `NewIndex`
* `Call`
* `ToString`
* `Length`
* `Pairs`
* `IPairs`
* `UnaryMinus`
* `Add`
* `Subtract`
* `Multiply`
* `Divide`
* `FloorDivide`
* `Modulo`
* `Power`
* `Concat`
* `BitwiseAnd`
* `BitwiseOr`
* `BitwiseXor`
* `BitwiseNot`
* `ShiftLeft`
* `ShiftRight`
* `Equals`
* `LessThan`
* `LessOrEqual`