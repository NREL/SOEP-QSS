:: GCC C++ Compile Warnings
@echo off
g++ -pipe -std=c++20 -pedantic -fopenmp -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough=5 -Weffc++ -c %*
