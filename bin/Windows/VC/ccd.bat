@echo off
rem Visual C++ Compile Debug
cl /nologo /EHsc /W3 /wd4244 /wd4258 /wd4355 /wd4996 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /TP /Od /Ob0 /Z7 /RTCsu /MTd /c %CxxMainSource% %1 %2 %3 %4 %5 %6 %7 %8 %9
rem /RTCc gave exe blocked by Windows 8.1 Defender with ostringstream
