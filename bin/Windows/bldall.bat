@echo off

rem Build all QSS builds from a repository root
rem Usage: bldall [make args]
rem Example: bldall -j8

call %~dp0\bld.bat GCC d %*
rem call %~dp0\bld.bat GCC p %*
rem call %~dp0\bld.bat GCC r %*

rem call %~dp0\bld.bat Intel d %*
call %~dp0\bld.bat Intel p %*
call %~dp0\bld.bat Intel r %*

rem call %~dp0\bld.bat VC d %*
rem call %~dp0\bld.bat VC p %*
call %~dp0\bld.bat VC r %*
