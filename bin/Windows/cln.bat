@echo off

:: Build QSS
:: Usage: cln [<compiler> <build>]
:: Example: bld GCC r

setlocal

:: Set up build environment
if not "%2" == "" (
  call %QSS%\bin\Windows\%1\%2\setQSS.bat
)

:: Check QSS env var is set
if "%QSS%" == "" (
  echo QSS environment variable is not set
  exit /B 1
)

:: Clean objects
cd %QSS%\src\QSS\app && call mak.bat clean && call cln_dirs.bat
cd %QSS%\tst\QSS\unit && call mak.bat clean && call cln_dirs.bat

:: Clean binaries
if not "%QSS_bin%" == "" (
  for %%e in ( a def exe exp ilk lib pdb pyd ) do (
    del %QSS_bin%\*QSS.%%e >nul 2>&1
  )
)

endlocal
