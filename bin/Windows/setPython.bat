@echo off
rem Python Setup

if exist C:\Python38\python.exe (
  set PYTHON_DIR=C:\Python38
  set PYTHON_LIB=C:\Python38\libs\python38.lib
) else (
if exist C:\Python37\python.exe (
  set PYTHON_DIR=C:\Python37
  set PYTHON_LIB=C:\Python37\libs\python37.lib
) else (
if exist C:\Python36\python.exe (
  set PYTHON_DIR=C:\Python36
  set PYTHON_LIB=%PYTHON_DIR%\libs\python36.lib
) else (
if exist C:\Python27\python.exe (
  set PYTHON_DIR=C:\Python27
  set PYTHON_LIB=C:\Python27\libs\python27.lib
) else (
if exist C:\Python\libs\python38.lib (
  set PYTHON_DIR=C:\Python
  set PYTHON_LIB=C:\Python\libs\python38.lib
) else (
if exist C:\Python\libs\python37.lib (
  set PYTHON_DIR=C:\Python
  set PYTHON_LIB=C:\Python\libs\python37.lib
) else (
if exist C:\Python\libs\python36.lib (
  set PYTHON_DIR=C:\Python
  set PYTHON_LIB=%PYTHON_DIR%\libs\python36.lib
) else (
if exist C:\Python\libs\python27.lib (
  set PYTHON_DIR=C:\Python
  set PYTHON_LIB=C:\Python\libs\python27.lib
) else (
  set PYTHON_DIR=
  echo Python not found
  exit /B 1
))))))))

set PYTHONDONTWRITEBYTECODE=x

echo. "%PATH%" | findstr /C:"%PYTHON_DIR%;%PYTHON_DIR%\Scripts;%PYTHON_DIR%\Tools\scripts" >nul 2>nul
if errorlevel 1 set PATH=%PATH%;%PYTHON_DIR%;%PYTHON_DIR%\Scripts;%PYTHON_DIR%\Tools\scripts
