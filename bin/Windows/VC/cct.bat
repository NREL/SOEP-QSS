@echo off
rem Visual C++ Compile Test
cl /nologo /EHsc /W3 /wd4244 /wd4258 /wd4355 /wd4996 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /TP /DNDEBUG /O2 /Z7 /GS- /c %*
