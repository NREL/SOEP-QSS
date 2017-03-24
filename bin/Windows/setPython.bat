@echo off
rem Python Setup

if exist C:\Python36\python.exe (
  set PYTHON_DIR=C:\Python36
  set PYTHON_LIB=%PYTHON_DIR%\libs\python36.lib
) else (
if exist C:\Python27\python.exe (
  set PYTHON_DIR=C:\Python27
  set PYTHON_LIB=%PYTHON_DIR%\libs\python27.lib
) else (
if exist C:\Python\libs\python36.lib (
  set PYTHON_DIR=C:\Python
  set PYTHON_LIB=%PYTHON_DIR%\libs\python36.lib
) else (
if exist C:\Python\libs\python27.lib (
  set PYTHON_DIR=C:\Python
  set PYTHON_LIB=%PYTHON_DIR%\libs\python27.lib
) else (
  set PYTHON_DIR=
  echo Python not found
  exit /B 1
))))
set PYTHONDONTWRITEBYTECODE=x
set PATH=%PATH%;%PYTHON_DIR%;%PYTHON_DIR%\Scripts;%PYTHON_DIR%\Tools\scripts
