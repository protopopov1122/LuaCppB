project('LuaCppB', 'cpp', default_options : ['cpp_std=c++17', 'warning_level=3'])

if meson.get_compiler('cpp').get_id() == 'gcc'
  add_global_arguments('-Wno-unused-parameter', language : 'cpp')
endif

headers = include_directories('headers')
LuaCppBLib = static_library('luacppb',
  'source/State.cpp', 'source/Value/Value.cpp', 'source/Value/Types.cpp',
  'source/Reference/Field.cpp', 'source/Reference/Handle.cpp',
  'source/Reference/Primary.cpp', 'source/Reference/Registry.cpp',
  'source/Error.cpp', 'source/Core/StackGuard.cpp',
  include_directories : headers)
lua = dependency('lua')
lcppbtest = executable('lcppbtest',
  'tests/main.cpp', 'tests/Reference.cpp',
  include_directories: headers, dependencies : lua, link_with : LuaCppBLib)