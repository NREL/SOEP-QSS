@echo off
:: OCT MinGW C++ Compile Release
g++ -pipe -std=c++20 -pedantic -fopenmp -march=native -Wall -Wextra -Wno-unknown-pragmas -Wno-attributes -Wimplicit-fallthrough=5 -DWIN32 -DNDEBUG -O3 -ffloat-store -fno-stack-protector -finline-limit=2000 -c %*
