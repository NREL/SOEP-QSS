@echo off
rem Visual C++ Compiler 64-Bit Setup

if exist "%PROGRAMFILES(X86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" (
  "%PROGRAMFILES(X86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
) else (
if exist "%PROGRAMFILES(X86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" (
  "%PROGRAMFILES(X86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
) else (
if exist "%PROGRAMFILES(X86)%\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" (
  "%PROGRAMFILES(X86)%\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
) else (
if exist "%PROGRAMFILES(X86)%\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" (
  "%PROGRAMFILES(X86)%\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86_amd64
) else (
  echo Visual C++ not found
  exit /B 1
))))
