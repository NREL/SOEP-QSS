@echo off
rem GCC C++ Compile Release
g++ -pipe -std=c++11 -pedantic -Wall -Wextra -Winline -Wno-unknown-pragmas -m64 -march=native -DNDEBUG -Ofast -fno-stack-protector -finline-limit=2000 -s -c %*
