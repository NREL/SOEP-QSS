@echo off
:: Intel Compiler 64-Bit Setup: Customize to your system

:: oneAPI
if not "%ONEAPI_ROOT%" == "" (
  @call "%ONEAPI_ROOT%\compiler\latest\env\vars.bat" intel64 vs2022
)

echo Supported Intel compiler not found!
