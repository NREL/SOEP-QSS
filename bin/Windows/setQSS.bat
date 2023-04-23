@echo off
:: QSS Windows Setup

if not (%QSS%) == () goto Step2
set QSS=%~dp0
set QSS=%QSS:~0,-13%
:Step2

set Path=%Path%;%QSS%\bin\Windows;%QSS%\bin

call setPython.QSS.bat

if not "%QSS_bin%" == "" (
  if "%PYTHONPATH%" == "" (
    set PYTHONPATH=%QSS_bin%
  ) else (
    set PYTHONPATH=%PYTHONPATH%;%QSS_bin%
  )
)

set "PYBIND_INC=%PYTHON_DIR%\lib\site-packages\pybind11\include"
if not "%CPATH%" == "" set CPATH=%CPATH%;%PYTHON_INC%;%PYBIND_INC%
if not "%INCLUDE%" == "" set INCLUDE=%INCLUDE%;%PYTHON_INC%;%PYBIND_INC%
if not "%LIB%" == "" set LIB=%LIB%;%PYTHON_DIR%\libs

set OMP_NUM_THREADS=6
set OMP_WAIT_POLICY=ACTIVE
