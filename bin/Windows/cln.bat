@echo off

setlocal

:: Check QSS env var is set
if "%QSS%" == "" (
  echo QSS environment variable is not set
  exit /B 1
)

cd %QSS%\src\QSS\app && call mak.bat clean
cd %QSS%\tst\QSS\unit && call mak.bat clean

:: Clean binaries
if not "%QSS_bin%" == "" (
  del %QSS_bin%\*.a >nul 2>&1
  del %QSS_bin%\*.exe >nul 2>&1
  del %QSS_bin%\*.exp >nul 2>&1
  del %QSS_bin%\*.def >nul 2>&1
  del %QSS_bin%\*.ilk >nul 2>&1
  del %QSS_bin%\*.lib >nul 2>&1
  del %QSS_bin%\*.pdb >nul 2>&1
  del %QSS_bin%\*.pyd >nul 2>&1
)

endlocal
