@echo off
setlocal

set PATH=C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\bin;C:\CMake\bin
set INCLUDE=
set LIB=
set LIBPATH=
call setICX.64.bat
set CC=icx
set CXX=icx
set FMIL_SRC_DIR=%~dp0..\..\custom
set FMIL_INS_DIR=C:\FMIL.IX.r
if exist "%FMIL_INS_DIR%" rd /S /Q "%FMIL_INS_DIR%" >nul 2>nul
del CMakeCache.txt >nul 2>nul

:: IC++
C:\CMake\bin\cmake.exe -G "MinGW Makefiles" -DFMILIB_INSTALL_PREFIX=%FMIL_INS_DIR% %FMIL_SRC_DIR% -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS_RELEASE="/nologo /Wall /QxHOST /Qansi-alias /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DNDEBUG /O3 /fp:fast /Qip /MD"
make -j%NUMBER_OF_PROCESSORS% VERBOSE=1 install

:: IC++ 2020 with VC++ 2019 Generator
:: C:\CMake\bin\cmake.exe -G "Visual Studio 16 2019" -A x64 -T "Intel C++ Compiler 19.1" -DFMILIB_INSTALL_PREFIX=%FMIL_INS_DIR% %FMIL_SRC_DIR% -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS_RELEASE="/nologo /Wall /QxHOST /Qansi-alias /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DNDEBUG /O3 /fp:fast /Qip /MD"
:: C:\CMake\bin\cmake.exe --build . --config MinSizeRel --target install

call "%~dp0\hdr.bat" "%FMIL_INS_DIR%"

endlocal
