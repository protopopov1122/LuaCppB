## Lua C++ Bindings

[![Build Status](https://travis-ci.org/protopopov1122/LuaCppB.svg?branch=master)](https://travis-ci.org/protopopov1122/LuaCppB)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/protopopov1122/LuaCppB.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/protopopov1122/LuaCppB/context:cpp)

This repo contains the implementation of Lua-C++ bindings. Currently it has following features:
* support of Lua 5.1+ (including LuaJIT).
* support of scalar data types and strings.
* access and modification of Lua variables an tables.
* basic support of C++ enums.
* transparent invocation of Lua functions.
* C++ class and object binding to Lua.
* transparent C++ object passing to/from Lua.
* custom user data types
* support of smart pointers.
* support of some STL types - vectors, maps, sets, pairs, tuples and optionals.
* support of coroutines.
* support of std::exception-based exception propagation and handlers.
* wrappers for Lua stack, garbage collector, panic wrapper and custom memory allocator.
* debugger interface.
* `lua_dump` and `lua_load` wrappers.

#### Supported architectures & platforms
| Processor arhitecture | Operating system | Compilers            | Notes |
|-----------------------|------------------|----------------------|-------|
| amd64                 | Windows 10 1809  | MS Visual C++, MinGW ||
| amd64                 | Arch Linux       | GCC, Clang           ||
| amd64                 | FreeBSD 12       | GCC, Clang           ||
| amd64                 | OpenBSD 6.4      | Clang                ||
| amd64                 | macOS            | Clang                ||
| amd64                 | HaikuOS          | GCC 7.3              | Hybrid image |
| i386                  | Debian Stretch   | GCC                  ||
| armv7l                | Raspbian         | GCC                  | Custom GCC build |
| armv6l                | Raspbian         | GCC                  | Tested on emulator, custom GCC build |
| riscv64               | Fedora RISC-V    | GCC                  | Tested on emulator |
| WebAssembly           | Browser          | Emscripten           ||

Project requires C++17-compliant compiler version. It uses RTTI and exceptions.

### Documentation & examples

You can read the [manual](MANUAL.md). \
Example preparation is in progress. You can also consult the unit tests. See `*.cpp` files in `tests/unit` directory. LuaCppB compatibility issues are briefly discussed [there](COMPAT.md).

### Building

Project is meant to be used as a static library (that is - it's not header-only and requires intergration into build system).
Currently CMake and Meson are supported.
CMake package is prepared in `cmake` directory, refer to `CMakeLists.txt` (which builds unit tests) as an usage example. \
It's strongly recommended to obtain Lua source code and cofigure build system to use it. It will be built along with other project code in C++ mode, this allows safe usage of coroutines and error mechanism (in C mode Lua uses longjmp which violates RAII). If you are building Lua in C++ mode, you should define `LUACPPB_CXX_MODE` macro.
Otherwise corresponding features of the library will be disabled. \
LuaCppB supports amalgamation using the tool from [vinniefalco](https://github.com/vinniefalco/Amalgamate/), thus providing only two files: `LuaCppB.cpp` and `LuaCppB.h`; see `amalgamate.sh` script.


### Motivation
Project inspired by two things:
* create own compact Lua-C++ bindings to use in my projects.
* interop with C++ is my bachelor thesis topic.

### Author & License
Project author: Jevgēnijs Protopopovs. \
Project is licensed under the terms of MIT license. See LICENSE and NOTICE files.
