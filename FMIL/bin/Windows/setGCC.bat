@echo off
:: GCC Setup: Customize to your system

set "PATH=%PATH%;C:\MinGW\bin"

if  "%CPATH%" == "" (
  set CPATH=.
) else (
  set "CPATH=%CPATH%;."
)
