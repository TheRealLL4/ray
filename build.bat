@ECHO OFF

SETLOCAL

REM cd to batch script location
CD /D "%~dp0"

REM Unpack command line arguments
FOR %%a IN (%*) DO SET "%%a=1"
IF "%debug%"=="1" (SET debug=1)

SET PROGRAM_NAME=ray

REM Set and create build directory
SET BUILD_DIR=build\release
IF "%debug%"=="1" (SET BUILD_DIR=build\debug)
IF NOT EXIST "%BUILD_DIR%\" (mkdir "%BUILD_DIR%")

REM Set target compiler flags
SET DEBUG_COMPILER_FLAGS=^
    -O0 -g -DDEVELOPER=1 -ftrapv

SET RELEASE_COMPILER_FLAGS=^
    -O2 -fwrapv

SET TARGET_COMPILER_FLAGS=%RELEASE_COMPILER_FLAGS%
IF "%debug%"=="1" (SET TARGET_COMPILER_FLAGS=%DEBUG_COMPILER_FLAGS%)

REM Set target linker flags (comma separated, no spaces)
SET DEBUG_LINKER_FLAGS=-debug:full,-opt:ref

SET RELEASE_LINKER_FLAGS=

SET TARGET_LINKER_FLAGS=%RELEASE_LINKER_FLAGS%
IF "%debug%"=="1" (SET TARGET_LINKER_FLAGS=%DEBUG_LINKER_FLAGS%)

REM Check for clang
where clang 1>NUL 2>NUL
IF %ERRORLEVEL% NEQ 0 (echo "ERROR: clang not found. Please set up LLVM and add clang to PATH.") && (exit /b)

SET COMPILER_FLAGS=^
    %TARGET_COMPILER_FLAGS%^
    -Wall -Wextra -Wpedantic -Wno-language-extension-token -Wno-unused-parameter -Wno-reorder-init-list -Wno-c99-designator^
    -D_CRT_SECURE_NO_WARNINGS=1 -Wno-gnu-anonymous-struct -Wno-missing-braces -Wno-nested-anon-types^
    -std=c++11 -fvisibility=hidden -fvisibility-inlines-hidden^
    -D_HAS_EXCEPTIONS=0 -fno-exceptions -fno-unwind-tables^
    -fno-rtti -mavx2 -ffast-math^
    -fuse-ld=lld -Wl,%TARGET_LINKER_FLAGS%,-incremental:no,-subsystem:console,-manifest:no^
    -o "%BUILD_DIR%\%PROGRAM_NAME%.exe"

clang %COMPILER_FLAGS% src\main.cpp

