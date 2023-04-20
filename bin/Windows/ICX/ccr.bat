@echo off
:: Intel C++ Compile Release
icx /nologo /Qstd=c++20 /EHsc /Wall /QxHOST /Qansi-alias /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /DQSS_FMU /DNDEBUG /Qiopenmp /O3 /fp:precise /c %*
