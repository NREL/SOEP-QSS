@echo off
rem OCT MinGW C++ Compile Fast
g++ -pipe -std=c++11 -pedantic -Wall -Wextra -Winline -Wno-unknown-pragmas -Wno-attributes -fmessage-length=0 -m64 -march=native -DNDEBUG -Ofast -funroll-loops -s -c %*
