## Lua C++ Bindings

This repo contains the prototype of Lua-C++ bindings. Currently it has following features:
* support of scalar data types and strings.
* access and modification of Lua variables an tables.
* transparent invocation of Lua functions.
* C++ class and object binding to Lua.
* transparent C++ object passing to/from Lua.
* support of smart pointers.
* support of some STL types - vectors, maps, pairs, tuples.
* support of coroutines.
* support of std::exception-based exception propagation and handlers

### Motivation
Project inspired by two things:
* create own compact Lua-C++ bindings to use in my projects.
* interop with C++ is my bachelor thesis topic.

### Author & License
Project author: Jevgenijs Protopopovs \
License: currently the project has no license, in future it will be licensed under some sort of permissive license.
