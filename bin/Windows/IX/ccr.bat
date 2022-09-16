@echo off
:: Intel C++ Compile Release
icx /nologo /Qstd=c++20 /Qansi-alias /Qiopenmp /QxHOST /EHsc /Wall /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /DNDEBUG /O3 /fp:precise /c %*
