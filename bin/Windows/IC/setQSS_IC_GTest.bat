:: Google Test Intel C++ Setup
@echo off

:: Put a custom version earlier in your PATH to adapt to your system

set GTEST_BIN=C:\gtest\bin\VC\MDd
set GTEST_INC=C:\gtest\include
set "PATH=%PATH%;%GTEST_BIN%"
