@echo off
rem MinGW C++ Compile Warnings
g++ -pipe -std=c++11 -pedantic -ansi -Wall -Wextra -Winline -Weffc++ -c %*
