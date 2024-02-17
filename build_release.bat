@ECHO OFF

SETLOCAL

SET PROGRAM_NAME=ray

SET BUILD_DIR=build\release
IF NOT EXIST "%BUILD_DIR%\" (mkdir "%BUILD_DIR%")

REM Check for clang++
where clang++ 1>NUL 2>NUL
IF %ERRORLEVEL% NEQ 0 (echo "ERROR: clang++ not found. Please set up LLVM and add clang++ to PATH.") && (exit /b)

SET COMPILER_FLAGS=^
    -Ofast -fwrapv^
    -Wall -Wextra -Wpedantic -Wno-language-extension-token -Wno-unused-parameter -Wno-reorder-init-list -Wno-c99-designator -D_CRT_SECURE_NO_WARNINGS=1^
    -std=c++20 -fvisibility=hidden^
    -fno-exceptions -fno-unwind-tables -D_HAS_EXCEPTIONS=0^
    -fno-rtti -mavx2^
    -fuse-ld=lld -Wl,-opt:ref,-debug:full,-incremental:no,-subsystem:console,-manifest:no^
    -o "%BUILD_DIR%\%PROGRAM_NAME%.exe"

clang++ %COMPILER_FLAGS% src\main.cpp

