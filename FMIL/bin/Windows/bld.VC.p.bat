@echo off
setlocal

set PATH=C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\CMake\bin;C:\bin
set INCLUDE=
set LIB=
set LIBPATH=
call setVC.bat
set CC=cl
set FMIL_SRC_DIR=%~dp0..\..\custom
set FMIL_INS_DIR=C:\FMIL.VC.p
if exist "%FMIL_INS_DIR%" rd /S /Q "%FMIL_INS_DIR%" >nul 2>&1
del CMakeCache.txt >nul 2>&1

set FMIL_C_FLAGS="/nologo /std:c17 /EHsc /arch:AVX2 /W4 /wd4068 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DWIN32 /DNDEBUG /O2 /fp:fast /Qpar /GS- /Z7 /MD"
set FMIL_LINK_FLAGS="/DEBUG /PROFILE"
set FMIL_CMAKE_ARGS=-DFMILIB_INSTALL_PREFIX=%FMIL_INS_DIR% %FMIL_SRC_DIR% -DCMAKE_BUILD_TYPE=Release -DCMAKE_FMIL_C_FLAGS_RELEASE=%FMIL_C_FLAGS% -DCMAKE_FMIL_LINK_FLAGS=%FMIL_LINK_FLAGS%

:: VC++ + Make
cmake -G "MinGW Makefiles" %FMIL_CMAKE_ARGS%
make -j%NUMBER_OF_PROCESSORS% VERBOSE=1 install

:: VC++ + VS
:: cmake -G "Visual Studio 17 2022" %FMIL_CMAKE_ARGS%
:: cmake -G "Visual Studio 16 2019" %FMIL_CMAKE_ARGS%
:: cmake --build . --config MinSizeRel --target install

call "%~dp0\hdr.bat" "%FMIL_INS_DIR%"

endlocal
