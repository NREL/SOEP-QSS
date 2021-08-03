@echo off
:: Intel Compiler 64-Bit Setup

:: oneAPI
if not "%ONEAPI_ROOT%" == "" (
rem	@call "%ONEAPI_ROOT%\setvars.bat" intel64 vs2019
	@call "%ONEAPI_ROOT%\compiler\latest\env\vars.bat" intel64 vs2019
rem	@call "%ONEAPI_ROOT%\mkl\latest\env\vars.bat" intel64
rem	@call "%ONEAPI_ROOT%\tbb\latest\env\vars.bat" intel64 vs2019
rem	@call "%ONEAPI_ROOT%\mpi\latest\env\vars.bat"
	set "INTEL_DLL_DIR=%ONEAPI_ROOT%\compiler\latest\windows\redist\intel64_win\compiler"
	goto Exit
)

:: 2020
if not "%ICPP_COMPILER20%" == "" (
	@call "%ICPP_COMPILER20%\bin\ipsxe-comp-vars.bat" intel64 vs2019
	set "INTEL_DLL_DIR=%ICPP_COMPILER20%redist\intel64_win\compiler"
	goto Exit
)

:: 2019
if not "%ICPP_COMPILER19%" == "" (
	@call "%ICPP_COMPILER19%\bin\ipsxe-comp-vars.bat" intel64 vs2017
	set "INTEL_DLL_DIR=%ICPP_COMPILER19%redist\intel64_win\compiler"
	goto Exit
)

echo Supported Intel compiler not found!

:Exit
