project('LuaCppB', 'cpp', 'c', default_options : ['cpp_std=c++17', 'warning_level=3'])

if meson.get_compiler('cpp').get_id() == 'gcc' or meson.get_compiler('cpp').get_id() == 'clang'
  add_project_arguments('-Wno-unused-parameter', '-pedantic', language : 'cpp')
  add_project_arguments('-x', 'c++', language : 'c')
endif


subdir('lua-source')
subdir('source')
subdir('tests/unit')
subdir('benchmarks')
