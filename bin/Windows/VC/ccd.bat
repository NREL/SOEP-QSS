:: Visual C++ Compile Debug
@echo off
cl /nologo /TP /std:c++20 /EHsc /arch:AVX2 /Zc:__cplusplus /W4 /wd4068 /wd4100 /wd4505 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /Od /fp:strict /GS /Gs0 /RTCsu /Z7 /MDd /c %*
