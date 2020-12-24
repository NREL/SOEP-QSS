@echo off
:: GCC C++ Compile Release
g++ -pipe -std=c++17 -pedantic -Wall -Wextra -Wno-unknown-pragmas -m64 -march=native -DWIN32 -DQSS_FMU -DNDEBUG -fopenmp -O3 -fno-stack-protector -finline-limit=2000 -s -c %*
