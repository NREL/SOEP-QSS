@echo off
rem QSS Windows Setup

if not (%QSS%) == () goto Step2
set QSS=%~dp0
set QSS=%QSS:~0,-13%
:Step2

set Path=%Path%;%QSS%\bin\Windows;%QSS%\bin

rem ObjexxPlot Setup
set ObjexxPlot_bin=C:\Projects\ObjexxPlot\dev\dev\bin\Windows\VC\64\r
set Path=%Path%;%ObjexxPlot_bin%
call %ObjexxPlot_bin%\..\..\setQt.bat
call %ObjexxPlot_bin%\..\..\setQwt.bat
