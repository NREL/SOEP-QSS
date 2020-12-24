@echo off
:: MinGW C++ Compile Warnings
g++ -pipe -std=c++17 -pedantic -ansi -Wall -Wextra -Winline -Weffc++ -c %*
