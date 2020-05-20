@echo off
rem Visual C++ Compiler 64-Bit Setup

if exist "%PROGRAMFILES(X86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" (
  "%PROGRAMFILES(X86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
) else (
if exist "%PROGRAMFILES(X86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" (
  "%PROGRAMFILES(X86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
) else (
  echo Visual C++ not found
  exit /B 1
))
