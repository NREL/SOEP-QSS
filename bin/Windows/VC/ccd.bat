@echo off
rem Visual C++ Compile Debug
cl /nologo /TP /EHsc /W4 /wd4068 /wd4996 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /Od /Ob0 /Z7 /RTCsu /MD /c %*
