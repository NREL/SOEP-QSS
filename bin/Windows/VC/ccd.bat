@echo off
:: Visual C++ Compile Debug
cl /nologo /TP /std:c++20 /EHsc /arch:AVX2 /Zc:__cplusplus /W4 /wd4068 /wd4100 /wd4505 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /Od /fp:strict /RTCsu /Z7 /MDd /c %*
