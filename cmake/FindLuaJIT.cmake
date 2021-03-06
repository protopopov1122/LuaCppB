if (DEFINED LUAJIT_DIR)
	set(LUAJIT_HEADERS ${LUAJIT_DIR})
	find_library(LUAJIT_LIBS NAMES luajit-5.1 lua51 PATHS ${LUAJIT_DIR})
else ()
	find_path(LUAJIT_DIR_HEADERS NAMES luajit-2.0)
	set(LUAJIT_HEADERS "${LUAJIT_DIR_HEADERS}/luajit-2.0")
	find_library(LUAJIT_LIBS NAMES luajit-5.1)
endif (DEFINED LUAJIT_DIR)
