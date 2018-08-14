@echo off
rem GCC C++ Compile Debug
g++ -pipe -std=c++11 -pedantic -Wall -Wextra -Wno-unknown-pragmas -m64 -march=native -ffloat-store -fsignaling-nans -O0 -ggdb -c %*
