luacppb_test_source = [
  'main.cpp',
  'Core.cpp',
  'Debug.cpp',
  'Reference.cpp',
  'Value.cpp',
  'Invoke.cpp',
  'Object.cpp',
  'Container.cpp',
  'Coro.cpp',
  'UserData.cpp'
]


lcppbunit = executable('lcppbunit', luacppb_test_source,
  dependencies : LUACPPB_DEPENDENCY,
  cpp_args : LUACPPB_CXX_FLAGS)
test('Unit tests', lcppbunit)