@echo off
:: Intel C++ Compile Debug
icx /nologo /Qstd=c++20 /EHsc /Wall /QxHOST /Qansi-alias /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /DQSS_FMU /fp:source /Qtrapuv /check:stack,uninit /Gs0 /GS /Qfp-stack-check /traceback /Od /Z7 /MD /c %*
