@echo off
rem Intel Compiler 64-Bit Setup

rem 2020
if not "%ICPP_COMPILER20%" == "" (
  @call "%ICPP_COMPILER20%\bin\ipsxe-comp-vars.bat" intel64 vs2019
  goto Exit
) else (
if not "%IFORT_COMPILER20%" == "" (
  @call"%IFORT_COMPILER20%\bin\ipsxe-comp-vars.bat" intel64 vs2019
  goto Exit
))

rem 2019
if not "%ICPP_COMPILER19%" == "" (
  @call "%ICPP_COMPILER19%\bin\ipsxe-comp-vars.bat" intel64 vs2017
  goto Exit
) else (
if not "%IFORT_COMPILER19%" == "" (
  @call"%IFORT_COMPILER19%\bin\ipsxe-comp-vars.bat" intel64 vs2017
  goto Exit
))

:Exit
