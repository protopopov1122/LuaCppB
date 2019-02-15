# Parameters:
#	LUACPPB_COMPILE_LUA (on/off) - define for compiling custom Lua (recommended)
#	LUACPPB_LUA_SOURCE (path) - Lua source code directory (only if LUACPPB_COMPILE_LUA is defined)
#	LUACPPB_LUA_HEADERS (path)	- Lua header directory (if LUACPPB_COMPILE_LUA is not defined)
# LUACPPB_CXX_MODE (on/off) - Lua compiled as C++
# LUACPPB_LUAJIT (on/off) - use LuaJIT
# LUACPPB_FPIC (on/off) - produce position-independent code
# Results:
#	LUACPPB_CUSTOM_LUA - custom Lua was build as the static library 'luacppb_lua'
#	LUACPPB_LUA_HEADERS (path) - Lua header directory (if LUACPPB_COMPILE_LUA)
#	LUACPPB_HEADERS (path) - LuaCppB header directory
#	LUACPPB - static library 'luacppb'

if (LUACPPB_COMPILE_LUA)
	file(GLOB LUA_SRC ${LUACPPB_LUA_SOURCE}/src/*.c)
	SET_SOURCE_FILES_PROPERTIES(${LUA_SRC} PROPERTIES LANGUAGE CXX)
	set(LUACPPB_LUA_HEADERS ${LUACPPB_LUA_SOURCE}/src)
	add_library(luacppb_lua STATIC ${LUA_SRC})
	target_include_directories(luacppb_lua PUBLIC ${LUACPPB_LUA_HEADERS})
	if (LUACPPB_FPIC)
		set_property(TARGET luacppb_lua PROPERTY POSITION_INDEPENDENT_CODE ON)
	endif (LUACPPB_FPIC)
	set(LUACPPB_CUSTOM_LUA ON)
	set(LUACPPB_CXX_MODE ON)
endif (LUACPPB_COMPILE_LUA)


file(GLOB_RECURSE LUACPPB_SRC ${CMAKE_CURRENT_LIST_DIR}/../source/*.cpp)
set(LUACPPB_HEADERS ${CMAKE_CURRENT_LIST_DIR}/../headers)
add_library(luacppb STATIC ${LUACPPB_SRC})
if (LUACPPB_LUAJIT)
	target_compile_definitions(luacppb PUBLIC -DLUACPPB_LUAJIT)
endif (LUACPPB_LUAJIT)
if (LUACPPB_CXX_MODE)
	target_compile_definitions(luacppb PUBLIC -DLUACPPB_CXX_MODE)
endif (LUACPPB_CXX_MODE)
target_include_directories(luacppb PUBLIC ${LUACPPB_HEADERS})
if (LUACPPB_LUA_HEADERS)
	target_include_directories(luacppb PUBLIC ${LUACPPB_LUA_HEADERS})
endif (LUACPPB_LUA_HEADERS)
if (LUACPPB_FPIC)
	set_property(TARGET luacppb PROPERTY POSITION_INDEPENDENT_CODE ON)
endif (LUACPPB_FPIC)
set(LUACPPB ON)
