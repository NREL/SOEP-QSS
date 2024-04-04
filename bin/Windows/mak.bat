@echo off
if "%OS_Compiler%" == "" (
  echo OS_Compiler environment variable is not set
  exit /B 1
)
if exist %OS_Compiler%\GNUmakefile (
  if not exist %OS_Compiler_Build% md %OS_Compiler_Build%
  cd %OS_Compiler_Build%
  make -O -j%NUMBER_OF_PROCESSORS% --makefile=../GNUmakefile SHELL=CMD %*
  cd ..\..\..
) else (
  echo %OS_Compiler%\GNUmakefile not found
  exit /B 1
)
