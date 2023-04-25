@echo off
:: GCC Compiler Setup

:: Put a custom version first in your PATH to adapt to your system

set GCC=C:\MinGW
set Path=%GCC%\bin;%Path%

if  "%CPATH%" == "" (
  set CPATH=.
) else (
  set CPATH=%CPATH%;.
)
