@echo off
:: Intel Compiler 64-Bit Setup

:: OneAPI
if not "%ONEAPI_ROOT%" == "" "%ONEAPI_ROOT%\compiler\latest\env\vars.bat" intel64 vs2019

echo Supported Intel compiler not found!
