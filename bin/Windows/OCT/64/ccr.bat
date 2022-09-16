@echo off
:: OCT MinGW C++ Compile Release
g++ -pipe -std=c++20 -pedantic -Wall -Wextra -Wno-unknown-pragmas -Wno-attributes -m64 -march=native -DWIN32 -DNDEBUG -fopenmp -O3 -fno-stack-protector -finline-limit=2000 -s -c %*
