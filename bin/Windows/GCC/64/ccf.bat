@echo off
rem GCC C++ Compile Fast
g++ -pipe -std=c++11 -pedantic -Wall -Wextra -Winline -Wno-unknown-pragmas -fmessage-length=0 -m64 -march=native -DNDEBUG -Ofast -funroll-loops -s -c %*
