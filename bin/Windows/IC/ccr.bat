@echo off
rem Intel C++ Compile Release
icl /nologo /Qstd=c++11 /Qcxx-features /Wall /Qdiag-disable:809,1786,2259,11074,11075 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DNDEBUG /Qansi-alias /QxHOST /O3 /fp:fast=2 /Qprec-div- /Qip /c %*
