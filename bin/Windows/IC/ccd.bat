@echo off
:: Intel C++ Compile Debug
icl /nologo /Qstd:c++20 /Qansi-alias /Qopenmp /EHsc /QxHOST /Zc:__cplusplus /Wall /Qdiag-disable:10382,10441,11074,11075 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /Od /fp:strict /GS /Gs0 /RTCsu /traceback /Z7 /Qtrapuv /check:stack,uninit /Qfp-stack-check /Qcheck-pointers:rw /Qcheck-pointers-dangling:all /MDd /c %*
