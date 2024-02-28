@echo off
:: Clang Compiler Setup

:: Put a custom version earlier in your PATH to adapt to your system

set Clang=C:\LLVM
set Path=%Path%;%Clang%\bin

if  "%CPATH%" == "" (
  set CPATH=.
) else (
  set CPATH=%CPATH%;.
)
