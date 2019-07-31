@echo off
rem MinGW Compiler Setup
rem Put a custom version first in your PATH to adapt to your system

set Path=%Path%;C:\MinGW\bin

if  "%CPATH%" == "" (
  set CPATH=.
) else (
  set CPATH=%CPATH%;.
)

title MinGW
