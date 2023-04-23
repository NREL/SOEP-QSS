@echo off
:: Visual C++ Compile Release
cl /nologo /TP /std:c++20 /EHsc /arch:AVX2 /Zc:__cplusplus /W4 /wd4068 /wd4100 /wd4505 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /DNDEBUG /O2 /fp:precise /Qpar /GS- /MD /c %*
