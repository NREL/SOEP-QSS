@echo off
setlocal

set PATH=C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\CMake\bin;C:\bin
set CPATH=
set LIB=
set LIBPATH=
call setOCT.bat
set CC=gcc
set FMIL_SRC_DIR=%~dp0..\..\custom
set FMIL_INS_DIR=C:\FMIL.OCT.r
if exist "%FMIL_INS_DIR%" rd /S /Q "%FMIL_INS_DIR%" >nul 2>&1
del CMakeCache.txt >nul 2>&1

:: OCT GCC + Make
cmake -G "MinGW Makefiles" -DFMILIB_INSTALL_PREFIX=%FMIL_INS_DIR% %FMIL_SRC_DIR% -DCMAKE_BUILD_TYPE=Release
make -j%NUMBER_OF_PROCESSORS% VERBOSE=1 install

call "%~dp0\hdr.bat" "%FMIL_INS_DIR%"

endlocal
