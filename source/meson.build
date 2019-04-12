luacppb_source = [
  'Compat.cpp',
  'Core/Error.cpp',
  'Core/GC.cpp',
  'Core/RuntimeInfo.cpp',
  'Core/Stack.cpp',
  'Core/StackGuard.cpp',
  'Core/State.cpp',
  'Invoke/Callable.cpp',
  'Invoke/Continuation.cpp',
  'Invoke/Coro.cpp',
  'Invoke/Lua.cpp',
  'Invoke/Native.cpp',
  'LuaJIT.cpp',
  'Object/Field.cpp',
  'Reference/Base.cpp',
  'Reference/Field.cpp',
  'Reference/Handle.cpp',
  'Reference/Primary.cpp',
  'Reference/Registry.cpp',
  'Value/Iterator.cpp',
  'Value/Types.cpp',
  'Value/Value.cpp'
]

cpp = meson.get_compiler('cpp')
dep_atomic = declare_dependency()
if not cpp.links('''#include <atomic>
                    #include <cinttypes>
                    int main() {
                      std::atomic<uint64_t> n(0);
                      return n++;
	         }''', name : 'G++ atomic do not require -latomic')
  dep_atomic = cpp.find_library('atomic')
endif

luacppb_headers = include_directories('../headers')
LuaCppBLib = static_library('luacppb', luacppb_source,
  include_directories : [luacppb_headers, LUA_HEADERS],
  cpp_args : LUACPPB_CXX_FLAGS,
  dependencies: dep_atomic)
LUACPPB_DEPENDENCY = declare_dependency(dependencies : LUA,
  link_with : LuaCppBLib,
  include_directories : [luacppb_headers, LUA_HEADERS])
