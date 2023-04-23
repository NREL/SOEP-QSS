@echo off
:: GCC C++ Compile Debug
g++ -pipe -std=c++20 -pedantic -fopenmp -march=native -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough=5 -DWIN32 -O0 -ffloat-store -fsignaling-nans -ftrapv -ggdb -c %*
