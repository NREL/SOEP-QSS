@echo off
rem Visual C++ Compile Release
cl /nologo /TP /EHsc /W3 /wd4244 /wd4258 /wd4355 /wd4996 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DNDEBUG /O2 /fp:fast /Qpar /GS- /c %*
