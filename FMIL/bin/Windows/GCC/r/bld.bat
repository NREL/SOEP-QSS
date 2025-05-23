:: Build FMIL Windows GCC Release
@echo off

setlocal

:: Check FMIL_bin is set
if "%FMIL_bin%" == "" (
  echo FMIL_bin environment variable is not set
  exit /B 1
)

:: Set build-specific variables
set CC=gcc
set C_FLAGS=-pipe -std=c2x -march=native -Wall -Wno-address -Wno-unknown-pragmas -Wno-implicit-fallthrough -Wno-unused-function -Wno-unused-label -Wno-unused-parameter -Wno-unused-but-set-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-sign-compare -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -Wno-implicit-function-declaration -Wno-old-style-definition -Wno-attributes -Wno-misleading-indentation -Wno-strict-aliasing -Wno-stringop-truncation -Wno-maybe-uninitialized -DNDEBUG -O3 -ffloat-store -mdaz-ftz -fno-stack-protector -finline-limit=2000

:: Enter CMake directory
if not exist %FMIL_cmk% md %FMIL_cmk% >nul 2>&1
cd %FMIL_cmk%

:: Run CMake if needed
if not exist Makefile (
  set CMAKE_COLOR_DIAGNOSTICS=OFF
  call cmake -G "MinGW Makefiles" %QSS%\FMIL\src -DFMILIB_INSTALL_PREFIX="%FMIL_ins%" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS_RELEASE="%C_FLAGS%"
)

:: Run GNU Make
make -j%NUMBER_OF_PROCESSORS% install
