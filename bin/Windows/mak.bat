@echo off
if exist %PlatformOSCompiler%\GNUmakefile (
  if not exist %PlatformFull% md %PlatformFull%
  cd %PlatformFull%
  make -O -j%NUMBER_OF_PROCESSORS% --makefile=../GNUmakefile SHELL=CMD %*
  cd ..\..\..
) else (
  echo %PlatformOSCompiler%\GNUmakefile not found
)
