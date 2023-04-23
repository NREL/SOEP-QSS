@echo off
:: OCT MinGW C++ Compile Debug
g++ -pipe -std=c++20 -pedantic -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough=5 -Wno-attributes -march=native -DWIN32 -O0 -ffloat-store -fsignaling-nans -ftrapv -ggdb -c %*
