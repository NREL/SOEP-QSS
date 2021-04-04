@echo off
setlocal

set PATH=C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\bin;C:\CMake\bin
set INCLUDE=
set LIB=
set LIBPATH=
call setVC.64.bat
set CC=cl
set CXX=cl
set FMIL_SRC_DIR=%~dp0..\..\custom
set FMIL_INS_DIR=C:\FMIL.VC.d
if exist "%FMIL_INS_DIR%" rd /S /Q "%FMIL_INS_DIR%" >nul 2>nul
del CMakeCache.txt >nul 2>nul

:: VC++
C:\CMake\bin\cmake.exe -G "MinGW Makefiles" -DFMILIB_INSTALL_PREFIX=%FMIL_INS_DIR% %FMIL_SRC_DIR% -DCMAKE_BUILD_TYPE=Debug
make -j%NUMBER_OF_PROCESSORS% VERBOSE=1 install
ren %FMIL_INS_DIR%\lib\zlibd.lib zlib.lib >nul 2>nul

:: VC++ 2019: This builds with /MD instead of /MDd
:: C:\CMake\bin\cmake.exe -G "Visual Studio 16 2019" -A x64 -DFMILIB_INSTALL_PREFIX=%FMIL_INS_DIR% %FMIL_SRC_DIR% -DCMAKE_BUILD_TYPE=Debug
:: C:\CMake\bin\cmake.exe --build . --config MinSizeRel --target install

:: VC++ 2017: This builds with /MD instead of /MDd
:: C:\CMake\bin\cmake.exe -G "Visual Studio 15 2017" -A x64 -DFMILIB_INSTALL_PREFIX=%FMIL_INS_DIR% %FMIL_SRC_DIR% -DCMAKE_BUILD_TYPE=Debug
:: C:\CMake\bin\cmake.exe --build . --config MinSizeRel --target install

call "%~dp0\hdr.bat" "%FMIL_INS_DIR%"

endlocal
