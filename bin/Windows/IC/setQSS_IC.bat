:: Intel Compiler Setup
@echo off

:: oneAPI root setup
if "%ONEAPI_ROOT%" == "" (
  if exist "C:\Program Files (x86)\Intel\oneAPI\" (
    set "ONEAPI_ROOT=C:\Program Files (x86)\Intel\oneAPI"
  )
)

:: oneAPI + VS2022
if not "%ONEAPI_ROOT%" == "" (
  @call "%ONEAPI_ROOT%\compiler\latest\env\vars.bat" intel64 vs2022
  if exist "%ONEAPI_ROOT%\compiler\latest\windows\redist\intel64_win\compiler\" (
    set "INTEL_DLL_DIR=%ONEAPI_ROOT%\compiler\latest\windows\redist\intel64_win\compiler"
  ) else (
    set "INTEL_DLL_DIR=%ONEAPI_ROOT%\compiler\latest\bin"
  )
  goto Exit
)

:: oneAPI + VS2019
if not "%ONEAPI_ROOT%" == "" (
  @call "%ONEAPI_ROOT%\compiler\latest\env\vars.bat" intel64 vs2019
  if exist "%ONEAPI_ROOT%\compiler\latest\windows\redist\intel64_win\compiler\" (
    set "INTEL_DLL_DIR=%ONEAPI_ROOT%\compiler\latest\windows\redist\intel64_win\compiler"
  ) else (
    set "INTEL_DLL_DIR=%ONEAPI_ROOT%\compiler\latest\bin"
  )
  goto Exit
)

echo Supported Intel compiler not found!
exit /B 1

:Exit
