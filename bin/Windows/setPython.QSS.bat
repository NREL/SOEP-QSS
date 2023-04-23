@echo off
:: Python Setup

if "%OCT_BIN_HOME%" == "" (
  if exist "C:\OCT" set "OCT_BIN_HOME=C:\OCT"
)

:: Find Python: Prefer OCT Python for building QSS Python extensions that work with OCT PyFMI
for %%V in (314 313 312 311 310 39 38 37 36 35) do (
  if exist "%OCT_BIN_HOME%" (
    if exist "%OCT_BIN_HOME%\Python%%V\python.exe" (
      set "PYTHON_DIR=%OCT_BIN_HOME%\Python%%V"
      set PYTHON_VER=%%V
      set "PYTHONPATH=%OCT_BIN_HOME%\install\Python;%PYTHONPATH%"
      goto Step2
    )
  )
)
for %%V in (314 313 312 311 310 39 38 37 36 35) do (
  if exist C:\Python%%V\python.exe (
    set PYTHON_DIR=C:\Python%%V
    set PYTHON_VER=%%V
    goto Step2
  ) else (
  if exist C:\Python\libs\python%%V.lib (
    set PYTHON_DIR=C:\Python
    set PYTHON_VER=%%V
    goto Step2
  ) else (
  if exist "C:\Program Files\Python%%V\python.exe" (
    set "PYTHON_DIR=C:\Program Files\Python%%V"
    set PYTHON_VER=%%V
    goto Step2
  ) else (
  if exist "C:\Program Files\Python\libs\python%%V.lib" (
    set "PYTHON_DIR=C:\Program Files\Python"
    set PYTHON_VER=%%V
    goto Step2
  ) else (
  if exist "C:%HOMEPATH%\AppData\Local\Programs\Python\Python%%V\python.exe" (
    set "PYTHON_DIR=C:%HOMEPATH%\AppData\Local\Programs\Python\Python%%V"
    set PYTHON_VER=%%V
    goto Step2
  )))))
)

:: Look for Anaconda installation
if exist "C:%HOMEPATH%\Anaconda3\python.exe" (
  set "PYTHON_DIR=C:%HOMEPATH%\Anaconda3"
  set PYTHON_VER=3
  goto Step2
) else (
if exist "C:%HOMEPATH%\AppData\Local\Continuum\anaconda3\python.exe" (
  set "PYTHON_DIR=C:%HOMEPATH%\AppData\Local\Continuum\anaconda3"
  set PYTHON_VER=3
  goto Step2
) else (
if exist "C:\ProgramData\Miniconda3\python.exe" (
  set "PYTHON_DIR=C:%HOMEPATH%\Anaconda3"
  set PYTHON_VER=3
  goto Step2
)))

echo Python not found
exit /B 1

:Step2
set "PYTHONHOME=%PYTHON_DIR%"
set "PYTHON_INC=%PYTHON_DIR%\include"
set "PYTHON_LIB_DIR=%PYTHON_DIR%\libs"
set "PYTHON_LIB=%PYTHON_LIB_DIR%\python%PYTHON_VER%.lib"
set PY_PYTHON=%PYTHON_VER:~0,1%.%PYTHON_VER:~1%
set PYTHONDONTWRITEBYTECODE=x

echo. "%PATH%" | findstr /C:"%PYTHON_DIR%;%PYTHON_DIR%\Scripts;%PYTHON_DIR%\Tools\scripts" >nul 2>&1
if errorlevel 1 set "PATH=%PATH%;%PYTHON_DIR%;%PYTHON_DIR%\Scripts;%PYTHON_DIR%\Tools\scripts"
