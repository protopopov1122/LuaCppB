cmake_minimum_required(VERSION 3.1)
project(LuaCppB)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(LUA_VERSION 5.3.5)

set(LUACPPB_COMPILE_LUA ON)
set(LUACPPB_LUA_SOURCE ${CMAKE_SOURCE_DIR}/lua-source/lua-${LUA_VERSION})
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_LIST_DIR}/cmake/")
find_package(LuaCppB REQUIRED)

if (LUACPPB)
	# Build unit tests
	file(GLOB_RECURSE LUACPPB_UNIT_SRC ${CMAKE_SOURCE_DIR}/tests/unit/*.cpp)
	add_executable(lcppbunit ${LUACPPB_UNIT_SRC})
	if (LUACPPB_CUSTOM_LUA)
		target_include_directories(lcppbunit PUBLIC ${LUACPPB_LUA_HEADERS})
	endif (LUACPPB_CUSTOM_LUA)
	target_include_directories(lcppbunit PUBLIC ${CMAKE_SOURCE_DIR}/tests/unit ${LUACPPB_HEADERS})
	target_link_libraries(lcppbunit luacppb luacppb_lua)
endif (LUACPPB)
