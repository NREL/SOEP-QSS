@echo off

:: Clean QSS
:: Usage: clnQSS [<compiler> <build>]
:: Example: clnQSS GCC r

setlocal

:: Set environment
if not "%2" == "" (
  call "%~dp0\Windows\%1\%2\setQSS.bat"
)

:: Check environment
if "%QSS%" == "" (
  echo QSS environment variable is not set
  exit /B 1
)
if "%QSS_bin%" == "" (
  echo QSS_bin environment variable is not set
  exit /B 1
)

:: Clean QSS
cd %QSS%\src\QSS\app && call mak.bat clean && call cln_dirs.bat
cd %QSS%\tst\QSS\unit && call mak.bat clean && call cln_dirs.bat
for %%e in ( a def exe exp ilk lib pdb ) do (
  del %QSS_bin%\*QSS.%%e >nul 2>&1
)
