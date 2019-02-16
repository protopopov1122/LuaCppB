# Parameters:
#	LUACPPB_COMPILE_LUA (on/off) - define for compiling custom Lua (recommended)
#	LUACPPB_LUA_SOURCE (path) - Lua source code directory (only if LUACPPB_COMPILE_LUA is defined)
#	LUACPPB_LUA_HEADERS (path)	- Lua header directory (if LUACPPB_COMPILE_LUA is not defined)
# LUACPPB_CXX_MODE (on/off) - Lua compiled as C++
# LUACPPB_LUAJIT (on/off) - use LuaJIT
# LUACPPB_FPIC (on/off) - produce position-independent code
# Results:
#	LUACPPB_LUA_HEADERS (path) - Lua header directory (if LUACPPB_COMPILE_LUA)
#	LUACPPB_HEADERS (path) - LuaCppB header directory
#	LUACPPB - static library 'luacppb'

if (LUACPPB_COMPILE_CUSTOM_LUA)
	file(GLOB LUA_SRC ${LUACPPB_CUSTOM_LUA_SOURCE}/src/*.c)
	SET_SOURCE_FILES_PROPERTIES(${LUA_SRC} PROPERTIES LANGUAGE CXX)
	set(LUACPPB_LUA_HEADERS ${LUACPPB_CUSTOM_LUA_SOURCE}/src)
	set(LUACPPB_DEFINES "LUACPPB_CXX_MODE")
	add_library(luacppb_lua STATIC ${LUA_SRC})
	target_include_directories(luacppb_lua PUBLIC ${LUACPPB_LUA_HEADERS})
	if (LUACPPB_FPIC)
		set_property(TARGET luacppb_lua PROPERTY POSITION_INDEPENDENT_CODE ON)
	endif (LUACPPB_FPIC)
	set(LUACPPB_LUA luacppb_lua)
elseif (LUACPPB_LUAJIT)
	find_package(LuaJIT REQUIRED)
	set(LUACPPB_DEFINES "LUACPPB_LUAJIT")
	set(LUACPPB_LUA_HEADERS ${LUAJIT_HEADERS})
	set(LUACPPB_LUA ${LUAJIT_LIBS})
else ()
	find_package(Lua REQUIRED)
	set(LUACPPB_DEFINES "")
	set(LUACPPB_LUA_HEADERS ${LUA_INCLUDE_DIR})
	set(LUACPPB_LUA ${LUA_LIBRARIES})
endif (LUACPPB_COMPILE_CUSTOM_LUA)


file(GLOB_RECURSE LUACPPB_SRC ${CMAKE_CURRENT_LIST_DIR}/../source/*.cpp)
set(LUACPPB_HEADERS ${CMAKE_CURRENT_LIST_DIR}/../headers ${LUACPPB_LUA_HEADERS})
add_library(luacppb STATIC ${LUACPPB_SRC})
target_include_directories(luacppb PUBLIC ${LUACPPB_HEADERS} ${LUACPPB_LUA_HEADERS})
target_compile_definitions(luacppb PUBLIC ${LUACPPB_DEFINES})
set(LUACPPB_LIBS luacppb ${LUACPPB_LUA})

if (LUACPPB_FPIC)
	set_property(TARGET luacppb PROPERTY POSITION_INDEPENDENT_CODE ON)
endif (LUACPPB_FPIC)
set(LUACPPB ON)
