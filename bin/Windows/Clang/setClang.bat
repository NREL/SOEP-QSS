@echo off
:: Clang Compiler Setup

:: Put a custom version earlier in your PATH to adapt to your system

:: Clang from LLVM
if exist "%ProgramFiles%\LLVM" (
  set "Clang="%ProgramFiles%\LLVM"
) else (
if exist "%HOMEDRIVE%\LLVM" (
  set "Clang=C:\LLVM"
))
set "Path=%Path%;%Clang%\bin"

if  "%CPATH%" == "" (
  set CPATH=.
) else (
  set CPATH=%CPATH%;.
)
