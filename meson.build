project('LuaCppB', 'cpp', default_options : ['cpp_std=c++17', 'warning_level=3'])

if meson.get_compiler('cpp').get_id() == 'gcc'
  add_global_arguments('-Wno-unused-parameter', language : 'cpp')
endif

lua_source_dir = 'lua-source/lua-5.3.5/src'
lua_header_dir = 'lua-source/lua-5.3.5/src'

lua_src = [
  lua_source_dir + '/ldblib.cc',
  lua_source_dir + '/lmathlib.cc',
  lua_source_dir + '/loslib.cc',
  lua_source_dir + '/lvm.cc',
  lua_source_dir + '/loadlib.cc',
  lua_source_dir + '/lmem.cc',
  lua_source_dir + '/lopcodes.cc',
  lua_source_dir + '/lbaselib.cc',
  lua_source_dir + '/ltable.cc',
  lua_source_dir + '/ldump.cc',
  lua_source_dir + '/liolib.cc',
  lua_source_dir + '/lzio.cc',
  lua_source_dir + '/lctype.cc',
  lua_source_dir + '/lstring.cc',
  lua_source_dir + '/lbitlib.cc',
  lua_source_dir + '/llex.cc',
  lua_source_dir + '/linit.cc',
  lua_source_dir + '/ldebug.cc',
  lua_source_dir + '/ldo.cc',
  lua_source_dir + '/lauxlib.cc',
  lua_source_dir + '/lcorolib.cc',
  lua_source_dir + '/lutf8lib.cc',
  lua_source_dir + '/lgc.cc',
  lua_source_dir + '/lstate.cc',
  lua_source_dir + '/lundump.cc',
  lua_source_dir + '/ltablib.cc',
  lua_source_dir + '/ltm.cc',
  lua_source_dir + '/lparser.cc',
  lua_source_dir + '/lobject.cc',
  lua_source_dir + '/lcode.cc',
  lua_source_dir + '/lfunc.cc',
  lua_source_dir + '/lapi.cc',
  lua_source_dir + '/lstrlib.cc'
]

# Expose library target.
lua = static_library('lua-cpp',
	lua_src,
	include_directories: include_directories(lua_header_dir)
)

headers = include_directories('headers', lua_header_dir)
LuaCppBLib = static_library('luacppb',
  'source/Core/State.cpp', 'source/Value/Value.cpp', 'source/Value/Types.cpp',
  'source/Reference/Field.cpp', 'source/Reference/Handle.cpp',
  'source/Reference/Primary.cpp', 'source/Reference/Registry.cpp',
  'source/Core/Error.cpp', 'source/Core/StackGuard.cpp',
  'source/Core/Stack.cpp', 'source/Invoke/Coro.cpp',
  'source/Core/RuntimeInfo.cpp', 'source/Invoke/Lua.cpp',
  'source/Invoke/Native.cpp',
  include_directories : headers)

lcppbunit = executable('lcppbunit',
  'tests/unit/main.cpp', 'tests/unit/Core.cpp', 'tests/unit/Reference.cpp',
  'tests/unit/Value.cpp', 'tests/unit/Invoke.cpp', 'tests/unit/Object.cpp',
  'tests/unit/Container.cpp',
  include_directories: headers, link_with : [lua, LuaCppBLib])

test('Unit tests', lcppbunit)