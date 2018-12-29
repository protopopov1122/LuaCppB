cmake_minimum_required(VERSION 3.1)
project(LuaCppB)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(LUACPPB_COMPILE_LUA ON)
set(LUACPPB_LUA_SOURCE ${CMAKE_SOURCE_DIR}/lua-source/lua-5.3.5)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_LIST_DIR}/cmake/")
message(STATUS "${CMAKE_MODULE_PATH}")
find_package(LuaCppB REQUIRED)

if (LUACPPB_CUSTOM_LUA AND LUACPPB)
	file(GLOB_RECURSE LUACPPB_UNIT_SRC ${CMAKE_SOURCE_DIR}/tests/unit/*.cpp)
	add_executable(lcppunit ${LUACPPB_UNIT_SRC})
	target_include_directories(lcppunit PUBLIC ${CMAKE_SOURCE_DIR}/tests/unit ${LUACPPB_LUA_HEADERS} ${LUACPPB_HEADERS})
	target_link_libraries(lcppunit luacppb luacppb_lua)
endif (LUACPPB_CUSTOM_LUA AND LUACPPB)