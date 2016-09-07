@echo off
rem Intel Compiler 64-Bit Setup

rem Inspector XE 2016
if not "%INSPECTOR_XE_2016_DIR%" == "" (
  call "%INSPECTOR_XE_2016_DIR%\inspxe-vars.bat" quiet
  goto Compilers
)

rem Inspector XE 2015
if not "%INSPECTOR_XE_2015_DIR%" == "" (
  call "%INSPECTOR_XE_2015_DIR%\inspxe-vars.bat" quiet
  goto Compilers
)

rem Inspector XE 2013
if not "%INSPECTOR_XE_2013_DIR%" == "" (
  call "%INSPECTOR_XE_2013_DIR%\inspxe-vars.bat" quiet
  goto Compilers
)

:Compilers

rem Composer XE 2016
if not "%ICPP_COMPILER16%" == "" "%ICPP_COMPILER16%\bin\compilervars.bat" intel64
if not "%IFORT_COMPILER16%" == "" "%IFORT_COMPILER16%\bin\compilervars.bat" intel64

rem Composer XE 2015
if not "%ICPP_COMPILER15%" == "" "%ICPP_COMPILER15%\bin\compilervars.bat" intel64
if not "%IFORT_COMPILER15%" == "" "%IFORT_COMPILER15%\bin\compilervars.bat" intel64

rem Composer XE 2013 SP1
if not "%ICPP_COMPILER14%" == "" "%ICPP_COMPILER14%\bin\compilervars.bat" intel64
if not "%IFORT_COMPILER14%" == "" "%IFORT_COMPILER14%\bin\compilervars.bat" intel64

rem Composer XE 2013
if not "%ICPP_COMPILER13%" == "" "%ICPP_COMPILER13%\bin\compilervars.bat" intel64
if not "%IFORT_COMPILER13%" == "" "%IFORT_COMPILER13%\bin\compilervars.bat" intel64

rem Composer XE 2011
if not "%ICPP_COMPILER12%" == "" "%ICPP_COMPILER12%\bin\compilervars.bat" intel64
if not "%IFORT_COMPILER12%" == "" "%IFORT_COMPILER12%\bin\compilervars.bat" intel64

rem C++ and Fortran 11.x
if not "%ICPP_COMPILER11%" == "" call "%ICPP_COMPILER11%\bin\iclvars.bat" intel64
if not "%IFORT_COMPILER11%" == "" "%IFORT_COMPILER11%\bin\ifortvars.bat" intel64
