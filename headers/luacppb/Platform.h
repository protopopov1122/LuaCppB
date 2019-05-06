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

#ifndef LUACPPB_PLATFORM_H_
#define LUACPPB_PLATFORM_H_

#if defined(__clang__)
#define LUACPPB_COMPILER_CLANG

#elif defined(__ICC) || defined(__INTEL_COMPILER)
#define LUACPPB_COMPILER_UNSUPPORTED

#elif defined(__GNUC__) || defined(__GNUG__)
#define LUACPPB_COMPILER_GCC

#elif defined(_MSC_VER)
#define LUACPPB_COMPILER_MSC

#else
#define LUACPPB_COMPILER_UNSUPPORTED

#endif

#if defined(__arm__) || defined(_M_ARM)
#define LUACPPB_ARCH_ARM
#endif

#ifdef LUACPPB_COMPILER_UNSUPPORTED
#warning "Build using your compiler is not tested and may fail"
#endif

#endif