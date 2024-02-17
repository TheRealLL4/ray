#! /bin/sh

gcc -O3 -fwrapv -fno-exceptions -fno-rtti -fno-unwind-tables -Wall -Wpedantic -Wextra -mavx2 -fno-strict-aliasing -oray main.cpp

