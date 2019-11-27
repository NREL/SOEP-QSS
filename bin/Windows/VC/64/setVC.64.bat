@echo off
rem Visual C++ Compiler 64-Bit Setup

if exist "%ProgramFiles32%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" (
  "%ProgramFiles32%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
) else (
if exist "%ProgramFiles32%\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvarsall.bat" (
  "%ProgramFiles32%\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvarsall.bat" x64
) else (
if exist "%ProgramFiles32%\Microsoft Visual Studio\2017\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" (
  "%ProgramFiles32%\Microsoft Visual Studio\2017\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64
) else (
if exist "%ProgramFiles32%\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" (
  "%ProgramFiles32%\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
) else (
if exist "%ProgramFiles32%\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" (
  "%ProgramFiles32%\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86_amd64
) else (
  echo Visual C++ not found
  exit /B 1
)))))
