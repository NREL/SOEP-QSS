@echo off
rem Visual C++ Compile Release
cl /nologo /TP /EHsc /W4 /wd4068 /wd4996 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DNDEBUG /arch:AVX2 /O2 /fp:fast /Qpar /GS- /c %*
