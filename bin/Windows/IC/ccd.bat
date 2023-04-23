@echo off
:: Intel C++ Compile Debug
icl /nologo /Qstd:c++20 /Qansi-alias /Qopenmp /EHsc /QxHOST /Zc:__cplusplus /Wall /Qdiag-disable:10382,10441,11074,11075 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /Od /fp:strict /Qtrapuv /RTCsu /check:stack,uninit /GS /Gs0 /Qfp-stack-check /Qcheck-pointers:rw /Qcheck-pointers-dangling:all /traceback /Z7 /MDd /c %*
