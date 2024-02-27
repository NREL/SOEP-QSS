@echo off
:: Build FMIL Windows Intel C++ Debug

setlocal

:: Check FMIL_bin is set
if (%FMIL_bin%) == () (
  echo FMIL_bin environment variable is not set
  exit /B 1
)

:: Set build-specific variables
set CC=icx
set C_FLAGS="/nologo /Qstd:c2x /Wall /Wno-unused-function /DNOMINMAX /DWIN32_LEAN_AND_MEAN /Od /fp:strict /GS /Gs0 /RTCsu /traceback /Z7 /MDd"

:: Enter CMake directory
if not exist %FMIL_cmk% md %FMIL_cmk% >nul 2>&1
cd %FMIL_cmk%

:: Run CMake if needed
if not exist Makefile (
  set CMAKE_COLOR_DIAGNOSTICS=OFF
  call cmake -G "MinGW Makefiles" %QSS%\FMIL\src -DFMILIB_INSTALL_PREFIX=%FMIL_ins% -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS_DEBUG=%C_FLAGS%
)

:: Run GNU Make
make -j%NUMBER_OF_PROCESSORS% install
ren %FMIL_ins%\lib\zlibd.lib zlib.lib >nul 2>&1
