@echo off
:: QSS Windows Setup

if not "%QSS%" == "" goto Step2
set QSS=%~dp0
set QSS=%QSS:~0,-13%
:Step2

set Path=%Path%;%QSS%\bin\Windows;%QSS%\bin

call setQSS_Python.bat

:: Use 6 P cores with 1 thread per core
set KMP_HW_SUBSET=6c:intel_core,1t
:: Don't use OMP_NUM_THREADS if using KMP_HW_SUBSET
::set OMP_NUM_THREADS=4
set OMP_PLACES=cores
set OMP_PROC_BIND=true
::set OMP_WAIT_POLICY=ACTIVE
