@echo off

rem Build QSS
rem Usage: bld [<compiler> <build> [make args]]
rem Example: bld GCC r

setlocal

rem Set build
if not "%2" == "" (
  call %QSS%\bin\Windows\%1\64\%2\setProject.bat
)

rem Build QSS
cd %QSS%\src\QSS\app
call mak.bat %3 %4 %5 %6 %7 %8 %9
if "%2" == "d" (
  if not "%3" == "clean" (
    cd %QSS%\tst\QSS\unit
    call mak.bat %3 %4 %5 %6 %7 %8 %9 run
  )
)

endlocal
