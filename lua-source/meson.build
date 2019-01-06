custom_lua_version = get_option('custom_lua_version')
custom_lua_source_dir = 'lua-' + custom_lua_version + '/src'

custom_lua_source = [
  custom_lua_source_dir + '/ldblib.c',
  custom_lua_source_dir + '/lmathlib.c',
  custom_lua_source_dir + '/loslib.c',
  custom_lua_source_dir + '/lvm.c',
  custom_lua_source_dir + '/loadlib.c',
  custom_lua_source_dir + '/lmem.c',
  custom_lua_source_dir + '/lopcodes.c',
  custom_lua_source_dir + '/lbaselib.c',
  custom_lua_source_dir + '/ltable.c',
  custom_lua_source_dir + '/ldump.c',
  custom_lua_source_dir + '/liolib.c',
  custom_lua_source_dir + '/lzio.c',
  custom_lua_source_dir + '/lctype.c',
  custom_lua_source_dir + '/lstring.c',
  custom_lua_source_dir + '/lbitlib.c',
  custom_lua_source_dir + '/llex.c',
  custom_lua_source_dir + '/linit.c',
  custom_lua_source_dir + '/ldebug.c',
  custom_lua_source_dir + '/ldo.c',
  custom_lua_source_dir + '/lauxlib.c',
  custom_lua_source_dir + '/lcorolib.c',
  custom_lua_source_dir + '/lutf8lib.c',
  custom_lua_source_dir + '/lgc.c',
  custom_lua_source_dir + '/lstate.c',
  custom_lua_source_dir + '/lundump.c',
  custom_lua_source_dir + '/ltablib.c',
  custom_lua_source_dir + '/ltm.c',
  custom_lua_source_dir + '/lparser.c',
  custom_lua_source_dir + '/lobject.c',
  custom_lua_source_dir + '/lcode.c',
  custom_lua_source_dir + '/lfunc.c',
  custom_lua_source_dir + '/lapi.c',
  custom_lua_source_dir + '/lstrlib.c'
]

if get_option('system_lua')
  LUACPPB_CXX_MODE = ''
  LUA_HEADERS = []
  LUA = dependency('lua')
else
  LUACPPB_CXX_MODE = '-DLUACPPB_CXX_MODE'
  LUA_HEADERS = include_directories(custom_lua_source_dir)
  lua = static_library('lua-cpp', custom_lua_source,
    include_directories : LUA_HEADERS)
  LUA = declare_dependency(link_with : lua)
endif