@echo off
:: Intel C++ Compile Debug
icx /nologo /Qstd=c++20 /Qansi-alias /QxHOST /EHsc /Wall /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /Od /fp:strict /Gs0 /GS /traceback /Z7 /MDd /c %*
