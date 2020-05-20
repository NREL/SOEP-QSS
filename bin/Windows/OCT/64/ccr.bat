@echo off
rem OCT MinGW C++ Compile Release
g++ -pipe -std=c++17 -pedantic -Wall -Wextra -Wno-unknown-pragmas -Wno-attributes -m64 -march=native -DWIN32 -DQSS_FMU -DNDEBUG -fopenmp -Ofast -fno-stack-protector -finline-limit=2000 -s -c %*
