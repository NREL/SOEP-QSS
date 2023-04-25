@echo off
:: Intel C++ Compile Debug
icx /nologo /Qstd:c++20 /Qansi-alias /Qiopenmp /EHsc /QxHOST /Wall /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /Od /fp:strict /GS /Gs0 /RTCsu /traceback /Z7 /MDd /c %*
