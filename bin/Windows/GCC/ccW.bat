@echo off
:: GCC C++ Compile Warnings
g++ -pipe -std=c++20 -pedantic -fopenmp -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough=5 -Weffc++ -c %*
