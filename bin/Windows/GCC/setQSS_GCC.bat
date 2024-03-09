@echo off
:: GCC Compiler Setup

:: Put a custom version earlier in your PATH to adapt to your system

:: Put installed GCC at front of PATH to avoid using (old) GCC bundled with OCT
set GCC=C:\MinGW
set Path=%GCC%\bin;%Path%

if  "%CPATH%" == "" (
  set CPATH=.
) else (
  set CPATH=%CPATH%;.
)
