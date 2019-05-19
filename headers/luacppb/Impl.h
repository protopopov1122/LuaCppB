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

#ifndef LUACPPB_LUACPPB_IMPL_H_
#define LUACPPB_LUACPPB_IMPL_H_

#include "luacppb/Fwd.h"
#include "luacppb/Core/Impl/Stack.h"
#include "luacppb/Core/Impl/Debug.h"
#include "luacppb/Core/Impl/Library.h"
#include "luacppb/Core/Impl/Throw.h"
#include "luacppb/Container/Impl/Container.h"
#include "luacppb/Container/Impl/Map.h"
#include "luacppb/Container/Impl/Set.h"
#include "luacppb/Container/Impl/Tuple.h"
#include "luacppb/Container/Impl/Vector.h"
#include "luacppb/Invoke/Impl/ArgRet.h"
#include "luacppb/Invoke/Impl/Callable.h"
#include "luacppb/Invoke/Impl/Continuation.h"
#include "luacppb/Invoke/Impl/Coro.h"
#include "luacppb/Invoke/Impl/Descriptor.h"
#include "luacppb/Invoke/Impl/Invoke.h"
#include "luacppb/Invoke/Impl/Lua.h"
#include "luacppb/Invoke/Impl/Method.h"
#include "luacppb/Invoke/Impl/Native.h"
#include "luacppb/Object/Impl/Bind.h"
#include "luacppb/Object/Impl/Boxer.h"
#include "luacppb/Object/Impl/Class.h"
#include "luacppb/Object/Impl/Field.h"
#include "luacppb/Object/Impl/Method.h"
#include "luacppb/Object/Impl/Native.h"
#include "luacppb/Object/Impl/Object.h"
#include "luacppb/Object/Impl/Registry.h"
#include "luacppb/Object/Impl/Wrapper.h"
#include "luacppb/Reference/Impl/Base.h"
#include "luacppb/Reference/Impl/Field.h"
#include "luacppb/Reference/Impl/Handle.h"
#include "luacppb/Reference/Impl/FastPath.h"
#include "luacppb/Value/Impl/Factory.h"
#include "luacppb/Value/Impl/FastPath.h"
#include "luacppb/Value/Impl/Native.h"
#include "luacppb/Value/Impl/Types.h"
#include "luacppb/Value/Impl/UserData.h"
#include "luacppb/Value/Impl/Value.h"

#endif
