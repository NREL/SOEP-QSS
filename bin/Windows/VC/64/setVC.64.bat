@echo off
rem Visual C++ 2015 or 2013 Compiler 64-Bit Setup

if exist "%ProgramFiles32%\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" (
  "%ProgramFiles32%\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
)

"%ProgramFiles32%\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86_amd64
