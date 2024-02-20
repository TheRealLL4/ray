#!/bin/sh

PROGRAM_NAME=ray

BUILD_DIR=build/release
mkdir -p "$BUILD_DIR"

TARGET_COMPILER_FLAGS="
    -O2 -fwrapv
"

COMPILER_FLAGS="
    $TARGET_COMPILER_FLAGS
    -Wall -Wextra -Wpedantic -Wno-language-extension-token -Wno-unused-parameter -Wno-reorder-init-list -Wno-c99-designator -D_CRT_SECURE_NO_WARNINGS=1
    -Wno-gnu-anonymous-struct -Wno-missing-braces -Wno-unused-command-line-argument
    -std=c++11 -fvisibility=hidden -fvisibility-inlines-hidden
    -fno-exceptions -fno-unwind-tables -D_HAS_EXCEPTIONS=0
    -fno-rtti -ffast-math -march=native
    -static-libgcc -static-libstdc++
    -fms-extensions
    -lm
    -o "$BUILD_DIR/$PROGRAM_NAME"
"

clang $COMPILER_FLAGS src/main.cpp

