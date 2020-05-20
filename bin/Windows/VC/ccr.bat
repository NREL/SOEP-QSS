@echo off
rem Visual C++ Compile Release
cl /nologo /std:c++17 /TP /EHsc /W4 /wd4068 /wd4100 /wd4127 /wd4505 /wd4996 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DNDEBUG /arch:AVX2 /O2 /fp:fast /Qpar /GS- /c %*
