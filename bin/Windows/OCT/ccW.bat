@echo off
:: OCT MinGW C++ Compile Warnings
g++ -pipe -std=c++20 -pedantic -ansi -Wall -Wextra -Winline -Weffc++ -c %*
