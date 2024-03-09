@echo off
:: QSS Windows Setup

if not (%QSS%) == () goto Step2
set QSS=%~dp0
set QSS=%QSS:~0,-13%
:Step2

set Path=%Path%;%QSS%\bin\Windows;%QSS%\bin

call setQSS_Python.bat

set OMP_NUM_THREADS=4
set OMP_PLACES=cores
set OMP_PROC_BIND=true
::set OMP_WAIT_POLICY=ACTIVE
