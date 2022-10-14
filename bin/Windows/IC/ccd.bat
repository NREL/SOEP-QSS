@echo off
:: Intel C++ Compile Debug
icl /nologo /Qstd=c++20 /Zc:__cplusplus /Qcxx-features /Wall /Qdiag-disable:809,1786,2259,3280,10382,10441,11074,11075 /QxHOST /Qansi-alias /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /fp:source /Qtrapuv /check:stack,uninit /Gs0 /GS /Qfp-stack-check /traceback /Od /Z7 /MD /c %*
