:: GCC C++ Compile Debug
@echo off
g++ -pipe -std=c++20 -pedantic -fopenmp -march=native -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough=5 -O0 -ffloat-store -mdaz-ftz -fsignaling-nans -ftrapv -fstack-protector -ggdb -c %*
