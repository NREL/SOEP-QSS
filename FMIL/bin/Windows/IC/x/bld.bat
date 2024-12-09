@echo off
:: Build FMIL Windows Intel C++ Release x

setlocal

:: Check FMIL_bin is set
if "%FMIL_bin%" == "" (
  echo FMIL_bin environment variable is not set
  exit /B 1
)

:: Set build-specific variables
set CC=icx
set C_FLAGS=/nologo /Qstd:c2x /EHsc /QxHOST /Wall /Wno-unused-function /DNOMINMAX /DWIN32_LEAN_AND_MEAN /DNDEBUG /O3 /fp:fast /MD

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
