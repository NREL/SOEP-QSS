@echo off
:: OCT MinGW Compiler Setup
:: Put a custom version first in your PATH to adapt to your system

set Path=%Path%;C:\OCT\MinGW\bin

if  "%CPATH%" == "" (
  set CPATH=.
) else (
  set CPATH=%CPATH%;.
)

title OCT MinGW
