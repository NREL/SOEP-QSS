@echo off
:: GCC C++ Compile Debug
g++ -pipe -std=c++17 -pedantic -Wall -Wextra -Wno-unknown-pragmas -m64 -march=native -DWIN32 -DQSS_FMU -ffloat-store -fsignaling-nans -fno-omit-frame-pointer -O0 -ggdb -c %*
