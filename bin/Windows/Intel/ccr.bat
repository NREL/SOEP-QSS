@echo off
rem Intel C++ Compile Release
icl /nologo /Qstd=c++11 /Qcxx-features /Wall /Qdiag-disable:809,1786,2259,11074,11075 /DVC_EXTRALEAN /DWIN32_LEAN_AND_MEAN /DNOMINMAX /DNDEBUG /O3 /fp:fast=2 /Qprec-div- /Qip -D__builtin_huge_val()=HUGE_VAL -D__builtin_huge_valf()=HUGE_VALF -D__builtin_nan=nan -D__builtin_nanf=nanf -D__builtin_nans=nan -D__builtin_nansf=nanf /c %*
