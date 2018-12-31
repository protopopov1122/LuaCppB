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
* support of std::exception-based exception propagation and handlers

Supported platforms: Windows (MinGW only), Linux (GCC and Clang), macOS(Clang), FreeBSD (Clang and GCC) & OpenBSD (Clang). \
Project requires C++17-compliant compiler version.

### Motivation
Project inspired by two things:
* create own compact Lua-C++ bindings to use in my projects.
* interop with C++ is my bachelor thesis topic.

### Author & License
Project author: Jevgenijs Protopopovs \
Project is licensed under the terms of MIT license. See LICENSE and NOTICE files.
