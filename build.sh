#!/bin/sh

PROGRAM_NAME=ray

BUILD_DIR=build/release
mkdir -p "$BUILD_DIR"

TARGET_COMPILER_FLAGS=(
    -O2 -fwrapv
)

COMPILER_FLAGS=(
    $TARGET_COMPILER_FLAGS
    -Wall -Wextra -Wpedantic -Wno-language-extension-token -Wno-unused-parameter -Wno-reorder-init-list -Wno-c99-designator -D_CRT_SECURE_NO_WARNINGS=1
    -Wno-gnu-anonymous-struct -Wno-missing-braces
    -std=c++20 -fvisibility=hidden
    -fno-exceptions -fno-unwind-tables -D_HAS_EXCEPTIONS=0
    -fno-rtti -mavx2 -ffast-math
    -o "$BUILD_DIR/$PROGRAM_NAME"
)

clang++ $COMPILER_FLAGS src/main.cpp

