@echo off
:: GCC Compiler Setup

:: Put a custom version earlier in your PATH to adapt to your system

set GCC=C:\MinGW
set Path=%Path%;%GCC%\bin

if  "%CPATH%" == "" (
  set CPATH=.
) else (
  set CPATH=%CPATH%;.
)
