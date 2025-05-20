:: Intel C++ Compile Debug
@echo off
icx /nologo /Qstd:c++20 /Qansi-alias /Qiopenmp /EHsc /QxHOST /Wall /Wno-unknown-pragmas /DNOMINMAX /DWIN32_LEAN_AND_MEAN /Od /fp:precise /Qftz /GS /Gs0 /RTCsu /Z7 /MDd /c %*
