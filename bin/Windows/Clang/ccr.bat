@echo off
:: Clang C++ Compile Release
clang++ -pipe -std=c++20 -pedantic -fopenmp=libomp -march=native -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough -DNDEBUG -O3 -ffp-model=precise -mdaz-ftz -fno-stack-protector -c %*
