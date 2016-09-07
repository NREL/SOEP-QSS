@echo off
rem Intel C++ Compile Debug
icl /nologo /Qstd=c++11 /Qcxx-features /Wall /Qdiag-disable:809,1786,2259,11074,11075 /QxHOST /DNOMINMAX /DWIN32_LEAN_AND_MEAN /fp:source /Qtrapuv /check:stack,uninit /Gs0 /GS /Qfp-stack-check /traceback /Od /Z7 /MTd -D__builtin_huge_val()=HUGE_VAL -D__builtin_huge_valf()=HUGE_VALF -D__builtin_nan=nan -D__builtin_nanf=nanf -D__builtin_nans=nan -D__builtin_nansf=nanf /c %*
