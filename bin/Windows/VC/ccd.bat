@echo off
:: Visual C++ Compile Debug
cl /nologo /std:c++17 /Zc:__cplusplus /TP /EHsc /W4 /wd4068 /wd4100 /wd4127 /wd4505 /wd4996 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /Od /Z7 /RTCsu /MD /c %*
