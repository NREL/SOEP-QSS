@echo off
if exist %PlatformCompiler%\GNUmakefile (
  if not exist %PlatformFull% md %PlatformFull%
  cd %PlatformFull%
  make -j%NUMBER_OF_PROCESSORS% --makefile=../../GNUmakefile SHELL=CMD %*
  cd ..\..\..\..
) else (
  echo %PlatformCompiler%\GNUmakefile not found
)
