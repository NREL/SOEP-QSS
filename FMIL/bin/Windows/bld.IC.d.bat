@echo off
setlocal

set PATH=C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\bin;C:\CMake\bin
set INCLUDE=
set LIB=
set LIBPATH=
call setIC.64.bat
set CC=icl
set CXX=icl
set FMIL_SRC_DIR=%~dp0..\..\custom
set FMIL_INS_DIR=C:\FMIL.IC.d
if exist "%FMIL_INS_DIR%" rd /S /Q "%FMIL_INS_DIR%" >nul 2>nul
del CMakeCache.txt >nul 2>nul

:: IC++
C:\CMake\bin\cmake.exe -G "MinGW Makefiles" -DFMILIB_INSTALL_PREFIX=%FMIL_INS_DIR% %FMIL_SRC_DIR% -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS_DEBUG="/nologo /Wall /DNOMINMAX /DWIN32_LEAN_AND_MEAN /fp:source /Qtrapuv /check:stack,uninit /Gs0 /GS /Qfp-stack-check /traceback /Od /Z7 /MDd"
make -j%NUMBER_OF_PROCESSORS% VERBOSE=1 install
ren %FMIL_INS_DIR%\lib\zlibd.lib zlib.lib >nul 2>nul

:: IC++ 2020 with VC++ 2019 Generator
:: C:\CMake\bin\cmake.exe -G "Visual Studio 16 2019" -A x64 -T "Intel C++ Compiler 19.1" -DFMILIB_INSTALL_PREFIX=%FMIL_INS_DIR% %FMIL_SRC_DIR% -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS_DEBUG="/nologo /Wall /DNOMINMAX /DWIN32_LEAN_AND_MEAN /fp:source /Qtrapuv /check:stack,uninit /Gs0 /GS /Qfp-stack-check /traceback /Od /Z7 /MDd"
:: C:\CMake\bin\cmake.exe --build . --config MinSizeRel --target install

:: IC++ 2019 with VC++ 2017 Generator
:: C:\CMake\bin\cmake.exe -G "Visual Studio 15 2017" -A x64 -T "Intel C++ Compiler 19.0" -DFMILIB_INSTALL_PREFIX=%FMIL_INS_DIR% %FMIL_SRC_DIR% -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS_DEBUG="/nologo /Wall /DNOMINMAX /DWIN32_LEAN_AND_MEAN /fp:source /Qtrapuv /check:stack,uninit /Gs0 /GS /Qfp-stack-check /traceback /Od /Z7 /MDd"
:: C:\CMake\bin\cmake.exe --build . --config MinSizeRel --target install

call "%~dp0\hdr.bat" "%FMIL_INS_DIR%"

endlocal
