## Lua C++ Bindings

This repo contains the prototype of Lua-C++ bindings. Currently it has following features:
* support of scalar data types and strings.
* access and modification of Lua variables an tables.
* basic support of enums
* transparent invocation of Lua functions.
* C++ class and object binding to Lua.
* transparent C++ object passing to/from Lua.
* support of smart pointers.
* support of some STL types - vectors, maps, pairs, tuples.
* support of coroutines.
* support of std::exception-based exception propagation and handlers.

Supported platforms: Windows (MinGW only), Linux (GCC and Clang), macOS(Clang), FreeBSD (Clang and GCC) & OpenBSD (Clang). \
Project requires C++17-compliant compiler version.

### Examples

Currently the only source of examples are unit tests. See `*.cpp` files in `tests/unit` directory. \
I'll prepare more meaningful examples soon.

### Building

Project is meant to be used as a static library (that is - it's not header-only and requires intergration into build system).
Currently CMake and Meson are supported (CMake is preferrable, currently Meson build file is quite messy).
CMake package is prepared in `cmake` directory, refer to `CMakeLists.txt` (which builds unit tests) as an usage example. \
It's strongly recommended to obtain Lua source code and cofigure build system to use it. It will be built along with other project code in C++ mode, this allows safe usage of coroutines and error mechanism (in C mode Lua uses longjmp which violates RAII).
Otherwise corresponding features of the library will be disabled.


### Motivation
Project inspired by two things:
* create own compact Lua-C++ bindings to use in my projects.
* interop with C++ is my bachelor thesis topic.

### Author & License
Project author: Jevgēnijs Protopopovs. \
Project is licensed under the terms of MIT license. See LICENSE and NOTICE files.
