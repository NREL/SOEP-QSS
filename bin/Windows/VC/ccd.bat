@echo off
rem Visual C++ Compile Debug
cl /nologo /TP /EHsc /W3 /wd4244 /wd4258 /wd4355 /wd4996 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /Od /Ob0 /Z7 /RTCsu /MD /c %*
rem /RTCc gave exe blocked by Windows 8.1 Defender with ostringstream
