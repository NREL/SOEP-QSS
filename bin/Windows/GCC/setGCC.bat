@echo off
:: GCC Compiler Setup
:: Put a custom version first in your PATH to adapt to your system

set Path=%Path%;C:\GCC\bin
set EQ_LIBRARY_PATH=C:\GCC\x86_64-w64-mingw32\lib

if  "%CPATH%" == "" (
  set CPATH=.
) else (
  set CPATH=%CPATH%;.
)

title GCC
