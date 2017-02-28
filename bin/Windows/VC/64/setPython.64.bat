@echo off
rem Python 64-Bit Development Setup
rem Put a custom version earlier in your PATH to adapt to your system

if exist C:\Python27\python.exe (
  set PYTHON_DIR=C:\Python27
) else (
  if exist C:\Python\python.exe (
    set PYTHON_DIR=C:\Python
  ) else (
    set PYTHON_DIR=
    echo Python not found: Put a custom setPython.64.bat file in your PATH
    exit /B 1
  )
)
set PYTHON_INC=%PYTHON_DIR%\include
set PYTHON_NUMPY_INC=%PYTHON_DIR%\lib\site-packages\numpy\core\include
set PYTHON_LIB_DIR=%PYTHON_DIR%\libs
set PYTHON_LIB=%PYTHON_LIB_DIR%\python27.lib
set PYTHONDONTWRITEBYTECODE=x
set PATH=%PATH%;%PYTHON_DIR%;%PYTHON_DIR%\Scripts;%PYTHON_DIR%\Tools\Scripts
