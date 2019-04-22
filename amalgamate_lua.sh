#!/usr/bin/env bash
# This script uses the tool Amalgamate from https://github.com/vinniefalco/Amalgamate/

LUA_DIR=$1
DEST_DIR=$2
AMALGAMATE=$3

if [[ "$2" == "" ]]; then
	echo "Usage: $0 'Lua root' 'destination directory' [amalgamate tool path]"
	exit
fi

if [[ "$3" == "" ]]; then
	AMALGAMATE=amalgamate
fi

amalgamate=$(which "$AMALGAMATE")
if [[ -x "$amalgamate" ]] ; then
	echo "Install amalgamate from https://github.com/vinniefalco/Amalgamate/"
fi

find "$LUA_DIR/src" -name "*.c" -not -name "lua.c" -not -name "luac.c" -exec cat {} \; > "$DEST_DIR/lua.raw.c"
$amalgamate -i "$LUA_DIR/src" "$DEST_DIR/lua.raw.c" "$DEST_DIR/lua.c"
$amalgamate -i "$LUA_DIR/src" "$LUA_DIR/src/lua.h" "$DEST_DIR/lua.h"
$amalgamate -i "$LUA_DIR/src" "$LUA_DIR/src/lualib.h" "$DEST_DIR/lualib.h"
$amalgamate -i "$LUA_DIR/src" "$LUA_DIR/src/lauxlib.h" "$DEST_DIR/lauxlib.h"
rm "$DEST_DIR/lua.raw.c"
