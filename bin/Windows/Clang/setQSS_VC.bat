@echo off
:: Visual C++ Compiler Setup

:: VS2022
if exist "%PROGRAMFILES%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" (
  @"%PROGRAMFILES%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul
)

:: VS2019
if exist "%PROGRAMFILES(X86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" (
  @"%PROGRAMFILES(X86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul
)

:: VS2017
if exist "%PROGRAMFILES(X86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" (
  @"%PROGRAMFILES(X86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul
)

echo Visual C++ not found!
exit /B 1
