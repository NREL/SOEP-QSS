:: Build FMIL Windows Visual C++ Debug
@echo off

setlocal

:: Check FMIL_bin is set
if "%FMIL_bin%" == "" (
  echo FMIL_bin environment variable is not set
  exit /B 1
)

:: Set build-specific variables
set CC=cl
set C_FLAGS=/nologo /std:c17 /EHsc /arch:AVX2 /W3 /wd4013 /wd4068 /wd4244 /wd4267 /wd4273 /wd5105 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /Od /fp:strict /GS /Gs0 /RTCsu /Z7 /MDd

:: Enter CMake directory
if not exist %FMIL_cmk% md %FMIL_cmk% >nul 2>&1
cd %FMIL_cmk%

:: Run CMake if needed
if not exist Makefile (
  set CMAKE_COLOR_DIAGNOSTICS=OFF
  call cmake -G "MinGW Makefiles" %QSS%\FMIL\src -DFMILIB_INSTALL_PREFIX="%FMIL_ins%" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS_DEBUG="%C_FLAGS%"
)

:: Run GNU Make
call make -j%NUMBER_OF_PROCESSORS% install
copy "%FMIL_lib%\zlibd.lib" "%FMIL_lib%\zlib.lib" >nul 2>&1
