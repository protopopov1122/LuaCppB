## LuaCppB Compatibility

LuaCppB is aiming to the most recent Lua version (5.3.5 at the moment) as well as LuaJIT (compatible with
Lua 5.1), so overall LuaCppB is compatible with Lua API versions 5.1, 5.2 and 5.3. The complete inter-version 
compatibility is not an aim, so LuaCppB reduces it's functionality according to available Lua APIs and provides only polyfills that are vital for it's basic features. There is a list of macros allowing user to detect supported features:
* `LUACPPB_COROUTINE_SUPPORT` - Lua coroutines are completely supported for versions since 5.2. On 5.1 LuaCppB supports only basic coroutine operations.
* `LUACPPB_CONTINUATION_SUPPORT` - Lua continuations are supported only since Lua 5.2.
  mode.
* `LUACPPB_ERROR_SUPPORT` - the support of Lua error mechanism is available only in C++ compiled Lua.
* `LUACPPB_EXCEPTION_PROPAGATION_SUPPORT` - the support of C++ `std::exception`-based propagation is also tied
  to C++ mode.
* `LUACPPB_CUSTOM_ALLOCATOR_SUPPORT` - custom allocators are disabled for LuaJIT.
* `LUACPPB_DEBUG_SUPPORT` - debugging may be disabled for better performance.
* `LUACPPB_FAST_REFERENCE_SUPPORT` - faster access to variables using static polymorphism with fallback to default behaviour.
* `LUACPPB_HAS_JIT` - presence of LuaJIT specific APIs.

LuaCppB also disables some minor functions if they're not available:
* `LUACPPB_GC_ISRUNNING_SUPPORT` - existence of `LuaGC::isRunning` method, disabled for Lua 5.1.
* `LUACPPB_CONTAINER_PAIRS_SUPPORT` - `pairs` and `ipairs` functions for C++ container wrappers, disabled for Lua 5.1.
* `LUACPPB_DEBUG_EXTRAS_SUPPORT` - some extra debugging methods, disabled for Lua 5.1.
* `LUACPPB_GLOBAL_TABLE_SUPPORT` - `LuaState::getGlobals` method is disabled for Lua 5.1.
* `LUACPPB_NO_EXCEPTION_PASSTHROUGH` - C++ exception passing through C code is disabled on ARMs.

You can also forcefully disable some LuaCppB features in `Config.h` by uncommenting macro definitions:
* `LUACPPB_DISABLE_EXCEPTION_PROPAGATION`
* `LUACPPB_DISABLE_ERROR_SUPPORT`
* `LUACPPB_DISABLE_COROUTINE_SUPPORT`
* `LUACPPB_DISABLE_CUSTOM_ALLOCATOR`
* `LUACPPB_DISABLE_DEBUG`
* `LUACPPB_DISABLE_FAST_REFERENCES`
  