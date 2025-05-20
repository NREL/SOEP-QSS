:: Clang Compiler Setup
@echo off

:: Put a custom version earlier in your PATH to adapt to your system

:: Clang from LLVM
if exist "%ProgramFiles%\LLVM" (
  set "Clang="%ProgramFiles%\LLVM"
) else if exist "%HOMEDRIVE%\LLVM" (
  set "Clang=C:\LLVM"
) else (
  echo Clang not found!
  exit /B 1
)
set "PATH=%PATH%;%Clang%\bin"

if  "%CPATH%" == "" (
  set CPATH=.
) else (
  set "CPATH=%CPATH%;."
)
