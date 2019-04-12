@echo off

rem Build QSS
rem Usage: bld [<compiler> <build> [make args]]
rem Example: bld GCC r -j12

setlocal

rem Set build
if not "%2" == "" (
  call bin\Windows\%1\64\%2\setProject.bat
)

rem Build QSS
cd %QSS%\src\QSS\app
call mak.bat %3 %4 %5 %6 %7 %8 %9
if "%2" == "d" (
  if not "%3" == "clean" (
    cd %QSS%\tst\QSS\unit
    call mak.bat -j8 %3 %4 %5 %6 %7 %8 %9 run
  )
)

endlocal
