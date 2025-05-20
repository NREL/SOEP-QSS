:: Clang C++ Compile Debug
@echo off
clang++ -pipe -std=c++20 -pedantic -fopenmp=libomp -march=native -Wall -Wextra -Wno-unknown-pragmas -Wimplicit-fallthrough -O0 -ffp-model=strict -mdaz-ftz -ftrapv -fstack-protector -ggdb -c %*
:: -fsanitize=undefined not used because current Clang only ships with a release mode ubsan library
