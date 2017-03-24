@echo off
rem Intel C++ Compile Debug
icl /nologo /Qstd=c++11 /Qcxx-features /Wall /Qdiag-disable:809,1786,2259,11074,11075 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /fp:source /Qtrapuv /check:stack,uninit /Gs0 /GS /Qfp-stack-check /traceback /Od /Z7 /MD /c %*
