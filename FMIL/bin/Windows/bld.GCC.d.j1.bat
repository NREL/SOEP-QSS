@echo off
setlocal

set PATH=C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\bin;C:\CMake\bin;C:\MinGW\bin
set INCLUDE=
set LIB=
set LIBPATH=
set CC=gcc
set CXX=g++
set FMIL_SRC_DIR=%~dp0..\..\custom
set FMIL_INS_DIR=C:\FMIL.GCC.d
if exist "%FMIL_INS_DIR%" rd /S /Q "%FMIL_INS_DIR%" >nul 2>nul
del CMakeCache.txt >nul 2>nul

:: GCC
C:\CMake\bin\cmake.exe -G "MinGW Makefiles" -DFMILIB_INSTALL_PREFIX=%FMIL_INS_DIR% %FMIL_SRC_DIR% -DCMAKE_BUILD_TYPE=Debug
make -j1 VERBOSE=1 install

call "%~dp0\hdr.bat" "%FMIL_INS_DIR%"

endlocal
