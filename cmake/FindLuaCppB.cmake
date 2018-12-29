# Parameters:
#	LUACPPB_COMPILE_LUA (on/off) - define for compiling custom Lua (recommended)
#	LUACPPB_LUA_SOURCE (path) - Lua source code directory (only if LUACPPB_COMPILE_LUA is defined)
#	LUACPPB_LUA_HEADERS (path)	- Lua header directory (if LUACPPB_COMPILE_LUA is not defined)
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
	set(LUACPPB_CUSTOM_LUA ON)
endif (LUACPPB_COMPILE_LUA)

if (LUACPPB_LUA_HEADERS)
	file(GLOB_RECURSE LUACPPB_SRC ${CMAKE_CURRENT_LIST_DIR}/../source/*.cpp)
	set(LUACPPB_HEADERS ${CMAKE_CURRENT_LIST_DIR}/../headers)
	add_library(luacppb STATIC ${LUACPPB_SRC})
	if (LUACPPB_CUSTOM_LUA)
		target_compile_definitions(luacppb PUBLIC -DLUACPPB_CXX_MODE)
	endif (LUACPPB_CUSTOM_LUA)
	target_include_directories(luacppb PUBLIC ${LUACPPB_HEADERS} ${LUACPPB_LUA_HEADERS})
	set(LUACPPB ON)
else (LUACPPB_LUA_HEADERS)
	message(FATAL_ERROR "Provide lua headers")
endif (LUACPPB_LUA_HEADERS)
