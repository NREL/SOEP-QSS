@echo off

:: Build FMIL
:: Usage: bldFMIL [<compiler> <build>]
:: Example: bldFMIL GCC r

setlocal

:: Set environment
if not "%2" == "" (
  call "%~dp0..\..\bin\Windows\%1\%2\setQSS.bat"
)

:: Check environment
if "%FMIL_bin%" == "" (
  echo FMIL_bin environment variable is not set
  exit /B 1
)

:: Build
call "%FMIL_bin%\bld.bat"
rd /S /Q "%FMIL_ins%\doc" >nul 2>&1
rd /S /Q "%FMIL%\src\ThirdParty\Expat\expat-2.4.8\src" >nul 2>&1
