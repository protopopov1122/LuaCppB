luacppb_bench_source = [
  'lua_c_api.cpp',
  'luacppb.cpp',
  'main.cpp',
  'sol.cpp'
]

cpp = meson.get_compiler('cpp')

lib_benchmark = cpp.find_library('benchmark',
  dirs : [meson.current_source_dir() + '/benchmark/build/src'],
  required : false)

if lib_benchmark.found() and get_option('benchmarks')
  if meson.get_compiler('cpp').get_id() == 'gcc'
    benchmark_cpp_args = [LUACPPB_CXX_FLAGS, '-Wno-unused-but-set-variable']
  else
    benchmark_cpp_args = LUACPPB_CXX_FLAGS
  endif
  inc_benchmark = include_directories('benchmark/include')
  thread_dep = dependency('threads')
  lcppbbench = executable('lcppbbench', luacppb_bench_source,
    dependencies : [LUACPPB_DEPENDENCY, lib_benchmark, thread_dep],
    include_directories: [inc_benchmark],
    cpp_args : benchmark_cpp_args)
endif

