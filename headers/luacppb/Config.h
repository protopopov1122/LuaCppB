/*
  SPDX short identifier: MIT

  Copyright 2018-2019 Jevgēnijs Protopopovs

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

#ifndef LUACPPB_CONFIG_H_
#define LUACPPB_CONFIG_H_

// You can use this file to configure your LuaCppB

// Uncomment these defines to explicitly disable some features
// #define LUACPPB_DISABLE_EXCEPTION_PROPAGATION
// #define LUACPPB_DISABLE_ERROR_SUPPORT
// #define LUACPPB_DISABLE_COROUTINE_SUPPORT
// #define LUACPPB_DISABLE_CONTINUATION_SUPPORT
// #define LUACPPB_DISABLE_CUSTOM_ALLOCATOR
// #define LUACPPB_DISABLE_DEBUG
// #define LUACPPB_DISABLE_FAST_REFERENCES

// Place there your Lua includes, if they use non-standard include paths. E.g.
// #include <lua5.3/lua.h>
// #include <lua5.3/lualib.h>
// #include <lua5.3/lauxlib.h>
// If your custom build is compiled by C++ compiler, uncomment line below
// #define LUACPPB_CXX_MODE

#endif
