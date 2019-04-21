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

#ifndef LUACPPB_CORE_STATE_H_
#define LUACPPB_CORE_STATE_H_

#include "luacppb/Base.h"
#include "luacppb/Core/Status.h"
#include "luacppb/Core/Runtime.h"
#include "luacppb/Core/Alloc.h"
#include "luacppb/Core/Panic.h"
#include "luacppb/Core/Debug.h"
#include "luacppb/Invoke/Error.h"
#include "luacppb/LuaJIT.h"
#include <string>
#include <memory>

namespace LuaCppB {

	class LuaReferenceHandle;
	namespace Internal {
		class LuaCppClassRegistry;
		class LuaFunctionCallResult;
		class LuaRegistryReference;
	}

#ifdef LUACPPB_DEBUG_SUPPORT
	using LuaDebugFrame = LuaAbstractDebugFrame<LuaReferenceHandle, Internal::LuaRegistryReference>;
	using LuaDebugHooks = LuaDebugAbstractHooks<LuaDebugFrame>;
#endif

	class LuaState : public LuaCppRuntime {
	 public:
		LuaState(lua_State *, std::shared_ptr<Internal::LuaRuntimeInfo> = nullptr);
		virtual ~LuaState() = default;
		bool isValid() const;
		lua_State *getState() const;
		Internal::LuaCppClassRegistry &getClassRegistry();
#ifdef LUACPPB_GLOBAL_TABLE_SUPPORT
		LuaReferenceHandle getGlobals();
#endif
		LuaState getMainThread();
#ifdef LUACPPB_DEBUG_SUPPORT
		LuaDebugFrame getDebugFrame(int = 0);
		LuaDebugHooks &getDebugHooks();
#endif
#ifdef LUACPPB_CUSTOM_ALLOCATOR_SUPPORT
		void setCustomAllocator(std::shared_ptr<LuaAllocator>);
#endif
#ifdef LUACPPB_HAS_JIT
		LuaJIT &getJit();
#endif
		void setPanicHandler(std::function<int(LuaState &)>);
		Internal::LuaCppObjectBoxerRegistry &getObjectBoxerRegistry() override;
		std::shared_ptr<Internal::LuaRuntimeInfo> &getRuntimeInfo() override;
    void setExceptionHandler(std::function<void(std::exception &)>) override;
    std::function<void(std::exception &)> getExceptionHandler() override;
		
		LuaReferenceHandle operator[](const std::string &);
		LuaReferenceHandle operator[](lua_Integer);
		Internal::LuaFunctionCallResult operator()(const std::string &, bool = true);
	 protected:
		lua_State *state;
		std::shared_ptr<Internal::LuaRuntimeInfo> runtime;
		std::function<void(std::exception &)> exception_handler;
#ifdef LUACPPB_CUSTOM_ALLOCATOR_SUPPORT
		std::shared_ptr<LuaAllocator> allocator;
#endif
#ifdef LUACPPB_HAS_JIT
		LuaJIT luaJit;
#endif
#ifdef LUACPPB_DEBUG_SUPPORT
	std::shared_ptr<LuaDebugHooks> debug;
#endif
	};

	class LuaUniqueState : public LuaState {
	 public:
		LuaUniqueState(lua_State * = nullptr);
		LuaUniqueState(const LuaUniqueState &) = delete;
		LuaUniqueState(LuaUniqueState &&);
		virtual ~LuaUniqueState();
		LuaUniqueState &operator=(const LuaUniqueState &) = delete;
		LuaUniqueState &operator=(LuaUniqueState &&);
	 private:
		Internal::LuaPanicUnbind panicUnbind;
	};

	class LuaEnvironment : public LuaUniqueState {
	 public:
		LuaEnvironment(bool = true);
		Internal::LuaFunctionCallResult load(const std::string &);
		Internal::LuaFunctionCallResult execute(const std::string &);
	 private:
		Internal::LuaFunctionCallResult pollResult(bool, int);
	};
}

#endif
