@echo off
:: Intel C++ Compile Release
icl /nologo /Qstd=c++17 /Qcxx-features /Qvc14.2 /Wall /Qdiag-disable:809,1786,2259,3280,10382,11074,11075 /QxHOST /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /DQSS_FMU /DNDEBUG /Qopenmp /O3 /fp:precise /Qprec-div- /Qip /Qinline-factor:225 /c %*
:: /Qansi-alias removed to work-around Intel C++ 19.1 and 2020.1 bug
