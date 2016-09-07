@echo off
rem GCC C++ Compile Warnings
g++ -pipe -std=c++11 -pedantic -ansi -Wall -Wextra -Winline -Weffc++ -ffor-scope -c %*
