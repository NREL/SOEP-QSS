@echo off
:: Intel Compiler 64-Bit Setup

:: OneAPI
if not "%ONEAPI_ROOT%" == "" "%ONEAPI_ROOT%\setvars.bat" intel64 vs2019

:: 2020
if not "%ICPP_COMPILER20%" == "" "%ICPP_COMPILER20%\bin\ipsxe-comp-vars.bat" intel64 vs2019

:: 2019
if not "%ICPP_COMPILER19%" == "" "%ICPP_COMPILER19%\bin\ipsxe-comp-vars.bat" intel64 vs2017

echo Supported Intel compiler not found!