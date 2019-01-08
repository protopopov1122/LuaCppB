luacppb_source = [
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
  'Reference/Base.cpp',
  'Reference/Field.cpp',
  'Reference/Handle.cpp',
  'Reference/Primary.cpp',
  'Reference/Registry.cpp',
  'Value/Types.cpp',
  'Value/Value.cpp'
]

luacppb_headers = include_directories('../headers')
LuaCppBLib = static_library('luacppb', luacppb_source,
  include_directories : [luacppb_headers, LUA_HEADERS],
  cpp_args : LUACPPB_CXX_MODE)
LUACPPB_DEPENDENCY = declare_dependency(dependencies : LUA,
  link_with : LuaCppBLib,
  include_directories : [luacppb_headers, LUA_HEADERS])