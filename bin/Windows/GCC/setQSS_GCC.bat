:: GCC Compiler Setup
@echo off

:: Put a custom version earlier in your PATH to adapt to your system

set GCC=C:\MinGW
set "PATH=%PATH%;%GCC%\bin"
:: Put installed GCC at front of PATH to avoid using (old) GCC bundled with OCT
:: set "PATH=%GCC%\bin;%PATH%"

if  "%CPATH%" == "" (
  set CPATH=.
) else (
  set "CPATH=%CPATH%;."
)
