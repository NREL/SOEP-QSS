@echo off
:: OCT GCC Setup: Customize to your system

set "PATH=%PATH%;C:\OCT\MinGW\bin"

if  "%CPATH%" == "" (
  set CPATH=.
) else (
  set "CPATH=%CPATH%;."
)
