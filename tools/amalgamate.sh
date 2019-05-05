#!/usr/bin/env bash
# This script uses the tool Amalgamate from https://github.com/vinniefalco/Amalgamate/

LUACPPB_DIR=$1
DEST_DIR=$2
AMALGAMATE=$3

if [[ "$2" == "" ]]; then
	echo "Usage: $0 'LuaCppB root' 'destination directory' [amalgamate tool path]"
	exit
fi

if [[ "$3" == "" ]]; then
	AMALGAMATE=amalgamate
fi

amalgamate=$(which "$AMALGAMATE")
if [[ -x "$amalgamate" ]] ; then
	echo "Install amalgamate from https://github.com/vinniefalco/Amalgamate/"
fi

find "$LUACPPB_DIR/source" -name "*.cpp" -exec cat {} \; -exec printf "\n" \; > "$DEST_DIR/LuaCppB.raw.cpp"
$amalgamate -i "$LUACPPB_DIR/headers" "$DEST_DIR/LuaCppB.raw.cpp" "$DEST_DIR/LuaCppB.cpp"
$amalgamate -i "$LUACPPB_DIR/headers" "$LUACPPB_DIR/headers/luacppb/LuaCppB.h" "$DEST_DIR/LuaCppB.h"
rm "$DEST_DIR/LuaCppB.raw.cpp"
