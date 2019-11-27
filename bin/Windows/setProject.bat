@echo off
rem QSS Windows Setup

if not (%QSS%) == () goto Step2
set QSS=%~dp0
set QSS=%QSS:~0,-13%
:Step2

set Path=%Path%;%QSS%\bin\Windows;%QSS%\bin

call setPython.bat
