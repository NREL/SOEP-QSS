@echo off
:: Visual C++ Compiler 64-Bit Setup

:: VC2019
if exist "%PROGRAMFILES(X86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" (
  "%PROGRAMFILES(X86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

::VC2017
if exist "%PROGRAMFILES(X86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" (
  "%PROGRAMFILES(X86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

echo Visual C++ not found
