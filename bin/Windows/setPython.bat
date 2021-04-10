@echo off
:: Python Setup

for %%V in (310 39 38 37 36 35 27) do (
  if exist C:\Python%%V\python.exe (
    set PYTHON_DIR=C:\Python%%V
	set PYTHON_VER=%%V
	goto :Step2
  ) else (
  if exist C:\Python\libs\python%%V.lib (
    set PYTHON_DIR=C:\Python
	set PYTHON_VER=%%V
	goto :Step2
  ) else (
  if exist "C:\Program Files\Python%%V\python.exe" (
    set "PYTHON_DIR=C:\Program Files\Python%%V"
	set PYTHON_VER=%%V
	goto :Step2
  ) else (
  if exist "C:\Program Files\Python\libs\python%%V.lib" (
    set "PYTHON_DIR=C:\Program Files\Python"
	set PYTHON_VER=%%V
	goto :Step2
  ) else (
  if exist "C:%HOMEPATH%\AppData\Local\Programs\Python\Python%%V\python.exe" (
    set "PYTHON_DIR=C:%HOMEPATH%\AppData\Local\Programs\Python\Python%%V"
	set PYTHON_VER=%%V
	goto :Step2
  )))))
)
echo Python not found
exit /B 1

:Step2
set "PYTHON_INC=%PYTHON_DIR%\include"
set "PYTHON_LIB_DIR=%PYTHON_DIR%\libs"
set "PYTHON_LIB=%PYTHON_LIB_DIR%\python%PYTHON_VER%.lib"
set PYTHONDONTWRITEBYTECODE=x

echo. "%PATH%" | findstr /C:"%PYTHON_DIR%;%PYTHON_DIR%\Scripts;%PYTHON_DIR%\Tools\scripts" >nul 2>nul
if errorlevel 1 set PATH=%PATH%;%PYTHON_DIR%;%PYTHON_DIR%\Scripts;%PYTHON_DIR%\Tools\scripts
