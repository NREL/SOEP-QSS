@echo off
setlocal

set PATH=C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\bin;C:\CMake\bin;C:\OCT\MinGW\bin
set INCLUDE=
set LIB=
set LIBPATH=
set CC=gcc
set CXX=g++
set FMIL_SRC_DIR=%~dp0..\..\custom
set FMIL_INS_DIR=C:\FMIL.OCT.d
if exist "%FMIL_INS_DIR%" rd /S /Q "%FMIL_INS_DIR%" >nul 2>&1
del CMakeCache.txt >nul 2>&1

:: OCT MinGW
C:\CMake\bin\cmake.exe -G "MinGW Makefiles" -DFMILIB_INSTALL_PREFIX=%FMIL_INS_DIR% %FMIL_SRC_DIR% -DCMAKE_BUILD_TYPE=Debug
make -j%NUMBER_OF_PROCESSORS% VERBOSE=1 install

call "%~dp0\hdr.bat" "%FMIL_INS_DIR%"

endlocal
