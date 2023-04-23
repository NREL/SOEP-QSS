@echo off
:: Intel Compiler Setup

:: oneAPI
if not "%ONEAPI_ROOT%" == "" (
	@"%ONEAPI_ROOT%\compiler\latest\env\vars.bat" intel64 vs2022
)

echo Intel C++ not found!
exit /B 1
