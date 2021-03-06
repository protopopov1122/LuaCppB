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

#ifndef LUACPPB_BASE_H_
#define LUACPPB_BASE_H_

#include "luacppb/Config.h"
#include "luacppb/Platform.h"

#ifdef LUA_VERSION_NUM
  #define LUACPPB_CUSTOM_LUA_INCLUDES
#else
  #ifdef LUACPPB_CXX_MODE
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
  #else
    #ifdef LUACPPB_LUAJIT
      #if __has_include(<luajit-2.0/lua.hpp>)
        #include <luajit-2.0/lua.hpp>
      #else
        #include <lua.hpp>
      #endif
    #else
      extern "C" {
      #include <lua.h>
      #include <lualib.h>
      #include <lauxlib.h>
      }
    #endif
  #endif
#endif

#ifdef LUACPPB_CXX_MODE
  #ifndef LUACPPB_DISABLE_COROUTINE_SUPPORT
    #define LUACPPB_COROUTINE_SUPPORT
  #endif

  #if !defined(LUACPPB_DISABLE_CONTINUATION_SUPPORT) && !defined(LUACPPB_DISABLE_COROUTINE_SUPPORT)
    #define LUACPPB_CONTINUATION_SUPPORT
  #endif

  #ifndef LUACPPB_DISABLE_ERROR_SUPPORT
    #define LUACPPB_ERROR_SUPPORT
  #endif

  #ifndef LUACPPB_DISABLE_EXCEPTION_PROPAGATION
    #define LUACPPB_EXCEPTION_PROPAGATION_SUPPORT
  #endif
#endif

#ifdef LUAJIT_VERSION_NUM
  #define LUACPPB_HAS_JIT
  #define LUACPPB_DISABLE_CUSTOM_ALLOCATOR
#endif

#ifndef LUACPPB_DISABLE_CUSTOM_ALLOCATOR
  #define LUACPPB_CUSTOM_ALLOCATOR_SUPPORT
#endif

#ifndef LUACPPB_DISABLE_DEBUG
  #define LUACPPB_DEBUG_SUPPORT
#endif

#ifndef LUACPPB_DISABLE_FAST_REFERENCES
  #define LUACPPB_FAST_REFERENCE_SUPPORT
#endif

#if defined(LUACPPB_ARCH_ARM) && !defined(LUACPPB_CXX_MODE)
  #define LUACPPB_NO_EXCEPTION_PASSTHROUGH
#endif

#include <cstddef>
#include "luacppb/Compat.h"

#endif
