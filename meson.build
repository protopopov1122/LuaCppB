project('LuaCppB', 'cpp', 'c', default_options : ['cpp_std=c++17', 'warning_level=3'])

if meson.get_compiler('cpp').get_id() == 'gcc' or meson.get_compiler('cpp').get_id() == 'clang'
  add_global_arguments('-Wno-unused-parameter', '-pedantic', language : 'cpp')
  add_global_arguments('-x', 'c++', language : 'c')
endif

lib_src = [
  'source/Core/State.cpp', 'source/Value/Value.cpp', 'source/Value/Types.cpp',
  'source/Reference/Field.cpp', 'source/Reference/Handle.cpp',
  'source/Reference/Primary.cpp', 'source/Reference/Registry.cpp',
  'source/Core/Error.cpp', 'source/Core/StackGuard.cpp',
  'source/Core/Stack.cpp', 'source/Invoke/Coro.cpp',
  'source/Core/RuntimeInfo.cpp', 'source/Invoke/Lua.cpp',
  'source/Invoke/Native.cpp'
]

tests_src = [
  'tests/unit/main.cpp', 'tests/unit/Core.cpp', 'tests/unit/Reference.cpp',
  'tests/unit/Value.cpp', 'tests/unit/Invoke.cpp', 'tests/unit/Object.cpp',
  'tests/unit/Container.cpp'
]

if get_option('system_lua')
  lua = dependency('lua')
  headers = include_directories('headers')
  LuaCppBLib = static_library('luacppb', lib_src,
    include_directories : headers)

  lcppbunit = executable('lcppbunit', tests_src,
    include_directories: headers,
    link_with: [LuaCppBLib],
    dependencies: lua)
else
  lua_version = '5.3.5'
  lua_source_dir = 'lua-source/lua-' + lua_version + '/src'
  lua_header_dir = 'lua-source/lua-' + lua_version + '/src'

  lua_src = [
    lua_source_dir + '/ldblib.c',
    lua_source_dir + '/lmathlib.c',
    lua_source_dir + '/loslib.c',
    lua_source_dir + '/lvm.c',
    lua_source_dir + '/loadlib.c',
    lua_source_dir + '/lmem.c',
    lua_source_dir + '/lopcodes.c',
    lua_source_dir + '/lbaselib.c',
    lua_source_dir + '/ltable.c',
    lua_source_dir + '/ldump.c',
    lua_source_dir + '/liolib.c',
    lua_source_dir + '/lzio.c',
    lua_source_dir + '/lctype.c',
    lua_source_dir + '/lstring.c',
    lua_source_dir + '/lbitlib.c',
    lua_source_dir + '/llex.c',
    lua_source_dir + '/linit.c',
    lua_source_dir + '/ldebug.c',
    lua_source_dir + '/ldo.c',
    lua_source_dir + '/lauxlib.c',
    lua_source_dir + '/lcorolib.c',
    lua_source_dir + '/lutf8lib.c',
    lua_source_dir + '/lgc.c',
    lua_source_dir + '/lstate.c',
    lua_source_dir + '/lundump.c',
    lua_source_dir + '/ltablib.c',
    lua_source_dir + '/ltm.c',
    lua_source_dir + '/lparser.c',
    lua_source_dir + '/lobject.c',
    lua_source_dir + '/lcode.c',
    lua_source_dir + '/lfunc.c',
    lua_source_dir + '/lapi.c',
    lua_source_dir + '/lstrlib.c'
  ]
  lua = static_library('lua-cpp',
    lua_src,
    include_directories: include_directories(lua_header_dir)
  )

  headers = include_directories('headers', lua_header_dir)
  LuaCppBLib = static_library('luacppb', lib_src,
    include_directories : headers,
    cpp_args : '-DLUACPPB_CXX_MODE')

  lcppbunit = executable('lcppbunit', tests_src,
    include_directories: headers,
    cpp_args : '-DLUACPPB_CXX_MODE',
    link_with : [lua, LuaCppBLib])
endif

test('Unit tests', lcppbunit)