:: Build FMIL Windows Clang Debug
@echo off

setlocal

:: Check FMIL_bin is set
if "%FMIL_bin%" == "" (
  echo FMIL_bin environment variable is not set
  exit /B 1
)

:: Set build-specific variables
set CC=clang
set C_FLAGS=-pipe -std=c2x -march=native -Wall -Wno-unused-function -Wno-unused-local-typedef -Wno-unused-label -Wno-unused-parameter -Wno-unused-but-set-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-sign-compare -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -Wno-implicit-function-declaration -Wno-misleading-indentation -Wno-pointer-bool-conversion -Wno-parentheses-equality -Wno-incompatible-pointer-types-discards-qualifiers -Wno-extra-tokens -Wno-shift-negative-value -O0 -ffp-model=strict -mdaz-ftz -ftrapv -fstack-protector -ggdb
:: -fsanitize=undefined not used because current Clang only ships with a release mode ubsan library

:: Enter CMake directory
if not exist %FMIL_cmk% md %FMIL_cmk% >nul 2>&1
cd %FMIL_cmk%

:: Run CMake if needed
if not exist Makefile (
  set CMAKE_COLOR_DIAGNOSTICS=OFF
  call cmake -G "MinGW Makefiles" %QSS%\FMIL\src -DFMILIB_INSTALL_PREFIX="%FMIL_ins%" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS_DEBUG="%C_FLAGS%"
)

:: Run GNU Make
make -j%NUMBER_OF_PROCESSORS% install
