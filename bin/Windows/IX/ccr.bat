@echo off
:: Intel C++ Compile Release
icx /nologo /Qstd:c++20 /Qansi-alias /Qiopenmp /EHsc /QxHOST /Wall /Wno-unknown-pragmas /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DNDEBUG /O3 /fp:precise /MD /c %*
