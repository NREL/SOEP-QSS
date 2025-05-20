:: GCC C++ Compile Release
@echo off
g++ -pipe -std=c++20 -pedantic -fopenmp -march=native -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough=5 -DNDEBUG -O3 -ffloat-store -mdaz-ftz -fno-stack-protector -finline-limit=2000 -c %*
