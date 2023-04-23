@echo off
:: OCT MinGW C++ Compile Warnings
g++ -pipe -std=c++20 -pedantic -Wall -Wextra -Wimplicit-fallthrough=5 -Wno-unknown-pragmas -Weffc++ -c %*
